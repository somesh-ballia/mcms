#ifndef CSMNGRINTERNALSTRUCTS_H_
#define CSMNGRINTERNALSTRUCTS_H_

typedef struct
{

    DWORD               boardId;    // the boardid that will config the internal and external cs ip
    DWORD               pqId;       // the pqid   that will config the internal and external cs ip
} CS_IP_CONFIG_MS_PARAMS_S;
/*
#include "CSMngrDefines.h"


typedef struct
{
	WORD                unitId;
	compStatuses        unitStatus;
	DWORD               faultId; 	
} CS_UNIT_FAILURE_S;


typedef struct
{
	CS_UNIT_FAILURE_S  unitFailure[NumOfComponents];
} CS_UNITS_FAILURES_LIST_S;
*/


#endif /*CSMNGRINTERNALSTRUCTS_H_*/
