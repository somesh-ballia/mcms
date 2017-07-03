// CollectorManager.h

#if !defined(_DEMOMANAGER_H__)
#define _DEMOMANAGER_H__

#include "ManagerTask.h"
#include "Macros.h"

void CollectorManagerEntryPoint(void* appParam);

class CCollectorProcess;
class CCollectorManager : public CManagerTask
{
CLASS_TYPE_1(CCollectorManager,CManagerTask )
public:
	CCollectorManager();
	virtual ~CCollectorManager();

	virtual void ManagerPostInitActionsPoint();

    virtual void SelfKill();

	TaskEntryPoint GetMonitorEntryPoint();

	STATUS OnCollectInfo(CRequest* pRequest);
	void OnStartCollectInfoEstimatedSize(CRequest* pRequest);
	void OnAbortCollectInfo(CRequest* pRequest);

	void OnCollectInfoEndInd(CSegment* pSeg);

	void OnCollectInfoFailedInd(CSegment* pSeg);
	void SimulateCollectInfo(CSegment* pSeg);

	STATUS HandleTerminalCollectInfo(CTerminalCommand & command, std::ostream& answer);

private:
	virtual unsigned int GetMaxLegitimateUsagePrecents(void) const {return 50;}

	CCollectorProcess *m_pCollectorProcess;
  CTaskApi*  m_pCollectorUnitApi;

  PDECLAR_MESSAGE_MAP;
  PDECLAR_TERMINAL_COMMANDS;
  PDECLAR_TRANSACTION_FACTORY;
};

#endif // !defined(_DEMOMANAGER_H__)
