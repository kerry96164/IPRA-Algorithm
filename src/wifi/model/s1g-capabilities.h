/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013
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

#ifndef S1G_CAPABILITIES_H
#define S1G_CAPABILITIES_H

#include <stdint.h>
#include "ns3/buffer.h"
#include "ns3/attribute-helper.h"
#include "wifi-information-element.h"

namespace ns3 {

/**
 * \ingroup wifi
 * 
 * The IEEE 802.11ah S1g Capabilities
 */
class S1gCapabilities : public WifiInformationElement
{
public:
  S1gCapabilities ();
  /**
   * Set S1G supported
   * \param s1gsupported the S1G supported indicator
   */
  void SetS1gSupported (uint8_t s1gsupported);

  /**
   * Get the wifi information element ID
   * \returns the wifi information element ID
   */
  WifiInformationElementId ElementId () const;
  /**
   * Get information field size
   * \returns the information field size
   */
  uint8_t GetInformationFieldSize () const;
  /**
   * Serialize the information field.
   *
   * \param start the information field iterator
   */
  void SerializeInformationField (Buffer::Iterator start) const;
  /**
   * Deserialize the information field.
   *
   * \param start the information field iterator
   * \param length the information field
   * \returns the information field
   */
  uint8_t DeserializeInformationField (Buffer::Iterator start, uint8_t length);
  
  /**
   * Set the S1G Capabilties Info field in the S1G Capabilities information element.
   * Total of 80 bits split into a lowest 64-bit section and a higest 16-bit section.
   *
   * \param ctrl the S1G Capabilties Info field in the S1G Capabilities information element
   */
  //Set the lowest 64bytes of the S1gCapabilitiesInfo in the S1G Capabilities information element
  void SetS1gCapabilitiesInfoL64 (uint64_t ctrl);
  //Set the higest 16bytes of the S1gCapabilitiesInfo in the S1G Capabilities information element
  void SetS1gCapabilitiesInfoH16 (uint16_t ctrl);
  
  /**
   * Return the S1G Capabilties Info field in the S1G Capabilities information element.
   * Total of 80 bits split into a lowest 64-bit section and a higest 16-bit section.
   *
   * \return the S1G Capabilties Info field in the S1G Capabilities information element
   */
  //Return the lowest 64bytes of the S1gCapabilitiesInfo in the S1G Capabilities information element
  uint64_t GetS1gCapabilitiesInfoL64 (void) const;
  //Return the highest 16bytes of the S1gCapabilitiesInfo in the S1G Capabilities information element
  uint16_t GetS1gCapabilitiesInfoH16 (void) const;

  /**
   * Set the MCS and NSS field in the S1G Capabilities information element.
   *
   * \param ctrl the MCS and NSS field in the S1G Capabilities information element
   */
  //Set lowest 8 bytes of Supported MCS and NSS set
  void SetSupportedMcsAndNssSetH32 (uint32_t ctrl);
  //Set higest 32 bytes of Supported MCS and NSS set
  void SetSupportedMcsAndNssSetL8 (uint8_t ctrl);
  
  /**
   * Return the MCS and NSS field in the S1G Capabilities information element.
   *
   * \return the MCS and NSS field in the S1G Capabilities information element
   */
  //Return lowest 8 bytes of Supported MCS and NSS set
  uint8_t GetSupportedMcsAndNssSetL8 () const;
  //Return higest 32 bytes of Supported MCS and NSS set
  uint32_t GetSupportedMcsAndNssSetH32 () const;

