// ApacheModuleMonitor.cpp: implementation of the ApacheModuleMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "ApacheModuleMonitor.h"
#include "TaskApi.h"
#include "OpcodesMcmsCommon.h"
#include "ConnectionListGet.h"
#include "Request.h"
#include "FileListGet.h"
#include "DummyEntry.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CApacheModuleMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CApacheModuleMonitor::HandlePostRequest)
PEND_MESSAGE_MAP(CApacheModuleMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CApacheModuleMonitor)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CApacheModuleMonitor::HandleOperLogin)
	ON_TRANS("TRANS_CONNECTIONS_LIST","GET",CConnectionListGet,CApacheModuleMonitor::HandleGetConnectionList)
	ON_TRANS("TRANS_MCU","GET_VIRTUAL_DIRECTORY",CFileListGet,CApacheModuleMonitor::HandleGetVirtualDirectory)	
	ON_TRANS("TRANS_MCU","GET_VIRTUAL_DIRECTORY_RECURSIVE",CFileListGet,CApacheModuleMonitor::HandleGetVirtualDirectoryRecursive)		
END_TRANSACTION_FACTORY



////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void ApacheModuleMonitorEntryPoint(void* appParam)
{  
	CApacheModuleMonitor *monitorTask = new CApacheModuleMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CApacheModuleMonitor::CApacheModuleMonitor()
{

}

CApacheModuleMonitor::~CApacheModuleMonitor()
{

}

STATUS CApacheModuleMonitor::HandleGetConnectionList(CRequest *pRequest)
{
	CConnectionListGet* pRequestConnectionListGet = (CConnectionListGet*)pRequest->GetRequestObject();

	CConnectionListGet* pConfirmConnectionListGet = new CConnectionListGet(*pRequestConnectionListGet);
	
	pRequest->SetConfirmObject(pConfirmConnectionListGet);

	return STATUS_OK;
}

STATUS CApacheModuleMonitor::HandleGetVirtualDirectory(CRequest *pRequest)
{
	return GetVirtualDirectory(pRequest,FALSE);
}

STATUS CApacheModuleMonitor::HandleGetVirtualDirectoryRecursive(CRequest *pRequest)
{
	return GetVirtualDirectory(pRequest,TRUE);
}

STATUS CApacheModuleMonitor::GetVirtualDirectory(CRequest *pRequest, WORD bNested)
{
	CFileListGet* pRequestConnectionListGet = (CFileListGet*)pRequest->GetRequestObject();
	CFileListGet* pConfirmConnectionListGet	= new CFileListGet(*pRequestConnectionListGet);
	
	pRequest->SetStatus(pConfirmConnectionListGet->FillFileList(bNested));
	
	if(pRequest->GetStatus() == STATUS_OK)
		pRequest->SetConfirmObject(pConfirmConnectionListGet);
	else
	{
		POBJDELETE(pConfirmConnectionListGet);
		pRequest->SetConfirmObject(new CDummyEntry);
	}
	
	return STATUS_OK;
}

