#include <iostream>
using namespace std;

#include "NetSnmpIncludes.h"
#include "SNMPUtils.h"
#include "HlogElement.h"
#include "alarmActiveTable.h"
#include "StructTm.h"
#include "NStream.h"
#include "TraceStream.h"
#include "ObjString.h"
#include "SystemFunctions.h"
#include "OsProcessUtils.h"
#include "DefinesGeneral.h"

extern int alarmActiveLastChanged;



/////////////////////////////////////////////////////////////////////////////
void CSnmpUtils::UpdateAlarmActiveLastChanged()
{
    int pid = GetProcessId("snmpd");

    if(-1 != pid)
    {
        alarmActiveLastChanged = GetProcessAbsTime(pid);
    }
    return;

    
    
    struct snmp_pdu *pdu;
    struct snmp_pdu *response;
    oid anOID[MAX_OID_LEN];
    size_t anOID_len = MAX_OID_LEN;
    int status;
    struct snmp_session session, *ss;
    struct variable_list *vars;

    int count = 0;
 /*
     * Initialize the SNMP library
     */
//    init_snmp("snmpapp");

    /*
     * Initialize a "session" that defines who we're going to talk to
     */
    snmp_sess_init( &session );                   /* set up defaults */
    session.peername = strdup(TRUE == IsTarget() ? "localhost:161" : "localhost:8090");
//    session.timeout = 1;

    /* set up the authentication parameters for talking to the server */

// #ifdef DEMO_USE_SNMP_VERSION_3

//     /* Use SNMPv3 to talk to the experimental server */

//     /* set the SNMP version number */
//     session.version=SNMP_VERSION_3;
//     /* set the SNMPv3 user name */
//     session.securityName = strdup("MD5User");
//     session.securityNameLen = strlen(session.securityName);

//     /* set the security level to authenticated, but not encrypted */
//     session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;

//     /* set the authentication method to MD5 */
//     session.securityAuthProto = usmHMACMD5AuthProtocol;
//     session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
//     session.securityAuthKeyLen = USM_AUTH_KU_LEN;

//     /* set the authentication key to a MD5 hashed version of our
//        passphrase "The UCD Demo Password" (which must be at least 8
//        characters long) */
//     if (generate_Ku(session.securityAuthProto,
//                     session.securityAuthProtoLen,
//                     (u_char *) our_v3_passphrase, strlen(our_v3_passphrase),
//                     session.securityAuthKey,
//                     &session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
//         snmp_perror(argv[0]);
//         snmp_log(LOG_ERR,
//                  "Error generating Ku from authentication pass phrase. \n");
//         exit(1);
//     }
//    
//#else /* we'll use the insecure (but simplier) SNMPv1 */

    /* set the SNMP version number */
    session.version = SNMP_VERSION_1;

    /* set the SNMPv1 community name used for authentication */
    session.community =(u_char *)"public";
    session.community_len = strlen((const char *)session.community);

//#endif /* SNMPv1 */

    /*
     * Open the session
     */

    SOCK_STARTUP;

    ss = snmp_open(&session);                     /* establish the session */

    if (!ss) {
        snmp_perror("ack");
        snmp_log(LOG_ERR, "something horrible happened!!!\n");
        exit(2);
    }
    
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    read_objid(".1.3.6.1.2.1.1.3.0", anOID, &anOID_len);

#if OTHER_METHODS
    get_node("system.sysUptime.0", anOID, &anOID_len);
    read_objid("system.sysUptime.0", anOID, &anOID_len);
#endif

    snmp_add_null_var(pdu, anOID, anOID_len);
  
    /*
     * Send the Request out.
     */

    status = snmp_synch_response(ss, pdu, &response);

    /*
     * Process the response.
     */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
      /*
       * SUCCESS: Print the result variables
       */

      for(vars = response->variables; vars; vars = vars->next_variable)
        print_variable(vars->name, vars->name_length, vars);

      /* manipuate the information ourselves */
      for(vars = response->variables; vars; vars = vars->next_variable) {
        if (vars->type == ASN_OCTET_STR) {
	  char *sp = (char *)malloc(1 + vars->val_len);
	  memcpy(sp, vars->val.string, vars->val_len);
	  sp[vars->val_len] = '\0';
      printf("value #%d is a string: %s\n", count++, sp);
	  free(sp);
        }
        else{
                       cout <<"vars->val is :"<<*(vars->val.integer)<<endl;
            alarmActiveLastChanged = *(vars->val.integer);
            printf("value #%d is NOT a string! Ack!\n", count++);
        }
      }
    } else {
      /*
       * FAILURE: print what went wrong!
       */
         if (status == STAT_SUCCESS)
        fprintf(stderr, "Error in packet\nReason: %s\n",
                snmp_errstring(response->errstat));
      else
      {
          snmp_sess_perror("snmpget", ss);
      }
      
    }

    /*
     * Clean up:
     *  1) free the response.
     *  2) close the session.
     */
    if (response)
      snmp_free_pdu(response);

    if(session.peername)
    {
        free(session.peername);
    }
}

