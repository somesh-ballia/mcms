#include <iostream>
#include <string.h>
#include "DataTypes.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "MplMcmsStructs.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include "DiagnosticsInfo.h"
#include "DiagnosticsErrHandle.h"
#include "IpmcInt.h"

using namespace std;

#include "EmaApi.h"
#include "DiagnosticsShared.h"

#define READ_REST_OF_REQUEST(CLASS,OBJECT) CLASS OBJECT;\
memset((void *)&OBJECT, 0, sizeof(CLASS));\
recv(m_descriptor,((char *)(&OBJECT)) + sizeof(EmaReqHeader),sizeof(CLASS) - sizeof(EmaReqHeader), 0)


#define E_CHASSIS_ALL_SYSTEMS (E_CHASSIS_TYPE_RMX2000 | E_CHASSIS_TYPE_RMX1500 | E_CHASSIS_TYPE_RMX2000)

dgnsTestInfo currentSystemTests[] =
{
	// Test ID                   		  	Test Name                       UNIT   			Loop  	   Quick 	Time   System   						Func Pointer

	// Memory Tests
	{eCNTL_DDR_MEMORY_DATA_BUS,				"Memory Data bus",			eUndefined,			TRUE,      	TRUE,		1,		E_CHASSIS_ALL_SYSTEMS,      CNTLMemoryDataBusTest},
	{eCNTL_DDR_MEMORY_ADDRESS_BUS,			"Memory Address bus",		eUndefined,			TRUE,      	TRUE,		5,		E_CHASSIS_ALL_SYSTEMS,     	CNTLMemoryAddressBusTest},
	{eCNTL_DDR_MEMORY_INTEGRITY ,			"Memory integrity",			eUndefined,			TRUE,      	FALSE,		90,		E_CHASSIS_ALL_SYSTEMS,      CNTLMemoryIntegrityTest},
	{eCNTL_DDR_MEMORY_ENERGY,				"Memory energy",			eUndefined,			TRUE,      	FALSE,		15,		E_CHASSIS_ALL_SYSTEMS,      CNTLMemoryEnergyTest},

	// Compact Flash Tests
	{eCNTL_CF_CREATE_DELETE_READ_WRITE,		"System create/del/read/write",		eUndefined,			TRUE,		TRUE,		10,		E_CHASSIS_ALL_SYSTEMS,      CNTLCompactFlashReadWriteTest},
	{eCNTL_CF_FILE_SYSTEM_CHECK,			"System file system check",			eUndefined,			TRUE,      	TRUE,		10,		E_CHASSIS_ALL_SYSTEMS,      CNTLCompactFlashFSCheck},
	{eCNTL_CF_MD5				,			"System MD5 on version files",		eUndefined,			TRUE,      	TRUE,		60,		E_CHASSIS_ALL_SYSTEMS,      CNTLCompactFlashMD5Test},

	// Hard Disk Tests
	{eCNTL_HARD_DISK_SMART_CHECK,			"Hard disk smart check",		eUndefined,			TRUE,		TRUE,		1,		E_CHASSIS_ALL_SYSTEMS, 		CNTLHardDiskSmartCheck},
	{eCNTL_HARD_DISK_READ_WRITE,	        "Hard disk create/del/read/write",	eUndefined,			TRUE,		TRUE,		5,		E_CHASSIS_ALL_SYSTEMS, 	CNTLHardDiskReadWriteCheck},

	//USB
	{eCNTL_USB_READ_WRITE,	                "USB create/del/read/write",	eUndefined,			TRUE,		TRUE,		5,		E_CHASSIS_ALL_SYSTEMS, 		CNTLUSBReadWriteCheck},	

	//IPMC Tests
	{eCNTL_IPMC_CONNECTION,					"IPMC Connection Test",	     eUndefined,			TRUE,		TRUE,		1,		E_CHASSIS_ALL_SYSTEMS, 		CNTLIPMCConntionTest},

	// Last - Invalid Test
	{0xffff,								"Empty",						0,				0,			0,			0,		0,							NULL}
};

dgnsTestSession diagCurrentSession;
int wasTestAskedToStop;
static int g_flagAutoTest = 0;

