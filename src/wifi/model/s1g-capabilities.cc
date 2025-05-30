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
 * Author: Ghada Badawy <gbadawy@rim.com>
 */

#include "s1g-capabilities.h"
#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("S1gCapabilities");

S1gCapabilities::S1gCapabilities ()
  : m_s1gLongSupport (0),
    m_shortGuardIntervalFor1Mhz  (0),
    m_shortGuardIntervalFor2Mhz  (0),
    m_shortGuardIntervalFor4Mhz  (0),
    m_shortGuardIntervalFor8Mhz  (0),
    m_shortGuardIntervalFor16Mhz  (0),
    m_supportedChannelWidth (0),
    m_rxLdpc (0),
    m_txStbc (0),
    m_rxStbc (0),
    m_suBeamformerCapable (0),
    m_suBeamformeeCapable (0),
    m_beamformeeStsCapable (0),
    m_numberOfSoundingDimensions (0),
    m_muBeamformerCapable (0),
    m_muBeamformeeCapable (0),
    m_htcVhtCapable (0),
    m_travwlingPilotSupport (0),
    m_rdResponder (0),
    m_htDelayedBlockAck (0),
    m_maxMpduLength (0),
    m_maxAmpduLengthExponent (0),
    m_minMpduStartSpacing (0),
    m_uplinkSyncCapable (0),
    m_dynamicAid (0),
    m_batSupport (0),
    m_timAdeSupport (0),
    m_nonTimSupport (0),
    m_groupAidSupport (0),
    m_staTypeSupport (0),
    m_centralizedAuthenticationControl (0),
    m_distributedAuthenticationControl (0),
    m_aMsduSupported (0),
    m_aMpduSupported (0),
    m_asymmetricBlockAckSupported (0),
    m_flowControlSupported (0),
    m_sectorizedBeamCapable (0),
    m_obssMitigationSupport (0),
    m_fragmentBaSupport (0),
    m_ndpPsPollSupported (0),
    m_rawOperationSupport (0),
    m_pageSlicingSupport (0),
    m_txopSharingImplicitAckSupport (0),
    m_vhtLinkAdaptationCapable (0),
    m_tackSupportAsPsPollResponse (0),
    m_duplicate1MHzSupport (0),
    m_mcsNegotiationSupport (0),
    m_1MHzControlResponsePreambleSupport (0),
    m_ndpBeamformingReportPollSupported (0),
    m_unsolicitedDynamicAid (0),
    m_sectorTrainingOperationSupported (0),
    m_temporaryPsModeSwitch (0),
    m_twtGroupingSupport (0),
    m_dbtCapable (0),
    m_color (0),
    m_twtRequesterSupport (0),
    m_twtResponderSupport (0),
    m_pv1FrameSupport (0),
    m_linkAdaptationPerNormalControlResponseCapable (0),
    m_rxHighestSupportedLongGuardIntervalDataRate (0),
    m_txHighestSupportedLongGuardIntervalDataRate (0),
    m_rxSingleSpatialStreamAndS1gMcsMapFor1MHz (0),
    m_txSingleSpatialStreamAndS1gMcsMapFor1MHz (0),
    m_s1gSupported (0)
{
  m_rxS1gMcsMap.resize (4,0);
  m_txS1gMcsMap.resize (4,0);
  for (uint8_t i = 0; i < 4; i++) //set to 3 by default, i.e. #spatial streams not supported. 0 means supported up to MCS 2, not what we want to imply at this stage.
    {
      m_rxS1gMcsMap[i] = 3;
      m_txS1gMcsMap[i] = 3;
    }
}

WifiInformationElementId
S1gCapabilities::ElementId () const
{
  return IE_S1G_CAPABILITIES;
}

void
S1gCapabilities::SetS1gSupported (uint8_t s1gsupported)
{
  m_s1gSupported = s1gsupported;
}

