#ifndef __MCU_IPMI_SENSOR_LIST_GET_H__
#define __MCU_IPMI_SENSOR_LIST_GET_H__

#include "SerializeObject.h"
#include "McuMngrProcess.h"

class CSysState;

class CMcuIpmiSensorListGet : public CSerializeObject
{
    CLASS_TYPE_1(CMcuIpmiSensorListGet, CSerializeObject)
public:

    //Constructors
    CMcuIpmiSensorListGet();
    virtual const char* NameOf() const
    {
        return "CMcuIpmiSensorListGet";
    }
    CMcuIpmiSensorListGet(const CMcuIpmiSensorListGet &other);
    CMcuIpmiSensorListGet& operator = (const CMcuIpmiSensorListGet& other);
    virtual ~CMcuIpmiSensorListGet();

    void SetCpuMemUsageGetter(CSysState * pCpuMemUsageGetter)
    {
        m_pCpuMemUsageGetter = pCpuMemUsageGetter;
    }
    
    void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
    CSerializeObject* Clone()
    {
        return new CMcuIpmiSensorListGet();
    }

    //int convertStrActionToNumber(const char * strAction);

protected:
    CMcuMngrProcess* m_pProcess;
    CSysState * m_pCpuMemUsageGetter;
};

#endif /*__MCU_IPMI_SENSOR_LIST_GET_H__*/

