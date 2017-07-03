#ifndef __MCU_IPMI_FRU_GET_H__
#define __MCU_IPMI_FRU_GET_H__

#include "SerializeObject.h"
#include "McuMngrProcess.h"

class CMcuIpmiFruGet : public CSerializeObject
{
    CLASS_TYPE_1(CMcuIpmiFruGet, CSerializeObject)
public:

    //Constructors
    CMcuIpmiFruGet();
    virtual const char* NameOf() const
    {
        return "CMcuIpmiFruGet";
    }
    CMcuIpmiFruGet(const CMcuIpmiFruGet &other);
    CMcuIpmiFruGet& operator = (const CMcuIpmiFruGet& other);
    virtual ~CMcuIpmiFruGet();


    void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    int DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
   int GetSoltID(){return m_slotID;}
    CSerializeObject* Clone()
    {
        return new CMcuIpmiFruGet();
    }

    //int convertStrActionToNumber(const char * strAction);

protected:
    CMcuMngrProcess* m_pProcess;
       int m_slotID;
};

#endif /*__MCU_IPMI_FRU_GET_H__*/

