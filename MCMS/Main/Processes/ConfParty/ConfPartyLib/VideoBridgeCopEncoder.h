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
// FILE:       VideoBridgeCopEncoder.H                                      |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Yoella                                                       |
// -------------------------------------------------------------------------|
// Who  | Date  Aug-2009  | Description                                     |
// -------------------------------------------------------------------------|
// +========================================================================+

#ifndef _CVideoBridgeCopEncoder_H_
#define _CVideoBridgeCopEncoder_H_

#include "VideoBridgePartyCntl.h"
#include "Image.h"
#include "UpdatePartyVideoInitParams.h"
#include "TextOnScreenMngr.h"
#include "LPRData.h"

class CBridgePartyInitParams;
class CLayoutHandler;
class CLayout;
class CVisualEffectsParams;
class CVideoBridgePartyInitParams;
class CSiteNameInfo;


////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeCopEncoder
////////////////////////////////////////////////////////////////////////////
class CVideoBridgeCopEncoder : public CVideoBridgePartyCntl
{
  CLASS_TYPE_1(CVideoBridgeCopEncoder, CVideoBridgePartyCntl)

public:
  enum STATE {SETUP = (IDLE+1), CONNECTED_STANDALONE, CONNECTED, DISCONNECTING, EXPORT};

                          CVideoBridgeCopEncoder();
  virtual                ~CVideoBridgeCopEncoder ();
                          CVideoBridgeCopEncoder(const CVideoBridgeCopEncoder& rOtherBridgePartyCntl);
  CVideoBridgeCopEncoder& operator=(const CVideoBridgeCopEncoder& rOtherBridgePartyCntl);

public:
  virtual void            Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
  virtual void            NewPartyOut();
  virtual const char*     NameOf() const { return "CVideoBridgeCopEncoder";}
  virtual void            Destroy(void);
  virtual void            FastUpdate(BYTE isPartyLinkToMgc = FALSE);
  virtual void            ChangeLayout(CLayout* layout, CVisualEffectsParams* visualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout, BYTE sendChangeLayoutAnyWay = FALSE);
  virtual void            ChangeLayoutAttributes(CVisualEffectsParams* visualEffects, DWORD speakerPlaceInLayout);
  virtual void            VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection);
  virtual void            VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection);
  virtual WORD            IsValidState(WORD state) const;
  virtual WORD            IsValidEvent(OPCODE event) const;

  BOOL                    IsCopEncoderInConnectedState();
  void                    ConnectPCMEncoderToPCMMenu(DWORD pcmMenuId);
  void                    DisConnectPCMEncoderFromPCMMenu(DWORD pcmMenuId);
  virtual void            UpdateVideoOutParams(CBridgePartyVideoOutParams* pOutVideoParams);
  BYTE                    IsClosed() const;
  void                    SetEncoderFlowControlRate(DWORD rate);
  void                    UpdateEncoderFlowControlRate(DWORD rate);
  DWORD                   GetEncoderFlowControlRate() const;
  CLayout*                GetCopEncoderLayout();
  ECopDecoderResolution   GetCopDecoderResolutionFromVideoParams();
  eVideoFrameRate         GetVidFrameRate();
  void                    DSPSmartSwitchChangeLayout(CLayout* layout, CVisualEffectsParams* visualEffects, CSiteNameInfo* pSiteNameInfo, DWORD speakerPlaceInLayout, BYTE sendChangeLayoutAnyWay);
  void                    SetSiteNameConfiguration(CSiteNameInfo* pSiteNameInfo);

protected:
  virtual void            OnVideoBridgeConnectIDLE(CSegment* pParam);
  virtual void            OnVideoBridgeConnectSETUP(CSegment* pParam);
  virtual void            OnVideoBridgeConnectCONNECTED(CSegment* pParam);
  virtual void            OnVideoBridgeConnectDISCONNECTING(CSegment* pParam);
  virtual void            OnVideoBridgeDisconnect(CSegment* pParam);

  virtual void            OnVideoOutConnectedCONNECTED(CSegment* pParam);
  virtual void            OnVideoOutUpdated(CSegment* pParam);

  virtual void            OnTimerPartySetupCONNECTED(CSegment* pParams);
  virtual void            OnTimerPartySetup(CSegment* pParams);
  virtual void            OnTimerPartyDisconnectSETUP(CSegment* pParams);
  virtual void            OnTimerPartyDisconnectCONNECTED(CSegment* pParams);
  virtual void            OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams);

  virtual void            OnTimerIgnoreIntraSETUP(CSegment* pParams);
  virtual void            OnTimerIgnoreIntraCONNECTED(CSegment* pParams);

  virtual void            IllegalCopEncoderState(CSegment* pParam);

  void                    OnTimerIgnoreIntra();
  WORD                    GetCopEncoderIndex() const;

  PDECLAR_MESSAGE_MAP
};
////////////////////////////////////////////////////////////////////////////
class CVideoBridgeXCodeEncoder : public CVideoBridgeCopEncoder
{
  CLASS_TYPE_1(CVideoBridgeXCodeEncoder, CVideoBridgeCopEncoder)

public:
		                  CVideoBridgeXCodeEncoder();
  virtual                ~CVideoBridgeXCodeEncoder ();

public:
  virtual void            Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
  virtual void            NewPartyOut();
  virtual const char*     NameOf() const { return "CVideoBridgeXCodeEncoder";}
  virtual void            FastUpdate(BYTE isPartyLinkToMgc = FALSE);
  virtual void            VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection);
  virtual void            VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection);
  virtual WORD            IsValidState(WORD state) const;
  virtual WORD            IsValidEvent(OPCODE event) const;
  virtual void            UpdateVideoOutParams(CBridgePartyVideoOutParams* pOutVideoParams);
  virtual WORD            IsFaultyXCodeEncoder(){return m_isFaulty;}

protected:
  virtual void            OnVideoBridgeConnectIDLE(CSegment* pParam);
  virtual void            OnVideoBridgeConnectSETUP(CSegment* pParam);
  virtual void            OnVideoBridgeConnectCONNECTED(CSegment* pParam);
  virtual void            OnVideoBridgeConnectDISCONNECTING(CSegment* pParam);

  virtual void            OnVideoOutUpdated(CSegment* pParam);

  virtual void            OnTimerPartySetup(CSegment* pParams);
  virtual void            OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams);

  void                    OnTimerIgnoreIntra();

  WORD                    m_isFaulty;


  PDECLAR_MESSAGE_MAP
};
#endif // _CVideoBridgeCopEncoder_H_


