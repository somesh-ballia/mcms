#ifndef CARDSINTERNALSTRUCTS_H_
#define CARDSINTERNALSTRUCTS_H_

#include "CardsDefines.h"



typedef struct
{
	WORD                   unitId;
	eCardUnitLoadedStatus  unitStatus;
	DWORD                  faultId; 	
} UNIT_FAILURE_S;


typedef struct
{
	UNIT_FAILURE_S  unitFailure[MAX_NUM_OF_UNITS];
} UNITS_FAILURES_LIST_S;


typedef struct
{   
	BOOL  isPowerOffProblem;
	BOOL  isVoltageProblem;
	BOOL  isTemperatureMajorProblem;
	BOOL  isTemperatureCriticalProblem;
	BOOL  isFailureProblem;
	BOOL  isOtherProblem;
	BOOL  isRtmIsdnMissingProblem;
} ACTIVE_ALARMS_SPECIFIC_CARD_S;


typedef struct
{
	BAD_SPONTANEOUS_IND_S		badSpontaneousStruct;
	DWORD                       MfaBoardId;

} MFA_BAD_SPONTANEOUS_IND_S;

typedef struct
{
	eCSExtIntMsgState   state;
    DWORD               boardId;    // the boardid that will config the internal and external cs ip
    BOOL                ExtMsgArrived;
    BOOL                IntMsgArrived;
} CS_EXT_INT_MSG_CARD_PARAMS_S;
/*
typedef struct
{
	WORD                unitId;
	eSmComponentStatus  compStatus;
	DWORD               faultId; 	
} SM_COMPONENT_FAILURE_S;


typedef struct
{
	SM_COMPONENT_FAILURE_S  unitFailure[eMaxNumOfSmComponents];
} SM_COMPONENTS_FAILURES_LIST_S;
*/

#endif /*CARDSINTERNALSTRUCTS_H_*/
