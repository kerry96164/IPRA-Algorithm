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

#include "wifi-phy.h"
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
#include "wifi-mac-header.h"//////////liang
#include "mpdu-aggregator.h"
#include "ampdu-tag.h"
#include "ns3/mac48-address.h"
#include "wifi-net-device.h"
#include "regular-wifi-mac.h"
#include "ns3/wifi-module.h"
#include "ns3/core-module.h"
#define time 1
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WifiPhy");

/****************************************************************
 *       This destructor is needed.
 ****************************************************************/
/*WifiPhyListener::WifiPhyListener ()
{
}*/

WifiPhyListener::~WifiPhyListener ()
{
}
/*void
WifiPhyListener::NotifyBusytoneTxPowerReady(Time rxDuration)
{
}
void 
WifiPhyListener::NotifyToleranceInterference (double ToleranceInterference)////從WifiPhyStateHelper::NotifyToleranceInterference() 得到還可以忍受的的干擾
{
}*/

/****************************************************************
 *       The actual WifiPhy class
 ****************************************************************/

NS_OBJECT_ENSURE_REGISTERED (WifiPhy);

/**
 * This table maintains the mapping of valid ChannelNumber to
 * Frequency/ChannelWidth pairs.  If you want to make a channel applicable
 * to all standards, then you may use the WIFI_PHY_STANDARD_UNSPECIFIED
 * standard to represent this, as a wildcard.  If you want to limit the
 * configuration of a particular channel/frequency/width to a particular
 * standard(s), then you can specify one or more such bindings.
 */
WifiPhy::ChannelToFrequencyWidthMap WifiPhy::m_channelToFrequencyWidth =
{
  // 802.11b uses width of 22, while OFDM modes use width of 20
  { std::make_pair (1, WIFI_PHY_STANDARD_80211b), std::make_pair (2412, 22) },
  { std::make_pair (1, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2412, 20) },
  { std::make_pair (2, WIFI_PHY_STANDARD_80211b), std::make_pair (2417, 22) },
  { std::make_pair (2, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2417, 20) },
  { std::make_pair (3, WIFI_PHY_STANDARD_80211b), std::make_pair (2422, 22) },
  { std::make_pair (3, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2422, 20) },
  { std::make_pair (4, WIFI_PHY_STANDARD_80211b), std::make_pair (2427, 22) },
  { std::make_pair (4, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2427, 20) },
  { std::make_pair (5, WIFI_PHY_STANDARD_80211b), std::make_pair (2432, 22) },
  { std::make_pair (5, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2432, 20) },
  { std::make_pair (6, WIFI_PHY_STANDARD_80211b), std::make_pair (2437, 22) },
  { std::make_pair (6, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2437, 20) },
  { std::make_pair (7, WIFI_PHY_STANDARD_80211b), std::make_pair (2442, 22) },
  { std::make_pair (7, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2442, 20) },
  { std::make_pair (8, WIFI_PHY_STANDARD_80211b), std::make_pair (2447, 22) },
  { std::make_pair (8, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2447, 20) },
  { std::make_pair (9, WIFI_PHY_STANDARD_80211b), std::make_pair (2452, 22) },
  { std::make_pair (9, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2452, 20) },
  { std::make_pair (10, WIFI_PHY_STANDARD_80211b), std::make_pair (2457, 22) },
  { std::make_pair (10, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2457, 20) },
  { std::make_pair (11, WIFI_PHY_STANDARD_80211b), std::make_pair (2462, 22) },
  { std::make_pair (11, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2462, 20) },
  { std::make_pair (12, WIFI_PHY_STANDARD_80211b), std::make_pair (2467, 22) },
  { std::make_pair (12, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2467, 20) },
  { std::make_pair (13, WIFI_PHY_STANDARD_80211b), std::make_pair (2472, 22) },
  { std::make_pair (13, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2472, 20) },
  // Only defined for 802.11b
  { std::make_pair (14, WIFI_PHY_STANDARD_80211b), std::make_pair (2484, 22) },

  // Now the 5GHz channels; UNSPECIFIED for 802.11a/n/ac/ax channels
  // 20 MHz channels
  { std::make_pair (36, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5180, 20) },
  { std::make_pair (40, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5200, 20) },
  { std::make_pair (44, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5220, 20) },
  { std::make_pair (48, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5240, 20) },
  { std::make_pair (52, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5260, 20) },
  { std::make_pair (56, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5280, 20) },
  { std::make_pair (60, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5300, 20) },
  { std::make_pair (64, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5320, 20) },
  { std::make_pair (100, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5500, 20) },
  { std::make_pair (104, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5520, 20) },
  { std::make_pair (108, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5540, 20) },
  { std::make_pair (112, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5560, 20) },
  { std::make_pair (116, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5580, 20) },
  { std::make_pair (120, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5600, 20) },
  { std::make_pair (124, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5620, 20) },
  { std::make_pair (128, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5640, 20) },
  { std::make_pair (132, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5660, 20) },
  { std::make_pair (136, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5680, 20) },
  { std::make_pair (140, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5700, 20) },
  { std::make_pair (144, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5720, 20) },
  { std::make_pair (149, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5745, 20) },
  { std::make_pair (153, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5765, 20) },
  { std::make_pair (157, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5785, 20) },
  { std::make_pair (161, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5805, 20) },
  { std::make_pair (165, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5825, 20) },
  // 40 MHz channels
  { std::make_pair (38, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5190, 40) },
  { std::make_pair (46, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5230, 40) },
  { std::make_pair (54, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5230, 40) },
  { std::make_pair (62, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5310, 40) },
  { std::make_pair (102, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5510, 40) },
  { std::make_pair (110, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5550, 40) },
  { std::make_pair (118, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5590, 40) },
  { std::make_pair (126, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5590, 40) },
  { std::make_pair (134, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5670, 40) },
  { std::make_pair (142, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5710, 40) },
  { std::make_pair (151, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5755, 40) },
  { std::make_pair (159, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5795, 40) },
  // 80 MHz channels
  { std::make_pair (42, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5210, 80) },
  { std::make_pair (58, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5290, 80) },
  { std::make_pair (106, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5530, 80) },
  { std::make_pair (122, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5610, 80) },
  { std::make_pair (138, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5690, 80) },
  { std::make_pair (155, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5775, 80) },
  // 160 MHz channels
  { std::make_pair (50, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5250, 160) },
  { std::make_pair (114, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5570, 160) },

  // 802.11p (10 MHz channels at the 5.855-5.925 band
  { std::make_pair (172, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5860, 10) },
  { std::make_pair (174, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5870, 10) },
  { std::make_pair (176, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5880, 10) },
  { std::make_pair (178, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5890, 10) },
  { std::make_pair (180, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5900, 10) },
  { std::make_pair (182, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5910, 10) },
  { std::make_pair (184, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5920, 10) },

  // 802.11ah channels (from 802.11ah-2016 Table E-4a—S1G Operating classes)
  // 1 MHz channels 
  //NOTE: Index 1 實際是 902.5Mhz，但儲存型別是整數，所以取整數，其他亦同。
  { std::make_pair (1, WIFI_PHY_STANDARD_80211ah), std::make_pair (902, 1) },
  { std::make_pair (3, WIFI_PHY_STANDARD_80211ah), std::make_pair (903, 1) },
  { std::make_pair (5, WIFI_PHY_STANDARD_80211ah), std::make_pair (904, 1) },
  { std::make_pair (7, WIFI_PHY_STANDARD_80211ah), std::make_pair (905, 1) },
  { std::make_pair (9, WIFI_PHY_STANDARD_80211ah), std::make_pair (906, 1) },
  { std::make_pair (11, WIFI_PHY_STANDARD_80211ah), std::make_pair (907, 1) },
  { std::make_pair (13, WIFI_PHY_STANDARD_80211ah), std::make_pair (908, 1) },
  { std::make_pair (15, WIFI_PHY_STANDARD_80211ah), std::make_pair (909, 1) },
  { std::make_pair (17, WIFI_PHY_STANDARD_80211ah), std::make_pair (910, 1) },
  { std::make_pair (19, WIFI_PHY_STANDARD_80211ah), std::make_pair (911, 1) },
  { std::make_pair (21, WIFI_PHY_STANDARD_80211ah), std::make_pair (912, 1) },
  { std::make_pair (23, WIFI_PHY_STANDARD_80211ah), std::make_pair (913, 1) },
  { std::make_pair (25, WIFI_PHY_STANDARD_80211ah), std::make_pair (914, 1) },
  { std::make_pair (27, WIFI_PHY_STANDARD_80211ah), std::make_pair (915, 1) },
  { std::make_pair (29, WIFI_PHY_STANDARD_80211ah), std::make_pair (916, 1) },
  { std::make_pair (31, WIFI_PHY_STANDARD_80211ah), std::make_pair (917, 1) },
  { std::make_pair (33, WIFI_PHY_STANDARD_80211ah), std::make_pair (918, 1) },
  { std::make_pair (35, WIFI_PHY_STANDARD_80211ah), std::make_pair (919, 1) },
  { std::make_pair (37, WIFI_PHY_STANDARD_80211ah), std::make_pair (920, 1) },
  { std::make_pair (39, WIFI_PHY_STANDARD_80211ah), std::make_pair (921, 1) },
  { std::make_pair (41, WIFI_PHY_STANDARD_80211ah), std::make_pair (922, 1) },
  { std::make_pair (43, WIFI_PHY_STANDARD_80211ah), std::make_pair (923, 1) },
  { std::make_pair (45, WIFI_PHY_STANDARD_80211ah), std::make_pair (924, 1) },
  { std::make_pair (47, WIFI_PHY_STANDARD_80211ah), std::make_pair (925, 1) },
  { std::make_pair (49, WIFI_PHY_STANDARD_80211ah), std::make_pair (926, 1) },
  { std::make_pair (51, WIFI_PHY_STANDARD_80211ah), std::make_pair (927, 1) },
  // 2 MHz channels
  { std::make_pair (2, WIFI_PHY_STANDARD_80211ah), std::make_pair (903, 2) },
  { std::make_pair (6, WIFI_PHY_STANDARD_80211ah), std::make_pair (905, 2) },
  { std::make_pair (10, WIFI_PHY_STANDARD_80211ah), std::make_pair (907, 2) },
  { std::make_pair (14, WIFI_PHY_STANDARD_80211ah), std::make_pair (909, 2) },
  { std::make_pair (18, WIFI_PHY_STANDARD_80211ah), std::make_pair (911, 2) },
  { std::make_pair (22, WIFI_PHY_STANDARD_80211ah), std::make_pair (913, 2) },
  { std::make_pair (26, WIFI_PHY_STANDARD_80211ah), std::make_pair (915, 2) },
  { std::make_pair (30, WIFI_PHY_STANDARD_80211ah), std::make_pair (917, 2) },
  { std::make_pair (34, WIFI_PHY_STANDARD_80211ah), std::make_pair (919, 2) },
  { std::make_pair (38, WIFI_PHY_STANDARD_80211ah), std::make_pair (921, 2) },
  { std::make_pair (42, WIFI_PHY_STANDARD_80211ah), std::make_pair (923, 2) },
  { std::make_pair (46, WIFI_PHY_STANDARD_80211ah), std::make_pair (925, 2) },
  { std::make_pair (50, WIFI_PHY_STANDARD_80211ah), std::make_pair (927, 2) },
  // 4 MHz channels
  { std::make_pair (8, WIFI_PHY_STANDARD_80211ah), std::make_pair (906, 4) },
  { std::make_pair (16, WIFI_PHY_STANDARD_80211ah), std::make_pair (910, 4) },
  { std::make_pair (24, WIFI_PHY_STANDARD_80211ah), std::make_pair (914, 4) },
  { std::make_pair (32, WIFI_PHY_STANDARD_80211ah), std::make_pair (918, 4) },
  { std::make_pair (40, WIFI_PHY_STANDARD_80211ah), std::make_pair (922, 4) },
  { std::make_pair (48, WIFI_PHY_STANDARD_80211ah), std::make_pair (926, 4) },
  // 8 MHz channels
  { std::make_pair (12, WIFI_PHY_STANDARD_80211ah), std::make_pair (908, 8) },
  { std::make_pair (28, WIFI_PHY_STANDARD_80211ah), std::make_pair (916, 8) },
  { std::make_pair (44, WIFI_PHY_STANDARD_80211ah), std::make_pair (924, 8) },
  // 16 MHz channels
  { std::make_pair (20, WIFI_PHY_STANDARD_80211ah), std::make_pair (912, 16) },
};

TypeId
WifiPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WifiPhy")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddAttribute ("Frequency",
                   "The operating center frequency (MHz)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&WifiPhy::GetFrequency,
                                         &WifiPhy::SetFrequency),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ChannelWidth",
                   "Whether 1Mhz, 2MHz, 4MHz, 5MHz, 8MHz, 10MHz, 16MHz, 20MHz, 22MHz, 40MHz, 80 MHz or 160 MHz.", //add 805.11ah
                   //UintegerValue (20),
                   UintegerValue (1), //802.11ah
                   MakeUintegerAccessor (&WifiPhy::GetChannelWidth,
                                         &WifiPhy::SetChannelWidth),
                   //MakeUintegerChecker<uint8_t> (5, 160))
                   MakeUintegerChecker<uint8_t> (1, 160)) //802.11ah
    .AddAttribute ("ChannelNumber",
                   "If set to non-zero defined value, will control Frequency and ChannelWidth assignment",
                   UintegerValue (0),
                   MakeUintegerAccessor (&WifiPhy::SetChannelNumber,
                                         &WifiPhy::GetChannelNumber),
                   MakeUintegerChecker<uint8_t> (0, 196))
    .AddAttribute ("EnergyDetectionThreshold",
                   "The energy of a received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to detect the signal.",
                   DoubleValue (-96.0),
                   MakeDoubleAccessor (&WifiPhy::SetEdThreshold,
                                       &WifiPhy::GetEdThreshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CcaMode1Threshold",
                   "The energy of a received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to declare CCA BUSY state.",
                   DoubleValue (-99.0),
                   MakeDoubleAccessor (&WifiPhy::SetCcaMode1Threshold,
                                       &WifiPhy::GetCcaMode1Threshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxGain",
                   "Transmission gain (dB).",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&WifiPhy::SetTxGain,
                                       &WifiPhy::GetTxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxGain",
                   "Reception gain (dB).",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&WifiPhy::SetRxGain,
                                       &WifiPhy::GetRxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerLevels",
                   "Number of transmission power levels available between "
                   "TxPowerStart and TxPowerEnd included.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::m_nTxPower),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("TxPowerEnd",
                   "Maximum available transmission level (dbm).",
                   DoubleValue (16.0204),
                   MakeDoubleAccessor (&WifiPhy::SetTxPowerEnd,
                                       &WifiPhy::GetTxPowerEnd),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerStart",
                   "Minimum available transmission level (dbm).",
                   DoubleValue (16.0204),
                   MakeDoubleAccessor (&WifiPhy::SetTxPowerStart,
                                       &WifiPhy::GetTxPowerStart),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxNoiseFigure",
                   "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                   " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                   "\"the difference in decibels (dB) between"
                   " the noise output of the actual receiver to the noise output of an "
                   " ideal receiver with the same overall gain and bandwidth when the receivers "
                   " are connected to sources at the standard noise temperature T0 (usually 290 K)\".",
                   DoubleValue (7),
                   MakeDoubleAccessor (&WifiPhy::SetRxNoiseFigure,
                                       &WifiPhy::GetRxNoiseFigure),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("State",
                   "The state of the PHY layer.",
                   PointerValue (),
                   MakePointerAccessor (&WifiPhy::m_state),
                   MakePointerChecker<WifiPhyStateHelper> ())
    .AddAttribute ("ChannelSwitchDelay",
                   "Delay between two short frames transmitted on different frequencies.",
                   TimeValue (MicroSeconds (250)),
                   MakeTimeAccessor (&WifiPhy::m_channelSwitchDelay),
                   MakeTimeChecker ())
    .AddAttribute ("TxAntennas",
                   "The number of supported Tx antennas.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::m_numberOfTransmitters),
                   MakeUintegerChecker<uint8_t> (1, 8),
                   TypeId::DEPRECATED,
                   "Not used anymore.")
    .AddAttribute ("RxAntennas",
                   "The number of supported Rx antennas.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::m_numberOfReceivers),
                   MakeUintegerChecker<uint8_t> (1, 8),
                   TypeId::DEPRECATED,
                   "Not used anymore.")
    .AddAttribute ("Antennas",
                   "The number of antennas on the device.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::GetNumberOfAntennas,
                                         &WifiPhy::SetNumberOfAntennas),
                   MakeUintegerChecker<uint8_t> (1, 8))
    .AddAttribute ("MaxSupportedTxSpatialStreams",
                   "The maximum number of supported TX spatial streams."
                   "This parameter is only valuable for 802.11n/ac/ah/ax STAs and APs.",//add 802.11ah
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::GetMaxSupportedTxSpatialStreams,
                                         &WifiPhy::SetMaxSupportedTxSpatialStreams),
                   MakeUintegerChecker<uint8_t> (1, 8))
    .AddAttribute ("MaxSupportedRxSpatialStreams",
                   "The maximum number of supported RX spatial streams."
                   "This parameter is only valuable for 802.11n/ac/ah/ax STAs and APs.",//add 802.11ah
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::GetMaxSupportedRxSpatialStreams,
                                         &WifiPhy::SetMaxSupportedRxSpatialStreams),
                   MakeUintegerChecker<uint8_t> (1, 8))
    .AddAttribute ("ShortGuardEnabled",
                   "Whether or not short guard interval is enabled for HT/VHT transmissions."
                   "This parameter is only valuable for 802.11n/ac/ah/ax STAs and APs.", //add 802.11ah
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetShortGuardInterval,
                                        &WifiPhy::SetShortGuardInterval),
                   MakeBooleanChecker ())
    .AddAttribute ("GuardInterval",
                   "Whether 800ns, 1600ns or 3200ns guard interval is used for HE transmissions."
                   "This parameter is only valuable for 802.11ax STAs and APs.",
                   TimeValue (NanoSeconds (800)),
                   MakeTimeAccessor (&WifiPhy::GetGuardInterval,
                                     &WifiPhy::SetGuardInterval),
                   MakeTimeChecker (NanoSeconds (400), NanoSeconds (3200)))
    .AddAttribute ("LdpcEnabled",
                   "Whether or not LDPC is enabled (not supported yet!).",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetLdpc,
                                        &WifiPhy::SetLdpc),
                   MakeBooleanChecker ())
    .AddAttribute ("STBCEnabled",
                   "Whether or not STBC is enabled (not supported yet!).",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetStbc,
                                        &WifiPhy::SetStbc),
                   MakeBooleanChecker ())
    .AddAttribute ("GreenfieldEnabled",
                   "Whether or not Greenfield is enabled."
                   "This parameter is only valuable for 802.11n STAs and APs.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetGreenfield,
                                        &WifiPhy::SetGreenfield),
                   MakeBooleanChecker ())
    // .AddAttribute ("S1g1MfieldEnabled",
    //                 "Whether or not S1g1Mfield is enabled."
    //                 "This parameter is only valuable for 802.11ah STAs and APs.",
    //                 BooleanValue (false),
    //                 MakeBooleanAccessor (&WifiPhy::GetS1g1Mfield,
    //                                      &WifiPhy::SetS1g1Mfield),
    //                 MakeBooleanChecker ())
    // .AddAttribute ("S1gShortfieldEnabled",
    //                 "Whether or not S1gShortfield is enabled."
    //                 "This parameter is only valuable for 802.11ah STAs and APs.",
    //                 BooleanValue (false),
    //                 MakeBooleanAccessor (&WifiPhy::GetS1gShortfield,
    //                                      &WifiPhy::SetS1gShortfield),
    //                 MakeBooleanChecker ())
    // .AddAttribute ("S1gLongfieldEnabled",
    //                 "Whether or not S1gLongfield is enabled."
    //                 "This parameter is only valuable for 802.11ah STAs and APs.",
    //                 BooleanValue (false),
    //                 MakeBooleanAccessor (&WifiPhy::GetS1gLongfield,
    //                                      &WifiPhy::SetS1gLongfield),
    //                 MakeBooleanChecker ())
    .AddAttribute ("ShortPlcpPreambleSupported",
                   "Whether or not short PLCP preamble is supported."
                   "This parameter is only valuable for 802.11b STAs and APs."
                   "Note: 802.11g APs and STAs always support short PLCP preamble.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetShortPlcpPreambleSupported,
                                        &WifiPhy::SetShortPlcpPreambleSupported),
                   MakeBooleanChecker ())
    .AddAttribute ("FrameCaptureModel",
                   "Ptr to an object that implements the frame capture model",
                   PointerValue (0), //StringValue ("ns3::SimpleFrameCaptureModel"),
                   MakePointerAccessor (&WifiPhy::GetFrameCaptureModel,
                                        &WifiPhy::SetFrameCaptureModel),
                   MakePointerChecker <FrameCaptureModel>())
    .AddTraceSource ("PhyTxBegin",
                     "Trace source indicating a packet "
                     "has begun transmitting over the channel medium",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyTxBeginTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyTxEnd",
                     "Trace source indicating a packet "
                     "has been completely transmitted over the channel. "
                     "NOTE: the only official WifiPhy implementation "
                     "available to this date never fires "
                     "this trace source.",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyTxEndTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyTxDrop",
                     "Trace source indicating a packet "
                     "has been dropped by the device during transmission",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyTxDropTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyRxBegin",
                     "Trace source indicating a packet "
                     "has begun being received from the channel medium "
                     "by the device",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyRxBeginTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyRxEnd",
                     "Trace source indicating a packet "
                     "has been completely received from the channel medium "
                     "by the device",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyRxEndTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyRxDrop",
                     "Trace source indicating a packet "
                     "has been dropped by the device during reception",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyRxDropTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MonitorSnifferRx",
                     "Trace source simulating a wifi device in monitor mode "
                     "sniffing all received frames",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyMonitorSniffRxTrace),
                     "ns3::WifiPhy::MonitorSnifferRxTracedCallback")
    .AddTraceSource ("MonitorSnifferTx",
                     "Trace source simulating the capability of a wifi device "
                     "in monitor mode to sniff all frames being transmitted",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyMonitorSniffTxTrace),
                     "ns3::WifiPhy::MonitorSnifferTxTracedCallback")
  ;
  return tid;
}

