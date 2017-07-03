#ifndef _EMA_API_H
#define _EMA_API_H

#define MAX_DESC_STR_SIZE 			100
#define MAX_NUM_OF_TESTS	 		20
#define MAX_NUM_OF_UNITS_TO_TEST	30 // 26 * DSPs + 2 * PQ + 1 FPGA + 1 Flash
#define MAX_NUM_OF_ERRORS_TO_REPORT		30

typedef enum
{
	eEmaStatOk,
	eEmaStatFail
}eEmaStatus;

typedef enum
{
  e_unsignedChar,
  e_signedChar,
  e_unsignedShort,
  e_signedShort,
  e_unsignedLong,
  e_signedLong,
  e_string
} eVarType;

typedef struct SStructToStr
{
	UINT32 	varType;
	UINT32 	varCount;
	UINT32 	jumpOffst;
	INT8 	varString[32];
}TStructToStr;

/*****************************************************************************/
/*	EMA GENERAL HEADERS														 */
/*****************************************************************************/
typedef struct
{
	UINT32	opcode;
	UINT32	msgId;
	UINT32	slotId;

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
	UINT32				unNumOfReqLoops;
	UINT32				unStopOnFail;
	TStartTestList		tTStartTestList;
}EmaStartTestReqMsg;

/*****************************************************************************/
/*	EMA INDICATIONS															 */
/*****************************************************************************/
/* 1. Enter Test Mode Indication */
typedef struct
{
	EmaIndHeader		tEmaIndHdr;
}EmaEnterTestModeInd;

/* 2. Test List Indication */
typedef struct
{
	INT8				testName[40];
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
	EmaIndHeader			tEmaIndHdr;
}EmaStartTestInd;

/* 4. Stop Test Indication */
typedef struct
{
	EmaIndHeader			tEmaIndHdr;
}EmaStopTestInd;

/* 5. Get Test Status Indication */
typedef struct
{
	INT8					testName[40];
	UINT32					unTestID;
	UINT32					unUnitId;
	UINT32					unLoopNum;
	UINT32					unTestState;
	UINT32					unNumOfSuccesses;
	UINT32					unNumOfFailures;
	UINT32					unDuration;
	UINT32					unQuick;
}TTestStatusData;

typedef struct
{
	UINT32					unNumOfReqLoops;
	UINT32					unStopOnFail;
	UINT32					unDuration;
}TTestGeneralParams;

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TTestStatusData 	tTestStatusData[MAX_NUM_OF_TESTS];

}TTestStatusList,*PTTestStatusList;

typedef struct
{
	EmaIndHeader			tEmaIndHdr;
	TTestGeneralParams		tTestGeneralParams;
	TTestStatusList			tTestStatusList;
}EmaGetTestStatusInd;

/* 6. Get Units List Indication */
typedef struct
{
	UINT32					unUnitID;
	UINT32					unUnitType;
	UINT32					unUnitStatus;
}TUnitStatusData;

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TUnitStatusData 	tUnitStatusData[MAX_NUM_OF_UNITS_TO_TEST];

}TUnitList,*PTUnitList;

typedef struct
{
	EmaIndHeader			tEmaIndHdr;
	TUnitList				tUnitList;
}EmaGetUnitsStatusInd;


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

/*****************************************************************************/
/*	TESTS DATABASE															 */
/*****************************************************************************/
typedef struct
{
	UINT32	unTestID;
	UINT32	unUnitID;
	UINT32	unLoopNum;
	UINT32	testInProgress;
	UINT32 	testSuccessCounter;
	UINT32	unDuration;	// Bracha - check this out
	UINT32	unQuick;
	APIUBOOL performTestFlag;
	INT8	 testName[40];
}TestInfo;

typedef struct
{
	UINT32		unNumOfReqLoops;
	UINT32		unPerformedLoopNum;
	UINT32		unStopOnFail;
	UINT32		unDuration;	// Bracha - check this out
	TestInfo	tTestInfo[MAX_NUM_OF_TESTS + 1];
}TESTS_DATABASE;



static TStructToStr atEnterDiagModeIndSpecArray[]=
{
	{(e_unsignedLong),   (1),              (4),                   "Opcode"},
	{(e_unsignedLong),   (1),              (4),                   "MsgID"},
	{(e_unsignedLong),   (1),              (4),                   "SlotID"},
	{(e_unsignedLong),   (1),              (4),                   "Status"},
	{(e_string),		  (1),             (MAX_DESC_STR_SIZE),   "Description"},
};

