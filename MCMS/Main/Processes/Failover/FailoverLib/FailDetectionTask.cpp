/*
 * FailDetectionTask.cpp
 *
 *  Created on: Aug 25, 2009
 *      Author: yael
 */

#include "FailDetectionTask.h"
#include "Trace.h"
#include "TraceStream.h"
#include "ConfigManagerApi.h"
#include "FailoverConfiguration.h"
#include "OpcodesMcmsInternal.h"
#include "FailoverDefines.h"
#include "psosxml.h"
#include "FailoverCommunication.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "StringsLen.h"
#include "XmlMiniParser.h"

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CFailDetectionTask)
	ONEVENT(MASTER_KEEP_ALIVE_TIMER_SEND,		ANYCASE,	CFailDetectionTask::OnTimerMasterKeepAliveSendTimeout)
	ONEVENT(MASTER_KEEP_ALIVE_TIMER_FAILURE,	ANYCASE,	CFailDetectionTask::OnTimerMasterKeepAliveFailureTimeout)
	ONEVENT(FAILOVER_SOCKET_RCV_IND,			ANYCASE,	CFailDetectionTask::OnSocketRcvInd)
	ONEVENT(FAILOVER_RESUME_SLAVE_TASK, 		ANYCASE,	CFailDetectionTask::OnFailureResumeSlaveTask)

PEND_MESSAGE_MAP(CFailDetectionTask,CStateMachine);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
extern "C" void FailDetectionEntryPoint(void* appParam)
{
	CFailDetectionTask*  pTaskApp = new CFailDetectionTask;
	pTaskApp->Create(*(CSegment*)appParam);
}


//////////////////////////////////////////////////////////////////////////////
CFailDetectionTask::CFailDetectionTask() // constructor
{
	PTRACE(eLevelInfoNormal, "CFailDetectionTask - CFailDetectionTask") ;
	
	m_pProcess = (CFailoverProcess*)(CProcessBase::GetProcess());

	DWORD detectionDuration = 10;
	CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if (pSysConfig)
	    pSysConfig->GetDWORDDataByKey(CFG_KEY_HOT_BACKUP_FAILURE_DETECTION_IN_SECONDS, detectionDuration);
	m_keepAliveFailureTimeout = detectionDuration * SECOND - MASTER_KEEP_ALIVE_SEND_TIMEOUT;
	m_isMasterDownSentToManager = false;
	m_mcuStateTransStr = NULL;
	m_isKeepAliveResponseReceived = false;
}

/////////////////////////////////////////////////////////////////////////////
CFailDetectionTask::~CFailDetectionTask() // destructor
{
	DEALLOCBUFFER(m_mcuStateTransStr);
	if ( IsValidTimer(MASTER_KEEP_ALIVE_TIMER_SEND) )
		DeleteTimer(MASTER_KEEP_ALIVE_TIMER_SEND);

	if ( IsValidTimer(MASTER_KEEP_ALIVE_TIMER_FAILURE) )
		DeleteTimer(MASTER_KEEP_ALIVE_TIMER_FAILURE);
}

/////////////////////////////////////////////////////////////////////////////
const char * CFailDetectionTask::GetTaskName() const
{
	return "CFailDetectionTask";
}

/////////////////////////////////////////////////////////////////////////////
const char*   CFailDetectionTask::NameOf()  const
{
	return "CFailDetectionTask";
}

/////////////////////////////////////////////////////////////////////////////
void CFailDetectionTask::InitTask()
{
	PTRACE(eLevelInfoNormal, "CFailDetectionTask - InitTask");
	StartFailureDetection();
}

////////////////////////////////////////////////////////////////////////////
void CFailDetectionTask::OnFailureResumeSlaveTask(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal, "CFailDetectionTask - OnFailureResumeSlaveTask");
	StartFailureDetection();
}

/////////////////////////////////////////////////////////////////////////////
void CFailDetectionTask::StartFailureDetection()
{
	m_isKeepAliveResponseReceived = true;
	m_isMasterDownSentToManager   = false;

	m_mcuStateTransStr = NULL;
	//InitMcuStateTransStr();

	StartTimer(MASTER_KEEP_ALIVE_TIMER_SEND, MASTER_KEEP_ALIVE_SEND_TIMEOUT);
}

/////////////////////////////////////////////////////////////////////////////
void CFailDetectionTask::InitMcuStateTransStr()
{
	DWORD mcuToken	= m_pProcess->GetMcuToken();
	DWORD msgId		= m_pProcess->GetAndIncreaseMsgId();
	
	CXMLDOMElement* pRootNode =  NULL;
	pRootNode = new CXMLDOMElement("TRANS_MCU");
	
	CXMLDOMElement* pTempNode;

	pTempNode = pRootNode->AddChildNode("TRANS_COMMON_PARAMS");
	pTempNode->AddChildNode("MCU_TOKEN", mcuToken);
	pTempNode->AddChildNode("MCU_USER_TOKEN", mcuToken);
	pTempNode->AddChildNode("MESSAGE_ID", msgId);
	pTempNode = pRootNode->AddChildNode("ACTION");
	pTempNode = pTempNode->AddChildNode("RMX_GET_STATE_EX");

	char ipStr[IP_ADDRESS_LEN];
	if(m_pProcess->GetMngmntInfo())//Fix Core ,the m_pFailoverCommunication created in the managerTask ,and should be deleted in this context VNGR-19696
		SystemDWORDToIpString((DWORD)(m_pProcess->GetMngmntInfo()->mngmntIpAddress),ipStr);

	pTempNode->AddChildNode("CLIENT_IP", ipStr);
	
	pRootNode->DumpDataAsLongStringEx(&m_mcuStateTransStr);

	if (m_mcuStateTransStr)
	{
		FTRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::InitMcuStateTransStr"
								<< "\n" << m_mcuStateTransStr;
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::InitMcuStateTransStr"
							   << "\nm_mcuStateTransStr = NULL";
	}

	PDELETE(pRootNode);
}

