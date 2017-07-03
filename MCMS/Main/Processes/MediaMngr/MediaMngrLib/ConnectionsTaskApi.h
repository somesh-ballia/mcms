#ifndef CONNECTIONSTASKAPI_H_
#define CONNECTIONSTASKAPI_H_

#include "TaskApi.h"


//class COsQueue;

class CConnectionsTaskApi : public CTaskApi
{
CLASS_TYPE_1(CConnectionsTaskApi, CTaskApi)

public:
	
	// Constructors / Destructors
	CConnectionsTaskApi();
	virtual ~CConnectionsTaskApi();
	
	// Initializations

	// Operations
	void  Create(COsQueue& creatorRcvMbx);
	
	const char*  NameOf() const {return "CConnectionsTaskApi";}
	
	void Init();
	
};

#endif /*CONNECTIONSTASKAPI_H_*/
