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
// FILE:       VideoBridgeCopDeccoder.H                                     |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Keren                                                        |
// -------------------------------------------------------------------------|
// Who  | Date  Aug-2009  | Description                                     |
// -------------------------------------------------------------------------|
// +========================================================================+

#ifndef _CVideoBridgeCopDecoder_H_
#define _CVideoBridgeCopDecoder_H_

#include "VideoBridgePartyCntl.h"


////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeCopDecoder
////////////////////////////////////////////////////////////////////////////
class CVideoBridgeCopDecoder : public CVideoBridgePartyCntl
{
  CLASS_TYPE_1(CVideoBridgeCopDecoder, CVideoBridgePartyCntl)

public:
  enum STATE {SETUP = (IDLE+1), CONNECTED_STANDALONE, CONNECTED, DISCONNECTING, EXPORT, CLOSING, CONNECTING, OPENED};

                      CVideoBridgeCopDecoder ();
                      CVideoBridgeCopDecoder(const CVideoBridgeCopDecoder& rOtherBridgePartyCntlContent);
  virtual            ~CVideoBridgeCopDecoder();

  virtual void        Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
  virtual void        NewPartyIn();
  virtual const char* NameOf() const;
  virtual void        Close();
  virtual WORD        IsValidState(WORD state) const;
  virtual WORD        IsValidEvent(OPCODE event) const;
  DWORD               GetCopDecoderArtId() const;

  BYTE                IsClosed() const;
  BYTE                IsDetectedVideoMatchCurrentResRatio();

protected:
  virtual void        VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection);

  virtual void        OnVideoBridgeConnectIDLE(CSegment* pParam);
  virtual void        OnVideoBridgeConnectSETUP(CSegment* pParam);
  virtual void        OnVideoBridgeConnectCONNECTED(CSegment* pParam);

  virtual void        OnVideoBridgeConnectOPENED(CSegment* pParam);
  virtual void        OnVideoBridgeConnectCONNECTING(CSegment* pParam);
  virtual void        OnVideoBridgeConnectCLOSING(CSegment* pParam);

  virtual void        OnVideoBridgeDisconnectDISCONNECTING(CSegment* pParam);
  virtual void        OnVideoBridgeDisconnectCONNECTING(CSegment* pParam);
  virtual void        OnVideoBridgeDisconnectOPENED(CSegment* pParam);
  virtual void        OnVideoBridgeDisconnectCLOSING(CSegment* pParam);

  virtual void        OnVideoBridgeUpdateVideoInParamsCONNECTING(CSegment* pParam);
  virtual void        OnVideoBridgeUpdateVideoInParamsOPENED(CSegment* pParam);
  virtual void        OnVideoBridgeUpdateVideoInParamsCLOSING(CSegment* pParam);

  virtual void        OnVideoBridgeCloseIDLE(CSegment* pParam);
  virtual void        OnVideoBridgeCloseSETUP(CSegment* pParam);
  virtual void        OnVideoBridgeCloseCONNECTED(CSegment* pParam);
  virtual void        OnVideoBridgeCloseDISCONNECTING(CSegment* pParam);
  virtual void        OnVideoBridgeCloseCLOSING(CSegment* pParam);
  virtual void        OnVideoBridgeCloseCONNECTING(CSegment* pParam);
  virtual void        OnVideoBridgeCloseOPENED(CSegment* pParam);
  virtual void        OnVideoBridgeClose(CSegment* pParam);

  virtual void        OnVideoInConnectedCONNECTING(CSegment* pParam);
  virtual void        OnVideoInConnectedOPENED(CSegment* pParam);
  virtual void        OnVideoInConnectedCLOSING(CSegment* pParam);

  void                VideoInSyncedCopDecoder();
  virtual void        OnVideoInSynced();
  virtual void        OnVideoInDisconnected(CSegment* pParam);
  virtual void        OnVideoInUpdated(CSegment* pParam);
  virtual void        OnVideoOutVideoRefresh(CSegment* pParam);
  virtual void        OnVideoInSendH239Caps(CSegment* pParam);
  virtual void        OnVideoInUpdateDecoderDetectedModeCONNECTED(CSegment* pParam);
  virtual void        OnVideoInUpdateDecoderDetectedMode(CSegment* pParam);

  virtual void        OnVideoInClosedSETUP(CSegment* pParam);
  virtual void        OnVideoInClosedCONNECTED(CSegment* pParam);
  virtual void        OnVideoInClosedDISCONNECTING(CSegment* pParam);
  virtual void        OnVideoInClosedCLOSING(CSegment* pParam);
  virtual void        OnVideoInClosedCONNECTING(CSegment* pParam);
  virtual void        OnVideoInClosedOPENED(CSegment* pParam);
  virtual void        OnVideoInClosed(CSegment* pParam);

  virtual void        OnVideoInDisconnectedOPENED(CSegment* pParam);
  virtual void        OnVideoInDisconnectedCLOSING(CSegment* pParam);

  virtual void        OnTimerPartySetup(CSegment* pParams);
  virtual void        OnTimerPartyDisconnect(CSegment* pParams);
  virtual void        OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams);
  virtual void        OnTimerPartyClosingCLOSING(CSegment* pParam);

  WORD                GetCopDecoderIndex() const;
  void                SendKillPort();

  PDECLAR_MESSAGE_MAP
};

#endif // _CVideoBridgeCopDecoder_H_

