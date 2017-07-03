
#include <stdlib.h>
#define _GNU_SOURCE // for warning of implicit declaration of function ‘getline’ on OS 5.8
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "DiagDataTypes.h"
#include "DbgCfg.h"
#include "Print.h"
#include "tools.h"
#include "MplStartup.h"
//#include "McmsApi.h"
//#include "general.h"      

extern ssize_t getline (char **lineptr, size_t *n, FILE *stream);

extern UINT32 unLoggerConnectionStatus;





INT8 mfaDbgInfoString[MFA_DBG__MAX_ENTRIES][30] ={
	"IGNORE_THIS_FILE",
	"RELEASE_DSP",
	
	"CM_INITIATOR_PRINT_LEVEL", 
	"DIAG_PRINT_LEVEL", 
	"DISPATCH_PRINT_LEVEL", 
	"E2PROM_PRINT_LEVEL", 
	"IPMI_PRINT_LEVEL",
	"SHELF_COM_PRINT_LEVEL",
	"LAN_SWITCH_PRINT_LEVEL",
	"MCMS_COM_PRINT_LEVEL",
	"NTP_PRINT_LEVEL",
	"SHARED_PRINT_LEVEL",
	"STARTUP_PRINT_LEVEL",
	"TCP_SERVER_PRINT_LEVEL",
	"USB_PRINT_LEVEL",
	"TIMER_PRINT_LEVEL",
	"OUTPUT_DESTINATION",
	"LOG_FILE_NAME"

};


INT8 cDbgInfoFile[MFA_DBG__MAX_ENTRIES][70] = {
	
	"IGNORE_THIS_FILE=NO",
	"RELEASE_DSP=YES",
	"CM_INITIATOR_PRINT_LEVEL=NORMAL", 
	"DIAG_PRINT_LEVEL=NORMAL", 
	"DISPATCH_PRINT_LEVEL=NORMAL", 
	"E2PROM_PRINT_LEVEL=NORMAL", 
	"IPMI_PRINT_LEVEL=NORMAL",
	"SHELF_COM_PRINT_LEVEL=NORMAL",
	"LAN_SWITCH_PRINT_LEVEL=NORMAL",
	"MCMS_COM_PRINT_LEVEL=NORMAL",
	"NTP_PRINT_LEVEL=NORMAL",
	"SHARED_PRINT_LEVEL=NORMAL",
	"STARTUP_PRINT_LEVEL=NORMAL",
	"TCP_SERVER_PRINT_LEVEL=NORMAL",
	"USB_PRINT_LEVEL=NORMAL",
	"TIMER_PRINT_LEVEL=NORMAL",
	"OUTPUT_DESTINATION=FILE",
	"LOG_FILE_NAME=DiagLog.txt"
	};	



INT8 acPrintLevelStr[4][7]={
	"***",
	"ERROR",
	"MAJOR",
	"MINOR"
};



MFA_DBG_S	tMfaDbgInfo;


extern UINT32 unCmInitiatorPrintLevel;
extern UINT32 unDiagPrintLevel;
extern UINT32 unDispatchPrintLevel;
extern UINT32 unE2promPrintLevel;
extern UINT32 unIpmiPrintLevel;
extern UINT32 unShelfComPrintLevel;
extern UINT32 unLanSwitchPrintLevel;
extern UINT32 unMcmsComPrintLevel;
extern UINT32 unNtpPrintLevel;
extern UINT32 unSharedPrintLevel;
extern UINT32 unStartupPrintLevel;
extern UINT32 unTcpServerPrintLevel;
extern UINT32 unUsbPrintLevel;
extern UINT32 unTimerPrintLevel;

void BackUpLogFile();
UINT32 CopyDbgcfgStructToMem(UINT8* pucBuffer);



/* InitDefaultMfaDbg

** init default values of print levels and output dst

*/

