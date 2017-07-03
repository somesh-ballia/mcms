// +========================================================================+
// BridgeVideoOutCopEncoder.cpp                                             |
// Copyright 2005 Polycom                                                   |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// FILE:       BridgeVideoOutCopEncoder.cpp                                 |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Keren                                                        |
// -------------------------------------------------------------------------|
// Who  | Date  Aug-2009  | Description                                     |
// -------------------------------------------------------------------------|
// +========================================================================+

#include "BridgeVideoOutCopEncoder.h"
#include "VideoHardwareInterface.h"
#include "StatusesGeneral.h"
#include "Layout.h"
#include "LayoutHandler.h"
#include "TraceStream.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "HostCommonDefinitions.h"
#include "ConfPartyGlobals.h"
#include "SiteNameInfo.h"
#include "VideoApiDefinitionsStrings.h"

// Time-out values
#define VIDEO_OUT_CHANGE_LAYOUT_TOUT 15 ///8*SECOND // 8 second timeout

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable(void);
LayoutType GetNewLayoutType(const BYTE oldLayoutType);

PBEGIN_MESSAGE_MAP(CBridgeVideoOutCopEncoder)
  ONEVENT(CHANGECOPLAYOUT,           IDLE,          CBridgeVideoOutCopEncoder::NullActionFunction)
  ONEVENT(CHANGECOPLAYOUT,           SETUP,         CBridgeVideoOutCopEncoder::NullActionFunction)
  ONEVENT(CHANGECOPLAYOUT,           CONNECTED,     CBridgeVideoOutCopEncoder::OnVideoBridgeChangeCopLayoutCONNECTED)
  ONEVENT(CHANGECOPLAYOUT,           CHANGE_LAYOUT, CBridgeVideoOutCopEncoder::OnVideoBridgeChangeCopLayoutCHANGE_LAYOUT)
  ONEVENT(CHANGECOPLAYOUT,           DISCONNECTING, CBridgeVideoOutCopEncoder::NullActionFunction)

  ONEVENT(CHANGECOPLAYOUTATTRIBUTES, IDLE,          CBridgeVideoOutCopEncoder::NullActionFunction)
  ONEVENT(CHANGECOPLAYOUTATTRIBUTES, SETUP,         CBridgeVideoOutCopEncoder::NullActionFunction)
  ONEVENT(CHANGECOPLAYOUTATTRIBUTES, CONNECTED,     CBridgeVideoOutCopEncoder::OnVideoBridgeChangeCopLayoutAttributesCONNECTED)
  ONEVENT(CHANGECOPLAYOUTATTRIBUTES, CHANGE_LAYOUT, CBridgeVideoOutCopEncoder::OnVideoBridgeChangeCopLayoutAttributesCHANGE_LAYOUT)
  ONEVENT(CHANGECOPLAYOUTATTRIBUTES, DISCONNECTING, CBridgeVideoOutCopEncoder::NullActionFunction)

  ONEVENT(VIDEO_ENCODER_SYNC_IND,    IDLE,          CBridgeVideoOutCopEncoder::NullActionFunction)
  ONEVENT(VIDEO_ENCODER_SYNC_IND,    SETUP,         CBridgeVideoOutCopEncoder::NullActionFunction)
  ONEVENT(VIDEO_ENCODER_SYNC_IND,    CONNECTED,     CBridgeVideoOutCopEncoder::OnMplEncoderSyncIndCONNECTED)
  ONEVENT(VIDEO_ENCODER_SYNC_IND,    CHANGE_LAYOUT, CBridgeVideoOutCopEncoder::OnMplEncoderSyncIndCHANGE_LAYOUT)
  ONEVENT(VIDEO_ENCODER_SYNC_IND,    DISCONNECTING, CBridgeVideoOutCopEncoder::NullActionFunction)

  ONEVENT(UPDATE_FLOW_CONTROL_RATE,  IDLE,          CBridgeVideoOutCopEncoder::NullActionFunction)
  ONEVENT(UPDATE_FLOW_CONTROL_RATE,  SETUP,         CBridgeVideoOutCopEncoder::NullActionFunction)
  ONEVENT(UPDATE_FLOW_CONTROL_RATE,  CONNECTED,     CBridgeVideoOutCopEncoder::OnUpdateFlowControlRateCONNECTED)
  ONEVENT(UPDATE_FLOW_CONTROL_RATE,  CHANGE_LAYOUT, CBridgeVideoOutCopEncoder::OnUpdateFlowControlRateCHANGE_LAYOUT)
  ONEVENT(UPDATE_FLOW_CONTROL_RATE,  DISCONNECTING, CBridgeVideoOutCopEncoder::NullActionFunction)
PEND_MESSAGE_MAP(CBridgeVideoOutCopEncoder, CBridgePartyVideoOut);


