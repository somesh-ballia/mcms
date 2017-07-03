#include "LedLightTask.h"
#include <sys/msg.h>
#include <sys/types.h> 
//compile comand: g++ -g cppTest.cpp -o gTest
//g++  cppTest.cpp -o cTest
int m_process_quit = 0;

LedLightTask::LedLightTask()
{
  m_Argv = NULL;
  m_Argc = 0;
}

LedLightTask::~LedLightTask()
{
  //do nothing
}

int LedLightTask::Run()
{
	//do nothing
	return -1;
}

int LedLightTask::Stop()
{
	//do nothing
	return m_process_quit;
}


int LedLightTask::GetSysCallMsgQue(void)
{
	key_t key;
	int qid;
	key = ftok(FIFO_SERVER,SYS_CALL_KEY);

	qid = msgget(key,0);
	return qid;
}

int LedLightTask::CreateSysCallMsgQue(void)
{
	key_t key;
	int qid;
	key = ftok(FIFO_SERVER,SYS_CALL_KEY);

	qid = msgget(key,IPC_CREAT|S_IRWXU);
	return qid;
}


