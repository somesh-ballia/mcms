//+========================================================================+
//                     TokenMsgMngr.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       TokenMsgMngr.h	                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2006  |                                                      |
//+========================================================================+

#ifndef _TOKENMSGMNGR_H__
#define _TOKENMSGMNGR_H__

#include <vector>
#include "ConfPartyDefines.h"
#include "ConfPartyOpcodes.h"
#include "PObject.h"
#include "TokenMsg.h"

typedef std::vector< CTokenMsg *>TOKEN_MSG_LIST;

typedef enum {
	eMsgInvalid = 0, 
	eMsgDelayed,
	eMsgFree
}EMsgStatus;

typedef enum {
	eHwStreamStateNone = 0,
	eHwStreamStateOn,  
	eHwStreamStateOff
}EHwStreamState;
#define ENABLE  1
#define DISABLE 0

class CTokenMsgMngr : public CPObject
{
CLASS_TYPE_1(CTokenMsgMngr,CPObject)
public: 
	
	// Constructors
	CTokenMsgMngr();
	virtual const char* NameOf() const { return "CTokenMsgMngr";}
	virtual ~CTokenMsgMngr(); 
	CTokenMsgMngr (const CTokenMsgMngr& rOtherTokenMsgMngr);

	// Overloaded operators
	CTokenMsgMngr&	operator= (const CTokenMsgMngr& rOther);

	EMsgStatus NewTokenMsg(CTokenMsg* pTokenMsg, EHwStreamState eHwStreamState);
	void StreamUpdate(CTokenMsgMngr* pList);
	void EnableTokenMsgMngr(){ m_state=ENABLE; };
	WORD isEnable(){return m_state;};
	void DisableTokenMsgMngr(){m_state=DISABLE;};
	DWORD Size();
	void ClearAndDestroy();
	TOKEN_MSG_LIST::iterator Begin();
	TOKEN_MSG_LIST::iterator End();
	void Clear();
	
protected:
	
	// Internal use operations on the list (vector)
	EStat AddMsg(CTokenMsg* pTokenMsg);	
	
	// Attributes             
	TOKEN_MSG_LIST		  m_tokenMsgList;	
	WORD                  m_state;

private:
    
    BOOL IsListEmpty();
   
	EMsgStatus AddMsgToListAndRetDelayed(CTokenMsg* pMsg);
	
	
};


#endif /* _TOKENMSGMNGR_H__ */
