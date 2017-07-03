#ifndef __MCU_IPMI_ENTITY_FAN_LEVEL_GET_H__
#define __MCU_IPMI_ENTITY_FAN_LEVEL_GET_H__

#include "SerializeObject.h"
#include "McuMngrProcess.h"

class CMcuIpmiEntityFanLevelGet : public CSerializeObject
{
    CLASS_TYPE_1(CMcuIpmiEntityFanLevelGet, CSerializeObject)
public:

    //Constructors
    CMcuIpmiEntityFanLevelGet();
    virtual const char* NameOf() const
    {
        return "CMcuIpmiEntityFanLevelGet";
    }
    CMcuIpmiEntityFanLevelGet(const CMcuIpmiEntityFanLevelGet &other);
    CMcuIpmiEntityFanLevelGet& operator = (const CMcuIpmiEntityFanLevelGet& other);
    virtual ~CMcuIpmiEntityFanLevelGet();


    void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
    CSerializeObject* Clone()
    {
        return new CMcuIpmiEntityFanLevelGet();
    }

    //int convertStrActionToNumber(const char * strAction);

protected:
    CMcuMngrProcess* m_pProcess;
};

#endif /*__MCU_IPMI_ENTITY_FAN_LEVEL_GET_H__*/

