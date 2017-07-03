//+========================================================================+
//                     EndpointSip.cpp									   |
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

#include <string>

#include "Macros.h"
#include "Trace.h"
#include "IpCsOpcodes.h"
#include "IpMfaOpcodes.h"
#include "ConfPartyDefines.h"
#include "ConfPartySharedDefines.h"
#include "ChannelParams.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "BfcpStructs.h"
#include "IpRtpFeccRoleToken.h"
#include "IpRtpInd.h"
#include "RvCommonDefs.h"

#include "SystemFunctions.h"

#include "Segment.h"
#include "MplMcmsProtocol.h"
#include "TaskApi.h"

#include "SimApi.h"
#include "EndpointsSim.h"
#include "EndpointsSimConfig.h"

#include "EpSimCapSetsList.h"
#include "EpSimH323BehaviorList.h"
#include "EndpointSip.h"

#include "OpcodesMcmsCommon.h"
#include "HostCommonDefinitions.h"

#include "TraceStream.h"
#include "SipUtils.h"
#include "CSSimTaskApi.h"
#include "OpcodesMcmsInternal.h"

// global static parameters
static DWORD	LocalChannelIndex = 111;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// for dial in internal debug structures
typedef struct {
	APIU32						entry1_numberOfMediaLines;
	APIU32						entry1_lenOfDynamicSection;

	xmlDynamicHeader			ml1_xmlHeader;
	mcXmlTransportAddress		ml1_mediaIp;
	APIU32						ml1_rtcpPort;
	APIU8						ml1_index;
	APIU8						ml1_type;
	APIU8						ml1_subType;
	APIU8						ml1_content;
	APIS8						ml1_label[32];
	APIU32						ml1_numberOfCaps;
	APIU32						ml1_lenOfDynamicSection;
	xmlDynamicProperties		ml1_xmlDynamicProps;

	xmlDynamicHeader			xmlHeader1;
	APIU8						capTypeCode1;
	APIU8						sipPayloadType1;
	APIU16						capLength1;
	xmlDynamicProperties		xmlDynamicProps1;
	audioCapStructBase			g711u;

	xmlDynamicHeader			ml2_xmlHeader;
	mcXmlTransportAddress		ml2_mediaIp;
	APIU32						ml2_rtcpPort;
	APIU8						ml2_index;
	APIU8						ml2_type;
	APIU8						ml2_subType;
	APIU8						ml2_content;
	APIS8						ml2_label[32];
	APIU32						ml2_numberOfCaps;
	APIU32						ml2_lenOfDynamicSection;
	xmlDynamicProperties		ml2_xmlDynamicProps;

	xmlDynamicHeader			xmlHeader2;
	APIU8						capTypeCode3;
	APIU8						sipPayloadType3;
	APIU16						capLength3;
	xmlDynamicProperties		xmlDynamicProps2;
	h263CapStruct				h263;

} dummySipCap;

typedef struct {
	sipMessageHeadersBase		sipMsgHeaders;
	sipHeaderElement			elem[10];
	char						sList[256];
} dummySipHeaders;

typedef struct{
	APIU32						callRate;
	APIU32						sipMediaLinesOffset;
	APIU32						sipMediaLinesLength;
	APIU32						sipHeadersOffset;
	APIU32						sipHeadersLength;
	APIU32						sipIceOffset;
	APIU32						sipIceLength;
	APIU32						lenOfDynamicSection;
	dummySipCap					cap;
	dummySipHeaders				headers;
}dummySIPSdpAndHdrsSt;


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CEndpointSip)
	//ONEVENT(1				,1				,CEndpointH323::OnStartElement)
