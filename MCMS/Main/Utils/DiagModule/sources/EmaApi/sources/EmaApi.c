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


#include "LinuxSystemCallsApi.h"
#include "SocketApiWrapExt.h"
#include "EmaApi.h"
#include "string.h"
#include "DiagnosticsShared.h"
#include "SharedDefines.h"
#include "Print.h"
#include "emaCtl.h"
#include "tools.h"
#include "McuMngrStructs.h"
#include "arpa/inet.h"
#include "Diagnostics.h"
#include "timers.h"

extern UINT8	tpcktBugOverrideBuffer[];
extern UINT32	tpcktBugOverrideLen;

//extern void reStartEmaWatchdogTimer();
extern void EnterDiagnosticsMode( UINT32 ulMsgId, UINT32 ulSlotId);
extern void SendDbgModeReq(APIUBOOL isDbgMode);
extern void	BuildControlResponseMsg(UINT32 ulMsgId, UINT32 ulSlotId);

extern void handleEmaLoginMsg(char **commandArgs, int numArgs, int ulMsgId, int ulSlotId);
extern void handleEmaCtlMsg(char **commandArgs,int numArgs);

#define MAX_NUM_OF_EMA_CLIENTS  	10
#define MAX_NUM_OF_EMA_SIM_CLIENTS  10
#define MAX_NUM_OF_MFA_DIAG_CLIENTS 10
#define MAX_NUM_OF_CPU_DIAG_CLIENTS 1

extern UINT32 switch_diag_mode;
UINT32 StringWrapperArrayHandler(UINT8* apucString,TStructToStr** pptStructSpecPtr,UINT32 IndHdr,UINT32 ulNumOfElem,UINT32 ulNumOfElemFields);

//extern UINT32 unEmaReset;

extern IF_NAME_STRUCT tIfNameStruct;

// The size of the buffer is:  MAX_PARAMS_IN_STRING * MAX_STRING_PARAM = 100 * 24 = 2400 bytes
UINT8 reqBuffer[MAX_PARAMS_IN_STRING * MAX_STRING_PARAM];
#define  MAX_EMA_CONN 30
t_TcpConnParams g_emaTcpConnParams[MAX_EMA_CONN];
UINT32 g_emaTcpConnParamsInd=0;
//UINT32* swapMessage(UINT32* rcvBuf, UINT32 bufSize);
void swapMessage(UINT32* rcvBuf, UINT32 bufSize);

