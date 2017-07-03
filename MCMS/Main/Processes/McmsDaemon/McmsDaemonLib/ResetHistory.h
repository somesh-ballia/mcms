#ifndef __RESET_LIST_H__
#define __RESET_LIST_H__

#include "DataTypes.h"


//#define RESET_HISTORY_TIME_INTERVAL (SECOND * 5 * 60) // 5 MINUTES
#define MAX_RESET_NUM_BEFORE_SAFE_MODE 5



int ResetHistory_GetResetNumber();
void ResetHistory_AddStartup();
void ResetHistory_Remove();




#endif  // __RESET_LIST_H__
