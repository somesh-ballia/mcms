/*
 * Very simple (yet, for some reason, very effective) memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <memtest@discworld.dyndns.org>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2005 Charles Cazabon <memtest@discworld.dyndns.org>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file contains the declarations for the functions for the actual tests,
 * called from the main routine in memtester.c.  See other comments in that
 * file.
 *
 */

/* Function declaration. */
#include "Diagnostics.h"		//for data type info

typedef struct
{
	UINT32 	TestId;
	const INT8 *	CmdPath;
	INT8 	TestName[DGNS_TEST_NAME_MAX_LEN];
}ninjaCmdInfo;

typedef struct
{
	UINT32 	TestId;
	float 	LowerCritical;
	float 	Normal;
	float 	UpperCritical;
}ninjaThreshold;

void CNTLMemoryDataBusTest(dgnsTestResult *diagTestRes);
void CNTLMemoryAddressBusTest(dgnsTestResult *diagTestRes);
void CNTLMemorySizeTest(dgnsTestResult *diagTestRes);
void CNTLLocalBusCPLDTest(dgnsTestResult *diagTestRes);
void CNTLCoreClockTest(dgnsTestResult *diagTestRes);
void FPGADDR3MemTest(dgnsTestResult *diagTestRes);
void FPGASRAMMemTest(dgnsTestResult *diagTestRes);
void FPGAPCIeClockTest(dgnsTestResult *diagTestRes);
void CodecStressTest(dgnsTestResult *diagTestRes);
void DspDownloadTest(dgnsTestResult *diagTestRes);
void DspMemoryDataBusTest(dgnsTestResult *diagTestRes);
void DspMemoryAddressBusTest(dgnsTestResult *diagTestRes);
void DspCoreClockTest(dgnsTestResult *diagTestRes);
void RtmDspDiagTest(dgnsTestResult *diagTestRes);

//Ram tests
#if 0
void PqMemoryDataBusTest(dgnsTestResult *diagTestRes);
void PqMemoryAddressBusTest(dgnsTestResult *diagTestRes);
void PqMemoryEnergyTest(dgnsTestResult *diagTestRes);
void PqMemoryIntegrityTest(dgnsTestResult *diagTestRes);
#endif
//Flash tests
#define	FLASH_DIAG_DEVICE	"/dev/mtd0"

#define SHOVAL_BASE_OFFSET			0x10000 // 64K


#define	SHOVAL_GENERAL_TIMEOUT_MS		5000
#define SHOVAL_MEMTEST_DURATION_MS		1000

#define	SHOVAL_KEEP_ALIVE_OFFSET		658

#define	SHOVAL_MEMORY_CONTROL_OFFSET	300
#define	SHOVAL_START_MEM_TEST_BIT		0x1

#define	SHOVAL_MEMORY_STATUS_OFFSET		302
#define	SHOVAL_STATUS_TEST_IN_PROGRESS_BIT	0x1

#define	SHOVAL_MEMORY_ERROR_REPORT_OFFSET	306
#define	SHOVAL_ERROR_IN_MEMTEST_BIT			0x1
#define	SHOVAL_MEM_ERR_AT_LOW				0x2
#define	SHOVAL_MEM_ERR_AT_HIGH				0x4


/*	This bit has to be checked before starting mem test */
#define	SHOVAL_SEGMENTOR_STATUS				126
#define	SHOVAL_SEGMENTOR_OK_TO_START_MEMTEST	0x1


/*	This register is made of 4 groups of 4 bits. This is my indication,that the other board is inserted	*/
#define	SHOVAL_RX_LINK_LOCK_OFFSET		406


/*	This register has 4 first bits as status of "link ok" , and other 4 bits as "link rx ok"	*/
#define SHOVAL_LINK_STATUS_OFFSET		408
#define SHVL_LINK_OK_0_BIT       		0x01 // @ SHVL_LINK_STATUS_REG
#define SHVL_LINK_OK_1_BIT       		0x02 // @ SHVL_LINK_STATUS_REG
#define SHVL_LINK_OK_2_BIT       		0x04 // @ SHVL_LINK_STATUS_REG
#define SHVL_LINK_OK_3_BIT       		0x08 // @ SHVL_LINK_STATUS_REG
#define SHVL_LINK_RX_OK_0_BIT       	0x10 // @ SHVL_LINK_STATUS_REG
#define SHVL_LINK_RX_OK_1_BIT       	0x20 // @ SHVL_LINK_STATUS_REG
#define SHVL_LINK_RX_OK_2_BIT       	0x40 // @ SHVL_LINK_STATUS_REG
#define SHVL_LINK_RX_OK_3_BIT       	0x80 // @ SHVL_LINK_STATUS_REG



#define SHOVAL_CLOCK_STATUS_OFFSET		26
#define	SHOVAL_MGT_CLOCK_312_5MHZ_SIDE_A 0x1
#define	SHOVAL_MGT_CLOCK_312_5MHZ_SIDE_B 0x2
#define	SHOVAL_TAP_CLOCK_200			 0x4
#define SHOVAL_MEMTEST_DURATION_MS		1000


#define	CPLD_KEEP_ALIVE_OFFSET		0x0
#define	CPLD_KEEP_ALIVE_TEST_VAL	0x5A

#define	CPLD_VERSION_OFFSET			0x1


#define		FLASH_FREE_ADDRESSES		8*1024//0x10000 // 0x10000 * sizeof(int) = 0x40000

#define ADDRESS_BUS_MAX_VALUE 			100
#define DDR_BUS_SIZE					32
#define FLASH_BUS_SIZE					32
#define DDR_SIZE						100000
#define INTEGRITY_TEST_VALUE			0xAAAAAAAA
#define INTEGRITY_UNIQUE_LOCATION		15
#define FLASH_SECTOR_SIZE 				256*1024
#define	DDR_START_ADDR					0x00002000
#define	DDR_END_ADDR					0x0FFBFFFF
#define	FLASH_DATA_BUS_ADDR				0xFF000000

/* this is defines for md5checksum for rtmip*/
#define	ONE_LINE_MAX_SIZE	300
#define TMP_MD5_FILE "/tmp/RTMIP_tmpMD5.txt"



#define		IP_TEST_STATUS_FILE			"/tmp/iptestStat"

