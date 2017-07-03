// DNSAgentManager.cpp

#include "DNSAgentManager.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "SystemFunctions.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsCommon.h"
#include "IpCsOpcodes.h"
#include "Trace.h"
#include "DNSAgentProcess.h"
#include "Segment.h"
#include "DNSAgentManagerStructs.h"
#include "CsCommonStructs.h"
#include "MplMcmsProtocol.h"
#include "FaultsContainer.h"
#include "StatusesGeneral.h"
#include "SystemQuery.h"
#include "FaultsDefines.h"
#include "DNSAgentStatuses.h"
#include "TraceStream.h"
#include "ManagerApi.h"
#include "SipProxyTaskApi.h"
#include "GkTaskApi.h"
#include  "NStream.h"
#include "IpCsDnsApi.h"
#include "TerminalCommand.h"

//-S- PLCM_DNS ------------------------------//
#include "PlcmDNS_Tools.h" 
#include "PlcmDNS_Processing.h" 

cPLCM_DNS	 G_PlcmDns;

cPLCM_DNS * G_GetPlsmDnsPtr()
{
	return &G_PlcmDns;
}
//-E- PLCM_DNS ------------------------------//

// definitions
#define TIMER_RESOLVE_REQ		100
#define TIMER_REFRESH_TOUT		101

#define	RefreshTimeOutDeviation		10
#define ResolveTout					40
#define StandardTTL					86400//in seconds =  24 hours

PBEGIN_MESSAGE_MAP(CDNSQuery)
  ONEVENT(TIMER_RESOLVE_REQ,			DNS_RESOLVING,			CDNSQuery::OnResolveToutRESOLVING )
  ONEVENT(DNS_RESOLVE_IND,				DNS_RESOLVING,			CDNSQuery::OnResolveIndRESOLVING  )
  ONEVENT(TIMER_REFRESH_TOUT,			DNS_VALID,				CDNSQuery::OnRefreshTimerVALID    )
  ONEVENT(TIMER_RESOLVE_REQ,			DNS_VALID,				CDNSQuery::OnResolveToutVALID     )
  ONEVENT(DNS_RESOLVE_IND,				DNS_VALID,				CDNSQuery::OnResolveIndVALID      )
PEND_MESSAGE_MAP(CDNSQuery, CStateMachine);

CDNSQuery::CDNSQuery()
{
	m_state = DNS_IDLE;
	m_queryType = eResolveQuery;
	m_protocolType = eIPProtocolType_None;
	m_processType=eProcessTypeInvalid;
	m_connId = 0;
	m_partyId = 0;
	m_transportType = 0;
	m_serviceID =  999;
	m_index = 999;
	m_strDomainName = new char[DnsQueryNameSize];
	m_strDomainName[0] = '\0';
	m_strService[0] = '\0';
	setAddressesToZero(m_resolvedAddresses);
	m_timerRefresh = 0;
	m_timerResolve = 0;
	m_pDNSMngrRcvMbx = NULL;
	VALIDATEMESSAGEMAP;
}

CDNSQuery::CDNSQuery(const CDNSQuery& other) :CStateMachine(other)
{
	m_state = other.m_state;
	m_queryType = other.m_queryType;
	m_protocolType = other.m_protocolType;
	m_processType=other.m_processType;
	m_connId=other.m_connId;
	m_partyId=other.m_partyId;
	m_transportType = other.m_transportType;
	m_serviceID =  other.m_serviceID;
	m_index = other.m_index;
	m_strDomainName = new char[DnsQueryNameSize];
	strncpy(m_strDomainName, other.m_strDomainName, DnsQueryNameSize);
	strncpy(m_strService, other.m_strService, NET_SERVICE_PROVIDER_NAME_LEN);

	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memcpy(&m_resolvedAddresses[i], &other.m_resolvedAddresses[i], sizeof(ipAddressStruct));
	}
	m_timerRefresh = other.m_timerRefresh;
	m_timerResolve =other.m_timerResolve;

	m_pDNSMngrRcvMbx = other.m_pDNSMngrRcvMbx;

	VALIDATEMESSAGEMAP;
}

/////////////////////////////////////////////////////////////////////////////
CDNSQuery::~CDNSQuery()
{
	DEALLOCBUFFER(m_strDomainName);
	if (m_timerResolve)
		DeleteTimer( TIMER_RESOLVE_REQ );
	if (m_timerRefresh)
		DeleteTimer( TIMER_REFRESH_TOUT );

	DeleteAllTimers();
}

/////////////////////////////////////////////////////////////////////////////
CDNSQuery& CDNSQuery::operator = (const CDNSQuery& other)
{
	if(this == &other)
		return *this;

	m_state = other.m_state;
	m_queryType = other.m_queryType;
	m_protocolType = other.m_protocolType;
	m_processType=other.m_processType;
	m_connId=other.m_connId;
	m_partyId=other.m_partyId;
	m_transportType = other.m_transportType;
	m_serviceID =  other.m_serviceID;
	m_strService[0]='\0';
	m_index = other.m_index;

	DEALLOCBUFFER(m_strDomainName);
	m_strDomainName = new char[DnsQueryNameSize];
	strncpy(m_strDomainName, other.m_strDomainName, DnsQueryNameSize);

	strncpy(m_strService, other.m_strService, NET_SERVICE_PROVIDER_NAME_LEN);

	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memcpy(&m_resolvedAddresses[i], &other.m_resolvedAddresses[i], sizeof(ipAddressStruct));
	}
	m_timerRefresh = other.m_timerRefresh;
	m_timerResolve =other.m_timerResolve;

	m_pDNSMngrRcvMbx = other.m_pDNSMngrRcvMbx;
	return *this;
}

void CDNSQuery::HandleEvent(CSegment* pMsg)
{
	DWORD  msgLen;
	WORD   opCode;

	*pMsg >> msgLen;
	*pMsg >> opCode;

	DispatchEvent(opCode,pMsg);
}

void CDNSQuery::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
		DispatchEvent(opCode,pMsg);

}

int  CDNSQuery::Create(WORD serviceID,WORD index, char* pDomainName, COsQueue* pRcvMbx
					   , eIPProtocolType protocolType, DWORD transportType, eProcessType processType,
					   DWORD connId, DWORD  partyId)
{

	DNS_CS_RESOLVE_REQ_ST csDnsResolveReq;

	PTRACE2(eLevelInfoNormal, "CDNSQuery::Create,  domain name: ", pDomainName);

	csDnsResolveReq.transactionId = index;
	memcpy(&csDnsResolveReq.hostName[0], pDomainName, DnsQueryNameSize);

	m_serviceID = serviceID;
	m_index = index;
	strncpy(m_strDomainName, pDomainName, DnsQueryNameSize);
	m_protocolType  = protocolType;
	m_transportType = transportType;
	m_processType = processType;
	m_connId = connId;
	m_partyId = partyId;

	m_pDNSMngrRcvMbx = pRcvMbx;

	m_state = DNS_RESOLVING;

	m_ttl	= StandardTTL;

	SendToCsApi(DNS_CS_RESOLVE_REQ, sizeof(DNS_CS_RESOLVE_REQ_ST), (char *) &csDnsResolveReq);

	return 0;    // OK
}

/////////////////////////////////////////////////////////////////////////////
WORD  CDNSQuery::GetServiceId()
{
	return m_serviceID;
}

/////////////////////////////////////////////////////////////////////////////
const char* CDNSQuery::GetDomainName() const
{
	return m_strDomainName;
}

/////////////////////////////////////////////////////////////////////////////
const char* CDNSQuery::GetServiceName() const
{
	return (char*)&m_strService;
}

/////////////////////////////////////////////////////////////////////////////
/*
mcTransportAddress* CDNSQuery::GetIp()
{
	return &m_resolvedAddress;
}
*/
/////////////////////////////////////////////////////////////////////////////
ipAddressStruct* CDNSQuery::GetAdressList()
{
	return m_resolvedAddresses;
}

DWORD CDNSQuery::GetPort()
{
	return m_resolvedPort;
}

ipAddressStruct* CDNSQuery::GetIpAddress(WORD index)
{
	return &m_resolvedAddresses[index];
}

eIPProtocolType CDNSQuery::GetProtocolType()
{
	return m_protocolType;
}

eProcessType CDNSQuery::GetProcessType()
{
	return m_processType;
}

DWORD CDNSQuery::GetTransportType()
{
	return m_transportType;
}

WORD CDNSQuery::GetQueryState()
{
	return m_state;
}

enQueryType  CDNSQuery::GetQueryType()
{
	return m_queryType;
}

DWORD CDNSQuery::GetTtl()
{
	return m_ttl;
}

//No Resolve response was received
void  CDNSQuery::OnResolveToutRESOLVING(CSegment* pParam)
{
	PTRACE2(eLevelError, "CDNSQuery::OnResolveToutRESOLVING, timeout resolving ", m_strDomainName);

	//_M_S
	/*m_state = DNS_IDLE;
	m_timerResolve = 0;
	setAddressesToZero(m_resolvedAddresses);
	Retry();*/
	m_timerResolve = 0;
	Retry();
}

//No Resolve response was received
void  CDNSQuery::OnResolveToutVALID(CSegment* pParam)
{
	PTRACE2(eLevelError, "CDNSQuery::OnResolveToutVALID, timeout resolving ", m_strDomainName);
	m_state = DNS_IDLE;
	m_timerResolve = 0;
	setAddressesToZero(m_resolvedAddresses);
}

