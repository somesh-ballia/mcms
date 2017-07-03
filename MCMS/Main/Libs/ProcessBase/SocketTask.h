#ifndef SOCKET_TASK_H__
#define SOCKET_TASK_H__

//////////////////////////////////////////////////////////////////////
#include "TaskApp.h"
#include "Trace.h"

#include "OsSocketConnected.h"


#define TRACEINTO_SOCKET TRACESTR_HELPER(eLevelInfoNormal, this) << "_SOCKET_ FD:" << GetFD() << " " << trim_pretty_function(__PRETTY_FUNCTION__, ARRAYEND(__PRETTY_FUNCTION__))<< " - "
#define TRACEINTO_ERR_SOCKET TRACESTR_HELPER(eLevelError, this) << "_SOCKET_ FD:" << GetFD() << " " << trim_pretty_function(__PRETTY_FUNCTION__, ARRAYEND(__PRETTY_FUNCTION__))<< " - "
#define TRACEINTO_HIGH_SOCKET TRACESTR_HELPER(eLevelInfoHigh, this) << "_SOCKET_ FD:" << GetFD() << " " << trim_pretty_function(__PRETTY_FUNCTION__, ARRAYEND(__PRETTY_FUNCTION__))<< " - "


//////////////////////////////////////////////////////////////////////
const WORD OPEN_SOCKET_TIMER  = 111;

//////////////////////////////////////////////////////////////////////
class CSegment;

//////////////////////////////////////////////////////////////////////
class CSocketTask : public CTaskApp
{
	CLASS_TYPE_1(CSocketTask, CTaskApp)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	CSocketTask(COsSocketConnected* pSocketDesc = NULL);

	virtual ~CSocketTask();

	void Create(CSegment& appParam);

	void SetTwinMbx(const COsQueue& twinMailbox)
	{ *m_twinTask = twinMailbox; }

	void SetTwinSocketTask(CSocketTask* twinSocketTask)
	{ twinSocketTask_ = twinSocketTask; }

	int GetSocketConnectionId() const
	{ return m_socketConnectionId; }

	DWORD GetRemoteIp() const
	{ return m_connection ? m_connection->GetRemoteIp() : 0; }

	int GetFD() const { return m_connection ? m_connection->GetDescriptor() : -1; }

protected:

	void CloseSocket();

	void SendMsgToCreator(OPCODE opcode, CSegment* pSeg) const;

	void SendMessageToTwin(CSegment* pSeg, OPCODE opcode) const;

protected: // overrides

	virtual eTaskRecoveryPolicyAfterSeveralRetries GetTaskRecoveryPolicyAfterSeveralRetries() const
	{ return eDoNothing; }

	virtual BOOL TaskHandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	virtual void HandleDisconnect();
	virtual void SetSelfKill();

private:

	void SetSelfKillForTwinTask();

private: // actions

	void OnOpenSocketTimer(CSegment* pParam);

protected:

	COsSocketConnected* m_connection;

	COsQueue* m_twinTask;

	int  m_socketConnectionId;
	bool isSecured_;

	PDECLAR_MESSAGE_MAP

private:

	CSocketTask* twinSocketTask_;
};

//////////////////////////////////////////////////////////////////////
#endif // SOCKET_TASK_H__
