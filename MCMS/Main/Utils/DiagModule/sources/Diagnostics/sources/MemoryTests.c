/*
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <memtest@discworld.dyndns.org>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2005 Charles Cazabon <memtest@discworld.dyndns.org>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file contains the functions for the actual tests, called from the
 * main routine in memtester.c.  See other comments in that file.
 *
 */

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <malloc.h>
#include <mtd/mtd-user.h>
#include <getopt.h>
#include <fcntl.h>

#include "MemtesterTypes.h"
#include "MemtesterSizes.h"
#include "Memtester.h"
#include "DiagDataTypes.h"
#include "SharedDefines.h"
#include "Print.h"
#include "Diagnostics.h"
//#include "mfa_board.h"
#include "EmaShared.h"
#include "DiagnosticsApiExt.h"
#include "DiagnosticsApi.h"
#include "SystemInfo.h"
#include "tools.h"
#include "timers.h"

extern eChassisType chassisType;

char progress[] = "-\\|/";
#define PROGRESSLEN 4
#define PROGRESSOFTEN 2500
#define CRITICAL_CNTL_TEST	0
#define CRITICAL_DSP_TEST	1

extern SIM_ERROR_BUFFER	errorBuffer;
extern void UpdateErrorBuffer();

pthread_mutex_t ninjaCmdMutex = PTHREAD_MUTEX_INITIALIZER;

/*
void WalkingOnes(dgnsTestResult *diagTestResult, volatile unsigned int *addr, unsigned int busSize);
void CheckAddressBus(dgnsTestResult *diagTestResult, volatile char *baseAddr, unsigned int memSize);
void EnergyTest(dgnsTestResult *diagTestResult, volatile unsigned int *baseAddr, unsigned int memSize);
void IntegrityTest(dgnsTestResult *diagTestResult, volatile unsigned int *baseAddr, unsigned int memSize);

extern UINT32 pQMainPalBaseAddr;

void PqCoreClockTest(dgnsTestResult *diagTestResult);


extern INT32 FlashOpenDev (INT8 *DevName,INT32 flags);
extern INT32 FlashReadDev(INT32 fd,void *buf,INT32 count);
extern INT32 FlashWriteDev(INT32 fd,void *buf,INT32 count);
extern void FlashCleanupDev (INT32 fd);
extern INT32 RunSystemCmd(INT8 *strCmd);
extern INT32 InitFdPtr (INT32 fd,char *devname, INT32 offset);
extern INT32 FlashEraseDev(INT32 Fd, INT32 start, INT32 count, INT32 unlock);


extern unicastTestStatus	isUnicastTestInProgress;
extern pthread_mutex_t 	unicastTestMutex;
extern boardConnectInfo brdConnList[AMOS_BOARD_COUNT];	//connection to baraks
*/

