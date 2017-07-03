// +========================================================================+
// VideoBridgeCopEncoder.H                                                  |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       VideoBridgeCopEncoder.CPP                                    |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Yoella                                                       |
// -------------------------------------------------------------------------|
// Who  | Date  Aug-2009  | Description                                     |
// -------------------------------------------------------------------------|
// +========================================================================+

#include "VideoBridgeCopEncoder.h"
#include "TextOnScreenMngr.h"
#include "PartyApi.h"
#include "VisualEffectsParams.h"
#include "BridgeVideoOutCopEncoder.h"
#include "BridgePartyVideoOut.h"
#include "StatusesGeneral.h"
#include "VideoBridge.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsVideo.h"
#include "VideoBridgePartyInitParams.h"
#include "VideoApiDefinitions.h"
#include "VideoApiDefinitions.h"
#include "ManagerApi.h" // For LEGACY
#include "IntraSuppression.h"
#include "SiteNameInfo.h"
#include "Macros.h"

// ~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable(void);

#define VIDEO_BRDG_PARTY_SETUP_TOUT                   ((WORD)200)
#define VIDEO_BRDG_PARTY_DISCONNECT_TOUT              ((WORD)201)

#ifndef VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT
  #define VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT    ((WORD)205)
#endif

#define VIDEO_BRDG_PARTY_DISCONNECT_TOUT_VALUE        500 // 5*SECOND (Moti temp - to be adjusted)

PBEGIN_MESSAGE_MAP(CVideoBridgeCopEncoder)

  ONEVENT(VIDCONNECT                              , SETUP         , CVideoBridgeCopEncoder::OnVideoBridgeConnectSETUP)
  ONEVENT(VIDCONNECT                              , CONNECTED     , CVideoBridgeCopEncoder::OnVideoBridgeConnectCONNECTED)
  ONEVENT(VIDCONNECT                              , DISCONNECTING , CVideoBridgeCopEncoder::OnVideoBridgeConnectDISCONNECTING)

  ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, IDLE          , CVideoBridgeCopEncoder::NullActionFunction)
  ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, SETUP         , CVideoBridgeCopEncoder::OnTimerIgnoreIntraSETUP)
  ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, CONNECTED     , CVideoBridgeCopEncoder::OnTimerIgnoreIntraCONNECTED)
  ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, DISCONNECTING , CVideoBridgeCopEncoder::NullActionFunction)

