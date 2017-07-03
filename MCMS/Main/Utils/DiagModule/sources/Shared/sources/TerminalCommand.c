#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "terminalCommand.h"
//#include "MsCommon.h"
#include "signal.h"
#include "EmaApi.h"
#include "DiagnosticsShared.h"
#include "SystemInfo.h"

extern APIU32 SystemGetTickCount();

//#include "MsFunctions.h"
//#include "Recording.h"
//#include "keepalive.h"
//#include "MsHandler.h"

//void PrintTerminalCommand(INT8 *pcFormat, ...);
//void SendTerminalCommandToMcms(TMessageThreadType tMessageThreadType);
//void TerminalCommandPrintFunction(FILE *fFIleDescriptor,INT8 *pcFormat, ...);

void FillTerminalMsgHeaders(UINT32 id, UINT8* pContent, UINT32 ulBufferSize);
void FillCommonHeaderTerminalCommand(COMMON_HEADER_S *pCommonHeader);
void FillTraceHeaderTerminalCommand(TRACE_HEADER_S *pTraceHeader, UINT32 id, UINT32 ulBufferSize);
//void SendSSHModeMessageToMpl(APIUBOOL isOnRequest);

void updtFilterProtectFlag(INT32 flag);
INT32 readFilterProtectFlag();

//yigal remove later
extern void SetDspMode(UINT32 unDspNum , UINT32 unMode);
extern void SetDspState(UINT32 unDspNum , UINT32 unDspState);
extern void DownloadDspBackupDatabase(UINT32 unDspNum);

extern void LSCreateLog(char * path,char * command1,char * command2,char * command3,char * command4,char * command5);

extern void StartDspRecovery(UINT32 unDspNum);
extern void h2_set_mirror_port (UINT8 port_no);
extern void h2_set_mirror_source (UINT8 port_no, UINT8 enable);
extern void terminalShowPortDataBase(UINT32 unDSPNum , UINT32 unPortNum , UINT32 terminal);
extern void TriggerAllPiChips(void);
extern INT32 SendSysCall(INT8 *pCmdString,UINT8 waitOnMsg);


extern MFA_DBG_S	tMfaDbgInfo;
extern MFA_DBG_S	tMfaDbgInfo;
//extern UINT32 ulDebugMode;
//pthread_t EmaSimulationListenThreadId;
extern void EmaSimulationListenThread();

UINT32 unTerminalMsgCnt	= 0;
char* terminalName = NULL;
//----------------------
//PI/PCI info globals
BOOL isShowPciInfo = 0;
UINT32 pciInfoOutputFlag = 1;
APIUBOOL	isSSH_ON = FALSE;

extern int RegisterExceptions();
extern int UnRegisterExceptions();
extern APIUBOOL	isCoreDumpMode;


extern void ShovalLinkTest();


