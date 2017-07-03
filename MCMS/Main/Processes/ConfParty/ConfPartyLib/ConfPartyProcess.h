#if !defined(_ConfPartyPROCESS_H__)
#define _ConfPartyPROCESS_H__

#include "LookupTables.h"
#include "ProcessBase.h"
#include "SharedMemoryMapCommon.h"
#include "IpServiceListManager.h"
#include "LobbyApi.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "SystemFeatures.h"
#include <list>
#include "EncryptionCommon.h"
#include "Image.h"
#include "LayoutSharedMemoryMap.h"
#include "IndicationIconSharedMemoryMap.h"


typedef class CCustomizeDisplaySettingForOngoingConfConfiguration CustomizeDisplaySetting;

enum  eIceStatus
{
	eIceStatusOFF = 0,
	eIceStatusRegister,
	eIceStatusON
};

////////////////////////////////////////////////////////////////////////////
//                        CConfPartyProcess
////////////////////////////////////////////////////////////////////////////
class CConfPartyProcess : public CProcessBase
{
	CLASS_TYPE_1(CConfPartyProcess, CProcessBase)

public:
	typedef std::list< RTM_ISDN_PARAMS_MCMS_S* > IsdnServicesList;
	friend class CTestConfPartyProcess;

	                            CConfPartyProcess();
	virtual                    ~CConfPartyProcess();

	virtual const char*         NameOf() const { return "CConfPartyProcess";}

	virtual void                TearDownProcess();
	virtual eProcessType        GetProcessType() {return eProcessConfParty;}
	virtual BOOL                UsingSockets()   {return NO;}
	virtual TaskEntryPoint      GetManagerEntryPoint();
	virtual void                AddExtraStringsToMap();
	virtual int                 GetProcessAddressSpace();

	virtual BOOL                IsHasSettings(){return TRUE;}
	virtual void                CreateTask(const char* taskName);
	virtual bool                RequiresProcessInstanceForUnitTests() {return true;}

	// Manage IP service list
	void                        Clear();
	WORD                        numberOfIpServices();
	STATUS                      insertIpService(CConfIpParameters* pConfIpParameters);
	STATUS                      updateIpService(CConfIpParameters* pConfIpParameters);
	STATUS                      removeIpService(DWORD serviceID);
	CConfIpParameters*          FindIpService(DWORD serviceID);
	CConfIpParameters*          FindServiceByName(const char* serviceName);
	CIpServiceListManager*      GetIpServiceListManager();
	const CLobbyApi*            GetpLobbyApi();
	void                        SetpLobbyApi(CLobbyApi* pLobbyApi);

	const CTaskApp*             GetpRsrvManagerTask();
	void                        SetpRsrvManagerTask(CTaskApp* pRsrvManagerTask);

	void                        KillAllConfAndPartyTasks();
	// Isdn services
	WORD                        numberOfIsdnServices();
	void                        AddIsdnService(RTM_ISDN_PARAMS_MCMS_S* pRtmIsdnServiceParams);
	void                        DeleteIsdnService(const std::string& serviceName);
	RTM_ISDN_PARAMS_MCMS_S*     GetIsdnService(std::string serviceName);
	RTM_ISDN_PARAMS_MCMS_S*     RemoveIsdnService(std::string serviceName);
	void                        SetIsdnServiceAsDefault(const std::string& serviceName);

	// System Based Mode
	void                        SetSystemCardsBasedMode(const eSystemCardsMode systemBasedMode);
	eSystemCardsMode            GetSystemCardsBasedMode();
	BOOL                        IsValidSystemCardsBasedMode(const eSystemCardsMode systemCardsBasedMode);

	// System Capacity Limits
	void                        UpdateSystemCapacityLimitsAccordingToSystemMode();
	WORD                        GetMaxNumberOfOngoingConferences();
	WORD                        GetMaxNumberOfPartiesInConf();
	WORD                        GetMaxNumberOfVideoPartiesInConf();

	virtual DWORD               GetMaxTimeForIdle(void) const { return 12000; }

	virtual bool                IsFailoverBlockTransaction_SlaveMode(string sAction);
	eIceStatus                  m_IceInitializationStatus;
	eIceStatus                  m_WebRTCIceInitializationStatus;
	BYTE                        m_IsEnableBWPolicyCheck;
	DWORD                       m_UcMaxVideoRateAllowed;

	CSharedMemMap*              GetSharedMemoryMap();

	void                        InitializeEncryptionKeysSharedMemory();
	void                        FreeEncryptionKeysSharedMemory();
	EncyptedSharedMemoryTables* GetEncryptionKeysSharedMemory();

	// Change Layout Improvement - Layout Shared Memory (CL-SM)
	void                        InitializeLayoutSharedMemory();
	CLayoutSharedMemoryMap*     GetLayoutSharedMemory();
	void                        FreeLayoutSharedMemory();

	// Indication Icon change Improvement - Indication Icon Shared Memory (CL-SM)
	void                        InitializeIndicationIconSharedMemory();
	CIndicationIconSharedMemoryMap*     GetIndicationIconSharedMemory();
	void                        FreeIndicationIconSharedMemory();

	CustomizeDisplaySetting*    GetCustomizeDisplaySettingForOngoingConfConfiguration();

	BOOL                        IsFeatureSupportedByHardware(const eFeatureName featureName) const;

	CPartyImageLookupTable*     GetPartyImageLookupTable()       { return &m_PartyImageLookupTable; }
	CLookupIdParty*             GetLookupIdParty()               { return &m_LookupIdParty; }
	CLookupTableParty*          GetLookupTableParty()            { return &m_LookupTableParty; }

	virtual int                 GetTaskMbxSndBufferSize() const  {return 1024 * 1024 -1;}

	void                        SetIsSNMPEnabled(BOOL bEnabled)  { m_bSNMPEnabled = bEnabled; }
	BOOL                        GetIsSNMPEnabled() const         { return m_bSNMPEnabled; }

protected:
	virtual void                AddExtraStatusesStrings();

	CustomizeDisplaySetting*    m_pCustomizeSettingForOngoingConfConfiguration;
	// Attributes
	CLobbyApi*                  m_pLobbyApi;
	CTaskApp*                   m_pRsrvManagerTask;
	CIpServiceListManager*      m_ipServiceListManager;
	IsdnServicesList            m_serviecsList;
	eSystemCardsMode            m_systemCardsBasedMode;

	// /Capacity parameters, updated according to the system mode when initiating the ConfParty process
	WORD                        m_maxNumberOfOngoingConferences;
	WORD                        m_maxNumberOfPartiesInConf;
	WORD                        m_maxNumberOfVideoPartiesInConf;

	CSharedMemMap*              m_pSharedMemoryMap;

	CSystemFeatures             m_systemFeatures;
	CPartyImageLookupTable      m_PartyImageLookupTable;
	CLookupIdParty              m_LookupIdParty;
	CLookupTableParty           m_LookupTableParty;
	EncyptedSharedMemoryTables* m_encyptedSharedMemoryTables;
	BOOL                        m_bSNMPEnabled;

	//Change Layout Improvement - Layout Shared Memory (CL-SM)
	CLayoutSharedMemoryMap*     m_layoutSharedMemoryMap;

	// Indication Icon change - Indication Icon Shared Memory (CL-SM)
	CIndicationIconSharedMemoryMap*     m_indicationIconSharedMemoryMap;
};

#endif // !defined(_ConfPartyPROCESS_H__)
