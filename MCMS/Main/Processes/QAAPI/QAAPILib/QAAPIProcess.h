// QAAPIProcess.h

#ifndef QA_API_PROCESS_H_
#define QA_API_PROCESS_H_

#include "ProcessBase.h"

class CQAAPIProcess : public CProcessBase  
{
CLASS_TYPE_1(CQAAPIProcess, CProcessBase)
public:
	friend class CTestQAAPIProcess;

	CQAAPIProcess();
	virtual const char* NameOf() const { return "CQAAPIProcess";}
	virtual ~CQAAPIProcess();
	virtual eProcessType GetProcessType() {return eProcessQAAPI;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
  virtual BOOL HasMonitorTask() {return FALSE;}
  virtual int GetProcessAddressSpace() {return 35 * 1024 * 1024;}
  virtual DWORD GetMaxTimeForIdle(void) const
  {
    return 12000;
  }
};

#endif  // QA_API_PROCESS_H_
