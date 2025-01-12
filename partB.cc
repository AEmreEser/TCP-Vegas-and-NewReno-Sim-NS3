#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"

#include <iomanip>

using namespace ns3;

#ifndef SIM_END
#define SIM_END 10.0 
#endif

NS_LOG_COMPONENT_DEFINE("PartB");

// Global variables for statistics
uint64_t g_totalBytesReceived = 0;
Time g_lastRxTime;
std::vector<double> g_delays;

// Callback function to track received packets
void RxTrace(Ptr<const Packet> packet, const Address& address) {
    g_totalBytesReceived += packet->GetSize();
    g_lastRxTime = Simulator::Now();
    g_delays.push_back(Simulator::Now().GetMilliSeconds());
}


void RunSimulation(double load, std::string tcpVariant, std::ofstream & outFile) {
    // Create nodes
    NodeContainer lanNodes;
    lanNodes.Create(3);  // A1, A2, A3
    NodeContainer routerNode;
    routerNode.Create(1);  // Router B
    NodeContainer destNode;
    destNode.Create(1);  // Node C

    // Create LAN
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps")); // lan rate 100Mbps
    csma.SetChannelAttribute("Delay", StringValue("1ms"));

    NetDeviceContainer lanDevices = csma.Install(lanNodes);

    // Create point-to-point links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps")); // router & dest conn. rate: 100Mbps
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));
    // p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("10p")); // fifo queue in every connection // makes almost no difference

    // A3 - Router Connection
    NetDeviceContainer routerDevices = p2p.Install(lanNodes.Get(2), routerNode.Get(0));
    // Router - Node C connection
    NetDeviceContainer destDevices = p2p.Install(routerNode.Get(0), destNode.Get(0));

    // Install Internet stack with TCP variant
    InternetStackHelper internet;
    Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                      StringValue("ns3::" + tcpVariant));

    internet.Install(lanNodes);
    internet.Install(routerNode);
    internet.Install(destNode);

    // Assign IP addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.1.0", "255.255.255.0");
    // addresses 10.0.1.1 to .3 assigned to A{1..3} nodes
    Ipv4InterfaceContainer lanInterfaces = ipv4.Assign(lanDevices);

    ipv4.SetBase("10.0.1.252", "255.255.255.252"); // A1 - Router gets ip 10.0.1.253,254
    Ipv4InterfaceContainer routerInterfaces = ipv4.Assign(routerDevices);

    ipv4.SetBase("192.168.3.0", "255.255.255.0");
    Ipv4InterfaceContainer destInterfaces = ipv4.Assign(destDevices); // router - C 

    // Enable routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();



    // Create TCP application
    uint16_t port = 8080;
    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(destInterfaces.GetAddress(1), port));
    ApplicationContainer sinkApp = sink.Install(destNode.Get(0)); // install it on C
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(SIM_END));

    // Configure TCP sender
    // cannot use bulk send application since we cannot control its sending rate 
    OnOffHelper source("ns3::TcpSocketFactory", InetSocketAddress(destInterfaces.GetAddress(1), port));
    source.SetAttribute("DataRate", DataRateValue(DataRate(std::to_string(load) + "Mbps")));
    source.SetAttribute("PacketSize", UintegerValue(512));
    source.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")); // always working
    source.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    ApplicationContainer sourceApp = source.Install(lanNodes.Get(0));  // installed on A1
    sourceApp.Start(Seconds(0.0));
    sourceApp.Stop(Seconds(SIM_END));

    // Add tracing
    Config::ConnectWithoutContext(
        "/NodeList/" + std::to_string(destNode.Get(0)->GetId()) +
        "/ApplicationList/*/$ns3::PacketSink/Rx",
        MakeCallback(&RxTrace));

    // Enable PCAP tracing
    csma.EnablePcapAll("part_b_lan");
    p2p.EnablePcapAll("part_b_p2p");

    // this helps us obtain statistics
    FlowMonitorHelper flowmonitor;
    Ptr<FlowMonitor> flowmon = flowmonitor.InstallAll();

    // Run simulation
    Simulator::Stop(Seconds(SIM_END));
    Simulator::Run();

    flowmon->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmonitor.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = flowmon->GetFlowStats();

    auto iter = stats.begin(); // only pkts from A1 to C
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(iter->first);

    double simulationTime = (iter->second.timeLastRxPacket.GetSeconds() - iter->second.timeFirstTxPacket.GetSeconds());
    double throughput = ((iter->second.rxBytes * 8.0) / simulationTime) * 0.000001f; // Throughput in Mbps
    double delay = (iter->second.delaySum.GetSeconds() / iter->second.rxPackets); // Average delay in seconds
    double packetLoss = ((double)(iter->second.txPackets - iter->second.rxPackets) / iter->second.txPackets) * 100.0f; // Packet loss in Mbps


    std::cout << "Flow " << iter->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
    std::cout << "  Tx Bytes: " << iter->second.txBytes << "\n";
    std::cout << "  Rx Bytes: " << iter->second.rxBytes << "\n";
    std::cout << "  Load: " << load << " Mbps\n";
    std::cout << "  Throughput: " << throughput << " Mbps\n";
    std::cout << "  Average Delay: " << delay << " s\n";
    std::cout << "  Packet Loss: " << std::setprecision(9) << packetLoss << "%\n";
    outFile << iter->first << "," << load << "," << throughput << "," << delay << "," << packetLoss << "\n";

    Simulator::Destroy();
}

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    std::cout << "Running Part B simulations..." << std::endl;
    std::vector<std::string> tcpVariants = {"TcpNewReno", "TcpVegas"};
    std::ofstream outFile("b_metrics.csv");
    outFile << "Flow,Load (Mbps),Throughput (Mbps),Delay (s), Packet Loss (%)\n";

    for (const auto& variant : tcpVariants) {
        std::cout << "\nTCP Variant: " << variant << std::endl;
        for (double load = 10.0; load <= 105.0; load += 10.0) {
            std::cout << "\nLoad: " << load << " Mbps" << std::endl;
            RunSimulation(load, variant, outFile);
        }
    }

    outFile.close();

    return 0;
}
