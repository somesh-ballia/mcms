//+========================================================================+
//                     EndpointISDN.cpp									   |
//            	  Copyright 2007 Polycom Ltd.                              |
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
#include "ObjString.h"
#include "SimApi.h"
#include "EndpointsSim.h"
//#include "EndpointsSimConfig.h"

#include "EpSimCapSetsList.h"
#include "EpSimH323BehaviorList.h"
#include "EndpointISDN.h"

#include "OpcodesMcmsBonding.h"
#include "OpcodesMcmsMux.h"
#include "H221.h"
#include "muxint.h"
#include "H239Defines.h"


#include "TraceStream.h"
#include "EncryptionDefines.h"


// global static parameters
static DWORD  g_dwNetConnectionId = 33300;
static DWORD  g_nextPhoneNumber = 1000100;

#define POLYCOM_ENCRYPT 0
#define TANDBERG_ENCRYPT 1
// disconnection cause test
#define WRONG_P3 0


/////////////////////////////////////////////////////////////////////////////
//   CBondingEp - Bonding parameters for ISDN party
/////////////////////////////////////////////////////////////////////////////
CMuxEpParams::CMuxEpParams()
{
	m_muxConnectionId=m_muxBoardId=m_muxUnitId=m_muxPortId=0;
}

CMuxEpParams::~CMuxEpParams()
{
}

void CMuxEpParams::SaveMuxEpParams( DWORD connId, WORD boardId, WORD unitId, WORD portId )
{
	m_muxConnectionId = connId;
	m_muxBoardId = boardId;
	m_muxUnitId = unitId;
	m_muxPortId = portId;
}




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CEndpointISDN)
	//ONEVENT(1				,1				,CEndpointH323::OnStartElement)
	  ONEVENT( ROLE_TOKEN_OWNER_TOUT, ANYCASE, CEndpointISDN::OnRoleTokenOwnerTout )
PEND_MESSAGE_MAP(CEndpointISDN, CEndpoint);



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CEndpointISDN::CEndpointISDN( CTaskApi *pCSApi, const CCapSet& rCap, const CH323Behavior& rBehav ) 
		: CEndpoint(pCSApi,rCap,rBehav)     // constructor
{
	for (int i = 0; i < SIM_ISDN_MAX_API_COMMANDS; i++)
		m_apiArray[i] = 0;

	m_netConnectionId = 0;

	// audio board details
	CleanAudioBoardDetails();
	
	for (int i=0; i< MAX_ADDITIONAL_PHONE_NUM; i++)
	{
		m_channelArr[i] = new CIsdnChannel;		
	}
	
	memset(m_szCliPhoneNum, 0, PRI_LIMIT_PHONE_DIGITS_LEN+1);
	
	m_numOfBondingChannels = -1;
	m_connectedChannelsCounter = 0;
	
	
}

/////////////////////////////////////////////////////////////////////////////
CEndpointISDN::CEndpointISDN(const CEndpointISDN& other) : CEndpoint(other)
{
	// illegal use
	PASSERT(1);
}


/////////////////////////////////////////////////////////////////////////////
CEndpointISDN::~CEndpointISDN()     // destructor
{
	for (int i=0; i< MAX_ADDITIONAL_PHONE_NUM; i++)
	{
		delete m_channelArr[i];		
	}
}

