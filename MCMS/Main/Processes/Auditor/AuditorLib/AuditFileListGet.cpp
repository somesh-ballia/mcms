// AuditFileListGet.cpp: implementation of the CAuditFileListGet class.
//
//////////////////////////////////////////////////////////////////////

#include "AuditFileListGet.h"
#include "AuditorProcess.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "AuditFileList.h"
  
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAuditFileListGet::CAuditFileListGet()
{
	m_updateCounter = 0;
}


/////////////////////////////////////////////////////////////////////////////
CAuditFileListGet& CAuditFileListGet::operator = (const CAuditFileListGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CAuditFileListGet::~CAuditFileListGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CAuditFileListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CAuditorProcess* pAuditorProcess = (CAuditorProcess*)CProcessBase::GetProcess();
	if (!pAuditorProcess)
	{
		PTRACE(eLevelError, "CAuditFileListGet::SerializeXml : Failed to get a pointer to Auditor process");
		return;
	}
	
	CAuditFileList* pAuditFileList = pAuditorProcess->GetAuditFileList();
	if (!pAuditFileList)
	{
		PTRACE(eLevelError, "CAuditFileListGet::SerializeXml : Failed to get a pointer to the Audit file list");
		return;
	}

//    pAuditFileList->FixLastFile();
	pAuditFileList->SerializeXml(pActionsNode);
}



/////////////////////////////////////////////////////////////////////////////
int CAuditFileListGet::DeSerializeXml(CXMLDOMElement* pResNode,char* pszError,const char* strAction)
{

	DeSerializeXml(pResNode,pszError);
	

	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CAuditFileListGet::DeSerializeXml(CXMLDOMElement* pActionsNode, char* pszError)
{
		
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionsNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
		
	return nStatus;
	
}


  
  

