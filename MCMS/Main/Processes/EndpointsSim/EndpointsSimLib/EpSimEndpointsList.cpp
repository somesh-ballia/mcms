//+========================================================================+
//                  EpSimEndpointsList.cpp								   |
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

//#include <string>

#include "Macros.h"
#include "Trace.h"
#include "TraceStream.h"
#include "IpCsOpcodes.h"
#include "IVRPlayMessage.h"
#include "SipCsReq.h"
#include "ConfPartyDefines.h"
#include "ConfPartySharedDefines.h"
#include "OpcodesRanges.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsNetQ931.h"
#include "SystemFunctions.h"

#include "Segment.h"
#include "ProcessBase.h"
#include "MplMcmsProtocol.h"
#include "SocketApi.h"

#include "SimApi.h"
#include "EndpointsSim.h"
#include "EndpointsGuiApi.h"

#include "EndpointH323.h"
#include "EndpointSip.h"
#include "EndpointPstn.h"
#include "EndpointISDN.h"
#include "EndpointLinker.h"
#include "EndpointsSimConfig.h"

#include "EpSimCapSetsList.h"
#include "EpSimH323BehaviorList.h"
#include "EpSimEndpointsList.h"
#include "EndpointsSimConfig.h"

#include "TBStructs.h"
#include "ObjString.h"

#include "Bonding.h"
#include "OpcodesMcmsBonding.h"
#include "OpcodesMcmsMux.h"
#include "OpcodesMcmsInternal.h"

// static global CallIndex for CS header
static DWORD g_dwCsCallIndex = 101;

static DWORD g_dwInitialIp = (123 << 24) | (123 << 16) | (0 << 8) | 1; // Initial Ip = "123.123.0.1"

static DWORD g_dwInitialEpId = 10001; // Initial ID = 10001"
static DWORD g_dwInitialLinkId = 20001; // Initial ID = 20001"



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//   CEndpointsList - Unified list of EndPoint elements
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
CEndpointsList::CEndpointsList(CTaskApi* /*pCSApi*/,CCapSetsList* pCapList,CH323BehaviorList* pBehavList)
//			: CEpSimBasicEpList(pCSApi,pCapList,pBehavList,GUI_TO_ENDPOINTS)     // constructor
{
	m_updateCounter = 1;
	m_pCapList      = pCapList;
	m_pBehaviorList = pBehavList;

	for (int i = 0; i < MAX_H323_EPS; i++)
	{
		m_paEpArray[i] = NULL;
		m_raAudioBoards[i].CleanDetails();
	}
//	m_pCSApi = pCSApi;
}

/////////////////////////////////////////////////////////////////////////////
CEndpointsList::~CEndpointsList()     // destructor
{
	m_pCapList      = NULL;
	m_pBehaviorList = NULL;
	for (int i = 0; i < MAX_H323_EPS; i++)
		POBJDELETE(m_paEpArray[i]);
}

