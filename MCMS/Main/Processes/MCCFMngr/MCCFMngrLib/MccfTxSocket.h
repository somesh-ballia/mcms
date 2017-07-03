#ifndef __MCCFTXSOCKET_H__
#define __MCCFTXSOCKET_H__
//////////////////////////////////////////////////////////////////////
#include "SocketTxTask.h"

#include "MccfMsgFactory.h" // HMccfMessage

#include <string>
#include <set>

//////////////////////////////////////////////////////////////////////
extern "C" void MccfTxEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
class CMccfTxSocket : public CSocketTxTask
{
	CLASS_TYPE_1(CMccfTxSocket, CSocketTxTask)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	virtual ~CMccfTxSocket();

	CMccfTxSocket(COsSocketConnected* pSocketDesc = NULL);

private: // overrides

	virtual BOOL IsSingleton() const
	{ return false; }

	virtual const char* GetTaskName() const
	{ return NameOf(); }

	virtual void InitTask()
	{}

private: // Action functions

	void OnMccfChannelOpened(CSegment* pSeg);

	void OnAppServerACK(CSegment* pSeg);

	void OnMccfRequestDone(CSegment* pSeg);

	void OnIvrResponse(CSegment* pSeg);
	void OnCdrResponse(CSegment* pSeg);

	PDECLAR_MESSAGE_MAP

private:

	typedef std::set<HMccfMessage> Messages;

	Messages messages_;
};

//////////////////////////////////////////////////////////////////////
#endif // __MCCFTXSOCKET_H__
