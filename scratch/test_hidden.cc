#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include <stdlib.h>
#include <time.h>
#define nodeNum 16

using namespace ns3;

double globaltotal = 0;
double globalthroughput[nodeNum/2] = {0};
double globalreceivepacket[nodeNum/2] = {0};

ApplicationContainer gencbr(NodeContainer server,NodeContainer client,Ipv4Address address,double simulationTime,double starttime,double pingtime)
{
  UdpServerHelper myServer (12345);
  ApplicationContainer serverApp = myServer.Install (server);
  serverApp.Start (Seconds (0.0));
  serverApp.Stop (Seconds (simulationTime + 1));

  UdpClientHelper myClient (address, 12345);
  myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  myClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
  myClient.SetAttribute ("PacketSize", UintegerValue (1472));

  ApplicationContainer clientApp = myClient.Install (client);
  clientApp.Start (Seconds (1.0+starttime));
  clientApp.Stop (Seconds (simulationTime + 1));
  
  UdpEchoClientHelper echoClientHelper (address, 9);
  echoClientHelper.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClientHelper.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  echoClientHelper.SetAttribute ("PacketSize", UintegerValue (10));
  ApplicationContainer pingApps;

  echoClientHelper.SetAttribute ("StartTime", TimeValue (Seconds (pingtime/10)));
  pingApps.Add (echoClientHelper.Install (client)); 
  
  return serverApp;
}