PEND_MESSAGE_MAP(CVideoBridgeCopEncoder, CVideoBridgePartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeCopEncoder
////////////////////////////////////////////////////////////////////////////
CVideoBridgeCopEncoder::CVideoBridgeCopEncoder()
{
  m_isIntraForLinksSuppressed = FALSE;
  m_isIntraRequestReceivedFromWhileSuppressed = FALSE;
}

////////////////////////////////////////////////////////////////////////////
CVideoBridgeCopEncoder::~CVideoBridgeCopEncoder ()
{
  POBJDELETE(m_pConfApi);
}

////////////////////////////////////////////////////////////////////////////
CVideoBridgeCopEncoder& CVideoBridgeCopEncoder::operator=(const CVideoBridgeCopEncoder& rOtherBridgePartyCntl)
{
  if (&rOtherBridgePartyCntl == this)
    return *this;

  (CVideoBridgePartyCntl&)(*this) = (CVideoBridgePartyCntl&)rOtherBridgePartyCntl;

  m_isIntraForLinksSuppressed = rOtherBridgePartyCntl.m_isIntraForLinksSuppressed;
  m_isIntraRequestReceivedFromWhileSuppressed = rOtherBridgePartyCntl.m_isIntraRequestReceivedFromWhileSuppressed;
  return *this;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::Create(const CVideoBridgePartyInitParams* pBridgePartyInitParams)
{
  if (NULL == pBridgePartyInitParams) {
    PASSERT_AND_RETURN(101);
  }

  if (!pBridgePartyInitParams->GetMediaOutParams()) {
    PASSERT_AND_RETURN(102);
  }

  CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
  if (NULL == pRoutingTbl) {
    PASSERT_AND_RETURN(103);
  }

  // Create base params to set a new function
  strncpy(m_name, pBridgePartyInitParams->GetPartyName(), H243_NAME_LEN-1);
  m_name[H243_NAME_LEN -1] = '\0';

  SetFullName(pBridgePartyInitParams->GetPartyName(), pBridgePartyInitParams->GetConfName());

  m_pBridge           = (CBridge*)(pBridgePartyInitParams->GetBridge());
  m_pParty            = (CTaskApp*)(pBridgePartyInitParams->GetParty());
  m_partyRsrcID       = pBridgePartyInitParams->GetPartyRsrcID();
  m_confRsrcID        = pBridgePartyInitParams->GetConfRsrcID();
  m_wNetworkInterface = pBridgePartyInitParams->GetNetworkInterface();
  m_bCascadeLinkMode  = pBridgePartyInitParams->GetCascadeLinkMode();

  if (CPObject::IsValidPObjectPtr(m_pConfApi))
    POBJDELETE(m_pConfApi);

  m_pConfApi  = new CConfApi;
  m_pConfApi->CreateOnlyApi(((CConf*)(m_pBridge->GetConf()))->GetRcvMbx(), this);
  m_pConfApi->SetLocalMbx(((CConf*)(m_pBridge->GetConf()))->GetLocalQueue());

  CRsrcParams* pRsrcParams = new CRsrcParams;
  pRsrcParams->SetConfRsrcId(m_confRsrcID);
  pRsrcParams->SetPartyRsrcId(m_partyRsrcID);

  NewPartyOut();
  CTaskApi* pTaskApiVideoOut = new CTaskApi(*m_pConfApi);
  pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);

  CBridgePartyVideoOutParams* bridgeVideoOutParams = (CBridgePartyVideoOutParams*)pBridgePartyInitParams->GetMediaOutParams();
  eLogicalResourceTypes       copEncoderLRT = bridgeVideoOutParams->GetCopLrt();

  CRsrcDesc* pRsrcDesc = pRoutingTbl->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, copEncoderLRT, pTaskApiVideoOut);

  if (!pRsrcDesc) // Entry not found in Routing Table
  {
    PTRACE2(eLevelError, "CVideoBridgeCopEncoder::Create - Video-Out creation failure : Name - ", GetFullName());
    POBJDELETE(m_pBridgePartyOut);
    PASSERT(105);
  }
  else
  {
    pRsrcParams->SetRsrcDesc(*pRsrcDesc);
    ((CBridgePartyVideoOut*)(m_pBridgePartyOut))->Create(this, pRsrcParams, pBridgePartyInitParams->GetMediaOutParams());
  }

  POBJDELETE(pTaskApiVideoOut);
  POBJDELETE(pRsrcParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::NewPartyOut()
{
  if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
  {
    POBJDELETE(m_pBridgePartyOut);
  }

  m_pBridgePartyOut = new CBridgeVideoOutCopEncoder();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::Destroy()
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::Destroy : Name - ", GetFullName());

  CBridgePartyCntl::Destroy();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::FastUpdate(BYTE isPartyLinkToMgc)
{
  if (isPartyLinkToMgc && IsIntraSuppressEnabled(SUPPRESS_TYPE_ALL) == true)
  {
    if (m_isIntraForLinksSuppressed == TRUE)
    {
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::FastUpdate requesting party is link to MGC and intra for links is supressed!! ", m_partyConfName);
      m_isIntraRequestReceivedFromWhileSuppressed = TRUE; // this flag indicates that when the timer will expire we will send new intra and activate the timer again
      return;
    }
    else
    {
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::FastUpdate requesting party is link to MGC, intra requests from links will be ignored for next 10 seconds ", m_partyConfName);
      m_isIntraForLinksSuppressed = TRUE;

      DWORD       dwIgnoreIntraDuration = 10; // default
      CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
      sysConfig->GetDWORDDataByKey("COP_ENCODER_IGNORE_INTRA_DURATION_IN_SECONDS", dwIgnoreIntraDuration);
      if (dwIgnoreIntraDuration > 0)
        StartTimer(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, dwIgnoreIntraDuration*SECOND);
    }
  }

  DispatchEvent(FASTUPDATE, NULL);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnTimerIgnoreIntraSETUP(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::OnTimerIgnoreIntraSETUP ", m_partyConfName);
  OnTimerIgnoreIntra();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnTimerIgnoreIntraCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::OnTimerIgnoreIntraCONNECTED ", m_partyConfName);
  OnTimerIgnoreIntra();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnTimerIgnoreIntra()
{
  m_isIntraForLinksSuppressed = FALSE;
  // will send intra and activate the filter mechanism again
  if (m_isIntraRequestReceivedFromWhileSuppressed == TRUE)
  {
    m_isIntraRequestReceivedFromWhileSuppressed = FALSE;
    FastUpdate(TRUE);
  }
  else  // just send intra
  {
    FastUpdate(FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::ChangeLayout(CLayout* layout, CVisualEffectsParams* visualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout, BYTE sendChangeLayoutAnyWay)
{
  if (m_pBridgePartyOut)
    ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->ChangeLayout(layout, visualEffects, pSiteNameInfo, speakerPlaceInLayout, sendChangeLayoutAnyWay);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::ChangeLayoutAttributes(CVisualEffectsParams* visualEffects, DWORD speakerPlaceInLayout)
{
  if (m_pBridgePartyOut)
    ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->ChangeLayoutAttributes(visualEffects, speakerPlaceInLayout);
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCopEncoder::IsValidState(WORD state) const
{
  WORD valid_state = TRUE;
  switch (state)
  {
    case CONNECTED_STANDALONE:
    case EXPORT:
    {
      valid_state = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::IsValidState NOT VALID STATE!!!", m_partyConfName);
      PASSERT(1);
      break;
    }
  } // switch

  return valid_state;
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCopEncoder::IsValidEvent(OPCODE event) const
{
  WORD valid_event = TRUE;
  switch (event)
  {
    case VIDEO_IN_CONNECTED:
    case VIDEO_IN_SYNCED:
    case VIDEO_IN_DISCONNECTED:
    case UPDATE_VIDEO_IN_PARAMS:
    case PARTY_VIDEO_IN_UPDATED:
    case ADDIMAGE:
    case UPDATE_VIDEO_OUT_PARAMS:  // the cop encoder params are supposed to be static
    case MUTEIMAGE:
    case UNMUTEIMAGE:
    case SPEAKERS_CHANGED:
    case AUDIO_SPEAKER_CHANGED:
    case DELIMAGE:
    case CHANGECONFLAYOUT:
    case END_CHANGE_LAYOUT:
    case CHANGEPARTYLAYOUT:
    case CHANGEPARTYPRIVATELAYOUT:
    case SETPARTYPRIVATELAYOUTONOFF:
    case IVR_SHOW_SLIDE_REQ:
    case IVR_STOP_SHOW_SLIDE_REQ:
    case IVR_JOIN_CONF_VIDEO:
    case VIDEO_GRAPHIC_OVERLAY_START_REQ:
    case VIDEO_GRAPHIC_OVERLAY_STOP_REQ:
    case PLC_SETPARTYPRIVATELAYOUTTYPE:
    case PLC_RETURNPARTYTOCONFLAYOUT:
    case PLC_FORCECELLZERO:
    case PLC_CANCELALLPRIVATELAYOUTFORCES:
    case VIDEO_DECODER_SYNC:
    case PRIVATELAYOUT_ONOFF_CHANGED:
    case UPDATE_DECODER_DETECTED_MODE:
    {
      valid_event = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::IsValidEvent NOT VALID EVENTS!!!", m_partyConfName);
      PASSERT(1);
      break;
    }
  } // switch

  return valid_event;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnVideoBridgeConnectIDLE(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::OnVideoBridgeConnectIDLE: ", m_partyConfName);
  if (!m_pBridgePartyOut)
  {
    DWORD     copEncoderIndex = GetCopEncoderIndex();
    CSegment* pSeg = new CSegment;
    *pSeg << (DWORD)copEncoderIndex;
    m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_CONNECT_COP_ENCODER, statInconsistent, FALSE, eNoDirection, pSeg);
    POBJDELETE(pSeg);
    return;
  }

  Setup();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnVideoBridgeConnectSETUP(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::OnVideoBridgeConnectSETUP we ignore the connect event in SETUP state!: ", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnVideoBridgeConnectCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::OnVideoBridgeConnectCONNECTED the encoder is all ready connected!: ", m_partyConfName);
  DWORD     copEncoderIndex = GetCopEncoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copEncoderIndex;
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_CONNECT_COP_ENCODER, statOK, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnVideoBridgeConnectDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::OnVideoBridgeConnectDISCONNECTING we ignore the connect event in CONNECTED state!: ", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnVideoOutConnectedCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::OnVideoOutConnectedCONNECTED we ignore the VIDEO_OUT_CONNECTED event in CONNECTED state!: ", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::VideoConnectionCompletion:", m_partyConfName);
  if (!m_pBridgePartyOut) {
    DBGPASSERT_AND_RETURN(1);
  }

  BOOL  isVideoConnectionCompleted  = FALSE;
  EStat receivedStatus        = statOK;

  *pParams >> (BYTE&)receivedStatus;

  DWORD     copEncoderIndex = GetCopEncoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copEncoderIndex;

  DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);
  if (statOK != receivedStatus)
  {
    // Inform Video Bridge about connection failure
    DumpMcuInternalProblemDetailed((BYTE)eConnectedMediaDirection, eMipStatusFail, eMipVideo);
    *pSeg << *pParams;
  }

  if (statOK == receivedStatus)
  {
    m_state = CONNECTED;
    CMedString logStr;
    logStr << "CVideoBridgeCopEncoder::VideoConnectionCompletion: Connected direction : "<< eNoDirection;
    PTRACE(eLevelInfoNormal, logStr.GetString());
  }

  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_CONNECT_COP_ENCODER, receivedStatus, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::IllegalCopEncoderState(CSegment* pParam)
{
  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCopEncoder::IllegalCopEncoderState: Not valid!!!", m_state);
  PASSERT(100);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnVideoBridgeDisconnect(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::OnVideoBridgeDisconnect: ", m_partyConfName);

  StartTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT, VIDEO_BRDG_PARTY_DISCONNECT_TOUT_VALUE);

  // Disconnect Video Out
  if (m_pBridgePartyOut)
    m_pBridgePartyOut->DisConnect();

  if (m_state == SETUP)
    DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);

  m_state = DISCONNECTING;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::VideoDisConnectionCompletion: ", m_partyConfName);
  if (!m_pBridgePartyOut || !(eMediaOut == eDisConnectedMediaDirection)) {
    DBGPASSERT_AND_RETURN(1);
  }

  EStat receivedStatus = statOK;
  BYTE  videoOutClosePortStatus = statOK;


  *pParams >> (BYTE&)receivedStatus;

  DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);
  m_state = IDLE;

  SetDisConnectingDirectionsReq(eNoDirection);

  videoOutClosePortStatus = m_pBridgePartyOut->GetClosePortAckStatus();

  // for debug info in case of the "MCU internal problem"
  BYTE failureCauseDirection =   eMipNoneDirction;

  if (videoOutClosePortStatus != STATUS_OK)
  {
    receivedStatus = (EStat)videoOutClosePortStatus;
    failureCauseDirection = eMipOut;
  }

  DWORD     copEncoderIndex = GetCopEncoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copEncoderIndex;

  // Inform Video Bridge In case of problem
  if (statVideoInOutResourceProblem == receivedStatus)
  {
    *pSeg << (BYTE)receivedStatus << (BYTE)failureCauseDirection << (BYTE)eMipStatusFail <<(BYTE) eMipClose;
    DumpMcuInternalProblemDetailed((BYTE)failureCauseDirection, eMipStatusFail, eMipVideo);
  }

  else
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::VideoDisConnectionCompletion - Encoder was disconnected - state is IDLE : Name - ", m_partyConfName);
    PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCopEncoder::VideoDisConnectionCompletion: copEncoderIndex = ", (DWORD)copEncoderIndex);
  }

  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DISCONNECT_COP_ENCODER, receivedStatus, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCopEncoder::GetCopEncoderIndex() const
{
  WORD encoderIndex = (0xFFFF);
  if (m_pBridgePartyOut)
    encoderIndex = ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->GetCopEncoderIndex();

  return encoderIndex;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnTimerPartySetupCONNECTED(CSegment* pParams)
{
  PTRACE2(eLevelError, "CVideoBridgeCopEncoder::OnTimerPartySetupCONNECTED we ignore the VIDEO_OUT_CONNECTED event in CONNECTED state!: ", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnTimerPartySetup(CSegment* pParams)
{
  DWORD     copEncoderIndex = GetCopEncoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copEncoderIndex;

  CMedString encoderString, decoderString;
  CMedString logStr;
  logStr << "CVideoBridgeCopEncoder::OnTimerPartySetup : Name - " << m_partyConfName;

  if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
  {
    logStr <<" CBridgePartyVideoOut State: ";
    ((CBridgePartyVideoOut*)m_pBridgePartyOut)->DumpAllInfoOnConnectionState(&logStr);
    ((CBridgePartyVideoOut*)m_pBridgePartyOut)->DumpAllInfoOnConnectionState(&encoderString, true);
  }

  logStr <<"\nm_partyRsrcID = " << m_partyRsrcID << " , m_confRsrcID = " << m_confRsrcID;
  PTRACE(eLevelError, logStr.GetString());

  // Add Fault to EMA
  std::string faultString = "Video Encoder:";
  faultString += encoderString.GetString();
  CBridgePartyMediaUniDirection::AddFaultAlarm(faultString.c_str(), m_partyRsrcID, STATUS_OK, true);

  // for debug info in case of the "MCU internal problem"
  BYTE failureCauseDirection =   eMipNoneDirction;
  BYTE failureCauseAction    =   eMipNoAction;
  GetRsrcProbAdditionalInfoOnVideoTimerSetup(failureCauseDirection, failureCauseAction);

  *pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer <<(BYTE) eMipOpen;
  DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);

  // Inform Video Bridge of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder+Decoder
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_CONNECT_COP_ENCODER, statVideoInOutResourceProblem, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnTimerPartyDisconnectSETUP(CSegment* pParams)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::OnTimerPartyDisconnectSETUP ignored!", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnTimerPartyDisconnectCONNECTED(CSegment* pParams)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::OnTimerPartyDisconnectCONNECTED ignored!", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams)
{
  DWORD     copEncoderIndex = GetCopEncoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copEncoderIndex;


  CMedString logStr;
  logStr << "CVideoBridgeCopEncoder::OnTimerPartyDisconnectDISCONNECTING: Name - " << m_partyConfName;
  logStr << "\nm_partyRsrcID = " << m_partyRsrcID << " , m_confRsrcID = " << m_confRsrcID;
  PTRACE(eLevelError, logStr.GetString());

  // Add Fault to EMA
  CBridgePartyMediaUniDirection::AddFaultAlarm("Did not receive all acks in Video disconnection", m_partyRsrcID, STATUS_OK, true);

  m_state = IDLE;
  // for debug info in case of the "MCU internal problem"
  BYTE failureCauseDirection =   eMipIn; // eMipNoneDirction;
  *pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer <<(BYTE) eMipClose;

  if (GetConnectionFailureCause() == statOK)
  {
    DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);
  }

  // Inform Video Bridge of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder+Decoder
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DISCONNECT_COP_ENCODER, statVideoInOutResourceProblem, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
BOOL CVideoBridgeCopEncoder::IsCopEncoderInConnectedState()
{
  if (m_state == CONNECTED)
    return TRUE;
  else
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::ConnectPCMEncoderToPCMMenu(DWORD pcmMenuId)
{
  PASSERTMSG_AND_RETURN(!m_pBridgePartyOut, "Invalid 'm_pBridgePartyOut'");
  ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->ConnectPCMEncoderToPCMMenu(pcmMenuId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::DisConnectPCMEncoderFromPCMMenu(DWORD pcmMenuId)
{
  PASSERTMSG_AND_RETURN(!m_pBridgePartyOut, "Invalid 'm_pBridgePartyOut'");
  ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->DisConnectPCMEncoderFromPCMMenu(pcmMenuId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::UpdateVideoOutParams(CBridgePartyVideoOutParams* pOutVideoParams)
{
  PASSERTMSG_AND_RETURN(!m_pBridgePartyOut, "Invalid 'm_pBridgePartyOut'");
  ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->UpdateVideoOutParams(pOutVideoParams);
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCopEncoder::IsClosed() const
{
  BYTE isClosed = NO;
  if (m_state == IDLE)
  {
    isClosed = YES;
  }

  return isClosed;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::SetEncoderFlowControlRate(DWORD rate)
{
  if (m_pBridgePartyOut)
    ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->SetFlowControlRate(rate);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::UpdateEncoderFlowControlRate(DWORD rate)
{
  if (m_pBridgePartyOut)
    ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->UpdateFlowControlRate(rate);
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoBridgeCopEncoder::GetEncoderFlowControlRate() const
{
  DWORD fcRate = 0;
  if (m_pBridgePartyOut)
    fcRate = ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->GetEncoderFlowControlRate();

  return fcRate;
}

////////////////////////////////////////////////////////////////////////////
CLayout* CVideoBridgeCopEncoder::GetCopEncoderLayout()
{
  CLayout* pLayout = NULL;
  if (m_pBridgePartyOut)
    pLayout = ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->GetCopEncoderLayout();

  return pLayout;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::DSPSmartSwitchChangeLayout(CLayout* layout, CVisualEffectsParams* visualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout, BYTE sendChangeLayoutAnyWay)
{
  if (m_pBridgePartyOut)
    ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->DSPSmartSwitchChangeLayout(layout, visualEffects, pSiteNameInfo, speakerPlaceInLayout, sendChangeLayoutAnyWay);
}

////////////////////////////////////////////////////////////////////////////
ECopDecoderResolution CVideoBridgeCopEncoder::GetCopDecoderResolutionFromVideoParams()
{
  ECopDecoderResolution copDecoderRes = COP_decoder_resolution_Last;
  if (m_pBridgePartyOut)
    copDecoderRes = ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->GetCopDecoderResolutionFromVideoParams();

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCopEncoder::GetCopDecoderResolutionFromVideoParams ", copDecoderRes);

  return copDecoderRes;
}

////////////////////////////////////////////////////////////////////////////
eVideoFrameRate CVideoBridgeCopEncoder::GetVidFrameRate()
{
  eVideoFrameRate videoFrameRate = eVideoFrameRateDUMMY;
  if (m_pBridgePartyOut)
    videoFrameRate = ((CBridgePartyVideoOut*)m_pBridgePartyOut)->GetVidFrameRate();

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCopEncoder::GetVidFrameRate ", videoFrameRate);

  return videoFrameRate;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::OnVideoOutUpdated(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopEncoder::OnVideoOutUpdated the encoder is all ready connected!: ", m_partyConfName);
  DWORD     copEncoderIndex = GetCopEncoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copEncoderIndex;

  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, COP_ENCODER_VIDEO_OUT_UPDATED, statOK, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopEncoder::SetSiteNameConfiguration(CSiteNameInfo* pSiteNameInfo)
{
  if (m_pBridgePartyOut)
    ((CBridgeVideoOutCopEncoder*)m_pBridgePartyOut)->SetSiteNameInfo(pSiteNameInfo);
}
///////////////////////////////////////////////////////////////////////////
///////////  class CVideoBridgeXCodeEncoder
///////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CVideoBridgeXCodeEncoder)

  /*ONEVENT(VIDCONNECT                              , SETUP         , CVideoBridgeCopEncoder::OnVideoBridgeConnectSETUP)
  ONEVENT(VIDCONNECT                              , CONNECTED     , CVideoBridgeCopEncoder::OnVideoBridgeConnectCONNECTED)
  ONEVENT(VIDCONNECT                              , DISCONNECTING , CVideoBridgeCopEncoder::OnVideoBridgeConnectDISCONNECTING)

  ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, IDLE          , CVideoBridgeCopEncoder::NullActionFunction)
  ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, SETUP         , CVideoBridgeCopEncoder::OnTimerIgnoreIntraSETUP)
  ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, CONNECTED     , CVideoBridgeCopEncoder::OnTimerIgnoreIntraCONNECTED)
  ONEVENT(VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT, DISCONNECTING , CVideoBridgeCopEncoder::NullActionFunction)*/

PEND_MESSAGE_MAP(CVideoBridgeXCodeEncoder, CVideoBridgeCopEncoder);

////////////////////////////////////////////////////////////////////////////
CVideoBridgeXCodeEncoder::CVideoBridgeXCodeEncoder()
{
	m_isFaulty = FALSE;
}
////////////////////////////////////////////////////////////////////////////
CVideoBridgeXCodeEncoder::~CVideoBridgeXCodeEncoder()
{

}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::Create:", m_partyConfName);
	CVideoBridgeCopEncoder::Create(pVideoBridgePartyInitParams);
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::NewPartyOut()
{
	CVideoBridgeCopEncoder::NewPartyOut();
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::FastUpdate(BYTE isPartyLinkToMgc)
{
	DispatchEvent(FASTUPDATE, NULL);
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::VideoConnectionCompletion:", m_partyConfName);
	if (!m_pBridgePartyOut) {
		DBGPASSERT_AND_RETURN(1);
	}

	BOOL  isVideoConnectionCompleted  = FALSE;
	EStat receivedStatus        = statOK;

	*pParams >> (BYTE&)receivedStatus;

	DWORD     xcodeEncoderIndex = GetCopEncoderIndex();
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)xcodeEncoderIndex;

	DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);
	if (statOK != receivedStatus)
	{
		// Inform Video Bridge about connection failure
		DumpMcuInternalProblemDetailed((BYTE)eConnectedMediaDirection, eMipStatusFail, eMipVideo);
		*pSeg << *pParams;
	}

	if (statOK == receivedStatus)
	{
		m_state = CONNECTED;
		CMedString logStr;
		logStr << "CCVideoBridgeXCodeEncoder::VideoConnectionCompletion: Connected direction : "<< eNoDirection;
		PTRACE(eLevelInfoNormal, logStr.GetString());
	}
	m_pConfApi->PartyBridgeResponseMsg(m_pParty, XCODE_BRDG_MSG, END_CONNECT_COP_ENCODER, receivedStatus, FALSE, eNoDirection, pSeg);


	POBJDELETE(pSeg);
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::VideoDisConnectionCompletion: ", m_partyConfName);
	if (!m_pBridgePartyOut || !(eMediaOut == eDisConnectedMediaDirection)) {
		DBGPASSERT_AND_RETURN(1);
	}

	EStat receivedStatus = statOK;
	BYTE  videoOutClosePortStatus = statOK;


	*pParams >> (BYTE&)receivedStatus;

	DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);
	m_state = IDLE;

	SetDisConnectingDirectionsReq(eNoDirection);

	videoOutClosePortStatus = m_pBridgePartyOut->GetClosePortAckStatus();

	// for debug info in case of the "MCU internal problem"
	BYTE failureCauseDirection =   eMipNoneDirction;

	if (videoOutClosePortStatus != STATUS_OK)
	{
		receivedStatus = (EStat)videoOutClosePortStatus;
		failureCauseDirection = eMipOut;
		m_isFaulty = TRUE;
	}

	DWORD     xcodeEncoderIndex = GetCopEncoderIndex();
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)xcodeEncoderIndex;

	// Inform Video Bridge In case of problem
	if (statVideoInOutResourceProblem == receivedStatus)
	{
		*pSeg << (BYTE)receivedStatus << (BYTE)failureCauseDirection << (BYTE)eMipStatusFail <<(BYTE) eMipClose;
		DumpMcuInternalProblemDetailed((BYTE)failureCauseDirection, eMipStatusFail, eMipVideo);
	}

	else
	{
		PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::VideoDisConnectionCompletion - Encoder was disconnected - state is IDLE : Name - ", m_partyConfName);
		PTRACE2INT(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::VideoDisConnectionCompletion: copEncoderIndex = ", (DWORD)xcodeEncoderIndex);
	}
	m_pConfApi->PartyBridgeResponseMsg(m_pParty, XCODE_BRDG_MSG, END_DISCONNECT_COP_ENCODER, receivedStatus, FALSE, eNoDirection, pSeg);

	POBJDELETE(pSeg);
}
////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeXCodeEncoder::IsValidState(WORD state) const
{
	WORD valid_state = TRUE;
	switch (state)
	{
	case CONNECTED_STANDALONE:
	case EXPORT:
	{
		valid_state = FALSE;
		PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::IsValidState NOT VALID STATE!!!", m_partyConfName);
		PASSERT(1);
		break;
	}
	} // switch

	return valid_state;
}
////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeXCodeEncoder::IsValidEvent(OPCODE event) const
{
	  WORD valid_event = TRUE;
	  switch (event)
	  {
	    case VIDEO_IN_CONNECTED:
	    case VIDEO_IN_SYNCED:
	    case VIDEO_IN_DISCONNECTED:
	    case UPDATE_VIDEO_IN_PARAMS:
	    case PARTY_VIDEO_IN_UPDATED:
	    case ADDIMAGE:
	    case MUTEIMAGE:
	    case UNMUTEIMAGE:
	    case SPEAKERS_CHANGED:
	    case AUDIO_SPEAKER_CHANGED:
	    case DELIMAGE:
	    case CHANGECONFLAYOUT:
	    case END_CHANGE_LAYOUT:
	    case CHANGEPARTYLAYOUT:
	    case CHANGEPARTYPRIVATELAYOUT:
	    case SETPARTYPRIVATELAYOUTONOFF:
	    case IVR_SHOW_SLIDE_REQ:
	    case IVR_STOP_SHOW_SLIDE_REQ:
	    case IVR_JOIN_CONF_VIDEO:
	    case VIDEO_GRAPHIC_OVERLAY_START_REQ:
	    case VIDEO_GRAPHIC_OVERLAY_STOP_REQ:
	    case PLC_SETPARTYPRIVATELAYOUTTYPE:
	    case PLC_RETURNPARTYTOCONFLAYOUT:
	    case PLC_FORCECELLZERO:
	    case PLC_CANCELALLPRIVATELAYOUTFORCES:
	    case VIDEO_DECODER_SYNC:
	    case PRIVATELAYOUT_ONOFF_CHANGED:
	    case UPDATE_DECODER_DETECTED_MODE:
	    {
	      valid_event = FALSE;
	      PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::IsValidEvent NOT VALID EVENTS!!!", m_partyConfName);
	      PASSERT(event);
	      break;
	    }
	  } // switch

	  return valid_event;
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::UpdateVideoOutParams(CBridgePartyVideoOutParams* pOutVideoParams)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::UpdateVideoOutParams: ", m_partyConfName);
	CVideoBridgeCopEncoder::UpdateVideoOutParams(pOutVideoParams);
}
////////////////////////////////////////////////////////////////////////////
///// Action Functions
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::OnVideoBridgeConnectIDLE(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::OnVideoBridgeConnectIDLE: ", m_partyConfName);
	if (!m_pBridgePartyOut)
	{
		DWORD     copEncoderIndex = GetCopEncoderIndex();
		CSegment* pSeg = new CSegment;
		m_pConfApi->PartyBridgeResponseMsg(m_pParty, XCODE_BRDG_MSG, END_CONNECT_COP_ENCODER, statInconsistent, FALSE, eNoDirection, pSeg);

		POBJDELETE(pSeg);
		return;
	}

	Setup();
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::OnVideoBridgeConnectSETUP(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::OnVideoBridgeConnectSETUP we ignore the connect event in SETUP state!: ", m_partyConfName);
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::OnVideoBridgeConnectCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::OnVideoBridgeConnectCONNECTED the encoder is all ready connected!: ", m_partyConfName);
	DWORD     xcodeEncoderIndex = GetCopEncoderIndex();
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)xcodeEncoderIndex;

	m_pConfApi->PartyBridgeResponseMsg(m_pParty, XCODE_BRDG_MSG, END_CONNECT_COP_ENCODER, statOK, FALSE, eNoDirection, pSeg);
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::OnVideoBridgeConnectDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::OnVideoBridgeConnectDISCONNECTING we ignore the connect event in CONNECTED state!: ", m_partyConfName);
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::OnVideoOutUpdated(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoBridgeXCodeEncoder::OnVideoOutUpdated, Name: ", m_partyConfName);
	DWORD     xcodeEncoderIndex = GetCopEncoderIndex();
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)xcodeEncoderIndex;

	m_pConfApi->PartyBridgeResponseMsg(m_pParty, XCODE_BRDG_MSG, COP_ENCODER_VIDEO_OUT_UPDATED, statOK, FALSE, eNoDirection, pSeg);
	POBJDELETE(pSeg);
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::OnTimerPartySetup(CSegment* pParams)
{
	DWORD     xcodeEncoderIndex = GetCopEncoderIndex();
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)xcodeEncoderIndex;

	CMedString encoderString, decoderString;
	CMedString logStr;
	logStr << "CVideoBridgeXCodeEncoder::OnTimerPartySetup : Name - " << m_partyConfName;

	if (CPObject::IsValidPObjectPtr(m_pBridgePartyOut))
	{
		logStr <<" CBridgePartyVideoOut State: ";
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->DumpAllInfoOnConnectionState(&logStr);
		((CBridgePartyVideoOut*)m_pBridgePartyOut)->DumpAllInfoOnConnectionState(&encoderString, true);
	}

	logStr <<"\nm_partyRsrcID = " << m_partyRsrcID << " , m_confRsrcID = " << m_confRsrcID;
	PTRACE(eLevelError, logStr.GetString());

	// Add Fault to EMA
	std::string faultString = "Video Encoder:";
	faultString += encoderString.GetString();
	CBridgePartyMediaUniDirection::AddFaultAlarm(faultString.c_str(), m_partyRsrcID, STATUS_OK, true);

	// for debug info in case of the "MCU internal problem"
	BYTE failureCauseDirection =   eMipNoneDirction;
	BYTE failureCauseAction    =   eMipNoAction;
	GetRsrcProbAdditionalInfoOnVideoTimerSetup(failureCauseDirection, failureCauseAction);

	*pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer <<(BYTE) eMipOpen;
	DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);

	m_pConfApi->PartyBridgeResponseMsg(m_pParty, XCODE_BRDG_MSG, END_CONNECT_COP_ENCODER, statVideoInOutResourceProblem, FALSE, eNoDirection, pSeg);

	POBJDELETE(pSeg);
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams)
{
	DWORD     xcodeEncoderIndex = GetCopEncoderIndex();
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)xcodeEncoderIndex;


	CMedString logStr;
	logStr << "CVideoBridgeXCodeEncoder::OnTimerPartyDisconnectDISCONNECTING: Name - " << m_partyConfName;
	logStr << "\nm_partyRsrcID = " << m_partyRsrcID << " , m_confRsrcID = " << m_confRsrcID;
	PTRACE(eLevelError, logStr.GetString());
	m_isFaulty = TRUE;
	// Add Fault to EMA
	CBridgePartyMediaUniDirection::AddFaultAlarm("Did not receive all acks in Video disconnection", m_partyRsrcID, STATUS_OK, true);

	m_state = IDLE;
	// for debug info in case of the "MCU internal problem"
	BYTE failureCauseDirection =   eMipIn; // eMipNoneDirction;
	*pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)failureCauseDirection << (BYTE)eMipTimer <<(BYTE) eMipClose;

	if (GetConnectionFailureCause() == statOK)
	{
		DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);
	}

	// Inform XCode Bridge of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder+Decoder

	m_pConfApi->PartyBridgeResponseMsg(m_pParty, XCODE_BRDG_MSG, END_DISCONNECT_COP_ENCODER, statVideoInOutResourceProblem, FALSE, eNoDirection, pSeg);

	POBJDELETE(pSeg);
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeXCodeEncoder::OnTimerIgnoreIntra()
{

}
////////////////////////////////////////////////////////////////////////////

