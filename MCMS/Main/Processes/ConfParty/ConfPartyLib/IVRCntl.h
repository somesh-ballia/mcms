//+========================================================================+
//                            IVRCntl.h                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IVRCntl.h                                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 25.09.2000 |                                                      |
//+========================================================================+
#ifndef __IVRCNTL_H__
#define __IVRCNTL_H__

#include "StateMachine.h"
#include "ConfPartyDefines.h"
#include "ConfPartySharedDefines.h"
#include "IVRDtmfColl.h"
#include "IVRFeatures.h"
#include "ConfApi.h"
#include "Party.h"
#include "ConfPartyOpcodes.h"
#include "OpcodesMcmsAudio.h"
#include "CommConfDB.h"
class CParty;
class CConfApi;

const WORD   NOTACTIVE		= 1;
const WORD   ACTIVE			= 2;
const WORD   WAIT_FOR_DB	= 3;


#define PRIVATE_MSG                     1
#define PUBLIC_MSG                      2
#define MAX_DTMF_CHUNK                  31


// ------- Ivr stages definition --------
#define IVR_STAGE_LANGUAGE              0
#define IVR_STAGE_NO_VIDEO_RSRC         1
#define IVR_STAGE_GENERAL_WELCOME       2
#define IVR_STAGE_NUMERIC_CONF_ID       3
#define IVR_STAGE_CONF_PASSWORD         4
#define IVR_STAGE_CONF_LEADER           5
#define IVR_STAGE_BILLING_CODE          6
#define IVR_STAGE_PIN_CODE              7
#define IVR_STAGE_CONF_WELCOME          8
#define IVR_STAGE_LOCK_SECURE           9
#define IVR_STAGE_CHANGE_PWDS_MENU      10
#define IVR_STAGE_CHANGE_CONF_PWD       11
#define IVR_STAGE_CHANGE_LEADER_PWD     12
#define IVR_STAGE_CHANGE_PWDS_CONFIRM   13
#define IVR_STAGE_CHANGE_PWDS_OK        14
#define IVR_STAGE_CHANGE_PWDS_INVALID   15
#define IVR_STAGE_ROLL_CALL             16
#define IVR_STAGE_MAX_PARTICIPANTS      17
#define IVR_STAGE_CHANGE_TO_LEADER      18
#define IVR_STAGE_RECORDING_IN_PROGRESS 19
#define IVR_STAGE_RECORDING_FAILED      20
#define IVR_STAGE_CASCADE_MCU_PARTY     21
#define IVR_STAGE_VIDEO_INVITE          22
#define IVR_STAGE_INVITE_PARTY          23
#define IVR_STAGE_INVITE_PARTY_ADDRESS  24
#define IVR_STAGE_INVITE_PARTY_MENU     25
#define IVR_STAGE_GW_REINVITE_PARTY		26
#define IVR_STAGE_DTMF_REINVITE_PARTY	27
#define IVR_STAGE_PLAY_BUSY_MSG			28
#define IVR_STAGE_PLAY_NOANSWER_MSG		29
#define IVR_STAGE_PLAY_WRONG_NUMBER_MSG 30
#define IVR_STAGE_AUDIO_EXTERNAL        31
#define IVR_STAGE_COLLECT_EXTERNAL      32
#define IVR_STAGE_WAIT                  33


// insert here, and increase the "NUM_OF_FEATURES"
#define NUM_OF_FEATURES                 34


typedef enum
{
  CASCADE_LINK_DUMMY   = 0,
  CASCADE_LINK_IN_CONF = 1,
  CASCADE_LINK_IN_EQ   = 2,
  CASCADE_LINK_MAX     = 3
} TCascadeLink;


typedef enum      // for IvrProviderEQ
{
  eStsOk     = 0,
  eStsRetry  = 1,
  eStsReject = 2
} SipIvrEQsts;

// ---------------------------------------
// FeatureStatus
typedef struct
{
  int bToDo;      // 0: No, 1: Yes
  int bStarted;   // 0: No, 1: Yes
  int bFinished;  // 0: No, 1: Yes,OK, 2: Yes,Error
} SFeatureStatus;


////////////////////////////////////////////////////////////////////////////
//                        CIvrCntl
////////////////////////////////////////////////////////////////////////////
class CIvrCntl : public CStateMachine
{
  CLASS_TYPE_1(CIvrCntl, CStateMachine)

public:
    CIvrCntl(CTaskApp* pOwnerTask,const DWORD dwMonitorConfId,const char* pNumericId = NULL);
    ~CIvrCntl();

    virtual const char*  NameOf() const             { return "CIvrCntl";}
    virtual void*        GetMessageMap()            { return (void*)m_msgEntries; }

    virtual void         Create(CParty* pParty, COsQueue* pConfRcvMbx);
    virtual  void        Start(CSegment* pParam = NULL);
    virtual  void        Resume(CSegment* pParam = NULL);

    virtual const char*  GetLeaderPassword()  {return 0;};
    virtual WORD         IsLeaderReqForStartConf() ;

