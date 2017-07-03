/*==================================================================================================*/
/*            Copyright     ???    2001  Polycom Israel, Ltd. All rights reserved                   */
/*--------------------------------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary   information of             */
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form   */
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without         */
/* prior written authorization from Polycom Israel Ltd.                                             */
/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* FILE:     	McmsSimTools.c																	    */
/* PROJECT:  	McmsSim                                                                             */
/* PROGRAMMER:  Bracha Schushan                                                                     */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                                                  */
/*--------------------------------------------------------------------------------------------------*/
/*         |                 |  			                                                        */
/*==================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "DiagDataTypes.h"  
#include "SharedDefines.h"
#include "McmsApi.h"
#include "SystemInfo.h"
#include "DiagnosticsApiExt.h"
#include "DiagnosticsApi.h"
#include "DiagnosticsShared.h"
//#include "CardsStructs.h"
#include "SocketApiTypes.h" 
#include "LinuxSystemCallsApi.h"
#include "SocketApiWrapExt.h"
#include "StatusesGeneral.h"
#include "Print.h"
#include "DbgCfg.h"
#include "EmaShared.h"
#include "timers.h"
#include "tools.h"
#include "McmsCom.h"
#include "dspTestCtrl.h"

extern int usb_flag;
extern unsigned int currentLogLoopNumber;
//extern e_TypeOfTest	currentTestType;
UINT8	tpcktBugOverrideBuffer[200];
UINT32	tpcktBugOverrideLen;

// Threads
pthread_t SwitchDiagListenThreadId;
pthread_t SwitchDiagTestsThreadId;
pthread_t SwitchCheckSW2ButtonThread;
extern dgnsTestInfo currentSystemTests[];
extern UINT32	wasTestAskedToStop[];	//need to set this variable from DiagnosticsApi.c , so that diag thread
									//could refer to it, if needed
//INT32 	  l_SwitchTestsQueue;
extern INT32 			l_PrintQueue;
extern pthread_t 		PrintThreadId;
extern pthread_t 		TimerManagerThreadId;
extern t_TcpConnParams	TcpConnection[eMaxTcpConnections];
extern MFA_DBG_S	tMfaDbgInfo;
UINT32	IAmChildProcess = NO;

#define INTERNAL_TEST_REQUEST_OPCODE					987654
// Tests
#define MEMORY_TEST_SIZE_OF_MEMORY						10
#define MEMORY_TEST_NUM_OF_LOOPS						1
#define DIAG_ERROR_LOG_FILE								"/mnt/mfa_cm_fs/SwitchDiag_Error_Log.txt"

SIM_TEST_REPORT_S	switchMemoryTest;
SIM_TEST_REPORT_S	switchNetworkTest;

extern TESTS_DATABASE	TestsDatabase;
extern APIUBOOL memoryTestFailure;
extern APIUBOOL pingTestFailure;
APIUBOOL switchNetworkTestFailure = NO;

SIM_ERROR_BUFFER	errorBuffer;

APIUBOOL			inLoopMode;
UINT32 				testNum;
UINT32 				loopCounter;
UINT32 				EmaLoopCounter = 1;
UINT32				startTestValue;
UINT32				currentTestId;
struct timeval 		tTimer_vals;

APIUBOOL diagFlag = NO;
int wereDspsLoaded;

void isToStartTestsThread();
//void InitErrorBufferDB();
void EmaLoopTests();
void DiagnosticsMain();
//void WriteTestReportFile(char* file_name);
void WriteReportFile();
void UpdateErrorBuffer();
//void SwitchDiagTestsThread();
//INT32 ReadShelfSlotId(char *pShelfSlots,char maxLength);

extern char g_diagResultFileName[512];
extern char g_diagUSBResultFileName[512];

const char *testTypeStr[] = 
{
	"SWITCH MEMORY TEST",
	"SWITCH LAN TEST"
};

extern pthread_t CreateThreadWrapper(void *pFuncAddr, const char *pFuncName, UINT32 ulPriority);
extern void SwitchDiagListenThread();
extern char* FileNameWithPath(char* file_name, char* file_path);
extern void PrintThread();
extern void TimerMngrThread();
extern void ConnectToLoggerTask();
extern void ParseMfaDbgCfg();
extern void InitDefaultMfaDbg();
extern void InitSchedStruct_Other();
extern eChassisType chassisType;

void EnterDiagnosticsMode()
{
	pid_t	childPID;
	MfaBoardPrint(CM_INITIATOR_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "EnterDiagnosticsMode");

	// Create diagnostics server thread
	SwitchDiagListenThreadId  = CreateThread(SwitchDiagListenThread, 2);
	MfaBoardPrint(CM_INITIATOR_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(EnterDiagnosticsMode): SwitchDiagListenThreadId = %d",SwitchDiagListenThreadId);
	// Init diagnostics process process. Return without sending any message
	MfaBoardPrint(	TCP_SERVER_PRINT,
					PRINT_LEVEL_ERROR,
					PRINT_TO_TERMINAL,
					"(EnterDiagnosticsMode): Create process DiagnosticsMain");

	//childPID = CreateProcessWithFunc(DiagnosticsMain);
	DiagnosticsMain();
}

void prepareDspInfrastructure()
{
		UINT32 i;
	
/*
		UINT32	unBoardHardVer;
		unSubBoardId = MAIN_SUB_BOARD;
		API_setEnv("APPLICATION", "./diagnostics");
		
					

		InitTraceLogger();
		InitIpmcMutex();
		SetZeroMessageParam();
		
	//	InitSchedStruct();
		InitDefaultValues();
		GetE2promInformation(SAVE_EPROM_INFORMATION);
		unBoardHardVer = GetBoardHardwareVer(SAVE_EPROM_INFORMATION);
		SetBoardHardVer(unBoardHardVer);
		GetBoardDspClock(SAVE_EPROM_INFORMATION);
		CheckIsStandAloneEnv();
		 
		tmp_BoardType = GetBoardType(SAVE_EPROM_INFORMATION);
		SetMfaAssemblyType(GetBoardDescription(SAVE_EPROM_INFORMATION));
			
		IpmcGetFruLedProperties();
		 	
	//			CreateFirstMbx(); //have to create keepalive queues
		//		CreateAllMbx(); 
			 	
		//	 	UpdateFilesFromVersionTxt();

			InitHardware();
			printf("success initializing!!,starting burn\n");
			
		
			SetWatchDogFunc(0xff);
			
			
			l_SenderQueue =  CreateMessageQueue();
			if (l_SenderQueue == -1)
				printf("cant create sending mbx\n");
			else
			    dsp_EntityToQueue[eApiSenderThread] = l_SenderQueue;
			   
			l_DispatcherQueue = CreateMessageQueue();
			if (l_DispatcherQueue == -1)
				printf("cant create receiving mbx thread\n");
			else
			{
			   dsp_EntityToQueue[eMfaHighDispacherThread] = l_DispatcherQueue;
			}
			    
			
			
			//EmbSleep(10);

			
			
			printf("creating threads:\n");	    
			 pthread_create (&LoggerDispatchThreadId, NULL, (void*)LoggerDispatchThread, NULL);
			 pthread_create (&MfaLowDispacherId, NULL, (void*)MfaLowDispatchThread, NULL);
			 pthread_create (&ApiSenderId, NULL, (void*)ApiSender, NULL);
			 printf("configure dsp streams:\n");
			 configureDSPStreams();
			 			
			 EmbSleep(10);
*/			 
			 //  Get valid dsp 
			 UINT32 ulSlotId = DSP_CARD_SLOT_ID_0;
			 for (i = 1 ; i< num_OF_DSPS_IN_BOARD + 1 ; i++)
			 {
				 if (loadedDSPInformation[ulSlotId][i] != eDspStatusNonExistant)
					 loadedDSPInformation[ulSlotId][i] = eDSPNotLoaded;
			 }
		printf("end of prepareDspInfrastructure\n");
}

