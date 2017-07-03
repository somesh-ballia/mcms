// ==========================================
//    Copyright (C) 2013           "POLYCOM"
// ==========================================
// FileName:           PlcmDNS_Processing.cpp  
// 
// ==========================================

#include <string>

#include "TraceStream.h"
#include "ProcessBase.h"
#include "SystemQuery.h"


#include "PlcmDNS_Processing.h" 
#include "StatusesGeneral.h"

#include "DNSAgentProcess.h"
#include "Trace.h"
#include "DnsRecordsMngr.h"
#include "PrettyTable.h"


#include <unistd.h>

cPLCM_cREQ_ID   G_cDnsReqID;

char GaSzSelectRc[4][32]=
{
	 {"FD_SELECT_TIMEOUT"}
	,{"FD_SELECT_EVENT  "}
	,{"FD_SELECT_EXCEPT "}

	,{"FD_SELECT_UnDEF! "}
};

char GaSzSelectEvent[5][32]=
{
	 {"FD_EVENT_TIMEOUT"}
	,{"FD_EVENT_READ   "}
	,{"FD_EVENT_WRITE  "}
	,{"FD_EVENT_EXCEPT "}

	,{"FD_EVENT_UnDEF! "} 
};
//-S- ----- BRIDGE-16374 ---------------------------------//
PLCM_DNS_BIND_PORTS  G_arrBindPorts[DNS_MAX_SERVICES]=\
{
	  //IPv4   IPv6 // -----------
	  {31000, 31001}// Management
	, {31002, 31003}// Service ID 1
	, {31004, 31005}// Service ID 2
	, {31006, 31007}// Service ID 3
	, {31008, 31009}// Service ID 4
	, {31010, 31011}// Service ID 5
	, {31012, 31013}// Service ID 6
	, {31014, 31015}// Service ID 7
	, {31016, 31017}// Service ID 8
	, {31018, 31019}// Service ID RESERVE
};
//-S- ----- BRIDGE-16374 ---------------------------------//


//==============================================================
char * m_szGetSelectRc(int par_nRc)
{
	char * pRc = GaSzSelectRc[3];
	switch(par_nRc)
	{
	case FD_SELECT_TIMEOUT: {pRc = GaSzSelectRc[0];} break;
	case FD_SELECT_EVENT  : {pRc = GaSzSelectRc[1];} break;
	case FD_SELECT_EXCEPT : {pRc = GaSzSelectRc[2];} break;
	default               : {pRc = GaSzSelectRc[3];} break;
	}
	return pRc;
}
//==============================================================

//==============================================================
char * m_szGetSelectEvent(int par_nEvent)
{
	char * pRc = GaSzSelectEvent[3];
	switch(par_nEvent)
	{
	case FD_EVENT_TIMEOUT: {pRc = GaSzSelectEvent[0];} break;
	case FD_EVENT_READ   : {pRc = GaSzSelectEvent[1];} break;
	case FD_EVENT_WRITE  : {pRc = GaSzSelectEvent[2];} break;
	case FD_EVENT_EXCEPT : {pRc = GaSzSelectEvent[3];} break;
	 
	default              : {pRc = GaSzSelectEvent[4];} break;
	}
	return pRc;
}
//==============================================================

//====================================================================//
void vCutEOL(char * par_Str)
{
	if(NULL != par_Str)
	{
		for(size_t n=0; n < strlen(par_Str); n++)
		{
			if(  ('\r' == par_Str[n])
				||('\n' == par_Str[n])
				)
			{
				par_Str[n] = '\0';
				break;
			}
		}
	}
}
//====================================================================//
//====================================================================//
void vCutSpase(char ** par_ppStr)
{
	if(NULL != par_ppStr)
	{ 
		size_t  n=0;	
		for(n=0; n < strlen(*par_ppStr); n++)
		{
			if(' ' != (*par_ppStr)[n])
			{
				*par_ppStr = &((*par_ppStr)[n]);
				break;
			}
		}

		for(n=0; n < strlen(*par_ppStr); n++)
		{
			if(' ' == (*par_ppStr)[n])
			{
				(*par_ppStr)[n] = '\0';
				break;
			}
		}
	}
}
//====================================================================//
//====================================================================//
void vParseDomainNames(DNS_PLCM_DOMAINNAME * par_aDomName, unsigned int * par_pInOutLen, char * par_pStr)
{
	if((NULL != par_aDomName)&&(NULL != par_pInOutLen)&&(NULL != par_pStr))
	{
		unsigned int A = *par_pInOutLen; 
		unsigned int n = 0;
		unsigned int R = 0;

		if(0 == memcmp(&par_pStr[0], "domain", strlen("domain")))
			par_pStr = &par_pStr[strlen("domain ")];

		if(0 == memcmp(&par_pStr[0], "search", strlen("search")))
			par_pStr = &par_pStr[strlen("search ")];	

		char * pName = par_aDomName[A].szDomainName;

		while(1)
		{
			if(A < (PLCM_DNS_DOMAINNAME_AMOUNT_MAX-1))
			{
				if(('\0' != par_pStr[n])&&(' ' != par_pStr[n]))
				{
					if(R < (PLCM_DNS_DOMAIN_NAME_SIZE-1))
					{
						pName[R] = par_pStr[n];
						R++;
					}
				}
				else
				{

					if('.' == pName[strlen(pName)-1])
						pName[strlen(pName)-1] ='\0';


					if(0 < R)
					{
						BOOL bIfDup = FALSE;

						if(0 < A)
						{
							for(unsigned int nD = 0; nD < A; nD++)
							{
								if(0 == strcmp(pName, par_aDomName[nD].szDomainName))
								{
									bIfDup = TRUE;
									memset(pName, '\0', sizeof(par_aDomName[A].szDomainName));
									break;
								}
							}
						}

						if(FALSE == bIfDup)
						{
							A++ ;
							pName = par_aDomName[A].szDomainName;
						}
						else
						{
							pName[0]= '\0';
						}
					}
					if('\0' == par_pStr[n])
						break;
					R = 0;
				}
			}
			else 
				break;

			n++;
		}

		*par_pInOutLen = A;
	}
}
//====================================================================//
//====================================================================//
cPLCM_cREQ_ID::cPLCM_cREQ_ID()
{
	MutexObjInit(&this->m_DnsReqIGMutex);
	this->m_wDnsReqIG = 0x01;
};
//====================================================================//
//====================================================================//
cPLCM_cREQ_ID::~cPLCM_cREQ_ID()
{
	M_DEINIT(this->m_DnsReqIGMutex);
	this->m_wDnsReqIG = 0x01;
};
//====================================================================//
//====================================================================//
unsigned short cPLCM_cREQ_ID::wGetReqId()
{
	M_LOCK(this->m_DnsReqIGMutex);
		this->m_wDnsReqIG++;
	M_UnLOCK(this->m_DnsReqIGMutex);

	return this->m_wDnsReqIG;
}
//====================================================================//

//====================================================================//
void vDebugPrintAddrList_String(ipAddressStruct * aDnsAddrr, unsigned int dwArrLen, string * par_cSrting)
{
	if((NULL != aDnsAddrr)&&(NULL != par_cSrting))
	{
		char szFullLog[1024 * PLCM_MAX_RESOLVED_IP]="";
		for(unsigned int nInd = 0; nInd < dwArrLen; nInd++)
		{
			char szLog[128]="";
			plcmDNS_nGetStrIpFromipAddressStruct(szLog, sizeof(szLog)-1, &aDnsAddrr[nInd]);
			strncat(szFullLog, szLog, sizeof(szFullLog) - (strlen(szFullLog) -1));
			strncat(szFullLog,"\n", sizeof(szFullLog) - strlen(szFullLog) -1);
		}

		par_cSrting->append((const char*)szFullLog);
	}
}
//====================================================================//

//====================================================================//
void vDebugPrintAddrList_StringL(ipAddressStruct * aDnsAddrr, unsigned int dwArrLen, string * par_cSrting)
{
	if((NULL != aDnsAddrr)&&(NULL != par_cSrting))
	{
		char szFullLog[1024 * PLCM_MAX_RESOLVED_IP]="";
		for(unsigned int nInd = 0; nInd < dwArrLen; nInd++)
		{
			char szLog[128]="";
			plcmDNS_nGetStrIpFromipAddressStruct(szLog, sizeof(szLog)-1, &aDnsAddrr[nInd]);
			strncat(szFullLog, szLog, sizeof(szFullLog) - (strlen(szFullLog) -1));
			strncat(szFullLog," ", sizeof(szFullLog) - strlen(szFullLog) -1);
		}

		par_cSrting->append((const char*)szFullLog);
	}
}
//====================================================================//

//====================================================================//
void vDebugPrintDnsAddrList_String(cDNS_PLCM_REQ_NODE * par_pNode, string * par_cSrting)
{
	if(NULL != par_pNode)
	{
		char szTxt[1024]="";
		sprintf(szTxt, "     DNS-Addr List: Amount[%d] | Current index[%d]\n"
			, par_pNode->m_sReqNode.dwDnsAddressAmount, par_pNode->m_sReqNode.dwDnsAddress_Current);
		par_cSrting->append((const char*)szTxt);
		vDebugPrintAddrList_String(par_pNode->m_sReqNode.aDnsAddress, par_pNode->m_sReqNode.dwDnsAddressAmount, par_cSrting);
	}
}
//====================================================================//

//====================================================================//
void vDebugPrintResolveAddrList_String(DNS_IPADDR_TTL * par_aResolved, unsigned int par_dwArrLen, string * par_cSrting)
{
	if((NULL != par_aResolved)&&(NULL != par_cSrting))
	{
		char szTxt[1024]="";
		par_cSrting->append((const char*)szTxt);
	
		if(0 < par_dwArrLen)
		{
			for(unsigned int dwN = 0; dwN < par_dwArrLen; dwN++)
			{
				sprintf(szTxt, "| TTL: %5d | ", par_aResolved[dwN].dwTTL);
				par_cSrting->append((const char*)szTxt);

				plcmDNS_nGetStrIpFromipAddressStruct(szTxt, sizeof(szTxt)-1, &par_aResolved[dwN].sIpAddr);
				par_cSrting->append((const char*)szTxt);

				sprintf(szTxt, "\n     ");
				par_cSrting->append((const char*)szTxt);
			}
		}
	}
}
//====================================================================//
//====================================================================//
void vDebugPrintResolveAddrList_String(cDNS_PLCM_REQ_NODE * par_pNode, string * par_cSrting)
{
	if((NULL != par_pNode)&&(NULL != par_cSrting))
	{
		char szTxt[1024]="";
		sprintf(szTxt, "     Resolved-Addr List: Amount[%d] \n     ", par_pNode->m_sReqNode.dwDnsAddressAmount);
		par_cSrting->append((const char*)szTxt);

		vDebugPrintResolveAddrList_String(par_pNode->m_sReqNode.aResoledAddressAndTTL, par_pNode->m_sReqNode.dwResolvedAddrAmount, par_cSrting);
	}
}
//====================================================================//

//====================================================================//
void vDebugPrintDomainName_StringL(DNS_PLCM_DOMAINNAME * par_pNode, DWORD par_arrLen, string * par_cSrting)
{
	if(NULL != par_pNode)
	{
		char szDomainList[1024 * PLCM_DNS_DOMAINNAME_AMOUNT_MAX] = "" ;
		for(unsigned int nInd = 0; nInd < par_arrLen; nInd++)
		{
			strncat(szDomainList
				, (0 == strlen(par_pNode[nInd].szDomainName))? "<empty>": par_pNode[nInd].szDomainName
				, sizeof(szDomainList) - (strlen(szDomainList)-1));

			strncat(szDomainList, " ", sizeof(szDomainList) - (strlen(szDomainList)-1));
		}
		par_cSrting->append((const char*)szDomainList);	
	}
}
//====================================================================//

//====================================================================//
void vDebugPrintDomainName_String(cDNS_PLCM_REQ_NODE * par_pNode, string * par_cSrting)
{
	if(NULL != par_pNode)
	{
		char szDomainList[1024 * PLCM_DNS_DOMAINNAME_AMOUNT_MAX] = "" ;
		sprintf(szDomainList, "     DomainName(s) List [%d] | Current index[%d] \n"
			               , par_pNode->m_sReqNode.dwDomainNameAmount
						   , par_pNode->m_sReqNode.dwDomainName_Current);
		for(unsigned int nInd = 0; nInd < par_pNode->m_sReqNode.dwDomainNameAmount; nInd++)
		{
			strncat(szDomainList
				, ('\0' == par_pNode->m_sReqNode.aDomainName[nInd].szDomainName[0])? "<empty>": par_pNode->m_sReqNode.aDomainName[nInd].szDomainName
				, sizeof(szDomainList) - (strlen(szDomainList)-1));

			strncat(szDomainList, "\n", sizeof(szDomainList) - (strlen(szDomainList)-1));
		}
		par_cSrting->append((const char*)szDomainList);	
	}

}
//====================================================================//
//====================================================================//
void vReqNode_DebugPrint(cDNS_PLCM_REQ_NODE * par_RequestNode, string * par_pOutString)
{
	if(NULL != par_RequestNode)
	{
		char    szTxt[2048]="";
		
		snprintf(szTxt, sizeof(szTxt),"     Name:[%s]\n     Req.Id:[%d](0x%X) | Partner Req.Id.[%d](0x%X)\n"
			, par_RequestNode->m_sReqNode.szHostName
			, par_RequestNode->m_sReqNode.wDnsReqID, par_RequestNode->m_sReqNode.wDnsReqID
			, par_RequestNode->m_sReqNode.wPartnerDnsReqID, par_RequestNode->m_sReqNode.wPartnerDnsReqID);
		par_pOutString->append((const char*)szTxt);	

		//Klocwork.Sasha #ID.3999
		snprintf(szTxt, sizeof(szTxt),"     Required Type :[%d] (%s)\n", par_RequestNode->m_sReqNode.wReqType, plcmDNS_szGetReqTypeName(par_RequestNode->m_sReqNode.wReqType));
		par_pOutString->append((const char*)szTxt);	

		snprintf(szTxt, sizeof(szTxt), "     ServiceID:[%d]\n     Process type:[%d]/%s\n"
			, par_RequestNode->m_sReqNode.wServiceID, par_RequestNode->m_sReqNode.nProcessType
			, ((eProcessLogger <= par_RequestNode->m_sReqNode.nProcessType)&&(eProcessDNSAgent >= par_RequestNode->m_sReqNode.nProcessType))?
			           CProcessBase::GetProcessName((eProcessType)par_RequestNode->m_sReqNode.nProcessType):" ????? "
					   );
		par_pOutString->append((const char*)szTxt);	

		snprintf(szTxt, sizeof(szTxt), "     IP version: IPv4:[%d/%s] | IPv6[%d/%s]\n"
			, par_RequestNode->m_sReqNode.bIsRequiredIpv4, (TRUE == par_RequestNode->m_sReqNode.bIsRequiredIpv4)? "Required":"NOT Required"
			, par_RequestNode->m_sReqNode.bIsRequiredIpv6, (TRUE == par_RequestNode->m_sReqNode.bIsRequiredIpv6)? "Required":"NOT Required");
		par_pOutString->append((const char*)szTxt);	

		char szTimeStamp[64]="";
		Pm_GetTimeString(E_TIMER_RESOLUTION_MILI, E_TIMER_FORMAT_MONTH,  sizeof(szTimeStamp)-1, szTimeStamp);   
		snprintf(szTxt, sizeof(szTxt), "     TimeStamp_Request:[%s]\n     TimeOut (mSec):[%d]\n     wAttempt:[%d/%d]\n"
			   , szTimeStamp, par_RequestNode->m_sReqNode.dwTimeOut_mSec, par_RequestNode->m_sReqNode.wAttempt_Current +1
			   , par_RequestNode->m_sReqNode.wAttempts);

		par_pOutString->append((const char*)szTxt);	

		vDebugPrintDnsAddrList_String(par_RequestNode, par_pOutString);
		vDebugPrintDomainName_String (par_RequestNode, par_pOutString);

		snprintf(szTxt, sizeof(szTxt), " --------------------------------------------\n");
		par_pOutString->append((const char*)szTxt);	
	
	}
}
//====================================================================//

//====================================================================//
void vGetHostNameReal(char * pOut, size_t par_dwOutLen, char * par_pHostName, char * par_pDomainName)
{
	if((NULL != pOut)&&(NULL != par_pHostName)&&(1 < par_dwOutLen))
	{
		memset(pOut, '\0', par_dwOutLen);
		strncpy(pOut, par_pHostName, par_dwOutLen - 1);
        pOut[par_dwOutLen - 1]='\0'; 
		if('\0' != par_pDomainName[0])
		{
			char * pBreak = strstr(pOut, par_pDomainName);
			if(NULL != pBreak)
				pBreak[0]='\0';
		}
	}
}
//====================================================================//