TStructToStr *atEnterDiagModeIndSpecArray_Net;
TStructToStr *atTestListIndSpecArray_Net;
TStructToStr *atStartTestIndSpecArray_Net;
TStructToStr *atStopTestIndSpecArray_Net;
TStructToStr *atTestStatusIndSpecArray_Net;
TStructToStr *atUnitStatusListIndSpecArray_Net;
TStructToStr *atErrorListIndSpecArray_Net;


////////////////////////////////////////////////////////////////////

int SendGetTestListInd(EmaIndHeader & ema_ind_header);
int SendStartTestInd(EmaIndHeader & ema_ind_header);
int SendGetTestStatusInd(EmaIndHeader & ema_ind_header);
int SendStopTestInd(EmaIndHeader & ema_ind_header);
int SendGetUnitsStateInd(EmaIndHeader & ema_ind_header);
int SendStartDiagnosticsMode(EmaIndHeader & ema_ind_header);
int SendGetErrorListInd(EmaIndHeader & ema_ind_header);
void OnFailureToReadFromSocket(int errnoCode);
void *StartTestHandling(void *arg);
void ConvertSpecArraytoNetworkOrder(TStructToStr *pOrig, uint size, TStructToStr* pNew);
void InitSpecArray_Net();
void FreeSpecArray_Net();
int TestFlashReadWrite();
int FlashFSCheck();
int FlashMD5Check();
void CNTLCompactFlashMD5Test(dgnsTestResult *diagTestRes);
void* diagModePollThread(void* arg);
void StartAutoTest();
void StartupIperfServ()
{
	/*kill httpd and apache since we need to use port 80*/
	system("killall -9 ApacheModule");
	system("killall -9 httpd");
	SLEEP(1000);
	system("killall -9 iperf");
	SLEEP(1000);
	/*stop the firewall and allow all the traffic to the system*/
	/*for VNGR-20536 and VNGR-22149*/
	system("iptables -I cs2fw -p tcp -m multiport --dports 1:65535 -j ACCEPT");
	
	SLEEP(1000);
	/*startup the iperf*/
	system("/usr/bin/iperf -s -p 80 &");

}

/*
 * copy /config/mcms/IPServiceList.xml to /output/media_rec/ folder
 * Then RTM-IP can mount it by NFS to get all the IP address for signalling and media
*/
void CopyIPListforRTM()
{
	/* for VNGR-20536: just copy IPServiceList.xml since our test is under none multipleservices mode */
	system("cp -f /config/mcms/IPServiceList.xml /output/media_rec/");
	return;

}

////////////////////////////////////////////////////////////////////

int m_descriptor;

