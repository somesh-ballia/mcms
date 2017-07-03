// OsQueue.cpp

#include "OsQueue.h"

#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <poll.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "Trace.h"
#include "Segment.h"
#include "NStream.h"
#include "OsFileIF.h"
#include "ObjString.h"
#include "ProcessBase.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "SystemFunctions.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "InternalProcessStatuses.h"

class CQueueStatistics
{
public:
  CQueueStatistics();
  void Clear();
  int          m_numberOfMessages;
  long long    m_numberOfBytes;
  long long    m_numberOfBytesInBody;
  int          m_numberOfRetries;
  int          m_numberOfSegmentsPointers;
  int          m_maxUsed;
  eProcessType m_process;
};



class COsQueueInternals : public CPObject
{
	CLASS_TYPE_1(COsQueueInternals, CPObject)
public:
  COsQueueInternals();
  virtual const char*          NameOf() const {return "COsQueueInternals";}

  // Makes two times spare slots for socket descriptors in case
  // of restore to defaults
  enum {
    MAX_CONNECTIONS = NUM_OF_PROCESS_TYPES * 2
  };

  int              m_bufferSize;
  int m_SndBufferSize;
  int              m_bufferThreshold;
  int              m_lastHandled;
  int              m_lastICPAEvent;
  int              m_opened[MAX_CONNECTIONS];
  char             m_UniqueName[MAX_QUEUE_NAME_LEN];
  CQueueStatistics m_statistics[MAX_CONNECTIONS];
};

CQueueStatistics::CQueueStatistics()
{
  Clear();
}

void CQueueStatistics::Clear()
{
  m_numberOfMessages         = 0;
  m_numberOfBytes            = 0;
  m_numberOfBytesInBody      = 0;
  m_numberOfRetries          = 0;
  m_numberOfSegmentsPointers = 0;
  m_maxUsed                  = 0;
  m_process                  = eProcessTypeInvalid;
}


COsQueueInternals::COsQueueInternals() :
  m_bufferSize(128 * 1024 - 1),
  m_bufferThreshold(-1),
  m_lastHandled(-1),
  m_lastICPAEvent(-1)
{
  std::fill(m_opened, ARRAYEND(m_opened), -1);
  std::fill(m_UniqueName, ARRAYEND(m_UniqueName), '\0');
  m_SndBufferSize = -1; // use kernel default queue size
}

CSegment& operator<<(CSegment& seg, eProcessType v)
{
	return seg << (int)(v);
}

CSegment& operator>>(CSegment& seg, eProcessType& v)
{
	int i = 0;
	seg >> i;
	v = static_cast<eProcessType>(i);
	return seg;
}

CSegment& operator<<(CSegment& seg, const COsQueue& obj)
{
	return seg << obj.m_id << (byte)obj.m_idType << (void*)obj.m_internals << obj.m_process << obj.m_scope;
}

CSegment& operator>>(CSegment& seg, COsQueue& obj)
{
	return seg >> obj.m_id >> (byte&)obj.m_idType >> (void*&)obj.m_internals >> obj.m_process >> obj.m_scope;
}

COsQueue::COsQueue() :
  m_id(0xffffffff),
  m_idType(eInvalidId),
  m_process(eProcessTypeInvalid),
  m_scope(eProcessTypeInvalid),
  m_internals(NULL)
{}

COsQueue::COsQueue(eQueueIdType type, int id, eProcessType process) :
  m_id(id),
  m_idType(type),
  m_process(process),
  m_scope(process),
  m_internals(NULL)
{}

COsQueue::COsQueue(const COsQueue& rhs) :
  m_id(rhs.m_id),
  m_idType(rhs.m_idType),
  m_process(rhs.m_process),
  m_scope(rhs.m_scope),
  m_internals(rhs.m_internals)
{
  // Works as default copy-constructor
  // m_internals is shared between objects
}