//====================================================================//
cMsgDnsRECV::cMsgDnsRECV()
{
	this->vClean();
}
//====================================================================//
//====================================================================//
cMsgDnsRECV::cMsgDnsRECV(
			  DNS_IPADDR_TTL * par_aResTTLAndAddr
			, int              par_nArrLen
			, char           * par_szHostName
			, WORD             par_wReqId
			, WORD             par_wReqType
			, BOOL             par_bIsRecv)
{
	this->vClean();

	if(NULL != par_aResTTLAndAddr)
		memcpy(this->m_sMsgData.m_aResoledAddrrAndTTL, par_aResTTLAndAddr, sizeof(DNS_IPADDR_TTL)* PLCM_MAX_RESOLVED_IP);
	this->m_sMsgData.m_nResolved = par_nArrLen;
	if(NULL != par_szHostName)
		strncpy(this->m_sMsgData.m_szHostNameComplex, par_szHostName, sizeof(this->m_sMsgData.m_szHostNameComplex)-1);
    this->m_sMsgData.m_ReqId = par_wReqId;
	this->m_sMsgData.m_wReqType = par_wReqType;
	this->m_sMsgData.m_bIsResponseArrived = par_bIsRecv;
}
//====================================================================//
//====================================================================//
cMsgDnsRECV::~cMsgDnsRECV()
{
	this->vClean();
}
//====================================================================//
//====================================================================//
void cMsgDnsRECV::vClean()
{
	memset(&this->m_sMsgData, 0, sizeof(sDNS_RECV));
}
//====================================================================//

//====================================================================//
void cMsgDnsRECV::Serialize(WORD format, CSegment &seg)
{
	if (format == NATIVE)
	{
		seg.Put((BYTE *) &this->m_sMsgData, sizeof(sDNS_RECV));
	}
}
//====================================================================//
//====================================================================//
void cMsgDnsRECV::DeSerialize(WORD format, CSegment &seg)
{
	if (format == NATIVE)
	{
		seg.Get((BYTE *) &this->m_sMsgData, sizeof(sDNS_RECV));
	}  
}
//====================================================================//
//====================================================================//
void cMsgDnsRECV::DumpR(char * headerTxt)
{
	CLargeString msg;
	msg << "*** cMsgDnsRECV::DumpR - " << headerTxt;
	if(FALSE == this->m_sMsgData.m_bIsResponseArrived)
		msg << "\n ------!!DNS Response TIMEOUTED!! -----------";
	msg << "\n DNS Request           :  #" <<  this->m_sMsgData.m_ReqId;
	msg << "\n DNS Req.Type          :  " << plcmDNS_szGetReqTypeName(this->m_sMsgData.m_wReqType);
	msg << "\n DNS Required HOST name:  " << this->m_sMsgData.m_szHostNameComplex ;

	if(TRUE == this->m_sMsgData.m_bIsResponseArrived)
	{
		msg << "\n DNS Resolved Amount   :  " << this->m_sMsgData.m_nResolved ;
		if(0 < this->m_sMsgData.m_nResolved)
		{
			for(int nInd = 0; nInd < this->m_sMsgData.m_nResolved; nInd++)
			{
				char szIP[128]="";
				plcmDNS_nGetStrIpFromipAddressStruct(szIP, sizeof(szIP)-1, &this->m_sMsgData.m_aResoledAddrrAndTTL[nInd].sIpAddr);

				msg << "\n    | TTL:"<<this->m_sMsgData.m_aResoledAddrrAndTTL[nInd].dwTTL
					      << "| " << szIP;  
			}
		}
	}
	msg << "\n ----------------------------------";

	PTRACE (eLevelInfoNormal, msg.GetString());
}
//====================================================================//






//====================================================================//
cPLCM_DnsUdpRecvThr::cPLCM_DnsUdpRecvThr(/*cDnsReqList  * par_pDnsReqList*/
									     void		  * par_pVoidcPLCM_DNS)
: PLc_THREAD("cPLCM_DnsUdpRecvThr")
{
	this->m_nThrState		= 0;
	this->m_pVoidcPLCM_DNS  = NULL;
	
	if(NULL != par_pVoidcPLCM_DNS)
	{
		m_pVoidcPLCM_DNS	= par_pVoidcPLCM_DNS;

		this->ActivateDeTach();
		this->m_nThrState = 1;
	}
}
//====================================================================//
//====================================================================//
cPLCM_DnsUdpRecvThr::~cPLCM_DnsUdpRecvThr()
{
	if(0 != this->m_nThrState)
	{
		this->EndSignalClose();
		this->m_nThrState		= 0;
	}
}
//====================================================================//
//====================================================================//
void cPLCM_DnsUdpRecvThr::vReadProcessing(EVENT_INFO * par_pEvenInfo)
{
	cPLCM_DNS * pPlcmDns = (cPLCM_DNS *)this->m_pVoidcPLCM_DNS;

	if((NULL != par_pEvenInfo)&&(NULL != pPlcmDns))
	{
		DNS_IPADDR_TTL		aResoledAddrrAndTTL[PLCM_MAX_RESOLVED_IP];
		int					nResolved		   = PLCM_MAX_RESOLVED_IP;

		for(int nS = 0; nS <DNS_MAX_SERVICES; nS++)
		{
			if(FD_EVENT_READ == par_pEvenInfo[nS].eSelectEvent)
			{
				cDNS_SOCKET * pDnsCosket = par_pEvenInfo[nS].pDnsSocket;
				if(NULL != pDnsCosket)
				{
					char				RecvBuff[1024 * 10]= "";
					char				szRecvIP[128]	   = "";
					unsigned short		wRecvPort		   = 0 ;
					int					nRcReceived		   = 0 ;

					if(eIpVersion4 == pDnsCosket->m_IpV)

						nRcReceived = pDnsCosket->m_RecvFrom4A(RecvBuff, sizeof(RecvBuff),szRecvIP, sizeof(szRecvIP)-1,&wRecvPort);
					else if(eIpVersion6 == pDnsCosket->m_IpV)
						nRcReceived = pDnsCosket->m_RecvFrom6A(RecvBuff, sizeof(RecvBuff),szRecvIP, sizeof(szRecvIP)-1,&wRecvPort);

					if( 0 < nRcReceived)
					{
						plcmPrintBuffHexStr(RecvBuff, nRcReceived, "DNS response:", 0);

						memset(aResoledAddrrAndTTL, 0, sizeof(DNS_IPADDR_TTL) * PLCM_MAX_RESOLVED_IP);

						DNS_PACKET_HEADER  *pDnsHeader = (DNS_PACKET_HEADER*) RecvBuff;
						char				szHostNameComplex[PLCM_DNS_HOST_NAME_SIZE]="";
						unsigned short		wReqId     = 0; 	
						unsigned short		wOutType   = 0;	

						plcmDNS_DnsAnswerParsing(RecvBuff, nRcReceived, aResoledAddrrAndTTL, &nResolved, szHostNameComplex, sizeof(szHostNameComplex)-1, &wReqId, &wOutType);

						TRACEINTO << "PLCM_DNS.[DNS_RECV]. " << "#[" << wReqId << "]"
							<< " | " << plcmDNS_szGetReqTypeName(wOutType) 
							<< " | HostName:[" << szHostNameComplex << "]"
							<< " | Form DNS:[" << szRecvIP << "]"
							<< " || Received bytes: [" << nRcReceived <<"]"; 

						{//---------- SEND MSG TO State-Mash-----------//
							cMsgDnsRECV DnsRecvMsg (aResoledAddrrAndTTL, nResolved, szHostNameComplex, wReqId, wOutType, TRUE);
							CSegment*  pSegRecv = new CSegment();
							DnsRecvMsg.Serialize (NATIVE, *pSegRecv);

							pPlcmDns->m_pRcvMbxDns->Send(pSegRecv, DNS_REQ_RECV);
						}
					}
				}//if(NULL != pDnsCosket)
			}//if(FD_EVENT_READ == par_pEvenInfo[nS].eSelectEvent)
			else
			{
				//TRACEINTO << "PLCM_DNS. EVENT_READ [" << par_pEvenInfo[nS].eSelectEvent <<"]"; 
			}
		}//for(int nS = 0; nS <DNS_MAX_SERVICES; nS++)
	}
}
//====================================================================//
//====================================================================//
void cPLCM_DnsUdpRecvThr::Svc()
{
	FTRACEINTO << "   PLCM_DNS.[DNS_RECV]. cPLCM_DnsUdpRecvThr::Svc [started]" ;
	cPLCM_DNS * pPlcmDns = (cPLCM_DNS *)this->m_pVoidcPLCM_DNS;

	for(int nS = 0; nS < DNS_MAX_SERVICES; nS++)
	{
		if(  ((0 == pPlcmDns->m_DnsArrSocket4.m_arSocketsSelected[nS])    ||(65535 == pPlcmDns->m_DnsArrSocket4.m_arSocketsSelected[nS]))
           ||((0 == pPlcmDns->m_DnsArrSocket4.m_SockObjArray[nS].m_Socket)||(65535 == pPlcmDns->m_DnsArrSocket4.m_SockObjArray[nS].m_Socket))
		  )
		{
			pPlcmDns->m_DnsArrSocket4.m_arSocketsSelected[nS] = INVALID_SOCKET;
			pPlcmDns->m_DnsArrSocket4.m_SockObjArray[nS].m_Socket = INVALID_SOCKET;
		}

		if(  ((0 == pPlcmDns->m_DnsArrSocket6.m_arSocketsSelected[nS])    ||(65535 == pPlcmDns->m_DnsArrSocket6.m_arSocketsSelected[nS]))
			||((0 == pPlcmDns->m_DnsArrSocket6.m_SockObjArray[nS].m_Socket)||(65535 == pPlcmDns->m_DnsArrSocket6.m_SockObjArray[nS].m_Socket))
			)
		{
			pPlcmDns->m_DnsArrSocket6.m_arSocketsSelected[nS] = INVALID_SOCKET;
			pPlcmDns->m_DnsArrSocket6.m_SockObjArray[nS].m_Socket = INVALID_SOCKET;
		}
	}

	while(false == this->IsFinished() && (NULL != pPlcmDns))
	{
		if(   (ePLCM_DNS_STATE_ACTIVATED == pPlcmDns->m_eObjState)
			&&(TRUE == pPlcmDns->m_DnsArrSocket4.m_IsSocketArrActived) 
			&&(TRUE == pPlcmDns->m_DnsArrSocket6.m_IsSocketArrActived) 
		  )
		{
			char				RecvBuff[1024 * 10]= "";
			char				szRecvIP[128]	   = "";
			unsigned short		wRecvPort		   = 0 ;
			int					nRcReceived		   = 0 ;
			DNS_IPADDR_TTL		aResoledAddrrAndTTL[PLCM_MAX_RESOLVED_IP];
			int					nResolved		   = PLCM_MAX_RESOLVED_IP;
			BOOL                bIsIpv6SockNow     = FALSE; 
			int					nMultiSelectRc4     = FD_SELECT_TIMEOUT;
			int					nMultiSelectRc6     = FD_SELECT_TIMEOUT;

			memset(szRecvIP,'\0', sizeof(szRecvIP));
			for(int nS = 0; nS < DNS_MAX_SERVICES; nS++)
			{
				pPlcmDns->m_DnsArrSocket4.m_arEventInfo[nS].eSelectEvent = FD_EVENT_TIMEOUT;
				pPlcmDns->m_DnsArrSocket6.m_arEventInfo[nS].eSelectEvent = FD_EVENT_TIMEOUT;
			}

			nMultiSelectRc4	= plcmSockListReadSelectTimeOut(pPlcmDns->m_DnsArrSocket4.m_arSocketsSelected
				                                          , pPlcmDns->m_DnsArrSocket4.m_arEventInfo, DNS_MAX_SERVICES, 20);
			if(FD_SELECT_EVENT == nMultiSelectRc4)
			{
				this->vReadProcessing(pPlcmDns->m_DnsArrSocket4.m_arEventInfo);
			}

			nMultiSelectRc6	= plcmSockListReadSelectTimeOut(pPlcmDns->m_DnsArrSocket6.m_arSocketsSelected
				                                          , pPlcmDns->m_DnsArrSocket6.m_arEventInfo, DNS_MAX_SERVICES, 20);
			if(FD_SELECT_EVENT == nMultiSelectRc6)
			{
				this->vReadProcessing(pPlcmDns->m_DnsArrSocket6.m_arEventInfo);
			}
		}
		else
		{
			this->Sleep(200);
		}
	}
	TRACEINTO << "   PLCM_DNS.[DNS_RECV]. cPLCM_DnsUdpRecvThr::Svc [finished]" ;
}
//====================================================================//


//---for TEST only!!!
PLCM_DNS_SERVICE_CONFIG  G_TestServConf[DNS_MAX_SERVICES];