void DiagnosticsMain()
{

	t_TcpConnParams*	ptConParams = &TcpConnection[eSwitchDiagClient];
	TEmaReqHeader*		ptEmaReqHeader;

	SOCKET				clientSocket;
	UINT8				*uc_RcvBuf;
	UINT32				ul_BytesReceived;
	TMessageThreadType	tMessageThreadType;
	
	printf("\n******* SWITCH DIAGNOSTICS STARTING TO RUN ******** \n");

	memset(g_diagResultFileName, 0, sizeof(g_diagResultFileName));
	memset(g_diagUSBResultFileName, 0, sizeof(g_diagUSBResultFileName));


	//IpmcLedsActRdyOff();
	//IpmcLedErrorOff();
	//IpmcActivityLedBlink();
	LedDiagComplete();
	
	IAmChildProcess = NO;
	// To create message queues with unique Ids, set enviroment variable for my application.
	//system("mkdir SwitchDiag");
	//API_setEnv("APPLICATION", "./SwitchDiag");
	wereDspsLoaded = 0;
	
	InitDspStatus();
	InitCardHWVersion();
	InitRtmIsdnDspStatus();
//From MediaCard
//BARAK - change it
	num_OF_DSPS_IN_BOARD = MAX_DSPUNIT_ON_CARD_NUM;

	PQUnitSlot = num_OF_DSPS_IN_BOARD + 1; //PQ now follows the DSP
	RTMUnitSlot	= num_OF_DSPS_IN_BOARD + 2; // RTM follows the PQ
	totalUnitsOnBoard = num_OF_DSPS_IN_BOARD + 2; //total units we have.

	if (initAllDspRelevantStructures() ==0)
	{
		printf("Couldn't allocate system structures!\n");
		exit(0);
	}
	//If we want to exit in a nice way,we need to call freeAllDspRelevantStructures()
	// but we never will,so...

	/*----------*/

	if (wereDspsLoaded == 0)
	{
		printf("Loading dsps:\n");

		/*for (i = 1 ; i< num_OF_DSPS_IN_BOARD + 1 ; i++)
			loadedDSPInformation[i] = eDSPNotLoaded;
		*/
		prepareDspInfrastructure();
		wereDspsLoaded = 1;
	}
#if 0
	/*----------------------------*/
	dgnsTestResult diagTestRes;
	FpgaPIKeepAliveTest(&diagTestRes);
	if (diagTestRes.testResult == eStatFail)
		errReportError(eFPGA_PI_KEEP_ALIVE_DIAG,diagTestRes.errString);
	FpgaMSKeepAliveTest(&diagTestRes);
	if (diagTestRes.testResult == eStatFail)
	{
		/*	Pi keep alive failed - i fail all the pi's	*/
		fpgaWorkingMap[FPGA_MEDIA_SWITCH] = 0;
		errReportError(eFPGA_MS_KEEP_ALIVE_DIAG,diagTestRes.errString);
	}

	FpgaMSLinkToPIClockTest(&diagTestRes);
	/*	In this case, the function sets the failed bits to fpgaWorkingMap by itself	*/
	if (diagTestRes.testResult == eStatFail)
		errReportError(eFPGA_MS_LINK_TO_PI_CLOCK_DIAG,diagTestRes.errString);


	char *piStatusReg;
	piStatusReg = (char *)(pQMainPalBaseAddr + PI_STATUS_BIT_AT_FPGA);

	/*	address already initialized	*/
	if (!pQMainPalBaseAddr) //dsp load failed, exit the program.
		return;


	currentBoardType = GetBoardDescription(SAVE_EPROM_INFORMATION);
	//(currentBoardType == eMfa_26) is the regular deal (16 dsps),
	//(currentBoardType == eMfa_Barak9) means that several dsp's are missing

#endif

		printf("Number of DSPs on this system = %d\n",num_OF_DSPS_IN_BOARD);
//End From MediaCard


	InitSchedStruct_Other();

//	ParseMfaDbgCfg();
	//InitDefaultMfaDbg();

	//This thread will check SW2 (button on back of the switch) through IPMC. If it is pressed for long
	// - it will start tests
	SwitchCheckSW2ButtonThread = CreateThread(isToStartTestsThread,2);

// already created before    
//	TimerManagerThreadId = CreateThreadOther(TimerMngrThread, 2);
//	printf("TimerMngrThread: TimerManagerThreadId %d",TimerManagerThreadId);
	
	
	//simTest();
	
	//start the print task
/*
	l_PrintQueue = CreateMessageQueue();
	if (l_PrintQueue == -1)  
//		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"McmsComMain: create Print queue failed");
		printf("McmsComMain: create Print queue failed\n");
	else
	{
//	    MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"McmsComMain: Diag Print Queue id %d",l_DiagPrintQueue);
	    printf("McmsComMain: Diag Print Queue id %d\n",l_PrintQueue);
	
//  already created before           
//	    PrintThreadId = CreateThreadOther(PrintThread, 2);
//		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"McmsComMain: DiagPrintThreadId %d",DiagPrintThreadId);
		printf("McmsComMain: DiagPrintThreadId %d\n",PrintThreadId);
		
//		ConnectToLoggerTaskThreadId = CreateThread(ConnectToLoggerTask, 0);
//		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ConnectToLoggerTask: DiagConnectToLoggerTaskThreadId %d",ConnectToLoggerTaskThreadId);

	}
*/

#if 0
	//start the tests task
	l_SwitchTestsQueue = CreateMessageQueue();
	if (l_SwitchTestsQueue == -1)  
//		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"McmsComMain: create Print queue failed");
		printf("EnterDiagnosticsMode: create tests queue failed\n");
	else
	{
//	    MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"McmsComMain: Diag Print Queue id %d",l_DiagPrintQueue);
	    printf("EnterDiagnosticsMode: Diag tests Queue id %d\n",l_SwitchTestsQueue);
	
	    SwitchDiagTestsThreadId = CreateThreadOther(SwitchDiagTestsThread, 1);
//		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"McmsComMain: DiagPrintThreadId %d",DiagPrintThreadId);
		printf("EnterDiagnosticsMode: SwitchDiagTestsThreadId = %d\n",SwitchDiagTestsThreadId);

	}
#endif

	diagFlag = YES;
	// Init client params
	MfaBoardPrint(	DIAG_PRINT,
					PRINT_LEVEL_ERROR,
					PRINT_TO_TERMINAL,
					"(DiagnosticsMain): Thread Running....\n");
	
	// connect to SwitchDiag TCP Server...
	ptConParams->e_Id 		= eSwitchDiagClient;
	ptConParams->us_Port 	= LISTEN_SWITCH_DIAG_PORT;
	ptConParams->ul_ClientOrServer = eConnTypeClient;
	strcpy(&ptConParams->IpV4Addr.auc_IpV4Address[0]   , "127.0.0.1"	);
	
	// Connect as client to SwitchDiagServer
	ConnectToTcpServer(ptConParams);


	//send diagnostics mode to ipmc, in the format of post results - same as in the MFA
	//SetPostKeepAliveResultFunc(0x80000000 , 0); // bit #31 = diagnostics bit
	MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
				  "DiagnosticsMain: Sent diagnostics mode to the shelf");

