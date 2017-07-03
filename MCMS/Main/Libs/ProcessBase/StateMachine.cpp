// StateMachine.cpp

#include "StateMachine.h"

#include <iomanip>
#include <ostream>
#include <algorithm>

#include "Timer.h"
#include "Trace.h"
#include "Macros.h"
#include "OsTask.h"
#include "OsQueue.h"
#include "TaskApp.h"
#include "Segment.h"
#include "TimerDesc.h"
#include "PrettyTable.h"
#include "ProcessBase.h"
#include "TraceStream.h"
#include "SystemFunctions.h"
#include "OpcodesMcmsCommon.h"

// Explicit.
CMsgMapEntryIndex::CMsgMapEntryIndex(const PMSG_MAP_ENTRY* entry)
{
  for (; entry->actFunc != 0; entry++)
  {
    Key key = std::make_pair(entry->event, entry->state);
    m_index.insert(std::make_pair(key, entry));
  }
}

const PMSG_MAP_ENTRY* CMsgMapEntryIndex::Find(OPCODE event, WORD state) const
{
  // Looks for event of the current state.
  Map::const_iterator it = m_index.find(std::make_pair(event, state));

  if (m_index.end() != it)
    return it->second;

  // Looks for event in ANYCASE state.
  it = m_index.find(std::make_pair(event, ANYCASE));
  if (m_index.end() != it)
    return it->second;

  return NULL;
}

void CMsgMapEntryIndex::Dump(std::ostream& out) const
{
  CPrettyTable<const char*,
               const char*,
               unsigned int,
               unsigned int,
               unsigned int,
               unsigned int>
  tbl("event", "state", "cnt", "real", "sys", "user");

  for (Map::const_iterator i = m_index.begin(); i != m_index.end(); ++i)
    tbl.Add(i->second->eventStr,
            i->second->stateStr,
            i->second->msgCount,
            i->second->real.GetIntegerPartForTrace(),
            i->second->system.GetIntegerPartForTrace(),
            i->second->user.GetIntegerPartForTrace());

  tbl.SetCaption("event list");
  tbl.Sort(2);
  tbl.Dump(out);
}

bool CMsgMapEntryIndex::ValidateIntersection(const CMsgMapEntryIndex& rhs) const
{
  bool ret = true;

  for (Map::const_iterator it = m_index.begin(); it != m_index.end(); it++)
  {
    for (Map::const_iterator i2 = rhs.m_index.begin(); i2 != rhs.m_index.end(); i2++)
    {
      if (it->first != i2->first)
        continue;

      if (ret)
        ret = false;

      FTRACEWARN << "Event conflicts " << it->second->eventStr
                 << ", state "         << it->second->stateStr;
    }
  }

  return ret;
}

bool CMsgMapEntryIndex::Validate() const
{
  bool ret = true;

  for (Map::const_iterator it = m_index.begin(); it != m_index.end(); it++)
  {
    // Searches the container for an element with a key of x and returns the
    // number of elements having that key.
    size_t cnt = m_index.count(it->first);
    if (cnt > 1)
    {
      if (ret)
        ret = false;

      FTRACEWARN << "Event conflicts " << it->second->eventStr
                 << ", state "         << it->second->stateStr
                 << ", counter "       << cnt;
    }
  }

  return ret;
}

PBEGIN_MESSAGE_MAP(CStateMachine)
PEND_MESSAGE_MAP(CStateMachine, CStateMachine);

CStateMachine::CStateMachine(CTaskApp* pOwnerTask) :
    m_state(IDLE),
    m_suspended(FALSE),
    m_current_log_entry(0)
{
  // Data-member of the base class.
  m_type = GetCompileType();
  RegisterInTask(pOwnerTask);
}

CStateMachine::~CStateMachine()
{
  UnregisterInTask();
}

// Virtual.
const char* CStateMachine::NameOf() const
{
  return GetCompileType();
}

// Virtual.
void CStateMachine::HandleEvent(CSegment* msg, DWORD, OPCODE opcode)
{
  DispatchEvent(opcode, msg);
}

void CStateMachine::UnregisterInTask()
{
  if (!m_suspended)
    DeleteAllTimers();

  CTaskApp* task = COsTask::GetTaskApp();
  if (task && task != m_OwnerTaskApp)
  {
    // State machine must be created and destroyed in the same task.
    SystemCoreDump(TRUE, TRUE);
  }

  if (m_OwnerTaskApp && m_OwnerTaskApp != this && m_StateMachineHandle)
  {
    m_OwnerTaskApp->DeleteStateMachine(m_StateMachineHandle, this);
    m_StateMachineHandle = 0;
  }
}

