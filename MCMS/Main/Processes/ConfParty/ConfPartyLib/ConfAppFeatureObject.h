#ifndef __CONF_APP_FEATURE_OBJECT_H__
#define __CONF_APP_FEATURE_OBJECT_H__

#include "ConfAppMngr.h"
#include "ConfAppActiveEventsList.h"
#include "StateMachine.h"
#include "IvrApiStructures.h"

#define MAX_SUB_FEATURES      2000

class CAVmsgService;
class COstrStream;

////////////////////////////////////////////////////////////////////////////
//                        CFeatureObject
////////////////////////////////////////////////////////////////////////////
class CFeatureObject : public CStateMachine
{
CLASS_TYPE_1(CFeatureObject,CStateMachine )

public:
                      CFeatureObject(CConfAppInfo* confAppInfo, CConfAppPartiesList* participants);
                     ~CFeatureObject();
  virtual const char* NameOf() const { return "CFeatureObject"; }

  virtual void*       GetMessageMap();
  virtual void        HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);

  // action functions
  void                OnTimerEndIvrMsg(CSegment* pParam);
  void                OnTimerRequestToSpeak(CSegment* pParam);

  // others
  int                 AskForObjection(TConfAppEvents feature, DWORD partyRsrcID);
  void                AskForPreAction(TConfAppEvents feature, DWORD partyRsrcID, WORD ConfOrParty);
  void                StartNewFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID);
  void                CreateNewConfFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID);
  void                StartNewConfFeature();
  void                StartNewPartyFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyID);
  void                StartMusicToConfOrParty(WORD confOrParty);
  void                StopConfFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyID);
  void                StopPartyFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyID);
  void                StopPartyFeatureWithoutNotifyCAM(DWORD opcode);

  TAppFeatures        GetFeatureType()                      { return m_featureType; } 
  DWORD               GetOpcode()                           { return m_opcode; }
  void                SetOpcode(DWORD opcode)               { m_opcode = opcode; }
  DWORD               GetPartyRscrID()                      { return m_partyRsrcID; }
  void                SetPartyRscrID(DWORD PartyRscrID)     { m_partyRsrcID = PartyRscrID; }
  WORD                GetIsLeaderParty();
  WORD                GetNumberOfMediaForRollCallReview(DWORD *fileNameArray);
  int                 DoPartyAction(DWORD opcode, DWORD partyRsrcID, DWORD action, CSegment* pParam = NULL);
  int                 DoPlcAction(DWORD opcode, DWORD action, CSegment* pParam = NULL);
  int                 DoVenusAction(DWORD opcode, DWORD action, CSegment* pParam = NULL);
  int                 PlayPartyMessage(DWORD partyRsrcID, CSegment* pParam);
  void                RecordRollCall(DWORD partyRsrcID, CSegment* pParam);
  void                StopRollCallRecording(DWORD partyRsrcID, CSegment* pParam);
  void                StopPlayMediaCommand(DWORD confOrParty, DWORD mediaType);
  void                DtmfReceived(DWORD opcode, DWORD partyRsrcID, CSegment* pParam);
  char                TranslateAudioDTMF(BYTE dtmf);
  void                EnterPartyToMix(DWORD partyRsrcID);
  void                StopIVRUponDisconnecting(DWORD confOrParty);
  void                StopSlideUponDisconnecting();
  void                StopMusicUponDisconnecting(DWORD confOrParty);
  int                 ReplaceTokenWithSeqNumIfNeeded(DWORD seqNumToken, DWORD sequenceNum);
  void                EndFeatureAndStopPlayMsgUponRequest(BOOL stopPlayMsg = TRUE);
  void                OnAckEndIvrMsg();
  void                OnEndIvrMsg();
  DWORD               GetEventUniqueNumber()                  { return m_eventUniqueNumber; }
  WORD                GetEndFeatureUponDTMF()                 { return m_endFeatureUponDTMF; }
  DWORD               GetPartyRsrcID()                        { return m_partyRsrcID; }
  DWORD               GetIvrMessageSessionIDCam()             { return m_IvrMessageSessionIDCam; }
  void                SetIvrMessageSessionIDCam(DWORD seqNum) { m_IvrMessageSessionIDCam = seqNum; }
  void                RestartConfWaitForChair();
  void                StartIVR(DWORD confOrParty, WORD startIVRMode);
  void                SendDtmf(CSegment* pParam);

  void                Dump(DWORD level);
  void                Dump(COstrStream& trace_str);

  void                SendClientDtmf(CSegment* pParam); // for Call Generator

