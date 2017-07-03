// InstallerManager.h: interface for the CInstallerManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_INSTALLERMANAGER_H__)
#define _INSTALLERMANAGER_H__

#include "ManagerTask.h"
#include "Macros.h"
#include "KeyCode.h"
#include "McuMngrInternalStructs.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"

class CKeyCode;
class CInstallerProcess;

void InstallerManagerEntryPoint(void* appParam);

struct TRAILER_INFO_S
{
	std::string 	TrailerVersion;
	std::string		FirmwareVersion;
	std::string     FirmwareDate;
	std::string		KernelSize;
	std::string		KernelMD5SUM;
	std::string     SquashFsSize;
	std::string     SquashFsMD5SUM;
	std::string		NoTrailerSize;
	std::string		NoTrailerMD5SUM;
	std::string 	SubTrailerVersion;
	std::string		EmbSize;
	std::string		EmbMd5SUM;
	std::string     NoTrailerSHA1SUM;
	std::string     Sub2TrailerVersion;
	std::string		NewSqFsSize;
	std::string		NewSqFsMD5SUM;
	std::string		ProductType;

};

/* in DefinesGeneral.h
enum eBackupRestoreAction
{
	eIdle =	0,
	eBackup,
	eRestore
};*/

//OPCODE progressStatus[] =
//{
//	STATUS_OK,
//	STATUS_BACKUP_IN_PROGRESS,
//	STATUS_RESTORE_IN_PROGRESS
//};

typedef struct
{
	int		stepTime[eFPGAActionMaxNum];
	int		oneStepTime[eFPGAActionMaxNum];
	volatile eFPGAUpgradeAction	stepIndex;
	int		currentTime;
	int		totalTime;
	int		retryMax;
} FPGA_UPGRADE_S;

class CInstallerManager : public CManagerTask
{
CLASS_TYPE_1(CInstallerManager,CManagerTask )
public:
	CInstallerManager();
//	CInstallerManager(const CInstallerManager &other);
//	CInstallerManager& operator=(const CInstallerManager& other);
	virtual ~CInstallerManager();


	TaskEntryPoint GetMonitorEntryPoint();

	// Transaction Map
	STATUS HandleReceiveVersion(CRequest *pRequest);
    STATUS HandleFinishReceiveVersion(CRequest *pRequest);
    STATUS HandleUpdateKeyCode(CRequest *pRequest);
    STATUS HandleInstallPreviousVersion(CRequest *pRequest);

//	STATUS		Treat_X_KeyCodeUpdate(CKeyCode* pNewKeyCode);
//	STATUS		Treat_U_KeyCodeUpdate(CKeyCode* pNewKeyCode);
	STATUS		TreatKeyCodeUpdate_notVersionUpgrade(CKeyCode* pNewKeyCode);
	STATUS		TreatKeyCodeUpdate_versionUpgrade(CKeyCode* pNewKeyCode);
	STATUS		CheckNew_U_KeyCodeVsOriginalKeycode(VERSION_S newKcVersion);
//	STATUS		CheckKeyCodeVersionVsMcuVersion(VERSION_S verFromKc, VERSION_S mcuVer, BOOL isInstallVersion);
	STATUS		CheckCurrentVersionVsMcuVersion(VERSION_S mcuVer, BOOL isInstallVersion);
	STATUS 		CompareTrailerVersions(std::string newTrailerVersion);
	STATUS		AcceptVersion(const std::string & new_version_name, BOOL isInstallVersion);
	void		SetVersionInCfsParamsStruct(VERSION_S ver);
	static VERSION_S	ConvertStringToVersionStruct(const string & versionStr);
	void		SendUpdateCfsKeyCodeReqToMplApi(CSmallString keyCode, VERSION_S mcuVersion);
	void		SendUpdatedKeyCodeToMcuMngr(CKeyCode* pkeyCode);

	void	AskMcuMngrForCfsParams();
	void	OnMcuMngrInstallerCfsParamsInd(CSegment* pSeg);
    STATUS	OnInstallerFinishInstallation(CSegment *pSeg);
    STATUS 	ValidateProductFamily(TRAILER_INFO_S &trailerInfo); // for Call Generator - ValidateProductFamily
    void	OnMcuMngrInitKeycodeFailureInd(CSegment* pSeg);
    void    OnIPMCUpgradeProgress(CSegment* pSeg);
    void 	OnIPMCUpgradeNotNeeded(CSegment* pSeg);
    void    OnInstallationSoftwareProgress(CSegment* pSeg);
    void    OnInstallationSoftwareDone(CSegment* pSeg);
    void    OnInstallationIpmcProgress(CSegment* pSeg);
    void    OnInstallationIpmcFromCardsFinished(CSegment* pSeg);
    void	OnCheckCpuIPMCProgress(CSegment* pSeg);
    void 	OnResetTimer(CSegment* pSeg);
    void 	OnTimeoutCardsBeginInstall(CSegment* pSeg);
    void 	OnTurnOffLocalTracerTimer(CSegment* pSeg);
    void    OnInstallerValidateKeyCode(CSegment *pSeg);
    void    SendUpdateKeyCodeStatusToInstallerMonitor(STATUS status , std::string  description);
	STATUS HandleTerminalCheckNewVersionValidity(CTerminalCommand & command, std::ostream& answer);


private:
    virtual void ManagerStartupActionsPoint();

