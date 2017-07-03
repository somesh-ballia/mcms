#ifndef COMMRESFAILEDRECCURENCE_H_
#define COMMRESFAILEDRECCURENCE_H_

#include "PObject.h"
#include "StructTm.h"
#include "SerializeObject.h"

class CXMLDOMElement;

class CCommResFailedRecurrence : public CSerializeObject
{
CLASS_TYPE_1(CCommResFailedRecurrence,CSerializeObject)
public:                           
	//Constructors
    CCommResFailedRecurrence(); 
    virtual ~CCommResFailedRecurrence() {};                 
    CCommResFailedRecurrence(const CCommResFailedRecurrence &other);
    CCommResFailedRecurrence& operator = (const CCommResFailedRecurrence &other);
    CCommResFailedRecurrence(CStructTm& StartTime, char* pszName, STATUS status);
    const char*  NameOf() const;  
    
    virtual CSerializeObject* Clone(){return new CCommResFailedRecurrence();}
	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int	   DeSerializeXml(CXMLDOMElement *pRecurrenceNode, char *pszError, const char* action);	
     
    void  SetStartTime(const CStructTm &other);
    const CStructTm*  GetStartTime() const;
	
    void  SetName(const char*  name);                 
    const char*  GetName () const;     
    
    void  SetStatus(DWORD status);
    DWORD GetStatus() const;
	
protected:
	// Attributes
	CStructTm m_startTime;
    char      m_H243confName[H243_NAME_LEN];      
    DWORD     m_status;
};

#endif /*COMMRESFAILEDRECCURENCE_H_*/
