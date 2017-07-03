#ifndef LEDLIGHTSERVER_H_
#define LEDLIGHTSERVER_H_

#include "LedLightTask.h"
#include "LedLightServer.h"
#include "SystemAlarmProcessor.h"
#include "UsbUpgradeProcessor.h"
#include "HotStandbyProcessor.h"
#include "DiagnosticProcessor.h"
#include "sysLightTimerMngr.h"
#include "commonDate.h"

using namespace std; 

class LedLightServer;

enum eProcId
{
	eProc_SysAlarm,
	eProc_Diagnostic,
	eProc_UsbUpgrade,
	eProc_HotStandby,
	eProc_Max
};

#define DEFAULT_TV_SEC		0
#define DEFAULT_TV_USEC		300000
#define DEFAULT_ITV_SEC		0
#define DEFAULT_ITV_USEC	300000

class LedLightServer: public LedLightTask
{
public:
    LedLightServer();
    virtual ~LedLightServer();

    int            Run();
    int RecvMsg();
    void InitTask();
    void HandleTerminalCommand(std::string &procName, std::string &CmdStr);

    static LedLightServer*     GetCurrentServer(){ return m_pCurrentServer; };

    sysLightTimerMngr*     GetLightTimerMngr(){ return m_pLightTimerMngr; };
    static int     GetProcId(string &procName);
	void Init();

private:
	LedLightServer(const LedLightServer& rhs):LedLightTask()
	{
		Init();
		/* do not create copies until now, implementation  it while needed */ 
	}
	
	LedLightServer& operator=(const LedLightServer&)
	{
		return *this;
	}
	
    sysLightTimerMngr *m_pLightTimerMngr;
    static LedLightServer* m_pCurrentServer;
    int SysCallQueId;
    
    struct ProcessorTableItem {
		string procName;
		eProcId procId;
	};
	static ProcessorTableItem m_processorTable[];

    //std::list<CLiEntry> m_cli_table;
    SystemAlarmProcessor *m_sys_alarm;
    UsbUpgradeProcessor *m_usb_upgrade;
    HotStandbyProcessor *m_hot_standby;
    DiagnosticProcessor *m_diagnostic;    
};

#endif

