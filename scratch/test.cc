// Description: This file is used to simulate the throughput of the network.
// Author: Kerry
// Date: 2025-03-27


#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/config-store-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/aodv-module.h"
#include "ns3/log.h"

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#define numNodePairs 1 // number of node pairs

using namespace ns3;

double globaltotal = 0;
double globalthroughput[numNodePairs] = {0};
double globalreceivepacket[numNodePairs] = {0};
double globalloss[numNodePairs]={0};
double globaltotalthroughput=0;
int hidden = 0; // 1: hidden terminal, 0: exposed terminal
int far=0; 
double xspace=100; // x space between tx and rx node
double yspace=200; // y space between nodes
double distance=500; // x space between 2 node-pair
// Jonathan
uint32_t numNodes = 2 * numNodePairs; // number of nodes

int PacketSize = 100; // bytes
double  TotalSimulationTime_sec = 10; // seconds

NS_LOG_COMPONENT_DEFINE ("Simulation");


// 產生constant bit rate的流量
ApplicationContainer gencbr(NodeContainer server,NodeContainer client,Ipv4Address address,double simulationTime,double starttime,double pingtime)
{
  //NS_LOG_UNCOND("In gencbr");

  // 建立 UDP Server
  UdpServerHelper myServer (8);
  ApplicationContainer serverApp = myServer.Install (server);
  serverApp.Start (Seconds (0.0));
  serverApp.Stop (Seconds (simulationTime+1));

  // 建立 UDP Client
  UdpClientHelper myClient (address, 8); // address: server的IP, 8: port
  myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u)); // 最大封包數量
  //myClient.SetAttribute ("Interval", TimeValue (Time ("0.0001"))); // 每個封包之間的發送間隔 packets/s
  //myClient.SetAttribute ("PacketSize", UintegerValue (1472)); // 封包大小 bytes
  /*  (問題) 1472*8/0.0001 = 117Mbps ，在cris中以ac VHT MCS3 搭配，80MH 的頻寬、800ns GI，1x1，理論上傳輸速率能達到117Mbps 。*/
  /**
   * 802.11ah MCS 9 in 1MHz channel, normal GI
   * max data rate is 4.0 Mbps
   * 100*8/0.0002 = 4Mbps
   */
  myClient.SetAttribute ("Interval", TimeValue (Time ("0.0002"))); // 每個封包之間的發送間隔 packets/s
  myClient.SetAttribute ("PacketSize", UintegerValue (PacketSize)); // 封包大小 bytes

  ApplicationContainer clientApp = myClient.Install (client);
  clientApp.Start (Seconds (1.0+starttime)); // 開始發送封包的時間
  clientApp.Stop (Seconds (1.0+starttime+TotalSimulationTime_sec)); // 總共傳TotalSimulationTime_sec秒的封包
  
  // 建立 UDP Echo Client，?用來測量封包的往返時間?
  UdpEchoClientHelper echoClientHelper (address, 9);
  echoClientHelper.SetAttribute ("MaxPackets", UintegerValue (1)); // 只發送一個封包
  echoClientHelper.SetAttribute ("Interval", TimeValue (Seconds (0.1))); // 每0.1秒發送一個封包
  echoClientHelper.SetAttribute ("PacketSize", UintegerValue (10)); // 封包大小10 bytes
  ApplicationContainer pingApps;

  echoClientHelper.SetAttribute ("StartTime", TimeValue (Seconds (pingtime/10)));
  pingApps.Add (echoClientHelper.Install (client)); 
  
  return serverApp;
}

// 取得YansWifiPhy的指標 (用來設定頻道)
Ptr<YansWifiPhy>
GetYansWifiPhyPtr (const NetDeviceContainer &nc)
{
  Ptr<WifiNetDevice> wnd = nc.Get (0)->GetObject<WifiNetDevice> ();
  Ptr<WifiPhy> wp = wnd->GetPhy ();
  return wp->GetObject<YansWifiPhy> ();
}

