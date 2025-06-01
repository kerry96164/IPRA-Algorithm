/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2025 YuKai Lu
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
 * Author: YuKai Lu <yukai.ee12@nycu.edu.tw>
 */

#include "ipra-wifi-manager.h"
#include "wifi-phy.h"
#include "ns3/log.h"

namespace ns3 {

/**
 * \brief hold per-remote-station state for Ideal Wifi manager.
 *
 * This struct extends from WifiRemoteStation struct to hold additional
 * information required by the IPRA Wifi manager
 */
struct IpraWifiRemoteStation : public WifiRemoteStation
{
  double m_lastSnrObserved;  //!< SNR of most recently reported packet sent to the remote station
  double m_lastSnrCached;    //!< SNR most recently used to select a rate
  double m_nss;          //!< SNR most recently used to select a rate
  WifiMode m_lastMode;       //!< Mode most recently used to the remote station
};

// To avoid using the cache before a valid value has been cached
static const double CACHE_INITIAL_VALUE = -100;

NS_OBJECT_ENSURE_REGISTERED (IpraWifiManager);

NS_LOG_COMPONENT_DEFINE ("IpraWifiManager");

TypeId
IpraWifiManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IpraWifiManager")
    .SetParent<WifiRemoteStationManager> ()
    .SetGroupName ("Wifi")
    .AddConstructor<IpraWifiManager> ()
    .AddAttribute ("BerThreshold",
                   "The maximum Bit Error Rate acceptable at any transmission mode",
                   DoubleValue (1e-5),
                   MakeDoubleAccessor (&IpraWifiManager::m_ber),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("Rate",
                     "Traced value for rate changes (b/s)",
                     MakeTraceSourceAccessor (&IpraWifiManager::m_currentRate),
                     "ns3::TracedValueCallback::Uint64")
  ;
  return tid;
}

IpraWifiManager::IpraWifiManager ()
  : m_currentRate (0)
{
}

IpraWifiManager::~IpraWifiManager ()
{
}

void
IpraWifiManager::SetupPhy (const Ptr<WifiPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  WifiRemoteStationManager::SetupPhy (phy);
}

uint8_t
IpraWifiManager::GetChannelWidthForMode (WifiMode mode) const
{
  NS_ASSERT (mode.GetModulationClass () != WIFI_MOD_CLASS_HT
             && mode.GetModulationClass () != WIFI_MOD_CLASS_VHT
             && mode.GetModulationClass () != WIFI_MOD_CLASS_S1G
             && mode.GetModulationClass () != WIFI_MOD_CLASS_HE);
  if (mode.GetModulationClass () == WIFI_MOD_CLASS_DSSS
      || mode.GetModulationClass () == WIFI_MOD_CLASS_HR_DSSS)
    {
      return 22;
    }
  else
    {
      return 20;
    }
}

void
IpraWifiManager::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  WifiMode mode;
  WifiTxVector txVector;
  uint8_t nss = 1;
  uint32_t nModes = GetPhy ()->GetNModes ();
  // Add all S1G MCSes
  if (HasS1gSupported () == true)
    {
      nModes = GetPhy ()->GetNMcs ();
      for (uint32_t i = 0; i < nModes; i++)
        {
          for (uint16_t j = 1; j <= GetPhy ()->GetChannelWidth (); j*=2)
            {
              txVector.SetChannelWidth (j);
              mode = GetPhy ()->GetMcs (i);
              uint16_t guardInterval = GetPhy ()->GetShortGuardInterval () ? 4000 : 8000;
              for (uint8_t i = 1; i <= GetPhy ()->GetMaxSupportedTxSpatialStreams (); i++)
                {
                  NS_LOG_DEBUG ("Initialize, adding mode = " << mode.GetUniqueName () <<
                                " channel width " << (uint16_t) j <<
                                " nss " << (uint16_t) i <<
                                " GI " << guardInterval);
                  NS_LOG_DEBUG ("In SetupPhy, adding mode = " << mode.GetUniqueName ());
                  txVector.SetNss (i);
                  txVector.SetMode (mode);
                  AddSnrThreshold (txVector, GetPhy ()->CalculateSnr (txVector, m_ber));
                }
            }
        }
    }
  else
    {
      // Add all legacy modes
      for (uint32_t i = 0; i < nModes; i++)
      {
        mode = GetPhy ()->GetMode (i);
        txVector.SetChannelWidth (GetChannelWidthForMode (mode));
        txVector.SetNss (nss);
        txVector.SetMode (mode);
        NS_LOG_DEBUG ("Initialize, adding mode = " << mode.GetUniqueName ());
        AddSnrThreshold (txVector, GetPhy ()->CalculateSnr (txVector, m_ber));
      }
      // Add all Ht and Vht MCSes
      if (HasVhtSupported () == true || HasHtSupported () == true || HasHeSupported () == true)
        {
          nModes = GetPhy ()->GetNMcs ();
          for (uint32_t i = 0; i < nModes; i++)
            {
              for (uint16_t j = 20; j <= GetPhy ()->GetChannelWidth (); j*=2)
                {
                  txVector.SetChannelWidth (j);
                  mode = GetPhy ()->GetMcs (i);
                  if (mode.GetModulationClass () == WIFI_MOD_CLASS_HT)
                    {
                      uint16_t guardInterval = GetPhy ()->GetShortGuardInterval () ? 400 : 800;
                      txVector.SetGuardInterval (guardInterval);
                      //derive NSS from the MCS index
                      nss = (mode.GetMcsValue () / 8) + 1;
                      NS_LOG_DEBUG ("Initialize, adding mode = " << mode.GetUniqueName () <<
                                    " channel width " << (uint16_t) j <<
                                    " nss " << (uint16_t) nss <<
                                    " GI " << guardInterval);
                      NS_LOG_DEBUG ("In SetupPhy, adding mode = " << mode.GetUniqueName ());
                      txVector.SetNss (nss);
                      txVector.SetMode (mode);
                      AddSnrThreshold (txVector, GetPhy ()->CalculateSnr (txVector, m_ber));
                    }
                  else //VHT or HE
                    {
                      uint16_t guardInterval;
                      if (mode.GetModulationClass () == WIFI_MOD_CLASS_VHT)
                        {
                          guardInterval = GetPhy ()->GetShortGuardInterval () ? 400 : 800;
                        }
                      else
                        {
                          guardInterval = GetPhy ()->GetGuardInterval ().GetNanoSeconds ();
                        }
                      for (uint8_t i = 1; i <= GetPhy ()->GetMaxSupportedTxSpatialStreams (); i++)
                        {
                          NS_LOG_DEBUG ("Initialize, adding mode = " << mode.GetUniqueName () <<
                                        " channel width " << (uint16_t) j <<
                                        " nss " << (uint16_t) i <<
                                        " GI " << guardInterval);
                          NS_LOG_DEBUG ("In SetupPhy, adding mode = " << mode.GetUniqueName ());
                          txVector.SetNss (i);
                          txVector.SetMode (mode);
                          AddSnrThreshold (txVector, GetPhy ()->CalculateSnr (txVector, m_ber));
                        }
                    }
                }
            }
        }
    }
}

