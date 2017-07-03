// IceServiceManager.cpp: implementation of the CIceServiceManager class.
//
//////////////////////////////////////////////////////////////////////


#include "IceServiceManager.h"
#include "IpCsOpcodes.h"
#include "MplMcmsProtocol.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Trace.h"
#include "SIPProxyStructs.h"
#include  "StatusesGeneral.h"
#include  "DataTypes.h"
#include  "CsCommonStructs.h"
#include  "SystemFunctions.h"
#include "FaultsDefines.h"
#include "ConfPartyManagerApi.h"
#include "ApiStatuses.h"
#include "InternalProcessStatuses.h"
#include "SysConfig.h"
#include "ICEApiDefinitions.h"
#include "IceCmReq.h"
#include "TraceStream.h"
#include "ServiceConfigList.h"
#include "DefinesIpService.h"//carmit45
#include "SysConfigKeys.h"


// Ice service Manager states
const WORD	 SET_UP		= 1;
const WORD   CONNECT    = 2;

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CIceServiceManager)

	ONEVENT(CS_PROXY_DELETE_IP_SERVICE_IND				,CONNECT	,CIceServiceManager::OnIpServiceParamDel)
	ONEVENT(SIPPROXY_MCUMNGR_CONFIGURATION_IND			,SET_UP		,CIceServiceManager::OnMcuMngrConfigSetup)
	ONEVENT(SIPPROXY_MCUMNGR_CONFIGURATION_IND			,CONNECT	,CIceServiceManager::OnMcuMngrConfigCONNECT)
	ONEVENT(SIPPROXY_CONFIG_CS_IND						,SET_UP		,CIceServiceManager::OnMediaCardSetupEnd)
	ONEVENT(SIPPROXY_CONFIG_CS_IND						,CONNECT	,CIceServiceManager::NullActionFunction)
	ONEVENT(MCUMNGR_TO_SIPPROXY_MULTIPLE_SERVICES_IND	,ANYCASE	,CIceServiceManager::OnMultipleServicesInd)


PEND_MESSAGE_MAP(CIceServiceManager,CTaskApp);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////


extern void IceMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void IceServiceManagerEntryPoint(void* appParam)
{
	CIceServiceManager * pIceServiceManager = new CIceServiceManager;
	pIceServiceManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CIceServiceManager::GetMonitorEntryPoint()
{
	return IceMonitorEntryPoint;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CIceServiceManager::CIceServiceManager() :   m_SCisUp(FALSE), m_serviceEndWasReceived(FALSE)
                            , m_mediaCardSetupEndWasReceived(FALSE), m_bSystemMultipleServices(NO)
{
	int i = 0;
	m_state = SET_UP;

	m_ServiceIceType = eIceEnvironment_None;
	m_ServiceId = -1;
	m_pProxyService = NULL;
	m_pICEserviceId = 0;
	isActive=FALSE;

}


/////////////////////////////////////////////////////////////////////////////
CIceServiceManager::~CIceServiceManager()
{
	POBJDELETE(m_pProxyService);
}


/////////////////////////////////////////////////////////////////////
void CIceServiceManager::ManagerStartupActionsPoint()
{
	McuMngrConfigSetup();
}

/////////////////////////////////////////////////////////////////////////////
void*  CIceServiceManager::GetMessageMap()
{
  return (void*)m_msgEntries;
}


/////////////////////////////////////////////////////////////////////////////
void  CIceServiceManager::OnIpServiceParamDel(CSegment* pSeg)
{
	Del_Ip_Service_S *param = (Del_Ip_Service_S*)pSeg->GetPtr();
	TRACEINTO << "serviceId:" << param->service_id;

	POBJDELETE(m_pProxyService);
}


/////////////////////////////////////////////////////////////////////////
void 	CIceServiceManager::McuMngrConfigSetup()
{
	BYTE bIsMultipleServices =  GetMultipleServices();
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if((YES == bIsMultipleServices && m_mediaCardSetupEndWasReceived) || (NO == bIsMultipleServices)
			|| eProductTypeGesher==curProductType || eProductTypeNinja == curProductType)
		{
			if(m_SCisUp && m_serviceEndWasReceived)
			{
				m_state = CONNECT;

			}
		}
}
/////////////////////////////////////////////////////////////////////////////
void  CIceServiceManager::OnMcuMngrConfigSetup(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CIceServiceManager::OnMcuMngrConfigSetup");

	WORD flag = (WORD)eDnsConfigurationFailure;
	*pSeg >> flag;

	McuMngrConfigSetup();
}

/////////////////////////////////////////////////////////////////////////////
void  CIceServiceManager::OnMcuMngrConfigCONNECT(CSegment* pSeg)
{
	WORD flag = (WORD)eDnsConfigurationFailure;
	*pSeg >> flag;
}

/////////////////////////////////////////////////////////////////////////////
void  CIceServiceManager::OnMediaCardSetupEnd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CIceServiceManager::OnMediaCardSetupEnd");
	m_mediaCardSetupEndWasReceived = TRUE;
}


/////////////////////////////////////////////////////////////////////////////
void  CIceServiceManager::Dump(COstrStream& msg) const
{

}


/////////////////////////////////////////////////////////////////////////////
// Checks whether the received service changes an existing service params and handle the changes if needed.
void  CIceServiceManager::HandleChangesInServiceIfNeeded(CSipProxyIpParams* pReceivedService)
{
	BYTE bUnregisterAll = FALSE;

	DWORD serviceId = 0;

	if(pReceivedService)
		serviceId = pReceivedService->GetServiceId();

	// check if it is new server - only add the new server.
	if(NULL == m_pProxyService)
	{
		PTRACE(eLevelInfoNormal, "CIceServiceManager::HandleChangesInServiceIfNeeded : New service. Not need to handle changes in an existing service");
		return;
	}

	COstrStream msg;

	msg << "\nExisiting service:\n";
	m_pProxyService->Dump(msg);
	msg << "New service:\n";
	if(pReceivedService)
		pReceivedService->Dump(msg);
	PTRACE2(eLevelInfoNormal, "CIceServiceManager::HandleChangesInServiceIfNeeded:\n", msg.str().c_str());

	// sip server is set to off - unregister all. The service will be removed and we won't request conf db from ConfParty.
	if (pReceivedService && pReceivedService->GetServersConfig() == eConfSipServerAuto)
	{
		PTRACE(eLevelInfoNormal, "CIceServiceManager::HandleChangesInServiceIfNeeded - Sip server is set to off");
		bUnregisterAll = TRUE;
	}

	PTRACE(eLevelInfoNormal, "CIceServiceManager::HandleChangesInServiceIfNeeded - Temporary: unregister all always!");
	bUnregisterAll = FALSE;


}

void CIceServiceManager::SetEndService()
{
	PTRACE(eLevelInfoNormal, "CIceServiceManager::SetEndService -bad situation");
}

void CIceServiceManager::CreateTaskName()
{
	char buff[256];
	snprintf(buff, sizeof(buff), "CIceServiceManager (ServiceId %d)", m_ServiceId);
	m_TaskName = buff;
}

void CIceServiceManager::SetServiceId(DWORD id)
{
	m_ServiceId = id;
	CreateTaskName();
}

//////////////////////////////////////////////////////////////////////
DWORD CIceServiceManager::GetServiceId()
{
	return m_ServiceId;
}


////////////////////////////////////////////////////////////////////////////
void CIceServiceManager::NullActionFunction(CSegment* pParam)
{
	CSmallString str;
	str << "Opcode " << m_log[m_current_log_entry].eventStr << " arrived at state " << m_state << "\n";
	PTRACE2(eLevelInfoNormal, "CIceServiceManager::NullActionFunction: ---- ! ---- ",str.GetString());
}

/////////////////////////////////////////////////////////////////////////////
BYTE CIceServiceManager::GetMultipleServices()
{
    BOOL isMultipleServicesCfgFlag = NO;

    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_MULTIPLE_SERVICES, isMultipleServicesCfgFlag);
	if (YES == m_bSystemMultipleServices  && isMultipleServicesCfgFlag)
	{
		return YES;
	}

	return NO;
}

