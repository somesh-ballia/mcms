#include "LedLightServer.h"
#include "LightCntlApi.h"
#include <sys/msg.h>

//compile comand: g++ -g cppTest.cpp -o gTest
//g++ -o ledSrv main.cpp LedLightServer.cpp LedLightTask.cpp SystemAlarmProcessor.cpp UsbUpgradeProcessor.cpp HotStandbyProcessor.cpp DiagnosticProcessor.cpp sysLightTimerMngr.cpp BaseProcessor.cpp LightCntlApi.cpp -DLED_SRV -lledCntl 
extern int m_process_quit;

void SigRouting(int dunno)
{
	
	switch (dunno) 
	{
		case SIGHUP:
			cout << "Get a signal -- SIGHUP Quit! \n" << endl;
			m_process_quit = 1;
			break;
		case SIGINT:
			cout << "Get a signal -- SIGINT Quit! \n"<< endl;
			m_process_quit = 1;
			break;
		case SIGQUIT:
			cout << "Get a signal -- SIGQUIT Quit! \n"<< endl;
			m_process_quit = 1;
			break;
		case SIGTERM:
			cout << "Get a signal -- SIGTERM Quit! \n"<< endl;
			m_process_quit = 1;			
			break;
		default:
			break;
	}
		return;
}


LedLightServer* LedLightServer::m_pCurrentServer = NULL;

struct LedLightServer::ProcessorTableItem LedLightServer::m_processorTable[]={
	{ "sysAlarm",			 eProc_SysAlarm },
	{ "Diagnostic",			 eProc_Diagnostic },
	{ "UsbUpgrade",			 eProc_UsbUpgrade },
	{ "HotStandby",			 eProc_HotStandby }
};

LedLightServer::LedLightServer()
{
	Init();
	m_pCurrentServer = this;

	m_pLightTimerMngr = new sysLightTimerMngr();
	SysCallQueId = -1;

	m_sys_alarm = new SystemAlarmProcessor();
	m_usb_upgrade = new UsbUpgradeProcessor();
	m_hot_standby = new HotStandbyProcessor();
	m_diagnostic = new DiagnosticProcessor();
	//Do nothing
}

LedLightServer::~LedLightServer()
{
	//Do nothing
	m_pCurrentServer = NULL;
	
	if (m_pLightTimerMngr)
	{
		delete m_pLightTimerMngr;
	}

	if (m_sys_alarm)
	{
		delete m_sys_alarm;
	}

	if (m_usb_upgrade)
	{
		delete m_usb_upgrade;
	}

	if (m_hot_standby)
	{
		delete m_hot_standby;
	}

	if (m_diagnostic)
	{
		delete m_diagnostic;
	}

	LightCntlApi api;
	api.LightClose();
	
}

void LedLightServer::Init()
{
	m_pCurrentServer = NULL;

	m_pLightTimerMngr = NULL;
	SysCallQueId = -1;

	m_sys_alarm = NULL;
	m_usb_upgrade = NULL;
	m_hot_standby = NULL;
	m_diagnostic = NULL;
}

LedLightTask* CreateNewProcess()
{
  return new LedLightServer;
}

int LedLightServer::Run()
{
	//Cmd: lightSrv
	InitTask();
	m_pLightTimerMngr->Run();
	RecvMsg();
	return 0;
}

void LedLightServer::InitTask()
{
	signal(SIGHUP, SigRouting);
    signal(SIGINT, SigRouting);
    signal(SIGQUIT, SigRouting);
    signal(SIGTERM, SigRouting);

	SysCallQueId = -1;
	m_pLightTimerMngr->SetTimerInterval(DEFAULT_TV_SEC,DEFAULT_TV_USEC,DEFAULT_ITV_SEC,DEFAULT_ITV_USEC);

	m_sys_alarm->InitCliTable();
	m_usb_upgrade->InitCliTable();
	m_hot_standby->InitCliTable();
	m_diagnostic->InitCliTable();

	LightCntlApi api;
	api.LightInit();

	//other processeor InitCliTable
}

int LedLightServer::GetProcId(string &procName)
{
	for(unsigned int i=0; i < (sizeof(m_processorTable) / sizeof(ProcessorTableItem)); i++) 
	{
		if(m_processorTable[i].procName == procName)
		{
			return m_processorTable[i].procId;
		}
	}
	return 8;
}

void LedLightServer::HandleTerminalCommand(std::string &procName, std::string &CmdStr)
{		
	std::list<CLiEntry>::iterator itr;
	bool found = false;
	int procId = LedLightServer::GetProcId(procName);
	switch (procId)
	{
		case eProc_SysAlarm:
			m_sys_alarm->HandleTerminalCommand(CmdStr);
			break;
		case eProc_UsbUpgrade:
			m_usb_upgrade->HandleTerminalCommand(CmdStr);
			break;
		case eProc_HotStandby:
			m_hot_standby->HandleTerminalCommand(CmdStr);
			break;
		case eProc_Diagnostic:
			m_diagnostic->HandleTerminalCommand(CmdStr);
			break;
		default:
			cout << "TO Be Done!!!" << endl;
			break;
	}

	return;
}

int LedLightServer::RecvMsg()
{
	int rc;
	TSysCallReqMsg SysCallReqMsg ,*pSysCallReqMsg;
	TSysCallResMsg SysCallResMsg,*pSysCallResMsg;

	pSysCallReqMsg=&SysCallReqMsg;
	pSysCallResMsg=&SysCallResMsg;
	SysCallQueId = CreateSysCallMsgQue();
	int cmdCount = 0;

	if(SysCallQueId == -1)
	{
		cout << "FAILED TO INIT Q" << endl;
		return -1;
	}

	cout << "Client Server Get syscall Msg Que id: " << SysCallQueId << endl;

	while(1)
	{
		memset(pSysCallResMsg,0,sizeof(TSysCallResMsg));
		memset(pSysCallReqMsg,0,sizeof(TSysCallReqMsg));
		pSysCallReqMsg->msgType = SYS_CALL_REQ_MSG_TYPE;
		rc = msgrcv(SysCallQueId,pSysCallReqMsg,sizeof(TSysCallReqMsg) - 4,SYS_CALL_REQ_MSG_TYPE,0);
		
		if (rc != -1)
		{
			printf("Execute system Cmd[%d]: %s \n",cmdCount,pSysCallReqMsg->msgStr);
			cmdCount++;
            // if the command is marked to run with sudo permissions we will 
            // create a script file which contains the command and run this script file.
            // the script file has sudo permissions in the /etc/sudoers
            //
            // if the command isnt marked to run with sudo permissions we will
            // run it using system command.
            //int procId = 0;
            string tmpProcName = pSysCallReqMsg->procName;
            string tmpCmd = pSysCallReqMsg->msgStr;
			HandleTerminalCommand(tmpProcName,tmpCmd);   
		} // else of if (rc == -1)

		if (Stop())
		{
			printf("Quit from RecvMsg!\n");
			break;
		}
	} // while(1)

	return 0;
}


