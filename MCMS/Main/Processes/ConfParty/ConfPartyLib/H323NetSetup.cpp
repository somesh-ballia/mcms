//+========================================================================+
//                            H323NetS.cpp                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323NetSetup.cpp                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Uri                                                         |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 4/12/95     |                                                     |
//+========================================================================+
// H323NetS.cpp : implementation of net utility classes:
//
//                                      CNetCallingParty
//                                      CNetCalledParty
//                                      CNetCause
//                                      CNetProgress
//                                      CNetOpenDchnl
//                                      CnetStatusAlarm
//                                      CnetStatusDchnl
//

#include <stdlib.h>
#include "H323NetSetup.h"
#include "Segment.h"
#include "Macros.h"
#include "IPUtils.h"
#include "SystemFunctions.h"
#include "IpServiceListManager.h"
#include "ConfPartyGlobals.h"


using namespace std;


#define	MinimumBandwidth	64
#define MaxPortSize			6

extern CIpServiceListManager* GetIpServiceListMngr();


////////////////////////////////////////////////////////////////////////////
//                       CH323NetSetup functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CH323NetSetup::CH323NetSetup()          // constructor
{
	m_pH323PointerToAlias	  = NULL;
	memset(m_bestH323PartyAlias, 0, ALIAS_NAME_LEN+1);
	m_h323PartyAlias[0]		  = '\0';
	m_h323PartyAliasType	  = 0;
	m_sH323userUser[0]        = '\0';
	m_userUserSize			  = 0;

	m_conferenceGoal = 0;	
	m_connectionType = 0;	
	m_bIsActiveMc    = 0;
	m_bIsOrigin		 = 0;
	m_bH245Establish = 0; 

	m_localH225Port			  = 0;

	memset(m_conferenceId, 0,MaxConferenceIdSize);
	memset(m_callId, 0,MaxConferenceIdSize);
	m_callReferenceValue	  = 0;

	m_generator	= 0;
	m_hkLen		= 0;
	m_hkType	= 0;
	m_pHalfKey	= NULL;
	// IpV6
	memset(m_srcH323PartyAliases,'\0',MaxAddressListSize);
	memset(m_destH323PartyAliases,'\0',MaxAddressListSize);

	memset(&m_h245Address, 0, sizeof(m_h245Address));
}

/////////////////////////////////////////////////////////////////////////////
CH323NetSetup::~CH323NetSetup()     // destructor
{	
	PDELETEA(m_pHalfKey);
}


/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////  
void CH323NetSetup::Serialize(WORD format,CSegment& seg) 
{ 

	switch ( format ) 
	{    
	case NATIVE: 
		{
			CIpNetSetup::Serialize(format,seg);
			
			DWORD aliasLen = strlen((char*)m_bestH323PartyAlias);
			seg << aliasLen;
			seg.Put((BYTE *)m_bestH323PartyAlias,aliasLen);
			
			aliasLen = strlen((char*)m_h323PartyAlias);
			seg << aliasLen;
			seg.Put((BYTE *)m_h323PartyAlias,aliasLen);
			
			seg << m_h323PartyAliasType;
			// IpV6
			aliasLen = strlen((char*)m_srcH323PartyAliases);
			seg << aliasLen;
			seg.Put((BYTE *)m_srcH323PartyAliases,aliasLen);
			aliasLen = strlen((char*)m_destH323PartyAliases);
			seg << aliasLen;
			seg.Put((BYTE *)m_destH323PartyAliases,aliasLen);
			
			seg << m_userUserSize;
			seg << m_conferenceGoal;
			seg << m_connectionType;
//			seg << m_callModel;
			seg << m_bIsActiveMc;
			seg << m_bIsOrigin;
			seg << m_bH245Establish;
			
			seg.Put((BYTE *)&m_h245Address, sizeof(mcTransportAddress));
			
			aliasLen = strlen(m_sH323userUser);
			seg << aliasLen;
			seg.Put((BYTE *)m_sH323userUser, aliasLen);
			
			//to connect with the remote EP in DahliaIn
			aliasLen = MaxConferenceIdSize;
			seg << aliasLen;
			seg.Put((BYTE *)m_conferenceId, aliasLen);
			
			aliasLen = MaxConferenceIdSize;
			seg << aliasLen;
			seg.Put((BYTE *)m_callId, aliasLen);
			
			seg << m_callReferenceValue;
			seg << m_localH225Port;
			
			seg << m_generator
				<< m_hkLen
				<< m_hkType;
			
			if(m_hkLen)
				seg.Put((BYTE *)m_pHalfKey,m_hkLen);

			break;        
		}																		
		
	   default: 
		   break;                                   
				 
		   
	}       //End switch                                                                     
}

