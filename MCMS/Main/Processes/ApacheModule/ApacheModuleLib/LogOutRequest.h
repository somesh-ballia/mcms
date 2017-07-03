// LogOutRequest.h: interface for the CLogOutRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LOGOUTREQUEST_H__)
#define _LOGOUTREQUEST_H__

#include "SerializeObject.h"
#include <string>

class CXMLDOMElement;

enum eLogoutReason
{
    logoutNormal,
    logoutSessionExpired
};



/////////////////////////////////////////////////////////////////////////////
// CLogOutRequest

class CLogOutRequest : public CSerializeObject
{

CLASS_TYPE_1(CLogOutRequest,CSerializeObject)	

public:

	CLogOutRequest();
	virtual ~CLogOutRequest();
	CLogOutRequest& operator = (const CLogOutRequest &other);
	
   	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action=NULL);
	CSerializeObject* Clone() {return new CLogOutRequest;}
	


	eLogoutReason GetLogourReason()const {return m_reason;}

private:

    eLogoutReason m_reason;
};

#endif // !defined(_LOGOUTREQUEST_H__)
