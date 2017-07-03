#ifndef __IPMI_SENSOR_READING_LIST_H__
#define __IPMI_SENSOR_READING_LIST_H__

#include "SerializeObject.h"
#include "SysConfig.h"
#include "McuMngrDefines.h"
#include "McuMngrStructs.h"
#include <vector>
using std::vector;

class CXMLDOMElement;
class CStructTm;
class CSysState;

struct IpmiSensorReadingSummary
{
    int sensorState1;
    int sensorState2;
    int sensorNumber;
    int slotID;
    float sensorReading;
};

class CIpmiSensorReadingList : public CSerializeObject
{
    CLASS_TYPE_1(CIpmiSensorReadingList,CPObject)
public:
    //Constructors
    CIpmiSensorReadingList();
    virtual ~CIpmiSensorReadingList();


    // Implementation
    virtual CSerializeObject* Clone()
    {
        return new CIpmiSensorReadingList;
    }
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    const char*  NameOf() const;

    void Update(int slotId, CSysState * pCpuMemUsageGetter);

protected:
    vector<IpmiSensorReadingSummary> m_readings;
    IpmiSensorReadingSummary m_cpuUsage;
    IpmiSensorReadingSummary m_memUsage;
};

#endif /* __IPMI_SENSOR_READING_LIST_H__ */

