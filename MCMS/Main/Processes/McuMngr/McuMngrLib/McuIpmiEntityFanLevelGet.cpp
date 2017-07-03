#include "McuIpmiEntityFanLevelGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "IpmiEntityFanLevel.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMcuIpmiEntityFanLevelGet::CMcuIpmiEntityFanLevelGet()
{
    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiEntityFanLevelGet& CMcuIpmiEntityFanLevelGet::operator = (const CMcuIpmiEntityFanLevelGet &other)
{
	if(this == &other){
	    return *this;
	}

    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = other.m_updateCounter;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiEntityFanLevelGet::~CMcuIpmiEntityFanLevelGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CMcuIpmiEntityFanLevelGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
    CIpmiEntityFanLevel level;
    level.Update();
    level.SerializeXml(pActionsNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuIpmiEntityFanLevelGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
    int nStatus = STATUS_OK;
    //GET_VALIDATE_CHILD(pNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD); Not implemented yet
    return nStatus;
}

