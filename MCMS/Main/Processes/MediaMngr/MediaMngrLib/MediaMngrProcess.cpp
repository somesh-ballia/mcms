// MediaMngrProcess.cpp: implementation of the CMediaMngrProcess class.
//
//////////////////////////////////////////////////////////////////////
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include "MediaMngrProcess.h"
#include "SystemFunctions.h"

extern void MediaMngrManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CMediaMngrProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CMediaMngrProcess::GetManagerEntryPoint()
{
	return MediaMngrManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CMediaMngrProcess::CMediaMngrProcess()
{
    struct rlimit rlim;
    getrlimit (RLIMIT_STACK,&rlim);
    rlim.rlim_cur = 1024*256;
    setrlimit(RLIMIT_STACK,&rlim);

}

//////////////////////////////////////////////////////////////////////
CMediaMngrProcess::~CMediaMngrProcess()
{

}

