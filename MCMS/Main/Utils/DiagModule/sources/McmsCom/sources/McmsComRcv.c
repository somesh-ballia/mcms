/*
*****************************************************************************
*
* Copyright (C) 2005 POLYCOM NETWORKS Ltd.
* This file contains confidential information proprietary to POLYCOM NETWORKSO
*  Ltd. The use or disclosure of any information contained
* in this file without the written consent of an officer of POLYCOM NETWORKS
* Ltd is expressly forbidden.
*
*****************************************************************************

*****************************************************************************

 Module Name: McmsComRcv.c

 General Description:  Module "" contains:

      1.

 Generated By:	Yigal Mizrahi       Date: 10.5.2005

*****************************************************************************/

/***** Include Files *****/
#include <string.h>
#include <pthread.h>	//mutexes for ipc

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



#include "LinuxSystemCallsApi.h"
#include "SocketApiWrapExt.h"
#include "McmsApi.h"
#include "EmaShared.h"
#include "DiagnosticsShared.h"
#include "Diagnostics.h"
#include "McmsCom.h"
#include "Print.h"
#include "tools.h"
#include "EmaApi.h"
#include "timers.h"

extern void StringWrapper(e_TcpConn eID,UINT32 Msg);
extern INT32 TCPDoSelect(fd_set *pActiveSet,fd_set *pTmpSet);

int isMessageFromIpmiToEmaReady();
void rcvMsgFromIpmiToEma(e_TcpConn * eID, unsigned int *msgSize,char *msgRcvdBuf);


/***** Public Variables *****/
#define	FIRST_BOARD_SLOT_ID		67
#define		IP_TEST_STATUS_FILE			"/tmp/iptestStat"
#define		BOARD_IP_PREFIX				"169.254.128."
//this is in mbits - minimum 900mbits

int			isLanTestInProgress = 0;

unicastTestStatus	isUnicastTestInProgress = nothingIsRunning;
int					boardsRunningUnicastTest = 0;
pthread_mutex_t 	unicastTestMutex = PTHREAD_MUTEX_INITIALIZER;

/***** Global Variables *****/

/***** Public Functions Declarations *****/

/***** Private Functions Declarations *****/

//extern INT32 nMcmsSocket;
extern TStructToStr atTestListIndSpecArray;
extern char externalIps[MAX_IP_NUMBER][MAX_IP_ADDRESS_SIZE];

/***** Functions Code *****/

fd_set tTcpActiveReadSockets;
fd_set *ptTcpActiveReadSockets = &tTcpActiveReadSockets;

fd_set tTcpActiveSelectSockets;
fd_set *ptTcpActiveSelectSockets = &tTcpActiveSelectSockets;

boardConnectInfo brdConnList[AMOS_BOARD_COUNT];	//connection to baraks

