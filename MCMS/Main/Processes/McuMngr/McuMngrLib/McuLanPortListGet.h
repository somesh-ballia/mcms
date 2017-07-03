#ifndef __MCU_LAN_PORT_LIST_GET_H__
#define __MCU_LAN_PORT_LIST_GET_H__

#include "SerializeObject.h"
#include "McuMngrProcess.h"

class CMcuLanPortListGet : public CSerializeObject
{
    CLASS_TYPE_1(CMcuLanPortListGet, CSerializeObject)
public:

    //Constructors
    CMcuLanPortListGet();
    virtual const char* NameOf() const
    {
        return "CMcuLanPortListGet";
    }
    CMcuLanPortListGet(const CMcuLanPortListGet &other);
    CMcuLanPortListGet& operator = (const CMcuLanPortListGet& other);
    virtual ~CMcuLanPortListGet();


    void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
    CSerializeObject* Clone()
    {
        return new CMcuLanPortListGet();
    }

    //int convertStrActionToNumber(const char * strAction);

protected:
    CMcuMngrProcess* m_pProcess;
};

#endif /*__MCU_LAN_PORT_LIST_GET_H__*/

