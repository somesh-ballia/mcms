/*===================================================================================================================*/
/*            Copyright     ???    2001  Polycom Israel, Ltd. All rights reserved                                    */
/*-------------------------------------------------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary   information of                              */
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form                    */
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without                          */
/* prior written authorization from Polycom Israel Ltd.                                                              */
/*-------------------------------------------------------------------------------------------------------------------*/
/* FILE:     LSMain.c                                                                                   */
/* PROJECT:  Gideon CM                                                                                               */
/* PROGRAMMER:  Nir Ben Shachar                                            .                                            */
/*                                                                                                                   */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                                                                   */
/*-------------------------------------------------------------------------------------------------------------------*/
/*              |                     |                                                                              */
/*===================================================================================================================*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h> 
#include <sys/stat.h> 
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/io.h>
#include <fcntl.h>
#include "CmdCard.h"
#include "Print.h"
#include "tools.h"

#include "LinuxSystemCallsApi.h"

extern void HandleTerminalCommand(UINT8* pMsg); 

int error_handle(char * pstring)
{
	printf("%s",pstring);
	return -1;
}

void  CmdCardThread()
{
	int fd;
	int nQid,size;
	struct msg_buf_def msg_buf;
	char pmsg[80];
	key_t key;
	struct msgbuf *pmsgbuf;
	const char *ucEnv=CARDCMD_PATH;

	EnrollInThreadList(eCmdCardThread);


	printf("\nCmdCardThread started.");	
	key = ftok(ucEnv,PROJECT_ID);
 	if (key == -1)
	{
		error_handle("ftok failed");
	}
	nQid = msgget(key,IPC_CREAT|S_IRWXU);
	if (nQid == -1) 
	{ 
		error_handle("msgget failed");
	}
	msg_buf.mtype = eSwitchManager;
	pmsgbuf= (struct msgbuf *)&msg_buf;
	while(g_isServiceRun)
	{
		printf("\nStarting to sleep on msg .\n");
		size = msgrcv(nQid, pmsgbuf, sizeof(struct msg_buf_def), msg_buf.mtype, 0); 
		if (size == -1)
		{
			error_handle("msgrcv failed");
	 	}
	 	
		printf("\nreceived msg and sending to handle terminal command"); 	
	 	snprintf(pmsg, sizeof(pmsg), "xxx %s %s",msg_buf.terminal_file_name,msg_buf.command);
	 	HandleTerminalCommand(pmsg);
	}
 	
}

