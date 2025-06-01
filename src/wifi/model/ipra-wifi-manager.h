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

#ifndef IPRA_WIFI_MANAGER_H
#define IPRA_WIFI_MANAGER_H

#include "ns3/traced-value.h"
#include "wifi-remote-station-manager.h"

namespace ns3 {

/**
 * \brief IPRA rate control algorithm
 * \ingroup wifi
 *
 * This class implements the Interference and Power-Aware Rate Adaptation (IPRA) algorithm.
 */
class IpraWifiManager : public WifiRemoteStationManager
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  IpraWifiManager ();
  virtual ~IpraWifiManager ();

  void SetupPhy (const Ptr<WifiPhy> phy);


private:
  //overriden from base class
  void DoInitialize (void);
  WifiRemoteStation* DoCreateStation (void) const;
  void DoReportRxOk (WifiRemoteStation *station,
                     double rxSnr, WifiMode txMode);
  void DoReportRtsFailed (WifiRemoteStation *station);
  void DoReportDataFailed (WifiRemoteStation *station);
  void DoReportRtsOk (WifiRemoteStation *station,
                      double ctsSnr, WifiMode ctsMode, double rtsSnr);
  void DoReportDataOk (WifiRemoteStation *station,
                       double ackSnr, WifiMode ackMode, double dataSnr);
  void DoReportAmpduTxStatus (WifiRemoteStation *station,
                              uint8_t nSuccessfulMpdus, uint8_t nFailedMpdus,
                              double rxSnr, double dataSnr);
  void DoReportFinalRtsFailed (WifiRemoteStation *station);
  void DoReportFinalDataFailed (WifiRemoteStation *station);
  void DoSetLastSnrObserved (WifiRemoteStation *st, double dataSnr);
  WifiTxVector DoGetDataTxVector (WifiRemoteStation *station);
  WifiTxVector DoGetRtsTxVector (WifiRemoteStation *station);
  bool IsLowLatency (void) const;

  /**
   * Return the minimum SNR needed to successfully transmit
   * data with this WifiTxVector at the specified BER.
   *
   * \param txVector WifiTxVector (containing valid mode, width, and nss)
   *
   * \return the minimum SNR for the given WifiTxVector
   */
  double GetSnrThreshold (WifiTxVector txVector) const;
  /**
   * Adds a pair of WifiTxVector and the minimum SNR for that given vector
   * to the list.
   *
   * \param txVector the WifiTxVector storing mode, channel width, and nss
   * \param snr the minimum SNR for the given txVector
   */
  void AddSnrThreshold (WifiTxVector txVector, double snr);

  /**
   * Convenience function for selecting a channel width for legacy mode
   * \param mode non-(V)HT/S1G WifiMode
   * \return the channel width (MHz) for the selected mode
   */
  uint8_t GetChannelWidthForMode (WifiMode mode) const;

  /**
   * A vector of <snr, WifiTxVector> pair holding the minimum SNR for the
   * WifiTxVector
   */
  typedef std::vector<std::pair<double, WifiTxVector> > Thresholds;

  double m_ber;             //!< The maximum Bit Error Rate acceptable at any transmission mode
  Thresholds m_thresholds;  //!< List of WifiTxVector and the minimum SNR pair

  TracedValue<uint64_t> m_currentRate; //!< Trace rate changes
};

} //namespace ns3

#endif /* IPRA_WIFI_MANAGER_H */
