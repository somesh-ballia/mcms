// MessagePool.cpp

#include "MessagePool.h"

#include <ostream>
#include <iomanip>
#include <algorithm>
#include <vector>

#include "ProcessBase.h"
#include "TraceStream.h"
#include "Trace.h"
#include "ObjString.h"

extern const char* ProcessTypeToString(eProcessType processType);

void CMessageQueue::DumpMessagePool(std::ostream& ostr, DWORD numOfMessages)
{
  CProcessBase* proc = CProcessBase::GetProcess();
  DWORD queueSize = m_OpcodeQueue.size();
  DWORD startIndex = (queueSize < numOfMessages ? 0 : queueSize - numOfMessages);
  for (DWORD i = 0; i < queueSize; i++)
  {
    // Keeps copy of the front element on stack
    const CMessageInfo info = m_OpcodeQueue.front();

    if (i == startIndex)
      ostr << std::setfill(' ') << std::right << std::showbase
           << "--------------------------------------------------------------------------------\n"
           << std::setw(3) << "num" << " | "
           << std::setw(9) << "ticks" << " | "
           << std::setw(7) << "size" << " | "
           << std::setw(20) << "sender" << " | "
           << std::setw(8) << "opcode" << " | "
           << std::setw(18) << "name"
           << std::endl
           << "----+-----------+---------+----------------------+----------+-------------------\n";

    if (i >= startIndex)
    {
      DWORD ticks = info.m_Ticks.GetIntegerPartForTrace();
      const char* processName = ProcessTypeToString(info.m_SenderProcessType);

      ostr << std::setw(3) << i - startIndex + 1 << " | "
           << std::setw(9) << ticks << " | "
           << std::setw(7) << info.m_Len << " | "
           << std::setw(20) << processName << " | "
           << std::setw(8) << info.m_Opcode << " | "
           << proc->GetOpcodeAsString(info.m_Opcode) << std::endl;
    }

    // Removes front element
    m_OpcodeQueue.pop();

    // Puts the element to the tail
    m_OpcodeQueue.push(info);
  }
  ostr << endl;
}

void CMessageQueue::PushMessage(OPCODE opcode, DWORD len, eProcessType processType)
{
	bool res = CheckFilter(opcode);
	if (!res)
		return;

	if(m_OpcodeQueue.size() == m_Size)
		m_OpcodeQueue.pop();
	
	TICKS ticks = SystemGetTickCount();
	CMessageInfo info(opcode, len, processType, ticks);
	m_OpcodeQueue.push(info);
}

void CMessageQueue::AddFilter(OPCODE opcode)
{
	m_FilterMessages.push_back(opcode);
}

bool CMessageQueue::CheckFilter(OPCODE opcode)
{
	CFilterMessages::iterator iBegin 	= m_FilterMessages.begin();
	CFilterMessages::iterator iEnd 		= m_FilterMessages.end();	
	CFilterMessages::iterator iFound 	= std::find(iBegin, iEnd, opcode);
	bool isNotFound = (iFound == iEnd);
	return isNotFound;
}
