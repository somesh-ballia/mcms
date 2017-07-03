//////////////////////////////////////////////////////////////////////
//
// EndpointsSimManager.cpp: implementation of the CEndpointsSimManager class.
//
//////////////////////////////////////////////////////////////////////


#include "EndpointsSimManager.h"

#include <sstream>
#include "Macros.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "IpCsOpcodes.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsBonding.h"
#include "OpcodesMcmsMux.h"
#include "OpcodesMcmsMux.h"

#include "XmlEngineApi.h"
#include "ListenSocketApi.h"
#include "DummyEntry.h"
#include "Request.h"
#include "MplMcmsProtocol.h"
#include "SystemFunctions.h"
#include "psosxml.h"
#include "OperatorDefines.h"
#include "TerminalCommand.h"
#include "ManagerApi.h"

#include "SimApi.h"
#include "EndpointsGuiApi.h"
#include "EndpointsSim.h"
#include "EndpointsSimConfig.h"
#include "EpSimEndpointsTask.h"
#include "CommEndpointsSimSet.h"
#include "EpGuiRxSocket.h"
#include "EpGuiTxSocket.h"
#include "EpSimCapSetsList.h"
#include "ZipWrapper.h"
#include "ApiStatuses.h"
#include "PostXmlHeader.h"
#include "UnicodeDefines.h"
#include "TBStructs.h"
#include "CSSimCluster.h"
#include "Bonding.h"

#include "CSIDTaskApp.h"

extern void EndpointsSimMonitorEntryPoint(void* appParam);
extern "C" void epSimScriptsEntryPoint(void* appParam);

//  Etxernal DB Keep Alive timer
const OPCODE EPSIM_EXTERNAL_DB_KEEP_ALIVE_TOUT = 20501;
const DWORD  EPSIM_EXTERNAL_DB_KEEP_ALIVE_TIME = 12 * SECOND * 60; // once per 12 minutes

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CEndpointsSimManager)

	ONEVENT( OPEN_SOCKET_CONNECTION,  ANYCASE,  CEndpointsSimManager::OnListenSocketOpenConnection)
	ONEVENT( CLOSE_SOCKET_CONNECTION, ANYCASE,  CEndpointsSimManager::OnListenSocketCloseConnection)
//	ONEVENT( SIM_API_CS_MSG,  ANYCASE,  CEndpointsSimManager::OnGideonCSMsgAnystate)
	ONEVENT( GUI_SOCKET_RCV_MSG,  ANYCASE,  CEndpointsSimManager::OnGuiMsgAnycase)
	ONEVENT( SC_MSG_TO_MNGR,      ANYCASE,  CEndpointsSimManager::OnCsMsgAnystate)

		// messages from GideonSim process
	//ONEVENT( SIM_API_AUDIO_MSG,  ANYCASE, CEndpointsSimManager::OnGideonAudioMsgAnystate)
		// messages from GideonSim process
	ONEVENT( SIM_API_AUDIO_MSG,  ANYCASE, CEndpointsSimManager::OnGideonAudioMsgAnystate)
	ONEVENT( SIM_API_ISDN_MSG,  ANYCASE,  CEndpointsSimManager::OnGideonIsdnMsgAnystate)
	ONEVENT( SIM_API_MUX_MSG,  ANYCASE,  CEndpointsSimManager::OnGideonAudioMsgAnystate)
	ONEVENT( SIM_API_MRM_MSG,  ANYCASE,  CEndpointsSimManager::OnGideonMRMMsgAnystate)

		// External DB keep_alive timer
		// External DB keep_alive timer
	ONEVENT( EPSIM_EXTERNAL_DB_KEEP_ALIVE_TOUT, ANYCASE, CEndpointsSimManager::OnExtDbKeppAliveTout)

