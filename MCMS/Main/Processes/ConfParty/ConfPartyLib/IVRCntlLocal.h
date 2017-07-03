//+========================================================================+
//                            IVRCntlLocal.cpp                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IVRCntlLocal.h                                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 25.09.2000 |                                                      |
//+========================================================================+
#ifndef __IVRCNTLLOCAL_H__
#define __IVRCNTLLOCAL_H__

#include "IVRCntl.h"

class CIVRService;


class CIvrSubBaseSM;
class CIVRFeature;
class CDTMFCodeList;


////////////////////////////////////////////////////////////////////////////
//                        CIvrCntlLocal
////////////////////////////////////////////////////////////////////////////
class CIvrCntlLocal : public CIvrCntl

{
  CLASS_TYPE_1(CIvrCntlLocal, CIvrCntl)

public:
   CIvrCntlLocal(CTaskApp* pOwnerTask,
				 const CIVRService& pIvrService,
                 const char* pConfPassword,
                 const char* pConfLeaderPassword,
                 WORD startConfReqLeader,
                 const DWORD dwMonitorConfId,
                 const char* pNumericId = NULL,
                 WORD IsPartyInCascadeEQ = 0,
                 eGatewayPartyType gatewayPartyType = eRegularPartyNoGateway,
                 BYTE bNoVideRsrcForVideoParty = FALSE /*, CConf* pCConf = NULL*/);

  ~CIvrCntlLocal();

  virtual const char* NameOf() const       { return "CIvrCntlLocal";}
  virtual void*       GetMessageMap()      { return (void*)m_msgEntries; }

  // more global functions
  virtual void        Create(CParty* pParty, COsQueue* pConfRcvMbx);
  void                Start(CSegment* pParam = NULL);
  void                Restart();
  void                SetBoardID(WORD boardId);
  virtual const char*         GetLeaderPassword();
  WORD          	  IsLeaderReqForStartConf()         { return m_startConfReqLeader; }
  void                SetLeader(WORD bIsLeader);
  void                UpdateConfApi(COsQueue* pConfRcv);
  DWORD         	  GetMonitorConfId(void) const      { return m_dwMonitorConfId; }
  virtual void        HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);


  // action functions
  virtual void        OnEndFeature(CSegment* pParam);
  void                OnEndFeatureNotActive(CSegment* pParam);
  WORD                IsIvrFeatureUseDtmFforwarding(WORD opcode);
  void                OnDtmfOpcode(CSegment* pParam);
  void                OnPartyInviteResultInd( CSegment* pParam );
  void                OnErrorFeature(CSegment* pParam);
  void                OnIvrReset(CSegment* pParam);
  void                OnTimerSendfExt(CSegment* pParam);
  void                OnDtmfOpcodeNotActive(CSegment* pParam);