ninjaCmdInfo ninjaCmd[] = 
{
	// CNTL Memory Tests
	{eCNTL_CODEC_STRESS_TEST,   			PATHNAME_NinjaCodecStressCmdPath,	"/usr/rmx/bin/stress_test.conf"},
	{eCNTL_DDR_MEMORY_DATA_BUS,			PATHNAME_NinjaDiagCmdPath,			"CPUMemoryTest"},
	{eCNTL_DDR_MEMORY_ADDRESS_BUS,		PATHNAME_NinjaDiagCmdPath,			"CPUMemoryTest"},
	{eCNTL_DDR_MEMORY_INTEGRITY ,			PATHNAME_NinjaDiagCmdPath,			"GetCPUMemSize"},
	{eCNTL_DDR_MEMORY_ENERGY,				PATHNAME_NinjaDiagCmdPath,			"LBCpldTest"},
	{eCNTL_CF_CREATE_DELETE_READ_WRITE,	PATHNAME_NinjaDiagCmdPath,			"GetCPUClock"},
	
	// CNTL FPGA Tests
	{eCNTL_CF_FILE_SYSTEM_CHECK,			PATHNAME_NinjaDiagCmdPath,			"FSDRamTest"},
	{eCNTL_CF_MD5				,			PATHNAME_NinjaDiagCmdPath,			"FSRamTest"},
	{eCNTL_HARD_DISK_SMART_CHECK,			PATHNAME_NinjaDiagCmdPath,			"GetFPcieSpeed"},
	
	// DSP Tests
	{eDSP_DOWNLOAD_DIAG,					PATHNAME_NinjaDiagCmdPath,			"DspConnectTest"},
	{eDSP_DATA_BUS_DIAG,					PATHNAME_NinjaDiagCmdPath,			"DSPMemoryTest"},
	{eDSP_ADDRESS_BUS_DIAG,					PATHNAME_NinjaDiagCmdPath,			"DSPMemoryTest"},
	{eDSP_CORE_CLOCK_DIAG,					PATHNAME_NinjaDiagCmdPath,			"GetDspClock"},

    	// RTM Tests
	{eRTM_DSP_DOWNLOAD_DIAG,					PATHNAME_NinjaRtmDiagCmdPath,			"700"},
	{eRTM_DSP_DATA_BUS_DIAG,					PATHNAME_NinjaRtmDiagCmdPath,			"701"},
	{eRTM_DSP_ADDRESS_BUS_DIAG,				PATHNAME_NinjaRtmDiagCmdPath,			"702"},
	{eRTM_DSP_ENERGY_DIAG,                                  PATHNAME_NinjaRtmDiagCmdPath,			"703"},
	{eRTM_DSP_INTEGRITY_DIAG,					PATHNAME_NinjaRtmDiagCmdPath,			"704"},
	{eRTM_DSP_CLOCKS_DIAG,                                  PATHNAME_NinjaRtmDiagCmdPath,			"706"},
	{eRTM_DSP_T1_FALC1_DIAG,					PATHNAME_NinjaRtmDiagCmdPath,			"711"},
	{eRTM_DSP_T1_FALC2_DIAG,					PATHNAME_NinjaRtmDiagCmdPath,			"712"},
	{eRTM_DSP_T1_FALC3_DIAG,					PATHNAME_NinjaRtmDiagCmdPath,			"713"},
	{eRTM_DSP_E1_FALC1_DIAG,					PATHNAME_NinjaRtmDiagCmdPath,			"714"},
	{eRTM_DSP_E1_FALC2_DIAG,					PATHNAME_NinjaRtmDiagCmdPath,			"715"},
	{eRTM_DSP_E1_FALC3_DIAG,					PATHNAME_NinjaRtmDiagCmdPath,			"716"},
	{eRTM_DSP_LAST_TEST,                                     PATHNAME_NinjaRtmDiagCmdPath,			"720"},
};

ninjaThreshold ninjaCmdThreshold[] = 
{
	// CNTL Memory Tests
	{eCNTL_DDR_MEMORY_INTEGRITY ,			(float)4000000,	(float)7954740,	(float)16000000},
	{eCNTL_CF_CREATE_DELETE_READ_WRITE,	(float)1000,		(float)1999,		(float)3000},
	
	// CNTL FPGA Tests
	{eCNTL_HARD_DISK_SMART_CHECK,			(float)4,			(float)5,		(float)6},
	
	// DSP Tests
	{eDSP_CORE_CLOCK_DIAG,					(float)500,		(float)998,		(float)1500},
};

BOOL s_bFPGADDR3MemTest = FALSE;

void LockCNTLTest()
{
	if(CRITICAL_CNTL_TEST) pthread_mutex_lock (&ninjaCmdMutex);
}

void UnlockCNTLTest()
{
	if(CRITICAL_CNTL_TEST) pthread_mutex_unlock (&ninjaCmdMutex);
}

void LockDSPTest()
{
	if(CRITICAL_DSP_TEST) pthread_mutex_lock (&ninjaCmdMutex);
}