protected:
  DWORD               GetNextIvrMessageSessionID();
  int                 PlayMessage(DWORD confOrParty, WORD isNeedStartIvr, DWORD event);
  STATUS              UpdatePlayMessageWithOneRollCallFile(SIVRPlayMessageStruct* playMsg, DWORD opcode);
  WORD                UpdatePlayMessageForRollCallReview(SIVRPlayMessageStruct* playMsg, DWORD opcode, DWORD *fileNameArray, WORD numOfMediaFiles);
  void                ShowSlide(CSegment* pParam = NULL);
  void                InitVideoSlideParams(SIVRPlayMessageStruct* pPlayMsg, const char *slideName = NULL);
  const char*         GetSlideName();
  CAVmsgService*      GetAVmsgService();

  void                StopConfWaitForChair();
  void                StopConfIVR();
  void                StopPartyWaitForChair();
  void                StopSinglePartyMusic();
  void                MovePartyAudioToMix();
  void                MovePartyVideoToMix();
  void                StartIVRForPartyIfNeeded(WORD startIVRMode,BOOL bFromTipMaster=FALSE);
  void                StartStopPlcCommand(DWORD opcode);
  void                StartStopVenusCommand(DWORD opcode);
  void                StopMusicToConfOrParty(DWORD confOrParty);
  void                StopAudioVideoIVRMode();
  void                StopIVR(DWORD confOrParty);
  void                UpdateMediaEntry(SIVRPlayMessageStruct* playMsg, WORD ind, const char* mediaName, WORD checkSum, WORD duration);
  void                UpdateSilenceEntry(SIVRPlayMessageStruct* playMsg, WORD ind, WORD silenceDuration);
  WORD                CalculateTotalPlayMsgDuration(SIVRPlayMessageStruct* playMsg);
  void                JoinConfVideo();

protected:
  WORD                m_stopUponFirstDTMF;
  WORD                m_endFeatureUponDTMF;
  TAppFeatures        m_featureType;
  DWORD               m_partyRsrcID;
  DWORD               m_eventUniqueNumber;
  WORD                m_waitListOriginated;
  DWORD               m_opcode;
  CConfAppInfo*       m_confAppInfo;
  DWORD               m_IvrMessageSessionID;
  DWORD               m_IvrMessageSessionIDCam;
  CConfAppPartiesList*m_participants;

  PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CFeatureObjectList
////////////////////////////////////////////////////////////////////////////
class CFeatureObjectList : public CStateMachine
{
CLASS_TYPE_1(CFeatureObjectList,CStateMachine)

public:
                      CFeatureObjectList(CConfAppInfo *confAppInfo, CConfAppPartiesList* participants);
                     ~CFeatureObjectList();
  virtual const char* NameOf() const { return "CFeatureObjectList"; }

