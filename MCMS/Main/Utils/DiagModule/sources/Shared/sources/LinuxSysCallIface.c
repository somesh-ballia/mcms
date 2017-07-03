#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "LinuxSysCallIface.h"
#include "Print.h"

static INT32 s_sysCallQid=-1;
static UINT32 s_sysCallmsgInd=1;
INT32 GetSysCallMsgQue(void)
{
	key_t key;
	INT32 qid;
	key = ftok("/",SYS_CALL_KEY);

	qid = msgget(key,IPC_CREAT|S_IRWXU);
	return qid;  					  
}

void InitSysCallMsgQue(void)
{
   s_sysCallQid = GetSysCallMsgQue();
}

/****************************************************************************
* Prototype:        SendSysCall
* Description:      Send system call through process responsible sending
*                   system calls. 
* Return Value:     when waitOnMsg is false, on success return code 1. 
*					when waitOnMsg is true, on success return code is command return code.
*
* 
* Arguments:        pCmdString - string for linux system command
*                   waitOnMsg  - when true sleep until command is completed.  
*****************************************************************************/
INT32 SendSysCall(INT8 *pCmdString,UINT8 waitOnMsg)
{
   static UINT8 fisrtTime = 1;
   TSysCallResMsg SysCallResMsg,*pSysCallResMsg;
   TSysCallReqMsg SysCallReqMsg ,*pSysCallReqMsg;
   INT32 rc;
   
   pSysCallReqMsg=&SysCallReqMsg;
   pSysCallResMsg=&SysCallResMsg;
   if (fisrtTime)
   {
   	 	InitSysCallMsgQue();
   	 	fisrtTime=0;
   }
   if (strlen(pCmdString)> MAX_SYSCALL_MSG_SIZE)
	  return SYSCALL_ERR_STR;
   else if (s_sysCallQid == -1)
	  return SYSCALL_ERR_QUE;
   else
   {
	  memset(pSysCallReqMsg,0,sizeof(TSysCallReqMsg));
	  pSysCallReqMsg->msgType = SYS_CALL_REQ_MSG_TYPE;
	  strncpy(pSysCallReqMsg->msgStr,pCmdString, sizeof(pSysCallReqMsg->msgStr) - 1);
	  pSysCallReqMsg->msgInd = s_sysCallmsgInd++;
	  pSysCallReqMsg->waitOnMsg=waitOnMsg;
	  rc = msgsnd(s_sysCallQid,pSysCallReqMsg,sizeof(TSysCallReqMsg)-4,IPC_NOWAIT);
	  if (rc == -1)
		return SYSCALL_ERR_SEND;

   	  if (waitOnMsg)
	  {
		do
		{
			memset(pSysCallResMsg,0,sizeof(TSysCallResMsg));
			pSysCallReqMsg->msgType=SYS_CALL_RES_MSG_TYPE;
			rc = msgrcv(s_sysCallQid,pSysCallResMsg,sizeof(TSysCallResMsg) - 4,SYS_CALL_RES_MSG_TYPE,0);
	  	}
	  	while ((pSysCallResMsg->msgInd != pSysCallReqMsg->msgInd) && (rc != -1));
	  	
	  	if (rc != -1)											   
			return pSysCallResMsg->result;
		else return SYSCALL_ERR_RCV;
	  }
	  else return 1;
   }
}
