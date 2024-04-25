/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 The Boeing Company
 *
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
 *
 */

// This script configures two nodes on an 802.11b physical layer, with
// 802.11b NICs in adhoc mode, and by default, sends one packet of 1000
// (application) bytes to the other node.  The physical layer is configured
// to receive at a fixed RSS (regardless of the distance and transmit
// power); therefore, changing position of the nodes has no effect.
//
// There are a number of command-line options available to control
// the default behavior.  The list of available command-line options
// can be listed with the following command:
// ./waf --run "wifi-simple-adhoc --help"
//
// For instance, for this configuration, the physical layer will
// stop successfully receiving packets when rss drops below -97 dBm.
// To see this effect, try running:
//
// ./waf --run "wifi-simple-adhoc --rss=-97 --numPackets=20"
// ./waf --run "wifi-simple-adhoc --rss=-98 --numPackets=20"
// ./waf --run "wifi-simple-adhoc --rss=-99 --numPackets=20"
//
// Note that all ns-3 attributes (not just the ones exposed in the below
// script) can be changed at command line; see the documentation.
//
// This script can also be helpful to put the Wifi layer into verbose
// logging mode; this command will turn on all wifi logging:
//
// ./waf --run "wifi-simple-adhoc --verbose=1"
//
// When you are done, you will notice two pcap trace files in your directory.
// If you have tcpdump installed, you can try this:
//
// tcpdump -r wifi-simple-adhoc-0-0.pcap -nn -tt
//

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/config-store-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include <fstream>
#include <math.h>