//====================================================================//
cDNS_SERVICE_CONFIG::cDNS_SERVICE_CONFIG()
{
	this->m_wSerID = 0;
	this->vClean(0);
}
//====================================================================//
//====================================================================//
cDNS_SERVICE_CONFIG::~cDNS_SERVICE_CONFIG()
{
	this->m_wSerID = 0;
	this->vClean(0);
}
//====================================================================//
//====================================================================//
void cDNS_SERVICE_CONFIG::vClean(WORD par_ServId)
{
	memset(&this->m_sConf, 0, sizeof(PLCM_DNS_SERVICE_CONFIG));
	this->m_wSerID = par_ServId;
//-S- ----- BRIDGE-16374 ---------------------------------//
	if(par_ServId < DNS_MAX_SERVICES)
	{
		this->m_sConf.sBindPorts.wPort_v4 = G_arrBindPorts[par_ServId].wPort_v4;
		this->m_sConf.sBindPorts.wPort_v6 = G_arrBindPorts[par_ServId].wPort_v6;
	}
//-E- ----- BRIDGE-16374 ---------------------------------//	
}
//====================================================================//
//====================================================================//
BOOL cDNS_SERVICE_CONFIG::bGetResolvConfData(PLCM_DNS_SERVICE_CONFIG * par_pConf, WORD par_ServId)
{
	BOOL bRc = FALSE;

	if(NULL != par_pConf)
	{
		FILE		*	fS       = NULL   ;
		char		*	pStr     = NULL   ;
		PLCM_DNS_SERVICE_CONFIG  sConf	  ;	

		memset(&sConf, 0, sizeof(PLCM_DNS_SERVICE_CONFIG));

		sConf.dwTimeOut_InSec    = PLCM_DNS_TIMEOUT_DEFAULT_SEC;
		sConf.dwDomainNameAmount = 1; //For EMPTY domain!!!

		char        szResovvConf[256]="/etc/resolv.conf";

		pStr = new char[1024 * 10];
		if(NULL != pStr)
		{
			memset(pStr, 0,1024 * 10);

			fS = fopen(szResovvConf, "rb");  

			if(NULL == fS)
			{
				TRACEINTO << "PLCM_DNS.Configuration . File: etc/resolv.conf - not found";  
			}
			else
			{
				while(NULL != fgets(pStr, 1024 * 10 , fS))  
				{
					if(0 == memcmp(&pStr[0], "nameserver", strlen("nameserver")))
					{
						char   szIp[256]="";
						char * pIp = &pStr[strlen("nameserver ")];
						char * pIpR = NULL; 
						strncpy(szIp, pIp, sizeof(szIp)-1);
						pIpR = szIp;
						vCutEOL(pIpR);
						vCutSpase(&pIpR);

						if((NULL != pIpR)&&(0 < strlen(pIpR)))	
						{
							if(sConf.dwDnsAddressAmount < PLCM_DNS_MAX_ADDITIONAL_IP)
							{	
								stringToIp(&sConf.aDnsAddress[sConf.dwDnsAddressAmount], pIpR, eNetwork);
								sConf.dwDnsAddressAmount++;
							}
						}
					}
					else
						if(  (0 == memcmp(&pStr[0], "domain", strlen("domain")))
							||(0 == memcmp(&pStr[0], "search", strlen("search")))
							)
						{
							char   szDom[256]="";
							strncpy(szDom, pStr, sizeof(szDom)-1);
							vCutEOL(szDom);
							vParseDomainNames(&sConf.aDomainName[0], &sConf.dwDomainNameAmount, szDom);
						}
						else
						if(0 == memcmp(&pStr[0], "options attempts", strlen("options attempts")))
						{//default 2
							char   szAt[256]="";
							char * pAt = &pStr[strlen("options attempts:")];
							char * pAtR = NULL; 
							strncpy(szAt, pAt, sizeof(szAt)-1);
							pAtR = szAt;
							vCutEOL(pAtR);
							vCutSpase(&pAtR);
							sConf.wAttempts = (unsigned short) atoi(pAtR);
						}
						else
						if(0 == memcmp(&pStr[0], "options timeout", strlen("options timeout")))
						{//default 5 sec
							char   szTo[256]="";
							char * pTo = &pStr[strlen("options timeout:")];
							char * pToR = NULL; 
							strncpy(szTo, pTo, sizeof(szTo)-1);
							pToR = szTo;
							vCutEOL(pToR);
							vCutSpase(&pToR);
							sConf.dwTimeOut_InSec = (unsigned short) atoi(pToR);
						}
				}
			}

			memcpy(par_pConf, &sConf , sizeof(PLCM_DNS_SERVICE_CONFIG));
			bRc = TRUE;

			if(NULL != pStr)
				delete [] pStr;
			pStr = NULL;
			if(NULL != fS)
				fclose(fS);
			fS = NULL;
		}//
	}
	return bRc;
}
//====================================================================//
//====================================================================//
void cDNS_SERVICE_CONFIG::vSetConf(WORD par_ServId, DNS_IP_SERVICE_S * par_pServConfParam, WORD par_wIpType)//eIpType
{
	if(NULL != par_pServConfParam)
	{
		DNS_IP_SERVICE_S  * pServ = par_pServConfParam; 
		char szPrefix[64]="";

		if(  (eServerStatusOff  == (eServerStatus)par_pServConfParam->dnsStatus)//eServerStatus
           ||(eServerStatusAuto == (eServerStatus)par_pServConfParam->dnsStatus) 
		  )
		{
			snprintf(szPrefix, sizeof(szPrefix)-1, "DNS.Conf:[%s]"
                     , (eServerStatusOff  == (eServerStatus)par_pServConfParam->dnsStatus)? "OFF": "AUTO"
					);

			this->bGetResolvConfData(&this->m_sConf, par_ServId);
		}
		else
		{
			DWORD nD  = 0;
			this->m_sConf.dwDnsAddressAmount = 0;

			snprintf(szPrefix, sizeof(szPrefix)-1, "DNS.Conf Arrived:");

			for(nD = 0; nD < PLCM_DNS_MAX_ADDITIONAL_IP; nD++)
			{
				if(0 != pServ->serversIpv4List[nD])
				{
					this->m_sConf.aDnsAddress[this->m_sConf.dwDnsAddressAmount].ipVersion = eIpVersion4;
					this->m_sConf.aDnsAddress[this->m_sConf.dwDnsAddressAmount].addr.v4.ip =  ntohl(pServ->serversIpv4List[nD]);
					this->m_sConf.dwDnsAddressAmount++;
				}
				else
				{
					char * pIpv6 = pServ->serversIpv6List[nD];
					if(0 != strlen(pIpv6) )
					{
						stringToIp(&this->m_sConf.aDnsAddress[this->m_sConf.dwDnsAddressAmount], pIpv6, eNetwork);
						this->m_sConf.dwDnsAddressAmount++;
					}
				}

			}

            memset(this->m_sConf.aDomainName, 0, sizeof(DNS_PLCM_DOMAINNAME)* PLCM_DNS_DOMAINNAME_AMOUNT_MAX);
			this->m_sConf.dwDomainNameAmount = 1;

			if(0 < strlen(pServ->szDomainName))
			{
				strncpy(this->m_sConf.aDomainName[1].szDomainName
					  , pServ->szDomainName
					  , sizeof(this->m_sConf.aDomainName[1].szDomainName)-1);

				this->m_sConf.dwDomainNameAmount++;
			}
		}

		switch(par_wIpType)
		{
		case eIpType_IpV4:{this->m_sConf.bIsRequest_A = TRUE ; this->m_sConf.bIsRequest_AAAA = FALSE;} break;
		case eIpType_IpV6:{this->m_sConf.bIsRequest_A = FALSE; this->m_sConf.bIsRequest_AAAA = TRUE ;} break;

		case eIpType_Both: 
		default          :{this->m_sConf.bIsRequest_A = TRUE ; this->m_sConf.bIsRequest_AAAA = TRUE ;} break;
		}
//-S- ----- BRIDGE-16374 ---------------------------------//		
		if(par_ServId < DNS_MAX_SERVICES)
		{
			this->m_sConf.sBindPorts.wPort_v4 = G_arrBindPorts[par_ServId].wPort_v4;
			this->m_sConf.sBindPorts.wPort_v6 = G_arrBindPorts[par_ServId].wPort_v6;
		}
//-E- ----- BRIDGE-16374 ---------------------------------//		
		this->vDumpConfig(szPrefix);
	}
}
//====================================================================//
//====================================================================//
void cDNS_SERVICE_CONFIG::vGetConf(WORD par_ServId)
{
	memset(&this->m_sConf, 0, sizeof(PLCM_DNS_SERVICE_CONFIG));
	this->m_wSerID = par_ServId;
	

	if(TRUE == this->bGetResolvConfData(&this->m_sConf, this->m_wSerID) )
	{
		this->m_sConf.bIsRequest_A    = TRUE; //TBD
		this->m_sConf.bIsRequest_AAAA = TRUE; //TBD
	}
	else
	{
		this->m_sConf.dwTimeOut_InSec = 5;
		this->m_sConf.wAttempts       = 0;
	}
//-S- ----- BRIDGE-16374 ---------------------------------//
	if(par_ServId < DNS_MAX_SERVICES)
	{
		this->m_sConf.sBindPorts.wPort_v4 = G_arrBindPorts[par_ServId].wPort_v4;
		this->m_sConf.sBindPorts.wPort_v6 = G_arrBindPorts[par_ServId].wPort_v6;
	}
//-E- ----- BRIDGE-16374 ---------------------------------//
	if(0 < this->m_wSerID)
	{
		//TBD per ServiseID
	}

	
}
//====================================================================//
//====================================================================//
void cDNS_SERVICE_CONFIG::vDumpConfig(char * par_pPrefix)
{
	string  cSrtingLog;
	char	szTxt[1024]="";
	snprintf(szTxt, sizeof(szTxt)-1,"PLCM_DNS.Configuration. ServId[%d] %s \n DNS-Servers[%2d] : "
		       , this->m_wSerID
			   ,(NULL != par_pPrefix)? par_pPrefix:"" 
			   , this->m_sConf.dwDnsAddressAmount);
	cSrtingLog.append((const char*)szTxt);

	vDebugPrintAddrList_StringL(this->m_sConf.aDnsAddress,  this->m_sConf.dwDnsAddressAmount, &cSrtingLog);

    snprintf(szTxt, sizeof(szTxt)-1,"\n Domain name[%2d] : ", this->m_sConf.dwDomainNameAmount);
	cSrtingLog.append((const char*)szTxt);

	for(unsigned int nD = 0; nD < this->m_sConf.dwDomainNameAmount; nD++)
	{
		snprintf(szTxt, sizeof(szTxt)-1,"%s "
			, ('\0' == this->m_sConf.aDomainName[nD].szDomainName[0])? "<empty>" : this->m_sConf.aDomainName[nD].szDomainName);
		cSrtingLog.append((const char*)szTxt);	
	}
//-S- ----- BRIDGE-16374 ---------------------------------//	
	snprintf(szTxt, sizeof(szTxt)-1,"\n Req.IP-Type               : IPv4=%s | IPv6=%s"
		      , (FALSE == this->m_sConf.bIsRequest_A)? "FALSE":"TRUE"
		      , (FALSE == this->m_sConf.bIsRequest_AAAA)? "FALSE":"TRUE" );
	cSrtingLog.append((const char*)szTxt);

	snprintf(szTxt, sizeof(szTxt)-1,"\n TimeOut (second)          : %d", this->m_sConf.dwTimeOut_InSec);
	cSrtingLog.append((const char*)szTxt);

	snprintf(szTxt, sizeof(szTxt)-1,"\n Add. Attempt              : %d", this->m_sConf.wAttempts);
	cSrtingLog.append((const char*)szTxt);

	snprintf(szTxt, sizeof(szTxt)-1,"\n Management/Signaling IPv4 : %s (%d)", this->m_sConf.szSignalingIPv4, this->m_sConf.sBindPorts.wPort_v4);
	cSrtingLog.append((const char*)szTxt);
	snprintf(szTxt, sizeof(szTxt)-1,"\n Management/Signaling IPv6 : %s (%d)\n", this->m_sConf.szSignalingIPv6, this->m_sConf.sBindPorts.wPort_v6);
	cSrtingLog.append((const char*)szTxt);
//-E- ----- BRIDGE-16374 ---------------------------------//

	TRACEINTO << cSrtingLog.c_str();
}
//====================================================================//



//====================================================================//
cPLCM_DNS::cPLCM_DNS()
{
//---Outbound Proxy ---//
	memset(this->n_Buff1, 0, sizeof(this->n_Buff1));
	memset(this->n_Buff2, 0, sizeof(this->n_Buff2));
	memset(this->n_Buff3, 0, sizeof(this->n_Buff3));
//---Outbound Proxy ---//

	this->m_pRcvMbxDns = NULL;	
	this->m_ReqList.bClean();
	this->m_pDnsRecvThr = NULL;
	this->m_eObjState   = ePLCM_DNS_STATE_IDLE;
	this->m_pTTLService = NULL;
    this->m_pShManager	= NULL;	

	this->m_state       = IDLE;



	//---for TEST only!!!
	//-------------------
/*
	{
		memset(G_TestServConf, 0, sizeof(PLCM_DNS_SERVICE_CONFIG) * DNS_MAX_SERVICES);
		
		PLCM_DNS_SERVICE_CONFIG  * pTestCong = NULL;

        //Serv 0:

		//Serv 1:
		//POLYCOM DNS address 10.226.232.10
		
		pTestCong = &G_TestServConf[1];
			
			pTestCong->aDnsAddress[0].ipVersion = eIpVersion4;
			pTestCong->aDnsAddress[0].addr.v4.ip = 0x0AE8E20A ;//NETWORK Mode! //10.226.232.10 
			
			pTestCong->dwDnsAddressAmount = 1;

			strcpy(pTestCong->aDomainName[0].szDomainName, ""); 
			pTestCong->dwDomainNameAmount++;
			strcpy(pTestCong->aDomainName[1].szDomainName, "dev13.std"); 
			pTestCong->dwDomainNameAmount++;
			strcpy(pTestCong->aDomainName[2].szDomainName, "com"); 
			pTestCong->dwDomainNameAmount++;

			pTestCong->bIsRequest_A    = TRUE;
			pTestCong->bIsRequest_AAAA = TRUE;

			pTestCong->dwTimeOut_InSec = 5;// in sec.

			pTestCong->wAttempts       = 0;

		//Serv 2:
		pTestCong = &G_TestServConf[2];
			//pTestCong->aDnsAddress[0].ipVersion = eIpVersion4;
			//pTestCong->aDnsAddress[0].addr.v4.ip = 0x08080808;//NETWORK Mode! //10.226.232.10
			//                                    // 0xDEDE43D0 ;//NETWORK Mode! //208.67.222.222 

		pTestCong->aDnsAddress[0].ipVersion  = eIpVersion6;
		//abcd:0010:0226:0113::0002
		pTestCong->aDnsAddress[0].addr.v6.ip[0] = 0xAB; 
		pTestCong->aDnsAddress[0].addr.v6.ip[1] = 0xCD; 
		pTestCong->aDnsAddress[0].addr.v6.ip[2] = 0x00; 
		pTestCong->aDnsAddress[0].addr.v6.ip[3] = 0x10; 
		pTestCong->aDnsAddress[0].addr.v6.ip[4] = 0x02; 
		pTestCong->aDnsAddress[0].addr.v6.ip[5] = 0x26; 
		pTestCong->aDnsAddress[0].addr.v6.ip[6] = 0x01; 
		pTestCong->aDnsAddress[0].addr.v6.ip[7] = 0x13; 
		pTestCong->aDnsAddress[0].addr.v6.ip[8] = 0x00; 
		pTestCong->aDnsAddress[0].addr.v6.ip[9] = 0x00; 
		pTestCong->aDnsAddress[0].addr.v6.ip[10] =0x00; 
		pTestCong->aDnsAddress[0].addr.v6.ip[11] =0x00; 
		pTestCong->aDnsAddress[0].addr.v6.ip[12] =0x00; 
		pTestCong->aDnsAddress[0].addr.v6.ip[13] =0x00; 
		pTestCong->aDnsAddress[0].addr.v6.ip[14] =0x00; 
		pTestCong->aDnsAddress[0].addr.v6.ip[15] =0x02; 

			pTestCong->dwDnsAddressAmount = 1;

			strcpy(pTestCong->aDomainName[0].szDomainName, ""); 
			pTestCong->dwDomainNameAmount++;
			strcpy(pTestCong->aDomainName[1].szDomainName, "com"); 
			pTestCong->dwDomainNameAmount++;

			pTestCong->bIsRequest_A    = TRUE;
			pTestCong->bIsRequest_AAAA = FALSE;

			pTestCong->dwTimeOut_InSec = 5;// in sec.

			pTestCong->wAttempts       = 0;

		//Serv 3:
		pTestCong = &G_TestServConf[3];
			pTestCong->aDnsAddress[0].ipVersion = eIpVersion4;
			pTestCong->aDnsAddress[0].addr.v4.ip = 0x08080808;//NETWORK Mode! //10.226.232.10 

		//pTestCong->aDnsAddress[0].ipVersion  = eIpVersion6;
		////abcd:0010:0226:0113::0002
		//pTestCong->aDnsAddress[0].addr.v6.ip[0] = 0xAB; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[1] = 0xCD; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[2] = 0x00; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[3] = 0x10; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[4] = 0x02; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[5] = 0x26; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[6] = 0x01; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[7] = 0x13; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[8] = 0x00; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[9] = 0x00; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[10] =0x00; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[11] =0x00; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[12] =0x00; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[13] =0x00; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[14] =0x00; 
		//pTestCong->aDnsAddress[0].addr.v6.ip[15] =0x02; 

			pTestCong->dwDnsAddressAmount = 1;

			strcpy(pTestCong->aDomainName[0].szDomainName, ""); 
			pTestCong->dwDomainNameAmount++;
			strcpy(pTestCong->aDomainName[1].szDomainName, "com"); 
			pTestCong->dwDomainNameAmount++;

			pTestCong->bIsRequest_A    = TRUE;
			pTestCong->bIsRequest_AAAA = FALSE;

			pTestCong->dwTimeOut_InSec = 5;// in sec.

			pTestCong->wAttempts       = 0;

	}
*/
}
//====================================================================//

