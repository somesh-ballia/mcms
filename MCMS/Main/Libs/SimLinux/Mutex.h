#ifndef MUTEX_H__
#define MUTEX_H__

#include <pthread.h>

////////////////////////////////////////////////////////////////////////////
template <class L>
class ScopeGuard;

class ConditionVariable;

////////////////////////////////////////////////////////////////////////////
class Mutex
{
	template <class L>
	friend class ScopeGuard;

	friend class ConditionVariable;

public:

	Mutex()
		: isLocked_(false)
	{
		pthread_mutex_init(&id_, NULL);
	}

	~Mutex()
	{
		FPASSERTMSG(isLocked_, "Destroying a locked mutex!");
		pthread_mutex_destroy(&id_);
	}

private:

	void lock()
	{
		pthread_mutex_lock(&id_);
		isLocked_ = true;
	}

	void unlock()
	{
		isLocked_ = false;
		pthread_mutex_unlock(&id_);
	}

private:

	pthread_mutex_t id_;
	bool            isLocked_;
};

////////////////////////////////////////////////////////////////////////////
class ConditionVariable
{
public:

	ConditionVariable()
	{ pthread_cond_init(&id_, NULL); }

	~ConditionVariable()
	{ pthread_cond_destroy(&id_); }

	void wait(Mutex& mutex)
	{ pthread_cond_wait(&id_, &mutex.id_); }

	void notify()
	{ pthread_cond_signal(&id_); }

	void broadcast()
	{ pthread_cond_broadcast(&id_);}

private:

	pthread_cond_t id_;
};

////////////////////////////////////////////////////////////////////////////
template <class L>
class ScopeGuard
{
public:

	ScopeGuard(L& lock)
		: lock_(lock)
	{
		lock_.lock();
	}

	~ScopeGuard()
	{
		lock_.unlock();
	}

private:

	L& lock_;
};

////////////////////////////////////////////////////////////////////////////
#endif // MUTEX_H__
