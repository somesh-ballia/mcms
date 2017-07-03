// ExchangeModuleProcess.cpp

#include "ExchangeModuleProcess.h"

#include "TaskApi.h"
#include "SystemFunctions.h"
#include "ExchangeModuleCfg.h"
#include "ExchangeDataTypes.h"

extern void ExchangeModuleManagerEntryPoint(void* appParam);

CProcessBase* CreateNewProcess(void)
{
	return new CExchangeModuleProcess;
}

CExchangeModuleProcess::CExchangeModuleProcess(void)
{
	m_pExchangeModuleCfg = new CExchangeModuleCfg;
	m_pCalendarItemList = new CCalendarItemList;
	m_mcuMngrIp_Ind = FALSE;
}

CExchangeModuleProcess::~CExchangeModuleProcess(void)
{
	delete m_pExchangeModuleCfg;
	delete m_pCalendarItemList;
}

// Virtual
TaskEntryPoint CExchangeModuleProcess::GetManagerEntryPoint()
{
    return ExchangeModuleManagerEntryPoint;
}

// Virtual
const char* CExchangeModuleProcess::NameOf(void) const
{
    return GetCompileType();
}

// Virtual
eProcessType CExchangeModuleProcess::GetProcessType(void)
{
    return eProcessExchangeModule;
}

// Virtual
BOOL CExchangeModuleProcess::UsingSockets(void)
{
    return NO;
}

// Virtual
int CExchangeModuleProcess::GetProcessAddressSpace(void)
{
    return 56 * 1024 * 1024;
}

BOOL CExchangeModuleProcess::GetMcuExchangeCfg(CExchangeModuleCfg &exchangeModuleCfg) const
{
	exchangeModuleCfg = *m_pExchangeModuleCfg;
	return TRUE;
}

void CExchangeModuleProcess::SetMcuExchangeCfg(const CExchangeModuleCfg &exchangeModuleCfg)
{
	*m_pExchangeModuleCfg = exchangeModuleCfg;
}

void CExchangeModuleProcess::UpdateCalendarItemsList(const CCalendarItemList& calendarItemList)
{
	*m_pCalendarItemList = calendarItemList;


	std::stringstream list;
	m_pCalendarItemList->Dump(list);

	PTRACE2(eLevelInfoNormal,"CExchangeModuleProcess::UpdateCalendarItemsList - ",list.str().c_str());
	puts(list.str().c_str());
}
