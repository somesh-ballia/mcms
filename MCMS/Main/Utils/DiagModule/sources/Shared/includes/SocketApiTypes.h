#ifndef _SOCKEAPITYPES_
#define _SOCKEAPITYPES_


/*====================================================================*/
/*            Copyright     ???    2005  Polycom Israel, Ltd. All rights reserved                                       */
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

#include <sys/select.h>


#include "DiagDataTypes.h"
#include "TimerMngr.h"
#include <net/if.h>
#include <errno.h>

// #define X86_SIM 1

/** include files **/


/** local definitions **/

#define SOCKET INT32

#define SOCKET_OPERATION_OK 1
#define SOCKET_OPERATION_FAILED -1
#define	TPCKT_OPERATION_FAILED -2
#define NO_DATA 0
#define NO_FLAGS 0
#define V4_ADDRESS_SIZE 32
#define V6_ADDRESS_SIZE	100

/* connection status */
#define NOT_CONNECTED 0xDEAD
#define CONNECTED 0xF00D






#define GetLastError() errno

typedef struct
{
  UINT8 auc_IpV4Address[V4_ADDRESS_SIZE];
  UINT8 auc_NetMask[V4_ADDRESS_SIZE];
  UINT8 auc_DefaultGetway[V4_ADDRESS_SIZE];

}t_IpV4Address;


typedef struct
{
	INT8 acIfName[IFNAMSIZ];
	INT8 acIfNameVlan[IFNAMSIZ];
	INT8 acIfNameModem[15];
	UINT32 unInternalNetMask;
	UINT32 unModemNetMask;
	UINT8 aucEthName[5];
	
}	IF_NAME_STRUCT;




typedef struct 
{
 UINT8 auc_IpV6Address[V6_ADDRESS_SIZE];
}t_IpV6Address;

typedef struct
{
  UINT16 us_Length;  
  UINT8 uc_Version;
  UINT8 uc_Reserved;
}t_TpktType;

typedef struct
{
  UINT8 uc_Version;
  UINT8 uc_Reserved;
  UINT16 us_Length;  
}t_LETpktType;


typedef struct
{
  SOCKET l_Socket;
  VOID *vp_RcvBuffer; 
  INT32 s_Length;
  INT32 l_flags;
}t_RcvParamsType;
  
typedef struct
{
  SOCKET l_Socket;
  VOID *vp_RcvBuffer; 
  INT32 s_Length;
  INT32 l_flags;
}t_XmitParamsType;

typedef struct
{
	INT32 l_onoff;
	INT32 l_linger;
	
}linger;

typedef struct 
{
	UINT32 ulIpAddr ;
	UINT32 ulSubNetMask ;
	UINT32 ulDefGateWay ;
}Ni_Params ;

//l.a. startpu api state machine values
enum eSTAT__INIT_BOARD
{
 eSTAT__INIT_BOARD_CFG,
 eSTAT__WAIT_FOR_UNIT_CFG_REQ, 
 eSTAT__WAIT_FOR_MEDIA_IP_REQ, 
 eSTAT__INIT_BOARD_DONE,
 eMAX_STAT_NUM
};

#define MAX_SUB_BOARDS_NUM 2
#define GET_CARD_NUM()	(0)

#define TPKT_SIZE sizeof(t_LETpktType)

extern fd_set *ptTcpActiveReadSockets;
extern fd_set *ptTcpActiveSelectSockets;

// t.r definitions for tcp connections
typedef enum
{
  eConnTypeClient,
  eConnTypeServer
}e_ClientOrServer;

typedef enum
{
  eMcmsCom,
  eLoggerCom,
  eIpmiClient,
  eIpmiServer,
  eLanStatClient,
  eLanStatServer,
  eEmaApiClient,
  eEmaApiServer,
  eEmaSimServer, 
  eMfa1DiagServer,
  eMfa2DiagServer,
  eMfa3DiagServer,
  eMfa4DiagServer, 
  eMfa1ResetServer,
  eMfa2ResetServer,
  eMfa3ResetServer, 
  eMfa4ResetServer,
  eShelfApiServer, 
  eSwitchDiagClient, 
  eSwitchDiagServer, 
  eCpuDiagServer, 
  eMaxTcpConnections
}e_TcpConn;



//#define MAX_TCP_CONNECTIONS eMaxTcpConnections;


typedef struct 
{
	e_TcpConn     e_Id;
	SOCKET        s;
  	UINT16        us_Port;
  	t_IpV4Address IpV4Addr;
  	UINT32        ul_TimerHandle;
  	TTimerJobReq  TimerJobReq;
  	UINT32 		  ul_ConnectionStatus; 
  	UINT32 		  ul_PrevConnState;
	UINT32		  ul_ClientOrServer; // use enum e_ClientOrServer
} 	t_TcpConnParams;

			 /* read from flash  the port number used to communicate with MP-H */


#define GET_MFA_V4_IP_ADDRESS(X)       strcpy((char *)X,"192.168.100.100") 
#define GET_MFA_V4_DEFAULT_GATEWAY(X)  strcpy((char *)X,"192.168.100.1")
#define GET_MFA_V4_SUBNET_MASK(X)	   strcpy((char *)X,"255.255.0.0")
#endif