int main()
{

    cout << "Starting Diagnostics" << std::endl;
	//FlashFSCheck();
	//TestFlashReadWrite();
	//FlashMD5Check();
	//////////////////////////////////////////////
	/*  VNGR-17402 4.10.10 removed by Rachel Cohen */
	/*pthread_t threadPoll;
    printf("create diagModePollThread\n");
    pthread_create(&threadPoll, NULL, diagModePollThread, NULL);
    pthread_detach(threadPoll);*/
	//////////////////////////////////////////////
    EmaReqHeader req_header;
    BOOL failed = 0;
	int err=0;
	m_descriptor = socket( AF_INET, SOCK_STREAM, 0 );
	if( m_descriptor == -1 )
	{
		perror("CClientSocket::ConfigureSocketConnection - socket not created");
	}

    cout << "Init Spec Array into Netword order" << std::endl;
    InitSpecArray_Net();

    struct	sockaddr_in		server_adr;

	server_adr.sin_family		= AF_INET;
	server_adr.sin_addr.s_addr	= inet_addr("169.254.128.16");
	server_adr.sin_port			= htons(3333);

	int res = connect( m_descriptor, (struct sockaddr *) &server_adr, sizeof(server_adr));

    if (res ==0)
    {

        do
        {
            memset((void *)&req_header, 0, sizeof(EmaReqHeader));
            res = recv(m_descriptor, &req_header , sizeof(EmaReqHeader), 0);
            if(-1 == res)
            {
                cout << "diagnostics: recv failed" << endl;
                int errnoCode = errno;
                OnFailureToReadFromSocket(errnoCode);
                FreeSpecArray_Net();
                return 1;

            }

            EmaIndHeader ind_header = {0,
                                       htonl(req_header.msgId),
                                       htonl(req_header.slotId),
                                       htonl(eEmaStatOk),
                                       "OK"};

            switch (req_header.opcode)
            {
                case EMA_ENTER_DIAG_MODE_REQ:
                    cout << "diagnostics: EMA_ENTER_DIAG_MODE_REQ" << endl;

                    failed = SendStartDiagnosticsMode(ind_header);
                    return 2; // process must start again since switch is resetting (version 5.0)

                    break;
                case EMA_GET_TEST_LIST_REQ:
                    cout << "diagnostics: EMA_GET_TEST_LIST_REQ" << endl;
                    failed = SendGetTestListInd(ind_header);
                    break;
                case EMA_START_TEST_REQ:
                    cout << "diagnostics: EMA_START_TEST_REQ" << endl;
                    failed = SendStartTestInd(ind_header);
                    break;
                case EMA_GET_TEST_STATUS_REQ:
                    cout << "diagnostics: EMA_GET_TEST_STATUS_REQ" << endl;
                    failed = SendGetTestStatusInd(ind_header);
                    break;
                case EMA_STOP_TEST_REQ:
                    cout << "diagnostics: EMA_STOP_TEST_REQ" << endl;
                    failed = SendStopTestInd(ind_header);
                    break;
                case EMA_GET_UNITS_STATE_REQ:
                    cout << "diagnostics: EMA_GET_UNITS_STATE_REQ" << endl;
                    failed = SendGetUnitsStateInd(ind_header);
                    break;
				case EMA_GET_ERROR_LIST_REQ:
                    cout << "diagnostics: EMA_GET_ERROR_LIST_REQ" << endl;
					failed = SendGetErrorListInd(ind_header);
					break;
                default:
                    cout << "diagnostics: unknown OPCODE:" << req_header.opcode << endl;
                    break;
            }

			////////////////////////
			if(g_flagAutoTest == 1)
			{
				g_flagAutoTest = 0;
				StartAutoTest();
			}
			////////////////////////

        } while (!failed);

    }
    else
    {
        perror("Socket closed on switch");
    }
    return 0;
}

void ConvertSpecArraytoNetworkOrder(TStructToStr *pOrig, uint size, TStructToStr** pNew)
{
	int i=0;
	TStructToStr *tmp;

	int nSize = size/sizeof(TStructToStr);
	if (*pNew != NULL) {
		printf("the new spec Array is not NULL\n");
		return;
	}
	tmp = (TStructToStr *)malloc(size);
	if (tmp == NULL) {
		printf("Can not allocate memory for spec arrary\n");
		return;
	}
	memcpy(tmp, pOrig, size);
	for (i =0; i< nSize; i++){
		tmp[i].varType = htonl(pOrig[i].varType);
		tmp[i].varCount = htonl(pOrig[i].varCount);
		tmp[i].jumpOffst = htonl(pOrig[i].jumpOffst);
	}
	*pNew = tmp;
	printf("Convert %d items in a spec array\n", nSize);
	return;
}

void InitSpecArray_Net()
{
	atEnterDiagModeIndSpecArray_Net = NULL;
	ConvertSpecArraytoNetworkOrder(atEnterDiagModeIndSpecArray,
			sizeof(atEnterDiagModeIndSpecArray),
			&atEnterDiagModeIndSpecArray_Net);

	atTestListIndSpecArray_Net = NULL;
	ConvertSpecArraytoNetworkOrder(atTestListIndSpecArray,
			sizeof(atTestListIndSpecArray),
			&atTestListIndSpecArray_Net);

	atStartTestIndSpecArray_Net = NULL;
	ConvertSpecArraytoNetworkOrder(atStartTestIndSpecArray,
			sizeof(atStartTestIndSpecArray),
			&atStartTestIndSpecArray_Net);

	atStopTestIndSpecArray_Net = NULL;
	ConvertSpecArraytoNetworkOrder(atStopTestIndSpecArray,
			sizeof(atStopTestIndSpecArray),
			&atStopTestIndSpecArray_Net);

	atTestStatusIndSpecArray_Net = NULL;
	ConvertSpecArraytoNetworkOrder(atTestStatusIndSpecArray,
			sizeof(atTestStatusIndSpecArray),
			&atTestStatusIndSpecArray_Net);

	atUnitStatusListIndSpecArray_Net = NULL;
	ConvertSpecArraytoNetworkOrder(atUnitStatusListIndSpecArray,
			sizeof(atUnitStatusListIndSpecArray),
			&atUnitStatusListIndSpecArray_Net);

	atErrorListIndSpecArray_Net = NULL;
	ConvertSpecArraytoNetworkOrder(atErrorListIndSpecArray,
			sizeof(atErrorListIndSpecArray),
			&atErrorListIndSpecArray_Net);
}

