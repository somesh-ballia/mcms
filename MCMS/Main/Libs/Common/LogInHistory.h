// LogInHistory.h: interface for the LogInHistory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LOGINHISTORY_H__)
#define _LOGINHISTORY_H__

#include <string>
#include "CommonStructs.h"
#include "SerializeObject.h"
#include "DefinesGeneral.h"
#include "CardsStructs.h"
#include "StructTm.h"

class CXMLDOMElement;

#define MAX_FAILED_LOGINS       10


typedef struct
{
	CStructTm    login_time;
	std::string  address;
    void Init();

} LOGIN_RECORD_S;


class CLogInHistory : public CSerializeObject
{
CLASS_TYPE_1(CLogInHistory,CSerializeObject )
public:
	//Constructors
	CLogInHistory();
	CLogInHistory(const CLogInHistory &other);
	virtual ~CLogInHistory();
///	const CLogInHistory& operator=(const CLogInHistory& other);

   	virtual void SerializeXml(CXMLDOMElement*& pXMLResponse) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action = 0);

	CSerializeObject* Clone() {return new CLogInHistory;}

    const char*  NameOf() const {return "CLogInHistory";}

	CStructTm    GetLastLogin() const;
	void         SetLastLogin(const CStructTm last_login);
	std::string  GetLastLoginIPaddress() const;
	void         SetLastLoginIPaddress(const std::string address);
	void         SetFailedLoginInfo(const CStructTm last_login, const std::string address);
	void         ResetLogInHistory();
	BOOL         IsFirstLogin() const;
	CStructTm	 GetLastFailedLoginInList()const;

protected:

	LOGIN_RECORD_S  m_lastLogin;
	LOGIN_RECORD_S  m_failedLogins[MAX_FAILED_LOGINS];

	DWORD m_NumberOfFailedLogin;
};

#endif // !defined(_LOGINHISTORY_H__)
