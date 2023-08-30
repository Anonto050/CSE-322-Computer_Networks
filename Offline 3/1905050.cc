/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "tutorial-app.h"

#include "ns3/applications-module.h"
#include "ns3/callback.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/stats-module.h"

#include <fstream>
#include <iostream>
#include <string>
#include <time.h>

using namespace ns3;

// ===========================================================================
//
//            s0----          -----r0
//  senders - s1--- rt0 --- rt1---r1 - receivers
//
//                 1 Gbps, 100 ms
//
// ===========================================================================

#define VARY_BOOTTLENECK 1
#define VARY_PACKET_LOSS 2

// Ptr<OutputStreamWrapper> outputStream;
Ptr<OutputStreamWrapper> outputStream2;
int decision = 0;

/**
 * Congestion window change callback
 *
 * \param oldCwnd Old congestion window.
 * \param newCwnd New congestion window.
 */
static void
CwndChange(Ptr<OutputStreamWrapper> outputStream, uint32_t oldCwnd, uint32_t newCwnd)
{
    // NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd);
    // if (decision == 0)
    *outputStream->GetStream() << Simulator::Now().GetSeconds() << "\t" << newCwnd << std::endl;
    // else
    //     *outputStream2->GetStream()
    //         << Simulator::Now().GetSeconds() << "\t" << newCwnd << std::endl;
}