/****************************************************************************
* Prototype:        TCPrcvThread
* Description:
* Return Value:     no return.
* Arguments:
* Global Variable Used:
* Global Variables Changed:
* Cautions:
*****************************************************************************/
void TCPrcvThread()
{
	UINT8 *uc_RcvBuf,*tmp_uc_RcvBuf;
	UINT32 ul_BytesReceived,ul_msgSize;
	UINT32 ul_ErrorNum;
	UINT32 ulOpcode,ulOpcode_tmp;
	INT32 lSelectOpRc;
	e_TcpConn eId;
	//COMMON_HEADER_S *ptGeneralMcmsCommonHeader;
	TMessageThreadType	tMessageThreadType;
	TEmaReqHeader*		ptEmaReqHeader;
	EnrollInThreadList(eTCPrcvThread);

    MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TCPrcvThread");

    uc_RcvBuf = NULL;
    tmp_uc_RcvBuf = NULL;
        
	while(g_isServiceRun)
	{
        //----------------------------------------------------------------------------------
        // receive response messages from SwitchDiag, using Pavel's queues
        // req:  EMA -> Switch -> SwitchDiag
        // resp: EMA <- Switch [<-] SwitchDiag
        //----------------------------------------------------------------------------------
		if (isMessageFromDiagToEmaReady())	//pavelk fix
		{
			uc_RcvBuf = malloc(MAX_MSG_EMA_SIZE);
			if (uc_RcvBuf == NULL){
				printf("malloc error uc_RcvBuf =null");
			}else{
				rcvMsgFromDiagToEma(&ul_msgSize,uc_RcvBuf);			
				StringWrapper( eSwitchDiagServer, (UINT32)uc_RcvBuf );//	PAVELK
			}
		}
        else
        {
            uc_RcvBuf = NULL;
        }

        //----------------------------------------------------------------------------------
        // receive response messages from IpmiThread, using Pavel's queues
        // req:  EMA -> Switch -> IpmiThread
        // resp: EMA <- Switch [<-] IpmiThread
        //----------------------------------------------------------------------------------
/*		if (isMessageFromIpmiToEmaReady())	//pavelk fix
		{
			uc_RcvBuf = malloc(MAX_MSG_EMA_SIZE);
			if (uc_RcvBuf == NULL){
				printf("malloc error uc_RcvBuf =null");
			}else{
				e_TcpConn eID;
				rcvMsgFromIpmiToEma(&eID, &ul_msgSize,uc_RcvBuf);			
				StringWrapper( eID, uc_RcvBuf );//	PAVELK
			}
		}
        else
        {
            uc_RcvBuf = NULL;
        }*/


        //----------------------------------------------------------------------------------
        // receive sockets
        //----------------------------------------------------------------------------------
        lSelectOpRc = TCPDoSelect(ptTcpActiveReadSockets,ptTcpActiveSelectSockets);   
		if(lSelectOpRc > 0)
		{
			MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"TCPrcvThread: lSelectOpRc:%d.",lSelectOpRc);

			for(eId = eMcmsCom ; eId < eMaxTcpConnections ; eId++)
			{
				t_TcpConnParams* ptTcpCon = &TcpConnection[eId];
				SOCKET ulSocket = TcpConnection[eId].s;

				if( ulSocket <= 0 || ulSocket >= FD_SETSIZE )
				{
/*					MfaBoardPrint(	MCMS_COM_PRINT,
									PRINT_LEVEL_ERROR,
									PRINT_TO_TERMINAL,
									"TCPrcvThread: Warning:: Invalid Tcp Socket %d for eId %d !!!\n",ulSocket,eId);
*/
					continue;
				}

		/*		MfaBoardPrint(	MCMS_COM_PRINT,
								PRINT_LEVEL_MINOR,
								PRINT_TO_TERMINAL,
								"TCPrcvThread: Tcp Socket %d for eId %d.\n",ulSocket,eId);*/


				if(FD_ISSET(ulSocket,ptTcpActiveSelectSockets))
				{
					uc_RcvBuf = NULL;
					ul_BytesReceived = TCPRecvData( ulSocket,(VOID **)&uc_RcvBuf,0/*MSG_DONTWAIT*/);

					if (ul_BytesReceived != 0)
						MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TCPrcvThread: Socket %d for eId:%d is Active.",ulSocket,eId);
				   	if (ul_BytesReceived == (UINT32)SOCKET_OPERATION_FAILED)
				   	{
						FD_CLR(ulSocket,ptTcpActiveReadSockets); // Benson
				        MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TCPrcvThread: Problem Receiving Data !!! (eId:%d).",eId);
//						ptTcpCon->s			= 0xFFFF;
						ptTcpCon->ul_ConnectionStatus = NOT_CONNECTED;
						ptTcpCon->ul_PrevConnState	= CONNECTED;
						if(uc_RcvBuf != NULL)
						{
							free(uc_RcvBuf);
						}
					}
					else if (ul_BytesReceived == NO_DATA)
					{
						FD_CLR(ulSocket,ptTcpActiveReadSockets); // Nir
						ptTcpCon->ul_ConnectionStatus = NOT_CONNECTED;
						ptTcpCon->ul_PrevConnState	= CONNECTED;
						if(uc_RcvBuf != NULL)
						{
							free(uc_RcvBuf);
						}
					}
					else
					{
				         if (ul_BytesReceived > 0)
				         {
							MfaBoardPrint(	MCMS_COM_PRINT,
											PRINT_LEVEL_MAJOR,
											PRINT_TO_TERMINAL,
											"McmsComRcv: eId:%d received.",eId);

							switch(eId)
							{
								case eLoggerCom:
								{
									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_MAJOR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: eLoggerCom Rcv Msg.");

									free((void*)uc_RcvBuf);
								}
								break;

								case eLanStatClient:// Rcv Request from EmaApiServer
								{
									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_MAJOR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: BASE2:: eLanStatClient Rcv Msg.");

									//LanStatHandleReq( (UINT32)uc_RcvBuf );
									free((void*)uc_RcvBuf);
								}
								break;

								case eLanStatServer: // Rcv Indication from eLanStatClient
								{
									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_MAJOR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: BASE3:: eLanStatServer Rcv Msg.");

									StringWrapper( eLanStatServer, (UINT32)uc_RcvBuf );

								}
								break;
								
								case eIpmiClient:// Rcv Request from IpmiHandleThread
								{
									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_MAJOR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: BASE2:: eLanStatClient Rcv Msg.");

									//LanStatHandleReq( (UINT32)uc_RcvBuf );
									free((void*)uc_RcvBuf);
								}
								break;
								
								case eIpmiServer: // Rcv Indication from eLanStatClient
								{
									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_MAJOR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: BASE3:: eIpmiServer Rcv Msg.");


									StringWrapper( eIpmiServer, (UINT32)uc_RcvBuf );

								}
								break;

								case eEmaApiClient: // Rcv String Indication from EmaApiServer
								{
									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_ERROR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: BASE4:: eEmaApiClient Rcv Msg (%s).",uc_RcvBuf);
									//CHECK1
									printf("pavel eEmaApiClient before free\n");
									free((void*)uc_RcvBuf);
									printf("pavel eEmaApiClient after free\n");
								}
								break;

								case eEmaApiServer: // Rcv String Request from Ema Apach Module
								{
									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_ERROR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: BASE1:: eEmaApiServer Rcv Msg.");
//									printf("eEmaApiServer, size %d ,before StringStripper",ul_BytesReceived);
									StringStripper( uc_RcvBuf ,ul_BytesReceived);
//									printf("eEmaApiServer, size %d ,after StringStripper",ul_BytesReceived);
								}
								break;

								case eEmaSimServer: // Rcv String Messages from Ema Simulation Module
								{
									UINT8 ucDestId, ucBoardNum, ucMsgSent=0;
									UINT8 *pReqBuff;
									UINT32 ulSizeof_Headers, ul_BytesToSend = 0;


									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_ERROR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: eEmaSimServer Rcv Msg");

									StringStripper( uc_RcvBuf, ul_BytesReceived );
								}
								break;

								case eMfa1DiagServer: // Rcv String Messages from Mfa1Diag Module
								case eMfa2DiagServer: // Rcv String Messages from Mfa2Diag Module
								case eMfa3DiagServer: // Rcv String Messages from Mfa3Diag Module
								case eMfa4DiagServer: // Rcv String Messages from Mfa4Diag Module
								{
									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_ERROR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: eMfaDiagServer Rcv Msg.");

									/// get opcode of the message
									//tmp_uc_RcvBuf = uc_RcvBuf;
									//ulOpcode_tmp = StringWrapper_tmp( eId, tmp_uc_RcvBuf/*uc_RcvBuf*/ );
									//printf("yosi - opcode after wrapper = %x ",ulOpcode_tmp);

									//if(ulOpcode_tmp == EMA_GET_TEST_STATUS_REQ)
									//{
									//	printf("ignore recieving data from barak req 772 - rcv string Message");
									//	free(uc_RcvBuf);
									//}
									//else

										StringWrapper( eId, (UINT32)uc_RcvBuf );


								}
								break;

								case eMfa1ResetServer: // Rcv String Messages from Mfa1Reset Module
								case eMfa2ResetServer: // Rcv String Messages from Mfa2Reset Module
								case eMfa3ResetServer: // Rcv String Messages from Mfa3Reset Module
								case eMfa4ResetServer: // Rcv String Messages from Mfa4Reset Module
								{
									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_ERROR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: eMfaResetServer Rcv Msg.\n");

									/// get opcode of the message
									//tmp_uc_RcvBuf = uc_RcvBuf;
									//ulOpcode_tmp = StringWrapper_tmp( eId, tmp_uc_RcvBuf );
									//printf("yosi reset- opcode after wrapper = %x ",ulOpcode_tmp);
									//if(ulOpcode_tmp == EMA_GET_TEST_STATUS_REQ)
									//{
									//	printf("ignore recieving data from barak req 772 - reset server");
									//	free(uc_RcvBuf);
									//}
									//else

										StringWrapper( eId, (UINT32)uc_RcvBuf );


								}
								break;
/*
								case eSwitchDiagServer: // Rcv Indication from eSwitchDiagServer
								{


									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_MAJOR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: eSwitchDiagServer Rcv Msg.\n");

									if (uc_RcvBuf != (UINT8*)NULL)
										free((void*)uc_RcvBuf);
								//	StringWrapper( eSwitchDiagServer, uc_RcvBuf );

								}
								break;
*/
								case eCpuDiagServer: // Rcv Indication from eCpuDiagServer
								{
									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_ERROR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: eCpuDiagServer Rcv Msg.\n");

//									for (i = 0; i < (ul_BytesReceived / 4); i++)
//									{
//										MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
//										"TCPrcvThread: uc_RcvBuf = 0x%X", *((UINT32*)uc_RcvBuf + i));
//									}
									StringWrapper( eCpuDiagServer, (UINT32)uc_RcvBuf );
									//free((void*)uc_RcvBuf);	//ignore the cpu PAVELK

								}
								break;

								default:
								{
									MfaBoardPrint(	MCMS_COM_PRINT,
													PRINT_LEVEL_ERROR,
													PRINT_TO_TERMINAL,
													"TCPrcvThread: Unknown eId:%d received.",eId);
                                    
                                    printf("-- %s:%d -- ASSERT ----> unknown eId:%d received", 
                                           __FILE__, __LINE__, eId);

									free((void*)uc_RcvBuf);
								}
								break;


							}//switch

						}//if (ul_BytesReceived > 0)
						else
						{
							 if(uc_RcvBuf != NULL)
							 {
								 free(uc_RcvBuf);
							 }
							 if (ul_BytesReceived != 0 )
                             	MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TCPrcvThread: ul_BytesReceived < or = 0 %d",ul_BytesReceived);
						}
				     }//else
				}//if FD_ISSET
			}//for
			EmbSleep(1);
		}//if select

        EmbSleep(10);
        
	}//while(1)
}



