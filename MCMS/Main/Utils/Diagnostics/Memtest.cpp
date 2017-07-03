
#include <iostream>
#include <string.h>
#include "DataTypes.h"
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include "DiagnosticsInfo.h"

#include "IpmcInt.h"


#define DDR_BUS_SIZE 	32
#define DDR_SIZE_RESERVED	(32 * 1024 *1024)		/* to reserve 32M memory */
#define DDR_SIZE		(64 * 1024 *1024)		/* to test 64M everytime*/
#define ADDRESS_BUS_MAX_VALUE 100
#define INTEGRITY_UNIQUE_LOCATION 15
#define INTEGRITY_TEST_VALUE 0xAAAAAAAA

/*
 *Return the size of free memory in Kb
 */ 
int CheckFreememSize()
{
	int memsize;
	std::string answer;
	int stat = SystemPipedCommand("cat /proc/meminfo | grep MemFree | awk -F ' ' '{ print $2 }'", answer);
	if(0 != stat)
	{
		SLEEP(2000);
		stat = SystemPipedCommand("cat /proc/meminfo | grep MemFree | awk -F ' ' '{ print $2 }'", answer);
		if(0 != stat)
		{
			return 0;
		}
	}
	memsize = atoi(answer.c_str());
	return memsize;
} 

/*
 * writing Walking 1 to a bus and verify that there is no shortage
*/
void WalkingOnes(dgnsTestResult *diagTestRes, unsigned int *addr, unsigned int bussize)
{
	unsigned int i;
	diagTestRes->testResult = eStatOk;
	for (i=0; i<bussize; i++) /*bussize is in bits*/
	{
		/* write value */
		*(addr+i) = 0x1 << i;

		/* error behavior for unit test*/
		#ifdef UNIT_TEST
		if (i%5 == 0)
			*(addr + i) = *(addr+i) << 1;
		#endif
	}

	/* verify the data*/
	for (i=0; i<bussize; i++)
	{
		if (*(addr+i) != (unsigned int)(0x1 << i)) {
			sprintf(diagTestRes->errString, "Bus is Shortage at %p place."
				" Expected: 0x%x, Found: 0x%x", addr+i, 0x1<<i, *(addr+i));
			diagTestRes->testResult = eStatFail;
			return;
		}
	}
	return;
}

// Function that writes value to each of power of two addresses,
// writes invert of this val at one of these addresses, and verify
// that the writing was at the correct place.
// memSize - in bytes!!
int CheckAddressBus(dgnsTestResult *diagTestResult, volatile unsigned char *baseAddr, unsigned int memSize)
{
	unsigned int i=0, j;
	unsigned char volatile *addr;
	unsigned char val;
	#ifdef UNIT_TEST
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

			addr = (unsigned char*)(baseAddr + j);
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
		addr = (unsigned char*)(baseAddr + i);
		*addr = (~val) & 0xFF;
		#ifdef DEBUG_MODE
		if (i < 10)
			printf("\nAddress: 0x%p, Write 0x%x, Read 0x%x", addr, ((~val) & 0xFF), *addr);
		#endif
		// verify
		j=0;
		while (j < memSize)
		{
			addr = (unsigned char*)(baseAddr + j);
			if ((addr != ((unsigned char*)(baseAddr + i)))&&(*addr != val))
			{
				sprintf(diagTestResult->errString,"Address Bus is Wrong at address %p, %d Place."
						" Expected: 0x%X Found: 0x%X", addr, j, val, *addr);
				diagTestResult->testResult = eStatFail;
				return -1;

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
	return 0;
}

// Writing values in absending order from beginning and descending order from end
// (their invert value).
int EnergyTest(dgnsTestResult *diagTestResult, volatile unsigned int *baseAddr, unsigned int memSize)
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
	for (i=0, num=1 ; i<((unsigned int)(memSize)/8) ; i++, num++)
	{
		if (*((unsigned int*)baseAddr+i) != num)	// lower addresses
		{
			diagTestResult->testResult = eStatFail;
			sprintf(diagTestResult->errString,"Memory Wrong at 0x%p Address. Expected: 0x%x , Found: 0x%x", baseAddr+i, num, *(baseAddr+i));
			return -1;

		}

		if (*((unsigned int*)endAddr-i) != (~num & 0xFFFFFFFF))		// higher addresses
		{
			diagTestResult->testResult = eStatFail;
			sprintf(diagTestResult->errString,"Memory Wrong at %p Address. Expected: 0x%x , Found: 0x%x", endAddr-i, ~num, *(endAddr-i));
			return -1;
		}
	}
	return 0;
}

// writing and verifing ~INTEGRITY_TEST_VALUE to 15th location of memory.
int IntegrityTest(dgnsTestResult *diagTestResult, volatile unsigned int *baseAddr, unsigned int memSize)
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
				return -1;
			}
		}
	}
	return 0;
}

/*
 * CNTL Memory Data Bus Test implementation
 */
void CNTLMemoryDataBusTest(dgnsTestResult *diagTestRes)
{
	unsigned int *area = (unsigned int *) malloc(DDR_BUS_SIZE * sizeof(int));
	if (area == NULL) {
		diagTestRes->testResult = eStatFail;
		strcpy(diagTestRes->errString, "Allocation failed. Not enought free memory.");
		return;
	}
#ifdef UNIT_TEST
	printf("\nNow In CNTL Memory Data Bus Test");
#endif
	WalkingOnes(diagTestRes, area, DDR_BUS_SIZE);
	free(area);
	return;
}

