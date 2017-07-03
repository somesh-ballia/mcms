#ifndef _SOCKEAPIWRAP_
#define _SOCKEAPIWRAP_


/*====================================================================*/
/*            Copyright     ???    2001  Polycom Israel, Ltd. All rights reserved                                       */
/*-------------------------------------------------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary   information of                */
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form   */
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without          */
/* prior written authorization from Polycom Israel Ltd.                                                                    */
/*-------------------------------------------------------------------------------------------------------------------*/
/* FILE:     SocketApiWrap.h                                                                                                                        */
/* PROJECT:  Gideon                                                                                                   */
/* PROGRAMMER:  Tzvicka Reznick                                              .                                                  */
/* FUNCTION LIST:                                                                                                                         */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                                                                                         */
/*-------------------------------------------------------------------------------------------------------------------*/
/*  t.r   |   21/03/05      |    create file                                                                                                                */
/*====================================================================*/


/** include files **/

#include  <sys/socket.h>
#include  <netinet/in.h>

#include "DiagDataTypes.h" 
#include "SocketApiTypes.h"
// #include "TimerMngr.h"

/* #include "SocketApiTypes.h" */

/** local definitions **/
/* Benson - Moved to SocketApiTypesExt.h
typedef struct 
{
	UINT32 ulIpAddr ;
	UINT32 ulSubNetMask ;
	UINT32 ulDefGateWay ;
}Ni_Params ;
*/

/* default settings */
#define EMB_OK    0
#define EMB_FAIL -1
#define SEND_WRAP_RETRIES 100

/** external functions **/

//extern SOCKET l_ClientSocket;
extern SOCKET nLoggerSocket;
//extern UINT32 ul_ConnectionStatus;
extern UINT32 unPrintLoggerConnectionStatus;

/** external data **/

/** internal functions **/

/** public data **/

/** private data **/

/** public functions **/

//[Dotan.H 02/08/2009 send() wrapper]
INT32 sendWrap( SOCKET l_Socket, UINT8* vp_RcvBuffer, UINT16 us_Length,INT32 l_flags , UINT32 unPrintFlag);

void  ConnectToTcpServer(t_TcpConnParams* ptConParams);
void AddVlan(UINT16 unVlanID, UINT8 * puc_Ifname);
SOCKET InitV4Connection();
INT32 TCPRecvData( SOCKET l_Socket, VOID **vp_RcvBuffer,INT32 l_flags);
INT32 TCPSendData( SOCKET l_Socket, VOID *vp_RcvBuffer, UINT16 us_Length,INT32 l_flags,UINT32 unPrintFlag);
INT32 CloseConnection( SOCKET l_Socket, INT32 l_How );
INT32 SendV4ConnectReq(SOCKET s , UINT16 us_RemotePort,t_IpV4Address t_DestIpV4Addr,e_TcpConn eId);	
INT32 ConnectV4Connection(SOCKET s , UINT16 us_RemotePort,t_IpV4Address t_DestIpV4Addr,e_TcpConn eId);
void  ReConnectV4Connection(SOCKET s , UINT16 us_RemotePort,t_IpV4Address t_DestIpV4Addr,e_TcpConn eId);
INT32 main();
INT32 CreateNewProcessFromFunc();
INT32 InitClient();
void CheckClientConnStatus(void *p);
void CheckServerConnStatus(void *p);
INT32 GetV4NiParams(UINT8 * puc_Ifname, Ni_Params *pt_V4NiAddress) ;

void ClientProcess();
void TCPrcvThread();


//typedef struct 
//{
//	SOCKET        s;
//  	UINT16        us_Port;
//  	t_IpV4Address IpV4Addr;
//  	UINT32        ul_TimerHandle;
//  	TTimerJobReq  TimerJobReq;  
//} 	t_TcpConnParams;

//typedef struct 
//{
//	INT32 l_onoff;
//	INT32 l_linger;
	
// }   linger;




/** private functions **/


#endif

