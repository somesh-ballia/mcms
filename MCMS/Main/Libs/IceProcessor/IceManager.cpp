// IceManager.cpp

#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__


#include <assert.h>
#include <stdarg.h>

#include "MplMcmsStructs.h"
#include "MplMcmsProtocol.h"
//#include "Opcodes.h"
//#include "Opcodes.h"
#include "DataTypes.h"
//#include "CommonDefs.h"
#include "IpChannelParams.h"
#include "IceManager.h"
//#include "IceDefs.h"
#include "auto_array.h"
#include "IceCmReq.h"
#include "IceCmInd.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "Segment.h"
#include "IceLogAdapter.h"
#include "IceSession.h"


CEventCallback::CEventCallback(IceManager *pIceManager)
	:m_iceManager(pIceManager)
{
	assert(m_iceManager);
	assert(m_iceManager->GetAFEngine());
}

// the actual callback implementation
void CEventCallback::HandleCallback(int iChannel, char *pData, int iLen, const CAfStdString& sSrcAddress, int iSrcPort, const void *pContext)
{
	int 	status;
	struct 	AfEvent* tEvent;

	tEvent = (AfEvent*)pData;

	//unsigned char temp = (unsigned char)*pData;
	if (tEvent->uEvent == EAfEventFirewallTypeDetected)
	{
		EAfDetectedFirewallType eFirewallType = m_iceManager->GetAFEngine()->GetFirewallType();
		switch(eFirewallType)
		{
			case EAfFirewallTypeNAT:
				ICE_LOG_TRACE("Firewall Type: NAT\n" );
				fprintf(stderr, "Firewall Type: NAT\n");
				break;
			case EAfFirewallTypeTCPOnly:
				ICE_LOG_TRACE("Firewall Type: TCP Only\n" );
				fprintf(stderr, "Firewall Type: TCP Only\n");
				break;
			case EAfFirewallTypeProxy:
				ICE_LOG_TRACE("Firewall Type: Proxy\n" );
				fprintf(stderr, "Firewall Type: Proxy\n");
				break;
			case EAfFirewallTypeBlocked:
				ICE_LOG_TRACE("Firewall Type: Blocked\n" );
				fprintf(stderr, "Firewall Type: Blocked\n");
				break;
			case EAfFirewallTypeUnknown:
				ICE_LOG_TRACE("Firewall type: Unknown\n" );
				fprintf(stderr, "Firewall type: Unknown\n");
				break;
			case EAfFirewallTypeNone:
				ICE_LOG_TRACE("Firewall Type: None\n" );
				fprintf(stderr, "Firewall Type: None\n");
				break;
			default:
				ICE_LOG_TRACE("Firewall type: Unknown\n" );
				fprintf(stderr, "Firewall type: Unknown\n");
				break;
		}

		m_iceManager->SetFirewallTypeDetectionStatus(true);
		m_iceManager->SendIceInitInd(eFirewallType);
	}
	else if (tEvent->uEvent == EAfEventStunPassServerAuthenticationFailure)
	{
		ICE_LOG_TRACE("Stun-pass server authentication failure\n" );
		fprintf(stderr, "Stun-pass server authentication failure\n");
	}
	else if(tEvent->uEvent == EAfEventUPnPDeviceDiscovered)
	{
		ICE_LOG_TRACE("UPnP device discovered\n" );
		fprintf(stderr, "UPnP device discovered\n");
	}
	else if (tEvent->uEvent == EAfEventStunPassServerConnectionFailure)
	{
		ICE_LOG_TRACE("Stun-pass server connection failure" );
		fprintf(stderr, "Stun-pass server connection failure");
	}
	else if (tEvent->uEvent == EAfEventNoDataReceived)
	{
		char szTempBuf[10];
		string sMsg =	"No data received for five seconds on channel " ;
		sMsg		+=	sprintf(szTempBuf, "%d", tEvent->uChannel);//, 10);
		sMsg        +=  "\n";
		ICE_LOG_TRACE((char*)sMsg.c_str() );
		fprintf(stderr, sMsg.c_str());

//		if((int)tEvent->uChannel == m_iAudioChannelRTP)
//			SendReinvite();
	}
	else if (tEvent->uEvent == EAfEventNoDataReceivedAfterFallbackToRelay)
	{
		char szTempBuf[10];
		string sMsg =	"No data received for five seconds on channel " ;
		sMsg		+=	sprintf(szTempBuf, "%d", tEvent->uChannel);//, 10);
		sMsg        +=  " after fallback to relay";
		sMsg        +=  "\n";
		ICE_LOG_TRACE((char*)sMsg.c_str() );
		fprintf(stderr, sMsg.c_str());

	} else if (tEvent->uEvent == EAfEventNetworkInterfacesChanged) {

		ICE_LOG_TRACE("NetworkInterfacesChanged, redetect conncetivity" );
		fprintf(stderr, "NetworkInterfacesChanged, redetect conncetivity");

		m_iceManager->SetFirewallTypeDetectionStatus(false);

		status = m_iceManager->GetAFEngine()->DetectConnectivity();

		if (status = false)
			ICE_LOG_TRACE("m_pAFEngine isn't initialize" );

//		if (status == true) {
//			status = m_pAFEngine->WaitForDetectConnectivity(10000);
//
//			if (status)
//				ICE_LOG_TRACE("2. Connectivity fail, status:%d" , status);
//		} else
//			ICE_LOG_TRACE("m_pAFEngine isn't initialize" );
	}
	if(tEvent->uEvent == 0)
	{
		ICE_LOG_TRACE("Error on event channel." );
               	// closed. quit
                //break;
//		m_iSipThreadID = 0;
	}

	return;

}

