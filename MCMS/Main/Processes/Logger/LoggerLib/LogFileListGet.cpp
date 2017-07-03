// LogFileListGet.cpp: implementation of the CLogFileListGet class.
//
//////////////////////////////////////////////////////////////////////

#include "LogFileListGet.h"
#include "LoggerProcess.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CLogFileListGet::CLogFileListGet()
{
	m_updateCounter = 0;
}


/////////////////////////////////////////////////////////////////////////////
CLogFileListGet& CLogFileListGet::operator = (const CLogFileListGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CLogFileListGet::~CLogFileListGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CLogFileListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{

	CLoggerProcess* pLoggerProcess = (CLoggerProcess*)CProcessBase::GetProcess();
	if (!pLoggerProcess)
	{
		PTRACE(eLevelError, "CLogFileListGet::SerializeXml : Failed to get a pointer to Logger process");
		return;
	}

	CLoggerFileList* pLoggerFileList = pLoggerProcess->GetLoggerFileList();
	if (!pLoggerFileList)
	{
		PTRACE(eLevelError, "CLogFileListGet::SerializeXml : Failed to get a pointer to the Logger file list");
		return;
	}

	if (true == pLoggerProcess->GetDropMessageFlag()) //on high CPU Usage fetch only 200 Last Records
	    pLoggerFileList->SerializeXmlPartialList(pActionsNode,200);
	else
	    pLoggerFileList->SerializeXml(pActionsNode);

}



/////////////////////////////////////////////////////////////////////////////
int CLogFileListGet::DeSerializeXml(CXMLDOMElement* pResNode,char* pszError,const char* strAction)
{

	DeSerializeXml(pResNode,pszError);


	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CLogFileListGet::DeSerializeXml(CXMLDOMElement* pActionsNode, char* pszError)
{

	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionsNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);

	return nStatus;

}