WifiPhy::WifiPhy ()
  : m_mpdusNum (0),
    m_plcpSuccess (false),
    m_txMpduReferenceNumber (0xffffffffffffffff),
    m_rxMpduReferenceNumber (0xffffffffffffffff),
    m_endRxEvent (),
    m_endPlcpRxEvent (),
    m_endRxWrongEvent(),
    m_standard (WIFI_PHY_STANDARD_UNSPECIFIED),
    m_isConstructed (false),
    m_channelCenterFrequency (0),
    m_initialFrequency (0),
    m_frequencyChannelNumberInitialized (false),
    m_channelNumber (0),
    m_initialChannelNumber (0),
    m_totalAmpduSize (0),
    m_totalAmpduNumSymbols (0),
    m_currentEvent (0),
    m_txpowerDbm(0),
    m_waitforcts(false)
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (m_numberOfTransmitters);
  NS_UNUSED (m_numberOfReceivers);
  m_random = CreateObject<UniformRandomVariable> ();
  m_state = CreateObject<WifiPhyStateHelper> ();
  //m_currentRate = 11;
}

WifiPhy::~WifiPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
WifiPhy::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_device = 0;
  m_mobility = 0;
  m_state = 0;
  m_deviceRateSet.clear ();
  m_deviceMcsSet.clear ();
}

void
WifiPhy::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_isConstructed = true;
  if (m_frequencyChannelNumberInitialized == true)
    {
      NS_LOG_DEBUG ("Frequency already initialized");
      return;
    }
  InitializeFrequencyChannelNumber ();
}

void
WifiPhy::SetReceiveOkCallback (RxOkCallback callback)
{
  m_state->SetReceiveOkCallback (callback);
}

void
WifiPhy::SetReceiveErrorCallback (RxErrorCallback callback)
{
  m_state->SetReceiveErrorCallback (callback);
}

void
WifiPhy::RegisterListener (WifiPhyListener *listener)
{
  m_state->RegisterListener (listener);
}

void
WifiPhy::UnregisterListener (WifiPhyListener *listener)
{
  m_state->UnregisterListener (listener);
}

void
WifiPhy::InitializeFrequencyChannelNumber (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_frequencyChannelNumberInitialized == false, "Initialization called twice");

  // If frequency has been set to a non-zero value during attribute
  // construction phase, the frequency and channel width will drive the
  // initial configuration.  If frequency has not been set, but both
  // standard and channel number have been set, that pair will instead
  // drive the configuration, and frequency and channel number will be
  // aligned
  if (m_initialFrequency != 0)
    {
      SetFrequency (m_initialFrequency);
    }
  else if (m_initialChannelNumber != 0 && GetStandard () != WIFI_PHY_STANDARD_UNSPECIFIED)
    {
      SetChannelNumber (m_initialChannelNumber);
    }
  else if (m_initialChannelNumber != 0 && GetStandard () == WIFI_PHY_STANDARD_UNSPECIFIED)
    {
      NS_FATAL_ERROR ("Error, ChannelNumber " << (uint16_t)GetChannelNumber () << " was set by user, but neither a standard nor a frequency");
    }
  m_frequencyChannelNumberInitialized = true;
}

void
WifiPhy::SetEdThreshold (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  m_edThresholdW = DbmToW (threshold);
}

double
WifiPhy::GetEdThresholdW (void) const
{
  return m_edThresholdW;
}

double
WifiPhy::GetEdThreshold (void) const
{
  return WToDbm (m_edThresholdW);
}

void
WifiPhy::SetCcaMode1Threshold (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  m_ccaMode1ThresholdW = DbmToW (threshold);
}

double
WifiPhy::GetCcaMode1Threshold (void) const
{
  return WToDbm (m_ccaMode1ThresholdW);
}

InterferenceHelper
WifiPhy::GetInterference (void) const
{
  return m_interference;
}

void
WifiPhy::SetRxNoiseFigure (double noiseFigureDb)
{
  NS_LOG_FUNCTION (this << noiseFigureDb);
  m_interference.SetNoiseFigure (DbToRatio (noiseFigureDb));
  m_interference.SetNumberOfReceiveAntennas (GetNumberOfAntennas ());
}

double
WifiPhy::GetRxNoiseFigure (void) const
{
  return RatioToDb (m_interference.GetNoiseFigure ());
}

void
WifiPhy::SetTxPowerStart (double start)
{
  NS_LOG_FUNCTION (this << start);
  m_txPowerBaseDbm = start;
}

double
WifiPhy::GetTxPowerStart (void) const
{
  return m_txPowerBaseDbm;
}

void
WifiPhy::SetTxPowerEnd (double end)
{
  NS_LOG_FUNCTION (this << end);
  m_txPowerEndDbm = end;
}

double
WifiPhy::GetTxPowerEnd (void) const
{
  return m_txPowerEndDbm;
}

void
WifiPhy::SetNTxPower (uint32_t n)
{
  NS_LOG_FUNCTION (this << n);
  m_nTxPower = n;
}

uint32_t
WifiPhy::GetNTxPower (void) const
{
  return m_nTxPower;
}

void
WifiPhy::SetTxGain (double gain)
{
  NS_LOG_FUNCTION (this << gain);
  m_txGainDb = gain;
}

double
WifiPhy::GetTxGain (void) const
{
  return m_txGainDb;
}

void
WifiPhy::SetRxGain (double gain)
{
  NS_LOG_FUNCTION (this << gain);
  m_rxGainDb = gain;
}

double
WifiPhy::GetRxGain (void) const
{
  return m_rxGainDb;
}

void
WifiPhy::SetLdpc (bool ldpc)
{
  NS_LOG_FUNCTION (this << ldpc);
  m_ldpc = ldpc;
}

bool
WifiPhy::GetLdpc (void) const
{
  return m_ldpc;
}

void
WifiPhy::SetStbc (bool stbc)
{
  NS_LOG_FUNCTION (this << stbc);
  m_stbc = stbc;
}

bool
WifiPhy::GetStbc (void) const
{
  return m_stbc;
}

void
WifiPhy::SetGreenfield (bool greenfield)
{
  NS_LOG_FUNCTION (this << greenfield);
  m_greenfield = greenfield;
}

// void //802.11ah
// WifiPhy::SetS1g1Mfield (bool s1g1mfield)
// {
//   NS_LOG_FUNCTION (this << s1g1mfield);
//   m_s1g1mfield = s1g1mfield;
// }

// void //802.11ah
// WifiPhy::SetS1gShortfield (bool s1gshortfield)
// {
//   NS_LOG_FUNCTION (this << s1gshortfield);
//   m_s1gshortfield = s1gshortfield;
// }

// void //802.11ah
// WifiPhy::SetS1gLongfield (bool s1glongfield)
// {
//   NS_LOG_FUNCTION (this << s1glongfield);
//   m_s1glongfield = s1glongfield;
// }

bool
WifiPhy::GetGreenfield (void) const
{
  return m_greenfield;
}

// bool //802.11ah
// WifiPhy::GetS1g1Mfield (void) const
// {
//   return m_s1g1mfield;
// }

// bool //802.11ah
// WifiPhy::GetS1gShortfield (void) const
// {
//   return m_s1gshortfield;
// }

// bool //802.11ah
// WifiPhy::GetS1gLongfield (void) const
// {
//   return m_s1glongfield;
// }

void
WifiPhy::SetShortGuardInterval (bool shortGuardInterval)
{
  NS_LOG_FUNCTION (this << shortGuardInterval);
  m_shortGuardInterval = shortGuardInterval;
}

bool
WifiPhy::GetShortGuardInterval (void) const
{
  return m_shortGuardInterval;
}

void
WifiPhy::SetGuardInterval (Time guardInterval)
{
  NS_LOG_FUNCTION (this << guardInterval);
  NS_ASSERT (guardInterval == NanoSeconds (800) || guardInterval == NanoSeconds (1600) || guardInterval == NanoSeconds (3200));
  m_guardInterval = guardInterval;
}

Time
WifiPhy::GetGuardInterval (void) const
{
  return m_guardInterval;
}

void
WifiPhy::SetShortPlcpPreambleSupported (bool enable)
{
  NS_LOG_FUNCTION (this << enable);
  m_shortPreamble = enable;
}

bool
WifiPhy::GetShortPlcpPreambleSupported (void) const
{
  return m_shortPreamble;
}

void
WifiPhy::SetDevice (const Ptr<NetDevice> device)
{
  m_device = device;
}

Ptr<NetDevice>
WifiPhy::GetDevice (void) const
{
  return m_device;
}

void
WifiPhy::SetMobility (const Ptr<MobilityModel> mobility)
{
  m_mobility = mobility;
}

Ptr<MobilityModel>
WifiPhy::GetMobility (void) const
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

void
WifiPhy::SetErrorRateModel (const Ptr<ErrorRateModel> rate)
{
  m_interference.SetErrorRateModel (rate);
  m_interference.SetNumberOfReceiveAntennas (GetNumberOfAntennas ());
}

Ptr<ErrorRateModel>
WifiPhy::GetErrorRateModel (void) const
{
  return m_interference.GetErrorRateModel ();
}

void
WifiPhy::SetFrameCaptureModel (const Ptr<FrameCaptureModel> model)
{
  m_frameCaptureModel = model;
}

Ptr<FrameCaptureModel>
WifiPhy::GetFrameCaptureModel (void) const
{
  return m_frameCaptureModel;
}

double
WifiPhy::GetPowerDbm (uint8_t power) const
{
  NS_ASSERT (m_txPowerBaseDbm <= m_txPowerEndDbm);
  NS_ASSERT (m_nTxPower > 0);
  double dbm;
  if (m_nTxPower > 1)
    {
      dbm = m_txPowerBaseDbm + power * (m_txPowerEndDbm - m_txPowerBaseDbm) / (m_nTxPower - 1);
    }
  else
    {
      NS_ASSERT_MSG (m_txPowerBaseDbm == m_txPowerEndDbm, "cannot have TxPowerEnd != TxPowerStart with TxPowerLevels == 1");
      dbm = m_txPowerBaseDbm;
    }
  return dbm;
}

Time
WifiPhy::GetChannelSwitchDelay (void) const
{
  return m_channelSwitchDelay;
}

double
WifiPhy::CalculateSnr (WifiTxVector txVector, double ber) const
{
  return m_interference.GetErrorRateModel ()->CalculateSnr (txVector, ber);
}

void
WifiPhy::ConfigureDefaultsForStandard (WifiPhyStandard standard)
{
  NS_LOG_FUNCTION (this << standard);
  switch (standard)
    {
    case WIFI_PHY_STANDARD_80211a:
      SetChannelWidth (20);
      SetFrequency (5180);
      // Channel number should be aligned by SetFrequency () to 36
      NS_ASSERT (GetChannelNumber () == 36);
      break;
    case WIFI_PHY_STANDARD_80211b:
      SetChannelWidth (22);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      break;
    case WIFI_PHY_STANDARD_80211g:
      SetChannelWidth (20);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      break;
    case WIFI_PHY_STANDARD_80211_10MHZ:
      SetChannelWidth (10);
      SetFrequency (5860);
      // Channel number should be aligned by SetFrequency () to 172
      NS_ASSERT (GetChannelNumber () == 172);
      break;
    case WIFI_PHY_STANDARD_80211_5MHZ:
      SetChannelWidth (5);
      SetFrequency (5860);
      // Channel number should be aligned by SetFrequency () to 0
      NS_ASSERT (GetChannelNumber () == 0);
      break;
    case WIFI_PHY_STANDARD_holland:
      SetChannelWidth (20);
      SetFrequency (5180);
      // Channel number should be aligned by SetFrequency () to 36
      NS_ASSERT (GetChannelNumber () == 36);
      break;
    case WIFI_PHY_STANDARD_80211n_2_4GHZ:
      SetChannelWidth (20);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      break;
    case WIFI_PHY_STANDARD_80211n_5GHZ:
      SetChannelWidth (20);
      SetFrequency (5180);
      // Channel number should be aligned by SetFrequency () to 36
      NS_ASSERT (GetChannelNumber () == 36);
      break;
    case WIFI_PHY_STANDARD_80211ac:
      SetChannelWidth (80);
      SetFrequency (5210);
      // Channel number should be aligned by SetFrequency () to 42
      NS_ASSERT (GetChannelNumber () == 42);
      break;
    case WIFI_PHY_STANDARD_80211ah:
      SetChannelWidth (1);
      SetFrequency (902);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      break;
    
    case WIFI_PHY_STANDARD_80211ax_2_4GHZ:
      SetChannelWidth (20);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      break;
    case WIFI_PHY_STANDARD_80211ax_5GHZ:
      SetChannelWidth (80);
      SetFrequency (5210);
      // Channel number should be aligned by SetFrequency () to 42
      NS_ASSERT (GetChannelNumber () == 42);
      break;
    case WIFI_PHY_STANDARD_UNSPECIFIED:
      NS_LOG_WARN ("Configuring unspecified standard; performing no action");
      break;
    default:
      NS_ASSERT (false);
      break;
    }
}

void
WifiPhy::Configure80211a (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate9Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate18Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate24Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate36Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate48Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate54Mbps ());
}

void
WifiPhy::Configure80211b (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetDsssRate1Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetDsssRate2Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetDsssRate5_5Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetDsssRate11Mbps ());
}

void
WifiPhy::Configure80211g (void)
{
  NS_LOG_FUNCTION (this);
  Configure80211b ();

  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate6Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate9Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate12Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate18Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate24Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate36Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate48Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate54Mbps ());
}

void
WifiPhy::Configure80211_10Mhz (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate3MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate4_5MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate9MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate18MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate24MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate27MbpsBW10MHz ());
}

void
WifiPhy::Configure80211_5Mhz (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate1_5MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate2_25MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate3MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate4_5MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate9MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate13_5MbpsBW5MHz ());
}

void
WifiPhy::ConfigureHolland (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate18Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate36Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate54Mbps ());
}

void
WifiPhy::ConfigureHtDeviceMcsSet (void)
{
  NS_LOG_FUNCTION (this);

  bool htFound = false;
  for (std::vector<uint32_t>::size_type i = 0; i < m_bssMembershipSelectorSet.size (); i++)
    {
      if (m_bssMembershipSelectorSet[i] == HT_PHY)
        {
          htFound = true;
          break;
        }
    }
  if (htFound)
    {
      // erase all HtMcs modes from deviceMcsSet
      size_t index = m_deviceMcsSet.size () - 1;
      for (std::vector<WifiMode>::reverse_iterator rit = m_deviceMcsSet.rbegin (); rit != m_deviceMcsSet.rend (); ++rit, --index)
        {
          if (m_deviceMcsSet[index].GetModulationClass () == WIFI_MOD_CLASS_HT)
            {
              m_deviceMcsSet.erase (m_deviceMcsSet.begin () + index);
            }
        }
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs0 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs1 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs2 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs3 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs4 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs5 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs6 ());
      m_deviceMcsSet.push_back (WifiPhy::GetHtMcs7 ());
      if (GetMaxSupportedTxSpatialStreams () > 1)
        {
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs8 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs9 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs10 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs11 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs12 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs13 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs14 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs15 ());
        }
      if (GetMaxSupportedTxSpatialStreams () > 2)
        {
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs16 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs17 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs18 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs19 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs20 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs21 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs22 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs23 ());
        }
      if (GetMaxSupportedTxSpatialStreams () > 3)
        {
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs24 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs25 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs26 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs27 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs28 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs29 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs30 ());
          m_deviceMcsSet.push_back (WifiPhy::GetHtMcs31 ());
        }
    }
}

void
WifiPhy::Configure80211n (void)
{
  NS_LOG_FUNCTION (this);
  if (Is2_4Ghz (GetFrequency ()))
    {
      Configure80211b ();
      Configure80211g ();
    }
  if (Is5Ghz (GetFrequency ()))
    {
      Configure80211a ();
    }
  m_bssMembershipSelectorSet.push_back (HT_PHY);
  ConfigureHtDeviceMcsSet ();
}

void
WifiPhy::Configure80211ac (void)
{
  NS_LOG_FUNCTION (this);
  Configure80211n ();

  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs0 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs1 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs2 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs3 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs4 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs5 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs6 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs7 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs8 ());
  m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs9 ());
  // m_deviceMcsSet.push_back (WifiPhy::GetVhtMcs10 ()); // Jonathan  
  m_bssMembershipSelectorSet.push_back (VHT_PHY);
}

