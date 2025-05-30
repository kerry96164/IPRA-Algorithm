/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Ghada Badawy <gbadawy@gmail.com>
 */

#ifndef S1G_WIFI_MAC_HELPER_H
#define S1G_WIFI_MAC_HELPER_H

#include "ns3/string.h"
#include "wifi-helper.h"
#include "ns3/qos-utils.h"
#include "qos-wifi-mac-helper.h"
#include <map>

namespace ns3 {

/**
 * \brief create S1G-enabled MAC layers for a ns3::WifiNetDevice.
 *
 * This class can create MACs of type ns3::ApWifiMac, ns3::StaWifiMac,
 * and, ns3::AdhocWifiMac, with QosSupported and S1GSupported attributes set to True.
 */
class S1gWifiMacHelper : public QosWifiMacHelper
{
public:
  /**
   * Create a S1gWifiMacHelper that is used to make life easier when working
   * with Wifi devices using a QOS MAC layer.
   */
  S1gWifiMacHelper ();

  /**
   * Destroy a S1gWifiMacHelper
   */
  virtual ~S1gWifiMacHelper ();

  /**
   * Create a mac helper in a default working state.
   * 
   * \returns S1gWifiMacHelper
   */
  static S1gWifiMacHelper Default (void);

  /**
   * Converts a S1G MCS value into a DataRate value
   *
   * \param mcs MCS Value
   * \return data rate value (StringValue)
   */
  static StringValue DataRateForMcs (int mcs);

};

} //namespace ns3

#endif /* S1G_WIFI_MAC_HELPER_H */