#if 0//NBS need to change
//---------------------------
void SendTerminalNameToDiag(char *terminalName)
{
	TEmaDebug pmsg;
	sprintf(pmsg.ulMsgID,"%s",terminalName);

	pmsg.ulOpcode = PRIVATE_OPCODE;

	t_TcpConnParams*	ptTcpCon = &TcpConnection[eSwitchDiagServer]; // Send message to Switch Diag


	if ( TCPSendData( ptTcpCon->s, (VOID *)&pmsg, sizeof(TEmaDebug),0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(StringStripper): problem in sending data...\n");
		ptTcpCon->ul_ConnectionStatus = NOT_CONNECTED;
		ptTcpCon->ul_PrevConnState = CONNECTED;

		// TBD: Find out if there is any need to send failure notice to EMA.
	}
}
#endif
void HandleTerminalCommand(UINT8* pMsg)
{
#if 0
	FILE* fd;
	char* pcToken;
	char* commandArgs[MAX_NUM_OF_PARAM + 4]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	DWORD i         = 0;
	DWORD ArgsNum   = 0;
	DWORD paramsNum = 0;
	char * test;
	char* argc = NULL;
	char* commandName = NULL;
	char* destination;
	char* param[MAX_NUM_OF_PARAM];
	char help0[80]  = "Here are all possible commands For the Switch:\n\r";
	char help1[80]  = "1.  BurnUboot - burn the uboot\n\r";
	char help2[80]  = "2.  vitregs All - prints all switch registers\n\r";
	char help3[80]  = "3.  vitregs phy - prints all switch phy registers\n\r";
	char help4[80]  = "4.  vitregs VlanRegs - prints all switch vlan registers\n\r";
	char help5[80]  = "5.  vitregs RmonCouters \n\r";
	char help6[80]  = "6.  vitregs h2_mac_regs - mac registers \n\r";
	char help7[80]  = "7.  vitregs general - print general registers\n\r";
	char help8[80]  = "8.  vitregs info - prints you the name of the registers\n\r";
	char help9[80]  = "9.  vitregs phy/h2_mac_regs/VlanRegs  port_num - prints all register for";
	char help10[80] = "10. phy/h2_mac_regs/VlanRegs for specified port\n\r";
	char help11[80] = "11. vitregs VlanRegs  port_num reg_name -";
	char help12[80] = "12. print a specified register on a specified port\n\r";
	char help13[80] = "13. vitregs phy/h2_mac_regs  port_num reg_num -";
	char help14[80] = "14. print a specified register on a specified port\n\r";
	char help15[80] = "15. vitregs phy_write port_num reg_num value (num in hex)\n\r";
	char help16[80] = "16. vitregs WriteReg block ( 1-7) sub-block address value(num in hex)\n\r";
	char help17[80] = "17. dbgMode param1 - changes debug mode NO/YES\n\r";
	char help18[80] = "18. dbgMode without parameter - returns the current value of dbgMode\n\r";
	char help19[80] = "19. printLevel - changes the print levels\n\r";
	char help20[80] = "20. ThreadList\n\r";
	char help21[80] = "21. GenerateSig\n\r";
	char help22[80] = "22. vitregs ReadVlanEntry vlanId Warning ALL NUMBERS MUST BE IN DECIMAL!!!\n\r";
	char help23[80] = "23. EmaSim - open simulation port\n\r";
	char help24[80] = "24. DirectPrintToTerminal\n\r";
	char help25[80] = "25. DirectPrintTo param1 (param1 = TERMINAL/LOG_FILE/TCP_LOGGER)\n\r";
	char help26[80] = "26. PQ_Mem_Test\n\r";
	char help27[80] = "27. SSHMode (param1) (param1 = ON/OFF)\n\r";
	char help28[80] = "28. TelnetMode param1 (param1 = OPEN/CLOSE)\n\r";
	char help29[80] = "29. RegisterExceptions param1 ( param1 = YES/NO )\n\r";

	char help30[80] = "30. WriteReg param1 param2 (param1 = Address , param2 = Data)\n\r";
	char help31[80] = "31. ReadReg param1 (param1 = Address)\n\r";
	//ascii message to the IPMC
	char help45[80] = "45. GetE2PromData\n\r";

	char help46[80] = "46. GetWatchDog\n\r";
	char help47[200] = "47. SetWatchDog param1 (param1 = interval in sec , 1 - turn off , 0 turn on)\n\r";
	char help48[80] = "48. GetIpmcVersion\n\r";
	char help49[80] = "49. GetFruState\n\r";
	char help50[80] = "50. SetFruState\n\r";

	char help51[80] = "51. GetCardSlotId\n\r";

	char help52[80] = "52. TestCardLeds param1 (param1 - 1 = on (start) , 0 = off (stop))\n\r";
	char help53[200] = "53. GetCardLeds param1 (param1 - 0 = blue , 1 = red , 2 = green , 3 = amber)\n\r";
	char help54[200] = "54. SetCardLeds param1 param2 (param1 - 2 = red , 3 = green , 4 = amber , param2 - 0 = Off , 1 = On , 2 = Blink)\n\r";

	char help55[80] = "55. GetHotSwapHandels\n\r";
	char help56[80] = "56. SetHotSwapHandels param1 (param1 - 1 = on (ignore) , 0 = off)\n\r";

	char help57[80] = "57. SetFanSpeed\n\r";
	char help58[80] = "58. GetFanSpeed\n\r";

	char help59[80] = "59. GetPowerForceOn\n\r";
	char help60[80] = "60. SetPowerForceOn param1 (param1 - 0 = off , 1 = on)\n\r";
	char help61[80] = "61. ChgHttpd param1 (param1 - 0\n\r";
    char help62[80] = "62. ClosePermanent\n\r";
    char help63[80] = "63. SetJITCmode [0,1]\n\r";
    char help64[80] = "64. GetJITCmode\n\r";

	char help70[80] = "70. SetFltrProtect (param1 - 0=off ,1=on(powerDown for MPM_P)\n\r";
    char help71[200] = "71. EnableMirrorPort (param1 - source port, param2 - mirror port *** ports: Mfa upper = 6, Mfa Lower =3, External Lan 1 = 0, External Lan 2 = 1)\n\r";
    char help72[200] = "72. SendAuthToMcms \n\r";

    char help73[80] = "73. sendAlarm [add,rm] [isfault] \n\r";
	char help74[80] = "74. GetSeparatedNetworksParams \n\r";

    char help75[80] = "75. GetSeparatedNetworkMode - from flash \n\r";
	char help76[200] = "76. SetSeparatedNetworkMode param1 (param1 - 0 = off , 1 = on)- on flash and sysInfo\n\r";
	
	char help80[200] = "80. Get Shoval Link Status\n\r";

	INT32 rc;

	DWORD sum = 0;

	if (pMsg == NULL)
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TerminalCommand Error pMsg == NULL");
		return;
	}
	for (i = 0; i < MAX_NUM_OF_PARAM; i++)
	{
		param[i] = NULL;
	}

	i = 0;
	pcToken = (char *)strtok(pMsg," ");
	while(pcToken != NULL)
	{
	   commandArgs[i] = pcToken;
	   pcToken = (char *)strtok(NULL," ");
	   i++;
	}

	ArgsNum = i;

	argc		 = commandArgs[0];
	terminalName = commandArgs[1];
	commandName  = commandArgs[2];
	//destination  = commandArgs[3];

	if ( (argc == NULL) || (terminalName == NULL) || (commandName == NULL) )
		return;

	for (i = 0 ; commandArgs[i + 3] != NULL ; i++)
	{
		param[i] = commandArgs[i + 3];
	}

	paramsNum = i + 1;
	if (!strcmp(argc,"xxx"))// && strcmp(commandName,"DiagPrintToTerminal"))
	{
		fd =fopen(terminalName,"w");
		if (NULL == fd)
		{
			MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TerminalCommand Error fd == NULL");
			return;
		}
	}

    if (!strcmp(commandName,"BurnUboot") || !strcmp(commandName,"1"))
	{
           INT32 rc;
           rc = system("cp /mnt/mfa_cm_fs/mcms_share/bin/u-bootSwitch.bin /tmp");
           if (rc != 0)
             	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ERROR unable to copy file from mcms - Switch uboot burning failed\n");
           else
           {
             MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"copying u-boot file from mcms\n");
             if (!strcmp(argc,"xxx"))
			 {
				fprintf(fd, "updating Switch u-boot - please wait !!!\n");
			 }
			 else
			 {
             	printf("updating Switch u-boot - please wait !!!\n");
			 }
             rc = system("/sbin/flash_erase /dev/mtd/3");
             rc = system("/sbin/flashcp /tmp/u-bootSwitch.bin /dev/mtd/3");
             if (!strcmp(argc,"xxx"))
             {
				fprintf(fd, "updating Switch u-boot - please wait !!!\n");
			 }
			 else
			 {
             	printf("Switch u-boot updated :-)\n");
			 }
             if (rc!= -1)
               MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"u-boot version updated \n");
           }
	}
	else if (!strcmp(commandName, "DiagPrintToTerminal"))
	{
		//fprintf(fd,"Send Terminal Command to Diagnostics %d",(UINT32)terminalName);
        // Send terminal name to the Diag
        //fclose(fd);
//		SendTerminalNameToDiag(terminalName);//NBS
		return;
	}
	else if (!strcmp(commandName,"vitregs"))
	{
		LSCreateLog(terminalName,param[0],param[1],param[2],param[3],param[4]);
	}
	else if (!strcmp(commandName,"SEG_FAULT"))
	{
		*test = 'a';
	}
	else if (!strcmp(commandName,"printLevel")|| !strcmp(commandName,"19")) //change print levels
	{
		if (NULL != param[0] && NULL != param[1])
		{
			HandlePrintLevelChange(param[0], param[1]);
		}
	}
	else if (!strcmp(commandName, "dbgMode") || !strcmp(commandName,"17") || !strcmp(commandName,"18")) //change debug mode
	{
		if(NULL == param[0])
		{
			if (!strcmp(argc,"xxx"))
			{
    			fprintf(fd,"DebugMode: %d. (0 - NO; 1 - YES).\n\r", GetDbgModeVal() ); //ulDebugMode);
			}
		}
		else if(! strcasecmp(param[0], "YES") )
		{
	    	if (!strcmp(argc,"xxx"))
    			fprintf(fd,"ChangeDebugMode: changing debugMode to YES.\n\r");
			else
    			PrintTerminalCommand("ChangeDebugMode: changing debugMode to YES.\n\r");

			ChangeDebugMode("YES");
		}
		else if(! strcasecmp(param[0], "NO") )
		{
	    	if (!strcmp(argc,"xxx"))
    			fprintf(fd,"ChangeDebugMode: changing debugMode to NO.\n\r");
			else
    			PrintTerminalCommand("ChangeDebugMode: changing debugMode to NO.\n\r");

			ChangeDebugMode("NO");
		}

		else
		{
	    	MfaBoardPrint(MCMS_COM_PRINT, PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ChangeDebugMode: illegal value %s, ignoring", param[0]);
	    	if (!strcmp(argc,"xxx"))
    		{
	    		fprintf(fd,"ChangeDebugMode: illegal value %s, ignoring\n\r", param[0]);
    		}
		}
	}

	else if (!strcmp(commandName, "ThreadList") || !strcmp(commandName,"20")) //print threadId list
	{
		UINT32 i;

		if (!strcmp(argc,"xxx"))
		{
			fprintf(fd,"\ndbg : printing threadsList\n");
			for(i=0; i<eMaxThread; i++)
				fprintf(fd, " %d. %s : id = %d, priority = %d\n", i, tThreadDesc[i].c_ThreadName, tThreadDesc[i].ul_ThreadId,  tThreadDesc[i].l_ThreadPriority);

		    fprintf(fd, "\n\n");
		}
		else
		{
			for(i=0; i<eMaxThread; i++)
				MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, " %d. %s : id = %d, priority = %d", i, tThreadDesc[i].c_ThreadName, tThreadDesc[i].ul_ThreadId,  tThreadDesc[i].l_ThreadPriority);
		}
	}
	else if (!strcmp(commandName, "GenerateSig") || !strcmp(commandName,"21"))
	{
		unsigned int sig, pid;
		union sigval qval;
		if ( NULL != param[0])
		{
			sig = atol(param[0]);
			pid = getpid();
			if (!strcmp(argc,"xxx"))
			{
				fprintf(fd,"\ndbg: sending signal %d to process %d", sig, pid);
			}
			else
			{
				printf("\ndbg: sending signal %d to process %d", sig, pid);
			}
			sigqueue(pid, sig, qval);
		}
	}
	else if (!strcmp(commandName, "EmaSim") || !strcmp(commandName,"23"))
	{
		fprintf(fd,"\ndbg : open Ema simulation port\n");
		EmaSimulationListenThreadId  = CreateThread(EmaSimulationListenThread,2);
		printf("(Terminal command): EmaSimulationListenThreadId %d",EmaSimulationListenThreadId);

	}
	else if (!strcmp(commandName, "DirectPrintToTerminal")|| !strcmp(commandName,"24"))
	{
		if(tMfaDbgInfo.terminalFileHandle )
		{
			fclose(tMfaDbgInfo.terminalFileHandle );
			tMfaDbgInfo.terminalFileHandle = NULL;
		}
		tMfaDbgInfo.ul_OutputDest = eSPECIFIED_TERMINAL;
		tMfaDbgInfo.terminalFileHandle =fd;
		return;
	}
	else if (!strcmp(commandName, "DirectPrintTo")|| !strcmp(commandName,"25"))
	{
		if(NULL != param[0] )
		{
			if (!strcmp(param[0], "TERMINAL"))
			{
				tMfaDbgInfo.ul_OutputDest = eTERMINAL;
			}
			else if (!strcmp(param[0], "LOG_FILE"))
			{
				tMfaDbgInfo.ul_OutputDest = eLOG_FILE;
			}
			else if (!strcmp(param[0], "TCP_LOGGER"))
			{
				tMfaDbgInfo.ul_OutputDest = eTCP_LOGGER;
			}
			if(tMfaDbgInfo.terminalFileHandle )
			{
				fclose(tMfaDbgInfo.terminalFileHandle );
				tMfaDbgInfo.terminalFileHandle = NULL;
			}

		}
	}
	else if ( (!strcmp(commandName, "PQ_Mem_Test")) || (!strcmp(commandName, "26")) )
	{
		fprintf(fd,"\ndbg : Start PQ Mem Test\n");
		printf("(Terminal command): Start PQ Mem Test");
		Memtester(10, 1);
	}
	else if(!strcmp(commandName, "SSHMode") || !strcmp(commandName, "27"))
	{
		if(NULL != param[0] )
		{
			if (!strcmp(param[0], "ON"))
			{
				APIUBOOL isOnRequest = 1;
				//
	    		/*if (mfaInfoStruct.MngInfo.isSecured != YES)
				{
					SendSysCall ("dropbear -g -d /etc/SecretKey", 0);
					isSSH_ON = TRUE;
				}*/
				if (!strcmp(argc,"xxx"))
				{
					fprintf(fd,"Turned ON SSH\n");
				}
				else
				{
					PrintTerminalCommand("Turned ON SSH\n");
				}
				SendSSHModeMessageToMpl(isOnRequest);
			}
			if (!strcmp(param[0], "OFF"))
			{
				APIUBOOL isOnRequest = 0;
				// Turn OFF SSH
				SendSysCall ("killall -9 dropbear 2>dev/null", 0);

				if (!strcmp(argc,"xxx"))
				{
					fprintf(fd,"Turned OFF SSH\n");
				}
				else
				{
					PrintTerminalCommand("Turned OFF SSH\n");
				}
				isSSH_ON = FALSE;
				SendSSHModeMessageToMpl(isOnRequest);
			}
		}
		else
		{
			if (isSSH_ON)
			{
				if (!strcmp(argc,"xxx"))
				{
					fprintf(fd,"SSH is turned ON\n");
				}
				else
				{
					PrintTerminalCommand("SSH is turned ON\n");
				}
			}
			else
			{
				if (!strcmp(argc,"xxx"))
				{
					fprintf(fd,"SSH is turned OFF\n");
				}
				else
				{
					PrintTerminalCommand("SSH is turned OFF\n");
				}
			}
		}
	}
	else if(!strcmp(commandName, "TelnetMODE") || !strcmp(commandName, "28"))
	{
		// Open telnet
		if(NULL != param[0] )
		{
			if (!strcmp(param[0], "OPEN"))
			{
				// Open telnet
				SendSysCall ("/usr/sbin/telnetd", 0);

				if (!strcmp(argc,"xxx"))
				{
					fprintf(fd,"Telnet is opened\n");
				}
				else
				{
					PrintTerminalCommand("Telnet is opened\n");
				}
			}
			if (!strcmp(param[0], "CLOSE"))
			{
				// Close telnet
				SendSysCall ("killall -9 telnetd", 0);

				if (!strcmp(argc,"xxx"))
				{
					fprintf(fd,"Telnet is closed\n");
				}
				else
				{
					PrintTerminalCommand("Telnet is closed\n");
				}
			}
		}
	}

	else if(!strcmp(commandName, "RegisterExceptions") || !strcmp(commandName, "29"))
	{
		if(NULL != param[0] )
		{
			if (!strcmp(param[0], "YES"))
			{
				if(isCoreDumpMode)
				{
					// Change from coredump to ecxeption handler
					RegisterExceptions(); // Exception handler
					if (!strcmp(argc,"xxx"))
					{
						fprintf(fd,"Register in the exception handler\n");
					}
					else
					{
						PrintTerminalCommand("Register in the exception handler\n");
					}
				}
				else
				{
					if (!strcmp(argc,"xxx"))
					{
						fprintf(fd,"Already registered in the exception handler\n");
					}
					else
					{
						PrintTerminalCommand("Already registered in the exception handler\n");
					}
				}
			}
			else
			{
				if(!isCoreDumpMode)
				{
					// Change from exception handler to coredump
					UnRegisterExceptions(); // Create core dump
					if (!strcmp(argc,"xxx"))
					{
						fprintf(fd,"Change to coredump mode\n");
					}
					else
					{
						PrintTerminalCommand("Change to coredump mode\n");
					}
				}
				else
				{
					if (!strcmp(argc,"xxx"))
					{
						fprintf(fd,"Already in coredump mode\n");
					}
					else
					{
						PrintTerminalCommand("Already in coredump mode\n");
					}
				}
			}
		}
		// Print the current status
		else
		{
			if (isCoreDumpMode)
			{
				if (!strcmp(argc,"xxx"))
				{
					fprintf(fd,"Coredump mode\n");
				}
				else
				{
					PrintTerminalCommand("Coredump mode\n");
				}
			}
			else
			{
				if (!strcmp(argc,"xxx"))
				{
					fprintf(fd,"Exception handler mode\n");
				}
				else
				{
					PrintTerminalCommand("Exception handler mode\n");
				}
			}
		}
	}
	else if(!strcmp(commandName, "WriteReg") || !strcmp(commandName, "30"))
	{
		UINT16 unAddress = 0 , unData = 0;

		if(pQMainPalBaseAddr != 0)
		{
			if (param[0] != NULL)
			{
				unAddress = atol(param[0]);
				if (param[1] != NULL)
				{
					unData = atol(param[1]);
					(*(UINT16 *)(pQMainPalBaseAddr + SHVL_BASE_OFFSET + unAddress)) = unData;
					TerminalCommandPrintFunction(fd,"\nWriteReg:\n Reg address = 0x%x, data = 0x%x\n", (pQMainPalBaseAddr + SHVL_BASE_OFFSET + unAddress), unData);
				}
				else
				{
					TerminalCommandPrintFunction(fd,"\nWriteReg: \nPlease insert the register data\n");
				}
			}
			else
			{
				TerminalCommandPrintFunction(fd,"\nWriteReg:\n Please insert the register address\n");
			}
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nWriteReg: \nFPGA is not ready yet. Please wait\n");
		}
	}
	else if(!strcmp(commandName, "ReadReg") || !strcmp(commandName, "31"))
	{
		UINT16 unAddress = 0 , unData = 0;


		if(pQMainPalBaseAddr != 0)
		{
			if (param[0] != NULL)
			{
				unAddress = atol(param[0]);
				unData = (*(UINT16 *)(pQMainPalBaseAddr + SHVL_BASE_OFFSET + unAddress));
				TerminalCommandPrintFunction(fd,"\nReadReg:\n pQMainPalBaseAddr = %x , SHVL_BASE_OFFSET = %x , unAddress = %d , Reg address = 0x%x, data = 0x%x\n",pQMainPalBaseAddr , SHVL_BASE_OFFSET , unAddress , (pQMainPalBaseAddr + SHVL_BASE_OFFSET + unAddress), unData);
			}
			else
			{
				TerminalCommandPrintFunction(fd,"\nReadReg:\n Please insert the register address\n");
			}
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nWriteReg:\n FPGA is not ready yet. Please wait\n");
		}
	}
	else if (!strcmp(commandName, "GetE2PromData")|| !strcmp(commandName,"45"))
	{/*
		//check if the wd is on if yes then set to off
		if( GetDbgModeVal() == 0)
		{
			//set the wd to off
	//		rc = SetWatchDogFunc(0xff);
			if (rc == 0)
			{
				TerminalCommandPrintFunction(fd,"\nGetE2PromData Set WatchDog Off\n");
			}
			else
			{
				TerminalCommandPrintFunction(fd,"\nGetE2PromData Fail Set WatchDog Off\n");	
			}	
		}
		
		EmbSleep(1);
		
		rc = GetE2promInformation(0);
		
		if (rc == 0)
		{
			if (!strcmp(argc,"xxx"))
			{
				printe2promdata(fd,0,0);
			}
			else
			{
				printe2promdata(NULL,MAX_PRINT_TO_MCMS_TERMINAL_SIZE,0);
			}
			
			FreeE2PromData(tTE2promGeneralDataNotSaved);	
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nFail Read E2prom Data\n");
		}
		
		if( GetDbgModeVal() == 0)
		{
			//set the wd to on
		//	rc = SetWatchDogFunc(300*10);
			if (rc == 0)
			{
				TerminalCommandPrintFunction(fd,"\nGetE2PromData Set WatchDog On\n");
			}
			else
			{
				TerminalCommandPrintFunction(fd,"\nGetE2PromData Fail Set WatchDog On\n");	
			}
		}
		
		EmbSleep(1);
		*/
	}
	else if (!strcmp(commandName, "GetWatchDog")|| !strcmp(commandName,"46"))
	{
		UINT8  ucIsWdOn , ucGetWdAction;
		UINT32 unInitialCounter , unCurrentCounter;		
		
		rc = GetWatchDogFunc(&ucIsWdOn , &ucGetWdAction , &unInitialCounter , &unCurrentCounter);
		if (rc == 0)
		{
			if ( (ucIsWdOn & 0x40) == 0x40)
			{
				TerminalCommandPrintFunction(fd,"\nWatchDog Is On %x\n",ucIsWdOn);	
			}
			else
			{
				TerminalCommandPrintFunction(fd,"\nWatchDog Is Off %x\n",ucIsWdOn);	
			}	
			
			if (ucGetWdAction == 0)
			{
				TerminalCommandPrintFunction(fd,"\nGetWdAction Is No Act\n");	
			}
			else if (ucGetWdAction == 0x01) 
			{
				TerminalCommandPrintFunction(fd,"\nGetWdAction Is HARD RESET\n");	
			}
			else
			{
				TerminalCommandPrintFunction(fd,"\nGetWdAction Is POWER DOWN\n");	
			}
			
			TerminalCommandPrintFunction(fd,"\nInitial Counter = %d\n",unInitialCounter);	
			
			TerminalCommandPrintFunction(fd,"\nCurrent Counter = %d\n",unCurrentCounter);	
		}
	}
	else if (!strcmp(commandName, "SetWatchDog")|| !strcmp(commandName,"47"))
	{
		UINT32 unInterval = 0;
		
		if (NULL != param[0])
		{
			unInterval = atol(param[0]);
			if (unInterval == 0)
			{
		//		rc = SetWatchDogFunc(32);
				if (rc == 0)
				{
					TerminalCommandPrintFunction(fd,"\nSet WatchDog On\n");	
				}
				else
				{
					TerminalCommandPrintFunction(fd,"\nFail Set WatchDog On\n");
				}
			}
			else if (unInterval == 0x01)
			{
		//		rc = SetWatchDogFunc(0xff);
				
				if (rc == 0)
				{
					TerminalCommandPrintFunction(fd,"\nSet WatchDog Off\n");
				}
				else
				{
					TerminalCommandPrintFunction(fd,"\nFail Set WatchDog Off\n");	
				}
			}
			else
			{
	//			rc = SetWatchDogFunc(unInterval);
				
				if (rc == 0)
				{
					TerminalCommandPrintFunction(fd,"\nSet WatchDog To Interval %d\n",unInterval);
				}
				else
				{
					TerminalCommandPrintFunction(fd,"\nFail Set WatchDog To Interval %d\n",unInterval);	
				}	
			}
		}
		else
		{
		//	rc = SetWatchDogFunc(32);
			
			if (rc == 0)
			{
				TerminalCommandPrintFunction(fd,"\nSet WatchDog To Interval 3.2 sec\n");	
			}
		}		
	}
	else if (!strcmp(commandName, "GetIpmcVersion")|| !strcmp(commandName,"48"))
	{
		APIU8 ipmcVerNum[MAX_NUM_OF_SW_VERSION_DIGITS];
		INT32 rc;

		memset(ipmcVerNum,0,MAX_NUM_OF_SW_VERSION_DIGITS);
		
		rc = GetIpmcVersionNumber(ipmcVerNum); //Get the version number from the IPMC	
		if (rc == 0) //success
		{
			TerminalCommandPrintFunction(fd,"IPMC version = %s\n",ipmcVerNum);
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nFailed to get IPMC version rc = %d\n",rc);
		}	
	}
	else if (!strcmp(commandName, "GetFruState")|| !strcmp(commandName,"49"))
	{
		UINT8 ucFruState;
		
		rc = GetFruStateFunc(&ucFruState);
		if (rc == 0)
		{
			TerminalCommandPrintFunction(fd,"\nGetFruState = %d\n",ucFruState);	
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nFail Read Fru State\n");	
		}	
	}
	else if (!strcmp(commandName, "SetFruState")|| !strcmp(commandName,"50"))
	{
		TerminalCommandPrintFunction(fd,"\nSetFruState - not used\n");	
	}
	else if (!strcmp(commandName, "GetCardSlotId")|| !strcmp(commandName,"51"))
	{
		UINT32 ulSlotNum;
		
		ulSlotNum = IpmcGetHWSlotId();  
		
		TerminalCommandPrintFunction(fd,"\nIPMC Slot Id = %d\n",ulSlotNum);	
	}
	else if (!strcmp(commandName, "TestCardLeds")|| !strcmp(commandName,"52"))
	{
		UINT32 ucTestMode = 0;
		
		if (NULL != param[0])
		{
			ucTestMode = atol(param[0]);
			if (ucTestMode == 1)
			{
				rc = LedTestFunc(0x11);
				if (rc == 0)
				{
					TerminalCommandPrintFunction(fd,"\nTestCardLeds Start Test\n");	
				}
				else
				{
					TerminalCommandPrintFunction(fd,"\nFail TestCardLeds Start Test\n");
				}
			}
			else if (ucTestMode == 0)
			{
				rc = LedTestFunc(0);				
				if (rc == 0)
				{
					TerminalCommandPrintFunction(fd,"\nTestCardLeds Stop Test\n");
				}
				else
				{
					TerminalCommandPrintFunction(fd,"\nFail TestCardLeds Stop Test\n");	
				}
			}
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nPlease Set 1 to start or 0 to stop\n");	
		}
	}
	else if (!strcmp(commandName, "GetCardLeds")|| !strcmp(commandName,"53"))
	{
		UINT32 ucLedId = 0;
		UINT8 ucLedStatus;
		
		if (NULL != param[0])
		{
			ucLedId = atol(param[0]);
			if ( (ucLedId == 0) || (ucLedId == 1) || (ucLedId == 2) || (ucLedId == 3) )
			{
				rc = GetLedStateFunc(ucLedId , &ucLedStatus);
				
				if (rc == 0)
				{
					if (ucLedStatus == 0)
						TerminalCommandPrintFunction(fd,"\nLed %d Is Off\n",ucLedId);
					else if (ucLedStatus == 0xff)
						TerminalCommandPrintFunction(fd,"\nLed %d Is On\n",ucLedId);
					else
						TerminalCommandPrintFunction(fd,"\nLed %d Is Blink\n",ucLedId);	
				}
				else
				{
					TerminalCommandPrintFunction(fd,"\nFail Get Led State\n");
				}
			}
			else if (ucLedId == 0)
			{
				TerminalCommandPrintFunction(fd,"\nPlease Insert Led Num: 0 - blue , 1 - red , 2 - green , 3 - amber\n");	
			}
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nPlease Insert Led Num: 0 - blue , 1 - red , 2 - green , 3 - amber\n");	
		}
	}
	else if (!strcmp(commandName, "SetCardLeds")|| !strcmp(commandName,"54"))
	{/*
		UINT32 ucLedId = 0 , unLedAction = 0;
				
		if (NULL != param[0])
		{
			ucLedId = atol(param[0]);
			
			if (NULL != param[1])
			{
				unLedAction = atol(param[1]);
				
				if ( (ucLedId == 1) || (ucLedId == 2) || (ucLedId == 3) ) 
				{
					if ( (unLedAction == 0) || (unLedAction == 1) || (unLedAction == 2) ) 
					{
						if (ucLedId == 1)
							LedInterface(unRedAmberLedId ,RED , unLedAction);
						else if (ucLedId == 2)
							LedInterface(unGreenLedId ,GREEN , unLedAction);
						else 
							LedInterface(unAmberLedId ,AMBER , unLedAction);
					}
					else
					{
						TerminalCommandPrintFunction(fd,"\nSetCardLeds Led Action Is Not Good (0 - off , 1 - on , 2 - blink)\n");	
					}
				}
				else
				{
					TerminalCommandPrintFunction(fd,"\nSetCardLeds Led Id Is Not Good (1 - red , 2 - green , 3 - amber)\n");	
				}
			}
			else
			{
				TerminalCommandPrintFunction(fd,"\nSetCardLeds Please Insert Led Action (0 - off , 1 - on , 2 - blink)\n");	
			}
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nSetCardLeds Please Insert Led Id (1 - red , 2 - green , 3 - amber)\n");	
		}*/
	}
	else if (!strcmp(commandName, "GetHotSwapHandels")|| !strcmp(commandName,"55"))
	{
		INT8 ucHandleState;
		
		rc = GetHandleStateFunc(&ucHandleState);
		
		if (rc == 0)
		{
			if (ucHandleState == 0)
				TerminalCommandPrintFunction(fd,"\nGetHotSwapHandels = Open\n");
			else
				TerminalCommandPrintFunction(fd,"\nGetHotSwapHandels = Close\n");
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nFail Read Hot Swap Handels\n");	
		}	
	}
	else if (!strcmp(commandName, "SetHotSwapHandels")|| !strcmp(commandName,"56"))
	{
		UINT32 unHandle = 0;
		
		if (NULL != param[0])
		{
			unHandle = atol(param[0]);
			if (unHandle == 1)
			{
				rc = SetHandleStateFunc(0x11);
				if (rc == 0)
				{
					TerminalCommandPrintFunction(fd,"\nSetHandleState On Ignore Handle\n");	
				}
				else
				{
					TerminalCommandPrintFunction(fd,"\nFail SetHandleState On Ignore Handle\n");
				}
			}
			else if (unHandle == 0)
			{
				rc = SetHandleStateFunc(0);
				
				if (rc == 0)
				{
					TerminalCommandPrintFunction(fd,"\nSetHandleState Off Do Not Ignore Handle\n");
				}
				else
				{
					TerminalCommandPrintFunction(fd,"\nFail SetHandleState Off Do Not Ignore Handle\n");	
				}
			}
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nPlease Set 1 or 0\n");	
		}			
	}
	else if (!strcmp(commandName, "SetFanSpeed")|| !strcmp(commandName,"57"))
	{
		UINT32 unFanSpeed = 0;
		
		if (NULL != param[0])
		{
			unFanSpeed = atol(param[0]);
			
			if (unFanSpeed > 4)
			{
				TerminalCommandPrintFunction(fd,"\nSetFanSpeed Please Set 0 till 4\n");	
			}
			else
			{
				rc = SetFanSpeedFunc(unFanSpeed);
				if (rc =! 0)
				{
					TerminalCommandPrintFunction(fd,"\nFail SetFanSpeedFunc Fan Speed %d\n",unFanSpeed);
				}
				else
				{
					TerminalCommandPrintFunction(fd,"\nSuccess SetFanSpeedFunc Fan Speed %d\n",unFanSpeed);
				}
			}
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nPlease Set 0 till 4\n");	
		}			
	}
	else if (!strcmp(commandName, "GetFanSpeed")|| !strcmp(commandName,"58"))
	{
		TerminalCommandPrintFunction(fd,"\nGetFanSpeed - Not Used In MPM Card\n");		
	}
	else if (!strcmp(commandName, "GetPowerForceOn")|| !strcmp(commandName,"59"))
	{
		TerminalCommandPrintFunction(fd,"\nGetPowerForceOn - not used\n");		
	}
	else if (!strcmp(commandName, "SetPowerForceOn")|| !strcmp(commandName,"60"))
	{
		UINT32 unForceMode = 0;
		
		if (NULL != param[0])
		{
			unForceMode = atol(param[0]);
			
			if (unForceMode == 0)
			{
				SetPowerForceFunc(0);
				
				TerminalCommandPrintFunction(fd,"\nSet Power Force Off\n");	
			}
			else
			{
				SetPowerForceFunc(0xff);
				
				TerminalCommandPrintFunction(fd,"\nSet Power Force On\n");
			}
		}
		else
		{
			TerminalCommandPrintFunction(fd,"\nPlease Set 0 to Off or 1 to On\n");	
		}	
	}
	else if (!strcmp(commandName, "ChgHttpd")|| !strcmp(commandName,"61"))
	{			/*
		UINT32 isSecured = 0;
		UINT32 isPermanentOpen = 0;
		char ucFieldStr[100];
        
        printf("ChgHttpd %s %s\n",param[0],param[1]);

        if ((NULL != param[0])&&(NULL != param[1]))
		{
            isSecured = atol(param[0]);
            isPermanentOpen = atol(param[1]);
            printf("ChgHttpd unHttpdModeSec %x unHttpdModePer %x\n",isSecured,isPermanentOpen);

            printf("ChgHttpd isSecured %x isPermanentOpen %x\n",
                   mfaInfoStruct.MngInfo.isSecured,mfaInfoStruct.MngInfo.isPermanentOpen);
            
            
            if ((mfaInfoStruct.MngInfo.isSecured != isSecured)||
                (mfaInfoStruct.MngInfo.isPermanentOpen 	!= isPermanentOpen))	
            {
                UpdateHttpdConfig(isSecured,isPermanentOpen);
                
                mfaInfoStruct.MngInfo.isSecured			= isSecured;
                if( isSecured == YES )
                    memcpy((void*)ucFieldStr,"YES",sizeof(ucFieldStr));
                else
                    memcpy((void*)ucFieldStr,"NO",sizeof(ucFieldStr));
                
                UpdateSysInfoFileWithNewParams(	USB_ENTRY__MNG_IS_SECURED ,
                                                ucFieldStr ,
                                                strlen(ucFieldStr) );
                
                mfaInfoStruct.MngInfo.isPermanentOpen = isPermanentOpen;

                if(isPermanentOpen  == YES )
                    memcpy((void*)ucFieldStr,"YES",sizeof(ucFieldStr));
                else
                    memcpy((void*)ucFieldStr,"NO",sizeof(ucFieldStr));
                
                UpdateSysInfoFileWithNewParams(	USB_ENTRY__MNG_IS_PERMANENT_OPEN,
                                                ucFieldStr ,
                                                strlen(ucFieldStr) );
            }
		}				*/
	}
	else if(!strcmp(commandName, "ClosePermanent")|| !strcmp(commandName,"62"))
    {/*
        phy_disable_port(MODEM_PORT_GDNLT);
    */}
    else if(!strcmp(commandName, "SetJITCmode")|| !strcmp(commandName,"63"))
    {
        UINT32 JITCmode;
        INT32 rc;

        if(NULL != param[0])
        {
            JITCmode = atoi(param[0]);

            rc = flashUtilSetJITCmodeAndNetSeparate(JITCmode,-1);
            fprintf(fd, "SetJITCmode : rc %d flashUtilSetJITCmode(mode = %d)\n", rc, JITCmode);
        }
        else
        {
            fprintf(fd, "SetJITCmode : bad usage: ca SWM 63 [1,0]\n");
        }
    }
    else if(!strcmp(commandName, "GetJITCmode")|| !strcmp(commandName,"64"))
    {
        UINT32 JITCmode = 0;
        INT32 rc = 0;
     
        rc = flashUtilGetJITCmode(&JITCmode);
        if (!strcmp(argc,"xxx"))
        {
            fprintf(fd, "GetJITCmode : rc %d flashUtilGetJITCmode(mode = %d)\n", rc, JITCmode);
        }
        else
        {
            PrintTerminalCommand("GetJITCmode : rc %d flashUtilGetJITCmode(mode = %d)\n", rc, JITCmode);
        }
    }
	else if (!strcmp(commandName, "SetFltrProtect")||( !strcmp(commandName,"70")))
	{
		UINT32 flag;
		if (NULL != param[0])
            updtFilterProtectFlag(atol(param[0]));
		flag = readFilterProtectFlag();
		if (flag==1)
			TerminalCommandPrintFunction(fd,"\nFilter protect is on\n");	
		else if (flag==0)
			TerminalCommandPrintFunction(fd,"\nFilter protect is off\n");
	}
	else if (!strcmp(commandName, "EnableMirrorPort")||( !strcmp(commandName,"71")))
	{
		UINT8 source;
		UINT8 mirror;
		UINT8 enable;


		if ((NULL != param[0]) && (NULL != param[1]) && (NULL != param[2]))
		{
			source = (UINT8)atol(param[0]);
			mirror = (UINT8)atol(param[1]);
			enable = (UINT8)atol(param[2]);

			if (enable == TRUE)
			{
				TerminalCommandPrintFunction(fd,"\nSet Mirror to port %d (source) on port %d (mirror)\n", source, mirror );
			}

			if (enable == FALSE)
			{
				TerminalCommandPrintFunction(fd,"\nDisable Mirror to port %d (source) on port %d (mirror)\n", source, mirror );
			}

			//setting mirror source
			h2_set_mirror_source (source, enable);

			//setting mirror port
			h2_set_mirror_port (mirror);

		}
		else
		{	
			//Wrong input
			TerminalCommandPrintFunction(fd,"\nFail EnableMirrorPort - missing source/mirror/enable , see help\n");
		}
	}
    else if(!strcmp(commandName, "exit")||( !strcmp(commandName,"72")))
    {
        TerminalCommandPrintFunction(fd,"switch will exit(0) now\n----------------------------------\n\n");
        exit(0);
    }
	else if(!strcmp(commandName,"help"))			
	{
		if (!strcmp(argc,"xxx"))
		{
            //  52 strings        01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123
			fprintf(fd,          "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                    help0,help1,help2,help3,help4,help5,help6,help7,help8,help9,
                    help10,help11,help12,help13,help14,help15,help16,help17,help19,
                    help20,help21,help22,help23,help24,help25,help26,help27,help28,help29,
                    help45,help46,help47,help48,help49,help50,help51,help52,help53,help54,
                    help55,help56,help57,help58,help59,help60,help61, help62, help63, help64,
                    help70, help71, help72);
		}
		else
		{
            //  52 strings           01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123
			PrintTerminalCommand("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                    help0,help1,help2,help3,help4,help5,help6,help7,help8,help9,
                    help10,help11,help12,help13,help14,help15,help16,help17,help19,
                    help20,help21,help22,help23,help24,help25,help26,help27,help28,help29,
                    help45,help46,help47,help48,help49,help50,help51,help52,help53,help54,
                    help55,help56,help57,help58,help59,help60,help61, help62, help63, help64,
                                 help70, help71, help72);
        }
    }	
	else
	{
	  if (!strcmp(argc,"xxx"))
		{
			fprintf(fd,"TerminalCommand Error\n\r");
		}
		else
		{
		  MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TerminalCommand Error , unknown commandName=%s",commandName);
		}
	}
	if (!strcmp(argc,"xxx"))
    {
		fclose(fd);	
    }	