void UnlockDSPTest()
{
	if(CRITICAL_DSP_TEST) pthread_mutex_unlock (&ninjaCmdMutex);
}

void LockRTMTest()
{
	if(CRITICAL_DSP_TEST) pthread_mutex_lock (&ninjaCmdMutex);
}

void UnlockRTMTest()
{
	if(CRITICAL_DSP_TEST) pthread_mutex_unlock (&ninjaCmdMutex);
}

BOOL checkThreshold(UINT32 testid, float value)
{
	size_t i;
	for(i = 0; i < (sizeof(ninjaCmdThreshold)/sizeof(ninjaCmdThreshold[0])); i++)
	{
		if(ninjaCmdThreshold[i].TestId == testid)
		{
			if(value < ninjaCmdThreshold[i].LowerCritical || value > ninjaCmdThreshold[i].UpperCritical)
			{
				return FALSE;
			}
			return TRUE;
		}
	}
	return TRUE;
}

const char * GetCmdPath(UINT32 testid)
{
	size_t i;
	for(i = 0; i < sizeof(ninjaCmd)/sizeof(ninjaCmd[0]); i ++)
	{
		if(ninjaCmd[i].TestId == testid) return ninjaCmd[i].CmdPath;
	}
	
	return "";
}

const char * GetCommand(UINT32 testid)
{
	size_t i;
	for(i = 0; i < sizeof(ninjaCmd)/sizeof(ninjaCmd[0]); i ++)
	{
		if(ninjaCmd[i].TestId == testid) return ninjaCmd[i].TestName;
	}
	
	return "";
}

void DoNinjaCmdTest(dgnsTestResult *diagTestRes, BOOL hasDspID, char * szOut, int size)
{
	int ret;
	char ** strArray = NULL;
	int count = 0;
	char szDspId[64]= {0};
	char * ptDspId = NULL;
	if(hasDspID)
	{
		//snprintf(szDspId, 64, "%d", (diagTestRes->slotId - DSP_CARD_SLOT_ID_0)*MAX_DSPUNIT_ON_CARD_NUM + diagTestRes->unitId - 1);
		snprintf(szDspId, 64, "%d", CovnUnitIdToHW(diagTestRes->slotId, diagTestRes->unitId));
		ptDspId = szDspId;
	}
	
	ret = ExecNinjaDiagCmd(&strArray, &count, GetCmdPath(diagTestRes->testId), GetCommand(diagTestRes->testId), ptDspId);
	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
		"DoNinjaCmdTest: testid(%d)  slotId(%d)  unitId(%d) Exec(%s %s %s)  ret(%d)",
		diagTestRes->testId, diagTestRes->slotId, diagTestRes->unitId, GetCmdPath(diagTestRes->testId), GetCommand(diagTestRes->testId), NVSTR(ptDspId), ret);
	
	if(ret == 0)
	{
		diagTestRes->testResult = eStatOk;
		if(NULL != strArray && count >= 1 && szOut != NULL)
		{
			strncpy(szOut, strArray[0], size);
			szOut[size - 1] = '\0';
			MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
				"DoNinjaCmdTest: eStatOk(%s)", szOut);
		}
	}
	else
	{
		diagTestRes->testResult = eStatFail;
		if(NULL != strArray && count >= 1 && NULL != strArray[0])
		{
			if(hasDspID)
			{
				snprintf(diagTestRes->errString, DGNS_MAX_ERR_STRING_LEN, "DSP (%d) %s", diagTestRes->unitId, strArray[0]);
			}
			else
			{
				snprintf(diagTestRes->errString, DGNS_MAX_ERR_STRING_LEN, "%s", strArray[0]);
			}
			MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
				"DoNinjaCmdTest: eStatFail(%s)", diagTestRes->errString);
		}
	}
	
	if(strArray)
	{
		LineSplitFree(strArray, count);
	}
	
	return;
}