COsQueue& COsQueue::operator=(const COsQueue& rhs)
{
  if (this == &rhs)
    return *this;

  // Works as default copy-constructor
  // m_internals is shared between objects
  m_id        = rhs.m_id;
  m_idType    = rhs.m_idType;
  m_process   = rhs.m_process;
  m_scope     = rhs.m_scope;
  m_internals = rhs.m_internals;

  return *this;
}

// Virtual
COsQueue::~COsQueue()
{}

void COsQueue::Serialize(CSegment& seg) const
{
  seg << (DWORD) m_id;
  seg << (DWORD) m_idType;
  seg << (DWORD) m_process;
  seg << (DWORD) m_scope;
}

BOOL COsQueue::IsValid() const
{
  if (m_id <= 0 ||
      m_idType == eInvalidId ||
      m_process <= eProcessTypeInvalid ||
      m_scope <= eProcessTypeInvalid)
    return FALSE;

  return TRUE;
}

void COsQueue::DeSerialize(CSegment& seg)
{
  DWORD temp;

  seg >> temp;
  m_id = temp;

  seg >> temp;
  m_idType = (eQueueIdType) temp;

  seg >> temp;
  m_process = (eProcessType) temp;

  seg >> temp;
  m_scope = (eProcessType) temp;

  IsValid();
}

int COsQueue::GetICPAEvent() const
{
  if (!CPObject::IsValidPObjectPtr(m_internals))
    return -1;

  return m_internals->m_lastICPAEvent;
}

STATUS COsQueue::Delete()
{
  STATUS        status = STATUS_OK;
  CProcessBase* currentProcess = CProcessBase::GetProcess();
  eProcessType  currentProcessType = eProcessTypeInvalid;
  if (currentProcess)
    currentProcessType = currentProcess->GetProcessType();

  if (m_scope == currentProcessType)
  {
    if (m_idType == eReadHandle)
    {
      // Closes all open connection to this socket
      for (int* fd = m_internals->m_opened;
          fd < ARRAYEND(m_internals->m_opened);
          fd++)
      {
        if (-1 == *fd)
          continue;

        if (0 == close(*fd))
          continue;

        std::ostringstream msg;
        msg << "close: " << *fd << ": "
            << strerror(errno) << " (" << errno << ")";
        WriteLocalLog(msg.str().c_str());

        status = STATUS_FAIL;
      }
    }

    // Closes the socket
    if (-1 == close(m_id))
    {
      std::ostringstream msg;
      msg << "close: " << m_id << ": "
          << strerror(errno) << " (" << errno << ")";
      WriteLocalLog(msg.str().c_str());

      status = STATUS_FAIL;
    }

    // Deletes the socket file (anyway)
    if (m_idType == eReadHandle)
    {
      int res = unlink(m_internals->m_UniqueName);
      if (-1 == res && errno != ENOENT)
      {
        std::ostringstream msg;
        msg << "unlink: " << m_internals->m_UniqueName << ": "
            << strerror(errno) << " (" << errno << ")";
        WriteLocalLog(msg.str().c_str());

        status = STATUS_FAIL;
      }
    }
  }
  else
  {
    // Never delete queues of other processes
    if (m_scope != eProcessTypeInvalid)
      status = STATUS_FAIL;
  }
  if (CPObject::IsValidPObjectPtr(m_internals))
  {
	  //FPASSERTMSG(1, "processTyep Exceeded");
	  delete m_internals;
  }

  m_internals = NULL;

  return status;
}