/////////////////////////////////////////////////////////////////////////////
CEndpointISDN& CEndpointISDN::operator= (const CEndpointISDN& other)
{
	// illegal use
	PASSERT(1);

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::DeSerialize( CSegment& rParam )
{
	DWORD tempEpID = 0xFFFFFFFF;
	char  szPhoneTemp[MAX_ADDITIONAL_PHONE_NUM*(BND_MAX_PHONE_LEN+1)], szTemp[128];

	rParam >> tempEpID;
	rParam >> m_szEpName;
	
	rParam >> szPhoneTemp; //szPhoneTemp;	
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::DeSerialize - szPhoneTemp - ", szPhoneTemp);
	
	/*for( int i=0; i<MAX_ADDITIONAL_PHONE_NUM; i++ )
	{
		char* pch;
		pch = (char*) memchr (szPhoneTemp + (i*BND_MAX_PHONE_LEN), '\0', BND_MAX_PHONE_LEN+1);		
		
		strcpy(m_szPhoneNum[i], pch);
	}*/
	

	rParam >> m_szCliPhoneNum;
	//rParam >> szTemp;
	
	m_pCap->DeSerialize(rParam);
	m_pBehavior->DeSerialize(rParam);
	
	DWORD tmpNumOfChannels = 0xFFFFFFFF;
	rParam >> tmpNumOfChannels;
	m_numOfBondingChannels = (APIS8)tmpNumOfChannels;	
	
	SetPhoneNum(szPhoneTemp);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::Serialize(  CSegment& rSegment ) const
{
	rSegment << (DWORD)GetEpType();
	rSegment << m_nEpID;
	rSegment << m_szEpName;
	
	for( int i=0; i<MAX_ADDITIONAL_PHONE_NUM; i++ )
	{
		m_channelArr[i]->Serialize(rSegment);
	}
	
	rSegment << m_szCliPhoneNum;
	
	rSegment << (WORD)m_enEpState;
	rSegment << m_dialDirection;
	m_pCap->Serialize(rSegment);
	m_pBehavior->Serialize(rSegment);
	
	rSegment << (DWORD)m_numOfBondingChannels;
	
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SerializeDetails(  CSegment& rSegment ) const
{
//	m_pCap->Serialize(rSegment);
//	m_pBehavior->Serialize(rSegment);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SetPhoneNum( const char* pszPhoneNum )
{
///	strncpy(m_szPhoneNum[0], pszPhoneNum, PRI_LIMIT_PHONE_DIGITS_LEN);
///	m_szPhoneNum[0][PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';

	m_channelArr[0]->SetPhoneNum(pszPhoneNum);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SetPhoneNum( const char* pszPhoneNum , int channelIndex)
{
///	strncpy(m_szPhoneNum[channelIndex], pszPhoneNum, PRI_LIMIT_PHONE_DIGITS_LEN);
///	m_szPhoneNum[channelIndex][PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';	
	
	m_channelArr[channelIndex]->SetPhoneNum(pszPhoneNum);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SetIp( const char* pszEpIp )
{
	memset(m_szCliPhoneNum, 0, PRI_LIMIT_PHONE_DIGITS_LEN+1);
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
BOOL CEndpointISDN::IsMyRtmSpanPort(const DWORD spanId, const DWORD portId )
{
	if( m_rRtmBoard.GetRtmSpanId() == spanId  &&  m_rRtmBoard.GetRtmPortId() == portId )
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointISDN::SetNetBoardDetails(const WORD boardId, const WORD subBoardId, const WORD spanId, const WORD portId, const DWORD netConnectionId)
{
	int index = -1;
	if (netConnectionId == (DWORD)-1)
		index = 0;
	else
		index = FindChannelIndex(netConnectionId);
	
	if (index >= 0 && index <= MAX_ADDITIONAL_PHONE_NUM)
	{
		m_channelArr[index]->SetBoardDetails(boardId, subBoardId, spanId, portId);
		return TRUE;
	}
	
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SetCallIndex(const DWORD nNetConId)
{
	m_netConnectionId = nNetConId;
}

/////////////////////////////////////////////////////////////////////////////
//void CEndpointISDN::SetArrayIndex( const WORD ind )
//{
//	CEndpoint::SetArrayIndex(ind);
//}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::HandleEvent( CSegment *pMsg, DWORD msgLen, OPCODE opCode )
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
//void CEndpointISDN::HandleProtocolEvent( CMplMcmsProtocol* pMplProtocol ) 
//{
//}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::HandleIsdnProtocolEvent( const DWORD opcode, BYTE* pData, CSegment* pNetSetupParams )
{
	switch( opcode )
	{
		///////////////////////////////////////////////////
		//   CONNECT PARTY
		///////////////////////////////////////////////////

		//------------------------------------
		// Dial-In Response Opcodes
		//------------------------------------
		case NET_ALERT_REQ:
		{
			OnNetAlertReq(*((NET_ALERT_REQ_S*)pData));
			break;
		}
//		case NET_PROGRESS_REQ: {
//			OnNetProgressReq(segment);
//			break;
//		}
		case NET_CONNECT_REQ:
		{
			OnNetConnectReq(*((NET_CONNECT_REQ_S*)pData));
			break;
		}
		//------------------------------------
		// Dial-Out Opcodes
		//------------------------------------
		case NET_SETUP_REQ:
		{
			OnNetSetupReq(*((NET_SETUP_REQ_S*)pData), pNetSetupParams);
			break;
		}
		
		case BND_CONNECTION_INIT:
		{
			OnBondingInitReq(*((BND_CONNECTION_INIT_REQUEST_S*)pData));
			break;
		}

		///////////////////////////////////////////////////
		//   DISCONNECT PARTY
		///////////////////////////////////////////////////

		case NET_CLEAR_REQ:
		{
			OnNetClearReq(*((NET_CLEAR_REQ_S*)pData), pNetSetupParams);
			break;
		}
		
		case NET_DISCONNECT_ACK_REQ:
		{
			OnNetDisconnectAckReq(*((NET_DISCONNECT_ACK_REQ_S*)pData));
			break;
		}
		
		case MOVE_RSRC_REQ:
		{
			PTRACE2(eLevelInfoNormal,"CEndpointISDN::HandleIsdnProtocolEvent - MOVE_RSRC_REQ (NOTHING), Name - ",m_szEpName);
			break;
		}
		case SEND_H_230:
		{
						
			BAS_CMD_DESC* p = (BAS_CMD_DESC*)pData;
			DWORD rmt_h230_length = p->number_of_bytes;
			
			BYTE* ptr = pData + sizeof (APIU32);
			
			CSegment  h230Seg;
			h230Seg.Put(ptr,rmt_h230_length);
			//h230Seg.DumpHex();
			OnMuxH230(&h230Seg);
			
			break;
		}
		case SET_ECS:
		{
//			for (WORD i=0; i<sizeof(SET_ECS_S)+20-sizeof(DWORD); i++)
//				TRACEINTO << "****** ECS *******"<< (DWORD)pData[i];

			OnSetEcsReq(*((SET_ECS_S*)pData));
			
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
void CEndpointISDN::FillCommonNetHeader(NET_COMMON_PARAM_S& rNetHeader) const
{
	rNetHeader.span_id              = m_rRtmBoard.GetRtmSpanId();
	rNetHeader.net_connection_id    = m_netConnectionId;
	rNetHeader.physical_port_number = m_rRtmBoard.GetRtmPortId();
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::PrepareNetMessageSeg(CSegment& rMsgSeg, const DWORD opcode, BYTE* pDataBytes, const DWORD dwDataSize, const DWORD chanConnectionId /*CSegment* pNetSetupParams*/) ///const
{
//	DWORD chanConnectionId, boardId, subBoardId, chanSpanId, chanPortId;
//	
//	if ((NET_ALERT_IND == opcode) || (NET_CONNECT_IND == opcode) || (NET_CLEAR_IND == opcode))
//	{
//		if (pNetSetupParams)
//		{
//			*pNetSetupParams >> chanConnectionId
//							 >> boardId
//							 >> subBoardId
//					 		 >> chanSpanId
//							 >> chanPortId;
//		}
//	}
	
	//start segment writing
	///////////////////////
	rMsgSeg << opcode;

	// put RTM board/port fields to segment
	if ((NET_ALERT_IND == opcode) || (NET_CONNECT_IND == opcode) || (NET_CLEAR_IND == opcode) 
		|| (NET_DISCONNECT_IND == opcode) || (NET_DISCONNECT_ACK_IND == opcode))
	{
		for (int i = 0; i < MAX_ADDITIONAL_PHONE_NUM; i++)
		{
			if (m_channelArr[i]->GetConnectionId() == chanConnectionId)
			{
				CRtmBoardDetails rtmBoardDetails = m_channelArr[i]->GetRtmBoardDetails(); 
				rMsgSeg	<< rtmBoardDetails.GetBoardId()
						<< rtmBoardDetails.GetSubBoardId()
						<< rtmBoardDetails.GetRtmSpanId()
						<< rtmBoardDetails.GetRtmPortId();
				break;
			}
		}
				
	}
	else
	{
		rMsgSeg	<< m_rRtmBoard.GetBoardId()
				<< m_rRtmBoard.GetSubBoardId()
				<< m_rRtmBoard.GetRtmSpanId()
				<< m_rRtmBoard.GetRtmPortId();
	}
	
	
//	if ((NET_ALERT_IND == opcode) || (NET_CONNECT_IND == opcode) || (NET_CLEAR_IND == opcode))
//	{
//		rMsgSeg	<< (WORD)chanSpanId
//				<< (WORD)chanPortId;
//	}
//	else
//	{
//		rMsgSeg	<< m_rRtmBoard.GetRtmSpanId()
//				<< m_rRtmBoard.GetRtmPortId();
//	}

	// put party details
	rMsgSeg	<< m_confID
			<< m_partyID;

	if ((NET_ALERT_IND == opcode) || (NET_CONNECT_IND == opcode) || (NET_CLEAR_IND == opcode) 
		|| (NET_DISCONNECT_IND == opcode) || (NET_DISCONNECT_ACK_IND == opcode))
	{
		rMsgSeg	<< (DWORD)chanConnectionId;
	}
	else
	{
		rMsgSeg	<< m_connectionID;
	}

	rMsgSeg.Put(pDataBytes, dwDataSize);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::OnGuiUpdateEp( CSegment* pParam )
{
//	COsQueue txMbx;
//	txMbx.DeSerialize(*pParam);	
//      or?
//	m_guiTxMbx.DeSerialize(*pParam);

//	DeSerialize(*pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::OnGuiConnectEp()
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnGuiConnectEp - connect party, Name - ", m_szEpName);
	 
	if( m_enEpState == eEpStateIdle  ||  m_enEpState == eEpStateDisconnected )
	{
		SetState(eEpStateConnecting);
		
		// set net connection ids for all ISDN channels
		SetNetConnectionIDs();
		
		SendSetupInd();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::OnGuiDisconnectEp()
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnGuiDisconnectEp - disconnect party, Name - ", m_szEpName);

	if( m_enEpState == eEpStateConnected  ||  m_enEpState == eEpStateConnecting )
	{
		SetState(eEpStateDisconnecting);
		Disconnect(NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::OnGuiDeleteEp()
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnGuiDeleteEp - disconnect and delete party, Name - ", m_szEpName);

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
void CEndpointISDN::Disconnect( CMplMcmsProtocol* pMplProtocol )
{
	SendDisconnectInd();
}

/////////////////////////////////////////////////////////////////////////////
BOOL CEndpointISDN::IsConnectionCompleted() const
{
	/* yair
	 * 
	if( DIAL_IN == m_dialDirection && m_apiArray[SIM_ISDN_CONNECT_REQ_RECEIVED]<1 )
		return FALSE;
	if( DIAL_OUT == m_dialDirection && m_apiArray[SIM_ISDN_CONNECT_IND_SENT]<1 )
		return FALSE;
		
	*/

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::CleanAfterDisconnect()
{
		// clean commands array
	for (int i = 0; i < SIM_ISDN_MAX_API_COMMANDS; i++)
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
void  CEndpointISDN::OnNetAlertReq(NET_ALERT_REQ_S& tAlertReq)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnNetAlertReq - NET_ALERT_REQ, ep - ", m_szEpName);

	m_apiArray[SIM_ISDN_ALERT_REQ_RECEIVED]++;
}

/////////////////////////////////////////////////////////////////////////////
//void  CEndpointISDN::OnNetProgressReq(CSegment& segment)
//{
//	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnNetProgressReq - NET_PROGRESS_REQ, ep - ", m_szEpName);
//}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnNetConnectReq(NET_CONNECT_REQ_S& tConnectReq)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnNetConnectReq - NET_CONNECT_REQ, ep - ", m_szEpName);
	
	m_apiArray[SIM_ISDN_CONNECT_REQ_RECEIVED]++;

	if( m_apiArray[SIM_ISDN_SETUP_IND_SENT] > 0 )
	{
		SetState(eEpStateConnected);
	}
	else
	{
		PASSERT(1);
	}
	
	m_connectedChannelsCounter++;
	
}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnNetSetupReq(NET_SETUP_REQ_S& tSetupReq, CSegment* pNetSetupParams)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnNetSetupReq - NET_SETUP_REQ, ep - ", m_szEpName);
	
	m_apiArray[SIM_ISDN_SETUP_REQ_RECEIVED]++;

	char szTemp[128];
	memset(szTemp, 0, 128);
	
	// Get the calling party phone number and set it in the CLI number member
	strncpy(szTemp, (char*)tSetupReq.calling_party.digits, sizeof(szTemp) - 1);
	szTemp[sizeof(szTemp) - 1] = '\0';
	SetIp(szTemp);
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnNetSetupReq - NET_SETUP_REQ, calling_party.digits - ", szTemp);

	// Get the called party phone number (the phone number that was dial for this channel connection)
	memset(szTemp, 0, 128);
	strncpy(szTemp, (char*)tSetupReq.called_party.digits, sizeof(szTemp) - 1);
	szTemp[sizeof(szTemp) - 1] = '\0';
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnNetSetupReq - NET_SETUP_REQ, called_party.digits - ", szTemp);
	
	int phoneNumOffset = tSetupReq.called_party.num_digits - BND_MAX_PHONE_LEN;
	
	char szTemp1[BND_MAX_PHONE_LEN + 1];
	memset(szTemp1, 0, BND_MAX_PHONE_LEN + 1);
	if(phoneNumOffset >= 0)
	{
		strncpy(szTemp1, szTemp+phoneNumOffset, BND_MAX_PHONE_LEN);
		szTemp1[BND_MAX_PHONE_LEN] = '\0';
	}
	else
		PASSERTMSG(1, "possible access out of bound for szTemp"); 
	
	szTemp1[BND_MAX_PHONE_LEN] = '\0';
	
	// Get channel parameters 
	DWORD chanConnectionId, boardId, subBoardId, chanSpanId, chanPortId, isFirstChannel;
	if (NULL == pNetSetupParams)
	{
		PTRACE(eLevelError, "CEndpointISDN::OnNetSetupReq - No channel params! ");
		PASSERT(1);
		return;
	}
	CSegment *pAlertIndParams = new CSegment(*pNetSetupParams);
	CSegment *pConnectIndlParams = new CSegment(*pNetSetupParams);

	*pNetSetupParams >> chanConnectionId
					 >> boardId
					 >> subBoardId
					 >> chanSpanId
					 >> chanPortId
					 >> isFirstChannel;


	if (isFirstChannel)
	{	// In case of NET_SETUP of first channel - update phone# + other params
		m_channelArr[0]->SetPhoneNum(szTemp1);
		m_channelArr[0]->SetParams(chanConnectionId, boardId, subBoardId, chanSpanId, chanPortId );
		m_channelArr[0]->SetState(eEpStateConnected);
		m_connectedChannelsCounter++;
	}
	else
	{	// In case of NET_SETUP of all other channels - find the relevant channel 
		// according to the phone# and update connection ID + state
		for (int i = 1; i < MAX_ADDITIONAL_PHONE_NUM; i++)
		{
			char* buff = m_channelArr[i]->GetPhoneNum();
			if ((0 == strncmp(buff, szTemp1, BND_MAX_PHONE_LEN)) &&
					(m_channelArr[i]->GetState() != eEpStateConnected ))
			{
				m_channelArr[i]->SetParams(chanConnectionId, boardId, subBoardId, chanSpanId, chanPortId );
				m_channelArr[i]->SetState(eEpStateConnected);
				m_connectedChannelsCounter++;
				break;
			}
		}
	}

	SetState(eEpStateConnecting);
	
	SendAlertInd(tSetupReq, pAlertIndParams);
	SendConnectInd(pConnectIndlParams);

	if (m_connectedChannelsCounter == m_numOfBondingChannels)
		SetState(eEpStateConnected);
	
	 
	POBJDELETE(pAlertIndParams);
	POBJDELETE(pConnectIndlParams);
}


/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnBondingInitReq(BND_CONNECTION_INIT_REQUEST_S& tBondingInitReq)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnBondingInitReq - BND_CONNECTION_INIT_REQUEST_S, ep - ", m_szEpName);
	
	m_apiArray[SIM_ISDN_BONDING_INIT_REQ_RECEIVED]++;

	if (m_numOfBondingChannels > 0)
	{
		if (m_numOfBondingChannels > tBondingInitReq.callParams.NumOfBndChnls)
			m_numOfBondingChannels = tBondingInitReq.callParams.NumOfBndChnls;

		tBondingInitReq.callParams.NumOfBndChnls = m_numOfBondingChannels;
	}
	else
	{
		m_numOfBondingChannels = tBondingInitReq.callParams.NumOfBndChnls;
	}
	
	if (m_numOfBondingChannels > MAX_ADDITIONAL_PHONE_NUM)
	{
		PTRACE(eLevelError, "CEndpointISDN::OnBondingInitReq - Illegal channels number ");
		PASSERT(m_numOfBondingChannels);
		m_numOfBondingChannels = MAX_ADDITIONAL_PHONE_NUM;
	}
	if (m_numOfBondingChannels < 1)
	{
		PTRACE(eLevelError, "CEndpointISDN::OnBondingInitReq - Illegal channels number (0) ");
		PASSERT(1);
		m_numOfBondingChannels = 1;
	}
	if (DIRECTION_DIAL_OUT == tBondingInitReq.callParams.direction)
		PreparePhoneNumbers();
	else  // if direction = DIRECTION_DIAL_IN
		SetPhoneNumbers((BYTE*)tBondingInitReq.additional_dial_in_phone_num.digits);
		
	// Send BND_END_NEGOTIATION_IND
	SendBondingEndNegotiationInd(tBondingInitReq);	
	
	// For Dial-In: Send NET_SETUP_IND for all additional channels
	if (DIRECTION_DIAL_IN == tBondingInitReq.callParams.direction)
	{
		SystemSleep(100);
		for (int ChanInd = 1; ChanInd < m_numOfBondingChannels; ChanInd++)
		{
			SendSetupInd(ChanInd);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnSetEcsReq(SET_ECS_S& tSetEcs)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - SET_ECS_S, ep - ", m_szEpName);
	
	APIU32  opcode = tSetEcs.p_opcode;
  	APIU32  len = tSetEcs.len;
  	APIU8*  pMsgInfo = (BYTE*)(&tSetEcs.asn1_message);
  	  	
  	BYTE* pIndArray = new BYTE[sizeof(REMOTE_ECS_SE) - sizeof (APIU8*) + len];
	REMOTE_ECS_SE* pRemoteEcsInd = (REMOTE_ECS_SE*)pIndArray;
  	

 
// 	SET_ECS_HEADER_S* header_s = (SET_ECS_HEADER_S*)& tSetEcs;
// 	TRACESTR(eLevelInfoNormal) << "CEndpointISDN::OnSetEcsReq: opcode=" << (DWORD)header_s-> p_opcode << " , len=" << (DWORD)header_s->len << " , header_byte="  << (DWORD)header_s->header_byte; 
   


 	switch (opcode)
  	{
  		case P0_Identifier:
  		{
		  PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - sending P0, ep - ", m_szEpName);
  			pRemoteEcsInd->len = len;
  			memcpy(&pRemoteEcsInd->asn1_message, pMsgInfo, len);
  			break;
  		}
  		case P3_Identifier:
  		{

		  //  current block
		  char* ecs_data = (char*)&(tSetEcs.asn1_message);
		  BYTE header_byte = ecs_data[0];
		  switch(header_byte){
		  case SE_HEADER_SINGLE_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P3 single block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_FIRST_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P3 first block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_INTERMEDIATE_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P3 intermidiate block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_LAST_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P3 last block , ep - ", m_szEpName);			
		    break;
		  }
		  default:{
		    PTRACE2INT(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P3 unknown block header = ", (DWORD)header_byte);
			PDELETEA(pIndArray);
			
		    return;
		  }
		}

			
		  PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - sending P3, ep - ", m_szEpName);

		  delete[] pIndArray;
		  DWORD p3_data_len = 272;
		  len = p3_data_len;
		  pIndArray = new BYTE[sizeof(REMOTE_ECS_SE) - sizeof (APIU8*) + p3_data_len];
		  pRemoteEcsInd = (REMOTE_ECS_SE*)pIndArray;


		  pRemoteEcsInd->len = p3_data_len;

		  if(TANDBERG_ENCRYPT){
		    BYTE pConstTandbergP3[272] = {0xa3,0x82,0x01,0x0c,0x80,0x02,0x00,0x05,0x81,0x81,0x81,0x00,0xa4,0x78,0x8e,0x21,0x84,0xb8,0xd6,0x8b,0xfe,0x02,0x69,0x0e,0x4d,0xbe,0x48,0x5b,0x17,0xa8,0x0b,0xc5,0xf2,0x1d,0x68,0x0f,0x1a,0x84,0x13,0x13,0x97,0x34,0xf7,0xf2,0xb0,0xdb,0x4e,0x25,0x37,0x50,0x01,0x8a,0xad,0x9e,0x86,0xd4,0x9b,0x60,0x04,0xbb,0xbc,0xf0,0x51,0xf5,0x2f,0xcb,0x66,0xd0,0xc5,0xfc,0xa6,0x3f,0xbf,0xe6,0x34,0x17,0x34,0x85,0xbb,0xbf,0x76,0x42,0xe9,0xdf,0x9c,0x74,0xb8,0x5b,0x68,0x55,0xe9,0x42,0x13,0xb8,0xc2,0xd8,0x91,0x62,0xab,0xef,0xf4,0x34,0x24,0x35,0x0e,0x96,0xbe,0x41,0xed,0xd4,0x2d,0xe9,0x9a,0x69,0x61,0x63,0x8c,0x1d,0xac,0x59,0x8b,0xc9,0x0d,0xa0,0x69,0xb5,0x0c,0x41,0x4d,0x8e,0xb8,0x65,0x2a,0xdc,0xff,0x4a,0x27,0x0d,0x56,0x7f,0x82,0x81,0x81,0x00,0x37,0x4d,0x40,0xbd,0xff,0x16,0x61,0xf3,0xc8,0xfa,0xb3,0xb9,0x9b,0x06,0x38,0x30,0x3a,0x61,0xdd,0x81,0xab,0x8d,0x01,0x79,0x87,0x3a,0x63,0xca,0x04,0x15,0x2f,0xe5,0x81,0x61,0x0c,0xea,0x44,0x0f,0x1c,0x34,0x5c,0xe4,0xb0,0xf4,0xdb,0x28,0x3a,0x7b,0xe6,0x5f,0xa1,0x93,0x08,0x4c,0x27,0x97,0x00,0x5d,0x25,0x91,0x42,0xb9,0xe9,0xe2,0x7a,0xf5,0xb2,0x3c,0x74,0xb5,0xca,0xa7,0x75,0xc3,0xb6,0x46,0xda,0x01,0x3f,0xce,0xbd,0x7a,0xf4,0x51,0x7e,0xd6,0x11,0x64,0x8c,0x2f,0xff,0x83,0x43,0xe3,0xc5,0x21,0x3a,0xc1,0x98,0xfd,0x4c,0x00,0xe4,0xe2,0x1c,0x64,0xb1,0xc3,0x0a,0xba,0x24,0xe1};
		    BYTE* asn1_message = ((BYTE*)&pRemoteEcsInd->asn1_message);
		    memcpy(asn1_message, pConstTandbergP3, p3_data_len);

		  }else if(POLYCOM_ENCRYPT){
		    BYTE pConstPolycomP3[272] = {0xa3,0x82,0x01,0x09,0x80,0x02,0x00,0x02,0x81,0x81,0x81,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xc9,0x0f,0xda,0xa2,0x21,0x68,0xc2,0x34,0xc4,0xc6,0x62,0x8b,0x80,0xdc,0x1c,0xd1,0x29,0x02,0x4e,0x08,0x8a,0x67,0xcc,0x74,0x02,0x0b,0xbe,0xa6,0x3b,0x13,0x9b,0x22,0x51,0x4a,0x08,0x79,0x8e,0x34,0x04,0xdd,0xef,0x95,0x19,0xb3,0xcd,0x3a,0x43,0x1b,0x30,0x2b,0x0a,0x6d,0xf2,0x5f,0x14,0x37,0x4f,0xe1,0x35,0x6d,0x6d,0x51,0xc2,0x45,0xe4,0x85,0xb5,0x76,0x62,0x5e,0x7e,0xc6,0xf4,0x4c,0x42,0xe9,0xa6,0x37,0xed,0x6b,0x0b,0xff,0x5c,0xb6,0xf4,0x06,0xb7,0xed,0xee,0x38,0x6b,0xfb,0x5a,0x89,0x9f,0xa5,0xae,0x9f,0x24,0x11,0x7c,0x4b,0x1f,0xe6,0x49,0x28,0x66,0x51,0xec,0xe6,0x53,0x81,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x82,0x81,0x81,0x00,0x65,0xca,0xed,0x47,0x5c,0xec,0x51,0x18,0x66,0x8d,0xcd,0x5d,0xfa,0xeb,0xb8,0x1b,0x0b,0x2e,0x65,0x35,0x1b,0x35,0x57,0xf6,0x42,0x3e,0x99,0x35,0xba,0x25,0xa6,0x98,0xfb,0xa2,0x9e,0x8e,0xc8,0x1b,0x4c,0xb4,0x9b,0x59,0x89,0x7a,0x48,0x56,0x5e,0xb6,0x67,0xbf,0xfe,0x7c,0x8d,0x8e,0x54,0x54,0x60,0x37,0x3a,0xc0,0x20,0x02,0x4f,0x5e,0x77,0x0d,0x10,0x5a,0xd2,0xa0,0x4b,0x48,0xf9,0xfc,0xab,0x37,0x51,0x80,0x42,0xf9,0x3f,0x03,0x91,0x61,0xf4,0xae,0x70,0x62,0xcf,0x8b,0x26,0x66,0xbf,0xa0,0x84,0x72,0x89,0x2b,0x3f,0x77,0x3e,0x4e,0xea,0xad,0x42,0xc5,0xc9,0x99,0x25,0x7d,0xda,0xd4,0x3d,0x36,0xc8,0xe4,0x75,0x57,0xb0,0x7b,0x89,0x36,0x97,0x81,0xdc,0xa1,0xa4,0xfe};
		    memcpy(&pRemoteEcsInd->asn1_message, pConstPolycomP3, p3_data_len);
		  }else if(WRONG_P3){
		    BYTE pWrongPolycomP3[272] = {0xa3,0x82,0x01,0x09,0x80,0x02,0x00,0x07,0x81,0x81,0x81,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xc9,0x0f,0xda,0xa2,0x21,0x68,0xc2,0x34,0xc4,0xc6,0x62,0x8b,0x80,0xdc,0x1c,0xd1,0x29,0x02,0x4e,0x08,0x8a,0x67,0xcc,0x74,0x02,0x0b,0xbe,0xa6,0x3b,0x13,0x9b,0x22,0x51,0x4a,0x08,0x79,0x8e,0x34,0x04,0xdd,0xef,0x95,0x19,0xb3,0xcd,0x3a,0x43,0x1b,0x30,0x2b,0x0a,0x6d,0xf2,0x5f,0x14,0x37,0x4f,0xe1,0x35,0x6d,0x6d,0x51,0xc2,0x45,0xe4,0x85,0xb5,0x76,0x62,0x5e,0x7e,0xc6,0xf4,0x4c,0x42,0xe9,0xa6,0x37,0xed,0x6b,0x0b,0xff,0x5c,0xb6,0xf4,0x06,0xb7,0xed,0xee,0x38,0x6b,0xfb,0x5a,0x89,0x9f,0xa5,0xae,0x9f,0x24,0x11,0x7c,0x4b,0x1f,0xe6,0x49,0x28,0x66,0x51,0xec,0xe6,0x53,0x81,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x82,0x81,0x81,0x00,0x65,0xca,0xed,0x47,0x5c,0xec,0x51,0x18,0x66,0x8d,0xcd,0x5d,0xfa,0xeb,0xb8,0x1b,0x0b,0x2e,0x65,0x35,0x1b,0x35,0x57,0xf6,0x42,0x3e,0x99,0x35,0xba,0x25,0xa6,0x98,0xfb,0xa2,0x9e,0x8e,0xc8,0x1b,0x4c,0xb4,0x9b,0x59,0x89,0x7a,0x48,0x56,0x5e,0xb6,0x67,0xbf,0xfe,0x7c,0x8d,0x8e,0x54,0x54,0x60,0x37,0x3a,0xc0,0x20,0x02,0x4f,0x5e,0x77,0x0d,0x10,0x5a,0xd2,0xa0,0x4b,0x48,0xf9,0xfc,0xab,0x37,0x51,0x80,0x42,0xf9,0x3f,0x03,0x91,0x61,0xf4,0xae,0x70,0x62,0xcf,0x8b,0x26,0x66,0xbf,0xa0,0x84,0x72,0x89,0x2b,0x3f,0x77,0x3e,0x4e,0xea,0xad,0x42,0xc5,0xc9,0x99,0x25,0x7d,0xda,0xd4,0x3d,0x36,0xc8,0xe4,0x75,0x57,0xb0,0x7b,0x89,0x36,0x97,0x81,0xdc,0xa1,0xa4,0xfe};
		    memcpy(&pRemoteEcsInd->asn1_message, pWrongPolycomP3, p3_data_len);
		  }
		    
  			break;
  		}
  		case P4_Identifier:
  		{

		  //  current block

		  char* ecs_data = (char*)&(tSetEcs.asn1_message);
		  BYTE header_byte = ecs_data[0];
		  switch(header_byte){
		  case SE_HEADER_SINGLE_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P4 single block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_FIRST_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P4 first block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_INTERMEDIATE_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P4 intermidiate block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_LAST_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P4 last block , ep - ", m_szEpName);
		    break;
		  }
		  default:{
		    PTRACE2INT(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P4 unknown block header = ", (DWORD)header_byte);
			PDELETEA(pIndArray);
			
		    return;
		  }
		}

			
		  PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - sending P4, ep - ", m_szEpName);

		  delete[] pIndArray;
		  DWORD p4_data_len = 132;
		  len = p4_data_len;
		  pIndArray = new BYTE[sizeof(REMOTE_ECS_SE) - sizeof (APIU8*) + p4_data_len];
		  pRemoteEcsInd = (REMOTE_ECS_SE*)pIndArray;


		  pRemoteEcsInd->len = p4_data_len;
		  BYTE pConstP4[132] = {0x84,0x81,0x81,0x00,0xa2,0xb9,0xd6,0x90,0xf4,0x40,0x4b,0x62,0x44,0x8d,0x7b,0x7e,0x37,0x59,0x48,0x5f,0x72,0x53,0x1a,0x7f,0x65,0xf7,0xc7,0x10,0x80,0xd4,0x57,0x89,0xba,0xdf,0xa7,0x6d,0x15,0x3a,0x9e,0x6d,0x37,0xc5,0xcb,0xf0,0x7e,0x6c,0xbf,0x8c,0x24,0x08,0x24,0x07,0xea,0x9a,0x47,0x1f,0x39,0x29,0x2f,0x6b,0x1d,0xcc,0xd1,0x6c,0x8e,0xa5,0x33,0xfa,0x8b,0xb6,0x3a,0xe1,0x4a,0x68,0xc8,0xc4,0x3c,0xca,0x38,0x04,0xc3,0xcc,0x4b,0x95,0x05,0x59,0x66,0x53,0x33,0x39,0x62,0x9b,0x2b,0x36,0x67,0xa9,0xbf,0x12,0xb7,0xde,0x43,0x8a,0xca,0x29,0xcf,0x1f,0xdc,0x17,0xdb,0xbf,0x83,0x50,0x19,0xaa,0x2c,0xbe,0x8f,0x8d,0xb6,0xfc,0xb8,0x91,0xf0,0x29,0x74,0xea,0x34,0x8d,0x09,0x31,0xaa,0x81};
		  BYTE* asn1_message = ((BYTE*)&pRemoteEcsInd->asn1_message);
		  memcpy(asn1_message, pConstP4, p4_data_len);
  			break;

  		}
 		case P5_Identifier:
 		{

		  //  current block

		  char* ecs_data = (char*)&(tSetEcs.asn1_message);
		  BYTE header_byte = ecs_data[0];
		  switch(header_byte){
		  case SE_HEADER_SINGLE_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P5 single block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_FIRST_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P5 first block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_INTERMEDIATE_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P5 intermidiate block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_LAST_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P5 last block , ep - ", m_szEpName);
		    break;
		  }
		  default:{
		    PTRACE2INT(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P5 unknown block header = ", (DWORD)header_byte);
			PDELETEA(pIndArray);
			
		    return;
		  }
		}

			
		  PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - sending P5, ep - ", m_szEpName);

		  delete[] pIndArray;
		  DWORD p5_data_len = 20;
		  len = p5_data_len;
		  pIndArray = new BYTE[sizeof(REMOTE_ECS_SE) - sizeof (APIU8*) + p5_data_len];
		  pRemoteEcsInd = (REMOTE_ECS_SE*)pIndArray;


		  pRemoteEcsInd->len = p5_data_len;
		  BYTE pConstP5[20] = {0xa5,0x0f,0x80,0x05,0x00,0x08,0x00,0x00,0x00,0x81,0x09,0x00,0xf0,0xff,0xff,0xff,0x88,0x88,0x51,0x01};
		  BYTE* asn1_message = ((BYTE*)&pRemoteEcsInd->asn1_message);
		  memcpy(asn1_message, pConstP5, p5_data_len);
  			break;



  
		}
 		case P6_Identifier:
 		{

		  //  current block

		  char* ecs_data = (char*)&(tSetEcs.asn1_message);
		  BYTE header_byte = ecs_data[0];
		  switch(header_byte){
		  case SE_HEADER_SINGLE_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P6 single block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_FIRST_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P6 first block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_INTERMEDIATE_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P6 intermidiate block , ep - ", m_szEpName);
			PDELETEA(pIndArray);
			
		    return;
		  }
		  case SE_HEADER_LAST_BLOCK:{
		    PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P6 last block , ep - ", m_szEpName);
		    break;
		  }
		  default:{
		    PTRACE2INT(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - received P6 unknown block header = ", (DWORD)header_byte);
			PDELETEA(pIndArray);
			
		    return;
		  }
		}

			
		  PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - sending P6, ep - ", m_szEpName);

		  delete[] pIndArray;
		  DWORD p6_data_len = 88;
		  len = p6_data_len;
		  pIndArray = new BYTE[sizeof(REMOTE_ECS_SE) - sizeof (APIU8*) + p6_data_len];
		  pRemoteEcsInd = (REMOTE_ECS_SE*)pIndArray;


		  pRemoteEcsInd->len = p6_data_len;

		  BYTE pConstP6[88] = {0xa6,0x53,0x80,0x11,0x00,0x53,0x6a,0x11,0x70,0x93,0x3b,0xc3,0x30,0xe3,0xcc,0x66,0x44,0x82,0x5a,0x41,0x6a,0x81,0x41,0x00,0x44,0x15,0x8c,0xc3,0x3b,0xd7,0xf2,0xd5,0x25,0xa8,0x82,0x37,0x99,0xa7,0xe6,0x30,0xfd,0x11,0x76,0x4d,0xd4,0xf0,0x3c,0xbe,0x3b,0x33,0xcb,0x6a,0xb7,0xe7,0x3f,0xde,0x8c,0x38,0xe8,0xc9,0x0f,0x7c,0x5f,0x45,0x74,0x29,0x01,0xdd,0x2c,0xd0,0xba,0xa5,0x48,0xc1,0xc6,0x64,0xab,0x96,0x3c,0xa1,0x55,0xac,0x1e,0x80,0xc0,0xcd,0x62,0x0e};
		  BYTE* asn1_message = ((BYTE*)&pRemoteEcsInd->asn1_message);
		  memcpy(asn1_message, pConstP6, p6_data_len);
  			break;


		}
  		case P8_Identifier:
  		{
		  PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - sending P8, ep - ", m_szEpName);
  			pRemoteEcsInd->len = len;
  			memcpy(&pRemoteEcsInd->asn1_message, pMsgInfo, len);
  			break;
  		}
  		case P9_Identifier:
  		{
		  PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnSetEcsReq - sending P9, ep - ", m_szEpName);
  			pRemoteEcsInd->len = len;
  			memcpy(&pRemoteEcsInd->asn1_message, pMsgInfo, len);
  			break;
  		}
  		default:
 		{
			DBGPASSERT(opcode+1000);
			break;
		} 		
  	} 
  	
//  for (WORD i=0; i<sizeof(SET_ECS_S)+20-sizeof(DWORD); i++)
//		TRACEINTO << "****** ECS - epsim *******"<< (DWORD)((BYTE*)pRemoteEcsInd)[i];
  	
  	
	SendRemoteEcsInd((BYTE*)pRemoteEcsInd, len);
	
	PDELETEA(pIndArray);
	
}


/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnNetClearReq(NET_CLEAR_REQ_S& tClearReq, CSegment* pNetSetupParams)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnNetClearReq - NET_CLEAR_REQ, ep - ", m_szEpName);
	m_apiArray[SIM_ISDN_CLEAR_REQ_RECEIVED]++;

	SetState(eEpStateDisconnecting);	
	
	
	// Get channel parameters 
	DWORD chanConnectionId, chanSpanId, chanPortId, isFirstChannel;
	if (NULL == pNetSetupParams)
	{
		PTRACE(eLevelError, "CEndpointISDN::OnNetClearReq - No channel params! ");
		PASSERT(1);
		return;
	}
	
///	CSegment* pNetSetupParamsNetClearInd = new CSegment(*pNetSetupParams);
	
	*pNetSetupParams >> chanConnectionId;
///					 >> chanSpanId
///					 >> chanPortId
///					 >> isFirstChannel;
	
	//clean the relevant channel found using the connectionId
	for (int i = 0; i < MAX_ADDITIONAL_PHONE_NUM; i++)
	{
		if (m_channelArr[i]->GetConnectionId() == chanConnectionId)
		{
			m_channelArr[i]->SetState(eEpStateDisconnected);
			m_connectedChannelsCounter--;
			break;
		}
	}
	

///	SendClearInd(pNetSetupParamsNetClearInd);
	SendClearInd(chanConnectionId);
	
	TRACEINTO << "CEndpointISDN::OnNetClearReq - NET_CLEAR_REQ, m_connectedChannelsCounter=" << m_connectedChannelsCounter;
	 
	
	//check if this is the last channel to be cleared (and mux)
	
	if (m_connectedChannelsCounter == 0)// apply this code only on last NET_CLEAR_REQ and after MUX is disconnected !!!
	{
		CleanAfterDisconnect();
		
		SetState(eEpStateDisconnected);
		
		// check this 
		if( TRUE == m_isToBeDeleted )
			m_isReadyToDelete = TRUE;
	}
	
	
	
}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnNetDisconnectAckReq(NET_DISCONNECT_ACK_REQ_S& tDiscoAckReq)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::OnNetDisconnectAckReq - NET_DISCONNECT_REQ, ep - ", m_szEpName);
	m_apiArray[SIM_ISDN_DISCONNECT_ACK_REQ_RECEIVED]++;

	if( m_apiArray[SIM_ISDN_DISCONNECT_IND_SENT] > 0 )
	{
		SendDisconnectAckInd();
		CleanAfterDisconnect();
		SetState(eEpStateDisconnected);

		if( TRUE == m_isToBeDeleted )
			m_isReadyToDelete = TRUE;
	}
	else
	{
		PASSERT(1);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendSetupInd(int channelIndex)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::SendSetupInd - NET_SETUP_IND, ep - ", m_szEpName );
	m_apiArray[SIM_ISDN_SETUP_IND_SENT]++;

	// prepare and send SETUP_IND
	NET_SETUP_IND_S  rSetupStruct;
	memset(&rSetupStruct, 0, sizeof(NET_SETUP_IND_S));

	if (0 == channelIndex)
	{
		m_confID = 0;
		m_partyID = 0;
	}
	else 	// for additional channels of an ISDN party
	{
		PTRACE(eLevelInfoNormal, "CEndpointISDN::SendSetupInd - additional channel (not the first channel)");
	}
	
	m_connectionID = LOBBY_CONNECTION_ID;
	// fill setup struct
	// net header
///	FillCommonNetHeader(rSetupStruct.net_common_header);
	rSetupStruct.net_common_header.span_id              = m_channelArr[channelIndex]->GetRtmBoardDetails().GetRtmSpanId();
	rSetupStruct.net_common_header.net_connection_id    = m_channelArr[channelIndex]->GetNetConnectionId();
	rSetupStruct.net_common_header.physical_port_number = m_channelArr[channelIndex]->GetRtmBoardDetails().GetRtmPortId();

	rSetupStruct.net_spfc  = 1;
	rSetupStruct.call_type = 2;
	// Calling party
	rSetupStruct.calling_party.num_type = 6;//ACU_NB_TYPE_UNKNOWN;
	rSetupStruct.calling_party.num_plan = 7;//ACU_NB_PLAN_UNKNOWN;
	rSetupStruct.calling_party.presentation_ind = 9;//ACU_NB_PRES_ALLOWED;
	rSetupStruct.calling_party.screening_ind    = 9;//ACU_NB_SCREEN_USER_PROVIDED;
	
	rSetupStruct.calling_party.num_digits = strlen(m_szCliPhoneNum);
	
	strncpy(((char*)rSetupStruct.calling_party.digits), m_szCliPhoneNum, strlen(m_szCliPhoneNum));
	
	// Called party
	rSetupStruct.called_party.num_type = 6;//ACU_NB_TYPE_UNKNOWN;
	rSetupStruct.called_party.num_plan = 7;//ACU_NB_PLAN_UNKNOWN;
	//rSetupStruct.called_party.num_digits = strlen(m_szCliPhoneNum);
	//strncpy(((char*)rSetupStruct.called_party.digits),m_szCliPhoneNum,strlen(m_szCliPhoneNum));
///	rSetupStruct.called_party.num_digits = strlen(m_szPhoneNum[0]);
	rSetupStruct.called_party.num_digits = strlen(m_channelArr[channelIndex]->GetPhoneNum());
///	strncpy(((char*)rSetupStruct.called_party.digits), m_szPhoneNum[0], strlen(m_szPhoneNum[0]));
	strncpy(((char*)rSetupStruct.called_party.digits), m_channelArr[channelIndex]->GetPhoneNum(), strlen(m_channelArr[0]->GetPhoneNum()));
	
	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg, NET_SETUP_IND, (BYTE*)(&rSetupStruct), sizeof(NET_SETUP_IND_S));

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendProgressInd()
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::SendProgressInd - NET_PROGRESS_IND, ep - ", m_szEpName );
	m_apiArray[SIM_ISDN_PROGRESS_IND_SENT]++;

//	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendAlertInd(NET_SETUP_REQ_S& tSetupReq, CSegment* pNetSetupParams)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::SendAlertInd - NET_ALERT_IND, ep - ", m_szEpName );
	m_apiArray[SIM_ISDN_ALERT_IND_SENT]++;

	// prepare and send ALERT_IND
	NET_ALERT_IND_S  rAlertStruct;
	memset(&rAlertStruct, 0, sizeof(NET_ALERT_IND_S));

	// fill alert struct net header
	//FillCommonNetHeader(rAlertStruct.net_common_header);
	
	DWORD chanConnectionId, chanSpanId, chanPortId;
	CSegment *pParamTemp = new CSegment;
	*pParamTemp = *pNetSetupParams;
	
	*pParamTemp >> chanConnectionId
				>> chanSpanId
				>> chanPortId;
	
	rAlertStruct.net_common_header.span_id              = chanSpanId;
	rAlertStruct.net_common_header.net_connection_id    = tSetupReq.net_setup_header.net_connection_id;
	rAlertStruct.net_common_header.virtual_port_number  = tSetupReq.net_setup_header.virtual_port_number;
	rAlertStruct.net_common_header.physical_port_number = chanPortId;

	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg, NET_ALERT_IND, (BYTE*)(&rAlertStruct), sizeof(NET_ALERT_IND_S), chanConnectionId);

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
	
	POBJDELETE(pParamTemp);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendConnectInd(CSegment* pNetSetupParams)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::SendConnectInd - NET_CONNECT_IND, ep - ", m_szEpName );
	m_apiArray[SIM_ISDN_CONNECT_IND_SENT]++;

	// prepare and send CONNECT_IND
	NET_CONNECT_IND_S  rConnectStruct;
	memset(&rConnectStruct,0,sizeof(NET_CONNECT_IND_S));

	// fill connect struct
		// net header
///	FillCommonNetHeader(rConnectStruct.net_common_header);

	DWORD chanConnectionId, chanSpanId, chanPortId;
	CSegment *pParamTemp = new CSegment;
	*pParamTemp = *pNetSetupParams;
	
	*pParamTemp >> chanConnectionId
				>> chanSpanId
				>> chanPortId;
	
	rConnectStruct.net_common_header.span_id              = chanSpanId;
	rConnectStruct.net_common_header.net_connection_id    = m_netConnectionId;;
	rConnectStruct.net_common_header.physical_port_number = chanPortId;

	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg, NET_CONNECT_IND, (BYTE*)(&rConnectStruct), sizeof(NET_CONNECT_IND_S), chanConnectionId);

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
	
	POBJDELETE(pParamTemp);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendDisconnectInd()
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::SendDisconnectInd - NET_DISCONNECT_IND, ep - ", m_szEpName );
	m_apiArray[SIM_ISDN_DISCONNECT_IND_SENT]++;

	// prepare and send DISCONNECT_IND
	NET_DISCONNECT_IND_S  rDisConnectStruct;
	memset(&rDisConnectStruct, 0, sizeof(NET_DISCONNECT_IND_S));

	// fill disconnect struct
		// net header
///	FillCommonNetHeader(rDisConnectStruct.net_common_header);
	
	rDisConnectStruct.net_common_header.span_id              = m_channelArr[0]->GetRtmBoardDetails().GetRtmSpanId();
	rDisConnectStruct.net_common_header.net_connection_id    = m_netConnectionId;;
	rDisConnectStruct.net_common_header.physical_port_number = m_channelArr[0]->GetRtmBoardDetails().GetRtmPortId();


	rDisConnectStruct.cause.cause_val = 0; // causDEFAULT_VAL;

	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg,NET_DISCONNECT_IND,(BYTE*)(&rDisConnectStruct),sizeof(NET_DISCONNECT_IND_S), m_channelArr[0]->GetConnectionId());

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendDisconnectAckInd()
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::SendDisconnectAckInd - NET_DISCONNECT_ACK_IND, ep - ", m_szEpName );
	m_apiArray[SIM_ISDN_DISCONNECT_ACK_IND_SENT]++;

	// prepare and send DISCONNECT_IND
	NET_DISCONNECT_ACK_IND_S  rDisConnectAckStruct;
	memset(&rDisConnectAckStruct,0,sizeof(NET_DISCONNECT_ACK_IND_S));

	// fill disconnect struct
		// net header
///	FillCommonNetHeader(rDisConnectAckStruct.net_common_header);

	rDisConnectAckStruct.net_common_header.span_id              = m_channelArr[0]->GetRtmBoardDetails().GetRtmSpanId();
	rDisConnectAckStruct.net_common_header.net_connection_id    = m_netConnectionId;;
	rDisConnectAckStruct.net_common_header.physical_port_number = m_channelArr[0]->GetRtmBoardDetails().GetRtmPortId();

	rDisConnectAckStruct.cause.cause_val = 0; // causDEFAULT_VAL;

	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg, NET_DISCONNECT_ACK_IND, (BYTE*)(&rDisConnectAckStruct), sizeof(NET_DISCONNECT_ACK_IND_S), m_channelArr[0]->GetConnectionId());

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendClearInd(DWORD chanConnectionId /*CSegment* pNetSetupParams*/)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::SendClearInd - NET_CLEAR_IND, ep - ", m_szEpName );
	m_apiArray[SIM_ISDN_CLEAR_IND_SENT]++;

	// prepare and send DISCONNECT_IND
	NET_CLEAR_IND_S  rClearStruct;
	memset(&rClearStruct, 0, sizeof(NET_CLEAR_IND_S));

	// fill clear struct
		// net header
	FillCommonNetHeader(rClearStruct.net_common_header);
	rClearStruct.cause.cause_val = 0; // causDEFAULT_VAL;
	

	CSegment rMsgSeg;

	PrepareNetMessageSeg(rMsgSeg, NET_CLEAR_IND, (BYTE*)(&rClearStruct), sizeof(NET_CLEAR_IND_S), chanConnectionId);

	::SendIsdnMessageToGideonSimApp(rMsgSeg);
}



/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendMuteIndication() const
{
	PTRACE2(eLevelError, "CEndpointISDN::SendMuteIndication , Name - ", m_szEpName );
	
	CSegment  H221String, H320StringToSend;
	H221String  << (BYTE)(H230_Esc | ESCAPECAPATTR)
	            << (BYTE)(H230_Code_000 | AIM);
	    
	PrepareH230(H221String,H320StringToSend);
	
	CSegment rMsgSeg;
	PrepareMuxMessageSeg(rMsgSeg, REMOTE_CI, H320StringToSend.GetPtr(),H320StringToSend.GetWrtOffset());
	::SendMuxMessageToGideonSimApp(rMsgSeg);
	
	
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendUnmuteIndication() const
{
	PTRACE2(eLevelError, "CEndpointISDN::SendUnmuteIndication , Name - ", m_szEpName );
	CSegment  H221String, H320StringToSend;
	H221String  << (BYTE)(H230_Esc | ESCAPECAPATTR)
	            << (BYTE)(H230_Code_000 | AIA);
		    
	PrepareH230(H221String,H320StringToSend);
	
	CSegment rMsgSeg;
	PrepareMuxMessageSeg(rMsgSeg, REMOTE_CI, H320StringToSend.GetPtr(),H320StringToSend.GetWrtOffset());
	::SendMuxMessageToGideonSimApp(rMsgSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendFeccTokenRequestIndication() const
{
	PTRACE2(eLevelError, "CEndpointISDN::SendFeccTokenRequestIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendFeccTokenReleaseIndication() const
{
	PTRACE2(eLevelError, "CEndpointISDN::SendFeccTokenReleaseIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendFeccKeyRequestIndication(const char* pszDtmf) const
{
        PTRACE2(eLevelError, "CEndpointISDN::SendFeccKeyRequestIndication (NOTHING), Name - ", m_szEpName );
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendH239TokenRequestIndication()
{
	PTRACE2(eLevelError, "CEndpointISDN::SendH239TokenRequestIndication , Name - ", m_szEpName );
			//		  subOpcode				  isAckNack		isRand	,  randNum 
	SendH239Message(PresentationTokenRequest,  	0 		,     1		,	RAND_NUM_FOR_EP_SIM);
	m_enRoleTokenLastCmd = kPresentationTokenRequest;

}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendH239TokenReleaseIndication()
{
	PTRACE2(eLevelError, "CEndpointISDN::SendH239TokenReleaseIndication , Name - ", m_szEpName );
	SendH239Message(PresentationTokenRelease);
	m_enRoleTokenLastCmd = kPresentationTokenRelease;
	DeleteTimer(ROLE_TOKEN_OWNER_TOUT);
}

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendLprModeChangeRequestIndication(DWORD lossProtection,DWORD mtbf,DWORD congestionCeiling,DWORD fill,DWORD modeTimeout)
{
	PTRACE2(eLevelError, "CEndpointISDN::SendLprModeChangeRequestIndication (NOTHING), Name - ", m_szEpName );	
}
/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::PreparePhoneNumbers()
{
	ALLOCBUFFER(buff, BND_MAX_PHONE_LEN+1);
	
	for (int index=1; index < (m_numOfBondingChannels); index++)
	{
        ostringstream str;
		memset(buff, '\0', BND_MAX_PHONE_LEN+1);
		if (7 > BND_MAX_PHONE_LEN)	// we count on it!
			PASSERT(BND_MAX_PHONE_LEN);	
		snprintf(buff, BND_MAX_PHONE_LEN+1, "%7d", g_nextPhoneNumber);
		m_channelArr[index]->SetPhoneNum(buff);
		
        PTRACE2(eLevelInfoNormal, "CEndpointISDN::PreparePhoneNumbers string phone num: ", buff );
        
        
        // Change phone number format from string to numerical value per digit (= emb_digit)
        for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++)
        {
            str << buff[digit_index] << "==>";
            BYTE emb_digit = (BYTE)(buff[digit_index]-'0');
            buff[digit_index]=emb_digit;// convert '1' to (BYTE)1
            str << (WORD)buff[digit_index] << " , ";
        }
		
 		buff[BND_MAX_PHONE_LEN] = BND_EON;
        str << buff[BND_MAX_PHONE_LEN] << "\n ";
		m_channelArr[index]->SetPhoneNumDigit(buff);       
            
        PTRACE2(eLevelInfoNormal,"CEndpointISDN::PreparePhoneNumbers embedded phone num: ",str.str().c_str());
        

	}
	
	g_nextPhoneNumber += 100;	// for the next party
	
	DEALLOCBUFFER(buff);
}



/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SetPhoneNumbers(const BYTE* phoneNumber)
{
	
	ALLOCBUFFER(buff, BND_MAX_PHONE_LEN+1);

	for (int index=1; index < (m_numOfBondingChannels); index++)
	{
		m_channelArr[index]->SetPhoneNumDigit((char*)phoneNumber);
	
		// Convert phone number from numerical value to string
        for(int digit_index = 0; digit_index < BND_MAX_PHONE_LEN; digit_index++)
        {
        	buff[digit_index] = (phoneNumber[digit_index] + '0');
        }

		m_channelArr[index]->SetPhoneNum(buff);
		
///		m_channelArr[index]->SetPhoneNum("3000");
	}
	
	DEALLOCBUFFER(buff);
	
	
	
//	ALLOCBUFFER(buff, BND_MAX_PHONE_LEN+1);
//	
//	for (int index=1; index < (m_numOfBondingChannels); index++)
//	{
//        ostringstream str;
//		memset(buff, '\0', BND_MAX_PHONE_LEN+1);
//		if (7 > BND_MAX_PHONE_LEN)	// we count on it!
//			PASSERT(BND_MAX_PHONE_LEN);	
//		sprintf(buff, "%7d", g_nextPhoneNumber);
//		m_channelArr[index]->SetPhoneNum(buff);
//		
//        PTRACE2(eLevelInfoNormal, "CEndpointISDN::PreparePhoneNumbers string phone num: ", buff );
//        
//        
//        // Change phone number format from string to numerical value per digit (= emb_digit)
//        for(int digit_index=0;digit_index<BND_MAX_PHONE_LEN;digit_index++)
//        {
//            str << buff[digit_index] << "==>";
//            BYTE emb_digit = (BYTE)(buff[digit_index]-'0');
//            buff[digit_index]=emb_digit;// convert '1' to (BYTE)1
//            str << (WORD)buff[digit_index] << " , ";
//        }
//		
// 		buff[BND_MAX_PHONE_LEN] = BND_EON;
//        str << buff[BND_MAX_PHONE_LEN] << "\n ";
//		m_channelArr[index]->SetPhoneNumDigit(buff);
//        
//        PTRACE2(eLevelInfoNormal,"CEndpointISDN::PreparePhoneNumbers embedded phone num: ",str.str().c_str());
//        
//	}
//	
//	g_nextPhoneNumber += 100;	// for the next party
//	
//	DEALLOCBUFFER(buff);
}



/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendBondingEndNegotiationInd(BND_CONNECTION_INIT_REQUEST_S& tBondingInitReq)
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::SendBondingEndNegotiationInd - BND_END_NEGOTIATION_INDICATION_S, ep - ", m_szEpName );
	m_apiArray[SIM_ISDN_BONDING_END_NEGOTIATION_IND_SENT]++;

	int numOfBytes = m_numOfBondingChannels*(BND_MAX_PHONE_LEN);
	
	// prepare and send BND_END_NEGOTIATION	
	
	BYTE* pIndArray = new BYTE[sizeof(BND_END_NEGOTIATION_INDICATION_S) + numOfBytes];
	
	BND_END_NEGOTIATION_INDICATION_S* pEndNegotiationInd = (BND_END_NEGOTIATION_INDICATION_S*)pIndArray;	

	// fill end negotioation struct
	pEndNegotiationInd->callParams.direction = tBondingInitReq.callParams.direction;
	pEndNegotiationInd->callParams.NumOfBndChnls = tBondingInitReq.callParams.NumOfBndChnls;
	pEndNegotiationInd->callParams.restrictType = tBondingInitReq.callParams.restrictType;
	pEndNegotiationInd->callParams.dummy = tBondingInitReq.callParams.dummy;

	for (int index = 1; index < MAX_ADDITIONAL_PHONE_NUM; index++)
	{
		memcpy(pEndNegotiationInd->phoneList.startOfPhoneList[index-1].digits, m_channelArr[index]->GetPhoneNumDigit(), BND_MAX_PHONE_LEN);	// include the null-terminated sign
	}
	pEndNegotiationInd->phoneList.numberOfPhoneNums = (m_numOfBondingChannels -1);
	pEndNegotiationInd->phoneList.dummy[0] = (BYTE)(-1);
	pEndNegotiationInd->phoneList.dummy[1] = (BYTE)(-1);
	pEndNegotiationInd->phoneList.dummy[2] = (BYTE)(-1);
	
	CSegment rMsgSeg;
	PrepareMuxMessageSeg(rMsgSeg, BND_END_NEGOTIATION, (BYTE*)(pEndNegotiationInd), sizeof(BND_END_NEGOTIATION_INDICATION_S) + numOfBytes);
	::SendMuxMessageToGideonSimApp(rMsgSeg);

	PDELETEA(pIndArray);
}


/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::PrepareMuxMessageSeg(CSegment& rMsgSeg, const DWORD opcode, BYTE* pDataBytes, const DWORD dwDataSize) const
{
	rMsgSeg << opcode;

	// put MUX parameters
	rMsgSeg	<< (WORD)m_muxParams.m_muxBoardId		// MUX board ID
			<< (WORD)0								// MUX sub board (not in use)
			<< (WORD)m_muxParams.m_muxUnitId;		// MUX unit ID

	// put party details
	rMsgSeg	<< (DWORD)m_confID
			<< (DWORD)m_partyID
			<< (DWORD)m_muxParams.m_muxConnectionId;	// MUX connection ID

	rMsgSeg	<< (WORD)m_muxParams.m_muxPortId;
	rMsgSeg.Put(pDataBytes,dwDataSize);
}

/////////////////////////////////////////////////////////////////////////////
// void CEndpointISDN::SendRemoteEcsInd(BYTE* pRemoteEcsInd, WORD msgDataLen)
// {
// 	PTRACE2(eLevelInfoNormal, "CEndpointISDN::SendRemoteEcsInd - REMOTE_ECS_SE, ep - ", m_szEpName );
	
// 	// prepare and send REMOTE_ECS		
// 	CSegment rMsgSeg;

// 	PrepareMuxMessageSeg(rMsgSeg, REMOTE_ECS, (BYTE*)(pRemoteEcsInd), sizeof(REMOTE_ECS_SE) -sizeof(APIU8*) + msgDataLen);

// 	::SendMuxMessageToGideonSimApp(rMsgSeg);
	
// }
/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendRemoteEcsInd(BYTE* pRemoteEcsInd, DWORD msgDataLen)
{
  PTRACE2INT(eLevelInfoNormal, "CEndpointISDN::SendRemoteEcsInd - REMOTE_ECS_SE,  msgDataLen - ", msgDataLen );
	
  CSegment* rMsgSeg = NULL;

  if(msgDataLen <= ECS_BLOCK_DATA_SIZE){
    PTRACE2(eLevelInfoNormal, "CEndpointISDN::SendRemoteEcsInd - sending single block, Name - ", m_szEpName);
     rMsgSeg = new CSegment();
     PrepareMuxMessageSeg(*rMsgSeg, REMOTE_ECS, (BYTE*)(pRemoteEcsInd), sizeof(REMOTE_ECS_SE) -sizeof(APIU8*) + msgDataLen);
     ::SendMuxMessageToGideonSimApp(*rMsgSeg);
     //    POBJDELETE(rMsgSeg);

//     // send single block
//     //    DWORD data_len = msgDataLen+1+4;
//     DWORD data_len = msgDataLen;
//     BYTE* blockData = new BYTE[data_len+5];
//     memcpy(&blockData[0],&data_len,4);
//     blockData[4] = SE_HEADER_SINGLE_BLOCK;
//     for(WORD i=0;i<data_len-5;i++){
//       blockData[i+5] = pRemoteEcsInd[i+5];
//     }
//     // prepare and send REMOTE_ECS		
//     //    CSegment rMsgSeg;
//     rMsgSeg = new CSegment();
//     PrepareMuxMessageSeg(*rMsgSeg, REMOTE_ECS, blockData, sizeof(REMOTE_ECS_SE) -sizeof(APIU8*) + data_len);
//     ::SendMuxMessageToGideonSimApp(*rMsgSeg);
//     delete[]blockData;
//     POBJDELETE(rMsgSeg);
  }else{
    DWORD remaining_length = msgDataLen;
    WORD data_index = 0;
    WORD block_counter = 1;
    
    // send first block
    PTRACE2INT(eLevelInfoNormal, "CEndpointISDN::SendRemoteEcsInd - sending first block, block_counter: ", block_counter);
    // 0x84,0x81,0x81,0x00,0xa2,0xb9,0xd6,0x90,0xf4
    DWORD data_len = ECS_BLOCK_DATA_SIZE;
    DWORD data_len_with_header = ECS_BLOCK_DATA_SIZE+1;
    BYTE* blockData = new BYTE[data_len+5];
    memcpy(blockData,&data_len_with_header,4);
    blockData[4] = SE_HEADER_FIRST_BLOCK;
    data_index+=4;
    for(WORD i=0;i<data_len;i++){
      blockData[i+5] = pRemoteEcsInd[data_index++];
    }
    // prepare and send REMOTE_ECS		
    //    CSegment rMsgSeg;
    rMsgSeg = new CSegment();
    PrepareMuxMessageSeg(*rMsgSeg, REMOTE_ECS, blockData, sizeof(REMOTE_ECS_SE) -sizeof(APIU8*) + data_len_with_header);
    PTRACE(eLevelInfoNormal, "CEndpointISDN::SendRemoteEcsInd - sending first block, start send");
    //SystemSleep(100);
    ::SendMuxMessageToGideonSimApp(*rMsgSeg);
    SystemSleep(20);
     PTRACE(eLevelInfoNormal, "CEndpointISDN::SendRemoteEcsInd - sending first block, end send");
    delete[]blockData;
    POBJDELETE(rMsgSeg);
    remaining_length-=data_len;

    while(remaining_length > ECS_BLOCK_DATA_SIZE){
      block_counter++;
      PTRACE2INT(eLevelInfoNormal, "CEndpointISDN::SendRemoteEcsInd - sending intermidiate block, block_counter: ", block_counter);
      data_len = ECS_BLOCK_DATA_SIZE;
      data_len_with_header = ECS_BLOCK_DATA_SIZE+1;
      blockData = new BYTE[data_len+5];
      memcpy(blockData,&data_len_with_header,4);
      blockData[4] = SE_HEADER_INTERMEDIATE_BLOCK;
      for(WORD i=0;i<data_len;i++){
	blockData[i+5] = pRemoteEcsInd[data_index++];
      }
      remaining_length-=data_len;
      // prepare and send REMOTE_ECS		
      //CSegment rMsgSeg;
      rMsgSeg = new CSegment();
      PrepareMuxMessageSeg(*rMsgSeg, REMOTE_ECS, blockData, sizeof(REMOTE_ECS_SE) -sizeof(APIU8*) + data_len_with_header);
      ::SendMuxMessageToGideonSimApp(*rMsgSeg);
      SystemSleep(20);
      delete[]blockData;
      POBJDELETE(rMsgSeg);
    }
    // send last block
    block_counter++;
    PTRACE2INT(eLevelInfoNormal, "CEndpointISDN::SendRemoteEcsInd - sending last block, block_counter: ", block_counter);
    data_len = remaining_length;
    data_len_with_header = remaining_length+1;
    blockData = new BYTE[data_len+5];
    memcpy(blockData,&data_len_with_header,4);
    blockData[4] = SE_HEADER_LAST_BLOCK;
    for(WORD i=0;i<data_len;i++){
      blockData[i+5] = pRemoteEcsInd[data_index++];
    }
    // prepare and send REMOTE_ECS		
    //CSegment rMsgSeg;
    rMsgSeg = new CSegment();
    PrepareMuxMessageSeg(*rMsgSeg, REMOTE_ECS, blockData, sizeof(REMOTE_ECS_SE) -sizeof(APIU8*) + data_len_with_header);
    SystemSleep(20);
    ::SendMuxMessageToGideonSimApp(*rMsgSeg);
    delete[]blockData;
    POBJDELETE(rMsgSeg);
  }
}

//===============================================================================================================

/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SetNetConnectionIDs()
{
	PTRACE2(eLevelInfoNormal, "CEndpointISDN::SetNetConnectionIDs, Name - ", m_szEpName);
	
	DWORD channelNetConnectionId = GetCallIndex();
	for (int i=0; i< MAX_ADDITIONAL_PHONE_NUM; i++)
	{
		m_channelArr[i]->SetNetConnectionId(channelNetConnectionId++);
	}
}


/////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SetEpConnectionID( const DWORD connID, const DWORD netConnectionId)
{
	int index = FindChannelIndex(netConnectionId);
	if (index >= 0 && index <= MAX_ADDITIONAL_PHONE_NUM)
	{
		m_channelArr[index]->SetConnectionId(connID);
	}
	else
	{
		TRACEINTO << "CEndpointISDN::SetEpConnectionID - couldn't find channel by netConnectionId <"
				  << netConnectionId << ">";
	}
}

/////////////////////////////////////////////////////////////////////////////
int CEndpointISDN::FindChannelIndex(DWORD netConnId) const
{
	for (int i = 0; i < MAX_ADDITIONAL_PHONE_NUM; i++)
	{
		if (m_channelArr[i]->GetNetConnectionId() == netConnId)
		{
			return i;
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
//
//	add the size of the message (4 BYTES) at the start of the message
//
void CEndpointISDN::PrepareH230(CSegment& h230String, CSegment& StringToSend) const
{
	
	BAS_CMD_DESC msg; 
			
	msg.number_of_bytes = h230String.GetWrtOffset();
	StringToSend.Put((BYTE*)&msg, sizeof(BAS_CMD_DESC));
	StringToSend << h230String;

	//StringToSend.DumpHex();
	
}
/////////////////////////////////////////////////////////////////////////////
//
//	Handle SEND_H_230 from RMX 
void CEndpointISDN::OnMuxH230(CSegment* pParam)
{
	
	//pParam->DumpHex();
	CMedString msg;
	msg << "CEndpointISDN::OnMuxH230 : Name - " << m_szEpName << "\n\t";
	
	BYTE   opcode = 0;
	*pParam >> opcode;
	switch ( opcode ) {

	case (Start_Mbe | ESCAPECAPATTR)  :  {
		msg << " H230 - Start_Mbe "; 
		PTRACE(eLevelInfoNormal,msg.GetString());
		OnMuxH230Mbe(pParam);
		break;
	}
	case (Ns_Com | ESCAPECAPATTR):{
		msg << " H230 - Ns_Comm "; 
		PTRACE(eLevelInfoNormal,msg.GetString());
		// epep OnMuxH230NS_Com(pParam);
		break;
	}
	
	case (H230_Esc | ESCAPECAPATTR) :  {
		msg << " H230 - "; 
		WORD i = 0;
		H230_ENTRY*  pEntry = m_H230Entries;
		BYTE opcode;
		*pParam >> opcode;
		while ( pEntry->actFunc != 0 ) {
			if ( pEntry->opcode == opcode )  {
				msg << pEntry->opcodeStr << " , " << pEntry->descStr;
				PTRACE(eLevelInfoNormal,msg.GetString());       
				(this->*(pEntry->actFunc))(pParam); 
				break;
			}  
			pEntry++;
		}
		if ( pEntry->actFunc == 0 )  {
			char buf[30];
			sprintf(buf," %02x",opcode);
			msg << "\'opcode not found !!!\', Opcode is: " << buf;
			PTRACE(eLevelError,msg.GetString());
		}  
		
		break;
	}
	
	default   :  {  // bas h230 opcodes
		//OnMuxH230Bas(opcode);      
		break;
	}
	}

}
//////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::OnMuxH230Mbe(CSegment* pParam)
{
//	ALLOCBUFFER(msg,ONE_LINE_BUFFER_LEN * 2);
	CMedString msg;
	msg << "CEndpointISDN::OnMuxH230Mbe : Name - " << m_szEpName << "\n";
	
	BYTE   traceAtEnd = TRUE;
	BYTE   msgLen   = 0;
	BYTE   opcode   = 0;
	*pParam >> msgLen >> opcode;
   switch ( opcode ) {
     case TIL  :  {
    	 msg << " H230 - TIL";
       break;
     }
     case IIS  :  {
    	 msg << " H230 - IIS";
       BYTE  tcs_n = 0;      
       *pParam >> tcs_n;
       
 	  switch(tcs_n )
 	  {
 	  	msg << " TCS_";
 	    case 1:
 	    {
 	    	msg << "1";
            break;
 		}
        case 2:
        {
        	msg << "2";
 			break;
 		}      
 		case 4:
 		{
 			msg << "4";
 			break;
 		}      
        default: 
        {
 			break;
 		}
 	  }
 	  break;
 	}
     case TIR  :  {
    	 msg << " H230 - TIR";
 	   break;
     }
     case TIP  :  {
    	 msg << " H230 - TIP";
 	  break;
     }
    case NIA  :  {
    	msg << " H230 - NIA";
      break;
    }
    case NIAP  :  {
    	msg << " H230 - NIAP";
      break;
    }
    case Au_MAP  :  {
    	msg << " H230 - Au_MAP";
      break;
    }
    case MRQ  :  {
    	msg << " H230 - MRQ";
      break;
    }
 	case VideoNotDecodedMBs  :  {
 		msg << " H230 - VideoNotDecodedMBs";
     }
     case H239_messsage  :  {
    	msg << " H230 - H239_messsage";
    	PTRACE(eLevelInfoNormal,msg.GetString());
    	traceAtEnd = FALSE;
 		OnMuxH239Message(msgLen,pParam);
 		break;
     }
 	case AMC_CI  :  {
 		msg << " H230 - AMC-CI_messsage";
 		// - epep OnMuxAMC_CI(msgLen,pParam);
 		break;
     }
 	case h239ExtendedVideoCapability :	{
 	  msg << " H320 - h239ExtendedVideoCapability";
 	  PTRACE(eLevelInfoNormal,msg.GetString());
 	  traceAtEnd = FALSE;
 	  // Reply To MCU with the same H239ExtendedVideoCaps
 	  SendH230(pParam);
 	  break;
 	  
 	}
 	case AMC_cap :	{
 		msg << " H230 - AMC_cap ERROR - should be sent in caps set";
 	 // - epep m_pMuxCntl->SetRmtAMCCaps(*pParam, msgLen);
 	  break;
 	}
     default   :  {
      msg << " H230 - ???";
 	  break;
     }
     
   }
   PTRACE(eLevelInfoNormal,msg.GetString());     
//    DEALLOCBUFFER(msg);
}
//////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnMuxH239Message(BYTE msgLen,CSegment* pParam)
{
	BYTE subMsgIdentifier;
	int channelId = 0, terminalLabel = 0, symmetryBreaking = 0, bitRate = 0;
	BYTE MCUNumber = 0, terminalNumber = 0, label = 0, PID = 0, ack_nak = 0;
	*pParam >> subMsgIdentifier;
	if(IsH239Call())
	{
		BYTE  traceAtEnd = TRUE;
		CLargeString str;
		str << "CEndpointISDN::OnMuxH239Message";
		switch (subMsgIdentifier)
		{
			case PresentationTokenResponse:
			{
				*pParam >> ack_nak;
				terminalLabel = MbeBytesToInt(pParam);
				channelId = MbeBytesToInt(pParam);
				terminalNumber = terminalLabel%TERMINAL_LABEL_MULTIPLIER;
				MCUNumber = (terminalLabel-terminalNumber)/TERMINAL_LABEL_MULTIPLIER;
				str << "PresentationTokenResponse: Channel Id: "<< channelId<< " MCUNumber: "<<MCUNumber<<" terminalNumber: "<<terminalNumber <<"\n"; 
				DBGPASSERT(MCUNumber != m_nMcuId);
				DBGPASSERT(terminalNumber != m_nTerminalId);
				DeleteTimer(ROLE_TOKEN_OWNER_TOUT);
				if(ack_nak==AcknowledgeParamIdent ) // if is_ack
				{
					// ack for request_token
					if( kPresentationTokenRequest == m_enRoleTokenLastCmd )
					{
						StartTimer(ROLE_TOKEN_OWNER_TOUT,ROLE_TOKEN_OWNER_TIME*SECOND);
						str << " - Ack for TokenRequest received, Name - " << m_szEpName ;
					}
					// ack for release_token
					else if( kPresentationTokenRelease == m_enRoleTokenLastCmd )
					{
						str << " - Ack for TokenRelease received, Name - " << m_szEpName ;
					}
				}
				else if(ack_nak==RejectParamIdent)
				{
					str << " - NACK for TokenRequest received, Name - " << m_szEpName ;
				}
				else
				{
					str << " **** UNKNOWN ack BYTE: " << ack_nak << " **** ";
				}
				m_enRoleTokenLastCmd = kUnknownRoleTokenOpcode;
				break;
			}
			case PresentationTokenIndicateOwner:
			{
				BYTE tempSize = 0;
				BYTE* pData = NULL;
				terminalLabel = MbeBytesToInt(pParam);
				channelId = MbeBytesToInt(pParam);
				terminalNumber = terminalLabel%TERMINAL_LABEL_MULTIPLIER;
				MCUNumber = (terminalLabel-terminalNumber)/TERMINAL_LABEL_MULTIPLIER;
				str << "PresentationTokenIndicateOwner: Channel Id: "<< channelId<<" MCUNumber: "<<MCUNumber<<" terminalNumber: "<<terminalNumber << "\n"; 
				DBGPASSERT(terminalNumber == m_nTerminalId);
				break;
			}
			case FlowControlReleaseRequest:
			{
				channelId = MbeBytesToInt(pParam);
				bitRate = MbeBytesToInt(pParam);
				str << "FlowControlReleaseRequest: Channel Id: "<< channelId<< " bitRate: "<<bitRate; 
				break;
			}
			case FlowControlReleaseResponse:
			{
				str << "FlowControlReleaseRequest: DO NOTHING!";
				break;
			}
			case PresentationTokenRequest: //terminal label, channel id, symb
			{
				terminalLabel = MbeBytesToInt(pParam);
				channelId = MbeBytesToInt(pParam);
				symmetryBreaking = MbeBytesToInt(pParam);
				terminalNumber = terminalLabel%TERMINAL_LABEL_MULTIPLIER;
				MCUNumber = (terminalLabel-terminalNumber)/TERMINAL_LABEL_MULTIPLIER;
				DBGPASSERT(MCUNumber != m_nMcuId);
				DBGPASSERT(terminalNumber != m_nTerminalId);
				DeleteTimer(ROLE_TOKEN_OWNER_TOUT);
				str << "PresentationTokenRequest: Channel Id: "<< channelId<< " symmetryBreaking: "<<symmetryBreaking<<" MCUNumber: "<<MCUNumber<<" terminalNumber: "<<terminalNumber << "\n"; 
				str << "Withdraw for TokenRequest received, Name - " <<  m_szEpName;
				PTRACE(eLevelInfoNormal,str.GetString());
				traceAtEnd = FALSE;
				SendH239TokenWithdrawResponse();
				break;                      
			}
			case PresentationTokenRelease:
			{	
				terminalLabel = MbeBytesToInt(pParam);
				channelId = MbeBytesToInt(pParam);
				terminalNumber = terminalLabel%TERMINAL_LABEL_MULTIPLIER;
				MCUNumber = (terminalLabel-terminalNumber)/TERMINAL_LABEL_MULTIPLIER;
				str << "PresentationTokenRelease: Channel Id: "<< channelId<<" MCUNumber: "<<MCUNumber<<" terminalNumber: "<<terminalNumber; 
				break;
			}
			default:
			{
				str	<< " ******* - Unknown Token request received, subOpcode <"<< subMsgIdentifier	<< ">, EpName - " << m_szEpName <<" *******";
				break;
			}
		}//switch
		if (traceAtEnd == TRUE)
			PTRACE(eLevelInfoNormal,str.GetString());
	}//if

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::OnRoleTokenOwnerTout(CSegment* pParam)
{
	SendH239Message(PresentationTokenIndicateOwner);
	StartTimer(ROLE_TOKEN_OWNER_TOUT,ROLE_TOKEN_OWNER_TIME*SECOND);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendH239TokenWithdrawResponse()
{
	SendH239Message(PresentationTokenResponse, AcknowledgeParamIdent);
	m_enRoleTokenLastCmd = kUnknownRoleTokenOpcode;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEndpointISDN::SendH239Message(BYTE SubMessageOpcode, BYTE isAckNack,BYTE addRandNum, BYTE randNum)
{
	CSegment H320StringToSend;
	BYTE msgLength;
	
	BYTE mcuNum = (BYTE)m_nMcuId;
	BYTE terminalNum = (BYTE)m_nTerminalId;
	
	CSegment*  seg = new CSegment;
    *seg    << (BYTE)(Start_Mbe | ESCAPECAPATTR);
	
	CSegment* segTemp = new CSegment;
	*segTemp << (BYTE)H239_messsage;
	*segTemp << (BYTE) SubMessageOpcode;
	if (isAckNack)
		SerializeGenericParameter(isAckNack,0,segTemp);
	SerializeGenericParameter(TerminalLabelParamIdent,((mcuNum*TERMINAL_LABEL_MULTIPLIER)+terminalNum),segTemp);
	SerializeGenericParameter(ChannelIdParamIdent,SecondVideoChannel,segTemp);
	if (addRandNum)
		SerializeGenericParameter(SymmetryBreakingParamIdent,randNum,segTemp);
	
	msgLength = (BYTE)segTemp->GetWrtOffset();	
	*seg << msgLength;
	*seg << *segTemp;

	PrepareH230(*seg,H320StringToSend);
	
	POBJDELETE(seg);
	POBJDELETE(segTemp);
	
	CSegment rMsgSeg;
	PrepareMuxMessageSeg(rMsgSeg, REMOTE_CI, H320StringToSend.GetPtr(),H320StringToSend.GetWrtOffset());
	::SendMuxMessageToGideonSimApp(rMsgSeg);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
//
#define    _MCC      Cancel_MCC
#define    _MIZ      Cancel_MIZ
#define    _MIS      Cancel_MIS
#define    _MCV      Cancel_MCV
#define    _MIV      Cancel_MIV
#define    _VCS      Cancel_VCS
#define    _MIM      Cancel_MIM

BEGIN_H230_TBL
	/* THE FOLLOWING 230 OPCODES ARE CURRENTLY (01/2009) BEEN SENT FROM RMX TO END POINT 
	 *  If you are adding a new feture for H.320 party -> implement the correspond H230 function from the table below!!!*/
 			// opcode related to audio
  ONH230(AIM | Attr000,  CEndpointISDN::OnH230AudioMute                   ,"AIM",  "Audio indicate muted")
  ONH230(AIA | Attr000,  CEndpointISDN::OnH230AudioActive                 ,"AIA",  "Audio indicate active")
  			// opcode related to video 
  ONH230(VIS | Attr000,  CEndpointISDN::OnH230VideoIndSuppressed          ,"VIS",  "Video indicate suppressed")
  ONH230(VIA | Attr000,  CEndpointISDN::OnH230VideoActive                 ,"VIA",  "Video indicate active")
  			// opcode related to chair control
  ONH230(TIA | Attr001,  CEndpointISDN::OnH230AssignTerminaNum            ,"TIA",  "Assign terminal number")
  ONH230(TCS_2 | Attr011,  CEndpointISDN::OnH230TerminalIdentity          ,"TCS_2","Terminal string identity")                    

END__H230_TBL

/*
BEGIN_H230_TBL - THE FULL H230 TABLE

                    // opcode related to audio
  ONH230(AIM | Attr000,  (AFUNC)CEndpointISDN::OnH230AudioMute                   ,"AIM",  "Audio indicate muted")
  ONH230(AIA | Attr000,  (AFUNC)CEndpointISDN::OnH230AudioActive                 ,"AIA",  "Audio indicate active")
  ONH230(ACE | Attr000,  (AFUNC)CEndpointISDN::OnH230AudioEqualize               ,"ACE",  "Audio command equalize")
  ONH230(ACZ | Attr000,  (AFUNC)CEndpointISDN::OnH230AudioZeroDelay              ,"ACZ",  "Audio zero delay")
                    // opcode related to video 
  ONH230(VIS | Attr000,  (AFUNC)CEndpointISDN::OnH230VideoIndSuppressed          ,"VIS",  "Video indicate suppressed")
  ONH230(VIA | Attr000,  (AFUNC)CEndpointISDN::OnH230VideoActive                 ,"VIA",  "Video indicate active")
                    // opcode related to chair control
  ONH230(CIC | Attr010,  (AFUNC)CEndpointISDN::OnH230ChairCntlCap                ,"CIC",  "Chair control capbility")                    
  ONH230(CCA | Attr010,  (AFUNC)CEndpointISDN::OnH230ChairCmdAcquire             ,"CCA",  "Chair token acquire")                    
  ONH230(CIS | Attr010,  (AFUNC)CEndpointISDN::OnH230ChairStopToken              ,"CIS",  "Chair token release")                    
  ONH230(CIR | Attr010,  (AFUNC)CEndpointISDN::OnH230ChairIndRefuse              ,"CIR",  "Chair indicate releas/refuse terminal drop")                    
  ONH230(CIT | Attr010,  (AFUNC)CEndpointISDN::OnH230ChairIndToken               ,"CIT",  "Chair indicate token")                    
  ONH230(CCR | Attr010,  (AFUNC)CEndpointISDN::OnH230ChairReleaseToken           ,"CCR",  "Chair token releas/refuse")                    
  ONH230(CCD | Attr010,  (AFUNC)CEndpointISDN::OnH230ChairDropTerm               ,"CCD",  "Chair drop terminal")                    
  ONH230(CCK | Attr010,  (AFUNC)CEndpointISDN::OnH230ChairDropConf               ,"CCK",  "Chair drop conference") 
  ONH230(TIF | Attr010,  (AFUNC)CEndpointISDN::OnH230FloorReq                    ,"TIF",  "OnH230FloorReq")        
  ONH230(TIE | Attr010,  (AFUNC)CEndpointISDN::OnH230IndicateEndOfString         ,"TIE",  "OnH230IndicateEndOfString") 
  //ONH230(TCU | Attr001,  (AFUNC)CEndpointISDN::NullActorReq                    ,"TIF",  "Request for floor")
  ONH230(TCU | Attr001,  (AFUNC)CEndpointISDN::OnH230UpdateTerminalList          ,"TCU",  "Update terminal list")                        
  ONH230(TIA | Attr001,  (AFUNC)CEndpointISDN::OnH230AssignTerminaNum            ,"TIA",  "Assign terminal number")
  ONH230(TIN | Attr001,  (AFUNC)CEndpointISDN::OnH230IndTerminaNum               ,"TID",  "Indicate terminal number")    
  ONH230(TID | Attr001,  (AFUNC)CEndpointISDN::OnH230DropTerminaNum              ,"TID",  "Dropped  terminal number") 
                         
  ONH230(VCB | Attr001,  (AFUNC)CEndpointISDN::OnH230ChairVideoBroadcast         ,"VCB",  "Chair video command broadcast")
  ONH230(VCE | Attr001,  (AFUNC)CEndpointISDN::OnH230ChairCancelVideoBroadcast   ,"VCE",  "Chair video cancel command broadcast")
  ONH230(TCA | Attr001,  (AFUNC)CEndpointISDN::OnH230TokenCommandAssociation         ,"TCA",  "OnH230CommandAssociation")
    // cancelVCB ?????                    
                    // opcode related to data control
  ONH230(DCA_L | Attr010,  (AFUNC)CEndpointISDN::OnH230LsdAcquireToken           ,"DCA_L",  "Lsd acquire token")                    
  ONH230(DCA_H | Attr010,  (AFUNC)CEndpointISDN::OnH230HsdAcquireToken           ,"DCA_H",  "Hsd acquire token")                    
  ONH230(DIT_L | Attr010,  (AFUNC)CEndpointISDN::OnH230LsdIndicateToken          ,"DIT_L",  "Lsd indicate token")                    
  ONH230(DIT_H | Attr010,  (AFUNC)CEndpointISDN::OnH230HsdIndicateToken          ,"DIT_H",  "Hsd indicate token")                    
  ONH230(DIS_L | Attr010,  (AFUNC)CEndpointISDN::OnH230LsdStopToken              ,"DIS_L",  "Lsd stop using token")                    
  ONH230(DIS_H | Attr010,  (AFUNC)CEndpointISDN::OnH230HsdStopToken              ,"DIS_H",  "Hsd stop using token")                    
  ONH230(DCR_L | Attr010,  (AFUNC)CEndpointISDN::OnH230LsdReleaseToken           ,"DCR_L",  "Lsd releas/refuse token")                    
  ONH230(DCR_H | Attr010,  (AFUNC)CEndpointISDN::OnH230HsdReleaseToken           ,"DCR_H",  "Hsd releas/refuse token")                    
  ONH230(DCC_L | Attr010,  (AFUNC)CEndpointISDN::OnH230LsdCloseChannel           ,"DCC_L",  "Lsd close channel")                    
                    // opcode related to terminal identification
  ONH230(TCI   | Attr000,  (AFUNC)CEndpointISDN::OnH230TerminalIdentify          ,"TCI",  "Terminal command identify")                    
  ONH230(TII   | Attr000,  (AFUNC)CEndpointISDN::OnH230TerminalIndIdentity       ,"TII",  "Terminal indicate identity")                    
  ONH230(TIS   | Attr000,  (AFUNC)CEndpointISDN::OnH230TerminalIndIdentityStop   ,"TIS",  "Terminal indicate identity stop")                    
  ONH230(TCS_0 | Attr011,  (AFUNC)CEndpointISDN::NullActionFunction              ,"TCS_0","Terminal string reservrd")                    
  ONH230(TCS_1 | Attr011,  (AFUNC)CEndpointISDN::OnH230TerminalPassword          ,"TCS_1","Terminal string password")                    
  ONH230(TCS_2 | Attr011,  (AFUNC)CEndpointISDN::OnH230TerminalIdentity          ,"TCS_2","Terminal string identity")                    
  ONH230(TCS_3 | Attr011,  (AFUNC)CEndpointISDN::OnH230TerminalConfIdentify      ,"TCS_3","Terminal string conf identify")
  ONH230(TCP_  | Attr011,  (AFUNC)CEndpointISDN::OnH230TerminalPersonaldentify   ,"TCP_", "Terminal personal identify")                      
  ONH230(TCS_4 | Attr011,  (AFUNC)CEndpointISDN::OnH230TerminalExtensionAddress  ,"TCS_4","Terminal extension address")                              
                    // opcode related to broudcast control - CURRENTLY DISABLED
  
  ONH230(MCV  | Attr001,  (AFUNC)CEndpointISDN::OnH230MultiCmdVisualize          ,"MCV" ,"Multipoint Visualize Force")
  ONH230(_MCV | Attr001,  (AFUNC)CEndpointISDN::OnH230MultiCancelCmdVisualize    ,"_MCV","Cancel Multipoint Visualize Force")
  ONH230(MIV  | Attr001,  (AFUNC)CEndpointISDN::OnH230MultiIndVisualize          ,"MIV" ,"Multipoint Visualize Indication")
  ONH230(_MIV | Attr001,  (AFUNC)CEndpointISDN::OnH230MultiCancelIndVisualize    ,"_MIV","Cancel Multipoint Visualize Indication") 
 
  ONH230(MIV  | Attr001,  (AFUNC)CEndpointISDN::NullActionFunction    ,"MIV" ,"Multipoint Visualize Indication")
  ONH230(_MIV | Attr001,  (AFUNC)CEndpointISDN::NullActionFunction    ,"_MIV","Cancel Multipoint Visualize Indication")

  ONH230(MVA  | Attr010,  (AFUNC)CEndpointISDN::NullActionFunction               ,"MVA" ,"Multipoint Visualization Achieved")
  ONH230(MVR  | Attr010,  (AFUNC)CEndpointISDN::NullActionFunction               ,"MVR" ,"Multipoint Visualization Refused/Revoked")

                          // opcode related to select control - CURRENTLY DISABLED
  ONH230(VCS  | Attr001,  (AFUNC)CEndpointISDN::OnH230VideoCmdSelect           ,"VCS" ,"Multipoint Visualize Force")
  ONH230(_VCS | Attr001,  (AFUNC)CEndpointISDN::OnH230CancelVideoCmdSelect     ,"_VCS","Cancel Multipoint Visualize Force")
  
  //ONH230(VCR  | Attr001,  (AFUNC)CEndpointISDN::OnH230RejectForcedVideo           ,"VCR","Reject video enforce")                    
  
  ONH230(VCR  | Attr001,  (AFUNC)CEndpointISDN::NullActionFunction    ,"VCR","Reject video enforce")                    

                   // opcode related to cascading
  ONH230(MIL  | Attr001,  (AFUNC)CEndpointISDN::OnH230MultiIndLopp               ,"MIL" ,"Multipoint indicate loop")
  ONH230(MIM  | Attr001,  (AFUNC)CEndpointISDN::OnH230MultiIndMaster             ,"MIM" ,"Multipoint indicate master")
  ONH230(RAN  | Attr001,  (AFUNC)CEndpointISDN::OnH230RandomNum                  ,"RAN" ,"Random number")
                    // opcode related to other
  ONH230(VIN | Attr001,  (AFUNC)CEndpointISDN::OnH230IndVideoSrcNum              ,"VIN","Indicate video src number")                                         
  ONH230(MIZ | Attr001,  (AFUNC)CEndpointISDN::OnH230ZeroCom                     ,"MIZ","Multipoint zero communication")                    
  ONH230(MIS | Attr001,  (AFUNC)CEndpointISDN::OnH230SecondaryStatus             ,"MIS","Multipoint secondery status") 
                     
  ONH230(_MCC | Attr001,  (AFUNC)CEndpointISDN::NullActionFunction               ,"Cancel_MCC","Cancel multipoint command conference")                    
  ONH230(_MIZ | Attr001,  (AFUNC)CEndpointISDN::OnH230CancelZeroCom              ,"Cancel_MIZ","Cancel multipoint command symmetrical")                    
  ONH230(_MIS | Attr001,  (AFUNC)CEndpointISDN::OnH230CancelSecondaryStatus      ,"Cancel_MIS","Cancel multipoint secondery status")                    
  ONH230(_MIM | Attr001,  (AFUNC)CEndpointISDN::OnH230CancelMultiIndMaster       ,"Cancel_MIM","Cancel Multipoint indicate master")

  ONH230(DCM  | Attr010,  (AFUNC)CEndpointISDN::OnH230MlpData              ,"DCM","Data command MLP")                    
                                      // probly will be send in exchange cap sequence
  ONH230(MCS | Attr001,  (AFUNC)CEndpointISDN::OnH230MultiCmdSymetric            ,"MCS","Multipoint command symmetrical")                    
  ONH230(MCC | Attr001,  (AFUNC)CEndpointISDN::OnH230MultiCmdConf                ,"MCC","Multipoint command conference") 
  ONH230(MIH | Attr001,  (AFUNC)CEndpointISDN::OnH230MultiIndicateHierarchy	  ,"MIH","Multipoint Indicate Hierarchy")
                    
END__H230_TBL
*/

/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnH230AudioMute(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CEndpointISDN::OnH230AudioMute (RMX is MUTED): Name - ",m_szEpName);
  
}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnH230AudioActive(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CEndpointISDN::OnH230AudioActive (RMX is UNMUTED): Name - ",m_szEpName);
  
}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnH230VideoIndSuppressed(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CEndpointISDN::OnH230VideoIndSuppressed (RMX VIDEO IS NOT VALID!!!): Name - ",m_szEpName);
  }

/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnH230VideoActive(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CEndpointISDN::OnH230VideoActive RMX VIDEO IS OK!!!: Name - ",m_szEpName);
  
}


/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnH230AssignTerminaNum(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CEndpointISDN::OnH230AssignTerminaNum : Name - ",m_szEpName);
  BYTE  mcu,terminal;
  GetMcuTerminalNum(pParam,mcu,terminal);
  m_nMcuId = mcu;
  m_nTerminalId = terminal;
       
}
/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::OnH230TerminalIdentity(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CEndpointISDN::OnH230TerminalIdentity : Name - ",m_szEpName);
  
  CSegment seg;	
  BYTE length = strlen(m_szEpName);

  length+=2;
  
  seg << (BYTE)(Start_Mbe | ESCAPECAPATTR)
  	  << length 
	  << (BYTE)IIS  // opcode
	  << (BYTE) 2;  // Value 2 corresponds to the TCS_2 message
	 
  seg.Put((unsigned char*)m_szEpName, length-2); 
  
  SendH230(&seg);
}

/////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::GetMcuTerminalNum(CSegment* pParam,BYTE& mcu,BYTE& terminal)
{
	BYTE  sbe;
	if(!pParam->EndOfSegment())
	{
		*pParam >> sbe;
		if ( sbe != (H230_Sbe_Esc | ESCAPECAPATTR))
		{
			PTRACE(eLevelError,"CEndpointISDN::GetMcuTerminalNum : invalid mcu sbe");
			return;
		}
		else
		{
			*pParam >> mcu ;
		}
		
		*pParam >> sbe ;
		if ( sbe != (H230_Sbe_Esc | ESCAPECAPATTR)) 
		{
			PTRACE(eLevelError,"CEndpointISDN::GetMcuTerminalNum : invalid terminal sbe");
			return;
		}
		else
		{
			*pParam >> terminal;
		}
	}
	else
		PTRACE(eLevelError,"CEndpointISDN::GetMcuTerminalNum :  Terminal did not send SBE information( mcu  & terminal numbers)");
}
///////////////////////////////////////////////////////////////////////////////////////////
void  CEndpointISDN::SendH230(CSegment* pParam)
{
	CSegment  H230StringToSend;
		 
	PrepareH230(*pParam,H230StringToSend);
	 
	CSegment rMsgSeg;
	PrepareMuxMessageSeg(rMsgSeg, REMOTE_CI, H230StringToSend.GetPtr(),H230StringToSend.GetWrtOffset());
	::SendMuxMessageToGideonSimApp(rMsgSeg); 
	  
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////	CIsdnChannel
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CIsdnChannel::CIsdnChannel()
{
	memset(m_szPhoneNum, 0, BND_MAX_PHONE_LEN+1);		
	m_connectionState = eEpStateIdle;
	m_connectionId = (DWORD)(-1);
	m_netConnectionId = (DWORD)(-1);
}


/////////////////////////////////////////////////////////////////////////////
CIsdnChannel::~CIsdnChannel()     // destructor
{
}


/////////////////////////////////////////////////////////////////////////////
void CIsdnChannel::Serialize( CSegment& rSegment ) const
{
	rSegment << m_szPhoneNum;
	rSegment << (WORD)m_connectionState;
}


/////////////////////////////////////////////////////////////////////////////
void CIsdnChannel::SetPhoneNum( const char* pszPhoneNum )
{
	strncpy(m_szPhoneNum, pszPhoneNum, BND_MAX_PHONE_LEN);
	m_szPhoneNum[BND_MAX_PHONE_LEN] = '\0';
}


/////////////////////////////////////////////////////////////////////////////
void CIsdnChannel::SetPhoneNumDigit( const char* pszPhoneNum )
{
    for(int digit_index = 0; digit_index < BND_MAX_PHONE_LEN; digit_index++)
    {
    	m_szPhoneNumDigit[digit_index] = pszPhoneNum[digit_index];
    }
    
	m_szPhoneNumDigit[BND_MAX_PHONE_LEN] = '\0';
}


/////////////////////////////////////////////////////////////////////////////
void CIsdnChannel::SetParams(DWORD connectionId, DWORD boardId, DWORD subBoardId, DWORD spanId, DWORD portId)
{
	m_connectionId = connectionId;
	m_rRtmBoardDetails.SetBoardDetails(boardId, subBoardId, spanId, portId);
///	m_rRtmBoardDetails.SetRtmSpanId(spanId);
///	m_rRtmBoardDetails.SetRtmPortId(portId);
}


/////////////////////////////////////////////////////////////////////////////
void CIsdnChannel::SetState(const enEndpointState state)
{
	m_connectionState = state;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIsdnChannel::GetConnectionId() const
{
	return m_connectionId;
}

/////////////////////////////////////////////////////////////////////////////
void CIsdnChannel::SetConnectionId(const DWORD connectionId)
{
	m_connectionId = connectionId;
}

/////////////////////////////////////////////////////////////////////////////
enEndpointState CIsdnChannel::GetState() const
{
	return m_connectionState;
}

/////////////////////////////////////////////////////////////////////////////
void CIsdnChannel::SetNetConnectionId(const DWORD netConnectionId)
{
	m_netConnectionId = netConnectionId;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CIsdnChannel::GetNetConnectionId() const
{
	return m_netConnectionId;
}

/////////////////////////////////////////////////////////////////////////////
void CIsdnChannel::SetBoardDetails(const WORD boardId, const WORD subBoardId, const WORD spanId, const WORD portId)
{
	m_rRtmBoardDetails.SetBoardId(boardId);
	m_rRtmBoardDetails.SetSubBoardId(subBoardId);
	m_rRtmBoardDetails.SetRtmSpanId(spanId);
	m_rRtmBoardDetails.SetRtmPortId(portId);
}

/////////////////////////////////////////////////////////////////////////////
CRtmBoardDetails CIsdnChannel::GetRtmBoardDetails() const
{
	return m_rRtmBoardDetails;
}