#endif
}

/*
void PrintTerminalCommand(INT8 *pcFormat, ...)
{
    static INT8  *string; 
    static BOOL once = YES;
    INT8 *pucPrintString = NULL;
    UINT32 unLoggerHeaderSize, unContentLen;
	TMessageThreadType tMessageThreadType;
    va_list tVarList;
	memset(&tMessageThreadType,0,sizeof(TMessageThreadType));
	if (once)
	{
		if(NULL == (string = (UINT8*)malloc(MAX_PRINT_TO_MCMS_TERMINAL_SIZE))) return;
		once = NO;
	}
	
	tMessageThreadType.ulData = 0;
	tMessageThreadType.ulSize = 0;
	va_start(tVarList,pcFormat);
	
	vsprintf(string,pcFormat,tVarList);
	unContentLen = strlen(string) + 1;
	
	pucPrintString = (UINT8*)malloc(unContentLen);
	if (pucPrintString == NULL)
	{
	 	printf("\n(PrintTerminalCommand) Terminal Command Malloc Failure.\n");
		va_end(tVarList);
	 	return;   		
	}
		
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(PrintTerminalCommand): string = %s , unContentLen = %d",string,unContentLen);

	memcpy( (void*)(pucPrintString) , (void*)&string[0] , unContentLen );
	
	tMessageThreadType.ulData   = (UINT32)pucPrintString;
	tMessageThreadType.ulSize   = unContentLen;

   	SendTerminalCommandToMcms(tMessageThreadType);
	
	va_end(tVarList);
}		



void SendTerminalCommandToMcms(TMessageThreadType tMessageThreadType)
{
	UINT32 unTerminalHeaderSize , unContentLen = 0;
	INT32 rc = 0;
	UINT8 *pucTerminalString = NULL;	

	if( (tMessageThreadType.ulData == 0) || (tMessageThreadType.ulSize == 0) )
	{
		printf("(SendTerminalCommandToMcms) tMessageThreadType == 0\n");
		if (tMessageThreadType.ulData != 0)
		{
			free((void *)tMessageThreadType.ulData);
		}
		return;   		
	}
	unTerminalHeaderSize = sizeof(COMMON_HEADER_S) + sizeof(TRACE_HEADER_S);
	unContentLen = tMessageThreadType.ulSize;
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(SendTerminalCommandToMcms): unContentLen = %d , unContentLen + unTerminalHeaderSize = %d",unContentLen,(unContentLen + unTerminalHeaderSize));
	
	if ((unContentLen + unTerminalHeaderSize) > MAX_PRINT_TO_MCMS_TERMINAL_SIZE)
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SendTerminalCommandToMcms): string size:%d > maxsize:%d", unContentLen , (MAX_PRINT_TO_MCMS_TERMINAL_SIZE - unTerminalHeaderSize));
		unContentLen = (MAX_PRINT_TO_MCMS_TERMINAL_SIZE - unTerminalHeaderSize);
	}

	pucTerminalString = (UINT8*)malloc(unContentLen + unTerminalHeaderSize);
	if (pucTerminalString == NULL)
	{
		printf("\n(SendTerminalCommandToMcms)Terminal Command Malloc Failure.\n");
		if (tMessageThreadType.ulData != 0)
		{
			free((void *)tMessageThreadType.ulData);
		}
		return;   		
	}

	//fill the common header
	FillTerminalMsgHeaders(0 , (UINT8*)pucTerminalString, unContentLen);
	
	//fill the content
	memcpy( (void*)(pucTerminalString + unTerminalHeaderSize) , (void*)(INT8*)tMessageThreadType.ulData , tMessageThreadType.ulSize); 

	if (TcpConnection[eLoggerCom].ul_ConnectionStatus == CONNECTED)
	{
		if ( TCPSendData( TcpConnection[eLoggerCom].s , (void*)pucTerminalString , (tMessageThreadType.ulSize + unTerminalHeaderSize) , 0 , BOARD_PRINT_NO,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
		{
#if 0
			if (isSendBeforeOk == YES)
			{
				gettimeofday(&tTimer_vals,NULL) ;
       			timep = time((time_t*)NULL);
				gmtime_r(&timep, &result);
				printf("D: %d/%d/%d  T: %d:%d:%d:%d  (PrintThread): problem in sending data \n\n ",result.tm_mday,(result.tm_mon + 1),(result.tm_year + 1900),result.tm_hour,result.tm_min,result.tm_sec,tTimer_vals.tv_usec);
				isSendBeforeOk = NO;
			}
#endif
			//TcpConnection[eLoggerCom].ul_ConnectionStatus = NOT_CONNECTED;	 
			//TcpConnection[eLoggerCom].ul_PrevConnState = CONNECTED;		
		}
		else
		{
			if (tMessageThreadType.ulData != 0)
			{
				free((void *)tMessageThreadType.ulData);
			}
		}
	}//if (TcpConnection[eLoggerCom].ul_ConnectionStatus == CONNECTED)
	else
	{
		if(NULL != pucTerminalString)
		{
			free((void *)pucTerminalString);
		}
	}




#if 0
	if (SendControlMsg(PRINT_QUEUE , (UINT8 *)pucTerminalString , (unContentLen + unTerminalHeaderSize) , LOCAL_TRANSFER , 0))
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TerminalCommand Error , Failed send msg");
		free(pucTerminalString);
	}
	else
	{
		unTerminalMsgCnt++;
	}
	
	if (tMessageThreadType.ulData != 0)
	{
		free((void *)tMessageThreadType.ulData);
	}
endif
}
*/