  virtual void*       GetMessageMap();
  virtual void        HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);

  int                 AskForObjection(TConfAppEvents feature, DWORD partyRsrcID);
  void                AskForPreAction(TConfAppEvents feature, DWORD partyRsrcID, WORD ConfOrParty);

  int                 CreateNewConfFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID, WORD replaceOrAppend);
  void                StartNewConfFeature(int featureInd);
  void                StartNewPartyFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID, WORD confOrParty);
  void                StopNewFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID, WORD confOrParty);

  int                 DoPartyAction(DWORD opcode, DWORD partyRsrcID, DWORD action, CSegment* pParam = NULL);
  int                 PlayPartyMessage(DWORD partyRsrcID, CSegment* pParam);
  int                 StopPartyMessage(DWORD partyRsrcID, DWORD mediaType);
  void                RecordRollCall(DWORD partyRsrcID, CSegment* pParam);
  int                 DtmfReceived(DWORD opcode, DWORD partyRsrcID, CSegment* pParam);
  int                 IsFeatureObjectInList(DWORD opcode, DWORD partyRsrcID);
  void                StopRollCallRecording(DWORD partyRsrcID, CSegment* pParam);

  void                ActionsUponPartyDisconnecting(DWORD partyRsrcID, DWORD audioOrVideo);
  void                ActionsUponPartyMoving(DWORD partyRsrcID);
  void                ActionsUponLastPartyDisconnecting();
  void                DoActionForParty(DWORD opcode, DWORD partyRsrcID, DWORD action, CSegment* pParam = NULL);
  BOOL                IsPartyInIVR(DWORD partyRsrcID, DWORD audioOrVideo);
  void                RemoveAllFeaturesUponConfTerminating();
  int                 ReplaceTokenWithSequenceNum(DWORD seqNumToken, DWORD sequenceNum);
  int                 HandlePlayMsgAckInd(DWORD ack_seq_num);
  void                HandleChangeIC();
  void                Dump(DWORD level);
  void                DumpFeatures(const char* functionName);
  void                RemoveFeatureByOpcode(DWORD opcode);

protected:
  int                 FindIndexForNewFeature();
  int                 AddFeature(int index);
  void                OnStopFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyID, WORD confOrParty);
  void                StopPartiesWaitForChair(DWORD eventUniqueNumber);
  int                 FindFeature(WORD findType, DWORD param, DWORD param2 = 0);
  int                 AddFeatureToHead();
  int                 AddFeatureToTail();
  int                 AddFeatureAfterCurrent();
  STATUS              RemoveFeature(int index);
  void                MoveFirstFeatureToEnd();

  void                OnTimerStopWaitForChairDelay(CSegment* pParam);
  void                OnTimerChairDropped(CSegment* pParam);

public:
  WORD                m_numOfFeatures;
  CConfAppInfo*       m_confAppInfo;
  CConfAppPartiesList*m_participants;

protected:
  CFeatureObject*     m_list[MAX_SUB_FEATURES];

  PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CConfAppFeatureObject
////////////////////////////////////////////////////////////////////////////
class CConfAppFeatureObject : public CStateMachine
{
  CLASS_TYPE_1(CConfAppFeatureObject,CStateMachine)

public:
                      CConfAppFeatureObject(TAppFeatures featureCode);
                     ~CConfAppFeatureObject();
  virtual const char* NameOf() const { return "CConfAppFeatureObject"; }

  virtual int         GetIVRMessageID(DWORD messageID, DWORD& subInd);
  virtual int         AskForObjection(TConfAppEvents feature, DWORD partyRsrcID) = 0;
  virtual void        AskForPreAction(TConfAppEvents feature, DWORD partyRsrcID) = 0;
  virtual void        PlayMessage(CSegment* pParam) = 0;
  virtual void        HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

protected:
  CConfAppInfo*       m_confAppInfo;
  CConfAppPartiesList*m_participants;
  TAppFeatures        m_featureType;
  WORD                m_featureState;
  DWORD               m_partyRsrcID;
};


////////////////////////////////////////////////////////////////////////////
//                        CConfAppFeatureConfIVR
////////////////////////////////////////////////////////////////////////////
class CConfAppFeatureConfIVR : public CConfAppFeatureObject
{
public:
                      CConfAppFeatureConfIVR(CConfAppInfo *confAppInfo, CConfAppPartiesList* participants);
                     ~CConfAppFeatureConfIVR();
  virtual const char* NameOf() const { return "CConfAppFeatureConfIVR"; }

