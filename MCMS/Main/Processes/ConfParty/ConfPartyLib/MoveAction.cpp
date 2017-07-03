// RsrvPartyAction.h: interface for the CRsrvPartyAction class.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class conf actions with one int param - such as auto layout = on/off
//========   ==============   =====================================================================






#include "MoveAction.h"
#include "ConfPartyDefines.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMoveBaseAction::CMoveBaseAction():m_SourceConfID(0xFFFFFFFF),m_PartyID(0xFFFFFFFF)
{
}
/////////////////////////////////////////////////////////////////////////////
CMoveBaseAction::~CMoveBaseAction()
{
}
/////////////////////////////////////////////////////////////////////////////
int CMoveBaseAction::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char * strAction)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode,"ID",&m_SourceConfID,_0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_PartyID,_0_TO_DWORD);

  COstrStream trace_str;
  trace_str << " m_SourceConfID = " << m_SourceConfID << " , m_PartyID = " << m_PartyID;
  PTRACE2(eLevelInfoNormal,"CMoveBaseAction::DeSerializeXml: ",trace_str.str().c_str());

  return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CMoveBaseAction::SerializeXml(CXMLDOMElement*& pFatherNode) const
{

}
//////////////////////////////////////////////////////////////////////////
DWORD CMoveBaseAction::GetSourceConfID()const
{
  return m_SourceConfID;
}
//////////////////////////////////////////////////////////////////////////
DWORD CMoveBaseAction::GetPartyID()const
{
  return m_PartyID;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
CMoveAction::CMoveAction():m_TargetConfID(0xFFFFFFFF)
{
}
/////////////////////////////////////////////////////////////////////////////
CMoveAction::~CMoveAction()
{
}
/////////////////////////////////////////////////////////////////////////////
int CMoveAction::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char * strAction)
{
  int nStatus = STATUS_OK;

  CMoveBaseAction::DeSerializeXml(pActionNode,pszError,strAction);
  GET_VALIDATE_CHILD(pActionNode,"TARGET_ID",&m_TargetConfID,_0_TO_DWORD);

  COstrStream trace_str;
  trace_str << " m_SourceConfID = " << m_SourceConfID << " , m_PartyID = " << m_PartyID << " , m_TargetConfID = " << m_TargetConfID;
  PTRACE2(eLevelInfoNormal,"CMoveAction::DeSerializeXml: ",trace_str.str().c_str());

  return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CMoveAction::SerializeXml(CXMLDOMElement*& pFatherNode) const
{

}
//////////////////////////////////////////////////////////////////////////
DWORD CMoveAction::GetTargetConfID()const
{
  return m_TargetConfID;
}
//////////////////////////////////////////////////////////////////////////

