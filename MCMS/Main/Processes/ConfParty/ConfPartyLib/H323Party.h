//+========================================================================+
//                            H323PART.H                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323PART.H                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 6/20/99     |                                                     |
//+========================================================================+

#ifndef _H323PARTY
#define _H323PARTY

#include "H323Scm.h"
#include "H323Util.h"
#include "H323NetSetup.h"
#include "H323Control.h"
#include "Party.h"
#include "TokenMsgMngr.h"
#include "H323TransUpgradeToMixed.h"


// defined compilation flags in H323party files
// __H239__
// __VCU_TOUT__
// __TCall__
// __Resources__
// __AUTHENTICATION__


class CH323Cntl;
class CCapH323;
class CTerminalNumberingManager;

#define PARTY_CHANGEVIDEO_TIME  PARTYCNTL_CHANGEVIDEO_TIME - 2*SECOND //defined in h file, since h323cntl uses it.
#define VSW_FLOW_CONTROL_RATE_THRESHOLD 0.50  // Percent from original video rate

const WORD   PARTY_ALLOCATE_TRANSLATOR_ARTS	= 12;
const WORD   PARTY_TRANSLATOR_ARTS_DISCONNECTING = 13;
const WORD   PARTY_DEESCALATING = 14;

typedef enum
{
	// No active transaction:
	kTransNone = 0,
	kTransUpgradeAvcOnlyToMixReq,
} ETransactionType;

class CH323Party : public CParty
{
	CLASS_TYPE_1(CH323Party, CParty)
public:

	// Constructors
	CH323Party();
	virtual ~CH323Party();
	virtual const char* NameOf() const { return "CH323Party";}

	// Initializations
	void Create(CSegment& appParam);
	// Operations
	void  InitTask();

//	void HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode);
	BOOL  DispatchEvent(OPCODE event,CSegment* pParam);

    BYTE GetAudioRate() const { return m_AudioRate ; }
	DWORD GetVideoRate() const { return  m_videoRate ; }
	void SetVideoRate(DWORD newrate){m_videoRate = newrate; };

	DWORD GetContentRate() const { return m_contentRate ; }
	void  SetInitialTotalVideoRate (DWORD totalVideoRate) {m_initialTotalVideoRate = totalVideoRate;};
	DWORD GetInitialTotalVideoRate() const {return m_initialTotalVideoRate;}
/*	DWORD GetContTdmRate() const { return m_contTdmRate ; }
	char* GetServiceName()  { return m_serviceName; }
*/	void  UpdateVideoRate(int newVidRate);
	void  UpdateContentRate(int newContRate);
/*	void  UpdateContTdmRate(DWORD newContTdmRate);
*/
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	virtual void  OnPartyRemoteH230(CSegment* pParam);
	const char* GetPartyStateAsString(int PartyStateNumber);

	void  H323VideoMute(CSegment* pParam);
	BYTE  H323EndChannelConnect(CSegment* pParam, BYTE* pIsLateReleaseResourcesInConfCntl);
	void  H323UpdateBaudRate();

