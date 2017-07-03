// ==========================================
//    Copyright (C) 2014           "POLYCOM"
// ==========================================
// FileName:           PlcmDNS_TTLService.cpp  
// 
// ==========================================



#include "PlcmDNS_TTLService.h" 
#include "PlcmDNS_Processing.h"  //

#include "TraceStream.h"

#define TTL_SERVICE_TIMER		201

PBEGIN_MESSAGE_MAP(cTTLService)
  ONEVENT(TTL_SERVICE_TIMER				,ANYCASE 	,cTTLService::vTimerArrived)
PEND_MESSAGE_MAP(cTTLService, CStateMachine);


//==============================================================================//
cTTL_NODE::cTTL_NODE(char          * par_szHostName
				  , unsigned short	 par_wSerID
				  , unsigned int     par_dwTTL
				  , unsigned short   par_wReqType)
{
	this->vClear();
	if(NULL != par_szHostName)
		strncpy(this->m_sTTLNode.szHostName, par_szHostName, sizeof(this->m_sTTLNode.szHostName)-1);
	this->m_sTTLNode.wServiceID   = par_wSerID;
	this->m_sTTLNode.dwTTL        = par_dwTTL ;
	this->m_sTTLNode.dwTimeStamp  = Pm_getCurrentTimestamp(E_TIMER_RESOLUTION_SEC);
	this->m_sTTLNode.dwUpdateTime = this->m_sTTLNode.dwTimeStamp + par_dwTTL;
	this->m_sTTLNode.eDnsReqResType = par_wReqType;
}
//==============================================================================//
//==============================================================================//
cTTL_NODE::~cTTL_NODE()
{
	vClear();
}
//==============================================================================//
//==============================================================================//
void cTTL_NODE::vClear()
{
	memset(&this->m_sTTLNode, 0, sizeof(TTL_NODE));
}
//==============================================================================//
//==============================================================================//
string * cTTL_NODE::sDump(string * par_pString)
{
    string * pRc = NULL;
	if(NULL != par_pString)
	{
		char    szTxt[512]="";
		char    szOrderedTimeStamp[32]="";
		char    szUpdateTimeStamp[32]="";

		Pm_GetTimeStringT(this->m_sTTLNode.dwTimeStamp, E_TIMER_RESOLUTION_SEC, E_TIMER_FORMAT_MONTH, sizeof(szOrderedTimeStamp)-1, szOrderedTimeStamp); 
		Pm_GetTimeStringT(this->m_sTTLNode.dwUpdateTime, E_TIMER_RESOLUTION_SEC, E_TIMER_FORMAT_MONTH, sizeof(szUpdateTimeStamp)-1, szUpdateTimeStamp); 

		snprintf(szTxt, sizeof(szTxt)-1,"| %s | %5d | %s | %2d | %s |%s\n" 
				 , szOrderedTimeStamp
				 , this->m_sTTLNode.dwTTL	
				 , szUpdateTimeStamp
				 , this->m_sTTLNode.wServiceID
				 , plcmDNS_szGetReqTypeName(this->m_sTTLNode.eDnsReqResType)
				 , this->m_sTTLNode.szHostName);
		par_pString->append((const char*)szTxt);
	}

	return pRc;
}
//==============================================================================//