void FillTerminalMsgHeaders(UINT32 id, UINT8* pContent, UINT32 ulBufferSize)
{
	COMMON_HEADER_S tCommonHeader;
	TRACE_HEADER_S 	tTraceHeader;
	UINT32 commonHeaderLen, traceHeaderLen;//, contentLen;

	if( (pContent == NULL) || (ulBufferSize == 0) )
	{
		printf("\n(FillTerminalMsgHeaders) pContent = %p , ulBufferSize = %d\n",pContent,ulBufferSize);
		return;
	}
	
	commonHeaderLen = sizeof(COMMON_HEADER_S);
	traceHeaderLen  = sizeof(TRACE_HEADER_S);
	//contentLen 	  	= ulBufferSize - commonHeaderLen - traceHeaderLen;

	FillCommonHeaderTerminalCommand((COMMON_HEADER_S*)pContent);
	FillTraceHeaderTerminalCommand((TRACE_HEADER_S*)(pContent + commonHeaderLen), id, ulBufferSize);
}




void FillCommonHeaderTerminalCommand(COMMON_HEADER_S *pCommonHeader)
{
	if(	pCommonHeader != 0)
	{
		memset(pCommonHeader, 0, sizeof(COMMON_HEADER_S));
		pCommonHeader->src_id 				= eMpl;
		pCommonHeader->dest_id 				= eMcms;
		pCommonHeader->next_header_type		= SWAPL(eHeaderTrace);
	}
	else
	{
		printf("(FillCommonHeader) pCommonHeader = NULL\n");	
	}
}

