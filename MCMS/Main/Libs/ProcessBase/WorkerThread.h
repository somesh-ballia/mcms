#ifndef WORKER_THREAD_H__
#define WORKER_THREAD_H__

////////////////////////////////////////////////////////////////////////////
#include "OsTask.h"

////////////////////////////////////////////////////////////////////////////
class WorkerThread
{
public:

	enum State
	{
		s_Idle         = 0x0000,
		s_Initializing = 0xABCD,
		s_Running      = 0xCAFE,
		s_Terminating  = 0xDCBA,
		s_Zombie       = 0xBEAF,
		s_Destroyed    = 0xDEAD
	};

public:

	WorkerThread()
		: state_(s_Idle)
		, result_(0)
		, detached_(false)
	{}

protected:

	virtual ~WorkerThread()
	{ state_ = s_Destroyed; }

private: // intentionally unimplemented to prohibit copying

	WorkerThread(const WorkerThread&);
	void operator =(const WorkerThread&);

public:

	State state() const volatile
	{ return state_; }

	int result() const
	{ return result_; }

	bool run(bool detach = false);

	void terminate()
	{
		if (s_Running == state_)
			state_ = s_Terminating;
	}

private:

	virtual int threadRoutine() = 0;

private:

	void wrapper();

private:

	// This routine is invoked by OS
	// The actual job is done by a virtual function threadRoutine - to be defined by derived class
	static void* threadStartRoutine(void* data)
	{
		reinterpret_cast<WorkerThread*>(data)->wrapper();
		return NULL;
	}

private:

	volatile State state_;

	int  result_;

	bool detached_;

	COsTask osTask_;
};

////////////////////////////////////////////////////////////////////////////
#endif // WORKER_THREAD_H__
