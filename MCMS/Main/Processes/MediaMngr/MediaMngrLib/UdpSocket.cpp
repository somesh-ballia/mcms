#include "UdpSocket.h"
#include "MediaMngr.h"
#include <cerrno>
#include "SystemFunctions.h"
#include "Trace.h"
#include "TraceStream.h"

using namespace std;

//PBEGIN_MESSAGE_MAP(CUdpSocket)
	
//PEND_MESSAGE_MAP(CUdpSocket, CStateMachine)


CUdpSocket::CUdpSocket(DWORD address, DWORD port)
{
	CreateUdpSocket();
	InitUdpSocket(address, port);
}

////////////////////////////////////////////////////////////////////////////////////

CUdpSocket::~CUdpSocket()
{
	CloseUdpSocket();
}

////////////////////////////////////////////////////////////////////////////////////

int CUdpSocket::CreateUdpSocket()
{
	/* Open a datagram socket */
	m_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	if (m_descriptor == STATUS_ERROR)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CUdpSocket::CreateUdpSocket(): socket() errno=" << myerrno << " description: " << myerrnostr.c_str();
		return STATUS_ERROR;
	}
	
	TRACEINTO << "CUdpSocket::CreateUdpSocket() new file descriptor created. m_descriptor = " << m_descriptor;
	
	return m_descriptor;
}

////////////////////////////////////////////////////////////////////////////////////
#if 0
int CUdpSocket::InitUdpSocket()
{
	/*set up client address*/
	memset((void *)&m_client, 0, sizeof(m_client));
	m_client.sin_family = AF_INET; //Address family
	m_client.sin_port = INADDR_ANY;//  htons(0); //port to use
	m_client.sin_addr.s_addr = INADDR_ANY; //Wild card IP address - with more than 1 NIC - need to investigate!!!
	
	/* Bind local address to socket */
	int bindResult = bind(m_descriptor, (struct sockaddr *)&m_client, sizeof(m_client));
	
	if (bindResult == STATUS_ERROR)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CUdpSocket::InitUdpSocket(): bind() errno=" << myerrno << " description: " << myerrnostr.c_str();
		CloseUdpSocket();
		return STATUS_ERROR;
	}
	
	TRACEINTO << "CUdpSocket::InitUdpSocket() bind: " << bindResult;
	
	return bindResult;
}
#endif
////////////////////////////////////////////////////////////////////////////////////

int CUdpSocket::InitUdpSocket(DWORD address, DWORD port)
{
	/*set up client address*/
	memset((void *)&m_client, 0, sizeof(m_client));
	m_client.sin_family = AF_INET; //Address family
	m_client.sin_port = port;
	m_client.sin_addr.s_addr = address; //with more than 1 NIC - need to investigate!!!

	
	/* Bind local address to socket */
	int bindResult = bind(m_descriptor, (struct sockaddr *)&m_client, sizeof(m_client));
	
	if (bindResult == STATUS_ERROR)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CUdpSocket::InitUdpSocket(address, port): bind() errno=" << myerrno << " description: " << myerrnostr.c_str();
		CloseUdpSocket();
		return STATUS_ERROR;
	}
	
	TRACEINTO << "CUdpSocket::InitUdpSocket(address, port) bind: " << bindResult;
	
	return bindResult;
}

////////////////////////////////////////////////////////////////////////////////////

int CUdpSocket::CloseUdpSocket()
{
	TRACEINTO << "CUdpSocket::CloseUdpSocket(): m_descriptor = " << m_descriptor;
	
	/* Close a datagram socket */
	int closeResult = close(m_descriptor);
	
	if (closeResult == STATUS_ERROR)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CUdpSocket::CloseUdpSocket(): close() errno=" << myerrno << " description: " << myerrnostr.c_str();
		return STATUS_ERROR;
	}
	
	TRACEINTO << "CUdpSocket::CloseUdpSocket(): closeResult = " << closeResult;
	
	return closeResult;
}

////////////////////////////////////////////////////////////////////////////////////
#if 0
int CUdpSocket::SendTo(const char* buffer, const sockaddr* dest)
{
	//TRACEINTO << "CUdpSocket::SendTo(char*)";
	
	int sizeSent = sendto(m_descriptor,
						  buffer,
						  strlen(buffer),
						  0,
						  (struct sockaddr *)dest,
						  sizeof((struct sockaddr_in&)dest));
	
	//TRACEINTO << "CUdpSocket::SendTo(char*) sizeSent=" << sizeSent;
	
	if (sizeSent == STATUS_ERROR)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CUdpSocket::SendTo(char*): sendto() errno=" << myerrno << " description: " << myerrnostr.c_str();
	}
	
	return sizeSent;
}
#endif
////////////////////////////////////////////////////////////////////////////////////

int CUdpSocket::SendTo(const char* buffer, const sockaddr* dest, DWORD len)
{
	//TRACEINTO << "CUdpSocket::SendTo(BYTE*)";
	
	int sizeSent = sendto(m_descriptor,
						  buffer,
						  len,
						  0,
						  (struct sockaddr *)dest,
						  sizeof((struct sockaddr_in&)dest));
	
	//TRACEINTO << "CUdpSocket::SendTo(BYTE*) sizeSent=" << sizeSent;
	
	if (sizeSent == STATUS_ERROR)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CUdpSocket::SendTo(BYTE*): sendto() errno=" << myerrno << " description: " << myerrnostr.c_str();
	}
	
	return sizeSent;
}

////////////////////////////////////////////////////////////////////////////////////

