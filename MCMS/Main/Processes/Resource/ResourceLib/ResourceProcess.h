#if !defined(_ResourcePROCESS_H__)
#define _ResourcePROCESS_H__

#include "ConfResources.h"
#include "SystemResources.h"
#include "ProcessBase.h"
#include "StringsMaps.h"
#include "ConfResources.h"
#include "StringsMaps.h"
#include "MoveManager.h"
#include "CentralConferencesDB.h"

class CRsrvManager;
class CReservator;
class CRsrvDB;

typedef std::map<PortID, std::string> PortPerProcess;
typedef std::map<ServiceID, std::pair<std::string, PortPerProcess> > AllocatedUdpPorts;

////////////////////////////////////////////////////////////////////////////
//                        CResourceProcess
////////////////////////////////////////////////////////////////////////////
class CResourceProcess : public CProcessBase
{
	CLASS_TYPE_1(CResourceProcess, CProcessBase)

public:
	friend class CTestResourceProcess;

	                       CResourceProcess();
	virtual               ~CResourceProcess();

	virtual const char*    NameOf() const                             { return "CResourceProcess"; }
	virtual eProcessType   GetProcessType()                           { return eProcessResource; }
	virtual BOOL           UsingSockets()                             { return NO; }
	virtual TaskEntryPoint GetManagerEntryPoint();
	virtual void           AddExtraStringsToMap();
	virtual int            GetProcessAddressSpace()                   { return 0; } // 35 MB
	virtual BOOL           IsHasSettings()                            { return TRUE; }
	virtual void           SetUpProcess();
	int                    TearDown();
	CSystemResources*      GetSystemResources() const                 { return m_pSystemResources; };
	CCentralConferencesDB* GetCentralConferencesDB() const            { return m_pCentralConferencesDB; };
	CConfRsrcDB*           GetConfRsrcDB() const                      { return m_pCentralConferencesDB->GetConfRsrcDB(); };
	CRsrvDB*               GetRsrvDB() const                          { return m_pCentralConferencesDB->GetRsrvDB(); };
	ReservedConferences*   GetConfRsrvRsrcs() const                   { return m_pCentralConferencesDB->GetConfRsrvRsrcs(); };
	SleepingConferences*   GetSleepingConferences() const             { return m_pCentralConferencesDB->GetSleepingConferences(); };
	CProfilesDB*           GetProfilesDB() const                      { return m_pCentralConferencesDB->GetProfilesDB(); };
	void                   SetRsrvManager(CRsrvManager* pRsrvManager) { m_pRsrvManager = pRsrvManager; };
	CReservator*           GetReservator() const;
	CMoveManager*          GetMoveManager() const                     { return m_pMoveManager; };
	virtual bool           IsFailoverBlockTransaction_SlaveMode(string sAction);
	virtual int            GetTaskMbxSndBufferSize() const            { return 1024 * 1024 - 1; }
	void                   SetIsSNMPEnabled(BOOL bEnabled)            { m_bSNMPEnabled = bEnabled; }
	BOOL                   GetIsSNMPEnabled() const                   { return m_bSNMPEnabled; }
	AllocatedUdpPorts&     GetAllocatedUdpPorts()                     { return m_allocatedUdpPorts; }

private:
	CMoveManager*          m_pMoveManager;
	CSystemResources*      m_pSystemResources;
	CCentralConferencesDB* m_pCentralConferencesDB;
	CRsrvManager*          m_pRsrvManager;
	BOOL                   m_bSNMPEnabled;
	AllocatedUdpPorts      m_allocatedUdpPorts;
};

#endif // !defined(_ResourcePROCESS_H__)