void
WifiPhy::Configure80211ah (void)
{
  NS_LOG_FUNCTION (this);
  
  // 802.11ah 尚缺MCS10
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate300KbpsBW1MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate650KbpsBW2MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate1_35MbpsBW4MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate2_925MbpsBW8MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate5_85MbpsBW16MHz ());

  m_deviceMcsSet.push_back (WifiPhy::GetS1gMcs0 ());
  m_deviceMcsSet.push_back (WifiPhy::GetS1gMcs1 ());
  m_deviceMcsSet.push_back (WifiPhy::GetS1gMcs2 ());
  m_deviceMcsSet.push_back (WifiPhy::GetS1gMcs3 ());
  m_deviceMcsSet.push_back (WifiPhy::GetS1gMcs4 ());
  m_deviceMcsSet.push_back (WifiPhy::GetS1gMcs5 ());
  m_deviceMcsSet.push_back (WifiPhy::GetS1gMcs6 ());
  m_deviceMcsSet.push_back (WifiPhy::GetS1gMcs7 ());
  m_deviceMcsSet.push_back (WifiPhy::GetS1gMcs8 ());
  m_deviceMcsSet.push_back (WifiPhy::GetS1gMcs9 ());
  m_bssMembershipSelectorSet.push_back (S1G_PHY);
}

void
WifiPhy::Configure80211ax (void)
{
  NS_LOG_FUNCTION (this);
  Configure80211n ();
  if (Is5Ghz (GetFrequency ()))
    {
      Configure80211ac ();
    }
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs0 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs1 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs2 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs3 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs4 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs5 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs6 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs7 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs8 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs9 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs10 ());
  m_deviceMcsSet.push_back (WifiPhy::GetHeMcs11 ());

  m_bssMembershipSelectorSet.push_back (HE_PHY);
}

bool
WifiPhy::DefineChannelNumber (uint8_t channelNumber, WifiPhyStandard standard, uint16_t frequency, uint8_t channelWidth)
{
  NS_LOG_FUNCTION (this << (uint16_t)channelNumber << standard << frequency << (uint16_t)channelWidth);
  ChannelNumberStandardPair p = std::make_pair (channelNumber, standard);
  ChannelToFrequencyWidthMap::const_iterator it;
  it = m_channelToFrequencyWidth.find (p);
  if (it != m_channelToFrequencyWidth.end ())
    {
      NS_LOG_DEBUG ("channel number/standard already defined; returning false");
      return false;
    }
  FrequencyWidthPair f = std::make_pair (frequency, channelWidth);
  m_channelToFrequencyWidth[p] = f;
  return true;
}

uint8_t
WifiPhy::FindChannelNumberForFrequencyWidth (uint16_t frequency, uint8_t width) const
{
  NS_LOG_FUNCTION (this << frequency << (uint16_t)width);
  bool found = false;
  FrequencyWidthPair f = std::make_pair (frequency, width);
  ChannelToFrequencyWidthMap::const_iterator it = m_channelToFrequencyWidth.begin ();
  while (it != m_channelToFrequencyWidth.end ())
    {
      if (it->second == f)
        {
          found = true;
          break;
        }
      ++it;
    }
  if (found)
    {
      NS_LOG_DEBUG ("Found, returning " << it->first.first);
      return (it->first.first);
    }
  else
    {
      NS_LOG_DEBUG ("Not found, returning 0");
      return 0;
    }
}

void
WifiPhy::ConfigureChannelForStandard (WifiPhyStandard standard)
{
  NS_LOG_FUNCTION (this << standard);
  // If the user has configured both Frequency and ChannelNumber, Frequency
  // takes precedence
  if (GetFrequency () != 0)
    {
      // If Frequency is already set, then see whether a ChannelNumber can
      // be found that matches Frequency and ChannelWidth. If so, configure
      // the ChannelNumber to that channel number. If not, set ChannelNumber to zero.
      NS_LOG_DEBUG ("Frequency set; checking whether a channel number corresponds");
      uint8_t channelNumberSearched = FindChannelNumberForFrequencyWidth (GetFrequency (), GetChannelWidth ());
      if (channelNumberSearched)
        {
          NS_LOG_DEBUG ("Channel number found; setting to " << (uint16_t)channelNumberSearched);
          SetChannelNumber (channelNumberSearched);
        }
      else
        {
          NS_LOG_DEBUG ("Channel number not found; setting to zero");
          SetChannelNumber (0);
        }
    }
  else if (GetChannelNumber () != 0)
    {
      // If the channel number is known for this particular standard or for
      // the unspecified standard, configure using the known values;
      // otherwise, this is a configuration error
      NS_LOG_DEBUG ("Configuring for channel number " << (uint16_t)GetChannelNumber ());
      FrequencyWidthPair f = GetFrequencyWidthForChannelNumberStandard (GetChannelNumber (), standard);
      if (f.first == 0)
        {
          // the specific pair of number/standard is not known
          NS_LOG_DEBUG ("Falling back to check WIFI_PHY_STANDARD_UNSPECIFIED");
          f = GetFrequencyWidthForChannelNumberStandard (GetChannelNumber (), WIFI_PHY_STANDARD_UNSPECIFIED);
        }
      if (f.first == 0)
        {
          NS_FATAL_ERROR ("Error, ChannelNumber " << (uint16_t)GetChannelNumber () << " is unknown for this standard");
        }
      else
        {
          NS_LOG_DEBUG ("Setting frequency to " << f.first << "; width to " << (uint16_t)f.second);
          SetFrequency (f.first);
          SetChannelWidth (f.second);
        }
    }
}

void
WifiPhy::ConfigureStandard (WifiPhyStandard standard)
{
  NS_LOG_FUNCTION (this << standard);
  m_standard = standard;
  m_isConstructed = true;
  if (m_frequencyChannelNumberInitialized == false)
    {
      InitializeFrequencyChannelNumber ();
    }
  if (GetFrequency () == 0 && GetChannelNumber () == 0)
    {
      ConfigureDefaultsForStandard (standard);
    }
  else
    {
      // The user has configured either (or both) Frequency or ChannelNumber
      ConfigureChannelForStandard (standard);
    }
  switch (standard)
    {
    case WIFI_PHY_STANDARD_80211a:
      Configure80211a ();
      break;
    case WIFI_PHY_STANDARD_80211b:
      Configure80211b ();
      break;
    case WIFI_PHY_STANDARD_80211g:
      Configure80211g ();
      break;
    case WIFI_PHY_STANDARD_80211_10MHZ:
      Configure80211_10Mhz ();
      break;
    case WIFI_PHY_STANDARD_80211_5MHZ:
      Configure80211_5Mhz ();
      break;
    case WIFI_PHY_STANDARD_holland:
      ConfigureHolland ();
      break;
    case WIFI_PHY_STANDARD_80211n_2_4GHZ:
      Configure80211n ();
      break;
    case WIFI_PHY_STANDARD_80211n_5GHZ:
      Configure80211n ();
      break;
    case WIFI_PHY_STANDARD_80211ac:
      Configure80211ac ();
      break;
    case WIFI_PHY_STANDARD_80211ah:
      Configure80211ah ();
      break;
    case WIFI_PHY_STANDARD_80211ax_2_4GHZ:
      Configure80211ax ();
      break;
    case WIFI_PHY_STANDARD_80211ax_5GHZ:
      Configure80211ax ();
      break;
    default:
      NS_ASSERT (false);
      break;
    }
}

WifiPhyStandard
WifiPhy::GetStandard (void) const
{
  return m_standard;
}

void
WifiPhy::SetFrequency (uint16_t frequency)
{
  NS_LOG_FUNCTION (this << frequency);
  if (m_isConstructed == false)
    {
      NS_LOG_DEBUG ("Saving frequency configuration for initialization");
      m_initialFrequency = frequency;
      return;
    }
  if (GetFrequency () == frequency)
    {
      NS_LOG_DEBUG ("No frequency change requested");
      return;
    }
  if (frequency == 0)
    {
      DoFrequencySwitch (0);
      NS_LOG_DEBUG ("Setting frequency and channel number to zero");
      m_channelCenterFrequency = 0;
      m_channelNumber = 0;
      return;
    }
  // If the user has configured both Frequency and ChannelNumber, Frequency
  // takes precedence.  Lookup the channel number corresponding to the
  // requested frequency.
  uint8_t nch = FindChannelNumberForFrequencyWidth (frequency, GetChannelWidth ());
  if (nch != 0)
    {
      NS_LOG_DEBUG ("Setting frequency " << frequency << " corresponds to channel " << (uint16_t)nch);
      if (DoFrequencySwitch (frequency))
        {
          NS_LOG_DEBUG ("Channel frequency switched to " << frequency << "; channel number to " << (uint16_t)nch);
          m_channelCenterFrequency = frequency;
          m_channelNumber = nch;
        }
      else
        {
          NS_LOG_DEBUG ("Suppressing reassignment of frequency");
        }
    }
  else
    {
      NS_LOG_DEBUG ("Channel number is unknown for frequency " << frequency);
      if (DoFrequencySwitch (frequency))
        {
          NS_LOG_DEBUG ("Channel frequency switched to " << frequency << "; channel number to " << 0);
          m_channelCenterFrequency = frequency;
          m_channelNumber = 0;
        }
      else
        {
          NS_LOG_DEBUG ("Suppressing reassignment of frequency");
        }
    }
}

uint16_t
WifiPhy::GetFrequency (void) const
{
  return m_channelCenterFrequency;
}

bool
WifiPhy::Is2_4Ghz (double frequency)
{
  if (frequency >= 2400 && frequency <= 2500)
    {
      return true;
    }
  return false;
}

bool
WifiPhy::Is5Ghz (double frequency)
{
  if (frequency >= 5000 && frequency <= 6000)
    {
      return true;
    }
  return false;
}

void
WifiPhy::SetChannelWidth (uint8_t channelwidth)
{
  //NS_ASSERT_MSG (channelwidth == 5 || channelwidth == 10 || channelwidth == 20 || channelwidth == 22 || channelwidth == 40 || channelwidth == 80 || channelwidth == 160, "wrong channel width value");
  //add 802.11ah CW 1Hhz
   if (!(channelwidth == 1 || channelwidth == 5 || channelwidth == 10 || channelwidth == 20 || channelwidth == 22 || channelwidth == 40 || channelwidth == 80 || channelwidth == 160))
   {
       std::ostringstream oss;
       oss << "wrong channel width value " << static_cast<int>(channelwidth);
       NS_ASSERT_MSG(false, oss.str().c_str());
   }
  
  m_channelWidth = channelwidth;
  AddSupportedChannelWidth (channelwidth);
}

uint8_t
WifiPhy::GetChannelWidth (void) const
{
  return m_channelWidth;
}

void
WifiPhy::SetNumberOfAntennas (uint8_t antennas)
{
  NS_ASSERT_MSG (antennas > 0 && antennas <= 4, "unsupported number of antennas");
  m_numberOfAntennas = antennas;
  m_interference.SetNumberOfReceiveAntennas (antennas);
}

uint8_t
WifiPhy::GetNumberOfAntennas (void) const
{
  return m_numberOfAntennas;
}

void
WifiPhy::SetMaxSupportedTxSpatialStreams (uint8_t streams)
{
  NS_ASSERT (streams <= GetNumberOfAntennas ());
  m_txSpatialStreams = streams;
  ConfigureHtDeviceMcsSet ();
}

uint8_t
WifiPhy::GetMaxSupportedTxSpatialStreams (void) const
{
  return m_txSpatialStreams;
}

void
WifiPhy::SetMaxSupportedRxSpatialStreams (uint8_t streams)
{
  NS_ASSERT (streams <= GetNumberOfAntennas ());
  m_rxSpatialStreams = streams;
}

uint8_t
WifiPhy::GetMaxSupportedRxSpatialStreams (void) const
{
  return m_rxSpatialStreams;
}

uint32_t
WifiPhy::GetNBssMembershipSelectors (void) const
{
  return m_bssMembershipSelectorSet.size ();
}

uint32_t
WifiPhy::GetBssMembershipSelector (uint32_t selector) const
{
  return m_bssMembershipSelectorSet[selector];
}

WifiModeList
WifiPhy::GetMembershipSelectorModes (uint32_t selector)
{
  uint32_t id = GetBssMembershipSelector (selector);
  WifiModeList supportedmodes;
  if (id == HT_PHY || id == VHT_PHY || id == HE_PHY)
    {
      //mandatory MCS 0 to 7
      supportedmodes.push_back (WifiPhy::GetHtMcs0 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs1 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs2 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs3 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs4 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs5 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs6 ());
      supportedmodes.push_back (WifiPhy::GetHtMcs7 ());
    }
  if (id == VHT_PHY || id == HE_PHY)
    {
      //mandatory MCS 0 to 9
      supportedmodes.push_back (WifiPhy::GetVhtMcs0 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs1 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs2 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs3 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs4 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs5 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs6 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs7 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs8 ());
      supportedmodes.push_back (WifiPhy::GetVhtMcs9 ());
    }
  if (id == S1G_PHY) //802.11ah
    {
      //mandatory MCS 0 to 9
      //TODO: add MCS 10
      supportedmodes.push_back (WifiPhy::GetS1gMcs0 ());
      supportedmodes.push_back (WifiPhy::GetS1gMcs1 ());
      supportedmodes.push_back (WifiPhy::GetS1gMcs2 ());
      supportedmodes.push_back (WifiPhy::GetS1gMcs3 ());
      supportedmodes.push_back (WifiPhy::GetS1gMcs4 ());
      supportedmodes.push_back (WifiPhy::GetS1gMcs5 ());
      supportedmodes.push_back (WifiPhy::GetS1gMcs6 ());
      supportedmodes.push_back (WifiPhy::GetS1gMcs7 ());
      supportedmodes.push_back (WifiPhy::GetS1gMcs8 ());
      supportedmodes.push_back (WifiPhy::GetS1gMcs9 ());
    }
  if (id == HE_PHY)
    {
      //mandatory MCS 0 to 11
      supportedmodes.push_back (WifiPhy::GetHeMcs0 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs1 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs2 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs3 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs4 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs5 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs6 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs7 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs8 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs9 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs10 ());
      supportedmodes.push_back (WifiPhy::GetHeMcs11 ());
    }
  return supportedmodes;
}

void
WifiPhy::AddSupportedChannelWidth (uint8_t width)
{
  NS_LOG_FUNCTION (this << (uint16_t)width);
  for (std::vector<uint32_t>::size_type i = 0; i != m_supportedChannelWidthSet.size (); i++)
    {
      if (m_supportedChannelWidthSet[i] == width)
        {
          return;
        }
    }
  NS_LOG_FUNCTION ("Adding " << (uint16_t)width << " to supported channel width set");
  m_supportedChannelWidthSet.push_back (width);
}

std::vector<uint8_t>
WifiPhy::GetSupportedChannelWidthSet (void) const
{
  return m_supportedChannelWidthSet;
}

WifiPhy::FrequencyWidthPair
WifiPhy::GetFrequencyWidthForChannelNumberStandard (uint8_t channelNumber, WifiPhyStandard standard) const
{
  ChannelNumberStandardPair p = std::make_pair (channelNumber, standard);
  FrequencyWidthPair f = m_channelToFrequencyWidth[p];
  return f;
}

void
WifiPhy::SetChannelNumber (uint8_t nch)
{
  NS_LOG_FUNCTION (this << (uint16_t)nch);
  if (m_isConstructed == false)
    {
      NS_LOG_DEBUG ("Saving channel number configuration for initialization");
      m_initialChannelNumber = nch;
      return;
    }
  if (GetChannelNumber () == nch)
    {
      NS_LOG_DEBUG ("No channel change requested");
      return;
    }
  if (nch == 0)
    {
      // This case corresponds to when there is not a known channel
      // number for the requested frequency.  There is no need to call
      // DoChannelSwitch () because DoFrequencySwitch () should have been
      // called by the client
      NS_LOG_DEBUG ("Setting channel number to zero");
      m_channelNumber = 0;
      return;
    }

  // First make sure that the channel number is defined for the standard
  // in use
  FrequencyWidthPair f = GetFrequencyWidthForChannelNumberStandard (nch, GetStandard ());
  if (f.first == 0)
    {
      f = GetFrequencyWidthForChannelNumberStandard (nch, WIFI_PHY_STANDARD_UNSPECIFIED);
    }
  if (f.first != 0)
    {
      if (DoChannelSwitch (nch))
        {
          NS_LOG_DEBUG ("Setting frequency to " << f.first << "; width to " << (uint16_t)f.second);
          m_channelCenterFrequency = f.first;
          SetChannelWidth (f.second);
          m_channelNumber = nch;
        }
      else
        {
          // Subclass may have suppressed (e.g. waiting for state change)
          NS_LOG_DEBUG ("Channel switch suppressed");
        }
    }
  else
    {
      NS_FATAL_ERROR ("Frequency not found for channel number " << nch);
    }
}

uint8_t
WifiPhy::GetChannelNumber (void) const
{
  return m_channelNumber;
}

bool
WifiPhy::DoChannelSwitch (uint8_t nch)
{
  if (!IsInitialized ())
    {
      //this is not channel switch, this is initialization
      NS_LOG_DEBUG ("initialize to channel " << (uint16_t)nch);
      return true;
    }

  NS_ASSERT (!IsStateSwitching ());
  switch (m_state->GetState ())
    {
    case WifiPhy::RX:
      NS_LOG_DEBUG ("drop packet because of channel switching while reception");
      m_endPlcpRxEvent.Cancel ();
      m_endRxEvent.Cancel ();
      goto switchChannel;
      break;
    case WifiPhy::TX:
      NS_LOG_DEBUG ("channel switching postponed until end of current transmission");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetChannelNumber, this, nch);
      break;
    case WifiPhy::CCA_BUSY:
    case WifiPhy::IDLE:
      goto switchChannel;
      break;
    case WifiPhy::SLEEP:
      NS_LOG_DEBUG ("channel switching ignored in sleep mode");
      break;
    default:
      NS_ASSERT (false);
      break;
    }

  return false;

switchChannel:

  NS_LOG_DEBUG ("switching channel " << (uint16_t)GetChannelNumber () << " -> " << (uint16_t)nch);
  m_state->SwitchToChannelSwitching (GetChannelSwitchDelay ());
  m_interference.EraseEvents ();
  /*
   * Needed here to be able to correctly sensed the medium for the first
   * time after the switching. The actual switching is not performed until
   * after m_channelSwitchDelay. Packets received during the switching
   * state are added to the event list and are employed later to figure
   * out the state of the medium after the switching.
   */
  return true;
}

bool
WifiPhy::DoFrequencySwitch (uint16_t frequency)
{
  if (!IsInitialized ())
    {
      //this is not channel switch, this is initialization
      NS_LOG_DEBUG ("start at frequency " << frequency);
      return true;
    }

  NS_ASSERT (!IsStateSwitching ());
  switch (m_state->GetState ())
    {
    case WifiPhy::RX:
      NS_LOG_DEBUG ("drop packet because of channel/frequency switching while reception");
      m_endPlcpRxEvent.Cancel ();
      m_endRxEvent.Cancel ();
      goto switchFrequency;
      break;
    case WifiPhy::TX:
      NS_LOG_DEBUG ("channel/frequency switching postponed until end of current transmission");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetFrequency, this, frequency);
      break;
    case WifiPhy::CCA_BUSY:
    case WifiPhy::IDLE:
      goto switchFrequency;
      break;
    case WifiPhy::SLEEP:
      NS_LOG_DEBUG ("frequency switching ignored in sleep mode");
      break;
    default:
      NS_ASSERT (false);
      break;
    }

  return false;

switchFrequency:

  NS_LOG_DEBUG ("switching frequency " << GetFrequency () << " -> " << frequency);
  m_state->SwitchToChannelSwitching (GetChannelSwitchDelay ());
  m_interference.EraseEvents ();
  /*
   * Needed here to be able to correctly sensed the medium for the first
   * time after the switching. The actual switching is not performed until
   * after m_channelSwitchDelay. Packets received during the switching
   * state are added to the event list and are employed later to figure
   * out the state of the medium after the switching.
   */
  return true;
}

