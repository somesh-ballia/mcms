//+========================================================================+
//               GideonSimMonitor.cpp                                      |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimMonitor.cpp                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// GideonSimMonitor.cpp: implementation of the GideonSimMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "GideonSimMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CGideonSimMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CGideonSimMonitor::HandlePostRequest)
PEND_MESSAGE_MAP(CGideonSimMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CGideonSimMonitor)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CGideonSimMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void GideonSimMonitorEntryPoint(void* appParam)
{  
	CGideonSimMonitor *monitorTask = new CGideonSimMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGideonSimMonitor::CGideonSimMonitor()
{

}

CGideonSimMonitor::~CGideonSimMonitor()
{

}


