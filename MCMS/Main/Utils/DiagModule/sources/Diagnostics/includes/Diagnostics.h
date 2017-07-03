#ifndef _DIAGNOSTICS_H
#define _DIAGNOSTICS_H

#include "McmsApi.h"
#include "EmaShared.h"

#if 0
#define MY_APP_PATH	"/app"
#else
#define MY_APP_PATH	"/output"
#endif


typedef enum
{
	
				ePQ_MEMORY_DATA_BUS_DIAG		=	100,
				ePQ_MEMORY_ADDRESS_BUS_DIAG		=	101,
				ePQ_MEMORY_ENERGY_DIAG		=	102,
				ePQ_MEMORY_INTEGRITY_DIAG		=	103,

				eIPMC_UART_CHANNEL_DIAG		=	104,
				eIPMC_I2C_EEPROM_CHKSUM_DIAG		=	105,
				eIPMC_I2C_AD_DIAG		=	106,
				eIPMC_I2C_IPMA_DIAG		=	107,
				eIPMC_I2C_IPMB_DIAG		=	108,
				eIPMC_I2C_SENSORS_DIAG		=	109,
				eIPMC_I2C_TEMPERATURE_DIAG		=	110,

				eFLASH_DATA_BUS_DIAG		=	111,
				eFLASH_ADDRESS_INTEGRITY_DIAG		=	112,

				eFPGA_MS_KEEP_ALIVE_DIAG		=	200,
				eFPGA_SHOVAL_KEEP_ALIVE_TEST		=	201,
				eFPGA_MS_PCI_CLOCK_DIAG		=	202,
				eFPGA_SHOVAL_MEM_TEST		=	203,
				eFPGA_MS_LINK_TO_PI_CLOCK_DIAG		=	204,
				eFPGA_SHOVAL_LINK_TEST		=	205,
				eFPGA_MS_FABRIC_CLOCK_DIAG		=	206,
				eFPGA_SHOVAL_CLOCKS_TEST		=	207,
				eFPGA_SHOVAL_CPLD_TEST		=	208,
				eFPGA_SWITCH_MEMORY_DIAG		=	209,
				eFPGA_PI_KEEP_ALIVE_DIAG		=	210,
				eFPGA_PI_PCI0_CLOCK_DIAG		=	211,
				eFPGA_PI_PCI1_CLOCK_DIAG		=	212,
				eFPGA_MS_CORE_CLOCK_DIAG		=	213,
				eFPGA_PI_LINK_TO_MS_CLOCK_DIAG		=	214,
				eFPGA_RTM_PCI_CLOCK_DIAG		=	215,
				eFPGA_BOTTOM_LINK_CLOCK_DIAG		=	216,
				eFPGA_TOP_LINK_CLOCK_DIAG		=	217,

				eLAN_TEST_SIGNALING		=	300,
				eLAN_TEST_MFA1		=	301,
				eLAN_TEST_MFA1_EXT		=	302,
				eLAN_TEST_MFA2		=	303,
				eLAN_TEST_MFA2_EXT		=	304,
				eLAN_TEST_MFA3		=	305,
				eLAN_TEST_MFA3_EXT		=	306,
				eLAN_TEST_MFA4		=	307,
				eLAN_TEST_MFA4_EXT		=	308,
				eLAN_TEST_CNTL		=	309,
				eLAN_TEST_CNTL_EXT		=	310,
				eLAN0_INTERFACE_DIAG		=	311,
				eLAN1_INTERFACE_DIAG		=	312,
				eLAN2_INTERFACE_DIAG		=	313,
				eLAN3_INTERFACE_DIAG		=	314,
				eLAN_TRAFFIC_LOAD_DIAG		=	315,

				ePQ_LOCAL_BUS_DIAG		=	320,
				ePQ_CORE_CLOCK_DIAG		=	321,
				ePQ_MD5FILESTEST		=	322,
				ePQ_IPMC_TEST_SENSORS		=	323,
				ePQ_CORE_CLOCK_TEST		=	324,
				ePQ_DDR_CLOCK_DIAG		=	325,
				ePQ_LOCAL_BUS_CLOCK_DIAG		=	326,
				ePQ_CPLD_DIAG		=	327,

				eSHOVAL_MCAST_TEST		=	330,
				eSHOVAL_UNICAST_LOAD		=	331,
				eSHOVAL_MULTICAST_LOAD		=	332,

				eDSP_DOWNLOAD_DIAG		=	401,
				eDSP_CONNECTIVITY_DIAG		=	402,
				eDSP_BUS_LOAD_DIAG		=	403,
				eDSP_DATA_BUS_DIAG		=	404,
				eDSP_ADDRESS_BUS_DIAG		=	405,
				eDSP_ENERGY_DIAG		=	406,
				eDSP_INTEGRITY_DIAG		=	407,
				eDSP_CORE_CLOCK_DIAG		=	408,
				eDSP_SDRAM_CLOCK_DIAG		=	409,

				eBOARD_BUS_LOAD_DIAG		=	500,
				eBOARD_CONNECTIVITY_DIAG		=	501,
				eBOARD_TO_BOARD_BUS_LOAD_DIAG		=	502,
				eBOARD_HOMOLOGATION_TEST		=	503,

				eCNTL_CODEC_STRESS_TEST   	        = 599,
				eCNTL_DDR_MEMORY_DATA_BUS		= 600,
				eCNTL_DDR_MEMORY_ADDRESS_BUS	= 601,
				eCNTL_DDR_MEMORY_INTEGRITY		= 602,
				eCNTL_DDR_MEMORY_ENERGY			= 603,

				eCNTL_CF_CREATE_DELETE_READ_WRITE 	= 604,
				eCNTL_CF_FILE_SYSTEM_CHECK	        = 605,
				eCNTL_CF_MD5				        = 606,

				eCNTL_HARD_DISK_SMART_CHECK			= 607,
				eCNTL_HARD_DISK_READ_WRITE	        = 608,
				
				eCNTL_USB_READ_WRITE	        = 609,

				eCNTL_IPMC_CONNECTION				= 610,

				eRTM_DSP_DOWNLOAD_DIAG		=	700,
				eRTM_DSP_DATA_BUS_DIAG		=	701,
				eRTM_DSP_ADDRESS_BUS_DIAG		=	702,
				eRTM_DSP_ENERGY_DIAG		=	703,
				eRTM_DSP_INTEGRITY_DIAG		=	704,
				eRTM_DSP_CPLD_DIAG		=	705,
				eRTM_DSP_CLOCKS_DIAG		=	706,
//				eRTM_DSP_TDM_FALC1_DIAG		=	707,
				eRTM_DSP_T1_INF_DIAG		=	708,
				eRTM_DSP_E1_INF_DIAG		=	709,
				eRTM_DSP_FALC_DIAG		=	710,
				eRTM_DSP_T1_FALC1_DIAG		=	711,
				eRTM_DSP_T1_FALC2_DIAG		=	712,
				eRTM_DSP_T1_FALC3_DIAG		=	713,
				eRTM_DSP_E1_FALC1_DIAG		=	714,
				eRTM_DSP_E1_FALC2_DIAG		=	715,
				eRTM_DSP_E1_FALC3_DIAG		=	716,
				eRTM_DSP_TDM_FALC1_DIAG		=	717,
				eRTM_DSP_TDM_FALC2_DIAG		=	718,
				eRTM_DSP_TDM_FALC3_DIAG		=	719,
				eRTM_DSP_LAST_TEST		=	720,

				eMASSDSP_CORE_CLOCK_DIAG		=	800,
				eMASSDSP_DATA_BUS_DIAG		=	802,
				eMASSDSP_ADDRESS_BUS_DIAG		=	803,
				eMASSDSP_ENERGY_DIAG		=	804,
				eMASSDSP_INTEGRITY_DIAG		=	805,
				eMASSDSP_DOWNLOAD_DIAG		=	806,
				eMASSDSP_CONNECTIVITY_DIAG		=	807
				
}eTestID;

