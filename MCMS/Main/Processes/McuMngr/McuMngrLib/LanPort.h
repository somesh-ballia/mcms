#ifndef __IPMI_LAN_PORT_H__
#define __IPMI_LAN_PORT_H__

#include "SerializeObject.h"
#include "SysConfig.h"
#include "McuMngrDefines.h"
#include "McuMngrStructs.h"
#include <strings.h>

class CXMLDOMElement;
class CStructTm;

struct LanPortInfo
{
    unsigned portID;

    unsigned actLinkAutoNeg;
    unsigned actLinkStatus;
    unsigned actLinkMode;
    unsigned advLinkAutoNeg;
    unsigned advLinkMode;

    char txOctets[24];
    char maxTxOctets[24];
    char txPackets[24];
    char maxTxPackets[24];
    char txBadPackets[24];
    char maxTxBadPackets[24];
    char txFifoDrops[24];
    char maxTxFifoDrops[24];

    char rxOctets[24];
    char maxRxOctets[24];
    char rxPackets[24];
    char maxRxPackets[24];
    char rxBadPackets[24];
    char maxRxBadPackets[24];
    char rxCRC[24];
    char maxRxCRC[24];
};

class CLanPort : public CSerializeObject
{
    CLASS_TYPE_1(CLanPort,CPObject)
public:
    //Constructors
    CLanPort();
    virtual ~CLanPort();


    // Implementation
    virtual CSerializeObject* Clone()
    {
        return new CLanPort;
    }
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    const char*  NameOf() const;

    void Update(int lanNo);

protected:
    LanPortInfo m_portInfo;
};

#endif /* __IPMI_LAN_PORT_H__ */