void CStateMachine::RegisterInTask(CTaskApp* myNewTask)
{
  CTaskApp* task = myNewTask ? myNewTask : COsTask::GetTaskApp();

  if (task)
  {
    m_StateMachineHandle = task->AddStateMachine(this);
    m_OwnerTaskApp = task;
  }
  else
  {
    m_OwnerTaskApp = NULL;
    m_StateMachineHandle = 0;
  }
}

void CStateMachine::Dump(std::ostream& out) const
{
  for (const PMSG_MAP* mm = GetMessageMap(); mm != mm->m_base; mm = mm->m_base)
  {
    out << "\n " << mm->m_name << "\n";
    mm->m_index.Dump(out);
  }
}

WORD CStateMachine::GetState() const
{
  return m_state;
}

const char* CStateMachine::GetOpcodeAsString(OPCODE event) const
{
  for (const PMSG_MAP* mm = GetMessageMap(); mm != mm->m_base; mm = mm->m_base)
  {
    for (const PMSG_MAP_ENTRY* entry = mm->m_entries; entry->actFunc != 0; entry++)
    {
      if (entry->event == event)
        return entry->eventStr;
    }
  }

  // Unknown event.
  std::ostringstream buf;
  buf << "#" << event;

  static std::string s;
  s = buf.str();

  return s.c_str();
}

const char* CStateMachine::GetStateAsString() const
{
  for (const PMSG_MAP* mm = GetMessageMap(); mm != mm->m_base; mm = mm->m_base)
  {
    for (const PMSG_MAP_ENTRY* entry = mm->m_entries; entry->actFunc != 0; entry++)
    {
      if (entry->state == m_state)
        return entry->stateStr;
    }
  }

  // Unknown state.
  std::ostringstream buf;
  buf << "#" << m_state;

  static std::string s;
  s = buf.str();

  return s.c_str();
}

void CStateMachine::ActivateActFunc(CSegment* msg, const PMSG_MAP_ENTRY* entry)
{
  (this->*entry->actFunc)(msg);
}

const PMSG_MAP_ENTRY* CStateMachine::GetEntryByEvent(OPCODE event) const
{
  for (const PMSG_MAP* mm = GetMessageMap(); mm != mm->m_base; mm = mm->m_base)
  {
    const PMSG_MAP_ENTRY* entry = mm->m_index.Find(event, m_state);
    if (NULL != entry)
     return entry;
  }

  return NULL;
}

// This function is currently used only by SipParty implementation,
// for not ASSERTING on non existing event in state machine.
BOOL CStateMachine::IsEventExist(OPCODE event) const
{
  TRACECOND_AND_RETURN_VALUE(m_suspended,
      NameOf() << " is suspended" << ", ignored event is "
               << GetOpcodeAsString(event),
      FALSE);

  const PMSG_MAP_ENTRY* entry = GetEntryByEvent(event);
  if (NULL != entry)
    return TRUE;

  return FALSE;
}

BOOL CStateMachine::DispatchEvent(OPCODE event, CSegment* msg)
{
  if (!m_isValid)
    ValidateMessageMap(m_isValid);

  TRACECOND_AND_RETURN_VALUE(m_suspended,
    NameOf() << " is suspended, ignored event is "
             << GetOpcodeAsString(event),
    FALSE);

  const PMSG_MAP_ENTRY* entry = GetEntryByEvent(event);
  PASSERTSTREAM_AND_RETURN_VALUE(NULL == entry && event != DESTROY,
    "GetEntryByEvent: "    << NameOf()                 <<
    " got invalid event: " << GetOpcodeAsString(event) << " (" << event  <<
    "), state: "           << GetStateAsString()       << " (" << m_state << ")",
    FALSE);

  if (NULL == entry)
    return FALSE;

  AddEventLogEntry(entry);

  // Gets time stamps before and after.
  TICKS bsystem;
  TICKS buser;
  TICKS breal = SystemGetTickCount();
  GetSelfCpuUsage(buser, bsystem);

  // Calls event callback.
  ActivateActFunc(msg, entry);

  TICKS asystem;
  TICKS auser;
  TICKS areal = SystemGetTickCount();
  GetSelfCpuUsage(auser, asystem);

  // Updates statistics at the entry.
  entry->user = entry->user + (auser - buser);
  entry->system = entry->system + (asystem - bsystem);
  entry->real = entry->real + (areal - breal);
  entry->msgCount++;

  return TRUE;
}

