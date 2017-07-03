#include "IpmiEntityReset.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"

CIpmiEntityReset::CIpmiEntityReset()
{
    m_slotID = IPMI_SLOT_ID_RESET;
    m_resetType = RESET_TYPE_NORMAL;
}

CIpmiEntityReset::~CIpmiEntityReset()
{
}

//////////////////////////////////////////////////////////////////////
void CIpmiEntityReset::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	PASSERTMSG(1, "this function should not be called");
}

//////////////////////////////////////////////////////////////////////
int CIpmiEntityReset::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
    STATUS nStatus = STATUS_OK;
    (void)pszError;
    (void)action;
    
    char * value = NULL;

    pActionNode->getChildNodeValueByName("ResetType", &value);
    if (NULL==value)
    {
        m_resetType = RESET_TYPE_NORMAL;
    }
    else
    {
        std::string resetVal(value);
	  if (std::string("255")==resetVal)
        {
            m_resetType = RESET_TYPE_DIAGNOSTIC;
        }
        else
        {
            m_resetType = RESET_TYPE_NORMAL;
        }
    }

    value = NULL;
    pActionNode->getChildNodeValueByName("SlotID", &value);
    if (NULL==value)
    {
        return STATUS_NODE_MISSING;
    }

    std::string slotIdVal(value);
    if (std::string("-1")==slotIdVal)
    {
        m_slotID = IPMI_SLOT_ID_RESET;
    }
    else if (std::string("-2")==slotIdVal)
    {
        m_slotID = IPMI_SLOT_ID_SHUTDOWN;
    }
    else
    {
        return STATUS_NODE_VALUE_INVALID;
    }
	
    return nStatus;
}
