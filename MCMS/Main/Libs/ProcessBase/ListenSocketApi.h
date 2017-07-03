#ifndef LISTEN_SOCKET_API__
#define LISTEN_SOCKET_API__

/////////////////////////////////////////////////////////////////////////////
#include "TaskApi.h"
#include "ListenSocket.h"

#include <netinet/in.h>

/////////////////////////////////////////////////////////////////////////////
class COsQueue;

/////////////////////////////////////////////////////////////////////////////
typedef TASK_ENTRY_POINT SOCKET_ENTRY_POINT;

/////////////////////////////////////////////////////////////////////////////
class CListenSocketApi : public CTaskApi
{
	CLASS_TYPE_1(CListenSocketApi, CTaskApi)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public: 

	CListenSocketApi(
		SOCKET_ENTRY_POINT rx,
		SOCKET_ENTRY_POINT tx,
		WORD port,
		DWORD ip = INADDR_ANY,
		const char* interface = NULL);

	CListenSocketApi(
		SOCKET_ENTRY_POINT rx,
		SOCKET_ENTRY_POINT tx,
		WORD port,
		const char* ip);

	CListenSocketApi(
		SOCKET_ENTRY_POINT rx,
		SOCKET_ENTRY_POINT tx,
		const mcTransportAddress& address,
		const char* interface = NULL);

	~CListenSocketApi();

	void Create(COsQueue& creatorRcvMbx);

	void SetCreateConnectionMode(ServerConnectionModeEnum mode)
	{ createMode_ = mode; }

	void SetMaxNumConnections(size_t num)
	{ maxConnections_ = num; }

protected:

	SOCKET_ENTRY_POINT rxEntry_;
	SOCKET_ENTRY_POINT txEntry_;

	mcTransportAddress address_;

	const char* interface_;

	ServerConnectionModeEnum createMode_;

	size_t maxConnections_;
};

/////////////////////////////////////////////////////////////////////////////
#endif // LISTEN_SOCKET_API__