public:
  void                SetLanguage(WORD language);
  void                SetStartNextFeature();
  virtual BYTE                IsEmptyConf();
  void                SetIsLeader(WORD bUpdateParty, WORD bUpdateDB, BYTE bUpdateDTMFCollector = 0);
  CDtmfCollector*     GetDtmfCollector() const                                    { return m_pDtmfCollector;}
  void                MuteParty(CConfApi* pConfApi, CTaskApp* pParty, BYTE bPlayMessage = YES);
  void                UnMuteParty(CConfApi* pConfApi, CTaskApp* pParty);
  void                OverrideMuteAll(CConfApi* pConfApi);
  void                IncreaseVolume(CConfApi* pConfApi, BYTE bVolumeInOut = 0);  // 0 = Out, 1 = In
  void                DecreaseVolume(CConfApi* pConfApi, BYTE bVolumeInOut = 0);  // 0 = Out, 1 = In

  void                ResetIvr();
  void                StopIVR();
  void                OnTIPMasterStartedIVR(CSegment* pParam);
  void                OnPartyOnHoldInd(CSegment* pParam);

  void                MuteAllButX(CConfApi* pConfApi, BYTE yesNo);
  void                LockConference(CConfApi* pConfApi, BYTE yesNo);
  void                ConferenceOnHold(CConfApi* pConfApi, BYTE yesNo)            { }

  void                BillingCodeToCDR(const char* billingCode);


  CIVRService*        GetIVRService(void)                                         { return m_pIvrService;} // Add
  void                StartDialOut();
  void                InviteParty(CConfApi* pConfApi /*, BYTE byteEnable*/);
  void                ReInviteParty(CConfApi *pConfApi, BOOL bIsGwInvite);
  void                PlayBusyMsg(CConfApi *pConfApi);
  void                PlayNoAnswerMsg(CConfApi *pConfApi);
  void                PlayWrongNumberMsg(CConfApi *pConfApi);

  void                SetInvitePartyFeature();
  void                SetInvitePartyType(BYTE type)                               { m_invitePartyType = type; }
  void                SetInvitePartyAddress(const char* sPartyAddress);
  const char*         GetInvitePartyAddress()                                     { return m_invitePartyAddress; }


  void                SecureConf(CConfApi* pConfApi, BYTE SecureFlag);
  void                ChangePwds();
  void                SetChangePasswordFeature();
  void                SetChangeConfPasswordFeature();
  void                SetChangeLeaderPasswordFeature();
  void                SetChangePasswordFailed();
  void                SetIVRWaitStateForTIPSlave();
  virtual BYTE                GetChangePasswordType()                                     { return m_ChangePWDType; }
  void                SetChangePasswordType(BYTE type)                            { m_ChangePWDType = type; }

  const char*         GetNewPassword()                                            { return m_new_password; }
  void                SetNewPassword(const char* pass);
  void                ReviewNames(CConfApi* pConfApi);
  void                StopNamesReviewing(CConfApi* pConfApi);
  void                EnableRollCall(CConfApi* pConfApi, BYTE byteEnable);
  void                SendGeneralMessageMENU();

  void                SendDtmfForwarding(WORD wEventOpCode);

  void                SendRecordingLinkControlDTMF(WORD wEventOpCode);
  void                ShowParticipants(CConfApi* pConfApi);
  void                RequestToSpeak(CConfApi* pConfApi);
  void                ShowGathering();
  void                StartPcm();

  // Toggle dtmf functions
  BYTE                IsToggleSelfMute()                                          { return m_bIsToggleSelfMute; }
  void                SetToggleSelfMute(BYTE bIsToggleSelfMute)                   { m_bIsToggleSelfMute = bIsToggleSelfMute; }
  BYTE                IsToggleSecuredConference()                                 { return m_bIsToggleSecuredConference; }
  void                SetToggleSecuredConference(BYTE bIsToggleSecuredConference) { m_bIsToggleSecuredConference = bIsToggleSecuredConference; }
  BYTE                IsToggleMuteAllButMe()                                      { return m_bIsToggleMuteAllButMe; }
  void                SetToggleMuteAllButMe(BYTE bIsToggleMuteAllButMe)           { m_bIsToggleMuteAllButMe = bIsToggleMuteAllButMe; }

  void                MovePartyToInConf();
  void                ChangeToLeader();
  void                CancelSetLeader();
  void                ActivateIvrMuteNoisyLine();
  void                GetGeneralSystemMsgsParams(WORD event_op_code, char* msgFullPath, WORD* msgDuration, WORD* msgCheckSum);

  void                PartyNetDisconnected(WORD initiator);
  void                KeepNetDisconnect();
  void                Start_PLC();
  void                StartVenus();
  void                SetCascadeLinkInConf(BOOL isCascadeLinkInConf = TRUE);
  virtual int                 IsPSTNCall();
  virtual WORD                IsRecordingLinkParty();
  void                DoRecordingAction(CConfApi* pConfApi, WORD opcode);
  BOOL 				  IsRelayParty();
  void                PrepareForwardDtmfOpcodesTable(CDTMFCodeList* pCDTMFCodeList);
  bool                GetTurnOffIvrAfterMove()                                    {return m_turnOffIvrAfterMove;}
  void                SetTurnOffIvrAfterMove(bool turnOffIvrAfterMove)            {m_turnOffIvrAfterMove = turnOffIvrAfterMove;}
  WORD                IsOperatorParty();
  BOOL                IsTIPSlaveParty();

  WORD                OperatorAssistance(CConfApi* pConfApi, DWORD request_type);

  eGatewayPartyType   GetGatewayPartyType()                                       { return m_GatewayPartyType;}
  virtual void         SetGwDTMFForwarding(BYTE val);
  virtual void         SetInvitePartyDTMFForwarding(BYTE val);
  virtual void         onInvitePartyDTMFForwardingTout();


  virtual void                PcmConnected();
  virtual void                PcmDisconnected();

  void                HandleDtmfForwarding(CSegment* pParam);
  virtual void        SetLinkParty();

  void                setNoVideRsrcForVideoParty(BYTE mbNoVideRsrcForVideoParty);
  BYTE                getoVideRsrcForVideoParty()                                 {return m_bNoVideRsrcForVideoParty;}

  virtual BOOL                IsIvrProviderEQPartyInConf();
  void                SipConfNIDConfirmationInd(DWORD sts); // for IvrProviderEQ

  void                ResetFeaturesList();
  void                ResumeFeaturesList(DWORD lastStageBeforeResume = (DWORD)-1);
  void                ResetIvrHoldParams();
  virtual BOOL        IsIvrOnHold();
  bool				  IsConfOrLeaderPasswordRequired()const;
  void				  ResetFeaturesListForCallFromGW();

  static const char*  DtmfCodeToString(WORD opcode);
  static const char*  IvrStageToString(WORD stage);

  virtual bool IsMasterEndFeatures() { return m_masterEndFeatures; } //IVR for TIP