//------------------------------------------------------------------------------//
//--------------cTTLList -------------------------------------------------------//
//------------------------------------------------------------------------------//
//==============================================================================//
cTTLList::cTTLList()
{
	MutexObjInit(&this->m_Mutex);
}
//==============================================================================//
//==============================================================================//
cTTLList::~cTTLList()
{
	this->bClean();
	M_DEINIT(this->m_Mutex);
}
//==============================================================================//
//==============================================================================//
BOOL cTTLList::bInsert(cTTL_NODE * par_pNode)
{
	BOOL  bRc = FALSE;

	if(NULL != par_pNode)
	{
		M_LOCK(this->m_Mutex);
		 this->m_TTLList.push_back(par_pNode);
		M_UnLOCK(this->m_Mutex);  
		bRc = TRUE;
	}
	return bRc;
}
//==============================================================================//
//==============================================================================//
BOOL cTTLList::bDelete(cTTL_NODE * par_pNode)
{
	BOOL  bRc = FALSE;
	if(NULL != par_pNode)
	{
		unsigned int	nInd		= 0;

		M_LOCK(this->m_Mutex);
		for (nInd = 0; nInd <  m_TTLList.size(); nInd++) 
		{
			cTTL_NODE * pNode = m_TTLList[nInd];
			if(  (pNode->m_sTTLNode.wServiceID == par_pNode->m_sTTLNode.wServiceID)
			   &&(0 == strncmp(pNode->m_sTTLNode.szHostName, par_pNode->m_sTTLNode.szHostName, sizeof(pNode->m_sTTLNode.szHostName)))
			  )
			{
				m_TTLList.erase(m_TTLList.begin() + nInd);
				delete(pNode);
				break;
			}
		}
		M_UnLOCK(this->m_Mutex);
	}
	return bRc;
}
//==============================================================================//
//==============================================================================//
BOOL cTTLList::bDelete(char * par_szHostName, unsigned short par_wServiceID)
{
	BOOL  bRc = FALSE;
	unsigned int	nInd		= 0;

	M_LOCK(this->m_Mutex);
	for (nInd = 0; nInd < m_TTLList.size(); nInd++) 
	{
		cTTL_NODE * pNode = m_TTLList[nInd];
		if(  (pNode->m_sTTLNode.wServiceID == par_wServiceID)
		   &&(0 == strncmp(pNode->m_sTTLNode.szHostName, par_szHostName, sizeof(pNode->m_sTTLNode.szHostName)))
		  )
		{
			m_TTLList.erase(m_TTLList.begin() + nInd);
			//TRACEINTO << "PLCM_DNS. TTLService. DELETED. ServiceId:[" << par_wServiceID <<"] | HostName:["<< par_szHostName << "]";
			bRc = TRUE;
			delete(pNode);
			break;
		}

		if(FALSE == bRc)
			TRACEINTO << "PLCM_DNS. TTLService. Deleting. NOT FOUND!! ServiceId:[" << par_wServiceID <<"] | HostName:["<< par_szHostName << "]";
	}
	M_UnLOCK(this->m_Mutex);  
	return bRc;
}
//==============================================================================//
//==============================================================================//
cTTL_NODE * cTTLList::Find(char * par_szHostName, unsigned short par_wServiceID)
{
	cTTL_NODE  * pRc = NULL;
	unsigned int		nInd = 0;

	M_LOCK(this->m_Mutex);
	for (nInd = 0; nInd < m_TTLList.size(); nInd++) 
	{
		cTTL_NODE * pNode = m_TTLList[nInd];
		if(  (pNode->m_sTTLNode.wServiceID == par_wServiceID)
			&&(0 == strncmp(pNode->m_sTTLNode.szHostName, par_szHostName, sizeof(pNode->m_sTTLNode.szHostName)))
		  )
		{
			pRc = pNode;
			break;
		}
	}
	M_UnLOCK(this->m_Mutex);
	return pRc;
}
//==============================================================================//
//==============================================================================//
BOOL  cTTLList::bClean()
{
	BOOL			bRc = TRUE;
	unsigned int	nInd		= 0;
	M_LOCK(this->m_Mutex);
	while(0 < m_TTLList.size())
	{
		cTTL_NODE * pNode = m_TTLList[0];
		unsigned int dwCurrSize = m_TTLList.size();
		m_TTLList.erase(m_TTLList.begin() + 0);
		delete(pNode);
	}
	M_UnLOCK(this->m_Mutex);
	return bRc;
}
//==============================================================================//
//==============================================================================//
BOOL cTTLList::DnsReqListAudit(DnsTTLListAuditFun par_pAuditFunction, int par_nDataType, void * par_pData)
{
	BOOL			bRc = FALSE;
	unsigned int	nInd		= 0;
	unsigned int	nListSize	= m_TTLList.size();

	if(NULL != par_pAuditFunction)
	{
		M_LOCK(this->m_Mutex);
		for (nInd = 0; nInd < nListSize; nInd++) 
		{
			cTTL_NODE * pNode = m_TTLList[nInd];
			bRc = par_pAuditFunction(pNode, par_nDataType, par_pData);
		}
		M_UnLOCK(this->m_Mutex);
	}

	return bRc;
}
//==============================================================================//
//==============================================================================//
BOOL cTTLList::DnsReqListAuditBreak(DnsTTLListAuditFun par_pAuditFunction, int par_nDataType, void * par_pData)// First return from "par_pAuditFunction" - is break from loop
{
	BOOL			bRc = FALSE;
	unsigned int	nInd		= 0;
	unsigned int	nListSize	= m_TTLList.size();

	if(NULL != par_pAuditFunction)
	{
		M_LOCK(this->m_Mutex);
		for (nInd = 0; nInd < nListSize; nInd++) 
		{
			cTTL_NODE * pNode = m_TTLList[nInd];
			if(TRUE == par_pAuditFunction(pNode, par_nDataType, par_pData))
			{
				break;
				bRc = TRUE;
			}
		}
		M_UnLOCK(this->m_Mutex);
	}

	return bRc;
}
//==============================================================================//
//==============================================================================//
unsigned int cTTLList::dwGetSize()
{
	M_LOCK(this->m_Mutex);
	 return m_TTLList.size();
	M_UnLOCK(this->m_Mutex);
}
//==============================================================================//