  int                 GetIVRMessageID(DWORD messageID, DWORD& subInd);
  int                 AskForObjection(TConfAppEvents feature, DWORD partyRsrcID);
  void                AskForPreAction(TConfAppEvents feature, DWORD partyRsrcID);

  void                PlayMessage(CSegment* pParam);
  void                StartConfNewFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID);
  void                StopConfNewFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID);

  CFeatureObjectList* GetFeaturesList() { return m_features; }
  void                RemoveAllFeaturesUponConfTerminating();
  int                 ReplaceTokenWithSequenceNum(DWORD seqNumToken, DWORD sequenceNum);
  int                 HandlePlayMsgAckInd(DWORD ack_seq_num);
  void                HandleChangeIC();
  BOOL                IsAlwaysForwardDtmfInGWSessionToIsdn();
  BOOL                IsISDNParty(DWORD partyRsrcID);
  void                DoConfAction(DWORD opcode, DWORD partyRsrcID, CSegment *pSeg = NULL);

  virtual void*       GetMessageMap();
  virtual void        HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);
  void                OnTimerPlayIVRMuteAllMsgDelay(CSegment* pParam);

protected:
  void                MuteAll(WORD onOff, DWORD partyRsrcID, WORD isIVRRequest = FALSE , WORD isForMuteAllButLecture = TRUE, BYTE isMuteAllButLeader = FALSE);
  void                SecureConf(WORD onOff, DWORD partyRsrcID);
  void                ShowParticipants(DWORD partyRsrcID);
  void                ShowGathering(DWORD partyRsrcID);
  void                ForwardDTMF(DWORD partyRsrcID, CSegment *pSeg);

public:
  CFeatureObjectList* m_features;

  PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CConfAppFeaturePartyIVR
////////////////////////////////////////////////////////////////////////////
class CConfAppFeaturePartyIVR : public CConfAppFeatureObject
{
public:
                      CConfAppFeaturePartyIVR(CConfAppInfo *confAppInfo, CConfAppPartiesList* participants);
                     ~CConfAppFeaturePartyIVR();
  virtual const char* NameOf() const { return "CConfAppFeaturePartyIVR"; }

  int                 GetIVRMessageID(DWORD messageID, DWORD& subInd);
  int                 AskForObjection(TConfAppEvents feature, DWORD partyRsrcID);
  void                AskForPreAction(TConfAppEvents feature, DWORD partyRsrcID);

  void                PlayMessage(CSegment* pParam);
  void                StopMessage(CSegment* pParam);
  void                RecordRollCall(CSegment* pParam);
  void                StopRollCallRecording(CSegment* pParam);
  void 				  StopRollCallRecordingAck(DWORD PartyId, DWORD status);

  void                StartPartyNewFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID);
  void                StopPartyNewFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID);
  int                 DoPartyAction(DWORD opcode, DWORD partyRsrcID, DWORD action, CSegment* pParam = NULL);

  void                StartIVR(DWORD partyRsrcID);
  void                EnterPartyToMix(DWORD partyRsrcID);
  void                DtmfReceived(DWORD opcode, CSegment* pParam);

  void                RollCallRecorded(CSegment* pParam);
  void                DoActionForParty(DWORD opcode, PartyRsrcID PartyId, DWORD action, CSegment* pParam = NULL);

  CFeatureObjectList* GetFeaturesList() { return m_features; }

  void                RemoveAllFeaturesUponConfTerminating();
  int                 ReplaceTokenWithSequenceNum(DWORD seqNumToken, DWORD sequenceNum);
  int                 HandlePlayMsgAckInd(DWORD ack_seq_num);

  void                RemoveFeatureByOpcode(DWORD opcode);

public:
  CFeatureObjectList* m_features;
};

#endif	// __CONF_APP_FEATURE_OBJECT_H__
