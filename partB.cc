#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

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

void CalculateStats() {
    double throughput = (g_totalBytesReceived * 8.0) / (g_lastRxTime.GetSeconds() * 1000000.0);
    double avgDelay = 0;
    if (!g_delays.empty()) {
        avgDelay = std::accumulate(g_delays.begin(), g_delays.end(), 0.0) / g_delays.size();
    }

    std::cout << "Results:" << std::endl;
    std::cout << "Throughput: " << throughput << " Mbps" << std::endl;
    std::cout << "Average Delay: " << avgDelay << " ms" << std::endl;
    std::cout << "Total Bytes Received: " << g_totalBytesReceived << std::endl;

    // Reset for next run
    g_totalBytesReceived = 0;
    g_delays.clear();
}

void RunSimulation(double load, std::string tcpVariant) {
    // Create nodes
    NodeContainer lanNodes;
    lanNodes.Create(3);  // A1, A2, A3
    NodeContainer routerNode;
    routerNode.Create(1);  // Router B
    NodeContainer destNode;
    destNode.Create(1);  // Node C

    // Create LAN
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", StringValue("1ms"));

    NetDeviceContainer lanDevices = csma.Install(lanNodes);

    // Create point-to-point links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer routerDevices = p2p.Install(lanNodes.Get(2), routerNode.Get(0));
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
    Ipv4InterfaceContainer lanInterfaces = ipv4.Assign(lanDevices);

    ipv4.SetBase("192.168.3.0", "255.255.255.0");
    Ipv4InterfaceContainer routerInterfaces = ipv4.Assign(routerDevices);
    Ipv4InterfaceContainer destInterfaces = ipv4.Assign(destDevices);

    // Enable routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create TCP application
    uint16_t port = 8080;
    PacketSinkHelper sink("ns3::TcpSocketFactory",
                         InetSocketAddress(destInterfaces.GetAddress(1), port));
    ApplicationContainer sinkApp = sink.Install(destNode.Get(0));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(30.0));

    // Configure TCP sender
    BulkSendHelper source("ns3::TcpSocketFactory",
                         InetSocketAddress(destInterfaces.GetAddress(1), port));
    source.SetAttribute("MaxBytes", UintegerValue(load * 1000000));
    ApplicationContainer sourceApp = source.Install(lanNodes.Get(0));  // A1
    sourceApp.Start(Seconds(0.0));
    sourceApp.Stop(Seconds(30.0));

    // Add tracing
    Config::ConnectWithoutContext(
        "/NodeList/" + std::to_string(destNode.Get(0)->GetId()) +
        "/ApplicationList/*/$ns3::PacketSink/Rx",
        MakeCallback(&RxTrace));

    // Enable PCAP tracing
    csma.EnablePcapAll("part_b_lan");
    p2p.EnablePcapAll("part_b_p2p");

    // Run simulation
    Simulator::Stop(Seconds(30.0));
    Simulator::Run();
    Simulator::Destroy();

    // Calculate and print statistics
    CalculateStats();
}

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    std::cout << "Running Part B simulations..." << std::endl;
    std::vector<std::string> tcpVariants = {"TcpNewReno", "TcpVegas"};

    for (const auto& variant : tcpVariants) {
        std::cout << "\nTCP Variant: " << variant << std::endl;
        for (double load = 10.0; load <= 100.0; load += 10.0) {
            std::cout << "\nLoad: " << load << " Mbps" << std::endl;
            RunSimulation(load, variant);
        }
    }

    return 0;
}
