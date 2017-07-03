#ifndef TASK_API_H_
#define TASK_API_H_

#include "Segment.h"
#include "StateMachine.h"
#include "MessageType.h"
#include "DataTypes.h"
#include "TaskApp.h"
#include "ProcessBase.h"
#include "Macros.h"

////////////////////////////////////////////////////////////////////////////
typedef void (*TASK_ENTRY_POINT)(void*);

////////////////////////////////////////////////////////////////////////////
class COsQueue;

////////////////////////////////////////////////////////////////////////////
class CTaskApi : public CPObject
{
	CLASS_TYPE_1(CTaskApi, CPObject)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

	virtual void Dump(WORD i = 0) const;

public: // static interface

	static void StaticSendLocalMessage(CSegment* msg, OPCODE opcode);
	static void StaticSendLocalOpcodeMsg(OPCODE opcode);

	static STATUS SendMsgWithTrace(
		eProcessType ptype,
		eOtherProcessQueueEntry qtype,
		CSegment* msg,
		OPCODE opcode);

public:

	CTaskApi();
	CTaskApi(eProcessType process, eOtherProcessQueueEntry queueType);
	CTaskApi(const CTaskApi& other);

	virtual ~CTaskApi();

private: // intentionally unimplemented

	CTaskApi& operator =(const CTaskApi&);

public:

	void LoadApp(TASK_ENTRY_POINT entryPoint);

	// Creates api to an existing task.rcvMbx is task's mailbox
	void CreateOnlyApi(
		const COsQueue& rcvMbx,
		CStateMachine* pStateMachine = NULL,
		LocalQueue* pLocalRcvMbx = NULL,
		WORD syncCall = 0,
		DWORD ClientRspMsgSeqNum = 0);

	void CreateOnlyApi(
		const COsQueue& rcvMbx,
		const StateMachineDescriptor& stateMachineDesc,
		LocalQueue* pLocalRcvMbx = NULL,
		WORD syncCall = 0,
		DWORD ClientRspMsgSeqNum = 0);

	void SetLocalMbx(LocalQueue* localRcvMbx)
	{ m_pLocalRcvMbx = localRcvMbx; }

	// Destroys the api object but not the task app.
	void DestroyOnlyApi();

	// Creates api instance togethher with a task app instance.
	void Create(const COsQueue& creatorRcvMbx, WORD syncCall = 0);

	// Destroys api instance and delets task app.
	void Destroy(const char* reason = "");
	void SyncDestroy(const char* reason = "", BOOL free_semaphre = FALSE);
	void SyncDestroyTaskID(DWORD m_id, BOOL free_semaphre);

	// Operations
	CTaskApp* GetTaskAppPtr()
	{ return m_pTaskApp; }

	bool TaskExists() const;

	const COsQueue& GetRcvMbx() const
	{ return *m_pRcvMbx; }

	const LocalQueue& GetLocalRcvMbx() const
	{ return *m_pLocalRcvMbx; }

	STATUS GetLastSendError() const
	{ return m_lastSendError; }

	STATUS SendMsg(
		CSegment* msg,
		OPCODE opcode,
		const COsQueue* myQueue = NULL,
		const StateMachineDescriptor* myStateMachineDesc = NULL,
		WORD /*numOfRetries*/ = 4,
		OPCODE /*sub_opcode*/ = 0) const
	{ return Send(msg, opcode, NULL, myQueue, myStateMachineDesc, true); }

	STATUS SendMsgWithStateMachine(CSegment* msg, OPCODE opcode) const;
	STATUS SendMessageSync(CSegment* msg, OPCODE opcode, TICKS tout) const;
	STATUS SendMessageSync(CSegment* msg, OPCODE opcode, TICKS tout, OPCODE& respondOpcode) const;
	STATUS SendMessageSync(CSegment* msg, OPCODE opcode, TICKS tout, OPCODE& respondeOpcode, CSegment& pRspMsg) const;

	STATUS SendOpcodeMsg(OPCODE opcode) const
	{ return SendMsg(NULL, opcode); }

	STATUS SendSyncOpcodeMsg(OPCODE opcode, TICKS tout, OPCODE& outRespOpcode) const;

	void SendLocalMessage(CSegment* msg, OPCODE opcode) const;

	void SendLocalOpcodeMsg(OPCODE opcode) const
	{ SendLocalMessage(NULL, opcode); }

public: // virtual interface

	virtual void Create(TASK_ENTRY_POINT entryPoint, const COsQueue& creatorRcvMbx);

	virtual void ObserverUpdate(
		WORD event,
		WORD type,
		DWORD observerInfo1,
		DWORD val,
		void* pPointer = NULL);

protected:

	STATUS Send(
		CSegment* msg,
		OPCODE opcode,
		const StateMachineDescriptor* pStateMachineDesc,
		const COsQueue* myQueue = NULL,
		const StateMachineDescriptor* pMyStateMachine = NULL,
		bool assert_on_would_block = true) const;

public:

	StateMachineDescriptor m_StateMachineDesc;

protected:

	CSegment          m_appParam;
	DWORD             m_ClientRspMsgSeqNum;
	COsQueue*         m_pRspMbx;
	LocalQueue*       m_pLocalRcvMbx;
	CTaskApp*         m_pTaskApp;

	mutable COsQueue* m_pRcvMbx;
	mutable STATUS    m_lastSendError;
private:

	mutable eProcessType            m_process;
	mutable eOtherProcessQueueEntry m_entry;
};

#endif  // TASK_API_H_
