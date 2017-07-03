//+========================================================================+
//                         SipProxyConfCntl.cpp                            |
//            Copyright 2003 Polycom.									   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom.							   |
//-------------------------------------------------------------------------|
// FILE:       SipProxyConfCntl.cpp                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Ori P.                                                      |
// Date:	   5/5/03                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>

// includes section
#include  "SipProxyConfCntl.h"
#include  "SipProxyServiceManager.h"
#include  "SIPProxyStructs.h"
#include  "SipProxyApi.h"
#include  "IpCsOpcodes.h"
#include  "SystemFunctions.h"
#include  "SipUtils.h"
#include  "MplMcmsProtocol.h"
#include  "IpCommonDefinitions.h"
#include  "OpcodesMcmsInternal.h"
#include  "DefinesIpService.h"

#include  "StatusesGeneral.h"
#include  "MplMcmsProtocolTracer.h"
#include  "SipProxyMplMcmsProtocolTracer.h"
//#include  "IpPartyMonitorDefinitions.h"
#include  "ICEApiDefinitions.h"
#include  "IceCmReq.h"
#include "FaultsDefines.h"
#include "TraceStream.h"

#include "WrappersGK.h"
#include "IpService.h"
#include "SipProxyGlobals.h"

#include "ConfigHelper.h"
#include "ServiceConfigList.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "DefinesIpService.h"

// definitions


extern WORD GetPresentedConfNumber();
extern void DecreasePresentedConfNumber();
// general definition section



//+========================================================================+
//
// this file includes the following classes:
//
//
//+========================================================================+


//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// COneConfRegistration
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//===========================================================================

COneConfRegistration::COneConfRegistration()
{
	m_status = CONF_IDLE;
	m_confID = 0xFFFFFFFF;
	m_serviceId = 0;
//	m_serviceIp = 0;
//	memset(&m_serviceAddressIpV4, 0, sizeof(mcTransportAddress));
//	memset(&m_serviceAddressIpV6, 0, sizeof(mcTransportAddress));
	for(int i=0; i<TOTAL_NUM_OF_IP_ADDRESSES; i++)
	{
		memset(&m_serviceAddrList[i] ,0, sizeof(ipAddressStruct));
	}
	m_transportType = 0;
	m_ipType = eIpType_None;
	m_serversConfig = 0;
//	m_outboundProxyIP = 0;
	memset(&m_outboundProxyAddress, 0, sizeof(mcTransportAddress));
//	m_outboundProxyPort = 0;

//	m_proxyIP = 0;
	memset(&m_proxyAddress, 0, sizeof(mcTransportAddress));
//	m_proxyPort = 0;
	m_poutboundProxyName = new char[H243_NAME_LEN];
	m_pProxyName = new char[H243_NAME_LEN];
	m_pHostName = new char[H243_NAME_LEN];
	m_pConfName = new char[H243_NAME_LEN];
	m_expires = 0;
	m_move = FALSE;
	m_remove = FALSE;
	m_bIsDiscovery = FALSE;
	m_DNSStatus = NO;
	m_bIsOngoing = FALSE;
	m_bIsMR = FALSE;
	m_bIsEQ = FALSE;
	m_bIsFactory = FALSE;
	m_bIsGWProfile = FALSE;
	m_bIsMSIceUser = FALSE;
	m_bIsStandIceUser = FALSE;
	m_bIsFromOldService = FALSE;
	m_sipServerType = 0;
	m_lastRegStatus = eSipServerStatusNotAvailable;
}


COneConfRegistration::~COneConfRegistration()
{
	PDELETEA(m_poutboundProxyName);
	PDELETEA(m_pProxyName);
	PDELETEA(m_pHostName);
	PDELETEA(m_pConfName);
}
const char* COneConfRegistration:: NameOf() const
{
	return "COneConfRegistration";
}
/////////////////////////////////////////////////////////////////////////////
void COneConfRegistration::SetServiceAddresses(ipAddressStruct ipAddrSt, WORD index)
{
	memcpy(&m_serviceAddrList[index] ,&ipAddrSt, sizeof(ipAddressStruct));
}
/////////////////////////////////////////////////////////////////////////////
ipAddressStruct COneConfRegistration::GetServiceAddress(WORD index)
{
	return m_serviceAddrList[index];
}

//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// COneConf
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//===========================================================================



// definitions
#define TIMER_START_REGISTERING		100
#define TIMER_REFRESH_REGISTRATION	101
#define TIMER_START_UNREGISTER		102
#define TIMER_REFRESH_SUBSCRIBE		103
#define TIMER_DNS_RESOLVING			104

// temporary timeout definitions
#define	RegisteringTimeOut			180
#define FirstRegTout				40
#define	RefreshTimeOutDeviation		30
#define	UnRegisteringTimeOut		180
#define UnRegisteringDummyTimeOut	60
#define ResolvingTimeOut			60

//presence definitions
#define	START_PRESENCE_ONLINE		300
#define	START_PRESENCE_BUSY			301

// MS keep alive
const WORD MS_KEEP_ALIVE_CLIENT_TIMER      = 131;

// start message map -------------------------------------------
PBEGIN_MESSAGE_MAP(COneConf)

    // conf events
  ONEVENT(SIP_START_REGISTER ,			CONF_IDLE,				COneConf::OnStartRegisterIDLE	)
  ONEVENT(UNREGISTER,					CONF_IDLE,				COneConf::OnUnRegisterIDLE		)
  ONEVENT(AUTHENTICATION_REQ,			CONF_REGISTERING,		COneConf::OnAuthenticationRequest	)
  ONEVENT(AUTHENTICATION_REQ,			CONF_REGISTERED,		COneConf::OnAuthenticationRequest	)
  ONEVENT(TIMER_START_REGISTERING,		CONF_REGISTERING,		COneConf::OnRegisteringTimer		)
  ONEVENT(TIMER_START_REGISTERING,		CONF_REGISTERED,		COneConf::OnRegisteringTimer		)
  ONEVENT(REGISTERING_OK,				CONF_REGISTERING,		COneConf::OnRegisteringOKRegistering	)
  ONEVENT(REGISTERING_OK,				CONF_REGISTERED,		COneConf::OnRegisteringOKRegistered	)
  ONEVENT(REGISTERING_OK,				CONF_UNREGISTERING,		COneConf::OnRegisteringOKUnRegistering	)
  ONEVENT(TIMER_REFRESH_REGISTRATION,	CONF_REGISTERED,		COneConf::OnRefreshTimer	)
  ONEVENT(TIMER_REFRESH_REGISTRATION,	CONF_IDLE,				COneConf::OnRegisteringRefreshTimer	)
  ONEVENT(UNREGISTER,					CONF_REGISTERING,		COneConf::OnUnRegisterRequestRegistering	)
  ONEVENT(UNREGISTER,					CONF_REGISTERED,		COneConf::OnUnRegisterRequestRegistered	)
  ONEVENT(UNREGISTER,					CONF_UNREGISTERING,		COneConf::OnUnRegisterRequestUnRegistering	)
  ONEVENT(UNREGISTER,					CONF_IDLE_DNS_RESOLVING,			COneConf::OnUnRegisterRequestIdleDNSResolving	)
  ONEVENT(UNREGISTER,					CONF_REGISTERING_DNS_RESOLVING,		COneConf::OnUnRegisterRequestRegisteringDNSResolving	)
  ONEVENT(UNREGISTER,					CONF_REGISTERED_DNS_RESOLVING,		COneConf::OnUnRegisterRequestRegisteredDNSResolving	)
  ONEVENT(UNREGISTER,					CONF_UNREGISTERING_DNS_RESOLVING,	COneConf::OnUnRegisterRequestUnRegisteringDNSResolving	)

  ONEVENT(TIMER_START_UNREGISTER,		CONF_UNREGISTERING,		COneConf::OnUnRegisteringTimer	)
  ONEVENT(REGISTERING_FAILED,			CONF_REGISTERED,		COneConf::OnRegisteringFailedRegistered	)
  ONEVENT(REGISTERING_FAILED,			CONF_REGISTERING,		COneConf::OnRegisteringFailedRegistering	)
  ONEVENT(REGISTERING_FAILED,			CONF_UNREGISTERING,		COneConf::OnRegisteringFailedUnRegistering	)
  ONEVENT(REGISTERING_FAILED,			CONF_IDLE,				COneConf::NullActionFunction	)

  ONEVENT(SUBSCRIBING_OK,				CONF_REGISTERED,		COneConf::OnSubscribingOKRegistered	)
  ONEVENT(SUBSCRIBING_FAILED,			CONF_REGISTERED,		COneConf::OnSubscribingFailedRegistered	)
  ONEVENT(SUBSCRIBING_OK,				CONF_UNREGISTERING,		COneConf::NullActionFunction)
  ONEVENT(TIMER_REFRESH_SUBSCRIBE,		CONF_REGISTERED,		COneConf::OnSubscribRefreshTimer	)

  ONEVENT(NOTIFY_IND,					CONF_REGISTERED,		COneConf::OnNotifyIndRegistered	)

  ONEVENT(DNS_RESOLVE_IND,				CONF_IDLE,				COneConf::NullActionFunction )
  ONEVENT(DNS_RESOLVE_IND,				CONF_REGISTERED,		COneConf::NullActionFunction )
  ONEVENT(DNS_RESOLVE_IND,				CONF_REGISTERING,		COneConf::NullActionFunction )//rons
  ONEVENT(DNS_RESOLVE_IND,				CONF_IDLE_DNS_RESOLVING,			COneConf::OnDNSResolveIndConfIDLEResolving	)
  ONEVENT(DNS_RESOLVE_IND,				CONF_REGISTERING_DNS_RESOLVING,		COneConf::OnDNSResolveIndConfREGISTERINGResolving	)
  ONEVENT(DNS_RESOLVE_IND,				CONF_REGISTERED_DNS_RESOLVING,	 	COneConf::OnDNSResolveIndConfREGISTEREDResolving	)
  ONEVENT(DNS_RESOLVE_IND,				CONF_UNREGISTERING_DNS_RESOLVING,	COneConf::OnDNSResolveIndConfUNREGISTERINGResolving	)


  ONEVENT(DNS_SERVICE_IND,				CONF_IDLE,				COneConf::NullActionFunction )
  ONEVENT(DNS_SERVICE_IND,				CONF_REGISTERED,		COneConf::NullActionFunction )
  ONEVENT(DNS_SERVICE_IND,				CONF_IDLE_DNS_RESOLVING,			COneConf::OnDNSServiceIndConfIDLEResolving	)
  ONEVENT(DNS_SERVICE_IND,				CONF_REGISTERING_DNS_RESOLVING,		COneConf::OnDNSServiceIndConfREGISTERINGResolving	)
  ONEVENT(DNS_SERVICE_IND,				CONF_REGISTERED_DNS_RESOLVING,		COneConf::OnDNSServiceIndConfREGISTEREDResolving	)
  ONEVENT(DNS_SERVICE_IND,				CONF_UNREGISTERING_DNS_RESOLVING,	COneConf::OnDNSServiceIndConfUNREGISTERINGResolving	)

  ONEVENT(TIMER_DNS_RESOLVING,			CONF_IDLE_DNS_RESOLVING,			COneConf::OnDNSResolvingTimer	)
  ONEVENT(TIMER_DNS_RESOLVING,			CONF_REGISTERING_DNS_RESOLVING,		COneConf::OnDNSResolvingTimer	)
  ONEVENT(TIMER_DNS_RESOLVING,			CONF_REGISTERED_DNS_RESOLVING,		COneConf::OnDNSResolvingTimer	)
  ONEVENT(TIMER_DNS_RESOLVING,			CONF_UNREGISTERING_DNS_RESOLVING,	COneConf::OnDNSResolvingTimer	)


  ONEVENT(START_PRESENCE_ONLINE,		CONF_REGISTERED,			COneConf::OnStartPresenceOnlineRegistered)
  ONEVENT(START_PRESENCE_ONLINE,		CONF_UNREGISTERING,			COneConf::NullActionFunction)
  ONEVENT(START_PRESENCE_ONLINE,		CONF_IDLE,					COneConf::NullActionFunction )
  ONEVENT(START_PRESENCE_ONLINE,		CONF_REGISTERING,			COneConf::NullActionFunction )
  ONEVENT(START_PRESENCE_ONLINE,		CONF_IDLE_DNS_RESOLVING,	COneConf::NullActionFunction )

  ONEVENT(START_PRESENCE_BUSY,			CONF_REGISTERED,			COneConf::OnStartPresenceBusyRegistered)
  ONEVENT(START_PRESENCE_BUSY,			CONF_UNREGISTERING,			COneConf::NullActionFunction)
  ONEVENT(START_PRESENCE_BUSY,			CONF_IDLE,					COneConf::NullActionFunction )
  ONEVENT(START_PRESENCE_BUSY,			CONF_REGISTERING,			COneConf::NullActionFunction )
  ONEVENT(START_PRESENCE_BUSY,			CONF_IDLE_DNS_RESOLVING,	COneConf::NullActionFunction )

  // MS keep alive
  ONEVENT(MS_KEEP_ALIVE_CLIENT_TIMER,     CONF_REGISTERED,  COneConf::OnMsKeepAliveClientTimeOut )
  ONEVENT(MS_KEEP_ALIVE_CLIENT_TIMER,     CONF_IDLE,  COneConf::NullActionFunction )

//  ONEVENT(LOADACCEPTMSGACK,			ANYCASE,			COneConf::NullActionFunction	)

PEND_MESSAGE_MAP(COneConf,CStateMachine);
// end   message map -------------------------------------------

static CCloudInfo glbCloudInfo;
/////////////////////////////////////////////////////////////////////////////
COneConf::COneConf() : m_pSipMngrRcvMbx(NULL), m_data(0), m_indInMngrDB(MAX_CONF_REGISTRATIONS)
                    , m_timerRegister(0), m_timerRefresh(0), m_timerUnRegister(0)
                    , m_timerSubscribeRefresh(0), m_countFailedRegs(0), m_callId(NULL)
					, m_DNSResolvedIp(FALSE), m_proxyType(eUNKNOWN), m_src_unit_id(eSipBalancer), m_busyServer(FALSE)
                    , m_pMockMplProtocol(NULL), m_proxyVersionType(eUnknownServerVersion), m_presenceState(SIP_PRESENCE_OFFLINE)
                    , m_presenceNeedToRepublish(FALSE), m_dwKeepAliveFrequencyConf_Sec(0), m_bIsCrlfRegistration(FALSE)
{
	m_state = CONF_IDLE;
//	m_pLoadMngrConnector = NULL;
}

