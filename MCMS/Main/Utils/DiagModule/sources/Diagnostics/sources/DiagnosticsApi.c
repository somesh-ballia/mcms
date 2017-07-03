/*============================================================================*/
/*            Copyright ?? 2006 Polycom Israel,Ltd. All rights reserved        */
/*----------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary        */
/* information of Polycom Israel, Ltd. and is protected by law.               */
/* It may not be copied or distributed in any form or medium, disclosed  to   */
/* third parties, reverse engineered or used in any manner without prior      */
/* written authorization from Polycom Israel Ltd.                             */
/*----------------------------------------------------------------------------*/
/* FILE:     	EmaApi.c                                                      */
/* PROJECT:  	Switch Card - Ema API Module								  */
/* PROGRAMMER:  Eyal Ben-Sasson												  */
/* DESCRIPTION: This Module Rcv on TCP connection the EMA string Req Messages,*/
/*				Then transfer the req to the relevant TCP connection ...	  */
/*				( LanTcpClient... DiagTcpClient... IpmiTcpClient... ).		  */
/*              This Module Trnasmit string Req Messages on TCP connection    */
/*				to the EMA From all the above modules.						  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                            */
/*----------------------------------------------------------------------------*/
/*         |                 |                                       		  */
/*============================================================================*/

#include <sys/time.h>
#include <string.h>

#include "LinuxSystemCallsApi.h"
#include "SocketApiTypes.h"
#include "SocketApiWrapExt.h"
#include "DiagnosticsShared.h"
#include "DiagnosticsApiExt.h"
#include "DiagnosticsApi.h"
#include "Memtester.h"
#include "EmaShared.h"
//#include "CardsStructs.h"
#include "StatusesGeneral.h"

#include "Print.h"
#include "SystemInfo.h"
#include "dspTestCtrl.h"
#include "IpmiConsts.h"
#include "tools.h"
#include "timers.h"
#include "MplStartup.h"

#define DIAG_RESULT_USB_PATH								"/mnt/usb/diagnostic/"
#define DIAG_RESULT_LOCAL_PATH							"/output/diagnostic/"
#define FILENAME_sysInfo     "/config/mfa_x86_env/cfg/sysInfo.txt"

char g_diagResultFileName[512];
char g_diagUSBResultFileName[512];
char g_mngIPAddr[MAX_IPV6_ADDRESS_SIZE];

extern void IpmcCheckTempSensors(dgnsTestResult *diagTestRes);
//extern void IpmcCheckComm(dgnsTestResult *diagTestRes);

extern char externalIps[MAX_IP_NUMBER][MAX_IP_ADDRESS_SIZE];

extern void PqCoreClockTest(dgnsTestResult *diagTestResult);

//From DiagMediaCard
//extern void PqLocalBusTest(dgnsTestResult *diagTestRes);
//extern void FpgaSwitchMemoryTest(dgnsTestResult *diagTestRes);
//extern void FpgaPIPCI0ClockTest(dgnsTestResult *diagTestRes);
extern void DspDownloadTest(dgnsTestResult *diagTestRes);
//extern void DspConnectivityTest(dgnsTestResult *diagTestRes);

extern void my_changeColonToDot(char *pSource);

extern eChassisType chassisType;


void dummyTest(dgnsTestResult *diagTestRes);
void dummySleepSuccessTest(dgnsTestResult *diagTestRes);

void PrintDgnsTestInfo(const dgnsTestInfo *pTestEntry);
void WriteDiagResultFile(const dgnsTestInfo *pTestEntry, const dgnsCtrlTestInfo * pTestCurrent, const dgnsTestResult * pTestResults);


unsigned int currentLogLoopNumber = 0;
UINT8 dspTestsFailed[MAX_SLOT_NUM]; //this is used by DSP,to tell us,that dsps had failed test.
					//it is useful,when we use the StopOnFail flag,so main program will know to stop
UINT32 RtmCardType = 0;  ///0 unknown  1  ISDN  2 LAN
UINT8 cm_rtm_receive = 0;

