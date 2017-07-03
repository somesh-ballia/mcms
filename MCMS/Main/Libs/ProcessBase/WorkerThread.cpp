#include "WorkerThread.h"

////////////////////////////////////////////////////////////////////////////
bool WorkerThread::run(bool detach/* = false*/)
{
	if (s_Idle == state_)
	{
		state_ = s_Initializing;

		if (!osTask_.Create(this, &threadStartRoutine))
		{
			FPASSERTMSG(true, "Task creation failed");
			state_ = s_Zombie;
		}

		detached_ = detach;
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////
void WorkerThread::wrapper()
{
	state_ = s_Running;
	result_ = threadRoutine();
	state_ = s_Zombie;

	if (detached_)
		delete this; // !!!
}

////////////////////////////////////////////////////////////////////////////
