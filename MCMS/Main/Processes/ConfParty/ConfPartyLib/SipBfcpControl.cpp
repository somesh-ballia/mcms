/*
 * TipRtcpControl.cpp
 *
 *  Created on: Feb 15, 2011
 *      Author: shmuell
 */

//#include "ConfPartyOpcodes.h"
#include "OpcodesRanges.h"

#include "Trace.h"
#include "TraceHeader.h"
#include "ObjString.h"
#include "ConfPartyOpcodes.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "SIPControl.h"
#include "SipBfcpControl.h"


#define Max_T1_Retransmission		3

const WORD BFCP_RETRANSMISSION_T1 = 1; // BFCP send retransmission time out is 0.5 sec, so we set it to 1 second in MCMS

PBEGIN_MESSAGE_MAP(CSipBfcpCtrl)
// BFCP
ONEVENT(IP_CM_BFCP_MESSAGE_IND,			sBFCP_HELLO_KEEP_ALIVE_REQUEST, 		CSipBfcpCtrl::OnIpCmBfcpMessageInd)
ONEVENT(IP_CM_BFCP_MESSAGE_IND,			IDLE, 									CSipBfcpCtrl::OnIpCmBfcpMessageInd)
ONEVENT(IP_CM_BFCP_MESSAGE_REQ,			IDLE,									CSipBfcpCtrl::OnSendBfcpMessageReq)
ONEVENT(IP_CM_BFCP_MESSAGE_IND,			sBFCP_READY,							CSipBfcpCtrl::OnIpCmBfcpMessageInd)
ONEVENT(IP_CM_BFCP_MESSAGE_REQ,			sBFCP_MESSAGE_RECEIVED_REQUEST,			CSipBfcpCtrl::OnSendBfcpMessageReq)
ONEVENT(IP_CM_BFCP_MESSAGE_REQ,			sBFCP_MESSAGE_SENT_REQUEST,				CSipBfcpCtrl::OnSendBfcpMessageReq)
ONEVENT(IP_CM_BFCP_MESSAGE_IND,			sBFCP_MESSAGE_RECEIVED_REQUEST,			CSipBfcpCtrl::OnIpCmBfcpMessageInd)

ONEVENT(IP_CM_BFCP_MESSAGE_REQ,			sBFCP_READY,							CSipBfcpCtrl::OnSendBfcpMessageReq)
ONEVENT(IP_CM_BFCP_MESSAGE_IND,			sBFCP_MESSAGE_SENT_REQUEST,				CSipBfcpCtrl::OnIpCmBfcpMessageInd)

ONEVENT(BFCP_MESSAGE_REQUEST_TOUT,		sBFCP_MESSAGE_SENT_REQUEST,				CSipBfcpCtrl::OnIpCmBfcpMessageSentTout)
ONEVENT(BFCP_MESSAGE_REQUEST_TOUT,		sBFCP_MESSAGE_RETRANSMIT,				CSipBfcpCtrl::OnIpCmBfcpMessageSentTout)
ONEVENT(IP_CM_BFCP_MESSAGE_IND,			sBFCP_MESSAGE_RETRANSMIT,				CSipBfcpCtrl::OnIpCmBfcpMessageInd)
ONEVENT(IP_CM_BFCP_MESSAGE_REQ,			sBFCP_MESSAGE_RETRANSMIT,				CSipBfcpCtrl::OnSendBfcpMessageReq)
ONEVENT(BFCP_HELLO_KEEP_ALIVE,          ANYCASE,                                CSipBfcpCtrl::OnKeepAlive)


PEND_MESSAGE_MAP(CSipBfcpCtrl,CStateMachine);

////////////////////////////////////////////////////////////////////////////
CSipBfcpCtrl::CSipBfcpCtrl(CTaskApp *pOwnerTask, eSipBfcpMode4DialOut mode) : CStateMachine(pOwnerTask)
{
	m_pMfaInterface 			= NULL;
	m_pPartyApi 				= NULL;
    m_mcuNum 					= 0;
    m_termNum 					= 0;
    m_bWaitForWithdrawAck 		= FALSE;
    m_bIsRequestSent 			= FALSE;

    m_DialOutMode = mode;
    m_connMode = (m_DialOutMode == BFCP_TCP_ONLY) ? eTcpPassive : eUdp;
    m_setupMode                 = kSetupPassive; //kSetupActPass

    m_pTranslator				= NULL;
    m_lastFloorRequestID 		= 1;

    m_msgRetransmitReqCounter	= 0;
    m_bIsWaitingForAck			= FALSE;

    m_lastTransactionType		= 0;
    m_lastTransactionIdReq		= 0;

    m_transactionInitiator		= kBFCPFloorCtrlInvalid;

    m_state						= IDLE;

    m_pBfcpReq					= NULL;

	VALIDATEMESSAGEMAP;
}
////////////////////////////////////////////////////////////////////////////
CSipBfcpCtrl::~CSipBfcpCtrl()
{
	if (m_pBfcpReq)
		PDELETEA(m_pBfcpReq);

	if (m_pTranslator)
		PDELETEA(m_pTranslator);
}

////////////////////////////////////////////////////////////////////////////
void  CSipBfcpCtrl::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

