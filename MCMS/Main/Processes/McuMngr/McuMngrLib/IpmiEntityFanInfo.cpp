#include "IpmiEntityFanInfo.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "StringsLen.h"
#include <strings.h>

/////////////////////////////////////////////////////////////////////////////
// CIpmiEntityFanInfo

CIpmiEntityFanInfo::CIpmiEntityFanInfo()
{
	m_entry.maxSpeedLevel = 0;
	m_entry.minSpeedLevel = 0;
	m_entry.normalOperatingLevel = 0;
}

CIpmiEntityFanInfo::~CIpmiEntityFanInfo()
{
}

char const * CIpmiEntityFanInfo::NameOf() const
{
    return "CIpmiEntityFanInfo";
}

////////////////////////////////////////////////////////////////////////////
void CIpmiEntityFanInfo::Update()
{
    bzero(&m_entry, sizeof(m_entry));

    {
        m_entry.maxSpeedLevel = 4;
        m_entry.normalOperatingLevel = 2;
        m_entry.minSpeedLevel = 0;
    }
}

void CIpmiEntityFanInfo::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement* pRootNode;

    if (!pFatherNode)
    {
        pFatherNode =  new CXMLDOMElement();
        pFatherNode->set_nodeName("IPMI_FAN_INFO");
        pRootNode = pFatherNode;
    }
    else
    {
        pRootNode = pFatherNode->AddChildNode("IPMI_FAN_INFO");
    }

    {
        CXMLDOMElement * const pInfoNode = pRootNode;
        pInfoNode->AddChildNode("MaxSpeedLevel", m_entry.maxSpeedLevel);
        pInfoNode->AddChildNode("NormalOperatingLevel", m_entry.normalOperatingLevel);
        pInfoNode->AddChildNode("MinSpeedLevel", m_entry.minSpeedLevel);
    }
}

///////////////////////////////////////////////////////////////////////////////
int CIpmiEntityFanInfo::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    int nStatus=STATUS_OK;

    return nStatus;
}

