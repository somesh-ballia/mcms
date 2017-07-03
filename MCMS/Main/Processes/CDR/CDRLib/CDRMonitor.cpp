// CDRMonitor.cpp: implementation of the CDRMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "CDRMonitor.h"
#include "TaskApi.h"
#include "TraceStream.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "Request.h"
#include "CDRShort.h"
#include "ProcessBase.h"
#include "CDRProcess.h"
#include "CdrListGet.h"
#include "DummyEntry.h"
#include "CdrFullGet.h"
#include "CDRDetal.h"
#include "CDRLog.h"
#include "ApacheDefines.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"
#include "CdrSettingsGet.h"


////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CCDRMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CCDRMonitor::HandlePostRequest )
  //ONEVENT(CDR_CREATE_XML_FOLDER, ANYCASE, CCDRMonitor::HandleHandleCDRCreateXmlFolder)
PEND_MESSAGE_MAP(CCDRMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CCDRMonitor)
	ON_TRANS("TRANS_CDR_LIST", "GET", CCdrListGet, CCDRMonitor::HandleCdrGet)
	ON_TRANS("TRANS_CDR_FULL", "GET", CCdrFullGet, CCDRMonitor::HandleCdrFullGet)
	ON_TRANS("TRANS_MCU", "GET_CDR_SETTINGS", CCdrSettingsGet, CCDRMonitor::HandleCdrSettingsGet)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void CDRMonitorEntryPoint(void* appParam)
{  
	CCDRMonitor *monitorTask = new CCDRMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCDRMonitor::CCDRMonitor()
{
	m_CDRProcess = dynamic_cast<CCDRProcess*>(CProcessBase::GetProcess());
}

// Virtual
CCDRMonitor::~CCDRMonitor(void)
{ }

//////////////////////////////////////////////////////////////////////  
STATUS CCDRMonitor::HandleCdrGet(CRequest *pGetRequest)
{	
	if(pGetRequest->GetAuthorization() != SUPER && pGetRequest->GetAuthorization() != ADMINISTRATOR_READONLY)
	{
	    TRACEINTO << __PRETTY_FUNCTION__ << ": "
	              << "No permission to get list of cdr files";

		pGetRequest->SetConfirmObject(new CDummyEntry);
		pGetRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;		
	}
	
	CCdrList* pCdrList = m_CDRProcess->GetCdrLog()->GetCdrList();
	CCdrListGet* pCdrListGet = new CCdrListGet(pCdrList);
	pGetRequest->SetConfirmObject(pCdrListGet);
	pGetRequest->SetStatus(STATUS_OK);
	
	return STATUS_OK;
}



////////////////////////////////////////////////////////////////////////////////
STATUS CCDRMonitor::HandleCdrFullGet(CRequest* pGetRequest)
{		
	if(pGetRequest->GetAuthorization() != SUPER)
	{
	    TRACEINTO << __PRETTY_FUNCTION__ << ": "
	            << "No permission to get full cdr file";

		pGetRequest->SetConfirmObject(new CDummyEntry);
		pGetRequest->SetStatus(STATUS_NO_PERMISSION);		
		return STATUS_OK;		
	}
	
	CCdrFullGet* cdr = static_cast<CCdrFullGet*>(pGetRequest->GetRequestObject());
	DWORD confID = cdr->GetConfId();
	DWORD partID = cdr->GetFilePartIndex();

	CCdrLongStruct* cdr_long = new CCdrLongStruct;
	STATUS status = m_CDRProcess->GetCdrLog()->GetLongStructPtrArray(confID,
                                                                     partID,
                                                                     *cdr_long);
	CSerializeObject* pResponse = NULL;
	if(STATUS_OK == status)
	{
        CManagerApi api(eProcessCDR);
        api.SendMsg(NULL, CDR_RETRIVED_INTERNAL_NOTIFY);

        pResponse = cdr->Clone();
        //todo: write to xml
        TRACEINTO << " CCDRMonitor::HandleCdrFullGet ";
        static_cast<CCdrFullGet*>(pResponse)->SetCDRLong(cdr_long);
        string fileNameStr = cdr_long->GetShortCdrStruct()->GetFileName();
       // AddCdrToXml(pResponse, fileNameStr);
       /* char buffer [10];
        printf(buffer, "%d", 3);
        string outStrVal = buffer;*/
        /*
        string xmlStr = ".xml";
        string dirctorySt = MCU_TMP_DIR+"/CdrXml/";
        string str = cdr_long->GetShortCdrStruct()->GetFileName();
        str = str + xmlStr.c_str();
        string path = dirctorySt + str;
        TRACEINTO << "path = " << path;
        STATUS st = pResponse->WriteXmlFile(path.c_str(), "CDR");*/

       // TRACEINTO << " CCDRMonitor::HandleCdrFullGet status= " <<st;
	}
	else
	{	
		delete cdr_long;
		pResponse = new CDummyEntry;
	}
	
	pGetRequest->SetConfirmObject(pResponse);
	pGetRequest->SetStatus(status);

	//CCDRMonitor::AddAllCdrToXmlDirector();

	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////
STATUS CCDRMonitor::HandleCdrSettingsGet(CRequest* pGetRequest)
{
//	CCdrSettingsGet* pGet = (CCdrSettingsGet*)pGetRequest;
	CCDRSettings* pCdrSettings = new CCDRSettings();

	CCDRProcess* pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
	*pCdrSettings = *(pProcess->GetCdrSettings());

	if (pGetRequest->GetRequestObject()->m_apiFormat)
	{
		TRACESTR(eLevelInfoNormal) << "CCDRMonitor::HandleCdrSettingsGet, m_apiFormat=" << pGetRequest->GetRequestObject()->m_apiFormat;
		pCdrSettings->m_apiFormat = (eApiFormat)pGetRequest->GetRequestObject()->m_apiFormat;
	}

	pGetRequest->SetConfirmObject(pCdrSettings);
	return STATUS_OK;
}

/*
STATUS CCDRMonitor::AddCdrToXml(CSerializeObject* pResponse, string fileName)
{
	string xmlStr = ".xml";
	string dirctorySt = MCU_TMP_DIR+"/CdrXml/";
	string newFileName = fileName + xmlStr.c_str();
	string path = dirctorySt + newFileName;
	TRACEINTO << "path = " << path;
	STATUS status = pResponse->WriteXmlFile(path.c_str(), "CDR");
	return status;
}
STATUS CCDRMonitor::AddAllCdrToXmlDirector()
{
	CCdrList* cdrlist = m_CDRProcess->GetCdrLog()->GetCdrList();
	size_t index = 0;
	STATUS status = NULL;
	CCdrFullGet* pResponse = new CCdrFullGet();
	for (const CCdrShort* cdr = cdrlist->GetFirstShort();
			NULL != cdr;
			cdr = cdrlist->GetNextShort(), ++index)
	{
		CCdrLongStruct* cdr_long = new CCdrLongStruct;
		status = m_CDRProcess->GetCdrLog()->GetLongStructPtrArray(cdr->GetConfId(),
				cdr->GetFilePartIndex(),
				*cdr_long);
		//CSerializeObject* pResponse = NULL;

		//pResponse = cdr->Clone();
		//todo: write to xml
		TRACEINTO << " CCDRMonitor::HandleCdrFullGet ";
		static_cast<CCdrFullGet*>(pResponse)->SetCDRLong(cdr_long);
		string fileNameStr = cdr_long->GetShortCdrStruct()->GetFileName();
		AddCdrToXml(pResponse, fileNameStr);
	}
	delete pResponse;


	return status;
}

STATUS CCDRMonitor::HandleHandleCDRCreateXmlFolder(CSegment* pSeg)
{
	TRACEINTO << "HandleHandleCDRCreateXmlFolder";
	AddAllCdrToXmlDirector();
	return STATUS_OK;
}
*/
