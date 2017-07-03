#if !defined(_Lobby_H__)
#define _Lobby_H__

#include "DataTypes.h"
#include "Reception.h"
#include "ReceptionList.h"
#include "SipNetSetup.h"
#include "SipStructures.h"
#include "SipCsInd.h"
#include "DelayedH323Call.h"

extern "C" void LobbyEntryPoint(void* appParam);

// timer size
const DWORD IDENT_TOUT             = 300*SECOND;

// timer events
const WORD  TRANSFERTOUT           = 4030;
const WORD  IDENTTOUT              = 4031;

// lobby reception states
const WORD  IDENT                  = 0;
const WORD  TRANSFER               = 1;
const WORD  REJECT                 = 2;
const WORD  SUSPEND                = 3;
const WORD  GK_LCF                 = 4;

const WORD  INITIAL                = 0;   // channel type
const WORD  MUXIDENT               = 1;
const WORD  NETIDENT               = 0;
const WORD  BOND                   = 1;   // party type , 0 - no bonding

const WORD  U_LAW                  = 2;
const DWORD B_Channel_Bandwidth    = 64000;
const DWORD FasBusBandwidth        = 1600;
const DWORD MaxCallType            = 10;  // 4
const WORD  TimerForUnreservedCall = 100;
const WORD  LCF_SetupIntervalTout  = 30 * SECOND;

#define TIMER_LCF_SETUP 100

typedef enum
{
  Status_OK               = 1,
  Status_RejectCall       = 2,
  Status_InvalidArguments = 3
} IdentificationStatus;

typedef enum
{
  MsAdhoc_CreateFromFactory = 0,
  MsAdhoc_ExistingConf      = 1,
  MsAdhoc_ExistingSuspend   = 2,
  MsAdhoc_ConfNotFound      = 3,
  MsAdhoc_Unknown           = 4
} MsAdhocConfType;

#include "CommConf.h"
#include "H323NetSetup.h"
#include "Reception.h"
#include "ReceptionH323.h"
#include "ReceptionIVR.h"
#include "RsrcParams.h"
#include "TaskApp.h"

typedef std::vector<CDelayedH323Call*>  VEC_DELAYED_CALLS;
typedef VEC_DELAYED_CALLS::iterator VEC_DELAYED_CALLS_ITR;

typedef enum {
  kSrc,
  kDst,
  kDst2,
  kDstReqUri,
  kDstReqUri2
} eAddressType;

// Code refactor OnSipCallIn
typedef struct findConfAndPartyKeys
{
  int         numOfSrcAlias;
  CH323Alias *pPartySrcAliasList;
  int         numOfDestAlias;
  CH323Alias *pPartyDstAliasList;
  char       *pAdHocConfName;
  char       *pAdHocNID;
  //char        strMsConversationID[MS_CONVERSATION_ID_LEN];
  char        strClickToConfID[MS_CONVERSATION_ID_LEN];
} findConfAndPartyKeysSt;

/*---------------------------------------------------------------------
  this enum is used to tell the result of finding conf and party
---------------------------------------------------------------------*/
typedef enum
{
    eFindConfAndParty_PLCM_IVR_REJECT = -1000,
    eFindConfAndParty_H323_PARTY_FOUND_FOR_SIP = -6,
    eFindConfAndParty_CONF_IS_SECURED = -5,
    eFindConfAndParty_MAX_PARTIES_REACHED = -4,
    eFindConfAndParty_CONF_IS_LOCKED = -3,
    eFindConfAndParty_ONLY_CONF_FOUND = -2,
    eFindConfAndParty_CONF_NOT_FOUND = -1,
    eFindConfAndParty_OK = 0,	
    eFindConfAndParty_MEETING_ROOM_FOUND = 1,
    eFindConfAndParty_SIP_FACTORY_FOUND = 2,
    eFindConfAndParty_NOT_A_SIP_SERVICE = 20,
    eFindConfAndParty_ON_AIR_MS_ADHOC_CONF_FOUND = 30,
    eFindConfAndParty_MS_ADHOC_CONF_NOT_FOUND = 31,

    eFindConfAndPartyResultLast
} eFindConfAndPartyResult;

