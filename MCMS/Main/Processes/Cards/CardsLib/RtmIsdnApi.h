// RtmIsdnApi.h: interface for the CRtmIsdnApi class.
//
//////////////////////////////////////////////////////////////////////

#ifndef RTMISDNAPI_H_
#define RTMISDNAPI_H_



typedef void(*RtmIsdn_ENTRY_POINT)(void*); 

#include "TaskApi.h"

/*
const WORD SOCKET_CONNECTED = 30001;
const WORD SOCKET_FAILED    = 30002;
const WORD SOCKET_DROPPED   = 30003;
const WORD SOCKET_WRITE     = 30004;
const WORD SOCKET_RCV_MSG   = 30005;
*/

extern "C" void RtmIsdnEntryPoint(void* appParam);


class CRtmIsdnApi : public CTaskApi  
{
CLASS_TYPE_1(CRtmIsdnApi,CTaskApi )

public:
	CRtmIsdnApi();
	virtual ~CRtmIsdnApi();
	virtual const char* NameOf() const { return "CRtmIsdnApi";}


	void Create(void (*entryPoint)(void*),const COsQueue& creatorRcvMbx);
};



#endif /*RTMISDNAPI_H_*/
