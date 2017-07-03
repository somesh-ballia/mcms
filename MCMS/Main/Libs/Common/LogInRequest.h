// LogInRequest.h: interface for the CLogInRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LOGINREQUEST_H__)
#define _LOGINREQUEST_H__

#include "SerializeObject.h"
//#include "Operator.h"
#include <string>

class CXMLDOMElement;

/////////////////////////////////////////////////////////////////////////////
// CLogInRequest

class CLogInRequest : public CSerializeObject
{

CLASS_TYPE_1(CLogInRequest,CSerializeObject)	

public:

	virtual const char* NameOf() const { return "CLogInRequest";}
	CLogInRequest();
	virtual ~CLogInRequest();
	CLogInRequest& operator = (const CLogInRequest &other);
	
   	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action=NULL);
	CSerializeObject* Clone() {return new CLogInRequest;}
	

	//COperator* GetOperator() const { return m_pOperator; };
    const std::string GetPassword() const{return m_password;}
    const std::string GetLoginName() const{return m_loginName;}
	const std::string GetStationName() const {return m_strStationName;}
    const std::string GetNewPassword() const {return m_newPassword;}
	void SetLoginName(const string & user) {m_loginName = user;}
	void SetPassword(const string & pwd) {m_password = pwd;}
    

	void SetExternalDbAuthorized(BYTE bAuthohrized);
	BYTE GetExternalDbAuthorized()const {return m_bExternalDbAuthorized;}
	WORD GetMasterSlaveCurrentState()const {return m_MasterSlaveCurrentState;};
	std::string GetClientIP()const {return m_Client_ip;};
	BYTE GetCompressionLevel();
private:

//	COperator* m_pOperator;
    std::string m_loginName;
    std::string m_password;
    std::string m_newPassword;
    
//    std::string m_oldPassword;
	std::string m_strStationName;
	BYTE m_bCompressionLevel;
	BYTE m_bConferenceRecorderLogin;
	BYTE m_bExternalDbAuthorized;
	WORD m_MasterSlaveCurrentState;
	char  m_Client_ip[IP_ADDRESS_STR_LEN];
};

#endif // !defined(_LOGINREQUEST_H__)
