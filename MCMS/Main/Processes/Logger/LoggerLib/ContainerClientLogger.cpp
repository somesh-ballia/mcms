// ContainerClientLogger.cpp: implementation of the CContainerClientLogger class.
//
//////////////////////////////////////////////////////////////////////


#include <algorithm>
#include "ContainerClientLogger.h"
#include "TaskApi.h"
#include "OpcodesMcmsCommon.h"
#include "StatusesGeneral.h"
//#include "NStream.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CContainerClientLogger::CContainerClientLogger()
{

}

CContainerClientLogger::~CContainerClientLogger()
{
	RemoveAll();
}

void CContainerClientLogger::DispatchTrace(const char *buffer)
{
	VCOsQueueIterator itr		= begin();
	VCOsQueueIterator itrEnd	= end();

	while(itr != itrEnd)
	{
		COsQueue *queue = *itr;
		if(STATUS_OK != SendTrace(queue, buffer))
		{
			DeleteQueue(queue);
			*itr = NULL;
		}

		itr++;
	}

	erase(std::remove(begin(), end(), (COsQueue*)NULL) , end());	
}

void CContainerClientLogger::RemoveClient(char *uniqueName)
{
	VCOsQueueIterator itr		= begin();
	VCOsQueueIterator itrEnd	= end();

	while(itr != itrEnd)
	{
		COsQueue *queue = *itr;
		if(0 == strcmp(queue->GetName(), uniqueName))
		{
			erase(itr);
			DeleteQueue(queue);
			return;
		}
		
		itr++;
	}	
}

void CContainerClientLogger::RemoveAll()
{
	VCOsQueueIterator itr		= begin();
	VCOsQueueIterator itrEnd	= end();

	while(itr != itrEnd)
	{
		COsQueue *queue = *itr;
		DeleteQueue(queue);
		itr++;
	}
	clear();
}

STATUS CContainerClientLogger::SendTrace(COsQueue *queue, const char *buffer)
{
	CTaskApi api;
	api.CreateOnlyApi(*queue);

	CSegment *seg = new CSegment;
	*seg << buffer;

	STATUS res =  api.SendMsg(seg,TERMINAL_COMMAND);
	return res;
}

void CContainerClientLogger::DeleteQueue(COsQueue *& queue)
{
	queue->Delete();
	PDELETE(queue); 
}
