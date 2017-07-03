#include "psosxml.h"
#include "ApiStatuses.h"
#include "Trace.h"
#include "TraceStream.h"
#include "McuIpmiSensorReadingListGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "IpmiSensorReadingList.h"

static int g_slotId = -1;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMcuIpmiSensorReadingListGet::CMcuIpmiSensorReadingListGet()
    : m_pCpuMemUsageGetter(0)
{
    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiSensorReadingListGet& CMcuIpmiSensorReadingListGet::operator = (const CMcuIpmiSensorReadingListGet &other)
{
	if(this == &other){
	    return *this;
	}

    m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
    m_updateCounter = other.m_updateCounter;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuIpmiSensorReadingListGet::~CMcuIpmiSensorReadingListGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CMcuIpmiSensorReadingListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
    PTRACE2INT(eLevelError, "CMcuIpmiSensorReadingListGet::SerializeXml slot id = ", g_slotId);
    CIpmiSensorReadingList list;
    list.Update(g_slotId, m_pCpuMemUsageGetter);
    list.SerializeXml(pActionsNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuIpmiSensorReadingListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
    int nStatus = STATUS_OK;
    //GET_VALIDATE_CHILD(pNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD); Not implemented yet
    DWORD tmp = -1;
    GET_VALIDATE_CHILD(pNode, "SlotID", &tmp, _0_TO_DWORD);
    g_slotId = tmp;
    PTRACE2INT(eLevelError, "CMcuIpmiSensorReadingListGet::DeSerializeXml slot id = ", g_slotId);
    return nStatus;
}