STATUS COsQueue::Receive(CMessageHeader& header,
                         TICKS timeOut,
                         int otherFD) const
{
  FTRACECOND_AND_RETURN_VALUE(m_idType != eReadHandle,
      "Queue ID type " << m_idType << " isn't matched to eReadHandle",
      STATUS_QUEUE_ID_INVALID);

  TICKS  expire = SystemGetTickCount() + timeOut;
  STATUS ret_status;
  int    iter = 0;
  do
  {
    iter++;
    if (iter > 1000)
      return STATUS_FAIL;

    ret_status = STATUS_OK;
    int con = -1;
    int res = -1;
    int idx = -1;
    int num_fds = 0;
    pollfd my_fds[ARRAYSIZE(m_internals->m_opened) + 2];

    my_fds[num_fds].fd = m_id;
    my_fds[num_fds].events = POLLIN;
    my_fds[num_fds].revents = 0;
    num_fds++;

    // We need to listen to other fd too
    if (otherFD != -1)
    {
      my_fds[num_fds].fd = otherFD;
      my_fds[num_fds].events = POLLIN;
      my_fds[num_fds].revents = 0;
      num_fds++;
    }

    int first_index_of_internal = num_fds;
    for (const int* fd = m_internals->m_opened;
        fd < ARRAYEND(m_internals->m_opened);
        fd++)
    {
      if (-1 == *fd)
        continue;

      my_fds[num_fds].fd = *fd;
      my_fds[num_fds].events = POLLIN;
      my_fds[num_fds].revents = 0;
      num_fds++;
    }

    if (timeOut != NEVER)
    {
      // This is not the first iteration of the do while statement
      if (iter > 1)
      {
        TICKS now = SystemGetTickCount();
        if (now > expire)
          return STATUS_QUEUE_TIMEOUT;

        timeOut = expire - now;
      }

      DWORD miliseconds = timeOut.GetMiliseconds();
      res = poll(my_fds, num_fds, miliseconds);
    }
    else
    {
      res = poll(my_fds, num_fds, -1);
    }

    // Does not report about the failure because it is as-designed flow in
    // case of log message flooding
    if (-1 == res)
      return STATUS_FAIL;

    if (res == 0)
      return STATUS_QUEUE_TIMEOUT;

    if (my_fds[0].revents != 0)
    {
      con = accept(m_id, NULL, NULL);
      FTRACECOND_AND_RETURN_VALUE(-1 == con,
          "accept:  [Scope:FD]-[" << m_scope << ":" << m_id << "] " << strerror(errno) << " (" << errno << ")",
          STATUS_FAIL);

      res = setsockopt(con,
                       SOL_SOCKET,
                       SO_RCVBUF,
                       &m_internals->m_bufferSize,
                       sizeof m_internals->m_bufferSize);
      FTRACECOND_AND_RETURN_VALUE(-1 == res,
          "setsockopt: " << con << ": " << strerror(errno) << " (" << errno << ")",
          STATUS_FAIL);

      // Finds first free slot for fd
      for (int* fd = m_internals->m_opened;
          fd < ARRAYEND(m_internals->m_opened);
          fd++)
      {
        if (-1 != *fd)
          continue;

        *fd = con;
        idx = std::distance(m_internals->m_opened, fd);
        m_internals->m_statistics[idx].Clear();
        break;
      }

      if (-1 == idx)
      {
        FTRACEWARN << "There is no free slot for a socket's descriptor";
        close(con);
        return STATUS_FAIL;
      }
    }

    if (otherFD != -1 && my_fds[1].revents != 0)
    {
      m_internals->m_lastICPAEvent = (int)(my_fds[1].revents);
      return STATUS_PRIVATE_FD;
    }

    // This is not new connection
    if (con == -1)
    {
      int index_internal = first_index_of_internal;
      for (int* fd2 = m_internals->m_opened;
          fd2 < ARRAYEND(m_internals->m_opened);
          fd2++)
      {
        if (-1 == *fd2)
          continue;

        if (0 == my_fds[index_internal++].revents)
          continue;

        con = *fd2;
        idx = std::distance(m_internals->m_opened, fd2);
        break;
      }

      FTRACECOND_AND_RETURN_VALUE(-1 == idx,
          "WARNING: select failed",
          STATUS_FAIL);

      // Reads the message itself.
      int n = read(con, (char*)&header, sizeof header);
      if (n != sizeof header || !header.IsValid())
      {
    	  if(n != 0) // case where connection is not closed.
    	  {
            FTRACEWARN << "readBytes:"    << n
                   << ", validation:" << header.m_validFlag
                   << ", opcode:"     << header.m_opcode
                   << ", process:"    << header.m_process
                   << ", type:"       << header.m_msgType
                   << ": dropped corrupted message";
    	  }
        close(con);
        m_internals->m_opened[idx] = -1;
        return STATUS_FAIL;
      }

      int unread;
      int res1 = ioctl(con, FIONREAD, &unread);
      if (unread > m_internals->m_statistics[idx].m_maxUsed)
      {
    	  m_internals->m_statistics[idx].m_maxUsed = unread;
	  #ifdef PRFFORMANCE_ANALYSIS
    	  if (m_process == eProcessConfParty)
    	  {
    		  if (unread > m_internals->m_bufferSize * 0.01)
    			  FTRACEINTO << "Performance analysis:\n\tUnread length: " << unread;
    		  else if (unread > m_internals->m_bufferSize * 0.5)
    			  FTRACESTRFUNC(eLevelError) << "Performance analysis:\n\tUnread length: " << unread;
    	  }
	  #endif
      }
      m_internals->m_statistics[idx].m_numberOfBytes += n;
      m_internals->m_statistics[idx].m_process = header.m_process;
      m_internals->m_lastHandled = idx;

      if (header.m_segment)
      {
        m_internals->m_statistics[idx].m_numberOfSegmentsPointers++;
        m_internals->m_statistics[idx].m_numberOfMessages++;
        return STATUS_OK;
      }

      if (header.m_bufferLen)
      {
        // peak who much bytes are waiting on the buffer (total)
       // int unread;
        if ((unread == 0) ||         // the buffer is empty ???
            (m_internals->m_bufferThreshold != -1 &&
             m_internals->m_bufferThreshold < unread))                                                 // the buffer contains more than the queue is expected (logger overflow)
        {
          close(con);
          m_internals->m_opened[idx] = -1;
          return STATUS_FAIL;
        }

       // if (unread > m_internals->m_statistics[idx].m_maxUsed)
        //  m_internals->m_statistics[idx].m_maxUsed = unread;

        char* buffer = new char[header.m_bufferLen];
        int   r = 0;
        DWORD dwReceived = 0;
        int   nRetries = 0;
        m_internals->m_lastHandled = idx;
        while ((dwReceived < header.m_bufferLen) &&
               ((r >= 0) || ((r < 0) && (errno != EAGAIN))) &&
               (nRetries < 1000))
        {
          r = read(con, buffer + dwReceived, header.m_bufferLen - dwReceived);
          if (r >= 0)
          {
            dwReceived += r;
            m_internals->m_statistics[idx].m_numberOfBytes += r;
            m_internals->m_statistics[idx].m_numberOfBytesInBody += r;
            if (dwReceived < header.m_bufferLen && nRetries > 500)
              SystemSleep(3);
          }

          nRetries++;
        }

        if (nRetries == 1000)
        {
          close(con);
          m_internals->m_opened[idx] = -1;
          delete [] buffer;
          return STATUS_FAIL;
        }

        m_internals->m_statistics[idx].m_numberOfRetries += nRetries-1;

        if (dwReceived < header.m_bufferLen)
        {
          close(con);
          m_internals->m_opened[idx] = -1;
          delete [] buffer;
          return STATUS_FAIL;
        }

        m_internals->m_statistics[idx].m_numberOfMessages++;
        header.m_segment = new CSegment;
        header.m_segment->Create(buffer, header.m_bufferLen);

#ifdef PRFFORMANCE_ANALYSIS
        if (m_process == eProcessConfParty && header.m_process == eProcessMplApi)
        {
        	static long iCounter = 0;
        	static long long nBytesReceived = 0;
        	nBytesReceived += sizeof header + header.m_bufferLen;
			OPCODE msgOpcode = 0;
			COMMON_HEADER_S stMcmsCommonHeader;
			int nLen = (header.m_bufferLen < (int) (sizeof(COMMON_HEADER_S)) ?
							header.m_bufferLen : (int) (sizeof(COMMON_HEADER_S)));
			int iRetVal = header.m_segment->LookAt((BYTE*) (&stMcmsCommonHeader), 0, nLen);
			if (iRetVal >= 0)
				msgOpcode = stMcmsCommonHeader.opcode;
			FTRACEINTO << "Performance analysis\nHeader length: " << sizeof header
					<< "\nSegment length: " << header.m_bufferLen
					<< "\nQueueID:" << m_id
					<< ", QueueIDType:" << m_idType
					<< ", DstProcess: "	<< CProcessBase::GetProcessName(m_process)
                    << ", SrcProcess: "    << CProcessBase::GetProcessName(header.m_process)
					<< ", Opcode: "	<< (iRetVal == 0 ? 0 : msgOpcode)
					<< ", Unread length: "	<< unread
					<< ", Counter: "	<< ++iCounter
 					<< ", BytesReceived: "	<< nBytesReceived;
       }
#endif
      }
    }
    else
    {
      ret_status = STATUS_NEW_CONNECTION;
    }
  }
  while (ret_status == STATUS_NEW_CONNECTION);

  return STATUS_OK;
}

