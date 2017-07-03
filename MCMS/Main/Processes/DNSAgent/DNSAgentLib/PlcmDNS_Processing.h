// 
// ==========================================
//    Copyright (C) 2014           "POLYCOM"
// ==========================================
// FileName:               PlcmDNS_Processing.h  
// Include line recommended:
// #include "PlcmDNS_Processing.h"  //
// 
// ==========================================

#ifndef PLCMDNS_PROCESSING_H_
#define PLCMDNS_PROCESSING_H_

//#include "StateMachine.h"
#include "Segment.h"
#include "DNSAgentManagerStructs.h"

#include "PlcmDNS_Packet.h"    //
#include "PlcmDNS_Network.h"   //
#include "PlcmDNS_Packet.h"    //
#include "PlcmDNS_Tools.h"     //
#include "PlcmDNS_Request.h"   //
#include "PlcmDNS_TTLService.h"//
#include "DnsRecordsMngr.h"    // 
//-S- ----- BRIDGE-16374 ---------------------------------//
typedef struct _PLCM_DNS_BIND_PORTS
{
	unsigned short		wPort_v4;
	unsigned short		wPort_v6;
}
PLCM_DNS_BIND_PORTS;
//-E- ----- BRIDGE-16374 ---------------------------------//
typedef struct _PLCM_DNS_SERVICE_CONFIG
{
	BOOL				bIsRequest_A									;
	BOOL				bIsRequest_AAAA									;

	unsigned short		wAttempts										;
	unsigned int		dwTimeOut_InSec									;      

	unsigned int		dwDnsAddressAmount;
	ipAddressStruct		aDnsAddress[PLCM_DNS_MAX_ADDITIONAL_IP]         ;	

	unsigned int		dwDomainNameAmount								;
	DNS_PLCM_DOMAINNAME	aDomainName[PLCM_DNS_DOMAINNAME_AMOUNT_MAX]		;

	char                szSignalingIPv4[PLCM_STRING_SIZE_256]			;
	char                szSignalingIPv6[PLCM_STRING_SIZE_256]			;

	PLCM_DNS_BIND_PORTS sBindPorts                                      ;//--- BRIDGE-16374
}
PLCM_DNS_SERVICE_CONFIG;

typedef enum _ePLCM_DNS_STATE
{
	ePLCM_DNS_STATE_IDLE			= 0
	, ePLCM_DNS_STATE_ACTIVATED		= 1
	, ePLCM_DNS_STATE_ACTIVE_FAILED = 2
}
ePLCM_DNS_STATE;    


//====================================================================//
class cDNS_SERVICE_CONFIG : public CPObject
{
	CLASS_TYPE_1(cDNS_SERVICE_CONFIG, CPObject)
public: 
	virtual const char*  NameOf() const {return "cDNS_SERVICE_CONFIG";}

	cDNS_SERVICE_CONFIG			();
	virtual ~cDNS_SERVICE_CONFIG();

	void	vClean				(WORD par_ServId);
	void    vGetConf			(WORD par_ServId);

	void    vSetConf            (WORD par_ServId, DNS_IP_SERVICE_S * par_pServConfParam, WORD  par_wIpType);//eIpType

	BOOL    bGetResolvConfData  (PLCM_DNS_SERVICE_CONFIG * par_pConf, WORD par_ServId);

	void    vDumpConfig         (char * par_szPrefix);
public:
	unsigned short			m_wSerID;	
	PLCM_DNS_SERVICE_CONFIG m_sConf   ;
};
//====================================================================//

//====================================================================//
class cPLCM_cREQ_ID 
{
public:
	cPLCM_cREQ_ID ();
	~cPLCM_cREQ_ID();

	unsigned short wGetReqId();
public:
	unsigned short	m_wDnsReqIG		;
	PLc_MUTEX		m_DnsReqIGMutex ;
};
//====================================================================//
//====================================================================//
class cPLCM_DnsUdpRecvThr  : public PLc_THREAD 
{
	CLASS_TYPE_1(cPLCM_DnsUdpRecvThr, PLc_THREAD)
public:
	virtual const char*          NameOf() const {return "cPLCM_DnsUdpRecvThr";}
	cPLCM_DnsUdpRecvThr         (/*cDnsReqList * par_pDnsReqList,*/ void * par_pVoidcPLCM_DNS);
	~cPLCM_DnsUdpRecvThr        ();
	virtual void Svc			();

