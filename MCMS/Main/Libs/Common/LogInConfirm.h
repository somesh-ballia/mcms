// LogInConfirm.h: interface for the CLogInConfirm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LOGINCONFIRM_H__)
#define _LOGINCONFIRM_H__

#include <string>
#include "CommonStructs.h"
#include "SerializeObject.h"
#include "DefinesGeneral.h"
#include "CardsStructs.h"
#include "ProductType.h"
#include "LogInHistory.h"
#include "AvcSvcCapStruct.h"

#define NUMERIC_ID_ROUTING              0x1
#define API_NUMBER						2000	// for backward compatibility

class CXMLDOMElement;

class CLogInConfirm : public CSerializeObject
{
CLASS_TYPE_1(CLogInConfirm,CSerializeObject )	
public:
	//Constructors
	CLogInConfirm(BYTE bMultipleServices, BYTE bV35JitcSupport);
	virtual ~CLogInConfirm();
	virtual const char* NameOf() const { return "CLogInConfirm";}
	
   	virtual void SerializeXml(CXMLDOMElement*& pXMLResponse) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action = 0);
	CSerializeObject* Clone() {return new CLogInConfirm(m_bMultipleServices, m_bV35JitcSupport);}
	
    void   SetCompatibilityLevel(const WORD level);                 
    WORD   GetCompatibilityLevel() const;                 
    void   SetMCMS_Version(const VERSION_S version);                 
    const  VERSION_S  GetMCMS_Version () const;                 
    void   SetMCU_Version(const VERSION_S version);                 
    const  VERSION_S  GetMCU_Version () const;       
	void   SetApiNumber(const DWORD api_number); 
	DWORD  GetApiNumber() const;
	WORD   GetProductType () const;
	void   SetProductType (const WORD type);
	eProductType	GetLoginConfirmProductType () const;
	void	SetLoginConfirmProductType (const eProductType theType);
	void   SetOperatingSystem(const DWORD os);
	DWORD  GetOperatingSystem() const;
	void   SetAuthorization(const WORD group);                 
    WORD   GetAuthorization() const;    
	DWORD  GetHttpPort() const;
	void   SetHttpPort(DWORD dwPort);
	BYTE   GetJITC_Mode() const;
	const std::string & GetMcmsPrivateDesc() const;
	void   SetMcmsPrivateDesc(const std::string& privateDesc);
	const std::string & GetMcuPrivateDesc() const;
	void   SetMcuPrivateDesc(const std::string& privateDesc);
	const std::string & GetMcuDesc() const;
	void   SetMcuDesc(const std::string& privateDesc);
   	void   SetLoginInfoFlag(const DWORD InfoFlag);                 
    DWORD  GetLoginInfoFlag() const;
    void   SetRmxSystemCardsMode(const eSystemCardsMode curMode); 
	eSystemCardsMode	GetRmxSystemCardsMode() const;
	eSystemCardsMode	GetRmxSystemCardsModeDefault() const;
	eSystemRamSize		GetRmxSystemRamSize() const;

	DWORD   GetNumCpParties() const;
	void    SetNumCpParties(const DWORD numCpParties);
	DWORD   GetNumCopParties() const;
	void    SetNumCopParties(const DWORD numCopParties);
	void    SetAvcSvcCap(AvcSvcCap const & cap);

	DWORD GetMcuToken() const;
	void   SetMasterSlaveState(WORD eCurrentState) {m_eMasterSlaveCurrentState = eCurrentState;};
	WORD   GetMasterSlaveState()const {return m_eMasterSlaveCurrentState;};
	CStructTm    GetLastLogin() const;
	void         SetLastLogin(const CStructTm last_login);
	std::string  GetLastLoginIPaddress() const;
	void         SetLastLoginIPaddress(const std::string address);
	void         SetFailedLoginInfo(const CStructTm last_login, const std::string address);
	void         SetLoginHistory(const CLogInHistory& loginHistory);

	DWORD  GetSessionTimeoutInMinutes() const;
	void 	SetDaysUntilPwdExpires(DWORD days);
	DWORD  	GetDaysUntilPwdExpires();
	DWORD	GetPwdExpirationWarningPeriod();
	void   SetIsAudibleAlarmEnable(WORD bAudibleAlarmEnable);

	BYTE IsMachineAccount() const;
	void SetMachineAccount(const BYTE bMachineAccount);

private:
    int IsPlatformGesherNinja() const;

protected:
  	 // Attributes
    VERSION_S  m_MCMS_Version;     
    VERSION_S  m_MCU_Version;     
    WORD m_compatibilityLevel;  
	DWORD m_apiNumber;
	DWORD m_reserved1;
	DWORD m_reserved2;
	WORD  m_productType;
	DWORD m_numCpParties;
	DWORD m_numCopParties;
	eProductType  m_loginConfirmProductType;
	DWORD m_operating_system;
	WORD  m_authorizationGroup;
	DWORD m_loginInfoFlag;
	DWORD m_HttpPort;
	DWORD m_mcuToken;
	BYTE  m_bJITC_Mode;
	BOOL  m_bLastLoginAttempts;
	DWORD  m_SessionTimeoutInMinutes;
	BYTE  m_bHidePsw;
	BYTE  m_bNetworkSeparation;
	WORD m_bAudibleAlarmEnable;
	eSystemCardsMode	m_rmxSystemCardsMode;
   	eSystemRamSize		m_ramSize;
        BYTE  m_bMultipleServices;
        BYTE  m_bV35JitcSupport;
	BYTE  m_bVideoPreviewEnable;
	//char  m_mcmsPrivateDesc[PRIVATE_VERSION_DESC_LEN + 1];
	//char  m_mcuPrivateDesc[PRIVATE_VERSION_DESC_LEN + 1];
	std::string m_mcmsPrivateDesc;
	std::string m_mcuPrivateDesc;
	std::string m_mcuDesc;
	WORD m_eMasterSlaveCurrentState;

	CLogInHistory 	 m_loginHistory;
	DWORD	m_pwdExpirationWarningPeriod;
	DWORD	m_daysUntilPwdExpires;

	BYTE m_bMachineAccount;
	AvcSvcCap m_avcSvcCap;
	/*Begin:added by Richer for BRIDGE-14278 , 2014.7.29*/
	BOOL m_bSimulationMode;
	/*End:added by Richer for BRIDGE-14278 , 2014.7.29*/
	//eLicenseMode m_LicenseMode;
};

#endif // !defined(_LOGINCONFIRM_H__)
