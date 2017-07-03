#include "IpmiSensorList.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "StringsLen.h"
#include "sensor_read.h"
#include "IpmiSensorDescrToType.h"
#include "IpmiSensorEnums.h"
#include "copy_string.h"
#include "to_string.h"
#include "GetCpuMemUsage.h"
#include "IpmiEntitySlotIDs.h"
#include <strings.h>
#include "Trace.h"

/////////////////////////////////////////////////////////////////////////////
// CIpmiSensorList

CIpmiSensorList::CIpmiSensorList()
{
    bzero(&m_cpuUsage, sizeof(m_cpuUsage));
    bzero(&m_memUsage, sizeof(m_memUsage));
}

CIpmiSensorList::~CIpmiSensorList()
{
}

char const * CIpmiSensorList::NameOf() const
{
    return "CIpmiSensorList";
}

////////////////////////////////////////////////////////////////////////////
void CIpmiSensorList::Update(int slotId, CSysState * pCpuMemUsageGetter)
{
    m_sensors.clear();

    vector<sensor_t> sensors;
    sensor_read(sensors);
    int const count = sensors.size();
    for (int i=0; i<count; ++i)
    {
        sensor_t const & item = sensors[i];

        int const sensorType = IpmiSensorDescrToSensorType(item.SensorName);
        if (IPMI_SENSOR_TYPE_FAN==sensorType) continue;
        if(slotId != IpmiSensorDescrToSlotID(item.SensorName)) continue;
        
        IpmiSensorSummary summary;
        summary.nominalVal = IpmiSensorDescrToNominalVal(item.SensorName);
        summary.slotID = slotId;
        summary.lowerNonRecoverable = item.LowerNonRecoverable;
        summary.entityId = IpmiSensorDescrToEntityType(item.SensorName);
        summary.lowerCritical = item.LowerCritical;
        summary.upperNonRecoverable = item.UpperNonRecoverable;
        CopyString(summary.sensorDescr, item.SensorName);
        summary.lowerNonCritical = item.LowerNonCritical;
        summary.upperCritical = item.UpperCritical;
        summary.sensorNumber = IpmiSensorDescrToSensorNumber(item.SensorName);
        summary.sensorType = IpmiSensorDescrToSensorType(item.SensorName);
        summary.eventReadType = item.eventReadType;
        summary.entityInstance = 96;
        summary.upperNonCritical = item.UpperNonCritical;

        m_sensors.push_back(summary);
    }

    if ((CNTL_SLOT_ID == slotId) && pCpuMemUsageGetter)
    {
        {
            IpmiSensorSummary & summary = m_cpuUsage;
            char const * sensorName = "CPU Usage";
            summary.nominalVal = IpmiSensorDescrToNominalVal(sensorName);
            summary.slotID = CNTL_SLOT_ID;
            summary.entityId = IpmiSensorDescrToEntityType(sensorName);
            CopyString(summary.sensorDescr, sensorName);
            summary.sensorNumber = IpmiSensorDescrToSensorNumber(sensorName);
            summary.sensorType = IpmiSensorDescrToSensorType(sensorName);
            summary.eventReadType = EVENT_READ_TYPE_NO_COLOR;
            summary.entityInstance = 96;

            summary.upperNonRecoverable = 1001.0;
            summary.upperCritical = 990.0;
            summary.upperNonCritical = 950.0;

            summary.lowerNonRecoverable = 0.0;
            summary.lowerCritical = 1.0;
            summary.lowerNonCritical = 2.0;
        }

        {
            IpmiSensorSummary & summary = m_memUsage;
            char const * sensorName = "Memory Usage";
            summary.nominalVal = IpmiSensorDescrToNominalVal(sensorName);
            summary.slotID = CNTL_SLOT_ID;
            summary.entityId = IpmiSensorDescrToEntityType(sensorName);
            CopyString(summary.sensorDescr, sensorName);
            summary.sensorNumber = IpmiSensorDescrToSensorNumber(sensorName);
            summary.sensorType = IpmiSensorDescrToSensorType(sensorName);
            summary.eventReadType = EVENT_READ_TYPE_NO_COLOR;
            summary.entityInstance = 96;

            summary.upperNonRecoverable = 1001.0;
            summary.upperCritical = 990.0;
            summary.upperNonCritical = 950.0;

            summary.lowerNonRecoverable = 0.0;
            summary.lowerCritical = 1.0;
            summary.lowerNonCritical = 2.0;
        }
    }
}

