/*
 *
 * Network topology:
 * 
 * S0                                            R0
 * 
 * S1                                            R1
 *
 * S2-------------R1-------------R2--------------R2
 * 
 * S3                                            R3
 * 
 * S4                                            R4

 * Calculate throughput for this network and plot a throughput vs time graph
 */
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/stats-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/csma-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"

//#include "ns3/range-propagation-loss-model.h"


#include <fstream>
#include <iostream>
#include <string>

NS_LOG_COMPONENT_DEFINE("wifi");

using namespace ns3;

std :: string offline_name = "wifi";

uint32_t totalReceivedBits = 0;
uint32_t totalReceivedPackets = 0;
std ::vector<uint32_t> totalSentPackets;

void
PacketSinkRxTrace(Ptr<const Packet> p, const Address& addr)
{
    totalReceivedBits += p->GetSize() * 8;
    totalReceivedPackets++;
    //std ::cout << "Received " << totalReceivedPackets << " packets" << std ::endl;
}

void
PacketSourceTxTrace(Ptr<const Packet> p)
{
    uint32_t nodeId = Simulator::GetContext();
    totalSentPackets[nodeId]++;
    //std ::cout << "Node " << nodeId << " sent " << totalSentPackets[nodeId] << " packets" << std ::endl;
}