uint8_t
S1gCapabilities::GetInformationFieldSize () const
{
  //we should not be here if s1g is not supported
  NS_ASSERT (m_s1gSupported > 0);
  return 15;
}

Buffer::Iterator
S1gCapabilities::Serialize (Buffer::Iterator i) const
{
  if (m_s1gSupported < 1)
    {
      return i;
    }
  return WifiInformationElement::Serialize (i);
}

uint16_t
S1gCapabilities::GetSerializedSize () const
{
  if (m_s1gSupported < 1)
    {
      return 0;
    }
  return WifiInformationElement::GetSerializedSize ();
}

void
S1gCapabilities::SerializeInformationField (Buffer::Iterator start) const
{
  if (m_s1gSupported == 1)
    {
      //write the corresponding value for each bit
      start.WriteHtolsbU16 (GetS1gCapabilitiesInfoH16 ());
      start.WriteHtolsbU64 (GetS1gCapabilitiesInfoL64 ());

      start.WriteHtolsbU32 (GetSupportedMcsAndNssSetH32 ());
      start.WriteU8 (GetSupportedMcsAndNssSetL8 ());
    }
}

uint8_t
S1gCapabilities::DeserializeInformationField (Buffer::Iterator start,
                                             uint8_t length)
{
  Buffer::Iterator i = start;
    //NS_LOG_UNCOND ("WifiInformationElement::DeserializeIfPresent 79" << length); //for test
  
  uint16_t s1ginfoH16 = i.ReadLsbtohU16 ();
  uint64_t s1ginfoL64 = i.ReadLsbtohU64 ();
  uint32_t MCSNSSH32 = i.ReadLsbtohU32 ();
  uint8_t MCSNSSL8 = i.ReadU8 ();
    
  SetS1gCapabilitiesInfoH16 (s1ginfoH16);
  SetS1gCapabilitiesInfoL64 (s1ginfoL64);
    
  SetSupportedMcsAndNssSetH32 (MCSNSSH32);
  SetSupportedMcsAndNssSetL8 (MCSNSSL8);
    
  return length;
}

