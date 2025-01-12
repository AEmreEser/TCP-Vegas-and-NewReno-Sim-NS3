#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

#ifndef SIM_END
#define SIM_END 5.0
#endif

// #define USE_TCP  // uncomment this to use tcp instead of udp

NS_LOG_COMPONENT_DEFINE("PartA");

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

void RunSimulation(int load, std::ofstream & outFile) {
    // Create nodes
    NodeContainer nodes;
    nodes.Create(3);  // Node A, Router B, Node C

    // Create point-to-point links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps")); // Set high enough to handle 10 Mbps load
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("10p")); // fifo queue in every connection

    // Install devices
    NetDeviceContainer devices1 = p2p.Install(nodes.Get(0), nodes.Get(1));  // A to B
    NetDeviceContainer devices2 = p2p.Install(nodes.Get(1), nodes.Get(2));  // B to C

    // Install Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces1 = ipv4.Assign(devices1);

    ipv4.SetBase("192.168.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces2 = ipv4.Assign(devices2);

    // Enable routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create UDP application
    uint16_t port = 8080;

    #ifdef USE_TCP
    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(interfaces2.GetAddress(1), port));
    #else
    PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(interfaces2.GetAddress(1), port));
    #endif
    ApplicationContainer sinkApp = sink.Install(nodes.Get(2));  // Node C
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(SIM_END));

    // Configure UDP sender
    #ifdef USE_TCP
    OnOffHelper source("ns3::TcpSocketFactory", InetSocketAddress(interfaces2.GetAddress(1), port));
    #else
    OnOffHelper source("ns3::UdpSocketFactory", InetSocketAddress(interfaces2.GetAddress(1), port));
    #endif

    // source.SetAttribute("MaxBytes", UintegerValue(load * 1000000));  // Convert to bytes
    source.SetAttribute("DataRate", DataRateValue(DataRate(std::to_string(load) + "Mbps"))); // Set exact load
    source.SetAttribute("PacketSize", UintegerValue(1024)); // Packet size in bytes
    source.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")); // always working
    source.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    source.SetAttribute("StartTime", TimeValue(Seconds(0.0)));
    source.SetAttribute("StopTime", TimeValue(Seconds(SIM_END)));
    ApplicationContainer sourceApp = source.Install(nodes.Get(0));  // Node A
    sourceApp.Start(Seconds(0.0));
    sourceApp.Stop(Seconds(SIM_END));

    // Add tracing
    Config::ConnectWithoutContext(
        "/NodeList/2/ApplicationList/*/$ns3::PacketSink/Rx",
        MakeCallback(&RxTrace));

    // Enable PCAP tracing
    p2p.EnablePcapAll("part_a");
    
    FlowMonitorHelper flowmonitor;
    Ptr<FlowMonitor> flowmon = flowmonitor.InstallAll();

    // Run simulation
    Simulator::Stop(Seconds(SIM_END));
    Simulator::Run();

    flowmon->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmonitor.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = flowmon->GetFlowStats();

    auto iter = stats.begin(); // only pkts from 1.1 to 2.2
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(iter->first);

    double simulationTime = (iter->second.timeLastRxPacket.GetSeconds() - iter->second.timeFirstTxPacket.GetSeconds());
    double throughput = ((iter->second.rxBytes * 8.0) / simulationTime) * 0.000001f; // Throughput in Mbps
    double delay = (iter->second.delaySum.GetSeconds() / iter->second.rxPackets); // Average delay in seconds

    std::cout << "Flow " << iter->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
    std::cout << "  Tx Bytes: " << iter->second.txBytes << "\n";
    std::cout << "  Rx Bytes: " << iter->second.rxBytes << "\n";
    std::cout << "  Load: " << load << " Mbps\n";
    std::cout << "  Throughput: " << throughput << " Mbps\n";
    std::cout << "  Average Delay: " << delay << " s\n";
    outFile << iter->first << "," << load << "," << throughput << "," << delay << "\n";

    Simulator::Destroy();
}

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    std::ofstream outFile("a_metrics.csv");
    outFile << "Flow,Load (Mbps),Throughput (Mbps),Delay (s)\n";

    std::cout << "Running Part A simulations..." << std::endl;
    for (int load = 1; load <= 10; load += 1) { // Adjusted to simulate up to 10 Mbps
        std::cout << "\nLoad: " << load << " Mbps" << std::endl;
        RunSimulation(load, outFile);
    }

    outFile.close();

    return 0;
}
