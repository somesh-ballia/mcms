 /*====================================================================*/
/*            Copyright     ???    2001  Polycom Israel, Ltd. All rights reserved                                       */
/*-------------------------------------------------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary   information of                */
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form   */
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without          */
/* prior written authorization from Polycom Israel Ltd.                                                                    */
/*-------------------------------------------------------------------------------------------------------------------*/
/* FILE:     SocketApiWrap.c                                                                                                                        */
/* PROJECT:  Gideon                                                                                                   */
/* PROGRAMMER:  Tzvicka Reznick                                              .                                                  */
/* FUNCTION LIST:                                                                                                                         */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                                                                                         */
/*-------------------------------------------------------------------------------------------------------------------*/
/*  t.r   |   21/03/05      |    create file                                                                                                                */
/*====================================================================*/




/** include files **/
#include  <stdio.h>
#include  <string.h>
#include  <sys/types.h>
#include  <arpa/inet.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <string.h>
#include  <pthread.h>
#include  <unistd.h>
#include <sys/ioctl.h>
#include <net/route.h>
#include <sys/wait.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <poll.h>
#include "SocketApiWrapExt.h"
#include "LinuxSystemCallsApi.h"
#include "McmsApi.h"
#include "SocketApiWrap.h"
#include "Print.h"
#include "timers.h"
#include "Diagnostics.h"

extern UINT32 TimerResetInterval(UINT32 ulJobID, UINT32 ulInterval);

/* global variables */

//#define NUM_OF_NI 8
//static UINT32 aulClientId[NUM_OF_NI] = {110,111,112,113,114,115,116,117} ;
//SOCKET l_ClientSocket;
SOCKET nLoggerSocket;
// UINT32 ul_ConnectionStatus = NOT_CONNECTED;
UINT32 unPrintLoggerConnectionStatus = NOT_CONNECTED;
extern UINT32	tpcktBugOverrideLen;


t_TcpConnParams TcpConnection[eMaxTcpConnections];

extern fd_set *ptTcpActiveReadSockets;

extern IF_NAME_STRUCT tIfNameStruct;

UINT32 IpV6JobId = 0;
UINT32 IsThereIPV6 = 0;

#define SIZE_OF_COMMAND_LINE 100

INT32 GetV4NiParams(UINT8 * puc_Ifname, Ni_Params *pt_V4NiAddress)
{
	int sock ;
	struct ifreq tIfReqParams ;
	struct sockaddr_in *tpAddr ;

	sock = socket(AF_INET ,SOCK_DGRAM,0) ;
	if(sock < 0)
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetV4NiParams : socket creation failed: error %d\n",GetLastError());
		return (-1);

	}

	/* Set Interface name */
	memcpy(&tIfReqParams.ifr_name[0] ,puc_Ifname,IFNAMSIZ) ;

	if(ioctl(sock,SIOCGIFADDR,&tIfReqParams) < 0 )
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetV4NiParams : ioctl SIOCGIFADDR operation failed: error %d\n",GetLastError());
		return (-1);
	}
	else
	{
		tpAddr = (struct sockaddr_in *)(&tIfReqParams.ifr_addr) ;
		pt_V4NiAddress->ulIpAddr = tpAddr->sin_addr.s_addr ;

	}


	if(ioctl(sock,SIOCGIFNETMASK,&tIfReqParams) < 0 )
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetV4NiParams : ioctl SIOCGIFNETMASK operation failed: error %d\n",GetLastError());
		return (-1);
	}
	else
	{
		tpAddr = (struct sockaddr_in *)(&tIfReqParams.ifr_addr) ;
		pt_V4NiAddress->ulSubNetMask = tpAddr->sin_addr.s_addr ;

	}


	if(ioctl(sock,SIOCGIFBRDADDR,&tIfReqParams) < 0 )
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"GetV4NiParams : ioctl SIOCGIFBRDADDR operation failed: error %d\n",GetLastError());
		return (-1);
	}
	else
	{
		tpAddr = (struct sockaddr_in *)(&tIfReqParams.ifr_broadaddr) ;
		pt_V4NiAddress->ulDefGateWay = tpAddr->sin_addr.s_addr ;
	}

	close(sock);

	return 0 ;


}

