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

#include "s1g-wifi-mac-helper.h"
#include "ns3/wifi-mac.h"
#include "ns3/edca-txop-n.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"

namespace ns3 {

S1gWifiMacHelper::S1gWifiMacHelper ()
{
}

S1gWifiMacHelper::~S1gWifiMacHelper ()
{
}

S1gWifiMacHelper
S1gWifiMacHelper::Default (void)
{
  S1gWifiMacHelper helper;

  //We're making S1G-enabled Wi-Fi MACs here, so we set the necessary
  //attribute. I've carefully positioned this here so that someone
  //who knows what they're doing can override with explicit attributes.
  helper.SetType ("ns3::StaWifiMac",
                  "QosSupported", BooleanValue (true),
                  "S1gSupported", BooleanValue (true));

  return helper;
}

StringValue
S1gWifiMacHelper::DataRateForMcs (int mcs)
{
  std::stringstream sstmp;
  std::string strtmp, dataRate;

  sstmp << mcs;
  sstmp >> strtmp;
  dataRate = "S1gMcs" + strtmp;
  
  return StringValue (dataRate);
}

} //namespace ns3
