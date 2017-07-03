#ifndef __IPMI_LAN_PORT_LIST_H__
#define __IPMI_LAN_PORT_LIST_H__

#include "SerializeObject.h"
#include "SysConfig.h"
#include "McuMngrDefines.h"
#include "McuMngrStructs.h"
#include "IpmiEntitySlotIDs.h"
#include <vector>
using std::vector;

class CXMLDOMElement;
class CStructTm;

struct LanPortSummary
{
    int status;
    int portID;
    int slotID;
    APIU32 e802_1xSuppPortStatus;//added by Richer for 802.1x project om 2013.12.26
    APIU32 e802_1xMethod;//added by Richer for 802.1x project om 2013.12.26
    APIU32 e802_1xFailReason;//added by Richer for 802.1x project om 2013.12.26
};

class CLanPortList : public CSerializeObject
{
    CLASS_TYPE_1(CLanPortList,CPObject)
public:
    //Constructors
    CLanPortList();
    virtual ~CLanPortList();


    // Implementation
    virtual CSerializeObject* Clone()
    {
        return new CLanPortList;
    }
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    const char*  NameOf() const;

    void Update();

protected:
    vector<LanPortSummary> m_lans;
};

//added by Richer for 802.1x project om 2013.12.26
#define P802_1x_STR_SUP_PORT_STAT "suppPortStatus="
#define P802_1x_STR_EAP_STATE "EAP state="
#define P802_1x_STR_STATUS_COMMAND_FAILED "'STATUS' command failed"
#define P802_1x_STR_FAILED_TO_CONNECT "Failed to connect to wpa_supplicant"
#define P802_1x_STR_UNAUTHORIZED "Unauthorized"
#define P802_1x_STR_AUTHORIZED "Authorized"
#define P802_1x_STR_IDLE "IDLE"
#define P802_1x_STR_FAILURE "FAILURE"
#define P802_1x_STR_SUCCESS "SUCCESS"

#define P802_1x_STR_METHOD_TYPE "selectedMethod="
#define P802_1x_STR_METHOD_MD5_TYPE "EAP-MD5"
#define P802_1x_STR_METHOD_CHAP_TYPE "EAP-PEAP"
#define P802_1x_STR_METHOD_TLS_TYPE "EAP-TLS"

#define P802_1x_STR_METHOD_TYPE_FROM_CONF_FILE      "eap="
#define P802_1x_STR_METHOD_MD5_TYPE_FROM_CONF_FILE  "MD5"
#define P802_1x_STR_METHOD_CHAP_TYPE_FROM_CONF_FILE "PEAP"
#define P802_1x_STR_METHOD_TLS_TYPE_FROM_CONF_FILE  "TLS"
//extern int errno;

int Get802_1xStatus(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);
int Get802_1xMethodType(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);

#endif /* __IPMI_LAN_PORT_LIST_H__ */