  //========Capabilities Info fields============
  /**
   * Set the short guard interval 1 Mhz.
   *
   * \param shortguardinterval the short guard interval 1 Mhz
   */
  void SetShortGuardIntervalFor1Mhz (uint8_t shortguardinterval);
  /**
   * Set the short guard interval 2 Mhz.
   *
   * \param shortguardinterval the short guard interval 2 Mhz
   */
  void SetShortGuardIntervalFor2Mhz (uint8_t shortguardinterval);
  /**
   * Set the short guard interval 4 Mhz.
   *
   * \param shortguardinterval the short guard interval 4 Mhz
   */
  void SetShortGuardIntervalFor4Mhz (uint8_t shortguardinterval);
  /**
   * Set the short guard interval 8 Mhz.
   *
   * \param shortguardinterval the short guard interval 8 Mhz
   */
  void SetShortGuardIntervalFor8Mhz (uint8_t shortguardinterval);
  /**
   * Set the short guard interval 16 Mhz.
   *
   * \param shortguardinterval the short guard interval 16 Mhz
   */
  void SetShortGuardIntervalFor16Mhz (uint8_t shortguardinterval);
  /**
   * Set the maximum MPDU length.
   *
   * \param maxmpdulength the maximum MPDU length
   */
  void SetMaxMpduLength (uint8_t maxmpdulength);
  /**
   * Get the maximum MPDU length.
   *
   * \returns the maximum MPDU length
   */
  uint8_t GetMaxMpduLength (void) const;
  /**
   * Set the supported channel width set.
   *
   * \param channelwidthset the supported channel width set
   */
  void SetSupportedChannelWidthSet (uint8_t channelwidthset);
  /**
   * Get the supported channel width set.
   * 
   * \returns the supported channel width set
   */
  uint8_t GetSupportedChannelWidthSet (void) const;
  /**
   * Set the receive LDPC.
   *
   * \param rxldpc the receive LDPC
   */
  void SetRxLdpc (uint8_t rxldpc);
  /**
   * Get the receive LDPC.
   *
   * \returns the receive LDPC
   */
  uint8_t GetRxLdpc () const;
    /**
   * Get the short guard interval 1 MHz.
   *
   * \returns the short guard interval 1 MHz
   */
  uint8_t GetShortGuardIntervalFor1Mhz () const;
  /**
   * Get the short guard interval 2 MHz.
   *
   * \returns the short guard interval 2 MHz
   */
  uint8_t GetShortGuardIntervalFor2Mhz () const;
    /**
   * Get the short guard interval 4 MHz.
   *
   * \returns the short guard interval 4 MHz
   */
  uint8_t GetShortGuardIntervalFor4Mhz () const;
  /**
   * Get the short guard interval 8 MHz.
   *
   * \returns the short guard interval 8 MHz
   */
  uint8_t GetShortGuardIntervalFor8Mhz () const;
    /**
   * Get the short guard interval 16 MHz.
   *
   * \returns the short guard interval 16 MHz
   */
  uint8_t GetShortGuardIntervalFor16Mhz () const;
  /**
   * Set the maximum AMPDU length exponent.
   *
   * \param exponent the maximum AMPDU length exponent
   */
  void SetMaxAmpduLengthExponent (uint8_t exponent);
  /**
   * Get the maximum AMPDU length exponent.
   *
   * \returns the maximum AMPDU length exponent
   */
  uint8_t GetMaxAmpduLengthExponent () const;
  /**
   * Set the STA type.
   * 
   * \param type the STA type
   */
  void SetStaTypeSupport (uint8_t type);
  void SetPageSlicingSupport (uint8_t pageSlicingImplemented);
  void SetNdpPsPollSupport (uint8_t psPollingSupported);

  uint8_t GetStaTypeSupport () const;
  uint8_t GetPageSlicingSupport () const;
  uint8_t GetNdpPsPollSupport () const;

  // TODO: Add the Set/Get functions for other parameters to the class.

  // MCS and NSS field information
  // For each value of NSS ranging from 1 to 4, we need to encode two bits.
  // The value 0 indicates that the maximum S1G-MCS for that spatial stream is 2.
  // The value 1 indicates that the maximum S1G-MCS for that spatial stream is 7.
  // The value 2 indicates that the maximum S1G-MCS for that spatial stream is 9.
  //
  // The maps may be set all at once by passing in a 8-bit field corresponding
  // to the above, or incrementally for each NSS separately, where the
  // MCS value must be in the range 2-9.
  /**
   * \param map The 8-bit encoding of Max MCS for each of 4 spatial streams
   */
  void SetRxS1gMcsMap (uint8_t map);
  /**
   * \param mcs Max MCS value (between 2 and 9)
   * \param nss Spatial stream for which the Max MCS value is being set
   */
  void SetRxS1gMcsMap (uint8_t mcs, uint8_t nss);
  /**
   * \param map The 8-bit encoding of Max MCS for each of 4 spatial streams
   */
  void SetTxS1gMcsMap (uint8_t map);
  /**
   * \param mcs Max MCS value (between 2 and 9)
   * \param nss Spatial stream for which the Max MCS value is being set
   */
  void SetTxS1gMcsMap (uint8_t mcs, uint8_t nss);
  /**
   * Set the receive highest supported LGI data rate.
   *
   * \param supporteddatarate receive highest supported LGI data rate
   */
  void SetRxHighestSupportedLgiDataRate (uint16_t supporteddatarate);
  /**
   * Set the transmit highest supported LGI data rate.
   *
   * \param supporteddatarate transmit highest supported LGI data rate
   */
  void SetTxHighestSupportedLgiDataRate (uint16_t supporteddatarate);
  /**
   * Get the is MCS supported.
   *
   * \param mcs the MCS
   * \param Nss the NSS
   * \returns the is MCS supported
   */
  bool IsSupportedMcs (uint8_t mcs, uint8_t Nss) const;