	// Action functions
	void  OnH323LogicalChannelUpdateDB(DWORD channelType, DWORD vendorType);
	void  OnH323LogicalChannelUpdate(CSegment* pParam);
	void  OnH323LogicalChannelConnect(CSegment* pParam);
	void  OnH323LogicalChannelDisConnect(CSegment* pParam);
	void  OnH323LogicalChannelDisConnectChangeMode(CSegment* pParam);
	void  OnH323GateKeeperStatus(CSegment* pParam);
	void  OnH323PartyMonitoring(CSegment* pParam);
	void  OnH323Rmt323ComModUpdateDB(CSegment *pParam);
/*	void  OnH323EndContentReConnect(CSegment* pParam);

//	void  OnH323H230SetUp(CSegment* pParam);
//	void  OnH323H230ChangeMode(CSegment* pParam);
//	void  OnH323H230Connect(CSegment* pParam);
*/
	void  OnH323VideoMuteSetup(CSegment* pParam);
	void  OnH323VideoMuteConnect(CSegment* pParam);
	void  OnH323VideoMuteChangeMode(CSegment* pParam);
/*
	void  OnConfStreamOffMediaChannel(CSegment* pParam);
	void  OnConfStreamOnMediaChannel(CSegment* pParam);
	void  OnH323RemoteCapTransferToConferenceSetup(CSegment* pParam);
	void  OnGWConfSendNewCapToRemoteEP(CSegment* pParam);
	void  OnH323CallConnectedIndicationSetup(CSegment* pParam);

    void  OnH323RmtCISetup(CSegment* pParam);
    void  OnH323RmtCIChangeMode(CSegment* pParam);
    void  OnH323RmtCIConnect(CSegment* pParam);
*/	void  SendCloseChannelToConfLevel(CSegment* pParam);
    void  OnH323DisconectChannelDisconnecting(CSegment* pParam);


/*
	virtual void  OnConfSetService(CSegment* pParam);

	BYTE  ServiceTypeToSpanType (const BYTE serviceType);

	virtual BYTE CheckRequestIndex(CSegment* pMsg);

	// disconnect functions ( channels, call, ...)
*/	void  OnConfDisconnectMediaChannel(CSegment* pParam);
//	void  OnH323PartyDisconnectMmlp(CSegment* pParam);*/
	void  OnH323EndChannelConnectSetupOrConnect(CSegment* pParam);
	void  OnH323EndChannelConnectChangeMode(CSegment* pParam);
    void  H323LogicalChannelDisConnect(DWORD channelType, WORD dataType, WORD bTransmitting, BYTE roleLabel, BYTE bUpdateCommMode);



	void  OnConfDeActiveMediaChannel(CSegment* pParam);
    WORD  AllocateMcuNumber();
    // MVC video force events
 /*   virtual void OnVidBrdgVisualMVCAcceptConnect(CSegment* pParam);
    virtual void OnVidBrdgVisualMVCRejectConnect(CSegment* pParam);

    // NonStandard command events
    void OnH323NonStandardInd(CSegment* pParam);
	//DBC2
	void OnH323DBC2Command(CSegment* pParam);
	void OnConfDBC2EpInd(CSegment* pParam);
	void OnConfDBC2RtpInd(CSegment* pParam);
*/
    // Brdg events
	void  OnVidBrdgRefresh(CSegment* pParam);
    void  OnContentBrdgRefreshVideo(CSegment* pParam);
/*	void  OnAudBrdgValidation(CSegment* pParam);

//	void  OnMlpBrdgValidation(CSegment* pParam);
*/
	//FECC brdg
	void  OnBridgeNumberingMessage(CSegment* pParam);
//	void  OnH323PartAssignTerminalNum(CSegment* pParam); //in cascade*/
//	void  OnH323FeccControlConnection(CSegment* pParam);

	//data token
	void  OnFeccBridgeTokenRequest(CSegment* pParam);
	void  OnFeccBridgeTokenAccept(CSegment* pParam);
	void  OnFeccBridgeTokenReject(CSegment* pParam);
	void  OnFeccBridgeTokenWithdraw(CSegment* pParam);
	void  OnFeccBridgeTokenReleaseRequest(CSegment* pParam);
	void  OnFeccBridgeTokenRelease(CSegment* pParam);
	void  OnIpDataTokenMsg(CSegment* pParam);
	void  OnIpFeccKeyMsg(CSegment* pParam);
	void  OnConfSetCapsAccordingToNewAllocation(CSegment* pParam);

	void  IncreaseDisconnctingTimerInPartyCntl();
/*
	// audio only - fot internal use
	int IsAudioOnly()const { return m_voice != 0;};


	// AutoDetection events

	// Action function utils
    void  OnH323RmtCI(CSegment* pParam);*/
	void  SendAddedProtocolToConfLevel(CSegment* pParam);
	void  SendRemovedProtocolToConfLevel(CSegment* pParam);
	void  UpdateLocalCapsInConfLevel(CSegment* pParam);
	void  UpdatePartyH323VideoBitRate(CSegment *pParam);
/*	void  SendNewVideoRateToConflevel(CSegment* pParam);
	void  SendFlowControlPartyToConflevel(CSegment* pParam);
	void  IsPossibleToChangeCaps(CSegment* pParam);

*/	void  SendFlowControlToCs(CSegment* pParam);
    void  OnRemoveSelfFlowControlConstraint(CSegment* pParam);
	void  OnH323DTMFInd(CSegment* pParam);
	void  SendSiteAndVisualNamePlusProductIdToPartyControl(CSegment* pParam);
	void  SetPartySecondaryCause(CSegment* pParam);
    void  ReActivateIncomingChannelIfNeeded();

