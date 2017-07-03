// MediaMngrProcess.h

#ifndef MEDIA_MNGR_PROCESS_H_
#define MEDIA_MNGR_PROCESS_H_

#include "ProcessBase.h"

class CMediaMngrProcess : public CProcessBase  
{
CLASS_TYPE_1(CMediaMngrProcess, CProcessBase)
public:
	friend class CTestMediaMngrProcess;

	CMediaMngrProcess();
	virtual ~CMediaMngrProcess();
	const char * NameOf() const {return "CMediaMngrProcess";}
	virtual eProcessType GetProcessType() {return eProcessMediaMngr;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
    virtual int GetProcessAddressSpace() {return 64 * 1024 * 1024;}
};

#endif  // MEDIA_MNGR_PROCESS_H_
