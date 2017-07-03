#ifndef _BRIDGE_VIDEO_OUT_COP_ENCODER
  #define _BRIDGE_VIDEO_OUT_COP_ENCODER
// +========================================================================+
// BridgeVideoOutCopEncoder.h                                               |
// Copyright 2005 Polycom                                                   |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// FILE:       BridgeVideoOutCopEncoder.h                                   |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Talya                                                        |
// -------------------------------------------------------------------------|
// Who  | Date  Aug-2005  | Description                                     |
// -------------------------------------------------------------------------|
// +========================================================================+

#include "BridgePartyVideoOut.h"

class CBridgePartyVideoOut;
class CBridgePartyCntl;
class CRsrcParams;
class CBridgePartyVideoOutParams;
class CTaskApi;
class CLayoutHandler;
class CLayout;
class CConf;
class CSiteNameInfo;

////////////////////////////////////////////////////////////////////////////
//                        CBridgeVideoOutCopEncoder
////////////////////////////////////////////////////////////////////////////
class CBridgeVideoOutCopEncoder : public CBridgePartyVideoOut
{
  CLASS_TYPE_1(CBridgeVideoOutCopEncoder, CBridgePartyVideoOut)

public:
  enum STATE {SETUP = (IDLE+1), SLIDE, CONNECTED, CHANGE_LAYOUT, DISCONNECTING};

                        CBridgeVideoOutCopEncoder ();
  virtual              ~CBridgeVideoOutCopEncoder ();
  virtual const char*   NameOf() const                           { return "CBridgeVideoOutCopEncoder";}

  virtual void          Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pBridgePartyMediaParams);
  virtual void          UpdateNewConfParams(DWORD confRsrcId, const CBridgePartyMediaParams* pBridgePartyVideoOutParams);
  virtual BYTE          BuildLayout();

  WORD                  GetCopEncoderIndex() const               { return m_copEncoderIndex;}
  virtual void          ChangeLayout(CLayout* layout, CVisualEffectsParams* visualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout, BYTE sendChangeLayoutAnyWay = NO);
  virtual void          ChangeLayoutAttributes(CVisualEffectsParams* visualEffects, DWORD speakerPlaceInLayout);
  virtual WORD          IsValidState(WORD state) const;
  virtual WORD          IsValidEvent(OPCODE event) const;
  void                  ConnectPCMEncoderToPCMMenu(DWORD pcmMenuId);
  void                  DisConnectPCMEncoderFromPCMMenu(DWORD pcmMenuId);
  void                  UpdateVideoOutParams(CBridgePartyVideoOutParams* pOutVideoParams);
  void                  UpdateFlowControlRate(DWORD flowControlRate);
  void                  SetFlowControlRate(DWORD flowControlRate){m_flowControlRate = flowControlRate;}
  DWORD                 GetEncoderFlowControlRate() const        {return m_flowControlRate;}
  CLayout*              GetCopEncoderLayout()                    {return m_pCopConfLayout;}
  ECopDecoderResolution GetCopDecoderResolutionFromVideoParams();
  void                  DSPSmartSwitchChangeLayout(CLayout* layout, CVisualEffectsParams* visualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout, BYTE sendChangeLayoutAnyWay);

protected:
  virtual void          SendOpenEncoder();
  virtual void          SendChangeLayoutToHardware(BYTE isVSW = NO);
  virtual void          SendChangeLayoutAttributesToHardware(BYTE isFromAudioSpeakerChange = NO);
  virtual void          BuildLayoutAndSendOnEncoderResolutionChange();
  virtual void          SetLayoutHandler();
  virtual void          OnVideoBridgePartyConnectIDLE(CSegment* pParam);
  virtual void          OnMplAckSETUP(CSegment* pParam);
  virtual void          OnVideoBridgePartyFastUpdateCHANGELAYOUT(CSegment* pParam);
  virtual void          OnVideoBridgePartyEncoderResolutionChangedCONNECTED(CSegment* pParam);
  virtual void          OnVideoBridgePartyEncoderResolutionChangedCHANGELAYOUT(CSegment* pParam);
  virtual void          OnMplOpenPortAck(STATUS status);
  virtual void          OnMplChangeLayoutAck(STATUS status);
  virtual void          OnVideoBridgeChangeCopLayoutCONNECTED(CSegment* pParam);
  virtual void          OnVideoBridgeChangeCopLayoutCHANGE_LAYOUT(CSegment* pParam);
  virtual void          OnVideoBridgeChangeCopLayoutAttributesCONNECTED(CSegment* pParam);
  virtual void          OnVideoBridgeChangeCopLayoutAttributesCHANGE_LAYOUT(CSegment* pParam);
  virtual void          OnUpdateFlowControlRateCONNECTED(CSegment* pParam);
  virtual void          OnUpdateFlowControlRateCHANGE_LAYOUT(CSegment* pParam);
  virtual void          OnUpdateFlowControlRate(CSegment* pParam);


  BYTE                  IsVsw() const;

  void                  OnMplEncoderSyncIndCONNECTED(CSegment* pParam);
  void                  OnMplEncoderSyncIndCHANGE_LAYOUT(CSegment* pParam);
  void                  OnMplEncoderSyncInd(CSegment* pParam);

  // DSP Smart Switch
  void                  SendDSPSmartSwitchChangeLayoutToHardware(CLayout* layout, CVisualEffectsParams* visualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout);

  // members
  eLogicalResourceTypes m_copLrt;
  WORD                  m_copEncoderIndex;
  CLayout*              m_pCopConfLayout;
  DWORD                 m_pCopConfSpeakerPlaceInLayout;
  DWORD                 m_flowControlRate;
  BYTE                  m_sendFastUpdateAtChangeLayoutAck;

  PDECLAR_MESSAGE_MAP
};

#endif // _BRIDGE_VIDEO_OUT_COP_ENCODER

