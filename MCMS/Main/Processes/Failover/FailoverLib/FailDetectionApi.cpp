/*
 * FailDetectionApi.cpp
 *
 *  Created on: Aug 25, 2009
 *      Author: yael
 */

#include "FailDetectionApi.h"
#include "OpcodesMcmsInternal.h"

//////////////////////////////////////////////////////////////////////////
const char* CFailDetectionApi::NameOf() const
{
 	return "CFailDetectionApi";
}

////////////////////////////////////////////////////////////////////////////
void CFailDetectionApi::ResumeSlaveTask()
{
	SendOpcodeMsg(FAILOVER_RESUME_SLAVE_TASK);
}