// CNTL Memory Address Bus test implementation
void CNTLMemoryAddressBusTest(dgnsTestResult *diagTestRes)
{
	unsigned int tempAreaList[64];
	int flagFirst = 1;
	int flagStop = 0;
	int nAreaCnt = 0;
	unsigned char *area = NULL;
	int i;
	int freeMemSize;
	int testBlocks; 
	 
	for (i=0; i<64; i++) {
		tempAreaList[i] = 0;
	}
	freeMemSize = CheckFreememSize();
	testBlocks = (freeMemSize * 1024 - DDR_SIZE_RESERVED)/DDR_SIZE;
	if (testBlocks < 0) testBlocks = 0;	
	printf("CNTL Memory AddressBus Test: %d * 64M memory will be tested\r\n", testBlocks);

	while (flagStop != 1 && nAreaCnt < testBlocks) {

		area =(unsigned char*)malloc(DDR_SIZE*sizeof(char));
		if (area==NULL)
		{
			if (flagFirst == 1) {
				diagTestRes->testResult = eStatFail;
				strcpy(diagTestRes->errString,"Allocation failed. Not enough free memory.");
				return;
			} else {
				flagStop = 1;
				break;
			}
		}
		flagFirst = 0;
		if (CheckAddressBus(diagTestRes, area, DDR_SIZE) !=0)// memSize - in bytes!!
		{
			flagStop = 1;
		}
		tempAreaList[nAreaCnt] = (unsigned int )area;
		nAreaCnt++;
		printf("Now In CNTL AddressBus Test (Memory Addr: %p; loop: %d)\r\n", area,nAreaCnt);
	}
	for (i=0; i<nAreaCnt; i++) {
		free ((void*)tempAreaList[i]);
		tempAreaList[i] =0;
	}
	return;
}

// CNTL Memory Energy test implementation
void CNTLMemoryEnergyTest(dgnsTestResult *diagTestRes)
{
	unsigned int tempAreaList[64];
	int flagFirst = 1;
	int flagStop = 0;
	int nAreaCnt = 0;
	unsigned int *area = NULL;
	int i;
	int freeMemSize;
	int testBlocks; 

	for (i=0; i<64; i++) {
		tempAreaList[i] = 0;
	}

	freeMemSize = CheckFreememSize();
	testBlocks = (freeMemSize * 1024 - DDR_SIZE_RESERVED)/DDR_SIZE;
	if (testBlocks < 0) testBlocks = 0;	
	printf("CNTL Memory Energy Test: %d * 64M memory will be tested\r\n", testBlocks);

	while (flagStop != 1 && nAreaCnt < testBlocks) {

		area =(unsigned int*)malloc(DDR_SIZE*sizeof(int));
		if (area==NULL)
		{
			if (flagFirst == 1) {
				diagTestRes->testResult = eStatFail;
				strcpy(diagTestRes->errString,"Allocation failed. Not enough free memory.");
				return;
			} else {
				flagStop = 1;
				break;
			}
		}
		flagFirst = 0;
		if (EnergyTest(diagTestRes, area, DDR_SIZE) !=0)// memSize - in bytes!!
		{
			flagStop = 1;
		}
		tempAreaList[nAreaCnt] = (unsigned int )area;
		nAreaCnt++;
		printf("Now In CNTL Memory Energy Test (Memory Addr: %p; loop: %d)\r\n", area,nAreaCnt);
	}
	for (i=0; i<nAreaCnt; i++) {
		free ((void*)tempAreaList[i]);
		tempAreaList[i] =0;
	}
}


// CNTL Memory Integrity test implementation
void CNTLMemoryIntegrityTest(dgnsTestResult *diagTestRes)
{
	unsigned int tempAreaList[64];
	int flagFirst = 1;
	int flagStop = 0;
	int nAreaCnt = 0;
	unsigned int *area = NULL;
	int i;
	int freeMemSize;
	int testBlocks; 

	for (i=0; i<64; i++) {
		tempAreaList[i] = 0;
	}

	freeMemSize = CheckFreememSize();
	testBlocks = (freeMemSize * 1024 - DDR_SIZE_RESERVED)/DDR_SIZE;
	if (testBlocks < 0) testBlocks = 0;	
	printf("CNTL Memory Integrity Test: %d * 64M memory will be tested\r\n", testBlocks);

	while (flagStop != 1 && nAreaCnt < testBlocks) {

		area =(unsigned int*)malloc(DDR_SIZE*sizeof(int));
		if (area==NULL)
		{
			if (flagFirst == 1) {
				diagTestRes->testResult = eStatFail;
				strcpy(diagTestRes->errString,"Allocation failed. Not enough free memory.");
				return;
			} else {
				flagStop = 1;
				break;
			}
		}
		flagFirst = 0;
		if (IntegrityTest(diagTestRes, area, DDR_SIZE) !=0)// memSize - in bytes!!
		{
			flagStop = 1;
		}
		tempAreaList[nAreaCnt] = (unsigned int )area;
		nAreaCnt++;
		printf("Now In CNTL Memory Integrity Test (Memory Addr: %p; loop: %d)\r\n", area,nAreaCnt);
	}
	for (i=0; i<nAreaCnt; i++) {
		free ((void*)tempAreaList[i]);
		tempAreaList[i] =0;
	}
}
