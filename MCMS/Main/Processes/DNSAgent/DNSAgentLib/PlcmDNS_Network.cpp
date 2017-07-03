//==============================================================
// PlcmDNS_Network.cpp
//==============================================================

#include "PlcmDNS_Network.h"
#include "TraceStream.h"
#include "PlcmDNS_Tools.h" 


#define PL_ERR_OPEN_SOCKET4			"Failed to open socket IPv4. Error:[%d]"
#define PL_ERR_OPEN_SOCKET6			"Failed to open socket IPv6. Error:[%d]"
#define PL_ERR_PTON_IPADDR6			"Failed inet_pton for IPv6:[%s]"
#define PL_ERR_SEND_TO				"Failed SENDTO [%s:%d] Data length:%d"
#define PL_ERR_SEND_TO_IN_PARAMS	"Failed SENDTO. Error IN params"
#define PL_ERR_SEND_TO_IN_PAR_IPV	"Failed SENDTO. IP-Add doesn't match to IPver. Par:[%s] | Det:[%s] | IP[%s]"
#define PL_ERR_RECVFROM_IN_PARAMS	"Failed RECVFROM. Error IN params"
#define PL_ERR_BIND_IN_PARAMS       "Failed BIND. Error IN params"
#define PL_ERR_BIND			        "Failed BIND. Error:[%d]"


#define PL_ERR_BIND_SOCKET      "Failed to bind check listener socket. Error:[%d] Port:[%d]"
#define PL_ERR_LISTENER_SOCKET  "Failed to try listening. (%d)"
#define PL_ERR_CLOSE_SOCKET     "The socket of the listener was closed outside. (%d)"
#define PL_ERR_CONNECT_SOCKET   "Failed to connect the socket. (%d)"
#define PL_ERR_READ_DATA_SOCKET "Failed to read data from the socket. (%d)"
#define PL_ERR_SEND_DATA_SOCKET "Failed to send data to the socket. (%d)"
//#define AQS_ERR_GET_IP            "Failed to get local IP address. (%d)"


#define PL_SOCKET_INET_PTON_FAILED 0xAB10002


#define PL_EINPROGRESS   EINPROGRESS
#define PL_EWOULDBLOCK   EWOULDBLOCK

//==============================================================
int  plGetlastError()
{
	return errno ;
}
//==============================================================
//==============================================================
SOCKET      plUDPSocket4(char * par_pszErrorMsg)
{
	SOCKET  sock	= INVALID_SOCKET;
	int     nError	= 0;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (INVALID_SOCKET == sock)
	{
		if (par_pszErrorMsg)
		{
			nError = plGetlastError ();
			sprintf(par_pszErrorMsg, PL_ERR_OPEN_SOCKET4,	nError);
		}
	}
	return sock;
}
//==============================================================
//==============================================================
SOCKET      plUDPSocket6(char *par_pszErrorMsg)
{
	SOCKET  sock	= INVALID_SOCKET;
	int     nError	= 0;

	sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	if (INVALID_SOCKET == sock)
	{
		if (par_pszErrorMsg)
		{
			nError = plGetlastError ();
			sprintf(par_pszErrorMsg, PL_ERR_OPEN_SOCKET6,	nError);
		}
	}
	return sock;
}
//==============================================================
//==============================================================
SOCKET      plUDPSocket(enIpVersion par_eIpVer, char *par_pszErrorMsg)
{
	SOCKET  sock       = INVALID_SOCKET;
	int     nError     = 0;
	int		nIPVersion = AF_UNSPEC;

	if(eIpVersion4 == par_eIpVer)
		sock = plUDPSocket4(par_pszErrorMsg);
	else if(eIpVersion6 == par_eIpVer)
		sock = plUDPSocket6(par_pszErrorMsg);

	return sock;
}
//==============================================================
//==============================================================
void        plCloseUdpSocket(SOCKET par_sock)
{
	if(INVALID_SOCKET != par_sock)
    {
       close (par_sock);
	}
}
//==============================================================

//==============================================================
BOOL spIsIpV6Str(char *sIpAddress)
{
	int index;

	//If we add port-number to the IPv6, the IPv6 must be under [].
	if (sIpAddress[0] == '[')
		return TRUE;

	//In IPv6 one of the first 5 characters must be ':'
	for (index=0; index<5; index++){
		if (sIpAddress[index] == ':'){
			return TRUE;
		}
	}

	return FALSE;
}//spIsIpV6Str
//==============================================================

