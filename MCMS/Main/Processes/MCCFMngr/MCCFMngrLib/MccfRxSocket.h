#ifndef MCCFRXSOCKET_H__
#define MCCFRXSOCKET_H__

//////////////////////////////////////////////////////////////////////
#include "SocketRxTask.h"
#include "SocketApi.h"

#include "MccfContext.h"

#include "MccfMsgFactory.h" // HMccfMessage

#include <queue>

//////////////////////////////////////////////////////////////////////
extern "C" void MccfRxEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
class COsSocketConnected;
class IMccfMessage;

//////////////////////////////////////////////////////////////////////
class CMccfRxSocket : public CSocketRxTask
{
	friend class CMccfContext;

	CLASS_TYPE_1(CMccfRxSocket, CSocketRxTask)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	virtual ~CMccfRxSocket();

	CMccfRxSocket(COsSocketConnected* pSocketDesc = NULL);

	HANDLE AppServerID() const
	{ return reinterpret_cast<HANDLE>(reinterpret_cast<size_t>(this)); }

	const COsQueue& twinMailbox() const
	{ return *m_twinTask; }

private: // CMccfContext's interface

	void OnSync() const;

private: // Actions

	void OnChannelSync(CSegment* pSeg);
	void OnChannelDrop(CSegment* pSeg);
	void OnMessageQueueTimer(CSegment* pSeg);

private: // helpers

	bool ReadHeader(std::string& buffer);
	bool ReadMessage(IMccfMessage& message);

	void EnqueueMessage(const HMccfMessage& hrequest);
	bool DispatchMessage(HMccfMessage& hrequest) const;
	void SendACK(const HMccfMessage& hrequest) const;

private: // overrides

	virtual BOOL IsSingleton() const
	{ return false; }

	virtual const char* GetTaskName() const
	{ return NameOf(); }

	virtual void InitTask()
	{}

	virtual void ReceiveFromSocket();

private:

	PDECLAR_MESSAGE_MAP

private:

	CMccfContext context_;

	struct StampedMessage
	{
		time_t       stamp;
		HMccfMessage hrequest;
	};

	typedef std::queue<StampedMessage> MessageQueue;
	MessageQueue messages_;
};

//////////////////////////////////////////////////////////////////////
#endif // MCCFRXSOCKET_H__