SOCKET InitV4Connection()
{
    INT32 l_Rc;
	UINT32 ul_Optval = 0xffff; /* 64k bytes */
	SOCKET l_Socket;
	int val=1;

	linger 	my_linger;

	my_linger.l_onoff = 1;
	my_linger.l_linger = 0;

	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitV4Connection");

    /* open v4 TCP socket */
	l_Socket = socket( AF_INET, SOCK_STREAM, 0 );
	if( l_Socket < 0 )
	{
        MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitV4Connection : socket request failed: error %d\n",GetLastError());
		return (SOCKET_OPERATION_FAILED);
	}



   // l_Rc  = setsockopt( l_Socket,SOL_SOCKET,SO_LINGER | TCP_NODELAY ,(char*)&my_linger,sizeof(linger)) ;

	return (l_Socket);
}


INT32 TCPRecvData( SOCKET l_Socket, VOID **vp_RcvBuffer,INT32 l_flags)
{
	UINT8                uc_TpktBytesReceived, uc_version,uc_reserved = 0;
	UINT8				 *cp_RcvBuffer;
	INT32				 l_Rc = 0,l_Err,i;
	INT32				 l_BytesReceived =0 ,l_TotalBytesReceived = 0,l_TotalBytesLeft= 0;
	UINT32               ul_RcvBufferLen;
	BOOL                 b_RcvDone;


	int tpcktBugOccured = 0;	//pavelk fix


	int rv;

	struct pollfd ufds[1];



	ufds[0].fd = l_Socket;
	ufds[0].events = POLLIN; // check for normal or out-of-band


//  #ifdef X86_SIM
    t_LETpktType           t_Tpkt;
//  #else
//  	t_TpktType             t_Tpkt;
//  #endif

	UINT8 uc_Reserved, uc_Version;

    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"TCPRecvData");

	*vp_RcvBuffer = NULL;
    /* check errors in function parameters */
    if( l_Socket < 0 )
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TCPRecvData: socket is not valid %d",l_Socket);
	  	return (SOCKET_OPERATION_FAILED);
	}
//	if (*vp_RcvBuffer == NULL)
//	{
    //MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"RecvData: receive buffer is null") ;
//	}

/*********************************************************************************************/
/*           Receive the data through the socket to a buffer                                 */
/*********************************************************************************************/
  /* TPKT handling - we use tpkt format but not on port 102 but on the port we comunicate with MP-H */

	uc_TpktBytesReceived=0;

/*	while(uc_TpktBytesReceived < 4)
	{		 // receive data
*/
	rv = poll(ufds, 1, 1000);

	if (rv == -1)
	{
		return EMB_FAIL; // error occurred in poll()
	}
	else if (rv == 0)
	{
		//printf("Timeout occurred!  No data after 1 seconds.\n");
	}
	else
	{
	// check for events on s1:
		if (ufds[0].revents & POLLHUP)
		{
			printf("Yosi - rcv thread POLLHUP error on socket.\n");
			return SOCKET_OPERATION_FAILED;
		}
		if (ufds[0].revents & POLLERR)
		{
			printf("Yosi - rcv thread POLLERR error on socket.\n");
			return SOCKET_OPERATION_FAILED;
		}
		if (ufds[0].revents & POLLIN)
		{
			l_Rc = recv( l_Socket, &t_Tpkt, (TPKT_SIZE - uc_TpktBytesReceived), l_flags) ;
		}
		else
		{
			printf("Yosi - rcv thread POLLOUT error on socket.\n");
			return SOCKET_OPERATION_FAILED;
		}
	}






	if(l_Rc == 0)
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"TCPRecvData: recv - l_Rc = %d",l_Rc);
		return (NO_DATA);
	}
	else
	{
		if(l_Rc < 0)
		{			/* get error information */
			l_Err = GetLastError();
			MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TCPRecvData : Recv failed: error %d",errno);
			/* error handling  - should be a function later on*/
			if ((l_Err <109 ) && (l_Err > 99))
			{
				//ul_ConnectionStatus = NOT_CONNECTED;
			}
			return (SOCKET_OPERATION_FAILED);
		}
		else
		{
		   MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"TCPRecvData : tpkt Received %d bytes of data",l_Rc);
		//   printf("TCPRecvData : tpkt Received %d bytes of data",l_Rc);
		   EmbSleep(1);
		}
		/* count bytes received	*/
		uc_TpktBytesReceived +=(UINT8)l_Rc;
	}
