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


using namespace ns3;

double globaltotal = 0;
double globalthroughput[8] = {0};
double globalreceivepacket[8] = {0};

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

double run(double nodeNum,double simulationTime,double toposize)
{
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (999999));
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", UintegerValue (999999));
    std::string phyMode ("DsssRate11Mbps");
  NodeContainer nodes;
  nodes.Create (nodeNum);
  
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  BusytoneChannelHelper bChannel = BusytoneChannelHelper::Default ();
  BusytonePhyHelper bPhy =  BusytonePhyHelper::Default ();
  bPhy.SetChannel (bChannel.Create ());
  //WifiHelper wifi = WifiHelper::Default ();
  WifiHelper wifi ;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
  
  StringValue DataRate = VhtWifiMacHelper::DataRateForMcs (3);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (phyMode),
                                "ControlMode", StringValue (phyMode));
  
  mac.SetType ("ns3::AdhocWifiMac"); // use ad-hoc MAC
  wifi.SetBusytonePhyHelper(bPhy);
  NetDeviceContainer devices = wifi.Install (phy, mac, nodes);
  

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (toposize / 3),
                                 "DeltaY", DoubleValue (toposize / 3),
                                 "GridWidth", UintegerValue (4),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  
  InternetStackHelper internet;
  internet.Install (nodes);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer NodeInterface = ipv4.Assign (devices);
  //double a[8]={0.001,0.001,0.001,0.001,0.001,0.001,0.001,0.001};
  double a[8]={0.001,0.0015,0.002,0.0025,0.003,0.0035,0.004,0.0045};
  double  temp;
int c ,d;
  for(int i=0;i<8;++i)
  {
	c = rand() % 8;
 	//std::cout<<c<<std::endl;
        d = rand() % 8;
	//std::cout<<d<<std::endl;
	temp = a[c];
	a[c] = a[d];
	a[d] = temp;	
  }


  for(int i=0;i<8;i++)
 {
	//std::cout<<a[i]<<std::endl;
 }
  
  ApplicationContainer cbrApps;
  cbrApps.Add(gencbr(nodes.Get (0),nodes.Get (1),NodeInterface.GetAddress (0),simulationTime,a[0],0.001));
  cbrApps.Add(gencbr(nodes.Get (3),nodes.Get (2),NodeInterface.GetAddress (3),simulationTime,a[1],0.002));
  cbrApps.Add(gencbr(nodes.Get (4),nodes.Get (5),NodeInterface.GetAddress (4),simulationTime,a[2],0.003));
  cbrApps.Add(gencbr(nodes.Get (7),nodes.Get (6),NodeInterface.GetAddress (7),simulationTime,a[3],0.004));
  cbrApps.Add(gencbr(nodes.Get (8),nodes.Get (9),NodeInterface.GetAddress (8),simulationTime,a[4],0.0045));
  cbrApps.Add(gencbr(nodes.Get (11),nodes.Get (10),NodeInterface.GetAddress (11),simulationTime,a[5],0.0035));
  cbrApps.Add(gencbr(nodes.Get (12),nodes.Get (13),NodeInterface.GetAddress (12),simulationTime,a[6],0.0025));
  cbrApps.Add(gencbr(nodes.Get (15),nodes.Get (14),NodeInterface.GetAddress (15),simulationTime,a[7],0.0015));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  Simulator::Stop (Seconds (simulationTime+1));	  
  Simulator::Run ();


  // 10. Print per flow statistics
  double throughput = 0,totalthroughput = 0,totalreceivepacket;
  for (size_t i = 0; i < cbrApps.GetN (); ++i)
  {
	uint32_t totalPacketsThrough = DynamicCast<UdpServer> (cbrApps.Get (i))->GetReceived ();
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
  //std::cout << "  totaloacket: " << totalreceivepacket  << std::endl;;

  // 11. Cleanup
  Simulator::Destroy ();
  return totalthroughput;
}

void CwTrace(uint32_t before ,uint32_t change)
{
std::cout<<before<<" "<<change<<std::endl;
}

int main(int argc, char **argv)
{
    std::cout<<"change random"<<std::endl;
  double nodeNum = 40;
  double simulationTime = 0.5;//s
  double toposize = 40;
int traintime =1;
  srand(time(NULL));	
  CommandLine cmd;
  cmd.AddValue ("nodeNum", "node number", nodeNum);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("toposize", "Simulation time in seconds", toposize);
  cmd.AddValue ("traintime", "Simulation time in seconds", traintime);
  cmd.Parse (argc,argv);
  
 clock_t t1, t2;
  t1 = clock();

  int i;
  for(i=0;i<traintime;i++)
  {
  std::cout<<i<<std::endl;
  run(16,simulationTime,toposize);
  }
  for(i=0;i<8;i++)
  {
  std::cout<<"Flow "<<i<<" : "<<std::endl;
  std::cout<<"  AverageThroughput :   "<<globalthroughput[i]/traintime<<" Mbps "<<std::endl;
  std::cout<<"  Average RxPackets :   "<<globalreceivepacket[i]/traintime<<std::endl;
  std::cout<<"  Average RxBytes   :   "<<globalreceivepacket[i]/traintime*1472<<std::endl;
  }
  std::cout<<std::endl;
  std::cout<<"  Totalthrouhput    :   "<<globaltotal/traintime<<" Mbps "<<std::endl;

  t2 = clock();
  printf("%lf sec\n", (t2-t1)/(double)(CLOCKS_PER_SEC));



  return 0;
}
