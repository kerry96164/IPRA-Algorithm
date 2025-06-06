/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006, 2009 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
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
 *          Mirko Banchi <mk.banchi@gmail.com>
 */

#include "adhoc-wifi-mac.h"
#include "ns3/log.h"
#include "mac-low.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AdhocWifiMac");

NS_OBJECT_ENSURE_REGISTERED (AdhocWifiMac);

TypeId
AdhocWifiMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AdhocWifiMac")
    .SetParent<RegularWifiMac> ()
    .SetGroupName ("Wifi")
    .AddConstructor<AdhocWifiMac> ()
    .AddAttribute ("PositionPacketInterval",
                   "Delay between two position control packet",
                   TimeValue (MicroSeconds (400000)),
                   MakeTimeAccessor (&AdhocWifiMac::GetPositionPacketInterval,
                                     &AdhocWifiMac::SetPositionPacketInterval),
                   MakeTimeChecker ())
  ;
  return tid;
}

AdhocWifiMac::AdhocWifiMac () // Jonathan
{
  NS_LOG_FUNCTION (this);
  //Let the lower layers know that we are acting in an IBSS
  SetTypeOfStation (ADHOC_STA);
}

AdhocWifiMac::~AdhocWifiMac ()
{
  NS_LOG_FUNCTION (this);
}

void
AdhocWifiMac::SetAddress (Mac48Address address)
{
  NS_LOG_FUNCTION (this << address);
  //In an IBSS, the BSSID is supposed to be generated per Section
  //11.1.3 of IEEE 802.11. We don't currently do this - instead we
  //make an IBSS STA a bit like an AP, with the BSSID for frames
  //transmitted by each STA set to that STA's address.
  //
  //This is why we're overriding this method.
  RegularWifiMac::SetAddress (address);
  RegularWifiMac::SetBssid (address);
}

void // Jonathan
AdhocWifiMac::DoInitialize (void)
{
  NS_LOG_FUNCTION(this);

  // Stagger the transmision of first control packet
  // static Time delay;
  // Time IntervalBetweenDevices = MicroSeconds(1000);
  // Simulator::Schedule (delay, &AdhocWifiMac::BroadcastPosition, this);
  // delay += IntervalBetweenDevices;
  // TODO mac address larger than 00:00:00:00:00:14 don't need to broadcast
  if(m_phy->GetDevice()->GetIfIndex()==0)
  {
    Simulator::ScheduleNow (&AdhocWifiMac::BroadcastPosition, this);
  }

}

void
AdhocWifiMac::Enqueue (Ptr<const Packet> packet, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << to);
  if (m_stationManager->IsBrandNew (to))
    {
      //In ad hoc mode, we assume that every destination supports all
      //the rates we support.
      if (m_htSupported || m_vhtSupported || m_heSupported || m_s1gSupported) //802.11ah
        {
          m_stationManager->AddAllSupportedMcs (to);
        }
      if (m_htSupported)
        {
          m_stationManager->AddStationHtCapabilities (to, GetHtCapabilities());
        }
      if (m_vhtSupported)
        {
          m_stationManager->AddStationVhtCapabilities (to, GetVhtCapabilities());
        }
      if (m_s1gSupported)
        {
          m_stationManager->AddStationS1gCapabilities (to, GetS1gCapabilities());
        }
      if (m_heSupported)
        {
          m_stationManager->AddStationHeCapabilities (to, GetHeCapabilities());
        }
      m_stationManager->AddAllSupportedModes (to);
      m_stationManager->RecordDisassociated (to);
    }

  WifiMacHeader hdr;

  //If we are not a QoS STA then we definitely want to use AC_BE to
  //transmit the packet. A TID of zero will map to AC_BE (through \c
  //QosUtilsMapTidToAc()), so we use that as our default here.
  uint8_t tid = 0;

  //For now, a STA that supports QoS does not support non-QoS
  //associations, and vice versa. In future the STA model should fall
  //back to non-QoS if talking to a peer that is also non-QoS. At
  //that point there will need to be per-station QoS state maintained
  //by the association state machine, and consulted here.
  if (m_qosSupported)
    {
      hdr.SetType (WIFI_MAC_QOSDATA);
      hdr.SetQosAckPolicy (WifiMacHeader::NORMAL_ACK);
      hdr.SetQosNoEosp ();
      hdr.SetQosNoAmsdu ();
      //Transmission of multiple frames in the same TXOP is not
      //supported for now
      hdr.SetQosTxopLimit (0);

      //Fill in the QoS control field in the MAC header
      tid = QosUtilsGetTidForPacket (packet);
      //Any value greater than 7 is invalid and likely indicates that
      //the packet had no QoS tag, so we revert to zero, which will
      //mean that AC_BE is used.
      if (tid > 7)
        {
          tid = 0;
        }
      hdr.SetQosTid (tid);
    }
  else
    {
      hdr.SetTypeData ();
    }

  if (m_htSupported || m_vhtSupported || m_heSupported || m_s1gSupported) //802.11ah
    {
      hdr.SetNoOrder ();
    }
  hdr.SetAddr1 (to);
  hdr.SetAddr2 (m_low->GetAddress ());
  hdr.SetAddr3 (GetBssid ());
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();

  if (m_qosSupported)
    {
      //Sanity check that the TID is valid
      NS_ASSERT (tid < 8);
      m_edca[QosUtilsMapTidToAc (tid)]->Queue (packet, hdr);
    }
  else
    {
      m_dca->Queue (packet, hdr);
    }
}