INT32 TCPDoSelect(fd_set *pActiveSet,fd_set *pTmpSet)
{
	INT32 lSelectOpRc ;
	struct timeval tTimer_vals ;
	struct timeval *pTimer_vals = NULL;
	UINT32 Tmp = 0;
	int ret;
	UINT32 size = sizeof(UINT32);
	e_TcpConn eId;

	memcpy(pTmpSet,pActiveSet,sizeof(fd_set)) ;

	tTimer_vals.tv_sec = 1;
	tTimer_vals.tv_usec = 0 ;

	pTimer_vals = &tTimer_vals ;

	lSelectOpRc = select(FD_SETSIZE,pTmpSet,NULL,NULL,pTimer_vals) ;
	if(lSelectOpRc < 0)
	{
		MfaBoardPrint(	MCMS_COM_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"(TCPDoSelect): Select Error %d !!!",GetLastError());

		if(GetLastError() == EBADF || GetLastError() == EHOSTUNREACH || GetLastError() == ETIMEDOUT)
		{
			for(eId = eMcmsCom ; eId < eMaxTcpConnections ; eId++)
			{
				t_TcpConnParams* ptTcpCon = &TcpConnection[eId];
				SOCKET ulSocket = TcpConnection[eId].s;

				if( ulSocket <= 0 || ulSocket >= FD_SETSIZE )
				{
					continue;
				}
				if (FD_ISSET(ulSocket, pActiveSet))
				{
					ret =  getsockopt(ulSocket,SOL_SOCKET,SO_RCVBUF,(UINT8 *)&Tmp,&size);
					if(ret < 0)
					{
						/** if there is an  error */
						if (GetLastError() == EBADF || GetLastError() == EHOSTUNREACH || GetLastError() == ETIMEDOUT)
						{
							MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TCPDoSelect): Got ERROR on socket%d !!!\n",socket) ;
							FD_CLR(ulSocket,pActiveSet);
//							ptTcpCon->s			= 0xFFFF;
							ptTcpCon->ul_ConnectionStatus = NOT_CONNECTED;
							ptTcpCon->ul_PrevConnState	= CONNECTED;
						}
					}
				}
			}
		}
	}