void
WifiPhy::SetSleepMode (void)
{
  NS_LOG_FUNCTION (this);
  switch (m_state->GetState ())
    {
    case WifiPhy::TX:
      NS_LOG_DEBUG ("setting sleep mode postponed until end of current transmission");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetSleepMode, this);
      break;
    case WifiPhy::RX:
      NS_LOG_DEBUG ("setting sleep mode postponed until end of current reception");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetSleepMode, this);
      break;
    case WifiPhy::SWITCHING:
      NS_LOG_DEBUG ("setting sleep mode postponed until end of channel switching");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetSleepMode, this);
      break;
    case WifiPhy::CCA_BUSY:
    case WifiPhy::IDLE:
      NS_LOG_DEBUG ("setting sleep mode");
      m_state->SwitchToSleep ();
      break;
    case WifiPhy::SLEEP:
      NS_LOG_DEBUG ("already in sleep mode");
      break;
    default:
      NS_ASSERT (false);
      break;
    }
}

void
WifiPhy::ResumeFromSleep (void)
{
  NS_LOG_FUNCTION (this);
  switch (m_state->GetState ())
    {
    case WifiPhy::TX:
    case WifiPhy::RX:
    case WifiPhy::IDLE:
    case WifiPhy::CCA_BUSY:
    case WifiPhy::SWITCHING:
      {
        NS_LOG_DEBUG ("not in sleep mode, there is nothing to resume");
        break;
      }
    case WifiPhy::SLEEP:
      {
        NS_LOG_DEBUG ("resuming from sleep mode");
        Time delayUntilCcaEnd = m_interference.GetEnergyDuration (DbmToW (GetCcaMode1Threshold ()));
        m_state->SwitchFromSleep (delayUntilCcaEnd);
        break;
      }
    default:
      {
        NS_ASSERT (false);
        break;
      }
    }
}

WifiMode
WifiPhy::GetHtPlcpHeaderMode ()
{
  return WifiPhy::GetHtMcs0 ();
}

WifiMode
WifiPhy::GetVhtPlcpHeaderMode ()
{
  return WifiPhy::GetVhtMcs0 ();
}

//802.11ah
WifiMode
WifiPhy::GetS1gPlcpHeaderMode ()
{
  return WifiPhy::GetS1gMcs0 ();
}

WifiMode
WifiPhy::GetHePlcpHeaderMode ()
{
  return WifiPhy::GetHeMcs0 ();
}

Time
WifiPhy::GetPlcpTrainingSymbolDuration (WifiTxVector txVector)
{
  uint8_t Ndltf, Neltf;
  //We suppose here that STBC = 0.
  //If STBC > 0, we need a different mapping between Nss and Nltf (IEEE 802.11n-2012 standard, page 1682).
  //Nss = number of spatial streams
  //Ness = number of spatial streams in beamforming 額外支援的空間串流數
  //NSS + NESS = total number of space-time streams supported
  if (txVector.GetNss () < 3)
    {
      Ndltf = txVector.GetNss ();
    }
  else if (txVector.GetNss () < 5)
    {
      Ndltf = 4;
    }
  else if (txVector.GetNss () < 7)
    {
      Ndltf = 6;
    }
  else
    {
      Ndltf = 8;
    }

  if (txVector.GetNess () < 3)
    {
      Neltf = txVector.GetNess ();
    }
  else
    {
      Neltf = 4;
    }

  switch (txVector.GetPreambleType ())
    {
    case WIFI_PREAMBLE_HT_MF:
      return MicroSeconds (4 + (4 * Ndltf) + (4 * Neltf));
    case WIFI_PREAMBLE_HT_GF:
      return MicroSeconds ((4 * Ndltf) + (4 * Neltf));
    case WIFI_PREAMBLE_VHT:
      return MicroSeconds (4 + (4 * Ndltf));
    case WIFI_PREAMBLE_HE_SU:
      return MicroSeconds (4 + (8 * Ndltf));
    //802.11ah-2016
    //Table 23-4
    //Figure 23-1~3
    case WIFI_PREAMBLE_S1G_LONG:
      //D-STF + D-LTF-1~N
      return MicroSeconds (40 + (40 * Ndltf));
    case WIFI_PREAMBLE_S1G_SHORT:
    case WIFI_PREAMBLE_S1G_1M:
      //LTF-2~N
      return MicroSeconds (40 * (Ndltf - 1));
    default:
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpHtSigHeaderDuration (WifiPreamble preamble)
{
  switch (preamble)
    {
    case WIFI_PREAMBLE_HT_MF:
    case WIFI_PREAMBLE_HT_GF:
      //HT-SIG
      return MicroSeconds (8);
    default:
      //no HT-SIG for non HT
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpSigA1Duration (WifiPreamble preamble)
{
  switch (preamble)
    {
    case WIFI_PREAMBLE_VHT:
    case WIFI_PREAMBLE_HE_SU:
      //VHT-SIG-A1 and HE-SIG-A1
      return MicroSeconds (4);
    //802.11ah no SIG-A1 except WIFI_PREAMBLE_S1G_LONG
    case WIFI_PREAMBLE_S1G_LONG:
      return MicroSeconds(40);
    case WIFI_PREAMBLE_S1G_SHORT:
    case WIFI_PREAMBLE_S1G_1M:
    default:
      // no SIG-A1
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpSigA2Duration (WifiPreamble preamble)
{
  switch (preamble)
    {
    case WIFI_PREAMBLE_VHT:
    case WIFI_PREAMBLE_HE_SU:
      //VHT-SIG-A2 and HE-SIG-A2
      return MicroSeconds (4);
    //802.11ah no SIG-A2 except WIFI_PREAMBLE_S1G_LONG
    case WIFI_PREAMBLE_S1G_LONG:
      return MicroSeconds(40);
    case WIFI_PREAMBLE_S1G_SHORT:
    case WIFI_PREAMBLE_S1G_1M:
    default:
      // no SIG-A2
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpSigBDuration (WifiPreamble preamble)
{
  switch (preamble)
    {
    case WIFI_PREAMBLE_VHT:
      //VHT-SIG-B
      return MicroSeconds (4);
    case WIFI_PREAMBLE_HE_SU:
      //HE-SIG-B: MU not supported so HE-SIG-B not used
      return MicroSeconds (0);
    //802.11ah no SIG-B except WIFI_PREAMBLE_S1G_LONG
    case WIFI_PREAMBLE_S1G_LONG:
      return MicroSeconds(40);
    case WIFI_PREAMBLE_S1G_SHORT:
    case WIFI_PREAMBLE_S1G_1M:
    default:
      // no SIG-B
      return MicroSeconds (0);
    }
}

WifiMode
WifiPhy::GetPlcpHeaderMode (WifiTxVector txVector)
{
  switch (txVector.GetMode ().GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
    case WIFI_MOD_CLASS_HT:
    case WIFI_MOD_CLASS_VHT:
    case WIFI_MOD_CLASS_HE:
      switch (txVector.GetChannelWidth ())
        {
        case 5:
          return WifiPhy::GetOfdmRate1_5MbpsBW5MHz ();
        case 10:
          return WifiPhy::GetOfdmRate3MbpsBW10MHz ();
        case 20:
        case 40:
        case 80:
        case 160:
        default:
          //(Section 18.3.2 "PLCP frame format"; IEEE Std 802.11-2012)
          //actually this is only the first part of the PlcpHeader,
          //because the last 16 bits of the PlcpHeader are using the
          //same mode of the payload
          return WifiPhy::GetOfdmRate6Mbps ();
        }
    case WIFI_MOD_CLASS_S1G: //802.11ah
      {
        switch (txVector.GetChannelWidth ())
          {
          case 1:
          default:
            return WifiPhy::GetOfdmRate300KbpsBW1MHz (); //802.11ah maybe should be MCS 10?
          case 2:
            return WifiPhy::GetOfdmRate650KbpsBW2MHz ();
          case 4:
            return WifiPhy::GetOfdmRate1_35MbpsBW4MHz ();
          case 8:
            return WifiPhy::GetOfdmRate2_925MbpsBW8MHz ();
          case 16:
            return WifiPhy::GetOfdmRate5_85MbpsBW16MHz ();
          }
      }
    case WIFI_MOD_CLASS_ERP_OFDM:
      return WifiPhy::GetErpOfdmRate6Mbps ();
    case WIFI_MOD_CLASS_DSSS:
    case WIFI_MOD_CLASS_HR_DSSS:
      if (txVector.GetPreambleType () == WIFI_PREAMBLE_LONG || txVector.GetMode () == WifiPhy::GetDsssRate1Mbps ())
        {
          //(Section 16.2.3 "PLCP field definitions" and Section 17.2.2.2 "Long PPDU format"; IEEE Std 802.11-2012)
          return WifiPhy::GetDsssRate1Mbps ();
        }
      else //WIFI_PREAMBLE_SHORT
        {
          //(Section 17.2.2.3 "Short PPDU format"; IEEE Std 802.11-2012)
          return WifiPhy::GetDsssRate2Mbps ();
        }
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return WifiMode ();
    }
}

Time
WifiPhy::GetPlcpHeaderDuration (WifiTxVector txVector)
{
  WifiPreamble preamble = txVector.GetPreambleType ();
  if (preamble == WIFI_PREAMBLE_NONE)
    {
      return MicroSeconds (0);
    }
  switch (txVector.GetMode ().GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
      {
        switch (txVector.GetChannelWidth ())
          {
          case 20:
          default:
            //(Section 18.3.3 "PLCP preamble (SYNC))" and Figure 18-4 "OFDM training structure"; IEEE Std 802.11-2012)
            //also (Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012)
            //We return the duration of the SIGNAL field only, since the
            //SERVICE field (which strictly speaking belongs to the PLCP
            //header, see Section 18.3.2 and Figure 18-1) is sent using the
            //payload mode.
            return MicroSeconds (4);
          case 10:
            //(Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012)
            return MicroSeconds (8);
          case 5:
            //(Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012)
            return MicroSeconds (16);
          }
      }
    case WIFI_MOD_CLASS_HT:
      {
        //L-SIG
        //IEEE 802.11n Figure 20.1
        switch (preamble)
          {
          case WIFI_PREAMBLE_HT_MF:
          default:
            return MicroSeconds (4);
          case WIFI_PREAMBLE_HT_GF:
            return MicroSeconds (0);
          }
      }
    case WIFI_MOD_CLASS_ERP_OFDM:
    case WIFI_MOD_CLASS_VHT:
      //L-SIG
      return MicroSeconds (4);
    case WIFI_MOD_CLASS_S1G:
      {
        //SIG
        switch (preamble)
          {
          case WIFI_PREAMBLE_S1G_1M:
            //802.11ah-2016 Figure 23-3. S1G_1M format
            return MicroSeconds (40 * 6); 
          case WIFI_PREAMBLE_S1G_SHORT:
            //802.11ah-2016 Figure 23-1. S1G_SHORT format
            return MicroSeconds (40 * 2); 
          case WIFI_PREAMBLE_S1G_LONG:
            //802.11ah-2016 Figure 23-2. S1G_LONG format
          default:
            return MicroSeconds (0);
          }
      }
    case WIFI_MOD_CLASS_HE:
      //LSIG + R-LSIG
      return MicroSeconds (8);
    case WIFI_MOD_CLASS_DSSS:
    case WIFI_MOD_CLASS_HR_DSSS:
      if ((preamble == WIFI_PREAMBLE_SHORT) && (txVector.GetMode ().GetDataRate (22) > 1000000))
        {
          //(Section 17.2.2.3 "Short PPDU format" and Figure 17-2 "Short PPDU format"; IEEE Std 802.11-2012)
          return MicroSeconds (24);
        }
      else //WIFI_PREAMBLE_LONG
        {
          //(Section 17.2.2.2 "Long PPDU format" and Figure 17-1 "Short PPDU format"; IEEE Std 802.11-2012)
          return MicroSeconds (48);
        }
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpPreambleDuration (WifiTxVector txVector)
{
  WifiPreamble preamble = txVector.GetPreambleType ();
  if (preamble == WIFI_PREAMBLE_NONE)
    {
      return MicroSeconds (0);
    }
  switch (txVector.GetMode ().GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
      {
        switch (txVector.GetChannelWidth ())
          {
          case 20:
          default:
            //(Section 18.3.3 "PLCP preamble (SYNC))" Figure 18-4 "OFDM training structure"
            //also Section 18.3.2.3 "Modulation-dependent parameters" Table 18-4 "Modulation-dependent parameters"; IEEE Std 802.11-2012)
            return MicroSeconds (16);
          case 10:
            //(Section 18.3.3 "PLCP preamble (SYNC))" Figure 18-4 "OFDM training structure"
            //also Section 18.3.2.3 "Modulation-dependent parameters" Table 18-4 "Modulation-dependent parameters"; IEEE Std 802.11-2012)
            return MicroSeconds (32);
          case 5:
            //(Section 18.3.3 "PLCP preamble (SYNC))" Figure 18-4 "OFDM training structure"
            //also Section 18.3.2.3 "Modulation-dependent parameters" Table 18-4 "Modulation-dependent parameters"; IEEE Std 802.11-2012)
            return MicroSeconds (64);
          }
      }
    case WIFI_MOD_CLASS_HT:
    case WIFI_MOD_CLASS_VHT:
    case WIFI_MOD_CLASS_HE:
      //L-STF + L-LTF
      return MicroSeconds (16);
    case WIFI_MOD_CLASS_S1G:
      //IEEE 802.11ah 
      //STF + LTF1
      if (preamble == WIFI_PREAMBLE_S1G_1M)
        {
          //802.11ah-2016 Figure 23-3. S1G_1M format
          return MicroSeconds (40 * 8); 
        }
      else
        {
          //802.11ah-2016 Figure 23-1. S1G_SHORT format
          //802.11ah-2016 Figure 23-2. S1G_LONG format
          return MicroSeconds (40 * 4); 
        }
    case WIFI_MOD_CLASS_ERP_OFDM:
      return MicroSeconds (16);
    case WIFI_MOD_CLASS_DSSS:
    case WIFI_MOD_CLASS_HR_DSSS:
      if ((preamble == WIFI_PREAMBLE_SHORT) && (txVector.GetMode ().GetDataRate (22) > 1000000))
        {
          //(Section 17.2.2.3 "Short PPDU format)" Figure 17-2 "Short PPDU format"; IEEE Std 802.11-2012)
          return MicroSeconds (72);
        }
      else //WIFI_PREAMBLE_LONG
        {
          //(Section 17.2.2.2 "Long PPDU format)" Figure 17-1 "Long PPDU format"; IEEE Std 802.11-2012)
          return MicroSeconds (144);
        }
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPayloadDuration (uint32_t size, WifiTxVector txVector, uint16_t frequency)
{
  return GetPayloadDuration (size, txVector, frequency, NORMAL_MPDU, 0);
}

Time
WifiPhy::GetPayloadDuration (uint32_t size, WifiTxVector txVector, uint16_t frequency, MpduType mpdutype, uint8_t incFlag)
{
  WifiMode payloadMode = txVector.GetMode ();
  WifiPreamble preamble = txVector.GetPreambleType ();
  NS_LOG_FUNCTION (size << payloadMode);

  double stbc = 1;
  if (txVector.IsStbc ()
      && (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_HT
          || payloadMode.GetModulationClass () == WIFI_MOD_CLASS_VHT
          || payloadMode.GetModulationClass () == WIFI_MOD_CLASS_S1G)) //802.11ah
    {
      //802.11ah For S1G_1M PPDU modulated using MCS10, STBC shall not be applied.
      NS_ASSERT (!(payloadMode.GetModulationClass () == WIFI_MOD_CLASS_S1G && payloadMode.GetMcsValue () == 10));
      stbc = 2;
    }

  double Nes = 1;
  //todo: improve logic to reduce the number of if cases
  //todo: extend to NSS > 4 for VHT rates
  //(802.11ah) Nes always is 1 in 802.11ah-2016, Table 24-38
  if (payloadMode.GetUniqueName () == "HtMcs21"
      || payloadMode.GetUniqueName () == "HtMcs22"
      || payloadMode.GetUniqueName () == "HtMcs23"
      || payloadMode.GetUniqueName () == "HtMcs28"
      || payloadMode.GetUniqueName () == "HtMcs29"
      || payloadMode.GetUniqueName () == "HtMcs30"
      || payloadMode.GetUniqueName () == "HtMcs31")
    {
      Nes = 2;
    }
  if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_VHT)
    {
      if (txVector.GetChannelWidth () == 40
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () >= 8)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss () == 2
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () == 9)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 4)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 2
          && payloadMode.GetMcsValue () >= 4)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 2
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () >= 3)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () >= 5)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 3
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 4;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 2)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 4)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 5)
        {
          Nes = 4;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss () == 4
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 6;
        }
    }

  Time symbolDuration = Seconds (0);
  switch (payloadMode.GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
    case WIFI_MOD_CLASS_ERP_OFDM:
      {
        //(Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012
        //corresponds to T_{SYM} in the table)
        switch (txVector.GetChannelWidth ())
          {
          case 20:
          default:
            symbolDuration = MicroSeconds (4);
            break;
          case 10:
            symbolDuration = MicroSeconds (8);
            break;
          case 5:
            symbolDuration = MicroSeconds (16);
            break;
          }
        break;
      }
    case WIFI_MOD_CLASS_HT:
    case WIFI_MOD_CLASS_VHT:
      {
        //if short GI data rate is used then symbol duration is 3.6us else symbol duration is 4us
        //In the future has to create a stationmanager that only uses these data rates if sender and receiver support GI
        uint16_t gi = txVector.GetGuardInterval ();
        NS_ASSERT (gi == 400 || gi == 800);
        symbolDuration = NanoSeconds (3200 + gi);
      }
      break;
    case WIFI_MOD_CLASS_S1G: //802.11ah
      {
        //if short GI data rate is used then symbol duration is 36us else symbol duration is 40us
        //In the future has to create a stationmanager that only uses these data rates if sender and receiver support GI
        uint16_t gi = txVector.GetGuardInterval ();
        NS_ASSERT (gi == 4000 || gi == 8000);
        symbolDuration = NanoSeconds (32000 + gi);   //normal GI 32 + 8 = 40 µs
      }
      break;
    case WIFI_MOD_CLASS_HE:
      {
        //if short GI data rate is used then symbol duration is 3.6us else symbol duration is 4us
        //In the future has to create a stationmanager that only uses these data rates if sender and receiver support GI
        uint16_t gi = txVector.GetGuardInterval ();
        NS_ASSERT (gi == 800 || gi == 1600 || gi == 3200);
        symbolDuration = NanoSeconds (12800 + gi);
      }
      break;
    default:
      break;
    }

  double numDataBitsPerSymbol = payloadMode.GetDataRate (txVector) * symbolDuration.GetNanoSeconds () / 1e9;

  double numSymbols = 0;
  if (mpdutype == MPDU_IN_AGGREGATE && preamble != WIFI_PREAMBLE_NONE)
    {
      //First packet in an A-MPDU
      if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_S1G) //802.11ah
        {
          //802.11ah-2016 Equation 23-75
          //N_sym = m_stbc * ceil ((N_service + APEP_LENGTH * 8 + N_tail * N_ES) / (m_stbc * N_DPBS));
          numSymbols = ceil (stbc *(8 + size * 8.0 + 6 * Nes) / (stbc * numDataBitsPerSymbol));
        }
      else //non-S1G
        {
          numSymbols = (stbc * (16 + size * 8.0 + 6 * Nes) / (stbc * numDataBitsPerSymbol));
        }
      if (incFlag == 1)
        {
          m_totalAmpduSize += size;
          m_totalAmpduNumSymbols += numSymbols;
        }
    }
  else if (mpdutype == MPDU_IN_AGGREGATE && preamble == WIFI_PREAMBLE_NONE)
    {
      //consecutive packets in an A-MPDU
      numSymbols = (stbc * size * 8.0) / (stbc * numDataBitsPerSymbol); //same as 802.11ah
      if (incFlag == 1)
        {
          m_totalAmpduSize += size;
          m_totalAmpduNumSymbols += numSymbols;
        }
    }
  else if (mpdutype == LAST_MPDU_IN_AGGREGATE && preamble == WIFI_PREAMBLE_NONE)
    {
      //last packet in an A-MPDU
      uint32_t totalAmpduSize = m_totalAmpduSize + size;
      if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_S1G) //802.11ah
        {
          numSymbols = lrint (stbc * ceil ((8 + totalAmpduSize * 8.0 + 6 * Nes) / (stbc * numDataBitsPerSymbol)));
        }
      else //non-S1G
        {
          numSymbols = lrint (stbc * ceil ((16 + totalAmpduSize * 8.0 + 6 * Nes) / (stbc * numDataBitsPerSymbol)));
        }
      NS_ASSERT (m_totalAmpduNumSymbols <= numSymbols);
      numSymbols -= m_totalAmpduNumSymbols;
      if (incFlag == 1)
        {
          m_totalAmpduSize = 0;
          m_totalAmpduNumSymbols = 0;
        }
    }
  else if (mpdutype == NORMAL_MPDU && preamble != WIFI_PREAMBLE_NONE)
    {
      //Not an A-MPDU
      if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_S1G) //802.11ah
        {
          numSymbols = lrint (stbc * ceil ((8 + size * 8.0 + 6.0 * Nes) / (stbc * numDataBitsPerSymbol)));
        }
      else //non-S1G
        {
          numSymbols = lrint (stbc * ceil ((16 + size * 8.0 + 6.0 * Nes) / (stbc * numDataBitsPerSymbol)));
        }
    }
  else
    {
      NS_FATAL_ERROR ("Wrong combination of preamble and packet type");
    }

  switch (payloadMode.GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
    case WIFI_MOD_CLASS_ERP_OFDM:
      {
        //Add signal extension for ERP PHY
        if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_ERP_OFDM)
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ()) + MicroSeconds (6);
          }
        else
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ());
          }
      }
    case WIFI_MOD_CLASS_HT:
    case WIFI_MOD_CLASS_VHT:
      {
        if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_HT && Is2_4Ghz (frequency)
            && ((mpdutype == NORMAL_MPDU && preamble != WIFI_PREAMBLE_NONE)
                || (mpdutype == LAST_MPDU_IN_AGGREGATE && preamble == WIFI_PREAMBLE_NONE))) //at 2.4 GHz
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ()) + MicroSeconds (6);
          }
        else //at 5 GHz
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ());
          }
      }
    case WIFI_MOD_CLASS_S1G: //802.11ah
      return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ());
    case WIFI_MOD_CLASS_HE:
      {
        if (Is2_4Ghz (frequency)
            && ((mpdutype == NORMAL_MPDU && preamble != WIFI_PREAMBLE_NONE)
                || (mpdutype == LAST_MPDU_IN_AGGREGATE && preamble == WIFI_PREAMBLE_NONE))) //at 2.4 GHz
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ()) + MicroSeconds (6);
          }
        else //at 5 GHz
          {
            return FemtoSeconds (numSymbols * symbolDuration.GetFemtoSeconds ());
          }
      }
    case WIFI_MOD_CLASS_DSSS:
    case WIFI_MOD_CLASS_HR_DSSS:
      return MicroSeconds (lrint (ceil ((size * 8.0) / (payloadMode.GetDataRate (22) / 1.0e6))));
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return MicroSeconds (0);
    }
}

