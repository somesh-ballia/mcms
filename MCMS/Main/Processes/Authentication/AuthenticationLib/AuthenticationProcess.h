// AuthenticationProcess.h

#ifndef AUTHENTICATION_PROCESS_H_
#define AUTHENTICATION_PROCESS_H_

#include "ProcessBase.h"
#include "McuMngrInternalStructs.h"
#include "McmsAuthentication.h"

class COperatorList;

class CAuthenticationProcess : public CProcessBase  
{
CLASS_TYPE_1(CAuthenticationProcess, CProcessBase)
public:
	friend class CTestAuthenticationProcess;

	CAuthenticationProcess();
	virtual ~CAuthenticationProcess();

	virtual const char* NameOf(void) const;
	virtual eProcessType GetProcessType(void);
	virtual BOOL UsingSockets(void);
	virtual TaskEntryPoint GetManagerEntryPoint(void);
	virtual int GetProcessAddressSpace(void);

	void                   SetAuthenticationStruct(MCMS_AUTHENTICATION_S* authentStruct);
	MCMS_AUTHENTICATION_S* GetAuthenticationStruct() const;

	void SetPasswordFileFlg(BYTE isPasswordFileRemoved);
	BYTE GetPasswordFileFlg();

	COperatorList* GetOperatorList() { return m_pOperatorList; };
	COperatorList* GetLdapOperatorList() { return m_pLdapOperatorList; };
	void SetOperatorList(COperatorList *operatorList);
	void SetLdapOperatorList(COperatorList *ldapOperatorList);
	virtual void SetUpProcess();
	virtual void TearDownProcess();

	virtual DWORD GetMaxTimeForIdle(void) const
	{
	  return 12000;
	}

 	virtual bool RequiresProcessInstanceForUnitTests() {return true;}

protected:
	MCMS_AUTHENTICATION_S* m_pAuthenticationStruct;

private:
	COperatorList* m_pOperatorList;
	COperatorList* m_pLdapOperatorList;
	BYTE m_isPasswordFileRemoved;
};

#endif  // AUTHENTICATION_PROCESS_H_