    void                 UpdateConfApi(COsQueue* pConfRcv);
    DWORD         	     GetMonitorConfId(void) const   { return m_dwMonitorConfId; }
    virtual void         HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

public:
   virtual  void        SetStartNextFeature();
   virtual BYTE         IsEmptyConf();
   virtual BOOL         IsExternalIVR() { return FALSE; }
   virtual  void        SetIsLeader(WORD bUpdateParty, WORD bUpdateDB, BYTE bUpdateDTMFCollector = 0);
   CDtmfCollector*      GetDtmfCollector() const              { return m_pDtmfCollector;}

  void                  SetMonitorPartyID(DWORD monitorPartyId)                     { m_monitorPartyId = monitorPartyId; }
  DWORD                 GetMonitorPartyID()		                                  {return m_monitorPartyId;}
  void                  SetPartyRsrcID(DWORD rsrcPartyId);
  virtual  void         MuteParty(CConfApi* pConfApi, CTaskApp* pParty, BYTE bPlayMessage = YES);
  virtual  void         UnMuteParty(CConfApi* pConfApi, CTaskApp* pParty);
  virtual  void         ResetIvr();
  virtual  void         StopIVR();
  virtual  void         BillingCodeToCDR(const char* billingCode);
  virtual  void         SetInvitePartyAddress(const char* sPartyAddress);
  virtual  const char*  GetInvitePartyAddress();
  virtual  void         SetChangePasswordFeature();
  virtual void          SetChangeConfPasswordFeature();
  virtual  void         SetChangeLeaderPasswordFeature();
  virtual  void         SetChangePasswordFailed();
  virtual  BYTE         GetChangePasswordType();
  virtual  void         SetChangePasswordType(BYTE type);

  virtual  const char*  GetNewPassword();
  virtual  void         SetNewPassword(const char* pass);
  virtual  void         MovePartyToInConf();
  virtual  void         CancelSetLeader();
  virtual  void         GetGeneralSystemMsgsParams(WORD event_op_code, char* msgFullPath, WORD* msgDuration, WORD* msgCheckSum);
  virtual  void         SetCascadeLinkInConf(BOOL isCascadeLinkInConf = TRUE);
  virtual  DWORD        GetStage() { return m_stage; }
  virtual  int          IsPSTNCall();
  virtual  WORD         IsRecordingLinkParty();
  virtual  void         SetTurnOffIvrAfterMove(bool turnOffIvrAfterMove);
  virtual  void         SetGwDTMFForwarding(BYTE val);
  virtual  void         SetInvitePartyDTMFForwarding(BYTE val);
  virtual  void         onInvitePartyDTMFForwardingTout();
  virtual  void         PcmConnected();
  virtual  void         PcmDisconnected();
  virtual  void         HandleDtmfForwarding(CSegment* pParam);
  virtual  void         SetLinkParty();
  virtual  void         setNoVideRsrcForVideoParty(BYTE mbNoVideRsrcForVideoParty);
  virtual  BOOL         IsIvrProviderEQPartyInConf() {return NO;}
  virtual  void         SipConfNIDConfirmationInd(DWORD sts); // for IvrProviderEQ
  virtual  void         ResetFeaturesList();
  virtual  bool			IsConfOrLeaderPasswordRequired()const;
  virtual  void			ResetFeaturesListForCallFromGW();


  virtual void        StartNewFeature();
  virtual void        OnEndFeature(CSegment* pParam);
  void                OnStartIVR();
  virtual void        OnPlayMusic(CSegment* pParam);
  virtual void        RecivedPlayMessageAckTimer(CSegment *pSeg) {TRACEINTO;}
  virtual void        RecivedPlayMessageAck();
  virtual void        RecivedShowSlideAck();
  CIvrSubBaseSM*      GetIvrSubGenSM() { return m_pIvrSubGenSM; }
  virtual bool        IsMasterEndFeatures() { return false; } //IVR for TIP
  virtual BOOL        IsIvrOnHold() { return false; } // BRIDGE-8051 - hold.resume TIP call during IVR
  DWORD			GetConfPwdEnabled(){return m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo; }


protected:
  CParty*             m_pParty;
  CDtmfCollector*     m_pDtmfCollector;
  CIvrSubBaseSM*      m_pIvrSubGenSM;
  CConfApi*           m_pConfApi;
  SFeatureStatus      m_featuresList[NUM_OF_FEATURES+1];
  DWORD               m_stage;
  DWORD               m_startNextFeature;
  const char*         m_pNumericConferenceId;
  DWORD               m_monitorPartyId;
  DWORD               m_rsrcPartyId;
  WORD                m_startConfReqLeader;
  DWORD               m_dwMonitorConfId;
  char                m_invitePartyAddress[PARTY_ADDRESS_LEN];
  DWORD				  m_MCUproductType;
  BYTE				  m_bIsResumeAfterHold;

  PDECLAR_MESSAGE_MAP
};

#endif  // __IVRCNTL_H__


