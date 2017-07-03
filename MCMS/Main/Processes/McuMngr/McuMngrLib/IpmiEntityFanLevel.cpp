#include "IpmiEntityFanLevel.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "StringsLen.h"
#include <functional>
#include <algorithm>
#include "sensor_read.h"
#include "IpmiSensorDescrToType.h"
#include "IpmiSensorEnums.h"

/////////////////////////////////////////////////////////////////////////////
// CIpmiEntityFanLevel

CIpmiEntityFanLevel::CIpmiEntityFanLevel()
{
}

CIpmiEntityFanLevel::~CIpmiEntityFanLevel()
{
}

char const * CIpmiEntityFanLevel::NameOf() const
{
    return "CIpmiEntityFanLevel";
}

////////////////////////////////////////////////////////////////////////////
void CIpmiEntityFanLevel::Update()
{
    m_levels.clear();

    vector<sensor_t> sensors;
    sensor_read(sensors);
    int const count = sensors.size();
    for (int i=0; i<count; ++i)
    {
        sensor_t const & elem = sensors[i];

        int const sensorType = IpmiSensorDescrToSensorType(elem.SensorName);
        if (IPMI_SENSOR_TYPE_FAN!=sensorType) continue;
        
        m_levels.levels.push_back(IpmiFanLevel((int)elem.CurrentVal));
    }

    m_levels.levelOfAll.fanLevel = IPMI_FAN_LEVEL_DEFAULT;
}

namespace
{
    class FanLevelSummaryAdder
    {
    public:
        FanLevelSummaryAdder(CXMLDOMElement * pLSNode) : pLSNode_(pLSNode) {}

        void operator()(IpmiFanLevel const & level)
        {
            CXMLDOMElement * const pSummaryNode = pLSNode_->AddChildNode("IPMI_FAN_LEVEL_SUMMARY");
            pSummaryNode->AddChildNode("FanLevel", level.fanLevel);
        }
        
    private:
        CXMLDOMElement * const pLSNode_;
    };
    void AddFanLevelSummary(CXMLDOMElement * pLSNode, IpmiFanLevel const & level)
    {
        CXMLDOMElement * const pSummaryNode = pLSNode->AddChildNode("IPMI_FAN_LEVEL_SUMMARY");
        pSummaryNode->AddChildNode("FanLevel", level.fanLevel);
    }
}

void CIpmiEntityFanLevel::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement* pRootNode;

    if (!pFatherNode)
    {
        pFatherNode =  new CXMLDOMElement();
        pFatherNode->set_nodeName("IPMI_FAN_LEVEL_LS");
        pRootNode = pFatherNode;
    }
    else
    {
        pRootNode = pFatherNode->AddChildNode("IPMI_FAN_LEVEL_LS");
    }

    FanLevelSummaryAdder adder(pRootNode);
    AddFanLevelSummary(pRootNode, m_levels.levelOfAll);
    std::for_each(m_levels.levels.begin(), m_levels.levels.end(), adder);
}

///////////////////////////////////////////////////////////////////////////////
int CIpmiEntityFanLevel::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    int nStatus=STATUS_OK;

    return nStatus;
}

