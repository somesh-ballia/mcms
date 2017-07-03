//+===========================================================================+
//                            CONF.CPP                                       |
//            Copyright 1995 Pictel Technologies Ltd.                         |
//                   All Rights Reserved.                                     |
//----------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary        |
// information of Pictel Technologies Ltd. and is protected by law.           |
// It may not be copied or distributed in any form or medium, disclosed       |
// to third parties, reverse engineered or used in any manner without         |
// prior written authorization from Pictel Technologies Ltd.                  |
//----------------------------------------------------------------------------|
// FILE:       CONF.CPP                                                      |
// SUBSYSTEM:  MCMS                                                           |
// PROGRAMMER: Sami                                                           |
//----------------------------------------------------------------------------|
// Who | Date       | Description                                             |
//----------------------------------------------------------------------------|
//     | 7/6/95     |                                                         |
//Ori P| 16.4.01    | Voting functions									      |
//Ori P| 20.5.01	| Q&A functions	                                          |
//Zohar| 15.1.04    | Dividing Conf.cpp into 2 files.                         |
//     |            | Conf.cpp contains all and only reqular CConf functions. |
//     |            | Conf1.cpp contains all and only CConf action functions. |
//Talya| 18.5.05	| Moved to Carmel										  |
//+===========================================================================+


#include <ostream>
#include "Conf.h"
#include "ConfDef.h"
#include "IsdnNetSetup.h"
#include "SystemFunctions.h"
#include "Bridge.h"
#include "VideoBridgeInterface.h"
#include "VideoBridgeInitParams.h"
#include "AudioBridgeInterface.h"
#include "IpScm.h"
#include "H221StrCap.h"
#include "H323Caps.h"
#include "H320ComMode.h"
#include "CommModeInfo.h"
#include "ConfPartyGlobals.h"
#include "StatusesGeneral.h"
#include "SysConfig.h"
#include "H323NetSetup.h"
#include "H264.h"
#include "VideoDefines.h"
#include "Image.h"
#include "IpServiceListManager.h"
#include "ConfAppMngrInterface.h"
#include "CommResDB.h"
#include "HostCommonDefinitions.h"
#include "FECCBridgeInitParams.h"
#include "FECCBridge.h"
#include "ContentBridge.h"
#include "TerminalListManager.h"
#include "IVRServiceList.h"
#include "IVRService.h"
#include "IpCsOpcodes.h"
#include "RsrvManagerApi.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "NonStandardCaps.h"
#include "McmsPCMManager.h"
#include "McmsPCMManagerInitParams.h"
#include "COP_video_mode.h"
#include "HlogApi.h"
#include "Gathering.h"
#include "Media.h"
#include "StlUtils.h"
#include "EventPackage.h"
#include "Stopper.h"

/*Begin:added by Richer for BRIDGE-15015, 11/13/2014*/
#include "ConfigManagerOpcodes.h"
/*End:added by Richer for BRIDGE-15015, 11/13/2014*/

// test
#include "H221Str.h"
#include "H323StrCap.h"

//SIP includes
#include "SIPConfPack.h"
#include "SvcEventPackage.h"
#include "SipNetSetup.h"
#include "IpCommon.h"

#include "IpCommonDefinitions.h"
#include "IpAddressDefinitions.h"
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "SipCaps.h"
#include "SipScm.h"
#include "SIPPartyControl.h"
#include "SIPPartyControlAdd.h"
#include "NetSetup.h"
#include "IpNetSetup.h"
#include "SipNetSetup.h"
#include "SipProxyManagerApi.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsAudioCntl.h"
#include "OpcodesMcmsInternal.h"
#include "H264Util.h"
#include "ConfPartyProcess.h"
#include "IsdnPartyCntl.h"
#include "H263VideoMode.h"
#include "H263.h"
#include "ConfPartiesDialingSequence.h"
#include "TextOnScreenMngr.h"
#include "CopVideoTxModes.h"
#include "COP_ConfParty_Defs.h"
#include "OngoingConfStore.h"
#include "CdrPersistHelper.h"
#include <ctime>

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
void PutCommonHeader(CSegment** ppParam,DWORD opcode,DWORD dwSeqNumber);
void PutMsgDescHeader(CSegment** ppParam,DWORD dwSeqNumber);
void PutPortDescHeader( CSegment** ppParam,DWORD partyId,DWORD confId,BYTE nLtr1,BYTE nLtr2);
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );
extern CIpServiceListManager* GetIpServiceListMngr();
extern CCommResDB* GetpMeetingRoomDB();
extern CLobbyApi*  GetpLobbyApi();
extern BYTE GetTmpIsCopFlag();
extern const char* GetSystemCardsModeStr(eSystemCardsMode theMode);
extern std::map<DWORD, PartyMonitorID> * GetMapPartiesTasksIds();
extern const char* MediaStateToString(eConfMediaState confState);
typedef std::map<DWORD, PartyMonitorID>  TaskIdToPartyId;

// for dial in internal debug structures


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void ConfEntryPoint(void* appParam)
{
	CConf*  pConfTaskApp = new  CConf;
	pConfTaskApp->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
//  constructor
CConf::CConf()
{
	m_pPartyList                             = NULL;
	m_pCommConf                              = NULL;
	m_ConfRsrcId                             = DUMMY_CONF_ID;
	m_pAudBrdgInterface                      = NULL;
	m_pVideoBridgeInterface                  = NULL;
	m_pFECCBridge                            = NULL;
	m_pContentBridge                         = NULL;
	m_pContentXcodeBridgeInterface           = NULL;
	m_pTerminalNumberingManager              = NULL;
	m_confMediaState                         = eMediaStateEmpty;
	m_inCallCounter                          = 0;
	m_confTerminateCause                     = 0;
	m_isBridgeError                          = 0;
	m_pUnifiedComMode                        = NULL;
	m_monitorConfId                          = 0;
	m_isAudConnected                         = FALSE;
	m_isVidConnected                         = FALSE;
	m_isFeccConnected                        = FALSE;
	m_isContentConnected                     = FALSE;
	m_isContentXCodeConnected                = FALSE;
	m_isUpdateContentPending                 = FALSE;
	m_master                                 = 0;
	m_StandByStart                           = 0;
	m_IsAnyPartyWasConnected                 = FALSE;
	m_AutoTerminateBeforeFirstJoin           = 0;
	m_AutoTerminateAfterLastQuit             = 0;
	m_AccumulatedExtensionTime               = 0;
	m_TcMode                                 = 0; // VS only / Smart VS / TR only.
	m_initTimerDtmfForwarding                = 0;

	memset(m_aConfGUID, '\0', 16);

	m_pSipEventPackage                       = NULL;
	m_pSvcEventPackage                       = NULL;
	m_pConfAppMngrInterface                  = NULL;
	m_pMcmsPCMManager                        = NULL;

	// Blast handling
	m_firstParticipantConnetingTicksTime     = 0;
	m_lastPartyConnectedInTicks              = 0;
	m_numOfBlastParticipants                 = 0;
	m_firstParticipantDisconnectingTicksTime = 0;
	m_lastPartyDisconnectedInTicks           = 0;
	m_numOfBlastDisconnectingParticipants    = 0;
	m_isLastPartyConnectedAudioOnly          = NO;
	m_LastPartyInterfaceType                 = H323_INTERFACE_TYPE;

	m_CurrentIpBitRateForContentSession      = 0;
	m_isGateWay                              = FALSE;
	m_pGatheringManager                      = NULL;
	m_pOriginalVisualEffects                 = NULL;
	m_isOriginalCropping                     = YES;
	m_pInvitedDialingSequence                = new CConfPartiesDialingSequence;
	m_pDialingSequence                       = NULL;
	m_pGWPartiesState                        = NULL;
	m_pInvitedPartiesState                   = NULL;
	GwPartiesStateFlag                       = GW_SETUP;
	m_pTextOnScreenMngrForGwSession          = NULL;
	OFF(m_initialSync);
	OFF(m_startDialingOutGW);
	memset(m_IsdnDialOutServiceNameForGW, '\0', NET_SERVICE_PROVIDER_NAME_LEN);
	m_pGwDtmfForwarderConnection             = NULL;

	m_bEnableRedialOnWrongNumber             = TRUE;
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if (sysConfig)
		sysConfig->GetBOOLDataByKey("ENABLE_REDIAL_ON_WRONG_NUMBER", m_bEnableRedialOnWrongNumber);

	m_invitePartyDtmfForwardDec              = 0;
	m_isDtmfInviteParty                      = FALSE;
	m_pInvitedPartiesState                   = NULL;
	m_pTextOnScreenMngrForInvitedSession     = NULL;
	m_pDtmfInvitedPartyDialingSequence       = NULL;

	m_pCopRsrcs                              = NULL;
	m_rsrcDeallocateStatus                   = STATUS_OK;
	m_isAllocateConfResourcesFailed          = NO;
	m_lastContentRateFromMaster              = 0;
	m_pTelepresenceOnOffParams               = new TELEPRESENCE_ON_OFF_PARAMS_S();
	m_invitedPartiesStateFlag                = GW_SETUP;
	m_lastInvitedParticipantId               = DUMMY_PARTY_ID;

	m_pConfOperationPointsSet                = NULL;

	// VNGR-26449 - unencrypted conference message
	m_numOfUnencryptedParty                  = 0;
	m_contentXcodeSrcProtocol                = Video_Off;
	m_IsAsSipContentEnable                   = FALSE;
	m_AvMcuPartyRsrcId                       =  0;
	m_AudioParticipantsNumber                = 0;
	m_IsAudioParticipantsNumberNotSent       = NO;

	VALIDATEMESSAGEMAP;
}

/////////////////////////////////////////////////////////////////////////////
WORD CConf::GetVoiceType(CConfParty* pConfParty )
{
	return 1;
}
const CCommConf* CConf::GetCommConf() const
{
	return (const CCommConf*)m_pCommConf;
}

/////////////////////////////////////////////////////////////////////////////
CConf::~CConf() // constructor
{
	POBJDELETE(m_pPartyList);
	POBJDELETE(m_pVideoBridgeInterface);
	POBJDELETE(m_pContentXcodeBridgeInterface);
	POBJDELETE(m_pAudBrdgInterface);
	POBJDELETE(m_pFECCBridge);
	POBJDELETE(m_pContentBridge);
	POBJDELETE(m_pTerminalNumberingManager);
	POBJDELETE(m_pUnifiedComMode);
	POBJDELETE(m_pSipEventPackage);
	POBJDELETE(m_pSvcEventPackage);

	POBJDELETE(m_pConfAppMngrInterface);
	POBJDELETE(m_pMcmsPCMManager);
	POBJDELETE(m_pDialingSequence);
	m_PartiesInviteResults.clear();
	m_PartiesRedialNum.clear();
	if (m_pGWPartiesState)
	{
		m_pGWPartiesState->clear();
		delete m_pGWPartiesState;
	}
	if (m_pInvitedPartiesState)
	{
		m_pInvitedPartiesState->clear();
		delete m_pInvitedPartiesState;
	}

	POBJDELETE(m_pCopRsrcs);
	POBJDELETE(m_pInvitedDialingSequence);
	POBJDELETE(m_pDtmfInvitedPartyDialingSequence);
	POBJDELETE(m_pGatheringManager);
	if(m_pOriginalVisualEffects)
		POBJDELETE(m_pOriginalVisualEffects);

	POBJDELETE(m_pTelepresenceOnOffParams);
	POBJDELETE(m_pConfOperationPointsSet);
	POBJDELETE(m_pTextOnScreenMngrForGwSession);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
const char* CConf::GetTaskName() const
{
	return "CConf";
}

/////////////////////////////////////////////////////////////////////////////
std::ostream& CConf::PrintName(std::ostream& ostr)
{
	ostr << m_name;
	return ostr;
}

/////////////////////////////////////////////////////////////////////////////
void* CConf::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CConf::Create(CSegment& appParam)
{
	CTaskApp::Create(appParam/*,"CONF"*/);   // get default param  & create task

	appParam >> m_monitorConfId;
	appParam >> m_name;

	m_pGatheringManager = new CGatheringManager(this);
}

/////////////////////////////////////////////////////////////////////////////
const char* CConf::GetName() const
{
	return m_name;
}

//--------------------------------------------------------------------------
BOOL CConf::TaskHandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	switch (opCode)
	{
		case PARTY_RSRC_ID_MSG:
		{
			HandlePartyRsrcIdEvent(pMsg);
			return TRUE;
		}

		case PARTY_MSG: // party messages to party  controllers + bridge messages to  party controllers
		{
			HandlePartyEvent(pMsg);
			return TRUE;
		}

		case EXTERNAL_PARTY_MSG:
		{
			HandlePartyExternalEvent(pMsg);
			return TRUE;
		}

		case BRDG_MSG: // messages to bridge controllers
		{
			HandleBrdgEvent(pMsg);
			return TRUE;
		}

		case  VIDEO_BRIDGE_MSG: // messages to Video Bridge
		{
			if (m_pVideoBridgeInterface)
				m_pVideoBridgeInterface->HandleEvent(pMsg);
			return TRUE;
		}

		case AUDIO_BRIDGE_MSG: // messages to Audio Bridge
		{
			if (m_pAudBrdgInterface)
				m_pAudBrdgInterface->HandleEvent(pMsg);
			return TRUE;
		}

		case CONFAPP_MNGR_MSG: // messages to Conf Applications Manager
		{
			if (m_pConfAppMngrInterface)
				m_pConfAppMngrInterface->HandleEvent(pMsg);
			return TRUE;
		}

		case  PCM_MNGR_MSG: // messages to Mcms PCM Manager
		{
			OPCODE opcode;
			*pMsg >> opcode;
			if (m_pMcmsPCMManager)
				m_pMcmsPCMManager->HandleEvent(pMsg, 0, opcode);
			return TRUE;
		}

		case FECC_BRIDGE_MSG:// messages to FECC Bridge
		{
			OPCODE internalOpcode;
			*pMsg >> internalOpcode;
			if (m_pFECCBridge)
			{
				m_pFECCBridge->HandleEvent(pMsg, 0, internalOpcode);
				return TRUE;
			}
			return FALSE;
		}

		case CONTENTCNTL_MSG:// messages to CONTENT controller
		{
			OPCODE internalOpcode;
			*pMsg >> internalOpcode;
			if (m_pContentBridge)
			{
				m_pContentBridge->HandleEvent(pMsg, 0, internalOpcode);
				return TRUE;
			}
			return FALSE;
		}

		case CHAIRCNTL_MSG:// messages to CHAIR controller
		{
			TRACEINTO << "Failed, CHAIRCNTL_MSG message is not supported";
			return TRUE;
		}

		case UPDATEDB:
		{
			HandleUpdateEvent(pMsg);
			return TRUE;
		}

		case OBSERVER_UPDATE:
		{
			HandleObserverUpdateEvent(pMsg);
			return TRUE;
		}

		case XCODE_BRDG_MSG:
		{
			if (m_pContentXcodeBridgeInterface )
				m_pContentXcodeBridgeInterface->HandleEvent(pMsg);
			return TRUE;
		}

		case CONF_MPL_MSG:
		{
			HandleConfMplEvent(pMsg);
			return TRUE;
		}

		case UPDATECDR:
		{
			HandleCdrUpdateEvent(pMsg);
			return TRUE;
		}
		case MS_LYNC_SLAVES_CONTROLLER_MSG:
		{
			HandleMsLyncSlavesControllerEvent(pMsg);
			return TRUE;
		}
		case MS_LYNC_CONF_PARTY_CNTL_MSG:
		{
            HandleMsLyncConfEvent(pMsg);
			return TRUE;
		}
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
CPartyCntl* CConf::GetPartyCntl(const CTaskApp* pParty)
{
	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	return pPartyConnection ? pPartyConnection->GetPartyCntl() : NULL;
}

/////////////////////////////////////////////////////////////////////////////
CPartyCntl* CConf::GetPartyCntl(PartyRsrcID partyId)
{
	CPartyConnection* pPartyConnection = GetPartyConnection(partyId);
	return pPartyConnection ? pPartyConnection->GetPartyCntl() : NULL;
}

/////////////////////////////////////////////////////////////////////////////
CPartyCntl* CConf:: GetPartyCntl(const char* name)
{
	CPartyConnection* pPartyConnection = GetPartyConnection(name);
	return pPartyConnection ? pPartyConnection->GetPartyCntl() : NULL;
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnection* CConf::GetPartyConnection(const CTaskApp* pParty)
{
	return m_pPartyList->find(pParty);
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnection* CConf::GetPartyConnection(const char* name)
{
	return m_pPartyList->find(name);
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnection* CConf::GetPartyConnection(PartyRsrcID partyId)
{
	return m_pPartyList->find(partyId);
}

/////////////////////////////////////////////////////////////////////////////
CPartyConnection* CConf::RemovePartyConnection(const CTaskApp* pParty)
{
	CPartyConnection* pPartyConnection = m_pPartyList->remove(pParty);
	PASSERT(!pPartyConnection);
	return pPartyConnection;
}

/////////////////////////////////////////////////////////////////////////////
void CConf::InsertPartyConnection(CPartyConnection* pPartyConnection)
{
	int result = m_pPartyList->insert(pPartyConnection);
	PASSERT(result != STATUS_OK);
}


/////////////////////////////////////////////////////////////////////////////
void CConf::CleanUp()
{
	if (m_pPartyList)
	{
		WORD numParties = m_pPartyList->entries();
		if (numParties != 0)       // no active parties in conf
		{
			TRACESTRFUNC(eLevelError) << "ConfName:" << m_name << " - Non deleted active parties exists";
			PASSERT(numParties);
			PDELETE(m_pPartyList);
		}
	}

	// In Standalone Conferences Spreading and Terminate Conf Requests are not sent
	if (!m_pCommConf->GetEntryQ() && !m_isAllocateConfResourcesFailed) // in case the allocation failed we don't need to deallocate.
	{
		STATUS deallocateConfStatus = SyncRsrcAllocConfSpreading(TERMINATE_CONF_RSRC_REQ); // TODO to see if we need to set the Resource Alloc reson for terminate
		if (STATUS_OK != deallocateConfStatus)
		{
			TRACESTRFUNC(eLevelError) << "ConfName:" << m_name << " - Failed to deallocate conference resources";
			PASSERT(deallocateConfStatus);
		}
	}

	OPCODE opcode;
	if (m_pCommConf->IsMeetingRoom())
		opcode = SYNC_DEACTIVATE_MR_REQ;
	else
		opcode = SYNC_DEL_RSRV_CONF_REQ;

	STATUS deallocateConfStatus = SyncRsrcRsrvManagerConfTerminate(opcode); // TODO to see if we need to set the Resource Alloc reson for terminate
	if (STATUS_OK != deallocateConfStatus)
	{
		TRACESTRFUNC(eLevelError) << "ConfName:" << m_name << " - Failed to deallocate conference Basic resources";
		PASSERT(deallocateConfStatus);
	}

	if (!m_pCommConf->GetEntryQ())
		RemoveConfEntryFromRsrcRoutingTbl();

	m_pCommConf->EndConference(m_confTerminateCause);

	// in case of Meeting Room Conf , update the MR dB
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pCommConf = pCommConfDB->GetCurrentConf(m_monitorConfId);
	if (pCommConf)
	{
		if (pCommConf->IsMeetingRoom())
		{
			CCommRes* pMeetingRoom = 0;
			const char* mrName = pCommConf->GetName();
			if (pCommConf->GetEntryQ())
			{
				mrName = ::GetpMeetingRoomDB()->GetOrigionEqReservationName(pCommConf->GetName(), m_monitorConfId);
			}

			pMeetingRoom = ::GetpMeetingRoomDB()->GetCurrentRsrv(mrName);
			if (pMeetingRoom)
			{
				pMeetingRoom->SetMeetingRoomState(MEETING_ROOM_PASSIVE_STATE);
				::GetpMeetingRoomDB()->Update(*pMeetingRoom);
				POBJDELETE(pMeetingRoom);
			}
			else
				PASSERTMSG(NULL == pMeetingRoom, "MR is NOT found !!!");
		}

		ONGOING_CONF_STORE->Remove(pCommConf->GetMonitorConfId());
	}

	// Notify ConfPartyManager to remove the conference from TaskId-ConfId map
	CSegment* taskAndConfId = new CSegment;
	*taskAndConfId << GetTaskId();
	CManagerApi confpartyManagerApi(eProcessConfParty);
	STATUS res = confpartyManagerApi.SendMsg(taskAndConfId, CONF_DELETED_CONFERENCE);

	// Remove the Conf from the DB and delete the Queue which was allocated in the ConfPartyManager
	if (pCommConf)
	{
		COsQueue* pConfRcvMailBox = pCommConf->GetRcvMbx();
		delete pConfRcvMailBox;
		pCommConf->SetRcvMbx(NULL);
	}

	pCommConfDB->Cancel(m_monitorConfId);

	m_selfKill = 1;
	TRACEINTO << "ConfName:" << m_name << " - Self kill";
}

/////////////////////////////////////////////////////////////////////////////
void CConf::Destroy(WORD discCause)
{
	TRACEINTO << "ConfName:" << m_name << ", DiscCause:" << discCause;

	// Remove Sip Conf Registry
	AnnounceDelConfToSipProxy();

	// start by changing audio and video bridge state to disconnecting
	if (m_pAudBrdgInterface)
		m_pAudBrdgInterface->Disconnect();

	if (m_pVideoBridgeInterface)
		m_pVideoBridgeInterface->Disconnect();

	if (m_pFECCBridge)
		m_pFECCBridge->Disconnect();

	if (m_pContentBridge)
		m_pContentBridge->Disconnect();

	if (m_pContentXcodeBridgeInterface)
		m_pContentXcodeBridgeInterface->Disconnect();

	if (IsValidPObjectPtr(m_pSipEventPackage))
		m_pSipEventPackage->TerminateConf();

	if (IsValidPObjectPtr(m_pSvcEventPackage))
		m_pSvcEventPackage->TerminateConf();

	if (IsValidPObjectPtr(m_pMcmsPCMManager))
		m_pMcmsPCMManager->Disconnect();

	if (m_pConfAppMngrInterface)
	{
		CSegment* pMsg = new CSegment;
		*pMsg << (DWORD)EVENT_NOTIFY
		      << (OPCODE)TERMINATECONF;
		m_pConfAppMngrInterface->HandleEvent(pMsg);
		POBJDELETE(pMsg);
	}

	m_state = TERMINATION;
	TRACEINTO_GLA << "Conf state = " << ConfStateToString(m_state);
	DeleteAllTimers();

	TRACEINTO << "PARTY_DISCONNECTION - confRsrcId: " << m_ConfRsrcId << " , Start Conf Termination";


	if (m_pCommConf)
	{
		WORD numParties = m_pCommConf->GetNumParties();
		WORD numOfNonDisconnectedParties = 0;
		if (numParties == 0)
		{
			OnEndDelAllParties();
		}
		else
		{
			CConfParty* pConfParty = m_pCommConf->GetFirstParty();

			PASSERT(!pConfParty);
			if (!pConfParty)
			{
				OnEndDelAllParties();
				return;
			}

			// because confDB is updated async copy party name to private place
			char** partyNameBuf = new char*[numParties];
			WORD i;
			for (i = 0; i < numParties; i++)
			{
				partyNameBuf[i] = new char[H243_NAME_LEN];
				partyNameBuf[i][0] = '\0';
			}

			for (i = 0; i < numParties; i++)
			{
				strncpy(partyNameBuf[i], pConfParty->GetName(), H243_NAME_LEN-1);
				partyNameBuf[i][H243_NAME_LEN-1] = '\0';
				if (i+1 < numParties)
				{
					pConfParty = NULL;
					if (m_pCommConf) pConfParty = m_pCommConf->GetNextParty();

					PASSERT(!pConfParty);
					if (!pConfParty) break;
				}
			}

			for (i = 0; i < numParties; i++)
			{
				DelParty(partyNameBuf[i], 0, discCause);
				delete [] partyNameBuf[i];
			}

			delete [] partyNameBuf;
		}
	}
	else
	{
		PASSERT(!m_pCommConf);
		OnEndDelAllParties();
	}
}

//--------------------------------------------------------------------------
void CConf::HandlePartyEvent(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);

	PartyRsrcID PartyId;
	WORD opcode;

	*pMsg >> PartyId >> opcode;

	CPartyCntl* pPartyCtrl = NULL;

	switch (opcode)
	{
		case PARTYEXPORT:
		case REMOTE_SENT_DISCONNECT_BRIDGES:
		case REMOTE_SENT_CONNECT_BRIDGES:
		case PARTYENDINITCOM:
		case PARTYCONNECT:
		case H323PARTYCONNECT:
		case H323PARTYCONNECTALL:
		case ADDPROTOCOL:
		case REMOVEPROTOCOL:
		case UPDATE_CAPS:
		case UPDATE_VIDEO_RATE:
		case LPR_CHANGE_RATE:
		case FEC_RED_CHANGE_RATE:
		case CLEAN_VIDEO_RATE_LIMIT:
		//case UPDATEVISUALNAME: BRIDGE-3519
		case IP_MUTE_MEDIA:
		case IPPARTYUPDATEBRIDGES:
		case SIP_PARTY_CHANS_CONNECTED:
		case PARTYRMTCAP:
		case IPPARTYCONNECTED:
		case SIP_PARTY_SEND_CHANNEL_HANDLE:
		case FALL_BACK_FROM_TIP_TO_REGULAR_SIP:
		case PARTY_RECEIVE_ECS:
		case PARTY_FAULTY_RSRC:
		case REMOTE_SENT_RE_CAPS:
		case COP_VIDEO_IN_CHANGE_MODE:
		case COP_VIDEO_OUT_CHANGE_MODE:
		case COP_START_CASCADE_LINK_LECTURE_MODE:
		case COP_UPDATE_CASCADE_LINK_LECTURE_MODE:
		case SCP_REQUEST_BY_EP:
		case SCP_NOTIFICATION_BY_EP:
		case PARTYENDDISCONNECT:
		case INCREASE_DISCONNECT_TIMER:
		case ENDNETCHNLDISCONNECT:
		case PARTYENDCHANGEMODE:
		case PARTY_CLOSE_CHANNEL:
		case VIDEOMUTE:
		case AUDIOMUTE:
		case VIDREFRESH:
		case REQUEST_PREVIEW_INTRA:
		case IPPARTYMSECONDARY:
		case ITPSPEAKERIND:
		case ITPSPEAKERACKIND:
		case SECONDARYCAUSEH323:
		case PARTY_IN_CONF_IND:
		case RMTXFRMODE:
		case ALLRMTCAPSRECEIVED:
		case PCM_PARTY_STATE_CHANGED:
		case TMPPHONENUMBER_FREE:
		case UPDATENETCHANNEL:
		case REALLOCATERTM:
		case BOARDFULL:
		case SMART_RECOVERY:
		case UPDATEICEPARAMS:
		case FALL_BACK_FROM_ICE_TO_SIP:
		case UPDATE_VIDEO_RESOLUTION:
		case UPDATE_VIDEO_AFTER_VSR_MSG:
		case SINGLE_UPDATE_PACSI_INFO:
		case ADDSLAVEPARTY:
		case ADDSUBLINKSPARTIES:
		case PARTY_PARTYCONTROL_MASTER_TO_SLAVE:
		case PARTY_PARTYCONTROL_SLAVE_TO_MASTER:
		case PARTY_PARTYCONTROL_MSSLAVE_TO_MAIN:
		case RELAY_ENDPOINT_ASK_FOR_INTRA:
		case SCP_NOTIFICATION_REQ:
		case CONF_API_SCP_NOTIFICATION_ACK_FROM_EP:
		case SCP_PIPES_MAPPING_NOTIFICATION_REQ:
		case SCP_IVR_SHOW_SLIDE_REQ:
		case SCP_IVR_STOP_SHOW_SLIDE_REQ:
		case SIP_PARTY_CONF_PWD_STAUTS_ACK:
		case RMTCAP:
		case SETNODETYPE:
		case PRESENTATION_OUT_STREAM_UPDATED:
		case UPDATE_ART_WITH_SSRC_REQ:
		case END_VIDEO_UPGRADE_TO_MIX_AVC_SVC:
		case END_AUDIO_UPGRADE_TO_MIX_AVC_SVC:
		case END_VIDEO_DOWNGRADE_MIX_AVC_SVC:
		case END_AUDIO_DOWNGRADE_MIX_AVC_SVC:
		case PARTY_UPGRADE_TO_MIX_CHANNELS_UPDATED:
		case UPDATE_REMOTE_CAPS_FROM_PARTY:
		case CHANGE_VIDEO_OUT_TIP_POLYCOM:
		case SEND_MRMP_STREAM_IS_MUST_REQ:
		case MSORGANIZERENDCONNECT:
		case MSFOCUSENDCONNECT:
		case MSFOCUSENDDISCONNECT:
		case MSSUBSCRIBERENDCONNECT:
		case MSSUBSCRIBERENDDISCONNECT:
		case MSFT_AVMCU2013DETECTED:
		case UPDATE_VIDBRDG_TELEPRESENSE_EP_INFO: //_e_m_
		case  SRS_RECORDING_CONTROL_ACK:
		case  SRS_LAYOUT_CONTROL_PARTY_TO_CONF:
		case UPDATE_LAST_TARGET_MODE_MSG:
		case CHANGE_CONTENT_BIT_RATE_BY_LPR:// VNGFE-8204
			pPartyCtrl = GetPartyCntl(PartyId);
			if (!pPartyCtrl)
			{
				TRACEWARN << "PartyId:" << PartyId << ", Opcode:" << opcode << " - Failed, party control not found";
				DBGPASSERT_AND_RETURN(1);
			}
			TRACEINTO << "PartyId:" << PartyId << ", Opcode:" << opcode << ", pPartyCtrl:" << std::hex << pPartyCtrl << ", PartyCtrlState:" << pPartyCtrl->GetStateAsString();
			break;

		default:
			pPartyCtrl = GetPartyCntl((CTaskApp*)PartyId);//D.K.Temporary. We will send soon party id instead of pointer
			if (!pPartyCtrl)
			{
				TRACEWARN << "pParty:" << std::hex << PartyId << std::dec << ", Opcode:" << opcode << " - Failed, party control not found";
				DBGPASSERT_AND_RETURN(1);
			}
			TRACEINTO << "PartyId:" << PartyId << ", pParty:" << std::hex << (CTaskApp*)PartyId << ", Opcode:" << std::dec << opcode << ", pPartyCtrl:" << std::hex << pPartyCtrl << ", PartyCtrlState:" << pPartyCtrl->GetStateAsString();
			break;
	}

	pPartyCtrl->HandleEvent(pMsg, 0, opcode);

	if (PARTY_IN_CONF_IND == opcode && m_pSvcEventPackage)
		m_pSvcEventPackage->PartyLeaveIVR(PartyId);
}

//////////////////////////////////////////////////////////////////////////////
void CConf::HandlePartyRsrcIdEvent(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);

	PartyRsrcID PartyID;
	OPCODE opcode;

	*pMsg >> PartyID;
	*pMsg >> opcode;
	CPartyCntl* pPartyCntl = GetPartyCntl(PartyID);
	TRACECOND_AND_RETURN(!pPartyCntl, "PartyId:" << PartyID << " - Failed, Party does not exist");

	pPartyCntl->HandleEvent(pMsg, 0, opcode);
}

//////////////////////////////////////////////////////////////////////////////
void CConf::HandlePartyExternalEvent(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);

	PartyRsrcID partyID;
	OPCODE opcode;

	*pMsg >> partyID;
	*pMsg >> opcode;

	CPartyCntl* pPartyCntl = GetPartyCntl(partyID);
	TRACECOND_AND_RETURN(!pPartyCntl, "PartyId:" << partyID << " - Failed, Party does not exist");

	CSegment *pNewSeg = new CSegment;
	*pNewSeg = *pMsg;
	pPartyCntl->HandlePartyExternalEvent(pNewSeg, opcode);

}

//////////////////////////////////////////////////////////////////////////////
void CConf::HandleConfMplEvent(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);

	OPCODE opcode;

	*pMsg >> opcode;

	switch (opcode)
	{
		case AC_ACTIVE_SPEAKER_IND:
		case AC_AUDIO_SPEAKER_IND:
		{
			m_pAudBrdgInterface->SpeakerChangeIndication(pMsg, opcode);
			break;
		}
		case ACK_IND :
		{
			OPCODE ack_opcode;
			*pMsg >> ack_opcode;
			if(IsIvrAcknowledge(ack_opcode))
			{
				DWORD  ack_seq_num;
				STATUS  status;
				*pMsg >> ack_seq_num >> status;

				CSegment *pSeg = new CSegment;
				DWORD invalidPartyRsrcId = INVALID; //we always send conf msgs to cam with invalid instead of partyRsrcId
				*pSeg << (DWORD)EVENT_NOTIFY << (OPCODE)ACK_IND << (DWORD)invalidPartyRsrcId << (OPCODE)ack_opcode << ack_seq_num << status << *pMsg;

				if ( m_pConfAppMngrInterface )
					m_pConfAppMngrInterface->HandleEvent(pSeg);
				POBJDELETE(pSeg);
				break;
			}
			TRACEINTO << "ConfName:" << m_name << ", AckOpcode:" << ack_opcode << " - Ack currently ignored";
			break;
		}
		default:
		{
			PASSERTSTREAM(1, "Opcode:" << opcode << " - Invalid opcode value");
			break;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
void CConf::HandleMsLyncSlavesControllerEvent(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);

	PartyRsrcID partyRsrcID;
	OPCODE opcode;

	*pMsg >> opcode;
	*pMsg >> partyRsrcID;

	if (LYNC_CONF_INFO_UPDATED == opcode)
		pMsg->DecRead(sizeof(PartyRsrcID));	//partyRsrcId is needed for notification container deserialization

	CSipPartyCntl* pSipPartyCntl = (CSipPartyCntl*)GetPartyCntl(partyRsrcID);
	TRACECOND_AND_RETURN(!pSipPartyCntl, "partyRsrcID:" << partyRsrcID << " - Failed, Party does not exist");

	CMsSlavesController* pMsSlavesController = pSipPartyCntl->GetMsSlavesController();
	TRACECOND_AND_RETURN(!pMsSlavesController, "pMsSlavesController is NULL" << ", partyRsrcID: " << partyRsrcID);

	pMsSlavesController->HandleEvent(pMsg, opcode);
}
/////////////////////////////////////////////////////////////////////////////
void CConf::HandleMsLyncConfEvent(CSegment* pMsg)
{
	TRACEINTO;
	PASSERT_AND_RETURN(!pMsg);

	PartyRsrcID partyRsrcID;
	OPCODE opcode;

	*pMsg >> opcode;
	*pMsg >> partyRsrcID;

	if (LYNC_CONF_INFO_UPDATED == opcode)
		pMsg->DecRead(sizeof(PartyRsrcID));	//partyRsrcId is needed for notification container deserialization



	DispatchEvent(opcode,pMsg);
}

/*//////////////////////////////////////////////////////////////////////////////
void CConf::HandleMsLyncMainPartyCntlEvent(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);

	PartyRsrcID PartyRsrcID;
	OPCODE opcode;

	*pMsg >> opcode;
	*pMsg >> PartyRsrcID;

	CPartyCntl* pPartyCntl = GetPartyCntl(PartyRsrcID);
	TRACECOND_AND_RETURN(!pPartyCntl, "PartyRsrcId:" << PartyRsrcID << " - Failed, Party does not exist");

	pPartyCntl->HandleEvent(pMsg, 0, opcode);
}*/

BYTE CConf::IsIvrAcknowledge(OPCODE ack_opcode)
{
	switch (ack_opcode)
	{
		case IVR_PLAY_MESSAGE_REQ:
		case IVR_STOP_PLAY_MESSAGE_REQ:
		case IVR_START_IVR_REQ:
		case IVR_STOP_IVR_REQ:
		case IVR_PLAY_MUSIC_REQ:
		case IVR_STOP_PLAY_MUSIC_REQ:
		case IVR_RECORD_ROLL_CALL_REQ:
		case AUDIO_PLAY_TONE_REQ:
		case IVR_STOP_RECORD_ROLL_CALL_REQ:
		// case	IVR_STOP_PLAY_TONE_REQ:
		{
			return TRUE;
		}

		default:
		{
			return FALSE;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
void CConf::HandleUpdateEvent(CSegment* pMsg)
{
	CTaskApp* pParty;
	WORD      type;
	DWORD     param;

	*pMsg >> (void*&)pParty >> type >> param;
	UpdateDB(pParty, type, param, pMsg);
}

//////////////////////////////////////////////////////////////////////////////
void CConf::HandleBrdgEvent(CSegment* pMsg)
{
	CBridge* pBrdg  = NULL;
	DWORD    msgLen = 0;
	OPCODE   opCode;

	*pMsg >> (void*&)pBrdg
	>>  msgLen
	>> opCode;

// PASSERT(! pBrdg);

	if (IsValidPObjectPtr(pBrdg))
		pBrdg->HandleEvent(pMsg, msgLen, opCode);
	else
		DBGPASSERT(opCode);
}

//////////////////////////////////////////////////////////////////////////////
void CConf::HandleObserverUpdateEvent(CSegment* pMsg)
{
	WORD type = 0;
	*pMsg >> type;

	TRACEINTO << "ConfName:" << m_name << ", UpdateEventType:" << type;

	switch (type)
	{
		case (0):
		{
			break;
		}

		case (SIP_EVENT_PACKAGE):
			if (IsValidPObjectPtr(m_pSvcEventPackage))
			{
				CSegment* pSegTmp = new CSegment(*pMsg);
				m_pSvcEventPackage->HandleObserverUpdate(pSegTmp, type);
				PDELETE(pSegTmp);
			}

		case (SIP_CX_PACKAGE):
		case (SIP_REFER):
		{
			if (IsValidPObjectPtr(m_pSipEventPackage))
				m_pSipEventPackage->HandleObserverUpdate(pMsg, type);

			break;
		}

		case (OBSERVER_TYPE_PCM):
		{
			if (IsValidPObjectPtr(m_pMcmsPCMManager))
			{
				m_pMcmsPCMManager->HandleObserverUpdate(pMsg, type);
			}
			break;
		}

		default:
		{
			DBGPASSERT(type);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
void CConf::HandleCdrUpdateEvent(CSegment* pMsg)
{
	CTaskApp* pParty;

	*pMsg >> (void*&)pParty;

	if (pParty == (CTaskApp*)0xffff || !pParty)
	{
		PTRACE(eLevelError, "CConf::HandleCdrUpdateEvent - Failed, 'pParty' is invalid");
		return;
	}

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	if (!pPartyConnection)
	{
		PTRACE(eLevelError, "CConf::HandleCdrUpdateEvent - Failed, 'pPartyConnection' is invalid");
		return;
	}

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
	if (!pConfParty)
	{
		PTRACE(eLevelError, "CConf::HandleCdrUpdateEvent - Failed, 'pConfParty' is invalid");
		return;
	}

	DWORD updateOpcode;
	*pMsg >> updateOpcode;

	switch (updateOpcode)
	{
		case EVENT_PARTY_DISCONNECTED:
		case EVENT_PARTY_CONNECTED:
		case IP_PARTY_CONNECTED:
		case SIP_PARTY_CONNECTED:
		{
			PlcmCdrEventDisconnectedExtendedHelper *disconnectedExtendedHelperPtr = NULL;

			if (pConfParty->GetPartyState() == PARTY_DISCONNECTED)
			{
				disconnectedExtendedHelperPtr = new PlcmCdrEventDisconnectedExtendedHelper();
				disconnectedExtendedHelperPtr->SetInterfaceType(pConfParty->GetNetInterfaceType());
			//	PlcmCdrEventDisconnectedExtendedHelper cdrEventDisconnectedExtendedHelper;

				//cdrEventDisconnectedExtendedHelper.SetInterfaceType(pConfParty->GetNetInterfaceType());
			}


			//m_pCommConf->PartyConnectDisconnectToCDR(pConfParty, &cdrEventDisconnectedExtendedHelper);
			m_pCommConf->PartyConnectDisconnectToCDR(pConfParty, disconnectedExtendedHelperPtr);

			if (pConfParty->GetPartyState() == PARTY_DISCONNECTED)
			{
				WORD sumL_syncLost      = pConfParty->GetL_syncLostCounter();
				WORD sumR_syncLost      = pConfParty->GetR_syncLostCounter();
				WORD sumL_videoSyncLost = pConfParty->GetL_videoSyncLostCounter();
				WORD sumR_videoSyncLost = pConfParty->GetR_videoSyncLostCounter();

				//m_pCommConf->PartyDisconnectCont1(sumL_syncLost, sumR_syncLost, sumL_videoSyncLost, sumR_videoSyncLost, &cdrEventDisconnectedExtendedHelper);
				m_pCommConf->PartyDisconnectCont1(sumL_syncLost, sumR_syncLost, sumL_videoSyncLost, sumR_videoSyncLost, disconnectedExtendedHelperPtr);
				pConfParty->SetL_syncLostCounter(0);
				pConfParty->SetR_syncLostCounter(0);
				pConfParty->SetL_videoSyncLostCounter(0);
				pConfParty->SetR_videoSyncLostCounter(0);

				if (disconnectedExtendedHelperPtr != NULL)
				{
					m_pCommConf->SendCdrEvendToCdrManager((ApiBaseObjectPtr)&(disconnectedExtendedHelperPtr->GetCdrObject()), false, disconnectedExtendedHelperPtr->GetCdrObject().m_partyId, pConfParty->GetCorrelationId());
					delete  disconnectedExtendedHelperPtr;
				}
			}
			//m_pCommConf->SendCdrEvendToCdrManager((ApiBaseObjectPtr)&cdrEventDisconnectedExtendedHelper.GetCdrObject(), false, cdrEventDisconnectedExtendedHelper.GetCdrObject().m_partyId, pConfParty->GetCorrelationId());


			break;
		}

		case SVC_SIP_PARTY_CONNECTED:
		{
			DWORD n = 0, dwTmp = 0;
			*pMsg >> n;
			std::list<SvcStreamDesc> listStreams;
			for (DWORD i = 0; i < n; ++i)
			{
				SvcStreamDesc svcStream;
				*pMsg >> dwTmp; svcStream.m_bitRate   = dwTmp;
				*pMsg >> dwTmp; svcStream.m_frameRate = dwTmp;
				*pMsg >> dwTmp; svcStream.m_height    = dwTmp;
				*pMsg >> dwTmp; svcStream.m_width     = dwTmp;
				listStreams.push_back(svcStream);
			}

			*pMsg >> dwTmp;
			ECodecSubType eAudioCodec = (ECodecSubType)dwTmp;
			DWORD dwBitRateOut = 0;
			DWORD dwBitRateIn  = 0;
			*pMsg >> dwBitRateOut >> dwBitRateIn;
			m_pCommConf->SvcSipPartyConnectCDR(pConfParty, &listStreams, eAudioCodec, dwBitRateOut, dwBitRateIn);
			break;
		}

		case GK_INFO:
		{
			BYTE gkCallId[SIZE_OF_CALL_ID];
			memset(gkCallId, '\0', SIZE_OF_CALL_ID);
			pMsg->Get((unsigned char*)gkCallId, SIZE_OF_CALL_ID);
			m_pCommConf->GkInfoToCDR(pPartyConnection->GetName(), pConfParty->GetPartyId(), gkCallId);
			break;
		}

		case PARTY_NEW_RATE:
		{
			DWORD cuurentRate = 0;
			*pMsg >> cuurentRate;
			if(pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveOut && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveIn)
				m_pCommConf->NewRateInfoToCdr(pPartyConnection->GetName(), pConfParty->GetPartyId(), cuurentRate);
			break;
		}

		case PARTICIPANT_MAX_USAGE_INFO:
		{
			DWORD maxBitRate = 0;
			DWORD format     = 0;
			WORD  maxFR      = 0;

			*pMsg >> maxBitRate >> format >> maxFR;

			EFormat     maxFormat     = (EFormat)format;
			CCapSetInfo capInfo;
			char*       maxResolution = capInfo.GetFormatStrForCDREvent(maxFormat);
			char*       maxFrameRate  = capInfo.GetMaxFrameRateRoundUpStrForCDREvent(maxFR);
			CConfParty* pConfParty    = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());

			if (pConfParty)
			{
				char tempName[IPV6_ADDRESS_LEN+1];
				memset(&tempName, '\0', IPV6_ADDRESS_LEN);
				ipToString(pConfParty->GetIpAddress(), tempName, 1);

				if ((pConfParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE) && (pConfParty->GetConnectionType() == DIAL_IN))
				{
					// PTRACE2INT( eLevelInfoNormal, "CConf::HandleCdrUpdateEvent ADDRESS: ", strlen(tempName) );
					strcpy_safe(tempName, pConfParty->GetSipPartyAddress());
				}

				m_pCommConf->CallInfoPerPartyAfterDisconnectionToCdr(pPartyConnection->GetName(),
				                                                     pConfParty->GetPartyId(), maxBitRate,
				                                                     maxResolution, maxFrameRate, tempName); // endPointType);
			}
			else
			{
				PASSERTMSG(NULL == pConfParty, "ConfParty is NOT found !!!");
			}
			break;
		}
	} // switch
}


//////////////////////////////////////////////////////////////////////////////
void CConf::AddConfEntryToRsrcRoutingTbl()
{
#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	CConfApi* pConfApi = new CConfApi(m_monitorConfId);
#else
	CConfApi* pConfApi = new CConfApi;
#endif
	pConfApi->CreateOnlyApi(GetRcvMbx(), NULL);

	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if (pRoutingTbl == NULL)
	{
		POBJDELETE(pConfApi);
		PASSERT_AND_RETURN(101);
	}

	pRoutingTbl->AddConfToRoutingTbl(m_ConfRsrcId, pConfApi);
	POBJDELETE(pConfApi);

	StartTimer(MESSAGE_QUEUE_TOUT, SECOND);
}

//////////////////////////////////////////////////////////////////////////////
void CConf::SetPartyAsLeader(const char * partyName, EOnOff onOff)
{
	CConfApi * pConfApi = new CConfApi;
	pConfApi->CreateOnlyApi(GetRcvMbx(),NULL);

	pConfApi->SetPartyAsLeader(partyName, onOff);

	POBJDELETE(pConfApi);
}
//////////////////////////////////////////////////////////////////////////////
void CConf::RemoveConfEntryFromRsrcRoutingTbl()
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if(pRoutingTbl== NULL)
	{
		PASSERT_AND_RETURN(101);
	}

	pRoutingTbl->RemoveConfFromRoutingTbl(m_ConfRsrcId);
}

//////////////////////////////////////////////////////////////////////////////
void CConf::DelParty(const char* name, WORD mode, WORD discCause, BOOL isViolent, DWORD taskId) // shiraITP - 124
{
	CPartyConnection*  pPartyConnection = GetPartyConnection(name);

	TRACEINTO << "PartyName:" << name << ", Mode:" << mode << ", DisCause:" << discCause << ", pPartyConnection:" << (DWORD)pPartyConnection;

	if (pPartyConnection)
	{
		CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
		TRACECOND_AND_RETURN(!pConfParty, "pConfParty is NULL");

		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		TRACECOND_AND_RETURN(!pPartyCntl, "pPartyCntl is NULL");

		if(m_state == TERMINATION){
			std::ostringstream msg;
			msg << "PARTY_DISCONNECTION - confRsrcId: " << m_ConfRsrcId << " , partyRsrcId: " << pPartyConnection->GetPartyRsrcId() << " , Start Disconnection From EMA/DMA conf TERMINATION";
			TRACEINTO << msg.str().c_str();
		}

		// We need to add treatment for full blast disconnection too since CsRcvSocket can't handle full capacity disconnection of parties.
		DWORD connectDelay_2     = 0;
		DWORD sysCfgConnectDelay = 0;
		DWORD disconnectionDelay = 0;
		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("DELAY_BETWEEN_DIAL_OUT_PARTY", connectDelay_2);
		if (pPartyConnection->GetVoice())
			connectDelay_2 = connectDelay_2-100;             // Since the party is Audio Only then "disconnect delay" should be smaller.

		sysCfgConnectDelay = connectDelay_2/10; // connectDelay is in SECONDS - should be 0.25 SECONDS

		if (m_firstParticipantDisconnectingTicksTime == 0) // First party in conference
		{ // No need for delay
			m_firstParticipantDisconnectingTicksTime = m_lastPartyDisconnectedInTicks = SystemGetTickCount().GetIntegerPartForTrace();
			m_numOfBlastDisconnectingParticipants++;
			TRACEINTO << "PartyName:" << name << ", FirstParticipantDisconnectingTicksTime:" << m_firstParticipantDisconnectingTicksTime.GetIntegerPartForTrace();
		}
		else // Start computing delay algorithm
		{
			// Step 2: Compute current gap and check we need to add delay
			TICKS currentDisConnectionInTicks = SystemGetTickCount().GetIntegerPartForTrace();
			TICKS currentGapInTicks           = currentDisConnectionInTicks - m_lastPartyDisconnectedInTicks;
			TICKS totalGapBetweenDelRequest   = currentDisConnectionInTicks - m_firstParticipantDisconnectingTicksTime;
			DWORD computedDisConnectionDelay  = (m_numOfBlastDisconnectingParticipants * sysCfgConnectDelay) - totalGapBetweenDelRequest.GetIntegerPartForTrace();
			DWORD supposedDisConnectionTime   = currentDisConnectionInTicks.GetIntegerPartForTrace() + computedDisConnectionDelay;
			DWORD absDisConnectionTime        = abs((int)(m_lastPartyDisconnectedInTicks.GetIntegerPartForTrace() - currentDisConnectionInTicks.GetIntegerPartForTrace()));
			if ((currentGapInTicks < sysCfgConnectDelay) || (absDisConnectionTime < sysCfgConnectDelay))
			{
				// compute and send to delay PC
				disconnectionDelay             = computedDisConnectionDelay;
				TRACEINTO << "PartyName:" << name << ", DisconnectionDelay:" << disconnectionDelay;
				m_lastPartyDisconnectedInTicks = supposedDisConnectionTime;
				m_numOfBlastDisconnectingParticipants++;
			}
			else
			{
				// Restart the whole procedure with the last party as the first connected one for this session
				m_numOfBlastDisconnectingParticipants    = 1;
				m_firstParticipantDisconnectingTicksTime = m_lastPartyDisconnectedInTicks = currentDisConnectionInTicks;
			}
		}

		switch (pPartyConnection->GetInterfaceType())
		{
			case H323_INTERFACE_TYPE:
			{
				pPartyConnection->DisconnectH323(mode, 1, disconnectionDelay, isViolent, taskId); // 1 = Set redial counter to zero  //shiraITP - 125
				break;
			}

			case SIP_INTERFACE_TYPE:
			{
				CSipNetSetup* pNetSetup = (CSipNetSetup*)((CSipPartyCntl*)pPartyCntl)->GetSipNetSetup();
				if (pNetSetup)
				{
					const char* strRemoteAddr = pNetSetup->GetRemoteSipAddress();
					pConfParty->SetSipPartyAddress(strRemoteAddr);
					pConfParty->SetSipPartyAddressType(PARTY_SIP_SIPURI_ID_TYPE);
				}

				if (pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone &&  pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveOut && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveIn )
				{
						if(GetCommConf() && !strlen(GetCommConf()->GetFocusUriScheduling()))
						{
							TRACEINTO << "PartyId:" << m_AvMcuPartyRsrcId << " meet now- av-mcu unregister";
							EventPackage::Manager& lyncEventManager = EventPackage::Manager::Instance();
							lyncEventManager.DelSubscriber(pPartyConnection->GetPartyRsrcId(), EventPackage::eEventType_MediaDeleted, std::make_pair((COsQueue*)&(GetRcvMbx()), MS_LYNC_CONF_PARTY_CNTL_MSG));
						}
						else
							TRACEINTO << "PartyId:" << m_AvMcuPartyRsrcId << " schedule no need to unregister - av-mcu ";

						m_AvMcuPartyRsrcId  = 0;
				}

				pPartyConnection->DisconnectSip(mode, 0, NULL, disconnectionDelay, isViolent, taskId);
				break;
			}

			case ISDN_INTERFACE_TYPE:
			{
				if (pPartyConnection->GetVoice())
					pPartyConnection->DisconnectPstn(mode, disconnectionDelay, isViolent, taskId);
				else
					pPartyConnection->DisconnectIsdn(mode, disconnectionDelay, isViolent, taskId);
				break;
			}
		} // switch

		// check new content protocol when parties disconnect
		if (IsEnableH239())
		{
			UpdateContentProtocolOnDisconnectParty();
		}

		UpdateDB(pPartyConnection->GetPartyTaskApp(), DISCAUSE, (discCause) ? discCause : DISCONNECTED_BY_OPERATOR);
	}
	else // meet me party that does not have party connection yet
	{
		DelPartyWithNoConnection(name, mode, discCause);

		// According to VNGFE-4035 (very infrequent case) - party is not contained in PartyList, but contained in the bridges.
		// After discuss with Dmitry/Ron/Boris it was decided when the MCMS disconnects a party the MCMS should try clean this party in the bridges.
		if (m_pVideoBridgeInterface)
			m_pVideoBridgeInterface->DeletePartyFromConf(name);
	}
}

//////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CConf::DisconnectAllRoomControlLinks
void CConf::DisconnectAllRoomControlLinks(const char* mainLinkName, BYTE cascadedLinkNunber, WORD mode) // shiraITP - 124.mine
{
	WORD discCause = 0;

	for (BYTE i = 2; i <= cascadedLinkNunber; i++)
	{
		// Find sub name:
		char subPartyName[H243_NAME_LEN];
		::GetSubLinkName(mainLinkName, i, (char*)subPartyName);

		CPartyConnection* pPartyConnection = GetPartyConnection(subPartyName);

		if (pPartyConnection && m_pCommConf)
		{
			CConfParty* pConfPartySub = m_pCommConf->GetCurrentParty(subPartyName);
			if (pConfPartySub)
			{
				DWORD subPartyState = pConfPartySub->GetPartyState();

				TRACEINTO << "SubPartyName:" << subPartyName << ", SubPartyState:" << subPartyState;

				if (subPartyState == PARTY_CONNECTED || subPartyState == PARTY_CONNECTED_PARTIALY ||
				    subPartyState == PARTY_SECONDARY || subPartyState == PARTY_CONNECTED_WITH_PROBLEM || subPartyState == PARTY_CONNECTING)
				{
					pPartyConnection->DisconnectH323(mode);           // shiraITP - 125

					// check new content protocol when parties disconnect
					if (IsEnableH239())
						DispatchEvent(UPDATE_CONTENT_PROTOCOL);

					UpdateDB(pPartyConnection->GetPartyTaskApp(), DISCAUSE, (discCause) ? discCause : DISCONNECTED_BY_OPERATOR);
				}
				else if (subPartyState == PARTY_DISCONNECTED || subPartyState == PARTY_DISCONNECTING)
				{
					TRACEINTO << "SubPartyName:" << subPartyName << " - Disconnected";

					pPartyConnection->DisconnectH323(0);               // mode = 0 = delete party
					UpdateDB(pPartyConnection->GetPartyTaskApp(), DISCAUSE, (discCause) ? discCause : DISCONNECTED_BY_OPERATOR);
				}
			}
			else
			{
				TRACEINTO << "SubPartyName:" << subPartyName << " - pConfPartySub is NULL";
			}
		}
		else     // meet me party that does not have party connection yet
		{
			CPartyConnection* pMainPartyConnection = GetPartyConnection(mainLinkName);

			CH323PartyCntl* pMainPartyCntl = NULL;

			if (pMainPartyConnection)
				pMainPartyCntl = (CH323PartyCntl*)(pMainPartyConnection->GetPartyCntl());

			if (pMainPartyCntl && pMainPartyCntl->IsSubPartyConnected(subPartyName, i) == TRUE)
			{
				TRACEINTO << "SubPartyName:" << subPartyName << " - Still connected";
				DelPartyWithNoConnection(subPartyName, mode, discCause);
				if (m_pVideoBridgeInterface)
					m_pVideoBridgeInterface->DeletePartyFromConf(subPartyName);
			}
			else
				TRACEINTO << "SubPartyName:" << subPartyName << " - Already disconnected";
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void CConf::DelPartyWithNoConnection(const char* name, WORD mode, WORD discCause)
{
	TRACEINTO << "PartyName:" << name;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	TRACECOND_AND_RETURN(!pCommConf, "pCommConf is NULL");

	CConfParty* pConfParty = pCommConf->GetCurrentParty(name);
	TRACECOND_AND_RETURN(!pConfParty, "pConfParty is NULL");

	if (!mode)
	{
		COsQueue* pConfRcvMailBox = pCommConf->GetRcvMbx();

		pCommConf->SetRcvMbx(m_pRcvMbx);
		pCommConf->Cancel(name);
		pCommConf->SetRcvMbx(pConfRcvMailBox);
	}

	UpdateConfStatus();
	if (m_state == TERMINATION && !pCommConf->GetNumParties())
		OnEndDelAllParties();
}

/////////////////////////////////////////////////////////////////////////////
void CConf::OnEndDelAllParties()
{
	TRACEINTO << "ConfName:" << m_name;
	// delete timer ???
	if (!m_isAudConnected && !m_isVidConnected && !m_isFeccConnected && !m_isContentConnected /*&& !m_isMlpConnected*/)
		CleanUp();
	else
	{
		if (m_isAudConnected)
		{
			TRACEINTO << "ConfName:" << m_name << " - Disconnecting Conf Audio...";
			if (m_pAudBrdgInterface)
				m_pAudBrdgInterface->Terminate();
		}

		if (m_isVidConnected)
		{
			TRACEINTO << "ConfName:" << m_name << " - Disconnecting Conf Video...";
			if (m_pVideoBridgeInterface)
				m_pVideoBridgeInterface->Terminate();
		}

		if (m_isFeccConnected)
		{
			TRACEINTO << "ConfName:" << m_name << " - Disconnecting Conf FECC...";
			if (m_pFECCBridge)
				m_pFECCBridge->Terminate();
		}

		if (m_isContentConnected)
		{
			TRACEINTO << "ConfName:" << m_name << " - Disconnecting Conf Content...";
			if (m_pContentBridge)
				m_pContentBridge->Terminate();
		}

		if (m_pContentXcodeBridgeInterface)
		{
			TRACEINTO << "ConfName:" << m_name << " - Disconnecting Conf XCode...";
			m_pContentXcodeBridgeInterface->Terminate();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CConf::BridgeDisConnectionCompleted()
{
	TRACEINTO << "ConfName:" << m_name;
	if (!m_isAudConnected && !m_isVidConnected && !m_isFeccConnected && !m_isContentConnected  && !m_isContentXCodeConnected/*&& !m_isMlpConnected*/)
		CleanUp();
}

/////////////////////////////////////////////////////////////////////////////
void CConf::BridgeConnectionCompleted()
{
	if (m_isBridgeError)
	{
		m_state = IDLE;
		TRACEINTO_GLA << "Conf state = " << ConfStateToString(m_state);
		TRACESTRFUNC(eLevelError) << "ConfName:" << m_name << ", IsBridgeError:" << m_isBridgeError << " - Failed to connect one of the bridges";
		// In case of COP if the VB fails to connect we will terminate the conference.
		if (GetVideoSession() == VIDEO_SESSION_COP || (m_pCommConf->GetContentMultiResolutionEnabled() && m_pCommConf->GetIsAsSipContent()))
		{
			// 1. faults
			CMedString str;
			str << "Fail to create COP conference Name: "<< m_name;
			CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, CONFERENCE_END_DUE_TO_RESOURCE_DEFICIENCY, MAJOR_ERROR_LEVEL, str.GetString(), TRUE);
			// 2. set terminate cause.
			SetConfTerminateCause(RESOURCE_DEFICIENCY);
			// 3. Update the RA that there was problem with the conference resources so it will init kill port
			m_rsrcDeallocateStatus = STATUS_FAIL;
			// 4. destroy conference.
			Destroy();
		}
		return;
	}

	WORD targetState  = 1; // audio always connected

	if (NULL != m_pVideoBridgeInterface) targetState++;
	if (NULL != m_pFECCBridge) targetState++;
	if (NULL != m_pContentBridge) targetState++;
	if (NULL != m_pContentXcodeBridgeInterface) targetState++;

	WORD curState = 0;

	if (m_isAudConnected) curState++;
	if (m_isVidConnected) curState++;
	if (m_isFeccConnected) curState++;
	if (m_isContentConnected) curState++;
	if (m_isContentXCodeConnected) curState++;

	TRACEINTO << "ConfName:" << m_name << ", CurState:" << curState << ", TargetState:" << targetState;

	if (curState == targetState)
	{
		m_state = CONNECT;
		TRACEINTO_GLA << "Conf state = " << ConfStateToString(m_state);

		AnnounceLobbyConfOnAirIfNeeded();
		// VNGR-23989
		ReleaseLobbySuspendPartiesIfNeeded();

		CConfParty* pConfParty = m_pCommConf->GetFirstParty();
		m_pGatheringManager->Start();

		if (pConfParty)
		{
			DWORD connectDelay = 0;

			for (WORD i = 0; i < m_pCommConf->GetNumParties(); i++)        // dial out loop
			{
				if (pConfParty->CanConnectParty())
				{
					connectDelay = ComputeConnectingConnectionDelay(pConfParty->GetVoice(), pConfParty->GetName(), pConfParty->GetNetInterfaceType());
					if(pConfParty->GetIsDMAAVMCUParty())
						ConnectDMAAVMCUParty(pConfParty, connectDelay);
					else
					ConnectParty(pConfParty, connectDelay);
				}

				WORD numOfParties = m_pCommConf->GetNumParties();
				if (i+1 < numOfParties)
				{
					int next = i+1;
					pConfParty = m_pCommConf->GetNextParty(next);
					PASSERT(!pConfParty);
					if (!pConfParty) break;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConf::CreateVideoBridge(EBridgeImplementationTypes vbImplementaionType)
{
	CVisualEffectsParams* pVisualEffects = new CVisualEffectsParams(*m_pCommConf->GetVisualEffects());

	if (m_pCommConf->GetIsTelePresenceMode())
	{
		DWORD colorBlack = 0x00108080;
		pVisualEffects->SetSpeakerNotationEnable(NO);
		pVisualEffects->SetlayoutBorderEnable(NO);
		pVisualEffects->SetlayoutBorderWidth(eLayoutBorderNone);
		pVisualEffects->SetBackgroundColorRGB(0);
		pVisualEffects->SetBackgroundColorYUV(colorBlack);
		pVisualEffects->SetSiteNamesEnable(NO);
	}

	TRACEINTO << "ConfName:" << m_name;

	CLectureModeParams* pLectureMode = new CLectureModeParams(*m_pCommConf->GetLectureMode());
	pLectureMode->SetLectureModeRegularFromLecturerName();
	pLectureMode->PrintAll();

	CVideoBridgeInitParams* pVideoBridgeInitParams = new CVideoBridgeInitParams(this, m_name, m_ConfRsrcId,
	                                                                            vbImplementaionType, pVisualEffects, pLectureMode,
											m_pCommConf->GetIsSameLayout(),m_pCommConf->GetIsAutoLayout(),
											m_pCommConf->GetVideoQuality(), m_pCommConf->GetIsVideoClarityEnabled(),
											m_pCommConf, m_pCommConf->GetIsSiteNamesEnabled(),m_pCommConf->GetMuteAllPartiesVideoExceptLeader());


	if (eVideoCOP_Bridge_V1 == vbImplementaionType)
	{
		// COP_VIDEO_BRIDGE
		// add encoders level information to pVideoBridgeInitParams
		pVideoBridgeInitParams->SetCopRsrcsParams(m_pCopRsrcs);
	}

	POBJDELETE(pVisualEffects);
	POBJDELETE(pLectureMode);

	if (eVideoXcode_Bridge_V1 != vbImplementaionType)
	{
		m_pVideoBridgeInterface->Create(pVideoBridgeInitParams);
		if (m_pCommConf->GetIsTelePresenceMode())
			m_pVideoBridgeInterface->TurnOnOffTelePresence(TRUE, NULL);
	}
	else
	{
		InitAllXCodePortsParams(pVideoBridgeInitParams);
		m_pContentXcodeBridgeInterface = new CVideoBridgeInterface;
		CVideoBridgePartyCntlContent* pContentDecoder = m_pVideoBridgeInterface->GetContentDecoder();
		pVideoBridgeInitParams->SetVideoBridgeContentDecoder(pContentDecoder);
		m_pContentXcodeBridgeInterface->Create(pVideoBridgeInitParams);
	}

	POBJDELETE(pVideoBridgeInitParams);

	m_pCommConf->SetIsVideoPlusConf(YES);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CConf::InitAllXCodePortsParams(CVideoBridgeInitParams* pVideoBridgeInitParams)
{
	TRACEINTO << "ConfName:" << m_name;

	XCODE_INIT_PARAMS_LIST* pVideoBridgeXCodeInitParamsList = pVideoBridgeInitParams->GetXCodeInitParamsList();
	XCODE_RESOURCES_MAP* pXCodeRsrcMap = pVideoBridgeInitParams->GetXCodeResourcesMap();
	if (pVideoBridgeXCodeInitParamsList == NULL || pXCodeRsrcMap == NULL)
		PASSERT_AND_RETURN(1);

	CBridgePartyVideoParams* pBridgePartyVideoParams = NULL;

	for (XCODE_RSRC::iterator _ii = m_mapXCodeRsrc.begin(); _ii != m_mapXCodeRsrc.end(); _ii++)
	{
		eXcodeRsrcType eXCodeResourceType = _ii->first;
		ALLOC_PARTY_PARAMS_S* pAllocPartyParams = _ii->second;
		pBridgePartyVideoParams = new CBridgePartyVideoParams;
		InitPartyXCodeParamsAccordingToEncoderType(eXCodeResourceType, pBridgePartyVideoParams);

		ContentXcodeRsrcDesc* pContentXcodeRsrcDesc = new ContentXcodeRsrcDesc;

		pContentXcodeRsrcDesc->connectionId       = pAllocPartyParams->allocInd.allocIndBase.allocatedRrcs[0].connectionId;
		pContentXcodeRsrcDesc->logicalRsrcType    = pAllocPartyParams->allocInd.allocIndBase.allocatedRrcs[0].logicalRsrcType;
		pContentXcodeRsrcDesc->rsrcEntityId       = pAllocPartyParams->allocInd.allocIndBase.rsrc_party_id;
		pContentXcodeRsrcDesc->videoPartyType     = pAllocPartyParams->allocInd.allocIndBase.videoPartyType;
		pContentXcodeRsrcDesc->monitorRsrcPartyId = pAllocPartyParams->partyId;
		pVideoBridgeXCodeInitParamsList->insert(XCODE_INIT_PARAMS_LIST::value_type(eXCodeResourceType, pBridgePartyVideoParams));
		pXCodeRsrcMap->insert(XCODE_RESOURCES_MAP::value_type(eXCodeResourceType, pContentXcodeRsrcDesc));
	}
}

/////////////////////////////////////////////////////////////////////////////
void CConf::InitPartyXCodeParamsAccordingToEncoderType(eXcodeRsrcType eEncoderPartyType, CBridgePartyVideoParams* pBridgePartyVideoParams, BYTE isForUpdateEncoder)
{
	TRACEINTO << "ConfName:" << m_name;

	BYTE   lConfRate           = m_pCommConf->GetConfTransferRate();
	BYTE   actualContentRate   = 0;
	DWORD  videoBridgeBitRate  = 0;
	APIU16 profile             = 0;
	long   fs                  = INVALID;
	long   mbps                = INVALID;
	long   sar                 = 0;
	long   staticMB            = DEFAULT_STATIC_MB;
	long   dpb                 = DEFAULT_STATIC_MB;
	long   brAndCpb            = 0;
	APIU8  level               = 0;
	DWORD  videoBridgeFs       = INVALID;
	DWORD  videoBridgeMbps     = INVALID;
	DWORD  videoBridgeSar      = 0;
	DWORD  videoBridgeStaticMB = DEFAULT_STATIC_MB;
	DWORD  videoBridgeDPB      = INVALID;

	switch (eEncoderPartyType)
	{
		case eXcodeContentDecoder:
		{
			actualContentRate = SelectContentRate();
			videoBridgeBitRate = TranslateAMCRateIPRate(actualContentRate)*100;
			if (m_contentXcodeSrcProtocol == H264)
			{
				const CVidModeH323& rH264ContentMode = (const CVidModeH323 &)m_pUnifiedComMode->GetIPComMode()->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
				rH264ContentMode.GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
				pBridgePartyVideoParams->SetVideoAlgorithm(H264);
			}

			if (m_contentXcodeSrcProtocol == H263)
			{
				pBridgePartyVideoParams->SetVideoAlgorithm(H263);
				pBridgePartyVideoParams->SetVideoResolution(eVideoResolutionXGA);
				pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionQCIF, eVideoFrameRate30FPS);
				pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionCIF, eVideoFrameRate30FPS);
				pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolution4CIF, eVideoFrameRate15FPS);
				pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionVGA, eVideoFrameRate15FPS);
				pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionSVGA, eVideoFrameRate10FPS);
				pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionXGA, eVideoFrameRate7_5FPS);
			}
			break;
		}

		case eXcodeH264LinksEncoder:
		case eXcodeH264Encoder:
		{
			BYTE HDMpi = 0;
			EHDResolution eHDRes;
			BYTE isHDContent1080Supported = FALSE;
			eEnterpriseMode ContRatelevel = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
			if (eEncoderPartyType == eXcodeH264LinksEncoder)
			{
				actualContentRate = m_pUnifiedComMode->FindAMCContentRateByLevel(lConfRate, (eConfMediaType)(m_pCommConf->GetConfMediaType()), ContRatelevel, eH264Fix, m_pCommConf->GetCascadeOptimizeResolution());
				isHDContent1080Supported = SelectContentHDResolution(actualContentRate, H264, HDMpi, FALSE, eEncoderPartyType);
			}
			else
			{
				BYTE MaxContentRate = m_pUnifiedComMode->FindAMCContentRateByLevel(lConfRate, (eConfMediaType)(m_pCommConf->GetConfMediaType()), ContRatelevel, eH264Dynamic, e_res_dummy);
				actualContentRate = SelectContentRate(FALSE, eEncoderPartyType);
				if (isForUpdateEncoder)
				{
					isHDContent1080Supported = SelectContentHDResolution(actualContentRate, H264, HDMpi, TRUE, eEncoderPartyType); // Highest Common is TRUE
				}
				else
				{
					isHDContent1080Supported = SelectContentHDResolution(MaxContentRate, H264, HDMpi, FALSE, eEncoderPartyType);   // Highest Common is FALSE
				}
			}

			videoBridgeBitRate = TranslateAMCRateIPRate(actualContentRate)*100;
			if (isHDContent1080Supported)
				eHDRes = eHD1080Res;
			else
				eHDRes = eHD720Res;

			CIpComMode* pH264ContentScm = new CIpComMode;
			BYTE bContentAsVideo = m_pCommConf->IsLegacyShowContentAsVideo();
			pH264ContentScm->RemoveContent(cmCapReceiveAndTransmit);
			pH264ContentScm->SetIsShowContentAsVideo(bContentAsVideo);
			pH264ContentScm->SetHDContent(videoBridgeBitRate, cmCapReceiveAndTransmit, eHDRes, HDMpi);
			const CVidModeH323& rH264ContentMode = (const CVidModeH323 &)pH264ContentScm->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
			rH264ContentMode.GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
			POBJDELETE(pH264ContentScm);
			pBridgePartyVideoParams->SetVideoAlgorithm(H264);
			pBridgePartyVideoParams->SetVideoResolution(eVideoResolutionDummy);
			pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionQCIF, eVideoFrameRateDUMMY);
			pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionCIF, eVideoFrameRateDUMMY);
			pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolution4CIF, eVideoFrameRateDUMMY);
			pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionVGA, eVideoFrameRateDUMMY);
			pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionSVGA, eVideoFrameRateDUMMY);
			pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionXGA, eVideoFrameRateDUMMY);
			break;
		}

		case eXcodeH263Encoder:
		{
			pBridgePartyVideoParams->SetVideoAlgorithm(H263);
			actualContentRate  = SelectContentRate(FALSE, eEncoderPartyType);
			videoBridgeBitRate = TranslateAMCRateIPRate(actualContentRate)*100;
			if(actualContentRate <= AMC_64k)
			{
				pBridgePartyVideoParams->SetVideoResolution(eVideoResolutionCIF);
				pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionCIF, eVideoFrameRate7_5FPS);
			}
			else if(actualContentRate <= AMC_128k)
			{
				pBridgePartyVideoParams->SetVideoResolution(eVideoResolutionCIF);
				pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionCIF, eVideoFrameRate15FPS);
			}
			else
			{
			pBridgePartyVideoParams->SetVideoResolution(eVideoResolutionXGA);
			pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionCIF, eVideoFrameRate30FPS);
			pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolution4CIF, eVideoFrameRate15FPS);
			pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionVGA, eVideoFrameRate15FPS);
			pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionSVGA, eVideoFrameRate10FPS);
			pBridgePartyVideoParams->SetVideoFrameRate(eVideoResolutionXGA, eVideoFrameRate7_5FPS);
			}
			pBridgePartyVideoParams->SetProfile(eVideoProfileDummy);
			pBridgePartyVideoParams->SetPacketFormat(eVideoPacketPayloadFormatDummy);
			pBridgePartyVideoParams->SetMBPS(videoBridgeMbps);
			pBridgePartyVideoParams->SetFS(videoBridgeFs);
			pBridgePartyVideoParams->SetSampleAspectRatio(videoBridgeSar);
			pBridgePartyVideoParams->SetStaticMB(videoBridgeStaticMB);
			pBridgePartyVideoParams->SetMaxDPB(videoBridgeDPB);
			pBridgePartyVideoParams->SetIsH263Plus(TRUE);

			break;
		}

		default:
			PASSERT(eEncoderPartyType+1);
	} // switch

	if (eEncoderPartyType != eXcodeH263Encoder)
	{
		CH264Details thisH264Details = level;
		if (fs == -1)
			fs = thisH264Details.GetDefaultFsAsDevision();

		if (mbps == -1)
			mbps = thisH264Details.GetDefaultMbpsAsDevision();

		if (sar == -1)
			sar = 0;

		if (staticMB == -1)
			staticMB = DEFAULT_STATIC_MB;

		// Calculate Vide Bridge Values
		videoBridgeFs       = (DWORD)fs;
		videoBridgeMbps     = (DWORD)mbps;
		videoBridgeSar      = (DWORD)sar;
		videoBridgeStaticMB = (DWORD)staticMB;
		videoBridgeDPB      = (DWORD)dpb;


		pBridgePartyVideoParams->SetProfile(eVideoProfileBaseline);
		pBridgePartyVideoParams->SetPacketFormat(eVideoPacketPayloadFormatDummy);
		pBridgePartyVideoParams->SetMBPS(videoBridgeMbps);
		pBridgePartyVideoParams->SetFS(videoBridgeFs);
		pBridgePartyVideoParams->SetSampleAspectRatio(videoBridgeSar);
		pBridgePartyVideoParams->SetStaticMB(videoBridgeStaticMB);
		pBridgePartyVideoParams->SetMaxDPB(videoBridgeDPB);
	}

	pBridgePartyVideoParams->SetVideoBitRate(videoBridgeBitRate);
	pBridgePartyVideoParams->SetIsTipMode(NO);
}

/////////////////////////////////////////////////////////////////////////////
void CConf::ActionsOnStartingContentSession(BYTE selectedContentRateAMC, bool isDowngradingfToContentVSW)
{
	TRACEINTO << "ConfName:" << m_name;

	if (selectedContentRateAMC == AMC_0k)
		return;

	// 2. set new content rate to UnifideComMode
	SetNewContentBitRate(selectedContentRateAMC);

	// 3 . Update Content Bridge - CHANGE_RATE
	if (!CPObject::IsValidPObjectPtr(m_pContentBridge))
		PASSERT_AND_RETURN(1);

	m_pContentBridge->ContentRate(selectedContentRateAMC);

	// 4. ChangeMode for all parties
	if (isDowngradingfToContentVSW)
	{
		ChangeContentMode(NULL, TRUE, TRUE, isDowngradingfToContentVSW);
		TRACEINTO << "ConfName:" << m_name << " - Downgrade from XCode to Content VSW, initiate content change mode for all parties";
	}
	else
		ChangeContentMode(NULL, TRUE, FALSE, FALSE);

	// 5. StartContent to VideoBridge in Legacy content as video confrences
	BYTE activeProtocol = GetCurrentContentProtocolInConfValues();
	DWORD activeContentRate = 100*TranslateAMCRateIPRate(selectedContentRateAMC);
	if (m_pCommConf->GetContentMultiResolutionEnabled())
	{
		activeProtocol = m_contentXcodeSrcProtocol;
		activeContentRate = SelectMaxContentRateForXCodeDecoder();
	}

	InformVideoBridgeStartContentPresentation(activeProtocol, activeContentRate);
}

/////////////////////////////////////////////////////////////////////////////
void CConf::InitCopEncodersVideoOutParams(CVideoBridgeInitParams* pVideoBridgeInitParams)
{
}

/////////////////////////////////////////////////////////////////////////////
void CConf::GetCopLevelVideoParams(WORD encoder_index,DWORD& videoAlg,DWORD& videoBitRate,eVideoResolution& videoResolution,eVideoFrameRate&  videoFrameRate,DWORD& h264_MBPS,DWORD& h264_FS,DWORD& sampleAspectRatio)
{
}

/////////////////////////////////////////////////////////////////////////////
void CConf::GetCopEncoderResourceParams(WORD encoder_index,DWORD& copConnectionId,DWORD& copPartyId,eLogicalResourceTypes& copLrt)
{
}

/////////////////////////////////////////////////////////////////////////////
void CConf::CreateCopEncoderVideoOutParams(CBridgePartyVideoOutParams& bridgePartyVideoOutParams,DWORD videoAlg,DWORD videoBitRate,eVideoResolution videoResolution,eVideoFrameRate  videoFrameRate,DWORD h264_MBPS,DWORD h264_FS,DWORD sampleAspectRatio,DWORD copConnectionId,DWORD copPartyId,WORD copResourceIndex,eLogicalResourceTypes copLrt)
{
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConf::CreateFECCBridge()
{
	CFECCBridgeInitParams* pFECCBBridgeInitParams = new CFECCBridgeInitParams(this, m_name,m_ConfRsrcId, eFECC_Bridge_V1 ,m_pCommConf->GetLSDRate ());

	m_pFECCBridge->Create(pFECCBBridgeInitParams);

	POBJDELETE(pFECCBBridgeInitParams);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConf::CreateInActiveContentBridge()
{
	BYTE contentProtocolMode = GetCurrentContentProtocolInConfValues();

	//HP content
	BYTE contentH264HighProfile = FALSE;
	if (contentProtocolMode == H264)
		contentH264HighProfile = GetCurrentContentIsHighProfile();

	CContentBridgeInitParams* pContentBridgeInitParams = new CContentBridgeInitParams(this, m_name,m_ConfRsrcId, eContent_Bridge_V1, contentProtocolMode, GetCommConf()->IsExclusiveContentMode(), contentH264HighProfile);

	m_pContentBridge->CreateInActive(pContentBridgeInitParams);

	POBJDELETE(pContentBridgeInitParams);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConf::CreateContentBridge()
{
	BYTE contentProtocolMode = GetCurrentContentProtocolInConfValues();

	//HP content
	BYTE contentH264HighProfile = FALSE;
	if (contentProtocolMode == H264)
		contentH264HighProfile = GetCurrentContentIsHighProfile();

	CContentBridgeInitParams* pContentBridgeInitParams = new CContentBridgeInitParams(this, m_name,m_ConfRsrcId, eContent_Bridge_V1, contentProtocolMode, GetCommConf()->IsExclusiveContentMode(), contentH264HighProfile);

	m_pContentBridge->Create(pContentBridgeInitParams);

	POBJDELETE(pContentBridgeInitParams);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
WORD CConf::CheckIfIVRServiceIsOn()
{
	CAVmsgServiceList*  pAVmsgServiceList = ::GetpAVmsgServList();
	PASSERT_AND_RETURN_VALUE(!pAVmsgServiceList, 0);

	CAvMsgStruct* pAvMsgStruct = m_pCommConf->GetpAvMsgStruct();
	PASSERT_AND_RETURN_VALUE(!pAvMsgStruct, 0);

	const char* pAvServiceName = pAvMsgStruct->GetAvMsgServiceName();
	CAVmsgService* pAvService = pAVmsgServiceList->GetCurrentAVmsgService(pAvServiceName);

	return (pAvService) ? 1 : 0;
}

/////////////////////////////////////////////////////////////////////////////
WORD CConf::AreAllPartiesDisconnected(WORD& numOfNonDisconnected,bool ignoreItpSlaves)
{
	numOfNonDisconnected = 0;

	WORD numParties = m_pCommConf->GetNumParties();
	ELastQuitType lastQuitType = (ELastQuitType)m_pCommConf->GetLastQuitType();

	std::ostringstream msg;
	msg.precision(0);
	msg << "IsGateWay:" << (int)m_isGateWay << ", LastQuitType:" << LastQuitTypeToString(lastQuitType) << endl;

	if (numParties > 0)
	{
		msg << " --------+-------+---------+---------------" << endl;
		msg << " Cascade | State | PartyId | PartyName"      << endl;
		msg << " --------+-------+---------+---------------" << endl;

		CConfParty*  pConfParty = m_pCommConf->GetFirstParty(); // get the first party

		for (WORD i = 0; i < numParties; i++)
		{
			if (pConfParty)
			{
				WORD  party_cascade = 0;
				DWORD party_state   = PARTY_DISCONNECTED;

		        if (pConfParty->GetRecordingPort() || pConfParty->GetRecordingLinkParty() || ( ignoreItpSlaves && pConfParty->IsTIPSlaveParty() ) || pConfParty->GetAvMcuLinkType() == eAvMcuLinkSlaveOut || pConfParty->GetAvMcuLinkType() == eAvMcuLinkSlaveIn || pConfParty->GetIsLyncPlugin() )
					party_state = PARTY_DISCONNECTED;                 // recording port is not in count
				else
					party_state = pConfParty->GetPartyState();

				switch (party_state)
				{
					case PARTY_DISCONNECTED:
					case PARTY_STAND_BY:
					case PARTY_IDLE:
					case PARTY_WAITING_FOR_DIAL_IN:
					case PARTY_DELETED_BY_OPERATOR:
					{
						party_cascade = 0;
						break;
					}

					default:
					{
						numOfNonDisconnected++;
						// vngfe-4087 - H323/H320 Gateway-When H323 call is released, ISDN connection is maintened if far end endpoint is behind MCU (cascade link)
						// added condition && !m_isGateWay && lastQuitType==eTerminateWithLastRemains to the Amdocs patch
						party_cascade = pConfParty->GetCascadeMode();
						if (party_cascade == CASCADE_MODE_MASTER && !m_isGateWay && lastQuitType == eTerminateWithLastRemains)
							numOfNonDisconnected++;
						break;
					}
				}

				msg << " " << setw( 7) << right << party_cascade << " |"
						<< " " << setw( 5) << right << party_state << " |"
						<< " " << setw( 7) << right << pConfParty->GetPartyId() << " |"
						<< " " <<             left  << pConfParty->GetName() << endl;

				pConfParty = m_pCommConf->GetNextParty();
			}
		}
		msg << " --------+-------+---------+---------------" << endl;
	}

	msg << "numOfNonDisconnected:" << numOfNonDisconnected;
	TRACEINTO << msg.str().c_str();

	return numOfNonDisconnected ? FALSE : TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CConf::MuteVideoOfCascadeLinksIfNoVideoParties()
{
	// not count the recording port.
	DWORD party_state = 0;
	BOOL bAvoidVideoLoopBack = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("AVOID_VIDEO_LOOP_BACK_IN_CASCADE", bAvoidVideoLoopBack);
	TRACECOND_AND_RETURN(!bAvoidVideoLoopBack, "The system flag AVOID_VIDEO_LOOP_BACK_IN_CASCADE is set to NO - exit function");

	// Find if the only parties with active video are cascaded link.
	CConfParty* pConfParty = m_pCommConf->GetFirstParty(); // get the first party
	WORD numParties = m_pCommConf->GetNumParties();
	WORD numOfVideoNonCascadeParties = 0;

	for (WORD i = 0; i < numParties; i++)
	{
		if (pConfParty)
		{
			if (pConfParty->GetRecordingPort() || pConfParty->GetRecordingLinkParty())
				party_state = PARTY_DISCONNECTED; // recording port is not in count
			else
				party_state = pConfParty->GetPartyState();

			switch (party_state)
			{
				case PARTY_CONNECTED:
				case PARTY_CONNECTED_WITH_PROBLEM:
				{
					if (!pConfParty->GetCascadeMode() && !pConfParty->GetVoice())
						numOfVideoNonCascadeParties++;
					pConfParty = m_pCommConf->GetNextParty();
					break;
				}

				default:
				{
					pConfParty = m_pCommConf->GetNextParty();
					break;
				}
			} // switch
		}
	}

	std::ostringstream msg;
	msg << "numParties:" << numParties << ", numOfVideoNonCascadeParties:" << numOfVideoNonCascadeParties;

	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (!pPartyConnection)
			continue;

		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		if (!pPartyCntl)
			continue;

		if (!strcmp(pPartyCntl->NameOf(), "CH323DelPartyCntl"))
			continue;

		if ((pPartyConnection->GetInterfaceType() == SIP_INTERFACE_TYPE) || (pPartyConnection->GetInterfaceType() == ISDN_INTERFACE_TYPE))
			continue;

		// if party is secondary or connected with problems
		pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
		if (!pConfParty || !(pConfParty->GetPartyState() == PARTY_CONNECTED || pConfParty->GetPartyState() == PARTY_CONNECTED_WITH_PROBLEM))
			continue;

		if (pConfParty->GetRecordingPort() || pConfParty->GetRecordingLinkParty())
			continue;

		if (!pConfParty->GetCascadeMode() || pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone)
			continue;

		if (!numOfVideoNonCascadeParties) // Integration with Michael's API.
		{ // send mute if active
			if (pPartyCntl->GetIsVideoMuted() == NO)
			{
				pPartyCntl->PartySendMuteVideo(NO);
				TRACEINTO << pPartyConnection->GetFullName() << " - Sent video mute to video cascade link";
			}
		}
		else
		{
			// send unmute if muted
			if (pPartyCntl->GetIsVideoMuted() == YES)
			{
				pPartyCntl->PartySendMuteVideo(YES);
				TRACEINTO << pPartyConnection->GetFullName() << " - Sent video unmute to video cascade link";
			}
			else
			{
				TRACEINTO << pPartyConnection->GetFullName() << " - Cascade link is not muted";
			}
		}
	}
	TRACEINTO << msg.str().c_str();
}

/////////////////////////////////////////////////////////////////////////////
void CConf::OnPartyEndDel(CSegment* pParam)  // shiraITP - 140
{
	CTaskApp* pParty;
	WORD      status;
	*pParam >> (DWORD&)pParty >> status;

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	PASSERT_AND_RETURN(!pPartyConnection);

	// if the party was in the operator-help queue
	PartyMonitorID PartyID = pPartyConnection->GetMonitorPartyId();

	TRACEINTO << "MonitorPartyId:" << PartyID << ", PartyName:" << pPartyConnection->GetName();

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(PartyID);
	if (pConfParty)
	{
		if (m_pGatheringManager)
			m_pGatheringManager->OnPartyDisConnected(pConfParty);

		pConfParty->SetWaitForOperAssistance(0);
		if (pPartyConnection->GetDialType() == DIALIN && !pConfParty->IsUndefinedParty())
		{
			TRACEINTO << "MonitorPartyId:" << PartyID << " - Setting reservation defaults for IP and Alias...";
			pConfParty->SetIpAddress(pConfParty->GetBackupIpAddress());
			pConfParty->SetH323PartyAlias(pConfParty->GetBackupH323PartyAlias());
		}

		// VNGFE_5887: Party task finished, reset pointer to NULL
		pConfParty->SetTask(NULL);
	}

	// 26449
	SendSecureMessageWhenDelParty(pPartyConnection);


	ActivateAutoTermenation("CConf::OnPartyEndDel");
	if( pConfParty && strlen(m_pCommConf->GetFocusUriScheduling())  && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveOut && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveIn &&  pConfParty->GetAvMcuLinkType() != eAvMcuLinkMain)
			ActivateAutoTerminationForMsAvMcu("CConf::OnPartyEndDel");

	// Re-calculate Conference TelePresence Mode
	DetermineTelePresenceConfMode(0);
	if (!pPartyConnection->GetDisconnectMode() || m_state == TERMINATION) // party delete
	{
		TRACEINTO << "MonitorPartyId:" << PartyID << ", DisconnectMode:" << pPartyConnection->GetDisconnectMode() << ", State:" << ConfStateToString(m_state) << " - Delete party from conference...";
		m_pVideoBridgeInterface->DeletePartyFromConf(pPartyConnection->GetName());
		RemovePartyConnection(pParty, (BYTE)1);
	}
	else // party disconnect
	{
		// Disconnect party should clean the deleted party from the VideoBridge
		if (pPartyConnection->GetDialType() == DIALIN)
		{
			if (pConfParty && pConfParty->IsUndefinedParty())
			{
				TRACEINTO << "MonitorPartyId:" << PartyID << " - Undefined dial-in disconnected instead of deleted";
			}

			// In case of video plus we update the bridge about its maps status.
			RemovePartyConnection(pParty, (BYTE)0);
		}
		else // dial out party will remain with its own resources
		{
			// In case of video plus we update the bridge about its maps status.
		}
	}

	//If this party is counted as a audio participant, update the layout indication
	if(pConfParty && pConfParty->IsCountedInAudioIndication())
	{
		UpdateAudioParticipantsCount(pConfParty,FALSE);
	}

	UpdateConfStatus();

	std::map<DWORD, PartyMonitorID>* taskIdToPartyId = GetMapPartiesTasksIds();
	DWORD taskId;
	for (TaskIdToPartyId::iterator itr = taskIdToPartyId->begin(); itr != taskIdToPartyId->end(); ++itr)
	{
		if (itr->second == PartyID)
		{
			taskId = itr->first;

			if (taskId != 0)
			{
				if (taskIdToPartyId->find(taskId) != taskIdToPartyId->end())
				{
					GetMapPartiesTasksIds()->erase(taskId);
				}
			}

			break;
		}
	}

	// Multiple links for ITP in cascaded conference feature: CConf::OnPartyEndDel - disconnectAllRoom if needed (update main link -> will update room Control) & setOriginalName
	if (pPartyConnection && pConfParty && CPObject::IsValidPObjectPtr(pConfParty))
	{
		TRACEINTO << "PartyName:" << pConfParty->GetName() << ", DisconnectCause:" << pConfParty->GetDisconnectCause();

		if (pConfParty->GetPartyType() == eSubLinkParty && pPartyConnection->GetDialType() == DIALOUT &&
		    (pConfParty->GetDisconnectCause() == RESOURCES_DEFICIENCY || pConfParty->GetDisconnectCause() == SUB_OR_MAIN_LINK_IS_SECONDARY))
		{
			TRACEINTO << "PartyName:" << pConfParty->GetName() << ", PartyType:" << pConfParty->GetPartyType() << " - eSubLinkParty is secondary need to disconnect the room";

			// disconnect mainLink -> will disconnect all the subs in his room..
			char mainPartyName[H243_NAME_LEN];
			::GetMainLinkName(pPartyConnection->GetName(), (char*)mainPartyName);

			CPartyConnection* pMainPartyConnection = GetPartyConnection(mainPartyName);
			CPartyCntl* pMainPartyCntl = NULL;

			if (pMainPartyConnection)
				pMainPartyCntl = pMainPartyConnection->GetPartyCntl();

			if (pMainPartyCntl && CPObject::IsValidPObjectPtr(pMainPartyCntl))
			{
				CTaskApp* pMainParty = pMainPartyCntl->GetPartyTaskApp();
				if (pConfParty->GetDisconnectCause() == SUB_OR_MAIN_LINK_IS_SECONDARY && pMainParty)
				{
					UpdateDB(pMainParty, DISCAUSE, SUB_OR_MAIN_LINK_IS_SECONDARY /*cause*/, NULL, 0 /*MipErrorNumber*/);
					TRACEINTO << "PartyName:" << pConfParty->GetName() << " - Update MainLink with disconnect cause SUB_OR_MAIN_LINK_IS_SECONDARY";
				}
				else if (pConfParty->GetDisconnectCause() == RESOURCES_DEFICIENCY && pMainParty)
				{
					UpdateDB(pMainParty, DISCAUSE, RESOURCES_DEFICIENCY /*cause*/, NULL, 0 /*MipErrorNumber*/);
					TRACEINTO << "PartyName:" << pConfParty->GetName() << " - Update MainLink with disconnect cause RESOURCES_DEFICIENCY";
				}
				else
					PASSERTMSG(1, "MainLink has no pMainParty");
			}
			else
				PASSERTMSG(1, "MainLink has no pMainPartyCntl");

			if (pMainPartyConnection)
			{
				TRACEINTO << "MonitorPartyId:" << PartyID << " - Disconnect MainLink";
				pMainPartyConnection->DisconnectH323(1);         // mode = 1 = disconnect party
			}
			else
				PASSERTMSG(1, "pMainPartyConnection is NULL");
		}
		else if (pConfParty->GetPartyType() == eSubLinkParty)
		{
			SendToMainLinkThatSubWasDisconnectedOrDeleted(pPartyConnection);
		}
		else if (pConfParty->GetPartyType() == eMainLinkParty)
		{
			TRACEINTO << "PartyName:" << pConfParty->GetName() << ", PartyType:" << pConfParty->GetPartyType() << ", DisconnectCause:" << pConfParty->GetDisconnectCause();

			if (pPartyConnection->GetDialType() == DIALOUT)
			{
				BYTE cascadedLinkNunber = pConfParty->GetCascadedLinksNumber();
				DisconnectAllRoomControlLinks(pPartyConnection->GetName(), cascadedLinkNunber, 0 /*delete*/);
			}

			if (!pConfParty->IsUndefinedParty())
				SetOriginalName(pPartyConnection, pConfParty);
		}
	}

	if(pConfParty && strlen(pConfParty->GetMsConversationId()))
	{
		DWORD TmpPartyId = FindMatchingConversationId(pConfParty);
		if(TmpPartyId)
		{
			CConfParty* pAvMCUConfParty = (CConfParty*)::GetpConfDB()->GetCurrentParty(m_monitorConfId, TmpPartyId);
			PASSERT_AND_RETURN(!pAvMCUConfParty);

			if(pAvMCUConfParty->GetMsftMediaEscalationStatus() == eMsftEscalationInActive)
			{
			//	pAvMCUConfParty->SetMsMediaEscalationStatus(eMsftEscalationActive);

				CPartyConnection*  pAVMCUPartyConnection = NULL;

				//pAVMCUPartyConnection = ((CConf*)this)->GetPartyConnection(pAvMCUConfParty->GetName());

		//		pAVMCUPartyConnection->UpdateAVMCUPartyToStartMedia();



			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
void CConf::SetOriginalName(CPartyConnection* pPartyConnection, CConfParty* pConfParty)  // shiraITP - 142
{
	const char* partyName = pPartyConnection->GetName();
	char* posChar = (char*)strrchr(partyName, '_');

	if (posChar != NULL)
	{
		char originalName[H243_NAME_LEN];

		int pos = posChar - (partyName);
		pos = min(pos, H243_NAME_LEN-1);
		strncpy(originalName, partyName, pos);
		originalName[pos] = '\0';

		pPartyConnection->SetPartyName(originalName);
		pConfParty->SetName(originalName);

		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		pPartyCntl->SetFullName(originalName, m_name);

		pConfParty->SetVisualPartyName(originalName);
	}
	else
	{
		FTRACESTRFUNC(eLevelError) << "PartyName:" << partyName << " - There is no '_' symbol in party name";
	}
}

//////////////////////////////////////////////////////////////////////////////
void CConf::SendToMainLinkThatSubWasDisconnectedOrDeleted(CPartyConnection* pPartyConnection)  // shiraITP - 141
{
	const char* subPartyName = pPartyConnection->GetName();
	char mainPartyName[H243_NAME_LEN];
	::GetMainLinkName(subPartyName, (char*)mainPartyName);

	TRACEINTO << "MainLinkName:" << mainPartyName;

	CPartyConnection* pMainPartyConnection = GetPartyConnection(mainPartyName);
	if (pMainPartyConnection != NULL)
	{
		CH323PartyCntl* pPartyCntl = (CH323PartyCntl*)(pMainPartyConnection->GetPartyCntl());
		if (CPObject::IsValidPObjectPtr(pPartyCntl))
		{
			char* pch1 = (char*)strrchr(subPartyName, '_');
			DWORD indexOfSub = (DWORD)atoi(pch1+1);

			TRACEINTO << "Index:" << indexOfSub-1;
			pPartyCntl->OnRemoveSubLinkFromRoomControl(subPartyName, indexOfSub-1);
		}
	}
	else
	{
		TRACEWARN << "MainParty was not found (already disconnected)";
	}
}

/////////////////////////////////////////////////////////////////////////////
void CConf::ConnectParty(CConfParty* pConfParty, DWORD connectDelay) //shiraITP - 3
{
	PASSERT_AND_RETURN(!pConfParty);

	if (!m_StandByStart)// Only at start Immediately case - delete Auto termination.
		DeleteTimer(AUTOTERMINATE);

	TRACEINTO << "MonitorPartyId:" << pConfParty->GetPartyId() << ", PartyName:" << pConfParty->GetName();

	// for H323 / VOIP
	if (pConfParty->IsIpNetInterfaceType())
	{
		ConnectIpParty(pConfParty, /*avServiceNameStr*/'\0', /*welcomeMsgTime*/0,/*pDialInConfParty,*/connectDelay/*, pMsgDesc*/);
		return;
	}
	// for PSTN / ISDN
	else if (pConfParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE)
	{
		ConnectPstnIsdnParty(pConfParty, /*avServiceNameStr*/'\0', /*welcomeMsgTime*/0, connectDelay);
		return;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CConf::ConnectDMAAVMCUParty(CConfParty* pConfParty, DWORD connectDelay)
{
	PASSERT_AND_RETURN(!pConfParty);

	if (!m_StandByStart) // Only at start Immediately case - delete Auto termination.
		DeleteTimer(AUTOTERMINATE);

	// SET new partyId to party
	if (pConfParty &&  pConfParty->GetPartyId() == 0xFFFFFFFF)
		pConfParty->SetPartyId(m_pCommConf->NextPartyId());



	TRACEINTO << "MonitorPartyId:" << pConfParty->GetPartyId() << ", PartyName:" << pConfParty->GetName();

	CPartyConnection* pPartyConnection = new CPartyConnection;

	const char* avServiceNameStr = '\0';
	WORD welcomeMsgTime   = 0;

	PartyControlDataParameters partyControlDataParams;
	memset(&partyControlDataParams, 0, sizeof (partyControlDataParams));
	PartyControlInitParameters partyControInitParam;
	memset(&partyControInitParam, 0, sizeof (partyControInitParam));

	SetControlDataParams(partyControlDataParams, pConfParty, NULL, NULL, NULL, TRUE, FALSE, FALSE, Regular, AvMcuLync2013Main, 0, eRegularParty);
	SetControlInitParams(partyControInitParam, pConfParty, FALSE, avServiceNameStr, welcomeMsgTime, connectDelay, 0, NULL);

	pPartyConnection->ConnectDMAAVMCUIP(partyControInitParam, partyControlDataParams, pConfParty->GetFocusUri());
	InsertPartyConnection(pPartyConnection);
}

/////////////////////////////////////////////////////////////////////////////
void CConf::ConnectAVMCUParty(CSipNetSetup* pNetSetup, CConfParty* pConfParty, DWORD connectDelay, sipSdpAndHeaders* pSdpAndHeaders)
{
	PASSERT_AND_RETURN(!pConfParty);

	if (!m_StandByStart) // Only at start Immediately case - delete Auto termination.
		DeleteTimer(AUTOTERMINATE);

	TRACEINTO << "MonitorPartyId:" << pConfParty->GetPartyId() << ", PartyName:" << pConfParty->GetName();

	CPartyConnection* pPartyConnection = new CPartyConnection;

	const char* avServiceNameStr = '\0';
	WORD welcomeMsgTime   = 0;

	PartyControlDataParameters partyControlDataParams;
	memset(&partyControlDataParams, 0, sizeof (partyControlDataParams));
	PartyControlInitParameters partyControInitParam;
	memset(&partyControInitParam, 0, sizeof (partyControInitParam));

	SetControlDataParams(partyControlDataParams, pConfParty, NULL, NULL, NULL, TRUE, FALSE, FALSE, Regular, AvMcuLync2013Main, 0, eRegularParty);
	SetControlInitParams(partyControInitParam, pConfParty, FALSE, avServiceNameStr, welcomeMsgTime, connectDelay, 0, NULL);

	pPartyConnection->ConnectAVMCUIP(pNetSetup, partyControInitParam, partyControlDataParams, pSdpAndHeaders);
	InsertPartyConnection(pPartyConnection);
}

/////////////////////////////////////////////////////////////////////////////
void CConf::LedSysAlarmInd()
{
	std::string answer;
	eProductType procType = CProcessBase::GetProcess()->GetProductType();
	if ((procType == eProductTypeNinja) || (procType == eProductTypeGesher))
	{
		if ( !m_pCommConf->GetConnectedPartiesNumber())
		{
            /*Begin:Added by Richer for BRIDGE-15015, 11/13/2014*/
            SendLedSysAlarmIndToConfigurator (true);
            //SystemPipedCommand(("sudo "+MCU_MCMS_DIR+"/Bin/LightCli sysAlarm del_ep").c_str(),answer);
            /*End:Added by Richer for BRIDGE-15015, 11/13/2014*/
		}
		else
		{
            /*Begin:Added by Richer for BRIDGE-15015, 11/13/2014*/
            SendLedSysAlarmIndToConfigurator (false);
            //SystemPipedCommand(("sudo "+MCU_MCMS_DIR+"/Bin/LightCli sysAlarm add_ep").c_str(),answer);
            /*End:Added by Richer for BRIDGE-15015, 11/13/2014*/
		}
	}
	return;
}

/*Begin:Added by Richer for BRIDGE-15015, 11/13/2014*/
/////////////////////////////////////////////////////////////////////////////
STATUS CConf::SendLedSysAlarmIndToConfigurator (bool isDel) /*" true = del_ep","false = add_ep"*/
{
    DWORD LedSysAlarmInd = 0;

    if ( isDel )
    {
        LedSysAlarmInd = 1; /*"del_ep"*/
    }
    else
    {
        LedSysAlarmInd = 2; /*"add_ep"*/
    }

    CSegment*  pSeg = new CSegment;
    pSeg ->Put( (BYTE*) (&LedSysAlarmInd), sizeof(DWORD) );

    STATUS status = CProcessBase::GetProcess()->SendLedAlarmEventToConfigMngr(pSeg, CONFIGURATOR_LED_SYS_ALARM);

    return status;
    
}
/*End:Added by Richer for BRIDGE-15015, 11/13/2014*/


/////////////////////////////////////////////////////////////////////////////
void CConf::UpdateDB(CTaskApp* pParty, WORD type, DWORD param, CSegment* pMsg, DWORD MpiErrorNumber)
{
	CPartyConnection* pPartyConnection = (pParty == (CTaskApp*)0xffff) ? (CPartyConnection*)0xffff : GetPartyConnection(pParty);

	// This is in order to cover the case that we didn't manage to allocate a valid TaskApp for the party
	// - Rejected while allocating resources.
	// don't print here - to many prints - TRACEINTO << "pParty:" << (DWORD)pParty << " type:" << type << " param:" << param;
	if (pParty == NULL && (type == DISCAUSE || type == PARTY_REQUEST_TO_SPEAK || IPLOGICALCHANNELUPDATE == type))
	{
		if (pMsg == NULL)
		{
			TRACEINTO << "Failed, 'pParty' is NULL and 'pMsg' is NULL";
			return;
		}

		char partyName[H243_NAME_LEN];
		memset(partyName, '\0', sizeof(partyName));
		*pMsg >> partyName;
		pPartyConnection = GetPartyConnection(partyName);
		TRACEINTO << "Type:" << type <<  ", PartyName:" << partyName  << ", Param:" << param << ", PartyConnection:" << (hex) << pPartyConnection;
	}

	if (!pPartyConnection && (type != NOAUDIOSRC))
	{
		TRACEINTO << "Party not found";
	}
	else
	{
		CConfParty* pConfParty = NULL;
		if (type != NOAUDIOSRC)
		{
			if (pPartyConnection != (CPartyConnection*)0xffff)
			{
				pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
				if (!pConfParty)
					return;
			}
		}

		#define CHECK_PARTY_CONNECTION(s) if (pConfParty == NULL) { TRACEINTO << "Type:" << s << " - Failed, invalid party connection, PartyConnection:0xffff, Param:" << param; break; }

		switch (type)
		{
			case NOAUDIOSRC:
			{
				m_pCommConf->SetSourceAudioId(0xFFFFFFFF);
				break;
			}

			case AUDIOSRC:
			{
				CHECK_PARTY_CONNECTION("AUDIOSRC");
				m_pCommConf->SetSourceAudioId(pConfParty->GetPartyId());
				break;
			}

			case NOISE_DETECTION:
			{
				CHECK_PARTY_CONNECTION("NOISE_DETECTION");
				if (param == PARTY_CONNECTED)
					pConfParty->SetIsNoiseDetected(NO);
				else if (param == PARTY_CONNECTED_WITH_PROBLEM)
					pConfParty->SetIsNoiseDetected(YES);
				else
					DBGPASSERT(param ? param : 101);
				break;
			}

			case BLOCK_STATE:
			{
				CHECK_PARTY_CONNECTION("BLOCK_STATE");
				if (param == 0x00000000)
					pConfParty->SetAudioBlocked(FALSE);
				if (param & 0x00000001)
					pConfParty->SetAudioBlocked(TRUE);
				break;
			}

			case LSDSRC:
			{
				m_pCommConf->SetLSDRate((BYTE)param);
				if (pParty == (CTaskApp*)0xffff)
					m_pCommConf->SetLSDSourceId(0xffffffff);
				else
					m_pCommConf->SetLSDSourceId(pConfParty->GetPartyId());
				break;
			}

			case PARTYSTATE:
			{
				CHECK_PARTY_CONNECTION("PARTYSTATE");

				if (param == pConfParty->GetPartyState())
					break;

				TRACEINTO << "Type:PARTYSTATE, PartyName:" << pConfParty->GetName() << ", State:" << GetPartyStateStr(param) << "(" << (int)param << ")";

				DWORD oldState = pConfParty->GetPartyState();
				pConfParty->SetPartyState(param, m_pCommConf->GetMonitorConfId());
				if (IsValidPObjectPtr(m_pSvcEventPackage))
				{
					CSegment seg;
					seg << (DWORD)(pConfParty->GetPartyId()) << (WORD)type << (DWORD)param;
					m_pSvcEventPackage->HandleObserverUpdate(&seg, SIP_EVENT_PACKAGE);
				}

				UpdateConfStatus();
				//for RDP Gw: update the AV_MCU_FOCUS_URI to DAM when AV-MCU is connected
				if(pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveOut && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveIn )
				{
					if(param == PARTY_CONNECTING || param == PARTY_CONNECTED_PARTIALY || param == PARTY_CONNECTED || param == PARTY_CONNECTED_WITH_PROBLEM ||  param == PARTY_SECONDARY )
					{
						TRACEINTO << "SetIsNotifyAvMcuUri - true, PartyName:" << pConfParty->GetName() ;
						m_pCommConf->SetIsNotifyAvMcuUri(TRUE);
					}
					else
					{
						TRACEINTO << "SetIsNotifyAvMcuUri - false, PartyName:" << pConfParty->GetName() ;
						m_pCommConf->SetIsNotifyAvMcuUri(FALSE);
						//Disconnect the RDP Gw after AV-MCU leaves
						if(param == PARTY_DISCONNECTED)
							DisconnectRdpGw();
					}
				}
				if (param == PARTY_CONNECTED || param == PARTY_DISCONNECTED || param == PARTY_SECONDARY)
				{
					CStructTm curTime;
					STATUS    timeStatus = SystemGetTime(curTime);

					if (param == PARTY_CONNECTED || param == PARTY_SECONDARY)
					{
						pConfParty->SetConnectTime(curTime);
						/*VNGR-26326.  In case of hotbackup restore, we need to clear the flag after first connected*/
						pConfParty->SetFirstConnectionAfterHobackupRestore(FALSE);
					}
					if (param == PARTY_DISCONNECTED)
					{
						pConfParty->SetDisconnectTime(curTime);
						pConfParty->SetIpVideoBchCounter(0);
						pConfParty->SetIpProtocolSyncCounter(0);
						/* VNGR-23140: Operator's function here. We should not clean up it after disconnection.*/
						// pConfParty->SetAudioMuteByOperator(FALSE);
						// pConfParty->SetVideoMuteByOperator(FALSE);
						pConfParty->SetAudioMuteByParty(FALSE);
						pConfParty->SetVideoMuteByParty(FALSE);
						pConfParty->SetAudioMuteByMCU(FALSE);
						pConfParty->SetVideoMuteByMCU(FALSE);
						pConfParty->SetDefinedPartyAssigned(FALSE);
						pConfParty->SetIsLeader(0); // reset 'Leader' Icon
						pConfParty->SetOrdinaryParty(STATUS_PARTY_NONE); //reset "(IVR)" connectivity status, bridge-12482: change from inconf to none
						pConfParty->Set_Lpr_Rmt_SyncLoss(FALSE); // LPR
						pConfParty->Set_Lpr_Local_SyncLoss(FALSE);
						pConfParty->SetTelePresenceMode(eTelePresencePartyNone); // TelePresence Mode
						pConfParty->SetIsPartyCurrentlyEncrypted((BYTE)NO);
						pConfParty->Set_Is_Lpr_Headers_Activated((BYTE)NO);
					}
				}

				if (param == PARTY_STAND_BY) // increment Number Of Recording Ports in reservation
					pConfParty->SetRetriesNumber(0);

				if (param == PARTY_CONNECTED || param == PARTY_SECONDARY || param == PARTY_CONNECTED_WITH_PROBLEM)
				{
					pConfParty->SetRetriesNumber(0);
					if (oldState != PARTY_CONNECTED && oldState != PARTY_SECONDARY && oldState != PARTY_CONNECTED_WITH_PROBLEM)
					{
						// Increment 'connected parties' counter
						m_pCommConf->IncConnectedPartiesNum();
						/* Flora added for LED&LCD Feature */
						LedSysAlarmInd();
						if (m_isGateWay)
							OnGateWayPartyConnected(pConfParty);

						if (m_isDtmfInviteParty)
						{
							OnDtmfInvitedPartyConnected(pConfParty);
						}
						if (strstr(pConfParty->GetName(), "_InvitedByPcm") != NULL || strstr(pConfParty->GetAdditionalInfo(), "InvitedByPcm"))
							OnInvitedPartyConnected(pConfParty);
						if (strstr(pConfParty->GetName(), "_Invited") != NULL || strstr(pConfParty->GetAdditionalInfo(), "Invited"))
							pConfParty->SetAdditionalInfo("");
					}
				}

				// if a connected party is now disconnecting
				if (param == PARTY_DISCONNECTING && (oldState == PARTY_CONNECTED || oldState == PARTY_SECONDARY || oldState == PARTY_CONNECTED_WITH_PROBLEM))
				{
					m_pCommConf->DecConnectedPartiesNum();  //Decrement 'connected parties' counter
					/* Flora added for LED&LCD Feature */
					LedSysAlarmInd();
				}

				if (param != PARTY_SECONDARY)
				{
					CSecondaryParams secParams;

					pConfParty->SetSecondaryCause(SECONDARY_CAUSE_DEFAULT);
					pConfParty->SetSecondaryCauseParams(secParams);
				}
				// avoid video loop-back of cascaded link
				if (param == PARTY_SECONDARY || param == PARTY_CONNECTED || param == PARTY_CONNECTED_WITH_PROBLEM || param == PARTY_DISCONNECTED || param == PARTY_DELETED_BY_OPERATOR)
					MuteVideoOfCascadeLinksIfNoVideoParties();
				if (param == PARTY_DISCONNECTED)
					m_pCommConf->UpdateActiveSpeakersListUponPartyLeftTheConf( pConfParty->GetPartyId() );

				// do the statistics for SNMP added by huiyu
				CConfPartyProcess* pProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
				FPASSERT_AND_RETURN(!pProcess);
				if (pProcess->GetIsSNMPEnabled() == TRUE)
				{
					if (param == PARTY_CONNECTED)
					{
						if (PARTY_CONNECTED_WITH_PROBLEM != oldState)
						{
							SendMessageToSNMP(eTT_SuccessfulNewCalls, 1);
							pConfParty->SetIsPartyConnected(TRUE);
						}
					}
					if (param == PARTY_DISCONNECTED)
					{
						if (pConfParty->GetIsPartyConnected())
						{
							if ((pConfParty->GetDisconnectCause() == PARTY_HANG_UP) ||
							    (pConfParty->GetDisconnectCause() == DISCONNECTED_BY_OPERATOR) ||
							    (pConfParty->GetDisconnectCause() == DISCONNECTED_BY_CHAIR))
							{
								SendMessageToSNMP(eTT_SuccessfulEndCalls, 1);
							}
							else
							{
								SendMessageToSNMP(eTT_FailedEndCalls, 1);
							}
						}
						else
						{
							SendMessageToSNMP(eTT_FailedNewCalls, 1);
						}
						pConfParty->SetIsPartyConnected(FALSE);
					}
				}
				break;
			}

			case PARTYSTATUS:
			{
				CHECK_PARTY_CONNECTION("PARTYSTATUS");

				TRACEINTO << "Type:PARTYSTATUS, PartyName:" << pConfParty->GetName() << ", PARAM: "
					<<  (PARTY_AUDIO_ONLY== param)? "PARTY_AUDIO_ONLY" : "PARTY_RESET_STATUS";

				switch (param)
				{
					case PARTY_AUDIO_ONLY:
					{
						if (!pConfParty->GetVoice())
						{
							WORD numVideoParties = m_pCommConf->GetNumVideoParties();
							TRACEINTO << "Type:PARTYSTATUS, PartyName:" << pConfParty->GetName() << ", Status:PARTY_AUDIO_ONLY, numVideoParties:" << numVideoParties;
							if (numVideoParties == 0)
								PASSERT(1);

							m_pCommConf->SetNumVideoParties(--numVideoParties);
							pConfParty->SetVoice(YES);

							UpdateAudioParticipantsCount(pConfParty,YES);
						}
						break;
					}

					case PARTY_RESET_STATUS:
					{
						if (pConfParty->GetVoice())
						{
							WORD numVideoParties = m_pCommConf->GetNumVideoParties();
							TRACEINTO << "Type:PARTYSTATUS, PartyName:" << pConfParty->GetName() << ", Status:PARTY_RESET_STATUS, numVideoParties:" << numVideoParties;
							m_pCommConf->SetNumVideoParties(++numVideoParties);
							pConfParty->SetVoice(NO);
							UpdateAudioParticipantsCount(pConfParty,NO);
						}
						break;
					}
				}
				break;
			}

			case UPDATELECTUREMODE:
			{
				CLectureModeParams* pLectureMode = new CLectureModeParams;
				pLectureMode->DeSerialize(NATIVE, *pMsg);

				TRACEINTO << "Type:UPDATELECTUREMODE, LectureModeType:" << (WORD)pLectureMode->GetLectureModeType() << ", AudioActivated:" << (WORD)pLectureMode->GetAudioActivated();

				if (pLectureMode->GetLectureModeType())
				{
					// In Presentation Mode (Remove Lecturer name because we do not want him to appear as the chosen lecturer but only as the current lecturer - only icon in party list)
					CConfParty* pSrcConfParty = m_pCommConf->GetCurrentParty(pLectureMode->GetLecturerName());
					if (pSrcConfParty)
					{
						PartyMonitorID lectPartyId = pSrcConfParty->GetPartyId();
						pLectureMode->SetLecturerId(lectPartyId);
					}
					// In Presentation Mode (Remove Lecturer name because we do not want him to appear as the chosen lecturer but only as the current lecturer - only icon in party list)
					if (pLectureMode->GetLectureModeType() == 3 || pLectureMode->GetAudioActivated() == 1)
					{
						pLectureMode->SetLecturerName("");
					}
					// COP - Lecture mode with disconnected lecturer - should keep the lecturer name as it in DB now
					if (pLectureMode->GetAudioActivated() == 2)
					{
						CLectureModeParams* pCurrentLectureModeParams = m_pCommConf->GetLectureMode();
						pLectureMode->SetLecturerName(pCurrentLectureModeParams->GetLecturerName());
					}
				}

				m_pCommConf->SetLectureMode(*((const CLectureModeParams*)pLectureMode));
				SetMuteIncomingForLectureMode(pLectureMode->GetLecturerName());

				POBJDELETE(pLectureMode);
				break;
			}

			case UPDATELECTURERNAME:
			{
				DWORD len;
				*pMsg >> len;
				if (len == 0)
				{
					TRACEINTO << "Type:UPDATELECTURERNAME - Failed, invalid lecturer name";
					break;
				}

				ALLOCBUFFER(new_name, len+1);
				memset(new_name, '\0', len+1);
				pMsg->Get((unsigned char*)new_name, len);

				CLectureModeParams currentLectureModeParams = *(m_pCommConf->GetLectureMode());
				currentLectureModeParams.SetLecturerName(new_name);
				m_pCommConf->SetLectureMode(currentLectureModeParams);
				SetMuteIncomingForLectureMode(new_name);
				DEALLOCBUFFER(new_name);
				break;
			}

			case UPDATEFOCUSURI:
			{
				DWORD len;
				*pMsg >> len;
				if (len == 0)
				{
					TRACEINTO << "Type:UPDATEFOCUSURI - Failed, invalid FocusUri";
					break;
				}

				ALLOCBUFFER(focus_uri, len+1);
				memset(focus_uri, '\0', len+1);
				pMsg->Get((unsigned char*)focus_uri, len);
				m_pCommConf->SetFocusUriCurrently(focus_uri);

				DEALLOCBUFFER(focus_uri);
				break;
			}

			case UPDATEAUTOLAYOUT:
			{
				m_pCommConf->SetIsAutoLayout((BYTE)param);
				break;
			}

			case UPDATE_VIDEO_CLARITY:
			{
				m_pCommConf->SetIsVideoClarityEnabled((BYTE)param);
				break;
			}

			case AUDIOVOLUME:
			{
				CHECK_PARTY_CONNECTION("AUDIOVOLUME");
				pConfParty->SetAudioVolume((BYTE)param);
				break;
			}

			case PARTYHIGHPROFILE:
			{
				CHECK_PARTY_CONNECTION("PARTYHIGHPROFILE");
				pConfParty->SetIsHighProfile((BYTE)param);
				break;
			}

			case LISTENINGAUDIOVOLUME:
			{
				CHECK_PARTY_CONNECTION("LISTENINGAUDIOVOLUME");
				pConfParty->SetListeningAudioVolume((BYTE)param);
				break;
			}

			case PARTY_ENCRYPTION_STATE:
			{
				CHECK_PARTY_CONNECTION("PARTY_ENCRYPTION_STATE");
				pConfParty->SetIsPartyCurrentlyEncrypted((BYTE)param);
				break;
			}

			case SETISLEADER:
			{
				CHECK_PARTY_CONNECTION("SETISLEADER");
				TRACEINTO << "Type:SETISLEADER, PartyName:" << pConfParty->GetName() << ", IsLeader:" << (WORD)param;
				pConfParty->SetIsLeader((WORD)param);
				m_pCommConf->ChairPersonEnteredCDR(pConfParty);
				break;
			}

			case NETCHNL:
			{
				CHECK_PARTY_CONNECTION("NETCHNL");

				char party_name[H243_NAME_LEN];
				char PhoneNum[PHONE_NUMBER_DIGITS_LEN];

				strncpy(party_name, pPartyConnection->GetName(), sizeof(party_name) - 1);
				party_name[sizeof(party_name) - 1] = '\0';
				BYTE discoInitiator = DIALOUT;
				WORD numChan = 1; // pnetRsrcDesk->m_numPorts;
				if (0xc0000000 & param)
				{
					param ^= 0xc0000000;
					pConfParty->SetNet_channels(TRUE, param, numChan, pPartyConnection->GetNetSetUp()->m_callType);

					Phone* Phonen = pConfParty->GetActualPartyPhoneNumber(param - 1);
					if (Phonen != NULL)
					{
						strncpy(PhoneNum, Phonen->phone_number, sizeof(PhoneNum) - 1);
						PhoneNum[sizeof(PhoneNum) - 1] = '\0';
					}
					else
						PhoneNum[0] = '\0';

					m_pCommConf->NetChnnelConnToCDR(party_name, pConfParty->GetPartyId(), param, pConfParty->GetNetChannelNumber(), pConfParty->GetConnectionType(), (CIsdnNetSetup*)pPartyConnection->GetNetSetUp(), PhoneNum);

					if (m_isGateWay && !pConfParty->GetVoice() && pConfParty->GetConnectionType() == DIAL_OUT)
					{
						BYTE numConnectedChannels = pConfParty->GetNumChannelConnected();
						BYTE numChannelsTotal = pConfParty->GetNetChannelNumber();
						CSmallString cstr;
						float connectionRate = 60.0 / numChannelsTotal * numConnectedChannels;
						cstr << pConfParty->GetPhoneNumber() << " [" << (int)connectionRate << "%]";
						AddPartyMsgOnScreenForGWConf(pConfParty->GetPartyId(), cstr.GetString());
					}

					if (m_isDtmfInviteParty && !pConfParty->GetVoice() && pConfParty->GetConnectionType() == DIAL_OUT)
					{
						BYTE numConnectedChannels = pConfParty->GetNumChannelConnected();
						BYTE numChannelsTotal = pConfParty->GetNetChannelNumber();
						CSmallString cstr;
						float connectionRate = 60.0 / numChannelsTotal * numConnectedChannels;
						cstr << pConfParty->GetPhoneNumber() << " [" << (int)connectionRate << "%]";
						AddPartyMsgOnScreenForInvitedConf(pConfParty->GetPartyId(), cstr.GetString());
					}
				}
				else
				{
					pConfParty->SetNet_channels(FALSE, param, numChan, pPartyConnection->GetNetSetUp()->m_callType);
					if (param <= pConfParty->GetNetChannelNumber())
						m_pCommConf->NetChnnelDisconnToCDR(party_name, pConfParty->GetPartyId(), param, discoInitiator, (BYTE)14, (BYTE)14, (BYTE)pConfParty->GetDisconnectCause(), NET_CHANNEL_DISCONNECTED);
				}
				break;
			}

			case H221:
			{
				CHECK_PARTY_CONNECTION("H221");

				if (param == 0xFFFFFFFF) // party was disconnected
				{
					pConfParty->SetSyncLoss(FALSE);
					pConfParty->Set_R_SyncLoss(FALSE);
					pConfParty->Set_M_SyncLoss(FALSE);
					pConfParty->Set_L_Video_SyncLoss(FALSE);
					break;
				}

				if (param == 0xFFFFFFFE) // party starts connection
				{
					pConfParty->SetSyncLoss(TRUE);
					pConfParty->Set_R_SyncLoss(TRUE);
					pConfParty->Set_M_SyncLoss(TRUE);
					pConfParty->Set_L_Video_SyncLoss(TRUE);
					break;
				}

				DWORD localRemote, lostRegain;
				localRemote   = param;
				localRemote >>= 16;
				lostRegain    = param & 0x0000FFFF;

				if (localRemote == LOCAL)
					lostRegain == 1 ? pConfParty->SetSyncLoss(FALSE) : pConfParty->SetSyncLoss(TRUE);
				if (localRemote == REMOTE)
					lostRegain == 1 ? pConfParty->Set_R_SyncLoss(FALSE) : pConfParty->Set_R_SyncLoss(TRUE);
				if (localRemote == REMOTEVIDEO) // remote video vcu...
					lostRegain == 1 ? pConfParty->Set_M_SyncLoss(FALSE) : pConfParty->Set_M_SyncLoss(TRUE);
				if (localRemote == LOCALVIDEO)  // vcp sent 0x30a...
					lostRegain == 1 ? pConfParty->Set_L_Video_SyncLoss(FALSE) : pConfParty->Set_L_Video_SyncLoss(TRUE);
				break;
			}

			case DISCAUSE:
			{
				CHECK_PARTY_CONNECTION("DISCAUSE");
				TRACEINTO << "Type:DISCAUSE, PartyName:" << pConfParty->GetName() << ", DisconnectCause:" << param;
				pConfParty->SetDisconnectCause(param, MpiErrorNumber);
				break;
			}

			case SECONDARYCAUSE:
			{
				CHECK_PARTY_CONNECTION("SECONDARYCAUSE");
				CSecondaryParams secParams;
				secParams.DeSerialize(NATIVE, *pMsg);
				pConfParty->SetSecondaryCause(param);
				pConfParty->SetSecondaryCauseParams(secParams);
				break;
			}

			case UPDATEVISUALNAME:
			{
				CHECK_PARTY_CONNECTION("UPDATEVISUALNAME");

				DWORD len;
				*pMsg >> len;
				if (len == 0)
				{
					TRACEINTO << "Type:UPDATEVISUALNAME, PartyName:" << pConfParty->GetName() << " - Failed, invalid visual name";
					break;
				}

				DWORD party_id = 0;
				ALLOCBUFFER(new_name, len+1);
				memset(new_name, '\0', len + 1);
				pMsg->Get((unsigned char*)new_name, len);

				pConfParty->SetVisualPartyName(new_name);

				DWORD PartyId = pConfParty->GetPartyId();
				char  party_name[H243_NAME_LEN];
				strncpy(party_name, pPartyConnection->GetName(), sizeof(party_name) - 1);
				party_name[sizeof(party_name) - 1] = '\0';
				m_pCommConf->OperatorSetVisualName(new_name, PartyId, party_name, pConfParty->GetCorrelationId());

				DEALLOCBUFFER(new_name);
				break;
			}

			case MUTE_STATE:
			{
				CHECK_PARTY_CONNECTION("MUTE_STATE");
				TRACEINTO << "Type:MUTE_STATE, PartyName:" << pConfParty->GetName() << ", MuteState:" << (hex) << param;

				if (param & 0xF0000000) // remote self block
				{
					if (param == 0xF0000000) pConfParty->SetAudioMuteByParty(FALSE);
					if (param  & 0x00000001) pConfParty->SetAudioMuteByParty(TRUE);
					if (param  & 0x00000010) pConfParty->SetVideoMuteByParty(TRUE);
					if (param  & 0x0000000E) pConfParty->SetVideoMuteByParty(FALSE);
				}
				else if (param & 0x0F000000) // block (mute) by MCU
				{
					if (param == 0x0F000000) pConfParty->SetAudioMuteByMCU(FALSE);
					if (param  & 0x00000001) pConfParty->SetAudioMuteByMCU(TRUE);
					if (param  & 0x00000010) pConfParty->SetVideoMuteByMCU(TRUE);
					if (param  & 0x0000000E) pConfParty->SetVideoMuteByMCU(FALSE);
				}
				else // operator block
				{
					if (param == 0x00000000) pConfParty->SetAudioMuteByOperator(FALSE);
					if (param  & 0x00000001) pConfParty->SetAudioMuteByOperator(TRUE);
					if (param  & 0x00000010) pConfParty->SetVideoMuteByOperator(TRUE);
					if (param  & 0x0000000E) pConfParty->SetVideoMuteByOperator(FALSE);
				}
				break;
			}

			case UPDATE_AGC_EXEC:
			{
				CHECK_PARTY_CONNECTION("UPDATE_AGC_EXEC");
				pConfParty->SetAGC(param ? YES : NO);
				break;
			}

			case AUDCON:
			{
				CHECK_PARTY_CONNECTION("AUDCON");
				pConfParty->SetAudio_Member(param ? TRUE : FALSE);
				break;
			}

			case VIDCON:
			{
				CHECK_PARTY_CONNECTION("VIDCON");
				pConfParty->SetVideo_Member(param ? TRUE : FALSE);
				break;
			}

			case CONTENTCON:
			{
				CHECK_PARTY_CONNECTION("CONTENTCON");
				pConfParty->SetContent_Member(param ? TRUE : FALSE);
				break;
			}

			case PARTYTELEPRESENCEMODE:
			{
				CHECK_PARTY_CONNECTION("PARTYTELEPRESENCEMODE");
				if (::GetDongleTelepresenceValue() == FALSE)
				{
					TRACEINTO << "Type:PARTYTELEPRESENCEMODE, PartyName:" << pConfParty->GetName() << "- Failed, no license for Telepresence";
					break;
				}
				if (m_pCommConf->GetTelePresenceModeConfiguration() != NO)
				{
					pConfParty->SetTelePresenceMode(param);
					if (!m_pCommConf->GetIsTelePresenceMode())
					{
						if ((param != eTelePresencePartyNone) || (pConfParty->GetCascadedLinksNumber() > 1))
						{
							DetermineTelePresenceConfMode(1);
							TRACEINTO << "Type:PARTYTELEPRESENCEMODE, PartyName:" << pConfParty->GetName() << "- RPX/FLEX/MCC party is detected";
						}
					}
					if (m_pVideoBridgeInterface)
					{
						if (pParty && (pParty != (CTaskApp*)0xffff))
						{
							if (m_pVideoBridgeInterface->IsPartyConnected(pParty))
								m_pVideoBridgeInterface->UpdatePartyTelePresenceMode(pParty, (eTelePresencePartyType)param);
						}
					}
				}
				break;
			}

			case SIPPRIVATEEXTENSION:
			{
				CSipHeaderList*   pHeaders = new CSipHeaderList(0, 0);
				pHeaders->DeSerialize(NATIVE, *pMsg);

				// cdr headers (according to Lucent)
				const CSipHeader* pCalledPartyId     = pHeaders->GetNextPrivateOrProprietyHeader(kPrivateHeader, strlen("P-Called-Party-ID"), "P-Called-Party-ID");
				const CSipHeader* pAssertedIdentity  = pHeaders->GetNextPrivateOrProprietyHeader(kPrivateHeader, strlen("P-Asserted-Identity"), "P-Asserted-Identity");
				const CSipHeader* pChargingVector    = pHeaders->GetNextPrivateOrProprietyHeader(kPrivateHeader, strlen("P-Charging-Vector"), "P-Charging-Vector");
				const CSipHeader* pPreferredIdentity = pHeaders->GetNextPrivateOrProprietyHeader(kPrivateHeader, strlen("P-Preferred-Identity"), "P-Preferred-Identity");

				const char* strCalledPartyID     = pCalledPartyId ? pCalledPartyId->GetHeaderStr() : "";
				const char* strAssertedIdentity  = pAssertedIdentity ? pAssertedIdentity->GetHeaderStr() : "";
				const char* strChargingVector    = pChargingVector ? pChargingVector->GetHeaderStr() : "";
				const char* strPreferredIdentity = pPreferredIdentity ? pPreferredIdentity->GetHeaderStr() : "";

				m_pCommConf->SipPrivateExtensionsToCDR(pPartyConnection->GetName(), pConfParty->GetPartyId(), strCalledPartyID, strAssertedIdentity, strChargingVector, strPreferredIdentity);

				POBJDELETE(pHeaders);
				break;
			}

			case CONTENTPROVIDER:
			{
				CHECK_PARTY_CONNECTION("CONTENTPROVIDER");
				pConfParty->SetEPCContentProvider(param ? TRUE : FALSE);
				break;
			}

			case CONTENTSRC:
			{
				if (pParty == (CTaskApp*)0xffff)
					m_pCommConf->SetEPCContentSourceId(0xffffffff);
				else
					m_pCommConf->SetEPCContentSourceId(pConfParty->GetPartyId());

				break;
			}

			case NUMRETRY:
			{
				CHECK_PARTY_CONNECTION("NUMRETRY");
				pConfParty->SetRetriesNumber(param);
				break;
			}

			case RMOT323CAP:
			{
				CH323StrCap rmtCap;
				rmtCap.DeSerialize(NATIVE, pMsg);
				const CH221Str& str_ref = rmtCap;
				CHECK_PARTY_CONNECTION("RMOT323CAP");
				pConfParty->SetIpCapabilities(str_ref);
				break;
			}

			case RMOTCAP:
			{
				CH221strCapDrv str_ref;
				CCapH320 rmtCap;
				rmtCap.DeSerialize(NATIVE, *pMsg);
				rmtCap.UpdateH221string(str_ref);
				CHECK_PARTY_CONNECTION("RMOTCAP");
				pConfParty->SetCapabilities(str_ref);
				break;
			}

			case CURCOMMODE:
			{
				DWORD transmitBaudRate;
				CComMode curComMode;
				curComMode.DeSerialize(NATIVE, *pMsg);
				CH221Str& str_ref = curComMode.m_H221string;
				CHECK_PARTY_CONNECTION("CURCOMMODE");
				pConfParty->SetLocalCommMode(str_ref);
				::GetXferBitrate(transmitBaudRate, curComMode);
				pConfParty->SetTransmitBaudRate(transmitBaudRate);
				break;
			}

			case REMOTECOMMODE:
			{
				DWORD rcvBaudRate;
				CComMode rmtComMode;
				rmtComMode.DeSerialize(NATIVE, *pMsg);
				CH221Str& str_ref = rmtComMode.m_H221string;
				CHECK_PARTY_CONNECTION("REMOTECOMMODE");
				pConfParty->SetRemoteCommMode(str_ref);
				::GetXferBitrate(rcvBaudRate, rmtComMode);
				pConfParty->SetReceiveBaudRate(rcvBaudRate);
				break;
			}

			case RMOT323COMMODE:
			{
				CH323StrCap rmtScm;
				rmtScm.DeSerialize(NATIVE, pMsg);
				const CH221Str& str_ref = rmtScm;
				CHECK_PARTY_CONNECTION("RMOT323COMMODE");
				pConfParty->SetIpRemoteCommMode(str_ref);
				break;
			}

			case LOCAL323COMMODE:
			{
				CH323StrCap localScm;
				localScm.DeSerialize(NATIVE, pMsg);
				const CH221Str& str_ref = localScm;
				CHECK_PARTY_CONNECTION("LOCAL323COMMODE");
				pConfParty->SetIpLocalCommMode(str_ref);
				break;
			}

			case RECEIVEBAUDRATE:
			{
				DWORD rcvBaudRate;
				*pMsg >> rcvBaudRate;
				CHECK_PARTY_CONNECTION("RECEIVEBAUDRATE");
				pConfParty->SetReceiveBaudRate(rcvBaudRate);
				break;
			}

			case TRANSMITBAUDRATE:
			{
				DWORD transmitBaudRate;
				*pMsg >> transmitBaudRate;
				CHECK_PARTY_CONNECTION("TRANSMITBAUDRATE");
				pConfParty->SetTransmitBaudRate(transmitBaudRate);
				break;
			}

			case CPCONFLAYOUT:
			{
				BYTE bytempByteVar;
				VideoActivities vidActiv;
				RequestPriority reqPrio;
				BYTE isActiveLayot = NO;
				BYTE isHided;    // don't need it here, only for compatibility with CPARTYLAYOUT
				char pSeenParty[H243_NAME_LEN];
				WORD numb_sub_img;

				*pMsg >> bytempByteVar;
				Force_Level forceLevel = (Force_Level)bytempByteVar;
				PASSERT_AND_RETURN(forceLevel != CONF_lev);

				*pMsg >> numb_sub_img;
				*pMsg >> bytempByteVar;
				*pMsg >> isActiveLayot;
				LayoutType layoutType = (LayoutType)bytempByteVar;
				PASSERT_AND_RETURN(numb_sub_img != GetNumbSubImg(layoutType));

				CVideoLayout confLayoutOper;
				// build partyLayoutOper from partyLayout
				confLayoutOper.SetScreenLayout(::GetOldLayoutType(layoutType));
				confLayoutOper.SetActive(isActiveLayot);
				for (int i = 0; i < numb_sub_img; i++)
				{
					CVideoCellLayout cellLayout; // operator party layout
					cellLayout.SetCellId(i+1);
					*pMsg >> isHided;
					*pMsg >> bytempByteVar;
					vidActiv = (VideoActivities)bytempByteVar;
					*pMsg >> bytempByteVar;
					reqPrio = (RequestPriority)bytempByteVar;
					*pMsg >> pSeenParty;
					switch (vidActiv)
					{
						case DEFAULT_Activ:
						{
							cellLayout.SetAudioActivated();
							break;
						}

						case BLANK_CONF_Activ:
						{
							switch (reqPrio)
							{
								case OPERATOR_Prior:
								{
									cellLayout.SetBlank(EMPTY_BY_OPERATOR_ALL_CONF);
									break;
								}
								default:
								{
									PASSERT(vidActiv);
								}
							}
							break;
						}

						case FORCE_CONF_Activ:
						case FORCE_MVC_CONF_Activ:
						case FORCE_CONTENT_Activ:
						{
							switch (reqPrio)
							{
								case OPERATOR_Prior:
								{
									CPartyConnection* pResSrcPartyConnection = GetPartyConnection(pSeenParty);
									if (pResSrcPartyConnection)
									{
										CConfParty* pSrcConfParty = m_pCommConf->GetCurrentParty(pResSrcPartyConnection->GetName());
										DWORD partyId = (pSrcConfParty) ? pSrcConfParty->GetPartyId() : 0;
										cellLayout.SetCurrentPartyId(partyId, BY_OPERATOR_ALL_CONF);
										cellLayout.SetForcedPartyId(partyId, BY_OPERATOR_ALL_CONF);
										cellLayout.SetName(pSrcConfParty->GetName());
									}
									else
									{
										// party isn't connected
										CConfParty* pSrcConfParty = m_pCommConf->GetCurrentParty(pSeenParty);
										if (pSrcConfParty)
										{
											DWORD partyId = pSrcConfParty->GetPartyId();
											cellLayout.SetCurrentPartyId(partyId, BY_OPERATOR_ALL_CONF);
											cellLayout.SetForcedPartyId(partyId, BY_OPERATOR_ALL_CONF);
											cellLayout.SetName(pSrcConfParty->GetName());
										}
									}
									break;
								}

								case CHAIRMAN_Prior:
								{
									CPartyConnection* pResSrcPartyConnection = GetPartyConnection(pSeenParty);
									if (pResSrcPartyConnection)
									{
										CConfParty* pSrcConfParty = m_pCommConf->GetCurrentParty(pResSrcPartyConnection->GetName());
										DWORD partyId = (pSrcConfParty) ? pSrcConfParty->GetPartyId() : 0;
										cellLayout.SetCurrentPartyId(partyId, BY_H243_CHAIR_ALL_CONF);
										cellLayout.SetForcedPartyId(partyId, BY_H243_CHAIR_ALL_CONF);
									}
									break;
								}

								case PARTY_Prior:
								{
									CPartyConnection* pResSrcPartyConnection = GetPartyConnection(pSeenParty);
									if (pResSrcPartyConnection)
									{
										CConfParty* pSrcConfParty = m_pCommConf->GetCurrentParty(pResSrcPartyConnection->GetName());
										DWORD partyId = (pSrcConfParty) ? pSrcConfParty->GetPartyId() : 0;
										cellLayout.SetCurrentPartyId(partyId, BY_H243_REQUEST_ALL_CONF);
										cellLayout.SetForcedPartyId(partyId, BY_H243_REQUEST_ALL_CONF);
									}
									break;
								}

								default:
								{
									PASSERT(reqPrio ? reqPrio : 1001);
									break;
								}
							} // switch
							break;
						}

						case AUTO_SCAN_Active:
						{
							cellLayout.SetAutoScan();
							break;
						}

						default:
						{
							DBGPASSERT((DWORD)vidActiv);
						}
					} // switch
					confLayoutOper.AddCell(cellLayout);
				} // end for
				m_pCommConf->AddVideoLayout(confLayoutOper);
				break;
			}

			case MEDIA:
			{
				CMediaList mediaList;
				mediaList.DeSerialize(pMsg);
				pConfParty->SetMediaList(mediaList);
				if (IsValidPObjectPtr(m_pSvcEventPackage))
				{
					CSegment seg;
					seg << (DWORD)(pConfParty->GetPartyId()) << (WORD)type << (DWORD)param;
					m_pSvcEventPackage->HandleObserverUpdate(&seg, SIP_EVENT_PACKAGE);
				}
				break;
			}

			case MEDIA_REMOVE:
			{
				std::list<unsigned int> listMediaID;
				unsigned int isUrgent = 0;
				CStlUtils::DeSerializeListWithFlag(pMsg, listMediaID, isUrgent);
				pConfParty->RemoveMedia(listMediaID, isUrgent == 1);
				if (IsValidPObjectPtr(m_pSvcEventPackage))
				{
					CSegment seg;
					seg << (DWORD)(pConfParty->GetPartyId()) << (WORD)type << (DWORD)param;
					m_pSvcEventPackage->HandleObserverUpdate(&seg, SIP_EVENT_PACKAGE);
				}
				break;
			}

			case CPPARTYLAYOUT:
			{
				BYTE isHided;
				VideoActivities vidActiv;
				RequestPriority reqPrio;
				BYTE bytempByteVar;
				BYTE st = 0;
				CTaskApp* pSeenParty;
				char szForcedName[H243_NAME_LEN];
				*pMsg >> bytempByteVar;
				Force_Level inf_level = (Force_Level)bytempByteVar;
				PASSERT_AND_RETURN(inf_level != PARTY_lev);
				WORD numb_sub_img;
				*pMsg >> numb_sub_img;
				*pMsg >> bytempByteVar;
				LayoutType      newLayType = (LayoutType)bytempByteVar;
				PASSERT_AND_RETURN(numb_sub_img != GetNumbSubImg(newLayType));
				// build partyLayoutOper from partyLayout
				CVideoLayout partyLayoutOper; // operator party layout
				partyLayoutOper.SetScreenLayout(::GetOldLayoutType(newLayType));
				CHECK_PARTY_CONNECTION("CPPARTYLAYOUT");

				for (WORD i = 0; i < numb_sub_img; i++)
				{
					CVideoCellLayout* pCellLayout = new CVideoCellLayout;
					pCellLayout->SetCellId(i+1);
					*pMsg >> isHided;
					*pMsg >> bytempByteVar;
					vidActiv = (VideoActivities)bytempByteVar;
					*pMsg >> bytempByteVar;
					reqPrio = (RequestPriority)bytempByteVar;
					*pMsg >> szForcedName;
					szForcedName[H243_NAME_LEN-1] = '\0';
					*pMsg >> (DWORD&)pSeenParty;
					if (pSeenParty != NIL(CTaskApp) && isHided == YES)
					{
						pCellLayout->SetBlank(EMPTY_BY_OPERATOR_THIS_PARTY);
						partyLayoutOper.AddCell(*pCellLayout);
						continue;
					}

					switch (vidActiv)
					{
						case BLANK_CONF_Activ:
						{
							switch (reqPrio)
							{
								case OPERATOR_Prior:
								{
									pCellLayout->SetBlank(EMPTY_BY_OPERATOR_ALL_CONF);
									break;
								}
								default:
								{
									PASSERT(reqPrio ? reqPrio : 90);
									break;
								}
							}
							break;
						}

						case BLANK_PARTY_Activ:
						case BLANK_PRIVATE_PARTY_Active:
						{
							switch (reqPrio)
							{
								case OPERATOR_Prior:
								{
									pCellLayout->SetBlank(EMPTY_BY_OPERATOR_THIS_PARTY);
									break;
								}
								default:
								{
									PASSERT(reqPrio ? reqPrio : 91);
									break;
								}
							}
							break;
						}

						case DEFAULT_Activ:
						{
							st = AUDIO_ACTIVATED;
							break;
						}

						case FORCE_CONF_Activ:
						case FORCE_MVC_CONF_Activ:
						case FORCE_CONTENT_Activ:
						{
							switch (reqPrio)
							{
							case OPERATOR_Prior : st = BY_OPERATOR_ALL_CONF; break;
							case CHAIRMAN_Prior : st = BY_H243_CHAIR_ALL_CONF; break;
							case PARTY_Prior    : st = BY_H243_REQUEST_ALL_CONF; break;
							default             : PASSERT(reqPrio ? reqPrio : 92); break;
							}
							break;
						}

						case FORCE_PARTY_Activ:
						{
							switch (reqPrio)
							{
								case MCMS_Prior     : st = BY_ITP_LAYOUT_MGMT; break;
								case OPERATOR_Prior : st = BY_OPERATOR_THIS_PARTY; break;
								case CHAIRMAN_Prior : st = BY_H243_CHAIR_THOS_PARTY; break;
								case PARTY_Prior    : st = BY_H243_REQUEST_THIS_PARTY; break;
								default             : PASSERT(reqPrio ? reqPrio : 93); break;
							}
							break;
						}

						case FORCE_PRIVATE_PARTY_Active:
						{ // Private layout forcing
							switch (reqPrio)
							{
								case MCMS_Prior     : st = BY_ITP_LAYOUT_MGMT; break;
								case PARTY_Prior    : st = BY_H243_REQUEST_THIS_PARTY; break;
								case OPERATOR_Prior : st = BY_OPERATOR_THIS_PARTY; break;
								default             : PASSERT(reqPrio ? reqPrio : 94); break;
							}
							break;
						}

						case AUTO_SCAN_Active:
						{
							st = AUTO_SCAN;
							break;
						}

						default:
							break;
					} // end switch (vidActiv)

					if (vidActiv != BLANK_CONF_Activ && vidActiv != BLANK_PARTY_Activ && vidActiv != BLANK_PRIVATE_PARTY_Active)
					{
						// handle here anatoly's party id !!!
						DWORD partyId = 0xffffffff;
						CConfParty* pForceParty = m_pCommConf->GetCurrentParty(szForcedName);
						if (pForceParty)
						{
							partyId = pForceParty->GetPartyId();
							pCellLayout->SetForcedPartyId(partyId, st);
						}

						CPartyConnection* pResSrcPartyConnection = GetPartyConnection(pSeenParty);
						if (pResSrcPartyConnection)
						{
							CConfParty* pSrcConfParty = m_pCommConf->GetCurrentParty(pResSrcPartyConnection->GetName());
							if (pSrcConfParty)
							{
								partyId = pSrcConfParty->GetPartyId();
								pCellLayout->SetCurrentPartyId(partyId, st);
							}
						}
					}

					partyLayoutOper.AddCell(*pCellLayout);
					POBJDELETE(pCellLayout);
				} // for

				pConfParty->SetVideoLayout(partyLayoutOper);
				if (param == 1)
					pConfParty->AddPrivateVideoLayout(partyLayoutOper);
				else
					pConfParty->SetPartyConfVideoLayout();

				if (m_pGatheringManager && m_pGatheringManager->IsGatheringEnabled() && (pConfParty->GetPartyState() == PARTY_CONNECTED || pConfParty->GetPartyState() == PARTY_SECONDARY))
					m_pGatheringManager->OnPartyLayoutChanged(pConfParty->GetName(), &partyLayoutOper, param);

				// VNGR-22639
				RestoreLecturerVideoLayoutIfNeeded(pConfParty);
				break;
			}

			case PRIVATEON:
			{
				CHECK_PARTY_CONNECTION("PRIVATEON");

				if (param == 0)
				{
					pConfParty->SetIsPrivateLayout(NO);
					pConfParty->SetPartyConfVideoLayout();
					// Set the current personal layout to non-active
					CVideoLayout* pVideoLayout = pConfParty->GetCurPrivateVideoLayout();
					if (pVideoLayout)
						pVideoLayout->SetActive(NO);
				}
				else
					pConfParty->SetIsPrivateLayout(YES);

				break;
			}

			case PARTYAVSTATUS:
			{
				CHECK_PARTY_CONNECTION("PARTYAVSTATUS");
				TRACEINTO << "Type:PARTYAVSTATUS, PartyName:" << pConfParty->GetName() << ", IVR State:" << (int)param;
				if ((param != STATUS_PARTY_IVR) && (param != STATUS_PARTY_INCONF))
					break;

				//BRIDGE-12419: Lync Plugin enter here only in EQ, and we don't show the IVR stage
				if (pConfParty->GetIsLyncPlugin())
					pConfParty->SetOrdinaryParty(STATUS_PARTY_INCONF);
				else
					pConfParty->SetOrdinaryParty(param);
				break;
			}

			case MONITORPHONENUM:
			{
				BYTE channel;
				char mcuNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
				char partyNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];

				*pMsg >> channel >> mcuNumber >> partyNumber;
				CHECK_PARTY_CONNECTION("MONITORPHONENUM");
				pConfParty->SetActualMCUandPartyPhoneNumbers(channel, mcuNumber, partyNumber);
				break;
			}

			case SETPARTYCHNLNUM:
			{
				CHECK_PARTY_CONNECTION("SETPARTYCHNLNUM");
				TRACEINTO << "Type:SETPARTYCHNLNUM, PartyName:" << pConfParty->GetName() << ", ChannelsNumber:" << param;
				pConfParty->SetNetChannelNumber((BYTE)param);
				break;
			}

			case IPLOGICALCHANNELUPDATE:
			{
				DWORD vendorType;
				DWORD channelType;

				*pMsg >> vendorType >> channelType;

				if ((pConfParty->GetNetInterfaceType() == H323_INTERFACE_TYPE && channelType == H225) ||
				    (pConfParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE && channelType == SIGNALING))
				{
					CIpNetSetup* pIpNetSetup = (pConfParty->GetNetInterfaceType() == H323_INTERFACE_TYPE) ? (CIpNetSetup*)new CH323NetSetup : (CIpNetSetup*)new CSipNetSetup;
					pIpNetSetup->DeSerialize(NATIVE, *pMsg);
					char party_name[H243_NAME_LEN];
					strcpy_safe(party_name, pPartyConnection->GetName());
					if( pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveOut && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveIn)
					{
						m_pCommConf->IpChnnelConnToCDR(party_name, pConfParty->GetPartyId(),
					                               pConfParty->GetNetInterfaceType(), pIpNetSetup,
					                               pConfParty->GetConnectionType());
					}

					if (vendorType)
					{
						pConfParty->SetH323PartyAlias(((CH323NetSetup*)pIpNetSetup)->GetH323PartyAlias());
						pConfParty->SetH323PartyAliasType(((CH323NetSetup*)pIpNetSetup)->GetH323PartyAliasType());
					}

					POBJDELETE(pIpNetSetup);
				}
				break;
			}

			case IPLOGICALCHANNELCONNECT:
			{
				DWORD vendorType;
				DWORD channelType;
				*pMsg >> vendorType >> channelType;

				CPrtMontrBaseParams* pPrtMonitrParams = CPrtMontrBaseParams::AllocNewClass((EIpChannelType)channelType);
				if (pPrtMonitrParams)
				{
					pPrtMonitrParams->DeSerialize(NATIVE, *pMsg);
					pPrtMonitrParams->Dump1();
					CHECK_PARTY_CONNECTION("SETPARTYCHNLNUM");
					pConfParty->SetIpChannelDetails((CPrtMontrBaseParams*)pPrtMonitrParams, TRUE);
				}
				POBJDELETE(pPrtMonitrParams);
				break;
			}

			case IPLOGICALCHANNELDISCONNECT:
			{
				DWORD channelTypeD;
				*pMsg >>channelTypeD;
				EIpChannelType channelTypeE = (EIpChannelType)channelTypeD;

				CPrtMontrBaseParams* pPrtMonitrParams = CPrtMontrBaseParams::AllocNewClass(channelTypeE);
				if (pPrtMonitrParams)
				{
					CHECK_PARTY_CONNECTION("IPLOGICALCHANNELDISCONNECT");
					pConfParty->SetIpChannelDetails(pPrtMonitrParams, TRUE);
				}
				POBJDELETE(pPrtMonitrParams);
				break;
			}

			case H323GATEKEEPERSTATUS:
			{
				BYTE  gkState;
				DWORD reqBandwidth;
				DWORD allocBandwidth;
				WORD  requestInfoInterval;
				BYTE  gkRouted;

				*pMsg >> gkState >> reqBandwidth >> allocBandwidth >> requestInfoInterval >> gkRouted;
				pConfParty->SetH323GatekeeperStatus(gkState, reqBandwidth, allocBandwidth, requestInfoInterval, gkRouted);
				break;
			}

			case IPPARTYMONITORING:
			{
				WORD channelType            = 0;
				BYTE intraSyncFlag          = 0;
				BYTE videoBCHSyncFlag       = 0;
				BYTE protocolSyncFlag       = 0;
				WORD bchOutOfSyncCount      = 0;
				WORD protocolOutOfSyncCount = 0;
				BYTE bShowMonitoring        = TRUE;

				*pMsg >> channelType >> intraSyncFlag >> videoBCHSyncFlag >> bchOutOfSyncCount >> protocolSyncFlag >> protocolOutOfSyncCount >> bShowMonitoring;
				CHECK_PARTY_CONNECTION("IPPARTYMONITORING");
				if ((EIpChannelType)channelType == VIDEO_OUT)
				{
					pConfParty->SetIpVideoIntraSync(intraSyncFlag);
					pConfParty->SetIpVideoBch(videoBCHSyncFlag);
					pConfParty->SetIpVideoBchCounter(bchOutOfSyncCount);
					pConfParty->SetIpProtocolSync(protocolSyncFlag);
					pConfParty->SetIpProtocolSyncCounter(protocolOutOfSyncCount);
				}
				break;
			}

			case LPR_SYNC:
			{
				CHECK_PARTY_CONNECTION("LPR_SYNC");

				if (param == 0xFFFFFFFF) // party was disconnected
				{
					pConfParty->Set_Lpr_Rmt_SyncLoss(FALSE);
					pConfParty->Set_Lpr_Local_SyncLoss(FALSE);
					break;
				}

				if (param == 0xFFFFFFFE) // party starts connection
				{
					pConfParty->Set_Lpr_Rmt_SyncLoss(TRUE);
					pConfParty->Set_Lpr_Local_SyncLoss(TRUE);
					break;
				}

				DWORD localRemote = param; localRemote >>= 16;
				DWORD lostRegain  = param & 0x0000FFFF;
				if (localRemote == REMOTELPRVID) // remote video vcu...
					pConfParty->Set_Lpr_Rmt_SyncLoss(lostRegain == 1 ? FALSE : TRUE);

				if (localRemote == LOCALLPRVID)  // vcp sent 0x30a...
					pConfParty->Set_Lpr_Local_SyncLoss(lostRegain == 1 ? FALSE : TRUE);

				break;
			}

			case CHANNELSWITHLPRPAYLOAD:
			{
				pConfParty->Set_Is_Lpr_Headers_Activated((BYTE)param);
				break;
			}

			case PARTY_REQUEST_TO_SPEAK:
			{
				BYTE onOff;
				*pMsg >> onOff;
				pConfParty->SetRequestToSpeak(onOff);
				break;
			}

			case UPDATE_EXCLUSIVE_CONTENT: // Restricted content
			{
				if (!m_isContentConnected)
				{
					PASSERTSTREAM(1, "Failed, content is not connected (invalid state)");
					break;
				}
				if (pConfParty == NULL && param != FALSE)
				{
					PASSERTSTREAM(1, "Failed, cannot set exclusive content to NULL");
					break;
				}

				m_pCommConf->updateExclusiveContent(pConfParty, param);
				break;
			}

			case UPDATE_EXCLUSIVE_CONTENT_MODE: // Restricted content
			{
				m_pCommConf->SetExclusiveContentMode((BOOL)param); // param is TRUE for set and FALSE for cancel.
				break;
			}

			case UPDATE_MUTE_INCOMING_LECTURE_MODE:
			{
				m_pCommConf->SetMuteIncomingPartiesLectureMode((BOOL)param); // param is TRUE for set and FALSE for cancel.
				break;
			}

			case PARTY_INTRA_SUPPRESS:
			{
				CHECK_PARTY_CONNECTION("PARTY_INTRA_SUPPRESS");
				pConfParty->SetEventModeIntraSuppressed((param == 0) ? NO : YES);
				break;
			}

			case EVENT_MODE_LEVEL:
			{
				CHECK_PARTY_CONNECTION("EVENT_MODE_LEVEL");
				WORD encoderLevel = param;
				if (encoderLevel <= NUM_OF_LEVEL_ENCODERS)
					pConfParty->SetEventModeLevel(encoderLevel);
				else
					TRACEINTO << "Type:EVENT_MODE_LEVEL, PartyName:" << pConfParty->GetName() << ", EncoderLevel:" << encoderLevel << " - Failed, not valid level";
				break;
			}

			case SIPALLOCATEDBANDWIDTHSTATUS:
			{
				DWORD reqBandwidth;
				DWORD allocBandwidth;

				*pMsg >> reqBandwidth >> allocBandwidth;
				CHECK_PARTY_CONNECTION("SIPALLOCATEDBANDWIDTHSTATUS");
				pConfParty->SetSipBWStatus(reqBandwidth, allocBandwidth);
				break;
			}
			case UPDATE_MUTE_ALL_AUDIO_EXCEPT_LEADER:
			{
				m_pCommConf->SetMuteAllPartiesAudioExceptLeader((BYTE)param);
				break;
			}
			case UPDATE_MUTE_ALL_VIDEO_EXCEPT_LEADER:
			{
				m_pCommConf->SetMuteAllPartiesVideoExceptLeader((BYTE)param);
				break;
			}
			case UPDATE_ACTIVE_SPEAKERS_LIST:
			{
				UpdateActiveSpeakersList( pMsg );
				break;
			}
		} // switch

		if (pConfParty)
			UpdatePartyState(pConfParty);
	}
	UpdateConfStatus();
}


////////////////////////////////////////////////////////////////////////////
void CConf::UpdateActiveSpeakersList( CSegment *pSeg )
{
	DWORD unActiveSpeakerRsrcId;
	DWORD unActiveSpeakerMonitorIdList[MAX_ACTIVE_SPEAKER_LIST];
	for (int i = 0; i < MAX_ACTIVE_SPEAKER_LIST; i++)
	{
		unActiveSpeakerRsrcId = 0xFFFFFFFF;
		*pSeg >> unActiveSpeakerRsrcId;
		if (0xFFFFFFFF != unActiveSpeakerRsrcId)
			unActiveSpeakerMonitorIdList[i] = GetPartyMonitorIdFromPartyRsrcId( unActiveSpeakerRsrcId );
		else
			unActiveSpeakerMonitorIdList[i] = 0xFFFFFFFF;
	}
	m_pCommConf->UpdateActiveSpeakersList( unActiveSpeakerMonitorIdList );
}

////////////////////////////////////////////////////////////////////////////
PartyMonitorID CConf::GetPartyMonitorIdFromPartyRsrcId(PartyRsrcID partyId)
{
	CPartyCntl* pPartyCntl = GetPartyCntl(partyId);
	if (pPartyCntl)
		return pPartyCntl->GetMonitorPartyId();
	TRACEINTO << "PartyId:" << partyId << " - Failed, party does not found";
	return 0xFFFFFFFF;
}

////////////////////////////////////////////////////////////////////////////
void CConf::SetCapCommonDenominator(CTaskApp* pParty, BYTE bTakeCurrent)
{
	enum VidModeType {VSW_FIXED, VSW_AUTO, TRANSCODE, SOFTWARE_CP, COP};
	VidModeType ConfVidMode = VSW_FIXED;

	WORD video_session = GetVideoSession();

	if ((video_session == CONTINUOUS_PRESENCE) || (video_session == VIDEO_TRANSCODING))
		ConfVidMode = TRANSCODE;
	else if (video_session == VIDEO_SESSION_COP)
		ConfVidMode = COP;

	// when we connect voice participants (into video conference)
	// we don't want the common denominator to change
	// we don't want to send change SCM to all the other participants
	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	TRACECOND_AND_RETURN(!pPartyConnection, "Failed to find party connection");

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
	PASSERT_AND_RETURN(!pConfParty);

	WORD partyInterfaceType = pPartyConnection->GetInterfaceType();
	BYTE isVoiceParty = pConfParty->GetVoice();

	TRACEINTO
		<< "ConfName:" << m_name
		<< ", PartyId:"       << pPartyConnection->GetPartyRsrcId()
		<< ", IsTakeCurrent:" << (int)bTakeCurrent
		<< ", InterfaceType:" << partyInterfaceType
		<< ", isVoiceParty:"  << (int)isVoiceParty;

	CComModeH323* pH323NewMode         = NULL;
	CComMode*     pIsdnNewModeTransmit = NULL;
	CComMode*     pIsdnNewModeReceive  = NULL;
	CPartyCntl* pPartyCntl  = pPartyConnection->GetPartyCntl();

	if (partyInterfaceType == ISDN_INTERFACE_TYPE)
	{
		// ISDN party...
		// if the party is voice only we only send change SCM to the current party in order to start the change mode.
		if (isVoiceParty)  // UDI - Add for PSTN party
		{
			pPartyConnection->ChangePSTNScm();
			return;
		}

		pIsdnNewModeTransmit = pPartyConnection->GetIsdnTargetTransmitScm();
		PASSERT_AND_RETURN(!pIsdnNewModeTransmit);

		pIsdnNewModeReceive =  pPartyConnection->GetIsdnTargetReceiveScm();
		PASSERT_AND_RETURN(!pIsdnNewModeReceive);

		if (ConfVidMode == VSW_FIXED)
		{
			TRACEINTO << "Setting VSW_FIXED SCM...";
			CComMode* pIsdnConfMode = m_pUnifiedComMode->GetIsdnComMode();
			pIsdnNewModeTransmit = new CComMode(*pIsdnConfMode);
			*pIsdnNewModeReceive = *pIsdnNewModeTransmit;
		}
	}
	else
	{
		// IP party...
		CComModeH323* pH323CurrentScm = pPartyConnection->GetCurrentIpScm();
		CComModeH323* pH323InitialScm = pPartyConnection->GetInitialIpScm();

		//BRIDGE-9132, if the content rate below than threshold, then close content channel
		/*if (!IsEnableH239() ||
			(IsEnableH239() && IsValidPObjectPtr(pPartyCntl) && (eContentSecondaryCauseBelowRate == pPartyCntl->IsLegacyContentParty())))
		{
			pH323CurrentScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			pH323InitialScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

			pH323CurrentScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
			pH323InitialScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		}*/

		// Set the SCM for the change mode based on the partyConnection(that updated only when change mode completed)
		if (ConfVidMode == VSW_FIXED)
		{
			TRACEINTO << "Setting VSW_FIXED SCM...";
			CComModeH323* pH323ConfMode = m_pUnifiedComMode->GetIPComMode();
			pH323NewMode = new CComModeH323(*pH323ConfMode);
		}
		else // CP or COP
		{
			pH323NewMode = (bTakeCurrent) ? new CComModeH323(*pH323CurrentScm) : new CComModeH323(*pH323InitialScm);
		}
		pH323NewMode->Dump("CConf::SetCapCommonDenominator - H323NewMode is: ", eLevelInfoNormal);
	}

	BYTE regularChangeMode  = FALSE;
	BYTE curContentProtocol = GetCurrentContentProtocolInConfValues();
	if (IsTIPContentEnable())
		PTRACE(eLevelInfoNormal, "CConf::SetCapCommonDenominator - TIP content enable");

	BYTE newContentProtocol = curContentProtocol;
	BYTE isHD1080ContentSupportedBefore = FALSE;
	BYTE isHD1080ContentSupportedCurrently = FALSE;
	BYTE IsRateChanged = FALSE;
	BYTE IsProtocolChanged = FALSE;
	BYTE IsHDResChanged = FALSE;
	BYTE curHD1080ContentMpi = 0;
	BYTE curHD720ContentMpi = 0;
	BYTE isHD1080WillBeSupported = 0;

	if (IsEnableH239() && IsValidPObjectPtr(pPartyCntl) && !pPartyCntl->IsRemoteAndLocalCapSetHasContent(eToPrintYes) && ConfVidMode != VSW_FIXED)
	{
		curHD1080ContentMpi = m_pUnifiedComMode->isHDContent1080Supported(cmCapTransmit);
	}

	if (IsEnableH239() && IsValidPObjectPtr(pPartyCntl) && pPartyCntl->IsRemoteAndLocalCapSetHasContent() && ConfVidMode != VSW_FIXED)
	{
		TRACEINTO << "Check content mode for CP";
		BYTE  curContentRateAMC = GetCurrentContentBitRateAMC();
		BYTE  newContentRateAMC = AMSC_0k;
		DWORD curContentRateIP  = GetActualIpRateForContentSession();

		if (pPartyConnection->GetInterfaceType() != ISDN_INTERFACE_TYPE && pH323NewMode && pH323NewMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation) && !pPartyCntl->IsLegacyContentParty())
		{
			// add here content mode from conf
			PTRACE(eLevelInfoNormal, "Yoella check the casting !!! Guy should set an abstract function");
			DWORD partyRate = ((CIpPartyCntl*)pPartyCntl)->GetMinContentPartyRate();   // Get the party call rate
			BYTE  partyRateAMC = CUnifiedComMode::TranslateRateToAMCRate(partyRate/100);

			//HP content:
			TRACEINTO << "PartyContentRateAMC:" << (int)partyRateAMC <<", curContentRateAMC: " << (int)curContentRateAMC;

			CComModeInfo cmInfo = (CapEnum)pH323NewMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
			if (partyRateAMC != AMSC_0k)
			{
				newContentRateAMC  = SelectContentRate();
				if (curContentRateAMC != AMSC_0k)
				{
					if (newContentRateAMC != curContentRateAMC)
					{
						// update the UnifiedComMode with the new rate
						SetNewContentBitRate(newContentRateAMC);
						TRACEINTO << "NewContentRateAMC:" << (int)newContentRateAMC << " - Changing Content Rate";
					}
				}
				newContentProtocol = SelectContentProtocol(bTakeCurrent);
			}

			// Only for down-grade we stop content
			// if the case is new protocol = H264 and curProtocol = H263
			// we won't stop content for upgrade.
			if (newContentProtocol == H263 && curContentProtocol == H264)
				IsProtocolChanged = TRUE;

			//HP content:
			if (!IsTIPContentEnable() && !IsProtocolChanged && newContentProtocol == H264 && GetCurrentContentIsHighProfile())
			{
				BYTE isPartyHPContent= ((CIpPartyCntl*)pPartyCntl)->IsRemoteAndLocalHasHighProfileContent();
				if (!isPartyHPContent)
				{
					TRACEINTO << "Changing Content profile from HighP to BaseP";
					IsProtocolChanged = TRUE;
					SetCurrentContentIsHighProfile(FALSE);
				}
			}

			if (!IsTIPContentEnable() && (newContentProtocol == H264) && (curContentProtocol == H264)&& !m_pCommConf->GetContentMultiResolutionEnabled() && !IsProtocolChanged) //HP content
			{
				// Romem - check if HD res changed
				newContentRateAMC  = SelectContentRate();
				BYTE newHDContentMpi = 0;
				isHD1080WillBeSupported = SelectContentHDResolution(newContentRateAMC, newContentProtocol, newHDContentMpi);

				curHD1080ContentMpi = m_pUnifiedComMode->isHDContent1080Supported(cmCapTransmit);
				curHD720ContentMpi  = m_pUnifiedComMode->isHDContent720Supported(cmCapTransmit);

				if ((isHD1080WillBeSupported && !curHD1080ContentMpi) || (!isHD1080WillBeSupported && curHD1080ContentMpi))
				{
					IsHDResChanged = TRUE;
					if (isHD1080WillBeSupported)
						TRACEINTO << "Changing Content HD Resolution from HD720 to HD1080";
					else
						TRACEINTO << "Changing Content HD Resolution from HD1080 to HD720";
				}
				//HP content, solve previous bug
				if (!IsHDResChanged && isHD1080WillBeSupported && curHD1080ContentMpi) // Romem - HD 720 - check MPI change
				{
					if(newHDContentMpi != curHD1080ContentMpi)
					{
						TRACEINTO << "NewHD1080ContentMPI:" << (int)newHDContentMpi  << ", OldHD1080ContentMPI:" << (int)curHD1080ContentMpi << " - Changing Content HD 1080 MPI";
						IsHDResChanged = TRUE;
					}
				}
				if (!IsHDResChanged && !isHD1080WillBeSupported) // Romem - HD 720 - check MPI change
				{
					if(newHDContentMpi != curHD720ContentMpi)
					{
						TRACEINTO << "NewHD720ContentMPI:" << (int)newHDContentMpi  << ", OldHD720ContentMPI:" << (int)curHD720ContentMpi << " - Changing Content HD 720 MPI";
						IsHDResChanged = TRUE;
					}
				}
			}

			if (IsProtocolChanged)
			{
				UpdateUnifiedAndContentBrdg(newContentProtocol);
			}
			else if (curContentRateAMC != AMSC_0k && newContentRateAMC != AMC_0k) // check if content is open and rate changed
			{
				if (AMSC_0k != newContentRateAMC/*VNGFE-8198*/ && newContentRateAMC != curContentRateAMC)
				{
					// send new rate to Content Bridge
					if (CPObject::IsValidPObjectPtr(m_pContentBridge))
					{
						m_pContentBridge->ContentRate(newContentRateAMC);
						TRACEINTO << "Update Content Bridge on changing content rate";
					}

					IsRateChanged = TRUE;
					if (IsHDResChanged)
					{
						SetNewContentProtocol(newContentProtocol, cmCapReceiveAndTransmit);
						IsProtocolChanged = TRUE;
					}

					DWORD H323ContentRate = m_pUnifiedComMode->TranslateAMCRateIPRate(newContentRateAMC);
					SetActualIpRateForContentSession(H323ContentRate);
					ChangeContentMode(pPartyConnection, IsRateChanged, IsProtocolChanged);
				}
				else
				{
					if (IsHDResChanged && (newContentRateAMC == curContentRateAMC))  //HP content: solve previous bug, currently RMX does not support HDResChange due to content rate upgrade.
					{
						TRACEINTO << "Changing Content HD Resolution during content session";
						SetNewContentProtocol(newContentProtocol, cmCapReceiveAndTransmit);
						IsProtocolChanged = TRUE;
						ChangeContentMode(pPartyConnection, IsRateChanged, IsProtocolChanged);
					}
				}
			}
			else if (IsHDResChanged)
			{
				UpdateUnifiedAndContentBrdg(newContentProtocol, NULL, FALSE);
			}

			//HP content:
			TRACEINTO << "IsRateChanged:" << (int)IsRateChanged  << ", IsProtocolChanged:" << (int)IsProtocolChanged << ", IsHDResChanged: " << (int)IsHDResChanged;
			// If the rate and protocol remain the same .
			// we need to finish the process with change scm (the move from add to change mode etc.
			if (!IsRateChanged && !IsProtocolChanged)
			{
				CorrectContentModeForParty(pPartyConnection, pH323NewMode, !IsRateChanged, !IsProtocolChanged); // change mode for this party only
				if (pPartyConnection->GetInterfaceType() == H323_INTERFACE_TYPE)
				{
					pPartyConnection->ChangeH323Scm(pH323NewMode, eCanChangeVideoAndContent);
				}
				// For NOW we wont get here since SIP has NO H239
				else if (pPartyConnection->GetInterfaceType() == SIP_INTERFACE_TYPE)
				{
					pPartyConnection->ChangeSipScm(pH323NewMode, m_IsAsSipContentEnable);  // to change name to pInitialMode
				}
			}
		}  // End IP parties

		else if (pPartyConnection->GetInterfaceType() == ISDN_INTERFACE_TYPE) // ISDN parties
		{
			DWORD tempIpContentBitRate = GetActualIpRateForContentSession();
			newContentRateAMC  = SelectContentRate();
			newContentProtocol = SelectContentProtocol(bTakeCurrent);

			// Only for down-grade we stop content
			// if the case is new protocol = H264 and curProtocol = H263
			// we won't stop content for upgrade.
			if (newContentProtocol == H263 && curContentProtocol == H264)
				IsProtocolChanged = TRUE;

			if (IsProtocolChanged)
			{
				UpdateUnifiedAndContentBrdg(newContentProtocol);
			}
			else if (GetCurrentContentBitRateAMC() != AMSC_0k) // If content is open...
			{
				if (newContentRateAMC != curContentRateAMC)
				{
					// update the UnifiedComMode with the new rate
					SetNewContentBitRate(newContentRateAMC);
					// send new rate to Content Bridge
					if (CPObject::IsValidPObjectPtr(m_pContentBridge))
						m_pContentBridge->ContentRate(newContentRateAMC);
					IsRateChanged = TRUE;

					DWORD H323ContentRate = m_pUnifiedComMode->TranslateAMCRateIPRate(newContentRateAMC);
					SetActualIpRateForContentSession(H323ContentRate);
				}
			}

			// we send change mode to all parties because content rate might change
			// even if newContentRateAMC = CurrentContentBitRateAMC, actual bit rate for IP
			// can be different, so we calculate the new rate and send change mode
			// (EITAN 01/08)

			// change content mode for all parties
			if (GetActualIpRateForContentSession() != tempIpContentBitRate)
				IsRateChanged = TRUE;

			// If only rate changed - need to change mode to all parties.
			// Only if we didn't stop the content
			if (IsRateChanged && !IsProtocolChanged)
				ChangeContentMode(pPartyConnection, IsRateChanged, IsProtocolChanged);

			// Don't change content protocol for ISDN party
			if (!IsRateChanged && !IsProtocolChanged) // content rate can`t increase so we are here only if content rate stay the same
			{
				PASSERT_AND_RETURN(!pIsdnNewModeTransmit);
				CorrectContentModeForIsdnParty(pPartyConnection, pIsdnNewModeTransmit); // change mode for this party only
				pIsdnNewModeReceive->m_contentMode = pIsdnNewModeTransmit->m_contentMode;
				pPartyConnection->ChangeIsdnScm(pIsdnNewModeTransmit, pIsdnNewModeReceive);
			}
		}
		else
		{
			regularChangeMode = TRUE;
		}

		if(m_pCommConf->GetContentMultiResolutionEnabled())
		{
			if(m_mapXCodeRsrc.size() > 0)
			{
				eXcodeRsrcType ePartyXCodeEncoderType = ConnectPartyToXCodeEncoder(pPartyConnection,pConfParty);
				if(ePartyXCodeEncoderType != eXcodeEncoderDummy)
				{
					CBridgePartyVideoOutParams* pBridgePartyVideoOutParams = new CBridgePartyVideoOutParams;
					InitPartyXCodeParamsAccordingToEncoderType(ePartyXCodeEncoderType, pBridgePartyVideoOutParams,TRUE);
					m_pContentXcodeBridgeInterface->UpdateVideoOutParams(ePartyXCodeEncoderType,pBridgePartyVideoOutParams);
					POBJDELETE(pBridgePartyVideoOutParams);
				}
			}
		}
	}   // content
	else   // VSW - Add change content protocol to VSW
	{
		if (IsEnableH239() && IsValidPObjectPtr(pPartyCntl) && pPartyCntl->IsRemoteAndLocalCapSetHasContent() && ConfVidMode == VSW_FIXED)
		{
			TRACEINTO << "Check content mode for VSW";

			newContentProtocol = SelectContentProtocol(bTakeCurrent);

			if (newContentProtocol == H263 && curContentProtocol == H264)
			{
				IsProtocolChanged = TRUE;
				UpdateUnifiedAndContentBrdg(newContentProtocol, pPartyConnection);
			}

			if (!IsProtocolChanged)
				regularChangeMode = TRUE;
		}
		else
		{
			regularChangeMode = TRUE;
		}
	}

	if (regularChangeMode == TRUE)
	{
		switch (pPartyConnection->GetInterfaceType())
		{
			case H323_INTERFACE_TYPE:
			{
				pPartyConnection->ChangeH323Scm(pH323NewMode, eCanChangeVideoAndContent);
				break;
			}
			case SIP_INTERFACE_TYPE :
			{
				pPartyConnection->ChangeSipScm(pH323NewMode, m_IsAsSipContentEnable);
				break; // to change name to pInitialMode
			}
			case ISDN_INTERFACE_TYPE:
			{
				pPartyConnection->ChangeIsdnScm(pIsdnNewModeTransmit, pIsdnNewModeReceive);
				break;
			}
		}
	}
	POBJDELETE(pH323NewMode);
}


////////////////////////////////////////////////////////////////////////////
void CConf::UpdateConfStatus(DWORD status, BYTE bYesNo)
{
	// counters
	WORD  numConnected            = 0;
	WORD  numOfProblemParties     = 0;
	WORD  numPartiesWaitForAssist = 0;
	BYTE  presenceState           = 0xFF;

	// update the status
	DWORD confStat    = m_pCommConf->GetStatus();
	DWORD oldConfStat = m_pCommConf->GetStatus();

	// Action on Conference statuses
	if (status != 0xFFFFFFFF)
	{
		if (bYesNo == NO)
		{
			if (confStat & status)
				confStat ^= status;   // OFF status
		}
		else
			confStat |= status;     // ON status
	}
	else
	{
		// keep the statuses CONFERENCE_RESOURCES_DEFICIENCY + CONFERENCE_BAD_RESOURCES + CONFERENCE_CONTENT_RESOURCES_DEFICIENCY if they exist
		DWORD statToBeSave = (CONFERENCE_RESOURCES_DEFICIENCY | CONFERENCE_BAD_RESOURCES | CONFERENCE_CONTENT_RESOURCES_DEFICIENCY);
		// confStat &= 0x98; //statToBeSave//0x18; without content (legacy)deficiency
		confStat &= statToBeSave;

		// start count loop
		CConfParty* pCurConfParty = m_pCommConf->GetFirstParty();
		while (pCurConfParty != NULL)
		{
			// count numConnected, numOfProblemParties
			switch (pCurConfParty->GetPartyState())
			{
				case PARTY_IDLE:
				case PARTY_STAND_BY:
				case PARTY_WAITING_FOR_DIAL_IN:
				case PARTY_CONNECTING:
				case PARTY_DISCONNECTING:
				case PARTY_CONNECTED_PARTIALY:
				case PARTY_DELETED_BY_OPERATOR:
				{
					break;
				}

				case PARTY_DISCONNECTED:
				{
					// not normal disconnection
					if (pCurConfParty->GetDisconnectCause() > DISCONNECTED_BY_OPERATOR)
						numOfProblemParties++;
					break;
				}

				case PARTY_CONNECTED:
				{
					numConnected++;
					break;
				}

				case PARTY_SECONDARY:
				case PARTY_CONNECTED_WITH_PROBLEM:
				{
					numConnected++;
					numOfProblemParties++;
					break;
				}

				default:
				{
					break;
				}
			} // end switch

			// count  numPartiesWaitForAssist
			BYTE is_party_wait_for_operator_assist = pCurConfParty->GetWaitForOperAssistance();
			if (is_party_wait_for_operator_assist)
			{
				numPartiesWaitForAssist++;
			}

			pCurConfParty = m_pCommConf->GetNextParty();
		} // end while

		// update PROBLEM_PARTY
		if (numOfProblemParties == 0) // No problematic End-points
		{
			if (confStat & PROBLEM_PARTY)
				confStat ^= PROBLEM_PARTY;
		}
		else
		{
			confStat |= PROBLEM_PARTY;
		}

		// update CONFERENCE_EMPTY / CONFERENCE_SINGLE_PARTY / CONFERENCE_NOT_FULL
		WORD numParties = m_pCommConf->GetNumParties();
		// if the number of connected parties is zero,the conf state is : CONFERENCE_EMPTY
		if ((0 == numConnected))
		{
			confStat     |= CONFERENCE_EMPTY;
			presenceState = SIP_PRESENCE_ONLINE;
		}
		else
			presenceState = SIP_PRESENCE_BUSY;

		// if the number of connected parties is 1,the conf state is : CONFERENCE_SINGLE_PARTY
		if ((1 == numConnected))
		{
			confStat |= CONFERENCE_SINGLE_PARTY;
		}

		// if the number of parties in conf is big than the number of the connected parties and the number of connected parties is at least 1,
		// the conf state is : CONFERENCE_NOT_FULL
		if (1 <= numConnected && numConnected < numParties)
		{
			confStat |= CONFERENCE_NOT_FULL;
		}

		// update PARTY_REQUIRES_OPERATOR_ASSIST
		if (numPartiesWaitForAssist == 0) // No numPartiesWaitForAssist
		{
			if (confStat & PARTY_REQUIRES_OPERATOR_ASSIST)
				confStat ^= PARTY_REQUIRES_OPERATOR_ASSIST;
		}
		else
		{
			confStat |= PARTY_REQUIRES_OPERATOR_ASSIST;
		}
	} // update party status

	if (presenceState != 0xFF && oldConfStat != confStat)
	{
		CSipProxyManagerApi SipProxyApi;
		CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();

		for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
		{
			CConfIpParameters* pServiceParams = pIpServiceListManager->FindServiceByName(m_pCommConf->GetServiceRegistrationContentServiceName(i));
			if (pServiceParams != NULL)
			{
				if (m_pCommConf->GetServiceRegistrationContentRegister(i) == TRUE && !m_pCommConf->GetEntryQ() && !m_pCommConf->IsSIPFactory() && !m_pCommConf->GetIsGateway() && !m_pCommConf->GetIsAdHocConf())
				{
					if (m_pCommConf->IsMeetingRoom())
					{
						CCommRes* pMRoom = ::GetpMeetingRoomDB()->GetCurrentRsrv(m_name); AUTO_DELETE(pMRoom);
						// Romem - klocwork
						if (pMRoom)
						{
							DWORD mrId = pMRoom->GetMonitorConfId();
							SipProxyApi.ChangePresenceStatus(pServiceParams->GetServiceId(), m_name, mrId, presenceState);
						}
					}
					else
					{
						SipProxyApi.ChangePresenceStatus(pServiceParams->GetServiceId(), m_name, m_pCommConf->GetMonitorConfId(), presenceState);
					}
				}
			}
		}
	}

	// save the conf status
	m_pCommConf->SetStatus(confStat);
}

// ///////////////////////////////////////////////////////////////////////////
void CConf::UpdatePartyState(CConfParty* pConfParty)
{
	if (pConfParty)
	{
		WORD isProblem = pConfParty->IsProblem();
		if (isProblem && pConfParty->GetPartyState() == PARTY_CONNECTED)
		{
			pConfParty->SetPartyState(PARTY_CONNECTED_WITH_PROBLEM, m_pCommConf->GetMonitorConfId());
			TRACEINTO << "PartyName:" << pConfParty->GetName() << ", IsProblem:" << isProblem << " - Problematic Party becomes FAULTY";
		}

		WORD AudioOnly = pConfParty->GetVoice();

		if (!isProblem && pConfParty->GetPartyState() == PARTY_CONNECTED_WITH_PROBLEM && !AudioOnly)
		{
			pConfParty->SetPartyState(PARTY_CONNECTED, m_pCommConf->GetMonitorConfId());
			TRACEINTO << "PartyName:" << pConfParty->GetName() << " - Problematic Party becomes CONNECTED";
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CConf::StartDialOutLoopForGw(PartyMonitorID inviterPartyId)
{
	// set the inviter state as setup
	// PTRACE(eLevelInfoNormal,"VNGFE-4787 CConf::StartDialOutLoopForGw");

	(*m_pGWPartiesState)[inviterPartyId] = GW_CONNECT;

	if (m_pDialingSequence)
	{
		SEQUENCE_DIAL_MAP_PER_CONF* partiesMap = m_pDialingSequence->GetDialMap();

		if (partiesMap && !partiesMap->empty())
		{
			DWORD cfgWrongNumberDialRetries;
			CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("WRONG_NUMBER_DIAL_RETRIES", cfgWrongNumberDialRetries);
			m_PartiesRedialNum[inviterPartyId]  = cfgWrongNumberDialRetries;
			m_PartiesInviteType[inviterPartyId] = eInviteTypeGw;

			TRACEINTO << "PartyMonitorId:" << inviterPartyId << " - Set to WRONG_NUMBER_DIAL_RETRIES";

			SEQUENCE_DIAL_MAP_PER_CONF::iterator it;
			for (it = partiesMap->begin(); it != partiesMap->end(); ++it)
			{
				DWORD partyId = it->first;
				m_pGWPartiesState->insert(GW_PARTIES_STATUS::value_type(partyId, GW_SETUP));

				eGeneralDisconnectionCause defaultCause = eGeneralDisconnectionCauseLast;
				CMedString defaultCauseStr;

				m_PartiesInvitorId[partyId] = inviterPartyId;

				AddNextPartyForGWSession(partyId, (BYTE)defaultCause, defaultCauseStr);
			}
			return;
		}
		else
		{
			TRACEINTO << "PartyMonitorId:" << inviterPartyId << " - Empty dial map";
		}
	}

	FailToStartGWSession();
}

/////////////////////////////////////////////////////////////////////////////
void CConf::FailToStartGWSession()
{
	m_pTextOnScreenMngrForGwSession->AddParty(0);
	AddPartyMsgOnScreenForGWConf(0, "ILLEGAL", 5*SECOND, FALSE);
	if (m_pConfAppMngrInterface)
	{
		CSegment* pMsg = new CSegment;
		*pMsg << (DWORD)EVENT_NOTIFY
		      << (OPCODE)eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE
		      << INVALID; // we always send conf msgs to cam with invalid instead of partyRsrcId

		m_pConfAppMngrInterface->HandleEvent(pMsg);
		POBJDELETE(pMsg);
	}

	StartTimer(ENDGWSETUP, 5*SECOND);
	PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CConf::FailToStartDtmfInvitedSession()
{
	m_pTextOnScreenMngrForInvitedSession->AddParty(0);
	AddPartyMsgOnScreenForInvitedConf(0, "ILLEGAL", 5*SECOND, FALSE);
	if (m_pConfAppMngrInterface)
	{
		CSegment* pMsg = new CSegment;
		*pMsg << (DWORD)EVENT_NOTIFY
		      << (OPCODE)eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE
		      << INVALID; // we always send conf msgs to cam with invalid instead of partyRsrcId

		m_pConfAppMngrInterface->HandleEvent(pMsg);
		POBJDELETE(pMsg);
	}

	StartTimer(ENDDTMFINVITEPARTYSETUP, 5*SECOND);
	PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
WORD CConf::GetNodeType(CConfParty* pConfParty)
{
	WORD nodeType = pConfParty->GetNodeType();
	if (nodeType == 0)    // mcu
	{
		switch (m_master)
		{
			case 0:
			{
				nodeType = CASCADE_MODE_MASTER;
				break;
			}

			case 1:
			{
				nodeType = CASCADE_MODE_SLAVE;
				break;
			}

			case 255:
			{
				nodeType = CASCADE_MODE_NEGOTIATED;
				break;
			}

			default:
			{
				break;
			}
		} // switch
	}
	else
		nodeType = CASCADE_MODE_NONE;

	return nodeType;
}

/////////////////////////////////////////////////////////////////////////////
WORD CConf::GetVideoSession() const
{
	PASSERT_AND_RETURN_VALUE(!m_pCommConf, 0);

	BYTE vid_sess = m_pCommConf->GetVideoSession();
	if (IsHDVSW())
		vid_sess = VIDEO_SWITCH_FIXED;

	switch (vid_sess)
	{
		case VIDEO_SWITCH                : return VIDEO_SWITCH;
		case VIDEO_SWITCH_FIXED          : return VIDEO_SWITCH_FIXED;
		case CONTINUOUS_PRESENCE         : return CONTINUOUS_PRESENCE;
		case SOFTWARE_CONTINUOUS_PRESENCE: return SOFTWARE_CONTINUOUS_PRESENCE;
		case ADVANCED_LAYOUTS            : return ADVANCED_LAYOUTS;
		case VIDEO_SESSION_COP           : return VIDEO_SESSION_COP;
	}

	PASSERTSTREAM_AND_RETURN_VALUE(1, "VideoSession:" << (int)vid_sess, 0);
}

/////////////////////////////////////////////////////////////////////////////
void CConf::SetH320PartyCapsAndVideoParam(CComMode* pPartyScm, CComMode* pPartyTransmitScm, CCapH320* pH320Cap, CConfParty* pConfParty, DWORD vidBitrate, WORD dialType, DWORD setupRate, DWORD confRate)
{
	const char* pPartyName = pConfParty->GetName();
	APIS8 h263_4CifMpi = -1;
	BOOL bIsForceG711A = TRUE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_FORCE_G711A, bIsForceG711A);

	if ((TRUE == bIsForceG711A) || (strstr(pPartyName, "##FORCE_MEDIA") != NULL))
	{
		ostringstream str;
		str << "party_name = " << pPartyName << " , force media by party name:\n";

		pH320Cap->CreateDefault();
		pH320Cap->RemoveAudioCap(e_G722_48); //because CCapH320::CreateDefault() always adds this algorithm
		CAudMode partyAudioMode;
		CVidMode partyVidMode;

		// Audio algorithms
		if ((TRUE == bIsForceG711A) || (strstr(pPartyName, "G711_A") != NULL))
		{
			partyAudioMode.SetBitRate(A_Law_OF);
			pH320Cap->SetAudioCap(e_A_Law);
			str << "set G711_A\n";
		}
		if (strstr(pPartyName, "G711_U") != NULL)
		{
			partyAudioMode.SetBitRate(U_Law_OF);
			pH320Cap->SetAudioCap(e_U_Law);
			str << "set G711_U\n";
		}
		if (strstr(pPartyName, "G722_64") != NULL)
		{
			partyAudioMode.SetBitRate(G722_m1);
			pH320Cap->SetAudioCap(e_G722_64);
			str << "set G722_64\n";
		}
		if (strstr(pPartyName, "G722_56") != NULL)
		{
			partyAudioMode.SetBitRate(G722_m2);
			pH320Cap->SetAudioCap(e_G722_56);
			str << "set G722_56\n";
		}
		if (strstr(pPartyName, "G722_48") != NULL)
		{
			partyAudioMode.SetBitRate(G722_m3);
			pH320Cap->SetAudioCap(e_G722_48);
			str << "set G722_48\n";
		}
		if (strstr(pPartyName, "G722_1_C_48") != NULL)
		{
			partyAudioMode.SetBitRate(G7221_AnnexC_48k);
			pH320Cap->SetAudioCap(e_G722_1_Annex_C_48);
			str << "set G722_1_C_48\n";
		}
		if (strstr(pPartyName, "G722_1_C_32") != NULL)
		{
			partyAudioMode.SetBitRate(G7221_AnnexC_32k);
			pH320Cap->SetAudioCap(e_G722_1_Annex_C_32);
			str << "set G722_1_C_32\n";
		}
		if (strstr(pPartyName, "G722_1_C_24") != NULL)
		{
			partyAudioMode.SetBitRate(G7221_AnnexC_24k);
			pH320Cap->SetAudioCap(e_G722_1_Annex_C_24);
			str << "set G722_1_C_24\n";
		}
		if (strstr(pPartyName, "G722_1_32") != NULL)
		{
			partyAudioMode.SetBitRate(Au_32k);
			pH320Cap->SetAudioCap(e_G722_1_32);
			str << "set G722_1_32\n";
		}
		if (strstr(pPartyName, "G722_1_24") != NULL)
		{
			partyAudioMode.SetBitRate(Au_24k);
			pH320Cap->SetAudioCap(e_G722_1_24);
			str << "set G722_1_24\n";
		}
		if (strstr(pPartyName, "SIREN14_48") != NULL)
		{
			partyAudioMode.SetBitRate(Au_Siren14_48k);
			pH320Cap->SetNScapSiren1448();
			str << "set SIREN14_48\n";
		}
		if (strstr(pPartyName, "SIREN14_32") != NULL)
		{
			partyAudioMode.SetBitRate(Au_Siren14_32k);
			pH320Cap->SetNScapSiren1432();
			str << "set SIREN14_32\n";
		}
		if (strstr(pPartyName, "SIREN14_24") != NULL)
		{
			partyAudioMode.SetBitRate(Au_Siren14_24k);
			pH320Cap->SetNScapSiren1424();
			str << "set SIREN14_24\n";
		}

		if (strstr(pPartyName, "G728") != NULL)
		{
			WORD isG728 = FALSE;
			std::string key = "G728_ISDN";
			BOOL bIsG728ISDN = FALSE;
			CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			pSysConfig->GetBOOLDataByKey(key, bIsG728ISDN);
			if (bIsG728ISDN)
			{
				partyAudioMode.SetBitRate(G728);
				pH320Cap->SetAudioCap(e_Au_16k);
				str << "G728\n";
			}
		}
		// Video formats
		WORD force_video = 0;
		if (strstr(pPartyName, "H261_CIF30") != NULL)
		{
			force_video = 1;
			pH320Cap->SetH261Caps(V_Cif, V_1_29_97, V_1_29_97);
			;
			partyVidMode.SetFreeBitRate(TRUE);
			partyVidMode.SetH261VidMode(1, V_Cif, V_1_29_97, V_1_29_97);
			str << "set H261_CIF30\n";
		}
		if (strstr(pPartyName, "H263_CIF30") != NULL)
		{
			force_video = 1;
			pH320Cap->GetCapH263()->SetOneH263Cap(H263_CIF, MPI_1);
			partyVidMode.SetFreeBitRate(TRUE);
			partyVidMode.SetVidMode(H263);
			partyVidMode.SetVidImageFormat(H263_CIF);
			partyVidMode.SetH263Mpi(MPI_1);
			str << "set H263_CIF30\n";
		}

		if (strstr(pPartyName, "H264_CIF30") != NULL)
		{

			force_video = 1;
			pH320Cap->RemoveH264Caps();
			CCapSetH264* pCapSetH264 = new CCapSetH264;
			pCapSetH264->Create((BYTE)H264_Level_2, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
			pH320Cap->AddH264CapSet(pCapSetH264);
			partyVidMode.SetH264VidMode(*pCapSetH264);
			str << "set H264_CIF30\n";
		}

		// xfer mode according to party rate
		pPartyScm->SetXferMode(0, pConfParty->GetNetChannelNumber(), 1);

		if (!force_video)
		{

			pH320Cap->SetH261Caps(V_Cif, V_1_29_97, V_1_29_97); // add H.261 to local caps
			pH320Cap->GetCapH263()->SetOneH263Cap(H263_CIF, MPI_1);

			DWORD h264Rate = pConfParty->GetNetChannelNumber() * rate64K;
			eVideoQuality vidQuality = GetCommConf()->GetVideoQuality();
			H264VideoModeDetails h264VidModeDetails;
			GetH264VideoParams(h264VidModeDetails, h264Rate, vidQuality, eHD1080At60Symmetric, FALSE);
			pPartyScm->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR, 1);

			CCapSetH264* pCapSetH264 = new CCapSetH264;
			pCapSetH264->Create(h264VidModeDetails.levelValue, h264VidModeDetails.maxMBPS, h264VidModeDetails.maxFS, h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR);
			pH320Cap->AddH264CapSet(pCapSetH264);

			pPartyScm->m_vidMode.SetH263VidMode(1, AUTO);
			pPartyScm->m_vidMode.SetVidMode(H264);
			partyVidMode = pPartyScm->m_vidMode;
		}
		pPartyScm->SetAudMode(partyAudioMode);
		pPartyScm->SetVidMode(partyVidMode);

		// encryption

		BYTE is_conf_encrypted = m_pCommConf->GetIsEncryption();
		if (is_conf_encrypted == YES)
		{
			PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam is_conf_encrypted == YES");
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam is_conf_encrypted == NO");
		}

		BYTE is_party_encrypted = pConfParty->GetIsEncrypted();
		if (is_party_encrypted == YES)
		{
			PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam is_party_encrypted==YES");
		}
		else if (is_party_encrypted == AUTO)
		{
			PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam is_party_encrypted==AUTO");
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam is_party_encrypted==NO");
		}

		BYTE bShouldEncrypt = NO;
		BYTE bShouldDisconnectOnEncryptionFail = NO;
		::ResolveEncryptionParameters(GetCommConf()->GetEncryptionType(), pConfParty, bShouldEncrypt, bShouldDisconnectOnEncryptionFail);
		if (bShouldEncrypt == YES)
		{
			// isdn_encryption
			PTRACE2(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam set Encryp_On [enc_trace] , ", pPartyName);
			pPartyScm->SetOtherEncrypMode(Encryp_On);
		}
		else
		{
			pPartyScm->SetOtherEncrypMode(Encryp_Off);
			pH320Cap->RemoveEncCaps();
		}
		pPartyScm->SetShouldDisconnectOnEncrypFailure(bShouldDisconnectOnEncryptionFail);

		if (strstr(pPartyName, "H239") != NULL)
		{
			pPartyScm->SetContentFromComMode(*(m_pUnifiedComMode->GetIsdnComMode()));

			CCapH239 H239Cap;
			H239Cap.CreateCapH239(pPartyScm->GetXferMode(), pPartyScm->IsFreeVideoRate(), pPartyScm->m_lsdMlpMode.GetlsdCap() != LSD_NONE, pPartyScm->m_contentMode.GetContentLevel());
			pH320Cap->SetOnlyExtendedVideoCaps(&H239Cap);
			pH320Cap->SetH239ControlCap(1);
			str << "set H239\n";
		}

		*pPartyTransmitScm = *pPartyScm;

		PTRACE2(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam: ", str.str().c_str());

	} //end ##FORCE_MEDIA

	else
	{
		*pPartyScm = *(m_pUnifiedComMode->GetIsdnComMode());
		WORD numOfChannels = 0;
		if (pConfParty->GetNetChannelNumber() != 0xFF)
			numOfChannels = pConfParty->GetNetChannelNumber();
		else
			numOfChannels = pPartyScm->GetNumChnl();
		pPartyScm->SetXferMode(0, numOfChannels, 1);

		BYTE is_conf_encrypted = m_pCommConf->GetIsEncryption();
		if (is_conf_encrypted == YES)
		{
			PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam is_conf_encrypted == YES");
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam is_conf_encrypted == NO");
		}

		BYTE is_party_encrypted = pConfParty->GetIsEncrypted();
		if (is_party_encrypted == YES)
		{
			PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam is_party_encrypted==YES");
		}
		else if (is_party_encrypted == AUTO)
		{
			PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam is_party_encrypted==AUTO");
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam is_party_encrypted==NO");
		}

		BYTE isPartyEncryptionOn = NO;
		if (is_party_encrypted == YES || (is_conf_encrypted == YES && is_party_encrypted == AUTO))
		{
			isPartyEncryptionOn = YES;
			PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam Setting isPartyEncryptionOn to YES");
		}

		BYTE bShouldEncrypt = NO;
		BYTE bShouldDisconnectOnEncryptionFail = NO;
		::ResolveEncryptionParameters(GetCommConf()->GetEncryptionType(), pConfParty, bShouldEncrypt, bShouldDisconnectOnEncryptionFail);
		if (bShouldEncrypt == YES)
		{
			// isdn_encryption
			PTRACE2(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam set Encryp_On [enc_trace] , ", pPartyName);
			PTRACE2INT(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam set disconnect on Encryp failure -  ", bShouldDisconnectOnEncryptionFail);
			pPartyScm->SetOtherEncrypMode(Encryp_On);
		}
		else
		{
			pPartyScm->SetOtherEncrypMode(Encryp_Off);
			pH320Cap->RemoveEncCaps();
		}
		pPartyScm->SetShouldDisconnectOnEncrypFailure(bShouldDisconnectOnEncryptionFail);

		if (pPartyScm->IsFreeVideoRate())
		{
			//SD changes - use decision matrix regard to conf rate / setup rate / party reservation rate
			DWORD decisionRate = 0;
			DWORD callRate = m_pUnifiedComMode->GetCallRate();
			BYTE numOfChannels = pConfParty->GetNetChannelNumber();
			DWORD partyReservationRate = 0;
			if (numOfChannels != 0xFF)
				partyReservationRate = numOfChannels * rate64K; //?? //callWidth * 1000;//pConfParty->
			else
				partyReservationRate = callRate;
			BYTE partyVidProtocol = pConfParty->GetVideoProtocol();
			BYTE scmProtocolInReservationTerms = pPartyScm->GetVidMode();
			WORD isAutoVidScm = (partyVidProtocol == AUTO) ? TRUE : FALSE;
			eVideoQuality vidQuality = GetCommConf()->GetVideoQuality();

			decisionRate = partyReservationRate; //((partyReservationRate == 0xFFFFFFFF) ? callRate : partyReservationRate);

			BYTE bRecalcH264 = IsNeedRecalcH264ParamsAccordingToPartySettings(pConfParty); //VNGR-21510

			if ((partyVidProtocol == H264) && (scmProtocolInReservationTerms != H264))
				bRecalcH264 = TRUE; // conf is H263 and party is H264 => re-calc scm
			else if (((partyVidProtocol == AUTO) || (partyVidProtocol == H264)) && (scmProtocolInReservationTerms == H264))
			{
				BYTE bIsLowerRate = (decisionRate < callRate);
				BYTE bIsHigherConfiguraedRate = ((decisionRate > callRate) && (decisionRate == partyReservationRate)); //if it is because of a higher setup rate - we won't change scm
				if (bIsLowerRate || bIsHigherConfiguraedRate)
					bRecalcH264 = TRUE;
			}

			if (bRecalcH264)
			{
				H264VideoModeDetails h264VidModeDetails;

				// isdn behaves like it does not support high profile in low rates
				// high profile resolution slider threshold for version 7.0.1
				BOOL bEnableHighfProfileInIsdn = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_HIGH_PROFILE_WITH_ISDN);
				GetH264VideoParamsAccordingToPartySettings(h264VidModeDetails, pConfParty, decisionRate, bEnableHighfProfileInIsdn);

				BOOL bEnableHighfProfile = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_HIGH_PROFILE);
				if (bEnableHighfProfile && bEnableHighfProfileInIsdn)
				{
					if (IsFeatureSupportedBySystem(eFeatureH264HighProfile) && (GetVideoSession() == CONTINUOUS_PRESENCE))
						h264VidModeDetails.profileValue = H264_Profile_High;
				}
				pPartyScm->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR, isAutoVidScm);
			}

			// Add H.261 to party scm
			APIS8 qcifMpi = 0;
			APIS8 cifMpi = 0;
			WORD isdnFormat = 0;

			DWORD vidRate = partyReservationRate / 1000;
			::Get261VideoCardMPI(vidRate, &qcifMpi, &cifMpi, vidQuality);

			if (cifMpi == -1)
			{
				isdnFormat = V_Qcif;
				qcifMpi += 21; // according to Table A1 [H221]
				cifMpi = 0;
			}
			else
			{
				isdnFormat = V_Cif;
				qcifMpi += 21; // according to Table A1 [H221]
				cifMpi += 21;  // according to Table A1 [H221]
			}

			pPartyScm->m_vidMode.SetH261VidMode(isAutoVidScm, isdnFormat, qcifMpi, cifMpi);

			// WE SHOULD DECIDE WHAT KIND OF H.263 CAPS TO INSERT HERE (THIS IS OUR LOCAL CAPS)
			// AT THE MOMENT - WE DECLARE H263 CIF 30  (Eitan)
			WORD highestResolution = H263_CIF;
			APIS8 buffer[highestResolution + 1];
			memset(buffer, -1, highestResolution + 1);
			CH263VideoMode::Get263VideoCardMPI(vidRate * 10 - 1, buffer, vidQuality);
			CCapH263 CapH263;

			for (int i = H263_QCIF_SQCIF; i <= highestResolution; i++)
			{
				if (buffer[i] != -1)
				{
					pPartyScm->m_vidMode.SetH263VidMode(1, i, buffer[i] - 1);
					CapH263.SetOneH263Cap(i, buffer[i] - 1);
				}
			}

			h263_4CifMpi = CH263VideoMode::GetH263Cif4VideoCardMPI(vidRate * 10 - 1, vidQuality);
			switch (partyVidProtocol)
			{
				case (H264):
				{
					pPartyScm->m_vidMode.SetVidMode(H264);
					break;
				}
				case (H263):
				{
					pPartyScm->m_vidMode.SetVidMode(H263);
					pPartyScm->SetHd720Enabled(FALSE);
					pPartyScm->SetHd1080Enabled(FALSE);
					pPartyScm->SetHd720At60Enabled(FALSE);
					break;
				}
				case (H261):
				{
					pPartyScm->m_vidMode.SetH261VidMode(isAutoVidScm, isdnFormat, qcifMpi, cifMpi);
					pPartyScm->SetHd720Enabled(FALSE);
					pPartyScm->SetHd1080Enabled(FALSE);
					pPartyScm->SetHd720At60Enabled(FALSE);
					h263_4CifMpi = -1;
					break;
				}
				case (AUTO):
				{
					pPartyScm->m_vidMode.SetVidMode(H264);
					break;
				}
			}

			pH320Cap->SetH261Caps(isdnFormat, qcifMpi, cifMpi); // add H.261 to local caps
			pH320Cap->Create(*pPartyScm, &CapH263, NULL, YES);

			*pPartyTransmitScm = *pPartyScm;
			//in CP HD720at30fps asymmetric mode we set the target transmit scm to HD720 while our local caps are SD30
			//in CP HD1080 asymmetric mode we set the target transmit scm to HD1080 while our local caps are HD720
			//in CP HD720at60fps asymmetric mode we set the target transmit scm to HD720at60 while our local caps are SDat 60
			BYTE isHd720At30Enabled = pPartyScm->IsHd720Enabled();
			BYTE isHd1080Enabled = pPartyScm->IsHd1080Enabled();
			BYTE isHd720At60Enabled = pPartyScm->IsHd720At60Enabled();
			BYTE isHd1080At60Enabled = pPartyScm->IsHd1080At60Enabled();
			if (isHd720At30Enabled || isHd1080Enabled || isHd720At60Enabled || isHd1080At60Enabled)
			{
				PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam HD720At30/HD1080/HD720At60 asymmetric CP - Set Target Transmit TO HD720At30/HD1080/HD720At60/HD1080At60!!!");
				int level;
				long mbps, fs, dpb, brAndCpb, sar;
				Eh264VideoModeType eH264VidModeType = eInvalidModeType;
				if (isHd720At30Enabled)
					eH264VidModeType = eHD720Asymmetric;
				else if (isHd1080Enabled)
					eH264VidModeType = eHD1080Asymmetric;
				else if (isHd1080At60Enabled)
					eH264VidModeType = eHD1080At60Asymmetric;
				else
					eH264VidModeType = eHD720At60Asymmetric;

				GetH264VideoTransmitParamForAsymmetricModes(level, mbps, fs, dpb, brAndCpb, eH264VidModeType);

				sar = H264_ALL_LEVEL_DEFAULT_SAR; // sar for cp is different than sar for vsw
				DWORD productMbps = ConvertMaxMbpsToProduct(mbps);
				DWORD productFs = ConvertMaxFsToProduct(fs);
				DWORD productDpb = ConvertMaxDpbToProduct(dpb);
				DWORD productBr = ConvertMaxBrAndCpbToMaxBrProduct(brAndCpb);

				pPartyTransmitScm->SetH264VideoMode(level, productBr, productMbps, productFs, productDpb,/*maxSAR,*/isAutoVidScm);
			}
		}
		else // VSW
		{
			pH320Cap->Create(*pPartyScm);
			*pPartyTransmitScm = *pPartyScm;
		}
	}

	if (h263_4CifMpi != -1)
	{
		pPartyTransmitScm->m_vidMode.SetVidImageFormat(H263_CIF_4);
		pPartyTransmitScm->m_vidMode.SetH263Mpi(h263_4CifMpi - 1);
	}
	PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam pPartyScm is:");
	pPartyScm->UpdateH221string(TRUE);
	ostringstream strPartyScm;
	pPartyScm->Dump(strPartyScm);

	PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam pPartyTransmitScm is:");
	ostringstream strPartyTransmitScm;
	pPartyTransmitScm->Dump(strPartyTransmitScm);

	PTRACE(eLevelInfoNormal, "CConf::SetH320PartyCapsAndVideoParam pH320 Cap is:");
	pH320Cap->Dump();
}

/////////////////////////////////////////////////////////////////////////////
//In CP, the party video scm isn't is set according to reservation's video protocol
void CConf::ChangeScmOfIpPartyInCpOrCopConf(CIpComMode* pPartyScm, CConfParty* pConfParty, BYTE partyVidProtocol, DWORD setupRate, BYTE bCreateNewScm,DWORD confRate,WORD dialType,DWORD serviceId,CSipCaps* pRmtCaps,RemoteIdent epType, CSipNetSetup* pNetSetup)
{
	PASSERT_AND_RETURN(NULL == pConfParty);
	if (pPartyScm->GetConfType() != kCp && pPartyScm->GetConfType() != kCop)
		return;

	PASSERTMSG_AND_RETURN(NULL == pConfParty, "CConf::ChangeScmOfIpPartyInCpOrCopConf pConfParty is NULL!");

	CPartyConnection* pPartyConnection = GetPartyConnection( pConfParty->GetName());

	if (IsEnableH239() )
	{
		// Find the Max content rate of the conf
		BYTE lConfRate = 0;
		eEnterpriseMode ContRatelevel = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
		if (confRate)
			lConfRate = m_pUnifiedComMode->TranslateIPRateToXferRate(confRate);
		else
			lConfRate = m_pCommConf->GetConfTransferRate();
		CCapSetInfo  lCapInfo((payload_en)lConfRate,0);
		DWORD h323ConfRate = lCapInfo.TranslateReservationRateToIpRate(lConfRate);
		DWORD reservationVideoRate = pConfParty->GetVideoRate();

		TRACEINTO << "ConfRate:" << h323ConfRate << ", VideoRate:" << reservationVideoRate;

		DWORD H323AMCRate = 0;
		ePresentationProtocol presentationProtocol = (ePresentationProtocol)m_pCommConf->GetPresentationProtocol();

		if (reservationVideoRate != 0xFFFFFFFF)
		{
			if(reservationVideoRate != h323ConfRate)
			{
				DWORD reservationRate = TranslateVideoIpRateToCorrectReservationRate(reservationVideoRate,h323ConfRate);
				if (reservationRate == 0)
				{
					PASSERTMSG( reservationRate, "CConf::ChangeScmOfIpPartyInCpOrCopConf - Problem with conditioning");
					H323AMCRate = m_pUnifiedComMode->GetContentModeAMCInIPRate(lConfRate,ContRatelevel, presentationProtocol, m_pCommConf->GetCascadeOptimizeResolution(), m_pCommConf->GetConfMediaType());
				}
				else
				{
					BYTE XferResRate = m_pUnifiedComMode->TranslateIPRateToXferRate(reservationRate*1000);
					H323AMCRate = m_pUnifiedComMode->GetContentModeAMCInIPRate(XferResRate,ContRatelevel, presentationProtocol, m_pCommConf->GetCascadeOptimizeResolution(), m_pCommConf->GetConfMediaType());
				}
			}
			else
				H323AMCRate = m_pUnifiedComMode->GetContentModeAMCInIPRate(lConfRate,ContRatelevel, presentationProtocol, m_pCommConf->GetCascadeOptimizeResolution(), m_pCommConf->GetConfMediaType());
		}
		else
		{
			BYTE inputRate = lConfRate;
			if (epType == MicrosoftEP_Lync_CCS)
			{
				BYTE lSetupRate =m_pUnifiedComMode->TranslateIPRateToXferRate(setupRate);
				if (setupRate < (h323ConfRate*1000))
					inputRate = lSetupRate;
			}

			H323AMCRate = m_pUnifiedComMode->GetContentModeAMCInIPRate(inputRate,ContRatelevel, presentationProtocol, m_pCommConf->GetCascadeOptimizeResolution(), m_pCommConf->GetConfMediaType());

		}
		if (pConfParty->GetPartyMediaType() == eSvcPartyType)
		{
			PTRACE(eLevelInfoNormal,"CConf::ChangeScmOfIpPartyInCpOrCopConf : Svc call - H323AMCRate = AMC_128k");
			H323AMCRate = m_pUnifiedComMode->TranslateAMCRateIPRate(AMC_128k);
		}
		PTRACE2INT(eLevelInfoNormal,"CConf::ChangeScmOfIpPartyInCpOrCopConf :  H323AMCRate rate - ",H323AMCRate);
		// Romem
		BOOL isHDContent1080Supported = FALSE;
		BYTE ConfAMCRate = m_pUnifiedComMode->GetContentModeAMC(lConfRate,ContRatelevel, presentationProtocol, m_pCommConf->GetCascadeOptimizeResolution(), m_pCommConf->GetConfMediaType());

		BYTE newContentProtocol = SelectContentProtocol(FALSE);
		BYTE HDResMpi = 0;
		// Extract Conf caps and not HC caps

		eXcodeRsrcType eXCodeEncoderType;
		//In case this is Link party we want to set FIXED Content mode to prevent change mode.
		BYTE LinkCascadeMode = pConfParty->GetCascadeMode();

		PTRACE2INT(eLevelInfoNormal,"CConf::ChangeScmOfIpPartyInCpOrCopConf - LinkCascadeMode ",LinkCascadeMode);

		if(LinkCascadeMode != CASCADE_MODE_NONE)
		{
			PTRACE(eLevelInfoNormal,"CConf::ChangeScmOfIpPartyInCpOrCopConf - This is Link ");
			eXCodeEncoderType = eXcodeH264LinksEncoder;
		}
		else
			eXCodeEncoderType = eXcodeEncoderDummy;

		isHDContent1080Supported = SelectContentHDResolution(ConfAMCRate,newContentProtocol,HDResMpi,TRUE,eXCodeEncoderType);
		if (H323AMCRate == 0)
		{
            PTRACE(eLevelInfoNormal,"CConf::ChangeScmOfIpPartyInCpOrCopConf - disabling BFCP");
			pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			pPartyScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		}
		else
		{
			CapEnum contentProtocol = GetCurrentContentProtocolInIpValues();

			if (!IsTIPContentEnable())
			{
				pPartyScm->SetContent(H323AMCRate,cmCapReceiveAndTransmit,contentProtocol,isHDContent1080Supported,HDResMpi,  GetCurrentContentIsHighProfile());  //HP content
			}
			else if ( m_pCommConf->GetIsPreferTIP() )
			{
				pPartyScm->SetTIPContent(H323AMCRate,cmCapReceiveAndTransmit,FALSE);
			}
			else //eTipCompatibleVideoAndContent
			{
				pPartyScm->SetTIPContent(H323AMCRate,cmCapReceiveAndTransmit);
			}

			if (pConfParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE)
			{
				SetBfcpInSipPartyScm(pPartyScm);
			}
		}
	}

	CapEnum scmProtocol = (CapEnum)pPartyScm->GetMediaType(cmCapVideo, cmCapTransmit);
	CComModeInfo cmInfo = scmProtocol;
	WORD scmProtocolInReservationTerms = cmInfo.GetH320ModeType();

	CCapSetInfo capInfo(scmProtocol);

	if ((scmProtocol == eH264CapCode) && (capInfo.IsSupporedCap() == NO))
	{
		capInfo = eH263CapCode;
		if(capInfo.IsSupporedCap())
			partyVidProtocol = H263;
		else
		{
			capInfo = eH261CapCode;
			if(capInfo.IsSupporedCap())
				partyVidProtocol = H261;
		}
	}
	else if ((scmProtocol == eH263CapCode) && (capInfo.IsSupporedCap() == NO))
		return;

	if(pPartyScm->GetConfType() == kCp)
	{
		//SD changes - use decision matrix regard to conf rate / setup rate / party reservation rate
		DWORD decisionRate 			= 0;
		DWORD callRate 				= m_pUnifiedComMode->GetCallRate();
		DWORD partyReservationRate 	= pConfParty->GetVideoRate();
		if (partyReservationRate != 0xFFFFFFFF)
		{
			DWORD audioRate = pPartyScm->GetMediaBitRate(cmCapAudio);
			partyReservationRate += audioRate;
			partyReservationRate *= 1000;
		}

		if ( (partyReservationRate == 0xFFFFFFFF) && (setupRate == 0) )
			decisionRate = callRate;
		else if ( (partyReservationRate != 0xFFFFFFFF) && (setupRate != 0) )
			decisionRate = min (partyReservationRate, setupRate);
		else
			decisionRate = (setupRate ? setupRate : partyReservationRate);

		BYTE bRecalcH264 = FALSE;
		BOOL useHpDescionMatrix =TRUE;
		if ( (partyVidProtocol == H264) && (scmProtocolInReservationTerms != H264))
			bRecalcH264 = TRUE; // conf is H263 and party is H264 => re-calc scm
		if(partyVidProtocol == VIDEO_PROTOCOL_H264 && pPartyScm->GetH264Profile(cmCapReceiveAndTransmit) == H264_Profile_High)
		{
			PTRACE(eLevelInfoNormal,"CConf::ChangeScmOfIpPartyInCpOrCopConf - FORCE h264 BASELINE");
			bRecalcH264 = TRUE;//change from HP to baseline
			useHpDescionMatrix = FALSE;
			pPartyScm->SetH264Profile(H264_Profile_BaseLine,cmCapReceiveAndTransmit);

		}
		else if ( ((partyVidProtocol == AUTO) || (partyVidProtocol == H264) ) && (scmProtocolInReservationTerms == H264))
		{
			BYTE bIsLowerRate = (decisionRate < callRate);
			BYTE bIsHigherConfiguraedRate = ((decisionRate > callRate) && (partyReservationRate != 0xFFFFFFFF && decisionRate <= partyReservationRate)); //if it is because of a higher setup rate - we won't change scm
			if (bIsLowerRate || bIsHigherConfiguraedRate)
				bRecalcH264 = TRUE;
		}
		//Olga - Resolution Slider: a new 'Resolution' field is added in the participant's properties.
		//This field (together with the video bit rate field) will override any general definition in the conference properties.
		if( !bRecalcH264 )
			bRecalcH264 = IsNeedRecalcH264ParamsAccordingToPartySettings( pConfParty );

		eVideoQuality vidQuality = GetCommConf()->GetVideoQuality();
		Eh264VideoModeType partyMaxVideoMode;

		if (bRecalcH264)
		{
			APIU16 lastprofile = 0;
			if(scmProtocol == eH264CapCode)
			{
				lastprofile = pPartyScm->GetH264Profile(cmCapReceiveAndTransmit);
				PTRACE2INT(eLevelInfoNormal,"CConf::ChangeScmOfIpPartyInCpOrCopConf - THIS IS THE PROFILE ",lastprofile);
			}
			BOOL bIsSharpness = NO;

			H264VideoModeDetails h264VidModeDetails;
			GetH264VideoParamsAccordingToPartySettings( h264VidModeDetails, pConfParty, decisionRate,useHpDescionMatrix );
			pPartyScm->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR, cmCapReceiveAndTransmit);
			if(scmProtocol == eH264CapCode)
			{
				pPartyScm->SetH264Profile(lastprofile,cmCapReceiveAndTransmit);
			}
		}
		else
		{
			BOOL bMsEnviroment = FALSE;

			if(pConfParty->GetNetInterfaceType() != H323_INTERFACE_TYPE)
			{
				CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(serviceId);
				if( pService != NULL && pService->GetConfigurationOfSipServers() )
				{
					if(pService->GetSipServerType() == eSipServer_ms)
						bMsEnviroment = TRUE;
				}

			}

			if	(partyVidProtocol == RTV && bMsEnviroment && (pConfParty->GetNetInterfaceType() != H323_INTERFACE_TYPE) && IsFeatureSupportedBySystem(eFeatureRtv) )
			{
				RTVVideoModeDetails rtvVidModeDetails;
				BYTE partyResolution =  pConfParty->GetMaxResolution();
				BYTE maxConfResolution = m_pCommConf->GetConfMaxResolution();
				BYTE maxPartyResolution = ( partyResolution == eAuto_Res && maxConfResolution != eAuto_Res) ? maxConfResolution : partyResolution;

				if( partyResolution == eAuto_Res )
				{
					partyMaxVideoMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)maxPartyResolution,vidQuality );
					TRACEINTO << "RTV -  partyResolution is Auto_Res ==> chosen partyMaxVideoMode = " << (WORD)partyMaxVideoMode;
					GetRtvVideoParams(rtvVidModeDetails, decisionRate, vidQuality, partyMaxVideoMode);
				}
				else
				{
					partyMaxVideoMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)partyResolution,vidQuality);
					TRACEINTO << "RTV -  partyResolution = " << (WORD)partyResolution << " ==> chosen h264VidMode = " << (WORD)partyMaxVideoMode;
					GetRtvVideoParams(rtvVidModeDetails, decisionRate, vidQuality, partyMaxVideoMode);
				}

				pPartyScm->SetRtvVideoParams(rtvVidModeDetails,cmCapReceiveAndTransmit);
			}
		}


		/*
		BOOL checkIfRemoteSDPisTipCompatible = TRUE;
		BOOL remoteSDPAvailable = FALSE;
	    WORD confPartyConnectionType         = TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());

	    if ( (confPartyConnectionType==DIALIN) && pRmtCaps && (pRmtCaps->GetNumOfCapSets()!=0) )  // Dial in with SDP (TIP call from Polycom EPs feature)
	    {
	        checkIfRemoteSDPisTipCompatible = ::CheckIfRemoteSdpIsTipCompatible(pRmtCaps,TRUE);
	        remoteSDPAvailable = TRUE;
	    }

		BYTE bIsPossibleTipCall = ( CheckIfTipEnableByParams(pPartyScm, pConfParty, callRate, partyReservationRate) && checkIfRemoteSDPisTipCompatible );

		PTRACE2INT(eLevelInfoNormal,"CConf::ChangeScmOfIpPartyInCpOrCopConf checkIfRemoteSDPisTipCompatible:",checkIfRemoteSDPisTipCompatible);
		PTRACE2INT(eLevelInfoNormal,"CConf::ChangeScmOfIpPartyInCpOrCopConf bIsPossibleTipCall:",bIsPossibleTipCall);
		*/

	    WORD confPartyConnectionType    = TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());
		BYTE bIsTipCall 				= pConfParty->GetIsTipCall();
		BOOL remoteSDPAvailable 		= (confPartyConnectionType==DIALIN) && pRmtCaps && (pRmtCaps->GetNumOfCapSets()!=0);

		PTRACE2INT(eLevelInfoNormal,"CConf::ChangeScmOfIpPartyInCpOrCopConf bIsTipCall: ",bIsTipCall);

		if (bIsTipCall)
		{
			SetPartyScmForTip(pPartyScm, pConfParty, pNetSetup, serviceId, remoteSDPAvailable); //shira-TIP
			//pPartyScm->Dump("CConf::ChangeScmOfIpPartyInCpOrCopConf - scm " ,eLevelInfoNormal);
		}
		else
		{
			pConfParty->SetIsTipCall(FALSE);
			pPartyScm->SetTipMode(eTipModeNone);
			pPartyScm->SetTipAuxFPS(eTipAuxNone);
		}

		pPartyScm->Dump("CConf::ChangeScmOfIpPartyInCpOrCopConf - scm " , eLevelInfoNormal);
	}
	else if (pPartyScm->GetConfType() == kCop)
	{
		if (pConfParty->GetVideoRate() !=0xFFFFFFFF)
		{
			// In this stage, the scm receive is the unified mode, that was built according to highest cop level. We need to check if the Party rate is fit for this level.
			CCOPConfigurationList* pCOPConfigurationList = m_pCommConf->GetCopConfigurationList();
			CCopVideoParams* pCopHighestLevelParams = pCOPConfigurationList->GetVideoMode(0);
			if (pConfParty->GetVideoRate() < GetMinBitRateForCopLevel(pCopHighestLevelParams->GetFormat(),pCopHighestLevelParams->GetFrameRate(),pCopHighestLevelParams->GetProtocol()))
			{
				PTRACE2INT(eLevelInfoNormal,"CConf::ChangeScmOfIpPartyInCpOrCopConf : Change scm receive video according to party rate - ", pConfParty->GetVideoRate());
				// Change the scm receive video:
				sCopH264VideoMode copH264VideoMode;
				CCopVideoModeTable* pCopTable = new CCopVideoModeTable;
				BOOL isUseHD1080 = FALSE;
				APIU16 profile = H264_Profile_BaseLine;

				if (IsFeatureSupportedBySystem(eFeatureH264HighProfile))
				{
					if( pCOPConfigurationList->IsHighProfileFoundInOneLevelAtLeast() )
						profile = H264_Profile_High;
				    isUseHD1080 = TRUE;
				}
				pCopTable->GetSignalingH264ModeAccordingToReservationParams(pCopHighestLevelParams, copH264VideoMode, isUseHD1080, pConfParty->GetVideoRate());
				pPartyScm->SetH264Scm(profile, copH264VideoMode.levelValue, copH264VideoMode.maxMBPS, copH264VideoMode.maxFS, copH264VideoMode.maxDPB, copH264VideoMode.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, copH264VideoMode.maxStaticMbps, cmCapReceive);
				POBJDELETE(pCopTable);
			}
		}
	}

	Eh264VideoModeType vidMode = GetMaxVideoModeBySysCfg();
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();

	if (partyVidProtocol == H263)
	{
		pPartyScm->SetHighestH263ScmForCP(cmCapReceiveAndTransmit, GetCommConf()->GetVideoQuality());
	}
	if (partyVidProtocol == H261)
		pPartyScm->SetHighestH261ScmForCP(cmCapReceiveAndTransmit, GetCommConf()->GetVideoQuality());
}

//////////////////////////////////////////////////////////////////////////
BYTE CConf::DecideOnProtocolForCpAuto(CConfParty* pConfParty)
{
	return H264;
}

/////////////////////////////////////////////////////////////////////////////
void CConf::RemovePartyConnection(CTaskApp* pParty, BYTE isCancel)
{
	// the reason we send message to remove CPartyConnection object
	// is to be sure this is the last message of the party that will be
	// received by the conference. In this way we can guarantee that no party
	// message will be executed in the conference after PartyConnection is deleted.

	CSegment* seg = new CSegment;
	*seg << (DWORD)pParty;
	*seg << isCancel;

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	CConfApi* pSelfApi = new CConfApi(GetMonitorConfId());
	pSelfApi->CreateOnlyApi(GetRcvMbx(), this);
	pSelfApi->SendLocalMessage(seg, REMOVPARTYCONNECTION);
	pSelfApi->DestroyOnlyApi();
	POBJDELETE(pSelfApi);
#else
	// save seg if deleted in SendMsg in case of failure - 16357
	CSegment* savedSeg = new CSegment;
	*savedSeg << (DWORD)pParty;
	*savedSeg << isCancel;

	CConfApi* pSelfApi = new CConfApi();
	pSelfApi->CreateOnlyApi(GetRcvMbx(), this);
	STATUS    send_status = pSelfApi->SendMsg(seg, REMOVPARTYCONNECTION);
	// vngr-16047
	// mass disconnection - massage lost
	// to prevent undefined parties from not delete - call function directly (not through message queue)
	if (send_status != STATUS_OK)
	{
		if (m_state == CONNECT)
		{
			PASSERT(send_status);
			OnConfRemovePartyConnection(savedSeg);
		}
		else if (m_state == TERMINATION)
		{
			OnConfRemovePartyConnectionTerminate(savedSeg);
		}
	}
	pSelfApi->DestroyOnlyApi();
	POBJDELETE(pSelfApi);
	POBJDELETE(savedSeg);
#endif
}

 /////////////////////////////////////////////////////////////////////////////
void  CConf::AdjustVideoCapToHW(WORD action, CPartyConnection* pPartyConnection)
{
	PASSERT_AND_RETURN(!action);

	//The new video card supports several resolutions (higher then CIF)
	//which their FPS are less then 30. Therefore we have to find the minimum
	//between end point cap and card.
	WORD HighestResolution = H263_CIF;
	CCapH323*		pLocalCapH323 = NULL;
	WORD protocol = m_pCommConf->GetVideoProtocol();

	// update m_pUnifiedComMode
	switch(protocol)
	{
	case H264:
	case AUTO:
		{
			eVideoQuality vidQuality = GetCommConf()->GetVideoQuality();
			DWORD confRate = m_pUnifiedComMode->GetCallRate();

			//Olga - Resolution Slider: In the conference profile it's possible to set 'Max conference resolution'
			//       which limits the conference resolution regardless of the call rate.
			BYTE maxConfResolution = m_pCommConf->GetConfMaxResolution();
			Eh264VideoModeType resourceMaxVideoMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)maxConfResolution ,vidQuality);
			Eh264VideoModeType resourcesSliderMax =  GetMaxVideoModeBySysCfg();
			if(resourcesSliderMax < resourceMaxVideoMode )
			{
				resourceMaxVideoMode = resourcesSliderMax;
				PTRACE2INT(eLevelInfoNormal,"CConf::AdjustVideoCapToHW res change according to slider to, ",(WORD)resourceMaxVideoMode);
			}

			/************************************************************************************************/
			/************************************************************************************************/
// 			// 03-11-11 1080 60 feature - merav: This code should be removed - only for integration testing

// 			BOOL bIsSupport1080_60 = NO;
// 			CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
// 		    if (pSysConfig)
// 			   pSysConfig->GetBOOLDataByKey("HD_1080_60_FPS", bIsSupport1080_60);
// 		    if(bIsSupport1080_60 == YES)
// 		    {
// 			   resourceMaxVideoMode = eHD1080At60Asymmetric;
// 		    }

//		    PTRACE2INT(eLevelInfoNormal,"CConf::AdjustVideoCapToHW removed 1080_60 patch: res change according to slider to, ",(WORD)resourceMaxVideoMode);

			/************************************************************************************************/
			/************************************************************************************************/
			BOOL isHighProfile = IsFeatureSupportedBySystem(eFeatureH264HighProfile);
			BOOL bEnableHighfProfile = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_HIGH_PROFILE);
			if(!bEnableHighfProfile)
				isHighProfile = FALSE;
			H264VideoModeDetails h264VidModeDetails;
			GetH264VideoParams(h264VidModeDetails, confRate, vidQuality, resourceMaxVideoMode, isHighProfile);
			h264VidModeDetails.profileValue = H264_Profile_BaseLine;

			if(isHighProfile)
			{
				h264VidModeDetails.profileValue = H264_Profile_High; // Always declare high profile in Breeze/CP
				PTRACE(eLevelInfoNormal,"CConf::AdjustVideoCapToHW set high profile");
			}
			m_pUnifiedComMode->SetH264VidMode(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR, m_pCommConf->GetVideoProtocol());
			break;
		}
	case H263:
		{
			m_pUnifiedComMode->SetProtocolClassicVidMode(protocol, 1, 1, 0, 0,m_pCommConf->GetVideoProtocol());
			break;
		}
	case H261:
		{
			m_pUnifiedComMode->SetProtocolClassicVidMode(H261,m_pCommConf->GetQCIFframeRate(),m_pCommConf->GetCIFframeRate(),0,0,m_pCommConf->GetVideoProtocol());
			break;
		}
		case VP8:
		{//N.A. DEBUG VP8
			VP8VideoModeDetails vp8VideoDetails;
			m_pUnifiedComMode->SetVP8VidMode(vp8VideoDetails, protocol);
			break;
		}
	}

	if(action == UpdateSCM)
		return;
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::SetConfTerminateCause(const BYTE  conf_terminate_cause)
{
	TRACESTR(eLevelInfoNormal) <<"CConf::SetConfTerminateCause, conf_terminate_cause = " << (WORD)conf_terminate_cause;
	m_confTerminateCause=conf_terminate_cause;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CConf::GetConfTerminateCause () const
{
	return m_confTerminateCause;
}

/////////////////////////////////////////////////////////////////////////////
void CConf::ConnectIpParty(CConfParty* pConfParty, const char* avServiceNameStr, WORD welcomeMsgTime, DWORD connectDelay, DWORD roomID, eTypeOfLinkParty partyType)
{
	TRACEINTO << "MonitorPartyId:" << pConfParty->GetPartyId() << ", PartyName:" << pConfParty->GetName();
	CPartyConnection* pPartyConnection = new CPartyConnection;

	PartyControlDataParameters partyControlDataParams;
	memset(&partyControlDataParams, 0, sizeof(partyControlDataParams));

	PartyControlInitParameters partyControInitParam;
	memset(&partyControInitParam, 0, sizeof(partyControInitParam));

	  SetControlDataParams(partyControlDataParams, pConfParty,NULL,NULL,NULL,TRUE,FALSE,FALSE,Regular,AvMcuLync2013Main,roomID,partyType);
	AjustPartyAndRoomIdByLinkType(partyControlDataParams, pConfParty);
	SetControlInitParams(partyControInitParam, pConfParty, FALSE, avServiceNameStr, welcomeMsgTime, connectDelay, 0, NULL);

	pPartyConnection->ConnectIP(partyControInitParam, partyControlDataParams);
	InsertPartyConnection(pPartyConnection);
}

/////////////////////////////////////////////////////////////////////////////
void CConf::ReconnectIpParty(CConfParty* pConfParty, CPartyConnection* pPartyConnection, DWORD redialInterval, DWORD connectDelay)
{
	DeleteTimer(AUTOTERMINATE);

	if (pConfParty && pPartyConnection)
	{
		BYTE interfaceType = pPartyConnection->GetInterfaceType();
		BYTE bIsH323 = (interfaceType == H323_INTERFACE_TYPE);
		BYTE bIsDisconnectState = pPartyConnection->IsDisconnectState();
		WORD welcomMode = 0;

		TRACEINTO << "PartyName:" << pConfParty->GetName() << ", IsDisconnected:" << (int)bIsDisconnectState << ", IsH323:" << (int)bIsH323;

		PartyControlDataParameters partyControlDataParams;
		memset( &partyControlDataParams, 0, sizeof (PartyControlDataParameters) );
		PartyControlInitParameters partyControInitParam;
		memset( &partyControInitParam, 0, sizeof (PartyControlInitParameters) );
		SetControlDataParams(partyControlDataParams, pConfParty, NULL, NULL, NULL, TRUE);
		AjustPartyAndRoomIdByLinkType(partyControlDataParams, pConfParty);
		SetControlInitParams(partyControInitParam, pConfParty, TRUE, NULL, 0, connectDelay, redialInterval, NULL);

		if (pConfParty && pPartyConnection && pPartyConnection->GetDialType() == DIALOUT)
		{
			if (bIsH323 && bIsDisconnectState)
			{
				// Multiple links for ITP in cascaded conference feature: CConf::ReConnectIpParty - rename if you are mainLink
				if (pConfParty->GetPartyType() == eMainLinkParty)
				{
					char mainPartyName[H243_NAME_LEN];
					::CreateMainLinkName(pConfParty->GetName(), (char*)mainPartyName);

					TRACEINTO << "MainPartyName:" << mainPartyName;

					pConfParty->SetName(mainPartyName);

					CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();

					if (pPartyCntl)
					{
						pPartyCntl->SetPartyName(mainPartyName);
						pPartyCntl->SetFullName(pConfParty->GetName(), m_name);
					}
					else
					{
						TRACESTRFUNC(eLevelError) << "Failed, pPartyCntl is NULL";
					}
					pConfParty->SetVisualPartyName(mainPartyName);
				}

				pPartyConnection->ReconnectH323(partyControInitParam, partyControlDataParams);

				UpdateDB(pPartyConnection->GetPartyTaskApp(), DISCAUSE, NO_DISCONNECTION_CAUSE);
				UpdateDB(pPartyConnection->GetPartyTaskApp(), DISQ931, 0);
			}
			else
			{
				char* strReferredBy = pConfParty->GetReferredBy();
				CIpPartyCntl* pPartyCntl = (CIpPartyCntl*)pPartyConnection->GetPartyCntl();
				BYTE bIsRedialImmediately = pPartyCntl && pPartyCntl->IsRedialImmediately();
				if (bIsDisconnectState || (strReferredBy[0] && (bIsRedialImmediately || redialInterval)))
				{
					TRACEINTO << "SIP Party with a special reason to reconnect";
					pPartyConnection->ReconnectSip(partyControInitParam, partyControlDataParams);
					UpdateDB(pPartyConnection->GetPartyTaskApp(), DISCAUSE, NO_DISCONNECTION_CAUSE);
					UpdateDB(pPartyConnection->GetPartyTaskApp(), DISQ931, 0);
				}
				else
					TRACESTRFUNC(eLevelError) << "Party should be delete or not disconnected or not SIP with a special reason";
			}
		}
		else
			TRACESTRFUNC(eLevelError) << "\'PARTY NOT EXIST OR NOT DISCONNECTED\'";
	}
	else
		PASSERTMSG(4, "CConf::ReConnectIpParty - Invalid pointer");
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::ReconnectPstnIsdnParty(CConfParty* pConfParty, CPartyConnection*  pPartyConnection, WORD redialInterval,DWORD connectDelay)
{
  PTRACE2(eLevelInfoNormal, "CConf::ReconnectPstnIsdnParty - ConfName=", m_name);
  DeleteTimer(AUTOTERMINATE);

  //only if conf is not secured enable reconnect
	if(!m_pCommConf->IsConfSecured()){
		if ( pConfParty && pPartyConnection && pPartyConnection->IsDisconnectState() &&
			pPartyConnection->GetDialType() == DIALOUT )  {
			if(pConfParty->GetVoice())
			{
				pPartyConnection->ReconnectPstn(m_name,'\0',
					m_pRcvMbx,/*terminalNumber*/2,/*welcomMode*/0,/*isRecording*/FALSE, redialInterval);
			}
			else // ISDN
			{
				DWORD vidBitrate = 0;
				WORD terminalNumber = 0;
				CCapH320* pH320Caps = new CCapH320;
				CComMode* pPartyScm = new CComMode;
				CComMode* pPartyTransmitScm = new CComMode;
				WORD confPartyConnectionType = TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());
				SetH320PartyCapsAndVideoParam(pPartyScm, pPartyTransmitScm, pH320Caps, pConfParty, vidBitrate, confPartyConnectionType);

				pPartyConnection->ReconnectIsdn(m_name,pH320Caps, pPartyScm, pPartyTransmitScm, m_pRcvMbx,
												terminalNumber,/*isRecording*/FALSE,redialInterval,connectDelay);

				POBJDELETE(pPartyTransmitScm);
				POBJDELETE(pPartyScm);
				POBJDELETE(pH320Caps);

			}
			UpdateDB(pPartyConnection->GetPartyTaskApp(),DISCAUSE,NO_DISCONNECTION_CAUSE);
			UpdateDB(pPartyConnection->GetPartyTaskApp(),DISQ931,0);
		}
		else
			PTRACE2(eLevelError,"CConf::ReconnectPstnIsdnParty : \'PARTY NOT EXIST OR NOT DISCONNECTED\'",m_name);
	}
	else
	{
		PTRACE(eLevelError,"CConf::ReconnectPstnParty : Conference is SECURED.");
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::ConnectPstnIsdnParty(CConfParty* pConfParty, char *avServiceNameStr, WORD welcomeMsgTime, WORD connectDelay)
{
  TRACEINTO << "PartyId:" << pConfParty->GetPartyId() << ", PartyName:" << pConfParty->GetName();

  if (!m_StandByStart)// Only at start Immediately case - delete Auto termination.
    DeleteTimer(AUTOTERMINATE);



	CIsdnNetSetup*  pNetSetup = NULL;
	Phone* pPhone = pConfParty->GetFirstCallingPhoneNumber();

	pNetSetup = new CIsdnNetSetup;//[MAX_CHNLS_IN_PARTY];


	if (pConfParty->GetVoice() == YES)
		pNetSetup->m_callType =ACU_VOICE_SERVICE;
    else
        pNetSetup->m_callType =ACU_DATA_SERVICE;
	if (pPhone != NULL){
		pNetSetup[0].m_called.m_numDigits = strlen(pPhone->phone_number);
		strncpy((char*)pNetSetup[0].m_called.m_digits,pPhone->phone_number,PRI_LIMIT_PHONE_DIGITS_LEN );
		pNetSetup[0].m_called.m_digits[PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';
	}  else  {
		pNetSetup[0].m_called.m_numDigits = 0;
		pNetSetup[0].m_called.m_digits[0] = '\0';
	}
	//get global pointer to service proivers list
	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	const char * serviceName = pConfParty->GetServiceProviderName ();
	const RTM_ISDN_PARAMS_MCMS_S * pIsdnServiceStruct = pConfPartyProcess->GetIsdnService(serviceName);
	ENetworkType networkType = E_NETWORK_TYPE_DUMMY;

	if (pIsdnServiceStruct)
	  {
	    //Set service provider name
	    std::string serviceNameStr = ((char*)pIsdnServiceStruct->serviceName);
	    pConfParty->SetServiceProviderName(serviceNameStr.c_str());

	    //Set net specific service
	    pNetSetup->m_netSpcf = pIsdnServiceStruct->netSpecFacility;

	    //Set Span Type
	    networkType = GetNetoworkType(pIsdnServiceStruct->spanDef.spanType) ;

	    //Set the NumType
	    pNetSetup->m_calling.m_numType=pIsdnServiceStruct->dfltNumType;
	    pNetSetup->m_called.m_numType=pIsdnServiceStruct->dfltNumType;

	    //Set the NumPlan according to the service
	    pNetSetup->m_calling.m_numPlan=pIsdnServiceStruct->numPlan;
	    pNetSetup->m_called.m_numPlan=pIsdnServiceStruct->numPlan;

	    //Set the MCU CLI from the service
	    memset(pNetSetup->m_calling.m_digits,0,PRI_LIMIT_PHONE_DIGITS_LEN);

	    // VNGR-24806
	    WORD len = strlen((char *)pIsdnServiceStruct->mcuCli);
	    if(len >PRI_LIMIT_PHONE_DIGITS_LEN){
	       TRACEINTO << "illegal CLI length = " << len << " , CLI will not be set , PartyName:" << pConfParty->GetName();
	       len = 0;
	    }

	    memcpy(pNetSetup->m_calling.m_digits,pIsdnServiceStruct->mcuCli,len);
	    pNetSetup->m_calling.m_numDigits = len;

	    if (pConfParty->GetVoice() == YES)
	      {
		pNetSetup->m_callType =ACU_VOICE_SERVICE;
	      }


	    //Add the prefix to the called number
	    std::string prefix=(char *)(pIsdnServiceStruct->dialOutPrefix);
	    if (  PRI_LIMIT_PHONE_DIGITS_LEN < (prefix.size() + pNetSetup->m_called.m_numDigits) )
	      {
		PTRACE(eLevelError,"CConf::ConnectPstnIsdnParty : \'PREFIX+PHONE NUMBER IS TOO LONG !!! \'");
		PASSERT(1);
	      }
	    else
	      {
		std::string fullPhone = prefix + (char *)(pNetSetup->m_called.m_digits);
		memset(pNetSetup->m_called.m_digits,0,PRI_LIMIT_PHONE_DIGITS_LEN+1);
		strcpy((char *)(pNetSetup->m_called.m_digits),fullPhone.c_str());
		pNetSetup->m_called.m_numDigits = fullPhone.size();
	      }
	  }
	else
	  {
	    TRACESTR (eLevelError) << "CConf::ConnectPstnIsdnParty Can not find service" << pConfParty->GetName();
	    PASSERT(1);
	  }
	WORD voice_type = pConfParty->GetVoice();
	if	(voice_type == YES)
	  voice_type = GetVoiceType(pConfParty);

	// Save the dial out manually value of the conference
	BYTE StandByStart = m_StandByStart;

	CPartyConnection*  pPartyConnection = new CPartyConnection;
	if(pConfParty->GetVoice())
	{
		// There are parameters we not use, we kept them in interface compatibility purposes
		pPartyConnection->ConnectPSTN(this, pNetSetup,
					      m_pRcvMbx,m_pAudBrdgInterface,m_pConfAppMngrInterface, NULL, NULL, 2,pConfParty->GetPhoneNumber(),DIALOUT,
			pConfParty->GetName(), m_name, '\0',
					      m_monitorConfId, pConfParty->GetPartyId(),networkType, voice_type,
			pConfParty->GetAudioVolume(),pConfParty->GetServiceProviderName(), StandByStart,
			connectDelay, avServiceNameStr, welcomeMsgTime,FALSE);
		InsertPartyConnection(pPartyConnection); //
	}
	else
	{

		DWORD vidBitrate = 0;
		WORD terminalNumber = 0;
		CCapH320* pH320Caps = new CCapH320;
		CComMode* pPartyScm = new CComMode;
		CComMode* pPartyTransmitScm = new CComMode;
		WORD confPartyConnectionType = TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());
		SetH320PartyCapsAndVideoParam(pPartyScm, pPartyTransmitScm, pH320Caps, pConfParty, vidBitrate, confPartyConnectionType);
		WORD numChnl = pConfParty->GetNetChannelNumber();
		DWORD confRate = m_pUnifiedComMode->GetCallRate();
		eTelePresencePartyType eTelePresenceMode = (eTelePresencePartyType)pConfParty->GetTelePresenceMode();


		// There are parameters we not use, we kept them in interface compatibility purposes
		pPartyConnection->ConnectIsdn(this, pNetSetup, pH320Caps, pPartyScm, pPartyTransmitScm, m_pRcvMbx,m_pAudBrdgInterface,
									  m_pVideoBridgeInterface,m_pConfAppMngrInterface, m_pFECCBridge,
									  m_pContentBridge,m_pTerminalNumberingManager,NULL, NULL, terminalNumber,0,
									  numChnl,confPartyConnectionType, pConfParty->GetName(), m_name,
									  m_monitorConfId, pConfParty->GetPartyId(),serviceName, networkType,GetNodeType(pConfParty),
									  voice_type, StandByStart, connectDelay,eTelePresenceMode);

		InsertPartyConnection(pPartyConnection);

		POBJDELETE(pPartyTransmitScm);
		POBJDELETE(pPartyScm);
		POBJDELETE(pH320Caps);
	}
	POBJDELETE(pNetSetup);
}

//--------------------------------------------------------------------------
// This function ends the import action
void CConf::EndImportParty(PartyRsrcID partyId, WORD status)
{
	CParty* pParty = GetLookupTableParty()->Get(partyId);
	PASSERTSTREAM_AND_RETURN(!pParty, "PartyId:" << partyId);

	if (status != STATUS_OK)
	{
		ImportPartyFailed(partyId, status);
		return;
	}

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	if (!pPartyConnection)
	{
		ImportPartyFailed(partyId, STATUS_PARTY_DOES_NOT_EXIST);
		return;
	}

	TRACEINTO << pPartyConnection->GetFullName() << " - Party imported successfully";

	UpdateConfStatus();

	if (!(pPartyConnection->ContinueAddPartyAfterMoveIfNeeded()))
		SetCapCommonDenominator(pParty, YES /*,HC_ADD,network_interface,TRUE*/); // true for try to upgrade from secondary

	// Increment 'connected parties' counter only if back from operator assistance
	CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
	PASSERT_AND_RETURN(!pPartyCntl);

	// Play blip sound when auto cascade link is connected
	if (pPartyCntl->IsPartyCascade() && GetSystemCfgFlagInt<BOOL>(CFG_KEY_CASCADE_LINK_PLAY_TONE_ON_CONNECTION))
	{
		TRACEINTO << "Party is cascade link, play blip sound";

		const CCommConf* pCurConf = GetCommConf();

		if (pCurConf)
		{
			CConfApi confApi;
			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			confApi.SendCAMGeneralActionCommand(pPartyCntl->GetPartyRsrcId(), EVENT_CONF_REQUEST, eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK, 0);
			confApi.DestroyOnlyApi();
		}
	}

	if (m_pCommConf == NULL)
	{
		DeleteSubscriberByRsrcID(pPartyCntl->GetPartyRsrcId());
		ImportPartyFailed(partyId, STATUS_PARTY_DOES_NOT_EXIST);
		return;
	}

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
	if (pConfParty)
	{
		DWORD state = pConfParty->GetPartyState();

		TRACEINTO << pPartyConnection->GetFullName() << ", connection state:" << state;
		if (state == PARTY_CONNECTED || state == PARTY_CONNECTED_PARTIALY)
		{
			if (m_pGatheringManager)
			{
				if (m_pGatheringManager->IsGatheringEnabled())
					m_pGatheringManager->OnPartyConnecting(pConfParty);
			}
		}

		if(pConfParty->GetVoice())
 		{
			TRACEINTO << ", import a audio only party, update the audio participants layout indication";
			pConfParty->SetCountedInAudioIndication(NO);
			UpdateAudioParticipantsCount(pConfParty,YES);
  		}
	}

	ConnectSubscriberByRsrcID(pPartyCntl->GetPartyRsrcId());

	if( IsMixAvcSvcDynamicAllocation() && eMediaStateMixAvcSvc == GetMediaState()){
	  TRACEINTO << " (DYNAMIC_ALLOCATION_AVC_SVC) UpdatePartyAvcSvcMode after move";
	  pPartyCntl->UpdatePartyAvcSvcMode(eMediaStateMixAvcSvc,m_monitorConfId,pPartyConnection->GetMonitorPartyId());
	}


	//VNGR-27030 - secure call indication
	SendSecureMessageWhenAddParty(pPartyConnection);
}
//////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
// This function ends the import action in case of failure
void CConf::ImportPartyFailed(PartyRsrcID partyId, WORD status)
{
	TRACESTRFUNC(eLevelError) << "PartyId:" << partyId << ", status:" << status << " - Failed, Cannot import party";

	ActivateAutoTermenation("CConf::ImportPartyFailed");
}

//--------------------------------------------------------------------------
// This function ends the export party
// After received ENDEXPORT message from the party (or on failure before)
void CConf::EndExportParty(PartyRsrcID partyId, WORD status)
{
	CParty* pParty = GetLookupTableParty()->Get(partyId);
	PASSERTSTREAM_AND_RETURN(!pParty, "PartyId:" << partyId);

	if (status != STATUS_OK)
	{
		TRACEINTO << "PartyId:" << partyId << ", Status :" << status;
		ExportPartyFailed(partyId, status);
		return;
	}

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	if (!pPartyConnection)
	{
		TRACEINTO << "PartyId:" << partyId << ", pPartyConnection:NULL";
		ExportPartyFailed(partyId, STATUS_PARTY_DOES_NOT_EXIST);
		return;
	}

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
	if (!pConfParty)
	{
		TRACEINTO << "PartyId:" << partyId << ", pConfParty:NULL";
		ExportPartyFailed(partyId, STATUS_PARTY_DOES_NOT_EXIST);
		return;
	}

	TRACEINTO << "PartyId:" << partyId << " - Party exported successfully";

	//If party is counted in audio participants layout indication, update the number icon
	if (pConfParty->IsCountedInAudioIndication())
	{
		TRACEINTO << "PartyId:" << partyId << " - A audio only party exported, update the audio indication in layout";
		pConfParty->SetCountedInAudioIndication(YES);
		UpdateAudioParticipantsCount(pConfParty,NO);
	}

	// decrement 'connected parties number'
	DWORD state = pConfParty->GetPartyState();
	if (state == PARTY_CONNECTED || state == PARTY_SECONDARY || state == PARTY_CONNECTED_WITH_PROBLEM)
		m_pCommConf->DecConnectedPartiesNum();
		/* Flora added for LED&LCD Feature */
		LedSysAlarmInd();

	//VNGR-27030 - secure call indication
	SendSecureMessageWhenDelParty(pPartyConnection);

	pPartyConnection = RemovePartyConnection(pParty);
	PASSERT(!pPartyConnection);
	if (!pPartyConnection)
	{
		ExportPartyFailed(partyId, STATUS_PARTY_DOES_NOT_EXIST);
		return;
	}

	// get move type
	CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
	PASSERT(!pPartyCntl);

	// Today we support only Move Default
	COsQueue* pConfRcvMailBox = m_pCommConf->GetRcvMbx();

	m_pCommConf->SetRcvMbx(m_pRcvMbx);
	int cancel_status = m_pCommConf->Cancel(pPartyConnection->GetName());
	m_pCommConf->SetRcvMbx(pConfRcvMailBox);

	PASSERTSTREAM(cancel_status != STATUS_OK, pPartyConnection->GetFullName() << ", Status:" << cancel_status << " - Failed to delete party from DB");

	DetermineTelePresenceConfMode(0);

	UpdateConfStatus();

	pPartyConnection->Destroy();
	POBJDELETE(pPartyConnection);

	ActivateAutoTermenation("CConf::EndExportParty");
}

//--------------------------------------------------------------------------
// This function ends the export party in case of failure
void CConf::ExportPartyFailed(PartyRsrcID partyId, WORD status)
{
	TRACESTRFUNC(eLevelError) << "PartyId:" << partyId << ", status:" << status << " - Failed, Cannot export party" << ", ConfName=" << m_name;

	CParty* pParty = GetLookupTableParty()->Get(partyId);
	if (pParty)
	{
		CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
		if (pPartyConnection)
		  DelParty(pPartyConnection->GetName());
}
}

//--------------------------------------------------------------------------
// This function perform the export operation
void CConf::ExportParty(PartyMonitorID monitorPartyId, ConfMonitorID monitorDestConfId, EMoveType eMoveType)
{
	TRACEINTO << "PartyMonitorId:" << monitorPartyId << ", DestMonitorConfId:" << monitorDestConfId << ", MoveType:" << eMoveType;

	// get part name from DB by party id and conference id
	const char* pPartyName = ::GetpConfDB()->GetPartyName(m_monitorConfId, monitorPartyId);
	PASSERTSTREAM_AND_RETURN(!pPartyName, "Failed, Party not found, monitorPartyId:" << monitorPartyId);

	// get party connection by name
	CPartyConnection* pPartyConnection = GetPartyConnection(pPartyName);
	PASSERTSTREAM_AND_RETURN(!pPartyConnection, "Failed, Party connection not found, monitorPartyId:" << monitorPartyId);

	// get destination conference from DB
	CCommConf* pDestCommConf = ::GetpConfDB()->GetCurrentConf(monitorDestConfId);
	ISDEBUGMODE_SET_VAL("MOVE", 3, pDestCommConf, 0)
	PASSERTSTREAM_AND_RETURN(!pDestCommConf, "Failed, Destination conference not found in DB, monitorDestConfId:" << monitorDestConfId);

	// get conference-party from source conference DB
	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyName);
	PASSERTSTREAM_AND_RETURN(!pConfParty, "Failed, conf Party not found, monitorPartyId:" << monitorPartyId);

	CConfParty* pDestConfParty =  new CConfParty(*pConfParty);

	// Set new monitor party id
	pDestConfParty->SetPartyId(pDestCommConf->NextPartyId());

	pConfParty->SetPartyState(PARTY_CONNECTED_WITH_PROBLEM, m_monitorConfId);

	pPartyConnection->Export(pDestCommConf->GetRcvMbx(), pDestConfParty, monitorDestConfId, pDestConfParty->GetPartyId(), eMoveType);
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::AllocPartyList()
{
	POBJDELETE(m_pPartyList);
	m_pPartyList = new CPartyList;
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::AnnounceNewConfToSipProxy()
{
	const CStructTm* pStartTime = m_pCommConf->GetStartTime();
	const CStructTm* pEndTime = m_pCommConf->GetEndTime();
	DWORD  durationTime = *pEndTime - *pStartTime;
	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();

	CSipProxyManagerApi SipProxyApi;

	for (int i = 0; i < NUM_OF_IP_SERVICES; ++i)
	{
		CConfIpParameters* pServiceParams = pIpServiceListManager->FindServiceByName(m_pCommConf->GetServiceRegistrationContentServiceName(i));
		if (pServiceParams != NULL)
		{
			if (m_pCommConf->GetServiceRegistrationContentRegister(i) == TRUE && !m_pCommConf->IsMeetingRoom() && !m_pCommConf->GetEntryQ() && !m_pCommConf->IsSIPFactory() && !m_pCommConf->GetIsGateway() && !m_pCommConf->GetIsAdHocConf())
			    SipProxyApi.AddConference(pServiceParams->GetServiceId(), m_name, m_pCommConf->GetMonitorConfId(), m_pCommConf->GetEntryQ(), durationTime);
		    }
		}
}

/////////////////////////////////////////////////////////////////////////////
void CConf::AnnounceDelConfToSipProxy()
{
	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	for (int i = 0; i < NUM_OF_IP_SERVICES; ++i)
	{
		CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(i);
		if (pServiceParams == NULL)
			continue;

		if (m_pCommConf->IsMeetingRoom())      //BRIDGE-13381
		{
			TRACEINTO << "Index:" << i << ", ConfName:" << m_name << ", ConfMonitorId:" << m_pCommConf->GetMonitorConfId() << ", IsMeetingRoom:" << (int)m_pCommConf->IsMeetingRoom() << " - Skipped";
			continue;
		}
		else if (m_pCommConf->IsSIPFactory()) //BRIDGE-13381
		{
			TRACEINTO << "Index:" << i << ", ConfName:" << m_name << ", ConfMonitorId:" << m_pCommConf->GetMonitorConfId() << ", IsSIPFactory:" << (int)m_pCommConf->IsSIPFactory() << " - Skipped";
			continue;
		}

			TRACEINTO << "Index:" << i << ", ConfName:" << m_name << ", ConfMonitorId:" << m_pCommConf->GetMonitorConfId();

		CSipProxyManagerApi SipProxyApi;
		SipProxyApi.DelConference(i, m_name, m_pCommConf->GetMonitorConfId());
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CConf::AnnounceLobbyConfOnAir(ConfMonitorID confMonitorID, char* targetConfName)
{
	CLobbyApi* pLobbyApi = (CLobbyApi*)::GetpLobbyApi();
	pLobbyApi->ConfOnAir(confMonitorID, targetConfName);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CConf::AnnounceLobbyConfOnAirIfNeeded()
{
	const char* targetConfName = m_pCommConf->GetName();

	//MR/EQ or this conf born AdHoc by EntryQueue
	if ((m_pCommConf->IsMeetingRoom()) || (YES == m_pCommConf->GetEntryQ()) || (YES == m_pCommConf->GetIsAdHocConf()))
	{
		ConfMonitorID confMonitorID = m_pCommConf->GetMonitorConfId();

		if (m_pCommConf->GetEntryQ())
		{ //Eq ongoing name is different from EQ reservation
			targetConfName = ::GetpMeetingRoomDB()->GetOrigionEqReservationName(m_pCommConf->GetName(), confMonitorID);

			TRACEINTO << "ConfName:" << targetConfName << " - Sending to Lobby EQ original reservation name instead of ongoing name";
		}
		AnnounceLobbyConfOnAir(confMonitorID, (char*)targetConfName);
	}
	else
	{
		TRACEINTO << "ConfName:" << targetConfName << " - Conference is not MR/EQ/Ad-Hoc, no need to send Lobby a message";
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STATUS  CConf::SyncRsrcAllocConfSpreading(OPCODE opcode)
{
	CProcessBase* process = CProcessBase::GetProcess();
	eSessionType  confType = GetSessionTypeForResourceAllocator();
	CSegment*     seg      = new CSegment;

	TRACEINTO << "Opcode:" << opcode;

	CONFERENCE_RSRC_IND_PARAMS_S conferenceIndicationParams;
	memset(&conferenceIndicationParams, 0, sizeof(conferenceIndicationParams));

	CONF_RSRC_IND_PARAMS_S confIndicationParams;
	memset(&confIndicationParams, 0, sizeof(confIndicationParams));

	// Treat terminate also
	if (opcode == ALLOCATE_CONTENT_XCODE_REQ)
	{
		CONFERENCE_RSRC_REQ_PARAMS_S conferernceRequestParams;
		memset(&conferernceRequestParams, 0, sizeof(conferernceRequestParams));
		SetConfRsrcReqForXCode(conferernceRequestParams);
		seg->Put((BYTE*)(&conferernceRequestParams), sizeof(conferernceRequestParams));
	}
	else
	{
		CONF_RSRC_REQ_PARAMS_S confRequestParams;
		memset(&confRequestParams, 0, sizeof(confRequestParams));
	    confRequestParams.monitor_conf_id = m_monitorConfId;
	    confRequestParams.sessionType     = confType;
	    confRequestParams.status = m_rsrcDeallocateStatus;
	    confRequestParams.confMediaType = (eConfMediaType)m_pCommConf->GetConfMediaType();
	    confRequestParams.mrcMcuId = m_pCommConf->GetMrcMcuId();

	if (GetVideoSession() == VIDEO_SESSION_COP)
		SetConfRsrcReqForCop(confRequestParams);

		seg->Put((BYTE*)(&confRequestParams), sizeof(confRequestParams));
		DumpRsrcReqToTrace(confRequestParams);
	}

	CTaskApi resourceManagerApi(eProcessResource, eManager);

	STATUS returnStatus = STATUS_OK;
	STATUS   status       = STATUS_OK;

	CSegment rspMsg;
    OPCODE resOpcode;
	STATUS responseStatus = resourceManagerApi.SendMessageSync(seg, opcode, CONF_RSRC_REQ_TOUT, resOpcode, rspMsg);
	TRACEINTO << "ResponseStatus:" << responseStatus;

	if (STATUS_OK == responseStatus)
	{
		if (rspMsg.GetLen() > 0)
        {
			if (opcode == ALLOCATE_CONTENT_XCODE_REQ)
			{
				rspMsg.Get((BYTE*)(&conferenceIndicationParams), sizeof(CONFERENCE_RSRC_IND_PARAMS_S));
				status = conferenceIndicationParams.status;
			}
			else
			{
				rspMsg.Get((BYTE*)(&confIndicationParams), sizeof(CONF_RSRC_IND_PARAMS_S));
				status = confIndicationParams.status;
			}

			if (status != STATUS_OK)
			{
				TRACEINTO << "Status:" << status << " - Conference resources ALLOC/DEALLOC failed";
				if (!GetVideoSession() == VIDEO_SESSION_COP || opcode != ALLOCATE_CONTENT_XCODE_REQ) // in cop conference it's not rare that there are no resources, we will send a fault message when that is the case
					PASSERT(status);
				if (opcode != ALLOCATE_CONTENT_XCODE_REQ)
					m_state = IDLE;
				returnStatus = status;
			}
			else
			{
				TRACEINTO << "Status:" << status << " - Conference resources ALLOC/DEALLOC OK";

				if (GetVideoSession() == VIDEO_SESSION_COP)
				{
					HandleConfRsrcIndForCop(opcode, confIndicationParams);
				}
				if (opcode == ALLOCATE_CONTENT_XCODE_REQ)
				{
					HandleConfRsrcIndForXCode(opcode, conferenceIndicationParams);
				}
				else
					m_ConfRsrcId = confIndicationParams.rsrc_conf_id;
			}
		}
		else
		{
			TRACEINTO << "Conference resources ALLOC/DEALLOC failed, no content";
            if (opcode != ALLOCATE_CONTENT_XCODE_REQ)
            {
            	PASSERT(1);
            	m_state = IDLE;
            }
            else
            	DBGPASSERT(1);

            returnStatus = STATUS_ILLEGAL;
        }
    }
	else  // timeout or dead process
	{
		TRACEINTO << "Conference resources ALLOC/DEALLOC failed, TIME OUT";
	   if (opcode != ALLOCATE_CONTENT_XCODE_REQ)
	   {
		   PASSERT(responseStatus);
	   	   m_state = IDLE;
	   	TRACEINTO_GLA << "Conf state = " << ConfStateToString(m_state);
	   }
	   else
		   DBGPASSERT(1);

	   returnStatus = responseStatus;
	}

	return returnStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CConf::SetCopRsrcs(CONF_RSRC_IND_PARAMS_S &confIndicationParams)
{
	WORD i;
	eLogicalResourceTypes rsrcType;
	DWORD numOfRsrcs = confIndicationParams.num_predefinedRsrcs;
	//In MPM+ evnt mode the 3rd and 4th levels are CIF, in MPMx the 3rd is 4CIF and 4ty is CIF
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();

	if (numOfRsrcs > eLastCopConstRsrcs)
	{
		DBGPASSERT(numOfRsrcs);
		numOfRsrcs = eLastCopConstRsrcs;
	}

	POBJDELETE(m_pCopRsrcs);
	m_pCopRsrcs = new CopRsrcsParams;
	memset(m_pCopRsrcs, 0, sizeof(CopRsrcsParams));

	// Set const resources:
	for (i = 0; i < numOfRsrcs; i++)
	{
		ECopConstRsrcs eRsrcEntry = eCopConstRsrcNone;
		rsrcType = confIndicationParams.allocatedRrcs[i].logicalRsrcType;

		// Find the entry to set the resoure:
		if (rsrcType == eLogical_COP_HD1080_encoder)
			eRsrcEntry = eEncoderLevel1;
		else if (rsrcType == eLogical_COP_HD720_encoder)
		{
			if ((GetRsrcTypeForHighestCopLevel() == eLogical_COP_HD720_encoder) && (m_pCopRsrcs->constRsrcs[eEncoderLevel1].logicalRsrcType == eLogical_res_none))
				eRsrcEntry = eEncoderLevel1;
			else
				eRsrcEntry = eEncoderLevel2;
		}
		else if (rsrcType == eLogical_COP_4CIF_encoder)
			eRsrcEntry = eEncoderLevel3;
		else if (rsrcType == eLogical_COP_CIF_encoder)
			eRsrcEntry = eEncoderLevel4;
		else if (rsrcType == eLogical_COP_VSW_encoder)
			eRsrcEntry = eVswEncoder;
		else if (rsrcType == eLogical_COP_VSW_decoder)
			eRsrcEntry = eVswDecoder;
		else if (rsrcType == eLogical_COP_PCM_encoder)
			eRsrcEntry = ePcmEncoder;
		else if (rsrcType == eLogical_COP_LM_decoder)
			eRsrcEntry = eLectureDecoder;
		else if (rsrcType ==  eLogical_PCM_manager)
			eRsrcEntry = ePcmManager;
		else
			DBGPASSERT(rsrcType + 1000);

		if (eRsrcEntry > eCopConstRsrcNone  && eRsrcEntry < eLastCopConstRsrcs)
		{
			m_pCopRsrcs->constRsrcs[eRsrcEntry].connectionId = confIndicationParams.allocatedRrcs[i].connectionId;
			m_pCopRsrcs->constRsrcs[eRsrcEntry].rsrcEntityId = confIndicationParams.allocatedRrcs[i].rsrcEntityId;
			m_pCopRsrcs->constRsrcs[eRsrcEntry].logicalRsrcType = confIndicationParams.allocatedRrcs[i].logicalRsrcType;
		}

	}
	// set pcm menu id
	m_pCopRsrcs->pcmMenuId = confIndicationParams.pcmMenuId;

	// Set dynamic resources:
	for (i = 0; i < MAX_NUM_COP_DYNAMIC_RSRCS; i++)
	{
		if (confIndicationParams.dec_conn_id_list[i].logicalRsrcType == eLogical_COP_Dynamic_decoder)
		{
			m_pCopRsrcs->dynamicDecoders[i].connectionId = confIndicationParams.dec_conn_id_list[i].connectionId;
			m_pCopRsrcs->dynamicDecoders[i].logicalRsrcType = confIndicationParams.dec_conn_id_list[i].logicalRsrcType;
			m_pCopRsrcs->dynamicDecoders[i].rsrcEntityId = confIndicationParams.dec_conn_id_list[i].rsrcEntityId;
		}
		else
			DBGPASSERT(confIndicationParams.dec_conn_id_list[i].logicalRsrcType + 1000);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
eLogicalResourceTypes CConf::GetRsrcTypeForHighestCopLevel() const
{
	eLogicalResourceTypes retRsrcType;

	// Get the highest cop level params:
	CCOPConfigurationList* pCOPConfigurationList = m_pCommConf->GetCopConfigurationList();
	CCopVideoParams* pCopHighestLevelParams = pCOPConfigurationList->GetVideoMode(0);

	// set logical resource type for highest level according to highest level parameters:
	if ((pCopHighestLevelParams->GetFormat() == eCopLevelEncoderVideoFormat_HD1080p) || ((pCopHighestLevelParams->GetFormat() == eCopLevelEncoderVideoFormat_HD720p) && ((pCopHighestLevelParams->GetFrameRate() == eCopVideoFrameRate_60) || (pCopHighestLevelParams->GetFrameRate() == eCopVideoFrameRate_50))))
		retRsrcType = eLogical_COP_HD1080_encoder;
	else
		retRsrcType = eLogical_COP_HD720_encoder;

	return retRsrcType;
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::DumpCopRsrcs(CONF_RSRC_IND_PARAMS_S &confIndicationParams)
{
	std::ostringstream msg;
	msg
		<< "\n  ConfName      :" << m_name
		<< "\n  Status        :" << confIndicationParams.status
		<< "\n  ConfId        :" << confIndicationParams.rsrc_conf_id
		<< "\n  NumPredefined :" << confIndicationParams.num_predefinedRsrcs
		<< "\n  PcmMenuId     :" << confIndicationParams.pcmMenuId;

	msg << "\n  AllocatedRrcsArray:";
	for (int i = 0; i < (int)confIndicationParams.num_predefinedRsrcs; i++)
		msg << "\n  ConnId:" << confIndicationParams.allocatedRrcs[i].connectionId << ", RsrcType:" << LogicalResourceTypeToString(confIndicationParams.allocatedRrcs[i].logicalRsrcType) << ", EntityId:" << confIndicationParams.allocatedRrcs[i].rsrcEntityId;

	msg << "\n  DynamicDecodersArray:";
	for (int i = 0; i < MAX_NUM_COP_DYNAMIC_RSRCS; i++)
		msg << "\n  ConnId: " << confIndicationParams.dec_conn_id_list[i].connectionId << ", RsrcType:" << LogicalResourceTypeToString(confIndicationParams.dec_conn_id_list[i].logicalRsrcType) << ", EntityId:" << confIndicationParams.dec_conn_id_list[i].rsrcEntityId;

	TRACEINTO << msg.str().c_str();
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetConfRsrcReqForCop(CONF_RSRC_REQ_PARAMS_S &confRequestParams)
{
	//In MPM+ evnt mode the 3rd and 4th levels are CIF, in MPMx the 3rd is 4CIF and 4ty is CIF
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	switch(systemCardsBasedMode)
	{
		case eSystemCardsMode_mpm_plus:
		{
			SetConfRsrcReqForCopMPMPlus(confRequestParams);
			break;
		}
		case eSystemCardsMode_breeze:
		{
			SetConfRsrcReqForCopMPMx(confRequestParams);
			break;
		}
		default:
		{
			PASSERT(101);
			PTRACE2(eLevelInfoNormal,"CConf::SetConfRsrcReqForCop wrong system mode ",::GetSystemCardsModeStr(systemCardsBasedMode));
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetConfRsrcReqForCopMPMPlus(CONF_RSRC_REQ_PARAMS_S &confRequestParams)
{
	confRequestParams.logicalTypeList[0] = GetRsrcTypeForHighestCopLevel();
	confRequestParams.logicalTypeList[1] = eLogical_COP_HD720_encoder;
	confRequestParams.logicalTypeList[2] = eLogical_COP_CIF_encoder;
	confRequestParams.logicalTypeList[3] = eLogical_COP_CIF_encoder;
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetConfRsrcReqForCopMPMx(CONF_RSRC_REQ_PARAMS_S &confRequestParams)
{
	confRequestParams.logicalTypeList[0] = GetRsrcTypeForHighestCopLevel();
	confRequestParams.logicalTypeList[1] = eLogical_COP_HD720_encoder;
	confRequestParams.logicalTypeList[2] = eLogical_COP_4CIF_encoder;
	confRequestParams.logicalTypeList[3] = eLogical_COP_CIF_encoder;
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::HandleConfRsrcIndForCop(OPCODE opcode, CONF_RSRC_IND_PARAMS_S &confIndicationParams)
{
	if (opcode == START_CONF_RSRC_REQ)
	{
		DumpCopRsrcs(confIndicationParams);
		SetCopRsrcs(confIndicationParams);
		SetCopRsrcsInRoutingTable();
	}
	else if (opcode == TERMINATE_CONF_RSRC_REQ)
	{
		DeleteCopRsrcsFromRoutingTable();
		POBJDELETE(m_pCopRsrcs);
	}
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetCopRsrcsInRoutingTable()
{
	WORD i;

	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if (pRoutingTbl== NULL)
	{
		PASSERT_AND_RETURN(101);
	}

	//Add const resources entries to Routing Table
	for (i=0;i<eLastCopConstRsrcs;i++)
	{
		CPartyRsrcRoutingTblKey routingKey = CPartyRsrcRoutingTblKey(m_pCopRsrcs->constRsrcs[i].connectionId, m_pCopRsrcs->constRsrcs[i].rsrcEntityId, m_pCopRsrcs->constRsrcs[i].logicalRsrcType);
		pRoutingTbl->AddPartyRsrcDesc(routingKey);
	}
	//Add dynamic decoders resources entries to Routing Table
	for (i=0;i<MAX_NUM_COP_DYNAMIC_RSRCS;i++)
	{
		CPartyRsrcRoutingTblKey routingKey = CPartyRsrcRoutingTblKey(m_pCopRsrcs->dynamicDecoders[i].connectionId, m_pCopRsrcs->dynamicDecoders[i].rsrcEntityId, m_pCopRsrcs->dynamicDecoders[i].logicalRsrcType);
		pRoutingTbl->AddPartyRsrcDesc(routingKey);
	}
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::DeleteCopRsrcsFromRoutingTable()
{
	WORD i;

	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if(pRoutingTbl== NULL)
	{
		PASSERT_AND_RETURN(101);
	}

	CLargeString cstr;
	for (i=0;i<eLastCopConstRsrcs;i++)
	{
		cstr << "\n" << i << ". ConnId: " << m_pCopRsrcs->constRsrcs[i].connectionId
								<< " RsrcType: " << LogicalResourceTypeToString(m_pCopRsrcs->constRsrcs[i].logicalRsrcType)
								<< " EntityId: " << m_pCopRsrcs->constRsrcs[i].rsrcEntityId;

	}
		cstr << "\n\nDynamic decoders array:";
	for (i=0;i<MAX_NUM_COP_DYNAMIC_RSRCS;i++)
	{
		cstr << "\n" << i << ". ConnId: " << m_pCopRsrcs->dynamicDecoders[i].connectionId
	   					<< " RsrcType: " << LogicalResourceTypeToString(m_pCopRsrcs->dynamicDecoders[i].logicalRsrcType)
						<< " EntityId: " << m_pCopRsrcs->dynamicDecoders[i].rsrcEntityId;
	}

	PTRACE2(eLevelInfoNormal,"CConf::DeleteCopRsrcsFromRoutingTable ",cstr.GetString());

	//Delete const resources entries from Routing Table
	for (i=0;i<eLastCopConstRsrcs;i++)
	{
		// pcm manager partyId is being change during the conf so it may not found in the table
		// the deallocation will be from the McmsPcmManager destructor
		if (i != ePcmManager)
			pRoutingTbl->RemoveAllPartyRsrcs(m_pCopRsrcs->constRsrcs[i].rsrcEntityId);

	}

	//Delete dynamic decoders resources entries from Routing Table
	for (i=0;i<MAX_NUM_COP_DYNAMIC_RSRCS;i++)
		pRoutingTbl->RemoveAllPartyRsrcs(m_pCopRsrcs->dynamicDecoders[i].rsrcEntityId);
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void CConf::SetXcodeRsrcs(CONFERENCE_RSRC_IND_PARAMS_S &confIndicationParams)
{
	// Set const resources:
	for (XCODE_RSRC::iterator _ii = m_mapXCodeRsrc.begin(); _ii != m_mapXCodeRsrc.end() ; )
	{
		eXcodeRsrcType erase_element = _ii->first;
		if(erase_element >= MAX_CONTENT_XCODE_RSRCS)
		{
			PASSERT(erase_element);
			continue;
		}
		if (confIndicationParams.allocatedRsrcs[erase_element].allocIndBase.videoPartyType != eVideo_party_type_dummy &&
			confIndicationParams.allocatedRsrcs[erase_element].allocIndBase.status == STATUS_OK)
		{
			memcpy(&(_ii->second->allocInd),&confIndicationParams.allocatedRsrcs[erase_element], sizeof(ALLOC_PARTY_IND_PARAMS_S));
			_ii++;
		}
		else
		{
			delete _ii->second;
			_ii++;
			m_mapXCodeRsrc.erase(erase_element);
		}
	}
	DumpConfMapListOfXcodeRsrcs();
}

/////////////////////////////////////////////////////////////////////////////
void CConf::DumpXcodeRsrcs(CONFERENCE_RSRC_IND_PARAMS_S& confIndicationParams)
{
	std::ostringstream msg;
	msg << "ConfName:" << m_name;
	for (int i = 0; i < MAX_CONTENT_XCODE_RSRCS; i++)
	{
		ALLOC_PARTY_IND_PARAMS_S_BASE& allocIndBase = confIndicationParams.allocatedRsrcs[i].allocIndBase;
		if (allocIndBase.rsrc_party_id != 0)
		{
			msg << "\nALLOC_PARTY_IND_PARAMS_S [" << i << "]:"
			    << "\n  rsrc_conf_id      :" << allocIndBase.rsrc_conf_id
			    << "\n  rsrc_party_id     :" << allocIndBase.rsrc_party_id
			    << "\n  status            :" << CProcessBase::GetProcess()->GetStatusAsString(allocIndBase.status).c_str()
			    << "\n  numRsrcs          :" << allocIndBase.numRsrcs
			    << "\n  networkPartyType  :" << eNetworkPartyTypeNames[allocIndBase.networkPartyType]
			    << "\n  partyRole         :" << ePartyRoleNames[allocIndBase.partyRole]
			    << "\n  videoPartyType    :" << eVideoPartyTypeNames[allocIndBase.videoPartyType];
		}
	}
	TRACEINTO << msg.str().c_str();
}

/////////////////////////////////////////////////////////////////////////////
void CConf::DumpConfMapListOfXcodeRsrcs()
{
	std::ostringstream msg;
	msg << "ConfName:" << m_name;
    int i = 0;
	for (XCODE_RSRC::iterator _ii = m_mapXCodeRsrc.begin(); _ii != m_mapXCodeRsrc.end() ; _ii++)
	{
		i++;
		msg << "\n  mapXCodeRsrc [" << i << "]:"
				<< "\n  rsrc_conf_id      :" << _ii->second->allocInd.allocIndBase.rsrc_conf_id
				<< "\n  rsrc_party_id     :" << _ii->second->allocInd.allocIndBase.rsrc_party_id
				<< "\n  Monitor Party ID  :" << _ii->second->partyId
				<< "\n  status            :" << CProcessBase::GetProcess()->GetStatusAsString(_ii->second->allocInd.allocIndBase.status).c_str()
				<< "\n  networkPartyType  :" << eNetworkPartyTypeNames[_ii->second->allocInd.allocIndBase.networkPartyType]
				<< "\n  partyRole         :" << ePartyRoleNames[_ii->second->allocInd.allocIndBase.partyRole]
				<< "\n  videoPartyType    :" << eVideoPartyTypeNames[_ii->second->allocInd.allocIndBase.videoPartyType];
	}
	TRACEINTO << msg.str().c_str();
}

/////////////////////////////////////////////////////////////////////////////
void CConf::SetConfRsrcReqForXCode(CONFERENCE_RSRC_REQ_PARAMS_S &confRequestParams)
{
	const char*        conf_serv_name = m_pCommConf->GetServiceNameForMinParties();
	CConfIpParameters* pServiceParams = ::GetIpServiceListMngr()->GetRelevantService(conf_serv_name, H323_INTERFACE_TYPE);
	WORD               serviceId      = (pServiceParams) ? pServiceParams->GetServiceId() : 0;
	ALLOC_PARTY_PARAMS_S* pAllocXCodeRsrc = NULL;
	for (int i = 0; i < MAX_CONTENT_XCODE_RSRCS; i++)
	{
		confRequestParams.rsrc_params_list[i].monitor_conf_id          = m_pCommConf->GetMonitorConfId();
		confRequestParams.rsrc_params_list[i].networkPartyType         = eIP_network_party_type;
		confRequestParams.rsrc_params_list[i].sessionType              = eCP_ContentXcode_session;
		confRequestParams.rsrc_params_list[i].serviceId                = 0; // As default m_ServiceId in PartyCntl;
		confRequestParams.rsrc_params_list[i].optionsMask              = 0;
		confRequestParams.rsrc_params_list[i].allocationPolicy         = eAllocateAllRequestedResources;
		confRequestParams.rsrc_params_list[i].isWaitForRsrcAndAskAgain = NO;
		confRequestParams.rsrc_params_list[i].serviceId                = serviceId;
	}
	eEnterpriseMode ContRatelevel = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
	BYTE            lConfRate     = m_pCommConf->GetConfTransferRate();
	BYTE MaxContentRate = m_pUnifiedComMode->FindAMCContentRateByLevel(lConfRate, (eConfMediaType)(m_pCommConf->GetConfMediaType()),ContRatelevel, eH264Dynamic, m_pCommConf->GetCascadeOptimizeResolution());
	BYTE lContentHD1080SupportedByConfSetting = GetContentHD1080SupportedByConfSettings(MaxContentRate);
	BYTE bIsMultiContentResEnabled = m_pCommConf->GetContentMultiResolutionEnabled();
	BYTE bIsContentXCodeH64Enabled = m_pCommConf->GetContentXCodeH264Supported();
	BYTE bIsContentXCodeH63Enabled = m_pCommConf->GetContentXCodeH263Supported();
	BYTE bIsLegacyShowContent = m_pCommConf->IsLegacyShowContentAsVideo();
	BYTE bIsCascadingOptimized = m_pCommConf->GetIsCascadeOptimized();
	if (bIsContentXCodeH64Enabled && bIsMultiContentResEnabled)
	{
		confRequestParams.rsrc_params_list[eXcodeH264Encoder].monitor_party_id = m_pCommConf->NextPartyId();
		if(confRequestParams.rsrc_params_list[eXcodeH264Encoder].monitor_party_id == 0)
			confRequestParams.rsrc_params_list[eXcodeH264Encoder].monitor_party_id = m_pCommConf->NextPartyId();

		confRequestParams.rsrc_params_list[eXcodeH264Encoder].party_id = GetLookupIdParty()->Alloc();

		confRequestParams.rsrc_params_list[eXcodeH264Encoder].partyRole = eParty_Role_content_encoder;
		pAllocXCodeRsrc = new ALLOC_PARTY_PARAMS_S;
		pAllocXCodeRsrc->partyId = confRequestParams.rsrc_params_list[eXcodeH264Encoder].monitor_party_id;
		m_mapXCodeRsrc[eXcodeH264Encoder] = pAllocXCodeRsrc;
		if(lContentHD1080SupportedByConfSetting)
			confRequestParams.rsrc_params_list[eXcodeH264Encoder].videoPartyType = eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
		else
			confRequestParams.rsrc_params_list[eXcodeH264Encoder].videoPartyType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
	}
	if (bIsContentXCodeH63Enabled && bIsMultiContentResEnabled)
	{
		confRequestParams.rsrc_params_list[eXcodeH263Encoder].monitor_party_id = m_pCommConf->NextPartyId();
		confRequestParams.rsrc_params_list[eXcodeH263Encoder].party_id = GetLookupIdParty()->Alloc();
		confRequestParams.rsrc_params_list[eXcodeH263Encoder].partyRole = eParty_Role_content_encoder;
		confRequestParams.rsrc_params_list[eXcodeH263Encoder].videoPartyType   = eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type;
		pAllocXCodeRsrc = new ALLOC_PARTY_PARAMS_S;
		pAllocXCodeRsrc->partyId = confRequestParams.rsrc_params_list[eXcodeH263Encoder].monitor_party_id;
		m_mapXCodeRsrc[eXcodeH263Encoder] = pAllocXCodeRsrc;
	}
	if(bIsCascadingOptimized == TRUE)
	{
		confRequestParams.rsrc_params_list[eXcodeH264LinksEncoder].monitor_party_id = m_pCommConf->NextPartyId();
		confRequestParams.rsrc_params_list[eXcodeH264LinksEncoder].party_id = GetLookupIdParty()->Alloc();
		confRequestParams.rsrc_params_list[eXcodeH264LinksEncoder].partyRole = eParty_Role_content_encoder;
		BYTE newHD1080ContentMpi = 0;
		MaxContentRate = m_pUnifiedComMode->FindAMCContentRateByLevel(lConfRate, (eConfMediaType)(m_pCommConf->GetConfMediaType()),ContRatelevel, eH264Fix, m_pCommConf->GetCascadeOptimizeResolution());
		BYTE isHD1080WillBeSupported = SelectContentHDResolution(MaxContentRate,H264,newHD1080ContentMpi,FALSE);

		if(isHD1080WillBeSupported)
		{
			confRequestParams.rsrc_params_list[eXcodeH264LinksEncoder].videoPartyType = eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
		}
		else
		{
			confRequestParams.rsrc_params_list[eXcodeH264LinksEncoder].videoPartyType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
		}

		confRequestParams.rsrc_params_list[eXcodeH264LinksEncoder].partyRole = eParty_Role_content_encoder;
		pAllocXCodeRsrc = new ALLOC_PARTY_PARAMS_S;
		pAllocXCodeRsrc->partyId = confRequestParams.rsrc_params_list[eXcodeH264LinksEncoder].monitor_party_id;
		m_mapXCodeRsrc[eXcodeH264LinksEncoder] = pAllocXCodeRsrc;
	}

	if (bIsLegacyShowContent)
	{
		confRequestParams.rsrc_params_list[eXcodeContentDecoder].monitor_party_id = m_pCommConf->NextPartyId();
		confRequestParams.rsrc_params_list[eXcodeContentDecoder].party_id = GetLookupIdParty()->Alloc();
		confRequestParams.rsrc_params_list[eXcodeContentDecoder].partyRole = eParty_Role_content_decoder;
		if(lContentHD1080SupportedByConfSetting)
			confRequestParams.rsrc_params_list[eXcodeContentDecoder].videoPartyType = eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
		else
			confRequestParams.rsrc_params_list[eXcodeContentDecoder].videoPartyType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
		pAllocXCodeRsrc = new ALLOC_PARTY_PARAMS_S;
		pAllocXCodeRsrc->partyId = confRequestParams.rsrc_params_list[eXcodeContentDecoder].monitor_party_id;
		m_mapXCodeRsrc[eXcodeContentDecoder] = pAllocXCodeRsrc;
	}
}

/////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
void CConf::SetConfRsrcDeallocReqForXCode(CONFERENCE_RSRC_REQ_PARAMS_S& confRequestParams)
{
	memset(&confRequestParams, 0, sizeof(confRequestParams));

	if (m_mapXCodeRsrc.size() == 0)
	{
		TRACEINTO << "CConf::SetConfRsrcDeallocReqForXCode - No XCode Resources for this conference, ConfName:" << m_name;
		return;
	}

	const char*        conf_serv_name = m_pCommConf->GetServiceNameForMinParties();
	CConfIpParameters* pServiceParams = ::GetIpServiceListMngr()->GetRelevantService(conf_serv_name, H323_INTERFACE_TYPE);
	WORD               serviceId      = (pServiceParams) ? pServiceParams->GetServiceId() : 0;

	int i = 0;
	for (XCODE_RSRC::iterator _ii = m_mapXCodeRsrc.begin(); _ii != m_mapXCodeRsrc.end() ; _ii++)
	{
		confRequestParams.rsrc_params_list[i].monitor_conf_id          = m_pCommConf->GetMonitorConfId();
		confRequestParams.rsrc_params_list[i].monitor_party_id         = 0;
		confRequestParams.rsrc_params_list[i].networkPartyType         = eIP_network_party_type;
		confRequestParams.rsrc_params_list[i].sessionType              = eCP_ContentXcode_session;
		confRequestParams.rsrc_params_list[i].serviceId                = 0; // As default m_ServiceId in PartyCntl;
		confRequestParams.rsrc_params_list[i].optionsMask              = 0;
		confRequestParams.rsrc_params_list[i].allocationPolicy         = eAllocateAllRequestedResources;
		confRequestParams.rsrc_params_list[i].isWaitForRsrcAndAskAgain = NO;
		confRequestParams.rsrc_params_list[i].serviceId                = serviceId;
		confRequestParams.rsrc_params_list[i].partyRole                = _ii->second->allocInd.allocIndBase.partyRole;
		confRequestParams.rsrc_params_list[i].videoPartyType           = _ii->second->allocInd.allocIndBase.videoPartyType;
		i++;
	}
}

/////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
void CConf::HandleConfRsrcIndForXCode(OPCODE opcode, CONFERENCE_RSRC_IND_PARAMS_S &confIndicationParams)
{
	if (opcode == ALLOCATE_CONTENT_XCODE_REQ)
	{
		DumpXcodeRsrcs(confIndicationParams);
		SetXcodeRsrcs(confIndicationParams);
		SetXcodeRsrcsInRoutingTable();
	}
	else if (opcode == TERMINATE_CONF_RSRC_REQ)
	{
		DeleteXcodeRsrcsFromRoutingTable();
		for (XCODE_RSRC::iterator _ii = m_mapXCodeRsrc.begin(); _ii != m_mapXCodeRsrc.end() ; _ii++)
			delete _ii->second;
	}
}

/////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
void CConf::SetXcodeRsrcsInRoutingTable()
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(pRoutingTbl == NULL);

	// Add const resources entries to Routing Table
	for (XCODE_RSRC::iterator _ii = m_mapXCodeRsrc.begin(); _ii != m_mapXCodeRsrc.end() ; _ii++)
	{
		CPartyRsrcRoutingTblKey routingKey = CPartyRsrcRoutingTblKey(_ii->second->allocInd.allocIndBase.allocatedRrcs[0].connectionId, _ii->second->allocInd.allocIndBase.rsrc_party_id, _ii->second->allocInd.allocIndBase.allocatedRrcs[0].logicalRsrcType);
		pRoutingTbl->AddPartyRsrcDesc(routingKey);
	}
}

/////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
void CConf::DeleteXcodeRsrcsFromRoutingTable()
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(pRoutingTbl == NULL);

	std::ostringstream msg;

	for (XCODE_RSRC::iterator _ii = m_mapXCodeRsrc.begin(); _ii != m_mapXCodeRsrc.end() ; _ii++)
	{
		msg << "\n ConnId:"   << _ii->second->allocInd.allocIndBase.allocatedRrcs[0].connectionId
		    << ", RsrcType:" << LogicalResourceTypeToString(_ii->second->allocInd.allocIndBase.allocatedRrcs[0].logicalRsrcType)
		    << ", EntityId:" << _ii->second->allocInd.allocIndBase.rsrc_party_id;
	}

	TRACEINTO << msg.str().c_str();

	// Delete const resources entries from Routing Table
	for (XCODE_RSRC::iterator _ii = m_mapXCodeRsrc.begin(); _ii != m_mapXCodeRsrc.end() ; _ii++)
		pRoutingTbl->RemoveAllPartyRsrcs(_ii->second->allocInd.allocIndBase.rsrc_party_id);
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS  CConf::SyncRsrcRsrvManagerConfTerminate(OPCODE opcode)
{
	TRACEINTO << "ConfName:" << m_name;

	CRsrvManagerApi rsrvManagerApi;

	STATUS returnStatus = STATUS_OK;
	STATUS allocateStatus = STATUS_OK;

	DWORD confMonitorID = m_pCommConf->GetMonitorConfId();

	CSegment rspMsg;
	OPCODE   rspOpcode;

	CSegment* seg = new CSegment;
	*seg << confMonitorID;

	STATUS responseStatus = rsrvManagerApi.SendMessageSync(seg, opcode, CONF_RSRC_REQ_TOUT, rspOpcode, rspMsg);

	if (STATUS_OK == responseStatus)
	{
		if (rspMsg.GetLen() > 0)
        {
			rspMsg >> allocateStatus;
			if (allocateStatus != STATUS_OK) //TODO to see if we need more information on Status and GET here the resource ID !!!
			{
				TRACEINTO_GLA << "ConfState:" << ConfStateToString(m_state) << ", AllocateStatus:" << allocateStatus << " - Conference resources ALLOC/DEALLOC failed";
				PASSERT(allocateStatus);
			    m_state = IDLE;
				returnStatus = allocateStatus;
			}
			else
			{
				TRACEINTO << "Conference resources ALLOC/DEALLOC OK";
			}
		}
        else // no content in segemnt
        {
			TRACEINTO_GLA << "ConfState:" << ConfStateToString(m_state) << " - Conference resources ALLOC/DEALLOC failed, no content";
            PASSERT(1);
            m_state = IDLE;
            returnStatus = STATUS_ILLEGAL;
        }
    }
	else
	{
		TRACEINTO_GLA << "ConfState:" << ConfStateToString(m_state) << ", ResponseStatus:" << responseStatus << " - Conference resources ALLOC/DEALLOC failed, TIME OUT";
        PASSERT(responseStatus);
        m_state = IDLE;
        returnStatus = responseStatus;
	}
	return returnStatus;
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
eSessionType CConf::GetSessionTypeForResourceAllocator()
{
	eSessionType rval= esession_type_none;

	const CCommConf* pCommConf = GetCommConf();
	if(pCommConf)
	{
		if(pCommConf->GetEntryQ())
		{
			rval = eSTANDALONE_session;
		}
		else if(pCommConf->IsAudioConf())
		{
			rval = eVOICE_session;
		}
		else if(0)
		{
			rval = eVS_session;
		}
		else if(pCommConf->GetVideoSession()==VIDEO_SESSION_COP)
		{
			rval = GetSessionTypeForCop();
		}
		else if(pCommConf->GetVideoSession()==CONTINUOUS_PRESENCE)
		{
			if(pCommConf->GetContentMultiResolutionEnabled())
				rval = eCP_ContentXcode_session;
			else
				rval =  eCP_session;
		}
		else if(pCommConf->GetIsHDVSW())//HD conferences are set as VideoSwitch in CommRes
		{
			if (::GetIsCOPdongleSysMode())
			{	//VSW NxM
				rval = eVSW_Auto_session;
			}
			else
			{	//regular VSW
				rval = eCP_session;
			}
		}
		else
		{
			PASSERT(101);
		}

	}
	else
	{
		PASSERT(101);
	}

	return rval;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
eSessionType CConf::GetSessionTypeForCop() const
{
	eSessionType retSession = eCOP_HD1080_session;

	// Get the highest cop level params:
	CCOPConfigurationList* pCOPConfigurationList = m_pCommConf->GetCopConfigurationList();
	CCopVideoParams* pCopHighestLevelParams = pCOPConfigurationList->GetVideoMode(0);

	// set session type according to highest level parameters:
	if ((pCopHighestLevelParams->GetFormat() == eCopLevelEncoderVideoFormat_HD720p)
		&& ((pCopHighestLevelParams->GetFrameRate() == eCopVideoFrameRate_50)||(pCopHighestLevelParams->GetFrameRate() == eCopVideoFrameRate_60)))
		retSession = eCOP_HD720_50_session;

	return retSession;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
STATUS CConf::GetPartyTerminalNumber(const CTaskApp* pParty, WORD& mcuNumber, WORD& terminalNumber)
{
	PASSERTSTREAM_AND_RETURN_VALUE(!m_pTerminalNumberingManager, "", STATUS_FAIL);

	return m_pTerminalNumberingManager->GetPartyTerminalNumber(pParty, mcuNumber, terminalNumber);
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
BYTE CConf::SelectContentRate(bool isUpdateIpRate, eXcodeRsrcType eXCodeEncoderType)
{
#ifdef CONTENT_LOAD
	TRACEINTO << "IsUpdateIpRate:" << (int)isUpdateIpRate << ", XCodeEncoderType:" << eXcodeRsrcTypeNames[eXCodeEncoderType];
	CStopper _stopper("CConf::SelectContentRate", "Duration");
#endif

	if (IsTIPContentEnable())
	{
		BYTE newConfContentRateAMC = AMC_512k;
		DWORD actualBitRate = TranslateAMCRateIPRate(newConfContentRateAMC);
		if (isUpdateIpRate || actualBitRate < GetActualIpRateForContentSession())
		{
			TRACEINTO << "ActualBitRate:" << actualBitRate << " - Set actual IP rate for content session";
			SetActualIpRateForContentSession(actualBitRate);
		}
		return newConfContentRateAMC;
	}

	bool isXCodeConference = m_pCommConf->GetContentMultiResolutionEnabled() ? true : false;

	if (isXCodeConference && (eXCodeEncoderType == eXcodeH263Encoder) && !m_pCommConf->GetContentXCodeH263Supported())
	{
		PASSERT(1);
		return 0;
	}

	ePresentationProtocol presentationProtocol = (ePresentationProtocol)m_pCommConf->GetPresentationProtocol();
	eEnterpriseMode enterpriseMode = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
	BYTE confTransferRate = m_pCommConf->GetConfTransferRate();
	BYTE maxContentRate = AMC_0k;

	if (isXCodeConference && eXCodeEncoderType != eXcodeEncoderDummy)
	{
		switch (eXCodeEncoderType)
		{
			case eXcodeH264LinksEncoder:
			{
				maxContentRate = m_pUnifiedComMode->FindAMCContentRateByLevel(confTransferRate, m_pCommConf->GetConfMediaType(), enterpriseMode, eH264Fix, m_pCommConf->GetCascadeOptimizeResolution());
				TRACEINTO << "MaxContentRate:" << CContentBridge::GetContentRateAsString(maxContentRate) << ", MaxResolution:" << m_pCommConf->GetCascadeOptimizeResolution() << ", XCodeEncoderType:" << eXcodeRsrcTypeNames[eXCodeEncoderType];
				return maxContentRate;
			}
			case eXcodeH264Encoder:
			{
				maxContentRate = m_pUnifiedComMode->FindAMCContentRateByLevel(confTransferRate, m_pCommConf->GetConfMediaType(), enterpriseMode, eH264Dynamic);
				break;
			}
			case eXcodeH263Encoder:
			{
				maxContentRate = m_pUnifiedComMode->FindAMCContentRateByLevel(confTransferRate, m_pCommConf->GetConfMediaType(), enterpriseMode, eH263Fix);
				break;
			}
			default:
			{
				PASSERT(eXCodeEncoderType + 1);
			}
		}
	}
	else
	{
		maxContentRate = m_pUnifiedComMode->FindAMCContentRateByLevel(confTransferRate, m_pCommConf->GetConfMediaType(), enterpriseMode, presentationProtocol, m_pCommConf->GetCascadeOptimizeResolution());
	}

	if (!m_pCommConf->GetContentMultiResolutionEnabled())
	{
		if (m_pContentBridge && (!strcmp(m_pContentBridge->NameOf(), "CContentBridgeSlave") || !strcmp(m_pContentBridge->NameOf(), "CContentBridgeMaster")))
		{
			DWORD actualBitRate = TranslateAMCRateIPRate(maxContentRate);
			if (presentationProtocol != eH264Fix && presentationProtocol != eH264Dynamic)// VNGFE-8024 - use BCH factor only when cascade isn't H264 protocol (fix or 264 dynamic)
				actualBitRate = actualBitRate * BCH_FACTOR / 100;
			if (m_lastContentRateFromMaster)
				actualBitRate = min(actualBitRate, m_lastContentRateFromMaster);

			if (isUpdateIpRate || actualBitRate < GetActualIpRateForContentSession())
				SetActualIpRateForContentSession(actualBitRate);

			maxContentRate = CUnifiedComMode::TranslateRateToAMCRate(actualBitRate);
			TRACEINTO << "MaxContentRate:" << CContentBridge::GetContentRateAsString(maxContentRate) << ", MaxResolution:" << m_pCommConf->GetCascadeOptimizeResolution() << ", PresentationProtocol:" << presentationProtocol;
			return maxContentRate;
		}
	}

	if (presentationProtocol == eH264Fix)
	{
		TRACEINTO << "MaxContentRate:" << CContentBridge::GetContentRateAsString(maxContentRate) << ", Protocol:eH264Fix";
		DWORD actualBitRate = TranslateAMCRateIPRate(maxContentRate);
		if (isUpdateIpRate || actualBitRate < GetActualIpRateForContentSession())
		{
			TRACEINTO << "ActualBitRate:" << actualBitRate << " - Set actual IP rate for H264Fix content session";
			SetActualIpRateForContentSession(actualBitRate);
		}
		return maxContentRate;
	}

	//Not Cascade
	DWORD newConfContentRateAMC = AMC_0k;
	DWORD allowedConfContentRateAMC = AMC_0k;
	DWORD partyContentRate = 0;
	DWORD minPartyContentRate = 4096 * 1000; //set to the maximum value
	BYTE partyContentRateAMC = AMC_0k;
	BYTE needToRemoveBCH = FALSE;
	// we taking care about rate of all parties

#ifdef CONTENT_LOAD
	_stopper.AddTime();
#endif

	// search for weakest party rate
	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (!pPartyConnection)
			continue;

		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		if (!pPartyCntl)
			continue;

		if (strcmp(pPartyCntl->NameOf(), "CDelIsdnPartyCntl") && strcmp(pPartyCntl->NameOf(), "CSipDelPartyCntl") && strcmp(pPartyCntl->NameOf(), "CH323DelPartyCntl"))
		{
			// if party is secondary or connected with problems
			CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
			if (!pConfParty)
				continue;

			if (!pConfParty->GetIsLyncPlugin()) //non plugin
			{
				DWORD partyState = pConfParty->GetPartyState();    //FSN-613: Dynamic Content for SVC/Mix Conf
				if (!(partyState == PARTY_CONNECTED || partyState == PARTY_CONNECTED_PARTIALY || partyState == PARTY_CONNECTED_WITH_PROBLEM || (partyState == PARTY_CONNECTING && pPartyCntl->IsRemoteAndLocalCapSetHasContent())))
					continue;
			}

			// skip parties without H239 cap
			if (!pPartyCntl->IsRemoteAndLocalCapSetHasContent(eToPrintOnFalseOnly))
				continue;

			if (pPartyCntl->IsLegacyContentParty())
				continue;

			if (pPartyCntl->IsLync())
				continue;

			if (isXCodeConference)
			{
				if (eXCodeEncoderType == eXcodeH264Encoder && !pPartyCntl->IsRemoteAndLocalHasHDContent720() && !pPartyCntl->IsRemoteAndLocalHasHDContent1080())
					continue;

				if (eXCodeEncoderType == eXcodeH263Encoder && (pPartyCntl->IsRemoteAndLocalHasHDContent720() || pPartyCntl->IsRemoteAndLocalHasHDContent1080()))
					continue;
			}

			if (pPartyCntl->GetInterfaceType() != ISDN_INTERFACE_TYPE)
			{
				// get the minimum content rate between the party allowed content rate
				// resulting from call rate and the remote caps content rate
				partyContentRate = ((CIpPartyCntl*)pPartyCntl)->GetMinContentPartyRate();

				if (pConfParty->GetIsTransparentGw() == YES)
				{
					PTRACE(eLevelInfoNormal, "CConf::SelectContentRate :  Transparent GW link need to remove BCH ");
					needToRemoveBCH = TRUE;
				}
			}
			else
			{
				WORD reservationRate = ((CIsdnPartyCntl*)pPartyCntl)->GetTargetTransmitScm()->GetXferMode();
				CCapSetInfo lCapInfo = eUnknownAlgorithemCapCode;
				partyContentRate = (lCapInfo.TranslateReservationRateToIpRate(reservationRate)) * 1000;

				PTRACE2INT(eLevelInfoNormal, "****CConf::SelectContentRate :  ISDN Party content rate - ", partyContentRate);
				needToRemoveBCH = TRUE;
				BYTE PartyXferConfRate = TranslateConfIPRateToXferRate(partyContentRate);
				allowedConfContentRateAMC = GetAllowedContentRateAMC(PartyXferConfRate); //Get the allowed content rate according the conf table for the weakes
				partyContentRate = (TranslateAMCRateIPRate(allowedConfContentRateAMC)) * 100;

				PTRACE2INT(eLevelInfoNormal, "****CConf::SelectContentRate :  ISDN Party selected content rate - ", partyContentRate);
			}

			partyContentRateAMC = CUnifiedComMode::TranslateRateToAMCRate(partyContentRate / 100);

			// if party doesn't support content rate according to it line rate
			if (AMC_0k == partyContentRateAMC)
			{
				PTRACE2(eLevelError, "CConf::SelectContentRate, ZERO Content rate for Party - ", pPartyCntl->GetFullName());
				continue; //break its minimum
			}

			minPartyContentRate = min(partyContentRate, minPartyContentRate);
		}
	} //Go over all parties to find the min call rate of parties

#ifdef CONTENT_LOAD
	_stopper.AddTime();
#endif

	BYTE minPartyContentRateAMC = CUnifiedComMode::TranslateRateToAMCRate(minPartyContentRate / 100);

	newConfContentRateAMC = minPartyContentRateAMC;

	if (newConfContentRateAMC > maxContentRate) //the limit is the conf allowed content rate we can't get any higher then that.
		newConfContentRateAMC = maxContentRate;

#ifdef CONTENT_LOAD
	TRACEINTO << "NewConfContentRate:" << CContentBridge::GetContentRateAsString(newConfContentRateAMC);
#else
	TRACEINTO << "IsUpdateIpRate:" << (int)isUpdateIpRate << ", XCodeEncoderType:" << eXcodeRsrcTypeNames[eXCodeEncoderType] << ", SELECTED CONTENT RATE:" << CContentBridge::GetContentRateAsString(newConfContentRateAMC);
#endif

	DWORD actualBitRate = TranslateAMCRateIPRate(newConfContentRateAMC);

	if (needToRemoveBCH == TRUE)
	{
		actualBitRate = actualBitRate * BCH_FACTOR / 100;
	}

	if (isUpdateIpRate != FALSE || actualBitRate < GetActualIpRateForContentSession())
	{
		PTRACE2INT(eLevelInfoNormal, "CConf::SelectContentRate, Set Actual Ip Rate For Content Session : ", actualBitRate);
		SetActualIpRateForContentSession(actualBitRate);
	}
#ifdef CONTENT_LOAD
	_stopper.Stop();
#endif

	return newConfContentRateAMC;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
BYTE  CConf::DoesPartyMeetConfContentHDResolution(char* partyName)
{
	PASSERT_AND_RETURN_VALUE(!partyName, FALSE);

	TRACEINTO << "PartyName:" << partyName;

   BYTE doesPartyMeetConfContentHDResolution = TRUE;
   ePresentationProtocol contentProtocolMode = (ePresentationProtocol)m_pCommConf->GetPresentationProtocol();
   DWORD partyContentRate = 0;

	if (contentProtocolMode != eH264Fix)
    {
    	 return  doesPartyMeetConfContentHDResolution;
    }

    CConfParty*        pConfParty       = NULL;
    CPartyCntl*		   pPartyCntl		= NULL;

    CPartyConnection* pPartyConnection = GetPartyConnection(partyName);
	if (!pPartyConnection)
    {
    	TRACEINTO << "PartyName:" << partyName << " - Failed, Party does not exist";
    	doesPartyMeetConfContentHDResolution = FALSE;
    	return 	doesPartyMeetConfContentHDResolution;
    }

    pPartyCntl = pPartyConnection->GetPartyCntl();

	if (!CPObject::IsValidPObjectPtr(pPartyCntl))
 	{
 	    doesPartyMeetConfContentHDResolution = FALSE;
 	    return 	doesPartyMeetConfContentHDResolution;
 	}

	if (!strcmp(pPartyCntl->NameOf(), "CDelIsdnPartyCntl") || !strcmp(pPartyCntl->NameOf(), "CSipDelPartyCntl") || !strcmp(pPartyCntl->NameOf(), "CH323DelPartyCntl"))
    {
     	    doesPartyMeetConfContentHDResolution = FALSE;
     	    return 	doesPartyMeetConfContentHDResolution;
    }

    partyContentRate = ((CIpPartyCntl*)pPartyCntl)->GetMinContentPartyRate();

    BYTE remoteHD720Mpi = 0;
    BYTE remoteHD1080Mpi = 0;

    remoteHD1080Mpi = pPartyCntl->IsRemoteAndLocalHasHDContent1080();

	//BRIDGE-15488, if remote have 1080p capability, the HD720 Mpi will be 2 (720p30)
	if (remoteHD1080Mpi)
		remoteHD720Mpi = 2;
	else
    	remoteHD720Mpi = pPartyCntl->IsRemoteAndLocalHasHDContent720();

	if (!remoteHD1080Mpi && !remoteHD720Mpi)
    {
        doesPartyMeetConfContentHDResolution = FALSE;
		PTRACE2(eLevelInfoNormal, "CConf::DoesPartyMeetConfContentHDResolution, Party does not Support Content HD Resolution, Party Name: ", pPartyCntl->GetFullName());
    	return 	doesPartyMeetConfContentHDResolution;
    }

   // Get Party Conetent HD data
   BYTE bPartySupportsContentHD1080 = TRUE;
   BYTE HDPartyMpi = 0;
	BYTE partyContentRateAMC = m_pUnifiedComMode->TranslateRateToAMCRate(partyContentRate / 100);
	bPartySupportsContentHD1080 = SelectContentHDResolution(partyContentRateAMC, H264, HDPartyMpi, FALSE);

   // Get Conf Conetent HD data
   BYTE bConfSupportsContentHD1080 = TRUE;
   BYTE HDConfMpi = 0;
   BYTE confContentRateAMC  = SelectContentRate();
	bConfSupportsContentHD1080 = SelectContentHDResolution(confContentRateAMC, H264, HDConfMpi, FALSE);

	if (!remoteHD1080Mpi || !bPartySupportsContentHD1080) //BRIDGE-10976
   {
       OFF(bPartySupportsContentHD1080);
		HDPartyMpi = max(remoteHD720Mpi, HDPartyMpi);
   }
   else
		HDPartyMpi = max(remoteHD1080Mpi, HDPartyMpi);

	if (bConfSupportsContentHD1080 != bPartySupportsContentHD1080 || HDConfMpi < HDPartyMpi)
   {
	   doesPartyMeetConfContentHDResolution = FALSE;
		std::ostringstream msg;
		msg
			<< "\n  PartyId                    :" << pPartyCntl->GetPartyId()
			<< "\n  PartySupportsHDContent1080 :" << (int)bPartySupportsContentHD1080
			<< "\n  HD Party MPI               :" << (int)HDPartyMpi
			<< "\n  ConfSupportsHDContent1080  :" << (int)bConfSupportsContentHD1080
			<< "\n  HD Conf MPI                :" << (int)HDConfMpi;
			TRACEINTO << msg.str().c_str();
   }
   return doesPartyMeetConfContentHDResolution;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
bool CConf::SelectContentHDResolution(BYTE newContentRateAMC, BYTE newContentProtocol, BYTE& HDMpi, BOOL isHighestCommonRequired, eXcodeRsrcType eXCodeEncoderType)
{
#ifdef CONTENT_LOAD
	CStopper _stopper("CConf::SelectContentHDResolution", "Duration");
#endif

	TRACEINTO << "ContentRate:" << (int)newContentRateAMC << ", ContentProtocol:" << (int)newContentProtocol << ", IsHighestCommonRequired:" << (int)isHighestCommonRequired << ", XCodeEncoderType:" << eXcodeRsrcTypeNames[eXCodeEncoderType];

	// Bridge-11896
	if (IsTIPContentEnable())
	{
		PTRACE(eLevelInfoNormal, "CConf::SelectContentHDResolution : TIP content mode is active, only support XGA");
		return false;
	}

	bool isPartySupportsContentHD1080 = true;
	bool isPartySupportsContentH720   = true;

	// The MPI concept is changed for supporting 60fps:
	// ->  MPI 1 = 60fps; MPI 2 = 30 fps; MPI 4 = 15fps; MPI 10 = 5fps
	BYTE higestCommon1080HDMpi = 0;
	BYTE higestCommon720HDMpi = 0;
	BYTE HD1080PartyMpi = 0;
	BYTE HD720PartyMpi = 0;
	WORD numOfActiveContentParties = 0;

	BYTE confTransferRate = m_pCommConf->GetConfTransferRate();

	ePresentationProtocol presentationProtocol = (ePresentationProtocol)m_pCommConf->GetPresentationProtocol();
	eEnterpriseMode enterpriseMode = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();

	if (presentationProtocol == eH264Fix || presentationProtocol == eH264Dynamic)
		newContentProtocol = H264;

	if (presentationProtocol == eH264Fix || (m_pCommConf->GetContentMultiResolutionEnabled() && eXCodeEncoderType == eXcodeH264LinksEncoder))
	{
		eCascadeOptimizeResolutionEnum eMaxResolution = m_pCommConf->GetCascadeOptimizeResolution();

		TRACEINTO << "MaxResolution:" << eMaxResolution;

		switch (eMaxResolution)
		{
			case e_res_720_5fps  : HDMpi = 10; return false;
			case e_res_720_30fps : HDMpi =  2; return false;
			case e_res_1080_15fps: HDMpi =  4; return true;
			case e_res_1080_30fps: HDMpi =  2; return true;
			case e_res_1080_60fps: HDMpi =  1; return true;
			default              : HDMpi = 10; return false;
		}
	}

	if (newContentProtocol != H264)
	{
		PTRACE(eLevelInfoNormal, "CConf::SelectContentHDResolution, ContentProtocol is not H264, return!!!! : ");
		return isPartySupportsContentHD1080;
	}

	bool isXCodeConference = m_pCommConf->GetContentMultiResolutionEnabled() ? true : false;

	TRACEINTO << "IsXCodeConference:" << (int)isXCodeConference;

	if (isHighestCommonRequired)
	{
		// search for weakest party rate

		PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
		for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
		{
			CPartyConnection* pPartyConnection = _itr->second;
			if (!pPartyConnection)
				continue;

			CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
			if (!pPartyCntl)
				continue;

			if (strcmp(pPartyCntl->NameOf(), "CDelIsdnPartyCntl") && strcmp(pPartyCntl->NameOf(), "CSipDelPartyCntl") && strcmp(pPartyCntl->NameOf(), "CH323DelPartyCntl"))
			{
				if (pPartyCntl->IsLegacyContentParty())
					continue;

				if (pPartyCntl->IsLync())
					continue;

				// if party is secondary or connected with problems
				CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
				if (!pConfParty)
					continue;

				DWORD partyState = pConfParty->GetPartyState();
				if (!(partyState == PARTY_CONNECTED || partyState == PARTY_CONNECTED_PARTIALY || partyState == PARTY_CONNECTED_WITH_PROBLEM || (partyState == PARTY_CONNECTING && pPartyCntl->IsRemoteAndLocalCapSetHasContent()))) //Anna - tandberg EP sends content in the first invite
					continue;

				if (isXCodeConference)
				{
					if (pPartyCntl->GetInterfaceType() == ISDN_INTERFACE_TYPE)
						continue;

					if (eXCodeEncoderType == eXcodeH264Encoder && pConfParty->GetCascadeMode())
						continue;

					if (!pPartyCntl->IsRemoteAndLocalHasHDContent720() && !pPartyCntl->IsRemoteAndLocalHasHDContent1080())
						continue;
				}

				numOfActiveContentParties++;
				if (pPartyCntl->GetInterfaceType() != ISDN_INTERFACE_TYPE)
				{
					HD720PartyMpi = pPartyCntl->IsRemoteAndLocalHasHDContent720();
					HD1080PartyMpi = pPartyCntl->IsRemoteAndLocalHasHDContent1080();

					if (HD1080PartyMpi == 0)
					{
						isPartySupportsContentHD1080 = false;
					}
					if (HD720PartyMpi == 0)
					{
						if (isPartySupportsContentHD1080)
						{
							HD720PartyMpi = 2;
						}
						else
						{
							isPartySupportsContentH720 = false;
							break;
						}
					}
				}
				else
				{
					isPartySupportsContentHD1080 = false;
					isPartySupportsContentH720   = false;
					continue;
				}
				if (HD1080PartyMpi)
				{
					if (HD1080PartyMpi > higestCommon1080HDMpi)
					{
						higestCommon1080HDMpi = HD1080PartyMpi;
					}
				}
				if (HD720PartyMpi)
				{
					if (HD720PartyMpi > higestCommon720HDMpi)
					{
						higestCommon720HDMpi = HD720PartyMpi;
					}
				}
			}
		}
	} //Go over all parties to find the min call rate of parties

	eCascadeOptimizeResolutionEnum eMaxResolution = m_pUnifiedComMode->getMaxContentResolutionbyAMCRate(newContentRateAMC);
	//HP Content:
	TRACEINTO << "MaxResolution:" << eMaxResolution;

	if (eMaxResolution < e_res_1080_15fps)
		isPartySupportsContentHD1080 = false;

	if (isPartySupportsContentHD1080)
	{
		switch (eMaxResolution)
		{
			case e_res_1080_60fps:
				HDMpi = 1;
				break;
			case e_res_1080_30fps:
				HDMpi = 2;
				break;
			default:
				HDMpi = 4;
				break;
		}
		if (isHighestCommonRequired && numOfActiveContentParties)
		{
			if (HDMpi < higestCommon1080HDMpi || higestCommon1080HDMpi == 0)
				HDMpi = higestCommon1080HDMpi;
		}
		if (HDMpi < 4) // 1080p30/60
		{
			// only supported by non-XCode conference and RMX with MPM-Rx
			if (isXCodeConference || !IsFeatureSupportedBySystem(eFeatureHD1080p30Content))
			{
				HDMpi = 4;
			}
			else if (HDMpi == 1 && !IsFeatureSupportedBySystem(eFeatureHD1080p60Content)) // 1080p60
			{
				HDMpi = 2;
			}
		}
		TRACEINTO << "IsPartySupportsContentHD1080:" << (int)isPartySupportsContentHD1080 << ", MPI:" << (int)HDMpi << " (Romem)";
	}
	else
	{
		if (isPartySupportsContentH720)
		{
			switch (eMaxResolution)
			{
				//HP content: modify previous bug
				case e_res_720_30fps:
				case e_res_1080_15fps:
				case e_res_1080_30fps:
				case e_res_1080_60fps:
					HDMpi = 2;
					break;
				default:
					HDMpi = 10;
			}
			if (isHighestCommonRequired && numOfActiveContentParties)
			{
				if (HDMpi < higestCommon720HDMpi || (higestCommon720HDMpi == 0))
					HDMpi = higestCommon720HDMpi;
			}
			TRACEINTO << "IsPartySupportsContentHD1080:" << (int)isPartySupportsContentHD1080 << ", MPI:" << (int)HDMpi << " (Romem)";
		}
		else
		{
			PTRACE2(eLevelInfoNormal, "CConf::SelectContentHDResolution : HD Content 720/1080 not supported in this Conf, Conf name: ", m_name);
		}
	}
#ifdef CONTENT_LOAD
	_stopper.Stop();
#endif

	return isPartySupportsContentHD1080;
}

/////////////////////////////////////////////////////////////////////////////
//HP content:
BYTE CConf::SelectContentH264HighProfile()
{
	BYTE isXCodeConf = m_pCommConf->GetContentMultiResolutionEnabled();
	if (isXCodeConf)
		return FALSE;

	ePresentationProtocol contentProtocolMode = (ePresentationProtocol)m_pCommConf->GetPresentationProtocol();
	if (contentProtocolMode == eH264Fix)
		return m_pCommConf->GetIsHighProfileContent();

	if (contentProtocolMode != eH263Fix)
	{
		BYTE selectedContentProtocol = SelectContentProtocol(FALSE);
		if (selectedContentProtocol == H264 && m_pCommConf->GetIsHighProfileContent())
		{
			PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
			for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
			{
				CPartyConnection* pPartyConnection = _itr->second;
				if (!pPartyConnection)
					continue;

				CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
				if (!pPartyCntl)
					continue;

				if (strcmp(pPartyCntl->NameOf(), "CDelIsdnPartyCntl") && strcmp(pPartyCntl->NameOf(), "CSipDelPartyCntl") && strcmp(pPartyCntl->NameOf(), "CH323DelPartyCntl"))
				{
					// if party is secondary or connected with problems
					CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
					if (!pConfParty || !(pConfParty->GetPartyState() == PARTY_CONNECTED || pConfParty->GetPartyState() == PARTY_CONNECTED_PARTIALY || pConfParty->GetPartyState() == PARTY_CONNECTED_WITH_PROBLEM || (pConfParty->GetPartyState() == PARTY_CONNECTING && pPartyCntl->IsRemoteAndLocalCapSetHasContent()))) //Anna - tandberg EP sends content in the first invite
						continue;

					// skip parties without H239 cap
					if (pPartyCntl->IsLegacyContentParty() || (pPartyCntl->IsLync()))
					{
						PTRACE(eLevelInfoNormal, "CConf::SelectContentH264HighProfile, legacy only or Lync: ");
						continue;
					}

					if (pPartyCntl->GetInterfaceType() == ISDN_INTERFACE_TYPE)
						continue;

					if (!pPartyCntl->IsRemoteAndLocalHasHighProfileContent())
					{
						PTRACE(eLevelInfoNormal, "CConf::SelectContentH264HighProfile, find the party with Baseline content profile");
						return FALSE;
					}
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
BYTE CConf::GetContentHD1080SupportedByConfSettings(BYTE AMCContentRate)
{
	BYTE ContentHD1080ResolutionSupported = 0;
	BYTE possibleConfAMCContentRate = 0;
	eCascadeOptimizeResolutionEnum possibleContentResolution = e_res_dummy;

	if (AMCContentRate)
	{
		possibleConfAMCContentRate = AMCContentRate;
	}
	else
	{
		BYTE XferConfRate = m_pCommConf->GetConfTransferRate();
		eEnterpriseMode ContentRatelevel = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
		possibleConfAMCContentRate = m_pUnifiedComMode->FindAMCContentRateByLevel(XferConfRate, (eConfMediaType)(m_pCommConf->GetConfMediaType()), ContentRatelevel, (ePresentationProtocol)m_pCommConf->GetPresentationProtocol(), m_pCommConf->GetCascadeOptimizeResolution());
	}

	possibleContentResolution= m_pUnifiedComMode->getMaxContentResolutionbyAMCRate(possibleConfAMCContentRate);

	if (possibleContentResolution >= e_res_1080_15fps)
	{
		ContentHD1080ResolutionSupported = (BYTE)possibleContentResolution;
	}

	TRACEINTO << "PossibleConfAMCContentRate:" << (int)possibleConfAMCContentRate << ", ContentHD1080SupportedByConfSettings:" << (int)ContentHD1080ResolutionSupported << " Conf Name: " << m_name << "\n";

	return ContentHD1080ResolutionSupported;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
BYTE CConf::FindMinPartiesContentProtocol(BYTE bTakeCurrent)
{
	BYTE MinProtocol = H264;

	EHDResolution eHDRes;
	BYTE newContentRateAMC = SelectContentRate();
	BYTE HDMpi = 0;
	BOOL isHDContent1080Supported = SelectContentHDResolution(newContentRateAMC, H264, HDMpi);

	if (HDMpi == 0)
	{
		TRACEINTO << "HDMpi:0, Protocol:H263";
		return H263;
	}

	BOOL bStrictPolicyForH239HighestCommon = GetSystemCfgFlagInt<BOOL>(CFG_KEY_H239_FORCE_CAPABILITIES);
	//Create scm with H264 content caps

	//Only in case system work in HD1080 mode and force flag is on - we will compare remote caps to HD1080
	//If remote caps doesn't support HD1080 - will downgrade to H263.
	BYTE bContentAsVideo = m_pCommConf->IsLegacyShowContentAsVideo();
	DWORD contentRate = m_pUnifiedComMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	BOOL bHighProfileContent = GetCurrentContentIsHighProfile();   //HP content

	//if(bStrictPolicyForH239HighestCommon && isHDContent1080Supported)
	if (isHDContent1080Supported)
		eHDRes = eHD1080Res;

	//If system work in HD1080 mode but force is off - we will compare remote caps to HD720 -
	// if not supported will downgrade to H263
	//if system work in HD720 mode - we'll compare remote cap to HD720.
	else
		eHDRes = eHD720Res;

	CIpComMode* pH264ContentScm = new CIpComMode;

	pH264ContentScm->RemoveContent(cmCapReceiveAndTransmit);
	pH264ContentScm->SetIsShowContentAsVideo(bContentAsVideo);
	pH264ContentScm->SetHDContent(contentRate, cmCapReceiveAndTransmit, eHDRes, HDMpi, bHighProfileContent);
	pH264ContentScm->Dump("CConf::FindMinPartiesContentProtocol - IP CONTENT SCM", eLevelInfoNormal);
	const CMediaModeH323& rH264ContentMode = pH264ContentScm->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	std::ostringstream msg;
	msg.precision(0);
	msg << "CConf::FindMinPartiesContentProtocol" << endl;
	std::ostringstream msgH264;
	msgH264.precision(0);
	bool needtoTrace = false;

	// search for weakest party protocol
	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (!pPartyConnection)
			continue;

		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		if (!pPartyCntl)
			continue;

		if (strcmp(pPartyCntl->NameOf(), "CDelIsdnPartyCntl") && strcmp(pPartyCntl->NameOf(), "CSipDelPartyCntl") && strcmp(pPartyCntl->NameOf(), "CH323DelPartyCntl"))
		{
			// if party is secondary or connected with problems
			CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
			if (!pConfParty || !(pConfParty->GetPartyState() == PARTY_CONNECTED || pConfParty->GetPartyState() == PARTY_CONNECTED_PARTIALY || pConfParty->GetPartyState() == PARTY_CONNECTED_WITH_PROBLEM))
				continue;

			// skip parties without H239 cap
			if (!pPartyCntl->IsRemoteAndLocalCapSetHasContent(eToPrintOnFalseOnly))
				continue;
			needtoTrace = true;
			BYTE partyH239Protocol = H264;
			BYTE IsContainH264Scm = TRUE;

			//H323 and SIP party
			if (pPartyCntl->GetInterfaceType() != ISDN_INTERFACE_TYPE)
			{	//Check if presentation channel out is open
				if (((CIpPartyCntl*)pPartyCntl)->IsOpenContentTxChannel(bTakeCurrent))
				{
					if (((CIpPartyCntl*)pPartyCntl)->IsRemoteAndLocalHasEPCContentOnly())
					{
						msg << "	IsRemoteAndLocalHasEPCContentOnly selecting H263" << endl;
						partyH239Protocol = H263;
					}
					else if (pPartyCntl->GetPartyCascadeType() != CASCADE_NONE)
					{
						msg << "	GetPartyCascadeType()" << (DWORD)pPartyCntl->GetPartyCascadeType() << " != CASCADE_NONE selecting H263" << endl;
						partyH239Protocol = H263;
					}
					else
					{
						IsContainH264Scm = ((CIpPartyCntl*)pPartyCntl)->IsPartyCapsContainsH264SCM(&rH264ContentMode, kRoleContentOrPresentation);
						msg << "  	IsContainH264Scm For Party: " << pPartyCntl->GetFullName() << " IsContainH264Scm= " << (DWORD)IsContainH264Scm << endl;
					}
				}
			}
			//ISDN party
			else
			{
				partyH239Protocol = H263;
				msg << "		ISDN Party: " << pPartyCntl->GetFullName() << endl;
			}

			if (partyH239Protocol == H263)
			{
				msg << "	Choosing H263, Party: " << pPartyCntl->GetFullName() << endl;
				MinProtocol = partyH239Protocol;
				break;
			}
			else if (partyH239Protocol == H264)
			{
				msg << "	partyH239Protocol == H264" << endl;
				//Check if rmt caps contain the current H264 mode
				//if not will downgrade to 263.
				if (!IsContainH264Scm)
				{
					msg << "	!IsContainH264Scm, Party: " << pPartyCntl->GetFullName() << endl;
					MinProtocol = H263;
					break;
				}
			}
			msgH264 << "  	H264,party " << pPartyCntl->GetFullName();
		}
	}
	if (needtoTrace)
	{
		msgH264 << endl;
		msg << msgH264.str().c_str();
		PTRACE(eLevelInfoNormal, msg.str().c_str());
	}
	if (CPObject::IsValidPObjectPtr(m_pContentBridge))
		PTRACE2(eLevelInfoNormal, "CConf::FindMinPartiesContentProtocol, Minimum Parties content protocol : ", m_pContentBridge->GetContentProtocolAsString(MinProtocol));

	POBJDELETE(pH264ContentScm);
	return MinProtocol;
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
BYTE CConf::SelectContentProtocol(BYTE bTakeCurrent)
{
	if ( !IsEnableH239() )
		return Video_Off;

	if( IsTIPContentEnable() )
	{
		PTRACE(eLevelInfoNormal, "CConf::SelectContentProtocol : TIP content mode is active, Chosen Protocol=H264");
		return H264;
	}
	if(m_pCommConf->GetContentMultiResolutionEnabled())
	{
		PTRACE(eLevelInfoNormal, "CConf::SelectContentProtocol : XCode Conference, Chosen Protocol=H264");
		return H264;
	}

	//Conference setting - H263Fix / AUTO
	ePresentationProtocol ContSettingProtocol = (ePresentationProtocol)m_pCommConf->GetPresentationProtocol();

	if(ContSettingProtocol == eH264Fix  || ContSettingProtocol == eH264Dynamic)
		return H264;

	//in cascade the content protocol is 263 always
	if (CPObject::IsValidPObjectPtr(m_pContentBridge) &&
			(!strcmp(m_pContentBridge->NameOf(), "CContentBridgeSlave") ||
					!strcmp(m_pContentBridge->NameOf(), "CContentBridgeMaster")))
		return H263;


	//Find the Conf content protocol (H263/H264)
	BYTE ConfH239CurrentProtocol = GetCurrentContentProtocolInConfValues();

	BYTE ChosenProtocol = H264;

	if(ContSettingProtocol == eH263Fix)
		ChosenProtocol = H263;
	BYTE MinPartiesProtocol = FindMinPartiesContentProtocol(bTakeCurrent);

	if(MinPartiesProtocol == H263 || ContSettingProtocol == eH263Fix) //ANNA
		ChosenProtocol = H263;
	else
		ChosenProtocol = H264;

	if (CPObject::IsValidPObjectPtr(m_pContentBridge))
		PTRACE2(eLevelError,"CConf::SelectContentProtocol, SELECTED CONTENT PROTOCOL : ",m_pContentBridge->GetContentProtocolAsString(ChosenProtocol));

	return ChosenProtocol;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void CConf::SetContentMode(BYTE Xfer_XCallRate,eEnterpriseMode ContRatelevel,ePresentationProtocol ContProtocol, eCascadeOptimizeResolutionEnum resolutionLevel, cmCapDirection eDirection,BYTE bContentAsVideo, BOOL isContentHD1080, BYTE HDMpi)
{
	if( !CPObject::IsValidPObjectPtr(m_pUnifiedComMode) )
		return;
	if( IsTIPContentEnable() )
	{
		if ( m_pCommConf->GetIsPreferTIP() )
			m_pUnifiedComMode->SetContentTIPMode( ContRatelevel, eDirection, FALSE );
		else //eTipCompatibleVideoAndContent
		{
			m_pUnifiedComMode->SetContentTIPMode( ContRatelevel, eDirection);
		}
	}
	else
	{
		m_pUnifiedComMode->SetContentMode(Xfer_XCallRate,ContRatelevel,eDirection, ContProtocol, resolutionLevel, bContentAsVideo,
	        (eConfMediaType)(m_pCommConf->GetConfMediaType()), isContentHD1080, HDMpi);
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
BYTE CConf::GetAllowedContentRateAMC(BYTE lConfRate)
{
	BYTE MaxContentRate = 0;
	eEnterpriseMode ContRatelevel = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
	if( CPObject::IsValidPObjectPtr(m_pUnifiedComMode) )
		MaxContentRate = m_pUnifiedComMode->GetContentModeAMC(lConfRate,ContRatelevel,
				(ePresentationProtocol)m_pCommConf->GetPresentationProtocol(),
				m_pCommConf->GetCascadeOptimizeResolution(), m_pCommConf->GetConfMediaType());

	return 	MaxContentRate;
}

/////////////////////////////////////////////////////////////////////////////
//Set the current content Bit Rate
void CConf::SetNewContentBitRate(BYTE newAMCBitRate,cmCapDirection eDirection)
{
   if( !CPObject::IsValidPObjectPtr(m_pUnifiedComMode) )
	   return;
   if( IsTIPContentEnable() )
	   PTRACE(eLevelInfoNormal, "CConf::SetNewContentBitRate : TIP content mode - don't allow to change bitrate!");
   m_pUnifiedComMode->SetNewContentBitRate(newAMCBitRate,eDirection);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetNewContentProtocol(BYTE newProtocol,cmCapDirection eDirection)
{
   BOOL isHD1080 = FALSE;
   BYTE HDMpi = 0;
   if(CPObject::IsValidPObjectPtr(m_pUnifiedComMode) )
   {
   	  if(newProtocol == H264)
   	  {
   		 PTRACE2(eLevelInfoNormal,"CConf::SelectContentProtocol, SELECTED CONTENT PROTOCOL is H264, choosing HD rate, Conf Name: ",m_name);
   		 BYTE newContentRateAMC		= AMSC_0k;
   		 newContentRateAMC  = SelectContentRate();
   		isHD1080 = SelectContentHDResolution(newContentRateAMC,newProtocol,HDMpi);
   	  }
   	  m_pUnifiedComMode->SetNewContentProtocol(newProtocol,eDirection,isHD1080,HDMpi);
   }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
DWORD CConf::GetActualIpRateForContentSession()
{
	return m_CurrentIpBitRateForContentSession;
}

/////////////////////////////////////////////////////////////////////////////
// get the current bit rate with/without the BCH factor
void CConf::SetActualIpRateForContentSession(DWORD actualBitRate)
{
	if( IsTIPContentEnable() )
	{
		PTRACE2INT(eLevelInfoNormal, "CConf::SetActualIpRateForContentSession - TIP content mode is active, actualBitRate=", actualBitRate);
		if(0 == actualBitRate)
			m_CurrentIpBitRateForContentSession = actualBitRate;
		else
		{
			DWORD actualBitRateForTip = ((CConf*)this)->TranslateAMCRateIPRate(AMC_512k);
			m_CurrentIpBitRateForContentSession = actualBitRateForTip;
			DBGPASSERT(actualBitRate != actualBitRateForTip);
		}
	}
	else
		m_CurrentIpBitRateForContentSession = actualBitRate;
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BYTE CConf::GetCurrentContentBitRateAMC() const
{
	return (m_pUnifiedComMode) ? m_pUnifiedComMode->GetCurrentContentBitRateAMC() : 255;
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BYTE CConf::GetCurrentContentProtocolInConfValues() const
{
	return (m_pUnifiedComMode) ? m_pUnifiedComMode->GetCurrentContentProtocolInIsdnValues() : 255;
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
CapEnum CConf::GetCurrentContentProtocolInIpValues() const
{
	return (m_pUnifiedComMode) ? m_pUnifiedComMode->GetCurrentContentProtocolInIpValues() : eUnknownAlgorithemCapCode;
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BYTE CConf::GetCurrentContentIsHighProfile() const
{
	return (m_pUnifiedComMode) ? m_pUnifiedComMode->GetIsHighProfileContent() : 0;
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void CConf::SetCurrentContentIsHighProfile(const BYTE isHPContent)
{
	if ((m_pUnifiedComMode))
		m_pUnifiedComMode->SetIsHighProfileContent(isHPContent);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void CConf::UpdateContentProtocolOnDisconnectParty()
{
	if(IsValidTimer(DELAY_UPDATE_CONTENT_TOUT)) // BRIDGE-15573:  Timer added to control frequency of updating content on disconnection blast
	{
		m_isUpdateContentPending = TRUE;
	}
	else
	{
		DWORD delayBetweenUpdateContent = 0;
		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("DELAY_BETWEEN_UPDATE_CONTENT_ON_DISCONNECTION", delayBetweenUpdateContent);
		StartTimer(DELAY_UPDATE_CONTENT_TOUT, delayBetweenUpdateContent * SECOND);
		DispatchEvent(UPDATE_CONTENT_PROTOCOL);
	}
}
/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
DWORD CConf::TranslateAMCRateIPRate(BYTE AMCRate)
{
	return CUnifiedComMode::TranslateAMCRateIPRate(AMCRate);
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
BYTE CConf::TranslateConfIPRateToXferRate(DWORD H323Rate)
{
	return CUnifiedComMode::TranslateIPRateToXferRate(H323Rate);
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void CConf::LobbyAddPartyVoiceConnect(CSegment* pParam, CTaskApp* pParty, char *name, CConfParty* pConfParty)
{
  CIsdnNetSetup * pNetSetUp = new CIsdnNetSetup;

	pNetSetUp->DeSerialize(NATIVE, *pParam);

      // update party monitoring phone number for first channal
      WORD numDigits = 0;
      char calledNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
      char callingNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];

      pNetSetUp->GetCalledNumber(&numDigits, calledNumber);
      pNetSetUp->GetCallingNumber(&numDigits, callingNumber);
      pConfParty->SetActualMCUandPartyPhoneNumbers((BYTE)0, calledNumber, callingNumber);

      CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
      const RTM_ISDN_PARAMS_MCMS_S * pIsdnServiceStruct = pConfPartyProcess->GetIsdnService("");
	ENetworkType networkType = E_NETWORK_TYPE_DUMMY;
      if (pIsdnServiceStruct)
	{
	  //Set Span Type
		networkType = GetNetoworkType(pIsdnServiceStruct->spanDef.spanType);
	}
      else
	{
	  TRACESTR (eLevelError) << "CConf::LobbyAddPartyVoiceConnect Can not find service" << pConfParty->GetName();
	  PASSERT(1);
	}
  // attach resources to conf
      COsQueue*  pPartyRcvMbx = new COsQueue;
      pPartyRcvMbx->DeSerialize(*pParam);

  PASSERT(pConfParty->GetVoice() != YES);

  WORD voice_type = GetVoiceType(pConfParty);

  CPartyConnection*  pPartyConnection = new CPartyConnection;

  BYTE StandByStart = m_StandByStart;
  WORD connectDelay = 0;
	char *avServiceNameStr = "\0";
  WORD welcomeMsgTime = 0;

// There are parameters we not use, we kept them in interface compatibility purposes
  pPartyConnection->ConnectPSTN(this, pNetSetUp, /*pCap, pPartyScm, */
	m_pRcvMbx, m_pAudBrdgInterface, m_pConfAppMngrInterface, pPartyRcvMbx, pParty,
	/*m_pTermNumAllocator->GetTermNumber()*/2, pConfParty->GetPhoneNumber(), DIALIN, pConfParty->GetName(), m_name, /*pConfParty->GetPassword()*/'\0', m_monitorConfId, pConfParty->GetPartyId(), networkType, voice_type, pConfParty->GetAudioVolume(), pConfParty->GetServiceProviderName()/*'\0'*/, StandByStart, connectDelay, avServiceNameStr, welcomeMsgTime,/*pConfParty->GetRecordingPort()*/FALSE);

  InsertPartyConnection(pPartyConnection);

  POBJDELETE(pNetSetUp);
  POBJDELETE(pPartyRcvMbx);
	UpdateDB(pParty, PARTYSTATE, PARTY_CONNECTING);
	UpdateDB(pParty, NETCHNL, 0xc0000000 | 0x00000001);
	UpdateDB(pParty, DISQ931, 0);
	UpdateDB(pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);

  CSegment* pRspMsg = new CSegment;
  *pRspMsg << (WORD)statOK;
  ResponedClientRequest(ADDINPARTY, pRspMsg);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CConf::LobbyAddPartyIsdnVideoConnect(CSegment* pParam, CTaskApp* pParty, char *name, CConfParty* pConfParty)
{
	 CIsdnNetSetup * pNetSetUp = new CIsdnNetSetup;

	pNetSetUp->DeSerialize(NATIVE, *pParam);
	      // update party monitoring phone number for first channal
	      WORD numDigits = 0;
	      char calledNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
	      char callingNumber[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];

	      pNetSetUp->GetCalledNumber(&numDigits, calledNumber);
	      pNetSetUp->GetCallingNumber(&numDigits, callingNumber);
	      pConfParty->SetActualMCUandPartyPhoneNumbers((BYTE)0, calledNumber, callingNumber);

	      CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	      const RTM_ISDN_PARAMS_MCMS_S * pIsdnServiceStruct = pConfPartyProcess->GetIsdnService("");
	ENetworkType networkType = E_NETWORK_TYPE_DUMMY;
	      if (pIsdnServiceStruct)
		{
		  //Set Span Type
		networkType = GetNetoworkType(pIsdnServiceStruct->spanDef.spanType);
		}
	      else
		{
		  TRACESTR (eLevelError) << "CConf::LobbyAddPartyVoiceConnect Can not find service" << pConfParty->GetName();
		  PASSERT(1);
		}
	      // attach resources to conf
	      COsQueue*  pPartyRcvMbx = new COsQueue;
	      pPartyRcvMbx->DeSerialize(*pParam);

	  WORD voice_type = pConfParty->GetVoice(); //GetVoiceType(pConfParty);

	  CPartyConnection*  pPartyConnection = new CPartyConnection;

	  BYTE StandByStart = m_StandByStart;
	  WORD connectDelay = 0;
	char *avServiceNameStr = "\0";
	  WORD welcomeMsgTime = 0;
	  DWORD vidBitrate = 0;
	  WORD terminalNumber = 0;
	  CCapH320* pH320Caps = new CCapH320;
	  CComMode* pPartyScm = new CComMode;
	  CComMode* pPartyTransmitScm = new CComMode;
  	  WORD confPartyConnectionType = TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());
  	  SetH320PartyCapsAndVideoParam(pPartyScm, pPartyTransmitScm, pH320Caps, pConfParty, vidBitrate, confPartyConnectionType);
  	  WORD numChnl = pConfParty->GetNetChannelNumber();
  	  DWORD confRate = m_pUnifiedComMode->GetCallRate();

	// There are parameters we not use, we kept them in interface compatibility purposes
	pPartyConnection->ConnectIsdn(this, pNetSetUp, pH320Caps, pPartyScm, pPartyTransmitScm, m_pRcvMbx, m_pAudBrdgInterface, m_pVideoBridgeInterface, m_pConfAppMngrInterface, m_pFECCBridge, m_pContentBridge, m_pTerminalNumberingManager, pPartyRcvMbx, pParty, terminalNumber, 0, numChnl, confPartyConnectionType, pConfParty->GetName(), m_name, m_monitorConfId, pConfParty->GetPartyId(), pConfParty->GetServiceProviderName(), networkType, GetNodeType(pConfParty), voice_type, StandByStart, connectDelay);

	  InsertPartyConnection(pPartyConnection);

	  POBJDELETE(pH320Caps);
	  POBJDELETE(pPartyScm);
	  POBJDELETE(pPartyTransmitScm);
	  POBJDELETE(pNetSetUp);

	  POBJDELETE(pPartyRcvMbx);
	UpdateDB(pParty, PARTYSTATE, PARTY_CONNECTING);
	UpdateDB(pParty, DISQ931, 0);
	UpdateDB(pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);

	  CSegment* pRspMsg = new CSegment;
	  *pRspMsg << (WORD)statOK;
	  ResponedClientRequest(ADDINPARTY, pRspMsg);
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void CConf::CorrectContentModeForParty(CPartyConnection* pPartyConnection, CIpComMode* pH323NewMode, bool isRateChanged, bool isProtocolChanged, bool isTriggeredParty)
{
#ifdef CONTENT_LOAD
	CStopper _stopper("CConf::CorrectContentModeForParty", "Duration");
#endif

	#undef TRACE_LOG
	#define TRACE_LOG pPartyConnection->GetFullName() << ", IsRateChanged:" << (int)isRateChanged << ", IsProtocolChanged:" << (int)isProtocolChanged << ", IsTriggeredParty:" << (int)isTriggeredParty

	PASSERT_AND_RETURN(!pPartyConnection);

	TRACECOND_AND_RETURN(!pH323NewMode, TRACE_LOG);

	CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
	TRACECOND_AND_RETURN(!pPartyCntl, TRACE_LOG << " - Failed, invalid PartyControl");

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyCntl->GetMonitorPartyId());
	TRACECOND_AND_RETURN(!pConfParty, TRACE_LOG << " - Failed, invalid ConfParty");

	DWORD partyState = pConfParty->GetPartyState();
	bool isPartyConnectedOrConnecting = (partyState == PARTY_CONNECTED || partyState == PARTY_CONNECTING || partyState == PARTY_CONNECTED_PARTIALY || partyState == PARTY_CONNECTED_WITH_PROBLEM);
	TRACECOND_AND_RETURN(!isPartyConnectedOrConnecting, TRACE_LOG << " - Failed, party is not connected or connecting");

	BYTE newContentRateAMC = GetCurrentContentBitRateAMC();
	BYTE newContentProtocolIsdn = GetCurrentContentProtocolInConfValues();
	CapEnum newContentProtocolIp   = GetCurrentContentProtocolInIpValues();

	eXcodeRsrcType ePartyXCodeEncoderType = eXcodeEncoderDummy;
	BYTE isXCodeConf = m_pCommConf->GetContentMultiResolutionEnabled();

	if (isXCodeConf)
		ePartyXCodeEncoderType = GetPartyXCodeEncoderType(pPartyConnection, pConfParty);

	if (!m_pCommConf->GetIsAsSipContent())
	{
		bool isRemoteAndLocalCapSetHasContent = pPartyCntl->IsRemoteAndLocalCapSetHasContent(eToPrintOnFalseOnly);
		TRACECOND_AND_RETURN(!isRemoteAndLocalCapSetHasContent, TRACE_LOG << " - Failed, remote and local CapSet does not has content");

		if (isXCodeConf)
			TRACECOND_AND_RETURN(ePartyXCodeEncoderType == eXcodeEncoderDummy, TRACE_LOG << " - Failed, party does not support content");

		TRACECOND_AND_RETURN(pPartyCntl->GetPossibleContentRate() < newContentRateAMC, TRACE_LOG << " - Failed, party does not support current content rate");
	}

#ifdef CONTENT_LOAD
	BYTE actulaContentRateAMC = AMC_128k;
#else
	BYTE actulaContentRateAMC = SelectContentRate();//TODO remove from here
#endif

	bool isTIPContentEnable = pH323NewMode->IsTIPContentEnableInH264Scm();

	TRACEINTO << TRACE_LOG << ", isTIPContentEnable:" << (int)isTIPContentEnable
		<< ", newContentRateAMC: " << (int)newContentRateAMC
		<< ", newContentProtocolIsdn: " << (int)newContentProtocolIsdn
		<< ", newContentProtocolIp: " << (int)newContentProtocolIp;

	if (isProtocolChanged)
	{
		//FSN-613: Dynamic Content for SVC/Mix Conf, just delete redundant check.
		pH323NewMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
		pH323NewMode->CopyMediaMode(*m_pUnifiedComMode->GetIPComMode(), cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

		if (!isTIPContentEnable && !m_pCommConf->GetIsPreferTIP() && IsTIPContentEnable())
		{
			pH323NewMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			PTRACE(eLevelInfoNormal, "CConf::CorrectContentModeForParty -- TIPContent not match, set content off");
			return;
		}
	}

	bool isContentHD1080Suppported = false;

	if (isRateChanged)
	{
		if (m_IsAsSipContentEnable && pPartyConnection->GetInterfaceType() == SIP_INTERFACE_TYPE && pPartyCntl->IsRemoteAndLocalCapSetHasBfcpUdp() && pPartyCntl->IsFirstContentNegotiation() && !pPartyCntl->IsRemoteAndLocalCapSetHasContent())
		{
			pH323NewMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			pH323NewMode->CopyMediaMode(*m_pUnifiedComMode->GetIPComMode(), cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

			CSipCaps* pSipLocalCaps = ((CSipPartyCntl*)pPartyCntl)->GetLocalCap();

			CSdesCap *pSdesCap = pSipLocalCaps->GetSdesCap(cmCapVideo, kRolePresentation);

			if (pSdesCap && pH323NewMode->GetIsEncrypted())
			{
				pH323NewMode->SetSipSdes(cmCapVideo, cmCapReceive, kRolePresentation, pSdesCap); // Sdes for receive direction will be updated after remote will send its 200ok
				pH323NewMode->SetSipSdes(cmCapVideo, cmCapTransmit, kRolePresentation, pSdesCap);
			}
			delete pSdesCap;
		}

		//pH323NewMode->Dump("CConf::CorrectContentModeForParty - IP CONTENT SCM", eLevelInfoNormal);

		BYTE HDMpi = 0;

		DWORD newContentRate = GetActualIpRateForContentSession();

		if (!isXCodeConf)
		{
			if (newContentProtocolIsdn == H264)
			{
#ifdef CONTENT_LOAD
				isContentHD1080Suppported = false;
				HDMpi = 10;
#else
				isContentHD1080Suppported = SelectContentHDResolution(actulaContentRateAMC, newContentProtocolIsdn, HDMpi);//TODO remove from here
#endif
				PTRACE2INT(eLevelInfoNormal, "CConf::CorrectContentModeForParty -New HDMpi: ", HDMpi);
			}

			if (IsTIPContentEnable())
			{
				if (isTIPContentEnable) //just for TipCompatibility:video&content!
				{
					pH323NewMode->SetTIPContent(newContentRateAMC, cmCapReceiveAndTransmit);
				}
				else // preferTip
				{
					pH323NewMode->SetTIPContent(newContentRateAMC, cmCapReceiveAndTransmit, FALSE);
				}
			}
			else
			{
				pH323NewMode->SetContent(newContentRateAMC, cmCapReceiveAndTransmit, newContentProtocolIp, isContentHD1080Suppported, HDMpi, GetCurrentContentIsHighProfile());  //HP content
			}

			pH323NewMode->SetVideoBitRate(newContentRate, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
		}
		else
		{
#ifdef CONTENT_LOAD
			actulaContentRateAMC = AMC_128k;
#else
			actulaContentRateAMC = SelectContentRate(0, ePartyXCodeEncoderType);//TODO remove from here
#endif
			DWORD newPartyContentRate = TranslateAMCRateIPRate(actulaContentRateAMC);

			BYTE newPartyContentAMC;
			if (newContentRate == 0)
			{
				newPartyContentRate = 0;
				newPartyContentAMC = AMC_0k;
			}
			else
			{
				newPartyContentAMC = actulaContentRateAMC;
			}

			if ((isTriggeredParty && ePartyXCodeEncoderType == eXcodeH264Encoder) || (ePartyXCodeEncoderType == eXcodeH264LinksEncoder))
			{
				PTRACE2INT(eLevelInfoNormal, "CConf::CorrectContentModeForParty -This is triggered party we will update the SCM ", newContentRate);
				isContentHD1080Suppported = SelectContentHDResolution(actulaContentRateAMC, H264, HDMpi, TRUE, ePartyXCodeEncoderType);
				pH323NewMode->SetContent(newPartyContentAMC, cmCapReceiveAndTransmit, GetCurrentContentProtocolInIpValues(), isContentHD1080Suppported, HDMpi, m_pCommConf->GetIsHighProfileContent());
			}

			if (ePartyXCodeEncoderType == eXcodeH263Encoder)
			{
				pH323NewMode->SetContent(newPartyContentAMC, cmCapReceiveAndTransmit, eH263CapCode);
				TRACEINTO << "Content Transcoding Conf - Leave H263 Content to EP which does not supporat H264 Content, EP Name: " << pPartyCntl->GetFullName() << "\n";
			}
			pH323NewMode->SetVideoBitRate(newPartyContentRate, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			TRACEINTO << "XCode Conference - Party's content Encoder is: " << eXcodeRsrcTypeNames[ePartyXCodeEncoderType] << " party's Content Rate: " << newPartyContentRate << " Party Name: " << pPartyCntl->GetFullName() << "\n";
		}
	}
#ifdef CONTENT_LOAD
	_stopper.Stop();
#endif
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CConf::CorrectContentModeForIsdnParty(CPartyConnection* pPartyConnection, CComMode* pNewMode) const
{
	//  conference content rate - by the weakest party
	BYTE  newContentRateAMC = ((CConf*)this)->GetCurrentContentBitRateAMC(); //from unified scm
	BYTE  resonContentRateAMC = newContentRateAMC;

	eXcodeRsrcType  ePartyXCodeEncoderType = eXcodeEncoderDummy;

	int   reason         = 0;
	CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();

	//  party channel number
	BYTE  partyChanNum   = pNewMode->GetNumB0Chnl();  //m_xferMode.GetNumChnl();
	BYTE isRestricted = (pNewMode->GetOtherRestrictMode() == Restrict) ? YES : NO;

	do
	{
		CConfParty*  pConfParty = m_pCommConf->GetCurrentParty(pPartyCntl->GetName());
		if (!pConfParty || !(pConfParty->GetPartyState() == PARTY_CONNECTED || pConfParty->GetPartyState() == PARTY_CONNECTED_PARTIALY || pConfParty->GetPartyState() == PARTY_CONNECTED_WITH_PROBLEM))
		{
			resonContentRateAMC = AMC_0k;
			reason         = 1;
			break;
		}
		if (!pPartyCntl->IsRemoteAndLocalCapSetHasContent())
		 {
			resonContentRateAMC = AMC_0k;
			reason         = 2;
			break;
		}
		if (m_pCommConf->GetContentMultiResolutionEnabled())
		 {
			 ePartyXCodeEncoderType = ((CConf*)this)->GetPartyXCodeEncoderType(pPartyConnection, pConfParty);
			if (ePartyXCodeEncoderType == eXcodeEncoderDummy)
			 {
				 resonContentRateAMC = AMC_0k;
				 reason         = 3;
				 break;
			 }
		 }
	} while (0);

	if (IsValidPObjectPtr(pNewMode))
	{
		// in the scm we use AMSC
		if (newContentRateAMC > AMC_0k && m_pCommConf->GetContentMultiResolutionEnabled())
		{
			if (ePartyXCodeEncoderType != eXcodeEncoderDummy)
				// Romem now
			{
				newContentRateAMC = ((CConf*)this)->SelectContentRate(FALSE, ePartyXCodeEncoderType);
			}
		}
		BYTE newContentRateAMSC = m_pUnifiedComMode->TranslateAmcRateToAmscRate(newContentRateAMC);
		pNewMode->m_contentMode.SetContentRate(newContentRateAMSC);
		pNewMode->m_contentMode.CalculateSartEndDummyForH239(partyChanNum);
		if (newContentRateAMSC > AMSC_0k)
		{
			pNewMode->m_contentMode.SetOpcode(NS_COM_AMSC_ON);
			PTRACE(eLevelInfoNormal, "**newContentRateAMSC > AMSC_0k SetOpcode(NS_COM_AMSC_ON)");
		}
	}

	if (AMC_0k == resonContentRateAMC)
	{
		char* pMess = new char[ONE_LINE_BUFFER_LEN];
		sprintf(pMess, "CConf::CorrectContentModeForIsdnParty : Rate AMC_0k, Reason (%d), Name - ", reason);
		PTRACE2(eLevelInfoNormal, pMess, pPartyCntl->GetFullName());
		PDELETEA(pMess);
		// if partyContentRate not the same as conf ContentRate
		if (resonContentRateAMC != newContentRateAMC)
		{
			CMedString cstr;
			cstr << "**resonContentRateAMC != newContentRateAMSC SetOpcode(NS_COM_AMSC_OFF)" << "\n";
			cstr << "resonContentRateAMC : " << m_pContentBridge->GetContentRateAsString(resonContentRateAMC) << "\n";
			cstr << "newContentRateAMC   : " << m_pContentBridge->GetContentRateAsString(resonContentRateAMC);
			PTRACE(eLevelInfoNormal, cstr.GetString());
			pNewMode->m_contentMode.SetContentRate(AMSC_0k);
			pNewMode->m_contentMode.CalculateSartEndDummyForH239(partyChanNum);
			pNewMode->m_contentMode.SetOpcode(NS_COM_AMSC_OFF); //??
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void CConf::ChangeContentMode(CPartyConnection* pTriggeredPartyConnection, bool isRateChanged,
	bool isProtocolChanged, bool isDowngradingfToContentVSW)
{
	TRACEINTO << "ConfName: " << m_name << ", IsRateChanged:" << (int)isRateChanged
		<< ", IsProtocolChanged:" << (int)isProtocolChanged
		<< ", isDowngradingfToContentVSW:" << (int)isDowngradingfToContentVSW
		<< ", IsAsSipContentEnable:" << (int)m_IsAsSipContentEnable;

#ifdef CONTENT_LOAD
	CStopper _stopper("CConf::ChangeContentMode", "Duration");
#endif

	BYTE sel_contentAmcRate = SelectContentRate();
	DWORD sel_contentIPRate = TranslateAMCRateIPRate(sel_contentAmcRate);

	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();

	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;

		if (!pPartyConnection)
		{
			TRACEINTO << "No valid pPartyConnection";
			continue;
		}

		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();

		if (!pPartyCntl)
		{
			TRACEINTO << "No valid pPartyCntl";
			continue;
		}

		// Bridge-11896
		if (pPartyCntl->IsDisconnect())
		{
			TRACEINTO << "PartyName:" << pPartyCntl->GetName() << " - Party is disconnecting";
			continue;
		}

		WORD partyInterfaceType = pPartyConnection->GetInterfaceType();

		if (!pPartyCntl->IsRemoteAndLocalCapSetHasContent(eToPrintOnFalseOnly))
		{
			if (partyInterfaceType == SIP_INTERFACE_TYPE && m_IsAsSipContentEnable && pPartyCntl->IsRemoteAndLocalCapSetHasBfcpUdp())
				TRACEINTO << "PartyName:" << pPartyCntl->GetName() << " - AS-SIP conference, party with  BFCP_UDP, without content caps, proceed change content mode";
			else
			{
				TRACEINTO << "No valid IsRemoteAndLocalCapSetHasContent";
				continue;
			}
		}

		BYTE isLegacy = pPartyCntl->IsLegacyContentParty();
		BYTE isLync = pPartyCntl->IsLync();

		// In case of down-grade to VSW content force change mode on Legacy party in the VSW configuration
		if (!isDowngradingfToContentVSW && (isLegacy || isLync))
		{
			//In AS-SIP conference the SIP Party is set as legacy party, but we want to change him to content party.
			if (partyInterfaceType == SIP_INTERFACE_TYPE && m_IsAsSipContentEnable && pPartyCntl->IsRemoteAndLocalCapSetHasBfcpUdp())
				TRACEINTO << "PartyName:" << pPartyCntl->GetName() << " - AS-SIP conference, Legacy party with BFCP_UDP, without content caps, proceed change content mode";
			else
			{
				TRACEINTO << "No valid. isDowngradingfToContentVSW - " << isDowngradingfToContentVSW << ", Legacy Content - " << (int)isLegacy << ", IsLync - " << (int)isLync;
				continue;
			}
		}

		if (partyInterfaceType == ISDN_INTERFACE_TYPE && ((CIsdnPartyCntl*)pPartyCntl)->IsRemoteContentFailed())
		{
			TRACEINTO << "PartyName:" << pPartyCntl->GetName() << " - Remote has no content, ISDN EP did non opened content";
			continue;
		}

		CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyCntl->GetMonitorPartyId());
		if (!pConfParty)
		{
			TRACEINTO << "No valid pConfParty";
			continue;
		}

		WORD partyConnectionState = pConfParty->GetPartyState();

		if (!(partyConnectionState == PARTY_CONNECTED || partyConnectionState == PARTY_CONNECTED_PARTIALY || partyConnectionState == PARTY_CONNECTED_WITH_PROBLEM || (partyConnectionState == PARTY_CONNECTING && pPartyCntl->IsRemoteAndLocalCapSetHasContent())))
		{
			TRACEINTO << "PartyName:" << pPartyCntl->GetName() << " - Party is not connected";
			continue;
		}

		switch (partyInterfaceType)
		{
			case ISDN_INTERFACE_TYPE:
			{
				// send VCF to video - Only for H320 parties - added following video team request (Eitan 08/2008)
				CPartyApi* pPartyApi = pPartyConnection->GetPartyApi();
				if (pPartyApi)
					pPartyApi->PartyVideoFreeze();

				// Eitan - We use target scm because:
				// if we call this func from SetCapCommonDenominator current scm are with no video
				// if we call this func from content bridge (start content) current and target scm should be the same
				CComMode* pIsdnTargetTransmitScm = pPartyConnection->GetIsdnTargetTransmitScm();
				CComMode* pIsdnTargetReceiveScm = pPartyConnection->GetIsdnTargetReceiveScm();

				CComMode* pIsdnNewModeTransmit = new CComMode(*pIsdnTargetTransmitScm);
				CComMode* pIsdnNewModeReceive = new CComMode(*pIsdnTargetReceiveScm);

				CorrectContentModeForIsdnParty(pPartyConnection, pIsdnNewModeTransmit);
				PTRACE2(eLevelInfoNormal, "CConf::ChangeContentMode for ISDN: Name - ", m_name);
				pIsdnNewModeTransmit->Dump(1);

				pIsdnNewModeReceive->m_contentMode = pIsdnNewModeTransmit->m_contentMode;

				pPartyConnection->ChangeIsdnScm(pIsdnNewModeTransmit, pIsdnNewModeReceive);

				POBJDELETE(pIsdnNewModeTransmit);
				POBJDELETE(pIsdnNewModeReceive);
				break;
			}

			default:
			{
				EChangeMediaType eChangeMedia = eCanChangeVideoAndContent;

				// change Content rate for party
				CComModeH323* pH323CurrentScm = pPartyConnection->GetCurrentIpScm();

				CIpComMode* pPartyScmNewContent = NULL;

				if (pH323CurrentScm->GetConfType() == kVSW_Fixed)
					pPartyScmNewContent = new CIpComMode(*m_pUnifiedComMode->GetIPComMode());
				else if (pPartyCntl->GetState() == PARTY_RE_CAPS || pPartyCntl->GetState() == REALLOCATE_RSC)
				{ //bridge 13419 & bridge 14527 - in case of re cap, use initial for current scm, it's updated than current.
					pPartyScmNewContent = new CIpComMode(*(pPartyConnection->GetInitialIpScm())) ;
				}
				else
					pPartyScmNewContent = new CIpComMode(*pH323CurrentScm); //to avoid sending conference unified pointer (function param NOT const)

				if (pPartyConnection->ProceedToChangeContentMode())
				{
					if (!isRateChanged && pConfParty->GetIsLyncPlugin())
					{
						DWORD party_contentRate = ((CSipPartyCntl*)pPartyCntl)->GetMinContentPartyRate();
						if (party_contentRate > sel_contentIPRate)
							isRateChanged = true;
					}

					bool isTriggeredParty = (pTriggeredPartyConnection == pPartyConnection);

					CorrectContentModeForParty(pPartyConnection, pPartyScmNewContent, isRateChanged, isProtocolChanged, isTriggeredParty);

					//FSN-613: Dynamic Content for SVC/Mix Conf
					if (pPartyCntl->GetIsMrcCall())
						pPartyScmNewContent->SetDeclareContentRate(sel_contentIPRate);

					pPartyScmNewContent->Dump("***CConf::ChangeContentMode - IP CONTENT SCM", eLevelInfoNormal);

					if (partyInterfaceType == H323_INTERFACE_TYPE)
						pPartyConnection->ChangeH323Scm(pPartyScmNewContent, eChangeMedia);
					if (partyInterfaceType == SIP_INTERFACE_TYPE)
						pPartyConnection->ChangeSipScm(pPartyScmNewContent, m_IsAsSipContentEnable);
				}
				else
					PTRACE2(eLevelInfoNormal, "CConf::ChangeContentMode : Not fully connected party - do not send change scm!!, Name - ", m_name);

				POBJDELETE(pPartyScmNewContent);
				break;
			}
		}
	}

#ifdef CONTENT_LOAD
	_stopper.Stop();
#endif

}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CConf::IsConfRateValidForH239()
{
	BOOL bIsRateValid = FALSE;
	BYTE lConfRate = m_pCommConf->GetConfTransferRate();
	if(!((lConfRate == Xfer_64)||(lConfRate == Xfer_96)))
		bIsRateValid = TRUE;

	return bIsRateValid;
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CConf::IsEnableH239()
{
	static BOOL bIsH239 = 0xFF;
	if (bIsH239 == 0xFF)
		bIsH239 = GetSystemCfgFlag<BOOL>("ENABLE_H239");
	return (bIsH239) ? IsConfRateValidForH239() : bIsH239;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
DWORD CConf::TranslateVideoIpRateToCorrectReservationRate(DWORD reservationVideoRate,DWORD h323ConfRate)
{

	// Only for 96 rate - Private case
	if (reservationVideoRate <= 64	&& reservationVideoRate > 0)
		return 64;
	if (reservationVideoRate > 64 && reservationVideoRate <= 96 && h323ConfRate == 96)
		return 96;
	else if (reservationVideoRate >96 && reservationVideoRate <=128)
		return 128;
	else if (reservationVideoRate >128 && reservationVideoRate <= 192)
		return 192;
	else if (reservationVideoRate >192 && reservationVideoRate <= 256)
		return 256;
	else if (reservationVideoRate >256 && reservationVideoRate <= 320)
		return 320;
	else if (reservationVideoRate >256 && reservationVideoRate <= 384)
		return 384;
	else if (reservationVideoRate >384 && reservationVideoRate <= 512)
		return 512;
	else if (reservationVideoRate >512 && reservationVideoRate <= 768)
		return 768;
	else if (reservationVideoRate >768 && reservationVideoRate <= 1024)
		return 1024;
	else if (reservationVideoRate >1024 && reservationVideoRate <= 1472)
		return 1472;
	else if (reservationVideoRate >1472 && reservationVideoRate <= 1920)
		return 1920;
	else if (reservationVideoRate > 1920 && reservationVideoRate <= 4076)
		return 4076;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
bool CConf::IsLegacyShowContentAsVideo() const
{
	return (m_pCommConf && m_pCommConf->IsLegacyShowContentAsVideo());
}

/////////////////////////////////////////////////////////////////////////////
bool CConf::IsHDVSW() const
{
	return (m_pCommConf && m_pCommConf->GetIsHDVSW());
}

/////////////////////////////////////////////////////////////////////////////
void CConf::SetHDVSWComMode()
{
	// HD is implemented as VSW fixed conference
	m_pUnifiedComMode->SetConfType(VIDEO_SWITCH_FIXED);
	m_pUnifiedComMode->SetIsAutoVidProtocol(NO);
	m_pUnifiedComMode->SetIsAutoVidRes(NO);

	WORD isAutoVidScm = NO;
	int profile = H264_Profile_None, level;
	long mbps, fs, dpb, brAndCpb, sar, staticMB;
	EHDResolution HDResolution = m_pCommConf->GetHDResolution();
	BYTE H264VSWHighProfilePreferenceType = m_pCommConf->GetH264VSWHighProfilePreference();

	if (HDResolution != eH263Res && HDResolution != eH261Res)
		::GetH264VideoHdVswParam(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, HDResolution);
	m_pUnifiedComMode->SetHdVswResolution((EHDResolution)HDResolution);

	if (H264VSWHighProfilePreferenceType == eAlways)
	{
		profile = H264_Profile_High;
	}

	if (HDResolution == eH263Res)
		m_pUnifiedComMode->SetProtocolClassicVidMode(H263, -1, 1, -1, -1);
	else if (HDResolution == eH261Res)
		m_pUnifiedComMode->SetProtocolClassicVidMode(H261, -1, 1, -1, -1);
	else
		m_pUnifiedComMode->SetH264VidMode(profile, level, brAndCpb, mbps, fs, dpb, sar, staticMB, isAutoVidScm, cmCapReceiveAndTransmit);

	//VNGFE-8408/BRIDGE-14183 -- To keep compatibility with old VVX, we need set H264PacketizationMode as 0 in VSW conf
	m_pUnifiedComMode->SetH264PacketizationMode(0);

	DWORD conf_rate = 0;
	DWORD audio_rate = 0;
	DWORD video_rate = 0;
	CalculateReservationRates(m_pCommConf, conf_rate, audio_rate, video_rate);

	TRACEINTO << "ConfRate:" << conf_rate << ", AudioRate:" << audio_rate << ", VideoRate:" << video_rate << ", HDResolution:" << HDResolution << ", Profile:" << profile;

	video_rate = video_rate / 100; // divided by 100 because is used in system in
	m_pUnifiedComMode->SetVideoBitRate(video_rate, cmCapReceiveAndTransmit);
}

/////////////////////////////////////////////////////////////////////////////
void CConf::SetCopComMode()
{
	PTRACE2(eLevelInfoNormal, "CConf::SetCopComMode : Name - ", m_name);

	sCopH264VideoMode copH264VideoMode;
	CCopVideoModeTable* pCopTable = new CCopVideoModeTable;

	// Get the highest cop level params:
	CCOPConfigurationList* pCOPConfigurationList = m_pCommConf->GetCopConfigurationList();
	CCopVideoParams* pCopHighestLevelParams = pCOPConfigurationList->GetVideoMode(0);
	pCOPConfigurationList->Dump("CConf::SetCopComMode - cop configuration:", eLevelInfoNormal);

	if (pCopHighestLevelParams->GetProtocol() == VIDEO_PROTOCOL_H264 || pCopHighestLevelParams->GetProtocol() == VIDEO_PROTOCOL_H264_HIGH_PROFILE)
	{
		// Set Scm - transmit:
		pCopTable->GetSignalingH264ModeAccordingToReservationParams(pCopHighestLevelParams, copH264VideoMode, TRUE);
		APIU16 profile = GetProfileAccordingToCopProtocol(pCopHighestLevelParams->GetProtocol());
		m_pUnifiedComMode->SetH264VidMode(profile, copH264VideoMode.levelValue, copH264VideoMode.maxBR, copH264VideoMode.maxMBPS, copH264VideoMode.maxFS, copH264VideoMode.maxDPB, copH264VideoMode.maxSAR, copH264VideoMode.maxStaticMbps, m_pCommConf->GetVideoProtocol(), cmCapTransmit);
		// Set Scm - receive:
		BOOL isUseHD1080 = FALSE;
		APIU16 profileRec = H264_Profile_BaseLine;
		eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
		if (IsFeatureSupportedBySystem(eFeatureH264HighProfile))
		{
			isUseHD1080 = TRUE;
			if (pCOPConfigurationList->IsHighProfileFoundInOneLevelAtLeast())
				profileRec = H264_Profile_High;
		}
		else if (pCopHighestLevelParams->GetProtocol() == VIDEO_PROTOCOL_H264_HIGH_PROFILE)
			DBGPASSERT(pCopHighestLevelParams->GetProtocol() + 1000);

		pCopTable->GetSignalingH264ModeAccordingToReservationParams(pCopHighestLevelParams, copH264VideoMode, isUseHD1080);
		m_pUnifiedComMode->SetH264VidMode(profileRec, copH264VideoMode.levelValue, copH264VideoMode.maxBR, copH264VideoMode.maxMBPS, copH264VideoMode.maxFS, copH264VideoMode.maxDPB, H264_ALL_LEVEL_DEFAULT_SAR, copH264VideoMode.maxStaticMbps, m_pCommConf->GetVideoProtocol(), cmCapReceive);
	}
	else
		DBGPASSERT(pCopHighestLevelParams->GetProtocol() + 1000); // The highest level is always h264

	// Set cop encoders levels:
	CCopVideoTxModes* pCopVideoTxModes = new CCopVideoTxModes;
	pCopVideoTxModes->SetModesAccodingToCopParams(pCOPConfigurationList);
	m_pUnifiedComMode->SetCopVideoTxModes(pCopVideoTxModes);
	POBJDELETE(pCopVideoTxModes);

	// Set additional parameters:
	m_pUnifiedComMode->SetConfType(VIDEO_SESSION_COP);
	m_pUnifiedComMode->SetIsAutoVidProtocol(m_pCommConf->GetVideoProtocol());
	m_pUnifiedComMode->SetIsAutoVidRes(m_pCommConf->GetVideoPictureFormat());

	POBJDELETE(pCopTable);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
WORD CConf::TranslateToConfPartyConnectionType(WORD dialType)
{
	if (dialType == DIAL_OUT)
		return DIALOUT;
	else if (dialType == DIAL_IN)
		return DIALIN;
	else
	{
		PASSERTMSG(dialType,"CConf::TranslateToConfPartyConnectionType");
		return  dialType;
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
ENetworkType CConf::GetNetoworkType(WORD spanType)const
{
  if ( eSpanTypeE1 == spanType )
    return E_NETWORK_TYPE_PSTN_E1;
  else if (eSpanTypeT1 == spanType )
    return E_NETWORK_TYPE_PSTN_T1;
  else
    {
      PASSERT(1);
      TRACESTR (eLevelInfoNormal) <<"CConf::GetNetoworkType Undefined span type:" << spanType;
    }

  return E_NETWORK_TYPE_DUMMY;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CConf::ComputeConnectingConnectionDelay(BYTE isAudio, const char* partyName, BYTE netInterfaceType)
{
  // We need to insert blast control here too
  // Instead of the connectionDelay - We will use the redailInterval.
  CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

  DWORD videoConnectDelay = 0, audioConnectDelay = 0, isdnConnectDelay = 0;
  sysConfig->GetDWORDDataByKey("DELAY_BETWEEN_DIAL_OUT_PARTY", videoConnectDelay);
  sysConfig->GetDWORDDataByKey("DELAY_BETWEEN_AUDIO_DIAL_OUT_PARTY", audioConnectDelay);
  sysConfig->GetDWORDDataByKey("DELAY_BETWEEN_H320_DIAL_OUT_PARTY", isdnConnectDelay);

  // Connection delay should be according to the last party connected
  DWORD sysCfgConnectDelay = 0;
  if (TRUE == m_isLastPartyConnectedAudioOnly)
    sysCfgConnectDelay = audioConnectDelay/10;
  else if (m_LastPartyInterfaceType == ISDN_INTERFACE_TYPE)
    sysCfgConnectDelay = isdnConnectDelay/10;
  else
    sysCfgConnectDelay = videoConnectDelay/10;    // connectDelay is in SECONDS - should be 0.25 SECONDS

  // Set the party interface for the next connection
  m_isLastPartyConnectedAudioOnly = isAudio;
  m_LastPartyInterfaceType        = netInterfaceType;

  DWORD computedConnectionDelay   = 0;
  TICKS currentConnectionInTicks  = 0;            // Current participant ticks
  TICKS currentGapInTicks         = 0;
  TICKS totalGapBetweenAddRequest = 0;
  DWORD supposedConnectionTime    = 0;
  DWORD absConnectionTime         = 0;
  DWORD connectionDelay           = 0;

  if (m_firstParticipantConnetingTicksTime == 0)  // First party in conf.
  { // No need for delay
    m_firstParticipantConnetingTicksTime = m_lastPartyConnectedInTicks = SystemGetTickCount().GetIntegerPartForTrace();
    m_numOfBlastParticipants++;
    TRACEINTO << "PartyName:" << partyName << ", FirstParticipantConnetingTicksTime:" << m_firstParticipantConnetingTicksTime.GetIntegerPartForTrace();
  }
  else                                            // Start computing delay algorithm
  {
    // Step 2: Compute current gap and check we need to add delay
    currentConnectionInTicks = SystemGetTickCount().GetIntegerPartForTrace();

    // Get the gap between the last party connected and this new party
    currentGapInTicks = currentConnectionInTicks - m_lastPartyConnectedInTicks;

    // Get the Gap between the first party and this new party
    totalGapBetweenAddRequest = currentConnectionInTicks - m_firstParticipantConnetingTicksTime;

    // NumOfPArties*Cfgdelay - GapFromTheFirstParty
    if ((m_numOfBlastParticipants*sysCfgConnectDelay) > totalGapBetweenAddRequest.GetIntegerPartForTrace())
      computedConnectionDelay = (m_numOfBlastParticipants*sysCfgConnectDelay) - totalGapBetweenAddRequest.GetIntegerPartForTrace();
    else
      computedConnectionDelay = 0;

    // Compute the connection time of the new party
    supposedConnectionTime = currentConnectionInTicks.GetIntegerPartForTrace() + computedConnectionDelay;

    // Compute the gap
    absConnectionTime = abs((int)(m_lastPartyConnectedInTicks.GetIntegerPartForTrace() - currentConnectionInTicks.GetIntegerPartForTrace()));
    if ((currentGapInTicks < sysCfgConnectDelay) || (absConnectionTime < sysCfgConnectDelay))
    {
      // compute and send to delay PC
      connectionDelay = computedConnectionDelay;
      TRACEINTO << "PartyName:" << partyName << ", ConnectionDelay:" << connectionDelay;
      m_lastPartyConnectedInTicks = supposedConnectionTime;
      m_numOfBlastParticipants++;
    }
    else
    {
      TRACEINTO << "PartyName:" << partyName << ", No need to add delay for party";
      // Restart the whole procedure with the last party as the first connected one for this session
      m_numOfBlastParticipants             = 1;
      m_firstParticipantConnetingTicksTime = m_lastPartyConnectedInTicks = currentConnectionInTicks;
    }
  }

  return connectionDelay;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CConf::isISDNPartyExistInConf()
{
	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (pPartyConnection)
		{
			CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
			if (pPartyCntl && pPartyCntl->GetInterfaceType() == ISDN_INTERFACE_TYPE)
				return TRUE;
		}
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////
void CConf::ActivateAutoTermenation(const char* callerFunc)
{
  ELastQuitType lastQuitType = (ELastQuitType)m_pCommConf->GetLastQuitType();

  // Active auto termination of the conf
  std::ostringstream msg;
  msg << "CConf::ActivateAutoTermination(" << callerFunc << ") "
      << "- State:"                 << ConfStateToString(m_state)
      << ", AutomaticTermination:"  << (int)m_pCommConf->IsAutomaticTermination()
      << ", LastQuitType:"          << LastQuitTypeToString(lastQuitType);

  // after at least one party was fully connected.
  if ((m_state != TERMINATION) && m_pCommConf->IsAutomaticTermination())
  {

	bool ignoreItpSlaves = false;
	if( m_isGateWay )
	    ignoreItpSlaves = true;
    WORD numOfNonDisconnectedParties = 0;
    BYTE areAllDisconnected          = AreAllPartiesDisconnected(numOfNonDisconnectedParties, ignoreItpSlaves);

    if (lastQuitType == eTerminateAfterLastLeaves)
    {
      if (m_IsAnyPartyWasConnected && areAllDisconnected)
      {
        msg << ", RC:1 [Timer:AUTOTERMINATE, duration:" << m_AutoTerminateAfterLastQuit << " [AfterLastQuit]]";
        StartTimer(AUTOTERMINATE, m_AutoTerminateAfterLastQuit * SECOND);
      }
      else
      {
        msg << ", RC:0 [Not all participats disconnected]";
      }
    }
    else
    {
      if (m_IsAnyPartyWasConnected && numOfNonDisconnectedParties < 2)
      {
        if (m_isGateWay && (strstr(m_pCommConf->GetName(), "DMAGW")))
        {
          msg << ", Is GW-DMA Conference:1";
          GwPartiesStateFlag = GW_CONNECT;
        }

        // don't activate autoterminate in GW sessions before setup completes
        if (!m_isGateWay || (m_isGateWay && GwPartiesStateFlag))
        {
          msg << ", RC:1 [Timer:AUTOTERMINATE, duration:" << m_AutoTerminateAfterLastQuit << " [WithLastRemains]]";
          StartTimer(AUTOTERMINATE, m_AutoTerminateAfterLastQuit * SECOND);
        }
        else
        {
          msg << ", RC:0, [GW conference with GwPartiesStateFlag=SETUP]";
        }
      }
    }

    if (m_AutoTerminateBeforeFirstJoin && !m_IsAnyPartyWasConnected && areAllDisconnected)
    {
      msg << ", RC:1 [Timer:AUTOTERMINATE, duration:" << m_AutoTerminateBeforeFirstJoin << " [BeforeFirstJoin]]";
      StartTimer(AUTOTERMINATE, m_AutoTerminateBeforeFirstJoin * SECOND);
    }
  }
  else
  {
    msg << ", RC:0";
  }

  PTRACE(eLevelInfoNormal, msg.str().c_str());
}
//////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
void CConf::ActivateAutoTerminationForMsAvMcu(const char* callerFunc)
{
  ELastQuitType lastQuitType = (ELastQuitType)m_pCommConf->GetLastQuitType();
  BYTE m_isAvMcuConnected = FALSE;
  // Active auto termination of the conf
  std::ostringstream msg;
  msg << "CConf::ActivateAutoTerminationForMsAvMcu(" << callerFunc << ") "
      << "- State:"                 << ConfStateToString(m_state)
      << ", AutomaticTermination:"  << (int)m_pCommConf->IsAutomaticTermination()
      << ", LastQuitType:"          << LastQuitTypeToString(lastQuitType);
  if(m_AvMcuPartyRsrcId == 0 )
  {
	  TRACEINTO <<" m_AvMcuPartyRsrcId is 0  ";
	 // return;
  }

  // after at least one party was fully connected.
  if ((m_state != TERMINATION) && m_pCommConf->IsAutomaticTermination())
  {
		  CConfParty*  pConfParty = m_pCommConf->GetFirstParty(); // get the first party
		  WORD numParties = m_pCommConf->GetNumParties();
	  		for (WORD i = 0; i < numParties; i++)
	  		{
	  			if (pConfParty)
	  			{
	  				if(pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveOut && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveIn )
	  				{
	  					if(pConfParty->GetPartyState() == PARTY_CONNECTING || pConfParty->GetPartyState() == PARTY_CONNECTED_PARTIALY || pConfParty->GetPartyState() == PARTY_CONNECTED || pConfParty->GetPartyState() == PARTY_CONNECTED_WITH_PROBLEM ||  pConfParty->GetPartyState() == PARTY_SECONDARY )
	  						m_isAvMcuConnected = TRUE;

	  					break;
	  				}
	  			}
	  			pConfParty = m_pCommConf->GetNextParty();
	  		}


	  		if(m_isAvMcuConnected == FALSE || IsValidTimer(AUTOTERMINATE))
	  		{
	  			return;
	  		}
				//now we know we have an av-mcu connected
	  		bool ignoreItpSlaves = true;

	  		WORD numOfNonDisconnectedParties = 0;
	  		BYTE areAllDisconnected          = AreAllPartiesDisconnected(numOfNonDisconnectedParties, ignoreItpSlaves);
	  		if(numOfNonDisconnectedParties == 1 /*RMX conf has only AV-MCU link connected*/)
	  		{
				PTRACE2INT(eLevelError, "CConf::ActivateAutoTerminationForMsAvMcu - start auto av-mcu termination! ",m_AutoTerminateAfterLastQuit);
				DelParty(pConfParty->GetName());

	  		}
	}
 }



///////////////////////////////////////////////////////////////
//
//			GATEWAY FUNCTIONS
//
///////////////////////////////////////////////////////////////
void CConf::AddPartyMsgOnScreenForGWConf(PartyMonitorID partyId, const char* msgStr, DWORD timeout, BYTE allowOverride, BYTE displayImmediately)
{
	std::ostringstream msg;
	msg << "PartyMonitorId:" << partyId << ", Message:'" << msgStr << "', Timeout:" << timeout << ", AllowOverride:" << (int)allowOverride;

	if (AreTextMessagesNeededForThisParty(partyId, msg))
		m_pTextOnScreenMngrForGwSession->PrepareMessage(partyId, msgStr, timeout, allowOverride, displayImmediately);

	TRACEINTO << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CConf::AreTextMessagesNeededForThisParty(DWORD partyId, std::ostringstream& msg)
{
	if (IsGWPartyInSetupStage(partyId, msg))
	{
		BOOL disableTextMessages = GetSystemCfgFlagInt<BOOL>(CFG_KEY_DISABLE_GW_OVERLAY_INDICATION);
		if (disableTextMessages)
			msg << "\nDISABLE_GW_OVERLAY_INDICATION = YES";
		else
		{
			if (m_pTextOnScreenMngrForGwSession)
				return true;
			else
				msg << "\nm_pTextOnScreenMngrForGwSession is Not valid";
		}
	}

	msg << "\nText messages are not needed for this party";

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CConf::IsGWPartyInSetupStage(DWORD partyId, std::ostringstream& msg)
{
	CSmallString details;
	if (m_isGateWay)
	{
		if (!GwPartiesStateFlag) // not all parties finished setup
		{
			if (m_pGWPartiesState)
			{
				GW_PARTIES_STATUS::iterator it = m_pGWPartiesState->find(partyId);
				if (it != m_pGWPartiesState->end())
				{// this party is part of the gw dial out sequence
					if ((*m_pGWPartiesState)[partyId] == GW_SETUP) // this party
					{
						return true;
					}
					else
					{
						details << "party finished SETUP stage";
					}
				}
				else
				{
					details << "can't find party Id in m_pGWPartiesState map";
				}
			}
			else
			{
				details << "m_pGWPartiesState is Not valid!!!";
			}
		}
		else
		{
			details << "GwPartiesStateFlag is 1 - All parties finished setup";
		}
	}
	else
	{
		details << "Not Gateway Session!!!";
	}

	msg << "\nparty is not at setup stage detailes: " << details;

	return false;
}

////////////////////////////////////////////////////////////////////////////
void CConf::AddPartyMsgOnScreenForInvitedConf(PartyMonitorID partyId, const char* msgStr, DWORD timeout, BYTE allowOverride, BYTE displayImmediately)
{
	std::ostringstream msg;
	msg << "Message:'" << msgStr << "'" << ", Timeout:" << timeout << ", AllowOverride:" << (int)allowOverride << ", ";

	if (AreTextMessagesNeededForThisInvitedParty(partyId, msg))
		m_pTextOnScreenMngrForInvitedSession->PrepareMessage(partyId, msgStr, timeout, allowOverride, displayImmediately);

	TRACEINTO << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
bool CConf::AreTextMessagesNeededForThisInvitedParty(PartyMonitorID partyId, std::ostringstream& msg)
{
	if (IsInvitedPartyInSetupStage(partyId, msg))
	{
		BOOL disableTextMessages = GetSystemCfgFlagInt<BOOL>(CFG_KEY_DISABLE_GW_OVERLAY_INDICATION);
		if (disableTextMessages)
			msg << "\nDISABLE_GW_OVERLAY_INDICATION = YES";
		else
		{
			if (m_pTextOnScreenMngrForInvitedSession)
				return true;
			else
				msg << "\nm_pTextOnScreenMngrForInvitedSession is Not valid";
		}
	}

	msg << "\nText messages are not needed for this party";

	return false;
}

////////////////////////////////////////////////////////////////////////////
bool CConf::IsInvitedPartyInSetupStage(PartyMonitorID partyId, std::ostringstream& msg)
{
	msg << "PartyMonitorId:" << partyId;

	if (!m_isDtmfInviteParty)
	{
		msg << " - Not Invited session";
		return false;
	}
	if (m_invitedPartiesStateFlag)
	{
		msg << " - All parties finished SETUP stage";
		return false;
	}
	if (!m_pInvitedPartiesState)
	{
		msg << " - Failed, Invalid object";
		return false;
	}

	INVITED_PARTIES_STATUS::iterator it = m_pInvitedPartiesState->find(partyId);
	if (it != m_pInvitedPartiesState->end())
	{
		// this party is part of the gw dial out sequence
		if ((*m_pInvitedPartiesState)[partyId] == GW_SETUP) // this party
		{
			msg << " - Party not finished SETUP stage";
			return true;
		}
		else
		{
			msg << " - Party finished SETUP stage";
			return false;
		}
	}
	msg << " - Failed, Can't find party Id in m_pInvitedPartiesState map";
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SendPartyMsgOnScreenForGWConf()
{
	PASSERT_AND_RETURN(GwPartiesStateFlag);

	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (!pPartyConnection)
			continue;

		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		if (!pPartyCntl)
			continue;

		if (pPartyCntl->GetDialType() == DIALIN && pPartyCntl->GetmIsMemberInVidBridge())
		{
			PartyRsrcID partyRsrcID = pPartyCntl->GetPartyRsrcId();

			DWORD numLines = m_pTextOnScreenMngrForGwSession->Size();
			char** pDisplayStringArr = new char*[MAX_TEXT_LEN];
			m_pTextOnScreenMngrForGwSession->TextMsgListToCharArray(pDisplayStringArr);

			CMedString displayStr;
			displayStr << "CConf::SendPartyMsgOnScreenForGWConf\n" << "party name: " << pPartyCntl->GetName() << "\n" << "partyRsrcId: " << partyRsrcID << "\n";
			for (DWORD j = 0; j < numLines; j++)
				displayStr << "message " << j << ":" << pDisplayStringArr[j] << "\n";
			PTRACE(eLevelInfoNormal, displayStr.GetString());

			if (numLines)
				m_pVideoBridgeInterface->DisplayPartyTextListOnScreen(partyRsrcID, m_pTextOnScreenMngrForGwSession, 0);

			for (DWORD k = 0; k < numLines; k++)
				delete[] pDisplayStringArr[k];

			delete[] pDisplayStringArr;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CConf::SendPartyMsgOnScreenForDtmfInvitePartyConf(DWORD partyId)
{
	PASSERT_AND_RETURN(m_invitedPartiesStateFlag);

	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (!pPartyConnection)
			continue;

		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		if (!pPartyCntl)
			continue;

		if (m_invitePartyMap.find(pPartyCntl->GetPartyId()) == m_invitePartyMap.end())
			continue;

		if (pPartyCntl->GetmIsMemberInVidBridge())
		{
			PartyRsrcID partyRsrcID = pPartyCntl->GetPartyRsrcId();

			DWORD numLines = m_pTextOnScreenMngrForInvitedSession->Size();
			char** pDisplayStringArr = new char*[MAX_TEXT_LEN];
			m_pTextOnScreenMngrForInvitedSession->TextMsgListToCharArray(pDisplayStringArr);

			CMedString displayStr;
			displayStr << "CConf::SendPartyMsgOnScreenForDtmfInvitePartyConf\n" << "party name: " << pPartyCntl->GetName() << "\n" << "partyRsrcId: " << partyRsrcID << "\n";
			for (DWORD j = 0; j < numLines; j++)
				displayStr << "message " << j << ":" << pDisplayStringArr[j] << "\n";
			PTRACE(eLevelInfoNormal, displayStr.GetString());

			if (numLines)
				m_pVideoBridgeInterface->DisplayPartyTextListOnScreen(partyRsrcID, m_pTextOnScreenMngrForInvitedSession, 0);

			for (DWORD k = 0; k < numLines; k++)
				delete[] pDisplayStringArr[k];

			delete[] pDisplayStringArr;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::CalcGwPartiesStateFlag()
{
	if(GwPartiesStateFlag)
		return;

	BYTE flag = 1;
	GW_PARTIES_STATUS::iterator it;
	for (it = m_pGWPartiesState->begin();it != m_pGWPartiesState->end() && flag ;++it)
			flag &= it->second;

	GwPartiesStateFlag = flag;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::GenerateGeneralDisconnectionCause(CConfParty* pConfParty,eGeneralDisconnectionCause& discCauseForGW)
{

	#define  causeNO_ROUTE_TO_DEST	   3
	#define  causeNO_ANSWER_FRM_USR   19
	//it's better to define in Q931Structs.h

	switch(pConfParty->GetDisconnectCause())
	{
		case caus_USER_BUSY_VAL:
		case H323_CALL_CLOSED_REMOTE_BUSY:
		case SIP_BUSY_HERE:
		case SIP_BUSY_EVERYWHERE:
		{
			discCauseForGW = eRemoteBusy;
			break;
		}
		// ISDN Wrong Number causes
		case causeNO_ROUTE_TO_DEST:
		case causNO_USER_RSP_V:
		case causINVALID_NUM_VAL:
		case causTMP_FAILURE_VAL:
		// H323 Wrong Number causes
		case H323_CALL_CLOSED_REMOTE_UNREACHABLE:
		// SIP Wrong Number causes
		case SIP_CLIENT_ERROR_484:
		case SIP_NOT_FOUND:
		case SIP_CLIENT_ERROR_414:
		case SIP_CLIENT_ERROR_420:
		case SIP_UNSUPPORTED_URI_SCHEME:
		case SIP_EXTENSION_REQUIRED:

		{
			discCauseForGW = eRemoteWrongNumber;
			break;
		}
		case causeNO_ANSWER_FRM_USR:
		case H323_CALL_CLOSED_REMOTE_NO_ANSWER:
		case SIP_REMOTE_NO_ANSWER:

		{
			discCauseForGW = eRemoteNoAnswer;
			break;
		}
		default:
		{
			discCauseForGW = eRemoteFailed;
			break;
		}
	}

}


void CConf::GenerateGeneralDisconnectionMessageFromCause(CConfParty* pConfParty,eGeneralDisconnectionCause discCauseForGW,CMedString& cstr)
{

	CSmallString prefixStr;
	// in case there is only one message omit the number to avoid cut messages
	BYTE omitNumber = m_pTextOnScreenMngrForGwSession && m_pTextOnScreenMngrForGwSession->GetNumOfTextLines() == 1;

	mcTransportAddress partyAddr = pConfParty->GetIpAddress();
    int isIpNonValid = ::isIpTaNonValid(&partyAddr);
	int isIpZero  = ::isApiTaNull(&partyAddr);

	ALLOCBUFFER(ipAdrStr,20);
	switch (pConfParty->GetNetInterfaceType())
	{
		case(H323_INTERFACE_TYPE):
		{
			if (!omitNumber)
			{
				if ((!isIpNonValid)&&(!isIpZero)/*ipAdrDword*/)
				{
					if(eIpVersion4 == partyAddr.ipVersion)
					{
						SystemDWORDToIpString(partyAddr.addr.v4.ip, ipAdrStr);
					}
					else if(eIpVersion6 == partyAddr.ipVersion)
					{
						ipV6ToString(partyAddr.addr.v6.ip, ipAdrStr, TRUE);
					}
					prefixStr << ipAdrStr;
				}
				else
				{
					prefixStr << pConfParty->GetH323PartyAlias();
				}
			}
			break;
		}
		case(SIP_INTERFACE_TYPE):
		{
			if (!omitNumber)
			{
				if ((!isIpNonValid)&&(!isIpZero)/*ipAdrDword*/)
				{
					if(eIpVersion4 == partyAddr.ipVersion)
					{
						SystemDWORDToIpString(partyAddr.addr.v4.ip, ipAdrStr);
					}
					else if(eIpVersion6 == partyAddr.ipVersion)
					{
						ipV6ToString(partyAddr.addr.v6.ip, ipAdrStr, TRUE);
					}
					prefixStr << ipAdrStr;
				}
				else
				{
					prefixStr << pConfParty->GetSipPartyAddress();
				}
			}
			break;
		}
		case(ISDN_INTERFACE_TYPE):
		{
			if (!omitNumber)
				prefixStr << pConfParty->GetPhoneNumber();

			break;
		} // case ISDN
	} //switch
	DEALLOCBUFFER(ipAdrStr);

	prefixStr.ReplaceChar('@','\0');
	cstr << prefixStr.GetString();

	if (discCauseForGW != eRemoteBusy)
		cstr << " call ";
	else
		cstr << " is ";

	cstr << ::GetGeneralDisconnectionCause(discCauseForGW);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::GenerateGeneralDialingMessage(CRsrvParty* pParty, CSmallString& cstr)
{
	CSmallString aliasName = "";
	WORD callingInterfaceType;
	cstr << "Dialing ";

	callingInterfaceType = pParty->GetNetInterfaceType();
	switch (callingInterfaceType)
	{
	case(H323_INTERFACE_TYPE):
	{
		aliasName = pParty->GetH323PartyAlias();
		break;
	}
	case(SIP_INTERFACE_TYPE):
	{
		aliasName = pParty->GetSipPartyAddress();
		break;
	}
	case(ISDN_INTERFACE_TYPE):
	{
		aliasName = pParty->GetPhoneNumber();
		break;
	}
	}//switch

	if(aliasName == "" && (callingInterfaceType == H323_INTERFACE_TYPE || callingInterfaceType == SIP_INTERFACE_TYPE))
	{
		char tempName[64];
		memset (&tempName,'\0',IPV6_ADDRESS_LEN);

		ipToString(pParty->GetIpAddress(), tempName, 1);
		aliasName = tempName;
	}
	cstr << aliasName;
}

////////////////////////////////////////////////////////////////////////////
void CConf::GenerateGeneralConnectedMessage(CConfParty* pConfParty, CSmallString& cstr)
{
	mcTransportAddress partyAddr = pConfParty->GetIpAddress();

	int isIpNonValid = ::isIpTaNonValid(&partyAddr);
	int isIpZero  = ::isApiTaNull(&partyAddr);

	ALLOCBUFFER(ipAdrStr,20);
	switch (pConfParty->GetNetInterfaceType())
	{
		case(H323_INTERFACE_TYPE):
		{
			if ((!isIpNonValid)&&(!isIpZero))
			{
				if(eIpVersion4 == partyAddr.ipVersion)
				{
					SystemDWORDToIpString(partyAddr.addr.v4.ip, ipAdrStr);
				}
				else if(eIpVersion6 == partyAddr.ipVersion)
				{
					ipV6ToString(partyAddr.addr.v6.ip, ipAdrStr, TRUE);
				}
				cstr << ipAdrStr;
			}
			else
			{
				cstr << pConfParty->GetH323PartyAlias();
			}
			break;
		}
		case(SIP_INTERFACE_TYPE):
		{
			if ((!isIpNonValid)&&(!isIpZero)/*ipAdrDword*/)
			{
				if(eIpVersion4 == partyAddr.ipVersion)
				{
					SystemDWORDToIpString(partyAddr.addr.v4.ip, ipAdrStr);
				}
				else if(eIpVersion6 == partyAddr.ipVersion)
				{
					ipV6ToString(partyAddr.addr.v6.ip, ipAdrStr, TRUE);
				}
				cstr << ipAdrStr;
			}
			else
			{
				cstr << pConfParty->GetSipPartyAddress();
			}
			break;
		}
		case(ISDN_INTERFACE_TYPE):
		{
			cstr << pConfParty->GetPhoneNumber();
			break;
		}
	}
	DEALLOCBUFFER(ipAdrStr);

	cstr << " CONNECTED";
}

////////////////////////////////////////////////////////////////////////////
void CConf::GenerateGeneralDisconnectedMessage(CConfParty* pConfParty, CSmallString& cstr)
{
	mcTransportAddress partyAddr = pConfParty->GetIpAddress();

	int isIpNonValid = ::isIpTaNonValid(&partyAddr);
	int isIpZero  = ::isApiTaNull(&partyAddr);

	ALLOCBUFFER(ipAdrStr,20);
	switch (pConfParty->GetNetInterfaceType())
	{
		case(H323_INTERFACE_TYPE):
		{
			if ((!isIpNonValid)&&(!isIpZero))
			{
				if(eIpVersion4 == partyAddr.ipVersion)
				{
					SystemDWORDToIpString(partyAddr.addr.v4.ip, ipAdrStr);
				}
				else if(eIpVersion6 == partyAddr.ipVersion)
				{
					ipV6ToString(partyAddr.addr.v6.ip, ipAdrStr, TRUE);
				}
				cstr << ipAdrStr;
			}
			else
			{
				cstr << pConfParty->GetH323PartyAlias();
			}
			break;
		}
		case(SIP_INTERFACE_TYPE):
		{
			if ((!isIpNonValid)&&(!isIpZero)/*ipAdrDword*/)
			{
				if(eIpVersion4 == partyAddr.ipVersion)
				{
					SystemDWORDToIpString(partyAddr.addr.v4.ip, ipAdrStr);
				}
				else if(eIpVersion6 == partyAddr.ipVersion)
				{
					ipV6ToString(partyAddr.addr.v6.ip, ipAdrStr, TRUE);
				}
				cstr << ipAdrStr;
			}
			else
			{
				cstr << pConfParty->GetSipPartyAddress();
			}
			break;
		}
		case(ISDN_INTERFACE_TYPE):
		{
			cstr << pConfParty->GetPhoneNumber();
			break;
		}
	}
	DEALLOCBUFFER(ipAdrStr);

	cstr << " DISCONNECTED By DisconnectInvitedParticipant Function";
}

////////////////////////////////////////////////////////////////////////////
void CConf::AddNextPartyForGWSession(PartyMonitorID partyId, BYTE discCause, CMedString& discCauseStr, int i)
{
	TRACEINTO << "PartyMonitorId:" << partyId << ", DiscCause:" << (int)discCause << ", Iteration:" << i;

	CRsrvParty* nextParty = m_pDialingSequence->GetNextParty(partyId);

	if (discCause > eRemoteBusy && CPObject::IsValidPObjectPtr(nextParty))
	{
		m_pTextOnScreenMngrForGwSession->AddParty(nextParty->GetPartyId());

		CConfParty tmpConfParty(*nextParty); // create an instance of ongoing party for some external functions
		// validity test??
		DWORD status = TestGWPartyValidity(nextParty);
		// there is next party with bad status
		if (status != STATUS_OK)
		{
			// in case the last disconnection cause is failed
			// create a new default disconnection string with the correct protocol
			// otherwise keep the last disconnection string
			if (discCause > eRemoteWrongNumber)
			{
				eGeneralDisconnectionCause tmpCause = eRemoteFailed;
				GenerateGeneralDisconnectionCause(&tmpConfParty, tmpCause);
				discCause = (BYTE)tmpCause;
				SaveInviteResult(partyId, nextParty->GetNetInterfaceType(), (eGeneralDisconnectionCause)discCause);
				discCauseStr.Clear();
				GenerateGeneralDisconnectionMessageFromCause(&tmpConfParty, m_PartiesInviteResults[partyId].eOverallInviteResult, discCauseStr);
			}
			else
			{
				SaveInviteResult(partyId, nextParty->GetNetInterfaceType(), (eGeneralDisconnectionCause)discCause);
			}

			AddNextPartyForGWSession(partyId, discCause, discCauseStr, i + 1);
		}
		// there is next party with good status
		else
		{
			CSmallString cstr;
			GenerateGeneralDialingMessage(nextParty, cstr);
			// add next party to conf
			AddPartyMsgOnScreenForGWConf(nextParty->GetPartyId(), cstr.GetString());
			// CDR???
			m_pCommConf->NewUndefinedParty(&tmpConfParty, EVENT_NEW_UNDEFINED_PARTY);
			TRACEINTO << "PartyName:" << nextParty->GetName() << ", PartyId:" << nextParty->GetPartyId();

			COstrStream str;
			CRsrvParty* pRsrvParty = new CRsrvParty(*nextParty);
			pRsrvParty->Serialize(NATIVE, str);
			str << (DWORD)0 << "\n";
			CSegment* pSeg = new CSegment;
			*pSeg << (DWORD)0;
			*pSeg << str.str().c_str();
			DispatchEvent(ADDPARTY, pSeg);
			POBJDELETE(pRsrvParty);
			POBJDELETE(pSeg);
		}
		POBJDELETE(nextParty);
	}
	// no more parties to dial
	else
	{

		BOOL b_RetryProcessed = FALSE;
		DWORD numDialParty = m_pDialingSequence->GetDialMap()->size();
		TRACEINTO << "numDialParty:" << numDialParty;
		//Dial retry only works for single destination
		if (numDialParty < 2)
		{
			b_RetryProcessed = HandlePartyDialRetry(partyId);
		}

		if (!discCauseStr.IsEmpty()) // protection - should not happen
		{
			AddPartyMsgOnScreenForGWConf(partyId, discCauseStr.GetString(), 5 * SECOND, FALSE, TRUE);
		}
		else
		{
			PASSERT(1);
		}
		GateWayPartyEndSetup(partyId, b_RetryProcessed);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CConf::HandlePartyDialRetry(PartyMonitorID partyId)
{
	TRACEINTO << "PartyMonitorId:" << partyId;

	BOOL b_RetryProcessed = FALSE;

	if (m_bEnableRedialOnWrongNumber)
	{
		PartyMonitorID invitorId = m_PartiesInvitorId[partyId];
		eGeneralDisconnectionCause disconnectionCause;

		CheckInviteResult(partyId, disconnectionCause);

		CPartyApi* pInvitorPartyApi = NULL;

		CConfParty* pInvitorConfParty = m_pCommConf->GetCurrentParty(invitorId);
		if (pInvitorConfParty)
		{
			CPartyConnection* pInvitorPartyConnection = GetPartyConnection(pInvitorConfParty->GetName());
			if (!CPObject::IsValidPObjectPtr(pInvitorPartyConnection))
			{
				TRACEINTOLVLERR << "PartyMonitorId:" << invitorId << " - Failed, Inviter party connection was not found";
			}
			else
			{
				TRACEINTO << "PartyMonitorId:" << partyId << ", PartyName:" << pInvitorConfParty->GetName();

				pInvitorPartyApi = pInvitorPartyConnection->GetPartyApi();
				if (!CPObject::IsValidPObjectPtr(pInvitorPartyApi))
					pInvitorPartyApi = NULL;
			}
		}

		if (eRemoteWrongNumber == disconnectionCause)
		{
			DWORD leftWrongNumberDialRetries = m_PartiesRedialNum[invitorId];

			TRACEINTO << "leftWrongNumberDialRetries:" << leftWrongNumberDialRetries;

			if (pInvitorPartyApi)
			{
				if (leftWrongNumberDialRetries)
				{
					leftWrongNumberDialRetries--;
					m_PartiesRedialNum[invitorId] = leftWrongNumberDialRetries;
					b_RetryProcessed = TRUE;  //retry is started

					if (m_PartiesInviteType[invitorId] == eInviteTypeGw)
					{
						pInvitorPartyApi->SendPartyInviteResultInd(disconnectionCause, b_RetryProcessed, TRUE);
					}
					else
					{
						pInvitorPartyApi->SendPartyInviteResultInd(disconnectionCause, b_RetryProcessed, FALSE);
					}
				}
				else  //no need to retry
				{
					if (m_PartiesInviteType[invitorId] == eInviteTypeGw)
					{
						pInvitorPartyApi->SendPartyInviteResultInd(disconnectionCause, b_RetryProcessed, TRUE);
					}
				}

			}
		}
		else // no answer, busy or fail; no need to retry
		{
			if (pInvitorPartyApi)
			{
				if (m_PartiesInviteType[invitorId] == eInviteTypeGw)
				{
					pInvitorPartyApi->SendPartyInviteResultInd(disconnectionCause, b_RetryProcessed, TRUE);
				}
			}
		}

		if (!b_RetryProcessed)
		{
			m_PartiesRedialNum.erase(invitorId);
			m_PartiesInviteType.erase(invitorId);
		}
	}

	return b_RetryProcessed;
}


////////////////////////////////////////////////////////////////////////////
DWORD CConf::TestGWPartyValidity(CRsrvParty* pRsrvParty)
{
	DWORD status = STATUS_OK;
	CLargeString cstr;
	cstr << "GATEWAY_LOG: CConf::TestValidityAndAddPartyToGW";
	DWORD confStat = m_pCommConf->GetStatus();
	if(confStat & CONFERENCE_RESOURCES_DEFICIENCY)
	{
		cstr << " Insufficient Resources";
		status = STATUS_INSUFFICIENT_RSRC;
	}

	WORD confNumParts = m_pCommConf->GetNumParties();
	if (STATUS_OK == status)
	{
		WORD confMaxParts = m_pCommConf->GetMaxParties();
		if(m_pCommConf->IncludeRecordingParty())
			confNumParts = confNumParts -1; //we don't include the recording party in the max participants limitation.
		if ((confMaxParts <= confNumParts) && (confMaxParts != 255) && !pRsrvParty->GetRecordingLinkParty())
		{
			cstr << " Conference has already : " << m_pCommConf->GetNumParties()<< ", max parties allowed is: "<< confMaxParts;
			status=STATUS_NUMBER_OF_PARTICIPANTS_EXCEEDS_THE_DEFINED_MAXIMUM_FROM_PROFILE;
		}
	}

	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	WORD maxPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfPartiesInConf();
	BYTE isAudioOnlyParty = pRsrvParty->GetVoice();

	if (confNumParts>=maxPartiesInConfPerSystemMode)
		status =  STATUS_PARTIES_NUMBER_LIMIT_EXCEEDED;
	if(!isAudioOnlyParty)
	{
		WORD confNumVideoParts = m_pCommConf->GetNumVideoParties();
		WORD maxVideoPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf();
		if(confNumVideoParts>=maxVideoPartiesInConfPerSystemMode)
		{
			cstr << " max video participants exceeded Name : " << pRsrvParty->GetName();
			status = STATUS_PARTIES_NUMBER_LIMIT_EXCEEDED;
		}
	}

	if (STATUS_OK == status)
	{
	  //Only in case of DialOut it should be blocked.
	  if(m_pCommConf->IsConfSecured() && pRsrvParty->GetConnectionType() == DIAL_OUT)
      {
		cstr << " - can not be add any parties to the Secured conference: ";
		status=STATUS_ADD_PARTICIPANT_TO_SECURE_CONF;
      }
	}

	if(STATUS_OK == status)
		status = m_pCommConf->TestPartyRsrvValidity(pRsrvParty);

	if(STATUS_OK == status)
		status = TestPartyNameValidity(pRsrvParty);

	cstr << "   status = " << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
	PTRACE(eLevelInfoNormal,cstr.GetString());
	return status;
}

////////////////////////////////////////////////////////////////////////////
DWORD CConf::TestDtmfInvitedPartyValidity(CRsrvParty* pRsrvParty)
{
	DWORD status = STATUS_OK;
	CLargeString cstr;
	cstr << "CConf::TestDtmfInvitedPartyValidity";
	DWORD confStat = m_pCommConf->GetStatus();
	if(confStat & CONFERENCE_RESOURCES_DEFICIENCY)
	{
		cstr << " Insufficient Resources";
		status = STATUS_INSUFFICIENT_RSRC;
	}

	WORD confNumParts = m_pCommConf->GetNumParties();
	if (STATUS_OK == status)
	{
		WORD confMaxParts = m_pCommConf->GetMaxParties();
		if(m_pCommConf->IncludeRecordingParty())
			confNumParts = confNumParts -1; //we don't include the recording party in the max participants limitation.
		if ((confMaxParts <= confNumParts) && (confMaxParts != 255) && !pRsrvParty->GetRecordingLinkParty())
		{
			cstr << " Conference has already : " << m_pCommConf->GetNumParties()<< ", max parties allowed is: "<< confMaxParts;
			status=STATUS_NUMBER_OF_PARTICIPANTS_EXCEEDS_THE_DEFINED_MAXIMUM_FROM_PROFILE;
		}
	}

	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	WORD maxPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfPartiesInConf();
	BYTE isAudioOnlyParty = pRsrvParty->GetVoice();

	if (confNumParts>=maxPartiesInConfPerSystemMode)
		status =  STATUS_PARTIES_NUMBER_LIMIT_EXCEEDED;
	if(!isAudioOnlyParty)
	{
		WORD confNumVideoParts = m_pCommConf->GetNumVideoParties();
		WORD maxVideoPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf();
		if(confNumVideoParts>=maxVideoPartiesInConfPerSystemMode)
		{
			cstr << " max video participants exceeded Name : " << pRsrvParty->GetName();
			status = STATUS_PARTIES_NUMBER_LIMIT_EXCEEDED;
		}
	}

	if (STATUS_OK == status)
	{
	  //Only in case of DialOut it should be blocked.
	  if(m_pCommConf->IsConfSecured() && pRsrvParty->GetConnectionType() == DIAL_OUT)
      {
		cstr << " - can not be add any parties to the Secured conference: ";
		status=STATUS_ADD_PARTICIPANT_TO_SECURE_CONF;
      }
	}

	if(STATUS_OK == status)
		status = m_pCommConf->TestPartyRsrvValidity(pRsrvParty);

	if(STATUS_OK == status)
		status = TestPartyNameValidity(pRsrvParty);

	cstr << "   status = " << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
	PTRACE(eLevelInfoNormal,cstr.GetString());
	return status;
}

////////////////////////////////////////////////////////////////////////////
void CConf::GateWayPartyEndSetup(DWORD partyId,BOOL bRedialProcessed)
{
	PTRACE2INT(eLevelInfoNormal,"CConf::GateWayPartyEndSetup , party id: ",partyId );

	// update the local status map
	if(m_pGWPartiesState != NULL)
	{
		(*m_pGWPartiesState)[partyId] = GW_CONNECT;
	}

	if ( m_pConfAppMngrInterface )
	{
		CSegment*  pMsg = new CSegment;
		*pMsg << (DWORD)EVENT_NOTIFY
			  << (OPCODE)eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE
			  << INVALID;//we always send conf msgs to cam with invalid instead of partyRsrcId

		m_pConfAppMngrInterface->HandleEvent(pMsg);
		POBJDELETE(pMsg);
	}

	if (!bRedialProcessed)
	{
		// after 5 seconds (time for user visual indication) check if all parties connected/disconnected
		StartTimer(ENDGWSETUP,5*SECOND);
	}

}

////////////////////////////////////////////////////////////////////////////
void CConf::InvitedPartyEndSetup(PartyMonitorID partyId, BOOL bRedialProcessed)
{
	TRACEINTO << "PartyMonitorId:" << partyId;

	// update the local status map
	if (m_pInvitedPartiesState != NULL)
	{
		(*m_pInvitedPartiesState)[partyId] = GW_CONNECT;
	}

	if (m_pConfAppMngrInterface)
	{
		CSegment* pMsg = new CSegment;
		*pMsg
			<< (DWORD)EVENT_NOTIFY
			<< (OPCODE)eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE
			<< INVALID; //we always send conf msgs to cam with invalid instead of partyRsrcId

		m_pConfAppMngrInterface->HandleEvent(pMsg);
		POBJDELETE(pMsg);
	}

	if (!bRedialProcessed)
	{
		// after 5 seconds (time for user visual indication) check if all parties connected/disconnected
		StartTimer(ENDDTMFINVITEPARTYSETUP, 5 * SECOND);
	}
}

////////////////////////////////////////////////////////////////////////////
void CConf::InitPartiesFromString(CConfPartiesDialingSequence* pConfPartiesDialingSequence, CCommRes* pMR, char* dialString)
{
	CLargeString cstr;
	cstr << "DialString:" << dialString;

	while (dialString != NULL) // && IsContainingNumbers(dialString)): VNGR 25970-SIP GW calls fail if SIP party = Alpha
	{
		ALLOCBUFFER(partyNumOrIp, 256);
		memset(partyNumOrIp, '\0', 256);

		// get the number until the first *
		BYTE isPstn = ParseString(partyNumOrIp, dialString, NO);
		// create parties with different protocols

		cstr << "\n";
		AddPartyDialingSequenceForGw(pConfPartiesDialingSequence, pMR, partyNumOrIp, isPstn, cstr);

		DEALLOCBUFFER(partyNumOrIp);
	}

	TRACEINTO << cstr.GetString();
}

////////////////////////////////////////////////////////////////////////////
BYTE CConf::ParseString(char* partyNumOrIp, char*& dialString, BYTE isPstn)
{
	char* pch;
	int dialStringLen = strlen(dialString);
	pch=strchr(dialString,'*');

	if (pch != NULL)
	{
	  	dialStringLen = pch - dialString;
		strncpy(partyNumOrIp,dialString,dialStringLen);
		dialString = pch+1;

	}
	else
	{
		strncpy(partyNumOrIp,dialString,dialStringLen);
		dialString = NULL;
	}

	if (dialStringLen)
	{
		return isPstn;
	}
	else
	{
		return ParseString(partyNumOrIp,dialString,YES);
	}
}

////////////////////////////////////////////////////////////////////////////
BYTE CConf::IsContainingNumbers(char* stringToCheck)
{
	char keys[] = "1234567890";
	DWORD i = strcspn (stringToCheck,keys);
	if (i != strlen(stringToCheck))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////
void CConf::ReplaceChar(char* str, char charToReplace, char newChar)
{
	size_t n = strlen(str);
	for(size_t i = 0; i < n; i++)
	{
		if(str[i] == charToReplace)
			str[i] = newChar;
	}
}

////////////////////////////////////////////////////////////////////////////
DWORD CConf::AddPartyDialingSequenceForGw(CConfPartiesDialingSequence* pConfPartiesDialingSequence, CCommRes* pMR, char* dialString,BYTE isPstn,CLargeString& cstr)
{
	PARTY_SEQUENCE_LIST* pList  = new PARTY_SEQUENCE_LIST;
	DWORD status = 0;

	DWORD partyId = pMR->NextPartyId();
	BYTE isH323 = pMR->GetIsGWDialOutToH323();
	BYTE isSip = pMR->GetIsGWDialOutToSIP();
	BYTE isIsdn = pMR->GetIsGWDialOutToH320();
	BYTE isPstnFromApi = pMR->GetIsGWDialOutToPSTN();

	TRACEINTO << "PartyId:" << partyId << ", isH323:" << (WORD)isH323 << ", isSip:" << (WORD)isSip << ", isIsdn:" << (WORD)isIsdn << ", isPstnFromApi:" << (WORD)isPstnFromApi;

	ALLOCBUFFER(ISDNServiceNameStr, NET_SERVICE_PROVIDER_NAME_LEN);
	memset(ISDNServiceNameStr, '\0', NET_SERVICE_PROVIDER_NAME_LEN);
	ALLOCBUFFER(IPServiceNameStr, NET_SERVICE_PROVIDER_NAME_LEN);
	memset(IPServiceNameStr, '\0', NET_SERVICE_PROVIDER_NAME_LEN);

	// get global pointer to service providers list
	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	// VNGR-18702 take 3
	const char* conf_serv_name = pMR->GetServiceNameForMinParties();
	BYTE netInterfaceType = H323_INTERFACE_TYPE;
	if (isSip)
	{
		netInterfaceType = SIP_INTERFACE_TYPE;
	}

	CConfIpParameters* pIpParams = ::GetIpServiceListMngr()->GetRelevantService(conf_serv_name, netInterfaceType);
	// GetRelevantService always return service: by name, default, or first in list
	if (pIpParams == NULL)
	{
		PASSERT(101);
		DEALLOCBUFFER(ISDNServiceNameStr);
		DEALLOCBUFFER(IPServiceNameStr);
		delete pList;

		return INVALID;
	}

	const char* serv_name = (const char*)(pIpParams ? pIpParams->GetServiceName() : NULL);

	CServicePhoneStr* pServicePhone = pMR->GetFirstServicePhone();

	const RTM_ISDN_PARAMS_MCMS_S* pIsdnServiceStruct = pConfPartyProcess->GetIsdnService(serviceName);
	if (pIsdnServiceStruct)
		strncpy(ISDNServiceNameStr, (char*)pIsdnServiceStruct->serviceName, NET_SERVICE_PROVIDER_NAME_LEN);
	else
	{
		isIsdn = FALSE;
		isPstn = FALSE;
	}

	if (serv_name)
	{
		strncpy(IPServiceNameStr, serv_name, NET_SERVICE_PROVIDER_NAME_LEN);
	}

	if (!dialString)
	{
		PAssert(__FILE__, __LINE__, this, !dialString, "CConf::AddPartyDialingSequenceForGw - Received Null Dial String - Cannot Dial Out! partyId = ");

		DEALLOCBUFFER(ISDNServiceNameStr);
		DEALLOCBUFFER(IPServiceNameStr);
		delete pList;

		return partyId;
	}

	if (isPstn)
	{
		if (isPstnFromApi)
		{
			CRsrvParty* pPSTNGateWayParty = new CRsrvParty();

			pPSTNGateWayParty->SetServiceProviderName(ISDNServiceNameStr);
			pPSTNGateWayParty->SetConnectionType(DIAL_OUT);
			pPSTNGateWayParty->SetNetInterfaceType(ISDN_INTERFACE_TYPE);
			ALLOCBUFFER(h243name, H243_NAME_LEN);
			sprintf(h243name, "%s_%s_%d", pMR->GetName(), "PSTN_OUT", partyId);
			pPSTNGateWayParty->SetName(h243name);
			DEALLOCBUFFER(h243name);

			pPSTNGateWayParty->SetVoice(YES);
			pPSTNGateWayParty->SetNetChannelNumber(AUTO);
			pPSTNGateWayParty->SetBondingMode1(AUTO);
			pPSTNGateWayParty->AddCallingPhoneNumber(dialString);
			pPSTNGateWayParty->SetUndefinedType(UNRESERVED_PARTY);
			pPSTNGateWayParty->SetPartyId(partyId);

			cstr << "Adding PSTN party with num: " << dialString << " and status: " << status <<"\n";
			pList->push_back(pPSTNGateWayParty);
		}
	}
	else
	{
		// ////////////////////////////////////////////////////////////////////////////////////////
		// Add H323 Party
		if (isH323)
		{
			CRsrvParty* p323GateWayPartyOut = new CRsrvParty();

			p323GateWayPartyOut->SetConnectionType(DIAL_OUT);
			p323GateWayPartyOut->SetNetInterfaceType(H323_INTERFACE_TYPE);
			p323GateWayPartyOut->SetServiceProviderName(IPServiceNameStr);
			p323GateWayPartyOut->SetUndefinedType(UNRESERVED_PARTY);

			ALLOCBUFFER(h243name, H243_NAME_LEN);
			strncpy(h243name, dialString, H243_NAME_LEN);
			h243name[H243_NAME_LEN - 1] = '\0';

			mcTransportAddress IpAddr;
			memset(&IpAddr, 0, sizeof(mcTransportAddress));

			// if (strchr(dialString,'.') != NULL)  //BRIDGE-2769 - check if it is really an IP address
			if (::isIpV6Str(dialString) || ::IsValidIPAddress(dialString))
			{
				stringToIp(&IpAddr, dialString);
				ReplaceChar(h243name, '.', '_');
			}
			else
			{
				p323GateWayPartyOut->SetH323PartyAliasType(PARTY_H323_ALIAS_E164_TYPE);
				p323GateWayPartyOut->SetH323PartyAlias(dialString);
			}

			p323GateWayPartyOut->SetIpAddress(IpAddr);
			p323GateWayPartyOut->SetCallSignallingPort(1720);

			p323GateWayPartyOut->SetName(h243name);
			DEALLOCBUFFER(h243name);

			p323GateWayPartyOut->SetPartyId(partyId);

			cstr << "Adding H323 party with num: " << dialString << " and status: " << status <<"\n";
			pList->push_back(p323GateWayPartyOut);
		}

		if (isSip)
		{
			CRsrvParty* pSipGateWayPartyOut = new CRsrvParty();

			pSipGateWayPartyOut->SetConnectionType(DIAL_OUT);
			pSipGateWayPartyOut->SetNetInterfaceType(SIP_INTERFACE_TYPE);
			pSipGateWayPartyOut->SetServiceProviderName(IPServiceNameStr);
			pSipGateWayPartyOut->SetUndefinedType(UNRESERVED_PARTY);

			ALLOCBUFFER(h243name, H243_NAME_LEN);
			strncpy(h243name, dialString, H243_NAME_LEN-1);

			mcTransportAddress sipIpAddr;
			memset(&sipIpAddr, 0, sizeof(mcTransportAddress));

			// if (strchr(dialString,'.') != NULL)  //BRIDGE-2769 - check if it is really an IP address
			if (::isIpV6Str(dialString) || ::IsValidIPAddress(dialString))
			{
				stringToIp(&sipIpAddr, dialString);
				ReplaceChar(h243name, '.', '_');
			}
			else
			{
				const char* pDialStrPtr = dialString;

				if (pDialStrPtr)
				{
					char 	fixedDialStr[MaxAddressListSize];
					DWORD 	dialStrLen = 0;

					memset(fixedDialStr, '\0', MaxAddressListSize);

					const char* pDomain2Ptr = strstr(pDialStrPtr, "%40");

					if (pDomain2Ptr)
					{
						const char* pReadPtr2 	= strchr(pDialStrPtr, '@'); //check if dial String arrived without the @domain cut - if so need to cut it.;
						int 		lenOpt		= (pReadPtr2) ? strlen (pReadPtr2): 0;

						// replace %40 with @
						dialStrLen = pDomain2Ptr - pDialStrPtr;
						dialStrLen = ((dialStrLen > 0) && (dialStrLen < MaxAddressListSize)) ? dialStrLen : MaxAddressListSize; //N.A KW 4930

						if( (dialStrLen < (MaxAddressListSize - 2)) && (dialStrLen + strlen(pDomain2Ptr) - lenOpt + 2) < MaxAddressListSize)
						{
							strncpy(fixedDialStr, pDialStrPtr, dialStrLen);
							fixedDialStr[dialStrLen] = '@';

							// skip over "%40" and copy the rest of the string without last @mcu.domain if exist
							pDomain2Ptr += 3;
							strncpy(&fixedDialStr[dialStrLen + 1], pDomain2Ptr, strlen(pDomain2Ptr) - lenOpt);

							fixedDialStr[dialStrLen + strlen(pDomain2Ptr) - lenOpt + 1] = '\0';// N.A. KW 4932, KW 4931

							pSipGateWayPartyOut->SetSipPartyAddress(fixedDialStr);

							//make dialOut Name with _ instead of .
							int namelen = ( strlen(fixedDialStr) < H243_NAME_LEN ) ? strlen(fixedDialStr) : H243_NAME_LEN - 1;//N.A. KW 4933
							strncpy(h243name, fixedDialStr, namelen); //N.A. KW 4933 - copy only to max length of H243 Name
							h243name[namelen] = '\0';
							ReplaceChar(h243name, '.', '_');
							cstr << " N.A. DEBUG Or fixedDialStr: " << fixedDialStr << " h243name = "<<h243name;
						}
						else
						{
							cstr << " Dial String Dialed is incorrent - Too Long  ";
						}

					}
					else
					{
				pSipGateWayPartyOut->SetSipPartyAddress(dialString);
			}
				}
			}

			pSipGateWayPartyOut->SetIpAddress(sipIpAddr);
			pSipGateWayPartyOut->SetCallSignallingPort(5060); // ??

			pSipGateWayPartyOut->SetName(h243name);
			DEALLOCBUFFER(h243name);

			pSipGateWayPartyOut->SetPartyId(partyId);

			cstr << "Adding SIP party with num: " << dialString << " and status: " << status <<"\n";
			pList->push_back(pSipGateWayPartyOut);
		} // sip

		// ////////////////////////////////////////////////////////////////////////////////////////
		// Add ISDN Party
		if (isIsdn)
		{
			CRsrvParty* p320GateWayParty = new CRsrvParty();

			p320GateWayParty->SetServiceProviderName(ISDNServiceNameStr);
			p320GateWayParty->SetConnectionType(DIAL_OUT);
			p320GateWayParty->SetNetInterfaceType(ISDN_INTERFACE_TYPE);
			ALLOCBUFFER(h243name, H243_NAME_LEN);
			sprintf(h243name, "%s_%s_%d", pMR->GetName(), "H320_OUT", partyId);
			p320GateWayParty->SetName(dialString);
			DEALLOCBUFFER(h243name);

			p320GateWayParty->SetNetChannelNumber(AUTO);
			p320GateWayParty->SetBondingMode1(AUTO);
			p320GateWayParty->AddCallingPhoneNumber(dialString);
			p320GateWayParty->SetUndefinedType(UNRESERVED_PARTY);
			p320GateWayParty->SetPartyId(partyId);

			cstr << "Adding ISDN party with num: " << dialString << " and status: " << status <<"\n";
			pList->push_back(p320GateWayParty);
		} // isdn

	} // not pstn

	if (!pList->empty())
		pConfPartiesDialingSequence->AddPartyListToMap(partyId, pList);
	else
	{
		CMedString errorStr;
		errorStr << "CConf::AddPartyDialingSequenceForGw, PARTY LIST IS EMPTY!!!, dialString: " << dialString << "\n";
		errorStr << "isH323: " << isH323 << "\n";
		errorStr << "isSip: " << isSip << "\n";
		errorStr << "isIsdn: " << isIsdn << "\n";
		errorStr << "isPstn (string started with *): " << isPstn << " , isPstnFromApi: " << isPstnFromApi;
		PTRACE(eLevelInfoNormal, errorStr.GetString());
		delete pList;
	}

	DEALLOCBUFFER(ISDNServiceNameStr);
	DEALLOCBUFFER(IPServiceNameStr);
	return partyId;
}

////////////////////////////////////////////////////////////////////////////
DWORD CConf::TestPartyNameValidity(CRsrvParty* pParty)
{
	DWORD status = STATUS_OK;
	const char* party_name = pParty->GetName();
	DWORD confId = m_pCommConf->GetMonitorConfId();


	status = ::GetpConfDB()->SearchPartyName(confId,party_name);
	if (status == STATUS_OK)
		status = STATUS_PARTY_NAME_EXISTS;
	else if (status == STATUS_PARTY_DOES_NOT_EXIST)
			status = STATUS_OK;

	if(STATUS_OK == status)//if the party name is identical to visual name that already exist
	{
		status = ::GetpConfDB()->SearchPartyVisualName(confId,party_name);
		if (status == STATUS_OK)
		  	status = STATUS_PARTY_NAME_EXISTS;
		else if (status == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS)
			  	status = STATUS_OK;
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
BOOL CConf::IsAlwaysForwardDtmfInGWSessionToIsdn()
{
  BOOL isAlwaysForwardDtmfInGWSessionToIsdn = NO;
  CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
  if (pSysConfig)
    pSysConfig->GetBOOLDataByKey("ALWAYS_FORWARD_DTMF_IN_GW_SESSION_TO_ISDN", isAlwaysForwardDtmfInGWSessionToIsdn);

  return isAlwaysForwardDtmfInGWSessionToIsdn;
}

////////////////////////////////////////////////////////////////////////////
void CConf::SetGwDtmfForwardIfNeeded()
{
	if (m_state == TERMINATION)
		return;

	// GW DTMF forwarding is only if there are 2 parties and one of them is link
	// We forward DTMFs from the non-link party to the link

	WORD numOfParties = m_pPartyList->entries();
	BOOL isAlwaysForwardDtmfInGWSessionToIsdn = IsAlwaysForwardDtmfInGWSessionToIsdn();

	TRACEINTO << "ConfName:" << m_name << ", NumOfParties:" << numOfParties << ", IsAlwaysForwardDtmfInGWSessionToIsdn:" << (int)isAlwaysForwardDtmfInGWSessionToIsdn;
	if (numOfParties != 2 && CPObject::IsValidPObjectPtr(m_pGwDtmfForwarderConnection))
	{
		TRACEINTO << "Set GW with link DTMF forwarding to OFF";
		CPartyApi* pPartyApi = m_pGwDtmfForwarderConnection->GetPartyApi();
		if (pPartyApi)
			pPartyApi->SetDTMFForwarding(FALSE, SET_GW_DTMF_FORWARD);
		m_pGwDtmfForwarderConnection = NULL;
	}
	if (numOfParties >= 2 && isAlwaysForwardDtmfInGWSessionToIsdn)
	{
		// On IVRCntl we forward DTMFs only to the ISDNs if ALWAYS_FORWARD_DTMF_IN_GW_SESSION_TO_ISDN is YES
		PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
		for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
		{
			CPartyConnection* pPartyConnection = _itr->second;
			if (!pPartyConnection)
				continue;

			CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
			if (!pPartyCntl)
				continue;

			CPartyApi* pPartyApi = pPartyConnection->GetPartyApi();
			if (!pPartyApi)
			{
				PASSERTSTREAM(1, pPartyCntl->GetName() << " - Invalid PartyApi");
				continue;
			}
			TRACEINTO << "PartyName:" << pPartyConnection->GetName() << " - Set party to forward DTMFs";
			pPartyApi->SetDTMFForwarding(TRUE, SET_GW_DTMF_FORWARD);
		}
	}
	else if (numOfParties == 2)
	{
		// linkPartyType is CASCADE_NONE, CASCADE_GW or CASCADE_MCU.
		// We want to forward only in the case where we have a link to GW or MCU and a participant of a lower cascade type:
		// MCU and GW/Non-link
		// GW and Non Link

		// Find link to conference or GW if exist
		BYTE linkPartyType = CASCADE_NONE;

		PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
		for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
		{
			CPartyConnection* pPartyConnection = _itr->second;
			if (!pPartyConnection)
				continue;

			CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
			if (!pPartyCntl)
				continue;

			linkPartyType = max(linkPartyType, pPartyCntl->GetPartyCascadeType());
		}
		if (linkPartyType == CASCADE_NONE)
			return;

		for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
		{
			CPartyConnection* pPartyConnection = _itr->second;
			if (!pPartyConnection)
				continue;

			CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
			if (!pPartyCntl)
				continue;

			if (pPartyCntl->GetPartyCascadeType() < linkPartyType)
			{
				m_pGwDtmfForwarderConnection = pPartyConnection;
				CPartyApi* pPartyApi = m_pGwDtmfForwarderConnection->GetPartyApi();
				if (!pPartyApi)
				{
					PASSERTSTREAM(1, pPartyCntl->GetName() << " - Invalid PartyApi");
					continue;
				}
				TRACEINTO << "PartyName:" << pPartyConnection->GetName() << " - The party is not a link, set it to forward DTMFs";
				pPartyApi->SetDTMFForwarding(TRUE, SET_GW_DTMF_FORWARD);
			}
			else
			{
				TRACEINTO << "PartyName:" << pPartyConnection->GetName() << " - The party is a link, set it to forward DTMFs";
			}
		}
	}
	if (!CPObject::IsValidPObjectPtr(m_pGwDtmfForwarderConnection))
		m_pGwDtmfForwarderConnection = NULL;
}

////////////////////////////////////////////////////////////////////////////
void CConf::SetDtmfForwardAllIfNeeded()
{
	// DTMF forwarding - we forward DTMFs from all the non-link parties to the link

	DWORD dtmfFwdAllTime = 0; // 1 minutes as default
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if (pSysConfig)
		pSysConfig->GetDWORDDataByKey(CFG_KEY_DTMF_FORWARD_ANY_DIGIT_TIMER_SECONDS, dtmfFwdAllTime);

	// VNGFE-8096: When the conference is locked, the DTMF FW timer should not be set
	if (dtmfFwdAllTime > 0 && !m_pCommConf->IsConfSecured())
	{
		TRACEINTO << "ConfName:" << m_name << ", NumOfParties:" << m_pPartyList->entries();

		BYTE linkPartyType = CASCADE_NONE;

		PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
		for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
		{
			CPartyConnection* pPartyConnection = _itr->second;
			if (!pPartyConnection)
				continue;

			CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
			if (!pPartyCntl)
				continue;

			if (CASCADE_NONE == pPartyCntl->GetPartyCascadeType()) //All parties are going to forward their DTMFs
			{
				CPartyApi* pPartyApi = pPartyConnection->GetPartyApi();
				if (!pPartyApi)
				{
					PASSERTSTREAM(1, pPartyCntl->GetName() << " - Invalid PartyApi");
					continue;
				}

				TRACEINTO << "PartyName:" << pPartyConnection->GetName() << " - The party is not a link, set it to forward all DTMFs";
				pPartyApi->SetDTMFForwarding(TRUE, SET_GW_DTMF_FORWARD);
				DeleteTimer(DTMF_FWD_ALL_TOUT); //the last party will set the whole
				StartTimer(DTMF_FWD_ALL_TOUT, dtmfFwdAllTime * SECOND);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnTimerDtmfFwdAll(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_name << ", NumOfParties:" << m_pPartyList->entries() << " - Set all DTMF forwarding to OFF";

	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (pPartyConnection)
		{
			if (pPartyConnection->IsDisconnect() == TRUE)
				continue; //VNGR-22996 - check if party is not in disconnect mode

			CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
			if (pPartyCntl)
			{
				if (CASCADE_NONE == pPartyCntl->GetPartyCascadeType())
				{
					CPartyApi* pPartyApi = pPartyConnection->GetPartyApi();
					if (pPartyApi)
						pPartyApi->SetDTMFForwarding(FALSE, SET_GW_DTMF_FORWARD);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnTimerDelayIsdnLinkDisconnect(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_name << " - Allow Terminate of linked conference";

	CTaskApp* pParty;
	*pParam >> (DWORD&)pParty;

	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end;)
	{
		BOOL bRemoved = FALSE;
		CPartyConnection* pPartyConnection = _itr->second;
		PASSERT(!pPartyConnection);
		if (!CPObject::IsValidPObjectPtr(pPartyConnection))
			return;

		if (pPartyConnection && pPartyConnection->GetInterfaceType() == ISDN_INTERFACE_TYPE)
		{
			CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
			if (pPartyCntl)
			{
				if (CASCADE_NONE != pPartyCntl->GetPartyCascadeType())
				{
					CTaskApp* pIsdnParty = pPartyCntl->GetPartyTaskApp();
					if (pIsdnParty == pParty)
					{
						TRACEINTO << "ConfName:" << m_name << " removing isdn party " << pPartyCntl->GetName();
						m_pPartyList->m_PartyList.erase(_itr++);
						bRemoved = TRUE;
						//pPartyConnection = RemovePartyConnection(pParty);
						if (pPartyConnection)
						{
							pPartyConnection->Destroy();
							POBJDELETE(pPartyConnection);
						}

					}
				}

			}

		}

		if (bRemoved == FALSE)
			++_itr;
	}
}

////////////////////////////////////////////////////////////////////////////
void CConf::DetermineTelePresenceConfMode(BYTE onOff)
{
	if(m_state == TERMINATION || m_pCommConf->GetTelePresenceModeConfiguration() != AUTO)
		return;
	if(onOff)
	{
		PTRACE2(eLevelInfoNormal, "CConf::DetermineTelePresenceConfMode - Turn On TelePresence - Name: ",m_name);
		// If party is RPX/Flex - set m_isTelePresenceMode to YES
		if(!m_pCommConf->GetIsTelePresenceMode())
		{
			PTRACE2(eLevelInfoNormal,"CConf::DetermineTelePresenceConfMode  - update conference to TelePresence one. Name: ",m_name);
			m_pCommConf->SetIsTelePresenceMode(YES);
			m_pOriginalVisualEffects = new CVisualEffectsParams(*m_pCommConf->GetVisualEffects());
			CVisualEffectsParams *pVisualEffects = new CVisualEffectsParams(*m_pCommConf->GetVisualEffects());
			m_pTelepresenceOnOffParams->isAutoBrightness = m_pCommConf->GetAutoBrightness();
			m_pTelepresenceOnOffParams->isAutoLayout = m_pCommConf->GetIsAutoLayout();
			m_pTelepresenceOnOffParams->isSameLayout = m_pCommConf->GetIsSameLayout();
			m_pTelepresenceOnOffParams->pLectureModeParams = m_pCommConf->GetLectureMode();
			DWORD colorBlack =  0x00108080;
			pVisualEffects->SetSpeakerNotationEnable(NO);
			pVisualEffects->SetlayoutBorderEnable(NO);
			pVisualEffects->SetlayoutBorderWidth(eLayoutBorderNone);
			pVisualEffects->SetBackgroundColorRGB(0);
			pVisualEffects->SetBackgroundColorYUV(colorBlack);
			pVisualEffects->SetBackgroundImageID(0);
			m_pCommConf->UpdateVisualEffectsInfo(pVisualEffects);
			m_isOriginalCropping =  m_pCommConf->GetIsCropping();
			// Disable Same Layout, Auto Layout, Lecture Mode, Presentation Mode
			m_pCommConf->SetIsSameLayout(FALSE);
			m_pCommConf->SetIsAutoLayout(FALSE);
			m_pTelepresenceOnOffParams->pLectureModeParams->SetLectureModeType(0);
			m_pCommConf->SetIsAutoBrightness(FALSE);

			if ( m_pVideoBridgeInterface )
			{
				m_pVideoBridgeInterface->TurnOnOffTelePresence(TRUE,pVisualEffects);
			}
			POBJDELETE(pVisualEffects);
		}
	}
	else
	{
		BYTE areAnyITPPartiesConnected = m_pCommConf->AreAnyITPPartiesConnected();
		if(!areAnyITPPartiesConnected && m_pCommConf->GetIsTelePresenceMode())
		{
			PTRACE2(eLevelInfoNormal, "CConf::DetermineTelePresenceConfMode - Turn Off TelePresence - Name:  ",m_name);
			m_pCommConf->SetIsCropping(m_isOriginalCropping);
			m_pCommConf->SetIsTelePresenceMode(NO);
			m_pCommConf->SetIsAutoBrightness(m_pTelepresenceOnOffParams->isAutoBrightness);
			m_pCommConf->SetIsAutoLayout(m_pTelepresenceOnOffParams->isAutoLayout);
			m_pCommConf->SetIsSameLayout(m_pTelepresenceOnOffParams->isSameLayout);
			m_pCommConf->SetLectureMode(*(m_pTelepresenceOnOffParams->pLectureModeParams));

			if(m_pOriginalVisualEffects)
			{
				m_pCommConf->UpdateVisualEffectsInfo(m_pOriginalVisualEffects);
				PTRACE2(eLevelInfoNormal, "CConf::DetermineTelePresenceConfMode - Turn Off TelePresence - Back to previous Visuall Effects - Name:  ",m_name);
				POBJDELETE(m_pOriginalVisualEffects);
			}
			if(m_pVideoBridgeInterface )
			{
				m_pVideoBridgeInterface->TurnOnOffTelePresence(FALSE,m_pCommConf->GetVisualEffects());
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CConf::DumpRsrcReqToTrace(CONF_RSRC_REQ_PARAMS_S& pParam)
{
	std::ostringstream msg;

	msg
		<< "CONF_RSRC_REQ_PARAMS_S:"
		<< "\n  monitor_conf_id        :" << pParam.monitor_conf_id
		<< "\n  sessionType            :" << eSessionTypeNames[pParam.sessionType]
		<< "\n  status                 :" << pParam.status
		<< "\n  confMediaType          :" << ConfMediaTypeToString(pParam.confMediaType)
		<< "\n  mrcMcuId               :" << pParam.mrcMcuId;

	for(int i = 0; i < 4; ++i)
		msg << "\n  LogicalResourceType[" << i << "] :" << LogicalResourceTypeToString(pParam.logicalTypeList[i]);

	TRACEINTO << msg.str().c_str();
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void CConf::DumpXCodeRsrcReqToTrace(CONFERENCE_RSRC_REQ_PARAMS_S& pParam)
{
	std::ostringstream msg;
	msg.precision(0);
	msg << "CConf::DumpXCodeRsrcReqToTrace:" << endl;
	msg << " -----+-------+-----------------------------+-----------------------------------------------------" << endl;
	msg << " conf | party | video party role            | video party type"                                     << endl;
	msg << " id   | id    |                             |"                                                      << endl;
	msg << " -----+-------+-----------------------------+-----------------------------------------------------" << endl;

	for (uint i = 0; i < ARRAYSIZE(pParam.rsrc_params_list); ++i)
	{
		msg << " " << setw( 4) << right << (WORD)pParam.rsrc_params_list[i].monitor_conf_id << " |"
		    << " " << setw( 5) << right << (WORD)pParam.rsrc_params_list[i].monitor_party_id << " |"
		    << " " << setw(27) << left  << ePartyRoleNames[pParam.rsrc_params_list[i].partyRole] << " |"
		    << " " << setw(27) << left  << eVideoPartyTypeNames[pParam.rsrc_params_list[i].videoPartyType]  << endl;
	}
	msg << " -----+-------+-----------------------------+-----------------------------------------------------";

	TRACEINTO << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
DWORD CConf::InsertInvitedPartyToSequence(char* dialString, WORD interfaceType)
{
	PARTY_SEQUENCE_LIST* pList = new PARTY_SEQUENCE_LIST;
	DWORD ans = 0xFFFFFFFF;
	char suffix[H243_NAME_LEN] = "_InvitedByPcm";
	WORD NAME_PREFIX_LEN = H243_NAME_LEN - strlen(suffix);

	BYTE isH323 = (interfaceType == 0 || interfaceType == 1)? TRUE : FALSE;
	BYTE isSip  = (interfaceType == 0 || interfaceType == 2)? TRUE : FALSE;
	BYTE isVoip = (interfaceType == 3)? TRUE : FALSE;
	BYTE isIsdn = ((CONTINUOUS_PRESENCE == GetVideoSession()) && (interfaceType == 0 || interfaceType == 4))? TRUE : FALSE;
	BYTE isPstn = ((CONTINUOUS_PRESENCE == GetVideoSession()) &&(interfaceType == 0 || interfaceType == 5))? TRUE : FALSE;

	ALLOCBUFFER(ISDNServiceNameStr,NET_SERVICE_PROVIDER_NAME_LEN);
	memset(ISDNServiceNameStr,'\0',NET_SERVICE_PROVIDER_NAME_LEN);

	if (isIsdn || isPstn)
	{
		//get global pointer to service providers list
		CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();

		const RTM_ISDN_PARAMS_MCMS_S * pIsdnServiceStruct = pConfPartyProcess->GetIsdnService("");
		if (pIsdnServiceStruct)
			strncpy(ISDNServiceNameStr,(char*)pIsdnServiceStruct->serviceName,NET_SERVICE_PROVIDER_NAME_LEN);
		else
		{
			isIsdn = FALSE;
			isPstn = FALSE;
		}
	}

	ALLOCBUFFER(IPServiceNameStr,NET_SERVICE_PROVIDER_NAME_LEN);
	memset(IPServiceNameStr,'\0',NET_SERVICE_PROVIDER_NAME_LEN);

	//get global pointer to service providers list
	const char* conf_serv_name = m_pCommConf->GetServiceNameForMinParties();
	BYTE netInterfaceType = H323_INTERFACE_TYPE;
	if(isSip){
	  netInterfaceType = SIP_INTERFACE_TYPE;
	}
	CConfIpParameters* pIpParams = ::GetIpServiceListMngr()->GetRelevantService(conf_serv_name,netInterfaceType);
	// GetRelevantService always return service: by name, default, or first in list
	if(pIpParams == NULL){
	  PASSERT(101);

	  DEALLOCBUFFER(IPServiceNameStr);
	  DEALLOCBUFFER(ISDNServiceNameStr);
	  delete pList;

	  return ans;
	}
	if( pIpParams )
	{
		strncpy(IPServiceNameStr,((char*)pIpParams->GetServiceName()),NET_SERVICE_PROVIDER_NAME_LEN);
	}
	else
	{
		PASSERT(1);
		return ans;
	}

	DWORD partyId = m_pCommConf->NextPartyId();
	//////////////////////////////////////////////////////////////////////////////////////////
	// Add H323 Party
	if (isH323 || isVoip)
	{
		CRsrvParty* p323InvitedPartyOut = new CRsrvParty();

		p323InvitedPartyOut->SetConnectionType(DIAL_OUT);
		p323InvitedPartyOut->SetNetInterfaceType(H323_INTERFACE_TYPE);
		p323InvitedPartyOut->SetServiceProviderName(IPServiceNameStr);
		p323InvitedPartyOut->SetUndefinedType(UNRESERVED_PARTY);

		ALLOCBUFFER(h243name,NAME_PREFIX_LEN);
		strncpy(h243name,dialString,NAME_PREFIX_LEN);
		h243name[NAME_PREFIX_LEN - 1] = '\0';

		mcTransportAddress IpAddr;
		memset(&IpAddr,0,sizeof(mcTransportAddress));

		if (strchr(dialString,'.') != NULL)
		{
			stringToIp(&IpAddr,dialString);
			ReplaceChar(h243name, '.', '_');
		}
		else
		{
			p323InvitedPartyOut->SetH323PartyAliasType(PARTY_H323_ALIAS_E164_TYPE);
			p323InvitedPartyOut->SetH323PartyAlias(dialString);
		}
		p323InvitedPartyOut->SetIpAddress(IpAddr);
		p323InvitedPartyOut->SetCallSignallingPort(1720);
		strcat(h243name,suffix);
		p323InvitedPartyOut->SetName(h243name);
		DEALLOCBUFFER(h243name);

		p323InvitedPartyOut->SetPartyId(partyId);
		p323InvitedPartyOut->SetVoice(isVoip);
		pList->push_back(p323InvitedPartyOut);
	}

	if (isSip)
	{
		CRsrvParty* pSipInvitedPartyOut = new CRsrvParty();

		pSipInvitedPartyOut->SetConnectionType(DIAL_OUT);
		pSipInvitedPartyOut->SetNetInterfaceType(SIP_INTERFACE_TYPE);
		pSipInvitedPartyOut->SetServiceProviderName(IPServiceNameStr);
		pSipInvitedPartyOut->SetUndefinedType(UNRESERVED_PARTY);

		ALLOCBUFFER(h243name,NAME_PREFIX_LEN);
		strncpy(h243name,dialString,NAME_PREFIX_LEN);
		h243name[NAME_PREFIX_LEN - 1] = '\0';

		mcTransportAddress sipIpAddr;
		memset(&sipIpAddr,0,sizeof(mcTransportAddress));

		if (strchr(dialString,'.') != NULL)
		{
			stringToIp(&sipIpAddr,dialString);
			ReplaceChar(h243name, '.', '_');
		}
		else
		{
			pSipInvitedPartyOut->SetSipPartyAddress(dialString);
		}

		pSipInvitedPartyOut->SetIpAddress(sipIpAddr);
		pSipInvitedPartyOut->SetCallSignallingPort(5060); //??
		strcat(h243name,suffix);

		pSipInvitedPartyOut->SetName(h243name);
		DEALLOCBUFFER(h243name);

		pSipInvitedPartyOut->SetPartyId(partyId);
		pList->push_back(pSipInvitedPartyOut);
	} // sip
	if (isIsdn)
	{
		CRsrvParty* pIsdnInvitedPartyOut = new CRsrvParty();

		pIsdnInvitedPartyOut->SetServiceProviderName(ISDNServiceNameStr);
		pIsdnInvitedPartyOut->SetConnectionType(DIAL_OUT);
		pIsdnInvitedPartyOut->SetNetInterfaceType(ISDN_INTERFACE_TYPE);
		pIsdnInvitedPartyOut->SetUndefinedType(UNRESERVED_PARTY);

		ALLOCBUFFER(h243name,H243_NAME_LEN);
		strncpy(h243name,dialString,NAME_PREFIX_LEN);
		h243name[NAME_PREFIX_LEN - 1] = '\0';
		strcat(h243name,suffix);
		pIsdnInvitedPartyOut->SetName(h243name);
		DEALLOCBUFFER(h243name);

		pIsdnInvitedPartyOut->SetNetChannelNumber(AUTO);
		pIsdnInvitedPartyOut->SetBondingMode1(AUTO);
		pIsdnInvitedPartyOut->AddCallingPhoneNumber(dialString);
		pIsdnInvitedPartyOut->SetUndefinedType(UNRESERVED_PARTY);
		pIsdnInvitedPartyOut->SetPartyId(partyId);

		pList->push_back(pIsdnInvitedPartyOut);
	} // isdn
	if (isPstn)
	{
		CRsrvParty* pPstnInvitedPartyOut = new CRsrvParty();

		pPstnInvitedPartyOut->SetServiceProviderName(ISDNServiceNameStr);
		pPstnInvitedPartyOut->SetConnectionType(DIAL_OUT);
		pPstnInvitedPartyOut->SetNetInterfaceType(ISDN_INTERFACE_TYPE);
		pPstnInvitedPartyOut->SetUndefinedType(UNRESERVED_PARTY);
		ALLOCBUFFER(h243name,H243_NAME_LEN);
		strncpy(h243name,dialString,NAME_PREFIX_LEN);
		h243name[NAME_PREFIX_LEN - 1] = '\0';
		strcat(h243name,suffix);
		pPstnInvitedPartyOut->SetName(h243name);
		DEALLOCBUFFER(h243name);

		pPstnInvitedPartyOut->SetNetChannelNumber(AUTO);
		pPstnInvitedPartyOut->SetBondingMode1(AUTO);
		pPstnInvitedPartyOut->AddCallingPhoneNumber(dialString);
		pPstnInvitedPartyOut->SetUndefinedType(UNRESERVED_PARTY);
		pPstnInvitedPartyOut->SetPartyId(partyId);
		pPstnInvitedPartyOut->SetVoice(YES);

		pList->push_back(pPstnInvitedPartyOut);
	} //pstn

	if (!pList->empty())
		m_pInvitedDialingSequence->AddPartyListToMap(partyId,pList);
	else
	{
		CMedString errorStr;
		errorStr << "CConf::InitInvitedPartiesSequence PARTY LIST IS EMPTY!!!, dialString: " << dialString;
		errorStr << "interfaceType: " << interfaceType;
		PTRACE(eLevelInfoNormal,errorStr.GetString());

		DEALLOCBUFFER(IPServiceNameStr);
		DEALLOCBUFFER(ISDNServiceNameStr);
		delete pList;

		return ans;;
	}

	DEALLOCBUFFER(IPServiceNameStr);
	DEALLOCBUFFER(ISDNServiceNameStr);

	return partyId;
}

////////////////////////////////////////////////////////////////////////////
void CConf::AddNextInvitedParty(DWORD partyId,int i)
{
	CRsrvParty* nextParty = m_pInvitedDialingSequence->GetNextParty(partyId);

	CSmallString cstr1;
	cstr1 << "(start) CConf::AddNextInvitedParty, partyId: " << partyId << " iteration: " << i;
	PTRACE(eLevelInfoNormal,cstr1.GetString());
	cstr1.Clear();
	if(CPObject::IsValidPObjectPtr(nextParty))
	{
		// validity test??
		DWORD status = TestGWPartyValidity(nextParty);
		// there is next party with bad status
		if (status != STATUS_OK)
		{
			AddNextInvitedParty(partyId,i+1);
			cstr1 << "(back from recursive call) CConf::AddNextInvitedParty, partyId: " << partyId << " iteration: " << i;
			PTRACE(eLevelInfoNormal,cstr1.GetString());
			cstr1.Clear();
		}
		// there is next party with good status
		else
		{
			CConfParty tmpConfParty(*nextParty);
			m_pCommConf->NewUndefinedParty(&tmpConfParty,EVENT_NEW_UNDEFINED_PARTY);


			COstrStream str;
			CRsrvParty* pRsrvParty = new CRsrvParty(*nextParty);
			pRsrvParty->Serialize(NATIVE,str);
			str << (DWORD)0 << "\n";
			CSegment*  pSeg = new CSegment;
			*pSeg << (DWORD)0;
			*pSeg << str.str().c_str();
			DispatchEvent(ADDPARTY,pSeg);
			POBJDELETE(pRsrvParty);
			POBJDELETE(pSeg);
		}
		POBJDELETE(nextParty);
	}
	// no more parties to dial
	else
	{
		cstr1 << "(End) CConf::AddNextInvitedParty, partyId: " << partyId << " iteration: " << i;
		PTRACE(eLevelInfoNormal,cstr1.GetString());
		cstr1.Clear();

		m_pInvitedDialingSequence->RemoveMapEntry(partyId);
		RespondPCMManagerInviteFailed((CSegment*)NULL);
	}
}

////////////////////////////////////////////////////////////////////////////
BYTE CConf::CheckIfTipEnableByParams(CIpComMode* pPartyScm, CConfParty* pConfParty, DWORD confRate, DWORD partyRate)
{
	CapEnum scmVideoProtocol 	= (CapEnum)pPartyScm->GetMediaType(cmCapVideo, cmCapTransmit);

	DWORD	minRate 			= min(confRate, partyRate);

	if (!IsFeatureSupportedBySystem(eFeatureTip))
	{
		PTRACE(eLevelInfoNormal, "CConf::CheckIfTipEnableByParams, NOT TIP - not supported system card !!!");
		return FALSE;
	}

	if (!pConfParty->GetIsTipCall())
	{
		PTRACE(eLevelInfoNormal, "CConf::CheckIfTipEnableByParams, NOT TIP - not a TIP call !!!");
		return FALSE;
	}

	if (scmVideoProtocol != eH264CapCode)
	{
		PTRACE2INT(eLevelInfoNormal, "CConf::CheckIfTipEnableByParams, NOT TIP - scmVideoProtocol: ", scmVideoProtocol);
		return FALSE;
	}

	int videoBitRate = pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);

	PTRACE2INT(eLevelInfoNormal, "CConf::CheckIfTipEnableByParams, DEBUG TIP - rate: ", videoBitRate);

	if (videoBitRate < 9360)
	{
		PTRACE2INT(eLevelInfoNormal, "CConf::CheckIfTipEnableByParams, NOT TIP - conf rate: ", videoBitRate);
		return FALSE;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
void CConf::SetPartyScmForTip(CIpComMode* pPartyScm, CConfParty* pConfParty,  CSipNetSetup* pNetSetup , DWORD serviceId, BOOL remoteSDPAvailable) //shira-TIP
{
	PASSERTMSG_AND_RETURN(!pConfParty || !pPartyScm || !m_pCommConf , "!pConfParty || !pPartyScm || !m_pCommConf");
	pPartyScm->SetTipMode(eTipModePossible);

	int videoBitRate = pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);

	PTRACE2INT(eLevelInfoNormal, "CConf::SetPartyScmForTip, DEBUG TIP - rate: ", videoBitRate);
	//BRIDGE-5753
	// Change PartyScm resolution according to TIP definitions
	//Eh264VideoModeType h264VidMode = GetTipResolutionTypeAccordingToVideoRate(videoBitRate);
	EVideoResolutionType maxResolution  = (EVideoResolutionType)(m_pCommConf->GetConfMaxResolution());
	Eh264VideoModeType   h264VidMode 	= GetTipResolutionTypeAccordingToMaxResolutionAndVidRate(maxResolution, videoBitRate, pConfParty);

	FixTipScmVideoBitRateIfNeeded(pConfParty , pPartyScm, pNetSetup, h264VidMode);

	pPartyScm->SetAudioAlg(eAAC_LDCapCode,cmCapReceiveAndTransmit);

	CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(serviceId);

	if ( (pConfParty->GetRemoteIdent() == CiscoCucm) || ((pService != NULL) && (pService->GetSipServerType() == eSipServer_CiscoCucm)) )
	{
		PTRACE(eLevelInfoNormal, "CConf::SetPartyScmForTip : remote ident: CiscoCucm ");

		pPartyScm->SetIsLpr(FALSE);

		//pPartyScm->SetMediaOff(cmCapData, cmCapReceiveAndTransmit, kRolePeople);
	}

	PTRACE2INT(eLevelInfoNormal, "CConf::SetPartyScmForTip : TIP partyResolution ==> chosen h264VidMode: ", (WORD)h264VidMode);
	H264VideoModeDetails h264VidModeDetails;

	CH264VideoMode* pH264VidMode = new CH264VideoMode();
	pH264VidMode->GetH264VideoModeDetailsAccordingToTypeForTIP(h264VidModeDetails, h264VidMode);
	POBJDELETE(pH264VidMode);

	PTRACE(eLevelInfoNormal, "CConf::SetPartyScmForTip, might be a TIP call, set main profile for TIP !!!");
	pPartyScm->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR, cmCapReceiveAndTransmit);

	if (IsTIPContentEnable() && pPartyScm->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation)) //shira-TIP
		pPartyScm->SetTipAuxFPS(eTipAux5FPS);

	if(remoteSDPAvailable)
	{
		pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
		pPartyScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
	}
}

////////////////////////////////////////////////////////////////////////////
BOOL CConf::GetIsNeedToChangeTipResAccordingToConfVidQuality(CConfParty* pConfParty)
{//BRIDGE-5753
	if(!pConfParty)
	{
		PASSERTMSG(!pConfParty, "!pConfParty");
		return FALSE;
	}

	BOOL bIsFlagOn 		= GetFlagTipResAccordingToConfVidQuality();
	BOOL bIsTipCall		= pConfParty->GetIsTipCall();
	BOOL bIsTipHeader  	= pConfParty->GetIsTipHeader();

	if(!bIsFlagOn)
	{
		PTRACE(eLevelInfoNormal,"CConf::GetIsNeedToChangeTipResAccordingToConfVidQuality - TIP_RESOLUTION_ACCORDING_TO_CONF_VID_QUALITY is NO ");
		return FALSE;
	}

	if(!bIsTipCall || !bIsTipHeader)
	{
		PTRACE(eLevelInfoNormal,"CConf::GetIsNeedToChangeTipResAccordingToConfVidQuality - not TIP! ");
		return FALSE;
	}

	return TRUE;;
}


////////////////////////////////////////////////////////////////////////////
BOOL CConf::GetFlagTipResAccordingToConfVidQuality()
{//BRIDGE-5753
	BOOL          bRetVal 	 = YES; //Default
	CSysConfig*   sysConfig  = NULL;
	CProcessBase* pProcess   = CProcessBase::GetProcess();

	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();

	if(sysConfig)
		sysConfig->GetBOOLDataByKey(CFG_KEY_TIP_RESOLUTION_ACCORDING_TO_CONF_VID_QUALITY, bRetVal);  // the default value is "NO"

	return bRetVal;
}

////////////////////////////////////////////////////////////////////////////
DWORD CConf::GetMaxVidBitrateForHD720()
{//BRIDGE-5753
	return 2560;
}

////////////////////////////////////////////////////////////////////////////
BOOL CConf::IsNeedToChangeTipVidRate(Eh264VideoModeType selectedVideoMode, DWORD vidBitrate)
{//BRIDGE-5753
	BOOL       bNeedToChange 		  = FALSE;
	DWORD      nVidBitRateFor720HD 	  = GetMaxVidBitrateForHD720() ;
	CMedString str	   		 		  = "";

	if(selectedVideoMode == eHD720Symmetric && vidBitrate > nVidBitRateFor720HD * 10)
	{
		str << "CConf::IsNeedToChangeTipVidRate - TIP call - res HD720 - need to change bit rate from "<< (int)vidBitrate <<" to " << (int)nVidBitRateFor720HD;
		bNeedToChange = TRUE;
	}
	else
	{
		str << "CConf::IsNeedToChangeTipVidRate - no need to change bit rate";
		bNeedToChange = FALSE;
	}

	PTRACE(eLevelInfoNormal,str.GetString());
	return bNeedToChange;
}

////////////////////////////////////////////////////////////////////////////
void CConf::ChangeVideoBitrateForTipIfNeeded(CConfParty* pConfParty, CIpComMode* pPartyScm , DWORD& vidBitrate)
{//BRIDGE-5753
	PASSERTMSG_AND_RETURN(!pConfParty || !pPartyScm, "!pConfParty || !pPartyScm");

	if(!GetIsNeedToChangeTipResAccordingToConfVidQuality(pConfParty))
	{
		PTRACE(eLevelInfoNormal,"CConf::ChangeVideoBitrateForTipIfNeeded - no need");
		return;
	}

	int videoBitRateFromScm = pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);

	vidBitrate = videoBitRateFromScm;
}


////////////////////////////////////////////////////////////////////////////
void CConf::FixTipScmVideoBitRateIfNeeded(CConfParty* pConfParty, CIpComMode* pPartyScm , CSipNetSetup* pNetSetup,  Eh264VideoModeType selectedVideoMode)
{//BRIDGE-5753
	PASSERTMSG_AND_RETURN(!pConfParty || !pPartyScm , "!pConfParty || !pPartyScm");

	if(!GetIsNeedToChangeTipResAccordingToConfVidQuality(pConfParty))
	{
		PTRACE(eLevelInfoNormal,"CConf::FixTipScmVideoBitRateIfNeeded - no need ");
		return;
	}

	int nVidBitRateFor720HD = GetMaxVidBitrateForHD720() ;
	int videoBitRate 		= pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);

	CMedString str = "";
	str << "CConf::SetTipVideoBitRateAccordingToMaxResolution - (Eh264VideoModeType)selectedVideoMode " << (int)selectedVideoMode << " videoBitRate " << videoBitRate;
	PTRACE(eLevelInfoNormal,str.GetString());

	if(IsNeedToChangeTipVidRate(selectedVideoMode, videoBitRate))
	{
		pPartyScm->SetVideoBitRate(nVidBitRateFor720HD * 10, cmCapReceiveAndTransmit);
		pPartyScm->SetTotalVideoRate(nVidBitRateFor720HD * 10);
		pPartyScm->SetCallRate(nVidBitRateFor720HD);

		if(pNetSetup)
		{
			TRACEINTO << "pNetSetup->SetMaxRate: " << (int)(nVidBitRateFor720HD * 1000);
			pNetSetup->SetMaxRate(nVidBitRateFor720HD * 1000);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
Eh264VideoModeType CConf::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate(EVideoResolutionType maxResolution, DWORD videoBitRate, CConfParty* pConfParty)
{//BRIDGE-5753
	CMedString str = "";
	str << "CConf::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate - (EVideoResolutionType)maxResolution " << (int)maxResolution << " videoBitRate " << videoBitRate ;
	PTRACE(eLevelInfoNormal,str.GetString());

	BOOL bResAccordingToVidQuality = GetIsNeedToChangeTipResAccordingToConfVidQuality(pConfParty);

	if( bResAccordingToVidQuality && maxResolution <= eHD720_Res)
	{
		PTRACE(eLevelInfoNormal, "CConf::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate,  TIP call, force eHD720_Res");
		return eHD720Symmetric;
	}
	else
	{
		if (videoBitRate >= 30000)
		{
			PTRACE(eLevelInfoNormal, "CConf::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate, set eHD1080Symmetric");
			return eHD1080Symmetric;// when we'll support 1080 for TIP we need to set it to: eHD1080Symmetric (or maybe even eHD1080At60Asymmetric)
		}

		else if (videoBitRate >= 936 && videoBitRate < 30000)
		{
			PTRACE(eLevelInfoNormal, "CConf::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate, set eHD720Symmetric");
			return eHD720Symmetric;
		}

		PASSERTMSG(1,"CConf::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate - TIP video rate is lower than 936 kbps!!!");
	}

	return eInvalidModeType;
}

////////////////////////////////////////////////////////////////////////////
Eh264VideoModeType CConf::GetTipResolutionTypeAccordingToVideoRate(DWORD videoBitRate)
{
	if (videoBitRate >= 30000)
    	return eHD1080Symmetric;// when we'll support 1080 for TIP we need to set it to: eHD1080Symmetric (or maybe even eHD1080At60Asymmetric)

    else if (videoBitRate >= 936 && videoBitRate < 30000)
    	return eHD720Symmetric;

	PASSERTMSG(1,"CConf::GetTipResolutionTypeAccordingToVideoRate - TIP video rate is lower than 936 kbps!!!");

	return eInvalidModeType;
}

////////////////////////////////////////////////////////////////////////////
void  CConf::ConnectSipSlaveParty( CConfParty* pConfParty, DWORD connectDelay,
		                           WORD tipPartyType, DWORD room_Id, DWORD pPeerPartyRsrcID, DWORD masterVideoPartyType,
		                           CIpComMode *pIpMasterInitialMode)
{
	PASSERT_AND_RETURN(NULL == pConfParty);

	TRACEINTO << "CConf::ConnectSipSlaveParty - PartyId:" << pConfParty->GetPartyId() << ", PartyName:" << pConfParty->GetName();

	DWORD nextPartyId = m_pCommConf->NextPartyId();
	//SET new partyId to party
    if (pConfParty->GetPartyId() <= HALF_MAX_DWORD || pConfParty->GetPartyId() == 0xFFFFFFFF) {
			 ((CRsrvParty*)pConfParty)->SetPartyId(nextPartyId);
    }

	PartyControlDataParameters partyControlDataParams;
    memset(&partyControlDataParams, 0, sizeof(PartyControlDataParameters));

	partyControlDataParams.tipPartyType = (ETipPartyTypeAndPosition)tipPartyType;
	partyControlDataParams.peerPartyRsrcID = pPeerPartyRsrcID;
	partyControlDataParams.masterVideoPartyType = (eVideoPartyType)masterVideoPartyType;
	partyControlDataParams.pIpMasterInitialMode = pIpMasterInitialMode;

	SetControlDataParams(partyControlDataParams, pConfParty,NULL,NULL,NULL,TRUE,FALSE,FALSE,Regular,No_Lync,room_Id,eRegularParty);

	PartyControlInitParameters partyControInitParam;
    memset(&partyControInitParam, 0, sizeof(PartyControlInitParameters));
	SetControlInitParams(partyControInitParam,pConfParty,FALSE,NULL,0,connectDelay,0,NULL);

	CPartyConnection* pPartyConnection = new CPartyConnection;
	pPartyConnection->ConnectIP(partyControInitParam,partyControlDataParams);
	InsertPartyConnection(pPartyConnection);
}

////////////////////////////////////////////////////////////////////////////
BOOL CConf::IsTIPContentEnable() const
{
	return m_pCommConf->GetIsTipCompatibleContent();
}


////////////////////////////////////////////////////////////////////////////
void CConf::ConnectMsSlaveParty(CConfParty* pConfParty, DWORD connectDelay, DWORD mainPartyRsrcID, eAvMcuLinkType AvMcuLinkType, DWORD msSlaveIndex, DWORD msSsrcRangeStart, CSipCaps* remoteCaps, CVidModeH323	*pLocalSdesCap)    // / similar to CConf::ConnectSipSlaveParty
{
	PASSERT_AND_RETURN(!pConfParty);

	// SET new partyId to party
	if (pConfParty->GetPartyId() <= HALF_MAX_DWORD || pConfParty->GetPartyId() == 0xFFFFFFFF)
		pConfParty->SetPartyId(m_pCommConf->NextPartyId());

	TRACEINTO << "MonitorPartyId:" << pConfParty->GetPartyId() << ", PartyName:" << pConfParty->GetName();

	PartyControlDataParameters partyControlDataParams;
	memset(&partyControlDataParams, 0, sizeof(PartyControlDataParameters));

	partyControlDataParams.tipPartyType          = eTipNone;
	partyControlDataParams.peerPartyRsrcID       = mainPartyRsrcID;
	partyControlDataParams.AvMcuLinkType         = AvMcuLinkType;
	partyControlDataParams.msSlaveIndex          = msSlaveIndex;
	partyControlDataParams.msSlaveSsrcRangeStart = msSsrcRangeStart;
	partyControlDataParams.pLocalSdesCap		 = pLocalSdesCap;

	SetControlDataParams(partyControlDataParams, pConfParty, NULL, NULL, remoteCaps, TRUE, FALSE, FALSE, Microsoft_AV_MCU2013, AvMcuLync2013Slave, (DWORD)(-1), eRegularParty);

	PartyControlInitParameters partyControInitParam;
	memset(&partyControInitParam, 0, sizeof(PartyControlInitParameters));
	SetControlInitParams(partyControInitParam, pConfParty, FALSE, NULL, 0, connectDelay, 0, NULL);

	CPartyConnection* pPartyConnection = new CPartyConnection;
	pPartyConnection->ConnectIP(partyControInitParam, partyControlDataParams);
	InsertPartyConnection(pPartyConnection);
}

////////////////////////////////////////////////////////////////////////////
// Resolution Slider: a new 'Resolution' field is added in the participant's properties.
//This field (together with the video bit rate field) will override any general definition in the conference properties.
BOOL CConf::IsNeedRecalcH264ParamsAccordingToPartySettings( CConfParty* pConfParty )
{
	eVideoQuality vidQuality = m_pCommConf->GetVideoQuality();
	BYTE partyResolution =  pConfParty->GetMaxResolution();
	BYTE maxConfResolution = m_pCommConf->GetConfMaxResolution();
	BYTE maxPartyResolution = ( partyResolution == eAuto_Res && maxConfResolution != eAuto_Res) ? maxConfResolution : partyResolution;
	if( maxPartyResolution != maxConfResolution && maxPartyResolution != eAuto_Res )
		return TRUE;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CConf::GetH264VideoParamsAccordingToPartySettings( H264VideoModeDetails& returnH264VidModeDetails, CConfParty* pConfParty, DWORD decisionRate, BOOL isHighProfile )
{
	eVideoQuality vidQuality = m_pCommConf->GetVideoQuality();
	BYTE partyResolution =  pConfParty->GetMaxResolution();
	BYTE maxConfResolution = m_pCommConf->GetConfMaxResolution();
	BYTE maxPartyResolution = ( partyResolution == eAuto_Res && maxConfResolution != eAuto_Res) ? maxConfResolution : partyResolution;

	if( partyResolution == eAuto_Res )
	{
		Eh264VideoModeType partyMaxVideoMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)maxPartyResolution, vidQuality );
		TRACEINTO << "partyResolution is Auto_Res ==> chosen partyMaxVideoMode = " << (WORD)partyMaxVideoMode;
		GetH264VideoParams( returnH264VidModeDetails, decisionRate, vidQuality, partyMaxVideoMode, isHighProfile );
	}
	else
	{
		Eh264VideoModeType h264VidMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)partyResolution, vidQuality);
		TRACEINTO << "partyResolution = " << (WORD)partyResolution << " ==> chosen h264VidMode = " << (WORD)h264VidMode;
		CH264VideoMode* pH264VidMode = new CH264VideoMode();
		pH264VidMode->GetH264VideoModeDetailsAccordingToType( returnH264VidModeDetails, h264VidMode );
		POBJDELETE(pH264VidMode);
	}
}
////////////////////////////////////////////////////////////////////////////

std::string CConf::GetProductTypeAsString()const
{
	std::string productTypeStr = "Polycom RMX 2000";
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if(eProductTypeRMX4000 == curProductType)
			productTypeStr = "Polycom RMX 4000";
	if(eProductTypeCallGenerator == curProductType)
			productTypeStr = "Polycom Call Generator";
	else if (eProductTypeRMX1500 == curProductType)
			productTypeStr = "Polycom RMX 1500";
	else if (eProductTypeSoftMCU == curProductType)
			productTypeStr = "Polycom Soft MCU";
	else if (eProductTypeGesher == curProductType)
			productTypeStr = "Polycom RMX 800S";
	else if (eProductTypeNinja == curProductType)
			productTypeStr = "Polycom RMX 1800";
	else if (eProductTypeSoftMCUMfw == curProductType)
		productTypeStr = "Polycom Soft MCU MFW";
	else if (eProductTypeEdgeAxis == curProductType)
			productTypeStr = "Polycom RMX 800VE";
	else if(eProductTypeCallGeneratorSoftMCU  == curProductType)
		productTypeStr = "Polycom Call Generator Soft MCU";
 return productTypeStr;
}

////////////////////////////////////////////////////////////////////////////
void CConf::SetConfOperationPoints()
{
	// this function valid only for SVC type (any conference who allowed to connect SVC party)
	DWORD confMediaType = m_pCommConf->GetConfMediaType(); // AVC/SVC/Mix-CP/Mix-VSW
	if (eSvcOnly != confMediaType && eMixAvcSvc != confMediaType && eMixAvcSvcVsw != confMediaType)
		return;

	if(m_pConfOperationPointsSet)
		POBJDELETE(m_pConfOperationPointsSet);
	m_pConfOperationPointsSet = new CVideoOperationPointsSet();
	DWORD reservationRate = m_pCommConf->GetConfTransferRate();
	CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
	DWORD confRate = lCapInfo.TranslateReservationRateToIpRate(reservationRate);

	// find if Mix-VSW conf
	bool isAvcVswInSvcOnlyConf = (confMediaType == /*SVC-*/eMixAvcSvcVsw);
	TRACEINTOFUNC << "confMediaType: " << confMediaType << ", (isAvcVswInSvcOnlyConf: " << (isAvcVswInSvcOnlyConf ? "yes)" : "no)");

	// get systemcfg flag ENABLE_1080_SVC
	BOOL bIsEnable_1080_SVC = FALSE;
	std::string key1 = CFG_KEY_ENABLE_1080_SVC;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(key1, bIsEnable_1080_SVC);  // the default value is "NO"

	EOperationPointPreset eOPPreset = m_pCommConf->GetOperationPointPreset();

//	m_pConfOperationPointsSet->SetDefaultParams(confRate);
	m_pConfOperationPointsSet->SetDefaultParams(confRate, isAvcVswInSvcOnlyConf, bIsEnable_1080_SVC, eOPPreset);

	m_pConfOperationPointsSet->SetSetId(m_ConfRsrcId);
	m_pConfOperationPointsSet->Trace("CConf::SetConfOperationPoints - set the op. points for the conf");
}
///////////////////////////////////////////////////////////////
const char* CConf::ConfStateToString(WORD state)
{
  switch (state)
  {
    case SETUP      : return "SETUP";
    case TERMINATION: return "TERMINATION";
    case CONNECT    : return "CONNECT";
    case CHANGEMODE : return "CHANGEMODE";
  }
  return "UNDEFINED";
}

///////////////////////////////////////////////////////////////
const char* CConf::LastQuitTypeToString(ELastQuitType lastQuitType)
{
  switch (lastQuitType)
  {
    case eTerminateAfterLastLeaves: return "eTerminateAfterLastLeaves";
    case eTerminateWithLastRemains: return "eTerminateWithLastRemains";
  }
  return "UNDEFINED";
}

void CConf::SaveInviteResult(DWORD partyId, WORD interfaceType,
	eGeneralDisconnectionCause disconnectionCause)
{
	SInviteResult inviteResult;

	if(m_PartiesInviteResults.find(partyId) != m_PartiesInviteResults.end())
	{
		inviteResult = m_PartiesInviteResults[partyId];
	}
	else
	{
		memset(&inviteResult, 0, sizeof(inviteResult));
		inviteResult.eOverallInviteResult = disconnectionCause;
	}

	switch (interfaceType)
	{
		case H323_INTERFACE_TYPE:
		{
			inviteResult.bIsH323Used = TRUE;
			inviteResult.eH323InviteResult = disconnectionCause;

			break;
		}
		case SIP_INTERFACE_TYPE:
		{
			inviteResult.bIsSIPUsed = TRUE;
			inviteResult.eSIPInviteResult = disconnectionCause;

			break;
		}
		case ISDN_INTERFACE_TYPE:
		{
			inviteResult.bIsISDNUsed = TRUE;
			inviteResult.eISDNInviteResult = disconnectionCause;

			break;
		}
		case PSTN_INTERFACE_TYPE:
		{
			inviteResult.bIsPSTNUsed = TRUE;
			inviteResult.ePSTNInviteResult = disconnectionCause;

			break;
		}
		default:
			PASSERT(2012);
			return;
	}

	if (disconnectionCause == eRemoteBusy)
	{
		inviteResult.eOverallInviteResult = eRemoteBusy;
	}
	else if (disconnectionCause == eRemoteNoAnswer &&
		inviteResult.eOverallInviteResult != eRemoteBusy)
	{
		inviteResult.eOverallInviteResult = eRemoteNoAnswer;
	}
	else if (disconnectionCause == eRemoteWrongNumber &&
		inviteResult.eOverallInviteResult != eRemoteBusy &&
		inviteResult.eOverallInviteResult != eRemoteNoAnswer)
	{
		inviteResult.eOverallInviteResult = eRemoteWrongNumber;
	}

	m_PartiesInviteResults[partyId] = inviteResult;
}

void CConf::CheckInviteResult(DWORD partyId, eGeneralDisconnectionCause &disconnectionCause)
{
	SInviteResult inviteResult;

	if(m_PartiesInviteResults.find(partyId) != m_PartiesInviteResults.end())
	{
		disconnectionCause = m_PartiesInviteResults[partyId].eOverallInviteResult;
	}
	else
	{
		PASSERT(2012);
		disconnectionCause = eRemoteFailed;
	}
}

// VNGR-23989
void CConf::ReleaseLobbySuspendPartiesIfNeeded()
{
	for (PARTIES_TO_RELEASE_FROM_LOBBY::iterator itr = m_PartiesToReleaseFromLobby.begin();
		itr != m_PartiesToReleaseFromLobby.end();
		itr++)
	{
		//send message to lobby to release the suspended call
		char s[ONE_LINE_BUFFER_LEN];
		sprintf(s,"auto add party, party id = %u, party list id = %u ", itr->first, itr->second);
		PTRACE2(eLevelInfoNormal,"CConf::ReleaseLobbySuspendPartiesIfNeeded : ",s);

		CLobbyApi* pLobbyApi = (CLobbyApi*)::GetpLobbyApi();
		pLobbyApi->ReleaseUnreservedParty(m_pCommConf->GetMonitorConfId(),
			itr->first, itr->second);
	}

	m_PartiesToReleaseFromLobby.clear();
}

STATUS CConf::CheckVideoLayout(CVideoLayout* pVideoLayoutOper, CVideoLayout* pVideoLayoutDB,BYTE isHDVSW,const WORD isPrivate)
{
    CVideoCellLayout*  pVideoCellLayoutOper = NULL;
    CVideoCellLayout*  pVideoCellLayoutDB = NULL;
    DWORD forced_partyId;
    WORD  j = 0;
    BYTE cellStatus;

    if (!pVideoLayoutOper /*|| !pVideoLayoutDB*/)
        return STATUS_ILLEGAL;

    for (WORD i=0; i<pVideoLayoutOper->m_numb_of_cell; i++)
    {
        // Check the structure received from OperatorWS

        pVideoCellLayoutOper = pVideoLayoutOper->m_pCellLayout[i];

        if (pVideoCellLayoutOper)
        {
            forced_partyId = pVideoCellLayoutOper->GetForcedPartyId();

            if (forced_partyId!=0xFFFFFFFF)
            {
                cellStatus = pVideoCellLayoutOper->GetCellStatus();

                if (cellStatus!=EMPTY_BY_OPERATOR_THIS_PARTY && cellStatus!=EMPTY_BY_OPERATOR_ALL_CONF &&
                    cellStatus!=AUTO && cellStatus!=AUDIO_ACTIVATED)
                {
                    if ((pVideoLayoutOper->GetNumCells(forced_partyId))>1)
                        return  STATUS_PARTY_IS_PRESENT_IN_SEVERAL_CELLS;

                    // Check database current layout and received info consistency

                    if (pVideoLayoutDB && !isPrivate)
                        for (j=0; j<pVideoLayoutDB->m_numb_of_cell; j++)
                        {
                            pVideoCellLayoutDB = pVideoLayoutDB->m_pCellLayout[j];

                            if (pVideoCellLayoutDB)
                                if (pVideoLayoutOper->FindCell(pVideoCellLayoutDB->GetCellId()) == NOT_FIND)
                                {
                                    if (pVideoCellLayoutDB->GetCurrentPartyId() == forced_partyId)
                                    {
                                        cellStatus = pVideoCellLayoutDB->GetCellStatus();

                                        if (cellStatus!=EMPTY_BY_OPERATOR_THIS_PARTY && cellStatus!=EMPTY_BY_OPERATOR_ALL_CONF &&
                                            cellStatus!=AUTO && cellStatus!=AUDIO_ACTIVATED)
                                            return STATUS_PARTY_IS_PRESENT_IN_SEVERAL_CELLS;
                                    }
                                }
                        }
                }
            }
        }
    }

    return STATUS_OK;
}

// VNGR-22639: Restore the lecturer video layout in Slave becomes Master case
void CConf::RestoreLecturerVideoLayoutIfNeeded(CConfParty* pParty)
{
    CLectureModeParams* pLectureMode;
    CConfParty* pLecturerConfParty;
    CVideoLayout*  pLecturerVideoLayout;

    m_pCommConf->GetLecturerVideoLayout(pLectureMode, pLecturerVideoLayout);

    if (pLectureMode && (pLectureMode->GetLecturerId() == pParty->GetPartyId()))
    {
        STATUS status = STATUS_OK;
        BYTE isHDVSW  = FALSE;
        const WORD isPrivate = 0;
        CVideoLayout* pVideoLayout = pLecturerVideoLayout;
        const CCommConf* pCommConf = m_pCommConf;

        if(STATUS_OK == status)
        {// Check  if the same party is present in several cells.
            isHDVSW = pCommConf->GetIsHDVSW();
            if (pVideoLayout)
            {
                if((pParty->GetPartyState() == PARTY_SECONDARY)||(pParty->GetVoice()) || (pParty->GetPartyState() == PARTY_DISCONNECTED) || (pParty->GetPartyState() == PARTY_REDIALING))
                {
                    status = STATUS_ILLEGAL_OPERATION_VIDEO_CAPABILITIES_INACTIVE;
                }
                else if(pCommConf->GetIsSameLayout())
                {
                    status = STATUS_IN_SAME_LAYOUT_MODE_VIDEO_FORCE_IS_FORBIDDEN;
                }
                else
                {
                    CVideoLayout*  pVideoLayoutParty =   pParty->GetVideoLayout();
                    status = CheckVideoLayout(pVideoLayout, pVideoLayoutParty,isHDVSW,isPrivate);
                }
            }
            else
                status = STATUS_INCONSISTENT_PARAMETERS;
        }

    //      if (status == STATUS_OK){
    //          if (!m_pMoveMngr->IsMoveCompleted(confId,pPartyName))
    //            status = STATUS_MOVE_PARTY_NOT_COMPLETED;
    //      }

        if (status == STATUS_OK)
        {
            BYTE isCOP = pCommConf->GetIsCOP();
            if(!isCOP)
            {
                const char* pPartyName = pParty->GetName();
                CConfApi confApi;
                confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));

                if(isPrivate)
                {
                    if(!isHDVSW)
                    confApi.SetVideoPrivateLayout(pPartyName, *pVideoLayout);
                    else
                    {
                        PTRACE(eLevelInfoNormal, "CConf::RestoreLecturerVideoLayoutIfNeeded cant set private layout in VSW conference");
                        status = STATUS_IN_VSW_CONFERENCE_PRIVATE_LAYOUT_IS_FORBIDDEN;
                    }
                }
                else
                {
                    CVideoLayout * pConfLayout = m_pCommConf->GetVideoLayout();
                    if(pConfLayout)
                    {
                      CVideoLayout confLayout(*pConfLayout);
                      std::string szConfLayout;
                      confLayout.ToString(szConfLayout);
                      PTRACE2(eLevelInfoNormal,"CConfApi::RestoreLecturerVideoLayoutIfNeeded\n", szConfLayout.c_str());
                      confApi.SetVideoConfLayoutSeeMeAll(confLayout,1);
                    }
                    else
                    {
                      PTRACE(eLevelInfoNormal,"CConfApi::RestoreLecturerVideoLayoutIfNeeded, Can't find ConfLayout\n");
                    }
                    //confApi.SetVideoConfLayoutSeeMeParty(pPartyName, *pVideoLayout);
                }
                confApi.DestroyOnlyApi();
            }
            else
            {
                PTRACE(eLevelInfoNormal, "CConf::RestoreLecturerVideoLayoutIfNeeded,No personal layouts in COP conf");
                status = STATUS_IN_EVENT_MODE_CONFERENCE_PERSONAL_LAYOUT_IS_FORBIDDEN;
            }
        }

        //Trace
        PTRACE2(eLevelInfoNormal,"CConf::RestoreLecturerVideoLayoutIfNeeded end: "
              , CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

        POBJDELETE(pLectureMode);
        POBJDELETE(pLecturerVideoLayout);

        m_pCommConf->SaveLecturerVideoLayout(pLectureMode, pLecturerVideoLayout);
    }

}

// Does the statistics to SNMP
void CConf::SendMessageToSNMP(eTelemetryType type, DWORD value)
{
  TRACEINTO << "Type:" << TelemetaryTypeToStr(type) << ", value:" << value;

  CSegment* seg = new CSegment;
  *seg << static_cast<unsigned int>(type) << value;

  CManagerApi(eProcessSNMPProcess).SendMsg(seg, SNMP_UPDATE_TELEMETRY_DATA_IND);
}
//unencrypted conference message
void CConf::SendSecureMessageWhenAddParty(CPartyConnection*  pPartyConnection)
{
	PTRACE2(eLevelInfoNormal,"CConf::SendSecureMessageWhenAddParty - ", pPartyConnection->GetFullName());

	eProductFamily eProdFamily = CProcessBase::GetProcess()->GetProductFamily();

	if(m_pCommConf->GetEncryptionType()==eEncryptWhenAvailable)
	{
		CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
		PASSERT_AND_RETURN(NULL == pConfParty);
		BYTE isEncrypted = pConfParty->GetIsPartyCurrentlyEncrypted();
		//saved encrypted state of the party for the delete
		pConfParty->SetIsDelPartyWasEncrypted(isEncrypted);
		if (isEncrypted == NO)
		{
			//update conference state
			int numOfUnencrypted = GetNumOfUnencryptedParty();
			numOfUnencrypted++;
			TRACEINTO << "isEncrypted == NO, num of unencrypted "<<numOfUnencrypted;
			SetNumOfUnencryptedParty(numOfUnencrypted);
			//send message to all parties(including the new) if the numOfUnencrypted is now 1
			if(1 == numOfUnencrypted)
			{
				//send message to all parties
				CConfApi* pConfApi = new CConfApi;
				pConfApi->CreateOnlyApi(GetRcvMbx());
				pConfApi->UpdateSecureMesaageToAllParties();
				POBJDELETE(pConfApi);
			}
		}

	}

}

//unencrypted conference message
void CConf::SendSecureMessageWhenDelParty(CPartyConnection*  pPartyConnection)
{

	if (!CPObject::IsValidPObjectPtr(pPartyConnection))
	{
		PASSERTSTREAM(1, "CConf::SendSecureMessageWhenDelParty - Failed, Party connection not found");
		return;
	}

	PTRACE2(eLevelInfoNormal,"CConf::SendSecureMessageWhenDelParty - party name", pPartyConnection->GetName());

	//secure message is sent only if we in encrypt when available state
	//we want to send a message only if party going out and not if all the conference terminate.

	eProductFamily eProdFamily = CProcessBase::GetProcess()->GetProductFamily();

	if(m_pCommConf->GetEncryptionType()==eEncryptWhenAvailable &&
	   ConfStateToString(m_state) != std::string("TERMINATION"))
	{
		CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
		PASSERT_AND_RETURN(!pConfParty);
		BYTE isEncrypted = pConfParty->GetIsDelPartyWasEncrypted();
		if (isEncrypted == NO)
		{
			//update conference state
			int numOfUnencrypted = GetNumOfUnencryptedParty();
			numOfUnencrypted--;
			TRACEINTO << "isEncrypted == NO, num of unencrypted "<<numOfUnencrypted;
			SetNumOfUnencryptedParty(numOfUnencrypted);
			//send message to all parties if the numOfUnencrypted is now 0
			if(0 == numOfUnencrypted)
			{
				//send message to all parties
				CConfApi* pConfApi = new CConfApi;
				pConfApi->CreateOnlyApi(GetRcvMbx());
				pConfApi->UpdateSecureMesaageToAllParties();
				POBJDELETE(pConfApi);
			}
		}
	}
}



//////////////////////////////////////////////////////////////
void CConf::GetH264ContentParams(ePresentationProtocol contentProtocolMode, APIU16& profile, APIU8& level, long& mbps, long& fs, long& dpb, long& brAndCpb, long& sar, long& staticMB) const
{

	BYTE HDMpi = 0;
	EHDResolution eHDRes;

	BYTE lConfRate = m_pCommConf->GetConfTransferRate();
	eEnterpriseMode ContRatelevel = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
	BYTE MaxContentRate = m_pUnifiedComMode->FindAMCContentRateByLevel(lConfRate, (eConfMediaType)(m_pCommConf->GetConfMediaType()), ContRatelevel, contentProtocolMode, m_pCommConf->GetCascadeOptimizeResolution());

	MaxContentRate = m_pUnifiedComMode->FindAMCContentRateByLevel(lConfRate, (eConfMediaType)(m_pCommConf->GetConfMediaType()), ContRatelevel, eH264Dynamic, m_pCommConf->GetCascadeOptimizeResolution());
	DWORD videoBridgeBitRate =  ((CConf*)this)->TranslateAMCRateIPRate(MaxContentRate)*100;
	BYTE isHDContent1080Supported = ((CConf*)this)->SelectContentHDResolution(MaxContentRate,H264,HDMpi,FALSE);
	if(isHDContent1080Supported)
		eHDRes = eHD1080Res;
	else
		eHDRes = eHD720Res;

	CIpComMode*	pH264ContentScm  = new CIpComMode;
	BYTE bContentAsVideo = m_pCommConf->IsLegacyShowContentAsVideo();
	BOOL bHighProfileContent = GetCurrentContentIsHighProfile();  //HP content
	pH264ContentScm->RemoveContent(cmCapReceiveAndTransmit);
	pH264ContentScm->SetIsShowContentAsVideo(bContentAsVideo);
	pH264ContentScm->SetHDContent(videoBridgeBitRate,cmCapReceiveAndTransmit,eHDRes,HDMpi,bHighProfileContent);
	const CVidModeH323& rH264ContentMode = (const CVidModeH323 &)pH264ContentScm->GetMediaMode(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation);
	rH264ContentMode.GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
	POBJDELETE(pH264ContentScm);
}
///////////////////////////////////////////////////////////////
eXcodeRsrcType CConf::ConnectPartyToXCodeEncoder(CPartyConnection* pPartyConnection,CConfParty* pConfParty)
{
	eXcodeRsrcType ePartyContentEncoderType = eXcodeEncoderDummy;
	XCODE_RSRC::iterator resourceIt;

	if(!CPObject::IsValidPObjectPtr(pConfParty))
	{
		PASSERT(201);
		return ePartyContentEncoderType;
	}
	if(!CPObject::IsValidPObjectPtr(pPartyConnection))
	{
		PASSERT(202);
		return ePartyContentEncoderType;
	}


	CPartyCntl* pPartyCntl  = pPartyConnection->GetPartyCntl();

	if(!CPObject::IsValidPObjectPtr(pPartyCntl))
	{
		PASSERT(203);
		return ePartyContentEncoderType;
	}

	CTaskApp* pParty = pPartyCntl->GetPartyTaskApp();
	if(!CPObject::IsValidPObjectPtr(pParty))
	{
		PASSERT(204);
		return ePartyContentEncoderType;
	}

	PTRACE2(eLevelInfoNormal,"CConf::ConnectPartyToXCodeEncoder After some tests - Party Name: ",pPartyConnection->GetFullName());

	ePartyContentEncoderType = GetPartyXCodeEncoderType(pPartyConnection,pConfParty);
	resourceIt = m_mapXCodeRsrc.find(ePartyContentEncoderType);

	if(ePartyContentEncoderType == eXcodeEncoderDummy || resourceIt == m_mapXCodeRsrc.end())
	{
		PASSERT(205);
		TRACEINTO << " CConf::ConnectPartyToXCodeEncoder - Fails to connect party to XCode Encoder, Encoder Type: " <<  eXcodeRsrcTypeNames[ePartyContentEncoderType] << " Party Name: " << pPartyConnection->GetFullName() << "\n";

		return eXcodeEncoderDummy;
	}

	// Connectparty to the correct XCode encoder
	CBridgePartyVideoOutParams* pBridgePartyVideoOutParams = new CBridgePartyVideoOutParams;
	InitPartyXCodeParamsAccordingToEncoderType(ePartyContentEncoderType, pBridgePartyVideoOutParams);
	pBridgePartyVideoOutParams->SetXCodeConnectionId(resourceIt->second->allocInd.allocIndBase.allocatedRrcs[0].connectionId);
	pBridgePartyVideoOutParams->SetXCodeResourceIndex(ePartyContentEncoderType);
	pBridgePartyVideoOutParams->SetXCodePartyId(resourceIt->second->allocInd.allocIndBase.rsrc_party_id);
	CBridgePartyInitParams* pBrdgPartyInitParams = new CBridgePartyInitParams(pPartyCntl->GetName(), pParty,
			pPartyCntl->GetPartyRsrcId(), pConfParty->GetRoomId(),
			pPartyCntl->GetInterfaceType(),
			NULL, pBridgePartyVideoOutParams,
			NULL,
			pPartyCntl->GetSiteName(), pPartyCntl->GetPartyCascadeType());
	m_pContentXcodeBridgeInterface->ConnectParty(pBrdgPartyInitParams);
	pPartyCntl->SetNewXCodeBridgeInterface(m_pContentXcodeBridgeInterface);
	delete pBrdgPartyInitParams;

	return ePartyContentEncoderType;
}
///////////////////////////////////////////////////////////////
void CConf::DowngradeConferenceFromXCodeToVSW()
{
	if(!m_pCommConf->GetIsCascadeOptimized())
	{
		m_pCommConf->SetPresentationProtocol(eH264Dynamic);
		PTRACE2(eLevelInfoNormal,"CConf::DowngradeConferenceFromXCodeToVSW to H264 Content, Conf Name:  ",m_name);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CConf::DowngradeConferenceFromXCodeToVSW to H264 Cacscade Optimized Content, Conf Name:  ",m_name);
		m_pCommConf->SetPresentationProtocol(eH264Fix);
	}

	m_pCommConf->SetContentMultiResolutionEnabled(NO);

	// 1. Select Content Rate
	//  Highest Common mechanism for content rate
	BYTE selectedContentRateAMC = SelectContentRate(TRUE); //TRUE->update actual ip rate for content

	// if there is no parties able to open content
	if (selectedContentRateAMC == AMC_0k)
	{
		PTRACE2(eLevelInfoNormal,"CConf::DowngradeConferenceFromXCodeToVSW Selected content rate is 0, Dismiss Content token, Conf Name:  ",m_name);
		CConfApi *pSelfApi = new CConfApi;
		pSelfApi->CreateOnlyApi(GetRcvMbx(), this);
		pSelfApi->ContentTokenWithdraw();
		pSelfApi->DestroyOnlyApi();
		POBJDELETE(pSelfApi);
		return;
	}

	BOOL isHDContent1080Supported = FALSE;

	BYTE bContentAsVideo = m_pCommConf->IsLegacyShowContentAsVideo();
	eEnterpriseMode ContRatelevel = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
	ePresentationProtocol ContProtocol = (ePresentationProtocol)m_pCommConf->GetPresentationProtocol();
	//Set max content rate by conf rate
	BYTE lConfRate = m_pCommConf->GetConfTransferRate();
	BYTE HDResMpi = 0;
	isHDContent1080Supported = SelectContentHDResolution(selectedContentRateAMC,H264,HDResMpi,TRUE);
	SetContentMode(lConfRate,ContRatelevel,ContProtocol,m_pCommConf->GetCascadeOptimizeResolution(), cmCapReceiveAndTransmit,bContentAsVideo,isHDContent1080Supported,HDResMpi);

	ActionsOnStartingContentSession(selectedContentRateAMC,TRUE);

}

///////////////////////////////////////////////////////////////
CPartyRsrcDesc* CConf::GetXCodeContentDecoderPartyRsrc()
{
	CPartyRsrcDesc* pPartyContentDecoderdRsrcDesc = NULL;
	XCODE_RSRC::iterator resourceIt;

	resourceIt = m_mapXCodeRsrc.find(eXcodeContentDecoder);

	if(resourceIt == m_mapXCodeRsrc.end())
	{
		// error handling - TBD
		PASSERT(1);
	}
	else
	{
		ALLOC_PARTY_PARAMS_S* pAllocPartyParams = resourceIt->second;
		ContentXcodeRsrcDesc* pContentXcodeRsrcDesc = new ContentXcodeRsrcDesc;

		pContentXcodeRsrcDesc->connectionId = pAllocPartyParams->allocInd.allocIndBase.allocatedRrcs[0].connectionId;
		pContentXcodeRsrcDesc->logicalRsrcType = pAllocPartyParams->allocInd.allocIndBase.allocatedRrcs[0].logicalRsrcType;
		pContentXcodeRsrcDesc->rsrcEntityId = pAllocPartyParams->allocInd.allocIndBase.rsrc_party_id;
		pContentXcodeRsrcDesc->videoPartyType = pAllocPartyParams->allocInd.allocIndBase.videoPartyType;
		pContentXcodeRsrcDesc->monitorRsrcPartyId = pAllocPartyParams->partyId;
		pPartyContentDecoderdRsrcDesc = new CPartyRsrcDesc;
		pPartyContentDecoderdRsrcDesc->SetConfRsrcId(m_ConfRsrcId);
		pPartyContentDecoderdRsrcDesc->SetPartyRsrcId(pContentXcodeRsrcDesc->rsrcEntityId);
		pPartyContentDecoderdRsrcDesc->SetVideoPartyType(pContentXcodeRsrcDesc->videoPartyType);
		CRsrcDesc* pRsrcDesc = new CRsrcDesc(pContentXcodeRsrcDesc->connectionId,pContentXcodeRsrcDesc->logicalRsrcType);
		PTRACE2INT(eLevelInfoNormal,"CConf::GetXCodeContentDecoderPartyRsrc, Content Decoder Rsrc Party ID: ",pPartyContentDecoderdRsrcDesc->GetPartyRsrcId());
		pPartyContentDecoderdRsrcDesc->AddNewRsrcDesc(pRsrcDesc);
		PDELETE(pContentXcodeRsrcDesc);
		PDELETE(pRsrcDesc);
	}
	return pPartyContentDecoderdRsrcDesc;
}
///////////////////////////////////////////////////////////////
DWORD CConf::GetContentDecoderXCodeMonitorPartyId()
{
	DWORD contentDecoderXCodeMonitorPartyId = 0xFFFFFFFF;
	XCODE_RSRC::iterator resourceIt;

	resourceIt = m_mapXCodeRsrc.find(eXcodeContentDecoder);

	if(resourceIt == m_mapXCodeRsrc.end())
	{
		// error handling - TBD
		PASSERT(1);
	}
	else
	{
		ALLOC_PARTY_PARAMS_S* pAllocPartyParams = resourceIt->second;
		contentDecoderXCodeMonitorPartyId = pAllocPartyParams->partyId;
	}
	return contentDecoderXCodeMonitorPartyId;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
eXcodeRsrcType CConf::GetPartyXCodeEncoderType(CPartyConnection* pPartyConnection, CConfParty* pConfParty)
{
	PASSERT_AND_RETURN_VALUE(!pPartyConnection, eXcodeEncoderDummy);
	PASSERT_AND_RETURN_VALUE(!pConfParty, eXcodeEncoderDummy);

	CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
	PASSERT_AND_RETURN_VALUE(!pPartyCntl, eXcodeEncoderDummy);

	CTaskApp* pParty = pPartyCntl->GetPartyTaskApp();
	PASSERT_AND_RETURN_VALUE(!pParty, eXcodeEncoderDummy);

	TRACECOND_AND_RETURN_VALUE(pPartyCntl->IsLegacyContentParty(), pPartyCntl->GetFullName() << " - Legacy Party", eXcodeEncoderDummy);
	TRACECOND_AND_RETURN_VALUE(pPartyCntl->IsLync(), pPartyCntl->GetFullName() << " - Lync Party", eXcodeEncoderDummy);

	eXcodeRsrcType ePartyContentEncoderType = eXcodeEncoderDummy;

	if (!pPartyCntl->IsRemoteAndLocalHasHDContent720() && !pPartyCntl->IsRemoteAndLocalHasHDContent1080())
	{
		if (pPartyCntl->IsRemoteAndLocalCapSetHasContent(eToPrintOnFalseOnly))
		{
			ePartyContentEncoderType = eXcodeH263Encoder;
		}
	}
	else if (pConfParty->GetCascadeMode() != CASCADE_MODE_NONE)
	{
		ePartyContentEncoderType = eXcodeH264LinksEncoder;
	}
	else
	{
		ePartyContentEncoderType = eXcodeH264Encoder;
	}

	TRACEINTO << pPartyCntl->GetFullName() << ", PartyXCodeEncoderType:" << eXcodeRsrcTypeNames[ePartyContentEncoderType];

	return ePartyContentEncoderType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CConf::SelectMaxContentRateForXCodeDecoder()
{
	BYTE lConfRate = m_pCommConf->GetConfTransferRate();
	eEnterpriseMode ContRatelevel = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
	DWORD maxXCodeDecoderConteRate = 0;
	BYTE  maxConfContentRateAMC = AMC_0k;
	BYTE  maxH264LinkEncoderRateAMC = AMC_0k;
	BYTE  maxCalcConfContentRateAMC = AMC_0k;

	if(m_pCommConf->GetContentMultiResolutionEnabled())
	{
		maxConfContentRateAMC =  m_pUnifiedComMode->FindAMCContentRateByLevel(lConfRate,(eConfMediaType)(m_pCommConf->GetConfMediaType()), ContRatelevel, eH264Dynamic, m_pCommConf->GetCascadeOptimizeResolution());
		if(m_pCommConf->GetIsCascadeOptimized())
		{
			maxH264LinkEncoderRateAMC = m_pUnifiedComMode->FindAMCContentRateByLevel(lConfRate,(eConfMediaType)(m_pCommConf->GetConfMediaType()), ContRatelevel, eH264Fix, m_pCommConf->GetCascadeOptimizeResolution());
		}
		maxCalcConfContentRateAMC = max(maxConfContentRateAMC,maxH264LinkEncoderRateAMC);
		maxXCodeDecoderConteRate  = 100*TranslateAMCRateIPRate(maxCalcConfContentRateAMC);
	}

	TRACEINTO << "CConf::SelectMaxContentRateForXCodeDecoder, Selected Content Decoder Rate:" << maxXCodeDecoderConteRate << " Conf Name: " << m_name << "\n";
	return maxXCodeDecoderConteRate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::InitConfMediaState()
{
  bool isDynamicAllocation = IsMixAvcSvcDynamicAllocation();
  eConfMediaType confMediaType = m_pCommConf->GetConfMediaType();

  if(isDynamicAllocation && eMixAvcSvc == confMediaType){
    SetMediaState(eMediaStateEmpty);
  }else{
    SetMediaStateByMediaType();
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::UpdateConfMediaState(eConfMediaState state)
{
  bool isDynamicAllocation = IsMixAvcSvcDynamicAllocation();
  eConfMediaType confMediaType = m_pCommConf->GetConfMediaType();

  if(isDynamicAllocation && eMixAvcSvc == confMediaType){
    SetMediaState(state);
  }
  // do nothing - no need to update
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CConf::IsMixAvcSvcDynamicAllocation() const
{
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	PASSERT_AND_RETURN_VALUE(!pSysConfig, false);

	bool bIsAvcSvcDymamicAllocation = true;
	pSysConfig->GetBOOLDataByKey("MIX_AVC_SVC_DYNAMIC_ALLOCATION", bIsAvcSvcDymamicAllocation);
	return bIsAvcSvcDymamicAllocation;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetMediaStateByMediaType()
{
  eConfMediaType confMediaType = m_pCommConf->GetConfMediaType();
  switch(confMediaType){
  case eMixAvcSvc:
    SetMediaState(eMediaStateMixAvcSvc);
    break;
  case eAvcOnly:
    SetMediaState(eMediaStateAvcOnly);
    break;
  case eSvcOnly:
  case eMixAvcSvcVsw:
    SetMediaState(eMediaStateSvcOnly);
    break;
  default:
    SetMediaState(eMediaStateEmpty);
    break;
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetMediaState(eConfMediaState state)
{
	if (eMediaStateMixAvcSvc == m_confMediaState)
	{
		TRACEINTO << "OldMediaState:" << MediaStateToString(m_confMediaState) << ", NewMediaState:" << MediaStateToString(state) << " - We don't change ConfMediaState after it already in mixed";
	}
	else
	{
		TRACEINTO << "OldMediaState:" << MediaStateToString(m_confMediaState) << ", NewMediaState:" << MediaStateToString(state);
		m_confMediaState = state;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CConf::GetContentModeAMC()
{
	DWORD confRate = 0;
	BYTE lConfRate = 0;
	eEnterpriseMode ContRatelevel = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
	if (confRate)
		lConfRate = m_pUnifiedComMode->TranslateIPRateToXferRate(confRate);
	else
		lConfRate = m_pCommConf->GetConfTransferRate();

	ePresentationProtocol presentationProtocol = (ePresentationProtocol)m_pCommConf->GetPresentationProtocol();

	return m_pUnifiedComMode->GetContentModeAMC(lConfRate,ContRatelevel, presentationProtocol, m_pCommConf->GetCascadeOptimizeResolution(),m_pCommConf->GetConfMediaType());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CConf::GetLocalContentModeAMCInIPRate(CConfParty* pConfParty, DWORD setupRate)
{
	DWORD reservationVideoRate = pConfParty->GetVideoRate();
	DWORD confRate = 0;
	BYTE lConfRate = 0;
	eEnterpriseMode ContRatelevel = (eEnterpriseMode)m_pCommConf->GetEnterpriseMode();
	if (confRate)
		lConfRate = m_pUnifiedComMode->TranslateIPRateToXferRate(confRate);
	else
		lConfRate = m_pCommConf->GetConfTransferRate();

	CCapSetInfo lCapInfo((payload_en)lConfRate, 0);
	DWORD h323ConfRate = lCapInfo.TranslateReservationRateToIpRate(lConfRate);

	TRACEINTO << "H323ConfRate:" << h323ConfRate << ", H323ReservationRate:" << reservationVideoRate << ", setupRate:" << setupRate;

	DWORD H323AMCRate = 0;
	ePresentationProtocol presentationProtocol = (ePresentationProtocol)m_pCommConf->GetPresentationProtocol();

	//FSN-613: Dynamic Content for SVC/Mix Conf: in case partycontentRate < confcontentRate, no need to open content channels. It can be improved in future
	if (reservationVideoRate == 0xFFFFFFFF && pConfParty->GetPartyMediaType() == eSvcPartyType  && presentationProtocol == eH264Fix  && setupRate > 0 && setupRate < h323ConfRate*1000 )
	{
		reservationVideoRate = setupRate/1000;
		TRACEINTO << "updated H323ReservationRate from setup:" << reservationVideoRate;
	}

	if (reservationVideoRate != 0xFFFFFFFF)
	{
		if (reservationVideoRate != h323ConfRate)
		{
			DWORD reservationRate = TranslateVideoIpRateToCorrectReservationRate(reservationVideoRate, h323ConfRate);
			if (reservationRate == 0)
			{
				PASSERTMSG(reservationRate, "CConf::ConnectIpParty - Problem with conditioning");
				H323AMCRate = m_pUnifiedComMode->GetContentModeAMCInIPRate(lConfRate, ContRatelevel, presentationProtocol, m_pCommConf->GetCascadeOptimizeResolution(), m_pCommConf->GetConfMediaType());
			}
			else
			{
				BYTE XferResRate = m_pUnifiedComMode->TranslateIPRateToXferRate(reservationRate * 1000);
				H323AMCRate = m_pUnifiedComMode->GetContentModeAMCInIPRate(XferResRate, ContRatelevel, presentationProtocol, m_pCommConf->GetCascadeOptimizeResolution(), m_pCommConf->GetConfMediaType());
			}
		}
		else
			H323AMCRate = m_pUnifiedComMode->GetContentModeAMCInIPRate(lConfRate, ContRatelevel, presentationProtocol, m_pCommConf->GetCascadeOptimizeResolution(), m_pCommConf->GetConfMediaType());
	}
	else
	{
		H323AMCRate = m_pUnifiedComMode->GetContentModeAMCInIPRate(lConfRate, ContRatelevel, presentationProtocol, m_pCommConf->GetCascadeOptimizeResolution(), m_pCommConf->GetConfMediaType());
		if (pConfParty->GetIsLyncPlugin()) //BRIDGE-9748
		{
			// Quantize the setupRate to AMC rate, (there's no video for plugin, we use the lower level for content)
			BYTE xferSetupRate = m_pUnifiedComMode->TranslateIPRateToXferRateForCss(setupRate);
			// Reserve audio rate for plugin by decrease one grade
			DWORD ipSetupRate = m_pUnifiedComMode->TranslateXferRateToIpRate(xferSetupRate);
			PTRACE2INT(eLevelInfoNormal, "CConf::GetLocalContentModeAMCInIPRate : css:  ipSetupRate - ", ipSetupRate);
			H323AMCRate = min(H323AMCRate, (ipSetupRate * 10));
		}

	}



	PTRACE2INT(eLevelInfoNormal, "CConf::GetLocalContentModeAMCInIPRate :  H323AMCRate rate - ", H323AMCRate);
	return H323AMCRate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetControlDataParams(PartyControlDataParameters& partyControlDataParams, CConfParty* pConfParty, CIpNetSetup* pIpNetSetup, CTaskApp* pParty, CSipCaps* pRmtSipCaps,
                                 BYTE bIsOffer, BYTE bIsMrcHeader, BYTE bIsWebRtcCall, RemoteIdent epType,LyncConnType lyncEpType ,DWORD roomID, eTypeOfLinkParty partyType, DWORD setupRate, eIsUseOperationPointsPreset isUseOperationPointesPresets)
{
	BYTE newContentProtocol = SelectContentProtocol(FALSE);
	BYTE HDResMpi = 0;
	BYTE LinkCascadeMode = pConfParty->GetCascadeMode(); //BRIDGE-10391
	BOOL isHighestCommonRequired  = IsTIPContentEnable() ? FALSE : TRUE ; // in case of TIP - content will be fix on 512K VGA, no need for HighestCommon
	eXcodeRsrcType ePartyContentEncoderType = (m_pCommConf->GetContentMultiResolutionEnabled() && LinkCascadeMode!=CASCADE_MODE_NONE)? eXcodeH264LinksEncoder : eXcodeEncoderDummy;
	BYTE isHDContent1080Supported = SelectContentHDResolution(GetContentModeAMC(), newContentProtocol, HDResMpi, isHighestCommonRequired, ePartyContentEncoderType);

	partyControlDataParams.bIsEnableH239                  = IsEnableH239();
	partyControlDataParams.contentCap                     = GetCurrentContentProtocolInIpValues();
	partyControlDataParams.presentationProtocol           = (ePresentationProtocol)m_pCommConf->GetPresentationProtocol();
	partyControlDataParams.bIsHDContent1080Supported      = isHDContent1080Supported;
	partyControlDataParams.bIsTIPContentEnable            = IsTIPContentEnable();
	partyControlDataParams.h323MaxContentAMCRate          = GetLocalContentModeAMCInIPRate(pConfParty, setupRate);
	partyControlDataParams.hdResMpi                       = HDResMpi;
	partyControlDataParams.vidQuality                     = GetCommConf()->GetVideoQuality();
	partyControlDataParams.maxConfResolution              = m_pCommConf->GetConfMaxResolution();
	partyControlDataParams.callRate                       = m_pUnifiedComMode->GetCallRate();
	partyControlDataParams.pCOPConfigurationList          = m_pCommConf->GetCopConfigurationList();
	partyControlDataParams.contentProtocol                = GetCurrentContentProtocolInIpValues();
	partyControlDataParams.pParty                         = pParty;
	partyControlDataParams.pIpNetSetup                    = pIpNetSetup;
	partyControlDataParams.networkType                    = m_pCommConf->GetNetwork();
	partyControlDataParams.bIsOffer                       = bIsOffer;
	partyControlDataParams.pSipRmtCaps                    = pRmtSipCaps;
	partyControlDataParams.confMediaType                  = (eConfMediaType)m_pCommConf->GetConfMediaType();
	partyControlDataParams.pConfOperationPointsSet        = m_pConfOperationPointsSet;
	partyControlDataParams.bIsMrcHeader                   = bIsMrcHeader;
	partyControlDataParams.bIsMrcCall                     = GetIsMrcCall(bIsMrcHeader, pConfParty);
	partyControlDataParams.bIsWebRtcCall             = bIsWebRtcCall;
	partyControlDataParams.bIsLync                        = (lyncEpType == Lync || lyncEpType == Lync2013 || lyncEpType == AvMcuLync2013Slave);
	partyControlDataParams.epType                         = epType;
	partyControlDataParams.lyncEpType                     = lyncEpType;//this identify the mode we are working with in addition to remoteident!
	partyControlDataParams.roomID                         = roomID;
	partyControlDataParams.pStrPartyName                  = (char*)pConfParty->GetName();
	partyControlDataParams.linkType                       = partyType;
	partyControlDataParams.bIsPreferTIP                   = m_pCommConf->GetIsPreferTIP();
	partyControlDataParams.enterpriseMode                 = m_pCommConf->GetEnterpriseMode();
	partyControlDataParams.bContentMultiResolutionEnabled = m_pCommConf->GetContentMultiResolutionEnabled();
	partyControlDataParams.bContentXCodeH263Supported     = m_pCommConf->GetContentXCodeH263Supported();
	partyControlDataParams.bIsASSIPcontentEnable          = m_IsAsSipContentEnable;
	partyControlDataParams.isUseOperationPointesPresets   = isUseOperationPointesPresets;
	partyControlDataParams.bIsHighProfileContent          = GetCurrentContentIsHighProfile(); //HP content
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::AjustPartyAndRoomIdByLinkType(PartyControlDataParameters& partyControlDataParams, CConfParty* pConfParty)
{
	char mainPartyName[H243_NAME_LEN];
	memset(mainPartyName, '\0', H243_NAME_LEN);

	if (pConfParty &&
			pConfParty->GetCascadedLinksNumber() > 1 &&
			pConfParty->GetCascadeMode() != CASCADE_MODE_NONE &&
			partyControlDataParams.linkType != eSubLinkParty)
	{
		partyControlDataParams.linkType = eMainLinkParty;
		pConfParty->SetPartyType(eMainLinkParty);

		::CreateMainLinkName(pConfParty->GetName(), mainPartyName);

		TRACEINTO << "MainPartyName:" << mainPartyName;

		pConfParty->SetName(mainPartyName);
		partyControlDataParams.pStrPartyName = (char*)pConfParty->GetName();
	}

	if (pConfParty && pConfParty->GetPartyType() == eSubLinkParty)
	{
		::GetMainLinkName(partyControlDataParams.pStrPartyName, mainPartyName);

		CConfParty* pConfPartyOfMain = m_pCommConf->GetCurrentParty(mainPartyName);
		if (pConfPartyOfMain != NULL)
		{
			partyControlDataParams.roomID = pConfPartyOfMain->GetRoomId();
		}
		else
		{
			partyControlDataParams.roomID = 0;
			TRACESTRFUNC(eLevelError) << "MainPartyName:" << partyControlDataParams.pStrPartyName << " - Not found";
		}

		// mute the audio transmit:
		pConfParty->SetAudioBlocked(TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetControlInitParams(PartyControlInitParameters& partyControInitParam, CConfParty* pConfParty, BYTE bIsRedial, const char* avServiceNameStr,
                                 WORD welcomeMsgTime, DWORD connectDelay, DWORD redialInterval, COsQueue* pPartyRcvMbx)
{
	partyControInitParam.pCopVideoTxModes          = m_pUnifiedComMode->GetCopVideoTxModes();
	partyControInitParam.pConf                     = this;
	partyControInitParam.bIsGateWay                = GetIsGateWay();
	partyControInitParam.bIsStreaming              = GetCommConf()->GetIsStreaming();
	partyControInitParam.bIsTipCompatibleVideo     = GetCommConf()->GetIsTipCompatibleVideo();
	partyControInitParam.connectDelay              = connectDelay;
	partyControInitParam.bIsRedial                 = bIsRedial;
	partyControInitParam.redialInterval            = redialInterval;
	partyControInitParam.monitorConfId             = GetMonitorConfId();
	partyControInitParam.pAvServiceNameStr         = avServiceNameStr;
	partyControInitParam.pConfRcvMbx               = m_pRcvMbx;
	partyControInitParam.pPartyRcvMbx              = pPartyRcvMbx;
	partyControInitParam.pAudioBridgeInterface     = m_pAudBrdgInterface;
	partyControInitParam.pConfAppMngrInterface     = m_pConfAppMngrInterface;
	partyControInitParam.pVideoBridgeInterface     = m_pVideoBridgeInterface;
	partyControInitParam.pContentBridge            = m_pContentBridge;
	partyControInitParam.pFECCBridge               = m_pFECCBridge;
	partyControInitParam.pTerminalNumberingManager = m_pTerminalNumberingManager;
	partyControInitParam.pStrAppoitnmentID         = GetCommConf()->GetAppointmentId();
	partyControInitParam.pStrConfGuid              = m_aConfGUID;
	partyControInitParam.pStrConfName              = m_name;
	partyControInitParam.TcMode                    = m_TcMode;
	partyControInitParam.voiceType                 = GetVoiceType(pConfParty);
	partyControInitParam.welcomeMsgTime            = welcomeMsgTime;
	partyControInitParam.pConfParty                = pConfParty;
	partyControInitParam.pStrAppointmentId         = (char*)GetCommConf()->GetAppointmentId();
	partyControInitParam.bIsEncript                = GetCommConf()->GetIsEncryption();
	partyControInitParam.encryptionType            = GetCommConf()->GetEncryptionType();
	partyControInitParam.pConfIpScm                = m_pUnifiedComMode->GetIPComMode();
	partyControInitParam.termNum                   = 0;

	WORD voiceType    = 0;
	WORD bIsAudioOnly = pConfParty->GetVoice();
	if (bIsAudioOnly)
		voiceType = GetVoiceType(pConfParty);

	BYTE StandByStart = m_StandByStart;
	// 3289 - 17800 - 16965
	if (pConfParty->GetRecordingLinkParty() || pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone)
		StandByStart = 0; // Recording link party should connect automatically

	partyControInitParam.voiceType    = voiceType;
	partyControInitParam.standByStart = StandByStart;


}

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
int CConf::GetTaskMbxBufferSize() const // VNGFE-7734 - from system cfg flag
{
	int buffer_config_mb = 1;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetIntDataByKey(CFG_KEY_CONF_TASK_RECEIVE_BUFFER_SIZE, buffer_config_mb);
	int buffer_size = buffer_config_mb*1024*1024-1;
	TRACESTRFUNC(eLevelError) << " buffer_size = " << buffer_size;
	return buffer_size;
}

int CConf::GetTaskMbxSndBufferSize() const //  VNGFE-7734 - from system cfg flag
{
	int buffer_config_mb = 1;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetIntDataByKey(CFG_KEY_CONF_TASK_SEND_BUFFER_SIZE, buffer_config_mb);
	int buffer_size = buffer_config_mb*1024*1024-1;
	TRACESTRFUNC(eLevelError) << " buffer_size = " << buffer_size;
	return buffer_size;
}
#endif // MESSAGE_QUEUE_BLOCKING_RESERCH
