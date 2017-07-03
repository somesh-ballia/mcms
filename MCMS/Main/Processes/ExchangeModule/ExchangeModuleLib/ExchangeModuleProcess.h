// ExchangeModuleProcess.h

#ifndef EXCHANGE_MODULE_PROCESS_H_
#define EXCHANGE_MODULE_PROCESS_H_

#include "ProcessBase.h"

class CurlHTTPS;
class CExchangeModuleCfg;
class CCalendarItemList;

class CExchangeModuleProcess : public CProcessBase
{
CLASS_TYPE_1(CExchangeModuleProcess, CProcessBase)
public:
	friend class CTestExchangeModuleProcess;

	CExchangeModuleProcess(void);
	virtual ~CExchangeModuleProcess(void);

	virtual const char* NameOf(void) const;
	virtual eProcessType GetProcessType(void);
	virtual BOOL UsingSockets(void);
	virtual TaskEntryPoint GetManagerEntryPoint(void);
	virtual int GetProcessAddressSpace(void);
  virtual DWORD GetMaxTimeForIdle(void) const { return 12000; }

	BOOL GetMcuExchangeCfg(CExchangeModuleCfg &exchangeModuleCfg) const;
	void SetMcuExchangeCfg(const CExchangeModuleCfg &exchangeModuleCfg);

	const CCalendarItemList* GetCalendarItemList(void) const
    {
	    return m_pCalendarItemList;
    }

	void UpdateCalendarItemsList(const CCalendarItemList& calendarItemList);
	std::string			GetMngntIp(){return m_mngmntIp;};
	BOOL			    IsMngntIpIndRecieved(){return m_mcuMngrIp_Ind;};
	void 	SetMngnt(std::string ip)
	{
		m_mcuMngrIp_Ind = TRUE;
		m_mngmntIp = ip;
	};
protected:
	CExchangeModuleCfg* m_pExchangeModuleCfg;
	CCalendarItemList*  m_pCalendarItemList;
	BOOL			    m_mcuMngrIp_Ind;
	std::string		    m_mngmntIp;
};

#endif  // EXCHANGE_MODULE_PROCESS_H_