//Resolve response was received
void  CDNSQuery::OnResolveIndRESOLVING(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING,  ", m_strDomainName);
	m_timerResolve = 0;
	DeleteTimer(TIMER_RESOLVE_REQ);

	// read data
	WORD	boardId = 0, index = 0;
	DWORD	ttl = 0;
	DWORD   errorCode = 0;
	ALLOCBUFFER(hostName,  DnsQueryNameSize);
	ALLOCBUFFER(errReason, DnsErrDescSize);

	*pParams 	>> index
		    	>> hostName
				>> errorCode
				>> errReason
				>> ttl;

	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];

	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}

	pParams->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memcpy(&m_resolvedAddresses[i], &pAddrList[i], sizeof(ipAddressStruct));
	}

	DNS_PARAMS_IP_S pParam;
	strncpy(pParam.domainName,m_strDomainName,sizeof(pParam.domainName) - 1);
	pParam.domainName[sizeof(pParam.domainName) - 1] = '\0';
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memcpy(&pParam.pAddrList[i], &m_resolvedAddresses[i], sizeof(ipAddressStruct));
	}

	pParam.ServiceId=m_serviceID;

	CSegment* pSeg=new CSegment();
	pSeg->Put((BYTE*)&pParam, sizeof(DNS_PARAMS_IP_S));
	const COsQueue* pCsMbx =
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);
	PTRACE(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING, sending resolvedIp to CSMngr.");
//	STATUS res = pCsMbx->Send(pSeg,CS_DNS_AGENT_RESOLVE_IND);
	STATUS res = pCsMbx->Send(pSeg,CS_DNS_AGENT_RESOLVE_IND);

	char cardIpV4[16];
	char cardIpV6[128];
	char traceStr[300];

	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{

		if(m_resolvedAddresses[i].ipVersion == eIpVersion4)
		{
			SystemDWORDToIpString(m_resolvedAddresses[i].addr.v4.ip, cardIpV4);
			sprintf(traceStr, "Address number %d, %s is on %s, ttl = %d", i, hostName, cardIpV4, ttl);
			PTRACE2(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING:", traceStr);

		}
		else if(m_resolvedAddresses[i].ipVersion == eIpVersion6)
		{
			ipV6ToString(m_resolvedAddresses[i].addr.v6.ip, cardIpV6, TRUE);
			sprintf(traceStr, "Address number %d, %s is on %s, ttl = %d", i, hostName, cardIpV6, ttl);
			PTRACE2(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING:", traceStr);
		}
	}

	//Update DNS manager with status
	CSegment*  pSeg1 = new CSegment();
	*pSeg1 << errorCode;
	m_pDNSMngrRcvMbx->Send(pSeg1, DNS_QUERY_STATUS_UPDATE);

	CSegment*  pRetParam = new CSegment();
	*pRetParam << m_serviceID
			   << m_strDomainName;

	pRetParam->Put((BYTE*)&m_resolvedAddresses, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

    eOtherProcessQueueEntry queueType = eManager;

    if(m_processType == eProcessConfParty)
    	queueType = eDispatcher;

    const COsQueue* pProccessMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(m_processType, queueType);

	//if failed
	if(STATUS_OK != errorCode)
	{
		m_state = DNS_IDLE;
		PTRACE2(eLevelError, "CDNSQuery::OnResolveIndRESOLVING, errReason:", errReason);
		for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
		{
			memset(&m_resolvedAddresses[i], 0, sizeof(ipAddressStruct));
		}

		if(m_processType == (WORD)eProcessConfParty)
		{
			PTRACE(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING, sending resolve failed indication to party.");
			res = m_pDNSMngrRcvMbx->Send(pRetParam, DNS_RESOLVE_IND);
		}
		else if(m_processType == (WORD)eProcessSipProxy)
 		{
 		    CSipProxyTaskApi api(m_serviceID);
 		   STATUS res = api.SendMsg(pRetParam, DNS_RESOLVE_IND);
 		}
		else if (m_processType == (WORD)eProcessGatekeeper)
		{
			CGatekeeperTaskApi api(m_serviceID);
			STATUS res = api.SendMsg(pRetParam, DNS_RESOLVE_IND);
		}
		else // if not eProcessConfParty
		{
			PTRACE(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING, sending resolve failed indication.");
			res = pProccessMbx->Send(pRetParam,DNS_RESOLVE_IND);//NEED TO CHECK IF DNS_RESOLVE_IND

		}
	}
	else
	{
		if(!strncmp(hostName, m_strDomainName, DnsQueryNameSize))
		{
			if(ttl!=0 && ResolveTout < ttl)
			{
				ttl = ttl - ResolveTout;
				m_state = DNS_VALID;
				// start refresh resolution timer
				if(!m_timerRefresh)
				{
					StartTimer(TIMER_REFRESH_TOUT, ttl * SECOND);
					m_timerRefresh = 1;
				}

				if(m_processType == (WORD)eProcessConfParty)
				{
					PTRACE(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING, sending resolve indication to party.");
					res = m_pDNSMngrRcvMbx->Send(pRetParam, DNS_RESOLVE_IND);
				}
				else if(m_processType == eProcessSipProxy)
		 		{
					PTRACE(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING, sending resolve indication to SipProxy.");
		 		    CSipProxyTaskApi api(m_serviceID);
		 		    STATUS res = api.SendMsg(pRetParam, DNS_RESOLVE_IND);

		 		}
				else if (m_processType == (WORD)eProcessGatekeeper)
				{
					CGatekeeperTaskApi api(m_serviceID);
					STATUS res = api.SendMsg(pRetParam, DNS_RESOLVE_IND);
		 		}
				else // if not eProcessConfParty
				{
					PTRACE(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING, sending resolve indication.");
					res = pProccessMbx->Send(pRetParam,DNS_RESOLVE_IND);//NEED TO CHECK IF DNS_RESOLVE_IND
				}
			}
		}
		else
			DBGPASSERT(1);
	}
	DEALLOCBUFFER(hostName);
	DEALLOCBUFFER(errReason);
}

/////////////////////////////////////////////////////////////////////////////
//Resolve response was received
void  CDNSQuery::OnResolveIndVALID(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal, "CDNSQuery::OnResolveIndVALID,  ", m_strDomainName);
	m_timerResolve = 0;
	DeleteTimer(TIMER_RESOLVE_REQ);

	// read data
	WORD	boardId = 0, index = 0;
	DWORD	ttl = 0, resolvedIp=0;
	DWORD   errorCode = 0;
	ALLOCBUFFER(hostName, DnsQueryNameSize);
	ALLOCBUFFER(errReason, DnsErrDescSize);

	*pParams >> index
			>> hostName
			>> errorCode
			>> errReason
			>> ttl;

	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];

	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}

	pParams->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memcpy(&m_resolvedAddresses[i], &pAddrList[i], sizeof(ipAddressStruct));
	}

	DNS_PARAMS_IP_S pParam;
	strncpy(pParam.domainName,m_strDomainName,sizeof(pParam.domainName) - 1);
	pParam.domainName[sizeof(pParam.domainName) - 1] = '\0';

	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memcpy(&pParam.pAddrList[i], &m_resolvedAddresses[i], sizeof(ipAddressStruct));
	}

	pParam.ServiceId=m_serviceID;

	CSegment* pSeg1=new CSegment();
	pSeg1->Put((BYTE*)&pParam, sizeof(DNS_PARAMS_IP_S));
	const COsQueue* pCsMbx =
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);
	PTRACE(eLevelInfoNormal, "CDNSQuery::OnResolveIndVALID, sending resolvedIp to CSMngr.");
	STATUS res = pCsMbx->Send(pSeg1,CS_DNS_AGENT_RESOLVE_IND);

	char cardIpV4[16];
	char cardIpV6[128];
	char traceStr[300];

	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{

		if(m_resolvedAddresses[i].ipVersion == eIpVersion4)
		{
			SystemDWORDToIpString(m_resolvedAddresses[i].addr.v4.ip, cardIpV4);
			snprintf(traceStr, 300, "address number %d, %s is on %s, ttl = %d", i, hostName, cardIpV4, ttl);
			PTRACE2(eLevelInfoNormal, "CDNSQuery::OnResolveIndVALID:", traceStr);

		}
		else if(m_resolvedAddresses[i].ipVersion == eIpVersion6)
		{
			ipV6ToString(m_resolvedAddresses[i].addr.v6.ip, cardIpV6, TRUE);
			snprintf(traceStr, 300, "address number %d, %s is on %s, ttl = %d", i, hostName, cardIpV6, ttl);
			PTRACE2(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING:", traceStr);

		}

	}

	//Update DNS manager with status
	CSegment*  pSeg = new CSegment();
	*pSeg << errorCode;
	m_pDNSMngrRcvMbx->Send(pSeg, DNS_QUERY_STATUS_UPDATE);

	if(STATUS_OK != errorCode)
	{
		m_state = DNS_IDLE;

		for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
		{
			memset(&m_resolvedAddresses[i], 0, sizeof(ipAddressStruct));
		}

		CSegment*  pRetParam = new CSegment();
		*pRetParam << m_serviceID
				   << m_strDomainName
				   << m_connId
    		   	   << m_partyId;

       pRetParam->Put((BYTE*)&m_resolvedAddresses, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

		if(m_processType == eProcessSipProxy)
		{
		    CSipProxyTaskApi api(m_serviceID);
		   STATUS res = api.SendMsg(pRetParam, DNS_RESOLVE_IND);
		}
		else if (m_processType == (WORD)eProcessGatekeeper)
		{
			CGatekeeperTaskApi api(m_serviceID);
			STATUS res = api.SendMsg(pRetParam, DNS_RESOLVE_IND);
		}
		else
		{
			eOtherProcessQueueEntry queueType = eManager;
			if(m_processType == eProcessConfParty)
				queueType = eDispatcher;

			const COsQueue* pProccessMbx =
				CProcessBase::GetProcess()->GetOtherProcessQueue(m_processType, queueType);
			PTRACE2(eLevelError, "CDNSQuery::OnResolveIndVALID, errReason:", errReason);
			PTRACE(eLevelInfoNormal, "CDNSQuery::OnResolveIndVALID, sending resolve error indication.");
			STATUS res = pProccessMbx->Send(pRetParam,DNS_RESOLVE_IND);//NEED TO CHECK IF DNS_DOMAIN_IND
		}
	}
	else
	{
		if(strncmp(hostName, m_strDomainName, DnsQueryNameSize) == 0)
		{

			if(ttl!=0 && ResolveTout < ttl)
			{
				ttl = ttl - ResolveTout;
				m_state = DNS_VALID;
				// start refresh resolution timer
				if(!m_timerRefresh)
				{
					StartTimer(TIMER_REFRESH_TOUT, ttl * SECOND);
					m_timerRefresh = 1;
				}

				CSegment*  pRetParam = new CSegment();
					*pRetParam << m_serviceID
							   << m_strDomainName;

				pRetParam->Put((BYTE*)&m_resolvedAddresses, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

				if(m_processType == eProcessSipProxy)
				{
					PTRACE(eLevelInfoNormal, "CDNSQuery::OnResolveIndVALID, sending resolve indication to SipProxy.");
					CSipProxyTaskApi api(m_serviceID);
					STATUS res = api.SendMsg(pRetParam, DNS_RESOLVE_IND);
				}
			}
		}
		else
			DBGPASSERT(1);
	}
	DEALLOCBUFFER(hostName);
	DEALLOCBUFFER(errReason);
}

/////////////////////////////////////////////////////////////////////////////
//TTL over, resolve again
void  CDNSQuery::OnRefreshTimerVALID(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CDNSQuery::OnRefreshTimerVALID, resolving: ", m_strDomainName);

	DNS_CS_RESOLVE_REQ_ST dnsCsResolveReq;

	m_state = DNS_RESOLVING;

	dnsCsResolveReq.transactionId = m_index;
	memcpy(&dnsCsResolveReq.hostName[0], m_strDomainName, DnsQueryNameSize);

	SendToCsApi(DNS_CS_RESOLVE_REQ, sizeof(DNS_CS_RESOLVE_REQ_ST), (char *) &dnsCsResolveReq);
}

/////////////////////////////////////////////////////////////////////////////
//Try to resolve again
void CDNSQuery::Retry()
{
	PTRACE2(eLevelInfoNormal, "CDNSQuery::Retry, resolving: ", m_strDomainName);
	m_state = DNS_RESOLVING;

	DNS_CS_RESOLVE_REQ_ST dnsCsResolveReq;

	m_state = DNS_RESOLVING;

	dnsCsResolveReq.transactionId = m_index;
	memcpy(&dnsCsResolveReq.hostName[0], m_strDomainName, DnsQueryNameSize);

	SendToCsApi(DNS_CS_RESOLVE_REQ, sizeof(DNS_CS_RESOLVE_REQ_ST), (char *) &dnsCsResolveReq);
}
/////////////////////////////////////////////////////////////////////////////
void CDNSQuery::setAddressesToZero(ipAddressStruct* resolvedAddresses)
{
	for(int i=0; i<TOTAL_NUM_OF_IP_ADDRESSES; i++)
	{
		memset(&resolvedAddresses[i], 0, sizeof(ipAddressStruct));
	}
}

/////////////////////////////////////////////////////////////////////////////

STATUS CDNSQuery::SendToCsApi(OPCODE opcode, const int dataLen, const char *data)
{
	CMplMcmsProtocol	mplProt;
	mplProt.AddCommonHeader(opcode, 0, 0, 0, eCentral_signaling);

	mplProt.AddMessageDescriptionHeader();
	mplProt.AddCSHeader(m_serviceID,0,eServiceMngr);
	mplProt.AddData(dataLen, data);
	mplProt.AddPayload_len(CS_API_TYPE);
	PTRACE2INT(eLevelInfoNormal, "CDNSQuery::SendToCsApi, sending request to cs with opcode : " ,opcode);
	STATUS res = mplProt.SendMsgToCSApiCommandDispatcher();
	if(res)
	{
		PTRACE(eLevelError, "CDNSQuery::SendToCsApi, failed to send request to cs.");
	}

	if (!m_timerResolve) {

		StartTimer(TIMER_RESOLVE_REQ, 10 * SECOND);
		m_timerResolve = 1;
 	}

	return res;
}

PBEGIN_MESSAGE_MAP(CDNSDummyQuery)
  ONEVENT(DNS_SERVICE_IND,				DNS_RESOLVING,			CDNSDummyQuery::OnResolveIndRESOLVING		)
  ONEVENT(DNS_SERVICE_IND,				DNS_VALID,				CDNSDummyQuery::OnResolveIndVALID			)
PEND_MESSAGE_MAP(CDNSDummyQuery,CDNSQuery);

void  CDNSDummyQuery::OnResolveIndRESOLVING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CDNSDummyQuery::OnResolveIndRESOLVING,  ", m_strDomainName);
	m_timerResolve = 0;
	DeleteTimer(TIMER_RESOLVE_REQ);

	// read data
	WORD	index = 0;
	DWORD	ttl = 0, resolvedIp=0;
	DWORD   errorCode = 0;
	ALLOCBUFFER(hostName, DnsQueryNameSize);

	*pParam >> index
		    >> hostName
			>> errorCode
	//		>> resolvedIp
			>> ttl;


	//Update DNS manager with status
	CSegment*  pSeg = new CSegment();
	*pSeg << errorCode;
	m_pDNSMngrRcvMbx->Send(pSeg, DNS_QUERY_STATUS_UPDATE);

	DEALLOCBUFFER(hostName);

	m_state = DNS_VALID;
	// start refresh resolution timer
	if(!m_timerRefresh)
	{
		StartTimer(TIMER_REFRESH_TOUT, 5*60* SECOND);
		m_timerRefresh = 1;
	}
}


////////////////////////////////////////////////////////////////////////////
void  CDNSDummyQuery::OnResolveIndVALID(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CDNSDummyQuery::OnResolveIndVALID,  ", m_strDomainName);
	m_timerResolve = 0;
	DeleteTimer(TIMER_RESOLVE_REQ);

	// read data
	WORD	index = 0;
	DWORD	ttl = 0, resolvedIp=0;
	DWORD   errorCode = 0;
	ALLOCBUFFER(hostName, DnsQueryNameSize);

	*pParam >> index
		    >> hostName
			>> errorCode
			>> ttl;


	//Update DNS manager with status
	CSegment*  pSeg = new CSegment();
	*pSeg << errorCode;
	m_pDNSMngrRcvMbx->Send(pSeg, DNS_QUERY_STATUS_UPDATE);

	DEALLOCBUFFER(hostName);

	// start refresh resolution timer
	if(!m_timerRefresh)
	{
		StartTimer(TIMER_REFRESH_TOUT, 5*60* SECOND);
		m_timerRefresh = 1;
	}
}

PBEGIN_MESSAGE_MAP(CDNSServiceQuery)
  ONEVENT(TIMER_REFRESH_TOUT,			DNS_VALID,				CDNSServiceQuery::OnRefreshTimerVALID		)
  ONEVENT(DNS_SERVICE_IND,				DNS_RESOLVING,			CDNSServiceQuery::OnResolveIndRESOLVING		)
  ONEVENT(DNS_SERVICE_IND,				DNS_VALID,				CDNSServiceQuery::OnResolveIndVALID			)
  ONEVENT(TIMER_RESOLVE_REQ,			DNS_RESOLVING,			CDNSServiceQuery::OnResolveToutRESOLVING 	)
PEND_MESSAGE_MAP(CDNSServiceQuery,CDNSQuery);

CDNSServiceQuery::CDNSServiceQuery()
{
	m_queryType = eServiceQuery;
	VALIDATEMESSAGEMAP;
}

CDNSServiceQuery::~CDNSServiceQuery()
{
}

int  CDNSServiceQuery::Create(WORD serviceID, WORD index,  char* pDomainName, COsQueue* pRcvMbx
					   , eIPProtocolType protocolType, DWORD transportType, eProcessType processType,
					   DWORD connId, DWORD  partyId)
{
	char request[DnsQueryNameSize];

	DNS_CS_SERVICE_REQ_ST dnsCsServiceReq;

	PTRACE2(eLevelInfoNormal, "CDNSServiceQuery::Create,  domain name: ", pDomainName);

	dnsCsServiceReq.transactionId = index;
	dnsCsServiceReq.transportType = transportType;

	strncpy(&dnsCsServiceReq.domainName[0], pDomainName, DnsQueryNameSize);

	m_serviceID = serviceID;
	m_index = index;
	strncpy(m_strDomainName, pDomainName, DnsQueryNameSize);
	m_protocolType  = protocolType;
	m_transportType = eTransportTypeTcp;
	m_processType = processType;
	m_connId = connId;
	m_partyId = partyId;
	m_pDNSMngrRcvMbx = pRcvMbx;
	m_state = DNS_RESOLVING;

	SendToCsApi(DNS_CS_SERVICE_REQ, sizeof(DNS_CS_SERVICE_REQ_ST), (char *) &dnsCsServiceReq);
	return 0;    // OK
}

//TTL over, resolve again
void  CDNSServiceQuery::OnRefreshTimerVALID(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CDNSServiceQuery::OnRefreshTimerVALID, resolving: ", m_strDomainName);

	DNS_CS_SERVICE_REQ_ST dnsCsServiceReq;

	PTRACE2(eLevelInfoNormal, "CDNSServiceQuery::Retry, resolving: ", m_strDomainName);

	m_state = DNS_RESOLVING;

	dnsCsServiceReq.transactionId = m_index;
	dnsCsServiceReq.transportType = m_transportType;

	memcpy(&dnsCsServiceReq.domainName[0], m_strDomainName, DnsQueryNameSize);

	SendToCsApi(DNS_CS_SERVICE_REQ, sizeof(DNS_CS_SERVICE_REQ_ST), (char *) &dnsCsServiceReq);
}

/////////////////////////////////////////////////////////////////////////////
//No Resolve response was received
void  CDNSServiceQuery::OnResolveToutRESOLVING(CSegment* pParam)
{
	PTRACE2(eLevelError, "CDNSServiceQuery::OnResolveToutRESOLVING, timeout resolving ", m_strDomainName);
	PTRACE2INT(eLevelError, "CDNSServiceQuery::OnResolveToutRESOLVING, m_transportType = ", m_transportType);
	if(eTransportTypeTcp == m_transportType)
	{
		m_transportType = eTransportTypeUdp;
		Retry();
	}
	else
	{
		m_state = DNS_IDLE;
		m_timerResolve = 0;
//		m_resolvedIp = 0;
//		memset(&m_resolvedAddress, 0, sizeof(mcTransportAddress));
		for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
		{
			memset(&m_resolvedAddresses[i], 0, sizeof(ipAddressStruct));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//Resolve response was received
void  CDNSServiceQuery::OnResolveIndRESOLVING(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndRESOLVING,  ", m_strDomainName);

	m_timerResolve = 0;
	DeleteTimer(TIMER_RESOLVE_REQ);

	// read data
	WORD	boardId = 0, index = 0, priority = 0, weight = 0;
	DWORD	ttl = 0, resolvedIp=0;
	STATUS  status=STATUS_OK;
	ALLOCBUFFER(domainName, DnsQueryNameSize);
	ALLOCBUFFER(hostName, DnsQueryNameSize);
	ALLOCBUFFER(errReason, DnsErrDescSize);

	*pParams >> index
			>> domainName
			>> hostName
			>> status
			>> errReason
		//	>> resolvedIp
			>> ttl
			>> m_resolvedPort
			>> priority
			>> weight;

//	mcTransportAddress	resolvedAddr;
//	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
//	pParams->Get((BYTE *)&resolvedAddr, sizeof(mcTransportAddress));

	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}
	pParams->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

//	if(m_resolvedIp != resolvedIp)
//	if(0 != memcmp(&m_resolvedAddress, &resolvedAddr, sizeof(mcTransportAddress)))
//	{
		//m_resolvedIp=resolvedIp;
	//	memcpy(&m_resolvedAddress, &resolvedAddr, sizeof(mcTransportAddress));
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memcpy(&m_resolvedAddresses[i], &pAddrList[i], sizeof(ipAddressStruct));
	}

	DNS_PARAMS_IP_S pParam;
	strncpy(pParam.domainName,m_strDomainName,sizeof(pParam.domainName) - 1);
	pParam.domainName[sizeof(pParam.domainName) - 1] = '\0';
	//pParam.Ip=m_resolvedIp;
//	memcpy(&pParam.Ip, &m_resolvedAddress, sizeof(mcTransportAddress));
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memcpy(&pParam.pAddrList[i], &m_resolvedAddresses[i], sizeof(ipAddressStruct));
	}

	pParam.ServiceId=m_serviceID;

	CSegment* pSeg=new CSegment();
	pSeg->Put((BYTE*)&pParam, sizeof(DNS_PARAMS_IP_S));
	PTRACE(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndRESOLVING, sending resolvedIp to CSMngr.");
	const COsQueue* pCsMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

	STATUS res = pCsMbx->Send(pSeg,CS_DNS_AGENT_RESOLVE_IND);
//	}

//	char cardIp[16];
//	SystemDWORDToIpString(m_resolvedIp, cardIp);
//	char cardIp[128];

	char cardIpV4[16];
	char cardIpV6[128];
	ALLOCBUFFER(s, 300);
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{

		if(m_resolvedAddresses[i].ipVersion == eIpVersion4)
		{
			SystemDWORDToIpString(m_resolvedAddresses[i].addr.v4.ip, cardIpV4);
			sprintf(s, "addres number %d, %s is on %s, ttl = %d", i, hostName, cardIpV4, ttl);
			PTRACE2(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING:", s);

		}
		else if(m_resolvedAddresses[i].ipVersion == eIpVersion6)
		{
			ipV6ToString(m_resolvedAddresses[i].addr.v6.ip, cardIpV6, TRUE);
			sprintf(s, "addres number %d, %s is on %s, ttl = %d", i, hostName, cardIpV6, ttl);
			PTRACE2(eLevelInfoNormal, "CDNSQuery::OnResolveIndRESOLVING:", s);
		}
	}
	DEALLOCBUFFER(s);
	//Update DNS manager with status
	CSegment*  pSeg1 = new CSegment();
	*pSeg1 << status;
	m_pDNSMngrRcvMbx->Send(pSeg1, DNS_QUERY_STATUS_UPDATE);

	CSegment*  pRetParam = new CSegment();
	*pRetParam << m_serviceID
			<< domainName
		//	<<(DWORD)m_resolvedIp
			<< m_resolvedPort
			<< (DWORD) m_resolvedPort
			<< m_transportType
			<< m_connId
			<< m_partyId;

//	pRetParam->Put((BYTE*)&m_resolvedAddress, sizeof(mcTransportAddress));
	pRetParam->Put((BYTE*)&m_resolvedAddresses, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	eOtherProcessQueueEntry queueType = eManager;
    if(m_processType == eProcessConfParty)
    	queueType = eDispatcher;

    const COsQueue* pProcessMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(m_processType, queueType);

	if(STATUS_OK != status)
	{
		if(eTransportTypeTcp == m_transportType)
		{
			m_transportType = eTransportTypeUdp;
			Retry();
		}
		else
		{
			m_state = DNS_IDLE;
			//m_resolvedIp = 0;
			//memset(&m_resolvedAddress, 0, sizeof(mcTransportAddress));
			for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
			{
				memset(&m_resolvedAddresses[i], 0, sizeof(ipAddressStruct));
			}

			PTRACE(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndRESOLVING, sending resolve error indication.");
			PTRACE2(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndRESOLVING, errReason:", errReason);
	 		if(m_processType == eProcessSipProxy)
	 		{
	 		    CSipProxyTaskApi api(m_serviceID);
	 		   STATUS res = api.SendMsg(pRetParam, DNS_SERVICE_IND);
	 		}
	 		else
			   STATUS res = pProcessMbx->Send(pRetParam,DNS_SERVICE_IND);
			//retry next time with tcp
			m_transportType = eTransportTypeTcp;
		}
	}
	else
	{
		if(!strncmp(domainName, m_strDomainName, DnsQueryNameSize))
		{
			if(ttl!=0 && ResolveTout < ttl)
			{
				ttl = ttl - ResolveTout;
				m_state = DNS_VALID;
				// start refresh resolution timer
				if(!m_timerRefresh)
				{
					StartTimer(TIMER_REFRESH_TOUT, ttl * SECOND);
					m_timerRefresh = 1;
				}

		 		if(m_processType == eProcessSipProxy)
		 		{
		 		    CSipProxyTaskApi api(m_serviceID);
		 		   STATUS res = api.SendMsg(pRetParam, DNS_SERVICE_IND);
		 		  PTRACE(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndRESOLVING, sending resolve indication to SipProxy.");
		 		}
		 		else
		 		{
				    STATUS res = pProcessMbx->Send(pRetParam,DNS_SERVICE_IND);
				    PTRACE(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndRESOLVING, sending resolve indication to DNS service");
		 		}
			}
		}
		else
			DBGPASSERT(1);
	}
	DEALLOCBUFFER(domainName);
	DEALLOCBUFFER(hostName);
	DEALLOCBUFFER(errReason);

}

/////////////////////////////////////////////////////////////////////////////
//Resolve response was received
void  CDNSServiceQuery::OnResolveIndVALID(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndVALID,  ", m_strDomainName);
	m_timerResolve = 0;
	DeleteTimer(TIMER_RESOLVE_REQ);

	// read data
	WORD	boardId = 0, index = 0, priority = 0, weight = 0;
	DWORD	ttl = 0, resolvedIp=0;
	STATUS  status = STATUS_OK;
	ALLOCBUFFER(domainName, DnsQueryNameSize);
	ALLOCBUFFER(hostName, DnsQueryNameSize);
	ALLOCBUFFER(errReason, DnsErrDescSize);

	*pParams >> index
			>> domainName
			>> hostName
			>> status
			>> errReason
	//		>> resolvedIp
			>> ttl
			>> m_resolvedPort
			>> priority
			>> weight;

//	mcTransportAddress	resolvedAddr;
//	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
//	pParams->Get((BYTE *)&resolvedAddr, sizeof(mcTransportAddress));
	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}
	pParams->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	//if(m_resolvedIp != resolvedIp)
//	if(0 != memcmp(&m_resolvedAddress, &resolvedAddr, sizeof(mcTransportAddress)))
//	{
		//m_resolvedIp=resolvedIp;
	//	memcpy(&m_resolvedAddress, &resolvedAddr, sizeof(mcTransportAddress));
		for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
		{
			memcpy(&m_resolvedAddresses[i], &pAddrList[i], sizeof(ipAddressStruct));
		}

		DNS_PARAMS_IP_S pParam;
		strncpy(pParam.domainName,m_strDomainName, sizeof(pParam.domainName) - 1);
		pParam.domainName[sizeof(pParam.domainName) - 1] = '\0';
		//pParam.Ip=m_resolvedIp;
		//memcpy(&pParam.Ip, &m_resolvedAddress, sizeof(mcTransportAddress));
		for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
		{
			memcpy(&pParam.pAddrList[i], &m_resolvedAddresses[i], sizeof(ipAddressStruct));
		}

		pParam.ServiceId=m_serviceID;

		CSegment* pSeg=new CSegment();
		pSeg->Put((BYTE*)&pParam, sizeof(DNS_PARAMS_IP_S));
		PTRACE(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndVALID, sending resolvedIp to CSMngr.");
		const COsQueue* pCsMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

		STATUS res = pCsMbx->Send(pSeg,CS_DNS_AGENT_RESOLVE_IND);
//	}

//	char cardIp[16];
//	SystemDWORDToIpString(m_resolvedIp, cardIp);
//	char cardIp[128];


	char cardIpV4[16];
	char cardIpV6[128];
	ALLOCBUFFER(s, 300);
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{

		if(m_resolvedAddresses[i].ipVersion == eIpVersion4)
		{
			SystemDWORDToIpString(m_resolvedAddresses[i].addr.v4.ip, cardIpV4);
			sprintf(s, "addres number %d, %s is on %s, ttl = %d", i, hostName, cardIpV4, ttl);
			PTRACE2(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndVALID:", s);
		}
		else if(m_resolvedAddresses[i].ipVersion == eIpVersion6)
		{
			ipV6ToString(m_resolvedAddresses[i].addr.v6.ip, cardIpV6, TRUE);
			sprintf(s, "addres number %d, %s is on %s, ttl = %d", i, hostName, cardIpV6, ttl);
			PTRACE2(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndVALID:", s);
		}
	}
	DEALLOCBUFFER(s);

	//Update DNS manager with status
	CSegment*  pSeg1 = new CSegment();
	*pSeg1 << status;
	m_pDNSMngrRcvMbx->Send(pSeg1, DNS_QUERY_STATUS_UPDATE);

	if(STATUS_OK != status)
	{
		m_state = DNS_IDLE;
	//	m_resolvedIp = 0;
		//memset(&m_resolvedAddress, 0, sizeof(mcTransportAddress));
		for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
		{
			memset(&m_resolvedAddresses[i], 0, sizeof(ipAddressStruct));
		}

		CSegment*  pRetParam = new CSegment();
		*pRetParam << m_serviceID
				<< domainName
			//	<<(DWORD)m_resolvedIp
				<< m_resolvedPort
				<< m_transportType
				<< m_connId
				<< m_partyId;

		//pRetParam->Put((BYTE*)&m_resolvedAddress, sizeof(mcTransportAddress));
		pRetParam->Put((BYTE*)&m_resolvedAddresses, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

		PTRACE(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndVALID, sending resolve error indication.");
		PTRACE2(eLevelInfoNormal, "CDNSServiceQuery::OnResolveIndVALID, errReason:", errReason);

		eOtherProcessQueueEntry queueType = eManager;
    	if(m_processType == eProcessConfParty)
    		queueType = eDispatcher;

   		const COsQueue* pProcessMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(m_processType, queueType);

 		if(m_processType == eProcessSipProxy)
 		{
 		    CSipProxyTaskApi api(m_serviceID);
 		   STATUS res = api.SendMsg(pRetParam, DNS_SERVICE_IND);
 		}
 		else
 			STATUS res = pProcessMbx->Send(pRetParam,DNS_SERVICE_IND);
	}
	else
	{
		if(!strncmp(domainName, m_strDomainName, DnsQueryNameSize))
		{
			if(ttl!=0 && ResolveTout < ttl)
			{
				ttl = ttl - ResolveTout;
				m_state = DNS_VALID;
				// start refresh resolution timer
				if(!m_timerRefresh)
				{
					StartTimer(TIMER_REFRESH_TOUT, ttl * SECOND);
					m_timerRefresh = 1;
				}
			}
		}
		else
			DBGPASSERT(1);
	}
	DEALLOCBUFFER(domainName);
	DEALLOCBUFFER(hostName);
	DEALLOCBUFFER(errReason);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CDNSServiceQuery::Retry()
{
	DNS_CS_SERVICE_REQ_ST dnsCsServiceReq;

	PTRACE2(eLevelInfoNormal, "CDNSServiceQuery::Retry, resolving: ", m_strDomainName);
	m_state = DNS_RESOLVING;

	dnsCsServiceReq.transactionId = m_index;
	dnsCsServiceReq.transportType = m_transportType;

	memcpy(&dnsCsServiceReq.domainName[0], m_strDomainName, DnsQueryNameSize);

	SendToCsApi(DNS_CS_SERVICE_REQ, sizeof(DNS_CS_SERVICE_REQ_ST), (char *) &dnsCsServiceReq);
	dnsCsServiceReq.transactionId = m_index;
	dnsCsServiceReq.transportType = m_transportType;

	memcpy(&dnsCsServiceReq.domainName[0], m_strDomainName, DnsQueryNameSize);
	SendToCsApi(DNS_CS_SERVICE_REQ, sizeof(DNS_CS_SERVICE_REQ_ST), (char *) &dnsCsServiceReq);
}

extern void DNSAgentMonitorEntryPoint(void* appParam);
void DNSAgentManagerEntryPoint(void* appParam)
{
	CDNSAgentManager * pDNSAgentManager = new CDNSAgentManager();
	pDNSAgentManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CDNSAgentManager::GetMonitorEntryPoint()
{
	return DNSAgentMonitorEntryPoint;
}

// DNS Manager states
const WORD	 DISCONNECTED	= 0;
const WORD	 SET_UP			= 1;
const WORD   CONNECT        = 2;

// temporary as a sample
const WORD   DUMMY          = 1;

PBEGIN_MESSAGE_MAP(CDNSAgentManager)
	ONEVENT(CSAPI_MSG				,ANYCASE 	,CDNSAgentManager::OnCSApi_Msg)
	ONEVENT(DNSAGENT_MCUMNGR_CONFIGURATION_IND	,ANYCASE		,CDNSAgentManager::OnMcuMngrConfigurationStatusInd )
	//ONEVENT(DNS_RESOLVE_DOMAIN_REQ	,CONNECT    ,CDNSAgentManager::OnResolveDomainReq)
	//ONEVENT(DNS_SERVICE_REQ			,CONNECT    ,CDNSAgentManager::OnServiceReq		)
	//ONEVENT(DNS_RESOLVE_IND			,CONNECT    ,CDNSAgentManager::OnResolveInd		)
	ONEVENT(DNS_SERVICE_IND			,CONNECT    ,CDNSAgentManager::OnServiceInd		)
	ONEVENT(DNS_CLEAR_ALL			,CONNECT    ,CDNSAgentManager::OnClearQueries	)
	ONEVENT(CS_DNS_AGENT_DELETE_IP_SERVICE_IND	,CONNECT    ,CDNSAgentManager::RemoveServiceIdFromQueries	)
	ONEVENT(DNS_QUERY_STATUS_UPDATE	,CONNECT	,CDNSAgentManager::OnStatusUpdate	)
	//ONEVENT(DNS_CS_SERVICE_IND		,CONNECT	,CDNSAgentManager::OnServiceInd)//FROM CS
	//-S- PLCM_DNS ------------------------------//
	ONEVENT(DNS_RESOLVE_DOMAIN_REQ	       ,ANYCASE, CDNSAgentManager::OnResolveDomainReq)
	ONEVENT(DNS_SERVICE_REQ			       ,ANYCASE, CDNSAgentManager::OnServiceReq		 )
	ONEVENT(DNS_REQ_RECV			       ,ANYCASE, CDNSAgentManager::OnRecvDnsResponse )
	ONEVENT(DNSAGENT_TO_CSMNGR_GET_IPCONFIG,ANYCASE, CDNSAgentManager::OnCsMngrDnsConfig )
	//ONEVENT(DNS_RESOLVE_IND			       ,ANYCASE, CDNSAgentManager::OnResolveInd		 )//FROM CS
	ONEVENT(DNS_CS_RESOLVE_IND		       ,ANYCASE, CDNSAgentManager::OnResolveInd      )//From CS
	ONEVENT(CSMNGR_TO_DNSAGENT_CS_CONFIGURED,ANYCASE, CDNSAgentManager::OnCsMngrCsConfig ) // From CsMngr
	//-E- PLCM_DNS ------------------------------//
PEND_MESSAGE_MAP(CDNSAgentManager,CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CDNSAgentManager)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CDNSAgentManager)
  ONCOMMAND("dump_dns_agent",CDNSAgentManager::HandleTerminalDumpAgent,"dump DNS agent data")
  ONCOMMAND("dns_records",CDNSAgentManager::HandleTerminalDumpRecords,"DNS records available commands: [help | list | add | clean_by_name]")
  ONCOMMAND("dns_resolve",CDNSAgentManager::HandleTerminalDnsResolve ,"DNS Resolve.\nCommand format:\n  Bin/McuCmd dns_resolve DNSAgent <Serv.ID> <HOST-NAME> \n   Example: Bin/McuCmd dns_resolve DNSAgent 1 www.google.com")
  ONCOMMAND("dns_conf",CDNSAgentManager::HandleTerminalDnsConf    ,"DNS Configure.\nCommand format:\n  Bin/McuCmd dns_conf DNSAgent <Serv.ID> -n<DNS-Srver 1> -4Y -6N -n<DNS-Srver 2> -d<domain 1> -d<domain 2> -t<timeout in sec> -a<attempts> -s<source.IP>\
\n   Example: Bin/McuCmd dns_conf DNSAgent 1 -4Y -6N -n8.8.8.8 -dpolycom.com -t5 -a1 -s10.227.2.151 -Sabcd:10:226:113::2");
END_TERMINAL_COMMANDS
//-S- PLCM_DNS ------------------------------//
//=============================================================================//
void CDNSAgentManager::OnRecvDnsResponse(CSegment* pParam) 
{ 
	cMsgDnsRECV msgDnsResponse; 
	msgDnsResponse.DeSerialize(NATIVE, *pParam);

	msgDnsResponse.DumpR("CDNSAgentManager::OnRecvDnsResponse. PLCM_DNS. DNS Response:");

	cPLCM_DNS * pPlcmDns = G_GetPlsmDnsPtr();
	if(NULL != pPlcmDns)
		pPlcmDns->bAnalysisAndProcessing(
			  msgDnsResponse.m_sMsgData.m_aResoledAddrrAndTTL
			, msgDnsResponse.m_sMsgData.m_nResolved
			, msgDnsResponse.m_sMsgData.m_szHostNameComplex
			, msgDnsResponse.m_sMsgData.m_ReqId
			, msgDnsResponse.m_sMsgData.m_wReqType
			, msgDnsResponse.m_sMsgData.m_bIsResponseArrived );
}
//=============================================================================//
//-E- PLCM_DNS ------------------------------//

CDNSAgentManager::CDNSAgentManager()
{
	m_state = DISCONNECTED;
	for(int i=0;i<MAX_QUERIES;i++)
		m_pDNSQueryEnteties[i]=NULL;
}

CDNSAgentManager::~CDNSAgentManager()
{
	OnClearQueries();
}

void CDNSAgentManager::ManagerPostInitActionsPoint()
{
	m_state = SET_UP;

	CManagerApi mngrApi(eProcessCSMngr);
	mngrApi.SendOpcodeMsg(DNSAGENT_TO_CSMNGR_GET_IPCONFIG);

	//-S- PLCM_DNS ------------------------------//
	if(ePLCM_DNS_STATE_IDLE == G_PlcmDns.m_eObjState)
	{
		BOOL bRcDnsObj = G_PlcmDns.bActivate(this->m_pRcvMbx);
		if(TRUE == bRcDnsObj)
			PTRACE(eLevelInfoNormal,"PLCM_DNS. G_PlcmDns has STARTED. (Successfully)");
		else
			PTRACE(eLevelInfoNormal,"PLCM_DNS. G_PlcmDns has NOT Started (UN-Successful)!!!");

	}
	//-E- PLCM_DNS ------------------------------//
}

void  CDNSAgentManager::Dump(COstrStream& msg)const
{
	int i = 0, non = 1;

	if(msg){
		msg << "CDNSAgentManager::Dump\n";
		msg << "----------------------------------------------------------------------------------------------------------------\n";
		msg << "\tService\t |     Type     \t | Domain\t\t | ResolvedIp\t | ResolvedPort | Transport | Status\n";
		msg << "----------------------------------------------------------------------------------------------------------------\n";
		for(int i = 0; i<MAX_QUERIES; i++)
		{
			if(IsValidPObjectPtr(m_pDNSQueryEnteties[i]))
			{
				non = 0;
				msg << i << " :\t " << m_pDNSQueryEnteties[i]->GetServiceName() << "\t | ";
				if(!strncmp("CDNSQuery", m_pDNSQueryEnteties[i]->NameOf(), 10))
					msg << "CDNSQuery\t | ";
				else if (!strncmp(m_pDNSQueryEnteties[i]->NameOf(), "CDNSServiceQuery", 17))
					msg << "CDNSServiceQuery\t | ";
				else if (!strncmp(m_pDNSQueryEnteties[i]->NameOf(), "CDNSPartyQuery", 15))
					msg << "CDNSPartyQuery\t | ";
				else if (!strncmp(m_pDNSQueryEnteties[i]->NameOf(), "CDNSPartySrvQuery", 18))
					msg << "CDNSPartySrvQuery\t | ";
				else
					msg << "CDNSGkQuery\t | ";

				msg << m_pDNSQueryEnteties[i]->GetDomainName() << "\t | ";
	            for(int j=0; j<TOTAL_NUM_OF_IP_ADDRESSES; j++)
	            {
	            	ipAddressStruct* ipAddrSt = m_pDNSQueryEnteties[i]->GetIpAddress(j);
					if(ipAddrSt->ipVersion == eIpVersion4)
					{
						char proxyIpV4[16];
						SystemDWORDToIpString(ipAddrSt->addr.v4.ip, proxyIpV4);
						msg << "ProxyIpV4 = " << proxyIpV4 << " | ";
					}
					else if(ipAddrSt->ipVersion == eIpVersion6)
					{
						char proxyIpV6[128];
						ipV6ToString(ipAddrSt->addr.v6.ip, proxyIpV6, TRUE);
						msg << "ProxyIpV6 = " << proxyIpV6 << " | ";
					}
	            }

				if (!strncmp(m_pDNSQueryEnteties[i]->NameOf(), "CDNSServiceQuery", 17))
				{
					//msg << m_pDNSQueryEnteties[i]->GetPort() << "\t\t | "; //rons
					if(eTransportTypeUdp == m_pDNSQueryEnteties[i]->GetTransportType())
						msg << "UDP \t\t |";
					else
						msg << "TCP \t\t |";
				}
				else
					msg << "\t\t | \t\t | ";

				switch(m_pDNSQueryEnteties[i]->GetQueryState())
				{
					case(DNS_IDLE):
					{
						msg << "Idle \t |\n";
						break;
					}
					case(DNS_RESOLVING):
					{
						msg << "Resolving \t |\n";
						break;
					}
					case(DNS_VALID):
					{
						msg << "Valid \t |\n";
						break;
					}
					default:
					{
						msg << m_pDNSQueryEnteties[i]->GetQueryState() << " \t |\n";
						break;
					}
				}
			}
		}

		if(non == 1)
			msg<<"No Resolutions exist.\n";

		PTRACE(eLevelInfoNormal,msg.str().c_str());
	}
}

void CDNSAgentManager::Create(CSegment& appParam)
{
	PTRACE(eLevelInfoNormal, "CDNSAgentManager::Create");
	CTaskApp::Create(appParam/*,"DNSM"*/);
}

void  CDNSAgentManager::HandleEvent(CSegment* pMsg)
{
	DWORD  msgLen;
	WORD   opCode;

	*pMsg >> msgLen;
	*pMsg >> opCode;
	switch(opCode)
	{
		default:
		{
			DispatchEvent( opCode, pMsg );
		}
	}
}

void	CDNSAgentManager::OnResolveDomainReq(CSegment* pParam)
{
	WORD serviceID;
	DWORD connId = 0, partyId = 0;
	WORD  temp;
	eProcessType processType;
	char szDomainName[256]="";
	memset(szDomainName, '\0', sizeof(szDomainName));

	//ALLOCBUFFER(pDomainName, DnsQueryNameSize);
	int indInDB = -1;
	*pParam >> serviceID
			>> szDomainName//pDomainName
			>> temp;

	processType = (eProcessType)temp;
	if (processType == eProcessConfParty)
	{// get the partyId
		*pParam >> partyId;
	}


	//-S- PLCM_DNS ------------------------------//
	//for DEBUG!!!--------------------------------------------------------//
			//TRACEINTO << "PLCM_DNS. DNS_RESOLVE_DOMAIN_REQ (TEST): Host:[" << "www.cnn.com"
			//<< "] ServiceID[" << 2 
			//	<< "] ProccessType:["<< processType<<"]" 
			//	<< ((eProcessLogger <= processType)&&(eProcessDNSAgent >= processType))?
			//	CProcessBase::GetProcessName((eProcessType)processType):" ????? "	;
			//G_PlcmDns.bResolveHostName("www.cnn.com", 2, processType, connId, partyId);

			//TRACEINTO << "PLCM_DNS. DNS_RESOLVE_DOMAIN_REQ (TEST): Host:[" << "FUFLO12345.com"
			//	<< "] ServiceID[" << 3 
			//	<< "] ProccessType:["<< processType<<"]" 
			//	<< ((eProcessLogger <= processType)&&(eProcessDNSAgent >= processType))?
			//	CProcessBase::GetProcessName((eProcessType)processType):" ????? "	;
			//G_PlcmDns.bResolveHostName("FUFLO12345.com", 3, processType, connId, partyId);

			//TRACEINTO << "PLCM_DNS. DNS_RESOLVE_DOMAIN_REQ (TEST): Host:[" << "siguser30.dev13.std"
			//	<< "] ServiceID[" << 3 
			//	<< "] ProccessType:["<< processType<<"]" 
			//	<< ((eProcessLogger <= processType)&&(eProcessDNSAgent >= processType))?
			//	CProcessBase::GetProcessName((eProcessType)processType):" ????? "	;
			//G_PlcmDns.bResolveHostName("siguser30.dev13.std", 1, processType, connId, partyId);

    //for DEBUG!!!--------------------------------------------------------// 
    
	TRACEINTO << "PLCM_DNS. DNS_RESOLVE_DOMAIN_REQ. Host:[" << szDomainName//pDomainName 
		<< "] Serv.ID:[" << serviceID 
		<< "] ProccessType:["<< processType<<"]"
		<< " ("<< ( ((eProcessLogger <= processType)&&(eProcessDNSAgent >= processType))?
			CProcessBase::GetProcessName((eProcessType)processType):" ????? ") 
		<< ")"
		<< " || PartyId: " << partyId;

	G_PlcmDns.bResolveHostName(/*pDomainName*/szDomainName, serviceID, processType, connId, partyId, FALSE, m_pClientRspMbx);
	

	//---For debug ONLY -----------------------------------------------------------------------//
	{
		//G_PlcmDns.bSendRequestSrv("_sip._tls.dev13.std", serviceID, processType, eIPProtocolType_SIP, eTransportTypeTls);
	}
    //-----------------------------------------------------------------------------------------// 

	//DEALLOCBUFFER(pDomainName);

	return;

	//-E- PLCM_DNS ------------------------------//
    
}

void CDNSAgentManager::OnServiceReq(CSegment* pParam)
{
	WORD serviceID;
	DWORD connId=0, partyId=0;
	WORD temp;
	eProcessType processType;
	ALLOCBUFFER(pDomainName, DnsQueryNameSize);
	int indInDB = -1;
	eIPProtocolType protocolType;
	WORD transportType, protocol;

	*pParam >> serviceID
			>> pDomainName
			>> protocol
			>> transportType
			>> temp;

	protocolType = (eIPProtocolType)protocol;
	processType = (eProcessType)temp;

	PTRACE2(eLevelInfoNormal,"CDNSAgentManager::OnServiceReq, ", pDomainName);

    //-S- PLCM_DNS ------------------------------// 
	{
		char szReqHostName[PLCM_DNS_HOST_NAME_SIZE]	="";
		char szSignalProt[32]						="";
		char szTransportProt[32]					="";

		if(eIPProtocolType_SIP == protocolType)	    strncpy(szSignalProt, "_sip" ,sizeof(szSignalProt)-1);
		if(eIPProtocolType_H323 == protocolType)	strncpy(szSignalProt, "_h323",sizeof(szSignalProt)-1);
	
		if(eTransportTypeUdp == transportType)	strncpy(szTransportProt, "_udp", sizeof(szTransportProt)-1);
		if(eTransportTypeTcp == transportType)	strncpy(szTransportProt, "_tcp", sizeof(szTransportProt)-1);
		if(eTransportTypeTls == transportType)	strncpy(szTransportProt, "_tls", sizeof(szTransportProt)-1);

		if((0 < strlen(szSignalProt))&&(0 < strlen(szTransportProt)))
		{
			if(0 < strlen(pDomainName))
				snprintf(szReqHostName, sizeof(szReqHostName)-1,"%s.%s.%s", szSignalProt, szTransportProt, pDomainName);
			else
				snprintf(szReqHostName, sizeof(szReqHostName)-1,"%s.%s", szSignalProt, szTransportProt);
		
			TRACEINTO << "PLCM_DNS. DNS_SERVICE_REQ. SRV:[" << szReqHostName 
				<< "] Serv.ID[" << serviceID 
				<< "] ProccessType:["<< processType<<"]"
				<< " ("<< ( ((eProcessLogger <= processType)&&(eProcessDNSAgent >= processType))?
				CProcessBase::GetProcessName((eProcessType)processType):" ????? ") 
				<< ")";

			G_PlcmDns.bSendRequestSrv(szReqHostName, serviceID, processType
									  , (eIPProtocolType)protocolType //eIPProtocolType
									  , (enTransportType)transportType//enTransportType
									  );
		}
	}

	DEALLOCBUFFER(pDomainName);

	return;

	//-E- PLCM_DNS ------------------------------//

	enQueryType eQueryType = eServiceQuery;
	indInDB = IsQueryExist(eAllQuery,processType, serviceID, pDomainName);
	if(indInDB == -1)
	{
		AddQuery(eQueryType,serviceID, pDomainName, protocolType, transportType, processType, connId, partyId);
	}
	else
	{
		WORD queryState = GetQueryState(indInDB);
		if(DNS_VALID == queryState)
		{
			ipAddressStruct ipAddrSt[TOTAL_NUM_OF_IP_ADDRESSES];
			for(int i=0; i<TOTAL_NUM_OF_IP_ADDRESSES; i++)
			{
		    	ipAddressStruct* ipAddrSt = m_pDNSQueryEnteties[i]->GetIpAddress(i);
    			memcpy(&ipAddrSt[i], m_pDNSQueryEnteties[i]->GetIpAddress(i), sizeof(ipAddressStruct));
	 			if(ipAddrSt->ipVersion == eIpVersion4)
				{
					char strIpV4[16];
					SystemDWORDToIpString(ipAddrSt->addr.v4.ip, strIpV4);
					PTRACE2(eLevelInfoNormal,"CDNSAgentManager::OnResolveDomainReq, No need to resolve, returning result = ", strIpV4);
				}
				else if(ipAddrSt->ipVersion == eIpVersion6)
				{
					char strIpV6[128];
					ipV6ToString(ipAddrSt->addr.v6.ip, strIpV6, TRUE);
					PTRACE2(eLevelInfoNormal,"CDNSAgentManager::OnResolveDomainReq, No need to resolve, returning result = ", strIpV6);
				}

			}

			DWORD port = m_pDNSQueryEnteties[indInDB]->GetPort(); //rons

			CSegment*  pRetParam = new CSegment();
		    *pRetParam << serviceID
			        << pDomainName
		    //     	<< (DWORD)ip
					<< (DWORD)port
					<< transportType;

		//	pRetParam->Put((BYTE*)&ip, sizeof(mcTransportAddress));
			pRetParam->Put((BYTE*)&ipAddrSt, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

			const COsQueue* pEntityMbx =
	    		CProcessBase::GetProcess()->GetOtherProcessQueue(processType, eManager);

	 		if(processType == eProcessSipProxy)
	 		{
	 		    CSipProxyTaskApi api(serviceID);
	 		   STATUS res = api.SendMsg(pParam, DNS_SERVICE_IND);
	 		}
	 		else
	    	   STATUS res = pEntityMbx->Send(pRetParam,DNS_SERVICE_IND);
		}
		else
		{
			if(DNS_IDLE == queryState)
				m_pDNSQueryEnteties[indInDB]->Retry();
		}
	}
	DEALLOCBUFFER(pDomainName);
}

void CDNSAgentManager::OnCSApi_Msg(CSegment *pSeg)
{
	PTRACE(eLevelInfoNormal, "CDNSAgentManager::OnCSApi_Msg");

	CMplMcmsProtocol  mplMcmsProtocol;
    mplMcmsProtocol.DeSerialize(*pSeg, CS_API_TYPE);
    OPCODE opcode = mplMcmsProtocol.getOpcode();
    pSeg->ResetRead();

    DispatchEvent(opcode, pSeg);
}
//-S-PLCM_DNS---------------------------------------------------//
void CDNSAgentManager::OnResolveInd(CSegment *pParam)
{
	int 	nAddresses;
	DWORD 	ipAddr = 0;

	CMplMcmsProtocol  mplMcmsProtocol;

	mplMcmsProtocol.DeSerialize(*pParam, CS_API_TYPE);
	int data_len= mplMcmsProtocol.getDataLen();
	ipAddressStruct ipAddrSt[TOTAL_NUM_OF_IP_ADDRESSES];
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&ipAddrSt[i], 0, sizeof(ipAddressStruct));
	}
	//DNS_CS_RESOLVE_IND_ST * pDnsCsResolveInd = (DNS_CS_RESOLVE_IND_ST *)malloc(data_len);
	DNS_CS_RESOLVE_IND_ST  csDnsIndResolve;// = *pDnsCsResolveInd;

	memcpy(&csDnsIndResolve, mplMcmsProtocol.GetData(), data_len);
	int transId		= csDnsIndResolve.transactionId;
	int status	 	= csDnsIndResolve.status;

	char			szHostName[PLCM_DNS_HOST_NAME_SIZE]="";
	char			errReason[DnsErrDescSize] = "";
	eProcessType	processType					= eProcessTypeInvalid;	
	DWORD			connId = 0xFFFFFFFF, partyId = 0;

	strncpy(szHostName, csDnsIndResolve.hostName, sizeof(szHostName)-1);

	TRACEINTO << "PLCM_DNS. DNS_RESOLVE_DOMAIN_REQ FROM CS. Host:[" << szHostName << "]"
		<< " | Service[" << csDnsIndResolve.transactionId << "]"
		<< " | From CS | Status:[" << status;

	BOOL bIsForceResolving = FALSE;
	if(-777 == status)
		bIsForceResolving = TRUE;
	G_PlcmDns.bResolveHostName(szHostName, csDnsIndResolve.transactionId, processType, connId, partyId, bIsForceResolving, m_pClientRspMbx);
	//free(pDnsCsResolveInd);

return;
/*
	CDNSQuery *pDnsQuery = m_pDNSQueryEnteties[transId];

	PTRACE2(eLevelInfoNormal,"CDNSAgentManager::OnResolveInd, domain name:", domainName);

	//_M_S_
	if (status == -3) //DNS temp unavailable - will retry on timeout
	{
		PTRACE2(eLevelInfoNormal, "CDNSAgentManager::OnResolveInd, temporary error (DNS not ready yet), domain name:", domainName);
	}
	else // if query wasn't found
	if (strcmp(pDnsQuery->GetDomainName(), domainName)) {
		char buf[MaxPingStrLen];
		snprintf(buf, sizeof(buf), "CDNSAgentManager::OnResolveInd, query not found, domain name:%s", domainName);
		PASSERTMSG(TRUE, buf );
	} else {

		PTRACE2INT(eLevelInfoNormal,"CDNSAgentManager::OnResolveInd, status:", status);
		// if resolve succedded
		if (status == STATUS_OK) {

			ipAddressElemSt 	*pElem;
			xmlIpAddressElemSt 	*pXmlElem;

			pXmlElem = (xmlIpAddressElemSt *) &csDnsIndResolve.ipAddrList;
			char *pNextChar = NULL;
			int addrCount = csDnsIndResolve.nAddresses;

			if (addrCount > TOTAL_NUM_OF_IP_ADDRESSES)
			{
				addrCount = TOTAL_NUM_OF_IP_ADDRESSES;
			}

			for (int i = 0; i < addrCount; i++) 
			{

				pElem = &pXmlElem->elem;

				switch (pElem->ipType) 
				{
					case eIpVersion4:

					ipAddrSt[i].ipVersion 	= eIpVersion4;
					ipAddrSt[i].addr.v4.ip 	= ntohl(inet_addr(pElem->ipAddress));
					break;

					case eIpVersion6:

						mcTransportAddress tmpIPv6Addr;
						memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
						::stringToIpV6( &tmpIPv6Addr, (char*)pElem->ipAddress );
						ipAddrSt[i].ipVersion 	= eIpVersion6;
						memcpy(&ipAddrSt[i].addr.v6.ip, &(tmpIPv6Addr.addr.v6.ip), IPV6_ADDRESS_BYTES_LEN);
						// BRIDGE-6051
						enScopeId dwScopeID;
						char szIP[256];
						memset(szIP, '\0', sizeof(szIP));
						ipV6ToString(tmpIPv6Addr.addr.v6.ip, szIP, FALSE);
						dwScopeID = ::getScopeId(szIP);
						ipAddrSt[i].addr.v6.scopeId = (APIU32)dwScopeID;

					break;

					default:
					PTRACE2INT(eLevelInfoNormal, "CDNSAgentManager::OnResolveInd, unknown ip type:", pElem->ipType);
					break;
				}
				
				pNextChar = (char*)pXmlElem;
				pNextChar += sizeof(xmlIpAddressElemSt);
				pXmlElem = (xmlIpAddressElemSt *)pNextChar;
			}

		} else {
			memcpy(
				errReason,
				csDnsIndResolve.errReason,
				DnsErrDescSize);
		}

		CSegment* pSeg = new CSegment();

		*pSeg 	<< (WORD) transId
				<< domainName
				<< (DWORD) status
				<< errReason
				<< (DWORD) pDnsQuery->GetTtl();

		pSeg->Put((BYTE*)&ipAddrSt, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

		pDnsQuery->HandleEvent(pSeg, 0, DNS_RESOLVE_IND);

		free(pDnsCsResolveInd);
	}
*/
}
//-E-PLCM_DNS---------------------------------------------------//

void	CDNSAgentManager::OnServiceInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CDNSAgentManager::OnServiceInd");

	int     nAddresses;
	DWORD 	resolvedIp 	= 0;
	DWORD 	ttl 		= 0;
	WORD 	port		= 0;
	WORD 	priority   	= 0;
	WORD    weight		= 0;
	char	hostName[DnsQueryNameSize] = "\0";

	CMplMcmsProtocol  mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pParam, CS_API_TYPE);
	int data_len= mplMcmsProtocol.getDataLen();

	ipAddressStruct ipAddrSt[TOTAL_NUM_OF_IP_ADDRESSES];

	DNS_CS_SERVICE_IND_ST 	csDnsIndService;

	memcpy(&csDnsIndService, mplMcmsProtocol.GetData(), data_len);

	int transId		= csDnsIndService.transactionId;
	int status	 	= csDnsIndService.status;

	char domainName[DnsQueryNameSize];
	char errReason[DnsErrDescSize] = "\0";

	memcpy(
		domainName,
		&csDnsIndService.domainName[0],
		DnsQueryNameSize);

	CDNSServiceQuery *pDnsSrvQuery = (CDNSServiceQuery *) m_pDNSQueryEnteties[transId];

	PTRACE2(eLevelInfoNormal,"CDNSAgentManager::OnServiceInd, domain name:", domainName);

	// if query wasn't found
	if (strcmp(pDnsSrvQuery->GetDomainName(), domainName)) {
		PTRACE2(eLevelInfoNormal, "CDNSAgentManager::OnServiceInd, query not found, domain name:", domainName);
	} else {

		PTRACE2INT(eLevelInfoNormal,"CDNSAgentManager::OnServiceInd, status:", status);
		// if resolve succedded
		if (status == STATUS_OK) {

			ipAddressElemSt 	*pElem;
			xmlIpAddressElemSt 	*pXmlElem;

			nAddresses = csDnsIndService.nAddresses;

			for (int i = 0; i < nAddresses; i++) {

				pXmlElem = (xmlIpAddressElemSt *) &csDnsIndService.ipAddrList[i];

				pElem = &pXmlElem->elem;

				switch (pElem->ipType) {

					case eIpVersion4:
					ipAddrSt[i].ipVersion 	= eIpVersion4;
					ipAddrSt[i].addr.v4.ip 	= ntohl(inet_addr(pElem->ipAddress));
					break;

					case eIpVersion6:
					ipAddrSt[i].ipVersion 	= eIpVersion6;
					memcpy(&ipAddrSt[i].addr.v6.ip, pElem->ipAddress, IPV6_ADDRESS_BYTES_LEN);
					break;

					default:
					PTRACE2INT(eLevelInfoNormal, "CDNSAgentManager::OnServiceInd, unknown ip type:", pElem->ipType);
					break;
				}
			}

			memcpy(
				hostName,
				&csDnsIndService.hostName[0],
				DnsQueryNameSize);

			ttl			= csDnsIndService.ttl;
			port 		= csDnsIndService.port;
			priority	= csDnsIndService.priority;
			weight 		= csDnsIndService.weight;

		} else {
			memcpy(
				errReason,
				csDnsIndService.errReason,
				 DnsErrDescSize);
		}
	}

	CSegment* pSeg = new CSegment();

	*pSeg	<< (DWORD) transId
			<< domainName
			<< hostName
			<< (DWORD) status
			<< errReason
			<< (DWORD) ttl
			<< (WORD) port
			<< (WORD) priority
			<< (WORD) weight;

	pSeg->Put((BYTE*)&ipAddrSt, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	pDnsSrvQuery->HandleEvent(pSeg, 0, DNS_SERVICE_IND);
}

void CDNSAgentManager::OnClearQueries(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CDNSAgentManager::OnClearQueries");
	WORD bIsSip;
	enQueryType eQueryType;
	for(int i = 0; i<MAX_QUERIES; i++)
	{
		if(IsValidPObjectPtr(m_pDNSQueryEnteties[i]))
		{
				POBJDELETE(m_pDNSQueryEnteties[i]);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CDNSAgentManager::OnStatusUpdate(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CDNSAgentManager::OnStatusUpdate");
	DWORD status = STATUS_OK;
	*pParam >> status;

	if(STATUS_TRY_AGAIN == status || STATUS_FAIL == status)
	{
		AddActiveAlarmSingleton(	FAULT_GENERAL_SUBJECT,
						FAILED_TO_ACCESS_DNS_SERVER,
						SYSTEM_MESSAGE,
						"Failed to access DNS server",
						true,
						true
					);
	}
	else
		RemoveActiveAlarmByErrorCode((WORD)FAILED_TO_ACCESS_DNS_SERVER);
}

void CDNSAgentManager::RemoveServiceIdFromQueries(CSegment* pSeg)
{

	PTRACE(eLevelInfoNormal, "CDNSAgentManager::RemoveServiceFromQueries");
	Del_Ip_Service_S param;
	memcpy(&param, pSeg->GetPtr(), sizeof(param));

	WORD serviceId=param.service_id;

	for(int i = 0; i<MAX_QUERIES; i++)
	{
		if(IsValidPObjectPtr(m_pDNSQueryEnteties[i]))
		{
			if(m_pDNSQueryEnteties[i]->GetServiceId() == serviceId)
				POBJDELETE(m_pDNSQueryEnteties[i]);
		}
	}
}

int	CDNSAgentManager::IsQueryExist(enQueryType queryType,eProcessType processType, WORD serviceID, char* pDomainName, WORD index )
{
	int result = -1;
	for(int i = 0; i<MAX_QUERIES; i++)
	{
		if(IsValidPObjectPtr(m_pDNSQueryEnteties[i]))
		{
				if( serviceID == m_pDNSQueryEnteties[i]->GetServiceId() && processType == m_pDNSQueryEnteties[i]->GetProcessType())
				{
					if(strcmp(pDomainName, "") && index != (WORD)-1)
					{
						if(!strncmp(m_pDNSQueryEnteties[i]->GetDomainName(), pDomainName, DnsQueryNameSize)
							&& i == index)
						{
							result = i;
							break;
						}
					}
					else
					{
						if(strcmp(pDomainName, ""))
						{
							if(!strncmp(m_pDNSQueryEnteties[i]->GetDomainName(), pDomainName, DnsQueryNameSize))
							{
								result = i;
								break;
							}
						}

						else if (index != (WORD)-1)
						{
							if (i == index)
							{
								result = i;
								break;
							}
						}
					}
				}
		}
	}
	return result;
}

int	CDNSAgentManager::IsQueryExist(enQueryType queryType, char* pDomainName, WORD index )
{
	int result = -1;
	for(int i = 0; i<MAX_QUERIES; i++)
	{
		if(IsValidPObjectPtr(m_pDNSQueryEnteties[i]))
		{
			if(eAllQuery || (eResolveQuery == queryType && !strncmp(m_pDNSQueryEnteties[i]->NameOf(), "CDNSQuery", 10))
				|| (eServiceQuery == queryType && !strncmp(m_pDNSQueryEnteties[i]->NameOf(), "CDNSServiceQuery", 17))) {
				if(strcmp(pDomainName, "") && index != (WORD)-1)
				{
					if(!strncmp(m_pDNSQueryEnteties[i]->GetDomainName(), pDomainName, DnsQueryNameSize)
						&& i == index)
					{
						result = i;
						break;
					}
				}
				else
				{
					if(strcmp(pDomainName, ""))
					{
						if(!strncmp(m_pDNSQueryEnteties[i]->GetDomainName(), pDomainName, DnsQueryNameSize))
						{
							result = i;
							break;
						}
					}

					else if (index != (WORD)-1)
					{
						if (i == index)
						{
							result = i;
							break;
						}
					}
				}
			}

		}
	}
	return result;
}

void	CDNSAgentManager::AddQuery(enQueryType queryType,WORD serviceID, char* pDomainName, eIPProtocolType protocolType, DWORD transportType,
									eProcessType processType, DWORD connId, DWORD partyId)
{
	int i;
	for( i=0 ; i<MAX_QUERIES; i++)
		if(m_pDNSQueryEnteties[i] == NULL)
		{
			switch(queryType)
			{
				case(eResolveQuery):
					m_pDNSQueryEnteties[i] = new CDNSQuery;
					break;
				case(eServiceQuery):
					m_pDNSQueryEnteties[i] = new CDNSServiceQuery;
					break;
				case(eResolveDummy):
					m_pDNSQueryEnteties[i] = new CDNSDummyQuery;
				    break;
				default:
					// Note: some enumeration value are not handled in switch. Add default to suppress warning.
					break;
			}

			if(m_pDNSQueryEnteties[i]==NULL) {
				DBGPASSERT_AND_RETURN(2);
			}


			m_pDNSQueryEnteties[i]->Create(serviceID,i, pDomainName, m_pRcvMbx, protocolType, transportType, processType, connId, partyId);

			break;
		}
	if(i==MAX_QUERIES)
	{
		PTRACE(eLevelError, "CDNSAgentManager::AddQuery, can not add. array is full.");
		DBGPASSERT(1);
	}
}

WORD	CDNSAgentManager::GetQueryState(int indDB)
{
	WORD result = FALSE;
	if(m_pDNSQueryEnteties[indDB] != NULL)
		result = m_pDNSQueryEnteties[indDB]->GetQueryState();
	return result;
}

/////////////////////////////////////////////////////////////////////
void CDNSAgentManager::DeclareStartupConditions()
{
	CActiveAlarm aa(FAULT_GENERAL_SUBJECT,
					NO_IND_DNS_SUCCESS,
					MAJOR_ERROR_LEVEL,
					"DNS configuration error.",
					true,
					true);
	AddStartupCondition(aa);
}
////////////////////////////////////////////////////////////////////////////
void CDNSAgentManager::ManagerStartupActionsPoint()
{

	CProcessBase::GetProcess()->m_NetSettings.LoadFromFile();

	WORD flag = eDnsNotConfigured;
	if(eServerStatusOff != CProcessBase::GetProcess()->m_NetSettings.m_ServerDnsStatus )
	{
		flag =CProcessBase::GetProcess()->m_NetSettings.m_DnsConfigStatus;
	}

	OnMcuMngrConfigurationStatus(flag);

}

////////////////////////////////////////////////////////////////////////////

void CDNSAgentManager::OnMcuMngrConfigurationStatus(WORD flag)
{
	switch(flag)
		{
			case ( (WORD)eDnsConfigurationSuccess ):
			{
				TRACEINTO << "CDNSAgentManager::OnMcuMngrConfigurationStatus DNS Configuration: OK";
				m_state=CONNECT;
				RemoveActiveAlarmByErrorCode(NO_IND_DNS_SUCCESS);
				break;
			}

			case ( (WORD)eDnsConfigurationFailure ):
			{
				TRACEINTO << "CDNSAgentManager::OnMcuMngrConfigurationStatus DNS Configuration: Failed";
				m_state = DISCONNECTED;
				UpdateStartupConditionByErrorCode(NO_IND_DNS_SUCCESS, eStartupConditionFail);
				break;
			}

			case ( (WORD)eDnsNotConfigured ):
			{
				TRACEINTO << "CDNSAgentManager::OnMcuMngrConfigurationStatus DNS Configuration: Not Configured";
			//	m_state = DISCONNECTED;
				m_state = CONNECT;
				RemoveActiveAlarmByErrorCode(NO_IND_DNS_SUCCESS);
				RemoveActiveAlarmByErrorCode(FAILED_TO_ACCESS_DNS_SERVER);

				break;
			}

			default:
			{
				TRACEINTO << "CDNSAgentManager::OnMcuMngrConfigurationStatus DNS Configuration: Illegal";
				m_state = DISCONNECTED;
				UpdateStartupConditionByErrorCode(NO_IND_DNS_SUCCESS, eStartupConditionFail);
				break;
			}
		} // end switch



		OnClearQueries();
/*
		if(CONNECT == m_state)
			CreateDummyResolution();
			switch(flag)
			{
				case ( (WORD)eDnsConfigurationSuccess ):
				{
					TRACEINTO << "CDNSAgentManager::OnMcuMngrConfigurationStatus DNS Configuration: OK";
					m_state=CONNECT;
					RemoveActiveAlarmByErrorCode(NO_IND_DNS_SUCCESS);
					break;
				}

				case ( (WORD)eDnsConfigurationFailure ):
				{
					TRACEINTO << "CDNSAgentManager::OnMcuMngrConfigurationStatus DNS Configuration: Failed";
					m_state = DISCONNECTED;
					UpdateStartupConditionByErrorCode(NO_IND_DNS_SUCCESS, eStartupConditionFail);
					break;
				}

				case ( (WORD)eDnsNotConfigured ):
				{
					TRACEINTO << "CDNSAgentManager::OnMcuMngrConfigurationStatus DNS Configuration: Not Configured";
				//	m_state = DISCONNECTED;
					m_state = CONNECT;
					RemoveActiveAlarmByErrorCode(NO_IND_DNS_SUCCESS);
					RemoveActiveAlarmByErrorCode(FAILED_TO_ACCESS_DNS_SERVER);

					break;
				}

				default:
				{
					TRACEINTO << "CDNSAgentManager::OnMcuMngrConfigurationStatus DNS Configuration: Illegal";
					m_state = DISCONNECTED;
					UpdateStartupConditionByErrorCode(NO_IND_DNS_SUCCESS, eStartupConditionFail);
					break;
				}
			} // end switch

			OnClearQueries();

			if(CONNECT == m_state)
				CreateDummyResolution();
*/		

////-S- PLCM_DNS ------------------------------//
//	if(CONNECT == m_state)
//	{
//		if(ePLCM_DNS_STATE_IDLE == G_PlcmDns.m_eObjState)
//		{
//			BOOL bRcDnsObj = G_PlcmDns.bActivate(this->m_pRcvMbx);
//			if(TRUE == bRcDnsObj)
//				PTRACE(eLevelInfoNormal,"PLCM_DNS. G_PlcmDns has STARTED. (Successfully)");
//			else
//				PTRACE(eLevelInfoNormal,"PLCM_DNS. G_PlcmDns has NOT Started (UN-Successful)!!!");
//		}
//	}
//	//-E- PLCM_DNS ------------------------------//
		
}

void CDNSAgentManager::OnMcuMngrConfigurationStatusInd(CSegment* pParam)
{
	TRACEINTO << "CDNSAgentManager::OnMcuMngrConfigurationStatusInd DNS Configuration: Indication recieved.";
	WORD flag = (WORD)eDnsConfigurationFailure;
	*pParam >> flag;

	OnMcuMngrConfigurationStatus(flag);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CDNSAgentManager::OnCsMngrDnsConfig(CSegment* pParam)
{
	WORD    tmp;

	DNS_IP_CONFIG_PARAM_S  config;
	pParam->Get((BYTE*)&config,sizeof(DNS_IP_CONFIG_PARAM_S));

	std::stringstream sstr;
	sstr << "IpType:" << config.ipType << ", NumOfSrv:" << config.servicesNum;

	char szTemp[IPV6_ADDRESS_LEN];

	int nSerAmount = min(config.servicesNum, (WORD)MAX_NUM_OF_IP_SERVICES);
	
	TRACEINTO << "PLCM_DNS. Configuration ARRIVED for [" << nSerAmount<<"] SERVICES"; 	
	

	for (int i=0; ((i< nSerAmount) && (i < MAX_NUM_OF_IP_SERVICES)); ++i)//Klocwork.Sasha #3980
	{
		sstr << "\n  Service:" << i << ", Status:" << config.services[i].dnsStatus << "; DomainName:" << config.services[i].szDomainName;
		for (int j=0; j<NUM_OF_DNS_SERVERS; ++j)
		{
			SystemDWORDToIpString(config.services[i].serversIpv4List[j],szTemp);
			sstr << "; IPv4:" << szTemp;
		}

		for (int j=0; j<NUM_OF_DNS_SERVERS; ++j)
		{
			sstr << "; IPv6:" << config.services[i].serversIpv6List[j];
		}

		if((0 <= i)&&(MAX_NUM_OF_IP_SERVICES > i))
		{
			G_PlcmDns.m_aConf[i+1].vSetConf(i+1, &config.services[i], config.ipType);
		}
	}

	G_PlcmDns.m_DnsArrSocket4.vArrSocketInit(eIpVersion4 , (void *) &G_PlcmDns);
	G_PlcmDns.m_DnsArrSocket6.vArrSocketInit(eIpVersion6 , (void *) &G_PlcmDns);

	G_PlcmDns.m_eObjState = ePLCM_DNS_STATE_ACTIVATED;

	TRACEINTO << sstr.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CDNSAgentManager::OnCsMngrCsConfig(CSegment* pParam)
{
	WORD csId;
	std::string sIpv4, sIpv6;

	*pParam >> csId >> sIpv4 >> sIpv6;

	if((2 == strlen(sIpv6.c_str()))&&(0 == strncmp("::", sIpv6.c_str(), 2)))
	{
		sIpv6 = "";
	}

	TRACEINTO << "PLCM_DNS. CS config received, CS.Id:" << csId 
		      << "| IPv4: [" << sIpv4 << "]"
		      << "| IPv6: [" << sIpv6 << "]";

	{
		BOOL bRcConf_IPv4 = FALSE;
		BOOL bRcConf_IPv6 = FALSE;

		char szIpv4[PLCM_STRING_SIZE_256]="";
		char szIpv6[PLCM_STRING_SIZE_256]="";
		memset(szIpv4, '\0', sizeof(szIpv4));
		memset(szIpv6, '\0', sizeof(szIpv6));

		strncpy(szIpv4, sIpv4.c_str(), sizeof(szIpv4)-1);
		strncpy(szIpv6, sIpv6.c_str(), sizeof(szIpv4)-1);

		//G_PlcmDns.bReConfigSourceIpForBind(csId, eIpVersion4, sIpv4.c_str());
		bRcConf_IPv4 = G_PlcmDns.bReConfigSourceIpForBind_IPv4(csId, szIpv4);
		G_PlcmDns.m_DnsArrSocket4.vSocketArrayDump("PLCM_DNS. ReConfigSourceIpForBind IPv4", csId, csId);
		//G_PlcmDns.bReConfigSourceIpForBind(csId, eIpVersion6, sIpv6.c_str());
		bRcConf_IPv6 = G_PlcmDns.bReConfigSourceIpForBind_IPv6(csId, szIpv6);
		G_PlcmDns.m_DnsArrSocket6.vSocketArrayDump("PLCM_DNS. ReConfigSourceIpForBind IPv6", csId, csId);

		{//-S-- BRIDGE-18110 -----------------------//
			BOOL bIsRequired_IPv4 = FALSE;
			BOOL bIsRequired_IPv6 = FALSE;

			G_PlcmDns.bGetRequiredAorAAAAByServiceId(csId, &bIsRequired_IPv4, &bIsRequired_IPv6);

			if((TRUE == bIsRequired_IPv4)&&(FALSE == bRcConf_IPv4))
			{//cPLCM_DNS

				cDNS_SOCKET           * pSocketObj = &G_PlcmDns.m_DnsArrSocket4.m_SockObjArray[csId];
                cDNS_SERVICE_CONFIG   * pConfObj   = &G_PlcmDns.m_aConf[csId];
				TRACEINTO << "PLCM_DNS.  ERROR! CS.Id:" << csId 
						  << " | cDNS_SOCKET for SignalingIP:" << pConfObj->m_sConf.szSignalingIPv4   
						  << "  (" <<   pConfObj->m_sConf.sBindPorts.wPort_v4 << ")" 
						  << " | Status: " << pSocketObj->m_szGetStrSocketState() ;
			}

			if((TRUE == bIsRequired_IPv6)&&(FALSE == bRcConf_IPv6))
			{//cPLCM_DNS

				cDNS_SOCKET           * pSocketObj = &G_PlcmDns.m_DnsArrSocket6.m_SockObjArray[csId];
				cDNS_SERVICE_CONFIG   * pConfObj   = &G_PlcmDns.m_aConf[csId];
				TRACEINTO << "PLCM_DNS.  ERROR! CS.Id:" << csId 
					<< " | cDNS_SOCKET for SignalingIP:" << pConfObj->m_sConf.szSignalingIPv6   
					<< "  (" <<   pConfObj->m_sConf.sBindPorts.wPort_v6 << ")" 
					<< " | Status: " << pSocketObj->m_szGetStrSocketState() ;
			}
		}//-E-- BRIDGE-18110 -----------------------//
	}
}

/////////////////////////////////////////////////////////////////////////////

STATUS CDNSAgentManager::HandleTerminalDumpAgent(CTerminalCommand & command, std::ostream& answer)
{
	COstrStream msg;
	Dump(msg);
	answer << msg.str().c_str();

    return STATUS_OK;
}

//-S-PLCM_DNS---------------------------------------------------------------------------------------//

//===============================================================================================//
void vParsingConfParam(const char * par_szParam, cDNS_SERVICE_CONFIG * par_cConf, int * par_pDnsInd, int * par_pDomInd)
{
	if((NULL != par_szParam)&&(0 < strlen(par_szParam))&&(NULL != par_cConf))
	{
//1 -4Y -6N -n8.8.8.8 -dpolycom.com -t5 -a1 -c -r -s

		if('-' == par_szParam[0])
		{
			switch(par_szParam[1])
			{
			case 's':
				{//IP source for BIND IPv4
					char * pSourceIP = (char*)(&par_szParam[2]);

					G_PlcmDns.bReConfigSourceIpForBind_IPv4(par_cConf->m_wSerID, pSourceIP);
					G_PlcmDns.m_DnsArrSocket4.vSocketArrayDump("PLCM_DNS. ReConfigSourceIpForBind IPv4 (s)", par_cConf->m_wSerID, par_cConf->m_wSerID);
				}
            break; 
			case 'S':
				{//IP source for BIND IPv6
					char * pSourceIP = (char*)(&par_szParam[2]);

					G_PlcmDns.bReConfigSourceIpForBind_IPv6(par_cConf->m_wSerID, pSourceIP);
					G_PlcmDns.m_DnsArrSocket6.vSocketArrayDump("PLCM_DNS. ReConfigSourceIpForBind IPv6 (s)", par_cConf->m_wSerID, par_cConf->m_wSerID);
				}
			break;

			case 'c':
			case 'C':
				{
					par_cConf->vClean(par_cConf->m_wSerID);
				}
            break;  
			case 'r':
			case 'R':
				{
					par_cConf->bGetResolvConfData(&par_cConf->m_sConf, par_cConf->m_wSerID);
				}
            break;  
			case '4': 
				{
					if(('Y'== par_szParam[2])||('y'== par_szParam[2])) par_cConf->m_sConf.bIsRequest_A = TRUE;
					else 
						par_cConf->m_sConf.bIsRequest_A = FALSE;
				}
				break;

			case '6': 
				{
					if(('Y'== par_szParam[2])||('y'== par_szParam[2])) par_cConf->m_sConf.bIsRequest_AAAA = TRUE;
					else 
						par_cConf->m_sConf.bIsRequest_AAAA = FALSE;
				}
				break;

			case 't':
			case 'T':  
				{
					par_cConf->m_sConf.dwTimeOut_InSec = atoi(&par_szParam[2]);
				}
				break;

			case 'a':
			case 'A':  
				{
					par_cConf->m_sConf.wAttempts = atoi(&par_szParam[2]);
				}
				break;

			case 'n':
			case 'N':
				{
					if((0 <= (*par_pDnsInd))&&( PLCM_DNS_MAX_ADDITIONAL_IP > (*par_pDnsInd)) )
					{
						if(0 == (*par_pDnsInd))
						{
							memset(par_cConf->m_sConf.aDnsAddress, 0, sizeof(ipAddressStruct)*PLCM_DNS_MAX_ADDITIONAL_IP);
							par_cConf->m_sConf.dwDnsAddressAmount = 0;
						}

						stringToIp(&par_cConf->m_sConf.aDnsAddress[(*par_pDnsInd)], (char*)(&par_szParam[2]), eNetwork);
						par_cConf->m_sConf.dwDnsAddressAmount++;
						(*par_pDnsInd) = (*par_pDnsInd) +1;
					}
				}
            break;

			case 'd':
			case 'D':
				{
					if((0 <= (*par_pDomInd))&&( PLCM_DNS_DOMAINNAME_AMOUNT_MAX > (*par_pDomInd)) )	
					{
						if(0 == (*par_pDomInd))
						{
							memset(par_cConf->m_sConf.aDomainName, 0, sizeof(DNS_PLCM_DOMAINNAME)*PLCM_DNS_DOMAINNAME_AMOUNT_MAX);
							par_cConf->m_sConf.dwDomainNameAmount = 0;
						}

						strncpy(par_cConf->m_sConf.aDomainName[(*par_pDomInd)].szDomainName, &par_szParam[2]
						          , sizeof(par_cConf->m_sConf.aDomainName[(*par_pDomInd)].szDomainName)-1);

						par_cConf->m_sConf.dwDomainNameAmount++;

						(*par_pDomInd) = (*par_pDomInd) +1;
					}
				}
            break; 
			default:
            break;   
			}
		}
	}
}
//===============================================================================================//
//===============================================================================================//
STATUS CDNSAgentManager::HandleTerminalDnsConf(CTerminalCommand & command, std::ostream& answer)
{
	const std::string& strServId   = command.GetToken(eCmdParam1);
	const std::string& strPar1     = command.GetToken(eCmdParam2);
	const std::string& strPar2     = command.GetToken(eCmdParam3);
	const std::string& strPar3     = command.GetToken(eCmdParam4);
	const std::string& strPar4     = command.GetToken(eCmdParam5);
	const std::string& strPar5     = command.GetToken(eCmdParam6);
	const std::string& strPar6     = command.GetToken(eCmdParam7);
	const std::string& strPar7     = command.GetToken(eCmdParam8);
	const std::string& strPar8     = command.GetToken(eCmdParam9);
	const std::string& strPar9     = command.GetToken(eCmdParam10);

	//1 -4Y -6N -n8.8.8.8 -dpolycom.com -t5 -a1

	unsigned short serviceID = atoi(strServId.c_str());


    if((0 < serviceID)&&(DNS_MAX_SERVICES > serviceID))  
	{
		int nDnsInd = 0;
		int nDomInd = 0;

		
		vParsingConfParam(strPar1.c_str (), &G_PlcmDns.m_aConf[serviceID], &nDnsInd, &nDomInd);
		vParsingConfParam(strPar2.c_str (), &G_PlcmDns.m_aConf[serviceID], &nDnsInd, &nDomInd);
		vParsingConfParam(strPar3.c_str (), &G_PlcmDns.m_aConf[serviceID], &nDnsInd, &nDomInd);
		vParsingConfParam(strPar4.c_str (), &G_PlcmDns.m_aConf[serviceID], &nDnsInd, &nDomInd);
		vParsingConfParam(strPar5.c_str (), &G_PlcmDns.m_aConf[serviceID], &nDnsInd, &nDomInd);
		vParsingConfParam(strPar6.c_str (), &G_PlcmDns.m_aConf[serviceID], &nDnsInd, &nDomInd);
		vParsingConfParam(strPar7.c_str (), &G_PlcmDns.m_aConf[serviceID], &nDnsInd, &nDomInd);
		vParsingConfParam(strPar8.c_str (), &G_PlcmDns.m_aConf[serviceID], &nDnsInd, &nDomInd);
		//vParsingConfParam(strPar9.c_str (), &G_PlcmDns.m_aConf[serviceID], &nDnsInd, &nDomInd);

		G_PlcmDns.vDumpConf(serviceID, serviceID, answer);

		
	}
	else
	{//Print DNS configuration for ALL services  
		G_PlcmDns.vDumpConf(0, DNS_MAX_SERVICES-1, answer);
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CDNSAgentManager::HandleTerminalDnsResolve(CTerminalCommand & command, std::ostream& answer)
{
	const std::string& strServId   = command.GetToken(eCmdParam1);
	const std::string& strHostName = command.GetToken(eCmdParam2);
	
	unsigned short serviceID = atoi(strServId.c_str());

	if(  (0 != strlen(strHostName.c_str()))
	   &&(serviceID < DNS_MAX_SERVICES) //---BRIDGE-16374
	  )
	{//-S- ----- BRIDGE-16374_1 ---------------------------------//
		char szHostName[256]="";

		strncpy(szHostName, strHostName.c_str(), sizeof(szHostName)-1);
		if(0 == strcmp(szHostName, "Invalide Token"))
			szHostName[0]='\0';

		TRACEINTO << "PLCM_DNS. DNS_RESOLVE_DOMAIN_REQ. (Term.Command) Host:[" << szHostName  << "]"
				  << "  Service[" << serviceID <<"]" ;

		G_PlcmDns.bResolveHostName(szHostName, serviceID, eProcessTypeInvalid, 0, 0, FALSE, m_pClientRspMbx);
	}//-E- ----- BRIDGE-16374_1 ---------------------------------//
	else
	{
		answer << "Format command error\n"
			<< "  DNS Resolve.\n"
			<< "    Command format:\n"
			<< "    Bin/McuCmd dns_resolve DNSAgent HOST-NAME Serv.ID\n"
			<< "    Example: Bin/McuCmd dns_resolve DNSAgent www.google.com 1\n";
	}
	return STATUS_OK;
}
//-S-PLCM_DNS---------------------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CDNSAgentManager::HandleTerminalDumpRecords(CTerminalCommand & command, std::ostream& answer)
{
	CProcessBase* proc = CProcessBase::GetProcess();
	PASSERTMSG_AND_RETURN_VALUE(!proc,"bad process",STATUS_FAIL);

	CDNSAgentProcess* pDnsProc = dynamic_cast<CDNSAgentProcess*>(proc);
	PASSERTMSG_AND_RETURN_VALUE(!pDnsProc,"bad CDNSAgentProcess process",STATUS_FAIL);

	DnsRecordsMngr* mngr = (dynamic_cast<CDNSAgentProcess*>(proc))->GetDnsRecordsMngr();
	PASSERTMSG_AND_RETURN_VALUE(!mngr,"bad DnsRecordsMngr !!!!",STATUS_FAIL);

	const std::string& cmd = command.GetToken(eCmdParam1);

	if ( cmd == "list" )
	{
		mngr->DumpRecords(answer);
	}
	else if ( cmd == "add" )
	{
		const std::string& name = command.GetToken(eCmdParam2);

		int number = atoi(command.GetToken(eCmdParam3).c_str());

		sDNS_RECORD record;
		memset(&record,0,sizeof(record));
		strncpy(record.szHostName,name.c_str(),sizeof(record.szHostName)-1);
		record.wServiceId = 1;

		char domainName[PLCM_DNS_DOMAIN_NAME_SIZE];
		char ip[32];
		for ( int i=0; i<number; ++i )
		{
			snprintf(record.szDomainName,PLCM_DNS_DOMAIN_NAME_SIZE-1,"Domain%d",i%5+1);
			snprintf(ip,32,"123.123.%d.%d",i/16+1,i%16+1);

			record.sResolveIp.ipVersion = eIpVersion4;
			::stringToIpV4(&(record.sResolveIp),ip,eNetwork);

			answer << "\nadd:" << mngr->DnsInsertHostByName(record);
		}
	}
	else if ( cmd == "clean_by_name" )
	{
		const std::string& name = command.GetToken(eCmdParam2);
		WORD nServID = 0;
		nServID = atoi(command.GetToken(eCmdParam3).c_str());

		if(0 == nServID)
		{
			answer << "\nclean:" << mngr->DnsDeleteHostByNameAllDns(name.c_str());

			//{
			//	if(NULL == G_PlcmDns.m_pTTLService)
			//		G_PlcmDns.m_pTTLService = new cTTLService();

			//	G_PlcmDns.m_pTTLService->vClean();
			//	G_PlcmDns.m_pTTLService->vNearestUpdate();
			//}

		}
		else
		{
			answer << "\nclean:" << mngr->DnsDeleteHostByName(name.c_str(), nServID);
			{
				//if(NULL == G_PlcmDns.m_pTTLService)
				//	G_PlcmDns.m_pTTLService = new cTTLService();

				//G_PlcmDns.m_pTTLService->bDelete((char*)name.c_str(), nServID);
				//G_PlcmDns.m_pTTLService->vNearestUpdate();
			}
		}
	}
	else // ( cmd == "help" || cmd == "" )
	{
		answer << "available params:\n"
				<< "  list - dumps records list\n"
				<< "  add [host_name] [num] - creates NUM sim records with HOST_NAME\n"
				<< "  clean_by_name [host_name] <serv.ID>- removes all records by HOST_NAME\n"
				<< "  ?";
	}

    return STATUS_OK;
}

void  CDNSAgentManager::CreateDummyResolution()
{
}
