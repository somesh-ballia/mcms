#include "LanPort.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "StringsLen.h"
#include "GetIFFeatures.h"
#include "GetIFStatus.h"
#include "GetIFStatistics.h"
#include "GetIFFeatures.h"
#include "copy_string.h"
#include "IpmiSensorEnums.h"
#include "GetEthBondStatus.h"

/////////////////////////////////////////////////////////////////////////////
// CLanPort

CLanPort::CLanPort()
{
}

CLanPort::~CLanPort()
{
}

char const * CLanPort::NameOf() const
{
    return "CLanPort";
}

////////////////////////////////////////////////////////////////////////////
void CLanPort::Update(int lanNo)
{
    {
        bzero(&m_portInfo, sizeof(m_portInfo));
        
        struct EthBondingInfo bondingInfo;
        GetEthBondingInfo(bondingInfo);

        {
            IFFeatures ifFeatures;
            (*GetIFFeatures)(lanNo, &ifFeatures);

            m_portInfo.actLinkAutoNeg = ifFeatures.activeAutoNeg;
            m_portInfo.actLinkMode = ifFeatures.activeLinkMode;
            int isLinkUp = false;
            if ((*IsLinkUp)(lanNo, isLinkUp))
            {
                m_portInfo.actLinkStatus = GetLinkStatusWithBondingInfo(lanNo, isLinkUp, bondingInfo);
            }
            else
            {
                m_portInfo.actLinkStatus = LAN_STATUS_INACTIVE;
            }

            m_portInfo.advLinkAutoNeg = ifFeatures.advertisedAutoNeg;
            m_portInfo.advLinkMode = ifFeatures.advertisedLinkMode;
        }

        {
            IFStatistics ifStatistics;
            GetIFStatistics(lanNo, &ifStatistics);

            CopyString(m_portInfo.txOctets, ifStatistics.bytesSent);
            CopyString(m_portInfo.maxTxOctets, ifStatistics.bytesSent);
            CopyString(m_portInfo.txPackets, ifStatistics.pktsSent);
            CopyString(m_portInfo.maxTxPackets, ifStatistics.pktsSent);
            CopyString(m_portInfo.txBadPackets, ifStatistics.pktsSentError);
            CopyString(m_portInfo.maxTxBadPackets, ifStatistics.pktsSentError);
            CopyString(m_portInfo.txFifoDrops, ifStatistics.pktsSentFifoError);
            CopyString(m_portInfo.maxTxFifoDrops, ifStatistics.pktsSentFifoError);
            
            CopyString(m_portInfo.rxOctets, ifStatistics.bytesRecved);
            CopyString(m_portInfo.maxRxOctets, ifStatistics.bytesRecved);
            CopyString(m_portInfo.rxPackets, ifStatistics.pktsRecved);
            CopyString(m_portInfo.maxRxPackets, ifStatistics.pktsRecved);
            CopyString(m_portInfo.rxBadPackets, ifStatistics.pktsRecvedError);
            CopyString(m_portInfo.maxRxBadPackets, ifStatistics.pktsRecvedError);
            CopyString(m_portInfo.rxCRC, ifStatistics.pktsRecvedFrameError);
            CopyString(m_portInfo.maxRxCRC, ifStatistics.pktsRecvedFrameError);
        }
    }
}

void CLanPort::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement* pRootNode;

    if (!pFatherNode)
    {
        pFatherNode =  new CXMLDOMElement();
        pFatherNode->set_nodeName("LAN_PORT");
        pRootNode = pFatherNode;
    }
    else
    {
        pRootNode = pFatherNode->AddChildNode("LAN_PORT");
    }

    {
        CXMLDOMElement * const pLanNode = pRootNode;
        LanPortInfo const & lan = m_portInfo;
        pLanNode->AddChildNode("MaxTxOctets", lan.maxTxOctets);
        pLanNode->AddChildNode("TxPackets", lan.txPackets);
        pLanNode->AddChildNode("MaxTxBadPackets", lan.maxTxBadPackets);
        pLanNode->AddChildNode("ActLinkAutoNeg", lan.actLinkAutoNeg);
        pLanNode->AddChildNode("ActLinkStatus", lan.actLinkStatus);
        pLanNode->AddChildNode("RxOctets", lan.rxOctets);
        pLanNode->AddChildNode("MaxRxCRC", lan.maxRxCRC);
        pLanNode->AddChildNode("TxBadPackets", lan.txBadPackets);
        pLanNode->AddChildNode("TxOctets", lan.txOctets);
        pLanNode->AddChildNode("RxPackets", lan.rxPackets);
        pLanNode->AddChildNode("MaxRxOctets", lan.maxRxOctets);
        pLanNode->AddChildNode("AdvLinkAutoNeg", lan.advLinkAutoNeg);
        pLanNode->AddChildNode("RxCRC", lan.rxCRC);
        pLanNode->AddChildNode("MaxTxPackets", lan.maxTxPackets);
        pLanNode->AddChildNode("ActLinkMode", lan.actLinkMode);
        pLanNode->AddChildNode("MaxRxBadPackets", lan.maxRxBadPackets);
        pLanNode->AddChildNode("MaxRxPackets", lan.maxRxPackets);
        pLanNode->AddChildNode("PortID", lan.portID);
        pLanNode->AddChildNode("TxFifoDrops", lan.txFifoDrops);
        pLanNode->AddChildNode("AdvLinkMode", lan.advLinkMode);
        pLanNode->AddChildNode("RxBadPackets", lan.rxBadPackets);
        pLanNode->AddChildNode("MaxTxFifoDrops", lan.maxTxFifoDrops);
    }
}

///////////////////////////////////////////////////////////////////////////////
int CLanPort::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    int nStatus=STATUS_OK;

    return nStatus;
}

