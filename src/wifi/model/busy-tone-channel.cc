/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
 * Author: Mathieu Lacage, <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "busy-tone-channel.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "wifi-utils.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BusytoneChannel");

NS_OBJECT_ENSURE_REGISTERED (BusytoneChannel);

TypeId
BusytoneChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BusytoneChannel")
    .SetParent<Channel> ()
    .SetGroupName ("Wifi")
    .AddConstructor<BusytoneChannel> ()
    .AddAttribute ("PropagationLossModel", "A pointer to the propagation loss model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&BusytoneChannel::m_loss),
                   MakePointerChecker<PropagationLossModel> ())
    /*.AddAttribute ("PropagationDelayModel", "A pointer to the propagation delay model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&BusytoneChannel::m_delay),
                   MakePointerChecker<PropagationDelayModel> ())////check*/
  ;
  return tid;
}

BusytoneChannel::BusytoneChannel ()
{
  NS_LOG_FUNCTION (this);
}

BusytoneChannel::~BusytoneChannel ()
{
  NS_LOG_FUNCTION (this);
  m_phyList.clear ();
}

void
BusytoneChannel::SetPropagationLossModel (const Ptr<PropagationLossModel> loss)
{
  m_loss = loss;
}

/*void////check
BusytoneChannel::SetPropagationDelayModel (const Ptr<PropagationDelayModel> delay)
{
  m_delay = delay;
}*/

void////liang
BusytoneChannel::Send(Ptr<BusytonePhy> sender,double txPowerDbm,Time duration) const
{
	sender->SetTxPowerDbm(txPowerDbm);
	Simulator::Schedule(duration, &BusytoneChannel::Turnoff, this, sender);
}

void ////liang
BusytoneChannel::Turnoff(Ptr<BusytonePhy> sender)const
{                                               /////dbm =0 >>W =0.001 too strong
	sender->SetTxPowerDbm(-120);/////////////shut down but can't not be zero
}

double //////liang
BusytoneChannel::CalculateBusytonePowerW(Ptr<BusytonePhy> receiver)
{
	double rxPowerW = 0;
	Ptr<MobilityModel> receiverMobility = receiver->GetMobility ();
	for (PhyList::const_iterator i = m_phyList.begin (); i != m_phyList.end (); i++)
    {
      if (receiver != (*i))
        {
          Ptr<MobilityModel> senderMobility = (*i)->GetMobility ()->GetObject<MobilityModel> ();
          double txPowerDbm = (*i)->GetTxPowerDbm();
          //std::cout<<"txpowerdbm = "<< txPowerDbm<<std::endl;
          double rxPowerDbm = m_loss->CalcRxPower (txPowerDbm, senderMobility, receiverMobility);
          //std::cout<<"rxpowerdbm = "<< rxPowerDbm<<" "<<std::endl;;
          rxPowerW += DbmToW(rxPowerDbm);
         
        }
    }

        //std::cout<<"totalrxpowerdbm = "<< WToDbm(rxPowerW)<<" "<<rxPowerW<<" "<<Simulator::Now().GetSeconds()<<std::endl;
	return rxPowerW;
}


/*
void
BusytoneChannel::Send (Ptr<BusytonePhy> sender,double txPowerDbm, Time duration) const
{
  NS_LOG_FUNCTION (this << sender << packet << txPowerDbm << duration.GetSeconds ());
  Ptr<MobilityModel> senderMobility = sender->GetMobility ();
  NS_ASSERT (senderMobility != 0);
  for (PhyList::const_iterator i = m_phyList.begin (); i != m_phyList.end (); i++)
    {
      if (sender != (*i))
        {
          //For now don't account for inter channel interference nor channel bonding
          if ((*i)->GetChannelNumber () != sender->GetChannelNumber ())
            {
              continue;
            }

          Ptr<MobilityModel> receiverMobility = (*i)->GetMobility ()->GetObject<MobilityModel> ();
          Time delay = m_delay->GetDelay (senderMobility, receiverMobility);
          double rxPowerDbm = m_loss->CalcRxPower (txPowerDbm, senderMobility, receiverMobility);
          NS_LOG_DEBUG ("propagation: txPower=" << txPowerDbm << "dbm, rxPower=" << rxPowerDbm << "dbm, " <<
                        "distance=" << senderMobility->GetDistanceFrom (receiverMobility) << "m, delay=" << delay);
          Ptr<Packet> copy = packet->Copy ();
          Ptr<NetDevice> dstNetDevice = (*i)->GetDevice ();
          uint32_t dstNode;
          if (dstNetDevice == 0)
            {
              dstNode = 0xffffffff;
            }
          else
            {
              dstNode = dstNetDevice->GetNode ()->GetId ();
            }

          Simulator::ScheduleWithContext (dstNode,
                                          delay, &BusytoneChannel::Receive,
                                          (*i), copy, rxPowerDbm, duration);
        }
    }
}
*/
/*
void
BusytoneChannel::Receive (Ptr<BusytonePhy> phy, Ptr<Packet> packet, double rxPowerDbm, Time duration)
{
  NS_LOG_FUNCTION (phy << packet << rxPowerDbm << duration.GetSeconds ());
  phy->StartReceivePreambleAndHeader (packet, DbmToW (rxPowerDbm + phy->GetRxGain ()), duration);
}
*/

uint32_t
BusytoneChannel::GetNDevices (void) const
{
  return m_phyList.size ();
}

Ptr<NetDevice>
BusytoneChannel::GetDevice (uint32_t i) const
{
  return m_phyList[i]->GetDevice ()->GetObject<NetDevice> ();
}

void
BusytoneChannel::Add (Ptr<BusytonePhy> phy)
{
  m_phyList.push_back (phy);
}

/*int64_t
BusytoneChannel::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  int64_t currentStream = stream;
  currentStream += m_loss->AssignStreams (stream);
  return (currentStream - stream);
}*/

} //namespace ns3
