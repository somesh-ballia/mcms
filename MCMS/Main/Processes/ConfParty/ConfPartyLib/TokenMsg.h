//+========================================================================+
//                   TokenMsg.H                                     |
//					 Copyright 2005 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       TokenMsg.H                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                    |
//-------------------------------------------------------------------------|
// Who  | Date  June-2006  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================+

#ifndef _CTokenMsg_H_
#define _CTokenMsg_H_

#include "PObject.h"
#include "Segment.h"

typedef enum {
	eMsgIn = 0, 
	eMsgOut
}EMsgDirection;

class CTokenMsg : public CPObject {
CLASS_TYPE_1(CTokenMsg,CPObject)
public:
	virtual ~CTokenMsg();
	CTokenMsg();
	CTokenMsg(OPCODE opcode,EMsgDirection msgDirection,CSegment* msgParam);
	
	virtual const char* NameOf() const { return "CTokenMsg";}
	CTokenMsg (const CTokenMsg& rOtherTokenMsg);
	CTokenMsg& operator= (const CTokenMsg& rOtherTokenMsg);
	OPCODE GetMsgOpcode() { return m_dwMsgOpcode;}
	EMsgDirection GetMsgDirection() { return m_eMsgDirection;}
	CSegment* GetMsgSegment() { return m_pMsgParams;}
	

private:
	 	
	OPCODE			m_dwMsgOpcode;
	EMsgDirection	m_eMsgDirection;
	CSegment*		m_pMsgParams;

};

#endif