dgnsTestInfo currentSystemTests[] =
{

#if 0 // Test From Switch

    // RMX1500, RMX2000, RMX4000
    
	// Test ID                   		  Test Name                     UNIT   			Loop  	 Quick 			Time    Func Pointer // PQ Tests
	{ePQ_MEMORY_DATA_BUS_DIAG           ,"PQ Mem-Data Bus"           ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_ALL_SYSTEMS, PqMemoryDataBusTest},
	{ePQ_MEMORY_ADDRESS_BUS_DIAG        ,"PQ Mem-Addr Bus"           ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_ALL_SYSTEMS, PqMemoryAddressBusTest},
	{ePQ_MEMORY_ENERGY_DIAG		        ,"PQ Mem-Ram Energy"         ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_ALL_SYSTEMS, PqMemoryEnergyTest},
	{ePQ_MEMORY_INTEGRITY_DIAG        	,"PQ Mem-Ram Integr"         ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_ALL_SYSTEMS, PqMemoryIntegrityTest},

	//{eFLASH_DATA_BUS_DIAG        		,"PQ Flash-Data Bus"         ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_ALL_SYSTEMS, FlashDataBusTest},
	//{eFLASH_ADDRESS_INTEGRITY_DIAG		,"PQ Flash-Addr/Integ"       ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_ALL_SYSTEMS, FlashAddressIntegrityTest},

	//{eIPMC_UART_CHANNEL_DIAG        	,"IPMC Uart Channel"         ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_ALL_SYSTEMS, IpmcCheckComm},
	//{eIPMC_I2C_SENSORS_DIAG				,"IPMC Sensors Test"		 ,ePQ			,TRUE,		TRUE,	   1,  E_BIT_OP_CHASSIS_ALL_SYSTEMS, IpmcCheckTempSensors},


    // RMX4000

	{eFPGA_SHOVAL_KEEP_ALIVE_TEST  		,"Shoval Keep Alive"         ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_TYPE_RMX4000, ShovalFpgaKeepAliveTest},

	{eFPGA_SHOVAL_MEM_TEST        		,"Shoval Mem Test"           ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_TYPE_RMX4000, ShovalFgpaMemTest},
	{eFPGA_SHOVAL_LINK_TEST		        ,"Shoval Link Test"          ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_TYPE_RMX4000, ShovalFpgaLinkTest},
	{eFPGA_SHOVAL_CLOCKS_TEST        	,"Shoval Clocks Test"        ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_TYPE_RMX4000, ShovalFgpaClockTest},

	{eFPGA_SHOVAL_CPLD_TEST        		,"RTP IP CPLD Test" 	     ,ePQ     		,TRUE,      TRUE,      1,  E_BIT_OP_CHASSIS_TYPE_RMX4000, ShovalCPLDVersionKeepAliveTest},
    
    
//	{eSHOVAL_MCAST_TEST					,"McastLoad Test"			 ,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_TYPE_RMX4000, artMulticastTest},

    // RMX1500, RMX2000, RMX4000
    
	/*{eLAN_TEST_MPMS						,"MPM Lan Tests"			,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_ALL_SYSTEMS, startLanTest},*/

//the first test is good for rmx4000 (first card) , and rmx1500

	{eLAN_TEST_SIGNALING				,"LAN Load SIGNALING"		,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_ALL_SYSTEMS, startLanLoadSignaling},

	{eLAN_TEST_MFA1						,"LAN Load MPM1"			,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_ALL_SYSTEMS, startLanLoadBoard1},
	{eLAN_TEST_MFA1_EXT					,"EXT LAN Load MPM1"			,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_ALL_SYSTEMS, startLanLoadBoard1Ext},

	{eLAN_TEST_MFA2						,"LAN Load MPM2"			,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_TYPE_RMX4000, startLanLoadBoard2},
	{eLAN_TEST_MFA2_EXT					,"EXT LAN Load MPM2"			,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_TYPE_RMX4000, startLanLoadBoard2Ext},

	{eLAN_TEST_MFA3						,"LAN Load MPM3"			,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_TYPE_RMX4000, startLanLoadBoard3},
	{eLAN_TEST_MFA3_EXT					,"EXT LAN Load MPM3"			,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_TYPE_RMX4000, startLanLoadBoard3Ext},

	{eLAN_TEST_MFA4						,"LAN Load MPM4"			,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_TYPE_RMX4000, startLanLoadBoard4},
	{eLAN_TEST_MFA4_EXT					,"EXT LAN Load MPM4"			,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_TYPE_RMX4000, startLanLoadBoard4Ext},

	{eLAN_TEST_CNTL						,"LAN Load CNTL"			,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_ALL_SYSTEMS, startLanLoadCntl},
	//{eLAN_TEST_CNTL_EXT					,"LanLoadExt CNTL"			,ePQ			,TRUE,		TRUE,	   200,	E_BIT_OP_CHASSIS_ALL_SYSTEMS, startLanLoadCntlExt},

	{ePQ_CORE_CLOCK_TEST				,"Core Clock Test"			,ePQ			,TRUE,		TRUE,	   5,	E_BIT_OP_CHASSIS_ALL_SYSTEMS, PqCoreClockTest},
	{ePQ_MD5FILESTEST					,"SW Verification"			,ePQ			,TRUE,		TRUE,	   4,	E_BIT_OP_CHASSIS_ALL_SYSTEMS, PqMD5FilesTest},


	{0xFFFF								,"End"						,ePQ			,FALSE,		FALSE,		1,	E_BIT_OP_CHASSIS_ALL_SYSTEMS, dummyTest}

#endif

//From CPU/MPMx
	// Test ID                   		  			Test Name                     			UNIT   		Loop  	 	Quick		Time  	system type  					Func Pointer 
	// CNTL Memory Tests
#ifdef ENABLE_CODEC_STRESS_TEST
	{eCNTL_CODEC_STRESS_TEST,			        "Codec Stress Test",				eUndefined,	TRUE,		TRUE,		300,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	CodecStressTest},
#endif	
	{eCNTL_DDR_MEMORY_DATA_BUS,			"System Memory-Data Bus",			eUndefined,	TRUE,      	TRUE,		50,		E_BIT_OP_CHASSIS_ALL_SYSTEMS, 	CNTLMemoryDataBusTest},
	{eCNTL_DDR_MEMORY_ADDRESS_BUS,		"System Memory Address Bus",			eUndefined,	TRUE,      	TRUE,		50,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	CNTLMemoryAddressBusTest},
	{eCNTL_DDR_MEMORY_INTEGRITY ,			"System Memory Size",					eUndefined,	TRUE,      	TRUE,		1,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	CNTLMemorySizeTest},
	{eCNTL_DDR_MEMORY_ENERGY,				"System Local Bus & CPLD",				eUndefined,	TRUE,      	TRUE,		1,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	CNTLLocalBusCPLDTest},
	{eCNTL_CF_CREATE_DELETE_READ_WRITE,	"System Core Clock",					eUndefined,	TRUE,		TRUE,		1,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	CNTLCoreClockTest},
	
	// CNTL FPGA Tests
	{eCNTL_CF_FILE_SYSTEM_CHECK,			"FPGA DDR3 Mem",					eUndefined,	TRUE,      	TRUE,		60,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	FPGADDR3MemTest},
	{eCNTL_CF_MD5				,			"FPGA SRAM Mem",					eUndefined,	TRUE,      	TRUE,		60,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	FPGASRAMMemTest},
	{eCNTL_HARD_DISK_SMART_CHECK,			"FPGA PCIe Clock",					eUndefined,	TRUE,		TRUE,		1,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	FPGAPCIeClockTest},

	// DSP Tests
	{eDSP_DOWNLOAD_DIAG,					"DSP Download/Connectivity",		eDsp,		TRUE,		TRUE,      	30,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	DspDownloadTest},
	{eDSP_DATA_BUS_DIAG,					"DSP Memory-Data Bus",			eDsp,		TRUE,      	TRUE,      	20,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	DspMemoryDataBusTest},
	{eDSP_ADDRESS_BUS_DIAG,					"DSP Memory-Address bus",			eDsp,		TRUE,      	TRUE,      	20,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	DspMemoryAddressBusTest},
	{eDSP_CORE_CLOCK_DIAG,					"DSP Core Clock",					eDsp,		TRUE,      	TRUE,      	40,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,	DspCoreClockTest},
	
//	{eCNTL_HARD_DISK_READ_WRITE,	        "Hard disk create/del/read/write",	eUndefined,			TRUE,		TRUE,		5,		E_CHASSIS_ALL_SYSTEMS, 	CNTLHardDiskReadWriteCheck},

	//USB
//	{eCNTL_USB_READ_WRITE,	                "USB create/del/read/write",	eUndefined,			TRUE,		TRUE,		5,		E_CHASSIS_ALL_SYSTEMS, 		CNTLUSBReadWriteCheck},	

	//IPMC Tests
//	{eCNTL_IPMC_CONNECTION,					"IPMC Connection Test",	     eUndefined,			TRUE,		TRUE,		1,		E_CHASSIS_ALL_SYSTEMS, 		CNTLIPMCConntionTest},


//	{ePQ_MEMORY_DATA_BUS_DIAG,				"PQ Memory-Data Bus",					ePQ,		TRUE,      	TRUE,      1,      	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			PqMemoryDataBusTest},
//	{ePQ_MEMORY_ADDRESS_BUS_DIAG,			"PQ Memory Address Bus",				ePQ,		TRUE,      	TRUE,	   1,      	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	PqMemoryAddressBusTest},
//	{ePQ_MEMORY_ENERGY_DIAG,				"PQ Memory Energy",						ePQ,		TRUE,      	FALSE,	   1,      	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	PqMemoryEnergyTest},
//	{ePQ_MEMORY_INTEGRITY_DIAG,				"PQ Memory Size",					ePQ,		TRUE,      	FALSE,     1,      	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	PqMemoryIntegrityTest},

	// IPMC Tests
//	{eIPMC_UART_CHANNEL_DIAG,				"IPMC UART Channel",					ePQ,		TRUE,      	TRUE,      10,     	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	IpmcUartChannelTest},
//	{eIPMC_I2C_EEPROM_CHKSUM_DIAG,			"IPMC I2C EEPROM Checksum",				ePQ,		TRUE,      	TRUE,      10,     	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	IpmcI2CEepromChecksumTest},
//	{eIPMC_I2C_AD_DIAG,						"IPMC A/D",								ePQ,		TRUE,      	TRUE,      10,  	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			IpmcADTest},
//	{eIPMC_I2C_IPMB_DIAG,					"IPMC I2C IPMB",						ePQ,		TRUE,      	TRUE,      10,     	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	IpmcI2CIpmbTest},
//	{eIPMC_I2C_IPMA_DIAG,					"IPMC I2C IPMA",						ePQ,		TRUE,      	TRUE,      10,     	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	IpmcI2CIpmaTest},
//	{ePQ_IPMC_TEST_SENSORS,					"IPMC Sensors test",					ePQ,		TRUE,      	TRUE,      10,  	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			IpmcCheckTempSensors},

//	{ePQ_LOCAL_BUS_DIAG,					"System Local Bus & CPLD",					eUndefined,		TRUE,      	TRUE,      1,      	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	PqLocalBusTest},
//	{eFLASH_DATA_BUS_DIAG,					"Flash-Data Bus",						ePQ,		TRUE,      	TRUE,      2,  		E_BIT_OP_CHASSIS_ALL_SYSTEMS, 			FlashDataBusTest},
//	{eFLASH_ADDRESS_INTEGRITY_DIAG,			"Flash-Address & Integrity",			ePQ,		TRUE,      	TRUE,      5,  		E_BIT_OP_CHASSIS_ALL_SYSTEMS, 			FlashAddressIntegrityTest},
//	{ePQ_CORE_CLOCK_DIAG,					"System Core Clock",						eUndefined,		TRUE,      	TRUE,      1,      	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	PqCoreClockTest},
//	{ePQ_MD5FILESTEST,						"Software Verification",				ePQ,		TRUE,		TRUE,	   4,	   	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	PqMD5FilesTest},
	/*{ePQ_DDR_CLOCK_DIAG,					"PQ DDR Clock",							eUndefined,	TRUE,      	TRUE,      250,  	E_BIT_OP_CHASSIS_ALL_SYSTEMS, 			PqDDRClockTest},*/
	/*{ePQ_LOCAL_BUS_CLOCK_DIAG,			"PQ Local Bus Clock",					eUndefined,	TRUE,      	TRUE,      250,  	E_BIT_OP_CHASSIS_ALL_SYSTEMS, 			PqLocalBusClockTest},*/

	// FPGA Tests
//	{eFPGA_MS_KEEP_ALIVE_DIAG,				"FPGA DDR3 Mem",					eUndefined,		TRUE,      	TRUE,      	3,      E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaSwitchMemoryTest},
//	{eFPGA_SWITCH_MEMORY_DIAG,				"FPGA SRAM Mem",					eUndefined,		TRUE,      	TRUE,      	3,      E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaSwitchMemoryTest},
//	{eFPGA_PI_PCI0_CLOCK_DIAG,				"FPGA PCIe Clock",					eUndefined,		TRUE,      	TRUE,      	10,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaPIPCI0ClockTest},
	/*
	{eFPGA_MS_KEEP_ALIVE_DIAG,				"FPGA M/S Keep Alive",					ePQ,		TRUE,		TRUE,      	1,      E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaMSKeepAliveTest},
	{eFPGA_MS_PCI_CLOCK_DIAG,				"FPGA M/S PCI Clock",					ePQ,		TRUE,      	TRUE,      	1,      E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaMSSwitchPCIClockTest},
	{eFPGA_MS_LINK_TO_PI_CLOCK_DIAG,		"FPGA M/S Link to PI",					ePQ,		TRUE,      	TRUE,      	1,      E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaMSLinkToPIClockTest},
	{eFPGA_MS_FABRIC_CLOCK_DIAG,			"FPGA M/S Fabric Clock",				ePQ,		TRUE,      	TRUE,      	1,      E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaMSFabricClockTest},
	{eFPGA_SWITCH_MEMORY_DIAG,				"FPGA Switch Memory",					ePQ,		TRUE,      	TRUE,      	3,      E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaSwitchMemoryTest},
	{eFPGA_PI_KEEP_ALIVE_DIAG,				"FPGA PI Keep Alive",					ePQ,		TRUE,      	TRUE,      	10,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaPIKeepAliveTest},
	{eFPGA_PI_PCI0_CLOCK_DIAG,				"FPGA PI PCI0 Clock",					ePQ,		TRUE,      	TRUE,      	10,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaPIPCI0ClockTest},
	{eFPGA_PI_PCI1_CLOCK_DIAG,				"FPGA PI PCI1 Clock",					ePQ,		TRUE,      	TRUE,      	10,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaPIPCI1ClockTest},
	{eFPGA_TOP_LINK_CLOCK_DIAG,				"FPGA M/S TOP Link Clock",				ePQ,		TRUE,		TRUE,		1,     	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaTopLinkClockTest},
	{eFPGA_BOTTOM_LINK_CLOCK_DIAG,			"FPGA M/S and PI Bottom Clock",			ePQ,		TRUE,		TRUE,		1,     	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaBottomLinkClockTest},
	{eFPGA_RTM_PCI_CLOCK_DIAG,				"FPGA RTM PCI Clock",					ePQ,		TRUE,		TRUE,		1,     	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	FpgaRtmPciClockTest},
	*/

//	{eLAN0_INTERFACE_DIAG,					"LAN0 Interface",						ePQ,		TRUE,      	TRUE,      	18,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	Lan0InterfaceTest},
//	{eLAN_TRAFFIC_LOAD_DIAG,				"LAN Load test",						ePQ,		TRUE,      	TRUE,      	10, 	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			BoardIpTrafficTest},
//	{eLAN1_INTERFACE_DIAG,					"LAN1 Interface",						ePQ,		TRUE,      	TRUE,      	18, 	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			Lan1InterfaceTest},
//	{eLAN2_INTERFACE_DIAG,					"LAN2 Interface",						ePQ,		TRUE,      	TRUE,      	18, 	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			Lan2InterfaceTest},
//	{eLAN3_INTERFACE_DIAG,					"LAN3 Interface",						ePQ,		TRUE,      	TRUE,      	18, 	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			Lan3InterfaceTest},


	// DSP Tests
//	{eDSP_DOWNLOAD_DIAG,					"DSP Download/Connectivity",							eDsp,		TRUE,		TRUE,      	6,      E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	DspDownloadTest},
//	{eDSP_CONNECTIVITY_DIAG,				"DSP Connectivity",						eDsp,		TRUE,		FALSE,      10,    	E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	DspConnectivityTest},
//	{eDSP_BUS_LOAD_DIAG,					"Bus Load On Single DSP",				eDsp,		TRUE,      	TRUE,      	200,    E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	NULL},
//	{eDSP_DATA_BUS_DIAG,					"DSP Memory-Data Bus",					eDsp,		TRUE,      	TRUE,      	150,    E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	NULL},
//	{eDSP_ADDRESS_BUS_DIAG,					"DSP Memory-Address bus",				eDsp,		TRUE,      	TRUE,      	150,    E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	NULL},
//	{eDSP_ENERGY_DIAG,						"DSP Memory-Energy",					eDsp,		TRUE,      	TRUE,      	150,    E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	NULL},
//	{eDSP_INTEGRITY_DIAG,					"DSP Memory-Integrity",					eDsp,		TRUE,      	TRUE,      	300,    E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	NULL},
//	{eDSP_CORE_CLOCK_DIAG,					"DSP Core Clock",						eDsp,		TRUE,      	TRUE,      	30,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	NULL},

	/*{eDSP_SDRAM_CLOCK_DIAG,				"DSP SDRAM Clock",						eDsp,		TRUE,      	TRUE,      	250,  	E_BIT_OP_CHASSIS_ALL_SYSTEMS, 			NULL},*/


	// BUS Load Tests
//	{eBOARD_BUS_LOAD_DIAG,					"Bus Load On Single Board",				ePQ,		TRUE,      	TRUE,      	600,    E_BIT_OP_CHASSIS_ALL_SYSTEMS,          SingleBoardBusLoadTest},
//	{eBOARD_CONNECTIVITY_DIAG,				"Board Connectivity (Needs 2 boards)",	ePQ,		TRUE,     	FALSE,      10,     E_BIT_OP_CHASSIS_TYPE_RMX2000,          BoardConnectivityTest},
//	{eBOARD_TO_BOARD_BUS_LOAD_DIAG,			"Between Boards load (Needs 2 boards)",	ePQ,		TRUE,      	TRUE,      	600,    E_BIT_OP_CHASSIS_TYPE_RMX2000,          BoardToBoardBusLoadTest},
//	{eSHOVAL_UNICAST_LOAD,					"Shoval Unicast load (each board)",		ePQ,		TRUE,		TRUE,		600,	E_BIT_OP_CHASSIS_TYPE_RMX4000,   	   	ShovalUnicastLoad},
//	{eBOARD_HOMOLOGATION_TEST,				"Board homologation test",				ePQ,		FALSE,     	FALSE,     	0,  	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			HomologationTest},



	// RTM ISDN Tests
//	{eRTM_DSP_DOWNLOAD_DIAG,				"RTM DSP Download",						eRtm,		TRUE,      	TRUE,      	23,		E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
	{eRTM_DSP_DATA_BUS_DIAG,				"RTM DSP Memory-Data Bus",				eRtm,		TRUE,      	TRUE,      	45,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
	{eRTM_DSP_ADDRESS_BUS_DIAG,				"RTM DSP Memory-Address bus",			eRtm,		TRUE,      	TRUE,      	45,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
	{eRTM_DSP_ENERGY_DIAG,					"RTM DSP Memory-Energy",				eRtm,		TRUE,      	TRUE,      	45,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
	{eRTM_DSP_INTEGRITY_DIAG,				"RTM DSP Memory-Integrity",				eRtm,		TRUE,      	TRUE,      	2000,   E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
	{eRTM_DSP_CLOCKS_DIAG,					"RTM DSP Core Clock",						eRtm,		TRUE,      	TRUE,      	45,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
//	{eRTM_DSP_CPLD_DIAG,					"RTM DSP CPLD",							eRtm,		TRUE,      	TRUE,      	10,  	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			RtmDspDiagTest},
//	{eRTM_DSP_TDM_DIAG,						"RTM DSP TDM",							eRtm,		TRUE,      	TRUE,      	70,  	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			RtmDspDiagTest},
//	{eRTM_DSP_E1_INF_DIAG,					"RTM DSP E1 Interface",					eRtm,		TRUE,      	TRUE,      	180,  	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			RtmDspDiagTest},
//	{eRTM_DSP_T1_INF_DIAG,					"RTM DSP T1 Interface",					eRtm,		TRUE,      	TRUE,     	600,  	E_BIT_OP_CHASSIS_ALL_SYSTEMS,			RtmDspDiagTest},
//	{eRTM_DSP_T1_FALC1_DIAG,				"RTM DSP T1 FALC1 Diag",				eRtm,		TRUE,      	TRUE,      	700,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
//	{eRTM_DSP_T1_FALC2_DIAG,				"RTM DSP T1 FALC2 Diag",				eRtm,		TRUE,      	TRUE,      	700,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
	{eRTM_DSP_T1_FALC3_DIAG,				"RTM DSP T1 FALC3 Diag",				eRtm,		TRUE,      	TRUE,      	45,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
//	{eRTM_DSP_E1_FALC1_DIAG,				"RTM DSP E1 FALC1 Diag",				eRtm,		TRUE,      	TRUE,      	700,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
//	{eRTM_DSP_E1_FALC2_DIAG,				"RTM DSP E1 FALC2 Diag",				eRtm,		TRUE,      	TRUE,      	700,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
	{eRTM_DSP_E1_FALC3_DIAG,				"RTM DSP E1 FALC3 Diag",				eRtm,		TRUE,      	TRUE,      	45,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
//	{eRTM_DSP_TDM_FALC1_DIAG,				"RTM TDM FALC1 Diag",					eRtm,		TRUE,      	TRUE,      	70,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
//	{eRTM_DSP_TDM_FALC2_DIAG,				"RTM TDM FALC2 Diag",					eRtm,		TRUE,      	TRUE,      	70,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
//	{eRTM_DSP_TDM_FALC3_DIAG,				"RTM TDM FALC3 Diag",					eRtm,		TRUE,      	TRUE,      	70,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},
//	{eRTM_DSP_LAST_TEST,					"RTM TDM Load Test",					eRtm,		TRUE,      	TRUE,      	60,     E_BIT_OP_CHASSIS_ALL_SYSTEMS,         	RtmDspDiagTest},



	/*{eRTM_DSP_FALC_DIAG                 ,"RTM DSP FALC",eRtm	,TRUE,      TRUE,      250,  NULL},*/
	/*
	{eMASSDSP_CORE_CLOCK_DIAG           ,"All DSP Core Clock"             ,eAllDsps   	,TRUE,      TRUE,      3,  NULL},
	{eMASSDSP_DATA_BUS_DIAG             ,"All DSP Memory-Data Bus"        ,eAllDsps    	,TRUE,      TRUE,      1,  NULL},
	{eMASSDSP_ADDRESS_BUS_DIAG          ,"All DSP Memory-Address bus"     ,eAllDsps    	,TRUE,      TRUE,      1,  NULL},
	{eMASSDSP_ENERGY_DIAG               ,"All DSP Memory-Energy"          ,eAllDsps    	,TRUE,      TRUE,      1,  NULL},
	{eMASSDSP_INTEGRITY_DIAG            ,"All DSP Memory-Integrity"       ,eAllDsps    	,TRUE,      TRUE,      130,  NULL},
	{eMASSDSP_DOWNLOAD_DIAG             ,"All DSP Download"               ,eAllDsps     ,TRUE,      TRUE,      1,  DspDownloadTest},
	{eMASSDSP_CONNECTIVITY_DIAG         ,"DSP Connectivity"               ,eAllDsps		,TRUE,      FALSE,      10,  DspConnectivityTest},
*/
	// Last - Invalid Test
	{0xffff,								"Empty",								0,			0,			0,			0,		0,								0}


};

//e_TypeOfTest	currentTestType;

TESTS_DATABASE			TestsDatabase;
extern const char 		*testTypeStr[];
extern UINT32 			EmaLoopCounter;
extern void 			SendInternalStartTestMessage(UINT32 ulSlotId);
extern SIM_ERROR_BUFFER	errorBuffer;
extern UINT32			startTestValue;
extern t_TcpConnParams SwitchDiagTcpConnection[eMaxTcpConnections];
//extern INT32			l_SwitchTestsQueue;
UINT32 					wasTestAskedToStop[MAX_SLOT_NUM];

// Enter diag mode indication message
static TStructToStr atEnterDiagModeIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
};

// Test list indication message
static TStructToStr atTestListIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_string,		   1,   		   DGNS_TEST_NAME_MAX_LEN,   	 "TestName"},
	{e_unsignedLong,   1,              4,        "TestID"},
	{e_unsignedLong,   1,              4,        "TestOn"},
	{e_unsignedLong,   1,              4,        "Loop"},
	{e_unsignedLong,   1,              4,        "Quick"},
	{e_unsignedLong,   1,              4,        "EstimatedTime"},
};

// Start test indication message
static TStructToStr atStartTestIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
};

// Stop test indication message
static TStructToStr atStopTestIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
};

// Test status indication message
static TStructToStr atTestStatusIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "Loop"},
	{e_unsignedLong,   1,              4,        "StopOnFail"},
	{e_unsignedLong,   1,              4,        "Duration"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_string,		   1,   		   DGNS_TEST_NAME_MAX_LEN,   	 "TestName"},
	{e_unsignedLong,   1,              4,        "TestID"},
	{e_signedLong,     1,              4,        "UnitID"},
	{e_unsignedLong,   1,              4,        "Loop"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_unsignedLong,   1,              4,        "Pass"},
	{e_unsignedLong,   1,              4,        "Failed"},
	{e_unsignedLong,   1,              4,        "Duration"},
	{e_unsignedLong,   1,              4,        "Quick"},
};