/////////////////////////////////////////////////////////////////////////////
void CH323NetSetup::DeSerialize(WORD format,CSegment& seg)     
{    
	switch ( format ) 
	{    
	case NATIVE: 
		{
			CIpNetSetup::DeSerialize(format,seg);
			
			DWORD aliasLen;
			
			seg >> aliasLen;
			seg.Get((BYTE *)m_bestH323PartyAlias,aliasLen);
			m_bestH323PartyAlias[aliasLen] = 0;
						
			seg >> aliasLen;
			seg.Get((BYTE *)m_h323PartyAlias,aliasLen);
			m_h323PartyAlias[aliasLen] = 0;
			
			seg >> m_h323PartyAliasType;
			// IpV6
			seg >> aliasLen;
			seg.Get((BYTE *)m_srcH323PartyAliases,aliasLen);
			m_srcH323PartyAliases[aliasLen] = 0;
			seg >> aliasLen;
			seg.Get((BYTE *)m_destH323PartyAliases,aliasLen);
			m_destH323PartyAliases[aliasLen] = 0;
			
			
			seg >> m_userUserSize;
			seg >> m_conferenceGoal;
			seg >> m_connectionType;
//			seg >> m_callModel;
			seg >> m_bIsActiveMc;
			seg >> m_bIsOrigin;
			seg >> m_bH245Establish;
			
			seg.Get((BYTE *)&m_h245Address, sizeof(mcTransportAddress));
			
			DWORD strlen1;
			seg >> strlen1;
			seg.Get((BYTE *)m_sH323userUser, strlen1);
			m_sH323userUser[strlen1] = 0;
			
			//to connect with the remote EP in DahliaIn
			seg >> strlen1;
			seg.Get((BYTE *)m_conferenceId, strlen1);
			
			seg >> strlen1;
			seg.Get((BYTE *)m_callId, strlen1);
			
			seg >> m_callReferenceValue;
			seg >> m_localH225Port;	
			
			seg >> m_generator;
			seg	>> m_hkLen;
			seg	>> m_hkType;
			
			if(m_hkLen)
			{
				PDELETEA(m_pHalfKey);
				m_pHalfKey = new BYTE[m_hkLen];
				seg.Get((BYTE *)m_pHalfKey,m_hkLen);
			}

			break;
		}
		
	   default : 
		   break;
		   
	}       //End switch                                                                     
	
}

/////////////////////////////////////////////////////////////////////////////
void CH323NetSetup::SetVariabels(const mcIndCallOffering& pCallOfferingInd)
{
	// IpV6
	SetTaSrcPartyAddr(&(pCallOfferingInd.srcIpAddress.transAddr));
	SetTaDestPartyAddr(&(pCallOfferingInd.destIpAddress.transAddr));
	SetIpVersion((enIpVersion)pCallOfferingInd.srcIpAddress.transAddr.ipVersion);
	SetSrcPartyAliases(pCallOfferingInd.srcPartyAliases);
	SetDestPartyAliases(pCallOfferingInd.destPartyAliases);


	 strncpy(m_sH323userUser,
			 pCallOfferingInd.userUser,
			 MaxUserUserSize);
	 m_sH323userUser[MaxUserUserSize - 1] = '\0';
	 memcpy(&m_h245Address,
			 &pCallOfferingInd.h245IpAddress.transAddr,
			 sizeof(mcTransportAddress));

	 SetMaxRate(pCallOfferingInd.rate / 2);
	 SetMinRate(MinimumBandwidth * 1000);//write as kiloBit
	 SetEndpointType(pCallOfferingInd.srcEndpointType);
//	 PASSERT(1);//SetCallIndex(pCallOfferingInd.header.pmHeader.callIndex);
	 m_userUserSize		= pCallOfferingInd.userUserSize;
	 m_conferenceGoal	= pCallOfferingInd.conferenceGoal;
	 m_connectionType	= pCallOfferingInd.type;
//	 m_callModel		= pCallOfferingInd.model;
	 m_bIsActiveMc		= pCallOfferingInd.bIsActiveMc;
	 m_bIsOrigin		= pCallOfferingInd.bIsOrigin;
	 m_bH245Establish	= pCallOfferingInd.bH245Establish;
//	 PASSERT(1);//SetConnectionId(pCallOfferingInd.header.serialHeader.connectionId);
	 m_localH225Port	= pCallOfferingInd.localH225Port;
	 
	 memcpy(m_conferenceId, pCallOfferingInd.conferenceId, MaxConferenceIdSize);
	 memcpy(m_callId, pCallOfferingInd.callId, MaxConferenceIdSize);
	 m_callReferenceValue = pCallOfferingInd.referenceValue;

	//---encryption section
	//there is at least one token - for now we support only in one token
	if(pCallOfferingInd.encryTokens.numberOfTokens > 0) 
	{
		m_generator	= ((encryptionToken *)pCallOfferingInd.encryTokens.token)->generator;
		m_hkType	= ((encryptionToken *)pCallOfferingInd.encryTokens.token)->tokenOID;
		m_hkLen		= ((encryptionToken *)pCallOfferingInd.encryTokens.token)->hkLen;
		if(m_hkLen)
		{
			m_pHalfKey	= new BYTE[m_hkLen]; //I will delete it in partyApi.cpp
			memcpy((char *)m_pHalfKey,&(((encryptionToken *)pCallOfferingInd.encryTokens.token)->halfKey),m_hkLen);
		}
	}

}