//	InitErrorBufferDB();
//	InitTestsDatabase();
//	InitReportsDB();
//	diagFlag = YES;
	while(1)
	{

		// Handle diagnostics requests (from SwithcConnection ....)
		// Receive message, get opcode and build a state machine to handle the requests)
		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
					  "DiagnosticsMain: main loop");

		EmbSleep(100);

	if (ptConParams->ul_ConnectionStatus == CONNECTED)
     	{       
			clientSocket = ptConParams->s;

			MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   			"DiagnosticsMain: clientSocket = %d \n", clientSocket);

	/*	
			printf("socket blocked with data\n");
     			ul_BytesReceived = TCPRecvData(clientSocket, (VOID **)&uc_RcvBuf, 0);     	
        	    printf("socket %d unblocked with data , bytes received = %d\n",clientSocket,ul_BytesReceived);
       	*/
			
			while (!isMessageFromEmaToDiagReady())
            {
                EmbSleep(100);
            }
            
            if(NULL == (uc_RcvBuf = malloc(MAX_MSG_EMA_SIZE))) continue;
            rcvMsgFromEmaToDiag(&ul_BytesReceived,&(uc_RcvBuf[0]));
            
       		if ((ul_BytesReceived == (UINT32)SOCKET_OPERATION_FAILED) || (ul_BytesReceived == (UINT32)TPCKT_OPERATION_FAILED)) //< TODO: condition will never be met, to remove?
       		{
				MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
       			"DiagnosticsMain: problem receiving data \n");

                TraceDiagSwitchEmaCom(eHandle, ul_BytesReceived, -1, -1, "FAILED to receive data");
				free(uc_RcvBuf);
			}
       		else
       		{
       			
   				tMessageThreadType.ulData = (UINT32)uc_RcvBuf;  
   				tMessageThreadType.ulSize = ul_BytesReceived;
       			
//	   			printf("parsing ema request\n");

				MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
			       			"DiagnosticsMain: Received data from switch \n");