// Unit list indication message
static TStructToStr atUnitStatusListIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "UnitID"},
	{e_unsignedLong,   1,              4,        "Type"},
	{e_signedLong,     1,              4,        "Status"},
};

// Error list indication message
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

// Control response message
static TStructToStr atControlResponseMsgArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
};

/*======================================================================*/
/* FUNCTION:		BuildEnterDiagModeIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)				*/
/* PURPOSE :		Test list indication	 							*/
/* PARAMETERS:		msgId, slotId										*/
/* RETURN VALUE:	void												*/
/* LIMITATION:															*/
/*======================================================================*/
void BuildEnterDiagModeIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr			*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	 		*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	printf("BuildEnterDiagModeIndMsg\n");
	// Allocate message	pointer
	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atEnterDiagModeIndSpecArray) +
				   sizeof(TEmaEnterTestModeInd);

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildEnterDiagModeInd): sizeof EmaIndGeneralDescHeader is: %d\n, sizeof atEnterDiagModeIndSpecArray is: %d\n, sizeof EmaTestListInd is: %d\n",
   	 sizeof(TSpecGnrlHdr),
   	 sizeof(atEnterDiagModeIndSpecArray),
   	 sizeof(TEmaEnterTestModeInd));

	if(NULL == (pMsg = currentMsgPointer = (UINT32*)malloc( unIndBufSize ))) return;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset = sizeof(TSpecGnrlHdr) + sizeof(atEnterDiagModeIndSpecArray);

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (message begin value)", *currentMsgPointer);
   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"ptEmaIndGeneralDescHdr->ulMsgOffset (in bytes) = %d", ptEmaIndGeneralDescHdr->ulMsgOffset);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atEnterDiagModeIndSpecArray,sizeof(atEnterDiagModeIndSpecArray));

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atEnterDiagModeIndSpecArray))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Diag mode Indication Message
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= EMA_ENTER_DIAG_MODE_IND;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);

	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildEnterDiagModeInd): sending message: %s\n", currentMsgPointer);

	//StringWrapper( eSwitchDiagServer, (UINT32)pMsg);
	if ( TCPSendData(TcpConnection[eSwitchDiagClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
	   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildEnterDiagModeInd): problem in sending data...\n");
	    TcpConnection[eSwitchDiagServer].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eSwitchDiagServer].ul_PrevConnState = CONNECTED;
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}

	if ((void *)pMsg != NULL)
		  free((void *)pMsg);
}


/*======================================================================*/
/* FUNCTION:		BuildTestListIndMsg()								*/
/* PURPOSE :		Test list indication	 							*/
/* PARAMETERS:		board number										*/
/* RETURN VALUE:	void												*/
/* LIMITATION:															*/
/*======================================================================*/
extern boardConnectInfo brdConnList[AMOS_BOARD_COUNT];	//connection to baraks
void BuildTestListIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	EmaIndGeneralDescHeader	*ptEmaIndGeneralDescHdr;
	EmaIndHeader	 		*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TTestList* 				ptTestList;

	//whether to show the md5checksum test or not
	FILE*					pFile;
	UINT32					isMd5FilePresent,reportedArrayIndex;
	printf("BuildTestListIndMsg\n");
	// Allocate message	pointer
	unIndBufSize = sizeof(EmaIndGeneralDescHeader) +
				   sizeof(atTestListIndSpecArray) +
				   sizeof(EmaTestListInd);

	printf("asked test list\n");

	if(NULL == (pMsg = currentMsgPointer = (UINT32*)malloc( unIndBufSize ))) return;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (EmaIndGeneralDescHeader*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->msgOffset = sizeof(EmaIndGeneralDescHeader) + sizeof(atTestListIndSpecArray);

//   	SimBoardPrint(TCP_CONNECTION_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
//	"currentMsgPointer value = %d (message begin value)", *currentMsgPointer);
//   	SimBoardPrint(TCP_CONNECTION_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
//	"ptEmaIndGeneralDescHdr->msgOffset (in bytes) = %d", ptEmaIndGeneralDescHdr->msgOffset);

	currentMsgPointer += (sizeof(EmaIndGeneralDescHeader))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atTestListIndSpecArray,sizeof(atTestListIndSpecArray));

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atTestListIndSpecArray))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Test List Indication Message
	// Fill Test List Message Header
	ptEmaIndHdr 					= (EmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= EMA_GET_TEST_LIST_IND;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(EmaIndHeader)/4);

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);

	// Fill test list data
	ptTestList = (TTestList*)currentMsgPointer;

	ptTestList->ulNumOfElem = 0;
	ptTestList->ulNumOfElemFields = 6;
	/*	check for md5checksum file presence	*/
	pFile = fopen(MD5FILENAME, "r");
	if (pFile == NULL)
	{
		isMd5FilePresent = 0;
	}
	else
	{
		isMd5FilePresent = 1;
		fclose(pFile);
	}

    reportedArrayIndex = 0;
	printf("started combining tests \n");
    for(i = 0 ; (i < (sizeof(currentSystemTests)/sizeof(currentSystemTests[0]))) && (currentSystemTests[i].TestId != 0xffff) ; i++)
    {
        if (ePQ_MD5FILESTEST == currentSystemTests[i].TestId && EMB_FALSE == isMd5FilePresent)
        {
            // no support for md5sum test, so skip it
            printf("Skipping %s...\n", currentSystemTests[i].TestName);
            continue;
        }
        switch(chassisType)
        {
			case eChassisType_Ninja:
				{
					if(0 == (currentSystemTests[i].SystemType & E_BIT_OP_CHASSIS_TYPE_NINJA) )
					{
						printf("Skipping %s...\n", currentSystemTests[i].TestName);
						continue;
					}

					if((CNTL_SLOT_ID == ulSlotID) && (eUndefined != currentSystemTests[i].TestOn))
					{
						printf("Skipping %s...\n", currentSystemTests[i].TestName);
						continue;
					}

					if((DSP_CARD_SLOT_ID_0 <= ulSlotID) && (DSP_CARD_SLOT_ID_2 >= ulSlotID) && (eDsp != currentSystemTests[i].TestOn))
					{
						printf("Skipping %s...\n", currentSystemTests[i].TestName);
						continue;
					}
                    
					if((ISDN_CARD_SLOT_ID == ulSlotID) && (eRtm != currentSystemTests[i].TestOn))
					{
						printf("Skipping %s...\n", currentSystemTests[i].TestName);
						continue;
					}
				}
				break;
			default:
				continue;
        };

        PrintDgnsTestInfo(&(currentSystemTests[i]));
        
	
        //strncpy(ptTestList->tTestData[reportedArrayIndex].testName, currentSystemTests[i].TestName, sizeof(ptTestList->tTestData[reportedArrayIndex].testName));
		strcpy(ptTestList->tTestData[reportedArrayIndex].testName, currentSystemTests[i].TestName);
		ptTestList->tTestData[reportedArrayIndex].unTestID = currentSystemTests[i].TestId;
		ptTestList->tTestData[reportedArrayIndex].unQuick = currentSystemTests[i].isQuickVersion;
		ptTestList->tTestData[reportedArrayIndex].unLoop = currentSystemTests[i].canBeLooped;
		ptTestList->tTestData[reportedArrayIndex].unEstimateTime = currentSystemTests[i].esimatedRunTime;
		ptTestList->tTestData[reportedArrayIndex].unTestOn = currentSystemTests[i].TestOn;

        ptTestList->ulNumOfElem++;
		reportedArrayIndex++;
    }

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildTestListInd): sending message on socket: %d, message: %s\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildTestListInd): sending message: %s\n", currentMsgPointer);
	// Send Message Switch Server ...
	if ( TCPSendData( TcpConnection[eSwitchDiagClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildTestListInd): problem in sending data...\n");
	    TcpConnection[eSwitchDiagClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eSwitchDiagClient].ul_PrevConnState = CONNECTED;

	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}

/*======================================================================*/
/* FUNCTION:		BuildStartTestIndMsg(UINT32 ulMsgId, UINT32 ulSlotID, UINT32 ulTestNumber)	*/
/* PURPOSE :		Start test indication	 							*/
/* PARAMETERS:		board number										*/
/* RETURN VALUE:	void												*/
/* LIMITATION:															*/
/*======================================================================*/
void BuildStartTestIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	EmaIndGeneralDescHeader	*ptEmaIndGeneralDescHdr;
	EmaIndHeader	 		*ptEmaIndHdr;
//	StartTestData			*ptStartTestData;
	UINT32					unIndBufSize;
	UINT32					i;
	printf("BuildStartTestIndMsg\n");
   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildStartTestInd): sizeof EmaIndGeneralDescHeader is: %d\n, sizeof atStartTestIndSpecArray is: %d\n, sizeof EmaStartTestInd is: %d\n",
   	 sizeof(EmaIndGeneralDescHeader),
   	 sizeof(atStartTestIndSpecArray),
   	 sizeof(TEmaStartTestInd));

	// Allocate message	pointer
	unIndBufSize = sizeof(EmaIndGeneralDescHeader) +
				   sizeof(atStartTestIndSpecArray) +
				   sizeof(TEmaStartTestInd);

	if(NULL == (pMsg = currentMsgPointer = (UINT32*)malloc( unIndBufSize ))) return;

	// Fill General Header
	ptEmaIndGeneralDescHdr = (EmaIndGeneralDescHeader*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->msgOffset = sizeof(EmaIndGeneralDescHeader) + sizeof(atStartTestIndSpecArray);

	currentMsgPointer += (sizeof(EmaIndGeneralDescHeader))/4;

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer, (void*)atStartTestIndSpecArray, sizeof(atStartTestIndSpecArray));

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atStartTestIndSpecArray))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Start Test Indication Message
	// Fill Start Test Message Header
	ptEmaIndHdr 					= (EmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= EMA_START_TEST_IND;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(EmaIndHeader))/4;

	// Fill Start Test data
