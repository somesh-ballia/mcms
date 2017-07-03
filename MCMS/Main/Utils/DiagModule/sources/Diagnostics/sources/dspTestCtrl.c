#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "DiagDataTypes.h"
#include "dspTestCtrl.h"
#include "StatusesGeneral.h"
//#include "DspCommunications.h"
#include "timers.h"
//#include "RtmEngine.h"
//#include "ConnectivityTests.h"
#include "tools.h"

extern char APPL_DSP_DIAG[]; 
extern e_RtmStat RtmStatus;
extern UINT32	isRtmLoaded; //This flag is 1 when RTM is already loaded,and 0 otherwise
UINT32	*massDspTestsStatus[MAX_SLOT_NUM];	//This will hold information about what dsps failed, for final report
massTestDspStatus	*loadTestStatus[MAX_SLOT_NUM]; //This variable will tells us,whether DSP acked homologation/bus load test.
UINT32	dspWorkingCount;	//counts how much dsps are working

UINT32	*lastReqTimeStamp[MAX_SLOT_NUM];
UINT32	num_OF_DSPS_IN_BOARD;
UINT32	PQUnitSlot;
UINT32	CardUnitSlot;	//general card view,that includes "all dsp" tests
UINT32	RTMUnitSlot;
UINT32	totalUnitsOnBoard;
UINT32	dspWorkingCount;	//counts how much dsps are working
INT32 	*errorOccured[MAX_SLOT_NUM];//[NUM_OF_DSPS_IN_BOARD + 1];	//i keep track of which DSP had errors

dspStreamLoadTest	*loadStreamInformation[MAX_SLOT_NUM];//[NUM_OF_DSPS_IN_BOARD + 1];
dspLoadState		*loadedDSPInformation[MAX_SLOT_NUM];//[NUM_OF_DSPS_IN_BOARD + 1];
char 		*isDspSessionRunning[MAX_SLOT_NUM]; //this will hold session status per dsp.

/* Shows whether this list is empty. Because the list points to tests of DSP number,
 * i use this,to find out whether current dsp has any tests to do*/
int isNotEmptyList(void *TargetList)
{
	if (TargetList)
		return (SWAPL(((dgnsDSPTestSession*)TargetList)->numOfElements) != 0);
	else
		return 0;
}
/*
int initDspTestList(void **TargetList,UINT32 stopOnFail,UINT32 isQuickSession)
{
	dgnsDSPTestSession*	dspSessPtr;
	void *testPtr;
	//our empty list only contains header.
	if(NULL != (testPtr = (void*)malloc(sizeof(dgnsDSPTestSession)))) return -1;
	*TargetList = testPtr;//(void*)malloc(sizeof(dgnsDSPTestSession));
	dspSessPtr = (dgnsDSPTestSession*)(*TargetList);
	dspSessPtr->stopOnFail = SWAPL(stopOnFail);
	dspSessPtr->quickSession = SWAPL(isQuickSession);
	dspSessPtr->numOfElements = SWAPL(0);	
	return 1;
}
*/
int freeDspTestList(void **TargetList)
{
	if (*TargetList)
	{
		free(*TargetList);
		*TargetList = 0;
	}
	return 1;
}


/*	This function will add another DSP test to struct, which will be sent
 * 	to DSP, to parse,fill and return 										*/  