STATUS COsQueue::Flush() const
{
  CMessageHeader header;
  STATUS res = STATUS_OK;
  int counter = 0;
  while (res == STATUS_OK && counter < 100)
  {
    counter++;
    res = Receive(header, 10, -1);
    if (res == STATUS_OK)
      POBJDELETE(header.m_segment);
  }

  return STATUS_OK;
}

void COsQueue::CreateQueueName(char* queueName,
                               const eProcessType processType,
                               const char* taskName)
{
  const char* processName = CProcessBase::GetProcessName(processType);
  std::string queue = MCU_TMP_DIR+"/queue";
  sprintf(queueName, "%s/%s_%s", queue.c_str(), processName, taskName);
}

void COsQueue::DeleteAllProcessQueues(const eProcessType processType)
{}

STATUS COsQueue::CreateRead(const eProcessType processType,
                            const char* taskName,
                            int bufferSize,
                            int bufferThreshhold)
{
   if (!CPObject::IsValidPObjectPtr(m_internals))
  {
      m_internals = new COsQueueInternals;
  }

  m_internals->m_bufferSize = bufferSize;
  m_internals->m_bufferThreshold = bufferThreshhold;
  CreateQueueName(m_internals->m_UniqueName, processType, taskName);
  std::fill(m_internals->m_opened, ARRAYEND(m_internals->m_opened), -1);

  unlink(m_internals->m_UniqueName);
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (-1 == fd)
  {
    std::ostringstream msg;
    msg << "socket: " << strerror(errno) << " (" << errno << ")";
    WriteLocalLog(msg.str().c_str());
    return STATUS_FAIL;
  }

  struct sockaddr_un addr;
  strncpy(addr.sun_path, m_internals->m_UniqueName, sizeof addr.sun_path - 1);
  addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
  addr.sun_family = AF_UNIX;
  int res = bind(fd,
                 (struct sockaddr*) &addr,
                 strlen(addr.sun_path) +sizeof (addr.sun_family));

  if (-1 == res)
  {
    std::ostringstream msg;
    msg << "bind: " << fd << ": " << strerror(errno) << " (" << errno << ")";
    WriteLocalLog(msg.str().c_str());
    return STATUS_FAIL;
  }

  res = listen(fd, ARRAYSIZE(m_internals->m_opened));
  if (-1 == res)
  {
    std::ostringstream msg;
    msg << "listen: " << fd << ": " << strerror(errno) << " (" << errno << ")";
    WriteLocalLog(msg.str().c_str());
    return STATUS_FAIL;
  }

  m_process = processType;
  m_scope   = CProcessBase::GetProcess()->GetProcessType();
  m_id      = fd;
  m_idType  = eReadHandle;

  return STATUS_OK;
}