	//Party monitoring
	virtual void  OnMcuMngrPartyMonitoringReq(CSegment* pParam);
/*
	WORD  GetMaxOfPmReq(CTaskApp *pTaskApp,int *pTotalLoad);

	// Sofware CP functions
	void  OnVidBrdgSetSofwareCPStreamConnect(CSegment* pParam);
	void  OnH323SoftwareCPPartyVideoUpdate(CSegment* pParam);

*/
	BYTE  IsIpOnlyConf() {return m_isIpOnlyConf;};
	BYTE  IsCallGeneratorParty(){return m_isCallGeneratorParty;}; // for Call Generator
	// EPC

//	void  OnConfContentTokenMessage(CSegment* pParam);
/*	void  OnConfContentMasterRateChange(CSegment* pParam);*/
//	void  OnConfContentRateChange(CSegment* pParam);
//	void  SendMediaProducerStatusToConfLevel(CSegment* pParam);
	void  SendEndChangeContentToConfLevel(CSegment* pParam);

	// H239
	void  SpreadAllH239Msgs(CSegment* pParam,EMsgDirection direction,WORD isFirstMsgAfterUpdate = 0, DWORD OpCode = 0,EMsgStatus msgStat = eMsgFree);
	BYTE  IsH239Conf()		    const {return (m_confDualStreamMode == confTypeH239);}
	DWORD GetConfDualStreamMode() const {return m_confDualStreamMode;}
	void  SendStreamStateToContentBrdg(eContentState eContentInState) {}
	void  OnRtpAckForContentOnOff(CSegment* pParam);
	void  OnRtpAckForContentTout(CSegment* pParam);
    void  OnContentMsgFromMaster (CSegment* pParam);
    void  OnContentRateChangeDone (CSegment* pParam);
	EHwStreamState SetCorrectStreamStateForTMM();
	void HandleTMMList(CTokenMsgMngr* tokenMsgList);
	void DisconnectPartyDueToProblemsWithH239RtpStream();
	void HandleRtpProblemsDuringClosingContentStream();
	void OnRtpAckForEvacuateTout(CSegment* pParam);
	void OnRtpAckForEvacuateContentStream(CSegment* pParam);
	void OnRtpAckForContentOnOffWhileDisconnecting(CSegment* pParam);

    //Party Control Events
    void ConfUpdatedCapsAndAudioRate(CSegment* pParam);

	//ECS:
	void  SendECSToPartyControl();
	BYTE  IsRemoteCapsEcs() {return m_isECS;}
	void  ResetEcs()		{m_isECS = FALSE;}
/*	//Highest Common:
*/	BYTE  IsPartyInChangeVideoMode()   const;
/*	BYTE  IsPartyInChangeCopMode()     const;
	BYTE  IsPartyInChangeCopModeOfOutgoing() const;
*/	BYTE  IsPartyInChangeCpVideoMode() const;
	BYTE  IsChangeIncomingState() const;
//	BYTE  IsPartyInConfOnPort() const {return ((m_videoPlusType == eSINGLE_PORT) || (m_videoPlusType == eDUAL_PORT));}
	eChangeModeState GetChangeModeState() const {return m_changeModeState;}
/*	BOOL  IsAvayaEnvironment() const {return m_bIsAvaya;}
	BOOL  GetAuthFlag() const {return m_bIsAuthenticated;}



protected:

*/
    void  GetIpCallIdentifiers (IP_EXT_DB_STRINGS* ipStringsStruct);
	void  OnSendFaultyMfaToPartyCntl(CSegment *pParam);
	void  OnConfEstablishH323Call(CSegment* pParam, WORD &encAlg, WORD &halfKeyAlg, CH323NetSetup* pH323NetSetup,CRsrcParams** avcToSvcTranslatorRsrcParams);
	void  OnH323UpdateGkCallId(CSegment* pParam);

	void  UpdateInitialContentModeIfNeeded();

	//Move:
	void  OnConfSetMoveParams(CSegment* pParam);
	void  OnConfExportChangeMode(CSegment* pParam);