IceManager::IceManager(IceMplMsgSender &mplSender)
	:IceProcessor(mplSender),
	m_pAFEngine(0),
	m_bAFEngineInit(false),
	m_isFirewallTypeDetected(false),
	m_FireWallType(EAfFirewallTypeUnknown),
	m_lastIceSessionID(0)
{
	//status = IndexTableCreate(&m_sessionIndexTable, &m_sessionIndexTableArray[0], MaxSessions);

    //if(status) {
    //   AcLogTrap(status);
    //}

	m_iEventChannel = 0;

}

IceManager::~IceManager()
{
	for(ICE_SessionMap::iterator pos=m_IceSessionTable.begin();
		pos!=m_IceSessionTable.end(); ++pos) {
		delete pos->second;
	}
	m_IceSessionTable.clear();
	
	if(m_pAFEngine) {
		m_pAFEngine->Release();
		delete m_pAFEngine;
	}
}

void IceManager::SetStunPassParams(ICE_SERVER_PARAMS_S *pServerParams)
{
	m_sSTUNPassServerAddress 	= pServerParams->sIpAddr;

	m_iSTUNPassServerPort 		= pServerParams->port;
}

void IceManager::SetUserPasswordRealParams(ICE_SERVER_TYPES_S *pServerParams)
{
	m_sSTUNPassServerUsername 	= pServerParams->authParams.user_name;

	m_sSTUNPassServerPassword 	= pServerParams->authParams.password;

	m_sSTUNPassServerRealm 		= pServerParams->authParams.realm;
}

void IceManager::SetStunUdpParams(ICE_SERVER_PARAMS_S *pServerParams)
{
//	struct in_addr inAddr;
//
//	inAddr.s_addr = pServerParams->transAddr.addr.v4.ip;

	m_sSTUNUdpServerAddress = pServerParams->sIpAddr;//inet_ntoa(inAddr);

	m_iSTUNUdpServerPort 	= pServerParams->port;
}

void IceManager::SetStunTcpParams(ICE_SERVER_PARAMS_S *pServerParams)
{
//	struct in_addr inAddr;
//
//	inAddr.s_addr = pServerParams->transAddr.addr.v4.ip;

	m_sSTUNTcpServerAddress = pServerParams->sIpAddr;//inet_ntoa(inAddr);

	m_iSTUNTcpServerPort 	= pServerParams->port;
}

void IceManager::SetRelayUdpParams(ICE_SERVER_PARAMS_S *pServerParams)
{
//	struct in_addr inAddr;
//
//	inAddr.s_addr = pServerParams->transAddr.addr.v4.ip;

	m_sSTUNRelayUdpServerAddress 	= pServerParams->sIpAddr;//inet_ntoa(inAddr);

	m_iSTUNRelayUdpServerPort 		= pServerParams->port;
}

