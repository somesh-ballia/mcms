#ifndef EMA_CTL_H
#define EMA_CTL_H

#include "AuthenticationStructs.h"


#define STATUS_CONTROL_BASE                 0
#define STATUS_CONTROL_LOGIN_EXISTS         STATUS_CONTROL_BASE+0  
#define STATUS_CONTROL_LOGIN_NOT_EXISTS     STATUS_CONTROL_BASE+1  
#define STATUS_CONTROL_NO_PERMISSION        STATUS_CONTROL_BASE+2
#define STATUS_CONTROL_LOGIN_INVALID        STATUS_CONTROL_BASE+3  
#define STATUS_CONTROL_PASSWORD_NOT_VALID   STATUS_CONTROL_BASE+4  




typedef struct SSwitchUserList
{
   int          numEmaUsers;	
   USERS_LIST_S emaUserList;
}TSwitchUserList;

int checkEmaUsers(INT8 *pUser,INT8 *pPass,UINT16 *authorizationGroup);


void updateEmaWatchdogTimer_(const char *file, int line);
void reStartEmaWatchdogTimer_(const char *file, int line);
void SetEmaWatchdogTimer_(const char *file, int line, int var);


#define updateEmaWatchdogTimer() updateEmaWatchdogTimer_(__FILE__, __LINE__)
#define SetEmaWatchdogTimer(var) SetEmaWatchdogTimer_(__FILE__, __LINE__, var)
#define reStartEmaWatchdogTimer() reStartEmaWatchdogTimer_(__FILE__, __LINE__)

#endif

