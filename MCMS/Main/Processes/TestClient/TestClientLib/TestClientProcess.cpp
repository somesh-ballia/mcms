// TestClientProcess.cpp: implementation of the CTestClientProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestClientProcess.h"
#include "StringsMaps.h"

#include "XmlDefines.h" // REMOVE THIS
#include "StringsLen.h" // REMOVE THIS

extern void TestClientManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CTestClientProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CTestClientProcess::GetManagerEntryPoint()
{
	return TestClientManagerEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTestClientProcess::CTestClientProcess()
{
	m_ping_pong_count = 0;

}

CTestClientProcess::~CTestClientProcess()
{

}

