// OsSocketConnected.cpp

#include "OsSocketConnected.h"

#include <errno.h>
#include <ostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include "Trace.h"
#include "Segment.h"
#include "DataTypes.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "SystemFunctions.h"
#include "InternalProcessStatuses.h"

COsSocketConnected::COsSocketConnected(int size, int threshold)
{
  m_descriptor = -1;
  m_numberOfMessages = 0;
  m_numberOfBytes = 0;
  m_numberOfRetries = 0;
  m_bufferSize = size;
  m_bufferThreshold = threshold;
  m_remoteIp = 0;
}

COsSocketConnected::~COsSocketConnected()
{ }

void COsSocketConnected::SetDescriptor(int desc)
{
  m_descriptor = desc;
}

DWORD COsSocketConnected::GetRemoteIp() const
{
  return m_remoteIp;
}

void COsSocketConnected::SetRemoteIp(DWORD ip)
{
  m_remoteIp = ip;
}

void COsSocketConnected:: Serialize(CSegment& seg) const
{
  seg << (DWORD) m_descriptor;
  seg << (DWORD) m_remoteIp;
}

void COsSocketConnected:: DeSerialize(CSegment& seg)
{
  DWORD temp;
  seg >> temp;
  m_descriptor = temp;

  seg >> temp;
  m_remoteIp = temp;
}

void COsSocketConnected::Close()
{
  int               retVal = close(m_descriptor);
  std::string       str;
  std::stringstream descStr;
  descStr << m_descriptor;
  str = "COsSocketConnected::Close ,m_descriptor: ";
  str += descStr.str();
  if (retVal == -1)
  {
    str += " *** close() failed (retVal == -1), ";
    str += strerror(errno);
  }
  else
  {
    str += " close() OK ";
  }

  FPTRACE(eLevelInfoNormal, str.c_str());

  m_descriptor = -1;
}

int COsSocketConnected::Receive(char* buffer, int bytesToRead)
{
	// Allows to run other tasks during the operation
	//CTaskApp::Unlocker unlocker(true);// removed for BRIDGE-13399 

	if (m_descriptor != -1)
		return recv(m_descriptor, buffer, bytesToRead, 0);

	FTRACEINTO << "m_descriptor is -1";
	return -1;
}

int COsSocketConnected::Read(char* buffer,
                             int len,
                             int& sizeRead,
                             const CTaskApp& task,
                             BYTE partialRcv /*=FALSE*/)
{
  // Returns number of bytes actually read
  int counter = 0;
  int ReadEmptyCounter = 0;
  int bytesToRead = len;
  sizeRead = 0;
  int nRead = 0;

  while (bytesToRead > 0)
  {
    if (task.GetSelfKill())
    {
      FTRACEINTO << "Got task selfkill. Stop reading.";
      return STATUS_FAIL;
    }
    nRead = Receive(buffer, bytesToRead);

    if (nRead > 0)
    {
      m_numberOfBytes += nRead;
      bytesToRead -= nRead;
      buffer += nRead;

      if (bytesToRead <= 0 || partialRcv == TRUE)
        break;

      SystemSleep(6);
    }
    else if (nRead == 0)
    {
      ReadEmptyCounter++;
      m_numberOfRetries++;
      if (ReadEmptyCounter > 10)
      {
        FPTRACE(eLevelInfoHigh,
                "COsSocketConnected::Read - failed reading (read =0)");
        return STATUS_FAIL;
      }

      SystemSleep(6);
    }
    else
    {
      if (errno != EWOULDBLOCK)
      {
        //FTRACEINTOFUNC << "Failed reading. errno = " << errno;
        return STATUS_FAIL;
      }
      else
      {
        counter++;
        if (counter > 1000)
        {
          FPASSERT(errno);
          return STATUS_FAIL;
        }
      }

      SystemSleep(6);
    }
  } // end while

  sizeRead = len-bytesToRead;
  m_numberOfMessages++;
  return STATUS_OK;
}

int COsSocketConnected::Select(int timeout)
{
  fd_set         read_mask;
  fd_set         write_mask;
  fd_set         except_mask;
  struct timeval wait;
  int            res;

  wait.tv_sec = 2; // Wait 0 secs.
  wait.tv_usec = 0;
  FD_ZERO(&read_mask);
  FD_SET(m_descriptor, &read_mask);
  FD_ZERO(&except_mask);
  FD_SET(m_descriptor, &except_mask);
  FD_ZERO(&write_mask);
  FD_SET(m_descriptor, &write_mask);

  res = select(FD_SETSIZE,
               &read_mask,
               (fd_set*)&write_mask,
               (fd_set*)&except_mask,
               &wait);

  int i = FD_ISSET(m_descriptor, &except_mask);
  i = FD_ISSET(m_descriptor, &read_mask);
  i = FD_ISSET(m_descriptor, &write_mask);
  return res;
}

