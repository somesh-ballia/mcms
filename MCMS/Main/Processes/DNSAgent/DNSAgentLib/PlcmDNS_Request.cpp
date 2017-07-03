// ==========================================
//    Copyright (C) 2013           "POLYCOM"
// ==========================================
// FileName:             plcmDNS_Request.cpp  
// 
// ==========================================


#include "PlcmDNS_Request.h"
#include "TraceStream.h"
#include "ProcessBase.h"
#include "SystemQuery.h"

#include "PlcmDNS_Processing.h" 

#include <string>

//====================================================================//
cDNS_PLCM_REQ_NODE::cDNS_PLCM_REQ_NODE()
{
	this->vClear();
}
//====================================================================//

//====================================================================//
cDNS_PLCM_REQ_NODE::~cDNS_PLCM_REQ_NODE()
{
	this->vClear();
}
//====================================================================//
//====================================================================//
void cDNS_PLCM_REQ_NODE::vTimerAwake(int par_TimerID)
{
	if(TRUE == this->m_sReqNode.bTimeOutTimerOrdered)
		this->vTimerStop(par_TimerID, "STOP_4");

	int  dwTimeOut_mSec = PLCM_DNS_TIMEOUT_DEFAULT_SEC * 1000;
	
	if(1 < this->m_sReqNode.dwDnsAddressAmount)
		dwTimeOut_mSec = ((this->m_sReqNode.dwTimeOut_mSec )/2) + 1000;	
	else
		dwTimeOut_mSec =   this->m_sReqNode.dwTimeOut_mSec      + 1000;

	dwTimeOut_mSec = min(PLCM_DNS_TIMEOUT_DEFAULT_SEC * 1000, dwTimeOut_mSec); 
	
	this->StartTimer(par_TimerID, dwTimeOut_mSec/10);
	this->m_sReqNode.bTimeOutTimerOrdered = TRUE;
}
//====================================================================//
//====================================================================//
void cDNS_PLCM_REQ_NODE::vTimerStop(int par_dwTimerId, char *pat_Log)
{
	if(TRUE == this->m_sReqNode.bTimeOutTimerOrdered)
	{
		this->DeleteTimer(par_dwTimerId);
	}
	this->m_sReqNode.bTimeOutTimerOrdered = FALSE;
}
//====================================================================//
//====================================================================//
void cDNS_PLCM_REQ_NODE::vTimerArrived(CSegment* par_pParam)
{
	//DNS_REQ_TIMER_TOUT
	this->m_sReqNode.bTimeOutTimerOrdered = FALSE;

	this->m_sReqNode.bIsResultIsTimeouted = TRUE;

	DNS_IPADDR_TTL aResoledAddrrAndTTL[20];
	memset(aResoledAddrrAndTTL, 0, sizeof(DNS_IPADDR_TTL) * 20);

	char				szHostNameComplex[PLCM_DNS_HOST_NAME_SIZE]="";
	unsigned short		wReqId     = this->m_sReqNode.wDnsReqID; 	
	unsigned short		wOutType   = this->m_sReqNode.wReqType;	

	ipAddressStruct     sDnsIPAddr = this->m_sReqNode.aDnsAddress[this->m_sReqNode.dwDnsAddress_Current];
	char				szDnsIPAddr[128]="";

	if(this->m_sReqNode.dwDomainName_Current < PLCM_DNS_DOMAINNAME_AMOUNT_MAX)
	{
		snprintf(szHostNameComplex, sizeof(szHostNameComplex)-1, "%s%s%s"
			,this->m_sReqNode.szHostName
			,('\0' == this->m_sReqNode.aDomainName[this->m_sReqNode.dwDomainName_Current].szDomainName[0])?
			"":"."
			,('\0' == this->m_sReqNode.aDomainName[this->m_sReqNode.dwDomainName_Current].szDomainName[0])?
			"":this->m_sReqNode.aDomainName[this->m_sReqNode.dwDomainName_Current].szDomainName
			); 
	}

	plcmDNS_nGetStrIpFromipAddressStruct(szDnsIPAddr, sizeof(szDnsIPAddr), &sDnsIPAddr);

	TRACEINTO << "   PLCM_DNS.[DNS_RECV]. #[" << wReqId << "]" 
		<< " | TIMEOUT  for: "
		<< " | " << plcmDNS_szGetReqTypeName(wOutType)
		<< " | HostName:[" << szHostNameComplex << "]"
		<< " | IP[" << szDnsIPAddr <<"]"
		<< " | Attempt[" << this->m_sReqNode.wAttempt_Current+1 << "/"<<this->m_sReqNode.wAttempts<<"]";


	cPLCM_DNS * pPlcmDns = G_GetPlsmDnsPtr();
	if(NULL != pPlcmDns)
		pPlcmDns->bAnalysisAndProcessing(aResoledAddrrAndTTL, 0, szHostNameComplex, wReqId, wOutType, FALSE);

}
//====================================================================//
//====================================================================//
void cDNS_PLCM_REQ_NODE::vClear()
{
	memset(&this->m_sReqNode, 0, sizeof(DNS_REQ_NODE));
}
//====================================================================//

