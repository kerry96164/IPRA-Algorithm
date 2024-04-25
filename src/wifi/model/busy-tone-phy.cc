/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Sébastien Deronne <sebastien.deronne@gmail.com>
 */
#include "busy-tone-channel.h"
#include "busy-tone-phy.h"
//#include "wifi-phy.h"
#include "wifi-phy-state-helper.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "wifi-phy-tag.h"
#include "ampdu-tag.h"
#include "wifi-utils.h"
#include "frame-capture-model.h"
#define parameter 7.93315E-13
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BusytonePhy");
NS_OBJECT_ENSURE_REGISTERED (BusytonePhy);

/****************************************************************
 *       The actual BusytonePhy class
 ****************************************************************/

TypeId
BusytonePhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BusytonePhy")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddConstructor<BusytonePhy> ()
	.AddAttribute ("BusytonePower",
                   "In W.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&BusytonePhy::m_busytonetxpower),
                   MakeDoubleChecker<double> ())
	.AddAttribute ("BusytonePowerDbm",
                   "In Dbm.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&BusytonePhy::m_busytonetxpowerDbm),
                   MakeDoubleChecker<double> ())
    ;
  return tid;
}

BusytonePhy::BusytonePhy ()
{
  NS_LOG_FUNCTION (this);
 m_busytonetxpower = 0;
 m_busytonetxpowerDbm = 0;
  //NS_UNUSED (m_numberOfTransmitters);
  //NS_UNUSED (m_numberOfReceivers);
  //m_random = CreateObject<UniformRandomVariable> ();
  //m_state = CreateObject<WifiPhyStateHelper> ();//////liang用不到
}

BusytonePhy::~BusytonePhy ()
{
  NS_LOG_FUNCTION (this);
}

void
BusytonePhy::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_device = 0;
  m_mobility = 0;
  //m_state = 0;////liang
  m_channel = 0;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

///liang//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ptr<Channel>
BusytonePhy::GetChannel (void) const
{
  return m_channel;
}
///liang
void
BusytonePhy::SetChannel (const Ptr<BusytoneChannel> channel)
{
  m_channel = channel;
  m_channel->Add (this);
}
////liang
/*void
BusytonePhy::StartTx (Ptr<Packet> packet, WifiTxVector txVector, Time txDuration)
{
  NS_LOG_DEBUG ("Start transmission: signal power before antenna gain=" << GetPowerDbm (txVector.GetTxPowerLevel ()) << "dBm");
  m_channel->Send (this, packet, GetPowerDbm (txVector.GetTxPowerLevel ()) + GetTxGain (), txDuration);
}*/

void////liang/////////////////////////////要有一個上下線 等傳的時候在判斷
BusytonePhy::SetTxPowerW(double ToleranceInterference)
{//std::cout<<"settxpowerw = "<<ToleranceInterference<<" "<<Simulator::Now().GetSeconds()<<std::endl;
	m_busytonetxpower = ToleranceInterference;
}

double ////liang
BusytonePhy::GetTxPowerW(void)
{/////std::cout<<"gettxpowerw = "<<m_busytonetxpower<<" "<<Simulator::Now().GetSeconds()<<std::endl;
	return m_busytonetxpower;
}

void///////////liang///////////////////////要有一個上下線 等傳的時候在判斷
BusytonePhy::SetTxPowerDbm(double txPowerDbm)
{/////std::cout<<"settxpowerdbm = "<<txPowerDbm<<" "<<Simulator::Now().GetSeconds()<<std::endl;
	m_busytonetxpowerDbm = txPowerDbm;
///std::cout<<"gettxpowerdbm = "<<m_busytonetxpowerDbm<<" "<<Simulator::Now().GetSeconds()<<std::endl;
}

double//liang
BusytonePhy::GetTxPowerDbm(void)
{/////std::cout<<"gettxpowerdbm = "<<m_busytonetxpowerDbm<<" "<<Simulator::Now().GetSeconds()<<std::endl;
	return m_busytonetxpowerDbm;
}

void//////liang
BusytonePhy::SendBusytoneSignal(double txPowerW,Time rxDuration)
{ ////std::cout<<"duartion= "<<rxDuration.GetSeconds()<<" "<<Simulator::Now()<<std::endl;
	m_channel->Send(this,WToDbm(txPowerW),rxDuration);
}

double ////liang   在這邊轉換成W
BusytonePhy::ReceivedBusytonePowerW()
{//std::cout<<"gettxpowerW= "<<parameter/m_channel->CalculateBusytonePowerW(this)<<" "<<m_channel->CalculateBusytonePowerW(this)<<" "<<Simulator::Now().GetSeconds()<<std::endl;
        double busytonerxpower = parameter/m_channel->CalculateBusytonePowerW(this);
//std::cout<<"gettxpower= "<<busytonerxpower<<" "<<Simulator::Now().GetSeconds()<<std::endl;
	return busytonerxpower;
}

void
BusytonePhy::SetDevice (const Ptr<NetDevice> device)
{
  m_device = device;
}

Ptr<NetDevice>
BusytonePhy::GetDevice (void) const
{
  return m_device;
}

void
BusytonePhy::SetMobility (const Ptr<MobilityModel> mobility)
{
  m_mobility = mobility;
}

Ptr<MobilityModel>
BusytonePhy::GetMobility (void) const
{
  if (m_mobility != 0)
    {
      return m_mobility;
    }
  else
    {
      return m_device->GetNode ()->GetObject<MobilityModel> ();
    }
}
} //namespace ns3