void InitDefaultMfaDbg()
{
	tMfaDbgInfo.bIgnoreDbgCfgFile = EMB_NO;
	tMfaDbgInfo.bReleaseDsp 	  = EMB_YES;	
 	tMfaDbgInfo.ul_OutputDest	  = eLOG_FILE;
	memset(tMfaDbgInfo.ac_LogFileName, 0, MAX_DBG_FILE_PATH_LEN);
	memcpy(tMfaDbgInfo.ac_LogFileName, FILENAME_defaultLogFile, strlen(FILENAME_defaultLogFile));
	
	memset(tMfaDbgInfo.ac_PrevLogFileName, 0, MAX_DBG_FILE_PATH_LEN);
	memcpy(tMfaDbgInfo.ac_PrevLogFileName, FILENAME_defaultPrevLogFile, strlen(FILENAME_defaultPrevLogFile));
	tMfaDbgInfo.logFileHandle	  = NULL;
	tMfaDbgInfo.DiagTerminalFileHandle = NULL;
	tMfaDbgInfo.terminalFileHandle = NULL;
	
	unCmInitiatorPrintLevel = eLevelInfoNormal;
	unDiagPrintLevel        = eLevelInfoNormal;
	unDispatchPrintLevel    = eLevelInfoNormal;
	unE2promPrintLevel      = eLevelInfoNormal;
	unIpmiPrintLevel        = eLevelInfoNormal;
	unLanSwitchPrintLevel   = eLevelInfoNormal;
	unMcmsComPrintLevel     = eLevelInfoNormal;
	unNtpPrintLevel         = eLevelInfoNormal;
	unSharedPrintLevel      = eLevelInfoNormal;
	unStartupPrintLevel     = eLevelInfoNormal;
	unTcpServerPrintLevel   = eLevelInfoNormal;
	unUsbPrintLevel         = eLevelInfoNormal;
	unTimerPrintLevel       = eLevelInfoNormal;
	unShelfComPrintLevel    = eLevelInfoNormal;
	
	return;
}





/* ParseMfaDbgCfg
** read dbg cfg file, init print levels and sebug options
*/

void ParseMfaDbgCfg()
{
	INT32 rc = 1;
	INT32 rc_getl, buff_size;
	size_t len = 0;
	FILE *fp;
	INT8 *pInfoBuff, *tmp_buff;

	InitDefaultMfaDbg();

	if(!IsDirFileExist(PATHNAME_defaultLogPath))
	{
		if(0 != mkdir(PATHNAME_defaultLogPath, 0777))
		{
			printf("ParseMfaDbgCfg mkdir failed: %s errno: %d", PATHNAME_defaultLogPath, errno);
		}
	}
		
	printf("Parse dbgCfg file\n");

	pInfoBuff = 0;
	tmp_buff = 0;

	fp = fopen(FILENAME_dbgCfg, "r");
	if(fp == NULL)
	{
		printf("ERROR: dbgCfg file DOES NOT EXIST\n");
		goto ParseMfaDbgCfg_end;
	}
	
	pInfoBuff = (INT8*)malloc(DBG_FILE_BUFFER_SIZE);
	if(pInfoBuff == 0)
	{
		printf("ERROR: dbgCfg MALLOC FAILED\n");
		goto ParseMfaDbgCfg_end;
	}
	
	memset(pInfoBuff, 0, DBG_FILE_BUFFER_SIZE);	

	//buffer usb.txt locally:
	buff_size = 0;
	tmp_buff = (char*)malloc(200);
	if(tmp_buff == NULL)
	{
		printf("ERROR: sysInfo MALLOC FAILED\n");
		goto ParseMfaDbgCfg_end;
	}
	do
	{					
		rc_getl = getline( &tmp_buff, &len, fp);
		if(rc_getl != -1)
		{
			memcpy( (void*)(pInfoBuff+buff_size), (void*)tmp_buff, rc_getl);
			buff_size += rc_getl;			
		}
	}while(rc_getl != -1);

ParseMfaDbgCfg_end:
	InitMfaDbgInfo(pInfoBuff);  //if file not opened - init will use default
	
	if(fp != NULL)
		fclose(fp);	

	if(pInfoBuff != NULL)
		free(pInfoBuff);

	if(tmp_buff != NULL)
		free(tmp_buff);

	BackUpLogFile();

	//DumpDbgParams();

	return;
}