/////////////////////////////////////////////////////////////////////////////
// should suit the RMX status definition in the MIBS/POLYCOM-RMX-MIB.MIB
// rmxStatus ::= TEXTUAL-CONVENTION
//     STATUS          current
//     DESCRIPTION
//             "A possible status of RMX"
// 	SYNTAX  	INTEGER {
// 					normal  (0),
// 					startup (1),
// 					minor   (2),
// 					major   (3)
//     }
void ValidateRmxStatus(int &rmxStatus)
{
    if(0 <= rmxStatus || rmxStatus <= 3)
    {
        // Good
    }
    else
    {
        CSmallString message = "Bad RMX status should be [0,3], but is ";
        message << rmxStatus;
        FPASSERTMSG(TRUE, message.GetString());

        rmxStatus = 3; // major   (3)
    }
}

/////////////////////////////////////////////////////////////////////////////
void CSnmpUtils::SendAlarmListSnmpTrap(const vector<CLogFltElement> & alarmVector, bool isDeleted, int rmxStatus)
{
    for(vector<CLogFltElement>::const_iterator iTer = alarmVector.begin() ;
        iTer != alarmVector.end() ;
        iTer++)
    {
        const CLogFltElement & currentAlarm = *iTer;
        CSnmpUtils::SendSnmpTrap(currentAlarm, isDeleted, rmxStatus);
    }
}

