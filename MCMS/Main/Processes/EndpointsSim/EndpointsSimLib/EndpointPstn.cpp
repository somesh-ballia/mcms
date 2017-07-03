//+========================================================================+
//                     EndpointPstn.cpp									   |
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
#include "OpcodesMcmsCommon.h"
//#include "OpcodesRanges.h"
//#include "ACU_INT.H"
#include "OpcodesMcmsNetQ931.h"
#include "ConfPartyApiDefines.h"
#include "HostCommonDefinitions.h"

#include "SystemFunctions.h"

#include "Segment.h"
#include "TaskApi.h"

#include "SimApi.h"
#include "EndpointsSim.h"
//#include "EndpointsSimConfig.h"

#include "EpSimCapSetsList.h"
#include "EpSimH323BehaviorList.h"
#include "EndpointPstn.h"
#include "OpcodesMcmsNetQ931.h"


// global static parameters
static DWORD  g_dwNetConnectionId = 33300;




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CEndpointPstn)
	//ONEVENT(1				,1				,CEndpointH323::OnStartElement)
PEND_MESSAGE_MAP(CEndpointPstn,CEndpoint);



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CEndpointPstn::CEndpointPstn( CTaskApi *pCSApi, const CCapSet& rCap, const CH323Behavior& rBehav ) 
		: CEndpoint(pCSApi,rCap,rBehav)     // constructor
{
	for (int i = 0; i < SIM_PSTN_MAX_API_COMMANDS; i++)
		m_apiArray[i] = 0;

	m_netConnectionId = 0;

	// audio board details
	CleanAudioBoardDetails();

	memset(m_szPhoneNum,0,PRI_LIMIT_PHONE_DIGITS_LEN+1);
	memset(m_szCliPhoneNum,0,PRI_LIMIT_PHONE_DIGITS_LEN+1);
}

/////////////////////////////////////////////////////////////////////////////
CEndpointPstn::CEndpointPstn(const CEndpointPstn& other) : CEndpoint(other)
{
	// illegal use
	PASSERT(1);
}


