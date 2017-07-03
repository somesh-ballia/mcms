// +========================================================================+
// VideoBridgeCopEncoder.CPP                                                |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       VideoBridgeCopDecoder.CPP                                    |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Keren                                                        |
// -------------------------------------------------------------------------|
// Who  | Date  Aug-2009  | Description                                     |
// -------------------------------------------------------------------------|
// +========================================================================+

#include "VideoBridgeCopDecoder.h"
#include "TextOnScreenMngr.h"
#include "PartyApi.h"
#include "VisualEffectsParams.h"
#include "BridgeVideoInCopDecoder.h"
#include "BridgePartyVideoIn.h"
#include "StatusesGeneral.h"
#include "VideoBridge.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsVideo.h"
#include "VideoBridgePartyInitParams.h"
#include "VideoApiDefinitions.h"
#include "COP_ConfParty_Defs.h"
#include "Macros.h"

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable(void);

// Timers opcodes
#define VIDEO_BRDG_PARTY_SETUP_TOUT       ((WORD)200)
#define VIDEO_BRDG_PARTY_DISCONNECT_TOUT  ((WORD)201)
#define VIDEO_BRDG_PARTY_CLOSE_TOUT       ((WORD)205)

// Time-out values
#define VIDEO_BRDG_PARTY_SETUP_TOUT_VALUE 500 // 5*SECOND (Moti temp - to be adjusted)
#define VIDEO_BRDG_PARTY_CLOSE_TOUT_VALUE 200

PBEGIN_MESSAGE_MAP(CVideoBridgeCopDecoder)

  ONEVENT(VIDCONNECT                  , CONNECTING    , CVideoBridgeCopDecoder::OnVideoBridgeConnectCONNECTING)
  ONEVENT(VIDCONNECT                  , OPENED        , CVideoBridgeCopDecoder::OnVideoBridgeConnectOPENED)
  ONEVENT(VIDCONNECT                  , CLOSING       , CVideoBridgeCopDecoder::OnVideoBridgeConnectCLOSING)

  ONEVENT(VIDDISCONNECT               , CONNECTING    , CVideoBridgeCopDecoder::OnVideoBridgeDisconnectCONNECTING)
  ONEVENT(VIDDISCONNECT               , OPENED        , CVideoBridgeCopDecoder::OnVideoBridgeDisconnectOPENED)
  ONEVENT(VIDDISCONNECT               , CLOSING       , CVideoBridgeCopDecoder::OnVideoBridgeDisconnectCLOSING)

  ONEVENT(VIDEO_IN_CONNECTED          , CONNECTING    , CVideoBridgeCopDecoder::OnVideoInConnectedCONNECTING)
  ONEVENT(VIDEO_IN_CONNECTED          , OPENED        , CVideoBridgeCopDecoder::OnVideoInConnectedOPENED)
  ONEVENT(VIDEO_IN_CONNECTED          , CLOSING       , CVideoBridgeCopDecoder::OnVideoInConnectedCLOSING)

  ONEVENT(VIDCLOSE                    , IDLE          , CVideoBridgeCopDecoder::OnVideoBridgeCloseIDLE)
  ONEVENT(VIDCLOSE                    , SETUP         , CVideoBridgeCopDecoder::OnVideoBridgeCloseSETUP)
  ONEVENT(VIDCLOSE                    , CONNECTED     , CVideoBridgeCopDecoder::OnVideoBridgeCloseCONNECTED)
  ONEVENT(VIDCLOSE                    , DISCONNECTING , CVideoBridgeCopDecoder::OnVideoBridgeCloseDISCONNECTING)
  ONEVENT(VIDCLOSE                    , CLOSING       , CVideoBridgeCopDecoder::OnVideoBridgeCloseCLOSING)
  ONEVENT(VIDCLOSE                    , CONNECTING    , CVideoBridgeCopDecoder::OnVideoBridgeCloseCONNECTING)
  ONEVENT(VIDCLOSE                    , OPENED        , CVideoBridgeCopDecoder::OnVideoBridgeCloseOPENED)

  ONEVENT(VIDEO_IN_CLOSED             , SETUP         , CVideoBridgeCopDecoder::OnVideoInClosedSETUP)
  ONEVENT(VIDEO_IN_CLOSED             , CONNECTED     , CVideoBridgeCopDecoder::OnVideoInClosedCONNECTED)
  ONEVENT(VIDEO_IN_CLOSED             , DISCONNECTING , CVideoBridgeCopDecoder::OnVideoInClosedDISCONNECTING)
  ONEVENT(VIDEO_IN_CLOSED             , CLOSING       , CVideoBridgeCopDecoder::OnVideoInClosedCLOSING)
  ONEVENT(VIDEO_IN_CLOSED             , CONNECTING    , CVideoBridgeCopDecoder::OnVideoInClosedCONNECTING)
  ONEVENT(VIDEO_IN_CLOSED             , OPENED        , CVideoBridgeCopDecoder::OnVideoInClosedOPENED)

  ONEVENT(UPDATE_VIDEO_IN_PARAMS      , CONNECTING    , CVideoBridgeCopDecoder::OnVideoBridgeUpdateVideoInParamsCONNECTING)
  ONEVENT(UPDATE_VIDEO_IN_PARAMS      , OPENED        , CVideoBridgeCopDecoder::OnVideoBridgeUpdateVideoInParamsOPENED)
  ONEVENT(UPDATE_VIDEO_IN_PARAMS      , CLOSING       , CVideoBridgeCopDecoder::OnVideoBridgeUpdateVideoInParamsCLOSING)

  ONEVENT(PARTY_VIDEO_IN_UPDATED      , CONNECTING    , CVideoBridgeCopDecoder::OnVideoInUpdated)
  ONEVENT(PARTY_VIDEO_IN_UPDATED      , OPENED        , CVideoBridgeCopDecoder::OnVideoInUpdated)
  ONEVENT(PARTY_VIDEO_IN_UPDATED      , CLOSING       , CVideoBridgeCopDecoder::OnVideoInUpdated)

  // timers
  ONEVENT(VIDEO_BRDG_PARTY_CLOSE_TOUT , CLOSING       , CVideoBridgeCopDecoder::OnTimerPartyClosingCLOSING)

  ONEVENT(VIDEO_IN_DISCONNECTED       , OPENED        , CVideoBridgeCopDecoder::OnVideoInDisconnectedOPENED)
  ONEVENT(VIDEO_IN_DISCONNECTED       , CLOSING       , CVideoBridgeCopDecoder::OnVideoInDisconnectedCLOSING)

