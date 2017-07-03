
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "OsSocketListener.h"
#include "OsSocketConnected.h"
#include "StatusesGeneral.h"
#include "InternalProcessStatuses.h"
#include "Trace.h"

//////////////////////////////////////////////////////////////////
COsSocketListener::COsSocketListener(WORD port)
{
	m_portNum = port;
	m_descriptor = -1;
}

//////////////////////////////////////////////////////////////////
COsSocketListener::~COsSocketListener()
{
	
}


//////////////////////////////////////////////////////////////////
void COsSocketListener::Close()
{
	int status = close(m_descriptor);
	if (status != 0)
		FPASSERT(m_descriptor);
	m_descriptor = -1;
}

//////////////////////////////////////////////////////////////////
STATUS COsSocketListener::ConfigureListenSocket(DWORD remoteIp,
                                                const char* deviceName)
{
	m_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (m_descriptor == -1) 
	{
        perror("COsSocketListener::ConfigureListenSocket socket failed :");
		//error
		//TraceSocketErr(SOCKET_ACTION_SOCKET, pna_errno());
		return STATUS_FAIL;
	}
	
	//Bind socket to address
	struct sockaddr_in myaddr;
    
	memset((char *)&myaddr, 0, sizeof(struct sockaddr_in));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(remoteIp);//INADDR_ANY);
	myaddr.sin_port = htons(m_portNum);
	
	int err = bind(m_descriptor,
				  (struct sockaddr *) &myaddr,
				  sizeof(struct sockaddr_in));
	if (err !=0) 
	{
        perror("COsSocketListener::ConfigureListenSocket bind failed :");
        Close();
		return STATUS_FAIL;
	}
	
	
	DWORD asyncio = 0xffff; //0 = sync , non-zero = async 

    err = ioctl(m_descriptor, FIONBIO,&asyncio); 
	if (err !=0) 
	{
		//TraceSocketErr(SOCKET_ACTION_IOCTL, pna_errno());
		Close();
		return STATUS_FAIL;
     }

    struct  linger ling;
    
    ling.l_onoff = 1;
    ling.l_linger = 0;
    
    setsockopt(m_descriptor, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));


    if (deviceName)
    {
        setsockopt(m_descriptor,
                   SOL_SOCKET,
                   SO_BINDTODEVICE,
                   deviceName,
                   strlen(deviceName));
    }
    
	//Listen
	err = listen(m_descriptor,5);
	if (err !=0) 
	{
        perror("COsSocketListener::ConfigureListenSocket listen failed :");
		//TraceSocketErr(SOCKET_ACTION_LISTEN, pna_errno());
		Close();
		return STATUS_FAIL;
	}
	return STATUS_OK;
	
}


//////////////////////////////////////////////////////////////////
// not in use
STATUS COsSocketListener::Listen(TICKS timeout, 
								 COsSocketConnected & connected)
{
    
	struct sockaddr peeraddr;
	socklen_t addrlen = sizeof(struct sockaddr);
	fd_set read_mask;
	struct timeval wait;
	wait.tv_sec = 1; //wait one second
	wait.tv_usec = 0;
	FD_ZERO(&read_mask);
	FD_SET(m_descriptor,&read_mask);
	
	int nb = select(FD_SETSIZE,&read_mask,(fd_set *)0,(fd_set *)0,&wait);

	if (nb == 0) 
		return STATUS_SOCKET_TIMEOUT;

	if (nb<0 || ( !FD_ISSET(m_descriptor,&read_mask) ))
	{
        perror("COsSocketListener::Listen select failed");
        
		//select failed
		Close();
		return STATUS_FAIL;    
	}

	// socket created here
	int s = accept(m_descriptor, &peeraddr, &addrlen);

	connected.SetDescriptor(s);
	
	if (s == -1)
		return STATUS_FAIL;
    

	return STATUS_OK;
}

#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////
STATUS COsSocketListener::Accept(COsSocketConnected & connected)
{    
	struct sockaddr peeraddr;
	socklen_t addrlen = sizeof(struct sockaddr);
	int s = accept(m_descriptor, &peeraddr, &addrlen);

	 //VNGR-18245
	 //int optval =1;
	 //setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
     //int yes = 1;
     // ioctl(s, FIONBIO, &yes);

	connected.SetDescriptor(s);
    
    DWORD *ptrRemoteIp = (DWORD*)(peeraddr.sa_data + 2);
    DWORD remoteIp = ntohl(*ptrRemoteIp);
 	connected.SetRemoteIp(remoteIp);
    
	if (s == -1)
		return STATUS_FAIL;
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////
STATUS COsSocketListener::Reject(COsSocketConnected & connected)
{
    struct sockaddr peeraddr;
	socklen_t addrlen = sizeof(struct sockaddr);
	int s = accept(m_descriptor, &peeraddr, &addrlen);
    if(s != -1)
    {
    	int status = close(s);
    	if (status != 0)
    		FPASSERT(s);
    	
    }
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////
STATUS COsSocketListener::Close(COsSocketConnected & connected)
{
    int s = connected.GetDescriptor();
    if(s != -1)
    {
    	int status = close(s);
    	if (status != 0)
    		FPASSERT(s);
    }
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////
STATUS COsSocketListener::CloseSocket(int s)
{

    if(s != -1)
    {
        int status = close(s);
        if (status != 0)
            FPASSERT(s);
    }
    return STATUS_OK;
}