/////////////////////////////////////////////////////////////////////////////
void CH323NetSetup::copy(const CNetSetup * rhs)
{
  *this = *((CH323NetSetup *)rhs);
}

/////////////////////////////////////////////////////////////////////////////
CIpNetSetup& CH323NetSetup::operator=(const CIpNetSetup& other)
{
	if (this != &other)
	{
		CIpNetSetup::operator=(other);

		if (strcmp(other.NameOf() ,"CH323NetSetup") == 0)
		{
			CH323NetSetup & rOtherH323NetSetup = (CH323NetSetup &)other;
			
			strncpy(m_bestH323PartyAlias, rOtherH323NetSetup.m_bestH323PartyAlias, ALIAS_NAME_LEN);
			m_bestH323PartyAlias[ALIAS_NAME_LEN] = '\0';
			
			strncpy(m_h323PartyAlias,rOtherH323NetSetup.m_h323PartyAlias,IP_STRING_LEN - 1);
			m_h323PartyAlias[IP_STRING_LEN - 1] = '\0';
			m_h323PartyAliasType = rOtherH323NetSetup.m_h323PartyAliasType;
                         
                        			// IpV6
			strncpy(m_srcH323PartyAliases,rOtherH323NetSetup.m_srcH323PartyAliases,MaxAddressListSize - 1);
			m_srcH323PartyAliases[MaxAddressListSize - 1] = '\0';
			strncpy(m_destH323PartyAliases,rOtherH323NetSetup.m_destH323PartyAliases,MaxAddressListSize - 1);
			m_destH323PartyAliases[MaxAddressListSize - 1] = '\0';
						
			

			m_userUserSize	 = rOtherH323NetSetup.m_userUserSize;
			m_conferenceGoal = rOtherH323NetSetup.m_conferenceGoal;
			m_connectionType = rOtherH323NetSetup.m_connectionType;
//			m_callModel		 = rOtherH323NetSetup.m_callModel;
			m_bIsActiveMc	 = rOtherH323NetSetup.m_bIsActiveMc;
			m_bIsOrigin		 = rOtherH323NetSetup.m_bIsOrigin;
			m_bH245Establish = rOtherH323NetSetup.m_bH245Establish;
			
			memcpy(&m_h245Address, &rOtherH323NetSetup.m_h245Address, sizeof(mcTransportAddress));
			strncpy(m_sH323userUser, rOtherH323NetSetup.m_sH323userUser, MaxUserUserSize);
			m_sH323userUser[MaxUserUserSize - 1] = '\0';
			
			//to connect with the remote EP in DahliaIn
			memcpy(m_conferenceId, rOtherH323NetSetup.m_conferenceId, MaxConferenceIdSize);
			memcpy(m_callId, rOtherH323NetSetup.m_callId, MaxConferenceIdSize);
			m_callReferenceValue = rOtherH323NetSetup.m_callReferenceValue;
			
			m_localH225Port	  = rOtherH323NetSetup.m_localH225Port;

			m_generator	= rOtherH323NetSetup.m_generator;
			m_hkType	= rOtherH323NetSetup.m_hkType;
			m_hkLen		= rOtherH323NetSetup.m_hkLen;

			PDELETEA(m_pHalfKey);
			if(rOtherH323NetSetup.m_pHalfKey)
			{			
				m_pHalfKey	= new BYTE[m_hkLen]; //I will delete it in partyApi.cpp
				memcpy((char *)m_pHalfKey,rOtherH323NetSetup.m_pHalfKey,m_hkLen);
			}
			else
				m_pHalfKey = NULL;
		}
		else
			DBGPASSERT(YES);
	}
	return *this;                                       
}

