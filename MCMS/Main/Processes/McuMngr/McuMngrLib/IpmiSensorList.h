#ifndef __IPMI_SENSOR_LIST_H__
#define __IPMI_SENSOR_LIST_H__

#include "SerializeObject.h"
#include "SysConfig.h"
#include "McuMngrDefines.h"
#include "McuMngrStructs.h"
#include <strings.h>
#include <vector>
using std::vector;

class CXMLDOMElement;
class CStructTm;
class CSysState;

struct IpmiSensorSummary
{
    float nominalVal;
    int slotID;
    float lowerNonRecoverable;
    int entityId;
    float lowerCritical;
    float upperNonRecoverable;
    char sensorDescr[16];
    float lowerNonCritical;
    float upperCritical;
    int sensorNumber;
    int sensorType;
    int eventReadType;
    int entityInstance;
    float upperNonCritical;

    IpmiSensorSummary()
    {
        bzero(this, sizeof(*this));
    }
};

class CIpmiSensorList : public CSerializeObject
{
    CLASS_TYPE_1(CIpmiSensorList,CPObject)
public:
    //Constructors
    CIpmiSensorList();
    virtual ~CIpmiSensorList();


    // Implementation
    virtual CSerializeObject* Clone()
    {
        return new CIpmiSensorList;
    }
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    const char*  NameOf() const;

    void Update(int slotId, CSysState * pCpuMemUsageGetter);

protected:
    vector<IpmiSensorSummary> m_sensors;
    IpmiSensorSummary m_cpuUsage;
    IpmiSensorSummary m_memUsage;
};

#endif /* __IPMI_SENSOR_LIST_H__ */