//	ptStartTestData = (StartTestData*)currentMsgPointer;
//
//	ptStartTestData->testNumber = ulTestNumber;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildStartTestInd): sending message on socket: %d, message: %s\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildStartTestInd): sending message: %s\n", currentMsgPointer);

	// Send Message Switch Server ...
	if ( TCPSendData( TcpConnection[eSwitchDiagClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{

		//yosi fix return value socket
		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildStartTestInd): problem in sending data...\n");
	    TcpConnection[eSwitchDiagClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eSwitchDiagClient].ul_PrevConnState = CONNECTED;

	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}

/*======================================================================*/
/* FUNCTION:		BuildGetTestStatusIndMsg(UINT32 ulMsgId, UINT32 ulSlotID, UINT32 ulTestNumber)	*/
/* PURPOSE :		Start test indication	 							*/
/* PARAMETERS:		board number										*/
/* RETURN VALUE:	void												*/
/* LIMITATION:															*/
/*======================================================================*/
void BuildGetTestStatusIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	EmaIndGeneralDescHeader	*ptEmaIndGeneralDescHdr;
	EmaIndHeader	 		*ptEmaIndHdr;
	UINT32					unIndBufSize;
	TTestStatusList		 	*ptTestStatusList;
	TTestGeneralParams		*ptTestGeneralParams;
	UINT32					i,ulNumOfElem;
	struct timeval 			tTimer_vals;
	dgnsCtrlTestInfo *		testInfoStruct;
	dgnsTestInfo *		testListInfo;
	printf("BuildGetTestStatusIndMsg\n");
   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildGetTestStatusIndMsg): sizeof EmaIndGeneralDescHeader is: %d\n, sizeof atTestStatusIndSpecArray is: %d\n, sizeof EmaGetTestStatusInd is: %d\n",
   	 sizeof(TSpecGnrlHdr),
   	 sizeof(atTestStatusIndSpecArray),
   	 sizeof(TEmaGetTestStatusInd));

	// Allocate message	pointer
	unIndBufSize = sizeof(EmaIndGeneralDescHeader) +
				   sizeof(atTestStatusIndSpecArray) +
				   sizeof(TEmaGetTestStatusInd);

	if(NULL == (pMsg = currentMsgPointer = (UINT32*)malloc( unIndBufSize ))) return;

	// Fill General Header
	ptEmaIndGeneralDescHdr = (EmaIndGeneralDescHeader*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->msgOffset = sizeof(EmaIndGeneralDescHeader) + sizeof(atTestStatusIndSpecArray);

	currentMsgPointer += (sizeof(EmaIndGeneralDescHeader))/4;

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer, (void*)atTestStatusIndSpecArray, sizeof(atTestStatusIndSpecArray));

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atTestStatusIndSpecArray))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Start Test Indication Message
	// Fill Start Test Message Header
	ptEmaIndHdr 					= (EmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= EMA_GET_TEST_STATUS_IND;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(EmaIndHeader))/4;

	// Fill Test status data
	// Fill general params
	ptTestGeneralParams = (TTestGeneralParams*)currentMsgPointer;

	ptTestGeneralParams->unNumOfReqLoops = diagCurrentSession[ulSlotID].numOfLoops;
	ptTestGeneralParams->unStopOnFail = diagCurrentSession[ulSlotID].stopOnFail;
	//we check whether we are in progress currently,and if yes -
	// update the time accordingly.
	if (diagCurrentSession[ulSlotID].hasStarted == 1)
	{
		gettimeofday(&tTimer_vals,NULL);
		ptTestGeneralParams->unDuration = (UINT32)tTimer_vals.tv_sec - diagCurrentSession[ulSlotID].testSessionStartTime;
	}
	else
	{	//if it is not in progress currently,it means we already finished,and
		//can use the time inside the structure
		ptTestGeneralParams->unDuration = diagCurrentSession[ulSlotID].testSessionDurationTime;
	}
	// Fill all tests params
	currentMsgPointer += (sizeof(TTestGeneralParams))/4;
	ptTestStatusList = (TTestStatusList*)currentMsgPointer;



	ptTestStatusList->ulNumOfElemFields = 9;


	ulNumOfElem = 0;

	for(i = 0; (i < MAX_NUM_OF_REPORTED_TESTS) && (dgnsGetTestFromList(ulSlotID, i,&testInfoStruct)>0); i++)
	{
		testListInfo = testIdToTest(testInfoStruct->testId,currentSystemTests);
		strcpy(ptTestStatusList->tTestStatusData[i].testName, testListInfo->TestName);
		ptTestStatusList->tTestStatusData[i].unTestID = testInfoStruct->testId;
		ptTestStatusList->tTestStatusData[i].unUnitID = testInfoStruct->unitOnSlot;
		ptTestStatusList->tTestStatusData[i].unLoopNum = testInfoStruct->loopsDone;
		ptTestStatusList->tTestStatusData[i].unTestState = testInfoStruct->isActiveNow;
		ptTestStatusList->tTestStatusData[i].unNumOfSuccesses = testInfoStruct->successTests;
		ptTestStatusList->tTestStatusData[i].unNumOfFailures = testInfoStruct->failTests;



		if (getTestType(testInfoStruct) == e_typeDSPtest)
		{
			//dsp is the same whether its active or not.
			ptTestStatusList->tTestStatusData[i].unDuration = testInfoStruct->duration;
		}
		else if ((testInfoStruct->isActiveNow == 0) || (testInfoStruct->isActiveNow == 2))
		{
			ptTestStatusList->tTestStatusData[i].unDuration = testInfoStruct->duration;
		}
		else //if (testInfoStruct->isActiveNow == 1) && not dsp test
		{
			gettimeofday(&tTimer_vals,NULL) ;
			// Calculate the time that passed since the beginning of the test
			ptTestStatusList->tTestStatusData[i].unDuration = (UINT32)tTimer_vals.tv_sec - testInfoStruct->testStartTime;
		}

		ptTestStatusList->tTestStatusData[i].unQuick = NO;  //P.K - why is this here???
		ulNumOfElem++;
	}
	printf("prepared %d tests to report\n",ulNumOfElem);
	ptTestStatusList->ulNumOfElem = ulNumOfElem;



	// Send Message Switch Server ...
	if ( TCPSendData( TcpConnection[eSwitchDiagClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildStartTestInd): problem in sending data...\n");
	    TcpConnection[eSwitchDiagClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eSwitchDiagClient].ul_PrevConnState = CONNECTED;

	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}

/*======================================================================*/
/* FUNCTION:		BuildStopTestIndMsg(UINT32 ulMsgId, UINT32 ulSlotID, UINT32 ulTestNumber)	*/
/* PURPOSE :		Start test indication	 							*/
/* PARAMETERS:		board number										*/
/* RETURN VALUE:	void												*/
/* LIMITATION:															*/
/*======================================================================*/
void BuildStopTestIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	EmaIndGeneralDescHeader	*ptEmaIndGeneralDescHdr;
	EmaIndHeader	 		*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	struct timeval 			tTimer_vals;
	printf("BuildStopTestIndMsg\n");
	/* This flag will tell the testing thread,that it was asked to stop with the tests */
	wasTestAskedToStop[ulSlotID] = 1;
   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildStopTestInd): sizeof EmaIndGeneralDescHeader is: %d\n, sizeof atStopTestIndSpecArray is: %d\n, sizeof EmaStopTestInd is: %d\n",
   	 sizeof(EmaIndGeneralDescHeader),
   	 sizeof(atStopTestIndSpecArray),
   	 sizeof(EmaStopTestInd));

	// Allocate message	pointer
	unIndBufSize = sizeof(EmaIndGeneralDescHeader) +
				   sizeof(atStopTestIndSpecArray) +
				   sizeof(EmaStopTestInd);

	if(NULL == (pMsg = currentMsgPointer = (UINT32*)malloc( unIndBufSize ))) return;

	// Fill General Header
	ptEmaIndGeneralDescHdr = (EmaIndGeneralDescHeader*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->msgOffset = sizeof(EmaIndGeneralDescHeader) + sizeof(atStopTestIndSpecArray);

	currentMsgPointer += (sizeof(EmaIndGeneralDescHeader))/4;

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer, (void*)atStopTestIndSpecArray, sizeof(atStopTestIndSpecArray));

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atStopTestIndSpecArray))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Start Test Indication Message
	// Fill Start Test Message Header
	ptEmaIndHdr 					= (EmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= EMA_STOP_TEST_IND;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader))/4;

	// Fill Start Test data