/*
	else if(lSelectOpRc == 0)
	{
		MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"SelectOnClients Timed Out %d",GetLastError());
		return lSelectOpRc ;
	}
*/

	return lSelectOpRc ;
}


#define		LAN_INTERFACE_LOWEST_SPEED	50
#define		LAN_INTERFACE_CPU_LOWEST_SPEED	50

#define	MFA_BOARD_1_IP	"169.254.128.67"
#define	MFA_BOARD_2_IP	"169.254.128.68"
#define	MFA_BOARD_3_IP	"169.254.128.69"
#define	MFA_BOARD_4_IP	"169.254.128.70"

#define	CNTL_BOARD_INTERNAL_IP	"169.254.128.10"

////////////////////////////////

int isMessageReady(int Top, int Bottom)
{
	if (Top != Bottom)
			return 1;
		else
			return 0;
}

void sendMsg(unsigned int msgSize, char *msgSendBuf, pthread_mutex_t * sendMutex, EmaMsg * queue, int * top, int * bottom, e_TcpConn * eID)
{
	pthread_mutex_lock(sendMutex);
    UINT32 minSize = msgSize;
    if(msgSize > MAX_MSG_EMA_SIZE)
    {
        minSize = MAX_MSG_EMA_SIZE;
        printf("-- %s:%d -- ASSERT ----> message SIZE is too big: recved %d, max %d", __FILE__, __LINE__, msgSize, MAX_MSG_EMA_SIZE);
    }
    
	//copy the message to queue
	queue[*top].messageSize = minSize;
	if(NULL != eID) queue[*top].eID = *eID;
	memcpy(&(queue[*top].Message[0]),msgSendBuf,minSize);

	(*top) += 1;
	if (*top == MAX_MSG_EMA_COUNT)
	{
		*top = 0;
	}

	/*	If top = bottom after receiving new message, it means we had wraparound. Advance the bottom , to only
	 * loose 1 message	*/
	if (*top == *bottom)
	{
		(*bottom)++;
		if (*bottom == MAX_MSG_EMA_COUNT)
			*bottom = 0;
	}
	pthread_mutex_unlock(sendMutex);
}

