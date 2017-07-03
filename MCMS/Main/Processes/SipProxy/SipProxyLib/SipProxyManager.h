#if !defined(_SIPPROXYMANAGER_H__)
#define _SIPPROXYMANAGER_H__

#include <set>
#include <vector>

#include "ManagerTask.h"
#include "Macros.h"
#include "SipProxyServiceManager.h"
#include "SipProxyDispatcher.h"
#include "SipProxyProcess.h"
#include "Segment.h"
#include "DataTypes.h"
#include "TaskApi.h"
#include "SipProxyTaskApi.h"

void SipProxyManagerEntryPoint(void* appParam);

//	int  ResolveHostName(DWORD indInConfs,char* HostName);
class CSipProxyManager : public CManagerTask
{
CLASS_TYPE_1(CSipProxyManager,CManagerTask )
public:
	CSipProxyManager();
	virtual ~CSipProxyManager();

	virtual const char* NameOf() const { return "CSipProxyManager";}
	virtual bool IsResetInStartupFail() const {return true;}
	void ManagerPostInitActionsPoint();
	void CreateDispatcher();


	TaskEntryPoint GetMonitorEntryPoint();

//	SubscribersList			*m_SubscribersList;
	PDECLAR_MESSAGE_MAP
//	PDECLAR_TRANSACTION_FACTORY
//	PDECLAR_TERMINAL_COMMANDS

//	void  Create(CSegment& appParam);
	void* 	GetMessageMap(); 
	
	void CreateServiceTasks();
	void OnMcuMngrManagementIpUpdate(CSegment* pParam);
	void OnCSApiMsg(CSegment *pSeg);

// action functions 

		
protected:

	void  RequestIPServicesFromCsManager();
	void  AskMcuMngrForConfigurationStatus();
	
	CSipProxyProcess*  m_pProcess;


	void  	OnServiceCfgUpdate(CSegment* pSeg);

private:
	//virtual void DeclareStartupConditions();
/*attributes:	*/
	void SelfKill();

};

	WORD GetPresentedConfNumber();
	void IncreasePresentedConfNumber();
	void DecreasePresentedConfNumber();

#endif // !defined(_SIPPROXYMANAGER_H__)

