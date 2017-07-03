// AuthenticationManager.h

#ifndef AUTHENTICATION_MANAGER_H_
#define AUTHENTICATION_MANAGER_H_

#include "ManagerTask.h"
#include "AuthenticationProcess.h"
#include "AuthenticationStructs.h"
#include "ObjString.h"
#include "LogInRequest.h"
#include "LogInConfirm.h"
#include <valarray>

class COperator;
class CChangePassword;
class CObjString;

class CRequestQueue
{
public:
	CRequestQueue();
	CRequestQueue(WORD id, CRequest* pRequest);
	~CRequestQueue();
	CRequest* GetRequest();
	COsQueue* GetQueue();
private:
	WORD 	  m_Id;
	CRequest* m_pRequest;
};

class CRequestQueueList
{
public:
	CRequestQueueList();
	~CRequestQueueList();
	int AddRequest(CRequest* req_queue);
	void RemoveRequest(DWORD dwRequestId);
	CRequestQueue* GetRequestQueue(int index);
protected:
	int m_nCount;
	std::valarray<CRequestQueue*> *m_pRequestQueueArray;
};

enum eUpdateUserType
{
	eDeleteUser,
	eMaxNumOfUpdateUserType
};

class CAuthenticationManager : public CManagerTask
{
CLASS_TYPE_1(CAuthenticationManager, CManagerTask)
public:
	CAuthenticationManager();
	virtual ~CAuthenticationManager();

	const char* NameOf(void) const {return "CAuthenticationManager";}

	TaskEntryPoint GetMonitorEntryPoint();

	void  OnMcuMngrAuthenticationStruct(CSegment* pSeg);
	void  OnMcuMngrAuthenticationSuccess();
	void  OnMcuMngrTimeSet();
	void  OnMultipleServicesInd(CSegment* pParam);
	void  OnMcuMngrSecurityModeInd(CSegment* pParam);	
	void	OnSwitchLdapLoginRequest(CSegment* rspMsg);

	STATUS HandleOperLogin(CRequest *pRequest);
	STATUS HandleChangePassword(CRequest *pRequest);
	STATUS HandleDeleteOperator(CRequest *pRequest);
	STATUS HandleDisableOperator(CRequest *pRequest);
	STATUS HandleUnlockOperator(CRequest *pRequest);
	STATUS HandleNewOperator(CRequest *pRequest);
	STATUS HandleRenameOperator(CRequest *pRequest);
	STATUS SetOperatorAudibleALarm(CRequest *pRequest);
	
	
	void  SendSuperUsersListToMplApi();
	void  SendDefaultUserListToMplApi();
	
	void SendMplApiToSwitch(DWORD msgId, char* msg, DWORD msgSize, const string& msgStr);
	void SendUserLdapLoginRespToSwitch(DWORD requestId, BYTE allowLogin, DWORD authorizationGroup);
	
	STATUS  InitUserStruct(USER_S *pUserStruct, string newLogin, string newPwd, BOOL isPwdEncrypted, int newAuthorizationGroup);
	void  AddUserToUsersListString(CLargeString &usersListToMplApiStr, USER_S *pUserStruct, int i);
//	void  SendUserToMplApi(int updateType, COperator* pUser);
//	DWORD GetOpcodeByType(int updateType);
	
	int OnExtDBUserLoginConfirm(CSegment* pParam );
	void CreateReqToExternalDB(CXMLDOMElement *pRootNode, COperator* pOperator, int request_id);
	void OnTimerNoExtDBResponse(CSegment* pParam );
	void OnExtDBFailure(CSegment* pParam);
	void OnSetDefaultUserList(CSegment* pParam);
	void OnFailoverReq(CSegment* pSeg);
	void OnDailyTimerTests(CSegment* pParam);
	void OnLockTimerTests(CSegment* pParam);

