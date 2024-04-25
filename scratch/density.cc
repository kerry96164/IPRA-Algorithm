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
double globaltotalreceivepackets = 0;
double globalthroughput[200] = {0};
double globalreceivepacket[200] = {0};

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

double run(double simulationTime,double range,double radius,int nodeNum,double cca)
{
  double a[nodeNum/2] ;
  int nodearray[nodeNum];
  double temp;
  int i;
  int c,d;
  double x,y,x1,y1;
  double r=radius;
  r=r*0.3;
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (999999));
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", UintegerValue (999999));
  Config::SetDefault ("ns3::WifiPhy::CcaMode1Threshold", DoubleValue (-82.0));
  Config::SetDefault ("ns3::WifiPhy::EnergyDetectionThreshold", DoubleValue (-79.0));

  NodeContainer nodes;
  nodes.Create (nodeNum);
  
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  YansWifiPhyHelper phy2 = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  phy2.SetChannel (channel2.Create ());
  BusytoneChannelHelper bChannel = BusytoneChannelHelper::Default ();
  BusytonePhyHelper bPhy =  BusytonePhyHelper::Default ();
  bPhy.SetChannel (bChannel.Create ());
  WifiHelper wifi ;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
  VhtWifiMacHelper mac = VhtWifiMacHelper::Default ();
  VhtWifiMacHelper mac2 = VhtWifiMacHelper::Default ();
  
  StringValue DataRate = VhtWifiMacHelper::DataRateForMcs (2);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate,
                                "ControlMode", DataRate);
  
  mac.SetType ("ns3::AdhocWifiMac"); // use ad-hoc MAC
   mac2.SetType ("ns3::AdhocWifiMac"); // use ad-hoc MAC
  wifi.SetBusytonePhyHelper(bPhy);
  NetDeviceContainer devices = wifi.Install (phy, mac, nodes);
  phy2.Set("ChannelNumber",UintegerValue (58));
  NetDeviceContainer devices2 = wifi.Install (phy2, mac2, nodes);
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
 int area_size = range/25;
 int area_y = range/(nodeNum/50);
  for(i=0;i<nodeNum/2;i++)
  {
    int count = i/(nodeNum/50);
    int upperbound_x = area_size*(count+1);
    int lowerbound_x = area_size*count;
    int upperbound_y=range,lowerbound_y=0;

    lowerbound_y = i%(nodeNum/50)*area_y;
    upperbound_y = (i%(nodeNum/50)+1)*area_y;

    x = double(upperbound_x - lowerbound_x) * rand() / (RAND_MAX) + lowerbound_x;
    y = double(upperbound_y - lowerbound_y) * rand() / (RAND_MAX) + lowerbound_y;
    do
    {
      /* code */
      x1 = (2*r) * rand() / (RAND_MAX) + x - r;
      temp = sqrt(r*r-(x1-x)*(x1-x));
      y1 = (2*temp) * rand() / (RAND_MAX) + y-temp;
    }  while ((x1<0 || x1>range) || (y1<0 || y1>range ) || ((x1-x)*(x1-x)+(y1-y)*(y1-y) < (r/2)*(r/2)));
    
   
    ///std::cout<<x<<" "<<y<<" "<<x1<<" "<<y1<<" "<<sqrt((y1-y)*(y1-y) + (x1-x)*(x1-x))<<std::endl;
    positionAlloc->Add (Vector (x, y, 0.0));
    positionAlloc->Add (Vector (x1, y1, 0.0));
  }
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  
  InternetStackHelper internet;
  internet.Install (nodes);
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer NodeInterface = ipv4.Assign (devices);
  ipv4.SetBase ("10.1.10.0", "255.255.255.0");
  Ipv4InterfaceContainer j = ipv4.Assign (devices2);
  
  for(i=0;i<nodeNum/2;++i)
  {
	a[i] = 0.001 + 0.0005*i;
  }
  for(i=0;i<nodeNum/2;++i)
  {
	c = rand() % (nodeNum/2);
        d = rand() % (nodeNum/2);
	temp = a[c];
	a[c] = a[d];
	a[d] = temp;	
  }

  for(i=0;i<nodeNum;++i)
  {
	nodearray[i] = i;
  }

  ApplicationContainer cbrApps;
  
  for(i=0;i<nodeNum;)
  {
    cbrApps.Add(gencbr(nodes.Get (nodearray[i]),nodes.Get (nodearray[i+1]),NodeInterface.GetAddress (nodearray[i]),simulationTime,a[i],(i+1)/10));
	  i = i+2;
  }  

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
  globaltotalreceivepackets += totalreceivepacket;
  globaltotal += totalthroughput;
  //std::cout << "  totalThroughput: " << totalthroughput  << " Mbps\n";
  std::cout << "  totalreceivepacket: " << totalreceivepacket  << std::endl;;

  Simulator::Destroy ();
  return totalthroughput;
}

int main(int argc, char **argv)
{
  double simulationTime = 1;//s
  double traintime =1,range=3000.0,radius=100.0;	
  int nodeNum = 200;
  double cca = -82;
  srand(time(NULL));
  CommandLine cmd;
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("traintime", "Simulation time in seconds", traintime);
  cmd.AddValue ("range", "Simulation time in seconds", range);
  cmd.AddValue ("radius", "Simulation time in seconds", radius);
  cmd.AddValue ("nodeNum", "Simulation time in seconds", nodeNum);
  cmd.AddValue ("cca", "Simulation time in seconds", cca);
  cmd.Parse (argc,argv);
  
  clock_t t1, t2;
  t1 = clock();

  int i;
  for(i=0;i<traintime;i++)
  {
  std::cout<<i<<std::endl;
  run(simulationTime,range,radius,nodeNum,cca);
  }
  /*for(i=0;i<nodeNum/2;i++)
  {
  std::cout<<"Flow "<<i<<" : "<<std::endl;
  std::cout<<"  AverageThroughput :   "<<globalthroughput[i]/traintime<<" Mbps "<<std::endl;
  std::cout<<"  Average RxPackets :   "<<globalreceivepacket[i]/traintime<<std::endl;
  std::cout<<"  Average RxBytes   :   "<<globalreceivepacket[i]*1472/traintime<<std::endl;
  }*/
  std::cout<<std::endl;
  std::cout<<"  Totalthrouhput    :   "<<globaltotal/traintime<<" Mbps "<<std::endl;
    std::cout<<"  TotalRxpackets    :   "<<globaltotalreceivepackets/traintime<<std::endl;
  t2 = clock();
  printf("%lf sec\n", (t2-t1)/(double)(CLOCKS_PER_SEC));
  return 0;
}