//==============================================================
enIpVersion plGetIpAddressVer(char * par_szAddr)
{
	enIpVersion eRc = enIpVersionMAX;
	if(NULL != par_szAddr)
	{
		if(   ((par_szAddr[0] >= '0') && ((par_szAddr[0] <= '9') )) 
		   ||(strchr((par_szAddr), ':'))  
		   )
		{
			if(TRUE == spIsIpV6Str(par_szAddr))
				eRc = eIpVersion6;
			else
				eRc = eIpVersion4;
		}
	}
	return eRc;
}
//==============================================================

//==============================================================
BOOL    plSendTo(SOCKET par_sock , char * par_pszIP , unsigned short par_wPort
				 ,char * par_pData, int par_nDataSize, char * par_pszErr)
{
    BOOL				bRC			= FALSE;
    SOCKADDR_IN         ServAddr4;
	SOCKADDR_IN6		ServAddr6;	
    int					nError      = 0  ;
	unsigned int		dwServAddr4 = 0x0;
	int                 nSentLen	= 0  ;
	enIpVersion			eIpVer  = plGetIpAddressVer(par_pszIP);

	if(  (NULL != par_pszIP)&&(NULL != par_pData)&&(0 < par_nDataSize)&&(INVALID_SOCKET != par_sock)
       &&((eIpVersion6 == eIpVer)||(eIpVersion4 == eIpVer)) 
	  )
	{
		if(eIpVersion6 == eIpVer)
		{
			memset(&ServAddr6, 0, sizeof(struct sockaddr_in6));
			ServAddr6.sin6_family = AF_INET6;
			ServAddr6.sin6_port = htons(par_wPort);
			if(0 < inet_pton(AF_INET6, par_pszIP, &ServAddr6.sin6_addr))
			{
				nSentLen = sendto(par_sock, par_pData, par_nDataSize, 0, (struct sockaddr *) &ServAddr6, sizeof(SOCKADDR_IN6));
			}
			else
			{
				//ERROR
				nError = PL_SOCKET_INET_PTON_FAILED;
				if(NULL != par_pszErr)
					sprintf(par_pszErr, PL_ERR_PTON_IPADDR6, par_pszIP);
			}
		}
		else
		if(eIpVersion4 == eIpVer)
		{
			dwServAddr4 = inet_addr (par_pszIP);
			ServAddr4.sin_family = AF_INET;
			ServAddr4.sin_port = htons (par_wPort);
			ServAddr4.sin_addr.s_addr = dwServAddr4;

			nSentLen = sendto (par_sock, par_pData, par_nDataSize, 0, (SOCKADDR *) &ServAddr4, sizeof(SOCKADDR_IN));
		}

		if(PL_SOCKET_INET_PTON_FAILED != nError)
		{
			if(nSentLen != par_nDataSize)
			{
				if(NULL != par_pszErr)
					sprintf(par_pszErr, PL_ERR_SEND_TO, par_pszIP, par_wPort, par_nDataSize);
			}
			else
				bRC = TRUE;
		}
	}
	else
		if(NULL != par_pszErr)
			sprintf(par_pszErr, PL_ERR_SEND_TO_IN_PARAMS);

    return bRC;
}
//==============================================================

//==============================================================
BOOL    plBindA(SOCKET par_sock, unsigned short par_wPort, enIpVersion par_eIpVer, char *par_pszErr, char * par_szIP)
{
	bool			bRC			= TRUE	;
	int				nError				;
	SOCKADDR_IN		sockAddr4			;
	SOCKADDR_IN6	sockAddr6			;

	memset(&sockAddr4, 0, sizeof(SOCKADDR_IN) );
	memset(&sockAddr6, 0, sizeof(SOCKADDR_IN6));

	if(  (INVALID_SOCKET != par_sock)
		&&((eIpVersion4 == par_eIpVer)||(eIpVersion6 == par_eIpVer))
		)
	{
		if(eIpVersion4 == par_eIpVer)
		{
			sockAddr4.sin_family = AF_INET;
			sockAddr4.sin_port = htons (par_wPort);
			if(NULL == par_szIP)
				sockAddr4.sin_addr.s_addr = htonl(INADDR_ANY);
			else
			{   
				if(0 == strlen(par_szIP))
					sockAddr4.sin_addr.s_addr = htonl(INADDR_ANY);
				else
				{
					sockAddr4.sin_addr.s_addr = inet_addr(par_szIP); 
				}
			}

			nError = bind(par_sock, (SOCKADDR*) &sockAddr4, sizeof(SOCKADDR));
		}
		else
			if(eIpVersion6 == par_eIpVer)
			{
				sockAddr6.sin6_family   = AF_INET6;
				if(NULL == par_szIP)
					sockAddr6.sin6_addr     = in6addr_any;
				else
				{
					if(0 == strlen(par_szIP))
						sockAddr6.sin6_addr     = in6addr_any;
					else
					{
						inet_pton(AF_INET6, par_szIP, (void *)&sockAddr6.sin6_addr.s6_addr);
					}
				}
				sockAddr6.sin6_port     = htons(par_wPort);
				sockAddr6.sin6_scope_id = 0;

				nError = bind(par_sock, (SOCKADDR*) &sockAddr6, sizeof(SOCKADDR_IN6));
			}

			if(SOCKET_ERROR == nError)
			{
				if(NULL != par_pszErr)
				{
					int nBindErr = plGetlastError();
					sprintf(par_pszErr, PL_ERR_BIND, nBindErr);
				}
				bRC = FALSE;
			}
			else
				bRC = TRUE; 
	}
	else
	{
		if(NULL != par_pszErr)
			sprintf(par_pszErr, PL_ERR_BIND_IN_PARAMS);
	}


	return bRC;
}
//==============================================================

