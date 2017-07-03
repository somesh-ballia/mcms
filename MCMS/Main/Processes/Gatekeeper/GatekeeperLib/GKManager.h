
#if !defined(_GKMANAGER_H__)
#define _GKMANAGER_H__

#include <set>
#include <vector>

#include "ManagerTask.h"
#include "Macros.h"
#include "GKManagerStructs.h"
#include "GKManagerOpcodes.h"
#include "GKServiceManager.h"
#include "GKDispatcherTask.h"
#include "GKProcess.h"
//#include "GKService.h"
//#include "GKCall.h"
#include "Segment.h"
#include "DataTypes.h"
//#include "GKToCsInterface.h"
//#include "GkCsReq.h"
//#include "GkCsInd.h"

void GKManagerEntryPoint(void* appParam);

class CGKManager : public CManagerTask
{
CLASS_TYPE_1(CGKManager,CManagerTask )
public:
	CGKManager();
	virtual ~CGKManager();

	virtual const char* NameOf() const { return "CGKManager";}
	virtual bool IsResetInStartupFail() const {return true;}
	void ManagerPostInitActionsPoint();
	void CreateDispatcher();


	TaskEntryPoint GetMonitorEntryPoint();

	PDECLAR_MESSAGE_MAP
//	PDECLAR_TRANSACTION_FACTORY
//	PDECLAR_TERMINAL_COMMANDS

//	void  Create(CSegment& appParam);
	void* 	GetMessageMap(); 
	
	void CreateServiceTasks();
	void OnMcuMngrManagementIpUpdate(CSegment* pParam);
	void OnCSApiMsg(CSegment *pSeg);
	

// action functions 

	void  	OnCSMngrServiceCfgUpdate(CSegment *pSeg);
	void		OnFailoverSlaveBcmMasterInd(CSegment *pSeg);
	void 		OnGkMngrFailoverRefreshRegInd(CSegment* pMsg);
	void 		OnFailoverMasterBcmSlaveInd(CSegment* pMsg);
protected:

	void	UpdateFaultsAndActiveAlarms(DWORD serviceId, eGkFaultsAndServiceStatus eFaultsOpcode);
	BYTE 	IsErrorFaultsOpcode(eGkFaultsAndServiceStatus eFaultsOpcode);
	char*	GetFaultsOpcodeAsString(eGkFaultsAndServiceStatus eFaultsOpcode);
	BYTE	GetFaultsLevelAccordingToOpcde(eGkFaultsAndServiceStatus eFaultsOpcode);
	void    RequestManagementIp();
	
	CGKProcess*  m_pProcess;

private:
	//virtual void DeclareStartupConditions();
/*attributes:	*/
	void SelfKill();
	eGkFaultsAndServiceStatus m_prevFaultsOpcode;
};



#endif // !defined(_GKDISMANAGER_H__)