int
main(int argc, char* argv[])
{
    uint32_t port = 9;                     /* Discard port number. */
    uint32_t payloadSize = 1024;           /* Transport layer payload size in bytes. */
    //uint32_t maxPacketCount = 100;        /* Number of packets to transmit. */

    std::string dataRate = "5Mbps";      /* Application layer datarate. */
    std::string channelDelay = "5ms";     /* Channel delay. */

    double simulationTime = 10;            /* Simulation time in seconds. */
    
    // AsciiTraceHelper asciiTraceHelper;
    // stream  = asciiTraceHelper.CreateFileStream("practice.dat");

    // Parameters for the simulation
    uint32_t NUM_WIFI = 20;  // number of wifi nodes - 20, 40, 60, 80, 100
    int32_t NUM_FLOW = 10;   // number of flows - 10, 20, 30, 40, 50
    int32_t NUM_PACKET = 100; // number of packets - 100, 200, 300, 400, 500
    //int32_t NUM_SPEED = 5;   // speeds - 5, 10, 15, 20, 25
    int32_t COVERAGE_AREA = 2; // coverage area - 1, 2, 3, 4, 5 * tx_range

    int32_t Tx_range = 5;
    

    int32_t rate = (payloadSize * NUM_PACKET * 8)/ 1000;
    std :: string APP_DATA_RATE = std :: to_string(rate) + "Kbps";
    
    std :: cout << APP_DATA_RATE << std :: endl;

    

    // for sh file
    offline_name += "-nwifi" + std :: to_string(NUM_WIFI);
    offline_name += "-nflow" + std :: to_string(NUM_FLOW);
    offline_name += "-npacket" + std :: to_string(NUM_PACKET);
    //offline_name += "-nspeed" + std :: to_string(NUM_SPEED);   
    offline_name += "-cov" + std :: to_string(COVERAGE_AREA);


    //std :: cout << offline_name << std :: endl;


    // changing error_p for increasing error rate in error model - drop more packets 
    // double error_p = 0.0001;
    /* Command line argument parser setup. */

    CommandLine cmd(__FILE__);
    cmd.AddValue("NUM_WIFI", "Number of wifi nodes", NUM_WIFI);
    cmd.AddValue("NUM_FLOW", "Number of flows", NUM_FLOW);
    cmd.AddValue("NUM_PACKET", "Number of packets", NUM_PACKET);
    //cmd.AddValue("NUM_SPEED", "Number of speeds", NUM_SPEED);
    cmd.AddValue("COVERAGE_AREA", "Coverage area", COVERAGE_AREA);
  

    cmd.Parse(argc, argv);


    /* Configure TCP Options */
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));
 

    // Create gateways, sources, and sinks

    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    PointToPointHelper p2pLink;
    p2pLink.SetDeviceAttribute("DataRate", StringValue(dataRate));
    p2pLink.SetChannelAttribute("Delay", StringValue(channelDelay));

    NetDeviceContainer p2pDevices;
    p2pDevices = p2pLink.Install(p2pNodes);


   // Create wifi nodes
    
    NodeContainer wifiSenders;
    wifiSenders.Create(NUM_WIFI);

    NodeContainer wifiReceivers;
    wifiReceivers.Create(NUM_WIFI);

    NodeContainer wifiApNodeSenders = p2pNodes.Get(0);
    NodeContainer wifiApNodeReceivers = p2pNodes.Get(1);

    // Create wifi channel

    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(COVERAGE_AREA * Tx_range));

    YansWifiPhyHelper phySender;
    phySender.SetChannel(wifiChannel.Create());

    YansWifiPhyHelper phyReceiver;
    phyReceiver.SetChannel(wifiChannel.Create());

    // Create wifi mac
    WifiHelper wifiHelper;

    WifiMacHelper macSender, macReceiver;
    Ssid ssidSender = Ssid("ns3-ssid-sender");
    Ssid ssidReceiver = Ssid("ns3-ssid-receiver");

    // Configure wifi mac
    macSender.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssidSender), "ActiveProbing", BooleanValue(false));
    macReceiver.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssidReceiver), "ActiveProbing", BooleanValue(false));

    NetDeviceContainer wifiDevicesSender, wifiDevicesReceiver;
    wifiDevicesSender = wifiHelper.Install(phySender, macSender, wifiSenders);
    wifiDevicesReceiver = wifiHelper.Install(phyReceiver, macReceiver, wifiReceivers);

    // Configure wifi ap
    macSender.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssidSender));
    macReceiver.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssidReceiver));

    NetDeviceContainer wifiApDevicesSender, wifiApDevicesReceiver;
    wifiApDevicesSender = wifiHelper.Install(phySender, macSender, wifiApNodeSenders);
    wifiApDevicesReceiver = wifiHelper.Install(phyReceiver, macReceiver, wifiApNodeReceivers);

    // Configure mobility
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(0.5),
                                  "DeltaY", DoubleValue(1.0),
                                  "GridWidth", UintegerValue(5),
                                  "LayoutType", StringValue("RowFirst"));
    
    //mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)), "Speed", StringValue("ns3::ConstantRandomVariable[Constant="+std :: to_string(NUM_SPEED)+"]"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // install mobility on wifi nodes
    mobility.Install(wifiSenders);
    mobility.Install(wifiReceivers);

    // install constant mobility on wifi ap nodes
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodeSenders);
    mobility.Install(wifiApNodeReceivers);

    // install internet stack on wifi nodes
    InternetStackHelper stack;
    stack.Install(wifiApNodeSenders);
    stack.Install(wifiApNodeReceivers);
    stack.Install(wifiSenders);
    stack.Install(wifiReceivers);

    // assign ip address to wifi nodes
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    Ipv4InterfaceContainer wifiInterfacesSender, wifiInterfacesReceiver;
    Ipv4InterfaceContainer wifiApInterfacesSender, wifiApInterfacesReceiver;

    address.SetBase("10.1.2.0", "255.255.255.0");
    wifiInterfacesSender = address.Assign(wifiDevicesSender);
    wifiApInterfacesSender = address.Assign(wifiApDevicesSender);


    address.SetBase("10.1.3.0", "255.255.255.0");
    wifiInterfacesReceiver = address.Assign(wifiDevicesReceiver);
    wifiApInterfacesReceiver = address.Assign(wifiApDevicesReceiver);

    // install onoff application on senders
    OnOffHelper onoff("ns3::TcpSocketFactory", Address());
    onoff.SetAttribute("PacketSize", UintegerValue(payloadSize));
    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
    onoff.SetAttribute("DataRate", DataRateValue(DataRate(APP_DATA_RATE)));

    // install packet sink on receivers
    Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);

    ApplicationContainer receiverApps;
    ApplicationContainer senderApps;

    for (uint32_t i = 0; i < NUM_WIFI; i++)
    {
        receiverApps.Add(sinkHelper.Install(wifiReceivers.Get(i)));

        Ptr<PacketSink> sink = DynamicCast<PacketSink>(receiverApps.Get(i));
        sink->TraceConnectWithoutContext("Rx", MakeCallback(&PacketSinkRxTrace));
    }

    receiverApps.Start(Seconds(1.0));
    receiverApps.Stop(Seconds(simulationTime));

    // install onoff application on senders
    int32_t flow_count = 0;

    for(uint32_t i = 0; i < NUM_WIFI; i++){

        AddressValue remoteAddress(InetSocketAddress(wifiInterfacesReceiver.GetAddress(i), port));

        for(uint32_t j = 0; j < NUM_WIFI; j++){

            onoff.SetAttribute("Remote", remoteAddress);
            senderApps.Add(onoff.Install(wifiSenders.Get(j)));
            flow_count++;

            if(flow_count >= NUM_FLOW){
                break;
            }
        }

        if(flow_count >= NUM_FLOW){
            break;
        }
    }

    // iterate through all sender apps and connect trace
    for (uint32_t i = 0; i < senderApps.GetN(); i++)
    {
        Ptr<OnOffApplication> onoffApp = DynamicCast<OnOffApplication>(senderApps.Get(i));
        onoffApp->TraceConnectWithoutContext("Tx", MakeCallback(&PacketSourceTxTrace));
    }


    senderApps.Start(Seconds(2.0));
    senderApps.Stop(Seconds(simulationTime-1));

    // Routing table
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Stop(Seconds(simulationTime));

    Simulator::Run();

    // Calculate throughput using received bits and simulation time
    double avgThroughput = totalReceivedBits / (simulationTime * 1e6); // Mbps

    // Calculate packet delivery ratio
    uint32_t totalSentPacketsCount = 0;

    for (uint32_t i = 0; i < NUM_WIFI; i++)
    {
        totalSentPacketsCount += totalSentPackets[i];
    }

    double packetDeliveryRatio = static_cast<double>(totalReceivedPackets) / totalSentPacketsCount;

    // Open the output file in append mode
    std::ofstream outputStream("output.dat", std::ios_base::app);

    // Check if the file was successfully opened
    if (!outputStream.is_open())
    {
        std::cerr << "Failed to open output file!" << std::endl;
        return 1; // Return an error code
    }


    // Print or store the results
    std::cout << "Average Throughput: " << avgThroughput << " Mbps" << std::endl;
    std::cout << "Packet Delivery Ratio: " << packetDeliveryRatio << std::endl;

    // Append data to the output file
    outputStream << avgThroughput << " " << packetDeliveryRatio << " " << NUM_WIFI << " " << NUM_FLOW << " " << NUM_PACKET << " " << COVERAGE_AREA << std::endl;

    // Close the output file
    outputStream.close();

    Simulator::Destroy();

    return 0;
}
