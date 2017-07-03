#ifndef _GET_CONF_TEMP_SPECIFIC_H_
#define _GET_CONF_TEMP_SPECIFIC_H_

#include "SerializeObject.h"

class CConfTempSpecific : public CSerializeObject
{
	CLASS_TYPE_1(CConfTempSpecific, CSerializeObject)
    public:
       //Constructors
       CConfTempSpecific();
       CConfTempSpecific(const CConfTempSpecific &other);
       CConfTempSpecific& operator = (const CConfTempSpecific & other);
       virtual ~CConfTempSpecific();


       void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
       int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
       CSerializeObject* Clone() {return new CConfTempSpecific();}
    			
       const char * NameOf() const {return "CConfTempSpecific";}
    	    
       DWORD       GetConfTemplateID()const;
    protected:
    		DWORD       m_confTemplateID;
	
};

#endif