STATUS COsQueue::CreateWrite(const eProcessType processType,
                             const char* taskName,
                             BOOL isNonBlocked,
                             int bufferSize)
{
  if (!CPObject::IsValidPObjectPtr(m_internals))
  {
      m_internals = new COsQueueInternals;
  }

  m_internals->m_SndBufferSize = bufferSize;
  CreateQueueName(m_internals->m_UniqueName, processType, taskName);
  std::fill(m_internals->m_opened, ARRAYEND(m_internals->m_opened), -1);

  m_id = socket(AF_UNIX, SOCK_STREAM, 0);
  if (-1 == m_id)
  {
    std::ostringstream msg;
    msg << "socket: " << strerror(errno) << " (" << errno << ")";
    WriteLocalLog(msg.str().c_str());
    return STATUS_FAIL;
  }

  //if m_SndBufferSize==-1 use the default send buffer size, no need to call setsockopt
  if (m_internals->m_SndBufferSize>0 &&
		  	  setsockopt(m_id,
				  SOL_SOCKET,
				  SO_SNDBUF,
				  &(m_internals->m_SndBufferSize),
				  sizeof(m_internals->m_SndBufferSize)))
  {
    // Error
  }

  if (isNonBlocked)
  {
    int yes = 1;
    ioctl(m_id, FIONBIO, &yes);
  }

  struct sockaddr_un addr;
  snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", m_internals->m_UniqueName);
  addr.sun_family = AF_UNIX;
  int res = connect(m_id,
                    (struct sockaddr*) &addr,
                    strlen(addr.sun_path) + sizeof (addr.sun_family));

  chmod(m_internals->m_UniqueName, 0666);

  if (res == -1)
  {
    close(m_id);
    m_id = -1;
    return STATUS_FAIL;
  }

  m_process = processType;
  CProcessBase* pMyProcess = CProcessBase::GetProcess();
  m_scope   = (pMyProcess) ? pMyProcess->GetProcessType() : eProcessTypeInvalid;
  m_idType  = eWriteHandle;

  return STATUS_OK;
}

