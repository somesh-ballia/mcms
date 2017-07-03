#include "IpmiEntityList.h"
#include "IpmiConsts.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "StringsLen.h"
#include "GetCntlBoardGeneralInfo.h"

/////////////////////////////////////////////////////////////////////////////
// CIpmiEntityList

CIpmiEntityList::CIpmiEntityList()
{
}

CIpmiEntityList::~CIpmiEntityList()
{
}

char const * CIpmiEntityList::NameOf() const
{
    return "CIpmiEntityList";
}

char const * ConvertCardType(char const * originType, char * buf, size_t bufLen)
{
    if (0==strcmp(originType, CNTL_BOARD_NAME))
    {
        return GetCntlBoardName(buf, bufLen);
    }
    else if(NULL != strstr(originType, "DSP Card"))
    {
        return strncpy(buf, "DSP Card", bufLen - 1);
    }
    else
    {
        return originType;
    }
}

////////////////////////////////////////////////////////////////////////////
void CIpmiEntityList::Update()
{
    m_cardContents.clear();
    (*CollectCardContents)(m_cardContents);
}

void CIpmiEntityList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement* pRootNode;

    if (!pFatherNode)
    {
        pFatherNode =  new CXMLDOMElement();
        pFatherNode->set_nodeName("CARD_SUMMARY_LS");
        pRootNode = pFatherNode;
    }
    else
    {
        pRootNode = pFatherNode->AddChildNode("CARD_SUMMARY_LS");
    }

    int const childs = m_cardContents.size();
    for (int i=0; i<childs; ++i)
    {
        CardContent const & card = m_cardContents[i];
        
        CXMLDOMElement * const pCardNode = pRootNode->AddChildNode("CARD_CONTENT");
        pCardNode->AddChildNode("Temperature", card.temperature);
        pCardNode->AddChildNode("SlotID", card.slotID);
        pCardNode->AddChildNode("Status", card.status);
        pCardNode->AddChildNode("IpmbAddress", card.ipmbAddress);
        pCardNode->AddChildNode("Voltage", card.voltage);
        pCardNode->AddChildNode("NumMezzanine", card.numMezzanine);
        pCardNode->AddChildNode("SubBoardID", card.subBoardId);
        char bufCardType[64];
        pCardNode->AddChildNode("CardType", ConvertCardType(card.cardType, bufCardType, sizeof(bufCardType)));
        pCardNode->AddChildNode("BitFail", card.bitFail);
        pCardNode->AddChildNode("ENTITY_SUMMARY_LIST");
    }
}

///////////////////////////////////////////////////////////////////////////////
int CIpmiEntityList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    int nStatus=STATUS_OK;

    return nStatus;
}