void IceManager::SetRelayTcpParams(ICE_SERVER_PARAMS_S *pServerParams)
{
//	struct in_addr inAddr;
//
//	inAddr.s_addr = pServerParams->transAddr.addr.v4.ip;

	m_sSTUNRelayTcpServerAddress 	= pServerParams->sIpAddr;//inet_ntoa(inAddr);

	m_iSTUNRelayTcpServerPort 		= pServerParams->port;
}

BOOL IceManager::StartServersConfig()
{
	bool bInit = FALSE;

	string sStunPassHost 	= "\0";
	string sStunHosts 		= "\0";
	string sStunRelayHosts 	= "\0";

	if(!m_pAFEngine) {
		ICE_LOG_TRACE("IceManager::StartServersConfig failed. AFEngine is not initiated yet." );
		return 0;
	}

	m_pAFEngine->SetSTUNUsernamePasswordRealm(
			m_sSTUNPassServerUsername,
			m_sSTUNPassServerPassword,
			m_sSTUNPassServerRealm);

//	Every server can be found by direct address or by DNS SRV.
// 	if the port is 0, we use DNS SRV to find the server description and to create it.
	if (!m_sSTUNPassServerAddress.empty()) {

		if (m_iSTUNPassServerPort)
			sStunPassHost = m_pAFEngine->CreateHost(
				AF_HOST_PUBLIC,
				m_sSTUNPassServerAddress,
				m_iSTUNPassServerPort,
				AF_PROTOCOL_TLS);
		else
			sStunPassHost = m_pAFEngine->CreateHost(
				AF_HOST_DNS_SRV,
				m_sSTUNPassServerAddress,
				m_iSTUNPassServerPort,
				AF_PROTOCOL_TLS);

		bInit = m_pAFEngine->SetSTUNPassServer(
			sStunPassHost);
	}

	if (!m_sSTUNUdpServerAddress.empty()) {
		if (m_iSTUNUdpServerPort)
			sStunHosts = m_pAFEngine->CreateHost(
				AF_HOST_PUBLIC,
				m_sSTUNUdpServerAddress,
				m_iSTUNUdpServerPort,
				AF_PROTOCOL_UDP);
		else
			sStunHosts = m_pAFEngine->CreateHost(
				AF_HOST_DNS_SRV,
				m_sSTUNUdpServerAddress,
				m_iSTUNUdpServerPort,
				AF_PROTOCOL_UDP);
	}

	if (!m_sSTUNTcpServerAddress.empty()) {
		if (m_iSTUNTcpServerPort)
			sStunHosts += m_pAFEngine->CreateHost(
				AF_HOST_PUBLIC,
				m_sSTUNTcpServerAddress,
				m_iSTUNTcpServerPort,
				AF_PROTOCOL_TCP);
		else
			sStunHosts += m_pAFEngine->CreateHost(
				AF_HOST_DNS_SRV,
				m_sSTUNTcpServerAddress,
				m_iSTUNTcpServerPort,
				AF_PROTOCOL_TCP);
	}

	if (!sStunHosts.empty())
		bInit = m_pAFEngine->SetSTUNServer(sStunHosts);

	if (!m_sSTUNRelayUdpServerAddress.empty()) {
		if (m_iSTUNRelayUdpServerPort)
			sStunRelayHosts = m_pAFEngine->CreateHost(
				AF_HOST_PUBLIC,
				m_sSTUNRelayUdpServerAddress,
				m_iSTUNRelayUdpServerPort,
				AF_PROTOCOL_UDP);
		else
			sStunRelayHosts = m_pAFEngine->CreateHost(
				AF_HOST_DNS_SRV,
				m_sSTUNRelayUdpServerAddress,
				m_iSTUNRelayUdpServerPort,
				AF_PROTOCOL_UDP);
	}

//	if (!m_sSTUNRelayTcpServerAddress.empty()) {
//		if (m_iSTUNRelayTcpServerPort)
//			sStunRelayHosts += m_pAFEngine->CreateHost(
//				AF_HOST_PUBLIC,
//				m_sSTUNRelayTcpServerAddress,
//				m_iSTUNRelayTcpServerPort,
//				AF_PROTOCOL_TCP);
//		else
//			sStunRelayHosts += m_pAFEngine->CreateHost(
//				AF_HOST_DNS_SRV,
//				m_sSTUNRelayTcpServerAddress,
//				m_iSTUNRelayTcpServerPort,
//				AF_PROTOCOL_TCP);
//	}

	if (!sStunRelayHosts.empty())
		bInit = m_pAFEngine->SetSTUNRelayServer(sStunRelayHosts);

	return bInit;
}

