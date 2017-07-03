#ifndef LINUXSYSCALLIFACE_H
#define LINUXSYSCALLIFACE_H

#include "DiagDataTypes.h"
#define MAX_SYSCALL_MSG_SIZE 100
#define SYS_CALL_REQ_MSG_TYPE 20
#define SYS_CALL_RES_MSG_TYPE 40
#define SYS_CALL_KEY 400

#define SYSCALL_ERR_STR  -2
#define SYSCALL_ERR_QUE  -3
#define SYSCALL_ERR_SEND -4
#define SYSCALL_ERR_RCV  -5

void SysCallThreadCreate(void);
void* SysCallThread(void *argPtr);
INT32 GetSysCallMsgQue(void);

typedef struct SSysCallReqMsg
{
	UINT32  msgType;
	UINT32  msgInd;
	UINT32  waitOnMsg;
	INT8 msgStr[MAX_SYSCALL_MSG_SIZE];

}TSysCallReqMsg;

typedef struct SSysCallResMsg
{
	UINT32  msgType;
	UINT32  msgInd;
	UINT32  result;
}TSysCallResMsg;

#endif
