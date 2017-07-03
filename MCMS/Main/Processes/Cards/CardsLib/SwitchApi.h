// SwitchApi.h: interface for the CSwitchApi class.
//
//////////////////////////////////////////////////////////////////////
#ifndef SWITCHAPI_H_
#define SWITCHAPI_H_


typedef void(*Switch_ENTRY_POINT)(void*); 

#include "TaskApi.h"

/*
const WORD SOCKET_CONNECTED = 30001;
const WORD SOCKET_FAILED    = 30002;
const WORD SOCKET_DROPPED   = 30003;
const WORD SOCKET_WRITE     = 30004;
const WORD SOCKET_RCV_MSG   = 30005;
*/

extern "C" void switchEntryPoint(void* appParam);


class CSwitchApi : public CTaskApi  
{
CLASS_TYPE_1(CSwitchApi,CTaskApi )

public:
	CSwitchApi();
	virtual ~CSwitchApi();
	virtual const char* NameOf() const { return "CSwitchApi";}


	void Create(void (*entryPoint)(void*),const COsQueue& creatorRcvMbx);
};



#endif /*SWITCHAPI_H_*/