void FillTraceHeaderTerminalCommand(TRACE_HEADER_S *pTraceHeader, UINT32 id, UINT32 ulBufferSize)
{
	if(	pTraceHeader != 0)
	{
		pTraceHeader->m_processMessageNumber = SWAPL(unTerminalMsgCnt);
		pTraceHeader->m_systemTick 		= SWAPL(SystemGetTickCount() );
		pTraceHeader->m_processType 	= SWAPL(eMfaCardManager);
		pTraceHeader->m_level 			= SWAPL(eLevelFatal);
		pTraceHeader->m_sourceId 		= SWAPL((APIU32)id);
		pTraceHeader->m_messageLen 		= SWAPL(ulBufferSize);
		pTraceHeader->m_taskName[0] 	= '\0';
		pTraceHeader->m_objectName[0]	= '\0';
		pTraceHeader->m_topic_id 		= 0xFFFFFFFF;
		pTraceHeader->m_unit_id 		= 0;	//mfa
		pTraceHeader->m_conf_id 		= 0xFFFFFFFF;
		pTraceHeader->m_party_id 		= 0xFFFFFFFF;
		pTraceHeader->m_opcode 			= 0xFFFFFFFF;
		pTraceHeader->m_str_opcode[0] 	= '\0';
		
		memset((void*)(&pTraceHeader->m_terminalName[0]),0,MAX_TERMINAL_NAME_LEN);
		memcpy((void*)(&pTraceHeader->m_terminalName[0]),(void*)terminalName , strlen(terminalName));
		
		memset((void*)(&pTraceHeader->m_taskName[0]),0,MAX_TASK_NAME_LEN);
		memcpy((void*)(&pTraceHeader->m_taskName[0]),(void*)"TerminalCommand",(strlen("TerminalCommand") + 1) );
	}
	else
	{
		printf("(FillTraceHeader) pTraceHeader = NULL\n");	
	}
}
/*
void TerminalCommandPrintFunction(FILE *fFIleDescriptor,INT8 *pcFormat, ...)
{
	INT8  string[1000], *pucPrintString;
    va_list tVarList;
    
    memset(&tVarList,0,sizeof(va_list));
	memset((void*)&string[0], '\0', 1000);
	
	va_start(tVarList,pcFormat);
		
	vsprintf(string,pcFormat,tVarList);

	if (fFIleDescriptor == NULL)
	{
		PrintTerminalCommand(string);			
	}
	else
	{
		fprintf(fFIleDescriptor,string);
	}  
}

void SendSSHModeMessageToMpl(APIUBOOL isOnRequest)
{
	SWITCH_MPL_API_REQUEST_S	*ptSwitchMplApiReq;
	TMessageThreadType 			tMessageThreadType;

	MfaBoardPrint(	TCP_SERVER_PRINT,
					PRINT_LEVEL_ERROR,
					PRINT_TO_TERMINAL,
					"(SendSSHModeMessageToMpl): Enter build request message");

	tMessageThreadType.ulSize = sizeof(SWITCH_MPL_API_REQUEST_S);

	ptSwitchMplApiReq = malloc(tMessageThreadType.ulSize);

	tMessageThreadType.ulData = (UINT32)ptSwitchMplApiReq;

	ptSwitchMplApiReq->tEmaReqHeader.ulOpcode = SSH_MODE_REQUEST_OPCODE;
	ptSwitchMplApiReq->tEmaReqHeader.ulMsgID  = 0xBABA;
	ptSwitchMplApiReq->isOnRequest = isOnRequest;


	if (GetChasisPlatformTypeInDword(1) == eAmos) // RMX 4000 
	{
		if(TcpConnection[eMfa1ResetServer].ul_ConnectionStatus == CONNECTED)
		{
			ptSwitchMplApiReq->tEmaReqHeader.ulSlotID  = e_slotIdMfa1_4000;
			if ( TCPSendData( TcpConnection[eMfa1ResetServer].s, tMessageThreadType.ulData, tMessageThreadType.ulSize, 0, BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
			{
				MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SendSSHModeMessageToMpl): problem in sending data...\n");	
			}
		}
		if(TcpConnection[eMfa2ResetServer].ul_ConnectionStatus == CONNECTED)
		{
			ptSwitchMplApiReq->tEmaReqHeader.ulSlotID  = e_slotIdMfa2_4000;
			if ( TCPSendData( TcpConnection[eMfa2ResetServer].s, tMessageThreadType.ulData, tMessageThreadType.ulSize, 0, BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
			{
				MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SendSSHModeMessageToMpl): problem in sending data...\n");	
			}
		}	
		if(TcpConnection[eMfa3ResetServer].ul_ConnectionStatus == CONNECTED)
		{
			ptSwitchMplApiReq->tEmaReqHeader.ulSlotID  = e_slotIdMfa3_4000;
			if ( TCPSendData( TcpConnection[eMfa3ResetServer].s, tMessageThreadType.ulData, tMessageThreadType.ulSize, 0, BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
			{
				MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SendSSHModeMessageToMpl): problem in sending data...\n");	
			}
		}
		if(TcpConnection[eMfa4ResetServer].ul_ConnectionStatus == CONNECTED)
		{
			ptSwitchMplApiReq->tEmaReqHeader.ulSlotID  = e_slotIdMfa4_4000;
			if ( TCPSendData( TcpConnection[eMfa4ResetServer].s, tMessageThreadType.ulData, tMessageThreadType.ulSize, 0, BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
			{
				MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SendSSHModeMessageToMpl): problem in sending data...\n");	
			}
		}	
	}
	else // RMX 2000
	{
		if(TcpConnection[eMfa1ResetServer].ul_ConnectionStatus == CONNECTED)
		{
			ptSwitchMplApiReq->tEmaReqHeader.ulSlotID  = e_slotIdMfa1_2000;
			if ( TCPSendData( TcpConnection[eMfa1ResetServer].s, tMessageThreadType.ulData, tMessageThreadType.ulSize, 0, BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
			{
				MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SendSSHModeMessageToMpl): problem in sending data...\n");	
			}
		}
		if(TcpConnection[eMfa2ResetServer].ul_ConnectionStatus == CONNECTED)
		{
			ptSwitchMplApiReq->tEmaReqHeader.ulSlotID  = e_slotIdMfa2_2000;
			if ( TCPSendData( TcpConnection[eMfa2ResetServer].s, tMessageThreadType.ulData, tMessageThreadType.ulSize, 0, BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
			{
				MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SendSSHModeMessageToMpl): problem in sending data...\n");	
			}
		}	
	}




   	MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(SendSSHModeMessageToMpl): Data sent.");	
}
*/
void updtFilterProtectFlag(INT32 flag)
{
	INT32 rc;
	FILE *pFilterProtectFlagFile;
	pFilterProtectFlagFile=fopen("/mnt/mfa_cm_fs/filterProtectFlag","r+");
	if (pFilterProtectFlagFile != NULL)
	{
		fseek(pFilterProtectFlagFile,0,SEEK_SET); 
		rc = fwrite(&flag,sizeof(int),1,pFilterProtectFlagFile);
		fclose(pFilterProtectFlagFile);
	}
}

INT32 readFilterProtectFlag()
{
	INT32 rc,flag = 0;
	FILE *pFilterProtectFlagFile;
	pFilterProtectFlagFile=fopen("/mnt/mfa_cm_fs/filterProtectFlag","r+");
	if (pFilterProtectFlagFile != NULL)
	{
		rc = fread(&flag,sizeof(INT32),1,pFilterProtectFlagFile);
		fclose(pFilterProtectFlagFile);
	}
	return flag;
}


