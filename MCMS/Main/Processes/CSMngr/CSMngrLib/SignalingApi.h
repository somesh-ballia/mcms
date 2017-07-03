// SignalingApi.h: interface for the CSignalingApi class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SIGNALINGAPI_H__)
#define _SIGNALINGAPI_H__

//typedef void(*SIGNALING_ENTRY_POINT)(void*); 

#include "TaskApi.h"

/*
const WORD SOCKET_CONNECTED = 30001;
const WORD SOCKET_FAILED    = 30002;
const WORD SOCKET_DROPPED   = 30003;
const WORD SOCKET_WRITE     = 30004;
const WORD SOCKET_RCV_MSG   = 30005;
*/

extern "C" void signalingEntryPoint(void* appParam);


class CSignalingApi : public CTaskApi  
{
CLASS_TYPE_1(CSignalingApi,CTaskApi )

public:
	CSignalingApi();
	virtual ~CSignalingApi();
	virtual const char* NameOf() const { return "CSignalingApi";}


	void Create(void (*entryPoint)(void*),const COsQueue& creatorRcvMbx);
};

#endif // !defined(_SIGNALINGAPI_H__)
