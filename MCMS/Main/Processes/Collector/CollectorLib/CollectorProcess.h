// CollectorProcess.h: interface for the CCollectorProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_COLLECTOR_PROCESS_H__)
#define _COLLECTOR_PROCESS_H__

#include "ProcessBase.h"
#include "CollectorUnit.h"
#include "CollectorInfo.h"

typedef enum {
	eCollectingStatus_ready	= 0,
	eCollectingStatus_collecting,
	eCollectingStatus_aborting

}eCollectingStatus;

class CInfoTimeInterval;
class CCollectorProcess : public CProcessBase  
{
CLASS_TYPE_1(CCollectorProcess,CProcessBase )
public:
	friend class CTestCollectorProcess;

	CCollectorProcess();
	virtual ~CCollectorProcess();
	virtual eProcessType GetProcessType() {return eProcessCollector;}
	virtual BOOL UsingSockets() {return NO;}
    virtual BOOL HasMonitorTask() {return TRUE;}
    virtual BOOL HasDispatcherTask() {return TRUE;}
	virtual TaskEntryPoint GetManagerEntryPoint();
	virtual void AddExtraStringsToMap();

	void SetCollectingStatus(eCollectingStatus status);
	eCollectingStatus GetCollectingStatus();

	void SetCollectingFileName(string file_name);
	string GetCollectingFileName();

	void SetCollectingInfo(CInfoTimeInterval* pInfoTimeInterval);
	CInfoTimeInterval* GetCollectingInfo();

	void SendCollectingToMcuMngr(DWORD isCollecting);

	BOOL GetCollectFilesStarted() { return m_bCollectFilesStarted;}
	void SetCollectFilesStarted(BOOL bCollectFilesStarted) {m_bCollectFilesStarted = bCollectFilesStarted;}

	void HandleAbort();
	void AbortCreateXml();

private:
	void RestartCollectingDetails();

	eCollectingStatus m_CollectingStatus;

	string m_CollectingFileName;

	CInfoTimeInterval* m_pInfoTimeInterval;

	BOOL m_bCollectFilesStarted;
};

#endif // !defined(_COLLECTOR_PROCESS_H__)
