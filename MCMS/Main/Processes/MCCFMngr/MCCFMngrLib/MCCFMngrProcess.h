#ifndef MCCFMNGRPROCESS_H__
#define MCCFMNGRPROCESS_H__

//////////////////////////////////////////////////////////////////////
#include "ProcessBase.h"

//////////////////////////////////////////////////////////////////////
void MCCFMngrManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
class CMCCFMngrProcess : public CProcessBase
{
	friend class CTestMCCFMngrProcess;

	CLASS_TYPE_1(CMCCFMngrProcess, CProcessBase)

	virtual const char* NameOf() const
	{ return "MCCF Manager"; }

public:

	CMCCFMngrProcess() {}
	virtual ~CMCCFMngrProcess() {}

private:

	virtual eProcessType GetProcessType()
	{ return eProcessMCCFMngr; }

	virtual BOOL UsingSockets()
	{ return true; }

	virtual TaskEntryPoint GetManagerEntryPoint()
	{ return MCCFMngrManagerEntryPoint; }

	virtual int GetProcessAddressSpace()
	{ return 96 * 1024 * 1024; }

	virtual BOOL HasSyncDispathcerTask()
	{ return false; }

	virtual BOOL HasMonitorTask()
	{ return false; }
};

//////////////////////////////////////////////////////////////////////
#endif // MCCFMNGRPROCESS_H__