//	ptStopTestData = (StopTestData*)currentMsgPointer;
//
//	ptStopTestData->testNumber = ulTestNumber;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildStartTestInd): sending message on socket: %d, message: %s\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildTestListInd): sending message: %s\n", currentMsgPointer);

	// Send Message Switch Server ...
	if ( TCPSendData( TcpConnection[eSwitchDiagClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildStartTestInd): problem in sending data...\n");
	    TcpConnection[eSwitchDiagClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eSwitchDiagClient].ul_PrevConnState = CONNECTED;

	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}


/*======================================================================*/
/* FUNCTION:		BuildTestListIndMsg()								*/
/* PURPOSE :		Test list indication	 							*/
/* PARAMETERS:		board number										*/
/* RETURN VALUE:	void												*/
/* LIMITATION:															*/
/*======================================================================*/
int RtmAlive = 0; //TODO ydong
int PQAlive = 0;
void BuildUnitListIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	EmaIndGeneralDescHeader	*ptEmaIndGeneralDescHdr;
	EmaIndHeader	 		*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TUnitList* 				ptUnitList;
	TUnitStatusData 		*tUnitStatusData;
	UINT32					unitElementsNum,effectiveUnitElementNum,unitListSize;
	UINT32					unitListIndex;
	printf("BuildUnitListIndMsg\n");

	if(ISDN_CARD_SLOT_ID == ulSlotID)
    {
            unitElementsNum = MAX_DSPUNIT_ON_ISDN_CARD_NUM;
    }
	else
    {
        	unitElementsNum = PQUnitSlot - 1; // only dsp unit
        	if (PQAlive) //If there is PQ - add one more slot for its info
        		unitElementsNum++;	
        	if (RtmAlive) //If there is RTM - add one more slot for its info
        		unitElementsNum++;        
    }

	
	unitListSize = unitElementsNum * sizeof(TUnitStatusData);

	// Allocate message	pointer
	unIndBufSize = sizeof(EmaIndGeneralDescHeader) +
				   sizeof(atUnitStatusListIndSpecArray) +
				   sizeof(EmaGetUnitsStatusInd)+
				   unitListSize;
	/*
   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildUnitStatusListInd): sizeof EmaIndGeneralDescHeader is: %d\n, sizeof atUnitListIndSpecArray is: %d\n, sizeof EmaGetUnitsStatusInd is: %d\n",
   	 sizeof(TSpecGnrlHdr),
   	 sizeof(atUnitStatusListIndSpecArray),
   	 sizeof(EmaGetUnitsStatusInd));
   	*/
  // 	printf("!!!!!!!!!!!unIndBufSize to be malloced= %d+%d+%d+%d= %d bytes\n",sizeof(EmaIndGeneralDescHeader),
   	//		sizeof(atUnitStatusListIndSpecArray),sizeof(EmaGetUnitsStatusInd),unitListSize,unIndBufSize);

	if(NULL == (pMsg = currentMsgPointer = (UINT32*)malloc( unIndBufSize ))) return;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (EmaIndGeneralDescHeader*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->msgOffset = sizeof(EmaIndGeneralDescHeader) + sizeof(atUnitStatusListIndSpecArray);

//   	MfaBoardPrint(TCP_CONNECTION_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
//	"currentMsgPointer value = %d (message begin value)", *currentMsgPointer);
//   	MfaBoardPrint(TCP_CONNECTION_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
//	"ptEmaIndGeneralDescHdr->msgOffset (in bytes) = %d", ptEmaIndGeneralDescHdr->msgOffset);

	currentMsgPointer += (sizeof(EmaIndGeneralDescHeader))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atUnitStatusListIndSpecArray,sizeof(atUnitStatusListIndSpecArray));

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atUnitStatusListIndSpecArray))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Test List Indication Message
	// Fill Test List Message Header
	ptEmaIndHdr 					= (EmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= EMA_GET_UNITS_STATE_IND;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);

	// Fill unit list data
	ptUnitList = (TUnitList*)currentMsgPointer;


	currentMsgPointer += (sizeof(TUnitList)/4);
	tUnitStatusData = (TUnitStatusData*)currentMsgPointer;

	/*	unitListIndex is instead of I, because it won't always be incremented.
	 * it is possible, that we had i that described dsp that is not on board.
	 * This can only happen in MPM+H board	case */
	effectiveUnitElementNum = 0;
	printf("**combining unit list. \n");
	for(i = 0 , unitListIndex = 0; i < unitElementsNum; i++,unitListIndex++)
	{
		//valid dsp
		if(ulSlotID >= DSP_CARD_SLOT_ID_0 && ulSlotID <= DSP_CARD_SLOT_ID_2 && FALSE == isValidDsp(ulSlotID, (i + 1)))
			continue;

        //valid ISDN
		if(ulSlotID == ISDN_CARD_SLOT_ID && FALSE == isValidRtmIsdnDSP(ulSlotID, (i + 1)))
			continue;
		
		tUnitStatusData[unitListIndex].unUnitID = (i + 1);

		if ((i == PQUnitSlot - 1) && (PQAlive)) // PQ . DSP_NUM + 1
		{
			tUnitStatusData[unitListIndex].unUnitType = ePQ;
			tUnitStatusData[unitListIndex].unUnitStatus = errorOccured[ulSlotID][i + 1];
		}
		else if ((i == RTMUnitSlot - 1) && (RtmAlive) && (RtmCardType == 1)) 		// // PQ . DSP_NUM + 2
		{
			tUnitStatusData[unitListIndex].unUnitType = eRtm;
			if (cm_rtm_receive)
					tUnitStatusData[unitListIndex].unUnitStatus = errorOccured[ulSlotID][RTMUnitSlot];
				else
					tUnitStatusData[unitListIndex].unUnitStatus = STATUS_FAIL;
		}
		else if (i <= num_OF_DSPS_IN_BOARD) 					// DSP
		{
			if(ISDN_CARD_SLOT_ID == ulSlotID)
			    tUnitStatusData[unitListIndex].unUnitType = eRtm;
			else
			    tUnitStatusData[unitListIndex].unUnitType = eDsp;
                   
			tUnitStatusData[unitListIndex].unUnitStatus = errorOccured[ulSlotID][i + 1];
#if 0
			if (currentBoardType == eMfa_Barak9)	//in case the dsp does not exist
				if (isDspExistantOnMPMHBoard(i + 1) == 0)
				{
					//dsp does not exist
					unitListIndex --;
					effectiveUnitElementNum--;		//decrease the unit count
				}
#endif
		}
		else
		{
			//undefined
			tUnitStatusData[unitListIndex].unUnitType = 0xff;
			tUnitStatusData[unitListIndex].unUnitStatus = -1;
		}
		effectiveUnitElementNum++;
	}

	ptUnitList->ulNumOfElem = effectiveUnitElementNum;
	ptUnitList->ulNumOfElemFields = 3;


	/*
   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildUnitStatusListInd): sending message on socket: %d, message: %s\n",
   	TcpConnection[eSwitchDiagClient].s, currentMsgPointer);

	for(i = 0; i < (unIndBufSize / 4); i++)
	{
	    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildUnitStatusListInd): sending message: %s\n", currentMsgPointer);*/
	// Send Message Switch Server ...
	if ( TCPSendData( TcpConnection[eSwitchDiagClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{

		//   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildTestListInd): problem in sending data...\n");
	    TcpConnection[eSwitchDiagClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eSwitchDiagClient].ul_PrevConnState = CONNECTED;

	    printf("Failed sending!\n");
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	// Free memory
//	printf("Finished sending!\n");
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}

/*======================================================================*/
/* FUNCTION:		ParseStartTestReq()									*/
/* PURPOSE :								 							*/
/* PARAMETERS:		board number										*/
/* RETURN VALUE:	void												*/
/* LIMITATION:															*/
/*======================================================================*/
void ParseStartTestReq(UINT32* pMsg, UINT32 ulSlotId)
{
	UINT32	ulNumOfElem;
	UINT32	ulNumOfElemFields;
	UINT32	unTestId,unUnitOnSlot;
	UINT32*	currentPtr;
	UINT32	i;
	TEmaStartTestReq	*ptEmaStartTestReqMsg;
	pthread_t threadTesterManager;
	
	if (diagCurrentSession[ulSlotId].hasStarted == 1)
	{
		printf("Won't start new session, previous is in progress. SlotId: %d\n\n", ulSlotId);
		return;
	}

	printf("ParseStartTestReq\n");
	ptEmaStartTestReqMsg = (TEmaStartTestReq*)pMsg;
	currentPtr = (UINT32*)pMsg;

	dgnsClearTests(ulSlotId); //Erase all old tests from memory
	/* This one turns to 1,if i was asked to stop a test in progress */
	wasTestAskedToStop[ulSlotId] = 0;

	//take care of time:
	//gettimeofday(&tTimer_vals,NULL) ;
	//diagCurrentSession.testSessionStartTime =
	diagCurrentSession[ulSlotId].hasStarted = 1;
	diagCurrentSession[ulSlotId].numOfLoops = ptEmaStartTestReqMsg->unNumOfLoops;
	if (!diagCurrentSession[ulSlotId].numOfLoops)
		diagCurrentSession[ulSlotId].numOfLoops++; //if i was asked 0 loops,make it 1

	diagCurrentSession[ulSlotId].stopOnFail = ptEmaStartTestReqMsg->unStopOnFail;
	diagCurrentSession[ulSlotId].isQuickSession = 0;  //QUICK SESSION NOT DEFINED BY PROTOCOL!!!


	ulNumOfElem = ptEmaStartTestReqMsg->tTStartTestList.ulNumOfElem;
	ulNumOfElemFields = ptEmaStartTestReqMsg->tTStartTestList.ulNumOfElemFields;


	currentPtr += (sizeof(EmaReqHeader)/4);
	currentPtr += 2; // unNumOfReqLoops, unStopOnFail
	currentPtr += 2; // ulNumOfElem, ulNumOfElemFields

	printf("Received request for %d tests\n",ulNumOfElem);
	// Parse list of tests
	for(i = 0; i < ulNumOfElem; i++)
	{
		unTestId = *currentPtr;
		unUnitOnSlot = (INT32)*(currentPtr + 1);

		//Added test to tests list,with unit on slot as second parameter
	/*	if (unUnitOnSlot == eAllDsps)
		{
			for (k = 1; k < num_OF_DSPS_IN_BOARD + 1; k++)
				//i set the test id of new test from MASS-*test to *test
				//only if the dsp exists on board
				if (loadedDSPInformation[k] != eDSPNotExistantOnBoard)
					dgnsTestAddTest(unTestId - (eMASSDSP_CORE_CLOCK_DIAG - eDSP_CORE_CLOCK_DIAG),k);
		}
		else*/
		printf("inserting into queue test %d on slotId %d, unitid %d\n",unTestId, ulSlotId, unUnitOnSlot);
		dgnsTestAddTest(unTestId, ulSlotId, unUnitOnSlot);

		currentPtr += 2; // unTestID, unUnitID
	}

	// Send message to card thread
	//SendInternalStartTestMessage(ulSlotId);

	UINT32 * pulSlotId = (UINT32 *)malloc(sizeof(UINT32));
	if(NULL != pulSlotId)
	{
		*pulSlotId = ulSlotId;
		pthread_create(&threadTesterManager,NULL,StartTestHandling, pulSlotId);
		pthread_detach(threadTesterManager);
	}
}


/*======================================================================*/
/* FUNCTION:		BuildErrorListIndMsg()								*/
/* PURPOSE :		Error list indication	 							*/
/* PARAMETERS:		board number										*/
/* RETURN VALUE:	void												*/
/* LIMITATION:															*/
/*======================================================================*/
void BuildErrorListIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	EmaIndGeneralDescHeader	*ptEmaIndGeneralDescHdr;
	EmaIndHeader	 		*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TErrorList* 			ptErrorList;
	errList* 				errorInfo;
	printf("BuildErrorListIndMsg\n");
	// Allocate message	pointer
	unIndBufSize = sizeof(EmaIndGeneralDescHeader) +
				   sizeof(atErrorListIndSpecArray) +
				   sizeof(EmaErrorListInd);

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   	"(BuildErrorListInd): sizeof EmaIndGeneralDescHeader is: %d\n, sizeof atErrorListIndSpecArray is: %d\n, sizeof EmaErrorListInd is: %d\n",
   	 sizeof(EmaIndGeneralDescHeader),
   	 sizeof(atErrorListIndSpecArray),
   	 sizeof(EmaErrorListInd));

	if(NULL == (pMsg = currentMsgPointer = (UINT32*)malloc( unIndBufSize ))) return;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (EmaIndGeneralDescHeader*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->msgOffset = sizeof(EmaIndGeneralDescHeader) + sizeof(atErrorListIndSpecArray);

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (message begin value)", *currentMsgPointer);


	currentMsgPointer += (sizeof(EmaIndGeneralDescHeader))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atErrorListIndSpecArray,sizeof(atErrorListIndSpecArray));

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atErrorListIndSpecArray))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Error List Indication Message
	// Fill Error List Message Header
	ptEmaIndHdr 					= (EmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= EMA_GET_ERROR_LIST_IND;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);

	// Fill Error list data
	ptErrorList = (TErrorList*)currentMsgPointer;

	ptErrorList->ulNumOfElem = 0;
	ptErrorList->ulNumOfElemFields = 2;
	printf("Starting write of errors at offset %d\n",(UINT32)((UINT32)ptErrorList - (UINT32)pMsg));
	for (i = 0; (i < MAX_NUM_OF_ERRORS_TO_REPORT)&&(errGetError(ulSlotID, i, &errorInfo)); i++)
	{
		strncpy(ptErrorList->errors[i].errorString, errorInfo->errDescr,sizeof(ptErrorList->errors[i].errorString) - 1);
		ptErrorList->errors[i].errorString[99] = '\0';	//strncpy won't put null termination , if strlen(target)<strlen(source)
		ptErrorList->errors[i].testId = errorInfo->errTestId;

	//	printf("added id(%d) error (%s)\n",errorInfo->errTestId,errorInfo->errDescr);
	}
	errClearErrorLog(ulSlotID);
	ptErrorList->ulNumOfElem = i;
	printf("Sending %d errors with totalmessagesize = %d",i,unIndBufSize);


	// Send Message Switch Server ...
   	if ( TCPSendData( TcpConnection[eSwitchDiagClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
   	{
   		//yosi fix return value socket

   		TcpConnection[eSwitchDiagClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eSwitchDiagClient].ul_PrevConnState = CONNECTED;

	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	// Free memory
	if ((void *)pMsg != NULL)
		  free((void *)pMsg);
}


/*	This function is the manager of tests. It prepares DSP tests, runs the regular (asynch) tests,
 * and then takes care of DSP tests. Also the loops,return values of tests and error reporting is
 * handled here.	*/
void* StartTestHandling(void* arg)
{
	struct timeval testStartTime;
	dgnsCtrlTestInfo *currentTest; //the test in testsession linked list item
	dgnsTestInfo *testSpecificInfo; //generic test structure. I use this
							//so i could find pointer to function, that will handle
							//the test.
	dgnsTestResult TestResults;
	UINT32 uSlotId,i;
	INT32 loopsToBeDone,DSPworkingTimeCounter;
	//UINT32	simulTestTimeout;
	e_TypeOfTest	currentTestType;

	if(NULL != arg)
	{
		uSlotId = *((UINT32 *)arg);
		free(arg);
	}
	else
	{
		return 0;
	}

	errorOccured[uSlotId][PQUnitSlot] = STATUS_UNKNOWN_YET;
	errorOccured[uSlotId][RTMUnitSlot] = STATUS_UNKNOWN_YET;

	/* Set first test to run */
	currentTest = diagCurrentSession[uSlotId].firstTest;
	/* Measure time of session start */
	gettimeofday(&testStartTime,NULL);
	diagCurrentSession[uSlotId].testSessionStartTime = (UINT32)testStartTime.tv_sec;
#if 0
	/* Have to zeroize the array,that will hold the pointers to test lists of DSP	*/
	for (i = 1; i< num_OF_DSPS_IN_BOARD + 1; i++)
	{
		errorOccured[uSlotId][i] = STATUS_UNKNOWN_YET;
		//initDspTestList(&(dspTestListCollection[uSlotId][i]),diagCurrentSession[uSlotId].stopOnFail,diagCurrentSession[uSlotId].isQuickSession); //TEMP QUICK
	}

	/*	On our first pass, I fill the information about all DSP tests that should be done
		 * and call DSP managing thread	*/
	while (currentTest)
	{
		/* If currentTest is test for DSP, we add it to needed slot in our test list: */

		if (getTestType(currentTest) == e_typeDSPtest)
		{
		//	printf("adding test to unit %d, address: %x (%x)\n",currentTest->unitOnSlot,dspTestListCollection[currentTest->unitOnSlot],&(dspTestListCollection[currentTest->unitOnSlot]));
			addToDspTestStruct(&(dspTestListCollection[uSlotId][currentTest->unitOnSlot]),currentTest);
		}
		currentTest = currentTest->nextTest;
	}
	
	currentTest = diagCurrentSession[uSlotId].firstTest;
#endif

	/*	Ok. We have thread working on DSPs. Now continue with our regular serial tasks:	*/

	loopsToBeDone = 0;
	//IpmcLedErrorOff();		//and if there was red error light before - turn it off
	//IpmcLedsActRdyGreenBlink(); //those are "start blinkin" lights
	LedDiagInProgress();
	
	while ((wasTestAskedToStop[uSlotId] == 0) &&
           ((diagCurrentSession[uSlotId].numOfLoops == 99) || (loopsToBeDone < diagCurrentSession[uSlotId].numOfLoops)))
	{
		while ((currentTest) && (wasTestAskedToStop[uSlotId] == 0))
		{
#if 0
			//simulTestTimeout = 0;
			//meanwhile - no timeout, stuck here forever
			if (isBoardToBoardTestRequested == 1)
			{
				//SendBoardMessage(MAIN_SUB_BOARD,3 - GetBoardNum(),eBOARD_LOAD_SIMULT_OK,NULL,0);
				printf("Sent eBOARD_LOAD_SIMULT_OK!\n");
				boardLoadSendMessage(serverSockFd,eBOARD_LOAD_SIMULT_OK);
				while (isBoardToBoardTestRequested == 1)
				{
					//if simulTestTimeout BOARD_TO_BOARD_SIMUL_TEST_TIMEOUT_MSEC
					EmbSleep(100);
					//simulTestTimeout += 100;	//counting the timeout
				}
			}

			if (isLanTestRequested == 1)
			{
				printf("Lan test allowed!\n");
				while (isLanTestRequested == 1)
				{	//spam this "i am ready" every 2 seconds, until test is complete
					BarakReadyForLanTest();
					EmbSleep(2000);
				}
			}

			if (shovalMulticastTestRequested == 1)
			{
				ShovalMulticastLoadTest();
				while (shovalMulticastTestRequested == 1)
				{
					EmbSleep(100);
				}
			}
#endif
			currentTestType = getTestType(currentTest);
			/*I only work now on non tests	*/
			printf("current test: id(%d),unitonslot(%d),test type:%d\n",currentTest->testId,currentTest->unitOnSlot,currentTestType);

			//if (currentTestType != e_typeDSPtest)/*If NOT DSP - start working*/
			{
				testSpecificInfo = testIdToTest(currentTest->testId,currentSystemTests);

#if NOLOOPTEST
				/*Skip this IF{} , if the test can't be looped and has already been run	*/
				if ((testSpecificInfo->canBeLooped == 0) && (loopsToBeDone > 0))
									break;
#endif
				/*Save the current time,for measuring the test's duration */
				gettimeofday(&testStartTime,NULL);
				currentTest->testStartTime = (UINT32)testStartTime.tv_sec;

				/*Mark our test as active now */
				currentTest->isActiveNow = 1;


				/*using this ,i will find what function should this test run,from the
				 * test description array */
				//testSpecificInfo = testIdToTest(currentTest->testId,currentSystemTests);
#if 0
				/*	Some tests (like dsp download) have to know what are they working on	*/
				if ((currentTest->testId == eDSP_DOWNLOAD_DIAG) || (currentTest->testId == eDSP_CONNECTIVITY_DIAG))
					TestResults.testData = currentTest->unitOnSlot;
				else	//Passing testid for tests that need to know about it(like RTM)
					TestResults.testData = currentTest->testId;
#else 
				memset(&TestResults, 0, sizeof(dgnsTestResult));
				TestResults.slotId= uSlotId;
				TestResults.unitId= currentTest->unitOnSlot;
				TestResults.testId = currentTest->testId;
#endif
				/*			-----------------------------------------------------
				 I start the test in thread,while passing the TestResults structure to it
				Then I wait the amount of time, that is set for the test,and if the test does not
				return in time,i kill it and return error.
				 */
#ifndef SERIAL_TEST_TIMEOUT_DEFINED
				testSpecificInfo->testFunction(&TestResults);
				WriteDiagResultFile(testSpecificInfo, currentTest, &TestResults);
#else
				serialTestComplete = 0;
				/* This function runs the thread with given argument,and after its completion
				 * sets the third parameter (flag) to 1 	*/
				testInfoStruct.serialTestComplete = &serialTestComplete;
				testInfoStruct.testFunction = testSpecificInfo->testFunction;
				testInfoStruct.TestResults = &TestResults;

				pthread_create(&serialTestThread,NULL,runThreadSerialTest,&testInfoStruct);
				//It is possible the estimated time is 0,then the test can run indefinetely.
				//rtm tests have their own timeout system too.
				if((currentTestType != e_typeRTMTest) && (testSpecificInfo->esimatedRunTime > 0))
				{
					gettimeofday(&testStartTime,NULL);
					/*Starting second time*1000 + bonus second time*1000 + starting msec + expected time in msec	*/
					/*all the seconds ->milisec , all the useconds->milisec	*/
					serialTestTimeout = (testStartTime.tv_sec + TEST_BONUS_SECONDS_TIME);
					serialTestTimeout += testSpecificInfo->esimatedRunTime;
					testNowTime = testStartTime.tv_sec;
					/*	While our max test time > current time,go on	*/
					//printf("serialTestTimeout = %ld,testNowTime=%ld\n",serialTestTimeout,testNowTime);
					printf("test %d Started at: %d. EstimatedRunTime=%d. Bonus=2\n",currentTest->testId,testNowTime,testSpecificInfo->esimatedRunTime);
					while (( serialTestTimeout >= testNowTime) && (serialTestComplete == 0))
					{
							EmbSleep(50);
							gettimeofday(&testStartTime,NULL);
							testNowTime = testStartTime.tv_sec;
					}
					if (serialTestComplete == 0) //test was not complete. kill it
					{
						pthread_cancel(serialTestThread);
						sprintf(TestResults.errString,"Test %d timeout exceeded.",currentTest->testId);
						TestResults.testResult = eStatFail;
						printf("test %d timeout. Started at: %d,now is:%d\n",currentTest->testId,serialTestTimeout,testNowTime);
					//report error
					}
					//clean the thread up.
					pthread_join(serialTestThread,NULL);
				}
				else
				{
					//This test does not have a timeout,but runs indefinetely.
					while (serialTestComplete == 0)
						EmbSleep(50);
				}
				/*----------------------------------------------------------------------*/
#endif

				//parse the test's failed/succeed only if it wasnt skipped
				//if (1)
				if (TestResults.testData != TEST_DATA_FOR_SKIPPED_TEST)
				{
					if (TestResults.testResult == eStatFail)
					{
						if (currentTestType != e_typeRTMTest) //don't want pq updates if it was rtm test
						{
	#if 0
							if ((currentTest->testId == eDSP_DOWNLOAD_DIAG) || (currentTest->testId == eDSP_CONNECTIVITY_DIAG))
							{
								errorOccured[uSlotId][currentTest->unitOnSlot] = STATUS_FAIL;
							//	printf("err status fail for dsp %d\n",currentTest->unitOnSlot);
							}
							else
							{
								errorOccured[uSlotId][PQUnitSlot] = STATUS_FAIL;
							//	printf("err status fail for pq\n");
							}
	#else
							errorOccured[uSlotId][currentTest->unitOnSlot] = STATUS_FAIL;
	#endif
						}
						printf("After non dsp test.Failed: %s\n",TestResults.errString);

						//IpmcLedErrorOn();
						currentTest->failTests++;
						errReportError(uSlotId, currentTest->testId,TestResults.errString);
					/* If the session has been started with request to stop on error -
					 * we stop here. (Error occured here) */
						if (diagCurrentSession[uSlotId].stopOnFail == 1)
							wasTestAskedToStop[uSlotId] = 1;
					}
					else
					{
						if (currentTestType != e_typeRTMTest) //don't want pq updates if it was rtm test
						{
#if 0
							if ((currentTest->testId == eDSP_DOWNLOAD_DIAG) || (currentTest->testId == eDSP_CONNECTIVITY_DIAG))
							{
								if (errorOccured[uSlotId][currentTest->unitOnSlot] != STATUS_FAIL)
								{
								//	printf("err status ok for dsp %d\n",currentTest->unitOnSlot);
									errorOccured[uSlotId][currentTest->unitOnSlot] = STATUS_OK;
								}
							}
							else
							{
								if  (errorOccured[uSlotId][PQUnitSlot] != STATUS_FAIL)
								{
								//	printf("err status ok for pq\n");
									errorOccured[uSlotId][PQUnitSlot] = STATUS_OK;
								}
							}
#else
							if (errorOccured[uSlotId][currentTest->unitOnSlot] != STATUS_FAIL)
							{
							//	printf("err status ok for dsp %d\n",currentTest->unitOnSlot);
								errorOccured[uSlotId][currentTest->unitOnSlot] = STATUS_OK;
							}
#endif
						}
						printf("After non dsp test. Success\n");
						currentTest->successTests++;
                        //	errorOccured[uSlotId][PQUnitSlot] = STATUS_OK;
					}
					currentTest->isActiveNow = 0;	//the test is over
				}
				else
				{
					//this test is not "not active" , but "not relevant"
					currentTest->isActiveNow = 2;
				}
				/* Now get the time and calculate the duration of test */
				gettimeofday(&testStartTime,NULL);
				currentTest->duration = (UINT32)testStartTime.tv_sec - currentTest->testStartTime;


		//		printTestStatuses();
				currentTest->loopsDone++;
				/* No longer active test*/

			}	/*ENDOF:		if (!isTestDSPTest(currentTest))	*/

			/*	If i did the test, or it was DSP test,and i skipped it - i have to advance
			 * the pointer anyway:	*/
			currentTest= currentTest->nextTest;
			EmbSleep(MS_SLEEP_BETWEEN_TESTS);

			/*	Here i insert new code, that checks from a flag isBoardToBoardTestRequested.
			 * this flag means that another board wants to use hardware on this board for various tests.
			 * Currently there is only "Bus Load Between Boards" tests.	*/

		}
#if 0
		/*Go over all dsp tests on this loop cycle. i say those are all of DSP type. Even ALL-DSPTests	*/
		currentTestType = e_typeDSPtest;
		DSPworkingTimeCounter = 0;
		dspTestsFailed[uSlotId] = 0; //this is changed to 1 if even one of the dsps failed a test
		dspTestThread(uSlotId);

		while (areDspsBusy()) //while we have dsps working
		{
			DSPworkingTimeCounter += 50;
			EmbSleep(50);
			//ask dsps for status every N second.
			if (DSPworkingTimeCounter > MS_DELAY_BETWEEN_DSP_STATUS_POLLING)
			{
				DSPworkingTimeCounter = 0;
				askDspsForTestStatus();
			}
		}

		// If the dsps had an error,and we were asked to stop on fail... stop on fail!
		if (dspTestsFailed[uSlotId] == 1 )
		{
			//IpmcLedErrorOn();  //error occured - error red led on
			if  (diagCurrentSession[uSlotId].stopOnFail == 1)
				wasTestAskedToStop[uSlotId] = 1;
		}
#endif
		/*	new loop.	*/
		currentTest = diagCurrentSession[uSlotId].firstTest;
		loopsToBeDone++;
		currentLogLoopNumber++;
	}


	/*	The end of session:
		Session finishing moves,including time calculation:	*/
	/*	release our structs	*/

	/* Setting statuses for units */
	/*
	if (errorOccured[uSlotId][PQUnitSlot] != STATUS_FAIL) //if not failed - make it pass
		errorOccured[uSlotId][PQUnitSlot] = STATUS_OK;
	for ( i = 1 ; i < num_OF_DSPS_IN_BOARD + 1 ; i++)
		if (errorOccured[uSlotId][i] != STATUS_FAIL)
			errorOccured[uSlotId][i] = STATUS_OK;
		*/

	/*	If received a request for board to board on the last test, and didn't get
	 * to this check again	*/
#if 0
	if (isBoardToBoardTestRequested == 1)
	{
		printf("Sent eBOARD_LOAD_SIMULT_OK!\n");
		boardLoadSendMessage(serverSockFd,eBOARD_LOAD_SIMULT_OK);
		while (isBoardToBoardTestRequested == 1)
		{
			//if simulTestTimeout BOARD_TO_BOARD_SIMUL_TEST_TIMEOUT_MSEC
			EmbSleep(100);
			//simulTestTimeout += 100;	//counting the timeout
		}
	}
#endif
	diagCurrentSession[uSlotId].hasStarted = 0;
	currentTestType = e_typeNoTestRunning;
	//IpmcLedsActRdyOff(); //stop blinkin "working" lights
	for(i =0; i < sizeof(diagCurrentSession)/sizeof(diagCurrentSession[0]); i++)
	{
		if(diagCurrentSession[i].hasStarted == 1) break;
	}
	if(i == (sizeof(diagCurrentSession)/sizeof(diagCurrentSession[0]))) LedDiagComplete();
	
	wasTestAskedToStop[uSlotId] = 1;
	gettimeofday(&testStartTime,NULL);
	diagCurrentSession[uSlotId].testSessionDurationTime = (UINT32)testStartTime.tv_sec - diagCurrentSession[uSlotId].testSessionStartTime;

	printf("\nTotal session time: (%d)\n",diagCurrentSession[uSlotId].testSessionDurationTime);
#if 0
	for (i = 1; i < num_OF_DSPS_IN_BOARD + 1; i++)
		freeDspTestList(&(dspTestListCollection[uSlotId][i]));
#endif
	return 0;
}

#if 0
int	areDspsBusy(UINT32 ulSlotId)
{
	//meanwhile its simple working counter, but
	//i have to add some keep-alive timer
	UINT32	i;
	struct timeval tCurrentTime;
	UINT32	timeOutVal,currentTimeVal;
	/*	If this variable is still here,it means i dont have timing issues.
	 * Otherwise, it would've been moved to "outside" of function	*/
	char 	errToReport[200];

	for (i = 1; i <	num_OF_DSPS_IN_BOARD + 1; i++)
	{
		if (lastReqTimeStamp[ulSlotId][i] != DSP_INFINITE_VAL) //timeout has no sense if
													//we are talking about infinite test
													//like homologation
		{
			if ((lastReqTimeStamp[ulSlotId][i]) && (lastReqTimeStamp[ulSlotId][i] != DSP_TIMEDOUT_VAL))
			{
				gettimeofday(&tCurrentTime,NULL);
				timeOutVal = DSP_SEC_TIMEOUT;
				timeOutVal += lastReqTimeStamp[ulSlotId][i];
				currentTimeVal = (UINT32)tCurrentTime.tv_sec;
				if (timeOutVal < currentTimeVal)
				{
					/*One dsp communication failed. Meaning - it is not working */
					sprintf(errToReport,"TIMEOUT ERROR: Dsp number (%d) hasn't responded within defined time.",i);
					printf("connectivity error,dsp (%d)\nCurrenttime(%d),savedtime: (%d)",i,(UINT32)tCurrentTime.tv_sec,lastReqTimeStamp[ulSlotId][i]);
					lastReqTimeStamp[ulSlotId][i] = DSP_TIMEDOUT_VAL;
					//IpmcLedErrorOn();
					isDspSessionRunning[ulSlotId][i] = 0; //no longer running session
					errorOccured[ulSlotId][i] = STATUS_FAIL;
					printf("err status fail timeout dsp %d\n",i);
					errReportError(ulSlotId, eDSP_CONNECTIVITY_DIAG,errToReport);
					dspWorkingCount--;
				}
			}
		}
	}
	return (dspWorkingCount > 0);

}
#endif

/*======================================================================*/
/* FUNCTION:		BuildEnterDiagModeIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)				*/
/* PURPOSE :		Test list indication	 							*/
/* PARAMETERS:		msgId, slotId										*/
/* RETURN VALUE:	void												*/
/* LIMITATION:															*/
/*======================================================================*/
void BuildControlResponseMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr			*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	 		*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	printf("BuildControlResponseMsg\n");
	//return;

	// Allocate message	pointer
	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atControlResponseMsgArray) +
				   sizeof(TEmaControlResponseMessage);


	if(NULL == (pMsg = currentMsgPointer = (UINT32*)malloc( unIndBufSize ))) return;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset = sizeof(TSpecGnrlHdr) + sizeof(atControlResponseMsgArray);

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (message begin value)", *currentMsgPointer);
   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"ptEmaIndGeneralDescHdr->ulMsgOffset (in bytes) = %d", ptEmaIndGeneralDescHdr->ulMsgOffset);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atControlResponseMsgArray,sizeof(atControlResponseMsgArray));

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atControlResponseMsgArray))/4;

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Diag mode Indication Message
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= 3;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);
/*
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(DIAG_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}
*/
   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildEnterDiagModeInd): sending message: %s", currentMsgPointer);

	StringWrapper( eEmaApiServer, (UINT32)pMsg);