//	}//while(uc_TpktBytesReceived < 4)
	/* parse tpkt data */

	    /* get size of transmited data */
	ul_RcvBufferLen = t_Tpkt.us_Length - TPKT_SIZE;

    MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"TCPRecvData: expecting %d bytes of DATA",ul_RcvBufferLen);

    /* get reserved	*/
   	uc_Reserved = (UINT8)t_Tpkt.uc_Reserved;
	/* get version 	*/
    uc_Version = (UINT8)t_Tpkt.uc_Version;

    if ((uc_Reserved != 0) || (uc_Version != 3))
    {
    	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TCPRecvData: corrupted tpkt : Version: %d reserved: %d Len: %d",uc_Version,uc_reserved,ul_RcvBufferLen);
    //	printf("TCPRecvData: corrupted tpkt : Version: %d reserved: %d Len: %d",uc_Version,uc_reserved,ul_RcvBufferLen);
    	printf("---------TPKT CORRUPTED!!---------\n");

    	printf("expected:  uc_Version,  uc_reserved, usLength\n ");
    	printf("received: %x %x %x\n",uc_Version,uc_Reserved,ul_RcvBufferLen);


    	//purge the socket
    //	*vp_RcvBuffer = malloc(tpcktBugOverrideLen);
    //	cp_RcvBuffer = *vp_RcvBuffer;
     //   l_BytesReceived = recv( l_Socket, cp_RcvBuffer, tpcktBugOverrideLen, MSG_WAITALL) ;
   // 	printf("\nfirst 30bytes of message:\n");
   // 	for (i = 0 ;i < 30 && i < tpcktBugOverrideLen ; i++)
   // 		printf("%x",cp_RcvBuffer[i]);
    //    printf("\n-----------------------\n");
    //	free(cp_RcvBuffer);
	EmbSleep(1000);
    	return(SOCKET_OPERATION_FAILED);	//always report as failed receipt
    }



	/* init receive params */
	*vp_RcvBuffer = malloc(ul_RcvBufferLen);
	cp_RcvBuffer = *vp_RcvBuffer;
	if (cp_RcvBuffer == NULL)
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TCPRecvData: allocation fault");
		//printf("TCPRecvData: allocation fault");
        return(SOCKET_OPERATION_FAILED);
	}
	memset(cp_RcvBuffer,0,ul_RcvBufferLen);

	b_RcvDone = EMB_FALSE;
	l_TotalBytesLeft = 	ul_RcvBufferLen;

	/* start receiving the stream */
	while(b_RcvDone == EMB_FALSE)
	{
	    l_BytesReceived = recv( l_Socket, cp_RcvBuffer, l_TotalBytesLeft, MSG_WAITALL) ;
		l_TotalBytesReceived += l_BytesReceived;
		l_TotalBytesLeft -= l_BytesReceived;

		if(l_BytesReceived == 0)
		{
			/* no data received */
		    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"TCPRecvData: error total bytes received %d expected %d",l_TotalBytesReceived,ul_RcvBufferLen);
		 //   printf("TCPRecvData: error total bytes received %d expected %d",l_TotalBytesReceived,ul_RcvBufferLen);
            return (SOCKET_OPERATION_FAILED);
		}
		else
		{

			if(l_BytesReceived < 0)
			{
				/*  report error encountered */
	    		l_Err = GetLastError();
	            MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"TCPRecvData: received %d total bytes received %d, expected %d",l_BytesReceived,l_TotalBytesReceived, ul_RcvBufferLen);
	          //  printf("TCPRecvData: received %d total bytes received %d, expected %d",l_BytesReceived,l_TotalBytesReceived, ul_RcvBufferLen);
	            /* error handling  - should be a function later on*/
				if ((l_Err <109 ) && (l_Err > 99))
				{
					//ul_ConnectionStatus = NOT_CONNECTED;
				}

				return (SOCKET_OPERATION_FAILED);
			}
			else  /* data received */
			{
				MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"TCPRecvData: total bytes received %d expected %d",l_TotalBytesReceived,ul_RcvBufferLen);
				//printf("TCPRecvData: total bytes received %d expected %d",l_TotalBytesReceived,ul_RcvBufferLen);
				EmbSleep(1);
			}

			if (l_TotalBytesLeft == 0)
			   b_RcvDone = EMB_TRUE ;

			cp_RcvBuffer += l_BytesReceived;
		}
	}

	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"TCPRecvData:got all message. returning size %d", l_TotalBytesReceived);

	if (ul_RcvBufferLen != (UINT32)l_TotalBytesReceived)
		printf("instead of %u received %d bytes\n",ul_RcvBufferLen,l_TotalBytesReceived);

	return (l_TotalBytesReceived) ;
}