double
IpraWifiManager::GetSnrThreshold (WifiTxVector txVector) const
{
  NS_LOG_FUNCTION (this << txVector.GetMode ().GetUniqueName ());
  for (Thresholds::const_iterator i = m_thresholds.begin (); i != m_thresholds.end (); i++)
    {
      NS_LOG_DEBUG ("Checking " << i->second.GetMode ().GetUniqueName () <<
                    " nss " << (uint16_t) i->second.GetNss () <<
                    " GI " << i->second.GetGuardInterval () <<
                    " width " << (uint16_t) i->second.GetChannelWidth ());
      NS_LOG_DEBUG ("against TxVector " << txVector.GetMode ().GetUniqueName () <<
                    " nss " << (uint16_t) txVector.GetNss () <<
                    " GI " << txVector.GetGuardInterval () <<
                    " width " << (uint16_t) txVector.GetChannelWidth ());
      if (txVector.GetMode () == i->second.GetMode ()
          && txVector.GetNss () == i->second.GetNss ()
          && txVector.GetChannelWidth () == i->second.GetChannelWidth ())
        {
          return i->first;
        }
    }
  NS_ASSERT (false);
  return 0.0;
}

void
IpraWifiManager::AddSnrThreshold (WifiTxVector txVector, double snr)
{
  NS_LOG_FUNCTION (this << txVector.GetMode ().GetUniqueName () << snr);
  m_thresholds.push_back (std::make_pair (snr, txVector));
}