//====================================================================//
BOOL  cPLCM_DNS::bActivate (COsQueue*  par_pRcvMbxDns)
{
	BOOL	bRc  = FALSE;

	bRc = this->bActivate();
	this->m_pRcvMbxDns = par_pRcvMbxDns;

	m_state = ANYCASE;

	return bRc;
}
//====================================================================//
//====================================================================//
BOOL  cPLCM_DNS::bActivate()
{
	BOOL	bRc  = FALSE;
	//BOOL	bRc4 = FALSE, bRc6 = FALSE;

	//bRc4 = this->m_DnsSocket4.m_bSosketActivate(eIpVersion4, 0);//PLCM_DNS_CLIENT_PORT);
	//bRc6 = this->m_DnsSosket6.m_bSosketActivate(eIpVersion6, 0);//PLCM_DNS_CLIENT_PORT);

	//if((TRUE == bRc4) || (TRUE == bRc6))
	{
		for(int nIn = 0; nIn < DNS_MAX_SERVICES; nIn++)
		{
			this->m_aConf[nIn].vClean(nIn);
			this->m_aConf[nIn].m_wSerID = nIn; 

			this->m_aConf[nIn].vGetConf(nIn);

			//------For Debug ONLY 
			//this->m_aConf[2].m_sConf.dwDnsAddressAmount = 1;
			//this->m_aConf[2].m_sConf.aDnsAddress[0].ipVersion = eIpVersion4;
			//this->m_aConf[2].m_sConf.aDnsAddress[0].addr.v4.ip = 0x04080808 ;//NETWORK Mode! //8.8.8.4 

			//strncpy(this->m_aConf[2].m_sConf.szSignalingIPv4, "10.227.2.151", sizeof(this->m_aConf[2].m_sConf.szSignalingIPv4));//NETWORK Mode! //8.8.8.4 
			//strncpy(this->m_aConf[2].m_sConf.szSignalingIPv6, "abcd:10:227:2:250:56ff:fe86:23e", sizeof(this->m_aConf[2].m_sConf.szSignalingIPv6));//NETWORK Mode! //8.8.8.4 
            //------For Debug ONLY
		}
		this->m_aConf[0].vDumpConfig("Management/Default");

		this->m_pDnsRecvThr = new cPLCM_DnsUdpRecvThr((void*)this);
		if(NULL != this->m_pDnsRecvThr)
		{
			bRc = this->m_pDnsRecvThr->ActivateDeTach();


			CProcessBase* proc = CProcessBase::GetProcess();
			if(NULL != proc)
			{
				CDNSAgentProcess* pDnsProc = dynamic_cast<CDNSAgentProcess*>(proc);
				if(NULL != pDnsProc)
				{
					this->m_pShManager  = (dynamic_cast<CDNSAgentProcess*>(proc))->GetDnsRecordsMngr();
					if(NULL != this->m_pShManager)
					{
						//this->m_eObjState   = ePLCM_DNS_STATE_ACTIVATED;
						bRc = TRUE;
					}
				}
			}
			//PASSERTMSG_AND_RETURN_VALUE(!proc,"bad process",FALSE);
			//PASSERTMSG_AND_RETURN_VALUE(!pDnsProc,"bad CDNSAgentProcess process",FALSE);
			//PASSERTMSG_AND_RETURN_VALUE(!mngr,"bad DnsRecordsMngr !!!!",FALSE);
		}
	}

	if(TRUE != bRc)
	{
		if(NULL != this->m_pDnsRecvThr)
			delete this->m_pDnsRecvThr;
		this->m_pDnsRecvThr = NULL;

		this->m_eObjState = ePLCM_DNS_STATE_ACTIVE_FAILED;
	}
	return bRc;
}
//====================================================================//
//====================================================================//
cPLCM_DNS::~cPLCM_DNS()
{
	this->m_ReqList.bClean();

	if(NULL != this->m_pTTLService)
		delete this->m_pTTLService;
	this->m_pTTLService = NULL;

	if(NULL != this->m_pDnsRecvThr)
	{
		this->m_pDnsRecvThr->EndSignalClose();
		this->m_pDnsRecvThr->Wait();

		delete this->m_pDnsRecvThr;
		this->m_pDnsRecvThr = NULL;
	}
}
//====================================================================//
//====================================================================//
BOOL  cPLCM_DNS::bSendReq  ()
{
	BOOL bRc = FALSE;

	return bRc;
}
//====================================================================//
//====================================================================//
void cPLCM_DNS::vDebugPrintAddrList(ipAddressStruct * par_pAddr, int par_InLen, char * par_szPrefix)
{
	if((NULL != par_pAddr)&&(0 < (par_InLen)))
	{
		string  strFullLog;
		for(int nInd = 0; nInd < par_InLen; nInd++)
		{
			char szLog[128]="";
			plcmDNS_nGetStrIpFromipAddressStruct(szLog, sizeof(szLog)-1, &par_pAddr[nInd]);
			strFullLog.append((const char*)szLog);
			strFullLog.append((const char*)"\n");
		}

		TRACEINTO << "PLCM_DNS. "<< par_szPrefix <<"DNS-Address(s) ["<< par_InLen << "] :" << strFullLog.c_str();
	}
}
//====================================================================//
//====================================================================//
BOOL  cPLCM_DNS::bGetFromShMem(char * par_pHostName, WORD par_wServiceID
							 , ipAddressStruct * par_aOut, int * par_pInOutLen, BOOL par_bIsUsing)
{
	BOOL bRc = FALSE;
	
	if((NULL != par_aOut)&&(NULL != par_pInOutLen)&&(NULL != this->m_pShManager))
	{
		sDNS_RECORD			aOutDnsRec[PLCM_MAX_RESOLVED_IP];
		DWORD				dwDnsRecAmount = PLCM_MAX_RESOLVED_IP; 
		ESharedMemStatus	dnsRc	       = eSharedMem_StatusOk;

		cDNS_SERVICE_CONFIG *   pServConf = &this->m_aConf[par_wServiceID];

		memset(aOutDnsRec, 0, sizeof(sDNS_RECORD)*PLCM_MAX_RESOLVED_IP);

		DWORD dwIpv4Amount = PLCM_MAX_RESOLVED_IP;

		if(TRUE == pServConf->m_sConf.bIsRequest_A)
		{
			dnsRc =  this->m_pShManager->DnsGetHostByName((const char *)par_pHostName
											 ,(const WORD) par_wServiceID
											 ,(const DWORD) eIpVersion4
											 , dwIpv4Amount
											 , aOutDnsRec, par_bIsUsing);
			if(eSharedMem_StatusOk != dnsRc)
			{
				TRACEINTO << "PLCM_DNS. SH.Mem  GET !!! FAILED (rc:" << dnsRc <<")!!!  HostName[" << par_pHostName << "]"
					<< " | Serv.Id:[" << par_wServiceID <<"]"
					<< " ||  Amount IPv4:" << dwIpv4Amount;

				memset(aOutDnsRec, 0, sizeof(sDNS_RECORD)*PLCM_MAX_RESOLVED_IP);
				dwIpv4Amount = 0;
			}
			else
			{
				TRACEINTO << "PLCM_DNS. SH.Mem  GET:  HostName[" << par_pHostName << "]"
						  << " | Serv.Id:[" << par_wServiceID <<"]"
						  << " ||  Amount IPv4:" << dwIpv4Amount;
			}
		}
		else
			dwIpv4Amount = 0;


		DWORD dwIpv6Amount = PLCM_MAX_RESOLVED_IP - dwIpv4Amount;


		if(TRUE == pServConf->m_sConf.bIsRequest_AAAA)
		{
			dnsRc =  this->m_pShManager->DnsGetHostByName((const char *)par_pHostName
				,(const WORD) par_wServiceID
				,(const DWORD) eIpVersion6
				, dwIpv6Amount
				, &aOutDnsRec[dwIpv4Amount], par_bIsUsing);


			if(eSharedMem_StatusOk != dnsRc)
			{
				TRACEINTO << "PLCM_DNS. SH.Mem  GET(AAAA) !!! FAILED (rc:" << dnsRc <<")!!!  HostName[" << par_pHostName << "]"
					<< " | Serv.Id:[" << par_wServiceID <<"]"
					<< " ||  Amount IPv6:" << dwIpv6Amount;

				memset(&aOutDnsRec[dwIpv4Amount], 0, sizeof(sDNS_RECORD)*(PLCM_MAX_RESOLVED_IP - dwIpv4Amount));
				dwIpv6Amount = 0;
			}
			else
			{
				TRACEINTO << "PLCM_DNS. SH.Mem  GET(AAAA):  HostName[" << par_pHostName << "]"
					<< " | Serv.Id:[" << par_wServiceID <<"]"
					<< " ||  Amount IPv6:" << dwIpv6Amount;
			}
		}
		else
			dwIpv6Amount = 0;

		DWORD dwAllCounter = dwIpv4Amount + dwIpv6Amount;

		dwAllCounter = min(dwAllCounter, (DWORD)(*par_pInOutLen));

		for(DWORD nOutInd = 0; nOutInd < dwAllCounter; nOutInd++)
		{
			memcpy(&par_aOut[nOutInd], &aOutDnsRec[nOutInd].sResolveIp, sizeof(ipAddressStruct));
		}

		*par_pInOutLen = dwAllCounter;
		if(0 < dwAllCounter)
			bRc = TRUE;
		else
			bRc = FALSE;
	}

	return bRc;
}
//====================================================================//
//====================================================================//
BOOL  cPLCM_DNS::bGetDnsAddrByServiceId(WORD par_wServiceID, ipAddressStruct * par_pAddrOut, unsigned int * par_pInOutLen)
{
	BOOL bRc = FALSE;
	if(  (NULL != par_pAddrOut)&&(NULL != par_pInOutLen)&&((*par_pInOutLen) > 0) 
       &&(par_wServiceID < DNS_MAX_SERVICES)
	  )
	{
		unsigned int nInLen = *par_pInOutLen;

        for(DWORD nInd = 0 ; nInd < nInLen; nInd++)
		{
			if(nInd < PLCM_DNS_MAX_ADDITIONAL_IP)
			{
				memcpy(&par_pAddrOut[nInd]
					  , &this->m_aConf[par_wServiceID].m_sConf.aDnsAddress[nInd]
					  , sizeof(ipAddressStruct));
			}
			else
				break;
		}
		*par_pInOutLen = this->m_aConf[par_wServiceID].m_sConf.dwDnsAddressAmount;

		bRc = TRUE;

	}
	return bRc;
}
//====================================================================//
//====================================================================//
void cPLCM_DNS::bGetDomainByServiceId(WORD par_wServiceID, DNS_PLCM_DOMAINNAME * par_pOut, unsigned int * par_pInOutLen)
{
	if(  (NULL != par_pOut) &&(0 < (*par_pInOutLen)) 
	   &&(par_wServiceID < DNS_MAX_SERVICES)	
		)
	{
		unsigned int nInLen = *par_pInOutLen;

		for(DWORD nInd = 0 ; nInd < nInLen; nInd++)
		{
			if(nInd < PLCM_DNS_DOMAINNAME_AMOUNT_MAX)
			{
				strncpy(par_pOut[nInd].szDomainName
					  , this->m_aConf[par_wServiceID].m_sConf.aDomainName[nInd].szDomainName
					  , sizeof(par_pOut[nInd].szDomainName)-1);
			}
			else
				break;
		}
		*par_pInOutLen = this->m_aConf[par_wServiceID].m_sConf.dwDomainNameAmount;
	}
}
//====================================================================//
//====================================================================//
WORD  cPLCM_DNS::bGetDnsAttemptsByServiceId(WORD par_wServiceID)
{
	WORD wRc = 0;

	if(par_wServiceID < DNS_MAX_SERVICES)
		wRc = this->m_aConf[par_wServiceID].m_sConf.wAttempts;		

	return wRc;
}
//====================================================================//
//====================================================================//
void cPLCM_DNS::bGetRequiredAorAAAAByServiceId(WORD par_wServiceID, BOOL * par_pIsA, BOOL * par_pIsAAAA)
{
    if(par_wServiceID < DNS_MAX_SERVICES)
	{
		if(NULL != par_pIsA)
			*par_pIsA = this->m_aConf[par_wServiceID].m_sConf.bIsRequest_A;

		if(NULL != par_pIsAAAA)
			*par_pIsAAAA = this->m_aConf[par_wServiceID].m_sConf.bIsRequest_AAAA;
	}
}
//====================================================================//
//====================================================================//
BOOL cPLCM_DNS::bPrepareRequestNode(char			   *	par_pHostName
								  , WORD					par_wServiceID
								  , eProcessType			par_eProcessType
								  , DWORD					par_dwConnId
								  , DWORD					par_dwPartyId
								  , cDNS_PLCM_REQ_NODE *	par_pOut
								  , DNS_REQ_RES_TYPE		par_eRqType )
{

	BOOL bRc = FALSE;
	if((NULL != par_pHostName)&&(NULL != par_pOut))
	{
		//memset(par_pOut, 0, sizeof(DNS_PLCM_REQ_NODE));
		par_pOut->m_sReqNode.dwDnsAddressAmount = sizeof(par_pOut->m_sReqNode.aDnsAddress)/sizeof(ipAddressStruct);
		////1. Get DNS-addresses list.
		if(TRUE == this->bGetDnsAddrByServiceId(par_wServiceID, par_pOut->m_sReqNode.aDnsAddress, &par_pOut->m_sReqNode.dwDnsAddressAmount))
		{
			////1. Get "attempts" for sending to DNS-Server (FOR UDP only)
			par_pOut->m_sReqNode.wAttempts = this->bGetDnsAttemptsByServiceId(par_wServiceID);

			////2. Get DomainName's list
			par_pOut->m_sReqNode.dwDomainNameAmount = sizeof(par_pOut->m_sReqNode.aDomainName)/sizeof(DNS_PLCM_DOMAINNAME);
			this->bGetDomainByServiceId(par_wServiceID, par_pOut->m_sReqNode.aDomainName, &par_pOut->m_sReqNode.dwDomainNameAmount);
			strncpy(par_pOut->m_sReqNode.szHostName, par_pHostName, sizeof(par_pOut->m_sReqNode.szHostName)-1);

            par_pOut->m_sReqNode.wReqType  = par_eRqType;

			if(eDNS_TYPE_A_IPv4 == par_eRqType)
				par_pOut->m_sReqNode.bIsRequiredIpv4 = TRUE;

			if(eDNS_TYPE_AAAA_IPv6 == par_eRqType)
				par_pOut->m_sReqNode.bIsRequiredIpv6 = TRUE;

			par_pOut->m_sReqNode.wDnsReqID =  G_cDnsReqID.wGetReqId();

			par_pOut->m_sReqNode.wServiceID = par_wServiceID;
			par_pOut->m_sReqNode.nProcessType = par_eProcessType;
			par_pOut->m_sReqNode.pDNSMngrRcvMbx = this->m_pRcvMbxDns;

			par_pOut->m_sReqNode.dwTimeStamp_Request	=  Pm_getCurrentTimestampAdv(E_TIMER_RESOLUTION_MILI, NULL , NULL);
			if(par_wServiceID < DNS_MAX_SERVICES)
				par_pOut->m_sReqNode.dwTimeOut_mSec		    =  this->m_aConf[par_wServiceID].m_sConf.dwTimeOut_InSec * 1000	  ;	
			else
				par_pOut->m_sReqNode.dwTimeOut_mSec	        = 30 * 1000;
		}
	}
	return bRc;
}
//====================================================================//
//====================================================================//
BOOL  cPLCM_DNS::bSendRequestNode (cDNS_PLCM_REQ_NODE * par_pReqNode)
{
	BOOL bRc = FALSE;

	if(  (NULL != par_pReqNode)
	   &&(0 < par_pReqNode->m_sReqNode.dwDnsAddressAmount)
	   &&(par_pReqNode->m_sReqNode.dwDnsAddressAmount > par_pReqNode->m_sReqNode.dwDnsAddress_Current)
	   &&(par_pReqNode->m_sReqNode.dwDomainName_Current < PLCM_DNS_DOMAINNAME_AMOUNT_MAX) 
	  )
	{
		unsigned char Buff[1024 * 32]	= "";
		int			  nQueryLen			= 0;
		char        * szCurrDomainName  = par_pReqNode->m_sReqNode.aDomainName[par_pReqNode->m_sReqNode.dwDomainName_Current].szDomainName;

	
		nQueryLen = plcmMakeDnsQuery( par_pReqNode->m_sReqNode.szHostName
									, szCurrDomainName
			                        , par_pReqNode->m_sReqNode.wReqType //eDNS_TYPE_A_IPv4 | eDNS_TYPE_AAAA_IPv6 |eDNS_TYPE_SRV
					  			    , par_pReqNode->m_sReqNode.wDnsReqID
									, Buff, sizeof(Buff));

		char				szIp[128]= "";
		ipAddressStruct	*	pDnsAdd  = NULL; 

		pDnsAdd = &par_pReqNode->m_sReqNode.aDnsAddress[par_pReqNode->m_sReqNode.dwDnsAddress_Current];
		
		plcmDNS_nGetStrIpFromipAddressStruct(szIp, sizeof(szIp)-1, pDnsAdd);

		par_pReqNode->m_sReqNode.bIsResultIsTimeouted = FALSE;
		par_pReqNode->vTimerAwake(DNS_REQ_TIMER_TOUT);

		//============ SEND to DNS ==============================================//
		WORD wSrvId = par_pReqNode->m_sReqNode.wServiceID;

		if(wSrvId < DNS_MAX_SERVICES)
		{
			if(eIpVersion4 == pDnsAdd->ipVersion)
			{
				//bRc = this->m_DnsSocket4.m_SendTo(szIp , 53, (char*)Buff, nQueryLen);
				bRc = this->m_DnsArrSocket4.m_SockObjArray[wSrvId].m_SendTo(szIp , 53, (char*)Buff, nQueryLen);
			}
			else
			if(eIpVersion6 == pDnsAdd->ipVersion)
			{
				//bRc = this->m_DnsSosket6.m_SendTo(szIp , 53, (char*)Buff, nQueryLen);
				bRc = this->m_DnsArrSocket6.m_SockObjArray[wSrvId].m_SendTo(szIp , 53, (char*)Buff, nQueryLen);
			}
			par_pReqNode->m_sReqNode.eReqResStatus = DNS_REQ_HAS_SENT;
		}
		//=======================================================================//
	
		char szSentHostName[PLCM_DNS_HOST_NAME_SIZE]="";
		if('\0' != szCurrDomainName[0])
			snprintf(szSentHostName, sizeof(szSentHostName)-1,"%s.%s", par_pReqNode->m_sReqNode.szHostName, szCurrDomainName);
		else
			strncpy(szSentHostName, par_pReqNode->m_sReqNode.szHostName, sizeof(szSentHostName)-1);

		TRACEINTO << "PLCM_DNS.[DNS_SEND]. " << "#[" << par_pReqNode->m_sReqNode.wDnsReqID << "]"
			      << " | " << plcmDNS_szGetReqTypeName(par_pReqNode->m_sReqNode.wReqType) 
				  << " | HostName:[" << szSentHostName << "]"
			      << " | to DNS:[" << szIp << "]"
				  << " || " << ((FALSE == bRc)? "FAILED":"SUCCESSFULLY");
	}
	return bRc;
}
//====================================================================//