int addToDspTestStruct(void **TargetList,dgnsCtrlTestInfo* testToAdd)
{
	/* This will hold target data size */
	UINT32 		targetDataSize;
	/*	This pointer will hold address of where to add new info	*/
	dgnsCtrlTestInfo	*dataPtr;
	dgnsDSPTestSession*	dspSessPtr;
	/* How many data elements already exist in our TargetList	*/
	UINT32		elementsExist;
	UINT32		stopOnFail,isQuickSess;
	
	/*	We are checking how much data there is here. The first byte holds this info	*/
	dspSessPtr = (dgnsDSPTestSession*)*TargetList;
	elementsExist = SWAPL(dspSessPtr->numOfElements);
	//i dont SWAPL it,because idont use their values,i just store it.
	isQuickSess = dspSessPtr->quickSession;
	stopOnFail = dspSessPtr->stopOnFail;
	/*	our target data is size of Session info,and the next come dgnsCtrlTestInfo structures*/ 
	targetDataSize = sizeof(dgnsCtrlTestInfo)*(elementsExist + 1) + sizeof(dgnsDSPTestSession);
	*TargetList = realloc(*TargetList,targetDataSize);
	if (*TargetList)
	{			
		/*	next bytes is the new structure we are adding	*/
		dataPtr = (dgnsCtrlTestInfo*)((INT8*)(*TargetList) + sizeof(dgnsCtrlTestInfo)*(elementsExist) + sizeof(dgnsDSPTestSession));
		
		dataPtr->duration = SWAPL(testToAdd->duration);
		dataPtr->failTests = SWAPL(testToAdd->failTests);
		dataPtr->isActiveNow = SWAPL(testToAdd->isActiveNow);
		dataPtr->loopsDone = SWAPL(testToAdd->loopsDone);
		dataPtr->neededLoops = SWAPL(testToAdd->neededLoops);
		dataPtr->successTests = SWAPL(testToAdd->successTests);
		dataPtr->testId = SWAPL(testToAdd->testId);
		dataPtr->testStartTime = SWAPL(testToAdd->testStartTime);
		dataPtr->unitOnSlot = SWAPL(testToAdd->unitOnSlot);
		
		/*	first byte is number of members in array.But the address is different now	*/
		dspSessPtr = (dgnsDSPTestSession*)*TargetList;
		elementsExist++;
		dspSessPtr->numOfElements = SWAPL(elementsExist);
		dspSessPtr->quickSession = isQuickSess;
		dspSessPtr->stopOnFail = stopOnFail;
		return 1;
	}

	return -1;
}

int getFromDspTestStruct(void *SourceTestList,dgnsCtrlTestInfo *testToGet,int index)
{
	int elementsExist;
	dgnsCtrlTestInfo *ptrToOurData;
	dgnsDSPTestSession*	dspSessPtr;
	
	dspSessPtr = (dgnsDSPTestSession*)SourceTestList;
	elementsExist = SWAPL(dspSessPtr->numOfElements);
	
	
	/*i dont have the element requested... */
	if (elementsExist <= index)
		return -1;

	/*	here i find our struct. index*size of each struct, and 1 byte in the begining */
	ptrToOurData = (dgnsCtrlTestInfo*)((INT8*)SourceTestList + (index * sizeof(dgnsCtrlTestInfo)) + sizeof(dgnsDSPTestSession));
	
	/*	swapping to dsp endian	*/
	testToGet->duration = SWAPL(ptrToOurData->duration);
	testToGet->failTests = SWAPL(ptrToOurData->failTests);
	testToGet->isActiveNow = SWAPL(ptrToOurData->isActiveNow);
	testToGet->loopsDone = SWAPL(ptrToOurData->loopsDone);
	testToGet->neededLoops = SWAPL(ptrToOurData->neededLoops);
	testToGet->successTests = SWAPL(ptrToOurData->successTests);
	testToGet->testId = SWAPL(ptrToOurData->testId);
	testToGet->testStartTime = SWAPL(ptrToOurData->testStartTime);
	testToGet->unitOnSlot = SWAPL(ptrToOurData->unitOnSlot);
		
	return 1;
}



e_TypeOfTest	getTestType(dgnsCtrlTestInfo * testToExamine)
{
/*	
	if ((testToExamine->testId == eDSP_DOWNLOAD_DIAG) || (testToExamine->testId == eDSP_CONNECTIVITY_DIAG))
			return e_typeDSPtestRunSerialWay;
	if ((testToExamine->testId >= eRTM_DSP_CLOCKS_DIAG )&&(testToExamine->testId<=eRTM_DSP_CPLD_DIAG))
			return e_typeRTMTest;
	if ((testToExamine->testId == eBOARD_HOMOLOGATION_TEST) || (testToExamine->testId == eBOARD_BUS_LOAD_DIAG))
			return e_typeDSPtestRunSerialWay;
*/
	if ((testToExamine->unitOnSlot > 0) && (testToExamine->unitOnSlot <= num_OF_DSPS_IN_BOARD) )
		return e_typeDSPtest;
	
	
	/*I refer to RTM tests as serial, as of now	*/
	return e_typeSerialTest;
}

