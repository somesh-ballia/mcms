// ExchangeModuleMonitor.h: interface for the CExchangeModuleMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CExchangeModuleMONITOR__)
#define _CExchangeModuleMONITOR__

#include "MonitorTask.h"
#include "Macros.h"
#include "CommResApi.h"

class CExchangeModuleMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CExchangeModuleMonitor();
	virtual ~CExchangeModuleMonitor();
	STATUS OnGetExchangeCfg(CRequest* pGetRequest);
	STATUS OnConfPartyUpdateConfDetailsRequest(CSegment* pParam);
	STATUS OnSetLastSetExchangeCfgIndication(CSegment* pParam);

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
private:
	int m_nLastSetExchangeCfgIndication;
	std::string m_LastSetExchnageStatusDesc;
	STATUS OnGetLastSetExchangeCfgIndication(CRequest* pGetRequest);
	void   OnSpcialCharExceedBufferDisplayName(CCommResApi* pRsrv,const char* pDisplayName);
	// Copied from psosxml file for VNGR-14306
	virtual void ChangeSpecialChar(const char* org_str, BYTE bRemoveSpecialChar, char* new_str);
	virtual int NeededSizeForNewArray(const char* org_str, BOOL isAllocSpaceForSpecialChar = TRUE);


};

#endif // !defined(_CExchangeModuleMONITOR__)