////////////////////////////////////////////////////////////////////////////
void CSipBfcpCtrl::Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi, WORD mcuNum, WORD termNum,
		enTransportType transType, RemoteIdent remoteIdent)
{
	m_pMfaInterface 	= pMfaInterface;
	m_pPartyApi 		= pPartyApi;
    m_mcuNum 			= mcuNum;
    m_termNum 			= termNum;

    m_bfcpTransportType = transType;
    m_bfcpRemoteIdent	= remoteIdent;

    if (m_bfcpTransportType == eTransportTypeTcp)
    	m_state = sBFCP_READY;

    m_pTranslator = (BFCPH239Translator *) new char[sizeof(BFCPH239Translator)];

    if (m_pTranslator)
    {
    	if (statusOK != InitializeTranslatorDefaults(m_pTranslator, atoi(BFCP_FLOOR_ID_PPC), kBFCPPriorityNormal,
    			m_mcuNum, m_termNum, kBFCPFloorCtrlServer))
    	{

    		PTRACE(eLevelInfoNormal, "CSipBfcpCtrl::Initialize - fail to init m_pTranslator");

    		PDELETEA(m_pTranslator);
    	}
    }
}

////////////////////////////////////////////////////////////////////////////
void CSipBfcpCtrl::UpdateParams(enTransportType transType, RemoteIdent remoteIdent)
{
	m_bfcpTransportType	= transType;
	m_bfcpRemoteIdent	= remoteIdent;

	if (m_bfcpTransportType == eTransportTypeTcp)
	   	m_state = sBFCP_READY;
}


////////////////////////////////////////////////////////////////////////////
DWORD CSipBfcpCtrl::SendMsgToMpl(BYTE* pStructure, int structureSize, DWORD opcode)
{
	if (!IsValidPObjectPtr(m_pMfaInterface))
	{
		DBGPASSERT(1);
		return 0;
	}

	CSegment* pMsg = NULL;
	if (pStructure)
	{
		pMsg = new CSegment;
		pMsg->Put(pStructure ,structureSize);
	}

	DWORD seqNum = 0;
	seqNum = m_pMfaInterface->SendMsgToMPL(opcode,pMsg);

	POBJDELETE(pMsg);
	return seqNum;
}

////////////////////////////////////////////////////////////////////////////
enTransportType CSipBfcpCtrl::GetBfcpTransportType()
{
	return m_bfcpTransportType;
}