/*	checking whether the test is DSP test or not. All the DSP tests get different processing	*/
int isTestDSPTest(dgnsCtrlTestInfo *testToExamine)
{
	/*	if unitOnSlot in defined range -> it is DSP test	*/
	if (testToExamine->testId == eBOARD_HOMOLOGATION_TEST)
		return 1;
	if ((testToExamine->testId == eDSP_DOWNLOAD_DIAG) || (testToExamine->testId == eDSP_CONNECTIVITY_DIAG))
		return 0;
	if (
			((testToExamine->unitOnSlot > 0)&&(testToExamine->unitOnSlot < 33))
			|| (testToExamine->unitOnSlot == 0x99)
		)
		return 1;
	else
		return 0;
}


/*	To find ,where in memory resides the part information about test we recieved from dsp	*/
dgnsCtrlTestInfo *	findTestInMemory(UINT32 ulSlotID, dgnsCtrlTestInfo *testToExamine)
{
	dgnsCtrlTestInfo *testPtr;
	int i = 0;
	
	/*	if we manage to find test that has same UnitOnSlot and TestId in our list, it means
	 * 	that we just recieved response for it. I find it in memory,and return the address,
	 * 	to update later on						*/
	while (dgnsGetTestFromList(ulSlotID, i++,&testPtr) > 0)
	{
		if ( (testPtr->unitOnSlot == testToExamine->unitOnSlot)
			&&(testPtr->testId == testToExamine->testId) )
			return testPtr;
	}
	return 0;
}


/*	Each DSP test has thread for itself. It should recieve list of tests to perform
 * 	for the DSP, send it, communicate with it (keep alive/get status/stop test) and
 * update the results per test	*/
#if 0
void dspTestThread(void)
{
	int i;
	char errDescr[100];
	dgnsDSPTestSession	*testSession;
	UINT32	msgSize;
	struct timeval tCurrentTime;
	dgnsTestResult diagTestRes; //this is needed for dsp load test
	
	
	
	/*How much dsps working	*/
	dspWorkingCount = 0;
	for (i = 1; i < num_OF_DSPS_IN_BOARD + 1 ; i++)
	{
		if (loadedDSPInformation[ulSlotId][i] != eDSPNotExistantOnBoard)
		/*	If the pointer indeed points to test list, count it as working dsp! 	*/
		if (isNotEmptyList(dspTestListCollection[uSlotId][i]))
		{
			dspWorkingCount++;
			if  (loadedDSPInformation[ulSlotId][i] == eDSPNotLoaded)
			{
				diagTestRes.testData = i;
				diagTestRes.errString[0] = 0;
				DspDownloadTest(&diagTestRes);
				if (diagTestRes.testResult == eStatFail)
				{
					
					if (diagTestRes.errString[0] == 0 )
						sprintf(errDescr,"DSP (%d) load failed.",i); //hasnt changed
					else
						strcpy(errDescr,diagTestRes.errString); //hasnt changed
					//else = DspDownloadTest set its own error message
					errReportError(ulSlotId, eDSP_CONNECTIVITY_DIAG,errDescr);
				}
				EmbSleep(10);
			}
			
			/*If this dsp is not working... dont count it */
			if (loadedDSPInformation[ulSlotId][i] == eDSPLoadFailed)
			{
				dspWorkingCount--;
			}
		}
	}
	printf("Counted %d DSPs,go:\n",dspWorkingCount);
			
	/*	If the time timestamp of current dsp = 0,it means its ok with timeout engine */
	for (i = 1; i < num_OF_DSPS_IN_BOARD + 1; i++)
		lastReqTimeStamp[ulSlotId][i] = 0;
	
	/*	We have 32 pointers to listcollections,created by addToDspTestStruct. Some are empty 
	 * 	Now ,we have to start the process for those ,who do have tests:*/
	
	for (i = 1; i < num_OF_DSPS_IN_BOARD +1 ; i++)
	{
		/*	If the pointer indeed points to test list, get it and send to DSP! 	*/
		if (isNotEmptyList(dspTestListCollection[uSlotId][i]))
		{
			if ((loadedDSPInformation[ulSlotId][i] == eDSPLoadAsked) || (loadedDSPInformation[ulSlotId][i] == eDSPLoaded))
			{
				testSession = (dgnsDSPTestSession*)dspTestListCollection[uSlotId][i];
				msgSize = sizeof(dgnsDSPTestSession) + ((SWAPL(testSession->numOfElements))*sizeof(dgnsCtrlTestInfo));
		//		printf("Asking dsp (%d) to start test with size: (%d).\n",i,msgSize);
				//SendDSPMessage(1,i,eDSP_START_SESSION,dspTestListCollection[i],msgSize);
				gettimeofday(&tCurrentTime,NULL);
				lastReqTimeStamp[ulSlotId][i] = (UINT32)tCurrentTime.tv_sec;
			}
		}
	}
}
#endif

