// OsSocketClient.cpp

#include "OsSocketClient.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "SystemFunctions.h"
#include "FaultsDefines.h"
#include "ProcessBase.h"

COsSocketClient::COsSocketClient() :
  m_descriptor(-1)
{
  memset(&m_serverIp, 0, sizeof(mcTransportAddress));
}

void COsSocketClient::Close()
{
  close(m_descriptor);
  m_descriptor = -1;
}

//////////////////////////////////////////////////////////////////
// The socket has been closed by TX/RX task. 
// No need to close it again if the creator task gets SOCKET_DROPPED message 
void COsSocketClient::Drop()
{
	m_descriptor = -1;
}

void COsSocketClient::SetAddress(const mcTransportAddress ipAddr)
{
  m_serverIp = ipAddr;
}

void COsSocketClient::CreateSocketConnected(COsSocketConnected** pConnected)
{
  *pConnected = new COsSocketConnected(128 * 1024 - 1, -1);
  (*pConnected)->SetDescriptor(m_descriptor);
}

STATUS COsSocketClient::ConfigureClientSocket()
{
  if (eIpVersion4 == m_serverIp.ipVersion)
    m_descriptor = socket(PF_INET, SOCK_STREAM, 0);
  else
    m_descriptor = socket(PF_INET6, SOCK_STREAM, 0);

  if (m_descriptor == -1)
  {
    TRACEWARN << "socket: " << strerror(errno) << " (" << errno << ")";
    return STATUS_FAIL;
  }

  DWORD asyncio = 0xffff;  //0 = sync , non-zero = async
  int res = ioctl(m_descriptor, FIONBIO, &asyncio);
  if (res == -1)
  {
    TRACEWARN << "ioctl: " << strerror(errno) << " (" << errno << ")";
    Close();
    return STATUS_FAIL;
  }
    
  struct linger ling;
  ling.l_onoff = 1;
  ling.l_linger = 0;

  res = setsockopt(m_descriptor, SOL_SOCKET, SO_LINGER, &ling, sizeof ling);
  if (res == -1)
  {
    TRACEWARN << "setsockopt: " << strerror(errno) << " (" << errno << ")";
    Close();
    return STATUS_FAIL;
  }

  return STATUS_OK;
}

STATUS COsSocketClient::Connect()
{    
  struct sockaddr_storage server_adr;
  socklen_t               addrlen;

  if(eIpVersion4 == m_serverIp.ipVersion)
  {
    server_adr.ss_family = AF_INET;
    ((struct sockaddr_in*)&server_adr)->sin_family = AF_INET;
    ((struct sockaddr_in*)&server_adr)->sin_addr.s_addr = m_serverIp.addr.v4.ip;
    ((struct sockaddr_in*)&server_adr)->sin_port = htons(m_serverIp.port);
    addrlen = sizeof(sockaddr_in);
  }
  else
  {
    server_adr.ss_family = AF_INET6;
    ((struct sockaddr_in6*)&server_adr)->sin6_family = AF_INET6;

    memcpy(((struct sockaddr_in6*)&server_adr)->sin6_addr.s6_addr,
           m_serverIp.addr.v6.ip,
           IPV6_ADDRESS_BYTES_LEN);

    ((struct sockaddr_in6*)&server_adr)->sin6_port = htons(m_serverIp.port);
    addrlen = sizeof(sockaddr_in6);
  }

  int res = connect(m_descriptor, (struct sockaddr*) &server_adr, addrlen);
  if (res == -1)
  {
    char buf[64];
    TRACEWARN << "connect: " << "IP " << ipToString(m_serverIp, buf, 1)
              << ", port " << m_serverIp.port
              << ": " << strerror(errno) << " (" << errno << ")";

    return STATUS_FAIL;
  }

  return STATUS_OK;
}

BOOL COsSocketClient::Select(TICKS timeout)
{
  fd_set setWrite;

  int  rc, connectTimeOut = 10;
  BOOL bConnected  = FALSE;

  FD_ZERO(&setWrite);
  FD_SET(m_descriptor, &setWrite);

  for (int i=0; i < connectTimeOut && !bConnected; i++)
  {
    rc = select(FD_SETSIZE,
                (fd_set*)NULL,
                &setWrite,
                (fd_set*)NULL,
                &timeout.m_self);

    if (rc < 0)
    {
      if (errno != EWOULDBLOCK)
        bConnected = FALSE;
    }
    else if (rc > 0)
    {
      struct sockaddr_storage server_adr;
      socklen_t               addrlen;

      if(eIpVersion4 == m_serverIp.ipVersion)
      {
        server_adr.ss_family = AF_INET;
        ((struct sockaddr_in*)&server_adr)->sin_family = AF_INET;
        ((struct sockaddr_in*)&server_adr)->sin_addr.s_addr = m_serverIp.addr.v4.ip;
        ((struct sockaddr_in*)&server_adr)->sin_port = htons( m_serverIp.port );
        addrlen = sizeof(sockaddr_in);
      }
      else
      {
        server_adr.ss_family = AF_INET6;
        ((struct sockaddr_in6*)&server_adr)->sin6_family = AF_INET6;
        memcpy(((struct sockaddr_in6*)&server_adr)->sin6_addr.s6_addr,
               m_serverIp.addr.v6.ip,
               IPV6_ADDRESS_BYTES_LEN);

        ((struct sockaddr_in6*)&server_adr)->sin6_port = htons( m_serverIp.port );
        addrlen = sizeof(sockaddr_in6);
      }

      if (!getpeername(m_descriptor,(struct sockaddr *)&server_adr, &addrlen))
      {
        PTRACE(eLevelError,"COsSocketClient::Select - socket connected.");
        bConnected = TRUE;
      }
    }
  }

  return bConnected;
}