enum eCardUnitTypePhysical
{
	ePQ             = 0, //PowerQuick cpu
	eDsp            = 1,
	eRtm			= 2,
	eCntl           = 3,
	eAllDsps		= 99,
	eUndefined      = 0xff,

	NUM_OF_CARD_UNIT_TYPES_NOT_CONFIGURED = 4
};

typedef enum
{
	nothingIsRunning ,
	shovalMemTestInProgress,
	unicastTestInProgress
}unicastTestStatus;

#define MAX_SLOT_NUM	(ISDN_CARD_SLOT_ID + 1)

extern INT32 *	errorOccured[MAX_SLOT_NUM];

#define DGNS_TEST_NAME_MAX_LEN 	40	//dig later how to enlarge

#define DGNS_MAX_ERR_STRING_LEN 200

#define TEST_BONUS_SECONDS_TIME			2 //how much time can test take more than expected


#define		MS_SLEEP_BETWEEN_TESTS		300


typedef enum
{
	e_typeDSPtest, //usual test,sent to DSP for processing
	e_typeSerialTest,  //usual serial test,that is processed in normal serial way
	e_typeDSPtestRunSerialWay, //test that is processed in normal serial way,but with DSP interaction
	e_typeRTMTest,			//RTM test. is processed in serial way,but has own Timeout engine
	e_typeNoTestRunning,	//we're not running any test
	e_typeUnknown			//dummy error state
}e_TypeOfTest;