  /**
   * Get the receive MCS map.
   *
   * \returns the receive MCS map
   */
  uint8_t GetRxS1gMcsMap () const;
  /**
   * Get the transmit MCS map.
   *
   * \returns the transmit MCS map
   */
  uint8_t GetTxS1gMcsMap () const;
  /**
   * Get the receive highest supported LGI data rate.
   *
   * \returns the receive highest supported LGI data rate.
   */
  uint16_t GetRxHighestSupportedLgiDataRate () const;
  /**
   * Get the transmit highest supported LGI data rate.
   *
   * \returns the transmit highest supported LGI data rate.
   */
  uint16_t GetTxHighestSupportedLgiDataRate () const;

  /**
   * Get the is transmit MCS supported.
   *
   * \param mcs the MCS
   * \returns is transmit MCS supported
   */
  bool IsSupportedTxS1gMcs (uint8_t mcs, uint8_t channelwidth) const;
  /**
   * Get the is receive MCS supported.
   *
   * \param mcs the MCS
   * \returns is receive MCS supported
   */
  bool IsSupportedRxS1gMcs (uint8_t mcs, uint8_t channelwidth) const;
    
  /**
   * This information element is a bit special in that it is only
   * included if the STA is an S1G STA. To support this we
   * override the Serialize and GetSerializedSize methods of
   * WifiInformationElement.
   *
   * \param start
   *
   * \return an iterator
   */
  Buffer::Iterator Serialize (Buffer::Iterator start) const;
  /**
   * Return the serialized size of this S1G capability
   * information element.
   *
   * \return the serialized size of this S1G capability information element
   */
  uint16_t GetSerializedSize () const;

/**
 * Define the S1G Capabilities Info field
 * In 802.11ah-2016 9.4.2.201.2 S1G Capabilities Information field
 */
private:
  //Capabilities Information field
  uint8_t m_s1gLongSupport; ///< S1G long support
  uint8_t m_shortGuardIntervalFor1Mhz; ///< short guard interval for 1MHz
  uint8_t m_shortGuardIntervalFor2Mhz; ///< short guard interval for 2MHz
  uint8_t m_shortGuardIntervalFor4Mhz; ///< short guard interval for 4MHz
  uint8_t m_shortGuardIntervalFor8Mhz; ///< short guard interval for 8MHz
  uint8_t m_shortGuardIntervalFor16Mhz; ///< short guard interval for 16MHz
  uint8_t m_supportedChannelWidth; ///< supported channel width
  uint8_t m_rxLdpc; ///< receive LDPC
  uint8_t m_txStbc; ///< transmit STBC
  uint8_t m_rxStbc; ///< receive STBC
  uint8_t m_suBeamformerCapable; ///< single user beamformer capable
  uint8_t m_suBeamformeeCapable; ///< single user beamformee capable
  uint8_t m_beamformeeStsCapable; ///< beamformee STS capable
  uint8_t m_numberOfSoundingDimensions; ///< number of sounding dimensions
  uint8_t m_muBeamformerCapable; ///< multi user beamformer capable
  uint8_t m_muBeamformeeCapable; ///< multi user beamformee capable
  uint8_t m_htcVhtCapable; ///< HTC VHT capable
  uint8_t m_travwlingPilotSupport; ///< traveling pilot support
  uint8_t m_rdResponder; ///< RD responder
  uint8_t m_htDelayedBlockAck; ///< HT delayed block ack
  uint8_t m_maxMpduLength; ///< max MPDU length
  uint8_t m_maxAmpduLengthExponent; ///< max A-MPDU length exponent
  uint8_t m_minMpduStartSpacing; ///< min MPDU start spacing
  uint8_t m_uplinkSyncCapable; ///< uplink sync capable
  uint8_t m_dynamicAid; ///< dynamic AID
  uint8_t m_batSupport; ///< BAT support
  uint8_t m_timAdeSupport; ///< TIM ADE support
  uint8_t m_nonTimSupport; ///< non-TIM support
  uint8_t m_groupAidSupport; ///< group AID support
  uint8_t m_staTypeSupport; ///< STA type support
  uint8_t m_centralizedAuthenticationControl; ///< centralized authentication control
  uint8_t m_distributedAuthenticationControl; ///< distributed authentication control
  uint8_t m_aMsduSupported; ///< A-MSDU supported
  uint8_t m_aMpduSupported; ///< A-MPDU supported
  uint8_t m_asymmetricBlockAckSupported; ///< asymmetric block ack supported
  uint8_t m_flowControlSupported; ///< flow control supported
  uint8_t m_sectorizedBeamCapable; ///< sectorized beam capable
  uint8_t m_obssMitigationSupport; ///< OBSS mitigation support
  uint8_t m_fragmentBaSupport; ///< fragment BA support
  uint8_t m_ndpPsPollSupported; ///< NDP PS-Poll supported
  uint8_t m_rawOperationSupport; ///< raw operation support
  uint8_t m_pageSlicingSupport; ///< page slicing support
  uint8_t m_txopSharingImplicitAckSupport; ///< TXOP sharing implicit ACK support
  uint8_t m_vhtLinkAdaptationCapable; ///< VHT link adaptation capable
  uint8_t m_tackSupportAsPsPollResponse; ///< TACK support as PS-Poll response  
  uint8_t m_duplicate1MHzSupport; ///< duplicate 1MHz support
  uint8_t m_mcsNegotiationSupport; ///< MCS negotiation support
  uint8_t m_1MHzControlResponsePreambleSupport; ///< 1MHz control response preamble support
  uint8_t m_ndpBeamformingReportPollSupported; ///< NDP beamforming report poll supported
  uint8_t m_unsolicitedDynamicAid; ///< unsolicited dynamic AID
  uint8_t m_sectorTrainingOperationSupported; ///< sector training operation supported
  uint8_t m_temporaryPsModeSwitch; ///< temporary PS mode switch
  uint8_t m_twtGroupingSupport; ///< TWT grouping support
  uint8_t m_dbtCapable; ///< DBT capable
  uint8_t m_color; ///< color
  uint8_t m_twtRequesterSupport; ///< TWT requester support
  uint8_t m_twtResponderSupport; ///< TWT responder support
  uint8_t m_pv1FrameSupport; ///< PV1 frame support
  uint8_t m_linkAdaptationPerNormalControlResponseCapable; ///< link adaptation per normal control response capable
  
  //Table 9-262o—Supported S1G-MCS and NSS Set subfields
  std::vector<uint8_t> m_rxS1gMcsMap; ///< receive S1G-MCS map
  uint16_t m_rxHighestSupportedLongGuardIntervalDataRate;  ///< receive highest supported long guard interval data rate
  std::vector<uint8_t> m_txS1gMcsMap; ///< transmit S1G-MCS map
  uint16_t m_txHighestSupportedLongGuardIntervalDataRate;  ///< transmit highest supported long guard interval data rate
  uint8_t m_rxSingleSpatialStreamAndS1gMcsMapFor1MHz; ///< receive single spatial stream and S1G-MCS map for 1MHz
  uint8_t m_txSingleSpatialStreamAndS1gMcsMapFor1MHz; ///< transmit single spatial stream and S1G-MCS map for 1MHz

  //this is used to decide if this element should be added to the frame or not
  uint8_t m_s1gSupported;
};

std::ostream &operator << (std::ostream &os, const S1gCapabilities &s1gcapabilities);
std::istream &operator >> (std::istream &is, S1gCapabilities &s1gcapabilities);

ATTRIBUTE_HELPER_HEADER (S1gCapabilities);

} //namespace ns3

#endif /* S1G_CAPABILITY_H */