//	if ( TCPSendData( TcpConnection[eSwitchDiagServer].s, (VOID *)pMsg, unIndBufSize, 0) == SOCKET_OPERATION_FAILED)
//	{
//	   	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildEnterDiagModeInd): problem in sending data...\n");
//	    TcpConnection[eSwitchDiagServer].ul_ConnectionStatus = NOT_CONNECTED;
//	    TcpConnection[eSwitchDiagServer].ul_PrevConnState = CONNECTED;
//	    // TBD: Find out if there is any need to send failure notice to EMA.
//	}
}

errList* errAnchor[MAX_SLOT_NUM] = {0};
errList* errTail[MAX_SLOT_NUM] = {0};
pthread_mutex_t errMutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * What we can do with error log,is to add to it (linked list), clear it,
 * or request specific error from it (by number)
 * */


//adding new error (with error id and description) to errors linked list
void errReportError(UINT32 ulSlotId, int errNum,char* ErrorDesc)
{
	char 	errString[200];
	errList *newErrorLog = (errList *)malloc(sizeof(errList));
	printf("test id %d had error: %s\n",errNum,ErrorDesc);

	if (newErrorLog)
	{
		newErrorLog->errDescr = (char*)malloc((strlen(ErrorDesc) + 2)*sizeof(char));
		if (newErrorLog->errDescr)
		{
			strcpy(newErrorLog->errDescr,ErrorDesc);
			newErrorLog->errTestId = errNum;
			newErrorLog->nextError = NULL;
			pthread_mutex_lock (&errMutex);
			if ((errTail[ulSlotId] == NULL ) || ( errAnchor[ulSlotId] == NULL)) //tail and anchor are empty = empty list
			{
				errTail[ulSlotId] = newErrorLog;
				errAnchor[ulSlotId] = newErrorLog;
			}
			else
			{  //adding new error in the tail
				errTail[ulSlotId]->nextError = newErrorLog;
				errTail[ulSlotId] = newErrorLog;
			}
			pthread_mutex_unlock (&errMutex);
		}
		else
			free(newErrorLog);
	}

	snprintf(errString, sizeof(errString), "Test ID = %d: %s", errNum, ErrorDesc);
	//WriteLog(errString);
	return;
}