	//Highest Common:
		//Action functions:
	void  OnConfChangeModeConnect(CSegment* pParam);
 //   void  OnConfCntlReConnectStream(CSegment* pParam);
//	void  OnConfChangeDataMode(CSegment* pParam);
	void  OnH323LogicalChannelRejectChangeMode(CSegment* pParam);
	void  OnH323StreamViolationChangeMode(CSegment* pParam);
	void  OnH323StreamViolationConnect(CSegment* pParam);
    void  OnTimerChangeModeTimeOut(CSegment* pParam);
	void  OnPartyDowngradeToSecondary(CSegment* pParam);

	//Multiple links for ITP in cascaded conference feature:
	void  UpdateConfMainLinkIsConnected();
	void  OnPartyUpdateITPSpeaker(CSegment* pParam);
	void  OnPartyUpdateITPSpeakerAck(CSegment* pParam);
	void  OnPartySendITPSpeakerAckReq(CSegment* pParam);
	void  OnVBSendITPSpeakerReq(CSegment* pParam);


		//Operations:
    void  SendReCapsToPartyControl(CSegment* pParam);
	void  EndH323Disconnect(WORD isRejected = 0);
	void  UpdateCurrentRcvModeAccordingToTarget();
    void  UpdateCurrentTxModeAccordingToTarget();
	void  SendEndChangeVideoToConfLevel(EStat eStatus = statOK,WORD reason = SECONDARY_CAUSE_DEFAULT,CSecondaryParams *pSecParamps = NULL);
    void  StartReCapsProcess();
	void  SetPartyToSecondary(WORD reason,CSecondaryParams *pSecParamps = NULL);
	BYTE  IsNewModeEqualToCurMode(CComModeH323* pNewScm);
	BYTE  IsNewContentModeEqualToCurContentMode(CComModeH323* pNewScm, cmCapDirection direction);
	void  OnPartyUpdateConfRsrcIdForInterface(CSegment* pParam);
	void  PeopleIntraRequestFiltering(WORD videoDirection=3);
	void  ContentIntraRequestFiltering();

		// Encryption
	BYTE  AllocateLocalHalfKey(WORD isDialOut);
	
		// Content
	void OnConfContentTokenMessage(CSegment* pParam);
	void TranslateTokenMessageToStandardContentForH323Cntl(OPCODE subOpcode,CSegment* pParam, BYTE isSpeakerChange);
	void SendTokenMessageToConfLevel(CSegment* pParam);
	void CallGeneratorRecieveTokenFromRemote(CSegment* pParam);	// for Call Generator
	void OnConfContentMasterRateChange(CSegment* pParam);

	void ContentRateChange(DWORD newRate, BYTE bIsSpeaker);
	void OnConfContentRateChange(CSegment* pParam);
	void OnConfContentChangeMode(CSegment* pParam);
	void OnConfContentChangeModeDisconnecting(CSegment* pParam);
	void SendMediaProducerStatusToConfLevel(CSegment* pParam);

	void OnPartyCntlUpdatePresentationOutStream(CSegment* pParam);
	void OnPartyUpdatedPresentationOutStream(CSegment* pParam);

	void  OnTimerContentSpeakerIntraRequest(CSegment* pParam);

	// LPR
	void  UpdatePartyH323LprVideoBitRate(CSegment* pParam);
	void  OnLprUpdatedPartyH323VideoBitRate(CSegment* pParam);
	void  SendLprFlowControlToCard(CSegment* pParam);
	void  SendNewContentRateToConfLevel(DWORD newContentRate, DWORD isDba);
	void  OnSendNewContentRateToConfLevel(CSegment* pParam);

	// for Call Generator
	void OnCGStartContent(CSegment* pParam);
	void OnCGStopContent(CSegment* pParam);

	void OnMrmpRtcpFirInd(CSegment* pParam);

	void  OnMcuMngrStartPartyPreviewReq(CSegment* pParam);
	void OnMcuMngrStopPartyPreviewReq(CSegment* pParam);
	void OnMcuMngrIntraPreviewReq(CSegment* pParam);
	void StopAllPreviews();
//	void RestorePreviewReqIfNeeded();
	void OnH323SendVideoMute(CSegment* pParam);

	DWORD GetSSRcIdsForAvc(int ind, cmCapDirection direction, cmCapDataType aDataType);

