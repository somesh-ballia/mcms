#ifndef SOCKETAPI_H__
#define SOCKETAPI_H__

//////////////////////////////////////////////////////////////////////
#include "TaskApi.h"
#include "OpcodesMcmsCommon.h"

//////////////////////////////////////////////////////////////////////
typedef void (*SOCKET_ENTRY_POINT)(void*);

//////////////////////////////////////////////////////////////////////
const WORD SOCKET_CONNECTED     = 30001;
const WORD SOCKET_FAILED        = 30002;
const WORD SOCKET_DROPPED       = 30003;
const WORD SOCKET_WRITE         = 30004;
const WORD SOCKET_RCV_MSG       = 30005;
const WORD SOCKET_CONNECT       = 30006;
const WORD SOCKET_SND_MSG       = 30007;
const WORD RESUME_SOCKET        = 30008;
const WORD PAUSE_SOCKET         = 30009;
const WORD SOCKET_CORRUPTED_MSG = 30010;

//////////////////////////////////////////////////////////////////////
class COsSocketConnected;

//////////////////////////////////////////////////////////////////////
class CSocketApi : public CTaskApi
{
	CLASS_TYPE_1(CSocketApi, CTaskApi)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	CSocketApi();

	virtual ~CSocketApi();

public:

	void Create(
		SOCKET_ENTRY_POINT entryPoint,
		const COsQueue& creatorRcvMbx,
		const COsSocketConnected &,
		const COsQueue& twin,
		bool isSecured = false,
		int connectionId = -1);

	void KillByTwin()
	{ SendOpcodeMsg(KILLED_BY_TWIN); }

	void CloseSocketByTwin()
	{ SendOpcodeMsg(CLOSE_SOCKET_BY_TWIN); }

	void WriteSocket(CSegment* pMsg)
	{ SendMsg(pMsg, SOCKET_WRITE); }
};

//////////////////////////////////////////////////////////////////////
#endif // SOCKETAPI_H__
