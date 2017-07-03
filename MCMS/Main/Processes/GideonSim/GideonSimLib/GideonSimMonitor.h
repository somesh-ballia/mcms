//+========================================================================+
//                    GideonSimMonitor.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimMonitor.h                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// GideonSimMonitor.h: interface for the GideonSimMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_GIDEONSIMMONITOR__)
#define _GIDEONSIMMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CGideonSimMonitor : public CMonitorTask
{
CLASS_TYPE_1(CGideonSimMonitor,CMonitorTask )
public:
	CGideonSimMonitor();
	virtual ~CGideonSimMonitor();

	virtual const char* NameOf() const { return "CGideonSimMonitor";}
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
};

#endif // !defined(_GIDEONSIMMONITOR__)