typedef struct
{
	UINT32 stopOnFail; 	 			//whether we have to stop on failure or continue
	UINT32 numOfElements;			//Number of test elements on the current session
	UINT32 quickSession;			//Run quick test version (when supported)
}dgnsDSPTestSession;

/* Structure to hold the test function pointer per tesd ID */
typedef struct
{
	UINT32	testId;
	char 	errorBuf[200];
}ErrBufferInfo;


/*
 * This structure is linked list of all the requested tests. It has info about
 * every test that (has been)/(will be) executed. The first member of test
 * is pointed to by the next defined strucutre (Test Session)
 * */
typedef struct DGNSTESTSTRUCT
{
	UINT32 testStartTime;
	UINT32 testId;			//what test is it?
	UINT32 loopsDone;		//how many times has this test been executed
	INT32  neededLoops;		//how many loops of this test do i need to do
	UINT32 successTests;	//how many successfull tests out of loopsDone
	UINT32 failTests;		//# of failed tests. Though can be calculated,but still
	UINT32 slotID;		// Slot ID
	UINT32 unitOnSlot;		//there are several units on slot. Which one are we working at
	UINT32 duration;		//value in seconds. How long has the test lasted
	UINT32 isActiveNow;		//are we in this test now?
	struct DGNSTESTSTRUCT * nextTest;
}dgnsCtrlTestInfo;


/*
 * This structure describes test session,as it will be requested by CM (Card manager)
 * It includes information for overall test session,and has pointer to linked list
 * of requested tests. The description of each test is above this struct.
 */
typedef struct
{
	INT32  numOfLoops;  			//how many loops to perform
	UINT32 stopOnFail; 	 			//whether we have to stop on failure or continue
	UINT32 slotId;					//on what slot id we are working
	UINT32 testSessionStartTime;	//for time measures
	dgnsCtrlTestInfo * firstTest; 	//pointer to first test.
	dgnsCtrlTestInfo * lastTest;	//pointer to last test. Just for ease of adding tests
	UINT32 hasStarted;				//indicator of "are we on?"
	UINT32 testSessionDurationTime; //how much has the test session taken
	UINT32 isQuickSession;
}dgnsTestSession;


/* Test status enumeration */
typedef enum
{
	eStatOk,
	eStatFail
}eStatus;


