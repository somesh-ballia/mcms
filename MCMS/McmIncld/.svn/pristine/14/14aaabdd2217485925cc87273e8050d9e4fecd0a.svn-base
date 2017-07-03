#ifndef __AUDIT_DEFINES_H__
#define __AUDIT_DEFINES_H__


#include "DataTypes.h"




// #define AUDIT_EVENT_LOGIN                  1
// #define AUDIT_EVENT_LOGOUT                 2
// #define AUDIT_EVENT_DISCONNECT             3

// #define AUDIT_EVENT_HTTP_POST_SET          4
// #define AUDIT_EVENT_HTTP_PUT               5
// #define AUDIT_EVENT_HTTP_GET               6
// #define AUDIT_EVENT_HTTP_RMDIR             7
// #define AUDIT_EVENT_HTTP_MKDIR             8
// #define AUDIT_EVENT_HTTP_CONN_TOO_ACTIVE   9

// #define AUDIT_EVENT_ADD_USER               10
// #define AUDIT_EVENT_UPDATE_USER            11
// #define AUDIT_EVENT_DELETE_USER            12

// #define AUDIT_EVENT_SYS_START              13
// #define AUDIT_EVENT_SYS_SHUTDOWN           14
// #define AUDIT_EVENT_SYS_RESTART            15
// #define AUDIT_EVENT_SYS_DIAGNOSTIC_MODE    16
// #define AUDIT_EVENT_SYS_CHANGE_IP_BY_USB   17





// -------------------------------------------------------------------
// audit event types
// if you change it don't forget to update AuditInt2Str 
// -------------------------------------------------------------------
enum eAuditEventType
{
    eAuditEventTypeApi = 0,
    eAuditEventTypeHttp,
    eAuditEventTypeInternal,

    NUM_AUDIT_EVENT_TYPES
};




// -------------------------------------------------------------------
// audit statuses
// if you change it don't forget to update AuditInt2Str 
// -------------------------------------------------------------------
enum eAuditEventStatus
{
    eAuditEventStatusOk = 0,
    eAuditEventStatusFail,

    NUM_AUDIT_EVENT_STATUSES
};




// -------------------------------------------------------------------
// header for audit event
// -------------------------------------------------------------------
#define MAX_AUDIT_USER_NAME_LEN          32
#define MAX_AUDIT_WORKSTATION_NAME_LEN   62
#define MAX_AUDIT_IP_ADDRESS_LEN         44
#define MAX_AUDIT_ACTION_LEN             80
#define MAX_AUDIT_DESCRIPTION_LEN        80
#define MAX_AUDIT_DESCRIPTION_EX_LEN     80


typedef struct 
{   
    char userIdName[MAX_AUDIT_USER_NAME_LEN];              // name of the initializator user(SUPPORT)
    APIU32 reportModule;                                   // Mcms, Shelf
    char workStation[MAX_AUDIT_WORKSTATION_NAME_LEN];      // name of the computer (f3-yurir)
    char ipAddress[MAX_AUDIT_IP_ADDRESS_LEN];              // ip address of the initiator, EMA in most cases
    APIU32 eventType;                                      // must be valid, see list of event types
    APIU32 status;                                         // must be valid see list of statuses
    char action[MAX_AUDIT_ACTION_LEN];                     // action name, name of SET transaction.
    char description[MAX_AUDIT_DESCRIPTION_LEN];           // free string which supposed to describe status
    char descriptionEx[MAX_AUDIT_DESCRIPTION_EX_LEN];      // more details about status
    
}AUDIT_EVENT_HEADER_S;






#endif  // __AUDIT_DEFINES_H__
