//+========================================================================+
//                    EndpointH323.cpp									   |
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
#include <stdlib.h>
#include "Macros.h"
#include "Trace.h"
#include "IpCsOpcodes.h"
#include "IpMfaOpcodes.h"
#include "ConfPartyDefines.h"
#include "ConfPartySharedDefines.h"
#include "IpRtpFeccRoleToken.h"
#include "ChannelParams.h"
#include "Capabilities.h"
#include "IpRtpInd.h"
#include "SystemFunctions.h"
#include "Segment.h"
#include "MplMcmsProtocol.h"
#include "TaskApi.h"
#include "SimApi.h"
#include "EndpointsSim.h"
#include "EndpointsSimConfig.h"
#include "EpSimCapSetsList.h"
#include "EpSimH323BehaviorList.h"
#include "EndpointH323.h"
#include "OpcodesMcmsCommon.h"
#include "HostCommonDefinitions.h"
#include "TraceStream.h"
#include "CSSimTaskApi.h"
#include "OpcodesMcmsInternal.h"


// global static parameters
static DWORD	g_LocalChannelIndex = 111;



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CEndpointH323)

	ONEVENT( ROLE_TOKEN_OWNER_TOUT, ANYCASE, CEndpointH323::OnRoleTokenOwnerTout )
	ONEVENT( SIM_H323_TOUT_DELAY_SEND_INCOMING_CHANNEL_IND, ANYCASE, CEndpointH323::OnTimerDelaySendIncomingChanInd)

PEND_MESSAGE_MAP(CEndpointH323,CEndpoint);



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CEndpointH323::CEndpointH323( CTaskApi *pCSApi, const CCapSet& rCap, const CH323Behavior& rBehav )
		: CEndpoint(pCSApi,rCap,rBehav)     // constructor
{
	for (int i=0; i < SIM_H323_MAX_API_COMMANDS; i++)
		m_apiArray[i] = 0;

	m_isReopenChannel = false;

	m_lenReopenTimer = 0;
	m_conferenceType = 0;
	m_maxRate = 0;
    memset(&m_mcIncomingChannelResponse,0,sizeof(mcReqIncomingChannelResponse));

    memset(&m_sourceAliasNameOrTel, 0,H243_NAME_LEN);

	// Dial-Out:  gets from the setup command
	// Dial-In:   create for call-offering


	// Remote address - (EP)
	//////////////////////////////////////
	//H225
	memset(&m_H225RemoteIpAddress, 0, sizeof(mcTransportAddress));
	m_H225RemoteIpAddress.port = 0;

	//H245
	memset(&m_H245RemoteIpAddress, 0, sizeof(mcTransportAddress));
	m_H245RemoteIpAddress.port = 0;
	stringToIp(&m_H245RemoteIpAddress, "172.13.12.12");

	//RTP
	memset(&m_rtpRemoteIpAddress, 0, sizeof(mcTransportAddress));
	m_rtpRemoteIpAddress.port = 0;
	stringToIp(&m_rtpRemoteIpAddress, "172.14.12.12");


	// Local address - (RMX)
	//////////////////////////////////////
	//H225
	memset(&m_H225LocalIpAddress, 0, sizeof(mcTransportAddress));
	m_H225LocalIpAddress.port = 0;

	//H245
	memset(&m_H245LocalIpAddress, 0, sizeof(mcTransportAddress));
	m_H245LocalIpAddress.port = 0;
	stringToIp(&m_H245LocalIpAddress, "172.15.12.12");

	//RTP
	memset(&m_rtpLocalIpAddress, 0, sizeof(mcTransportAddress));
	m_rtpLocalIpAddress.port = 0;
	stringToIp(&m_rtpLocalIpAddress, "172.16.12.12");


	// fill all channels
	/////////////////////
	SetChannelDetails(&(m_taChannels[eAudioChannelIn]), cmCapAudio,kRolePeople, g_LocalChannelIndex++, 0/*mcIndex*/, cmCapReceive/*dir=IN*/);
	SetChannelDetails(&(m_taChannels[eAudioChannelOut]), cmCapAudio,kRolePeople, g_LocalChannelIndex++, 0/*mcIndex*/, cmCapTransmit/*dir=OUT*/);
	SetChannelDetails(&(m_taChannels[eVideoChannelIn]), cmCapVideo,kRolePeople, g_LocalChannelIndex++, 0/*mcIndex*/, cmCapReceive/*dir=IN*/);
	SetChannelDetails(&(m_taChannels[eVideoChannelOut]), cmCapVideo,kRolePeople, g_LocalChannelIndex++, 0/*mcIndex*/, cmCapTransmit/*dir=OUT*/);
	SetChannelDetails(&(m_taChannels[eFeccChannelIn]), cmCapData,kRolePeople, g_LocalChannelIndex++, 0/*mcIndex*/, cmCapReceive/*dir=IN*/);
	SetChannelDetails(&(m_taChannels[eFeccChannelOut]), cmCapData,kRolePeople, g_LocalChannelIndex++, 0/*mcIndex*/, cmCapTransmit/*dir=OUT*/);
	SetChannelDetails(&(m_taChannels[eH239ChannelIn]), cmCapVideo,kRolePresentation, g_LocalChannelIndex++, 0/*mcIndex*/, cmCapReceive/*dir=IN*/);
	SetChannelDetails(&(m_taChannels[eH239ChannelOut]), cmCapVideo,kRolePresentation, g_LocalChannelIndex++, 0/*mcIndex*/, cmCapTransmit/*dir=OUT*/);

	//LPR
	m_isLPRCap = FALSE;

	//Encryption
	m_pEncryptionStruct = NULL;

	// audio board details
	CleanAudioBoardDetails();

	m_enDiscoInitiator = eInitiatorUnknown;

//	m_nMcuId = m_nTerminalId = 0;
//	m_enRoleTokenLastCmd = kUnknownRoleTokenOpcode;
}