/*	This will hold statuses for our DSPs ,when requested board load test.
 *  This image will be sent between boards,so they would know what dsps exist on both sides.
 *  It has size of MAX_DSP_NUM_POSSIBLE	and is defined in BusLoadTests.c*/

typedef enum
{
	eDspStatusNonExistant,
	eDspStatusNotLoaded,
	eDspStatusLoaded,
	eDspStatusLoadFailed
}eDspStatusOnBoard;

/* Test results structure */
typedef struct{
	eStatus testResult;
	char	errString[DGNS_MAX_ERR_STRING_LEN];
	UINT32	testData;
	UINT32	testId;
	UINT32	slotId;
	UINT32	unitId;
}dgnsTestResult;

/*
 * This structure defines what fields each test that can be executed - must have.
 */

typedef struct
{
	UINT32 	TestId;
	INT8 	TestName[DGNS_TEST_NAME_MAX_LEN];
	UINT32 	TestOn;			//which unit run the test. Card=0xff,PQ=0,DSP=1
	UINT32 	canBeLooped; 	//Can the test be looped? boolean val
	UINT32 	isQuickVersion; 	//Is there short version of test? bool
	UINT32 	esimatedRunTime;	//in msec,the average runtime of test
	UINT32  SystemType;         // E_CHASSIS_TYPE_RMX2000, 4000, E_CHASSIS_TYPE_MAX for both
	VOID	(*testFunction)(dgnsTestResult*);
}dgnsTestInfo;


// Simulation tests
enum eTestType
{
	eMemoryTest		= 1,
	eNetworkTest		= 2,
	eLoopTests				= 3,

	NUM_OF_TEST_TYPES  = 3
};

#define DIAG_ROOT_DIR			"/mnt/mfa_cm_fs/diagnostics"
#define DIAG_TARGET_DIR			"/mnt/mfa_cm_fs/diagnostics/target"
#define DIAG_TARGET_FILE		"/mnt/mfa_cm_fs/diagnostics/target/Diagnostics.001"
#define DIAG_LOG_FILES_DIR	 	"/mnt/mfa_cm_fs/DiagLogFiles"
#define MD5FILENAME				"/mnt/mfa_cm_fs/VersionChecksums_DiagRtmIpPlus.txt"
#define FILENAME_ServiceList    "/mnt/mfa_cm_fs/ip/IPServiceList.xml"


#define MAX_NUM_OF_TESTS			70//
#define	MAX_NUM_OF_REPORTED_TESTS	320

#define DEFAULT_TIMEOUT		60000	// 1 min
#define ENTRY_QUEUE			STANDALONE_CONF_ID

//#define NUM_OF_DSPS_IN_SUB_BOARD		26
#define BOARD_NUMBER_1					1
#define BOARD_NUMBER_2					2

#define MAX_IP_ADDRESS_SIZE 			32
#define MAX_IPV6_ADDRESS_SIZE 			64

#define DEFAULT_SERVER_FIRST_PORT_RCV	2000
#define DEFAULT_SERVER_FIRST_PORT_XMT	4000
#define DEFAULT_MFA_FIRST_PORT_XMT		6000
#define DEFAULT_MFA_IP_ADDR				0xAC16BFDE	//172.22.191.222
#define DEFAULT_SERVER_IP_ADDR			0xAC16B855
#define DEFAULTC_SERVER_IP_ADDR			"172.22.184.85"

#define DEFAULT_SERVER_NET_MASK			0xFFFFF800
#define DEFAULTC_SERVER_NET_MASK 		"255.255.248.0"

#define DEFAULT_SERVER_DEF_GATEWAY		0xAc16B801
#define DEFAULTC_SERVER_DEF_GATEWAY		"172.22.184.1"

#define DEFAULT_SERVER_TCP_PORT			10004

#define DEFAULT_SWITCH_IP_ADDR			0xAC16B855
#define DEFAULTC_SWITCH_IP_ADDR			"172.22.184.85"

#define DEFAULT_SWITCH_NET_MASK			0xFFFFF800
#define DEFAULTC_SWITCH_NET_MASK 		"255.255.248.0"

