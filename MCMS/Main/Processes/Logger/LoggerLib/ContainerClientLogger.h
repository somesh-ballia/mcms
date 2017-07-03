// ContainerClientLogger.h: interface for the CContainerClientLogger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_ContainerClientLogger_H__)
#define _ContainerClientLogger_H__

#include <vector>
#include "DataTypes.h"


class COsQueue;

typedef std::vector<COsQueue*> VCOsQueue;
typedef VCOsQueue::iterator VCOsQueueIterator; 

class CContainerClientLogger : public VCOsQueue  
{
public:
	CContainerClientLogger();
	virtual ~CContainerClientLogger();

	void DispatchTrace(const char *buffer);
	void RemoveClient(char *uniqueName);
	void RemoveAll();

private:
	STATUS SendTrace(COsQueue *queue, const char *buffer);
	void DeleteQueue(COsQueue *& queue);
};

#endif // !defined(_ContainerClientLogger_H__)
