#include "McuIpmiEntityEventLogGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "IpmiEntityEventLog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMcuIpmiEntityEventLogGet::CMcuIpmiEntityEventLogGet()
{
    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiEntityEventLogGet& CMcuIpmiEntityEventLogGet::operator = (const CMcuIpmiEntityEventLogGet &other)
{
	if(this == &other){
	    return *this;
	}

    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = other.m_updateCounter;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiEntityEventLogGet::~CMcuIpmiEntityEventLogGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CMcuIpmiEntityEventLogGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
    CIpmiEntityEventLog log;
    log.Update();
    log.SerializeXml(pActionsNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuIpmiEntityEventLogGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
    int nStatus = STATUS_OK;
    //GET_VALIDATE_CHILD(pNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD); Not implemented yet
    return nStatus;
}