/////////////////////////////////////////////////////////////////////////////
CH323NetSetup& CH323NetSetup::operator=(const CH323NetSetup& other)
{
    if (this != &other)
    {
        CIpNetSetup::operator=(other);
        
        if (strcmp(other.NameOf() ,"CH323NetSetup") == 0)
        {
            
            CH323NetSetup & rOtherH323NetSetup = (CH323NetSetup &)other;
            
            strncpy(m_bestH323PartyAlias,rOtherH323NetSetup.m_bestH323PartyAlias, ALIAS_NAME_LEN);
            m_bestH323PartyAlias[ALIAS_NAME_LEN] = '\0';
            
            strncpy(m_h323PartyAlias,rOtherH323NetSetup.m_h323PartyAlias,IP_STRING_LEN - 1);
            m_h323PartyAlias[IP_STRING_LEN - 1] = '\0';
            m_h323PartyAliasType = rOtherH323NetSetup.m_h323PartyAliasType;
            
      		// IpV6
            strncpy(m_srcH323PartyAliases,rOtherH323NetSetup.m_srcH323PartyAliases,MaxAddressListSize - 1);
            m_srcH323PartyAliases[MaxAddressListSize - 1] = '\0';
            strncpy(m_destH323PartyAliases,rOtherH323NetSetup.m_destH323PartyAliases,MaxAddressListSize - 1);
            m_destH323PartyAliases[MaxAddressListSize - 1] = '\0';
			
            
            
            m_userUserSize	 = rOtherH323NetSetup.m_userUserSize;
            m_conferenceGoal = rOtherH323NetSetup.m_conferenceGoal;
            m_connectionType = rOtherH323NetSetup.m_connectionType;
            //			m_callModel		 = rOtherH323NetSetup.m_callModel;
            m_bIsActiveMc	 = rOtherH323NetSetup.m_bIsActiveMc;
            m_bIsOrigin		 = rOtherH323NetSetup.m_bIsOrigin;
            m_bH245Establish = rOtherH323NetSetup.m_bH245Establish;
            
            memcpy(&m_h245Address, &rOtherH323NetSetup.m_h245Address, sizeof(mcTransportAddress));
            strncpy(m_sH323userUser, rOtherH323NetSetup.m_sH323userUser, MaxUserUserSize);
            m_sH323userUser[MaxUserUserSize - 1] = '\0';
            
            //to connect with the remote EP in DahliaIn
            memcpy(m_conferenceId, rOtherH323NetSetup.m_conferenceId, MaxConferenceIdSize);
            memcpy(m_callId, rOtherH323NetSetup.m_callId, MaxConferenceIdSize);
            m_callReferenceValue = rOtherH323NetSetup.m_callReferenceValue;
            
            m_localH225Port	  = rOtherH323NetSetup.m_localH225Port;
            
            m_generator	= rOtherH323NetSetup.m_generator;
            m_hkType	= rOtherH323NetSetup.m_hkType;
            m_hkLen		= rOtherH323NetSetup.m_hkLen;
            
            PDELETEA(m_pHalfKey);
            if(rOtherH323NetSetup.m_pHalfKey)
            {			
                m_pHalfKey	= new BYTE[m_hkLen]; //I will delete it in partyApi.cpp
                memcpy((char *)m_pHalfKey,rOtherH323NetSetup.m_pHalfKey,m_hkLen);
            }
            else
            {
                m_pHalfKey = NULL;
            }
        }
        else //if (strcmp(other.NameOf() ,"CH323NetSetup") == 0)
        {
            DBGPASSERT(YES);
        }
    }
    
    return *this;                                       
}