typedef struct findConfAndPartyResult
{
  CCommConf  				*pComConf;
  CConfParty 				*pConfParty;
  DWORD 					meetingRoomId;
  BOOL 						useTransitEQOnly;
  char						strMsAdhocName[H243_NAME_LEN];
  eFindConfAndPartyResult	eResult;
} findConfAndPartyResultSt;

////////////////////////////////////////////////////////////////////////////
//                        CLobby
////////////////////////////////////////////////////////////////////////////
class CLobby : public CTaskApp
{
  CLASS_TYPE_1(CLobby, CTaskApp)

public:
                        CLobby();
  virtual              ~CLobby();
  virtual const char*   NameOf() const      { return "CLobby";}

  const char*           GetTaskName() const { return "LOBBY"; }
  virtual void*         GetMessageMap()     { return (void*)m_msgEntries; }

  void                  Create(CSegment& appParam);
  void                  Destroy();
  virtual void          SelfKill();
  virtual int           GetTaskMbxBufferSize() const {return 256 * 1024 - 1;}

  void                  HandleEvent(CSegment* pMsg);
  BOOL                  TaskHandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
  void                  HandleCentralSignalingEvent(CSegment* pMsg, OPCODE opcode);
  void                  OnEndPartyTransfer(CReception* pLobbyTransfer);
  void                  InitTask()          { }
  BOOL                  IsSingleton() const { return YES; }
  void                  RejectCall(CNetSetup* pNetSetUp, CConfParty* pBackUpPartyReservation = NULL, WORD BackUpScenario = 0, CConfParty* pConfParty = NULL);
  void                  RemoveReception(CReception* pReception);
  void                  OnConfConfOnAir(CSegment* pParam);
  void                  OnConfMngrSuspendIVRParty(CSegment* pParam);
  void                  OnTestTimer(CSegment* pParam);             // rons

  void                  AcceptH323CallIn(CH323NetSetup* pNetSetup, CCommConf* pComConf, CConfParty* pConfParty);
  void                  AcceptSipCallIn(CSipNetSetup* pNetSetup, CCommConf* pComConf, CConfParty* pConfParty, BYTE bIsSoftCp, const sipSdpAndHeadersSt* pSdpAndHeaders);

  BYTE                  IsReceptionStillInList(CReception* pReception);
  void                  RemoveFromList(CReception* pLobbyTransfer);
  void                  SetPreDefinedIvrForTransitAdhocEqIfNeeded(char* transitEQName, CIpNetSetup* pNetSetup, const char* strToSet);
  void                  OnDelayedH323Call(CDelayedH323Call* pCall);
  BOOL		  IsRssDialinRejected(CSipNetSetup* pNetSetup, CCommConf* pComConf, const sipSdpAndHeadersSt* pSdpAndHeaders);
  BOOL		  IsRssDialinRejected(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders);
  BOOL 		 RejectRssDialin(CSipNetSetup* pNetSetup, sipSdpAndHeadersSt* pSdpAndHeaders);
  BOOL		 IsRdpGwRejected(BOOL hasSdp, CSipNetSetup* pNetSetup, CCommConf* pComConf, const sipSdpAndHeadersSt* pSdpAndHeaders);


protected:
  void                  OnNetCallIn(CSegment* pMplParam);
  void                  OnNetDisconnect(CSegment* pMplParam);
  void                  AcceptCallIn(CIsdnNetSetup* pNetSetUp, CCommConf* pComConf, CConfParty* pConfParty);
  IdentificationStatus  IdentifyParty(CIsdnNetSetup* pNetSetUp, CCommConf* pComConf, CConfParty* pConfParty, COsQueue* pConfRcvMbx, COsQueue* pLobbyRcvMbx, const char* confName, DWORD monitorConfId);

  void                  AllocTransferList();