bool IceManager::Init(CMplMcmsProtocol &mplMsg)
{
	char 	ip[16];
	short 	port;
	int		transType;

	char    userName[IceStrLen];
	char	password[IceStrLen];
	char	realm[IceStrLen];

	m_lastMplMcmsMsg=std::auto_ptr<CMplMcmsProtocol>(new CMplMcmsProtocol(mplMsg));

	ICE_INIT_REQ_S *pIceConfig = (ICE_INIT_REQ_S *) mplMsg.GetData();

	ICE_SERVER_TYPES_S	*pServers = &pIceConfig->ice_servers;

	m_bIsMSOCS = pServers->ice_env==eIceEnvMs;

	// get stun pass server param
	ICE_SERVER_PARAMS_S	*pServerParams = &pServers->stun_pass_server_params;

	if (!m_bIsMSOCS)
		SetStunPassParams(pServerParams);

	SetUserPasswordRealParams(pServers);

	if (!m_bIsMSOCS) {

		pServerParams = &pServers->stun_udp_server_params;
		SetStunUdpParams(pServerParams);

		pServerParams = &pServers->stun_tcp_server_params;
		SetStunTcpParams(pServerParams);
	}

	pServerParams = &pServers->relay_udp_server_params;
	SetRelayUdpParams(pServerParams);

	pServerParams = &pServers->relay_udp_server_params;
	SetRelayTcpParams(pServerParams);

	if(!m_pAFEngine) {
		m_pAFEngine = new CAnyFirewallEngine();
	}
	
	if(!m_bAFEngineInit) {
		bool status=false;
		if (m_bIsMSOCS)
			status = m_pAFEngine->Init(AF_MODE_MSOCS, AF_OPTION_TRUE);
		else
			status = m_pAFEngine->Init(AF_MODE_STANDARD, AF_OPTION_TRUE);
	//	m_bAFEngineInit = af_init(2/*AF_MODE_MSOCS*/, AF_OPTION_FALSE, 100, 100, 100, 100);//

		if (status) {
			m_iEventChannel = m_pAFEngine->Create(AF_CHANNEL_EVENT, 0, 0, 0);

			// registering the callback with AnyFirewall Engine for an event channel
			CEventCallback		*pEventCallback = new CEventCallback(this);
			m_pAFEngine->SetCallbackHandler(m_iEventChannel, pEventCallback);

			m_bAFEngineInit=true;
		}
		else {
			ICE_LOG_TRACE("Failed to initialize AnyFirewallEngine." );
			if(m_pAFEngine->VersionCheckFailed())
				ICE_LOG_TRACE("Version check failed." );
			if(m_pAFEngine->LoadLibraryFailed())
				ICE_LOG_TRACE("Load library failed." );

			return false;
		}
	}

	StartServersConfig();

	//if server already have connected, do not DetectConnectivity again, otherwise the connect will be refused
	if(m_isFirewallTypeDetected) {
		SendIceInitInd(m_FireWallType);
		return true;
	}
	
	if (!m_pAFEngine->DetectConnectivity()) {
		ICE_LOG_TRACE("DetectConnectivity failed");
		return false;
	}
//		if (status == true) {
//			status = m_pAFEngine->WaitForDetectConnectivity(10000);
//
//			if (status)
//				ICE_LOG_TRACE("1. Connectivity fail, status:%d" , status);
//		} else
//			ICE_LOG_TRACE("m_pAFEngine isn't initialize" );

	return true;
}// IceInit

