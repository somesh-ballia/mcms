#ifndef __MCU_IPMI_ENTITY_LIST_GET_H__
#define __MCU_IPMI_ENTITY_LIST_GET_H__

#include "SerializeObject.h"
#include "McuMngrProcess.h"

class CMcuIpmiEntityListGet : public CSerializeObject
{
    CLASS_TYPE_1(CMcuIpmiEntityListGet, CSerializeObject)
public:

    //Constructors
    CMcuIpmiEntityListGet();
    virtual const char* NameOf() const
    {
        return "CMcuIpmiEntityListGet";
    }
    CMcuIpmiEntityListGet(const CMcuIpmiEntityListGet &other);
    CMcuIpmiEntityListGet& operator = (const CMcuIpmiEntityListGet& other);
    virtual ~CMcuIpmiEntityListGet();


    void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
    CSerializeObject* Clone()
    {
        return new CMcuIpmiEntityListGet();
    }

    //int convertStrActionToNumber(const char * strAction);

protected:
    CMcuMngrProcess* m_pProcess;
};

#endif /*__MCU_IPMI_ENTITY_LIST_GET_H__*/

