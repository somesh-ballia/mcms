/******************************************************************************
*
* Copyright (C) 2005 POLYCOM NETWORKS Ltd.
* This file contains confidential information proprietary to POLYCOM NETWORKSO
*  Ltd. The use or disclosure of any information contained
* in this file without the written consent of an officer of POLYCOM NETWORKS
* Ltd is expressly forbidden.
*
*****************************************************************************
 Module Name: main.c
*****************************************************************************
      1.

 Generated By:	Yigal Mizrahi       Date: 26.4.2005

*****************************************************************************/



/***** Include Files *****/
#include "MplStartup.h"
#include "SwInfoInit.h"
//#include "DownLoad.h"
//#include "IpmcInt.h"
#include "SystemInfo.h"
#include "McuMngrStructs.h"
//#include "mfa_board.h"
#include <sys/wait.h>
#include <sys/utsname.h>
//#include "e2prom.h"
#include "../sources/Diagnostics/includes/Diagnostics.h"
#include "Print.h"
//#include "LSMain.h"

//UINT32 unEmaReset = 0;
//////////////////////////////////////////////////////////////////////
#include "SocketApiWrapExt.h"
#include "timers.h"

/***** Public Variables *****/
//extern INT32 nMcmsSocket;
//extern INT32 nTCPXmitQueue;
//extern UINT8 unRestoreDefault;
extern IF_NAME_STRUCT tIfNameStruct;

/***** Global Variables *****/

/***** Public Functions Declarations *****/

/***** Private Functions Declarations *****/

/***** External Functions Declarations *****/
extern int LoadIpmcSoftware(int argc, char **argv) ;
char *upgradeCmd[5] = {"upgradefw","-s","ttyS1","/mnt/mfa_cm_fs/images/upgrade.img",NULL};

UINT8     CurrentSysFile[MAX_FILENAME_LEN];

UINT32 unResetCard = 0;
UINT32 unSwitchSendAuthenticationInd = 0;
UINT32 ulPlatformType= 0xFFFFFFFF;