	//AD:
	void OnLdapModuleAdServerAvailableInd(CSegment* pSeg);
///	void OnLdapLoginResponseTimeout(CSegment* pParam);
///	int OnLdapLoginInd(CSegment* pParam);
	STATUS ValidateUserListForLdapJitc();

	void SendResponse(int status, CRequest* req,  COsQueue* q);

	void ReceiveAdditionalParams(CSegment* pSeg);
	
	void AddDefaultUserAlertIfNeeded();
	WORD  CheckIfAudibleAlarmEnable(const std::string login_name);
	void UpdateHotBackupCurrentType(CLogInRequest* pLoginRequest, CLogInConfirm* pLoginConfirm);

	void OnCheckUserListForLdapReq(CSegment* pSeg);

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

protected:
	STATUS HandleTerminalUserList(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalNewOperator(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalEnableOperator(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalUnlockOperator(CTerminalCommand& command, std::ostream& answer);
	CAuthenticationProcess*   m_pProcess;

private:
	virtual void DeclareStartupConditions();
	virtual void ManagerPostInitActionsPoint();
	virtual void ManagerStartupActionsPoint();

    STATUS DeleteOperator(COperator* pOperator, bool isDeleteFromInternalDB);
    STATUS DisableEnableOperator(COperator* pOperator);
    STATUS DeleteAdminFromOS(COperator* pOperator);
    STATUS DeleteOperatorFromDB(COperator* pOperator);
    STATUS UpdateOperatorPasswordInDB(CChangePassword* pChangePassword, BOOL bForceChangePwd = FALSE);
    STATUS AddOperatorToDB(COperator* pOperator);
    STATUS AddOperatorToOs(COperator* pOperator);
    STATUS UpdateOperatorPasswordInOS(CChangePassword* pChangePassword);
    STATUS IsValidUserName(const string & userName, CObjString & description, BOOL bJitcMode);

    STATUS CheckIfStatusIsValidInJitc(STATUS status);

    //Federal
    STATUS VerifyStrongPassword(CChangePassword ChangePassword, CLargeString &StrDescription, BOOL bChangeBySuper=FALSE);
    STATUS CheckChangeFreq(std::string loginName);
    BOOL   CheckFirstJITCModeStartup();
    STATUS WasPasswordUsed(std::string loginName, std::string newPwd);
    BOOL   IsFederalOn();
    void   CountLoginPasswordFailures(COperator* pOperator);
    void   CountChangePasswordFailures(COperator* pOperator);
    BOOL   DisableAccount(std::string operatorName, std::string description, BOOL bAllowDisableLastAdmin=FALSE);
    void   LockAccount(const std::string& operatorName, const std::string& description);
    BYTE   GetV35JITCSupport();
    STATUS HandleLoginReqWithLdapModule(CRequest *pRequest, COperator& loginOperator);
    void   OnInstallerAuthenticationRemovePasswordFile();

    void ChangeFirstJITCModeIndictionToNo();
    
    STATUS HandleUpdateLdapOperList (STATUS status, const string& 	sLoginName, DWORD authorizationLevel, CLogInConfirm* pLoginConfirm );
    void InformSwitchOnLdapConfiguration();
    BOOL UnlockOperator(COperator *pOperator);
    
	CRequestQueueList *m_pRequestQueueList;
	
	char* m_operName;
	BOOL  m_isAuthenticationStructAlreadyReceived;
	BOOL  m_bIsAdAvailable;
	char* m_pszClientCertificateCN;

	void SendEventToAuditor(
	const string & userName,
    const string & station,
    const string & message,
    const string & action);
	
	BYTE  m_bSystemMultipleServices;
	BYTE  m_bSystemSecurityMode;
	BYTE  m_bRequestPeerCertificate;
	BYTE  m_bV35JITCSupport;
	
	static const std::string JITC_MODE_FIRST_RUN_FILENAME;
	
};

#endif  // AUTHENTICATION_MANAGER_H_