//CNTL Tests
void CNTLMemoryDataBusTest(dgnsTestResult *diagTestRes)
{
	LockCNTLTest();
	DoNinjaCmdTest(diagTestRes, FALSE, NULL, 0);
	UnlockCNTLTest();
}

void CNTLMemoryAddressBusTest(dgnsTestResult *diagTestRes)
{
	LockCNTLTest();
	DoNinjaCmdTest(diagTestRes, FALSE, NULL, 0);
	UnlockCNTLTest();
}

void CNTLMemorySizeTest(dgnsTestResult *diagTestRes)
{
	char memory[1024] = {0};
	int size = 1024;
	float fmem;
	LockCNTLTest();
	DoNinjaCmdTest(diagTestRes, FALSE, memory, size);
	UnlockCNTLTest();
	if(diagTestRes->testResult == eStatOk)
	{
		//7954740KB
		fmem = atof(memory);
		if(!checkThreshold(diagTestRes->testId, fmem))
		{
			diagTestRes->testResult = eStatFail;
			snprintf(diagTestRes->errString, DGNS_MAX_ERR_STRING_LEN, "CNTL Memory Size Wrong. Found: %s", memory);
		}
	}
}

void CNTLLocalBusCPLDTest(dgnsTestResult *diagTestRes)
{
	LockCNTLTest();
	DoNinjaCmdTest(diagTestRes, FALSE, NULL, 0);
	UnlockCNTLTest();
}

void CNTLCoreClockTest(dgnsTestResult *diagTestRes)
{
	char clock[1024] = {0};
	int size = 1024;
	float fclock;
	LockCNTLTest();
	DoNinjaCmdTest(diagTestRes, FALSE, clock, size);
	UnlockCNTLTest();
	if(diagTestRes->testResult == eStatOk)
	{
		//1999MHz
		fclock = atof(clock);
		if(!checkThreshold(diagTestRes->testId, fclock))
		{
			diagTestRes->testResult = eStatFail;
			snprintf(diagTestRes->errString, DGNS_MAX_ERR_STRING_LEN, "CNTL CPU Frequency Wrong. Found: %s", clock);
		}		
	}
}

void FPGADDR3MemTest(dgnsTestResult *diagTestRes)
{
	LockDSPTest();
	DoNinjaCmdTest(diagTestRes, FALSE, NULL, 0);
	s_bFPGADDR3MemTest = TRUE;
	UnlockDSPTest();
}

//FPGA Tests
void FPGASRAMMemTest(dgnsTestResult *diagTestRes)
{
	LockDSPTest();
	DoNinjaCmdTest(diagTestRes, FALSE, NULL, 0);
	UnlockDSPTest();
}

void FPGAPCIeClockTest(dgnsTestResult *diagTestRes)
{
	char clock[1024] = {0};
	int size = 1024;
	float fclock;
	LockDSPTest();
	DoNinjaCmdTest(diagTestRes, FALSE, clock, size);
	UnlockDSPTest();
	if(diagTestRes->testResult == eStatOk)
	{
		//5.0GT/s
		fclock = atof(clock);
		if(!checkThreshold(diagTestRes->testId, fclock))
		{
			diagTestRes->testResult = eStatFail;
			snprintf(diagTestRes->errString, DGNS_MAX_ERR_STRING_LEN, "FPGA PCIE Speed Wrong. Found: %s", clock);
		}	
	}
}

void CodecStressTest(dgnsTestResult *diagTestRes)
{
	LockDSPTest();
      if(FALSE == s_bFPGADDR3MemTest)
      {
        DoNinjaCmdTest(diagTestRes, FALSE, NULL, 0);
      }
      else
      {
        diagTestRes->testResult = eStatOk;
        snprintf(diagTestRes->errString, DGNS_MAX_ERR_STRING_LEN, "%s", "Skip Codec Stress Test After FPGA DDR3 Mem Test.");
        MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
				"DoNinjaCmdTest: eStatOk(%s)", diagTestRes->errString);
      }
	UnlockDSPTest();
}

