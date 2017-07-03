#ifndef TOKENMSGMNGRMOCK_H_
#define TOKENMSGMNGRMOCK_H_

#include "TokenMsgMngr.h"

class CTokenMsgMngrMock : public CTokenMsgMngr
{
public:
	virtual ~CTokenMsgMngrMock () {}
	CTokenMsgMngrMock () : CTokenMsgMngr() {}
	CTokenMsgMngrMock (const CTokenMsgMngrMock& rTokenMsgMngr): CTokenMsgMngr(rTokenMsgMngr) {}
	CTokenMsgMngrMock&	operator= (const CTokenMsgMngrMock& rOther) { (CTokenMsgMngr&)(*this) = (CTokenMsgMngr&)rOther; return *this; }
	
	virtual const char*	NameOf () const { return "CTokenMsgMngrMock"; }
	
	TOKEN_MSG_LIST::iterator Begin(){TOKEN_MSG_LIST::iterator itr = m_tokenMsgList.begin(); return itr;}
	DWORD GetSize(){DWORD size=(CTokenMsgMngr::Size());return size;}
	void ClearAndDestroyList(){CTokenMsgMngr::ClearAndDestroy();}
	EStat AddTokMsg(CTokenMsg* pTokenMsg){EStat status=CTokenMsgMngr::AddMsg(pTokenMsg);return status;}

};

#endif /*TOKENMSGMNGRMOCK_H_*/
