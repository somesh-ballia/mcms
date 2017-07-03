//+========================================================================+
//                BridgePartyMediaUniDirection.CPP                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyMediaUniDirection.CPP                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  June-2005  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#include "BridgePartyMediaUniDirection.h"
#include "HardwareInterface.h"
#include "ProcessBase.h"
#include "FaultsDefines.h"
#include "HlogApi.h"

// ------------------------------------------------------------
CBridgePartyMediaUniDirection::CBridgePartyMediaUniDirection ()
{
	m_pBridgePartyCntl		=	NULL;	
	m_pHardwareInterface	=	NULL;
	m_closePortAckStatus    =   statOK;
	m_lastReqId				=   0xFFFFFFFF;
	m_lastReq				=   0xFFFFFFFF;
}

// ------------------------------------------------------------
CBridgePartyMediaUniDirection::~CBridgePartyMediaUniDirection ()
{
	POBJDELETE(m_pHardwareInterface);
}

// ------------------------------------------------------------
CBridgePartyMediaUniDirection::CBridgePartyMediaUniDirection (const CBridgePartyMediaUniDirection& rBridgePartyMediaUniDirection)
    :CStateMachineValidation(rBridgePartyMediaUniDirection)
{
	m_pBridgePartyCntl	=	rBridgePartyMediaUniDirection.m_pBridgePartyCntl;

	m_pHardwareInterface = NULL;
	m_closePortAckStatus = rBridgePartyMediaUniDirection.m_closePortAckStatus;
	m_lastReqId = rBridgePartyMediaUniDirection.m_lastReqId;
	m_lastReq				=   0xFFFFFFFF;
}

// ------------------------------------------------------------
CBridgePartyMediaUniDirection& CBridgePartyMediaUniDirection::operator= (const CBridgePartyMediaUniDirection& rBridgePartyMediaUniDirection)
{
	if ( &rBridgePartyMediaUniDirection == this ) 
		return *this;

	m_pBridgePartyCntl	=	rBridgePartyMediaUniDirection.m_pBridgePartyCntl;

	if (NULL == rBridgePartyMediaUniDirection.m_pHardwareInterface) 
	{
		POBJDELETE(m_pHardwareInterface);
		m_pHardwareInterface = NULL;
	}
	else 
	{
		if (NULL != m_pHardwareInterface)
			*m_pHardwareInterface = *(rBridgePartyMediaUniDirection.m_pHardwareInterface);
	}
	
	m_closePortAckStatus = 	rBridgePartyMediaUniDirection.m_closePortAckStatus;
	m_lastReqId = rBridgePartyMediaUniDirection.m_lastReqId;
	m_lastReq				=   0xFFFFFFFF;
	
	return *this;
}

// ------------------------------------------------------------
void CBridgePartyMediaUniDirection::Create (const CBridgePartyCntl* pBridgePartyCntl, 
											const CRsrcParams* pRsrcParams)
{
	m_pBridgePartyCntl		=	(CBridgePartyCntl*)pBridgePartyCntl;

	if (NULL != m_pHardwareInterface)
		m_pHardwareInterface->Create((CRsrcParams*)pRsrcParams);
}

// ------------------------------------------------------------
void  CBridgePartyMediaUniDirection::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{

}

// ------------------------------------------------------------
CRsrcParams* CBridgePartyMediaUniDirection::GetRsrcParams()
{
	if(m_pHardwareInterface)
		return(m_pHardwareInterface->GetRsrcParams());
	
	return NULL;
}

// ------------------------------------------------------------
void CBridgePartyMediaUniDirection::RemoveConfParams ()
{
	m_pHardwareInterface->SetConfRsrcId(INVALID);
}

// ------------------------------------------------------------
void CBridgePartyMediaUniDirection::UpdateNewConfParams (DWORD confRsrcId)
{
	m_pHardwareInterface->SetConfRsrcId(confRsrcId);
}

// ------------------------------------------------------------
void CBridgePartyMediaUniDirection::AddFaultAlarm(std::string message,DWORD partyRsrcId,STATUS status,bool isAckNotRecieved)
{
	//Add assert to EMA in case of NACK
	ALLOCBUFFER(pStr,256);
	
	DWORD faultOpcode =0;
	
	if (isAckNotRecieved)
	{
		faultOpcode=ACK_NOT_RECEIVED;
		sprintf(pStr,"%s , PatyRsrId:%d",message.c_str(),partyRsrcId);
	}
	else
	{
		faultOpcode=ACK_FAILED;
		sprintf(pStr,"%s , PatyRsrId:%d,Status: %d",message.c_str(),partyRsrcId,status);
	}

	CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
						faultOpcode, 
						MAJOR_ERROR_LEVEL, 
						pStr,
						TRUE);

	DEALLOCBUFFER(pStr);
}
// ------------------------------------------------------------


