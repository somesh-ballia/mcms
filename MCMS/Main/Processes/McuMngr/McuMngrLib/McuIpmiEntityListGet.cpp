#include "McuIpmiEntityListGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "IpmiEntityList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMcuIpmiEntityListGet::CMcuIpmiEntityListGet()
{
    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiEntityListGet& CMcuIpmiEntityListGet::operator = (const CMcuIpmiEntityListGet &other)
{
	if(this == &other){
	    return *this;
	}

    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = other.m_updateCounter;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiEntityListGet::~CMcuIpmiEntityListGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CMcuIpmiEntityListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
    CIpmiEntityList list;
    list.Update();
    list.SerializeXml(pActionsNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuIpmiEntityListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
    int nStatus = STATUS_OK;
    //GET_VALIDATE_CHILD(pNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD); Not implemented yet
    return nStatus;
}

