//+========================================================================+
//                EpSimGKEndpointsTask.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//+========================================================================+

#ifndef __EPSIM_GK_ENDPOINTS_TASK_H__
#define __EPSIM_GK_ENDPOINTS_TASK_H__

#include "CSIDTaskApp.h"

class CGKeeperList;
class CCSSimTaskApi;

/////////////////////////////////////////////////////////////////////////////
//
//   SimCSEndpointsModule - Central Signaling Sim module Task
//
/////////////////////////////////////////////////////////////////////////////
class CSimGKEndpointsModule : public CCSIDTaskApp
{
CLASS_TYPE_1(CSimGKEndpointsModule, CCSIDTaskApp)
public:
			// Constructors
	CSimGKEndpointsModule();
	virtual ~CSimGKEndpointsModule();
	virtual const char* NameOf() const { return "CSimGKEndpointsModule";}

	eEPSimTaskType GetTaskType(void) const;

			// Initializations
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();

			// Operations
	void OnCSAPICommandToGK(CSegment* pParam);
//	void OnReceiveMbxCS(CSegment* pParam);
	void OnGKSelfRemoveRegistration(CSegment* pParam);
	void OnGuiGKCommand(CSegment* pParam);

protected:
			// Utilities
	void  InitTask(){;}
	BOOL  IsSingleton() const {return NO;}
	const char* GetTaskName() const { return "SimGKEndpointsTask"; }

protected:
			// Attributes
	//CCSSimTaskApi* m_pCSApi;
	CGKeeperList* m_GKList;

	PDECLAR_MESSAGE_MAP
};

#endif // __EPSIM_GK_ENDPOINTS_TASK_H__