using namespace ns3;
double globaltotal = 0;
double globalthroughput[12] = {0};
double globalreceivepacket[12] = {0};
double globalloss[12]={0};
double globaltotalthroughput=0;
int hidden =0;
double xspace;
double yspace;
double distance;
double  TotalSimulationTime_sec = 10;
NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhoc");
ApplicationContainer gencbr(NodeContainer server,NodeContainer client,Ipv4Address address,double simulationTime,double starttime,double pingtime)
{

  UdpServerHelper myServer (12345);
  ApplicationContainer serverApp = myServer.Install (server);
  serverApp.Start (Seconds (0.0));
  serverApp.Stop (Seconds (simulationTime+1));
  UdpClientHelper myClient (address, 12345);
  myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  myClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
  myClient.SetAttribute ("PacketSize", UintegerValue (1472));
  ApplicationContainer clientApp = myClient.Install (client);
  clientApp.Start (Seconds (1.0+starttime));
  clientApp.Stop (Seconds (simulationTime+1.0));
    
  
  UdpEchoClientHelper echoClientHelper (address, 9);
  echoClientHelper.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClientHelper.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  echoClientHelper.SetAttribute ("PacketSize", UintegerValue (10));
  ApplicationContainer pingApps;

  echoClientHelper.SetAttribute ("StartTime", TimeValue (Seconds (pingtime/10)));
  pingApps.Add (echoClientHelper.Install (client)); 
  
  return serverApp;
}

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
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("10000000"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("100000000"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiPhy::CcaMode1Threshold", DoubleValue (-82.0));
  Config::SetDefault ("ns3::WifiPhy::EnergyDetectionThreshold", DoubleValue (-79.0));
  NodeContainer c;
  
  /*sif(hidden==1)
  {
    space = 30;
    yspace = 10;
    distance =40;
  }
  else
  {
    space = 30;
    yspace = 10;
    distance = 40;
  }*/
  


  c.Create (16);
  
  
  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi= WifiHelper::Default();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
  //wifi.SetRemoteStationManager( "ns3::IdealWifiManager" );
  StringValue DataRate = VhtWifiMacHelper::DataRateForMcs (2);
  StringValue controlRate = VhtWifiMacHelper::DataRateForMcs (2);
  //std::cout<<DataRate<<"  "<<controlRate<<"\n";
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate,
                                "ControlMode", controlRate);
	
   
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiPhyHelper wifiPhy2 =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel=  YansWifiChannelHelper::Default();
  YansWifiChannelHelper wifiChannel2=  YansWifiChannelHelper::Default();
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy2.SetChannel (wifiChannel2.Create ());
  VhtWifiMacHelper wifiMac = VhtWifiMacHelper::Default ();
  VhtWifiMacHelper wifiMac2 = VhtWifiMacHelper::Default ();
  BusytoneChannelHelper bChannel = BusytoneChannelHelper::Default ();
  BusytonePhyHelper bPhy =  BusytonePhyHelper::Default ();
  bPhy.SetChannel (bChannel.Create ());
  wifi.SetBusytonePhyHelper(bPhy);
  
  
  wifiMac.SetType ("ns3::AdhocWifiMac");
  wifiMac2.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);
  wifiPhy2.Set("ChannelNumber",UintegerValue (58));
  NetDeviceContainer devices2 = wifi.Install (wifiPhy2, wifiMac2 , c);
  
  
  Ptr<YansWifiPhy> phySta,phySta2 ;

  phySta = GetYansWifiPhyPtr (devices);
  phySta2 = GetYansWifiPhyPtr (devices2);
  

	
  std::cout<<int(phySta->GetFrequency ())<<std::endl;
  std::cout<<int(phySta2->GetFrequency ())<<std::endl;
  std::cout<<int(phySta->GetFrequency ())<<"  "<<int(phySta->GetChannelWidth())<<"  "<<(phySta->GetGuardInterval())<<std::endl;
 
  // Note that with FixedRssLossModel, the positions below are not
  // used for received signal strength.
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (xspace, 0.0, 0.0));
  positionAlloc->Add (Vector (xspace+distance, 0.0, 0.0));
  positionAlloc->Add (Vector (2*xspace+distance ,0.0, 0.0));
  positionAlloc->Add (Vector (0.0, yspace, 0.0));
  positionAlloc->Add (Vector (xspace, yspace, 0.0));
  positionAlloc->Add (Vector (xspace+distance,yspace, 0.0));
  positionAlloc->Add (Vector (2*xspace+distance, yspace, 0.0));
  positionAlloc->Add (Vector (0.0, 2*yspace, 0.0));
  positionAlloc->Add (Vector (xspace, 2*yspace, 0.0));
  positionAlloc->Add (Vector (xspace+distance, 2*yspace, 0.0));
  positionAlloc->Add (Vector (2*xspace+distance, 2*yspace, 0.0));
  positionAlloc->Add (Vector (0.0, 3*yspace, 0.0));
  positionAlloc->Add (Vector (xspace, 3*yspace, 0.0));
  positionAlloc->Add (Vector (xspace+distance, 3*yspace, 0.0));
  positionAlloc->Add (Vector (2*xspace+distance,3*yspace, 0.0));
  mobility.SetPositionAllocator (positionAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);
  
  InternetStackHelper internet;
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);
  ipv4.SetBase ("10.1.10.0", "255.255.255.0");
  Ipv4InterfaceContainer j = ipv4.Assign (devices2);
  
 


  double  a[8]={0.001,0.0015,0.002,0.0025,0.003,0.0035,0.004,0.0045};
  double  temp;
  int r1 ,r2;
  srand(time(NULL));
  for(int i=0;i<4;++i)
  {
    r1 = rand() % 8;
    //std::cout<<c<<std::endl;
    r2 = rand() % 8;
    //std::cout<<d<<std::endl;
    temp = a[r1];
    a[r1] = a[r2];
    a[r2] = temp;	
  }
  ApplicationContainer cbrApps;


  if(hidden == 1)
  {
     cbrApps.Add(gencbr(c.Get (1),c.Get (0),i.GetAddress (1),TotalSimulationTime_sec,a[0],0.1));
    cbrApps.Add(gencbr(c.Get (2),c.Get (3),i.GetAddress (2),TotalSimulationTime_sec,a[1],0.2));
    cbrApps.Add(gencbr(c.Get (5),c.Get (4),i.GetAddress (5),TotalSimulationTime_sec,a[2],0.3));
    cbrApps.Add(gencbr(c.Get (6),c.Get (7),i.GetAddress (6),TotalSimulationTime_sec,a[3],0.4));
    cbrApps.Add(gencbr(c.Get (9),c.Get (8),i.GetAddress (9),TotalSimulationTime_sec,a[4],0.5));
    cbrApps.Add(gencbr(c.Get (10),c.Get (11),i.GetAddress (10),TotalSimulationTime_sec,a[5],0.6));
    cbrApps.Add(gencbr(c.Get (13),c.Get (12),i.GetAddress (13),TotalSimulationTime_sec,a[6],0.7));
    cbrApps.Add(gencbr(c.Get (14),c.Get (15),i.GetAddress (14),TotalSimulationTime_sec,a[7],0.8));
  }
  else
  {
    cbrApps.Add(gencbr(c.Get (0),c.Get (1),i.GetAddress (0),TotalSimulationTime_sec,a[0],0.1));
    cbrApps.Add(gencbr(c.Get (3),c.Get (2),i.GetAddress (3),TotalSimulationTime_sec,a[1],0.2));
    cbrApps.Add(gencbr(c.Get (4),c.Get (5),i.GetAddress (4),TotalSimulationTime_sec,a[2],0.3));
    cbrApps.Add(gencbr(c.Get (7),c.Get (6),i.GetAddress (7),TotalSimulationTime_sec,a[3],0.4));
    cbrApps.Add(gencbr(c.Get (8),c.Get (9),i.GetAddress (8),TotalSimulationTime_sec,a[4],0.5));
    cbrApps.Add(gencbr(c.Get (11),c.Get (10),i.GetAddress (11),TotalSimulationTime_sec,a[5],0.6));
    cbrApps.Add(gencbr(c.Get (12),c.Get (13),i.GetAddress (12),TotalSimulationTime_sec,a[6],0.7));
    cbrApps.Add(gencbr(c.Get (15),c.Get (14),i.GetAddress (15),TotalSimulationTime_sec,a[7],0.8));
  }
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  /*cbrApps.Add(gencbr(c.Get (0),c.Get (1),i.GetAddress (0),TotalSimulationTime_sec,a[0],0.1));
  cbrApps.Add(gencbr(c.Get (3),c.Get (2),i.GetAddress (3),TotalSimulationTime_sec,a[1],0.2));
  cbrApps.Add(gencbr(c.Get (4),c.Get (5),i.GetAddress (4),TotalSimulationTime_sec,a[2],0.3));
  cbrApps.Add(gencbr(c.Get (7),c.Get (6),i.GetAddress (7),TotalSimulationTime_sec,a[3],0.4));
  cbrApps.Add(gencbr(c.Get (8),c.Get (9),i.GetAddress (8),TotalSimulationTime_sec,a[4],0.5));
  cbrApps.Add(gencbr(c.Get (11),c.Get (10),i.GetAddress (11),TotalSimulationTime_sec,a[5],0.6));
  cbrApps.Add(gencbr(c.Get (12),c.Get (13),i.GetAddress (12),TotalSimulationTime_sec,a[6],0.7));
  cbrApps.Add(gencbr(c.Get (15),c.Get (14),i.GetAddress (15),TotalSimulationTime_sec,a[7],0.8));*/
  /*cbrApps.Add(gencbr(c.Get (1),c.Get (0),i.GetAddress (1),TotalSimulationTime_sec,a[0],0.1));
  cbrApps.Add(gencbr(c.Get (2),c.Get (3),i.GetAddress (2),TotalSimulationTime_sec,a[1],0.2));
  cbrApps.Add(gencbr(c.Get (5),c.Get (4),i.GetAddress (5),TotalSimulationTime_sec,a[2],0.3));
  cbrApps.Add(gencbr(c.Get (6),c.Get (7),i.GetAddress (6),TotalSimulationTime_sec,a[3],0.4));
  cbrApps.Add(gencbr(c.Get (9),c.Get (8),i.GetAddress (9),TotalSimulationTime_sec,a[4],0.5));
  cbrApps.Add(gencbr(c.Get (10),c.Get (11),i.GetAddress (10),TotalSimulationTime_sec,a[5],0.6));
  cbrApps.Add(gencbr(c.Get (13),c.Get (12),i.GetAddress (13),TotalSimulationTime_sec,a[6],0.7));
  cbrApps.Add(gencbr(c.Get (14),c.Get (15),i.GetAddress (14),TotalSimulationTime_sec,a[7],0.8));*/
    




    // Start Simulation

    Simulator::Stop( Seconds( TotalSimulationTime_sec+2 ) );
    Simulator::Run ();
    /*monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
    int num =0;
    for( std::map<FlowId , FlowMonitor::FlowStats>::const_iterator counter = stats.begin () ; counter != stats.end () ; ++counter )
    {
      Ipv4FlowClassifier::FiveTuple ft = classifier->FindFlow( counter->first );
      if( ft.destinationPort == 12345  ){*/
            //globalthroughput[num] +=(double)( ( counter->second.rxPackets ) * 8 *1472 )/ /*TotalSimulationTime_sec*1000000);*/(double)((counter->second.timeLastRxPacket.GetSeconds() -counter->second.timeFirstTxPacket.GetSeconds())*1000000 );
            //globaltotal  +=(double)( ( counter->second.rxPackets ));
            //globalreceivepacket[num] +=( counter->second.rxPackets );
            //globalloss[num]+=(double)(counter->second.txPackets-counter->second.rxPackets  )/counter->second.txPackets ;
            //globaltotalthroughput += (double)( ( counter->second.rxPackets ) * 8 *1472 )/ /*TotalSimulationTime_sec*1000000);*/(double)((counter->second.timeLastRxPacket.GetSeconds() -counter->second.timeFirstTxPacket.GetSeconds())*1000000 );
      //}
      //num++;
      
    //}
    for (size_t i = 0; i < cbrApps.GetN (); ++i)
    {
          uint32_t totalPacketsThrough = DynamicCast<UdpServer> (cbrApps.Get (i))->GetReceived ();
          globaltotalthroughput += totalPacketsThrough * 1472 * 8 / (TotalSimulationTime_sec * 1000000.0);
          globalreceivepacket[i] += totalPacketsThrough;
          globalreceivepacket[i] += totalPacketsThrough;
          globalthroughput[i] += totalPacketsThrough * 1472 * 8 / (TotalSimulationTime_sec * 1000000.0);
          globalloss[i]+=DynamicCast<UdpServer> (cbrApps.Get (i))->GetLost ();
    }
  //std::cout<<num<<std::endl;

    Simulator::Destroy ();
}
int main (int argc, char *argv[])
{
   int testtime = 10;
  CommandLine cmd;
  cmd.AddValue("hidden","hidden or not",hidden);
  cmd.AddValue("xspace","x space",xspace);
  cmd.AddValue("yspace","y space",yspace);
  cmd.AddValue("testtime", "traintime", testtime);
  cmd.AddValue("simulationTime", "Simulation time in seconds", TotalSimulationTime_sec);
  cmd.AddValue("distance","receiver distance",distance);
  cmd.Parse (argc,argv);
  

  for(int counter =0;counter<testtime;counter++){
      runthesimulation();
  }
  for(int counter =0;counter<8;counter++){
    std::cout<<"Flow "<<counter<<" : "<<std::endl;
    std::cout<<"  AverageThroughput :   "<<globalthroughput[counter]/testtime<<" Mbps "<<std::endl;
    std::cout<<"  Average RxPackets :   "<<globalreceivepacket[counter]/testtime<<std::endl;
    std::cout<<"  Average RxBytes   :   "<<globalreceivepacket[counter]/testtime*1472<<std::endl;
    std::cout<<"  loss              :   "<<globalloss[counter]/testtime<<std::endl;
  }
  std::cout<<"total throughput : "<<globaltotalthroughput/testtime<<std::endl;
  return 0;
}
