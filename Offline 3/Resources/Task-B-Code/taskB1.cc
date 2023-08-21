
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include "ns3/callback.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/csma-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TaskB1");

// ===========================================================================
//
//            t0----      -----h0
//  senders - t1---r0 --- r1---h1 - receivers
//            t2----      -----h2
//
// ===========================================================================
//
class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t simultime);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
  uint32_t        m_simultime;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0),
    m_simultime (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

/* static */
TypeId MyApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyApp")
    .SetParent<Application> ()
    .SetGroupName ("Tutorial")
    .AddConstructor<MyApp> ()
    ;
  return tid;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t simultime)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_dataRate = dataRate;
  m_simultime = simultime;
  // NS_LOG_UNCOND("Setting up :"<<socket<<"; address : "<<address<<" ; packetSize : "<<packetSize<<" ; nPackets : "<<nPackets<<" ; dataRate : "<<dataRate);
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
    if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if(Simulator::Now().GetSeconds() < m_simultime) ScheduleTx();
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  // NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " " << newCwnd);
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd << std::endl;
}



int main(int argc, char *argv[]){
    uint32_t payloadSize = 1472;
    std::string tcpVariant1 = "ns3::TcpNewReno"; // TcpNewReno
    std::string tcpVariant2 = "ns3::TcpHighSpeed"; //TcpAdaptiveReno TcpWestwood, TcpHighSpeed
    int nLeaf = 2;
    int nFlows = 2;
    std::string senderDataRate = "1Gbps";
    std::string senderDelay = "1ms";
    std::string output_folder = "taskB/tcphs";
    int simulationTimeInSeconds = 60;
    int cleanupTime = 2;
    int bttlnkRate = 50;
    int bttlnkDelay = 100;
    int packet_loss_exp = 6;
    int exp = 1;

    // input from CMD
    CommandLine cmd (__FILE__);
    cmd.AddValue ("tcp2","Name of TCP variant 2", tcpVariant2);
    cmd.AddValue ("nLeaf","Number of left and right side leaf nodes", nLeaf);
    cmd.AddValue ("bttlnkRate","Max Packets allowed in the device queue", bttlnkRate);
    cmd.AddValue ("plossRate", "Packet loss rate", packet_loss_exp);
    cmd.AddValue ("output_folder","Folder to store dara", output_folder);
    cmd.AddValue ("exp","1 for bttlnck, 2 for packet loss rate", exp);
    cmd.Parse (argc,argv);

    std::string file = output_folder + "/data_" + std::to_string(exp) + ".txt";
    nFlows = nLeaf;
    double packet_loss_rate = (1.0 / std::pow(10, packet_loss_exp));
    std::string bottleNeckDataRate = std::to_string(bttlnkRate) + "Mbps";
    std::string bottleNeckDelay = std::to_string(bttlnkDelay) + "ms";
    // senderDataRate = std::to_string(bttlnkRate*4) + "Mbps";

    NS_LOG_UNCOND("USING TCP 1 = "<<tcpVariant1<<" ; TCP 2 = "<<tcpVariant2<<" ; nLeaf = "<<nLeaf<<
                  " ; bottleneck rate = "<<bottleNeckDataRate<<
                  " ; packet loss rate = "<<packet_loss_rate<<
                  " ; sender data rate = "<<senderDataRate);

    // srand(time(0));

    // LogComponentEnable("TcpAdaptiveReno", LOG_LOGIC);

    // Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpVariant1));
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));

    // SETUP NODE AND DEVICE
    // Create the point-to-point link helpers
    PointToPointHelper bottleNeckLink;
    // CsmaHelper bottleNeckLink;

    bottleNeckLink.SetDeviceAttribute  ("DataRate", StringValue (bottleNeckDataRate));
    bottleNeckLink.SetChannelAttribute ("Delay", StringValue (bottleNeckDelay));


    PointToPointHelper pointToPointLeaf;
    // CsmaHelper pointToPointLeaf;
    pointToPointLeaf.SetDeviceAttribute  ("DataRate", StringValue (senderDataRate));
    pointToPointLeaf.SetChannelAttribute ("Delay", StringValue (senderDelay));
    // add router buffer capacity - equal to bandwidth delay product
    pointToPointLeaf.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue (std::to_string (bttlnkDelay * bttlnkRate) + "p"));

    PointToPointDumbbellHelper d (nLeaf, pointToPointLeaf,
                                  nLeaf, pointToPointLeaf,
                                  bottleNeckLink);

    // add error rate
    // need device container but its private: made public graph becomes weird
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (packet_loss_rate));
    // set receive error for bottleneck -> 0 dont work
    d.m_routerDevices.Get(1)->SetAttribute ("ReceiveErrorModel", PointerValue (em)); 

    // for (uint32_t i = 0; i < d.LeftCount (); ++i)
    //   {
    //     d.m_leftLeafDevices.Get (i)->SetAttribute ("ReceiveErrorModel", PointerValue (em)); // left leaves
    //     d.m_rightLeafDevices.Get (i)->SetAttribute ("ReceiveErrorModel", PointerValue (em)); // right leaves
    //   }

    // INSTALL STACK
    // tcp variant 1
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpVariant1));
    InternetStackHelper stack1;
    for (uint32_t i = 0; i < d.LeftCount (); i+=2)
      {
        stack1.Install (d.GetLeft (i)); // left leaves
      }
    for (uint32_t i = 0; i < d.RightCount (); i+=2)
      {
        stack1.Install (d.GetRight (i)); // right leaves
      }
    stack1.Install (d.GetLeft ());
    stack1.Install (d.GetRight ());

    // tcp variant 2
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpVariant2));
    InternetStackHelper stack2;
    for (uint32_t i = 1; i < d.LeftCount (); i+=2)
      {
        stack2.Install (d.GetLeft (i)); // left leaves
      }
    for (uint32_t i = 1; i < d.RightCount (); i+=2)
      {
        stack2.Install (d.GetRight (i)); // right leaves
      }

    // ASSIGN IP Addresses
    d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"), // left nodes
                          Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),  // right nodes
                          Ipv4AddressHelper ("10.3.1.0", "255.255.255.0")); // routers 
    Ipv4GlobalRoutingHelper::PopulateRoutingTables (); // populate routing table
    

    // install flow monitor
    FlowMonitorHelper flowmon;
    flowmon.SetMonitorAttribute("MaxPerHopDelay", TimeValue(Seconds(cleanupTime)));
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    uint16_t sp = 8080;
    for(int i=0;i<nFlows; i++){

      // if(i%2 == 0) Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpVariant1));
      // if(i%2 == 1) Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpVariant2));


      Address sinkAddress (InetSocketAddress (d.GetRightIpv4Address (i), sp));
      PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), sp));
      ApplicationContainer sinkApps = packetSinkHelper.Install (d.GetRight (i));
      sinkApps.Start (Seconds (0));
      sinkApps.Stop (Seconds (simulationTimeInSeconds+cleanupTime));

      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (d.GetLeft (i), TcpSocketFactory::GetTypeId ());
      Ptr<MyApp> app = CreateObject<MyApp> ();
      app->Setup (ns3TcpSocket, sinkAddress, payloadSize, DataRate (senderDataRate), simulationTimeInSeconds);
      d.GetLeft (i)->AddApplication (app);
      app->SetStartTime (Seconds (1));
      app->SetStopTime (Seconds (simulationTimeInSeconds));

      std::ostringstream oss;
      oss << output_folder << "/flow" << i+1 <<  ".cwnd";
      AsciiTraceHelper asciiTraceHelper;
      Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (oss.str());
      ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));
    }
    
  
    Simulator::Stop (Seconds (simulationTimeInSeconds+cleanupTime));
    Simulator::Run ();


    // flow monitor statistics
    int j = 0;
    float AvgThroughput = 0;
    float CurThroughput = 0;
    // float CurPacketLossRate = 0;
    float CurThroughputArr[] = {0, 0};

    double jain_index_numerator = 0;
    double jain_index_denominator = 0;

    std::ofstream MyFile(file, std::ios_base::app);

    // variables for output measurement
    uint32_t SentPackets = 0;
    uint32_t ReceivedPackets = 0;
    uint32_t LostPackets = 0;

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

    for (auto iter = stats.begin (); iter != stats.end (); ++iter) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first); 
            // classifier returns FiveTuple in correspondance to a flowID

      NS_LOG_UNCOND("----Flow ID:" <<iter->first);
      NS_LOG_UNCOND("Src Addr" <<t.sourceAddress << " -- Dst Addr "<< t.destinationAddress);
      NS_LOG_UNCOND("Sent Packets = " <<iter->second.txPackets);
      NS_LOG_UNCOND("Received Packets = " <<iter->second.rxPackets);
      NS_LOG_UNCOND("Lost Packets = " <<iter->second.lostPackets);
      NS_LOG_UNCOND("Packet delivery ratio = " <<iter->second.rxPackets*100.0/iter->second.txPackets << "%");
      NS_LOG_UNCOND("Packet loss ratio = " << (iter->second.lostPackets)*100.0/iter->second.txPackets << "%");
      // NS_LOG_UNCOND("Packet lost diff way = "<< iter->second.lostPackets);
      NS_LOG_UNCOND("Delay = " <<iter->second.delaySum / iter->second.rxPackets);
      NS_LOG_UNCOND("Jitter = " <<iter->second.jitterSum);
      // NS_LOG_UNCOND("Throughput = " <<iter->second.rxBytes * 8.0/((iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())*1024)<<"Kbps");
      NS_LOG_UNCOND("Throughput = " <<iter->second.rxBytes * 8.0/((simulationTimeInSeconds+cleanupTime)*1000)<<"Kbps");
    

      CurThroughput = iter->second.rxBytes * 8.0/((simulationTimeInSeconds+cleanupTime)*1000);
      // CurPacketLossRate = (iter->second.lostPackets)*100.0/iter->second.txPackets;
      if(j%2 == 0) { CurThroughputArr[0] += iter->second.rxBytes; }
      if(j%2 == 1) { CurThroughputArr[1] += iter->second.rxBytes; }


      SentPackets = SentPackets +(iter->second.txPackets);
      ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
      LostPackets = LostPackets + (iter->second.lostPackets);
      // AvgThroughput = AvgThroughput + CurThroughput;
      j = j + 1;

      // https://en.wikipedia.org/wiki/Fairness_measure
      jain_index_numerator += CurThroughput;
      jain_index_denominator += (CurThroughput * CurThroughput);

      // only want the src to dest throughput
      // if(j == nFlows) break;
    }

    double jain_index = (jain_index_numerator * jain_index_numerator) / ( j * jain_index_denominator);
    CurThroughputArr[0] /= ((simulationTimeInSeconds + cleanupTime)*1000);
    CurThroughputArr[1] /= ((simulationTimeInSeconds + cleanupTime)*1000);
    AvgThroughput = CurThroughputArr[0] + CurThroughputArr[1];

    // cols : bttlneck rate | random packet loss | throughput1 | throughput2 | jain_index
    MyFile << bottleNeckDataRate.substr(0, bottleNeckDataRate.length()-4) << " " << -1*packet_loss_exp << " " << CurThroughputArr[0] << " " << CurThroughputArr[1] << " " << " " << jain_index <<std::endl;
    // friendliness https://icapeople.epfl.ch/widmer/files/Widmer2001a.pdf

    // AvgThroughput = AvgThroughput/j;
    NS_LOG_UNCOND("\n--------Total Results of the simulation----------"<<std::endl);
    NS_LOG_UNCOND("Total sent packets  = " << SentPackets);
    NS_LOG_UNCOND("Total Received Packets = " << ReceivedPackets);
    NS_LOG_UNCOND("Total Lost Packets = " << LostPackets);
    NS_LOG_UNCOND("Packet Loss ratio = " << ((LostPackets*100.00)/SentPackets)<< "%");
    NS_LOG_UNCOND("Packet delivery ratio = " << ((ReceivedPackets*100.00)/SentPackets)<< "%");
    NS_LOG_UNCOND("Average Throughput = " << AvgThroughput<< "Kbps");
    NS_LOG_UNCOND("Jain's Fairness Index = " << jain_index);
    NS_LOG_UNCOND("Total Flow id " << j);
    NS_LOG_UNCOND("#######################################################################\n\n");


    Simulator::Destroy ();

    return 0;
}