void StringWrapper(e_TcpConn eID,UINT32 Msg)
{
	UINT8 *acString;
	UINT8 acParam[256];
	PTSpecGnrlHdr ptSpecGnrlHdr;
	UINT32	 IndHdr,IndHdrStart;
	PTStructToStr ptStructSpecPtr;
	PTStructToStr ptStructSpecNext;
	UINT32 i;
	UINT32 ulTempNumOfElem=0;
//	Bracha: now working with real EMA
	t_TcpConnParams* ptTcpCon = &TcpConnection[eEmaApiServer];
//	Bracha: now working with EMA simulation
	t_TcpConnParams* ptTcpCon_forSim = &TcpConnection[eEmaSimServer];
	int sendRetVal;

	MfaBoardPrint(	TCP_SERVER_PRINT,
					PRINT_LEVEL_MAJOR,
					PRINT_TO_TERMINAL,
					"(StringWrapper): Got a structure to wrap from eId: %d and send it to EMA",eID);

	acString = malloc(40000);
	if (acString != NULL)
	{
		switch(eID)
		{
			case eLanStatServer:
				sprintf(acString,"%s","LanInfo");
				break;

			case eIpmiServer:
				sprintf(acString,"%s","Ipmi");
				break;

			case eMfa1DiagServer:
			case eMfa2DiagServer:
			case eMfa3DiagServer:
			case eMfa4DiagServer:
			case eMfa1ResetServer:
			case eMfa2ResetServer:
			case eMfa3ResetServer:
			case eMfa4ResetServer:
			case eSwitchDiagServer:
			case eMcmsCom:				// For enter diag mode indication only
			case eCpuDiagServer:
			{
				sprintf(acString,"%s","Diag");
				break;
			}
			case eEmaApiServer:
			    sprintf(acString,"%s","Control");
			    break;

	     	case eMaxTcpConnections:
			    sprintf(acString,"%s","Control");
		        break;


			default:
			{
				MfaBoardPrint(	TCP_SERVER_PRINT,
					PRINT_LEVEL_MAJOR,
					PRINT_TO_TERMINAL,
					"(StringWrapper): ERROR Unknown User ID (%d)!!!",eID);
				free(acString);
				free((void*)Msg);
				return;
			}
		}//switch(eID)
/*
		for (i = 0; i < 200; i++)
		{
			MfaBoardPrint(	TCP_SERVER_PRINT,
							PRINT_LEVEL_MAJOR,
							PRINT_TO_TERMINAL,
							"(StringWrapper): *Msg[%d] = %X",i, ((UINT32*)Msg)[i]);
		}
*/
		ptSpecGnrlHdr = (PTSpecGnrlHdr)Msg;
/*
		MfaBoardPrint(	TCP_SERVER_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"(StringWrapper): ptSpecGnrlHdr->ulMsgOffset = %d", ptSpecGnrlHdr->ulMsgOffset);

		MfaBoardPrint(	TCP_SERVER_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"(StringWrapper): ptSpecGnrlHdr = %d", ptSpecGnrlHdr);
*/
		IndHdr = IndHdrStart = (UINT32)(Msg + ptSpecGnrlHdr->ulMsgOffset);

		ptStructSpecPtr = (PTStructToStr)(Msg + sizeof(TSpecGnrlHdr));

		while( (UINT32)ptStructSpecPtr < IndHdrStart)
		{
			ptStructSpecNext = ptStructSpecPtr + 1;

			switch(ptStructSpecPtr->varType)
			{

				case e_unsignedLong:
				case e_signedLong:
				{
					UINT32 ulDword;

					for( i=0; i < ptStructSpecPtr->varCount; i++)
					{
						memcpy(&ulDword,(void*)IndHdr,sizeof(UINT32));
						if(ptStructSpecPtr->varType == e_signedLong)
							snprintf(acParam, sizeof(acParam),"$%s=%d",ptStructSpecPtr->varString,(INT32)ulDword);
						else snprintf(acParam, sizeof(acParam),"$%s=%u",ptStructSpecPtr->varString,ulDword);

						MfaBoardPrint(	TCP_SERVER_PRINT,
									PRINT_LEVEL_MAJOR,
									PRINT_TO_TERMINAL,
									"(StringWrapper): acParam = %s" ,acParam);

						strcat(acString,acParam); // Concatenate current param to msg string
						IndHdr+=ptStructSpecPtr->jumpOffst;

						if(!strcmp(ptStructSpecPtr->varString,"NumOfElem"))
						{
							ulTempNumOfElem = ulDword;
						}
						else if(!strcmp(ptStructSpecPtr->varString,"NumOfElemFields"))
						{
//							printf("Before StringWrapperArrayHandler - !! eID = %d , acString=(%s)\n",eID,acString);
							IndHdr = StringWrapperArrayHandler(acString,&ptStructSpecNext,IndHdr,ulTempNumOfElem,ulDword);
						}

					}

				}
				break;

				case e_unsignedChar:
				case e_signedChar:
				{
					UINT8 ulByte;

					for( i=0; i < ptStructSpecPtr->varCount; i++,IndHdr+=ptStructSpecPtr->jumpOffst )
					{
						memcpy(&ulByte,(void*)IndHdr,sizeof(UINT8));
						if(ptStructSpecPtr->varType == e_signedChar)
							snprintf(acParam, sizeof(acParam),"$%s=%d",ptStructSpecPtr->varString,(INT8)ulByte);
						else snprintf(acParam, sizeof(acParam),"$%s=%u",ptStructSpecPtr->varString,ulByte);

						MfaBoardPrint(	TCP_SERVER_PRINT,
										PRINT_LEVEL_MAJOR,
										PRINT_TO_TERMINAL,
										"(StringWrapper): acParam = %s" ,acParam);

						strcat(acString,acParam); // Concatenate current param to msg string
					}
				}
				break;

				case e_unsignedShort:
				case e_signedShort:
				{
					UINT16 ulShort;

					for( i=0; i < ptStructSpecPtr->varCount; i++,IndHdr+=ptStructSpecPtr->jumpOffst )
					{
						memcpy(&ulShort,(void*)IndHdr,sizeof(UINT16));
						if(ptStructSpecPtr->varType == e_signedShort)
							snprintf(acParam, sizeof(acParam),"$%s=%d",ptStructSpecPtr->varString,(INT16)ulShort);
						else snprintf(acParam, sizeof(acParam),"$%s=%u",ptStructSpecPtr->varString,ulShort);

						MfaBoardPrint(	TCP_SERVER_PRINT,
										PRINT_LEVEL_MAJOR,
										PRINT_TO_TERMINAL,
										"(StringWrapper): acParam = %s" ,acParam);
						strcat(acString,acParam); // Concatenate current param to msg string
					}
				}
				break;

				case e_string:
				{
					for( i=0; i < ptStructSpecPtr->varCount; i++,IndHdr+=ptStructSpecPtr->jumpOffst )
					{
						snprintf(acParam, sizeof(acParam),"$%s=%s",ptStructSpecPtr->varString,(char *)IndHdr);
						MfaBoardPrint(	TCP_SERVER_PRINT,
										PRINT_LEVEL_MAJOR,
										PRINT_TO_TERMINAL,
										"(StringWrapper): acParam = %s" ,acParam);
						strcat(acString,acParam); // Concatenate current param to msg string
					}
				}
				break;

			}

			// Move to next Struct Specification Descriptor
			ptStructSpecPtr = ptStructSpecNext;
		}//while( (UINT32)ptStructSpecPtr < IndHdrStart)



	//	printf("(StringWrapper): acString[%d]: %s\n",strlen(acString),acString);
	/*	MfaBoardPrint(	TCP_SERVER_PRINT,
				PRINT_LEVEL_MAJOR,
				PRINT_TO_TERMINAL,
				"(StringWrapper): acString[%d]: %s",strlen(acString),acString);*/

		/*
		//get the reset system cammand
		if (eID == eIpmiServer)
		{
			unEmaReset = GetResetSource((TEmaIndHeader  *)IndHdrStart);
		}
		*/

		// Send Wrapped String Message to EmaApiClient ...




		if (ptTcpCon->ul_ConnectionStatus == CONNECTED)
		{
			printf("+-- StringWrapper send to EMA --+\n");
			printf("%s\n", acString);
			printf("+-- ----------------------- --+\n");
			sendRetVal = TCPSendData( ptTcpCon->s, (VOID *)(acString), strlen(acString),0,BOARD_PRINT_YES);
			if ( sendRetVal  == SOCKET_OPERATION_FAILED)
			{
				MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(StringWrapper): problem(1) in sending data...");
				ptTcpCon->ul_ConnectionStatus = NOT_CONNECTED;
				ptTcpCon->ul_PrevConnState = CONNECTED;
                TraceDiagSwitchEmaCom(eSend, -1, sendRetVal, -1, "FAILED to send data");
			}
            else
            {
              //  TraceDiagSwitchEmaCom(eSend, -1, sendRetVal, -1, acString);
            }
		}
        else
        {
        	TraceDiagSwitchEmaCom(eSend, -1, -1, -1, "FAILED to TCPSendData, because ptTcpCon != CONNECTED");
            //MFA_PRINTF("FAILED to TCPSendData, because ptTcpCon != CONNECTED, data %s", acString);
        }
        


		if (ptTcpCon_forSim->ul_ConnectionStatus == CONNECTED)
		{
			// Send Wrapped DIAG String Message to EmaSimApiClient (only if connected)...
			if (((eID == eMfa1DiagServer ) || (eID == eMfa2DiagServer ) ||
				 (eID == eMfa3DiagServer ) || (eID == eMfa4DiagServer ) ||
				 (eID == eMfa1ResetServer ) || (eID == eMfa2ResetServer ) ||
				 (eID == eMfa3ResetServer ) || (eID == eMfa4ResetServer ) ||
				 (eID == eSwitchDiagServer ) || (eID == eCpuDiagServer ))
			&&   (ptTcpCon_forSim->ul_ConnectionStatus == CONNECTED))
			{
				if ( TCPSendData( ptTcpCon_forSim->s, (VOID *)acString, strlen(acString),0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
				{
					MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(StringWrapper): problem(2) in sending data...");
					ptTcpCon_forSim->ul_ConnectionStatus = NOT_CONNECTED;
					ptTcpCon_forSim->ul_PrevConnState = CONNECTED;

				// TBD: Find out if there is any need to send failure notice to EMA.
				}
			}
		}
        else
        {
            // always here
            //MFA_PRINTF("FAILED to TCPSendData, because ptTcpCon_forSim != CONNECTED, data %s", acString);
        }
     //   printf("Freeing string\n");
		free(acString);

	}//if (acString != NULL)

	//printf("Freeing Msg\n");
	free((void*)Msg);

}


UINT32 StringWrapperArrayHandler(UINT8* apucString,TStructToStr** pptStructSpecPtr,UINT32 IndHdr,UINT32 ulNumOfElem,UINT32 ulNumOfElemFields)
{
	UINT8 acParam[384];
	UINT32 i,ulElem,ulElemField,ulTempNumOfElem=0;

	PTStructToStr ptStructSpecPtr		= *pptStructSpecPtr;
	PTStructToStr ptFirstStructSpecPtr	= *pptStructSpecPtr;
	PTStructToStr ptStructSpecNext		= *pptStructSpecPtr;

	for( ulElem=0; ulElem < ulNumOfElem; ulElem++ )
	{
		ptStructSpecPtr = ptFirstStructSpecPtr;

		for( ulElemField=0; ulElemField < ulNumOfElemFields; ulElemField++ )
		{

			ptStructSpecNext = ptStructSpecPtr + 1;

			switch(ptStructSpecPtr->varType)
			{

			case e_unsignedLong:
			case e_signedLong:
			{
				UINT32 ulDword;

				for( i=0; i < ptStructSpecPtr->varCount; i++ )
				{
					memcpy(&ulDword,(void*)IndHdr,sizeof(UINT32));
					// Bracha:
					if(strlen(ptStructSpecPtr->varString) > 380)
					{
						MfaBoardPrint(	TCP_SERVER_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
						"(StringWrapper): ptStructSpecPtr->varString is to long");

//						ptStructSpecPtr->varString[380] = '\0';
//						MfaBoardPrint(	TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
//						"(StringWrapper): ptStructSpecPtr->varString = %s",ptStructSpecPtr->varString);
					}

					if(ptStructSpecPtr->varType == e_signedLong)
						snprintf(acParam,sizeof(acParam),"$%s=%d",ptStructSpecPtr->varString,(INT32)ulDword);
					else snprintf(acParam,sizeof(acParam),"$%s=%u",ptStructSpecPtr->varString,ulDword);
					strcat(apucString,acParam); // Concatenate current param to msg string
					IndHdr+=ptStructSpecPtr->jumpOffst;
					if(!strcmp(ptStructSpecPtr->varString,"NumOfElem"))
					{
						ulTempNumOfElem = ulDword;
						ulElemField--;
					}
					else if(!strcmp(ptStructSpecPtr->varString,"NumOfElemFields"))
					{
						IndHdr = StringWrapperArrayHandler(apucString,&ptStructSpecNext,IndHdr,ulTempNumOfElem,ulDword);
					}

				}

			}
			break;

			case e_unsignedChar:
			case e_signedChar:
			{
				UINT8 ulByte;

				for( i=0; i < ptStructSpecPtr->varCount; i++,IndHdr+=ptStructSpecPtr->jumpOffst )
				{
					memcpy(&ulByte,(void*)IndHdr,sizeof(UINT8));
					if(ptStructSpecPtr->varType == e_signedChar)
						snprintf(acParam,sizeof(acParam),"$%s=%d",ptStructSpecPtr->varString,(INT8)ulByte);
					snprintf(acParam,sizeof(acParam),"$%s=%u",ptStructSpecPtr->varString,ulByte);
					strcat(apucString,acParam); // Concatenate current param to msg string
				}
			}
			break;

			case e_unsignedShort:
			case e_signedShort:
			{
				UINT16 ulShort;

				for( i=0; i < ptStructSpecPtr->varCount; i++,IndHdr+=ptStructSpecPtr->jumpOffst )
				{
					memcpy(&ulShort,(void*)IndHdr,sizeof(UINT16));
					if(ptStructSpecPtr->varType == e_signedShort)
						snprintf(acParam,sizeof(acParam),"$%s=%d",ptStructSpecPtr->varString,(INT16)ulShort);
					else snprintf(acParam,sizeof(acParam),"$%s=%u",ptStructSpecPtr->varString,ulShort);
					strcat(apucString,acParam); // Concatenate current param to msg string
				}
			}
			break;

			case e_string:
			{
				for( i=0; i < ptStructSpecPtr->varCount; i++,IndHdr+=ptStructSpecPtr->jumpOffst )
				{
					snprintf(acParam,sizeof(acParam),"$%s=%s",ptStructSpecPtr->varString,(char *)IndHdr);
					strcat(apucString,acParam); // Concatenate current param to msg string
				}
			}
			break;

			}


			// Move to next Struct Specification Descriptor
			ptStructSpecPtr = ptStructSpecNext;

		} // elem fields

	}// elem

	if (ulElem == 0)
	 ptStructSpecPtr += ulNumOfElemFields;
	*pptStructSpecPtr = ptStructSpecPtr;
	return IndHdr;
}


// ParseLoginTrans parses only the 2 following login commands:
// 1)	UserName ???? UserPassword ???? StationName ????
// 2)	UserName ???? UserPassword ????
// ----------------------------------
// "Control$Opcode=1$MsgID=14$SlotID=-1$UserName=POLYCOM$UserPassword=asdfASDF1234!@#$$StationName=EMA.F3-KOBIG"
// "Control$Opcode=1$MsgID=14$SlotID=-1$UserName=POLYCOM$UserPassword=asdfASDF1234!@#$"
// Important: At the end (2 option) there is no delimiter.
int ParseLoginTrans(char *command, char *outCommandArgs[], int *outArgs, unsigned int *outMsgId)
{
    char *pcTokenMsgId = 0;
    char *pcValMsgId = 0;
    char  msgId[10];
    char *pcMgsIdEnd = 0;

    char *pcTokenUser = 0;
    char *pcTokenPassword = 0;
    char *pcValUser = 0;
    char *pcValPassword = 0;
    char *pcTokenStation = 0;
    int   add_index = 0;
    char commandLen = strlen(command);


    // Message ID
    pcTokenMsgId = strstr(command, "MsgID=");
    if(NULL == pcTokenMsgId)
    {
        return -1;
    }
    pcValMsgId = pcTokenMsgId + strlen("MsgID=");
    pcMgsIdEnd = strstr(pcValMsgId, "$");
    memcpy(msgId, pcValMsgId, pcMgsIdEnd - pcValMsgId);
    msgId[pcMgsIdEnd - pcValMsgId] = '\0';
    *outMsgId = atoi(msgId);

    // User Name
    pcTokenUser = strstr(command, "UserName=");
    if(NULL == pcTokenUser)
    {
        return -1;
    }
    pcValUser = pcTokenUser + strlen("UserName=");

    // Password
    pcTokenPassword = strstr(command, "UserPassword=");
    if(NULL == pcTokenPassword)
    {
        return -1;
    }
    pcValPassword = pcTokenPassword + strlen("UserPassword=");

    // from this point ParseLoginTrans cannot return -1, command changed.
    pcValUser[pcTokenPassword - pcValUser - 1] = '\0';

    add_index = 1;
    pcTokenStation = strstr(pcValPassword, "StationName=");
    if(NULL == pcTokenStation)
    {
        pcTokenStation = command + commandLen;
        add_index = 0;
    }

    pcValPassword[pcTokenStation - pcValPassword - add_index] = '\0';

    outCommandArgs[0] = pcValUser;
    outCommandArgs[1] = pcValPassword;
    *outArgs = 2;

    return 0;
}


void StringStripper(UINT8* commandRcvd,UINT32 NumBytes)
{
	int		i=0,Args=0;
	char	*pcToken,prString,*pstrString;
	char	*commandArgs[MAX_PARAMS_IN_STRING];
	UINT32   ulReqSize=0;
	UINT32*  Req=NULL;
	UINT32*  ReqPtr = NULL;
	t_TcpConnParams* ptTcpCon = NULL;
	UINT8 *command=NULL;
	UINT32 ulOpcode = 0;
	UINT32 ulSlotId = 0;
	UINT32 ulMsgId = 0;
	UINT32*  swappedReq=NULL;
	int rc = 0;

	command = malloc(NumBytes + 1);
	if (command==NULL)
	{
		printf("Yosi - command==NULL StripperExit");
		goto StripperExit;
	}

	memcpy(command,commandRcvd,NumBytes);
	command[NumBytes]=0;
	MfaBoardPrint(	TCP_SERVER_PRINT,
		PRINT_LEVEL_MAJOR,
		PRINT_TO_TERMINAL,
		"(StringStripper): Entry (%s).",command);
	
	printf("the command to strip is (%s)\n",command);
	if (command[0] == 0)
	{
		printf("Yosi - command[0] == 0 StripperExit");
		goto StripperExit;
	}

	memset(commandArgs,0,sizeof(commandArgs));

	 rc = -1;
     // login transaction, parse it differently because password can contain $ - delimiter
     if(NULL != strstr(command, "UserName=") && NULL != strstr(command, "UserPassword="))
     {
         rc = ParseLoginTrans(command, commandArgs, &Args, &ulMsgId);
         
         MfaBoardPrint(	TCP_SERVER_PRINT,
                        PRINT_LEVEL_ERROR,
                        PRINT_TO_TERMINAL,
                        "(StringStripper): ParseLoginTrans returned %d",
	                        rc);
         
         if(-1 != rc)
         {
             printf("Yosi - inside if login transaction rc =%d after ParseLoginTrans inside if(-1 != rc) ",rc);
             handleEmaLoginMsg(commandArgs, Args, ulMsgId, ulSlotId);
          //   BuildControlResponseMsg(ulMsgId, ulSlotId);
             goto StripperExit;
         }
     }
     
     // not login OR login parser failed
     //PAVELK - disabled for debug
     
     if(-1 == rc)
     {
//         printf ("yosi - not login OR login parser failed -1 == rc");
         pcToken = strtok( command , "$" );
         while( pcToken != NULL )
         {
             commandArgs[i] = pcToken;
             pcToken = strtok( NULL, "$" );
             i++;
             if (i >= MAX_PARAMS_IN_STRING)
             {
                 MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,	"(%s): Out of range - Over than %d Parameters",__FUNCTION__, MAX_PARAMS_IN_STRING);
                 goto StripperExit;
             }
         }
         Args = i;
         
     }
     memset(reqBuffer, 0, (MAX_PARAMS_IN_STRING * MAX_STRING_PARAM));
     ReqPtr = (UINT32*)&reqBuffer[0];


	// NOTE: Copy all String Params into a buffer - assume they are all dwords !!!
	for(i = 1; i < Args; i++)
	{
		prString=0;
		if (!strcmp(commandArgs[0],"Ipmi"))
		{
			if ((i>4)&&(ulOpcode==521))
				prString=1;
		}
		if (prString)
		{
			if(NULL == (pstrString = strstr(commandArgs[i],"="))) continue;
			sscanf(pstrString,"=%23s",(char *)ReqPtr);
			ulReqSize += MAX_STRING_PARAM;
			ReqPtr[MAX_STRING_PARAM - 1]=0;
		}
		else
		{
			if(NULL == (pstrString = strstr(commandArgs[i],"="))) continue;
			sscanf(pstrString,"=%d",ReqPtr);
			ulReqSize += sizeof(UINT32);
		}

		if(i == 1)
			ulOpcode = *ReqPtr;
		if(i == 2)
			ulMsgId = *ReqPtr;
		if(i == 3)
			ulSlotId = *ReqPtr;

		if (prString)
			MfaBoardPrint(	TCP_SERVER_PRINT,
				PRINT_LEVEL_MAJOR,
				PRINT_TO_TERMINAL,"(StringStripper): Param%d: %s.",i,ReqPtr);
		else
			MfaBoardPrint(	TCP_SERVER_PRINT,
				PRINT_LEVEL_MAJOR,
				PRINT_TO_TERMINAL,"(StringStripper): Param%d: %d.",i,*ReqPtr);

		if (prString)
			ReqPtr = ReqPtr + MAX_STRING_PARAM/4;
		else ReqPtr++;
	}


	if(!strcmp(commandArgs[0],"LanInfo"))
	{
		ptTcpCon = &TcpConnection[eLanStatServer];
	}
	else if(!strcmp(commandArgs[0],"Ipmi"))
	{
		ptTcpCon = &TcpConnection[eIpmiServer];
	}
	else if(!strcmp(commandArgs[0],"Control"))
	{
//		printf("Yosi -commandArgs Control" );
		handleEmaCtlMsg(commandArgs,Args);
		BuildControlResponseMsg(ulMsgId, ulSlotId);
		goto StripperExit;
	}
	else if(!strcmp(commandArgs[0],"Diag"))
	{
		MfaBoardPrint(	TCP_SERVER_PRINT,
                        PRINT_LEVEL_MAJOR,
                        PRINT_TO_TERMINAL,
                        "(StringStripper): Recieved Diag message (Opcode=%Xh,SlotId=%d).",ulOpcode,ulSlotId);

        TraceDiagSwitchEmaCom(eRecv, ulMsgId, ulOpcode, ulSlotId, "None");

//		printf("received diag message , opcode = %d, ulSlotId = %d GetChasisPlatformTypeInDword(1)=%d\n",ulOpcode,ulSlotId,GetChasisPlatformTypeInDword(1));

        switch(ulOpcode)
		{
		case EMA_ENTER_DIAG_MODE_REQ:
			{
			MfaBoardPrint(	TCP_SERVER_PRINT,
				PRINT_LEVEL_MAJOR,
				PRINT_TO_TERMINAL,
				"(StringStripper): Enter diag mode request for board no %d.",ulSlotId);
			sendMsgFromEmaToDiag(ulReqSize,&(reqBuffer[0]));
			goto StripperExit;
			}
		case EMA_GET_TEST_LIST_REQ:
		case EMA_START_TEST_REQ:
		case EMA_GET_TEST_STATUS_REQ:
		case EMA_GET_UNITS_STATE_REQ:
		case EMA_STOP_TEST_REQ:
		case EMA_GET_ERROR_LIST_REQ:
			{
			MfaBoardPrint(	TCP_SERVER_PRINT,
				PRINT_LEVEL_MAJOR,
				PRINT_TO_TERMINAL,
				"(StringStripper): Request opcode %X for board no %d.", ulOpcode, ulSlotId);

			sendMsgFromEmaToDiag(ulReqSize,&(reqBuffer[0]));
			goto StripperExit;
			}
		default:
			printf("\n!!!!!!Unknown Diag Message ulOpcode %X slotId %d!!!!\n", ulOpcode, ulSlotId);
			goto StripperExit;
		}
	}
	else
	{
			MfaBoardPrint(	TCP_SERVER_PRINT,
			PRINT_LEVEL_ERROR,
			PRINT_TO_TERMINAL,
			"(StringStripper): Unknown Msg Type !!! (%s)",commandArgs[0]);

			goto StripperExit;
	}

//	printf("sending from StringStripper over socket %d\n",ptTcpCon->s);
	// Send Parsed Message to Relevant TCP Client ...
	if (ptTcpCon != NULL && ptTcpCon->ul_ConnectionStatus == CONNECTED)
	{
//		if ( TCPSendData( ptTcpCon->s, (VOID *)Req, ulReqSize,0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
		if ( TCPSendData( ptTcpCon->s, (VOID *)&reqBuffer[0], ulReqSize,0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
		{
			printf("failed sending tcp data from StringStripper\n");
			MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(StringStripper): problem in sending data...");
			ptTcpCon->ul_ConnectionStatus = NOT_CONNECTED;
			ptTcpCon->ul_PrevConnState = CONNECTED;

			// TBD: Find out if there is any need to send failure notice to EMA.
		}
	}

   	MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(StringStripper): Data sent. size = %d", ulReqSize);


StripperExit:

	MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(StringStripper): StripperExit");

	free(command);
	free(commandRcvd);

	return;
}


// Benson - Simulate the Ema Requset for Lan Statistics Info...
void BuildLanStatInfoReq(UINT32 ulOpcode)
{
	INT8 acMsg[256];
	TEmaReqHeader tEmaReqHdr;
	t_TcpConnParams* ptConParams = &TcpConnection[eEmaApiClient];
	static int flag = 1;


	tEmaReqHdr.ulMsgID  		= 0x1234;
	//tEmaReqHdr.ulOpcode 		= LAN_STAT_INFO_REQ;
	//tEmaReqHdr.ulOpcode 		= LAN_STAT_CLEAR_MAX_REQ;
	//tEmaReqHdr.ulOpcode 		= LAN_STAT_GET_PORTS_LIST_REQ;
	tEmaReqHdr.ulOpcode 		= ulOpcode;
	tEmaReqHdr.ulSlotID 		= 0;


	//BuildLanStatInfoReq(&tLanStatInfoReq);

	sprintf(acMsg,"LanInfo$Opcode=%d$MsgID=%d$SlotID=%d",
			tEmaReqHdr.ulOpcode,
			tEmaReqHdr.ulMsgID,
			tEmaReqHdr.ulSlotID);


	printf("(BuildLanStatInfoReq): BASE0:: acMsg: %s.",acMsg);

	if(flag)
	{
		// Connect to EmaApi TcpServer...
		ptConParams->e_Id 		= eEmaApiClient;
		ptConParams->us_Port 	= LISTEN_EMA_PORT;
		strcpy(&ptConParams->IpV4Addr.auc_IpV4Address[0]   , "127.0.0.1"	);
		flag = 0;

		ConnectToTcpServer(ptConParams);
	}

	//EmaApiConnect();

	// Send String Message to EmaApi TcpServer...
    if (ptConParams->ul_ConnectionStatus == CONNECTED)
    {
		if ( TCPSendData( ptConParams->s, (VOID *)acMsg, 256,0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
		{
    		MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(BuildLanStatInfoReq): problem in sending data...");
        	ptConParams->ul_ConnectionStatus = NOT_CONNECTED;
        	ptConParams->ul_PrevConnState = CONNECTED;
		}
	}

}


void EmaListenThread()
{
	SOCKET lEMAListenSock;
	SOCKET lEMAActiveSock;
	struct sockaddr_in tServerSaddrIn;
	struct sockaddr_in tActiveSockAddr;
    socklen_t nSizeofSocketaddrIn = sizeof(struct sockaddr_in);
    INT32 lSockOpRc ;

	EnrollInThreadList(eEmaListenThread);

   	MfaBoardPrint(	TCP_SERVER_PRINT,
   					PRINT_LEVEL_ERROR,
   					PRINT_TO_TERMINAL,
   					"(EmaListenThread): Start Running...");

    lEMAListenSock = InitV4Connection();

	// Bracha: For apache module debugging, connect to external port
//	IP_ADDR		mngIpAddr;
//	GET_MNG_V4_SM_IP_ADDRESS(&mngIpAddr.aAddr);
//	tServerSaddrIn.sin_addr.s_addr 	= inet_addr(mngIpAddr.aAddr);

	while(1)
	{
		memset(&tServerSaddrIn, 0, sizeof(struct sockaddr_in));

	// Bracha: For apache module debugging, disable this line. Connect to external port instead
		tServerSaddrIn.sin_addr.s_addr 	= inet_addr("127.0.0.1");
		tServerSaddrIn.sin_port 		= htons(LISTEN_EMA_PORT);
		tServerSaddrIn.sin_family 		= AF_INET;

		MfaBoardPrint(	TCP_SERVER_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"(EmaListenThread): tServerSaddrIn.sin_addr.s_addr = %x, ListenSock = %d ",tServerSaddrIn.sin_addr.s_addr, lEMAListenSock);
		
	  	lSockOpRc = bind(lEMAListenSock,(struct sockaddr *) &tServerSaddrIn,sizeof(tServerSaddrIn));
	  	if (lSockOpRc < 0)
	  	{
		    	MfaBoardPrint(	TCP_SERVER_PRINT,
		    					PRINT_LEVEL_ERROR,
		    					PRINT_TO_TERMINAL,
		    					"(EmaListenThread): Bind Error %d !!!",GetLastError());

	//		MfaBoardPrint(	TCP_SERVER_PRINT,
	//						PRINT_LEVEL_ERROR,
	//						PRINT_TO_TERMINAL,
	//						"(EmaListenThread): tServerSaddrIn.sin_addr.s_addr = %s ",tServerSaddrIn.sin_addr.s_addr);

			EmbSleep(1000);
		}
		else break;
	}

  	MfaBoardPrint(	TCP_SERVER_PRINT,
   					PRINT_LEVEL_ERROR,
   					PRINT_TO_TERMINAL,
   					"(EmaListenThread): Listen on socket %d ...",lEMAListenSock);

  	while(g_isServiceRun)
  	{
  		lSockOpRc = listen(lEMAListenSock, MAX_NUM_OF_EMA_CLIENTS);
  		if(lSockOpRc < 0 )
  		{
  			MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(EmaListenThread): Listen Error %d !!!",GetLastError());
  		}
  		else if(lSockOpRc == 0)
  		{
  			lEMAActiveSock = accept(lEMAListenSock,(struct sockaddr *) &tActiveSockAddr,&nSizeofSocketaddrIn);
  			if(lEMAActiveSock < 0)
  			{
  				MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(EmaListenThread): Accept Error %d !!!",GetLastError());
  			}
  			else
  			{
  				struct in_addr tIn;
  				t_TcpConnParams* ptTcpCon = &TcpConnection[eEmaApiServer];

  				ptTcpCon->s 		= lEMAActiveSock;
  				ptTcpCon->us_Port 	= tActiveSockAddr.sin_port;
				ptTcpCon->e_Id 		= eEmaApiServer;
					ptTcpCon->ul_ClientOrServer	= eConnTypeServer;

				tIn.s_addr = tActiveSockAddr.sin_addr.s_addr;
				strcpy(&ptTcpCon->IpV4Addr.auc_IpV4Address[0] , (char*)inet_ntoa(tIn) );
				//strcpy(&ptTcpCon->IpV4Addr.auc_NetMask[0]       , "255.255.255.0");
				//strcpy(&ptTcpCon->IpV4Addr.auc_DefaultGetway[0] , "127.0.0.0"	);

				ptTcpCon->ul_PrevConnState = CONNECTED;
				ptTcpCon->ul_ConnectionStatus = CONNECTED;

				MfaBoardPrint(	TCP_SERVER_PRINT,
									PRINT_LEVEL_ERROR,
									PRINT_TO_TERMINAL,
									"(EmaListenThread): eEmaApiServer client params:: s:%d,ip:%s,port:%d.",
									ptTcpCon->s,
									&(ptTcpCon->IpV4Addr.auc_IpV4Address),
									ptTcpCon->us_Port);

  				FD_SET(lEMAActiveSock,ptTcpActiveReadSockets);

  				RegisterToTimerJob(ptTcpCon, 500, CheckServerConnStatus);
  				g_emaTcpConnParamsInd=(g_emaTcpConnParamsInd+1)%MAX_EMA_CONN;
				memcpy(&g_emaTcpConnParams[g_emaTcpConnParamsInd],ptTcpCon,sizeof(t_TcpConnParams));
                reStartEmaWatchdogTimer();
  			}
  		}
  	}
}


void emaAppListenThread(UINT32 appPort,UINT32 appId)
{
	SOCKET lAppListenSock;
	SOCKET lAppActiveSock;
	struct sockaddr_in tServerSaddrIn;
	struct sockaddr_in tActiveSockAddr;
    socklen_t nSizeofSocketaddrIn = sizeof(struct sockaddr_in);
    INT32 lSockOpRc ;

   	MfaBoardPrint(	TCP_SERVER_PRINT,
   					PRINT_LEVEL_ERROR,
   					PRINT_TO_TERMINAL,
   					"(emaAppListenThread): Start Running... port %x",appPort);

   	//printf("(emaAppListenThread): Start Running... port %x",appPort);
	lAppListenSock = InitV4Connection();
	
	while(1)
	{
		memset(&tServerSaddrIn, 0, sizeof(struct sockaddr_in));

	    tServerSaddrIn.sin_addr.s_addr 	= inet_addr("127.0.0.1");
		tServerSaddrIn.sin_port  		= htons(appPort);
		tServerSaddrIn.sin_family 		= AF_INET;
  		lSockOpRc = bind(lAppListenSock,(struct sockaddr *) &tServerSaddrIn,sizeof(tServerSaddrIn));
  		if (lSockOpRc < 0)
  		{
    			MfaBoardPrint(	TCP_SERVER_PRINT,
    					PRINT_LEVEL_ERROR,
    					PRINT_TO_TERMINAL,
    					"(emaAppListenThread): Bind appPort %d Error %d !!!",appPort,GetLastError());
			EmbSleep(1000);
  		}
		else break;
	}

  		MfaBoardPrint(	TCP_SERVER_PRINT,
   					PRINT_LEVEL_ERROR,
   					PRINT_TO_TERMINAL,
   					"(emaAppListenThread): Listen appPort %d on nSocket %d",appPort,lAppListenSock);


  	while(g_isServiceRun)
  	{
  		lSockOpRc = listen(lAppListenSock, MAX_NUM_OF_EMA_CLIENTS);
  		if(lSockOpRc < 0 )
  		{
  			MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(emaAppListenThread): Listen appPort %x Error %d !!!",appPort,GetLastError());
  		}
  		else if(lSockOpRc == 0)
  		{
  			memset(&tActiveSockAddr, 0, sizeof(struct sockaddr_in));
  			lAppActiveSock = accept(lAppListenSock,(struct sockaddr *) &tActiveSockAddr,&nSizeofSocketaddrIn);
  			if(lAppActiveSock < 0)
  			{
  				MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(emaAppListenThread): Accept appPort %x Error %d !!!",appPort,GetLastError());
  			}
  			else
  			{
				struct in_addr tIn;
				t_TcpConnParams* ptTcpCon = &TcpConnection[appId];

				ptTcpCon->s 		= lAppActiveSock;
				ptTcpCon->us_Port 	= tActiveSockAddr.sin_port;
				ptTcpCon->e_Id 		= appId;

				tIn.s_addr = tActiveSockAddr.sin_addr.s_addr;
				strcpy(&ptTcpCon->IpV4Addr.auc_IpV4Address[0] , (char*)inet_ntoa(tIn) );
				//strcpy(&ptTcpCon->IpV4Addr.auc_NetMask[0]       , "255.255.255.0");
				//strcpy(&ptTcpCon->IpV4Addr.auc_DefaultGetway[0] , "127.0.0.0"	);

				ptTcpCon->ul_PrevConnState = CONNECTED;
				ptTcpCon->ul_ConnectionStatus = CONNECTED;

//				printf("server params: appPort = %d", appPort);
//				printf("client params: s = %d", ptTcpCon->s);
//				printf("client params: ip = %s", &(ptTcpCon->IpV4Addr.auc_IpV4Address));
//				printf("client params: port = %d", ptTcpCon->us_Port);

				MfaBoardPrint(	TCP_SERVER_PRINT,
								PRINT_LEVEL_ERROR,
								PRINT_TO_TERMINAL,
								"(emaAppListenThread):server appPort %x  client params:: s:%d,ip:%s,port:%d.",
								appPort,
								ptTcpCon->s,
								&(ptTcpCon->IpV4Addr.auc_IpV4Address),
								ptTcpCon->us_Port);

				FD_SET(lAppActiveSock,ptTcpActiveReadSockets);

				RegisterToTimerJob(ptTcpCon, 500, CheckServerConnStatus);
  			}
  		}
  	}
}

void LanStatListenThread()
{
	EnrollInThreadList(eLanStatListenThread);

   emaAppListenThread(LISTEN_LAN_STAT_PORT,eLanStatServer);
}

void IpmiListenThread()
{
   EnrollInThreadList(eIpmiListenThread);

   emaAppListenThread(LISTEN_IPMI_PORT,eIpmiServer);
}

void SwitchDiagListenThread()
{

   EnrollInThreadList(eSwitchDiagListenThread);

   emaAppListenThread(LISTEN_SWITCH_DIAG_PORT, eSwitchDiagServer);
}

//l.a. listenThread for EmaSim
void EmaSimulationListenThread()
{
	SOCKET lMFA_EmaSim_ListenSock;
	SOCKET lMFA_EmaSim_ActiveSock;
	struct sockaddr_in tServerSaddrIn;
	struct sockaddr_in tActiveSockAddr;
    socklen_t nSizeofSocketaddrIn = sizeof(struct sockaddr_in);
    INT32 lSockOpRc ;
	INT32  lTmp;
	Ni_Params t_V4NiAddress;

	EnrollInThreadList(eEmaSimulationListenThread);

	GetV4NiParams(tIfNameStruct.acIfName, &t_V4NiAddress); // Switch external ip address

	MfaBoardPrint(TCP_SERVER_PRINT,
				  PRINT_LEVEL_ERROR,
				  PRINT_TO_TERMINAL,"(EmaSimulationListenThread): From GetV4NiParams:: ip:%X,subnet:%X,def_gateway:%X.",
				  t_V4NiAddress.ulIpAddr,
				  t_V4NiAddress.ulSubNetMask,
				  t_V4NiAddress.ulDefGateWay);

   	MfaBoardPrint(	TCP_SERVER_PRINT,
   					PRINT_LEVEL_ERROR,
   					PRINT_TO_TERMINAL,
   					"(EmaSimulationListenThread): Start Running...");

    lMFA_EmaSim_ListenSock = InitV4Connection();

    tServerSaddrIn.sin_addr.s_addr 	= t_V4NiAddress.ulIpAddr;
	tServerSaddrIn.sin_port  		= htons(LISTEN_EMA_SIM_PORT);
	tServerSaddrIn.sin_family 		= AF_INET;

  	lSockOpRc = bind(lMFA_EmaSim_ListenSock,(struct sockaddr *) &tServerSaddrIn,sizeof(tServerSaddrIn));
  	if (lSockOpRc < 0)
  	{
    	MfaBoardPrint(	TCP_SERVER_PRINT,
    					PRINT_LEVEL_ERROR,
    					PRINT_TO_TERMINAL,
    					"(EmaSimulationListenThread): Bind Error %d !!!",GetLastError());

		//l.a.  and now kill thread???
  	}

  	MfaBoardPrint(	TCP_SERVER_PRINT,
   					PRINT_LEVEL_ERROR,
   					PRINT_TO_TERMINAL,
   					"(EmaSimulationListenThread): Listen on socket %d ...",lMFA_EmaSim_ListenSock);

  	while(g_isServiceRun)
  	{
  		lSockOpRc = listen(lMFA_EmaSim_ListenSock, MAX_NUM_OF_MFA_DIAG_CLIENTS);
  		if(lSockOpRc < 0 )
  		{
  			MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(EmaSimulationListenThread): Listen Error %d !!!",GetLastError());
  		}
  		else if(lSockOpRc == 0)
  		{
  			lMFA_EmaSim_ActiveSock = accept(lMFA_EmaSim_ListenSock,(struct sockaddr *) &tActiveSockAddr,&nSizeofSocketaddrIn);
  			if(lMFA_EmaSim_ActiveSock < 0)
  			{
  				MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(EmaSimulationListenThread): Accept Error %d !!!",GetLastError());
  			}
  			else
  			{
  					struct in_addr tIn;
					t_TcpConnParams tTcpCon;
  					t_TcpConnParams* ptTcpCon;

					memset(&tTcpCon, 0, sizeof(t_TcpConnParams));

  					tTcpCon.s 		= lMFA_EmaSim_ActiveSock;
  					tTcpCon.us_Port	= tActiveSockAddr.sin_port;
					tTcpCon.ul_ClientOrServer	= eConnTypeServer;


					tIn.s_addr = tActiveSockAddr.sin_addr.s_addr;
					strcpy(&tTcpCon.IpV4Addr.auc_IpV4Address[0] , (char*)inet_ntoa(tIn) );
					//strcpy(&ptTcpCon->IpV4Addr.auc_NetMask[0]       , "255.255.255.0");
					//strcpy(&ptTcpCon->IpV4Addr.auc_DefaultGetway[0] , "127.0.0.0"	);

//					tTcpCon.ul_PrevConnState = CONNECTED;

					MfaBoardPrint(	TCP_SERVER_PRINT,
									PRINT_LEVEL_ERROR,
									PRINT_TO_TERMINAL,
									"(EmaSimListenThread): dbg tmp eEmaSimServer client ip:%s",
								    &(tTcpCon.IpV4Addr.auc_IpV4Address) );


					tTcpCon.e_Id = eEmaSimServer;

  					ptTcpCon = &TcpConnection[tTcpCon.e_Id];
					memcpy(ptTcpCon, &tTcpCon, sizeof(t_TcpConnParams));
					ptTcpCon->ul_PrevConnState = CONNECTED;
					ptTcpCon->ul_ConnectionStatus = CONNECTED;

					MfaBoardPrint(	TCP_SERVER_PRINT,
									PRINT_LEVEL_ERROR,
									PRINT_TO_TERMINAL,
									"(EmaSimulationListenThread): eEmaSimServer client params: s:%d,ip:%s,port:%d.",
									ptTcpCon->s,
								    &(ptTcpCon->IpV4Addr.auc_IpV4Address),
									ptTcpCon->us_Port);


  					FD_SET(lMFA_EmaSim_ActiveSock,ptTcpActiveReadSockets);
					RegisterToTimerJob(ptTcpCon, 500, CheckServerConnStatus);

  			}
  		}
  	}
}

/* listenThread (server) for Cpu Diagnostics
** the connection opened here is used to send diagnostics requests to the MCMS.
*/
void CpuDiagListenThread()
{
	SOCKET lCpu_Diag_ListenSock;
	SOCKET lCpu_Diag_ActiveSock;
	struct sockaddr_in tServerSaddrIn;
	struct sockaddr_in tActiveSockAddr;
    socklen_t nSizeofSocketaddrIn = sizeof(struct sockaddr_in);
    INT32 lSockOpRc ;
	INT32  lTmp;
	Ni_Params t_V4NiAddress;

	EnrollInThreadList(eCpuDiagListenThread);

	GetV4NiParams(tIfNameStruct.acIfNameVlan, &t_V4NiAddress);  // Switch card address on the platform network (169.254.128.16)

	MfaBoardPrint(TCP_SERVER_PRINT,
				  PRINT_LEVEL_ERROR,
				  PRINT_TO_TERMINAL,"(CpuDiagListenThread): From GetV4NiParams:: ip:%X,subnet:%X,def_gateway:%X.",
				  t_V4NiAddress.ulIpAddr,
				  t_V4NiAddress.ulSubNetMask,
				  t_V4NiAddress.ulDefGateWay);

   	MfaBoardPrint(	TCP_SERVER_PRINT,
   					PRINT_LEVEL_ERROR,
   					PRINT_TO_TERMINAL,
   					"(CpuDiagListenThread): Start Running...");

    lCpu_Diag_ListenSock = InitV4Connection();

    tServerSaddrIn.sin_addr.s_addr 	= t_V4NiAddress.ulIpAddr;
	tServerSaddrIn.sin_port  		= htons(LISTEN_CPU_DIAG_PORT);
	tServerSaddrIn.sin_family 		= AF_INET;

  	lSockOpRc = bind(lCpu_Diag_ListenSock,(struct sockaddr *) &tServerSaddrIn,sizeof(tServerSaddrIn));
  	if (lSockOpRc < 0)
  	{
    	MfaBoardPrint(	TCP_SERVER_PRINT,
    					PRINT_LEVEL_ERROR,
    					PRINT_TO_TERMINAL,
    					"(CpuDiagListenThread): Bind Error %d !!!",GetLastError());

  	}

  	MfaBoardPrint(	TCP_SERVER_PRINT,
   					PRINT_LEVEL_ERROR,
   					PRINT_TO_TERMINAL,
   					"(CpuDiagListenThread): Listen on socket %d ...",lCpu_Diag_ListenSock);

  	while(g_isServiceRun)
  	{
  		lSockOpRc = listen(lCpu_Diag_ListenSock, MAX_NUM_OF_CPU_DIAG_CLIENTS);
  		if(lSockOpRc < 0 )
  		{
  			MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(CpuDiagListenThread): Listen Error %d !!!",GetLastError());
  		}
  		else if(lSockOpRc == 0)
  		{
  			lCpu_Diag_ActiveSock = accept(lCpu_Diag_ListenSock,(struct sockaddr *) &tActiveSockAddr,&nSizeofSocketaddrIn);
  			if(lCpu_Diag_ActiveSock < 0)
  			{
  				MfaBoardPrint(TCP_SERVER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
  				"(CpuDiagListenThread): Accept Error %d !!!",GetLastError());
  			}
  			else
  			{
  					struct in_addr tIn;
					t_TcpConnParams tTcpCon;
  					t_TcpConnParams* ptTcpCon;

					memset(&tTcpCon, 0, sizeof(t_TcpConnParams));

  					tTcpCon.s 					= lCpu_Diag_ActiveSock;
  					tTcpCon.us_Port				= tActiveSockAddr.sin_port;
					tTcpCon.ul_ClientOrServer	= eConnTypeServer;

					tIn.s_addr = tActiveSockAddr.sin_addr.s_addr;
					strcpy(&tTcpCon.IpV4Addr.auc_IpV4Address[0] , (char*)inet_ntoa(tIn) );

					MfaBoardPrint(	TCP_SERVER_PRINT,
									PRINT_LEVEL_ERROR,
									PRINT_TO_TERMINAL,
									"(CpuDiagListenThread): dbg tmp eCpuDiagServer client ip:%s",
								    &(tTcpCon.IpV4Addr.auc_IpV4Address) );

					tTcpCon.e_Id = eCpuDiagServer;

//					Bracha: are there 2 cpu?
//					//check the ip of the connecting board:
//					lTmp = strncmp( &tTcpCon.IpV4Addr.auc_IpV4Address[0],"169.254.128.67", V4_ADDRESS_SIZE);
//					if(lTmp == 0)
//						tTcpCon.e_Id = eMfa1ResetServer;
//					else
//					{
//						lTmp = strncmp( &tTcpCon.IpV4Addr.auc_IpV4Address[0], "169.254.128.68", V4_ADDRESS_SIZE);
//						if(lTmp == 0)
//							tTcpCon.e_Id = eMfa2ResetServer;
//					}

//					//if have not recognized client ip - ignore request:
//					if(lTmp != 0)
//					{
//						MfaBoardPrint(	TCP_SERVER_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,"(MfaResetListenThread): Error: ignoring unrecognized Mfa Ip %s. ", &(tTcpCon.IpV4Addr.auc_IpV4Address) );
//						continue;
//					}

  					ptTcpCon = &TcpConnection[tTcpCon.e_Id];
					memcpy(ptTcpCon, &tTcpCon, sizeof(t_TcpConnParams));
					ptTcpCon->ul_PrevConnState = CONNECTED;
					ptTcpCon->ul_ConnectionStatus = CONNECTED;

					MfaBoardPrint(	TCP_SERVER_PRINT,
									PRINT_LEVEL_ERROR,
									PRINT_TO_TERMINAL,
									"(CpuDiagListenThread): eCpuDiagServer client params: s:%d,ip:%s,port:%d.",
									ptTcpCon->s,
								    &(ptTcpCon->IpV4Addr.auc_IpV4Address),
									ptTcpCon->us_Port);


  					FD_SET(lCpu_Diag_ActiveSock,ptTcpActiveReadSockets);

					RegisterToTimerJob(ptTcpCon, 500, CheckServerConnStatus);

  			}
  		}
  	}
}



// Bracha - SWAP message before sending to Mcms
//UINT32* swapMessage(UINT32* rcvBuf, UINT32 bufSize)
void swapMessage(UINT32* rcvBuf, UINT32 bufSize)
{
	UINT32*		swappedBuffer;
	UINT32		i;

	if(NULL == (swappedBuffer = (UINT32*)malloc(bufSize))) return;

	memset(swappedBuffer, 0, bufSize);

	for(i = 0; i < (bufSize / sizeof(UINT32)); i++)
	{
		MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
		"swapMessage: rcvBuf = 0x%X", *(rcvBuf + i));

		(*(swappedBuffer + i)) = SWAPL(*(rcvBuf + i));

		MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
		"swapMessage: swappedBuffer (transmitted to the Mcms) = 0x%X", *(swappedBuffer + i));

	}

	memcpy(rcvBuf, swappedBuffer, bufSize);

	free(swappedBuffer);

//	return(swappedBuffer);
}