  void                  OnH323CallIn(CSegment* pMplParam);
  void                  SuspendH323Call(CH323NetSetup* pNetSetup, DWORD ConfId, char* sH323srcPartyAddress, char* sH323destPartyAddress, char* confName);
  void                  SuspendH323CallConfExist(CH323NetSetup* pNetSetup, DWORD ConfId, char* strSrcPartyAddress, char* strDestPartyAddress, char* confName);
  void                  SuspendH323CallAndCreateMeetingRoom(CH323NetSetup* pNetSetup, DWORD confId, char* sH323srcPartyAddress, char* sH323destPartyAddress, char* mrName);
  void                  SuspendH323CallAndCreateGateWayConf(CH323NetSetup* pNetSetup, DWORD confId, char* sH323srcPartyAddress, char* sH323destPartyAddress, char* mrName, char* TargetNumber);
  void                  IdentifyH323Party(CH323NetSetup* pH323NetSetUp, CCommConf* pComConf, CConfParty* pConfParty, COsQueue* pConfRcvMbx, COsQueue* pLobbyRcvMbx, const char* confName, DWORD monitorConfId, WORD isChairEnabled);
  void                  RejectH323Call(CH323NetSetup* pH323NetSetUp, int reason);

  void                  OnSipCallIn(CSegment* pCSParam);
  void 					OnSipMsConfInvite(CSegment* pCSParam);
  void                  SuspendSipCall(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD ConfId, const char* strSrcPartyAddress, const char* strDestPartyAddress, char* confName);
  void                  SuspendSipCallConfExist(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD ConfId, const char* strSrcPartyAddress, const char* strDestPartyAddress, char* confName);
  void                  SuspendSipCallAndCreateMeetingRoom(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD confId, const char* strSrcPartyAddress, const char* strDestPartyAddress, char* confName);
  void                  SuspendSipCallAndCreateGateWayConf(CSipNetSetup* pNetSetup, const sipSdpAndHeadersSt* pSdpAndHeaders, DWORD confId, const char* strSrcPartyAddress, const char* strDestPartyAddress, char* mrName, char* dialString);
  void                  SuspendSipCallAndCreateConfFromFactory(CSipNetSetup* pNetSetup, const char* strSrcPartyAddress, const char* strDestPartyAddress, const sipSdpAndHeadersSt* pSdpAndHeaders, CCommRes* pFactory);
  void                  IdentifySipParty(CSipNetSetup* pNetSetUp, CCommConf* pComConf, CConfParty* pConfParty, COsQueue* pConfRcvMbx, COsQueue* pLobbyRcvMbx, const char* confName, DWORD confMonitorId, WORD isChairEnabled, BYTE bIsSoftCp, const sipSdpAndHeadersSt* pSdpAndHeaders);
  void                  RejectSipCall(CSipNetSetup* pNetSetUp, int reason, const sipSdpAndHeadersSt* pSdpAndHeaders, char* pAltAddress = NULL, STATUS status = 0);

  void                  SuspendIsdnCall(CIsdnNetSetup* pNetSetup, char* confName);
  void                  SuspendIsdnCallAndCreateMeetingRoom(CNetSetup* pNetSetup, DWORD confId, char* mrName);
  void                  SuspendIsdnCallAndCreateGateWayConf(CNetSetup* pNetSetup, DWORD confId, char* mrName, char* targetNumber);
  void                  SuspendIsdnCallConfExist(CIsdnNetSetup* pNetSetup, DWORD ConfId, char* confName, const char* pCalledTelNumber, const char* pCallingTelNumber);
  void                  RejectIsdnCall(CIsdnNetSetup* pNetSetup, int reason);

  void                  SuspendCall(CNetSetup* pNetSetUp, CRsrcParams* pNetDesc, DWORD ConfId);
  void                  SuspendVoiceCall(CIsdnNetSetup* pNetSetup, char* confName);
  void                  SuspendVoiceCallAndCreateMeetingRoom(CNetSetup* pNetSetUp, DWORD confId, char* mrName);
  void                  SuspendVoiceCallAndCreateGateWayConf(CNetSetup* pNetSetup, DWORD confId, char* mrName, char* targetNumber);

  int                   IdentifyCallTypeAndTargetNo(const char* sH323destPartyAddress, char** TargetNumber, char** prefixNumber);
  DWORD                 TranslateRate(BYTE rate);

  void                  SetPartyListIDForSuspendCall(const char* strSrcPartyAddress = NULL, const char* strDestPartyAddress = NULL);

