//+========================================================================+
//                   GideonSimProcess.cpp                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimProcess.cpp                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

// GideonSimProcess.cpp: implementation of the CGideonSimProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "GideonSimProcess.h"

extern void GideonSimManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CGideonSimProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CGideonSimProcess::GetManagerEntryPoint()
{
	return GideonSimManagerEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGideonSimProcess::CGideonSimProcess()
{

}

CGideonSimProcess::~CGideonSimProcess()
{

}