void InitMfaDbgInfo(INT8 *pInfoBuff)
{
	INT8 *pTmp;
	INT8  value[50];
	UINT32 *pU32;
	UINT8  *pU8;
	FILE   *fp;
  
	printf("InitMfaDbgInfo, pInfoBuff = %p\n", pInfoBuff);

	if(pInfoBuff == NULL)
	{
		/*
		if(tMfaDbgInfo.ul_OutputDest == eLOG_FILE)
		{
			tMfaDbgInfo.logFileHandle = fopen(tMfaDbgInfo.ac_LogFileName, "a+");
			if(tMfaDbgInfo.logFileHandle  == NULL) //unable to open default file
				tMfaDbgInfo.ul_OutputDest = eTERMINAL;
		}	
		return;
		*/
		//need to create the dbgcfg.txt file with default params
		FILE *fSysInfoFile = NULL;
		INT32 uFileWriteSize = 0 , uBufferSize = 0xc00, uReadSize = 0;
		UINT8 *pucSysInfoInMem = NULL;
		
		fSysInfoFile = fopen(FILENAME_dbgCfg,"w");
		if (fSysInfoFile == NULL)
		{
			MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Fail Create dbgcfg.txt file\n");	
			return;
		}
		
		pucSysInfoInMem = (UINT8*)malloc(uBufferSize);
		if (pucSysInfoInMem == NULL)
			return;
			
		uReadSize = CopyDbgcfgStructToMem(pucSysInfoInMem);
		uBufferSize = uBufferSize < uReadSize ? uBufferSize : uReadSize;

		uFileWriteSize = fwrite((void*)(pucSysInfoInMem),sizeof(INT8),uBufferSize ,fSysInfoFile);
		if (uFileWriteSize != uBufferSize)
		{
			MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ERROR: (Create sysinfo.txt) problem writing to sysinfo file: FileWriteSize = %d , buff_size = %d",uFileWriteSize,uBufferSize);	
		}
		
		fclose(fSysInfoFile);
		free(pucSysInfoInMem);
			
		return; 
		
	}

	//// bIgnoreDbgCfgFile
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__IGNORE_DBG_CFG_FILE]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "YES")	)
			tMfaDbgInfo.bIgnoreDbgCfgFile = EMB_YES;
	}

	//if file says to ignore it - stop parsing and use defaults instead:
	if(tMfaDbgInfo.bIgnoreDbgCfgFile == EMB_YES)
		return; 


	//// bReleaseDsp
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__RELEASE_DSP]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "NO")	)
			tMfaDbgInfo.bReleaseDsp = EMB_NO;
	}

	//// ul_OutputDest
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__OUTPUT_DEST]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);

		printf("\n dbg: output dest read from cfg file  - %s\n", value);

		if(! strcmp(value, "TERMINAL") )
			tMfaDbgInfo.ul_OutputDest = eTERMINAL;
		else if(! strcmp(value, "FILE")	)
			tMfaDbgInfo.ul_OutputDest = eLOG_FILE;
		else if(! strcmp(value, "LOGGER")	)
			tMfaDbgInfo.ul_OutputDest = eTCP_LOGGER;
	}

	if( tMfaDbgInfo.ul_OutputDest == eLOG_FILE)
	{
		//// ac_LogFileName
	    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__LOGFILE_NAME]);
		if(pTmp != NULL)
		{
			fp = NULL;
			my_strccpy(value, pTmp);

			if( strcmp(value, "*") != 0	)	//if LOG_FILE_NAME != *, which is the default
			{
				memset(tMfaDbgInfo.ac_LogFileName, 0, MAX_DBG_FILE_PATH_LEN);
				memcpy(tMfaDbgInfo.ac_LogFileName, PATHNAME_defaultLogPath, strlen(PATHNAME_defaultLogPath));
				strcat(tMfaDbgInfo.ac_LogFileName, value); 

				fp = fopen(tMfaDbgInfo.ac_LogFileName, "a+");
				if(fp == NULL)
				{
					printf("DBG_CFG: can't open log file %s. using default.\n", tMfaDbgInfo.ac_LogFileName);
				}
				else
				{
//					memcpy(tMfaDbgInfo.ac_LogFileName, value, strlen(value) );
					tMfaDbgInfo.logFileHandle = fp;
				}
			}

			if(fp == NULL)	//write to default log file
			{	
				memset(tMfaDbgInfo.ac_LogFileName, 0, MAX_DBG_FILE_PATH_LEN);
				memcpy(tMfaDbgInfo.ac_LogFileName, FILENAME_defaultLogFile, strlen(FILENAME_defaultLogFile));				
				fp = fopen(tMfaDbgInfo.ac_LogFileName, "a+");
			}

			if(fp == NULL) //failed opening default file
			{
				printf("DBG_CFG: can't open default log file. printing to terminal. \n");
				tMfaDbgInfo.ul_OutputDest = eTERMINAL;
			}
			else
			{
				tMfaDbgInfo.logFileHandle = fp;	
			}
		}
	}

   ////////////////// INIT PRINT LEVELS:   ///////////////////////////////////////

	//// CM INITIATOR
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_CM_INITIATOR]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unCmInitiatorPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unCmInitiatorPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unCmInitiatorPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unCmInitiatorPrintLevel = eLevelInfoNormal;
	}

	//// IP MEDIA
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_DISPATCH]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unDispatchPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unDispatchPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unDispatchPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unDispatchPrintLevel = eLevelInfoNormal;
	}

	//// MCMS COM
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_E2PROM]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unE2promPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unE2promPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unE2promPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unE2promPrintLevel = eLevelInfoNormal;
	}

	//// MEDIA SWITCH
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_IPMI]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unIpmiPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unIpmiPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unIpmiPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unIpmiPrintLevel = eLevelInfoNormal;
	}

	//// SHELF MCMS COM
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_SHELFCOM]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unShelfComPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unShelfComPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unShelfComPrintLevel = PRINT_LEVEL_MINOR;
	}

	//// SHARED
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_LAN_SWITCH]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unLanSwitchPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unLanSwitchPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unLanSwitchPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unLanSwitchPrintLevel = eLevelInfoNormal;
	}

	//// STR_TX

    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_MCMS_COM]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unMcmsComPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unMcmsComPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unMcmsComPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unMcmsComPrintLevel = eLevelInfoNormal;
	}

	//// STR_RX
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_NTP]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unNtpPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unNtpPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unNtpPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unNtpPrintLevel = eLevelInfoNormal;
	}

	//// TB
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_SHARED]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unSharedPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unSharedPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unSharedPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unSharedPrintLevel = eLevelInfoNormal;
	}

	//// IVR
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_STARTUP]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unStartupPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unStartupPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unStartupPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unStartupPrintLevel = eLevelInfoNormal;
	}

	//// LOAD DSP
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_TCP_SERVER]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unTcpServerPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unTcpServerPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unTcpServerPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unTcpServerPrintLevel = eLevelInfoNormal;
	}

	//// STARTUP
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_USB]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unUsbPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unUsbPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unUsbPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unUsbPrintLevel = eLevelInfoNormal;
	}

	//// TIMER
    pTmp = strstr(pInfoBuff, mfaDbgInfoString[MFA_DBG__PRINT_TIMER]);
	if(pTmp != NULL)
	{
		my_strccpy(value, pTmp);
		if(! strcmp(value, "ERROR")	)
			unTimerPrintLevel = PRINT_LEVEL_ERROR;
		else if(! strcmp(value, "MAJOR")	)
			unTimerPrintLevel = PRINT_LEVEL_MAJOR;
		else if(! strcmp(value, "MINOR") )
			unTimerPrintLevel = PRINT_LEVEL_MINOR;
		else if(! strcmp(value, "NORMAL") )
			unTimerPrintLevel = eLevelInfoNormal;
	}
	
	return;
}