//====================================================================//
BOOL cPLCM_DNS::bSendRequestSrv( char			*	par_pHostName
								,WORD				par_wServiceID
								,eProcessType		par_eProcessType
								,eIPProtocolType	par_wSignalProtocolType
								,enTransportType	par_wTransportProtocolType)
{
	BOOL bRc = FALSE;

	TRACEINTO << "PLCM_DNS. bSendRequestSrv [Start]. par_pHostName: "
		<< ((NULL != par_pHostName)? par_pHostName:"NULL")
		<< " ServiceID:[" << par_wServiceID <<"]" ;

	if(NULL != par_pHostName)
	{
		////1. Forming cDNS_PLCM_REQ_NODE
		cDNS_PLCM_REQ_NODE	* pRequestNodeSrv   = NULL;

		if(NULL != (pRequestNodeSrv = new cDNS_PLCM_REQ_NODE))
		{
			this->bPrepareRequestNode(par_pHostName, par_wServiceID, par_eProcessType
				, 0, 0, pRequestNodeSrv, eDNS_TYPE_SRV);

			pRequestNodeSrv->m_sReqNode.wPriority			= 0;
			pRequestNodeSrv->m_sReqNode.wWeight				= 0;
			pRequestNodeSrv->m_sReqNode.wPort				= 0;
			pRequestNodeSrv->m_sReqNode.eSignalProtocolType	= par_wSignalProtocolType;
			pRequestNodeSrv->m_sReqNode.eTransportProtocol	= par_wTransportProtocolType; 

			string  OutStringSrv = "PLCM_DNS.  ===>  sRequestNode SRV: \n";
			vReqNode_DebugPrint(pRequestNodeSrv, &OutStringSrv);
			TRACEINTO << OutStringSrv.c_str();
			if(0 < pRequestNodeSrv->m_sReqNode.dwDnsAddressAmount)
			{
				this->m_ReqList.bInsert(pRequestNodeSrv);
				bRc = this->bSendRequestNode(pRequestNodeSrv);
			}
			else
			{
				delete (pRequestNodeSrv);
				bRc = FALSE;
			}
		}
		else
		{
			TRACEINTO << "PLCM_DNS.[DNS_SEND]. SRV " << "Operator NEW - FAILED";
			if(NULL != pRequestNodeSrv)
				delete (pRequestNodeSrv);
			bRc = FALSE;
		}
	}
	return bRc;
}
//====================================================================//
//====================================================================//
BOOL cPLCM_DNS::bSendRequestHostName(char		  * par_pHostName
								   , WORD			par_wServiceID
								   , eProcessType	par_eProcessType
								   , DWORD			par_dwConnId
								   , DWORD			par_dwPartyId  )
{
	BOOL bRc = FALSE;
	
	TRACEINTO << "PLCM_DNS. bSendRequestHostName [Start]. par_pHostName: "
		      << ((NULL != par_pHostName)? par_pHostName:"NULL")
			  << " ServiceID:[" << par_wServiceID <<"]" ;

	if(NULL != par_pHostName)
	{
		////1. Forming cDNS_PLCM_REQ_NODE
		BOOL				  bIsRequiredIpv4 = FALSE;
		BOOL				  bIsRequiredIpv6 = FALSE;
		cDNS_PLCM_REQ_NODE	* pRequestNode4   = NULL;
		cDNS_PLCM_REQ_NODE	* pRequestNode6   = NULL;

        if(  (NULL != (pRequestNode4 = new cDNS_PLCM_REQ_NODE))
           &&(NULL != (pRequestNode6 = new cDNS_PLCM_REQ_NODE)) 
		  )
		{
			////2. Detected(get): A or AAAA or A+AAAA
			this->bGetRequiredAorAAAAByServiceId(par_wServiceID, &bIsRequiredIpv4, &bIsRequiredIpv6);

			if(TRUE == bIsRequiredIpv4)
			{
				//-S-- BRIDGE-18110 -----------------------//
                cDNS_SOCKET				* pSocketObj = &this->m_DnsArrSocket4.m_SockObjArray[par_wServiceID]; 
				cDNS_SERVICE_CONFIG		* pConfObj   = &this->m_aConf[par_wServiceID];

				if((NULL != pSocketObj)&&(NULL != pConfObj))
				{
					if(eDNS_SOCKET_BINDED != pSocketObj->m_eSate)
					{
						this->bReConfigSourceIpForBind_IPv4(par_wServiceID,  pConfObj->m_sConf.szSignalingIPv4);
						this->m_DnsArrSocket4.vSocketArrayDump("PLCM_DNS. [repeat] ReConfigSourceIpForBind IPv4", par_wServiceID, par_wServiceID);
					}
				}
				//-E-- BRIDGE-18110 -----------------------//

				this->bPrepareRequestNode(par_pHostName, par_wServiceID, par_eProcessType
										 , par_dwConnId, par_dwPartyId, pRequestNode4, eDNS_TYPE_A_IPv4);
			}

			if(TRUE == bIsRequiredIpv6)
			{
				//-S-- BRIDGE-18110 -----------------------//
				cDNS_SOCKET				* pSocketObj = &this->m_DnsArrSocket6.m_SockObjArray[par_wServiceID]; 
				cDNS_SERVICE_CONFIG		* pConfObj   = &this->m_aConf[par_wServiceID];
				if((NULL != pSocketObj)&&(NULL != pConfObj))
				{
					if(eDNS_SOCKET_BINDED != pSocketObj->m_eSate)
					{
						this->bReConfigSourceIpForBind_IPv4(par_wServiceID,  pConfObj->m_sConf.szSignalingIPv6);
						this->m_DnsArrSocket6.vSocketArrayDump("PLCM_DNS. [repeat] ReConfigSourceIpForBind IPv6", par_wServiceID, par_wServiceID);
					}
				}
				//-E-- BRIDGE-18110 -----------------------//

				this->bPrepareRequestNode(par_pHostName, par_wServiceID, par_eProcessType
										 , par_dwConnId, par_dwPartyId, pRequestNode6, eDNS_TYPE_AAAA_IPv6);
			}

			// mutual-binding
			if((TRUE == bIsRequiredIpv4)&&(TRUE == bIsRequiredIpv6))
			{
				pRequestNode4->m_sReqNode.wPartnerDnsReqID = pRequestNode6->m_sReqNode.wDnsReqID;
				pRequestNode4->m_sReqNode.bIsRequiredIpv6  = TRUE;

				pRequestNode6->m_sReqNode.wPartnerDnsReqID = pRequestNode4->m_sReqNode.wDnsReqID;
				pRequestNode6->m_sReqNode.bIsRequiredIpv4  = TRUE;
			}

			if(TRUE == bIsRequiredIpv4)
			{
				string  OutString4 = "PLCM_DNS.  ===>  sRequestNode4: \n";
				vReqNode_DebugPrint(pRequestNode4, &OutString4);
				TRACEINTO << OutString4.c_str();
				if(0 < pRequestNode4->m_sReqNode.dwDnsAddressAmount)
				{
					this->m_ReqList.bInsert(pRequestNode4);
					bRc = this->bSendRequestNode(pRequestNode4);
				}
				else
					delete (pRequestNode4);
			}
			else
				delete (pRequestNode4);

			if(TRUE == bIsRequiredIpv6)
			{
				string  OutString6 = "PLCM_DNS.  ===>  sRequestNode6: \n";
				vReqNode_DebugPrint(pRequestNode6, &OutString6);
				TRACEINTO << OutString6.c_str();
				if(0 < pRequestNode6->m_sReqNode.dwDnsAddressAmount)
				{
					this->m_ReqList.bInsert(pRequestNode6);
					bRc = this->bSendRequestNode(pRequestNode6);
				}
				else
					delete (pRequestNode6);
			}
			else
				delete (pRequestNode6);
		}
		else
		{
			TRACEINTO << "PLCM_DNS.[DNS_SEND]. " << "Operator NEW - FAILED";
			if(NULL != pRequestNode4)
				delete (pRequestNode4);
			if(NULL != pRequestNode6)
				delete (pRequestNode4);
			bRc = FALSE;
		}
	}
	return bRc;
}
//====================================================================//
//====================================================================//
BOOL  cPLCM_DNS::bResolveHostName(char *		par_pHostName
								, WORD			par_wServiceID
								, eProcessType	par_eProcessType
								, DWORD			par_dwConnId
								, DWORD			par_dwPartyId
								, BOOL          par_bIsForeResolving
								, COsQueue*		par_pPartyQueue)
{
	BOOL bRc = FALSE;

	if(NULL != par_pHostName)
	{
		ipAddressStruct aIpResolveResult[PLCM_MAX_RESOLVED_IP];
		int				nResolveAmount = PLCM_MAX_RESOLVED_IP ;
		BOOL            bIsUseUpdate = FALSE;
	
		//-S- ----- BRIDGE-16374_1 ---------------------------------//
		memset(aIpResolveResult, 0, sizeof(ipAddressStruct)* PLCM_MAX_RESOLVED_IP);
		
		if('\0' == par_pHostName[0])
		{
			TRACEINTO << "PLCM_DNS. DNS_RESOLVE_DOMAIN_REQ. HostHane:[" << par_pHostName << "] is <EMPTY>!!!";
			nResolveAmount = 0;
			plcmDNS_SendResolveResult(par_wServiceID, par_pHostName, par_eProcessType
				, par_pPartyQueue, 0, aIpResolveResult , nResolveAmount);
			bRc = TRUE;
			return bRc;
		}
		//-E- ----- BRIDGE-16374_1 ---------------------------------//

		this->m_DnsArrSocket4.vArrOpenAndBinndByService(par_wServiceID); 
		this->m_DnsArrSocket6.vArrOpenAndBinndByService(par_wServiceID); 

		if(0xFFFFFFFF == par_dwConnId)
			bIsUseUpdate = TRUE;
		else
        if(eProcessTypeInvalid == par_eProcessType)  
			bIsUseUpdate = FALSE;
		else
			bIsUseUpdate = TRUE;

		if(TRUE == bIsItIpAddress(par_pHostName, NULL))
		{
			//memset(aIpResolveResult, 0, sizeof(ipAddressStruct)* PLCM_MAX_RESOLVED_IP);// BLOCKED //-S- ----- BRIDGE-16374_1 --
			stringToIp(&aIpResolveResult[0], par_pHostName, eNetwork);
			nResolveAmount = 1;

			if(eProcessTypeInvalid != par_eProcessType)
			{   //-S-- BRIDGE-18260 --------------------------------------------------------------------//
				plcmDNS_SendResolveResult(par_wServiceID, par_pHostName, par_eProcessType
					, par_pPartyQueue, STATUS_OK, aIpResolveResult , nResolveAmount);
			}  //-E-- BRIDGE-18260 --------------------------------------------------------------------//
			bRc = TRUE;
		}
		else
		{
			if(FALSE == this->bGetFromShMem(par_pHostName, par_wServiceID, aIpResolveResult, &nResolveAmount, bIsUseUpdate))
			{
				this->bSendRequestHostName(par_pHostName, par_wServiceID, par_eProcessType,par_dwConnId, par_dwPartyId);
				bRc = TRUE;
			}
			else
			{
				TRACEINTO << "PLCM_DNS. DNS_RESOLVE_DOMAIN_REQ. nResolved Amount: [" << nResolveAmount << "]";

				if(eProcessTypeInvalid != par_eProcessType)
				{
					plcmDNS_SendResolveResult(par_wServiceID, par_pHostName, par_eProcessType
										   , par_pPartyQueue, 0, aIpResolveResult , nResolveAmount);
				}
				bRc = TRUE;
			}
		}
	}
	return bRc;
}
//====================================================================//
//====================================================================//
BOOL  cPLCM_DNS::bBuildNewReqFromCurr(cDNS_PLCM_REQ_NODE * par_CurrNode, BOOL par_bIsResponceArrived)
{
	BOOL bRc = FALSE;

	if(NULL != par_CurrNode)
	{
		cDNS_PLCM_REQ_NODE * pPartnerNode = this->m_ReqList.Find(par_CurrNode->m_sReqNode.wPartnerDnsReqID);

		if(  (TRUE == par_bIsResponceArrived)
		   &&(0 < par_CurrNode->m_sReqNode.dwDomainNameAmount)
		   &&(par_CurrNode->m_sReqNode.dwDomainName_Current < ((par_CurrNode->m_sReqNode.dwDomainNameAmount)-1) )
		  )
		{// next DomainName
			par_CurrNode->m_sReqNode.dwDomainName_Current++;

			unsigned int dwDomNameInd = par_CurrNode->m_sReqNode.dwDomainName_Current;
			if(   (dwDomNameInd < PLCM_DNS_DOMAINNAME_AMOUNT_MAX)
			    &&('\0' != par_CurrNode->m_sReqNode.aDomainName[dwDomNameInd].szDomainName[0])
			  )
			{
				if(NULL != pPartnerNode)
					par_CurrNode->m_sReqNode.wPartnerDnsReqID = pPartnerNode->m_sReqNode.wDnsReqID;
				else
					par_CurrNode->m_sReqNode.wPartnerDnsReqID = 0;
				par_CurrNode->m_sReqNode.wDnsReqID = G_cDnsReqID.wGetReqId();
				this->m_ReqList.vUpdatePartnerId(par_CurrNode->m_sReqNode.wPartnerDnsReqID, par_CurrNode->m_sReqNode.wDnsReqID);

				bRc = TRUE;
			}
		}
		else
		{// Next DNS-address
			
			if(  (1 < par_CurrNode->m_sReqNode.dwDnsAddressAmount)
               &&(par_CurrNode->m_sReqNode.dwDnsAddress_Current < ((par_CurrNode->m_sReqNode.dwDnsAddressAmount)-1)  )
			  )
			{
				if(FALSE == par_bIsResponceArrived)
				{
					if(0 < par_CurrNode->m_sReqNode.wAttempts)
					{
						if((par_CurrNode->m_sReqNode.wAttempts -1) > (par_CurrNode->m_sReqNode.wAttempt_Current))
						{
							par_CurrNode->m_sReqNode.wAttempt_Current++;
						}
						else
						{
							par_CurrNode->m_sReqNode.wAttempt_Current = 0;
							par_CurrNode->m_sReqNode.dwDnsAddress_Current++;
							par_CurrNode->m_sReqNode.dwDomainName_Current = 0;
						}
					}
					else
					{
						par_CurrNode->m_sReqNode.dwDnsAddress_Current++;
						par_CurrNode->m_sReqNode.dwDomainName_Current = 0;
						par_CurrNode->m_sReqNode.wAttempt_Current     = 0;
					}
				}
				else
				{
					par_CurrNode->m_sReqNode.dwDnsAddress_Current++;
				    par_CurrNode->m_sReqNode.dwDomainName_Current = 0;
				}

				if(NULL != pPartnerNode)
					par_CurrNode->m_sReqNode.wPartnerDnsReqID = pPartnerNode->m_sReqNode.wDnsReqID;
				else
					par_CurrNode->m_sReqNode.wPartnerDnsReqID = 0;

				par_CurrNode->m_sReqNode.wDnsReqID = G_cDnsReqID.wGetReqId();
				this->m_ReqList.vUpdatePartnerId(par_CurrNode->m_sReqNode.wPartnerDnsReqID, par_CurrNode->m_sReqNode.wDnsReqID);

				bRc = TRUE;
			}
		}
	}
	return bRc;
}
//====================================================================//

