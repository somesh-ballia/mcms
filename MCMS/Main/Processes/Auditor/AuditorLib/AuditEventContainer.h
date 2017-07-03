#ifndef __AUDIT_EVENT_CONTAINER_H__
#define __AUDIT_EVENT_CONTAINER_H__

#include <list>
using namespace std;

#include "SerializeObject.h"



class CAuditHdrWrapper;
class CXMLDOMElement;


typedef list<CAuditHdrWrapper*> CAuditEventQueue;




class CAuditEventContainer : public CSerializeObject
{
public:
    CAuditEventContainer(DWORD maxNumEvents);
    virtual ~CAuditEventContainer();
    
    virtual const char*  NameOf() const {return "CAuditEventContainer";}
  
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
    void SerializeXml(CXMLDOMElement*& pFatherNode, DWORD objToken) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

    void AddEvent(const CAuditHdrWrapper & event);
    CSerializeObject* Clone(){return new CAuditEventContainer(*this);}

    DWORD GetBigestSequenceNumber()const;
    
private:
    // disabled
    CAuditEventContainer& operator=(const CAuditEventContainer&);
    
    


    void Enqueue(CAuditHdrWrapper * pEvent);
    CAuditHdrWrapper* Dequeue();

    
    CAuditEventQueue m_AuditEventQueue;
    const DWORD m_MaxEventNum;
};





#endif // __AUDIT_EVENT_CONTAINER_H__