void runthesimulation()
{
  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("10000000")); // bytes
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("100000000")); // bytes
  // Fix non-unicast data rate to be the same as that of unicast
  //Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue ("OfdmRate6MbpsBW10MHz")); // 6Mbps
  // Carrier sense threshold, Channel is sensed busy if energy above CST
  //Config::SetDefault ("ns3::WifiPhy::CcaMode1Threshold", DoubleValue (-106.0)); // dBm
  // Rx will try to decode the packet if RSSI > EDT
  //Config::SetDefault ("ns3::WifiPhy::EnergyDetectionThreshold", DoubleValue (-103.0)); // dBm
  
  // Create nodes
  NodeContainer container;

  container.Create (numNodes);
  
  
  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;

  wifi.SetStandard (WIFI_PHY_STANDARD_80211ah);
  StringValue DataRate = S1gWifiMacHelper::DataRateForMcs (8);
  // wifi.SetRemoteStationManager( "ns3::IdealWifiManager" );
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode", /*StringValue*/DataRate,
                                  "ControlMode", /*StringValue*/DataRate);
  
  // wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
  //                                 "DataMode", StringValue("OfdmRate300KbpsBW1MHz"),
  //                                 "ControlMode", StringValue("OfdmRate300KbpsBW1MHz"));
     


  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  wifiPhy.Set("CcaMode1Threshold", DoubleValue(-83.0)); 
  wifiPhy.Set("EnergyDetectionThreshold", DoubleValue(-80.0));

  YansWifiPhyHelper wifiPhy2 =  YansWifiPhyHelper::Default ();
  wifiPhy2.Set("CcaMode1Threshold", DoubleValue(-83.0));
  wifiPhy2.Set("EnergyDetectionThreshold", DoubleValue(-80.0));  

  YansWifiChannelHelper wifiChannel=  YansWifiChannelHelper::Default();
  YansWifiChannelHelper wifiChannel2=  YansWifiChannelHelper::Default();
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy2.SetChannel (wifiChannel2.Create ());
  S1gWifiMacHelper wifiMac = S1gWifiMacHelper::Default ();
  S1gWifiMacHelper wifiMac2 = S1gWifiMacHelper::Default ();
  BusytoneChannelHelper bChannel = BusytoneChannelHelper::Default ();
  BusytonePhyHelper bPhy =  BusytonePhyHelper::Default ();
  bPhy.SetChannel (bChannel.Create ());
  wifi.SetBusytonePhyHelper(bPhy);
  
  wifiMac.SetType ("ns3::AdhocWifiMac");
  wifiMac2.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, container);
  NetDeviceContainer devices2 = wifi.Install (wifiPhy2, wifiMac2 , container);

  //Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/Slot", TimeValue (MicroSeconds (0.0)));
  
  wifiPhy.EnablePcap("adhoc-test", devices); // enable pcap for all devices

  Ptr<YansWifiPhy> phySta,phySta2 ;

  phySta = GetYansWifiPhyPtr (devices);
  phySta2 = GetYansWifiPhyPtr (devices2);
  std::cout << "Data ch. Freq: " << int(phySta->GetFrequency ()) << std::endl;
  std::cout << "Data ch. Freq: " << int(phySta2->GetFrequency ()) << std::endl;
  std::cout << "ChannelWidth: " << int(phySta->GetChannelWidth()) << "\nGI: " << (phySta->GetShortGuardInterval()? 4000:8000) << "ns" << std::endl;
  std::cout << "Data rate: " << DataRate.Get() << std::endl;
  // Note that with FixedRssLossModel, the positions below are not
  // used for received signal strength.

  MobilityHelper mobility;
  /*----------------------------------------------- */
  // Fixed topology
  /*----------------------------------------------- */
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (xspace, 0.0, 0.0));
  // positionAlloc->Add (Vector (xspace+distance, 0.0, 0.0));
  // positionAlloc->Add (Vector (2*xspace+distance ,0.0, 0.0));
  // positionAlloc->Add (Vector (0.0, yspace, 0.0));
  // positionAlloc->Add (Vector (xspace, yspace, 0.0));
  // positionAlloc->Add (Vector (xspace+distance,yspace, 0.0));
  // positionAlloc->Add (Vector (2*xspace+distance, yspace, 0.0));
  // positionAlloc->Add (Vector (0.0, 2*yspace, 0.0));
  // positionAlloc->Add (Vector (xspace, 2*yspace, 0.0));
  // positionAlloc->Add (Vector (xspace+distance, 2*yspace, 0.0));
  // positionAlloc->Add (Vector (2*xspace+distance, 2*yspace, 0.0));
  // positionAlloc->Add (Vector (0.0, 3*yspace, 0.0));
  // positionAlloc->Add (Vector (xspace, 3*yspace, 0.0));
  // positionAlloc->Add (Vector (xspace+distance, 3*yspace, 0.0));
  // positionAlloc->Add (Vector (2*xspace+distance,3*yspace, 0.0));
  // positionAlloc->Add (Vector (0.0, 4*yspace, 0.0));
  // positionAlloc->Add (Vector (xspace, 4*yspace, 0.0));
  // positionAlloc->Add (Vector (xspace+distance, 4*yspace, 0.0));
  // positionAlloc->Add (Vector (2*xspace+distance,4*yspace, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  /*----------------------------------------------- */


  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (container);
  
  //AodvHelper aodv;

  //Ipv4ListRoutingHelper list;
  //list.Add (aodv, 10);//install Protocol to node


  InternetStackHelper stack;
  stack.Install (container);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);
  ipv4.SetBase ("10.1.10.0", "255.255.255.0");
  Ipv4InterfaceContainer j = ipv4.Assign (devices2);

  double  a[10]={0.001,0.0015,0.002,0.0025,0.003,0.0035,0.004,0.0045,0.005,0.0055};
  double  temp;
  int r1 ,r2;
  srand(time(NULL));
  for(int i=0;i<10;++i)
  {
    r1 = rand() % 10;
    r2 = rand() % 10;
    temp = a[r1];
    a[r1] = a[r2];
    a[r2] = temp;	
  }
  
  ApplicationContainer cbrApps;
  
  /*----------------------------------------------- */
  // Flows for fixed topology
  /*----------------------------------------------- */   
  if(hidden == 1)
  {
    // cbrApps.Add(gencbr(c.Get (1),c.Get (0),i.GetAddress (1),TotalSimulationTime_sec,a[0],0.1));
    // cbrApps.Add(gencbr(c.Get (3),c.Get (2),i.GetAddress (3),TotalSimulationTime_sec,a[1],0.2));
    // cbrApps.Add(gencbr(c.Get (5),c.Get (4),i.GetAddress (5),TotalSimulationTime_sec,a[2],0.3));
    // cbrApps.Add(gencbr(c.Get (7),c.Get (6),i.GetAddress (7),TotalSimulationTime_sec,a[3],0.4));
    // cbrApps.Add(gencbr(c.Get (9),c.Get (8),i.GetAddress (9),TotalSimulationTime_sec,a[4],0.5));
    // cbrApps.Add(gencbr(c.Get (11),c.Get (10),i.GetAddress (11),TotalSimulationTime_sec,a[5],0.6));
    // cbrApps.Add(gencbr(c.Get (13),c.Get (12),i.GetAddress (13),TotalSimulationTime_sec,a[6],0.7));
    // cbrApps.Add(gencbr(c.Get (15),c.Get (14),i.GetAddress (15),TotalSimulationTime_sec,a[7],0.8));
    // cbrApps.Add(gencbr(c.Get (17),c.Get (16),i.GetAddress (17),TotalSimulationTime_sec,a[8],0.9));
    // cbrApps.Add(gencbr(c.Get (19),c.Get (18),i.GetAddress (19),TotalSimulationTime_sec,a[9],1.0));
  }
  else
  {
    cbrApps.Add(gencbr(container.Get(0), container.Get(1), interfaces.GetAddress(0), TotalSimulationTime_sec, a[0], 0.1));
    // cbrApps.Add(gencbr(c.Get (3),c.Get (2),i.GetAddress (3),TotalSimulationTime_sec,a[1],0.2));
    // cbrApps.Add(gencbr(c.Get (4),c.Get (5),i.GetAddress (4),TotalSimulationTime_sec,a[2],0.3));
    // cbrApps.Add(gencbr(c.Get (7),c.Get (6),i.GetAddress (7),TotalSimulationTime_sec,a[3],0.4));
    // cbrApps.Add(gencbr(c.Get (8),c.Get (9),i.GetAddress (8),TotalSimulationTime_sec,a[4],0.5));
    // cbrApps.Add(gencbr(c.Get (11),c.Get (10),i.GetAddress (11),TotalSimulationTime_sec,a[5],0.6));
    // cbrApps.Add(gencbr(c.Get (12),c.Get (13),i.GetAddress (12),TotalSimulationTime_sec,a[6],0.7));
    // cbrApps.Add(gencbr(c.Get (15),c.Get (14),i.GetAddress (15),TotalSimulationTime_sec,a[7],0.8));
    // cbrApps.Add(gencbr(c.Get (16),c.Get (17),i.GetAddress (16),TotalSimulationTime_sec,a[8],0.9));
    // cbrApps.Add(gencbr(c.Get (19),c.Get (18),i.GetAddress (19),TotalSimulationTime_sec,a[9],1.0));
  }
  /*----------------------------------------------- */ 

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
 
    // Start Simulation
    Simulator::Stop( Seconds( TotalSimulationTime_sec + 10 ) );
    Simulator::Run ();


    for (size_t i = 0; i < cbrApps.GetN (); ++i)
    {
      Ptr<UdpServer> server = DynamicCast<UdpServer> (cbrApps.Get (i));
      uint32_t totalPacketsThrough = server->GetReceived ();
      uint32_t totalPacketsLost = server->GetLost ();
    
      double throughputkbps = totalPacketsThrough * PacketSize * 8 / (TotalSimulationTime_sec * 1000.0);
    
      globaltotalthroughput += throughputkbps;
      globalreceivepacket[i] += totalPacketsThrough;
      globalthroughput[i] += throughputkbps;
      globalloss[i] += totalPacketsLost;
    }
    
    
    Simulator::Destroy ();
}
int main (int argc, char *argv[])
{
  LogComponentEnable ("Simulation", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpServer", LOG_LEVEL_ALL);
  //LogComponentEnable ("ArpL3Protocol", LOG_LEVEL_ALL);
  //LogComponentEnable ("WifiNetDevice", LOG_LEVEL_ALL);
  //dataRate = LogComponentEnable ("AdhocWifiMac", LOG_LEVEL_ALL);
  //LogComponentEnable ("RegularWifiMac", LOG_LEVEL_ALL);
  //LogComponentEnable ("WifiPhy", LOG_LEVEL_ALL);

  CommandLine cmd;
  cmd.AddValue("hidden","hidden or not",hidden);
  cmd.AddValue("xspace","x space",xspace);
  cmd.AddValue("yspace","y space",yspace);
  cmd.AddValue("distance","receiver distance",distance);
  cmd.Parse (argc,argv);
  int testtime = 1;

  for(int i=0; i<testtime; i++){
      runthesimulation();
  }
  for(int i=0; i<numNodePairs; i++){
    std::cout<<"Flow "<<i<<" : "<<std::endl;
    std::cout<<"  AverageThroughput :   "<<globalthroughput[i]/testtime<<" kbps "<<std::endl;
    std::cout<<"  Average RxPackets :   "<<globalreceivepacket[i]/testtime<<std::endl;
    std::cout<<"  Average RxBytes   :   "<<globalreceivepacket[i]/testtime*PacketSize<<std::endl;
    std::cout<<"  loss              :   "<<globalloss[i]/testtime<<std::endl;
  }
  std::cout<<"Total throughput : "<<globaltotalthroughput/testtime<<" kbps "<<std::endl;
  return 0;
}