Time
WifiPhy::CalculatePlcpPreambleAndHeaderDuration (WifiTxVector txVector) //802.11ah checked
{
  WifiPreamble preamble = txVector.GetPreambleType ();
  Time duration = GetPlcpPreambleDuration (txVector)
    + GetPlcpHeaderDuration (txVector)
    + GetPlcpHtSigHeaderDuration (preamble)
    + GetPlcpSigA1Duration (preamble)
    + GetPlcpSigA2Duration (preamble)
    + GetPlcpTrainingSymbolDuration (txVector)
    + GetPlcpSigBDuration (preamble);
  return duration;
}

Time
WifiPhy::CalculateTxDuration (uint32_t size, WifiTxVector txVector, uint16_t frequency, MpduType mpdutype, uint8_t incFlag) //802.11ah checked
{
  Time duration = CalculatePlcpPreambleAndHeaderDuration (txVector)
    + GetPayloadDuration (size, txVector, frequency, mpdutype, incFlag);
  return duration;
}

Time
WifiPhy::CalculateTxDuration (uint32_t size, WifiTxVector txVector, uint16_t frequency) //802.11ah checked
{
  return CalculateTxDuration (size, txVector, frequency, NORMAL_MPDU, 0);
}

void
WifiPhy::NotifyTxBegin (Ptr<const Packet> packet)
{
  m_phyTxBeginTrace (packet);
}

void
WifiPhy::NotifyTxEnd (Ptr<const Packet> packet)
{
  m_phyTxEndTrace (packet);
}

void
WifiPhy::NotifyTxDrop (Ptr<const Packet> packet)
{
  m_phyTxDropTrace (packet);
}

void
WifiPhy::NotifyRxBegin (Ptr<const Packet> packet)
{
  m_phyRxBeginTrace (packet);
}

void
WifiPhy::NotifyRxEnd (Ptr<const Packet> packet)
{
  m_phyRxEndTrace (packet);
}

void
WifiPhy::NotifyRxDrop (Ptr<const Packet> packet)
{
  m_phyRxDropTrace (packet);
}

void
WifiPhy::NotifyMonitorSniffRx (Ptr<const Packet> packet, uint16_t channelFreqMhz, WifiTxVector txVector, MpduInfo aMpdu, SignalNoiseDbm signalNoise)
{
  m_phyMonitorSniffRxTrace (packet, channelFreqMhz, txVector, aMpdu, signalNoise);
}

void
WifiPhy::NotifyMonitorSniffTx (Ptr<const Packet> packet, uint16_t channelFreqMhz, WifiTxVector txVector, MpduInfo aMpdu)
{
  m_phyMonitorSniffTxTrace (packet, channelFreqMhz, txVector, aMpdu);
}

Ptr<WifiPhyStateHelper> 
WifiPhy::GetWifiPhyState()
{
return m_state;
}

void
WifiPhy::SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, MpduType mpdutype)
{///std::cout<<"node "<<GetDevice()->GetNode ()->GetId ()<<"sendpacket"<<Simulator::Now().GetMicroSeconds()<<std::endl;
  NS_LOG_FUNCTION (this << packet << txVector.GetMode ()
                        << txVector.GetMode ().GetDataRate (txVector)
                        << txVector.GetPreambleType ()
                        << (uint16_t)txVector.GetTxPowerLevel ()
                        << (uint16_t)mpdutype);
  /* Transmission can happen if:
   *  - we are syncing on a packet. It is the responsability of the
   *    MAC layer to avoid doing this but the PHY does nothing to
   *    prevent it.
   *  - we are idle
   */
  NS_ASSERT (!m_state->IsStateTx () && !m_state->IsStateSwitching ());
  //txVector.SetMode(ChooseRate(m_currentRate));
  if (txVector.GetNss () > GetMaxSupportedTxSpatialStreams ())
    {
      NS_FATAL_ERROR ("Unsupported number of spatial streams!");
    }

  if (m_state->IsStateSleep ())
    {
      NS_LOG_DEBUG ("Dropping packet because in sleep mode");
      NotifyTxDrop (packet);
      return;
    }

  Time txDuration = CalculateTxDuration (packet->GetSize (), txVector, GetFrequency (), mpdutype, 1);
  NS_ASSERT (txDuration.IsStrictlyPositive ());

  if (m_state->IsStateRx ())
    {
      m_endPlcpRxEvent.Cancel ();
      m_endRxEvent.Cancel ();
      m_interference.NotifyRxEnd ();
    }
  NotifyTxBegin (packet);
  if ((mpdutype == MPDU_IN_AGGREGATE) && (txVector.GetPreambleType () != WIFI_PREAMBLE_NONE))
    {
      //send the first MPDU in an MPDU
      m_txMpduReferenceNumber++;
    }
  MpduInfo aMpdu;
  aMpdu.type = mpdutype;
  aMpdu.mpduRefNumber = m_txMpduReferenceNumber;
  NotifyMonitorSniffTx (packet, GetFrequency (), txVector, aMpdu);
  /////////liang
  ///////////////////////////////////////////////////
  if(Simulator::Now().GetSeconds()> time && GetDevice()->GetIfIndex()==0)
  {
        Mac48Address address;
        address = Mac48Address::ConvertFrom(GetDevice()->GetAddress());
        AmpduTag mpdu;
        Ptr<Packet>nPacket = packet->Copy();
        if(nPacket->RemovePacketTag(mpdu))
        {

                  MpduAggregator::DeaggregatedMpdus packets = MpduAggregator::Deaggregate (nPacket);
                  MpduAggregator::DeaggregatedMpdusCI n = packets.begin ();
                  WifiMacHeader firsthdr;
                  (*n).first->PeekHeader (firsthdr);
                  ////std::cout<<address<<" "<<firsthdr.GetAddr1()<<std::endl;
                  //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<" switchtotx at "<<Simulator::Now().GetMicroSeconds()<<std::endl;
                  m_state->SwitchToTx (txDuration, packet, WToDbm(txVector.GetTxPowerFromVector()), txVector,address);
                  //if(Mac48Address::ConvertFrom(GetDevice()->GetAddress()) == firsthdr.GetAddr1())
                  //PacketIsMind = true;
        }
        else
        {        
                  WifiMacHeader hdr;
                  nPacket->PeekHeader(hdr);
 
                  if(hdr.IsAck() || hdr.IsBlockAck())
                    m_state->HasReserveForAck(false);
                  else if(hdr.IsCts())
                    m_state->HasReserveForCts(false);
                  
                  ///std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<" switchtotx at "<<Simulator::Now().GetMicroSeconds()<<std::endl;
                  m_state->SwitchToTx (txDuration, packet, WToDbm(txVector.GetTxPowerFromVector()), txVector,address);
                  //if(Mac48Address::ConvertFrom(GetDevice()->GetAddress()) == hdr.GetAddr1())
                  //PacketIsMind = true;
        }
  }
  else
  {
    m_state->SwitchToTx (txDuration, packet, GetPowerDbm (txVector.GetTxPowerLevel ()), txVector);
  }
  ///////////////////////////////////////////////////
  /// m_state->SwitchToTx (txDuration, packet, WToDbm(txVector.GetTxPowerFromVector()), txVector);
  ///  m_state->SwitchToTx (txDuration, packet, GetPowerDbm (txVector.GetTxPowerLevel ()), txVector);

  Ptr<Packet> newPacket = packet->Copy (); // obtain non-const Packet
  WifiPhyTag oldtag;
  newPacket->RemovePacketTag (oldtag);
  WifiPhyTag tag (txVector, mpdutype);
  newPacket->AddPacketTag (tag);
  SetTxPowerDbm(WToDbm(txVector.GetTxPowerFromVector()),txDuration);//////liang
  
  StartTx (newPacket, txVector, txDuration);
}

void
WifiPhy::StartReceivePreambleAndHeader (Ptr<Packet> packet, double rxPowerW, Time rxDuration)
{
  if(m_state->IsReservedForAck() || m_state->IsReservedForCts())
  {
    //std::cout<<"drop because reserved ack\n";
    NotifyRxDrop(packet);
    m_plcpSuccess = false;
  }
  else
  { 
   //std::cout<<"false id is "<<GetDevice()->GetNode ()->GetId ()<<std::endl;
  //This function should be later split to check separately whether plcp preamble and plcp header can be successfully received.
  //Note: plcp preamble reception is not yet modeled.
  NS_LOG_FUNCTION (this << packet << WToDbm (rxPowerW) << rxDuration);
  Time endRx = Simulator::Now () + rxDuration;
  
//////////////////////////////////////////////////////////////////////

       Mac48Address address;
       AmpduTag mpdu;
       Ptr<Packet>nPacket = packet->Copy();

       if(nPacket->RemovePacketTag(mpdu))
       {
                MpduAggregator::DeaggregatedMpdus packets = MpduAggregator::Deaggregate (nPacket);
                MpduAggregator::DeaggregatedMpdusCI n = packets.begin ();
                WifiMacHeader firsthdr;
                (*n).first->PeekHeader (firsthdr);
                address = firsthdr.GetAddr1();
       }
       else
       {        
                WifiMacHeader hdr;
                packet->PeekHeader(hdr);
                address = hdr.GetAddr1();
       }
////////////////////////////////////////////////////////////////////////

  WifiPhyTag tag;
  bool found = packet->RemovePacketTag (tag);
  if (!found)
    {
      NS_FATAL_ERROR ("Received Wi-Fi Signal with no WifiPhyTag");
      return;
    }

  WifiTxVector txVector = tag.GetWifiTxVector ();

  if (txVector.GetMode ().GetModulationClass () == WIFI_MOD_CLASS_HT
      && (txVector.GetNss () != (1 + (txVector.GetMode ().GetMcsValue () / 8))))
    {
      NS_FATAL_ERROR ("MCS value does not match NSS value: MCS = " << (uint16_t)txVector.GetMode ().GetMcsValue () << ", NSS = " << (uint16_t)txVector.GetNss ());
    }

  Ptr<InterferenceHelper::Event> event;
  event = m_interference.Add (packet,
                              txVector,
                              rxDuration,
                              rxPowerW);

  if (txVector.GetNss () > GetMaxSupportedRxSpatialStreams ())
    {
      NS_LOG_DEBUG ("drop packet because not enough RX antennas");
      NotifyRxDrop (packet);
      m_plcpSuccess = false;
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {
          //that packet will be noise _after_ the transmission of the
          //currently-transmitted packet.
          if(Simulator::Now().GetSeconds()> time && GetDevice()->GetIfIndex()==0)
            MaybeCcaBusyDuration (address);
          else
            MaybeCcaBusyDuration();
          return;
        }
    }

  MpduType mpdutype = tag.GetMpduType ();
  switch (m_state->GetState ())
    {
    case WifiPhy::SWITCHING:
      NS_LOG_DEBUG ("drop packet because of channel switching");
      NotifyRxDrop (packet);
      m_plcpSuccess = false;
      /*
       * Packets received on the upcoming channel are added to the event list
       * during the switching state. This way the medium can be correctly sensed
       * when the device listens to the channel for the first time after the
       * switching e.g. after channel switching, the channel may be sensed as
       * busy due to other devices' tramissions started before the end of
       * the switching.
       */
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {
          //that packet will be noise _after_ the completion of the
          //channel switching.
          if(Simulator::Now().GetSeconds()> time && GetDevice()->GetIfIndex()==0)
            MaybeCcaBusyDuration (address);
          else
            MaybeCcaBusyDuration();
          return;
        }
      break;
    case WifiPhy::RX:   
      //std::cout<<"rx "<<Simulator::Now().GetMicroSeconds()<<std::endl;
      NS_ASSERT (m_currentEvent != 0);
      if (m_frameCaptureModel != 0 &&
          m_frameCaptureModel->CaptureNewFrame(m_currentEvent, event))
        {
          //std::cout<<"+++++++++++++++++++++++++++++++++\n";
          AbortCurrentReception ();
          NS_LOG_DEBUG ("Switch to new packet");
          StartRx (packet, txVector, mpdutype, rxPowerW, rxDuration, event);
        }
      else
        {
          NS_LOG_DEBUG ("drop packet because already in Rx (power=" <<
                        rxPowerW << "W)");
          NotifyRxDrop (packet);
          if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
            {///std::cout<<" 2 "<<Simulator::Now().GetSeconds()<<std::endl;
              //that packet will be noise _after_ the reception of the
              //currently-received packet.
              if(Simulator::Now().GetSeconds()> time && GetDevice()->GetIfIndex()==0)
                MaybeCcaBusyDuration (address);
              else
                MaybeCcaBusyDuration();
              return;
            }
        }
      break;
    case WifiPhy::TX:
//std::cout<<"tx "<<Simulator::Now().GetMicroSeconds()<<std::endl;
      NS_LOG_DEBUG ("drop packet because already in Tx (power=" <<
                    rxPowerW << "W)");
      NotifyRxDrop (packet);
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {//std::cout<<" 1 "<<Simulator::Now().GetSeconds()<<std::endl;
          //that packet will be noise _after_ the transmission of the
          //currently-transmitted packet.
          if(Simulator::Now().GetSeconds()> time && GetDevice()->GetIfIndex()==0)
            MaybeCcaBusyDuration (address);
          else
            MaybeCcaBusyDuration();
          return;
        }
      break;
    case WifiPhy::CCA_BUSY:
    case WifiPhy::IDLE:
      StartRx (packet, txVector, mpdutype, rxPowerW, rxDuration, event);
      break;
    case WifiPhy::SLEEP:
      NS_LOG_DEBUG ("drop packet because in sleep mode");
      NotifyRxDrop (packet);
      m_plcpSuccess = false;
      break;
    }
    }
}

void
WifiPhy::MaybeCcaBusyDuration ()
{
  //We are here because we have received the first bit of a packet and we are
  //not going to be able to synchronize on it
  //In this model, CCA becomes busy when the aggregation of all signals as
  //tracked by the InterferenceHelper class is higher than the CcaBusyThreshold

  Time delayUntilCcaEnd = m_interference.GetEnergyDuration (DbmToW (GetCcaMode1Threshold ()));
  if (!delayUntilCcaEnd.IsZero ())
    {
      m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd);
    }
}

void
WifiPhy::MaybeCcaBusyDuration(Mac48Address address)
{
  Time delayUntilCcaEnd = m_interference.GetEnergyDuration (DbmToW (GetCcaMode1Threshold ()));
  if (!delayUntilCcaEnd.IsZero ())
    {
      m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd,address);
    }
}

