// TestClientProcess.h: interface for the CTestClientProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TestClientPROCESS_H__)
#define _TestClientPROCESS_H__

#include "ProcessBase.h"

class CTestClientProcess : public CProcessBase  
{
CLASS_TYPE_1(CTestClientProcess,CProcessBase )	
public:
	friend class CTestSockets;
	friend class CTestTestClientProcess;
	friend class CTestGetRequest;

	CTestClientProcess();
	virtual ~CTestClientProcess();
	virtual eProcessType GetProcessType() {return eProcessTestClient;}
	virtual BOOL UsingSockets() {return YES;}
	virtual TaskEntryPoint GetManagerEntryPoint();
	virtual bool RequiresProcessInstanceForUnitTests() {return true;}

	DWORD m_ping_pong_count;

};

#endif // !defined(_TestClientPROCESS_H__)