void
S1gCapabilities::SetS1gCapabilitiesInfoL64 (uint64_t ctrl)
{
  m_s1gLongSupport = (ctrl >> 0) & 0x01;
  m_shortGuardIntervalFor1Mhz = (ctrl >> 1) & 0x01;
  m_shortGuardIntervalFor2Mhz = (ctrl >> 2) & 0x01;
  m_shortGuardIntervalFor4Mhz = (ctrl >> 3) & 0x01;
  m_shortGuardIntervalFor8Mhz = (ctrl >> 4) & 0x01;
  m_shortGuardIntervalFor16Mhz = (ctrl >> 5) & 0x01;
  m_supportedChannelWidth = (ctrl >> 6) & 0x03;
  m_rxLdpc = (ctrl >> 8) & 0x01;
  m_txStbc = (ctrl >> 9) & 0x01;
  m_rxStbc = (ctrl >> 10) & 0x01;
  m_suBeamformerCapable = (ctrl >> 11) & 0x01;
  m_suBeamformeeCapable = (ctrl >> 12) & 0x01;
  m_beamformeeStsCapable = (ctrl >> 13) & 0x07;
  m_numberOfSoundingDimensions = (ctrl >> 16) & 0x07;
  m_muBeamformerCapable = (ctrl >> 19) & 0x01;
  m_muBeamformeeCapable = (ctrl >> 20) & 0x01;
  m_htcVhtCapable = (ctrl >> 21) & 0x01;
  m_travwlingPilotSupport = (ctrl >> 22) & 0x03;
  m_rdResponder = (ctrl >> 24) & 0x01;
  m_htDelayedBlockAck = (ctrl >> 25) & 0x01;
  m_maxMpduLength = (ctrl >> 26) & 0x01;
  m_maxAmpduLengthExponent = (ctrl >> 27) & 0x03;
  m_minMpduStartSpacing = (ctrl >> 29) & 0x07;
  m_uplinkSyncCapable = (ctrl >> 32) & 0x01;
  m_dynamicAid = (ctrl >> 33) & 0x01;
  m_batSupport = (ctrl >> 34) & 0x01;
  m_timAdeSupport = (ctrl >> 35) & 0x01;
  m_nonTimSupport = (ctrl >> 36) & 0x01;
  m_groupAidSupport = (ctrl >> 37) & 0x01;
  m_staTypeSupport = (ctrl >> 38) & 0x03;
  m_centralizedAuthenticationControl = (ctrl >> 40) & 0x01;
  m_distributedAuthenticationControl = (ctrl >> 41) & 0x01;
  m_aMsduSupported = (ctrl >> 42) & 0x01;
  m_aMpduSupported = (ctrl >> 43) & 0x01;
  m_asymmetricBlockAckSupported = (ctrl >> 44) & 0x01;
  m_flowControlSupported = (ctrl >> 45) & 0x01;
  m_sectorizedBeamCapable = (ctrl >> 46) & 0x03;
  m_obssMitigationSupport = (ctrl >> 48) & 0x01;
  m_fragmentBaSupport = (ctrl >> 49) & 0x01;
  m_ndpPsPollSupported = (ctrl >> 50) & 0x01;
  m_rawOperationSupport = (ctrl >> 51) & 0x01;
  m_pageSlicingSupport = (ctrl >> 52) & 0x01;
  m_txopSharingImplicitAckSupport = (ctrl >> 53) & 0x01;
  m_vhtLinkAdaptationCapable = (ctrl >> 54) & 0x03;
  m_tackSupportAsPsPollResponse = (ctrl >> 56) & 0x01;
  m_duplicate1MHzSupport = (ctrl >> 57) & 0x01;
  m_mcsNegotiationSupport = (ctrl >> 58) & 0x01;
  m_1MHzControlResponsePreambleSupport = (ctrl >> 59) & 0x01;
  m_ndpBeamformingReportPollSupported = (ctrl >> 60) & 0x01;
  m_unsolicitedDynamicAid = (ctrl >> 61) & 0x01;
  m_sectorTrainingOperationSupported = (ctrl >> 62) & 0x01;
  m_temporaryPsModeSwitch = (ctrl >> 63) & 0x01;
}

void
S1gCapabilities::SetS1gCapabilitiesInfoH16 (uint16_t ctrl)
{
  m_twtGroupingSupport = (ctrl >> 0) & 0x01;
  m_dbtCapable = (ctrl >> 1) & 0x01;
  m_color = (ctrl >> 2) & 0x07;
  m_twtRequesterSupport = (ctrl >> 5) & 0x01;
  m_twtResponderSupport = (ctrl >> 6) & 0x01;
  m_pv1FrameSupport = (ctrl >> 7) & 0x01;
  m_linkAdaptationPerNormalControlResponseCapable = (ctrl >> 8) & 0x01;
}

