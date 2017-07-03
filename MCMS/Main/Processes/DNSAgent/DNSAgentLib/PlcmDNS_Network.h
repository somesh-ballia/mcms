// 
// ==========================================
//    Copyright (C) 2014           "POLYCOM"
// ==========================================
// FileName:               PlcmDNS_Network.h  
// Include line recommended:
// #include "PlcmDNS_Network.h"  //
// 
// ==========================================

#ifndef _PLCMDNS_NETWORK_H_
#define _PLCMDNS_NETWORK_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/ioctl.h>

#include <errno.h>

#include "SharedDefines.h"
#include "IpAddressDefinitions.h"
#include "PlcmDNS_Defines.h"
#include "PObject.h"

typedef int					SOCKET		;
typedef struct sockaddr_in	SOCKADDR_IN ;
typedef struct sockaddr_in6 SOCKADDR_IN6;

typedef struct sockaddr		SOCKADDR	;
typedef struct sockaddr		*LPSOCKADDR ;

typedef struct hostent		HOSTENT		;
typedef socklen_t			SOCKLEN_T	;	

typedef unsigned short		PORTINT		;

#define INVALID_SOCKET		(-1)
#define SOCKET_ERROR		(-1)


typedef enum _PLe_SOCK_ERROR_CODE
{
	 PLE_SOCKET_OK       = 0
	,PLE_SOCKET_ERROR
	,PLE_SOCKET_TIMEOUT
	,PLE_RECV_REJECT
}
PLe_SOCK_ERROR_CODE;


#define  SOCK_TRY_TO_SELECT_INTERVAL_msec  120   // milliseconds
#define  SOCK_TRY_TO_SELECT_TIMEOUT_msec   10
#define  INTERNAL_SELECT_WAIT_sec          5    // Maximum permitted idle for "receive" completing
#define  INTERNAL_SOCKET_WAIT              500     
#define  MILLISEC_IN_SECOND                1000
#define  SOCK_TEST_READ_RECEIVED           1 
#define  SOCK_TEST_READ_TIMEOUT            2 
    

#define  FD_EVENT_TIMEOUT				   0
#define  FD_EVENT_READ					   1
#define  FD_EVENT_WRITE					   2
#define  FD_EVENT_EXCEPT				   3

#define  FD_SELECT_TIMEOUT				   0	
#define  FD_SELECT_EVENT				   1	
#define  FD_SELECT_EXCEPT				  -1	




//===============================================================//
int			plGetlastError() ;
SOCKET      plUDPSocket4			(char * par_pszErrorMsg);
SOCKET      plUDPSocket6			(char * par_pszErrorMsg);
SOCKET      plUDPSocket				(enIpVersion par_eIpVer, char *par_pszErrorMsg);
void        plCloseUdpSocket		(SOCKET sock);
enIpVersion plGetIpAddressVer		(char * par_szAddr);
BOOL		plSendTo				(SOCKET par_sock , char * par_pszIP , unsigned short par_wPort
									,char * par_pData, int par_nDataSize, char * par_pszErr);

BOOL		plBind					(SOCKET par_sock, unsigned short par_wPort
									 , enIpVersion par_eIpVer, char *par_pszErr);

BOOL		plBindA					(SOCKET par_sock, unsigned short par_wPort
									 , enIpVersion par_eIpVer, char *par_pszErr, char par_szIP);

BOOL        plRecvFrom4				(SOCKET par_sock, char * par_pData, int par_nSize
									,int  * par_pnBytesRcvd,  char *pszErrorMsg
									,unsigned int * par_dwSourceIP, unsigned short * par_pSourcePort);

enIpVersion		plcm_GetPeerAddressS(int par_nSock, char * szIpAddress, int nBffLen);
unsigned short  pclm_GetPeerPortS	(int  par_Sock);
char    *       plcmIpToStringV4    (unsigned int dwIP, char *pszOutIP, int par_nOutLen);
char    *		plcmIpToStringV6    (unsigned char * ipAddress,char	* par_szOutIpAddress, int par_nOutLen,BOOL addBrackets);



typedef enum _eDNS_SOCKET_STATE
{
    eDNS_SOCKET_UNDEF = 0
  
  , eDNS_SOCKET_OPENED
  , eDNS_SOCKET_BINDED 
  , eDNS_SOCKET_CLOSED

  , eDNS_SOCKET_BIND_FAILED
  , eDNS_SOCKET_OPEN_FAILED

  , eDNS_SOCKET_LASTENUM
}
eDNS_SOCKET_STATE;


class cDNS_SOCKET : public CPObject
{
	CLASS_TYPE_1(cDNS_SOCKET, CPObject)
public: 
	virtual const char*  NameOf() const {return "cDNS_SOCKET";}
	cDNS_SOCKET();
	cDNS_SOCKET(enIpVersion par_eIpV);
	cDNS_SOCKET(enIpVersion par_eIpV, unsigned short par_wPort);
   ~cDNS_SOCKET();

public:

	BOOL        m_bSosketActivateA      (enIpVersion par_eIpV, unsigned short par_wPort, char * par_szIP);  
	int			m_plGetlastError		();
	SOCKET      m_UDPSocket4			();
	SOCKET      m_UDPSocket6			();
	SOCKET      m_UDPSocket				();
	void        m_CloseUdpSocket		();
	BOOL        m_plSendTo				(char * par_pszIP , unsigned short par_wPort, char * par_pData, int par_nDataSize);
	BOOL		m_plBindA				(unsigned short par_wPort, char * par_szIP);

	BOOL		m_SendTo				(char * par_pszIP , unsigned short par_wPort,char * par_pData, int par_nDataSize);
	int         m_RecvFrom4			    (char * par_pData, int par_nSize, char * par_SourceIP, int par_nSourceIPLen, unsigned short * par_pSourcePort);
	int         m_RecvFrom6			    (char * par_pData, int par_nSize, char * par_SourceIP, int par_nSourceIPLen, unsigned short * par_pSourcePort);
	
	int         m_RecvFrom4A		    (char * par_pData, int par_nSize, char * par_SourceIP, int par_nSourceIPLen, unsigned short * par_pSourcePort);
	int         m_RecvFrom6A		    (char * par_pData, int par_nSize, char * par_SourceIP, int par_nSourceIPLen, unsigned short * par_pSourcePort);

	char   *    m_szGetStrSocketState    (); 

	void		m_SetSelectReadTimeout   (unsigned int par_dwSelectTimeoutMilliSec);

public:
	enIpVersion			m_IpV						;
	SOCKET				m_Socket					;
    char				m_szErrMsg[256]				;
	unsigned int        m_dwSelectReadTimeoutMillSec;
	eDNS_SOCKET_STATE	m_eSate						;
};



typedef struct _EVENT_INFO
{
	int				eSelectEvent; // FD_EVENT_TIMEOUT | FD_EVENT_READ |FD_EVENT_WRITE | FD_EVENT_EXCEPT
	cDNS_SOCKET  *  pDnsSocket  ;
}
EVENT_INFO;


int				plcmSockListReadSelectTimeOut(SOCKET * par_pSocketList, EVENT_INFO * par_pEventList
											, int par_nListLen, unsigned int  par_dwTimeoutMilliSec);



#endif //_PLCMDNS_NETWORK_H_