/****************************************************************************
* Prototype:        sendWrap
* Description:      send() command wrapper
* Return Value:     EMB_FAIL if an error occurred
* 					EMB_OK if succesfully transmitted
* Arguments:     	l_Socket     - Socket descriptor no.
*                   vp_RcvBuffer - sending buffer
*                   us_Length    - Size of message to send
*  					// D.H. Code taken from http://beej.us/guide/bgnet/output/html/multipage/advanced.html
*****************************************************************************/
INT32 sendWrap( SOCKET l_Socket, UINT8* vp_RcvBuffer, UINT16 us_Length,INT32 l_flags , UINT32 unPrintFlag)
{
	INT32 unRetCode    	= EMB_FAIL;
	INT16 unActualSent	= EMB_FAIL;
	INT32 unSendRetry	= 0;

	int total = 0;       	   // how many bytes we've sent
    int bytesleft = us_Length; // how many we have left to send


	int rv;

	struct pollfd ufds[1];



	ufds[0].fd = l_Socket;
	ufds[0].events = POLLOUT; // check for normal or out-of-band




    while((total < us_Length) && (unSendRetry < SEND_WRAP_RETRIES))
	{
		// wait for events on the sockets, 3.5 second timeout
		rv = poll(ufds, 1, 1000);

		if (rv == -1)
		{
			return EMB_FAIL; // error occurred in poll()
		}
		else if (rv == 0)
		{
			printf("Timeout occurred!  No data after 1 seconds.\n");
		}
		else
		{
		// check for events on s1:
			if (ufds[0].revents & POLLHUP)
			{
				printf("Yosi - POLLHUP error on socket.\n");
				return EMB_FAIL;
			}
			if (ufds[0].revents & POLLERR)
			{
				printf("Yosi - POLLERR error on socket.\n");
				return EMB_FAIL;
			}
			if (ufds[0].revents & POLLOUT)
			{
				unActualSent = send(l_Socket, vp_RcvBuffer+total, bytesleft, 0);
			}
			else
			{
				printf("Yosi - POLLOUT error on socket.\n");
				return EMB_FAIL;
			}
		}


        if (unActualSent == EMB_FAIL)
		{
				break;
		}
		if (unActualSent > 0)
		{
			total += unActualSent;
			bytesleft -= unActualSent;
			unSendRetry = 0;
		}
		else
		{
			unSendRetry++;
		}
    }

	if (unActualSent == EMB_FAIL || unSendRetry == SEND_WRAP_RETRIES)
	{
		unRetCode = EMB_FAIL;
	}
	else
	{
		unRetCode = EMB_OK;
	}
    return unRetCode; // return -1 on failure, 0 on success
}

