// EncryptionKeyServerProcess.cpp: implementation of the CEncryptionKeyServerProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "EncryptionKeyServerProcess.h"
#include "SystemFunctions.h"
#include "DHTask.h"
#include "OpcodesMcmsInternal.h"

extern void EncryptionKeyServerManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CEncryptionKeyServerProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CEncryptionKeyServerProcess::GetManagerEntryPoint()
{
	return EncryptionKeyServerManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CEncryptionKeyServerProcess::CEncryptionKeyServerProcess()
{

}

//////////////////////////////////////////////////////////////////////
CEncryptionKeyServerProcess::~CEncryptionKeyServerProcess()
{

}

//////////////////////////////////////////////////////////////////////
void CEncryptionKeyServerProcess::AddExtraOpcodesStrings()
{
	AddOpcodeString(REQ_DH_FOR_NEW_KEY, "REQ_DH_FOR_NEW_KEY");
	AddOpcodeString(DH_IND_ON_NEW_KEY, "DH_IND_ON_NEW_KEY");
}