//				printf("DiagnosticsMain: Received data from switch \n");

				ptEmaReqHeader = (TEmaReqHeader*)tMessageThreadType.ulData;
				if(ul_BytesReceived < sizeof(TEmaReqHeader))
				{
					MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"DiagnosticsMain: problem receiving data ul_BytesReceived < sizeof(TEmaReqHeader) \n");
					free(ptEmaReqHeader);
					continue;
				}
				
				UINT32 ulOpcode = ptEmaReqHeader->ulOpcode;
				UINT32 ulMsgId  = ptEmaReqHeader->ulMsgID;
				UINT32 ulSlotId = ptEmaReqHeader->ulSlotID;

				MfaBoardPrint(	DIAG_PRINT,
								PRINT_LEVEL_ERROR,
								PRINT_TO_TERMINAL,
								"(DiagnosticsMain): EmaReqHeader:: opcode:%Xh, msgID:%d, slotID:%d.\n",
								ulOpcode,
								ulMsgId,
								ulSlotId);	
//				printf("(DiagnosticsMain): EmaReqHeader:: opcode:%Xh, msgID:%d, slotID:%d.\n",
//					   	ulOpcode,
//					   	ulMsgId,
//					   	ulSlotId);	
//				printf("opcode %d requested \n",ulOpcode);

                TraceDiagSwitchEmaCom(eHandle, ulMsgId, ulOpcode, ulSlotId, "None");

				if(ulSlotId < CNTL_SLOT_ID || ulSlotId >= MAX_SLOT_NUM)
				{
					MfaBoardPrint(	DIAG_PRINT,
								PRINT_LEVEL_ERROR,
								PRINT_TO_TERMINAL,
								"(DiagnosticsMain): SlotId is out of range. skip handle!");
					if(ptEmaReqHeader != NULL)
						free(ptEmaReqHeader);
					continue;
				}
				
				switch(ulOpcode)
				{

					case EMA_ENTER_DIAG_MODE_REQ:
						// Send indication to the EMA
						// Bracha - Add a condition if all the above passed succesfully
						BuildEnterDiagModeIndMsg(ulMsgId, ulSlotId);
						break;
					case PRIVATE_OPCODE:
						
						MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"DiagnosticsMain: we are in PRIVATE_OPCODE \n");
						if(tMfaDbgInfo.DiagTerminalFileHandle)
				        {
				        	fclose(tMfaDbgInfo.DiagTerminalFileHandle);
				        	tMfaDbgInfo.DiagTerminalFileHandle = NULL;
				        }
					     tMfaDbgInfo.DiagTerminalFileHandle = fopen((char *)&(ptEmaReqHeader->ulMsgID),"w");
					     if (NULL == tMfaDbgInfo.DiagTerminalFileHandle)
						{
							MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TerminalCommand Error tMfaDbgInfo.DiagTerminalFileHandle == NULL");
							break;
						}
					     
					     fprintf(tMfaDbgInfo.DiagTerminalFileHandle,"I am ok");
					     break;
						
					case EMA_GET_TEST_LIST_REQ:

						// Send indication response
//						printf("test list requested\n");
						BuildTestListIndMsg(ulMsgId, ulSlotId);
//						printf("after BuildTestListIndMsg\n");
						break;

					case EMA_START_TEST_REQ:

						//IpmcLedsActRdyGreenBlink();
						// Send indication response
						BuildStartTestIndMsg(ulMsgId, ulSlotId);
						// Start test

						ParseStartTestReq((UINT32*)tMessageThreadType.ulData, ulSlotId);
//						printf("after ParseStartTestReq\n");
						break;

					case EMA_GET_TEST_STATUS_REQ:

						// Send indication response
						BuildGetTestStatusIndMsg(ulMsgId, ulSlotId);
//						printf("after BuildGetTestStatusIndMsg\n");
						break;

					case EMA_STOP_TEST_REQ:

						// Send indication response
						BuildStopTestIndMsg(ulMsgId, ulSlotId);
//						printf("after BuildStopTestIndMsg\n");
						break;

					case EMA_GET_UNITS_STATE_REQ:
						printf("unit state requested\n");
						// Send indication response
						BuildUnitListIndMsg(ulMsgId, ulSlotId);
//						printf("after BuildUnitListIndMsg\n");
						break;
						
					case EMA_GET_ERROR_LIST_REQ:

						// Send indication response
						BuildErrorListIndMsg(ulMsgId, ulSlotId);
//						printf("after BuildErrorListIndMsg\n");
						break;

					default:

						MfaBoardPrint(	DIAG_PRINT,
										PRINT_LEVEL_ERROR,
										PRINT_TO_TERMINAL,
										"(GetTestReqMsg): Unknown Opcode !!! (%Xh)",ulOpcode);
//						printf(	"(GetTestReqMsg): Unknown Opcode !!! (%Xh)",ulOpcode);
										
						break;
				}
				if(ptEmaReqHeader != NULL)
					free(ptEmaReqHeader);
	   		}//if (ul_BytesReceived == SOCKET_OPERTATION_FAILED) 
		
		
	}   
     	else
     	{
			MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
						  "DiagnosticsMain: main loop. Wait for connection");
			printf("Wait for connection\n");
	 		EmbSleep(30);
     	}
   	}
}

