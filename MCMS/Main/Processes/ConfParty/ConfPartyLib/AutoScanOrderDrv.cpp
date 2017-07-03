// AutoScanOrderDrv.cpp: implementation for the CAutoScanOrderDrv class.
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//9/05			Talya			  Class for Change Lecture Mode Params
//========   ==============   =====================================================================



#include "AutoScanOrderDrv.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "CommConfDB.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAutoScanOrderDrv::CAutoScanOrderDrv()
{
	m_ConfID				= 0xFFFFFFFF;
	m_pAutoScanOrder	= new CAutoScanOrder;
}
/////////////////////////////////////////////////////////////////////////////
CAutoScanOrderDrv& CAutoScanOrderDrv::operator = (const CAutoScanOrderDrv &other)
{
	if ( &other == this ) return *this;

	m_ConfID				= other.m_ConfID;
	*m_pAutoScanOrder	= *other.m_pAutoScanOrder;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CAutoScanOrderDrv::~CAutoScanOrderDrv()
{
	POBJDELETE(m_pAutoScanOrder);
}
/*
/////////////////////////////////////////////////////////////////////////////
int CAutoScanOrderDrv::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode,pszError,numAction);
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CAutoScanOrderDrv::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("SET_LECTURE_MODE",strAction,16))
		numAction = CHANGE_LECTURE_MODE;
	
	return numAction;
}

*/
////////////////////////////////////////////////////////////////////////////////////////////////
int CAutoScanOrderDrv::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"ID",&m_ConfID,_0_TO_DWORD);
	
	GET_MANDATORY_CHILD_NODE(pActionNode, "AUTO_SCAN_ORDER", pActionNode);
	
	CCommConf *pConf = ::GetpConfDB()->GetCurrentConf(m_ConfID);
	/*
	if(pConf)
	{
		*m_pAutoScanOrder = *(pConf->GetLectureMode());
	}
	*/
	nStatus = m_pAutoScanOrder->DeSerializeXml(pActionNode,pszError);
	return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CAutoScanOrderDrv::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////////
CAutoScanOrder*  CAutoScanOrderDrv::GetAutoScanOrder()
{
	return m_pAutoScanOrder;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CAutoScanOrderDrv::GetConfID()
{
	return m_ConfID;
}

