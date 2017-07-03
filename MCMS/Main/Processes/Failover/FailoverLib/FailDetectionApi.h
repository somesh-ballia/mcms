/*
 * FailDetectionApi.h
 *
 *  Created on: Aug 25, 2009
 *      Author: yael
 */

#ifndef FAILDETECTIONAPI_H_
#define FAILDETECTIONAPI_H_

#include "TaskApi.h"

class CFailDetectionApi : public CTaskApi
{
CLASS_TYPE_1(FailDetectionApi,CTaskApi )
public:
    							// Constructors
	CFailDetectionApi() {};
	~CFailDetectionApi() {};

	//void  Create(void (*entryPoint)(void*),COsQueue& creatorRcvMbx);
	virtual const char*  NameOf() const;

	void ResumeSlaveTask();
};


#endif /* FAILDETECTIONAPI_H_ */
