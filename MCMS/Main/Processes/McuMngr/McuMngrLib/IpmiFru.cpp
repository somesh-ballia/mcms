#include "IpmiFru.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "SystemFunctions.h"
#include "ApiStatuses.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StringsLen.h"
#include "GetCntlBoardGeneralInfo.h"
#include "copy_string.h"
#include "GetIFList.h"
#include "GetMacAddr.h"
#include "IpmiEntitySlotIDs.h"
#include "GetNetraBoardGeneralInfo.h"
#include "dsp_monitor_getter.h"
/////////////////////////////////////////////////////////////////////////////
// CIpmiFru
const int DSP_CARD_SLOT_ID_BASE = 6;

CIpmiFru::CIpmiFru()
{
}

CIpmiFru::~CIpmiFru()
{
}

char const * CIpmiFru::NameOf() const
{
    return "CIpmiFru";
}

////////////////////////////////////////////////////////////////////////////
void CIpmiFru::Update(int slotId)
{
     PTRACE2INT(eLevelInfoNormal,"CIpmiFru::Update() slot id  - ",slotId);
     m_curSlotId = slotId;
     switch(slotId)
    {
      case CNTL_SLOT_ID:
      {
         TRACESTR(eLevelInfoNormal) << "\n Simon trace CIpmiFru::Update:  CNTL_SLOT_ID";
          CntlBoardGeneralInfo info;
         GetCntlBoardGeneralInfo(info);
         CopyString(m_fruInfo.boardHardwareVers, info.hwver);
         CopyString(m_fruInfo.boardSoftwareVers, info.swver);
         CopyString(m_fruInfo.boardSerialNumber, info.serial);
         CopyString(m_fruInfo.boardPartNumber, info.partnumber);
         CopyString(m_fruInfo.boardProductName, info.name);
         CopyString(m_fruInfo.riserCardCpldVersion, info.riserCardCpldVersion);//modified by Richer for BRIDGE-12043 ,2014.2.27
         break;
       }
       case RISER_SLOT_ID:
       {
          CopyString(m_fruInfo.boardProductName, "Riser");
          break;
       }
        case DSP_CARD_SLOT_ID_0:
       {
         NetraBoardGeneralInfo info;
         memset(&info, 0, sizeof(info));
         GetNetraBoardGeneralInfo(info,slotId - DSP_CARD_SLOT_ID_BASE);
         CopyString(m_fruInfo.boardProductName, NETRA_DSP_BOARD_NAME);
         CopyString(m_fruInfo.boardHardwareVers, info.hwver);
         CopyString(m_fruInfo.boardSoftwareVers, info.swver);
         CopyString(m_fruInfo.boardSerialNumber, info.serial);
         CopyString(m_fruInfo.riserCardCpldVersion, info.riserCardCpldVersion);//modified by Richer for BRIDGE-12043 ,2014.2.27
         break;
       }
        case DSP_CARD_SLOT_ID_1:
       {
         CopyString(m_fruInfo.boardProductName, NETRA_DSP_BOARD_NAME);
         NetraBoardGeneralInfo info;
         memset(&info, 0, sizeof(info));
         GetNetraBoardGeneralInfo(info,slotId - DSP_CARD_SLOT_ID_BASE);
         CopyString(m_fruInfo.boardHardwareVers, info.hwver);
         CopyString(m_fruInfo.boardSoftwareVers, info.swver);
         CopyString(m_fruInfo.boardSerialNumber, info.serial);
         CopyString(m_fruInfo.riserCardCpldVersion, info.riserCardCpldVersion);//modified by Richer for BRIDGE-12043 ,2014.2.27
          break;
       }
        case DSP_CARD_SLOT_ID_2:
       {
          CopyString(m_fruInfo.boardProductName, NETRA_DSP_BOARD_NAME);
          NetraBoardGeneralInfo info;
         memset(&info, 0, sizeof(info));
         GetNetraBoardGeneralInfo(info,slotId - DSP_CARD_SLOT_ID_BASE);
         CopyString(m_fruInfo.boardHardwareVers, info.hwver);
         CopyString(m_fruInfo.boardSoftwareVers, info.swver);
         CopyString(m_fruInfo.boardSerialNumber, info.serial);
         CopyString(m_fruInfo.riserCardCpldVersion, info.riserCardCpldVersion);//modified by Richer for BRIDGE-12043 ,2014.2.27
          break;
       }
        case ISDN_CARD_SLOT_ID:
       {
          CopyString(m_fruInfo.boardProductName, ISDN_DSP_BOARD_NAME);
          NetraBoardGeneralInfo info;
         memset(&info, 0, sizeof(info));
         GetNetraBoardGeneralInfo(info, RTM_ISDN_BOARD_SLOT_ID);
         CopyString(m_fruInfo.boardHardwareVers, info.hwver);
         CopyString(m_fruInfo.boardSoftwareVers, info.swver);
         CopyString(m_fruInfo.boardSerialNumber, info.serial);
         CopyString(m_fruInfo.riserCardCpldVersion, info.riserCardCpldVersion);//modified by Richer for BRIDGE-12043 ,2014.2.27
          break;
       }
    }
}

void CIpmiFru::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement* pRootNode;

    if (!pFatherNode)
    {
        pFatherNode =  new CXMLDOMElement();
        pFatherNode->set_nodeName("IPMI_FRU");
        pRootNode = pFatherNode;
    }
    else
    {
        pRootNode = pFatherNode->AddChildNode("IPMI_FRU");
    }

    {
        CXMLDOMElement * const pFruNode = pRootNode;
        pFruNode->AddChildNode("BoardSerialNumber", m_fruInfo.boardSerialNumber);
        pFruNode->AddChildNode("BoardFileId", m_fruInfo.boardFileId);
        pFruNode->AddChildNode("BoardMfgDateTime", m_fruInfo.boardMfgDateTime);
        pFruNode->AddChildNode("BoardPartNumber", m_fruInfo.boardPartNumber);
        pFruNode->AddChildNode("ChassisType", m_fruInfo.chassisType);
        pFruNode->AddChildNode("BoardHardwareVers", m_fruInfo.boardHardwareVers);
        pFruNode->AddChildNode("BoardProductName", m_fruInfo.boardProductName);
        pFruNode->AddChildNode("SubBoardID", m_fruInfo.subBoardID);
        pFruNode->AddChildNode("ChassisHwVersion", m_fruInfo.boardSoftwareVers);
        if( CNTL_SLOT_ID  == m_curSlotId)
        {
            vector<IFInfo> ifs;
            GetIFList(ifs);
            int const count = ifs.size();
            for (int i=1; i<=count; ++i)
            {
                char bufNodeName[32];
                snprintf(bufNodeName, sizeof(bufNodeName), "BoardMacAddr%d", i);
                char bufMac[64];
                GetMacAddrWithSemiColon(ifs[i-1].name, bufMac, sizeof(bufMac));
                pFruNode->AddChildNode(bufNodeName, bufMac);
            }
        }
        //Begin:added by richer for displaying DCLP version
        pFruNode->AddChildNode("RiserCardCpldVersion", m_fruInfo.riserCardCpldVersion);
        //End:added by richer for displaying DCLP version
    }
}

///////////////////////////////////////////////////////////////////////////////
int CIpmiFru::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    int nStatus=STATUS_OK;

    return nStatus;
}

