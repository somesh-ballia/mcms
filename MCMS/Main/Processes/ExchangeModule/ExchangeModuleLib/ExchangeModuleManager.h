// ExchangeModuleManager.h: interface for the CExchangeModuleManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_ExchangeModuleMANAGER_H__)
#define _ExchangeModuleMANAGER_H__

#include "ManagerTask.h"
#include "Macros.h"

void ExchangeModuleManagerEntryPoint(void* appParam);

class CExchangeModuleCfg;
class CExchangeClientCntl;

class CExchangeModuleManager : public CManagerTask
{
CLASS_TYPE_1(CExchangeModuleManager,CManagerTask )
public:
	CExchangeModuleManager();
	virtual ~CExchangeModuleManager();

	// overrides
	virtual const char * NameOf() const { return "CExchangeModuleManager"; }
	virtual void ManagerPostInitActionsPoint();
	virtual TaskEntryPoint GetMonitorEntryPoint();

	STATUS HandleSetExchangeModuleCfg(CRequest *pRequest);

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

protected:

	void SendExchangeOnOfConfigurationToConfParty(bool bExchangeFeatureActivated);
	void OnUpdateExchangeCfgParams(CSegment* pParam);
	void OnMcuMngrStartupEndInd(CSegment *pSeg);
	void OnCheckStartupEndToutAnycase(CRequest *pRequest);
	STATUS HandleTerminalItems(CSegment* seg, std::ostream& answer);
	BOOL IsStartupFinished() const;

	CExchangeModuleCfg*		m_pExchangeModuleCfg;
	CExchangeClientCntl*	m_pExchangeClientCntl;

};

#endif // !defined(_ExchangeModuleMANAGER_H__)
