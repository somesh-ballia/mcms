// SNMPTask.cpp

#include "SNMPTask.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "MessageHeader.h"
#include "Segment.h"
#include "Trace.h"
#include "Macros.h"
#include "ProcessBase.h"
#include "ManagerApi.h"
#include "OpcodesMcmsCommon.h"
#include "SysConfig.h"
#include "OsTask.h"
#include "SysConfigKeys.h"

PBEGIN_MESSAGE_MAP(CSNMPTask)
PEND_MESSAGE_MAP(CSNMPTask, CStateMachine);


CSNMPTask::CSNMPTask()
{}

CSNMPTask::~CSNMPTask()
{}

void CSNMPTask::InitTask()
{
  netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID,
                         NETSNMP_DS_AGENT_ROLE,
                         1);

  eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
  /* Flora fix bug: BRIDGE-1335 */
  std::string agentxq = (MCU_TMP_DIR+"/queue/agentX");

  netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID,
                        NETSNMP_DS_AGENT_X_SOCKET,
                            agentxq.c_str());



  init_agent(GetTaskName());
  snmp_disable_stderrlog();
  snmp_enable_filelog("/dev/null", 1);
  init_snmp(GetTaskName());


  RegisterOID();

  DWORD taskId = m_pTask->GetTaskId();
  CProcessBase::GetProcess()->SetSNMPTaskId(taskId);
}

void CSNMPTask::WaitForEvent()
{
  while (!GetSelfKill())
    agent_check_and_process(1);

  snmp_shutdown(GetTaskName());
}