bool IceManager::SendIceInitInd(EAfDetectedFirewallType type)
{
	m_FireWallType=type;
	
	if(!m_lastMplMcmsMsg.get()) {
		ICE_LOG_TRACE("IceManager::SendIceInitInd failed, didn't receive ICE_INIT_REQ message before." );
		return false;
	}

	ICE_INIT_IND_S  iceInitIND;
	ICE_INIT_REQ_S *iceInitREQ=(ICE_INIT_REQ_S*)m_lastMplMcmsMsg->GetData();

	memset(&iceInitIND,0,sizeof(iceInitIND));
	iceInitIND.req_id=iceInitREQ->ice_servers.req_id;
	iceInitIND.status= STATUS_OK;
	iceInitIND.STUN_Pass_status=eIceServerUnavailble;
	iceInitIND.STUN_udp_status=eIceServerUnavailble;
	iceInitIND.STUN_tcp_status=eIceServerUnavailble;
	
	iceInitIND.Relay_tcp_status=eIceServerUnavailble;
	iceInitIND.Relay_udp_status=eIceServerUnavailble;
	iceInitIND.fw_type=eFwTypeUnknown;
	if(EAfFirewallTypeNAT==type) {
		iceInitIND.Relay_udp_status=eIceInitOk;
		iceInitIND.fw_type=eFwTypeUdp;
	}
	else if(EAfFirewallTypeTCPOnly==type) {
		iceInitIND.Relay_tcp_status=eIceInitOk;
		iceInitIND.fw_type=eFwTypeTcpOnly;
	}
	else if(EAfFirewallTypeNone==type) {
		iceInitIND.fw_type=eFwTypeNone;
	}
	else if(EAfFirewallTypeBlocked==type) {
		iceInitIND.fw_type=eFwTypeBlocked;
	}

	CMplMcmsProtocol iceInitInd(*m_lastMplMcmsMsg);
	iceInitInd.AddCommonHeader(ICE_INIT_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);

	iceInitInd.AddData(sizeof(iceInitIND),(char*)(&iceInitIND));

	SendForMplApi(iceInitInd);

	return true;
};

bool IceManager::CloseSession(CMplMcmsProtocol &mplMsg)
{
	IceSession *pIceSession = NULL;

	ICE_CLOSE_SESSION_REQ_S *pIceCloseSessionReq = (ICE_CLOSE_SESSION_REQ_S *) mplMsg.GetData();

	int	sessionId = pIceCloseSessionReq->ice_session_index;

	ICE_SessionMap::iterator pos=m_IceSessionTable.find(sessionId);
	if(pos==m_IceSessionTable.end()) {
		ICE_LOG_TRACE("CloseSession failed. Session doesn't exist, sessionId:%d" , sessionId);
		SendCloseSessionInd(mplMsg);
		return false;
	}

	pIceSession=pos->second;

//	delete m_iceSession;
	delete pIceSession;
	m_IceSessionTable.erase(pos);
	SendCloseSessionInd(mplMsg);

	return true;
}

IceSession* IceManager::NewSession()
{
	//bool status=false;
	//int	sessionId;

	BOOL bThreadFound = FALSE;

	if(!m_pAFEngine) {
		ICE_LOG_TRACE("IceManager::NewSession failed. AFEngine is not initiated yet." );
		return 0;
	}

	int sessId=++m_lastIceSessionID;
	ICE_SessionMap::const_iterator pos=m_IceSessionTable.find(sessId);
	if(pos!=m_IceSessionTable.end()) {
		ICE_LOG_TRACE("IceManager::NewSession failed. The ICE session(ID:%d) is already existed.", sessId );
		return 0;
	}

	IceSession *pIceSession = new IceSession(m_pAFEngine, sessId, m_MplMsgSender);

	m_IceSessionTable[sessId]=pIceSession;

	ICE_LOG_TRACE("IceManager::NewSession succeed. New ICE session(ID:%d)", sessId );
	//pIceSession->m_sessionId = sessionId;
	return pIceSession;

}// NewSession

IceSession* IceManager::GetIceSession(int iceSessionID)
{
	ICE_SessionMap::const_iterator pos=m_IceSessionTable.find(iceSessionID);

	if(pos==m_IceSessionTable.end()) {
		ICE_LOG_TRACE("IceManager: Can not find IceSession for ID: %d", iceSessionID);
		return 0;
	}

	return pos->second;
}

bool IceManager::NewOfferSession(CMplMcmsProtocol &mplMsg)
{
	//bool status=false;
	ICE_MAKE_OFFER_REQ_S *pMakeOffer = (ICE_MAKE_OFFER_REQ_S *) mplMsg.GetData();
	//mcIceChannelParams *pChannelParams = &pMakeOffer->candidate_list[0];

	if (!m_isFirewallTypeDetected) {
		ICE_LOG_TRACE("NewOfferSession failed, because FireWallType can not be detected.");
		return false;
	}

	IceSession *pIceSession = NewSession();
	if(!pIceSession)
		return false;

	pMakeOffer->ice_session_index=m_lastIceSessionID;
	
	pIceSession->Offer(mplMsg);
	//if(status) {
	//	localSdp=pIceSession->GetLocalSdp();
	//}

	return true;
}