/////////////////////////////////////////////////////////////////////////////
void CSnmpUtils::SendSnmpTrap(const CLogFltElement & refAlarm, bool isDeleted, int rmxStatus)
{
    int rmxStatusInSnmpFormat = ConvertRmxStatusToSnmpFormat(rmxStatus);
    
    const int errorCode = (isDeleted ? refAlarm.GetCode() + AA_RANGE_LAST : refAlarm.GetCode());
    const int uniqueIndex = refAlarm.GetIndex();

    CStructTm creationTime = refAlarm.GetTime();
    COstrStream ostrCreationTime;
    creationTime.SerializeSNMP(ostrCreationTime);
    const string &strCreationTime = ostrCreationTime.str();
    
    string description;
    refAlarm.GetCleanDescription(description);

    const DWORD MAX_DESC_LEN = 254;
    char strDesc[MAX_DESC_LEN + 1];
    strncpy(strDesc, description.c_str(), MAX_DESC_LEN);
    strDesc[MAX_DESC_LEN] = '\0';
    const int strDescLen = strlen(strDesc);
    
    oid             notification_oid[] =   { 1, 3 , 6 , 1 ,4 , 1, 13885 ,9 ,1 ,1 ,2 ,2 ,0, 999 };
    size_t          notification_oid_len = OID_LENGTH(notification_oid);
    notification_oid[13] = errorCode; // replace the 999 with the runtime errorcode
    
    oid             objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
    size_t          objid_snmptrap_len = OID_LENGTH(objid_snmptrap);

    oid    description_oid[] = { 1, 3, 6 ,1 ,4 ,1 ,13885 ,9 ,1 , 1 ,2 ,1 ,1};
    size_t   description_oid_len = OID_LENGTH(description_oid);

    
    oid    dateAndtime_oid[] = { 1, 3, 6 ,1 ,4 ,1 ,13885 ,9 ,1 , 1 ,2 ,1 ,2};
    size_t dateAndtime_oid_len = OID_LENGTH(dateAndtime_oid);

    
    oid    alarmIndex_oid[] = { 1, 3, 6 ,1 ,4 ,1 ,13885 ,9 ,1 , 1 ,2 ,1 ,3};
    size_t   alarmIndex_oid_len = OID_LENGTH(alarmIndex_oid);
    
    oid    alarmListName_oid[] = { 1, 3, 6 ,1 ,4 ,1 ,13885 ,9 ,1 , 1 ,2 ,1 ,4};
    size_t   alarmListName_oid_len = OID_LENGTH(alarmListName_oid);
    
    oid    rmxStatus_oid[] = { 1, 3, 6 ,1 ,4 ,1 ,13885 ,9 ,1 , 1 ,2 ,1 ,5};
    size_t rmxStatus_oid_len = OID_LENGTH(rmxStatus_oid);
    
    
    // here is where we store the variables to be sent in the trap
    netsnmp_variable_list *notification_vars = NULL;

    // add in the trap definition object 
    snmp_varlist_add_variable(&notification_vars,
                              objid_snmptrap, objid_snmptrap_len,
                              ASN_OBJECT_ID,
                              (u_char *) notification_oid,
                              notification_oid_len * sizeof(oid));
    
    // add description
    snmp_varlist_add_variable(&notification_vars,
                              description_oid, description_oid_len ,
                              ASN_OCTET_STR,
                              (const unsigned char*)strDesc, strDescLen);
    // add date and time
    snmp_varlist_add_variable(&notification_vars,
                              dateAndtime_oid, dateAndtime_oid_len ,
                              ASN_OCTET_STR,
                              (const unsigned char*)strCreationTime.c_str(), strCreationTime.length());

    // add unique index
    snmp_varlist_add_variable(&notification_vars,
                              alarmIndex_oid, alarmIndex_oid_len ,
                              ASN_UNSIGNED,
                              (const unsigned char*)&uniqueIndex, sizeof(uniqueIndex));

    // add Alarm list name
    snmp_varlist_add_variable(&notification_vars,
                              alarmListName_oid, alarmListName_oid_len,
                              ASN_OCTET_STR,
                              (const unsigned char*)ALARM_LIST_NAME,strlen(ALARM_LIST_NAME));

    // add RMX status
    ValidateRmxStatus(rmxStatusInSnmpFormat);
    snmp_varlist_add_variable(&notification_vars,
                              rmxStatus_oid, rmxStatus_oid_len,
                              ASN_UNSIGNED,
                              (const unsigned char*)&rmxStatusInSnmpFormat,sizeof(rmxStatusInSnmpFormat));
    
    // sends to Ver 1,2
    send_v2trap(notification_vars);

    FTRACEINTO << "\nCMcuMngrManager::SendSnmpTrap : " << errorCode << " : " << strDesc << " "<< uniqueIndex;

 //    cout << (isDeleted ? "Deleted" : "New") << "---"
//          << "SendSnmpTrap: " << errorCode << " : " << strDesc << " "<< uniqueIndex << endl;
    
//    cout.flush();
    
    snmp_free_varbind(notification_vars);
}




// see MIBS/POLYCOM-RMX-MIB.MIB
//
// rmxStatus ::= TEXTUAL-CONVENTION
//     STATUS          current
//     DESCRIPTION
//             "A possible status of RMX"
// 	SYNTAX  	INTEGER {
// 					normal  (0),
// 					startup (1),
// 					minor   (2),
// 					major   (3)
//     }
int CSnmpUtils::ConvertRmxStatusToSnmpFormat(int mcuStatus)
{
    int snmpStatus = 3;
    switch(mcuStatus)
    {
        case eMcuState_Startup:
            snmpStatus = 1;
            break;

        case eMcuState_Normal:
            snmpStatus = 0;
            break;

        case eMcuState_Minor:
            snmpStatus = 2;
            break;
    };
    return snmpStatus;
}