	void                        vReadProcessing(EVENT_INFO * par_pEvenInfo);

	unsigned int				dwGetFullResolvedArray(DNS_IPADDR_TTL    * par_aFullResolved
													, unsigned int		 par_ArrLen
													, cDNS_PLCM_REQ_NODE * par_pNode
													, cDNS_PLCM_REQ_NODE * par_pNodePartner);
public:
	int							m_nThrState			;
	//cDnsReqList			*		m_pDnsReqList		;

	void				*       m_pVoidcPLCM_DNS    ;

};
//====================================================================//



class cDNS_SOCKET_ARRAY: public CPObject
{
	CLASS_TYPE_1(cDNS_SOCKET_ARRAY, CPObject)
public:
	virtual const char*  NameOf() const {return "cDNS_SOCKET_ARRAY";}
	cDNS_SOCKET_ARRAY ();
	~cDNS_SOCKET_ARRAY();

	void	vArrSocketInit    (enIpVersion par_IPv, void * par_pPlcmDns);
	void	vArrSocketClose   ();

	void	vArrOpenAndBinnd  ();
	void	vArrOpenAndBinndByService  (WORD par_wServID);
	int		nMultiSelect	  (SOCKET * m_arSocketsSelected,EVENT_INFO * m_arEventInfo);

	void    vSocketArrayDump  (char * par_zsPrefix, int par_nCSFrom, int par_nCSUpto);

public:
	BOOL            m_IsSocketArrActived                 ; 
	void   	   *	m_pvPLCM_DNS                         ;
	enIpVersion		m_IpV								 ;

	cDNS_SOCKET		m_SockObjArray[DNS_MAX_SERVICES]     ;

	SOCKET          m_arSocketsSelected[DNS_MAX_SERVICES];
	EVENT_INFO		m_arEventInfo      [DNS_MAX_SERVICES];
};



//====================================================================//
class cPLCM_DNS :  public CPObject
{
	CLASS_TYPE_1(cPLCM_DNS, CPObject)
public: 
	virtual const char*  NameOf() const {return "cPLCM_DNS";}
	cPLCM_DNS				();
	virtual ~cPLCM_DNS		();

	//virtual void   HandleEvent(CSegment* pMsg);
	//virtual void   HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	BOOL  bActivate ();
	BOOL  bActivate (COsQueue*  m_pRcvMbxDns);

	BOOL  bResolveHostName		(char * par_pHostName, WORD par_wServiceID, eProcessType par_eProcessType, DWORD par_dwConnId, DWORD par_dwPartyId, BOOL par_bIsForeResolving, COsQueue* par_pPartyQueue);
	BOOL  bGetFromShMem			(char * par_pHostName, WORD par_wServiceID, ipAddressStruct * par_aOut, int * par_pInOutLen, BOOL par_bIsUsing);
	BOOL  bSendRequestHostName	(char * par_pHostName, WORD par_wServiceID);
	BOOL  bSendRequestHostName  (char * par_pHostName, WORD par_wServiceID, eProcessType par_eProcessType, DWORD par_dwConnId, DWORD par_dwPartyId);

	BOOL  bSendRequestSrv       (char * par_pHostName, WORD par_wServiceID, eProcessType par_eProcessType, eIPProtocolType par_wSignalProtocolType, enTransportType par_wTransportProtocolType);
	
	BOOL  bPrepareRequestNode   (char * par_pHostName, WORD par_wServiceID, eProcessType par_eProcessType, DWORD par_dwConnId, DWORD par_dwPartyId, cDNS_PLCM_REQ_NODE *par_pOut, DNS_REQ_RES_TYPE par_eReqType);	
	BOOL  bSendRequestNode      (cDNS_PLCM_REQ_NODE * par_pReqNode);

	BOOL  bGetDnsAddrByServiceId		(WORD par_wServiceID, ipAddressStruct * par_pAddrOut, unsigned int * par_pInOutLen);
	void  bGetDomainByServiceId			(WORD par_wServiceID, DNS_PLCM_DOMAINNAME * par_pOut, unsigned int * par_pInOutLen);
	void  bGetRequiredAorAAAAByServiceId(WORD par_wServiceID, BOOL * par_pIsA, BOOL * par_pIsAAAA);
	WORD  bGetDnsAttemptsByServiceId    (WORD par_wServiceID);