void errClearErrorLog(UINT32 ulSlotId)
{
	errList *currentErrorLog;

	currentErrorLog = errAnchor[ulSlotId];
	pthread_mutex_lock (&errMutex);
	while (errAnchor[ulSlotId])
	{
		currentErrorLog = errAnchor[ulSlotId]->nextError;
		free(errAnchor[ulSlotId]->errDescr);
		free(errAnchor[ulSlotId]);
		errAnchor[ulSlotId] = currentErrorLog;
	}
	errTail[ulSlotId] = 0;
	pthread_mutex_unlock(&errMutex);
}



//returns indexed error. Remember to allocate sting part of err,before calling this
int  errGetError(UINT32 ulSlotId, int errIndex,errList** errDescription) //if returns 0 - no such error in log
{
	errList *currentErrorLog;
	int i;



	pthread_mutex_lock (&errMutex);
	currentErrorLog = errAnchor[ulSlotId];

	for (i=0 ; i<errIndex; i++)
	{
		if (currentErrorLog)
			currentErrorLog = currentErrorLog -> nextError;
 	}
	pthread_mutex_unlock (&errMutex);

	if (currentErrorLog == 0) //didnt find our error
		return 0;
	/*
	errDescription->errDescr = (char*)malloc(strlen(currentErrorLog->errDescr)*sizeof(char));
	strcpy(errDescription->errDescr,currentErrorLog->errDescr);
	errDescription->errTestId = currentErrorLog->errTestId;
	errDescription->nextError = 0;
	*/
	*errDescription = currentErrorLog;
	return 1;
}


pthread_mutex_t dgnsTestMutex = PTHREAD_MUTEX_INITIALIZER;


/*
 * In this function,i am adding test to current TestSession (diagCurrentSession),
 * It will be linked list of tests to do.
 */

int dgnsTestAddTest(int testId, UINT32 ulSlotID, int unitOnSlot)
{
	dgnsCtrlTestInfo *newTest;

	newTest = (dgnsCtrlTestInfo*)malloc(sizeof(dgnsCtrlTestInfo));
	if (newTest)
	{
		memset(newTest, 0, sizeof(dgnsCtrlTestInfo));
		//newTest->testStartTime = now;
		newTest->testId = testId;
		newTest->loopsDone = 0;
		newTest->successTests = 0;
		newTest->failTests = 0;
		newTest->isActiveNow = 0;
		newTest->slotID = ulSlotID;
		newTest->unitOnSlot = unitOnSlot;
		newTest->duration = 0;
		newTest->nextTest = 0;

		pthread_mutex_lock (&dgnsTestMutex);

		if (diagCurrentSession[ulSlotID].firstTest == 0)
		{
			//first test
			diagCurrentSession[ulSlotID].firstTest = newTest;
			diagCurrentSession[ulSlotID].lastTest = newTest;
		}
		else
		{
			diagCurrentSession[ulSlotID].lastTest->nextTest = newTest;
			diagCurrentSession[ulSlotID].lastTest = newTest;
		}
		pthread_mutex_unlock(&dgnsTestMutex);
		return 1;
	}
	else
		return -1; //couldn't allocate new test from some reason

}

/* Erasing entire test list*/
int dgnsClearTests(UINT32 ulSlotID)
{
	dgnsCtrlTestInfo *delTest;
	pthread_mutex_lock (&dgnsTestMutex);
	while (diagCurrentSession[ulSlotID].firstTest)
	{
		delTest = diagCurrentSession[ulSlotID].firstTest->nextTest;
		free(diagCurrentSession[ulSlotID].firstTest);
		diagCurrentSession[ulSlotID].firstTest = delTest;
	}
	diagCurrentSession[ulSlotID].lastTest = 0; //it will be deleted in previous loop anyway
	pthread_mutex_unlock(&dgnsTestMutex);
	return 1;
}

/*This function receives index of test to find,and returns pointer to
 * test struct,or NULL, if no such index
 * */
int dgnsGetTestFromList(UINT32 ulSlotID, int testIndex,dgnsCtrlTestInfo **findTest)
{
	int i;
	//no tests!
	if (diagCurrentSession[ulSlotID].firstTest == 0 )
		return -1;
	pthread_mutex_lock (&dgnsTestMutex);
	*findTest = diagCurrentSession[ulSlotID].firstTest;

	for (i=0 ; i<testIndex; i++)
		if (*findTest)
			*findTest = (*findTest)->nextTest;

	pthread_mutex_unlock(&dgnsTestMutex);
	 //testid -> string test name.
	if (*findTest == 0) //didnt find our test
		return -1;
	else
		return 1;

}

dgnsTestInfo * testIdToTest(UINT32 TestId,dgnsTestInfo currentSystemTests[])
{
	int i=0;

	while (currentSystemTests[i].TestId != 0xffff)
	{
		if (currentSystemTests[i].TestId == TestId)
			break;
		i++;
	}
	//it will return test's name,or "Empty",if not found
	return &(currentSystemTests[i]);
}

/*
e_TypeOfTest	getTestType(dgnsCtrlTestInfo * testToExamine)
{
	//in switch - all tests are serial
	return e_typeSerialTest;
}
*/

void dummyTest(dgnsTestResult *diagTestRes)
{
	printf("running dummy test");
	strcpy(diagTestRes->errString,"Random error!");
	diagTestRes->testResult = eStatFail;
}

void dummySleepSuccessTest(dgnsTestResult *diagTestRes)
{
    static unsigned int cnt = 0;
    cnt++;
    if(0 == cnt % 1000)
    {
        printf("\n----------------\nrunning %s, %d\n----------------\n", __FUNCTION__, cnt / 1000);
    }
    
    EmbSleep(10);
    diagTestRes->testResult = eStatOk;
}


void printTestStatuses(UINT32 ulSlotID)
{

	int i;
	dgnsCtrlTestInfo *		testInfoStruct;
	dgnsTestInfo *		testListInfo;

	for(i = 0; (i < MAX_NUM_OF_REPORTED_TESTS) && (dgnsGetTestFromList(ulSlotID, i,&testInfoStruct)>0); i++)
	{
		testListInfo = testIdToTest(testInfoStruct->testId,currentSystemTests);
		printf("%d on %d)%s , Success:%d, Failed:%d , Loop: %d\n",testInfoStruct->testId,testInfoStruct->unitOnSlot,testListInfo->TestName,testInfoStruct->successTests,testInfoStruct->failTests,currentLogLoopNumber);
	}
}


void PrintDgnsTestInfo(const dgnsTestInfo *pTestEntry)
{
    printf("\nid %d, %s, hw %s, loop %s, quick %s, time %d, sys %d, func %p\n",
           pTestEntry->TestId,
           pTestEntry->TestName,
           (ePQ == pTestEntry->TestOn ? "PQ" : "DSP"),
           (TRUE == pTestEntry->canBeLooped ? "YES" : "NO"),
           (TRUE == pTestEntry->isQuickVersion ? "YES" : "NO"),
           pTestEntry->esimatedRunTime,
           pTestEntry->SystemType,
           pTestEntry->testFunction
           );
}

INT32 GetMngIPAddr(TMngrIpAddr * mngrIpAddr)
{
	INT32 rc = -1 , uFileWriteSize = 0, ret = -1;
	INT32 rc_getl, len, buff_size;
	FILE *fp;
	INT8 *pInfoBuff = NULL, *tmp_buff;
	UINT32 unMfaInfoFail = 0 ,unFileSize = 0;
	char szMngrIpType[64] = {0};
	char szMngrIpv6ConfType[64] = {0};
	
	MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetMngIPAddr");	
	
	pInfoBuff = 0;
	tmp_buff = 0;

	fp = fopen(FILENAME_sysInfo, "r");
	if(fp == NULL)
	{
		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ERROR: %s file DOES NOT EXIST",FILENAME_sysInfo);
		goto info_parse_end;
	}
	
	//get the file size
	fseek(fp,0,SEEK_END);
	unFileSize = ftell(fp);
	fseek(fp,0,SEEK_SET);
	
	pInfoBuff = (INT8*)malloc(unFileSize);
	if(pInfoBuff == 0)
	{
		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ERROR: %s MALLOC FAILED",FILENAME_sysInfo);
		goto info_parse_end;
	}
	
	memset(pInfoBuff, 0, unFileSize);	
	
	rc = fread((INT8*)pInfoBuff,sizeof(INT8),unFileSize,fp);
    	if (rc == 0)
  	{
         MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Unable To Read %s file",FILENAME_sysInfo) ;
	   ret = -1;
         goto info_parse_end;
  	}
	pInfoBuff[unFileSize - 1] = '\0';

	//Try to get management IP address
	/*
		MNG_IP_TYPE=BOTH (IPV6/IPV4)
		MNG_IPV6_CONFIGURATION_TYPE=AUTO  (MANUAL)
		MNG_IPV6_DEF_GATEWAY=fe80::b2c6:9aff:fed2:1f81
		MNG_MCMS_IPV6_ADDR1=2001:0:105:0:20c:29ff:fe68:4631
		MNG_MCMS_IPV6_ADDR2=fec0:0:105:0:20c:29ff:fe68:4631
		MNG_MCMS_IPV6_ADDR3=fe80::20c:29ff:fe68:4631

		MNG_MCMS_IP_ADDR=172.21.105.106
		MNG_SUBNET_MASK=255.255.254.0	
		MNG_DEF_GATEWAY=172.21.105.254

	*/

	//Get MNG_IP_TYPE
    	tmp_buff = strstr(pInfoBuff, "MNG_IP_TYPE");
	if(tmp_buff != NULL)
	{
		my_strccpy(szMngrIpType, tmp_buff);
		if(0 == strcmp(szMngrIpType, "IPV6")) mngrIpAddr->ipType = eIPV6;
		else if(0 == strcmp(szMngrIpType, "BOTH")) mngrIpAddr->ipType = eBoth;
		else  mngrIpAddr->ipType = eIPV4;
	}
	else
	{	
		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetMngIPAddr : not found MNG_IP_TYPE");
         	goto info_parse_end;
	}

	if( eIPV4 == mngrIpAddr->ipType || eBoth == mngrIpAddr->ipType)
	{
		//Get MNG_MCMS_IP_ADDR
	    	tmp_buff = strstr(pInfoBuff, "MNG_MCMS_IP_ADDR");
		if(tmp_buff != NULL)
		{
			my_strccpy(mngrIpAddr->IpAddr, tmp_buff);
		}
		else
		{	
			MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetMngIPAddr : not found MNG_MCMS_IP_ADDR");
	         	goto info_parse_end;
		}

		//Get MNG_SUBNET_MASK
	    	tmp_buff = strstr(pInfoBuff, "MNG_SUBNET_MASK");
		if(tmp_buff != NULL)
		{
			my_strccpy(mngrIpAddr->netmask, tmp_buff);
		}
		else
		{	
			MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetMngIPAddr : not found MNG_SUBNET_MASK");
	         	goto info_parse_end;
		}

		//Get MNG_DEF_GATEWAY
	    	tmp_buff = strstr(pInfoBuff, "MNG_DEF_GATEWAY");
		if(tmp_buff != NULL)
		{
			my_strccpy(mngrIpAddr->IpGateway, tmp_buff);
		}
		else
		{	
			MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetMngIPAddr : not found MNG_DEF_GATEWAY");
	         	goto info_parse_end;
		}
	}

	if( eIPV6 == mngrIpAddr->ipType || eBoth == mngrIpAddr->ipType)
	{
		//Get MNG_IPV6_CONFIGURATION_TYPE
	    	tmp_buff = strstr(pInfoBuff, "MNG_IPV6_CONFIGURATION_TYPE");
		if(tmp_buff != NULL)
		{
			my_strccpy(szMngrIpv6ConfType, tmp_buff);
			if(0 == strcmp(szMngrIpv6ConfType, "MANUAL")) mngrIpAddr->ipv6ConfType= eManual;
			else  mngrIpAddr->ipv6ConfType = eAuto;
		}
		else
		{	
			MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetMngIPAddr : not found MNG_IPV6_CONFIGURATION_TYPE");
	         	goto info_parse_end;
		}

		if( eManual == mngrIpAddr->ipv6ConfType)
		{			
			//Get MNG_MCMS_IPV6_ADDR1
		    	tmp_buff = strstr(pInfoBuff, "MNG_MCMS_IPV6_ADDR1");
			if(tmp_buff != NULL)
			{
				my_strccpy(mngrIpAddr->Ipv6Addr1, tmp_buff);
			}
			else
			{	
				MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetMngIPAddr : not found MNG_MCMS_IPV6_ADDR1");
		         	goto info_parse_end;
			}

			//Get MNG_MCMS_IPV6_ADDR2
		    	tmp_buff = strstr(pInfoBuff, "MNG_MCMS_IPV6_ADDR2");
			if(tmp_buff != NULL)
			{
				my_strccpy(mngrIpAddr->Ipv6Addr2, tmp_buff);
			}
			else
			{	
				MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetMngIPAddr : not found MNG_MCMS_IPV6_ADDR2");
		         	goto info_parse_end;
			}
			
			//Get MNG_MCMS_IPV6_ADDR3
		    	tmp_buff = strstr(pInfoBuff, "MNG_MCMS_IPV6_ADDR3");
			if(tmp_buff != NULL)
			{
				my_strccpy(mngrIpAddr->Ipv6Addr3, tmp_buff);
			}
			else
			{	
				MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetMngIPAddr : not found MNG_MCMS_IPV6_ADDR3");
		         	goto info_parse_end;
			}

			//Get MNG_IPV6_DEF_GATEWAY
		    	tmp_buff = strstr(pInfoBuff, "MNG_IPV6_DEF_GATEWAY");
			if(tmp_buff != NULL)
			{
				my_strccpy(mngrIpAddr->Ipv6Gateway, tmp_buff);
			}
			else
			{	
				MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetMngIPAddr : not found MNG_IPV6_DEF_GATEWAY");
		         	goto info_parse_end;
			}
		}
	}
	
	ret = 1;
info_parse_end:

	if (pInfoBuff != NULL)
	{
		free(pInfoBuff);
		pInfoBuff = NULL;
	}

	if(ret < 0)
	{
		mngrIpAddr->ipType = eIPV4;
		strcpy(mngrIpAddr->IpAddr, "192.168.1.254");
		strcpy(mngrIpAddr->netmask, "255.255.255.0");
		strcpy(mngrIpAddr->IpGateway, "192.168.1.1");
	}

	if(fp)
		fclose(fp);

	return(ret);
}