    STATUS OnInstallationTimeoutInstall();
    STATUS OnInstallationTimeoutReady();
    STATUS GetTrailerInfo(TRAILER_INFO_S& pTrailerInfoStruct,std::string version_name);
    STATUS CheckSHA1(const std::string& stamp,
                     const std::string& fname,
                     const std::string& size);
    STATUS CalculateFileSHA1Value(const std::string& fname,
                                  const std::string& size,
                                  std::string& out);
    STATUS CheckNewVersionMD5Validity();
    STATUS DistributeNewVersionToAll();
    int 	ReadUnlockedSemaphore(void *ptr, size_t size, size_t nmemb, FILE *stream);
    void 	SendEventToAuditor(const char* pszMessage);
    STATUS	SendConfBlockToConfParty(BYTE isBlock, WORD timeOutInSec = 15);
	STATUS CheckProductTypeValidity(std::string & productTypeList, std::string & product);
	STATUS StartLocalTracer(bool bStart);

	bool                m_isUnderTDD;

	MCMS_AUTHENTICATION_S* m_pAuthenticationStruct;

protected:
	// terminal commands
	STATUS HandleTerminalLicensingInfo(CTerminalCommand & command,std::ostream& answer);

	STATUS SendMsgToBackupRestore(OPCODE action);
	void OnBackupRestoreStartInd(CSegment *pSeg);
	void OnBackupRestoreFinishInd();
	STATUS OnBackupRestoreProgressTimeout();

	STATUS	VerifyTrailerInfoStruct_V_1_0_0(TRAILER_INFO_S TrailerInfoStruct);
	STATUS 	VerifyTrailerInfoStruct_V_1_0_1(TRAILER_INFO_S TrailerInfoStruct);
	STATUS 	VerifyTrailerInfoStruct_V_1_0_2(TRAILER_INFO_S TrailerInfoStruct);
	STATUS 	VerifyTrailerInfoStruct_V_1_0_3(TRAILER_INFO_S TrailerInfoStruct);
	STATUS	VerifySubTrailerInfoStruct_V_1_1_0(TRAILER_INFO_S TrailerInfoStruct);
	STATUS	VerifySub2TrailerInfoStruct_V_1_1_0(TRAILER_INFO_S TrailerInfoStruct);
	
	void	ParseTrailer_V_1_0_0(WORD &InfoLine, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct);
	void	ParseTrailer_V_1_0_1(WORD &InfoLine, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct);
	void	ParseTrailer_V_1_0_2(WORD &InfoLine, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct);
	void	ParseTrailer_V_1_0_3(WORD &InfoLine, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct);
	void	ParseSubTrailer_V_1_1_0(WORD &InfoLine, WORD &InfoSubLine, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct);
	void	ParseSub2Trailer_V_1_1_0(WORD &InfoLine, WORD &InfoSub2Line, char *pRowContent, WORD &EndOfInfo, char *pTrailerInfo, TRAILER_INFO_S& TrailerInfoStruct);
	
	void	InstallationFinished();
	void 	CalcIPMCBurnProgress(int cards, int cpu);
    void    DumpScriptsLogs();

    void    OnGetCnrlTypeFromCards(CSegment* pSeg);
    void    OnMcuMngrAuthenticationStruct(CSegment* pSeg);
    STATUS  preventSWDowngradeInNewCtrl(string firmVersion);
    void    SendAuthenticationStructReqToMcuMngr();
	
	//NINJA: FPGA Upgrade
	STATUS FPGAImageHeaderCompare(std::string szFPGAImgPath);
	void TestNinjaFPGAAndAlarm();
	int GetFPGARetryNumber();
	void GesherInstallationSoftwareDone();
	STATUS BackupFPGAImg(std::string & szTargetImgPath, std::string szSrcImgPath);
	STATUS SendFPGAUpgradeToConfig();
	void OnFPGAUpgradeProgress(CSegment* pSeg);
	void OnFPGAChechAndBurningTimer(CSegment* pSeg);
	void LedUpgradeInProgress();
	void LedUpgradeComplete();
	void LedUpgradeFailed();

	void CheckMultipleServicesBit(CKeyCode* pkeyCode);

	FPGA_UPGRADE_S m_fpgaUpgradeStatus;
	int      m_fpgaRetryCount;
	std::string  m_fpgaImgPath;
	
	eBackupRestoreAction m_backupRestoreInProgress;

	CInstallerProcess*	m_pProcess;
	eProductType m_productType;
	INSTALLER_CFS_S* 	m_pCfsParamsStruct;
	bool				m_isCfsParamsReceived;
	bool				m_isKeyCodeFailure;
    bool                m_installOperationFinished;
    bool				m_installFlowFinished;
    bool				m_CpuIPMCUpgradeNotNeeded;
    int					m_lastMinIpmcProgressCpu;
    int					m_lastMinIpmcProgressCards;
    bool                m_isNewVersionBlockedInMPMRX;
    bool                m_handleReceiveVersionStarted;
    bool                m_onGoingConf;

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

};


#endif // !defined(_INSTALLERMANAGER_H__)