int COsSocketConnected::Send(const char* buffer, int bytesToWrite)
{
  if (m_descriptor != -1)
    return send(m_descriptor, buffer, bytesToWrite, 0);

  FTRACEINTO << "m_descriptor is -1";
  return -1;
}

STATUS COsSocketConnected::Write(const char* buffer,
                                 int len,
                                 BOOL dropIfBlocked)
{
  int bytesToWrite = len;
  int wouldblock = 0;
  int nWrote = 0;
  int reSendCounter = 0;

  while (bytesToWrite > 0)
  {
    int end_loop = 0;

    nWrote = Send(buffer, bytesToWrite);

    // try to send 3 times and then finish the loop
    if (nWrote > 0)
    {
      bytesToWrite -= nWrote;
      buffer += nWrote;
    }
    else
    {
      if (nWrote == 0)
      {
        reSendCounter++;
        if (reSendCounter < 3)
        {
          continue;
        }
        else
        {
          FTRACEINTO << "reSendCounter = 3, sent zero bytes";
          return STATUS_OK;
        }
      }

      FTRACEWARN << "send: " << strerror(errno) << " (" << errno << ")";

      if (errno == EWOULDBLOCK)
      {
        if (dropIfBlocked && bytesToWrite == len)
          return STATUS_SOCKET_WOULD_BLOCKED;

        wouldblock++;
        if (wouldblock < 100)
        {
          SystemSleep(10);
          continue;
        }
      }

      return STATUS_FAIL;
    }
  } // end while

  return STATUS_OK;
}

COsSocketConnected& COsSocketConnected::operator=(const COsSocketConnected& other)
{
  if (this != &other)
  {
    m_numberOfBytes    = other.m_numberOfBytes;
    m_numberOfMessages = other.m_numberOfMessages;
    m_numberOfRetries  = other.m_numberOfRetries;
    m_descriptor       = other.m_descriptor;
    m_remoteIp         = other.m_remoteIp;
    m_bufferSize       = other.m_bufferSize;
    m_bufferThreshold  = other.m_bufferThreshold;
  }

  return *this;
}

std::ostream& operator<<(std::ostream& os, const COsSocketConnected& socket)
{
  int res1, res2;
  int bufferSize, unread;

  socklen_t len = sizeof(bufferSize);
  res1 = getsockopt(socket.m_descriptor,
                    SOL_SOCKET,
                    SO_RCVBUF,
                    &bufferSize,
                    &len);

  res2 = ioctl(socket.m_descriptor,
               FIONREAD,
               &unread);

  if (res1 == 0 && res2 == 0)
  {
    os <<  " Used:" << std::setw(10) << unread <<"/" << std::setw(12) <<
    bufferSize << " Msg:" << std::setw(10) << socket.m_numberOfMessages
               << " Bytes:" << std::setw(12) << socket.m_numberOfBytes;

    if (socket.m_numberOfRetries != 0)
    {
      os << " Retries:" << socket.m_numberOfRetries;
    }

    os << "\n";
  }

  return os;
}

void COsSocketConnected::SetTlsParams(void* ssl)
{}

BYTE COsSocketConnected::IsSecured()
{
  return FALSE;
}

int COsSocketConnected::IsPeerSocketDisconnected()
{
  int  res = SOCKET_STATUS_OK;
  int  size;
  char buffer[10];
  int  bytesToRead = 2;
  size = recv(m_descriptor, buffer, bytesToRead, MSG_PEEK | MSG_NOSIGNAL);

  FTRACEINTO << "size is " << size << ", errno is " << errno;

  if (size < 0)
  {
    switch (errno)
    {
      case ENETDOWN:
      case ENETRESET:
      case ENOTCONN:
      case EHOSTUNREACH:
      case ECONNABORTED:
      case ECONNRESET:
      case ETIMEDOUT:
      case ESHUTDOWN:
        res = SOCKET_STATUS_ERROR;
        break;

      case EAGAIN:
        break;

      default:
        break;
    }
  }

  return res;
}
