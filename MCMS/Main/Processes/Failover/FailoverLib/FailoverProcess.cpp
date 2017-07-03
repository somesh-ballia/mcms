// FailoverProcess.cpp: implementation of the CFailoverProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "FailoverProcess.h"
#include "FailoverConfiguration.h"
#include "SystemFunctions.h"
#include "FailoverCommunication.h"
#include "StringsMaps.h"


extern void FailoverManagerEntryPoint(void* appParam);



//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CFailoverProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CFailoverProcess::GetManagerEntryPoint()
{
	return FailoverManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CFailoverProcess::CFailoverProcess()
{
	PTRACE(eLevelInfoNormal,"CFailoverProcess - CFailoverProcess");

	m_mcuToken	= 0xffff;
	m_msgId		= FIRST_MSG_ID;

	m_pFailDetectionTaskMbx	= NULL;
	m_pFailoverSyncTaskMbx	= NULL;

	m_pFailoverConfiguration	= new CFailoverConfiguration();
	m_pFailoverCommunication	= NULL;

	m_pMngmntInfo = NULL;
	m_isRequestPeerCertificate = FALSE;
}

//////////////////////////////////////////////////////////////////////
CFailoverProcess::~CFailoverProcess()
{
	POBJDELETE(m_pFailoverConfiguration);
	//POBJDELETE(m_pFailoverCommunication); Fix Core ,the m_pFailoverCommunication created in the managerTask ,and should be deleted in this context VNGR-19696
	//POBJDELETE(m_pMngmntInfo); Fix Core ,the m_pFailoverCommunication created in the managerTask ,and should be deleted in this context VNGR-19696
}

//////////////////////////////////////////////////////////////////////
CFailoverConfiguration* CFailoverProcess::GetFailoverConfiguration() const
{
	return m_pFailoverConfiguration;
}

//////////////////////////////////////////////////////////////////////
DWORD CFailoverProcess::GetMcuToken() const
{
	return m_mcuToken;
}


//////////////////////////////////////////////////////////////////////
void CFailoverProcess::AddExtraStringsToMap()
{
	CStringsMaps::AddItem(FAILOVER_STATUS_TYPE,eFailoverStatusNone,"none");
	CStringsMaps::AddItem(FAILOVER_STATUS_TYPE,eFail,"fail");
	CStringsMaps::AddItem(FAILOVER_STATUS_TYPE,eAttempting,"attempting");
	CStringsMaps::AddItem(FAILOVER_STATUS_TYPE,eSynchOk,"ok");

	CStringsMaps::AddItem(FAILOVER_TYPE,eFailoverTypeNone,"none");
	CStringsMaps::AddItem(FAILOVER_TYPE,eMaster,"master");
	CStringsMaps::AddItem(FAILOVER_TYPE,eSlave,"slave");

	CStringsMaps::AddItem(FAILOVER_EVENT_TRIGGER,eFailoverMgtPortFailure,"MANAGEMENT_PORT_FAILURE");
	CStringsMaps::AddItem(FAILOVER_EVENT_TRIGGER,eFailoverMpmCardFailure,"MPM_CARD_FAILURE");
	CStringsMaps::AddItem(FAILOVER_EVENT_TRIGGER,eFailoverSignalPortFailure,"SIGNAL_PORT_FAILURE");
	CStringsMaps::AddItem(FAILOVER_EVENT_TRIGGER,eFailoverIsdnCardFailure,"ISDN_CARD_FAILURE");
	CStringsMaps::AddItem(FAILOVER_EVENT_TRIGGER,eFailoverSwitchCardFailure,"SWITCH_CARD_FAILURE");
}


//////////////////////////////////////////////////////////////////////
void CFailoverProcess::SetMcuToken(DWORD mcuToken)
{
	m_mcuToken = mcuToken;
}

//////////////////////////////////////////////////////////////////////
DWORD CFailoverProcess::GetAndIncreaseMsgId()
{
	DWORD messageId = m_msgId;

	m_msgId++;
	if (0xffffffff <= m_msgId)
		m_msgId = FIRST_MSG_ID;

	return messageId;
}

//////////////////////////////////////////////////////////////////////
CFailoverCommunication* CFailoverProcess::GetFailoverCommunication() const
{
	return m_pFailoverCommunication;
}

//////////////////////////////////////////////////////////////////////
void CFailoverProcess::SetFailoverCommunication(CFailoverCommunication* pComm)
{
	m_pFailoverCommunication = pComm;
}

//////////////////////////////////////////////////////////////////////
MASTER_SLAVE_DATA_S* CFailoverProcess::GetMngmntInfo() const
{
	return m_pMngmntInfo;
}

//////////////////////////////////////////////////////////////////////
void CFailoverProcess::SetMngmntInfo(MASTER_SLAVE_DATA_S* pData)
{
	m_pMngmntInfo = pData;
}

//////////////////////////////////////////////////////////////////////
void CFailoverProcess::UpdateMngmntInfo(MASTER_SLAVE_DATA_S* pData)
{
	*m_pMngmntInfo = *pData;
}

//////////////////////////////////////////////////////////////////////
COsQueue* CFailoverProcess::GetFailDetectionTaskMbx() const
{
	return m_pFailDetectionTaskMbx;
}

//////////////////////////////////////////////////////////////////////
void CFailoverProcess::SetFailDetectionTaskMbx(COsQueue* pMbx)
{
	m_pFailDetectionTaskMbx = pMbx;
}

//////////////////////////////////////////////////////////////////////
COsQueue* CFailoverProcess::GetFailoverSyncTaskMbx() const
{
	return m_pFailoverSyncTaskMbx;
}

//////////////////////////////////////////////////////////////////////
void CFailoverProcess::SetFailoverSyncTaskMbx(COsQueue* pMbx)
{
	m_pFailoverSyncTaskMbx = pMbx;
}