/////////////////////////////////////////////////////////////////////////////
void CIceServiceManager::OnMultipleServicesInd(CSegment* pParam)
{
	*pParam >> m_bSystemMultipleServices;
	TRACESTR(eLevelInfoNormal) << "\nCIceServiceManager::OnMultipleServicesInd"
						   << "\nMultiple services: : " << (YES ==  m_bSystemMultipleServices? "YES" : "NO");
}


//CWebRtcServiceManager
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CWebRtcServiceManager)

	ONEVENT(UPDATE_ICE_END      ,ANYCASE	,CWebRtcServiceManager::OnCSEndIceInd)
	ONEVENT(UPDATE_STATUS_END   ,ANYCASE	,CWebRtcServiceManager::OnCSEndIceStatInd)

PEND_MESSAGE_MAP(CWebRtcServiceManager,CIceServiceManager);

void IceWebRtcServiceManagerEntryPoint(void* appParam)
{
	CWebRtcServiceManager * pIceServiceManager = new CWebRtcServiceManager;
	pIceServiceManager->Create(*(CSegment*)appParam);
}

static void getStatusErrorText(iceServersStatus status, char * errorText)
{
	if(errorText == NULL)
		return;

	switch (status)
	{
	case eIceInitOk:
		strncpy(errorText, "ICE Init OK", strlen("ICE Init OK"));
		break;
	case eIceInitServerFail:
		strncpy(errorText, "ICE Init Server Fail", strlen("ICE Init Server Fail"));
		break;
	case eIceStunPassServerAuthenticationFailure:
		strncpy(errorText, "ICE Server Authentication Failure", strlen("ICE Server Authentication Failure"));
		break;
	case eIceStunPassServerConnectionFailure:
		strncpy(errorText, "ICE Stun Pass Server Connection Failure", strlen("ICE Stun Pass Server Connection Failure"));
		break;
	case eIceTurnServerDnsResolveFailure:
		strncpy(errorText, "ICE Turn Server DNS Resolve Failure", strlen("ICE Turn Server DNS Resolve Failure"));
		break;
	case eIceTurnServerUnreachable:
		strncpy(errorText, "ICE Turn Server Unreachable", strlen("ICE Turn Server Unreachable"));
		break;
	case eIceTurnServerAuthorizationFailure:
		strncpy(errorText, "ICE Turn Server Authorization Failure", strlen("ICE Turn Server Authorization Failure"));
		break;
	case eIceServerUnavailble:
		strncpy(errorText, "Ice Server Unavailable", strlen("Ice Server Unavailable"));
		break;
	case eIceUnknownProblem:
	default:
		strncpy(errorText, "Ice Unknown Problem", strlen("Ice Unknown Problem"));
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////
CWebRtcServiceManager::CWebRtcServiceManager()
{
	PTRACE(eLevelInfoNormal, "CWebRtcServiceManager::CWebRtcServiceManager");

	memset(m_stunPassServerHostName, 0, sizeof(m_stunPassServerHostName));
	memset(m_stunPassServerUserName, 0, sizeof(m_stunPassServerUserName));
	memset(m_stunPassServerPassword, 0, sizeof(m_stunPassServerPassword));
	memset(m_stunPassServerRealm, 0, sizeof(m_stunPassServerRealm));
	memset(m_stunServerHostName, 0, sizeof(m_stunServerHostName));
	memset(m_RelayServerHostName, 0, sizeof(m_RelayServerHostName));

	m_PassServerResolved = FALSE;
	m_StunServerResolved = FALSE;
	m_TurnServerResolved = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CWebRtcServiceManager::~CWebRtcServiceManager()
{

}

////////////////////////////////////////////////////////////////////////////
void CWebRtcServiceManager::NullActionFunction(CSegment* pParam)
{
	CSmallString str;
	str << "Opcode " << m_log[m_current_log_entry].eventStr << " arrived at state " << m_state << "\n";
	PTRACE2(eLevelInfoNormal, "CWebRtcServiceManager::NullActionFunction: ---- ! ---- ",str.GetString());
}


/////////////////////////////////////////////////////////////////////////////
void*  CWebRtcServiceManager::GetMessageMap()
{
  return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CWebRtcServiceManager::OnCSEndIceInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CWebRtcServiceManager::OnCardsMngrIceInitResponse.");
	DWORD id = 0;
	WORD NumOfCards = 0;
	//BYTE IsEnableBWPolicyCheck = FALSE;  //Currently always YES.
	BYTE atLeastOneBoardSucceed=FALSE;
	DWORD ice_init_status = STATUS_FAIL;

	CProcessBase *pProcess = CProcessBase::GetProcess();
	*pParam >> id;
	PTRACE2INT(eLevelInfoNormal,"WebRtcServiceManager::OnCSEndIceInd - id: ",id);
	*pParam >>NumOfCards;
	PTRACE2INT(eLevelInfoNormal,"WebRtcServiceManager::OnCSEndIceInd - NumOfCards: ",NumOfCards);

	WORD udp_status = eIceInitOk;
	WORD tcp_status = eIceInitOk;
	int  fwType 	= -1;
	if(NumOfCards)
	{
		ICE_INIT_IND_S ice_init_s;

		for(int i=0;i<NumOfCards;i++)
		{
			char errorText[100];
			pParam->Get( (BYTE*)&ice_init_s, sizeof(ICE_INIT_IND_S) );

			PTRACE2(eLevelInfoNormal,"CWebRtcServiceManager::OnCSEndIceInd Status:  ", ((ice_init_s.status) == STATUS_OK) ? "SUCCESS":"FAIL");

			memset(errorText, '\0', sizeof(errorText));

			getStatusErrorText((iceServersStatus)ice_init_s.Relay_udp_status, errorText);
			PTRACE2(eLevelInfoNormal,"CWebRtcServiceManager::OnCSEndIceInd Relay_udp_status:  ", errorText);

			memset(errorText, '\0', sizeof(errorText));

			getStatusErrorText((iceServersStatus)ice_init_s.Relay_tcp_status, errorText);
			PTRACE2(eLevelInfoNormal,"CWebRtcServiceManager::OnCSEndIceInd Relay_tcp_status:  ", errorText);


			if(ice_init_s.status == STATUS_OK)
			{
				udp_status = ice_init_s.Relay_udp_status;
				tcp_status = ice_init_s.Relay_tcp_status;
				fwType = ice_init_s.fw_type;
				if(udp_status==eIceInitOk || tcp_status==eIceInitOk || fwType == eFwTypeBlocked || ice_init_s.STUN_udp_status==eIceInitOk || ice_init_s.STUN_tcp_status==eIceInitOk ) //Blocked is for workaround - when can't connect to EDGE we don't have to use the relay server
				{
					atLeastOneBoardSucceed = TRUE;
					PTRACE(eLevelInfoNormal,"CWebRtcServiceManager::OnCSEndIceInd - atLeastOneBoardSucceed ");


					pProcess->RemoveActiveAlarmByErrorCodeUserIdFromProcess(AA_INITIALIZE_ICE_STACK_FAILURE,i);
				}
				else
				{
					memset(errorText, '\0', sizeof(errorText));

					getStatusErrorText((iceServersStatus)ice_init_s.Relay_tcp_status, errorText);
					PTRACE2(eLevelError,"CWebRtcServiceManager::OnCSEndIceInd ICE initiation Failed!! - status: ",errorText);
				}
			}
			else
			{
				PTRACE(eLevelError,"CWebRtcServiceManager::OnCSEndIceInd response Ack with status fail- status: ");
				pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
							AA_INITIALIZE_ICE_STACK_FAILURE,
							MAJOR_ERROR_LEVEL,
							"General Status failure",
							true,
							true);

			}

		}
	}
	if(atLeastOneBoardSucceed )
	{
		ice_init_status = STATUS_OK;
		PTRACE(eLevelInfoNormal,"CWebRtcServiceManager::OnCSEndIceInd - ICE initiation OK!! ");
	}
	else
	{
		ice_init_status = STATUS_FAIL;
		PTRACE(eLevelError,"CWebRtcServiceManager::OnCSEndIceInd - ICE initiation Failed!!");
	}

	CSegment *pSeg = new CSegment;
	*pSeg << (BYTE)ice_init_status;
	//*pSeg << (BYTE)IsEnableBWPolicyCheck;
	//*pSeg << (DWORD)0; // maxvideorateallowed !!
	*pSeg << (DWORD)eIceEnvironment_WebRtc;

	CTaskApi api(eProcessConfParty, eManager);
	api.SendMsg(pSeg, WEBRTC_ICE_SERVICE_MANG_TO_CONF_END_INIT_ICE);

}
/////////////////////////////////////////////////////////////////////////////
void  CWebRtcServiceManager::OnCSEndIceStatInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CWebRtcServiceManager::OnCSEndIceStatInd.");

	DWORD ice_status_update = STATUS_OK;
	ICE_STATUS_IND_S* pIceStatusInd =  (ICE_STATUS_IND_S*)pParam->GetPtr();

	// ===== 2. print the data to trace
	TRACESTR(eLevelInfoNormal)
		<< "\nCWebRtcServiceManager::OnCSEndIceStatInd: "
		<< "\nSTUN_udp_status:  " << pIceStatusInd->STUN_udp_status
		<< "\nSTUN_tcp_status:  " << pIceStatusInd->STUN_tcp_status
		<< "\nRelay_udp_status: " << pIceStatusInd->Relay_udp_status
		<< "\nRelay_tcp_status: " << pIceStatusInd->Relay_tcp_status
		<< "\nfw_type:          " << pIceStatusInd->fw_type
		<< "\nIce Type:			" << pIceStatusInd->ice_env;


	int sum = pIceStatusInd->STUN_udp_status + pIceStatusInd->STUN_tcp_status + pIceStatusInd->Relay_udp_status +pIceStatusInd->Relay_tcp_status ;
	PTRACE2INT(eLevelInfoNormal, "CWebRtcServiceManager::OnCSEndIceStatInd.sum = ", sum);

	if(sum != 0)
	{
		CProcessBase *pProcess = CProcessBase::GetProcess();
		PTRACE(eLevelError,"CWebRtcServiceManager::OnCSEndIceStatInd: Recieved WebRTC ICE_STATUS_IND -at least one server failed ");
		if(sum == 4)
		{
			ice_status_update = STATUS_FAIL;
			pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
								AA_WEBRTC_ICE_SERVER_FAILED,
								MAJOR_ERROR_LEVEL,
								"ALL WEBRTC ice Servers are down !!",
								true,
								true);
		}
		else
		{
			pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
					AA_WEBRTC_ICE_SERVER_FAILED,
					MAJOR_ERROR_LEVEL,
					"At least one ICE SERVER is down",
					true,
					true);
		}

	}




	CSegment *pSeg = new CSegment;
	*pSeg << (BYTE)ice_status_update;
	*pSeg << (DWORD)eIceEnvironment_WebRtc;

	CTaskApi api(eProcessConfParty, eManager);
	api.SendMsg(pSeg, SIP_PROXY_TO_CONF_END_STAT_ICE);

}
/////////////////////////////////////////////////////////////////////////////
void  CWebRtcServiceManager::SetWebRTCService(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CWebRtcServiceManager::SetWebRTCService ");
	char StunHostToResolve[H243_NAME_LEN];
	char RelayHostToResolve[H243_NAME_LEN];
	char StunPassHostToResolve[H243_NAME_LEN];

	CSipProxyIpParams* pService = new CSipProxyIpParams;
	pService->Deserialize(pSeg);
	COstrStream msg;
	pService->Dump(msg);
	PTRACE2(eLevelInfoNormal, "CWebRtcServiceManager::SetWebRTCService:\n ", msg.str().c_str());

	if (pService->GetIpType() == eIpType_IpV4)
	{
		for (int i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES; i++)
		{
			if (pService->GetServiceIpAddress(i).ipVersion == eIpVersion6)
				pService->SetServiceIpAddressToNull(i,0);
		}
	}
	else if (pService->GetIpType() == eIpType_IpV6)
	{
		for (int i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES; i++)
		{
			if (pService->GetServiceIpAddress(i).ipVersion == eIpVersion4)
				pService->SetServiceIpAddressToNull(i,1);

		}
	}

	for (int i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES; i++)
	{
		ipAddressStruct ipAddrSt = pService->GetServiceIpAddress(i);
		if (!IsIpNull(&ipAddrSt))
		{
			char a[200];
			ipToString(ipAddrSt, a, 0);
			break;
		}
	}

	COstrStream msg1;
	pService->Dump(msg1);
	PTRACE2(eLevelInfoNormal, "CWebRtcServiceManager::SetWebRTCService after:\n ", msg1.str().c_str());

	m_ServiceIceType = pService->GetICEType();
	POBJDELETE(m_pProxyService);
	m_pProxyService = new CSipProxyIpParams;
	*m_pProxyService = *pService;
	m_pICEserviceId = pService->GetServiceId();

	PTRACE2INT(eLevelInfoHigh, "CWebRtcServiceManager::SetWebRTCService #1 m_pProxyService->GetICEType: ", m_pProxyService->GetICEType());
	PTRACE2INT(eLevelInfoHigh, "CWebRtcServiceManager::SetWebRTCService #1 m_ServiceIceType: ", m_ServiceIceType);
	PTRACE2INT(eLevelInfoHigh, "CWebRtcServiceManager::SetWebRTCService #1 m_pICEserviceId: ", m_pICEserviceId);

	strncpy(m_stunServerHostName, pService->GetStunServerHostName(), H243_NAME_LEN-1); //kw added -1
	strncpy(m_RelayServerHostName, pService->GetRelayServerHostName(), H243_NAME_LEN-1); //kw added -1
	strncpy(m_stunPassServerHostName, pService->GetSTUNpassHostName(), H243_NAME_LEN-1);//kw added -1

	memset(StunHostToResolve, '\0', H243_NAME_LEN);
	memset(RelayHostToResolve, '\0', H243_NAME_LEN);
	memset(StunPassHostToResolve, '\0', H243_NAME_LEN);


	needToResolveHostname(StunHostToResolve,RelayHostToResolve,StunPassHostToResolve);

	if ( m_PassServerResolved && m_StunServerResolved && m_TurnServerResolved)
	{
		//IPV4/IPv6 configuration, we have all needed data, continue with ICE_INIT_REQ
		FillWebRTCICEparams(pService);

	} else {
		//put 0.0.0.0 to needed server IPs
		if(!m_StunServerResolved && isalnum(StunHostToResolve[0]) && strncmp(StunHostToResolve, "0.0.0.0",strlen("0.0.0.0")) !=0)
		{
			TRACEINTO << "\nStun host name : " << StunHostToResolve << " changed to 0.0.0.0";
			strncpy(m_stunServerUDPAddress.sIpAddr, "0.0.0.0", H243_NAME_LEN);
			strncpy(m_stunServerTCPAddress.sIpAddr, "0.0.0.0", H243_NAME_LEN);
			m_StunServerResolved = TRUE;
		}
		if(!m_TurnServerResolved && isalnum(RelayHostToResolve[0]) && strncmp(RelayHostToResolve, "0.0.0.0",strlen("0.0.0.0")) !=0)
		{
			TRACEINTO << "\nRelay host name : " << RelayHostToResolve << " changed to 0.0.0.0";;
			strncpy(m_RelayServerUDPAddress.sIpAddr, "0.0.0.0", H243_NAME_LEN);
			strncpy(m_RelayServerTCPAddress.sIpAddr, "0.0.0.0", H243_NAME_LEN);
			m_TurnServerResolved = TRUE;
		}
		if(!m_PassServerResolved && isalnum(StunPassHostToResolve[0]) && strncmp(StunPassHostToResolve, "0.0.0.0",strlen("0.0.0.0")) !=0)
		{
			TRACEINTO << "\nStun password server:" << StunPassHostToResolve << " changed to 0.0.0.0";;
			strncpy(m_stunPassServerAddress.sIpAddr, "0.0.0.0", H243_NAME_LEN);
			m_PassServerResolved = TRUE;
		}
		if ( m_PassServerResolved && m_StunServerResolved && m_TurnServerResolved)
		{

			FillWebRTCICEparams(pService);
		} else {
			//invalid HostName or empty setup
			DBGFPASSERT(99);
		}


	}


}

