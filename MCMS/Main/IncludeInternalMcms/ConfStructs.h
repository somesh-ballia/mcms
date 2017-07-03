// ConfStructs.h
//
//////////////////////////////////////////////////////////////////////
#if !defined(_ConfStructs_H__)
#define _ConfStructs_H__

#include "CsCommonStructs.h"
#include "DefinesIpService.h"
#include "StatusesGeneral.h"
#include "IpAddressDefinitions.h"
#include "McuMngrStructs.h"
#include "TBStructs.h"
#include "CommonStructs.h"
#include "AllocateStructs.h"
#include "VideoStructs.h"



struct BASE_SIP_SERVER_S
{
	DWORD        status;						//(on/off/auto)
    char         hostName[ONE_LINE_BUFFER_LEN];	// server name/ip string
    DWORD        port;							// server port
	public:
	BASE_SIP_SERVER_S()
	{
		status = STATUS_OK;
		memset(hostName, '\0', ONE_LINE_BUFFER_LEN);
		port = 0;
	}	
	BASE_SIP_SERVER_S& operator=(const BASE_SIP_SERVER_S &rHnd)
	{
		if(this != &rHnd)
		{
			status		= rHnd.status;
			port		= rHnd.port;
			memcpy(hostName, rHnd.hostName, ONE_LINE_BUFFER_LEN);
		}
		return *this;
	}
};

struct SIP_SERVER_S
{	
	BASE_SIP_SERVER_S	baseSipServer;
	char				domainName[ONE_LINE_BUFFER_LEN];
	public:
	SIP_SERVER_S()
	{
		memset(domainName, '\0', ONE_LINE_BUFFER_LEN);
	}	
	SIP_SERVER_S& operator=(const SIP_SERVER_S &rHnd)
	{
		if(this != &rHnd)
		{
			baseSipServer		= rHnd.baseSipServer;
			memcpy(domainName, rHnd.domainName, ONE_LINE_BUFFER_LEN);
		}
		return *this;
	}
};

struct SIP_S
{
	BASE_SIP_SERVER_S   proxy;
    BASE_SIP_SERVER_S   altProxy;
	SIP_SERVER_S		registrar;	   // Changed since API_IP_SERVICE_CORRECTIONS 
    SIP_SERVER_S		altRegistrar; // Changed since API_IP_SERVICE_CORRECTIONS
	BYTE                transportType;                 // PROTOCOL_TRASPORT_UDP
	                                       // PROTOCOL_TRASPORT_TCP
    WORD                configureSIPServersMode;       // CONFIGURE_SIP_SERVERS_AUTOMATICALLY
	                                       // CONFIGURE_SIP_SERVERS_MANUALLY
    
    WORD                registrationOngoingConfrences; // YES / NO
    WORD                registrationMeetingRoom;       // YES / NO
	WORD                registrationEntryQueue;        // YES / NO
    WORD                acceptMeetMe;                  // YES / NO
    WORD                acceptAdHoc;                   // YES / NO
    WORD                acceptFactory;                 // YES / NO
    WORD                refreshStatus;                 // YES / NO
    BYTE                registrationMode;                // Ori sad : not necessarily
    DWORD               refreshRegistrationTout;
    WORD			    IceType;
    WORD 			    SipServerType;

	SIP_S()
	{
		transportType = eTransportTypeUdp;
		configureSIPServersMode = eConfSipServerAuto;
		
		registrationOngoingConfrences = YES;
		registrationMeetingRoom = NO;
		registrationEntryQueue = YES;       
		acceptMeetMe = NO;    
		acceptAdHoc = YES;     
		acceptFactory = NO;              
		refreshStatus = YES;
		registrationMode = NO;          
		refreshRegistrationTout = YES;
		IceType = eIceEnvironment_None;
		SipServerType = eSipServer_generic;
	}
	SIP_S& operator=(const SIP_S &rHnd)
	{
		if(this != &rHnd)
		{
			transportType 					= rHnd.transportType;
			configureSIPServersMode 		= rHnd.configureSIPServersMode;
			
			registrationOngoingConfrences 	= rHnd.registrationOngoingConfrences;
			registrationMeetingRoom 		= rHnd.registrationMeetingRoom;
			registrationEntryQueue 			= rHnd.registrationEntryQueue;       
			acceptMeetMe 					= rHnd.acceptMeetMe;    
			acceptAdHoc 					= rHnd.acceptAdHoc;     
			acceptFactory 					= rHnd.acceptFactory;              
			refreshStatus 					= rHnd.refreshStatus;
			registrationMode 				= rHnd.registrationMode;          
			refreshRegistrationTout 		= rHnd.refreshRegistrationTout;
			IceType							= rHnd.IceType;
			SipServerType					= rHnd.SipServerType;
			// copy the structures
			proxy							= rHnd.proxy;
			altProxy						= rHnd.altProxy;
			registrar						= rHnd.registrar;
			altRegistrar					= rHnd.altRegistrar;
		}
		return *this;
	}
};



struct QOS_S 
{
	BYTE		m_IsDefault;

	BYTE		m_eIpStatus;       // eQoS_disable = 0, eQoS_enable, eQoS_service, eQoS_unknown   
	BYTE        m_bIpIsDiffServ;      
	BYTE        m_bIpValueTOS;        
	BYTE        m_bIpPrecedenceAudio; 
	BYTE        m_bIpPrecedenceVideo; 

	// ATM QoS fields
	BYTE		m_eAtmStatus;         
	BYTE        m_bAtmPrecedenceAudio;
	BYTE        m_bAtmPrecedenceVideo;

