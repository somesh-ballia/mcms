// SystemMonitoringProcess.cpp

#include "SystemMonitoringProcess.h"
#include "SystemFunctions.h"
#include "EthernetSettingsMonitoring.h"

extern void SystemMonitoringManagerEntryPoint(void* appParam);

CProcessBase* CreateNewProcess()
{
  return new CSystemMonitoringProcess;
}

TaskEntryPoint CSystemMonitoringProcess::GetManagerEntryPoint()
{
  return SystemMonitoringManagerEntryPoint;
}

CSystemMonitoringProcess::CSystemMonitoringProcess()
{
  for (int i = 0; i < MAX_NUM_OF_CPU_LAN_PORTS; i++)
  {
    m_pEthernetSettingsStructsList[i] = new CEthernetSettingsStructWrapper;
  }

  InitCpuLanPortsMembers();
}

// Virtual
int CSystemMonitoringProcess::GetProcessAddressSpace(void)
{
    return 72 * 1024 * 1024;
}

void CSystemMonitoringProcess::InitCpuLanPortsMembers()
{
  eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();
  switch (curProductType)
  {
    case eProductTypeRMX1500:
    {
      m_pEthernetSettingsStructsList[0]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
      m_pEthernetSettingsStructsList[0]->SetPortId(ETH_SETTINGS_CPU_MNGMNT_PORT_ON_SWITCH_BOARD_RMX1500);
      m_pEthernetSettingsStructsList[1]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
      m_pEthernetSettingsStructsList[1]->SetPortId(ETH_SETTINGS_CPU_MNGMNTB_PORT_ON_SWITCH_BOARD_RMX1500);
      break;
    }

    case eProductTypeRMX4000:
    {
      m_pEthernetSettingsStructsList[0]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
      m_pEthernetSettingsStructsList[0]->SetPortId(ETH_SETTINGS_CPU_MNGMNT_1_PORT_ON_SWITCH_BOARD);
      m_pEthernetSettingsStructsList[1]->SetSlotId(FIXED_DISPLAY_BOARD_ID_SWITCH);
      m_pEthernetSettingsStructsList[1]->SetPortId(ETH_SETTINGS_CPU_SGNLNG_1_PORT_ON_SWITCH_BOARD);
      break;
    }
	
    //added by Richer for 802.1x project om 2013.12.26 
    case eProductTypeNinja:
    case eProductTypeGesher:
    {
        m_pEthernetSettingsStructsList[0]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
        m_pEthernetSettingsStructsList[0]->SetPortId(ETH_SETTINGS_ETH0_PORT_IDX_SOFTMCU);
        m_pEthernetSettingsStructsList[1]->SetSlotId(FIXED_BOARD_ID_MAIN_SOFTMCU);
        m_pEthernetSettingsStructsList[1]->SetPortId(ETH_SETTINGS_ETH1_PORT_IDX_SOFTMCU);
        break;
    }

	default:
	  // Note: some enumeration value are not handled in switch. Add default to suppress warning.
	  break;
  }
}

CSystemMonitoringProcess::~CSystemMonitoringProcess()
{
  for (int i = 0; i < MAX_NUM_OF_CPU_LAN_PORTS; i++)
  {
    POBJDELETE(m_pEthernetSettingsStructsList[i]);
  }
}

// Virtual
DWORD CSystemMonitoringProcess::GetMaxTimeForIdle() const
{
  return 12000;
}

void CSystemMonitoringProcess::SetSpecEthernetSettingsStructWrapper(const ETH_SETTINGS_SPEC_S* pEthSettings)
{
  for (int i = 0; i < MAX_NUM_OF_CPU_LAN_PORTS; i++)
  {
    if (pEthSettings->portParams.portNum == m_pEthernetSettingsStructsList[i]->GetPortId())
    {
      m_pEthernetSettingsStructsList[i]->SetEthSettingsStruct(pEthSettings);
      break;
    }
  }
}

ETH_SETTINGS_SPEC_S* CSystemMonitoringProcess::GetSpecEthernetSettingsStructWrapper(DWORD portNum)
{
  for (int i = 0; i < MAX_NUM_OF_CPU_LAN_PORTS; i++)
  {
    if (portNum == m_pEthernetSettingsStructsList[i]->GetPortId())
      return m_pEthernetSettingsStructsList[i]->GetEthSettingsStruct();
  }

  return NULL;
}

void CSystemMonitoringProcess::UpdateSpecEthernetSettingsMaxCounters(DWORD portId)
{
  for (int i = 0; i < MAX_NUM_OF_CPU_LAN_PORTS; i++)
  {
    if (portId == m_pEthernetSettingsStructsList[i]->GetPortId())
    {
      m_pEthernetSettingsStructsList[i]->UpdateMaxCounters();
      break;
    }
  }
}

void CSystemMonitoringProcess::ClearSpecEthernetSettingsMaxCounters(DWORD portId)
{
  for (int i = 0; i < MAX_NUM_OF_CPU_LAN_PORTS; i++)
  {
    if (portId == m_pEthernetSettingsStructsList[i]->GetPortId())
    {
      m_pEthernetSettingsStructsList[i]->ClearMaxCounters();
      break;
    }
  }
}