#if 0
void ActivateSimulationTests(UINT32 userChoise)
{
	FILE *fp_error_log;
//	char* pTmpTestFile;
//	char strFileNum[2];
//	APIUBOOL returnValue;
	TShelfSlots shelfSlots;
	int ind,rc;
	char *slotsStr[6]={"",
	                     "169.254.128.67",
	                     "169.254.128.68",
	                     "",				  // cpu1
	                     "",				  //cpu2
	                     "169.254.128.16"};  
	/*if (!inLoopMode)
	{
		InitReportsDB();
	}*/

	// Open to append
 	fp_error_log = fopen(DIAG_ERROR_LOG_FILE, "a");

	switch (userChoise)
	{
		case(eMemoryTest):

			currentTestId = eMemoryTest;
		    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
		    "\n%s\n", testTypeStr[userChoise - 1]);
			fprintf(fp_error_log, "\n%s\r\n", testTypeStr[userChoise - 1]);

			switchMemoryTest.testInProgress = YES;
			// Update database
			TestsDatabase.tTestInfo[eMemoryTest].testInProgress = YES;
			// Update duration timer
			gettimeofday(&tTimer_vals,NULL) ;
			startTestValue = (UINT32)tTimer_vals.tv_sec ;
		    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
		    "Time of starting test (in sec) = %d\n", startTestValue);
			// Bracha - Start test
			Memtester(MEMORY_TEST_SIZE_OF_MEMORY, MEMORY_TEST_NUM_OF_LOOPS);
			// End of test
		    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
		    "SwitchSim: End of Test!\n");
			// Update error file
			WriteTestReportFile(DIAG_ERROR_LOG_FILE);
			EmaLoopTests();
			break;

		case(eNetworkTest):

			currentTestId = eNetworkTest;
		    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
		    "\n%s\n", testTypeStr[userChoise - 1]);
			fprintf(fp_error_log, "\n%s\r\n", testTypeStr[userChoise - 1]);

			// Update reports
			switchNetworkTest.testInProgress = YES;
			// Update database
			TestsDatabase.tTestInfo[eNetworkTest].testInProgress = YES;
			// Update duration timer
			gettimeofday(&tTimer_vals,NULL) ;
			startTestValue = (UINT32)tTimer_vals.tv_sec ;
		    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
		    "Time of starting test (in sec) = %d\n", startTestValue);
			// Start test
			
			//rc = ReadShelfSlotId((char *)&shelfSlots,sizeof(shelfSlots));
			if (rc != -1)
			{
				for (ind=0;ind<shelfSlots.numberOfSlots;ind++)
				{
				   if ((shelfSlots.slotId[ind]==1)||(shelfSlots.slotId[ind]==2))
				   {
					
						pingApp("169.254.128.16",slotsStr[shelfSlots.slotId[ind]], 20, 1);
						if (pingTestFailure)
						{
							switchNetworkTestFailure = YES;
							break;
						}
					}
				}
			}
			else switchNetworkTestFailure = YES;

			// End of test
			// Stop the timer - should be implemented
//				if(timerId[unBoardNum])
//				{
//					TimerDeleteJob(timerId[unBoardNum]);
//					timerId[unBoardNum] = 0;
//				}
//			EmbTimers( 0, 0, NULL);
		    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
		    "SwitchSim: End of Test!\n");

			// Update error file
			WriteTestReportFile(DIAG_ERROR_LOG_FILE);

			EmaLoopTests();

			break;

//		case(eLoopTests):
//
//		    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
//		    "\n%s\n", testTypeStr[userChoise - 1]);
//		    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
//		    "\nStarting loop tests\n");
//			printf("\n%s\n", testTypeStr[userChoise - 1]);
//			fprintf(fp_error_log, "\nStarting loop tests\r\n");
//			testNum = 1;
//			inLoopMode = YES;
//			loopCounter = 1;
//			LoopTests();
//			break;
	}
//	fclose(fp_error_log);

}

void WriteTestReportFile(char* file_name)
{
	FILE *fp_error_log;
	UINT32 i, j;


    MfaBoardPrint(TCP_SERVER_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
	"\nPreparing simulation report\n");

	// Open to append
 	fp_error_log = fopen(file_name, "a");

	if(fp_error_log == NULL)
	{
	    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
		"ERROR: Can not open file: %s\n",file_name);	
        return;
	}
	else
	{
		if (inLoopMode)
		{
		    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
			"\nLoop number: %d\r\n", loopCounter);
			fprintf(fp_error_log, "\nLoop number: %d\r\n", loopCounter);
		}

		if (switchMemoryTest.testInProgress)
		{
			if(!memoryTestFailure)
			{
				switchMemoryTest.testSuccessCounter += 1;
				TestsDatabase.tTestInfo[eMemoryTest].testSuccessCounter += 1;
			}

		    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
			"\nTest pass %d times. Test failed %d times.\r\n",switchMemoryTest.testSuccessCounter, (loopCounter - switchMemoryTest.testSuccessCounter));
			fprintf(fp_error_log, "\nTest pass %d times. Test failed %d times.\r\n",switchMemoryTest.testSuccessCounter, (EmaLoopCounter - switchMemoryTest.testSuccessCounter));
			// Update error buffer. will be reported to the EMA
			errorBuffer.error[errorBuffer.putNextError].testId = currentTestId;
			sprintf(errorBuffer.error[errorBuffer.putNextError].errorString,
			"\nTest pass %d times. Test failed %d times.\r\n",TestsDatabase.tTestInfo[eMemoryTest].testSuccessCounter, (EmaLoopCounter - TestsDatabase.tTestInfo[eMemoryTest].testSuccessCounter)); 
			UpdateErrorBuffer();

			switchMemoryTest.testInProgress = NO;
			TestsDatabase.tTestInfo[eMemoryTest].testInProgress = NO;
			memoryTestFailure = NO;
		}

		if (switchNetworkTest.testInProgress)
		{
			if(!switchNetworkTestFailure)
			{
				switchNetworkTest.testSuccessCounter += 1;
				TestsDatabase.tTestInfo[eNetworkTest].testSuccessCounter += 1;
			}
		    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
			"\nTest pass %d times. Test failed %d times.\r\n",switchNetworkTest.testSuccessCounter, (loopCounter - switchNetworkTest.testSuccessCounter));
			fprintf(fp_error_log, "\nTest pass %d times. Test failed %d times.\r\n",switchNetworkTest.testSuccessCounter, (EmaLoopCounter - switchNetworkTest.testSuccessCounter));
			// Update error buffer. will be reported to the EMA
			errorBuffer.error[errorBuffer.putNextError].testId = currentTestId;
			sprintf(errorBuffer.error[errorBuffer.putNextError].errorString,
			"\nTest pass %d times. Test failed %d times.\r\n",TestsDatabase.tTestInfo[eNetworkTest].testSuccessCounter, (EmaLoopCounter - TestsDatabase.tTestInfo[eNetworkTest].testSuccessCounter)); 
			UpdateErrorBuffer();

			switchNetworkTest.testInProgress = NO;
			TestsDatabase.tTestInfo[eNetworkTest].testInProgress = NO;
			switchNetworkTestFailure = NO;
		}
	}

	fclose(fp_error_log);

	return;
}