void FreeSpecArray_Net()
{
	free(atEnterDiagModeIndSpecArray_Net);
	atEnterDiagModeIndSpecArray_Net = NULL;

	free(atTestListIndSpecArray_Net);
	atTestListIndSpecArray_Net = NULL;

	free(atStartTestIndSpecArray_Net);
	atStartTestIndSpecArray_Net = NULL;

	free(atStopTestIndSpecArray_Net);
	atStopTestIndSpecArray_Net = NULL;

	free(atTestStatusIndSpecArray_Net);
	atTestStatusIndSpecArray_Net = NULL;

	free(atUnitStatusListIndSpecArray_Net);
	atUnitStatusListIndSpecArray_Net = NULL;

	free(atErrorListIndSpecArray_Net);
	atErrorListIndSpecArray_Net = NULL;
}

////////////////////////////////////////////////////////////////////
void SendEmaIndecation(void * str2Str, UINT32 sizeOfStr2Str,
                       void * emaInd, UINT32 sizeOfInd)
{
    TPKT_HEADER_S Tpkt_Header;
    Tpkt_Header.version_num = 3;
    Tpkt_Header.reserved = 0;
    Tpkt_Header.payload_len=htons((sizeof(TPKT_HEADER_S)
                             + sizeof(EmaIndGeneralDescHeader)+
                             sizeOfStr2Str +
                             sizeOfInd));


    EmaIndGeneralDescHeader indGeneralHeader;
    indGeneralHeader.msgOffset = htonl(sizeof(EmaIndGeneralDescHeader)+
                                        sizeOfStr2Str);

    send(m_descriptor, &Tpkt_Header , sizeof(TPKT_HEADER_S) , 0);
    send(m_descriptor, &indGeneralHeader , sizeof(EmaIndGeneralDescHeader), MSG_MORE);
    send(m_descriptor, (const char *)str2Str, sizeOfStr2Str,MSG_MORE);
    send(m_descriptor, (const char *)emaInd,sizeOfInd , MSG_EOR);
}

////////////////////////////////////////////////////////////////////
int SendGetTestListInd(EmaIndHeader & ema_ind_header)
{
    EmaTestListInd  testListInd;
    ema_ind_header.ulOpcode = htonl(EMA_GET_TEST_LIST_IND);
    testListInd.tEmaIndHdr = ema_ind_header;

	int nCount = sizeof(currentSystemTests)/sizeof(dgnsTestInfo) -1;
	printf("the count of test case list = %d\n",nCount);

    testListInd.tTestList.ulNumOfElem = htonl(nCount);
    testListInd.tTestList.ulNumOfElemFields = htonl(6);
	int i;
    for(i = 0 ; (i < nCount) && (currentSystemTests[i].TestId != 0xffff) ; i++)
    {
        strcpy(testListInd.tTestList.tTestData[i].testName, currentSystemTests[i].TestName);
		testListInd.tTestList.tTestData[i].unTestID = 		htonl(currentSystemTests[i].TestId);
		testListInd.tTestList.tTestData[i].unQuick =  		htonl(currentSystemTests[i].isQuickVersion);
		testListInd.tTestList.tTestData[i].unLoop =   		htonl(currentSystemTests[i].canBeLooped);
		testListInd.tTestList.tTestData[i].unEstimateTime = htonl(currentSystemTests[i].esimatedRunTime);
		testListInd.tTestList.tTestData[i].unTestOn = 		htonl(currentSystemTests[i].TestOn);
    }

    SendEmaIndecation(atTestListIndSpecArray_Net,sizeof(atTestListIndSpecArray),
    		&testListInd, sizeof(testListInd));

    return 0;

}