////////////////////////////////////////////////////////////////////////////
//                        CBridgeVideoOutCopEncoder
////////////////////////////////////////////////////////////////////////////
CBridgeVideoOutCopEncoder::CBridgeVideoOutCopEncoder() : CBridgePartyVideoOut()
{
  m_videoConfType                = eVideoConfTypeCopHD108025fps; // This is the default value, if it eVideoConfTypeCopHD72050fps we will receive via the init params
  m_copLrt                       = eLogical_res_none;
  m_copEncoderIndex              = (0xFFFF);
  m_pCopConfLayout               = NULL;
  m_pCopConfSpeakerPlaceInLayout = 0;
  m_flowControlRate              = 0;  // No flow control rate is received
  m_croppingVer                  = 50; // the cropping values of the cop encoders should be always 50
  m_croppingHor                  = 50; // the cropping values of the cop encoders should be always 50
}
// ------------------------------------------------------------------------------------------
CBridgeVideoOutCopEncoder::~CBridgeVideoOutCopEncoder()
{
  POBJDELETE(m_pCopConfLayout);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pBridgePartyVideoOutParams)
{
  CBridgePartyVideoUniDirection::Create(pBridgePartyCntl, pRsrcParams, pBridgePartyVideoOutParams);
  TRACEINTO << "CBridgeVideoOutCopEncoder::Create - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();

  if (((CBridgePartyVideoOutParams*)pBridgePartyVideoOutParams)->GetVisualEffects() != NULL)
    m_pPartyVisualEffects = new CVisualEffectsParams(*((CBridgePartyVideoOutParams*)pBridgePartyVideoOutParams)->GetVisualEffects());
  else
    m_pPartyVisualEffects = new CVisualEffectsParams();

  m_pCurrentView      = new CLayout(GetConfLayoutType(), m_pBridgePartyCntl->GetConfName());                // keren to decide if we use this as the layout* we receive in COP
  m_videoQuality       = ((CBridgePartyVideoOutParams*)pBridgePartyVideoOutParams)->GetVideoQualityType();  // KEREN
  m_isSiteNamesEnabled = ((CBridgePartyVideoOutParams*) pBridgePartyVideoOutParams)->GetIsSiteNamesEnabled();
  m_videoFrameRate     = ((CBridgePartyVideoOutParams*)pBridgePartyVideoOutParams)->GetVidFrameRate();      // relevant only in cop confrences
  m_copLrt             = ((CBridgePartyVideoOutParams*)pBridgePartyVideoOutParams)->GetCopLrt();
  m_copEncoderIndex    = ((CBridgePartyVideoOutParams*)pBridgePartyVideoOutParams)->GetCopResourceIndex();
  m_videoConfType      = ((CBridgePartyVideoOutParams*)pBridgePartyVideoOutParams)->GetVideoConfType();
  m_pSiteNameInfo      = new CSiteNameInfo();
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::ChangeLayout(CLayout* layout, CVisualEffectsParams* visualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout, BYTE sendChangeLayoutAnyWay)
{
  BYTE isLayoutChanged           = FALSE;
  BYTE isLayoutAttributesChanged = FALSE;
  if (m_pPartyVisualEffects == NULL || (!(*m_pPartyVisualEffects == *visualEffects)) || (speakerPlaceInLayout != m_pCopConfSpeakerPlaceInLayout))
  {
    POBJDELETE(m_pPartyVisualEffects);
    m_pPartyVisualEffects          = new CVisualEffectsParams(visualEffects);
    m_pCopConfSpeakerPlaceInLayout = speakerPlaceInLayout;
    isLayoutAttributesChanged      = TRUE;
  }

  if (!CPObject::IsValidPObjectPtr(layout)) {
    PASSERT_AND_RETURN(101);
  }

  // VNGR-18594 In case of delayed disconnection of party its sub image could be deleted an as a result because it wasnt updated in the m_pCopConfLayout when we compare the layouts we could have reach deleted pointer.
  // in this case we will send in the next change layout sendChangeLayoutAnyWay
  if (sendChangeLayoutAnyWay)
  {
    PTRACE(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::ChangeLayout sendChangeLayoutAnyWay");
    POBJDELETE(m_pCopConfLayout);
    m_pCopConfLayout = new CLayout(*layout);
    isLayoutChanged  = TRUE;
  }
  else if ((m_pCopConfLayout == NULL) || (CPObject::IsValidPObjectPtr(m_pCopConfLayout) && *layout != *m_pCopConfLayout))
  {
    POBJDELETE(m_pCopConfLayout);
    m_pCopConfLayout = new CLayout(*layout);
    isLayoutChanged  = TRUE;
  }

  if (pSiteNameInfo)
    *m_pSiteNameInfo = *pSiteNameInfo;
  else
    m_pSiteNameInfo->SetDisplayMode(eSiteNameOff);

  if (isLayoutChanged || pSiteNameInfo != NULL)
  {
    PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::ChangeLayout for sitename or layout being changed, encoder index: ", m_copEncoderIndex);
    DispatchEvent(CHANGECOPLAYOUT, NULL);
  }
  else
  {
    if (isLayoutAttributesChanged)
    {
      PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::ChangeLayout only attributes changed in encoder index: ", m_copEncoderIndex);
      DispatchEvent(CHANGECOPLAYOUTATTRIBUTES, NULL);
    }
    else
    {
      PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::ChangeLayout no change: ", m_copEncoderIndex);
    }
  }
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::ChangeLayoutAttributes(CVisualEffectsParams* visualEffects, DWORD speakerPlaceInLayout)
{
  PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::ChangeLayoutAttributes - EncoderIndex:", m_copEncoderIndex);

  POBJDELETE(m_pPartyVisualEffects);
  m_pPartyVisualEffects          = new CVisualEffectsParams(visualEffects);
  m_pCopConfSpeakerPlaceInLayout = speakerPlaceInLayout;

  DispatchEvent(CHANGECOPLAYOUTATTRIBUTES, NULL);
}
// ------------------------------------------------------------------------------------------
WORD CBridgeVideoOutCopEncoder::IsValidState(WORD state) const
{
  WORD valid_state = TRUE;
  switch (state)
  {
    case SLIDE:
    {
      valid_state = FALSE;
      PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::IsValidState - Failed, invalid state, EncoderIndex:", m_copEncoderIndex);
      PASSERT(1);
      break;
    }
  } // switch

  return valid_state;
}
// ------------------------------------------------------------------------------------------
WORD CBridgeVideoOutCopEncoder::IsValidEvent(OPCODE event) const
{
  WORD valid_event = TRUE;
  switch (event)
  {
    case UPDATE_VIDEO_OUT_PARAMS:
    case ADDIMAGE:
    case DELIMAGE:
    case MUTEIMAGE:
    case UNMUTEIMAGE:
    case SPEAKERS_CHANGED:
    case AUDIO_SPEAKER_CHANGED:
    case CHANGECONFLAYOUT:
    case CHANGEPARTYLAYOUT:
    case CHANGEPARTYPRIVATELAYOUT:
    case SETPARTYPRIVATELAYOUTONOFF:
    case IVR_SHOW_SLIDE_REQ:
    case IVR_STOP_SHOW_SLIDE_REQ:
    case PLC_SETPARTYPRIVATELAYOUTTYPE:
    case PLC_RETURNPARTYTOCONFLAYOUT:
    case PLC_FORCECELLZERO:
    case PLC_CANCELALLPRIVATELAYOUTFORCES:
    case UPDATE_DECODER_DETECTED_MODE:
    case SLIDE_INTRA_TOUT:
    {
      valid_event = FALSE;
      TRACEINTO << "CBridgeVideoOutCopEncoder::IsValidEvent - Failed, not valid event, ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
      PASSERT(1);
      break;
    }
  } // switch

  return valid_event;
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnVideoBridgePartyConnectIDLE(CSegment* pParam)
{
  TRACEINTO << "CBridgeVideoOutCopEncoder::OnVideoBridgePartyConnectIDLE - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
  m_state = SETUP;
  SetClosePortAckStatus(STATUS_OK);
  SendOpenEncoder();
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnMplAckSETUP(CSegment* pParam)
{
  OPCODE AckOpcode;
  DWORD  ack_seq_num;
  STATUS status;
  *pParam >> AckOpcode >> ack_seq_num >> status;

  switch (AckOpcode)
  {
    case  TB_MSG_OPEN_PORT_REQ:
    {
      OnMplOpenPortAck(status);
      break;
    }

    default:
    {
      CProcessBase* process = CProcessBase::GetProcess();
      std::string   str     = process->GetOpcodeAsString(AckOpcode);
      TRACEINTO << "CBridgeVideoOutCopEncoder::OnMplAckSETUP - ACK_IND Ignored, ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName() << ", AckOpcode:" << str.c_str();
    }
  } // end switch

}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnMplOpenPortAck(STATUS status)
{
  if (status != STATUS_OK)
  {
    TRACEINTO << "CBridgeVideoOutCopEncoder::OnMplOpenPortAck - Status not OK, so send answer to VideoBridge, ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
    // Add assert to EMA in case of NACK
    AddFaultAlarm("NACK on open video encoder", m_pHardwareInterface->GetPartyRsrcId(), status);

    CSegment* pSeg = new CSegment;
    *pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)eMipOut << (BYTE)eMipStatusFail << (BYTE)eMipOpen;

    // Inform BridgePartyCntl of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder VideoDecoder
    m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_CONNECTED);
    POBJDELETE(pSeg);
  }
  else
  {
    TRACEINTO << "CBridgeVideoOutCopEncoder::OnMplOpenPortAck - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
    m_state = CONNECTED;

    CSegment* pMsg = new CSegment;
    *pMsg << (BYTE)statOK;
    m_pBridgePartyCntl->HandleEvent(pMsg, 0, VIDEO_OUT_CONNECTED);
    POBJDELETE(pMsg);

    eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
    if (IsFeatureSupportedBySystem(eFeatureEncoderRecurrentIntra))
    {
      DWORD encoderRequestIntraToutValue = 0;
      CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("ENCODER_RECURRENT_INTRA_REQ_MINUTES", encoderRequestIntraToutValue);
      if (encoderRequestIntraToutValue)
        StartTimer(ENCODER_RECURRENT_INTRA_REQ_TIMEOUT, encoderRequestIntraToutValue*60*SECOND);
    }
  }
}
// ------------------------------------------------------------------------------------------
BYTE CBridgeVideoOutCopEncoder::BuildLayout()
{
  // WE don't use the BuildLayout function but the ChangeLayout function from the VBCop
  TRACEINTO << "CBridgeVideoOutCopEncoder::BuildLayout - Ignored, ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
  PASSERT(1);
  return 0;
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::UpdateNewConfParams(DWORD confRsrcId, const CBridgePartyMediaParams* pBridgePartyVideoOutParams)
{
  TRACEINTO << "CBridgeVideoOutCopEncoder::UpdateNewConfParams - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();

  CBridgePartyVideoUniDirection::UpdateNewConfParams(confRsrcId, pBridgePartyVideoOutParams);

  m_videoResolution = ((CBridgePartyVideoOutParams*)pBridgePartyVideoOutParams)->GetVideoResolution();

  if (CPObject::IsValidPObjectPtr(m_pPartyVisualEffects))
    POBJDELETE(m_pPartyVisualEffects);

  CVisualEffectsParams* pVisualEffectsParams = ((CBridgePartyVideoOutParams*)pBridgePartyVideoOutParams)->GetVisualEffects();
  if (pVisualEffectsParams != NULL)
    m_pPartyVisualEffects = new CVisualEffectsParams(*pVisualEffectsParams);
  else
    m_pPartyVisualEffects = NULL;

  for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; i++)
    m_pReservation[i] = new CLayout((LayoutType)i, m_pBridgePartyCntl->GetConfName());

  for (int i = (int)CP_LAYOUT_1X1; i < (int)CP_NO_LAYOUT; i++)
    m_pPrivateReservation[i] = new CLayout((LayoutType)i, m_pBridgePartyCntl->GetConfName());

  m_pCurrentView = new CLayout(GetConfLayoutType(), m_pBridgePartyCntl->GetConfName());

  SetLayoutHandler();

  m_partyLectureModeRole = ((CBridgePartyVideoOutParams*)pBridgePartyVideoOutParams)->GetPartyLectureModeRole();
  m_videoQuality         = ((CBridgePartyVideoOutParams*)pBridgePartyVideoOutParams)->GetVideoQualityType();
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::SetLayoutHandler()
{
  m_pCurrHandler = new CLayoutHandler((CVideoBridge*)(m_pBridgePartyCntl->GetBridge()), (CVideoBridgePartyCntl*)m_pBridgePartyCntl);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnVideoBridgeChangeCopLayoutCONNECTED(CSegment* pParam)
{
  SendChangeLayoutToHardware();
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnVideoBridgeChangeCopLayoutCHANGE_LAYOUT(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::OnVideoBridgeChangeCopLayoutCHANGE_LAYOUT - EncoderIndex:", m_copEncoderIndex);
  ON(m_layoutChangedWhileWaitingForAck);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::SendChangeLayoutToHardware(BYTE isVSW)
{
  PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::SendChangeLayoutToHardware - EncoderIndex:", m_copEncoderIndex);
  PASSERT_AND_RETURN(!m_pHardwareInterface);

  // changeLayout timeout - debug assert and return to state connect when end
  StartTimer(VIDEO_OUT_CHANGELAYOUT_TIMEOUT, VIDEO_OUT_CHANGE_LAYOUT_TOUT);

  isVSW = !IsVsw() ? FALSE : TRUE;

  if (m_pSiteNameInfo && m_pSiteNameInfo->GetDisplayMode() == eSiteNameAuto)
    StartSiteNamesOffTimer();

  if (m_pSiteNameInfo)
    m_pSiteNameInfo->Dump("CBridgeVideoOutCopEncoder::SendChangeLayoutToHardware");

  m_state = CHANGE_LAYOUT;
  BOOL bUseSharedMemForChangeLayoutReq = FALSE;

	//get if CONF is in telepresence mode
	BYTE isTelePresenceMode = 0;
	CVideoBridge* pBridge = (CVideoBridge*)m_pBridgePartyCntl->GetBridge();
	if (pBridge)
	{
		CConf* pConf = pBridge->GetConf();
		if (pConf)
		{
			const CCommConf* pCommConf = pConf->GetCommConf();
			if (pCommConf)
			{
				isTelePresenceMode = pCommConf->GetIsTelePresenceMode();
			}
		}
	}

  ((CVideoHardwareInterface*)m_pHardwareInterface)->ChangeLayoutSendOrUpdate(m_pCopConfLayout, m_pPartyVisualEffects, m_pSiteNameInfo, m_pCopConfSpeakerPlaceInLayout, m_videoResolution, m_decoderDetectedModeWidth, m_decoderDetectedModeHeight, m_decoderDetectedSampleAspectRatioWidth, m_decoderDetectedSampleAspectRatioHeight, m_videoAlg, m_FS, m_MBPS, m_videoConfType, m_isSiteNamesEnabled, isTelePresenceMode, bUseSharedMemForChangeLayoutReq, isVSW);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::SendChangeLayoutAttributesToHardware(BYTE isFromAudioSpeakerChange)
{
  PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::SendChangeLayoutAttributesToHardware - EncoderIndex:", m_copEncoderIndex);
  PASSERT_AND_RETURN(!m_pHardwareInterface);

  ((CVideoHardwareInterface*)m_pHardwareInterface)->SendChangeLayoutAttributes(m_pCopConfLayout,m_pPartyVisualEffects, m_pCopConfSpeakerPlaceInLayout, m_pSiteNameInfo, m_videoConfType);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnVideoBridgeChangeCopLayoutAttributesCONNECTED(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::OnVideoBridgeChangeCopLayoutAttributesCONNECTED - EncoderIndex:", m_copEncoderIndex);
  SendChangeLayoutAttributesToHardware();
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnVideoBridgeChangeCopLayoutAttributesCHANGE_LAYOUT(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::OnVideoBridgeChangeCopLayoutAttributesCHANGE_LAYOUT - EncoderIndex:", m_copEncoderIndex);

  ON(m_visualEffectsChangedWhileWaitingForAck);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnMplEncoderSyncIndCONNECTED(CSegment* pParam)
{
  TRACEINTO << "CBridgeVideoOutCopEncoder::OnMplEncoderSyncIndCONNECTED - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
  OnMplEncoderSyncInd(pParam);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnMplEncoderSyncIndCHANGE_LAYOUT(CSegment* pParam)
{
  TRACEINTO << "CBridgeVideoOutCopEncoder::OnMplEncoderSyncIndCHANGE_LAYOUT - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
  OnMplEncoderSyncInd(pParam);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnMplEncoderSyncInd(CSegment* pParam)
{
  if (IsVsw())
  {
    CSegment* pSeg = new CSegment;
    *pSeg << (DWORD)m_copEncoderIndex;
    CVideoBridge* pBridge = (CVideoBridge*)(m_pBridgePartyCntl->GetBridge());
    pBridge->HandleEvent(pSeg, 0, VIDEO_ENCODER_SYNC_IND);
    POBJDELETE(pSeg);
  }
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnMplChangeLayoutAck(STATUS status)
{
  PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::OnMplChangeLayoutAck - EncoderIndex:", m_copEncoderIndex);
  DeleteTimer(VIDEO_OUT_CHANGELAYOUT_TIMEOUT);

  if (status != STATUS_OK)
    DBGPASSERT(status);

  m_state = CONNECTED;

  if (m_sendFastUpdateAtChangeLayoutAck)
  {
    PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::OnMplChangeLayoutAck, we will send the fast update request which we delayed when we were in change layout state : Encoder Index - ", m_copEncoderIndex);
    if (!m_pHardwareInterface)
      PASSERT(1);
    else
      ((CVideoHardwareInterface*)m_pHardwareInterface)->SendFastUpdate();

    m_sendFastUpdateAtChangeLayoutAck = NO;
  }

  BYTE wasNewLayoutSent = FALSE;
  if (m_resolutionChangedWhileWaitingForAck)
  {
    BuildLayoutAndSendOnEncoderResolutionChange();
    wasNewLayoutSent = TRUE;
  }

  if (m_layoutChangedWhileWaitingForAck && !wasNewLayoutSent)
  {
    wasNewLayoutSent = TRUE;
    SendChangeLayoutToHardware();
  }

  if (m_visualEffectsChangedWhileWaitingForAck && !wasNewLayoutSent) // KEREN
    SendChangeLayoutAttributesToHardware(); // might be sent when not required (when there is no speaker in layout and the speaker indication params change - harmless)

  OFF(m_layoutChangedWhileWaitingForAck);
  OFF(m_visualEffectsChangedWhileWaitingForAck);
  OFF(m_resolutionChangedWhileWaitingForAck);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::SendOpenEncoder()
{
  PASSERT_AND_RETURN(!m_pHardwareInterface);

  ON(m_isPortOpened);

  DWORD parsingMode = E_PARSING_MODE_CP;
  if (IsVsw())
    parsingMode = E_PARSING_MODE_PSEUDO_VSW;

  ((CVideoHardwareInterface*)m_pHardwareInterface)->SendOpenEncoder(
    m_videoAlg, m_videoBitRate, m_videoQcifFrameRate, m_videoCifFrameRate, m_video4CifFrameRate,
    m_decoderDetectedModeWidth, m_decoderDetectedModeHeight, m_decoderDetectedSampleAspectRatioWidth, m_decoderDetectedSampleAspectRatioHeight,
    m_videoResolution, m_MBPS, m_FS, m_sampleAspectRatio, m_staticMB, m_videoQuality, m_isVideoClarityEnabled,
    m_videoConfType, m_maxDPB, m_pBridgePartyCntl->GetCascadeLinkMode() != NONE, parsingMode, m_eTelePresenceMode,
    m_videoFrameRate, E_VIDEO_RESOLUTION_TABLE_REGULAR, m_croppingHor, m_croppingVer, m_profile, m_packetPayloadFormat,
    false, false, false,0, // the method's *default* values
    GetFontType(), m_isH263Plus);
}
// ------------------------------------------------------------------------------------------
BYTE CBridgeVideoOutCopEncoder::IsVsw() const
{
  BYTE rc = (m_copLrt == eLogical_COP_VSW_encoder) ? YES : NO;
  TRACEINTO << "CBridgeVideoOutCopEncoder::IsVsw - LogicalResourceType:" << LogicalResourceTypeToString(m_copLrt) << ", IsVsw:"  << (int)rc;
  return rc;
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::ConnectPCMEncoderToPCMMenu(DWORD pcmMenuId)
{
  PASSERT_AND_RETURN(!m_pHardwareInterface);
  PASSERT_AND_RETURN(!m_isPortOpened);

  ((CVideoHardwareInterface*)m_pHardwareInterface)->SendConnectPCMMenu(m_pHardwareInterface->GetConnectionId(), pcmMenuId);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::DisConnectPCMEncoderFromPCMMenu(DWORD pcmMenuId)
{
  PASSERT_AND_RETURN(!m_pHardwareInterface);

  ((CVideoHardwareInterface*)m_pHardwareInterface)->SendDisconnectPCMMenu(m_pHardwareInterface->GetConnectionId(), pcmMenuId);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::UpdateVideoOutParams(CBridgePartyVideoOutParams* pBridgePartyVideoParams)
{
  EStat responseStatus = statOK;
  if (!CPObject::IsValidPObjectPtr(pBridgePartyVideoParams))
  {
    PASSERT(1);
    responseStatus = statIllegal;
  }
  else
  {
    DWORD                     newVideoAlg           = pBridgePartyVideoParams->GetVideoAlgorithm();
    DWORD                     newVideoBitRate       = pBridgePartyVideoParams->GetVideoBitRate();
    eVideoFrameRate           newVideoQCifFrameRate = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionQCIF);
    eVideoFrameRate           newVideoCifFrameRate  = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionCIF);
    eVideoFrameRate           newVideo4CifFrameRate = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolution4CIF);
    eVideoResolution          newVideoResolution    = pBridgePartyVideoParams->GetVideoResolution();
    DWORD                     newMBPS               = pBridgePartyVideoParams->GetMBPS();
    DWORD                     newFS                 = pBridgePartyVideoParams->GetFS();
    DWORD                     newSampleAspectRatio  = pBridgePartyVideoParams->GetSampleAspectRatio();
    DWORD                     newStaticMB           = pBridgePartyVideoParams->GetStaticMB();
    DWORD                     newMaxDPB             = pBridgePartyVideoParams->GetMaxDPB();
    eVideoProfile             profile               = pBridgePartyVideoParams->GetProfile();
    eVideoPacketPayloadFormat packetformat          = pBridgePartyVideoParams->GetPacketFormat();
    eVideoQuality             newVideoQuality       = pBridgePartyVideoParams->GetVideoQualityType();
    eVideoFrameRate           newVideoFrameRate     = pBridgePartyVideoParams->GetVidFrameRate(); // relevant only in cop conferences

    if (m_videoAlg == newVideoAlg && m_videoBitRate == newVideoBitRate &&
        m_videoQcifFrameRate == newVideoQCifFrameRate && m_videoCifFrameRate == newVideoCifFrameRate && m_video4CifFrameRate == newVideo4CifFrameRate &&
        m_videoResolution == newVideoResolution && m_MBPS == newMBPS && m_FS == newFS && m_sampleAspectRatio == newSampleAspectRatio && m_staticMB == newStaticMB &&
        m_videoQuality == newVideoQuality && m_videoFrameRate == newVideoFrameRate && m_maxDPB == newMaxDPB && m_profile == profile && m_packetPayloadFormat == packetformat)
    {
      TRACEINTO << "CBridgeVideoOutCopEncoder::UpdateVideoOutParams - Params does not changed, ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
    }
    else
    {
      if (!m_pHardwareInterface)
      {
        PASSERT(101);
        responseStatus = statIllegal;
      }
      else
      {
        DWORD oldResolutionRatio       = ((CVideoHardwareInterface*)m_pHardwareInterface)->TranslateToVideoResolutionRatio(m_videoAlg, m_videoResolution, m_FS, m_MBPS, m_videoConfType);
        DWORD newResolutionRatio       = ((CVideoHardwareInterface*)m_pHardwareInterface)->TranslateToVideoResolutionRatio(newVideoAlg, newVideoResolution, newFS, newMBPS, m_videoConfType);
        bool  isVideoResolutionChanged = (oldResolutionRatio != newResolutionRatio);

        m_videoAlg        = newVideoAlg;
        m_videoBitRate    = newVideoBitRate;
        m_videoResolution = newVideoResolution;    // In COP the resolution isn't relevant onlt for H263
        m_videoQuality    = newVideoQuality;
        m_videoFrameRate  = newVideoFrameRate;

        if (m_videoAlg == H264)
        {
          m_videoQcifFrameRate  = eVideoFrameRateDUMMY;
          m_videoCifFrameRate   = eVideoFrameRateDUMMY;
          m_video4CifFrameRate  = eVideoFrameRateDUMMY;
          m_MBPS                = newMBPS;
          m_FS                  = newFS;
          m_staticMB            = newStaticMB;
          m_maxDPB              = newMaxDPB;
          m_profile             = profile;
          m_packetPayloadFormat = packetformat;
        }
        else
        {
          m_videoQcifFrameRate = newVideoQCifFrameRate;
          m_videoCifFrameRate  = newVideoCifFrameRate;
          m_video4CifFrameRate = newVideo4CifFrameRate;
          m_MBPS               = INVALID;
          m_FS                 = INVALID;
          m_staticMB           = DEFAULT_STATIC_MB;
          m_maxDPB             = INVALID;
        }

        m_sampleAspectRatio = newSampleAspectRatio;

        DWORD newEncoderRate = m_videoBitRate;
        if (m_flowControlRate && m_flowControlRate < m_videoBitRate)
        {
          newEncoderRate = m_flowControlRate;
          TRACEINTO << "CBridgeVideoOutCopEncoder::UpdateVideoOutParams - Encoder rate updated with flow control rate"
                    << ", ConfName:"       << m_pBridgePartyCntl->GetConfName()
                    << ", EncoderName:"    << m_pBridgePartyCntl->GetName()
                    << ", NewEncoderRate:" << newEncoderRate;
        }

        BOOL oldLPRActive = m_isLprActive;
        m_isLprActive = IsValidPObjectPtr(pBridgePartyVideoParams->GetAckParams()) &&  pBridgePartyVideoParams->GetAckParams()->IsLprOn();
        if (oldLPRActive != m_isLprActive)
        {
          TRACEINTO << "CBridgeVideoOutCopEncoder::UpdateVideoOutParams - Lost Packet status has changed"
                    << ", ConfName:"       << m_pBridgePartyCntl->GetConfName()
                    << ", EncoderName:"    << m_pBridgePartyCntl->GetName()
                    << ", isLprActive:"    << m_isLprActive;
        }

        ((CVideoHardwareInterface*) m_pHardwareInterface)->SendUpdateEncoder(
          m_isLprActive, m_videoAlg, newEncoderRate, m_videoQcifFrameRate, m_videoCifFrameRate, m_video4CifFrameRate,
          m_decoderDetectedModeWidth, m_decoderDetectedModeHeight,
          m_decoderDetectedSampleAspectRatioWidth, m_decoderDetectedSampleAspectRatioHeight,
          m_videoResolution, m_MBPS, m_FS,
          m_sampleAspectRatio, m_staticMB, m_eTelePresenceMode, m_videoQuality, m_isVideoClarityEnabled, m_videoConfType, m_maxDPB,
          (m_pBridgePartyCntl->GetCascadeLinkMode() != NONE), m_videoFrameRate, E_VIDEO_RESOLUTION_TABLE_REGULAR,
          m_croppingHor, m_croppingVer,
          m_profile, m_packetPayloadFormat,
          false, false, false, m_dwFrThreshold,// default values taken from the declaration
          GetFontType(), m_isH263Plus);

        if (isVideoResolutionChanged)
        {
          TRACEINTO << "CBridgeVideoOutCopEncoder::UpdateVideoOutParams - Encoder resolution ratio has changed"
                    << ", ConfName:"       << m_pBridgePartyCntl->GetConfName()
                    << ", EncoderName:"    << m_pBridgePartyCntl->GetName()
                    << ", OldResRatio:"    << oldResolutionRatio
                    << ", NewResRatio:"    << newResolutionRatio;

          m_waitForUpdateEncoderAck = TRUE;
          StartTimer(ENCODER_UPDATE_PARAM_TOUT, 8*SECOND);
        }
      }
    }
  }

  CSegment* pMsg = new CSegment;
  *pMsg << (BYTE) responseStatus;
  m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);
  POBJDELETE(pMsg);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::UpdateFlowControlRate(DWORD flowControlRate)
{
  m_flowControlRate = flowControlRate;
  DispatchEvent(UPDATE_FLOW_CONTROL_RATE, NULL);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnUpdateFlowControlRate(CSegment* pParam)
{
	DWORD newEncoderRate = m_videoBitRate;
	if (m_flowControlRate && m_flowControlRate < m_videoBitRate)
		newEncoderRate = m_flowControlRate;

	TRACEINTO << "CBridgeVideoOutCopEncoder::UpdateFlowControlRate "
		<< "- ConfName:"       << m_pBridgePartyCntl->GetConfName()
		<< ", EncoderName:"    << m_pBridgePartyCntl->GetName()
		<< ", NewEncoderRate:" << newEncoderRate
		<< ", IsVSW:"          << (int)IsVsw();

	if (!IsVsw())
		((CVideoHardwareInterface*) m_pHardwareInterface)->SendUpdateEncoder(
			m_isLprActive, m_videoAlg, newEncoderRate, m_videoQcifFrameRate, m_videoCifFrameRate, m_video4CifFrameRate,
			m_decoderDetectedModeWidth, m_decoderDetectedModeHeight,
			m_decoderDetectedSampleAspectRatioWidth, m_decoderDetectedSampleAspectRatioHeight,
			m_videoResolution, m_MBPS, m_FS, m_sampleAspectRatio,
			m_staticMB, m_eTelePresenceMode, m_videoQuality, m_isVideoClarityEnabled, m_videoConfType, m_maxDPB,
			(m_pBridgePartyCntl->GetCascadeLinkMode() != NONE), m_videoFrameRate, E_VIDEO_RESOLUTION_TABLE_REGULAR,
			m_croppingHor, m_croppingVer,
			m_profile, m_packetPayloadFormat,
			false, false, false, m_dwFrThreshold, // default values taken from the declaration
			GetFontType(), m_isH263Plus);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnUpdateFlowControlRateCONNECTED(CSegment* pParam)
{
  TRACEINTO << "CBridgeVideoOutCopEncoder::OnUpdateFlowControlRateCONNECTED - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
  OnUpdateFlowControlRate(pParam);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnUpdateFlowControlRateCHANGE_LAYOUT(CSegment* pParam)
{
  TRACEINTO << "CBridgeVideoOutCopEncoder::OnUpdateFlowControlRateCHANGE_LAYOUT - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
  OnUpdateFlowControlRate(pParam);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::DSPSmartSwitchChangeLayout(CLayout* layout, CVisualEffectsParams* visualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout, BYTE sendChangeLayoutAnyWay)
{
  TRACEINTO << "CBridgeVideoOutCopEncoder::DSPSmartSwitchChangeLayout "
            << "- ConfName:"       << m_pBridgePartyCntl->GetConfName()
            << ", EncoderName:"    << m_pBridgePartyCntl->GetName()
            << ", SendAnyWay:"     << (int)sendChangeLayoutAnyWay
            << ", pCopConfLayout:" << (DWORD)m_pCopConfLayout;

  PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(layout));

  if ((m_pCopConfLayout == NULL) || (CPObject::IsValidPObjectPtr(m_pCopConfLayout) && *layout != *m_pCopConfLayout) || sendChangeLayoutAnyWay)
    SendDSPSmartSwitchChangeLayoutToHardware(layout, visualEffects, pSiteNameInfo, speakerPlaceInLayout);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::SendDSPSmartSwitchChangeLayoutToHardware(CLayout* layout, CVisualEffectsParams* visualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout)
{
  PTRACE2INT(eLevelInfoNormal, "CBridgeVideoOutCopEncoder::SendDSPSmartSwitchChangeLayoutToHardware : Encoder Index - ", m_copEncoderIndex);
  PASSERT_AND_RETURN(!m_pHardwareInterface);

  ((CVideoHardwareInterface*)m_pHardwareInterface)->SendDSPSmartSwitchChangeLayout(layout, visualEffects, pSiteNameInfo, speakerPlaceInLayout, m_videoResolution, m_decoderDetectedModeWidth, m_decoderDetectedModeHeight, m_decoderDetectedSampleAspectRatioWidth, m_decoderDetectedSampleAspectRatioHeight, m_videoAlg, m_FS, m_MBPS, m_videoConfType, FALSE, m_isSiteNamesEnabled);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnVideoBridgePartyFastUpdateCHANGELAYOUT(CSegment* pParam)
{
  // ICBC - in order to avoid intermediate layouts
  TRACEINTO << "CBridgeVideoOutCopEncoder::OnVideoBridgePartyFastUpdateCHANGELAYOUT - We delay the the fast update. When we will receive ack on the changelayout request we will send the request - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
  m_sendFastUpdateAtChangeLayoutAck = YES;
}
// ------------------------------------------------------------------------------------------
ECopDecoderResolution CBridgeVideoOutCopEncoder::GetCopDecoderResolutionFromVideoParams()
{
  ECopDecoderResolution copDecoderResolution = COP_decoder_resolution_Last;

  switch (m_videoResolution)
  {
    case (eVideoResolutionQCIF):
    case (eVideoResolutionQVGA):
    case (eVideoResolutionCIF):
    case (eVideoResolutionSIF):
    {
      copDecoderResolution = COP_decoder_resolution_CIF25;
      break;
    }

    case (eVideoResolutionVGA):
    case (eVideoResolution4SIF):
    case (eVideoResolution4CIF):
    {
      copDecoderResolution = COP_decoder_resolution_4CIF25;
      break;
    }

    case (eVideoResolution525SD):
    case (eVideoResolution625SD):
    {
      copDecoderResolution = COP_decoder_resolution_W4CIF25;
      break;
    }

    case (eVideoResolutionSVGA):
    case (eVideoResolutionXGA):
    case (eVideoResolutionHD720):
    case (eVideoResolution16CIF):
    {
      if ((m_videoFrameRate == eVideoFrameRate60FPS) || (m_videoFrameRate == eVideoFrameRate50FPS))
        copDecoderResolution = COP_decoder_resolution_HD720p50;
      else
        copDecoderResolution = COP_decoder_resolution_HD720p25;
      break;
    }

    case (eVideoResolutionHD1080):
    {
      copDecoderResolution = COP_decoder_resolution_HD108030;
      break;
    }

	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }
  return copDecoderResolution;
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnVideoBridgePartyEncoderResolutionChangedCONNECTED(CSegment* pParam)
{
  TRACEINTO << "CBridgeVideoOutCopEncoder::OnVideoBridgePartyEncoderResolutionChangedCONNECTED - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
  BuildLayoutAndSendOnEncoderResolutionChange();
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::OnVideoBridgePartyEncoderResolutionChangedCHANGELAYOUT(CSegment* pParam)
{
  TRACEINTO << "CBridgeVideoOutCopEncoder::OnVideoBridgePartyEncoderResolutionChangedCHANGELAYOUT - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
  ON(m_resolutionChangedWhileWaitingForAck);
}
// ------------------------------------------------------------------------------------------
void CBridgeVideoOutCopEncoder::BuildLayoutAndSendOnEncoderResolutionChange()
{
  TRACEINTO << "CBridgeVideoOutCopEncoder::BuildLayoutAndSendOnEncoderResolutionChange - ConfName:" << m_pBridgePartyCntl->GetConfName() << ", EncoderName:" << m_pBridgePartyCntl->GetName();
  DispatchEvent(CHANGECOPLAYOUT, NULL);
}