void UpdateHttpdConfig(BOOL isSecured,BOOL isPermanentOpen)
{
	FILE *fdSrc=NULL;
	FILE *fdDest=NULL;
	int rc;
	UINT32 ind;
	UINT32 maxInd;
	char WorkingStr[256],*pWorkingStr=WorkingStr;
	char InternalIpStr[64]="Listen 169.254.192.16:80\n";
	char LocalIpStr[64]="Listen 169.254.128.16:80\n";
	char CommentStr[64]="#";
	t_TcpConnParams* ptTcpCon;

	//MfaBoardPrint(STARTUP_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"UpdateHttpdConfig isSecured %x perm %x ulAddr(IPV4) %x\n",isSecured,isPermanentOpen,mfaInfoStruct.MngInfo.ipSmAddr.iPv4.ulAddr);

	fdSrc = fopen("/mcms/StaticCfg/httpd.diag.conf","r");
	fdDest = fopen("/tmp/mfa_cm_fs/httpdTemp.conf","w");
	if ((fdSrc != NULL)&&(fdDest != NULL))
	{
		while (fgets(WorkingStr,sizeof(WorkingStr),fdSrc)!=NULL)
		{

			if (strstr(pWorkingStr,"Listen 80")!=NULL)
			{
/*				if ((isSecured == 0) && (isPermanentOpen == 1))	// don't change file in this case
					fwrite(pWorkingStr,strlen(pWorkingStr),1,fdDest);
				else
				{
					fwrite(CommentStr,strlen(CommentStr),1,fdDest);
					fwrite(pWorkingStr,strlen(pWorkingStr),1,fdDest);
					if ((isSecured == 1) || (mfaInfoStruct.MngInfo.ipSmAddr.ulAddr == 0))
						fwrite(CommentStr,strlen(CommentStr),1,fdDest);
					sprintf(pWorkingStr,"Listen %d.%d.%d.%d:80\n",
						mfaInfoStruct.MngInfo.ipSmAddr.ulAddr>>24,
						(mfaInfoStruct.MngInfo.ipSmAddr.ulAddr>>16)&0xff,
						(mfaInfoStruct.MngIsystem("killall -9 httpd");nfo.ipSmAddr.ulAddr>>8)&0xff,
						(mfaInfoStruct.MngInfo.ipSmAddr.ulAddr)&0xff);
					fwrite(pWorkingStr,strlen(pWorkingStr),1,fdDest);
					if (isPermanentOpen == 0)
						fwrite(CommentStr,strlen(CommentStr),1,fdDest);
					fwrite(InternalIpStr,strlen(InternalIpStr),1,fdDest);
					if ((isSecured == 1) && (isPermanentOpen == 0))
						fwrite(LocalIpStr,strlen(LocalIpStr),1,fdDest);
				} */
/*				fwrite(CommentStr,strlen(CommentStr),1,fdDest);
				fwrite(pWorkingStr,strlen(psystem("killall -9 httpd");WorkingStr),1,fdDest);
				if ((isSecured == 1) || (mfaInfoStruct.MngInfo.ipSmAddr.ulAddr == 0))
					fwrite(CommentStr,strlen(CommentStr),1,fdDest);
				sprintf(pWorkingStr,"Listen %d.%d.%d.%d:80\n",
					mfaInfoStruct.MngInfo.ipSmAddr.ulAddr>>24,
					(mfaInfoStruct.MngInfo.ipSmAddr.ulAddr>>16)&0xff,
					(mfaInfoStruct.MngInfo.ipSmAddr.ulAddr>>8)&0xff,
					(mfaInfoStruct.MngInfo.system("killall -9 httpd");ipSmAddr.ulAddr)&0xff);
				fwrite(pWorkingStr,strlen(pWorkingStr),1,fdDest);
				if (isPermanentOpen == 0)
					fwrite(CommentStr,strlen(CommentStr),1,fdDest);
				fwrite(InternalIpStr,strlen(InternalIpStr),1,fdDest);
				fwrite(LocalIpStr,strlen(LocalIpStr),1,fdDest);	 */
				fwrite(CommentStr,strlen(CommentStr),1,fdDest);
				fwrite(pWorkingStr,strlen(pWorkingStr),1,fdDest);
				if (isSecured == 0)
					fwrite("Listen 80\n",strlen("Listen 80\n"),1,fdDest);
                else fwrite(LocalIpStr,strlen(LocalIpStr),1,fdDest);
			}
			else
			{
				fwrite(WorkingStr,strlen(WorkingStr),1,fdDest);
			}
		}
	}
	if (fdDest !=NULL)
		fclose(fdDest);
	if (fdSrc !=NULL)
		fclose(fdSrc);

	printf("yosi - after scan files");
	EmbSleep(1);
    extern fd_set *ptTcpActiveReadSockets;
   	extern fd_set *ptTcpActiveSelectSockets;

  	extern UINT32 g_emaTcpConnParamsInd;
	extern t_TcpConnParams g_emaTcpConnParams[30];;
	// sometimes more then one connection is open clean all connection
	maxInd=g_emaTcpConnParamsInd;
	for (ind=0;ind<maxInd;ind++)//system("killall -9 httpd");	- PAVELK disable
	{
		ptTcpCon = &g_emaTcpConnParams[ind];
	}
	for (ind=0;ind<maxInd;ind++)
	{
		ptTcpCon = &g_emaTcpConnParams[ind];
		if (ptTcpCon->s > 0)
		{
  		 	if (FD_ISSET(ptTcpCon->s, ptTcpActiveSelectSockets))
   			{
				FD_CLR(ptTcpCon->s,ptTcpActiveReadSockets); // Benson
   		 	}
		 	shutdown(ptTcpCon->s,SHUT_RDWR);
		 	close(ptTcpCon->s);
		}
	}
	printf("after clean all connection");
	EmbSleep(1);
    
	g_emaTcpConnParamsInd=0;
    ptTcpCon = &TcpConnection[eEmaApiServer];
    	EmbSleep(1);

	if(ptTcpCon->s > 0)
		TimerDeleteJob(ptTcpCon->ul_TimerHandle);
	    	EmbSleep(1);
	ptTcpCon->s=-1;
    ptTcpCon->ul_ConnectionStatus = NOT_CONNECTED;
    ptTcpCon->ul_PrevConnState = CONNECTED;
	system(HTTPD_KILL_COMMAND);
	EmbSleep(3000);
		    	EmbSleep(1);
#if 0
	system("rm /var/lock/httpd.pid");
#else
	system("rm /tmp/mfa_cm_fs/httpd.pid");
#endif
	rc = system(HTTPD_RUN_COMMAND);
			    	EmbSleep(1);
	if (isSecured == YES)
		system ("killall -9 dropbear 2>/dev/null");

				    	EmbSleep(1);
}


