#ifndef UTILITY_PROCESS_H__
#define UTILITY_PROCESS_H__

////////////////////////////////////////////////////////////////////////////
#include "ProcessBase.h"
#include "TcpDumpEntity.h"

////////////////////////////////////////////////////////////////////////////
class CUtilityProcess : public CProcessBase  
{
	friend class CTestUtilityProcess;

	CLASS_TYPE_1(CUtilityProcess, CProcessBase)

	virtual eProcessType GetProcessType()
	{ return eProcessUtility; }

	virtual BOOL UsingSockets()
	{ return false; }

	virtual int GetProcessAddressSpace()
	{ return 64 * 1024 * 1024; }

	virtual TaskEntryPoint GetManagerEntryPoint();

	virtual void AddExtraStringsToMap();

public:

	CUtilityProcess();
	virtual ~CUtilityProcess();

	CTcpDumpEntityList* GetTcpDumpEntityList();
	CTcpDumpEntityList* GetTcpDumpStartList();
	CTcpDumpStatus*     GetTcpDumpStatus();

	bool GetIsTcpDumpRunning();
	void SetIsTcpDumpRunning(bool mode);

	void    SetTimeElapsed(CStructTm time);
	CStructTm    GetTimeElapsed();

	bool GetIsUiUpdateNeeded();
	void SetIsUiUpdateNeeded(bool bNeeded);

	eSystemCardsMode GetSystemCardsMode() const
	{ return m_SystemCardsMode; }

	void SetSystemCardsMode(eSystemCardsMode curMode)
	{ m_SystemCardsMode = curMode; }

private:

	CTcpDumpEntityList* m_pTcpDumpEntityList;
	CTcpDumpEntityList* m_pTcpDumpStartList;
	CTcpDumpStatus*     m_pTcpDumpStatus;

	bool                m_isTcpDumpRunning;
	CStructTm           m_time_elapsed;

	// VNGR-22122
	bool                m_isUiUpdateNeeded;
	eSystemCardsMode    m_SystemCardsMode;
};

////////////////////////////////////////////////////////////////////////////
#endif // UTILITY_PROCESS_H__