/////////////////////////////////////////////////////////////////////////////
CEndpointPstn::~CEndpointPstn()     // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
CEndpointPstn& CEndpointPstn::operator= (const CEndpointPstn& other)
{
	// illegal use
	PASSERT(1);

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::DeSerialize( CSegment& rParam )
{
	DWORD tempEpID = 0xFFFFFFFF;
	char  szPhoneTemp[128], szTemp[128];

	rParam >> tempEpID;
	rParam >> m_szEpName;
	rParam >> szPhoneTemp;
	rParam >> szTemp;
	
	m_pCap->DeSerialize(rParam);
	m_pBehavior->DeSerialize(rParam);

	SetPhoneNum(szPhoneTemp);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::Serialize(  CSegment& rSegment ) const
{
	rSegment << (DWORD)GetEpType();
	rSegment << m_nEpID;
	rSegment << m_szEpName;
	rSegment << m_szPhoneNum;
	rSegment << m_szCliPhoneNum;
	rSegment << (WORD)m_enEpState;
	rSegment << m_dialDirection;
	m_pCap->Serialize(rSegment);
	m_pBehavior->Serialize(rSegment);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SerializeDetails(  CSegment& rSegment ) const
{
//	m_pCap->Serialize(rSegment);
//	m_pBehavior->Serialize(rSegment);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SetPhoneNum( const char* pszPhoneNum )
{
	strncpy(m_szPhoneNum,pszPhoneNum,PRI_LIMIT_PHONE_DIGITS_LEN);
	m_szPhoneNum[PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SetIp( const char* pszEpIp )
{
	memset(m_szCliPhoneNum,0,PRI_LIMIT_PHONE_DIGITS_LEN+1);
	if( NULL == pszEpIp )
		return;

	int i=0, j=0;
	while( '\0' != pszEpIp[i] && j<PRI_LIMIT_PHONE_DIGITS_LEN )
	{
		if( isdigit(pszEpIp[i]) )
			m_szCliPhoneNum[j++] = pszEpIp[i];
		i++;
	}
}

/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointPstn::IsMyRtmSpanPort(const DWORD spanId,const DWORD portId )
{
	if( m_rRtmBoard.GetRtmSpanId() == spanId  &&  m_rRtmBoard.GetRtmPortId() == portId )
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SetCallIndex(const DWORD nNetConId)
{
	m_netConnectionId = nNetConId;
}

/////////////////////////////////////////////////////////////////////////////
//void CEndpointPstn::SetArrayIndex( const WORD ind )
//{
//	CEndpoint::SetArrayIndex(ind);
//}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CEndpointPstn::HandleEvent( CSegment *pMsg,DWORD msgLen,OPCODE opCode )
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
//void CEndpointPstn::HandleProtocolEvent( CMplMcmsProtocol* pMplProtocol ) 
//{
//}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::HandleIsdnProtocolEvent( const DWORD opcode, BYTE* pData, CSegment* pNetSetupParams )
{
	switch( opcode )
	{
		///////////////////////////////////////////////////
		//   CONNECT PARTY
		///////////////////////////////////////////////////

		//------------------------------------
		// Dial-In Response Opcodes
		//------------------------------------
		case NET_ALERT_REQ: {
			OnNetAlertReq(*((NET_ALERT_REQ_S*)pData));
			break;
		}
//		case NET_PROGRESS_REQ: {
//			OnNetProgressReq(segment);
//			break;
//		}
		case NET_CONNECT_REQ: {
			OnNetConnectReq(*((NET_CONNECT_REQ_S*)pData));
			break;
		}
		//------------------------------------
		// Dial-Out Opcodes
		//------------------------------------
		case NET_SETUP_REQ: {
			OnNetSetupReq(*((NET_SETUP_REQ_S*)pData));
			break;
		}

		///////////////////////////////////////////////////
		//   DISCONNECT PARTY
		///////////////////////////////////////////////////

		case NET_CLEAR_REQ: {
			OnNetClearReq(*((NET_CLEAR_REQ_S*)pData));
			break;
		}
		case NET_DISCONNECT_ACK_REQ: {
			OnNetDisconnectAckReq(*((NET_DISCONNECT_ACK_REQ_S*)pData));
			break;
		}
		case MOVE_RSRC_REQ:
		{
			PTRACE2(eLevelInfoNormal,"CEndpointPstn::HandleIsdnProtocolEvent - MOVE_RSRC_REQ (NOTHING), Name - ",m_szEpName);
			break;
		}
		default:
		{
			DBGPASSERT(opcode+1000);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::FillCommonNetHeader(NET_COMMON_PARAM_S& rNetHeader) const
{
	rNetHeader.span_id              = m_rRtmBoard.GetRtmSpanId();
	rNetHeader.net_connection_id    = m_netConnectionId;
	rNetHeader.physical_port_number = m_rRtmBoard.GetRtmPortId();
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::PrepareNetMessageSeg(CSegment& rMsgSeg,const DWORD opcode,BYTE* pDataBytes,const DWORD dwDataSize) const
{
	rMsgSeg << opcode;

	// put RTM board/port fields to segment
	rMsgSeg	<< m_rRtmBoard.GetBoardId()
			<< m_rRtmBoard.GetSubBoardId()
			<< m_rRtmBoard.GetRtmSpanId()
			<< m_rRtmBoard.GetRtmPortId();

	// put party details
	rMsgSeg	<< m_confID
			<< m_partyID
			<< m_connectionID;
//			<< m_netConnectionId;

	rMsgSeg.Put(pDataBytes,dwDataSize);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::OnGuiUpdateEp( CSegment* pParam )
{
//	COsQueue txMbx;
//	txMbx.DeSerialize(*pParam);	
//      or?
//	m_guiTxMbx.DeSerialize(*pParam);

//	DeSerialize(*pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::OnGuiConnectEp()
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::OnGuiConnectEp - connect party, Name - ",m_szEpName);

	if( m_enEpState == eEpStateIdle  ||  m_enEpState == eEpStateDisconnected )
	{
		SetState(eEpStateConnecting);
		SendSetupInd();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::OnGuiDisconnectEp()
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::OnGuiDisconnectEp - disconnect party, Name - ",m_szEpName);

	if( m_enEpState == eEpStateConnected  ||  m_enEpState == eEpStateConnecting )
	{
		SetState(eEpStateDisconnecting);

		Disconnect(NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::OnGuiDeleteEp()
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::OnGuiDeleteEp - disconnect and delete party, Name - ",m_szEpName);

	m_isToBeDeleted = TRUE;

	if( m_enEpState == eEpStateConnected  ||  m_enEpState == eEpStateConnecting )
	{
		SetState(eEpStateDisconnecting);

		Disconnect(NULL);
	}
	else if( m_enEpState == eEpStateIdle || m_enEpState == eEpStateDisconnected || m_enEpState == eEpStateUnknown )
		m_isReadyToDelete = TRUE;
	else  // m_enEpState == eEpStateDisconnecting
	  {
		; // nothing to do
	  }
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::Disconnect( CMplMcmsProtocol* pMplProtocol )
{
	SendDisconnectInd();
}

/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointPstn::IsConnectionCompleted() const
{
	if( DIAL_IN == m_dialDirection && m_apiArray[SIM_PSTN_CONNECT_REQ_RECEIVED]<1 )
		return FALSE;
	if( DIAL_OUT == m_dialDirection && m_apiArray[SIM_PSTN_CONNECT_IND_SENT]<1 )
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::CleanAfterDisconnect()
{
		// clean commands array
	for (int i = 0; i < SIM_PSTN_MAX_API_COMMANDS; i++)
		m_apiArray[i] = 0;

	// clean conf caps and comm mode
//	m_pConfCap->Empty();
//	m_rComMode.Create(*m_pConfCap);

		// clean conf/party details
	m_confID = m_partyID = m_connectionID = 0xFFFFFFFF;

		// clean audio board details
	m_rAudioBoard.CleanDetails();

		// clean Muted flag
	m_isMuted = FALSE;

		// clean Net board details
	m_rRtmBoard.CleanDetails();
}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointPstn::OnNetAlertReq(NET_ALERT_REQ_S& tAlertReq)
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::OnNetAlertReq - NET_ALERT_REQ, ep - ",m_szEpName);

	m_apiArray[SIM_PSTN_ALERT_REQ_RECEIVED]++;
}

/////////////////////////////////////////////////////////////////////////////
//void  CEndpointPstn::OnNetProgressReq(CSegment& segment)
//{
//	PTRACE2(eLevelInfoNormal,"CEndpointPstn::OnNetProgressReq - NET_PROGRESS_REQ, ep - ",m_szEpName);
//}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointPstn::OnNetConnectReq(NET_CONNECT_REQ_S& tConnectReq)
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::OnNetConnectReq - NET_CONNECT_REQ, ep - ",m_szEpName);
	m_apiArray[SIM_PSTN_CONNECT_REQ_RECEIVED]++;

	if( m_apiArray[SIM_PSTN_SETUP_IND_SENT] > 0 )
	{
		SetState(eEpStateConnected);
	}
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointPstn::OnNetSetupReq(NET_SETUP_REQ_S& tSetupReq)
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::OnNetSetupReq - NET_SETUP_REQ, ep - ",m_szEpName);
	m_apiArray[SIM_PSTN_SETUP_REQ_RECEIVED]++;

	char szTemp[128];
	memset(szTemp,0,128);
	strncpy(szTemp,(char*)tSetupReq.called_party.digits,sizeof(szTemp) - 1);
	szTemp[sizeof(szTemp) - 1] = '\0';
	SetIp(szTemp);

	memset(szTemp,0,128);
	const int maxStrLenForKW = std::min(tSetupReq.calling_party.num_digits, sizeof(m_szPhoneNum) - 1);
	strncpy(m_szPhoneNum,(char*)tSetupReq.calling_party.digits,maxStrLenForKW);
	m_szPhoneNum[maxStrLenForKW] = '\0';
//	SetIp(szTemp);

	SetState(eEpStateConnecting);

//	SendProgressInd();
	SendAlertInd();
	SendConnectInd();

	SetState(eEpStateConnected);
}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointPstn::OnNetClearReq(NET_CLEAR_REQ_S& tClearReq)
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::OnNetClearReq - NET_CLEAR_REQ, ep - ",m_szEpName);
	m_apiArray[SIM_PSTN_CLEAR_REQ_RECEIVED]++;

	SetState(eEpStateDisconnecting);

	SendClearInd();
	CleanAfterDisconnect();

	SetState(eEpStateDisconnected);

	if( TRUE == m_isToBeDeleted )
		m_isReadyToDelete = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointPstn::OnNetDisconnectAckReq(NET_DISCONNECT_ACK_REQ_S& tDiscoAckReq)
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::OnNetDisconnectAckReq - NET_DISCONNECT_REQ, ep - ",m_szEpName);
	m_apiArray[SIM_PSTN_DISCONNECT_ACK_REQ_RECEIVED]++;

	if( m_apiArray[SIM_PSTN_DISCONNECT_IND_SENT] > 0 )
	{
		SendDisconnectAckInd();
		CleanAfterDisconnect();
		SetState(eEpStateDisconnected);

		if( TRUE == m_isToBeDeleted )
			m_isReadyToDelete = TRUE;
	}
	else
		PASSERT(1);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendSetupInd()
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::SendSetupInd - NET_SETUP_IND, ep - ", m_szEpName );
	m_apiArray[SIM_PSTN_SETUP_IND_SENT]++;

	// prepare and send SETUP_IND
	NET_SETUP_IND_S  rSetupStruct;
	memset(&rSetupStruct,0,sizeof(NET_SETUP_IND_S));

	m_confID = 0;
	m_partyID = 0;
	m_connectionID = LOBBY_CONNECTION_ID;
	// fill setup struct
		// net header
	FillCommonNetHeader(rSetupStruct.net_common_header);
	rSetupStruct.net_spfc  = 1;
	rSetupStruct.call_type = ACU_VOICE_SERVICE;
//	rSetupStruct.call_type = ACU_DATA_H0_SERVICE;//ACU_DATA_SERVICE;//rons - JUST FOR CHECKING ISDN PATH
	// Calling party
	rSetupStruct.calling_party.num_type = 6;//ACU_NB_TYPE_UNKNOWN;
	rSetupStruct.calling_party.num_plan = 7;//ACU_NB_PLAN_UNKNOWN;
	rSetupStruct.calling_party.presentation_ind = 9;//ACU_NB_PRES_ALLOWED;
	rSetupStruct.calling_party.screening_ind    = 9;//ACU_NB_SCREEN_USER_PROVIDED;
	rSetupStruct.calling_party.num_digits = strlen(m_szCliPhoneNum);
	strncpy(((char*)rSetupStruct.calling_party.digits),m_szCliPhoneNum,strlen(m_szCliPhoneNum));
	
	// Called party
	rSetupStruct.called_party.num_type = 6;//ACU_NB_TYPE_UNKNOWN;
	rSetupStruct.called_party.num_plan = 7;//ACU_NB_PLAN_UNKNOWN;
	//rSetupStruct.called_party.num_digits = strlen(m_szCliPhoneNum);
	//strncpy(((char*)rSetupStruct.called_party.digits),m_szCliPhoneNum,strlen(m_szCliPhoneNum));
	rSetupStruct.called_party.num_digits = strlen(m_szPhoneNum);
	strncpy(((char*)rSetupStruct.called_party.digits),m_szPhoneNum,strlen(m_szPhoneNum));
	
	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg,NET_SETUP_IND,(BYTE*)(&rSetupStruct),sizeof(NET_SETUP_IND_S));

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendProgressInd()
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::SendProgressInd - NET_PROGRESS_IND, ep - ", m_szEpName );
	m_apiArray[SIM_PSTN_PROGRESS_IND_SENT]++;

//	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendAlertInd()
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::SendAlertInd - NET_ALERT_IND, ep - ", m_szEpName );
	m_apiArray[SIM_PSTN_ALERT_IND_SENT]++;

	// prepare and send ALERT_IND
	NET_ALERT_IND_S  rAlertStruct;
	memset(&rAlertStruct,0,sizeof(NET_ALERT_IND_S));

	// fill alert struct
		// net header
	FillCommonNetHeader(rAlertStruct.net_common_header);

	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg,NET_ALERT_IND,(BYTE*)(&rAlertStruct),sizeof(NET_ALERT_IND_S));

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendConnectInd()
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::SendConnectInd - NET_CONNECT_IND, ep - ", m_szEpName );
	m_apiArray[SIM_PSTN_CONNECT_IND_SENT]++;

	// prepare and send CONNECT_IND
	NET_CONNECT_IND_S  rConnectStruct;
	memset(&rConnectStruct,0,sizeof(NET_CONNECT_IND_S));

	// fill connect struct
		// net header
	FillCommonNetHeader(rConnectStruct.net_common_header);

	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg,NET_CONNECT_IND,(BYTE*)(&rConnectStruct),sizeof(NET_CONNECT_IND_S));

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendDisconnectInd()
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::SendDisconnectInd - NET_DISCONNECT_IND, ep - ", m_szEpName );
	m_apiArray[SIM_PSTN_DISCONNECT_IND_SENT]++;

	// prepare and send DISCONNECT_IND
	NET_DISCONNECT_IND_S  rDisConnectStruct;
	memset(&rDisConnectStruct,0,sizeof(NET_DISCONNECT_IND_S));

	// fill disconnect struct
		// net header
	FillCommonNetHeader(rDisConnectStruct.net_common_header);
	rDisConnectStruct.cause.cause_val = 0; // causDEFAULT_VAL;

	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg,NET_DISCONNECT_IND,(BYTE*)(&rDisConnectStruct),sizeof(NET_DISCONNECT_IND_S));

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendDisconnectAckInd()
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::SendDisconnectAckInd - NET_DISCONNECT_ACK_IND, ep - ", m_szEpName );
	m_apiArray[SIM_PSTN_DISCONNECT_ACK_IND_SENT]++;

	// prepare and send DISCONNECT_IND
	NET_DISCONNECT_ACK_IND_S  rDisConnectAckStruct;
	memset(&rDisConnectAckStruct,0,sizeof(NET_DISCONNECT_ACK_IND_S));

	// fill disconnect struct
		// net header
	FillCommonNetHeader(rDisConnectAckStruct.net_common_header);
	rDisConnectAckStruct.cause.cause_val = 0; // causDEFAULT_VAL;

	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg,NET_DISCONNECT_ACK_IND,(BYTE*)(&rDisConnectAckStruct),sizeof(NET_DISCONNECT_ACK_IND_S));

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendClearInd()
{
	PTRACE2(eLevelInfoNormal,"CEndpointPstn::SendClearInd - NET_CLEAR_IND, ep - ", m_szEpName );
	m_apiArray[SIM_PSTN_CLEAR_IND_SENT]++;

	// prepare and send DISCONNECT_IND
	NET_CLEAR_IND_S  rClearStruct;
	memset(&rClearStruct,0,sizeof(NET_CLEAR_IND_S));

	// fill clear struct
		// net header
	FillCommonNetHeader(rClearStruct.net_common_header);
	rClearStruct.cause.cause_val = 0; // causDEFAULT_VAL;

	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg,NET_CLEAR_IND,(BYTE*)(&rClearStruct),sizeof(NET_CLEAR_IND_S));

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}



/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendMuteIndication() const
{
	PTRACE2(eLevelError,"CEndpointPstn::SendMuteIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendUnmuteIndication() const
{
	PTRACE2(eLevelError,"CEndpointPstn::SendUnmuteIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendFeccTokenRequestIndication() const
{
	PTRACE2(eLevelError,"CEndpointPstn::SendFeccTokenRequestIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendFeccTokenReleaseIndication() const
{
	PTRACE2(eLevelError,"CEndpointPstn::SendFeccTokenReleaseIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendFeccKeyRequestIndication(const char* pszDtmf) const
{
        PTRACE2(eLevelError,"CEndpointPstn::SendFeccKeyRequestIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendH239TokenRequestIndication()
{
	PTRACE2(eLevelError,"CEndpointPstn::SendH239TokenRequestIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendH239TokenReleaseIndication()
{
	PTRACE2(eLevelError,"CEndpointPstn::SendH239TokenReleaseIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointPstn::SendLprModeChangeRequestIndication(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout)
{
	PTRACE2(eLevelError, "CEndpointPstn::SendLprModeChangeRequestIndication (NOTHING), Name - ", m_szEpName );	
}













