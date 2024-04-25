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

#ifndef BUSY_TONE_PHY_H
#define BUSY_TONE_PHY_H

#include <map>
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/mobility-model.h"
#include "ns3/random-variable-stream.h"
#include "ns3/channel.h"
#include "wifi-phy-standard.h"
#include "interference-helper.h"
#include "ns3/node.h"
#include "ns3/string.h"

namespace ns3 {

#define HE_PHY 125
#define VHT_PHY 126
#define HT_PHY 127
class BusytoneChannel;
/**
 * FrameCaptureModel class
 */
class FrameCaptureModel;

/**
 * \brief 802.11 PHY layer model
 * \ingroup wifi
 *
 */
class BusytonePhy : public Object
{
public:
  BusytonePhy ();
  virtual ~BusytonePhy ();
  static TypeId GetTypeId(void);
  void SetChannel (const Ptr<BusytoneChannel> channel);
  ///liang
  double GetTxPowerW(void);
  void SetTxPowerW(double ToleranceInterference);
  double GetTxPowerDbm(void);
  void SetTxPowerDbm(double txPowerDbm);
  void SendBusytoneSignal(double txPowerW,Time rxDuration);/////接收端算出還可忍受的干擾後在busytone傳出
  double ReceivedBusytonePowerW();
  ///liang

  virtual Ptr<Channel> GetChannel (void) const ;
  //virtual int64_t AssignStreams (int64_t stream);////check
  double GetTxPowerStart (void) const;

  /**
   * Sets the device this PHY is associated with.
   *
   * \param device the device this PHY is associated with
   */
  void SetDevice (const Ptr<NetDevice> device);
  /**
   * Return the device this PHY is associated with
   *
   * \return the device this PHY is associated with
   */
  Ptr<NetDevice> GetDevice (void) const;
  /**
   * \brief assign a mobility model to this device
   *
   * This method allows a user to specify a mobility model that should be
   * associated with this physical layer.  Calling this method is optional
   * and only necessary if the user wants to override the mobility model
   * that is aggregated to the node.
   *
   * \param mobility the mobility model this PHY is associated with
   */
  void SetMobility (const Ptr<MobilityModel> mobility);
  /**
   * Return the mobility model this PHY is associated with.
   * This method will return either the mobility model that has been
   * explicitly set by a call to YansBusytonePhy::SetMobility(), or else
   * will return the mobility model (if any) that has been aggregated
   * to the node.
   *
   * \return the mobility model this PHY is associated with
   */
  Ptr<MobilityModel> GetMobility (void) const;

  
  //void SetErrorRateModel (const Ptr<ErrorRateModel> rate);///////check
  /**
   * Return the error rate model this PHY is using.
   *
   * \return the error rate model this PHY is using
   */
  /////Ptr<ErrorRateModel> GetErrorRateModel (void) const;////////check

  /**
   * Sets the frame capture model.
   *
   * \param rate the frame capture model
   */
  //////void SetFrameCaptureModel (const Ptr<FrameCaptureModel> rate);////check
  /**
   * Return the frame capture model this PHY is using.
   *
   * \return the frame capture model this PHY is using
   */
  //////Ptr<FrameCaptureModel> GetFrameCaptureModel (void) const;/////////check

protected:
  // Inherited
  //virtual void DoInitialize (void);
  virtual void DoDispose (void);////////////////////

private:
  
  Ptr<NetDevice>     m_device;   //!< Pointer to the device
  Ptr<MobilityModel> m_mobility; //!< Pointer to the mobility model
  Ptr<BusytoneChannel> m_channel;
  double m_busytonetxpower;
  double m_busytonetxpowerDbm;////////liang
};

/**
 * \param os          output stream
 * \param state       wifi state to stringify
 * \return output stream
 */
//std::ostream& operator<< (std::ostream& os, BusytonePhy::State state);

} //namespace ns3

#endif /* BUSY_TONE_PHY_H */
