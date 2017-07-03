#ifndef SOCKET_TX_TASK_H__
#define SOCKET_TX_TASK_H__

#include "SocketTask.h"

class COsSocketConnected;

class CSocketTxTask : public CSocketTask
{
	CLASS_TYPE_1(CSocketTxTask, CSocketTask)

public:

	CSocketTxTask(bool dropBlocked = false, COsSocketConnected* pSocketDesc = NULL);

	virtual const char* NameOf() const
	{ return GetCompileType(); }

protected:

	STATUS Write(const char* buffer, size_t size);

protected:

	bool m_dropBlocked;
};

#endif // SOCKET_TX_TASK_H__
