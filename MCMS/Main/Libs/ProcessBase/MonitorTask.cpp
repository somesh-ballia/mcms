//+========================================================================+
//                        MonitorTask.cpp                                  |
//                     Copyright 2005 Polycom                              |
//                      All Rights Reserved.                               |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MonitorTask.cpp                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Haggai                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Sagi| 23.2.05    |                                                      |
//+========================================================================+

#include "MonitorTask.h"
#include "ProcessBase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CMonitorTask::CMonitorTask()
{
	m_type = "CMonitorTask";
}


//////////////////////////////////////////////////////////////////////
CMonitorTask::~CMonitorTask()
{
}


//////////////////////////////////////////////////////////////////////
const char * CMonitorTask::GetTaskName() const 
{
	return InfrastructuresTaskNames[eMonitor];
}

