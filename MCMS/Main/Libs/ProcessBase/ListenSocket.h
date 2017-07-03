#ifndef LISTEN_SOCKET_H__
#define LISTEN_SOCKET_H__

/////////////////////////////////////////////////////////////////////////////
#include "SocketApi.h"
#include "TaskApp.h"

#include "OsSocketListener.h"

#include <vector>

/////////////////////////////////////////////////////////////////////////////
enum ListenSocketStateEnum
{
	LSTN_SOCKET_IDLE,
	LSTN_SOCKET_ACTIVE,
};

/////////////////////////////////////////////////////////////////////////////
enum ServerConnectionModeEnum
{
	eTxRxConnection,
	eRxConnection,
	eTxConnection
};

/////////////////////////////////////////////////////////////////////////////
class COsSocketConnected;
class CPairOfSockets;

/////////////////////////////////////////////////////////////////////////////
typedef std::vector<int> INTVECTOR;
typedef std::vector<CPairOfSockets> SOCKETSVECTOR;
typedef std::vector<DWORD> CIpVector;

/////////////////////////////////////////////////////////////////////////////
extern "C" void listenSocketEntryPoint(void* appParam);

/////////////////////////////////////////////////////////////////////////////
class CListenSocket : public CTaskApp
{
	CLASS_TYPE_1(CListenSocket, CTaskApp)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	enum { DEFAULT_MAX_NUM_CONNECTIONS = 40 };

	CListenSocket();
	virtual ~CListenSocket();

	void Create(CSegment& appParam);

	COsQueue* GetTxMbx(WORD conId);

protected: // Overrides

	virtual BOOL IsSingleton() const
	{ return false; }

	virtual const char* GetTaskName() const
	{ return NameOf(); }

	virtual int GetPrivateFileDescriptor()
	{ return socketListener_ ? socketListener_->m_descriptor : -1; }

	virtual void InitTask();
	virtual void SelfKill();
	virtual void HandlePrivateFileDescriptor();

protected:

	void CreateRxTxTasks(const COsSocketConnected& connected);
	void CreateRxTasksOnly(const COsSocketConnected& connected);

	void SetRestrictions(size_t size, int threshold);

	SOCKETSVECTOR& GetSockectsVector();

	void AddtoVectorEx(
		const COsQueue& rcv_task_mailslot,
		const COsQueue& trx_task_mailslot,
		const COsSocketConnected & connected,
		DWORD rx_pid = 0, DWORD tx_pid = 0,
		bool updatePIDs = false);

	void RemoveFromVector(CSegment* pMsg);
	void RemoveFromVectorAfterKillBoth(CSegment* pMsg);

	void OnSocketDroppedAnycase(CSegment* pParam);
	void OnSocketCorruptedMsg(CSegment* pParam);

	void ShowVector();

	STATUS SendMessageToCreator(CSegment* pSeg, OPCODE opcode);

private:

	bool IsIpBlocked(DWORD remoteIp);
	void IsKnownIp(DWORD remoteIp, int newFD);

private:

	COsSocketListener* socketListener_;

	SOCKET_ENTRY_POINT rxEntryPoint_;
	SOCKET_ENTRY_POINT txEntryPoint_;

	int connectionID_;

	mcTransportAddress address_;

	std::string m_interface;

	SOCKETSVECTOR socketsVector_;

	size_t bufferSize_;
	int    bufferThreshold_;
	size_t maxConnections_;

	ServerConnectionModeEnum connectionMode_;

	CIpVector blockedIpVector_;

	PDECLAR_MESSAGE_MAP
};

/////////////////////////////////////////////////////////////////////////////
#endif // LISTEN_SOCKET_H__