protected:
  CIvrSubBaseSM*      StartNextFeature();
  virtual void        StartNewFeature();
  void                GetFeaturesToDo();
  void				  GetFeaturesToDoMfwRestrictions();
  //void                OnStartIVR();
  void                GetHelpMessageParams(WORD* messageID, WORD* duration);
  void                OnConfTerminate(CConfApi* pConfApi, WORD IsDtmfForwarding = 0);
  void                RemoveLeaderPassword();
  WORD                GetMsgDuration(const CIVRFeature* feature, const WORD event_opcode);
  int                 NeedExtension();
  WORD                IsCascadeLinkPartyInConf();
  void                GetNumbersLen(int len, int* numLen, int* pLen);
  void                InformFeatureDTMF();
  int                 IsEnableDtmfForwarding();

  void                BlipOnCascadeLink();
  void                OnDtmfOpcodeImp(CSegment* pParam);

protected:
  WORD                m_language;
  const char*         m_pConfPassword;
  CIVRService*        m_pIvrService;
  char*               m_leaderPassword;
  WORD                m_startConfReqLeader;
  WORD                m_bIsLeaderFound;
  BYTE                m_ChangePWDType;
  BYTE                m_invitePartyType;
  char                m_new_password[CONFERENCE_ENTRY_PASSWORD_LEN];
  BYTE                m_bIsToggleSelfMute;
  BYTE                m_bIsToggleLockConference;
  BYTE                m_bIsToggleConferenceOnHold;
  BYTE                m_bIsToggleSecuredConference;
  BYTE                m_bIsToggleMuteAllButMe;
  char*               m_dtmfForExt;
  WORD                m_initExtensionTimer;
  WORD                m_pauseAtTheEnd;
  BYTE                m_updateDuringConf;   // Indication of an online change, used with "Change Password" and "Change to Leader"
  DWORD               m_featureActionDuringConf;
  WORD                m_cascadeLink;
  BOOL                m_bEnableExternalDB;
  bool                m_turnOffIvrAfterMove;
  eGatewayPartyType   m_GatewayPartyType;
  BYTE                m_bNoVideRsrcForVideoParty;
  char                m_invitePartyAddress[PARTY_ADDRESS_LEN];
  bool                m_masterEndFeatures; //IVR for TIP
  SFeatureStatus      m_featuresListForHoldCallScenario[NUM_OF_FEATURES+1];
  DWORD               m_stageForHoldCallScenario;

  PDECLAR_MESSAGE_MAP
};

#endif  // __IVRCNTLLOCAL_H__