WifiRemoteStation *
IpraWifiManager::DoCreateStation (void) const
{
  NS_LOG_FUNCTION (this);
  IpraWifiRemoteStation *station = new IpraWifiRemoteStation ();
  station->m_lastSnrObserved = 0.0;
  station->m_lastSnrCached = CACHE_INITIAL_VALUE;
  station->m_lastMode = GetDefaultMode ();
  station->m_nss = 1;
  return station;
}


void
IpraWifiManager::DoReportRxOk (WifiRemoteStation *station,
                                double rxSnr, WifiMode txMode)
{
}

void
IpraWifiManager::DoReportRtsFailed (WifiRemoteStation *station)
{
}

void
IpraWifiManager::DoReportDataFailed (WifiRemoteStation *station)
{
}

void
IpraWifiManager::DoReportRtsOk (WifiRemoteStation *st,
                                 double ctsSnr, WifiMode ctsMode, double rtsSnr)
{
  NS_LOG_FUNCTION (this << st << ctsSnr << ctsMode.GetUniqueName () << rtsSnr);
  NS_ASSERT (false); // IPRA does not support RTS/CTS

  IpraWifiRemoteStation *station = (IpraWifiRemoteStation *)st;
  station->m_lastSnrObserved = rtsSnr;
}

void
IpraWifiManager::DoReportDataOk (WifiRemoteStation *st,
                                  double ackSnr, WifiMode ackMode, double dataSnr)
{
  NS_LOG_FUNCTION (this << st << ackSnr << ackMode.GetUniqueName () << dataSnr);
  IpraWifiRemoteStation *station = (IpraWifiRemoteStation *)st;
  if (dataSnr == 0)
    {
      NS_LOG_WARN ("DataSnr reported to be zero; not saving this report.");
      return;
    }
  station->m_lastSnrObserved = dataSnr;
}

void
IpraWifiManager::DoReportAmpduTxStatus (WifiRemoteStation *st, uint8_t nSuccessfulMpdus, uint8_t nFailedMpdus, double rxSnr, double dataSnr)
{
  NS_LOG_FUNCTION (this << st << (uint16_t)nSuccessfulMpdus << (uint16_t)nFailedMpdus << rxSnr << dataSnr);
  IpraWifiRemoteStation *station = (IpraWifiRemoteStation *)st;
  if (dataSnr == 0)
    {
      NS_LOG_WARN ("DataSnr reported to be zero; not saving this report.");
      return;
    }
  station->m_lastSnrObserved = dataSnr;
}


void
IpraWifiManager::DoReportFinalRtsFailed (WifiRemoteStation *station)
{
}

void
IpraWifiManager::DoReportFinalDataFailed (WifiRemoteStation *station)
{
}

void
IpraWifiManager::DoSetLastSnrObserved (WifiRemoteStation *st, double dataSnr)
{
  NS_LOG_FUNCTION (this << st << dataSnr);
  IpraWifiRemoteStation *station = (IpraWifiRemoteStation *)st;
  station->m_lastSnrObserved = dataSnr;
}

