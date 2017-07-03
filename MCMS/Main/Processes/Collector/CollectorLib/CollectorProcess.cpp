// CollectorProcess.cpp: implementation of the CCollectorProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "CollectorProcess.h"
#include "SystemFunctions.h"
#include "CollectorInfo.h"
#include "StringsMaps.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"
#include "ManagerApi.h"

extern void CollectorManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CCollectorProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CCollectorProcess::GetManagerEntryPoint()
{
	return CollectorManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CCollectorProcess::CCollectorProcess()
{
	m_CollectingFileName="";
	m_pInfoTimeInterval = new CInfoTimeInterval;
	m_CollectingStatus = eCollectingStatus_ready;
	m_bCollectFilesStarted = FALSE;
}

//////////////////////////////////////////////////////////////////////
CCollectorProcess::~CCollectorProcess()
{
	PDELETE(m_pInfoTimeInterval);
}

//////////////////////////////////////////////////////////////////////
void CCollectorProcess::AddExtraStringsToMap()
{
	//////////Collecting type
	CStringsMaps::AddItem(COLLECTING_TYPE_ENUM, eCollectingType_audit, "collecting_type_audit");
	CStringsMaps::AddItem(COLLECTING_TYPE_ENUM, eCollectingType_cdr, "collecting_type_cdr");
	CStringsMaps::AddItem(COLLECTING_TYPE_ENUM, eCollectingType_cfg, "collecting_type_cfg");
	CStringsMaps::AddItem(COLLECTING_TYPE_ENUM, eCollectingType_coreDump, "collecting_type_core_dumps");
	CStringsMaps::AddItem(COLLECTING_TYPE_ENUM, eCollectingType_faults, "collecting_type_faults_logs");
	CStringsMaps::AddItem(COLLECTING_TYPE_ENUM, eCollectingType_logs, "collecting_type_logs");
	CStringsMaps::AddItem(COLLECTING_TYPE_ENUM, eCollectingType_processInfo, "collecting_type_processes_info");
	CStringsMaps::AddItem(COLLECTING_TYPE_ENUM, eCollectingType_network_traffic_capture, "collecting_type_network_traffic_capture");
	CStringsMaps::AddItem(COLLECTING_TYPE_ENUM, eCollectingType_participants_recordings, "collecting_type_participants_recordings");
	CStringsMaps::AddItem(COLLECTING_TYPE_ENUM, eCollectingType_nids, "collecting_type_nids");
}

//////////////////////////////////////////////////////////////////////
void CCollectorProcess::SetCollectingStatus(eCollectingStatus status)
{
	m_CollectingStatus = status;

	if (m_CollectingStatus == eCollectingStatus_ready)
	{
		RestartCollectingDetails();
	}


}

//////////////////////////////////////////////////////////////////////
eCollectingStatus CCollectorProcess::GetCollectingStatus()
{
	return m_CollectingStatus;
}
//////////////////////////////////////////////////////////////////////
void CCollectorProcess::SetCollectingFileName(string file_name)
{
	m_CollectingFileName = file_name;
}

//////////////////////////////////////////////////////////////////////
string CCollectorProcess::GetCollectingFileName()
{
	return m_CollectingFileName;
}

//////////////////////////////////////////////////////////////////////
void CCollectorProcess::SetCollectingInfo(CInfoTimeInterval* pInfoTimeInterval)
{
	*m_pInfoTimeInterval = *pInfoTimeInterval;
}

//////////////////////////////////////////////////////////////////////
CInfoTimeInterval* CCollectorProcess::GetCollectingInfo()
{
	return m_pInfoTimeInterval;
}

//////////////////////////////////////////////////////////////////////
void CCollectorProcess::RestartCollectingDetails()
{
	m_pInfoTimeInterval->RestartCollectingDetails();
}

//////////////////////////////////////////////////////////////////////
void CCollectorProcess::SendCollectingToMcuMngr(DWORD isCollecting)
{
   COLLECTOR_COLLECTING_INFO_S* pParams = new COLLECTOR_COLLECTING_INFO_S;
   pParams->isCollecting = isCollecting;

   PTRACE2INT(eLevelInfoNormal, "CCollectorProcess::SendCollectingToMcuMngr, isCollecting = ", isCollecting);

   CSegment*  pMsg = new CSegment();
   pMsg->Put((BYTE*)pParams,sizeof(COLLECTOR_COLLECTING_INFO_S));

   const COsQueue* pMcuMngrMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessMcuMngr,eManager);
   STATUS res = pMcuMngrMbx->Send(pMsg,COLLECTOR_COLLECTING_INFO_REQ);

   POBJDELETE( pParams );

   PASSERT(res);
}

//////////////////////////////////////////////////////////////////////
void CCollectorProcess::HandleAbort()
{
	FTRACESTR(eLevelInfoNormal) << "CCollectorProcess::HandleAbort";
	if (GetCollectingStatus() == eCollectingStatus_aborting)
	{
		std::string temp_output;
		std::string cmd = "rm -Rf "+MCU_TMP_DIR+"/Collect LogFiles/*.tgz";
		SystemPipedCommand(cmd.c_str(),temp_output);

		AbortCreateXml();

		SetCollectingFileName("");

		SetCollectingStatus(eCollectingStatus_ready);

		SendCollectingToMcuMngr(FALSE);
	}
	if (GetCollectingStatus() == eCollectingStatus_ready)
		FTRACESTR(eLevelInfoNormal) << "CCollectorProcess::HandleAbort - collector abort was already done. Nothing to do. ";
}

void CCollectorProcess::AbortCreateXml()
{
	CManagerApi cdrMngrApi(eProcessCDR);
	STATUS res  = cdrMngrApi.SendOpcodeMsg(CDR_COLLECTOR_ABORT);
}
