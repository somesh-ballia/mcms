// SNMPAgentTask.cpp

#include "SNMPAgentTask.h"

#include <iostream>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "ifTable.h"
#include "ipAddrTable.h"
#include "ipAddressTable.h"

#include "h323McConfigTable.h"
#include "TaskApi.h"
#include "SnmpData.h"
#include "OpcodesMcmsInternal.h"
#include "ManagerApi.h"
#include "TraceStream.h"
#include "Telemetry.h"
#include "PolycomMCUMIB.h"

Netsnmp_Node_Handler ifNumber_Handler;

void snmpAgentMngrEntryPoint(void* appParam)
{
  CSNMPAgentTask* task = new CSNMPAgentTask;
  task->Create(*(CSegment*)appParam);
}

PBEGIN_MESSAGE_MAP(CSNMPAgentTask)
PEND_MESSAGE_MAP(CSNMPAgentTask, CStateMachine);

CSNMPAgentTask::CSNMPAgentTask()
{
	m_ifNumber = 0;

}

CSNMPAgentTask::~CSNMPAgentTask()
{}

void CSNMPAgentTask::RegisterOID() const
{
  H323Table_Init();
  IfTable_Init();
  IpTable_Init();
  
  CIpTableAddress::instance().IpTableAddress_Init();
  
  init_polycom();

  static oid ifNumber_oid[] = { 1, 3, 6, 1, 2, 1, 2, 1, 0 };

  netsnmp_register_handler(netsnmp_create_handler_registration("ifNumber",
                                                               ifNumber_Handler,
                                                               ifNumber_oid,
                                                               OID_LENGTH(ifNumber_oid),
                                                               HANDLER_CAN_RONLY));

  CManagerApi apiCSMngr(eProcessSNMPProcess);
  apiCSMngr.SendOpcodeMsg(SNMP_AGENT_READY_IND);
}

int ifNumber_Handler(netsnmp_mib_handler* handler,
                     netsnmp_handler_registration* reginfo,
                     netsnmp_agent_request_info* reqinfo,
                     netsnmp_request_info* requests)
{
  static oid ifNumber_oid[] = { 1, 3, 6, 1, 2, 1, 2, 1, 0 };
  u_long     ifNumber = IfTable_GetIfNum();

  /*
   * loop through requests
   */
  while (requests)
  {
    netsnmp_variable_list* var = requests->requestvb;

    switch (reqinfo->mode)
    {
      case MODE_GET:
      if (netsnmp_oid_equals(var->name, var->name_length, ifNumber_oid,
                             OID_LENGTH(ifNumber_oid)) == 0)
      {
        snmp_set_var_typed_value(var, ASN_INTEGER,
                                 (u_char*) &ifNumber,
                                 sizeof(ifNumber));
        return SNMP_ERR_NOERROR;
      }

      break;

      case MODE_GETNEXT:
      if (snmp_oid_compare(var->name, var->name_length, ifNumber_oid,
                           OID_LENGTH(ifNumber_oid)) < 0)
      {
        snmp_set_var_objid(var, ifNumber_oid, OID_LENGTH(ifNumber_oid));
        snmp_set_var_typed_value(var, ASN_INTEGER,
                                 (u_char*) &ifNumber,
                                 sizeof(ifNumber));
        return SNMP_ERR_NOERROR;
      }

      break;

      default:
      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_GENERR);
      break;
    }

    requests = requests->next;
  }

  return SNMP_ERR_NOERROR;
}

// Static.
unsigned char* CSNMPAgentTask::ReadTelemetry(eTelemetryType type, size_t* var_len)
{
  FTRACECOND_AND_RETURN_VALUE(eTT_Unknown == type,
      TelemetaryTypeToStr(type) << " is not supported",
      NULL);

  CProcessBase* proc = CProcessBase::GetProcess();
  FPASSERT_AND_RETURN_VALUE(NULL == proc, NULL);
  FPASSERT_AND_RETURN_VALUE(NULL == var_len, NULL);

  CSegment* in = new CSegment;
  *in << static_cast<unsigned int>(type);

  CSegment out;
  OPCODE response;
  OPCODE request = SNMP_GET_TELEMETRY_DATA_REQ;

  STATUS status =
      CManagerApi(proc->GetProcessType()).SendMessageSync(in,
                                                          request,
                                                          1 * SECOND,
                                                          response,
                                                          out);
  FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != status,
    "Unable to send " << proc->GetOpcodeAsString(request)
      << ": " << proc->GetStatusAsString(status),
    NULL);

  FTRACECOND_AND_RETURN_VALUE(STATUS_OK != response,
      "Failed to get data for " << type << ": " << proc->GetStatusAsString(response),
      NULL);

  static CTelemetryValue ret;
  ret = CTelemetryValue(out);

  return ret.GetPtr(*var_len);
}