void rcvMsg(unsigned int *msgSize,char *msgRcvdBuf, pthread_mutex_t * recMutex, EmaMsg * queue, int * bottom, e_TcpConn * eID)
{
	pthread_mutex_lock(recMutex);

/*	get the message from "bottom"	*/
	*msgSize = queue[*bottom].messageSize;
	if(NULL != eID)  *eID = queue[*bottom].eID;

    UINT32 minSize = *msgSize;
    if(*msgSize > MAX_MSG_EMA_SIZE)
    {
        minSize = MAX_MSG_EMA_SIZE;
        printf("-- %s:%d -- ASSERT ----> message SIZE is too big: recved %d, max %d", __FILE__, __LINE__, *msgSize, MAX_MSG_EMA_SIZE);
    }    
    
	memcpy(msgRcvdBuf,&(queue[*bottom].Message[0]), minSize);
    
	/*	and advance the bottom	*/
	(*bottom)++;
	if (*bottom == MAX_MSG_EMA_COUNT)
		*bottom = 0;
	
	pthread_mutex_unlock(recMutex);
}


pthread_mutex_t fromEmaToDiagMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fromDiagToEmaMutex = PTHREAD_MUTEX_INITIALIZER;

int	msgsFromEmaToDiagTop = 0;
int msgsFromEmaToDiagBottom = 0;

