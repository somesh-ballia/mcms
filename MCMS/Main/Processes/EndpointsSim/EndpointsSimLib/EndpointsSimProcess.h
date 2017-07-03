// EndpointsSimProcess.h: interface for the CEndpointsSimProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_EndpointsSimPROCESS_H__)
#define _EndpointsSimPROCESS_H__

#include <map>
#include "OsQueue.h"

#include "ProcessBase.h"


enum eEPSimTaskType
{
    eUnknownTaskType,
    eCSTaskType,
    eProxyTaskType,
    eGKTypeTaskType
};

const char* EPSimTaskTypeToStr(eEPSimTaskType type);

class CEndpointsSimProcess : public CProcessBase  
{
CLASS_TYPE_1(CEndpointsSimProcess,CProcessBase )
public:
	friend class CTestEndpointsSimProcess;

	CEndpointsSimProcess();
	virtual const char* NameOf() const { return "CEndpointsSimProcess";}
	virtual ~CEndpointsSimProcess();
	virtual eProcessType GetProcessType() {return eProcessEndpointsSim;}
	virtual TaskEntryPoint GetManagerEntryPoint();

	virtual BOOL UsingSockets() {return YES;}
    virtual BOOL HasWatchDogTask() {return FALSE;}
	virtual void AddExtraStringsToMap();
    virtual BOOL HasMonitorTask() {return FALSE;}
    
    virtual int GetProcessAddressSpace() {return 0;}

    void RegisterQueue(DWORD csID, eEPSimTaskType type, const COsQueue & queue);
    void UnregisterQueue(DWORD csID, eEPSimTaskType type);
    COsQueue RetrieveQueue(DWORD csID, eEPSimTaskType type) const;

private:
    std::map<std::pair<DWORD, eEPSimTaskType>, COsQueue> m_csTasksQueue;
};

#endif // !defined(_EndpointsSimPROCESS_H__)