//====================================================================//
unsigned int cPLCM_DNS::dwGetFullResolvedArray(DNS_IPADDR_TTL    *  par_aFullResolved
											 , unsigned int		    par_ArrLen
											 , cDNS_PLCM_REQ_NODE * par_pNode
											 , cDNS_PLCM_REQ_NODE * par_pNodePartner)
{
	unsigned int dwRc = 0;

	if(NULL != par_aFullResolved)
	{
		memset(par_aFullResolved, 0, sizeof(DNS_IPADDR_TTL) * par_ArrLen);

		cDNS_PLCM_REQ_NODE * pNode4 = NULL;
		cDNS_PLCM_REQ_NODE * pNode6 = NULL;
		unsigned int        dwResolvedAmount4 = 0;
		unsigned int        dwResolvedAmount6 = 0;

		if((NULL != par_pNode)&&(eDNS_TYPE_A_IPv4 == par_pNode->m_sReqNode.wReqType))
			pNode4 = par_pNode;
		if((NULL != par_pNodePartner)&&(eDNS_TYPE_A_IPv4 == par_pNodePartner->m_sReqNode.wReqType))
			pNode4 = par_pNodePartner;

		if((NULL != par_pNode)&&(eDNS_TYPE_AAAA_IPv6 == par_pNode->m_sReqNode.wReqType))
			pNode6 = par_pNode;
		if((NULL != par_pNodePartner)&&(eDNS_TYPE_AAAA_IPv6 == par_pNodePartner->m_sReqNode.wReqType))
			pNode6 = par_pNodePartner;


		{
			char * pHostName_Log     = NULL;
			char * pDonaminName_Log  = NULL;
			cDNS_PLCM_REQ_NODE * pTraceNode = NULL;
			if(NULL != pNode4) pTraceNode = pNode4;
			else if(NULL != pNode6) pTraceNode = pNode6;

			if((NULL != pTraceNode)&&(pTraceNode->m_sReqNode.dwDomainName_Current < PLCM_DNS_DOMAINNAME_AMOUNT_MAX))
			{
				pHostName_Log = pTraceNode->m_sReqNode.szHostName;
				pDonaminName_Log = pTraceNode->m_sReqNode.aDomainName[pTraceNode->m_sReqNode.dwDomainName_Current].szDomainName;
			}
		}

		DWORD nIn = 0;

		if(NULL != pNode4)
		{
			dwResolvedAmount4 = min(pNode4->m_sReqNode.dwResolvedAddrAmount, (unsigned int)PLCM_HOST_IPV4_AMOUNT);
			if(0 < dwResolvedAmount4)
			{
				for(nIn = 0; (nIn < dwResolvedAmount4)&&(nIn < par_ArrLen); nIn++)
				{
					memcpy(&par_aFullResolved[nIn], &pNode4->m_sReqNode.aResoledAddressAndTTL[nIn], sizeof(DNS_IPADDR_TTL));  
				}
			}
		}

		if(NULL != pNode6)
		{
			dwResolvedAmount6 = min(pNode6->m_sReqNode.dwResolvedAddrAmount, (unsigned int)PLCM_HOST_IPV6_AMOUNT);

			if(0 < dwResolvedAmount6)
			{
				DWORD nV6 = 0;

				for(nIn = dwResolvedAmount4; (nV6 < dwResolvedAmount6)&&(nIn < (par_ArrLen-dwResolvedAmount4)); nIn++)
				{
					memcpy(&par_aFullResolved[nIn], &pNode6->m_sReqNode.aResoledAddressAndTTL[nV6], sizeof(DNS_IPADDR_TTL));  
					nV6++;
				}
			}
		}

		dwRc = nIn;//dwResolvedAmount4 + dwResolvedAmount6;

	}
	return dwRc;
}
//====================================================================//
//====================================================================//
DWORD dwGeTTLMin(DNS_IPADDR_TTL * par_aFullAddrTTLResolved, unsigned int par_dwFullResolvedAmount)
{
	DWORD dwRc = 0;
	if((NULL != par_aFullAddrTTLResolved)&&(0 < par_dwFullResolvedAmount))
	{
		dwRc = par_aFullAddrTTLResolved[0].dwTTL;

		for(unsigned int nInd = 0; nInd < par_dwFullResolvedAmount; nInd++)
		{
			if(dwRc > par_aFullAddrTTLResolved[nInd].dwTTL)
				dwRc = par_aFullAddrTTLResolved[nInd].dwTTL;
		}
	}

	return dwRc;
}
//====================================================================//
//====================================================================//
BOOL  cPLCM_DNS::bAnalysisAndProcessing(  DNS_IPADDR_TTL  * par_aIpaddTTL, int par_nArrLenResolved
							            , char			  * par_szHostNameComplex, unsigned short par_wReqId
							            , unsigned short	par_ReqType // 
										, BOOL				par_bIsResponceArrived
							            )
{
	BOOL bRc = FALSE;
	//1. Find into ReqList
	cDNS_PLCM_REQ_NODE * pNode = NULL;

	if(NULL != (pNode = this->m_ReqList.Find(par_wReqId)) )
	{
		char					szHostNameReal[PLCM_DNS_HOST_NAME_SIZE]="";//---Outbound Proxy ---//
		char				*	pSzDomainName = NULL;
		cDNS_PLCM_REQ_NODE	*	pNodePartner  = NULL;
		eDNS_REQ_RES_STATUS		ePartnerStaus = DNS_REQ_RESP_STAUS_IDLE;	

		memset(szHostNameReal, '\0', sizeof(szHostNameReal));//---Outbound Proxy ---//

		if(TRUE == par_bIsResponceArrived)
		{
			pNode->vTimerStop(DNS_REQ_TIMER_TOUT, "STOP_1");
			pNode->m_sReqNode.eReqResStatus = DNS_RES_HAS_RECV;
		}
		else
			pNode->m_sReqNode.eReqResStatus = DNS_RES_HAS_TIMEOUTED;

		if(NULL != (pNodePartner = this->m_ReqList.Find(pNode->m_sReqNode.wPartnerDnsReqID)) )
			ePartnerStaus = pNodePartner->m_sReqNode.eReqResStatus;
//---Outbound Proxy ---//
		if(pNode->m_sReqNode.dwDomainName_Current < PLCM_DNS_DOMAINNAME_AMOUNT_MAX)
		{
			if('\0' != pNode->m_sReqNode.aDomainName[pNode->m_sReqNode.dwDomainName_Current].szDomainName[0])
			{
				pSzDomainName = pNode->m_sReqNode.aDomainName[pNode->m_sReqNode.dwDomainName_Current].szDomainName;

				vGetHostNameReal(szHostNameReal, sizeof(szHostNameReal)-1, pNode->m_sReqNode.szHostName
							   , pNode->m_sReqNode.aDomainName[pNode->m_sReqNode.dwDomainName_Current].szDomainName);
			}
			else
				strncpy(szHostNameReal, pNode->m_sReqNode.szHostName, sizeof(szHostNameReal)-1);
		}
//---Outbound Proxy ---//
		if(0 == par_nArrLenResolved)
		{
			if(DNS_REQ_HAS_SENT == ePartnerStaus)
			{//Wait PARTNER";
				pNode->m_sReqNode.eReqResStatus	= DNS_RESP_FINISH_RESOLVED;
				return bRc = TRUE;
			}


	    //======All response(s) for current domain/DNS-ip - has arrived ======================//
			if(  (  (0 == pNode->m_sReqNode.dwResolvedAddrAmount)
                  &&( (NULL != pNodePartner)&&(0 == pNodePartner->m_sReqNode.dwResolvedAddrAmount)) 
			     )
               ||(  (0 == pNode->m_sReqNode.dwResolvedAddrAmount)
			      &&(NULL == pNodePartner)  
				  )
			  ) 
			{
				BOOL				bRcNewReq        = this->bBuildNewReqFromCurr(pNode, par_bIsResponceArrived);
				BOOL				bRcNewPartnerReq =  FALSE;

				//--------------------------- bBuildNewReqFromCurr --------------------------------------//
				if(NULL != pNodePartner)
				{
					bRcNewPartnerReq = this->bBuildNewReqFromCurr(pNodePartner, par_bIsResponceArrived);
				}
				//---------------------------------------------------------------------------------------//

				if(NULL != pNode)
				{
					if(TRUE == bRcNewReq)
					{
						pNode->m_sReqNode.eReqResStatus = DNS_REQ_HAS_SENT;
						this->bSendRequestNode(pNode);

						string  OutStringNew = "PLCM_DNS.  ===>  New (Next) \n";
						vReqNode_DebugPrint(pNode, &OutStringNew);
						TRACEINTO << OutStringNew.c_str();
					}
					else
						pNode->m_sReqNode.eReqResStatus = DNS_REQ_RESP_STAUS_IDLE;
				}


				if(NULL != pNodePartner)
				{
					if(TRUE == bRcNewPartnerReq)	
					{
						pNodePartner->m_sReqNode.eReqResStatus = DNS_REQ_HAS_SENT;
						this->bSendRequestNode(pNodePartner);
						ePartnerStaus = DNS_REQ_HAS_SENT;

						string  OutStringNew = "PLCM_DNS.  ===>  New (Next) \n";
						vReqNode_DebugPrint(pNodePartner, &OutStringNew);
						TRACEINTO << OutStringNew.c_str();
					}
					else
					{
						pNodePartner->m_sReqNode.eReqResStatus = DNS_REQ_RESP_STAUS_IDLE;
						ePartnerStaus = DNS_REQ_RESP_STAUS_IDLE;
					}
					ePartnerStaus = pNodePartner->m_sReqNode.eReqResStatus;
				}
			}
			else
			{
				pNode->m_sReqNode.eReqResStatus  = DNS_RESP_FINISH_RESOLVED;
				if(NULL != pNodePartner)
				{
					pNodePartner->m_sReqNode.eReqResStatus = DNS_RESP_FINISH_RESOLVED;
					ePartnerStaus = DNS_RESP_FINISH_RESOLVED;
				}
			}

			bRc = TRUE;
		}
		else
		{
			unsigned int dwRes = min(PLCM_MAX_RESOLVED_IP, par_nArrLenResolved);
			memcpy(&pNode->m_sReqNode.aResoledAddressAndTTL[0], par_aIpaddTTL, dwRes * sizeof(DNS_IPADDR_TTL));
			pNode->m_sReqNode.dwResolvedAddrAmount = dwRes;

			pNode->m_sReqNode.eReqResStatus  = DNS_RESP_FINISH_RESOLVED;

			bRc = TRUE;
		}

		//==============================================//

		if(   (  (  (DNS_REQ_RESP_STAUS_IDLE == pNode->m_sReqNode.eReqResStatus)
			      ||(DNS_RESP_FINISH_RESOLVED == pNode->m_sReqNode.eReqResStatus)
				 )
		       &&((DNS_REQ_RESP_STAUS_IDLE == ePartnerStaus)||(DNS_RESP_FINISH_RESOLVED == ePartnerStaus))
		      )
		  )
		{
				string  cSrtingLog;
				DNS_IPADDR_TTL  aFullAddrTTLResolved[PLCM_MAX_RESOLVED_IP];
				unsigned int dwFullResolvedAmount = 0; 

				memset(aFullAddrTTLResolved, 0, sizeof(DNS_IPADDR_TTL)* PLCM_MAX_RESOLVED_IP);
				
				if((eDNS_TYPE_A_IPv4 == pNode->m_sReqNode.wReqType)||(eDNS_TYPE_AAAA_IPv6 == pNode->m_sReqNode.wReqType))
					 dwFullResolvedAmount = this->dwGetFullResolvedArray(aFullAddrTTLResolved, PLCM_MAX_RESOLVED_IP,pNode, pNodePartner);
				else
                if(eDNS_TYPE_SRV == pNode->m_sReqNode.wReqType)
				{
					for(DWORD dwIn = 0; dwIn < pNode->m_sReqNode.dwResolvedAddrAmount; dwIn++)
					{
						aFullAddrTTLResolved[dwIn].dwTTL   = pNode->m_sReqNode.aResoledAddressAndTTL[dwIn].dwTTL;
						aFullAddrTTLResolved[dwIn].sIpAddr = pNode->m_sReqNode.aResoledAddressAndTTL[dwIn].sIpAddr;
					}
					dwFullResolvedAmount = pNode->m_sReqNode.dwResolvedAddrAmount;
				}


				char    szLog[512]="";
                DWORD   dwResolvedFromPartner = 0;

				if(NULL != pNodePartner)
					dwResolvedFromPartner = pNodePartner->m_sReqNode.dwResolvedAddrAmount;

				if(  (pNode->m_sReqNode.dwDomainName_Current < PLCM_DNS_DOMAINNAME_AMOUNT_MAX)
				   &&(0 < (pNode->m_sReqNode.dwResolvedAddrAmount + dwResolvedFromPartner))
				  )
				{
					snprintf(szLog, sizeof(szLog)-1, "PLCM_DNS. Resolving RESULT. Service[%d] | Host[.Domain]: %s%s%s%s | Resolved:[%d] address(s):\n     "
						, pNode->m_sReqNode.wServiceID
						, pNode->m_sReqNode.szHostName
						, ('\0' != pNode->m_sReqNode.aDomainName[pNode->m_sReqNode.dwDomainName_Current].szDomainName[0])? "[.":""
						, ('\0' != pNode->m_sReqNode.aDomainName[pNode->m_sReqNode.dwDomainName_Current].szDomainName[0])? 
												pNode->m_sReqNode.aDomainName[pNode->m_sReqNode.dwDomainName_Current].szDomainName:""
						, ('\0' != pNode->m_sReqNode.aDomainName[pNode->m_sReqNode.dwDomainName_Current].szDomainName[0])? "]":""
						,    pNode->m_sReqNode.dwResolvedAddrAmount + dwResolvedFromPartner 
						);  
				}
				else
				{
					snprintf(szLog, sizeof(szLog)-1, "PLCM_DNS. Resolving RESULT. Service[%d] | Host: %s | Resolved:[%d] address(s):\n     "
						, pNode->m_sReqNode.wServiceID
						, pNode->m_sReqNode.szHostName
						,    pNode->m_sReqNode.dwResolvedAddrAmount + dwResolvedFromPartner 
						);  
				}

				cSrtingLog.append((const char*)szLog);
				vDebugPrintResolveAddrList_String(aFullAddrTTLResolved, dwFullResolvedAmount, &cSrtingLog);
				TRACEINTO << cSrtingLog.c_str();
				
				//----- Insert to TTL-SERVICE -----------------------------------------------//
				if((eDNS_TYPE_A_IPv4 == pNode->m_sReqNode.wReqType)||(eDNS_TYPE_AAAA_IPv6 == pNode->m_sReqNode.wReqType))
				{
					if(NULL == this->m_pTTLService)
						this->m_pTTLService = new cTTLService();
					if(NULL != this->m_pTTLService)
					{
						//this->m_pTTLService->bDelete(pNode->m_sReqNode.szHostName, pNode->m_sReqNode.wServiceID);
						this->m_pTTLService->bDelete(pNode->m_sReqNode.szHostName, pNode->m_sReqNode.wServiceID);

						if(0 < dwFullResolvedAmount)
						{
							DWORD dwMinTTL = dwGeTTLMin(aFullAddrTTLResolved, dwFullResolvedAmount);
							//this->m_pTTLService->bInsert(pNode->m_sReqNode.szHostName, pNode->m_sReqNode.wServiceID
							this->m_pTTLService->bInsert(pNode->m_sReqNode.szHostName, pNode->m_sReqNode.wServiceID
								                                                    , dwMinTTL, pNode->m_sReqNode.wReqType);
						}
						else
							//this->m_pTTLService->bInsert(pNode->m_sReqNode.szHostName, pNode->m_sReqNode.wServiceID
							this->m_pTTLService->bInsert(pNode->m_sReqNode.szHostName, pNode->m_sReqNode.wServiceID
							                                     , PLCM_DNS_SERVICE_DEFAULT_TIME_SEC, pNode->m_sReqNode.wReqType);
					}
				}
                //---------------------------------------------------------------------------//
            
				//2. Delete & Save to Sh.Mem
				bRc = this->bUpdateDataIntoShMem(pNode, aFullAddrTTLResolved, min(dwFullResolvedAmount, (unsigned int)PLCM_MAX_RESOLVED_IP));

				//3. Send to Requester 
				ipAddressStruct  aResolvedAddr[PLCM_MAX_RESOLVED_IP];
				memset(aResolvedAddr, 0, sizeof(ipAddressStruct) * PLCM_MAX_RESOLVED_IP);
				for(unsigned int nInd= 0; nInd < dwFullResolvedAmount; nInd++)
					memcpy(&aResolvedAddr[nInd], &aFullAddrTTLResolved[nInd].sIpAddr, sizeof(ipAddressStruct));


				if(eProcessTypeInvalid != pNode->m_sReqNode.nProcessType)
				{//excluding from TTL-Service's
					if((eDNS_TYPE_A_IPv4 == pNode->m_sReqNode.wReqType)||(eDNS_TYPE_AAAA_IPv6 == pNode->m_sReqNode.wReqType))
					{
						plcmDNS_SendResolveResult(pNode->m_sReqNode.wServiceID
												, szHostNameReal
												, pNode->m_sReqNode.nProcessType
												, pNode->m_sReqNode.pDNSMngrRcvMbx
												, (0 < dwFullResolvedAmount)? STATUS_OK : -1 //0:OK; !=0:ERROR CODE
												, aResolvedAddr
												, dwFullResolvedAmount
												);
					}


					if(eDNS_TYPE_SRV == pNode->m_sReqNode.wReqType)
					{
						plcmDNS_SendResolveResultSRV(
							  pNode->m_sReqNode.wServiceID
							, szHostNameReal
							, pNode->m_sReqNode.nProcessType
							, pNode->m_sReqNode.pDNSMngrRcvMbx
							, (0 < dwFullResolvedAmount)? STATUS_OK : -1 //0:OK; !=0:ERROR CODE
							, aResolvedAddr
							, dwFullResolvedAmount
							, aFullAddrTTLResolved[0].dwTTL
							, pNode->m_sReqNode.wPort
							, pNode->m_sReqNode.wPriority
							, pNode->m_sReqNode.wWeight);
					}
				}
				else
				{
					string RString_ResIND;

					char szTxt[1024]="";
					snprintf(szTxt, sizeof(szTxt)-1, "PLCM_DNS. TTL-SERVICE | HostName:[%s] | ServiceId:[%d] |\n"
						, szHostNameReal
						, pNode->m_sReqNode.wServiceID);
					szTxt[sizeof(szTxt)-1]='\0';

					RString_ResIND.append((const char*)szTxt);

					vDebugPrintAddrList_String(aResolvedAddr, dwFullResolvedAmount, &RString_ResIND);
					FTRACEINTO << RString_ResIND.c_str();//---Outbound Proxy ---//
				}

				if(NULL != pNode)
				{
					//pNode->vTimerStop(DNS_REQ_TIMER_TOUT, "STOP_2");
					this->m_ReqList.bDelete(pNode->m_sReqNode.wDnsReqID, "");
				}

				if(NULL != pNodePartner)
				{
					//pNodePartner->vTimerStop(DNS_REQ_TIMER_TOUT, "STOP_3");
					this->m_ReqList.bDelete(pNodePartner->m_sReqNode.wDnsReqID, "");
				}

                pNode        = NULL;
				pNodePartner = NULL;
		}
	}
	else
		TRACEINTO << "   PLCM_DNS.[DNS_RECV]. cPLCM_DnsUdpRecvThr::Svc. Req.ID:[" << par_wReqId << "]"
                  << " | HostName:[" << par_szHostNameComplex <<"] !!! NOT found in m_pDnsReqList";  

	return bRc;
}
//====================================================================//
//====================================================================//
unsigned int cPLCM_DNS::dwGetUsingTimeStamp (char * par_pHostName, WORD par_wServiceID)
{
	unsigned int dwRc = 0xFFFFFFFF;

	if(NULL != par_pHostName)
	{
		ESharedMemStatus		eShrStatus = eSharedMem_StatusOk;

		sDNS_RECORD				aOutBuff[PLCM_MAX_RESOLVED_IP];
		DWORD					dwOutLen = PLCM_MAX_RESOLVED_IP;

        eShrStatus = this->m_pShManager->DnsGetHostByName((const char *)par_pHostName, par_wServiceID, enIpVersionMAX
			                                   , dwOutLen, aOutBuff, FALSE);
		if((eSharedMem_StatusOk == eShrStatus)&&(0 < dwOutLen))
		{
			dwRc = aOutBuff[0].dwLastUseTimeStamp;
		}
	}
	return dwRc;
}
//====================================================================//
//====================================================================//
BOOL  cPLCM_DNS::bUpdateDataIntoShMem(cDNS_PLCM_REQ_NODE * par_pNodeReq
									, DNS_IPADDR_TTL     * par_aFullAddrTTLResolved
									, unsigned int         par_dwFullResolvedAmount)
{
	BOOL bRc = TRUE;

	if((NULL != par_pNodeReq)&&(NULL != par_aFullAddrTTLResolved)&&(NULL != this->m_pShManager))
	{
		sDNS_RECORD				sDnsRecord;
		DNS_REQ_NODE		*	pReq = &par_pNodeReq->m_sReqNode;	
		ESharedMemStatus		eShrStatus = eSharedMem_StatusOk;

		memset(&sDnsRecord, 0, sizeof(sDNS_RECORD));

		TRACEINTO << "PLCM_DNS. SH.Mem  UPDATE:  HostName[" << pReq->szHostName << "]"
			      << " | Serv.Id:[" << pReq->wServiceID <<"]";

		unsigned int nPrevUsingTimeStamp = 0xFFFFFFF;

		nPrevUsingTimeStamp = this->dwGetUsingTimeStamp((char *)pReq->szHostName, pReq->wServiceID);

		eShrStatus = this->m_pShManager->DnsDeleteHostByName((const char *)pReq->szHostName, (const WORD)pReq->wServiceID);
		if(eSharedMem_StatusOk != eShrStatus)
			TRACEINTO << "PLCM_DNS. SH.Mem  DELETE:  HostName[" << pReq->szHostName << "]"
			<< " | Serv.Id:[" << pReq->wServiceID <<"] FAILED !!! Error:" << eShrStatus ;  

		if(0 < strlen(pReq->aDomainName[pReq->dwDomainName_Current].szDomainName))
		{
			char szFullHostName[PLCM_DNS_HOST_NAME_SIZE]="";
			snprintf(szFullHostName, sizeof(szFullHostName)-1, "%s.%s", pReq->szHostName,pReq->aDomainName[pReq->dwDomainName_Current].szDomainName);
			eShrStatus = this->m_pShManager->DnsDeleteHostByName((const char *)szFullHostName, (const WORD)pReq->wServiceID);
			if(eSharedMem_StatusOk != eShrStatus)
				TRACEINTO << "PLCM_DNS. SH.Mem  DELETE_D:  HostName[" << szFullHostName << "]"
				<< " | Serv.Id:[" << pReq->wServiceID <<"] FAILED !!! Error:" << eShrStatus ; 
		}

		strncpy(sDnsRecord.szHostName, pReq->szHostName, sizeof(sDnsRecord.szHostName)-1);
		sDnsRecord.szHostName[sizeof(sDnsRecord.szHostName)-1] = '\0';
		if((0 < par_dwFullResolvedAmount)&&(pReq->dwDomainName_Current < PLCM_DNS_DOMAINNAME_AMOUNT_MAX))
			strncpy(sDnsRecord.szDomainName, pReq->aDomainName[pReq->dwDomainName_Current].szDomainName, sizeof(sDnsRecord.szDomainName)-1);

		sDnsRecord.dwTimeStamp_Responce	= Pm_getCurrentTimestamp(E_TIMER_RESOLUTION_SEC);
		memcpy(&sDnsRecord.sDnsAddress, &pReq->aDnsAddress[pReq->dwDnsAddress_Current], sizeof(ipAddressStruct)); 	         			
		sDnsRecord.wServiceId = pReq->wServiceID;	

		if(0 < par_dwFullResolvedAmount)
		{
			for(unsigned int nInd = 0; nInd < par_dwFullResolvedAmount; nInd++)
			{
				memset(&sDnsRecord.sResolveIp, 0, sizeof(ipAddressStruct));
				memcpy(&sDnsRecord.sResolveIp, &par_aFullAddrTTLResolved[nInd].sIpAddr, sizeof(ipAddressStruct));				
				sDnsRecord.dwTTL = par_aFullAddrTTLResolved[nInd].dwTTL;
				sDnsRecord.dwTimeStamp_NextReq = sDnsRecord.dwTimeStamp_Responce + sDnsRecord.dwTTL;		
                sDnsRecord.eStatus	  = eDnsStatusResolved;

				if(eProcessTypeInvalid != par_pNodeReq->m_sReqNode.nProcessType)
					sDnsRecord.dwLastUseTimeStamp = Pm_getCurrentTimestamp(E_TIMER_RESOLUTION_SEC);
				else if(0xFFFFFFFF != nPrevUsingTimeStamp)
					sDnsRecord.dwLastUseTimeStamp = nPrevUsingTimeStamp;  
                
				eShrStatus = this->m_pShManager->DnsInsertHostByName(sDnsRecord);
				if(eSharedMem_StatusOk != eShrStatus)
					TRACEINTO << "PLCM_DNS. SH.Mem  INSERT: HostName[" << pReq->szHostName << "]"
					          << " | Serv.Id:[" << pReq->wServiceID <<"] FAILED !!! Error:" << eShrStatus ;  

				if(  (pReq->dwDomainName_Current < PLCM_DNS_DOMAINNAME_AMOUNT_MAX)
				   &&(0 < strlen(pReq->aDomainName[pReq->dwDomainName_Current].szDomainName))
				  )
				{
					sDNS_RECORD				sDnsRecordDom;

					memcpy(&sDnsRecordDom, &sDnsRecord, sizeof(sDNS_RECORD));
					snprintf(&sDnsRecordDom.szHostName[0], sizeof(sDnsRecordDom.szHostName)-1, "%s.%s"
						         , pReq->szHostName, pReq->aDomainName[pReq->dwDomainName_Current].szDomainName);

                    sDnsRecordDom.szHostName[sizeof(sDnsRecordDom.szHostName)-1]='\0'; 

					memset(sDnsRecordDom.szDomainName, '\0', sizeof(sDnsRecordDom.szDomainName));
					
					eShrStatus = this->m_pShManager->DnsInsertHostByName(sDnsRecordDom);
					if(eSharedMem_StatusOk != eShrStatus)
						TRACEINTO << "PLCM_DNS. SH.Mem  INSERT_D:  HostName[" << sDnsRecordDom.szHostName << "]"
						<< " | Serv.Id:[" << sDnsRecordDom.wServiceId <<"] FAILED !!! Error:" << eShrStatus ;  
				}
			}
		}
		else
		{
			sDnsRecord.eStatus	  = eDnsStatusNotResolved;
			if(TRUE == par_pNodeReq->m_sReqNode.bIsResultIsTimeouted)
				sDnsRecord.eStatus	  = eDnsStatusTimeOut;	

			sDnsRecord.dwTTL      = PLCM_DNS_SERVICE_DEFAULT_TIME_SEC;
			sDnsRecord.dwTimeStamp_NextReq = sDnsRecord.dwTimeStamp_Responce + sDnsRecord.dwTTL;
			eShrStatus = this->m_pShManager->DnsInsertHostByName(sDnsRecord);
			if(eSharedMem_StatusOk != eShrStatus)
				TRACEINTO << "   PLCM_DNS. SH.Mem  INSERT:  HostName[" << pReq->szHostName << "]"
				<< " | Serv.Id:[" << pReq->wServiceID <<"] FAILED !!! Error:" << eShrStatus ;  
		}


		{
			std::ostringstream answer;
			this->m_pShManager->DumpRecords(answer);
			TRACEINTO << answer.str();
		}
	}
	return bRc;
}
//====================================================================//
//====================================================================//
BOOL  cPLCM_DNS::bReConfigSourceIpForBind_IPv4(WORD serviceID, char * par_szSoureceIp)
{
	BOOL bRc = FALSE;
	if(DNS_MAX_SERVICES > serviceID)
	{//IP source for BIND IPv4
		
		cDNS_SERVICE_CONFIG  * pcConf = &this->m_aConf[serviceID];
		cDNS_SOCKET * pDnsSocket = &(this->m_DnsArrSocket4.m_SockObjArray[serviceID]);

		{//-S- ----- BRIDGE-16374 ---------------------------------//
			if((NULL != par_szSoureceIp)&&(0 < strlen(par_szSoureceIp))) 
			{
				strncpy(pcConf->m_sConf.szSignalingIPv4, par_szSoureceIp, sizeof(pcConf->m_sConf.szSignalingIPv4)-1);//Klocwork.Sasha #ID.3987
                pcConf->m_sConf.szSignalingIPv4[sizeof(pcConf->m_sConf.szSignalingIPv4)-1] = '\0';
            }
			else
			{
				memset(pcConf->m_sConf.szSignalingIPv4, '\0', sizeof(pcConf->m_sConf.szSignalingIPv4));
			}

			pcConf->m_sConf.sBindPorts.wPort_v4 = G_arrBindPorts[serviceID].wPort_v4;

			this->m_eObjState = ePLCM_DNS_STATE_IDLE;
   plcmSleepMs(150);
			plCloseUdpSocket(this->m_DnsArrSocket4.m_arSocketsSelected[serviceID]);
   plcmSleepMs(150);

			this->m_DnsArrSocket4.m_arSocketsSelected[serviceID] = INVALID_SOCKET;
			
			pDnsSocket->m_Socket = INVALID_SOCKET;
			pDnsSocket->m_eSate  = eDNS_SOCKET_CLOSED;
			pDnsSocket->m_eSate  = eDNS_SOCKET_UNDEF;

			bRc = pDnsSocket->m_bSosketActivateA(eIpVersion4
				                          , pcConf->m_sConf.sBindPorts.wPort_v4
										  , par_szSoureceIp);  

			this->m_DnsArrSocket4.m_arSocketsSelected[serviceID] = pDnsSocket->m_Socket;

			this->m_eObjState = ePLCM_DNS_STATE_ACTIVATED;
            plcmSleepMs(150);
		}//-E- ----- BRIDGE-16374 ---------------------------------//
    }

	return bRc;
}
//====================================================================//

