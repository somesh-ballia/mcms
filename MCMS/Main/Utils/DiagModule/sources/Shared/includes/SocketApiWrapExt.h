#ifndef _SOCKEAPITYPESEXT_
#define _SOCKEAPITYPESEXT_

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


/** local definitions **/
// extern SOCKET l_ClientSocket;
extern SOCKET nLoggerSocket;
// extern UINT32 ul_ConnectionStatus;
extern UINT32 unPrintLoggerConnectionStatus;


extern t_TcpConnParams TcpConnection[eMaxTcpConnections];



/* default settings */

/** external functions **/

/** external data **/

/** internal functions **/

/** public data **/

/** private data **/


/** public functions **/
extern void ConnectToTcpServer(t_TcpConnParams* ptConParams);
extern void RegisterToTimerJob(t_TcpConnParams* ptConParams, UINT32 ulInterval, void *pCallBackFunc);
extern void AddVlan(UINT16 unVlanID, UINT8 * puc_Ifname);
extern SOCKET InitV4Connection();
extern INT32 TCPRecvData( SOCKET l_Socket, VOID **vp_RcvBuffer,INT32 l_flags);
extern INT32 TCPSendData( SOCKET l_Socket, VOID *vp_RcvBuffer, UINT16 us_Length,INT32 l_flags, UINT32 unPrintFlag);
extern INT32 CloseConnection( SOCKET l_Socket, INT32 l_How );
extern INT32 SendV4ConnectReq(SOCKET s , UINT16 us_RemotePort,t_IpV4Address t_DestIpV4Addr,e_TcpConn eId);	
extern INT32 ConnectV4Connection(SOCKET s , UINT16 us_RemotePort,t_IpV4Address t_DestIpV4Addr,e_TcpConn eId);
extern void  ReConnectV4Connection(SOCKET s , UINT16 us_RemotePort,t_IpV4Address t_DestIpV4Addr,e_TcpConn eId);
extern void CheckClientConnStatus(void *p);
extern void CheckServerConnStatus(void *p);
extern INT32 main();
extern INT32 CreateNewProcessFromFunc();
extern INT32 CreateNewProcessFromExec();
extern INT32 InitClient();
extern INT32 GetV4NiParams(UINT8 * puc_Ifname, Ni_Params *pt_V4NiAddress) ;
extern void ClientProcess();
extern void TCPrcvThread();





/** private functions **/

#endif