void InitReportsDB(UINT32 unBoardNum)
{
	currentTestId = 0;
	loopCounter = 1;

	switchMemoryTest.testInProgress = NO;
	switchNetworkTest.testInProgress = NO;

	switchMemoryTest.testSuccessCounter = 0;
	switchNetworkTest.testSuccessCounter = 0;
}


void InitErrorBufferDB()
{
	UINT32 i;

	// Init error buffer database
	for(i = 0; i < MAX_NUM_OF_ERRORS_IN_BUFFER; i++)
	{
		// Max number of characters
		errorBuffer.error[i].errorString = malloc(100);
		errorBuffer.error[i].testId = 0;
	}
	errorBuffer.putNextError = 0;
	errorBuffer.getNextError = 0;
	errorBuffer.bufferOverlap = NO;
}

APIUBOOL IsCardReadyForTest(UINT32 unBoardNum)
{
/*
	if (boards[unBoardNum].cardIsConfigured)
	{
		if (testArtAudio[unBoardNum].testInProgress
		||	testArtAudioVideo[unBoardNum].testInProgress
		||	testArtAudioMulticast[unBoardNum].testInProgress
		||	testVideo[unBoardNum].testInProgress
		||	testVideoMulticast[unBoardNum].testInProgress
		||	testDspMemory[unBoardNum].testInProgress
		||	testDeveloper[unBoardNum].testInProgress)
		{
			printf("unBoardNum = %d, test is in progress\n", unBoardNum);
			return(EMB_FALSE);
		}
		else
		{
			return(EMB_TRUE);
		}
	}
	printf("unBoardNum = %d boards[unBoardNum].cardIsConfigured = %d\n", unBoardNum, boards[unBoardNum].cardIsConfigured);
	return(EMB_FALSE);
*/
    return(EMB_FALSE);
}

APIUBOOL IsEmaReqTestInProgress()
{
	UINT32	i;
	
	for(i = 1; i <= MAX_NUM_OF_TESTS ; i++)
	{

		if(TestsDatabase.tTestInfo[i].testInProgress)
		{
			printf("Test no %d is in progress\n", i);
			return(EMB_TRUE);
		}
	}
	return(EMB_FALSE);
}

void EmaLoopTests()
{
	UINT32		nextTestNum = 0;
	UINT32		i;
	char 		strFileNum[2];
	char*		pTempFileName;
	UINT32		startLoopTimer;

	// Update loop duration timer
	gettimeofday(&tTimer_vals,NULL) ;
	startLoopTimer = (UINT32)tTimer_vals.tv_sec ;
    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,
    "Time of starting test (in sec) = %d\n", startTestValue);

	if ((EmaLoopCounter <= TestsDatabase.unNumOfReqLoops) || ( TestsDatabase.unNumOfReqLoops == -1))
	{

		MfaBoardPrint(	TCP_SERVER_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"(EmaLoopTests): TestsDatabase.unNumOfReqLoops = %d\n", TestsDatabase.unNumOfReqLoops);	
		MfaBoardPrint(	TCP_SERVER_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"(EmaLoopTests): loopNumber = %d\n", EmaLoopCounter);	

		// Find the next test to perform
		for(i = 1; i <= MAX_NUM_OF_TESTS ; i++)
		{
			MfaBoardPrint(	TCP_SERVER_PRINT,
							PRINT_LEVEL_ERROR,
							PRINT_TO_TERMINAL,
							"(EmaLoopTests): TestsDatabase.tTestInfo[%d].unTestID = %d",
							 i,
							 TestsDatabase.tTestInfo[i].unTestID);	


			if (TestsDatabase.tTestInfo[i].performTestFlag == YES)
			{
				MfaBoardPrint(	TCP_SERVER_PRINT,
								PRINT_LEVEL_ERROR,
								PRINT_TO_TERMINAL,
								"(EmaLoopTests): EmaLoopCounter = %d\n", EmaLoopCounter);	
				MfaBoardPrint(	TCP_SERVER_PRINT,
								PRINT_LEVEL_ERROR,
								PRINT_TO_TERMINAL,
								"(EmaLoopTests): TestsDatabase.tTestInfo[%d].unLoopNum = %d\n", i, TestsDatabase.tTestInfo[i].unLoopNum);	
				// Check if already done in this loop
				if (TestsDatabase.tTestInfo[i].unLoopNum ==	(EmaLoopCounter - 1))
				{

					MfaBoardPrint(	TCP_SERVER_PRINT,
									PRINT_LEVEL_ERROR,
									PRINT_TO_TERMINAL,
									"(EmaLoopTests): performing test number = %d\n", i);	
					nextTestNum = i;
					break;
				}
			}
		}
		// No more tests in the current loop
		if (i == (MAX_NUM_OF_TESTS + 1))
		{
			TestsDatabase.unPerformedLoopNum = EmaLoopCounter;
			EmaLoopCounter++;
			// Search the next loop
			if ((EmaLoopCounter <= TestsDatabase.unNumOfReqLoops) || ( TestsDatabase.unNumOfReqLoops == -1))
			{
				for(i = 1; i <= MAX_NUM_OF_TESTS ; i++)
				{
					if (TestsDatabase.tTestInfo[i].performTestFlag == YES)
					{
						MfaBoardPrint(	TCP_SERVER_PRINT,
										PRINT_LEVEL_ERROR,
										PRINT_TO_TERMINAL,
										"(EmaLoopTests): EmaLoopCounter = %d\n", EmaLoopCounter);	
						MfaBoardPrint(	TCP_SERVER_PRINT,
										PRINT_LEVEL_ERROR,
										PRINT_TO_TERMINAL,
										"(EmaLoopTests): TestsDatabase.tTestInfo[%d].unLoopNum = %d\n", i, TestsDatabase.tTestInfo[i].unLoopNum);	
						// Check if already done in this loop
						if (TestsDatabase.tTestInfo[i].unLoopNum ==	(EmaLoopCounter - 1))
						{

							MfaBoardPrint(	TCP_SERVER_PRINT,
											PRINT_LEVEL_ERROR,
											PRINT_TO_TERMINAL,
											"(EmaLoopTests): performing test number = %d\n", i);	
							// Bracha - Add a condition to choose a test according to unit

							nextTestNum = i;
							break;
						}
					}
				}
			}
			else
			{
				MfaBoardPrint(	TCP_SERVER_PRINT,
								PRINT_LEVEL_ERROR,
								PRINT_TO_TERMINAL,
								"(EmaLoopTests): Finished loop tests, Performed %d loops\n", (EmaLoopCounter - 1));	

				return;
			}

		}


		// Activate test
		if(!IsEmaReqTestInProgress())
		{

			TestsDatabase.tTestInfo[i].unLoopNum++;
			ActivateSimulationTests(nextTestNum);
			nextTestNum = 0;
		}
	} //if (EmaLoopCounter <= TestsDatabase.unNumOfReqLoops)
	else
	{
		// Finish session. Calculate the duration of the entire session
		gettimeofday(&tTimer_vals,NULL) ;
		TestsDatabase.unDuration = (UINT32)tTimer_vals.tv_sec - startLoopTimer;

		MfaBoardPrint(	TCP_SERVER_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"(EmaLoopTests): Finished loop tests, Performed %d number of loops\n", (EmaLoopCounter - 1));	

		MfaBoardPrint(	TCP_SERVER_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"(EmaLoopTests): Duration of loop session: %d\n", TestsDatabase.unDuration);	
	}
}
#endif

