
#include "TerminalCommand.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "OsTask.h"
#include "CdrConvertToXml.h"
#include "CdrFullGet.h"
#include "TraceStream.h"
#include "CDRLog.h"
#include "CDRDetal.h"
#include "CDRLog.h"
#include <sys/signal.h>
#include "Native.h"
#include "SingleToneApi.h"



PBEGIN_MESSAGE_MAP(CCdrConvertToXml)

  ONEVENT(CDR_CONVERTOR_CREATE_XML,  ANYCASE, CCdrConvertToXml::CreateXmlFolder)
  ONEVENT(CDR_CONVERTOR_ABORT,  ANYCASE, CCdrConvertToXml::Abort)

PEND_MESSAGE_MAP(CCdrConvertToXml,CTaskApp);




void CdrConvertToXmlEntryPoint(void* appParam)
{
	CCdrConvertToXml* pCdrConverterTaskApp = new CCdrConvertToXml;
	pCdrConverterTaskApp->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////
CCdrConvertToXml::CCdrConvertToXml()
{
	m_CDRProcess = dynamic_cast<CCDRProcess*>(CProcessBase::GetProcess());
	m_isAborted = false;
}

CCdrConvertToXml::~CCdrConvertToXml()
{

}
/////////////////////////////////////////////////////////////////////////////
void CCdrConvertToXml::Create(CSegment& appParam, WORD limited)
{
    CTaskApp::Create( appParam, limited );
}

const char* CCdrConvertToXml::GetTaskName() const
{
	return "CCdrConvertToXml";
}

/////////////////////////////////////////////////////////////////////////////
void CCdrConvertToXml::CreateXmlFolder(CSegment* pSeg)
{
	
	m_isAborted  = false;
	CStructTm startTime;
	CStructTm endTime;

	CStructTmDrv startTimeDrv;
	startTimeDrv.DeSerialize(NATIVE, *pSeg);

	CStructTmDrv actualDurationDrv;
	actualDurationDrv.DeSerialize(NATIVE, *pSeg);

	//remove old files
	std::string temp_output;
	std::string cmd = "rm -Rf "+ MCU_TMP_DIR+"/CdrXml/";
	SystemPipedCommand(cmd.c_str(),temp_output);
	cmd = "mkdir "+ MCU_TMP_DIR+"/CdrXml/";
	SystemPipedCommand(cmd.c_str(),temp_output);
	//create the directory with the new files
	AddAllCdrToXmlDirector(&startTimeDrv, &actualDurationDrv);
}
void CCdrConvertToXml::Abort()
{
	m_isAborted = true;
}


STATUS CCdrConvertToXml::AddCdrToXml(CSerializeObject* pResponse, string fileName)
{
	string xmlStr = ".xml";
	string dirctorySt = MCU_TMP_DIR+"/CdrXml/";
	string newFileName = fileName + xmlStr.c_str();
	string path = dirctorySt + newFileName;
	STATUS status = pResponse->WriteXmlFile(path.c_str(), "CDR");
	return status;
}
STATUS CCdrConvertToXml::AddAllCdrToXmlDirector(CStructTm* startTime, CStructTm* endTime)
{
	CCdrList* cdrlist = m_CDRProcess->GetCdrLog()->GetCdrList();
	size_t index = 0;
	STATUS status = STATUS_OK;

	FTRACEINTO << "Start AddAllCdrToXmlDirector ";

	for (CCdrShort* cdr = cdrlist->GetFirstShort();
			NULL != cdr;
			cdr = cdrlist->GetNextShort(), ++index)
	{
		if(m_isAborted == true)
		{

			m_isAborted = false;
			return status;
		}
		if(!IsValidTime(cdr, startTime,  endTime))
		{
			continue;
		}
		CCdrLongStruct* cdr_long = new CCdrLongStruct;
		status = m_CDRProcess->GetCdrLog()->GetLongStructPtrArray(cdr->GetConfId(),
				cdr->GetFilePartIndex(),
				*cdr_long);
		CCdrFullGet responseCdr;
		responseCdr.SetCDRLong(cdr_long);

		string fileNameStr = cdr_long->GetShortCdrStruct()->GetFileName();

		AddCdrToXml(&responseCdr, fileNameStr);
	}

	CSegment *pSeg = new CSegment;

	CSingleToneApi collectorUnitApi(eProcessCollector,"CollectorUnit");

	FTRACEINTO << "Done AddAllCdrToXmlDirector ";

	collectorUnitApi.SendMsg(pSeg,CDR_XML_READY);
	collectorUnitApi.CleanUp();

	return status;
}

bool CCdrConvertToXml::IsValidTime(CCdrShort* cdrShort, CStructTm* startTime, CStructTm* endTime)
{
	bool valid = false;
	CStructTm* pCdrTime=cdrShort->GetActualStrtTime();
	if (pCdrTime->IsValidForCdr())
	{
		if ((*pCdrTime >= *startTime) && (*pCdrTime <= *endTime))
		{
			valid = true;
		}
	}
	return valid;
}