//DSP Tests
void DspDownloadTest(dgnsTestResult *diagTestRes)
{
	LockDSPTest();
	DoNinjaCmdTest(diagTestRes, TRUE, NULL, 0);
	UnlockDSPTest();
}

void DspMemoryDataBusTest(dgnsTestResult *diagTestRes)
{
	LockDSPTest();
	DoNinjaCmdTest(diagTestRes, TRUE, NULL, 0);
	UnlockDSPTest();
}

void DspMemoryAddressBusTest(dgnsTestResult *diagTestRes)
{
	LockDSPTest();
	DoNinjaCmdTest(diagTestRes, TRUE, NULL, 0);
	UnlockDSPTest();
}

void DspCoreClockTest(dgnsTestResult *diagTestRes)
{
	char clock[1024] = {0};
	int size = 1024;
	float fclock;
	LockDSPTest();
	DoNinjaCmdTest(diagTestRes, TRUE, clock, size);
	UnlockDSPTest();
	if(diagTestRes->testResult == eStatOk)
	{
		//998MHz
		fclock = atof(clock);
		if(!checkThreshold(diagTestRes->testId, fclock))
		{
			diagTestRes->testResult = eStatFail;
			snprintf(diagTestRes->errString, DGNS_MAX_ERR_STRING_LEN, "DSP (%d) Frequency Wrong. Found: %s", diagTestRes->unitId, clock);
		}		
	}
}

BOOL s_bRTMT1Tested = FALSE;    //eRTM_DSP_T1_FALC3_DIAG
BOOL s_bRTME1Tested = FALSE;    //eRTM_DSP_E1_FALC3_DIAG

void RtmDspDiagTest(dgnsTestResult *diagTestRes)
{
    LockRTMTest();
    if((diagTestRes->testId == eRTM_DSP_T1_FALC3_DIAG && s_bRTME1Tested) 
        || (diagTestRes->testId == eRTM_DSP_E1_FALC3_DIAG && s_bRTMT1Tested) )
    {
        char * testName = diagTestRes->testId == eRTM_DSP_T1_FALC3_DIAG ? "RTM DSP T1 FALC3" : "RTM DSP E1 FALC3";
        char * testedName = diagTestRes->testId == eRTM_DSP_T1_FALC3_DIAG ? "RTM DSP E1 FALC3" : "RTM DSP T1 FALC3";
        EmbSleep(10*1000);
        diagTestRes->testResult = eStatFail;
        snprintf(diagTestRes->errString, DGNS_MAX_ERR_STRING_LEN, "Skip %s Test After %s Test.", testName, testedName);
        MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
    			"DoNinjaCmdTest: eStatFail(%s)", diagTestRes->errString);
    }
    else
    {
        StartRtmDiagServer();
        DoNinjaCmdTest(diagTestRes, FALSE, NULL, 0);
        StopRtmDiagServer();
        if(diagTestRes->testId == eRTM_DSP_T1_FALC3_DIAG) s_bRTMT1Tested = TRUE;
        else if(diagTestRes->testId == eRTM_DSP_E1_FALC3_DIAG) s_bRTME1Tested = TRUE;
    }
    UnlockRTMTest();
}


#if 0

//-------------Start Pavel's code here:

// PQ Memory Data Bus test implementation
void PqMemoryDataBusTest(dgnsTestResult *diagTestRes)
{
	unsigned int *area = (unsigned int*)malloc(DDR_BUS_SIZE*sizeof(unsigned int));

	if (area==NULL)
	{
		diagTestRes->testResult = eStatFail;
		strcpy(diagTestRes->errString,"Allocation failed. Not enough free memory.");
		return;
	}
	#ifdef DEBUG_MODE
		printf("\nNow In Pq Memory Data Bus Test");
	#endif
	WalkingOnes(diagTestRes, area, DDR_BUS_SIZE);	// bus size in bits!
	free (area);
}

