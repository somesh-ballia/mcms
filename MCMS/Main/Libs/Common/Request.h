#ifndef _SET_REQEUST__
#define _SET_REQEUST__

#include <string>
#include "PObject.h"
#include "Transactions.h"
#include "XmlApi.h"
#include "OsQueue.h"
#include "SystemFunctions.h"

#define STRING_FLAG 0
#define OBJECT_FLAG 1

class COsQueue;
class CXMLDOMElement;
class CXMLDOMNodeList;
class CTransactionsFactory;
class CSerializeObject;

////////////////////////////////////////////////////////////////////////////
//                        CRequest
////////////////////////////////////////////////////////////////////////////
class CRequest : public CPObject
{
	CLASS_TYPE_1(CRequest, CPObject)

public:
	                    CRequest();
	                    CRequest(CXMLDOMElement* pXMLSetRequest, const CTransactionsFactory& transFactory);
	virtual            ~CRequest();
	virtual const char* NameOf() const { return "CRequest";}

	CRequest&           operator=(const CRequest& other);
	CXMLDOMElement*     SerializeXml();
	int                 DeserializeXml(const CTransactionsFactory& transFactory);
	CXMLDOMElement*     SetConfirmSerializeXml(std::string requestUserName = "");

	DWORD               GetHdr() const                                    { return m_mes_header; }
	void                SetHdr(DWORD msg_hdr)                             { m_mes_header = msg_hdr; }

	const std::string&  GetTransName() const                              { return m_transName; }
	void                SetTransName(const std::string& trans_name)       { m_transName = trans_name; }

	DWORD               GetConnectId() const                              { return m_conId; }
	void                SetConnectId(DWORD conId)                         { m_conId = conId; }

	WORD                GetObjectFlag() const                             { return m_objectFlag; }
	void                SetObjectFlag(WORD objFlag)                       { m_objectFlag = objFlag; }

	const std::string&  GetActionName() const                             { return m_actionName; }
	void                SetActionName(const std::string& action)          { m_actionName = action; }

	COsQueue*           GetQueue() const                                  { return m_pQueue; }
	void                SetQueue(const COsQueue& other)                   { *m_pQueue = other; }

	unsigned char       IsXmlStream() const                               { return m_bXmlStream; }
	void                SetXmlStream(unsigned char bXml)                  { m_bXmlStream = bXml; }

	DWORD               GetUserToken1() const                             { return m_UserToken1; }
	void                SetUserToken1(DWORD user_token1)                  { m_UserToken1 = user_token1; }

	DWORD               GetUserToken2() const                             { return m_UserToken2; }
	void                SetUserToken2(DWORD user_token2)                  { m_UserToken2 = user_token2; }

	WORD                GetAuthorization() const                          { return m_nAuthorization; }
	void                SetAuthorization(WORD nAuthorization)             { m_nAuthorization = nAuthorization; }

	int                 GetStatus() const                                 { return m_nStatus;}
	void                SetStatus(int status)                             { m_nStatus = status; }

	eXmlActionNodeType  GetXmlActionNodeType() const                      { return m_ActionNodeType; }
	void                SetXmlActionNodeType(eXmlActionNodeType nodeType) { m_ActionNodeType = nodeType; }

	const char*         GetExDescription() const                          { return m_szErrorMessage; }
	void                SetExDescription(const char* exDesc)              { strcpy_safe(m_szErrorMessage, exDesc); }

    DWORD GetIfNoneMatch()const{return m_ifNoneMatch;}
    void SetIfNoneMatch(DWORD ifNoneMatch){m_ifNoneMatch = ifNoneMatch;}


	CSerializeObject*   GetRequestObject() const;
	void                SetRequestObject(CSerializeObject* pRequestObj);
	void                SetConfirmObject(CSerializeObject* pConfirmObj);
	const CSerializeObject*   GetConfirmObject()const  { return m_confirmObject;}

	void                DumpSetRequestAsString(char** ppSetRequestString);

protected:
	DWORD               m_mes_header;              // message ID
	DWORD               m_conId;                   // connection ID (from LogIn confirmation) .

	CSerializeObject*   m_requestObject;
	CSerializeObject*   m_confirmObject;

	WORD                m_objectFlag;              // object or string

	std::string         m_actionName;
	std::string         m_transName;

	DWORD               m_UserToken1;
	DWORD               m_UserToken2;
	DWORD               m_ExtDbToken;

	unsigned char       m_bXmlStream;
	CXMLDOMElement*     m_pXMLSetRequest;
	eXmlActionNodeType  m_ActionNodeType;
	COsQueue*           m_pQueue;
	WORD                m_nAuthorization;
	DWORD 				m_ifNoneMatch;

	int                 m_nStatus;
	char                m_szErrorMessage[ERROR_MESSAGE_LEN];
};

#endif /* _SET_REQEUST__ */