void UpdateErrorBuffer()
{
	errorBuffer.putNextError++;
	// Overlap - the error buffer is cyclic
	if(errorBuffer.putNextError == MAX_NUM_OF_ERRORS_IN_BUFFER)
	{
		errorBuffer.putNextError = 0;
		errorBuffer.bufferOverlap = YES;
	}
}

#if 0
void SwitchDiagTestsThread()
{
	UINT32 l_RetVal;
	TMessageThreadType tMessageThreadType;
	COMMON_HEADER_S *ptGeneralMcmsCommonHeader;
    
	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
					  "DiagnosticsMain: SwitchDiagTestsThread created");

    EnrollInThreadList(eSwitchDiagTestsThread);
    
	while(1)
	{
		l_RetVal = ReceiveMessage(l_SwitchTestsQueue , &tMessageThreadType, 0);
		if (l_RetVal != -1)
		{
			MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
							  "DiagnosticsMain: Got a message in l_SwitchTestsQueue.");
			ptGeneralMcmsCommonHeader = (COMMON_HEADER_S *)(tMessageThreadType.ulData);
			MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
							  "Opcode = %d", GET_COMMON_HEADER_OPCODE(ptGeneralMcmsCommonHeader));
//			if(GET_COMMON_HEADER_OPCODE(ptGeneralMcmsCommonHeader) == INTERNAL_TEST_REQUEST_OPCODE)
//			{
				MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
								  "DiagnosticsMain: Activating loop tests.");
				EmaLoopTests();
//			}
		}
		else
		{
			EmbSleep(1000);
		}
	}
}
#endif

/*	This thread is checking with IPMC, whether the sw2 button was pressed for a long time (~5sec)
 * and if yes - starts all tests in system	*/
