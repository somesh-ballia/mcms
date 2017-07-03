// McuMngrSNMPTask.cpp: implementation of the McuMngrSNMPTask class.
//
//////////////////////////////////////////////////////////////////////

#include "McuMngrSNMPTask.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "McuMngrProcess.h"
#include "alarmActiveTable.h"
#include "SNMPUtils.h"



Netsnmp_Node_Handler RMX_Status_Handler;

//represented in ticks (hundredths of a second)
int alarmActiveLastChanged = 0;
////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void McuMngrSNMPTaskEntryPoint(void* appParam)
{  
	CMcuMngrSNMPTask *SNMPTask = new CMcuMngrSNMPTask;
	SNMPTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMcuMngrSNMPTask::CMcuMngrSNMPTask()
{

}
 
CMcuMngrSNMPTask::~CMcuMngrSNMPTask()
{

}

//void CMcuMngrSNMPTask::RegisterOID(oid * oidToRegister , int & DataToStore)
void CMcuMngrSNMPTask::RegisterOID() const
{
    InitSnmpAlarmTable();
    
    static oid RMX_Status_oid[] = { 1, 3, 6 ,1 ,4, 1,13885 ,9 ,1 ,1 ,1 ,1,0};
    static oid alarmActiveLastChanged_oid[]= { 1 ,3 ,6 ,1 ,2 ,1 ,118 ,1 ,2 ,1};

    netsnmp_register_handler(netsnmp_create_handler_registration
                                  ("RMX_STATUS", RMX_Status_Handler, RMX_Status_oid,OID_LENGTH(RMX_Status_oid),
                                   HANDLER_CAN_RONLY));
  
    netsnmp_register_int_instance("alarmActiveLastChanged",
                                 alarmActiveLastChanged_oid ,
                                 OID_LENGTH(alarmActiveLastChanged_oid),
                                 &alarmActiveLastChanged, NULL);
}

int RMX_Status_Handler (netsnmp_mib_handler *handler,
                     netsnmp_handler_registration *reginfo,
                     netsnmp_agent_request_info *reqinfo,
                     netsnmp_request_info *requests)
     {
         static oid RMX_Status_oid[] = { 1, 3, 6 ,1 ,4, 1,13885 ,9 ,1 ,1 ,1 ,1,0};
         static u_long accesses = 0;

         CProcessBase * m_pProcess =CProcessBase::GetProcess();
         CMcuMngrProcess *processMcuMngr = dynamic_cast<CMcuMngrProcess*>(m_pProcess);
         eMcuState mcuStatus = processMcuMngr->GetSystemState();
         int mcuSnmpStatus = CSnmpUtils::ConvertRmxStatusToSnmpFormat(mcuStatus);

//DEBUGMSGTL(("testhandler", "Got request:\n"));
         /*
          * loop through requests
          */
         while (requests) {
             netsnmp_variable_list *var = requests->requestvb;
     
             //DEBUGMSGTL(("testhandler", "  oid:"));
             //DEBUGMSGOID(("testhandler", var->name, var->name_length));
             //DEBUGMSG(("testhandler", "\n"));

             switch (reqinfo->mode) {
             case MODE_GET:
                 if (netsnmp_oid_equals(var->name, var->name_length, RMX_Status_oid,OID_LENGTH(RMX_Status_oid) )== 0)
                 {             
                     snmp_set_var_typed_value(var, ASN_INTEGER,
                                          (u_char *) & mcuSnmpStatus,
                                          sizeof(mcuSnmpStatus));
                     return SNMP_ERR_NOERROR;
                 }
                 
                 break; 
     
             case MODE_GETNEXT:
                 if (snmp_oid_compare(var->name, var->name_length,RMX_Status_oid,OID_LENGTH(RMX_Status_oid)) < 0)
                 {
                     snmp_set_var_objid(var,RMX_Status_oid ,OID_LENGTH(RMX_Status_oid));
                     snmp_set_var_typed_value(var, ASN_INTEGER,
                                              (u_char *) & mcuSnmpStatus ,
                                              sizeof(mcuSnmpStatus));
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


    













