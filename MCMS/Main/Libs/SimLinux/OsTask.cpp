#include "OsTask.h"

#include "WorkerThread.h"
#include "TaskApp.h"

#include "SystemFunctions.h"

#include "Trace.h"
#include "TraceStream.h"

/////////////////////////////////////////////////////////////////////////////
#include <sys/types.h>
#include <linux/unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/signal.h>

/////////////////////////////////////////////////////////////////////////////
pthread_key_t thread_key = 0xffff;

/////////////////////////////////////////////////////////////////////////////
COsTask::COsTask()
	: m_status(TS_CREATE)
	, m_id(0)
{}

/////////////////////////////////////////////////////////////////////////////
bool COsTask::Create(CTaskApp* self)
{
	int rc = pthread_create(&m_handle, NULL, &COsTask::StartWrapper, reinterpret_cast<void*>(self));
	TRACECOND_AND_RETURN_VALUE_HELPER(rc, "code:" << rc, false, eLevelError, this);

	m_id = static_cast<DWORD>(m_handle);

	if (!self->IsSingleton())
		pthread_detach(m_handle);

	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool COsTask::Create(WorkerThread* self, ThreadStartRoutinePtr wrapper)
{
	int rc = pthread_create(&m_handle, NULL, wrapper, reinterpret_cast<void*>(self));
	TRACECOND_AND_RETURN_VALUE_HELPER(rc, "code:" << rc, false, eLevelError, this);

	m_id = static_cast<DWORD>(m_handle);
	pthread_detach(m_handle);

	return true;
}

/////////////////////////////////////////////////////////////////////////////
COsTask::~COsTask()
{
//	pthread_exit(NULL);
}

/////////////////////////////////////////////////////////////////////////////
void COsTask::Join(DWORD id)
{
	pthread_join(id, NULL);
}

/////////////////////////////////////////////////////////////////////////////
void* COsTask::StartWrapper(void* arg)
{
	CTaskApp* pTask = reinterpret_cast<CTaskApp*>(arg);
	pthread_setspecific(thread_key, arg);

	int priority = pTask->GetTaskPrioirty();
	COsTask::SetTaskPrioirty(priority);

	CTaskApp::DummyStartTask(pTask);

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
int COsTask::SetTaskPrioirty(int p)
{
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
DWORD COsTask::GetCurrentTaskId()
{
	return (DWORD)(pthread_self());
}

/////////////////////////////////////////////////////////////////////////////
void COsTask::Exit()
{
	pthread_exit(NULL);
}

/////////////////////////////////////////////////////////////////////////////
void COsTask::Suspend()
{
	for ( ; ; )
		SystemSleep(-1, false);
}

/////////////////////////////////////////////////////////////////////////////
bool COsTask::InitTaskKey()
{
	pthread_key_create(&thread_key, NULL);
	return true;
}
/////////////////////////////////////////////////////////////////////////////
bool COsTask::DestroyTaskKey()
{
	pthread_key_delete(thread_key);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
CTaskApp* COsTask::GetTaskApp()
{
	return reinterpret_cast<CTaskApp*>(pthread_getspecific(thread_key));
}

/////////////////////////////////////////////////////////////////////////////
void COsTask::SendSignal(DWORD destTaskId, int signal)
{
	pthread_kill(destTaskId, signal);
}

/////////////////////////////////////////////////////////////////////////////