/////////////////////////////////////////////////////////////////////////////
CEndpointH323::CEndpointH323(const CEndpointH323& other) : CEndpoint(other)
{
	// illegal use
	PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
CEndpointH323::~CEndpointH323()
{
	BYTE* ptr = (BYTE*)m_pEncryptionStruct;
	PDELETEA( ptr );
	m_pEncryptionStruct = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CEndpointH323& CEndpointH323::operator= (const CEndpointH323& other)
{
	// illegal use
	PASSERT(1);

	return *this;
}

/////////////////////////////////////////////////////////////////////////////

char* CEndpointH323::GetIpStr() const
{
	static char tempName[128];
	memset (&tempName, '\0', 128);
	return ipToString(m_H225RemoteIpAddress, tempName, 1);
}

/////////////////////////////////////////////////////////////////////////////

void CEndpointH323::GetIpStr(char* str) const
{
	char tempName[128];
	memset (&tempName, '\0', 128);
	ipToString(m_H225RemoteIpAddress, tempName, 1);

	strcpy(str, tempName);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::HandleProtocolEvent( CMplMcmsProtocol* pMplProtocol )
{
	switch( pMplProtocol->getOpcode() )
	{
		// Dial-In Response Opcodes
		// =======================
		case H323_CS_SIG_CALL_ANSWER_REQ:
		{
			if( STATUS_OK != pMplProtocol->getCentralSignalingHeaderStatus() )
			{
				// if lobby rejected call or resource allocation failed or MFA port opening failed (PartyCnlt sent)
				SendCallIdleInd(pMplProtocol);
				CleanAfterDisconnect();
				SetState(eEpStateDisconnected);
				break;
			}
			OnCallAnswerReq( pMplProtocol );
			break;
		}

		// Open Party Opcodes
		// ==================
		case H323_CS_SIG_CALL_SETUP_REQ:
		{
			OnCsCallSetupReq( pMplProtocol );
			break;
		}
		case H323_CS_SIG_CREATE_CNTL_REQ:
		{
			OnCsCreateCntlReq( pMplProtocol );
			break;
		}
		case H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ:
		{
			OnCsIncomingChnlResponse( pMplProtocol );
			break;
		}
		case H323_CS_SIG_OUTGOING_CHNL_REQ:
		{
			OnOutgoingChnlReq( pMplProtocol );
			break;
		}

		// Close Party Opcodes
		// ==================
		// if disconnected from the party side
		// #define H323_CS_SIG_START_CHANNEL_CLOSE_IND
		case H323_CS_SIG_CHNL_DROP_REQ:
		{
			OnChnlDropReq( pMplProtocol );
			//#define H323_CS_SIG_CHANNEL_CLOSE_IND
			break;
		}
		case H323_CS_SIG_CALL_DROP_REQ:
		{
			OnCallDropReq( pMplProtocol );
			//#define H323_CS_SIG_CALL_IDLE_IND
			break;
		}
		// if connected from the party side
		// #define H323_CS_SIG_CALL_IDLE_IND
		case H323_CS_SIG_CALL_CLOSE_CONFIRM_REQ:
		{
			OnCallCloseConfirmReq(pMplProtocol);
			break;
		}
		// Temp - Till Vasily supports correctly
		case H323_CS_SIG_VIDEO_UPDATE_PIC_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandleProtocolEvent - H323_CS_SIG_VIDEO_UPDATE_PIC_REQ (Nothing)");
			break;
		}
		case H323_CS_SIG_MULTIPOINTMODECOM_TERMINALID_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandleProtocolEvent - H323_CS_SIG_MULTIPOINTMODECOM_TERMINALID_REQ");
			mcReqMultipointModeComTerminalIDMessage* pReq = (mcReqMultipointModeComTerminalIDMessage*)pMplProtocol->GetData();
			m_nMcuId      = pReq->mcuID;
			m_nTerminalId = pReq->terminalID;
			break;
		}
		case H323_CS_SIG_CONFERENCE_IND_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandleProtocolEvent - H323_CS_SIG_CONFERENCE_IND_REQ (Nothing)");
			break;
		}
		case H323_CS_SIG_CHNL_NEW_RATE_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandleProtocolEvent - H323_CS_SIG_CHNL_NEW_RATE_REQ (Nothing)");
			OnChnlNewRateReq(pMplProtocol);
			break;
		}
		case H323_CS_SIG_ROLE_TOKEN_REQ :
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandleProtocolEvent - H323_CS_SIG_ROLE_TOKEN_REQ");
			OnRoleTokenReq(pMplProtocol);
			break;
		}
		case H323_CS_SIG_CHANNEL_ON_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandleProtocolEvent - H323_CS_SIG_CHANNEL_ON_REQ (Nothing)");
			OnChannelOnReq(pMplProtocol);
			break;
		}
		case H323_CS_SIG_CHANNEL_OFF_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandleProtocolEvent - H323_CS_SIG_CHANNEL_OFF_REQ (Nothing)");
			break;
		}
		case H323_CS_SIG_CAPABILITIES_RES_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandelProtocolEvent - H323_CS_SIG_CAPABILITIES_RES_REQ (Nothing)");
			break;
		}
		case H323_CS_FACILITY_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandelProtocolEvent - H323_CS_FACILITY_REQ (Nothing)");
			break;
		}
		case H323_CS_SIG_RE_CAPABILITIES_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandelProtocolEvent - H323_CS_SIG_RE_CAPABILITIES_REQ");
			//SendCsCapResponseInd(pMplProtocol);
			OnCsReCapReq(pMplProtocol);
			break;
		}
		case H323_CS_SIG_LPR_MODE_CHANGE_RES_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandelProtocolEvent - H323_CS_SIG_LPR_MODE_CHANGE_RES_REQ");
			break;
		}
		case H323_CS_SIG_RSS_CMD_REQ:
		{
			PTRACE(eLevelInfoNormal,"CEndpointH323::HandelProtocolEvent - H323_CS_SIG_RSS_CMD_REQ");
			OnRssCmdReq(pMplProtocol);
			break;
		}

		default:
		{
			PASSERT(pMplProtocol->getOpcode() + 1000);
			break;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnCsReCapReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::OnCsReCapReq - H323_CS_SIG_RE_CAPABILITIES_REQ");

	// gets the req struct
	mcReqCreateControl *req = (mcReqCreateControl*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointH323::OnCsReCapReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_mcReqCreateControl, req, sizeof(mcReqCreateControl) );

	// Create conference cap set
	m_pConfCap->CreateFrom323Struct(&req->capabilities);

	//set LPR from conf cap to e.p. cap
	m_pCap->SetLPR(m_pConfCap->IsLPR());
	m_isLPRCap = m_pConfCap->IsLPR();

	// checks legal parameters
	IsLeagaParametersCreateCntl( req );

	// wait a while...

	// Send Cap Response Ind
	SendCsCapResponseInd( pMplProtocol );


	//////////////////////
	// re-open media channels
	//////////////////////
	if(m_isReopenChannel)
	{
		TChannelDetails *people_video=GetChannel(cmCapVideo,kRolePeople,cmCapReceive/*dir=IN*/);

		if ((people_video) && (people_video->openedInSim))
			SendStartChnlCloseInd( pMplProtocol, people_video);
		else if (people_video == NULL)
			PTRACE(eLevelError,"CEndpointLinker::ForwardCapabilityInd - pCSProt is NULL ");
	}

	// create highest capabilities and create communication mode
	CCapSet  rCapCommon(*m_pCap, *m_pConfCap);
	TChannelDetails *present_video=GetChannel(cmCapVideo,kRolePresentation,cmCapReceive/*dir=IN*/);
	UpdateIncomingChannByNewCap(present_video, rCapCommon);

	m_rComMode.Create(rCapCommon);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::DeSerialize( CSegment& rParam )
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

	rParam >> m_isReopenChannel;
	rParam >> m_lenReopenTimer;

    SetIp(szIpTemp);
    SetIpVersion(ipVer);
        // set CS ID
        DWORD cs_id;
        rParam >> cs_id;
        SetCsHandle(cs_id);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::Serialize(  CSegment& rSegment ) const
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
	rSegment << m_isReopenChannel;
	rSegment << m_lenReopenTimer;
        rSegment << GetCSID();
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SerializeDetails(  CSegment& rSegment ) const
{
//	m_pCap->Serialize(rSegment);
//	m_pBehavior->Serialize(rSegment);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SetIp( const char* pszEpIp )
{
	stringToIp(&m_H225RemoteIpAddress, (char*)pszEpIp);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SetIpVersion(DWORD ipVer)
{
	m_H225RemoteIpAddress.ipVersion = ipVer;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CEndpointH323::GetIpVersion() const
{
	return m_H225RemoteIpAddress.ipVersion;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SetTransportAddress( mcTransportAddress* mcTA )
{
	m_H225RemoteIpAddress.addr = mcTA->addr;
	m_H225RemoteIpAddress.ipVersion = mcTA->ipVersion;
	m_H225RemoteIpAddress.port = mcTA->port;
	m_H225RemoteIpAddress.distribution = mcTA->distribution;
	m_H225RemoteIpAddress.transportType = mcTA->transportType;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SetArrayIndex( const WORD ind )
{
	m_arrayIndex = ind;

	// we supply
	m_H245RemoteIpAddress.port = 6001 + (10 * m_arrayIndex);
	m_rtpRemoteIpAddress.port = 6002 + (10 * m_arrayIndex);

	// gets from MCMS
	m_H245LocalIpAddress.port = 6003 + (10 * m_arrayIndex);
	m_rtpLocalIpAddress.port = 6004 + (10 * m_arrayIndex);
}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointH323::HandleEvent( CSegment *pMsg, DWORD msgLen, OPCODE opCode )
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
void CEndpointH323::StartCallOffering()
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::StartCallOffering");

	// set the IP Address for H225
	stringToIp(&m_H225LocalIpAddress, "172.15.12.12");
	m_H225LocalIpAddress.port = 6005 + (10 * m_arrayIndex);

	m_H225RemoteIpAddress.port = 6006 + (10 * m_arrayIndex);

	if (GetIpVersion() == eIpVersion6)
	{
		stringToIp(&m_H225LocalIpAddress, "2001:db8:0:1:172:15:12:12");
		enScopeId scopeid = getScopeId("2001:db8:0:1:172:15:12:12");
		m_H225LocalIpAddress.addr.v6.scopeId = scopeid;

		stringToIp(&m_H245RemoteIpAddress, "2001:db8:0:1:172:13:12:12");
		scopeid = getScopeId("2001:db8:0:1:172:13:12:12");
		m_H245RemoteIpAddress.addr.v6.scopeId = scopeid;
	}


	// alias name
	strcpy(	m_targetAliasNamePar1, "335177" );
	strcpy(	m_targetAliasNamePar2, "335177" );
	strcpy(	m_targetAliasNameOrTel, "TEL:" );
	snprintf( m_sourceAliasNamePar, sizeof(m_sourceAliasNamePar),
			"%s %d:", "H323 Simulation - ", (int)m_arrayIndex );

	// sign as dial-in
	m_dialDirection = DIAL_IN;

	// Dial-In Start Call Command
	SendCallOfferingInd();

	m_enDiscoInitiator = eInitiatorUnknown;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendCallOfferingInd()
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::SendCallOfferingInd - H323_CS_SIG_CALL_OFFERING_IND");

	// create basic ind
	// ================
//	mcIndCallOffering *ind = new mcIndCallOffering;
	WORD indLen = 0;
	WORD size = sizeof(encTokensHeaderBasicStruct) + sizeof(encryptionToken) + 128;
	if( m_pCap->IsEncryption() )//::GetEpSystemCfg()->GetEncryptionDialIn() ) // encryption
	{
		indLen = sizeof(mcIndCallOffering) + sizeof(encryptionToken) + 128;
		BYTE* ptr = (BYTE*)m_pEncryptionStruct;
		PDELETEA( ptr );
		m_pEncryptionStruct = (encTokensHeaderStruct*) new BYTE[size];

		m_pEncryptionStruct->xmlDynamicProps.numberOfDynamicParts = 1;
		m_pEncryptionStruct->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(encryptionTokenBase) + 128;
		m_pEncryptionStruct->numberOfTokens = 1;	//For now we support only for one token.
		m_pEncryptionStruct->dynamicTokensLen =  sizeof(encryptionTokenBase) + 128;

		((encryptionToken*)(m_pEncryptionStruct->token))->xmlHeader.dynamicType = tblEncToken;
		((encryptionToken*)(m_pEncryptionStruct->token))->xmlHeader.dynamicLength = sizeof(encryptionTokenBase) + 128;
		((encryptionToken*)(m_pEncryptionStruct->token))->tokenOID	= kHalfKeyDH1024;
		((encryptionToken*)(m_pEncryptionStruct->token))->generator	= 2; //In dial out we will chose generator 2 - as polycom standard.
		((encryptionToken*)(m_pEncryptionStruct->token))->modSize	= 0;
		((encryptionToken*)(m_pEncryptionStruct->token))->hkLen		= 128;
		((encryptionToken*)(m_pEncryptionStruct->token))->filler	= 0;

		memset( (((encryptionToken*)(m_pEncryptionStruct->token))->halfKey),'1',128 );
	}
	else
	{
		BYTE* ptr = (BYTE*)m_pEncryptionStruct;
		PDELETEA( ptr );
		m_pEncryptionStruct = NULL;
		indLen = sizeof(mcIndCallOffering);
	}

	BYTE* pIndBytes = new BYTE [indLen];
	mcIndCallOffering* ind = (mcIndCallOffering*)pIndBytes;
	memset(ind, 0, indLen);	// zeroing the struct

	if( m_pEncryptionStruct != NULL )
	{
	  BYTE* encryTokens = ((BYTE*)&ind->encryTokens);
		memcpy(encryTokens, m_pEncryptionStruct, sizeof(encryptionToken) + 128);
	}

	char bufferAlias[128];

	char	portH225RemoteStr[16];
	char	portH225LocalStr[16];

	sprintf( portH225RemoteStr, "%u", m_H225RemoteIpAddress.port );
	sprintf( portH225LocalStr, "%u", m_H225LocalIpAddress.port );

	// creating source string
	/////////////////////////

	// fill XML type and size

	memcpy(&(ind->srcIpAddress.transAddr), (const void*)&m_H225RemoteIpAddress, sizeof(mcTransportAddress))	;

	memset ((char *)bufferAlias, '\0', 128);
	snprintf(bufferAlias, sizeof(bufferAlias), "%s,%s", m_sourceAliasNameOrTel , m_sourceAliasNamePar);
  strncpy(ind->srcPartyAliases, (const char*)bufferAlias, sizeof(ind->srcPartyAliases)-1);
  ind->srcPartyAliases[sizeof(ind->srcPartyAliases)-1] = '\0';


	ind->srcIpAddress.unionProps.unionType = (int)m_H225RemoteIpAddress.ipVersion;
	ind->srcIpAddress.unionProps.unionSize = sizeof(ipAddressIf);


	// creating destination string
	//////////////////////////////

	// fill XML type and size

	memcpy(&(ind->destIpAddress.transAddr), (const void*)&m_H225LocalIpAddress, sizeof(mcTransportAddress));

	memset ((char *)bufferAlias, '\0', 128);
	strncpy(m_targetAliasNameOrTel,m_szConfName, sizeof(m_targetAliasNameOrTel)-1);
	m_targetAliasNameOrTel[sizeof(m_targetAliasNameOrTel)-1] = '\0';

	snprintf(bufferAlias, sizeof(bufferAlias),"%s,%s,%s", m_targetAliasNameOrTel , m_targetAliasNamePar1, m_targetAliasNamePar2);
  strncpy(ind->destPartyAliases, (const char*)bufferAlias, sizeof(ind->destPartyAliases)-1);
  ind->destPartyAliases[sizeof(ind->destPartyAliases)-1] = '\0';

	ind->destIpAddress.unionProps.unionType = (int)m_H225LocalIpAddress.ipVersion;
	ind->destIpAddress.unionProps.unionSize = sizeof(ipAddressIf);

	ind->rate = m_pCap->GetCallRate() * 1000;	//384K;
	strcpy( ind->sDisplay, m_szEpName );
	strcpy( ind->userUser, "UserUser" );

	ind->conferenceGoal = cmCreate;
	ind->referenceValue = 0;
	ind->type = cmCallTypeP2P;
	ind->bIsActiveMc = NO;
	ind->bIsOrigin = FALSE;

	// fill h245 remote (addrs, ip version, ip, port, distrib, transport type)
	m_H245RemoteIpAddress.distribution = eDistributionUnicast;
	m_H245RemoteIpAddress.transportType = eTransportTypeTcp;
	FillMcTransportAddress( &ind->h245IpAddress, m_H245RemoteIpAddress);

	ind->bH245Establish = YES;
	ind->srcEndpointType = 0;
	ind->localH225Port = m_H225RemoteIpAddress.port;

	//////////////////////////////////
	// send the struct to CS Api
	//////////////////////////////////
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol;
	pCSProt->AddPortDescriptionHeader(0 , 0, LOBBY_CONNECTION_ID, ePhysical_art_light);
	pCSProt->AddCSHeader(5, eMcms, eCentral_signaling);

	::FillCsProtocol(pCSProt, GetCSID(),
			         H323_CS_SIG_CALL_OFFERING_IND,
			         (BYTE*)ind, indLen, m_nCsCallIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

	CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndBytes);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnGuiUpdateEp( CSegment* pParam )
{
	COsQueue txMbx;
	txMbx.DeSerialize(*pParam);

//	m_guiTxMbx.DeSerialize(*pParam);
//	DeSerialize(*pParam);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnGuiConnectEp()
{
	PTRACE2(eLevelInfoNormal,"CEndpointH323::OnGuiConnectEp - connect party, Name - ", m_szEpName);

	if( m_enEpState == eEpStateIdle  ||  m_enEpState == eEpStateDisconnected )
	{
		SetState(eEpStateConnecting);
		StartCallOffering();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnGuiDisconnectEp()
{
	PTRACE2(eLevelInfoNormal,"CEndpointH323::OnGuiDisconnectEp - disconnect party, Name - ", m_szEpName);

	if( m_enEpState == eEpStateConnected  ||  m_enEpState == eEpStateConnecting )
	{
		m_enDiscoInitiator = eInitiatorMe;

		SetState(eEpStateDisconnecting);

		CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
		pCSProt->AddPortDescriptionHeader(m_partyID, m_confID, m_connectionID, ePhysical_art_light);
		pCSProt->AddCSHeader(m_wCsHandle, 0, m_wCsSrcUnit);

		Disconnect(pCSProt);

		POBJDELETE(pCSProt);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnGuiDeleteEp()
{
	PTRACE2(eLevelInfoNormal,"CEndpointH323::OnGuiDeleteEp - disconnect and delete party, Name - ", m_szEpName);

	m_isToBeDeleted = TRUE;

	//TRACEINTO << "xxName: " << m_szEpName << " to be deleted: " << (DWORD)m_isToBeDeleted;

	if( m_enEpState == eEpStateConnected  ||  m_enEpState == eEpStateConnecting )
	{
		m_enDiscoInitiator = eInitiatorMe;

		SetState(eEpStateDisconnecting);

		CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
		pCSProt->AddPortDescriptionHeader(m_partyID, m_confID, m_connectionID, ePhysical_art_light);
		pCSProt->AddCSHeader(m_wCsHandle, 0, m_wCsSrcUnit);

		Disconnect(pCSProt);

		POBJDELETE(pCSProt);
	}
	else if( m_enEpState == eEpStateIdle || m_enEpState == eEpStateDisconnected || m_enEpState == eEpStateUnknown )
	{
		m_isReadyToDelete = TRUE;
	}
	else
	{  // m_enEpState == eEpStateDisconnecting
		; // nothing to do
}
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::InsertIPandPortToString( char* addressStr,
									   char* prefix,
									   char* ip,
									   char* port,
									   char* aliasName,
									   char* aliasPar1,
											 char* aliasPar2 )
{

	// samples:
	//=========
	// srcPartyAddress =  TA:10.1.4.93:4641,NAME:Richard,TEL:1322
	//	prefix="TA:" ip="10.1.4.93" port="4641" aliasName="NAME:" aliasPar1="Richard,TEL:1322" aliasPar2=NULL
	// destPartyAddress = TA:10.1.39.30:1720,TEL:332270,332270
	//	prefix="TA:" ip="10.1.39.30" port="1720" aliasName="TEL:" aliasPar1="332270" aliasPar2="332270"

	// checking legal parameters
	if ((NULL == addressStr) ||
		(NULL == prefix) ||
		(NULL == ip) ||
		(NULL == port) ||
		(NULL == aliasName) ||
		(NULL == aliasPar1))
	{
		PTRACE(eLevelError,"CEndpointH323::InsertIPandPortToString - NULL Parameter");
		if (addressStr)
			strcpy(addressStr, "Error" );
		return;
	}

	// getting strings len
	WORD len =	strlen(prefix) + strlen(ip) + strlen(port) + strlen(aliasName) + strlen(aliasPar1);
	if (NULL != aliasPar2)
		len += strlen(aliasPar2);

	if ((len + 4) > MaxAddressListSize) // checking sfor legal size
	{
		PTRACE(eLevelError,"CEndpointH323::InsertIPandPortToString - NULL Parameter");
		if (addressStr)
			strcpy(addressStr, "Error" );
		return;
	}

	// creating the string
	if (NULL == aliasPar2)	// xxxPartyAddress = TA:10.1.39.30:1720,TEL:332270
		sprintf( addressStr, "%s%s:%s,%s,%s",prefix, ip, port, aliasName, aliasPar1);
	else					// xxxPartyAddress = TA:10.1.39.30:1720,TEL:332270,332270
		sprintf( addressStr, "%s%s:%s,%s,%s,%s",prefix, ip, port, aliasName, aliasPar1, aliasPar2);

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::GetUniqueConfId( char* unique )
{
	DWORD dwConfID = 80000 + m_arrayIndex;
	std::string confID;
	confID += dwConfID ;
	strcpy( unique, confID.c_str());
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::GetUniqueCallId( char* unique )
{
	DWORD dwCallID = 90000 + m_arrayIndex;
	std::string callID;
	callID += dwCallID ;
	strcpy( unique, callID.c_str());
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendStartChnlCloseInd( const CMplMcmsProtocol* pMplProtocol, TChannelDetails* pChannel )
{
	if(!pMplProtocol || !pChannel)
		return;
	PTRACE(eLevelInfoNormal,"CEndpointH323::SendStartChnlCloseInd - H323_CS_SIG_START_CHANNEL_CLOSE_IND ");
	if( TRUE != pChannel->openedInSim )
	{
		TRACEINTO << " CEndpointH323::SendStartChnlCloseInd - channel already disconnected in SIM, chIndex <"
			<< (int)pChannel->channelIndex << ">, chMcIndex <" << (int)pChannel->channelMcIndex << ">";
		return;
	}

	if( TRUE != pChannel->openedInConf )
	{
		TRACEINTO << " CEndpointH323::SendStartChnlCloseInd - channel already disconnected in CONF, chIndex <"
			<< (int)pChannel->channelIndex << ">, chMcIndex <" << (int)pChannel->channelMcIndex << ">";
		return;
	}

	ALLOCBUFFER(pStr,256);
	sprintf(pStr,"chType <%d>, chIndex <%d>, chMcIndex <%d>, chDirection <%d>.",
			(int)pChannel->channelType,(int)pChannel->channelIndex,
			(int)pChannel->channelMcIndex,(int)pChannel->channelDirection);
	PTRACE2(eLevelInfoNormal,"CEndpointH323::SendStartChnlCloseInd - ",pStr);
	DEALLOCBUFFER(pStr);

	pChannel->openedInSim = FALSE;

	WORD indLen = sizeof(mcIndStartChannelClose);

	mcIndStartChannelClose*  ind = new mcIndStartChannelClose;
	ind->channelType       = pChannel->channelType;
	ind->channelIndex      = pChannel->channelIndex;
	ind->channelDirection  = pChannel->channelDirection;

	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(),
			         H323_CS_SIG_START_CHANNEL_CLOSE_IND,
			         (BYTE*)ind, indLen,
                     m_nCsCallIndex,pChannel->channelIndex,
                     pChannel->channelMcIndex);

	CSegment* pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

	CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	delete ind;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnCallAnswerReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::OnCallAnswerReq - H323_CS_SIG_CALL_ANSWER_REQ");

	// update port description details
	m_confID       = pMplProtocol->getPortDescriptionHeaderConf_id();
	m_partyID      = pMplProtocol->getPortDescriptionHeaderParty_id();
	m_connectionID = pMplProtocol->getPortDescriptionHeaderConnection_id();
//	m_wCsHandle    = pMplProtocol->getCentralSignalingHeaderCsId();
	m_wCsSrcUnit   = pMplProtocol->getCentralSignalingHeaderDestUnitId();

    SetCsHandle(pMplProtocol->getCentralSignalingHeaderCsId());

	// sign the API command
	m_apiArray[SIM_H323_RCV_CALL_ANSWER]++;

	// gets the req struct
	mcReqCallAnswer *req = (mcReqCallAnswer*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointH323::OnCallAnswerReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_mcReqCallAnswer, req, sizeof(mcReqCallAnswer) );

	BYTE* ptr = (BYTE*)m_pEncryptionStruct;
	PDELETEA( ptr );
	m_pEncryptionStruct = NULL;
	if( req->encryTokens.numberOfTokens > 0 )
	{
		APIU32 tokensLen = sizeof(encTokensHeaderBasicStruct) + req->encryTokens.dynamicTokensLen;
		m_pEncryptionStruct = (encTokensHeaderStruct*) new BYTE[tokensLen];
		memcpy( m_pEncryptionStruct, &req->encryTokens, tokensLen );
	}
	/*
	cmTransportAddress1			h245Address = local H245 address;		//The new RV stack struct
	*/

	// save some important parameters
	m_conferenceType = req->conferenceType;	// video type: CP, VSW, ...
	m_maxRate = req->maxRate;

	// send indication
	SendCsCallConnectedInd( pMplProtocol );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnCsCallSetupReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::OnCsCallSetupReq - H323_CS_SIG_CALL_SETUP_REQ");

	SetState(eEpStateConnecting);

	// sign the API command
	m_apiArray[SIM_H323_RCV_SETUP]++;

	// gets the req struct
	mcReqCallSetup *req = (mcReqCallSetup*)pMplProtocol->GetData();
	if (NULL == req)
	{
		PTRACE(eLevelError,"CEndpointH323::OnCsCallSetupReq - GetData is NULL ");
		return;
	}

	APIU32 lengthStructure = sizeof(mcReqCallSetupBase);
//	lengthStructure += (sizeof(encTokensHeaderStruct) - sizeof(encTokensHeaderBasicStruct));

	// save the req struct
	memcpy( &m_mcReqCallSetup, req, lengthStructure );

	BYTE* ptr = (BYTE*)m_pEncryptionStruct;
	PDELETEA( ptr );
	m_pEncryptionStruct = NULL;
	if (::GetEpSystemCfg()->GetEncryptionDialOut())
	{
		if( req->encryTokens.numberOfTokens > 0 )
		{
			APIU32 tokensLen = sizeof(encTokensHeaderBasicStruct) + req->encryTokens.dynamicTokensLen;
			m_pEncryptionStruct = (encTokensHeaderStruct*) new BYTE[tokensLen];
			memcpy( m_pEncryptionStruct, &req->encryTokens, tokensLen );
		}
	}

	// checks legal parameters
	IsLeagaParametersCallSetup( req );


	// set H225 Local ip+port (RMX)
	/////////////////////////////////
	m_H225LocalIpAddress.port = 6005 + (10 * m_arrayIndex);
	if (m_mcReqCallSetup.srcIpAddress.transAddr.port != 0)
		m_H225LocalIpAddress.port = m_mcReqCallSetup.srcIpAddress.transAddr.port;
	m_H225LocalIpAddress.transportType = m_mcReqCallSetup.srcIpAddress.transAddr.transportType;
	m_H225LocalIpAddress.distribution = m_mcReqCallSetup.srcIpAddress.transAddr.distribution;
	m_H225LocalIpAddress.ipVersion = m_mcReqCallSetup.srcIpAddress.transAddr.ipVersion;
	m_H225LocalIpAddress.addr = m_mcReqCallSetup.srcIpAddress.transAddr.addr;


	// set H225 Remote ip+port (E.P.)
	/////////////////////////////////
	m_H225RemoteIpAddress.port = 6006 + (10 * m_arrayIndex);
	if (m_mcReqCallSetup.destIpAddress.transAddr.port != 0)
		m_H225RemoteIpAddress.port = m_mcReqCallSetup.destIpAddress.transAddr.port;
	m_H225RemoteIpAddress.transportType = m_mcReqCallSetup.destIpAddress.transAddr.transportType;
	m_H225RemoteIpAddress.distribution = m_mcReqCallSetup.destIpAddress.transAddr.distribution;
	m_H225RemoteIpAddress.ipVersion = m_mcReqCallSetup.destIpAddress.transAddr.ipVersion;
	m_H225RemoteIpAddress.addr = m_mcReqCallSetup.destIpAddress.transAddr.addr;

	SetIpV6Address();

	// send indication
	SendCsCallConnectedInd( pMplProtocol );
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void CEndpointH323::SetIpV6Address()
{
	if (GetIpVersion() != eIpVersion6)
		return;

	enScopeId remoteScopeid = GetRemoteScopeId();
	enScopeId localScopeid = GetLocalScopeId();


	// set H245 Local ip+port (RMX)
	/////////////////////////////////

	m_H245LocalIpAddress.addr.v6.scopeId = localScopeid;
	if (localScopeid == eScopeIdSite)
		stringToIp(&m_H245LocalIpAddress, "fec0:db8:0:1:172:15:12:12");
	else if (localScopeid == eScopeIdLink)
		stringToIp(&m_H245LocalIpAddress, "fe80:db8:0:1:172:15:12:12");
	else // Global
		stringToIp(&m_H245LocalIpAddress, "2001:db8:0:1:172:15:12:12");

	// fill h245 remote (addrs, ip version, ip, port, distrib, transport type)
	m_H245LocalIpAddress.distribution = eDistributionUnicast;
	m_H245LocalIpAddress.transportType = eTransportTypeTcp;



	// set H245 Remote ip+port (E.P.)
	/////////////////////////////////

	m_H245RemoteIpAddress.addr.v6.scopeId = remoteScopeid;

	if (remoteScopeid == eScopeIdSite)
		stringToIp(&m_H245RemoteIpAddress, "fec0:db8:0:1:172:13:12:12");
	else if (remoteScopeid == eScopeIdLink)
		stringToIp(&m_H245RemoteIpAddress, "fe80:db8:0:1:172:13:12:12");
	else // Global
		stringToIp(&m_H245RemoteIpAddress, "2001:db8:0:1:172:13:12:12");

	// fill h245 remote (addrs, ip version, ip, port, distrib, transport type)
	m_H245RemoteIpAddress.distribution = eDistributionUnicast;
	m_H245RemoteIpAddress.transportType = eTransportTypeTcp;



	// set RTP Local ip+port (RMX)
	/////////////////////////////////

	m_rtpLocalIpAddress.addr.v6.scopeId = localScopeid;

	if (localScopeid == eScopeIdSite)
		stringToIp(&m_rtpLocalIpAddress, "fec0:db8:0:1:172:16:12:12");
	else if (localScopeid == eScopeIdLink)
		stringToIp(&m_rtpLocalIpAddress, "fe80:db8:0:1:172:16:12:12");
	else // Global
		stringToIp(&m_rtpLocalIpAddress, "2001:db8:0:1:172:16:12:12");



	// set RTP Remote ip+port (E.P.)
	/////////////////////////////////

	m_rtpRemoteIpAddress.addr.v6.scopeId = remoteScopeid;

	if (remoteScopeid == eScopeIdSite)
		stringToIp(&m_rtpRemoteIpAddress, "fec0:db8:0:1:172:14:12:12");
	else if (remoteScopeid == eScopeIdLink)
		stringToIp(&m_rtpRemoteIpAddress, "fe80:db8:0:1:172:14:12:12");
	else // Global
		stringToIp(&m_rtpRemoteIpAddress, "2001:db8:0:1:172:14:12:12");

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

int CEndpointH323::IsLeagaParametersCallSetup( mcReqCallSetup *req )
{
	// checks some parameters...
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


enScopeId CEndpointH323::GetLocalScopeId()
{
	char ipStr[128];
	memset (&ipStr, '\0', 128);
	return (getScopeId(ipToString(m_H225LocalIpAddress, ipStr, 1)));
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


enScopeId CEndpointH323::GetRemoteScopeId()
{
	char ipStr[128];
	memset (&ipStr, '\0', 128);
	return (getScopeId(ipToString(m_H225RemoteIpAddress, ipStr, 1)));

}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/*int CEndpointH323::RetrieveIPFromString( WORD srcOrdest)
{
	// destPartyAddress format  "TA:172.22.132.108:1720"

	char tempName[64];
	memset (&tempName, '\0', 64);
	std::string temp;
	if (0 == srcOrdest) // src
		temp = ipToString(m_mcReqCallSetup.srcIpAddress.transAddr, tempName, 1);
	else				// dest
		temp = ipToString(m_mcReqCallSetup.destIpAddress.transAddr, tempName, 1);

	std::string tempip;
	std::string tempport;

	if (temp.length() < 12) {
		return STATUS_FAIL; // error
	}

	int taInd = temp.find("TA:");		// find the start of TA:
	int ta3 = taInd + 3; // start of
	if (taInd == -1) {
		return STATUS_FAIL; // error
	}
	int portInd = temp.find_last_of(":");		// find the start of TA:
	if (ta3 > portInd) {	// the same ':' as "TA:"
		return STATUS_FAIL; // error
	}

	if ((WORD)temp.length() <= (WORD)(portInd + 1)) {
		return STATUS_FAIL; // error
	}

	tempip = temp.substr( ta3, (portInd - ta3));
	tempport = temp.substr( portInd+1, (temp.length() - portInd - 1));


	if (0 == srcOrdest) // src
	{
		// get dest port
		m_H225LocalIpAddress.port = 6005 + (10 * m_arrayIndex);

		if (m_mcReqCallSetup.srcIpAddress.transAddr.port != 0)
			m_H225LocalIpAddress.port = m_mcReqCallSetup.srcIpAddress.transAddr.port;


		if (tempport.length() > 0)
		{
			m_H225LocalIpAddress.port = ::SystemIpStringToDWORD( tempport.c_str() );
		}

		// get dest IP
		if (tempip.length() > 0)
		{
			//strncpy(m_IpH225LocalStr, tempip.c_str(), 15);
			//m_IpH225Local = ::SystemIpStringToDWORD( tempip.c_str() );
			stringToIp(&m_H225LocalIpAddress, (char*)tempip.c_str());
		}
	}
	else	// dest
	{
		// get dest port
		m_H225RemoteIpAddress.port = 6006 + (10 * m_arrayIndex);
		if (tempport.length() > 0)
		{
				m_H225RemoteIpAddress.port = ::SystemIpStringToDWORD( tempport.c_str() );
		}


		if (tempip.length() > 0)
		{
			SetIp(tempip.c_str());
		}
	}

	return STATUS_OK;
}*/

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendCsCallRingBackInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::SendCsCallRingBackInd - H323_CS_SIG_CALL_RING_BACK_IND ");

	// create basic ind
	// ================
	mcIndCallReport *ind = new mcIndCallReport;
	WORD indLen = sizeof(mcIndCallReport);
	memset( ind, 0, indLen);	// zeroing the struct

	// fill indication struct
	// =======================
	// fill h245 remote (addrs, ip version, ip, port, distrib, transport type)

	m_H245RemoteIpAddress.distribution = eDistributionUnicast;
	m_H245RemoteIpAddress.transportType = eTransportTypeTcp;
	FillMcTransportAddress( &ind->h245IpAddress, m_H245RemoteIpAddress);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(),
                     H323_CS_SIG_CALL_RING_BACK_IND,
                     (BYTE*)ind, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

//  m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
//	SendCommandToCS( H323_CS_SIG_CALL_RING_BACK_IND, (BYTE*)ind, indLen);

	delete ind;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendCsCallConnectedInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::SendCsCallConnectedInd - H323_CS_SIG_CALL_CONNECTED_IND");

	// create basic ind
	///////////////////

	WORD indLen = 0;
	if( m_pEncryptionStruct == NULL )
	{
		indLen = sizeof(mcIndCallConnected);
	}
	else
	{
		indLen = sizeof(mcIndCallConnected) + m_pEncryptionStruct->dynamicTokensLen;
	}

	BYTE*  pIndBytes = new BYTE [indLen];
	memset(pIndBytes, 0, indLen);	// zeroing the struct
	mcIndCallConnected *ind = (mcIndCallConnected*)pIndBytes;

	if( m_pEncryptionStruct != NULL )
	{
		memcpy(&(ind->encryTokens),m_pEncryptionStruct,sizeof(encTokensHeaderBasicStruct)+m_pEncryptionStruct->dynamicTokensLen);
		memset(((encryptionToken*)(ind->encryTokens.token))->halfKey,'1',128);
	}

	SetIpV6Address();

	// fill indication struct
	// =======================

	// fill h225 remote (addrs, ip version, ip, port, distrib, transport type)
	m_H225RemoteIpAddress.distribution = eDistributionUnicast;
	m_H225RemoteIpAddress.transportType = eTransportTypeTcp;
	FillMcTransportAddress(&ind->h225remote, m_H225RemoteIpAddress);

	// fill h225 local
	m_H225LocalIpAddress.distribution = eDistributionUnicast;
	m_H225LocalIpAddress.transportType = eTransportTypeTcp;
	FillMcTransportAddress(&ind->h225local, m_H225LocalIpAddress);


	// fill h245 remote
	m_H245RemoteIpAddress.distribution = eDistributionUnicast;
	m_H245RemoteIpAddress.transportType = eTransportTypeTcp;
	FillMcTransportAddress(&ind->h245remote, m_H245RemoteIpAddress);

	// fill h245 local
	m_H245LocalIpAddress.distribution = eDistributionUnicast;
	m_H245LocalIpAddress.transportType = eTransportTypeTcp;
	FillMcTransportAddress(&ind->h245local, m_H245LocalIpAddress);
	// sDisplay
	strncpy( ind->sDisplay, m_sourceAliasNameOrTel + strlen("NAME:"), MaxDisplaySize - 1 );
	ind->sDisplay[MaxDisplaySize - 1] = '\0';

	// userUser
	strncpy( ind->userUser, "User-User Simulation", MaxUserUserSize - 1);
	ind->userUser[MaxUserUserSize - 1] = '\0';
	//int userUserSize = strlen(ind->userUser);

	// more parameters
	ind->remoteEndpointType = 0;		// terminal
	ind->h225RemoteVersion	= 4;		// version
	ind->endPointNetwork	= 2;		// H323
	ind->bAuthenticated		= 0;		// No

	// remoteVendor

	ind->remoteVendor.info.t35CountryCode	= m_rVendor.GetCountryCode();
	ind->remoteVendor.info.t35Extension		= m_rVendor.GetT35Extension();
	ind->remoteVendor.info.manufacturerCode = m_rVendor.GetManufacturerCode();

	strncpy(ind->remoteVendor.productID, m_rVendor.GetProductId(), sizeof(ind->remoteVendor.productID)-1);
	ind->remoteVendor.productID[sizeof(ind->remoteVendor.productID)-1] = '\0';
	ind->remoteVendor.productLen = strlen(ind->remoteVendor.productID);

  strncpy(ind->remoteVendor.versionID, m_rVendor.GetVersionId(), sizeof(ind->remoteVendor.versionID)-1);
  ind->remoteVendor.versionID[sizeof(ind->remoteVendor.versionID)-1] = '\0';
	ind->remoteVendor.versionLen = strlen(ind->remoteVendor.versionID);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(),
                     H323_CS_SIG_CALL_CONNECTED_IND,
			         (BYTE*)ind, indLen, m_nCsCallIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndBytes);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::FillMcTransportAddress( mcXmlTransportAddress* pAddrs,
										APIU32 ipVersion,
										APIU32 ip,
										APIU32 port,
										APIU32 distribution,
										APIU32 transportType)
{
	// fill XML type and size
	pAddrs->unionProps.unionType = ipVersion;
	pAddrs->unionProps.unionSize = sizeof(ipAddressIf);
	pAddrs->transAddr.ipVersion = ipVersion;
	pAddrs->transAddr.addr.v4.ip = ip;
	pAddrs->transAddr.port = port;
	pAddrs->transAddr.distribution = distribution;
	pAddrs->transAddr.transportType = transportType;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::FillMcTransportAddress( mcXmlTransportAddress* pAddrs,
											mcTransportAddress	   mcTA)
{
	// fill XML type and size
	pAddrs->unionProps.unionType = mcTA.ipVersion;
	pAddrs->unionProps.unionSize = sizeof(ipAddressIf);
	pAddrs->transAddr.addr = mcTA.addr;
	pAddrs->transAddr.ipVersion = mcTA.ipVersion;
	pAddrs->transAddr.port = mcTA.port;
	pAddrs->transAddr.distribution = mcTA.distribution;
	pAddrs->transAddr.transportType = mcTA.transportType;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnCsCreateCntlReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::OnCsCreateCntlReq - H323_CS_SIG_CREATE_CNTL_REQ");

	// gets the req struct
	mcReqCreateControl *req = (mcReqCreateControl*)pMplProtocol->GetData();
	if (NULL == req)
	{
		PTRACE(eLevelError,"CEndpointH323::OnCsCreateCntlReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_mcReqCreateControl, req, sizeof(mcReqCreateControl) );

	// Create conference cap set
	m_pConfCap->CreateFrom323Struct(&req->capabilities);

	//set LPR from conf cap to e.p. cap
	m_pCap->SetLPR(m_pConfCap->IsLPR());
	m_isLPRCap = m_pConfCap->IsLPR();

//	CVideoCapH264(const enVideoModeH264 mode,const DWORD ratio=(DWORD)-1, const DWORD staticMB=(DWORD)-1,const DWORD profile=(H264_Profile_BaseLine));
// 	CVideoCapH264* pVideoCap = new CVideoCapH264(eVideoModeHD1080_60,(DWORD)-1,(DWORD)-1,8);
// 	m_pCap->AddVideoProtocol(*pVideoCap);
// 	m_pConfCap->AddVideoProtocol(*pVideoCap);
// 	POBJDELETE(pVideoCap);
// 	CVideoCapH264* 	pVideoCap1 = new CVideoCapH264(eVideoModeHD1080_60,(DWORD)-1,(DWORD)-1,64);
// 	m_pCap->AddVideoProtocol(*pVideoCap1);
// 	m_pConfCap->AddVideoProtocol(*pVideoCap1);
// 	POBJDELETE(pVideoCap1);


	// create highest capabilities and create communication mode
	// temp 1080_60
      	CCapSet  rCommonCap(*m_pCap, *m_pConfCap);
       	m_rComMode.Create(rCommonCap);

	// checks legal parameters
	IsLeagaParametersCreateCntl(req);


	//////////////////
	//Send Indications
	//////////////////

	// Send Cap Ind
	// temp 1080_60
	SendCapabilitiesInd(*m_pCap, pMplProtocol);
	//SendCapabilitiesInd(*m_pConfCap, pMplProtocol);

	// wait a while...

	// Send Cap Response Ind
	SendCsCapResponseInd(pMplProtocol);

	// wait a while...

	// Send Connected Ind
	SendCsCntlConnectedInd(pMplProtocol);


	//////////////////////
	// open media channels
	//////////////////////

		// create common capabilities
	CCapSet  rCapCommon(*m_pCap, *m_pConfCap);
	TChannelDetails*	pChnl = NULL;

	//Audio channels
	if( ( pChnl = GetChannel(cmCapAudio,kRolePeople,cmCapReceive/*dir=IN*/) ) != NULL )
	{
		if( rCapCommon.GetNumAudioAlg() )
		{
			SendIncomingChannelInd( pMplProtocol, pChnl, rCapCommon );
		}
	}

	//Video channels
	if( rCapCommon.GetNumVideoProtocols() )
	{
		//Video
		if( ( pChnl = GetChannel(cmCapVideo,kRolePeople,cmCapReceive/*dir=IN*/) ) != NULL )
		{
			SendIncomingChannelInd( pMplProtocol, pChnl, rCapCommon );
		}

		//Content
		if( rCapCommon.IsH239() )
		{
			if( ( pChnl = GetChannel(cmCapVideo,kRolePresentation,cmCapReceive/*dir=IN*/) ) != NULL )
			{
				SendIncomingChannelInd( pMplProtocol, pChnl, rCapCommon );
		}
		}

		//Fecc
		if( rCapCommon.IsFecc() )
		{
			if( ( pChnl = GetChannel(cmCapData,kRolePeople,cmCapReceive/*dir=IN*/) ) != NULL )
			{
				SendIncomingChannelInd( pMplProtocol, pChnl, rCapCommon );
			}
		}
	}

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CEndpointH323::IsLeagaParametersCreateCntl( mcReqCreateControl *req )
{
	// save Create Control Req - already saved
//	memcpy( &m_mcReqCreateControl, req, sizeof(mcReqCreateControl) );

	//h245IpAddress;				// H245 address
	//masterSlaveTerminalType;		// for cascade
	// capabilitiesStructLength;
	// ctCapabilitiesStruct		capabilities;

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendCapabilitiesInd( const CCapSet& cap, CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::SendCapabilitiesInd - H323_CS_SIG_CAPABILITIES_IND");

	BYTE* pIndication = NULL;
	//  create cap set for declaration
	CCapSet  rCapset(cap);
	if( m_pEncryptionStruct == NULL )
		rCapset.SetEncryption(FALSE);

	WORD  indLen = rCapset.CreateCapStructH323(&pIndication, cmCapReceive);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(),
                     H323_CS_SIG_CAPABILITIES_IND,
                     pIndication, indLen, m_nCsCallIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

//  m_pCSApi->SendMsg(pMsg,SEND_TO_CSAPI);

	POBJDELETE(pCSProt);

	PDELETEA(pIndication);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnRssCmdReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::OnRssCmdReq - H323_CS_SIG_CREATE_CNTL_REQ");

	// gets the req struct
	mcReqRssCommand *req = (mcReqRssCommand*)pMplProtocol->GetData();
	if (NULL == req)
	{
		PTRACE(eLevelError,"CEndpointH323::OnRssCmdReq - GetData is NULL ");
		return;
	}

	if(req->subOpcode == eRssCmdLiveStream)
		PTRACE(eLevelError,"CEndpointH323::OnRssCmdReq - Streaming Indication");
	if(req->subOpcode == eRssCmdExchangeID)
	{
		PTRACE(eLevelError,"CEndpointH323::OnRssCmdReq - Exchange server ID");
		if(req->data.length != 0)
			PTRACE2(eLevelError,"CEndpointH323::OnRssCmdReq - Exchange server ID", req->data.paramBuffer);
	}
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendCsCapResponseInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::SendCsCapResponseInd - H323_CS_SIG_CAP_RESPONSE_IND");

	// create basic ind
	// ================
	BYTE *ind = new BYTE[2];
	WORD indLen = 2;

	// fills indication struct (empty struct)
	// =======================
	ind[0] = 1;
	ind[1] = 2;

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(),
                     H323_CS_SIG_CAP_RESPONSE_IND,
                     ind, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(ind);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendCsCntlConnectedInd( CMplMcmsProtocol* pMplProtocol, APIU32 masterSlaveStat )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::SendCsCntlConnectedInd - H323_CS_SIG_CALL_CNTL_CONNECTED_IND");

	// create basic ind
	// ================
	mcIndCallControlConnected *ind = new mcIndCallControlConnected;
	WORD indLen = sizeof(mcIndCallControlConnected);
	memset( ind, 0, indLen);	// zeroing the struct

	// fill indication struct
	// =======================
	// fill h245 remote    ( addrs,  ip version,  ip,  port,  distrib, transport type)
	m_H245RemoteIpAddress.distribution = eDistributionUnicast;
	m_H245RemoteIpAddress.transportType = eTransportTypeTcp;
	FillMcTransportAddress( &ind->remoteH245Address, m_H245RemoteIpAddress);

	// fill h245 local     ( addrs,  ip version,  ip,  port,  distrib, transport type)
	m_H245RemoteIpAddress.distribution = eDistributionUnicast;
	m_H245RemoteIpAddress.transportType = eTransportTypeTcp;
	FillMcTransportAddress( &ind->localH245Address, m_H245LocalIpAddress);

	// master cascade
//	printf("CEndpointH323::SendCsCntlConnectedInd %d \n",m_dialDirection);
	if(m_dialDirection == DIAL_IN)// in case of dial in cascade we set the linker to slave
		ind->masterSlaveStatus = cmMSSlave;		// no
	else
		ind->masterSlaveStatus = cmMSMaster;		// no

	// userUser
	strcpy( ind->userUser, "userUser Sim");
	//int userUserSize = strlen(ind->userUser);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(),
                     H323_CS_SIG_CALL_CNTL_CONNECTED_IND,
                     (BYTE*)ind, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	delete ind;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnCsIncomingChnlResponse( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::OnCsIncomingChnlResponse - H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ ");

	// gets the req struct
	mcReqIncomingChannelResponse *req = (mcReqIncomingChannelResponse*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointH323::OnCsIncomingChnlResponse - GetData is NULL ");
		return;
	}

	// checks legal parameters
	IsLegalParametersIncomingChnlResponse( req );

	// save the req struct
	memcpy( &m_mcIncomingChannelResponse, req, sizeof(mcReqIncomingChannelResponse) );

	TChannelDetails*  pChannel = GetChannelByIndex(m_mcIncomingChannelResponse.channelIndex);
//	TChannelDetails*  pChannel = GetChannel(m_mcIncomingChannelResponse.dataType,cmCapReceive/*IN*/);
	if( pChannel == NULL ) {
		DBGPASSERT(1000+m_mcIncomingChannelResponse.dataType);
		DBGPASSERT(1000+m_mcIncomingChannelResponse.channelDirection);
		DBGPASSERT(1000+m_mcIncomingChannelResponse.channelIndex);
		return;
	}

/*	if( pChannel->channelIndex != m_mcIncomingChannelResponse.channelIndex ) {
		DBGPASSERT(1000+m_mcIncomingChannelResponse.channelIndex);
		return;
	}*/

	 // if status is not OK, remove channel
	if( STATUS_OK != pMplProtocol->getCentralSignalingHeaderStatus() )
	{
		pChannel->openedInConf = FALSE;
		pChannel->openedInSim  = FALSE;
		PTRACE(eLevelInfoNormal,"CEndpointH323::OnCsIncomingChnlResponse - status not OK, remove channel.");
		if( eEpStateDisconnecting == m_enEpState )
			WhenAllChannelsClosed(pMplProtocol);

		return;
	}

	pChannel->channelMcIndex = pMplProtocol->getCentralSignalingHeaderMcChannelIndex();
	pChannel->openedInConf = TRUE;

	if( req->bIsEncrypted != 0 )
	{
		if( m_pEncryptionStruct != NULL )
		{
			if( kAES_CBC != req->encryptionAlgorithm )
				DBGPASSERT(req->encryptionAlgorithm);
		}
		else
			DBGPASSERT(req->bIsEncrypted);
	}
	else // not encrypted
	{
		if( m_pEncryptionStruct != NULL )
			DBGPASSERT(req->bIsEncrypted+1000);
	}

	CCapSet  rCapCommon(*m_pCap,*m_pConfCap);
	if( pChannel->channelType == cmCapAudio ) {

		m_apiArray[SIM_H323_RCV_INCOMING_A_CHNL]++;
		// send indication
		if (m_apiArray[SIM_H323_SND_INCOMING_A_CHNL] == 0)	// only if the remote didn't send
			SendIncomingChannelInd(pMplProtocol,pChannel,rCapCommon);
		SendIncomingChannelConnectedInd(pMplProtocol,pChannel);

	} else if(pChannel->channelType == cmCapVideo ) {

		if( pChannel->channelRole == kRolePeople )
		m_apiArray[SIM_H323_RCV_INCOMING_V_CHNL]++;
		else
			m_apiArray[SIM_H323_RCV_INCOMING_239_CHNL]++;
		// send indication
			// if people
		if( pChannel->channelRole == kRolePeople  &&  m_apiArray[SIM_H323_SND_INCOMING_V_CHNL] == 0 )
			SendIncomingChannelInd(pMplProtocol,pChannel,rCapCommon);
			// if content
		else if( pChannel->channelRole == kRolePresentation  &&  m_apiArray[SIM_H323_SND_INCOMING_239_CHNL] == 0 )
			SendIncomingChannelInd(pMplProtocol,pChannel,rCapCommon);
		SendIncomingChannelConnectedInd(pMplProtocol,pChannel);

	} else if(pChannel->channelType == cmCapData ) {

		m_apiArray[SIM_H323_RCV_INCOMING_F_CHNL]++;
		// send indication
		if (m_apiArray[SIM_H323_SND_INCOMING_F_CHNL] == 0)	// only if the remote didn't send
			SendIncomingChannelInd(pMplProtocol,pChannel,rCapCommon);
		SendIncomingChannelConnectedInd(pMplProtocol,pChannel);

	}

	if( IsConnectionCompleted() == TRUE )
		SetState(eEpStateConnected);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CEndpointH323::IsLegalParametersIncomingChnlResponse( mcReqIncomingChannelResponse *req )
{
/*	if( req->channelIndex != m_tChannelAudioIncoming.channelIndex )
	{
		PASSERTMSG(req->channelIndex,
			"CEndpointH323::IsLeagaParametersIncomingChnlResponse - illegal. req->channelIndex = ");
		PASSERTMSG(m_tChannelAudioIncoming.channelIndex,
			"CEndpointH323::IsLeagaParametersIncomingChnlResponse - illegal. m_tChannelAudioIncoming->channelIndex = ");

		return STATUS_FAIL;
	}*/
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendIncomingChannelInd(CMplMcmsProtocol* pMplProtocol, TChannelDetails* channel, const CCapSet& rCapCommon)
{
	TRACEINTO << "chType:" << (int)channel->channelType << ", chIndex:" << (int)channel->channelIndex << ", chMcIndex:" << (int)channel->channelMcIndex << ", chDirection:" << (int)channel->channelDirection;

	channel->openedInSim = TRUE;

	APIU8 localCapTypeCode = eG711Ulaw64kCapCode;

	// size of message = static + dynamic
	// ===================================
	WORD indLen = sizeof(mcIndIncomingChannel) + sizeof(channelSpecificParameters);

	// allocate memory for indication - static + dynamic
	BYTE* pIndicationBytes = new BYTE[indLen];
	memset(pIndicationBytes, 0, indLen); // zeroing the struct

	mcIndIncomingChannel* pIncomChannInd = (mcIndIncomingChannel*) pIndicationBytes;
	channelSpecificParameters* pChnlSpecific  = (channelSpecificParameters*) &(pIncomChannInd->channelSpecificParams);

	// fill indication struct
	// =======================
	pIncomChannInd->dataType         = channel->channelType;      // cmCapDataType
	pIncomChannInd->channelIndex     = channel->channelIndex;     // same as incoming
	pIncomChannInd->channelDirection = channel->channelDirection; // incoming (cmCapReceive=in, cmCapTransmit=out)

	// fill RTP remote    ( addrs, ip version, ip, port, distrib, transport type)
	m_rtpRemoteIpAddress.distribution  = eDistributionUnicast;
	m_rtpRemoteIpAddress.transportType = eTransportTypeUdp;
	FillMcTransportAddress(&pIncomChannInd->rmtRtpAddress, m_rtpRemoteIpAddress);

	pIncomChannInd->bIsActive               = DONOT_CARE;         // active channel
	pIncomChannInd->sameSessionChannelIndex = DONOT_CARE;         // DONOT_CARE
	pIncomChannInd->sessionId               = DONOT_CARE;         // DONOT_CARE
	pIncomChannInd->dynamicPayloadType      = DONOT_CARE;         // DONOT_CARE

	if (m_pEncryptionStruct == NULL)
	{
		pIncomChannInd->bIsEncrypted        = 0;                    // NO
		pIncomChannInd->encryptionAlgorithm = 0;                    // NO
	}
	else
	{
		pIncomChannInd->bIsEncrypted        = 1;
		pIncomChannInd->encryptionAlgorithm = kAES_CBC;
	}

	pIncomChannInd->bUsedH263Plus = 0;

	pIncomChannInd->xmlDynamicProps.numberOfDynamicParts  = 1;
	pIncomChannInd->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(channelSpecificParameters);

	pIncomChannInd->sizeOfChannelParams = sizeof(channelSpecificParameters); // according to the channel you indented to open.

	if (channel->channelType == cmCapAudio)
	{
		switch ((localCapTypeCode = rCapCommon.GetAudioAlgType(0)))
		{
			case eG711Alaw64kCapCode:
			{
				pIncomChannInd->rate = 640;// 64k
				pIncomChannInd->payloadType = _PCMA;
				strcpy(pIncomChannInd->channelName, "g711Alaw64kCapCode");// just a name of channel
				pChnlSpecific->pAudioBase.maxValue = NonFrameBasedFPP;// 20
				break;
			}
			case eG711Alaw56kCapCode:
			{
				pIncomChannInd->rate = 560;// 56k
				pIncomChannInd->payloadType = _PCMA;
				strcpy(pIncomChannInd->channelName, "g711Alaw56kCapCode");
				pChnlSpecific->pAudioBase.maxValue = NonFrameBasedFPP;
				break;
			}
			case eG711Ulaw64kCapCode:
			{
				pIncomChannInd->rate = 640;// 64k
				pIncomChannInd->payloadType = _PCMU;
				strcpy(pIncomChannInd->channelName, "g711Ulaw64kCapCode");
				pChnlSpecific->pAudioBase.maxValue = NonFrameBasedFPP;
				break;
			}
			case eG711Ulaw56kCapCode:
			{
				pIncomChannInd->rate = 560;// 56k
				pIncomChannInd->payloadType = _PCMU;
				strcpy(pIncomChannInd->channelName, "g711Ulaw64kCapCode");
				pChnlSpecific->pAudioBase.maxValue = NonFrameBasedFPP;
				break;
			}
			case eG722_64kCapCode:
			{
				pIncomChannInd->rate = 640;// 64k
				pIncomChannInd->payloadType = _G722;
				strcpy(pIncomChannInd->channelName, "g722_64kCapCode");
				pChnlSpecific->pAudioBase.maxValue = NonFrameBasedFPP;
				break;
			}
			case eG722_56kCapCode:
			{
				pIncomChannInd->rate = 560;// 56k
				pIncomChannInd->payloadType = _G722;
				strcpy(pIncomChannInd->channelName, "g722_56kCapCode");
				pChnlSpecific->pAudioBase.maxValue = NonFrameBasedFPP;
				break;
			}
			case eG722_48kCapCode:
			{
				pIncomChannInd->rate = 480;// 48k
				pIncomChannInd->payloadType = _G722;
				strcpy(pIncomChannInd->channelName, "g722_48kCapCode");
				pChnlSpecific->pAudioBase.maxValue = NonFrameBasedFPP;
				break;
			}
			case eG722Stereo_128kCapCode:
			{
				pIncomChannInd->rate = 1280;// 128k
				pIncomChannInd->payloadType = _G722;
				strcpy(pIncomChannInd->channelName, "g722Stereo_128kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG728CapCode:
			{
				pIncomChannInd->rate = 160;// 16k
				pIncomChannInd->payloadType = _G728;
				strcpy(pIncomChannInd->channelName, "g728CapCode");
				pChnlSpecific->pAudioBase.maxValue = NonFrameBasedFPP;
				break;
			}
			case eG729CapCode:
			{
				pIncomChannInd->rate = 80;// 8k
				pIncomChannInd->payloadType = _G729;
				strcpy(pIncomChannInd->channelName, "g729CapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG729AnnexACapCode:
			{
				pIncomChannInd->rate = 80;// 8k
				pIncomChannInd->payloadType = _G729;
				strcpy(pIncomChannInd->channelName, "g729AnnexACapCode");
				pChnlSpecific->pAudioBase.maxValue = 2 * FrameBasedFPP;
				break;
			}
			case eG729wAnnexBCapCode:
			{
				pIncomChannInd->rate = 80;// 8k
				pIncomChannInd->payloadType = _G729;
				strcpy(pIncomChannInd->channelName, "g729wAnnexBCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG729AnnexAwAnnexBCapCode:
			{
				pIncomChannInd->rate = 80;// 8k
				pIncomChannInd->payloadType = _G729;
				strcpy(pIncomChannInd->channelName, "g729AnnexAwAnnexBCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG7231CapCode:
			{
				pIncomChannInd->rate = 70;// 7k
				pIncomChannInd->payloadType = _G7231;
				strcpy(pIncomChannInd->channelName, "g7231CapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG7221_32kCapCode:
			{
				pIncomChannInd->rate = 320;// 32k
				pIncomChannInd->payloadType = _G7221;
				strcpy(pIncomChannInd->channelName, "g7221_32kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG7221_24kCapCode:
			{
				pIncomChannInd->rate = 240;// 24k
				pIncomChannInd->payloadType = _G7221;
				strcpy(pIncomChannInd->channelName, "g7221_24kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG7221_16kCapCode:
			{
				pIncomChannInd->rate = 160;// 16k
				pIncomChannInd->payloadType = _G7221;
				strcpy(pIncomChannInd->channelName, "g7221_16kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren7_16kCapCode:
			{
				pIncomChannInd->rate		= 160; // 16k
				pIncomChannInd->payloadType	= _Siren7;
				strcpy( pIncomChannInd->channelName, "eSiren7_16kCapCode" );
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren14_48kCapCode:
			{
				pIncomChannInd->rate = 480;// 48k
				pIncomChannInd->payloadType = _Siren14;
				strcpy(pIncomChannInd->channelName, "Siren14_48kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren14_32kCapCode:
			{
				pIncomChannInd->rate = 320;// 32k
				pIncomChannInd->payloadType = _Siren14;
				strcpy(pIncomChannInd->channelName, "Siren14_32kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren14_24kCapCode:
			{
				pIncomChannInd->rate = 240;// 24k
				pIncomChannInd->payloadType = _Siren14;
				strcpy(pIncomChannInd->channelName, "Siren14_24kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eOpus_CapCode:
			{
				pIncomChannInd->rate		= 640; // 64k
				pIncomChannInd->payloadType	= _Opus;
				strcpy( pIncomChannInd->channelName, "Opus_CapCode" );
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG7221C_48kCapCode:
			{
				pIncomChannInd->rate = 480;// 32k
				pIncomChannInd->payloadType = _G7221;
				strcpy(pIncomChannInd->channelName, "g7221C_48kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG7221C_32kCapCode:
			{
				pIncomChannInd->rate = 320;// 24k
				pIncomChannInd->payloadType = _G7221;
				strcpy(pIncomChannInd->channelName, "g7221C_32kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG7221C_24kCapCode:
			{
				pIncomChannInd->rate = 240;// 16k
				pIncomChannInd->payloadType = _G7221;
				strcpy(pIncomChannInd->channelName, "g7221C_24kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren14Stereo_48kCapCode:
			{
				pIncomChannInd->rate = 480;// 48k
				pIncomChannInd->payloadType = _Siren14S;
				strcpy(pIncomChannInd->channelName, "Siren14Stereo_48kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren14Stereo_64kCapCode:
			{
				pIncomChannInd->rate = 640;// 32k
				pIncomChannInd->payloadType = _Siren14S;
				strcpy(pIncomChannInd->channelName, "Siren14Stereo_64kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren14Stereo_96kCapCode:
			{
				pIncomChannInd->rate = 960;// 24k
				pIncomChannInd->payloadType = _Siren14S;
				strcpy(pIncomChannInd->channelName, "Siren14Stereo_96kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eOpusStereo_CapCode:
			{
				pIncomChannInd->rate		= 1280; // 128k
				pIncomChannInd->payloadType	= _Opus;
				strcpy( pIncomChannInd->channelName, "OpusStereo_CapCode" );
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG719_32kCapCode:
			{
				pIncomChannInd->rate = 320;// 24k
				pIncomChannInd->payloadType = _G719;
				strcpy(pIncomChannInd->channelName, "g719_32kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG719_48kCapCode:
			{
				pIncomChannInd->rate = 480;// 24k
				pIncomChannInd->payloadType = _G719;
				strcpy(pIncomChannInd->channelName, "g719_48kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG719_64kCapCode:
			{
				pIncomChannInd->rate = 640;// 24k
				pIncomChannInd->payloadType = _G719;
				strcpy(pIncomChannInd->channelName, "g719_64kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG719Stereo_64kCapCode:
			{
				pIncomChannInd->rate = 640;// 24k
				pIncomChannInd->payloadType = _G719S;
				strcpy(pIncomChannInd->channelName, "g719Stereo_64kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG719Stereo_96kCapCode:
			{
				pIncomChannInd->rate = 960;// 24k
				pIncomChannInd->payloadType = _G719S;
				strcpy(pIncomChannInd->channelName, "g719Stereo_96kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eG719Stereo_128kCapCode:
			{
				pIncomChannInd->rate = 1280;// 128k
				pIncomChannInd->payloadType = _G719S;
				strcpy(pIncomChannInd->channelName, "g719Stereo_128kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren22_32kCapCode:
			{
				pIncomChannInd->rate = 320;// 32k
				pIncomChannInd->payloadType = _Siren22;
				strcpy(pIncomChannInd->channelName, "Siren22_32kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren22_48kCapCode:
			{
				pIncomChannInd->rate = 480;// 32k
				pIncomChannInd->payloadType = _Siren22;
				strcpy(pIncomChannInd->channelName, "Siren22_48kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren22_64kCapCode:
			{
				pIncomChannInd->rate = 640;// 32k
				pIncomChannInd->payloadType = _Siren22;
				strcpy(pIncomChannInd->channelName, "Siren22_64kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren22Stereo_64kCapCode:
			{
				pIncomChannInd->rate = 640;// 32k
				pIncomChannInd->payloadType = _Siren22S;
				strcpy(pIncomChannInd->channelName, "Siren22Stereo_64kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren22Stereo_96kCapCode:
			{
				pIncomChannInd->rate = 960;// 32k
				pIncomChannInd->payloadType = _Siren22S;
				strcpy(pIncomChannInd->channelName, "Siren22Stereo_96kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSiren22Stereo_128kCapCode:
			{
				pIncomChannInd->rate = 1280;// 128k
				pIncomChannInd->payloadType = _Siren22S;
				strcpy(pIncomChannInd->channelName, "Siren22Stereo_128kCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
			case eSirenLPR_32kCapCode:
			{
				pIncomChannInd->rate = 320;// 32k
				pIncomChannInd->payloadType = _SirenLPR;
				strcpy(pIncomChannInd->channelName, "SirenLPR_32kCapCode");
				pChnlSpecific->pSirenLPR.maxValue     = FrameBasedFPP;
				pChnlSpecific->pSirenLPR.sirenLPRMask = sirenLPRMono;
				break;
			}
			case eSirenLPR_48kCapCode:
			{
				pIncomChannInd->rate = 480;// 32k
				pIncomChannInd->payloadType = _SirenLPR;
				strcpy(pIncomChannInd->channelName, "SirenLPR_48kCapCode");
				pChnlSpecific->pSirenLPR.maxValue     = FrameBasedFPP;
				pChnlSpecific->pSirenLPR.sirenLPRMask = sirenLPRMono;
				break;
			}
			case eSirenLPR_64kCapCode:
			{
				pIncomChannInd->rate = 640;// 32k
				pIncomChannInd->payloadType = _SirenLPR;
				strcpy(pIncomChannInd->channelName, "SirenLPR_64kCapCode");
				pChnlSpecific->pSirenLPR.maxValue     = FrameBasedFPP;
				pChnlSpecific->pSirenLPR.sirenLPRMask = sirenLPRMono;
				break;
			}
			case eSirenLPRStereo_64kCapCode:
			{
				pIncomChannInd->rate = 640;// 32k
				pIncomChannInd->payloadType = _SirenLPR;
				strcpy(pIncomChannInd->channelName, "SirenLPRStereo_64kCapCode");
				pChnlSpecific->pSirenLPR.maxValue     = FrameBasedFPP;
				pChnlSpecific->pSirenLPR.sirenLPRMask = sirenLPRStereo;
				break;
			}
			case eSirenLPRStereo_96kCapCode:
			{
				pIncomChannInd->rate = 960;// 32k
				pIncomChannInd->payloadType = _SirenLPR;
				strcpy(pIncomChannInd->channelName, "SirenLPRStereo_96kCapCode");
				pChnlSpecific->pSirenLPR.maxValue     = FrameBasedFPP;
				pChnlSpecific->pSirenLPR.sirenLPRMask = sirenLPRStereo;
				break;
			}
			case eSirenLPRStereo_128kCapCode:
			{
				pIncomChannInd->rate = 1280;// 32k
				pIncomChannInd->payloadType = _SirenLPR;
				strcpy(pIncomChannInd->channelName, "SirenLPRStereo_128kCapCode");
				pChnlSpecific->pSirenLPR.maxValue     = FrameBasedFPP;
				pChnlSpecific->pSirenLPR.sirenLPRMask = sirenLPRStereo;
				break;
			}
			case eRfc2833DtmfCapCode:
			{
				pIncomChannInd->rate = (DWORD)-1;// NA
				pIncomChannInd->payloadType = _Rfc2833Dtmf;
				strcpy(pIncomChannInd->channelName, "Rfc2833DtmfCapCode");
				pChnlSpecific->pAudioBase.maxValue = FrameBasedFPP;
				break;
			}
		}
		m_apiArray[SIM_H323_SND_INCOMING_A_CHNL]++;

		pIncomChannInd->capTypeCode                  = localCapTypeCode;
		pChnlSpecific->pAudioBase.minValue           = 0;
		pChnlSpecific->pAudioBase.header.direction   = cmCapTransmit;
		pChnlSpecific->pAudioBase.header.type        = channel->channelType;
		pChnlSpecific->pAudioBase.header.roleLabel   = kRolePeople;                  // kRolePeople;
		pChnlSpecific->pAudioBase.header.capTypeCode = localCapTypeCode;             // eG711Ulaw64kCapCode

		pChnlSpecific->pAudioBase.header.xmlHeader.dynamicType   = localCapTypeCode; // eG711Ulaw64kCapCode
		pChnlSpecific->pAudioBase.header.xmlHeader.dynamicLength = sizeof(channelSpecificParameters);
	}
	else if (channel->channelType == cmCapVideo)
	{
		if (kRolePeople == channel->channelRole)
		{
			const CVideoCap* pVideoCap = rCapCommon.GetVideoProtocol(0);
			DWORD maxVideoBitRate = (rCapCommon.GetCallRate() - 64) * 10;

			if (pVideoCap != NULL)
			{
				switch ((localCapTypeCode = pVideoCap->GetPayloadType()))
				{
					case eH261CapCode:
					{
						pIncomChannInd->rate = maxVideoBitRate;
						pIncomChannInd->payloadType = _H261;
						pIncomChannInd->capTypeCode = localCapTypeCode;
						strcpy(pIncomChannInd->channelName, "h261CapCode");

						((CVideoCapH261*)pVideoCap)->FillStructH323(&(pChnlSpecific->p261), maxVideoBitRate, cmCapTransmit, kRolePeople);

						// change the "maxBitRate" to create error (simulation)
						DWORD errorBitRate = ::GetEpSystemCfg()->GetErrorBitRate();          // in K
						if (0 != errorBitRate)
						{
							pChnlSpecific->p261.maxBitRate += (errorBitRate * 10);
							TRACEINTO << "ErrorBitRate:" << errorBitRate;
						}
						break;
					}
					case eH263CapCode:
					{
						pIncomChannInd->rate = maxVideoBitRate;
						pIncomChannInd->payloadType = _H263;
						pIncomChannInd->capTypeCode = localCapTypeCode;
						strcpy(pIncomChannInd->channelName, "h263CapCode");

						((CVideoCapH263*)pVideoCap)->FillStructH323(&(pChnlSpecific->p263), maxVideoBitRate, cmCapTransmit, kRolePeople);

						// change the "maxBitRate" to create error (simulation)
						DWORD errorBitRate = ::GetEpSystemCfg()->GetErrorBitRate();          // in K
						if (0 != errorBitRate)
						{
							pChnlSpecific->p263.maxBitRate += (errorBitRate * 10);
							TRACEINTO << "ErrorBitRate:" << errorBitRate;
						}
						break;
					}
					case eH264CapCode:
					{
						pIncomChannInd->rate = maxVideoBitRate;
						pIncomChannInd->payloadType = _H264;
						pIncomChannInd->capTypeCode = localCapTypeCode;
						strcpy(pIncomChannInd->channelName, "h264CapCode");

						h264CapStruct* pH264CapStruct = &(pChnlSpecific->p264);
						((CVideoCapH264*)pVideoCap)->FillStructH323(pH264CapStruct, maxVideoBitRate, cmCapTransmit, kRolePeople);

						// Checking for assymetric 1080p60 mode (IncomingChannelInd should be overridden to 720p60)
						if (pH264CapStruct->customMaxMbpsValue >= 432 * 2)
						{
							pH264CapStruct->customMaxFsValue   = 15;
							pH264CapStruct->customMaxMbpsValue = 432;
						}

						// change the "maxBitRate" to create error (simulation)
						DWORD errorBitRate = ::GetEpSystemCfg()->GetErrorBitRate(); // in K
						if (0 != errorBitRate)
						{
							pChnlSpecific->p264.maxBitRate += (errorBitRate * 10);
							TRACEINTO << "ErrorBitRate:" << errorBitRate;
						}
						break;
					}
				}
				pIncomChannInd->bIsLPR = m_isLPRCap;
				m_apiArray[SIM_H323_SND_INCOMING_V_CHNL]++;
			}
		}
		else // kRolePresentation
		{
			DWORD bitRate = rCapCommon.GetH239Rate() * 10; // from 128 kb to 1280
			const CVideoCap* pPresCap = rCapCommon.GetPresentationVideo();
			if (bitRate > 0 && NULL != pPresCap)
			{
				pIncomChannInd->rate = bitRate;
				pIncomChannInd->capTypeCode = pPresCap->GetPayloadType();
				if (strcmp(pPresCap->NameOf(), "CVideoCapH264") == 0)
				{
					PTRACE(eLevelInfoNormal, "CEndpointH323::SendIncomingChannelInd - Incoming content channel H264");
					pIncomChannInd->payloadType = _H264;
					strcpy(pIncomChannInd->channelName, "h264CapCode");
					((CVideoCapH264*)pPresCap)->FillStructH323(&(pChnlSpecific->p264), bitRate, cmCapTransmit, kRolePresentation);
				}
				else
				{
					PTRACE(eLevelInfoNormal, "CEndpointH323::SendIncomingChannelInd - Incoming content channel H263");
					pIncomChannInd->payloadType = _H263;
					strcpy(pIncomChannInd->channelName, "h263CapCode");
					((CVideoCapH263*)pPresCap)->FillStructH323(&(pChnlSpecific->p263), bitRate, cmCapTransmit, kRolePresentation);
				}
				pIncomChannInd->bIsLPR = m_isLPRCap;
				m_apiArray[SIM_H323_SND_INCOMING_239_CHNL]++;
			}
			DWORD maxVideoBitRate = (rCapCommon.GetCallRate() - 48) * 10;
		}
	}
	else if (channel->channelType == cmCapData)
	{
		if (rCapCommon.IsFecc())
		{
			pIncomChannInd->rate = 640;                          // should be the rate of the channel you want to open.
			pIncomChannInd->payloadType = _RvFecc;               // can be _G722. It should be the pay load type of the channel you want to open.
			pIncomChannInd->capTypeCode = eRvFeccCapCode;
			strcpy(pIncomChannInd->channelName, "rvFeccCapCode");// just a name

			pChnlSpecific->pData.header.direction = cmCapTransmit;
			pChnlSpecific->pData.header.type = channel->channelType;
			pChnlSpecific->pData.header.roleLabel = kRolePeople; // kRolePeople;
			pChnlSpecific->pData.header.capTypeCode = eRvFeccCapCode;
			pChnlSpecific->pData.header.xmlHeader.dynamicType = eRvFeccCapCode;
			pChnlSpecific->pData.header.xmlHeader.dynamicLength = sizeof(dataCapStructBase);
			pChnlSpecific->pData.maxBitRate = 64; // means 6.4k

			m_apiArray[SIM_H323_SND_INCOMING_F_CHNL]++;
		}
	}

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(),
	                 H323_CS_SIG_INCOMING_CHANNEL_IND,
	                 pIndicationBytes, indLen,
	                 m_nCsCallIndex, channel->channelIndex,
	                 channel->channelMcIndex);

	CSegment* pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

	CCSSimTaskApi api(GetCSID());
	if (api.CreateOnlyApi() >= 0)
		api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETEA(pIndicationBytes);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendIncomingChannelConnectedInd( CMplMcmsProtocol* pMplProtocol, const TChannelDetails* pChannel )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::SendIncomingChannelConnectedInd - H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND");

//	m_apiArray[SIM_H323_SND_INCOMING_CHNL]++;
	if( pChannel->channelDirection != cmCapReceive )
		DBGPASSERT(1000+pChannel->channelDirection);

	ALLOCBUFFER(pStr,256);

	sprintf(pStr, "chType <%d>, chIndex <%d>, chMcIndex <%d>, chDirection <%d>.",
			(int)pChannel->channelType, (int)pChannel->channelIndex,
			(int)pChannel->channelMcIndex, (int)pChannel->channelDirection);
	PTRACE2(eLevelInfoNormal, "CEndpointH323::SendIncomingChannelConnectedInd - ", pStr);

	DEALLOCBUFFER(pStr);

	// size of message = static + dynamic
	// ===================================
	WORD indLen = sizeof(mcIndIncomingChannelConnected);

	mcIndIncomingChannelConnected* ind =  new mcIndIncomingChannelConnected;

	// fills indication struct
	// =======================
	ind->channelType		= pChannel->channelType;	// cmCapAudio or cmCapVideo
	ind->channelIndex		= pChannel->channelIndex;	// same as incoming
	ind->channelDirection	= 0;						// incoming (0=in, 1=out)
	ind->sessionId			= DONOT_CARE;				// DONOT_CARE
	ind->sameSessionChannelIndex = DONOT_CARE;			// DONOT_CARE
	ind->associatedChannelIndex  = DONOT_CARE;			// DONOT_CARE

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol* pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(),
			         H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND,
			         (BYTE*)ind, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	PDELETE(ind);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnOutgoingChnlReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::OnOutgoingChnlReq - H323_CS_SIG_OUTGOING_CHNL_REQ ");

	// Vasily - this message contents static AND dynamic parts
	//     currently we don't use dynamic
	//  length of dynamic - req->sizeOfChannelParams
	//  starts in         - req->channelSpecificParams

	// gets the req struct
	mcReqOutgoingChannel *req = (mcReqOutgoingChannel*)pMplProtocol->GetData();
	if ( NULL == req )
	{
		PTRACE(eLevelError,"CEndpointH323::OnOutgoingChnlReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_outGoingChannelReq, req, sizeof(mcReqOutgoingChannel));

	channelSpecificParameters* pChnlSpecific = (channelSpecificParameters*) &(req->channelSpecificParams);

	TChannelDetails*  pChannel = GetChannel(m_outGoingChannelReq.channelType,
									pChnlSpecific->pAudioBase.header.roleLabel,
									cmCapTransmit/*OUT*/);
	if( pChannel == NULL )
	{
		DBGPASSERT(1000+m_outGoingChannelReq.channelType);
		DBGPASSERT(1000+m_outGoingChannelReq.channelDirection);
		DBGPASSERT(1000+pChnlSpecific->pAudioBase.header.roleLabel);
		return;
	}

	if( req->bIsEncrypted != 0 )
	{
		if( m_pEncryptionStruct != NULL )
		{
			if( kAES_CBC != req->encryptionAlgorithm )
				DBGPASSERT(req->encryptionAlgorithm);
		}
		else
			DBGPASSERT(req->bIsEncrypted);
	}
	else // not encrypted
	{
		if( m_pEncryptionStruct != NULL )
			DBGPASSERT(req->bIsEncrypted+1000);
	}

	if( pChannel->channelType == cmCapAudio )
	{
		m_apiArray[SIM_H323_RCV_OUTGOING_A_CHNL]++;
	}
	else if( pChannel->channelType == cmCapVideo )
	{
		if( kRolePeople == pChannel->channelRole )
			m_apiArray[SIM_H323_RCV_OUTGOING_V_CHNL]++;
		else
			m_apiArray[SIM_H323_RCV_OUTGOING_239_CHNL]++;
	}
	else if( pChannel->channelType == cmCapData )
	{
		m_apiArray[SIM_H323_RCV_OUTGOING_F_CHNL]++;
	}


	pChannel->channelMcIndex = pMplProtocol->getCentralSignalingHeaderMcChannelIndex();
	pChannel->openedInConf = TRUE;

	// send indication
	SendOutgoingChannelResponseInd( pMplProtocol, pChannel );

	// for H239 channel
/*	if( kRolePresentation == pChannel->channelRole )
	{
		TChannelDetails* pInChannel = GetChannel(pChannel->channelType,pChannel->channelRole,cmCapReceive);
		if( NULL != pInChannel )
		{
			CCapSet  rCapCommon(*m_pCap,*m_pConfCap);
			SendIncomingChannelInd(pMplProtocol,pInChannel,rCapCommon);
		}
		else
		{
			DBGPASSERT(1000+pChannel->channelType);
			DBGPASSERT(1000+pChannel->channelRole);
		}
	}*/

	if( IsConnectionCompleted() == TRUE )
		SetState(eEpStateConnected);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendOutgoingChannelResponseInd( CMplMcmsProtocol* pMplProtocol, TChannelDetails* pChannel )
{
	PTRACE(eLevelInfoNormal, "CEndpointH323::SendOutgoingChannelResponseInd - H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND ");

	ALLOCBUFFER(pStr, 256);
	sprintf(pStr,"chType <%d>, chIndex <%d>, chMcIndex <%d>, chDirection <%d>.",
			(int)pChannel->channelType, (int)pChannel->channelIndex,
			(int)pChannel->channelMcIndex, (int)pChannel->channelDirection);

	PTRACE2(eLevelInfoNormal, "CEndpointH323::SendOutgoingChannelResponseInd - ", pStr);
	DEALLOCBUFFER(pStr);

	pChannel->openedInSim  = TRUE;

	if( pChannel->channelType == cmCapAudio )
		m_apiArray[SIM_H323_SND_OUTGOING_A_CHNL]++;
	else if( pChannel->channelType == cmCapVideo )
	{
		if( pChannel->channelRole == kRolePeople )
		m_apiArray[SIM_H323_SND_OUTGOING_V_CHNL]++;
		else
			m_apiArray[SIM_H323_SND_OUTGOING_239_CHNL]++;
	}
	else if( pChannel->channelType == cmCapData )
		m_apiArray[SIM_H323_SND_OUTGOING_F_CHNL]++;

	// create basic ind
	// ================
	mcIndOutgoingChannelResponse *ind = new mcIndOutgoingChannelResponse;
	WORD indLen = sizeof(mcIndOutgoingChannelResponse);
	memset(ind, 0, indLen);	// zeroing the struct

	// fills indication struct
	// =======================
	ind->channelType		= pChannel->channelType;				// audio || video || data || non standard ...
	ind->channelIndex		= pChannel->channelIndex;				// as req;
	ind->channelDirection	= pChannel->channelDirection;			// 0 - In / 1 - Out
	ind->sessionId			= DONOT_CARE;							// Session Id of channel
	ind->payloadType		= m_outGoingChannelReq.payloadType;		// Confirmation from the EP.
	ind->sameSessionChannelIndex	= DONOT_CARE;					// ?? - RTP associated
	ind->associatedChannelIndex		= DONOT_CARE;					// ?? - RTP associated


	// fill RTP remote    ( addrs,  ip version, ip, port, distrib, transport type)
	m_rtpRemoteIpAddress.distribution = eDistributionUnicast;
	m_rtpRemoteIpAddress.transportType = eTransportTypeUdp;
	FillMcTransportAddress( &ind->destRtpAddress, m_rtpRemoteIpAddress);

	// send the struct to CS simulation
	// ================================
	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(),
					 H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND,
					 (BYTE*) ind, indLen, 0,
					 pChannel->channelIndex, pChannel->channelMcIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
	delete ind;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnChnlDropReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::OnChnlDropReq - H323_CS_SIG_CHNL_DROP_REQ ");

//	SetState(eEpStateDisconnecting);

	// gets the req struct
	mcReqChannelDrop *req = (mcReqChannelDrop*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointH323::OnChnlDropReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_mcReqChannelDrop, req, sizeof(mcReqChannelDrop));

	// checks legal parameters
	//IsLeagaParametersChannelDrop( req );

	TChannelDetails*  pChannel = GetChannelByIndex(m_mcReqChannelDrop.channelIndex);
//	TChannelDetails*  pChannel = GetChannel(m_mcReqChannelDrop.channelType,m_mcReqChannelDrop.channelDirection);
	if( pChannel == NULL ) {
		DBGPASSERT(1000+m_mcReqChannelDrop.channelType);
		DBGPASSERT(1000+m_mcReqChannelDrop.channelDirection);
		return;
	}

	if( pChannel->channelIndex != m_mcReqChannelDrop.channelIndex ) {
		DBGPASSERT(1000+m_mcReqChannelDrop.channelIndex);
		return;
	}

	pChannel->openedInConf = FALSE;

	// send the struct to CS simulation
	// ================================
	SendChannelCloseInd(pMplProtocol,pChannel);

	if( eInitiatorUnknown != m_enDiscoInitiator ) // if it disconnection, not channel closing
		WhenAllChannelsClosed(pMplProtocol);
	else
	{
		//ReOpen media channel
		CCapSet  CommonCap(*m_pCap,*m_pConfCap);
		m_rComMode.Create(CommonCap);

		TChannelDetails *present_video=GetChannel(cmCapVideo,kRolePresentation,cmCapReceive/*dir=IN*/);
		if(present_video==pChannel)
		{
			if(m_rComMode.IsH239ToOpen() && m_rComMode.GetPresentVideoMode()!=eUnknownAlgorithemCapCode)
			{
				SendIncomingChannelInd(pMplProtocol, present_video, CommonCap);
			}
			return;
		}

		TChannelDetails *people_video=GetChannel(cmCapVideo,kRolePeople,cmCapReceive/*dir=IN*/);
		if(people_video==pChannel)
		{
			if(m_isReopenChannel && m_lenReopenTimer!=0xFFFF)
			{
				if(m_rComMode.IsVideoToOpen())
				{
					//delay a few second to reopen incoming channel
					CSegment *pTimerSegment = new CSegment;
					*pTimerSegment << (void*)people_video;
					StartTimer(SIM_H323_TOUT_DELAY_SEND_INCOMING_CHANNEL_IND, m_lenReopenTimer * SECOND , pTimerSegment);
				}
			}
			return;
		}

	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::WhenAllChannelsClosed( CMplMcmsProtocol* pMplProtocol )
{
	//TRACEINTO << "xxName: " << m_szEpName << " m_enDiscoInitiator: " << (DWORD)m_enDiscoInitiator;

//	m_taChannels[eAudioChannelIn]
	for( int i=0; i<eNumOfChannels; i++ )
		if( TRUE == m_taChannels[i].openedInSim || TRUE == m_taChannels[i].openedInConf )
			return;

	PTRACE(eLevelInfoNormal,"CEndpointH323::WhenAllChannelsClosed - All channels closed.");

	SendCallIdleInd(pMplProtocol);

		// if ConfParty is disconnection initiator, we don't receive confirm => clear all
	if( m_enDiscoInitiator == eInitiatorConf )
	{
		PTRACE(eLevelInfoNormal,"CEndpointH323::WhenAllChannelsClosed - Conf is disconnect initiator, clean all.");
		CleanAfterDisconnect();

		SetState(eEpStateDisconnected);

		if( TRUE == m_isToBeDeleted )
			m_isReadyToDelete = TRUE;

		//TRACEINTO << "xxName: " << m_szEpName << " ready to delete: " << (DWORD)m_isReadyToDelete;
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendChannelCloseInd( const CMplMcmsProtocol* pMplProtocol, TChannelDetails* pChannel )
{
	if(!pMplProtocol || !pChannel)
		return;
	PTRACE(eLevelInfoNormal,"CEndpointH323::SendChannelCloseInd - H323_CS_SIG_CHANNEL_CLOSE_IND");
	ALLOCBUFFER(pStr,256);
	sprintf(pStr,"chType <%d>, chIndex <%d>, chMcIndex <%d>, chDirection <%d>.",
			(int)pChannel->channelType,(int)pChannel->channelIndex,
			(int)pChannel->channelMcIndex,(int)pChannel->channelDirection);
	PTRACE2(eLevelInfoNormal,"CEndpointH323::SendChannelCloseInd - ",pStr);
	DEALLOCBUFFER(pStr);

	pChannel->openedInSim = FALSE;

	WORD indLen = sizeof(mcReqChannelDrop);

	mcReqChannelDrop*  ind = new mcReqChannelDrop;
	ind->channelType       = pChannel->channelType;
	ind->channelIndex      = pChannel->channelIndex;
	ind->channelDirection = pChannel->channelDirection;

	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(),
					 H323_CS_SIG_CHANNEL_CLOSE_IND,
					 (BYTE*)ind, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

POBJDELETE(pCSProt);
	delete ind;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnCallDropReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::OnCallDropReq - H323_CS_SIG_CALL_DROP_REQ ");

	m_enDiscoInitiator = eInitiatorConf;

	// gets the req struct
	mcReqCallDrop *req = (mcReqCallDrop*)pMplProtocol->GetData();
	if (NULL == req) {
		PTRACE(eLevelError,"CEndpointH323::OnCallDropReq - GetData is NULL ");
		return;
	}

	// save the req struct
	memcpy( &m_mcReqCallDrop, req, sizeof(mcReqCallDrop));
	m_apiArray[SIM_H323_RCV_CALL_DROP]++;

	// checks legal parameters
	//IsLegalParametersChannelDrop( req );
	SetState(eEpStateDisconnecting);

	//check if there are no open channels
	BOOL bConnectedChannelFound = FALSE;
	for( int i=0; i<eNumOfChannels; i++ )
		if( TRUE == m_taChannels[i].openedInSim || TRUE == m_taChannels[i].openedInConf )
		{
			bConnectedChannelFound = TRUE;
			break;
		}

	if (!bConnectedChannelFound)
		WhenAllChannelsClosed(pMplProtocol);	//no channels connected - send CALL_IDLE_IND
	else
		Disconnect(pMplProtocol);	//there are open channels - disconnect channels
}


/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::Disconnect( CMplMcmsProtocol* pMplProtocol )
{
	// AUDIO
	if( m_rComMode.IsAudioToOpen() )
	{
		// send channel drop for Incoming channel:
		SendStartChnlCloseInd( pMplProtocol, &(m_taChannels[eAudioChannelIn]) );
		// send channel drop for Outgoing channel:
		SendStartChnlCloseInd( pMplProtocol, &(m_taChannels[eAudioChannelOut]) );
	}

	if( m_rComMode.IsVideoToOpen() ) {
		// VIDEO
		// send channel drop for Incoming channel:
		SendStartChnlCloseInd( pMplProtocol, &(m_taChannels[eVideoChannelIn]) );
		// send channel drop for Outgoing channel:
		SendStartChnlCloseInd( pMplProtocol, &(m_taChannels[eVideoChannelOut]) );
		// If FECC is open
		if( m_rComMode.IsFeccToOpen() ) {
			// FECC
			// send channel drop for Incoming channel:
			SendStartChnlCloseInd( pMplProtocol, &(m_taChannels[eFeccChannelIn]) );
			// send channel drop for Outgoing channel:
			SendStartChnlCloseInd( pMplProtocol, &(m_taChannels[eFeccChannelOut]) );
		}
	}

	if( m_rComMode.IsH239ToOpen() )
	{
		// send channel drop for Incoming channel:
		SendStartChnlCloseInd( pMplProtocol, &(m_taChannels[eH239ChannelIn]) );
		// send channel drop for Outgoing channel:
		SendStartChnlCloseInd( pMplProtocol, &(m_taChannels[eH239ChannelOut]) );
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendCallIdleInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::SendCallIdleInd - H323_CS_SIG_CALL_IDLE_IND ");

	// send the Call Idle struct to CS simulation
	// ================================
	WORD indLen = sizeof(mcReqCallDrop);
	mcReqCallDrop  ind;

	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);
	// if received call_drop_req
	if( m_apiArray[SIM_H323_RCV_CALL_DROP] > 0 )
		ind.rejectCallReason = m_mcReqCallDrop.rejectCallReason;
	else
		ind.rejectCallReason = 0;

	::FillCsProtocol(pCSProt, GetCSID(),
					 H323_CS_SIG_CALL_IDLE_IND,
					 (BYTE*)&ind, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg, CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnCallCloseConfirmReq( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE(eLevelInfoNormal,"CEndpointH323::OnCallCloseConfirmReq - H323_CS_SIG_CALL_CLOSE_CONFIRM_REQ ");

	if( m_enEpState != eEpStateDisconnecting )
	{
		PASSERTMSG(1000+m_enEpState," CEndpointH323::OnCallCloseConfirmReq - Wrong state ");
		return;
	}

		// check for all channels closed
	for( int i=0; i<eNumOfChannels; i++ )
	{
		if( TRUE == m_taChannels[i].openedInSim )
		{
			PASSERTMSG(1000+i," CEndpointH323::OnCallCloseConfirmReq - channel open in sim ");
			return;
		}
		if( TRUE == m_taChannels[i].openedInConf )
		{
			PASSERTMSG(1000+i," CEndpointH323::OnCallCloseConfirmReq - channel open in conf ");
			return;
		}
	}

	CleanAfterDisconnect();

	SetState(eEpStateDisconnected);

	if( TRUE == m_isToBeDeleted )
		m_isReadyToDelete = TRUE;

	//TRACEINTO << "xxName: " << m_szEpName << " ready to delete: " << (DWORD)m_isReadyToDelete;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SetChannelDetails( TChannelDetails* pChannel, const DWORD channelType, const DWORD channelRole,
									   const DWORD channelIndex, const DWORD channelMcIndex, const DWORD channelDirection,
									   const BOOL openedInSim, const BOOL openedInConf )
{
	pChannel->channelType  = channelType;
	pChannel->channelRole  = channelRole;
	pChannel->channelIndex = channelIndex;
	pChannel->channelMcIndex   = channelMcIndex;
	pChannel->channelDirection = channelDirection;
	pChannel->openedInSim  = openedInSim;
	pChannel->openedInConf = openedInConf;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
TChannelDetails* CEndpointH323::GetChannel(const DWORD channelType,const DWORD channelRole,const DWORD channelDirection)
{
	for( int i=0; i<eNumOfChannels; i++ )
	{
		if( m_taChannels[i].channelType == channelType )
			if( m_taChannels[i].channelRole == channelRole )
				if( m_taChannels[i].channelDirection == channelDirection )
					return &(m_taChannels[i]);
	}

	return NULL;
}

bool CEndpointH323::IsIncomingChannCanOpen(TChannelDetails *chann)
{
	if(!chann)
	{
		DBGPASSERT(0);
		return false;
	}
	if(chann->channelDirection!=cmCapReceive)
		return false;

	if(chann->channelType==cmCapVideo)
	{
		if(chann->channelRole==kRolePresentation)
		{
			return m_rComMode.IsH239ToOpen();
		}
		else if(chann->channelRole==kRolePeople)
		{
			return m_rComMode.IsVideoToOpen();
		}
	}
	else if(chann->channelType==cmCapAudio)
	{
		// TODO:
	}
	else if(chann->channelType==cmCapData)
	{
		// TODO:
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
TChannelDetails* CEndpointH323::GetChannelByIndex(const DWORD channelIndex)
{
	for( int i=0; i<eNumOfChannels; i++ )
	{
		if( m_taChannels[i].channelIndex == channelIndex )
			return &(m_taChannels[i]);
	}

	return NULL;
}


/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointH323::IsConnectionCompleted() const
{
	if( m_rComMode.IsAudioToOpen() )
	{
		if( m_apiArray[SIM_H323_SND_INCOMING_A_CHNL] == 0 || m_apiArray[SIM_H323_SND_OUTGOING_A_CHNL] == 0 )
			return FALSE;
		if( m_taChannels[eAudioChannelIn].openedInSim == FALSE  ||  m_taChannels[eAudioChannelIn].openedInConf == FALSE )
			return FALSE;
		if( m_taChannels[eAudioChannelOut].openedInSim == FALSE  ||  m_taChannels[eAudioChannelOut].openedInConf == FALSE )
			return FALSE;
	}

	if( m_rComMode.IsVideoToOpen() )
	{
		if( m_apiArray[SIM_H323_SND_INCOMING_V_CHNL] == 0 || m_apiArray[SIM_H323_SND_OUTGOING_V_CHNL] == 0 )
			return FALSE;
		if( m_taChannels[eVideoChannelIn].openedInSim == FALSE  ||  m_taChannels[eVideoChannelIn].openedInConf == FALSE )
			return FALSE;
		if( m_taChannels[eVideoChannelOut].openedInSim == FALSE  ||  m_taChannels[eVideoChannelOut].openedInConf == FALSE )
			return FALSE;
	}

	if( m_rComMode.IsFeccToOpen() )
	{
		if( m_apiArray[SIM_H323_SND_INCOMING_F_CHNL] == 0 || m_apiArray[SIM_H323_SND_OUTGOING_F_CHNL] == 0 )
			return FALSE;
		if( m_taChannels[eFeccChannelIn].openedInSim == FALSE  ||  m_taChannels[eFeccChannelIn].openedInConf == FALSE )
			return FALSE;
		if( m_taChannels[eFeccChannelOut].openedInSim == FALSE  ||  m_taChannels[eFeccChannelOut].openedInConf == FALSE )
			return FALSE;
	}

	// Presentation channels may be opened later

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::CleanAfterDisconnect()
{
	int i = 0;
		// clean commands array
	for( i=0; i<SIM_H323_MAX_API_COMMANDS; i++ )
		m_apiArray[i] = 0;

		// clean channels MCMS indexes
	for( i=0; i<(int)eNumOfChannels; i++ )
	{
		m_taChannels[i].channelMcIndex = 0;
		m_taChannels[i].openedInSim  = FALSE;
		m_taChannels[i].openedInConf = FALSE;
	}

	// clean conf caps and comm mode
	m_pConfCap->Empty();
	m_rComMode.Create(*m_pConfCap);

		// clean conf/party details
	m_confID = m_partyID = m_connectionID = 0xFFFFFFFF;

		// clean audio board details
	m_rAudioBoard.CleanDetails();

		// clean Muted flag
	m_isMuted = FALSE;

		// clean all requests structures
	memset(&m_mcReqCallSetup,0,sizeof(mcReqCallSetup));
	memset(&m_mcReqCreateControl,0,sizeof(mcReqCreateControl));
	memset(&m_outGoingChannelReq,0,sizeof(mcReqOutgoingChannel));
	memset(&m_mcIncomingChannelResponse,0,sizeof(mcReqIncomingChannelResponse));
	memset(&m_mcReqCallAnswer,0,sizeof(mcReqCallAnswer));
	memset(&m_mcReqChannelDrop,0,sizeof(mcReqChannelDrop));
	memset(&m_mcReqCallDrop,0,sizeof(mcReqCallDrop));

		// encryption
	BYTE* ptr = (BYTE*)m_pEncryptionStruct;
	PDELETEA( ptr );
	m_pEncryptionStruct = NULL;

	m_nMcuId = m_nTerminalId = 0;
	m_enRoleTokenLastCmd = kUnknownRoleTokenOpcode;
	DeleteTimer(ROLE_TOKEN_OWNER_TOUT);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendMuteIndication() const
{
	PTRACE2(eLevelError,"CEndpointH323::SendMuteIndication - H323_CS_CHANNEL_OFF_IND, Name - ", m_szEpName );

	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
	pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
	pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

	const TChannelDetails* pChannel = &(m_taChannels[eAudioChannelIn]);

	::FillCsProtocol(pCSProt, GetCSID(),
					 H323_CS_CHANNEL_OFF_IND, NULL, 0,
					 0, pChannel->channelIndex, pChannel->channelMcIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendUnmuteIndication() const
{
	PTRACE2(eLevelError,"CEndpointH323::SendUnmuteIndication - H323_CS_CHANNEL_ON_IND, Name - ", m_szEpName );

	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
	pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
	pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

	const TChannelDetails* pChannel = &(m_taChannels[eAudioChannelIn]);

	::FillCsProtocol(pCSProt, GetCSID(),
			         H323_CS_CHANNEL_ON_IND, NULL, 0,
			         0, pChannel->channelIndex, pChannel->channelMcIndex);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointH323::IsFeccCall() const
{
	if( m_rComMode.IsFeccToOpen() )
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointH323::IsH239Call() const
{
	if( m_rComMode.IsH239ToOpen() )
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointH323::IsLprCall() const
{
	return m_isLPRCap;
}
/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendFeccTokenRequestIndication() const
{
	PTRACE2(eLevelError,"CEndpointH323::SendFeccTokenRequestIndication, Name - ", m_szEpName );

	CSegment  rMsgSeg;

	rMsgSeg << (DWORD)IP_RTP_FECC_KEY_IND;//IP_RTP_FECC_TOKEN_IND;

	rMsgSeg << m_rAudioBoard.GetBoardId()
			<< m_rAudioBoard.GetSubBoardId()
			<< m_rAudioBoard.GetUnitId();

	rMsgSeg << m_confID
			<< m_partyID
			<< m_connectionID;

	const TChannelDetails* pChannel = &(m_taChannels[eFeccChannelIn]);

	TRtpFeccTokenRequestInd  rTokenStruct;
	memset(&rTokenStruct,0,sizeof(TRtpFeccTokenRequestInd));
	rTokenStruct.unChannelType      = pChannel->channelType;
	rTokenStruct.unChannelDirection = pChannel->channelDirection;
	rTokenStruct.unTokenOpcode      = (DWORD)kTokenRequest;

	rMsgSeg.Put((BYTE*)(&rTokenStruct),sizeof(TRtpFeccTokenRequestInd));

	::SendAudioMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendFeccTokenReleaseIndication() const
{
	PTRACE2(eLevelError,"CEndpointH323::SendFeccTokenReleaseIndication, Name - ", m_szEpName );

	CSegment  rMsgSeg;

	rMsgSeg << (DWORD)IP_RTP_FECC_TOKEN_IND;//(DWORD)IP_RTP_FECC_KEY_IND;

	rMsgSeg << m_rAudioBoard.GetBoardId()
			<< m_rAudioBoard.GetSubBoardId()
			<< m_rAudioBoard.GetUnitId();

	rMsgSeg << m_confID
			<< m_partyID
			<< m_connectionID;

	const TChannelDetails* pChannel = &(m_taChannels[eFeccChannelIn]);

	TRtpFeccTokenRequestInd  rTokenStruct;
	memset(&rTokenStruct,0,sizeof(TRtpFeccTokenRequestInd));
	rTokenStruct.unChannelType      = pChannel->channelType;
	rTokenStruct.unChannelDirection = pChannel->channelDirection;
	rTokenStruct.unTokenOpcode      = (DWORD)kTokenRelease; //(DWORD)eFeccKeyUp;

	rMsgSeg.Put((BYTE*)(&rTokenStruct),sizeof(TRtpFeccTokenRequestInd));

	::SendAudioMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendFeccKeyRequestIndication(const char* pszDtmf) const
{
        PTRACE2(eLevelError,"CEndpointH323::SendFeccKeyRequestIndication, Name - ", m_szEpName );

        CSegment  rMsgSeg;

        rMsgSeg << (DWORD)IP_RTP_FECC_KEY_IND;//IP_RTP_FECC_TOKEN_IND;

        rMsgSeg << m_rAudioBoard.GetBoardId()
                        << m_rAudioBoard.GetSubBoardId()
                        << m_rAudioBoard.GetUnitId();

        rMsgSeg << m_confID
                        << m_partyID
                        << m_connectionID;

        const TChannelDetails* pChannel = &(m_taChannels[eFeccChannelIn]);

        TRtpFeccTokenRequestInd  rTokenStruct;
        memset(&rTokenStruct,0,sizeof(TRtpFeccTokenRequestInd));
        rTokenStruct.unChannelType      = pChannel->channelType;
        rTokenStruct.unChannelDirection = pChannel->channelDirection;
        //rTokenStruct.unTokenOpcode      = (DWORD)kTokenRequest;
        int nKey = atoi(pszDtmf);
        feccKeyEnum feccKey = feccKeyEnum(nKey);
        rTokenStruct.unTokenOpcode      = (DWORD)feccKey;

        rMsgSeg.Put((BYTE*)(&rTokenStruct),sizeof(TRtpFeccTokenRequestInd));

        ::SendAudioMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendH239TokenRequestIndication()
{
	PTRACE2(eLevelError,"CEndpointH323::SendH239TokenRequestIndication, Name - ", m_szEpName );

	const TChannelDetails* pChannel = &(m_taChannels[eH239ChannelIn]);

	if( NULL != pChannel )
	{
		CCapSet  rCapCommon(*m_pCap,*m_pConfCap);

		CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
		pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
		pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

		mcIndRoleToken  ind;
		DWORD  indLen = sizeof(mcIndRoleToken);
		memset(&ind,0,indLen);

		ind.subOpcode  = kPresentationTokenRequest;
		ind.bitRate    = 0; // rCapCommon.GetH239Rate() * 10;
		ind.mcuID      = m_nMcuId;
		ind.terminalID = m_nTerminalId;
//		ind.label      = LABEL_CONTENT;
		ind.randNumber = RAND_NUM_FOR_EP_SIM;

		::FillCsProtocol(pCSProt, GetCSID(),
						 H323_CS_SIG_CALL_ROLE_TOKEN_IND,
						 (BYTE*)&ind, indLen,
						 m_nCsCallIndex, pChannel->channelIndex,
						 pChannel->channelMcIndex);

		CSegment* pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

		CCSSimTaskApi api(GetCSID());
		if(api.CreateOnlyApi() >= 0)
		    api.SendMsg(pMsg, SEND_TO_CSAPI);

		POBJDELETE(pCSProt);

		m_enRoleTokenLastCmd = kPresentationTokenRequest;
	}
	else
		DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendH239TokenReleaseIndication()
{
	PTRACE2(eLevelError,"CEndpointH323::SendH239TokenReleaseIndication, Name - ", m_szEpName );

	const TChannelDetails* pChannel = &(m_taChannels[eH239ChannelIn]);

	if( NULL != pChannel )
	{
//		CCapSet  rCapCommon(*m_pCap,*m_pConfCap);

		CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
		pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
		pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

		mcIndRoleToken  ind;
		DWORD  indLen = sizeof(mcIndRoleToken);
		memset(&ind,0,indLen);

		ind.subOpcode  = kPresentationTokenRelease;
		ind.bitRate    = 0; // rCapCommon.GetH239Rate() * 10;
		ind.mcuID      = m_nMcuId;
		ind.terminalID = m_nTerminalId;
//		ind.label      = LABEL_CONTENT;
		ind.randNumber = RAND_NUM_FOR_EP_SIM;

		::FillCsProtocol(pCSProt, GetCSID(),
						 H323_CS_SIG_CALL_ROLE_TOKEN_IND,
						 (BYTE*)&ind, indLen,
						 m_nCsCallIndex, pChannel->channelIndex,
						 pChannel->channelMcIndex);

		CSegment* pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

		CCSSimTaskApi api(GetCSID());
		if(api.CreateOnlyApi() >= 0)
			api.SendMsg(pMsg, SEND_TO_CSAPI);

		POBJDELETE(pCSProt);
		m_enRoleTokenLastCmd = kPresentationTokenRelease;
	}
	else
		DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendH239TokenWithdrawResponse()
{
	PTRACE2(eLevelError,"CEndpointH323::SendH239TokenWithdrawResponse, Name - ", m_szEpName );

	const TChannelDetails* pChannel = &(m_taChannels[eH239ChannelIn]);

	if( NULL != pChannel )
	{
//		CCapSet  rCapCommon(*m_pCap,*m_pConfCap);

		CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
		pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
		pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

		mcIndRoleToken  ind;
		DWORD  indLen = sizeof(mcIndRoleToken);
		memset(&ind,0,indLen);

		ind.subOpcode  = kPresentationTokenResponse;
		ind.bitRate    = 0; // rCapCommon.GetH239Rate() * 10;
		ind.mcuID      = m_nMcuId;
		ind.terminalID = m_nTerminalId;
//		ind.label      = LABEL_CONTENT;
		ind.randNumber = RAND_NUM_FOR_EP_SIM;
		ind.bIsAck     = 1;

		::FillCsProtocol(pCSProt, GetCSID(),
						 H323_CS_SIG_CALL_ROLE_TOKEN_IND,
						 (BYTE*)&ind, indLen,
						 m_nCsCallIndex, pChannel->channelIndex,
						 pChannel->channelMcIndex);

		CSegment* pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

		CCSSimTaskApi api(GetCSID());
		if(api.CreateOnlyApi() >= 0)
			api.SendMsg(pMsg, SEND_TO_CSAPI);

		POBJDELETE(pCSProt);
		m_enRoleTokenLastCmd = kUnknownRoleTokenOpcode;
	}
	else
		DBGPASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnRoleTokenReq(CMplMcmsProtocol* pMplProtocol)
{
	PTRACE2(eLevelError,"CEndpointH323::OnRoleTokenReq, Name - ", m_szEpName );

	mcReqRoleTokenMessage* pReq = (mcReqRoleTokenMessage*)pMplProtocol->GetData();

	DBGPASSERT(pReq->mcuID != m_nMcuId);
	TRACESTR(eLevelError) << " CEndpointH323::OnRoleTokenReq - IsAck <" << (int)pReq->bIsAck << ">";

	switch( pReq->subOpcode )
	{
		case kPresentationTokenRequest:
		{
			DBGPASSERT(pReq->terminalID != m_nTerminalId);

			DeleteTimer(ROLE_TOKEN_OWNER_TOUT);

			PTRACE2(eLevelError,"CEndpointH323::OnRoleTokenReq - Withdraw for TokenRequest received, Name - ", m_szEpName );

			SendH239TokenWithdrawResponse();
			break;
		}
		case kPresentationTokenResponse:
		{
			DBGPASSERT(pReq->terminalID != m_nTerminalId);
	//		DBGPASSERT(1!=pReq->bIsAck);

			DeleteTimer(ROLE_TOKEN_OWNER_TOUT);
			if( 1 == pReq->bIsAck ) // if is_ack
			{
					// ack for request_token
				if( kPresentationTokenRequest == m_enRoleTokenLastCmd )
				{
					StartTimer(ROLE_TOKEN_OWNER_TOUT,ROLE_TOKEN_OWNER_TIME*SECOND);
					PTRACE2(eLevelError,"CEndpointH323::OnRoleTokenReq - Ack for TokenRequest received, Name - ", m_szEpName );
				}
					// ack for release_token
				else if( kPresentationTokenRelease == m_enRoleTokenLastCmd )
				{
					PTRACE2(eLevelError,"CEndpointH323::OnRoleTokenReq - Ack for TokenRelease received, Name - ", m_szEpName );
				}
			}
			else
			{
				PTRACE2(eLevelError,"CEndpointH323::OnRoleTokenReq - NACK for TokenRequest received, Name - ", m_szEpName );
			}

			m_enRoleTokenLastCmd = kUnknownRoleTokenOpcode;

			break;
		}
		case kPresentationTokenIndicateOwner:
		{
			DBGPASSERT(pReq->terminalID == m_nTerminalId);

			PTRACE2(eLevelError,"CEndpointH323::OnRoleTokenReq - PresentationTokenOwner received, Name - ", m_szEpName );

			break;
		}
		default:
		{
			TRACESTR(eLevelError)
				<< " CEndpointH323::OnRoleTokenReq - Unknown Token request received, subOpcode <"
				<< (int)pReq->subOpcode	<< ">, EpName - " << m_szEpName;
			break;
		}
	}
}

void CEndpointH323::OnChnlNewRateReq(CMplMcmsProtocol* pMplProtocol)
{
	return;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::OnRoleTokenOwnerTout(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CEndpointH323::OnRoleTokenOwnerTout, Name - ", m_szEpName );

	const TChannelDetails* pChannel = &(m_taChannels[eH239ChannelIn]);

	if( NULL != pChannel )
	{
		CCapSet  rCapCommon(*m_pCap,*m_pConfCap);

		CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
		pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
		pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

		mcIndRoleToken  ind;
		DWORD  indLen = sizeof(mcIndRoleToken);
		memset(&ind,0,indLen);

		ind.subOpcode  = kPresentationTokenIndicateOwner;
		ind.bitRate    = rCapCommon.GetH239Rate() * 10;
		ind.mcuID      = m_nMcuId;
		ind.terminalID = m_nTerminalId;
//		ind.label      = LABEL_CONTENT;
		ind.randNumber = RAND_NUM_FOR_EP_SIM;

		::FillCsProtocol(pCSProt, GetCSID(),
						 H323_CS_SIG_CALL_ROLE_TOKEN_IND,
						 (BYTE*)&ind, indLen,
					     m_nCsCallIndex, pChannel->channelIndex,
					     pChannel->channelMcIndex);

		CSegment* pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

		CCSSimTaskApi api(GetCSID());
		if(api.CreateOnlyApi() >= 0)
			api.SendMsg(pMsg, SEND_TO_CSAPI);

		POBJDELETE(pCSProt);
	}
	else
		DBGPASSERT(1);

	StartTimer(ROLE_TOKEN_OWNER_TOUT,ROLE_TOKEN_OWNER_TIME*SECOND);
}

void CEndpointH323::OnTimerDelaySendIncomingChanInd(CSegment* pParam)
{
	//There is no need to delete timer when it's time out

	//ReOpen media channel
	CCapSet  CommonCap(*m_pCap,*m_pConfCap);
	m_rComMode.Create(CommonCap);

	void *pvoid=0;
	*pParam >> pvoid;
	TChannelDetails *pChannel=(TChannelDetails*)pvoid;
	if(IsIncomingChannCanOpen(pChannel))
	{
		CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
		pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID);
		pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

		SendIncomingChannelInd(pCSProt, pChannel, CommonCap);

		POBJDELETE(pCSProt);
	}
}

//return action   0--do nothing   1--close   2--open
int CEndpointH323::GetActionForChannel(bool isOpened, DWORD prevPT, DWORD newPY)
{
	if(isOpened)
	{
		if(prevPT!=newPY)
			return 1;
	}
	else
	{
		if(newPY!=eUnknownAlgorithemCapCode)
			return 2;
	}
	return 0;
}

void CEndpointH323::UpdateIncomingChannByNewCap(TChannelDetails *pChannel, const CCapSet &rCapCommon)
{
	if(!pChannel)
	{
		DBGPASSERT(0);
		return;
	}
	if(pChannel->channelDirection!=cmCapReceive)
		return;

	int action=0;//0--do nothing   1--close   2--open

	if(pChannel->channelType==cmCapVideo)
	{
		if(pChannel->channelRole==kRolePresentation)
		{
			action=GetActionForChannel(pChannel->openedInSim
							,m_rComMode.GetPresentVideoMode()
							,rCapCommon.GetPresentVideoProtocolType());
		}
		else if(pChannel->channelRole==kRolePeople)
		{
			action=GetActionForChannel(pChannel->openedInSim
							,m_rComMode.GetVideoMode()
							,rCapCommon.GetVideoProtocolType(0));
		}
	}
	else if(pChannel->channelType==cmCapAudio)
	{
		// TODO:
	}
	else if(pChannel->channelType==cmCapData)
	{
		// TODO:
	}

	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
	pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID);
	pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);
	if(action==1)
	{
		SendStartChnlCloseInd( pCSProt, pChannel );
	}
	else if(action==2)
	{
		SendIncomingChannelInd(pCSProt, pChannel, rCapCommon);
	}
	POBJDELETE(pCSProt);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::UpdateChannels(const bool au,const bool vi,const bool fecc,const bool h239,
						const BYTE recapMode,const char* pszManufacturerName, const CCapSet *pCapSet)
{
	PTRACE2(eLevelError,"CEndpointH323::UpdateChannels, Name - ",m_szEpName);

	if( m_enEpState != eEpStateConnected )
	{
		PTRACE2(eLevelError,"CEndpointH323::UpdateChannels, EndPoint is not in CONNECTED state. Name - ",m_szEpName);
		return;
	}
//	m_enDiscoInitiator = eInitiatorUnknown; // should be

	if(pCapSet != NULL)
	{
		POBJDELETE(m_pCap);
		m_pCap=new CCapSet(*pCapSet);
	}

	if( !au )
		m_pCap->EmptyAudio();
	if( !vi )
		m_pCap->EmptyVideo();
	if( !fecc )
		m_pCap->EmptyFecc();
	if( !h239 )
		m_pCap->EmptyH239();

	// create highest capabilities and create communication mode
	CCapSet  rCapCommon(*m_pCap, *m_pConfCap);

	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
	pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID);
	pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

	if( SIM_RECAP_MODE_BEFORE == recapMode )
		SendCapabilitiesInd(*m_pCap,pCSProt);

	TChannelDetails* pChannel = NULL;

	//h239 channel
	TChannelDetails *present_video=GetChannel(cmCapVideo,kRolePresentation,cmCapReceive/*dir=IN*/);
	UpdateIncomingChannByNewCap(present_video, rCapCommon);

	//people video channel
	TChannelDetails *people_video=GetChannel(cmCapVideo,kRolePeople,cmCapReceive/*dir=IN*/);
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
	/*this code is replace by UpdateIncomingChannByNewCap
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
	}*/
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
	/*this code is replace by UpdateIncomingChannByNewCap
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
	}*/

	if( SIM_RECAP_MODE_AFTER == recapMode )
		SendCapabilitiesInd(rCapCommon,pCSProt);

	// send facility_ind after empty cap set
	if( SIM_RECAP_MODE_NONE != recapMode  &&  true != au  &&  true != vi  &&  true != fecc  &&  true != h239 )
	{
		if( 0 != strcmp(pszManufacturerName,"No change") )
		{
			CVendorInfo newVendorInfo(pszManufacturerName);

			if( !(newVendorInfo == m_rVendor) )
			{
				m_rVendor = newVendorInfo;
				SendFacilityInd(pCSProt);
			}
		}
	}

	POBJDELETE(pCSProt);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendFacilityInd( CMplMcmsProtocol* pMplProtocol )
{
	PTRACE2(eLevelError,"CEndpointH323::SendFacilityInd  H323_CS_FACILITY_IND, Name - ",m_szEpName);

	// send the Facility Ind struct to CS simulation
	// ================================
	WORD  indLen = sizeof(mcIndFacility);
	mcIndFacility  ind;
	memset(&ind,0,indLen);

	ind.avfFeVndIdInd.fsId = 1;

	ind.avfFeVndIdInd.countryCode  = m_rVendor.GetCountryCode();
	ind.avfFeVndIdInd.t35Extension = m_rVendor.GetT35Extension();
	ind.avfFeVndIdInd.manfctrCode  = m_rVendor.GetManufacturerCode();
  strncpy(ind.avfFeVndIdInd.productId, m_rVendor.GetProductId(), sizeof(ind.avfFeVndIdInd.productId)-1);
  ind.avfFeVndIdInd.productId[sizeof(ind.avfFeVndIdInd.productId)-1] = '\0';

  strncpy(ind.avfFeVndIdInd.versionId, m_rVendor.GetVersionId(), sizeof(ind.avfFeVndIdInd.versionId)-1);
  ind.avfFeVndIdInd.versionId[sizeof(ind.avfFeVndIdInd.versionId)-1] = '\0';


	CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol(*pMplProtocol);

	::FillCsProtocol(pCSProt, GetCSID(),
					 H323_CS_FACILITY_IND,
					 (BYTE*)&ind, indLen);

	CSegment *pMsg = new CSegment;
	pCSProt->Serialize(*pMsg,CS_API_TYPE);

    CCSSimTaskApi api(GetCSID());
    if(api.CreateOnlyApi() >= 0)
        api.SendMsg(pMsg, SEND_TO_CSAPI);

	POBJDELETE(pCSProt);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointH323::SendLprModeChangeRequestIndication(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout)
{
	PTRACE2(eLevelError,"CEndpointH323::SendLprModeChangeRequestIndication, Name - ", m_szEpName );

	if( IsLprCall() )
	{
		CMplMcmsProtocol*  pCSProt = new CMplMcmsProtocol;
		pCSProt->AddPortDescriptionHeader(m_partyID,m_confID,m_connectionID,ePhysical_art_light);
		pCSProt->AddCSHeader(m_wCsHandle,0,m_wCsSrcUnit);

		mcIndLPRModeChange  ind;
		DWORD  indLen = sizeof(mcIndLPRModeChange);
		memset(&ind,0,indLen);

		TRACEINTO << "CCEndpointH323::SendLprModeChangeRequestIndication - Party Name = " << m_szEpName
		 << " LossProtection = " << lossProtection << " Mtbf = " << mtbf << " Congestion = " << congestionCeiling
		 << " Fill = " << fill << " Mode timeout = " << modeTimeout;

		ind.lossProtection  = lossProtection;
		ind.mtbf    = mtbf;
		ind.congestionCeiling  = congestionCeiling;
		ind.fill = fill;
		ind.modeTimeout = modeTimeout;

		::FillCsProtocol(pCSProt, GetCSID(),
						 H323_CS_SIG_LPR_MODE_CHANGE_IND,
						 (BYTE*)&ind, indLen,
						 m_nCsCallIndex, 0, 0);

		CSegment* pMsg = new CSegment;
		pCSProt->Serialize(*pMsg,CS_API_TYPE);

		CCSSimTaskApi api(GetCSID());
		if(api.CreateOnlyApi() >= 0)
			api.SendMsg(pMsg, SEND_TO_CSAPI);

		POBJDELETE(pCSProt);
	}
	else
		DBGPASSERT(1);
}

void CEndpointH323::SetSourcePartyAlias(char* aliasName)
{

	if (aliasName != NULL && strlen(aliasName) < (H243_NAME_LEN - 5) )
	{
		strncpy(m_sourceAliasNameOrTel,"NAME:",H243_NAME_LEN - 1);
		m_sourceAliasNameOrTel[H243_NAME_LEN - 1] = '\0';
		strncat(m_sourceAliasNameOrTel,aliasName,H243_NAME_LEN - 1 - strlen(m_sourceAliasNameOrTel));
	}
}