uint64_t 
S1gCapabilities::GetS1gCapabilitiesInfoL64 () const
{
  uint64_t val = 0;
  val |= (m_s1gLongSupport & 0x01);
  val |= (m_shortGuardIntervalFor1Mhz & 0x01) << 1;
  val |= (m_shortGuardIntervalFor2Mhz & 0x01) << 2;
  val |= (m_shortGuardIntervalFor4Mhz & 0x01) << 3;
  val |= (m_shortGuardIntervalFor8Mhz & 0x01) << 4;
  val |= (m_shortGuardIntervalFor16Mhz & 0x01) << 5;
  val |= (m_supportedChannelWidth & 0x03) << 6;
  val |= (m_rxLdpc & 0x01) << 8;
  val |= (m_txStbc & 0x01) << 9;
  val |= (m_rxStbc & 0x01) << 10;
  val |= (m_suBeamformerCapable & 0x01) << 11;
  val |= (m_suBeamformeeCapable & 0x01) << 12;
  val |= (m_beamformeeStsCapable & 0x07) << 13;
  val |= (m_numberOfSoundingDimensions & 0x07) << 16;
  val |= (m_muBeamformerCapable & 0x01) << 19;
  val |= (m_muBeamformeeCapable & 0x01) << 20;
  val |= (m_htcVhtCapable & 0x01) << 21;
  val |= (m_travwlingPilotSupport & 0x03) << 22;
  val |= (m_rdResponder & 0x01) << 24;
  val |= (m_htDelayedBlockAck & 0x01) << 25;
  val |= (m_maxMpduLength & 0x01) << 26;
  val |= (m_maxAmpduLengthExponent & 0x03) << 27;
  val |= (m_minMpduStartSpacing & 0x07) << 29;
  val |= (m_uplinkSyncCapable & 0x01) << 32;
  val |= (m_dynamicAid & 0x01) << 33;
  val |= (m_batSupport & 0x01) << 34;
  val |= (m_timAdeSupport & 0x01) << 35;
  val |= (m_nonTimSupport & 0x01) << 36;
  val |= (m_groupAidSupport & 0x01) << 37;
  val |= (m_staTypeSupport & 0x03) << 38;
  val |= (m_centralizedAuthenticationControl & 0x01) << 40;
  val |= (m_distributedAuthenticationControl & 0x01) << 41;
  val |= (m_aMsduSupported & 0x01) << 42;
  val |= (m_aMpduSupported & 0x01) << 43;
  val |= (m_asymmetricBlockAckSupported & 0x01) << 44;
  val |= (m_flowControlSupported & 0x01) << 45;
  val |= (m_sectorizedBeamCapable & 0x03) << 46;
  val |= (m_obssMitigationSupport & 0x01) << 48;
  val |= (m_fragmentBaSupport & 0x01) << 49;
  val |= (m_ndpPsPollSupported & 0x01) << 50;
  val |= (m_rawOperationSupport & 0x01) << 51;
  val |= (m_pageSlicingSupport & 0x01) << 52;
  val |= (m_txopSharingImplicitAckSupport & 0x01) << 53;
  val |= (m_vhtLinkAdaptationCapable & 0x03) << 54;
  val |= (m_tackSupportAsPsPollResponse & 0x01) << 56;
  val |= (m_duplicate1MHzSupport & 0x01) << 57;
  val |= (m_mcsNegotiationSupport & 0x01) << 58;
  val |= (m_1MHzControlResponsePreambleSupport & 0x01) << 59;
  val |= (m_ndpBeamformingReportPollSupported & 0x01) << 60;
  val |= (m_unsolicitedDynamicAid & 0x01) << 61;
  val |= (m_sectorTrainingOperationSupported & 0x01) << 62;
  val |= (m_temporaryPsModeSwitch & 0x01) << 63;
  return val;
}

uint16_t
S1gCapabilities::GetS1gCapabilitiesInfoH16 (void) const
{
  uint16_t val = 0;
  val |= (m_twtGroupingSupport & 0x01);
  val |= (m_dbtCapable & 0x01) << 1;
  val |= (m_color & 0x07) << 2;
  val |= (m_twtRequesterSupport & 0x01) << 5;
  val |= (m_twtResponderSupport & 0x01) << 6;
  val |= (m_pv1FrameSupport & 0x01) << 7;
  val |= (m_linkAdaptationPerNormalControlResponseCapable & 0x01) << 8;
  return val;
}

void
S1gCapabilities::SetSupportedMcsAndNssSetL8 (uint8_t ctrl)
{
  uint8_t n;
  for (uint8_t i = 0; i < 4; i++)
    {
      n = i * 2;
      m_rxS1gMcsMap[i] = (ctrl >> n) & 0x03;
    }
}