/////////////////////////////////////////////////////////////////////////////
void  CH323NetSetup::SetH245Address()                 
{
	char *pFrontPtr, *pRearPtr;
	char port[MaxPortSize], ip[MaxAddressListSize];
	int i;
	
	pFrontPtr = strstr(m_strSrcPartyAddress,"TA:");
	if (pFrontPtr)
		pFrontPtr += 3;

	if(pFrontPtr)
	{
		pRearPtr  = strchr(pFrontPtr,':');
		if(pRearPtr)
		{
			int maxStrSizeForKW = min(MaxAddressListSize -1, pRearPtr - pFrontPtr);
			strncpy(ip,	pFrontPtr, maxStrSizeForKW);
			// if size in under the max, strncpy will copy the null as well so no need for : i = min(MaxAddressListSize, pRearPtr - pFrontPtr + 1);		ip[i - 1] = '\0';
			ip[maxStrSizeForKW] = 0;
			
			pFrontPtr = pRearPtr + 1;
			pRearPtr  = strchr(pFrontPtr,',');

			if(pRearPtr)
			{
				maxStrSizeForKW = min(MaxPortSize -1, pRearPtr - pFrontPtr);
				strncpy(port, pFrontPtr, maxStrSizeForKW);
				// if size in under the max, strncpy will copy the null as well so no need for : i = min(MaxPortSize, pRearPtr - pFrontPtr + 1);	port[i - 1] = '\0';
				port[maxStrSizeForKW] = 0;
			}
			else
			{
				strncpy(port, pFrontPtr, MaxPortSize - 1);
				port[MaxPortSize - 1] = '\0';
			}

		}
		else
		{
			strncpy(ip,	pFrontPtr, MaxAddressListSize - 1);
			ip[MaxAddressListSize - 1] = '\0';
			port[0]					= '\0';
		}
	}
	else
		return;

		m_h245Address.addr.v4.ip   = SystemIpStringToDWORD(ip, eHost);  				
	m_h245Address.port = atoi(port);
}


/////////////////////////////////////////////////////////////////////////////
void  CH323NetSetup::SetH323PartyAlias(const char* name)                 
{
	char			 AliasName[IP_STRING_LEN] = {0};
	char*		 pmyAliasName = const_cast<char*> (name);
	int ret =  HtmlToUtf8(&pmyAliasName, AliasName, strlen(name));
	if(1 == ret)
		strncpy(m_h323PartyAlias, AliasName, sizeof(m_h323PartyAlias)  - 1);
	else
		strncpy(m_h323PartyAlias, name, sizeof(m_h323PartyAlias)  - 1);

	  m_h323PartyAlias[sizeof(m_h323PartyAlias) - 1]='\0';
}

