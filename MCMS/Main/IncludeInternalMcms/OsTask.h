#ifndef OS_TASK_H__
#define OS_TASK_H__

#include "PObject.h"

////////////////////////////////////////////////////////////////////////////
enum TASK_STATUS
{
	TS_CREATE,
	TS_INIT,
	TS_WAIT,
	TS_RUN,
	TS_SUSPEND,
	TS_DESTROY,
};

////////////////////////////////////////////////////////////////////////////
typedef void* (*ThreadStartRoutinePtr)(void*);

////////////////////////////////////////////////////////////////////////////
class CTaskApi;

class WorkerThread;
class CTaskApp;

class CProcessBase;

////////////////////////////////////////////////////////////////////////////
class COsTask : public CPObject
{
	friend class CTaskApi;
	friend class CTaskApp;
	friend class CProcessBase;

	CLASS_TYPE_1(COsTask, CPObject)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	COsTask();

	virtual ~COsTask();

	bool Create(CTaskApp* self);
	bool Create(WorkerThread* self, ThreadStartRoutinePtr wrapper);

	bool Delete();

	void SetStatus(TASK_STATUS ts)
	{ m_status = ts; }

	DWORD GetTaskId() const
	{ return m_id; }

public: // static interface

	static void Suspend();
	static void Exit();
	static void Join(DWORD id);

	static DWORD GetCurrentTaskId();

	static CTaskApp* GetTaskApp();

	static bool InitTaskKey();
	static bool DestroyTaskKey();

	static int SetTaskPrioirty(int p);

	static void SendSignal(DWORD destTaskId, int signal);

private:

	static void* StartWrapper(void* arg);

private:

	TASK_STATUS  m_status;
	THREAD       m_handle;
	DWORD        m_id;
};

#endif // OS_TASK_H__