//==============================================================================//
void cTTLList::vNearestNode_Dump(unsigned int dwCurrntTime, unsigned int dwNearestTime, cTTL_NODE * pNode, char * par_szPrefix)
{
	if(NULL != pNode)
	{
		char szCurrentTime  [32]="";
		char szNodeTimeStamp[32]="";
		char szUpdateTime   [32]="";

        Pm_GetTimeStringT(dwCurrntTime, E_TIMER_RESOLUTION_SEC, E_TIMER_FORMAT_MONTH, sizeof(szCurrentTime)-1, szCurrentTime); 
		Pm_GetTimeStringT(pNode->m_sTTLNode.dwTimeStamp, E_TIMER_RESOLUTION_SEC, E_TIMER_FORMAT_MONTH, sizeof(szNodeTimeStamp)-1, szNodeTimeStamp); 
		Pm_GetTimeStringT(pNode->m_sTTLNode.dwUpdateTime + PLCM_DNS_SERVICE_REQ_SHIFT_TIME_Sec, E_TIMER_RESOLUTION_SEC, E_TIMER_FORMAT_MONTH, sizeof(szUpdateTime)-1, szUpdateTime); 

			TRACEINTO << "PLCM_DNS. TTLService. cTTLList. pGetNearestNode["<< ((NULL != par_szPrefix)? par_szPrefix:"") << "]" 
		    << " | Host: "<< pNode->m_sTTLNode.szHostName
			<< " | Serv.Id: "<<pNode->m_sTTLNode.wServiceID
			<< " | C.Time: "<< szCurrentTime 
			<< " | S.Time: "<< szNodeTimeStamp
			<< " | TTL: "   << pNode->m_sTTLNode.dwTTL
			<< " | U.Time: "<< szUpdateTime
			<< " | D.Time: " <<  dwNearestTime + PLCM_DNS_SERVICE_REQ_SHIFT_TIME_Sec 
			<< " | ReqType: "<< plcmDNS_szGetReqTypeName(pNode->m_sTTLNode.eDnsReqResType);
	}
}
//==============================================================================//
//==============================================================================//
cTTL_NODE * cTTLList::pGetNearestNode()
{
	cTTL_NODE * pRc = NULL;

	M_LOCK(this->m_Mutex);
		unsigned int dwCurrntTime = Pm_getCurrentTimestamp(E_TIMER_RESOLUTION_SEC);

		if(0 <  m_TTLList.size())
		{
			unsigned int dwNearestTime = 2;

			pRc = m_TTLList[0];

			if(dwCurrntTime < pRc->m_sTTLNode.dwUpdateTime)
				dwNearestTime = (pRc->m_sTTLNode.dwUpdateTime - dwCurrntTime);
			else
				dwNearestTime = 0;
		
        
			for(unsigned int nInd = 1; nInd <  m_TTLList.size(); nInd++) 
			{
				cTTL_NODE * pNode = m_TTLList[nInd];
				if(dwCurrntTime < pNode->m_sTTLNode.dwUpdateTime)
				{
					if(dwNearestTime > (pNode->m_sTTLNode.dwUpdateTime - dwCurrntTime))
					{
						dwNearestTime = (pNode->m_sTTLNode.dwUpdateTime - dwCurrntTime);
						pRc = pNode;
					}
				}	
				else//dwCurrntTime >= dwUpgradeTime
				{ 
					dwNearestTime = 0;
					pRc = pNode;
					break;
				}	
			}

			vNearestNode_Dump(dwCurrntTime, dwNearestTime, pRc, "-1-");
		}
	M_UnLOCK(this->m_Mutex);

	return pRc;
}
//==============================================================================//
//==============================================================================//
void  cTTLList::vListDump(string * par_pString)
{
	if(NULL != par_pString)
	{
		unsigned int	nInd		= 0;

		M_LOCK(this->m_Mutex);
		for (nInd = 0; nInd < m_TTLList.size(); nInd++) 
		{
			cTTL_NODE * pNode = m_TTLList[nInd];
			pNode->sDump(par_pString);
		}
		M_UnLOCK(this->m_Mutex);
	}
}
//==============================================================================//