/////////////////////////////////////////////////////////////////////////////
CEndpoint* CEndpointsList::CreateDialOutParty(const eEndpointsTypes type,
			const DWORD nConfID, const DWORD nPartyID,const DWORD nConnectionID,
			const WORD wCsHandle, const WORD wCsDestUnit,
			const WORD boardId,const WORD subBoardId,const WORD spanId,const WORD portId, bool isCascade)
{
	CEndpoint*		pEndpoint = NULL;

	CCapSet*		pCap = NULL;
	CH323Behavior*	pBehavior = NULL;

	const CCapSet*  pDefaultCap = m_pCapList->GetDefaultCap();
	if( NULL != pDefaultCap )
	{
		pCap = new CCapSet(*pDefaultCap);
	}
	else
		pCap = new CCapSet;

	//set encryption in capset according to encryption flag for dial-out
	pCap->SetEncryption(::GetEpSystemCfg()->GetEncryptionDialOut());


	const CH323Behavior*  pDefaultBehavior = m_pBehaviorList->GetDefaultBehavior();
	if( NULL != pDefaultBehavior )
		pBehavior = new CH323Behavior(*pDefaultBehavior);
	else
		pBehavior = new CH323Behavior;

	if(GetEpListLength() == 0)//list is empty, reset the initial value.
		g_dwInitialEpId = 10001;

	char  szName[MAX_EP_NAME];
	memset(szName,0,MAX_EP_NAME);
	DWORD nId = 0;
	if(type == eEndPointH323 && isCascade)
	{
		nId = g_dwInitialLinkId;
		g_dwInitialLinkId++;
		sprintf(szName,"LINK_OUT#%d",nId);
	}
	else
	{
		nId = GenerateEndpointID();
		sprintf(szName,"DIAL_OUT#%d",nId);
	}

//	if( type == eEndSip )
//		pEndpoint = new CEndpointSip(m_pCSApi,*pCap,*pBehavior);
//	else
//		pEndpoint = new CEndpointH323(m_pCSApi,*pCap,*pBehavior);
	switch( type )
	{
                case eEndPointH323:
			if(isCascade)
				pEndpoint=new CEndpointLinker(NULL /*m_pCSApi*/, *pCap, *pBehavior);
			else
				pEndpoint = new CEndpointH323(NULL /*m_pCSApi*/, *pCap, *pBehavior);
			break;
		//case eEndPointH323: pEndpoint = new CEndpointH323(NULL /*m_pCSApi*/, *pCap, *pBehavior); break;
		case eEndPointSip:  pEndpoint = new CEndpointSip(NULL /*m_pCSApi*/, *pCap, *pBehavior); break;
		case eEndPointPstn: pEndpoint = new CEndpointPstn(NULL /*m_pCSApi*/, *pCap, *pBehavior); break;
		case eEndPointIsdn: pEndpoint = new CEndpointISDN(NULL /*m_pCSApi*/, *pCap, *pBehavior); break;

		default: pEndpoint = new CEndpointH323(NULL /*m_pCSApi*/, *pCap, *pBehavior); break;
	}
	pEndpoint->SetName(szName);
	pEndpoint->SetID(nId);
	pEndpoint->SetDialDirection(DIAL_OUT);
	pEndpoint->SetEpConfID(nConfID);
	pEndpoint->SetEpPartyID(nPartyID);
	pEndpoint->SetEpConnectionID(nConnectionID);

	DWORD nCallIndex = GenerateCallIndex();
	pEndpoint->SetCallIndex(nCallIndex); // CsCallIndex for IP or net_connection_id for ISDN

		// IP specific fields
	if( eEndPointH323 == type  ||  eEndPointSip == type )
	{
		pEndpoint->SetCsHandle(wCsHandle);
		pEndpoint->SetCsSrcUnit(wCsDestUnit);
	}
		// ISDN specific fields
	else if( eEndPointPstn == type ||  eEndPointIsdn == type )
	{
		pEndpoint->SetNetBoardDetails(boardId, subBoardId, spanId, portId);
	}

	POBJDELETE(pCap);
	POBJDELETE(pBehavior);

	return pEndpoint;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CEndpointsList::GenerateEndpointID() const
{
/*const DWORD INITIAL_ID = 10000;

	DWORD  id = INITIAL_ID;
	for( DWORD i=0; i<MAX_H323_EPS; i++ )
		if( m_paEpArray[i] != NULL && m_paEpArray[i]->GetID() > id )
			id = m_paEpArray[i]->GetID();

	DWORD dwTempIp = g_dwInitialIp++;*/
	return g_dwInitialEpId++;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::GenerateEndpointIp(char* pszIp, DWORD ipVer) const
{
	DWORD dwTempIp = g_dwInitialIp++;

	if (ipVer == eIpVersion6)
	{
		sprintf(pszIp,"2001:0db8:85a3:08d3:%d:%d:%d:%d",
			(dwTempIp & 0xff000000)>>24,(dwTempIp & 0xff0000)>>16,(dwTempIp & 0xff00)>>8,(dwTempIp & 0xff));
	}
	else //(ipVer == eIpVersion4)
	{
		sprintf(pszIp,"%d.%d.%d.%d",
			(dwTempIp & 0xff000000)>>24,(dwTempIp & 0xff0000)>>16,(dwTempIp & 0xff00)>>8,(dwTempIp & 0xff));
	}
}

/////////////////////////////////////////////////////////////////////////////
DWORD CEndpointsList::GenerateCallIndex()
{
	//return g_dwCsCallIndex++;
	DWORD tmp = g_dwCsCallIndex;
	g_dwCsCallIndex += MAX_ADDITIONAL_PHONE_NUM;
	return tmp;
}

bool CEndpointsList::CheckCascade( mcReqCallSetup* pSetup )
{
	// destPartyAddress format  "TA:172.22.132.108:1720"
	// find the dest IP
	// V4.1C <--> V6 merge temp patch!!!
	char tempName[128];
	memset (&tempName, '\0', 128);
	ipToString(pSetup->destIpAddress.transAddr, tempName, 1);

//	FPASSERTMSG(1969,tempName);
	std::string temp;
	temp = tempName;
	//temp = pSetup->destPartyAddress;

	if(!strcmp(tempName, "1.1.2.2"))// a patch to check cascade
	{
//		FPASSERTMSG(1969,tempName);
		return true;
	}

	if (temp.length() < 12) {
		return false; // error
	}

	std::string::size_type taInd = temp.find("TA:");		// find the start of TA:
	if (taInd == std::string::npos) {
		return false; // error
	}
	std::string::size_type ta3 = taInd + 3; // start of
	if(temp[ta3]==0)//if end of string?
		return false;

	std::string::size_type portInd = temp.find(":", ta3);		// find the next ":" portInd
	if (portInd == std::string::npos) {	// if no portInd? default by 1720?
		return false;
		//temp = temp.substr(ta3);
	}
	else {
		temp = temp.substr( ta3, (portInd - ta3));
	}

	//compare with CS IP. If match, mean cascade
	std::string csIP= ::GetEpSystemCfg()->GetCSIpAddress();
	if(csIP==temp)
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::HandleCsEvent( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal, "CEndpointsList::HandleCsEvent");

	// search for the party. if not exists, creating a new one
	DWORD epPartyID      = pMplProtocol->getPortDescriptionHeaderParty_id();
	DWORD epConfID       = pMplProtocol->getPortDescriptionHeaderConf_id();
	DWORD epConnectionID = pMplProtocol->getPortDescriptionHeaderConnection_id();
	WORD  wCsHandle      = pMplProtocol->getCentralSignalingHeaderCsId();
	WORD  wCsDestUnit    = pMplProtocol->getCentralSignalingHeaderDestUnitId();
	DWORD dwCsCallIndex  = pMplProtocol->getCentralSignalingHeaderCallIndex();

	CEndpoint*  pCurrentEp = FindPartyByCallIndex(dwCsCallIndex);
//	CEndpoint*  pCurrentEp = FindPartyByConfParty(epConfID,epPartyID);
	if( NULL == pCurrentEp )
	{
		// if not found - search by IP - should be DIAL-IN
		pCurrentEp = FindByCallAnswerReq(pMplProtocol->getCommonHeaderOpcode(), (BYTE*)pMplProtocol->GetData());

		if( NULL == pCurrentEp )
		{
			char szStatusStr[256];
			if( H323_CS_SIG_CALL_SETUP_REQ == pMplProtocol->getCommonHeaderOpcode() )
			{
				// if not found - create new one
				// should be DIAL-OUT
				bool isCascade=CheckCascade((mcReqCallSetup*)pMplProtocol->GetData());
				pCurrentEp = CreateDialOutParty(eEndPointH323,epConfID,epPartyID,epConnectionID,
								wCsHandle,wCsDestUnit,0,0,0,0,isCascade);
				// add to list
				STATUS result = AddEndpoint(pCurrentEp,szStatusStr);
				if( STATUS_OK != result )
				{
					PTRACE(eLevelError,"CEndpointsList::HandleCsEvent - Cannot create EP");
					POBJDELETE(pCurrentEp);
					return;
				}
			}
			else if( SIP_CS_SIG_INVITE_REQ == pMplProtocol->getCommonHeaderOpcode() )
			{
				// if not found - create new one
				// should be DIAL-OUT
				pCurrentEp = CreateDialOutParty(eEndPointSip,epConfID,epPartyID,epConnectionID,
								wCsHandle,wCsDestUnit,0,0,0,0);
				// add to list
				STATUS result = AddEndpoint(pCurrentEp,szStatusStr);
				if( STATUS_OK != result )
				{
					PTRACE(eLevelError,"CEpSipList::HandleCsEvent - Cannot create EP");
					POBJDELETE(pCurrentEp);
					return;
				}
			}
			else
			{
				ALLOCBUFFER(pszMessage,512);
				sprintf(pszMessage,"Opcode<%d>, ConfId<%d>, PartyId<%d>, ConnId<%d>",
					pMplProtocol->getCommonHeaderOpcode(),epConfID,epPartyID,epConnectionID);
				PTRACE2(eLevelError,"CEndpointsList::HandleCsEvent - UNEXPECTED MESSAGE. Party not found: ",pszMessage);
				DEALLOCBUFFER(pszMessage);
				return;
			}
		}
	}

	// check waiting list for party
	// if party found in Board Details waiting list,
	//   update parameters of E.p. and remove from waiting list
	int  i = 0;
	for( i=0; i<MAX_H323_EPS; i++ )
	{
		if( m_raAudioBoards[i].GetConfId() == epConfID  && m_raAudioBoards[i].GetPartyId() == epPartyID )
		{
			pCurrentEp->SetAudioBoardDetails(m_raAudioBoards[i].GetBoardId(),
				m_raAudioBoards[i].GetSubBoardId(),m_raAudioBoards[i].GetUnitId(),
				m_raAudioBoards[i].GetPortId());
			m_raAudioBoards[i].CleanDetails();
			break;
		}
	}
	pCurrentEp->HandleProtocolEvent( pMplProtocol );

	return;

}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::AddEpFromGui(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CEndpointsList::AddEpFromGui");

	DWORD  epType = eEndPointUnknown;
	*pParam >> epType;

	CCapSet* pCap = new CCapSet;
	CH323Behavior* pBehav = new CH323Behavior;

	CEndpoint*  pEpTemp = NULL;

	if ( epType == eEndPointSip )
		pEpTemp = new CEndpointSip(NULL /*m_pCSApi*/, *pCap, *pBehav);
	else if ( epType == eEndPointH323 )
		pEpTemp = new CEndpointH323(NULL /*m_pCSApi*/, *pCap,*pBehav);
	else if ( epType == eEndPointPstn )
		pEpTemp = new CEndpointPstn(NULL /*m_pCSApi*/, *pCap, *pBehav);
	else if ( epType == eEndPointIsdn )
			pEpTemp = new CEndpointISDN(NULL /*m_pCSApi*/, *pCap, *pBehav);
	else
		DBGPASSERT(1000+epType);

	if( pEpTemp != NULL )
	{
		pEpTemp->SetDialDirection(DIAL_IN);
		pEpTemp->DeSerialize(*pParam);

		char szIp[IPV6_ADDRESS_LEN];
		GenerateEndpointIp(szIp, pEpTemp->GetIpVersion());
		pEpTemp->SetIp(szIp);

		char szStatusString[256];
		STATUS  result = AddEndpoint(pEpTemp, szStatusString);
		if( STATUS_OK != result )
		{
			POBJDELETE(pEpTemp);
		//	DBGPASSERT(1);

			// send error message to GUI
			m_guiTxMbx.DeSerialize(*pParam);

			CSegment* pMsgSeg = new CSegment;
			*pMsgSeg	<< GUI_TO_ENDPOINTS
						<< ADD_EP_REQ
						<< (DWORD)result;

			*pMsgSeg	<< szStatusString;

			CTaskApi api;
			api.CreateOnlyApi(m_guiTxMbx);
			api.SendMsg(pMsgSeg,SOCKET_WRITE);
			api.DestroyOnlyApi();
		}
	}
	POBJDELETE(pCap);
	POBJDELETE(pBehav);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::AddEpRangeFromGui(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CEndpointsList::AddEpRangeFromGui");

	DWORD  range = 0;
	*pParam >> range;

	DWORD  epType = eEndPointUnknown;
	*pParam >> epType;

	CCapSet* pCap = new CCapSet;
	CH323Behavior* pBehav = new CH323Behavior;

	STATUS  result = STATUS_OK;
	char szStatusStr[256];
	for( int i=0; i<(int)range; i++ )
	{
		CSegment  copySegment(*pParam);

		CEndpoint*  pEpTemp = NULL;

		if ( epType == eEndPointSip )
			pEpTemp = new CEndpointSip(NULL /*m_pCSApi*/, *pCap, *pBehav);
		else if ( epType == eEndPointH323 )
			pEpTemp = new CEndpointH323(NULL /*m_pCSApi*/, *pCap, *pBehav);
		else if ( epType == eEndPointPstn )
			pEpTemp = new CEndpointPstn(NULL /*m_pCSApi*/, *pCap, *pBehav);
		else if ( epType == eEndPointIsdn )
			pEpTemp = new CEndpointISDN(NULL /*m_pCSApi*/, *pCap, *pBehav);
		else
			DBGPASSERT(1000+epType);

		if( pEpTemp != NULL )
		{
			pEpTemp->SetDialDirection(DIAL_IN);
			pEpTemp->DeSerialize(copySegment);

			char szIp[IPV6_ADDRESS_LEN];
			GenerateEndpointIp(szIp, pEpTemp->GetIpVersion());
			pEpTemp->SetIp(szIp);

			char  szName[MAX_EP_NAME];
			char  szIndex[MAX_EP_NAME];
			
			memset(szName,MAX_EP_NAME,0);
			memset(szIndex,MAX_EP_NAME,0);

			sprintf(szIndex,"_%d",i+1);
			int nTemp = sizeof(szName) - strlen(szIndex)- 1;
			strncpy(szName,pEpTemp->GetName(),nTemp);
			szName[nTemp] = '\0';
			strncat(szName, szIndex, sizeof(szName) - strlen(szName) - 1 );
			szName[sizeof(szName) - 1] = '\0';

			pEpTemp->SetName(szName);

			result = AddEndpoint(pEpTemp,szStatusStr);
			if( STATUS_OK != result )
			{
				POBJDELETE(pEpTemp);
				m_guiTxMbx.DeSerialize(copySegment);
			}
		}
	}
	if( STATUS_OK != result )
	{
		// send error message to GUI
		CSegment* pMsgSeg = new CSegment;
		*pMsgSeg	<< GUI_TO_ENDPOINTS
					<< GUI_ADD_EP_RANGE_REQ
					<< (DWORD)result;

		*pMsgSeg	<< szStatusStr;

		CTaskApi api;
		api.CreateOnlyApi(m_guiTxMbx);
		api.SendMsg(pMsgSeg,SOCKET_WRITE);
		api.DestroyOnlyApi();
	}
	POBJDELETE(pCap);
	POBJDELETE(pBehav);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::HandleBatchEvent(const DWORD opcode, CSegment* pParam)
{
	TRACEINTO << " CEndpointsList::HandleBatchEvent, opcode: " << opcode;

	DumpEndpoints();

	switch( opcode )
	{
		case BATCH_SCP_STREAMS_REQUEST:
		{
			char	szPartyName[H243_NAME_LEN];
			char	szConfName[H243_NAME_LEN];
			*pParam >> szConfName >> szPartyName;
			TRACESTR(eLevelInfoNormal) << " BATCH_SCP_STREAMS_REQUEST for " << szPartyName << " confname=" << szConfName;

			CEndpoint* pEp = FindParty(szPartyName);
			if( NULL != pEp ){
				pEp->OnScpStreamsRequest(pParam);
			}
			else
			{
				TRACESTR(eLevelError) << " BATCH_SCP_STREAMS_REQUEST - ep <" << szPartyName << "> not found.";
				DumpEndpoints();
			}
			break;
		}
		case BATCH_ADD_PARTY:
		{
			char	szEpName[MAX_EP_NAME];
			char	szConfName[MAX_CONF_NAME];
			char	szCapSetName[MAX_CONF_NAME];
			DWORD	epTypeTmp = eEndPointUnknown;
			DWORD	dialInChannelsNumber = 6;	// default
			DWORD   ipVer = eIpVersion4;
			DWORD   csID = -1;
			char	szManufact[64];
			char	sourcePartyAliasName[H243_NAME_LEN];
			char	userAgent[IP_STRING_LEN];
			memset(szManufact,0,sizeof(szManufact));
			memset(sourcePartyAliasName,0,H243_NAME_LEN);
			memset(userAgent,0,IP_STRING_LEN);

			*pParam >> epTypeTmp;

			eEndpointsTypes epType  = (eEndpointsTypes)epTypeTmp;
			switch(epType)
			{
					case eEndPointH323:
						*pParam >> szEpName >> szConfName >> szCapSetName >> ipVer
								 >> csID >> szManufact >> sourcePartyAliasName;
						break;
					case eEndPointSip:
						*pParam >> szEpName >> szConfName >> szCapSetName >> ipVer  >> csID >> szManufact >> sourcePartyAliasName  >> userAgent;
						break;
					case eEndPointPstn:
						*pParam >> szEpName >> szConfName >> szCapSetName;
						break;
					case eEndPointIsdn:
						*pParam >> szEpName >> szConfName >> szCapSetName >> dialInChannelsNumber;
						break;
					default :
						DBGPASSERT(1000000+epType);
						return;
			}


			TRACESTR(eLevelInfoNormal) << " BATCH_ADD_PARTY for " << szEpName
					                   << " Capset name " << szCapSetName;

			CCapSet*		pCap = NULL;
			CH323Behavior*	pBehavior = NULL;

			if( strlen(szCapSetName) != 0 )
			{
				const CCapSet* pTempCap = m_pCapList->FindCapSet(szCapSetName);
				if( NULL != pTempCap )
					pCap = new CCapSet(*pTempCap);
			}

			if( NULL == pCap ) // not found set with this name
			{
				const CCapSet*  pDefaultCap = m_pCapList->GetDefaultCap();
				if( NULL != pDefaultCap )
					pCap = new CCapSet(*pDefaultCap);
				else
					pCap = new CCapSet;
			}

			const CH323Behavior*  pDefaultBehavior = m_pBehaviorList->GetDefaultBehavior();
			if( NULL != pDefaultBehavior )
				pBehavior = new CH323Behavior(*pDefaultBehavior);
			else
				pBehavior = new CH323Behavior;

			CEndpoint*  pEpTemp = NULL;
			if ( epType == eEndPointSip )
			{


				PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleBatchEvent -user agent ",userAgent);
				pEpTemp = new CEndpointSip(NULL /*m_pCSApi*/, *pCap, *pBehavior);
				((CEndpointSip*)pEpTemp)->SetUserAgent(userAgent);

			}
			else if ( epType == eEndPointH323 )
			{
				pEpTemp = new CEndpointH323(NULL /*m_pCSApi*/, *pCap, *pBehavior);
				((CEndpointH323*)pEpTemp)->SetSourcePartyAlias(sourcePartyAliasName);
			}
			else if ( epType == eEndPointPstn )
			{
				pEpTemp = new CEndpointPstn(NULL /*m_pCSApi*/, *pCap, *pBehavior);
				((CEndpointPstn*)pEpTemp)->SetPhoneNum(szConfName);
			}
			else if ( epType == eEndPointIsdn )
			{
				pEpTemp = new CEndpointISDN(NULL /*m_pCSApi*/, *pCap, *pBehavior);
				((CEndpointISDN*)pEpTemp)->SetPhoneNum(szConfName);
				((CEndpointISDN*)pEpTemp)->SetDialInChannelsNumber( dialInChannelsNumber );
			}
			else
			{
				DBGPASSERT(1000 + epType);
			}

			TRACESTR(eLevelInfoNormal) << " BATCH_ADD_PARTY 2 " << szEpName;

			if( pEpTemp != NULL )
			{
				pEpTemp->SetName(szEpName);
				pEpTemp->SetConfName(szConfName);
				pEpTemp->SetDialDirection(DIAL_IN);
				pEpTemp->SetIpVersion(ipVer);

				if(csID != (DWORD)-1)
				    pEpTemp->SetCsHandle(csID);

				char szIp[IPV6_ADDRESS_LEN];
				GenerateEndpointIp(szIp, pEpTemp->GetIpVersion());
//				szIp[0] = '\0';
				pEpTemp->SetIp(szIp);

				CVendorInfo newVendorInfo(szManufact);
				pEpTemp->SetVendor(&newVendorInfo);

				char szStatusStr[256];
				szStatusStr[0] = '\0';
				STATUS  result = AddEndpoint(pEpTemp,szStatusStr);
				if( STATUS_OK != result )
				{
					TRACEINTO << szStatusStr;
					POBJDELETE(pEpTemp);
					DBGPASSERT(1);
				}
			}
			POBJDELETE(pCap);
			POBJDELETE(pBehavior);
			break;
		}
		case BATCH_DEL_PARTY:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_DEL_PARTY for " << szEpName;

			if(strncmp(szEpName, "ALL_SIM_EPS_AND_RESET_IP", strlen("ALL_SIM_EPS_AND_RESET_IP")) == 0)
			{// reset simulation global IP address to enable repeat of several scripts without reset
				g_dwInitialIp = (123 << 24) | (123 << 16) | (0 << 8) | 1; // Initial Ip = "123.123.0.1"
			}
			if(strncmp(szEpName, "ALL_SIM_EPS", strlen("ALL_SIM_EPS")) == 0)
			{// delete all SIP EPs
				for ( int i = 0; i < MAX_H323_EPS; i++ )
				{
					if ( NULL != m_paEpArray[i] )
					{
						if( m_paEpArray[i]->IsReadyToDelete() == TRUE )
						{
							POBJDELETE(m_paEpArray[i]);
							m_updateCounter++;
						}
						else
							m_paEpArray[i]->OnGuiDeleteEp();
					}
				}
			}
			else
			{
				CEndpoint* pEp = FindParty(szEpName);
				if( NULL != pEp )
					pEp->OnGuiDeleteEp();
				else
				{
					TRACESTR(eLevelError) << " BATCH_DEL_PARTY - EP <" << szEpName << "> not found.";
					DumpEndpoints();
				}
			}

			break;
		}
		case BATCH_CONNECT_PARTY:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_CONNECT_PARTY for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->OnGuiConnectEp();
			else
			{
				TRACESTR(eLevelError) << " BATCH_CONNECT_PARTY - ep <" << szEpName << "> not found.";
				DumpEndpoints();
			}

			break;
		}
		case BATCH_SET_PARTY_CAPSET:
		{
			char	szEpName[MAX_EP_NAME];
			char	szCapSetName[MAX_CONF_NAME];
			*pParam >> szEpName
					>> szCapSetName;

			TRACESTR(eLevelInfoNormal) << " BATCH_SET_PARTY_CAPSET for " << szEpName << ", with CapSet " << szCapSetName;

			CCapSet*		pCap = NULL;

			if( strlen(szCapSetName) != 0 )
			{
				const CCapSet* pTempCap = m_pCapList->FindCapSet(szCapSetName);
				if( NULL != pTempCap )
					pCap = new CCapSet(*pTempCap);
			}

			if( NULL == pCap ) // not found set with this name
			{
				const CCapSet*  pDefaultCap = m_pCapList->GetDefaultCap();
				if( NULL != pDefaultCap )
					pCap = new CCapSet(*pDefaultCap);
				else
					pCap = new CCapSet;
			}
			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->OnGuiSetCapsForEp(pCap);
			else
			{
				TRACESTR(eLevelError) << " BATCH_SET_PARTY_CAPSET - ep <" << szEpName << "> not found.";
				DumpEndpoints();
			}

			POBJDELETE(pCap);
			break;
		}
		case BATCH_CHANGEMODE_PARTY:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_CHANGEMODE_PARTY for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->SendChangeMode();
			else
				TRACESTR(eLevelError) << " BATCH_CHANGEMODE_PARTY - ep " << szEpName << " not found.";

			break;
		}
		case BATCH_DISCONNECT_PARTY:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_DISCONNECT_PARTY for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->OnGuiDisconnectEp();
			else
			{
				TRACESTR(eLevelError) << " BATCH_DISCONNECT_PARTY - ep <" << szEpName << "> not found.";
				DumpEndpoints();
			}

			break;
		}
		case BATCH_DTMF_PARTY:
		{
			char	szEpName[MAX_EP_NAME];
			char	szDtmf[MAX_RCV_DTMF];
			WORD	wDtmfSource = 0xFFFF;
			*pParam >> szEpName
					>> szDtmf
					>> wDtmfSource;

			TRACESTR(eLevelInfoNormal) << " BATCH_DTMF_PARTY for " << szEpName << ", Dtmf=" << szDtmf
								 << ", Dtmf Source # - " << wDtmfSource << ".";

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->SendDtmf(szDtmf, wDtmfSource);
			else
			{
				TRACESTR(eLevelError) << " BATCH_DTMF_PARTY - ep <" << szEpName << "> not found.";
				DumpEndpoints();
			}

			break;
		}
		case BATCH_ACTIVE_SPEAKER:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_ACTIVE_SPEAKER for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->ActiveSpeaker();
			else
			{
				TRACESTR(eLevelError) << " BATCH_ACTIVE_SPEAKER - ep <" << szEpName << "> not found.";
				DumpEndpoints();
			}

			break;
		}
		case BATCH_AUDIO_SPEAKER:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_AUDIO_SPEAKER for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->AudioSpeaker();
			else
			{
				TRACESTR(eLevelError) << " BATCH_AUDIO_SPEAKER - ep <" << szEpName << "> not found.";
				DumpEndpoints();
			}

			break;
		}
		case BATCH_MUTE:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_MUTE for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
			{
				if(pEp->GetEpType() == eEndPointSip)
				{
					WORD wAudioMuteByPort = 0xFFFF;
					WORD wVideoMuteByPort = 0xFFFF;
					char szAudioMuteByDirection[H243_NAME_LEN];
					char szVideoMuteByDirection[H243_NAME_LEN];
					char szAudioMuteByInactive[H243_NAME_LEN];
					char szVideoMuteByInactive[H243_NAME_LEN];

					*pParam >> wAudioMuteByPort;
					*pParam >> wVideoMuteByPort;
					*pParam >> szAudioMuteByDirection;
					*pParam >> szVideoMuteByDirection;
					*pParam >> szAudioMuteByInactive;
					*pParam >> szVideoMuteByInactive;
					pEp->Mute(wAudioMuteByPort, wVideoMuteByPort, szAudioMuteByDirection, szVideoMuteByDirection,
								szAudioMuteByInactive, szVideoMuteByInactive);
				}
				else
					pEp->Mute();
			}
			else
			{
				TRACESTR(eLevelError) << " BATCH_MUTE - ep <" << szEpName << "> not found.";
				DumpEndpoints();
			}

			break;
		}
		case BATCH_UNMUTE:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_UNMUTE for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->Unmute();
			else
				TRACESTR(eLevelError) << " BATCH_UNMUTE - ep <" << szEpName << "> not found.";

			break;
		}
		case BATCH_FECC_TOKEN_REQUEST:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_FECC_TOKEN_REQUEST for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->FeccTokenRequest();
			else
				TRACESTR(eLevelError) << " BATCH_FECC_TOKEN_REQUEST - ep <" << szEpName << "> not found.";

			break;
		}
		case BATCH_FECC_TOKEN_RELEASE:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_FECC_TOKEN_RELEASE for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->FeccTokenRelease();
			else
				TRACESTR(eLevelError) << " BATCH_FECC_TOKEN_RELEASE - ep <" << szEpName << "> not found.";

			break;
		}
		case BATCH_H239_TOKEN_REQUEST:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_H239_TOKEN_REQUEST for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->H239TokenRequest();
			else
			{
				if(!strncmp("ByConf", szEpName, strlen("ByConf")))
				{
					char *temp = szEpName + strlen("ByConf");
//					char	szConfId[MAX_EP_NAME];
//					char	szPartyNo[MAX_EP_NAME];
//					*pParam >> szConfId;
//					*pParam >> szPartyNo;

					// current infrastructure doesn't support additional info so I hard ccoded it.
					DWORD confId 	= 1;//atoi(temp);
					DWORD partyNo 	= 2; //atoi(szPartyNo);
					TRACESTR(eLevelInfoNormal) << " BATCH_H239_TOKEN_REQUEST of confId - " << confId << ", and Party No. - " << partyNo;
					pEp = FindPartyNumberXinConf(confId, partyNo);
				}

				if( NULL != pEp )
					pEp->H239TokenRequest();
				else
					TRACESTR(eLevelError) << " BATCH_H239_TOKEN_REQUEST - ep <" << szEpName << "> not found.";
			}

			break;
		}
		case BATCH_H239_TOKEN_RELEASE:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_H239_TOKEN_RELEASE for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
				pEp->H239TokenRelease();
			else
				TRACESTR(eLevelError) << " BATCH_H239_TOKEN_RELEASE - ep <" << szEpName << "> not found.";

			break;
		}
		case BATCH_FECC_KEY_REQUEST:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_FECC_FECC_REQUEST for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
			{
				char  szDtmf[MAX_RCV_DTMF];
				*pParam >> szDtmf;
				pEp->FeccKeyRequest(szDtmf);
			}
			else
				TRACESTR(eLevelError) << " BATCH_FECC_FECC_REQUEST - ep <" << szEpName << "> not found.";

			break;
		}
		case BATCH_CHANNELS_UPDATE: //all BATCH_ prefix is from the python
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_CHANNELS_UPDATE for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
			{
				BYTE	recapMode;
				bool	au, vi, fecc, h239;
				BOOL	tmp;
				char	szManufact[64];

				*pParam >> recapMode;

				*pParam >> tmp;
				au = tmp? true : false;
				*pParam >> tmp;
				vi = tmp? true : false;
				*pParam >> tmp;
				fecc = tmp? true : false;
				*pParam >> tmp;
				h239 = tmp? true : false;

				*pParam >> szManufact;

				pEp->UpdateChannels(au,vi,fecc,h239,recapMode,szManufact);
			}
			else
			{
				TRACESTR(eLevelError) << " BATCH_CHANNELS_UPDATE - ep <" << szEpName << "> not found.";
				DumpEndpoints();
			}

			break;
		}
		case BATCH_LPR_MODE_CHANGE_REQUEST:
		{
			char	szEpName[MAX_EP_NAME];
			*pParam >> szEpName;

			TRACESTR(eLevelInfoNormal) << " BATCH_LPR_MODE_CHANGE_REQUEST for " << szEpName;

			CEndpoint* pEp = FindParty(szEpName);
			if( NULL != pEp )
			{
				APIU32		lossProtection;
				APIU32		mtbf;
				APIU32		congestionCeiling;
				APIU32		fill;
				APIU32		modeTimeout;
				*pParam >> lossProtection >> mtbf >> congestionCeiling >> fill >> modeTimeout;

				TRACEINTO << "CEndpointsList::HandleBatchEvent "
				 << " LossProtection = " << lossProtection << " Mtbf = " << mtbf << " Congestion = " << congestionCeiling
				 << " Fill = " << fill << " Mode timeout = " << modeTimeout;

				pEp->LprModeChangeReq(lossProtection, mtbf, congestionCeiling, fill, modeTimeout);
			}
			else
				TRACESTR(eLevelError) << " BATCH_LPR_MODE_CHANGE_REQUEST - ep <" << szEpName << "> not found.";

			break;
		}

		default :
		{
			TRACESTR(eLevelError) << " CEndpointsList::HandleBatchEvent - UNKNOWN OPCODE <" << (int)opcode << ">";
			break;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::GetFullEpListToTerminal()
{
	for( int i=0; i<MAX_H323_EPS; i++ )
	{
		if ( NULL != m_paEpArray[i] )
		{
			printf(m_paEpArray[i]->GetName());
			printf("\n");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::GetFullEpListToGui(CSegment* pParam)
{

	DWORD	nGuiUpdateCounter = 0;
	*pParam >> nGuiUpdateCounter;

	m_guiTxMbx.DeSerialize(*pParam);

	// check if there is e.p. changed
	BOOL need2update = ( m_updateCounter > nGuiUpdateCounter ) ? TRUE : FALSE;

	int  i = 0;
	for( i=0; i<MAX_H323_EPS; i++ )
	{
		if ( NULL != m_paEpArray[i] )
		{
			if( m_paEpArray[i]->IsReadyToDelete() == TRUE )
			{
				POBJDELETE(m_paEpArray[i]);
				need2update = TRUE;
			}
			else if( m_paEpArray[i]->IsChanged() != FALSE )
			{
				m_paEpArray[i]->ClearChanged();
				need2update = TRUE;
			}
		}
	}
	// all endpoints in list are not changed
	if( FALSE == need2update )
		return;

		// update GUI view with endpoints list
	CSegment* pMsgSeg = new CSegment;
	*pMsgSeg	<< GUI_TO_ENDPOINTS
				<< GET_EP_LIST_REQ
				<< (DWORD)STATUS_OK;

	*pMsgSeg	<< m_updateCounter
				<< GetEpListLength();

	for( i=0; i<MAX_H323_EPS; i++ )
	{
		if ( NULL != m_paEpArray[i] )
			m_paEpArray[i]->Serialize(*pMsgSeg);
	}

	CTaskApi api;
	api.CreateOnlyApi(m_guiTxMbx);
	api.SendMsg(pMsgSeg,SOCKET_WRITE);
	api.DestroyOnlyApi();
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::DeleteEpFromGui(CSegment* pParam)
{
	DWORD epid = 0xFFFFFFFF;
	*pParam >> epid;

	m_guiTxMbx.DeSerialize(*pParam);

	CEndpoint* pEp = FindParty(epid);
	if( NULL != pEp )
		pEp->OnGuiDeleteEp();

//	DeleteEndpoint(epid);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::UpdateEpFromGui(CSegment* pParam )
{
	DWORD  epType = eEndPointUnknown;
	*pParam >> epType;

/*	CEndpoint*  pEndpointSource = NULL;
	if( epType == eEndH323 )
		pEndpointSource = new CEndpointH323();
	else if( epType == eEndSip )
		pEndpointSource = new CEndpointSip();

	if( pEndpointSource != NULL )
	{
		pEndpointSource->DeSerialize()
	}*/
/*	for (int i = 0; i < MAX_H323_EPS; i++)
		if (m_paEpArray[i] != NULL)
		{
			if (m_paEpArray[i]->GetID()==epid)
			{
				m_paEpArray[i]->OnGuiUpdateEp(pParam);
			}
		}*/
}

/////////////////////////////////////////////////////////////////////////////
//sound request to play file by given file name
void CEndpointsList::playSoundReqReq(CSegment* pParam)
{
	m_guiTxMbx.DeSerialize(*pParam);

	char partyName[11]="party name";
	char soundFileName[16]="WELCMPNS.ACA";//"songsparrow.wav";

	for( int i=0; i < MAX_H323_EPS; i++ )
	{
		if( m_paEpArray[i] ) {

			CSegment* pMsgSeg = new CSegment;
			*pMsgSeg	<< GUI_TO_ENDPOINTS
						<< PLAY_SOUND_REQ
						<< (DWORD)STATUS_OK;

			*pMsgSeg 	<< m_paEpArray[i]->GetID()
						<< soundFileName;

			CTaskApi api;
			api.CreateOnlyApi(m_guiTxMbx);
			api.SendMsg(pMsgSeg,SOCKET_WRITE);
			api.DestroyOnlyApi();
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::HandleDtmf(CSegment* pParam)
{
	DWORD epid;
	char  szDtmf[MAX_RCV_DTMF];

	*pParam >> epid;
	*pParam >> szDtmf;

	PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleDtmf - DTMF :.", szDtmf);

	CEndpoint* pEp = FindParty(epid);
	if( NULL != pEp )
		pEp->SendDtmf(szDtmf);
	else
		DBGPASSERT(1000000+epid);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::HandleConnect(CSegment* pParam)
{
	DWORD epid = 0xFFFFFFFF;
	*pParam >> epid;

	m_guiTxMbx.DeSerialize(*pParam);

	CEndpoint* pEp = FindParty(epid);
	if( NULL != pEp )
		pEp->OnGuiConnectEp();
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::HandleDisconnect(CSegment* pParam)
{
	DWORD epid = 0xFFFFFFFF;
	*pParam >> epid;

	m_guiTxMbx.DeSerialize(*pParam);

	CEndpoint* pEp = FindParty(epid);
	if( NULL != pEp )
		pEp->OnGuiDisconnectEp();
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::HandleGuiEpMessage(const DWORD opcode,CSegment* pParam)
{
	DWORD epid;

	*pParam >> epid;

	CEndpoint* pEp = FindParty(epid);
	if( NULL == pEp ) {
		DBGPASSERT_AND_RETURN(1000000+epid);
	}

	const char* pszName = pEp->GetName();
	PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleGuiEpMessage - Ep name: ",pszName);

	switch( opcode )
	{
		case EP_MUTE_REQ:
		{
			PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleGuiEpMessage - GUI COMMAND <MUTE> for Ep: ",pszName);
		/*	if(pEp->GetEpType() == eEndSip)
			{
				pEp->Mute(wAudioMuteByPort, wVideoMuteByPort, szAudioMuteByDirection, szVideoMuteByDirection,
							szAudioMuteByInactive, szVideoMuteByInactive);
			}
			else*/
				pEp->Mute();
			break;
		}
		case EP_UNMUTE_REQ:
		{
			PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleGuiEpMessage - GUI COMMAND <UNMUTE> for Ep: ",pszName);
			pEp->Unmute();
			break;
		}
		case EP_ACTIVE_SPEAKER_REQ:
		{
			PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleGuiEpMessage - GUI COMMAND <ACTIVE_SPEAKER> for Ep: ",pszName);
			pEp->ActiveSpeaker();
			break;
		}
		case EP_AUDIO_SPEAKER_REQ:
		{
			PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleGuiEpMessage - GUI COMMAND <AUDIO_SPEAKER> for Ep: ",pszName);
			pEp->AudioSpeaker();
			break;
		}
		case EP_FECC_ASK_TOKEN_REQ:
		{
			PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleGuiEpMessage - GUI COMMAND <FECC_ACQUIRE_TOKEN> for Ep: ",pszName);
			pEp->FeccTokenRequest();
			break;
		}
		case EP_FECC_RELEASE_TOKEN_REQ:
		{
			PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleGuiEpMessage - GUI COMMAND <FECC_RELEASE_TOKEN> for Ep: ",pszName);
			pEp->FeccTokenRelease();
			break;
		}
		case EP_FECC_KEY_REQ:
		{
			char  szDtmf[MAX_RCV_DTMF];
			*pParam >> szDtmf;
			PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleGuiEpMessage - GUI COMMAND <EP_FECC_KEY_REQ> for Ep: ",pszName);
			pEp->FeccKeyRequest(szDtmf);
			break;
		}
		case EP_H239_ASK_TOKEN_REQ:
		{
			PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleGuiEpMessage - GUI COMMAND <H239_ACQUIRE_TOKEN> for Ep: ",pszName);
			pEp->H239TokenRequest();
			break;
		}
		case EP_H239_RELEASE_TOKEN_REQ:
		{
			PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleGuiEpMessage - GUI COMMAND <H239_RELEASE_TOKEN> for Ep: ",pszName);
			pEp->H239TokenRelease();
			break;
		}
		case EP_DETAILS_REQ:
		{
			m_guiTxMbx.DeSerialize(*pParam);
			CSegment* pMsgSeg = new CSegment;

			*pMsgSeg	<< GUI_TO_ENDPOINTS
						<< EP_DETAILS_REQ
						<< (DWORD)STATUS_OK;

			*pMsgSeg 	<< epid;

			pEp->SerializeDetails(*pMsgSeg);

			CTaskApi api;
			api.CreateOnlyApi(m_guiTxMbx);
			api.SendMsg(pMsgSeg,SOCKET_WRITE);
			api.DestroyOnlyApi();
			return; // need return because 	<m_guiTxMbx.DeSerialize(*pParam);> before response
		}
		case EP_ENDPOINT_CHANNELS_REQ:
		{
			PTRACE2(eLevelInfoNormal,"CEndpointsList::HandleGuiEpMessage - GUI COMMAND <EP_ENDPOINT_CHANNELS_REQ> for Ep: ",pszName);
			BYTE	recapMode;
			bool	au, vi, fecc, h239;
			DWORD	tmp;
			char	szManufact[64];

			*pParam >> tmp;
			recapMode = (BYTE)tmp;
			*pParam >> tmp;
			au = tmp? true : false;
			*pParam >> tmp;
			vi = tmp? true : false;
			*pParam >> tmp;
			fecc = tmp? true : false;
			*pParam >> tmp;
			h239 = tmp? true : false;
			*pParam >> tmp;
			DWORD iCapId = tmp;
			*pParam >> szManufact;

			//extract he CapSet object
			//because later the EP object doesn't have the caspset list information.
			CCapSet *pCapSet=m_pCapList->FindCapSet(iCapId);
			m_updateCounter++;
			pEp->UpdateChannels(au,vi,fecc,h239,recapMode,szManufact, pCapSet);
			break;
		}
		case EP_SIP_CS_SIG_REINVITE_IND:
		{
			DWORD	tmp;
			*pParam >> tmp;
			DWORD iCapId = tmp;

			CCapSet *pCapSet=m_pCapList->FindCapSet(iCapId);
			m_updateCounter++;

			pEp->UpdateChannels(false, false, false ,false ,FALSE ,NULL, pCapSet);

			break;
		}
		default:
		{
			TRACESTR(eLevelError)
				<< " CEndpointsList::HandleGuiEpMessage - UNKNOWN OPCODE <"
				<< (int)opcode
				<< ">! IGNORED"
				<< ", ep name: "
				<< pszName;
			break;
		}
	}
	m_guiTxMbx.DeSerialize(*pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::HandleArtGideonMsg(const DWORD opcode,CSegment* pParam)
{
	DWORD	confId = 0xFFFFFFFF, partId = 0xFFFFFFFF, connId = 0xFFFFFFFF;
	WORD	boardId = 0xFFFF, subBoardId = 0xFFFF, unitId = 0xFFFF;

	*pParam >> confId
			>> partId
			>> connId;
	*pParam >> boardId
			>> subBoardId
			>> unitId;

	TRACEINTO << " CEndpointsList::HandleArtGideonMsg - DBGDATA: "
		<< "ConfId<"  << (int)confId     << ">, "
		<< "PartyId<" << (int)partId     << ">, "
		<< "ConnId<"  << (int)connId     << ">, "
		<< "Board<"   << (int)boardId    << ">, "
		<< "Sub<"     << (int)subBoardId << ">, "
		<< "Unit<"    << (int)unitId     << ">.";

	DumpEndpoints();

	TRACEINTO << " CEndpointsList::HandleArtGideonMsg opcode: " << opcode;

	switch( opcode )
	{
		case TB_MSG_OPEN_PORT_REQ:
		{
			TRACEINTO << " CEndpointsList::HandleArtGideonMsg - Handle TB_MSG_OPEN_PORT_REQ.";
			DumpAudioBoardsWaitingList();
			// find e.p. in list
			CEndpoint*  pCurrentEp = FindPartyByConfParty(confId,partId);

			if( NULL != pCurrentEp )
			{
				pCurrentEp->SetAudioBoardDetails(boardId,subBoardId,unitId,0);
				TRACEINTO << " CEndpointsList::HandleArtGideonMsg - TB_MSG_OPEN_PORT_REQ: found EP, Name - "
					<< pCurrentEp->GetName();
			}
			else // not found
			{
				// place board details to waiting list
				//   1. check if details is in list
				//   2. find first free place
				int index = -1;
				for( int i = 0; i < MAX_H323_EPS; i++ )
				{
					if( m_raAudioBoards[i].GetBoardId() == boardId  &&
						m_raAudioBoards[i].GetSubBoardId() == subBoardId  &&
						m_raAudioBoards[i].GetUnitId() == unitId  &&
						m_raAudioBoards[i].GetConfId() == confId  &&
						m_raAudioBoards[i].GetPartyId() == partId )
					{
						index = -1;
						TRACEINTO << " CEndpointsList::HandleArtGideonMsg - TB_MSG_OPEN_PORT_REQ: "
							<< "found board in waiting list, index <" << i << ">";
						break; // found exact board
					}
					if( -1 == index )
						if( m_raAudioBoards[i].GetBoardId() == 0xFFFF ||
							m_raAudioBoards[i].GetSubBoardId() == 0xFFFF ||
							m_raAudioBoards[i].GetUnitId() == 0xFFFF )
							index = i;
				}
				// if details not in list and free place found
				if( -1 != index )
				{
					TRACEINTO << " CEndpointsList::HandleArtGideonMsg - TB_MSG_OPEN_PORT_REQ: "
						<< "add board to waiting list, index <" << index << ">.";
					m_raAudioBoards[index].SetPartyDetails(confId,partId);
					m_raAudioBoards[index].SetBoardDetails(boardId,subBoardId,unitId,0);
				}
			}
			DumpAudioBoardsWaitingList();
			break;
		}
		case TB_MSG_CLOSE_PORT_REQ:
		{
			TRACEINTO << " CEndpointsList::HandleArtGideonMsg - Handle TB_MSG_CLOSE_PORT_REQ.";
			DumpAudioBoardsWaitingList();
			// find e.p. in list
			CEndpoint*  pCurrentEp = FindPartyByConfParty(confId,partId);

			if( NULL != pCurrentEp )
			{
				pCurrentEp->CleanAudioBoardDetails();
				TRACEINTO << " CEndpointsList::HandleArtGideonMsg - TB_MSG_CLOSE_PORT_REQ: found EP, Name - "
					<< pCurrentEp->GetName();
			}
			else // e.p not found, search in waiting list
			{
				for( int i = 0; i < MAX_H323_EPS; i++ )
				{
					if( m_raAudioBoards[i].GetBoardId() == boardId  &&
						m_raAudioBoards[i].GetSubBoardId() == subBoardId  &&
						m_raAudioBoards[i].GetUnitId() == unitId )
					{
						m_raAudioBoards[i].CleanDetails();
						TRACEINTO << " CEndpointsList::HandleArtGideonMsg - TB_MSG_CLOSE_PORT_REQ: "
							<< "found board in waiting list, index <" << i << ">, CLEAN.";
						break; // found exact board
					}
				}
			}
			DumpAudioBoardsWaitingList();
			break;
		}
		case IVR_PLAY_MESSAGE_REQ:
		case IVR_RECORD_ROLL_CALL_REQ:
		{
			if (opcode == IVR_PLAY_MESSAGE_REQ)
				PTRACE(eLevelInfoNormal,"CEndpointsList::HandleArtGideonMsg - Handle IVR_PLAY_MESSAGE_REQ.");
			else
				PTRACE(eLevelInfoNormal,"CEndpointsList::HandleArtGideonMsg - Handle IVR_RECORD_ROLL_CALL_REQ.");

			CEndpoint*  pCurrentEp = FindPartyByConfParty(confId,partId);

			if ((0xFFFFFFFF == confId) && (0xFFFFFFFF == partId))	// message to conf
			{
				DWORD	nDataLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
				BYTE*	pData = new BYTE [nDataLen];
				pParam->Get(pData,nDataLen);
				CSegment  seg;
				seg.Put(pData,nDataLen);
				PDELETEA(pData);

				CIVRPlayMessage* pIVRPlayMessage = new CIVRPlayMessage;
				pIVRPlayMessage->DeSerialize(&seg);

				SIVRPlayMessageStruct*  pReqStruct = &(pIVRPlayMessage->play);

				for( int i=0; i<(int)pReqStruct->numOfMediaFiles; i++ )
				{
					if ((pReqStruct->mediaFiles[i].actionType == IVR_ACTION_TYPE_RECORD) || // silence or recording
						(pReqStruct->mediaFiles[i].actionType == IVR_ACTION_TYPE_SILENCE))
					{
						continue;
					}
					// send indication to GUI
					char* pszFileName = pReqStruct->mediaFiles[i].fileName;
					char pszFileNameRollCall[256];

					char pszFileNameRollCallDebug[256];
					char* pszFileNameDebug = NULL;

					PTRACE2(eLevelError,"CEndpointsList::HandleArtGideonMsg - IVR_PLAY_MESSAGE_REQ. Path: ",pszFileName);

					//if (0 == i)
					//{
						CSegment* pMsgSeg = new CSegment;

						*pMsgSeg	<< GUI_TO_ENDPOINTS
									<< PLAY_SOUND_REQ
									<< (DWORD)STATUS_OK;

						if (0 == strncmp("RollCall", pszFileName, 8))
						{
						  snprintf(pszFileNameRollCall, sizeof(pszFileNameRollCall), "IVRX/%s", pszFileName);
							pszFileName = pszFileNameRollCall;

							// temporary untill Vasily will solve the problem --------------------
							strcpy( pszFileNameRollCall, "IVR/msg/English/rollcall/roll/RC_p1_1.ACA");
							pszFileName = pszFileNameRollCall;
							// temporary --------------------------------------------------------

							PTRACE2(eLevelError,"CEndpointsList::HandleArtGideonMsg - IVRX added, FilePath: ",pszFileName);
						}
						else
						{
							PTRACE2(eLevelError,"CEndpointsList::HandleArtGideonMsg - IVR_PLAY_MESSAGE_REQ. Path: ",pszFileName);
						}

						*pMsgSeg 	<< (DWORD)0xFFFFFFFF
									<< pszFileName;

						CTaskApi api;
						api.CreateOnlyApi(m_guiTxMbx);
						api.SendMsg(pMsgSeg,SOCKET_WRITE);
						api.DestroyOnlyApi();
					//}
					SystemSleep(100*2);
				}

				PDELETE(pIVRPlayMessage);
				break;
			}

			if( NULL != pCurrentEp )
			{
				TRACEINTO << " CEndpointsList::HandleArtGideonMsg - IVR_PLAY_MESSAGE_REQ or IVR_RECORD_ROLL_CALL_REQ: found EP, Name - "
					<< pCurrentEp->GetName();

				DWORD	nDataLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
				BYTE*	pData = new BYTE [nDataLen];
				pParam->Get(pData,nDataLen);
				CSegment  seg;
				seg.Put(pData,nDataLen);
				PDELETEA(pData);

				CIVRPlayMessage* pIVRPlayMessage = new CIVRPlayMessage;
				pIVRPlayMessage->DeSerialize(&seg);

				SIVRPlayMessageStruct*  pReqStruct = &(pIVRPlayMessage->play);

				for( WORD i=0; i<pReqStruct->numOfMediaFiles; i++ )
				{
					// send indication to GUI
					char* pszFileName = pReqStruct->mediaFiles[i].fileName;

					PTRACE2(eLevelError,"CEndpointsList::HandleArtGideonMsg - IVR_PLAY_MESSAGE_REQ. Path: ",pszFileName);

					if ( (opcode == IVR_PLAY_MESSAGE_REQ) || (0 == i) )
					{
						CSegment* pMsgSeg = new CSegment;

						*pMsgSeg	<< GUI_TO_ENDPOINTS
									<< PLAY_SOUND_REQ
									<< (DWORD)STATUS_OK;

						*pMsgSeg 	<< pCurrentEp->GetID()
									<< pszFileName;

						CTaskApi api;
						api.CreateOnlyApi(m_guiTxMbx);
						api.SendMsg(pMsgSeg,SOCKET_WRITE);
						api.DestroyOnlyApi();
					}
					// in case of more then one message , sleep between the messages in order to let
					// ep to play everey msg
					if (pReqStruct->numOfMediaFiles > 1 && i < (int)pReqStruct->numOfMediaFiles - 1)
						SystemSleep(SECOND* pReqStruct->mediaFiles[i].duration);
				}

				PDELETE(pIVRPlayMessage);
			}
			else
			{
				PTRACE(eLevelError,"CEndpointsList::HandleArtGideonMsg - NULL pointer pCurrentEp.");
			}

			break;
		}
		case IVR_STOP_RECORD_ROLL_CALL_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointsList::HandleArtGideonMsg - Handle IVR_STOP_RECORD_ROLL_CALL_REQ.");
		//	SIVRStopIVRStruct
			break;
		}
		case IVR_STOP_PLAY_MESSAGE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointsList::HandleArtGideonMsg - Handle IVR_STOP_PLAY_MESSAGE_REQ.");
		//	SIVRStopIVRStruct
			break;
		}
		case IVR_PLAY_MUSIC_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointsList::HandleArtGideonMsg - Handle IVR_PLAY_MUSIC_REQ.");
			CEndpoint*  pCurrentEp = FindPartyByConfParty(confId,partId);

			DWORD nEpId = 0xFFFFFFFF;

			if ((0xFFFFFFFF == confId) && (0xFFFFFFFF == partId))	// message to conf
			{
				DWORD	nDataLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
			}
			else if( NULL != pCurrentEp )
				nEpId = pCurrentEp->GetID();

			char szFileName[] = "JunkName";
			CSegment* pMsgSeg = new CSegment;

			*pMsgSeg	<< GUI_TO_ENDPOINTS
						<< GUI_PLAY_MUSIC
						<< (DWORD)STATUS_OK;

			*pMsgSeg 	<< nEpId
						<< szFileName;

			CTaskApi api;
			api.CreateOnlyApi(m_guiTxMbx);
			api.SendMsg(pMsgSeg,SOCKET_WRITE);
			api.DestroyOnlyApi();
			break;
		}
		case IVR_STOP_PLAY_MUSIC_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointsList::HandleArtGideonMsg - Handle IVR_STOP_PLAY_MUSIC_REQ.");
			CEndpoint*  pCurrentEp = FindPartyByConfParty(confId,partId);

			DWORD nEpId = 0xFFFFFFFF;

			if ((0xFFFFFFFF == confId) && (0xFFFFFFFF == partId))	// message to conf
			{
				DWORD	nDataLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
			}
			else if( NULL != pCurrentEp )
				nEpId = pCurrentEp->GetID();

//			char szFileName[] = "JunkName";
			CSegment* pMsgSeg = new CSegment;

			*pMsgSeg	<< GUI_TO_ENDPOINTS
						<< GUI_STOP_PLAY_MUSIC
						<< (DWORD)STATUS_OK;

			*pMsgSeg 	<< nEpId;

			CTaskApi api;
			api.CreateOnlyApi(m_guiTxMbx);
			api.SendMsg(pMsgSeg,SOCKET_WRITE);
			api.DestroyOnlyApi();
			break;
		}
		case MOVE_RSRC_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointsList::HandleArtGideonMsg - Handle MOVE_RSRC_REQ.");
			CEndpoint*  pCurrentEp = FindPartyByConfParty(confId,partId);

			if( NULL != pCurrentEp )
			{
				TRACEINTO << " CEndpointsList::HandleArtGideonMsg - MOVE_RSRC_REQ: found EP, Name - "
					<< pCurrentEp->GetName();

				BYTE	nDataLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
				BYTE*	pData = new BYTE [nDataLen];
				pParam->Get(pData,nDataLen);

				// update ConfId of party
				MOVE_RESOURCES_PARAMS_S*  pRequest = (MOVE_RESOURCES_PARAMS_S*)pData;
				pCurrentEp->SetEpConfID(pRequest->newConfId);

				TRACEINTO << " CEndpointsList::HandleArtGideonMsg - Handle MOVE_RSRC_REQ. New ConfId<"
					<< (int)pRequest->newConfId << ">;";

				PDELETEA(pData);
			}
			else
				DBGPASSERT(1);

			break;
		}

		case BND_CONNECTION_INIT:

		{
			CEndpointISDN* pCurrentEp = (CEndpointISDN*)FindPartyByConfParty(confId, partId);

			if( NULL != pCurrentEp )
			{
				TRACEINTO << " CEndpointsList::HandleArtGideonMsg - BND_CONNECTION_INIT: found EP, Name - "	<< pCurrentEp->GetName();

				DWORD temp;
				*pParam >> temp;	// MUX port ID
				WORD portId = (WORD)temp;
				*pParam >> temp;	// 777 for debugging
				pCurrentEp->m_muxParams.SaveMuxEpParams( connId, boardId, unitId, portId );	// save parameters for MUX port

				BYTE	nDataLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
				BYTE*	pData = new BYTE [nDataLen];
				pParam->Get(pData, nDataLen);
				pCurrentEp->HandleIsdnProtocolEvent(opcode, pData);
				PDELETEA(pData);
			}
			break;
		}
		case SEND_H_230:
		{
			CEndpointISDN* pCurrentEp = (CEndpointISDN*)FindPartyByConfParty(confId, partId);

			if( NULL != pCurrentEp )
			{
				TRACEINTO << " CEndpointsList::HandleArtGideonMsg - SEND_H_230: found EP, Name - "	<< pCurrentEp->GetName();

				BYTE	nDataLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
				BYTE*	pData = new BYTE [nDataLen];
				pParam->Get(pData, nDataLen);
				pCurrentEp->HandleIsdnProtocolEvent(opcode, pData);
				PDELETEA(pData);
			}
			break;
		}


		case SET_ECS:
		{
			CEndpointISDN* pCurrentEp = (CEndpointISDN*)FindPartyByConfParty(confId, partId);

			if( NULL != pCurrentEp )
			{
				TRACEINTO << " CEndpointsList::HandleArtGideonMsg - SET_ECS: found EP, Name - "	<< pCurrentEp->GetName();

				BYTE	nDataLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
				BYTE*	pData = new BYTE [nDataLen];
				pParam->Get(pData, nDataLen);
				pCurrentEp->HandleIsdnProtocolEvent(opcode, pData);
				PDELETEA(pData);
			}

			break;
		}

		default :
			DBGPASSERT(1000+opcode);
			break;
	}
	DumpEndpoints();
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::HandleRtmGideonMsg(const DWORD opcode, CSegment* pParam)
{
	DWORD	confId = 0xFFFFFFFF, partId = 0xFFFFFFFF, connId = 0xFFFFFFFF, netConnId = 0xFFFFFFFF;
	WORD	boardId = 0xFFFF, subBoardId = 0xFFFF, spanId = 0xFFFF, portId = 0xFFFF;

	*pParam >> boardId
			>> subBoardId
			>> spanId
			>> portId;

	*pParam >> confId
			>> partId
			>> connId;


///	if (opcode == NET_ALERT_REQ || opcode == NET_CONNECT_REQ)
///	{
		*pParam >> netConnId;
///	}

	BYTE	nDataLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
	BYTE*	pData = new BYTE [nDataLen];
	pParam->Get(pData, nDataLen);

	TRACEINTO << " CEndpointsList::HandleRtmGideonMsg - DBGDATA: "
		<< "Opcode<"  << opcode     	 << ">, "
		<< "ConfId<"  << (int)confId     << ">, "
		<< "PartyId<" << (int)partId     << ">, "
		<< "ConnId<"  << (int)connId     << ">, "
		<< "Board<"   << (int)boardId    << ">, "
		<< "Sub<"     << (int)subBoardId << ">, "
		<< "Span<"    << (int)spanId     << ">, "
		<< "Port<"    << (int)portId     << ">.";

///	if (opcode == NET_ALERT_REQ || opcode == NET_CONNECT_REQ)
///	{
		TRACEINTO << " netConnId<" << (int)netConnId << "> ";
///	}


	DumpEndpoints();

	// search for endpoint by confId + partyId
	CEndpoint*  pCurrentEp = FindPartyByConfParty(confId, partId);
	DWORD isFirstChannel = 0;

	if( NULL == pCurrentEp ) //first time !
	{
		// if not found - search by IP or Net address - should be DIAL-IN
		pCurrentEp = FindByCallAnswerReq(opcode, pData);

		if( NULL == pCurrentEp )
		{
			isFirstChannel = 1;

			if( NET_SETUP_REQ == opcode )
			{
				// if not found - create new one
				// should be DIAL-OUT


				NET_SETUP_REQ_S*  pNetSetUpSt = (NET_SETUP_REQ_S*)pData;
				APIU32 callType = pNetSetUpSt->call_type;

				TRACEINTO << " CEndpointsList::HandleRtmGideonMsg CALL type: " <<  callType;

				if (callType == ACU_VOICE_SERVICE || callType == ACU_MODEM_SERVICE ) //PSTN
				{
					pCurrentEp = CreateDialOutParty(eEndPointPstn, confId, partId, connId,
													0, 0, boardId, subBoardId, spanId, portId);
				}
				else if (callType == ACU_DATA_SERVICE) //ISDN
				{
					pCurrentEp = CreateDialOutParty(eEndPointIsdn, confId, partId, connId,
													0, 0, boardId, subBoardId, spanId, portId);
				}
				else // default is PSTN
				{
					pCurrentEp = CreateDialOutParty(eEndPointPstn, confId, partId, connId,
													0, 0, boardId, subBoardId, spanId, portId);
				}

				// add endpoint to list
				char szStatusStr[256];
				STATUS result = AddEndpoint(pCurrentEp, szStatusStr);
				if( STATUS_OK != result )
				{
					PTRACE(eLevelError,"CEndpointsList::HandleRtmGideonMsg - Cannot create EP");
					POBJDELETE(pCurrentEp);
					PDELETEA(pData);
					return;
				}
			}
			else
			{
				TRACESTR(eLevelError) << "CEndpointsList::HandleRtmGideonMsg - UNEXPECTED MESSAGE. Party not found: "
					<< "Opcode<"  << (int)opcode << ">, "
//					<< "NetCallId<" << 1 << ">, "
					<< "ConfId<"  << (int)confId << ">, "
					<< "PartyId<" << (int)partId << ">, "
					<< "ConnId<"  << (int)connId << ">";
				PDELETEA(pData);
				return;
			}
		}
		else //if participant is found by net connection ID - dial in
		{
	if( NET_ALERT_REQ == opcode )// ||  NET_CONNECT_REQ == opcode )
	{
		pCurrentEp->SetEpConfID(confId);
		pCurrentEp->SetEpPartyID(partId);
				pCurrentEp->SetEpConnectionID(connId, netConnId);
				pCurrentEp->SetNetBoardDetails(boardId, subBoardId, spanId, portId, netConnId);
	}
		}
	}
	else
	{
		// same code like first transaction - put in function
		if( NET_ALERT_REQ == opcode )// ||  NET_CONNECT_REQ == opcode )
		{
///			pCurrentEp->SetEpConfID(confId);
///			pCurrentEp->SetEpPartyID(partId);
			pCurrentEp->SetEpConnectionID(connId, netConnId);
			pCurrentEp->SetNetBoardDetails(boardId, subBoardId, spanId, portId, netConnId);
		}
	}

	// check waiting list for party
	// if party found in Board Details waiting list,
	//   update parameters of E.p. and remove from waiting list
	int  i = 0;
	for( i=0; i<MAX_H323_EPS; i++ )
	{
		if( m_raAudioBoards[i].GetConfId() == confId  && m_raAudioBoards[i].GetPartyId() == partId )
		{
			pCurrentEp->SetAudioBoardDetails(m_raAudioBoards[i].GetBoardId(),
				m_raAudioBoards[i].GetSubBoardId(),m_raAudioBoards[i].GetUnitId(),
				m_raAudioBoards[i].GetPortId());
			m_raAudioBoards[i].CleanDetails();
			break;
		}
	}


	CSegment* pNetSetupParams = new CSegment;
	*pNetSetupParams << (DWORD)connId
					 << (DWORD)boardId
					 << (DWORD)subBoardId
					 <<	(DWORD)spanId
					 << (DWORD)portId
					 << (DWORD)isFirstChannel;

	pCurrentEp->HandleIsdnProtocolEvent(opcode, pData, pNetSetupParams);

	DumpEndpoints();

	PDELETEA(pData);
	POBJDELETE(pNetSetupParams);

	return;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::HandleMRMGideonMsg(const DWORD opcode, CSegment* pParam)
{
	DWORD	confId = 0xFFFFFFFF, partId = 0xFFFFFFFF, connId = 0xFFFFFFFF, channelId = 0xFFFFFFFF;
	WORD	boardId = 0xFFFF, subBoardId = 0xFFFF, unitId = 0xFFFF, portId = 0xFFFF;

	*pParam >> boardId
			>> subBoardId
			>> unitId
			>> portId;

	*pParam >> confId
			>> partId
			>> connId
			>> channelId;

	TRACEINTO << " CEndpointsList::HandleMRMGideonMsg - DBGDATA: "
		<< "Opcode<"  << opcode     	   << ">, "
		<< "ConfId<"  << (int)confId       << ">, "
		<< "PartyId<" << (int)partId       << ">, "
		<< "ConnId<"  << (int)connId       << ">, "
		<< "ChannlId<"<< (int)channelId    << ">, "
		<< "Board<"   << (int)boardId      << ">, "
		<< "Sub<"     << (int)subBoardId   << ">, "
		<< "UnitId<"  << (int)unitId       << ">, "
		<< "Port<"    << (int)portId       << ">.";

	DumpEndpoints();

	// search for endpoint by confId + partyId
	CEndpoint*  pCurrentEp = FindPartyByConfParty(confId, partId);
	if(pCurrentEp){
		TRACEINTO << "Set Mrm info !";
		CBoardDetails tmp;
		tmp.SetBoardDetails(boardId, subBoardId, unitId, partId);
		pCurrentEp->SetMRMInfo(tmp, connId, channelId);
	}else{
		TRACEINTO << "!! Not Set Mrm info due to being not able to find EP." ;
	}

}

/////////////////////////////////////////////////////////////////////////////
STATUS CEndpointsList::ParseIpAddress(const char* pszIpAddress, BYTE* bytes)
{
	memset(bytes,0,4);

	int   i = 0;
	int   value = 0;
	char  szAddress[16];
	strncpy(szAddress,pszIpAddress,15);
	szAddress[15] = 0;

	char* pszString = szAddress;
	char* pszDot = NULL;

	while( i < 4 ) {
		if( (pszDot = strchr(pszString,'.')) != NULL )
			*pszDot = '\0';
		else if( i != 3 )
			return STATUS_FAIL;

		if( sscanf(pszString,"%u",&value) != 1 )
			return STATUS_FAIL;
		if( value < 0  ||  value > 0xff )
			return STATUS_FAIL;
		bytes[i] = value;

		pszString = pszDot + 1;
		i++;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CEndpointsList::IpAddress2Dword(const char* pszIpAddress,DWORD& dwIp)
{
	BYTE  bytes[4];
	if( ParseIpAddress(pszIpAddress,bytes) != STATUS_OK )
		return STATUS_FAIL;
//	dwIp = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
	dwIp = bytes[0];
	dwIp = dwIp << 8;
	dwIp += bytes[1];
	dwIp = dwIp << 8;
	dwIp += bytes[2];
	dwIp = dwIp << 8;
	dwIp += bytes[3];
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
char* CEndpointsList::GetProtocolTypeAsString(const eEndpointsTypes type)
{
	switch( type )
	{
		case eEndPointH323:
			return "H.323";
		case eEndPointSip:
			return "SIP";
		case eEndPointPstn:
			return "PSTN";
		case eEndPointIsdn:
			return "ISDN";
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	return "Unknown";
}

/////////////////////////////////////////////////////////////////////////////
CEndpoint* CEndpointsList::FindParty(const DWORD nId)
{
	for ( int i = 0; i < MAX_H323_EPS; i++ )
	{
		if ( NULL != m_paEpArray[i] )
		{
			if( m_paEpArray[i]->IsReadyToDelete() == TRUE )
			{
				POBJDELETE(m_paEpArray[i]);
				m_updateCounter++;
				continue;
			}
			if ( m_paEpArray[i]->GetID() == nId )
				return m_paEpArray[i];
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CEndpoint* CEndpointsList::FindParty(const char* pszName)
{
	for ( int i = 0; i < MAX_H323_EPS; i++ )
	{
		if ( NULL != m_paEpArray[i] )
		{
			if( m_paEpArray[i]->IsReadyToDelete() == TRUE )
			{
				POBJDELETE(m_paEpArray[i]);
				m_updateCounter++;
				continue;
			}
			if ( 0 == strcmp(m_paEpArray[i]->GetName(),pszName) )
				return m_paEpArray[i];
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CEndpoint* CEndpointsList::FindPartyByConfParty(const DWORD nConfId, const DWORD nPartyId)
{
	for( int i = 0; i < MAX_H323_EPS; i++ )
	{
		if( NULL != m_paEpArray[i] )
		{
			if( m_paEpArray[i]->IsReadyToDelete() == TRUE )
			{
				POBJDELETE(m_paEpArray[i]);
				m_updateCounter++;
				continue;
			}
			if( nConfId == m_paEpArray[i]->GetEpConfID() && nPartyId == m_paEpArray[i]->GetEpPartyID() )
				return m_paEpArray[i];
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CEndpoint* CEndpointsList::FindPartyNumberXinConf(const DWORD nConfId, const DWORD partyNo)
{
	DWORD internalCounter = 0;
	for( int i = 0; i < MAX_H323_EPS; i++ )
	{
		if( NULL != m_paEpArray[i] )
		{
			if( m_paEpArray[i]->IsReadyToDelete() == TRUE )
			{
				POBJDELETE(m_paEpArray[i]);
				m_updateCounter++;
				continue;
			}
			if( nConfId == m_paEpArray[i]->GetEpConfID())
			{
				internalCounter++;
				if(internalCounter == partyNo)
					return m_paEpArray[i];
			}
		}
	}
	return NULL;
}


/////////////////////////////////////////////////////////////////////////////
CEndpoint* CEndpointsList::FindPartyByCallIndex(const DWORD nCsCallIndex)
{
	for( int i = 0; i < MAX_H323_EPS; i++ )
	{
		if( NULL != m_paEpArray[i] )
		{
			if( m_paEpArray[i]->IsReadyToDelete() == TRUE )
			{
				POBJDELETE(m_paEpArray[i]);
				m_updateCounter++;
				continue;
			}
			if( nCsCallIndex == m_paEpArray[i]->GetCallIndex() )
				return m_paEpArray[i];
			else
			{
				CEndpointLinker *cascadeLinker=dynamic_cast<CEndpointLinker*>(m_paEpArray[i]);
				if(cascadeLinker && nCsCallIndex==cascadeLinker->GetCallerCallIndex())
					return cascadeLinker;
			}
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CEndpoint* CEndpointsList::FindByCallAnswerReq(const DWORD opcode,BYTE* pContentData)//CMplMcmsProtocol* pMplProtocol)
{
	if( H323_CS_SIG_CALL_ANSWER_REQ == opcode )
	{
		mcReqCallAnswer* req = (mcReqCallAnswer*)pContentData;
			//DWORD dwEpIp = req->h245Address.transAddr.addr.v4.ip;
		mcTransportAddress taEPIpAddress = req->h245Address.transAddr;

		for( int i = 0; i < MAX_H323_EPS; i++ )
		{
			if( NULL != m_paEpArray[i] )
			{
					//if( m_paEpArray[i]->GetIpDword() == dwEpIp )
						//return m_paEpArray[i];
				if ( isIpAddressEqual(m_paEpArray[i]->GetTransportAddress(), &taEPIpAddress) )
					return m_paEpArray[i];
			}
		}
	}
	else if( SIP_CS_SIG_INVITE_RESPONSE_REQ == opcode )
	{
		CEndpoint*  pResultEp = NULL;

		mcReqInviteResponse* pReq = (mcReqInviteResponse*)pContentData;
		sipSdpAndHeadersSt*  pSdp = &(pReq->sipSdpAndHeaders);
		sipMessageHeaders *  pHeaders = (sipMessageHeaders *)(pSdp->capsAndHeaders + pSdp->sipHeadersOffset);

		sipHeaderElement* 	 pHeader = NULL;
		int  numOfHeaders  = pHeaders->numOfHeaders;

		char* pSList = pHeaders->headersList + numOfHeaders * sizeof(sipHeaderElement);
		for( int i=0; i<numOfHeaders; i++ )
		{
			pHeader = (sipHeaderElement*) (pHeaders->headersList + i * sizeof(sipHeaderElement));
			char* pString = &(pSList[pHeader->position]);

			if( pHeader->eHeaderField == kFrom )
			{
				char* pszTempName = new char[128];
				for( int j = 0; j < MAX_H323_EPS; j++ )
				{
					if( NULL != m_paEpArray[j] )
					{
						/// find ep with matching EpName and IP
						sprintf(pszTempName,"%s@%s",m_paEpArray[j]->GetName(),m_paEpArray[j]->GetIpStr());
						if( 0 == strcmp(pString,pszTempName) )
						{
							pResultEp = m_paEpArray[j];
							break;
						}
					}
				}
				PDELETEA(pszTempName);
				break;
			}
		}
		return pResultEp;
	}
	else if( NET_ALERT_REQ == opcode ) // || NET_CONNECT_REQ == opcode )
	{
		for( int i = 0; i < MAX_H323_EPS; i++ )
		{
			if( NULL != m_paEpArray[i] )
			{
				if( eEndPointPstn == m_paEpArray[i]->GetEpType() )
				{
					NET_COMMON_PARAM_S* pNetHeader = (NET_COMMON_PARAM_S*)pContentData;
					if( pNetHeader->net_connection_id == m_paEpArray[i]->GetCallIndex() )
//vb					if( ((CEndpointPstn*)m_paEpArray[i])->IsMyRtmSpanPort(pNetHeader->span_id,pNetHeader->physical_port_number) )
						return m_paEpArray[i];
				}
				else if( eEndPointIsdn == m_paEpArray[i]->GetEpType() )
				{
					NET_COMMON_PARAM_S* pNetHeader = (NET_COMMON_PARAM_S*)pContentData;
					if( pNetHeader->net_connection_id == m_paEpArray[i]->GetCallIndex() )
						return m_paEpArray[i];
				}
			}
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS	CEndpointsList::AddEndpoint(CEndpoint* pEp, char* pStatusStr)
{
	CEndpoint* pEpTemp = NULL;
		// check if exists
	if( NULL == pEp )
	{
		strcpy(pStatusStr,"Can't add endpoint: Bad endpoint.");
		return STATUS_FAIL;
	}
	if( NULL != (pEpTemp = FindParty(pEp->GetName())) )
	{
		strcpy(pStatusStr,"Can't add endpoint: Name already exists.");
		return STATUS_FAIL;
	}
	if( NULL != (pEpTemp = FindParty(pEp->GetID())) )
	{
		strcpy(pStatusStr,"Can't add endpoint: ID already exists.");
		return STATUS_FAIL;
	}

	// if not found
		// check for free place
	int index = GetNextEmptyPlace();
	if( -1 == index )
	{
		strcpy(pStatusStr,"Can't add endpoint: Max num of endpoints reached.");
		return STATUS_FAIL;
	}

		// allocate ID for endpoint
	if( 0xFFFFFFFF == pEp->GetID() )
	{
//		DWORD nId = GenerateEndpointID();
		pEp->SetID(GenerateEndpointID());
	}

	DWORD nCallIndex = GenerateCallIndex();
	pEp->SetCallIndex(nCallIndex); // CsCallIndex for IP or net_connection_id for ISDN

		// add party to list
	pEp->SetArrayIndex(index);
	m_paEpArray[index] = pEp;

	m_updateCounter++;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
/*STATUS	CEndpointsList::DeleteEndpoint(const DWORD nId)
{
	for ( int i = 0; i < MAX_H323_EPS; i++ ) {
		if ( NULL != m_paEpArray[i] ) {
			if ( m_paEpArray[i]->GetID() == nId ) {
				POBJDELETE(m_paEpArray[i]);
				m_updateCounter++;
				return STATUS_OK;
			}
		}
	}
	return STATUS_FAIL;
}*/

/////////////////////////////////////////////////////////////////////////////
/*STATUS	CEndpointsList::DeleteEndpoint(const char* pszName)
{
	for ( int i = 0; i < MAX_H323_EPS; i++ ) {
		if ( NULL != m_paEpArray[i] ) {
			if ( 0 == strcmp(m_paEpArray[i]->GetName(),pszName) ) {
				POBJDELETE(m_paEpArray[i]);
				m_updateCounter++;
				return STATUS_OK;
			}
		}
	}
	return STATUS_FAIL;
}*/

/////////////////////////////////////////////////////////////////////////////
DWORD CEndpointsList::GetEpListLength()
{
	DWORD size=0;
	for ( int i = 0; i < MAX_H323_EPS; i++ )
	{
		if ( NULL != m_paEpArray[i] )
		{
			if( m_paEpArray[i]->IsReadyToDelete() == TRUE )
			{
				POBJDELETE(m_paEpArray[i]);
				m_updateCounter++;
				continue;
			}
			size++;
		}
	}
	return size;
}

/////////////////////////////////////////////////////////////////////////////
int CEndpointsList::GetNextEmptyPlace()
{
	for ( int i = 0; i < MAX_H323_EPS; i++ )
	{
		if ( NULL == m_paEpArray[i] )
			return i;
		else if( m_paEpArray[i]->IsReadyToDelete() == TRUE )
		{
			POBJDELETE(m_paEpArray[i]);
			m_updateCounter++;
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::DumpAudioBoardsWaitingList()
{
	CLargeString str;
	str << " CEndpointsList::DumpAudioBoardsWaitingLis - DBGDATA\n"
		<< "\t\tBoard\tSubB\tUnit\tPort\tConfId\tPartyId\n";
	for( int i=0; i<MAX_H323_EPS; i++ )
	{
		if( 0xFFFF == m_raAudioBoards[i].GetBoardId() && 0xFFFF == m_raAudioBoards[i].GetSubBoardId() )
			continue;
		str << "[" << i << "]"
			<< "\t\t" << (int)m_raAudioBoards[i].GetBoardId()
			<< "\t\t" << (int)m_raAudioBoards[i].GetSubBoardId()
			<< "\t\t" << (int)m_raAudioBoards[i].GetUnitId()
			<< "\t\t" << (int)m_raAudioBoards[i].GetPortId()
			<< "\t\t" << (int)m_raAudioBoards[i].GetConfId()
			<< "\t\t" << (int)m_raAudioBoards[i].GetPartyId()
			<< "\n";
	}
	TRACEINTO << str.GetString();
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointsList::DumpEndpoints()
{
	CLargeString str;
	str << " CEndpointsList::DumpEndpoints - DBGDATA\n"
		<< "Index       Name        ID    Protocol ConfId PartyId Board  SubB   Unit   Port\n";

	ALLOCBUFFER(pszString,512);
	for( int i=0; i<MAX_H323_EPS; i++ )
	{
		if( NULL == m_paEpArray[i] )
			continue;

		const CBoardDetails* pBoard = m_paEpArray[i]->GetAudioBoard();

		sprintf(pszString,"[%3d]  %14s  %5d %7s   %3d    %3d     %5d  %5d  %5d  %5d\n",
			i,
			m_paEpArray[i]->GetName(),
			(int)m_paEpArray[i]->GetID(),
			GetProtocolTypeAsString(m_paEpArray[i]->GetEpType()),
			(int)m_paEpArray[i]->GetEpConfID(),
			(int)m_paEpArray[i]->GetEpPartyID(),
			(int)pBoard->GetBoardId(),
			(int)pBoard->GetSubBoardId(),
			(int)pBoard->GetUnitId(),
			(int)pBoard->GetPortId()
		);
/*		str << "[" << i << "]"
			<< "\t" << m_paEpArray[i]->GetName()
			<< "\t" << (int)m_paEpArray[i]->GetID()
			<< "\t" << GetProtocolTypeAsString(m_paEpArray[i]->GetEpType())
			<< "\t" << (int)m_paEpArray[i]->GetEpConfID()
			<< "\t" << (int)m_paEpArray[i]->GetEpPartyID()
			<< "\t" << (int)pBoard->GetBoardId()
			<< "\t" << (int)pBoard->GetSubBoardId()
			<< "\t" << (int)pBoard->GetUnitId()
			<< "\t" << (int)pBoard->GetPortId()
			<< "\n";*/
		str << pszString;
	}
	DEALLOCBUFFER(pszString);

	TRACEINTO << str.GetString();
}

