	BOOL  bBuildNewReqFromCurr	(cDNS_PLCM_REQ_NODE * par_CurrNode, BOOL par_bIsResponceArrived);

	void  vDebugPrintAddrList	(ipAddressStruct * par_pAddr, int  par_nInLen, char * par_szPrefix);

	BOOL  bAnalysisAndProcessing (DNS_IPADDR_TTL *	par_aIpaddTTL, int par_ArrLen
		                        , char			 *	par_szHostNameComplex, unsigned short par_wReqId
		                        , unsigned short	par_ReqType // 
								, BOOL				par_bIsResponceArrived);

	unsigned int dwGetFullResolvedArray(  DNS_IPADDR_TTL    *  par_aFullResolved
										, unsigned int		   par_ArrLen
										, cDNS_PLCM_REQ_NODE * par_pNode
										, cDNS_PLCM_REQ_NODE * par_pNodePartner);


	unsigned int dwGetUsingTimeStamp (char * par_pHostName, WORD par_wServiceID);
    BOOL         bUpdateDataIntoShMem(cDNS_PLCM_REQ_NODE * par_pNodeReq, DNS_IPADDR_TTL * par_aFullAddrTTLResolved, unsigned int par_dwFullResolvedAmount); 
	BOOL		 bSendReq  ();
	void         vDumpConf(int par_nFrom,int par_nUp, std::ostream& os);

	BOOL         bReConfigSourceIpForBind_IPv4(WORD par_wCsId, char * par_szSoureceIp);
	BOOL         bReConfigSourceIpForBind_IPv6(WORD par_wCsId, char * par_szSoureceIp);

	BOOL         bReConfigSourceIpForBind(WORD serviceID, enIpVersion par_eIpV, const char * par_szSoureceIp);

public:
	ePLCM_DNS_STATE			m_eObjState		;

	cDnsReqList				m_ReqList		;

	cDNS_SOCKET_ARRAY       m_DnsArrSocket4 ; 
	cDNS_SOCKET_ARRAY       m_DnsArrSocket6 ; 

	//cDNS_SOCKET				m_DnsSocket4	;
	//cDNS_SOCKET				m_DnsSosket6	;

	COsQueue*               m_pRcvMbxDns    ;

	cPLCM_DnsUdpRecvThr  *  m_pDnsRecvThr	;

	WORD					m_state			;	

	cDNS_SERVICE_CONFIG     m_aConf[DNS_MAX_SERVICES];
//---Outbound Proxy ---//	
	char                    n_Buff1[2048];

	DnsRecordsMngr		*   m_pShManager	;
	char                    n_Buff2[2048];

	cTTLService			* 	m_pTTLService	;
	char                    n_Buff3[2048];
//---Outbound Proxy ---//
};
//====================================================================//
//====================================================================//
typedef struct _sDNS_RECV
{
	DNS_IPADDR_TTL		m_aResoledAddrrAndTTL[PLCM_MAX_RESOLVED_IP]	;
	int					m_nResolved									;
	char				m_szHostNameComplex[PLCM_DNS_HOST_NAME_SIZE];
	WORD				m_ReqId										;
	WORD				m_wReqType									;
	BOOL				m_bIsResponseArrived						;	
}
sDNS_RECV;

class cMsgDnsRECV: public CPObject
{
	CLASS_TYPE_1(cMsgDnsRECV, CPObject)
public:
	//Constructor
	cMsgDnsRECV() ;
	cMsgDnsRECV(DNS_IPADDR_TTL * par_aResTTLAndAddr
		      , int              par_nArrLen
			  , char           * par_szHostName
			  , WORD             par_wReqId
			  , WORD             par_wReqType
			  , BOOL             par_bIsRecv) ;
	//Destructor
	virtual ~cMsgDnsRECV();
	virtual const char* NameOf() const {return "cMsgDnsRECV";}

	void Serialize  (WORD format, CSegment &seg);
	void DeSerialize(WORD format, CSegment &seg);
	void DumpR		(char * headerTxt);

	void vClean     ();

public:
	sDNS_RECV		m_sMsgData;
};
//====================================================================//



cPLCM_DNS * G_GetPlsmDnsPtr();

#endif //PLCMDNS_PROCESSING_H_