char * FindValidIpv6Addr(TMngrIpAddr * mngrIpAddr)
{
	if(0 == strncmp(mngrIpAddr->Ipv6Addr1, "2001", 4)) return mngrIpAddr->Ipv6Addr1;
	else if(0 == strncmp(mngrIpAddr->Ipv6Addr2, "2001", 4)) return mngrIpAddr->Ipv6Addr2;
	else if(0 == strncmp(mngrIpAddr->Ipv6Addr3, "2001", 4)) return mngrIpAddr->Ipv6Addr3;
	return mngrIpAddr->Ipv6Addr1;
}

void SetNICIpAddress(TMngrIpAddr * mngrIpAddr)
{
	char line[1024] = {0};
	char * ptValidIpv6Addr = NULL;
	
	if(NULL == mngrIpAddr) return;

	if(eIPV4 == mngrIpAddr->ipType || eBoth == mngrIpAddr->ipType)
	{
		
		snprintf(line, sizeof(line), "/sbin/ifconfig eth0 %s netmask %s", mngrIpAddr->IpAddr, mngrIpAddr->netmask);
		system(line);
		snprintf(line, sizeof(line), "/sbin/route add default gw %s eth0", mngrIpAddr->IpGateway);
		system(line);
	}

	if(eIPV6 == mngrIpAddr->ipType || eBoth == mngrIpAddr->ipType)
	{
		if(eAuto == mngrIpAddr->ipv6ConfType)
		{
			system("echo 1 | tee /proc/sys/net/ipv6/conf/eth0/autoconf");
		}
		else
		{
			system("echo 0 | tee /proc/sys/net/ipv6/conf/eth0/autoconf");
			ptValidIpv6Addr = FindValidIpv6Addr(mngrIpAddr);
			snprintf(line, sizeof(line), "/sbin/ifconfig eth0 add %s/64", ptValidIpv6Addr);
			system(line);
			snprintf(line, sizeof(line), "/sbin/route -A inet6 add default gw %s dev eth0", mngrIpAddr->Ipv6Gateway);
			system(line);			
		}
	}

	system("/sbin/ifconfig eth0 up");
	system("/sbin/ifconfig eth1 up");
}

void GetCurrentIpAddress(TMngrIpAddr * mngrIpAddr, char * IPAddrString, int size)
{
	char sshdListenAddr[1024] = {0}, sshdListenAddr2[1024] = {0}, line[1024] = {0}, addrBuf_global[IPV6_ADDRESS_LEN] = {0}, *ptValidIpv6Addr = NULL;
      BOOL bGetIPv6 = FALSE;
 
      if(eIPV4 != mngrIpAddr->ipType)
      {
           if(eAuto == mngrIpAddr->ipv6ConfType)
           {
                EmbSleep(5000);
                bGetIPv6 = RetrieveIpV6Address(addrBuf_global, IPV6_ADDRESS_LEN);
           }
           else
           {
                ptValidIpv6Addr = FindValidIpv6Addr(mngrIpAddr);
                strncpy(addrBuf_global, ptValidIpv6Addr, IPV6_ADDRESS_LEN - 1);
                bGetIPv6 = TRUE;
           }
      }

	if(eIPV4 == mngrIpAddr->ipType)
	{
		strncpy(IPAddrString, mngrIpAddr->IpAddr, size - 1);
            	snprintf(sshdListenAddr, sizeof(sshdListenAddr), "echo ListenAddress %s", mngrIpAddr->IpAddr);
	}
      else if(eIPV6 == mngrIpAddr->ipType)
	{
		if(bGetIPv6 && 0 != strlen(addrBuf_global))
            {
    		    strncpy(IPAddrString, addrBuf_global, size - 1);
                 my_changeColonToDot(IPAddrString);
                 snprintf(sshdListenAddr, sizeof(sshdListenAddr), "echo ListenAddress %s", addrBuf_global);
            }
            else
            {
                strncpy(IPAddrString, "192.168.1.254", size - 1);
                snprintf(sshdListenAddr, sizeof(sshdListenAddr), "echo ListenAddress %s", "192.168.1.254");
            }
	}
      else if(eBoth == mngrIpAddr->ipType)
      {
            	strncpy(IPAddrString, mngrIpAddr->IpAddr, size - 1);
            	snprintf(sshdListenAddr, sizeof(sshdListenAddr), "echo ListenAddress %s", mngrIpAddr->IpAddr);
                
             if(bGetIPv6 && 0 != strlen(addrBuf_global))
             {
                
                snprintf(sshdListenAddr2, sizeof(sshdListenAddr2), "; echo ListenAddress %s", addrBuf_global);
                strcat(sshdListenAddr, sshdListenAddr2);
             }
      }
	
#ifdef DIAG_SSHD_START
	if(IsDirFileExist("/output/DIAGSSHDSTART"))
	{
		snprintf(line, sizeof(line), "(cat /etc/ssh/sshd_config; %s) > /tmp/sshd.conf", sshdListenAddr);
		system(line);
		EmbSleep(1000);
		system("killall -1 sshd");
	}
#endif
}

static void GetStartTime(char * pszStartTime, BOOL bFormat)
{
	time_t timep;
	struct tm result;
	int i = 0;

	timep = time((time_t*)NULL);
	gmtime_r(&timep, &result);

	if(bFormat)
	{
		sprintf(pszStartTime, "%d/%d/%d %d:%02d",  result.tm_mon + 1, result.tm_mday, result.tm_year + 1900, result.tm_hour, result.tm_min);
	}
	else
	{
		sprintf(pszStartTime,
		      "%d%2d%2d%2d%2d",
		      result.tm_year + 1900,
		      result.tm_mon + 1,
		      result.tm_mday,
		      result.tm_hour,
		      result.tm_min);

		int len = strlen(pszStartTime);
		for (i = 0; i < len; i++)
		{
			if (pszStartTime[i] == ' ')
		  		pszStartTime[i] = '0';
		}
	}
}

static void WriteDiagFile(const char * path, char * filename, char *msg)
{
	FILE * pFile;
	unsigned int fileSize;
	size_t retVal;
	char title[2048];
	char szStartTime[256];
	
	//check dir exist
	if(!IsDirFileExist(path))
	{
		if(0 != mkdir(path, 0777))
		{
			//MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"WriteDiagResultFile mkdir failed: %s errno: %d", DIAG_RESULT_USB_PATH, errno);
			return;
		}
	}

	//create file name: DiagnosticsSummary_[Management IP]_[date].txt 
	if(filename[0] == '\0')
	{
		GetStartTime(szStartTime, FALSE);
		sprintf(filename, "%sDiagnosticsSummary_%s_%s.txt", path, g_mngIPAddr, szStartTime);
	}
	
	// open files
	pFile = fopen(filename, "a+");
	if (pFile == NULL)
	{
		return;
	}
	
	// find files size
	fseek (pFile, 0, SEEK_END);
	fileSize = ftell(pFile);

	if(fileSize == 0)
	{
		//write title
		GetStartTime(szStartTime, TRUE);
		snprintf(title, sizeof(title), "Diagnostics Summary created on %s\r\n",  szStartTime);
		retVal = fwrite(title, 1, strlen(title), pFile);
		if(retVal != strlen(title))
		{
			printf("\nError in WriteDiagFile title");
		}
		
		sprintf(title, "%-20s%-20s%-20s%-20s%-20s%-40s%-20s%-20s%s\r\n", "ID" ,"SlotID" ,"Module" ,"UnitID", "Type", "Test Name" ,"Test ID" ,"Status" ,"Details");
		retVal = fwrite(title, 1, strlen(title), pFile);
		if(retVal != strlen(title))
		{
			printf("\nError in WriteDiagFile title items");
		}
	}

	// write the line to the file
	retVal = fwrite(msg, 1, strlen(msg), pFile);
	if(retVal != strlen(msg))
	{
		printf("\nError in WriteDiagFile");
	}
	
	fclose(pFile);
}

const char * GetModuleFromSlotID(UINT32 slotID)
{
	switch(slotID)
	{
		case CNTL_SLOT_ID:
			return "CNTL";
		case DSP_CARD_SLOT_ID_0:
		case DSP_CARD_SLOT_ID_1:
		case DSP_CARD_SLOT_ID_2:
			return NETRA_DSP_BOARD_NAME;
		case ISDN_CARD_SLOT_ID:
			return NETRA_RTM_ISDN_NAME;
		default:
			return "Unknown";
	}
}

void WriteDiagResultFile(const dgnsTestInfo *pTestEntry, const dgnsCtrlTestInfo * pTestCurrent, const dgnsTestResult * pTestResults)
{	
	char msg[2046] = {0}, szUnitId[64] = {0};
	static int diagResultID = 0;
	
	if(NULL == pTestEntry || NULL == pTestResults)
	{
		MfaBoardPrint(DIAG_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"WriteDiagResultFile: pTestEntry(%x)  pTestResults(%x)", pTestEntry, pTestResults);
		return;
	}

	//sprintf(title, "%-20s%-20s%-20s%-20s%-20s%-40s%-20s%-20s%s\n", 
	// "ID" ,"SlotID" ,"Module" ,"UnitID", "Type", "Test Name" ,"Test ID" ,"Status" ,"Details");

	if(eUndefined == pTestEntry->TestOn)
	{
		strcpy(szUnitId, "N\\A");
	}
	else
	{
		snprintf(szUnitId, 64, "%d", pTestCurrent->unitOnSlot);		
	}
	
	
    	snprintf(msg, sizeof(msg), "%-20d%-20d%-20s%-20s%-20s%-40s%-20d%-20s%s\r\n",
           diagResultID,
           pTestCurrent->slotID,
           GetModuleFromSlotID(pTestCurrent->slotID),
           szUnitId,
           (eUndefined == pTestEntry->TestOn ? "System" : "DSP"),
           pTestEntry->TestName,
           pTestEntry->TestId,
           (eStatOk == pTestResults->testResult? "Passed" : "Failed"),
           (eStatOk == pTestResults->testResult? "" : pTestResults->errString)
        );

	WriteDiagFile(DIAG_RESULT_LOCAL_PATH, g_diagResultFileName, msg);
	WriteDiagFile(DIAG_RESULT_USB_PATH, g_diagUSBResultFileName, msg);
	diagResultID++;
}