void* diagModePollThread(void* arg)
{
	fprintf(stderr,"Enter the diagModePollThread\n");

	static int ii = 0;
	while(1)
	{
		ii++;
		int nRet = IpmcIsDiagMode();
		//diagnostic mode
		if(1 == nRet)
		{
			fprintf(stderr,"Enter the diagMode by standby button\n");		
			EmaIndHeader ema_ind_header;
			SendStartDiagnosticsMode(ema_ind_header);
			g_flagAutoTest = 1;
			break;
		}
		//normal mode
		else if(2 == nRet)
		{
			break;
		}
		else
		{
	        fprintf(stderr,"diagModePollThread is working\n");			
			SLEEP(1000);
		}

		if(ii>5)
		{
			break;
		}
	}
	
	fprintf(stderr,"Exit the diagModePollThread\n");	
	pthread_exit(NULL);
}


void StartAutoTest()
{
	dgnsClearTests();
	errClearErrorLog();

	wasTestAskedToStop = 0;
	diagCurrentSession.hasStarted = 1;
	diagCurrentSession.numOfLoops = 99;
	diagCurrentSession.stopOnFail = 0;
	diagCurrentSession.isQuickSession = 0;

	int nCount = sizeof(currentSystemTests)/sizeof(dgnsTestInfo) -1;
	int i;
	for(i = 0 ; (i < nCount) && (currentSystemTests[i].TestId != 0xffff) ; i++)
	{
		int test_id = currentSystemTests[i].TestId;
		int unit_id = 255;
		printf("inserting into queue test %u on slot unit id %u\n", test_id, unit_id);
		dgnsTestAddTest(test_id, unit_id);
	}
				
	pthread_t threadTestWorker;
	printf("creating thread to perform the test\n");
	pthread_create(&threadTestWorker, NULL, StartTestHandling, NULL);
	pthread_detach(threadTestWorker);
}


////////////////////////////////////////////////////////////////////
int SendStartTestInd(EmaIndHeader & ema_ind_header)
{
	pthread_t threadTestWorker;

	/* to read the message*/
	READ_REST_OF_REQUEST(EmaStartTestReqMsg,startTestReqMsg);
    /*to reply the indication first*/
	EmaStartTestInd startTestInd;
    ema_ind_header.ulOpcode = htonl(EMA_START_TEST_IND);
    startTestInd.tEmaIndHdr = ema_ind_header;
    SendEmaIndecation(atStartTestIndSpecArray_Net,sizeof(atStartTestIndSpecArray),
    		&startTestInd, sizeof(startTestInd));

    if (diagCurrentSession.hasStarted == 1) {
    	printf("The test is already in progress. Cannot proceed this request.\n");
    	return 0;
    }
    /**/
    dgnsClearTests();
	errClearErrorLog();

    wasTestAskedToStop = 0;
    diagCurrentSession.hasStarted = 1;
    diagCurrentSession.numOfLoops = startTestReqMsg.unNumOfReqLoops;
    if ( diagCurrentSession.numOfLoops == 0)
    	 diagCurrentSession.numOfLoops = 1;
    diagCurrentSession.stopOnFail = startTestReqMsg.unStopOnFail;
    diagCurrentSession.isQuickSession = 0;
    diagCurrentSession.slotId = ntohl(ema_ind_header.ulSlotID);

    for (UINT32 i=0; i<startTestReqMsg.tTStartTestList.ulNumOfElem; i++)
    {
        // we ignore the unit id - we hold only one unit (0)
        int test_id = startTestReqMsg.tTStartTestList.tStartTestReqParams[i].testId;
        int unit_id = 255;
        printf("inserting into queue test %u on slot unit id %u\n", test_id, unit_id);
        dgnsTestAddTest(test_id, unit_id);
    }

    printf("creating thread to perform the test\n");
    pthread_create(&threadTestWorker, NULL, StartTestHandling, NULL);
    pthread_detach(threadTestWorker);

    return 0;
}