bool IceManager::MakeOffer(CMplMcmsProtocol &mplMsg, int &iceSessionID)
{
	ICE_MAKE_OFFER_REQ_S *pMakeOffer = (ICE_MAKE_OFFER_REQ_S *) mplMsg.GetData();
	bool res=NewOfferSession(mplMsg);

	if(!res) {
		int size;
		std::string localSdp;//use empty SDP to send STATUS_FAIL indication
		auto_array<char> msgbuf=IceProcessor::BuildIceSdpIndByReq(*pMakeOffer, localSdp, size);

		mplMsg.AddCommonHeader(ICE_MAKE_OFFER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
		mplMsg.AddData(size,msgbuf.c_array());

		SendForMplApi(mplMsg);
	}
	else
		iceSessionID=m_lastIceSessionID;
		
	return true;
}// MakeOffer

bool IceManager::NewAnswerSession(CMplMcmsProtocol &mplMsg)
{
	bool status=false;
	ICE_MAKE_ANSWER_REQ_S &IceReq=*((ICE_MAKE_ANSWER_REQ_S*)mplMsg.GetData());
	//mcIceChannelParams *pChannelParams = &IceReq.candidate_list[0];

	if (!m_isFirewallTypeDetected)
		return false;

	if (IceReq.sdp_size) {
		IceSession *pIceSession = NewSession();
		if(!pIceSession)
			return false;

		IceReq.ice_session_index=m_lastIceSessionID;
		
		//pIceSession->StoreLastMplMcmsMsg(mplMsg);
		status = pIceSession->Answer(mplMsg);
		//if(status) {
		//	localSdp=pIceSession->GetLocalSdp();
		//}
	}
	else {
		ICE_LOG_TRACE("No SDP in ICE_MAKE_ANSWER_REQ message. This usage is not supported." );
		return false;
	}

	return true;
}

bool IceManager::MakeAnswer(CMplMcmsProtocol &mplMsg, int &iceSessionID)
{
	ICE_MAKE_ANSWER_REQ_S *pMakeAnswer=(ICE_MAKE_ANSWER_REQ_S*)mplMsg.GetData();
	bool res=NewAnswerSession(mplMsg);

	if(!res) {
		int size;
		std::string localSdp;//use empty SDP to send STATUS_FAIL indication
		auto_array<char> msgbuf=IceProcessor::BuildIceSdpIndByReq(*pMakeAnswer, localSdp, size);

		mplMsg.AddCommonHeader(ICE_MAKE_ANSWER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
		mplMsg.AddData(size,msgbuf.c_array());

		SendForMplApi(mplMsg);
	}
	else
		iceSessionID=m_lastIceSessionID;
	
	return true;
}// MakeAnswer

bool IceManager::MakeAnswerNoCreate(CMplMcmsProtocol &mplMsg)
{
	bool status=false;
	int sessionId;
	IceSession *pIceSession = NULL;

	ICE_MODIFY_SESSION_ANSWER_REQ_S *pMakeAnswer = (ICE_MODIFY_SESSION_ANSWER_REQ_S *) mplMsg.GetData();

	if (!m_isFirewallTypeDetected)
		return false;

	sessionId=pMakeAnswer->ice_session_index;
	ICE_SessionMap::const_iterator pos=m_IceSessionTable.find(sessionId);
	if(pos==m_IceSessionTable.end()) {
		ICE_LOG_TRACE("MakeAnswerNoCreate failed. Can not find IceSession for the ID:%d" , sessionId);
		return false;
	}

	pIceSession=pos->second;
	status=pIceSession->MakeAnswerNoCreate(mplMsg);

	return status;
}// MakeAnswerNoCreate

bool IceManager::MakeOfferNoCreate(CMplMcmsProtocol &mplMsg)
{
	bool status=false;
	int sessionId;
	IceSession *pIceSession = NULL;

	ICE_MODIFY_SESSION_OFFER_REQ_S *pMakeOffer = (ICE_MODIFY_SESSION_OFFER_REQ_S *) mplMsg.GetData();

	if (!m_isFirewallTypeDetected)
		return false;

	sessionId=pMakeOffer->ice_session_index;
	ICE_SessionMap::const_iterator pos=m_IceSessionTable.find(sessionId);
	if(pos==m_IceSessionTable.end()) {
		ICE_LOG_TRACE("MakeOfferNoCreate failed. Can not find IceSession for the ID:%d" , sessionId);
		return false;
	}

	pIceSession=pos->second;
	status=pIceSession->MakeOfferNoCreate(mplMsg);

	return status;
}// MakeAnswerNoCreate

//------------------------------------------------

bool IceManager::ProcessAnswer(CMplMcmsProtocol &mplMsg)
{
	bool status=false;

	IceSession *pIceSession = NULL;

	ICE_PROCESS_ANSWER_REQ_S *pIceProcAnswer = (ICE_PROCESS_ANSWER_REQ_S *) mplMsg.GetData();

	int sessionId = pIceProcAnswer->ice_session_index;

	ICE_SessionMap::const_iterator pos=m_IceSessionTable.find(sessionId);
	if(pos==m_IceSessionTable.end()) {
		ICE_LOG_TRACE("ProcessAnswer failed. Can not find IceSession for the ID:%d" , sessionId);
		return false;
	}

	pIceSession=pos->second;
	pIceSession->StoreLastMplMcmsMsg(mplMsg);
	status = pIceSession->ProcessAnswer(pIceProcAnswer->sdp);

	{
		ICE_PROCESS_ANSWER_IND_S procAnsInd;
		memset(&procAnsInd, 0, sizeof(procAnsInd));

		procAnsInd.status=status ? STATUS_OK:STATUS_FAIL;
		procAnsInd.ice_session_index=pIceProcAnswer->ice_session_index;

		CMplMcmsProtocol sendAnswerInd(mplMsg);
		sendAnswerInd.AddCommonHeader(ICE_PROCESS_ANSWER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
		sendAnswerInd.AddData(sizeof(procAnsInd), (char*)&procAnsInd);

		SendForMplApi(sendAnswerInd);
	}

	return status;

}// ProcessAnswer

#if 0
bool IceManager::ModifySession(void *pMsg, int opcode)
{
	bool status = false;

	IceSession *pIceSession = NULL;

	mcIceModifySessionReq *pIceModifySession = (mcIceModifySessionReq *) pMsg;

	int	sessionId = pIceModifySession->ice_session_index;

	ICE_SessionMap::const_iterator pos=m_IceSessionTable.find(sessionId);
	if(pos==m_IceSessionTable.end()) {
		ICE_LOG_TRACE("ModifySession failed. Can not find IceSession for the ID:%d" , sessionId);
		return false;
	}

	pIceSession=pos->second;

	mcIceChannelParams *pChannelParams = &pIceModifySession->candidate_list[0];

	if (opcode == ICE_MODIFY_SESSION_OFFER_REQ)
		status = pIceSession->ModifySessionOfferReq(pChannelParams, NULL);
	else
		status = pIceSession->ModifySessionAnswerReq(pChannelParams, pIceModifySession->sdp);

	return status;

}// ModifySession

void IceManager::GetIceSessionId(int *pId)
{
	*pId = m_iceSession->m_sessionId;
}

void IceManager::GetSdp(char *pSdp,int  sid)
{
	int status = 0;
	IceSession *pIceSession = NULL;

	if (status = IndexTableGetPtr(
		&m_sessionIndexTable,
		sid,
		(void**) &pIceSession)) {

		ICE_LOG_TRACE("GetSdpSize DirectIndexTableGetPtr fail, sessionId:%d" , sid);
		return ;
	}

	if (pIceSession)
		pIceSession->GetSdp(pSdp);

	return;
}

int IceManager::GetSdpSize(int sid)
{
	int status;
	IceSession *pIceSession = NULL;

	if (status = IndexTableGetPtr(
		&m_sessionIndexTable,
		sid,
		(void**) &pIceSession)) {

		ICE_LOG_TRACE("GetSdpSize DirectIndexTableGetPtr fail, sessionId:%d" , sid);
		return 0;
	}

	return 	(pIceSession ? pIceSession->GetSdpSize() : 0);
}
#endif

#endif //__DISABLE_ICE__