void
AdhocWifiMac::SetPositionPacketInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_positionpacketinterval = interval;
}

Time
AdhocWifiMac::GetPositionPacketInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_positionpacketinterval;
}

void // Jonathan
AdhocWifiMac::BroadcastPosition(void)
{
  uint8_t tid = 0;

  WifiMacHeader hdr;
  // reference enqueue in this .cc or how mac low make a packet
  hdr.SetType (WIFI_MAC_CTL_DISTANCE);
  hdr.SetAddr1 (Mac48Address::GetBroadcast ());
  hdr.SetAddr2 (GetAddress ());
  // hdr.SetAddr3 (GetAddress ());
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();
  hdr.SetNoOrder ();
  hdr.SetNoMoreFragments ();
  hdr.SetNoRetry ();
  // hdr.Settxpower(uint16_t(20)); // if you want this to be in your control packet, you have to edit wifi-mac-header

  // Insert the position into the packet content
  Ptr<Node> node = m_phy->GetDevice()->GetNode ();

  // uint32_t nodeid = node->GetId();
  // NS_LOG_UNCOND(nodeid);

  Vector Position = node->GetObject<MobilityModel>()->GetPosition();
  // NS_LOG_UNCOND(Position.x << " " << Position.y << " " << Position.z);  
  hdr.SetPosition(Position);


  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader(hdr);


  if (m_qosSupported)
    {
      tid = QosUtilsGetTidForPacket (packet);
      if (tid > 7)
      {
        tid = 7;
      }
      m_edca[QosUtilsMapTidToAc (tid)]->Queue (packet, hdr);
      NS_LOG_DEBUG("qos supported.");
    }
  else
    {
      m_dca->Queue (packet, hdr);
    }

  // Simulator::Schedule (m_positionpacketinterval, &AdhocWifiMac::BroadcastPosition, this); //Uncomment this if you want to broadcast periodically
}

void
AdhocWifiMac::SetLinkUpCallback (Callback<void> linkUp)
{
  NS_LOG_FUNCTION (this << &linkUp);
  RegularWifiMac::SetLinkUpCallback (linkUp);

  //The approach taken here is that, from the point of view of a STA
  //in IBSS mode, the link is always up, so we immediately invoke the
  //callback if one is set
  linkUp ();
}

void
AdhocWifiMac::Receive (Ptr<Packet> packet, const WifiMacHeader *hdr)
{
  NS_LOG_FUNCTION (this << packet << hdr);
  NS_ASSERT (!hdr->IsCtl ());
  Mac48Address from = hdr->GetAddr2 ();
  Mac48Address to = hdr->GetAddr1 ();
  if (m_stationManager->IsBrandNew (from))
    {
      //In ad hoc mode, we assume that every destination supports all
      //the rates we support.
      if (m_htSupported || m_vhtSupported || m_heSupported)
        {
          m_stationManager->AddAllSupportedMcs (from);
          m_stationManager->AddStationHtCapabilities (from, GetHtCapabilities());
        }
      if (m_htSupported)
        {
          m_stationManager->AddStationHtCapabilities (from, GetHtCapabilities());
        }
      if (m_vhtSupported)
        {
          m_stationManager->AddStationVhtCapabilities (from, GetVhtCapabilities());
        }
      if (m_s1gSupported)
        {
          m_stationManager->AddStationS1gCapabilities (from, GetS1gCapabilities());
        }
      if (m_heSupported)
        {
          m_stationManager->AddStationHeCapabilities (from, GetHeCapabilities());
        }
      m_stationManager->AddAllSupportedModes (from);
      m_stationManager->RecordDisassociated (from);
    }
  if (hdr->IsData ())
    {
      if (hdr->IsQosData () && hdr->IsQosAmsdu ())
        {
          NS_LOG_DEBUG ("Received A-MSDU from" << from);
          DeaggregateAmsduAndForward (packet, hdr);
        }
      else
        {
          ForwardUp (packet, from, to);
        }
      return;
    }

  //Invoke the receive handler of our parent class to deal with any
  //other frames. Specifically, this will handle Block Ack-related
  //Management Action frames.
  RegularWifiMac::Receive (packet, hdr);
}

} //namespace ns3
