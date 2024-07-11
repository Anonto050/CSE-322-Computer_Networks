# Network Throughput Simulation with NS-3

This project simulates a network topology using NS-3 and calculates the throughput over time. The topology includes multiple WiFi senders and receivers connected via point-to-point links. The throughput and packet delivery ratio are calculated and logged.

## Prerequisites

To run this simulation, ensure you have NS-3 installed on your system. You can download and install NS-3 from the [official website](https://www.nsnam.org/).

## Network Topology

```
S0                                            R0
S1                                            R1
S2-------------R1-------------R2--------------R2
S3                                            R3
S4                                            R4
```

## Code Structure

The code is divided into several parts:

1. **Include Headers:** Includes necessary NS-3 modules and standard libraries.
2. **Logging Component:** Sets up the logging component for the simulation.
3. **Global Variables:** Defines global variables for tracking received bits and packets.
4. **Helper Functions:** Functions for handling packet transmission and reception.
5. **Main Function:** The main simulation setup and execution.

## Running the Simulation

### Step-by-Step Guide

1. **Setup Parameters:**
   - **Purpose:** These parameters define the scale and configuration of the network simulation, such as the number of nodes and flows.
   - **Code:**
     ```cpp
     uint32_t NUM_WIFI = 20;  // Number of WiFi nodes
     int32_t NUM_FLOW = 10;   // Number of flows
     int32_t NUM_PACKET = 100; // Number of packets
     int32_t COVERAGE_AREA = 2; // Coverage area
     double simulationTime = 10; // Simulation time in seconds
     ```

2. **Command Line Argument Parser:**
   - **Purpose:** Allows dynamic configuration of parameters via command line for flexibility in testing different scenarios.
   - **Code:**
     ```cpp
     CommandLine cmd(__FILE__);
     cmd.AddValue("NUM_WIFI", "Number of WiFi nodes", NUM_WIFI);
     cmd.AddValue("NUM_FLOW", "Number of flows", NUM_FLOW);
     cmd.AddValue("NUM_PACKET", "Number of packets", NUM_PACKET);
     cmd.AddValue("COVERAGE_AREA", "Coverage area", COVERAGE_AREA);
     cmd.Parse(argc, argv);
     ```

3. **Configure TCP Options:**
   - **Purpose:** Ensures the TCP segment size matches the payload size for consistent packet transmission.
   - **Code:**
     ```cpp
     Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));
     ```

4. **Create Nodes:**
   - **Purpose:** Sets up the physical devices (nodes) in the simulation, including point-to-point and WiFi nodes.
   - **Code:**
     ```cpp
     NodeContainer p2pNodes;
     p2pNodes.Create(2);

     NodeContainer wifiSenders;
     wifiSenders.Create(NUM_WIFI);

     NodeContainer wifiReceivers;
     wifiReceivers.Create(NUM_WIFI);
     ```

5. **Set Up Point-to-Point Links:**
   - **Purpose:** Establishes the wired connections between nodes with specified data rate and delay.
   - **Code:**
     ```cpp
     PointToPointHelper p2pLink;
     p2pLink.SetDeviceAttribute("DataRate", StringValue(dataRate));
     p2pLink.SetChannelAttribute("Delay", StringValue(channelDelay));

     NetDeviceContainer p2pDevices;
     p2pDevices = p2pLink.Install(p2pNodes);
     ```

6. **Configure WiFi:**
   - **Purpose:** Sets up wireless connections, including channels, PHY, and MAC layers for WiFi communication.
   - **Code:**
     ```cpp
     YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
     wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(COVERAGE_AREA * Tx_range));

     YansWifiPhyHelper phySender;
     phySender.SetChannel(wifiChannel.Create());

     YansWifiPhyHelper phyReceiver;
     phyReceiver.SetChannel(wifiChannel.Create());

     WifiHelper wifiHelper;
     WifiMacHelper macSender, macReceiver;

     Ssid ssidSender = Ssid("ns3-ssid-sender");
     Ssid ssidReceiver = Ssid("ns3-ssid-receiver");
     ```

7. **Set Up Mobility:**
   - **Purpose:** Configures the movement or static positions of nodes in the simulation space.
   - **Code:**
     ```cpp
     MobilityHelper mobility;
     mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(0.5),
                                  "DeltaY", DoubleValue(1.0),
                                  "GridWidth", UintegerValue(5),
                                  "LayoutType", StringValue("RowFirst"));
    
     mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
     mobility.Install(wifiSenders);
     mobility.Install(wifiReceivers);
     ```

8. **Install Internet Stack:**
   - **Purpose:** Installs the necessary networking protocols (e.g., TCP/IP) on the nodes to enable communication.
   - **Code:**
     ```cpp
     InternetStackHelper stack;
     stack.Install(wifiApNodeSenders);
     stack.Install(wifiApNodeReceivers);
     stack.Install(wifiSenders);
     stack.Install(wifiReceivers);
     ```

9. **Assign IP Addresses:**
   - **Purpose:** Assigns unique IP addresses to each device for proper routing and communication.
   - **Code:**
     ```cpp
     Ipv4AddressHelper address;
     address.SetBase("10.1.1.0", "255.255.255.0");
     Ipv4InterfaceContainer p2pInterfaces = address.Assign(p2pDevices);

     address.SetBase("10.1.2.0", "255.255.255.0");
     Ipv4InterfaceContainer wifiInterfacesSender = address.Assign(wifiDevicesSender);
     Ipv4InterfaceContainer wifiApInterfacesSender = address.Assign(wifiApDevicesSender);

     address.SetBase("10.1.3.0", "255.255.255.0");
     Ipv4InterfaceContainer wifiInterfacesReceiver = address.Assign(wifiDevicesReceiver);
     Ipv4InterfaceContainer wifiApInterfacesReceiver = address.Assign(wifiApDevicesReceiver);
     ```

10. **Install Applications:**
    - **Purpose:** Sets up traffic generation and reception applications on the nodes to simulate network traffic.
    - **Code:**
      ```cpp
      OnOffHelper onoff("ns3::TcpSocketFactory", Address());
      onoff.SetAttribute("PacketSize", UintegerValue(payloadSize));
      onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
      onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
      onoff.SetAttribute("DataRate", DataRateValue(DataRate(APP_DATA_RATE)));

      Address sinkLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
      PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddress);

      ApplicationContainer receiverApps;
      ApplicationContainer senderApps;

      for (uint32_t i = 0; i < NUM_WIFI; i++) {
          receiverApps.Add(sinkHelper.Install(wifiReceivers.Get(i)));
          Ptr<PacketSink> sink = DynamicCast<PacketSink>(receiverApps.Get(i));
          sink->TraceConnectWithoutContext("Rx", MakeCallback(&PacketSinkRxTrace));
      }

      receiverApps.Start(Seconds(1.0));
      receiverApps.Stop(Seconds(simulationTime));

      int32_t flow_count = 0;

      for(uint32_t i = 0; i < NUM_WIFI; i++) {
          AddressValue remoteAddress(InetSocketAddress(wifiInterfacesReceiver.GetAddress(i), port));
          for(uint32_t j = 0; j < NUM_WIFI; j++) {
              onoff.SetAttribute("Remote", remoteAddress);
              senderApps.Add(onoff.Install(wifiSenders.Get(j)));
              flow_count++;
              if(flow_count >= NUM_FLOW) {
                  break;
              }
          }
          if(flow_count >= NUM_FLOW) {
              break;
          }
      }

      for (uint32_t i = 0; i < senderApps.GetN(); i++) {
          Ptr<OnOffApplication> onoffApp = DynamicCast<OnOffApplication>(senderApps.Get(i));
          onoffApp->TraceConnectWithoutContext("Tx", MakeCallback(&PacketSourceTxTrace));
      }

      senderApps.Start(Seconds(2.0));
      senderApps.Stop(Seconds(simulationTime - 1));
      ```

11. **Run Simulation:**
    - **Purpose:** Populates routing tables and executes the simulation to model the network behavior.
    - **Code:**
      ```cpp
      Ipv4GlobalRoutingHelper::PopulateRoutingTables();
      Simulator::Stop(Seconds(simulationTime));
      Simulator::Run();
      ```

12. **Calculate Throughput:**
    - **Purpose:** Calculates and logs the network performance metrics such as average throughput and packet delivery ratio.
    - **Code:**
      ```cpp
      double avgThroughput = totalReceivedBits / (simulationTime * 1e6); // Mbps
      uint32_t totalSentPacketsCount = 0;

      for (uint32_t i = 0; i < NUM_WIFI; i++) {
          totalSentPacketsCount += totalSentPackets[i];
      }

      double packetDeliveryRatio = static_cast<double>(totalReceivedPackets) / totalSentPacketsCount;

      std::ofstream outputStream("output.dat", std::ios_base::app);

      if (!outputStream.is_open()) {
          std::cerr << "Failed to open output file!" << std::endl;
          return 1;
      }

      std::cout << "Average Throughput: " << avgThroughput << " Mbps" << std::endl;
      std::cout << "Packet Delivery Ratio: " << packetDeliveryRatio << std::endl;

      outputStream << avgThroughput << " " << packetDeliveryRatio << " " << NUM_WIFI << " " << NUM_FLOW << " " << NUM_PACKET << " " << COVERAGE_AREA << std::endl;
      outputStream.close();
      ```

13. **Destroy Simulator:**
    - **Purpose:** Cleans up the simulation environment and exits the program.
    - **Code:**
      ```cpp
      Simulator::Destroy();
      return 0;
      ```

## Output

The output will display the average throughput in Mbps and the packet delivery ratio. The results are also appended to `output.dat`.

## Plotting Throughput vs Time

To plot the throughput vs time graph, you can use the data generated in `output.dat` and any plotting tool like Gnuplot, Python (matplotlib), or Excel.

Example using Python:

```python
import matplotlib.pyplot as plt

data = []
with open('output.dat', 'r') as f:
    for line in f:
        data.append([float(x) for x in line.split()])

throughput = [x[0] for x in data]
time = list(range(len(throughput)))

plt.plot(time, throughput)
plt.xlabel('Time (s)')
plt.ylabel('Throughput (Mbps)')
plt.title('Throughput vs Time')
plt.grid(True)
plt.show()
```

This guide should help you understand and run the network simulation using NS-3. For further customization and experimentation, refer to the [NS-3 documentation](https://www.nsnam.org/documentation/).
