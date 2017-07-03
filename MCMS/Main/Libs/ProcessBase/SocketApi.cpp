#include "SocketApi.h"

#include "OsSocketConnected.h"
#include "OsQueue.h"

//////////////////////////////////////////////////////////////////////
CSocketApi::CSocketApi()
{}

//////////////////////////////////////////////////////////////////////
CSocketApi::~CSocketApi()
{}

//////////////////////////////////////////////////////////////////////
void CSocketApi::Create(
	SOCKET_ENTRY_POINT entryPoint,
	const COsQueue& creatorRcvMbx,
	const COsSocketConnected & connected,
	const COsQueue& twin,
	bool isSecured/* = false*/,
	int connectionId/* = -1*/)
{
	m_appParam << isSecured << (DWORD)connectionId;
	connected.Serialize(m_appParam);
	twin.Serialize(m_appParam);
	CTaskApi::Create(creatorRcvMbx);

	LoadApp(entryPoint);
}

//////////////////////////////////////////////////////////////////////