void* StartTestHandling(void* arg)
{
	time_t testStartTime;
	dgnsCtrlTestInfo* currentTest;
	dgnsTestInfo *testInfo;
	dgnsTestResult testResult;
	int loopsToBeDone = 0;

    int nRedReturn = 0;
    int nGreenReturn = 0;
    int nAmberReturn = 0;

	nRedReturn   = LedInterface(eRed,eTurnOff);
	nGreenReturn = LedInterface(eGreen,eFlickering);
	nAmberReturn = LedInterface(eAmber,eFlickering);
	fprintf(stderr,"Start test:Return value of setting LEDs, red = %d, green = %d, amber = %d\n",nRedReturn, nGreenReturn, nAmberReturn);

	currentTest = diagCurrentSession.firstTest;
	testStartTime = time(NULL);
	diagCurrentSession.testSessionStartTime = (UINT32)testStartTime;

	while ((wasTestAskedToStop == 0 ) &&
		((loopsToBeDone < diagCurrentSession.numOfLoops) || (diagCurrentSession.numOfLoops == 99)))
	{
		while ((currentTest != NULL) && (wasTestAskedToStop == 0)){
			testStartTime = time(NULL);
			currentTest->testStartTime = (UINT32)testStartTime;
			currentTest->isActiveNow = 1;

			testInfo = testIdToTest(currentTest->testId, currentSystemTests);
			printf("start the test on test_id %d, name: %s\n",
					currentTest->testId,
					testInfo->TestName);

			testResult.testData = currentTest->testId;
			testInfo->testFunction(&testResult);

			if (testResult.testResult == eStatFail) {
				currentTest->failTests++;
				errReportError(currentTest->testId, testResult.errString);
				if (diagCurrentSession.stopOnFail == 1){
					wasTestAskedToStop = 1;
				}
			    nRedReturn = 0;
				nRedReturn   = LedInterface(eRed,eTurnOn);
				fprintf(stderr,"Testing error:Return value of setting LEDs, red = %d\n",nRedReturn);

			} else {
				currentTest->successTests++;
			}

			currentTest->isActiveNow = 0;
			testStartTime = time(NULL);
			currentTest->duration =+ ((UINT32)testStartTime - currentTest->testStartTime);

			printf("End of the test on test_id: %d; test_result: %s; test duration: %u\n",
					currentTest->testId,
					(testResult.testResult == eStatOk)?"OK":"Fail",
					currentTest->duration);

			currentTest->loopsDone++;
			currentTest = currentTest->nextTest;
		}
		currentTest = diagCurrentSession.firstTest;
		loopsToBeDone++;
		printf("%d loops test is done\n", loopsToBeDone);
	}
	diagCurrentSession.hasStarted = 0;
	wasTestAskedToStop = 1;
	testStartTime = time(NULL);
	diagCurrentSession.testSessionDurationTime = (UINT32)testStartTime - diagCurrentSession.testSessionStartTime;

    nGreenReturn = 0;
    nAmberReturn = 0;

	nGreenReturn = LedInterface(eGreen,eTurnOff);
	nAmberReturn = LedInterface(eAmber,eTurnOff);
	fprintf(stderr,"End test:Return value of setting LEDs, green = %d, amber = %d\n",nGreenReturn, nAmberReturn);

	printf("Test done(loops: %d time:(%d)seconds\n",
			loopsToBeDone,
			diagCurrentSession.testSessionDurationTime);

	pthread_exit(NULL);
}

