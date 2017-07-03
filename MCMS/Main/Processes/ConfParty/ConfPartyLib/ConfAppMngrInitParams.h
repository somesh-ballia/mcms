//+========================================================================+
//                   ConfAppMngrInitParams.H                               |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       ConfAppMngrInitParams.H                                     |
//-------------------------------------------------------------------------|
// Who  | Date  Aug-2005  | Description                                    |
//-------------------------------------------------------------------------|

#ifndef __CONF_APP_MNGR_INIT_PARAMS_H__
#define __CONF_APP_MNGR_INIT_PARAMS_H__

#include "ConfPartyDefines.h"

class CAudioBridgeInterface;
class CVideoBridgeInterface;
class CConf;

class CConfAppMngrInitParams
{
public:
  CConfAppMngrInitParams(const CConf* pConf,
                         const char* pConfName,
                         const DWORD confRsrcId,
                         const char* confIvrName,
                         CAudioBridgeInterface* pAudBrdgInterface,
                         CVideoBridgeInterface* pVideoBridgeInterface,
                         WORD isIvrInConf,
                         WORD isVideoConf,
                         WORD isWaitForChair,
                         WORD isTerminateConfAfterChairDropped,
                         WORD isEQConf,
                         WORD isCascadeEQ,
                         WORD enableRecording,
                         WORD enableRecordingIcon,
                         WORD enableRecNotify,
                         WORD startRecordingPolicy,
                         WORD isGateway,
                         WORD isOperatorConf,
                         BYTE isExternalIVRInConf,
                         BYTE isMuteAllPartiesAudioExceptLeader
                         )
  {
    m_pConf                 = pConf;
    m_pConfName             = pConfName;
    m_confRsrcId            = confRsrcId;
    m_pAudBrdgInterface     = pAudBrdgInterface;
    m_pVideoBridgeInterface = pVideoBridgeInterface;
    m_isIvrInConf           = isIvrInConf;
    m_isVideoConf           = isVideoConf;
    m_isWaitForChair        = isWaitForChair;
    m_isTerminateConfAfterChairDropped=isTerminateConfAfterChairDropped;
    m_confIvrName           = confIvrName;
    m_isEQConf              = isEQConf;
    m_isCascadeEQ           = isCascadeEQ;
    m_enableRecording       = enableRecording;
    m_enableRecordingIcon   = enableRecordingIcon;
    m_enablerecNotify		= enableRecNotify;
    m_startRecordingPolicy  = startRecordingPolicy;
    m_isGateWay             = isGateway;
    m_isOperatorConf        = isOperatorConf;
    m_isExternalIVRInConf   = isExternalIVRInConf;
    m_isMuteAllPartiesAudioExceptLeader = isMuteAllPartiesAudioExceptLeader;
  }

  ~CConfAppMngrInitParams()
  {
  }

public:
  const CConf*              m_pConf;
  const char*               m_pConfName;
  DWORD                     m_confRsrcId;
  CAudioBridgeInterface*    m_pAudBrdgInterface;
  CVideoBridgeInterface*    m_pVideoBridgeInterface;
  WORD                      m_isIvrInConf;
  WORD                      m_isVideoConf;
  WORD                      m_isWaitForChair;
  WORD 						m_isTerminateConfAfterChairDropped;
  const char*               m_confIvrName;
  WORD                      m_isEQConf;
  WORD                      m_isCascadeEQ;
  WORD                      m_startRecordingPolicy;
  WORD                      m_enableRecording;
  WORD                      m_enableRecordingIcon;
  WORD			m_enablerecNotify;
  WORD                      m_isGateWay;
  WORD                      m_isOperatorConf;
  BYTE                      m_isExternalIVRInConf;
  BYTE                      m_isMuteAllPartiesAudioExceptLeader;                                      // YES / NO
};

#endif //__CONF_APP_MNGR_INIT_PARAMS_H__