#define DEFAULT_SWITCH_DEF_GATEWAY		0xAc16B801
#define DEFAULTC_SWITCH_DEF_GATEWAY		"172.22.184.1"

#define DEFAULT_SWITCH_TCP_PORT			10011

#define INTERNAL_TEST_REQUEST_OPCODE	987654

#define MAX_NUM_OF_ERRORS_IN_BUFFER		5

#define	TEST_DATA_FOR_SKIPPED_TEST			0xFAE5D6C0

typedef struct
{
	UINT32		testId;				// The test that produced the error
	char*		errorString;
}SIM_ERROR;

typedef struct
{
	SIM_ERROR	error[MAX_NUM_OF_ERRORS_IN_BUFFER];
	UINT32		putNextError;		// The next place in the buffer to write an error
	UINT32		getNextError;		// The place of the last reported error in the buffer
	UINT32		bufferOverlap;		// If more than MAX_NUM_OF_REPORTED_ERRORS were written to the buffer before the next report.

}SIM_ERROR_BUFFER;

/*****************************************************************************/
/*	Test report manager														 */
/*****************************************************************************/
typedef struct
{
	APIUBOOL	testInProgress;
	UINT32 		testSuccessCounter;
}SIM_TEST_REPORT_S;

#define MAX_NUM_OF_UNITS_TO_TEST	2 // 1 * CPU + 1 * card (infrastructure)
typedef struct
{
	INT8		testName[20];
	UINT32		unTestID;
	INT32		unUnitID;  			// specific unit ID (unit ID num) or all (0xFF)
	APIUBOOL	performTestFlag;
	UINT32		unLoopNum;
	UINT32		testInProgress;
	UINT32		testSuccessCounter;
	UINT32		unDuration;
	UINT32		unQuick;

} T_TEST_INFO;

typedef struct
{
	// Init TESTS_DATABASE
	INT32	unNumOfReqLoops;
	UINT32	unPerformedLoopNum;
	UINT32	unStopOnFail;
	UINT32	unDuration;
	T_TEST_INFO	tTestInfo[MAX_NUM_OF_TESTS + 1];
}TESTS_DATABASE;

/*****************************************************************************/
/*	Other threads' API														 */
/*****************************************************************************/
#define MAIN_MENU_MSG_OPCODE						1000017
#define INTERNAL_START_TEST_MSG_OPCODE	   			1000019

// Internal Ema start test message
typedef struct
{
	TGeneralMcmsCommonHeader	tGeneralMcmsCommonHeader;

}TInternalStartTestMessage;

/*****************************************************************************/
/*	Timer thread API												 */
/*****************************************************************************/
#define TIMER_MSG_OPCODE 	 			   1000018

typedef struct
{
	UINT32	lastMessageReceivedOpcode;
	UINT32	unBoardNum;

}TTimerMessageParams;

typedef struct
{
	TGeneralMcmsCommonHeader	tGeneralMcmsCommonHeader;
    TTimerMessageParams			tTimerMessageParams;

}TTimerMessage;

/*****************************************************************************/
//extern vars
extern UINT8	cServerIpAddr[MAX_IP_ADDRESS_SIZE], cServerNetMask[MAX_IP_ADDRESS_SIZE], cServerDefGateway[MAX_IP_ADDRESS_SIZE];
extern UINT8	cSwitchIpAddr[MAX_IP_ADDRESS_SIZE], cSwitchNetMask[MAX_IP_ADDRESS_SIZE], cSwitchDefGateway[MAX_IP_ADDRESS_SIZE];
extern UINT32 	ServerTcpPort;
extern UINT32 	SwitchTcpPort;

