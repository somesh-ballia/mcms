#ifndef DIAGNOSTICS_INFO_H_
#define DIAGNOSTICS_INFO_H_

#include <string>
#include <list>
#include "DataTypes.h"
using namespace std;

#define DGNS_TEST_NAME_MAX_LEN 40
#define DNGS_MAX_ERR_STRING_LEN	200

#ifndef TRUE
#define TRUE 0x01
#endif

#ifndef FALSE
#define FALSE 0x00
#endif

typedef enum
{
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

	eCNTL_IPMC_CONNECTION				= 610
} CNTL_TYPE;

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

/*TODO: to use the global definition*/
typedef enum
{
	E_CHASSIS_TYPE_UNKNOWN = 0,
	E_CHASSIS_TYPE_RMX2000 = 1,
	E_CHASSIS_TYPE_RMX4000 = 2,
	E_CHASSIS_TYPE_RMX1500 = 4,
	E_CHASSIS_TYPE_MAX
} E_CHASSIS_TYPE;
/*
 * Test Result enum
 */

typedef enum
{
	eStatOk,
	eStatFail
}eStatus;

/*
 * Test Result structure
 */
typedef struct {
	eStatus testResult;
	char 	errString[DNGS_MAX_ERR_STRING_LEN];
	UINT32	testData;
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
	void	(*testFunction)(dgnsTestResult*);
}dgnsTestInfo;

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

extern dgnsTestSession diagCurrentSession;
/*
 * Functions for memory test
 */
void CNTLMemoryDataBusTest(dgnsTestResult *diagTestRes);
void CNTLMemoryAddressBusTest(dgnsTestResult *diagTestRes);
void CNTLMemoryEnergyTest(dgnsTestResult *diagTestRes);
void CNTLMemoryIntegrityTest(dgnsTestResult *diagTestRes);

int SystemPipedCommand(const string & system_command, string & output_string);
void CNTLHardDiskSmartCheck(dgnsTestResult *diagTestRes);
void CNTLHardDiskReadWriteCheck(dgnsTestResult *diagTestRes);
void CNTLUSBReadWriteCheck(dgnsTestResult *diagTestRes);
int  ReadWriteTemplate(unsigned char ch,char *szFileName,char * szMsgInfo,dgnsTestResult *diagTestRes);

void CNTLCompactFlashMD5Test(dgnsTestResult *diagTestRes);
void CNTLCompactFlashFSCheck(dgnsTestResult *diagTestRes);
void CNTLCompactFlashReadWriteTest(dgnsTestResult *diagTestRes);
int USBUnitTest(string szUSBDev,string szMountDir,dgnsTestResult *diagTestRes);
bool FindDevName(const string szMatch,const list<string> lstDevName,string &szDevName);
bool DelDevName(const string szDevName,list<string> &lstDevName);
/*
 * Functions for IPMC test
 */
void CNTLIPMCConntionTest(dgnsTestResult *diagTestRes);

int dgnsTestAddTest(int testId,int unitOnSlot);
int dgnsClearTests();
dgnsTestInfo * testIdToTest(UINT32 TestId,dgnsTestInfo currentSystemTests[]);
int dgnsGetTestFromList(int testIndex,dgnsCtrlTestInfo **findTest);

#endif /*DIAGNOSTICS_INFO_H_*/