//
//
//
///+========================================================================+
////                            IVRCntl.cpp                             |
////            Copyright 1995 Pictel Technologies Ltd.                      |
////                   All Rights Reserved.                                  |
////-------------------------------------------------------------------------|
//// NOTE: This software contains valuable trade secrets and proprietary     |
//// information of Pictel Technologies Ltd. and is protected by law.        |
//// It may not be copied or distributed in any form or medium, disclosed    |
//// to third parties, reverse engineered or used in any manner without      |
//// prior written authorization from Pictel Technologies Ltd.               |
////-------------------------------------------------------------------------|
//// FILE:       IVRCNTL.h                                                   |
//// SUBSYSTEM:  MCMS                                                        |
//// PROGRAMMER: Amir                                                        |
////-------------------------------------------------------------------------|
//// Who | Date       | Description                                          |
////-------------------------------------------------------------------------|
////     | 25.09.2000 |                                                      |
////+========================================================================+
//#ifndef __IVRCNTL_H__
//  #define __IVRCNTL_H__
//
//#include "StateMachine.h"
//#include "ConfPartyDefines.h"
//#include "ConfPartySharedDefines.h"
//
//class CIVRService;
//class CParty;
//class CConfApi;
//class CDtmfCollector;
//
//class CIvrSubBaseSM;
//class CIVRFeature;
//class CDTMFCodeList;
//
//#define PRIVATE_MSG                     1
//#define PUBLIC_MSG                      2
//#define MAX_DTMF_CHUNK                  31
//
//// ------- Ivr stages definition --------
//#define IVR_STAGE_LANGUAGE              0
//#define IVR_STAGE_NO_VIDEO_RSRC         1
//#define IVR_STAGE_GENERAL_WELCOME       2
//#define IVR_STAGE_NUMERIC_CONF_ID       3
//#define IVR_STAGE_CONF_PASSWORD         4
//#define IVR_STAGE_CONF_LEADER           5
//#define IVR_STAGE_BILLING_CODE          6
//#define IVR_STAGE_PIN_CODE              7
//#define IVR_STAGE_CONF_WELCOME          8
//#define IVR_STAGE_LOCK_SECURE           9
//#define IVR_STAGE_CHANGE_PWDS_MENU      10
//#define IVR_STAGE_CHANGE_CONF_PWD       11
//#define IVR_STAGE_CHANGE_LEADER_PWD     12
//#define IVR_STAGE_CHANGE_PWDS_CONFIRM   13
//#define IVR_STAGE_CHANGE_PWDS_OK        14
//#define IVR_STAGE_CHANGE_PWDS_INVALID   15
//#define IVR_STAGE_ROLL_CALL             16
//#define IVR_STAGE_MAX_PARTICIPANTS      17
//#define IVR_STAGE_CHANGE_TO_LEADER      18
//#define IVR_STAGE_RECORDING_IN_PROGRESS 19
//#define IVR_STAGE_RECORDING_FAILED      20
//#define IVR_STAGE_CASCADE_MCU_PARTY     21
//#define IVR_STAGE_VIDEO_INVITE          22
//#define IVR_STAGE_INVITE_PARTY          23
//#define IVR_STAGE_INVITE_PARTY_ADDRESS  24
//#define IVR_STAGE_INVITE_PARTY_MENU     25
//#define IVR_STAGE_GW_REINVITE_PARTY		26
//#define IVR_STAGE_DTMF_REINVITE_PARTY		27
//#define IVR_STAGE_PLAY_BUSY_MSG			28
//#define IVR_STAGE_PLAY_NOANSWER_MSG		29
//#define IVR_STAGE_PLAY_WRONG_NUMBER_MSG 		30
//
//
//// insert here, and increase the "NUM_OF_FEATURES"
//#define NUM_OF_FEATURES                 31
//
//// ---------------------------------------
//// Message Mode
//typedef enum
//{
//  IVR_STATUS_DUMMY       = 0,
//  IVR_STATUS_PLAY_ONCE   = 1,
//  IVR_STATUS_PLAY_LOOP   = 2,
//  IVR_STATUS_PLAY_RECORD = 3,
//  IVR_STATUS_RECORD      = 4,
//  IVR_STATUS_MAX         = 5
//} TIvrStatus, * pTIvrStatus;
//
//
//typedef enum
//{
//  CASCADE_LINK_DUMMY   = 0,
//  CASCADE_LINK_IN_CONF = 1,
//  CASCADE_LINK_IN_EQ   = 2,
//  CASCADE_LINK_MAX     = 3
//} TCascadeLink;
//
//typedef enum      // for IvrProviderEQ
//{
//  eStsOk     = 0,
//  eStsRetry  = 1,
//  eStsReject = 2
//} SipIvrEQsts;
//
//
//
//// ---------------------------------------
//// FeatureStatus
//typedef struct
//{
//  int bToDo;      // 0: No, 1: Yes
//  int bStarted;   // 0: No, 1: Yes
//  int bFinished;  // 0: No, 1: Yes,OK, 2: Yes,Error
//} SFeatureStatus;
//
//
//////////////////////////////////////////////////////////////////////////////
////                        CIvrCntl
//////////////////////////////////////////////////////////////////////////////
//class CIvrCntl : public CStateMachine
//{
//  CLASS_TYPE_1(CIvrCntl, CStateMachine)
//
//public:
//                      CIvrCntl(CTaskApp* pOwnerTask,
//                               const CIVRService& pIvrService,
//                               const char* pConfPassword,
//                               const char* pConfLeaderPassword,
//                               WORD startConfReqLeader,
//                               const DWORD dwMonitorConfId,
//                               const char* pNumericId = NULL,
//                               WORD IsPartyInCascadeEQ = 0,
//                               eGatewayPartyType gatewayPartyType = eRegularPartyNoGateway,
//                               BYTE bNoVideRsrcForVideoParty = FALSE /*, CConf* pCConf = NULL*/);
//
//                     ~CIvrCntl();
//
//  virtual const char* NameOf() const                                              { return "CIvrCntl";}
//  virtual void*       GetMessageMap()                                             { return (void*)m_msgEntries; }
//
//  // more global functions
//  void                Create(CParty* pParty, COsQueue* pConfRcvMbx);
//  void                Start(CSegment* pParam = NULL);
//  void                Restart();
//  void                SetBoardID(WORD boardId);
//  const char*         GetLeaderPassword() const;
//  WORD          	  IsLeaderReqForStartConf()                                   { return m_startConfReqLeader; }
//  void                SetLeader(WORD bIsLeader);
//  void                UpdateConfApi(COsQueue* pConfRcv);
//  DWORD         	  GetMonitorConfId(void) const                                { return m_dwMonitorConfId; }
//  void                HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
//
//
//  // action functions
//  void                OnEndFeature(CSegment* pParam);
//  void                OnEndFeatureNotActive(CSegment* pParam);
//  void                OnDtmfOpcode(CSegment* pParam);
//  void                OnPartyInviteResultInd( CSegment* pParam );
//  void                OnErrorFeature(CSegment* pParam);
//  void                OnIvrReset(CSegment* pParam);
//  void                OnTimerSendfExt(CSegment* pParam);
//  void                OnDtmfOpcodeNotActive(CSegment* pParam);
//
//public:
//  void                SetLanguage(WORD language);
//  void                SetStartNextFeature();
//  BYTE                IsEmptyConf();
//  void                SetIsLeader(WORD bUpdateParty, WORD bUpdateDB, BYTE bUpdateDTMFCollector = 0);
//  CDtmfCollector*     GetDtmfCollector() const                                    { return m_pDtmfCollector;}
//
//  void                SetMonitorPartyID(DWORD monitorPartyId)                     { m_monitorPartyId = monitorPartyId; }
//  void                SetPartyRsrcID(DWORD rsrcPartyId)                           { m_rsrcPartyId = rsrcPartyId; }
//  void                MuteParty(CConfApi* pConfApi, CTaskApp* pParty, BYTE bPlayMessage = YES);
//  void                UnMuteParty(CConfApi* pConfApi, CTaskApp* pParty);
//  void                OverrideMuteAll(CConfApi* pConfApi);
//  void                IncreaseVolume(CConfApi* pConfApi, BYTE bVolumeInOut = 0);  // 0 = Out, 1 = In
//  void                DecreaseVolume(CConfApi* pConfApi, BYTE bVolumeInOut = 0);  // 0 = Out, 1 = In
//
//  void                ResetIvr();
//  void                StopIVR();
//
//  void                MuteAllButX(CConfApi* pConfApi, BYTE yesNo);
//  void                LockConference(CConfApi* pConfApi, BYTE yesNo);
//  void                ConferenceOnHold(CConfApi* pConfApi, BYTE yesNo)            { }
//
//  void                BillingCodeToCDR(const char* billingCode);
//
//
//  CIVRService*        GetIVRService(void)                                         { return m_pIvrService;} // Add
//  void                StartDialOut();
//  void                InviteParty(CConfApi* pConfApi /*, BYTE byteEnable*/);
//  void                ReInviteParty(CConfApi *pConfApi, BOOL bIsGwInvite);
//  void                PlayBusyMsg(CConfApi *pConfApi);
//  void                PlayNoAnswerMsg(CConfApi *pConfApi);
//  void                PlayWrongNumberMsg(CConfApi *pConfApi);
//
//  void                SetInvitePartyFeature();
//  void                SetInvitePartyType(BYTE type)                               { m_invitePartyType = type; }
//  void                SetInvitePartyAddress(const char* sPartyAddress);
//  const char*         GetInvitePartyAddress()                                     { return m_invitePartyAddress; }
//
//
//  // Secure mode
//  void                SecureConf(CConfApi* pConfApi, BYTE SecureFlag);
//  void                ChangePwds();
//  void                SetChangePasswordFeature();
//  void                SetChangeConfPasswordFeature();
//  void                SetChangeLeaderPasswordFeature();
//  void                SetChangePasswordFailed();
//virtual  BYTE                GetChangePasswordType()                                     { return m_ChangePWDType; }
//  void                SetChangePasswordType(BYTE type)                            { m_ChangePWDType = type; }
//
//  const char*         GetNewPassword()                                            { return m_new_password; }
//  void                SetNewPassword(const char* pass);
//  void                ReviewNames(CConfApi* pConfApi);
//  void                StopNamesReviewing(CConfApi* pConfApi);
//  void                EnableRollCall(CConfApi* pConfApi, BYTE byteEnable);
//  void                SendGeneralMessageMENU();
//
//  void                SendDtmfForwarding(WORD wEventOpCode);
//
//  void                SendRecordingLinkControlDTMF(WORD wEventOpCode);
//  void                ShowParticipants(CConfApi* pConfApi);
//  void                RequestToSpeak(CConfApi* pConfApi);
//  void                ShowGathering();
//  void                StartPcm();
//
//  // Toggle dtmf functions
//  BYTE                IsToggleSelfMute()                                          { return m_bIsToggleSelfMute; }
//  void                SetToggleSelfMute(BYTE bIsToggleSelfMute)                   { m_bIsToggleSelfMute = bIsToggleSelfMute; }
//  BYTE                IsToggleSecuredConference()                                 { return m_bIsToggleSecuredConference; }
//  void                SetToggleSecuredConference(BYTE bIsToggleSecuredConference) { m_bIsToggleSecuredConference = bIsToggleSecuredConference; }
//  BYTE                IsToggleMuteAllButMe()                                      { return m_bIsToggleMuteAllButMe; }
//  void                SetToggleMuteAllButMe(BYTE bIsToggleMuteAllButMe)           { m_bIsToggleMuteAllButMe = bIsToggleMuteAllButMe; }
//
//  void                MovePartyToInConf();
//  void                ChangeToLeader();
//  void                CancelSetLeader();
//  void                ActivateIvrMuteNoisyLine();
//  void                GetGeneralSystemMsgsParams(WORD event_op_code, char* msgFullPath, WORD* msgDuration, WORD* msgCheckSum);
//
//  void                PartyNetDisconnected(WORD initiator);
//  void                KeepNetDisconnect();
//  void                Start_PLC();
//  void                StartVenus();
//  void                SetCascadeLinkInConf()                                      { m_cascadeLink = CASCADE_LINK_IN_CONF; }
//  DWORD               GetStage()                                                  { return m_stage; }
//  int                 IsPSTNCall();
//  WORD                IsRecordingLinkParty();
//  void                DoRecordingAction(CConfApi* pConfApi, WORD opcode);
//  void                PrepareForwardDtmfOpcodesTable(CDTMFCodeList* pCDTMFCodeList);
//  bool                GetTurnOffIvrAfterMove()                                    {return m_turnOffIvrAfterMove;}
//  void                SetTurnOffIvrAfterMove(bool turnOffIvrAfterMove)            {m_turnOffIvrAfterMove = turnOffIvrAfterMove;}
//  WORD                IsOperatorParty();
//  BOOL                IsTIPSlaveParty();
//
//  WORD                OperatorAssistance(CConfApi* pConfApi, DWORD request_type);
//
//  eGatewayPartyType   GetGatewayPartyType()                                       { return m_GatewayPartyType;}
//  void                SetGwDTMFForwarding(BYTE val);
//  void                SetInvitePartyDTMFForwarding(BYTE val);
//  void                onInvitePartyDTMFForwardingTout();
//
//
//  void                PcmConnected();
//  void                PcmDisconnected();
//
//  void                HandleDtmfForwarding(CSegment* pParam);
//  void                SetLinkParty();
//
//  void                setNoVideRsrcForVideoParty(BYTE mbNoVideRsrcForVideoParty);
//  BYTE                getoVideRsrcForVideoParty()                                 {return m_bNoVideRsrcForVideoParty;}
//
//  BOOL                IsIvrProviderEQPartyInConf();
//  void                SipConfNIDConfirmationInd(DWORD sts); // for IvrProviderEQ
//
//  void                ResetFeaturesList();
//
//  static const char*  DtmfCodeToString(WORD opcode);
//  static const char*  IvrStageToString(WORD stage);
//
//protected:
//  CIvrSubBaseSM*      StartNextFeature();
//  void                StartNewFeature();
//  void                GetFeaturesToDo();
//  void                OnStartIVR();
//  void                GetHelpMessageParams(WORD* messageID, WORD* duration);
//  void                OnConfTerminate(CConfApi* pConfApi, WORD IsDtmfForwarding = 0);
//  void                RemoveLeaderPassword();
//  WORD                GetMsgDuration(const CIVRFeature* feature, const WORD event_opcode);
//  int                 NeedExtension();
//  WORD                IsCascadeLinkPartyInConf();
//  void                GetNumbersLen(int len, int* numLen, int* pLen);
//  void                InformFeatureDTMF();
//  int                 IsEnableDtmfForwarding();
//
//  void                BlipOnCascadeLink();
//  void                OnDtmfOpcodeImp(CSegment* pParam);
//
//protected:
//  CParty*             m_pParty;
//  CDtmfCollector*     m_pDtmfCollector;
//  CIvrSubBaseSM*      m_pIvrSubGenSM;
//  CConfApi*           m_pConfApi;
//  SFeatureStatus      m_featuresList[NUM_OF_FEATURES+1];
//  DWORD               m_stage;
//  WORD                m_language;
//  DWORD               m_startNextFeature;
//  const char*         m_pConfPassword;
//  const char*         m_pNumericConferenceId;
//  CIVRService*        m_pIvrService;
//  DWORD               m_monitorPartyId;
//  DWORD               m_rsrcPartyId;
//  char*               m_leaderPassword;
//  WORD                m_startConfReqLeader;
//  WORD                m_bIsLeaderFound;
//  DWORD               m_dwMonitorConfId;
//  BYTE                m_ChangePWDType;
//  BYTE                m_invitePartyType;
//  char                m_new_password[CONFERENCE_ENTRY_PASSWORD_LEN];
//  BYTE                m_bIsToggleSelfMute;
//  BYTE                m_bIsToggleLockConference;
//  BYTE                m_bIsToggleConferenceOnHold;
//  BYTE                m_bIsToggleSecuredConference;
//  BYTE                m_bIsToggleMuteAllButMe;
//  char*               m_dtmfForExt;
//  WORD                m_initExtensionTimer;
//  WORD                m_pauseAtTheEnd;
//  BYTE                m_updateDuringConf;   // Indication of an online change, used with "Change Password" and "Change to Leader"
//  DWORD               m_featureActionDuringConf;
//  WORD                m_cascadeLink;
//  BOOL                m_bEnableExternalDB;
//  bool                m_turnOffIvrAfterMove;
//  eGatewayPartyType   m_GatewayPartyType;
//  BYTE                m_bNoVideRsrcForVideoParty;
//  char                m_invitePartyAddress[PARTY_ADDRESS_LEN];
//
//  PDECLAR_MESSAGE_MAP
//};
//
//#endif  // __IVRCNTL_H__