STATUS COsQueue::Send(CSegment* seg, OPCODE opcode,
					const COsQueue* respone,
					const StateMachineDescriptor* pStateMachineDesc,
					const StateMachineDescriptor* pSenderStateMachineDesc,
					eMessageType msgType,
					BOOL deleteSegment, // delete segment on error
					DWORD RspMsgSeqNum) const
{
	CProcessBase* pMyProcess = CProcessBase::GetProcess();
	eProcessType currentProcess = eProcessTypeInvalid;
	if (CPObject::IsValidPObjectPtr(pMyProcess))
		currentProcess = pMyProcess->GetProcessType();

	BOOL isQueueInScope = (currentProcess == m_scope);
	BOOL isInterProcess = (currentProcess != m_process);
	if (pMyProcess == NULL)
		isInterProcess = TRUE;

	BOOL isSegment = (seg != NULL);

	const COsQueue* sendToQueue = this;

	// In this case we need to send the message to the through the dispatcher
	if (!isQueueInScope)
	{
		eOtherProcessQueueEntry dispatcher = eDispatcher;
		if (msgType == eSyncMessageRsp)
			dispatcher = eSyncDispatcher;

		sendToQueue = CProcessBase::GetProcess()->GetOtherProcessQueue(m_process, dispatcher);

		if (sendToQueue == NULL)
		{
			if (deleteSegment)
				PDELETE(seg);

			return STATUS_FAIL;
		}
	}

	bool bBufferAllocated = false;
	CMessageHeader header(*this, respone ? *respone : COsQueue());

	header.m_process = currentProcess;
	header.m_RspMsgSeqNum = RspMsgSeqNum;

	if (isQueueInScope)
	{
		if (msgType == eSyncMessage)
			header.m_msgType = eDirectSyncMessage;
		else
			header.m_msgType = eDirectMessage;
	}
	else
		header.m_msgType = msgType;

	header.m_opcode = opcode;
	if (pStateMachineDesc)
		header.m_stateMachine = *pStateMachineDesc;

	if (pSenderStateMachineDesc)
		header.m_senderStateMachine = *pSenderStateMachineDesc;

	char* buffer_to_send = NULL;
	int buffer_size = 0;
	if (isSegment && isInterProcess)
	{
		// Appends the segment to the header
		int seg_len = seg->GetWrtOffset();
		header.m_bufferLen = seg_len;
		header.m_segment = NULL;

		buffer_size = sizeof header + seg_len;
		buffer_to_send = new char[buffer_size];
		bBufferAllocated = true;

		memset(buffer_to_send, 0, buffer_size);
		memcpy(buffer_to_send, &header, sizeof header);
		memcpy(buffer_to_send + sizeof header, seg->GetPtr(), seg_len);
	}
	else
	{
		// we send only the header - this message is opcode only,
		// or inside the same process (segment is sent by pointer)
		header.m_bufferLen = 0;
		header.m_segment = seg; // seg could be NULL here, (opcode message)
		buffer_to_send = (char*) &header;
		buffer_size = sizeof header;
	}

	DWORD wasWritten = 0;
	STATUS bRes = sendToQueue->Write(buffer_to_send,  // data buffer
			buffer_size,     // number of bytes to write
			&wasWritten);    // number of bytes written

	if (bBufferAllocated)
		delete[] buffer_to_send;

#ifdef PRFFORMANCE_ANALYSIS
	if (m_process == eProcessConfParty && m_scope == eProcessMplApi)
	{
		static long iCounterSent = 0;
		static long long nBytesSent = 0;
		nBytesSent += wasWritten;
		if (bRes == STATUS_OK)
		{
			OPCODE msgOpcode = 0;
			COMMON_HEADER_S stMcmsCommonHeader;
			int nLen = (
					header.m_bufferLen < (int) (sizeof(COMMON_HEADER_S)) ?
							header.m_bufferLen : (int) (sizeof(COMMON_HEADER_S)));
			int iRetVal = -1;
			if (seg)
				iRetVal = seg->LookAt((BYTE*) (&stMcmsCommonHeader), 0, nLen);
			if (iRetVal >= 0)
				msgOpcode = stMcmsCommonHeader.opcode;
			FTRACEINTO << "Performance analysis\nSent: " << wasWritten
					<< ", QueueID: "	<< m_id
					<< ", QueueIDType: "<< m_idType
					<< ", DstProcess: "	<< CProcessBase::GetProcessName(m_process)
					<< ", SrcProcess: "	<< CProcessBase::GetProcessName(m_scope)
					<< ", Opcode: "		<< (iRetVal == 0 ? 0 : msgOpcode)
					<< ", STATUS: "		<< bRes
					<< ", Counter: "	<< ++iCounterSent
					<< ", BytesSent: "	<< nBytesSent;
		}
	}
#endif

	if (bRes != STATUS_OK)
	{
		if (isSegment && (isInterProcess || !isQueueInScope)) // !isQueueInScope
		{
			if (deleteSegment)
				PDELETE(seg);
		}

		return bRes;
	}

	if (isSegment && (isInterProcess || !isQueueInScope))
		PDELETE(seg);

	return STATUS_OK;
}

