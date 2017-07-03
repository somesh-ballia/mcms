// DispatcherTask.h: interface for the CDispatcherTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CS_API_DISPATCHERTASK_H__)
#define _CS_API_DISPATCHERTASK_H__

#include "DispatcherTask.h"


class CPairOfSockets;
class CCSApiProcess;
class CCSApiMplMcmsProtocolTracer;


class CCSApiDispatcherTask : public CDispatcherTask
{
CLASS_TYPE_1(CCSApiDispatcherTask,CDispatcherTask )
public:
	
	CCSApiDispatcherTask();
	virtual ~CCSApiDispatcherTask();


	virtual void InitTask();

	void*  GetMessageMap();
	void   OnBasicMsgToCSApi(CSegment* pMsg);

	BOOL         IsSingleton() const {return YES;}	
    CPairOfSockets* m_CS_Api_Card2SocketTable[MAX_NUM_OF_BOARDS];

private:
	virtual void AddFilterOpcodePoint();


	CCSApiProcess* m_pProcess;
	CCSApiMplMcmsProtocolTracer *m_CSApiMplMcmsProtocolTracer;
	
	PDECLAR_MESSAGE_MAP

};

#endif // !defined(_CS_API_DISPATCHERTASK_H__)
