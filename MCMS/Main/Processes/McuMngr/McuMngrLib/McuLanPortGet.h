#ifndef __MCU_LAN_PORT_GET_H__
#define __MCU_LAN_PORT_GET_H__

#include "SerializeObject.h"
#include "McuMngrProcess.h"

class CMcuLanPortGet : public CSerializeObject
{
    CLASS_TYPE_1(CMcuLanPortGet, CSerializeObject)
public:

    //Constructors
    CMcuLanPortGet();
    virtual const char* NameOf() const
    {
        return "CMcuLanPortGet";
    }
    CMcuLanPortGet(const CMcuLanPortGet &other);
    CMcuLanPortGet& operator = (const CMcuLanPortGet& other);
    virtual ~CMcuLanPortGet();


    void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
    CSerializeObject* Clone()
    {
        return new CMcuLanPortGet();
    }

    //int convertStrActionToNumber(const char * strAction);

protected:
    CMcuMngrProcess* m_pProcess;
    int m_slotID;
};

#endif /*__MCU_LAN_PORT_GET_H__*/

