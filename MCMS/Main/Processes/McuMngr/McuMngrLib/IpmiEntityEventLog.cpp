#include "IpmiEntityEventLog.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "StringsLen.h"
#include <time.h>

/////////////////////////////////////////////////////////////////////////////
// CIpmiEntityEventLog

CIpmiEntityEventLog::CIpmiEntityEventLog()
{
}

CIpmiEntityEventLog::~CIpmiEntityEventLog()
{
}

char const * CIpmiEntityEventLog::NameOf() const
{
    return "CIpmiEntityEventLog";
}

////////////////////////////////////////////////////////////////////////////
void CIpmiEntityEventLog::Update()
{
    m_events.clear();

    {
        IpmiEventSummary event;

        {
            strcpy(event.sensorDescr, "FAN 3");
            event.recordIdLo = 137;
            event.recordIdHi = 0;
            event.eventDirType = 1;
            event.evmRev = 4;
            event.eventData3 = 255;
            event.ipmbSlaveAddr = 32;
            event.timestamp = time(0);
            event.sensorType = 4;
            event.sensorNum = 13;
            event.eventData1 = 7;
            event.recordType = 0;
            event.eventData2 = 255;
            event.channelNumber = 0;
        }
        
        m_events.push_back(event);
    }
}

void CIpmiEntityEventLog::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement* pRootNode;

    if (!pFatherNode)
    {
        pFatherNode =  new CXMLDOMElement();
        pFatherNode->set_nodeName("IPMI_LIST_SEL");
        pRootNode = pFatherNode;
    }
    else
    {
        pRootNode = pFatherNode->AddChildNode("IPMI_LIST_SEL");
    }

    int const count = m_events.size();
    for (int i=0; i<count; ++i)
    {
        IpmiEventSummary const & event = m_events[i];
        
        CXMLDOMElement * const pEventNode = pRootNode->AddChildNode("EVENT_SUMMARY");
        pEventNode->AddChildNode("SensorDescr", event.sensorDescr);
        pEventNode->AddChildNode("RecordIdLo", event.recordIdLo);
        pEventNode->AddChildNode("RecordIdHi", event.recordIdHi);
        pEventNode->AddChildNode("EventDirType", event.eventDirType);
        pEventNode->AddChildNode("EvmRev", event.evmRev);
        pEventNode->AddChildNode("EventData3", event.eventData3);
        pEventNode->AddChildNode("IpmbSlaveAddr", event.ipmbSlaveAddr);
        pEventNode->AddChildNode("Timestamp", event.timestamp);
        pEventNode->AddChildNode("SensorType", event.sensorType);
        pEventNode->AddChildNode("SensorNum", event.sensorNum);
        pEventNode->AddChildNode("EventData1", event.eventData1);
        pEventNode->AddChildNode("RecordType", event.recordType);
        pEventNode->AddChildNode("EventData2", event.eventData2);
        pEventNode->AddChildNode("ChannelNumber", event.channelNumber);
    }
}

///////////////////////////////////////////////////////////////////////////////
int CIpmiEntityEventLog::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    int nStatus=STATUS_OK;

    return nStatus;
}