WifiTxVector
IpraWifiManager::DoGetDataTxVector (WifiRemoteStation *st)
{
  NS_LOG_FUNCTION (this << st);
  IpraWifiRemoteStation *station = (IpraWifiRemoteStation *)st;
  //We search within the Supported rate set the mode with the
  //highest data rate for which the snr threshold is smaller than m_lastSnr
  //to ensure correct packet delivery.
  WifiMode maxMode = GetDefaultMode ();
  WifiTxVector txVector;
  WifiMode mode;
  uint64_t bestRate = 0;
  uint8_t selectedNss = 1;
  uint16_t guardInterval;
  uint8_t channelWidth = std::min (GetChannelWidth (station), GetPhy ()->GetChannelWidth ());
  txVector.SetChannelWidth (channelWidth);
  if (station->m_lastSnrCached != CACHE_INITIAL_VALUE && station->m_lastSnrObserved == station->m_lastSnrCached)
    {
      // SNR has not changed, so skip the search and use the last
      // mode selected
      maxMode = station->m_lastMode;
      selectedNss = station->m_nss;
      NS_LOG_DEBUG ("Using cached mode = " << maxMode.GetUniqueName () <<
                    " last snr observed " << station->m_lastSnrObserved <<
                    " cached " << station->m_lastSnrCached <<
                    " nss " << (uint16_t) selectedNss);
    }
  else
    {
      if ((HasS1gSupported () == true || HasVhtSupported () == true || HasHtSupported () == true || HasHeSupported () == true)
          && (GetS1gSupported (st) == true || GetHtSupported (st) == true || GetVhtSupported (st) == true || GetHeSupported (st) == true))
        {
          for (uint32_t i = 0; i < GetNMcsSupported (station); i++)
            {
              mode = GetMcsSupported (station, i);
              txVector.SetMode (mode);
              if (mode.GetModulationClass () == WIFI_MOD_CLASS_HT)
                {
                  guardInterval = std::max (GetShortGuardInterval (station) ? 400 : 800, GetPhy ()->GetShortGuardInterval () ? 400 : 800);
                  txVector.SetGuardInterval (guardInterval);
                  // If the node and peer are both VHT capable, only search VHT modes
                  if (HasVhtSupported () && GetVhtSupported (station))
                    {
                      continue;
                    }
                  // If the node and peer are both HE capable, only search HE modes
                  if (HasHeSupported () && GetHeSupported (station))
                    {
                      continue;
                    }
                  // Derive NSS from the MCS index. There is a different mode for each possible NSS value.
                  uint8_t nss = (mode.GetMcsValue () / 8) + 1;
                  txVector.SetNss (nss);
                  if (WifiPhy::IsValidTxVector (txVector) == false ||
                      nss > std::min (GetMaxNumberOfTransmitStreams (), GetNumberOfSupportedStreams (st)))
                    {
                      NS_LOG_DEBUG ("Skipping mode " << mode.GetUniqueName () <<
                                    " nss " << (uint16_t) nss << " width " <<
                                    (uint16_t) txVector.GetChannelWidth ());
                      continue;
                    }
                  double threshold = GetSnrThreshold (txVector);
                  uint64_t dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), nss);
                  NS_LOG_DEBUG ("Testing mode " << mode.GetUniqueName () <<
                                " data rate " << dataRate << 
                                " threshold " << threshold  << " last snr observed " <<
                                station->m_lastSnrObserved << " cached " <<
                                station->m_lastSnrCached);
                  if (dataRate > bestRate && threshold < station->m_lastSnrObserved)
                    {
                      NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () <<
                                    " data rate " << dataRate <<
                                    " threshold " << threshold  <<
                                    " last snr observed " <<
                                    station->m_lastSnrObserved);
                      bestRate = dataRate;
                      maxMode = mode;
                      selectedNss = nss;
                    }
                 }
              else if (mode.GetModulationClass () == WIFI_MOD_CLASS_VHT)
                {
                  guardInterval = std::max (GetShortGuardInterval (station) ? 400 : 800, GetPhy ()->GetShortGuardInterval () ? 400 : 800);
                  txVector.SetGuardInterval (guardInterval);
                  // If the node and peer are both HE capable, only search HE modes
                  if (HasHeSupported () && GetHeSupported (station))
                    {
                      continue;
                    }
                  // If the node and peer are not both VHT capable, only search HT modes
                  if (!HasVhtSupported () || !GetVhtSupported (station))
                    {
                      continue;
                    }
                  for (uint8_t nss = 1; nss <= std::min (GetMaxNumberOfTransmitStreams (), GetNumberOfSupportedStreams (station)); nss++)
                    {
                      txVector.SetNss (nss);
                      if (WifiPhy::IsValidTxVector (txVector) == false)
                        {
                          NS_LOG_DEBUG ("Skipping mode " << mode.GetUniqueName () <<
                                        " nss " << (uint16_t) nss << " width " <<
                                        (uint16_t) txVector.GetChannelWidth ());
                          continue;
                        }
                      double threshold = GetSnrThreshold (txVector);
                      uint64_t dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), nss);
                      NS_LOG_DEBUG ("Testing mode = " << mode.GetUniqueName () <<
                                    " data rate " << dataRate <<
                                    " threshold " << threshold << " last snr observed " <<
                                    station->m_lastSnrObserved << " cached " <<
                                    station->m_lastSnrCached);
                      if (dataRate > bestRate && threshold < station->m_lastSnrObserved)
                        {
                          NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () <<
                                        " data rate " << dataRate <<
                                        " threshold " << threshold  <<
                                        " last snr observed " <<
                                        station->m_lastSnrObserved);
                          bestRate = dataRate;
                          maxMode = mode;
                          selectedNss = nss;
                        }
                    }
                }
              else if (mode.GetModulationClass () == WIFI_MOD_CLASS_S1G)
                {
                  guardInterval = std::max (GetShortGuardInterval (station) ? 4000 : 8000, GetPhy ()->GetShortGuardInterval () ? 4000 : 8000);
                  txVector.SetGuardInterval (guardInterval);
                  for (uint8_t nss = 1; nss <= std::min (GetMaxNumberOfTransmitStreams (), GetNumberOfSupportedStreams (station)); nss++)
                    {
                      txVector.SetNss (nss);
                      if (WifiPhy::IsValidTxVector (txVector) == false)
                        {
                          NS_LOG_DEBUG ("Skipping mode " << mode.GetUniqueName () <<
                                        " nss " << (uint16_t) nss << " width " <<
                                        (uint16_t) txVector.GetChannelWidth ());
                          continue;
                        }
                      double threshold = GetSnrThreshold (txVector);
                      uint64_t dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), nss);
                      NS_LOG_DEBUG ("Testing mode = " << mode.GetUniqueName () <<
                                    " data rate " << dataRate <<
                                    " threshold " << threshold << " last snr observed " <<
                                    station->m_lastSnrObserved << " cached " <<
                                    station->m_lastSnrCached);
                      if (dataRate > bestRate && threshold < station->m_lastSnrObserved)
                        {
                          NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () <<
                                        " data rate " << dataRate <<
                                        " threshold " << threshold  <<
                                        " last snr observed " <<
                                        station->m_lastSnrObserved);
                          bestRate = dataRate;
                          maxMode = mode;
                          selectedNss = nss;
                        }
                    }
                }
              else //HE
                {
                  guardInterval = std::max (GetGuardInterval (station), static_cast<uint16_t> (GetPhy ()->GetGuardInterval ().GetNanoSeconds ()));
                  txVector.SetGuardInterval (guardInterval);
                  // If the node and peer are not both HE capable, only search (V)HT modes
                  if (!HasHeSupported () || !GetHeSupported (station))
                    {
                      continue;
                    }
                  for (uint8_t nss = 1; nss <= GetNumberOfSupportedStreams (station); nss++)
                    {
                      txVector.SetNss (nss);
                      if (WifiPhy::IsValidTxVector (txVector) == false)
                        {
                          NS_LOG_DEBUG ("Skipping mode " << mode.GetUniqueName () <<
                                        " nss " << (uint16_t) nss << " width " <<
                                        (uint16_t) txVector.GetChannelWidth ());
                          continue;
                        }
                      double threshold = GetSnrThreshold (txVector);
                      uint64_t dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), nss);
                      NS_LOG_DEBUG ("Testing mode = " << mode.GetUniqueName () <<
                                    " data rate " << dataRate <<
                                    " threshold " << threshold  << " last snr observed " <<
                                    station->m_lastSnrObserved << " cached " <<
                                    station->m_lastSnrCached);
                      if (dataRate > bestRate && threshold < station->m_lastSnrObserved)
                        {
                          NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () <<
                                        " data rate " << dataRate <<
                                        " threshold " << threshold  <<
                                        " last snr observed " <<
                                        station->m_lastSnrObserved);
                          bestRate = dataRate;
                          maxMode = mode;
                          selectedNss = nss;
                        }
                    }
                }
            }
        }
      else
        {
          // Non-HT selection
          selectedNss = 1;
          for (uint32_t i = 0; i < GetNSupported (station); i++)
            {
              mode = GetSupported (station, i);
              txVector.SetMode (mode);
              txVector.SetNss (selectedNss);
              txVector.SetChannelWidth (GetChannelWidthForMode (mode));
              double threshold = GetSnrThreshold (txVector);
              uint64_t dataRate = mode.GetDataRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), txVector.GetNss ());
              NS_LOG_DEBUG ("mode = " << mode.GetUniqueName () <<
                            " threshold " << threshold  <<
                            " last snr observed " <<
                            station->m_lastSnrObserved);
              if (dataRate > bestRate && threshold < station->m_lastSnrObserved)
                {
                  NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () <<
                                " data rate " << dataRate <<
                                " threshold " << threshold  <<
                                " last snr observed " <<
                                station->m_lastSnrObserved);
                  bestRate = dataRate;
                  maxMode = mode;
                }
            }
        }
      NS_LOG_DEBUG ("Updating cached values for station to " <<  maxMode.GetUniqueName () << " snr " << station->m_lastSnrObserved);
      station->m_lastSnrCached = station->m_lastSnrObserved;
      station->m_lastMode = maxMode;
      station->m_nss = selectedNss;
    }
  NS_LOG_DEBUG ("Found maxMode: " << maxMode << " channelWidth: " << (uint16_t) channelWidth);
  if (maxMode.GetModulationClass () == WIFI_MOD_CLASS_HE)
    {
      guardInterval = std::max (GetGuardInterval (station), static_cast<uint16_t> (GetPhy ()->GetGuardInterval ().GetNanoSeconds ()));
    }
  else if (maxMode.GetModulationClass () == WIFI_MOD_CLASS_S1G)
    {
      guardInterval = std::max (GetShortGuardInterval (station) ? 4000 : 8000, GetPhy ()->GetShortGuardInterval () ? 4000 : 8000);
    }
  else
    {
      guardInterval = std::max (GetShortGuardInterval (station) ? 400 : 800, GetPhy ()->GetShortGuardInterval () ? 400 : 800);
    }
  if (m_currentRate != maxMode.GetDataRate (channelWidth, guardInterval, selectedNss))
    {
      NS_LOG_DEBUG ("New datarate: " << maxMode.GetDataRate (channelWidth, guardInterval, selectedNss));
      m_currentRate = maxMode.GetDataRate (channelWidth, guardInterval, selectedNss);
    }
  return WifiTxVector (maxMode, GetDefaultTxPowerLevel (), GetLongRetryCount (station), GetPreambleForTransmission (maxMode, GetAddress (station)), guardInterval, GetNumberOfAntennas (), selectedNss, 0, channelWidth, GetAggregation (station), false);
}