void CIpmiSensorList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement* pRootNode;

    if (!pFatherNode)
    {
        pFatherNode =  new CXMLDOMElement();
        pFatherNode->set_nodeName("SENSOR_SUMMARY_LS");
        pRootNode = pFatherNode;
    }
    else
    {
        pRootNode = pFatherNode->AddChildNode("SENSOR_SUMMARY_LS");
    }

    int const count = m_sensors.size();
    for (int i=0; i<count; ++i)
    {
        IpmiSensorSummary const & sensor = m_sensors[i];
        
        CXMLDOMElement * const pSensorNode = pRootNode->AddChildNode("SENSOR_SUMMARY");
        pSensorNode->AddChildNode("NominalVal", FloatToString(sensor.nominalVal));
        pSensorNode->AddChildNode("SlotID", sensor.slotID);
        pSensorNode->AddChildNode("LowerNonRecoverable", FloatToString(sensor.lowerNonRecoverable));
        pSensorNode->AddChildNode("EntityId", sensor.entityId);
        pSensorNode->AddChildNode("LowerCritical", FloatToString(sensor.lowerCritical));
        pSensorNode->AddChildNode("UpperNonRecoverable", FloatToString(sensor.upperNonRecoverable));
        pSensorNode->AddChildNode("SensorDescr", sensor.sensorDescr);
        pSensorNode->AddChildNode("LowerNonCritical", FloatToString(sensor.lowerNonCritical));
        pSensorNode->AddChildNode("UpperCritical", FloatToString(sensor.upperCritical));
        pSensorNode->AddChildNode("SensorNumber", sensor.sensorNumber);
        pSensorNode->AddChildNode("SensorType", sensor.sensorType);
        pSensorNode->AddChildNode("EventReadType", sensor.eventReadType);
        pSensorNode->AddChildNode("EntityInstance", sensor.entityInstance);
        pSensorNode->AddChildNode("UpperNonCritical", FloatToString(sensor.upperNonCritical));
    }

    {
        IpmiSensorSummary const & sensor = m_cpuUsage;

        if(CNTL_SLOT_ID == sensor.slotID)
        {
            CXMLDOMElement * const pSensorNode = pRootNode->AddChildNode("SENSOR_SUMMARY");
            pSensorNode->AddChildNode("NominalVal", FloatToPercentString(sensor.nominalVal));
            pSensorNode->AddChildNode("SlotID", sensor.slotID);
            pSensorNode->AddChildNode("LowerNonRecoverable", FloatToPercentString(sensor.lowerNonRecoverable));
            pSensorNode->AddChildNode("EntityId", sensor.entityId);
            pSensorNode->AddChildNode("LowerCritical", FloatToPercentString(sensor.lowerCritical));
            pSensorNode->AddChildNode("UpperNonRecoverable", FloatToPercentString(sensor.upperNonRecoverable));
            pSensorNode->AddChildNode("SensorDescr", sensor.sensorDescr);
            pSensorNode->AddChildNode("LowerNonCritical", FloatToPercentString(sensor.lowerNonCritical));
            pSensorNode->AddChildNode("UpperCritical", FloatToPercentString(sensor.upperCritical));
            pSensorNode->AddChildNode("SensorNumber", sensor.sensorNumber);
            pSensorNode->AddChildNode("SensorType", sensor.sensorType);
            pSensorNode->AddChildNode("EventReadType", sensor.eventReadType);
            pSensorNode->AddChildNode("EntityInstance", sensor.entityInstance);
            pSensorNode->AddChildNode("UpperNonCritical", FloatToPercentString(sensor.upperNonCritical));
        }
    }

    {
        IpmiSensorSummary const & sensor = m_memUsage;
        if(CNTL_SLOT_ID == sensor.slotID)
        {        
            CXMLDOMElement * const pSensorNode = pRootNode->AddChildNode("SENSOR_SUMMARY");
            pSensorNode->AddChildNode("NominalVal", FloatToPercentString(sensor.nominalVal));
            pSensorNode->AddChildNode("SlotID", sensor.slotID);
            pSensorNode->AddChildNode("LowerNonRecoverable", FloatToPercentString(sensor.lowerNonRecoverable));
            pSensorNode->AddChildNode("EntityId", sensor.entityId);
            pSensorNode->AddChildNode("LowerCritical", FloatToPercentString(sensor.lowerCritical));
            pSensorNode->AddChildNode("UpperNonRecoverable", FloatToPercentString(sensor.upperNonRecoverable));
            pSensorNode->AddChildNode("SensorDescr", sensor.sensorDescr);
            pSensorNode->AddChildNode("LowerNonCritical", FloatToPercentString(sensor.lowerNonCritical));
            pSensorNode->AddChildNode("UpperCritical", FloatToPercentString(sensor.upperCritical));
            pSensorNode->AddChildNode("SensorNumber", sensor.sensorNumber);
            pSensorNode->AddChildNode("SensorType", sensor.sensorType);
            pSensorNode->AddChildNode("EventReadType", sensor.eventReadType);
            pSensorNode->AddChildNode("EntityInstance", sensor.entityInstance);
            pSensorNode->AddChildNode("UpperNonCritical", FloatToPercentString(sensor.upperNonCritical));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
int CIpmiSensorList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    int nStatus=STATUS_OK;

    return nStatus;
}