INT32 TCPSendData( SOCKET l_Socket, VOID *vp_RcvBuffer, UINT16 us_Length,INT32 l_flags , UINT32 unPrintFlag)
{
	INT32 l_Rc;
	INT32 l_ErrorNum;
	UINT32 ul_MsgLength;
	UINT32 unLoggerHeaderSize = 0;
	INT8  *pTmp = NULL;
    t_LETpktType           t_Tpkt;
	struct sockaddr_in t_SockAddr;

   if (l_Socket == TcpConnection[eSwitchDiagClient].s)	//switch server
    {
    	sendMsgFromDiagToEma(us_Length,vp_RcvBuffer);
    	return 1;
    }
   
  /* if (l_Socket == TcpConnection[eIpmiClient].s)	//switch server
    {
    	sendMsgFromIpmiToEma(eIpmiServer, us_Length,vp_RcvBuffer);
    	return 1;
    }*/
   
	memset(&t_Tpkt,0,sizeof(t_LETpktType));
	if( l_Socket < 0 )
		return (SOCKET_OPERATION_FAILED);

	/* init and send tpkt first	 */
    t_Tpkt.us_Length = (us_Length + sizeof(t_LETpktType));
    t_Tpkt.uc_Version = 0x3;
    t_Tpkt.uc_Reserved = 0;



    //l_Rc = send( l_Socket, (UINT8 *)&t_Tpkt, sizeof(t_TpktType),l_flags );
	l_Rc = sendWrap( l_Socket, (UINT8*) &t_Tpkt, sizeof(t_TpktType),l_flags, unPrintFlag ); //[Dotan.H 02/08/2009 Change from syscall send to wrapper]

    if (l_Rc < 0)
    {
   		printf("TCPSendData: error sending tpkt on socket %d !!! (error:%d). - Logger\n",l_Socket,GetLastError());

		/* Add error handling */
	 	return (SOCKET_OPERATION_FAILED);
   	}
   	else
   	{
//       	printf("tpkt sent");
   	}

	if (us_Length > 30000)	//split up the packet if it is too large:
	{
		l_Rc = sendWrap( l_Socket, (UINT8 *)vp_RcvBuffer, 30000,l_flags, unPrintFlag); //[Dotan.H 02/08/2009 Change from syscall send to wrapper]
		if (l_Rc < 0)
    		{
   		l_ErrorNum =  GetLastError();

   		printf("TCPSendData: error sending data on socket %d !!! (error:%d).  - Logger \n",l_Socket,l_ErrorNum);

   	 	/* Add error handling */

		/* network down errors */
		/*
		   108 ESHUTDOWN     Cannot send after transport endpoint shutdown
		   107 ENOTCONN      Transport endpoint is not connected
		   106 EISCONN       Transport endpoint is already connected
		   105 ENOBUFS       No buffer space available
		   104 ECONNRESET    Connection reset by peer
		   103 ECONNABORTED  Software caused connection abort
		   102 ENETRESET     Network dropped connection on reset
		   101 ENETUNREACH   Network is unreachable
		   100 ENETDOWN      Network is down
		*/

		    return (SOCKET_OPERATION_FAILED);
		}
	        else
  	       {
			printf("First chunk of large data , 30000bytes sent\n");
	        }	
		
		l_Rc = sendWrap( l_Socket, (UINT8 *)vp_RcvBuffer + 30000, us_Length-30000,l_flags, unPrintFlag); //[Dotan.H 02/08/2009 Change from syscall send to wrapper]
		if (l_Rc < 0)
    		{
   		l_ErrorNum =  GetLastError();

   		printf("TCPSendData: error sending data on socket %d !!! (error:%d).  - Logger \n",l_Socket,l_ErrorNum);

   	 	/* Add error handling */

		/* network down errors */
		/*
		   108 ESHUTDOWN     Cannot send after transport endpoint shutdown
		   107 ENOTCONN      Transport endpoint is not connected
		   106 EISCONN       Transport endpoint is already connected
		   105 ENOBUFS       No buffer space available
		   104 ECONNRESET    Connection reset by peer
		   103 ECONNABORTED  Software caused connection abort
		   102 ENETRESET     Network dropped connection on reset
		   101 ENETUNREACH   Network is unreachable
		   100 ENETDOWN      Network is down
		*/

		    return (SOCKET_OPERATION_FAILED);
		}
	        else
  	       {
			printf("Second-last chunk of large data sent (size-%d)\n",us_Length-30000);
	        }	
	}
	else
	{

		    /* if tpkt ok continue to send data */
		    l_Rc = sendWrap( l_Socket, (UINT8 *)vp_RcvBuffer, us_Length,l_flags, unPrintFlag); //[Dotan.H 02/08/2009 Change from syscall send to wrapper]

		    if (l_Rc < 0)
		    {
	   		l_ErrorNum =  GetLastError();

   			printf("TCPSendData: error sending data on socket %d !!! (error:%d).  - Logger \n",l_Socket,l_ErrorNum);

   	 	/* Add error handling */

		/* network down errors */
		/*
		   108 ESHUTDOWN     Cannot send after transport endpoint shutdown
		   107 ENOTCONN      Transport endpoint is not connected
		   106 EISCONN       Transport endpoint is already connected
		   105 ENOBUFS       No buffer space available
		   104 ECONNRESET    Connection reset by peer
		   103 ECONNABORTED  Software caused connection abort
		   102 ENETRESET     Network dropped connection on reset
		   101 ENETUNREACH   Network is unreachable
		   100 ENETDOWN      Network is down
		*/

			    return (SOCKET_OPERATION_FAILED);
		   }
		   else
		   {
				printf("Data sent");
		   }
	}
   return (SOCKET_OPERATION_OK);
}