WifiTxVector
IpraWifiManager::DoGetRtsTxVector (WifiRemoteStation *st)
{
  NS_LOG_FUNCTION (this << st);
  
  NS_ASSERT (false); // IPRA does not support RTS/CTS

  IpraWifiRemoteStation *station = (IpraWifiRemoteStation *)st;
  //We search within the Basic rate set the mode with the highest
  //snr threshold possible which is smaller than m_lastSnr to
  //ensure correct packet delivery.
  double maxThreshold = 0.0;
  WifiTxVector txVector;
  WifiMode mode;
  uint8_t nss = 1;
  WifiMode maxMode = GetDefaultMode ();
  //avoid to use legacy rate adaptation algorithms for IEEE 802.11n/ac/ax
  //RTS is sent in a legacy frame; RTS with HT/VHT/HE is not yet supported
  for (uint32_t i = 0; i < GetNBasicModes (); i++)
    {
      mode = GetBasicMode (i);
      txVector.SetMode (mode);
      txVector.SetNss (nss);
      txVector.SetChannelWidth (GetChannelWidthForMode (mode));
      double threshold = GetSnrThreshold (txVector);
      if (threshold > maxThreshold && threshold < station->m_lastSnrObserved)
        {
          maxThreshold = threshold;
          maxMode = mode;
        }
    }
  return WifiTxVector (maxMode, GetDefaultTxPowerLevel (), GetShortRetryCount (station), GetPreambleForTransmission (maxMode, GetAddress (station)), 800, GetNumberOfAntennas (), nss, 0, GetChannelWidthForMode (maxMode), GetAggregation (station), false);
}

bool
IpraWifiManager::IsLowLatency (void) const
{
  return true;
}

} //namespace ns3
