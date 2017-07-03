// MfaApi.h: interface for the CMfaApi class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MFAAPI_H__)
#define _MFAAPI_H__

typedef void(*MFA_ENTRY_POINT)(void*); 

#include "TaskApi.h"

/*
const WORD SOCKET_CONNECTED = 30001;
const WORD SOCKET_FAILED    = 30002;
const WORD SOCKET_DROPPED   = 30003;
const WORD SOCKET_WRITE     = 30004;
const WORD SOCKET_RCV_MSG   = 30005;
*/

extern "C" void mfaEntryPoint(void* appParam);


class CMfaApi : public CTaskApi  
{
CLASS_TYPE_1(CMfaApi,CTaskApi )

public:
	CMfaApi();
	virtual ~CMfaApi();
	virtual const char* NameOf() const { return "CMfaApi";}


	void Create(void (*entryPoint)(void*),const COsQueue& creatorRcvMbx);
};

#endif // !defined(_MFAAPI_H__)
