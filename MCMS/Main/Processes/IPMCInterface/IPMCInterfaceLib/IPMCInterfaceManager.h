// IPMCInterfaceManager.h: interface for the CIPMCInterfaceManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DEMOMANAGER_H__)
#define _DEMOMANAGER_H__

#include "ManagerTask.h"
#include "Macros.h"
#include "Segment.h"
#include "IPMC.h"



void IPMCInterfaceManagerEntryPoint(void* appParam);


class COstrStream;








class CIPMCInterfaceManager : public CManagerTask
{
CLASS_TYPE_1(CIPMCInterfaceManager,CManagerTask )
public:
	CIPMCInterfaceManager();
	virtual ~CIPMCInterfaceManager();

	void ManagerPostInitActionsPoint();

    static    void TimerSignalHandler(int SignalNumber);
	TaskEntryPoint GetMonitorEntryPoint();


	eLedState GetLastReqLedColorState(const eLedColor  color) const;
	void SetLastReqLedColorState(const eLedColor  color,const eLedState  state);

	eLedState GetLastConfigLedColorState(const eLedColor  color) const;
	void SetLastConfigLedColorState(const eLedColor  color,const eLedState  state);



	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS
private:
    void OnInitializeLeds();
    void OnChangeLedState(CSegment *pSeg);
    void OnChangeLedStateTimeout();
    void OnSetWatchdog(CSegment *pSeg);
    void OnSetPostCommand(CSegment *pSeg);
    void OnTriggerWatchdog();
    void OnTurnOffWatchdog();
    void OnSetCPUTemperature(CSegment *pSeg);
    void OnSetHardDriveTemperature(CSegment *pSeg);
    void OnUpgradeVersion(CSegment *pParam);
    void OnCheckIpmcVersionTout(CSegment *pSeg);
    void CheckIPMCVersion(bool bByRequest=FALSE, bool bForceBurn=FALSE);
    BOOL IsUpgradeIPMCMode();// This means we are in UpgradeIPMCMode
    int  IsUpgradeNeeded(DWORD &version_on_chip,
                        DWORD &version_on_build,
                        COstrStream & msg,
                        bool bByRequest=FALSE);
    DWORD 	TranslateChipVersionStringToNumber(char * Version_String);
    STATUS 	GetIPMCVersionFromBuild(DWORD &VersionNumber, bool bByRequest=FALSE);
    BOOL 	IsDiagnosticsMode();
    void 	UpgradeIPMCVersion(bool bByRequest=FALSE, bool bForceBurn=FALSE);
    void 	InitDiagnosticsMode();
    STATUS 	HandleGetVersion(CTerminalCommand &command,std::ostream& answer);
    STATUS  HandleUpgradeVersion(CTerminalCommand & command,std::ostream& answer);
    STATUS  HandleIpmcCommand(CTerminalCommand &command,std::ostream& answer);
    void 	MarkNextSystemReset();
    void 	ResetThroughIPMC();
    STATUS 	HandleBurnVersion(CTerminalCommand &command, std::ostream& answer);
    bool 	IsBurningWhileStartup(bool bByRequest);

    int open_session(int nFd);
    int close_session(int nFd);
    int show_version(int nFd);
    int wait_backup(int nFd);
    int do_restore(int nFd);
    int write_image(unsigned char *data, int target, unsigned long addr,
    				int size, int flag , int nFd);
    int LoadIpmcSoftware(int argc, char **argv);


    int m_count_failures;
    BOOL m_disable_IPMC_usage;

   // eLedColor m_LastConfigLedColor;
   // eLedState m_LastConfigLedState;
    eLedState m_arrayLastConfigLedsState[LAST_LED_COLOR];

   // eLedColor m_LastReqLedColor;
   // eLedState m_LastReqLedState;
    eLedState m_arrayLastReqLedsState[LAST_LED_COLOR];

};

#endif // !defined(_DEMOMANAGER_H__)
