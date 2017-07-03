#include "IpmiSensorReadingList.h"
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
#include "to_string.h"
#include "IpmiEntitySlotIDs.h"
#include "GetCpuMemUsage.h"

namespace
{
    int GetUsageReadingState1(int usage)
    {
        if (usage>998)
        {
            return IPMI_SENSOR_STATE1_UPPER_NON_RECOVERABLE_GOING_HIGH;
        }
        else if (usage>980)
        {
            return IPMI_SENSOR_STATE1_UPPER_CRITICAL_GOING_HIGH;
        }
        else if (usage>900)
        {
            return IPMI_SENSOR_STATE1_UPPER_MAJOR_GOING_HIGH;
        }
        else
        {
            return IPMI_SENSOR_STATE1_NORMAL;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// CIpmiSensorReadingList

CIpmiSensorReadingList::CIpmiSensorReadingList()
{
    bzero(&m_cpuUsage, sizeof(m_cpuUsage));
    bzero(&m_memUsage, sizeof(m_memUsage));
}

CIpmiSensorReadingList::~CIpmiSensorReadingList()
{
}

char const * CIpmiSensorReadingList::NameOf() const
{
    return "CIpmiSensorReadingList";
}

////////////////////////////////////////////////////////////////////////////
void CIpmiSensorReadingList::Update(int slotId, CSysState * pCpuMemUsageGetter)
{
    m_readings.clear();

    vector<sensor_t> sensors;
    sensor_read(sensors);

    int const count = sensors.size();
    for (int i=0; i<count; ++i)
    {
        sensor_t const & item = sensors[i];
        
        int const sensorType = IpmiSensorDescrToSensorType(item.SensorName);
        if (IPMI_SENSOR_TYPE_FAN==sensorType) continue;
        if(slotId != IpmiSensorDescrToSlotID(item.SensorName)) continue;

        IpmiSensorReadingSummary summary;
        summary.sensorState1 = GetSensorReadingState1(item);
        summary.sensorState2 = 255;
        summary.sensorNumber = IpmiSensorDescrToSensorNumber(item.SensorName);
        summary.slotID = slotId;
        summary.sensorReading = item.CurrentVal;

        m_readings.push_back(summary);
    }

    if ((CNTL_SLOT_ID == slotId) && pCpuMemUsageGetter)
    {
        {
            int const cpuUsage = pCpuMemUsageGetter->GetCPUUsage(); (void)cpuUsage;

            IpmiSensorReadingSummary & summary = m_cpuUsage;
            char const * sensorName = "CPU Usage";
            summary.sensorState1 = GetUsageReadingState1(cpuUsage);
            summary.sensorState2 = 255;
            summary.sensorNumber = IpmiSensorDescrToSensorNumber(sensorName);
            summary.slotID = CNTL_SLOT_ID;
            summary.sensorReading = cpuUsage;
        }

        {
            int const memUsage = pCpuMemUsageGetter->GetMemUsage(); (void)memUsage;
            
            IpmiSensorReadingSummary & summary = m_memUsage;
            char const * sensorName = "Memory Usage";
            summary.sensorState1 = GetUsageReadingState1(memUsage);
            summary.sensorState2 = 255;
            summary.sensorNumber = IpmiSensorDescrToSensorNumber(sensorName);
            summary.slotID = CNTL_SLOT_ID;
            summary.sensorReading = memUsage;
        }
    }
}

void CIpmiSensorReadingList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement* pRootNode;

    if (!pFatherNode)
    {
        pFatherNode =  new CXMLDOMElement();
        pFatherNode->set_nodeName("SENSOR_READING_SUMMARY_LS");
        pRootNode = pFatherNode;
    }
    else
    {
        pRootNode = pFatherNode->AddChildNode("SENSOR_READING_SUMMARY_LS");
    }

    int const count = m_readings.size();
    for (int i=0; i<count; ++i)
    {
        IpmiSensorReadingSummary const & reading = m_readings[i];
        
        CXMLDOMElement * const pReadingNode = pRootNode->AddChildNode("SENSOR_READING_SUMMARY");
        pReadingNode->AddChildNode("SensorState1", reading.sensorState1);
        pReadingNode->AddChildNode("SensorState2", reading.sensorState2);
        pReadingNode->AddChildNode("SensorNumber", reading.sensorNumber);
        pReadingNode->AddChildNode("SlotID", reading.slotID);
        pReadingNode->AddChildNode("SensorReading", FloatToString(reading.sensorReading));
    }

    {
        IpmiSensorReadingSummary const & reading = m_cpuUsage;
        if(CNTL_SLOT_ID == reading.slotID)
        {   
            CXMLDOMElement * const pReadingNode = pRootNode->AddChildNode("SENSOR_READING_SUMMARY");
            pReadingNode->AddChildNode("SensorState1", reading.sensorState1);
            pReadingNode->AddChildNode("SensorState2", reading.sensorState2);
            pReadingNode->AddChildNode("SensorNumber", reading.sensorNumber);
            pReadingNode->AddChildNode("SlotID", reading.slotID);
            pReadingNode->AddChildNode("SensorReading", FloatToPercentString(reading.sensorReading));
        }
    }

    {
        IpmiSensorReadingSummary const & reading = m_memUsage;
        if(CNTL_SLOT_ID == reading.slotID)
        {           
            CXMLDOMElement * const pReadingNode = pRootNode->AddChildNode("SENSOR_READING_SUMMARY");
            pReadingNode->AddChildNode("SensorState1", reading.sensorState1);
            pReadingNode->AddChildNode("SensorState2", reading.sensorState2);
            pReadingNode->AddChildNode("SensorNumber", reading.sensorNumber);
            pReadingNode->AddChildNode("SlotID", reading.slotID);
            pReadingNode->AddChildNode("SensorReading", FloatToPercentString(reading.sensorReading));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
int CIpmiSensorReadingList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    int nStatus=STATUS_OK;

    return nStatus;
}

