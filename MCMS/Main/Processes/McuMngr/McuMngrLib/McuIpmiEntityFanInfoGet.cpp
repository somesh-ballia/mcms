#include "McuIpmiEntityFanInfoGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "IpmiEntityFanInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMcuIpmiEntityFanInfoGet::CMcuIpmiEntityFanInfoGet()
{
    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiEntityFanInfoGet& CMcuIpmiEntityFanInfoGet::operator = (const CMcuIpmiEntityFanInfoGet &other)
{
	if(this == &other){
	    return *this;
	}

    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = other.m_updateCounter;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiEntityFanInfoGet::~CMcuIpmiEntityFanInfoGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CMcuIpmiEntityFanInfoGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
    CIpmiEntityFanInfo info;
    info.Update();
    info.SerializeXml(pActionsNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuIpmiEntityFanInfoGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
    int nStatus = STATUS_OK;
    //GET_VALIDATE_CHILD(pNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD); Not implemented yet
    return nStatus;
}

