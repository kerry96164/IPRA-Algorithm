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

#ifndef BUSY_TONE_CHANNEL_H
#define BUSY_TONE_CHANNEL_H

#include "ns3/channel.h"
#include "busy-tone-phy.h"

namespace ns3 {

class NetDevice;
class PropagationLossModel;
class PropagationDelayModel;
class BusytonePhy;/////liang

/**
 * \brief a channel to interconnect ns3::BusytonePhy objects.
 * \ingroup wifi
 *
 * This class is expected to be used in tandem with the ns3::BusytonePhy
 * class and supports an ns3::PropagationLossModel and an 
 * ns3::PropagationDelayModel.  By default, no propagation models are set; 
 * it is the caller's responsibility to set them before using the channel.
 */
class BusytoneChannel : public Channel
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  BusytoneChannel ();
  virtual ~BusytoneChannel ();

  //inherited from Channel.
  virtual uint32_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

  /**
   * Adds the given BusytonePhy to the PHY list
   *
   * \param phy the BusytonePhy to be added to the PHY list
   */
  void Add (Ptr<BusytonePhy> phy);

  /**
   * \param loss the new propagation loss model.
   */
  void SetPropagationLossModel (const Ptr<PropagationLossModel> loss);
  /**
   * \param delay the new propagation delay model.
   */
  //void SetPropagationDelayModel (const Ptr<PropagationDelayModel> delay);//////check

  /**
   * \param sender the phy object from which the packet is originating.
   * \param packet the packet to send
   * \param txPowerDbm the tx power associated to the packet, in dBm
   * \param duration the transmission duration associated with the packet
   *
   * This method should not be invoked by normal users. It is
   * currently invoked only from BusytonePhy::StartTx.  The channel
   * attempts to deliver the packet to all other BusytonePhy objects
   * on the channel (except for the sender).
   */
  void Send (Ptr<BusytonePhy> sender,double txPowerDbm, Time duration) const;///liang

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   *
   * \return the number of stream indices assigned by this model
   */
  //int64_t AssignStreams (int64_t stream);
  void Turnoff(Ptr<BusytonePhy>sender)const;//////liang
  double CalculateBusytonePowerW(Ptr<BusytonePhy>receiver);////liang

private:
  /**
   * A vector of pointers to BusytonePhy.
   */
  typedef std::vector<Ptr<BusytonePhy> > PhyList;

  /**
   * This method is scheduled by Send for each associated BusytonePhy.
   * The method then calls the corresponding BusytonePhy that the first
   * bit of the packet has arrived.
   *
   * \param receiver the device to which the packet is destined
   * \param packet the packet being sent
   * \param txPowerDbm the tx power associated to the packet being sent (dBm)
   * \param duration the transmission duration associated with the packet being sent
   */
  
  //static void Receive (Ptr<BusytonePhy> receiver, Ptr<Packet> packet, double txPowerDbm, Time duration);

  PhyList m_phyList;                   //!< List of YansWifiPhys connected to this BusytoneChannel
  Ptr<PropagationLossModel> m_loss;    //!< Propagation loss model
  //Ptr<PropagationDelayModel> m_delay;  //!< Propagation delay model//////check
};

} //namespace ns3

#endif /* YANS_WIFI_CHANNEL_H */