//==============================================================
BOOL    plBind(SOCKET par_sock, unsigned short par_wPort, enIpVersion par_eIpVer, char *par_pszErr)
{
	bool			bRC			= TRUE	;
	int				nError				;
	SOCKADDR_IN		sockAddr4			;
	SOCKADDR_IN6	sockAddr6			;

	memset(&sockAddr4, 0, sizeof(SOCKADDR_IN) );
	memset(&sockAddr6, 0, sizeof(SOCKADDR_IN6));

	if(  (INVALID_SOCKET != par_sock)
       &&((eIpVersion4 == par_eIpVer)||(eIpVersion6 == par_eIpVer))
	  )
	{
		if(eIpVersion4 == par_eIpVer)
		{
			sockAddr4.sin_family = AF_INET;
			sockAddr4.sin_port = htons (par_wPort);
			sockAddr4.sin_addr.s_addr = htonl(INADDR_ANY);

			nError = bind(par_sock, (SOCKADDR*) &sockAddr4, sizeof(SOCKADDR));
		}
		else
		if(eIpVersion6 == par_eIpVer)
		{
			sockAddr6.sin6_family   = AF_INET6;
			sockAddr6.sin6_addr     = in6addr_any;
			sockAddr6.sin6_port     = htons(par_wPort);
			sockAddr6.sin6_scope_id = 0;

			nError = bind(par_sock, (SOCKADDR*) &sockAddr6, sizeof(SOCKADDR_IN6));
		}

		if(SOCKET_ERROR == nError)
		{
			if(NULL != par_pszErr)
			{
				int nBindErr = plGetlastError();
				sprintf(par_pszErr, PL_ERR_BIND, nBindErr);
			}
			bRC = FALSE;
		}
		else
			bRC = TRUE; 
	}
	else
	{
		if(NULL != par_pszErr)
			sprintf(par_pszErr, PL_ERR_BIND_IN_PARAMS);
	}


	return bRC;
}
//==============================================================

//==============================================================
unsigned short  pclm_GetPeerPortS(int  par_Sock)
{
	unsigned short wRc = 0;
	if(INVALID_SOCKET != par_Sock)
	{
		struct sockaddr		addr;
		socklen_t			addr_len = sizeof(addr);
		int                 nSockError;

		memset(&addr, 0, addr_len);

		nSockError = getpeername (par_Sock,(struct sockaddr*)&addr, &addr_len);

		if (nSockError != SOCKET_ERROR)
		{
			if(addr.sa_family == AF_INET)
			{//AF_INET
				struct sockaddr_in *s4 = (struct sockaddr_in *)&addr;
				wRc = ntohs(s4->sin_port);
			} 
			else 
			{//AF_INET6
				struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)&addr;
				wRc = ntohs(s6->sin6_port);
			}
		}
	}

	return wRc; 
}
//==============================================================

