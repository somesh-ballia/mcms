#include "McuLanPortListGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "LanPortList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMcuLanPortListGet::CMcuLanPortListGet()
{
    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CMcuLanPortListGet& CMcuLanPortListGet::operator = (const CMcuLanPortListGet &other)
{
	if(this == &other){
	    return *this;
	}

    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = other.m_updateCounter;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuLanPortListGet::~CMcuLanPortListGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CMcuLanPortListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
    CLanPortList list;
    list.Update();
    list.SerializeXml(pActionsNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuLanPortListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
    int nStatus = STATUS_OK;
    //GET_VALIDATE_CHILD(pNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD); Not implemented yet
    return nStatus;
}

