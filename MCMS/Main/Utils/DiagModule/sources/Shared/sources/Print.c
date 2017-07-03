/*=====================================================================================================*/
/*            Copyright     ???    2005  Polycom Israel, Ltd. All rights reserved                      */
/*-----------------------------------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary   information of                */
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form      */
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without            */
/* prior written authorization from Polycom Israel Ltd.                                                */
/*-----------------------------------------------------------------------------------------------------*/
/* FILE:            Print.c                                                                            */
/* PROJECT:         GIDEON                                                                             */
/* PROGRAMMER:      Yigal Mizrahi                                              .                       */
/* FUNCTION LIST:                                                                                      */
/*-----------------------------------------------------------------------------------------------------*/
/* Who             |      Date           |         Description                                         */
/*-----------------------------------------------------------------------------------------------------*/
/* Yigal Mizrahi   |      10/7/05		 | print to logger or to terminal                              */
/*=====================================================================================================*/


#include <string.h>
#include <sys/time.h>

#include "SharedDefines.h"
#include "McmsApi.h"
#include "DbgCfg.h"
#include "OutsideEntities.h"
#include "stdarg.h"
#include "SocketApiWrapExt.h"  
#include "Print.h"
#include "timers.h"

extern void OpenSwitchLogFileInTmp();

extern MFA_DBG_S	tMfaDbgInfo;
extern UINT32	IAmChildProcess;
extern UINT32 switch_diag_mode;

UINT32 unGlobalPrintSource = (CM_INITIATOR_PRINT | DIAG_PRINT | DISPATCH_COM_PRINT | E2PROM_PRINT | IPMI_PRINT | LAN_SWITCH_PRINT | MCMS_COM_PRINT | NTP_PRINT |  SHARED_PRINT | STARTUP_PRINT | TCP_SERVER_PRINT | USB_PRINT | TIMER_PRINT | SHELFCOM_PRINT); 

      

UINT32 unCmInitiatorPrintLevel;
UINT32 unDiagPrintLevel;
UINT32 unDispatchPrintLevel;
UINT32 unE2promPrintLevel;
UINT32 unIpmiPrintLevel;
UINT32 unShelfComPrintLevel;
UINT32 unLanSwitchPrintLevel;
UINT32 unMcmsComPrintLevel;
UINT32 unNtpPrintLevel;
UINT32 unSharedPrintLevel;
UINT32 unStartupPrintLevel;
UINT32 unTcpServerPrintLevel;
UINT32 unUsbPrintLevel;
UINT32 unTimerPrintLevel;

UINT32 unByteCounterForLogFile = 0;


UINT32 unLoggerMsgCnt	= 0;

UINT32 unLoggerConnectionStatus = NOT_CONNECTED;

void FillLoggerMsgHeaders(UINT32 id , UINT8* content, UINT32 size , UINT32 unTaskName); 
void SendTrace(UINT8 id, UINT32 meEntity , UINT8* content); 
void FillCommonHeader(COMMON_HEADER_S *pCommonHeader);
void FillPhysicalHeader(PHYSICAL_INFO_HEADER_S *pPhysicalInfoHeader);
void FillTraceHeader(TRACE_HEADER_S *pTraceHeader, UINT32 id, UINT32 ulBufferSize , UINT32 unTaskName);
void SetSystemTime(INT8* acBuffer, INT8 *pcFormat, ...);
APIU32 SystemGetTickCount();
INT8 *GetTaskName(UINT32 unTaskName);

 
extern INT32 l_PrintQueue; 
extern pthread_t PrintThreadId;

extern APIUBOOL diagFlag;

#define GET_MFA_LOGGER_PORT_NUMBER              10009 
/*
#define GET_LOGGER_V4_LOCAL_IP_ADDRESS(X) 	    GET_CONTROL_V4_IP_ADDRESS(X)
#define GET_LOGGER_V4_LOCAL_SUBNET_MASK(X)	    GET_CONTROL_V4_DEFAULT_GATEWAY(X)
#define GET_LOGGER_V4_LOCAL_DEFAULT_GATEWAY(X)  GET_CONTROL_V4_SUBNET_MASK(X)
*/
#define MYDEF_TRACE_NORMAL	50	//l.a. to be defined by mcms in TraceHeader.h
#define MAX_LOG_FILE_SIZE   0x300000
      
      
extern void BackUpLogFile();
      