	QOS_S()
	{
		m_IsDefault = TRUE;

		m_eIpStatus = 0;
		m_bIpIsDiffServ = 0;      
		m_bIpValueTOS = 0; 
		m_bIpPrecedenceAudio = 0; 
		m_bIpPrecedenceVideo = 0; 

		m_eAtmStatus = 0;         
		m_bAtmPrecedenceAudio = 0;
		m_bAtmPrecedenceVideo = 0;
	}
	QOS_S& operator=(const QOS_S &rHnd)
	{
		if(this != &rHnd)
		{
			m_IsDefault 			= rHnd.m_IsDefault;

			m_eIpStatus 			= rHnd.m_eIpStatus;
			m_bIpIsDiffServ 		= rHnd.m_bIpIsDiffServ;      
			m_bIpValueTOS 			= rHnd.m_bIpValueTOS; 
			m_bIpPrecedenceAudio 	= rHnd.m_bIpPrecedenceAudio; 
			m_bIpPrecedenceVideo 	= rHnd.m_bIpPrecedenceVideo; 
		
			m_eAtmStatus 			= rHnd.m_eAtmStatus;         
			m_bAtmPrecedenceAudio 	= rHnd.m_bAtmPrecedenceAudio;
			m_bAtmPrecedenceVideo 	= rHnd.m_bAtmPrecedenceVideo;
		}
		return *this;
	}
};
 
struct CONF_IP_PARAMS_S
{
   	DWORD   	service_id;
	WORD    	service_protocol_type;//hold the protocol the service supports (like eIPProtocolType_SIP).
	char    	service_name[NET_SERVICE_PROVIDER_NAME_LEN];
	char    	default_service_name[NET_SERVICE_PROVIDER_NAME_LEN];
	char    	domain_name[DOMAIN_NAME_LEN]; // comes from DNS
	ALIAS_S 	aliases		[MAX_ALIAS_NAMES_NUM];
	eIpType		service_ip_protocol_types;
	ipAddressIf cs_ipV4;
	ipAddressIf cs_ipV6Array[NUM_OF_IPV6_ADDRESSES];
	BOOL    	is_gk_external;
	QOS_S   	qos;
	SIP_S   	sip;
	ALIAS_S     dialIn;	
	ipAddressStruct gk_ip;
    char  gkName[H243_NAME_LEN];
    BOOL        isAvfOn;
    BYTE 		service_default_type;
    
	DWORD  future_use1;
	DWORD  future_use2;

public:
	CONF_IP_PARAMS_S()
	{
		service_id		= 42;
		memset(service_name, '\0', NET_SERVICE_PROVIDER_NAME_LEN);
		memset(default_service_name, '\0', NET_SERVICE_PROVIDER_NAME_LEN);
		memset(domain_name, '\0', DOMAIN_NAME_LEN);		
		
		service_ip_protocol_types = eIpType_None;
		cs_ipV4.v4.ip	= 42;
		for (int i = 0; i < NUM_OF_IPV6_ADDRESSES; i++)
		{
			memset(cs_ipV6Array[i].v6.ip, 0, IPV6_ADDRESS_BYTES_LEN);
			cs_ipV6Array[i].v6.scopeId = 0;
		}

		is_gk_external	= FALSE;
		memset(aliases, 0, sizeof(ALIAS_S) * MAX_ALIAS_NAMES_NUM);
        gk_ip.ipVersion = eIpVersion4;
        gk_ip.addr.v4.ip = 42;
		isAvfOn = FALSE;            

		future_use1		= 42;
		future_use2		= 42;

		service_default_type = 0;
	}
	CONF_IP_PARAMS_S& operator=(const CONF_IP_PARAMS_S &rHnd)
	{
		if(this != &rHnd)
		{
			service_id = rHnd.service_id;
			service_protocol_type = rHnd.service_protocol_type;
			service_default_type = rHnd.service_default_type;
			strcpy(service_name, rHnd.service_name);
			strcpy(domain_name, rHnd.domain_name);
			
			for(int i = 0 ; i < MAX_ALIAS_NAMES_NUM ; i++)
			{
				memcpy(&(aliases[i]), &(rHnd.aliases[i]), sizeof(ALIAS_S));
			}

			service_ip_protocol_types			= rHnd.service_ip_protocol_types;
			cs_ipV4			= rHnd.cs_ipV4;
			for (int i = 0; i < NUM_OF_IPV6_ADDRESSES; i++)
			{
				memcpy( &(cs_ipV6Array[i].v6.ip), &(rHnd.cs_ipV6Array[i].v6.ip), IPV6_ADDRESS_BYTES_LEN );
				cs_ipV6Array[i].v6.scopeId = rHnd.cs_ipV6Array[i].v6.scopeId;
			}
			
			is_gk_external 	= rHnd.is_gk_external;
			sip 			= rHnd.sip;
			gk_ip 			= rHnd.gk_ip;
			isAvfOn         = rHnd.isAvfOn;
            
			qos				= rHnd.qos;
	   		sip				= rHnd.sip;
	
			memcpy(&dialIn, &(rHnd.dialIn), sizeof(ALIAS_S));
		}
		return *this;
	}
	
private:
	// disabled
	CONF_IP_PARAMS_S(const CONF_IP_PARAMS_S&);
} ;


enum eRsrcState
{
	eRsrcNonActive 	= 0,
	eRsrcActive 	= 1
};



typedef struct
{
	eRsrcState openLogicalResources[NUM_OF_LOGICAL_RESOURCE_TYPES]; // The position in this array will be as enum eLogicalResourceTypes:
	MOVE_RESOURCES_PARAMS_S moveRsrcParams;
} MOVE_RESOURCES_REQ_S;


#endif // !defined(_ConfStructs_H__)