PEND_MESSAGE_MAP(CEndpointsSimManager,CManagerTask)


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CEndpointsSimManager)
		// H323 party commands
	ON_TRANS("TRANS_SIMULATION", "H323_PARTY_ADD",        CCommSetPartyAdd,    CEndpointsSimManager::OnTransH323PartyAdd)
	ON_TRANS("TRANS_SIMULATION", "H323_PARTY_DEL",        CCommSetPartyCommon, CEndpointsSimManager::OnTransPartyDel)
	ON_TRANS("TRANS_SIMULATION", "H323_PARTY_CONNECT",    CCommSetPartyCommon, CEndpointsSimManager::OnTransPartyConnect)
	ON_TRANS("TRANS_SIMULATION", "H323_PARTY_DISCONNECT", CCommSetPartyCommon, CEndpointsSimManager::OnTransPartyDisconnect)
	ON_TRANS("TRANS_SIMULATION", "H323_PARTY_DTMF",       CCommSetPartyDtmf,   CEndpointsSimManager::OnTransPartyDtmf)
	ON_TRANS("TRANS_SIMULATION", "H323_PARTY_BITRATE_ERROR", CCommSetBitRateError, CEndpointsSimManager::OnTransH323PartyErrorBitrate)
		// SIP party commands
	ON_TRANS("TRANS_SIMULATION", "SIP_PARTY_ADD",        CCommSetPartyAddSipEP,    CEndpointsSimManager::OnTransSipPartyAdd)
	ON_TRANS("TRANS_SIMULATION", "SIP_PARTY_DEL",        CCommSetPartyCommon, CEndpointsSimManager::OnTransPartyDel)
	ON_TRANS("TRANS_SIMULATION", "SIP_PARTY_CONNECT",    CCommSetPartyCommon, CEndpointsSimManager::OnTransPartyConnect)
	ON_TRANS("TRANS_SIMULATION", "SIP_PARTY_DISCONNECT", CCommSetPartyCommon, CEndpointsSimManager::OnTransPartyDisconnect)
	ON_TRANS("TRANS_SIMULATION", "SIP_PARTY_DTMF",       CCommSetPartyDtmf,   CEndpointsSimManager::OnTransPartyDtmf)
	ON_TRANS("TRANS_SIMULATION", "SIP_PARTY_CHANGEMODE", CCommSetPartyDtmf,   CEndpointsSimManager::OnTransPartyChangeMode)
	ON_TRANS("TRANS_SIMULATION", "SIP_MUTE",             CCommSetPartyMute,	  CEndpointsSimManager::OnTransSipMute)
		// PSTN party commands
	ON_TRANS("TRANS_SIMULATION", "PSTN_PARTY_ADD",       CCommSetAddIsdnEndpoint, CEndpointsSimManager::OnTransPstnPartyAdd)
	ON_TRANS("TRANS_SIMULATION", "ISDN_PARTY_ADD",       CCommSetAddIsdnEndpoint, CEndpointsSimManager::OnTransIsdnPartyAdd)
		// common - for all endpoint types
	ON_TRANS("TRANS_SIMULATION", "PARTY_CONNECT",        CCommSetPartyCommon, CEndpointsSimManager::OnTransPartyConnect)
	ON_TRANS("TRANS_SIMULATION", "PARTY_DISCONNECT",     CCommSetPartyCommon, CEndpointsSimManager::OnTransPartyDisconnect)
	ON_TRANS("TRANS_SIMULATION", "PARTY_DELETE",         CCommSetPartyCommon, CEndpointsSimManager::OnTransPartyDel)
	ON_TRANS("TRANS_SIMULATION", "PARTY_DTMF",           CCommSetPartyDtmf,   CEndpointsSimManager::OnTransPartyDtmf)
	ON_TRANS("TRANS_SIMULATION", "ADD_CAP_SET",          CCommSetAddCapset,   CEndpointsSimManager::OnTransAddCapset)
	ON_TRANS("TRANS_SIMULATION", "DEL_CAP_SET",          CCommSetDelCapset,	  CEndpointsSimManager::OnTransDelCapset)
	ON_TRANS("TRANS_SIMULATION", "ACTIVE_SPEAKER",       CCommSetPartyCommon, CEndpointsSimManager::OnTransActiveSpeaker)
	ON_TRANS("TRANS_SIMULATION", "AUDIO_SPEAKER",        CCommSetPartyCommon, CEndpointsSimManager::OnTransAudioSpeaker)
	ON_TRANS("TRANS_SIMULATION", "MUTE",                 CCommSetPartyCommon, CEndpointsSimManager::OnTransMute)
	ON_TRANS("TRANS_SIMULATION", "UNMUTE",               CCommSetPartyCommon, CEndpointsSimManager::OnTransUnmute)
	ON_TRANS("TRANS_SIMULATION", "FECC_TOKEN_REQUEST",   CCommSetPartyCommon, CEndpointsSimManager::OnTransFeccTokenRequest)
	ON_TRANS("TRANS_SIMULATION", "FECC_TOKEN_RELEASE",   CCommSetPartyCommon, CEndpointsSimManager::OnTransFeccTokenRelease)
	ON_TRANS("TRANS_SIMULATION", "H239_TOKEN_REQUEST",   CCommSetPartyCommon, CEndpointsSimManager::OnTransH239TokenRequest)
	ON_TRANS("TRANS_SIMULATION", "H239_TOKEN_RELEASE",   CCommSetPartyCommon, CEndpointsSimManager::OnTransH239TokenRelease)
	ON_TRANS("TRANS_SIMULATION", "FECC_KEY_REQUEST",     CCommSetPartyFecc,   CEndpointsSimManager::OnTransFeccKeyRequest)


	ON_TRANS("TRANS_SIMULATION", "LPR_MODE_CHANGE_REQUEST",   CCommSetLprModeChangeReq, CEndpointsSimManager::OnTransLprModeChangeRequest)

	ON_TRANS("TRANS_SIMULATION", "SET_PARTY_CAPSET",   	 CCommSetPartyCommon, CEndpointsSimManager::OnTransSetPartyCapset)

	ON_TRANS("TRANS_SIMULATION", "ENDPOINT_UPDATE_CHANNELS", CCommSetPartyUpdateChannels, CEndpointsSimManager::OnTransEndpointUpdateChannels)
	ON_TRANS("TRANS_SIMULATION", "CS_SOCKET_DISCONNECT", CCommSetPartyCommon, CEndpointsSimManager::HandleSocketDisconnectAction)
		// External DB
	ON_TRANS("REQUEST_GENERAL",   "KEEP_ALIVE", CDummyEntry,              CEndpointsSimManager::OnTransExtermalDbKeepAlive)
	ON_TRANS("REQUEST_CONF_DETAILS",  "CREATE",	CCommSetExternalDbCreate, CEndpointsSimManager::OnTransExtermalDbCreate)
	ON_TRANS("REQUEST_PARTY_DETAILS", "ADD",    CCommSetExternalDbAdd,    CEndpointsSimManager::OnTransExtermalDbAdd)
	ON_TRANS("REQUEST_USER_DETAILS",  "AUTHENTICATE", CCommSetExternalDbUser, CEndpointsSimManager::OnTransExtermalDbUser)
		// SIP subscriptions requests
	ON_TRANS("TRANS_SIMULATION",  "SIP_ADD_SUBSCRIPTION", CCommSetAddSubscription, CEndpointsSimManager::OnTransSipAddSubscription)
	ON_TRANS("TRANS_SIMULATION",  "SIP_GET_NOTIFICATION", CCommSetGetNotification, CEndpointsSimManager::OnTransSipGetNotification)
	ON_TRANS("TRANS_SIMULATION",  "SCP_STREAMS_REQUEST", CCommSetScpStreamsRequest, CEndpointsSimManager::OnTransScpStreamsRequest)

END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CEndpointsSimManager)

    ONCOMMAND("start_cs", CEndpointsSimManager::HandleTerminalStartCS, "start_cs [CS_INDEX] [CS_NUM_OF_PORTS]")
    ONCOMMAND("stop_cs", CEndpointsSimManager::HandleTerminalStopCS, "stop_cs [CS_INDEX]")
    ONCOMMAND("list_cs", CEndpointsSimManager::HandleTerminalListCS, "list_cs")
    ONCOMMAND("set_encryption_flag", CEndpointsSimManager::HandleTerminalEncryptionFlag, "set encryption flag for dial-out calls")
// LPR indications
    ONCOMMAND("Send_LPR_Ind", CEndpointsSimManager::HandleTerminalSendLpr, "Send LPR Ind")
    ONCOMMAND("Show_All_Parties", CEndpointsSimManager::HandleTerminalShowAllParties, "Show_All_Parties")