//==============================================================
//Return code:    2(enIpVersionMAX) = error; 
//                0(eIpVersion4)    = IPv4 ; 
//                1(eIpVersion6)    = IPv6 ;
enIpVersion  plcm_GetPeerAddressS(int par_nSock, char * szIpAddress, int nBffLen)
{
	enIpVersion nRc = enIpVersionMAX;
	if(INVALID_SOCKET != par_nSock)
	{
		int                         nSockError;
		char                        ipstr[PLCM_STRING_SIZE_256]="";
		char						cAddr[PLCM_STRING_SIZE_256];

		memset(ipstr, '\0', sizeof(ipstr));
		memset(&cAddr[0], 0, PLCM_STRING_SIZE_256);
		socklen_t addr_len = PLCM_STRING_SIZE_256;

		nSockError = getpeername (par_nSock,(struct sockaddr*)&cAddr, &addr_len);

		if (nSockError != SOCKET_ERROR)
		{
			// deal with both IPv4 and IPv6:
			struct sockaddr *paddr;
			paddr = (struct sockaddr*)&cAddr;
			if ( paddr->sa_family == AF_INET)
			{//AF_INET
				struct sockaddr_in *s4 = (struct sockaddr_in *)&cAddr;
				inet_ntop(AF_INET, &s4->sin_addr, ipstr, sizeof(ipstr));
				nRc = eIpVersion4;
			} 
			else 
			{ // AF_INET6
				struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)&cAddr;
				inet_ntop(AF_INET6, &s6->sin6_addr, ipstr, sizeof(ipstr));
				nRc = eIpVersion6;
			}

			if((NULL != szIpAddress)&&(nBffLen > (int)strlen(ipstr)) )
			{
				strncpy(szIpAddress, ipstr, nBffLen);
			} 
			else 
			{
				//
			}
		}
	}

	return nRc;
}
//==============================================================
//==============================================================
BOOL        plRecvFromv4( SOCKET			par_sock
					    , char			*	par_pReceivedBuff
					    , int				par_nBuffSize
					    , int			*	par_pnBytesRcvd
					    , char			*	par_pszErrorMsg
					    , unsigned int	*	par_dwSourceIP
					    , unsigned short*	par_pSourcePort)
{
	 BOOL				bRC	= FALSE		;
	 SOCKLEN_T			nTmp			;
	 SOCKADDR_IN        ServAddr4		;
	 int				nBytesRcvd = 0	;

	 memset(&ServAddr4, 0, sizeof (ServAddr4));
	 nTmp = sizeof(ServAddr4);

	 if(  (NULL != par_pReceivedBuff)
		&&(0 < par_nBuffSize)
		&&(INVALID_SOCKET != par_sock)
	    )
	 {
		nBytesRcvd = recvfrom(par_sock , par_pReceivedBuff, par_nBuffSize, 0, (SOCKADDR *) & ServAddr4, & nTmp);

		if (nBytesRcvd > 0)
		{
			if(NULL != par_pnBytesRcvd)
			*par_pnBytesRcvd = nBytesRcvd;

			if(NULL != par_dwSourceIP)
				*par_dwSourceIP   = ntohl(ServAddr4.sin_addr.s_addr); 
			if(NULL != par_pSourcePort)
				*par_pSourcePort = ntohs(ServAddr4.sin_port); 

			bRC = TRUE;
		}
		else if(SOCKET_ERROR == nBytesRcvd)
			 {
				int nError = plGetlastError ();
				bRC = FALSE;
				if(NULL != par_pszErrorMsg)
					sprintf (par_pszErrorMsg, PL_ERR_READ_DATA_SOCKET, nError);
			 }
			 else// --- nBytesRcvd is 0
				 {//--- Socket was closed.
					if(NULL != par_pszErrorMsg)
						sprintf (par_pszErrorMsg, PL_ERR_READ_DATA_SOCKET, 0);
					bRC = FALSE;
				 }
	 }//INCOM.Parameters
	 else
	 {
		 if(NULL != par_pszErrorMsg)
			 sprintf(par_pszErrorMsg, PL_ERR_RECVFROM_IN_PARAMS);
	 }

	 return bRC;
}
//==============================================================

//==============================================================
BOOL        plRecvFromv6( SOCKET			par_sock
						 , char			 *	par_pReceivedBuff
						 , int				par_nBuffSize
						 , int			 *	par_pnBytesRcvd
						 , char			 *	par_pszErrorMsg
						 , unsigned char *  par_pBuffIp //[IPV6_ADDRESS_BYTES_LEN+1]
						 , unsigned short*	par_pSourcePort)
{
	BOOL				bRC	= FALSE		;
	SOCKLEN_T			nTmp			;
	SOCKADDR_IN6        ServAddr6		;
	int				    nBytesRcvd = 0	;

	memset(&ServAddr6, 0, sizeof (ServAddr6));
	nTmp = sizeof(ServAddr6);

	if(  (NULL != par_pReceivedBuff)
		&&(0 < par_nBuffSize)
		&&(INVALID_SOCKET != par_sock)
		)
	{
		nBytesRcvd = recvfrom(par_sock , par_pReceivedBuff, par_nBuffSize, 0, (SOCKADDR *) & ServAddr6, & nTmp);

		if (nBytesRcvd > 0)
		{
			if(NULL != par_pnBytesRcvd)
				*par_pnBytesRcvd = nBytesRcvd;

			if(NULL != par_pBuffIp)
				memcpy(par_pBuffIp , &ServAddr6.sin6_addr, IPV6_ADDRESS_BYTES_LEN);

			if(NULL != par_pSourcePort)
				*par_pSourcePort = ntohs(ServAddr6.sin6_port); 

			bRC = TRUE;
		}
		else if(SOCKET_ERROR == nBytesRcvd)
		{
			int nError = plGetlastError ();
			bRC = FALSE;
			if(NULL != par_pszErrorMsg)
				sprintf (par_pszErrorMsg, PL_ERR_READ_DATA_SOCKET, nError);
		}
		else// --- nBytesRcvd is 0
		{//--- Socket was closed.
			if(NULL != par_pszErrorMsg)
				sprintf (par_pszErrorMsg, PL_ERR_READ_DATA_SOCKET, 0);
			bRC = FALSE;
		}
	}//INCOM.Parameters
	else
	{
		if(NULL != par_pszErrorMsg)
			sprintf(par_pszErrorMsg, PL_ERR_RECVFROM_IN_PARAMS);
	}

	return bRC;
}
//==============================================================


