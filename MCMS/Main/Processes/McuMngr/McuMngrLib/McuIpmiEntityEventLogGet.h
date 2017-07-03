#ifndef __MCU_IPMI_ENTITY_EVENT_LOG_GET_H__
#define __MCU_IPMI_ENTITY_EVENT_LOG_GET_H__

#include "SerializeObject.h"
#include "McuMngrProcess.h"

class CMcuIpmiEntityEventLogGet : public CSerializeObject
{
    CLASS_TYPE_1(CMcuIpmiEntityEventLogGet, CSerializeObject)
public:

    //Constructors
    CMcuIpmiEntityEventLogGet();
    virtual const char* NameOf() const
    {
        return "CMcuIpmiEntityEventLogGet";
    }
    CMcuIpmiEntityEventLogGet(const CMcuIpmiEntityEventLogGet &other);
    CMcuIpmiEntityEventLogGet& operator = (const CMcuIpmiEntityEventLogGet& other);
    virtual ~CMcuIpmiEntityEventLogGet();


    void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
    CSerializeObject* Clone()
    {
        return new CMcuIpmiEntityEventLogGet();
    }

    //int convertStrActionToNumber(const char * strAction);

protected:
    CMcuMngrProcess* m_pProcess;
};

#endif /*__MCU_IPMI_ENTITY_EVENT_LOG_GET_H__*/