int initAllDspRelevantStructures()
{
    int i;
	unsigned int j;
	for(i = 0; i < MAX_SLOT_NUM; i ++)
	{
		loadStreamInformation[i] = (dspStreamLoadTest*)malloc(sizeof(dspStreamLoadTest) * (num_OF_DSPS_IN_BOARD + 1));
		loadedDSPInformation[i] = (dspLoadState *)malloc(sizeof(dspLoadState) * (num_OF_DSPS_IN_BOARD + 1));
		dspTestListCollection[i] = (void**)(malloc(sizeof(void*)*(num_OF_DSPS_IN_BOARD + 1))); 
		errorOccured[i] = (INT32*)malloc(sizeof(INT32)*(num_OF_DSPS_IN_BOARD + 3));
		isDspSessionRunning[i] = (char*)malloc(sizeof(char) * (num_OF_DSPS_IN_BOARD + 1));
		lastReqTimeStamp[i] = (UINT32*)malloc(sizeof(UINT32)*(num_OF_DSPS_IN_BOARD + 1));
		massDspTestsStatus[i] = (UINT32*)malloc(sizeof(UINT32)*(num_OF_DSPS_IN_BOARD + 1));
		loadTestStatus[i] = (massTestDspStatus*) malloc(sizeof(massTestDspStatus)* (num_OF_DSPS_IN_BOARD + 1));	
		
		/*
		if (!loadStreamInformation)
			printf("loadStreamInformation");
		if(!loadedDSPInformation)
			printf("loadedDSPInformation");
		if(!dspTestListCollection)
				printf("*dspTestListCollection");
		if(!errorOccured)
				printf("errorOccured");
		if(!lastReqTimeStamp)
				printf("lastReqTimeStamp");
		*/
		
		if ((
			(loadStreamInformation[i]) &&
			(loadedDSPInformation[i]) &&
			(isDspSessionRunning[i]) &&
			(dspTestListCollection[i]) &&
			(errorOccured[i])	&&
			(lastReqTimeStamp[i]) &&
			(massDspTestsStatus[i]) &&
			(loadTestStatus[i])
			) == 0)
			return 0; //alloc of one of them failed.
		
		/*	Zeroize	all */
		for ( j = 0 ; j < num_OF_DSPS_IN_BOARD + 1 ; j++)
		{
			loadStreamInformation[i][j] = 0;
			loadedDSPInformation[i][j] = 0;
			errorOccured[i][j] = STATUS_UNKNOWN_YET;
			lastReqTimeStamp[i][j] = 0;
			massDspTestsStatus[i][j] = 0;
			isDspSessionRunning[i][j] = 0;
		}
		errorOccured[i][num_OF_DSPS_IN_BOARD + 1] = STATUS_UNKNOWN_YET; //it is longer by 2 places
		errorOccured[i][num_OF_DSPS_IN_BOARD + 2] = STATUS_UNKNOWN_YET;
	}
	
	return 1;
	
}

void freeAllDspRelevantStructures()
{
	int i;
	for(i = 0; i < MAX_SLOT_NUM; i ++)
	{
		if (loadStreamInformation[i])
			free(loadStreamInformation[i]);
		if (loadedDSPInformation[i])
			free(loadedDSPInformation[i]);
		if (isDspSessionRunning[i])
			free(isDspSessionRunning[i]);
		if (dspTestListCollection[i])
			free (dspTestListCollection[i]);
		if (errorOccured[i])
			free(errorOccured[i]);
		if (lastReqTimeStamp[i])
			free (lastReqTimeStamp[i]);
		if (massDspTestsStatus[i])
			free(massDspTestsStatus[i]);
		if (loadTestStatus[i])
			free(loadTestStatus[i]);
	}
}