void
S1gCapabilities::SetSupportedMcsAndNssSetH32 (uint32_t ctrl)
{
  m_rxHighestSupportedLongGuardIntervalDataRate = ctrl & 0x0777;
  for (uint8_t i = 0; i < 4; i++)
    {
      uint16_t n = (i * 2) + 9;
      m_txS1gMcsMap[i] = (ctrl >> n) & 0x03;
    }
  m_txHighestSupportedLongGuardIntervalDataRate = (ctrl >> 17) & 0x0777;
  m_rxSingleSpatialStreamAndS1gMcsMapFor1MHz = (ctrl >> 26) & 0x03;
  m_txSingleSpatialStreamAndS1gMcsMapFor1MHz = (ctrl >> 28) & 0x03;
}

uint8_t
S1gCapabilities::GetSupportedMcsAndNssSetL8 () const
{
  uint8_t val = 0;
  uint8_t n;
  for (uint8_t i = 0; i < 4; i++)
    {
      n = i * 2;
      val |= ((uint8_t)m_rxS1gMcsMap[i] & 0x03) << n;
    }
  return val;
}

uint32_t
S1gCapabilities::GetSupportedMcsAndNssSetH32 () const
{
  uint32_t val = 0;
  uint8_t n;
  val |=  ((uint32_t)m_rxHighestSupportedLongGuardIntervalDataRate & 0x0777);
  for (uint8_t i = 0; i < 8; i++)
    {
      n = (i * 2) + 9;
      val |= ((uint32_t)m_txS1gMcsMap[i] & 0x03) << n;
    }
  val |= ((uint32_t)m_txHighestSupportedLongGuardIntervalDataRate & 0x0777) << 17;
  val |= ((uint32_t)m_rxSingleSpatialStreamAndS1gMcsMapFor1MHz & 0x03) << 26;
  val |= ((uint32_t)m_txSingleSpatialStreamAndS1gMcsMapFor1MHz & 0x03) << 28;
  return val;
}

//Capabilities Information field
//Set
void
S1gCapabilities::SetShortGuardIntervalFor1Mhz (uint8_t shortGI)
{
  m_shortGuardIntervalFor1Mhz = shortGI;
}
void
S1gCapabilities::SetShortGuardIntervalFor2Mhz (uint8_t shortGI)
{
  m_shortGuardIntervalFor1Mhz = shortGI;
}
void
S1gCapabilities::SetShortGuardIntervalFor4Mhz (uint8_t shortGI)
{
  m_shortGuardIntervalFor1Mhz = shortGI;
}
void
S1gCapabilities::SetShortGuardIntervalFor8Mhz (uint8_t shortGI)
{
  m_shortGuardIntervalFor1Mhz = shortGI;
}
void
S1gCapabilities::SetShortGuardIntervalFor16Mhz (uint8_t shortGI)
{
  m_shortGuardIntervalFor1Mhz = shortGI;
}
void
S1gCapabilities::SetSupportedChannelWidthSet (uint8_t channelwidthset)
{
  //802.11ah-2016 Table 9-262n
  // 0, 1/2 MHz bandwidth
  // 1, 1/2/4
  // 2, 1/2/4/8
  // 3, 1/2/4/8/16
  m_supportedChannelWidth = channelwidthset;
}
void
S1gCapabilities::SetRxLdpc (uint8_t rxldpc)
{
  m_rxLdpc = rxldpc;
}
void
S1gCapabilities::SetMaxMpduLength (uint8_t length)
{
  m_maxMpduLength = length;
}
void
S1gCapabilities::SetMaxAmpduLengthExponent (uint8_t exponent)
{
  m_maxAmpduLengthExponent = exponent;
}
void
S1gCapabilities::SetStaTypeSupport (uint8_t type)
{
  m_staTypeSupport = type;
}
void
S1gCapabilities::SetNdpPsPollSupport (uint8_t psPollingSupported)
{
	NS_ASSERT (psPollingSupported <= 1);
	m_ndpPsPollSupported = psPollingSupported;
}
void
S1gCapabilities::SetPageSlicingSupport (uint8_t pageSlicingImplemented)
{
	NS_ASSERT (pageSlicingImplemented <= 1);
	m_pageSlicingSupport = pageSlicingImplemented;
}