INT32 CloseConnection( SOCKET l_Socket, INT32 l_How )
{
	INT32 l_Err;

    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CloseConnection");

	if( l_Socket < 0 )
		return (SOCKET_OPERATION_FAILED);
    // Disable both sending and receiving on ClientSock.
	l_Err = shutdown (l_Socket, l_How);
	if (l_Err == -1)
	{
      MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CloseConnection: error shuting down socket - %d  ",l_Err);
	  return (SOCKET_OPERATION_FAILED);
	}

	return(0);

}


INT32 SendV4ConnectReq(SOCKET s , UINT16 us_RemotePort,t_IpV4Address t_DestIpV4Addr,e_TcpConn eId)
{
	struct sockaddr_in t_DestSIn;
	INT32 err ;
	t_TcpConnParams *TcpParams;

	TcpParams =  &(TcpConnection[eId]);
//	UINT8   *p_IpAddr;

//	ip=RemoteIPAddress;
	memset( &t_DestSIn, 0, sizeof(t_DestSIn) );
	t_DestSIn.sin_family = AF_INET;
	t_DestSIn.sin_port = htons(us_RemotePort);
/*
#ifdef 	X86_SIM
	t_DestSIn.sin_port = htons(us_RemotePort);
#else
  #ifdef x86_ARCH
  	t_DestSIn.sin_port = htons(us_RemotePort);
  #else
  	t_DestSIn.sin_port = us_RemotePort;
  #endif

#endif
*/
 	t_DestSIn.sin_addr.s_addr = inet_addr(t_DestIpV4Addr.auc_IpV4Address);


// 	if (TcpConnection->ul_PrevConnState == CONNECTED)
 	if (TcpParams->ul_PrevConnState == CONNECTED) 	// Benson Fix
 	{
 		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"SendV4ConnectReq port: %d,Socket %d ip %s",us_RemotePort,s,t_DestIpV4Addr.auc_IpV4Address);
 	}
	// Establish a connection to the server socket.
	if (connect (s, (const struct sockaddr *) &t_DestSIn, sizeof (t_DestSIn)) == SOCKET_OPERATION_FAILED)
	{
		err = GetLastError();

//		if (TcpConnection->ul_PrevConnState == CONNECTED)
		if (TcpParams->ul_PrevConnState == CONNECTED)  // Benson Fix
 	    {
        	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"SendV4ConnectReq: Error connecting %d\n", err);
 	    }
		return SOCKET_OPERATION_FAILED;
	}

    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"SendV4ConnectReq: Ack received.");

	return (0);
}


INT32 ConnectV4Connection(SOCKET s , UINT16 us_RemotePort,t_IpV4Address t_DestIpV4Addr,e_TcpConn eId)
{
	UINT32 ul_Counter = 0;
	INT32 l_Rc;
	t_TcpConnParams *TcpParams;

	TcpParams =  &(TcpConnection[eId]);

	while (TcpParams->ul_ConnectionStatus != CONNECTED)
	{
	    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"ConnectV4Connection ConnectionStatus : %d",TcpParams->ul_ConnectionStatus);
	    l_Rc = SendV4ConnectReq(s , us_RemotePort, t_DestIpV4Addr,eId);

		if (l_Rc != SOCKET_OPERATION_FAILED)
		{
			MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ConnectV4Connection : nClient and nServer are connected");

			TcpParams->ul_ConnectionStatus = CONNECTED;
//			TcpConnection[eMcms].ul_PrevConnState = CONNECTED;
			TcpParams->ul_PrevConnState = CONNECTED; // Benson Fix

		  	ul_Counter = 0;
		}
		else
		{
			 ul_Counter++;
	         EmbSleep(2000) ;
		     if (ul_Counter > 0xfffffff)
		     {
		     	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ConnectV4Connection:  could not establish connection between nClient and nServer, retrying....");
			 }
		}
		// t.r patch
		if (eId == eLoggerCom)
			break;
	}

    return 0;
}
// t.r - check connection status and reconnect if needed - called by timer task