/////////////////////////////////////////////////////////////////////////////
COneConf::~COneConf()
{

	m_data->m_serviceId = (DWORD)-1;
	m_data->m_status = CONF_IDLE;

	if (m_timerRegister)
		DeleteTimer( TIMER_START_REGISTERING );

	if (m_timerRefresh)
		DeleteTimer( TIMER_REFRESH_REGISTRATION );

	if (m_timerUnRegister)
		DeleteTimer( TIMER_START_UNREGISTER );

//	DestroyTimer();
    DeleteAllTimers();

	DEALLOCBUFFER(m_callId);

/*	if (m_pLoadMngrConnector->IsRequest())
		m_pLoadMngrConnector->ForcedReleaseAllCriticalSections();

	POBJDELETE(m_pLoadMngrConnector);*/
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
const char* COneConf:: NameOf() const
{
	return "COneConf";
}
/////////////////////////////////////////////////////////////////////////////
void*  COneConf::GetMessageMap()
{
  return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DWORD   Id=0;

	*pMsg >> Id;
	DispatchEvent( opCode, pMsg );
}


/////////////////////////////////////////////////////////////////////////////
void  COneConf::HandleEventByIndex(OPCODE opcode, CSegment* pMsg)
{
//	DispatchEvent( opcode, m_msgEntries, pMsg );
	DispatchEvent( opcode, pMsg );
}

/////////////////////////////////////////////////////////////////////////////
int  COneConf::Create( COneConfRegistration *data, unsigned int indInDB, COsQueue* pRcvMbx
					  , CMplMcmsProtocol* pMockMplProtocol)
{
	PTRACE(eLevelInfoNormal, "COneConf::Create");

	m_pSipMngrRcvMbx = pRcvMbx;
	m_data = data;	// gets the conf DB pointer
	if (!data)
	{
		PASSERT(1);
		return 1;
	}

	if(indInDB >= MAX_CONF_REGISTRATIONS)
	{
		PASSERT(2);
		return 2;
	}

	m_indInMngrDB = indInDB;
	m_callId = new char[MaxLengthOfSipCallId];
	m_pMockMplProtocol = pMockMplProtocol;
	//m_PartiesWaiting.clear();
	memset( m_GruuContact, 0, sizeof(m_GruuContact));

/*	m_pLoadMngrConnector = new CSIPProxyLoadMngrConnector((CParty*)NULL, m_data->m_serviceId, SIP_PROXY_ENTRY);
	m_pLoadMngrConnector->SetSipProxyMngrMbx(m_pSipMngrRcvMbx);*/

	// Init timer only if not under TDD
//	if(!m_pMockMplProtocol)
//		InitTimer(*m_pSipMngrRcvMbx);

	return 0;    // OK
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::StartRegister(DWORD delay)
{


	if (delay == 0)
	{
		if(IsValidTimer(TIMER_REFRESH_REGISTRATION))
		{
			DeleteTimer(TIMER_REFRESH_REGISTRATION);
			m_timerRefresh = 0;
		}
		DispatchEvent(SIP_START_REGISTER);
	}
	else
	{
		COstrStream msg;
		msg << "Conf=" << m_data->m_pConfName << " Register delay=" << delay;
		PTRACE2(eLevelInfoNormal, "COneConf::StartRegister : ", msg.str().c_str());
		if(IsValidTimer(TIMER_REFRESH_REGISTRATION))
			DeleteTimer(TIMER_REFRESH_REGISTRATION);
		StartTimer(TIMER_REFRESH_REGISTRATION, delay); // delay is in TICKS
		m_timerRefresh = 1;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::Unregister()
{
//	DispatchEvent(UNREGISTER,GetMessageMap(),NULL);
	DispatchEvent(UNREGISTER);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::StopReg()
{
	//m_data->m_serviceId = -1;
	//m_data->m_status = CONF_IDLE;

	if (m_timerRegister)
	{
		DeleteTimer( TIMER_START_REGISTERING );
		m_timerRegister = 0;
	}
	if (m_timerRefresh)
	{
		DeleteTimer( TIMER_REFRESH_REGISTRATION );
		m_timerRefresh = 0;
	}
	if (m_timerUnRegister)
	{
		DeleteTimer( TIMER_START_UNREGISTER );
		m_timerUnRegister = 0;
	}
	//DestroyTimer();
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::ResetConfCntl()
{
	m_state = CONF_IDLE;
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::SetBusyServer(BYTE busyServer)
{
	if (m_busyServer != busyServer)
	{
		COstrStream msg;
		msg << "Conf=" << m_data->m_pConfName << " Busy=" << (busyServer ? "true":"false");
		PTRACE2(eLevelInfoNormal, "COneConf::SetBusyServer : ", msg.str().c_str());

		m_busyServer = busyServer;
	}
}

/////////////////////////////////////////////////////////////////////////////
BYTE  COneConf::IsBusyServer()
{
	return m_busyServer;
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::HandleBusyServer(BYTE bRegister)
{
	PTRACE2(eLevelInfoNormal, "COneConf::HandleBusyServer, conf = ",m_data->m_pConfName);

	StopReg();  // stop timers of register and unregister.

	if (bRegister)
	{
		m_state = CONF_IDLE;
		m_data->m_status = CONF_IDLE;
	}
	else  // Unregister
	{
		m_state = CONF_REGISTERED;
		m_data->m_status = CONF_REGISTERED;

		m_data->m_remove = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
char* COneConf::GetCallId() const
{
	return m_callId;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  COneConf::GetId()
{
    // Sagi ???
	DWORD Id = (m_indInMngrDB<<16);// + (0x0000ffff & m_statMachId);
	return Id;
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::GetAddressFromList(mcTransportAddress* resolvedAddr,ipAddressStruct* pAddrList,eIpType ipType)
{
	char a[128];

	if (eIpType_Both == ipType || eIpType_None == ipType)
	{
		TRACEINTO << "(0=None, 3=both) ipType: " << ipType;
		//resolvedAddr->addr = pAddrList[0].addr;
		memcpy(&(resolvedAddr->addr), &(pAddrList[0].addr), sizeof(ipAddressIf));
		resolvedAddr->ipVersion = pAddrList[0].ipVersion;
	}
	else if (eIpType_IpV4 == ipType || eIpType_IpV6 == ipType)
	{
		for(int i=0; i< TOTAL_NUM_OF_IP_ADDRESSES; i++)
		{
			eIpType typeFromAddrList = IpVersionToIpType( (enIpVersion)(pAddrList[i].ipVersion) );
			TRACEINTO << "(1=V4, 2=V6) ipType: " << ipType << ", typeFromAddrList:" << typeFromAddrList;
					 // << ",(0=V4, 1=V6) pAddrList[i].ipVersion:" << pAddrList[i].ipVersion;
			if(typeFromAddrList == ipType)
			{
				memcpy(&(resolvedAddr->addr), &(pAddrList[i].addr), sizeof(ipAddressIf));//resolvedAddr->addr = pAddrList[i].addr;
				resolvedAddr->ipVersion = pAddrList[i].ipVersion;
				memset(a, 0 ,sizeof(a));
				TRACEINTO << ipToString(*resolvedAddr, a, 1);
				break;
			}
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnStartRegisterIDLE(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnStartRegisterIDLE, conf = ",m_data->m_pConfName);
	PTRACE2INT(eLevelInfoNormal, "COneConf::OnStartRegisterIDLE, dns status = ",m_data->m_DNSStatus);

	// --------- need to get the RcvMbx--------

	if(eConfSipServerManually == m_data->m_serversConfig)
	{
		//if(0 == m_data->m_proxyAddress.addr.v4.ip)
		if(isApiTaNull(&(m_data->m_proxyAddress)))
		{
			PTRACE(eLevelInfoNormal, "COneConf::OnStartRegisterIDLE, ProxyIp=0, Not sending REGISTER request.");
			if(eServerStatusSpecify == m_data->m_DNSStatus || eServerStatusAuto == m_data->m_DNSStatus)//rons
			//if(m_data->m_DNSStatus)			//rons
			{
				if(strcmp(m_data->m_pProxyName, ""))
				{
					ResolveDomainReq(m_data->m_serviceId, m_data->m_pProxyName);
					m_state = CONF_IDLE_DNS_RESOLVING;
				}
				else
					PTRACE(eLevelError, "COneConf::OnStartRegisterIDLE, ProxyName = "", Not sending REGISTER request.");
			}
		}
		if(CONF_IDLE_DNS_RESOLVING != m_state && isApiTaNull(&(m_data->m_outboundProxyAddress)) )
		{
			if(strcmp(m_data->m_poutboundProxyName, ""))
			{
				if(strncmp(m_data->m_poutboundProxyName, m_data->m_pProxyName, H243_NAME_LEN))
				{
					PTRACE(eLevelInfoNormal, "COneConf::OnStartRegisterIDLE, outboundProxyIP=0, Not sending REGISTER request.");
					if(m_data->m_DNSStatus)
					{
						ResolveDomainReq(m_data->m_serviceId, m_data->m_poutboundProxyName);
						m_state = CONF_IDLE_DNS_RESOLVING;
					}
				}
				//outbound proxy and registrar are the same
				else
				{
					if (m_data->m_proxyAddress.ipVersion == eIpVersion4)
						m_data->m_outboundProxyAddress.addr.v4.ip = m_data->m_proxyAddress.addr.v4.ip;
					else if (m_data->m_proxyAddress.ipVersion == eIpVersion6)
					{
					//	m_data->m_outboundProxyAddress.addr.v6.ip = m_data->m_proxyAddress.addr.v6.ip;
						memcpy(m_data->m_outboundProxyAddress.addr.v6.ip, m_data->m_proxyAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
						m_data->m_outboundProxyAddress.addr.v6.scopeId = m_data->m_proxyAddress.addr.v6.scopeId;
					}
					m_data->m_outboundProxyAddress.ipVersion = m_data->m_proxyAddress.ipVersion;
				}

			}
			else
			{
				PTRACE(eLevelError, "COneConf::OnStartRegisterIDLE, OutboundProxyName = "", Not sending REGISTER request.");
				if(m_data->m_proxyAddress.ipVersion == eIpVersion4)
					m_data->m_outboundProxyAddress.addr.v4.ip = m_data->m_proxyAddress.addr.v4.ip;
				else if(m_data->m_proxyAddress.ipVersion == eIpVersion6)
				{
					memcpy(m_data->m_outboundProxyAddress.addr.v6.ip, m_data->m_proxyAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
					m_data->m_outboundProxyAddress.addr.v6.scopeId = m_data->m_proxyAddress.addr.v6.scopeId;
				}
				m_data->m_outboundProxyAddress.ipVersion = m_data->m_proxyAddress.ipVersion;
			}
		}
	}
	else
		if(isApiTaNull(&(m_data->m_proxyAddress)) && eConfSipServerAuto == m_data->m_serversConfig)
		{
			if(m_data->m_DNSStatus)
			{
				if(strcmp(m_data->m_pHostName, ""))
				{
					ServiceReq(m_data->m_serviceId, m_data->m_pHostName, eIPProtocolType_SIP, m_data->m_transportType);
					m_state = CONF_IDLE_DNS_RESOLVING;
				}
				else
					PTRACE(eLevelError, "COneConf::OnStartRegisterIDLE, HostName = "", Not sending REGISTER request.");
			}
			//otherwise, try to get data from DHCP
			else
			{
				if(CheckDHCPData(m_data))
					OnStartRegisterIDLE(pParam);
				else
					PTRACE(eLevelError, "COneConf::OnStartRegisterIDLE, Failed on DHCP data, Not sending REGISTER request.");
			}
		}

	if(CONF_IDLE_DNS_RESOLVING == m_state)
	{
		m_data->m_status = CONF_REGISTERING;
		StartTimer(TIMER_DNS_RESOLVING, ResolvingTimeOut*SECOND);
		return;
	}

	if(isApiTaNull(&(m_data->m_proxyAddress)) || isApiTaNull(&(m_data->m_outboundProxyAddress)) )
		return;

	PTRACE2(eLevelInfoNormal, "COneConf::OnStartRegisterIDLE, registering conf = ",m_data->m_pConfName);

	m_state = CONF_REGISTERING;
	m_data->m_status = CONF_REGISTERING;
	if (!RegisterRequest(m_data->m_expires))
		return;

	DWORD tout = RegisteringTimeOut*SECOND;
	if(eTransportTypeTcp == m_data->m_transportType)
		tout = FirstRegTout*SECOND;
	else
		m_countFailedRegs = 3;
	// start registering timer
	StartTimer(TIMER_START_REGISTERING, tout);
	m_timerRegister = 1;
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnUnRegisterIDLE(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnUnRegisterIDLE, conf = ",m_data->m_pConfName);
	m_state = CONF_UNREGISTERING;
	m_data->m_status = CONF_UNREGISTERING;
	if(m_DNSResolvedIp)
		ClearResolvedIps();

	if(m_data->m_bIsDiscovery)
	{
		OnUnRegisterRequestRegistered(pParam);
	}
	else
	{
		SIPProxyMngrApi sipProxyMngrApi;
		sipProxyMngrApi.CreateOnlyApi(*m_pSipMngrRcvMbx);
		sipProxyMngrApi.KillOneConf(m_indInMngrDB);
		sipProxyMngrApi.DestroyOnlyApi();
	}
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnAuthenticationRequest(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnAuthenticationRequest, conf = ",m_data->m_pConfName);
	if (m_timerRegister)
	{
		DeleteTimer( TIMER_START_REGISTERING );
		m_timerRegister = 0;
	}

	// do something, Authentication ...
	// ....

	// send Authentication
	// ....

	// start registering timer
	StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
	m_timerRegister = 1;

}


/////////////////////////////////////////////////////////////////////////////
//This method is used in the first time we register a conf.
void  COneConf::OnRegisteringOKRegistering(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringOKRegistering, conf = ",m_data->m_pConfName);

//	DWORD Id = GetId();
//	m_pLoadMngrConnector->RegisterExitCriticalSection(Id);

	m_countFailedRegs = 3; //mark that we succeeded to contact registrar (switch problem -  overcome)

    *pParam >>	m_src_unit_id;

	mcIndRegisterResp * pRegResponseMsg = (mcIndRegisterResp *)pParam->GetPtr(1);
	DWORD expires = pRegResponseMsg->expires;
	if(expires!=0 && RefreshTimeOutDeviation + 10 < expires)
		m_data->m_expires = expires;// - RefreshTimeOutDeviation - GetRandomNumber();
	else
		m_data->m_expires = 4 * RefreshTimeOutDeviation;

	sipHeaderElement * pCurHeaderElement = (sipHeaderElement *)pRegResponseMsg->sipHeaders.headersList;
	char * pFirstHeader = pRegResponseMsg->sipHeaders.headersList + pRegResponseMsg->sipHeaders.numOfHeaders * sizeof(sipHeaderElement);
	char * pCurHeader = pFirstHeader;

	for (int i=0; i<pRegResponseMsg->sipHeaders.numOfHeaders; i++)
	{
		SetProxyServerType(pCurHeaderElement->eHeaderField,pCurHeader);
		SetProxyServerVersionType(pCurHeaderElement->eHeaderField,pCurHeader);
		SetMsKeepAliveTout(pCurHeaderElement->eHeaderField,pCurHeader);
		SetGruuContact(pCurHeaderElement->eHeaderField,pCurHeader);
		pCurHeaderElement += 1; //+ sizeof(sipHeaderElement)
		pCurHeader = pFirstHeader + pCurHeaderElement->position;
	}

	if (m_timerRegister)
	{
		DeleteTimer( TIMER_START_REGISTERING );	// delete registering timer
		m_timerRegister = 0;
	}

	m_state = CONF_REGISTERED;

	// update conf DB
	// ....
	m_data->m_status = CONF_REGISTERED;
	m_data->m_lastRegStatus	= eSipServerStatusTypeOk;

	if(m_dwKeepAliveFrequencyConf_Sec > 0)
    {
	    StartTimer(MS_KEEP_ALIVE_CLIENT_TIMER, m_dwKeepAliveFrequencyConf_Sec/2 * SECOND);
         TRACEINTO << "COneConf::OnRegisteringOKRegistering - Start Timer of CRLF*2. Frequency:" << m_dwKeepAliveFrequencyConf_Sec << "\n";
    }
	m_bIsCrlfRegistration = FALSE;

	if(m_data->m_remove)
	{
		PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringOKRegistering - Remove, conf = ",m_data->m_pConfName);
		OnUnRegisterRequestRegistered(pParam);

	}
	else
	{
		PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringOKRegistering - Start timer, conf = ",m_data->m_pConfName);
        int ticks = (m_data->m_expires - RefreshTimeOutDeviation - GetRandomNumber()) * SECOND;
        StartTimer(TIMER_REFRESH_REGISTRATION, ticks);
        m_timerRefresh = 1;
	}

	SetRegisterActiveAlarm(FALSE);
	if(!(m_data->m_remove))
		CallSubscribe(TRUE);


}
/////////////////////////////////////////////////////////////////////////////
void COneConf::SetProxyServerType(APIS8 HeaderField,char* Header)
{
	if(kAllowEvent == HeaderField)
		if(strstr(Header, "microsoft"))
		{
			m_proxyType = eLCS;
		}
}
/////////////////////////////////////////////////////////////////////////////
void COneConf::SetProxyServerVersionType(APIS8 HeaderField,char* Header)
{
	if(kUserAgent == HeaderField) {
		if(strstr(Header, "RTC/4.0") || strstr(Header, "RTC/5.0"))
		{
			m_proxyVersionType = eLync;
		}
		if(strstr(Header, "RTC/3.5"))
		{
			m_proxyVersionType = eW13;
		}
	}
}

//-------TCP-KEEP-ALIVE ---------------------------------------------------------------------//
//==================================================================//
void   COneConf::crlf_vGetKaParametersFromFormatedString(  char  * szMsKeepAlive_Data
                                                         , DWORD * pKa_Frequency
                                                         , DWORD * pnKa_PongTimeout
                                                         , DWORD * pnKa_Behavior
                                                         , DWORD * pnKa_Tipe)
{
    if(NULL != szMsKeepAlive_Data)
    {
        char * apStr[4];
        int    nDataLen     = strlen(szMsKeepAlive_Data);
        int    nInd         = 0;
        int    nI           = 0;

        apStr[0]= szMsKeepAlive_Data;
        apStr[1]= NULL;
        apStr[2]= NULL;
        apStr[3]= NULL;

        for(nInd = 0; nInd < nDataLen; nInd++)
        {
            if('|' == szMsKeepAlive_Data[nInd])
            {
                szMsKeepAlive_Data[nInd] ='\0';
                nI++;
                apStr[nI] = &szMsKeepAlive_Data[nInd+1];
            }
        }

        if(NULL != apStr[0])
            *pKa_Frequency = atoi(apStr[0]);
        if(NULL != apStr[1])
            *pnKa_PongTimeout = atoi(apStr[1]);
        if(NULL != apStr[2])
            *pnKa_Behavior = atoi(apStr[2]);
        if(NULL != apStr[3])
            *pnKa_Tipe = atoi(apStr[3]);
    }
}
//==================================================================//
/////////////////////////////////////////////////////////////////////////////
void COneConf::SetMsKeepAliveTout(APIS8 HeaderField,char* Header)
{
  if(kMsKeepAliveTimeout == HeaderField)
  {
      this->crlf_vGetKaParametersFromFormatedString(
          Header, &this->m_dwKeepAliveFrequencyConf_Sec, &this->m_MsKeepAlivePongTimeOutConf
          , &this->m_dwKeppAliveBehaviorConf, &this->m_dwKeepAliveTypeConf);
      TRACEINTO << "COneConf::SetMsKeepAliveTout  TCP-KEEP-ALIVE parameters: " << m_dwKeepAliveFrequencyConf_Sec << "|" << m_MsKeepAlivePongTimeOutConf << "|" << m_dwKeppAliveBehaviorConf << "|" << m_dwKeepAliveTypeConf << "\n";
  }
}

/////////////////////////////////////////////////////////////////////////////
void COneConf::SetGruuContact(APIS8 HeaderField, std::string Header)
{
  if (kContact == HeaderField)
  {
	  std::string sContact;

	  CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey("SIP_CONTACT_OVERRIDE_STR", sContact);

      TRACEINTO << "Contact: " << Header.c_str();
      TRACEINTO << "gruu sContact: " << sContact.c_str();

	  std::size_t psStart;
	  memset( m_GruuContact, 0, sizeof(m_GruuContact));



	  if (strncmp(sContact.c_str(), "sip:", 4) == 0)
	  {
		  std::string tmp = sContact.substr(4, MaxAddressListSize);
		  //strip sip: from string 256 is max gruu size
		  strncpy(m_GruuContact, tmp.c_str(), sizeof(m_GruuContact) - 1);
	  }
	  else
	  {
		  strncpy(m_GruuContact, sContact.c_str(), sizeof(m_GruuContact) - 1);
	  }

	  m_GruuContact[sizeof(m_GruuContact) - 1] = '\0';


	  TRACEINTO << "ContactGruu: " << m_GruuContact;

  }

}

////////////////////////////////////////////////////////////////////////////////
void COneConf::OnMsKeepAliveClientTimeOut(CSegment *pSeg)
{
	SendCrlfMessageNow();
    if(m_dwKeepAliveFrequencyConf_Sec > 0)
	    StartTimer(MS_KEEP_ALIVE_CLIENT_TIMER, m_dwKeepAliveFrequencyConf_Sec/2 * SECOND);
}

/////////////////////////////////////////////////////////////////////////////
void COneConf::OnMsKeepAliveToutErrInd(DWORD errCode)
{
  TRACEINTO << "COneConf::OnMsKeepAliveToutErrInd \n";
  if (IsValidTimer(TIMER_REFRESH_REGISTRATION))
    DeleteTimer(TIMER_REFRESH_REGISTRATION);

  if(m_dwKeepAliveFrequencyConf_Sec)
	{
      if(IsValidTimer(MS_KEEP_ALIVE_CLIENT_TIMER) )
      {
	    DeleteTimer(MS_KEEP_ALIVE_CLIENT_TIMER);
      }
      m_dwKeepAliveFrequencyConf_Sec = 0;
      m_MsKeepAlivePongTimeOutConf   = 0;
      m_dwKeppAliveBehaviorConf      = 0;
      m_dwKeepAliveTypeConf          = 0;
	}
  m_bIsCrlfRegistration = TRUE;
  StartTimer(TIMER_REFRESH_REGISTRATION, 30 * SECOND);
  TRACEINTO << "COneConf::OnMsKeepAliveToutErrInd. Re-Regisstration after [" << 30 <<"] seconds\n";
 //CSegment seg;
  //OnRefreshTimer(&seg);
}

/////////////////////////////////////////////////////////////////////////////
void COneConf::SendCrlfMessageNow()
{
	  mcReqSendCrlf* pMsg = new mcReqSendCrlf;
	  size_t size = sizeof(mcReqSendCrlf);
	  memset(pMsg, 0, size);
	  pMsg->dwMsKepAliveTimeOut_Sec = m_dwKeepAliveFrequencyConf_Sec;
	  pMsg->dwConfId = GetId();
	  if(m_data->m_proxyAddress.ipVersion == eIpVersion4)
	    pMsg->registrarTransportAddr.transAddr.addr.v4.ip = m_data->m_proxyAddress.addr.v4.ip;
	  else if(m_data->m_proxyAddress.ipVersion == eIpVersion6)
	  {
	    memcpy(pMsg->registrarTransportAddr.transAddr.addr.v6.ip, m_data->m_proxyAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
	    pMsg->registrarTransportAddr.transAddr.addr.v6.scopeId = m_data->m_proxyAddress.addr.v6.scopeId;
	  }
	  pMsg->registrarTransportAddr.transAddr.port = m_data->m_proxyAddress.port;
	  pMsg->registrarTransportAddr.transAddr.transportType = m_data->m_transportType;
	  pMsg->registrarTransportAddr.transAddr.ipVersion = m_data->m_proxyAddress.ipVersion;//eIpVersion4;
	  pMsg->registrarTransportAddr.transAddr.distribution = eDistributionUnicast;
	  pMsg->registrarTransportAddr.unionProps.unionType = m_data->m_proxyAddress.ipVersion;//eIpVersion4;
	  pMsg->registrarTransportAddr.unionProps.unionSize = 0x14;

	  CSegment* pSegToSend = new CSegment;
	  pSegToSend->Put((BYTE*)pMsg, size);
	  SendMsgToCS(SIP_CS_PROXY_SEND_CRLF_REQ, pSegToSend);

	  PDELETE(pSegToSend);
	  PDELETE(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void COneConf::SendCrlfMessageOnTransportError()
{
    //if( m_data->m_sipServerType == eSipServer_ms )
    {
		if( IsValidTimer(MS_KEEP_ALIVE_CLIENT_TIMER) )
        {
		  DeleteTimer(MS_KEEP_ALIVE_CLIENT_TIMER);
        }
        //m_dwKeepAliveFrequencyConf_Sec  = 0;
        m_MsKeepAlivePongTimeOutConf    = 0;
        m_dwKeppAliveBehaviorConf       = 0;
        m_dwKeepAliveTypeConf           = 0;

		SendCrlfMessageNow();
        if(m_dwKeepAliveFrequencyConf_Sec)
		    StartTimer(MS_KEEP_ALIVE_CLIENT_TIMER, m_dwKeepAliveFrequencyConf_Sec/2 * SECOND);
    }
}



/////////////////////////////////////////////////////////////////////////////
void COneConf::CallSubscribe(BOOL IsFirstRegisteration)
{

	if(IsFirstRegisteration)
	{
		if(eLCS == m_proxyType)
		{
			Subscribe("vnd-microsoft-roaming-ACL", "application/vnd-microsoft-roaming-acls+xml", 1800);
			SystemSleep(100*1);
			ServicePresence(eONLINE);
		}
		else
			PTRACE(eLevelInfoNormal, "COneConf::OnRegisteringOKRegistering, Not an LCS proxy. ");
	}

}
/////////////////////////////////////////////////////////////////////////////
//This method is used each time we refresh an existing registry.
void  COneConf::OnRegisteringOKRegistered(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringOKRegistered, conf = ",m_data->m_pConfName);

//	DWORD Id = GetId();

	*pParam >>	m_src_unit_id;
//	m_pLoadMngrConnector->RegisterExitCriticalSection(Id);

	mcIndRegisterResp * pRegResponseMsg = (mcIndRegisterResp *)pParam->GetPtr(1);
	DWORD expires = pRegResponseMsg->expires;
	if(expires!=0 && RefreshTimeOutDeviation + 10 < expires)
		m_data->m_expires = expires;// - RefreshTimeOutDeviation - GetRandomNumber();
	else
		m_data->m_expires = 4 * RefreshTimeOutDeviation;

  sipHeaderElement * pCurHeaderElement = (sipHeaderElement *)pRegResponseMsg->sipHeaders.headersList;
  char * pFirstHeader = pRegResponseMsg->sipHeaders.headersList + pRegResponseMsg->sipHeaders.numOfHeaders * sizeof(sipHeaderElement);
  char * pCurHeader = pFirstHeader;

  for (int i=0; i<pRegResponseMsg->sipHeaders.numOfHeaders; i++)
  {
    SetMsKeepAliveTout(pCurHeaderElement->eHeaderField,pCurHeader);
	SetGruuContact(pCurHeaderElement->eHeaderField,pCurHeader);

    pCurHeaderElement += 1; //+ sizeof(sipHeaderElement)
    pCurHeader = pFirstHeader + pCurHeaderElement->position;
  }

	if (m_timerRegister)
	{
		DeleteTimer( TIMER_START_REGISTERING );	// delete registering timer
		m_timerRegister = 0;
	}

	if(m_dwKeepAliveFrequencyConf_Sec > 0)
    {
	    StartTimer(MS_KEEP_ALIVE_CLIENT_TIMER, m_dwKeepAliveFrequencyConf_Sec/2 * SECOND);
        TRACEINTO << "COneConf::OnRegisteringOKRegistered - Start Timer of CRLF*2. Frequency:" << m_dwKeepAliveFrequencyConf_Sec << "\n";
    }
	m_bIsCrlfRegistration = FALSE;


	if(m_data->m_remove)
	{	OnUnRegisterRequestRegistered(pParam);
		PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringOKRegistered - Remove, conf = ",m_data->m_pConfName);
	}
	else
	{
		// start refresh registration timer
		StartTimer(TIMER_REFRESH_REGISTRATION, (m_data->m_expires - RefreshTimeOutDeviation - GetRandomNumber()) * SECOND);
		m_timerRefresh = 1;
		PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringOKRegistered - Start timer, conf = ",m_data->m_pConfName);
	}
	m_data->m_lastRegStatus	= eSipServerStatusTypeOk;
	SetRegisterActiveAlarm(FALSE);

	CallSubscribe(FALSE);

	OnRegisterEmptyWaitingPartiesVector(1);
}

/////////////////////////////////////////////////////////////////////////////
//time-out, registering failed
void  COneConf::OnRegisteringTimer(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringTimer, failed to register conf = ",m_data->m_pConfName);

	//this counter is for overcoming 'slow' switch problem, it enables to try register
	//again in short tout if no answer for register was recieved.
	//Short tout is a chance given only twice
	if(m_countFailedRegs<2)
		m_countFailedRegs++;

	if (m_timerRegister)
	{
		DeleteTimer( TIMER_START_REGISTERING );
		m_timerRegister = 0;

		if(m_DNSResolvedIp)
			ClearResolvedIps();
	}

	SIPProxyMngrApi sipProxyMngrApi;
	sipProxyMngrApi.CreateOnlyApi(*m_pSipMngrRcvMbx);
	if (m_data->m_bIsFromOldService != TRUE)
	{
		eSipRegistrationConfType sipConfType;
		sipConfType = SipProxyServiceManagerGetConfType(m_data);

		sipProxyMngrApi.RegistrarStatusUpdate(STATUS_FAIL, m_data->m_pProxyName, m_data->m_serviceId, m_data->m_proxyAddress, m_data->m_confID, (WORD)sipConfType, m_data->m_expires);
	}
	if(m_data->m_remove)
	{
		m_state = CONF_UNREGISTERING;
		m_data->m_status = CONF_UNREGISTERING;

		sipProxyMngrApi.KillOneConf(m_indInMngrDB);
		sipProxyMngrApi.DestroyOnlyApi();
	}
	//try to register again
	else
	{
		sipProxyMngrApi.DestroyOnlyApi();
		m_presenceNeedToRepublish = TRUE;
		if(eConfSipServerManually == m_data->m_serversConfig)
		{
			if(isApiTaNull(&(m_data->m_proxyAddress)))
			{
				PTRACE(eLevelInfoNormal, "COneConf::OnRegisteringTimer, ProxyIp=0, Not sending REGISTER request.");
				if(m_data->m_DNSStatus)
				{
					if(strcmp(m_data->m_pProxyName, ""))
					{
						ResolveDomainReq(m_data->m_serviceId, m_data->m_pProxyName);
						m_state = CONF_REGISTERING_DNS_RESOLVING;
					}
					else
						PTRACE(eLevelError, "COneConf::OnRegisteringTimer, ProxyName = "", Not sending REGISTER request.");
				}
			}
			if(CONF_REGISTERING_DNS_RESOLVING != m_state && isApiTaNull(&(m_data->m_outboundProxyAddress)))
			{
				if(strcmp(m_data->m_poutboundProxyName, ""))
				{
					if(strncmp(m_data->m_poutboundProxyName, m_data->m_pProxyName, H243_NAME_LEN))
					{
						PTRACE(eLevelInfoNormal, "COneConf::OnRegisteringTimer, outboundProxyIP=0, Not sending REGISTER request.");
						if(m_data->m_DNSStatus)
						{
							ResolveDomainReq(m_data->m_serviceId, m_data->m_poutboundProxyName);
							m_state = CONF_REGISTERING_DNS_RESOLVING;
						}
					}
					//outbound proxy and registrar are the same
					else
					{
						if (m_data->m_proxyAddress.ipVersion == eIpVersion4)
							m_data->m_outboundProxyAddress.addr.v4.ip = m_data->m_proxyAddress.addr.v4.ip;
						else if(m_data->m_proxyAddress.ipVersion == eIpVersion6)
						{
							memcpy(m_data->m_outboundProxyAddress.addr.v6.ip, m_data->m_proxyAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
							m_data->m_outboundProxyAddress.addr.v6.scopeId = m_data->m_proxyAddress.addr.v6.scopeId;
						}
						m_data->m_outboundProxyAddress.ipVersion = m_data->m_proxyAddress.ipVersion;
					}
				}
				else
				{
					PTRACE(eLevelError, "COneConf::OnRegisteringTimer, OutboundProxyName = "", Not sending REGISTER request.");
					if(m_data->m_proxyAddress.ipVersion == eIpVersion4)
						m_data->m_outboundProxyAddress.addr.v4.ip = m_data->m_proxyAddress.addr.v4.ip;
					else if(m_data->m_proxyAddress.ipVersion == eIpVersion6)
					{
						memcpy(m_data->m_outboundProxyAddress.addr.v6.ip, m_data->m_proxyAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
						m_data->m_outboundProxyAddress.addr.v6.scopeId = m_data->m_proxyAddress.addr.v6.scopeId;
					}
					m_data->m_outboundProxyAddress.ipVersion = m_data->m_proxyAddress.ipVersion;
				}
			}
		}
		else
			if(isApiTaNull(&m_data->m_proxyAddress) && eConfSipServerAuto == m_data->m_serversConfig)
			{
				if(m_data->m_DNSStatus)
				{
					if(strcmp(m_data->m_pHostName, ""))
					{
						ServiceReq(m_data->m_serviceId, m_data->m_pHostName, eIPProtocolType_SIP, m_data->m_transportType);
						m_state = CONF_REGISTERING_DNS_RESOLVING;
					}
					else
						PTRACE(eLevelError, "COneConf::OnRegisteringTimer, HostName = "", Not sending REGISTER request.");
				}
				//otherwise, try to get data from DHCP
				else
				{
					if(CheckDHCPData(m_data))
						OnRegisteringTimer(pParam);
				}
			}

		if(CONF_REGISTERING_DNS_RESOLVING == m_state)
		{
			m_data->m_status = CONF_REGISTERING;
			StartTimer(TIMER_DNS_RESOLVING, ResolvingTimeOut*SECOND);
			return;
		}

		if(isApiTaNull(&m_data->m_proxyAddress))
			return;

		DWORD tout = RegisteringTimeOut*SECOND;

		if(CONF_REGISTERING == m_data->m_status && m_countFailedRegs < 3)
			if(eTransportTypeTcp == m_data->m_transportType)
				tout = 2*FirstRegTout*SECOND;

		m_state = CONF_REGISTERING;
		m_data->m_status = CONF_REGISTERING;
		if (!RegisterRequest(m_data->m_expires))
			return;
		// start registering timer
		StartTimer(TIMER_START_REGISTERING, tout);
		m_timerRegister = 1;
	}
}


/////////////////////////////////////////////////////////////////////////////
//refresh time-out, reregister
void  COneConf::OnRegisteringRefreshTimer(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringRefreshTimer, conf = ",m_data->m_pConfName);

	if(m_timerRefresh)
	{
		m_state = CONF_REGISTERING;
		OnRefreshTimer(pParam);
	}

}

/////////////////////////////////////////////////////////////////////////////
//refresh time-out, reregister
void  COneConf::OnRefreshTimer(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnRefreshTimer, conf = ",m_data->m_pConfName);

	if(m_timerRefresh)
	{
		m_timerRefresh = 0;
		if(m_DNSResolvedIp)
			ClearResolvedIps();
	}

	if(m_dwKeepAliveFrequencyConf_Sec)
  {
    DeleteTimer( MS_KEEP_ALIVE_CLIENT_TIMER );
    m_dwKeepAliveFrequencyConf_Sec = 0;
    m_MsKeepAlivePongTimeOutConf   = 0;
    m_dwKeppAliveBehaviorConf      = 0;
    m_dwKeepAliveTypeConf          = 0;
  }


	if(m_data->m_bIsDiscovery)
	{
		PTRACE(eLevelInfoNormal, "COneConf::OnRefreshTimerGo to unregister ......");
		OnUnRegisterRequestRegistered(pParam);
		return;
	}

	if(eConfSipServerManually == m_data->m_serversConfig)
	{
		if(isApiTaNull(&m_data->m_proxyAddress))
		{
			PTRACE(eLevelInfoNormal, "COneConf::OnRefreshTimer, ProxyIp=0, Not sending REGISTER request.");
			if(eServerStatusSpecify == m_data->m_DNSStatus || eServerStatusAuto == m_data->m_DNSStatus)//rons
			//if(m_data->m_DNSStatus)
			{
				if(strcmp(m_data->m_pProxyName, ""))
				{
					ResolveDomainReq(m_data->m_serviceId, m_data->m_pProxyName);
					m_state = CONF_REGISTERED_DNS_RESOLVING;
				}
				else
					PTRACE(eLevelError, "COneConf::OnRefreshTimer, ProxyName = "", Not sending REGISTER request.");
			}
		}
		if(CONF_REGISTERED_DNS_RESOLVING != m_state && isApiTaNull(&m_data->m_outboundProxyAddress))
		{
			if(strcmp(m_data->m_poutboundProxyName, ""))
			{
				PTRACE(eLevelInfoNormal, "COneConf::OnRefreshTimer, outboundProxyIP=0, Not sending REGISTER request.");
				if(strncmp(m_data->m_poutboundProxyName, m_data->m_pProxyName, H243_NAME_LEN))
				{
					if(m_data->m_DNSStatus)
					{
						ResolveDomainReq(m_data->m_serviceId, m_data->m_poutboundProxyName);
						m_state = CONF_REGISTERED_DNS_RESOLVING;
					}
				}
				//outbound proxy and registrar are the same
				else
				{
					if(m_data->m_proxyAddress.ipVersion == eIpVersion4)
						m_data->m_outboundProxyAddress.addr.v4.ip = m_data->m_proxyAddress.addr.v4.ip;
					else if(m_data->m_proxyAddress.ipVersion == eIpVersion6)
					{
						memcpy(m_data->m_outboundProxyAddress.addr.v6.ip, m_data->m_proxyAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
						m_data->m_outboundProxyAddress.addr.v6.scopeId = m_data->m_proxyAddress.addr.v6.scopeId;
					}
					m_data->m_outboundProxyAddress.ipVersion = m_data->m_proxyAddress.ipVersion;
				}
			}
			else
			{
				PTRACE(eLevelError, "COneConf::OnRefreshTimer, OutboundProxyName = "", Not sending REGISTER request.");
				if(m_data->m_proxyAddress.ipVersion == eIpVersion4)
					m_data->m_outboundProxyAddress.addr.v4.ip = m_data->m_proxyAddress.addr.v4.ip;
				else if(m_data->m_proxyAddress.ipVersion == eIpVersion6)
				{
					memcpy(m_data->m_outboundProxyAddress.addr.v6.ip, m_data->m_proxyAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
					m_data->m_outboundProxyAddress.addr.v6.scopeId = m_data->m_proxyAddress.addr.v6.scopeId;
				}
				m_data->m_outboundProxyAddress.ipVersion = m_data->m_proxyAddress.ipVersion;
			}
		}
	}
	else
		if(isApiTaNull(&m_data->m_proxyAddress) && eConfSipServerAuto == m_data->m_serversConfig)
		{
			if(m_data->m_DNSStatus)
			{
				if(strcmp(m_data->m_pHostName, ""))
				{
					ServiceReq(m_data->m_serviceId, m_data->m_pHostName, eIPProtocolType_SIP, m_data->m_transportType);
					m_state = CONF_REGISTERED_DNS_RESOLVING;
				}
				else
					PTRACE(eLevelError, "COneConf::OnRefreshTimer, HostName = "", Not sending REGISTER request.");
			}
			//otherwise, try to get data from DHCP
			else
			{
				if(CheckDHCPData(m_data))
					OnRefreshTimer(pParam);
			}
		}

	if(CONF_REGISTERED_DNS_RESOLVING == m_state)
	{
		StartTimer(TIMER_DNS_RESOLVING, ResolvingTimeOut*SECOND);
		return;
	}

	if(isApiTaNull(&m_data->m_proxyAddress))
		return;
	if (!RegisterRequest(m_data->m_expires))
		return;
	if (m_data->m_status == CONF_IDLE)
		m_data->m_status = CONF_REGISTERING;
	// start registering timer
	StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
	m_timerRegister = 1;
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnUnRegisterRequestRegistering(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnUnRegisterRequestRegistering, conf = ",m_data->m_pConfName);

	if(m_timerRegister)
	{
		// if while trying to register, we receive unregister request -> m_data->m_remove = 1 in order to execute unregister once the register request is answered.
		m_data->m_remove = TRUE;
		m_timerRegister = 0;
		DeleteTimer( TIMER_START_REGISTERING );
//		OnUnRegisterRequestRegistered(pParam);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnUnRegisterRequestRegistered(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnUnRegisterRequestRegistered, conf = ",m_data->m_pConfName);

	// if while trying to register, we receive unregister request
	if(m_timerRegister)
	{
		PTRACE(eLevelInfoNormal, "COneConf::OnUnRegisterRequestRegistered: unregister request was received while trying to refresh registration. waiting for answer, and only then will unregister!");
		DeleteTimer( TIMER_START_REGISTERING );	// delete registering timer
		m_timerRegister = 0;

		//in order to execute unregister once the register request is answered.
		m_data->m_remove = TRUE;
		return;
	}
	//m_data->m_remove = FALSE;

	if(m_timerRefresh)
	{
		DeleteTimer( TIMER_REFRESH_REGISTRATION );	// delete registering timer
		m_timerRefresh = 0;
	}

  if(m_dwKeepAliveFrequencyConf_Sec)
  {
    DeleteTimer( MS_KEEP_ALIVE_CLIENT_TIMER );
    m_dwKeepAliveFrequencyConf_Sec = 0;
    m_MsKeepAlivePongTimeOutConf   = 0;
    m_dwKeppAliveBehaviorConf      = 0;
    m_dwKeepAliveTypeConf          = 0;
  }

	if(m_DNSResolvedIp)
		ClearResolvedIps();

	if(eConfSipServerManually == m_data->m_serversConfig)
	{
		if(isApiTaNull(&m_data->m_proxyAddress))
		{
			PTRACE(eLevelInfoNormal, "COneConf::OnUnRegisterRequestRegistered, ProxyIp=0, Not sending REGISTER request.");
			if(m_data->m_DNSStatus)
			{
				if(strcmp(m_data->m_pProxyName, ""))
				{
					ResolveDomainReq(m_data->m_serviceId, m_data->m_pProxyName);
					m_state = CONF_UNREGISTERING_DNS_RESOLVING;
				}
				else
					PTRACE(eLevelError, "COneConf::OnUnRegisterRequestRegistered, ProxyName = "", Not sending REGISTER request.");
			}
		}
		if(CONF_UNREGISTERING_DNS_RESOLVING != m_state && isApiTaNull(&m_data->m_outboundProxyAddress))
		{
			if(strcmp(m_data->m_poutboundProxyName, ""))
			{
				PTRACE(eLevelInfoNormal, "COneConf::OnUnRegisterRequestRegistered, outboundProxyIP=0, Not sending REGISTER request.");
				if(strncmp(m_data->m_poutboundProxyName, m_data->m_pProxyName, H243_NAME_LEN))
				{
					if(m_data->m_DNSStatus)
					{
						ResolveDomainReq(m_data->m_serviceId, m_data->m_poutboundProxyName);
						m_state = CONF_UNREGISTERING_DNS_RESOLVING;
					}
				}
				//outbound proxy and registrar are the same
				else
				{
					if(m_data->m_proxyAddress.ipVersion == eIpVersion4)
						m_data->m_outboundProxyAddress.addr.v4.ip = m_data->m_proxyAddress.addr.v4.ip;
					else if(m_data->m_proxyAddress.ipVersion == eIpVersion6)
					{
						memcpy(m_data->m_outboundProxyAddress.addr.v6.ip, m_data->m_proxyAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
						m_data->m_outboundProxyAddress.addr.v6.scopeId = m_data->m_proxyAddress.addr.v6.scopeId;
					}
					m_data->m_outboundProxyAddress.ipVersion = m_data->m_proxyAddress.ipVersion;
				}
			}
			else
			{
				PTRACE(eLevelError, "COneConf::OnUnRegisterRequestRegistered, OutboundProxyName = "", Not sending REGISTER request.");
				if(m_data->m_proxyAddress.ipVersion == eIpVersion4)
				{
					m_data->m_outboundProxyAddress.addr.v4.ip = m_data->m_proxyAddress.addr.v4.ip;
				}
				else if(m_data->m_proxyAddress.ipVersion == eIpVersion6)
				{
					memcpy(m_data->m_outboundProxyAddress.addr.v6.ip, m_data->m_proxyAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
					m_data->m_outboundProxyAddress.addr.v6.scopeId = m_data->m_proxyAddress.addr.v6.scopeId;
				}
				m_data->m_outboundProxyAddress.ipVersion = m_data->m_proxyAddress.ipVersion;
			}
		}
	}
	else
		if(isApiTaNull(&m_data->m_proxyAddress) && eConfSipServerAuto == m_data->m_serversConfig)
		{
			if(m_data->m_DNSStatus)
			{
				if(strcmp(m_data->m_pHostName, ""))
				{
					ServiceReq(m_data->m_serviceId, m_data->m_pHostName, eIPProtocolType_SIP, m_data->m_transportType);
					m_state = CONF_UNREGISTERING_DNS_RESOLVING;
				}
				else
					PTRACE(eLevelError, "COneConf::OnUnRegisterRequestRegistered, HostName = "", Not sending REGISTER request.");
			}
			//otherwise, try to get data from DHCP
			else
			{
				if(CheckDHCPData(m_data))
					OnUnRegisterRequestRegistered(pParam);
			}
		}

	if(CONF_UNREGISTERING_DNS_RESOLVING == m_state)
	{
		StartTimer(TIMER_DNS_RESOLVING, ResolvingTimeOut*SECOND);
		return;
	}

	if(isApiTaNull(&m_data->m_proxyAddress))
		return;

	m_state = CONF_UNREGISTERING;
	m_data->m_status = CONF_UNREGISTERING;
	CallSubscribe(FALSE);

	{
		DWORD exp = 0;
		/* if dummy_tester - use expires 3600 else use 0 */
		exp = strncmp(m_data->m_pConfName, m_data->m_dummy_name, H243_NAME_LEN) ? 0 : 3600;
		if (!RegisterRequest(exp))
			return;
	}

	if(TRUE == m_data->m_remove)
	{
		SIPProxyMngrApi sipProxyMngrApi;
		sipProxyMngrApi.CreateOnlyApi(*m_pSipMngrRcvMbx);
		sipProxyMngrApi.KillOneConf(m_indInMngrDB);
		sipProxyMngrApi.DestroyOnlyApi();
		return;
	}

	//if dummy conf set timer for shorter period.
	if(!strncmp(m_data->m_pConfName, m_data->m_dummy_name, H243_NAME_LEN))
    {
        {//---Sasha Sh. Disable/Enable dummy_tester1  REGISTER---------------------------------------------//
            BOOL bIsDisableDummyRegistration = 0;
            std::string key = "DISABLE_DUMMY_REGISTRATION";
            CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, bIsDisableDummyRegistration);
            if(true == bIsDisableDummyRegistration)
            {
                PTRACE(eLevelInfoNormal,"CSipProxyServiceManager::CreateDummyConf - Flag:DISABLE.");
                StartTimer(TIMER_START_UNREGISTER, UnRegisteringDummyTimeOut*SECOND);
            }
        }//-----------------------------------------------------------------------------------------------//
    }
	// start Un-Register timer
	else
		StartTimer(TIMER_START_UNREGISTER, UnRegisteringTimeOut*SECOND);
	m_timerUnRegister = 1;
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnUnRegisterRequestUnRegistering(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnUnRegisterRequestUnRegistering, conf = ",m_data->m_pConfName);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnUnRegisterRequestIdleDNSResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnUnRegisterRequestIdleDNSResolving, conf = ",m_data->m_pConfName);

	if(m_timerRegister)
	{
		// if while trying to register, we receive unregister request -> m_data->m_remove = 1 in order to execute unregister once the register request is answered.
		m_data->m_remove = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnUnRegisterRequestRegisteringDNSResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnUnRegisterRequestRegisteringDNSResolving, conf = ",m_data->m_pConfName);

	if(m_timerRegister)
	{
		// if while trying to register, we receive unregister request -> m_data->m_remove = 1 in order to execute unregister once the register request is answered.
		m_data->m_remove = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnUnRegisterRequestRegisteredDNSResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnUnRegisterRequestRegisteredDNSResolving, conf = ",m_data->m_pConfName);

	if(m_timerRegister)
	{
		// if while trying to register, we receive unregister request -> m_data->m_remove = 1 in order to execute unregister once the register request is answered.
		m_data->m_remove = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnUnRegisterRequestUnRegisteringDNSResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnUnRegisterRequestUnRegisteringDNSResolving, conf = ",m_data->m_pConfName);
}

/////////////////////////////////////////////////////////////////////////////
//This method is used when we finish unregistering a conf
void  COneConf::OnRegisteringOKUnRegistering(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringOKUnRegistering, conf = ",m_data->m_pConfName);

//	DWORD Id = GetId();
//	m_pLoadMngrConnector->RegisterExitCriticalSection(Id);

	if(m_timerUnRegister)
	{
		DeleteTimer( TIMER_START_UNREGISTER );	// delete registering timer
		m_timerUnRegister = 0;
	}
	m_data->m_remove = FALSE;
	m_state = CONF_IDLE;
	m_data->m_lastRegStatus	= eSipServerStatusTypeOk;
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnUnRegisteringTimer(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnUnRegisteringTimer, conf = ",m_data->m_pConfName);
	m_timerUnRegister = 0;
	if(m_DNSResolvedIp)
		ClearResolvedIps();

	SIPProxyMngrApi sipProxyMngrApi;
	sipProxyMngrApi.CreateOnlyApi(*m_pSipMngrRcvMbx);
	if (m_data->m_bIsFromOldService != TRUE)
	{
		eSipRegistrationConfType sipConfType;
		sipConfType = SipProxyServiceManagerGetConfType(m_data);

		sipProxyMngrApi.RegistrarStatusUpdate(STATUS_FAIL, m_data->m_pProxyName, m_data->m_serviceId, m_data->m_proxyAddress, m_data->m_confID, (WORD)sipConfType, m_data->m_expires);
	}
	if(m_data->m_bIsDiscovery)
		OnUnRegisterRequestRegistered(pParam);
	else
		sipProxyMngrApi.KillOneConf(m_indInMngrDB);

	sipProxyMngrApi.DestroyOnlyApi();
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnRegisteringFailedRegistered(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringFailedRegistered, conf = ",m_data->m_pConfName);
	OnRegisteringFailedRegistering(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnRegisteringFailedRegistering(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringFailedRegistering, conf = ",m_data->m_pConfName);

	m_countFailedRegs = 3; //mark that we succeeded to contact registrar (switch problem -  overcome)
//	DWORD Id = GetId();
//	m_pLoadMngrConnector->RegisterExitCriticalSection(Id);
	m_presenceNeedToRepublish = TRUE;
	if (m_timerRegister)
	{
		DeleteTimer( TIMER_START_REGISTERING );	// delete registering timer
		m_timerRegister = 0;
	}
	m_state = CONF_IDLE;
	m_data->m_lastRegStatus	= eSipServerStatusTypeFail;

	if(m_data->m_remove)
	{
		m_state = CONF_UNREGISTERING;
		m_data->m_status = CONF_UNREGISTERING;
		SIPProxyMngrApi sipProxyMngrApi;
		sipProxyMngrApi.CreateOnlyApi(*m_pSipMngrRcvMbx);
		sipProxyMngrApi.KillOneConf(m_indInMngrDB);
		sipProxyMngrApi.DestroyOnlyApi();
	}
	else
	{
		if(m_bIsCrlfRegistration)
			// start refresh registration timer for 45 seconds
			StartTimer(TIMER_REFRESH_REGISTRATION, 45*SECOND);
		else
		// start refresh registration timer
			StartTimer(TIMER_REFRESH_REGISTRATION, RegisteringTimeOut*SECOND);
		m_timerRefresh = 1;
	}
	if(m_DNSResolvedIp)
		ClearResolvedIps();

	SetRegisterActiveAlarm(TRUE);
	OnRegisterEmptyWaitingPartiesVector(0);
}
/////////////////////////////////////////////////////////////////////////////
void COneConf::SetRegisterActiveAlarm(BYTE IsAdd)
{
	PTRACE(eLevelInfoNormal, "COneConf::SetRegisterActiveAlarm Do Nothing");
}
/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnRegisteringFailedUnRegistering(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnRegisteringFailedUnRegistering, conf = ",m_data->m_pConfName);

//	DWORD Id = GetId();
//	m_pLoadMngrConnector->RegisterExitCriticalSection(Id);

	if (m_timerUnRegister)
	{
		DeleteTimer( TIMER_START_UNREGISTER );
		m_timerUnRegister = 0;
	}
	if(m_DNSResolvedIp)
		ClearResolvedIps();

	SIPProxyMngrApi sipProxyMngrApi;
	sipProxyMngrApi.CreateOnlyApi(*m_pSipMngrRcvMbx);

	if(m_data->m_bIsDiscovery)
	{
		// start Un-Register timer, to test proxy again
		StartTimer(TIMER_REFRESH_REGISTRATION, UnRegisteringTimeOut*SECOND);
		m_timerRefresh = 1;
		m_state = CONF_REGISTERED;
		m_data->m_status = CONF_REGISTERED;
		m_data->m_lastRegStatus  = eSipServerStatusTypeFail;
	}
	else
	{
		sipProxyMngrApi.KillOneConf(m_indInMngrDB);
	}
	sipProxyMngrApi.DestroyOnlyApi();
}

/////////////////////////////////////////////////////////////////////////////
BYTE COneConf::RegisterRequest(DWORD expires)
{
	if (IsBusyServer())
	{
		TRACEINTO << "Busy server, request isn't sent";
		BYTE bRegister = (expires != 0) ? TRUE : FALSE;
		HandleBusyServer(bRegister);
		return FALSE;
	}

	TRACEINTO << "Expires:" << expires;

	char serviceIpV4[16];
	char serviceIpV6Site[128];
	char serviceIpV6Local[128];
	char a[128];
	eIpType ipType = m_data->m_ipType;

	SetCsIpArrayAccordingToIpTypeAndScopeId(m_data);
	char addrListStr[TOTAL_NUM_OF_IP_ADDRESSES][128];
	memset(&addrListStr, 0, sizeof(addrListStr));
	int actualIps = 0;
	int indexAccordingToProxy = 0;

	std::ostringstream msg;

	for (int index = 0; index < TOTAL_NUM_OF_IP_ADDRESSES; index++)
	{
		ipAddressStruct ipAddrSt = m_data->GetServiceAddress(index);
		memset(a, 0, sizeof(a));
		msg << "\n  IpAddress         :" << ipToString(ipAddrSt, a, 0);

		//MS_IPV6:
		TRACEINTO << "MS_IPV6: outboundProxyVersion:" << (DWORD)m_data->m_outboundProxyAddress.ipVersion
				  << ", proxyVersion:" << (DWORD)m_data->m_proxyAddress.ipVersion
				  << ", m ipAddrSt.ipVersion:" << (DWORD)ipAddrSt.ipVersion;

		if (m_data->m_outboundProxyAddress.ipVersion == ipAddrSt.ipVersion)
			indexAccordingToProxy = index;

		if (!IsIpNull(&ipAddrSt))
		{
			if (eIpVersion4 == ipAddrSt.ipVersion && (ipType == eIpType_IpV4 || ipType == eIpType_Both))
			{
				SystemDWORDToIpString(ipAddrSt.addr.v4.ip, addrListStr[actualIps]);
				actualIps++;
			}
			else if (eIpVersion6 == ipAddrSt.ipVersion && (ipType == eIpType_IpV6 || ipType == eIpType_Both))
			{
				if (ipAddrSt.addr.v6.scopeId != (int)eScopeIdLink)
				{
					ipV6ToStringProtected(ipAddrSt.addr.v6.ip, addrListStr[actualIps], TRUE, 128); // B.S klocwork 2568
					actualIps++;
				}
			}
			msg << ", addrListStr[ " << index << "]:" << addrListStr[actualIps];
		}
	}

	TRACEINTO << "MS_IPV6: indexAccordingToProxy:" << (DWORD)indexAccordingToProxy;
	ALLOCBUFFER(strDisplay, strlen(m_data->m_pConfName) + strlen("\"\"") + 1);
	sprintf(strDisplay, "\"%s\"", m_data->m_pConfName);

	// start conf registration
	// ....
	CSipRegisterStruct* pSipRegister = new CSipRegisterStruct;
	if (m_data->m_outboundProxyAddress.ipVersion == eIpVersion4)
	{
		msg << "\n  IpVersion         :eIpVersion4";
		pSipRegister->SetProxyIpV4(m_data->m_outboundProxyAddress.addr.v4.ip);
		pSipRegister->SetProxyIpVersion(eIpVersion4);
	}
	else if (m_data->m_outboundProxyAddress.ipVersion == eIpVersion6)
	{
		msg << "\n  IpVersion         :eIpVersion6";
		pSipRegister->SetProxyIpV6((char *)m_data->m_outboundProxyAddress.addr.v6.ip);
		pSipRegister->SetProxyV6scopeId(m_data->m_outboundProxyAddress.addr.v6.scopeId);
		pSipRegister->SetProxyIpVersion(eIpVersion6);
	}

	msg << "\n  OutboundProxyPort :" << m_data->m_outboundProxyAddress.port;
	msg << "\n  RegistrarPort     :" << m_data->m_proxyAddress.port;

	TRACEINTO << msg.str().c_str();

	pSipRegister->SetProxyPort(m_data->m_outboundProxyAddress.port);
	pSipRegister->SetTransportType(m_data->m_transportType);
	if (m_data->m_proxyAddress.ipVersion == eIpVersion4)
	{
		pSipRegister->SetRegistrarIpV4(m_data->m_proxyAddress.addr.v4.ip);
		pSipRegister->SetRegistrarIpVersion(eIpVersion4);
	}
	else if (m_data->m_proxyAddress.ipVersion == eIpVersion6)
	{
		pSipRegister->SetRegistrarIpV6((char *)m_data->m_proxyAddress.addr.v6.ip);
		pSipRegister->SetRegistrarV6scopeId(m_data->m_proxyAddress.addr.v6.scopeId);
		pSipRegister->SetRegistrarIpVersion(eIpVersion6);
	}

	ALLOCBUFFER(confUri, MaxAddressListSize);
	strncpy(confUri, m_data->m_pConfName, H243_NAME_LEN);

	///VNGFE-7595 if we already have the name as URI do not strcat the @host
	if (strstr(m_data->m_pConfName, "@") == NULL)
	{
		strncat(confUri, "@", 1);
		if (strcmp(m_data->m_pHostName, "") != 0)
			strncat(confUri, m_data->m_pHostName, H243_NAME_LEN);
		else
		{
			// Insert the registrar address
			char tempName[H243_NAME_LEN];
			memset(&tempName, '\0', H243_NAME_LEN);
			ipToString(*(pSipRegister->GetRegistrar()), tempName, 1);
			strncat(confUri, tempName, H243_NAME_LEN);
		}
	}

	TRACEINTO << "ConfUri:" << confUri;

	pSipRegister->SetRegistrarPort(m_data->m_proxyAddress.port);
	pSipRegister->SetHeaderField(kFromDisplay, strDisplay);
	pSipRegister->SetHeaderField(kFrom, confUri);
	pSipRegister->SetHeaderField(kToDisplay, strDisplay);
	pSipRegister->SetHeaderField(kTo, confUri);

	if (pSipRegister->GetProxyPort() == 0)
		pSipRegister->SetProxyPort(m_data->m_proxyAddress.port);
	if ((m_data->m_proxyAddress.port == 0) && (m_data->m_outboundProxyAddress.port != 0))
		pSipRegister->SetRegistrarPort(m_data->m_outboundProxyAddress.port);

	if (strDisplay)
	{
		pSipRegister->SetHeaderField(kContactDisplay,strDisplay);
	}

	std::ostringstream msg1;

	char contactUriIp[MaxAddressListSize]; //B.S.: klocwork 2567
	//MS_IPV6:
	TRACEINTO << "@@@ BuildcontactUriIp - NO cloud!!! MS_IPV6: ipType:" << (DWORD)m_data->m_ipType
			  << ", sipServerType:" << (DWORD)m_data->m_sipServerType;

	if (m_data->m_ipType==eIpType_Both && m_data->m_sipServerType==eSipServer_ms)
	{
memset(contactUriIp,0,MaxAddressListSize); //B.S.: klocwork 2567
	//	if(contactUriIp)
	//	{
			if(addrListStr[indexAccordingToProxy])
			{
				//sprintf(contactUriIp, "%s@%s;%s", m_data->m_pConfName, addrListStr[index],"+sip.instance=\"<urn:uuid:0ff8c27f-a367-5be8-a16f-b06dec00352b>\"");
				snprintf(contactUriIp, MaxAddressListSize, "%s@%s", m_data->m_pConfName, addrListStr[indexAccordingToProxy]);

				TRACEINTO << "MS_IPV6: addrListStr[indexAccordingToProxy] is valid";
			}

			TRACEINTO << "MS_IPV6: contactUriIp is valid";
			pSipRegister->SetHeaderField(kContact,contactUriIp);
	//	}
	//	DEALLOCBUFFER(contactUriIp)
	}
	else
	{
	for (int index = 0; index < actualIps/*TOTAL_NUM_OF_IP_ADDRESSES*/; index++)
	{
		memset(contactUriIp, 0, MaxAddressListSize); //B.S.: klocwork 2567

		if (addrListStr[index])
		{
			///VNGFE-7595 if we already have the name as URI do not strcat the @host
			if (strstr(m_data->m_pConfName, "@"))
			{
				snprintf(contactUriIp, ARRAYSIZE(contactUriIp), "%s", m_data->m_pConfName);
			}
			else
			{
				snprintf(contactUriIp, ARRAYSIZE(contactUriIp), "%s@%s", m_data->m_pConfName, addrListStr[index]);
			}
			msg1 << "\n  ContactUriIp:" << contactUriIp;
		}
		pSipRegister->SetHeaderField(kContact, contactUriIp);
		}
	}

	TRACEINTO << "ConfName:" << m_data->m_pConfName << msg1.str().c_str();

	pSipRegister->SetExpireHeader(expires);
	DWORD Id = GetId();
	pSipRegister->SetId(Id);
	pSipRegister->SetRegisterSubOpcode(GetRegisterSubOpcode());

	mcReqRegister* pRegMsg = pSipRegister->BuildRegister();
	//set XML params

	if (pRegMsg)
	{
		pRegMsg->registrarTransportAddr.unionProps.unionType = m_data->m_proxyAddress.ipVersion;				//eIpVersion6;//eIpVersion4;
		pRegMsg->proxyTransportAddr.unionProps.unionType = m_data->m_proxyAddress.ipVersion;				//eIpVersion6;//eIpVersion4;

		POBJDELETE(pSipRegister);

		int size = sizeof(mcReqRegisterBase) + pRegMsg->sipHeaders.headersListLength;

		TRACEINTO
			<< "UnionType:" << pRegMsg->registrarTransportAddr.unionProps.unionType
			<< "UnionSize:" << pRegMsg->registrarTransportAddr.unionProps.unionSize
			<< "RegistrarTransportAddrSize:" << size;

		// send registration
		// ....
		CSegment* pSeg = new CSegment;
		pSeg->Put((BYTE*)pRegMsg, size);
		SendMsgToCS(SIP_CS_PROXY_REGISTER_REQ, pSeg);

		PDELETE(pSeg);
		PDELETEA(pRegMsg);
		DEALLOCBUFFER(confUri);
		DEALLOCBUFFER(strDisplay);
		return TRUE;
	}
	else
	{
		PTRACE(eLevelError, "COneConf::RegisterRequest - pRegMsg is NULL could not send REGISTER msg to CS");
		DEALLOCBUFFER(confUri);
		DEALLOCBUFFER(strDisplay);
		POBJDELETE(pSipRegister); //B.S. klocwork 2549
		return FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::Subscribe(char* pEvent, char* pAccept, DWORD expires)
{
	if(!pEvent || !pAccept)
		return;

	m_timerSubscribeRefresh = 0;

	ALLOCBUFFER(confUri, MaxAddressListSize);
	strncpy(confUri, m_data->m_pConfName, H243_NAME_LEN);

	///VNGFE-7595 if we already have the name as URI do not strcat the @host
	//if ( (strncmp(m_data->m_pConfName, "rmx", 3) != 0) ||
	if (strstr(m_data->m_pConfName, "@") == NULL)
	{
		strncat(confUri, "@", 1);
		strncat(confUri, m_data->m_pHostName, H243_NAME_LEN);
	}
	PTRACE2(eLevelInfoNormal,"@@@ BuildcontactUriIp - confUri = ", confUri);
	CSipSubscribeStruct SipSubscribe;
	SipSubscribe.SetTransportType(m_data->m_transportType);
	TRACEINTO << "MS_IPV6: ProxyVersion:" << (DWORD)m_data->m_proxyAddress.ipVersion
			  << ", outboundProxy" << (DWORD)m_data->m_outboundProxyAddress.ipVersion;

	int proxyVersion = m_data->m_proxyAddress.ipVersion;
	if (proxyVersion == eIpVersion4)
		SipSubscribe.SetProxyIpV4(m_data->m_proxyAddress.addr.v4.ip);
	//SipSubscribe.SetProxyIp(182644750);
	else //eIpVersion6
		SipSubscribe.SetProxyIpV6((char *)m_data->m_proxyAddress.addr.v6.ip);

	SipSubscribe.SetProxyPort(m_data->m_proxyAddress.port);
	SipSubscribe.SetHeaderField(kFromDisplay, m_data->m_pConfName);
	SipSubscribe.SetHeaderField(kFrom, confUri);
	//SipSubscribe.SetHeaderField(kToDisplay, m_data->m_pConfName);
	SipSubscribe.SetHeaderField(kTo, confUri);
	//SipSubscribe.SetHeaderField(kContactDisplay, m_data->m_pConfName);
	SipSubscribe.SetHeaderField(kContact, confUri);

	ALLOCBUFFER(event, MaxAddressListSize);
	strcpy(event, "Event: ");
	strncat(event, pEvent, H243_NAME_LEN);
	SipSubscribe.SetHeaderField(kProprietyHeader, event);
	DEALLOCBUFFER(event);

	ALLOCBUFFER(accept, MaxAddressListSize);
	strcpy(accept, "Accept: ");
	strncat(accept, pAccept, H243_NAME_LEN);
	SipSubscribe.SetHeaderField(kProprietyHeader, accept);
	DEALLOCBUFFER(accept);

	ALLOCBUFFER(supported, MaxAddressListSize);
	strcpy(supported, "Supported: ");
	strncat(supported, "com.microsoft.autoextend", H243_NAME_LEN);
	SipSubscribe.SetHeaderField(kProprietyHeader, supported);
	DEALLOCBUFFER(supported);

	//SipSubscribe.SetEvent(pEvent);
	//SipSubscribe.SetAccept(pAccept);
	//SipSubscribe.SetSupported("com.microsoft.autoextend");
	DWORD Id = GetId();
	SipSubscribe.SetId(Id);
	SipSubscribe.SetExpires(expires);

	COstrStream msg;
	SipSubscribe.Dump(msg);
	PTRACE2(eLevelInfoNormal, "COneConf::Subscribe req:\n ", msg.str().c_str());

	mcReqSubscribe* pSubMsg = SipSubscribe.BuildSubscribeReq();

	if (pSubMsg)
	{
		int size = sizeof(mcReqSubscribeBase) + pSubMsg->sipHeaders.headersListLength;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)pSubMsg, size);
	SendMsgToCS(SIP_CS_PROXY_SUBSCRIBE_REQ, pSeg);

		PDELETE(pSeg);
		PDELETEA(pSubMsg);
	}
	else
	{
		PTRACE(eLevelError,"COneConf::Subscribe - pSubMsg is NULL could not send SUBSCRIBE msg to CS");
	}
	DEALLOCBUFFER(confUri);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnSubscribingOKRegistered(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnSubscribingOKRegistered, conf = ",m_data->m_pConfName);
	mcIndSubscribeResp * pSubscribeResponseMsg = (mcIndSubscribeResp *)pParam->GetPtr(1);
	DWORD expires = pSubscribeResponseMsg->expires;
	if(expires!=0)
	{
		CSipHeaderList * pTemp = new CSipHeaderList(pSubscribeResponseMsg->sipHeaders);
		const CSipHeader* pCallIdHeader = pTemp->GetNextHeader(kCallId);
		if(pCallIdHeader)
		{
			const char* pCallId = pCallIdHeader->GetHeaderStr();
			if (pCallId)
			{
				PTRACE2(eLevelInfoNormal, "COneConf::OnSubscribingOKRegistered, CallId=", pCallId);
				strncpy(m_callId, pCallId, MaxLengthOfSipCallId-1);
			}
			/*// start refresh registration timer
			StartTimer(TIMER_REFRESH_SUBSCRIBE, expires * SECOND);
			m_timerSubscribeRefresh = 1;*/
		}
		POBJDELETE(pTemp);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnSubscribingFailedRegistered(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnSubscribingFailedRegistered - Do Nothing , conf = ",m_data->m_pConfName);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnSubscribRefreshTimer(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnSubscribRefreshTimer, conf = ",m_data->m_pConfName);

	m_timerSubscribeRefresh = 0;
	if(isApiTaNull(&m_data->m_proxyAddress))
	{
		PTRACE(eLevelError, "COneConf::OnSubscribRefreshTimer, ProxyIp=0, Not sending SUBSCRIBE request.");
		return;
	}

	Subscribe("vnd-microsoft-roaming-ACL", "application/vnd-microsoft-roaming-acls+xml", 0x7FFFFFFF);
	SystemSleep(100*1);
	///Subscribe("presence.wpending", "text/xml+msrtc.wpending", 0x7FFFFFFF);
	///self.WakeAfter(Ticks2Sec()*1);
	//Subscribe("vnd-microsoft-provisioning", "application/vnd-microsoft-roaming-provisioning+xml", 0);
	//self.WakeAfter(Ticks2Sec()*1);
	ServicePresence(eONLINE);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnNotifyIndRegistered(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnNotifyIndRegistered, conf = ",m_data->m_pConfName);
	mcIndNotify * pNotifyMsg = (mcIndNotify *)pParam->GetPtr(1);
/*	char* temp = strstr(pNotifyMsg->message, "ace type=\"ALL\" mask=\"\" rights=\"AA\"");
	if(!temp)
	{
		temp = strstr(pNotifyMsg->message, "deltaNum=");
		if(temp)
		{
			int deltaNum = atoi(&temp[10]);
			ServiceAutoAdd(deltaNum);
			SystemSleep(100*1);

		}
	}
*/	Subscribe("vnd-microsoft-roaming-ACL", "application/vnd-microsoft-roaming-acls+xml", 0);
	*m_callId = '\0';
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::ServiceAutoAdd(int deltaNum)
{
	if(deltaNum==-1)
	{
		PTRACE2(eLevelError, "COneConf::ServiceAutoAdd, can not send auto-add service request. conf = ",m_data->m_pConfName);
		return;
	}

	ALLOCBUFFER(confUri, MaxAddressListSize);
	strncpy(confUri, m_data->m_pConfName, H243_NAME_LEN);

	///VNGFE-7595 if we already have the name as URI do not strcat the @host
	//if ( (strncmp(m_data->m_pConfName, "rmx", 3) != 0) ||
	if (strstr(m_data->m_pConfName, "@") == NULL )
	{
		strncat(confUri, "@", 1);
		strncat(confUri, m_data->m_pHostName, H243_NAME_LEN);
	}
	PTRACE2(eLevelInfoNormal,"@@@ ServiceAutoAdd - confUri = ", confUri);
	ALLOCBUFFER(pXML, 400);
	sprintf(pXML, "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"><SOAP-ENV:Body><m:setACE xmlns:m=\"http://schemas.microsoft.com/winrtc/2002/11/sip\"><m:type>ALL</m:type><m:mask></m:mask><m:rights>AA</m:rights><m:deltaNum>%d</m:deltaNum></m:setACE></SOAP-ENV:Body></SOAP-ENV:Envelope>", deltaNum);

	CSipUnknownMsgStruct SipService;
	SipService.SetMethod(SIP_SERVICE_METHOD);
	if(m_data->m_proxyAddress.ipVersion == eIpVersion4)
	{
		SipService.SetRegistrarIpV4(m_data->m_proxyAddress.addr.v4.ip);
		SipService.SetRegistrarIpVersion(eIpVersion4);
	}
	else if(m_data->m_proxyAddress.ipVersion == eIpVersion6)
	{
		SipService.SetRegistrarIpV6((char *)m_data->m_proxyAddress.addr.v6.ip);
		SipService.SetRegistrarV6scopeId(m_data->m_proxyAddress.addr.v6.scopeId);
		SipService.SetRegistrarIpVersion(eIpVersion6);
	}

	SipService.SetRegistrarPort(m_data->m_proxyAddress.port);
	SipService.SetHeaderField(kFromDisplay, m_data->m_pConfName);
	SipService.SetHeaderField(kFrom, confUri);
	SipService.SetContentType("application/SOAP+xml");
	SipService.SetContent(pXML);
	SipService.SetTransportType(m_data->m_transportType);

	COstrStream msg;
	SipService.Dump(msg);
	PTRACE2(eLevelInfoNormal, "COneConf::ServiceAutoAdd req:\n ", msg.str().c_str());

	mcReqUnknownMethod* pMsg = SipService.BuildUnknownMsgReq();
//*****	SipService.Dump();
	int size = sizeof(mcReqUnknownMethod);

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)pMsg, size);
	SendMsgToCS(SIP_CS_PROXY_UNKNOWN_METHOD_REQ, pSeg);

	PDELETE(pSeg);
	PDELETE(pMsg);
	DEALLOCBUFFER(confUri);
	DEALLOCBUFFER(pXML);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::ServicePresence(PRESENCE newPresence)
{
	ALLOCBUFFER(confUri, MaxAddressListSize);
	strncpy(confUri, m_data->m_pConfName, H243_NAME_LEN);

	///VNGFE-7595 if we already have the name as URI do not strcat the @host
	//if ( (strncmp(m_data->m_pConfName, "rmx", 3) != 0) ||
	if (strstr(m_data->m_pConfName, "@") == NULL )
	{
		strncat(confUri, "@", 1);
		strncat(confUri, m_data->m_pHostName, H243_NAME_LEN);
	}
	PTRACE2(eLevelInfoNormal,"@@@ ServicePresence - confUri = ", confUri);
	ALLOCBUFFER(pXML, 670 + MaxAddressListSize + H243_NAME_LEN);

	switch(newPresence)
	{
		case(eOFFLINE):
		{
			sprintf(pXML, "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"><SOAP-ENV:Body><m:setPresence xmlns:m=\"http://schemas.microsoft.com/winrtc/2002/11/sip\"><m:presentity m:uri=\"sip:%s\"><m:availability m:aggregate=\"0\" m:description=\"offline\"/><m:activity m:aggregate=\"0\" m:description=\"Active\"/><deviceName xmlns=\"http://schemas.microsoft.com/2002/09/sip/client/presence\" name=\"%s\"/><rtc:devicedata xmlns:rtc=\"http://schemas.microsoft.com/2002/09/sip/client/presence\" namespace=\"rtcService\"><![CDATA[<caps><renders_gif/><renders_isf/></caps>]]></rtc:devicedata></m:presentity></m:setPresence></SOAP-ENV:Body></SOAP-ENV:Envelope>\n", confUri, m_data->m_pConfName);
			break;
		}
		case(eONLINE):
		{
			sprintf(pXML, "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"><SOAP-ENV:Body><m:setPresence xmlns:m=\"http://schemas.microsoft.com/winrtc/2002/11/sip\"><m:presentity m:uri=\"sip:%s\"><m:availability m:aggregate=\"300\" m:description=\"online\"/><m:activity m:aggregate=\"400\" m:description=\"Active\"/><deviceName xmlns=\"http://schemas.microsoft.com/2002/09/sip/client/presence\" name=\"%s\"/><rtc:devicedata xmlns:rtc=\"http://schemas.microsoft.com/2002/09/sip/client/presence\" namespace=\"rtcService\"><![CDATA[<caps><renders_gif/><renders_isf/></caps>]]></rtc:devicedata></m:presentity></m:setPresence></SOAP-ENV:Body></SOAP-ENV:Envelope>\n", confUri, m_data->m_pConfName);
			break;
		}
		case(eAWAY):
		{
			sprintf(pXML, "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"><SOAP-ENV:Body><m:setPresence xmlns:m=\"http://schemas.microsoft.com/winrtc/2002/11/sip\"><m:presentity m:uri=\"sip:%s\"><m:availability m:aggregate=\"300\" m:description=\"online\"/><m:activity m:aggregate=\"100\" m:description=\"Away\"/><deviceName xmlns=\"http://schemas.microsoft.com/2002/09/sip/client/presence\" name=\"%s\"/><rtc:devicedata xmlns:rtc=\"http://schemas.microsoft.com/2002/09/sip/client/presence\" namespace=\"rtcService\"><![CDATA[<caps><renders_gif/><renders_isf/></caps>]]></rtc:devicedata></m:presentity></m:setPresence></SOAP-ENV:Body></SOAP-ENV:Envelope>\n", confUri, m_data->m_pConfName);
			break;
		}
		case(eBUSY):
		{
			sprintf(pXML, "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"><SOAP-ENV:Body><m:setPresence xmlns:m=\"http://schemas.microsoft.com/winrtc/2002/11/sip\"><m:presentity m:uri=\"sip:%s\"><m:availability m:aggregate=\"300\" m:description=\"online\"/><m:activity m:aggregate=\"600\" m:description=\"Busy\"/><deviceName xmlns=\"http://schemas.microsoft.com/2002/09/sip/client/presence\" name=\"%s\"/><rtc:devicedata xmlns:rtc=\"http://schemas.microsoft.com/2002/09/sip/client/presence\" namespace=\"rtcService\"><![CDATA[<caps><renders_gif/><renders_isf/></caps>]]></rtc:devicedata></m:presentity></m:setPresence></SOAP-ENV:Body></SOAP-ENV:Envelope>\n", confUri, m_data->m_pConfName);
			break;
		}
		default:
		{
			sprintf(pXML, "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"><SOAP-ENV:Body><m:setPresence xmlns:m=\"http://schemas.microsoft.com/winrtc/2002/11/sip\"><m:presentity m:uri=\"sip:%s\"><m:availability m:aggregate=\"300\" m:description=\"online\"/><m:activity m:aggregate=\"400\" m:description=\"Active\"/><deviceName xmlns=\"http://schemas.microsoft.com/2002/09/sip/client/presence\" name=\"%s\"/><rtc:devicedata xmlns:rtc=\"http://schemas.microsoft.com/2002/09/sip/client/presence\" namespace=\"rtcService\"><![CDATA[<caps><renders_gif/><renders_isf/></caps>]]></rtc:devicedata></m:presentity></m:setPresence></SOAP-ENV:Body></SOAP-ENV:Envelope>\n", confUri, m_data->m_pConfName);
			break;
		}
	}
	CSipUnknownMsgStruct SipService;
	SipService.SetMethod(SIP_SERVICE_METHOD);
	if(m_data->m_proxyAddress.ipVersion == eIpVersion4)
	{
		SipService.SetRegistrarIpV4(m_data->m_proxyAddress.addr.v4.ip);
		SipService.SetRegistrarIpVersion(eIpVersion4);
	}
	else if(m_data->m_proxyAddress.ipVersion == eIpVersion6)
	{
		SipService.SetRegistrarIpV6((char *)m_data->m_proxyAddress.addr.v6.ip);
		SipService.SetRegistrarV6scopeId(m_data->m_proxyAddress.addr.v6.scopeId);
		SipService.SetRegistrarIpVersion(eIpVersion6);
	}

	SipService.SetRegistrarPort(m_data->m_proxyAddress.port);
	SipService.SetHeaderField(kFromDisplay, m_data->m_pConfName);
	SipService.SetHeaderField(kFrom, confUri);
	SipService.SetContentType("application/SOAP+xml");
	SipService.SetContent(pXML);
	SipService.SetTransportType(m_data->m_transportType);

	COstrStream msg;
	SipService.Dump(msg);
	PTRACE2(eLevelInfoNormal, "COneConf::ServicePresence req:\n ", msg.str().c_str());

	mcReqUnknownMethod* pMsg = SipService.BuildUnknownMsgReq();

	int size = sizeof(mcReqUnknownMethod);

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)pMsg, size);
	SendMsgToCS(SIP_CS_PROXY_UNKNOWN_METHOD_REQ, pSeg);

	PDELETE(pSeg);
	PDELETE(pMsg);
	DEALLOCBUFFER(pXML);
	DEALLOCBUFFER(confUri);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::SendMsgToCS(OPCODE opcode, CSegment* pseg1)
{
	CMplMcmsProtocol *pMplMcmsProtocol = NULL;
	//if TDD
	if(m_pMockMplProtocol)
		pMplMcmsProtocol = m_pMockMplProtocol;
	else
		pMplMcmsProtocol = new CMplMcmsProtocol;

	if(!pMplMcmsProtocol)
	{
		PASSERT(101);
		return;
	}
	//printf("COneConf::SendMsgToCS %d\n", m_data->m_serviceId);
	pMplMcmsProtocol->AddCommonHeader(opcode);
	pMplMcmsProtocol->AddMessageDescriptionHeader();
	pMplMcmsProtocol->AddPortDescriptionHeader(0, GetId());
	//pMplMcmsProtocol->AddCSHeader(0,0, m_src_unit_id);

	//pMplMcmsProtocol->AddCSHeader(0,0,m_src_unit_id,0,m_data->m_serviceId,0,0,0);

	pMplMcmsProtocol->AddCSHeader(m_data->m_serviceId, 0, m_src_unit_id, 0, m_data->m_serviceId,0,0,0);
	if(pseg1)
	{
		DWORD nMsgLen = pseg1->GetWrtOffset() - pseg1->GetRdOffset();
		BYTE* pMessage = new BYTE[nMsgLen];
		pseg1->Get(pMessage,nMsgLen);
		pMplMcmsProtocol->AddData(nMsgLen,(const char*)pMessage);
		PDELETEA(pMessage);
	}
	pMplMcmsProtocol->AddPayload_len(CS_API_TYPE);
	CSipProxyMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("CSipProxyManager::SendMsgToCS ",CS_API_TYPE);
	pMplMcmsProtocol->SendMsgToCSApiCommandDispatcher();

	if(!m_pMockMplProtocol)
		PDELETE(pMplMcmsProtocol);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnDNSResolveIndConfIDLEResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfIDLEResolving. conf = ",m_data->m_pConfName);
	WORD serviceId = 0;
//	DWORD ipAddr = 0;

	ALLOCBUFFER(hostName, DnsQueryNameSize);//H243_NAME_LEN);

	DeleteTimer(TIMER_DNS_RESOLVING);

	*pParam >> serviceId
			>> hostName;
	//		>> ipAddr;

//	mcTransportAddress	resolvedAddr;
//	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
//	pParam->Get((BYTE *)&resolvedAddr, sizeof(mcTransportAddress));
	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}
	pParam->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		TRACEINTO << CIpAddrStructWrapper(pAddrList[i], "SIP PROXY DNS RESOLUTION IP");
	}

//	char resolveIpV6[128];
//	ipV6ToString(pAddrList[0].addr.v6.ip, resolveIpV6, TRUE);
//	PTRACE2(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfIDLEResolving resolving ip = " ,resolveIpV6);
	// need to choose one Addres from the list
	mcTransportAddress	resolvedAddr;
	char mo[128];
	memset(mo, 0, 128);
	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
	GetAddressFromList(&resolvedAddr,pAddrList,m_data->m_ipType);

	TRACEINTO << __FUNCTION__ << ipToString(resolvedAddr, mo, 0) << "\nIpVer = "
	<< resolvedAddr.ipVersion << " Ip Port = " << resolvedAddr.port;

	if(serviceId == m_data->m_serviceId)
	{
		if(m_data->m_remove)
		{
			m_state = CONF_UNREGISTERING;
			m_data->m_status = CONF_UNREGISTERING;

			SIPProxyMngrApi sipProxyMngrApi;
			sipProxyMngrApi.CreateOnlyApi(*m_pSipMngrRcvMbx);
			sipProxyMngrApi.KillOneConf(m_indInMngrDB);
			sipProxyMngrApi.DestroyOnlyApi();
		}
		else
		{
			//if(ipAddr != 0)
			if(!isApiTaNull(&resolvedAddr))
			{
				APIU32 port = 0;
				if(isApiTaNull(&m_data->m_proxyAddress) && !strncmp(m_data->m_pProxyName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
				{
					//m_data->m_proxyAddress.addr.v4.ip = ipAddr;
					port = m_data->m_proxyAddress.port;
					memset(mo, 0, 128);

					TRACEINTO << __FUNCTION__ << ": Proxy address (1): " << ::ipToString(m_data->m_proxyAddress, mo, 0) << "\nIpVer = "
					<< m_data->m_proxyAddress.ipVersion << " Ip Port = " << m_data->m_proxyAddress.port;

					memcpy(&m_data->m_proxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
					m_data->m_proxyAddress.port = port;

					// In case both the proxy addr and the outbound proxy have no addr and they both need the ip from the resolving
					if (isApiTaNull(&m_data->m_outboundProxyAddress) && !strncmp(m_data->m_poutboundProxyName, hostName,H243_NAME_LEN))
					{
						TRACEINTO <<"on register idle saving address in proxy";
						port = m_data->m_outboundProxyAddress.port;
						memcpy(&m_data->m_outboundProxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
						m_data->m_outboundProxyAddress.port = port;

					}
				}
				else
				{
					if(isApiTaNull(&m_data->m_outboundProxyAddress) && !strncmp(m_data->m_poutboundProxyName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
					{
						//m_data->m_outboundProxyAddress.addr.v4.ip = ipAddr;
						port = m_data->m_outboundProxyAddress.port;
						memcpy(&m_data->m_outboundProxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
						m_data->m_outboundProxyAddress.port = port;
					}
					else
					{	//start timer so we will try again to register again
						StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
						m_timerRegister = 1;
						m_state = CONF_REGISTERING;

						PTRACE2(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfIDLEResolving, Resolution result: unrecognized hostName, Not sending REGISTER request. hostName = ", hostName);
						DEALLOCBUFFER(hostName);
						return;
					}
				}

				m_DNSResolvedIp = TRUE;
				m_state = CONF_IDLE;

				memset(mo, 0, 128);

				TRACEINTO << __FUNCTION__ << ": Proxy address: " << ::ipToString(m_data->m_proxyAddress, mo, 0) << "\nIpVer = "
				<< m_data->m_proxyAddress.ipVersion << " Ip Port = " << m_data->m_proxyAddress.port;

				memset(mo, 0, 128);
				TRACEINTO << __FUNCTION__  << ": Proxy outbound address: " << ::ipToString(m_data->m_outboundProxyAddress, mo, 0) << "\nIpVer = "
				<< m_data->m_outboundProxyAddress.ipVersion << " Ip Port = " << m_data->m_outboundProxyAddress.port;

				CSegment seg;
				OnStartRegisterIDLE(&seg);

			}
			else
			{
				//start timer so we will try again to register again
				StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
				m_timerRegister = 1;
				m_state = CONF_REGISTERING;

				PTRACE(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfIDLEResolving, Resolution result: ProxyIp=0, Not sending REGISTER request.");
			}
		}
	}
	else
		DBGPASSERT(2);

	DEALLOCBUFFER(hostName);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnDNSResolveIndConfREGISTERINGResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfREGISTERINGResolving. conf = ",m_data->m_pConfName);
	WORD serviceId = 0;
	DWORD ipAddr = 0;
	ALLOCBUFFER(hostName, DnsQueryNameSize);//H243_NAME_LEN);
	DeleteTimer(TIMER_DNS_RESOLVING);

	*pParam >> serviceId
			>> hostName;
//			>> ipAddr;

//	mcTransportAddress	resolvedAddr;
//	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
//	pParam->Get((BYTE *)&resolvedAddr, sizeof(mcTransportAddress));
	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}
	pParam->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	// need to choose one Addres from the list
	mcTransportAddress	resolvedAddr;
	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
	GetAddressFromList(&resolvedAddr,pAddrList,m_data->m_ipType);

	if(serviceId == m_data->m_serviceId)
	{
		if(m_data->m_remove)
		{
			m_state = CONF_UNREGISTERING;
			m_data->m_status = CONF_UNREGISTERING;

			SIPProxyMngrApi sipProxyMngrApi;
			sipProxyMngrApi.CreateOnlyApi(*m_pSipMngrRcvMbx);
			sipProxyMngrApi.KillOneConf(m_indInMngrDB);
			sipProxyMngrApi.DestroyOnlyApi();
		}
		else
		{
			//if(ipAddr != 0)
			if(!isApiTaNull(&resolvedAddr))
			{
				if(isApiTaNull(&m_data->m_proxyAddress) && !strncmp(m_data->m_pProxyName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
				{
					//m_data->m_proxyAddress.addr.v4.ip = ipAddr;
					memcpy(&m_data->m_proxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
				}
				else
				{
					if(isApiTaNull(&m_data->m_outboundProxyAddress) && !strncmp(m_data->m_poutboundProxyName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
						//m_data->m_outboundProxyAddress.addr.v4.ip = ipAddr;
						memcpy(&m_data->m_outboundProxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
					else
					{	//start timer so we will try again to register again
						StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
						m_timerRegister = 1;
						m_state = CONF_REGISTERING;

						PTRACE2(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfREGISTERINGResolving, Resolution result: unrecognized hostName, Not sending REGISTER request. hostName = ", hostName);
						DEALLOCBUFFER(hostName);
						return;
					}
				}
				m_DNSResolvedIp = TRUE;
				m_state = CONF_REGISTERING;

				CSegment seg;
				OnRegisteringTimer(&seg);
			}
			else
			{
				//start timer so we will try again to register again
				StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
				m_timerRegister = 1;
				m_state = CONF_REGISTERING;

				PTRACE(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfREGISTERINGResolving, Resolution result: ProxyIp=0, Not sending REGISTER request.");
			}
		}
	}
	else
		DBGPASSERT(3);

	DEALLOCBUFFER(hostName);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnDNSResolveIndConfREGISTEREDResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfREGISTEREDResolving. conf = ",m_data->m_pConfName);
	WORD serviceId = 0;
	DWORD ipAddr = 0;
	ALLOCBUFFER(hostName, DnsQueryNameSize);//H243_NAME_LEN);

	DeleteTimer(TIMER_DNS_RESOLVING);

	*pParam >> serviceId
			>> hostName;
		//	>> ipAddr

//	mcTransportAddress	resolvedAddr;
//	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
//	pParam->Get((BYTE *)&resolvedAddr, sizeof(mcTransportAddress));

	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}
	pParam->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	// need to choose one Addres from the list
	mcTransportAddress	resolvedAddr;
	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
	GetAddressFromList(&resolvedAddr,pAddrList,m_data->m_ipType);

	if(serviceId == m_data->m_serviceId)
	{
		if(m_data->m_remove)
		{
			CSegment seg;
			OnUnRegisterRequestRegistered(&seg);
		}
		else
		{
			//if(ipAddr != 0)
			if(!isApiTaNull(&resolvedAddr))
			{
				APIU32 port = 0;

				if(isApiTaNull(&m_data->m_proxyAddress) && !strncmp(m_data->m_pProxyName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
				{
					//m_data->m_proxyAddress.addr.v4.ip = ipAddr;
					port = m_data->m_proxyAddress.port;
					memcpy(&m_data->m_proxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
					m_data->m_proxyAddress.port = port;
				}
				else
				{
					if(isApiTaNull(&m_data->m_outboundProxyAddress) && !strncmp(m_data->m_poutboundProxyName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
					{	//m_data->m_outboundProxyAddress.addr.v4.ip = ipAddr;
						port = m_data->m_outboundProxyAddress.port;
						memcpy(&m_data->m_outboundProxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
						m_data->m_outboundProxyAddress.port = port;
					}
					else
					{	//start timer so we will try again to register again
						StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
						m_timerRegister = 1;
						m_state = CONF_REGISTERING;

						PTRACE2(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfREGISTEREDResolving, Resolution result: unrecognized hostName, Not sending REGISTER request. hostName = ", hostName);
						DEALLOCBUFFER(hostName);
						return;
					}
				}
				m_DNSResolvedIp = TRUE;

				//		m_state = CONF_REGISTERING;
				// for Ice user should remain REGISTERED else changed to REGISTERING
				UpdateRegisterState();

				CSegment seg;
				OnRefreshTimer(&seg);
			}
			else
			{
				//start timer so we will try again to register again
				StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
				m_timerRegister = 1;
				m_state = CONF_REGISTERING;

				PTRACE(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfREGISTEREDResolving, Resolution result: ProxyIp=0, Not sending REGISTER request.");
			}
		}
	}
	else
		DBGPASSERT(4);

	DEALLOCBUFFER(hostName);
}
/////////////////////////////////////////////////////////////////////////////
void  COneConf::UpdateRegisterState()
{
	PTRACE2(eLevelInfoNormal, "COneConf::UpdateRegisterState - CONF_REGISTERING, conf = ",m_data->m_pConfName);
	m_state = CONF_REGISTERING;
}


/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnDNSResolveIndConfUNREGISTERINGResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfUNREGISTERINGResolving. conf = ",m_data->m_pConfName);
	WORD serviceId = 0;
	DWORD ipAddr = 0;
	ALLOCBUFFER(hostName, DnsQueryNameSize);//H243_NAME_LEN);
	hostName[0]='\0';
	DeleteTimer(TIMER_DNS_RESOLVING);

	*pParam >> serviceId
			>> hostName;
	//		>> ipAddr;

//	mcTransportAddress	resolvedAddr;
//	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
//	pParam->Get((BYTE *)&resolvedAddr, sizeof(mcTransportAddress));
	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}
	pParam->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		TRACEINTO << CIpAddrStructWrapper(pAddrList[i], "SIP PROXY DNS RESOLUTION IP for unregister ");
	}
	// need to choose one Addres from the list
	mcTransportAddress	resolvedAddr;
	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
	GetAddressFromList(&resolvedAddr,pAddrList,m_data->m_ipType);

	if(serviceId == m_data->m_serviceId)
	{
		//if(ipAddr != 0)
		if(!isApiTaNull(&resolvedAddr))
		{
			if(isApiTaNull(&m_data->m_proxyAddress) && !strncmp(m_data->m_pProxyName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
			{
				//m_data->m_proxyAddress.addr.v4.ip = ipAddr;
				memcpy(&m_data->m_proxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
			}
			else
			{
				if(isApiTaNull(&m_data->m_outboundProxyAddress) && !strncmp(m_data->m_poutboundProxyName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
					//m_data->m_outboundProxyAddress.addr.v4.ip = ipAddr;
					memcpy(&m_data->m_outboundProxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
				else
				{	//start timer so we will try again to register again
					StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
					m_timerRegister = 1;
					m_state = CONF_REGISTERING;

					PTRACE2(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfUNREGISTERINGResolving, Resolution result: unrecognized hostName, Not sending REGISTER request. hostName = ", hostName);
					DEALLOCBUFFER(hostName);
					return;
				}
			}
			m_state = CONF_UNREGISTERING;
			CSegment seg;
			OnUnRegisterRequestRegistered(&seg);
		}
		else
			PTRACE(eLevelInfoNormal, "COneConf::OnDNSResolveIndConfUNREGISTERINGResolving, Resolution result: ProxyIp=0, Not sending UNREGISTER request.");
	}
	else
		DBGPASSERT(5);

	DEALLOCBUFFER(hostName);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnDNSServiceIndConfIDLEResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnDNSServiceIndConfIDLEResolving. conf = ",m_data->m_pConfName);
	WORD serviceId = 0;
	DWORD ipAddr = 0, port = 0, transport = 0;
	ALLOCBUFFER(hostName, DnsQueryNameSize);//H243_NAME_LEN);


	DeleteTimer(TIMER_DNS_RESOLVING);

	*pParam >> serviceId
			>> hostName
		//	>> ipAddr
			>> port
			>> transport;

//	mcTransportAddress	resolvedAddr;
//	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
//	pParam->Get((BYTE *)&resolvedAddr, sizeof(mcTransportAddress));
	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}
	pParam->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	// need to choose one Addres from the list
	mcTransportAddress	resolvedAddr;
	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
	GetAddressFromList(&resolvedAddr,pAddrList,m_data->m_ipType);

	if(serviceId == m_data->m_serviceId)
	{
		if(m_data->m_remove)
		{
			m_state = CONF_UNREGISTERING;
			m_data->m_status = CONF_UNREGISTERING;

			SIPProxyMngrApi sipProxyMngrApi;
			sipProxyMngrApi.CreateOnlyApi(*m_pSipMngrRcvMbx);
			sipProxyMngrApi.KillOneConf(m_indInMngrDB);
			sipProxyMngrApi.DestroyOnlyApi();
		}
		else
		{
			if(!isApiTaNull(&resolvedAddr) && !strncmp(m_data->m_pHostName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
		//	if(ipAddr != 0  && !strncmp(m_data->m_pHostName, hostName,H243_NAME_LEN))
			{
				//m_data->m_proxyAddress.addr.v4.ip = ipAddr;
				memcpy(&m_data->m_proxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
				m_data->m_proxyAddress.port = port;

				//m_data->m_outboundProxyAddress.addr.v4.ip = ipAddr;
				memcpy(&m_data->m_outboundProxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
				m_data->m_outboundProxyAddress.port = port;
				m_data->m_transportType = transport;
				m_DNSResolvedIp = TRUE;

				m_state = CONF_IDLE;

				CSegment seg;

				OnStartRegisterIDLE(&seg);
			}
			else
			{
				if(CheckDHCPData(m_data))
				{
					m_DNSResolvedIp = TRUE;
					m_state = CONF_IDLE;

					CSegment seg;
					OnStartRegisterIDLE(&seg);
				}
				else
				{
					PTRACE(eLevelInfoNormal, "COneConf::OnDNSServiceIndConfIDLEResolving, Resolution result: ProxyIp=0, Not sending REGISTER request.");
					//start timer so we will try to register again
					StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
					m_timerRegister = 1;
					m_state = CONF_REGISTERING;
				}
			}
		}
	}
	else
	{
		PTRACE(eLevelError, "COneConf::OnDNSServiceIndConfIDLEResolving, Illegal Card Id or service name.");
		DBGPASSERT(2);
	}

	DEALLOCBUFFER(hostName);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnDNSServiceIndConfREGISTERINGResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnDNSServiceIndConfREGISTERINGResolving. conf = ",m_data->m_pConfName);
	WORD serviceId = 0;
	DWORD ipAddr = 0, port = 0, transport = 0;
	ALLOCBUFFER(hostName, DnsQueryNameSize);//H243_NAME_LEN);

	DeleteTimer(TIMER_DNS_RESOLVING);

	*pParam >> serviceId
			>> hostName
		//	>> ipAddr
			>> port
			>> transport;

//	mcTransportAddress	resolvedAddr;
//	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
//	pParam->Get((BYTE *)&resolvedAddr, sizeof(mcTransportAddress));
	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}
	pParam->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	// need to choose one Addres from the list
	mcTransportAddress	resolvedAddr;
	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
	GetAddressFromList(&resolvedAddr,pAddrList,m_data->m_ipType);

	if(serviceId == m_data->m_serviceId)
	{
		if(m_data->m_remove)
		{
			m_state = CONF_UNREGISTERING;
			m_data->m_status = CONF_UNREGISTERING;

			SIPProxyMngrApi sipProxyMngrApi;
			sipProxyMngrApi.CreateOnlyApi(*m_pSipMngrRcvMbx);
			sipProxyMngrApi.KillOneConf(m_indInMngrDB);
			sipProxyMngrApi.DestroyOnlyApi();
		}
		else
		{
			//if(ipAddr != 0  && !strncmp(m_data->m_pHostName, hostName,H243_NAME_LEN))
			if(!isApiTaNull(&resolvedAddr) && !strncmp(m_data->m_pHostName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
			{
				//m_data->m_proxyAddress.addr.v4.ip = ipAddr;
				memcpy(&m_data->m_proxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
				m_data->m_proxyAddress.port = port;

				//m_data->m_outboundProxyAddress.addr.v4.ip = ipAddr;
				memcpy(&m_data->m_outboundProxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
				m_data->m_outboundProxyAddress.port = port;
				m_data->m_transportType = transport;
				m_DNSResolvedIp = TRUE;
				m_state = CONF_REGISTERING;

				CSegment seg;
				OnRegisteringTimer(&seg);
			}
			else
			{
				//start timer so we will try again to register again
				StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
				m_timerRegister = 1;
				m_state = CONF_REGISTERING;

				PTRACE(eLevelInfoNormal, "COneConf::OnDNSServiceIndConfREGISTERINGResolving, Resolution result: ProxyIp=0, Not sending REGISTER request.");
			}
		}
	}
	else
		DBGPASSERT(3);

	DEALLOCBUFFER(hostName);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnDNSServiceIndConfREGISTEREDResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnDNSServiceIndConfREGISTEREDResolving. conf = ",m_data->m_pConfName);
	WORD serviceId = 0;
	DWORD ipAddr = 0, port = 0, transport = 0;
	ALLOCBUFFER(hostName, DnsQueryNameSize);//H243_NAME_LEN);

	DeleteTimer(TIMER_DNS_RESOLVING);

	*pParam >> serviceId
			>> hostName
		//	>> ipAddr
			>> port
			>> transport;

//	mcTransportAddress	resolvedAddr;
//	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
//	pParam->Get((BYTE *)&resolvedAddr, sizeof(mcTransportAddress));
	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}
	pParam->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	// need to choose one Addres from the list
	mcTransportAddress	resolvedAddr;
	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
	GetAddressFromList(&resolvedAddr,pAddrList,m_data->m_ipType);

	if(serviceId == m_data->m_serviceId)
	{
		if(m_data->m_remove)
		{
			CSegment seg;
			OnUnRegisterRequestRegistered(&seg);
		}
		else
		{
			//if(ipAddr != 0  && !strncmp(m_data->m_pHostName, hostName,H243_NAME_LEN))
			if(!isApiTaNull(&resolvedAddr) && !strncmp(m_data->m_pHostName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
			{
				//m_data->m_proxyAddress.addr.v4.ip = ipAddr;
				memcpy(&m_data->m_proxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
				m_data->m_proxyAddress.port = port;

				//m_data->m_outboundProxyAddress.addr.v4.ip = ipAddr;
				memcpy(&m_data->m_outboundProxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
				m_data->m_outboundProxyAddress.port = port;
				m_data->m_transportType = transport;
				m_DNSResolvedIp = TRUE;
				m_state = CONF_REGISTERED;

				CSegment seg;
				OnRefreshTimer(&seg);
			}
			else
			{
				//start timer so we will try again to register again
				StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
				m_timerRegister = 1;
				m_state = CONF_REGISTERING;

				PTRACE(eLevelInfoNormal, "COneConf::OnDNSServiceIndConfREGISTEREDResolving, Resolution result: ProxyIp=0, Not sending REGISTER request.");
			}
		}
	}
	else
		DBGPASSERT(4);

	DEALLOCBUFFER(hostName);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnDNSServiceIndConfUNREGISTERINGResolving(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "COneConf::OnDNSServiceIndConfUNREGISTERINGResolving. conf = ",m_data->m_pConfName);
	WORD serviceId = 0;
	DWORD ipAddr = 0, port = 0, transport = 0;
	ALLOCBUFFER(hostName, DnsQueryNameSize);//H243_NAME_LEN);

	DeleteTimer(TIMER_DNS_RESOLVING);

	*pParam >> serviceId
			>> hostName
		//	>> ipAddr
			>> port
			>> transport;

//	mcTransportAddress	resolvedAddr;
//	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
//	pParam->Get((BYTE *)&resolvedAddr, sizeof(mcTransportAddress));
	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
	for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
	{
		memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
	}
	pParam->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

	// need to choose one Addres from the list
	mcTransportAddress	resolvedAddr;
	memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
	GetAddressFromList(&resolvedAddr,pAddrList,m_data->m_ipType);

	if(serviceId == m_data->m_serviceId)
	{
		//if(ipAddr != 0  && !strncmp(m_data->m_pHostName, hostName,H243_NAME_LEN))
		if(!isApiTaNull(&resolvedAddr) && !strncmp(m_data->m_pHostName, hostName,DnsQueryNameSize))//H243_NAME_LEN))
		{
			//m_data->m_proxyAddress.addr.v4.ip = ipAddr;
			memcpy(&m_data->m_proxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
			m_data->m_proxyAddress.port = port;

			//m_data->m_outboundProxyAddress.addr.v4.ip = ipAddr;
			memcpy(&m_data->m_outboundProxyAddress, &resolvedAddr, sizeof(mcTransportAddress));
			m_data->m_outboundProxyAddress.port = port;
			m_data->m_transportType = transport;
			m_data->m_status = CONF_UNREGISTERING;
			m_DNSResolvedIp = TRUE;
			m_state = CONF_UNREGISTERING;

			if(!RegisterRequest(0))
			{
				DEALLOCBUFFER(hostName);
				return;
			}

			// start Un-Register timer
			StartTimer(TIMER_START_UNREGISTER, UnRegisteringTimeOut*SECOND);
			m_timerUnRegister = 1;
		}
		else
			PTRACE(eLevelInfoNormal, "COneConf::OnDNSServiceIndConfUNREGISTERINGResolving, Resolution result: ProxyIp=0, Not sending REGISTER request.");
	}
	else
		DBGPASSERT(5);

	DEALLOCBUFFER(hostName);
}


/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnDNSResolvingTimer(CSegment* pParam)
{
	PTRACE2(eLevelError, "COneConf::OnDNSResolvingTimer. Failed to resolve Proxy. conf = ",m_data->m_pConfName);

	SIPProxyMngrApi sipProxyMngrApi;
	sipProxyMngrApi.CreateOnlyApi(*m_pSipMngrRcvMbx);

	if(m_data->m_remove)
	{
		m_state = CONF_UNREGISTERING;
		m_data->m_status = CONF_UNREGISTERING;
		if(m_DNSResolvedIp)
		{
			//m_data->m_proxyAddress.addr.v4.ip = 0;
			memset(&m_data->m_proxyAddress, 0, sizeof(mcTransportAddress));

			if(eConfSipServerAuto == m_data->m_serversConfig)
			{
				m_data->m_proxyAddress.port = 0;
			}
		}

		sipProxyMngrApi.KillOneConf(m_indInMngrDB);
	}

	if((!strncmp(m_data->m_pConfName, m_data->m_dummy_name, DnsQueryNameSize)) && (m_data->m_bIsFromOldService != TRUE))//H243_NAME_LEN))
	{
		eSipRegistrationConfType sipConfType;
		sipConfType = SipProxyServiceManagerGetConfType(m_data);

		sipProxyMngrApi.RegistrarStatusUpdate(STATUS_FAIL, m_data->m_pProxyName, m_data->m_serviceId, m_data->m_proxyAddress, m_data->m_confID, (WORD)sipConfType, m_data->m_expires);
	}
	sipProxyMngrApi.DestroyOnlyApi();

	//start timer so we will try again to register again
	StartTimer(TIMER_START_REGISTERING, RegisteringTimeOut*SECOND);
	m_timerRegister = 1;
	m_state = CONF_REGISTERING;
}

//////////////////////////////////////////////////////////////////////////////////
void  COneConf::ClearResolvedIps()
{
	if(eConfSipServerManually == m_data->m_serversConfig)
	{
		if(m_data->m_DNSStatus)
		{
			if(strcmp(m_data->m_pProxyName, ""))
			{
				m_data->m_proxyAddress.addr.v4.ip = 0;
				memset(m_data->m_proxyAddress.addr.v6.ip, 0, IPV6_ADDRESS_BYTES_LEN);
				//m_data->m_proxyPort = 0;
			}
			if(strcmp(m_data->m_poutboundProxyName, ""))
			{
				m_data->m_outboundProxyAddress.addr.v4.ip = 0;
				memset(m_data->m_outboundProxyAddress.addr.v6.ip, 0, IPV6_ADDRESS_BYTES_LEN);
				//m_data->m_outboundProxyPort = 0;
			}
		}
	}
	else
	{
		if(eConfSipServerAuto == m_data->m_serversConfig)
		{
			m_data->m_proxyAddress.addr.v4.ip = 0;
			memset(m_data->m_proxyAddress.addr.v6.ip, 0, IPV6_ADDRESS_BYTES_LEN);
			m_data->m_proxyAddress.port = 0;

			m_data->m_outboundProxyAddress.addr.v4.ip = 0;
			memset(m_data->m_outboundProxyAddress.addr.v6.ip, 0, IPV6_ADDRESS_BYTES_LEN);
			m_data->m_outboundProxyAddress.port = 0;
		}
	}
	m_DNSResolvedIp = FALSE;
}

//////////////////////////////////////////////////////////////////////////////////
//returns a random number in range 1 - 9
int  COneConf::GetRandomNumber()
{
// 	int randomNum = 0;
// 	char randString[60];
// 	randString[0] = 0;

//     sprintf(randString, "%5d", rand());
// 	randString[0] = randString[4];
// 	randString[1] = 0;
// 	randomNum = atoi(randString);

// 	if( 0 == randomNum)
// 		randomNum = 1;

// 	return randomNum;
    return (rand() % 9) + 1;

}

//////////////////////////////////////////////////////////////////////////////////
BYTE COneConf::CheckDHCPData(COneConfRegistration *pData)
{
	PTRACE(eLevelInfoNormal, "COneConf::CheckDHCPData, reading DHCP data.");
	BYTE result = FALSE;
/*	CCommDynCard* pCard;
	WORD line = 0xFFFF, serversConfig = 0;
	CIPService* pIpServ = NULL;
	CSip* pSip = NULL;
	CBaseSipServer*	pOutboundProxy = NULL;
	CSipServer* pRegistrar = NULL;
	CIpDns* pDNS = NULL;

	pCard = (CCommDynCard*) ::GetpCardDB()->GetCurrentCard( pData->m_serviceId );
	if(pCard)
	{
		if( ( (CCommDynCard*)pCard)->GetH323Spec() )
			line = ((CCommDynCard*)pCard)->GetH323Spec()->GetLineNumber();

		if( line != 0xFFFF )
		{
			pIpServ = (CIPService*)::GetpIPservList()->GetService(line);
			if(pIpServ)
			{
				pDNS = pIpServ->GetpDns();
				if(pDNS)
				{
					if(eConfSipServerAuto == pData->m_serversConfig)
					{
						if(pIpServ->GetDHCPServer())
						{
							CCommDynCard* pCard = (CCommDynCard*) ::GetpCardDB()->GetCurrentCard(pData->m_serviceId);
							if (pCard)
							{
								CCommH323Card* pCommH323Card = pCard->GetH323Spec();
								if(pCommH323Card)
								{
									CCommH323CardSIP CommH323CardSIP = pCommH323Card->GetCommH323CardSIP();
									if(CommH323CardSIP.GetNumberOfProxys() > 0)
									{
										pData->m_proxyIP = CommH323CardSIP.GetPrimaryProxyIP();
										strncpy(pData->m_pProxyName, CommH323CardSIP.GetPrimaryProxyName().GetString(), H243_NAME_LEN);
										pData->m_outboundProxyIP = pData->m_proxyIP;
										strncpy(pData->m_poutboundProxyName, pData->m_pProxyName, H243_NAME_LEN);
										pData->m_proxyPort = 5060;

										if(pData->m_proxyIP != 0 && strcmp(pData->m_pProxyName, ""))
											result = TRUE;
									}
								}
							}
						}
					}
				}
			}
		}
	}*/
	return result;
}


//////////////////////////////////////////////////////////////////////////////////
void COneConf::ResolveDomainReq(DWORD serviceId, char* pProxyName)
{
	PTRACE2(eLevelInfoNormal, "COneConf::ResolveDomainReq, resolving name=", pProxyName);
	CSegment*  pRetParam = new CSegment;
	*pRetParam << serviceId
			   << pProxyName
			   << (WORD)eProcessSipProxy;

	const COsQueue* pCsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessDNSAgent, eManager);

	STATUS res = pCsMbx->Send(pRetParam,DNS_RESOLVE_DOMAIN_REQ);
}


//////////////////////////////////////////////////////////////////////////////////
void COneConf::ServiceReq(DWORD serviceId, char* pHostName, eIPProtocolType eIPProtocolType_SIP, WORD transportType)
{
	PTRACE2(eLevelInfoNormal, "COneConf::ServiceReq, for domain=",pHostName);
	CSegment*  pRetParam = new CSegment;
	*pRetParam << serviceId
			   << pHostName
			   << (WORD)eIPProtocolType_SIP
			   << transportType
			   << (WORD)eProcessSipProxy;


	const COsQueue* pCsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessDNSAgent, eManager);

	STATUS res = pCsMbx->Send(pRetParam,DNS_SERVICE_REQ);
}
//////////////////////////////////////////////////////////////////////////////////
/*
BOOL COneConf::IsIpEqualToZero(mcTransportAddress Address)
{
	BOOL bIsIpEqualToZero = FALSE;
	if( (Address.ipVersion == eIpVersion6 && 0 == Address.addr.v6.ip) ||
	    (Address.ipVersion == eIpVersion4 && 0 == Address.addr.v4.ip) )
	  		bIsIpEqualToZero = TRUE;

	return bIsIpEqualToZero;
}*/

/////////////////////////////////////////////////////////////////////////////
void COneConf::SetCsIpArrayAccordingToIpTypeAndScopeId(COneConfRegistration* pData)
{
	TRACEINTO << __FUNCTION__ << "\n";

	enIpVersion eSipProxyIpVer = (enIpVersion)m_data->m_proxyAddress.ipVersion;
	ipAddressStruct	csIp[TOTAL_NUM_OF_IP_ADDRESSES];
	memset(&csIp,0,sizeof(ipAddressStruct)*TOTAL_NUM_OF_IP_ADDRESSES);
	int i = 0;
	int j = 0;
	BYTE isMatch = 0;
	enScopeId eSipProxyScopeId = eScopeIdOther;
	if (eSipProxyIpVer == eIpVersion6)
		eSipProxyScopeId = (enScopeId)m_data->m_proxyAddress.addr.v6.scopeId;

	if (eSipProxyIpVer == eIpVersion4 && eSipProxyIpVer == (enIpVersion)m_data->m_serviceAddrList[0].ipVersion)
	{
		// A private case for IpV4 as first address
		return;
	}
	char a[128];

	TRACEINTO << __FUNCTION__ << " First loop \n";
	for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
	{
		memset(a, '\0', 128);
		TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
		memset(a, '\0', 128);
		TRACEINTO << "m_data->m_serviceAddrList Arr : [" << i << "] - " << ipToString(m_data->m_serviceAddrList[i], a, 1);
	}

	// 1. Finding the exact address type and scope (In IpV6 case).
	for (i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
	{
		if (::IsIpNull(&(m_data->m_serviceAddrList[i])) == FALSE)
		{
			if (eSipProxyIpVer == (enIpVersion)m_data->m_serviceAddrList[i].ipVersion && eSipProxyIpVer == eIpVersion4)
			{
				memcpy(&(csIp[0]), &(m_data->m_serviceAddrList[i]), sizeof(ipAddressStruct));
				isMatch = 1;
				memset(&(m_data->m_serviceAddrList[i]), 0 , sizeof(ipAddressStruct));
			}
			else if (eSipProxyIpVer == (enIpVersion)m_data->m_serviceAddrList[i].ipVersion && eSipProxyIpVer == eIpVersion6)
			{
				if (m_data->m_serviceAddrList[i].addr.v6.scopeId == (DWORD)eSipProxyScopeId)
				{
					memcpy(&(csIp[0]), &(m_data->m_serviceAddrList[i]), sizeof(ipAddressStruct));
					isMatch = 1;
					memset(&(m_data->m_serviceAddrList[i]), 0, sizeof(ipAddressStruct));
				}
			}
		}
		if (isMatch)
			break;
	}
	TRACEINTO << __FUNCTION__ << " Second loop \n";
	for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
	{
		memset(a, '\0', 128);
		TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
		memset(a, '\0', 128);
		TRACEINTO << "m_data->m_serviceAddrList Arr : [" << i << "] - " << ipToString(m_data->m_serviceAddrList[i], a, 1);
	}

	// 2. Copying the rest of the addresses with the correct priority.
	if (isMatch)
	{
		// We found and located the first prioritized addr.
		// In this case we will copy the array according to Ip type.
		for (i = 1; i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
		{
			for (j = 0; j < TOTAL_NUM_OF_IP_ADDRESSES; j++)
			{
				if (::IsIpNull(&(m_data->m_serviceAddrList[j])) == FALSE)
				{
					if (eSipProxyIpVer == eIpVersion4)
					{
						memcpy(&(csIp[i]), &(m_data->m_serviceAddrList[j]), sizeof(ipAddressStruct));
						memset(&(m_data->m_serviceAddrList[j]), 0, sizeof(ipAddressStruct));
						break;
					}
					else if (eSipProxyIpVer == eIpVersion6)
					{
						if ((enIpVersion)m_data->m_serviceAddrList[j].ipVersion == eIpVersion6)
						{
							memcpy(&(csIp[i]), &(m_data->m_serviceAddrList[j]), sizeof(ipAddressStruct));
							memset(&(m_data->m_serviceAddrList[j]), 0, sizeof(ipAddressStruct));
							break;

						}
					}
				}
			}
		}
		TRACEINTO << __FUNCTION__ << " Third loop - MATCH  \n";
		for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
		{
			memset(a, '\0', 128);
			TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
			memset(a, '\0', 128);
			TRACEINTO << "m_data->m_serviceAddrList Arr : [" << i << "] - " << ipToString(m_data->m_serviceAddrList[i], a, 1);
		}


	}
	else
	{
		// This can only happen in IpV6 case - We have CS addresses but not in the matching ScopeId.
		// In this case - We simply copy all IpV6 addresses and then the V4 - That's the best match we can provide.
		for (i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
		{
			for (j = 0; j < TOTAL_NUM_OF_IP_ADDRESSES; j++)
			{
				if (::IsIpNull(&(m_data->m_serviceAddrList[j])) == FALSE)
				{
					if (eSipProxyIpVer == eIpVersion6)
					{
						if ((enIpVersion)m_data->m_serviceAddrList[j].ipVersion == eIpVersion6)
						{
							memcpy(&(csIp[i]), &(m_data->m_serviceAddrList[j]), sizeof(ipAddressStruct));
							memset(&(m_data->m_serviceAddrList[j]), 0, sizeof(ipAddressStruct));
							break;

						}
					}
				}
			}
		}
		TRACEINTO << __FUNCTION__ << " Third loop -  NO MATCH  \n";
		for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
		{
			memset(a, '\0', 128);
			TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
			memset(a, '\0', 128);
			TRACEINTO << "m_data->m_serviceAddrList Arr : [" << i << "] - " << ipToString(m_data->m_serviceAddrList[i], a, 1);
		}
	}
	// In case we copied all IpV6 addresses - We need to copy the V4 (If there is one).
	BYTE isIpv4Copy = 0;
	if (eSipProxyIpVer == eIpVersion6)
	{
		for (i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
		{
			if(::IsIpNull(&(csIp[i])) == TRUE)
			{
				for (j = 0; j < TOTAL_NUM_OF_IP_ADDRESSES ; j++)
				{
					if (::IsIpNull(&(m_data->m_serviceAddrList[j])) == FALSE)
					{
						memcpy(&(csIp[i]), &(m_data->m_serviceAddrList[j]), sizeof(ipAddressStruct));
						memset(&(m_data->m_serviceAddrList[j]), 0, sizeof(ipAddressStruct));
						isIpv4Copy = 1;
						break;

					}
				}
			}
			if (isIpv4Copy)
				break;
		}
	}
	TRACEINTO << __FUNCTION__ << " Last loop   \n";
	for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
	{
		memset(a, '\0', 128);
		TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
		memset(a, '\0', 128);
		TRACEINTO << "m_data->m_serviceAddrList Arr : [" << i << "] - " << ipToString(m_data->m_serviceAddrList[i], a, 1);
	}
	memcpy(&(m_data->m_serviceAddrList), csIp , sizeof(ipAddressStruct)*TOTAL_NUM_OF_IP_ADDRESSES);

	TRACEINTO << __FUNCTION__ << " After MEMCPY \n";
	for (i = 0;i < TOTAL_NUM_OF_IP_ADDRESSES ; i++ )
	{
		memset(a, '\0', 128);
		TRACEINTO << "Temp Arr : [" << i << "] - " << ipToString(csIp[i], a, 1);
		memset(a, '\0', 128);
		TRACEINTO << "m_data->m_serviceAddrList Arr : [" << i << "] - " << ipToString(m_data->m_serviceAddrList[i], a, 1);
	}

}
/////////////////////////////////////////////////////////////////////////////
WORD COneConf::GetTransportType()
{
	return m_data->m_transportType;
}
/////////////////////////////////////////////////////////////////////////////
mcTransportAddress COneConf::GetProxyAddress()
{
	return m_data->m_proxyAddress;
}
/////////////////////////////////////////////////////////////////////////////
void COneConf::BuildcontactUriIp(char* LocalUri)
{
	TRACEINTO << __FUNCTION__ << "\n";
	char ipV4or6IpAddressStr[IPV6_ADDRESS_LEN];

	for(int index=0; index<TOTAL_NUM_OF_IP_ADDRESSES; index++)
	{
		ipAddressStruct ipAddrSt = m_data->GetServiceAddress(index);

		if (!IsIpNull(&ipAddrSt))
		{
			TRACEINTO << "MS_IPV6: ipType:" << (DWORD)m_data->m_ipType << ", sipServerType:" << (DWORD)m_data->m_sipServerType
					  << ", outboundProxyVersion:" << (DWORD)m_data->m_outboundProxyAddress.ipVersion
					  << ", ProxyVersion:" << (DWORD)m_data->m_proxyAddress.ipVersion
					  << ", ipAddrStVersion:" << (DWORD)ipAddrSt.ipVersion;

			if ( (m_data->m_ipType==eIpType_Both && m_data->m_sipServerType==eSipServer_ms && m_data->m_outboundProxyAddress.ipVersion==ipAddrSt.ipVersion) || //MS_IPV6
			     (m_data->m_ipType==eIpType_IpV4 && eIpVersion4==ipAddrSt.ipVersion) ||
			     ( m_data->m_ipType==eIpType_IpV6 && eIpVersion6==ipAddrSt.ipVersion) )
			{
				if (ipAddrSt.ipVersion==eIpVersion4)
				{
					TRACEINTO << "MS_IPV6: eIpType_IpV4";
					SystemDWORDToIpString(ipAddrSt.addr.v4.ip, ipV4or6IpAddressStr);
				}
				else //eIpType_IpV6
				{
					TRACEINTO << "MS_IPV6: eIpType_IpV6";
					ipV6ToString(ipAddrSt.addr.v6.ip, ipV4or6IpAddressStr, TRUE);
				}

				strncpy(LocalUri, m_data->m_pConfName, H243_NAME_LEN);
				LocalUri[H243_NAME_LEN - 1] = '\0';
				///VNGFE-7595 if we already have the name as URI do not strcat the @host
				//if ( (strncmp(m_data->m_pConfName, "rmx", 3) != 0) ||
				if (strstr(m_data->m_pConfName, "@") == NULL )
				{
					strncat(LocalUri, "@", 1);		
					strncat(LocalUri, ipV4or6IpAddressStr, IPV6_ADDRESS_LEN);
				}
				PTRACE2(eLevelInfoNormal,"@@@ BuildcontactUriIp - LocalUri = ", LocalUri);
				break;
			}

		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::ChangePresence(BYTE presenceState)
{
	if(m_presenceState == presenceState)
		return;
	if(presenceState == SIP_PRESENCE_ONLINE)
		DispatchEvent(START_PRESENCE_ONLINE);
	else if (presenceState == SIP_PRESENCE_BUSY)
		DispatchEvent(START_PRESENCE_BUSY);
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnStartPresenceOnlineRegistered(CSegment* pParam)
{
  TRACEINTO << "COneConf::OnStartPresenceOnlineRegistered - function not implemented";
}

/////////////////////////////////////////////////////////////////////////////
void  COneConf::OnStartPresenceBusyRegistered(CSegment* pParam)
{
  TRACEINTO << "COneConf::OnStartPresenceBusyRegistered - function not implemented";
}

/////////////////////////////////////////////////////////////////////////////
void COneConf::AddPartyToRegisterWaitingPartiesVector(DWORD partyId, DWORD connId)
{
	/*TRACEINTO << "COneConf::AddPartyToRegisterWaitingPartiesVector - partyId " << partyId << " connId " << connId;
    StPartiesDetailes* pStruct = new StPartiesDetailes;
    pStruct->connId = connId;
    pStruct->partyId = partyId;
    m_PartiesWaiting.push_back(pStruct);*/

	TRACEINTO << "PartyId:" << partyId << ", ConnId:" << connId;
	m_PartiesWaiting.push_back(std::make_pair(connId, partyId));

	if( m_PartiesWaiting.size() == 1 )
	{
      TRACEINTO << "COneConf::AddPartyToRegisterWaitingPartiesVector sending register";
		if(IsValidTimer(TIMER_REFRESH_REGISTRATION))
		{
			DeleteTimer(TIMER_REFRESH_REGISTRATION);
			m_timerRefresh = 0;
		}
		if ( !RegisterRequest(m_data->m_expires) )
		{
			OnRegisterEmptyWaitingPartiesVector(0);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void COneConf::OnRegisterEmptyWaitingPartiesVector(DWORD status)
{
	TRACEINTO << "Status:" << status;
	// delete busy server vector:

	/*StPartiesDetailes* pStruct = NULL;
	  std::vector< StPartiesDetailes* >::iterator itr =  m_PartiesWaiting.begin();
	  while (itr != m_PartiesWaiting.end())
	  {
	    pStruct = (*itr);
	    CSegment *pSeg = new CSegment;
	 *pSeg << (DWORD)status;

	    SendReqToConfParty(DIALOG_RECOVERY_REQ, pStruct->connId, pStruct->partyId, pSeg);
	    m_PartiesWaiting.erase(itr);
	    POBJDELETE(pStruct);
	    itr = m_PartiesWaiting.begin();
	  }*/

	PartiesDetailesVec::iterator end = m_PartiesWaiting.end();
	for (PartiesDetailesVec::iterator itr = m_PartiesWaiting.begin(); itr != end; ++itr)
	{
		CSegment *pSeg = new CSegment;
		*pSeg << (DWORD)status;
		SendReqToConfParty(DIALOG_RECOVERY_REQ, itr->first, itr->second, pSeg);
	}

	m_PartiesWaiting.clear();
}



/////////////////////////////////////////////////////////////////////////////
void  COneConf::SetTransportErrorState()
{
	m_presenceNeedToRepublish = TRUE;
}


//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMsOneConf
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//===========================================================================
// Start message map -------------------------------------------
PBEGIN_MESSAGE_MAP(CMsOneConf)
	ONEVENT(SUBSCRIBING_OK,				CONF_REGISTERED,		CMsOneConf::OnSubscribingOKRegistered)
	ONEVENT(SUBSCRIBING_FAILED,			CONF_REGISTERED,		CMsOneConf::OnSubscribingFailedRegistered)
	ONEVENT(SUBSCRIBING_OK,				CONF_UNREGISTERING,		CMsOneConf::NullActionFunction)
	ONEVENT(SERVICE_IND_OK,				CONF_REGISTERED,		CMsOneConf::OnServiceIndOKRegistered)
	ONEVENT(SERVICE_IND_FAILED,			CONF_REGISTERED,		CMsOneConf::OnServiceIndFailedRegistered)
	ONEVENT(SERVICE_IND_OK,				CONF_UNREGISTERING,		CMsOneConf::OnServiceIndOKRegistered)
	ONEVENT(SERVICE_IND_FAILED,			CONF_UNREGISTERING,		CMsOneConf::OnServiceIndFailedRegistered)
	ONEVENT(START_PRESENCE_ONLINE,		CONF_REGISTERED,		CMsOneConf::OnStartPresenceOnlineRegistered)
	ONEVENT(START_PRESENCE_ONLINE,		CONF_UNREGISTERING,		CMsOneConf::NullActionFunction)
	ONEVENT(START_PRESENCE_ONLINE,		CONF_IDLE_DNS_RESOLVING,CMsOneConf::NullActionFunction )

	ONEVENT(START_PRESENCE_BUSY,		CONF_REGISTERED,		CMsOneConf::OnStartPresenceBusyRegistered)
	ONEVENT(START_PRESENCE_BUSY,		CONF_UNREGISTERING,		CMsOneConf::NullActionFunction)
	ONEVENT(START_PRESENCE_BUSY,		CONF_IDLE_DNS_RESOLVING,CMsOneConf::NullActionFunction )

PEND_MESSAGE_MAP(CMsOneConf,COneConf);
// End message map ---------------------------------------------

/////////////////////////////////////////////////////////////////////////////
CMsOneConf::CMsOneConf() : COneConf::COneConf()
{
	m_pMsPresence = NULL;
}
/////////////////////////////////////////////////////////////////////////////
CMsOneConf::~CMsOneConf()
{
	POBJDELETE(m_pMsPresence);
}
/////////////////////////////////////////////////////////////////////////////
const char* CMsOneConf:: NameOf() const
{
	return "CMsOneConf";
}
/////////////////////////////////////////////////////////////////////////////
void CMsOneConf::SetProxyServerType(APIS8 HeaderField,char* Header)
{
	if ((HeaderField == kAllowEvent) && (strstr(Header, "microsoft")))
		m_proxyType = eOCS;
}
/////////////////////////////////////////////////////////////////////////////
void CMsOneConf::SetProxyServerVersionType(APIS8 HeaderField,char* Header)
{
	if(kUserAgent == HeaderField) {
		if(strstr(Header, "RTC/4.0") || strstr(Header, "RTC/5.0"))
		{
			m_proxyVersionType = eLync;
		}
		if(strstr(Header, "RTC/3.5"))
		{
			m_proxyVersionType = eW13;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CMsOneConf::CallSubscribe(BOOL IsFirstRegisteration)
{
	WORD currentPresentedConfNumber = 0;

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
	WORD maxNumOfPossiblePresenceConf = 0;
	if (eProductTypeRMX2000 == curProductType  || eProductTypeRMX1500 == curProductType ||
	    eProductFamilySoftMcu == curProductFamily/*all SoftMcu products*/) // otherwise it was done in 'HandleNetworkSeparationConfigurations' method
	{
		maxNumOfPossiblePresenceConf = MAX_NUM_OF_PRESENCE_CONF_RMX2000;
	}
	else if (eProductTypeRMX4000 == curProductType)
	{
		maxNumOfPossiblePresenceConf = MAX_NUM_OF_PRESENCE_CONF_RMX4000;
	}
	else
	{
		maxNumOfPossiblePresenceConf = MAX_NUM_OF_PRESENCE_CONF_RMX2000;
		PTRACE(eLevelInfoNormal,"CMsOneConf::CallSubscribe - unknown product type");
	}

	currentPresentedConfNumber = ::GetPresentedConfNumber();
	if(IsFirstRegisteration)
	{
		PTRACE(eLevelInfoNormal,"CMsOneConf::CallSubscribe- First registration");
		if(m_proxyType == eOCS)
		{

			if(currentPresentedConfNumber >= maxNumOfPossiblePresenceConf)
			{
				PTRACE(eLevelInfoNormal,"CMsOneConf::CallSubscribe - presented conference number exceeded");
				return;
			}

			if(!m_pMsPresence)
			{
				 // Start presence flow:
				m_pMsPresence = new CSipProxyMsPresence;

				char LocalUri[MaxAddressListSize];
				memset(LocalUri, '\0', MaxAddressListSize);

				//yossi
				if( m_GruuContact[0] == '\0')
				   BuildcontactUriIp(LocalUri);
				else
				   strncpy(LocalUri, m_GruuContact, MaxAddressListSize-1);


				m_pMsPresence->create(m_pSipMngrRcvMbx, GetId(),m_src_unit_id,GetTransportType(),GetProxyAddress(),LocalUri);
				if(m_proxyVersionType == eLync)
				{
					m_pMsPresence->StartPresence(m_data->m_pConfName, m_data->m_pHostName, m_data->m_serviceId, TRUE);
					m_presenceState = SIP_PRESENCE_ONLINE;
				}
				else
				{
					m_pMsPresence->StartPresence(m_data->m_pConfName, m_data->m_pHostName, m_data->m_serviceId, FALSE);
					m_presenceState = SIP_PRESENCE_ONLINE;
				}
			}
			else
			{
				if(m_pMsPresence->GetIsNeedToReSubscribe() && (m_data->m_status != CONF_UNREGISTERING))
				{
					if(currentPresentedConfNumber >= maxNumOfPossiblePresenceConf)
					{
						PTRACE(eLevelInfoNormal,"CMsOneConf::CallSubscribe - presented conference number exceeded. Not try to presence again");
						return;
					}
					PTRACE(eLevelInfoNormal,"CMsOneConf::CallSubscribe - We try to presence again.");
					if(m_proxyVersionType == eLync)
					{
						m_pMsPresence->StartPresence(m_data->m_pConfName, m_data->m_pHostName, m_data->m_serviceId, TRUE);
						m_presenceState = SIP_PRESENCE_ONLINE;
					}
					else
					{
						m_pMsPresence->StartPresence(m_data->m_pConfName, m_data->m_pHostName, m_data->m_serviceId, FALSE);
						m_presenceState = SIP_PRESENCE_ONLINE;
					}
				}
				else if(m_data->m_status == CONF_UNREGISTERING) // check if need to unsubscribe!!!
				{
					PTRACE2INT(eLevelError,"CMsOneConf::CallSubscribe. Presented conf number before decrease is", currentPresentedConfNumber);
					::DecreasePresentedConfNumber();
					if(m_proxyVersionType == eLync)
					{
						m_pMsPresence->EndPresence(m_data->m_pConfName, m_data->m_pHostName, m_data->m_serviceId, TRUE);
						m_presenceState = SIP_PRESENCE_OFFLINE;
					}
					PTRACE(eLevelInfoNormal,"CMsOneConf::CallSubscribe - unsubscribe");
				}
				else if (TRUE == m_presenceNeedToRepublish) // after 408 re register
				{
					if (m_pMsPresence)
					{
						//send Service together with re-register
						if(SIP_PRESENCE_ONLINE == m_presenceState )
						{
							m_pMsPresence->SendSelfSubscribeAndSaveLastPresenceState(SIP_PRESENCE_ONLINE);
						}
						else if (SIP_PRESENCE_BUSY == m_presenceState)
						{
							m_pMsPresence->SendSelfSubscribeAndSaveLastPresenceState(SIP_PRESENCE_BUSY);
						}
					}
				}
			}
		}
		else
			PTRACE(eLevelInfoNormal,"CMsOneConf::CallSubscribe - Not OCS server");
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CMsOneConf::CallSubscribe- non first registration");
		if(m_proxyType == eOCS)
		{
			if(m_pMsPresence)
			{
				if(m_pMsPresence->GetIsNeedToReSubscribe() && (m_data->m_status != CONF_UNREGISTERING))
				{
					if(currentPresentedConfNumber >= maxNumOfPossiblePresenceConf)
					{
						PTRACE(eLevelInfoNormal,"CMsOneConf::CallSubscribe - presented conference number exceeded. Not try to presence again");
						return;
					}
					PTRACE(eLevelInfoNormal,"CMsOneConf::CallSubscribe - We try to presence again.");
					if(m_proxyVersionType == eLync)
					{
						m_pMsPresence->StartPresence(m_data->m_pConfName, m_data->m_pHostName, m_data->m_serviceId, TRUE);
						m_presenceState = SIP_PRESENCE_ONLINE;
					}
					else
					{
						m_pMsPresence->StartPresence(m_data->m_pConfName, m_data->m_pHostName, m_data->m_serviceId, FALSE);
						m_presenceState = SIP_PRESENCE_ONLINE;
					}
				}
				else if(m_data->m_status == CONF_UNREGISTERING) // check if need to unsubscribe!!!
				{
					PTRACE2INT(eLevelError,"CMsOneConf::CallSubscribe. Presented conf number before decrease is", currentPresentedConfNumber);
					::DecreasePresentedConfNumber();
					if(m_proxyVersionType == eLync)
					{
						m_pMsPresence->EndPresence(m_data->m_pConfName, m_data->m_pHostName, m_data->m_serviceId, TRUE);
						m_presenceState = SIP_PRESENCE_OFFLINE;
					}
					PTRACE(eLevelInfoNormal,"CMsOneConf::CallSubscribe - unsubscribe");
				}
				else if (TRUE == m_presenceNeedToRepublish && (m_data->m_status != CONF_UNREGISTERING))
				{
					if(m_pMsPresence)
					{
						//send Service together with re-register
						if(SIP_PRESENCE_ONLINE == m_presenceState)
						{
							m_pMsPresence->SendSelfSubscribeAndSaveLastPresenceState(SIP_PRESENCE_ONLINE);
						}
						else if (SIP_PRESENCE_BUSY == m_presenceState)
						{
							m_pMsPresence->SendSelfSubscribeAndSaveLastPresenceState(SIP_PRESENCE_BUSY);
						}
					}
				}

			}

		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CMsOneConf::OnSubscribingOKRegistered(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMsOneConf::OnSubscribingOKRegistered : Conf = ", m_data->m_pConfName);
	if (m_pMsPresence)
		m_pMsPresence->HandleSubscribeResponse(pParam, SUBSCRIBE_OK);
	else
		PASSERT (100);
}
/////////////////////////////////////////////////////////////////////////////
void CMsOneConf::OnServiceIndOKRegistered(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMsOneConf::OnServiceIndOKRegistered : Conf = ", m_data->m_pConfName);
	if (m_pMsPresence)
	{
		m_pMsPresence->HandleServiceResponse(pParam, SERVICE_OK);
		if(TRUE == m_presenceNeedToRepublish)
			m_presenceNeedToRepublish = FALSE;
	}
	else
		PASSERT (100);
}
/////////////////////////////////////////////////////////////////////////////
void CMsOneConf::OnSubscribingFailedRegistered(CSegment* pParam)
{
	DBGPASSERT(1);
	PTRACE2(eLevelInfoNormal, "CMsOneConf::OnSubscribingFailedRegistered : Conf = ", m_data->m_pConfName);
	if (m_pMsPresence)
		m_pMsPresence->HandleSubscribeResponse(pParam, SUBSCRIBE_FAILED);
	else
		PASSERT (100);
}
/////////////////////////////////////////////////////////////////////////////
void CMsOneConf::OnServiceIndFailedRegistered(CSegment* pParam)
{
	DBGPASSERT(1);
	PTRACE2(eLevelInfoNormal, "CMsOneConf::OnServiceIndFailedRegistered : Conf = ", m_data->m_pConfName);
	if (m_pMsPresence)
		m_pMsPresence->HandleServiceResponse(pParam, SERVICE_FAILED);
	else
		PASSERT (100);
}

/////////////////////////////////////////////////////////////////////////////
void  CMsOneConf::OnStartPresenceOnlineRegistered(CSegment* pParam)
{
	if(m_pMsPresence)
	{
		m_presenceState = SIP_PRESENCE_ONLINE;
		m_pMsPresence->ChangePresenceState(SIP_PRESENCE_ONLINE);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CMsOneConf::OnStartPresenceBusyRegistered(CSegment* pParam)
{
	if(m_pMsPresence)
	{
		m_presenceState = SIP_PRESENCE_BUSY;
		m_pMsPresence->ChangePresenceState(SIP_PRESENCE_BUSY);
	}
}
//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 	-------------		CMsIceUSer  ----------------------------------------
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//===========================================================================

// start message map -------------------------------------------
PBEGIN_MESSAGE_MAP(CMsIceUser)
    // conf events
  ONEVENT(REGISTERING_OK,				CONF_REGISTERING,		CMsIceUser::OnRegisteringOKRegistering)
  ONEVENT(SUBSCRIBING_OK,				CONF_REGISTERED,		CMsIceUser::OnSubscribingOKRegistered)
  ONEVENT(SUBSCRIBING_OK,				CONF_UNREGISTERING,		CMsIceUser::NullActionFunction)
  ONEVENT(SUBSCRIBING_FAILED,			CONF_REGISTERED,		CMsIceUser::OnSubscribingFailedRegistered)
  ONEVENT(NOTIFY_IND,					CONF_REGISTERED,		CMsIceUser::OnNotifyIndRegistered)

  ONEVENT(SERVICE_IND_OK,				CONF_REGISTERED,		CMsIceUser::OnServiceOKRegistered)
  ONEVENT(SERVICE_IND_FAILED,			CONF_REGISTERED,		CMsIceUser::OnServiceFailedRegistered)
  ONEVENT(END_ICE_INIT,					CONF_REGISTERED,		CMsIceUser::OnCSEndIceInit)

  ONEVENT(TIMER_REFRESH_SUBSCRIBE,		CONF_REGISTERED,		CMsIceUser::OnSubscribRefreshTimer	)
ONEVENT(TIMER_REFRESH_SUBSCRIBE,		ANYCASE		   ,		CMsIceUser::NullActionFunction	)
//  ONEVENT(LOADACCEPTMSGACK,			ANYCASE,			COneConf::NullActionFunction	)

PEND_MESSAGE_MAP(CMsIceUser,COneConf);
// end   message map -------------------------------------------

/////////////////////////////////////////////////////////////////////////////
CMsIceUser::CMsIceUser() : COneConf::COneConf()
{


}
/////////////////////////////////////////////////////////////////////////////
CMsIceUser::~CMsIceUser()
{
	if (m_pSubScriber)
	{
		POBJDELETE(m_pSubScriber);
	}
}
/////////////////////////////////////////////////////////////////////////////
const char* CMsIceUser:: NameOf() const
{
	return "CMsIceUser";
}
/////////////////////////////////////////////////////////////////////////////
int CMsIceUser::Create( COneConfRegistration *data, unsigned int indInDB, COsQueue* pRcvMbx , CMplMcmsProtocol* pMockMplProtocol)
{
	//COneConf::Create()
	PTRACE(eLevelInfoNormal, "CMsIceUser::Create");

	m_pSipMngrRcvMbx = pRcvMbx;
	m_data = data;	// gets the conf DB pointer
	if (!data)
	{
		PASSERT(1);
		return 1;
	}
	if(indInDB >= MAX_CONF_REGISTRATIONS)
	{
		PASSERT(2);
		return 2;
	}

	m_indInMngrDB = indInDB;

	m_callId = new char[MaxLengthOfSipCallId];
	m_pMockMplProtocol = pMockMplProtocol;

	m_pSubScriber = NULL;

	return 0;
}
/////////////////////////////////////////////////////////////////////////////
void  CMsIceUser::CallSubscribe(BOOL IsFirstRegisteration)
{

	if(eOCS == m_proxyType)
	{
		//After refresh in case the prev subscribe failed we will try to subscribe again
		if(IsValidPObjectPtr(m_pSubScriber) && !IsFirstRegisteration)
		{
			if(m_pSubScriber->GetIsNeedToReSubscribe())
			{
				PTRACE(eLevelInfoNormal,"CMsIceUser::CallSubscribe - We try to subscribe again.");
				m_pSubScriber->StartSubscribe(m_data->m_pConfName,m_data->m_pHostName,m_data->m_serviceId);
			}
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CMsIceUser::CallSubscribe - First subscribe.");
			m_pSubScriber = new CSipProxyMsSubscriber();

			char LocalUri[MaxAddressListSize];
			memset(LocalUri, '\0', MaxAddressListSize);

			if( m_GruuContact[0] == '\0')
			   BuildcontactUriIp(LocalUri);
			else
			   strncpy(LocalUri, m_GruuContact, MaxAddressListSize-1);

		m_pSubScriber->create(m_pSipMngrRcvMbx, GetId(),m_src_unit_id,GetTransportType(),GetProxyAddress(),LocalUri);
			m_pSubScriber->StartSubscribe(m_data->m_pConfName,m_data->m_pHostName,m_data->m_serviceId);
		}
	}
	else
		PTRACE(eLevelInfoNormal,"CMsIceUser::CallSubscribe - Not OCS server");
}
/////////////////////////////////////////////////////////////////////////////
void CMsIceUser::SetProxyServerType(APIS8 HeaderField,char* Header)
{
	if(kAllowEvent == HeaderField)
		if(strstr(Header, "microsoft"))
		{
			m_proxyType = eOCS;
		}
}
/////////////////////////////////////////////////////////////////////////////
void CMsIceUser::SetProxyServerVersionType(APIS8 HeaderField,char* Header)
{
	if(kUserAgent == HeaderField) {
		if(strstr(Header, "RTC/4.0") || strstr(Header, "RTC/5.0"))
		{
			m_proxyVersionType = eLync;
		}
		if(strstr(Header, "RTC/3.5"))
		{
			m_proxyVersionType = eW13;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CMsIceUser::OnSubscribingOKRegistered(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMsIceUser::OnSubscribingOKRegistered  , conf = ",m_data->m_pConfName);

	mcIndSubscribeResp * pSubscribeResponseMsg = (mcIndSubscribeResp *)pParam->GetPtr(1);

	CSipHeaderList * pHeaders = new CSipHeaderList(pSubscribeResponseMsg->sipHeaders);
	const CSipHeader* pCallIdHeader = pHeaders->GetNextHeader(kCallId);
	if(pCallIdHeader)
	{
		const char* pCallId = pCallIdHeader->GetHeaderStr();
		if (pCallId)  //isn't callId already equal to m_CallId????
		{
			PTRACE2(eLevelInfoNormal, "CMsIceUser::OnSubscribingOKRegistered, CallId=", pCallId);
			strncpy(m_callId, pCallId, MaxLengthOfSipCallId-1);
		}
	}
	POBJDELETE(pHeaders);

	if (m_pSubScriber)
		m_pSubScriber->HandleSubscribeResponse(pParam,SUBSCRIBE_OK);
	else
		PASSERT (100);
}
/////////////////////////////////////////////////////////////////////////////
void CMsIceUser::OnSubscribingFailedRegistered(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMsIceUser::OnSubscribingFailedRegistered  , conf = ",m_data->m_pConfName);

	mcIndSubscribeResp * pSubscribeResponseMsg = (mcIndSubscribeResp *)pParam->GetPtr(1);

	CSipHeaderList * pHeaders = new CSipHeaderList(pSubscribeResponseMsg->sipHeaders);
	const CSipHeader* pCallIdHeader = pHeaders->GetNextHeader(kCallId);
	if(pCallIdHeader)
	{
		const char* pCallId = pCallIdHeader->GetHeaderStr();
		if (pCallId)  //isn't callId already equal to m_CallId????
		{
			PTRACE2(eLevelInfoNormal, "COneConf::OnSubscribingOKRegistered, CallId=", pCallId);
			strncpy(m_callId, pCallId, MaxLengthOfSipCallId-1);
		}

	}
	POBJDELETE(pHeaders);

    	if (m_pSubScriber)
		m_pSubScriber->HandleSubscribeResponse(pParam,SUBSCRIBE_FAILED);
    	else
        	PASSERT (100);

}
/////////////////////////////////////////////////////////////////////////////
void CMsIceUser::OnNotifyIndRegistered(CSegment* pParam)
{
    	if (m_pSubScriber)
		m_pSubScriber->HandleNotifyInd(pParam);
    	else
        	PASSERT (100);
}
/////////////////////////////////////////////////////////////////////////////
void CMsIceUser::OnServiceOKRegistered(CSegment* pParam)
{
 	PTRACE2(eLevelInfoNormal, "CMsIceUser::OnServiceOKRegistered  , conf = ",m_data->m_pConfName);

 	mcIndServiceResp * pServiceResponseMsg = (mcIndServiceResp *)pParam->GetPtr(1);

 	CSipHeaderList * pHeaders = new CSipHeaderList(pServiceResponseMsg->sipHeaders);

 	const CSipHeader* pCallIdHeader = pHeaders->GetNextHeader(kCallId);
 	const CSipHeader* pDurationHeader = pHeaders->GetNextHeader(kCredDuration);

 	if(pCallIdHeader)
 	{
 		const char* pCallId = pCallIdHeader->GetHeaderStr();
 		if (pCallId)  //isn't callId already equal to m_CallId????
 		{
 			PTRACE2(eLevelInfoNormal, "CMsIceUser::OnServiceOKRegistered, CallId=", pCallId);
 			strncpy(m_callId, pCallId, MaxLengthOfSipCallId-1);
 		}

 	}
 	if(pDurationHeader)
 	{
 		const char* pDuration=NULL;
 		pDuration = pDurationHeader->GetHeaderStr();
 		if(pDuration)
 		{
 			DWORD ServiceRefreshTimer = atoi(pDuration);
 			if(ServiceRefreshTimer)
 			{
 				PTRACE2INT(eLevelInfoNormal,"CMsIceUser::OnServiceOKSubscribed : Duration = ",ServiceRefreshTimer);
 				StartTimer(TIMER_REFRESH_SUBSCRIBE, (ServiceRefreshTimer - RefreshTimeOutDeviation) * SECOND*60);//minutes
			//	m_timerSubscribeRefresh = 1;
 			}
 		}
 	}

 	POBJDELETE(pHeaders);
    if (m_pSubScriber)
        m_pSubScriber->HandleServiceResponse(pParam,SERVICE_OK);
    else
        PASSERT (100);

}
/////////////////////////////////////////////////////////////////////////////
void CMsIceUser::OnServiceFailedRegistered(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMsIceUser::OnServiceFailedRegistered  , conf = ",m_data->m_pConfName);

	if (pParam != NULL)
	{
		mcIndServiceResp * pServiceResponseMsg = (mcIndServiceResp *)pParam->GetPtr(1);

		if (pServiceResponseMsg != NULL)
		{
			CSipHeaderList * pHeaders = new CSipHeaderList(pServiceResponseMsg->sipHeaders);
			const CSipHeader* pCallIdHeader = pHeaders->GetNextHeader(kCallId);
			if(pCallIdHeader)
			{
				const char* pCallId = pCallIdHeader->GetHeaderStr();
				if (pCallId)  //isn't callId already equal to m_CallId????
				{
					PTRACE2(eLevelInfoNormal, "COneConf::OnServiceFailedRegistered, CallId=", pCallId);
					strncpy(m_callId, pCallId, MaxLengthOfSipCallId-1);
				}

			}
			POBJDELETE(pHeaders);

			if (m_pSubScriber)
				m_pSubScriber->HandleServiceResponse(pParam,SERVICE_FAILED);
			else
				PASSERT (100);
		}
		else
		{
			PTRACE(eLevelInfoNormal, "COneConf::OnServiceFailedRegistered, pServiceResponseMsg NULL - do nothing");
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal, "COneConf::OnServiceFailedRegistered, pParam NULL - do nothing");
	}
}
/////////////////////////////////////////////////////////////////////////////
void CMsIceUser::OnCSEndIceInit(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMsIceUser::OnCSEndIceInit  , conf = ",m_data->m_pConfName);

    	if (m_pSubScriber)
		m_pSubScriber->HandleCSICEInitResponse(pParam,END_INIT_ICE_IND);
    	else
        	PASSERT (100);

}
 /////////////////////////////////////////////////////////////////////////////
/* void  CMsIceUser::ResolveHostName(char* HostName)
 {
 	if(strcmp(HostName, ""))
 	{
 		ResolveDomainReq(m_data->m_serviceId, HostName);
 		m_state = CONF_IDLE_DNS_RESOLVING;
 	}
 }
*/
/////////////////////////////////////////////////////////////////////////////
void CMsIceUser::OnSubscribRefreshTimer(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMsIceUser::OnSubscribRefreshTimer, conf = ",m_data->m_pConfName);

	//	m_timerSubscribeRefresh = 0;

    	if (m_pSubScriber)
		m_pSubScriber->StartSubscribe(m_data->m_pConfName,m_data->m_pHostName, m_data->m_serviceId);

    	else
        	PASSERT (100);
}
/////////////////////////////////////////////////////////////////////////////
void CMsIceUser::SetRegisterActiveAlarm(BYTE IsAdd)
{
	PTRACE2(eLevelInfoNormal, "CMsIceUser::AddRegisterActiveAlarm, conf = ",m_data->m_pConfName);

	CProcessBase *pProcess = CProcessBase::GetProcess();

	if(IsAdd)
	{
		pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
		AA_REGISTER_ICE_USER_FAILED,
		MAJOR_ERROR_LEVEL,
		"Failed to register with OCS. Check the RMX Server Name.",
		true,
		true);

	}
	else
	{
		pProcess->RemoveActiveAlarmFromProcess(AA_REGISTER_ICE_USER_FAILED);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CMsIceUser::UpdateRegisterState()
{
	PTRACE2(eLevelInfoNormal, "CMsIceUser::UpdateRegisterState - REGISTERED, conf = ",m_data->m_pConfName);
	m_state = CONF_REGISTERED;
}
//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 	-------------		CStandIceUser  ----------------------------------------
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//===========================================================================
// start message map -------------------------------------------
PBEGIN_MESSAGE_MAP(CStandIceUser)
    // conf events
  ONEVENT(REGISTERING_OK,				CONF_IDLE,		CStandIceUser::OnRegisteringOKRegistering)
  ONEVENT(END_ICE_INIT,					CONF_IDLE,		CStandIceUser::OnCSEndIceInit)
  ONEVENT(DNS_RESOLVE_IND,				CONF_ICE_CONNECT,	CStandIceUser::OnConfIceDnsResolved)
  ONEVENT(DNS_RESOLVE_IND,				ANYCASE,			CStandIceUser::OnConfIceDnsResolved)
  ONEVENT(CS_DNS_AGENT_RESOLVE_IND,		ANYCASE,			CStandIceUser::OnConfIceDnsResolved)
  ONEVENT(TIMER_DNS_RESOLVING,          ANYCASE,			CStandIceUser::OnConfIceDnsResolveTimerExpired)

PEND_MESSAGE_MAP(CStandIceUser,COneConf);
// end   message map -------------------------------------------

/////////////////////////////////////////////////////////////////////////////
CStandIceUser::CStandIceUser() : COneConf::COneConf()
{
	m_PassServerResolved = FALSE;
	m_StunServerResolved = FALSE;
	m_TurnServerResolved = FALSE;
}
/////////////////////////////////////////////////////////////////////////////
CStandIceUser::~CStandIceUser()
{

}
/////////////////////////////////////////////////////////////////////////////
const char* CStandIceUser:: NameOf() const
{
	return "CStandIceUser";
}

/////////////////////////////////////////////////////////////////////////////
int CStandIceUser::Create( COneConfRegistration *data, unsigned int indInDB, COsQueue* pRcvMbx , CMplMcmsProtocol* pMockMplProtocol,CSipProxyIpParams* pService)
{
	//COneConf::Create()
	PTRACE(eLevelInfoNormal, "CStandIceUser::Create");
	char HostToResolve[H243_NAME_LEN];

	m_pSipMngrRcvMbx = pRcvMbx;
	m_data = data;	// gets the conf DB pointer
	if (!data)
	{
		PASSERT(1);
		return 1;
	}
	if(indInDB >= MAX_CONF_REGISTRATIONS)
	{
		PASSERT(2);
		return 2;
	}

	m_indInMngrDB = indInDB;

	m_callId = new char[MaxLengthOfSipCallId];
	m_pMockMplProtocol = pMockMplProtocol;
	m_serviceId = pService->GetServiceId();


	strncpy(m_stunServerHostName, pService->GetStunServerHostName(), H243_NAME_LEN-1); //kw added -1
	strncpy(m_RelayServerHostName, pService->GetRelayServerHostName(), H243_NAME_LEN-1); //kw added -1
	strncpy(m_stunPassServerHostName, pService->GetSTUNpassHostName(), H243_NAME_LEN-1);//kw added -1

	memset(HostToResolve, '\0', H243_NAME_LEN);

	needToResolveHostname(HostToResolve);

	if ( m_PassServerResolved && m_StunServerResolved && m_TurnServerResolved)
	{
		//IPV4/IPv6 configuration, we have all needed data, continue with ICE_INIT_REQ
		m_state = CONF_IDLE;

		FillICEparams(pService);

		UpdateCM();

	} else if(isalnum(HostToResolve[0]) && strncmp(HostToResolve, "0.0.0.0",strlen("0.0.0.0")) !=0 )
	{
		PTRACE2(eLevelError, "CStandIceUser::Create host name need to be resolve: ", HostToResolve);
		CSegment *pTimerSegment = new CSegment;

		FillICEparams(pService);

		m_state = CONF_ICE_DNS_RESOLVING;

		ResolveDomainReq(m_serviceId, HostToResolve);

		*pTimerSegment << (DWORD) m_serviceId << HostToResolve;
		StartTimer(TIMER_DNS_RESOLVING, RegisteringTimeOut*SECOND, pTimerSegment);
		//should we delete pTimerSegment;
	} else {

		//invalid HostName or empty setup
		DBGFPASSERT(99);
		SetRegisterActiveAlarm(TRUE);
	}

	return 0;
}

void CStandIceUser::needToResolveHostname(char *HostToResolve)
{

	memset(HostToResolve, '\0', H243_NAME_LEN);

	if(m_stunServerHostName[0] != 0) // we ask about 0 and not about '0' this would be dealt later
	{
		//we have turn server name only
		if((isIpV4Str(m_stunServerHostName)) || (isIpV6Str(m_stunServerHostName)))
		{

			PTRACE2(eLevelError, "CStandIceUser::Create, Found IP for STUN Server ", m_RelayServerHostName);
			strncpy(m_stunServerUDPAddress.sIpAddr, m_stunServerHostName, H243_NAME_LEN);
			strncpy(m_stunServerTCPAddress.sIpAddr, m_stunServerHostName, H243_NAME_LEN);
			m_StunServerResolved = TRUE;

		} else {

			PTRACE2(eLevelError, "CStandIceUser::Create, TURN Server name need to be resolved ", m_stunServerHostName);
			strncpy(HostToResolve, m_stunServerHostName, H243_NAME_LEN);

		}
	}
	if(HostToResolve[0] == 0)
	{

		if(m_RelayServerHostName[0] != 0)
		{
			if( (isIpV4Str(m_RelayServerHostName)) || (isIpV6Str(m_RelayServerHostName)) )
			{
				PTRACE2(eLevelError, "CStandIceUser::Create, Found IP for TURN Server ", m_RelayServerHostName);
				strncpy(m_RelayServerUDPAddress.sIpAddr, m_RelayServerHostName, H243_NAME_LEN);
				strncpy(m_RelayServerTCPAddress.sIpAddr, m_RelayServerHostName, H243_NAME_LEN);
				m_TurnServerResolved = TRUE;

			} else {

				PTRACE2(eLevelError, "CStandIceUser::Create, TURN Server name need to be resolved ", m_RelayServerHostName);
				strncpy(HostToResolve, m_RelayServerHostName, H243_NAME_LEN);
			}
		}
	}

	if(HostToResolve[0] == 0)
	{
		if(m_stunPassServerHostName[0] == 0)
		{
			//No password server defined, that's fine we don't really need it.
			memset(m_stunPassServerAddress.sIpAddr, 0, sizeof(m_stunPassServerAddress.sIpAddr));
			m_PassServerResolved = TRUE;
		}
		else if( (isIpV4Str(m_stunPassServerHostName)) || (isIpV6Str(m_stunPassServerHostName)))
		{
			strncpy(m_stunPassServerAddress.sIpAddr, m_stunPassServerHostName, H243_NAME_LEN);
			m_PassServerResolved = TRUE;
		}
		else
		{
			PTRACE2(eLevelError, "CStandIceUser::Create, Password Server name need to be resolved", m_stunPassServerHostName);
			strncpy(HostToResolve, m_stunPassServerHostName, H243_NAME_LEN);
		}
	}
}

void CStandIceUser::FillICEparams(CSipProxyIpParams* pService)
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
			m_StunServerResolved = TRUE;
		}
	}
	else
	{
		ipV6ToString(ipSTUNAddrSt.addr.v6.ip, strIp, TRUE);
		msg << std::setw(20) << "STUN password server IpV6: " << strIp << std::endl;
		strncpy(m_stunPassServerAddress.sIpAddr, strIp, IceStrLen - 1);
		m_stunPassServerAddress.sIpAddr[IceStrLen - 1] = 0;
		m_StunServerResolved = TRUE;
	}

	msg << std::setw(20) << "STUN password server port: " << ipSTUNAddrSt.port << std::endl;
	msg << std::setw(20) << "STUN password server user name: " << pService->GetStunPassUserName() << std::endl;

	m_stunPassServerAddress.port = ipSTUNAddrSt.port;
	if (0 == m_stunPassServerAddress.port)
	{
	    m_stunPassServerAddress.port = DEFAULT_STUN_PORT;
        msg << std::setw(20) << "STUN password server port (updated to default): " << m_stunPassServerAddress.port << std::endl;
	}

	memset(m_stunPassServerUserName, '\0', H243_NAME_LEN);
	strncpy(m_stunPassServerUserName, pService->GetStunPassUserName() , H243_NAME_LEN -1); // B.S. klocwork 2554
	m_stunPassServerUserName[H243_NAME_LEN - 1] = '\0';

	msg << std::setw(20) << "STUN password server password: " << pService->GetStunPassPassword() << std::endl;

	memset(m_stunPassServerPassword, '\0', H243_NAME_LEN);
	strncpy(m_stunPassServerPassword, pService->GetStunPassPassword() , H243_NAME_LEN - 1); // B.S. klocwork 2554
	m_stunPassServerPassword[H243_NAME_LEN - 1] = '\0';

//	msg << std::setw(20) << "STUN password server realim: " << pService->GetStunPassRealm() << std::endl;
//
//	memset(m_stunPassServerRealm, '\0', H243_NAME_LEN);
//	strncpy(m_stunPassServerRealm, pService->GetStunPassRealm() , H243_NAME_LEN);
//	m_stunPassServerRealm[H243_NAME_LEN - 1] = '\0';

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


    PTRACE2(eLevelInfoNormal, "CStandIceUser::Create : ", msg.str().c_str() );

	//UpdateCM();
	return;
}

/////////////////////////////////////////////////////////////////////////////
void CStandIceUser::UpdateCM()
{
	PTRACE(eLevelInfoNormal, "CStandIceUser::UpdateCM");

	char strIp[128];
	eIpType ipType = m_data->m_ipType;
	COstrStream msg;

	SetRegisterActiveAlarm(FALSE);

	/** building  ICE_INIT_REQ_S **/
	ICE_SERVER_TYPES_S	*pParams = new ICE_SERVER_TYPES_S;
	memset(pParams,0,sizeof(ICE_SERVER_TYPES_S));

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pParams->forced_MS_version = (int)(GetSystemCfgFlagHex<DWORD>("MS_ICE_VERSION"));

	if (pParams->forced_MS_version <= MS_TURN_ICE1)
	{
		if (ipType == eIpType_IpV6)
		{
			pParams->forced_MS_version = MS_TURN_ICE2_SHA256_IPv6;
			PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - setting MS_ICE_VERSION according to IP Type IPv6 ", pParams->forced_MS_version);
		}
		else
		{
			pParams->forced_MS_version = MS_TURN_ICE2;
			PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - setting MS_ICE_VERSION according to IP Type IPv4 ", pParams->forced_MS_version);
		}
	}
	else if (pParams->forced_MS_version > MS_TURN_ICE2_SHA256_IPv6)
	{
		PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - user set invalid value for MS_ICE_VERSION setting to ", MS_TURN_ICE2_SHA256_IPv6);
		pParams->forced_MS_version = MS_TURN_ICE2_SHA256_IPv6;
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "CStandIceUser::UpdateCM - user set MS_ICE_VERSION to ", pParams->forced_MS_version);
	}
	pParams->forced_MS_version = htonl(pParams->forced_MS_version);

	pParams->ice_env = eIceEnvStandard;
	pParams->req_id = GetId();
	pParams->service_id = m_serviceId;

	strncpy(pParams->authParams.user_name,m_stunPassServerUserName,IceStrLen - 1);
	strncpy(pParams->authParams.password,m_stunPassServerPassword,IceStrLen - 1);
	strncpy(pParams->authParams.realm,m_stunPassServerRealm,IceStrLen - 1);
	pParams->authParams.user_name[IceStrLen - 1] = 0;
	pParams->authParams.password[IceStrLen - 1] = 0;
	pParams->authParams.realm[IceStrLen - 1] = 0;

	strncpy(pParams->ipAddrsStr, m_ipAddrsStr, IceAddrInitStr);

	////// STUN PASS //////
	pParams->stun_pass_server_params.port = m_stunPassServerAddress.port;
	if (0 == pParams->stun_pass_server_params.port)
	{
	    pParams->stun_pass_server_params.port = DEFAULT_STUN_PORT;
	}
	strncpy(pParams->stun_pass_server_params.sIpAddr ,m_stunPassServerAddress.sIpAddr,IceStrLen - 1);
	pParams->stun_pass_server_params.sIpAddr[IceStrLen - 1] = 0;

	////// STUN UDP //////
	pParams->stun_udp_server_params.port = m_stunServerUDPAddress.port;
    if (0 == pParams->stun_udp_server_params.port)
    {
        pParams->stun_udp_server_params.port = DEFAULT_STUN_PORT;
    }
	strncpy(pParams->stun_udp_server_params.sIpAddr ,m_stunServerUDPAddress.sIpAddr,IceStrLen - 1);
	pParams->stun_udp_server_params.sIpAddr[IceStrLen - 1] = 0;

	////// STUN TCP //////
	pParams->stun_tcp_server_params.port = m_stunServerTCPAddress.port;
    if (0 == pParams->stun_tcp_server_params.port)
    {
        pParams->stun_tcp_server_params.port = DEFAULT_STUN_PORT;
    }
	strncpy(pParams->stun_tcp_server_params.sIpAddr ,m_stunServerTCPAddress.sIpAddr,IceStrLen - 1);
	pParams->stun_tcp_server_params.sIpAddr[IceStrLen - 1] = 0;

	//////Relay UDP /////
	pParams->relay_udp_server_params.port = m_RelayServerUDPAddress.port;
    if (0 == pParams->relay_udp_server_params.port)
    {
        pParams->relay_udp_server_params.port = DEFAULT_STUN_PORT;
    }
	strncpy(pParams->relay_udp_server_params.sIpAddr ,m_RelayServerUDPAddress.sIpAddr,IceStrLen - 1);
	pParams->relay_udp_server_params.sIpAddr[IceStrLen - 1] = 0;

	//////Relay TCP /////
	pParams->relay_tcp_server_params.port = m_RelayServerTCPAddress.port;
    if (0 == pParams->relay_tcp_server_params.port)
    {
        pParams->relay_tcp_server_params.port = DEFAULT_STUN_PORT;
    }
	strncpy(pParams->relay_tcp_server_params.sIpAddr ,m_RelayServerTCPAddress.sIpAddr,IceStrLen - 1);
	pParams->relay_tcp_server_params.sIpAddr[IceStrLen - 1] = 0;

	///print for debug
	msg << std::setw(20) << "print for debug - Request ID: " << pParams->req_id<< std::endl;
	msg << std::setw(20) << "print for debug - user_name : " << pParams->authParams.user_name<< std::endl;
	msg << std::setw(20) << "print for debug - password :  " << pParams->authParams.password<< std::endl;
	msg << std::setw(20) << "print for debug - STUN password server IP: " << m_stunPassServerAddress.sIpAddr << std::endl;
	msg << std::setw(20) << "print for debug - STUN password server port: " << m_stunPassServerAddress.port << std::endl;
	msg << std::setw(20) << "print for debug - service id: " << m_serviceId << std::endl;

	msg << std::setw(20) << "print for debug - stun udp port: " << pParams->stun_udp_server_params.port << std::endl;
	msg << std::setw(20) << "print for debug - stun udp IP: " << pParams->stun_udp_server_params.sIpAddr << std::endl;
	msg << std::setw(20) << "print for debug - stun tcp port: " << pParams->stun_tcp_server_params.port  << std::endl;
	msg << std::setw(20) << "print for debug - stun tcp IP: " << pParams->stun_tcp_server_params.sIpAddr << std::endl;
	msg << std::setw(20) << "print for debug - relay udp port: " << pParams->relay_udp_server_params.port << std::endl;
	msg << std::setw(20) << "print for debug - relay udp IP: " << pParams->relay_udp_server_params.sIpAddr << std::endl;
	msg << std::setw(20) << "print for debug - relay tcp port: " << pParams->relay_tcp_server_params.port  << std::endl;
	msg << std::setw(20) << "print for debug - relay tcp IP: " << pParams->relay_tcp_server_params.sIpAddr << std::endl;
	PTRACE2(eLevelInfoNormal, "CStandIceUser::UpdateCM : ", msg.str().c_str());
	PTRACE2INT(eLevelInfoNormal, "CSipProxyMsSubscriber::UpdateCardsMngr - pParams->forced_MS_version ", ntohl(pParams->forced_MS_version));


	CSegment*  pSegment = new CSegment;
	pSegment->Put( (BYTE*)pParams, sizeof(ICE_SERVER_TYPES_S) );
	delete pParams;

	CManagerApi apiCards(eProcessCards);
	apiCards.SendMsg(pSegment, SIPPROXY_TO_CARDS_ICE_INIT_REQ);
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

////////////////////////////////////////////////////////////////////////////
void CStandIceUser::OnCSEndIceInit(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CStandIceUser::OnCSEndIceInit ");
	DWORD responseStatus = STATUS_OK;
	WORD NumOfCards = 0;
	DWORD id = 0;
	BYTE IsEnableBWPolicyCheck = FALSE;  //Currently always YES.
	BYTE atLeastOneBoardSucceed=FALSE;
	// WORD server_status = eIceUnknownProblem;
	DWORD ice_init_status = STATUS_OK;

	CProcessBase *pProcess = CProcessBase::GetProcess();

	*pParam >>NumOfCards;
	PTRACE2INT(eLevelInfoNormal,"CStandIceUser::OnCSEndIceInit - NumOfCards: ",NumOfCards);

	WORD udp_status = eIceInitOk;
	WORD tcp_status = eIceInitOk;
	int  fwType 	= -1;

	ICE_INIT_IND_S* ice_init_array[NumOfCards];

	for(int i=0;i<NumOfCards;i++)
	{
		char errorText[100];
		ice_init_array[i] = new ICE_INIT_IND_S;
		pParam->Get( (BYTE*)ice_init_array[i], sizeof(ICE_INIT_IND_S) );

		PTRACE2(eLevelInfoNormal,"CStandIceUser::OnCSEndIceInit Status:  ", ((ice_init_array[i]->status) == STATUS_OK) ? "SUCCESS":"FAIL");

		memset(errorText, '\0', sizeof(errorText));

		getStatusErrorText((iceServersStatus)ice_init_array[i]->Relay_udp_status, errorText);
		PTRACE2(eLevelInfoNormal,"CStandIceUser::OnCSEndIceInit Relay_udp_status:  ", errorText);

		memset(errorText, '\0', sizeof(errorText));

		getStatusErrorText((iceServersStatus)ice_init_array[i]->Relay_tcp_status, errorText);
		PTRACE2(eLevelInfoNormal,"CStandIceUser::OnCSEndIceInit Relay_tcp_status:  ", errorText);


		if(ice_init_array[i]->status == STATUS_OK)
		{
			udp_status = ice_init_array[i]->Relay_udp_status;
			tcp_status = ice_init_array[i]->Relay_tcp_status;
			fwType = ice_init_array[i]->fw_type;
			if(udp_status==eIceInitOk || tcp_status==eIceInitOk || fwType == eFwTypeBlocked) //Blocked is for workaround - when can't connect to EDGE we don't have to use the relay server
			{
				atLeastOneBoardSucceed = TRUE;
				PTRACE(eLevelInfoNormal,"CStandIceUser::OnCSEndIceInit - atLeastOneBoardSucceed ");


				pProcess->RemoveActiveAlarmByErrorCodeUserIdFromProcess(AA_INITIALIZE_ICE_STACK_FAILURE,i);
			}
			else
			{
				memset(errorText, '\0', sizeof(errorText));

				getStatusErrorText((iceServersStatus)ice_init_array[i]->Relay_tcp_status, errorText);
				PTRACE2(eLevelError,"CStandIceUser::OnCSEndIceInit ICE initiation Failed!! - status: ",errorText);
		//		SetIceInitActiveAlarm((iceServersStatus)udp_status,i);
			}
		}
		else
		{
			PTRACE(eLevelError,"CStandIceUser::OnCSEndIceInit response Ack with status fail- status: ");
			pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
						AA_INITIALIZE_ICE_STACK_FAILURE,
						MAJOR_ERROR_LEVEL,
						"General Status failure",
						true,
						true);

			}
			PDELETE(ice_init_array[i]);
		}
	//	if(IsValidTimer(TIMER_CM_ICE_INIT))
	//		DeleteTimer(TIMER_CM_ICE_INIT);

		if(atLeastOneBoardSucceed )
		{
			ice_init_status = STATUS_OK;
			PTRACE(eLevelInfoNormal,"CStandIceUser::::OnCSEndIceInit - ICE initiation OK!! ");
		}
		else
		{
			ice_init_status = STATUS_FAIL;
			PTRACE(eLevelError,"CStandIceUser::::OnCSEndIceInit - ICE initiation Failed!!");
		}

		//DBGPASSERT(249);

		CSegment *pSeg = new CSegment;
		*pSeg << (BYTE)ice_init_status;
		*pSeg << (BYTE)IsEnableBWPolicyCheck;
		*pSeg << (DWORD)0;
		*pSeg << (DWORD)eIceEnvironment_Standard;

		CTaskApi api(eProcessConfParty, eManager);
		api.SendMsg(pSeg, SIP_PROXY_TO_CONF_END_INIT_ICE);


}

BOOL CStandIceUser::isHostNameAlreadyResolved(mcTransportAddress	*resolvedAddr)
{
	char strIP[IceStrLen];

	ipToString(*resolvedAddr, strIP, FALSE);

	if ( m_PassServerResolved && m_StunServerResolved && m_TurnServerResolved)
	{
		//check if the IP changed for hostname
		if(strncmp(m_stunPassServerAddress.sIpAddr, strIP,  H243_NAME_LEN) != 0)
		{
			m_PassServerResolved = FALSE;
		}
		if( (strncmp( m_stunServerUDPAddress.sIpAddr, strIP,  H243_NAME_LEN) != 0)	||
			(strncmp(m_stunServerTCPAddress.sIpAddr, strIP,  H243_NAME_LEN) != 0) )
		{
			m_StunServerResolved = FALSE;
		}
		if ( (strncmp(m_RelayServerUDPAddress.sIpAddr, strIP,  H243_NAME_LEN) != 0)	||
			 (strncmp(m_RelayServerTCPAddress.sIpAddr, strIP,  H243_NAME_LEN) != 0) )
		{
			m_TurnServerResolved = FALSE;
		}
	}

	if( !m_PassServerResolved || !m_StunServerResolved || !m_TurnServerResolved)
		return FALSE;

	return TRUE;
}

void CStandIceUser::OnConfIceDnsResolved(CSegment* pParam)
{
	PTRACE(eLevelError, "CStandIceUser::OnConfIceDnsResolved");
	WORD serviceId = 0;
	DWORD ipAddr = 0;
	char hostName[H243_NAME_LEN];
	BOOL hostResolved = FALSE;
	BOOL newResoleveRequestSent = FALSE;

	if(IsValidTimer(TIMER_DNS_RESOLVING))
	{
		DeleteTimer(TIMER_DNS_RESOLVING);
	}

	if (pParam == NULL)
	{
		PTRACE(eLevelError, "CStandIceUser::OnConfIceDnsResolved invalid parameter");

	}
	else
	{

		*pParam >> serviceId
				>> hostName;

		ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
		for(int	i=0; i<TOTAL_NUM_OF_IP_ADDRESSES ; i++)
		{
			memset(&pAddrList[i], 0, sizeof(ipAddressStruct));
		}
		pParam->Get((BYTE *)&pAddrList, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

		// need to choose one Address from the list
		mcTransportAddress	resolvedAddr;
		memset(&resolvedAddr, 0, sizeof(mcTransportAddress));
		GetAddressFromList(&resolvedAddr,pAddrList, m_data->m_ipType);

		if( ((resolvedAddr.ipVersion == eIpVersion4) &&  (resolvedAddr.addr.v4.ip == 0) ) ||
			((resolvedAddr.ipVersion == eIpVersion6) && (resolvedAddr.addr.v6.ip[0] == 0))	)
		{
			PTRACE2(eLevelError, "CStandIceUser::OnConfIceDnsResolved. failed to resolve hostname ", hostName);
			SetRegisterActiveAlarm(TRUE);

		} else if (isHostNameAlreadyResolved(&resolvedAddr))
		{
			//address already resolved this is a returning message from the DNSAgent - ignore!
			PTRACE2(eLevelError, "CStandIceUser::OnConfIceDnsResolved. hostname already resolved - ignore ", hostName);
		}
		else
		{

			PTRACE2INT(eLevelInfoNormal, "CStandIceUser::OnConfIceDnsResolved serviceId ", serviceId);
			PTRACE2(eLevelInfoNormal, "CStandIceUser::OnConfIceDnsResolved hostName ", hostName);
			PTRACE2(eLevelInfoNormal, "CStandIceUser::OnConfIceDnsResolved m_stunServerHostName ", m_stunServerHostName);
			PTRACE2(eLevelInfoNormal, "CStandIceUser::OnConfIceDnsResolved m_RelayServerHostName ", m_RelayServerHostName);
			PTRACE2(eLevelInfoNormal, "CStandIceUser::OnConfIceDnsResolved m_stunPassServerHostName ", m_stunPassServerHostName);

			if(serviceId == m_serviceId)
			{
				if (m_StunServerResolved == FALSE)
				{
					if	(strncmp(hostName, m_stunServerHostName, H243_NAME_LEN) == 0)
					{

						//ipToString deals with ip4/ip6 check
						ipToString(resolvedAddr, m_stunServerUDPAddress.sIpAddr, FALSE);
						ipToString(resolvedAddr, m_stunServerTCPAddress.sIpAddr, FALSE);

						PTRACE2(eLevelInfoNormal, "CStandIceUser::OnConfIceDnsResolved STUN server resolved ", m_stunServerUDPAddress.sIpAddr);
						m_StunServerResolved = TRUE;
					}
				}
				if (m_TurnServerResolved == FALSE)
				{
					if (strncmp(hostName, m_RelayServerHostName, H243_NAME_LEN) == 0)
					{
						ipToString(resolvedAddr, m_RelayServerUDPAddress.sIpAddr, FALSE);
						ipToString(resolvedAddr, m_RelayServerTCPAddress.sIpAddr, FALSE);
						PTRACE2(eLevelInfoNormal, "CStandIceUser::OnConfIceDnsResolved TURN server resolved ", m_RelayServerTCPAddress.sIpAddr);
						m_TurnServerResolved = TRUE;
					}
				}
				if (m_PassServerResolved == FALSE )
				{
					if (strncmp(hostName, m_stunPassServerHostName, H243_NAME_LEN) == 0)
					{
						ipToString(resolvedAddr, m_stunPassServerAddress.sIpAddr, FALSE);
						PTRACE2(eLevelInfoNormal, "CStandIceUser::OnConfIceDnsResolved password server resolved ", m_stunPassServerAddress.sIpAddr);
						m_PassServerResolved = TRUE;
					}
				}
			} else {
				PTRACE(eLevelError, "CStandIceUser::OnConfIceDnsResolved. Invalid serviceId = serviceId ");
				DBGFPASSERT(77);
				SetRegisterActiveAlarm(TRUE);
			}

			if ( m_PassServerResolved && m_StunServerResolved && m_TurnServerResolved)
			{
				// All resolved continue to ICE_INIT_REQ
				m_state = CONF_IDLE;
				UpdateCM();
				//return;
			} else
			{


				if ( !(m_TurnServerResolved))
				{
					if( (isIpV4Str(m_RelayServerHostName)) || (isIpV6Str(m_RelayServerHostName)) )
					{
						PTRACE2(eLevelInfoNormal, "CStandIceUser::OnConfIceDnsResolved, Found IP for TURN server ", m_RelayServerHostName);
						strncpy(m_RelayServerUDPAddress.sIpAddr, m_RelayServerHostName, H243_NAME_LEN);
						strncpy(m_RelayServerTCPAddress.sIpAddr, m_RelayServerHostName, H243_NAME_LEN);
						m_TurnServerResolved = TRUE;

					} else if(isalnum(m_RelayServerHostName[0]) && strncmp(m_RelayServerHostName, "0.0.0.0",strlen("0.0.0.0"))!=0)
					{
						m_state = CONF_ICE_DNS_RESOLVING;
						PTRACE2(eLevelError, "CStandIceUser::OnConfIceDnsResolved TURN server name need to be resolved ", m_RelayServerHostName);
						ResolveDomainReq(m_serviceId, m_RelayServerHostName);

						//set timer to receive the answer
						CSegment *pTimerSegment = new CSegment;
						*pTimerSegment << (DWORD)serviceId << m_RelayServerHostName;
						StartTimer(TIMER_DNS_RESOLVING, RegisteringTimeOut*SECOND, pTimerSegment);
						newResoleveRequestSent = TRUE;
					} else
					{
						//invalid HostName or empty setup
						DBGFPASSERT(88);
						SetRegisterActiveAlarm(TRUE);
					}
				}

				if( (!(m_StunServerResolved)) && (newResoleveRequestSent != TRUE) )
				{
					if( (isIpV4Str(m_stunServerHostName)) || (isIpV6Str(m_stunServerHostName)) )
					{
						PTRACE2(eLevelInfoNormal, "CStandIceUser::OnConfIceDnsResolved, Found IP for STUN server ", m_RelayServerHostName);
						strncpy(m_stunServerUDPAddress.sIpAddr, m_stunServerHostName, H243_NAME_LEN);
						strncpy(m_stunServerTCPAddress.sIpAddr, m_stunServerHostName, H243_NAME_LEN);
						m_StunServerResolved = TRUE;

					} else
					{
						m_state = CONF_ICE_DNS_RESOLVING;
						PTRACE2(eLevelError, "CStandIceUser::OnConfIceDnsResolved STUN server name need to be resolved ", m_stunServerHostName);
						ResolveDomainReq(m_serviceId, m_stunServerHostName);

						//set timer to receive the answer
						CSegment *pTimerSegment = new CSegment;
						*pTimerSegment << (DWORD)serviceId << m_stunServerHostName;
						StartTimer(TIMER_DNS_RESOLVING, RegisteringTimeOut*SECOND, pTimerSegment);
						newResoleveRequestSent = TRUE;
					}
				}

				if( (!(m_PassServerResolved)) && (newResoleveRequestSent != TRUE) )
				{

					if(isalnum(m_stunPassServerHostName[0]) && strncmp(m_stunPassServerHostName, "0.0.0.0",strlen("0.0.0.0"))!=0)
					{
						PTRACE2(eLevelInfoNormal, "CStandIceUser::OnConfIceDnsResolved password server name need to be resolved ", m_stunPassServerHostName);
						m_state = CONF_ICE_DNS_RESOLVING;
						ResolveDomainReq(m_serviceId, m_stunPassServerHostName);

						//set timer to receive the answer
						CSegment *pTimerSegment = new CSegment;
						*pTimerSegment << (DWORD)serviceId << m_stunPassServerHostName;
						StartTimer(TIMER_DNS_RESOLVING, RegisteringTimeOut*SECOND, pTimerSegment);
						newResoleveRequestSent = TRUE;
					} else {
						// password server is not mandatory we know that we have TURN and STUN server resolved
						// continue to ICE_INIT_REQ
						memset(m_stunPassServerAddress.sIpAddr, 0, sizeof(m_stunPassServerAddress.sIpAddr));
						m_PassServerResolved = TRUE;
						m_state = CONF_IDLE;
						UpdateCM();
					}
				}

			}
		}
	}

	//if (pTimerSegment) delete pTimerSegment;
	return;
}

void CStandIceUser::OnConfIceDnsResolveTimerExpired(CSegment* pParam)
{
	PTRACE(eLevelError, "CStandIceUser::OnConfIceDnsResolveTimerExpired");
	WORD serviceId = 0;
	DWORD ipAddr = 0;
	char hostName [H243_NAME_LEN];
	char ErrorText[200];

	if (pParam != NULL) {
		*pParam >> serviceId
				>> hostName;
	}

//	if(IsValidTimer(TIMER_DNS_RESOLVING))
//	{
//		DeleteTimer(TIMER_DNS_RESOLVING);
//	}

	snprintf(ErrorText, sizeof(ErrorText), "CStandIceUser::OnConfIceDnsResolveTimerExpired. Failed to resolve ICE Server HostName %s Service-Id %d", hostName, serviceId);

	PTRACE(eLevelError, ErrorText);


	SetRegisterActiveAlarm(TRUE);
	m_state = CONF_IDLE;
	return;
}
/////////////////////////////////////////////////////////////////////////////
void CStandIceUser::SetRegisterActiveAlarm(BYTE IsAdd)
{
	PTRACE(eLevelInfoNormal, "CStandIceUser::SetRegisterActiveAlarm");

	CProcessBase *pProcess = CProcessBase::GetProcess();

	if(IsAdd)
	{
		pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
		AA_RESOLVE_ICE_SERVER_NAME_FAILED,
		MAJOR_ERROR_LEVEL,
		"Failed to resolve ICE server Host Name, Check IPService->Sip Advanced->ICE server Host Name.",
		true,
		false);
	}
	else
	{
		pProcess->RemoveActiveAlarmFromProcess(AA_RESOLVE_ICE_SERVER_NAME_FAILED);
	}
}
