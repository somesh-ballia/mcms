// CDRProcess.cpp: implementation of the CCDRProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "CDRProcess.h"
#include "SystemFunctions.h"
#include "SystemFunctions.h"
#include "CDRShort.h"
#include "CDRLog.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "ConfPartySharedDefines.h"
#include "H221.h"
#include "CDREvent.h"
#include "CDRStatuses.h"



extern void CDRManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CCDRProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CCDRProcess::GetManagerEntryPoint()
{
	return CDRManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CCDRProcess::CCDRProcess()
{
//	m_pCDRList = NULL;
	m_pCdrLog = NULL;
	m_pCdrSettings = new CCDRSettings();
}

//////////////////////////////////////////////////////////////////////
CCDRProcess::~CCDRProcess()
{
//	PDELETE(m_pCDRList);
	PDELETE(m_pCdrLog);
	PDELETE(m_pCdrSettings);

}

// //////////////////////////////////////////////////////////////////////
// CCdrList* CCDRProcess::GetCdrList()
// {
// 	return m_pCDRList;
// }

// //////////////////////////////////////////////////////////////////////
// void CCDRProcess::SetCdrList(CCdrList *list)
// {
// 	m_pCDRList = list;
// }
	
//////////////////////////////////////////////////////////////////////
CCdrLog* CCDRProcess::GetCdrLog()
{
	return m_pCdrLog;
}

//////////////////////////////////////////////////////////////////////
void CCDRProcess::SetCdrLog(CCdrLog *log)
{
	m_pCdrLog = log;
}


void CCDRProcess::SetCdrSettings(CCDRSettings* pCdrSettings)
{
	*m_pCdrSettings = *pCdrSettings;
}

//////////////////////////////////////////////////////////////////////
void CCDRProcess::AddExtraStatusesStrings()
{
	AddStatusString(STATUS_NO_HARD_DISK, "Hard disk not found");
	AddStatusString(STATUS_FAIL_TO_PARSE_INDEX_FILE, "Failed to parse index file");
}

/////////////////////////////////////////////////////////////////////////////////////////////////
