#if !defined(_CMcuMngrSNMPTask__)
#define _CMcuMngrSNMPTask__

#include "ProcessBase.h"
#include "SNMPTask.h"
#include "NetSnmpIncludes.h"
//#include "DefinesGeneral.h"

class CMcuState;
class CMcuMngrSNMPTask : public CSNMPTask  
{
CLASS_TYPE_1(CMcuMngrSNMPTask,CSNMPTask )
public:
	friend class CTestMcuMngrSNMPTask;

	CMcuMngrSNMPTask();
	virtual const char* NameOf() const { return "CMcuMngrSNMPTask";}
	virtual ~CMcuMngrSNMPTask();
	virtual eProcessType GetProcessType() {return eProcessMcuMngr;}
	virtual BOOL UsingSockets() {return NO;}
    virtual void RegisterOID() const;
    

    
};

//m_pProcess->GetMcuStateObject()->GetMcuState

#endif //_CMcuMngrSNMPTask__
