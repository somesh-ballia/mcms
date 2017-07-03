// SystemMonitoringProcess.h

#ifndef SYSTEM_MONITORING_PROCESS_H_
#define SYSTEM_MONITORING_PROCESS_H_

#include "ProcessBase.h"

#define MAX_NUM_OF_CPU_LAN_PORTS 2 // Signaling & Mngmnt

class CEthernetSettingsStructWrapper;

class CSystemMonitoringProcess : public CProcessBase
{
  CLASS_TYPE_1(CSystemMonitoringProcess, CProcessBase)
public:
  friend class CTestSystemMonitoringProcess;

                         CSystemMonitoringProcess();
  virtual                ~CSystemMonitoringProcess();
  virtual DWORD          GetMaxTimeForIdle() const;
  virtual eProcessType   GetProcessType() {return eProcessSystemMonitoring;}
  virtual BOOL           UsingSockets()   {return NO;}
  virtual TaskEntryPoint GetManagerEntryPoint();
  virtual int GetProcessAddressSpace(void);

  void                   SetSpecEthernetSettingsStructWrapper(const ETH_SETTINGS_SPEC_S* pEthSettings);
  ETH_SETTINGS_SPEC_S*   GetSpecEthernetSettingsStructWrapper(DWORD portNum);
  void                   UpdateSpecEthernetSettingsMaxCounters(DWORD portId);
  void                   ClearSpecEthernetSettingsMaxCounters(DWORD portId);

private:
  void                   InitCpuLanPortsMembers();

  CEthernetSettingsStructWrapper* m_pEthernetSettingsStructsList[MAX_NUM_OF_CPU_LAN_PORTS];
};

#endif