	void  OnSetPartyToLeader(CSegment* pParam);

	void OnConfSendRssRequest(CSegment* pParam);
    virtual BYTE IsCascadeToPolycomBridge ();
    virtual BYTE IsRemoteIsSlaveMGCWithContent();
    virtual BYTE IsRemoteIsMGC();
    virtual BYTE IsRemoteIsRVpresentation();

    // upgrade AVConly to mixed
    void UpgradeAvcToMixed(CSegment* pParam, CComModeH323* pNewScm);
    void OnPartyUpgradeToMixChannelsUpdated(CSegment* pParam);
    void OnTransEndTransactionAnycase(CSegment* pParam);
    virtual void OnPartyTranslatorArtsDisconnected(CSegment* pParam);
    virtual void OnEndH323Disconnect(CSegment* pParam) = 0;
    void OnRemoveAvcToSvcArtTranslatorAnycase(CSegment* pParam);
    void OnPartyTranslatorArtsDisconnectedDeescalating(CSegment* pParam);

	void OnTokenRecapCollisionEndAnycase(CSegment* pParam);

    // transaction functions
    void  StartTransaction(ETransactionType eTransactionType, OPCODE opcode, CSegment * pParam);
    ETransactionType EndTransaction(BYTE &bPendingTrns);
    BYTE IsActiveTransaction() const;
    char* GetTransactionTypeAsString(ETransactionType type);
	
    /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
    void SendCallDropToPartyAnycase(CSegment* pParam);
    /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/


	// Attributes
	CH323Cntl*	   m_pH323Cntl;
    BYTE           m_AudioRate;
	DWORD		   m_videoRate;  // 100 bits/sec
	DWORD          m_contentRate;// 100 bits/sec
	DWORD 		   m_initialTotalVideoRate;
	//-------------------------------------
	CCapH323 *m_pLocalCapH323;
	CCapH323 *m_pRmtCapH323;
	CComModeH323	*m_pCurrentModeH323;
	CComModeH323	*m_pInitialModeH323;	// desirable mode
	CH323NetSetup*	m_pH323NetSetup;
	char*    m_serviceName;
	BYTE     m_contentState;

	// Parameters to distingush between different types of 323 conferences:
	BYTE		   m_isIpOnlyConf;
	DWORD          m_confDualStreamMode;

	//ECS:
	BYTE           m_isECS;
	//Highest Common:
	eChangeModeState m_changeModeState;
	eChangeModeState m_changeContentModeState;

	BOOL			m_bIsAvaya;
	BOOL		    m_bIsAuthenticated;

	// H239 TMM
	CTokenMsgMngr	*m_pH239TokenMsgMngr;
//	eContentState	m_eH239StreamState;

	BYTE			m_ContentChangeModeSpeaker; // IsSpeaker value that was received from P.C in change content mode, in order to use it in the end of change content mode.
	DWORD			m_ContentChangeModeRate;    // Content rate that was received from P.C in change content mode, in order to use it in the end of change content mode.

	eContentState   m_cgContentState;		// for Call Generator
	BYTE            m_isCallGeneratorParty;	// for Call Generator
	BYTE		m_bIsPreviewVideoIn;
	BYTE		m_bIsPreviewVideoOut;

	CPartyPreviewDrv* m_RcvPreviewReqParams;
	CPartyPreviewDrv* m_TxPreviewReqParams;

	BYTE			m_bIsVideoCapEqualScm; // When true, is means that the local video caps are set to one video cap according to scm. Should not return the removed video caps when sending re-cap for content change mode.
	CTerminalNumberingManager* m_pTerminalNumberingManager;

	WORD m_num_content_intra_filtered;
	
	//Multiple links for ITP in cascaded conference feature:
    eTypeOfLinkParty        m_linkType;

    CSvcPartyIndParamsWrapper  m_SsrcIdsForAvcParty;
    CSegment*      m_pDisconnectParams;

    ETransactionType		m_eActiveTransactionType;
	CH323TransUpgradeToMixed*	m_pTransaction;

	// Token/Recap Collision Detection
	//==================================
	CSegment	m_pendedToken;
	OPCODE		m_pendedTokenOpcode;
	BYTE 		m_pendedTokenIsSpeakerChange;

	PDECLAR_MESSAGE_MAP
};

#endif /* _H323PARTY*/
