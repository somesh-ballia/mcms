//+========================================================================+
//                  EpSimProxyConfList.cpp									   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
//+========================================================================+

#include <stdlib.h>

#include "Macros.h"
#include "Trace.h"
#include "IpCsOpcodes.h"
#include "SipUtils.h"

#include "TaskApi.h"
#include "MplMcmsProtocol.h"

#include "EndpointsSim.h"
#include "EpSimProxyConfList.h"
#include "CSSimTaskApi.h"
#include "OpcodesMcmsInternal.h"

static const char *MrasUri = "sip:insideedge.r13.vsg.local2@r13.vsg.local2;gruu;opaque=srvr:MRAS:_6B9h_10eEKox4mYPEHs7AAA";
static const char *isEnableBWPolicyCheck = "true";
static const char *ucMaxVideoRateAllowed = "VGA-600K";

static const char *RichPresence="1,1;0;0;0";

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//   CEpH323List - List of EP elements
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CProxyConfList::CProxyConfList( )      // constructor
{
	m_rcvMbx = new COsQueue();
	int i=0;
	for (i = 0; i < MAX_CONFS_IN_PROXY_LIST; i++)
		m_proxyConfArray[i] = NULL;
//	m_pCSApi = NULL;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CProxyConfList::~CProxyConfList()     // destructor
{
	int i=0;
	for (i = 0; i < MAX_CONFS_IN_PROXY_LIST; i++)
		POBJDELETE(m_proxyConfArray[i]);

	POBJDELETE( m_rcvMbx );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CProxyConfList::StamTest()
{
}


void CProxyConfList::SetTaskRcvMbx(const COsQueue& rcvMbx)
{
	(*m_rcvMbx) = rcvMbx;
}

//void CProxyConfList::SetCsApi( CTaskApi* pCSApi )
//{
//	m_pCSApi = pCSApi;
//}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CProxyConfList::HandleNewEvent( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CProxyConfList::HandleNewEvent");

	// search for the conf. if not exists, creating a new one
	DWORD proxyConfID  = pMplProtocol->getPortDescriptionHeaderConf_id();

	CProxyConf *currentProxyConf = GetCurrentProxyConf( proxyConfID  );
	if (NULL == currentProxyConf) {
		PTRACE(eLevelError,"CProxyConfList::HandleNewEvent Cannot find or create Proxy conf");
		return;
	}

	currentProxyConf->HandleNewEvent( pMplProtocol );

}

////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CProxyConf * CProxyConfList::GetCurrentProxyConf( DWORD proxyConfID  )
{
	PTRACE(eLevelInfoNormal,"CProxyConfList::GetCurrentProxyConf");

	int ind = -1;
	int nullFound = -1;
	int notFound = 1;

	for (int i = 0; i < MAX_CONFS_IN_PROXY_LIST; i++)
	{
		if (NULL == m_proxyConfArray[i]) {
			if (-1 == nullFound)
				nullFound = i;
			continue;
		}

		DWORD curProxyConfID = m_proxyConfArray[i]->GetConfID();
		if (curProxyConfID == proxyConfID )
		{
			ind = i;
			break;
		}
	}

	if (-1 == ind)	// conf not found
	{
		if (nullFound != -1)
		{
			ind = nullFound;
			m_proxyConfArray[nullFound] = new CProxyConf();
			m_proxyConfArray[nullFound]->InitProxyConfTimer( *m_rcvMbx );
			m_proxyConfArray[nullFound]->SetConfID( proxyConfID );
			//m_proxyConfArray[nullFound]->SetCsApi( m_pCSApi );
		}
	}

	if (ind != -1)
		return ( m_proxyConfArray[ind] );

	return NULL;
}


////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CProxyConfList::SelfRemoveConf( CSegment* pParam )
{

 	DWORD confId=0xFFFFFFFF;
	*pParam >> confId;

	int i=0;
	for (i = 0; i < MAX_CONFS_IN_PROXY_LIST; i++)
	{
		if (NULL == m_proxyConfArray[i])
			continue;
		if (m_proxyConfArray[i]->GetConfID() == confId )
			break;
	}

	if (i < MAX_CONFS_IN_PROXY_LIST)	// found
	{
		POBJDELETE( m_proxyConfArray[i] );
		PTRACE(eLevelInfoNormal,"CProxyConfList::SelfRemoveConf - Conf Deleted");
	}
	else
		PTRACE(eLevelInfoNormal,"CProxyConfList::SelfRemoveConf - Conf not found");

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//
//   CProxyConf - Endpoint element
//
/////////////////////////////////////////////////////////////////////////////


#define END_OF_REGISTRATION			10
#define TIME_REMOVE_REGISTRATION	11


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CProxyConf)

ONEVENT(1						,ANYCASE	,CProxyConf::OnStartElement)
ONEVENT(END_OF_REGISTRATION		,ANYCASE	,CProxyConf::OnTimerEndRegistration)
ONEVENT(TIME_REMOVE_REGISTRATION,ANYCASE	,CProxyConf::OnTimerRemoveRegistration)

PEND_MESSAGE_MAP(CProxyConf,CStateMachine);





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CProxyConf::CProxyConf( )      // constructor
{
	m_timerOn = 0;
//	m_pCSApi = NULL;
	m_registrationTime = 0;
	m_condId = 0;
	m_registered = 0;	// not register
	m_proxyApi = new CTaskApi();
	m_req = NULL;
	m_piggybackSupported=false;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CProxyConf::InitProxyConfTimer(const COsQueue& rcvMbx)
{
	m_proxyApi->CreateOnlyApi( rcvMbx );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CProxyConf::~CProxyConf()     // destructor
{
	BYTE* pPtr = (BYTE*)m_req; // for m_req memory allocated dynamically, need dynamical releasing
	PDELETEA(pPtr);
	m_req = NULL;

	if(m_proxyApi)
		m_proxyApi->DestroyOnlyApi();
	POBJDELETE( m_proxyApi );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//void CProxyConf::SetCsApi( CTaskApi* pCSApi )
//{
//	m_pCSApi = pCSApi;
//}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CProxyConf::HandleEvent( CSegment *pMsg,DWORD msgLen,OPCODE opCode )
{
	switch ( opCode )
	{
//	case TIMER:
//		{
//			break;
//		}

	default	:
		{
		DispatchEvent( opCode, pMsg );
		break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CProxyConf::HandleNewEvent( CMplMcmsProtocol* pMplProtocol )
{

	/*
define SIP_CS_PROXY_REGISTER_REQ							14500003
#define SIP_CS_PROXY_SUBSCRIBE_REQ							14500004
#define SIP_CS_PROXY_SUBSCRIBE_RESP_REQ						14500005
#define SIP_CS_PROXY_UNKNOWN_METHOD_REQ						14500006
#define SIP_CS_PROXY_NOTIFY_REQ								14500007
	 */

	switch( pMplProtocol->getOpcode() )
	{
//		case SIP_CS_SIG_REGISTER_REQ: {
		case SIP_CS_PROXY_REGISTER_REQ: {
			OnCSRegistrationReq( pMplProtocol );
			break;
			}

		case SIP_CS_PROXY_SUBSCRIBE_REQ: {
			OnCSSubscribeReq( pMplProtocol );
			break;
			}
		case SIP_CS_PROXY_SERVICE_REQ: {
			OnCSServiceReq( pMplProtocol );
			break;
			}
                case SIP_CS_PROXY_UNKNOWN_METHOD_REQ: {
                        PTRACE(eLevelInfoNormal,"CProxyConf::HandleNewEvent - ignore SIP_CS_PROXY_UNKNOWN_METHOD_REQ");
                        break;
                        }
		default   :  {
		    	if (NULL == getenv ("IGNORE_UNHANDLED_ASSERT"))
		            PASSERT(pMplProtocol->getOpcode()+1000);
			break;
			}

	}

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
DWORD CProxyConf::GetConfID()
{
	return m_condId;
}

void CProxyConf::SetConfID( DWORD confId)
{
	m_condId = confId;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CProxyConf::OnStartElement( CSegment* pParam )
{
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CProxyConf::OnTimerEndRegistration( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CProxyConf::OnTimerEndRegi371stration ");

	if (0 == m_timerOn)
		return; // nothing to do

	m_timerOn = 0;

	// 10 minutes fr debug purpuse, to save the API structures
	StartTimer( TIME_REMOVE_REGISTRATION, 10*6000); // 10 minutes
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CProxyConf::OnTimerRemoveRegistration( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CProxyConf::OnTimerRemoveRegistration ");

	if (0 != m_timerOn) {
		DeleteTimer( END_OF_REGISTRATION );	// 60 seconds
		m_timerOn = 0;
	}

	// need to remove this conf from proxy
	CSegment* pMsg = new CSegment;
	*pMsg << m_condId;
	m_proxyApi->SendMsg(pMsg, PROXY_REMOVE_CONF );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CProxyConf::OnCSRegistrationReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CProxyConf::OnCSRegistrationReq - SIP_CS_SIG_REGISTER_REQ");

	// stop timer if needed
	if (0 != m_timerOn) {
		DeleteTimer( END_OF_REGISTRATION );	// 60 seconds
		m_timerOn = 0;
	}
	m_registered = 0;

	// get the request struct
	mcReqRegister *req = (mcReqRegister*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CProxyConf::OnCSRegistrationReq - GetData is NULL ");
		return;
	}

	// checks if Cancel Registration
	if (0 == req->expires) {
		// cancel registration
		PTRACE(eLevelInfoNormal,"CProxyConf::OnCSRegistrationReq - expires=0 ");
		StartTimer( TIME_REMOVE_REGISTRATION, 10*6000); // 10 minutes
		return;
	}

	// gets the conf URI
	CSipHeaderList *pTemp = new CSipHeaderList(req->sipHeaders);
	const CSipHeader* pTo = pTemp->GetNextHeader(kTo);
	if(pTo) {
		const char *pToStr = pTo->GetHeaderStr();
		if (pToStr)
		{
		  strncpy(m_confUri, pToStr, sizeof(m_confUri)-1);
		  m_confUri[sizeof(m_confUri)-1] = '\0';
		}
	}

	// free request struct
	BYTE* pPtr = (BYTE*)m_req; // for m_req memory allocated dynamically, need dynamical releasing
	PDELETEA(pPtr);
	m_req = NULL;

	// save request struct
	int totalSize = sizeof(mcReqRegisterBase) - sizeof(sipMessageHeadersBase) + pTemp->GetTotalLen();
	m_req = (mcReqRegister*) new BYTE[totalSize];
	memcpy( m_req, req, totalSize);

	POBJDELETE( pTemp );

	// gets registration time
	m_registrationTime = min((APIS32)60, m_req->expires);	// seconds

	// start timer (end of registration)
	StartTimer( END_OF_REGISTRATION, m_registrationTime*100 );	// 60 seconds
	m_timerOn = 1;	// also sign registration

	// send indication
	SendCSRegistrationInd( pMplProtocol );
}

//////////////////////////////////////////////////////////////////////
void  CProxyConf::OnCSSubscribeReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CProxyConf::OnCSSubscribeReq - SIP_CS_PROXY_SUBSCRIBE_REQ");


	// get the request struct
	mcReqSubscribe *req = (mcReqSubscribe*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CProxyConf::OnCSSubscribeReq - GetData is NULL ");
		return;
	}

	// checks if Cancel Subscribe?
	//if (0 == req->expires) {
	//}

	// check if support piggyback
	CSipHeaderList *pTemp = new CSipHeaderList(req->sipHeaders);

	const CSipHeader* pSupported = pTemp->GetNextHeader(kSupported);
	if(pSupported) {
		const char* pSupportedStr = pSupported->GetHeaderStr();
		PTRACE2(eLevelError,"CProxyConf::OnCSSubscribeReq - kSupported: ",pSupportedStr);
		if(strstr(pSupportedStr, "ms-piggyback-first-notify"))
		{
			m_piggybackSupported=true;
			PTRACE(eLevelError,"CProxyConf::OnCSSubscribeReq - piggy is supported ");
		}
		else
			m_piggybackSupported=false;
	}

	// send indication
	SendCSSubscribeResInd( pMplProtocol );

	if(!m_piggybackSupported) {
		SendCSNotifyInd(pMplProtocol);
	}
	POBJDELETE(pTemp);
}

//////////////////////////////////////////////////////////////////////
void  CProxyConf::OnCSServiceReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CProxyConf::OnCSServiceReq - SIP_CS_PROXY_SERVICE_REQ");

	// get the request struct
	mcReqService *req = (mcReqService*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CProxyConf::OnCSServiceReq - GetData is NULL ");
		return;
	}

	// send indication mcIndServiceResp
	SendCSServiceRespInd( pMplProtocol );
}

/////////////////////////////////////////////////////////////////////////////
void CProxyConf::SendCSSubscribeResInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CProxyConf::SendCSSubscribeResInd - SIP_CS_PROXY_SUBSCRIBE_RESPONSE_IND ");

	// create basic ind
	mcIndSubscribeResp *ind = NULL;

	// class that treats the CS headers
	mcReqSubscribe *req = (mcReqSubscribe*)pMplProtocol->GetData();
	CSipHeaderList *pTemp = new CSipHeaderList(req->sipHeaders);
	if(m_piggybackSupported) {
		pTemp->AddHeader(kMrasUri, strlen(MrasUri), MrasUri);
		pTemp->AddHeader(kRichPresence,strlen(RichPresence),RichPresence);
		pTemp->AddHeader(kEnableBWPolicyCheck, strlen(isEnableBWPolicyCheck), isEnableBWPolicyCheck);
		pTemp->AddHeader(kUcMaxVideoRateAllowed, strlen(ucMaxVideoRateAllowed), ucMaxVideoRateAllowed);
	}

	// allocate indication struct
	int SipHeadersLen = pTemp->GetTotalLen();
	int totalSize = sizeof(mcIndSubscribeRespBase) - sizeof(sipMessageHeadersBase) + SipHeadersLen;
	ind = (mcIndSubscribeResp*) new BYTE[totalSize];
	memset( ind, 0, totalSize);			// zeroing the struct

	// fills the ind struct
	ind->id = req ->id;				// from the request
	ind->status=STATUS_OK;
	ind->expires = req ->expires;	// in seconds
	ind->subOpcode = req ->subOpcode;
	//memcpy( &ind->sipHeaders, &m_req->sipHeaders, SipHeadersLen);	// copy from request
	pTemp->BuildMessage(&ind->sipHeaders);

	// send the struct to CS simulation
	// ================================
	CCSSimTaskApi api;
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, api.GetCSID(),
                     SIP_CS_PROXY_SUBSCRIBE_RESPONSE_IND,
                     (BYTE*)(ind), totalSize);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	//if(m_pCSApi)
	//	m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	delete pTemp;
	char* ptr = (char*)ind;
	PDELETEA(ptr);
}

void CProxyConf::SendCSNotifyInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CProxyConf::SendCSNotifyInd - SIP_CS_PROXY_NOTIFY_IND ");

	// create basic ind
	mcIndNotify *ind = NULL;

	// class that treats the CS headers
	//mcReqSubscribe *req = (mcReqSubscribe*)pMplProtocol->GetData();
	CSipHeaderList *pTemp = new CSipHeaderList(8,0);

	pTemp->AddHeader(kMrasUri, strlen(MrasUri), MrasUri);

	// allocate indication struct
	int SipHeadersLen = pTemp->GetTotalLen();
	int totalSize = SipHeadersLen;
	ind = (mcIndNotify*) new BYTE[totalSize];
	memset( ind, 0, totalSize);			// zeroing the struct

	// fills the ind struct
	//memcpy( &ind->sipHeaders, &m_req->sipHeaders, SipHeadersLen);	// copy from request
	pTemp->BuildMessage(&ind->sipHeaders);

	// send the struct to CS simulation
	// ================================
	CCSSimTaskApi api;
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, api.GetCSID(),
                     SIP_CS_PROXY_NOTIFY_IND, (BYTE*)ind, totalSize);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	//if(m_pCSApi)
	//	m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	delete pTemp;
	char* ptr = (char*)ind;
	PDELETEA(ptr);
}

void CProxyConf::SendCSServiceRespInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CProxyConf::SendCSServiceRespInd - SIP_CS_PROXY_SERVICE_RESPONSE_IND");

	// create basic ind
	mcIndServiceResp *ind = NULL;

	// class that treats the CS headers
	mcReqService *req = (mcReqService*)pMplProtocol->GetData();
	CSipHeaderList *pTemp = new CSipHeaderList(8,0);

	const char *CredUser = "AgAAJMXNbKABynOSrECWwpK3+sRc7IZ01003lriWfcwAAAAAC+VEe4xIpRYucNyx7GCanNy6590=";
	const char *CredPass = "EKS8lJIh1zkTNVZCEH93L+Mo9qM=";
	const char *CredDuration = "480";
	const char *MRelayLocation = "intranet";
	const char *MRelayHostName = "insideedge.r13.vsg.local2";
	const char *MRelayUdpPort = "3478";
	const char *MRelayTcpPort = "443";

	pTemp->AddHeader(kCredUser, strlen(CredUser), CredUser);
	pTemp->AddHeader(kCredPass, strlen(CredPass), CredPass);
	pTemp->AddHeader(kCredDuration, strlen(CredDuration), CredDuration);
	pTemp->AddHeader(kMRelayLocation, strlen(MRelayLocation), MRelayLocation);
	pTemp->AddHeader(kMRelayHostName, strlen(MRelayHostName), MRelayHostName);
	pTemp->AddHeader(kMRelayUdpPort, strlen(MRelayUdpPort), MRelayUdpPort);
	pTemp->AddHeader(kMRelayTcpPort, strlen(MRelayTcpPort), MRelayTcpPort);

	// allocate indication struct
	int SipHeadersLen = pTemp->GetTotalLen();
	int totalSize = sizeof(mcIndServiceResp) - sizeof(sipMessageHeaders) + SipHeadersLen;
	ind = (mcIndServiceResp*) new BYTE[totalSize];
	memset( ind, 0, totalSize);			// zeroing the struct

	// fills the ind struct
	ind->status=STATUS_OK;
	ind->id=req->id;
	ind->subOpcode = req ->subOpcode;
	//memcpy( &ind->sipHeaders, &m_req->sipHeaders, SipHeadersLen);	// copy from request
	pTemp->BuildMessage(&ind->sipHeaders);

	// send the struct to CS simulation
	// ================================
	CCSSimTaskApi api;
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, api.GetCSID(),
                     SIP_CS_PROXY_SERVICE_RESPONSE_IND, (BYTE*)ind, totalSize);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	//if(m_pCSApi)
	//	m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	delete pTemp;
	char* ptr = (char*)ind;
	PDELETEA(ptr);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CProxyConf::SendCSRegistrationInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CProxyConf::SendCSRegistrationInd - SIP_CS_PROXY_REGISTER_RESPONSE_IND ");

	// create basic ind
	mcIndRegisterResp *ind = NULL;

	// class that treats the CS headers
	CSipHeaderList *pTemp = new CSipHeaderList(m_req->sipHeaders);
	const char *allowEventHdr="vnd-microsoft-provisioning,vnd-microsoft-roaming-contacts,vnd-microsoft-roaming-ACL,presence,presence.wpending,vnd-microsoft-roaming-self,vnd-microsoft-provisioning-v2";
	const char *userAgentHdr="RTC/4.0";

	pTemp->AddHeader(kAllowEvent, strlen(allowEventHdr), allowEventHdr);
	pTemp->AddHeader(kUserAgent, strlen(userAgentHdr), userAgentHdr);

	// allocate indication struct
	int SipHeadersLen = pTemp->GetTotalLen();
	int totalSize = sizeof(mcIndRegisterRespBase) - sizeof(sipMessageHeadersBase) + SipHeadersLen;
	ind = (mcIndRegisterResp*) new BYTE[totalSize];
	memset( ind, 0, totalSize);			// zeroing the struct

	// fills the ind struct
	ind->id = m_req->id;				// from the request
	ind->status=STATUS_OK;
	ind->expires = m_registrationTime;	// in seconds
	//memcpy( &ind->sipHeaders, &m_req->sipHeaders, SipHeadersLen);	// copy from request
	pTemp->BuildMessage(&ind->sipHeaders);

	char pTempStr[128];
	sprintf(pTempStr,"%ld ",ind->expires);
	PTRACE2(eLevelInfoNormal,"CProxyConf::SendCSRegistrationInd -  expires: ", pTempStr);

	// send the struct to CS simulation
	// ================================
	CCSSimTaskApi api;
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, api.GetCSID(),
                     SIP_CS_PROXY_REGISTER_RESPONSE_IND, (BYTE*)ind, totalSize);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

//	if(m_pCSApi)
//		m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	delete pTemp;
	char* ptr = (char*)ind;
	PDELETEA(ptr);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CProxyConf::SendCommandToCS(DWORD opcode, BYTE* buffer, WORD bufferLen )
{
	CSegment *pMsg = new CSegment();
	*pMsg << (DWORD)opcode;
	*pMsg << (WORD)bufferLen;
	pMsg->Put( (BYTE*)buffer, bufferLen );

	CCSSimTaskApi api;
        if(api.CreateOnlyApi() >= 0)
            api.SendMsg(pMsg, SEND_TO_CSAPI);

//	m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);
}