/////////////////////////////////////////////////////////////////////////////
void CWebRtcServiceManager::SetEndService()
{
	PTRACE(eLevelInfoNormal, "CWebRtcServiceManager::SetEndService ");

	// Send to CM
	UpdateCM();
}


/////////////////////////////////////////////////////////////////////////////
void CWebRtcServiceManager::FillWebRTCICEparams(CSipProxyIpParams* pService)
{
	////// STUN PASS //////
	char strIp[128];
	COstrStream msg;

	mcTransportAddress ipSTUNAddrSt = pService->GetSTUNpassIpAddress();

	if (eIpVersion4 == ipSTUNAddrSt.ipVersion)
	{
		if (ipSTUNAddrSt.addr.v4.ip)
		{
			SystemDWORDToIpString(ipSTUNAddrSt.addr.v4.ip, strIp);
			msg << std::setw(20) << "STUN password server IpV4: " << strIp << std::endl;
			strncpy(m_stunPassServerAddress.sIpAddr, strIp, IceStrLen - 1);
			m_stunPassServerAddress.sIpAddr[IceStrLen - 1] = 0;
		}
	}
	else
	{
		ipV6ToString(ipSTUNAddrSt.addr.v6.ip, strIp, TRUE);
		msg << std::setw(20) << "STUN password server IpV6: " << strIp << std::endl;
		strncpy(m_stunPassServerAddress.sIpAddr, strIp, IceStrLen - 1);
		m_stunPassServerAddress.sIpAddr[IceStrLen - 1] = 0;
	}

	msg << std::setw(20) << "STUN password server port: " << ipSTUNAddrSt.port << std::endl;
	msg << std::setw(20) << "STUN password server user name: " << pService->GetStunPassUserName() << std::endl;
	msg << std::setw(20) << "STUN password server host name: " << pService->GetSTUNpassHostName() << std::endl;

	m_stunPassServerAddress.port = ipSTUNAddrSt.port;
	if (0 == m_stunPassServerAddress.port)
	{
	    m_stunPassServerAddress.port = DEFAULT_STUN_PORT;
        msg << std::setw(20) << "STUN password server port (updated to default): " << m_stunPassServerAddress.port << std::endl;
	}

	memset(m_stunPassServerUserName, '\0', H243_NAME_LEN);
	strncpy(m_stunPassServerUserName, pService->GetStunPassUserName() , H243_NAME_LEN - 1);
	m_stunPassServerUserName[H243_NAME_LEN - 1] = '\0';

	msg << std::setw(20) << "STUN password server password: " << pService->GetStunPassPassword() << std::endl;

	memset(m_stunPassServerPassword, '\0', H243_NAME_LEN);
	strncpy(m_stunPassServerPassword, pService->GetStunPassPassword() , H243_NAME_LEN - 1);
	m_stunPassServerPassword[H243_NAME_LEN - 1] = '\0';

	////// STUN UDP //////
	mcTransportAddress ipStunUdpAddrSt = pService->GetStunUdpIpAddress();
	if (eIpVersion4 == ipStunUdpAddrSt.ipVersion)
	{
		if (ipSTUNAddrSt.addr.v4.ip)
		{
			SystemDWORDToIpString(ipStunUdpAddrSt.addr.v4.ip, strIp);
			msg << std::setw(20) << "STUN server UDP IpV4: " << strIp << std::endl;
			strncpy(m_stunServerUDPAddress.sIpAddr,strIp,IceStrLen - 1);
			m_stunServerUDPAddress.sIpAddr[IceStrLen - 1] = 0;
		}
	}
	else
	{
		ipV6ToString(ipStunUdpAddrSt.addr.v6.ip, strIp, TRUE);
		msg << std::setw(20) << "STUN server UDP IpV6: " << strIp << std::endl;
		strncpy(m_stunServerUDPAddress.sIpAddr,strIp,IceStrLen - 1);
		m_stunServerUDPAddress.sIpAddr[IceStrLen - 1] = 0;
	}

	msg << std::setw(20) << "STUN server UDP port: " << ipStunUdpAddrSt.port << std::endl;
	msg << std::setw(20) << "STUN server host name: " << pService->GetSTUNpassHostName() << std::endl;
	m_stunServerUDPAddress.port = ipStunUdpAddrSt.port;
    if (0 == m_stunServerUDPAddress.port)
    {
        m_stunServerUDPAddress.port = DEFAULT_STUN_PORT;
        msg << std::setw(20) << "STUN server UDP port (updated to default): " << m_stunServerUDPAddress.port << std::endl;
    }

	////// STUN TCP //////
	mcTransportAddress ipStunTcpAddrSt = pService->GetStunTcpIpAddress();
	if (eIpVersion4 == ipStunTcpAddrSt.ipVersion)
	{
		if (ipStunTcpAddrSt.addr.v4.ip)
		{
			SystemDWORDToIpString(ipStunTcpAddrSt.addr.v4.ip, strIp);
			msg << std::setw(20) << "STUN server TCP IpV4: " << strIp << std::endl;
			strncpy(m_stunServerTCPAddress.sIpAddr,strIp,IceStrLen - 1);
			m_stunServerTCPAddress.sIpAddr[IceStrLen - 1] = 0;
		}
	}
	else
	{
		ipV6ToString(ipStunTcpAddrSt.addr.v6.ip, strIp, TRUE);
		msg << std::setw(20) << "STUN server TCP IpV6: " << strIp << std::endl;
		strncpy(m_stunServerTCPAddress.sIpAddr,strIp,IceStrLen - 1);
		m_stunServerTCPAddress.sIpAddr[IceStrLen - 1] = 0;
	}

	msg << std::setw(20) << "STUN server TCP port: " << ipStunTcpAddrSt.port<< std::endl;
	m_stunServerTCPAddress.port = ipStunTcpAddrSt.port;
    if (0 == m_stunServerTCPAddress.port)
    {
        m_stunServerTCPAddress.port = DEFAULT_STUN_PORT;
        msg << std::setw(20) << "STUN server TCP port (updated to default): " << m_stunServerTCPAddress.port << std::endl;
    }

	////// Relay UDP //////
	mcTransportAddress ipRelayUdpAddrSt = pService->GetRelayUdpIpAddress();
	if (eIpVersion4 == ipRelayUdpAddrSt.ipVersion)
	{
		if (ipRelayUdpAddrSt.addr.v4.ip)
		{
			SystemDWORDToIpString(ipRelayUdpAddrSt.addr.v4.ip, strIp);
			msg << std::setw(20) << "RELAY server UDP IpV4: " << strIp << std::endl;
			strncpy(m_RelayServerUDPAddress.sIpAddr,strIp,IceStrLen - 1);
			m_RelayServerUDPAddress.sIpAddr[IceStrLen - 1] = 0;
		}
	}
	else
	{
		ipV6ToString(ipRelayUdpAddrSt.addr.v6.ip, strIp, TRUE);
		msg << std::setw(20) << "RELAY server UDP IpV6: " << strIp << std::endl;
		strncpy(m_RelayServerUDPAddress.sIpAddr,strIp,IceStrLen - 1);
		m_RelayServerUDPAddress.sIpAddr[IceStrLen - 1] = 0;
	}

	msg << std::setw(20) << "RELAY server UDP port: " << ipRelayUdpAddrSt.port << std::endl;
	msg << std::setw(20) << "RELAY server host name: " << pService->GetRelayServerHostName() << std::endl;
	m_RelayServerUDPAddress.port = ipRelayUdpAddrSt.port;
    if (0 == m_RelayServerUDPAddress.port)
    {
        m_RelayServerUDPAddress.port = DEFAULT_STUN_PORT;
        msg << std::setw(20) << "RELAY server UDP port (updated to default): " << m_RelayServerUDPAddress.port << std::endl;
    }

	////// Relay TCP //////
	mcTransportAddress ipRelayTcpAddrSt = pService->GetRelayTcpIpAddress();
	if (eIpVersion4 == ipRelayTcpAddrSt.ipVersion)
	{
		if (ipRelayTcpAddrSt.addr.v4.ip)
		{
			SystemDWORDToIpString(ipRelayTcpAddrSt.addr.v4.ip, strIp);
			msg << std::setw(20) << "RELAY server TCP IpV4: " << strIp << std::endl;
			strncpy(m_RelayServerTCPAddress.sIpAddr,strIp,IceStrLen - 1);
			m_RelayServerTCPAddress.sIpAddr[IceStrLen - 1] = 0;
		}
	}
	else
	{
		ipV6ToString(ipRelayTcpAddrSt.addr.v6.ip, strIp, TRUE);
		msg << std::setw(20) << "RELAY server TCP IpV6: " << strIp << std::endl;
		strncpy(m_RelayServerTCPAddress.sIpAddr,strIp,IceStrLen - 1);
		m_RelayServerTCPAddress.sIpAddr[IceStrLen - 1] = 0;
	}

	msg << std::setw(20) << "RELAY server TCP port: " << ipRelayTcpAddrSt.port << std::endl;
	m_RelayServerTCPAddress.port = ipRelayTcpAddrSt.port;
    if (0 == m_RelayServerTCPAddress.port)
    {
        m_RelayServerTCPAddress.port = DEFAULT_STUN_PORT;
        msg << std::setw(20) << "RELAY server TCP port (updated to default): " << m_RelayServerTCPAddress.port << std::endl;
    }

    PTRACE2(eLevelInfoNormal, "CWebRtcServiceManager::FillWebRTCICEparams :\n ", msg.str().c_str() );

	return;
}


