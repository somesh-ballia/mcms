#include "CommResRecurrenceResponse.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "FaultsDefines.h"
#include "TraceStream.h"
#include "CommResFailedRecurrence.h"
#include "CommResApi.h"

CCommResRecurrenceResponse::CCommResRecurrenceResponse()
{
	m_FailedInd = 0;
	m_pCommResApi = new CCommResApi();
	memset(m_failedRecurrenceList, 0, sizeof(m_failedRecurrenceList));
}
///////////////////////////////////////////////////////////////////////////////

CCommResRecurrenceResponse::~CCommResRecurrenceResponse()
{
    for(int i = 0; i < MAX_RSRV_IN_LIST_AMOS; i++)
        POBJDELETE(m_failedRecurrenceList[i]);
    
    POBJDELETE(m_pCommResApi);
}
///////////////////////////////////////////////////////////////////////////////

CCommResRecurrenceResponse::CCommResRecurrenceResponse(const CCommResRecurrenceResponse &other)
:CSerializeObject(other)
{
    *this = other;
}
///////////////////////////////////////////////////////////////////////////////
CCommResRecurrenceResponse& CCommResRecurrenceResponse::operator = (const CCommResRecurrenceResponse &other)
{
	m_FailedInd = other.m_FailedInd;

	for (int i = 0; i < MAX_RSRV_IN_LIST_AMOS; i++)
	{
		POBJDELETE(m_failedRecurrenceList[i]);
		if (other.m_failedRecurrenceList[i])
		{
			m_failedRecurrenceList[i] = new CCommResFailedRecurrence();
			*m_failedRecurrenceList[i] = *other.m_failedRecurrenceList[i];
		}
	}

	POBJDELETE(m_pCommResApi);
	m_pCommResApi = new CCommResApi();
	*m_pCommResApi = *(other.m_pCommResApi);

	return *this;
}
///////////////////////////////////////////////////////////////////////////////
void CCommResRecurrenceResponse::SerializeXml(CXMLDOMElement*& pActionNode) const
{
	CXMLDOMElement *pRecurListNode;

	pRecurListNode = pActionNode->AddChildNode("RECURRENCE_LIST");

	for(DWORD i=0; i<m_FailedInd; i++)
	{
		if(m_failedRecurrenceList[i])
			m_failedRecurrenceList[i]->SerializeXml(pRecurListNode);
	}
	
	m_pCommResApi->SerializeXml(pActionNode);
}
///////////////////////////////////////////////////////////////////////////////
int CCommResRecurrenceResponse::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action)
{
	TRACESTR(eLevelError) << "CCommResFailedRecurrence::DeSerializeXml This Code Should not be invoked !!!!! " ;
	PASSERT(1);
	return STATUS_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
const char*  CCommResRecurrenceResponse::NameOf() const
{
    return "CCommResRecurrenceResponse";
}
///////////////////////////////////////////////////////////////////////////////
void CCommResRecurrenceResponse::AddFailedRecurrence(CCommResFailedRecurrence* pFailedRecurrence)
{
    PASSERT_AND_RETURN(m_FailedInd >= MAX_RSRV_IN_LIST_AMOS);
    m_failedRecurrenceList[m_FailedInd++] = pFailedRecurrence;
}
///////////////////////////////////////////////////////////////////////////////
void CCommResRecurrenceResponse::SetCommRes(CCommResApi* pCommResApi) 
{
	*m_pCommResApi = *pCommResApi;
}