//==============================================================
char*       plcmIpToStringV4(unsigned int dwIP, char *pszIP, int par_nOutLen)
{
	if((NULL != pszIP)&&(par_nOutLen > 2))
	{
		struct in_addr 	inAddr;
		inAddr.s_addr  = dwIP ;
		strncpy (pszIP, inet_ntoa (inAddr), par_nOutLen);
	}
	return pszIP;
}
//==============================================================
//==============================================================
char *		plcmIpToStringV6(unsigned char * ipAddress
							,char		  *  par_szOutIpAddress
							,int             par_nOutLen
							,BOOL		     addBrackets)
{
	if((NULL != ipAddress)&&(NULL != par_szOutIpAddress)&&(par_nOutLen > 2))
	{
		struct sockaddr_in6 sockAddrV6;
		char				*pIpAddress = par_szOutIpAddress;

		if (addBrackets){
			par_szOutIpAddress[0] = '[';
			pIpAddress++;
		}

		memcpy(
			&sockAddrV6.sin6_addr.s6_addr,
			ipAddress,
			sizeof(sockAddrV6.sin6_addr.s6_addr));

		inet_ntop(AF_INET6, &sockAddrV6.sin6_addr, pIpAddress, INET6_ADDRSTRLEN);

		if (addBrackets)
			strncat(par_szOutIpAddress, "]", par_nOutLen - strlen(par_szOutIpAddress));
	}
	return par_szOutIpAddress;
}//plcmIpToStringV6
//================================================================================//
//================================================================================//
int plcmSockListReadSelectTimeOut(SOCKET * par_pSocketList, EVENT_INFO * par_pEventList, int par_nListLen
								, unsigned int  par_dwTimeoutMilliSec)
{
    int             nRC = FD_SELECT_TIMEOUT;
    fd_set          setRead, setExcept;
    struct timeval  tv;
    SOCKET          sockMax = 0;
	int             nSc     = 0;

	SOCKET          aRealSocketList[DNS_MAX_SERVICES];
	int             aIndexOfEventAr[DNS_MAX_SERVICES];
    EVENT_INFO		aRealEventList [DNS_MAX_SERVICES];
	int             nReal = 0;

	for(nSc = 0; nSc < DNS_MAX_SERVICES; nSc++)
	{
		aRealSocketList[nSc] = INVALID_SOCKET;
        aRealEventList[nSc].eSelectEvent = FD_EVENT_TIMEOUT;
		aRealEventList[nSc].pDnsSocket   = NULL;
		aIndexOfEventAr[nSc]             = -1  ; 
	}

	for(nSc = 0; nSc < (min(DNS_MAX_SERVICES, par_nListLen)); nSc++)
	{
		if(  (INVALID_SOCKET != par_pSocketList[nSc])
           &&(0 != par_pSocketList[nSc])
		   &&(65535 != par_pSocketList[nSc])
		  )
		{
			aRealSocketList[nReal] = par_pSocketList[nSc];
			aRealEventList [nReal] = par_pEventList [nSc];
			aIndexOfEventAr[nReal] = nSc                 ;
			nReal++;
		}
	}

	if((NULL != par_pSocketList)&&(0 < par_nListLen) &&(NULL != par_pEventList))
	{
		int			nFd				= 0;
		FD_ZERO(&setRead);
		FD_ZERO(&setExcept);

		//for(nFd = 0; nFd < par_nListLen; nFd++)
		//{
		//	FD_SET(par_pSocketList[nFd], &setRead);
		//	FD_SET(par_pSocketList[nFd], &setExcept);
		//	if(par_pSocketList[nFd] > sockMax)
		//		sockMax = par_pSocketList[nFd]; 
		//}

		for(nFd = 0; nFd < nReal; nFd++)
		{
			FD_SET(aRealSocketList[nFd], &setRead);
			FD_SET(aRealSocketList[nFd], &setExcept);

			
			int lBlockValue = 0;
			int  nIoctlRc = ioctl (aRealSocketList[nFd], FIONBIO, &lBlockValue);
			//if(nIoctlRc)
			//	printf("\n\n\n=== int  nIoctlRc = ioctl [%d] FALSE\n\n", aRealSocketList[nFd]);

			if(aRealSocketList[nFd] > sockMax)
				sockMax = aRealSocketList[nFd]; 
		}

		tv.tv_sec = par_dwTimeoutMilliSec / 1000;
		tv.tv_usec = (par_dwTimeoutMilliSec % 1000) * 1000;
	    
		nRC = select (sockMax + 1, &setRead, NULL, &setExcept, & tv);

		if(nRC > 0)
		{ 
			nRC = FD_SELECT_EVENT;
			for(nFd = 0; nFd < /*par_nListLen*/nReal ; nFd++) 
			{
				//if(FD_ISSET(par_pSocketList[nFd], &setRead))
				if(FD_ISSET(aRealSocketList[nFd], &setRead))
				{//DATA arrived on SOCKET
					int nIndex = aIndexOfEventAr[nFd];
					if(-1 != nIndex)
						par_pEventList[nIndex].eSelectEvent = FD_EVENT_READ;
				}
				else if (FD_ISSET(par_pSocketList[nFd], &setExcept))
				{//SOCKET has ERROR
					int nIndex = aIndexOfEventAr[nFd];
					if(-1 != nIndex)
						par_pEventList[nIndex].eSelectEvent = FD_EVENT_EXCEPT;
				}
			}
		}
		else if(0 == nRC)
			nRC = FD_SELECT_TIMEOUT;
		else
			nRC = FD_SELECT_EXCEPT ;
	}
    return nRC;
}
//================================================================================//
//================================================================================//
//Return:  > 0: Socket can READ
//         = 0: Timeout
//         < 0: Error   
int  plcmSockReadSelectTimeOut(SOCKET par_Socket, unsigned int  par_dwTimeoutMilliSec)
{
    int             nRC = 0;
    fd_set          setRead, setExcept;
    struct timeval  tv;
    SOCKET          sockMax = 0;

	if(SOCKET_ERROR != par_Socket)
	{
		FD_ZERO(&setRead);
		FD_ZERO(&setExcept);
		FD_SET(par_Socket, &setRead);
		FD_SET(par_Socket, &setExcept);

		sockMax =  par_Socket;

		tv.tv_sec = par_dwTimeoutMilliSec / 1000;
		tv.tv_usec = (par_dwTimeoutMilliSec % 1000) * 1000;
	    
		nRC = select (sockMax + 1, &setRead, NULL, &setExcept, & tv);
		if (nRC > 0)
		{
			if (FD_ISSET(par_Socket, &setExcept))
			{
				nRC = -1;
			}
			//else;  //* some data is ready for reading
		}
	}
    return nRC;
}
//================================================================================//