// Diagnostics
extern void BuildEnterDiagModeIndMsg(UINT32 ulMsgId, UINT32 ulSlotID);
extern void BuildTestListIndMsg(UINT32 ulMsgId, UINT32 ulSlotID);
extern void BuildStartTestIndMsg(UINT32 ulMsgId, UINT32 ulSlotID);
extern void BuildGetTestStatusIndMsg(UINT32 ulMsgId, UINT32 ulSlotID);
extern void BuildStopTestIndMsg(UINT32 ulMsgId, UINT32 ulSlotID);
extern void BuildUnitListIndMsg(UINT32 ulMsgId, UINT32 ulSlotID);
extern void BuildErrorListIndMsg(UINT32 ulMsgId, UINT32 ulSlotID);
extern void ParseStartTestReq(UINT32* pMsg, UINT32 ulSlotId);
e_TypeOfTest	getTestType(dgnsCtrlTestInfo * testToExamine);

typedef struct ERRLISTTYPE
{
	char *errDescr;
	int  errTestId;
	struct ERRLISTTYPE* 	nextError;
}errList;

/* This is the first and final init of mutex. As i dont release it,and use
 * for entire diag process,i dont reeinitialize it.
 * This is mutex for error logging operations. Write/Clear/Get from error linked list */

void errClearErrorLog(UINT32 ulSlotId);
void errReportError(UINT32 ulSlotId, int errNum,char* ErrorDesc);
int  errGetError(UINT32 ulSlotId, int errIndex,errList** errDescription); //if returns 0 - no such error in log

/* Adding new test to test session list*/
int dgnsTestAddTest(int testId, UINT32 ulSlotID, int unitOnSlot);
/* Erase whole list of tests*/
int dgnsClearTests(UINT32 ulSlotID);
/*Gets pointer to test, according to its index in the linked list. Returns 0 in pointer,if not found*/
int dgnsGetTestFromList(UINT32 ulSlotID, int testIndex, dgnsCtrlTestInfo **delTest);
dgnsTestInfo * testIdToTest(UINT32 TestId,dgnsTestInfo currentSystemTests[]);

//#define	PQUnitSlot 0
void printTestStatuses(UINT32 ulSlotID);

/*	message exchange queue between ema shel manager and diag. Pavelk	*/


int	isMessageFromEmaToDiagReady();
void sendMsgFromEmaToDiag(unsigned int msgSize,char	*msgSendBuf);
void rcvMsgFromEmaToDiag(unsigned int *msgSize,char *msgRcvdBuf);
int isMessageFromDiagToEmaReady();
void sendMsgFromDiagToEma(unsigned int msgSize, char *msgSendBuf);
void rcvMsgFromDiagToEma(unsigned int *msgSize,char *msgRcvdBuf);
/*---------------------------------------queue-----------*/

/*---------------------------------------EXT IP MEDIA CARD-----------*/

typedef enum
{
	eIPV4	= 0,
	eBoth	= 1,
	eIPV6	= 2
} eIpType;

typedef enum 
{
	eAuto		= 0,
	eManual		= 1,
} eIpv6ConfType;

typedef struct
{
	eIpType ipType;
	eIpv6ConfType ipv6ConfType;
	char IpAddr[MAX_IP_ADDRESS_SIZE];
	char netmask[MAX_IP_ADDRESS_SIZE];
	char IpGateway[MAX_IP_ADDRESS_SIZE];
	char Ipv6Addr1[MAX_IPV6_ADDRESS_SIZE];
	char Ipv6Addr2[MAX_IPV6_ADDRESS_SIZE];
	char Ipv6Addr3[MAX_IPV6_ADDRESS_SIZE];
	char Ipv6Gateway[MAX_IP_ADDRESS_SIZE];
}TMngrIpAddr;

extern INT32 GetMngIPAddr(TMngrIpAddr * mngrIpAddr);
extern void SetNICIpAddress(TMngrIpAddr * mngrIpAddr);
extern void GetCurrentIpAddress(TMngrIpAddr * mngrIpAddr, char * IPAddrString, int size);

#define EMPTY_IP "0.0.0.0"
#define MAX_IP_NUMBER 9
#define MAX_XML_LINE_LENGTH 512

#endif