//====================================================================//
BOOL  cPLCM_DNS::bReConfigSourceIpForBind_IPv6(WORD serviceID, char * par_szSoureceIp)
{
	BOOL bRc = FALSE;
	if(DNS_MAX_SERVICES > serviceID)
	{//IP source for BIND IPv4
		char * pSourceIP = (char*)(par_szSoureceIp);
		cDNS_SERVICE_CONFIG  * pcConf = &this->m_aConf[serviceID];

		if(0 < strlen(pSourceIP)) {
			strncpy(pcConf->m_sConf.szSignalingIPv6, pSourceIP, sizeof(pcConf->m_sConf.szSignalingIPv6)-1);//Klocwork.Sasha #ID.3986
            pcConf->m_sConf.szSignalingIPv6[sizeof(pcConf->m_sConf.szSignalingIPv6)-1] = '\0';
        }else{
			memset(pcConf->m_sConf.szSignalingIPv6, '\0', sizeof(pcConf->m_sConf.szSignalingIPv6));
	    }
		
//-S- ----- BRIDGE-16374 ---------------------------------//
        pcConf->m_sConf.sBindPorts.wPort_v6 = G_arrBindPorts[serviceID].wPort_v6;

		this->m_eObjState = ePLCM_DNS_STATE_IDLE;

   plcmSleepMs(150);
		plCloseUdpSocket(this->m_DnsArrSocket6.m_arSocketsSelected[serviceID]);
   plcmSleepMs(150);
		this->m_DnsArrSocket6.m_arSocketsSelected[serviceID] = INVALID_SOCKET;
		cDNS_SOCKET * pDnsCosket = &(this->m_DnsArrSocket6.m_SockObjArray[serviceID]);
        
        pDnsCosket->m_Socket = INVALID_SOCKET;
		pDnsCosket->m_eSate  = eDNS_SOCKET_CLOSED;
		pDnsCosket->m_eSate  = eDNS_SOCKET_UNDEF;
		
		bRc = pDnsCosket->m_bSosketActivateA(eIpVersion6
			                                , pcConf->m_sConf.sBindPorts.wPort_v6
											, pSourceIP);  
//-E- ----- BRIDGE-16374 ---------------------------------//
		this->m_DnsArrSocket6.m_arSocketsSelected[serviceID] = pDnsCosket->m_Socket;
        
        plcmSleepMs(150);
		this->m_eObjState = ePLCM_DNS_STATE_ACTIVATED;
	}

	return bRc;
}
//====================================================================//

