
/*#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"*/
#include "CollectorInfoGet.h"
#include "OsFileIF.h"

//////////////////////////////////////////////////////////////////////////////
CCollectorInfoGet::CCollectorInfoGet()
{
	m_pProcess = (CCollectorProcess*)CCollectorProcess::GetProcess();
}

/////////////////////////////////////////////////////////////////////////////
CCollectorInfoGet& CCollectorInfoGet::operator = (const CCollectorInfoGet &other)
{
	if(&other != this)
	{
		CSerializeObject::operator =(other);
		m_pProcess = other.m_pProcess;
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCollectorInfoGet::~CCollectorInfoGet()
{
}

/////////////////////////////////////////////////////////////////////////////
CCollectorInfoGet::CCollectorInfoGet( const CCollectorInfoGet &other )
:CSerializeObject(other)
{
	m_pProcess = other.m_pProcess;
}

///////////////////////////////////////////////////////////////////////////
void CCollectorInfoGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CXMLDOMElement* pCollectInfoSettings = pActionsNode->AddChildNode("COLLECT_INFO_SETTINGS");

	if(m_pProcess->GetCollectingFileName().length() > 0)
	{
		BOOL isExist = IsFileExists(m_pProcess->GetCollectingFileName());
		if(!isExist)
		{
			m_pProcess->SetCollectingFileName("");
		}  
	}
    
	pCollectInfoSettings->AddChildNode("LAST_GENERATED_FILE",m_pProcess->GetCollectingFileName());

	CInfoTimeInterval* pCollectingInfo = m_pProcess->GetCollectingInfo();

	if (m_pProcess->GetCollectingStatus() == eCollectingStatus_ready)
	{
		pCollectingInfo->SetStartAndEndTimeToCurrentTime();
	}

	if (pCollectingInfo)
		pCollectingInfo->SerializeXml(pCollectInfoSettings);
}

/////////////////////////////////////////////////////////////////////////////
int CCollectorInfoGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////


