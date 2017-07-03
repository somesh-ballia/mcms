//+========================================================================+
//                       DispatcherTask.cpp                                |
//                     Copyright 2005 Polycom                              |
//                      All Rights Reserved.                               |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       DispatcherTask.cpp                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sagi                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Sagi| 14.2.05    | Interprocess tasks                                   |
//+========================================================================+


#include "DispatcherTask.h"
#include "MessageHeader.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void dispatcherEntryPoint(void* appParam)
{
	CDispatcherTask * dispatcherTask = new CDispatcherTask(FALSE);
	dispatcherTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
void syncDispatcherEntryPoint(void* appParam)
{
	CDispatcherTask * dispatcherTask = new CDispatcherTask(TRUE);
	dispatcherTask->Create(*(CSegment*)appParam);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CDispatcherTask::CDispatcherTask(BOOL isSync)
{
    m_Thread_Group = eTaskGroupNone;
	m_isSync = isSync;
}


//////////////////////////////////////////////////////////////////////
CDispatcherTask::~CDispatcherTask()
{

}

//////////////////////////////////////////////////////////////////////
BOOL CDispatcherTask::Dispatcher(CMessageHeader& header)
{
    if (header.m_addressee.m_process ==
        CProcessBase::GetProcess()->GetProcessType() &&
        header.m_addressee.m_scope ==
        CProcessBase::GetProcess()->GetProcessType() &&
        header.m_addressee.m_idType == eWriteHandle)
    {
        // redirect the message to the right queue
        STATUS res = header.m_addressee.Send(header.m_segment,
                                             header.m_opcode,
                                             &(header.m_sender),
                                             &(header.m_stateMachine),
                                             &(header.m_senderStateMachine),
                                             eAsyncMessage,
                                             TRUE,
                                             header.m_RspMsgSeqNum);
        if (res != STATUS_OK)
        {
            POBJDELETE(header.m_segment);
        }
    }
    else
    {
        HandleOtherIdTypes(header);

        POBJDELETE(header.m_segment);
    }
    return TRUE;
}


//////////////////////////////////////////////////////////////////////
const char * CDispatcherTask::GetTaskName() const
{
	if (m_isSync)
	{
		return InfrastructuresTaskNames[eSyncDispatcher];
	}
	else
	{
		return InfrastructuresTaskNames[eDispatcher];
	}

}

