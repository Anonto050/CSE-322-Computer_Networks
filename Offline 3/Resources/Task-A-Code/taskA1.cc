#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/yans-error-rate-model.h"

#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TaskA1");

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
    uint32_t payloadSize = 1024;
    // std::string tcpVariant1 = "ns3::TcpCubic";
    // std::string tcpVariant2 = "ns3::TcpCubic";  // TcpNewReno

    int tx_range = 1;
    std::string bottleNeckDelay = "2ms";
    std::string output_folder = "taskA";
    std::string file = "taskA/taskA1/data.txt";


    // changes for TASK A
    int nNodes = 20;
    int nFlows = 20;
    int nPacketsPerSecond = 500;
    int coverageArea = 5;

    int simulationTimeInSeconds = 25;
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

    NS_LOG_UNCOND("Using nodes : "<<nNodes<<" ; flows : "<<2*nFlows<<" ; packets per sec : "<<nPacketsPerSecond<<" ; "\
                  "coverage area : "<<coverageArea<<" ; sender data rate : "<<senderDataRate<<" ; bottleneck data rate : "<<bottleNeckDataRate);
    // srand(time(0));

    // config some default values
    // Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpVariant));
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
    Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue(coverageArea));

    /////////////////////// SETUP NODES /////////////////////// 
    // setup routers
    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    // setup sender devices
    NodeContainer senderWifiStaNodes, receiverWifiStaNodes;
    NodeContainer senderWifiApNode = p2pNodes.Get(0);
    senderWifiStaNodes.Create(nLeaf);

    //setup receivers devices
    NodeContainer receiverWifiApNode = p2pNodes.Get(1);
    receiverWifiStaNodes.Create(nLeaf);


    /////////////////////// SETUP TOPOLOGY HELPERS ///////////////////////
    
   // routers in p2p
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue (bottleNeckDataRate));
    pointToPoint.SetChannelAttribute ("Delay", StringValue (bottleNeckDelay));

    NetDeviceContainer pointToPointDevies;
    pointToPointDevies = pointToPoint.Install(p2pNodes);

    

    // senders in wifi
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy;
    YansWifiPhyHelper phy2;

    channel.AddPropagationLoss("ns3::RangePropagationLossModel");
    channel2.AddPropagationLoss("ns3::RangePropagationLossModel");

    phy.SetChannel (channel.Create ());
    phy2.SetChannel (channel2.Create ());


    WifiHelper senderWifi;
    WifiHelper receiverWifi;

    // channel.AddPropagationLoss("ns3::RangePropagationLossModel","MaxRange", DoubleValue (coverageArea));

    senderWifi.SetRemoteStationManager ("ns3::AarfWifiManager");
    receiverWifi.SetRemoteStationManager ("ns3::AarfWifiManager");
    Ssid ssid = Ssid ("ns-3-ssid"); // this is 802.11


    WifiMacHelper mac;
    WifiMacHelper mac2;

    mac.SetType ("ns3::StaWifiMac",
                "Ssid", SsidValue (ssid),
                "ActiveProbing", BooleanValue (false));
    mac2.SetType ("ns3::StaWifiMac",
                "Ssid", SsidValue (ssid),
                "ActiveProbing", BooleanValue (false));

    // sender nodes
    NetDeviceContainer senderStaDevices;
    NetDeviceContainer receiverStaDevices;
    senderStaDevices = senderWifi.Install (phy, mac, senderWifiStaNodes);
    receiverStaDevices = receiverWifi.Install (phy2, mac2, receiverWifiStaNodes);


    mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
    mac2.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
    

    // sender ap
    NetDeviceContainer senderApDevices;
    NetDeviceContainer receiverApDevices;
    senderApDevices = senderWifi.Install (phy, mac, senderWifiApNode);
    receiverApDevices = receiverWifi.Install (phy2, mac2, receiverWifiApNode);

    
    // error model
    // Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    // em->SetAttribute("ErrorRate", DoubleValue (0.000001));

    // for(int i=0; i<2*nLeaf; i++){
    //   Config::Set("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/PostReceptionErrorModel", PointerValue(em));
    // }

    //setup mobility model
    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (1.0),
                                 "DeltaY", DoubleValue (1.0),
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
    mobility.Install(senderWifiStaNodes);
    mobility.Install(senderWifiApNode);
    mobility.Install(receiverWifiStaNodes);
    mobility.Install(receiverWifiApNode);

    // iterate our nodes and print their position.
    GetPosition(senderWifiStaNodes);
    NS_LOG_UNCOND("AP -> ");
    GetPosition(senderWifiApNode);
    GetPosition(receiverWifiStaNodes);
    NS_LOG_UNCOND("AP -> ");
    GetPosition(receiverWifiApNode);

    ///////  ROUTING STUFF
    // https://www.nsnam.org/docs/release/3.14/models/html/routing-overview.html
    // OlsrHelper protocol;
    // // AodvHelper protocol;
    // Ipv4StaticRoutingHelper staticRouting;

    // Ipv4ListRoutingHelper list;
    // list.Add (staticRouting, 0);
    // list.Add (protocol, 100);


    /////////////////////// INSTALL STACK ///////////////////////
    // tcp variant 1
    // Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpVariant1));
    InternetStackHelper stack1;
    stack1.Install(receiverWifiApNode);
    stack1.Install(senderWifiApNode);

    stack1.Install(senderWifiStaNodes);
    stack1.Install(receiverWifiStaNodes);
    

    /////////////////////// ASSIGN IP Addresses ///////////////////////
    Ipv4AddressHelper address;

    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign (pointToPointDevies);


    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer senderApInterface = address.Assign (senderApDevices);
    Ipv4InterfaceContainer senderStaInterfaces = address.Assign (senderStaDevices);


    address.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer receiverApInterface = address.Assign (receiverApDevices);
    Ipv4InterfaceContainer receiverStaInterfaces = address.Assign (receiverStaDevices);


    // setup default route
    // for(NodeContainer::Iterator itr = receiverWifiStaNodes.Begin(); itr != receiverWifiStaNodes.End(); itr++){
    //   Ptr<Ipv4StaticRouting> sr = Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting> ((*itr)->GetObject<Ipv4>()->GetRoutingProtocol());
    //   sr->SetDefaultRoute(receiverApInterface.GetAddress(0), 1);
    // }

    // for(NodeContainer::Iterator itr = senderWifiStaNodes.Begin(); itr != senderWifiStaNodes.End(); itr++){
    //   Ptr<Ipv4StaticRouting> sr = Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting> ((*itr)->GetObject<Ipv4>()->GetRoutingProtocol());
    //   sr->SetDefaultRoute(senderApInterface.GetAddress(0), 1);
    // }
    

    /////////////////////// SETUP SOURCE AND SINK ///////////////////////
    uint16_t sp = 8000;
    NS_LOG_UNCOND("Connections : ");
    
    int sender_no=0;
    int receiver_no, receiever_inc=0;
    for(int flow=0; flow<nFlows; flow++)
    {
      sp = 8000 + flow;
      receiver_no = (sender_no + receiever_inc) % nLeaf; 
      PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), sp)); // 0.0.0.0 address 
      ApplicationContainer sinkApps = packetSinkHelper.Install (receiverWifiStaNodes.Get (receiver_no));
      sinkApps.Start (Seconds (0.));
      sinkApps.Stop (Seconds (simulationTimeInSeconds + cleanupTime));

      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (senderWifiStaNodes.Get (sender_no), TcpSocketFactory::GetTypeId ());
      Ptr<MyApp> app = CreateObject<MyApp> ();
      app->Setup (ns3TcpSocket, InetSocketAddress (receiverStaInterfaces.GetAddress(receiver_no), sp), payloadSize, DataRate (senderDataRate), simulationTimeInSeconds);
      senderWifiApNode.Get (0)->AddApplication (app);
      app->SetStartTime (Seconds (1.));
      app->SetStopTime (Seconds (simulationTimeInSeconds));

      NS_LOG_UNCOND(sender_no<<" <------> "<<receiver_no << " : " <<senderStaInterfaces.GetAddress(sender_no)<<" : "<<receiverStaInterfaces.GetAddress(receiver_no));

      // boundary cases
      sender_no += 1;
      if(sender_no == nLeaf) {
        sender_no = 0;      // overflow once. now receiver node will be the one next to the current ones
        receiever_inc += 1;
      }
    }


    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
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

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

    int j=0;
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

    MyFile.close();

    Simulator::Destroy ();

    return 0;
}