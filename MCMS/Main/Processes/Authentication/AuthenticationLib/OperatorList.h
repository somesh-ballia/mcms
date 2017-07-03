// OperatorList.h

#ifndef OPERATOR_LIST_H_
#define OPERATOR_LIST_H_

#include <string>

#include "Operator.h"
#include "SharedDefines.h"

#define DEFAULT_USER_LOGIN			"POLYCOM"
#define DEFAULT_USER_PASSWORD		"POLYCOM"
#define DEFAULT_USER_AUTHORIZATION	SUPER

#define NOT_FOUND	-1

#define OLD_OPERATOR_DB_FILE	"Cfg/OperatorDB.xml"
#define ENC_OPERATOR_DB_FILE	"Cfg/EncOperatorDB.xml"
//#define SHA256_ENC_OPERATOR_DB_FILE	"Cfg/SHA256EncOperatorDB.xml"
#define LDAP_OPERATOR_DB_FILE	"Cfg/LdapOperatorDB.xml"

class CXMLDOMElement;

// VNGR-11252
// Yael & Ido
// This class is used to separate the Operator List indexes when accessing First and Next operators.
// Before this change, the FirstOperator & NextOperator used a global variable and it caused bugs, for example:
// Call FirstOperator() to get the first, call NextOperator() to get the next, then (accidently) someone called FirstOperator() again
// and destroys your loop.
////////////////////////////////////////////////////////////////////////////////////////////////
class COperatorListIterator
{
public:
	COperatorListIterator(COperator** operList)
	{
		m_pOperators = operList;
		m_nIndex = 0;
	}

	COperator*  FirstOperator()
	{
		m_nIndex = 0;
	    return m_pOperators[m_nIndex];
	}

	COperator*  NextOperator()
	{
		m_nIndex++;
	    COperator *pOperator = ( m_nIndex < MAX_OPERATORS_IN_MCU
	                             ?
	                             m_pOperators[m_nIndex] : NULL );
	    return pOperator;
	}
private:
	COperator** m_pOperators;
	int m_nIndex;
};

class COperatorList
{
public:
  static STATUS CheckIfAtLeast4CharChanged(const char* old_password,
                                           const char* new_password);

  COperatorList(BYTE bIsFirstJitc, BOOL isLdap);
  virtual ~COperatorList();

  void Add(COperator &other);
  STATUS UpdatePassword(std::string strLogin,
                        std::string strNewPassword,
                        BOOL bForceChangePwd = FALSE);
  void Cancel(const char* login);
  int FindOperator(COperator &oper); // on return fills all the operator fields
  int FindLogin(const char* login) const;
  int FindCaseSensitiveLogin(const char* login) const;
  int NumberOfAdministrator();
  int NumberOfDisabledAdministrator();
  COperator* GetOperatorById(const int idx);
  COperator* GetOperatorByLogin(const std::string login);
  STATUS CheckPassword(COperator* const pOper);
  int IsValidToChangePassword(std::string strLogin,
                              std::string strOldPassword,
                              int &authorization);
  static std::string GetCustomUserLogin();
  static std::string GetCustomUserPasswd();
  STATUS IsValidToAddUser(COperator& other, BOOL bIsAdAvailable = FALSE);
  STATUS IsValidToDeleteUser(std::string login, int& authorization);
  STATUS IsValidToDisableUser(const char* login);
  BOOL IsLastEnabledAdministrator(std::string loginName);
  std::string GetLastAdministrator();

  DWORD GetUpdateCounter() const;
  void IncreaseUpdateCounter();
  virtual void SerializeXml(CXMLDOMElement*& pActionNode, DWORD dwObjectToken,WORD nReqAuthorization=ANONYMOUS) const;
  int DeSerializeXml(CXMLDOMElement *pOperListNode,
                     char* pszError,
                     const char* action = NULL);
  WORD GetCount();
  std::string WriteUserList();

  bool GetIsListCorrupted() const;
  bool GetIsListDefault() const
  {
    return m_isListDefault;
  }
  bool GetIsDefaultUserExists() const;
  void UpdateIsDefaultUserExists();

  void Clear(COperator *defaultOperator);

  void CheckAndCancelInactiveAccounts();
  void CheckExpiredPasswords();
  void CheckLockedAccounts();
  void MarkAllOperatorsAsNewAccounts();
  BOOL WasPasswordUsed(std::string loginName, std::string newPwd);

  WORD GetChangePwdFailures(const std::string loginName);
  void IncrementChangePwdFailures(const std::string loginName);
  void ResetChangePwdFailures(const std::string loginName);
  WORD GetLoginPwdFailures(const std::string loginName);
  void IncrementLoginPwdFailures(const std::string loginName);
  void ResetLoginPwdFailures(const std::string loginName);
  WORD GetCountForceChangePwdNotifications(const std::string loginName);
  void IncrementCountForceChangePwdNotifications(const std::string loginName);
  void ResetCountForceChangePwdNotifications(const std::string loginName);
  void ResetLogInHistory(const std::string loginName);
  void SetFailedLoginInfo(COperator* pOperInDB, const CStructTm login,
      const std::string address);
  void SetFirstFailureTime(const std::string loginName);
  BOOL CheckUserDisabled(const std::string loginName);
  BOOL CheckUserLocked(const std::string loginName);
  BOOL DisableUser(const std::string loginName, BOOL bAllowDisableLastAdmin =
      FALSE);
  BOOL LockUser(const std::string& loginName);
  void AuditDisableUser(const std::string description);
  void AuditLockUser(const std::string description);
  void EnableUser(const std::string loginName);
  void UnlockUser(const std::string loginName);

  BOOL IsFederalOn() const;
  STATUS CheckChangeFreq(std::string strLogin) const;

  BOOL IsSupportNameExistInTheOperatorList();
  void SaveToFileDB();

  BOOL IsLdapOperatorList()
  {
    return m_isLdapOperatorList;
  }
  void DeleteUserWithOldestLoginRecord();

  //****** FOR TDD USE ONLY !!! *****
  void SetTDDState(BOOL isFederal);
  //****** FOR TDD USE ONLY !!! *****

private:
  bool IsLastAdministrator(const COperator& admin) const;

  void LoadFromFileDB(BYTE bIsFirstJitc = FALSE);
  STATUS CreateDefaultDBFile(void);
  void AddDefaultOperatorOnFailure();
  DWORD GetNumOfPassedHours(CStructTm &otherTime) const;

  WORD m_nCount;
  WORD m_nIterIndex;
  DWORD m_updateCounter;
  bool m_isListCorrupted;
  bool m_isListDefault;
  bool m_isDefaultUserExists;

  COperator* m_pOperators[MAX_OPERATORS_IN_MCU];

  BOOL m_isUnderTDD;
  BOOL m_isUnderFederalTDD;
  BOOL m_isLdapOperatorList;

  DISALLOW_COPY_AND_ASSIGN(COperatorList);
};

#endif  // OPERATOR_LIST_H_