STATUS COsQueue::Write(const char* buffer,
                       int len,
                       DWORD* numberOfBytesWritten) const
{
  *numberOfBytesWritten = write(m_id, buffer, len);

  if (*numberOfBytesWritten != (DWORD)len)
  {
    switch (errno)
    {
    case EAGAIN:
      return STATUS_SOCKET_WOULD_BLOCKED;

    case EPIPE:
      return STATUS_BROKEN_PIPE;

    case ENOTCONN:
      return STATUS_TRANSPORT_NOTCONNECTED;

    default:
      return STATUS_FAIL;
    }
  }
  return STATUS_OK;
}

const char* COsQueue::GetName() const
{
  if (!CPObject::IsValidPObjectPtr(m_internals))
    return "invalid";

  return m_internals->m_UniqueName;
}

// Builds unique name, use date and time functions in order to get the unique name,
// plus add counter at the end of the name
void COsQueue::CreateUniqueQueueName(char *buffer)
{
  static DWORD queueNameCounter = 0;
  char date_buffer[32];
  char time_buffer[32];
  time_t rawtime, seconds;
  struct tm * timeinfo;
  memset(time_buffer, 0, 32);
  seconds = time(&rawtime);
  sprintf(time_buffer, "%ld", seconds);

  queueNameCounter++;
  if (queueNameCounter > 0xFFFFFFFD)
    queueNameCounter = 0;

  sprintf(buffer, "%s_%d", time_buffer, queueNameCounter);
}