int
main(int argc, char* argv[])
{
    uint32_t port = 8080;
    uint32_t payloadsize = 1024;

    uint32_t NUM_NODE = 2;
    uint32_t NUM_FLOW = 2;
    int option = VARY_BOOTTLENECK;

    double simulationTime = 45;
    double perHopDelay = 2;

    std ::string nodeDataRate = "1Gbps";
    std ::string nodeDelay = "1ms";
    int bottleneckDataRate = 50;
    int bottleneckDelay = 100;

    int power_PacketLoss = -6;

    std ::string congControlAlgo1 = "TcpNewReno";
    std ::string congControlAlgo2 = "TcpHighSpeed";

    AsciiTraceHelper asciiTraceHelper;
    // outputStream = asciiTraceHelper.CreateFileStream("1905050.dat");

    CommandLine cmd(__FILE__);
    cmd.AddValue("congControlAlgo2", "Congestion control algorithm for flow 2", congControlAlgo2);
    cmd.AddValue("bottleneckDataRate",
                 "Packets allowed in the queue thus bottleneck",
                 bottleneckDataRate);
    cmd.AddValue("power_PacketLoss", "Packet loss rate", power_PacketLoss);
    cmd.AddValue("option", "Vary bottleneck or packet loss", option);
    cmd.Parse(argc, argv);

    double packetLossRate = std ::pow(10, power_PacketLoss);
    std ::string bottleneckDataRateString = std ::to_string(bottleneckDataRate) + "Gbps";
    std ::string bottleneckDelayString = std ::to_string(bottleneckDelay) + "ms";

    double bandwidthDelayProduct = (bottleneckDataRate * bottleneckDelay) / payloadsize;

    // In the following three lines, TCP NewReno is used as the congestion
    // control algorithm, the initial congestion window of a TCP connection is
    // set to 1 packet, and the classic fast recovery algorithm is used. Note
    // that this configuration is used only to demonstrate how TCP parameters
    // can be configured in ns-3. Otherwise, it is recommended to use the default
    // settings of TCP in ns-3.
    // Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    // Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));
    // Config::SetDefault("ns3::TcpL4Protocol::RecoveryType",
    //                    TypeIdValue(TypeId::LookupByName("ns3::TcpClassicRecovery")));

    Config ::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadsize));

    NodeContainer Nodes;
    Nodes.Create(2);

    PointToPointHelper channel;
    channel.SetDeviceAttribute("DataRate", StringValue(bottleneckDataRateString));
    channel.SetChannelAttribute("Delay", StringValue(bottleneckDelayString));

    PointToPointHelper nodes;
    nodes.SetDeviceAttribute("DataRate", StringValue(nodeDataRate));
    nodes.SetChannelAttribute("Delay", StringValue(nodeDelay));
    nodes.SetQueue("ns3::DropTailQueue",
                   "MaxSize",
                   StringValue(std ::to_string(bandwidthDelayProduct) + "p"));

    PointToPointDumbbellHelper ppDumble(NUM_NODE, nodes, NUM_NODE, nodes, channel);

    Ptr<RateErrorModel> errorModel = CreateObject<RateErrorModel>();
    errorModel->SetAttribute("ErrorRate", DoubleValue(packetLossRate));
    ppDumble.m_routerDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel));

    // install flow 1

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + congControlAlgo1));
    InternetStackHelper stack_flow1;
    stack_flow1.Install(ppDumble.GetLeft(0));
    stack_flow1.Install(ppDumble.GetRight(0));

    // print left and right count
    // std::cout << ppDumble.LeftCount() << std::endl;
    // std::cout << ppDumble.RightCount() << std::endl;

    stack_flow1.Install(ppDumble.GetLeft());
    stack_flow1.Install(ppDumble.GetRight());

    // install flow 2
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + congControlAlgo2));
    InternetStackHelper stack_flow2;
    stack_flow2.Install(ppDumble.GetLeft(1));
    stack_flow2.Install(ppDumble.GetRight(1));

    ppDumble.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"),
                                 Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),
                                 Ipv4AddressHelper("10.3.1.0", "255.255.255.0"));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Install flow monitor
    FlowMonitorHelper measure_flow;
    measure_flow.SetMonitorAttribute("MaxPerHopDelay", TimeValue(Seconds(perHopDelay)));
    // measure_flow.SetMonitorAttribute("JitterBinWidth", TimeValue(Seconds(0.001)));
    // measure_flow.SetMonitorAttribute("MaxPacketSize", UintegerValue(1472));

    Ptr<FlowMonitor> monitor = measure_flow.InstallAll();

    // Install applications on first flow

    Address receiverAddress(InetSocketAddress(ppDumble.GetRightIpv4Address(0), port));
    PacketSinkHelper receiverHelper("ns3::TcpSocketFactory",
                                    InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer receiverApp = receiverHelper.Install(ppDumble.GetRight(0));
    receiverApp.Start(Seconds(0.0));
    receiverApp.Stop(Seconds(simulationTime + perHopDelay));

    Ptr<Socket> socket_dumble =
        Socket::CreateSocket(ppDumble.GetLeft(0), TcpSocketFactory::GetTypeId());
    Ptr<TutorialApp> app = CreateObject<TutorialApp>();
    app->Setup(socket_dumble,
               receiverAddress,
               payloadsize,
               10000,
               DataRate(nodeDataRate),
               simulationTime);

    ppDumble.GetLeft(0)->AddApplication(app);
    app->SetStartTime(Seconds(2));
    app->SetStopTime(Seconds(simulationTime));

    // outputStream = asciiTraceHelper.CreateFileStream("Flow1.cwnd");
    // socket_dumble->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));
    if (decision == 0)
    {
        std ::ostringstream oss;
        oss << "Flow1.cwnd";
        Ptr<OutputStreamWrapper> outputStream = asciiTraceHelper.CreateFileStream(oss.str());
        socket_dumble->TraceConnectWithoutContext("CongestionWindow",
                                                  MakeBoundCallback(&CwndChange, outputStream));
        decision = 1;
    }

    // Install applications on second flow
    Address receiver2Address(InetSocketAddress(ppDumble.GetRightIpv4Address(1), port));
    PacketSinkHelper receiver2Helper("ns3::TcpSocketFactory",
                                     InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer receiver2App = receiver2Helper.Install(ppDumble.GetRight(1));
    receiver2App.Start(Seconds(0.0));
    receiver2App.Stop(Seconds(simulationTime + perHopDelay));

    Ptr<Socket> socket_dumble2 =
        Socket::CreateSocket(ppDumble.GetLeft(1), TcpSocketFactory::GetTypeId());
    Ptr<TutorialApp> app2 = CreateObject<TutorialApp>();
    app2->Setup(socket_dumble2,
                receiver2Address,
                payloadsize,
                10000,
                DataRate(nodeDataRate),
                simulationTime);

    ppDumble.GetLeft(1)->AddApplication(app2);
    app2->SetStartTime(Seconds(2));
    app2->SetStopTime(Seconds(simulationTime));
    decision = 1;

    // outputStream2 = asciiTraceHelper.CreateFileStream("Flow2.cwnd");
    // socket_dumble2->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));
    if (decision == 1)
    {
        std ::ostringstream oss;
        oss << "Flow2.cwnd";
        Ptr<OutputStreamWrapper> outputStream = asciiTraceHelper.CreateFileStream(oss.str());
        socket_dumble2->TraceConnectWithoutContext("CongestionWindow",
                                                   MakeBoundCallback(&CwndChange, outputStream));
        decision = 0;
    }

    Simulator::Stop(Seconds(simulationTime + perHopDelay));
    Simulator::Run();

    // Flow monitor output

    // open a text file in append mode.
    std::ofstream file_output;
    file_output.open("1905050.txt", std::ios_base::app);

    int flow_count = 0;
    double throughput_flow1 = 0;
    double throughput_flow2 = 0;

    double numerator_jain_index = 0;
    double denominator_jain_index = 0;

    int packets_sent = 0;
    int packets_received = 0;
    int packets_lost = 0;

    Ptr<Ipv4FlowClassifier> flow_classifier =
        DynamicCast<Ipv4FlowClassifier>(measure_flow.GetClassifier());
    FlowMonitor::FlowStatsContainer flow_stats = monitor->GetFlowStats();

    // Iterate over these flow stats to calculate the total throughput.
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iteration = flow_stats.begin();
         iteration != flow_stats.end();
         ++iteration)
    {
        Ipv4FlowClassifier::FiveTuple fiveTuple = flow_classifier->FindFlow(iteration->first);

        if (flow_count % 2 == 0)
        {
            throughput_flow1 +=
                iteration->second.rxBytes * 8.0 / ((simulationTime + perHopDelay) * 1000);
            packets_sent += iteration->second.txPackets;
            packets_received += iteration->second.rxPackets;
            packets_lost += iteration->second.lostPackets;
            numerator_jain_index +=
                iteration->second.rxBytes * 8.0 / ((simulationTime + perHopDelay) * 1000);
            denominator_jain_index +=
                (iteration->second.rxBytes * 8.0 / ((simulationTime + perHopDelay) * 1000)) *
                (iteration->second.rxBytes * 8.0 / ((simulationTime + perHopDelay) * 1000));
        }
        else
        {
            throughput_flow2 += iteration->second.rxBytes * 8.0 / (simulationTime * 1000000.0);
            packets_sent += iteration->second.txPackets;
            packets_received += iteration->second.rxPackets;
            packets_lost += iteration->second.lostPackets;
            numerator_jain_index +=
                iteration->second.rxBytes * 8.0 / ((simulationTime + perHopDelay) * 1000);
            denominator_jain_index +=
                (iteration->second.rxBytes * 8.0 / ((simulationTime + perHopDelay) * 1000)) *
                (iteration->second.rxBytes * 8.0 / ((simulationTime + perHopDelay) * 1000));
        }

        flow_count++;
    }

    double jain_index =
        (numerator_jain_index * numerator_jain_index) / (flow_count * denominator_jain_index);

    // columns of the output file - bottleneckDataRate, packetLossRate, throughput_flow1,
    // throughput_flow2, jain_index, packets_sent, packets_received, packets_lost
    file_output << bottleneckDataRate << "\t" << packetLossRate << "\t" << throughput_flow1 << "\t"
                << throughput_flow2 << "\t" << jain_index << "\t" << packets_sent << "\t"
                << packets_received << "\t" << packets_lost << std::endl;

    Simulator::Destroy();

    return 0;
}
