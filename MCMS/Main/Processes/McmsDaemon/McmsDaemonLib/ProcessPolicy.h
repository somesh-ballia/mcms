#ifndef PROCESSPOLICY_H_
#define PROCESSPOLICY_H_

#include "PObject.h"
#include "McmsProcesses.h"


enum eFaultedProcessAction
{
    eResetAfterRetryLoading = 0,
    eNoResetAfterRetryLoading,
    eNoRetryLoading,
    eReset,
    
    NumOfFaultProcessPolicies
};
static const char* GetFaultProcessesName(eFaultedProcessAction en)
{
    static char* FaultProcessesNames[] = 
        {
            "Try to reload x n, reset",
            "Try to reload x n, terminate",
            "No retry to reload",
            "Reset"
        };
    
	const char *name = (eResetAfterRetryLoading <= en && en < NumOfFaultProcessPolicies 
						?  
						FaultProcessesNames[en] : "Invalide name");
	return name; 
}


enum eMonitorProcessStatus
{
    eMonitorProcessStatusTerminated,
    eMonitorProcessStatusStartup,
    eMonitorProcessStatusAlive,
    eMonitorProcessWaitForWD
};
static const char* GetProcessStatusName(eMonitorProcessStatus en)
{
    static char* ProcessStatusNames[] = 
        {
            "Terminated",
            "Startup",
            "Alive",
            "Wait for WD"
        };
    const char *name = ((DWORD)en < sizeof(ProcessStatusNames) / sizeof(ProcessStatusNames[0]) 
						?  
						ProcessStatusNames[en] : "Invalide name");
	return name; 
}








class CProcessPolicy : public CPObject
{
CLASS_TYPE_1(CProcessPolicy, CPObject)	
public:
	CProcessPolicy();
	virtual ~CProcessPolicy();

	virtual const char* NameOf() const { return "CProcessPolicy";}
	virtual void Dump(std::ostream&) const;
	
	void UpdateTimes();

	bool IsAlive()const {return m_Status == eMonitorProcessStatusAlive;}
	bool IsTerminated()const {return m_Status == eMonitorProcessStatusTerminated;}
    bool IsInStartup()const{return m_Status == eMonitorProcessStatusStartup;}
    
	bool IsAnotherRetry()const {return m_NumRetry < m_MaxNumRetry;}
	eFaultedProcessAction GetPolicy()const {return m_PolicyType;}

    DWORD GetAbsCrashCounter()const {return m_AbsCrashCounter;}
    DWORD GetCrashCounter()const {return m_NumRetry;}
    
    eMonitorProcessStatus GetStatus()const{return m_Status;}
    void SetStatus(eMonitorProcessStatus status);
    
	void SetTimeFirstUp();
	void SetTimeUp();
	void SetPolicy(eFaultedProcessAction policy){m_PolicyType = policy;}
	void Terminate()							{m_Status = eMonitorProcessStatusTerminated;}
		
	void SetDefaults(eProcessType processType = eProcessTypeInvalid);
	void UpdateNumOfRetry();

    DWORD GetNumLaunch()const{return m_NumOfLaunch;}
    void IncNumLaunch(){m_NumOfLaunch++;}

    DWORD GetMaxNumLaunch()const{return m_MaxNumRetry;}
    eProcessType GetProcessType()const{return m_ProcessType;}
    
    DWORD GetMaxWaitWDNumber(eProcessType process);
	bool  IsAdditionalWDRetry(eProcessType process);
    
	static eFaultedProcessAction GetFaultedPolicy(eProcessType);
	
private:
	// disabled
	CProcessPolicy(const CProcessPolicy&);
	CProcessPolicy&operator=(const CProcessPolicy&);

    
    const DWORD m_MaxNumRetry;

    eMonitorProcessStatus m_Status;
	DWORD 	m_TimeUp;
	DWORD 	m_TimeLastUp;
	DWORD 	m_NumRetry;
    DWORD   m_AbsCrashCounter;
	DWORD   m_KeepAliveCnt;
    DWORD   m_NumOfLaunch;
    DWORD   m_NumWaitWDRetry;
    
    eProcessType m_ProcessType;
    eFaultedProcessAction m_PolicyType;
};

#endif /*PROCESSPOLICY_H_*/