void ReConnectV4Connection(SOCKET s , UINT16 us_RemotePort,t_IpV4Address t_DestIpV4Addr,e_TcpConn eId)
{
	INT32 l_Rc;
	void *pul_Msg;
	TMessageThreadType tMessageThreadType;
	memset(&tMessageThreadType,0,sizeof(TMessageThreadType));
	t_TcpConnParams *TcpParams;

	TcpParams =  &(TcpConnection[eId]);

	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"ReConnectV4Connection....eId %d", eId);
	if ( TcpParams->ul_ConnectionStatus != CONNECTED )
	{
		l_Rc = SendV4ConnectReq(s , us_RemotePort, t_DestIpV4Addr,eId);

	    if (l_Rc == SOCKET_OPERATION_FAILED)
	    {
	    	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ReConnectV4Connection:  could not establish connection between client and server, retrying....");
			TcpParams->ul_PrevConnState = NOT_CONNECTED; // Benson Fix
			// send reconnection massage to MPH here
	    }
	    else
	    {
	    	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"ReConnectV4Connection : client and server are connected");
	        TcpParams->ul_ConnectionStatus = CONNECTED;
	        TcpParams->ul_PrevConnState = CONNECTED; // Benson Fix
	   }
	}
}



void CheckClientConnStatus(void *p)
{
	t_TcpConnParams *TcpParams;
	INT32 rc = 0;

	TcpParams =   (t_TcpConnParams *)p;
	//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CheckClientConnStatus : eId = %d",TcpParams->e_Id);

	if (TcpParams->ul_ConnectionStatus != CONNECTED)
	{
     	if (TcpParams->s >= 0)
		{
			if (TcpParams->e_Id != eLoggerCom)
				FD_CLR(TcpParams->s,ptTcpActiveReadSockets); // Benson

        	rc = close(TcpParams->s);

		}

        if (TcpParams->ul_PrevConnState == CONNECTED) // Benson Fix
        {
          MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CheckClientConnStatus : closing socket %d returnd with %d",TcpParams->s,rc);
        }

		//reconnect:
        TcpParams->s = InitV4Connection();

    	if (TcpParams->ul_PrevConnState == CONNECTED) // Benson Fix
     	{
     	    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CheckClientConnStatus : new socket %d",TcpParams->s);
     	}

     	if (TcpParams->s >= 0)
     	{
     		if (TcpParams->ul_PrevConnState == CONNECTED) // Benson Fix
     	   	{
     	   		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"CheckClientConnStatus : TcpParams: address = 0x%x TcpConnection[eId] address 0x%x, socket %d, connection status %x",TcpParams,&(TcpConnection[TcpParams->e_Id]),TcpParams->s, TcpParams->ul_ConnectionStatus);
           	}

           	ReConnectV4Connection(TcpParams->s,TcpParams->us_Port, TcpParams->IpV4Addr, TcpParams->e_Id );

           	if ((TcpParams->ul_ConnectionStatus == CONNECTED)&&(TcpParams->e_Id != eLoggerCom))
           		FD_SET(TcpParams->s,ptTcpActiveReadSockets);

           	// send reconnect massage to Mcms
           	if ((TcpParams->ul_ConnectionStatus == CONNECTED) &&	(TcpParams->e_Id == eMcmsCom))
           	{
				//ReconnectToMcms(TcpParams->s);
           	}//if ((TcpParams->ul_ConnectionStatus == CONNECTED) &&	(TcpParams->e_Id == eMcmsCom))
     	}//if (TcpParams->s >= 0)
	}//if (TcpParams->ul_ConnectionStatus != CONNECTED)
}

