#include "ns3/point-to-point-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/mobility-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/propagation-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv6-flow-classifier.h"
#include "ns3/flow-monitor-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TaskA2");

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

  // takbir mod
  if(Simulator::Now().GetSeconds() < m_simultime) ScheduleTx();
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      // NS_LOG_UNCOND("Time is "<<tNext.GetSeconds());
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

// static void
// CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
// {
//   // NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
//   *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd << std::endl;
// }

// print positions
void GetPosition(NodeContainer container){
  for (NodeContainer::Iterator j = container.Begin ();
        j != container.End (); ++j)
    {
      Ptr<Node> object = *j;
      Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
      NS_ASSERT (position != 0);
      Vector pos = position->GetPosition ();
      std::cout << "x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
    }
}

int main(int argc, char *argv[]){
    uint32_t payloadSize = 100;

    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));


    // std::string tcpVariant1 = "ns3::TcpCubic";
    // std::string tcpVariant2 = "ns3::TcpCubic";  // TcpNewReno

    int tx_range = 1;
    std::string bottleNeckDelay = "1ms";
    std::string output_folder = "taskA";
    std::string file = "taskA/taskA2/data.txt";


    // changes for TASK A
    int nNodes = 20;
    int nFlows = 20;
    int nPacketsPerSecond = 500;
    int coverageArea = 5;

    int simulationTimeInSeconds = 60;
    int cleanupTime = 2;


    // input from CMD
    CommandLine cmd (__FILE__);
    cmd.AddValue ("nNodes","Number of total nodes", nNodes);
    cmd.AddValue ("nFlows","Number of total flows", nFlows);
    cmd.AddValue ("nPacketsPerSecond","Number of packets per second", nPacketsPerSecond);
    cmd.AddValue ("coverageArea","Static coverage area", coverageArea);

    cmd.AddValue ("file","File to store data", file);
    cmd.Parse (argc,argv);
    
    nFlows = nFlows/2; // considering ack flow as independent flow
    int nLeaf = nNodes/2; // minus 2 for ap nodes
    int dataRate = (payloadSize * nPacketsPerSecond * 8) / 1000;
    coverageArea *= tx_range;
    // std::string senderDataRate = std::to_string(dataRate) + "Kbps";
    std::string senderDataRate = std::to_string(dataRate) + "Kbps";
    // std::string bottleNeckDataRate = std::to_string(dataRate / 10) + "Kbps";
    std::string bottleNeckDataRate = "1Mbps";

    NS_LOG_UNCOND("Using nodes : "<<nNodes<<" ; flows : "<<nFlows<<" ; packets per sec : "<<nPacketsPerSecond<<" ; "\
                  "coverage area : "<<coverageArea<<" ; sender data rate : "<<senderDataRate<<" ; bottleneck data rate : "<<bottleNeckDataRate);
    // srand(time(0));

    // config some default values
    // Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpVariant));
    // Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
    // Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue(coverageArea));


    // 2 MB of TCP buffer
    // Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << 21));
    // Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << 21));
    // Config::SetDefault ("ns3::TcpSocketBase::Sack", BooleanValue (true));

    /////////////////////// SETUP NODES /////////////////////// 
    // setup routers
    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    // routers in p2p
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue (bottleNeckDataRate));
    pointToPoint.SetChannelAttribute ("Delay", StringValue (bottleNeckDelay));

    NetDeviceContainer pointToPointDevice;
    pointToPointDevice = pointToPoint.Install(p2pNodes);

    // setup sender
    NodeContainer senderNodes, receiverNodes;
    senderNodes.Add (p2pNodes.Get (0));
    senderNodes.Create (nLeaf);

    //setup receivers 
    receiverNodes.Add (p2pNodes.Get (1));
    receiverNodes.Create(nLeaf);

    //setup mobility model
    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (tx_range),
                                 "DeltaY", DoubleValue (tx_range),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

    // mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
    //                                 "rho", DoubleValue (5),
    //                                 "X", DoubleValue (2.5), 
    //                                 "Y", DoubleValue (2.5),
    //                                 "Z", DoubleValue (2.5));

    
    // as static
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    // TODO : add coverage area 
    mobility.Install(senderNodes);
    mobility.Install(receiverNodes);
    

    /////////////////////// SETUP TOPOLOGY HELPERS ///////////////////////

    // senders in wifi
    Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue(coverageArea));
    Ptr<SingleModelSpectrumChannel> leftChannel = CreateObject<SingleModelSpectrumChannel> ();
    Ptr<RangePropagationLossModel> leftproplossModel = CreateObject<RangePropagationLossModel> ();
    Ptr<ConstantSpeedPropagationDelayModel> leftdelayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
    // leftdelayModel->SetSpeed(leftdelayModel->GetSpeed()*dataRate);
    // leftdelayModel->SetSpeed(dataRate);
    leftChannel->AddPropagationLossModel (leftproplossModel); 
    leftChannel->SetPropagationDelayModel (leftdelayModel);

    // senders in wifi
    Ptr<SingleModelSpectrumChannel> rightChannel = CreateObject<SingleModelSpectrumChannel> ();
    Ptr<RangePropagationLossModel> rightproplossModel = CreateObject<RangePropagationLossModel> ();
    Ptr<ConstantSpeedPropagationDelayModel> rightdelayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
    // rightdelayModel->SetSpeed(rightdelayModel->GetSpeed()*dataRate);
    rightChannel->AddPropagationLossModel (rightproplossModel); 
    rightChannel->SetPropagationDelayModel (rightdelayModel);

    LrWpanHelper senderWpanH, receiverWpanH;
    senderWpanH.SetChannel(leftChannel);
    receiverWpanH.SetChannel(rightChannel);

    NetDeviceContainer senderDevices = senderWpanH.Install (senderNodes);
    NetDeviceContainer receiverDevices = receiverWpanH.Install (receiverNodes);
    
    senderWpanH.AssociateToPan (senderDevices, 0); // 2nd argument is pan-id, associates all devices to this pan 
    receiverWpanH.AssociateToPan (receiverDevices, 1); 

    // iterate our nodes and print their position.
    // GetPosition(senderWifiStaNodes);
    // NS_LOG_UNCOND("AP -> ");
    // GetPosition(senderWifiApNode);
    // GetPosition(receiverWifiStaNodes);
    // NS_LOG_UNCOND("AP -> ");
    // GetPosition(receiverWifiApNode);


    InternetStackHelper stack;
    stack.Install (senderNodes);
    stack.Install (receiverNodes);
    // stack.Install (p2pNodes);

    /////////////////////// ASSIGN IP Addresses ///////////////////////

    SixLowPanHelper senderSixHelper, receiverSixHelper;
    NetDeviceContainer senderSixDevices = senderSixHelper.Install (senderDevices);
    NetDeviceContainer receiverSixDevices = receiverSixHelper.Install (receiverDevices);

    Ipv6AddressHelper address;
    address.SetBase (Ipv6Address ("2001:aaaa::"), Ipv6Prefix (64));
    Ipv6InterfaceContainer p2pInterface = address.Assign (pointToPointDevice);
    
    p2pInterface.SetForwarding (1, true);
    p2pInterface.SetDefaultRouteInAllNodes (1);
    p2pInterface.SetForwarding (0, true);
    p2pInterface.SetDefaultRouteInAllNodes (0);

    address.SetBase (Ipv6Address ("2001:bbbb::"), Ipv6Prefix (64));
    Ipv6InterfaceContainer senderIpInterface = address.Assign (senderSixDevices);
    senderIpInterface.SetForwarding (0, true);
    senderIpInterface.SetDefaultRouteInAllNodes (0);

    address.SetBase (Ipv6Address ("2001:cccc::"), Ipv6Prefix (64));
    Ipv6InterfaceContainer receiverIpInterface = address.Assign (receiverSixDevices);
    receiverIpInterface.SetForwarding (0, true);
    receiverIpInterface.SetDefaultRouteInAllNodes (0);


    // setup routing protocols?
    for (uint32_t i = 0; i < senderSixDevices.GetN (); i++) {
      Ptr<NetDevice> dev = senderSixDevices.Get (i);
      dev->SetAttribute ("UseMeshUnder", BooleanValue (true));
      dev->SetAttribute ("MeshUnderRadius", UintegerValue (10));
    }
    for (uint32_t i = 0; i < receiverSixDevices.GetN (); i++) {
      Ptr<NetDevice> dev = receiverSixDevices.Get (i);
      dev->SetAttribute ("UseMeshUnder", BooleanValue (true));
      dev->SetAttribute ("MeshUnderRadius", UintegerValue (tx_range));
    }

    /////////////////////// SETUP SOURCE AND SINK ///////////////////////
    uint16_t sp = 8000;
    NS_LOG_UNCOND("Connections : ");
    
    int sender_no=0;
    int receiver_no, receiever_inc=0;
    for(int flow=0; flow<nFlows; flow++)
    {
      sp = 8000 + flow;
      receiver_no = ((sender_no + receiever_inc) % nLeaf); 
      PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", Inet6SocketAddress (Ipv6Address::GetAny(), sp)); // 0.0.0.0 address 
      // TODO : is this needed
      // packetSinkHelper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
      ApplicationContainer sinkApps = packetSinkHelper.Install (receiverNodes.Get (receiver_no + 1)); // + 1 as 2 nodes are in p2p
      sinkApps.Start (Seconds (0.));
      sinkApps.Stop (Seconds (simulationTimeInSeconds + cleanupTime));

      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (senderNodes.Get (sender_no + 1), TcpSocketFactory::GetTypeId ());
      Ptr<MyApp> app = CreateObject<MyApp> ();
      app->Setup (ns3TcpSocket, Inet6SocketAddress (receiverIpInterface.GetAddress(receiver_no + 1, 1), sp), payloadSize, DataRate (senderDataRate), simulationTimeInSeconds);
      senderNodes.Get (0)->AddApplication (app);
      app->SetStartTime (Seconds (1.));
      app->SetStopTime (Seconds (simulationTimeInSeconds));


      // GetAddress has 2 args, (i=interface index, j=address index)
      NS_LOG_UNCOND(sender_no+1<<" <------> "<<receiver_no+1 << " : " <<senderIpInterface.GetAddress(sender_no + 1, 1)<<" : "<<receiverIpInterface.GetAddress(receiver_no + 1, 1));

      // boundary cases
      sender_no += 1;
      if(sender_no == nLeaf) {
        sender_no = 0;      // overflow once. now receiver node will be the one next to the current ones
        receiever_inc += 1;
      }
    }


    // Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    // install flow monitor
    FlowMonitorHelper flowmon;
    flowmon.SetMonitorAttribute("MaxPerHopDelay", TimeValue(Seconds(cleanupTime)));
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();


    Simulator::Stop (Seconds (simulationTimeInSeconds + cleanupTime));
    Simulator::Run ();
    
    /////////////////////// FLOW MONITOR STATUS ///////////////////////

    // variables for output measurement
    float AvgThroughput = 0;
    Time Delay;
    uint32_t SentPackets = 0;
    uint32_t ReceivedPackets = 0;
    uint32_t ReceivedBytes = 0;
    uint32_t LostPackets = 0;

    std::ofstream MyFile(file, std::ios_base::app);

    Ptr<Ipv6FlowClassifier> classifier = DynamicCast<Ipv6FlowClassifier> (flowmon.GetClassifier6 ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

    int j=0;
    for (auto iter = stats.begin (); iter != stats.end (); ++iter) {
      Ipv6FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first); 
      // classifier returns FiveTuple in correspondance to a flowID

      NS_LOG_UNCOND("----Flow ID:" <<iter->first);
      NS_LOG_UNCOND("Src Addr" <<t.sourceAddress << " -- Dst Addr "<< t.destinationAddress);
      NS_LOG_UNCOND("Sent Packets = " <<iter->second.txPackets);
      NS_LOG_UNCOND("Received Packets = " <<iter->second.rxPackets);
      NS_LOG_UNCOND("Lost Packets = " <<iter->second.lostPackets);
      NS_LOG_UNCOND("Packet delivery ratio = " <<iter->second.rxPackets*100.0/iter->second.txPackets << "%");
      NS_LOG_UNCOND("Packet loss ratio = " << (iter->second.lostPackets)*100.0/iter->second.txPackets << "%");
      NS_LOG_UNCOND("Packet lost diff way = "<< iter->second.lostPackets);
      if(iter->second.rxPackets != 0) NS_LOG_UNCOND("Delay = " <<iter->second.delaySum / iter->second.rxPackets);
      // NS_LOG_UNCOND("Throughput = " <<iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024<<"Kbps");
      NS_LOG_UNCOND("Throughput = " <<iter->second.rxBytes * 8.0/((simulationTimeInSeconds+cleanupTime)*1000)<<"Kbps");
      NS_LOG_UNCOND(" ");
      SentPackets = SentPackets +(iter->second.txPackets);
      ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
      ReceivedBytes = ReceivedBytes + (iter->second.rxBytes);
      LostPackets = LostPackets + (iter->second.lostPackets);
      Delay = Delay + (iter->second.delaySum);

      j += 1;
    }

    AvgThroughput = ReceivedBytes*8.0 / ((simulationTimeInSeconds + cleanupTime)*1000);
    // AvgThroughput = AvgThroughput/(2*nFlows);
    NS_LOG_UNCOND("\n--------Total Results of the simulation----------"<<std::endl);
    NS_LOG_UNCOND("Total sent packets  = " << SentPackets);
    NS_LOG_UNCOND("Total Received Packets = " << ReceivedPackets);
    NS_LOG_UNCOND("Total Lost Packets = " << LostPackets);
    NS_LOG_UNCOND("METRICS >> ");
    NS_LOG_UNCOND("Average Throughput = " << AvgThroughput<< "Kbps");
    if(ReceivedPackets != 0) NS_LOG_UNCOND("End to End Delay = " << Delay/ReceivedPackets);
    NS_LOG_UNCOND("Packet Delivery Ratio = " << ((ReceivedPackets*100.00)/SentPackets)<< "%");
    NS_LOG_UNCOND("Packet Drop Ratio = " << ((LostPackets*100.00)/SentPackets)<< "%");
    NS_LOG_UNCOND("Total Flows " << j);
    NS_LOG_UNCOND("#######################################################################\n\n");




    // first x axes
    MyFile << nNodes << " " << 2*nFlows << " " << nPacketsPerSecond << " " << coverageArea  << " ";
    // then y values
    MyFile << AvgThroughput << " " << Delay/ReceivedPackets << " " << ((ReceivedPackets*100.00)/SentPackets) << " " << ((LostPackets*100.00)/SentPackets) <<std::endl;

    MyFile.close(); // TODO check this in task B files

    Simulator::Destroy ();

    return 0;
}