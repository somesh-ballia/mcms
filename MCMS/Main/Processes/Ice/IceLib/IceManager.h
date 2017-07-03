#if !defined(_ICEMANAGER_H__)
#define _ICEMANAGER_H__

#include <set>
#include <vector>

#include "ManagerTask.h"
#include "Macros.h"
#include "IceServiceManager.h"
#include "IceDispatcher.h"
#include "IceProcess.h"
#include "Segment.h"
#include "DataTypes.h"
#include "TaskApi.h"
//#include "IceTaskApi.h"

void IceManagerEntryPoint(void* appParam);

class CIceManager : public CManagerTask
{
CLASS_TYPE_1(CIceManager,CManagerTask )
public:
	CIceManager();
	virtual ~CIceManager();

	virtual const char* NameOf() const { return "CIceManager";}
	virtual bool IsResetInStartupFail() const {return true;}
	void ManagerPostInitActionsPoint();
	void CreateDispatcher();


	TaskEntryPoint GetMonitorEntryPoint();

	PDECLAR_MESSAGE_MAP
	void* 	GetMessageMap();
	void OnMcuMngrManagementIpUpdate(CSegment* pParam);

// action functions 

		
protected:

	void  RequestIPServicesFromCsManager();
	
	CIceProcess*  m_pProcess;


	void  	OnServiceCfgUpdate(CSegment* pSeg);
	void 	OnIpServiceParamInd(CSegment* pSeg);
	void	OnIpServiceParamEnd(CSegment* pSeg);

private:
	//virtual void DeclareStartupConditions();
/*attributes:	*/
	void SelfKill();

};



#endif // !defined(_ICEMANAGER_H__)

