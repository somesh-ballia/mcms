// Operator.h: interface for the COperator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(OPERATOR__H_)
#define OPERATOR__H_

#include <string>
#include "string.h"

#include "SerializeObject.h"
#include "Transactions.h"
#include "ApacheDefines.h"
#include "StructTm.h"
#include "LogInHistory.h"
#include "AudibleAlarm.h"

#define	SIZE_OF_OLD_PWD_LOG		16
/////////////////////////////////////////////////////////////////////////////
// COperator

class COperator : public CSerializeObject
{

CLASS_TYPE_1(COperator, CSerializeObject)

public:

    COperator();
    COperator(const COperator &other);

    virtual ~COperator();

    bool operator==(const COperator&rHnd)const;

    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action=NULL);
    const char* NameOf() const {return "COperator";}
    CSerializeObject* Clone() { return new COperator(*this);}

    void   SetLogin(const std::string  login);
    const std::string&   GetLogin () const;
    void   SetPassword(const std::string  password);
    void   SetPasswordOnly(const std::string  password);
    void   ClearPassword();
    const std::string&   GetPassword () const;
    const std::string& GetNonEncryptedPassword () const;
    void   SetAuthorization(const WORD group);
    WORD   GetAuthorization() const;
	void   RawSerializeXml(CXMLDOMElement*& pFatherNode, bool bSerializePassword=false) const;

	//Federal
	BOOL   IsForceChangePwd() const;
	void   SetForceChangePwd(const BOOL bForceNewPwd);
	std::string GetNewPassword() const;
	void   SetNewPassword(std::string newPwd);
	BOOL   IsEncryptedPassword() const;
	void   EncryptPassword();
	void   EncryptPasswordSHA1();
	void   SetEncPassword(const std::string  encPassword);
	CStructTm	GetLastPwdChanged() const;
	void   SetLastPwdChanged(CStructTm newTime);
	DWORD  GetDaysSinceLastPwdChange() const;
	CStructTm   GetLastLogin() const;
	void   SetLastLogin(CStructTm newTime);
	DWORD  GetDaysSinceLastLogin() const;
	DWORD  GetDaysSinceLastEnabledUser();
	BOOL   IsDisabled() const;
	void   SetDisabled(BOOL bDisabled);
	void   SetDisabledTime(CStructTm time);
	CStructTm GetDisabledTime();
	void   SetEnabledTime(CStructTm time);
	CStructTm GetEnabledTime();
	BOOL   IsLocked() const;
	void   SetLock(BOOL bLock);
	void   SetLockTime(CStructTm time);
	CStructTm GetLockTime(void) const;
	void   UpdatePwdLog();
	BOOL   WasPasswordUsed(std::string newPwd);
	WORD   GetChangePwdFailures() const;
	void   IncrementChangePwdFailures();
	void   ResetChangePwdFailures();
	WORD   GetLoginPwdFailures() const;
	void   IncrementLoginPwdFailures();
	void   ResetLoginPwdFailures();
	void   IncrementCountForceChangePwdNotifications();
	WORD   GetCountForceChangePwdNotifications();
	void   ResetCountForceChangePwdNotifications();
	CStructTm GetFirstFailureTime();
	void   SetFirstFailureTime(CStructTm time);
	std::string GetNewLogin() const;
	void   SetNewLogin(const std::string newLogin);
	std::string GetOldLogin() const;
	void   SetOldLogin(const std::string oldLogin);
	void  SetAudibleAlarm(CAudibleAlarm * audibleAlarm);
	CAudibleAlarm* GetAudibleAlarm();

	void CopyPWDLog(const COperator &other);

	std::string GetLastLoginIPaddress() const;
	void  SetLastLoginIPaddress(const std::string address);
	void  SetFailedLoginInfo(const CStructTm login, const std::string address);
	void  ResetLogInHistory();
	BOOL  IsFirstLogin() const;
	const CLogInHistory& GetLoginHistory() const { return m_loginHistory; }
	CStructTm GetLastFailedLoginInList()const;
	BOOL  IsMachineAccount() const { return m_bMachineAccount; }
	void  SetMachineAccount(const bool bMachineAccount);
	std::string GetFqdnName() const { return m_strFqdnName; }
	void  SetFqdnName(const std::string sFqdnName);

	//****** FOR TDD USE ONLY !!! *****
	void   SetTDDState(BOOL isFederal);
	//****** FOR TDD USE ONLY !!! *****

private:

	DWORD  GetNumOfPassedDays(CStructTm &otherTime) const;

protected:

	std::string m_login;
	std::string m_password;
    WORD 		m_authorizationGroup;

    //Federal
    BOOL 		m_isForceChangePwd;
    std::string m_newPassword;
    std::string m_encryptedPassword;
    BOOL		m_isPwdEncrypted;
    BOOL		m_isDisabled;
    BOOL		m_isLocked;
    CStructTm   m_LastPwdChanged;
    std::string m_pwdLog[SIZE_OF_OLD_PWD_LOG];
    WORD		m_countChangePwdFailures;
    WORD		m_countLoginPwdFailures;
    WORD		m_countForceChangePwdNotifications;
    CStructTm	m_firstLoginPwdFailureTime;
    CStructTm	m_accountDisabledAtTime;
    CStructTm	m_accountLockedAtTime;
    CStructTm	m_accountEnabledAtTime;

    std::string m_oldLogin;
    std::string m_newLogin;

    BOOL		m_isUnderTDD;
    BOOL		m_isUnderFederalTDD;

	CLogInHistory m_loginHistory;
	CAudibleAlarm *m_AudibleAlarm;

    BOOL 		m_bMachineAccount;
    std::string m_strFqdnName;


};

#endif // !defined(OPERATOR__H_)
