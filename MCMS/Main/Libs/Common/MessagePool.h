// MessagePool.h

#ifndef MESSAGEPOOL_H_
#define MESSAGEPOOL_H_

#include <queue>
#include <map>
#include <vector>

#include "PObject.h"
#include "McmsProcesses.h"
#include "SystemTick.h"
#include "Macros.h"

const DWORD DEFAULT_POOL_SIZE = 100;

struct CMessageInfo
{
  OPCODE       m_Opcode;
  DWORD        m_Len;
  eProcessType m_SenderProcessType;
  TICKS        m_Ticks;

public:
  CMessageInfo(OPCODE opcode,
               DWORD len,
               eProcessType senderProcessType,
               TICKS ticks)
  {
    m_Opcode            = opcode;
    m_SenderProcessType = senderProcessType;
    m_Ticks             = ticks;
    m_Len               = len;
  }
  CMessageInfo()
  {
    m_Opcode            = 0;
    m_SenderProcessType = eProcessTypeInvalid;
    m_Len               = 0;
  }
};

typedef std::queue<CMessageInfo> COpcodeQueue;
typedef std::vector<OPCODE> CFilterMessages;

class CMessageQueue;
typedef std::map<std::string, CMessageQueue*> CTaskPoolMap;

class CMessageQueue : public CPObject
{
CLASS_TYPE_1(CMessageQueue, CPObject)
public:
  CMessageQueue(DWORD poolSize = DEFAULT_POOL_SIZE):m_Size(poolSize){}

  virtual const char* NameOf() const { return "CMessageQueue"; }
  void DumpMessagePool(std::ostream& ostr, DWORD numOfMessages);

  void PushMessage(OPCODE opcode, DWORD len, eProcessType processType);
  void AddFilter(OPCODE opcode);
  DWORD GetLength() const
  {
    return m_OpcodeQueue.size();
  }

private:
  bool CheckFilter(OPCODE opcode);

  const DWORD 	  m_Size;
  COpcodeQueue 	  m_OpcodeQueue;
  CFilterMessages m_FilterMessages;

  DISALLOW_COPY_AND_ASSIGN(CMessageQueue);
};

#endif
