// MessageHeader.h

#ifndef MESSAGE_HEADER_H_
#define MESSAGE_HEADER_H_

#include "OsQueue.h"
#include "DataTypes.h"

struct StateMachineDescriptor
{
  StateMachineDescriptor();
  StateMachineDescriptor(const CStateMachine* pStateMachine,
                         DWORD stateMachineHandle);
  BOOL IsValid() const
  {
    return m_StateMachineHandle != 0;
  }

  const CStateMachine* m_pStateMachine;
  DWORD m_StateMachineHandle;
};

BOOL operator==(const StateMachineDescriptor& first,
                const StateMachineDescriptor& second);

BOOL operator<(const StateMachineDescriptor& first,
               const StateMachineDescriptor& second);

class CSegment;
class CMessageHeader
{
public:
  CMessageHeader();
  CMessageHeader(const COsQueue& addressee, const COsQueue& sendert);

  BOOL IsValid() const;

  DWORD                  m_validFlag;
  CSegment*              m_segment;
  DWORD                  m_bufferLen;
  OPCODE                 m_opcode;
  eMessageType           m_msgType;
  eProcessType           m_process;
  DWORD                  m_RspMsgSeqNum;
  COsQueue               m_addressee;
  COsQueue               m_sender;
  StateMachineDescriptor m_stateMachine;
  StateMachineDescriptor m_senderStateMachine;
  DWORD                  m_ifNoneMatch;
};

#endif  // ifndef MESSAGE_HEADER_H_