  WORD                  GetSeqNum(DWORD chnlMask);
  WORD                  IsInitialConnected(DWORD chnlMask);

  void                  OnConfReleaseParty(CSegment* pParam);
  void                  OnConfMngrRejectParty(CSegment* pParam);
  void                  OnRejectParty(CSegment* pParam);
  void                  ConfMngrRejectParty(CSegment* pParam, BYTE rejectOnlyParty);

  int                   GetIpConfPartyType(const mcTransportAddress* pPartyIPaddress, CH323Alias* pSrcH323AliasArray, WORD wSrcNumAlias, CCommConf** pConf, CConfParty** pParty, CH323Alias* pDestH323AliasArray, WORD wDestNumAlias, DWORD& meetingRoomId, BYTE isH323 = TRUE, char* pAdHocConfName = NULL, WORD useTransitEQ = FALSE);
  void                  SetReceptionAsDisconnected(CRsrcDesc* pRsrcDesc);

  WORD                  SetNewDestinationConfName(CIpNetSetup* pNetSetup, char* newDestConfName, WORD IsSip = FALSE);

  void                  ParseSipHost(const char* strAddr, CH323Alias* PartyAliasList, CSipNetSetup* pNetSetup);
  void                  ParseEqFormat(char *strUser, CH323Alias* PartyAliasList, CSipNetSetup* pNetSetup, char** ppAdHocConfName, char** ppAdHocNID);
  void                  ParseSipDest(const char* strAddr, CH323Alias* PartyAliasList, CSipNetSetup* pNetSetup, char** ppAdHocConfName, char** ppAdHocNID);
  void                  ParseSipUri(eAddressType eType, const char* strAddr, findConfAndPartyKeysSt &keys, CSipNetSetup* pNetSetup);
  int                   ParseSipToPhContextUri(const char* strTo, int* pNumOfDestAlias, char* pDestPartyAddress1, int sizeDestPartyAddress1, char* pDestPartyAddress2, int sizeDestPartyAddress2);

  void                  OnSipOptions(CSegment* pCSParam);
  void                  ResponseSipOptions(mcIndOptions* pOptionsMsg, DWORD callIndex, WORD srcUnitId, int reason = -1, CCommConf* pCommConf = NULL, CConfParty* pConfParty = NULL, DWORD serviceId = 0);
  void                  CalculateSipOptionsCaps(CCommConf* pComConf, CConfParty* pConfParty, CSipCaps* pSipLocalCaps, DWORD serviceId);
  void                  SendSipOptionsResponse(mcIndOptions* pOptionsInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId, CSipCaps* pSipCaps, DWORD serviceId);

  void                  OnGkManagerSearchForCall(CSegment* pParam);

  WORD                  IsVoiceCall(const CNetSetup* pNetSetUp) const;

  int                   FindDestConf(const char* calledPhoneNumber, const char* pCallingTelNumber, char* confName, DWORD& confId, CCommConf** ppComConf);

  BYTE                  TargetReceptionAlreadyExist(char* targetConfName);

  void                  ReleaseAllConfOnAirSuspendParties(DWORD confID, char* targetConfName);
  void                  SetMsConversationIdInConfDB(DWORD confId, char* targetConfName);
  MsAdhocConfType       FindAdhocConfForMsConversationId(char* pMsConversationId, char* pAdhocName);
  void                  UpdateInitialEncryptionForParty(CSipNetSetup* pNetSetup, CConfIpParameters* pService, BOOL partyHasSdp, BOOL partyHasSDES, CCommRes* pCommRes);

  void                  SendMsgToCS(OPCODE opcode, CSegment* pseg1, DWORD serviceId);
  VEC_DELAYED_CALLS_ITR FindPosition(const CDelayedH323Call* pCall);

  // vngr-25287
  BYTE 					McCheckIfNoSdp(const sipSdpAndHeadersSt *pSdp);

  // Code refactor OnSipCallIn
  void					ExtractAndSetPerferedIpV6ScopeAddr(sipSdpAndHeadersSt *pSdpAndHeaders, CSipNetSetup *pNetSetup);
  void					BuildSipNetSetup(CSipInviteStruct *pSipInvite, CSipNetSetup *pNetSetup);