void
WifiPhy::StartReceivePacket (Ptr<Packet> packet,
                             WifiTxVector txVector,
                             MpduType mpdutype,
                             Ptr<InterferenceHelper::Event> event,
							 Time rxDuration,double rxPowerW)
{
  NS_LOG_FUNCTION (this << packet << txVector.GetMode () << txVector.GetPreambleType () << (uint16_t)mpdutype);
  NS_ASSERT (IsStateRx ());
  NS_ASSERT (m_endPlcpRxEvent.IsExpired ());
  WifiMode txMode = txVector.GetMode ();
  WifiMacHeader hdr;
  //std::cout<<"wifiphy "<<packet->GetSize()<<std::endl;
  AmpduTag ampdu;
  Ptr<Packet>nPacket = packet->Copy();
  bool IsData = nPacket->RemovePacketTag(ampdu);
  
  double minSINR;/////計算最小的SINR(WE)
  double SNR;
  double tolerable;
  InterferenceHelper::SnrPer snrPer;

  snrPer = m_interference.CalculatePlcpHeaderSnrPer (event);
  minSINR = CalculateSnr(txVector,1E-6);////
  SNR = GetSNR(event->GetRxPowerW(),txVector.GetChannelWidth());
  
  tolerable = event->GetRxPowerW() * (SNR - minSINR) / (SNR * minSINR);
  Time preambleduration = CalculatePlcpPreambleAndHeaderDuration(txVector);
  

  NS_LOG_DEBUG ("snr(dB)=" << RatioToDb (snrPer.snr) << ", per=" << snrPer.per);
  //std::cout<<"mode "<<txMode<< " minsnr :"<<minSINR<<std::endl;
  if ((m_random->GetValue () > snrPer.per) || (Simulator::Now().GetSeconds() <0.999 ) ) //plcp reception succeeded
    {
      if (IsModeSupported (txMode) || IsMcsSupported (txMode))
        {
          NS_LOG_DEBUG ("receiving plcp payload"); //endReceive is already scheduled
          Ptr<Node> node= GetDevice()->GetNode ();
          Ptr<WifiNetDevice> device = GetDevice()->GetObject<WifiNetDevice>();
          Ptr<DcaTxop> dca = device->GetMac()->GetObject<RegularWifiMac>()->getdca();
          Ptr<MacLow> low = dca->GetLow();
          if (!IsData)
          { 
            nPacket->RemoveHeader (hdr);
            //std::cout<<"tolerate : "<<event->GetRxPowerW() * (SNR - minSINR) / (SNR * minSINR)<<std::endl;
            if(GetDevice()->GetAddress() == hdr.GetAddr1() && GetDevice()->GetIfIndex()==0)
            {
              if(low->ifneedrts())
                {
                  if((hdr.IsRts () || hdr.IsCts() || hdr.IsAck()) && tolerable>=0)
                    {
                      m_state->NotifyToleranceInterference(tolerable,txVector,rxDuration-preambleduration+MicroSeconds(10),hdr);/////計算還可以忍受的干擾 傳給WifiPhyStateHelper::NotifyToleranceInterference()
                    }
                }
              //if(hdr.IsAck())
              //{
                //m_state->NotifyToleranceInterference(tolerable,txVector,rxDuration-preambleduration+MicroSeconds(10),hdr);/////計算還可以忍受的干擾 傳給WifiPhyStateHelper::NotifyToleranceInterference()

              //}
            }
            /*if(GetDevice()->GetIfIndex()==1)
              {
                Ptr<Node> node = GetDevice()->GetNode();
                Ptr<WifiNetDevice> seconddevice = node ->GetDevice(0)->GetObject<WifiNetDevice>();
                Ptr<DcaTxop> secondca = seconddevice->GetMac()->GetObject<RegularWifiMac>()->getdca();
                Ptr<MacLow> second_low = secondca->GetLow();
                
                if(hdr.Ispowercontrol() && hdr.GetAddr1()==second_low->GetAddress())
                {
                  
                  local_gain = gain;
                }
                if(hdr.Ispowercontrol())
                {
                  double txpower_ofpacket =DbmToW(double(hdr.Gettxpower()));
                  double gain = rxPowerW/txpower_ofpacket;
                  uint16_t toleratew = hdr.Gettolerate();
                  double tol = (const char)toleratew;

                  double suggestpower;
                  suggestpower = DbmToW(tol)/gain;
                  if(hdr.GetAddr1()!=second_low->GetAddress())
                  {
                    
                    //std::cout<<tol<<"   "<<suggestpower<<"  "<<txpower_ofpacket<<"  "<<rxPowerW<<"  "<<local_gain<<"  "<<gain<<"  "<<hdr.GetDuration().GetSeconds()<<std::endl;
                    Time preambleheadertime = CalculatePlcpPreambleAndHeaderDuration (txVector);
                    txpowerlist.insert(txpowerlist.begin(),Transmitpower(suggestpower,hdr.GetDuration()+Simulator::Now(),local_gain));

                  
                    deletepowerevent = Simulator::Schedule (hdr.GetDuration(),
                                    &WifiPhy::Resetpowerlist,this);
                  }
                }
              }*/
            
            //std::cout<<"node "<<GetDevice()->GetNode ()->GetId ()<<" device "<<GetDevice()->GetIfIndex()<<" receive SNR is "<<SNR<<" rxpower "<<event->GetRxPowerW()<<" torpower "<<tolerable<<" at time : "<<Simulator::Now().GetMicroSeconds()<<std::endl;
          }
          else 
          {
            MpduAggregator::DeaggregatedMpdus packets = MpduAggregator::Deaggregate (nPacket);
            MpduAggregator::DeaggregatedMpdusCI n = packets.begin ();
            WifiMacHeader firsthdr;
            (*n).first->PeekHeader (firsthdr);
            if(firsthdr.GetAddr1 () == GetDevice()->GetAddress()&& GetDevice()->GetIfIndex()==0)
              {
                if(/*firsthdr.IsQosData() &&*/ !low->ifneedrts() && tolerable>=0) 
                {               
                  //std::cout<<"rxduration :"<<(rxDuration-preambleduration+MicroSeconds(17)).GetSeconds()<<std::endl;
                  m_state->NotifyToleranceInterference(tolerable,txVector,rxDuration-preambleduration+MicroSeconds(40),firsthdr);/////計算還可以忍受的干擾 傳給WifiPhyStateHelper::NotifyToleranceInterference()
                }
                  
              }
            
          }
          m_plcpSuccess = true;
        }
      else //mode is not allowed
        {
          NS_LOG_DEBUG ("drop packet because it was sent using an unsupported mode (" << txMode << ")");
          NotifyRxDrop (packet);
          m_plcpSuccess = false;
        }
    }
  else //plcp reception failed
    {
      NS_LOG_DEBUG ("drop packet because plcp preamble/header reception failed");
      NotifyRxDrop (packet);
      m_plcpSuccess = false;
    }
}

void
WifiPhy::EndReceive (Ptr<Packet> packet, WifiPreamble preamble, MpduType mpdutype, Ptr<InterferenceHelper::Event> event)
{
  if(GetDevice()->GetIfIndex()==0 && Simulator::Now().GetSeconds() >1)
  {
    NS_LOG_FUNCTION (this << packet << event);
    NS_ASSERT (IsStateRx ());
    NS_ASSERT (event->GetEndTime () == Simulator::Now ());
    
    InterferenceHelper::SnrPer snrPer;
    snrPer = m_interference.CalculatePlcpPayloadSnrPer (event);
    m_interference.NotifyRxEnd ();
    m_currentEvent = 0;

  ////////////////////////////////////////////////////////////////////////////
        bool IsDataOrNot;
        bool IsBARorNot = false;
        bool isrts = false;
        bool iscts = false;
        Mac48Address address;
        AmpduTag mpdu;
        Ptr<Packet>nPacket = packet->Copy();
        IsDataOrNot = nPacket->RemovePacketTag(mpdu);
        if( IsDataOrNot )
        {
                  MpduAggregator::DeaggregatedMpdus packets = MpduAggregator::Deaggregate (nPacket);
                  MpduAggregator::DeaggregatedMpdusCI n = packets.begin ();
                  WifiMacHeader firsthdr;
                  (*n).first->PeekHeader (firsthdr);
                  address = firsthdr.GetAddr1();
        }
        else
        {        
                  WifiMacHeader hdr;
                  nPacket->PeekHeader(hdr);
                  address = hdr.GetAddr1();
                  if( hdr.IsBlockAckReq())
                    IsBARorNot = true;
                  if(hdr.IsRts())
                    isrts = true;
                  else if(hdr.IsCts())
                  {
                   //std::cout<<GetDevice()->GetNode()->GetId();
                   //std::cout<<"cts!!!!!!!!\n";
                    iscts = true;
                  }
                  else
                  {
                    //std::cout<<GetDevice()->GetNode()->GetId();
                    //std::cout<<"something else~~~~~~~~~~~~~~\n";
                  }
                  
        }

  ////////////////////////////////////////////////////////////////////////////

    if (m_plcpSuccess == true)
      {
        NS_LOG_DEBUG ("mode=" << (event->GetPayloadMode ().GetDataRate (event->GetTxVector ())) <<
                      ", snr(dB)=" << RatioToDb (snrPer.snr) << ", per=" << snrPer.per << ", size=" << packet->GetSize ());
  ////std::cout<<"random "<<m_random->GetValue ()<<" per "<<snrPer.per<<" "<<Simulator::Now().GetMicroSeconds()<<std::endl;
        if ((m_random->GetValue () > snrPer.per) || (Simulator::Now().GetSeconds() <0.999 ) )
          {
            NotifyRxEnd (packet);
            SignalNoiseDbm signalNoise;
            signalNoise.signal = RatioToDb (event->GetRxPowerW ()) + 30;
            signalNoise.noise = RatioToDb (event->GetRxPowerW () / snrPer.snr) + 30;
            MpduInfo aMpdu;
            aMpdu.type = mpdutype;
            aMpdu.mpduRefNumber = m_rxMpduReferenceNumber;
            NotifyMonitorSniffRx (packet, GetFrequency (), event->GetTxVector (), aMpdu, signalNoise);
            if(Simulator::Now().GetSeconds()> time && GetDevice()->GetIfIndex()==0)/////liang
            {      //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<" switchfromrxendok at "<<Simulator::Now().GetMicroSeconds()<<std::endl;
                  if((IsDataOrNot || IsBARorNot) && address == GetDevice()->GetAddress() )
                    m_state->HasReserveForAck(true);/////////////////////liang
                  if(isrts && address == GetDevice()->GetAddress())
                    m_state->HasReserveForCts(true);/////////////////////liang

                  if(isrts || iscts)
                  {
                    //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ();
                    m_state->SwitchFromRxEndOk (packet, snrPer.snr, event->GetTxVector ());
                  }
                  else if(Simulator::Now().GetSeconds()> time && GetDevice()->GetIfIndex()==0 && address ==  GetDevice()->GetAddress())
                  {
                    //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ();
                    m_state->SwitchFromRxEndOk (packet, snrPer.snr, event->GetTxVector (),address);
                  }
                  else
                  {
                    //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ();
                    m_state->SwitchFromRxEndOk (packet, snrPer.snr, event->GetTxVector ());
                  }
            }
            else
              m_state->SwitchFromRxEndOk (packet, snrPer.snr, event->GetTxVector ());
          }
        else
          {
            /* failure. */
            NotifyRxDrop (packet);
                  ///std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<" switchfromrxenderror at "<<Simulator::Now().GetMicroSeconds()<<std::endl;
            if(iscts || isrts)
            {
              //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ();
              m_state->SwitchFromRxEndError (packet, snrPer.snr);
            }
            else if(Simulator::Now().GetSeconds()> time && GetDevice()->GetIfIndex()==0)
            {
              //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ();
              m_state->SwitchFromRxEndError (packet, snrPer.snr, address);
            }
            else
            {
              //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ();
              m_state->SwitchFromRxEndError (packet, snrPer.snr);
            }
          }
      }
    else
      {
            /*if(iscts || isrts)
              m_state->SwitchFromRxEndError (packet, snrPer.snr);
            else if(Simulator::Now().GetSeconds()> time && GetDevice()->GetIfIndex()==0)
              m_state->SwitchFromRxEndError (packet, snrPer.snr, address);
            else*/
              m_state->SwitchFromRxEndError (packet, snrPer.snr);
      }

    if (preamble == WIFI_PREAMBLE_NONE && mpdutype == LAST_MPDU_IN_AGGREGATE)
      {
        m_plcpSuccess = false;
      }
  }
  else
  {
     NS_LOG_FUNCTION (this << packet << event);
  NS_ASSERT (IsStateRx ());
  NS_ASSERT (event->GetEndTime () == Simulator::Now ());

  InterferenceHelper::SnrPer snrPer;
  snrPer = m_interference.CalculatePlcpPayloadSnrPer (event);
  m_interference.NotifyRxEnd ();
  m_currentEvent = 0;

  if (m_plcpSuccess == true)
    {
      NS_LOG_DEBUG ("mode=" << (event->GetPayloadMode ().GetDataRate (event->GetTxVector ())) <<
                    ", snr(dB)=" << RatioToDb (snrPer.snr) << ", per=" << snrPer.per << ", size=" << packet->GetSize ());

      if (m_random->GetValue () > snrPer.per)
        {
          NotifyRxEnd (packet);
          SignalNoiseDbm signalNoise;
          signalNoise.signal = RatioToDb (event->GetRxPowerW ()) + 30;
          signalNoise.noise = RatioToDb (event->GetRxPowerW () / snrPer.snr) + 30;
          MpduInfo aMpdu;
          aMpdu.type = mpdutype;
          aMpdu.mpduRefNumber = m_rxMpduReferenceNumber;
          NotifyMonitorSniffRx (packet, GetFrequency (), event->GetTxVector (), aMpdu, signalNoise);
          m_state->SwitchFromRxEndOk (packet, snrPer.snr, event->GetTxVector ());
        }
      else
        {
          /* failure. */
          NotifyRxDrop (packet);
          m_state->SwitchFromRxEndError (packet, snrPer.snr);
        }
    }
  else
    {
      m_state->SwitchFromRxEndError (packet, snrPer.snr);
    }

  if (preamble == WIFI_PREAMBLE_NONE && mpdutype == LAST_MPDU_IN_AGGREGATE)
    {
      m_plcpSuccess = false;
    }
  }
  

}
void
WifiPhy::ReceiveWrongPacket()
{////std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<" switchfromwrong at "<<Simulator::Now().GetMicroSeconds()<<std::endl;
        m_interference.NotifyRxEnd();
        m_state->SwitchFromRxBecauseWrongAddress();
        m_currentEvent = 0;
}


// Clause 15 rates (DSSS)

WifiMode
WifiPhy::GetDsssRate1Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate1Mbps",
                                     WIFI_MOD_CLASS_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDsssRate2Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate2Mbps",
                                     WIFI_MOD_CLASS_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     4);
  return mode;
}


// Clause 18 rates (HR/DSSS)

WifiMode
WifiPhy::GetDsssRate5_5Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate5_5Mbps",
                                     WIFI_MOD_CLASS_HR_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetDsssRate11Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate11Mbps",
                                     WIFI_MOD_CLASS_HR_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     256);
  return mode;
}


// Clause 19.5 rates (ERP-OFDM)

WifiMode
WifiPhy::GetErpOfdmRate6Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate6Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate9Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate9Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate12Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate12Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate18Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate18Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate24Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate24Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate36Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate36Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate48Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate48Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate54Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate54Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}


// Clause 17 rates (OFDM)

WifiMode
WifiPhy::GetOfdmRate6Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate9Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate9Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate12Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate12Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate18Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate18Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate24Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate24Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate36Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate36Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate48Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate48Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate54Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate54Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}


// 10 MHz channel rates

WifiMode
WifiPhy::GetOfdmRate3MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate4_5MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_5MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate6MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate9MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate9MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate12MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate12MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate18MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate18MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate24MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate24MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate27MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate27MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}


// 5 MHz channel rates

WifiMode
WifiPhy::GetOfdmRate1_5MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate2_25MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_25MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate3MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate4_5MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate6MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate9MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate9MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate12MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate12MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate13_5MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate13_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}

//802.11ah
   
WifiMode
WifiPhy::GetOfdmRate300KbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate300KbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 300000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate333_3KbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate333_3KbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 333300,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate600KbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate600KbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 600000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate666_7KbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate666_7KbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 666700,
                                     WIFI_CODE_RATE_1_2,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate900KbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate900KbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 900000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate1MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 1000000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate1_2MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_2MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 1200000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate1_333_3MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_333_3MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 1333300,
                                     WIFI_CODE_RATE_1_2,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate1_8MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_8MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 1800000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate2MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 2000000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate2_4MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_4MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 2400000,
                                     WIFI_CODE_RATE_2_3,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate2_666_7MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_666_7MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 2666700,
                                     WIFI_CODE_RATE_2_3,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate2_7MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_7MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 2700000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate3MbpsBW1MHzShGi ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3MbpsBW1MHzShGi",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 3000000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate3MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 3000000,
                                     WIFI_CODE_RATE_5_6,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate3_333_3MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3_333_3MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 3333300,
                                     WIFI_CODE_RATE_5_6,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate3_6MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3_6MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  1000000, 3600000,
                                     WIFI_CODE_RATE_3_4,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate4MbpsBW1MHzShGi ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4MbpsBW1MHzShGi",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  1000000, 4000000,
                                     WIFI_CODE_RATE_3_4,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate4MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  1000000, 4000000,
                                     WIFI_CODE_RATE_5_6,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate4_444_4MbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_444_4MbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  1000000, 4444400,
                                     WIFI_CODE_RATE_5_6,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate150KbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate150KbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 150000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate166_7KbpsBW1MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate166_7KbpsBW1MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  1000000, 166700,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate650KbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate650KbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 650000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate722_2KbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate722_2KbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 722200,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate1_3MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_3MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 1300000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate1_444_4MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_444_4MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 1444400,
                                     WIFI_CODE_RATE_1_2,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate1_95MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_95MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 1950000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate2_166_7MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_166_7MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 2166700,
                                     WIFI_CODE_RATE_3_4,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate2_6MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_6MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 2600000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate2_8889MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_8889MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 2888900,
                                     WIFI_CODE_RATE_1_2,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate3_9MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3_9MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 3900000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate4_333_3MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_333_3MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 4333300,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate5_2MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate5_2MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 5200000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate5_777_8MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate5_777_8MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 5777800,
                                     WIFI_CODE_RATE_2_3,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate5_85MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate5_85MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 5850000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate6_5MbpsBW2MHzShGi ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6_5MbpsBW2MHzShGi",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 6500000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate6_5MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6_5MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 6500000,
                                     WIFI_CODE_RATE_5_6,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate7_222_2MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate7_222_2MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     true,
                                    //  2000000, 7222200,
                                     WIFI_CODE_RATE_5_6,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate7_8MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate7_8MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  2000000, 7800000,
                                     WIFI_CODE_RATE_3_4,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate8_666_7MbpsBW2MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate8_666_7MbpsBW2MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  2000000, 8666700,
                                     WIFI_CODE_RATE_3_4,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate1_35MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_35MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 1350000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate1_5MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_5MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 1500000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate2_7MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_7MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 2700000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate3MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 3000000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate4_05MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_05MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 4050000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate4_5MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_5MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 4500000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate5_4MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate5_4MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 5400000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate6MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 6000000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate8_1MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate8_1MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 8100000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate9MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate9MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 9000000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate10_8MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate10_8MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 10800000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate12MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("GetOfdmRate12MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 12000000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
    return mode;
}
    
WifiMode
WifiPhy::GetOfdmRate12_15MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate12_15MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 12150000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
    return mode;
}
    
