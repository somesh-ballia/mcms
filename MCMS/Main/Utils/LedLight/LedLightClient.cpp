#include "LedLightClient.h"
#include <sys/ipc.h>
#include <sys/msg.h>

//compile comand: g++ -g cppTest.cpp -o gTest
//g++ -o ledCli main.cpp LedLightClient.cpp LedLightTask.cpp BaseProcessor.cpp -DLED_CLI
LedLightClient::LedLightClient()
{
	SysCallQueId = -1;
	m_cmdStr = "";
	s_sysCallmsgInd = 1;

	//Do nothing
}

LedLightTask* CreateNewProcess()
{
  return new LedLightClient;
}

LedLightClient::~LedLightClient()
{
	//Do nothing
}

int LedLightClient::Run()
{
	//Cmd: ledApp client  processor status
	char** argv = (char**) GetArgv();
	
	int argc = GetArgc();
	if (argc < 1)
	{
		cout << "Error. " << endl;
		cout << "usage: LightCli [procName] [status-info]"  << endl;
		return 1;
	}	
	SetCommandLine();
	SendMsg();
	return 0;
}

void LedLightClient::SetCommandLine()
{
	char** argv = (char**)GetArgv();
	int argc = GetArgc();
	m_procName += argv[1];
	m_cmdStr += argv[2];

}

int LedLightClient::SendMsg()
{
	int rc = -2;
	unsigned int  numOfRcv = 0;
	unsigned int  numOfSend = 0;
	int result = -1;
	unsigned int  unMaxNumOfRcv = 20;

	string pCmdString = GetCommandLine();
	string pProcNameStr = GetProcName();
	if (pCmdString.size() >= MAX_SYSCALL_MSG_SIZE)
	{
		cout << "LedLightClient::SendMsg: The Command is larger than MAX_SIZE. Exit!" << endl;
		return SYSCALL_ERR_STR;
	}
	else if ((SysCallQueId = GetSysCallMsgQue()) == -1)
	{
		cout << "LedLightClient::SendMsg: Get SysCall MsgQue Failed. Exit!" << endl;
		return SYSCALL_ERR_QUE;
	}
   else
	{
		cout << "LedLightClient::SysCallQueId = " <<  SysCallQueId << endl;
		TSysCallResMsg SysCallResMsg,*pSysCallResMsg;
		TSysCallReqMsg SysCallReqMsg ,*pSysCallReqMsg;

		pSysCallReqMsg=&SysCallReqMsg;
		pSysCallResMsg=&SysCallResMsg;

		memset(pSysCallReqMsg,0,sizeof(TSysCallReqMsg));
		pSysCallReqMsg->msgType = SYS_CALL_REQ_MSG_TYPE;
		strncpy(pSysCallReqMsg->procName,pProcNameStr.c_str(),MAX_PROC_NAME_SIZE - 1);
		strncpy(pSysCallReqMsg->msgStr,pCmdString.c_str(),MAX_SYSCALL_MSG_SIZE - 1);
		pSysCallReqMsg->msgInd = s_sysCallmsgInd++;

		memset(pSysCallResMsg,0,sizeof(TSysCallResMsg));

		rc = msgsnd(SysCallQueId,pSysCallReqMsg,sizeof(TSysCallReqMsg)-4,IPC_NOWAIT);
		if (rc == -1)
		{
			cout << "LedLightClient::SendMsg: msgsnd Failed : SysCallQueId = " << SysCallQueId << endl;
			return SYSCALL_ERR_SEND;
		}
		cout << "Send system Cmd[" << s_sysCallmsgInd << "]: " << pCmdString.c_str() << endl;

		return 1;
	}

}
