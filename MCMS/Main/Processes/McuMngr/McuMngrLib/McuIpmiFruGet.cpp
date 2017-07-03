#include "McuIpmiFruGet.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "IpmiFru.h"
#include "ApiStatuses.h"
#include "Trace.h"
#include "TraceStream.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static int g_slotId = -1;
CMcuIpmiFruGet::CMcuIpmiFruGet()
{
    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = 0;
    m_slotID = 0;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiFruGet& CMcuIpmiFruGet::operator = (const CMcuIpmiFruGet &other)
{
	if(this == &other){
	    return *this;
	}

    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = other.m_updateCounter;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiFruGet::~CMcuIpmiFruGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CMcuIpmiFruGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
       PTRACE2INT(eLevelError, "CMcuIpmiFruGet::SerializeXml slot id = ", g_slotId);
       CIpmiFru fru;
       fru.Update(g_slotId);
       fru.SerializeXml(pActionsNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuIpmiFruGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
    int nStatus = STATUS_OK;
    DWORD tmp = (DWORD)m_slotID;
    GET_VALIDATE_CHILD(pNode, "SlotID", &tmp, _0_TO_DWORD);
    g_slotId = tmp;
    PTRACE2INT(eLevelError, "CMcuIpmiFruGet::DeSerializeXml slot id = ", g_slotId);
    return nStatus;
}