//====================================================================//
BOOL  cPLCM_DNS::bReConfigSourceIpForBind(WORD serviceID, enIpVersion par_eIpV, const char * par_szSoureceIp)
{
	BOOL bRc = FALSE;
	if(DNS_MAX_SERVICES > serviceID)
	{
		cDNS_SERVICE_CONFIG * pcConfig = &this->m_aConf[serviceID];

		if(NULL != pcConfig)
		{//-S-- Klocwork.Sasha #ID.3997
			char		   * szConfSourceIp  = pcConfig->m_sConf.szSignalingIPv4;
            SOCKET		   * pSelectedSocket = &this->m_DnsArrSocket4.m_arSocketsSelected[serviceID];	
			cDNS_SOCKET    * pDnsCosket      = &(this->m_DnsArrSocket4.m_SockObjArray[serviceID]);
//-S- ----- BRIDGE-16374 ---------------------------------//			
			unsigned short   wPort           = pcConfig->m_sConf.sBindPorts.wPort_v4;

			if(eIpVersion6 == par_eIpV)
			{
				szConfSourceIp = pcConfig->m_sConf.szSignalingIPv6;
				if(NULL != pSelectedSocket)
				    pSelectedSocket = &this->m_DnsArrSocket6.m_arSocketsSelected[serviceID];
				if(NULL != pDnsCosket)
				    pDnsCosket = &(this->m_DnsArrSocket6.m_SockObjArray[serviceID]);

				wPort = pcConfig->m_sConf.sBindPorts.wPort_v6;
			}

			if(NULL == par_szSoureceIp)
				memset(szConfSourceIp, '\0', PLCM_STRING_SIZE_256);
			else
				strncpy(szConfSourceIp, par_szSoureceIp , PLCM_STRING_SIZE_256 -1);

			this->m_eObjState = ePLCM_DNS_STATE_IDLE;
            plCloseUdpSocket(*pSelectedSocket);
			*pSelectedSocket = INVALID_SOCKET;
              
		    pDnsCosket->m_Socket = INVALID_SOCKET;
			pDnsCosket->m_eSate  = eDNS_SOCKET_CLOSED;
			pDnsCosket->m_eSate  = eDNS_SOCKET_UNDEF;
			 
    	    pDnsCosket->m_bSosketActivateA(par_eIpV
				                           , wPort
										   , (char*)par_szSoureceIp);  
           	*pSelectedSocket = pDnsCosket->m_Socket;
			this->m_eObjState = ePLCM_DNS_STATE_ACTIVATED;
//-E- ----- BRIDGE-16374 ---------------------------------//			
		}//-E-- Klocwork.Sasha #ID.3997
	}
	return bRc;
}
//====================================================================//
//====================================================================//
void   cPLCM_DNS::vDumpConf(int par_nFrom,int par_nUp, std::ostream& os)
{
	if((0 <= par_nFrom)&&(DNS_MAX_SERVICES > par_nUp))
	{//-S- ----- BRIDGE-16374 ---------------------------------//
		CPrettyTable<WORD, const char *, DWORD, const char *, DWORD , const char *, const char*, const char*, DWORD, WORD> tbl("S.ID", "Sour.IP(Port)", "DNS#", "DNS server(s)","Dm#","Domain(s)","4","6", "T", "A");

		for(int n = par_nFrom; n <= par_nUp; n++)
		{
			char    szIp[IPV6_ADDRESS_LEN]="";

			string  cSrtingDns;
			vDebugPrintAddrList_StringL(this->m_aConf[n].m_sConf.aDnsAddress,  this->m_aConf[n].m_sConf.dwDnsAddressAmount, &cSrtingDns);

			string  sStringDomain;
			vDebugPrintDomainName_StringL(this->m_aConf[n].m_sConf.aDomainName, this->m_aConf[n].m_sConf.dwDomainNameAmount, &sStringDomain);

			string  sStringSourceIP;
			sStringSourceIP.append((const char*)this->m_aConf[n].m_sConf.szSignalingIPv4); 

			if(0 != strlen(this->m_aConf[n].m_sConf.szSignalingIPv4))
			{
				char szPort[32]="";
				snprintf(szPort,sizeof(szPort)-1,"(%d)", this->m_aConf[n].m_sConf.sBindPorts.wPort_v4);
				sStringSourceIP.append(szPort);
			}

			if(0 != strlen(this->m_aConf[n].m_sConf.szSignalingIPv6))
			{
				sStringSourceIP.append(" ");
				sStringSourceIP.append((const char*)this->m_aConf[n].m_sConf.szSignalingIPv6);  
				if(0 < strlen(sStringSourceIP.c_str()))
				{
					char szPort[32]="";
					snprintf(szPort,sizeof(szPort)-1,"(%d)", this->m_aConf[n].m_sConf.sBindPorts.wPort_v4);
					sStringSourceIP.append(szPort);
				}
			}
            
			tbl.Add(
				this->m_aConf[n].m_wSerID
				, sStringSourceIP.c_str()
				, this->m_aConf[n].m_sConf.dwDnsAddressAmount
				, cSrtingDns.c_str()
				, this->m_aConf[n].m_sConf.dwDomainNameAmount
				, sStringDomain.c_str()
				, (TRUE == this->m_aConf[n].m_sConf.bIsRequest_A)? "Y":"N" 
				, (TRUE == this->m_aConf[n].m_sConf.bIsRequest_AAAA)? "Y":"N"
				, this->m_aConf[n].m_sConf.dwTimeOut_InSec
				, this->m_aConf[n].m_sConf.wAttempts );
		}

		os << tbl.Get();
	}//-E- ----- BRIDGE-16374 ---------------------------------//
	else
	{
		char szError[64]="";
		snprintf(szError, sizeof(szError)-1,"Ser.ID NOT in range!!! (from:%d | Up:%d)", par_nFrom, par_nUp);

		os << szError;  
	}

	//DUMP of SOCKETS Arrays
	{//-S- ----- BRIDGE-16374 ---------------------------------//
		CPrettyTable<int,   int, const char *, WORD,     const char *,    int, const char *, WORD> tblS("S.ID", "S.IPv4", "Status", "Port", "||", "S.IPv6", "Status", "Port");

		for(int nS = 0; nS < DNS_MAX_SERVICES; nS++)
		{
			tblS.Add(
				  nS
				, this->m_DnsArrSocket4.m_SockObjArray[nS].m_Socket
				, (const char*)this->m_DnsArrSocket4.m_SockObjArray[nS].m_szGetStrSocketState()
				, (eDNS_SOCKET_BINDED == this->m_DnsArrSocket4.m_SockObjArray[nS].m_eSate)? this->m_aConf[nS].m_sConf.sBindPorts.wPort_v4:0
				, "||"
				, this->m_DnsArrSocket6.m_SockObjArray[nS].m_Socket
				, (const char*)this->m_DnsArrSocket6.m_SockObjArray[nS].m_szGetStrSocketState()
				, (eDNS_SOCKET_BINDED == this->m_DnsArrSocket6.m_SockObjArray[nS].m_eSate)? this->m_aConf[nS].m_sConf.sBindPorts.wPort_v6:0
				);
		}

		os << tblS.Get();
	}//-E- ----- BRIDGE-16374 ---------------------------------//
}
//====================================================================//


//====================================================================//
cDNS_SOCKET_ARRAY::cDNS_SOCKET_ARRAY()
{
	this->m_IpV = enIpVersionMAX;
	this->m_pvPLCM_DNS = NULL;
	this->m_IsSocketArrActived = FALSE;

	memset(this->m_arSocketsSelected, INVALID_SOCKET, sizeof(SOCKET)*DNS_MAX_SERVICES);
	memset(this->m_arEventInfo, 0, sizeof(EVENT_INFO)*DNS_MAX_SERVICES);

	for(int nS = 0;nS < DNS_MAX_SERVICES; nS++)
	{
		this->m_arEventInfo[nS].pDnsSocket    = &this->m_SockObjArray[nS];

		this->m_SockObjArray[nS].m_IpV        = this->m_IpV;
		this->m_SockObjArray[nS].m_Socket     = INVALID_SOCKET;
		this->m_SockObjArray[nS].m_szErrMsg[0]= '\0';
		this->m_SockObjArray[nS].m_dwSelectReadTimeoutMillSec = 10;
		this->m_SockObjArray[nS].m_eSate      = eDNS_SOCKET_UNDEF ;
	}

};
//====================================================================//
//====================================================================//
cDNS_SOCKET_ARRAY::~cDNS_SOCKET_ARRAY()
{
	m_IsSocketArrActived = FALSE;
};
//====================================================================//
//====================================================================//
void cDNS_SOCKET_ARRAY::vArrSocketClose()
{

	for(int nS = 0;nS < DNS_MAX_SERVICES; nS++)
	{
		if((INVALID_SOCKET != this->m_arSocketsSelected[nS])&&(0 < this->m_arSocketsSelected[nS]))
		{
			plCloseUdpSocket(this->m_arSocketsSelected[nS]);
			this->m_arSocketsSelected[nS] = INVALID_SOCKET;
		}
		this->m_arEventInfo[nS].pDnsSocket    = &this->m_SockObjArray[nS];

		this->m_SockObjArray[nS].m_IpV        = eIpVersion4;
		this->m_SockObjArray[nS].m_Socket     = INVALID_SOCKET;
		this->m_SockObjArray[nS].m_szErrMsg[0]= '\0';
		this->m_SockObjArray[nS].m_dwSelectReadTimeoutMillSec = 10;
		this->m_SockObjArray[nS].m_eSate      = eDNS_SOCKET_UNDEF ;
	}

	memset(this->m_arSocketsSelected, INVALID_SOCKET, sizeof(SOCKET)*DNS_MAX_SERVICES);
	memset(this->m_arEventInfo, 0, sizeof(EVENT_INFO)*DNS_MAX_SERVICES);

	this->m_IsSocketArrActived = FALSE;
}
//====================================================================//
//====================================================================//
void cDNS_SOCKET_ARRAY::vArrSocketInit(enIpVersion par_IPv, void * par_pPlcmDns)
{
	this->m_IpV = par_IPv;
	this->m_pvPLCM_DNS = par_pPlcmDns;

	memset(this->m_arSocketsSelected, INVALID_SOCKET, sizeof(SOCKET)*DNS_MAX_SERVICES);
	memset(this->m_arEventInfo, 0, sizeof(EVENT_INFO)*DNS_MAX_SERVICES);

	for(int nS = 0;nS < DNS_MAX_SERVICES; nS++)
	{
		this->m_arEventInfo[nS].pDnsSocket    = &this->m_SockObjArray[nS];

		this->m_SockObjArray[nS].m_IpV        = this->m_IpV;
		this->m_SockObjArray[nS].m_Socket     = INVALID_SOCKET;
		this->m_SockObjArray[nS].m_szErrMsg[0]= '\0';
		this->m_SockObjArray[nS].m_dwSelectReadTimeoutMillSec = 10;
		this->m_SockObjArray[nS].m_eSate      = eDNS_SOCKET_UNDEF ;
	}
	TRACEINTO <<"PLCM_DNS. vArrOpenAndBinnd for IPv["<<this->m_IpV<<"]";

	this->vArrOpenAndBinnd();

	this->m_IsSocketArrActived = TRUE;
};
//====================================================================//
//====================================================================//
void cDNS_SOCKET_ARRAY::vArrOpenAndBinndByService(WORD par_wServID)
{
	//-S-- Klocwork #ID.3995 & ID.4001
	if(NULL != this->m_pvPLCM_DNS)
	{
		cPLCM_DNS * pPlcmDns = (cPLCM_DNS*)this->m_pvPLCM_DNS;

		if((par_wServID < DNS_MAX_SERVICES)&&(NULL != pPlcmDns))
		{
			cDNS_SOCKET	* pDnsSocket = &this->m_SockObjArray[par_wServID];
			if( (eDNS_SOCKET_UNDEF == pDnsSocket->m_eSate) || 
			    (eDNS_SOCKET_CLOSED == pDnsSocket->m_eSate) )
			{//-S- ----- BRIDGE-16374 ---------------------------------//
				if(eIpVersion4 == this->m_IpV)
					this->m_SockObjArray[par_wServID].m_bSosketActivateA(this->m_IpV
					, pPlcmDns->m_aConf[par_wServID].m_sConf.sBindPorts.wPort_v4
					, pPlcmDns->m_aConf[par_wServID].m_sConf.szSignalingIPv4);

				else
					if(eIpVersion6 == this->m_IpV)
						this->m_SockObjArray[par_wServID].m_bSosketActivateA(this->m_IpV
						, pPlcmDns->m_aConf[par_wServID].m_sConf.sBindPorts.wPort_v6
						, pPlcmDns->m_aConf[par_wServID].m_sConf.szSignalingIPv6);

				this->m_arSocketsSelected[par_wServID] = this->m_SockObjArray[par_wServID].m_Socket;


				TRACEINTO <<"PLCM_DNS. [1] vArrOpenAndBinndByService ["<< ((eIpVersion4 == this->m_IpV)? "IPv4":"IPv6")<<"]"
					<<"  State : "<< pDnsSocket->m_eSate;

			}//-E- ----- BRIDGE-16374 ---------------------------------//
			else
				TRACEINTO <<"PLCM_DNS. [2] vArrOpenAndBinndByService ["<< ((eIpVersion4 == this->m_IpV)? "IPv4":"IPv6") << "]"
				<<"  State : "<< ((NULL != pDnsSocket)? pDnsSocket->m_eSate : eDNS_SOCKET_UNDEF);
		}
		//-E-- Klocwork #ID.3995 & ID.4001
	}
}
//====================================================================//
//====================================================================//
void cDNS_SOCKET_ARRAY::vArrOpenAndBinnd()
{
    cPLCM_DNS * pPlcmDns = (cPLCM_DNS*)this->m_pvPLCM_DNS; 

	if(NULL != pPlcmDns)
	{
		for(int nS =0; nS < /*DNS_MAX_SERVICES*/1; nS++)
		{//-S- ----- BRIDGE-16374 ---------------------------------//
			if(eIpVersion4 == this->m_IpV)
				this->m_SockObjArray[nS].m_bSosketActivateA(this->m_IpV
				                         , pPlcmDns->m_aConf[nS].m_sConf.sBindPorts.wPort_v4
										 , pPlcmDns->m_aConf[nS].m_sConf.szSignalingIPv4);
			else
			if(eIpVersion6 == this->m_IpV)
				this->m_SockObjArray[nS].m_bSosketActivateA(this->m_IpV
				                         , pPlcmDns->m_aConf[nS].m_sConf.sBindPorts.wPort_v6
										 , pPlcmDns->m_aConf[nS].m_sConf.szSignalingIPv6);

			this->m_arSocketsSelected[nS] = this->m_SockObjArray[nS].m_Socket;
		}//-E- ----- BRIDGE-16374 ---------------------------------//

		this->vSocketArrayDump("PLCM_DNS. SOCKET's vArrOpenAndBinnd", 0, DNS_MAX_SERVICES);
	}
	else
		TRACEINTO <<"PLCM_DNS. cDNS_SOCKET_ARRAY::vArrOpenAndBinnd - FAILED. m_pvPLCM_DNS=NULL!!";
};
//====================================================================//
//====================================================================//
void cDNS_SOCKET_ARRAY::vSocketArrayDump(char * par_zsPrefix, int par_nCSFrom, int par_nCSUpto)
{
	string cSrtingLog;
    char   szTxt[1024]="";
	snprintf(szTxt, sizeof(szTxt)-1, "%s . Socket-Arr. IPv%d\n", par_zsPrefix,(eIpVersion4 == this->m_IpV)? 4:6);
	cSrtingLog.append((const char*)szTxt);

    cPLCM_DNS * pPlcmDns = (cPLCM_DNS*)m_pvPLCM_DNS;

	if((par_nCSFrom < 0)|| (par_nCSFrom >=DNS_MAX_SERVICES) )//Klocwork.Sasha #ID.3990 & ID.4000
		return;

	if((par_nCSUpto < 0)|| (par_nCSUpto >= DNS_MAX_SERVICES) || (par_nCSUpto < par_nCSFrom))//Klocwork.Sasha #ID.3990 & ID.4000
		return;

	if(par_nCSFrom == par_nCSUpto)
		par_nCSUpto += 1;

	if(NULL != pPlcmDns)
	{
		for(int nS = par_nCSFrom; ((nS < par_nCSUpto)&&(nS<DNS_MAX_SERVICES)); nS++)//Klocwork.Sasha #ID.3990 & ID.4000
		{//-S- ----- BRIDGE-16374 ---------------------------------//
			char			*	pConfSourceIp = pPlcmDns->m_aConf[nS].m_sConf.szSignalingIPv4;
			unsigned short		wPort =  pPlcmDns->m_aConf[nS].m_sConf.sBindPorts.wPort_v4;

			if(eIpVersion6 == this->m_IpV)
			{
				pConfSourceIp = pPlcmDns->m_aConf[nS].m_sConf.szSignalingIPv6;
				wPort =  pPlcmDns->m_aConf[nS].m_sConf.sBindPorts.wPort_v6;
			}
			snprintf(szTxt, sizeof(szTxt)-1, "%d | %5d | %s | %s (%d)\n"
				   , nS
				   , m_arSocketsSelected[nS]
				   , m_SockObjArray[nS].m_szGetStrSocketState()
				   //, m_szGetSelectEvent(this->m_arEventInfo[nS].eSelectEvent) 
				   , ((NULL == pConfSourceIp)||(0 == strlen(pConfSourceIp)) )? "ANY_ADDR": pConfSourceIp
				   , wPort
				   );
			cSrtingLog.append((const char*)szTxt); 
		}//-E- ----- BRIDGE-16374 ---------------------------------//
		TRACEINTO << cSrtingLog.c_str();
	}
}
//====================================================================//