END_TERMINAL_COMMANDS

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void EndpointsSimManagerEntryPoint(void* appParam)
{
	CEndpointsSimManager * pEndpointsSimManager = new CEndpointsSimManager;
	pEndpointsSimManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CEndpointsSimManager::GetMonitorEntryPoint()
{
	return EndpointsSimMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CEndpointsSimManager::CEndpointsSimManager()
{
	m_pEpSimConfig  = NULL;
	m_pListenSocketApi	= NULL;
	m_pEndpointsTaskApi	= NULL;
	m_pScriptsApi      = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CEndpointsSimManager::~CEndpointsSimManager()
{
	POBJDELETE(m_pEpSimConfig);
	POBJDELETE(m_pListenSocketApi);
	POBJDELETE(m_pEndpointsTaskApi);
	POBJDELETE(m_pScriptsApi);
}

////////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::ManagerPostInitActionsPoint()
{
    // this function is called just before WaitForEvent

	int status = ::InitXmlEngine();

	// init application with XML configuration file
	m_pEpSimConfig = new CEndpointsSimSystemCfg;

	// init Scripts Task
    m_pScriptsApi = new CTaskApi;
    m_pScriptsApi->Create(epSimScriptsEntryPoint,GetRcvMbx());

	// init Endpoints Task
	m_pEndpointsTaskApi = new CTaskApi;
	m_pEndpointsTaskApi->Create(epSimEndpointsTaskEntryPoint,GetRcvMbx());

	// create Listen Socket task for GUI negotiation
    m_pListenSocketApi = new CListenSocketApi(EpGuiRxEntryPoint,EpGuiTxEntryPoint,
                                            ::GetEpSystemCfg()->GetGuiPortNumber());
    m_pListenSocketApi->Create(*m_pRcvMbx);

    // make sure that CSMngr process is already running
    for (int i = 1; !CProcessBase::IsProcessAlive(eProcessCSMngr); ++i)
    {
    	PASSERTMSG_AND_RETURN(i >= 5, "CSMngr is not alive, quit by timeout");

    	TRACEINTO << "Wait for CSMngr process is up, tick: " << i << "...";

    	SystemSleep(SECOND);
    }

    // notify CSMngr process about my startup
    CManagerApi api(eProcessCSMngr);
    STATUS stat = api.SendOpcodeMsg(EP_PROCESS_STARTED);

    PASSERTSTREAM(stat, "Unable to notify CSMngr: "
        << CProcessBase::GetProcess()->GetStatusAsString(api.GetLastSendError()));
}

////////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::SelfKill()
{
	PTRACE(eLevelInfoNormal,"CCEndpointsSimManager::SelfKill - Start Killing Tasks");

	if(CPObject::IsValidPObjectPtr(m_pScriptsApi))
	{
        m_pScriptsApi->SyncDestroy();
        POBJDELETE(m_pScriptsApi);
    }

	//SystemSleep(5);
	if(CPObject::IsValidPObjectPtr(m_pListenSocketApi))
	{
	    m_pListenSocketApi->SyncDestroy();
	    POBJDELETE(m_pListenSocketApi);
	}

	//SystemSleep(5);
	if(CPObject::IsValidPObjectPtr(m_pEndpointsTaskApi))
	{
        m_pEndpointsTaskApi->SyncDestroy();
        POBJDELETE(m_pEndpointsTaskApi);
    }

    //SystemSleep(5);
    m_csClusters.Clear();

    DeleteAllTimers();
	CManagerTask::SelfKill();
}

/////////////////////////////////////////////////////////////////////////////
void*  CEndpointsSimManager::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::HandlePostRequest(CSegment* pParamSeg)
{
///
///   This virtual function should check and response only for "External DB requests"
///     like REQUEST_CONF_DETAILS, REQUEST_PARTY_DETAILS, REQUEST_GENERAL
///
///   In all other cases it calls base-class function
///
	STATUS status = STATUS_OK;
	BYTE bCompressed=false;

	CSegment  rCopySeg(*pParamSeg);

	// 1. read return mailbox, len and buffer from segment
	COsQueue dualMbx; // mailbox of the dual task (Tx)
	dualMbx.DeSerialize(rCopySeg);
// 	WORD nAuthorization = GUEST;
// 	DWORD len;

    CPostXmlHeader postXmlHeader;
    postXmlHeader.DeSerialize(rCopySeg);

    DWORD len = postXmlHeader.GetLen();
    char * pContent = new char[len + 1];
    rCopySeg.Get((BYTE*)pContent, len);
    pContent[len] = '\0';

    WORD nAuthorization = postXmlHeader.GetAuthorization();
    bCompressed = postXmlHeader.GetIsCompressed();


//	rCopySeg >> len;

/*	char * pContent = new char[len+1];
	rCopySeg >> pContent;
	rCopySeg >> nAuthorization;

	ReceiveAdditionalParams(&rCopySeg);*/

// 	char * pContent = new char[len+1];
// 	rCopySeg.Get((BYTE*)pContent,len);
//     pContent[len] = '\0';

// 	rCopySeg >> nAuthorization;
// 	rCopySeg >> bCompressed;

	ReceiveAdditionalParams(&rCopySeg);

	if (bCompressed)
	{
		CZipWrapper ZipWrapper;

        const int pszNonZippedBufferLen = (/*MAX_NON_ZIPPED*/len * 100) + 1;
		char* pszUnZippedBuffer = new char[pszNonZippedBufferLen];
		int nUnZippedLen = ZipWrapper.Inflate((unsigned char*)pContent,len,
                                              (unsigned char*)pszUnZippedBuffer, pszNonZippedBufferLen);

		if(nUnZippedLen <= 0)
		{
			status = STATUS_DECOMPRESSION_FAILED;
			DEALLOCBUFFER(pszUnZippedBuffer);
		}
		else
		{
			DEALLOCBUFFER(pContent);
			pContent = pszUnZippedBuffer;
            pContent[nUnZippedLen - 1] = '\0';
		}
	}

	//the deletion of the dom is done in the mcumanager when the get or set event are handled
	CXMLDOMDocument* pDom = new CXMLDOMDocument;

	const char * statusString = NULL;

	if ((status == STATUS_OK) && (pDom->Parse((const char **)&pContent)==SEC_OK))
	{
		CXMLDOMElement * pRoot = pDom->GetRootElement();

		char* pszNodeName = NULL;
		pRoot->get_nodeName(&pszNodeName);

		if( NULL != pszNodeName &&
			(
				0 == strcmp(pszNodeName,"REQUEST_GENERAL") ||
				0 == strcmp(pszNodeName,"REQUEST_CONF_DETAILS") ||
				0 == strcmp(pszNodeName,"REQUEST_PARTY_DETAILS") ||
				0 == strcmp(pszNodeName,"REQUEST_USER_DETAILS")
			)
		)
		{
			CXMLDOMElement* pActionNode     = NULL;
			CXMLDOMElement* pActionTypeNode = NULL;
			pRoot->getChildNodeByName(&pActionNode,"ACTION");

			if(pActionNode)
			{
				pActionNode->firstChildNode(&pActionTypeNode);
			}
			else
			{
				pRoot->firstChildNode(&pActionTypeNode);
				pRoot->nextChildNode(&pActionTypeNode);//action node
			}

			if(pActionTypeNode)
			{
				char *pNodeName=NULL;
				pActionTypeNode->get_nodeName(&pNodeName);

				if(pNodeName)
				{
					HandleRequest(pDom->GetRootElement(),
                                  dualMbx,
                                  nAuthorization,0,
                                  MCMS_INTERNAL_STRING_ENCODE_TYPE);
				}
				else
				{
					status       = STATUS_NODE_MISSING;
					statusString = "(Action element)";
				}
			}
			else
			{
				status       = STATUS_NODE_MISSING;
				statusString = "(Action element)";
			}
		}
		else
		{
			CManagerTask::HandlePostRequest(pParamSeg);
		}
	}
	else
	{
		status       = STATUS_TRANSACTION_IS_NOT_WELL_FORMED;
		statusString = NULL;
	}

	CXMLDOMElement* pXmlResponse = NULL;
	if (status)
	{
		pXmlResponse=::BuildGeneralResponse(status,
											CProcessBase::GetProcess()->GetStatusAsString(status).c_str(),
											statusString,
											0,
											UNKNOWN_REQUEST,
											0,
											0);
	}

	if (pXmlResponse)
	{

//		socketApi.PostConfirm((DWORD)pXmlResponse,Z_NO_COMPRESSION); // ????? socketApi ?????
//		socketApi.DestroyOnlyApi();                                  // ????? socketApi ?????
		//the deletion of the pXmlResponse is done in the SKRX task
	}

	ResetAdditionalParams();

	delete pDom;
	delete [] pContent;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::OnGideonAudioMsgAnystate(CSegment* pParam)
{
	OPCODE	opcode = 0xFFFFFFFF;
	WORD	boardId = 0xFFFF, subBoardId = 0xFFFF, unitId = 0xFFFF;
	DWORD	partyId = 0xFFFFFFFF, confId = 0xFFFFFFFF, connectionId = 0xFFFFFFFF;

	*pParam >> opcode
			>> boardId
			>> subBoardId
			>> unitId;

	*pParam >> confId
			>> partyId
			>> connectionId;

//	DWORD  msgLen = pParam->GetOffsetWrite() - pParam->GetOffsetRead();
	switch( opcode )
	{
		case IVR_PLAY_MESSAGE_REQ:
		case IVR_RECORD_ROLL_CALL_REQ:
		case IVR_STOP_PLAY_MESSAGE_REQ:
		case TB_MSG_OPEN_PORT_REQ:
		case TB_MSG_CLOSE_PORT_REQ:
		case IVR_PLAY_MUSIC_REQ:
		case IVR_STOP_PLAY_MUSIC_REQ:
		case MOVE_RSRC_REQ:
		case BND_CONNECTION_INIT:
		case SEND_H_230:
		case SET_ECS:
		case IVR_STOP_RECORD_ROLL_CALL_REQ:
		{
			CSegment*  pMsgSeg = new CSegment;
			*pMsgSeg << opcode
					<< confId
					<< partyId
					<< connectionId;
			*pMsgSeg << boardId
					<< subBoardId
					<< unitId;

			if (BND_CONNECTION_INIT == opcode )
			{
				DWORD portId;
				*pParam >> portId;
				*pMsgSeg << portId;
				DWORD myDebug;
				*pParam >> myDebug;	// 777
				*pMsgSeg << myDebug;
			}

			DWORD	nDataLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
			BYTE*	pData = new BYTE [nDataLen];
			pParam->Get(pData,nDataLen);
			pMsgSeg->Put(pData,nDataLen);
			PDELETEA(pData);

			m_pEndpointsTaskApi->SendMsg(pMsgSeg,SIM_API_AUDIO_MSG);
			break;
		}

		default:
		{
			DBGPASSERT(opcode+1000);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::OnGideonMRMMsgAnystate(CSegment* pParam)
{
	TRACEINTO << "CEndpointsSimManager::OnGideonMRMMsgAnystate";

	CSegment* pMsg =  new CSegment(*pParam);
	m_pEndpointsTaskApi->SendMsg(pMsg,SIM_API_MRM_MSG);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::OnGideonIsdnMsgAnystate(CSegment* pParam)
{
	CSegment* pMsg =  new CSegment(*pParam);
	m_pEndpointsTaskApi->SendMsg(pMsg,SIM_API_ISDN_MSG);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::OnListenSocketOpenConnection(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CEndpointsSimManager::OnListenSocketOpenConnection - GUI connection opened.");
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::OnListenSocketCloseConnection(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CEndpointsSimManager::OnListenSocketCloseConnection - GUI connection closed.");
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::OnGuiMsgAnycase(CSegment* pParam)
{
	// get Mbx of response socket
	COsQueue  rTxSocketQueue;
	rTxSocketQueue.DeSerialize(*pParam);

	// get parameters
	DWORD  reqTarget = 0;
	*pParam >> reqTarget;

	switch (reqTarget) {
		case GUI_TO_ENDPOINTS:
			SendCommandToEndpoints( rTxSocketQueue,GUI_TO_ENDPOINTS, pParam );
			break;
		case GUI_TO_MANAGER:
			HandleGuiRequest(rTxSocketQueue,pParam);
			break;
		case GUI_TO_SCRIPTS:
			SendCommandToScripts( rTxSocketQueue, GUI_TO_SCRIPTS, pParam );
			break;
		case GUI_TO_GATEKEEPER:
			SendCommandToGKSim( rTxSocketQueue, GUI_TO_GATEKEEPER, pParam );
			break;
		default:
			break;
	}
	return;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::OnCsMsgAnystate(CSegment* pParam)
{
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pParam, CS_API_TYPE);

	if(SIP_CS_PROXY_NOTIFY_REQ != pMplProtocol->getOpcode())
	{
	    POBJDELETE(pMplProtocol);
	    return;
	}

	const DWORD cs_id = pMplProtocol->getCentralSignalingHeaderCsId();
	CCSSimCluster* cluster = m_csClusters.Get(cs_id);
	if(cluster)
	    cluster->GetSipSubscriptions().ProcessCsRequest(pMplProtocol);

	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::OnExtDbKeppAliveTout(CSegment* pParam)
{
	m_extDbSimulator.KeepAliveTimer();
	StartTimer(EPSIM_EXTERNAL_DB_KEEP_ALIVE_TOUT,EPSIM_EXTERNAL_DB_KEEP_ALIVE_TIME);
}


/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::SendCommandToEndpoints(COsQueue& txMbx,const OPCODE code, CSegment *pParam)
{
	CSegment *pMsg = new CSegment( *pParam );

	txMbx.Serialize(*pMsg);

	m_pEndpointsTaskApi->SendMsg( pMsg, code );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::SendCommandToScripts(const COsQueue& txMbx,const OPCODE code, CSegment *pParam)
{
	CSegment *pMsg = new CSegment( *pParam );

	txMbx.Serialize(*pMsg);

	m_pScriptsApi->SendMsg( pMsg, code );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::SendCommandToGKSim(const COsQueue& txMbx,const OPCODE code, CSegment *pParam)
{
    PASSERTMSG_AND_RETURN(1, "Not yet implemented");

//	CSegment *pMsg = new CSegment( *pParam );
//
//	txMbx.Serialize(*pMsg);
//
//	m_pGateKeeperTaskApi->SendMsg( pMsg, code );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsSimManager::HandleGuiRequest(COsQueue& txMbx,CSegment *pParam)
{
	DWORD opcode = 0;

	*pParam >> opcode;

	CSegment* pMsgParam = NULL;
	switch( opcode )
	{
		case GUI_EXTDB_STATUS:
			pMsgParam = new CSegment;
			*pMsgParam	<< m_extDbSimulator.GetConnectionStatus();
			break;
		case GUI_EXTDB_RECORDS_LIST:
			pMsgParam = new CSegment;
			m_extDbSimulator.Serialize(*pMsgParam);
			break;
		default:
			break;
	}

	if( pMsgParam != NULL )
	{
		CSegment* pMsgSeg = new CSegment;

		*pMsgSeg	<< GUI_TO_MANAGER
					<< opcode
					<< (DWORD)STATUS_OK;

		*pMsgSeg	<< *pMsgParam;

		POBJDELETE(pMsgParam);

		CTaskApi api;
		api.CreateOnlyApi(txMbx);
		api.SendMsg(pMsgSeg,SOCKET_WRITE);
		api.DestroyOnlyApi();
	}
}


/////////////////////////////////////////////////////////////////////////////
//    TRANSACTIONS FROM APACHE
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//  Add H323 party
STATUS CEndpointsSimManager::OnTransH323PartyAdd(CRequest *pRequest)
{
	return TransPartyAdd(pRequest, eEndPointH323);
}

/////////////////////////////////////////////////////////////////////////////
//  H323 party error bitrate
STATUS CEndpointsSimManager::OnTransH323PartyErrorBitrate(CRequest *pRequest)
{
	CCommSetBitRateError *pPartySet = (CCommSetBitRateError*)pRequest->GetRequestObject();

	DWORD errorBitrate = pPartySet->GetErrorBitRateVal();
	TRACEINTO 	<< " CEndpointsSimManager::OnTransH323PartyErrorBitrate - Error Bitrate=" << errorBitrate;

	// save in Endpoints-System-Cfg
	::GetEpSystemCfg()->SetErrorBitRate( errorBitrate );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
//  Add SIP party
STATUS CEndpointsSimManager::OnTransSipPartyAdd(CRequest *pRequest)
{
	//return TransPartyAdd(pRequest,eEndPointSip);
	TRACEINTO 	<< " CEndpointsSimManager::OnTransSipPartyAdd" ;
	CCommSetPartyAddSipEP *pPartySet = (CCommSetPartyAddSipEP*)pRequest->GetRequestObject();

		CSegment*  pMsg = new CSegment;
		DWORD dummyValue = 0;
		char  szManufacturer[H243_NAME_LEN];
		char  sourcePartyAlias[H243_NAME_LEN];
		memset(szManufacturer,0,H243_NAME_LEN);
		memset(sourcePartyAlias,0,H243_NAME_LEN);

		*pMsg 	<< (DWORD)BATCH_ADD_PARTY
				<< (DWORD)eEndPointSip
				<< pPartySet->GetPartyName()
				<< pPartySet->GetConfName()
				<< pPartySet->GetCapsetName()
				<< pPartySet->GetIpVersion()
				<< pPartySet->GetCSID()
				<<szManufacturer
				<<sourcePartyAlias
				<< pPartySet->GetPartyUserAgent();
		TRACEINTO 	<< " CEndpointsSimManager::OnTransSipPartyAdd ua "<< pPartySet->GetPartyUserAgent();
	// SAGI and YAIR to verify this - TODO

		m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

		// must be here for infrastructure, else it sends general response.
		pRequest->SetConfirmObject(new CDummyEntry);
		pRequest->SetStatus(STATUS_OK);

		return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Add PSTN party
STATUS CEndpointsSimManager::OnTransPstnPartyAdd(CRequest *pRequest)
{
	CCommSetAddIsdnEndpoint *pPartySet = (CCommSetAddIsdnEndpoint*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;
	DWORD dummyValue = 0;

	*pMsg 	<< (DWORD)BATCH_ADD_PARTY
			<< (DWORD)eEndPointPstn
			<< pPartySet->GetPartyName()
			<< pPartySet->GetPhoneNum()
			<< pPartySet->GetCapsetName()
			<< (DWORD)-1  // non ip party
			<< (DWORD)-1  // non ip version
			<< (DWORD)-1; // no multiple CS version

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Add ISDN party
STATUS CEndpointsSimManager::OnTransIsdnPartyAdd(CRequest *pRequest)
{
	CCommSetAddIsdnEndpoint *pPartySet = (CCommSetAddIsdnEndpoint*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_ADD_PARTY
			<< (DWORD)eEndPointIsdn
			<< pPartySet->GetPartyName()
			<< pPartySet->GetPhoneNum()
			<< pPartySet->GetCapsetName()
			<< pPartySet->GetNumberOfChannels();


	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
//  Add party
STATUS CEndpointsSimManager::OnTransSetPartyCapset(CRequest *pRequest) const
{
	CCommSetPartyAdd *pPartySet = (CCommSetPartyAdd*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_SET_PARTY_CAPSET
			<< pPartySet->GetPartyName()
			<< pPartySet->GetCapsetName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
//  Add party
STATUS CEndpointsSimManager::TransPartyAdd(CRequest *pRequest,const eEndpointsTypes type) const
{
	CCommSetPartyAdd *pPartySet = (CCommSetPartyAdd*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;
	DWORD dummyValue = 0;

	*pMsg 	<< (DWORD)BATCH_ADD_PARTY
			<< (DWORD)type
			<< pPartySet->GetPartyName()
			<< pPartySet->GetConfName()
			<< pPartySet->GetCapsetName()
			<< pPartySet->GetIpVersion()
			<< pPartySet->GetCSID()
			<< pPartySet->GetManufacturer()
			<< pPartySet->GetAliasName();

// SAGI and YAIR to verify this - TODO

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Delete party
STATUS CEndpointsSimManager::OnTransPartyDel(CRequest *pRequest)
{
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_DEL_PARTY
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Connect party
STATUS CEndpointsSimManager::OnTransPartyConnect(CRequest *pRequest)
{
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_CONNECT_PARTY
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Disconnect party
STATUS CEndpointsSimManager::OnTransPartyDisconnect(CRequest *pRequest)
{
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_DISCONNECT_PARTY
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Party sends Dtmf
STATUS CEndpointsSimManager::OnTransPartyDtmf(CRequest *pRequest)
{
	CCommSetPartyDtmf *pPartySet = (CCommSetPartyDtmf*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_DTMF_PARTY
			<< pPartySet->GetPartyName()
			<< pPartySet->GetDtmfString()
			<< pPartySet->GetDtmfSource();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Remote change it mode sends Dtmf
STATUS CEndpointsSimManager::OnTransPartyChangeMode(CRequest *pRequest)
{
	CCommSetPartyDtmf *pPartySet = (CCommSetPartyDtmf*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_CHANGEMODE_PARTY
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Common - for all endpoints types

/////////////////////////////////////////////////////////////////////////////
//  Add new capability set
STATUS CEndpointsSimManager::OnTransDelCapset(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal,"CEndpointsSimManager::OnTransDelCapset");
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_DEL_CAPSET
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Add new capability set
STATUS CEndpointsSimManager::OnTransAddCapset(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal,"CEndpointsSimManager::OnTransAddCapset");
	CCommSetAddCapset *pPartySet = (CCommSetAddCapset*)pRequest->GetRequestObject();

	CCapSet  rCapset;
	rCapset.Empty();
	rCapset.SetName(pPartySet->GetName());
	rCapset.SetRate(pPartySet->GetRate());
	rCapset.SetFecc(pPartySet->GetFecc());
	rCapset.SetEncryption(pPartySet->GetEncrypted());
	rCapset.SetH239Rate((TRUE == pPartySet->GetH239())? 1920:0);

		// audio
	if( TRUE == pPartySet->GetAudioSiren7() )
	{
		rCapset.AddAudioAlg(eSiren7_16kCapCode);
	}
	if( TRUE == pPartySet->GetAudioSiren14() )
	{
		rCapset.AddAudioAlg(eSiren14_24kCapCode);
		rCapset.AddAudioAlg(eSiren14_32kCapCode);
		rCapset.AddAudioAlg(eSiren14_48kCapCode);
	}
	if( TRUE == pPartySet->GetAudioOpus() )
	{
		rCapset.AddAudioAlg(eOpus_CapCode);
	}
	if( TRUE == pPartySet->GetAudioG7221() )
	{
		rCapset.AddAudioAlg(eG7221_32kCapCode);
		rCapset.AddAudioAlg(eG7221_24kCapCode);
		rCapset.AddAudioAlg(eG7221_16kCapCode);
	}
	if( TRUE == pPartySet->GetAudioG7221AnnexC() )
	{
		rCapset.AddAudioAlg(eG7221C_48kCapCode);
		rCapset.AddAudioAlg(eG7221C_32kCapCode);
		rCapset.AddAudioAlg(eG7221C_24kCapCode);
	}
	if( TRUE == pPartySet->GetAudioG7231() )
		rCapset.AddAudioAlg(eG7231CapCode);
	if( TRUE == pPartySet->GetAudioG729() )
		rCapset.AddAudioAlg(eG729AnnexACapCode);
	if( TRUE == pPartySet->GetAudioG728() )
		rCapset.AddAudioAlg(eG728CapCode);
	if( TRUE == pPartySet->GetAudioG722() )
	{
		rCapset.AddAudioAlg(eG722_48kCapCode);
		rCapset.AddAudioAlg(eG722_56kCapCode);
		rCapset.AddAudioAlg(eG722_64kCapCode);
	}
	if( TRUE == pPartySet->GetAudioG711() )
	{
		rCapset.AddAudioAlg(eG711Ulaw64kCapCode);
		rCapset.AddAudioAlg(eG711Alaw64kCapCode);
		rCapset.AddAudioAlg(eG711Ulaw56kCapCode);
		rCapset.AddAudioAlg(eG711Alaw56kCapCode);
	}

	//TIP support for testing embedded MLA feature
	if (CCapSetsList::m_isTIP == TRUE)
	{
		if (TRUE == pPartySet->GetAudioAACLD())// TIP
		{
			rCapset.AddAudioAlg(eAAC_LDCapCode);
		}
	}

		// video
	if( TRUE == pPartySet->GetVideoH264() )
	{
		CVideoCapH264	rH264cap((enVideoModeH264)pPartySet->GetVideoModeH264(),pPartySet->GetH264AspectRatio(),pPartySet->GetH264StaticMB()); // default H.264 cap
//		rH264cap.SetAspectRatio(pPartySet->GetH264AspectRatio());
		rCapset.AddVideoProtocol(rH264cap);
	}

	CVideoCapH263  rH263cap; // default H.263 cap
	if( TRUE == pPartySet->GetVideoH263() )
		rCapset.AddVideoProtocol(rH263cap);

	CVideoCapVP8  rVP8cap; // new VP8 Cap //N.A. DEBUG VP8
		if( TRUE == pPartySet->GetVideoVP8() )
			rCapset.AddVideoProtocol(rVP8cap);


		// sdes - add sdes automatic in func SetEncryption() if encrypted
//	if (pPartySet->GetEncrypted())
//		rCapset.AddSdesCaps();


	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_ADD_CAPSET;
	rCapset.Serialize(*pMsg);

//	txMbx.Serialize(*pMsg);

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Send active speaker from endpoint
STATUS CEndpointsSimManager::OnTransActiveSpeaker(CRequest *pRequest)
{
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_ACTIVE_SPEAKER
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Send audio speaker from endpoint
STATUS CEndpointsSimManager::OnTransAudioSpeaker(CRequest *pRequest)
{
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_AUDIO_SPEAKER
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Sip Mute endpoint
STATUS CEndpointsSimManager::OnTransSipMute(CRequest *pRequest)
{
	CCommSetPartyMute *pPartySet = (CCommSetPartyMute*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_MUTE
			<< pPartySet->GetPartyName()
			<< pPartySet->GetMuteAudioByPort()
			<< pPartySet->GetMuteVideoByPort()
			<< pPartySet->GetMuteAudioByDirection()
			<< pPartySet->GetMuteVideoByDirection()
			<< pPartySet->GetMuteAudioByInactive()
			<< pPartySet->GetMuteVideoByInactive();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Mute endpoint
STATUS CEndpointsSimManager::OnTransMute(CRequest *pRequest)
{
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_MUTE
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Unmute endpoint
STATUS CEndpointsSimManager::OnTransUnmute(CRequest *pRequest)
{
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_UNMUTE
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Send FECC token request from endpoint
STATUS CEndpointsSimManager::OnTransFeccTokenRequest(CRequest *pRequest)
{
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_FECC_TOKEN_REQUEST
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Send FECC token release from endpoint
STATUS CEndpointsSimManager::OnTransFeccTokenRelease(CRequest *pRequest)
{
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_FECC_TOKEN_RELEASE
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Send H239 token request from endpoint
STATUS CEndpointsSimManager::OnTransH239TokenRequest(CRequest *pRequest)
{
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_H239_TOKEN_REQUEST
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Send H239 token release from endpoint
STATUS CEndpointsSimManager::OnTransH239TokenRelease(CRequest *pRequest)
{
	CCommSetPartyCommon *pPartySet = (CCommSetPartyCommon*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_H239_TOKEN_RELEASE
			<< pPartySet->GetPartyName();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Send FECC key request from endpoint
STATUS CEndpointsSimManager::OnTransFeccKeyRequest(CRequest *pRequest)
{
	CCommSetPartyFecc *pPartySet = (CCommSetPartyFecc*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_FECC_KEY_REQUEST
			<< pPartySet->GetPartyName()
			<< pPartySet->GetFeccString();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
//  Update channels
STATUS CEndpointsSimManager::OnTransEndpointUpdateChannels(CRequest *pRequest)
{
	CCommSetPartyUpdateChannels *pPartySet = (CCommSetPartyUpdateChannels*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	*pMsg 	<< (DWORD)BATCH_CHANNELS_UPDATE
			<< pPartySet->GetPartyName()
			<< pPartySet->GetRecapMode()
			<< (BYTE)pPartySet->GetAudioChannelOpen()
			<< (BYTE)pPartySet->GetVideoChannelOpen()
			<< (BYTE)pPartySet->GetFeccChannelOpen()
			<< (BYTE)pPartySet->GetH239ChannelOpen()
			<< (DWORD)pPartySet->GetCapSetID()
			<< pPartySet->GetManufacturer();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  External DB request - Keep Alive
STATUS CEndpointsSimManager::OnTransExtermalDbKeepAlive(CRequest *pRequest)
{
	m_extDbSimulator.KeepAlive();
	DeleteTimer(EPSIM_EXTERNAL_DB_KEEP_ALIVE_TOUT);
	StartTimer(EPSIM_EXTERNAL_DB_KEEP_ALIVE_TOUT,EPSIM_EXTERNAL_DB_KEEP_ALIVE_TIME);

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  External DB request - Create
STATUS CEndpointsSimManager::OnTransExtermalDbCreate(CRequest *pRequest)
{
    PTRACE(eLevelInfoNormal, "CEndpointsSimManager::OnTransExtermalDbCreate");

	CCommSetExternalDbCreate* pCommSet = new CCommSetExternalDbCreate();
	*pCommSet = *((CCommSetExternalDbCreate*)pRequest->GetRequestObject());

	STATUS stat = m_extDbSimulator.ProcessRequest(pCommSet);

	pRequest->SetConfirmObject(pCommSet);
	pRequest->SetStatus(stat);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  External DB request - Add
STATUS CEndpointsSimManager::OnTransExtermalDbAdd(CRequest *pRequest)
{
    PTRACE(eLevelInfoNormal, "CEndpointsSimManager::OnTransExtermalDbAdd");

	CCommSetExternalDbAdd* pCommSet = new CCommSetExternalDbAdd();
	*pCommSet = *((CCommSetExternalDbAdd*)pRequest->GetRequestObject());

	STATUS stat = m_extDbSimulator.ProcessRequest(pCommSet);

	pRequest->SetConfirmObject(pCommSet);
	pRequest->SetStatus(stat);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  External DB request - Authenticate user
STATUS CEndpointsSimManager::OnTransExtermalDbUser(CRequest *pRequest)
{
	CCommSetExternalDbUser* pCommSet = new CCommSetExternalDbUser();
	*pCommSet = *((CCommSetExternalDbUser*)pRequest->GetRequestObject());

	STATUS stat = m_extDbSimulator.ProcessRequest(pCommSet);

	pRequest->SetConfirmObject(pCommSet);
	pRequest->SetStatus(stat);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  SIP subscriptions: add subscriptor
STATUS CEndpointsSimManager::OnTransSipAddSubscription(CRequest *pRequest)
{
	CCommSetAddSubscription* pSetAddSubscr = new CCommSetAddSubscription;
	*pSetAddSubscr = *((CCommSetAddSubscription*)pRequest->GetRequestObject());

    // TOREMOVE
    const DWORD cs_id = 1;
    //kobig : for now we will loose this ASSERt - till we support multiple cs
    //PASSERTMSG(1, "Multiple CS is not supported: apply to CS[1]");

	STATUS stat;

    CCSSimCluster* cluster = m_csClusters.Get(cs_id);
    if(cluster)
        stat = cluster->GetSipSubscriptions().ProcessBatchRequest(pSetAddSubscr);
    else
        stat = STATUS_FAIL;

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(pSetAddSubscr);
	pRequest->SetStatus(stat);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  SIP subscriptions: get last notification of subscriber
STATUS CEndpointsSimManager::OnTransSipGetNotification(CRequest *pRequest)
{
	CCommSetGetNotification*  pGetNotif = new CCommSetGetNotification();
	*pGetNotif = *((CCommSetGetNotification*)pRequest->GetRequestObject());

	// TOREMOVE
    const DWORD cs_id = 1;
    //kobig : for now we will loose this ASSERt - till we support multiple cs
    //PASSERTMSG(1, "Multiple CS is not supported: apply to CS[1]");

    STATUS stat;

    CCSSimCluster* cluster = m_csClusters.Get(cs_id);
    if(cluster)
        stat = cluster->GetSipSubscriptions().ProcessBatchRequest(pGetNotif);
    else
        stat = STATUS_FAIL;

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(pGetNotif);
	pRequest->SetStatus(stat);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Scp streams request
STATUS CEndpointsSimManager::OnTransScpStreamsRequest(CRequest *pRequest)
{
	CCommSetScpStreamsRequest*  pScpStreamsRequest = new CCommSetScpStreamsRequest();
	*pScpStreamsRequest = *((CCommSetScpStreamsRequest*)pRequest->GetRequestObject());

	CSegment*  pMsg = new CSegment;

	TRACEINTO << "CEndpointsSimManager::OnTransScpStreamsRequest - Conf Name = "
			<< pScpStreamsRequest->GetConfName()
			<< " Party Name = "
			<< pScpStreamsRequest->GetPartyName();

	*pMsg 	<< (DWORD)BATCH_SCP_STREAMS_REQUEST
	    << pScpStreamsRequest->GetConfName()
	    << pScpStreamsRequest->GetPartyName()
	    << pScpStreamsRequest->GetNumberOfStreams();
	pMsg->Put((BYTE*)(pScpStreamsRequest->GetMrmpStreamDescArray()),sizeof(MrmpStreamDesc)*pScpStreamsRequest->GetNumberOfStreams());

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Send H239 token request from endpoint
STATUS CEndpointsSimManager::OnTransLprModeChangeRequest(CRequest *pRequest)
{
	CCommSetLprModeChangeReq *pPartySet = (CCommSetLprModeChangeReq*)pRequest->GetRequestObject();

	CSegment*  pMsg = new CSegment;

	TRACEINTO << "CEndpointsSimManager::OnTransLprModeChangeRequest - Party Name = " << pPartySet->GetPartyName()
	 << " LossProtection = " << pPartySet->GetLossProtection() << " Mtbf = " << pPartySet->GetMtbf()
	 << " Congestion = " << pPartySet->GetCongestionCeiling() << " Fill = " << pPartySet->GetFill()
	 << " Mode timeout = " << pPartySet->GetModeTimeout();

	*pMsg 	<< (DWORD)BATCH_LPR_MODE_CHANGE_REQUEST
			<< pPartySet->GetPartyName()
			<< pPartySet->GetLossProtection()
			<< pPartySet->GetMtbf()
			<< pPartySet->GetCongestionCeiling()
			<< pPartySet->GetFill()
			<< pPartySet->GetModeTimeout();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

	// must be here for infrastructure, else it sends general response.
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//  Connect / Disconnect Socket from Card
STATUS CEndpointsSimManager::HandleSocketDisconnectAction(CRequest *pRequest)
{
    PASSERTMSG(true, "Check the program flow");
    return STATUS_FAIL;

   // CCommSetDisconnectOrConnectSockets *pPartySet = (CCommSetDisconnectOrConnectSockets*)pRequest->GetRequestObject();
//    PTRACE(eLevelInfoNormal,">>>>><<<<<CEndpointsSimManager::HandleSocketDisconnectAction");
//
//	CSegment*  pMsg = new CSegment;
//	*pMsg 	<< "1";
//	int boardId=1;
//
//		if (m_pCSApi)
//		{
//
//			m_pCSApi->SendMsg(pMsg,PAUSE_SOCKET);
//
//		}
//
//	//POBJDELETE( pMsg );
//
//	// must be here for infrastructure, else it sends general response.
//	pRequest->SetConfirmObject(new CDummyEntry);
//
//	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CEndpointsSimManager::HandleTerminalEncryptionFlag(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: number of parameters\n";
		answer << "usage: Bin/McuCmd set_encryption_flag EndpointsSim [ON | OFF]\n";
		return STATUS_FAIL;
	}

	const string& encryption = command.GetToken(eCmdParam1);

	BOOL bOnOff = FALSE;

	if (encryption == "ON")
	{
		bOnOff = TRUE;
	}
	else if (encryption == "OFF")
	{
		bOnOff = FALSE;
	}
	else
	{
		answer << "error: encryption value not supported!";
		return STATUS_FAIL;
	}

	::GetEpSystemCfg()->SetEncryptionDialOut(bOnOff);
	answer << "CEndpointsSimManager::HandleTerminalEncryptionFlag, encryption: " << encryption << "\n";

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CEndpointsSimManager::HandleTerminalSendLpr(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CEndpointsSimManager::HandleTerminalSendLpr - send LPR from first simulation party");
	DWORD numOfParams = command.GetNumOfParams();
	if(0 == numOfParams)
	{
		answer << "error: number of parameters\n";
		answer << "usage: Bin/McuCmd Send_LPR_Ind EndpointsSim [ON | OFF]\n";
		return STATUS_FAIL;
	}

	string simPartyName;
	const string &strLprCommand = command.GetToken(eCmdParam1);
	answer <<  "Send_LPR_Ind " << strLprCommand;

	APIU32 lossProtection = 0;
	APIU32 Mtbf = 0;
	APIU32 congestionCeiling = 9600;

	if(strLprCommand == "help")
	{
		answer << "LPR Help request\n"
				<< "Type 1 9600 to use 9600 DBA command\n"
				<< "Type 2 7680 to use 7680 LPR command\n"
				<< "Type 3 9600 to use 9600 LPR command\n"
				<< "Type help for the LPR commands list\n"
				<< "Command Syntax:ca Send_LPR_Ind EndpointsSim [command option 1/2/3] [congestionCeiling rate 960/768/other] [Sim Party Name]\n"
				<< "Use [ca Show_All_Parties EndpointsSim] to see parties list\n";
		return STATUS_OK;
	}
	else
	{
		const string &cmdCongestionCeiling 	= command.GetToken(eCmdParam2);
		simPartyName = command.GetToken(eCmdParam3);
		DWORD lprCommand = atoi(strLprCommand.c_str());
		if(lprCommand == 1)
		{
			// same default setting
		}
		else if (lprCommand == 2)
		{
			// LPR reducing to 7680
			lossProtection 		= 3;
			Mtbf 				= 6000;
		}
		else if (lprCommand == 3)
		{
			// LPR reducing to of 960
			lossProtection 		= 3;
			Mtbf 				= 6000;
		}
		else
		{
			answer << "error: unknown command\n";
			return STATUS_FAIL;
		}
		congestionCeiling = atoi(cmdCongestionCeiling.c_str());
	}

	CCommSetLprModeChangeReq *pPartySet =  new CCommSetLprModeChangeReq;
	CSegment*  pMsg = new CSegment;

	pPartySet->SetPartyName(const_cast<char *>(simPartyName.c_str()));
	pPartySet->SetLossProtection(lossProtection);
	pPartySet->SetMtbf(Mtbf);
	pPartySet->SetCongestionCeiling(congestionCeiling);
//	pPartySet->SetFill(0);
//	pPartySet->SetModeTimeout(0);

	TRACEINTO << "CEndpointsSimManager::OnTransLprModeChangeRequest - Party Name = " << pPartySet->GetPartyName()
	 << " LossProtection = " << pPartySet->GetLossProtection() << " Mtbf = " << pPartySet->GetMtbf()
	 << " Congestion = " << pPartySet->GetCongestionCeiling() << " Fill = " << pPartySet->GetFill()
	 << " Mode timeout = " << pPartySet->GetModeTimeout();

	*pMsg 	<< (DWORD)BATCH_LPR_MODE_CHANGE_REQUEST
			<< pPartySet->GetPartyName()
			<< pPartySet->GetLossProtection()
			<< pPartySet->GetMtbf()
			<< pPartySet->GetCongestionCeiling()
			<< pPartySet->GetFill()
			<< pPartySet->GetModeTimeout();

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );
	POBJDELETE(pPartySet);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//    GLOBAL FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CEndpointsSimSystemCfg* GetEpSystemCfg()
{
	return g_pEpSystemCfg;
}

/////////////////////////////////////////////////////////////////////////////
void SetEpSystemCfg(CEndpointsSimSystemCfg* p)
{
	g_pEpSystemCfg = p;
}

/////////////////////////////////////////////////////////////////////////////
void SendIsdnMessageToGideonSimApp(CSegment& rParam)
{
	CSegment*  pMsg = new CSegment(rParam);

	const COsQueue* pGideonMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessGideonSim, eManager);

	CTaskApi api;
	api.CreateOnlyApi(*pGideonMbx);
	api.SendMsg(pMsg, SIM_API_ISDN_MSG);
	api.DestroyOnlyApi();
}

/////////////////////////////////////////////////////////////////////////////
void SendAudioMessageToGideonSimApp(CSegment& rParam)
{
	CSegment*  pMsg = new CSegment(rParam);

	const COsQueue* pGideonMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessGideonSim, eManager);

	CTaskApi api;
	api.CreateOnlyApi(*pGideonMbx);
	api.SendMsg(pMsg, SIM_API_AUDIO_MSG);
	api.DestroyOnlyApi();
}

/////////////////////////////////////////////////////////////////////////////
void SendMuxMessageToGideonSimApp(CSegment& rParam)
{
	CSegment* pMsg = new CSegment(rParam);

	const COsQueue* pGideonMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessGideonSim, eManager);

	CTaskApi api;
	api.CreateOnlyApi(*pGideonMbx);
	api.SendMsg(pMsg, SIM_API_MUX_MSG);
	api.DestroyOnlyApi();
}

/////////////////////////////////////////////////////////////////////////////
void SendScpMessageToGideonSimApp(CSegment& rParam)
{
	CSegment*  pMsg = new CSegment(rParam);

	const COsQueue* pGideonMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessGideonSim, eManager);

	CTaskApi api;
	api.CreateOnlyApi(*pGideonMbx);
	api.SendMsg(pMsg, SIM_API_MRM_MSG);
	api.DestroyOnlyApi();
}

/*
 * Global function, defined at EndpointsSim.h
 */
void FillCsProtocol(CMplMcmsProtocol* pMplProt,
                    DWORD csID,
                    DWORD opcode,
                    const BYTE* pData,
                    DWORD nDataLen,
                    DWORD nCsCallIndex,
                    DWORD channelIndexParam,
                    DWORD channelMcIndexParam)
{
    DWORD payloadLen = sizeof(COMMON_HEADER_S)
                     + sizeof(MESSAGE_DESCRIPTION_HEADER_S)
                     + sizeof(CENTRAL_SIGNALING_HEADER_S)
                     + sizeof(PORT_DESCRIPTION_HEADER_S)*pMplProt->getPortDescriptionHeaderCounter()
                     + nDataLen;

    DWORD payloadOffset = sizeof(COMMON_HEADER_S);

    pMplProt->AddCommonHeader(opcode, MPL_PROTOCOL_VERSION_NUM, 0,
                              (BYTE)eCentral_signaling, (BYTE)eMcms,
                              ::SystemGetTickCount().GetIntegerPartForTrace(),
                              payloadLen, payloadOffset, (DWORD)eHeaderMsgDesc,
                              sizeof(MESSAGE_DESCRIPTION_HEADER_S));

    pMplProt->AddMessageDescriptionHeader(123,
                                          (DWORD)eCentral_signaling,
                                          ::SystemGetTickCount().GetIntegerPartForTrace(),
                                          eHeaderCs,
                                          sizeof(CENTRAL_SIGNALING_HEADER_S));

    DWORD channel_index = (channelIndexParam == 0xFFFFFFFF) ?
        pMplProt->getCentralSignalingHeaderChannelIndex() : channelIndexParam;

    DWORD channel_mc_index = (channelMcIndexParam == 0xFFFFFFFF) ?
        pMplProt->getCentralSignalingHeaderMcChannelIndex() : channelMcIndexParam;

    pMplProt->AddCSHeader(csID,
                          pMplProt->getCentralSignalingHeaderDestUnitId(),
                          pMplProt->getCentralSignalingHeaderSrcUnitId(),
                          (0 == nCsCallIndex) ? pMplProt->getCentralSignalingHeaderCallIndex() : nCsCallIndex, // call_index
                          csID,             // service_id
                          channel_index,    // channel_index
                          channel_mc_index, // mc_channel_index
                          (APIS32)STATUS_OK // status
                          );

    pMplProt->AddData(nDataLen, (const char*)pData);
}

//////////////////////////////////////////////////////////////////////////////
STATUS CEndpointsSimManager::HandleTerminalStartCS(CTerminalCommand& command,
                                                   std::ostream& answer)
{
    if (2 != command.GetNumOfParams())
    {
        answer << "usage: start_cs [CS_ID] [CS_NUM_OF_PORTS]";
        return STATUS_OK;
    }

    DWORD cs_id;
    std::istringstream buf1(command.GetToken(eCmdParam1));
    if (!(buf1 >> cs_id))
    {
        answer << "error: illegal CS ID";
        return STATUS_OK;
    }

    DWORD cs_num_ports;
    std::istringstream buf2(command.GetToken(eCmdParam2));
    if (!(buf2 >> cs_num_ports))
    {
        answer << "error: illegal number of ports";
        return STATUS_OK;
    }

    PASSERT_AND_RETURN_VALUE(!m_pEndpointsTaskApi, STATUS_FAIL);

    const COsQueue& eps = m_pEndpointsTaskApi->GetRcvMbx();
    if (m_csClusters.Add(cs_id, cs_num_ports, GetRcvMbx(), eps) < 0)
    {
        answer << "error: see assertion for more information";
        return STATUS_OK;
    }

    answer << "CS started, CS ID: " << cs_id
           << ", Number of Ports: " << cs_num_ports;

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////
STATUS CEndpointsSimManager::HandleTerminalStopCS(CTerminalCommand& command,
                                                  std::ostream& answer)
{
    if (1 != command.GetNumOfParams())
    {
        answer << "usage: stop_cs [CS_ID]";
        return STATUS_OK;
    }

    DWORD cs_id;
    std::istringstream buf1(command.GetToken(eCmdParam1));
    if (!(buf1 >> cs_id))
    {
        answer << "error: illegal CS ID";
        return STATUS_OK;
    }

    if (m_csClusters.Remove(cs_id) < 0)
    {
        answer << "error: see assertion for more information";
        return STATUS_OK;
    }

    answer << "CS stopped, CS ID: " << cs_id;

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////
STATUS CEndpointsSimManager::HandleTerminalListCS(CTerminalCommand& command,
                                                  std::ostream& answer)
{
    if (command.GetNumOfParams() > 0)
    {
        answer << "error: illegal number of parameters";
        return STATUS_OK;
    }

    m_csClusters.PrintOut(answer);

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////
STATUS CEndpointsSimManager::HandleTerminalShowAllParties(CTerminalCommand& command,
                                                  std::ostream& answer)
{
    if (command.GetNumOfParams() > 0)
    {
        answer << "error: illegal number of parameters";
        return STATUS_OK;
    }

    CSegment*  pMsg = new CSegment;
	*pMsg 	<< (DWORD)BATCH_SHOW_ALL_PARTIES_REQ;

	m_pEndpointsTaskApi->SendMsg( pMsg, (DWORD)BATCH_COMMAND );

    return STATUS_OK;
}


