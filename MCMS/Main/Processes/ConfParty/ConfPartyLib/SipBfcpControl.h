/*
 * SipBfcpControl.h
 *
 *  Created on: Nov 8, 2011
 *      Author: Ami Noy
 */

#ifndef SIPBFCPCONTROL_H_
#define SIPBFCPCONTROL_H_

#include "StateMachine.h"
#include "TaskApp.h"
#include "HardwareInterface.h"
#include "PartyApi.h"
#include "BFCPMessageNew.h"
#include "OpcodesMcmsCardMngrBFCP.h"
#include "BfcpStructs.h"
#include "BFCPH239Translator.h"

//// SIP BFCP states for state-machine:
const WORD sBFCP_HELLO_KEEP_ALIVE_REQUEST	= 110;
const WORD sBFCP_READY 						= 111;
const WORD sBFCP_MESSAGE_RECEIVED_REQUEST 	= 112;
const WORD sBFCP_MESSAGE_SENT_REQUEST		= 113;
const WORD sBFCP_MESSAGE_RETRANSMIT			= 114;


typedef enum
{
    BFCP_FULL_FLOW,
    BFCP_UDP_ONLY,
    BFCP_TCP_ONLY
} eSipBfcpMode4DialOut;

class CSipBfcpCtrl : public CStateMachine
{
	CLASS_TYPE_1(CSipBfcpCtrl, CStateMachine)

public:

	CSipBfcpCtrl(CTaskApp * pOwnerTask, eSipBfcpMode4DialOut mode);
	virtual const char* NameOf() const { return "CSipBfcpCtrl";}
	virtual ~CSipBfcpCtrl();
	virtual void* GetMessageMap() {return (void*)m_msgEntries;}
	virtual void  HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	void 	Initialize(CHardwareInterface* pMfaInterface, CPartyApi* pPartyApi, WORD mcuNum, WORD termNum,
				enTransportType tarnsType, RemoteIdent remoteIdent);

	void 	UpdateParams(enTransportType tarnsType, RemoteIdent remoteIdent);
	DWORD 	SendMsgToMpl(BYTE* pStructure, int structureSize, DWORD opcode);
	enTransportType 	GetBfcpTransportType();
	void 				SetBfcpConnMode(eBFCPSetup remoteSetup);
	eBFCPSetup 			GetBfcpSetupMode();
	void 				SetBfcpConnection(eBFCPConnection connection){ m_connection = connection; }
	eBFCPConnection		GetBfcpConnection(){ return m_connection; }
	eConnectionTransportMode 	GetBfcpConnMode();
	void			 	SetBfcpConnMode(eConnectionTransportMode connMode){ m_connMode = connMode; };
	eSipBfcpMode4DialOut GetDialOutMode(){ return  m_DialOutMode;}
	void				SetDialOutMode(eSipBfcpMode4DialOut enMode){m_DialOutMode = enMode;};
	//void 				SendBfcpMessageReq(DWORD opcode, WORD mcuId, WORD terminalId);
	int 				HandleTcpBfcpInd(BFCPFloorInfoT *pBFCPmsg, eBFCPPrimitive bfcpType, APIS32 *pOpcode, int connStatus);
	int 				HandleUdpBfcpInd(BFCPFloorInfoT *pBFCPmsg, eBFCPPrimitive bfcpType, APIS32 *pOpcode, int connStatus);

	int 				HandleTcpBfcpReq(DWORD opcode);
	int 				HandleUdpBfcpReq(DWORD opcode);
    void                SendHelloKeepAlive();
    void                OnKeepAlive();

	BYTE 				IsBfcpAck(DWORD opcode);
	BYTE 				IsFloorRequestResponse(eBFCPPrimitive lastTransType, DWORD opcode);
	BYTE 				IsFloorQueryResponse(eBFCPPrimitive lastTransType, DWORD opcode);
	BYTE 				IsFloorReleaseResponse(eBFCPPrimitive lastTransType, DWORD opcode);
	BYTE 				IsNeedToWaitForAck(eBFCPPrimitive transactionType);

	void OnIpCmBfcpMessageInd(CSegment* pParam);
	void OnSendBfcpMessageReq(CSegment* pParam);
	void OnIpCmBfcpMessageSentTout(CSegment* pParam);

protected:

	CHardwareInterface*			m_pMfaInterface;
	CPartyApi*					m_pPartyApi;
    WORD						m_mcuNum;
    WORD						m_termNum;
    BYTE						m_bWaitForWithdrawAck;
    BYTE						m_bIsRequestSent;

    BFCPH239Translator 			*m_pTranslator;
    APIU16						m_lastFloorRequestID;

    enTransportType				m_bfcpTransportType;
    eConnectionTransportMode	m_connMode;
    eBFCPSetup					m_setupMode;
    RemoteIdent 				m_bfcpRemoteIdent;
    eBFCPConnection				m_connection;

    int							m_msgRetransmitReqCounter;
    BYTE						m_bIsWaitingForAck;

    DWORD						m_lastOpcodeSentReq;
    int							m_lastTransactionType;

    int							m_transactionInitiator;

    WORD						m_lastTransactionIdReq;

    BFCPFloorInfoT  			m_rcvBfcpMsg;
    mcReqBfcpMessage			*m_pBfcpReq;

    eSipBfcpMode4DialOut        m_DialOutMode;

    DWORD                       m_keepAliveIterationsCount;

	PDECLAR_MESSAGE_MAP

};
#endif /* SIPBFCPCONTROL_H_ */
