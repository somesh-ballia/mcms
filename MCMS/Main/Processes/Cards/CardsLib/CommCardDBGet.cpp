// CommCardDBGet.cpp: implementation of the CCommCardDBGet class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Get XML Card List
//========   ==============   =====================================================================

#include "CommCardDBGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "HwMonitoring.h"
#include "ApiStatuses.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommCardDBGet::CCommCardDBGet()
{
	m_pProcess = (CCardsProcess*)CCardsProcess::GetProcess();
	m_updateCounter = 0;
}
/////////////////////////////////////////////////////////////////////////////
CCommCardDBGet& CCommCardDBGet::operator = (const CCommCardDBGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommCardDBGet::~CCommCardDBGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CCommCardDBGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CCommCardDB* pCardDB = m_pProcess->GetCardsMonitoringDB();

	if (pCardDB)
	{
		pCardDB->SerializeXml(pActionsNode,m_updateCounter);
	}
}



/////////////////////////////////////////////////////////////////////////////
int CCommCardDBGet::DeSerializeXml(CXMLDOMElement* pResNode,char* pszError,const char* strAction)
{

	DeSerializeXml(pResNode,pszError);
	

	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CCommCardDBGet::DeSerializeXml(CXMLDOMElement* pActionsNode, char* pszError)
{
		
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionsNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
		
	//This code can`t be implement on Serialize
	//DWORD summaryCounter = ::GetpCardDB()->GetSummaryUpdateCounter();
	//if(summaryCounter != GetUpdateCounter())
	//{
	   //SetUpdateCounter(summaryCounter);
	  //SetUpdateFlag(YES);
	//}
//	else
//	{
//	   //SetStatus(STATUS_OK);
//	   //SetUpdateFlag(NO);
//	}
	
	return nStatus;
	
}



/*
////////////////////////////////////////////////////////////////////////////////////////////////
int CCommCardDBGet::convertStrActionToNumber(const char* strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("START",strAction,5))
		numAction=ADD_RESERVE;
	return numAction;
}
*/