static TStructToStr atTestListIndSpecArray[]=
{
	{(e_unsignedLong),   (1),              (4),        			"Opcode"},
	{(e_unsignedLong),   (1),              (4),        			"MsgID"},
	{(e_unsignedLong),   (1),              (4),        			"SlotID"},
	{(e_unsignedLong),   (1),              (4),        			"Status"},
	{(e_string),		 (1),              (MAX_DESC_STR_SIZE), "Description"},
	{(e_unsignedLong),   (1),              (4),        			"NumOfElem"},
	{(e_unsignedLong),   (1),              (4),        			"NumOfElemFields"},
	{(e_string),		 (1),   		   (40),   	 			"TestName"},
	{(e_unsignedLong),   (1),              (4),        			"TestID"},
	{(e_unsignedLong),   (1),              (4),        			"TestOn"},
	{(e_unsignedLong),   (1),              (4),        			"Loop"},
	{(e_unsignedLong),   (1),              (4),        			"Quick"},
    {(e_unsignedLong),   (1),              (4),        			"EstimatedTime"},
};

// Start test indication message
static TStructToStr atStartTestIndSpecArray[]=
{
	{(e_unsignedLong),   (1),              (4),        			"Opcode"},
	{(e_unsignedLong),   (1),              (4),        			"MsgID"},
	{(e_unsignedLong),   (1),              (4),        			"SlotID"},
	{(e_unsignedLong),   (1),              (4),        			"Status"},
	{(e_string),		 (1),              (MAX_DESC_STR_SIZE), "Description"},
};

// Stop test indication message
static TStructToStr atStopTestIndSpecArray[]=
{
	{(e_unsignedLong),   (1),              (4),        			"Opcode"},
	{(e_unsignedLong),   (1),              (4),        			"MsgID"},
	{(e_unsignedLong),   (1),              (4),        			"SlotID"},
	{(e_unsignedLong),   (1),              (4),        			"Status"},
	{(e_string),		 (1),   		   (MAX_DESC_STR_SIZE), "Description"},
};

// Test status indication message
static TStructToStr atTestStatusIndSpecArray[]=
{
	{(e_unsignedLong),   (1),              (4),        "Opcode"},
	{(e_unsignedLong),   (1),              (4),        "MsgID"},
	{(e_unsignedLong),   (1),              (4),        "SlotID"},
	{(e_unsignedLong),   (1),              (4),        "Status"},
	{(e_string),		 (1),   (MAX_DESC_STR_SIZE),   "Description"},
	{(e_unsignedLong),   (1),              (4),        "Loop"},
	{(e_unsignedLong),   (1),              (4),        "StopOnFail"},
	{(e_unsignedLong),   (1),              (4),        "Duration"},
	{(e_unsignedLong),   (1),              (4),        "NumOfElem"},
	{(e_unsignedLong),   (1),              (4),        "NumOfElemFields"},
    {(e_string),		 (1),   		   (40) ,      "TestName"},
	{(e_unsignedLong),   (1),              (4),        "TestID"},
	{(e_unsignedLong),   (1),              (4),        "UnitID"},
	{(e_unsignedLong),   (1),              (4),        "Loop"},
	{(e_unsignedLong),   (1),              (4),        "Status"},
	{(e_unsignedLong),   (1),              (4),        "Pass"},
	{(e_unsignedLong),   (1),              (4),        "Failed"},
	{(e_unsignedLong),   (1),              (4),        "Duration"},
	{(e_unsignedLong),   (1),              (4),        "Quick"},
};

// Unit list indication message
static TStructToStr atUnitStatusListIndSpecArray[]=
{
	{(e_unsignedLong),   (1),              (4),        "Opcode"},
	{(e_unsignedLong),   (1),              (4),        "MsgID"},
	{(e_unsignedLong),   (1),              (4),        "SlotID"},
	{(e_unsignedLong),   (1),              (4),        "Status"},
	{(e_string),		 (1),   (MAX_DESC_STR_SIZE),   "Description"},
	{(e_unsignedLong),   (1),              (4),        "NumOfElem"},
	{(e_unsignedLong),   (1),              (4),        "NumOfElemFields"},
	{(e_unsignedLong),   (1),              (4),        "UnitID"},
	{(e_unsignedLong),   (1),              (4),        "UnitType"},
	{(e_unsignedLong),   (1),              (4),        "UnitStatus"},
};

static TStructToStr atErrorListIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "TestID"},
	{e_string,		   1,   		   100,   	 "ErrorString"},
};
#endif