// PQ Memory Address Bus test implementation
void PqMemoryAddressBusTest(dgnsTestResult *diagTestRes)
{
	char *area = (char*)malloc(DDR_SIZE*sizeof(char));

	if (area == NULL)
	{
		diagTestRes->testResult = eStatFail;
		strcpy(diagTestRes->errString,"Allocation failed. Not enough free memory.");
		return;
	}
	#ifdef DEBUG_MODE
		printf("\nNow In Pq Memory Address Bus Test");
	#endif
	CheckAddressBus(diagTestRes, area, DDR_SIZE); // memSize - in bytes!!
	free (area);
}

// PQ Memory Energy test implementation
void PqMemoryEnergyTest(dgnsTestResult *diagTestRes)
{
	unsigned int *area = (unsigned int*)malloc(DDR_SIZE*sizeof(int));

	if (area==NULL)
	{
		diagTestRes->testResult = eStatFail;
		strcpy(diagTestRes->errString,"Allocation failed. Not enough free memory.");
		return;
	}
	#ifdef DEBUG_MODE
		printf ("\nIn Pq Memory Energy Test");
	#endif
	EnergyTest(diagTestRes, area, DDR_SIZE); // memSize - in bytes!!
	free (area);
}

// PQ Memory Integrity test implementation
void PqMemoryIntegrityTest(dgnsTestResult *diagTestRes)
{
	unsigned int *area = (unsigned int*)malloc(DDR_SIZE*sizeof(unsigned int));

	if (area==NULL)
	{
		diagTestRes->testResult = eStatFail;
		strcpy(diagTestRes->errString,"Allocation failed. Not enough free memory.");
		return;
	}
	#ifdef DEBUG_MODE
		printf("\nNow In PQ Memory Integrity Test");
	#endif
	IntegrityTest(diagTestRes, area, DDR_SIZE); // memSize - in bytes!!
	free (area);
}


// Function that writing walking 1 to a bus and verify that there is no shortage.
void WalkingOnes(dgnsTestResult *diagTestResult, volatile unsigned int *addr, unsigned int busSize)
{
	unsigned int i;

	diagTestResult->testResult = eStatOk;
	for (i=0; i<busSize; i++)  // busSize in bits!!
	{
		// write value
		*(addr+i) = 0x1 << i;
		// error behavior
		#ifdef UNIT_TEST
			if (i%5 == 0)
				*(addr+i) = *(addr+i) << 1;
		#endif
	}

		// verify
	for (i=0; i<busSize; i++)  // busSize in bits!!
	{
		if (*(addr+i) != (0x1 << i))
		{
			sprintf(diagTestResult->errString,"Bus is Shortage at %d Place."
					" Expected: 0x%x , Found: 0x%x", i, 0x1 << i, *(addr+i));
			diagTestResult->testResult = eStatFail;
			#ifdef UNIT_TEST
				printf("\n%s", diagTestResult->errString);
			#else
				return;
			#endif
		}
		#ifdef DEBUG_MODE
			if (i%5 == 3)
				printf("\nWrite 0x%x, Read 0x%x",0x1 << i, *(addr+i));
		#endif
	}
}

