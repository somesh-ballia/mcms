//+========================================================================+
//                  EpSimProxyEndpointsTask.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//+========================================================================+

#ifndef __EPSIMPROXYENDPOINTSTASK_H__
#define __EPSIMPROXYENDPOINTSTASK_H__

#include "CSIDTaskApp.h"

class CProxyConfList;
class CCSSimTaskApi;

/////////////////////////////////////////////////////////////////////////////
//
//   CSimProxyEndpointsModule - Proxy module Task
//
/////////////////////////////////////////////////////////////////////////////
class CSimProxyEndpointsModule : public CCSIDTaskApp
{
CLASS_TYPE_1(CSimProxyEndpointsModule, CCSIDTaskApp)
public:
			// Constructors
	CSimProxyEndpointsModule();
	virtual ~CSimProxyEndpointsModule();
	virtual const char* NameOf() const { return "CSimProxyEndpointsModule";}

	eEPSimTaskType GetTaskType(void) const;

			// Initializations
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();

			// Operations
	void OnCSAPICommandToProxy(CSegment* pParam);
//	void OnReceiveMbxCS(CSegment* pParam);
	void OnProxySelfRemoveConf(CSegment* pParam);

protected:
			// Utilities
	void  InitTask(){;}
	BOOL  IsSingleton() const {return NO;}
	const char* GetTaskName() const { return "SimProxyEndpointsTask"; }

protected:
			// Attributes
	//CCSSimTaskApi* m_pCSApi;
	CProxyConfList* m_proxyList;

	PDECLAR_MESSAGE_MAP
};

#endif // __EPSIMPROXYENDPOINTSTASK_H__
