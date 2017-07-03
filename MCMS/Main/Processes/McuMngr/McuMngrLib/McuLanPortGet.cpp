#include "McuLanPortGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "psosxml.h"
#include "LanPort.h"
#include "LanPortList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMcuLanPortGet::CMcuLanPortGet()
{
    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CMcuLanPortGet& CMcuLanPortGet::operator = (const CMcuLanPortGet &other)
{
	if(this == &other){
	    return *this;
	}

    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = other.m_updateCounter;
    m_slotID = other.m_slotID;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuLanPortGet::~CMcuLanPortGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CMcuLanPortGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
    CLanPort port;
    port.Update(m_slotID-LAN_SLOT_ID_START);
    port.SerializeXml(pActionsNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuLanPortGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
    int nStatus = STATUS_OK;
    
    DWORD tmp = (DWORD)m_slotID;
	GET_VALIDATE_CHILD(pNode, "SlotID", &tmp, _0_TO_DWORD);
	m_slotID = tmp;
    
    return nStatus;
}

