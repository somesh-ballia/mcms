// ChangePassword.h: interface for the CChangePassword class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CHANGEPASSWORD__H_)
#define CHANGEPASSWORD__H_

#include "SerializeObject.h"


/////////////////////////////////////////////////////////////////////////////
// CChangePassword 

class CChangePassword : public CSerializeObject
{

CLASS_TYPE_1(CChangePassword, CSerializeObject)

public:

    CChangePassword();
	CChangePassword(const CChangePassword& other);

    virtual ~CChangePassword();

    const char * NameOf(void) const {return "CChangePassword";}
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action=NULL);
    CSerializeObject* Clone() { return new CChangePassword(*this);}

    void   SetLogin(const std::string  strLogin);
    const std::string&   GetLogin () const;
    void   SetNewPassword(const std::string  strNewPassword);
    const std::string&   GetNewPassword () const;
    void   SetOldPassword(const std::string  strOldPassword);
    const std::string&   GetOldPassword () const;

protected:

	std::string m_strLogin;
	std::string m_strNewPassword;
	std::string m_strOldPassword;
};

#endif // !defined(CHANGEPASSWORD__H_)
