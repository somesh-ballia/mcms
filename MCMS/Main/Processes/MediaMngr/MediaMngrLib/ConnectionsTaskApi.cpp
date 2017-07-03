#include "ConnectionsTaskApi.h"
#include "ConnectionsTask.h"



/////////////////////////////////////////////////////////////////////////////

CConnectionsTaskApi::CConnectionsTaskApi()
{
}


/////////////////////////////////////////////////////////////////////////////

CConnectionsTaskApi::~CConnectionsTaskApi()
{
}

/////////////////////////////////////////////////////////////////////////////

void  CConnectionsTaskApi::Create(COsQueue& creatorRcvMbx)
{
	void (*entryPoint)(void*) = ConnectionsTaskEntryPoint;
	
	CTaskApi::Create(creatorRcvMbx); // set default stack param i.e. creator rsv mbx 
	
	//  put all connectionslist data
	//m_appParam	<< 7;

	LoadApp(entryPoint);
}

/////////////////////////////////////////////////////////////////////////////


void  CConnectionsTaskApi::Init()
{
	CSegment*  seg = new CSegment;

	//SendMsg(seg, INIT_LIST);
}
