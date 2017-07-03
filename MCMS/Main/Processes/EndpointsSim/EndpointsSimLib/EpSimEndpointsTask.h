//+========================================================================+
//                   EpSimEndpointsTask.h                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpSimEndpointsTask.h                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef __EPSIMENDPOINTSTASK_H__
#define __EPSIMENDPOINTSTASK_H__


#include "TaskApp.h"


class CCapSetsList;
class CH323BehaviorList;
class CEndpointsList;


/// Task entry point
extern "C" void epSimEndpointsTaskEntryPoint(void* appParam);


/////////////////////////////////////////////////////////////////////////////
class CSimEndpointsTask : public CTaskApp
{
CLASS_TYPE_1(CSimEndpointsTask,CTaskApp)
public:
			// Constructors
	CSimEndpointsTask();
	virtual ~CSimEndpointsTask();
	virtual const char* NameOf() const { return "CSimEndpointsTask";}

			// Initializations
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();

			// Operations

protected:
			// Action functions
//	void OnReceiveMbxCsIdle(CSegment* pParam);
	void OnGideonArtMsgAll(CSegment* pParam);
	void OnGideonIsdnMsgAll(CSegment* pParam);
	void OnGideonMRMMsgAll(CSegment* pParam);
	void OnCSMsgAll(CSegment* pParam);
	void OnBatchCommandAll(CSegment* pParam);
	void OnGUIMsgAll(CSegment* pParam);

			// Utilities
	virtual void  SelfKill(); //Override
	void  InitTask(){;}
	BOOL  IsSingleton() const {return NO;}
	const char* GetTaskName() const { return "SimEndpointsTask"; }

protected:
			// Attributes
//	CTaskApi*			m_pCSApi;
	CCapSetsList*		m_pCapList;
	CH323BehaviorList*	m_pBehaviorList;
	CEndpointsList*		m_pEpList;

	PDECLAR_MESSAGE_MAP
};



#endif // __EPSIMENDPOINTSTASK_H__ 