/////////////////////////////////////////////////////////////////////////////
const char*  CH323NetSetup::GetH323PartyAlias () const                 
{
  return m_h323PartyAlias;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323NetSetup::SetH323PartyAliasType(WORD h323Type)                 
{
	m_h323PartyAliasType = h323Type;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CH323NetSetup::GetH323PartyAliasType()                 
{
    return m_h323PartyAliasType;
}

/////////////////////////////////////////////////////////////////////////////
void CH323NetSetup::GetH323srcPartyTA(char* sPartyTA)                 
{
	char *pFrontPtr, *pRearPtr;
	int i;

	pFrontPtr = strstr(m_strSrcPartyAddress,"TA:");
	if(pFrontPtr)
	{
		pRearPtr  = strchr(pFrontPtr+3,',');
		if(pRearPtr)
		{
			strncpy(sPartyTA,	pFrontPtr, pRearPtr - pFrontPtr);
			i = min(IP_STRING_LEN, pRearPtr - pFrontPtr + 1);
			sPartyTA[i - 1] = '\0';
		}
	}
}

//==========================================================================//
//---HOMOLOGATION. Test #RAS_TE_STA_02
DWORD CH323NetSetup::GetCallReferenceValue()
{
    return this->m_callReferenceValue;
}
//==========================================================================//
/////////////////////////////////////////////////////////////////////////////
// Function name: GetH323FirstE164PartyAlias             written by: Uri Avni
// Variables:     NONE.
// Description:	  Find the first party alias from type E.164 or party number.
/////////////////////////////////////////////////////////////////////////////
CH323Alias*  CH323NetSetup::GetH323FirstE164PartyAlias ()                 
{
	CH323Alias* pH323Alias = new CH323Alias;
	char *tmp1Pointer, *tmp2Pointer;
	
	tmp1Pointer = tmp2Pointer = m_pH323PointerToAlias = m_strSrcPartyAddress;
	
	while(m_pH323PointerToAlias && ( m_pH323PointerToAlias[0] != '\0'))
	{
		tmp1Pointer = strchr(m_pH323PointerToAlias,',');
		tmp2Pointer = strchr(m_pH323PointerToAlias,';');
		if(tmp1Pointer != NULL)
		{
			if(tmp2Pointer != NULL)
				tmp2Pointer = min( tmp2Pointer, tmp1Pointer);
			else
				tmp2Pointer = tmp1Pointer;
		}
		
		if( strncmp(m_pH323PointerToAlias, "TEL:",  4)) 
		{//it's not the E164 alias
			if(tmp2Pointer != NULL)
			{
				m_pH323PointerToAlias = tmp2Pointer + 1;
				continue;
			}
			else
				break;
		}
		
		InitComponnentInList(tmp2Pointer, pH323Alias);
		if((tmp1Pointer == NULL) && (tmp2Pointer == NULL))
			break;
	}
	
	return pH323Alias;
}//GetH323FirstE164PartyAlias

/////////////////////////////////////////////////////////////////////////////
// Method name:   SetH323HighestPriorityPartyAlias
// Variables:     NONE.
// Description:	  set first available alias with highest priority
// Params:	
/////////////////////////////////////////////////////////////////////////////
void CH323NetSetup::SetH323HighestPriorityPartyAlias ()              
{
	CH323Alias* pH323Alias = NULL;
	char *tmp1Pointer, *tmp2Pointer;
	char *pAlias;
	char *pBegin, *pTempBegin;  // for party number search
	char *pOriginalPointerToAlias =  m_pH323PointerToAlias;
	pBegin = tmp1Pointer = tmp2Pointer = m_pH323PointerToAlias = m_srcH323PartyAliases;
	
	if(m_pH323PointerToAlias)
	{
		// try to find first 323_id
 		pAlias = strstr(m_pH323PointerToAlias,"NAME:");
		if(!pAlias)
		// try to find first e164 id
			pAlias = strstr(m_pH323PointerToAlias,"TEL:");
		if(!pAlias)
		// try to find first e164 id
			pAlias = strstr(m_pH323PointerToAlias,"EXT:");
		if(!pAlias)
		// try to find first e164 id
			pAlias = strstr(m_pH323PointerToAlias,"SUB:");
				
		//	try to find partyNumber	alias
		// iterate over all aliases and try to find first party number alias
		if(!pAlias)
		{
			pBegin = strchr(m_pH323PointerToAlias,',');
			if(pBegin) pBegin++;
			while(pBegin)
			{
				// if start with an alphanumeric char (0-9, *, #), apparently its a partyNumber. all rest aliases start with description string 
				if( (pBegin[0] < '9' &&  pBegin[0] > '0') || pBegin[0] == '*' || pBegin[0] == '#' )
				{
					pAlias = pBegin; break;
				}
				else
				{
					pTempBegin = pBegin;
					// get next alias
					pBegin = strchr(pTempBegin,',');
					if(pBegin) pBegin++;
				}
					
			}
		}
		//	try to find first url id	
		if(!pAlias)
			pAlias = strstr(m_pH323PointerToAlias,"URL:");
		//	try to find first email id	
		if(!pAlias)
			pAlias = strstr(m_pH323PointerToAlias,"EMAIL:");
			
		
		if(pAlias) 
		{
			// find the pointer to the end of the alias
			tmp1Pointer = strchr(pAlias,',');
			tmp2Pointer = strchr(pAlias,';');
			if(tmp1Pointer != NULL)
			{
				if(tmp2Pointer != NULL)
					tmp2Pointer = min( tmp2Pointer, tmp1Pointer);
				else
					tmp2Pointer = tmp1Pointer;
			}
			
			pH323Alias = new CH323Alias;
			m_pH323PointerToAlias = pAlias;
			InitComponnentInList(tmp2Pointer, pH323Alias);
		}
	}
	if(pH323Alias) 
	{
		const char *pAliasName = pH323Alias->GetAliasName();
		if(pAliasName)
			memcpy(m_bestH323PartyAlias, pAliasName, ALIAS_NAME_LEN);
	}
	m_pH323PointerToAlias = pOriginalPointerToAlias;	
	POBJDELETE(pH323Alias);
}//SetH323HighestPriorityPartyAlias

/////////////////////////////////////////////////////////////////////////////
CH323Alias* CH323NetSetup::GetSrcPartyAliasList(int *numOfSrcAlias)
{
	CH323Alias* pH323Alias = NULL;
	char *tmp1Pointer, *tmp2Pointer;
	int i = 0;
	// IpV6
	tmp1Pointer = m_pH323PointerToAlias = m_srcH323PartyAliases;
	while(m_pH323PointerToAlias && ( m_pH323PointerToAlias[0] != '\0'))
	{//finding how many componnent are in the string
		if(strncmp( m_pH323PointerToAlias, "TA:", 3)) i++;
		tmp1Pointer			  = strchr(m_pH323PointerToAlias,',');
		m_pH323PointerToAlias = strchr(m_pH323PointerToAlias,';');
		if(tmp1Pointer != NULL)
		{
			if(m_pH323PointerToAlias != NULL)
				m_pH323PointerToAlias = min( m_pH323PointerToAlias, tmp1Pointer);
			else
				m_pH323PointerToAlias = tmp1Pointer;
		}
		if(m_pH323PointerToAlias != NULL)
			m_pH323PointerToAlias++;
	}

	//allocate the ch323AliasList
	if(i)
		pH323Alias  = new CH323Alias[i];
	*numOfSrcAlias = i;
	i = 0;
	tmp1Pointer = tmp2Pointer = m_pH323PointerToAlias = m_srcH323PartyAliases;
	
	while(m_pH323PointerToAlias && ( m_pH323PointerToAlias[0] != '\0'))
	{
		tmp1Pointer = strchr(m_pH323PointerToAlias,',');
		tmp2Pointer = strchr(m_pH323PointerToAlias,';');
		if(tmp1Pointer != NULL)
		{
			if(tmp2Pointer != NULL)
				tmp2Pointer = min( tmp2Pointer, tmp1Pointer);
			else
				tmp2Pointer = tmp1Pointer;
		}

		if(m_pH323PointerToAlias && strncmp( m_pH323PointerToAlias, "TA:", 3))
			i++;
		InitComponnentInList(tmp2Pointer, pH323Alias + i - 1);
		if((tmp1Pointer == NULL) && (tmp2Pointer == NULL))
			break;
	}
	
	return pH323Alias;
}//GetSrcPartyAliasList

/////////////////////////////////////////////////////////////////////////////
CH323Alias* CH323NetSetup::GetDestPartyAliasList(int *numOfDestAlias)
{
	CH323Alias* pH323Alias = NULL;
	char *tmp1Pointer, *tmp2Pointer;
	int i = 0;

	tmp1Pointer = m_pH323PointerToAlias = m_destH323PartyAliases;
	while(m_pH323PointerToAlias && ( m_pH323PointerToAlias[0] != '\0'))
	{//finding how many componnent are in the string
		if(strncmp( m_pH323PointerToAlias, "TA:", 3)) i++;
		tmp1Pointer			  = strchr(m_pH323PointerToAlias,',');
		m_pH323PointerToAlias = strchr(m_pH323PointerToAlias,';');
		if(tmp1Pointer != NULL)
		{
			if(m_pH323PointerToAlias != NULL)
				m_pH323PointerToAlias = min( m_pH323PointerToAlias, tmp1Pointer);
			else
				m_pH323PointerToAlias = tmp1Pointer;
		}
		if(m_pH323PointerToAlias != NULL)
			m_pH323PointerToAlias++;
	}

	//allocate the ch323AliasList
	if(i)
	pH323Alias  = new CH323Alias[i];
	*numOfDestAlias = i;
	i = 0;
	tmp1Pointer = tmp2Pointer = m_pH323PointerToAlias = m_destH323PartyAliases;

	while(m_pH323PointerToAlias && ( m_pH323PointerToAlias[0] != '\0'))
	{
		tmp1Pointer = strchr(m_pH323PointerToAlias,',');
		tmp2Pointer = strchr(m_pH323PointerToAlias,';');
		if(tmp1Pointer != NULL)
		{
			if(tmp2Pointer != NULL)
				tmp2Pointer = min( tmp2Pointer, tmp1Pointer);
			else
				tmp2Pointer = tmp1Pointer;
		}

		if(m_pH323PointerToAlias && strncmp( m_pH323PointerToAlias, "TA:", 3))
			i++;
		InitComponnentInList(tmp2Pointer, pH323Alias + i - 1);
		if((tmp1Pointer == NULL) && (tmp2Pointer == NULL))
			break;
	}
	
	return pH323Alias;
}//GetDestPartyAliasList

/////////////////////////////////////////////////////////////////////////////
void CH323NetSetup::InitComponnentInList(char* pRearPtr, CH323Alias* pH323Alias)
{

	PTRACE2(eLevelInfoNormal, "CH323NetSetup::InitComponnentInList, m_pH323PointerToAlias:", m_pH323PointerToAlias);
	if(!strncmp( m_pH323PointerToAlias, "TA:", 3))
	{//it's the ip address do not entered that to the list
		m_pH323PointerToAlias = pRearPtr + 1;
		return;
	}

	if (!strncmp(m_pH323PointerToAlias,"NAME:",5)) 
	{
		m_pH323PointerToAlias += 5;
		pH323Alias->SetAliasType( PARTY_H323_ALIAS_H323_ID_TYPE );
	}

	else if (!strncmp(m_pH323PointerToAlias,"TNAME:",6))        //added for BRIDGE-136
	{
		m_pH323PointerToAlias += 6;
		pH323Alias->SetAliasType( PARTY_H323_ALIAS_TRANSPORT_ID_TYPE );
	}

	else if (!strncmp(m_pH323PointerToAlias,"EMAIL:",6))
	{
		m_pH323PointerToAlias += 6;
		pH323Alias->SetAliasType( PARTY_H323_ALIAS_EMAIL_ID_TYPE );
	}
		
	else if (!strncmp(m_pH323PointerToAlias,"URL:",4))
	{
		m_pH323PointerToAlias += 4;
		PTRACE2(eLevelInfoNormal, "CH323NetSetup::InitComponnentInList, URL - m_pH323PointerToAlias:", m_pH323PointerToAlias);
		pH323Alias->SetAliasType( PARTY_H323_ALIAS_URL_ID_TYPE );
	}
		
	else if (!strncmp(m_pH323PointerToAlias,"TEL:",4) ||
			 !strncmp(m_pH323PointerToAlias,"EXT:",4) ||
			 !strncmp(m_pH323PointerToAlias,"SUB:",4))
	{
		m_pH323PointerToAlias += 4;
		pH323Alias->SetAliasType( PARTY_H323_ALIAS_E164_TYPE );
	}

	else
	// sequence of E164Address with ";" delimeter.
		pH323Alias->SetAliasType( PARTY_H323_ALIAS_PARTY_NUMBER_TYPE );

	if(pRearPtr)
	{// found the delimeter 
	    ALLOCBUFFER(tempString,IP_STRING_LEN);  
		strncpy(tempString, m_pH323PointerToAlias, pRearPtr - m_pH323PointerToAlias );
		tempString[ pRearPtr - m_pH323PointerToAlias ] = '\0';
		PTRACE2(eLevelInfoNormal, "1. CH323NetSetup::InitComponnentInList, tempString: ", tempString);
		pH323Alias->SetAliasName( tempString );
		m_pH323PointerToAlias = pRearPtr +1 ;
		DEALLOCBUFFER(tempString);  
	}
	else 
	{// end of string from stack of radvision
		PTRACE2(eLevelInfoNormal, "2. CH323NetSetup::InitComponnentInList, m_pH323PointerToAlias: ", m_pH323PointerToAlias);
		pH323Alias->SetAliasName( m_pH323PointerToAlias );
		m_pH323PointerToAlias = NULL;
	}	
}
/////////////////////////////////////////////////////////////////////////////
void CH323NetSetup::SetSrcPartyAddress(const char * strAddress)
{
	strncpy(m_strSrcPartyAddress,strAddress,sizeof(m_strSrcPartyAddress) - 1);
	m_strSrcPartyAddress[sizeof(m_strSrcPartyAddress) - 1] = '\0';
//	SetDwordIp(YES);
}
/////////////////////////////////////////////////////////////////////////////
void CH323NetSetup::SetDestPartyAddress(const char * strAddress)
{
	strncpy(m_strDestPartyAddress,strAddress,MaxAddressListSize - 1);
	m_strDestPartyAddress[MaxAddressListSize - 1] = '\0';
//	SetDwordIp(NO);
} 

/////////////////////////////////////////////////////////////////////////////
const char * CH323NetSetup::GetStartOfAddress(const char * strAddress) const
{
	char * pFrontPtr = NULL;
	pFrontPtr = (char*)strstr(strAddress,"TA:");
	if (pFrontPtr)
		pFrontPtr += 3;
	return pFrontPtr;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// IpV6
void CH323NetSetup::SetSrcPartyAliases(const char* srcPartyAlias)
{
	strncpy(m_srcH323PartyAliases,srcPartyAlias,sizeof(m_srcH323PartyAliases) - 1);
	m_srcH323PartyAliases[sizeof(m_srcH323PartyAliases) - 1] = '\0';
}
/////////////////////////////////////////////////////////////////////////////
void CH323NetSetup::SetDestPartyAliases(const char* destPartyAlias)
{
	strncpy(m_destH323PartyAliases,destPartyAlias,sizeof(m_destH323PartyAliases) - 1);
	m_destH323PartyAliases[sizeof(m_destH323PartyAliases) - 1] = '\0';
}
/////////////////////////////////////////////////////////////////////////////
const char * CH323NetSetup::GetH323SrcPartyAliases () const
{
	return m_srcH323PartyAliases;
}
/////////////////////////////////////////////////////////////////////////////
const char * CH323NetSetup::GetH323DestPartyAliases () const
{
	return m_destH323PartyAliases;
}





