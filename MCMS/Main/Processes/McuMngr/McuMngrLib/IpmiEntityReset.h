#ifndef __IPMI_ENTITY_RESET_H__
#define __IPMI_ENTITY_RESET_H__

#include "SerializeObject.h"
#include "IpmiEntitySlotIDs.h"

enum
{
      RESET_TYPE_NORMAL = 0
    , RESET_TYPE_DIAGNOSTIC = 1
};

class CIpmiEntityReset : public CSerializeObject
{
public:
    CIpmiEntityReset();
    virtual ~CIpmiEntityReset();
    virtual CSerializeObject* Clone(){return new CIpmiEntityReset;}
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual const char* NameOf() const { return "CIpmiEntityReset";}
    virtual int DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    int GetSlotId() const { return m_slotID; }
    int GetResetType() const { return m_resetType; }
    
private:
    // disabled
    CIpmiEntityReset(const CIpmiEntityReset&);
    CIpmiEntityReset& operator=(const CIpmiEntityReset&);

    int m_slotID;
    int m_resetType;
};

#endif /*__IPMI_ENTITY_RESET_H__*/

