// MessageHeader.cpp

#include "MessageHeader.h"

#include "OpcodesRanges.h"
#include "SharedDefines.h"
#include "StateMachine.h"
#include "MessageType.h"

#define VALID_HEADER 0x900df1f1

CMessageHeader::CMessageHeader() :
  m_validFlag(VALID_HEADER),
  m_segment(0),
  m_bufferLen(0),
  m_opcode(INVALID_OPCODE),
  m_msgType(eAsyncMessage),
  m_process(eProcessTypeInvalid),
  m_RspMsgSeqNum(0)
{
	m_ifNoneMatch = 0;
}

CMessageHeader::CMessageHeader(const COsQueue& addressee,
                               const COsQueue& sender) :
  m_validFlag(VALID_HEADER),
  m_segment(NULL),
  m_bufferLen(0),
  m_opcode(INVALID_OPCODE),
  m_msgType(eAsyncMessage),
  m_process(eProcessTypeInvalid),
  m_RspMsgSeqNum(0),
  m_addressee(addressee),
  m_sender(sender)
{
	m_ifNoneMatch = 0;
}

BOOL CMessageHeader::IsValid() const
{
  if (VALID_HEADER != m_validFlag)
    return FALSE;

  if (INVALID_OPCODE == m_opcode)
    return FALSE;

  if (m_process >= NUM_OF_PROCESS_TYPES || m_process <= eProcessTypeInvalid)
    return FALSE;

  if (m_msgType >= eInvalidMessageType || m_msgType < eAsyncMessage)
    return FALSE;

  return TRUE;
}

StateMachineDescriptor::StateMachineDescriptor() :
  m_pStateMachine(NULL),
  m_StateMachineHandle(0)
{}

StateMachineDescriptor::StateMachineDescriptor(const CStateMachine* pStateMachine,
                                               DWORD stateMachineHandle) :
  m_pStateMachine(pStateMachine),
  m_StateMachineHandle(stateMachineHandle)
{}

BOOL operator==(const StateMachineDescriptor& first,
                const StateMachineDescriptor& second)
{
  return (first.m_pStateMachine == second.m_pStateMachine &&
          first.m_StateMachineHandle == second.m_StateMachineHandle);
}

BOOL operator<(const StateMachineDescriptor& first,
               const StateMachineDescriptor& second)
{
  return (first.m_StateMachineHandle < second.m_StateMachineHandle);
}
