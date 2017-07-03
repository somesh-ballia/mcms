//+========================================================================+
//                   TokenMsg.CPP                                   |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       TokenMsg.CPP                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                    |
//-------------------------------------------------------------------------|
// Who  | Date  June-2006  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#include "TokenMsg.h"
#include "OpcodesRanges.h"

// ------------------------------------------------------------
CTokenMsg::~CTokenMsg() 
{
	if(CPObject::IsValidPObjectPtr(m_pMsgParams))
		POBJDELETE(m_pMsgParams);
}

/////////////////////////////////////////////////////////////////////////////
CTokenMsg::CTokenMsg()
{
	m_pMsgParams = NULL;
}
/////////////////////////////////////////////////////////////////////////////
CTokenMsg::CTokenMsg(OPCODE opcode,EMsgDirection eMsgDirection,CSegment* msgParam)
{
	m_dwMsgOpcode  = opcode;
	m_eMsgDirection = eMsgDirection;
	m_pMsgParams = new CSegment;
	
	*m_pMsgParams   = *msgParam;
}

/////////////////////////////////////////////////////////////////////////////
CTokenMsg::CTokenMsg (const CTokenMsg& rOtherTokenMsg)  
    :CPObject(rOtherTokenMsg)
{
	*this = rOtherTokenMsg;
}

/////////////////////////////////////////////////////////////////////////////
CTokenMsg&	CTokenMsg::operator= (const CTokenMsg& rOtherTokenMsg)
{
	if ( &rOtherTokenMsg == this ) 
		return *this;
	
	m_dwMsgOpcode  = rOtherTokenMsg.m_dwMsgOpcode;
	m_eMsgDirection = rOtherTokenMsg.m_eMsgDirection;
	m_pMsgParams = new CSegment;
	*m_pMsgParams   = *(rOtherTokenMsg.m_pMsgParams);  	

	return *this;
}
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