void DumpDbgParams()
{
	MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"DumpDbgParams:");

	if(tMfaDbgInfo.bIgnoreDbgCfgFile == EMB_YES)
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"bIgnoreDbgCfgFile = YES");
	else
	{
		if(tMfaDbgInfo.bReleaseDsp == EMB_YES)
			MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"bReleaseDsp = YES");
		else
			MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"bReleaseDsp = NO");

		if(tMfaDbgInfo.ul_OutputDest == eTERMINAL)
			MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"output dest = TERMINAL");
		else if(tMfaDbgInfo.ul_OutputDest == eLOG_FILE)
			MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"output dest = FILE");
		else if(tMfaDbgInfo.ul_OutputDest == eTCP_LOGGER)
			MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"output dest = LOGGER");
		else
			MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ERROR value: output dest = 0x%x", tMfaDbgInfo.ul_OutputDest);

		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ac_LogFileName = %s", tMfaDbgInfo.ac_LogFileName);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"logFileHandle = 0x%x", tMfaDbgInfo.logFileHandle);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Print Levels:");
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CmInitiator = %s", acPrintLevelStr[unCmInitiatorPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Diag =%s", 		acPrintLevelStr[unDiagPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Dispatcher =%s", 	acPrintLevelStr[unDispatchPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"E2prom =%s", 	    acPrintLevelStr[unE2promPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Ipmi =%s", 		acPrintLevelStr[unIpmiPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Shelf Com =%s", 	acPrintLevelStr[unShelfComPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Lan Switch =%s", 	acPrintLevelStr[unLanSwitchPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Mcms Com =%s", 	acPrintLevelStr[unMcmsComPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Ntp =%s", 			acPrintLevelStr[unNtpPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Shared =%s", 		acPrintLevelStr[unSharedPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Startup =%s", 		acPrintLevelStr[unStartupPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Tcp Server =%s", 	acPrintLevelStr[unTcpServerPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Usb =%s", 		    acPrintLevelStr[unUsbPrintLevel]);
		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"Timer =%s", 		acPrintLevelStr[unTimerPrintLevel]);
	}

	return;
}



void BackUpLogFile()
{
	INT8 systemString[1000];
	
	if (tMfaDbgInfo.logFileHandle)
	{
		//close the file
		
		printf("\nCopyLogFile\n");
		
		fclose(tMfaDbgInfo.logFileHandle);
		
		chdir("/tmp");		
		memset((void*)&systemString[0],0, 1000);
			
		//rm old prevFile:
		strcpy(systemString,"rm ");
		strcat(systemString,tMfaDbgInfo.ac_PrevLogFileName);
		system(systemString);

		//copy the logger to prev one
		memset((void*)&systemString[0],0, 1000);

		strcpy(systemString,"mv "); //l.a. "cp ");
		strcat(systemString,tMfaDbgInfo.ac_LogFileName);
		strcat(systemString," ");
		strcat(systemString,tMfaDbgInfo.ac_PrevLogFileName);
		system(systemString);
	
//		printf("\nsystemString = %s\n",systemString);
//		system("cp MfaLog.txt MfaLogPrev.txt");	

		chdir("/mcms/Bin");	//chdir("/mnt/mfa_cm_fs");	
		//open the file
		tMfaDbgInfo.logFileHandle = fopen(tMfaDbgInfo.ac_LogFileName, "w");
	}
}

UINT32 CalcLineSize(INT8* pcLine)
{
	UINT32 i = 0;
	
	while (pcLine[i] != '\0')
	{
		i++;
	}
	
	i++;
	
	return i;
}

UINT32 CopyDbgcfgStructToMem(UINT8* pucBuffer)
{
	UINT32 i , unStringSize = 0 , unTotalSize = 0;
	
	for (i = 0 ; i < (MFA_DBG__MAX_ENTRIES); i++)
	{
		unStringSize = CalcLineSize((INT8*)&cDbgInfoFile[i]);
		
		memcpy(pucBuffer,cDbgInfoFile[i],unStringSize);
		
		pucBuffer += unStringSize-1;
		
		*pucBuffer = 0x0d;
		pucBuffer++;		

		*pucBuffer = 0x0a;
		pucBuffer++;		
		
		unTotalSize += unStringSize + 1;
	}	
	
	return (unTotalSize);
}

UINT32 HandlePrintLevelChange(UINT8 *pModule, UINT8 *pLevel)
{
	UINT32 rc = 0;
	UINT32 ulLevel, ulPrintMask;
	UINT32 *pulModule;

	if( pModule == 0)
	{
		rc = 0xFF;
		goto exit_HandlePrintLevelChange;
	}

	if(pLevel == 0)
	{		
		if(! strcasecmp(pModule, "normal")	)
		{
			MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"HandlePrintLevelChange: setting all print levels to NORMAL.");
			InitDefaultMfaDbg();
		}

		else if(! strcasecmp(pModule, "Help")	)
			rc = 1;
		else
			rc = 0xFF;

		goto exit_HandlePrintLevelChange;
	}
	else if(!strcasecmp(pModule, "dest") )
	{
		if(!strcasecmp(pLevel, "Logger") )
		{
			printf("\n\n*** Redirecting printouts to Logger. ***\n");
	 		tMfaDbgInfo.ul_OutputDest	  = eTCP_LOGGER;
		}
		else if(!strcasecmp(pLevel, "Terminal") ) 
		{
			printf("\n\n*** Redirecting printouts to Terminal. ***\n");
	 		tMfaDbgInfo.ul_OutputDest	  = eTERMINAL;  
		}
		else
		{
			rc = 0xFF;
	 	 	goto exit_HandlePrintLevelChange;
		}
	}
	else
	{
		 //get print level value: 

		 if(! strcasecmp(pLevel, "CRITICAL")	)
			ulLevel = eLevelError; 
		 else if(! strcasecmp(pLevel, "API")	)
			ulLevel = eLevelWarn; 
		 else if(! strcasecmp(pLevel, "NORMAL")	)
			ulLevel = eLevelInfoNormal; 
		 else if(! strcasecmp(pLevel, "LOW")	)
			ulLevel = eLevelDebug; 
	 	 else if(! strcasecmp(pLevel, "DEBUG")	)
			ulLevel = eLevelTrace; 
	 	 else 
	 	 {
			rc = 0xFF;
	 	 	goto exit_HandlePrintLevelChange;
	 	 }
	 	 
	 	 //get module: 
		 if(! strcasecmp(pModule, "CmInit")	)
		 {
			pulModule = &unCmInitiatorPrintLevel;
			ulPrintMask = CM_INITIATOR_PRINT;
		 }
		 else if(! strcasecmp(pModule, "Diag")	)
		 {
			pulModule = &unDiagPrintLevel; 
			ulPrintMask = DIAG_PRINT;
		 }
		 else if(! strcasecmp(pModule, "Dispatch")	)
		 {
			pulModule = &unDispatchPrintLevel; 
			ulPrintMask = DISPATCH_COM_PRINT;
		 }
		 else if(! strcasecmp(pModule, "E2prom")	)
		 {
			pulModule = &unE2promPrintLevel; 
			ulPrintMask = E2PROM_PRINT;
		 }
		 else if(! strcasecmp(pModule, "ipmi")	)
		 {
			pulModule = &unIpmiPrintLevel; 
			ulPrintMask = IPMI_PRINT; 
		 }
		 else if(! strcasecmp(pModule, "LanSwitch")	)
		 {
			pulModule = &unLanSwitchPrintLevel; 
			ulPrintMask = LAN_SWITCH_PRINT;
		 }
		 else if(! strcasecmp(pModule, "McmsCom")	)
		 {
			pulModule = &unMcmsComPrintLevel; 
			ulPrintMask = MCMS_COM_PRINT;
		 }
		 else if(! strcasecmp(pModule, "NTP") )
		 {
			pulModule = &unNtpPrintLevel; 
			ulPrintMask = NTP_PRINT;
		 }
		 else if(! strcasecmp(pModule, "Shared")	)
		 {
			pulModule = &unSharedPrintLevel; 
			ulPrintMask = SHARED_PRINT;
		 }
		 else if(! strcasecmp(pModule, "Startup")	)
		 {
			pulModule = &unStartupPrintLevel; 
			ulPrintMask = STARTUP_PRINT;
		 }
		 else if(! strcasecmp(pModule, "TcpServer")	)
		 {
			pulModule = &unTcpServerPrintLevel; 
			ulPrintMask = TCP_SERVER_PRINT;
		 }
		 else if(! strcasecmp(pModule, "USB")	)
		 {
			pulModule = &unUsbPrintLevel; 
			ulPrintMask = USB_PRINT;
		 }
		 else if(! strcasecmp(pModule, "Timer")	)
		 {
			pulModule = &unTimerPrintLevel; 
			ulPrintMask = TIMER_PRINT;
		 }
		 else if(! strcasecmp(pModule, "Shelf")	)
		 {
			pulModule = &unShelfComPrintLevel; 
			ulPrintMask = SHELFCOM_PRINT;
		 }
		 else
	 	 {
			rc = 0xFF;
	 	 	goto exit_HandlePrintLevelChange;
	 	 }		  

		 *pulModule = ulLevel;
		 MfaBoardPrint(ulPrintMask, ulLevel, PRINT_TO_TERMINAL,"HandlePrintLevelChange: changed print level of module %s to %d. printMask is 0x%x.", pModule, ulLevel, ulPrintMask);
	}

exit_HandlePrintLevelChange:

	if(rc != 0)
	{
		if(rc == 0xFF) //error found. print error message, and then the help menu
			MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"HandlePrintLevelChange ERROR: illegal params. pModule=%s, pLevel=%s. ignoring.", pModule, pLevel);

		MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"HELP - printLevel format:\n \
					   1.printLevel dest <destination> (set the destination of printouts:)\n \
					   		<destination> = Logger / Terminal \n\
					   2. printLevel normal (set all print levels to normal) \n\
					   3. printLevel <module> <level> (set specific print level:)\n \
							<module> = CmInit / Diag / Dispatch / E2prom / ipmi / LanSwitch / McmsCom / NTP / Shared / Startup / TcpServer/ USB / Timer / Shelf \n \
							<level> = CRITICAL / API / NORMAL / LOW / DEBUG");

	}

	return(0);
}




void OpenSwitchLogFileInTmp()
{
	FILE *fp;
	
	memset(tMfaDbgInfo.ac_LogFileName, 0, MAX_DBG_FILE_PATH_LEN);
	memcpy(tMfaDbgInfo.ac_LogFileName, PATHNAME_defaultLogPath, strlen(PATHNAME_defaultLogPath));
	strcat(tMfaDbgInfo.ac_LogFileName, "DiagLog.txt"); 

	fp = fopen(tMfaDbgInfo.ac_LogFileName, "a+");
	if(fp == NULL)
	{
		printf("DBG_CFG: can't open log file %s. using default.\n", tMfaDbgInfo.ac_LogFileName);
	}
	else
	{
//					memcpy(tMfaDbgInfo.ac_LogFileName, value, strlen(value) );
		tMfaDbgInfo.logFileHandle = fp;
	}	
}