EmaMsg	fromEmaToDiag[MAX_MSG_EMA_COUNT];

int	msgsFromDiagToEmaTop = 0;
int msgsFromDiagToEmaBottom = 0;

EmaMsg	fromDiagToEma[MAX_MSG_EMA_COUNT];

int isMessageFromDiagToEmaReady()
{
	return isMessageReady(msgsFromDiagToEmaTop, msgsFromDiagToEmaBottom);
}

void sendMsgFromDiagToEma(unsigned int msgSize, char *msgSendBuf)
{

	sendMsg(msgSize, msgSendBuf, &fromDiagToEmaMutex, fromDiagToEma, &msgsFromDiagToEmaTop, &msgsFromDiagToEmaBottom, NULL);
}

void rcvMsgFromDiagToEma(unsigned int *msgSize,char *msgRcvdBuf)
{
      rcvMsg(msgSize, msgRcvdBuf, &fromDiagToEmaMutex, fromDiagToEma, &msgsFromDiagToEmaBottom, NULL);
}


int	isMessageFromEmaToDiagReady()
{    
	return isMessageReady(msgsFromEmaToDiagTop, msgsFromEmaToDiagBottom);
}


void sendMsgFromEmaToDiag(unsigned int msgSize,char	*msgSendBuf)
{
	sendMsg(msgSize, msgSendBuf, &fromEmaToDiagMutex, fromEmaToDiag, &msgsFromEmaToDiagTop, &msgsFromEmaToDiagBottom, NULL);
}

void rcvMsgFromEmaToDiag(unsigned int *msgSize,char *msgRcvdBuf)
{
	rcvMsg(msgSize, msgRcvdBuf, &fromEmaToDiagMutex, fromEmaToDiag, &msgsFromEmaToDiagBottom, NULL);
}


pthread_mutex_t fromEmaToIpmiMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fromIpmiToEmaMutex = PTHREAD_MUTEX_INITIALIZER;

int	msgsFromEmaToIpmiTop = 0;
int msgsFromEmaToIpmiBottom = 0;

EmaMsg	fromEmaToIpmi[MAX_MSG_EMA_COUNT];

int	msgsFromIpmiToEmaTop = 0;
int	msgsFromIpmiToEmaBottom = 0;

EmaMsg	fromIpmiToEma[MAX_MSG_EMA_COUNT];

int isMessageFromIpmiToEmaReady()
{
	return isMessageReady(msgsFromIpmiToEmaTop, msgsFromIpmiToEmaBottom);
}

void sendMsgFromIpmiToEma(e_TcpConn eID, unsigned int msgSize, char *msgSendBuf)
{

	sendMsg(msgSize, msgSendBuf, &fromIpmiToEmaMutex, fromIpmiToEma, &msgsFromIpmiToEmaTop, &msgsFromIpmiToEmaBottom, &eID);
}

void rcvMsgFromIpmiToEma(e_TcpConn * eID, unsigned int *msgSize,char *msgRcvdBuf)
{
      rcvMsg(msgSize, msgRcvdBuf, &fromIpmiToEmaMutex, fromIpmiToEma, &msgsFromIpmiToEmaBottom, eID);
}


int	isMessageFromEmaToIpmiReady()
{    
	return isMessageReady(msgsFromEmaToIpmiTop, msgsFromEmaToIpmiBottom);
}


void sendMsgFromEmaToIpmi(e_TcpConn eID, unsigned int msgSize,char	*msgSendBuf)
{
	sendMsg(msgSize, msgSendBuf, &fromEmaToIpmiMutex, fromEmaToIpmi, &msgsFromEmaToIpmiTop, &msgsFromEmaToIpmiBottom, &eID);
}

void rcvMsgFromEmaToIpmi(e_TcpConn * eID, unsigned int *msgSize,char *msgRcvdBuf)
{
	rcvMsg(msgSize, msgRcvdBuf, &fromEmaToIpmiMutex, fromEmaToIpmi, &msgsFromEmaToIpmiBottom, eID);
}

