#include "FailoverConfigurationDBGet.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "psosxml.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFailoverConfigurationDBGet::CFailoverConfigurationDBGet()
{
	m_pProcess = (CFailoverProcess*)CFailoverProcess::GetProcess();

	//CFailoverProcess * process = (CFailoverProcess *) CProcessBase::GetProcess();
	//m_pFailoverConfiguration=(process->GetFailoverConfiguration());

	//m_pFailoverConfiguration = new CFailoverConfiguration();

}

CFailoverConfigurationDBGet::CFailoverConfigurationDBGet(CFailoverConfiguration * pFailoverConfiguration)
: m_pProcess(NULL)
{
	//m_pFailoverConfiguration = new CFailoverConfiguration();
	//*m_pFailoverConfiguration = *pFailoverConfiguration;

}
CFailoverConfigurationDBGet::CFailoverConfigurationDBGet(const CFailoverConfigurationDBGet &other)
:CSerializeObject(other), m_pProcess(NULL)
{
	//*m_pFailoverConfiguration = *other.m_pFailoverConfiguration;

}

/////////////////////////////////////////////////////////////////////////////
CFailoverConfigurationDBGet& CFailoverConfigurationDBGet::operator = (const CFailoverConfigurationDBGet &other)
{
	if (&other == this)
		return *this;

	//*m_pFailoverConfiguration = *other.m_pFailoverConfiguration;
	m_pProcess = (CFailoverProcess*)CFailoverProcess::GetProcess();
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CFailoverConfigurationDBGet::~CFailoverConfigurationDBGet()
{
	//POBJDELETE(m_pFailoverConfiguration);
	//*m_pFailoverConfiguration = *m_pFailoverConfiguration;
}

/////////////////////////////////////////////////////////////////////////////
int CFailoverConfigurationDBGet::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int nStatus = STATUS_OK;

	//GET_VALIDATE_CHILD(pResNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);

	return nStatus;
}
///////////////////////////////////////////////////////////////////////////
void CFailoverConfigurationDBGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CFailoverConfiguration* pFailoverConfiguration = m_pProcess->GetFailoverConfiguration();
	pFailoverConfiguration->SerializeXml(pFatherNode);
}

//////////////////////////////////////////////////////////////////////////