WifiMode
WifiPhy::GetOfdmRate13_5MbpsBW4MHzShGi ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate13_5MbpsBW4MHzShGi",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 13500000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate13_5MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate13_5MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 13500000,
                                     WIFI_CODE_RATE_5_6,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate15MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate15MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 15000000,
                                     WIFI_CODE_RATE_5_6,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate16_2MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate16_2MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 16200000,
                                     WIFI_CODE_RATE_3_4,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate18MbpsBW4MHzShGi ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate18MbpsBW4MHzShGi",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 18000000,
                                     WIFI_CODE_RATE_3_4,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate18MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate18MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 18000000,
                                     WIFI_CODE_RATE_5_6,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate20MbpsBW4MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate20MbpsBW4MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  4000000, 20000000,
                                     WIFI_CODE_RATE_5_6,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate2_925MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_925MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 2925000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate3_25MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3_25MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 3250000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate5_85MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate5_85MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 5850000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate6_5MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("GetOfdmRate6_5MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 6500000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate8_775MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate8_775MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 8775000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate9_75MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate9_75MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 9750000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate11_7MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate11_7MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 11700000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate13MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate13MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 13000000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate17_55MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate17_55MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 17550000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate19_5MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate19_5MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 19500000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate23_4MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate23_4MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 23400000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate26MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate26MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 26000000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate26_325MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate26_325MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 26325000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate29_25MbpsBW8MHzShGi ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate29_25MbpsBW8MHzShGi",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 29250000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate29_25MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate29_25MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 29250000,
                                     WIFI_CODE_RATE_5_6,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate32_5MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate32_5MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 32500000,
                                     WIFI_CODE_RATE_5_6,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate35_1MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate35_1MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 35100000,
                                     WIFI_CODE_RATE_3_4,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate39MbpsBW8MHzShGi ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate39MbpsBW8MHzShGi",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 39000000,
                                     WIFI_CODE_RATE_3_4,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate39MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate39MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 39000000,
                                     WIFI_CODE_RATE_5_6,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate43_333_3MbpsBW8MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate43_333_3MbpsBW8MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  8000000, 43333300,
                                     WIFI_CODE_RATE_5_6,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate5_85MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate5_85MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 5850000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate6_5MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6_5MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 6500000,
                                     WIFI_CODE_RATE_1_2,
                                     2);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate11_7MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate11_7MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 11700000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate13MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate13MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 13000000,
                                     WIFI_CODE_RATE_1_2,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate17_55MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate17_55MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 17550000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate19_5MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate19_5MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 19500000,
                                     WIFI_CODE_RATE_3_4,
                                     4);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate23_4MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate23_4MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 23400000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate26MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate26MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 26000000,
                                     WIFI_CODE_RATE_1_2,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate35_1MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate35_1MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 35100000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate39MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate39MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 39000000,
                                     WIFI_CODE_RATE_3_4,
                                     16);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate46_8MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate46_8MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 46800000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate52MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate52MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 52000000,
                                     WIFI_CODE_RATE_2_3,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate52_65MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate52_65MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 52650000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate58_5MbpsBW16MHzShGi ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate58_5MbpsBW16MHzShGi",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 58500000,
                                     WIFI_CODE_RATE_3_4,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate58_5MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate58_5MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 58500000,
                                     WIFI_CODE_RATE_5_6,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate65MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate65MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 65000000,
                                     WIFI_CODE_RATE_5_6,
                                     64);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate70_2MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate70_2MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 70200000,
                                     WIFI_CODE_RATE_3_4,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate78MbpsBW16MHzShGi ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate78MbpsBW16MHzShGi",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 78000000,
                                     WIFI_CODE_RATE_3_4,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate78MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate78MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 78000000,
                                     WIFI_CODE_RATE_5_6,
                                     256);
    return mode;
}

WifiMode
WifiPhy::GetOfdmRate86_666_7MbpsBW16MHz ()
{
    static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate86_666_7MbpsBW16MHz",
                                     WIFI_MOD_CLASS_S1G,
                                     false,
                                    //  16000000, 86666700,
                                     WIFI_CODE_RATE_5_6,
                                     256);
    return mode;
}


// Clause 20

WifiMode
WifiPhy::GetHtMcs0 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs0", 0, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs1 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs1", 1, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs2 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs2", 2, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs3 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs3", 3, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs4 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs4", 4, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs5 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs5", 5, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs6 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs6", 6, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs7 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs7", 7, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs8 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs8", 8, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs9 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs9", 9, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs10 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs10", 10, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs11 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs11", 11, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs12 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs12", 12, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs13 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs13", 13, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs14 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs14", 14, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs15 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs15", 15, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs16 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs16", 16, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs17 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs17", 17, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs18 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs18", 18, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs19 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs19", 19, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs20 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs20", 20, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs21 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs21", 21, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs22 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs22", 22, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs23 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs23", 23, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs24 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs24", 24, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs25 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs25", 25, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs26 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs26", 26, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs27 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs27", 27, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs28 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs28", 28, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs29 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs29", 29, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs30 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs30", 30, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs31 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs31", 31, WIFI_MOD_CLASS_HT);
  return mcs;
}


// Clause 22

WifiMode
WifiPhy::GetVhtMcs0 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs0", 0, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs1 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs1", 1, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs2 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs2", 2, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs3 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs3", 3, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs4 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs4", 4, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs5 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs5", 5, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs6 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs6", 6, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs7 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs7", 7, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs8 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs8", 8, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs9 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs9", 9, WIFI_MOD_CLASS_VHT);
  return mcs;
}

// WifiMode // Jonathan
// WifiPhy::GetVhtMcs10 ()
// {
//   static WifiMode mcs =
//     WifiModeFactory::CreateWifiMcs ("VhtMcs10", 10, WIFI_MOD_CLASS_VHT);
//   return mcs;
// }


// Clause 23 

WifiMode
WifiPhy::GetS1gMcs0 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("S1gMcs0", 0, WIFI_MOD_CLASS_S1G);
  return mcs;
}

WifiMode
WifiPhy::GetS1gMcs1 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("S1gMcs1", 1, WIFI_MOD_CLASS_S1G);
  return mcs;
}

WifiMode
WifiPhy::GetS1gMcs2 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("S1gMcs2", 2, WIFI_MOD_CLASS_S1G);
  return mcs;
}

WifiMode
WifiPhy::GetS1gMcs3 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("S1gMcs3", 3, WIFI_MOD_CLASS_S1G);
  return mcs;
}

WifiMode
WifiPhy::GetS1gMcs4 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("S1gMcs4", 4, WIFI_MOD_CLASS_S1G);
  return mcs;
}

WifiMode
WifiPhy::GetS1gMcs5 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("S1gMcs5", 5, WIFI_MOD_CLASS_S1G);
  return mcs;
}

WifiMode
WifiPhy::GetS1gMcs6 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("S1gMcs6", 6, WIFI_MOD_CLASS_S1G);
  return mcs;
}

WifiMode
WifiPhy::GetS1gMcs7 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("S1gMcs7", 7, WIFI_MOD_CLASS_S1G);
  return mcs;
}

WifiMode
WifiPhy::GetS1gMcs8 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("S1gMcs8", 8, WIFI_MOD_CLASS_S1G);
  return mcs;
}

WifiMode
WifiPhy::GetS1gMcs9 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("S1gMcs9", 9, WIFI_MOD_CLASS_S1G);
  return mcs;
}

WifiMode
WifiPhy::GetS1gMcs10 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("S1gMcs10", 10, WIFI_MOD_CLASS_S1G);
  return mcs;
}


// Clause 26

WifiMode
WifiPhy::GetHeMcs0 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs0", 0, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs1 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs1", 1, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs2 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs2", 2, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs3 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs3", 3, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs4 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs4", 4, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs5 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs5", 5, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs6 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs6", 6, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs7 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs7", 7, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs8 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs8", 8, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs9 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs9", 9, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs10 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs10", 10, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs11 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs11", 11, WIFI_MOD_CLASS_HE);
  return mcs;
}

bool
WifiPhy::IsValidTxVector (WifiTxVector txVector)
{
  uint8_t chWidth = txVector.GetChannelWidth ();
  uint8_t nss = txVector.GetNss ();
  std::string modeName = txVector.GetMode ().GetUniqueName ();

  if (chWidth == 20)
    {
      if (nss != 3 && nss != 6)
        {
          return (modeName != "VhtMcs9");
        }
    }
  else if (chWidth == 80)
    {
      if (nss == 3 || nss == 7)
        {
          return (modeName != "VhtMcs6");
        }
      else if (nss == 6)
        {
          return (modeName != "VhtMcs9");
        }
    }
  else if (chWidth == 160)
    {
      if (nss == 3)
        {
          return (modeName != "VhtMcs9");
        }
    }

  return true;
}

bool
WifiPhy::IsModeSupported (WifiMode mode) const
{
  for (uint32_t i = 0; i < GetNModes (); i++)
    {
      if (mode == GetMode (i))
        {
          return true;
        }
    }
  return false;
}

bool
WifiPhy::IsMcsSupported (WifiMode mcs) const
{
  for (uint32_t i = 0; i < GetNMcs (); i++)
    {
      if (mcs == GetMcs (i))
        {
          return true;
        }
    }
  return false;
}

uint32_t
WifiPhy::GetNModes (void) const
{
  return m_deviceRateSet.size ();
}

WifiMode
WifiPhy::GetMode (uint32_t mode) const
{
  return m_deviceRateSet[mode];
}

uint8_t
WifiPhy::GetNMcs (void) const
{
  return m_deviceMcsSet.size ();
}

WifiMode
WifiPhy::GetMcs (int mcs) const
{
  return m_deviceMcsSet[mcs];
}

bool
WifiPhy::IsStateCcaBusy (void)
{
  return m_state->IsStateCcaBusy ();
}

bool
WifiPhy::IsStateIdle (void)
{
  return m_state->IsStateIdle ();
}

bool
WifiPhy::IsStateBusy (void)
{
  return m_state->IsStateBusy ();
}

bool
WifiPhy::IsStateRx (void)
{
  return m_state->IsStateRx ();
}

bool
WifiPhy::IsStateTx (void)
{
  return m_state->IsStateTx ();
}

bool
WifiPhy::IsStateSwitching (void)
{
  return m_state->IsStateSwitching ();
}

bool
WifiPhy::IsStateSleep (void)
{
  return m_state->IsStateSleep ();
}

Time
WifiPhy::GetStateDuration (void)
{
  return m_state->GetStateDuration ();
}

Time
WifiPhy::GetDelayUntilIdle (void)
{
  return m_state->GetDelayUntilIdle ();
}

Time
WifiPhy::GetLastRxStartTime (void) const
{
  return m_state->GetLastRxStartTime ();
}

void
WifiPhy::SwitchMaybeToCcaBusy (void)
{
  NS_LOG_FUNCTION (this);
  //We are here because we have received the first bit of a packet and we are
  //not going to be able to synchronize on it
  //In this model, CCA becomes busy when the aggregation of all signals as
  //tracked by the InterferenceHelper class is higher than the CcaBusyThreshold

  Time delayUntilCcaEnd = m_interference.GetEnergyDuration (DbmToW (GetCcaMode1Threshold ()));
  if (!delayUntilCcaEnd.IsZero ())
    {
      NS_LOG_DEBUG ("Calling SwitchMaybeToCcaBusy for " << delayUntilCcaEnd.As (Time::S));
      m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd);
    }
}

void
WifiPhy::AbortCurrentReception ()
{
  NS_LOG_FUNCTION (this);
  if (m_endPlcpRxEvent.IsRunning ())
    {
      m_endPlcpRxEvent.Cancel ();
    }
  if (m_endRxEvent.IsRunning ())
    {
      m_endRxEvent.Cancel ();
    }
  NotifyRxDrop (m_currentEvent->GetPacket ());
  m_interference.NotifyRxEnd ();
  m_state->SwitchFromRxAbort ();
  m_currentEvent = 0;
}

double////liang
WifiPhy::GetSNR(double rxPowerW, uint8_t channelWidth)
{
  //thermal noise at 290K in J/s = W
  static const double BOLTZMANN = 1.3803e-23;
  //Nt is the power of thermal noise in W
  double Nt = BOLTZMANN * 290.0 * channelWidth * 1000000;
  double m_noiseFigure = 5.01187;
  //receiver noise Floor (W) which accounts for thermal noise and non-idealities of the receiver
  double noiseFloor = m_noiseFigure * Nt;
  double noiseInterference = this->GetOthersTxpowerW() - rxPowerW;
  double noise = noiseFloor + noiseInterference;
  ///std::cout<<noiseInterference<<std::endl;
  double snr = rxPowerW / noise; 
  return snr;
}
double 
WifiPhy::GetOthersTxpowerW(){
  return DbmToW (m_txpowerDbm);
}

double ////////////////////////liang
WifiPhy::GetTxPowerDbm()
{
  return m_txpowerDbm;
}

void////////////////////liang
WifiPhy::SetTxPowerDbm(double txpower,Time duration)
{/////std::cout<<" No."<<GetDevice()->GetNode ()->GetId ()<<" device txpower is"<<txpower<<" "<<duration.GetMicroSeconds()<<" at "<<Simulator::Now().GetMicroSeconds()<<std::endl;
        m_txpowerDbm = txpower;
        Simulator::Schedule(duration, &WifiPhy::Turnoff, this);
}

void///liang
WifiPhy::Turnoff()
{
        m_txpowerDbm = -120;
////std::cout<<" No."<<GetDevice()->GetNode ()->GetId ()<<" device txpower is"<<m_txpowerW<<" at "<<Simulator::Now().GetMicroSeconds()<<std::endl;
}

void
WifiPhy::ResetAddressRegister()
{
        m_addressregister.clear();
}

