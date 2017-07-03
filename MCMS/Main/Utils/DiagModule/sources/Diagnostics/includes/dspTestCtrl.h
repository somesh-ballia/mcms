#ifndef DSPTESTCTRL_H_
#define DSPTESTCTRL_H_

#include "DiagnosticsApiExt.h"
#include "DiagnosticsApi.h"
#include "Dgnsgeneral.h"  //NUM_OF_DSPS_IN_BOARD define

 /* dsp as bad	*/
#define	DSP_SEC_TIMEOUT	15
#define	DSP_TIMEDOUT_VAL	0xFFFF
#define DSP_INFINITE_VAL	0xFFFE

extern	UINT32	*lastReqTimeStamp[MAX_SLOT_NUM];
//number of dsps we have. Is initialized at startup.
extern	UINT32	num_OF_DSPS_IN_BOARD;
extern	UINT32	PQUnitSlot;
extern	UINT32	CardUnitSlot;	//general card view,that includes "all dsp" tests
extern	UINT32	RTMUnitSlot;
extern	UINT32	totalUnitsOnBoard;
extern	UINT32	dspWorkingCount;	//counts how much dsps are working
extern	INT32 	*errorOccured[];//[NUM_OF_DSPS_IN_BOARD + 1];	//i keep track of which DSP had errors

/*	Those are pointers to test lists of each DSP. */
void **dspTestListCollection[MAX_SLOT_NUM];//[NUM_OF_DSPS_IN_BOARD + 1];
int isNotEmptyList(void *TargetList);
int freeDspTestList(void **TargetList);
//int initDspTestList(void **TargetList,UINT32 stopOnFail,UINT32 isQuickSession);

/*	Adding test info to db, before we send this db to dsp	*/
int	addToDspTestStruct(void **TargetTestList,dgnsCtrlTestInfo* testToAdd);
/*	Getting test info from the array we recieved.Index is index of test to extract
 * 	if get -1,it means the index is non-existant. Index 0..N-1	*/
int getFromDspTestStruct(void *SourceTestList,dgnsCtrlTestInfo *testToGet,int index);

/*	checking whether the test is DSP test or not. All the DSP tests get different processing	*/
int isTestDSPTest(dgnsCtrlTestInfo *testToExamine);

/*	To find ,where in memory resides the part information about test we recieved from dsp	*/
dgnsCtrlTestInfo *	findTestInMemory(UINT32 ulSlotID, dgnsCtrlTestInfo *testToExamine);

/*	Each DSP test has thread for itself. It should recieve list of tests to perform
 * 	for the DSP, send it, communicate with it (keep alive/get status/stop test) and
 * update the results per test	*/
//void dspTestThread(void);
int initAllDspRelevantStructures();
void freeAllDspRelevantStructures();
e_TypeOfTest	getTestType(dgnsCtrlTestInfo * testToExamine);
#define MS_DELAY_BETWEEN_DSP_STATUS_POLLING		10000	


/*
 * Stuff, copied from MFA project,for dsps:
 */
#define PARTIAL_BOARD_FIRST_DSP       11
#define PARTIAL_BOARD_LAST_DSP        17

#define LITE_PARTIAL_BOARD_FIRST_DSP       1
#define LITE_PARTIAL_BOARD_LAST_DSP        13

typedef enum
{
	eDSPNotLoaded,
	eDSPNotExistantOnBoard,
	eDSPLoadAsked,
	eDSPLoaded,
	eDSPLoadFailed
}dspLoadState;

typedef enum
{
	eDSPLoadTestStreamUndefinedState,
	eDSPLoadTestStreamOpen,
	eDSPLoadTestStreamOpenFailed,
}dspStreamLoadTest;

/*	This holds information about mass dsp tests (homologation - buss load)	*/
typedef enum
{
	eMassDspNotLoaded,
	eMassDspRequested,
	eMassDspAcked,
	eMassDspSuccess,
	eMassDspFail
}massTestDspStatus;

typedef enum
{
 eSTAT_NO_RTM,
 eSTAT_POST,
 eSTAT_RUNNING,
 eSTAT_DSP_DOWN,
 eSTAT_DSP_FATAL,
 eSTAT_DSP_RELOADING,
 eMAX_STAT_NUMER,
}e_RtmStat;

extern dspStreamLoadTest	*loadStreamInformation[MAX_SLOT_NUM];//[NUM_OF_DSPS_IN_BOARD + 1];
extern dspLoadState		*loadedDSPInformation[MAX_SLOT_NUM];//[NUM_OF_DSPS_IN_BOARD + 1];
extern char 		*isDspSessionRunning[MAX_SLOT_NUM]; //this will hold session status per dsp.

#endif /*DSPTESTCTRL_H_*/