/*====================================================================*/
/* FUNCTION:   MfaBoardPrint()                                        */
/*--------------------------------------------------------------------*/
/* PURPOSE : Register Flow of functions								  */
/* PARAMETERS:  String to print                                       */
/* RETURN VALUE:  void                                                */
/* LIMITATION:                                                        */
/*====================================================================*/
void MfaBoardPrint(UINT32 unPrintSource , UINT32 unPrintLevel , UINT32 PrintFlag , INT8 *pcFormat, ...)
{
    INT8  string[30000], cTimeString[1000] , *pucPrintString;
    UINT32 unLoggerHeaderSize, unContentLen;
	TMessageThreadType tMessageThreadType;
    UINT32 unPrintTheMessage = 0;
    time_t timep;
    struct tm result;
	struct timeval tTimer_vals;	
    va_list tVarList;
    memset(&tMessageThreadType,0,sizeof(TMessageThreadType));
	
	switch (unPrintSource)
	{
		case CM_INITIATOR_PRINT:
		{
			if ( (unGlobalPrintSource & CM_INITIATOR_PRINT) == CM_INITIATOR_PRINT)
		    {
		    	if (unPrintLevel <= unCmInitiatorPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
		}
    
    	case DIAG_PRINT:
    	{
		    if ( (unGlobalPrintSource & DIAG_PRINT) == DIAG_PRINT)
		    {
		    	if (unPrintLevel <= unDiagPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
    	}
    
    	case DISPATCH_COM_PRINT:
    	{
		    if ( (unGlobalPrintSource & DISPATCH_COM_PRINT) == DISPATCH_COM_PRINT)
		    {
		    	if (unPrintLevel <= unDispatchPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
    	}
    
   	 	case E2PROM_PRINT:
   	 	{
		    if ( (unGlobalPrintSource & E2PROM_PRINT) == E2PROM_PRINT)
		    {
		    	if (unPrintLevel <= unE2promPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
   	 	}
    
    	case IPMI_PRINT:
    	{
		    if ( (unGlobalPrintSource & IPMI_PRINT) == IPMI_PRINT)
		    {
		    	if (unPrintLevel <= unIpmiPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
    	}
    
    	case SHELFCOM_PRINT:
    	{
		    if ( (unGlobalPrintSource & SHELFCOM_PRINT) == SHELFCOM_PRINT)
		    {
		    	if (unPrintLevel <= unShelfComPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
    	}

    	case LAN_SWITCH_PRINT:
    	{
		    if ( (unGlobalPrintSource & LAN_SWITCH_PRINT) == LAN_SWITCH_PRINT)
		    {
		    	if (unPrintLevel <= unLanSwitchPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
    	}
    
    	case MCMS_COM_PRINT:
    	{
		    if ( (unGlobalPrintSource & MCMS_COM_PRINT) == MCMS_COM_PRINT)
		    {
		    	if (unPrintLevel <= unMcmsComPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
    	}
    
    	case NTP_PRINT:
    	{
		    if ( (unGlobalPrintSource & NTP_PRINT) == NTP_PRINT)
		    {
		    	if (unPrintLevel <= unNtpPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
    	}
    
    	case SHARED_PRINT:
    	{
		    if ( (unGlobalPrintSource & SHARED_PRINT) == SHARED_PRINT)
		    {
		    	if (unPrintLevel <= unSharedPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
    	}
    
    	case STARTUP_PRINT:
    	{
		    if ( (unGlobalPrintSource & STARTUP_PRINT) == STARTUP_PRINT)
		    {
		    	if (unPrintLevel <= unStartupPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
    	}
    
    	case TCP_SERVER_PRINT:
    	{
		    if ( (unGlobalPrintSource & TCP_SERVER_PRINT) == TCP_SERVER_PRINT)
		    {
		    	if (unPrintLevel <= unTcpServerPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    }
		    break;
    	}
	    
    	case USB_PRINT:
    	{
		    if ( (unGlobalPrintSource & USB_PRINT) == USB_PRINT)
		    {
		    	if (unPrintLevel <= unUsbPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    	
		    }
		    break;
    	}
	    
    	case TIMER_PRINT:
    	{
		    if ( (unGlobalPrintSource & TIMER_PRINT) == TIMER_PRINT)
		    {
		    	if (unPrintLevel <= unTimerPrintLevel)
		        {
		            unPrintTheMessage = 1;
		        }
		    	
		    }
		    break;
    	}	    
	}  
	 
    if (unPrintTheMessage == 1)
	{
		gettimeofday(&tTimer_vals,NULL) ;
		timep = time((time_t*)NULL);
		gmtime_r(&timep, &result);
		
		va_start(tVarList,pcFormat);
		
		if(IAmChildProcess)
		{
			SetSystemTime(&cTimeString[0], "\nSwitch_DIAG_CM_Print D: %d/%d/%d  T: %d:%d:%d:%d : ",result.tm_mday,(result.tm_mon + 1),(result.tm_year + 1900),result.tm_hour,result.tm_min,result.tm_sec,tTimer_vals.tv_usec);
		}
		else
		{
			SetSystemTime(&cTimeString[0], "\nSwitch_CM_Print D: %d/%d/%d  T: %d:%d:%d:%d : ",result.tm_mday,(result.tm_mon + 1),(result.tm_year + 1900),result.tm_hour,result.tm_min,result.tm_sec,tTimer_vals.tv_usec);
		}
        vsnprintf(string, sizeof(string), pcFormat, tVarList);
		unContentLen = strlen(cTimeString) + strlen(string) + 1;
		
		//malloc a pointer and send the time and string to print task
		//in the task we will fill the logger common header or send to file or terminal
		pucPrintString = (UINT8*)malloc(unContentLen);
	    if (pucPrintString == 0)
	    {
	 		printf("(MfaBoardPrint)TCP Logger Malloc Failure.\n");
		    va_end(tVarList);
	 		return;   		
	    }
	    
	    //fill the time stamp
	    memcpy( (void*)(pucPrintString) , (void*)&cTimeString[0] , strlen(cTimeString) );
		    
	    memcpy( (void*)(pucPrintString + strlen(cTimeString)) , (void*)&string[0] , (strlen(string) +1) );
	    pucPrintString[unContentLen - 1] = '\0';
		// Bracha - for debug don't sent diag messages to print task
		if(IAmChildProcess)
		{
			if (tMfaDbgInfo.DiagTerminalFileHandle)
			{
				fprintf(tMfaDbgInfo.DiagTerminalFileHandle,"%s",pucPrintString) ;
			}
	    	free(pucPrintString);	
		}
		else
		{
		    if (PrintThreadId != 0)
		    {
			    //send to print task 
			    tMessageThreadType.ulData   = (UINT32)pucPrintString;
			    tMessageThreadType.ulSize   = unContentLen;
			    tMessageThreadType.ulDummy1 = INTERNAL_LOGGER;
			    tMessageThreadType.ulStreamInfo = unPrintSource;
			    
			    if (SendMessage(l_PrintQueue ,&tMessageThreadType, IPC_NOWAIT) == -1)
		   	 	{
		   	   		printf("MfaBoardPrint: problem sending message to print thread");
		   	   		free(pucPrintString);
		   	 	}
		    }
		    else   
		    {
		    	/*
		    	if (tMfaDbgInfo.logFileHandle == NULL)
				{
					//open log file for switch
					OpenSwitchLogFileInTmp();	
				}
				
				if (tMfaDbgInfo.logFileHandle)
				{
					fprintf(tMfaDbgInfo.logFileHandle,"%s",(INT8*)pucPrintString);
					unByteCounterForLogFile += strlen(pucPrintString);
					if (unByteCounterForLogFile > MAX_LOG_FILE_SIZE)
					{
						BackUpLogFile();
						unByteCounterForLogFile = 0;
					}
				}
				else
				{
					printf("\n%s Problem With Log File\n",pucPrintString);
				}
				*/
				
				printf("%s",pucPrintString);
				
		    	free(pucPrintString);	
		    }
	    
		}
   	 	va_end(tVarList);
	}
}
/*
void MfaPrint(const char *file_name, const char *func, int line, INT8 *pcFormat, ...)
{
    va_list tVarList;
    char buffer[1024] = {0};

    snprintf(buffer, sizeof(buffer), "%s:%d : ", file_name, line);
    int len = strlen(buffer);
    
    va_start(tVarList, pcFormat);
   
    int rc = vsnprintf(buffer + len, sizeof(buffer) - len, pcFormat, tVarList);
    if(0 > rc)
    {
        perror("Failed to vsnprintf");
    }
    
    printf(buffer);
    printf("\n");
    
    va_end(tVarList);
}
*/


void PrintThread()
{
	TMessageThreadType tMessageThreadType;
    INT32 l_RetVal = 1 , l_Rc;
    INT32 unPort;
    t_IpV4Address t_V4IpAddr;
	UINT32 ul_Counter = 0;
	UINT32 unLoggerHeaderSize , unContentLen;
	time_t timep;
    struct tm result;
	struct timeval tTimer_vals;	 
	UINT8 *pucLoggerString;	
	UINT32 unByteCounterForLogFile = 0;
    memset(&tMessageThreadType,0,sizeof(TMessageThreadType));	
	static UINT32 isSendBeforeOk = YES;
	static UINT32 isRcvBeforeOk = YES;

	UINT32 i;
	UINT8* tmpBuf;
	
	EnrollInThreadList(ePrintThread);
	
	//TEST InitLoggerTcpConn();
	
	    
	while(l_RetVal > 0)
	{
		l_RetVal = ReceiveMessage(l_PrintQueue , &tMessageThreadType, IPC_NOWAIT);
		printf("l_PrintQueue: l_RetVal > 0. Clean print queue\n");
	}

	//printThread main loop. send prints to mcms logger:    	  
    while (1)
    {	
		l_RetVal = ReceiveMessage(l_PrintQueue, &tMessageThreadType, 0); 
 		if (l_RetVal == -1)
 		{
 	   		if (isRcvBeforeOk == YES)
 	   		{
 	   			gettimeofday(&tTimer_vals,NULL);
 	   			timep = time((time_t*)NULL);
				gmtime_r(&timep, &result);
	        	printf("D: %d/%d/%d  T: %d:%d:%d:%d  (PrintThread): problem in receiving message \n\n ",
	        			result.tm_mday,
	        			(result.tm_mon + 1),
	        			(result.tm_year + 1900),
	        			result.tm_hour,
	        			result.tm_min,
	        			result.tm_sec,
	        			(int)(tTimer_vals.tv_usec));
	        	isRcvBeforeOk = NO;
 	   		}
 		}		
 		else
 		{
 	  		if (l_RetVal > 0) 
 	  		{
//				if(diagFlag)
//				{
//					printf("%s",(INT8*)tMessageThreadType.ulData);
//				}


 	  			isRcvBeforeOk = YES;
 	  			
 	  			switch (tMfaDbgInfo.ul_OutputDest)
  				{
  					case eLOG_FILE:
  					{
  						
  						if (tMfaDbgInfo.logFileHandle == NULL)
						{
							//open log file for mfa
							OpenSwitchLogFileInTmp();	
						}
						
						if (tMfaDbgInfo.logFileHandle)
						{
  							fprintf(tMfaDbgInfo.logFileHandle,"%s",(INT8*)tMessageThreadType.ulData);
							unByteCounterForLogFile += tMessageThreadType.ulSize;
							if (unByteCounterForLogFile > MAX_LOG_FILE_SIZE)
							{
								BackUpLogFile();
								unByteCounterForLogFile = 0;
							}
							
							else
							{
								printf("\n%s Problem With Log File\n",(INT8*)tMessageThreadType.ulData) ;	
							}
						}
  						
  						break;
  					}
  					
  					case eTERMINAL:
  					{
  						if(switch_diag_mode==0)
						{
	  						printf("%s",(INT8*)tMessageThreadType.ulData) ;
	  						break;
						}
  					}
  					
  					case eTCP_LOGGER:
  					{
  						if(switch_diag_mode==0)
						{
	  						if (TcpConnection[eLoggerCom].ul_ConnectionStatus == CONNECTED)
	  						{
	  							pucLoggerString = (UINT8*)malloc(MAX_TRACE_SIZE);
							    if (pucLoggerString == 0)
							    {
							 		printf("(MfaBoardPrint)TCP Logger Malloc Failure.\n");
	  								printf("%s",(INT8*)tMessageThreadType.ulData);
	  								free((void *)tMessageThreadType.ulData);	
							 		return;   		
							    }
	  							memset(pucLoggerString,0,MAX_TRACE_SIZE);
	
								unLoggerHeaderSize = sizeof(COMMON_HEADER_S) + sizeof(PHYSICAL_INFO_HEADER_S) + sizeof(TRACE_HEADER_S);
								// update content length up to MAX_TRACE_SIZE length
								unContentLen = strlen((INT8*)tMessageThreadType.ulData);//% (MAX_TRACE_SIZE - unLoggerHeaderSize - 1);
								if ( (unContentLen + unLoggerHeaderSize) > MAX_TRACE_SIZE)
								{
									unContentLen = (MAX_TRACE_SIZE - unLoggerHeaderSize);
									printf("PrintThread: wrong!!! logger message content was too long!!!");	
								}
								//fill the common header
								FillLoggerMsgHeaders(0 , (UINT8*)pucLoggerString, unContentLen , tMessageThreadType.ulStreamInfo);
								
								//fill the content
							    memcpy( (void*)(pucLoggerString + unLoggerHeaderSize), (void*)(INT8*)tMessageThreadType.ulData,unContentLen); 
							    
						    if ( TCPSendData( TcpConnection[eLoggerCom].s , (void*)pucLoggerString , (unContentLen + unLoggerHeaderSize) , 0 ,BOARD_PRINT_NO) == SOCKET_OPERATION_FAILED)
				 				{
				 					//danny if there is problem on sending data we won't send it logger
				 					if (isSendBeforeOk == YES)
				 					{
					 					gettimeofday(&tTimer_vals,NULL) ;
					           			timep = time((time_t*)NULL);
										gmtime_r(&timep, &result);
										printf("D: %d/%d/%d  T: %d:%d:%d:%d  (PrintThread): problem in sending data \n\n ",
												result.tm_mday,
												(result.tm_mon + 1),
												(result.tm_year + 1900),
												result.tm_hour,
												result.tm_min,
												result.tm_sec,
												(int)(tTimer_vals.tv_usec));
										printf("%s",(INT8*)tMessageThreadType.ulData) ;
				        				isSendBeforeOk = NO;	
				 					}
				 			
				 					TcpConnection[eLoggerCom].ul_ConnectionStatus = NOT_CONNECTED;	 
				 					TcpConnection[eLoggerCom].ul_PrevConnState = CONNECTED;	
				 				}
				 				else
				 				{
				 					isSendBeforeOk = YES;
				 					unLoggerMsgCnt++;
				 				} 
	
				 				free((void *)pucLoggerString);	
	
	  						}//if (TcpConnection[eLoggerCom].ul_ConnectionStatus == CONNECTED)
	  						
	  						else
	  						{
	  							/*
	  							if (tMfaDbgInfo.logFileHandle == NULL)
								{
									//open log file for mfa
									OpenSwitchLogFileInTmp();	
								}
								
								if (tMfaDbgInfo.logFileHandle)
								{
		  							fprintf(tMfaDbgInfo.logFileHandle,"%s",(INT8*)tMessageThreadType.ulData);
									unByteCounterForLogFile += tMessageThreadType.ulSize;
									if (unByteCounterForLogFile > MAX_LOG_FILE_SIZE)
									{
										BackUpLogFile();
										unByteCounterForLogFile = 0;
									}
									
									else
									{
										printf("\n%s Problem With Log File\n",(INT8*)tMessageThreadType.ulData) ;	
									}
								}
								*/
								
								printf("\n%s Problem With Log File\n",(INT8*)tMessageThreadType.ulData) ;
	  						}
						}
  						
  						break;
  					}//case eTCP_LOGGER:
  					case eSPECIFIED_TERMINAL:
  					{
  						fprintf(tMfaDbgInfo.terminalFileHandle,"%s",(INT8*)tMessageThreadType.ulData) ;
  						break;
  					}
  				}//switch (tMfaDbgInfo.ul_OutputDest)
 	  		}//if (l_RetVal > 0) 
//			if(diagFlag && !IAmChildProcess)
//			{
//				*((INT8*)tMessageThreadType.ulData + (INT8)tMessageThreadType.ulSize - 1) = '\0';
//				printf("%s\n", tMessageThreadType.ulData);
//				printf("pointer address = %x\n", &tMessageThreadType.ulData);
//
//				EmbSleep(1);
//
//			}
//			else
//			{
				if ((void *)tMessageThreadType.ulData != NULL)
	 	  			free((void *)tMessageThreadType.ulData);
//			}
 		}//else recive msg
    }//while (1)
   
    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"PrintThread: error! exiting Dispatcher Thread");	
}


void SetSystemTime(INT8* acBuffer, INT8 *pcFormat, ...)
{
	va_list vaListTime;
			
	va_start(vaListTime,pcFormat);
	vsprintf(acBuffer,pcFormat,vaListTime);	
	
	va_end(vaListTime);
}



void FillLoggerMsgHeaders(UINT32 id, UINT8* pContent, UINT32 ulBufferSize ,  UINT32 unTaskName)
{
	UINT32 commonHeaderLen, traceHeaderLen , physicalInfoHeaderLen;

	
	commonHeaderLen       = sizeof(COMMON_HEADER_S);
	physicalInfoHeaderLen = sizeof(PHYSICAL_INFO_HEADER_S);
	traceHeaderLen        = sizeof(TRACE_HEADER_S);

	FillCommonHeader((COMMON_HEADER_S*)pContent);
	FillPhysicalHeader((PHYSICAL_INFO_HEADER_S*)(pContent + commonHeaderLen));
	FillTraceHeader((TRACE_HEADER_S*)(pContent + commonHeaderLen + physicalInfoHeaderLen), id, ulBufferSize , unTaskName);
	    	
	return;
}



void FillCommonHeader(COMMON_HEADER_S *pCommonHeader)
{
	if(	pCommonHeader != 0)
	{
		memset(pCommonHeader, 0, sizeof(COMMON_HEADER_S));
		pCommonHeader->src_id 			= eCM_Switch;
		pCommonHeader->dest_id 			= eMcms;
		pCommonHeader->next_header_type = SWAPL(eHeaderPhysical);
	}
	else
	{
		printf("(FillCommonHeader) pCommonHeader = NULL\n");	
	}
	
	return;
}


void FillPhysicalHeader(PHYSICAL_INFO_HEADER_S *pPhysicalInfoHeader)
{
	if(	pPhysicalInfoHeader != 0)
	{
		memset(pPhysicalInfoHeader, 0, sizeof(PHYSICAL_INFO_HEADER_S));
		pPhysicalInfoHeader->board_id         = 5;
		pPhysicalInfoHeader->next_header_type = SWAPL(eHeaderTrace);
	}
	else
	{
		printf("\n(FillPhysicalHeader) pPhysicalInfoHeader = NULL\n");	
	}
	
	return;
}

 

void FillTraceHeader(TRACE_HEADER_S *pTraceHeader, UINT32 id, UINT32 ulBufferSize , UINT32 unTaskName)
{
	if(	pTraceHeader != 0)
	{
		INT8 *pucTaskName = NULL;

		pucTaskName = (INT8*)GetTaskName(unTaskName);
		
		pTraceHeader->m_processMessageNumber = SWAPL(unLoggerMsgCnt);
		pTraceHeader->m_systemTick 		= SWAPL(SystemGetTickCount() );
		pTraceHeader->m_processType 	= SWAPL(eSwitchCardManager);
		pTraceHeader->m_level 			= SWAPL(MYDEF_TRACE_NORMAL);
		pTraceHeader->m_sourceId 		= SWAPL((APIU32)id);
		pTraceHeader->m_messageLen 		= SWAPL(ulBufferSize);
		//pTraceHeader->m_taskName[0] 	= '\0';
		pTraceHeader->m_objectName[0]	= '\0';
		pTraceHeader->m_topic_id 		= 0xFFFFFFFF;
		pTraceHeader->m_unit_id 		= 0;
		pTraceHeader->m_conf_id 		= 0xFFFFFFFF;
		pTraceHeader->m_party_id 		= 0xFFFFFFFF;
		pTraceHeader->m_opcode 			= 0xFFFFFFFF;
		pTraceHeader->m_str_opcode[0] 	= '\0';
		pTraceHeader->m_terminalName[0] = '\0';
		
		memset((void*)(&pTraceHeader->m_taskName[0]),0,MAX_TASK_NAME_LEN);
		if(NULL != pucTaskName)
		{
			memcpy((void*)(&pTraceHeader->m_taskName[0]),(void*)pucTaskName,strlen(pucTaskName));
			free(pucTaskName);
		}

	}
	else
	{
		printf("(FillTraceHeader) pTraceHeader = NULL\n");	
	}
	
	return;
}



INT8 *GetTaskName(UINT32 unTaskName)
{
	UINT8 *pucTaskName = NULL;
	
	pucTaskName = (UINT8*)malloc(MAX_TASK_NAME_LEN);
	if(NULL == pucTaskName) return NULL;
	memset(pucTaskName,0,MAX_TASK_NAME_LEN);
	
	switch (unTaskName)
	{
		case CM_INITIATOR_PRINT:
		{
			memcpy(pucTaskName,"CmInitiator",(strlen("CmInitiator") + 1));
		    break;
		}
	    
	    case DIAG_PRINT:
	    {
		    memcpy(pucTaskName,"Diag",(strlen("Diag") + 1));
		    break;
	    }
	    
	    case DISPATCH_COM_PRINT:
	    {
		    memcpy(pucTaskName,"Dispatch",(strlen("Dispatch") + 1));
		    break;
	    }
	    
	    case E2PROM_PRINT:
	    {
		    memcpy(pucTaskName,"EEprom",(strlen("EEprom") + 1));
		    break;
	    }
	    
	    case IPMI_PRINT:
	    {
		    memcpy(pucTaskName,"Ipmi",(strlen("Ipmi") + 1));
		    break;
	    }
	    
	    case SHELFCOM_PRINT:
	    {
		    memcpy(pucTaskName,"ShelfCom",(strlen("ShelfCom") + 1));
		    break;
	    }
	    
	    case LAN_SWITCH_PRINT:
	    {
		    memcpy(pucTaskName,"LanSwitch",(strlen("LanSwitch") + 1));
		    break;
	    }
	    
	    case MCMS_COM_PRINT:
	    {
		    memcpy(pucTaskName,"McmsCom",(strlen("McmsCom") + 1));
		    break;
	    }
	    
	    case NTP_PRINT:
	    {
		    memcpy(pucTaskName,"Ntp",(strlen("Ntp") + 1));
		    break;
	    }
	    
	    case SHARED_PRINT:
	    {
		    memcpy(pucTaskName,"Shared",(strlen("Shared") + 1));
		    break;
	    }
	    
	    case STARTUP_PRINT:
	    {
		    memcpy(pucTaskName,"StartUp",(strlen("StartUp") + 1));
		    break;
	    }
		    
		case TCP_SERVER_PRINT:
		{
		   memcpy(pucTaskName,"TcpServer",(strlen("TcpServer") + 1));
		   break;
		}
    	case USB_PRINT:
    	{
    		memcpy(pucTaskName,"Usb",(strlen("Usb") + 1));
			break;
    	}	    	    
	    
	    case TIMER_PRINT:
	    {
		    memcpy(pucTaskName,"Timer",(strlen("Timer") + 1));
		    break;
	    }	
	    default:
	    {
	    	memcpy(pucTaskName,"MfaUnKnownTask",(strlen("MfaUnKnownTask") + 1));
	    	break;
	    }
	}//switch (unTaskName)
	
	return (INT8*)pucTaskName;
}


INT32 InitLoggerTcpConn()
{
/*
	BOOL   b_Timeout = EMB_FALSE;
	UINT32 us_Port;
	UINT32 ul_Counter = 0;
	UINT32 l_Rc;
  	t_IpV4Address            t_V4IpAddr,t_V4;
  	t_TcpConnParams *LoggerConParams;
  	SOCKET l_ClientSocket;
 
    MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitLoggerTcpConn");
  	
    LoggerConParams =  &(TcpConnection[eLoggerCom]);
  	
	l_ClientSocket =  InitV4Connection(); // init client socket
		
	if (l_ClientSocket == SOCKET_OPERATION_FAILED)
		MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitLoggerTcpConn:  InitV4Connection failed");
	
	us_Port = GET_MFA_LOGGER_PORT_NUMBER;
	  
	GET_LOGGER_V4_LOCAL_IP_ADDRESS(&(LoggerConParams->IpV4Addr.auc_IpV4Address));
	GET_LOGGER_V4_LOCAL_SUBNET_MASK(&(LoggerConParams->IpV4Addr.auc_NetMask));
	GET_LOGGER_V4_LOCAL_DEFAULT_GATEWAY(&(LoggerConParams->IpV4Addr.auc_DefaultGetway));
	
	LoggerConParams->us_Port             = us_Port;  
	LoggerConParams->s                   = l_ClientSocket;
	LoggerConParams->e_Id                = eLoggerCom;
	LoggerConParams->ul_ConnectionStatus = NOT_CONNECTED;
	LoggerConParams->ul_PrevConnState    = CONNECTED; 


	MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitLoggerTcpConn: connecting to server IP: %s",&(LoggerConParams->IpV4Addr.auc_IpV4Address));
	
	  // t.r temporary down
	l_Rc = SendV4ConnectReq(LoggerConParams->s , LoggerConParams->us_Port, LoggerConParams->IpV4Addr,eLoggerCom);    
	if (l_Rc != SOCKET_OPERATION_FAILED)
	{
    	LoggerConParams->ul_ConnectionStatus = CONNECTED;
		LoggerConParams->ul_PrevConnState = CONNECTED;
    	MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitLoggerTcpConn : Logger client and server are connected");
	}
	else	
	{
		MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitLoggerTcpConn:  could not establish connection to looger, retrying.... rc = %d\n",l_Rc);
	}
	
	memset((void*)&(LoggerConParams->TimerJobReq),0,sizeof(TTimerJobReq));

    LoggerConParams->TimerJobReq.ulTicksCount 	= 5000;
    LoggerConParams->TimerJobReq.ulInterval 	= TIMER_TRUE;
    LoggerConParams->TimerJobReq.pvContext		= (void*)LoggerConParams;
    LoggerConParams->TimerJobReq.callback 		= CheckClientConnStatus;
    LoggerConParams->ul_TimerHandle             = TimerSetJob(&(LoggerConParams->TimerJobReq));
*/
    return 1;
}
	
	
	
	
APIU32 SystemGetTickCount()
{
	struct timeval tv;

    gettimeofday(&tv,NULL);
	return tv.tv_sec * 100 + (tv.tv_usec / 10000);
}





void ConnectToLoggerTask()
{
	void* retval = NULL;
	
	if  (TcpConnection[eLoggerCom].ul_ConnectionStatus != CONNECTED)
	{
		//init the logger connection with mcms.
		InitLoggerTcpConn();
		EmbSleep(1000);
	}

	pthread_exit(retval);
}




UINT32 VerifyGeneralMcmsCommonHeader(void *pv)
{
	TGeneralMcmsCommonHeader *p = (TGeneralMcmsCommonHeader *)pv;
	UINT32 ulStatus = 0;
	
	/* COMMON HEADER */
	UINT8 ch_protocol_version 		= GET_COMMON_HEADER_PROTOCOL_VERSION(p);
	UINT8 ch_option 			 	= GET_COMMON_HEADER_OPTION(p);
	UINT8 ch_src_id					= GET_COMMON_HEADER_SRC_ID(p);
	UINT8 ch_dest_id 				= GET_COMMON_HEADER_DEST_ID(p);
	UINT32 ch_opcode 				= GET_COMMON_HEADER_OPCODE(p);
	UINT32 ch_time_stamp 			= GET_COMMON_HEADER_TIME_STAMP(p);
	UINT32 ch_sequence_num 			= GET_COMMON_HEADER_SEQUENCE_NUM(p);
	UINT32 ch_payload_len 			= GET_COMMON_HEADER_PAYLOD_LEN(p);
	UINT32 ch_payload_offset		= GET_COMMON_HEADER_PAYLOAD_OFFSET(p);
	UINT32 ch_next_header_type 		= GET_COMMON_HEADER_NEXT_HEADER_TYPE(p);
	UINT32 ch_next_header_size 		= GET_COMMON_HEADER_NEXT_HEADER_SIZE(p);
	
	/* MESSAGE DESCRIPTION HEADER */
	UINT32 mdh_request_id 			= GET_MESSAGE_DESCRIPTION_HEADER_REQUEST_ID(p);
	UINT32 mdh_entity_type 			= GET_MESSAGE_DESCRIPTION_HEADER_ENTITY_TYPE(p);
	UINT32 mdh_time_stamp 			= GET_MESSAGE_DESCRIPTION_HEADER_TIME_STEMP(p);
	UINT32 mdh_next_header_type 	= GET_MESSAGE_DESCRIPTION_HEADER_NEXT_HEADER_TYPE(p);
	UINT32 mdh_next_header_size 	= GET_MESSAGE_DESCRIPTION_HEADER_NEXT_HEADER_SIZE(p);
	
	/* PHYSICAL INFO HEADER */
	UINT8 pih_box_id 				= GET_PHYSICAL_INFO_HEADER_BOX_ID(p);
	UINT8 pih_board_id 				= GET_PHYSICAL_INFO_HEADER_BOARD_ID(p);
	UINT8 pih_sub_board_id 			= GET_PHYSICAL_INFO_HEADER_SUB_BOARD_ID(p);
	UINT8 pih_unit_id 				= GET_PHYSICAL_INFO_HEADER_UNIT_ID(p);
	UINT8 pih_port_id 				= GET_PHYSICAL_INFO_HEADER_PORT_ID(p);
	UINT8 pih_resource_type 		= GET_PHYSICAL_INFO_HEADER_RESOURCE_TYPE(p);
	UINT32 pih_next_header_type 	= GET_PHYSICAL_INFO_NEXT_HEADER_TYPE(p);
	UINT32 pih_next_header_size 	= GET_PHYSICAL_INFO_NEXT_HEADER_SIZE(p);
	
	
	MfaBoardPrint(  SHARED_PRINT,
					PRINT_LEVEL_MAJOR,
					PRINT_TO_TERMINAL,
					"COMMON HDR: prot:%d , opt:%d , src:%d , dest:%d , opcode:%d , seq_num:%d , pay_len:%d , pay_offset:%d , n_type:%d , n_size:%d.",
					ch_protocol_version,
					ch_option,
					ch_src_id,
					ch_dest_id,
					ch_opcode,
					ch_time_stamp,
					ch_sequence_num,
					ch_payload_len,
					ch_payload_offset,
					ch_next_header_type,
					ch_next_header_size );
					
/*	if(ch_src_id >= NUM_OF_MAINE_NTITIES)
		ulStatus |= 0x0001;
	
	if(ch_dest_id >= NUM_OF_MAINE_NTITIES)
		ulStatus |= 0x0002;
*/
	if(ch_next_header_type >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0004;			

	MfaBoardPrint(  SHARED_PRINT,
					PRINT_LEVEL_MAJOR,
					PRINT_TO_TERMINAL,
					"MD HDR: req_id:%d , ent_type:%d , time_stamp:%d , n_type:%d , n_size:%d.",
					mdh_request_id,
					mdh_entity_type,
					mdh_time_stamp,
					mdh_next_header_type,
					mdh_next_header_size  );
					
					
	MfaBoardPrint(  SHARED_PRINT,
					PRINT_LEVEL_MAJOR,
					PRINT_TO_TERMINAL,
					"PHYS HDR: box_id:%d , board_id:%d , sub_board:%d , unit:%d , port:%d , res_type:%d , n_type:%d , n_size:%d.",
					pih_box_id,
					pih_board_id,
					pih_sub_board_id,
					pih_unit_id,
					pih_port_id,
					pih_resource_type,
					pih_next_header_type,
					pih_next_header_size );
					
	if(pih_box_id > 1) // TBD: Make #define MAX_BOX_ID
		ulStatus |= 0x0008;	
		
	if(pih_board_id > 2) // TBD: Make #define MAX_NUM_OF_BOARDS_IN_BOX
		ulStatus |= 0x0010;
/*	benson - when general.h will be moved to include dir - we will open this remark	
	if(pih_sub_board_id > NUM_OF_SUB_BOARDS)
		ulStatus |= 0x0020;
		
	if(pih_unit_id > NUM_OF_DSPS_IN_BOARD)
		ulStatus |= 0x0040;
*/		
	//if(pih_port_id > NUMBER_OF_LIGHT_PORTS_IN_DSP)
	//	ulStatus |= 0x0080;	
		
	if(pih_resource_type >= NUM_OF_LOGICAL_RESOURCE_TYPES)
		ulStatus |= 0x0100;			
		
	if(pih_next_header_type >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0200;	
												
					
	if(ulStatus)
	{					
		MfaBoardPrint(  SHARED_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"(VerifyGeneralMcmsCommonHeader): API Invalid Params !!! (status:%Xh)",ulStatus  );
	}
	
	return ulStatus;
	
}


UINT32 VerifyPortDescriptionHeader(void *pv)
{
	TPortMessagesHeader *p = (TPortMessagesHeader *)pv;
	UINT32 ulStatus = 0;
	
	/* PORT DESCRIPTION HEADER */
	UINT32 pdh_party_id 			= GET_PORT_DESCRIPTION_HEADER_PARTY_ID(p);
	UINT32 pdh_conf_id 				= GET_PORT_DESCRIPTION_HEADER_CONF_ID(p);
	UINT32 pdh_connection_id 		= GET_PORT_DESCRIPTION_HEADER_CONNECTION_ID(p);
	UINT32 pdh_next_header_type 	= GET_PORT_DESCRIPTION_HEADER_NEXT_HEADER_TYPE(p);
	UINT32 pdh_next_header_size 	= GET_PORT_DESCRIPTION_HEADER_NEXT_HEADER_SIZE(p);
		
		
	MfaBoardPrint(  SHARED_PRINT,
					PRINT_LEVEL_MAJOR,
					PRINT_TO_TERMINAL,
					"PORT HDR: party_id:%d , conf_id:%d , connection_id:%d , n_type:%d , n_size:%d.",
					pdh_party_id,
					pdh_conf_id,
					pdh_connection_id,
					pdh_next_header_type,
					pdh_next_header_size );
/*					
	if(pdh_party_id >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0001;	
		
	if(pdh_conf_id >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0002;	
		
	if(pdh_connection_id >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0004;	
	
	if(pdh_next_header_type >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0008;												
*/					
	if(ulStatus)
	{					
		MfaBoardPrint(  SHARED_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"(VerifyPortDescriptionHeader): API Invalid Params !!! (status:%Xh)",ulStatus  );
	}
	
	return ulStatus;				
}

UINT32 VerifyAckMessageContent(void* pv)
{
	ACK_IND_S *p = (ACK_IND_S *)pv;
	UINT32 ulStatus = 0;
	
	UINT32 ack_media_type 		= SWAPL(p->media_type);
	UINT32 ack_media_direction 	= SWAPL(p->media_direction);
	UINT32 ack_opcode 			= SWAPL(p->ack_base.ack_opcode);
	UINT32 ack_seq_num 			= SWAPL(p->ack_base.ack_seq_num);
	UINT32 ack_status 			= SWAPL(p->ack_base.status);
	UINT32 ack_reason 			= SWAPL(p->ack_base.reason);
		
	/* ACK MESSAGE */
	MfaBoardPrint(  SHARED_PRINT,
					PRINT_LEVEL_MAJOR,
					PRINT_TO_TERMINAL,
					"ACK HDR: media_type:%d , media_direction:%d , ack_opcode:%d , ack_seq_num:%d , status:%d , reason:%d.",
					ack_media_type,
					ack_media_direction,
					ack_opcode,
					ack_seq_num,
					ack_status,
					ack_reason );
/*					
	if(ack_media_type >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0001;	
		
	if(ack_media_direction >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0002;	
		
	if(ack_opcode >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0004;	
	
	if(ack_seq_num >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0008;
		
	if(ack_status >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0010;	
		
	if(ack_reason >= NUM_OF_HEADER_TYPES)
		ulStatus |= 0x0020;					
*/					
	if(ulStatus)
	{					
		MfaBoardPrint(  SHARED_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"(VerifyAckMessageContent): API Invalid Params !!! (status:%Xh)",ulStatus  );
	}
	
	return ulStatus;				
}

UINT32 VerifyPortMessageHeader(void* pv)
{
	TPortMessagesHeader *p = (TPortMessagesHeader *)pv;
	UINT32 ulStatus = 0;
	 
	ulStatus = VerifyGeneralMcmsCommonHeader(&p->tGeneralMcmsCommonHeader);
	if(ulStatus)
		return ulStatus;
	
	ulStatus = VerifyPortDescriptionHeader(&p->tPortDescriptionHeader);
	if(ulStatus)
		return ulStatus;
    
    return 0;
}

UINT32 VerifyPortMessageAckType(void* pv)
{
	TPortMessageAckType *p = (TPortMessageAckType *)pv;
	UINT32 ulStatus = 0;
	
	ulStatus = VerifyPortMessageHeader(&p->tPortMessagesHeader);
	if(ulStatus)
		return ulStatus;
	
	ulStatus = VerifyAckMessageContent(&p->tAckMessageContent);
	if(ulStatus)
		return ulStatus;

    return 0;
}

UINT32 VerifyGnrlMessageAckType(void* pv)
{
	TGeneralMessageAckType *p = (TGeneralMessageAckType *)pv;
	UINT32 ulStatus = 0;
	
	ulStatus = VerifyGeneralMcmsCommonHeader(&p->tGeneralMcmsCommonHeader);
	if(ulStatus)
		return ulStatus;
	
	ulStatus = VerifyAckMessageContent(&p->tAckMessageContent);
	if(ulStatus)
		return ulStatus;

    return 0;
}

 	  			 	  			
/****************************************************************************
* Prototype:        VerifyUINT32ParamInBoundaries
* Description:      Check that the parameter is in its boundaries
* Return Value:     STATUS (OK/ERROR = 0/-1)
* Arguments:     	Value - Parameter to check
* 			     	Low_bound - The low boundary (value must be bigger than Low_bound)
* 			     	High_bound - The high boundary (value must be smaller than Low_bound)
* 					ParamName - The parameter name for printout
*****************************************************************************/

UINT32 VerifyUINT32ParamInBoundaries(UINT32 Value,UINT32 Low_bound, UINT32 High_bound, UINT8* ParamName, UINT32 bIsNeedToPrint)
{

	UINT32 ReturnValue;
	//Init the return value to FALSE
	ReturnValue = -1;

	//Print Parameter and boundaries	
	if(bIsNeedToPrint)
	{   
		MfaBoardPrint(SHARED_PRINT ,PRINT_LEVEL_MINOR ,PRINT_TO_LOGGER, "(VerifyUINT32ParamInBoundaries) : Value = %d, Low_bound = %d, High_bound = %d, ParamName = %s\n", Value, Low_bound, High_bound, ParamName);
	}
	// if value in boundaries - return TRUE
	if((Value >= Low_bound)&&(Value <  High_bound))		                                      	
	{  
		ReturnValue = 0;
	}
	else //value not in boundaries, return False and update status
	{   
		MfaBoardPrint(SHARED_PRINT,	PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL, "(VerifyUINT32ParamInBoundaries) : Value = %d, Low_bound = %d, High_bound = %d, ParamName = %s, out of boundaries!!!", Value, Low_bound, High_bound, ParamName);      			                            	
		ReturnValue = -1;
	}   	
	return ReturnValue;
}