int CUdpSocket::RecvFrom(char* buffer, unsigned int buf_size, const sockaddr* from)
{
	//TRACEINTO << "CUdpSocket::RecvFrom(char*)";
	
	struct timeval tv;
	tv.tv_sec = 1; // seconds
	tv.tv_usec = ((tv.tv_sec*1000) % 1000) * 1000;
	
	int socketTimeOutRet = setsockopt(m_descriptor,
									  SOL_SOCKET,
									  SO_RCVTIMEO,
									  (char *)&tv,
									  sizeof tv);
	
	if (socketTimeOutRet<0)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CUdpSocket::RecvFrom(char*) setsockopt() errno=" << myerrno << " description: " << myerrnostr.c_str();		
		return STATUS_ERROR;
	}

	
	int fromlen = sizeof(from);
	
	int sizeRecv = recvfrom(m_descriptor,
							buffer,
							buf_size,
							0,
							(struct sockaddr *)&from,
							(socklen_t*)&fromlen);
	
	if (sizeRecv == STATUS_ERROR)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CUdpSocket::RecvFrom(char*) recvfrom() errno=" << myerrno << " description: " << myerrnostr.c_str();	   
	}
		
	//TRACEINTO << "CUdpSocket::RecvFrom(char*) sizeRecv=" << sizeRecv;
	
	
	/*if (sizeRecv>=0)
	{
		char* ipAddress = inet_ntoa(((sockaddr_in*)&from)->sin_addr);
		TRACEINTO << "CUdpSocket::RecvFrom(char*) client address=" << ipAddress << " port: " << ntohs(((sockaddr_in*)&from)->sin_port);
	}
	
	//check sizeRecv==0 (look in man recvfrom)
	*/ 
	
	
	return sizeRecv;
}

////////////////////////////////////////////////////////////////////////////////////

#if 0
int CUdpSocket::RecvFrom(BYTE* buffer, const sockaddr* from)
{
	//TRACEINTO << "CUdpSocket::RecvFrom(BYTE*)";
	
	struct timeval tv;
	tv.tv_sec = 1; // seconds
	tv.tv_usec = ((tv.tv_sec*1000) % 1000) * 1000;
	int socketTimeOutRet = setsockopt(m_descriptor,
									  SOL_SOCKET,
									  SO_RCVTIMEO,
									  (char *)&tv,
									  sizeof tv);
	
	if (socketTimeOutRet<0)
	{	
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CUdpSocket::RecvFrom(BYTE*) setsockopt() errno=" << myerrno << " description: " << myerrnostr.c_str();		
		return STATUS_ERROR;
	}

	int fromlen = sizeof(from);
	
	int sizeRecv = recvfrom(m_descriptor,
							buffer,
							MAX_NUM_OF_BYTES_IN_UDP_DATAGRAM,
							0,
							(struct sockaddr *)&from,
							(socklen_t*)&fromlen);
	
	if (sizeRecv == STATUS_ERROR)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CUdpSocket::RecvFrom(BYTE*) recvfrom() errno=" << myerrno << " description: " << myerrnostr.c_str();	   
	}
	
	//TRACEINTO << "CUdpSocket::RecvFrom(BYTE*) sizeRecv=" << sizeRecv;
	
	/*if (sizeRecv>=0)
	{
		char* ipAddress = inet_ntoa(((sockaddr_in*)&from)->sin_addr);
		TRACEINTO << "CUdpSocket::RecvFrom(BYTE*) client address=" << ipAddress << " port: " << ((sockaddr_in*)&from)->sin_port;
	}	
	//check sizeRecv==0 (look in man recvfrom)
	//The return value will be 0 when the peer has performed an orderly shutdown.
	// or udp packet sent was of size 0 
	 * */
	
	return sizeRecv;
}
#endif

////////////////////////////////////////////////////////////////////////////////////


int CUdpSocket::Select(int timeOut)
{
	fd_set read_mask;
    //fd_set write_mask;
    //fd_set except_mask;
    struct timeval tvWait;
    
    tvWait.tv_sec = timeOut;
    tvWait.tv_usec = 0;
    
    //TRACEINTO << "CUdpSocket::Select()";
	
	FD_ZERO(&read_mask);
	FD_SET(m_descriptor, &read_mask);

	/*FD_ZERO(&write_mask);
	FD_SET(m_descriptor, &write_mask);
	FD_ZERO(&except_mask);
	FD_SET(m_descriptor, &except_mask);*/


	//int selectResult = select(FD_SETSIZE, &read_mask, &write_mask, &except_mask, &tvWait);
	int selectResult = select(FD_SETSIZE, &read_mask, (fd_set*)0, (fd_set*)0, &tvWait);
	
	if (selectResult < 0) //Process the error condition
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CUdpSocket::Select select() errno=" << myerrno << " description: " << myerrnostr.c_str();
		return selectResult;
	}
	else if (selectResult == 0) //Process the select timeout condition
	{
		//TRACEINTO << "CUdpSocket::Select select() TIMEOUT";
		return selectResult;
	}
	
	//TRACEINTO << "CUdpSocket::Select select() selectResult: " << selectResult;
			
	int read = FD_ISSET(m_descriptor, &read_mask);
	/*int write = FD_ISSET(m_descriptor, &write_mask);
	int excepction = FD_ISSET(m_descriptor, &except_mask);*/
	
	//TRACEINTO << "CUdpSocket::Select select() read: " << read;// << " write: " << write << " excepction: " << excepction;
	
	return selectResult;
}