////////////////////////////////////////////////////////////////////
int SendGetTestStatusInd(EmaIndHeader & ema_ind_header)
{
	time_t currentTime;
	TTestStatusList *pTestStatusList = NULL;
	dgnsTestInfo	*testListInfo = NULL;
	dgnsCtrlTestInfo *testInfoStruct = NULL;

	EmaGetTestStatusInd TestStatusInd;
	UINT32	i,ulNumOfElem;
	ema_ind_header.ulOpcode = htonl(EMA_GET_TEST_STATUS_IND);
	TestStatusInd.tEmaIndHdr = ema_ind_header;
	pTestStatusList = &(TestStatusInd.tTestStatusList);

	TestStatusInd.tTestGeneralParams.unNumOfReqLoops = htonl(diagCurrentSession.numOfLoops);
	TestStatusInd.tTestGeneralParams.unStopOnFail = htonl(diagCurrentSession.stopOnFail);

	//we check whether we are in progress currently,and if yes -
	// update the time accordingly.
	if (diagCurrentSession.hasStarted == 1)
	{
		currentTime = time(NULL);
		TestStatusInd.tTestGeneralParams.unDuration = htonl((UINT32)currentTime - diagCurrentSession.testSessionStartTime);
	}
	else
	{	//if it is not in progress currently,it means we already finished,and
		//can use the time inside the structure
		TestStatusInd.tTestGeneralParams.unDuration = htonl(diagCurrentSession.testSessionDurationTime);
	}

	pTestStatusList->ulNumOfElem = htonl(0);
	pTestStatusList->ulNumOfElemFields = htonl(9);
	ulNumOfElem = 0;

	for(i = 0; (i < MAX_NUM_OF_TESTS) && (dgnsGetTestFromList(i,&testInfoStruct)>0); i++)
	{
		testListInfo = testIdToTest(testInfoStruct->testId,currentSystemTests);
		strcpy(pTestStatusList->tTestStatusData[i].testName, testListInfo->TestName);
		pTestStatusList->tTestStatusData[i].unTestID = htonl(testInfoStruct->testId);
		pTestStatusList->tTestStatusData[i].unUnitId = htonl(testInfoStruct->unitOnSlot);
		pTestStatusList->tTestStatusData[i].unLoopNum = htonl(testInfoStruct->loopsDone);
		pTestStatusList->tTestStatusData[i].unTestState = htonl(testInfoStruct->isActiveNow);
		pTestStatusList->tTestStatusData[i].unNumOfSuccesses = htonl(testInfoStruct->successTests);
		pTestStatusList->tTestStatusData[i].unNumOfFailures = htonl(testInfoStruct->failTests);


		currentTime = time(NULL);
		// Calculate the time that passed since the beginning of the test
		if (testInfoStruct->isActiveNow == 1)
			pTestStatusList->tTestStatusData[i].unDuration = htonl((UINT32)currentTime - testInfoStruct->testStartTime);
		else
			pTestStatusList->tTestStatusData[i].unDuration = htonl(testInfoStruct->duration);

		pTestStatusList->tTestStatusData[i].unQuick = htonl(0);

		printf("report test name: %s, testID: %u, unitID: %u, loop: %u,Statue: %u, numSucee: %u, numFail: %u, Dur: %u(s)\n",
				pTestStatusList->tTestStatusData[i].testName,
				ntohl(pTestStatusList->tTestStatusData[i].unTestID),
				ntohl(pTestStatusList->tTestStatusData[i].unUnitId),
				ntohl(pTestStatusList->tTestStatusData[i].unLoopNum),
				ntohl(pTestStatusList->tTestStatusData[i].unTestState),
				ntohl(pTestStatusList->tTestStatusData[i].unNumOfSuccesses),
				ntohl(pTestStatusList->tTestStatusData[i].unNumOfFailures),
				ntohl(pTestStatusList->tTestStatusData[i].unDuration));


		ulNumOfElem++;
	}
	printf("prepared %d tests to report\n",ulNumOfElem);
	pTestStatusList->ulNumOfElem = htonl(ulNumOfElem);

	SendEmaIndecation(atTestStatusIndSpecArray_Net, sizeof(atTestStatusIndSpecArray),
			&TestStatusInd, sizeof(TestStatusInd));

    return 0;

}

int SendGetErrorListInd(EmaIndHeader & ema_ind_header)
{
	EmaErrorListInd errorListInd;
	UINT32	i,ulNumOfElem;
	TErrorList * ptErrorList = NULL;
	errList* 				errorInfo;

	ema_ind_header.ulOpcode = htonl(EMA_GET_ERROR_LIST_IND);
	errorListInd.tEmaIndHdr = ema_ind_header;

	ptErrorList = &(errorListInd.tErrorList);

	ptErrorList->ulNumOfElem = htonl(0);
	ptErrorList->ulNumOfElemFields = htonl(2);
	for (i = 0; (i < MAX_NUM_OF_ERRORS_TO_REPORT)&&(errGetError(i,&errorInfo)); i++)
	{
		strncpy(ptErrorList->errors[i].errorString, errorInfo->errDescr,sizeof(ptErrorList->errors[i].errorString) - 1);
		ptErrorList->errors[i].errorString[sizeof(ptErrorList->errors[i].errorString) - 1] = '\0';
		ptErrorList->errors[i].testId = htonl(errorInfo->errTestId);
		printf("SendGetErrorListInd::send error list, testId = %d,errorString = %s\n", errorInfo->errTestId,errorInfo->errDescr);
	}
	ptErrorList->ulNumOfElem = htonl(i);
	printf("SendGetErrorListInd::,ptErrorList->ulNumOfElem = %d\n",i);

	errClearErrorLog();

	SendEmaIndecation(atErrorListIndSpecArray_Net, sizeof(atErrorListIndSpecArray),
			&errorListInd, sizeof(errorListInd));

	return 0;
}

