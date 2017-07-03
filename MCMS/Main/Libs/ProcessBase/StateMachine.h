// StateMachine.h

#ifndef STATE_MACHINE_
#define STATE_MACHINE_

#include <map>
#include <vector>
#include <utility>

#include "Macros.h"
#include "PObject.h"
#include "SystemTick.h"
#include "MessageHeader.h"

class CStateMachine;
class COsQueue;
class CSegment;
class CTimerDesc;
class CTaskApp;
struct PMSG_MAP_ENTRY;
struct PMSG_MAP;

const WORD MAX_SM_LOG_ENTRYS = 20;
const WORD IDLE = 0;                // Default state.
const WORD ANYCASE = 0xFFFF;        // Any other state.

struct PEVENT_LOG_ENTRY
{
  char        marker;
  DWORD       actFunc;
  const char* eventStr;
  const char* stateStr;
};

class CMsgMapEntryIndex : CNonCopyable
{
  friend class EventPrintOut;

 public:
  explicit              CMsgMapEntryIndex(const PMSG_MAP_ENTRY* entry);
  const PMSG_MAP_ENTRY* Find(OPCODE event, WORD state) const;
  void                  Dump(std::ostream& stream) const;
  bool                  Validate() const;
  bool                  ValidateIntersection(const CMsgMapEntryIndex& rhs) const;

 private:
  typedef std::pair<OPCODE, WORD> Key;
  typedef const PMSG_MAP_ENTRY*   Val;
  typedef std::multimap<Key, Val> Map;

  Map m_index;
};

// Type of pointer to member function.
typedef void (CStateMachine::*AFUNC)(CSegment*);

struct PMSG_MAP_ENTRY
{
  OPCODE        event;     // Event opcode.
  WORD          state;
  AFUNC         actFunc;   // Action function.
  const char*   eventStr;
  const char*   stateStr;
  mutable TICKS user;
  mutable TICKS system;
  mutable TICKS real;
  mutable DWORD msgCount;
};

struct PMSG_MAP
{
  const char* m_name;
  const PMSG_MAP* m_base;
  const PMSG_MAP_ENTRY* m_entries;
  const CMsgMapEntryIndex& m_index;
};

#define PDECLAR_MESSAGE_MAP \
  static WORD              m_isValid; \
  static PMSG_MAP_ENTRY    m_msgEntries[]; \
  static CMsgMapEntryIndex m_msgMapIndex; \
  static PMSG_MAP          m_msgMap; \
  virtual const PMSG_MAP*  GetMessageMap() const;

#define PBEGIN_MESSAGE_MAP(a) \
  WORD           a::m_isValid = 0; \
  PMSG_MAP_ENTRY a::m_msgEntries[] = {

#define ONEVENT(event, state, actFunc) \
  { event, state, FUNC actFunc, #event, #state, 0, 0, 0, 0 },

#define PEND_MESSAGE_MAP(a,b) \
  { 0, 0, 0, "UNKNOWN", "UNKNOWN", 0, 0, 0, 0 } }; \
  CMsgMapEntryIndex a::m_msgMapIndex(a::m_msgEntries); \
  PMSG_MAP a::m_msgMap = { #a, &b::m_msgMap, a::m_msgEntries, m_msgMapIndex }; \
  const PMSG_MAP* a::GetMessageMap() const \
  { \
    return &a::m_msgMap; \
  }

#define STATEASSTRING GetStateAsString()

// Deprecated, do not use it.
#define VALIDATEMESSAGEMAP ValidateMessageMap(m_isValid);

class CStateMachine : public CPObject
{
  CLASS_TYPE_1(CStateMachine, CPObject)

 public:
                         CStateMachine(CTaskApp* owner = NULL);
  virtual               ~CStateMachine();
  virtual const char*    NameOf() const;
  virtual void           UnregisterInTask();
  virtual void           RegisterInTask(CTaskApp* myNewTask = NULL);
  virtual void           NullActionFunction(CSegment* pParam);
  virtual void           StartTimer(OPCODE type, TICKS ticks, CSegment* pSeg = NULL);
  virtual void           DeleteTimer(OPCODE type);
  virtual void           DeleteAllTimers();
  virtual BOOL           IsValidTimer(OPCODE type);
  virtual void           HandleEvent(CSegment* msg, unsigned int len, OPCODE opcode);
  virtual void           Dump(std::ostream& stream) const;
  virtual BOOL           DispatchEvent(OPCODE event, CSegment* pParam = NULL);
  void                   ValidateMessageMap(WORD& flag);
  void                   Suspend(WORD onoff = 1);
  WORD                   GetState() const;
  const char*            GetOpcodeAsString(OPCODE opcode) const;
  const char*            GetStateAsString() const;
  DWORD                  GetHandle() const {return m_StateMachineHandle;}
  StateMachineDescriptor GetStateMachineDescriptor() const;
  BOOL                   IsEventExist(OPCODE event) const;

 protected:
  void                   AddEventLogEntry(const PMSG_MAP_ENTRY* entry);
  void                   ActivateActFunc(CSegment* pParam, const PMSG_MAP_ENTRY* entry);

  WORD             m_state;
  WORD             m_suspended;
  BYTE             m_current_log_entry;
  DWORD            m_StateMachineHandle;
  CTaskApp*        m_OwnerTaskApp;
  PEVENT_LOG_ENTRY m_log[MAX_SM_LOG_ENTRYS];

  PDECLAR_MESSAGE_MAP;

 private:
  const PMSG_MAP_ENTRY* GetEntryByEvent(OPCODE event) const;
};

class CStateMachineValidation : public CStateMachine
{
  CLASS_TYPE_1(CStateMachineValidation, CStateMachine);
 protected:
               CStateMachineValidation(CTaskApp* o = NULL) : CStateMachine(o) {}
  virtual BOOL DispatchEvent(OPCODE event, CSegment* pParam = NULL);
  virtual WORD IsValidEvent(OPCODE event) const;
  virtual WORD IsValidState(WORD state) const;
};

#endif