char GaSzSocketState[7][64]=
{
 {"SOCKET_UNDEF      "}
,{"SOCKET_OPENED     "}
,{"SOCKET_BINDED     "}
,{"SOCKET_CLOSED     "}
,{"SOCKET_BIND_FAILED"}
,{"SOCKET_OPEN_FAILED"}

,{"SOCKET_LASTENUM   "}

};

//==============================================================
cDNS_SOCKET::cDNS_SOCKET()
{
	this->m_IpV    = enIpVersionMAX;
	this->m_Socket = INVALID_SOCKET;
	memset(this->m_szErrMsg, '\0', sizeof(this->m_szErrMsg));
	this->m_eSate = eDNS_SOCKET_UNDEF;
	this->m_dwSelectReadTimeoutMillSec = 10;
}
//==============================================================

//==============================================================
cDNS_SOCKET::cDNS_SOCKET(enIpVersion par_eIpV)
{
FTRACEINTO << "PLCM_DNS. cDNS_SOCKET. [Constructor 1] for :"  << ((eIpVersion4 == par_eIpV)? "IPv4":"IPv6");
	this->m_IpV    = par_eIpV;
	this->m_Socket = INVALID_SOCKET;
	memset(this->m_szErrMsg, '\0', sizeof(this->m_szErrMsg));
	this->m_eSate = eDNS_SOCKET_UNDEF;
	this->m_dwSelectReadTimeoutMillSec = 10;

	this->m_UDPSocket();
}
//==============================================================
//==============================================================
cDNS_SOCKET::cDNS_SOCKET(enIpVersion par_eIpV, unsigned short par_wPort)
{
	FTRACEINTO << "PLCM_DNS. cDNS_SOCKET. [Constructor 2] for :" << ((eIpVersion4 == par_eIpV)? "IPv4":"IPv6") << " | Port: " << par_wPort;

	this->m_IpV    = par_eIpV;
	this->m_Socket = INVALID_SOCKET;
	memset(this->m_szErrMsg, '\0', sizeof(this->m_szErrMsg));
	this->m_eSate = eDNS_SOCKET_UNDEF;
	this->m_dwSelectReadTimeoutMillSec = 10;
}
//==============================================================
//==============================================================
cDNS_SOCKET::~cDNS_SOCKET()
{
	if(INVALID_SOCKET != this->m_Socket)
		this->m_CloseUdpSocket();
	this->m_Socket = INVALID_SOCKET;
	this->m_IpV    = enIpVersionMAX;
	memset(this->m_szErrMsg, '\0', sizeof(this->m_szErrMsg));
}
//==============================================================
//==============================================================
BOOL cDNS_SOCKET::m_bSosketActivateA(enIpVersion par_eIpV, unsigned short par_wPort, char * par_szIP) 
{
	BOOL bRc = FALSE;
	if((eDNS_SOCKET_UNDEF == this->m_eSate)||(eDNS_SOCKET_CLOSED == this->m_eSate))
	{
		this->m_IpV   = par_eIpV;
		this->m_UDPSocket();
		if(eDNS_SOCKET_OPENED == this->m_eSate)
		{
			bRc = this->m_plBindA(par_wPort, par_szIP);
			if(FALSE == bRc)
			{
				TRACEINTO << "PLCM_DNS. m_bActivateA: SOCKET:" << this->m_Socket
						  << " | SignalingIP:" <<  par_szIP 
						  << " | nRc : " << ((FALSE == bRc)? "FALSE":"TRUE");

				this->m_CloseUdpSocket();
			}
		}
	}

	return bRc;
}
//==============================================================
//==============================================================
SOCKET cDNS_SOCKET::m_UDPSocket()
{
	SOCKET eRc = INVALID_SOCKET;
	if((eIpVersion4 == this->m_IpV)||(eIpVersion6 == this->m_IpV)) 
	{	
		this->m_Socket = eRc = plUDPSocket(this->m_IpV, this->m_szErrMsg);
	}

    if(INVALID_SOCKET != this->m_Socket)
		this->m_eSate = eDNS_SOCKET_OPENED;
	else
		this->m_eSate = eDNS_SOCKET_OPEN_FAILED;

	if(eDNS_SOCKET_OPEN_FAILED == this->m_eSate)
		FTRACEINTO << "PLCM_DNS. cDNS_SOCKET. [OPEN]. " 
				   << ((eIpVersion4 == this->m_IpV)? "| IPv4":"IPv6") 
				   << " | Socket: " << eRc 
				   << " | State: " << (this->m_szGetStrSocketState());

	return eRc;
}
//==============================================================
//==============================================================
void cDNS_SOCKET::m_CloseUdpSocket()
{
	if(this->m_Socket != INVALID_SOCKET)
		plCloseUdpSocket(this->m_Socket);
	this->m_Socket = INVALID_SOCKET;
	this->m_eSate = eDNS_SOCKET_CLOSED;

	FTRACEINTO << "PLCM_DNS. cDNS_SOCKET. [CLOSE]. Socket: " << this->m_Socket <<"State: " << (this->m_szGetStrSocketState());
}
//==============================================================
//==============================================================
BOOL cDNS_SOCKET::m_plBindA(unsigned short par_wPort, char * par_szIP)
{
	BOOL  bRc = FALSE;

	bRc = plBindA(this->m_Socket, par_wPort, this->m_IpV, this->m_szErrMsg, par_szIP);

	if(TRUE == bRc)
		this->m_eSate = eDNS_SOCKET_BINDED;
	else
		this->m_eSate = eDNS_SOCKET_BIND_FAILED;

	if(FALSE == bRc)
		FTRACEINTO << "PLCM_DNS. cDNS_SOCKET. [BIND]. Socket: " << this->m_Socket 
				   << " | IP:" << (((NULL == par_szIP)||(0 == strlen(par_szIP)) )? "ANY_ADDR": par_szIP)
				   << " | Port: " <<  par_wPort << " | State: " <<(this->m_szGetStrSocketState());
	return bRc;
}
//==============================================================
//==============================================================
char * cDNS_SOCKET::m_szGetStrSocketState()
{
	if(  (eDNS_SOCKET_UNDEF   <= this->m_eSate) 
	   &&(eDNS_SOCKET_LASTENUM > this->m_eSate)
	  )
		return GaSzSocketState[this->m_eSate];
	else
		return GaSzSocketState[eDNS_SOCKET_LASTENUM];
}
//==============================================================
//==============================================================
BOOL cDNS_SOCKET::m_SendTo(char * par_pszIP , unsigned short par_wPort,char * par_pData, int par_nDataSize)
{
	BOOL bRc = FALSE;
	bRc = plSendTo(this->m_Socket, par_pszIP , par_wPort, par_pData, par_nDataSize, this->m_szErrMsg);
	return bRc;
}
//==============================================================
//==============================================================
int cDNS_SOCKET::m_RecvFrom4(char * par_pData, int par_nSize
						   , char * par_SourceIP
						   , int    par_nSourceIPLen
						   , unsigned short * par_pSourcePort)
{
	int				nRc		    = -1;
	unsigned int	dwIp	    =  0;
	int				nRcSelect   = -1;
	int				nBytesRcvd  =  0;

	if(0 < (nRcSelect = plcmSockReadSelectTimeOut(this->m_Socket, this->m_dwSelectReadTimeoutMillSec)) )
	{
		if(TRUE == plRecvFromv4( this->m_Socket, par_pData, par_nSize
						  , &nBytesRcvd, this->m_szErrMsg
						  , &dwIp, par_pSourcePort)
		  )
		{
			plcmIpToStringV4(ntohl(dwIp), par_SourceIP, par_nSourceIPLen);
			nRc = nBytesRcvd;
		}
	}
	if(0 == nRcSelect)//TIMEOUT
	{
		nRc = 0;
	}

	return nRc;
}
//==============================================================
//==============================================================
int cDNS_SOCKET::m_RecvFrom4A(char * par_pData, int par_nSize
							 , char * par_SourceIP
							 , int    par_nSourceIPLen
							 , unsigned short * par_pSourcePort)
{
	int				nRc		    = -1;
	unsigned int	dwIp	    =  0;
	int				nBytesRcvd  =  0;

	if(TRUE == plRecvFromv4( this->m_Socket, par_pData, par_nSize
		, &nBytesRcvd, this->m_szErrMsg
		, &dwIp, par_pSourcePort)
		)
	{
		plcmIpToStringV4(ntohl(dwIp), par_SourceIP, par_nSourceIPLen);
		nRc = nBytesRcvd;
	}

	return nRc;
}
//==============================================================
//==============================================================
int cDNS_SOCKET::m_RecvFrom6(  char * par_pData, int par_nSize
							 , char * par_SourceIP
							 , int    par_nSourceIPLen
							 , unsigned short * par_pSourcePort)
{
	int				nRc		    = -1;
	unsigned char   BuffIp[IPV6_ADDRESS_BYTES_LEN+1];
	int				nRcSelect   = -1;
	int				nBytesRcvd  =  0;

    memset(BuffIp, 0, sizeof(BuffIp));

	if(0 < (nRcSelect = plcmSockReadSelectTimeOut(this->m_Socket, this->m_dwSelectReadTimeoutMillSec)) )
	{
		if(TRUE == plRecvFromv6( this->m_Socket, par_pData, par_nSize
			, &nBytesRcvd, this->m_szErrMsg
			, BuffIp, par_pSourcePort)
			)
		{
			plcmIpToStringV6(BuffIp, par_SourceIP, par_nSourceIPLen, FALSE);
			nRc = nBytesRcvd;
		}
	}
	if(0 == nRcSelect)//TIMEOUT
	{
		nRc = 0;
	}

	return nRc;
}
//==============================================================
//==============================================================
int cDNS_SOCKET::m_RecvFrom6A( char * par_pData, int par_nSize
							 , char * par_SourceIP
							 , int    par_nSourceIPLen
							 , unsigned short * par_pSourcePort)
{
	int				nRc		    = 0;
	unsigned char   BuffIp[IPV6_ADDRESS_BYTES_LEN+1];
	int				nBytesRcvd  =  0;

	memset(BuffIp, 0, sizeof(BuffIp));

	if(TRUE == plRecvFromv6( this->m_Socket, par_pData, par_nSize
		, &nBytesRcvd, this->m_szErrMsg
		, BuffIp, par_pSourcePort)
		)
	{
		plcmIpToStringV6(BuffIp, par_SourceIP, par_nSourceIPLen, FALSE);
		nRc = nBytesRcvd;
	}

	return nRc;
}
//==============================================================




