// CFailoverSyncApi.h: interface for the CFailoverSyncApi class.
//
//////////////////////////////////////////////////////////////////////

#ifndef FailoverSyncApi_H_
#define FailoverSyncApi_H_

#include "TaskApi.h"


class CFailoverSyncApi : public CTaskApi
{
CLASS_TYPE_1(FailoverSyncApi,CTaskApi )

public:
    							// Constructors
	CFailoverSyncApi() {};
	~CFailoverSyncApi() {};

	virtual const char*  NameOf() const {return "CFailoverSyncApi";}

};


#endif /* FailoverSyncApi_H_ */