PEND_MESSAGE_MAP(CVideoBridgeCopDecoder, CVideoBridgePartyCntl);


////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeCopDecoder
////////////////////////////////////////////////////////////////////////////
CVideoBridgeCopDecoder::CVideoBridgeCopDecoder ()
{
}

////////////////////////////////////////////////////////////////////////////
CVideoBridgeCopDecoder::~CVideoBridgeCopDecoder()
{
}

////////////////////////////////////////////////////////////////////////////
const char* CVideoBridgeCopDecoder::NameOf() const
{
  return "CVideoBridgeCopDecoder";
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::Create(const CVideoBridgePartyInitParams* pBridgePartyInitParams)
{
  if (NULL == pBridgePartyInitParams) {
    PASSERT_AND_RETURN(101);
  }

  if (!pBridgePartyInitParams->GetMediaInParams()) {
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

  if (m_pConfApi)
  {
    m_pConfApi->DestroyOnlyApi();
    POBJDELETE(m_pConfApi);
  }

  m_pConfApi  = new CConfApi;
  m_pConfApi->CreateOnlyApi(((CConf*)(pBridgePartyInitParams->GetConf()))->GetRcvMbx(), this);
  m_pConfApi->SetLocalMbx(((CConf*)(pBridgePartyInitParams->GetConf()))->GetLocalQueue());

  m_pBridge           = (CBridge*)(pBridgePartyInitParams->GetBridge());
  m_pParty            = (CTaskApp*)(pBridgePartyInitParams->GetParty());
  m_partyRsrcID       = pBridgePartyInitParams->GetPartyRsrcID();
  m_confRsrcID        = pBridgePartyInitParams->GetConfRsrcID();
  m_wNetworkInterface = pBridgePartyInitParams->GetNetworkInterface();
  m_bCascadeLinkMode  = pBridgePartyInitParams->GetCascadeLinkMode();

  BOOL         bVideoInFailure = FALSE;
  CRsrcParams* pRsrcParams = new CRsrcParams;
  pRsrcParams->SetConfRsrcId(m_confRsrcID);
  pRsrcParams->SetPartyRsrcId(m_partyRsrcID);

  m_pUpdatePartyInitParams = NULL;

  CBridgePartyVideoInParams* videoInParams = (CBridgePartyVideoInParams*)pBridgePartyInitParams->GetMediaInParams();

  eLogicalResourceTypes decoderLRT = eLogical_COP_Dynamic_decoder;
  WORD resourceIndex = videoInParams->GetCopResourceIndex();

  if (LM_DECODER_INDEX == resourceIndex)
    decoderLRT = eLogical_COP_LM_decoder;
  else if (VSW_DECODER_INDEX == resourceIndex)
    decoderLRT = eLogical_COP_VSW_decoder;

  NewPartyIn();

  if (decoderLRT == eLogical_COP_VSW_decoder)
    ((CBridgePartyVideoInCOP*)m_pBridgePartyIn)->SetVsw(YES);

  CTaskApi* pTaskApiVideoIn = new CTaskApi(*m_pConfApi);
  pTaskApiVideoIn->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyIn);

  CRsrcDesc* pRsrcDesc = pRoutingTbl->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, decoderLRT, pTaskApiVideoIn);
  if (!pRsrcDesc)   // Entry not found in Routing Table
  {
    bVideoInFailure = TRUE;
    POBJDELETE(m_pBridgePartyIn);
    PASSERT(104);
  }
  else
  {
    pRsrcParams->SetRsrcDesc(*pRsrcDesc);
    ((CBridgePartyVideoIn*)(m_pBridgePartyIn))->Create(this, pRsrcParams, pBridgePartyInitParams->GetMediaInParams());
  }

  POBJDELETE(pTaskApiVideoIn);
  POBJDELETE(pRsrcParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::NewPartyIn()
{
  m_pBridgePartyIn = new CBridgeVideoInCopDecoder();
  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCopDecoder::NewPartyIn m_pBridgePartyIn==", (DWORD) m_pBridgePartyIn);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::Close()
{
  DispatchEvent(VIDCLOSE, NULL);
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCopDecoder::IsValidState(WORD state) const
{
  WORD valid_state = TRUE;
  switch (state)
  {
    case CONNECTED_STANDALONE:
    case EXPORT:
    {
      valid_state = FALSE;
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCopDecoder::IsValidState NOT VALID STATE!!!", state);
      PASSERT(1);
      break;
    }
  } // switch

  return valid_state;
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCopDecoder::IsValidEvent(OPCODE event) const
{
  WORD valid_event = TRUE;
  switch (event)
  {
    case VIDEO_OUT_CONNECTED:
    case VIDEO_OUT_DISCONNECTED:
    case UPDATE_VIDEO_OUT_PARAMS:
    case PARTY_VIDEO_OUT_UPDATED:
    case ADDIMAGE:
    case FASTUPDATE:
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
    case PARTYLAYOUTCHANGED:
    case PRIVATELAYOUT_ONOFF_CHANGED:
    case DISPLAY_TEXT_ON_SCREEN:
    {
      valid_event = FALSE;
      PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCopDecoder::IsValidEvent NOT VALID EVENTS!!!", event);
      PASSERT(1);
      break;
    }
  } // switch

  return valid_event;
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoBridgeCopDecoder::GetCopDecoderArtId() const
{
  DWORD artId = DUMMY_PARTY_ID;
  if (m_pBridgePartyIn)
  {
    artId = ((CBridgeVideoInCopDecoder*)m_pBridgePartyIn)->GetCopDecoderArtId();
  }

  return artId;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeConnectIDLE(CSegment* pParam)
{
  if (!m_pBridgePartyIn)
  {
    DWORD     copDecoderIndex = GetCopDecoderIndex();
    CSegment* pSeg = new CSegment;
    *pSeg << (DWORD)copDecoderIndex;
    m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_CONNECT_COP_DECODER, statInconsistent, FALSE, eNoDirection, pSeg);
    POBJDELETE(pSeg);
    return;
  }

  BYTE isIVR;
  *pParam >> isIVR;

  EMediaDirection eDirection = eNoDirection;
  if (m_pBridgePartyIn)
    eDirection |= eMediaIn;

  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeConnectIDLE :", m_partyConfName);
  Setup();
}

////////////////////////////////////////////////////////////////////////////
// There is only in direction we are not supposed to get connect request in SETUP
void CVideoBridgeCopDecoder::OnVideoBridgeConnectSETUP(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeConnectSETUP: IGNORED!", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
// There is only in direction we are not supposed to get connect request in SETUP
void CVideoBridgeCopDecoder::OnVideoBridgeConnectCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeConnectCONNECTED: IGNORED!", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeConnectOPENED(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeConnectOPENED");
  m_state = CONNECTING;
  StartTimer(VIDEO_BRDG_PARTY_SETUP_TOUT, VIDEO_BRDG_PARTY_SETUP_TOUT_VALUE);
  if (m_pBridgePartyIn)
    m_pBridgePartyIn->Connect();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeConnectCONNECTING(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeConnectCONNECTING -Ignored!");
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeConnectCLOSING(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeConnectCLOSING -Ignored!");
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection)
{
  if (!m_pBridgePartyIn) {
    DBGPASSERT_AND_RETURN(1);
  }

  if (eConnectedMediaDirection != eMediaIn) {
    DBGPASSERT_AND_RETURN(2);
  }

  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::VideoConnectionCompletion ", m_partyConfName);

  BOOL  isVideoConnectionCompleted  = FALSE;
  EStat receivedStatus        = statOK;
  *pParams >> (BYTE&)receivedStatus;

  DWORD     copDecoderIndex = GetCopDecoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copDecoderIndex;
  DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);
  if (statOK != receivedStatus)
  {
    DumpMcuInternalProblemDetailed((BYTE)eConnectedMediaDirection, eMipStatusFail, eMipVideo);
    *pSeg << *pParams;
  }
  else
    m_state = CONNECTED;

  PTRACE2INT(eLevelInfoNormal, "CVideoBridgeCopDecoder::VideoConnectionCompletion Decoder Index = ", copDecoderIndex);
  // Inform Video Bridge
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_CONNECT_COP_DECODER, receivedStatus, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnTimerPartySetup(CSegment* pParams)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnTimerPartySetup ", m_partyConfName);

  DWORD     copDecoderIndex = GetCopDecoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copDecoderIndex;

  CMedString decoderString;
  if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
  {
    ((CBridgePartyVideoIn*)m_pBridgePartyIn)->DumpAllInfoOnConnectionState(&decoderString, true);
  }

  // Add Fault to EMA
  std::string faultString = "Video Decoder:";
  faultString += decoderString.GetString();
  CBridgePartyMediaUniDirection::AddFaultAlarm(faultString.c_str(), m_partyRsrcID, STATUS_OK, true);

  // for debug info in case of the "MCU internal problem"
  BYTE failureCauseDirection =   eMipNoneDirction;
  BYTE failureCauseAction    =   eMipNoAction;
  GetRsrcProbAdditionalInfoOnVideoTimerSetup(failureCauseDirection, failureCauseAction);

  *pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)eMipIn << (BYTE)eMipTimer <<(BYTE) failureCauseAction;
  DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);

  // Inform Video Bridge of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoEncoder+Decoder
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_CONNECT_COP_DECODER, statVideoInOutResourceProblem, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInConnectedCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInConnectedCONNECTING: ", m_partyConfName);
  VideoConnectionCompletion(pParam, eMediaIn);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInConnectedOPENED(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInConnectedOPENED -Ignored!");
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInConnectedCLOSING(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInConnectedCLOSING -Ignored!");
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeCloseIDLE(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeCloseIDLE the decoder is already closed");
  DBGPASSERT(101);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeCloseSETUP(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeCloseSETUP");
  OnVideoBridgeClose(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeCloseCONNECTED(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeCloseCONNECTED");
  OnVideoBridgeClose(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeCloseDISCONNECTING(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeCloseDISCONNECTING");
  OnVideoBridgeClose(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeCloseCLOSING(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeCloseCLOSING -Ignored!");
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeCloseCONNECTING(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeCloseCONNECTING");
  OnVideoBridgeClose(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeCloseOPENED(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeCloseOPENED");
  OnVideoBridgeClose(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeClose(CSegment* pParam)
{
  StartTimer(VIDEO_BRDG_PARTY_CLOSE_TOUT, VIDEO_BRDG_PARTY_CLOSE_TOUT_VALUE);
  if (m_pBridgePartyIn)
  {
    m_state = CLOSING;
    ((CBridgeVideoInCopDecoder*)m_pBridgePartyIn)->Close();
  }
  else // KEREN
    PASSERT(1);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnTimerPartyClosingCLOSING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnTimerPartyClosingCLOSING Decoder Index = ", m_partyConfName);
  // Add Fault to EMA
  CBridgePartyMediaUniDirection::AddFaultAlarm("Did not receive CLOSE PORT ack ", m_partyRsrcID, STATUS_OK, true);

  // In case we didn't receive ack on close cop decoder we will send kill port.
  SendKillPort();

  m_state = IDLE;
  DWORD copDecoderIndex = GetCopDecoderIndex();

  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copDecoderIndex;

  *pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)eMipIn << (BYTE)eMipTimer <<(BYTE) eMipClose;

  DestroyPartyInOut(eNoDirection);

  // Inform Video Bridge about connection failure
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_CLOSE_COP_DECODER, statVideoInOutResourceProblem, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
WORD CVideoBridgeCopDecoder::GetCopDecoderIndex() const
{
  WORD decoderIndex = (0xFFFF);
  if (m_pBridgePartyIn)
    decoderIndex = ((CBridgeVideoInCopDecoder*)m_pBridgePartyIn)->GetCopDecoderIndex();

  return decoderIndex;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInClosedSETUP(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInClosedSETUP  Ignored!", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInClosedCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInClosedCONNECTED  Ignored!", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInClosedDISCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInClosedCONNECTED  Ignored!", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInClosedCLOSING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInClosedCLOSING ", m_partyConfName);
  OnVideoInClosed(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInClosedCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInClosedCONNECTING Ignored!", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInClosedOPENED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInClosedOPENED Ignored!", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInClosed(CSegment* pParams)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInClosed ", m_partyConfName);
  DWORD                   copDecoderIndex = GetCopDecoderIndex();
  CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
  if (pRoutingTbl == NULL)
  {
    PASSERT_AND_RETURN(101);
  }

  if (!m_pBridgePartyIn)
  {
    PASSERT_AND_RETURN(102);
  }

  if (!((CBridgeVideoInCopDecoder*)m_pBridgePartyIn)->IsClosed())
  {
    PASSERT_AND_RETURN(103);
  }

  CRsrcParams* pRsrcParams = m_pBridgePartyIn->GetRsrcParams();
  if (!pRsrcParams)
  {
    PASSERT_AND_RETURN(104);
  }

  if (STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams))
    DBGPASSERT(105);

  EStat receivedStatus = statOK;
  BYTE  videoInClosePortStatus = statOK;

  *pParams >> (BYTE&)receivedStatus;
  DeleteTimer(VIDEO_BRDG_PARTY_CLOSE_TOUT);
  m_state = IDLE;

  // for debug info in case of the "MCU internal problem"
  BYTE failureCauseDirection =   eMipNoneDirction;

  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copDecoderIndex;

  // Inform Video Bridge In case of problem
  if (statVideoInOutResourceProblem == receivedStatus)
  {
    *pSeg << (BYTE)receivedStatus << (BYTE)failureCauseDirection << (BYTE)eMipStatusFail <<(BYTE) eMipClose;
  }

  else
  {
    PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInClosed - Decoder was closed - state is IDLE : Name - ", m_partyConfName);
  }

  DestroyPartyInOut(eNoDirection);
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_CLOSE_COP_DECODER, receivedStatus, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInDisconnected(CSegment* pParams)
{
  DWORD copDecoderIndex = GetCopDecoderIndex();
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInDisconnected,", m_partyConfName);
  BOOL  isVideoConnectionCompleted  = FALSE;
  EStat receivedStatus        = statOK;

  *pParams >> (BYTE&)receivedStatus;

  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copDecoderIndex;

  if (statOK != receivedStatus)
  {
    *pSeg << *pParams;
  }

  DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);
  m_state = OPENED;

  // Inform Video Bridge
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DISCONNECT_COP_DECODER, statOK, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnTimerPartyDisconnectDISCONNECTING", m_partyConfName);
  OnTimerPartyDisconnect(pParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnTimerPartyDisconnect(CSegment* pParams)
{
  DWORD           copDecoderIndex = GetCopDecoderIndex();
  EMediaDirection eConnectedDirection = eNoDirection;
  BYTE            failureCauseDirection =   eMipNoneDirction;

  CMedString logStr;
  logStr << "CVideoBridgePartyCntl::OnTimerPartyDisconnect: Name - " << m_partyConfName;
  logStr << "\nm_partyRsrcID = " << m_partyRsrcID << " , m_confRsrcID = " << m_confRsrcID;
  PTRACE(eLevelError, logStr.GetString());

  // Add Fault to EMA
  CBridgePartyMediaUniDirection::AddFaultAlarm("Did not receive all acks in Decoder Disconnection", m_partyRsrcID, STATUS_OK, true);


  // for debug info in case of the "MCU internal problem"
  failureCauseDirection = eMipIn;

  CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();

  if (pRoutingTbl == NULL) {
    PASSERT_AND_RETURN(101);
  }

  if (!m_pBridgePartyIn) {
    PASSERT_AND_RETURN(102);
  }

  if (!m_pBridgePartyIn->IsDisConnected()) {
    PASSERT_AND_RETURN(103);
  }

  CRsrcParams* pRsrcParams = m_pBridgePartyIn->GetRsrcParams();

  if (!pRsrcParams)
    PTRACE(eLevelError, "CVideoBridgeCopDecoder::OnTimerPartyDisconnect - pRsrcParams = NULL");

  else
  {
    if (STATUS_FAIL == pRoutingTbl->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams))
      DBGPASSERT(105);
  }

  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copDecoderIndex;

  *pSeg << (BYTE)statVideoInOutResourceProblem << (BYTE)eMipIn << (BYTE)eMipTimer <<(BYTE) eMipDisconnect;

  if (GetConnectionFailureCause() == statOK)
  {
    DumpMcuInternalProblemDetailed(failureCauseDirection, eMipTimer, eMipVideo);
  }

  // Inform Video Bridge of statVideoInOutResourceProblem -> Inorder to initiate Kill port
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DISCONNECT_COP_DECODER, statVideoInOutResourceProblem, FALSE, (EMediaDirection)eMipIn, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoOutVideoRefresh(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoOutVideoRefresh = ", m_partyConfName);

  DWORD     copDecoderIndex = GetCopDecoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copDecoderIndex;

  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, VIDEO_REFRESH_COP_DECODER, statOK, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeUpdateVideoInParamsCONNECTING(CSegment* pParam)
{
  OnVideoBridgeUpdateVideoInParams(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeUpdateVideoInParamsOPENED(CSegment* pParam)
{
  OnVideoBridgeUpdateVideoInParams(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeUpdateVideoInParamsCLOSING(CSegment* pParam)
{
  OnVideoBridgeUpdateVideoInParams(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeDisconnectDISCONNECTING(CSegment* pParam)
{
  // VNGR-1254
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeDisConnectDISCONNECTING - we will resend the disconnect : Name - ", GetFullName());
  OnVideoBridgeDisconnect(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeDisconnectCONNECTING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeDisconnectCONNECTING : Name - ", m_partyConfName);
  OnVideoBridgeDisconnect(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeDisconnectOPENED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeDisconnectOPENED : Name - ", m_partyConfName);
  // vngr-13186
  // Disconnect Video In
  if (m_pBridgePartyIn)
    m_pBridgePartyIn->DisConnect();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoBridgeDisconnectCLOSING(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoBridgeDisconnectCLOSING : Name - ", m_partyConfName);
  // Disconnect Video In
  if (m_pBridgePartyIn)
    m_pBridgePartyIn->DisConnect();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInUpdated(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInUpdated In case of cop decoder we don't send it to party: Name - ", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInSendH239Caps(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInSendH239Caps IN CASE of cop decoder it shouldn't be relevant --> no ISDN: Name - ", m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInUpdateDecoderDetectedModeCONNECTED(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal, "CVideoBridgeCopDecoder::OnVideoInUpdateDecoderDetectedModeCONNECTED , Name - ", m_partyConfName);
  OnVideoInUpdateDecoderDetectedMode(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInUpdateDecoderDetectedMode(CSegment* pParam)
{
  // In COP lecture mode in some cases we are waiting for the decoder to sync with right resolution ratio
  VideoInSyncedCopDecoder();
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCopDecoder::IsClosed() const
{
  BYTE isClosed = NO;
  if (m_state == IDLE)
  {
    isClosed = YES;
  }

  return isClosed;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInSynced()
{
  CVideoBridgePartyCntl::OnVideoInSynced(NULL);
  VideoInSyncedCopDecoder();
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::VideoInSyncedCopDecoder()
{
  DWORD     copDecoderIndex = GetCopDecoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copDecoderIndex;
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, VIDEO_IN_SYNCED_COP_DECODER, statOK, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInDisconnectedOPENED(CSegment* pParam)
{
  PTRACE2(eLevelError, "CVideoBridgeCopDecoder::OnVideoInDisconnectedOPENED : Already disconnected! : Name - ", m_partyConfName);
  DWORD     copDecoderIndex = GetCopDecoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copDecoderIndex;
  DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);
  // Inform Video Bridge
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DISCONNECT_COP_DECODER, statOK, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::OnVideoInDisconnectedCLOSING(CSegment* pParam)
{
  PTRACE2(eLevelError, "CVideoBridgeCopDecoder::OnVideoInDisconnectedCLOSING : Already disconnected! : Name - ", m_partyConfName);
  DWORD     copDecoderIndex = GetCopDecoderIndex();
  CSegment* pSeg = new CSegment;
  *pSeg << (DWORD)copDecoderIndex;
  DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);
  // Inform Video Bridge
  m_pConfApi->PartyBridgeResponseMsg(m_pParty, VIDEO_BRIDGE_MSG, END_DISCONNECT_COP_DECODER, statOK, FALSE, eNoDirection, pSeg);
  POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeCopDecoder::SendKillPort()
{
  if (m_pBridgePartyIn)
  {
    PTRACE2(eLevelError, "CVideoBridgeCopDecoder::SendKillPort : Name - ", m_partyConfName);
    ((CBridgeVideoInCopDecoder*)m_pBridgePartyIn)->SendKillPort();
  }
}

////////////////////////////////////////////////////////////////////////////
BYTE CVideoBridgeCopDecoder::IsDetectedVideoMatchCurrentResRatio()
{
  BYTE isDetectedVideoMatchCurrentResRatio = NO;
  if (m_pBridgePartyIn)
  {
    isDetectedVideoMatchCurrentResRatio = ((CBridgeVideoInCopDecoder*)m_pBridgePartyIn)->IsDetectedVideoMatchCurrentResRatio();
  }

  return isDetectedVideoMatchCurrentResRatio;
}