void CheckServerConnStatus(void *p)
{
	t_TcpConnParams *pTcpParams;
	INT32 rc;
	void *pul_Msg;
	TMessageThreadType tMessageThreadType;
    memset(&tMessageThreadType,0,sizeof(TMessageThreadType));
	pTcpParams =   (t_TcpConnParams *)p;

	if (pTcpParams->ul_ConnectionStatus != CONNECTED)
	{
		// 1. unregister from timer task:
		// Bracha: Set the timer interval to zero
		TimerResetInterval(pTcpParams->ul_TimerHandle, TIMER_FALSE);

		if (pTcpParams->s >= 0)
		{
			//yigal - add fd_isset to close the ema socket from the wd task and not from mcmscomrcv - please do not delete!!!
			if (FD_ISSET(pTcpParams->s, ptTcpActiveSelectSockets))
			{
				FD_CLR(pTcpParams->s,ptTcpActiveReadSockets); // Benson - yigal
				MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CheckServerConnStatus : FD_CLR socket = %d",pTcpParams->s);
			}

    		rc = close(pTcpParams->s);
    		if (rc == 0)
    		{
    			pTcpParams->s = -1;
    			MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CheckServerConnStatus : success closing socket = %d",pTcpParams->s);
    		}
	    	else
	    	{
				MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CheckServerConnStatus : fail closing socket = %d, rc = %d",pTcpParams->s,GetLastError());
	    	}
		}

        //if (pTcpParams->ul_PrevConnState == CONNECTED)
        //{
        //  	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CheckServerConnStatus : closing socket %d returned with %d",pTcpParams->s,rc);
        //}
	}


}

void ConnectToTcpServer(t_TcpConnParams* ptConParams)
{
	if ( ( ptConParams->s = InitV4Connection() ) == SOCKET_OPERATION_FAILED)
	{
	     MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TcpConnectToServer): client init failed !!!\n");
	}


	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(ConnectToTcpServer): e_Id %d , nSocket %d is Connecting to nServer %s on nPort %d",ptConParams->e_Id,ptConParams->s,&(ptConParams->IpV4Addr.auc_IpV4Address),ptConParams->us_Port);

    ptConParams->ul_PrevConnState  = CONNECTED;
	ptConParams->ul_ClientOrServer = eConnTypeClient;

    ConnectV4Connection(ptConParams->s , ptConParams->us_Port, ptConParams->IpV4Addr,ptConParams->e_Id);

    FD_SET(ptConParams->s,ptTcpActiveReadSockets);

/*	l.a. moved to  RegisterToTimerJob()
    // Init timer to check for disconnection and reconnect
    memset((void*)&(ptConParams->TimerJobReq),0,sizeof(TTimerJobReq));

    ptConParams->TimerJobReq.ulTicksCount 	= 500;
    ptConParams->TimerJobReq.ulInterval 	= TIMER_TRUE;
    ptConParams->TimerJobReq.pvContext		= (void*)ptConParams;
    ptConParams->TimerJobReq.callback 		= CheckConnStatus;


    ptConParams->ul_TimerHandle = TimerSetJob(&(ptConParams->TimerJobReq));
*/
	RegisterToTimerJob(ptConParams, 500, CheckClientConnStatus);
}


void RegisterToTimerJob(t_TcpConnParams* ptConParams, UINT32 ulInterval, void *pCallBackFunc)
{
	MfaBoardPrint(SHARED_PRINT, PRINT_LEVEL_ERROR, PRINT_TO_TERMINAL,"RegisterToTimerJob: e_Id = %d", ptConParams->e_Id);
    // Init timer to check for disconnection and reconnect
    memset((void*)&(ptConParams->TimerJobReq),0,sizeof(TTimerJobReq));

    ptConParams->TimerJobReq.ulTicksCount 	= ulInterval;
    ptConParams->TimerJobReq.ulInterval 	= TIMER_TRUE;
    ptConParams->TimerJobReq.pvContext		= (void*)ptConParams;
    ptConParams->TimerJobReq.callback 		= pCallBackFunc;

    ptConParams->ul_TimerHandle = TimerSetJob(&(ptConParams->TimerJobReq));

	return;
}