//Get
uint8_t
S1gCapabilities::GetShortGuardIntervalFor1Mhz () const
{
  return m_shortGuardIntervalFor1Mhz;
}
uint8_t
S1gCapabilities::GetShortGuardIntervalFor2Mhz () const
{
  return m_shortGuardIntervalFor2Mhz;
}
uint8_t
S1gCapabilities::GetShortGuardIntervalFor4Mhz () const
{
  return m_shortGuardIntervalFor4Mhz;
}
uint8_t
S1gCapabilities::GetShortGuardIntervalFor8Mhz () const
{
  return m_shortGuardIntervalFor8Mhz;
}
uint8_t
S1gCapabilities::GetShortGuardIntervalFor16Mhz () const
{
  return m_shortGuardIntervalFor16Mhz;
}
uint8_t
S1gCapabilities::GetSupportedChannelWidthSet () const
{
  return m_supportedChannelWidth;
}
uint8_t
S1gCapabilities::GetRxLdpc () const
{
  return m_rxLdpc;
}
uint8_t
S1gCapabilities::GetMaxMpduLength () const
{
  return m_maxMpduLength;
}
uint8_t
S1gCapabilities::GetMaxAmpduLengthExponent () const
{
  return m_maxAmpduLengthExponent;
}

uint8_t
S1gCapabilities::GetStaTypeSupport () const
{
  return m_staTypeSupport;
}
uint8_t
S1gCapabilities::GetNdpPsPollSupport () const
{
	return m_ndpPsPollSupported;
}
uint8_t
S1gCapabilities::GetPageSlicingSupport () const
{
	return m_pageSlicingSupport;
}




void
S1gCapabilities::SetRxS1gMcsMap (uint8_t map)
{
  //Set each element in the map accoriding to the 2 bits representing it in 802.11ah-2016 9.4.2.201.3 Supported S1G-MCS and NSS Set field
  uint8_t n;
  for (uint8_t i = 0; i < 4; i++)
    {
      n = i * 2;
      m_rxS1gMcsMap[i] = (map >> n) & 0x03;
    }
}

void
S1gCapabilities::SetRxS1gMcsMap (uint8_t mcs, uint8_t nss)
{
  //MCS index should be 2 or 7 or 9
  NS_ASSERT (mcs == 2 && mcs ==7 && mcs == 9);
  switch (mcs)
    {
    case 2:
      m_rxS1gMcsMap[nss - 1] = 0;
      break;
    case 7:
      m_rxS1gMcsMap[nss - 1] = 1;
      break;
    case 9:
      m_rxS1gMcsMap[nss - 1] = 2;
      break;
    default:
      NS_ASSERT (false);
    }
}

void
S1gCapabilities::SetTxS1gMcsMap (uint8_t map)
{
  //Set each element in the map accoriding to the 2 bits representing it in 802.11ah-2016 9.4.2.201.3 Supported S1G-MCS and NSS Set field
  uint8_t n;
  for (uint8_t i = 0; i < 4; i++)
    {
      n = i * 2;
      m_txS1gMcsMap[i] = (map >> n) & 0x03;
    }
}

void
S1gCapabilities::SetTxS1gMcsMap (uint8_t mcs, uint8_t nss)
{
  //MCS index should be 2 or 7 or 9
  NS_ASSERT (mcs == 2 && mcs ==7 && mcs == 9);
  switch (mcs)
  {
  case 2:
    m_txS1gMcsMap[nss - 1] = 0;
    break;
  case 7:
    m_txS1gMcsMap[nss - 1] = 1;
    break;
  case 9:
    m_txS1gMcsMap[nss - 1] = 2;
    break;
  default:
    NS_ASSERT (false);
  }
}