////////////////////////////////////////////////////////////////////////////
void CSipBfcpCtrl::SetBfcpConnMode(eBFCPSetup remoteSetup)
{
	if (m_bfcpTransportType == eTransportTypeUdp)
	{
		m_connMode 	= eUdp;

		// comments are for the future if RMX will be slave (e.g. cascade)
//		if ((remoteSetup == kSetupActive) || (remoteSetup == kSetupActPass))
			m_setupMode	= kSetupPassive;
//		else if (remoteSetup == kSetupPassive)
//			m_setupMode	= kSetupActive;
	}
	else
	{
		if (remoteSetup == kSetupActive)
		{
			m_setupMode	= kSetupPassive;
			m_connMode 	= eTcpPassive;
		}
		else if ((remoteSetup == kSetupPassive) || (remoteSetup == kSetupActPass))
		{
			m_setupMode	= kSetupActive;
			m_connMode 	= eTcpActive;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
eBFCPSetup CSipBfcpCtrl::GetBfcpSetupMode()
{
	return m_setupMode;
}

////////////////////////////////////////////////////////////////////////////
eConnectionTransportMode CSipBfcpCtrl::GetBfcpConnMode()
{
	return m_connMode;
}

////////////////////////////////////////////////////////////////////////////
void CSipBfcpCtrl::OnSendBfcpMessageReq(CSegment *pParam)
{
	int 	status;
	DWORD 	opcode;
	BYTE 	mcuId;
	BYTE	terminalId;

	*pParam >> opcode;
	*pParam >> mcuId;
	*pParam >> terminalId;

	CMedString str;

	if (!m_pTranslator)
	{
		PTRACE(eLevelInfoNormal, "CSipBfcpCtrl::OnSendBfcpMessageReq - m_pTranslator is NULL");
		return;
	}

	if ((m_bfcpTransportType == eTransportTypeUdp && m_bfcpRemoteIdent != TandbergEp) &&
			(CONTENT_ROLE_BFCP_HELLO_ACK != opcode)&&
		(m_state == sBFCP_MESSAGE_SENT_REQUEST || m_state == sBFCP_MESSAGE_RETRANSMIT))
	{
		str << "opcode: " << opcode << ", mcuId: " << mcuId << ", terminalId: " << terminalId << ", state: " << m_state;
		PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::OnSendBfcpMessageReq - Can't send message. UDP/BFCP is waiting for remote response: ", str.GetString());
		str.Clear();
		return;
	}

	if (m_bfcpTransportType == eTransportTypeTcp)
		status = HandleTcpBfcpReq(opcode);
	else
		status = HandleUdpBfcpReq(opcode);
}

////////////////////////////////////////////////////////////////////////////
void CSipBfcpCtrl::OnIpCmBfcpMessageSentTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipBfcpCtrl::OnIpCmBfcpMessageSentTout");

	if (IsValidTimer(BFCP_MESSAGE_REQUEST_TOUT))
		DeleteTimer(BFCP_MESSAGE_REQUEST_TOUT);

	//BRIDGE-9948: the retransmition number is configurable [1, 7]
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	DWORD  bfcpReTxTimes =0;
	sysConfig->GetDWORDDataByKey(CFG_KEY_BFCP_REQUEST_RETRY_TIMES, bfcpReTxTimes);


	if (m_msgRetransmitReqCounter == static_cast<int> (bfcpReTxTimes))
	{
		// need to reestablish connection
		if (m_pBfcpReq)
			PDELETEA(m_pBfcpReq);

		PTRACE2INT(eLevelInfoNormal, "CSipBfcpCtrl::OnIpCmBfcpMessageSentTout - num of retransmit: ", m_msgRetransmitReqCounter);
		m_state = sBFCP_READY;

		m_msgRetransmitReqCounter = 0;

		m_keepAliveIterationsCount = 0; //no Hello should be sent when the channels are closed

		m_pPartyApi->BfcpStartReestablishConnection();

		return;
	}

	m_state = sBFCP_MESSAGE_RETRANSMIT;

	size_t size = sizeof(mcReqBfcpMessage) + m_pBfcpReq->length;

	m_msgRetransmitReqCounter += 1;

	int timeout =0;
	if(m_msgRetransmitReqCounter <= Max_T1_Retransmission)
	{
		 timeout = m_msgRetransmitReqCounter * BFCP_RETRANSMISSION_T1;
	}
	else
	{
		 timeout = Max_T1_Retransmission * BFCP_RETRANSMISSION_T1; 
		 // The rest retransmition timer will keep the same with the last one (Max_T1_Retransmission)
	}
	StartTimer(BFCP_MESSAGE_REQUEST_TOUT,  timeout * SECOND);

	SendMsgToMpl((BYTE*) m_pBfcpReq, (int) size, IP_CM_BFCP_MESSAGE_REQ);
}

////////////////////////////////////////////////////////////////////////////
void CSipBfcpCtrl::OnIpCmBfcpMessageInd(CSegment* pParam)
{
	APIU32 	status 			= STATUS_OK;
	APIS32 	opCode			= 0;

	BYTE 	mcuId 			= 0;
	BYTE 	terminalId 		= 0;
	
	if (!m_pTranslator)
	{
		PTRACE(eLevelInfoNormal, "CSipBfcpCtrl::OnIpCmBfcpMessageInd - m_pTranslator is NULL");
		return;
	}

	//Now let's decode the message to a struct, remember that data must be allocated in advance:
	BFCPFloorInfoT  BFCPmsg;
	memset(&BFCPmsg, 0, sizeof(BFCPFloorInfoT));

	mcIndBfcpMessage* pMsg = (mcIndBfcpMessage *)pParam->GetPtr(1);

	if (!pMsg)
	{
		PTRACE(eLevelInfoNormal, "CSipBfcpCtrl::OnIpCmBfcpMessageInd - pMsg is NULL");
		return;
	}

	CMedString 	str;

	str << "m_state: " << m_state << ", status:" << pMsg->status << ", length:" << pMsg->length;

	PTRACE2(eLevelInfoNormal,"CSipBfcpCtrl::OnIpCmBfcpMessageInd ", str.GetString());

	if (pMsg->length > 0 && pMsg->length < 512)
	{
        UInt8 		tempBuf[512];
        char 		s[16];

	    memset(tempBuf, 0, 512);
	    memcpy(tempBuf, pMsg->buffer, pMsg->length);

	    str.Clear();

	    for(int i= 0; i < 56 || i < pMsg->length; i++)
	    {
	       sprintf(s, "0x%x", tempBuf[i]);
	       str << s << " ";
	    }

	    PTRACE2(eLevelInfoNormal,"CSipBfcpCtrl::OnIpCmBfcpMessageInd Raw message  ", str.GetString());

	    if (statusOK != DecodeBFCPMsg (m_pTranslator, (UInt8*)pMsg->buffer, pMsg->length, &BFCPmsg))
	    {
	    	//Error decoding!
	        PTRACE(eLevelInfoNormal,"CSipBfcpCtrl::OnIpCmBfcpMessageInd decode returned error status");
	        return;
	    }

	    if (BFCPmsg.floorID != (UINT32) BFCP_FLOOR_ID)
		{
			PTRACE2INT(eLevelInfoNormal, "CSipBfcpCtrl::OnIpCmBfcpMessageInd - need to fix client floor id. server floor id:1, client request info for floor id:", BFCPmsg.floorID);

			BFCPmsg.floorID 							= BFCP_FLOOR_ID;
			m_pTranslator->BFCPObj.floorInfo.floorID 	= BFCP_FLOOR_ID;
		}

	    str.Clear();
	    str << "\nTransactionType = " ;
	    ::GetBFCPTransactionType(BFCPmsg.transactionType, str);
	    str << "\n";
	    str << "FloorId         = " << BFCPmsg.floorID << "\n";
	    str << "conferenceID    = " << BFCPmsg.conferenceID << "\n" ;
	    str << "userID          = " << BFCPmsg.userID << "\n" ;
	    str << "transactionID   = " << BFCPmsg.transactionID << "\n" ;
	    str << "floorRequestID  = " << BFCPmsg.floorRequestID << "\n" ;
	    str << "floorStatus     = ";
	    ::GetBFCPFloorStatusType(BFCPmsg.floorStatus, str);
	    str <<"\n";
	    str << "priority        = " << BFCPmsg.priority << "\n" ;

	    PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::OnIpCmBfcpMessageInd - ", str.GetString());

	    // vngr-25037: protect BFCP from invalid message
	    if (BFCPmsg.transactionType == kBFCPInvalidPrimitive)
	    {
	    	PTRACE(eLevelInfoNormal, "CSipBfcpCtrl::OnIpCmBfcpMessageInd - invalid primitive, reestablish connection");
	    	m_pPartyApi->BfcpStartReestablishConnection();
	    	return;
	    }

	    if (BFCPmsg.transactionType == kBFCPUserQuery)
	    {
	    	PTRACE(eLevelInfoNormal, "CSipBfcpCtrl::OnIpCmBfcpMessageInd - kBFCPUserQuery - Ignore!!");
	    	return;
	    }


	    if (m_bfcpTransportType == eTransportTypeTcp)
	    	status = HandleTcpBfcpInd(&BFCPmsg, BFCPmsg.transactionType, &opCode, pMsg->status);
	    else
	    	status = HandleUdpBfcpInd(&BFCPmsg, BFCPmsg.transactionType, &opCode, pMsg->status);

	}
	else
	{
		opCode = 0;
	    //status = pMsg->status;
	    status = STATUS_FAIL; //BRIDGE-12673
		PTRACE(eLevelInfoNormal, "CSipBfcpCtrl::OnIpCmBfcpMessageInd - invalid length");
	}

	if (status == STATUS_OK)
		m_pPartyApi->SendBFCPMessageInd(opCode, status, BFCPmsg.conferenceID, BFCPmsg.userID);
}

////////////////////////////////////////////////////////////////////////////
int CSipBfcpCtrl::HandleTcpBfcpInd(BFCPFloorInfoT *pBFCPmsg, eBFCPPrimitive bfcpType, APIS32 *pOpcode, int connStatus)
{
	PTRACE2INT(eLevelInfoNormal, "CSipBfcpCtrl::HandleTcpBfcpInd, bfcpType: ", bfcpType);

	BYTE 		mcuId 		= 0;
	BYTE 		terminalId 	= 0;

	//Now try to convert it to H239 struct:
	UInt16 		floorReqID 	= 0; //The Floor Request ID will be given to us in this case from the BFCP msg

	CMedString 	str;

	//If TCP/BFCP Convert the message to SIP indication - we are not waiting for a response for anything so fkUnknownRoleTokenOpcode
	eStatus	convertStatus = BFCPToSipInd (m_pTranslator, pBFCPmsg, pOpcode, &floorReqID);


	if(pBFCPmsg)
	{
		mcuId 		= pBFCPmsg->conferenceID;
		terminalId 	= pBFCPmsg->userID;
		if(pBFCPmsg->transactionType == kBFCPError)
		{
			str.Clear();
			str << "error-code: " << pBFCPmsg->errorCode;
			PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::HandleTcpBfcpInd - BFCPError: ", str.GetString());
		}
	}
	else
	{
		PASSERTMSG(1, "pBFCPmsg is NULL"); 
	}

	str.Clear();

	str << "state: " << m_state << ", convert status: " << convertStatus << ", opcode: " << *pOpcode << ", mcuId:" << mcuId << ", terminalId: "<< terminalId;

	PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::HandleTcpBfcpInd - : ", str.GetString());

	if (convertStatus == statusError)
		return STATUS_FAIL;

	// to add error handling
//	eBFCPErrors error = kBFCPErrorNoError;

//	if (mcuId != m_mcuNum)
//		error = kBFCPErrorConfDoesNotExist;
//	if (terminalId != m_termNum)
//		error = kBFCPErrorUserDoesNotExist;
//	if (convertStatus == statusError)
//		error = kBFCPErrorUnknownPrimitive;

//	if (error != kBFCPErrorNoError)
//		m_pPartyApi->SendBFCPMessageInd(CONTENT_ROLE_BFCP_ERROR, error, mcuId, terminalId);
//	else
//	m_pPartyApi->SendBFCPMessageInd(opCode, connStatus, mcuId, terminalId);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CSipBfcpCtrl::HandleUdpBfcpInd(BFCPFloorInfoT *pBFCPmsg, eBFCPPrimitive bfcpType, APIS32 *pOpcode, int connStatus)
{
	BYTE 		mcuId 		= 0;
	BYTE 		terminalId 	= 0;

	CMedString 	str;

	PASSERT_AND_RETURN_VALUE(NULL == pBFCPmsg, STATUS_FAIL);    

	if (m_state == sBFCP_HELLO_KEEP_ALIVE_REQUEST)
	{
		if ((pBFCPmsg->transactionType == kBFCPHello) || (pBFCPmsg->transactionType == kBFCPHelloAck))
		{
			str << "keep alive counter:" << m_keepAliveIterationsCount << ", received BFCP type:" << pBFCPmsg->transactionType;
			PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::HandleUdpBfcpInd, receive indication, stop sending Hello keep alive, ", str.GetString());

			if (IsValidTimer(BFCP_HELLO_KEEP_ALIVE))
				DeleteTimer(BFCP_HELLO_KEEP_ALIVE);

			m_keepAliveIterationsCount 	= 0;
			m_state 					= IDLE;
		}
	}

	if (m_state == IDLE)
	{
		// In case of UDP client must!!! send Hello as the first message and also can sent Hello as keep alive during the call
		if (pBFCPmsg->transactionType == kBFCPHello)
		{
			m_state = sBFCP_READY;
		}
		else if (pBFCPmsg->transactionType != kBFCPHelloAck)// BFCPmsg.transactionType != kBFCPHello
		{
			PTRACE2INT(eLevelInfoNormal, "CSipBfcpCtrl::HandleUdpBfcpInd - first UDP/BFCP msg requires Hello from client. Client transaction type: ", pBFCPmsg->transactionType);
			return STATUS_FAIL;
		}
	}

	if (m_bfcpRemoteIdent != TandbergEp)
	{
		// if RMX is waiting for client response on its request
		if (m_state == sBFCP_MESSAGE_SENT_REQUEST || m_state == sBFCP_MESSAGE_RETRANSMIT)
		{
			if ((pBFCPmsg->transactionType == kBFCPFloorRequestStatusAck) ||
						(pBFCPmsg->transactionType == kBFCPErrorAck)  ||
						(pBFCPmsg->transactionType == kBFCPFloorStatusAck) ||
						((int)pBFCPmsg->transactionType == kTandbergBFCPFloorStatusAck) ||
						((int)pBFCPmsg->transactionType == kTandbergBFCPFloorRequestStatusAck) )
			{
				if (pBFCPmsg->transactionID == m_lastTransactionIdReq)
				{
						if (IsValidTimer(BFCP_MESSAGE_REQUEST_TOUT))
								DeleteTimer(BFCP_MESSAGE_REQUEST_TOUT);
						m_msgRetransmitReqCounter = 0;
						m_state = sBFCP_READY;
						PTRACE2INT(eLevelInfoNormal,"CSipBfcpCtrl::HandleUdpBfcpInd - UDP/BFCP: received ACK on request, state: ", m_state);
				}
				// if client transaction id doesn't match RMX transaction id
				else
				{
					str << "\n last transaction id request: " << m_lastTransactionIdReq << ", client response transaction id: " <<  pBFCPmsg->transactionID << ", state: " << m_state;
					PTRACE2(eLevelInfoNormal,"CSipBfcpCtrl::HandleUdpBfcpInd - UDP/BFCP, receive wrong transaction id for server request", str.GetString());
				}
				return STATUS_FAIL;
            }
			//ONLY reply for HELLO request in this state, let it go
			else if(kBFCPHello == pBFCPmsg->transactionType)
			{
				 PTRACE(eLevelInfoNormal,"CSipBfcpCtrl::HandleUdpBfcpInd - UDP/BFCP, receive  HELLO when we expect some ACK");
			}
			//BLOCK others
			else
			{
				 str << "\n last transaction type: " << pBFCPmsg->transactionType  << ", state: " << m_state;
				  PTRACE2(eLevelInfoNormal,"CSipBfcpCtrl::HandleUdpBfcpInd - UDP/BFCP, receive UNEXPECTED transaction", str.GetString());
				  return STATUS_FAIL;
			}
        }
		else if (m_state == sBFCP_MESSAGE_RECEIVED_REQUEST)
		{
				BFCPFloorInfoT *pBfcpFloorInfo = (BFCPFloorInfoT *) &m_pTranslator->lastDecodedMsg;

				str.Clear();

				::GetBFCPTransactionType(pBfcpFloorInfo->transactionType, str);

				PTRACE2(eLevelInfoNormal,"CSipBfcpCtrl::HandleUdpBfcpInd - already in state of received message, need to respond first, last request from remote: ",str.GetString());

				return STATUS_FAIL;
		}
		else // if (m_state == sBFCP_READY)
		{
			if ((pBFCPmsg->transactionType != kBFCPFloorRequestStatusAck) 	&&
				(pBFCPmsg->transactionType != kBFCPErrorAck) 				&&
				(pBFCPmsg->transactionType != kBFCPFloorStatusAck)		&&
				(pBFCPmsg->transactionType != kBFCPGoodbyeAck)              &&
				(pBFCPmsg->transactionType != kBFCPHelloAck) &&
				((int)pBFCPmsg->transactionType != kTandbergBFCPFloorStatusAck) &&
			    ((int)pBFCPmsg->transactionType != kTandbergBFCPFloorRequestStatusAck))
				m_state = sBFCP_MESSAGE_RECEIVED_REQUEST;
		}
	}
	else
		m_state = sBFCP_READY;

	//Now try to convert it to H239 struct:
    UInt16 floorReqID = 0; //The Floor Request ID will be given to us in this case from the BFCP msg

    //If TCP/BFCP Convert the message to SIP indication - we are not waiting for a response for anything so fkUnknownRoleTokenOpcode
    eStatus convertStatus = BFCPToSipInd (m_pTranslator, pBFCPmsg, pOpcode, &floorReqID);

	mcuId 		= pBFCPmsg->conferenceID;
	terminalId 	= pBFCPmsg->userID;

	if(pBFCPmsg && (pBFCPmsg->transactionType == kBFCPError))
	{
		str.Clear();
		str << "error-code: " << pBFCPmsg->errorCode;
		PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::HandleUdpBfcpInd - BFCPError: ", str.GetString());
	}

	str.Clear();

	str << "state: " << m_state << ", convert status: " << convertStatus << ", opcode: " << *pOpcode << ", mcuId:" << mcuId << ", terminalId: "<< terminalId;

	PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::HandleUdpBfcpInd - : ", str.GetString());

	if (convertStatus == statusError)
		return STATUS_FAIL;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CSipBfcpCtrl::HandleTcpBfcpReq(DWORD opcode)
{
	PTRACE2INT(eLevelInfoNormal, "CSipBfcpCtrl::HandleTcpBfcpReq, opcode: ", opcode);

	CMedString str;

	char *pRemoteProduct = NULL;

	BFCPFloorInfoT BFCPmsg;

	memset(&BFCPmsg,0,sizeof(BFCPFloorInfoT));

	UInt32 	pMsgLen;
	UInt8 	pOutBinaryBFCPMsg[256];

	memset(pOutBinaryBFCPMsg,0,256);

	BFCPFloorInfoT *pLastDecodeMsg 	= (BFCPFloorInfoT *) &m_pTranslator->lastDecodedMsg;;

	BYTE bIsServerResponseAck = FALSE;

	eBFCPPrimitive lastTransactionTypeInd = pLastDecodeMsg->transactionType;

	if (IsBfcpAck(opcode)										||
		IsFloorRequestResponse(lastTransactionTypeInd, opcode)	||
		IsFloorQueryResponse(lastTransactionTypeInd, opcode)	||
		IsFloorReleaseResponse(lastTransactionTypeInd, opcode))
		bIsServerResponseAck = TRUE;

	str << "m_state: " << m_state << ", opcode: " << opcode << ", last transaction: " << lastTransactionTypeInd << ", is response: " << bIsServerResponseAck;
	PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::HandleTcpBfcpReq: ", str.GetString());
	str.Clear();

	BFCPFloorInfoT *pBfcpFloorInfo 	= NULL;

	WORD 			floorRequestId 	= 1;

	if (bIsServerResponseAck)
	{
		pBfcpFloorInfo = pLastDecodeMsg;

		WORD *pFloorRequestId = &m_pTranslator->BFCPObj.floorInfo.floorRequestID;

		if (IsBfcpAck(opcode))
			floorRequestId = 0;

		// if response to FloorRequest, change Floor-Request-Id
		else if (IsFloorRequestResponse(lastTransactionTypeInd, opcode))
		{
			(*pFloorRequestId)++;

			if (*pFloorRequestId == 0xFFFF)
				*pFloorRequestId = 1;

			floorRequestId = *pFloorRequestId;
		}

		// if response to FloorQuery, use the Floor-Request-Id that was used for last FloorRequest
		else if (IsFloorQueryResponse(lastTransactionTypeInd, opcode))
			floorRequestId = *pFloorRequestId;

		// if response to FloorRelease - copy Floor-Request-Id from FloorRelease message
		else if (IsFloorReleaseResponse(lastTransactionTypeInd, opcode))
			floorRequestId = pBfcpFloorInfo->floorRequestID;
	}
	else
	{
		pBfcpFloorInfo = &m_pTranslator->BFCPObj.floorInfo;

		pBfcpFloorInfo->transactionID 	= 0;

		floorRequestId = pBfcpFloorInfo->floorRequestID;
	}

	SipReqToBFCP (pBfcpFloorInfo/*m_pTranslator*/, opcode, &BFCPmsg, floorRequestId);//m_lastFloorRequestID++);

	str << "\nTransactionType = " ;
	::GetBFCPTransactionType(BFCPmsg.transactionType, str);
	str << "\n";
	str << "FloorId         = " << BFCPmsg.floorID << "\n";
	str << "conferenceID    = " << BFCPmsg.conferenceID << "\n" ;
	str << "userID          = " << BFCPmsg.userID << "\n" ;
	str << "transactionID   = " << BFCPmsg.transactionID << "\n" ;
	str << "floorRequestID  = " << BFCPmsg.floorRequestID << "\n" ;
	str << "floorStatus     = ";
	::GetBFCPFloorStatusType(BFCPmsg.floorStatus, str);//<< pBFCPmsg.floorStatus <<
	str << "\n" ;
	str << "priority        = " << BFCPmsg.priority << "\n" ;
	PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::HandleTcpBfcpReq- BFCPmsg: ", str.GetString());

	str.Clear();

	if (m_bfcpRemoteIdent == TandbergEp)
	   pRemoteProduct = "TANDBERG";

	EncodeBFCPMsg (m_pTranslator, &BFCPmsg, &pMsgLen, pOutBinaryBFCPMsg, pRemoteProduct, m_bfcpTransportType, bIsServerResponseAck);

	char s[16];
	for(int i=0; i<(int)pMsgLen || i<56; i++)
	{
	   sprintf(s, "0x%x", pOutBinaryBFCPMsg[i]);
	   str << s << " ";
	}

	str << "\n Message length is " << pMsgLen;
	PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::HandleTcpBfcpReq - encoded message: ", str.GetString());

	size_t size = sizeof(mcReqBfcpMessage) + pMsgLen;

	mcReqBfcpMessage *pReqBFCPMsg = (mcReqBfcpMessage*)new BYTE[size];

	pReqBFCPMsg->status = STATUS_OK;
	pReqBFCPMsg->length = pMsgLen;

	memcpy(&pReqBFCPMsg->buffer[0],  pOutBinaryBFCPMsg, pMsgLen);

	pLastDecodeMsg->transactionType = kBFCPInvalidPrimitive;

	PTRACE2INT(eLevelInfoNormal, "CSipBfcpCtrl::HandleTcpBfcpReq - new state: ", m_state);

	SendMsgToMpl((BYTE*) pReqBFCPMsg, (int) size, IP_CM_BFCP_MESSAGE_REQ);

	PDELETEA(pReqBFCPMsg);

	return 0;
}

////////////////////////////////////////////////////////////////////////////
int CSipBfcpCtrl::HandleUdpBfcpReq(DWORD opcode)
{
	PTRACE2INT(eLevelInfoNormal, "CSipBfcpCtrl::HandleUdpBfcpReq, opcode: ", opcode);

	CMedString str;

	char *pRemoteProduct = NULL;

	UInt32 	pMsgLen;
	UInt8 	pOutBinaryBFCPMsg[256];

	memset(pOutBinaryBFCPMsg,0,256);

	BFCPFloorInfoT *pLastDecodeMsg 	= (BFCPFloorInfoT *) &m_pTranslator->lastDecodedMsg;;
	BFCPFloorInfoT BFCPmsg;

	memset(&BFCPmsg,0,sizeof(BFCPFloorInfoT));

	BYTE bIsServerResponseAck = FALSE;

	eBFCPPrimitive lastTransactionTypeInd = pLastDecodeMsg->transactionType;

	if (IsBfcpAck(opcode)										||
		IsFloorRequestResponse(lastTransactionTypeInd, opcode)	||
		IsFloorQueryResponse(lastTransactionTypeInd, opcode)	||
		IsFloorReleaseResponse(lastTransactionTypeInd, opcode))
		bIsServerResponseAck = TRUE;

	if ((m_bfcpRemoteIdent != TandbergEp) &&
		((m_state == IDLE) ||  (m_state == sBFCP_READY)))// 1. udp server initiated request 2. tcp
	{
		if (bIsServerResponseAck && !IsBfcpAck(opcode))
		{
			PASSERTMSG(opcode, "CSipParty::HandleUdpBfcpReq: no need to response in idle or ready state");
			return STATUS_FAIL;
		}
	}

	str << "m_state: " << m_state << ", opcode: " << opcode << ", last transaction: " << lastTransactionTypeInd << ", is response: " << bIsServerResponseAck;
	PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::HandleUdpBfcpReq - ", str.GetString());
	str.Clear();

	BFCPFloorInfoT *pBfcpFloorInfo 	= NULL;

	WORD 			floorRequestId 	= 1;

	if (bIsServerResponseAck)
	{
		pBfcpFloorInfo = pLastDecodeMsg;

		WORD *pFloorRequestId = &m_pTranslator->BFCPObj.floorInfo.floorRequestID;

		if (IsBfcpAck(opcode))
			floorRequestId = 0;

		// if response to FloorRequest, change Floor-Request-Id
		else if (IsFloorRequestResponse(lastTransactionTypeInd, opcode))
		{
			(*pFloorRequestId)++;

			if (*pFloorRequestId == 0xFFFF)
				*pFloorRequestId = 1;

			floorRequestId = *pFloorRequestId;
		}

		// if response to FloorQuery, use the Floor-Request-Id that was used for last FloorRequest
		else if (IsFloorQueryResponse(lastTransactionTypeInd, opcode))
			floorRequestId = *pFloorRequestId;

		// if response to FloorRelease - copy Floor-Request-Id from FloorRelease message
		else if (IsFloorReleaseResponse(lastTransactionTypeInd, opcode))
			floorRequestId = pBfcpFloorInfo->floorRequestID;
	}
	else
	{
		pBfcpFloorInfo = &m_pTranslator->BFCPObj.floorInfo;

		pBfcpFloorInfo->transactionID++;

		if (pBfcpFloorInfo->transactionID == 0xFFFF)
			pBfcpFloorInfo->transactionID = 1;

		floorRequestId = pBfcpFloorInfo->floorRequestID;
	}

	SipReqToBFCP (pBfcpFloorInfo/*m_pTranslator*/, opcode, &BFCPmsg, floorRequestId);//m_lastFloorRequestID++);

	str << "\nTransactionType = " ;
	::GetBFCPTransactionType(BFCPmsg.transactionType, str);
	str << "\n";
	str << "FloorId         = " << BFCPmsg.floorID << "\n";
	str << "conferenceID    = " << BFCPmsg.conferenceID << "\n" ;
	str << "userID          = " << BFCPmsg.userID << "\n" ;
	str << "transactionID   = " << BFCPmsg.transactionID << "\n" ;
	str << "floorRequestID  = " << BFCPmsg.floorRequestID << "\n" ;
	str << "floorStatus     = ";
	::GetBFCPFloorStatusType(BFCPmsg.floorStatus, str);//<< pBFCPmsg.floorStatus <<
	str << "\n" ;
	str << "priority        = " << BFCPmsg.priority << "\n" ;
	PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::HandleUdpBfcpReq- BFCPmsg: ", str.GetString());

	str.Clear();

	if (m_bfcpRemoteIdent == TandbergEp)
		pRemoteProduct = "TANDBERG";

	EncodeBFCPMsg (m_pTranslator, &BFCPmsg, &pMsgLen, pOutBinaryBFCPMsg, pRemoteProduct, m_bfcpTransportType, bIsServerResponseAck);

	char s[16];
	for(int i=0; i<(int)pMsgLen || i<56; i++)
	{
	   sprintf(s, "0x%x", pOutBinaryBFCPMsg[i]);
	   str << s << " ";
	}

	str << "\n Message length is " << pMsgLen;
	PTRACE2(eLevelInfoNormal, "CSipBfcpCtrl::HandleUdpBfcpReq - encoded message: ", str.GetString());

	size_t size = sizeof(mcReqBfcpMessage) + pMsgLen;

	mcReqBfcpMessage *pReqBFCPMsg = (mcReqBfcpMessage*)new BYTE[size];

	pReqBFCPMsg->status = STATUS_OK;
	pReqBFCPMsg->length = pMsgLen;

	memcpy(&pReqBFCPMsg->buffer[0],  pOutBinaryBFCPMsg, pMsgLen);

	BYTE bIsNeedToWaitForAck = FALSE;

	if ((IsNeedToWaitForAck(BFCPmsg.transactionType)) && (m_bfcpRemoteIdent != TandbergEp))
	{
		bIsNeedToWaitForAck = TRUE;
		PTRACE(eLevelInfoNormal, "CSipBfcpCtrl::HandleUdpBfcpReq - need to wait for ACK: ");
	}

	if ((m_bfcpRemoteIdent != TandbergEp) && bIsNeedToWaitForAck)
	{
		m_lastTransactionIdReq = BFCPmsg.transactionID;

		int timeout;

		if (m_pBfcpReq)
		    PDELETEA(m_pBfcpReq);

		m_pBfcpReq = (mcReqBfcpMessage*)new BYTE[size];

		memcpy(m_pBfcpReq, pReqBFCPMsg, size);

		m_msgRetransmitReqCounter += 1;

		timeout = m_msgRetransmitReqCounter * BFCP_RETRANSMISSION_T1;

		StartTimer(BFCP_MESSAGE_REQUEST_TOUT,  timeout * SECOND);
	}

	if (m_bfcpRemoteIdent != TandbergEp)
	{
		if ((m_state == sBFCP_MESSAGE_RECEIVED_REQUEST) && !bIsNeedToWaitForAck)
			m_state = sBFCP_READY;
		else if (bIsNeedToWaitForAck)
			m_state = sBFCP_MESSAGE_SENT_REQUEST;
	}
	else
		m_state = sBFCP_READY;

	pLastDecodeMsg->transactionType = kBFCPInvalidPrimitive;

	PTRACE2INT(eLevelInfoNormal, "CSipBfcpCtrl::HandleUdpBfcpReq - new state: ", m_state);

	SendMsgToMpl((BYTE*) pReqBFCPMsg, (int) size, IP_CM_BFCP_MESSAGE_REQ);

	PDELETEA(pReqBFCPMsg);

	return 0;
}

////////////////////////////////////////////////////////////////////////////
void CSipBfcpCtrl::SendHelloKeepAlive()
{
    if (!m_pTranslator)
    {
        PTRACE(eLevelInfoNormal, "CSipBfcpCtrl::SendHelloKeepAlive - m_pTranslator is NULL");
        return;
    }

    // AN, 12.9.12, VNGR-26807
    // Need to send Hello regardless of EP ident in case that count bumber is higher than 1.
    // Generally used when working with RPAD
        CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
        BOOL res;
        res = sysConfig->GetDWORDDataByKey(CFG_KEY_BFCP_HELLO_KEEP_ALIVE_COUNT, m_keepAliveIterationsCount);

        PTRACE2INT(eLevelInfoNormal, "CSipBfcpCtrl::SendHelloKeepAlive - sends Hello request for keep alive purpose for EPs IterationsCount=", m_keepAliveIterationsCount);

        if(m_keepAliveIterationsCount > 0)
        {
            OnKeepAlive();
        }
}

void CSipBfcpCtrl::OnKeepAlive()
{
    if(m_keepAliveIterationsCount > 0)
    {
    	m_state = sBFCP_HELLO_KEEP_ALIVE_REQUEST;

        m_keepAliveIterationsCount--;
        HandleUdpBfcpReq(CONTENT_ROLE_BFCP_HELLO);
        if(m_keepAliveIterationsCount > 0)
            StartTimer(BFCP_HELLO_KEEP_ALIVE, 3 * SECOND);

        PTRACE2INT(eLevelInfoNormal, "CSipBfcpCtrl::OnKeepAlive - hello was sent, left iteration count=", m_keepAliveIterationsCount);

    }
}

////////////////////////////////////////////////////////////////////////////
BYTE CSipBfcpCtrl::IsBfcpAck(DWORD opcode)
{
	if ((opcode == CONTENT_ROLE_BFCP_HELLO_ACK)					||
		(opcode == CONTENT_ROLE_BFCP_GOODBYE_ACK) 				||
		(opcode == CONTENT_ROLE_BFCP_ERROR_ACK))
		return TRUE;

	return FALSE;
}
////////////////////////////////////////////////////////////////////////////
BYTE CSipBfcpCtrl::IsFloorRequestResponse(eBFCPPrimitive lastTransType, DWORD opcode)
{
	if (lastTransType != kBFCPFloorRequest)
		return FALSE;

	if ((opcode == CONTENT_ROLE_TOKEN_ACQUIRE_ACK) ||
	    (opcode == CONTENT_ROLE_TOKEN_ACQUIRE_NAK))
	   return TRUE;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
BYTE CSipBfcpCtrl::IsFloorQueryResponse(eBFCPPrimitive lastTransType, DWORD opcode)
{
	if (lastTransType != kBFCPFloorQuery)
		return FALSE;

	if ((opcode == CONTENT_ROLE_PROVIDER_IDENTITY) ||
		(opcode == CONTENT_NO_ROLE_PROVIDER))
		return TRUE;

	return FALSE;
}
////////////////////////////////////////////////////////////////////////////
BYTE CSipBfcpCtrl::IsFloorReleaseResponse(eBFCPPrimitive lastTransType, DWORD opcode)
{
	if (lastTransType != kBFCPFloorRelease)
		return FALSE;

	if (opcode == CONTENT_ROLE_TOKEN_RELEASE_ACK)
		return TRUE;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
BYTE CSipBfcpCtrl::IsNeedToWaitForAck(eBFCPPrimitive transactionType)
{
	if ((transactionType == kBFCPFloorRequestStatus) 	||
		(transactionType == kBFCPFloorStatus)			||
		(transactionType == kBFCPError))
		//(transactionType == kBFCPGoodbye))
		return TRUE;

	return FALSE;

}