//------------------------------------------------------------------------------//
//--------------cTTLService -------------------------------------------------------//
//------------------------------------------------------------------------------//
//==============================================================================//
cTTLService::cTTLService()
{
	this->m_cTTLList.bClean();
	this->m_bIsTimerOrdered = FALSE;
	this->m_pNearestNode    = NULL ;

	this->m_state = ANYCASE;

	TRACEINTO << "PLCM_DNS. TTLService. Constructed";
}
//==============================================================================//
//==============================================================================//
cTTLService::~cTTLService()
{
	this->m_cTTLList.bClean();
	this->vTimerStop();
}
//==============================================================================//
//==============================================================================//
void cTTLService::vClean()
{
	this->m_cTTLList.bClean();
	this->vTimerStop();

	this->m_bIsTimerOrdered = FALSE;
	this->m_pNearestNode    = NULL;
}
//==============================================================================//
//==============================================================================//
BOOL cTTLService::bInsert(char * par_szHostName, unsigned short par_wServiceID, unsigned int par_dwTTL, unsigned short par_wReqType)
{
	BOOL bRc = FALSE;

	if(NULL != par_szHostName)
	{
		cTTL_NODE * pNode = new cTTL_NODE(par_szHostName, par_wServiceID, par_dwTTL, par_wReqType);

		if(NULL != pNode)
		{
			bRc = this->m_cTTLList.bInsert(pNode);

			TRACEINTO << "PLCM_DNS. TTLService. bInsert. [" << ((bRc==FALSE)? "FALSE":"TRUE")<< "]"
				        << " | HostName:" << par_szHostName << " | Serv.Id:" << par_wServiceID << " | TTL:" << par_dwTTL 
						<< " | ReqType:" <<  plcmDNS_szGetReqTypeName(pNode->m_sTTLNode.eDnsReqResType)
						<< "  (" << pNode->m_sTTLNode.eDnsReqResType << ")"; 

			this->vNearestUpdate();
		}
		else
			TRACEINTO << "PLCM_DNS. TTLService. bInsert. allocating NEW of cTTL_NODE: FAILED";
	}
	else
		TRACEINTO << "PLCM_DNS. TTLService. bInsert. HostName is NULL!!!";

	return bRc;
}
//==============================================================================//
//==============================================================================//
BOOL cTTLService::bDelete(char * par_szHostName, unsigned short par_wServiceID)
{
	BOOL bRc = FALSE;

	if(NULL != par_szHostName)
	{
		bRc = this->m_cTTLList.bDelete(par_szHostName, par_wServiceID);
	}
	else
		TRACEINTO << "PLCM_DNS. TTLService. bDelete. HostName is NULL!!!";

	this->m_pNearestNode = NULL;

	return bRc;
}
//==============================================================================//
//==============================================================================//
cTTL_NODE * cTTLService::Find(char * par_szHostName, unsigned short par_wServiceID)
{
	cTTL_NODE * pRc = NULL;

	if(NULL != par_szHostName)
	{
		pRc = this->m_cTTLList.Find(par_szHostName, par_wServiceID);
	}
	else
		TRACEINTO << "PLCM_DNS. TTLService. Find. HostName is NULL!!!";

	return pRc;
}
//==============================================================================//
//==============================================================================//
void cTTLService::vTimerArrived	 (CSegment* par_pParam)
{
	TRACEINTO << "PLCM_DNS. TTLService. TIMER [0] <TTL_SERVICE_TIMER> has arrived";

	cTTL_NODE		*	pNearestNode = this->m_pNearestNode;


	if(NULL != this->m_pNearestNode)
	{
		cPLCM_DNS * pPlcmDns = G_GetPlsmDnsPtr();
		char		szHostName[PLCM_DNS_HOST_NAME_SIZE]="";
		WORD		wServId  = pNearestNode->m_sTTLNode.wServiceID;
		WORD        wReqType = pNearestNode->m_sTTLNode.eDnsReqResType; 
		strncpy(szHostName, pNearestNode->m_sTTLNode.szHostName, sizeof(szHostName)-1);

		//-------------- DELETE ----------------------//
		this->bDelete(pNearestNode->m_sTTLNode.szHostName, pNearestNode->m_sTTLNode.wServiceID);
		//-------------------------------------------//
	
		if(  (eDNS_TYPE_A_IPv4    == wReqType)
           ||(eDNS_TYPE_AAAA_IPv6 == wReqType)  
		  )
			pPlcmDns->bSendRequestHostName(szHostName, wServId, eProcessTypeInvalid,0, 0);
		else
        if(eDNS_TYPE_SRV == wReqType)
		{
			eIPProtocolType  eSigPrort   = eIPProtocolType_None;
			enTransportType  eTransPrort = eUnknownTransportType; 
			int              nOffset     = 0;

			if(0 == strncasecmp(&szHostName[0], "_sip." , strlen("_sip.") ))  {eSigPrort = eIPProtocolType_SIP ; nOffset=strlen("_sip.") ;}
			if(0 == strncasecmp(&szHostName[0], "_h323.", strlen("_h323.") )) {eSigPrort = eIPProtocolType_H323; nOffset=strlen("_h323.");}

			if(0 < nOffset)
			{
				if(0 == strncasecmp(&szHostName[nOffset], "_udp", strlen("_udp") )) eTransPrort = eTransportTypeUdp; 
				if(0 == strncasecmp(&szHostName[nOffset], "_tcp", strlen("_tcp") )) eTransPrort = eTransportTypeTcp; 
				if(0 == strncasecmp(&szHostName[nOffset], "_tls", strlen("_tls") )) eTransPrort = eTransportTypeTls; 

				if((eUnknownTransportType < eTransPrort)&&(enTransportTypeMAX > eTransPrort))
				{
					pPlcmDns->bSendRequestSrv(szHostName, wServId, eProcessTypeInvalid, eSigPrort,  eTransPrort);
				}
			}
		}
		this->vNearestUpdate();
	}
	else
		 this->vNearestUpdate();
}
//==============================================================================//
//==============================================================================//
void cTTLService::vTimerAwake(unsigned int par_TimeOut_Sec)//for TTL_SERVICE_TIMER
{
	TRACEINTO << "PLCM_DNS. TTLService. vTimerAwake for [" << par_TimeOut_Sec << "] Sec";
	this->StartTimer(TTL_SERVICE_TIMER, par_TimeOut_Sec * 100);
	this->m_bIsTimerOrdered = TRUE;
}
//==============================================================================//
//==============================================================================//
void cTTLService::vTimerStop      ()//for TTL_SERVICE_TIMER  
{
	TRACEINTO << "PLCM_DNS. TTLService. vTimerStop    m_bIsTimerOrdered:[" << this->m_bIsTimerOrdered <<"]";
	if(TRUE == this->m_bIsTimerOrdered)
		this->DeleteTimer(TTL_SERVICE_TIMER);
	this->m_bIsTimerOrdered = FALSE;
}
//==============================================================================//