/////////////////////////////////////////////////////////////////////////////
void CFailDetectionTask::OnTimerMasterKeepAliveSendTimeout(CSegment* pParam)
{
	TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::OnTimerMasterKeepAliveSendTimeout";
	
	if ( (false == m_isKeepAliveResponseReceived) &&
		 (!IsValidTimer(MASTER_KEEP_ALIVE_TIMER_FAILURE)) ) // the failureTimer was not activated yet
	{
		StartTimer(MASTER_KEEP_ALIVE_TIMER_FAILURE, m_keepAliveFailureTimeout);
		TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::OnTimerMasterKeepAliveSendTimeout - timer is up (for " << m_keepAliveFailureTimeout/SECOND << " sec)";
	}
	
	// ===== 1. send the transaction
	SendMcuStateTrans();
	
	// ===== 2. keep on polling KeepAlive requests
	m_isKeepAliveResponseReceived = false;
	StartTimer(MASTER_KEEP_ALIVE_TIMER_SEND, MASTER_KEEP_ALIVE_SEND_TIMEOUT);
}

/////////////////////////////////////////////////////////////////////////////
void CFailDetectionTask::OnSocketRcvInd(CSegment* pParam)
{
	DWORD len = 0;
	TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::OnSocketRcvInd (meaning: KeepAlive was answered)";

	/*reset the local variables*/
	m_isKeepAliveResponseReceived = true;
	m_isMasterDownSentToManager   = false;

	if ( IsValidTimer(MASTER_KEEP_ALIVE_TIMER_FAILURE) )
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::OnReceivingKeepAliveResponse - Failure Timer is deleted";
		DeleteTimer(MASTER_KEEP_ALIVE_TIMER_FAILURE);
	}
	/*analysis of the response is required */
	*pParam >> len;
	ALLOCBUFFER(pXMLString, len+1);
	*pParam >> pXMLString;
	pXMLString[len] = '\0';

	if (strstr(pXMLString, "RMX_GET_STATE_EX") )
	{
		if (CXmlMiniParser::GetResponseStatus(pXMLString) == STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::OnSocketRcvInd (meaning: KeepAlive was answered) with status ok";
			if (strstr(pXMLString, "<FAILOVER_TRIGGER>true</FAILOVER_TRIGGER>")) 
			{
				TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::OnSocketRcvInd: Failover trigger activated on Master";
				/*Master's trigger activated, we need to failover ASAP*/
				SendMasterDownIndToManager();
			}
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::OnSocketRcvInd (meaning: KeepAlive was answered) with status NOT ok!!!";
		    CManagerApi api(eProcessFailover);
			api.SendOpcodeMsg(FAILOVER_RESTART_SLAVE);	
		}
	}
	DEALLOCBUFFER(pXMLString);
	return;
	
}

/////////////////////////////////////////////////////////////////////////////
void CFailDetectionTask::OnTimerMasterKeepAliveFailureTimeout(CSegment* pParam)
{
	if (true == m_isKeepAliveResponseReceived)
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::OnTimerMasterKeepAliveFailureTimeout"
							   << "\nm_isKeepAliveResponseReceived is true, meaning it's ok now";		
	}
	
	else if (m_isMasterDownSentToManager == false)
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::OnTimerMasterKeepAliveFailureTimeout"
							   << "\nNo KeepAlive response received; sending 'MasterDown' indication";		

		SendMasterDownIndToManager();
	}

	else
		TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::OnTimerMasterKeepAliveFailureTimeout"
							   << " - m_isMasterDownSentToManager is true";
}

/////////////////////////////////////////////////////////////////////////////
void CFailDetectionTask::SendMasterDownIndToManager()
{
	TRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::SendMasterDownIndToManager";

	m_isMasterDownSentToManager = true;

    CManagerApi api(eProcessFailover);
	api.SendOpcodeMsg(MASTER_DOWN_IND);
}

/////////////////////////////////////////////////////////////////////////////
void CFailDetectionTask::SendMcuStateTrans()
{
	InitMcuStateTransStr();
	//Send trans;
	if (m_mcuStateTransStr)
	{
		FTRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::SendMcuStateTrans - sent";

		CFailoverCommunication *m_pComm = m_pProcess->GetFailoverCommunication();
		if (m_pComm)
		{
			m_pComm->SendToSocket(m_mcuStateTransStr);
		}
		else // m_pComm is NULL
		{
			FTRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::SendMcuStateTrans - FAIL - FailoverCommunication from process is NULL";
		}
	}

	else // m_mcuStateTransStr is empty
	{
		FTRACESTR(eLevelInfoNormal) << "\nCFailDetectionTask::SendMcuStateTrans - FAIL - str empty";
	}
	DEALLOCBUFFER(m_mcuStateTransStr);
}