uint8_t 
WifiPhy::GetAddressRegisterSize()
{
        return m_addressregister.size();
}
void
WifiPhy::Resetpowerlist()
{
  
  std::vector <Transmitpower>::iterator Iter;
  Time now = Simulator::Now();
  for(Iter=txpowerlist.begin();Iter!=txpowerlist.end();)
  {
    //std::cout<<now<<"  "<<Iter->duration<<std::endl;
      if(now >= Iter->duration)
      {
        Iter = txpowerlist.erase(Iter);
      }
      else 
      {
        ++Iter;
      }
  }
   //std::cout<<"done\n";
}
void
WifiPhy::StartRx (Ptr<Packet> packet, WifiTxVector txVector, MpduType mpdutype, double rxPowerW, Time rxDuration, Ptr<InterferenceHelper::Event> event)
{

if(Simulator::Now().GetSeconds()> time && GetDevice()->GetIfIndex()==0)
{
  /************************************************************************************************************************/
  NS_LOG_FUNCTION (this << packet << txVector << (uint16_t)mpdutype << rxPowerW << rxDuration);
  bool PacketIsMind = false;
  bool IsAckorNot = false;
  if (rxPowerW > GetEdThresholdW ())
    {
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      if(Simulator::Now().GetSeconds() > time /*&& GetDevice()->GetIfIndex()==0*/)
      {
                AmpduTag mpdu;
                Ptr<Packet>nPacket = packet->Copy();
                bool IsData = nPacket->RemovePacketTag(mpdu);
                bool flag = true;
                uint8_t i;
                  if(IsData)
                    {     
                      MpduAggregator::DeaggregatedMpdus packets = MpduAggregator::Deaggregate (nPacket);
                      MpduAggregator::DeaggregatedMpdusCI n = packets.begin ();
                      WifiMacHeader firsthdr;
                      (*n).first->PeekHeader (firsthdr);
                      //uint16_t txpowerofpacket = firsthdr.Gettxpower();
                      //double txp = (const char)txpowerofpacket;
                      //std::cout<<"data packet"<<"  "<<double(txp)<<"gain :"<<gain<<std::endl;
                      if(firsthdr.GetAddr1()==GetDevice()->GetAddress() )
                      {
                        double txpower_ofpacket =double(txVector.GetTxPowerFromVector());
                        local_gain = rxPowerW/txpower_ofpacket;
                        //std::cout<<GetDevice()->GetAddress()<<"  "<<local_gain<<"  "<<txpower_ofpacket<<std::endl;
                      }
                      //std::cout<<txpower_ofpacket<<std::endl;
                      if(m_addressregister.size() == 0)
                      {
                        m_addressregister.insert(m_addressregister.begin(),firsthdr.GetAddr2());
                      }
                      for(i=0;i<m_addressregister.size();++i)
                      {
                            if(firsthdr.GetAddr2() == m_addressregister[i])
                                    flag = false;
                      }
                      if(flag)
                      {
                              m_addressregister.insert(m_addressregister.begin(),firsthdr.GetAddr2());
                      }
                    }
                  else
                    {
                      WifiMacHeader hdr;
                      nPacket->PeekHeader(hdr);

                      if(m_addressregister.size() == 0)
                              m_addressregister.insert(m_addressregister.begin(),hdr.GetAddr2());
                      for(i=0;i<m_addressregister.size();++i)
                      {
                              if(hdr.GetAddr2() == m_addressregister[i]){
                                      flag = false;
                              }
                      }
                      if(flag)
                      {
                              m_addressregister.insert(m_addressregister.begin(),hdr.GetAddr2());
                      }
                    }       
      }
      //std::cout<<GetDevice()->GetNode()->GetId()<<" have "<<m_addressregister.size()<<" neighbor "<<Simulator::Now().GetMicroSeconds()<<std::endl;
      ///////////////////////////////std::cout<<GetDevice()->GetNode()->GetId()<<" have data"<<std::endl;  ///////////////////////////////
      AmpduTag ampduTag;
      WifiPreamble preamble = txVector.GetPreambleType ();
      if (preamble == WIFI_PREAMBLE_NONE && (m_mpdusNum == 0 || m_plcpSuccess == false))
        {
          m_plcpSuccess = false;
          m_mpdusNum = 0;
          NS_LOG_DEBUG ("drop packet because no PLCP preamble/header has been received");
          NotifyRxDrop (packet);
          MaybeCcaBusyDuration ();
          return;
        }
      else if (preamble != WIFI_PREAMBLE_NONE && packet->PeekPacketTag (ampduTag) && m_mpdusNum == 0)
        {
          //received the first MPDU in an MPDU
          m_mpdusNum = ampduTag.GetRemainingNbOfMpdus ();
          m_rxMpduReferenceNumber++;
        }
      else if (preamble == WIFI_PREAMBLE_NONE && packet->PeekPacketTag (ampduTag) && m_mpdusNum > 0)
        {
          //received the other MPDUs that are part of the A-MPDU
          if (ampduTag.GetRemainingNbOfMpdus () < (m_mpdusNum - 1))
            {
              NS_LOG_DEBUG ("Missing MPDU from the A-MPDU " << m_mpdusNum - ampduTag.GetRemainingNbOfMpdus ());
              m_mpdusNum = ampduTag.GetRemainingNbOfMpdus ();
            }
          else
            {
              m_mpdusNum--;
            }
        }
      else if (preamble != WIFI_PREAMBLE_NONE && packet->PeekPacketTag (ampduTag) && m_mpdusNum > 0)
        {
          NS_LOG_DEBUG ("New A-MPDU started while " << m_mpdusNum << " MPDUs from previous are lost");
          m_mpdusNum = ampduTag.GetRemainingNbOfMpdus ();
        }
      else if (preamble != WIFI_PREAMBLE_NONE && m_mpdusNum > 0 )
        {
          NS_LOG_DEBUG ("Didn't receive the last MPDUs from an A-MPDU " << m_mpdusNum);
          m_mpdusNum = 0;
        }

      NS_LOG_DEBUG ("sync to signal (power=" << rxPowerW << "W)");
      m_currentEvent = event;
      ////////////////////////////////////////////////////////////////////////////////////////////
      AmpduTag mpdu;
      Time preambleheadertime = CalculatePlcpPreambleAndHeaderDuration (txVector);
      ////std::cout<<preambleheadertime.GetMicroSeconds()<<" "<<rxDuration.GetMicroSeconds()<<std::endl;
      Ptr<Packet> nPacket = packet->Copy();
      IsAckorNot = nPacket->RemovePacketTag(mpdu);

        if(IsAckorNot)
          {
                  MpduAggregator::DeaggregatedMpdus packets = MpduAggregator::Deaggregate (nPacket);
                  MpduAggregator::DeaggregatedMpdusCI n = packets.begin ();
                  WifiMacHeader firsthdr;
                  (*n).first->PeekHeader (firsthdr);
                  if(Mac48Address::ConvertFrom(GetDevice()->GetAddress()) == firsthdr.GetAddr1())
                    PacketIsMind = true;
                    //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<" switchtorx at "<<Simulator::Now().GetMicroSeconds()<<std::endl;
                  if(PacketIsMind)
                  {
                    //std::cout<<"data "<<rxDuration<<"\n";
                    //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<" switchtorx at "<<Simulator::Now().GetMicroSeconds()<<" "<<rxDuration.GetMicroSeconds()<<std::endl;
                    m_state->SwitchToRx (rxDuration,firsthdr.GetAddr1());
                  }
                  else
                  {
                    
                    //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<" switchtorx at "<<Simulator::Now().GetMicroSeconds()<<" "<<preambleheadertime.GetMicroSeconds()<<std::endl;
                    m_state->SwitchToRx (preambleheadertime);
                  }
          }
        else
          {        
                  WifiMacHeader hdr;
                  nPacket->PeekHeader(hdr);
                  if(Mac48Address::ConvertFrom(GetDevice()->GetAddress()) == hdr.GetAddr1() || (hdr.IsCts() || hdr.IsRts()))
                  {

                    PacketIsMind = true;
                  }
                  //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<" switchtxrx at "<<Simulator::Now().GetMicroSeconds()<<std::endl;
                  if(hdr.IsCts() || hdr.IsRts())
                  {
                    //if(hdr.IsRts())
                      //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<"switch to rx because of rts\n";
                    //else if(hdr.IsCts())
                      //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<"switch to rx because of cts\n";
                    m_state->SwitchToRx (rxDuration);
                  }
                  else if(PacketIsMind)
                  {
                    
                    //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<" switchtorx at "<<Simulator::Now().GetMicroSeconds()<<" "<<rxDuration.GetMicroSeconds()<<std::endl;
                    m_state->SwitchToRx (rxDuration,hdr.GetAddr1());
                  }
                  else
                  {
                    //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<" switchtorx at "<<Simulator::Now().GetMicroSeconds()<<" "<<preambleheadertime.GetMicroSeconds()<<std::endl;
                    m_state->SwitchToRx (preambleheadertime);
                  }
          }
      ///////////////////////////////////////////////////////////////////////////////////////
      NS_ASSERT (m_endPlcpRxEvent.IsExpired ());
      NotifyRxBegin (packet);
      m_interference.NotifyRxStart ();
     
      if(PacketIsMind )
      {
        //std::cout<<" node "<<GetDevice()->GetNode ()->GetId ()<<"receive to myself\n";
        if (preamble != WIFI_PREAMBLE_NONE)
        {
          NS_ASSERT (m_endPlcpRxEvent.IsExpired ());
          Time preambleAndHeaderDuration = CalculatePlcpPreambleAndHeaderDuration (txVector);
          m_endPlcpRxEvent = Simulator::Schedule (preambleAndHeaderDuration, &WifiPhy::StartReceivePacket, this,
                                    packet, txVector, mpdutype, event, rxDuration,rxPowerW);
        }

        NS_ASSERT (m_endRxEvent.IsExpired ());
        m_endRxEvent = Simulator::Schedule (rxDuration, &WifiPhy::EndReceive, this,
                          packet, preamble, mpdutype, event);
      }
      else
      {
        NS_ASSERT (m_endRxWrongEvent.IsExpired ());
        m_endRxWrongEvent = Simulator::Schedule (preambleheadertime, &WifiPhy::ReceiveWrongPacket, this);
                                        
      }
    }
  else
    {
      //////////////////////////////////////////////////////////////////////
      Mac48Address address;
      AmpduTag mpdu;
      Ptr<Packet>nnPacket = packet->Copy();

      if(nnPacket->RemovePacketTag(mpdu))
      {
        MpduAggregator::DeaggregatedMpdus npackets = MpduAggregator::Deaggregate (nnPacket);
        MpduAggregator::DeaggregatedMpdusCI n = npackets.begin ();
        WifiMacHeader firsthdr;
        (*n).first->PeekHeader (firsthdr);
        address = firsthdr.GetAddr1();
      }
      else
      {        
              WifiMacHeader hdr;
              packet->PeekHeader(hdr);
              address = hdr.GetAddr1();
      }
      ////////////////////////////////////////////////////////////////////////
      NS_LOG_DEBUG ("drop packet because signal power too Small (" <<
                    rxPowerW << "<" << GetEdThresholdW () << ")");
      NotifyRxDrop (packet);
      m_plcpSuccess = false;
      //std::cout<<"drop packet because signal power too Small\n";
      MaybeCcaBusyDuration (address);

    }
 
    /************************************************************************************************************************/
}
else 
{
 //std::cout<<rxPowerW<<"        "<<GetEdThresholdW ()<<std::endl;
 NS_LOG_FUNCTION (this << packet << txVector << (uint16_t)mpdutype << rxPowerW << rxDuration);
   if (rxPowerW > GetEdThresholdW ())
    {
        AmpduTag mpdu;
        Ptr<Packet>nnPacket = packet->Copy();
        bool IsData = nnPacket->RemovePacketTag(mpdu);
        if( rxPowerW > DbmToW(GetCcaMode1Threshold()) )
        {
          //std::cout<<GetDevice()->GetIfIndex()<<" "<<Simulator::Now().GetSeconds()<<std::endl;
          if(!IsData)
            {
              WifiMacHeader hdr;
              nnPacket->PeekHeader(hdr);
              double txpower_ofpacket =DbmToW(double(hdr.Gettxpower()));
              double gain = rxPowerW/txpower_ofpacket;
              uint16_t toleratew = hdr.Gettolerate();
              double tol = (const char)toleratew;

              double suggestpower;
              suggestpower = DbmToW(tol)/gain;

              
              
              Ptr<Node> node = GetDevice()->GetNode();
              Ptr<WifiNetDevice> seconddevice = node ->GetDevice(0)->GetObject<WifiNetDevice>();
              Ptr<DcaTxop> secondca = seconddevice->GetMac()->GetObject<RegularWifiMac>()->getdca();
              Ptr<MacLow> second_low = secondca->GetLow();
              
              if(hdr.Ispowercontrol() && hdr.GetAddr1()==second_low->GetAddress())
              {
                //std::cout<<rxDuration<<"\n";
                local_gain = gain;
              }
              if(hdr.Ispowercontrol())
              {
                
                if(hdr.GetAddr1()!=second_low->GetAddress())
                {
                  
                  //std::cout<<tol<<"   "<<suggestpower<<"  "<<txpower_ofpacket<<"  "<<rxPowerW<<"  "<<local_gain<<"  "<<gain<<" duration "<<hdr.GetDuration().GetSeconds()<<"rx"<<rxDuration.GetSeconds()<<std::endl;
                  Time preambleheadertime = CalculatePlcpPreambleAndHeaderDuration (txVector);
                  txpowerlist.insert(txpowerlist.begin(),Transmitpower(suggestpower,hdr.GetDuration()+Simulator::Now(),local_gain));

                
                  deletepowerevent = Simulator::Schedule (hdr.GetDuration(),
                                  &WifiPhy::Resetpowerlist,this);
                }
              }
            }       
        }

      AmpduTag ampduTag;
      WifiPreamble preamble = txVector.GetPreambleType ();
      if (preamble == WIFI_PREAMBLE_NONE && (m_mpdusNum == 0 || m_plcpSuccess == false))
        {
          m_plcpSuccess = false;
          m_mpdusNum = 0;
          NS_LOG_DEBUG ("drop packet because no PLCP preamble/header has been received");
          NotifyRxDrop (packet);
          MaybeCcaBusyDuration ();
          return;
        }
      else if (preamble != WIFI_PREAMBLE_NONE && packet->PeekPacketTag (ampduTag) && m_mpdusNum == 0)
        {
          //received the first MPDU in an MPDU
          m_mpdusNum = ampduTag.GetRemainingNbOfMpdus ();
          m_rxMpduReferenceNumber++;
        }
      else if (preamble == WIFI_PREAMBLE_NONE && packet->PeekPacketTag (ampduTag) && m_mpdusNum > 0)
        {
          //received the other MPDUs that are part of the A-MPDU
          if (ampduTag.GetRemainingNbOfMpdus () < (m_mpdusNum - 1))
            {
              NS_LOG_DEBUG ("Missing MPDU from the A-MPDU " << m_mpdusNum - ampduTag.GetRemainingNbOfMpdus ());
              m_mpdusNum = ampduTag.GetRemainingNbOfMpdus ();
            }
          else
            {
              m_mpdusNum--;
            }
        }
      else if (preamble != WIFI_PREAMBLE_NONE && packet->PeekPacketTag (ampduTag) && m_mpdusNum > 0)
        {
          NS_LOG_DEBUG ("New A-MPDU started while " << m_mpdusNum << " MPDUs from previous are lost");
          m_mpdusNum = ampduTag.GetRemainingNbOfMpdus ();
        }
      else if (preamble != WIFI_PREAMBLE_NONE && m_mpdusNum > 0 )
        {
          NS_LOG_DEBUG ("Didn't receive the last MPDUs from an A-MPDU " << m_mpdusNum);
          m_mpdusNum = 0;
        }

      NS_LOG_DEBUG ("sync to signal (power=" << rxPowerW << "W)");
      m_currentEvent = event;
      m_state->SwitchToRx (rxDuration);//////////////////////////////////
      NotifyRxBegin (packet);
      m_interference.NotifyRxStart ();
             if (preamble != WIFI_PREAMBLE_NONE)
             {
             NS_ASSERT (m_endPlcpRxEvent.IsExpired ());
             Time preambleAndHeaderDuration = CalculatePlcpPreambleAndHeaderDuration (txVector);
             m_endPlcpRxEvent = Simulator::Schedule (preambleAndHeaderDuration, &WifiPhy::StartReceivePacket, this,
                                                packet, txVector, mpdutype, event, rxDuration,rxPowerW);
             }
             NS_ASSERT (m_endRxEvent.IsExpired ());
             m_endRxEvent = Simulator::Schedule (rxDuration, &WifiPhy::EndReceive, this,
                                          packet, preamble, mpdutype, event);
       }
       else
       {
       NS_LOG_DEBUG ("drop packet because signal power too Small (" <<
                      rxPowerW << "<" << GetEdThresholdW () << ")");
       NotifyRxDrop (packet);
       m_plcpSuccess = false;
       MaybeCcaBusyDuration ();
       }
}      
}

int64_t
WifiPhy::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_random->SetStream (stream);
  return 1;
}

std::ostream& operator<< (std::ostream& os, WifiPhy::State state)
{
  switch (state)
    {
    case WifiPhy::IDLE:
      return (os << "IDLE");
    case WifiPhy::CCA_BUSY:
      return (os << "CCA_BUSY");
    case WifiPhy::TX:
      return (os << "TX");
    case WifiPhy::RX:
      return (os << "RX");
    case WifiPhy::SWITCHING:
      return (os << "SWITCHING");
    case WifiPhy::SLEEP:
      return (os << "SLEEP");
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state");
      return (os << "INVALID");
    }
}

} //namespace ns3

namespace {

/**
 * Constructor class
 */
static class Constructor
{
public:
  Constructor ()
  {
    // ns3::WifiPhy::GetOfdmRate300KbpsBW1MHz (); //802.11ah
    ns3::WifiPhy::GetDsssRate1Mbps ();
    ns3::WifiPhy::GetDsssRate2Mbps ();
    ns3::WifiPhy::GetDsssRate5_5Mbps ();
    ns3::WifiPhy::GetDsssRate11Mbps ();
    ns3::WifiPhy::GetErpOfdmRate6Mbps ();
    ns3::WifiPhy::GetErpOfdmRate9Mbps ();
    ns3::WifiPhy::GetErpOfdmRate12Mbps ();
    ns3::WifiPhy::GetErpOfdmRate18Mbps ();
    ns3::WifiPhy::GetErpOfdmRate24Mbps ();
    ns3::WifiPhy::GetErpOfdmRate36Mbps ();
    ns3::WifiPhy::GetErpOfdmRate48Mbps ();
    ns3::WifiPhy::GetErpOfdmRate54Mbps ();
    ns3::WifiPhy::GetOfdmRate6Mbps ();
    ns3::WifiPhy::GetOfdmRate9Mbps ();
    ns3::WifiPhy::GetOfdmRate12Mbps ();
    ns3::WifiPhy::GetOfdmRate18Mbps ();
    ns3::WifiPhy::GetOfdmRate24Mbps ();
    ns3::WifiPhy::GetOfdmRate36Mbps ();
    ns3::WifiPhy::GetOfdmRate48Mbps ();
    ns3::WifiPhy::GetOfdmRate54Mbps ();
    ns3::WifiPhy::GetOfdmRate3MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate4_5MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate6MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate9MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate12MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate18MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate24MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate27MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate1_5MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate2_25MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate3MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate4_5MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate6MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate9MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate12MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate13_5MbpsBW5MHz ();
    //802.11ah
    ns3::WifiPhy::GetOfdmRate300KbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate333_3KbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate600KbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate666_7KbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate900KbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate1MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate1_2MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate1_333_3MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate1_8MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate2MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate2_4MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate2_666_7MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate2_7MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate3MbpsBW1MHzShGi ();
    ns3::WifiPhy::GetOfdmRate3MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate3_333_3MbpsBW1MHz();
    ns3::WifiPhy::GetOfdmRate3_6MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate4MbpsBW1MHzShGi ();
    ns3::WifiPhy::GetOfdmRate4MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate4_444_4MbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate150KbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate166_7KbpsBW1MHz ();
    ns3::WifiPhy::GetOfdmRate650KbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate722_2KbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate1_3MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate1_444_4MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate1_95MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate2_166_7MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate2_6MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate2_8889MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate3_9MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate4_333_3MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate5_2MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate5_777_8MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate5_85MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate6_5MbpsBW2MHzShGi ();
    ns3::WifiPhy::GetOfdmRate6_5MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate7_222_2MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate7_8MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate8_666_7MbpsBW2MHz ();
    ns3::WifiPhy::GetOfdmRate1_35MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate1_5MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate2_7MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate3MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate4_05MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate4_5MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate5_4MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate6MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate8_1MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate9MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate10_8MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate12MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate12_15MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate13_5MbpsBW4MHzShGi ();
    ns3::WifiPhy::GetOfdmRate13_5MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate15MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate16_2MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate18MbpsBW4MHzShGi ();
    ns3::WifiPhy::GetOfdmRate18MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate20MbpsBW4MHz ();
    ns3::WifiPhy::GetOfdmRate2_925MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate3_25MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate5_85MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate6_5MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate8_775MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate9_75MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate11_7MbpsBW8MHz();
    ns3::WifiPhy::GetOfdmRate13MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate17_55MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate19_5MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate23_4MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate26MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate26_325MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate29_25MbpsBW8MHzShGi ();
    ns3::WifiPhy::GetOfdmRate29_25MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate32_5MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate35_1MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate39MbpsBW8MHzShGi ();
    ns3::WifiPhy::GetOfdmRate39MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate43_333_3MbpsBW8MHz ();
    ns3::WifiPhy::GetOfdmRate5_85MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate6_5MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate11_7MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate13MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate17_55MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate19_5MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate23_4MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate26MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate35_1MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate39MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate46_8MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate52MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate52_65MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate58_5MbpsBW16MHzShGi ();
    ns3::WifiPhy::GetOfdmRate58_5MbpsBW16MHz();
    ns3::WifiPhy::GetOfdmRate65MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate70_2MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate78MbpsBW16MHzShGi ();
    ns3::WifiPhy::GetOfdmRate78MbpsBW16MHz ();
    ns3::WifiPhy::GetOfdmRate86_666_7MbpsBW16MHz ();
    
    ns3::WifiPhy::GetHtMcs0 ();
    ns3::WifiPhy::GetHtMcs1 ();
    ns3::WifiPhy::GetHtMcs2 ();
    ns3::WifiPhy::GetHtMcs3 ();
    ns3::WifiPhy::GetHtMcs4 ();
    ns3::WifiPhy::GetHtMcs5 ();
    ns3::WifiPhy::GetHtMcs6 ();
    ns3::WifiPhy::GetHtMcs7 ();
    ns3::WifiPhy::GetHtMcs8 ();
    ns3::WifiPhy::GetHtMcs9 ();
    ns3::WifiPhy::GetHtMcs10 ();
    ns3::WifiPhy::GetHtMcs11 ();
    ns3::WifiPhy::GetHtMcs12 ();
    ns3::WifiPhy::GetHtMcs13 ();
    ns3::WifiPhy::GetHtMcs14 ();
    ns3::WifiPhy::GetHtMcs15 ();
    ns3::WifiPhy::GetHtMcs16 ();
    ns3::WifiPhy::GetHtMcs17 ();
    ns3::WifiPhy::GetHtMcs18 ();
    ns3::WifiPhy::GetHtMcs19 ();
    ns3::WifiPhy::GetHtMcs20 ();
    ns3::WifiPhy::GetHtMcs21 ();
    ns3::WifiPhy::GetHtMcs22 ();
    ns3::WifiPhy::GetHtMcs23 ();
    ns3::WifiPhy::GetHtMcs24 ();
    ns3::WifiPhy::GetHtMcs25 ();
    ns3::WifiPhy::GetHtMcs26 ();
    ns3::WifiPhy::GetHtMcs27 ();
    ns3::WifiPhy::GetHtMcs28 ();
    ns3::WifiPhy::GetHtMcs29 ();
    ns3::WifiPhy::GetHtMcs30 ();
    ns3::WifiPhy::GetHtMcs31 ();
    ns3::WifiPhy::GetVhtMcs0 ();
    ns3::WifiPhy::GetVhtMcs1 ();
    ns3::WifiPhy::GetVhtMcs2 ();
    ns3::WifiPhy::GetVhtMcs3 ();
    ns3::WifiPhy::GetVhtMcs4 ();
    ns3::WifiPhy::GetVhtMcs5 ();
    ns3::WifiPhy::GetVhtMcs6 ();
    ns3::WifiPhy::GetVhtMcs7 ();
    ns3::WifiPhy::GetVhtMcs8 ();
    ns3::WifiPhy::GetVhtMcs9 ();
    //802.11ah
    ns3::WifiPhy::GetS1gMcs0 ();
    ns3::WifiPhy::GetS1gMcs1 ();
    ns3::WifiPhy::GetS1gMcs2 ();
    ns3::WifiPhy::GetS1gMcs3 ();
    ns3::WifiPhy::GetS1gMcs4 ();
    ns3::WifiPhy::GetS1gMcs5 ();
    ns3::WifiPhy::GetS1gMcs6 ();
    ns3::WifiPhy::GetS1gMcs7 ();
    ns3::WifiPhy::GetS1gMcs8 ();
    ns3::WifiPhy::GetS1gMcs9 ();
    ns3::WifiPhy::GetS1gMcs10 ();

    ns3::WifiPhy::GetHeMcs0 ();
    ns3::WifiPhy::GetHeMcs1 ();
    ns3::WifiPhy::GetHeMcs2 ();
    ns3::WifiPhy::GetHeMcs3 ();
    ns3::WifiPhy::GetHeMcs4 ();
    ns3::WifiPhy::GetHeMcs5 ();
    ns3::WifiPhy::GetHeMcs6 ();
    ns3::WifiPhy::GetHeMcs7 ();
    ns3::WifiPhy::GetHeMcs8 ();
    ns3::WifiPhy::GetHeMcs9 ();
    ns3::WifiPhy::GetHeMcs10 ();
    ns3::WifiPhy::GetHeMcs11 ();
  }
} g_constructor; ///< the constructor

}