std::ostream& operator <<(std::ostream& os, const COsQueue& queue)
{
	if (!CPObject::IsValidPObjectPtr(queue.m_internals))
		return os;

	for (unsigned int i = 0; i < ARRAYSIZE(queue.m_internals->m_opened); ++i)
	{
		if (queue.m_internals->m_opened[i] == -1)
			continue;

		int bufferSize, unread;
		socklen_t len = sizeof(bufferSize);

		int res1 = getsockopt(
			queue.m_internals->m_opened[i],
			SOL_SOCKET,
			SO_RCVBUF,
			&bufferSize,
			&len);

		int res2 = ioctl(queue.m_internals->m_opened[i], FIONREAD, &unread);

		if (res1 == 0 && res2 == 0)
		{
			const char*  processName = NULL;
			eProcessType processType = queue.m_internals->m_statistics[i].m_process;

			if (processType == eProcessTypeInvalid)
				processName = "IPCListen";
			else if (processType >= NUM_OF_PROCESS_TYPES)
				FPASSERTMSG(1, "processTyep Exceeded");
			else
				processName = ProcessNames[processType];

			COstrStream tmpString;
			tmpString << unread << "/" << queue.m_internals->m_statistics[i].m_maxUsed;

			os
				<< std::setw(3) << i
				<< " Name:" <<  std::setw(20) << processName
				<< " Used/max:" << std::setw(10) << tmpString.str()
				<< " Size:" << std::setw(10) << queue.m_internals->m_bufferSize
				<< " Msg:" << std::setw(10) << queue.m_internals->m_statistics[i].m_numberOfMessages
				<< " Bytes:" << std::setw(12) << queue.m_internals->m_statistics[i].m_numberOfBytes
				<< " Body:" << std::setw(12) << queue.m_internals->m_statistics[i].m_numberOfBytesInBody
				<< " Ptr:" << std::setw(10) << queue.m_internals->m_statistics[i].m_numberOfSegmentsPointers;

			if (queue.m_internals->m_bufferThreshold != -1)
				os << " Threshold:" << queue.m_internals->m_bufferThreshold;

			if (queue.m_internals->m_statistics[i].m_numberOfRetries != 0)
				os << " Retries:" << queue.m_internals->m_statistics[i].m_numberOfRetries;

			os << "\n";
		}
	}

	return os;
}
void COsQueue::ClearStatistics()
{
	unsigned int n = ARRAYSIZE(m_internals->m_opened);
	for (unsigned int i = 0; i < n; i++)
	{
	    if (m_internals->m_opened[i] != -1)
			m_internals->m_statistics[i].Clear();
	}
}