////////////////////////////////////////////////////////////////////
int SendGetUnitsStateInd(EmaIndHeader & ema_ind_header)
{
#if 0
	EmaGetUnitsStatusInd getUnitStatusInd;
    ema_ind_header.ulOpcode = htonl(EMA_GET_UNITS_STATE_IND);
    getUnitStatusInd.tEmaIndHdr = ema_ind_header;

    // PUT HERE CODE TO INIT getUnitStatusInd

    SEND_EMA_IND(atTestListIndSpecArray,getUnitStatusInd);
#endif
    return 0;

}

////////////////////////////////////////////////////////////////////
int SendStartDiagnosticsMode(EmaIndHeader & ema_ind_header)
{
	/*fix VNGR-22915. To add 2s delay to make sure the IPMC interface will send out the message*/
	/*We can not send more than 1 command to IPMC interface per 1s*/

	SLEEP(1000);
	system("/mcms/Bin/McuCmd change_led McuMngr red off");
	SLEEP(1500);
	system("/mcms/Bin/McuCmd change_led McuMngr red off");
	SLEEP(1500);
    system("/mcms/Bin/McuCmd change_led McuMngr green off");
	SLEEP(1500);
    system("/mcms/Bin/McuCmd change_led McuMngr amber flickering");
	SLEEP(5000);

    system("/mcms/Bin/McuCmd Enter_Diagnostics McmsDaemon");

    printf("Start iperf with port 80\n");
    StartupIperfServ();
    printf("copy IPMultipleServicesList.xml to /output/media_rec/ \n");
    CopyIPListforRTM();
    return 0;

}

////////////////////////////////////////////////////////////////////
int SendStopTestInd(EmaIndHeader & ema_ind_header)
{
    EmaStopTestInd stopTestInd;
    ema_ind_header.ulOpcode = htonl(EMA_STOP_TEST_IND);
    stopTestInd.tEmaIndHdr = ema_ind_header;

    wasTestAskedToStop = 1;

    SendEmaIndecation(atStopTestIndSpecArray_Net,sizeof(atStopTestIndSpecArray),
    		&stopTestInd, sizeof(stopTestInd));

    return 0;
}

////////////////////////////////////////////////////////////////////
void OnFailureToReadFromSocket(int errnoCode)
{
    const char *errnoName = "None";
    const char *errnoDesc = "None";

    switch(errnoCode)
    {
        case EBADF:
            errnoName = "EBADF";
            errnoDesc = "The argument s is an invalid descriptor";
            break;

        case ECONNREFUSED:
            errnoName = "ECONNREFUSED";
            errnoDesc = "A  remote  host  refused to allow the network connection";
            break;

        case ENOTCONN:
            errnoName = "ENOTCONN";
            errnoDesc = "The socket is associated with  a  connection-oriented  protocol and has not been connected";
            break;

        case ENOTSOCK:
            errnoName = "ENOTSOCK";
            errnoDesc = "The argument s does not refer to a socket";
            break;

        case EAGAIN:
            errnoName = "EAGAIN";
            errnoDesc = "The  socket  is  marked  non-blocking and the receive operation would block, \
or a receive timeout had been set and the  timeout expired before data was received.";
            break;

        case EINTR:
            errnoName = "EINTR";
            errnoDesc = "The  receive was interrupted by delivery of a signal before any data were available";
            break;

        case EFAULT:
            errnoName = "EFAULT";
            errnoDesc = "The receive  buffer  pointer(s)  point  outside  the  process's address space";
            break;

        case EINVAL:
            errnoName = "EINVAL";
            errnoDesc = "Invalid argument passed";
            break;

        case ENOMEM:
            errnoName = "ENOMEM";
            errnoDesc = "Could not allocate memory for recvmsg";
            break;

        default:
            errnoName = "Unknown";
            errnoDesc = "Unknown";
            break;
    }

    cout << "Failed to read from socket (-1 = recv )" << endl
         << " errno : " << errnoCode << ":" << errnoName << " - " << errnoDesc << endl;
}