// Function that writes value to each of power of two addresses,
// writes invert of this val at one of these addresses, and verify
// that the writing was at the correct place.
// memSize - in bytes!!
void CheckAddressBus(dgnsTestResult *diagTestResult, volatile char *baseAddr, unsigned int memSize)
{
	unsigned int i=0, j;
	char volatile *addr;
	char val;
	#if defined UNIT_TEST
		unsigned int modulu = 10;
	#endif
	// initialize random value
	val = rand() % ADDRESS_BUS_MAX_VALUE;
	diagTestResult->testResult = eStatOk;
	// start writing
	while (i < memSize)
	{
		j=0;
		while (j < memSize)
		{

			addr = (char*)(baseAddr + j);
			*addr = val;

			// error behavior
			#ifdef UNIT_TEST
			if (j%modulu == 0)
				*addr = *addr << 1;
			#endif
			#ifdef DEBUG_MODE
				if (i < 20)
					printf("\nAddress: 0x%p, Write 0x%x, Read 0x%x", addr, val, *addr);
			#endif
			if (j == 0)
				j++;
			else
			j <<= 1;
		}

		// one value is inverted
		addr = (char*)(baseAddr + i);
		*addr = (~val) & 0xFF;
		#ifdef DEBUG_MODE
		if (i < 10)
			printf("\nAddress: 0x%p, Write 0x%x, Read 0x%x", addr, ((~val) & 0xFF), *addr);
		#endif
		// verify
		j=0;
		while (j < memSize)
		{
			addr = (char*)(baseAddr + j);
			if ((addr != ((char*)(baseAddr + i)))&&(*addr != val))
			{
				sprintf(diagTestResult->errString,"Address Bus is Wrong at address %p, %d Place."
						" Expected: 0x%X Found: 0x%X", addr, j, val, *addr);
				diagTestResult->testResult = eStatFail;
			#ifdef UNIT_TEST
				printf("\n%s", diagTestResult->errString);
			#else
				return;
			#endif
			}
			if (j == 0)
				j++;
			else
				j <<= 1;
		}
		if (i == 0)
			i++;
		else
			i <<= 1;
	}
}

// Writing values in absending order from beginning and descending order from end
// (their invert value).
void EnergyTest(dgnsTestResult *diagTestResult, volatile unsigned int *baseAddr, unsigned int memSize)
{
	// Fill addresses with numbers
	unsigned int i, num = 0x1;
	unsigned int volatile *endAddr = (unsigned int*)((unsigned int)baseAddr + memSize);
	#if defined DEBUG_MODE || defined UNIT_TEST
		unsigned int modulu = (memSize/2)/6;
	#endif

	diagTestResult->testResult = eStatOk;
	// writing
	for (i=0 ; i<(memSize/8) ; i++ , num++)
	{
		*((unsigned int*)baseAddr+i) = num;
		*((unsigned int*)endAddr-i) = (~num & 0xFFFFFFFF);

		// Unit test mode
		#ifdef UNIT_TEST
			if (i%modulu == 0)
				*((unsigned int*)baseAddr+i) = num + 1;
			else if (i%modulu == 1)
				*((unsigned int*)endAddr-i) = (~num & 0xFFFFFFFF) + 1;
		#endif

		#ifdef DEBUG_MODE
			if (i%modulu == 1)
			{
				printf("\nAddress: %p, Write 0x%x, Read 0x%x", baseAddr+i, num, *(baseAddr+i));
				printf("\nAddress: %p, Write 0x%x, Read 0x%x-", endAddr-i, (~num & 0xFFFFFFFF), *(endAddr-i));
				printf("\n");
			}
		#endif
	}
	// check the values that were written
	for (i=0, num=1 ; i<((int)(memSize)/8) ; i++, num++)
	{
		if (*((unsigned int*)baseAddr+i) != num)	// lower addresses
		{
			diagTestResult->testResult = eStatFail;
			sprintf(diagTestResult->errString,"Memory Wrong at 0x%p Address. Expected: 0x%x , Found: 0x%x", baseAddr+i, num, *(baseAddr+i));

			#ifdef UNIT_TEST
				printf("\n%s",diagTestResult->errString);
			#else
				return;
			#endif
		}

		if (*((unsigned int*)endAddr-i) != (~num & 0xFFFFFFFF))		// higher addresses
		{
			diagTestResult->testResult = eStatFail;
			sprintf(diagTestResult->errString,"Memory Wrong at %p Address. Expected: 0x%x , Found: 0x%x", endAddr-i, ~num, *(endAddr-i));

			#ifdef UNIT_TEST
				printf("\n%s",diagTestResult->errString);
			#else
				return;
			#endif
		}
	}
}