double run(double simulationTime,double space)
{
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (999999));
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", UintegerValue (999999));
  Config::SetDefault ("ns3::WifiPhy::EnergyDetectionThreshold", DoubleValue (-82.0));
  Config::SetDefault ("ns3::WifiPhy::CcaMode1Threshold", DoubleValue (-82.0));  
  NodeContainer nodes;
  nodes.Create (nodeNum);
  
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  BusytoneChannelHelper bChannel = BusytoneChannelHelper::Default ();
  BusytonePhyHelper bPhy =  BusytonePhyHelper::Default ();
  bPhy.SetChannel (bChannel.Create ());
  WifiHelper wifi ;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
  VhtWifiMacHelper mac = VhtWifiMacHelper::Default ();
  
  StringValue DataRate = VhtWifiMacHelper::DataRateForMcs (2);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate,"ControlMode", DataRate);
  
  mac.SetType ("ns3::AdhocWifiMac"); // use ad-hoc MAC
  wifi.SetBusytonePhyHelper(bPhy);
  NetDeviceContainer devices = wifi.Install (phy, mac, nodes);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (30-space, 0.0, 0.0));
  positionAlloc->Add (Vector (30.0, 0.0, 0.0));
  positionAlloc->Add (Vector (70.0, 0.0, 0.0));
  positionAlloc->Add (Vector (70+space, 0.0, 0.0));
  positionAlloc->Add (Vector (30-space, -10.0, 0.0));
  positionAlloc->Add (Vector (30.0, -10.0, 0.0));
  positionAlloc->Add (Vector (70.0, -10.0, 0.0));
  positionAlloc->Add (Vector (70+space, -10.0, 0.0));
  positionAlloc->Add (Vector (30-space, -20.0, 0.0));
  positionAlloc->Add (Vector (30.0, -20.0, 0.0));
  positionAlloc->Add (Vector (70.0, -20.0, 0.0));
  positionAlloc->Add (Vector (70+space, -20.0, 0.0));
  positionAlloc->Add (Vector (30-space, -30.0, 0.0));
  positionAlloc->Add (Vector (30.0, -30.0, 0.0));
  positionAlloc->Add (Vector (70.0, -30.0, 0.0));
  positionAlloc->Add (Vector (70+space, -30.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  
  InternetStackHelper internet;
  internet.Install (nodes);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer NodeInterface = ipv4.Assign (devices);
  double a[nodeNum/2] ;
  //int nodearray[nodeNum];
  double temp;
  int i,c,d;

  for(i=0;i<nodeNum/2;++i)
  {
	a[i] = 0.001 + 0.0005*i;
  }
  for(i=0;i<nodeNum/2;++i)
  {
	c = rand() % 8;
        d = rand() % 8;
	temp = a[c];
	a[c] = a[d];
	a[d] = temp;	
  }

  ApplicationContainer cbrApps;
  /////////////////////////receive//////////send/////////////////////////receive
  cbrApps.Add(gencbr(nodes.Get (1),nodes.Get (0),NodeInterface.GetAddress (1),simulationTime,a[i],0.1));
  cbrApps.Add(gencbr(nodes.Get (2),nodes.Get (3),NodeInterface.GetAddress (2),simulationTime,a[i],0.2));
  cbrApps.Add(gencbr(nodes.Get (5),nodes.Get (4),NodeInterface.GetAddress (5),simulationTime,a[i],0.3));
  cbrApps.Add(gencbr(nodes.Get (6),nodes.Get (7),NodeInterface.GetAddress (6),simulationTime,a[i],0.4));
  cbrApps.Add(gencbr(nodes.Get (9),nodes.Get (8),NodeInterface.GetAddress (9),simulationTime,a[i],0.5));
  cbrApps.Add(gencbr(nodes.Get (10),nodes.Get (11),NodeInterface.GetAddress (10),simulationTime,a[i],0.6));
  cbrApps.Add(gencbr(nodes.Get (13),nodes.Get (12),NodeInterface.GetAddress (13),simulationTime,a[i],0.7));
  cbrApps.Add(gencbr(nodes.Get (14),nodes.Get (15),NodeInterface.GetAddress (14),simulationTime,a[i],0.8));
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (simulationTime+1));	  
  Simulator::Run ();

  double throughput = 0,totalthroughput = 0,totalreceivepacket=0;
  for (size_t i = 0; i < cbrApps.GetN (); ++i)
  {
	double totalPacketsThrough = DynamicCast<UdpServer> (cbrApps.Get (i))->GetReceived ();
        //uint32_t totalPacketsLost = DynamicCast<UdpServer> (cbrApps.Get (i))->GetLost ();
	throughput = totalPacketsThrough * 1472 * 8 / (simulationTime * 1000000.0); //Mbit/s
        //std::cout << "Flow " << i << std::endl;
        //std::cout << "  Rx Packets:   " << totalPacketsThrough << "\n";
        //std::cout << "  Rx Bytes:   " << totalPacketsThrough * 1472 << "\n";
	//std::cout << "  Throughput: " << throughput  << " Mbps\n";
        //std::cout << "  Lost Packet "<< totalPacketsLost <<"\n";
	totalthroughput += throughput;
	totalreceivepacket += totalPacketsThrough;
        globalreceivepacket[i] += totalPacketsThrough;
        globalthroughput[i] += throughput;
  }
  globaltotal += totalthroughput;
  //std::cout << "  totalThroughput: " << totalthroughput  << " Mbps\n";
  //std::cout << "  totalreceivepacket: " << totalreceivepacket  << std::endl;;

  Simulator::Destroy ();
  return totalthroughput;
}

int main(int argc, char **argv)
{
  double simulationTime = 0.1;//s
  double traintime =1;	
  double space = 30;
  srand(time(NULL));
  CommandLine cmd;
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("traintime", "Simulation time in seconds", traintime);
  cmd.AddValue ("space", "Simulation time in seconds", space);
  cmd.Parse (argc,argv);
  
  clock_t t1, t2;
  t1 = clock();

  int i;
  for(i=0;i<traintime;i++)
  {
  std::cout<<i<<std::endl;
  run(simulationTime,space);
  }
  for(i=0;i<nodeNum/2;i++)
  {
  std::cout<<"Flow "<<i<<" : "<<std::endl;
  std::cout<<"  AverageThroughput :   "<<globalthroughput[i]/traintime<<" Mbps "<<std::endl;
  std::cout<<"  Average RxPackets :   "<<globalreceivepacket[i]/traintime<<std::endl;
  std::cout<<"  Average RxBytes   :   "<<globalreceivepacket[i]*1472/traintime<<std::endl;
  }
  std::cout<<std::endl;
  std::cout<<"  Totalthrouhput    :   "<<globaltotal/traintime<<" Mbps "<<std::endl;

  t2 = clock();
  printf("%lf sec\n", (t2-t1)/(double)(CLOCKS_PER_SEC));
  return 0;
}
