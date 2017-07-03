#ifndef __AUDIT_EVENT_LIST_H__
#define __AUDIT_EVENT_LIST_H__

#include "SerializeObject.h"



class CAuditEventListGet : public CSerializeObject
{
public:
    CAuditEventListGet();
    virtual ~CAuditEventListGet();
    
    virtual const char*  NameOf() const{return "CAuditEventListGet";}
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
    

private:
    CSerializeObject* Clone(){return new CAuditEventListGet(*this);}

    DWORD m_Id;
};





#endif  // __AUDIT_EVENT_LIST_H__
