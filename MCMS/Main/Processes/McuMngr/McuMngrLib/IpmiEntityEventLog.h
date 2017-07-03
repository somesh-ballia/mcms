#ifndef __IPMI_ENTITY_EVENT_LOG_H__
#define __IPMI_ENTITY_EVENT_LOG_H__

#include "SerializeObject.h"
#include "SysConfig.h"
#include "McuMngrDefines.h"
#include "McuMngrStructs.h"
#include <strings.h>
#include <vector>
using std::vector;

class CXMLDOMElement;
class CStructTm;

struct IpmiEventSummary
{
    char sensorDescr[16];
    int recordIdLo;
    int recordIdHi;
    int eventDirType;
    int evmRev;
    int eventData3;
    int ipmbSlaveAddr;
    unsigned timestamp;
    int sensorType;
    int sensorNum;
    int eventData1;
    int recordType;
    int eventData2;
    int channelNumber;

    IpmiEventSummary()
    {
        bzero(this, sizeof(*this));
    }
};

class CIpmiEntityEventLog : public CSerializeObject
{
    CLASS_TYPE_1(CIpmiEntityEventLog,CPObject)
public:
    //Constructors
    CIpmiEntityEventLog();
    virtual ~CIpmiEntityEventLog();


    // Implementation
    virtual CSerializeObject* Clone()
    {
        return new CIpmiEntityEventLog;
    }
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    const char*  NameOf() const;

    void Update();

protected:
    vector<IpmiEventSummary> m_events;
};

#endif /* __IPMI_ENTITY_EVENT_LOG_H__ */

