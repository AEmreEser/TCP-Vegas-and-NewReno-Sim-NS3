
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

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

void RunSimulation(double load) {
    // Create nodes
    NodeContainer nodes;
    nodes.Create(3);  // Node A, Router B, Node C

    // Create point-to-point links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("200Mbps")); // Set high enough to handle 100 Mbps load
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

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

    // Create TCP application
    uint16_t port = 8080;
    PacketSinkHelper sink("ns3::TcpSocketFactory",
                         InetSocketAddress(interfaces2.GetAddress(1), port));
    ApplicationContainer sinkApp = sink.Install(nodes.Get(2));  // Node C
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(30.0));

    // Configure TCP sender
    BulkSendHelper source("ns3::TcpSocketFactory",
                         InetSocketAddress(interfaces2.GetAddress(1), port));
    source.SetAttribute("MaxBytes", UintegerValue(load * 1000000));  // Convert to bytes
    ApplicationContainer sourceApp = source.Install(nodes.Get(0));  // Node A
    sourceApp.Start(Seconds(0.0));
    sourceApp.Stop(Seconds(30.0));

    // Add tracing
    Config::ConnectWithoutContext(
        "/NodeList/2/ApplicationList/*/$ns3::PacketSink/Rx",
        MakeCallback(&RxTrace));

    // Enable PCAP tracing
    p2p.EnablePcapAll("part_a");

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

    std::cout << "Running Part A simulations..." << std::endl;
    for (double load = 1.0; load <= 110.0; load += 10.0) { // Adjusted to simulate up to 100 Mbps
        std::cout << "\nLoad: " << load << " Mbps" << std::endl;
        RunSimulation(load);
    }

    return 0;
}