//==============================================================================//
void cTTLService::vNearestUpdate()
{
	unsigned int dwTimeSec	  = PLCM_DNS_SERVICE_DEFAULT_TIME_SEC;

	TRACEINTO << "PLCM_DNS. TTLService.  vNearestUpdate()";
	
	TRACEINTO << "PLCM_DNS. TTLService.  vNearestUpdate() vTimerStop()";
    this->vTimerStop();

	this->m_pNearestNode = this->m_cTTLList.pGetNearestNode();

	if(NULL != this->m_pNearestNode)
	{
		unsigned int dwCurrntTime = Pm_getCurrentTimestamp(E_TIMER_RESOLUTION_SEC);
		unsigned int dwNodeTimeAwake = this->m_pNearestNode->m_sTTLNode.dwTimeStamp + this->m_pNearestNode->m_sTTLNode.dwTTL;
		if(dwCurrntTime < dwNodeTimeAwake)
			dwTimeSec = dwNodeTimeAwake - dwCurrntTime;
	}

	TRACEINTO << "PLCM_DNS. TTLService.  vNearestUpdate() vTimerAwake for [dwTimeSec" << dwTimeSec <<"] + [" << PLCM_DNS_SERVICE_REQ_SHIFT_TIME_Sec << "] Sec.";
	//dwTimeSec = min(dwTimeSec, (DWORD)PLCM_DNS_SERVICE_DEFAULT_TIME_SEC);
	this->vTimerAwake(dwTimeSec + PLCM_DNS_SERVICE_REQ_SHIFT_TIME_Sec);

	//if(NULL != this->m_pNearestNode)
	//{
	//	string  sNodeStr;
	//	sNodeStr.append((const char*)"PLCM_DNS. TTLService. vNearestUpdate. NODE:\n");
	//	this->m_pNearestNode->sDump(&sNodeStr);

	//	TRACEINTO << sNodeStr.c_str();
	//}
	//else
	//	TRACEINTO << "PLCM_DNS. TTLService. vNearestUpdate. List Empty. Will update after "<<PLCM_DNS_SERVICE_DEFAULT_TIME_SEC << " Seconds";
}
//==============================================================================//
//==============================================================================//
void cTTLService::vServiceDump(char * par_szPrefix)
{

	string   LocalStr;
	string * par_pString = &LocalStr;

	if(NULL != par_pString)
	{
		if(NULL != par_szPrefix)
		{	
			par_pString->append((const char*)par_szPrefix);
		}


		this->m_cTTLList.vListDump(par_pString);

		char szLog[512]="";
		if(NULL != this->m_pNearestNode)
		{
			char		szOrderedTimeStamp[32]="";
			char		szUpdateTime      [32]="";

			TTL_NODE *  pSNode = &this->m_pNearestNode->m_sTTLNode;

			Pm_GetTimeStringT(pSNode->dwTimeStamp, E_TIMER_RESOLUTION_SEC, E_TIMER_FORMAT_MONTH, sizeof(szOrderedTimeStamp)-1, szOrderedTimeStamp); 
			Pm_GetTimeStringT(pSNode->dwUpdateTime
				              , E_TIMER_RESOLUTION_SEC, E_TIMER_FORMAT_MONTH, sizeof(szUpdateTime)-1, szUpdateTime); 

			snprintf(szLog, sizeof(szLog)-1
				, "\n -----------------------------------\n\
				  The Nearest Update: HosName: %s | Serv.Id:%d | S.Time:%s | TTL:%d | U.Time:%s | ReqType:%s(%d) \n\
-----------------------------------"
			 , pSNode->szHostName, pSNode->wServiceID, szOrderedTimeStamp, pSNode->dwTTL, szUpdateTime, plcmDNS_szGetReqTypeName(pSNode->eDnsReqResType), pSNode->eDnsReqResType);

			par_pString->append((const char*)szLog);
		}
		else
			par_pString->append("\n -----------------------------------\n The Nearest Update:NULL\n-----------------------------------");

		TRACEINTO << par_pString->c_str();
	}
}
//==============================================================================//