// writing and verifing ~INTEGRITY_TEST_VALUE to 15th location of memory.
void IntegrityTest(dgnsTestResult *diagTestResult, volatile unsigned int *baseAddr, unsigned int memSize)
{
	unsigned int i;
	unsigned int j=0;
	unsigned int volatile *addr;

	diagTestResult->testResult = eStatOk;
	for (i=0; i < INTEGRITY_UNIQUE_LOCATION; i++) // num. of tests
	{
		// writing value
		for (addr = baseAddr+i , j=0 ; addr < (unsigned int*)((unsigned int)baseAddr+memSize) ;addr++,j++)
		{
			// every 15 addresses
			if ((j % INTEGRITY_UNIQUE_LOCATION) == 0)
				*addr = INTEGRITY_TEST_VALUE;
			// regular value
			else
				*addr = ((~INTEGRITY_TEST_VALUE) & 0xFFFFFFFF);
			// error behavior
			#ifdef UNIT_TEST
			if ((j % (INTEGRITY_UNIQUE_LOCATION*5)) == 0)
				*addr = INTEGRITY_TEST_VALUE + 1;
			#endif

			#ifdef DEBUG_MODE
				if ((j % ((INTEGRITY_UNIQUE_LOCATION*5) + 15) == 0) && (j < 1000))
				{
					printf("\nAddress: 0x%p, Write 0x%X, Read 0x%X", addr, INTEGRITY_TEST_VALUE, *addr);
				}
			#endif
		}

		// verify
		for (addr = baseAddr+i ; addr < (unsigned int*)((unsigned int)baseAddr+memSize) ;
		addr += INTEGRITY_UNIQUE_LOCATION)
		{
			if (*addr != INTEGRITY_TEST_VALUE)
			{
				diagTestResult->testResult = eStatFail;
				sprintf(diagTestResult->errString,"Memory Wrong at 0x%x Address. Expected: 0x%X, Found 0x%X", (int)addr, INTEGRITY_TEST_VALUE, *addr);
				// Unit test mode
				#ifdef UNIT_TEST
					printf("\n%s",diagTestResult->errString);
				#else
					return;
				#endif
			}
		}
	}
}


void PqCoreClockTest(dgnsTestResult *diagTestRes)
{
      diagTestRes->testResult = eStatOk;
	return;
	
	int			savedSectorVals[FLASH_SECTOR_SIZE];
	struct mtd_info_user mtd;
	int FlashDevfd = -1;
	MENV	*cfgSectorPtr;
	diagTestRes->testResult = eStatOk;

	// open flash for write
	FlashDevfd = FlashOpenDev (FLASH_DIAG_DEVICE,O_SYNC | O_RDWR);
	if (ioctl (FlashDevfd,MEMGETINFO,&mtd) < 0)
    {
		diagTestRes->testResult = eStatFail;
		sprintf(diagTestRes->errString,"This doesn't seem to be a valid MTD flash device!");
   	   	return;
    }

	//SaveConfigSectorVals(savedSectorVals,FLASH_SECTOR_SIZE,FlashDevfd);
	cfgSectorPtr = (MENV *)(&(savedSectorVals[0]));
	if (cfgSectorPtr->isClockTestPassed == 1)
	{
		diagTestRes->testResult = eStatOk;
	}
	else
	{
		diagTestRes->testResult = eStatFail;
		sprintf(diagTestRes->errString,"Core clock test failed in U-boot");
	}
	FlashCleanupDev(FlashDevfd);

}

void PqLocalBusTest(dgnsTestResult *diagTestRes)
{
}

void FpgaSwitchMemoryTest(dgnsTestResult *diagTestRes)
{
}


void FpgaPIPCI0ClockTest(dgnsTestResult *diagTestRes)
{
}

void DspConnectivityTest(dgnsTestResult *diagTestRes)
{
}

#endif