void CStateMachine::NullActionFunction(CSegment*)
{}

// Checks if two different events have same value.
void CStateMachine::ValidateMessageMap(WORD& flag)
{
  // State machine is validated already.
  if (flag)
    return;

  for (const PMSG_MAP* mm = GetMessageMap(); mm != mm->m_base; mm = mm->m_base)
  {
    if (!mm->m_index.Validate())
      TRACEINTO << mm->m_name << " failed validation";

    for (const PMSG_MAP* m2 = mm->m_base; m2 != m2->m_base; m2 = m2->m_base)
    {
      if (!mm->m_index.ValidateIntersection(m2->m_index))
        TRACEINTO << mm->m_name << " failed validation with " << m2->m_name;
    }
  }

  flag = 1;
}

void CStateMachine::StartTimer(OPCODE opcode, TICKS ticks, CSegment* pSeg)
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  CTaskApp* task = proc->GetCurrentTask();
  PASSERTSTREAM_AND_RETURN(NULL == task,
      "There is no task for opcode " << GetOpcodeAsString(opcode));

  CTimer* timer = task->m_pTimer;
  if (timer != NULL)
    timer->AddTimer(opcode, ticks, GetStateMachineDescriptor(), pSeg);
}

void CStateMachine::DeleteTimer(OPCODE opcode)
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  CTaskApp* task = proc->GetCurrentTask();

  // It is not a critical error on Delete
  TRACECOND_AND_RETURN(NULL == task,
      "There is no task for opcode " << GetOpcodeAsString(opcode));

  CTimer* timer = task->m_pTimer;
  if (NULL != timer)
    timer->DeleteTimers(GetStateMachineDescriptor(), opcode);
}

void CStateMachine::DeleteAllTimers()
{
  DeleteTimer(0);
}

BOOL CStateMachine::IsValidTimer(OPCODE opcode)
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN_VALUE(NULL == proc, FALSE);

  CTaskApp* task = proc->GetCurrentTask();
  PASSERTSTREAM_AND_RETURN_VALUE(NULL == task,
      "There is no task for opcode " << GetOpcodeAsString(opcode),
      FALSE);

  CTimer* timer = task->m_pTimer;
  if (NULL == timer)
    return FALSE;

  return timer->IsValid(GetStateMachineDescriptor(), opcode);
}

void CStateMachine::Suspend(WORD onoff)
{
  m_suspended = onoff ? TRUE : FALSE;
  TRACEINTO << NameOf() << ", suspend is " << (onoff ? "on" : "off");
}

void CStateMachine::AddEventLogEntry(const PMSG_MAP_ENTRY* entry)
{
  PASSERT_AND_RETURN(m_current_log_entry >= MAX_SM_LOG_ENTRYS);

  m_log[m_current_log_entry++].marker = '-';
  if (m_current_log_entry >= MAX_SM_LOG_ENTRYS)
    m_current_log_entry = 0;

  m_log[m_current_log_entry].marker = '*';
  m_log[m_current_log_entry].eventStr = entry->eventStr;
  m_log[m_current_log_entry].stateStr = entry->stateStr;
  m_log[m_current_log_entry].actFunc = (DWORD) &((entry->actFunc));
}

StateMachineDescriptor CStateMachine::GetStateMachineDescriptor() const
{
  return StateMachineDescriptor(this, m_StateMachineHandle);
}

// Virtual.
BOOL CStateMachineValidation::DispatchEvent(OPCODE event, CSegment* msg)
{
  TRACECOND_AND_RETURN_VALUE(!IsValidEvent(event),
    "Invalid event " << GetOpcodeAsString(event) << " for " << NameOf(),
    FALSE);

  TRACECOND_AND_RETURN_VALUE(!IsValidState(m_state),
    "Invalid state " << GetStateAsString() << " for " << NameOf(),
    FALSE);

  return CStateMachine::DispatchEvent(event, msg);
}

// Virtual.
WORD CStateMachineValidation::IsValidEvent(OPCODE event) const
{
  return TRUE;
}

// Virtual.
WORD CStateMachineValidation::IsValidState(WORD state) const
{
  return TRUE;
}