/////////////////////////////////////////////////////////////////////////////
void CWebRtcServiceManager::UpdateCM()
{
	PTRACE(eLevelInfoNormal, "CWebRtcServiceManager::UpdateCM");

	char strIp[128];
	DWORD Id = (1<<16);
	COstrStream msg;

	/** building  ICE_INIT_REQ_S **/
	ICE_SERVER_TYPES_S	*pParams = new ICE_SERVER_TYPES_S;
	memset(pParams,0,sizeof(ICE_SERVER_TYPES_S));

	pParams->ice_env = eIceEnvironment_WebRtc;
	pParams->req_id = Id;
	pParams->service_id = m_ServiceId;

	strncpy(pParams->authParams.user_name,m_stunPassServerUserName,IceStrLen - 1);
	strncpy(pParams->authParams.password,m_stunPassServerPassword,IceStrLen - 1);
	strncpy(pParams->authParams.realm,m_stunPassServerRealm,IceStrLen - 1);
	pParams->authParams.user_name[IceStrLen - 1] = 0;
	pParams->authParams.password[IceStrLen - 1] = 0;
	pParams->authParams.realm[IceStrLen - 1] = 0;

	////// STUN PASS //////
	pParams->stun_pass_server_params.port = m_stunPassServerAddress.port;
	if (0 == pParams->stun_pass_server_params.port)
	{
	    pParams->stun_pass_server_params.port = DEFAULT_STUN_PORT;
	}
	strncpy(pParams->stun_pass_server_params.sIpAddr ,m_stunPassServerAddress.sIpAddr,IceStrLen - 1);
	pParams->stun_pass_server_params.sIpAddr[IceStrLen - 1] = 0;
	strncpy(pParams->stun_pass_server_params.sHostName,m_stunPassServerHostName,IceStrLen - 1);
	pParams->stun_pass_server_params.sHostName[IceStrLen - 1] = 0;
	////// STUN UDP //////
	pParams->stun_udp_server_params.port = m_stunServerUDPAddress.port;
    if (0 == pParams->stun_udp_server_params.port)
    {
        pParams->stun_udp_server_params.port = DEFAULT_STUN_PORT;
    }
	strncpy(pParams->stun_udp_server_params.sIpAddr ,m_stunServerUDPAddress.sIpAddr,IceStrLen - 1);
	pParams->stun_udp_server_params.sIpAddr[IceStrLen - 1] = 0;
	strncpy(pParams->stun_udp_server_params.sHostName,m_stunServerHostName,IceStrLen - 1);
	pParams->stun_udp_server_params.sHostName[IceStrLen - 1] = 0;
	////// STUN TCP //////
	pParams->stun_tcp_server_params.port = m_stunServerTCPAddress.port;
    if (0 == pParams->stun_tcp_server_params.port)
    {
        pParams->stun_tcp_server_params.port = DEFAULT_STUN_PORT;
    }
	strncpy(pParams->stun_tcp_server_params.sIpAddr ,m_stunServerTCPAddress.sIpAddr,IceStrLen - 1);
	pParams->stun_tcp_server_params.sIpAddr[IceStrLen - 1] = 0;
	strncpy(pParams->stun_tcp_server_params.sHostName,m_stunServerHostName,IceStrLen - 1);
	pParams->stun_tcp_server_params.sHostName[IceStrLen - 1] = 0;
	//////Relay UDP /////
	pParams->relay_udp_server_params.port = m_RelayServerUDPAddress.port;
    if (0 == pParams->relay_udp_server_params.port)
    {
        pParams->relay_udp_server_params.port = DEFAULT_STUN_PORT;
    }
	strncpy(pParams->relay_udp_server_params.sIpAddr ,m_RelayServerUDPAddress.sIpAddr,IceStrLen - 1);
	pParams->relay_udp_server_params.sIpAddr[IceStrLen - 1] = 0;
	strncpy(pParams->relay_udp_server_params.sHostName,m_RelayServerHostName,IceStrLen - 1);
	pParams->relay_udp_server_params.sHostName[IceStrLen - 1] = 0;
	//////Relay TCP /////
	pParams->relay_tcp_server_params.port = m_RelayServerTCPAddress.port;
    if (0 == pParams->relay_tcp_server_params.port)
    {
        pParams->relay_tcp_server_params.port = DEFAULT_STUN_PORT;
    }
	strncpy(pParams->relay_tcp_server_params.sIpAddr ,m_RelayServerTCPAddress.sIpAddr,IceStrLen - 1);
	pParams->relay_tcp_server_params.sIpAddr[IceStrLen - 1] = 0;
	strncpy(pParams->relay_tcp_server_params.sHostName,m_RelayServerHostName,IceStrLen - 1);
	pParams->relay_tcp_server_params.sHostName[IceStrLen - 1] = 0;
	///print for debug
	msg << std::setw(20) << "print for debug - Request ID: " << pParams->req_id<< std::endl;
	msg << std::setw(20) << "print for debug - user_name : " << pParams->authParams.user_name<< std::endl;
	msg << std::setw(20) << "print for debug - password :  " << pParams->authParams.password<< std::endl;
	msg << std::setw(20) << "print for debug - STUN password server IP: " << m_stunPassServerAddress.sIpAddr << std::endl;
	msg << std::setw(20) << "print for debug - STUN password server port: " << m_stunPassServerAddress.port << std::endl;
	msg << std::setw(20) << "print for debug - service id: " << m_ServiceId << std::endl;

	msg << std::setw(20) << "print for debug - stun udp port: " << pParams->stun_udp_server_params.port << std::endl;
	msg << std::setw(20) << "print for debug - stun udp IP: " << pParams->stun_udp_server_params.sIpAddr << std::endl;
	msg << std::setw(20) << "print for debug - stun tcp port: " << pParams->stun_tcp_server_params.port  << std::endl;
	msg << std::setw(20) << "print for debug - stun tcp IP: " << pParams->stun_tcp_server_params.sIpAddr << std::endl;
	msg << std::setw(20) << "print for debug - relay udp port: " << pParams->relay_udp_server_params.port << std::endl;
	msg << std::setw(20) << "print for debug - relay udp IP: " << pParams->relay_udp_server_params.sIpAddr << std::endl;
	msg << std::setw(20) << "print for debug - relay tcp port: " << pParams->relay_tcp_server_params.port  << std::endl;
	msg << std::setw(20) << "print for debug - relay tcp IP: " << pParams->relay_tcp_server_params.sIpAddr << std::endl;
	PTRACE2(eLevelInfoNormal, "CWebRtcServiceManager::UpdateCM : ", msg.str().c_str());


	CSegment*  pSegment = new CSegment;
	pSegment->Put( (BYTE*)pParams, sizeof(ICE_SERVER_TYPES_S) );
	delete pParams;

	CManagerApi apiCards(eProcessCards);
	apiCards.SendMsg(pSegment, SIPPROXY_TO_CARDS_ICE_INIT_REQ);
}

