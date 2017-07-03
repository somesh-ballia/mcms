//+========================================================================+
//                BridgePartyMediaUniDirection.H                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyMediaUniDirection.H                              |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                   |
//-------------------------------------------------------------------------|
//
//+========================================================================+

#ifndef _CBridgePartyMediaUniDirection_H_
#define _CBridgePartyMediaUniDirection_H_

#include "StateMachine.h"
#include "ConfPartyDefines.h"
#include "ConfPartyOpcodes.h"

class CConfApi;
class CRsrcParams;
class CBridgePartyMediaParams;
class CHardwareInterface;
class CBridgePartyCntl;

class CBridgePartyMediaUniDirection : public CStateMachineValidation {
CLASS_TYPE_1(CBridgePartyMediaUniDirection,CStateMachineValidation)
public:
	CBridgePartyMediaUniDirection ();
	virtual ~CBridgePartyMediaUniDirection ();
	CBridgePartyMediaUniDirection (const CBridgePartyMediaUniDirection& rBridgePartyMediaUniDirection);
	CBridgePartyMediaUniDirection&	operator= (const CBridgePartyMediaUniDirection& rBridgePartyMediaUniDirection);
	virtual const char* NameOf() const { return "CBridgePartyMediaUniDirection";}

	virtual void	Create (const CBridgePartyCntl*	pBridgePartyCntl, const CRsrcParams* pRsrcParams);

	virtual void	Connect()		=	0;
	virtual void	DisConnect()	=	0;

	virtual BOOL	IsConnected()	= 0;
	virtual BOOL 	IsConnecting()	= 0;
	virtual BOOL	IsDisConnected()= 0;
	virtual BOOL    IsDisconnecting() = 0;


	virtual void	HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	CRsrcParams*	GetRsrcParams();
	virtual void 	RemoveConfParams ();
	virtual void 	UpdateNewConfParams (DWORD confRsrcId);

	static void AddFaultAlarm(std::string message,DWORD partyRsrcId,STATUS status,bool isAckNotRecieved=false);

	virtual BYTE   GetClosePortAckStatus(){return m_closePortAckStatus;}
	virtual void   SetClosePortAckStatus(BYTE status) {m_closePortAckStatus=status;}

	virtual void   CopyOldStateForMove(WORD state) {m_state = state;}
	virtual void   GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap) {}

	DWORD GetLastReqId(){return m_lastReqId;}
	DWORD GetLastReq(){return m_lastReq;}

protected:
   	CBridgePartyCntl*	m_pBridgePartyCntl;
	CHardwareInterface*	m_pHardwareInterface;
	BYTE m_closePortAckStatus;// we need to save the status from the Ack to the close port request.
	DWORD m_lastReqId; // for mcu internal problem print in case request fails / timeout
	DWORD m_lastReq;
};

#endif
