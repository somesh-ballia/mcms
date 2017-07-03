#ifndef _DIAGNOSTICS_API_H
#define _DIAGNOSTICS_API_H

#include "EmaApi.h"
#include "EmaShared.h"
#include "SharedDefines.h"
#include "Diagnostics.h"

#define MAX_DESC_STR_SIZE 				100
#define MAX_NUM_OF_ERRORS_TO_REPORT		30
void simTest();



/*****************************************************************************/
/*	EMA GENERAL HEADERS														 */
/*****************************************************************************/
typedef struct
{
	UINT32	ulOpcode;
	UINT32	ulMsgID;
	UINT32	ulSlotID;

}EmaReqHeader;

typedef struct
{
	UINT32 ulOpcode;
	UINT32 ulMsgID;
	UINT32 ulSlotID;
	UINT32 ulStatus;
	UINT8  acDesc[MAX_DESC_STR_SIZE];
}EmaIndHeader;

/* GDH - General Description Header */
typedef struct
{
	UINT32	msgOffset;
}EmaIndGeneralDescHeader;

/* SSH - Struct Specification Header */
typedef struct
{
	UINT32	fieldType;
	UINT32	numOfFields;
	UINT32	fieldOffset;
	UINT8	filedName[32];	
}EmaIndStructSpecHeader;

/*****************************************************************************/
/*	EMA REQUESTS															 */
/*****************************************************************************/
/* 1. Start Test */
typedef struct
{
	UINT32		testId;
	UINT32		unitId;
}TStartTestReqParams;

typedef struct
{
	UINT32 					ulNumOfElem;
	UINT32 					ulNumOfElemFields;
	TStartTestReqParams 	tStartTestReqParams[MAX_NUM_OF_TESTS];
}TStartTestList,*PTStartTestList;

typedef struct
{
	EmaReqHeader		tEmaReqHeader;
	UINT32				unNumOfLoops;
	UINT32				unStopOnFail;
	TStartTestList		tTStartTestList;
}TEmaStartTestReq;

/*****************************************************************************/
/*	EMA INDICATIONS															 */
/*****************************************************************************/
/* 1. Enter Test Mode Indication */
typedef struct
{
	EmaIndHeader		tEmaIndHdr;
}TEmaEnterTestModeInd;

/* 2. Test List Indication */
typedef struct
{
	INT8				testName[DGNS_TEST_NAME_MAX_LEN];
	UINT32 				unTestID;
	UINT32 				unTestOn;
	UINT32 				unLoop;
	UINT32 				unQuick;
	UINT32				unEstimateTime;
}TTestData;

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TTestData 			tTestData[MAX_NUM_OF_TESTS];
	
}TTestList,*PTTestList;

typedef struct
{
	EmaIndHeader			tEmaIndHdr;
	TTestList				tTestList;
}EmaTestListInd;

/* 3. Start Test Indication */
typedef struct
{
	UINT32	testNumber;
}StartTestData;

typedef struct
{
	EmaIndHeader			tEmaIndHdr;
	StartTestData			tStartTestData;
}TEmaStartTestInd;

/* 4. Stop Test Indication */
typedef struct
{
	UINT32	testNumber;
}StopTestData;

typedef struct
{
	TEmaIndHeader			tEmaIndHdr;
	StopTestData			tStopTestData;
}EmaStopTestInd;

/* 5. Get Test Status Indication */
typedef struct
{
	INT8					testName[DGNS_TEST_NAME_MAX_LEN];
	UINT32					unTestID;
	INT32					unUnitID;
	UINT32					unLoopNum;
	UINT32					unTestState;
	UINT32					unNumOfSuccesses;
	UINT32					unNumOfFailures;
	UINT32					unDuration;
	UINT32					unQuick;
}TTestStatusData;

typedef struct
{
	INT32					unNumOfReqLoops;
	UINT32					unStopOnFail;
	UINT32					unDuration;
}TTestGeneralParams;

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TTestStatusData 	tTestStatusData[MAX_NUM_OF_REPORTED_TESTS];
	
}TTestStatusList,*PTTestStatusList;

typedef struct
{
	TEmaIndHeader			tEmaIndHdr;
	TTestGeneralParams		tTestGeneralParams;
	TTestStatusList			tTestStatusList;
}TEmaGetTestStatusInd;

/* 6. Get Units List Indication */
typedef struct
{
	UINT32					unUnitID;
	UINT32					unUnitType;
	INT32					unUnitStatus;
}TUnitStatusData;

#if 0
typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TUnitStatusData 	tUnitStatusData[MAX_NUM_OF_UNITS];
	
}TUnitList,*PTUnitList;

typedef struct
{
	EmaIndHeader			tEmaIndHdr;
	TUnitList				tUnitList;
}EmaGetUnitsStatusInd;

#else

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
//	TUnitStatusData 	*tUnitStatusData;		
}TUnitList,*PTUnitList;
typedef struct
{
	EmaIndHeader			tEmaIndHdr;
    TUnitList				tUnitList;
}EmaGetUnitsStatusInd;

#endif

/* 7. Error List Indication */
typedef struct
{
	UINT32				testId;
	INT8	 			errorString[100];
}TErrorParams;

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TErrorParams		errors[MAX_NUM_OF_ERRORS_TO_REPORT];
	
}TErrorList,*PTErrorList;

typedef struct
{
	EmaIndHeader			tEmaIndHdr;
	TErrorList				tErrorList;
}EmaErrorListInd;

/* 8. Control Response Message */
typedef struct
{
	TEmaIndHeader		tEmaIndHdr;
}TEmaControlResponseMessage;

void* StartTestHandling(void* arg);
dgnsTestSession diagCurrentSession[MAX_SLOT_NUM];



// be carefull, there is similar enum eChassisType, it's not good for bit ops,
// but loader passes it, so it must stay
typedef enum
{
    E_BIT_OP_CHASSIS_TYPE_UNKNOWN = 0,
    E_BIT_OP_CHASSIS_TYPE_RMX2000 = 1,
    E_BIT_OP_CHASSIS_TYPE_RMX4000 = 2,
    E_BIT_OP_CHASSIS_TYPE_1500	   = 4,
    E_BIT_OP_CHASSIS_TYPE_NINJA = 8,
    E_BIT_OP_CHASSIS_TYPE_MAX
    
}	E_BIT_OP_CHASSIS_TYPE; 


// System , bitwise
// E_BIT_OP_CHASSIS_TYPE_RMX2000 - for RMX 2000
// E_BIT_OP_CHASSIS_TYPE_RMX4000 - for RMX 4000
// E_BIT_OP_CHASSIS_TYPE_1500    - for Yona
#define	E_BIT_OP_CHASSIS_ALL_SYSTEMS	(E_BIT_OP_CHASSIS_TYPE_RMX2000 | E_BIT_OP_CHASSIS_TYPE_RMX4000 | E_BIT_OP_CHASSIS_TYPE_1500 | E_BIT_OP_CHASSIS_TYPE_NINJA)


#endif