void CWebRtcServiceManager::needToResolveHostname(char *StunHostToResolve, char *RelayHostToResolve, char *StunPassHostToResolve)
{

	memset(StunHostToResolve, '\0', H243_NAME_LEN);
	memset(RelayHostToResolve, '\0', H243_NAME_LEN);
	memset(StunPassHostToResolve, '\0', H243_NAME_LEN);

	if(m_stunServerHostName[0] != 0) // we ask about 0 and not about '0' this would be dealt later
	{
		//we have turn server name only
		if((isIpV4Str(m_stunServerHostName)) || (isIpV6Str(m_stunServerHostName)))
		{
			TRACEINTO << "Found IP for STUN Server: " << m_RelayServerHostName;
			strncpy(m_stunServerUDPAddress.sIpAddr, m_stunServerHostName, H243_NAME_LEN - 1);
			m_stunServerUDPAddress.sIpAddr, m_stunServerHostName[H243_NAME_LEN - 1] = 0;
			strncpy(m_stunServerTCPAddress.sIpAddr, m_stunServerHostName, H243_NAME_LEN - 1);
			m_stunServerTCPAddress.sIpAddr[H243_NAME_LEN - 1] = 0;
			m_StunServerResolved = TRUE;

		} else {
			TRACEINTO << "Stun Server name need to be resolved: " << m_stunServerHostName;
			strncpy(StunHostToResolve, m_stunServerHostName, H243_NAME_LEN);

		}
	}


	if(m_RelayServerHostName[0] != 0)
	{
		if( (isIpV4Str(m_RelayServerHostName)) || (isIpV6Str(m_RelayServerHostName)) )
		{
			TRACEINTO << "Found IP for TURN Server: " << m_RelayServerHostName;

			strncpy(m_RelayServerUDPAddress.sIpAddr, m_RelayServerHostName, H243_NAME_LEN - 1);
			m_RelayServerUDPAddress.sIpAddr[H243_NAME_LEN - 1] = 0;
			strncpy(m_RelayServerTCPAddress.sIpAddr, m_RelayServerHostName, H243_NAME_LEN - 1 );
			m_RelayServerTCPAddress.sIpAddr[H243_NAME_LEN - 1] = 0;
			m_TurnServerResolved = TRUE;

		} else {
			TRACEINTO << "TURN Server name need to be resolved: " << m_RelayServerHostName;
			strncpy(RelayHostToResolve, m_RelayServerHostName, H243_NAME_LEN);
		}
	}


	if(m_stunPassServerHostName[0] == 0)
	{
		//No password server defined, that's fine we don't really need it.
		memset(m_stunPassServerAddress.sIpAddr, 0, sizeof(m_stunPassServerAddress.sIpAddr));
		m_PassServerResolved = TRUE;
	}
	else if( (isIpV4Str(m_stunPassServerHostName)) || (isIpV6Str(m_stunPassServerHostName)))
	{
		strncpy(m_stunPassServerAddress.sIpAddr, m_stunPassServerHostName, H243_NAME_LEN - 1);
		m_stunPassServerAddress.sIpAddr[H243_NAME_LEN - 1] = 0;
		m_PassServerResolved = TRUE;
	}
	else
	{
		TRACEINTO << "Password Server name need to be resolved: " << m_stunPassServerHostName;
		strncpy(StunPassHostToResolve, m_stunPassServerHostName, H243_NAME_LEN - 1);
		StunPassHostToResolve[H243_NAME_LEN - 1] = 0;
	}

}