void	isToStartTestsThread()
{
	int ipmcResult = 0;
	int i,ulSlotId,unitId;
	pthread_t threadTesterManager;

    EnrollInThreadList(eIsToStartTestsThread);
    
	//while (1)
	//{
		//ipmcResult = IpmcWasSW2PressedForLongTime();
		if (usb_flag == 1)
		{
			// Get valid SlotId and UnitId
			for(ulSlotId = 0; (size_t)ulSlotId < sizeof(diagCurrentSession)/sizeof(diagCurrentSession[0]); ulSlotId++)
			{
				if(!(ulSlotId == CNTL_SLOT_ID || ulSlotId == DSP_CARD_SLOT_ID_0 
					|| ulSlotId == DSP_CARD_SLOT_ID_1 || ulSlotId == DSP_CARD_SLOT_ID_2 || ulSlotId == ISDN_CARD_SLOT_ID))
				{
					continue;
				}
	
				if (diagCurrentSession[ulSlotId].hasStarted == 0)
				{
					//start all tests
					printf("enter diagnostic via USB. Starting Tests on Slot: %d", ulSlotId);
				
					
					/********* Init session parameters. Taken from ParseStartTestReq() in DiagnosticsApi.c	*/
					dgnsClearTests(ulSlotId);
					wasTestAskedToStop[ulSlotId] = 0;
					diagCurrentSession[ulSlotId].hasStarted = 1;
					diagCurrentSession[ulSlotId].numOfLoops = 1;
					diagCurrentSession[ulSlotId].isQuickSession = 0;
					diagCurrentSession[ulSlotId].stopOnFail = 0;
					
					
					/*	Go over all test DB and add tests to diagnostics session	*/
					i = 0;
					while (currentSystemTests[i].TestId != 0xFFFF)
					{					
						//check whether the test is applicable to current system chassis
						switch (chassisType)
						{
							case eChassisType_Ninja:
								{
									if(0 == (currentSystemTests[i].SystemType & E_BIT_OP_CHASSIS_TYPE_NINJA) )
									{
										printf("Skipping %s...\n", currentSystemTests[i].TestName);
										i++;
										continue;
									}

									if((CNTL_SLOT_ID == ulSlotId) && (eUndefined != currentSystemTests[i].TestOn))
									{
										printf("Skipping %s...\n", currentSystemTests[i].TestName);
										i++;
										continue;
									}

									if((DSP_CARD_SLOT_ID_0 <= ulSlotId) && (DSP_CARD_SLOT_ID_2 >= ulSlotId) && (eDsp != currentSystemTests[i].TestOn))
									{
										printf("Skipping %s...\n", currentSystemTests[i].TestName);
										i++;
										continue;
									}

									if((ISDN_CARD_SLOT_ID == ulSlotId) && (eRtm != currentSystemTests[i].TestOn))
									{
										printf("Skipping %s...\n", currentSystemTests[i].TestName);
										i++;
										continue;
									}
                                    
									break;

								}

							default:
								// Note: some enumeration value are not handled in switch. Add default to suppress warning.
								break;
						}

						if(ulSlotId >= DSP_CARD_SLOT_ID_0 && ulSlotId <= DSP_CARD_SLOT_ID_2)
						{
							for(unitId = 1; unitId <= MAX_DSPUNIT_ON_CARD_NUM; unitId++)
							{
								if(isValidDsp(ulSlotId, unitId))
									dgnsTestAddTest(currentSystemTests[i].TestId, ulSlotId, unitId);
							}
						}
						else if(ulSlotId == ISDN_CARD_SLOT_ID)
						{
							for(unitId = 1; unitId <= MAX_DSPUNIT_ON_ISDN_CARD_NUM; unitId++)
							{
								if(isValidRtmIsdnDSP(ulSlotId, unitId))
									dgnsTestAddTest(currentSystemTests[i].TestId, ulSlotId, unitId);
							}
						}
						else dgnsTestAddTest(currentSystemTests[i].TestId, ulSlotId, 0);
						i++;
					}

					UINT32 * pulSlotId = (UINT32 *)malloc(sizeof(UINT32));
					if(NULL != pulSlotId)
					{
						*pulSlotId = ulSlotId;
						pthread_create(&threadTesterManager,NULL,StartTestHandling,pulSlotId);
						pthread_detach(threadTesterManager);
					}
					/*--------------------Started new test------------------------------------------*/
				}
				else
				{
					printf("Tests in progress. Can't start new test\n");
				}
			}
		}
		
		EmbSleep(2000);
		
	//}
	
	
}

void askDspsForTestStatus()
{
UINT32 	i;

	/*I request status from all DSPS that have been asked to perform a test.	*/
		/*	On several cases,when tests like connectivity are running, we dont want to poll dsps
		 * for test, because their answer would touch our test structures we are preparing.
		 * And we don't need the test statuses anyway, because they don't change	*/
	/*
	if (currentTestType != e_typeDSPtestRunSerialWay)
		for (i = 1; i < num_OF_DSPS_IN_BOARD + 1 ; i++)
			if (isNotEmptyList(dspTestListCollection[uSlotId][i]))
				SendDSPMessage(1,i,eDSP_GET_TEST_STATUS,NULL,0);
	*/

}

void simTest()
{
	UINT32 ulSlotID = 4;
	dgnsClearTests(ulSlotID);

	dgnsTestAddTest(ePQ_MEMORY_DATA_BUS_DIAG, ulSlotID, 0);
	dgnsTestAddTest(ePQ_MEMORY_ADDRESS_BUS_DIAG, ulSlotID, 0);
	dgnsTestAddTest(ePQ_MEMORY_ENERGY_DIAG, ulSlotID, 0);
	dgnsTestAddTest(ePQ_MEMORY_INTEGRITY_DIAG, ulSlotID, 0);
//	dgnsTestAddTest(eFLASH_DATA_BUS_DIAG, ulSlotID, 0);
//	dgnsTestAddTest(eFLASH_ADDRESS_INTEGRITY_DIAG, ulSlotID, 0);
	dgnsTestAddTest(eIPMC_UART_CHANNEL_DIAG, ulSlotID, 0);
	dgnsTestAddTest(eFPGA_SHOVAL_KEEP_ALIVE_TEST, ulSlotID, 0);
	dgnsTestAddTest(eFPGA_SHOVAL_LINK_TEST, ulSlotID, 0);
	dgnsTestAddTest(eFPGA_SHOVAL_CLOCKS_TEST, ulSlotID, 0);
	dgnsTestAddTest(eFPGA_SHOVAL_CPLD_TEST, ulSlotID, 0);
	dgnsTestAddTest(ePQ_CORE_CLOCK_TEST, ulSlotID, 0);
	
	wasTestAskedToStop[ulSlotID] = 0; 
	
	diagCurrentSession[ulSlotID].isQuickSession = 0;  //QUICK SESSION NOT DEFINED BY PROTOCOL!!!
	diagCurrentSession[ulSlotID].numOfLoops = 99;
	diagCurrentSession[ulSlotID].stopOnFail = 0;
	
	currentLogLoopNumber = 0;
	diagCurrentSession[ulSlotID].hasStarted = 1;

	StartTestHandling(NULL);
	diagCurrentSession[ulSlotID].hasStarted = 0;
}