PEND_MESSAGE_MAP(CEndpointSip,CEndpoint);



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CEndpointSip::CEndpointSip( CTaskApi *pCSApi, const CCapSet& rCap, const CH323Behavior& rBehav )
		: CEndpoint(pCSApi, rCap, rBehav)
{
	for (int i = 0; i < SIM_SIP_MAX_API_COMMANDS; i++)
		m_apiArray[i] = 0;

	SetIp("0.0.0.0");

	// audio board details
	CleanAudioBoardDetails();
	strcpy(m_UserAgent,"Polycom SIM");

	if(307 == m_pCap->GetID() || !strcmp(m_pCap->GetName(), "FULL CAPSET SVC")) {
		m_isMRE = TRUE;
	}else{
		m_isMRE = FALSE;
	}

	m_isMRC = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CEndpointSip::CEndpointSip(const CEndpointSip& other) : CEndpoint(other)
{
	// illegal use
	PASSERT(1);
}


/////////////////////////////////////////////////////////////////////////////
CEndpointSip::~CEndpointSip()
{
}

/////////////////////////////////////////////////////////////////////////////
CEndpointSip& CEndpointSip::operator= (const CEndpointSip& other)
{
	// illegal use
	PASSERT(1);

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::DeSerialize( CSegment& rParam )
{
	DWORD tempEpID = 0xFFFFFFFF;
	char  szIpTemp[128];
	DWORD ipVer = 2;

	rParam >> tempEpID;
	rParam >> m_szEpName;
	rParam >> m_szConfName;
	rParam >> szIpTemp;

	m_pCap->DeSerialize(rParam);
	m_pBehavior->DeSerialize(rParam);

	rParam >> ipVer;

	// set CS ID
	DWORD cs_id;
	rParam >> cs_id;
	SetCsHandle(cs_id);
	SetIp(szIpTemp);
	SetIpVersion(ipVer);

	if(307 == m_pCap->GetID() || !strcmp(m_pCap->GetName(), "FULL CAPSET SVC")) {
		m_isMRE = TRUE;
	}else{
		m_isMRE = FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::Serialize(  CSegment& rSegment ) const
{
	rSegment << (DWORD)GetEpType();
	rSegment << m_nEpID;
	rSegment << m_szEpName;
	rSegment << m_szConfName;

	ALLOCBUFFER(pszIpStr, 128);

	GetIpStr(pszIpStr);
	rSegment << pszIpStr;
	DEALLOCBUFFER(pszIpStr);

	rSegment << (WORD)m_enEpState;
	rSegment << m_dialDirection;

	m_pCap->Serialize(rSegment);
	m_pBehavior->Serialize(rSegment);

	rSegment << GetIpVersion();
    rSegment << GetCSID();
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SerializeDetails(  CSegment& rSegment ) const
{
//	m_pCap->Serialize(rSegment);
//	m_pBehavior->Serialize(rSegment);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SetIp( const char* pszEpIp )
{
	//TRACEINTO << "CEndpointSip::SetIp: " << pszEpIp;
	stringToIp(&m_H225RemoteIpAddress, (char*)pszEpIp);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SetIpVersion(DWORD ipVer)
{
	m_H225RemoteIpAddress.ipVersion = ipVer;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CEndpointSip::GetIpVersion() const
{
	//TRACEINTO << "GetIpVersion(): " << m_H225RemoteIpAddress.ipVersion;
	return m_H225RemoteIpAddress.ipVersion;
}

/////////////////////////////////////////////////////////////////////////////

char* CEndpointSip::GetIpStr() const
{
	static char tempName[64];
	memset (&tempName, '\0', 64);
	return ipToString(m_H225RemoteIpAddress, tempName, 1);
}

/////////////////////////////////////////////////////////////////////////////

void CEndpointSip::GetIpStr(char* str) const
{
	char tempName[64];
	memset (&tempName, '\0', 64);
	ipToString(m_H225RemoteIpAddress, tempName, 1);

	strcpy(str, tempName);
}

/////////////////////////////////////////////////////////////////////////////
//void CEndpointSip::SetArrayIndex( const WORD ind )
//{
//	CEndpoint::SetArrayIndex(ind);
//}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CEndpointSip::HandleEvent( CSegment *pMsg,DWORD msgLen,OPCODE opCode )
{
	switch ( opCode )
	{
	case TIMER:
		{
			break;
		}

	default	:
		{
		DispatchEvent( opCode, pMsg );
		break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::HandleProtocolEvent( CMplMcmsProtocol* pMplProtocol )
{
	switch( pMplProtocol->getOpcode() )
	{
		///////////////////////////////////////////////////
		//   CONNECT PARTY
		///////////////////////////////////////////////////

		//------------------------------------
		// Dial-In Response Opcodes
		//------------------------------------
		case SIP_CS_SIG_INVITE_RESPONSE_REQ:
		{
			OnCsCallInviteResponseReq(pMplProtocol);
			break;
		}
		//------------------------------------
		// Dial-Out Opcodes
		//------------------------------------
		case SIP_CS_SIG_INVITE_REQ:
		{
			OnCsCallInviteReq(pMplProtocol);
			break;
		}
		case SIP_CS_SIG_INVITE_ACK_REQ:
		{
			OnCsCallInviteAckReq(pMplProtocol);
			break;
		}
		case SIP_CS_SIG_REINVITE_REQ:
		{
			OnCsCallReInviteReq(pMplProtocol);
			break;
		}

		///////////////////////////////////////////////////
		//   DISCONNECT PARTY
		///////////////////////////////////////////////////

		case SIP_CS_SIG_BYE_REQ:
		{
			OnCsCallByeReq( pMplProtocol );
			break;
		}
		case SIP_CS_SIG_BYE_200_OK_REQ:
		{
			OnCsCallByeOkReq( pMplProtocol );
			break;
		}
//		// Temp - Till Vasily supports correctly
//		case SIP_CS_SIG_VIDEO_FAST_UPDATE_REQ: {
//			PTRACE(eLevelInfoNormal,"CEndpointSip::HandleProtocolEvent - SIP_CS_SIG_VIDEO_FAST_UPDATE_REQ (Nothing)");
//			break;
//		}
		case SIP_CS_SIG_RINGING_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointSip::HandleProtocolEvent - SIP_CS_SIG_RINGING_REQ (Nothing)");
			break;
		}
		case SIP_CS_SIG_INFO_REQ:{
			PTRACE(eLevelInfoNormal,"CEndpointSip::HandleProtocolEvent - SIP_CS_SIG_INFO_REQ (Nothing)");
			break;
		}
		case SIP_CS_BFCP_MESSAGE_REQ:{
			PTRACE(eLevelInfoNormal,"CEndpointSip::HandleProtocolEvent - SIP_CS_BFCP_MESSAGE_REQ (Nothing)");
			break;
		}
		case SIP_CS_SIG_DIALOG_RECOVERY_REQ:{
			PTRACE(eLevelInfoNormal,"CEndpointSip::HandleProtocolEvent - SIP_CS_SIG_DIALOG_RECOVERY_REQ (Nothing)");
			break;
		}
		case SIP_CS_SIG_INFO_RESP_REQ:{
			PTRACE(eLevelInfoNormal,"CEndpointSip::HandleProtocolEvent - SIP_CS_SIG_INFO_RESP_REQ (Nothing)");
			break;
		}
		default   :
		{
			DBGPASSERT(pMplProtocol->getOpcode()+1000);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::OnGuiUpdateEp( CSegment* pParam )
{
//	COsQueue txMbx;
//	txMbx.DeSerialize(*pParam);
//      or?
//	m_guiTxMbx.DeSerialize(*pParam);

//	DeSerialize(*pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::OnGuiConnectEp()
{
	PTRACE2(eLevelInfoNormal,"CEndpointSip::OnGuiConnectEp - connect party, Name - ", m_szEpName);

	if( m_enEpState == eEpStateIdle  ||  m_enEpState == eEpStateDisconnected )
	{
		SetState(eEpStateConnecting);
		SendCsInviteInd();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::OnGuiDisconnectEp()
{
	PTRACE2(eLevelInfoNormal,"CEndpointSip::OnGuiDisconnectEp - disconnect party, Name - ",m_szEpName);

	if( m_enEpState == eEpStateConnected  ||  m_enEpState == eEpStateConnecting )
	{
		SetState(eEpStateDisconnecting);

		CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
		
		pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
		pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

		Disconnect(pCSProt);

		POBJDELETE(pCSProt);
	}
}


/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::Mute(WORD wAudioMuteByPort, WORD wVideoMuteByPort,
						char *szAudioMuteByDirection, char *szVideoMuteByDirection,
						char *szAudioMuteByInactive, char *szVideoMuteByInactive)
{
	SendCsMuteReinviteInd(wAudioMuteByPort, wVideoMuteByPort, szAudioMuteByDirection,
							szVideoMuteByDirection, szAudioMuteByInactive, szVideoMuteByInactive);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::OnGuiDeleteEp()
{
	PTRACE2(eLevelInfoNormal,"CEndpointSip::OnGuiDeleteEp - disconnect and delete party, Name - ",m_szEpName);

	m_isToBeDeleted = TRUE;

	if( m_enEpState == eEpStateConnected  ||  m_enEpState == eEpStateConnecting )
	{
		SetState(eEpStateDisconnecting);

		CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
		pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
		pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

		Disconnect(pCSProt);

		POBJDELETE(pCSProt);
	}
	else if( m_enEpState == eEpStateIdle || m_enEpState == eEpStateDisconnected || m_enEpState == eEpStateUnknown )
		m_isReadyToDelete = TRUE;
	else  // m_enEpState == eEpStateDisconnecting
	  {
		; // nothing to do
	  }
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::Disconnect( CMplMcmsProtocol* pMplProtocol )
{
	SendCsByeInd(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointSip::IsConnectionCompleted() const
{
	if( DIAL_IN == m_dialDirection && m_apiArray[SIM_SIP_RCV_INVITE_RESPONSE]<1 )
		return FALSE;
	if( DIAL_OUT == m_dialDirection && m_apiArray[SIM_SIP_RCV_INVITE]<1 )
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::CleanAfterDisconnect()
{
		// clean commands array
	for( int i=0; i<SIM_SIP_MAX_API_COMMANDS; i++ )
		m_apiArray[i] = 0;

	// clean conf caps and comm mode
	m_pConfCap->Empty();
	m_rComMode.Create(*m_pConfCap);

		// clean conf/party details
	m_confID = m_partyID = m_connectionID = 0xFFFFFFFF;

		// clean audio board details
	m_rAudioBoard.CleanDetails();

		// clean Muted flag
	m_isMuted = FALSE;

	m_isMRC = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendChangeMode()
{
	SendCsReinviteIndDummy();
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::OnCsCallInviteReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::OnCsCallInviteReq - SIP_CS_SIG_INVITE_REQ");

	mcReqInvite* pReq = (mcReqInvite*)pMplProtocol->GetData();

	sipMediaLinesEntrySt* pMediaLinesEntry = (sipMediaLinesEntrySt*)(pReq->sipSdpAndHeaders.capsAndHeaders);
	// Create conference cap set
	m_pConfCap->CreateFromSipStruct(pMediaLinesEntry);
	BYTE isSupportH261Only = FALSE;

	sipSdpAndHeadersSt*  pSdp = &(pReq->sipSdpAndHeaders);
	if(IsMrcHeader(pSdp, "MRD=MRM")){
		m_isMRC = TRUE;
	}else{
		m_isMRC = FALSE;
	}

	sipMessageHeaders *  pHeaders = (sipMessageHeaders *)(pSdp->capsAndHeaders + pSdp->sipHeadersOffset);

	sipHeaderElement* 	 pHeader = NULL;
	int  numOfHeaders  = pHeaders->numOfHeaders;

	char* pSList = pHeaders->headersList + numOfHeaders * sizeof(sipHeaderElement);
	for( int i=0; i<numOfHeaders; i++ )
	{
		pHeader = (sipHeaderElement*) (pHeaders->headersList + i * sizeof(sipHeaderElement));
		char* pString = &(pSList[pHeader->position]);

		TRACEINTO << "pHeader->eHeaderField: [" << (int)pHeader->eHeaderField << "]" << " --> " << pString;

		if (pHeader->eHeaderField == kFrom)
		{
			SetConfName(pString);
		}
		else if( pHeader->eHeaderField == kTo )
		{
			SetIp(pString);
		}
		else if(pHeader->eHeaderField == kToDisplay)
		{
			//PTRACE(eLevelInfoNormal,"CEndpointSip::OnCsCallInviteReq - 1");
			SetIsTIPEP(IsSIPEPNameIncludesSIM_TIP(pString));
			CCapSetsList::m_isTIP = GetIsTIPEP();

			if(strstr(pString,"##SIMULATION_FORCE_H261") != NULL)
			{
				PTRACE(eLevelInfoNormal,"CEndpointSip::OnCsCallInviteReq - 2");
				isSupportH261Only = TRUE;
			}
		}
	}

	//TIP support for testing embedded MLA feature
	if (GetIsTIPEP())
	{
		POBJDELETE(m_pCap);
		POBJDELETE(m_pConfCap);
		CCapSetsList *pCapList		= new CCapSetsList;
		const CCapSet* pTempCap = pCapList->FindCapSet("FULL CAPSET");
		if( NULL != pTempCap )
		{
			m_pCap = new CCapSet(*pTempCap);
			if (m_pCap != NULL)
			{
				m_pConfCap = new CCapSet();
				if (m_pConfCap!=NULL)
				{
					mcReqInvite* pReq = (mcReqInvite*)pMplProtocol->GetData();
					sipMediaLinesEntrySt* pMediaLinesEntry = (sipMediaLinesEntrySt*)(pReq->sipSdpAndHeaders.capsAndHeaders);
					m_pConfCap->CreateFromSipStruct(pMediaLinesEntry);
				}
			}
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CEndpointSip::OnCsCallInviteReq - 4");
			DBGPASSERT(1);
		}
		POBJDELETE(pCapList);
	}

	// create highest capabilities and create communication mode
	if(isSupportH261Only)
	{
		PTRACE(eLevelInfoNormal,"CEndpointSip::OnCsCallInviteReq - 3");
		POBJDELETE(m_pCap);
		CCapSetsList *pCapList		= new CCapSetsList;
		const CCapSet* pTempCap = pCapList->FindCapSet("H261+ALL");
		if( NULL != pTempCap )
			m_pCap = new CCapSet(*pTempCap);
		else
		{
			PTRACE(eLevelInfoNormal,"CEndpointSip::OnCsCallInviteReq - 4");
			DBGPASSERT(1);
		}
		POBJDELETE(pCapList);
	}
	CCapSet  rCommonCap(*m_pCap,*m_pConfCap,GetIsTIPEP());
	m_rComMode.Create(rCommonCap);


	if( m_enEpState == eEpStateIdle || m_enEpState == eEpStateDisconnected )
	{
		SetState(eEpStateConnecting);
		m_apiArray[SIM_SIP_RCV_INVITE]++;
		SendCsInviteResponseInd(pMplProtocol,FALSE /*isReinvite*/);
		if( rCommonCap.GetH239Rate() > 0 )
		{
#ifdef __BFCP_CS_CONNECTION_ENABLED__
			SendBfcpTransportInd(pMplProtocol);
#endif
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointSip::IsSIPEPNameIncludesSIM_TIP(const char* EPName)
{
	if (strstr(EPName,"SIM_TIP") == NULL)
	{
		return FALSE;
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendInfoFlowControlToParty(APIU32 videoType,APIU32 mediaDirection ,APIU32  rate)
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendInfoFlowControlToParty  ");
	DWORD  indLen    = sizeof(mcIndInfo) + sizeof(mcIndFlowControl);// + 1000; //+1000
	BYTE*  pIndArray = new BYTE [indLen];
	//memset(pIndArray,0,sizeof(mcIndInfo));
	mcIndInfo* mcIndStr = (mcIndInfo*)pIndArray;

	mcIndFlowControl* flowControlInd=(mcIndFlowControl*)(&(mcIndStr->buff));
	mcIndStr->subOpcode = FlowControl;
	mcIndStr->dynamicLen = (APIU32)(sizeof(mcIndFlowControl));
	memcpy( flowControlInd->label, "", sizeof("") );
	flowControlInd->mediaDirection = mediaDirection;
	flowControlInd->rate = rate;


	// send the struct to CS simulation
		// ================================
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;

	pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_res_none);
	pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

	//::FillCsProtocol(pCSProt,SImcIndFlowControl;P_CS_SIG_INFO_IND,pIndArray,indLen,m_nCsCallIndex);
	::FillCsProtocol(pCSProt, GetCSID(), SIP_CS_SIG_INFO_IND,
                     pIndArray, indLen, m_nCsCallIndex);
	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

	CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
       api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndArray);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::OnCsCallReInviteReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::OnCsCallReInviteReq - SIP_CS_SIG_REINVITE_REQ");

	mcReqReInvite* pReq = (mcReqReInvite*)pMplProtocol->GetData();

	sipMediaLinesEntrySt* pMediaLinesEntry = (sipMediaLinesEntrySt*)(pReq->sipSdpAndHeaders.capsAndHeaders);
	// Create conference cap set
	m_pConfCap->CreateFromSipStruct(pMediaLinesEntry);

	// create highest capabilities and create communication mode
	CCapSet  rCommonCap(*m_pCap,*m_pConfCap);
	m_rComMode.Create(rCommonCap);

	if( m_enEpState == eEpStateConnected )
	{
		SetState(eEpStateConnecting);
		m_apiArray[SIM_SIP_RCV_REINVITE]++;
		SendCsReInviteResponseInd(pMplProtocol);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::OnCsCallInviteAckReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::OnCsCallInviteAckReq - SIP_CS_SIG_INVITE_ACK_REQ");

	if( m_enEpState == eEpStateConnecting  &&  m_apiArray[SIM_SIP_RCV_INVITE] > 0 )
	{
		SetState(eEpStateConnected);
		PTRACE2(eLevelInfoNormal,"CEndpointSip::OnCsCallInviteAckReq - connection completed - ", m_szEpName);
		m_apiArray[SIM_SIP_RCV_INVITE_ACK]++;
	}
	else if (m_enEpState == eEpStateConnecting  &&  m_apiArray[SIM_SIP_RCV_REINVITE] > 0 )
	{
		SetState(eEpStateConnected);
		PTRACE2(eLevelInfoNormal,"CEndpointSip::OnCsCallInviteAckReq - ReInvite completed.",m_szEpName);
		m_apiArray[SIM_SIP_RCV_INVITE_ACK]++;
	}
	else if( m_enEpState == eEpStateDisconnecting ) // if rejected call
	{
		PTRACE2(eLevelInfoNormal,"CEndpointSip::OnCsCallInviteAckReq - rejection completed - ", m_szEpName);

		CleanAfterDisconnect();
		SetState(eEpStateDisconnected);

		m_isReadyToDelete = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::OnCsCallInviteResponseReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::OnCsCallInviteResponseReq - SIP_CS_SIG_INVITE_RESPONSE_REQ");

	// update port description details
	m_confID       = pMplProtocol->getPortDescriptionHeaderConf_id();
	m_partyID      = pMplProtocol->getPortDescriptionHeaderParty_id();
	m_connectionID = pMplProtocol->getPortDescriptionHeaderConnection_id();
	m_wCsHandle    = pMplProtocol->getCentralSignalingHeaderCsId();
	m_wCsSrcUnit   = pMplProtocol->getCentralSignalingHeaderDestUnitId();

	mcReqInviteResponse* p = (mcReqInviteResponse*)pMplProtocol->getpData();
	if (p->status)
	{
		// call rejected

		SetState(eEpStateDisconnecting);

		SendCsInviteAckInd(pMplProtocol);

		CleanAfterDisconnect();
		SetState(eEpStateDisconnected);

		if( TRUE == m_isToBeDeleted )
			m_isReadyToDelete = TRUE;
	}
	else
	{
		m_apiArray[SIM_SIP_RCV_INVITE_RESPONSE]++;

		sipMediaLinesEntrySt* pMediaLinesEntry = (sipMediaLinesEntrySt*)(p->sipSdpAndHeaders.capsAndHeaders);
		// Create conference cap set
		m_pConfCap->CreateFromSipStruct(pMediaLinesEntry);

		SendCsInviteAckInd(pMplProtocol);

		if(IsMrcHeader(&(p->sipSdpAndHeaders), "MRD=MRM")){
			m_isMRC = TRUE;
		}else{
			m_isMRC = FALSE;
		}

		// if we have BFCP and user declares BFCP - send reinvite with content video cap
		if( NeedReinviteForBfcp() == TRUE )
		{
#ifdef __BFCP_CS_CONNECTION_ENABLED__
			SendBfcpTransportInd(pMplProtocol);
#endif
			SendCsReinviteInd(pMplProtocol);
		}
		else
			SetState(eEpStateConnected);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::OnCsCallByeReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::OnCsCallByeReq - SIP_CS_SIG_BYE_REQ");

	SetState(eEpStateDisconnecting);

	SendCsByeAckInd(pMplProtocol);

	CleanAfterDisconnect();
	SetState(eEpStateDisconnected);

	if( TRUE == m_isToBeDeleted )
		m_isReadyToDelete = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::OnCsCallByeOkReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::OnCsCallByeOkReq - SIP_CS_SIG_BYE_200_OK_REQ");

	CleanAfterDisconnect();
	SetState(eEpStateDisconnected);

	if( TRUE == m_isToBeDeleted )
		m_isReadyToDelete = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::UpdateChannels(const bool au,const bool vi,const bool fecc,const bool h239,
						const BYTE recapMode,const char* pszManufacturerName, const CCapSet *pCapSet)
{
	PTRACE2(eLevelError,"CEndpointSip::UpdateChannels (no channels in SIP), Name - ",m_szEpName);

	if( m_enEpState != eEpStateConnected )
	{
		PTRACE2(eLevelError,"CEndpointSip::UpdateChannels, EndPoint is not in CONNECTED state. Name - ",m_szEpName);
		return;
	}

	if(pCapSet != NULL)
	{
		POBJDELETE(m_pCap);
		m_pCap=new CCapSet(*pCapSet);
	}

	// create highest capabilities and create communication mode
	CCapSet  rCapCommon(*m_pCap, *m_pConfCap);

	SendCsReInviteIndDynamic();

/*	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
	pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID);
	pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

	if( SIM_RECAP_MODE_BEFORE == recapMode )
		SendCapabilitiesInd(*m_pCap,pCSProt);

	TChannelDetails* pChannel = NULL;

	//h239 channel
	TChannelDetails *present_video=GetChannel(cmCapVideo,kRolePresentation,cmCapReceivedir=IN);
	UpdateIncomingChannByNewCap(present_video, rCapCommon);

	//people video channel
	TChannelDetails *people_video=GetChannel(cmCapVideo,kRolePeople,cmCapReceivedir=IN);
	UpdateIncomingChannByNewCap(people_video, rCapCommon);

	m_rComMode.Create(rCapCommon);

		// audio channel
	pChannel = GetChannel(cmCapAudio,kRolePeople,cmCapReceive);
	if( NULL != pChannel && m_rComMode.IsAudioToOpen() )
	{
		if( true != au && TRUE == pChannel->openedInSim )
			SendStartChnlCloseInd(pCSProt,pChannel);
		else if( true == au && TRUE != pChannel->openedInSim )
		{
			pChannel->channelIndex   = g_LocalChannelIndex++;
			pChannel->channelMcIndex = 0;
			SendIncomingChannelInd(pCSProt,pChannel,rCapCommon);
		}
	}
	this code is replace by UpdateIncomingChannByNewCap
		// video channel
	pChannel = GetChannel(cmCapVideo,kRolePeople,cmCapReceive);
	if( NULL != pChannel && m_rComMode.IsVideoToOpen() )
	{
		if( true != vi && TRUE == pChannel->openedInSim )
			SendStartChnlCloseInd(pCSProt,pChannel);
		else if( true == vi && TRUE != pChannel->openedInSim )
		{
			pChannel->channelIndex   = g_LocalChannelIndex++;
			pChannel->channelMcIndex = 0;
			SendIncomingChannelInd(pCSProt,pChannel,rCapSet);
		}
	}
		// fecc channel
	pChannel = GetChannel(cmCapData,kRolePeople,cmCapReceive);
	if( NULL != pChannel && m_rComMode.IsFeccToOpen() )
	{
		if( true != fecc && TRUE == pChannel->openedInSim )
			SendStartChnlCloseInd(pCSProt,pChannel);
		else if( true == fecc && TRUE != pChannel->openedInSim )
		{
			pChannel->channelIndex   = g_LocalChannelIndex++;
			pChannel->channelMcIndex = 0;
			SendIncomingChannelInd(pCSProt,pChannel,rCapCommon);
		}
	}
	this code is replace by UpdateIncomingChannByNewCap
		// h239 channel
	pChannel = GetChannel(cmCapVideo,kRolePresentation,cmCapReceive);
	if( NULL != pChannel && m_rComMode.IsH239ToOpen() )
	{
		if( true != h239 && TRUE == pChannel->openedInSim )
			SendStartChnlCloseInd(pCSProt,pChannel);
		else if( true == h239 && TRUE != pChannel->openedInSim )
		{
			pChannel->channelIndex   = g_LocalChannelIndex++;
			pChannel->channelMcIndex = 0;
			SendIncomingChannelInd(pCSProt,pChannel,rCapSet);
		}
	}

	if( SIM_RECAP_MODE_AFTER == recapMode )
		SendCapabilitiesInd(rCapCommon,pCSProt);

	// send facility_ind after empty cap set
	if( SIM_RECAP_MODE_NONE != recapMode  &&  true != au  &&  true != vi  &&  true != fecc  &&  true != h239 )
	{
		if( 0 != strcmp(pszManufacturerName,"No change") )
		{
			CVendorInfo  rNewVendor(pszManufacturerName);
			if( !(rNewVendor == m_rVendor) )
			{
				m_rVendor = rNewVendor;
				SendFacilityInd(pCSProt);
			}
		}
	}

	POBJDELETE(pCSProt);*/
}

/////////////////////////////////////////////////////////////////////////////
//Reinvite from the GUI with new capset
void CEndpointSip::SendCsReInviteIndDynamic()
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendCsReInviteIndDynamic - SIP_CS_SIG_REINVITE_IND ");

	m_pCap->SetEncryption(FALSE);
	m_rComMode.Create(*m_pCap);

	BYTE*  pSdpStructBytes = NULL;
	DWORD  nSdpStructLen = CreateSdpStructBuffer(&pSdpStructBytes,*m_pCap,m_rComMode,FALSE /* first cap */, STATUS_OK, FALSE /* no mute values */);

	sipSdpAndHeadersSt* pSdpStruct = (sipSdpAndHeadersSt*)pSdpStructBytes;

	DWORD  indLen    = sizeof(mcIndInvite) + pSdpStruct->lenOfDynamicSection;
	BYTE*  pIndArray = new BYTE [indLen];
	memset(pIndArray,0,indLen);

	mcIndInvite* pIndInvite = (mcIndInvite*)pIndArray;

	pIndInvite->status = STATUS_OK;
	pIndInvite->bIsFocus = 1;
	memcpy(&(pIndInvite->sipSdpAndHeaders),pSdpStructBytes,
		sizeof(sipSdpAndHeadersBaseSt) + pSdpStruct->lenOfDynamicSection);

	PDELETEA(pSdpStructBytes);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;

	pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
	//pCSProt->AddPortDescriptionHeader(0,0,LOBBY_CONNECTION_ID,ePhysical_art_light);
	pCSProt->AddCSHeader(5,eMcms,eCentral_signaling);
	::FillCsProtocol(pCSProt, GetCSID(), SIP_CS_SIG_REINVITE_IND,
                     pIndArray, indLen, m_nCsCallIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndArray);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendCsInviteInd()
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendCsInviteInd - SIP_CS_SIG_INVITE_IND ");

	m_rComMode.Create(*m_pCap);

	BYTE*  pSdpStructBytes = NULL;
	DWORD  nSdpStructLen = CreateSdpStructBuffer(&pSdpStructBytes, *m_pCap, m_rComMode, TRUE /* first cap */, STATUS_OK, FALSE /* no mute values */);

	sipSdpAndHeadersSt* pSdpStruct = (sipSdpAndHeadersSt*)pSdpStructBytes;

	DWORD  indLen    = sizeof(mcIndInvite) + pSdpStruct->lenOfDynamicSection;
	BYTE*  pIndArray = new BYTE [indLen];
	memset(pIndArray, 0, indLen);

	if(strstr(GetUserAgent(),"OC/15") )//for ms 2013
	{
		PTRACE(eLevelInfoNormal,"CEndpointSip::SendCsInviteInd - ms 2013 UA ");
		pSdpStruct->msVideoRateRx = 4000;
	    pSdpStruct->msVideoRateTx = 4000;
	}

	mcIndInvite* pIndInvite = (mcIndInvite*)pIndArray;

	pIndInvite->status = STATUS_OK;
	pIndInvite->bIsFocus = 1;
	//pSdpStruct->cCname = ;
	//strncpy(pSdpStruct->cCname, "vnd.polycom.PlcmMaskCap:0010 GoodDay", CNAME_STRING_MAX_LEN +1);
	memcpy(&(pIndInvite->sipSdpAndHeaders),pSdpStructBytes,
		sizeof(sipSdpAndHeadersBaseSt) + pSdpStruct->lenOfDynamicSection);

	PDELETEA(pSdpStructBytes);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol;

	pCSProt->AddPortDescriptionHeader(0,0,LOBBY_CONNECTION_ID,ePhysical_art_light);
	pCSProt->AddCSHeader(5,eMcms,eCentral_signaling);

	::FillCsProtocol(pCSProt, GetCSID(), SIP_CS_SIG_INVITE_IND,
                     pIndArray, indLen, m_nCsCallIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndArray);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendCsInviteResponseInd( CMplMcmsProtocol* pMplProtocol, const BOOL isReinvite )
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendCsInviteResponseInd - SIP_CS_SIG_INVITE_RESPONSE_IND");

	// create highest capabilities and create communication mode
	CCapSet  rCommonCap(*m_pCap, *m_pConfCap,GetIsTIPEP());
	m_rComMode.Create(rCommonCap);

	// get request structure
	mcReqInvite*  pRequest = (mcReqInvite*)pMplProtocol->GetData();

	// check CFG file for rejection case of dial out call
	DWORD  status = ::GetEpSystemCfg()->GetRejectStatus();
	if( status >= SipCodesOk && status < SipCodesMultiChoice )
	{
		status = STATUS_OK;
	}
	// if it's not redirected call, reject it
		//else if( NULL == strstr(::GetEpSystemCfg()->GetRedirectionSipAddress(),m_IpH225RemoteStr) )
	else if( NULL == strstr(::GetEpSystemCfg()->GetRedirectionSipAddress(), GetIpStr()) )
	{
		PTRACE2(eLevelInfoNormal,"CEndpointSip::SendCsInviteResponseInd - reject call, ep name: ",m_szEpName);
		SetState(eEpStateDisconnecting);
	}
	else
	{
		status = STATUS_OK;
	}

	BOOL  isFirstCap = !isReinvite;

	BYTE*  pSdpStructBytes = NULL;
	DWORD  nSdpStructLen = CreateSdpStructBuffer(&pSdpStructBytes, rCommonCap, m_rComMode, isFirstCap, status, FALSE /* no mute values */);

	sipSdpAndHeadersSt* pSdpStruct = (sipSdpAndHeadersSt*)pSdpStructBytes;

	DWORD  indLen    = sizeof(mcIndInviteResponse) + pSdpStruct->lenOfDynamicSection;
	BYTE*  pIndArray = new BYTE [indLen];
	memset(pIndArray,0,indLen);

	mcIndInviteResponse* pIndInviteResp = (mcIndInviteResponse*)pIndArray;

	pIndInviteResp->status = status; // STATUS_OK; // SipCodesOk; //
	memcpy(&(pIndInviteResp->sipSdpAndHeaders),pSdpStructBytes,
		sizeof(sipSdpAndHeadersBaseSt) + pSdpStruct->lenOfDynamicSection);

	PDELETEA(pSdpStructBytes);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(), SIP_CS_SIG_INVITE_RESPONSE_IND,
                     pIndArray, indLen, m_nCsCallIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndArray);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendCsReInviteResponseInd( CMplMcmsProtocol* pMplProtocol )
{
	return SendCsInviteResponseInd(pMplProtocol,TRUE);
	/*
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendCsReInviteResponseInd - SIP_CS_SIG_INVITE_RESPONSE_IND ");

	// get request structure
	mcReqReInvite*  pRequest = (mcReqReInvite*)pMplProtocol->GetData();

	// this struct has static and dynamic parts
	WORD indLen = sizeof(mcIndInviteResponse) + pRequest->sipSdpAndHeaders.lenOfDynamicSection;
	if( pMplProtocol->getDataLen() < sizeof(mcReqReInvite) + pRequest->sipSdpAndHeaders.lenOfDynamicSection)
	{
		DBGPASSERT(pMplProtocol->getDataLen()+10000);
		DBGPASSERT(sizeof(mcReqReInvite)+10000);
		DBGPASSERT(pRequest->sipSdpAndHeaders.lenOfDynamicSection+10000);
	}

	BYTE*  pIndArray = new BYTE [indLen];
	mcIndInviteResponse* pInd = (mcIndInviteResponse*)pIndArray;

	pInd->status = STATUS_OK;
	memcpy(&(pInd->sipSdpAndHeaders),&(pRequest->sipSdpAndHeaders),
		sizeof(sipSdpAndHeadersSt)+pRequest->sipSdpAndHeaders.lenOfDynamicSection);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	FillCsProtocol(pCSProt,SIP_CS_SIG_INVITE_RESPONSE_IND,pIndArray,indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

	m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndArray);*/
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendCsInviteAckInd( CMplMcmsProtocol* pMplProtocol ) const
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendCsInviteAckInd - SIP_CS_SIG_INVITE_ACK_IND ");

	mcIndInviteAck* pInd = new mcIndInviteAck;
	memset(pInd, 0, sizeof(mcIndInviteAck));

	pInd->status = STATUS_OK;

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(), SIP_CS_SIG_INVITE_ACK_IND,
                     (BYTE*)pInd, sizeof(mcIndInviteAck));

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETE(pInd);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendCsByeInd( CMplMcmsProtocol* pMplProtocol ) const
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendCsByeInd - SIP_CS_SIG_BYE_IND ");

	DWORD indLen = sizeof(mcIndBye);
	mcIndBye* pInd = new mcIndBye;

	pInd->status = STATUS_OK;

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(), SIP_CS_SIG_BYE_IND,
                     (BYTE*)pInd, indLen);

	CSegment* pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	POBJDELETE(pInd);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendCsByeAckInd( CMplMcmsProtocol* pMplProtocol ) const
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendCsByeAckInd - SIP_CS_SIG_BYE_200_OK_IND ");

	DWORD indLen = sizeof(mcIndBye200Ok);
	mcIndBye200Ok* pInd = new mcIndBye200Ok;

	pInd->status = STATUS_OK;

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(), SIP_CS_SIG_BYE_200_OK_IND,
                     (BYTE*)pInd, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	POBJDELETE(pInd);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendCsReinviteIndDummy() const
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendCsReinviteIndDummy - SIP_CS_SIG_REINVITE_IND ");

	WORD   indLen    = sizeof(mcIndReInvite) + 1000;
	BYTE*  pIndArray = new BYTE [indLen];

	mcIndReInvite* pIndReInvite = (mcIndReInvite*)pIndArray;
	memset(pIndArray,0,indLen);

	pIndReInvite->status = STATUS_OK;
	pIndReInvite->bIsFocus = 1;

	SipBuildDummySdpAndHeadersForSimulation(&(pIndReInvite->sipSdpAndHeaders));

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol;

	pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
	pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);
//	pCSProt->AddPortDescriptionHeader(0,0,LOBBY_CONNECTION_ID,ePhysical_art_light);
//	pCSProt->AddCSHeader(5,eMcms,eCentral_signaling);

	::FillCsProtocol(pCSProt, GetCSID(),
                     SIP_CS_SIG_REINVITE_IND, pIndArray, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndArray);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendCsMuteReinviteInd(WORD wAudioMuteByPort, WORD wVideoMuteByPort,
						char *szAudioMuteByDirection, char *szVideoMuteByDirection,
						char *szAudioMuteByInactive, char *szVideoMuteByInactive)
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendCsMuteReinviteInd - SIP_CS_SIG_REINVITE_IND ");

	m_rComMode.Create(*m_pCap);

	cmCapDirection eDirection;

	BYTE*  pSdpStructBytes = NULL;
	DWORD  nSdpStructLen = CreateSdpStructBuffer(&pSdpStructBytes,*m_pCap,m_rComMode,FALSE /* first cap */
												, STATUS_OK, TRUE /* no mute values */
												, wAudioMuteByPort, wVideoMuteByPort, szAudioMuteByDirection
												, szVideoMuteByDirection, szAudioMuteByInactive, szVideoMuteByInactive);

	sipSdpAndHeadersSt* pSdpStruct = (sipSdpAndHeadersSt*)pSdpStructBytes;

	DWORD  indLen    = sizeof(mcIndReInvite) + pSdpStruct->lenOfDynamicSection;
	BYTE*  pIndArray = new BYTE [indLen];
	memset(pIndArray, 0, indLen);

	mcIndReInvite* pIndReInvite = (mcIndReInvite*)pIndArray;

	pIndReInvite->status = STATUS_OK;
	pIndReInvite->bIsFocus = 1;

	memcpy(&(pIndReInvite->sipSdpAndHeaders),pSdpStructBytes,
		sizeof(sipSdpAndHeadersBaseSt) + pSdpStruct->lenOfDynamicSection);

	PDELETEA(pSdpStructBytes);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol;

	pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
	pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

	::FillCsProtocol(pCSProt, GetCSID(),
                     SIP_CS_SIG_REINVITE_IND, pIndArray, indLen, m_nCsCallIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndArray);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendMuteIndication() const
{
	SendCsReinviteIndDummy();
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendUnmuteIndication() const
{
	SendCsReinviteIndDummy();
}

/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointSip::IsFeccCall() const
{
	if( m_rComMode.IsFeccToOpen() )
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendFeccTokenRequestIndication() const
{
	PTRACE2(eLevelError,"CEndpointSip::SendFeccTokenRequestIndication, Name - ", m_szEpName );

	CSegment  rMsgSeg;

	rMsgSeg << (DWORD)IP_RTP_FECC_TOKEN_IND;

	rMsgSeg << m_rAudioBoard.GetBoardId()
			<< m_rAudioBoard.GetSubBoardId()
			<< m_rAudioBoard.GetUnitId();

	rMsgSeg << m_confID
			<< m_partyID
			<< m_connectionID;

	TRtpFeccTokenRequestInd  rTokenStruct;
	memset(&rTokenStruct,0,sizeof(TRtpFeccTokenRequestInd));
	rTokenStruct.unChannelType      = cmCapData;
	rTokenStruct.unChannelDirection = cmCapReceive;
	rTokenStruct.unTokenOpcode      = (DWORD)kTokenRequest;

	rMsgSeg.Put((BYTE*)(&rTokenStruct),sizeof(TRtpFeccTokenRequestInd));

	::SendAudioMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendFeccTokenReleaseIndication() const
{
	PTRACE2(eLevelError,"CEndpointSip::SendFeccTokenReleaseIndication, Name - ", m_szEpName );

	CSegment  rMsgSeg;

	rMsgSeg << (DWORD)IP_RTP_FECC_TOKEN_IND;

	rMsgSeg << m_rAudioBoard.GetBoardId()
			<< m_rAudioBoard.GetSubBoardId()
			<< m_rAudioBoard.GetUnitId();

	rMsgSeg << m_confID
			<< m_partyID
			<< m_connectionID;

	TRtpFeccTokenRequestInd  rTokenStruct;
	memset(&rTokenStruct,0,sizeof(TRtpFeccTokenRequestInd));
	rTokenStruct.unChannelType      = cmCapData;
	rTokenStruct.unChannelDirection = cmCapReceive;
	rTokenStruct.unTokenOpcode      = (DWORD)kTokenRelease;

	rMsgSeg.Put((BYTE*)(&rTokenStruct),sizeof(TRtpFeccTokenRequestInd));

	::SendAudioMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendFeccKeyRequestIndication(const char* pszDtmf) const
{
        PTRACE2(eLevelError,"CEndpointSip::SendFeccKeyRequestIndication, Name - ", m_szEpName );

        CSegment  rMsgSeg;

        rMsgSeg << (DWORD)IP_RTP_FECC_TOKEN_IND;

        rMsgSeg << m_rAudioBoard.GetBoardId()
                        << m_rAudioBoard.GetSubBoardId()
                        << m_rAudioBoard.GetUnitId();

        rMsgSeg << m_confID
                        << m_partyID
                        << m_connectionID;

        TRtpFeccTokenRequestInd  rTokenStruct;
        memset(&rTokenStruct,0,sizeof(TRtpFeccTokenRequestInd));
        rTokenStruct.unChannelType      = cmCapData;
        rTokenStruct.unChannelDirection = cmCapReceive;
        rTokenStruct.unTokenOpcode      = (DWORD)kTokenRequest;

        rMsgSeg.Put((BYTE*)(&rTokenStruct),sizeof(TRtpFeccTokenRequestInd));

        ::SendAudioMessageToGideonSimApp(rMsgSeg);
}


/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendH239TokenRequestIndication()
{
	PTRACE2(eLevelError,"CEndpointSip::SendH239TokenRequestIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendH239TokenReleaseIndication()
{
	PTRACE2(eLevelError,"CEndpointSip::SendH239TokenReleaseIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendLprModeChangeRequestIndication(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout)
{
	PTRACE2(eLevelInfoNormal, "CEndpointSip::SendLprModeChangeRequestIndication, Name - ", m_szEpName );
	DWORD  indLen    = sizeof(mcIndInfo) + sizeof(mcIndSipLPRModeChange);// + 1000; //+1000
	BYTE*  pIndArray = new BYTE [indLen];
	//memset(pIndArray,0,sizeof(mcIndInfo));
	mcIndInfo* mcIndStr = (mcIndInfo*)pIndArray;

	mcIndSipLPRModeChange* pLprModeChangeInd = (mcIndSipLPRModeChange*)(&(mcIndStr->buff));
	mcIndStr->subOpcode = LprChangeMode;
	mcIndStr->dynamicLen = (APIU32)(sizeof(mcIndSipLPRModeChange));

	pLprModeChangeInd->lossProtection	 = lossProtection;
	pLprModeChangeInd->mtbf 		     = mtbf;
	pLprModeChangeInd->congestionCeiling = congestionCeiling;
	pLprModeChangeInd->fill				 = fill;
	pLprModeChangeInd->modeTimeout		 = modeTimeout;

	// send the struct to CS simulation
		// ================================
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;

	pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_res_none);
	pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

	//::FillCsProtocol(pCSProt,SImcIndFlowControl;P_CS_SIG_INFO_IND,pIndArray,indLen,m_nCsCallIndex);
	::FillCsProtocol(pCSProt, GetCSID(), SIP_CS_SIG_INFO_IND,
                     pIndArray, indLen, m_nCsCallIndex);
	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

	CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
       api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndArray);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CEndpointSip::SipChangeSdpForSimulation(sipSdpAndHeadersSt* sdpAndHeaders)
{
//	sdpAndHeaders->mediaIps[GetSdpArrayIndex(kSdpVideo)].transAddr.port = 0;// mute video

/*
	if ( sdpAndHeaders->lenOfDynamicSection > sizeof(ctCapabilitiesBasicStruct) )
	{
		ctCapabilitiesStruct* pCapabilities = (ctCapabilitiesStruct*)sdpAndHeaders->capsAndHeaders;
		capBuffer* pCapBuffer = &pCapabilities->caps;
		BYTE*	pTemp = (BYTE*)pCapBuffer;

		for (int i = 0 ; i < pCapabilities->numberOfCaps; i++)
		{
			switch(pCapBuffer->capTypeCode)
			{
				case eSiren14_48kCapCode:
				{
					siren14_48kCapStruct *pCapStruct = (siren14_48kCapStruct *)pCapBuffer->dataCap;
					pCapStruct->maxValue	= 20;
					pCapStruct->minValue	= 0;
					pCapBuffer->capTypeCode = eG711Ulaw64kCapCode;
					break;
				}
				case eH261CapCode:
				{
					h261CapStruct *pStruct = (h261CapStruct *)pCapBuffer->dataCap;
					break;
				}
				case eH263CapCode:
				{
					h263CapStruct *pStruct = (h263CapStruct *)pCapBuffer->dataCap;
					break;
				}
				case eH264CapCode:
				{
					pCapBuffer->capTypeCode	= eH261CapCode;// convert the H264 to H261
					h261CapStruct *pStruct	= (h261CapStruct *)pCapBuffer->dataCap;
					pStruct->maxBitRate		= 4240;
					pStruct->qcifMPI		= 1;
					pStruct->cifMPI			= 1;
//					h264CapStruct *pCapStruct = (h264CapStruct *)pCapBuffer->dataCap;
//					pCapStruct->profileValue		   = 64;//H264_Profile_BaseLine;
//					pCapStruct->levelValue			   = 29;
//					pCapStruct->customMaxMbpsValue     = 20;
//					pCapStruct->customMaxFsValue       = -1;
//					pCapStruct->customMaxDpbValue      = -1;
//					pCapStruct->customMaxBrAndCpbValue = -1;
//					pCapStruct->maxBitRate 			   = 4240;
					break;
				}
			}
			pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)pTemp;
		}
	}
*/
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CEndpointSip::SipBuildMutedDummySdpAndHeadersForSimulation(sipSdpAndHeadersSt* sdpAndHeaders,
										WORD wAudioMuteByPort, WORD wVideoMuteByPort,
										char *szAudioMuteByDirection, char *szVideoMuteByDirection,
										char *szAudioMuteByInactive, char *szVideoMuteByInactive) const
{
	dummySIPSdpAndHdrsSt *pSdpAndHdrs = (dummySIPSdpAndHdrsSt *)sdpAndHeaders;

	pSdpAndHdrs->callRate = 3840;
	pSdpAndHdrs->sipMediaLinesOffset 	= 0;
	pSdpAndHdrs->sipMediaLinesLength	= sizeof(dummySipCap);
	pSdpAndHdrs->sipHeadersOffset   	= sizeof(dummySipCap);
	pSdpAndHdrs->sipHeadersLength 		= sizeof(dummySipHeaders);
	pSdpAndHdrs->sipIceOffset = 0;
	pSdpAndHdrs->sipIceLength = 0;
	pSdpAndHdrs->lenOfDynamicSection 	= sizeof(dummySipCap) + sizeof(dummySipHeaders);

	pSdpAndHdrs->cap.entry1_numberOfMediaLines = 2;
	pSdpAndHdrs->cap.entry1_lenOfDynamicSection = (char*)&pSdpAndHdrs->headers - (char*)&pSdpAndHdrs->cap.ml1_mediaIp;

	//Audio channel
	pSdpAndHdrs->cap.ml1_mediaIp.unionProps.unionType = eIpVersion4;
	pSdpAndHdrs->cap.ml1_mediaIp.unionProps.unionSize = sizeof(ipAddressIf);
	pSdpAndHdrs->cap.ml1_mediaIp.transAddr.ipVersion = eIpVersion4;
	pSdpAndHdrs->cap.ml1_mediaIp.transAddr.addr.v4.ip = 0x46be16ac;
	pSdpAndHdrs->cap.ml1_mediaIp.transAddr.port = wAudioMuteByPort;
	pSdpAndHdrs->cap.ml1_mediaIp.transAddr.distribution = 0;//Not used
	pSdpAndHdrs->cap.ml1_mediaIp.transAddr.transportType = eTransportTypeUdp;
	pSdpAndHdrs->cap.ml1_rtcpPort = 1 + pSdpAndHdrs->cap.ml1_mediaIp.transAddr.port;
	pSdpAndHdrs->cap.ml1_index = 0;
	pSdpAndHdrs->cap.ml1_type = eMediaLineTypeAudio;
	pSdpAndHdrs->cap.ml1_subType = eMediaLineSubTypeRtpAvp;
	pSdpAndHdrs->cap.ml1_content = 0;
	pSdpAndHdrs->cap.ml1_label[31] = 0;
	pSdpAndHdrs->cap.ml1_numberOfCaps = 1;
	pSdpAndHdrs->cap.ml1_lenOfDynamicSection = (char*)&pSdpAndHdrs->cap.ml2_mediaIp - (char*)&pSdpAndHdrs->cap.capTypeCode1;

	//Video channel
	pSdpAndHdrs->cap.ml2_mediaIp.unionProps.unionType = eIpVersion4;
	pSdpAndHdrs->cap.ml2_mediaIp.unionProps.unionSize = sizeof(ipAddressIf);
	pSdpAndHdrs->cap.ml2_mediaIp.transAddr.ipVersion = eIpVersion4;
	pSdpAndHdrs->cap.ml2_mediaIp.transAddr.addr.v4.ip = 0x46be16ac;
	pSdpAndHdrs->cap.ml2_mediaIp.transAddr.port = wVideoMuteByPort;
	pSdpAndHdrs->cap.ml2_mediaIp.transAddr.distribution = 0;//Not used
	pSdpAndHdrs->cap.ml2_mediaIp.transAddr.transportType = eTransportTypeUdp;
	pSdpAndHdrs->cap.ml2_rtcpPort = 1 + pSdpAndHdrs->cap.ml2_mediaIp.transAddr.port;
	pSdpAndHdrs->cap.ml2_index = 1;
	pSdpAndHdrs->cap.ml2_type = eMediaLineTypeVideo;
	pSdpAndHdrs->cap.ml2_subType = eMediaLineSubTypeRtpAvp;
	pSdpAndHdrs->cap.ml2_content = 0;
	pSdpAndHdrs->cap.ml2_label[31] = 0;
	pSdpAndHdrs->cap.ml2_numberOfCaps = 1;
	pSdpAndHdrs->cap.ml2_lenOfDynamicSection = (char*)&pSdpAndHdrs->headers - (char*)&pSdpAndHdrs->cap.capTypeCode3;

	//Data channel (Not implemented yet)

	//G711 cap
	pSdpAndHdrs->cap.capTypeCode1						= eG711Ulaw64kCapCode; // 2;
	pSdpAndHdrs->cap.sipPayloadType1					= 0x0;
	pSdpAndHdrs->cap.capLength1							= sizeof(audioCapStructBase); // 0xc;
	if(strcmp(szAudioMuteByDirection, "receive") == 0)
		pSdpAndHdrs->cap.g711u.header.direction				= 0x1;
	else if(strcmp(szAudioMuteByDirection, "transmit") == 0)
		pSdpAndHdrs->cap.g711u.header.direction				= 0x2;
	else if(strcmp(szAudioMuteByInactive, "inactive") == 0)
		pSdpAndHdrs->cap.g711u.header.direction				= 0x0;
	else
		pSdpAndHdrs->cap.g711u.header.direction				= 0x3;

	pSdpAndHdrs->cap.g711u.header.type					= 0x1;
	pSdpAndHdrs->cap.g711u.header.roleLabel				= 0x0;
	pSdpAndHdrs->cap.g711u.header.capTypeCode			= eG711Ulaw64kCapCode; // 0x2;
	pSdpAndHdrs->cap.g711u.maxValue						= 0x1e;
	pSdpAndHdrs->cap.g711u.minValue						= 0xa;

	//H263 cap
	pSdpAndHdrs->cap.capTypeCode3						= eH263CapCode; // 0x18;
	pSdpAndHdrs->cap.sipPayloadType3					= 0x22;
	pSdpAndHdrs->cap.capLength3							= sizeof(h263CapStruct); // 0x28;
	if(strcmp(szVideoMuteByDirection, "receive") == 0)
		pSdpAndHdrs->cap.h263.header.direction				= 0x1;
	else if(strcmp(szVideoMuteByDirection, "transmit") == 0)
		pSdpAndHdrs->cap.h263.header.direction				= 0x2;
	else if(strcmp(szVideoMuteByInactive, "inactive") == 0)
		pSdpAndHdrs->cap.h263.header.direction				= 0x0;
	else
		pSdpAndHdrs->cap.h263.header.direction				= 0x3;

	pSdpAndHdrs->cap.h263.header.type					= 0x2;
	pSdpAndHdrs->cap.h263.header.roleLabel				= 0x0;
	pSdpAndHdrs->cap.h263.header.capTypeCode			= eH263CapCode; // 0x18;
	pSdpAndHdrs->cap.h263.maxBitRate					= 3200; // 0x1e0;
	pSdpAndHdrs->cap.h263.hrd_B							= 0;
	pSdpAndHdrs->cap.h263.bppMaxKb						= 0;
	pSdpAndHdrs->cap.h263.slowSqcifMPI					= 0;
	pSdpAndHdrs->cap.h263.slowQcifMPI					= 0;
	pSdpAndHdrs->cap.h263.slowCifMPI					= 0;
	pSdpAndHdrs->cap.h263.slowCif4MPI					= 0;
	pSdpAndHdrs->cap.h263.slowCif16MPI					= 0;
	pSdpAndHdrs->cap.h263.sqcifMPI						= (APIS8)0xff;
	pSdpAndHdrs->cap.h263.qcifMPI						= (APIS8)1; // 0xff;
	pSdpAndHdrs->cap.h263.cifMPI						= (APIS8)1; // 0xff;
	pSdpAndHdrs->cap.h263.cif4MPI						= (APIS8)0xff;
	pSdpAndHdrs->cap.h263.cif16MPI						= (APIS8)0xff;
	pSdpAndHdrs->cap.h263.filler						= 0;
	pSdpAndHdrs->cap.h263.capBoolMask					= 0;
	pSdpAndHdrs->cap.h263.annexesMask.fds_bits[0]		= 0;
	pSdpAndHdrs->cap.h263.annexesPtr[0]					= 0;

	//SIP headers
	pSdpAndHdrs->headers.sipMsgHeaders.numOfHeaders			= 10;  // 0xa;
	pSdpAndHdrs->headers.sipMsgHeaders.headersListLength	= 256; // 0xd5;

	pSdpAndHdrs->headers.elem[0].eHeaderField	= kToDisplay;   // 0x1;
	pSdpAndHdrs->headers.elem[1].eHeaderField	= kTo;          // 0x2;
	pSdpAndHdrs->headers.elem[2].eHeaderField	= kFromDisplay; // 0x4;
	pSdpAndHdrs->headers.elem[3].eHeaderField	= kFrom;        // 0x5;
	pSdpAndHdrs->headers.elem[4].eHeaderField	= kContactDisplay; // 0x7;
	pSdpAndHdrs->headers.elem[5].eHeaderField	= kContact;     // 0x8;
	pSdpAndHdrs->headers.elem[6].eHeaderField	= kReqLine;     // 0xa;
	pSdpAndHdrs->headers.elem[7].eHeaderField	= kCallId;      // 0x13;
	pSdpAndHdrs->headers.elem[8].eHeaderField	= kVia;         // 0x9;
	pSdpAndHdrs->headers.elem[9].eHeaderField	= kUserAgent;   // 0xb;

	char* pszArray[10];
	for( int i=0; i<10; i++ )
	{
		pszArray[i] = new char[256];
		memset(pszArray[i],'\0', 256);
	}
	strcpy(pszArray[1], "conf1@172.13.12.12");  // kTo  "avner1@172.22.190.70");
		//sprintf(pszArray[3],"%s@%s",m_szEpName,m_IpH225RemoteStr); // kFrom  " partys IP avner2@172.22.169.70");
	if( strlen(m_szConfName) )
	{
		std::string fullConfName = GetConfName();
		int index = fullConfName.find("@");
		std::string partyName;
		if (index == -1)
			partyName = fullConfName;
		else
			partyName = fullConfName.substr(0, index);

		sprintf(pszArray[5],"%s@", partyName.c_str());
	}
	else
		//strcat(pszArray[5],m_IpH225RemoteStr);
		strcat(pszArray[5], GetIpStr());

//	sprintf(pszArray[5],"confN2@%s",m_IpH225RemoteStr); // kContact  " partys IP avner2@172.22.169.70");

	if( strlen(m_szConfName) )
	{
		std::string fullConfName = GetConfName();
		int index = fullConfName.find("@");
		std::string partyName;
		if (index == -1)
			partyName = fullConfName;
		else
			partyName = fullConfName.substr(0, index);

		sprintf(pszArray[6],"%s@", partyName.c_str());
	}
	else
	{
		strcat(pszArray[6],"172.13.12.12");
	}

//	strcpy(pszArray[6],"conf1@172.13.12.12");  // kReqLine  "avner1@172.22.190.70");
		//sprintf(pszArray[7],"1703e18-46a916ac-13ce-4280b6b1-4f76a2d-47bb@%s",m_IpH225RemoteStr); // kCallId // 172.22.169.70");

	sprintf(pszArray[7],"1703e18-46a916ac-13ce-4280b6b1-4f76a2d-47bb@%s", GetIpStr()); // kCallId // 172.22.169.70");

		//sprintf(pszArray[8],"%s:%d",m_IpH225RemoteStr, 5070);   // kVia // "172.22.169.70:5070");
	sprintf(pszArray[8],"%s:%d",GetIpStr(), 5070);   // kVia // "172.22.169.70:5070");

	int position = 0;
	for( int i=0; i<10; i++ )
	{
		pSdpAndHdrs->headers.elem[i].flags	   = 0x0;
		pSdpAndHdrs->headers.elem[i].position  = position;
		strcpy(&pSdpAndHdrs->headers.sList[position],pszArray[i]);
		position += strlen(pszArray[i]) + 1;
	}

	for( int i=0; i<10; i++ )
		DEALLOCBUFFER(pszArray[i]);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CEndpointSip::SipBuildDummySdpAndHeadersForSimulation(sipSdpAndHeadersSt* sdpAndHeaders) const
{
	dummySIPSdpAndHdrsSt *pSdpAndHdrs = (dummySIPSdpAndHdrsSt *)sdpAndHeaders;

	mcXmlTransportAddress audioIp;
	mcXmlTransportAddress videoIp;
	mcXmlTransportAddress dataIp;
	mcXmlTransportAddress bfcpIp;

	if (GetIpVersion() == eIpVersion4)
	{
		//Audio channel
		audioIp.transAddr.ipVersion = eIpVersion4;
		audioIp.transAddr.addr.v4.ip = 0x46be16ac; //172.22.190.70
		audioIp.transAddr.port = (m_isMuted == FALSE) ? 0xc020 : 0;
		audioIp.transAddr.distribution = 0;//Not used
		audioIp.transAddr.transportType = eTransportTypeUdp;

		//Video channel
		videoIp.transAddr.ipVersion = eIpVersion4;
		videoIp.transAddr.addr.v4.ip = 0x46be16ac; //172.22.190.70
		videoIp.transAddr.port = 0xaac2;
		videoIp.transAddr.distribution = 0;//Not used
		videoIp.transAddr.transportType = eTransportTypeUdp;

		//Data channel (Not implemented yet)
		dataIp.transAddr.ipVersion = eIpVersion4;
		dataIp.transAddr.addr.v4.ip = 0;
		dataIp.transAddr.port = 0xaac2;
		dataIp.transAddr.distribution = 0;
		dataIp.transAddr.transportType = eTransportTypeUdp;

		//Bfcp channel
		bfcpIp.transAddr.ipVersion = eIpVersion4;
		bfcpIp.transAddr.addr.v4.ip = 0;
		bfcpIp.transAddr.port = 0xaac2;
		bfcpIp.transAddr.distribution = 0;
		bfcpIp.transAddr.transportType = eTransportTypeUdp;
	}
	else if (GetIpVersion() == eIpVersion6)
	{
		std::string ipAddress;
		std::string prefix;
		enScopeId remoteScopeid;
		char ipStr[128];
		memset (&ipStr, '\0', 128);
		remoteScopeid = getScopeId(ipToString(m_H225RemoteIpAddress, ipStr, 1));

		if (remoteScopeid == eScopeIdSite)
		{
			prefix = "fec0:";
		}
		else if (remoteScopeid == eScopeIdGlobal)
		{
			prefix = "2001:";
		}
		else if (remoteScopeid == eScopeIdLink)
		{
			prefix = "fe80:";
		}

		ipAddress = prefix;
		ipAddress += "db8:0:1:172:22:190:70";
		char* ipAddressStr = (char*)ipAddress.c_str();

		//Audio channel
		audioIp.transAddr.ipVersion = eIpVersion6;
		audioIp.unionProps.unionType = eIpVersion6;
		stringToIp(&audioIp.transAddr, ipAddressStr);
		audioIp.transAddr.addr.v6.scopeId = remoteScopeid;
		audioIp.transAddr.port = (m_isMuted == FALSE) ? 0xc020 : 0;
		audioIp.transAddr.distribution = eDistributionUnicast;//Not used
		audioIp.transAddr.transportType = eTransportTypeUdp;

		//Video channel
		videoIp.transAddr.ipVersion = eIpVersion6;
		videoIp.unionProps.unionType = eIpVersion6;
		stringToIp(&videoIp.transAddr, ipAddressStr);
		videoIp.transAddr.addr.v6.scopeId = remoteScopeid;
		videoIp.transAddr.port = 0xaac2;
		videoIp.transAddr.distribution = eDistributionUnicast;//Not used
		videoIp.transAddr.transportType = eTransportTypeUdp;

		//Data channel (Not implemented yet)
		dataIp.transAddr.ipVersion = eIpVersion6;
		dataIp.unionProps.unionType = eIpVersion6;
		stringToIp(&dataIp.transAddr, ipAddressStr);
		dataIp.transAddr.addr.v6.scopeId = remoteScopeid;
		dataIp.transAddr.port = 0xaac2;
		dataIp.transAddr.distribution = eDistributionUnicast;//Not used
		dataIp.transAddr.transportType = eTransportTypeUdp;

		//Bfcp channel
		bfcpIp.transAddr.ipVersion = eIpVersion6;
		bfcpIp.unionProps.unionType = eIpVersion6;
		stringToIp(&bfcpIp.transAddr, ipAddressStr);
		bfcpIp.transAddr.addr.v6.scopeId = remoteScopeid;
		bfcpIp.transAddr.port = 0xaac2;
		bfcpIp.transAddr.distribution = eDistributionUnicast;//Not used
		bfcpIp.transAddr.transportType = eTransportTypeUdp;
	}


	audioIp.unionProps.unionType = eIpVersion4;
	audioIp.unionProps.unionSize = sizeof(ipAddressIf);
	videoIp.unionProps.unionType = eIpVersion4;
	videoIp.unionProps.unionSize = sizeof(ipAddressIf);
	dataIp.unionProps.unionType = eIpVersion4;
	dataIp.unionProps.unionSize = sizeof(ipAddressIf);
	bfcpIp.unionProps.unionType = eIpVersion4;
	bfcpIp.unionProps.unionSize = sizeof(ipAddressIf);

	pSdpAndHdrs->callRate = 3840;
	pSdpAndHdrs->sipMediaLinesOffset = 0;
	pSdpAndHdrs->sipMediaLinesLength = sizeof(pSdpAndHdrs->cap);
	pSdpAndHdrs->sipHeadersOffset = sizeof(pSdpAndHdrs->cap);
	pSdpAndHdrs->sipHeadersLength = sizeof(pSdpAndHdrs->headers);
	pSdpAndHdrs->sipIceOffset = 0;
	pSdpAndHdrs->sipIceLength = 0;
	pSdpAndHdrs->lenOfDynamicSection = pSdpAndHdrs->sipMediaLinesLength + pSdpAndHdrs->sipHeadersLength;

	pSdpAndHdrs->cap.entry1_numberOfMediaLines = 2;
	pSdpAndHdrs->cap.entry1_lenOfDynamicSection = (char*)&pSdpAndHdrs->headers - (char*)&pSdpAndHdrs->cap.ml1_mediaIp;

	//Audio channel
	pSdpAndHdrs->cap.ml1_mediaIp = audioIp;
	pSdpAndHdrs->cap.ml1_rtcpPort = audioIp.transAddr.port;
	pSdpAndHdrs->cap.ml1_index = 0;
	pSdpAndHdrs->cap.ml1_type = eMediaLineTypeAudio;
	pSdpAndHdrs->cap.ml1_subType = eMediaLineSubTypeRtpAvp;
	pSdpAndHdrs->cap.ml1_content = 0;
	pSdpAndHdrs->cap.ml1_label[31] = 0;
	pSdpAndHdrs->cap.ml1_numberOfCaps = 1;
	pSdpAndHdrs->cap.ml1_lenOfDynamicSection = (char*)&pSdpAndHdrs->cap.ml2_mediaIp - (char*)&pSdpAndHdrs->cap.capTypeCode1;

	//Video channel
	pSdpAndHdrs->cap.ml1_mediaIp = videoIp;
	pSdpAndHdrs->cap.ml2_rtcpPort = videoIp.transAddr.port;
	pSdpAndHdrs->cap.ml2_index = 1;
	pSdpAndHdrs->cap.ml2_type = eMediaLineTypeVideo;
	pSdpAndHdrs->cap.ml2_subType = eMediaLineSubTypeRtpAvp;
	pSdpAndHdrs->cap.ml2_content = 0;
	pSdpAndHdrs->cap.ml2_label[31] = 0;
	pSdpAndHdrs->cap.ml2_numberOfCaps = 1;
	pSdpAndHdrs->cap.ml2_lenOfDynamicSection = (char*)&pSdpAndHdrs->headers - (char*)&pSdpAndHdrs->cap.capTypeCode3;

	//Data channel (Not implemented yet)

	//G711 cap
	pSdpAndHdrs->cap.capTypeCode1						= eG711Ulaw64kCapCode; // 2;
	pSdpAndHdrs->cap.sipPayloadType1					= 0x0;
	pSdpAndHdrs->cap.capLength1							= sizeof(audioCapStructBase); // 0xc;
	pSdpAndHdrs->cap.g711u.header.direction				= 0x3;
	pSdpAndHdrs->cap.g711u.header.type					= 0x1;
	pSdpAndHdrs->cap.g711u.header.roleLabel				= 0x0;
	pSdpAndHdrs->cap.g711u.header.capTypeCode			= eG711Ulaw64kCapCode; // 0x2;
	pSdpAndHdrs->cap.g711u.maxValue						= 0x1e;
	pSdpAndHdrs->cap.g711u.minValue						= 0xa;

	//H263 cap
	pSdpAndHdrs->cap.capTypeCode3						= eH263CapCode; // 0x18;
	pSdpAndHdrs->cap.sipPayloadType3					= 0x22;
	pSdpAndHdrs->cap.capLength3							= sizeof(h263CapStruct); // 0x28;
	pSdpAndHdrs->cap.h263.header.direction				= 0x3;
	pSdpAndHdrs->cap.h263.header.type					= 0x2;
	pSdpAndHdrs->cap.h263.header.roleLabel				= 0x0;
	pSdpAndHdrs->cap.h263.header.capTypeCode			= eH263CapCode; // 0x18;
	pSdpAndHdrs->cap.h263.maxBitRate					= 3200; // 0x1e0;
	pSdpAndHdrs->cap.h263.hrd_B							= 0;
	pSdpAndHdrs->cap.h263.bppMaxKb						= 0;
	pSdpAndHdrs->cap.h263.slowSqcifMPI					= 0;
	pSdpAndHdrs->cap.h263.slowQcifMPI					= 0;
	pSdpAndHdrs->cap.h263.slowCifMPI					= 0;
	pSdpAndHdrs->cap.h263.slowCif4MPI					= 0;
	pSdpAndHdrs->cap.h263.slowCif16MPI					= 0;
	pSdpAndHdrs->cap.h263.sqcifMPI						= (APIS8)0xff;
	pSdpAndHdrs->cap.h263.qcifMPI						= (APIS8)1; // 0xff;
	pSdpAndHdrs->cap.h263.cifMPI						= (APIS8)1; // 0xff;
	pSdpAndHdrs->cap.h263.cif4MPI						= (APIS8)0xff;
	pSdpAndHdrs->cap.h263.cif16MPI						= (APIS8)0xff;
	pSdpAndHdrs->cap.h263.filler						= 0;
	pSdpAndHdrs->cap.h263.capBoolMask					= 0;
	pSdpAndHdrs->cap.h263.annexesMask.fds_bits[0]		= 0;
	pSdpAndHdrs->cap.h263.annexesPtr[0]					= 0;


	//SIP headers
	pSdpAndHdrs->headers.sipMsgHeaders.numOfHeaders			= 10;  // 0xa;
	pSdpAndHdrs->headers.sipMsgHeaders.headersListLength	= 256; // 0xd5;

	pSdpAndHdrs->headers.elem[0].eHeaderField	= kToDisplay;   // 0x1;
	pSdpAndHdrs->headers.elem[1].eHeaderField	= kTo;          // 0x2;
	pSdpAndHdrs->headers.elem[2].eHeaderField	= kFromDisplay; // 0x4;
	pSdpAndHdrs->headers.elem[3].eHeaderField	= kFrom;        // 0x5;
	pSdpAndHdrs->headers.elem[4].eHeaderField	= kContactDisplay; // 0x7;
	pSdpAndHdrs->headers.elem[5].eHeaderField	= kContact;     // 0x8;
	pSdpAndHdrs->headers.elem[6].eHeaderField	= kReqLine;     // 0xa;
	pSdpAndHdrs->headers.elem[7].eHeaderField	= kCallId;      // 0x13;
	pSdpAndHdrs->headers.elem[8].eHeaderField	= kVia;         // 0x9;
	pSdpAndHdrs->headers.elem[9].eHeaderField	= kUserAgent;   // 0xb;

	char* pszArray[10];
	for( int i=0; i<10; i++ )
	{
		pszArray[i] = new char[256];
		memset(pszArray[i],'\0', 256);
	}


	ALLOCBUFFER(pszIpStr, 128);
	GetIpStr(pszIpStr);

  const char* CSIpAddress = NULL;
  switch (GetIpVersion())
  {
    case eIpVersion4:
      CSIpAddress = ::GetEpSystemCfg()->GetCSIpAddress();
      break;
    case eIpVersion6:
      CSIpAddress = ::GetEpSystemCfg()->GetCSIpV6Address();
      break;
    default:
      PASSERT_AND_RETURN(1);
  }

	//kTo
	if (strcmp(CSIpAddress, "0.0.0.0") == 0)
	{
		std::string fullConfName = GetConfName();
		int index = fullConfName.find("@");
		std::string partyName;
		if (index == -1)
			partyName = fullConfName;
		else
			partyName = fullConfName.substr(0, index);

		sprintf(pszArray[1],"%s@172.22.192.26", partyName.c_str());
	}
	else
	{
		std::string fullConfName = GetConfName();
		int index = fullConfName.find("@");
		std::string partyName;
		if (index == -1)
			partyName = fullConfName;
		else
			partyName = fullConfName.substr(0, index);

		sprintf(pszArray[1],"%s@%s", partyName.c_str(), CSIpAddress);
	}


	//kFrom
	sprintf(pszArray[3], "%s@%s", m_szEpName, pszIpStr);

	// kContact
	sprintf(pszArray[5], "%s", pszIpStr);


	//kReqLine
	if( strlen(m_szConfName) )
	{
		std::string fullConfName = GetConfName();
		int index = fullConfName.find("@");
		std::string partyName;
		if (index == -1)
			partyName = fullConfName;
		else
			partyName = fullConfName.substr(0, index);

		sprintf(pszArray[6],"%s@%s", partyName.c_str(), CSIpAddress);
	}
	else
	{
		strcat(pszArray[6], CSIpAddress);
	}

	//kCallId
	sprintf(pszArray[7],"1703e18-46a916ac-13ce-4280b6b1-4f76a2d-47bb@%s", pszIpStr);

	//kVia
	sprintf(pszArray[8],"%s:%d", pszIpStr, 5070);

	DEALLOCBUFFER(pszIpStr);

	int position = 0;
	for( int i=0; i<10; i++ )
	{
		pSdpAndHdrs->headers.elem[i].flags	   = 0x0;
		pSdpAndHdrs->headers.elem[i].position  = position;
		strcpy(&pSdpAndHdrs->headers.sList[position],pszArray[i]);
		position += strlen(pszArray[i]) + 1;
	}

	for( int i=0; i<10; i++ )
		DEALLOCBUFFER(pszArray[i]);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CEndpointSip::CreateSdpStructBuffer(BYTE** ppCapsAndHeadersBuffer,
				const CCapSet& rCapSet, const CCommonComMode& rCommMode,
				const BOOL isFirstCap, const DWORD rejectStatus, const BOOL isMuteValues,
				WORD wAudioMuteByPort, WORD wVideoMuteByPort,
				char *szAudioMuteByDirection, char *szVideoMuteByDirection,
				char *szAudioMuteByInactive, char *szVideoMuteByInactive) const
{
	// if rejectStatus != STATUS_OK => reject call
	BYTE* pCapBuffer = NULL;
	//set the direction
	cmCapDirection eAudioDirection = cmCapReceiveAndTransmit;
	cmCapDirection eVideoDirection = cmCapReceiveAndTransmit;
	if(isMuteValues)
	{
		if(strcmp(szAudioMuteByDirection, "receive") == 0)
			eAudioDirection = cmCapReceive;
		else if(strcmp(szAudioMuteByDirection, "transmit") == 0)
			eAudioDirection = cmCapTransmit;
		else if(strcmp(szAudioMuteByInactive, "inactive") == 0)
			eAudioDirection = (cmCapDirection)kInactive;
		else
			eAudioDirection = cmCapReceiveAndTransmit;

		if(strcmp(szVideoMuteByDirection, "receive") == 0)
			eVideoDirection = cmCapReceive;
		else if(strcmp(szVideoMuteByDirection, "transmit") == 0)
			eVideoDirection = cmCapTransmit;
		else if(strcmp(szVideoMuteByInactive, "inactive") == 0)
			eVideoDirection = (cmCapDirection)kInactive;
		else
			eVideoDirection = cmCapReceiveAndTransmit;
	}

	WORD  nCapsLen = (rejectStatus == STATUS_OK)? rCapSet.CreateCapStructSIP(&pCapBuffer,isFirstCap, eAudioDirection, eVideoDirection, cmCapReceiveAndTransmit) : 0;

	BYTE* pHeadersBuffer = NULL;
	WORD  nHeadersLen = CreateSipHeadersBuffer(&pHeadersBuffer, rejectStatus);

	DWORD structLen = sizeof(sipSdpAndHeadersBaseSt) + nCapsLen + nHeadersLen;

	// result buffer
	*ppCapsAndHeadersBuffer = new BYTE [structLen];
	memset(*ppCapsAndHeadersBuffer, 0, structLen);

	sipSdpAndHeadersSt* pSdpStruct = (sipSdpAndHeadersSt*)(*ppCapsAndHeadersBuffer);

//	pIndInvite->status = STATUS_OK;
//	pIndInvite->bIsFocus = 1;

	pSdpStruct->callRate = rCapSet.GetCallRate() * 10;
	pSdpStruct->sipMediaLinesOffset = 0;
	pSdpStruct->sipMediaLinesLength = nCapsLen;
	pSdpStruct->sipHeadersOffset = nCapsLen;
	pSdpStruct->sipHeadersLength = nHeadersLen;
	pSdpStruct->lenOfDynamicSection = nCapsLen + nHeadersLen;

	std::string ipAddress;
	std::string prefix;
	enScopeId remoteScopeid = eScopeIdOther;
	if (GetIpVersion() == eIpVersion6)
	{
		char ipStr[128];
		memset (&ipStr, '\0', 128);
		remoteScopeid = getScopeId(ipToString(m_H225RemoteIpAddress, ipStr, 1));

		if (remoteScopeid == eScopeIdSite)
		{
			prefix = "fec0:";
		}
		else if (remoteScopeid == eScopeIdGlobal)
		{
			prefix = "2001:";
		}
		else if (remoteScopeid == eScopeIdLink)
		{
			prefix = "fe80:";
		}
	}

	if( nCapsLen ) // when reject, caps are empty
	{
		memcpy((pSdpStruct->capsAndHeaders), pCapBuffer, nCapsLen);
	}

	//Audio channel
	///////////////
	mcXmlTransportAddress dummyMediaIp;
	unsigned int dummyRtcpPort;
	mcXmlTransportAddress &audioMediaIp = ExtractMLineMediaIp(kMediaLineInternalTypeAudio, pSdpStruct, dummyMediaIp);
	unsigned int &audioRtcpPort = ExtractMLineRtcpPort(kMediaLineInternalTypeAudio, pSdpStruct, dummyRtcpPort);
	if (GetIpVersion() == eIpVersion4)
	{
		audioMediaIp.transAddr.ipVersion = eIpVersion4;
		stringToIp(&audioMediaIp.transAddr, "172.22.190.70");
	}
	else
	{
		ipAddress = prefix;
		ipAddress += "db8:0:1:172:22:190:70";
		char* ipStr = (char*)ipAddress.c_str();

		audioMediaIp.transAddr.ipVersion = eIpVersion6;
		audioMediaIp.unionProps.unionType = eIpVersion6;
		stringToIp(&audioMediaIp.transAddr  , ipStr);
		audioMediaIp.transAddr.addr.v6.scopeId = remoteScopeid;
	}
	// check all audio mute condition (mute flag, mute values + audio ports)
	if(m_isMuted != FALSE || isMuteValues)
	{
		audioMediaIp.transAddr.port = wAudioMuteByPort;
		if(wAudioMuteByPort == 0)
			audioRtcpPort = 0;
		else
			audioRtcpPort = 1 + wAudioMuteByPort;
	}
	else
	{
		audioMediaIp.transAddr.port = 0xc020;
		audioRtcpPort = 1 + audioMediaIp.transAddr.port;
	}


	//Video channel
	///////////////
	mcXmlTransportAddress &videoMediaIp = ExtractMLineMediaIp(kMediaLineInternalTypeVideo, pSdpStruct, dummyMediaIp);
	unsigned int &videoRtcpPort = ExtractMLineRtcpPort(kMediaLineInternalTypeVideo, pSdpStruct, dummyRtcpPort);
	if( rCommMode.IsVideoToOpen() )
	{
		if (GetIpVersion() == eIpVersion4)
		{
			videoMediaIp.transAddr.ipVersion = eIpVersion4;
			stringToIp(&videoMediaIp.transAddr, "172.22.190.70");

		}
		else
		{
			ipAddress = prefix;
			ipAddress += "db8:0:1:172:22:190:70";
			char* ipStr = (char*)ipAddress.c_str();

			videoMediaIp.transAddr.ipVersion = eIpVersion6;
			videoMediaIp.unionProps.unionType = eIpVersion6;
			stringToIp(&videoMediaIp.transAddr, ipStr);
			videoMediaIp.transAddr.addr.v6.scopeId = remoteScopeid;
		}
		// check all video mute condition (mute flag, mute values + audio ports)
		if(isMuteValues)
		{
			videoMediaIp.transAddr.port = wVideoMuteByPort;
			if(wVideoMuteByPort == 0)
				videoRtcpPort = 0;
			else
				videoRtcpPort = 1 + wVideoMuteByPort;
		}
		else
		{
			videoMediaIp.transAddr.port       = 0xaac2;
			videoRtcpPort = 1 + videoMediaIp.transAddr.port;
		}
	}
	else
		PTRACE2(eLevelInfoNormal,"CEndpointSip::CreateSdpStructBuffer:", "No video to open");

	//Data channel (Not implemented yet)
	////////////////////////////////////
	mcXmlTransportAddress &dataMediaIp = ExtractMLineMediaIp(kMediaLineInternalTypeFecc, pSdpStruct, dummyMediaIp);
	unsigned int &dataRtcpPort = ExtractMLineRtcpPort(kMediaLineInternalTypeFecc, pSdpStruct, dummyRtcpPort);
	if( rCommMode.IsFeccToOpen() )
	{
		if (GetIpVersion() == eIpVersion4)
		{
			dataMediaIp.transAddr.ipVersion = eIpVersion4;
			stringToIp(&dataMediaIp.transAddr, "172.22.190.70");
		}
		else
		{
			ipAddress = prefix;
			ipAddress += "db8:0:1:172:22:190:70";
			char* ipStr = (char*)ipAddress.c_str();


			dataMediaIp.transAddr.ipVersion = eIpVersion6;
			dataMediaIp.unionProps.unionType = eIpVersion6;
			stringToIp(&dataMediaIp.transAddr, ipStr);
			dataMediaIp.transAddr.addr.v6.scopeId = remoteScopeid;
		}
		dataMediaIp.transAddr.port       = 0xd020;
		dataRtcpPort = 1 + dataMediaIp.transAddr.port;
	}

	//Bfcp channel
	///////////////
	mcXmlTransportAddress &bfcpMediaIp = ExtractMLineMediaIp(kMediaLineInternalTypeBfcp, pSdpStruct, dummyMediaIp);
	unsigned int &bfcpRtcpPort = ExtractMLineRtcpPort(kMediaLineInternalTypeBfcp, pSdpStruct, dummyRtcpPort);

	if( rCommMode.IsVideoToOpen() && /*rCapSet.GetH239Rate() > 0 || */rCapSet.IsBfcpSupported())
	{
		if (GetIpVersion() == eIpVersion4)
		{
			bfcpMediaIp.transAddr.ipVersion = eIpVersion4;
			stringToIp(&bfcpMediaIp.transAddr, "172.22.190.70");
		}
		else
		{
			ipAddress = prefix;
			ipAddress += "db8:0:1:172:22:190:70";
			char* ipStr = (char*)ipAddress.c_str();

			bfcpMediaIp.transAddr.ipVersion = eIpVersion6;
			bfcpMediaIp.unionProps.unionType = eIpVersion6;
			stringToIp(&bfcpMediaIp.transAddr, ipStr);
			bfcpMediaIp.transAddr.addr.v6.scopeId = remoteScopeid;
		}
		bfcpMediaIp.transAddr.port       = 49165; // 0xc00d;
		bfcpRtcpPort = 1 + bfcpMediaIp.transAddr.port;
	}

	//Content channel
	///////////////
	mcXmlTransportAddress &contentMediaIp = ExtractMLineMediaIp(kMediaLineInternalTypeContent, pSdpStruct, dummyMediaIp);
	unsigned int &contentRtcpPort = ExtractMLineRtcpPort(kMediaLineInternalTypeContent, pSdpStruct, dummyRtcpPort);
	if( rCommMode.IsH239ToOpen() && rCapSet.IsBfcpSupported())
	{
		if (GetIpVersion() == eIpVersion4)
		{
			contentMediaIp.transAddr.ipVersion = eIpVersion4;
			stringToIp(&contentMediaIp.transAddr, "172.22.190.70");
		}
		else
		{
			ipAddress = prefix;
			ipAddress += "db8:0:1:172:22:190:70";
			char* ipStr = (char*)ipAddress.c_str();

			contentMediaIp.transAddr.ipVersion = eIpVersion6;
			contentMediaIp.unionProps.unionType = eIpVersion6;
			stringToIp(&contentMediaIp.transAddr, ipStr);
			contentMediaIp.transAddr.addr.v6.scopeId = remoteScopeid;
		}
		contentMediaIp.transAddr.port       = 49156; // 0xc004;
		contentRtcpPort = 1 + contentMediaIp.transAddr.port;
	}


	audioMediaIp.transAddr.distribution  = eDistributionUnicast;
	audioMediaIp.transAddr.transportType = eTransportTypeUdp;
	audioMediaIp.unionProps.unionSize    = sizeof(ipAddressIf);

	videoMediaIp.transAddr.distribution  = eDistributionUnicast;
	videoMediaIp.transAddr.transportType = eTransportTypeUdp;
	videoMediaIp.unionProps.unionSize    = sizeof(ipAddressIf);

	dataMediaIp.transAddr.distribution  = eDistributionUnicast;
	dataMediaIp.transAddr.transportType = eTransportTypeUdp;
	dataMediaIp.unionProps.unionSize    = sizeof(ipAddressIf);

	bfcpMediaIp.transAddr.distribution  = eDistributionUnicast;
	bfcpMediaIp.transAddr.transportType = eTransportTypeUdp;
	bfcpMediaIp.unionProps.unionSize    = sizeof(ipAddressIf);

	contentMediaIp.transAddr.distribution  = eDistributionUnicast;
	contentMediaIp.transAddr.transportType = eTransportTypeUdp;
	contentMediaIp.unionProps.unionSize    = sizeof(ipAddressIf);

	BYTE* pDestHeadersBuffer = (BYTE*)(pSdpStruct->capsAndHeaders+nCapsLen);
	memcpy(pDestHeadersBuffer, pHeadersBuffer, nHeadersLen);

	// release cap buffer memory
	PDELETEA(pCapBuffer);
	// release headers buffer memory
	PDELETEA(pHeadersBuffer);

	return structLen;
}


/////////////////////////////////////////////////////////////////////////////
DWORD CEndpointSip::CreateSipHeadersBuffer(BYTE** ppHeadersBuffer, const DWORD rejectStatus) const
{
	const int NUM_SIP_HEADERS = 12; // change carefully!!! check indexes in aHeaderElements[] and pszArray[]
	int usedSipHeaders = 10;
	if(GetIsMRE()){
		usedSipHeaders = 12;
	}

	WORD   nStringLen = 0; // len of string
	ALLOCBUFFER(pszStringList, 4096);
	memset(pszStringList, 0, 4096);

	sipHeaderElement aHeaderElements[NUM_SIP_HEADERS];

	aHeaderElements[0].eHeaderField	= kToDisplay;   // 0x1;
	aHeaderElements[1].eHeaderField	= kTo;          // 0x2;
	aHeaderElements[2].eHeaderField	= kFromDisplay; // 0x4;
	aHeaderElements[3].eHeaderField	= kFrom;        // 0x5;
	aHeaderElements[4].eHeaderField	= kContactDisplay; // 0x7;
	aHeaderElements[5].eHeaderField	= kContact;     // 0x8;
	aHeaderElements[6].eHeaderField	= kReqLine;     // 0xa;
	aHeaderElements[7].eHeaderField	= kCallId;      // 0x13;
	aHeaderElements[8].eHeaderField	= kVia;         // 0x9;
	aHeaderElements[9].eHeaderField	= kUserAgent;   // 0xb;

	if(GetIsMRE()){
		aHeaderElements[10].eHeaderField	= kMrd;
		aHeaderElements[11].eHeaderField	= kSdpSession_s;
	}

	char* pszArray[NUM_SIP_HEADERS];

	for( int i=0; i<usedSipHeaders; i++ )
	{
		pszArray[i] = new char[256];
		memset(pszArray[i], '\0', 256);
	}

	ALLOCBUFFER(pszIpStr, 128);
	GetIpStr(pszIpStr);

  const char* CSIpAddress = NULL;
  switch (GetIpVersion())
  {
    case eIpVersion4:
      CSIpAddress = ::GetEpSystemCfg()->GetCSIpAddress();
      break;
    case eIpVersion6:
      CSIpAddress = ::GetEpSystemCfg()->GetCSIpV6Address();
      break;
    default:
    {
    	DEALLOCBUFFER(pszStringList);
    	PASSERT_AND_RETURN_VALUE(1, 0);
    }
  }

  //kTo
	if (strcmp(CSIpAddress, "0.0.0.0") == 0)
	{
		std::string fullConfName = GetConfName();
		int index = fullConfName.find("@");
		std::string partyName;
		if (index == -1)
			partyName = fullConfName;
		else
			partyName = fullConfName.substr(0, index);


		sprintf(pszArray[1],"%s@172.22.192.26", partyName.c_str());
	}
	else
	{
		std::string fullConfName = GetConfName();
		int index = fullConfName.find("@");
		std::string partyName;
		if (index == -1)
			partyName = fullConfName;
		else
			partyName = fullConfName.substr(0, index);

		sprintf(pszArray[1],"%s@%s", partyName.c_str(), CSIpAddress);
	}

	//kFrom
	sprintf(pszArray[3], "%s@%s", m_szEpName, pszIpStr);


	// kContact: if reject status = moved, put redirection address
	if( rejectStatus == SipCodesMovedPerm  ||  rejectStatus == SipCodesMovedTemp )
	{
		strncpy(pszArray[5], ::GetEpSystemCfg()->GetRedirectionSipAddress(), 256);
	}
	else
	{
		sprintf(pszArray[5], "%s", pszIpStr);
	}

	//kReqLine
	if( strlen(m_szConfName) )
	{
		std::string fullConfName = GetConfName();
		int index = fullConfName.find("@");
		std::string partyName;
		if (index == -1)
			partyName = fullConfName;
		else
			partyName = fullConfName.substr(0, index);


		sprintf(pszArray[6],"%s@%s", partyName.c_str(), CSIpAddress);
	}
	else
	{
		strcat(pszArray[6], CSIpAddress);
	}

	//kCallId
	sprintf(pszArray[7],"1703e18-46a916ac-13ce-4280b6b1-4f76a2d-47bb@%s", pszIpStr);


	//kVia
	sprintf(pszArray[8],"%s:%d", pszIpStr , 5070);

	//kUserAgent
	strcpy(pszArray[9],m_UserAgent);

    DEALLOCBUFFER(pszIpStr);

    if(GetIsMRE()){
        //kMrd
        strcpy(pszArray[10], "MRE; MRC-V=1.0.1");

        //kSdpSession_s
        strcpy(pszArray[11], "MRD=MRE MRC-V=1.0.1");
    }

	WORD position = 0;
	for( int i=0; i<usedSipHeaders; i++ )
	{
		aHeaderElements[i].flags	 = 0x0;
		aHeaderElements[i].position  = position;

		strcpy( pszStringList+position, pszArray[i] );
		int len = strlen(pszArray[i]);
		len++;
		position +=  len;
	}
	nStringLen = position + 1;


	for( int i=0; i<usedSipHeaders; i++ )
		DEALLOCBUFFER(pszArray[i]);

	sipMessageHeadersBase  tHeadersBase;
	tHeadersBase.numOfHeaders = usedSipHeaders;
	tHeadersBase.headersListLength = usedSipHeaders * sizeof(sipHeaderElement) + nStringLen;

		// create result buffer
	DWORD nBufferLen = sizeof(sipMessageHeadersBase)
					 + usedSipHeaders * sizeof(sipHeaderElement)
					 + nStringLen;

	*ppHeadersBuffer = new BYTE[nBufferLen];
		// copy from temporary buffer to result
		//  a. sipMessageHeaderBase
	memcpy(*ppHeadersBuffer,&tHeadersBase,sizeof(sipMessageHeadersBase));
		//  b. 10 sipHeaderElements
	for( int i=0; i<usedSipHeaders; i++ )
	{
		BYTE*  pPtr = *ppHeadersBuffer + sizeof(sipMessageHeadersBase) + i*sizeof(sipHeaderElement);
		memcpy(pPtr,&(aHeaderElements[i]),sizeof(sipHeaderElement));
	}
		//  c. string list
	BYTE*  pPtr = *ppHeadersBuffer + sizeof(sipMessageHeadersBase) + usedSipHeaders*sizeof(sipHeaderElement);

	memcpy(pPtr, pszStringList, nStringLen);

	DEALLOCBUFFER(pszStringList);

	return nBufferLen;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointSip::NeedReinviteForBfcp() const
{
	// if we have Bfcp cap and rmx has Bfcp cap
	if( m_pCap->GetH239Rate() == 0  ||  m_pConfCap->GetH239Rate() == 0)
		return FALSE;

	// if SIM_SIP_RCV_INVITE_RESPONSE recieved only once (it maybe was reinvite because mute/unmute)
	if( m_apiArray[SIM_SIP_RCV_INVITE_RESPONSE] != 1 )
		return FALSE;

	return TRUE;
}
////////////////////////////////////////////////////////
void CEndpointSip::SetUserAgent( const char* pszConfName )
{
	if ( NULL != pszConfName ) {
		strncpy(m_UserAgent, pszConfName, IP_STRING_LEN-1);
		m_UserAgent[IP_STRING_LEN-1] = '\0';
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendBfcpTransportInd( CMplMcmsProtocol* pMplProtocol ) const
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendBfcpTransportInd - SIP_CS_BFCP_TRANSPORT_IND ");

	mcIndBfcpTransport* pInd = new mcIndBfcpTransport;
	DWORD dataLen = sizeof(mcIndBfcpTransport);
	memset(pInd, 0, dataLen);

	pInd->status = bfcp_msg_status_connected;

#ifdef __BFCP_CS_CONNECTION_ENABLED__

	if (GetIpVersion() == eIpVersion4)
	{
		pInd->hostAddress.transAddr.ipVersion = eIpVersion4;
		stringToIp(&(pInd->hostAddress.transAddr), "172.22.190.70");

		pInd->remoteAddress.transAddr.ipVersion = eIpVersion4;
		stringToIp(&(pInd->remoteAddress.transAddr), "123.0.0.70");
	}
	else
	{
		std::string ipAddress;
		std::string prefix;
		enScopeId remoteScopeid;

		char ipStr1[128];
		memset (&ipStr1, '\0', 128);
		remoteScopeid = getScopeId(ipToString(m_H225RemoteIpAddress, ipStr1, 1));

		if (remoteScopeid == eScopeIdSite)
		{
			prefix = "fec0:";
		}
		else if (remoteScopeid == eScopeIdGlobal)
		{
			prefix = "2001:";
		}
		else if (remoteScopeid == eScopeIdLink)
		{
			prefix = "fe80:";
		}

		ipAddress = prefix;
		ipAddress += "db8:0:1:172:22:190:70";

		char* ipStr = (char*)ipAddress.c_str();

		pInd->hostAddress.transAddr.ipVersion = eIpVersion6;
		pInd->hostAddress.unionProps.unionType = eIpVersion6;
		stringToIp(&(pInd->hostAddress.transAddr), ipStr);
		pInd->hostAddress.transAddr.addr.v6.scopeId = remoteScopeid;

		ipAddress = prefix;
		ipAddress += "db8:0:1:123:0:0:70";

		ipStr = (char*)ipAddress.c_str();

		pInd->remoteAddress.transAddr.ipVersion = eIpVersion6;
		pInd->remoteAddress.unionProps.unionType = eIpVersion6;
		stringToIp(&(pInd->remoteAddress.transAddr), ipStr);
		pInd->remoteAddress.transAddr.addr.v6.scopeId = remoteScopeid;
	}
	pInd->hostAddress.transAddr.port       = 49165; // 0xc00d;
	pInd->remoteAddress.transAddr.port     = 12345;
#endif //__BFCP_CS_CONNECTION_ENABLED__

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(), SIP_CS_BFCP_TRANSPORT_IND,
                     (BYTE*)pInd, dataLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETE(pInd);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointSip::SendCsReinviteInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointSip::SendCsReinviteInd - SIP_CS_SIG_REINVITE_IND ");

	// create highest capabilities and create communication mode
	//CCapSet  rCommonCap(*m_pCap, *m_pConfCap);
	m_rComMode.Create(*m_pCap);

	BYTE*  pSdpStructBytes = NULL;
	DWORD  nSdpStructLen = CreateSdpStructBuffer(&pSdpStructBytes,*m_pCap,m_rComMode,FALSE /* first cap */, STATUS_OK, FALSE /* no mute values */);

	sipSdpAndHeadersSt* pSdpStruct = (sipSdpAndHeadersSt*)pSdpStructBytes;

	DWORD  indLen    = sizeof(mcIndReInvite) + pSdpStruct->lenOfDynamicSection;
	BYTE*  pIndArray = new BYTE [indLen];
	memset(pIndArray, 0, indLen);

	mcIndReInvite* pIndReInvite = (mcIndReInvite*)pIndArray;

	pIndReInvite->status = STATUS_OK;
	pIndReInvite->bIsFocus = 1;

	memcpy(&(pIndReInvite->sipSdpAndHeaders),pSdpStructBytes,
		sizeof(sipSdpAndHeadersBaseSt) + pSdpStruct->lenOfDynamicSection);

	PDELETEA(pSdpStructBytes);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(), SIP_CS_SIG_REINVITE_IND,
                     (BYTE*)pIndArray, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndArray);
}