  void					AllocateAndExtractFindConfAndPartyKeys(CSipInviteStruct *pSipInvite, CSipNetSetup *pNetSetup, findConfAndPartyKeysSt &keys);
  void					FindMatchingConfAndParty(findConfAndPartyKeysSt &keys, CSipNetSetup *pNetSetup, findConfAndPartyResultSt &findResult);
  BOOL					IsServiceConfiguredAcceptCall(APIU32 csServiceId, CCommConf *pComConf, DWORD meetingRoomId, eFindConfAndPartyResult result, CH323Alias *pPartyDstAliasList);
  BOOL					IsIvrNotSupported(CSipInviteStruct *pSipInvite, CCommConf *pComConf, DWORD meetingRoomId, eFindConfAndPartyResult result);
  void					SubProcessConfAndPartyFound(CCommConf *pComConf, CConfParty *pConfParty, CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders);
  void					SubProcessConfNotFound(CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders, BOOL useTransitEQOnly);
  void					SubProcessConfFoundPartyNotFound(CCommConf *pComConf, CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders, APIU32 csServiceId);
  void					SubProcessConfFoundMSPartyNotFound(CCommConf *pComConf, CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders, APIU32 csServiceId);
  void					SubProcessConfLocked(CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders);
  void					SubProcessMaxPartiesReached(CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders);
  void					SubProcessConfSecured(CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders);
  void					SubProcessPlcmIVRReject(CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders);
  void					SubProcessMeetingRoomFound(sipSdpAndHeadersSt *pSdpAndHeaders, int numOfDestAlias, CH323Alias *pPartyDstAliasList, DWORD meetingRoomId, CSipNetSetup *pNetSetup, APIU32 csServiceId);
  void					SubProcessMeetingRoomFoundForMsParty(sipSdpAndHeadersSt *pSdpAndHeaders, int numOfDestAlias, CH323Alias *pPartyDstAliasList, DWORD meetingRoomId, CSipNetSetup *pNetSetup, APIU32 csServiceId);
  void					SubProcessSipFactory(CH323Alias *pPartyDstAliasList, CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders);
  void					SubProcessSuspendMsAdhoc(char *strMsAdhocName, CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders);
  void					SubProcessConfNotFoundForMsAdhoc(CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders);
  void					SubProcessDefault(CSipNetSetup *pNetSetup, sipSdpAndHeadersSt *pSdpAndHeaders, eFindConfAndPartyResult result);
  void					ProcessConfAndPartyFindResult(CSipInviteStruct *pSipInvite, CSipNetSetup *pNetSetup, 	findConfAndPartyKeysSt keys, findConfAndPartyResultSt &result);
  STATUS				BuildSipInviteStruct(CSegment* pCSParam, CSipInviteStruct *pSipInvite);
  void					CallFlowDecision(CSipInviteStruct *pSipInvite, CSipNetSetup *pNetSetup);
  void 					BuildSipNetSetupForMSConfInvite(CSipInviteStruct *pSipInvite,CSipNetSetup *pNetSetup);

  void 					SetTipForPartyIfNeeded(CConfParty* pConfParty , CCommConf* pCommConf, CSipNetSetup* pNetSetup);
  void 					SetAliasDisplayName(findConfAndPartyKeysSt &keys, const char* aliasName);
  void 					SetDBMSConversationId(CConfParty* pConfParty,  CSipNetSetup* pNetSetup);
  

  WORD                  m_chnlCnt;
  CIsdnNetSetup*        m_pBndNetSetup;
  CReceptionList*       m_pTransferList;              // List of CReception waiting for transfer
  CReception*           m_pLobbyTrans;                // The Current Transfered party to see if we need it ???
  CTaskApp*             m_pParyList_Id;               // Last party List ID
  VEC_DELAYED_CALLS*    m_pDelayedCalls;
  mcXmlTransportAddress m_dummyMediaIp;
  int                   gw_index;



  PDECLAR_MESSAGE_MAP
};

#endif // !defined(_Lobby_H__)