bool
S1gCapabilities::IsSupportedTxS1gMcs (uint8_t mcs, uint8_t channelwidth) const
{
  NS_ASSERT (mcs >= 0 && mcs <= 10);
  if (mcs <= 2)
    {
      return true;
    }
  else if (mcs <= 7 && (m_txS1gMcsMap[0] == 1 || m_txS1gMcsMap[0] == 2))
    {
      return true;
    }
  else if (mcs <= 9 && m_txS1gMcsMap[0] == 2)
    {
      return true;
    }
  else if (mcs == 10 && channelwidth == 1) //For 1 MHz, MCS10 is always supported.
    {
      return true;
    }
  else
    {
      return false;
    }
}


//MCS and NSS field information
bool
S1gCapabilities::IsSupportedRxS1gMcs (uint8_t mcs, uint8_t channelwidth) const
{
  NS_ASSERT (mcs >= 0 && mcs <= 10);
  if (mcs <= 2)
    {
      return true;
    }
  else if (mcs <= 7 && (m_rxS1gMcsMap[0] == 1 || m_rxS1gMcsMap[0] == 2))
    {
      return true;
    }
  else if (mcs <= 9 && m_rxS1gMcsMap[0] == 2)
    {
      return true;
    }
  else if (mcs == 10 && channelwidth == 1) //For 1 MHz, MCS10 is always supported.
    {
      return true;
    }
  else
    {
      return false;
    }
}

void
S1gCapabilities::SetRxHighestSupportedLgiDataRate (uint16_t supporteddatarate)
{
  m_rxHighestSupportedLongGuardIntervalDataRate = supporteddatarate;
}

void
S1gCapabilities::SetTxHighestSupportedLgiDataRate (uint16_t supporteddatarate)
{
  m_txHighestSupportedLongGuardIntervalDataRate = supporteddatarate;
}



bool
S1gCapabilities::IsSupportedMcs (uint8_t mcs, uint8_t nss) const
{
  //The MCS index starts at 0 and NSS starts at 1
  if (mcs <= 7 && m_rxS1gMcsMap[nss - 1] < 3)
    {
      return true;
    }
  if (mcs == 8 && m_rxS1gMcsMap[nss - 1] > 0 && m_rxS1gMcsMap[nss - 1] < 3)
    {
      return true;
    }
  if (mcs == 9 && m_rxS1gMcsMap[nss - 1] == 2)
    {
      return true;
    }
  return false;
}

uint8_t
S1gCapabilities::GetRxS1gMcsMap () const
{
  uint8_t val = 0;
  uint8_t n;
  for (uint8_t i = 0; i < 4; i++)
    {
      n = i * 2;
      val |= (m_rxS1gMcsMap[i] & 0x03) << n;
    }
  return val;
}

uint8_t
S1gCapabilities::GetTxS1gMcsMap () const
{
  uint8_t val = 0;
  uint8_t n;
  for (uint8_t i = 0; i < 4; i++)
    {
      n = i * 2;
      val |= (m_txS1gMcsMap[i] & 0x03) << n;
    }
  return val;
}

uint16_t
S1gCapabilities::GetRxHighestSupportedLgiDataRate () const
{
  return m_rxHighestSupportedLongGuardIntervalDataRate;
}

uint16_t
S1gCapabilities::GetTxHighestSupportedLgiDataRate () const
{
  return m_txHighestSupportedLongGuardIntervalDataRate;
}


ATTRIBUTE_HELPER_CPP (S1gCapabilities);

std::ostream &
operator << (std::ostream &os, const S1gCapabilities &s1gcapabilities)
{
  os <<  bool (s1gcapabilities.GetStaTypeSupport ());

  return os;
}

std::istream &operator >> (std::istream &is,S1gCapabilities &s1gcapabilities)
{
  bool c1;
  is >> c1;
  s1gcapabilities.SetS1gSupported (c1);

  return is;
}

} //namespace ns3
