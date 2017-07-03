#ifndef __MCU_IPMI_ENTITY_FAN_INFO_GET_H__
#define __MCU_IPMI_ENTITY_FAN_INFO_GET_H__

#include "SerializeObject.h"
#include "McuMngrProcess.h"

class CMcuIpmiEntityFanInfoGet : public CSerializeObject
{
    CLASS_TYPE_1(CMcuIpmiEntityFanInfoGet, CSerializeObject)
public:

    //Constructors
    CMcuIpmiEntityFanInfoGet();
    virtual const char* NameOf() const
    {
        return "CMcuIpmiEntityFanInfoGet";
    }
    CMcuIpmiEntityFanInfoGet(const CMcuIpmiEntityFanInfoGet &other);
    CMcuIpmiEntityFanInfoGet& operator = (const CMcuIpmiEntityFanInfoGet& other);
    virtual ~CMcuIpmiEntityFanInfoGet();


    void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
    CSerializeObject* Clone()
    {
        return new CMcuIpmiEntityFanInfoGet();
    }

    //int convertStrActionToNumber(const char * strAction);

protected:
    CMcuMngrProcess* m_pProcess;
};

#endif /*__MCU_IPMI_ENTITY_FAN_INFO_GET_H__*/

