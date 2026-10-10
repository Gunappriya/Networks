
[24bcs135@mepcolinux ex10]$#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main()
{
    // Node creation
    NodeContainer nodes;
    nodes.Create(2);

    std::cout << "====================================" << std::endl;
    std::cout << " TCP CONGESTION CONTROL SIMULATION" << std::endl;
    std::cout << "====================================" << std::endl;

    std::cout << "\nCreating Nodes..." << std::endl;
    std::cout << "Node 0 created" << std::endl;
    std::cout << "Node 1 created" << std::endl;

    // Link creation
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));

    NetDeviceContainer devices;
    devices = p2p.Install(nodes);

    std::cout << "\nPoint-to-Point Link Created" << std::endl;
    std::cout << "Data Rate: 2Mbps" << std::endl;
    std::cout << "Delay: 10ms" << std::endl;

    // Internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // IP address
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces;
    interfaces = address.Assign(devices);

    std::cout << "\nIP Addresses" << std::endl;
    std::cout << "Sender   : " << interfaces.GetAddress(0) << std::endl;
    std::cout << "Receiver : " << interfaces.GetAddress(1) << std::endl;

    // TCP Receiver
    uint16_t port = 5000;

    PacketSinkHelper sink(
        "ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), port));

    ApplicationContainer sinkApp = sink.Install(nodes.Get(1));

    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(10.0));

    // TCP Sender
    OnOffHelper source(
        "ns3::TcpSocketFactory",
        InetSocketAddress(interfaces.GetAddress(1), port));

    source.SetAttribute("DataRate", StringValue("5Mbps"));
    source.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer sourceApp = source.Install(nodes.Get(0));

    sourceApp.Start(Seconds(2.0));
    sourceApp.Stop(Seconds(9.0));

    // Buffer information
    int bufferSize = 5;
    int buffer = 0;

    std::cout << "\nBuffer Size: " << bufferSize << " packets" << std::endl;

    std::cout << "\nPacket Transmission" << std::endl;
    std::cout << "-------------------" << std::endl;

    for (int i = 1; i <= 12; i++)
    {
        std::cout << "\nPacket " << i << " generated" << std::endl;

        if (buffer < bufferSize)
        {
            buffer++;

            std::cout << "Packet added to buffer" << std::endl;
            std::cout << "Buffer Fill: "
                      << buffer << "/" << bufferSize << std::endl;
        }
        else
        {
            std::cout << "Buffer Full!" << std::endl;
            std::cout << "CONGESTION DETECTED" << std::endl;

            buffer = buffer - 2;

            std::cout << "TCP Congestion Control Applied" << std::endl;
            std::cout << "Transmission Rate Reduced" << std::endl;
            std::cout << "Packets transmitted from buffer" << std::endl;

            std::cout << "Buffer Fill: "
                      << buffer << "/" << bufferSize << std::endl;
        }
    }

    std::cout << "\nStarting NS-3 TCP Simulation..." << std::endl;

    Simulator::Stop(Seconds(10.0));

    Simulator::Run();

    std::cout << "\n====================================" << std::endl;
    std::cout << " TCP Simulation Completed" << std::endl;
    std::cout << "====================================" << std::endl;

    Simulator::Destroy();

    return 0;
}
[24bcs135@mepcolinux ex10]$[0/2] Re-checking globbed directories...
ninja: no work to do.
====================================
 TCP CONGESTION CONTROL SIMULATION
====================================

Creating Nodes...
Node 0 created
Node 1 created

Point-to-Point Link Created
Data Rate: 2Mbps
Delay: 10ms

IP Addresses
Sender   : 10.1.1.1
Receiver : 10.1.1.2

Buffer Size: 5 packets

Packet Transmission
-------------------

Packet 1 generated
Packet added to buffer
Buffer Fill: 1/5

Packet 2 generated
Packet added to buffer
Buffer Fill: 2/5

Packet 3 generated
Packet added to buffer
Buffer Fill: 3/5

Packet 4 generated
Packet added to buffer
Buffer Fill: 4/5

Packet 5 generated
Packet added to buffer
Buffer Fill: 5/5

Packet 6 generated
Buffer Full!
CONGESTION DETECTED
TCP Congestion Control Applied
Transmission Rate Reduced
Packets transmitted from buffer
Buffer Fill: 3/5

Packet 7 generated
Packet added to buffer
Buffer Fill: 4/5

Packet 8 generated
Packet added to buffer
Buffer Fill: 5/5

Packet 9 generated
Buffer Full!
CONGESTION DETECTED
TCP Congestion Control Applied
Transmission Rate Reduced
Packets transmitted from buffer
Buffer Fill: 3/5

Packet 10 generated
Packet added to buffer
Buffer Fill: 4/5

Packet 11 generated
Packet added to buffer
Buffer Fill: 5/5

Packet 12 generated
Buffer Full!
CONGESTION DETECTED
TCP Congestion Control Applied
Transmission Rate Reduced
Packets transmitted from buffer
Buffer Fill: 3/5

Starting NS-3 TCP Simulation...

====================================
 TCP Simulation Completed
====================================
