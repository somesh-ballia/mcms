// OsQueue.h                                    |

#ifndef _OSQUEUE
#define _OSQUEUE

#include <string>

#include "Macros.h"
#include "DataTypes.h"
#include "DataTypes.h"
#include "SystemTick.h"
#include "MessageType.h"
#include "SharedDefines.h"
#include "McmsProcesses.h"

#define MAX_QUEUE_NAME_LEN 160

class CSegment;
class CStateMachine;
class CMessageHeader;
class COsQueueInternals;
struct StateMachineDescriptor;

enum eQueueIdType
{
  eInvalidId,
  eReadHandle,
  eWriteHandle,
  ePartyId,
  eConfId,
  eStreamId,
  eServiceId
};

enum eBufferContent
{
  eHeaderBeforeMessage,
  eHeader,
  eMessage,
  eWindows
};

class COsQueue;
CSegment& operator<<(CSegment& seg, const COsQueue& obj);
CSegment& operator>>(CSegment& seg, COsQueue& obj);

CSegment& operator<<(CSegment& seg, eProcessType v);
CSegment& operator>>(CSegment& seg, eProcessType& v);

class COsQueue
{
  friend std::ostream& operator<<(std::ostream& os, const COsQueue& queue);
  friend CSegment& operator<<(CSegment& seg, const COsQueue& obj);
  friend CSegment& operator>>(CSegment& seg, COsQueue& obj);
public:
  static void CreateQueueName(char* queueName,
                              const eProcessType processType,
                              const char* taskName);
  static void CreateUniqueQueueName(char* tbuffer);
  static void DeleteAllProcessQueues(const eProcessType processType);

  COsQueue();
  COsQueue(eQueueIdType type, int id, eProcessType process);
  COsQueue(const COsQueue& rhs);
  COsQueue& operator=(const COsQueue& rhs);
  ~COsQueue();

  void        Serialize(CSegment& seg) const;
  void        DeSerialize(CSegment& seg);
  int         GetICPAEvent() const;
  STATUS      CreateRead(const eProcessType processType,
                         const char* taskName,
                         int bufferSize = 128*1024-1,
                         int bufferThreshold = -1);
  STATUS      CreateWrite(const eProcessType processType,
                          const char* taskName,
                          BOOL isNonBlocked = TRUE,
                          int bufferSize = -1);
  STATUS      Delete();
  STATUS      Receive(CMessageHeader& header,
                      TICKS timeOut = NEVER,
                      int otherFD = -1) const;
  STATUS      Send(CSegment* seg,
                   OPCODE opcode,
                   const COsQueue* rsp = NULL,
                   const StateMachineDescriptor* pStateMachineDesc = NULL,
                   const StateMachineDescriptor* pSenderStateMachineDesc = NULL,
                   eMessageType syncType = eAsyncMessage,
                   BOOL deleteSegment = TRUE,
                   DWORD RspMsgSeqNum = 0) const;
  STATUS      Flush() const;
  BOOL        IsValid() const;
  const char* GetName() const;
  STATUS      ReadOneLine(std::string& string, TICKS timeOut = NEVER);
  void        DumpQueues(std::ostream& msg) const;
  void        ClearStatistics();

  int          m_id;
  eQueueIdType m_idType;
  eProcessType m_process;  // The destination process of the queue
  eProcessType m_scope;    // The scope process of the id (HANDLE),
                           // the process that created the COsQueue object
private:
  STATUS      Write(const char* buffer,
                    int len,
                    DWORD* numberOfBytesWritten) const;

  COsQueueInternals* m_internals;
};

std::ostream& operator<<(std::ostream& os, const COsQueue& queue);

#endif
