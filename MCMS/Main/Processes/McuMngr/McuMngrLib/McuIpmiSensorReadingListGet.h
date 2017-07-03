#ifndef __MCU_IPMI_SENSOR_READING_LIST_GET_H__
#define __MCU_IPMI_SENSOR_READING_LIST_GET_H__

#include "SerializeObject.h"
#include "McuMngrProcess.h"

class CSysState;

class CMcuIpmiSensorReadingListGet : public CSerializeObject
{
    CLASS_TYPE_1(CMcuIpmiSensorReadingListGet, CSerializeObject)
public:

    //Constructors
    CMcuIpmiSensorReadingListGet();
    virtual const char* NameOf() const
    {
        return "CMcuIpmiSensorReadingListGet";
    }
    CMcuIpmiSensorReadingListGet(const CMcuIpmiSensorReadingListGet &other);
    CMcuIpmiSensorReadingListGet& operator = (const CMcuIpmiSensorReadingListGet& other);
    virtual ~CMcuIpmiSensorReadingListGet();

    void SetCpuMemUsageGetter(CSysState * pCpuMemUsageGetter)
    {
        m_pCpuMemUsageGetter = pCpuMemUsageGetter;
    }

    void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
    CSerializeObject* Clone()
    {
        return new CMcuIpmiSensorReadingListGet();
    }

    //int convertStrActionToNumber(const char * strAction);

protected:
    CMcuMngrProcess* m_pProcess;
    CSysState * m_pCpuMemUsageGetter;
};

#endif /*__MCU_IPMI_SENSOR_READING_LIST_GET_H__*/

