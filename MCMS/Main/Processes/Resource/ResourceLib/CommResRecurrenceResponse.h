#ifndef COMMRESRECURRENCERESPONSE_H_
#define COMMRESRECURRENCERESPONSE_H_

#include "PObject.h"
#include "SerializeObject.h"
#include "RepeatedScheduleCalc.h" 

class CXMLDOMElement;
class CCommResFailedRecurrence;
class CCommResApi;

class CCommResRecurrenceResponse : public CSerializeObject
{
	CLASS_TYPE_1(CCommResRecurrenceResponse,CSerializeObject)
	public:                           
		//Constructors
		CCommResRecurrenceResponse(); 
	    virtual ~CCommResRecurrenceResponse();                 
	    CCommResRecurrenceResponse(const CCommResRecurrenceResponse &other);
	    CCommResRecurrenceResponse& operator = (const CCommResRecurrenceResponse &other);
	    CCommResRecurrenceResponse(CStructTm& StartTime, char* pszName);
	    const char*  NameOf() const;  
	    
	    virtual CSerializeObject* Clone(){return new CCommResRecurrenceResponse();}
		void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
		int	   DeSerializeXml(CXMLDOMElement *pRecurrenceNode, char *pszError, const char* action);	
		
		void AddFailedRecurrence(CCommResFailedRecurrence* pFailedRecurrence);
		void SetCommRes(CCommResApi* pCommResApi);
		
	protected:
		// Attributes
		CCommResFailedRecurrence* m_failedRecurrenceList[MAX_RSRV_IN_LIST_AMOS];
	    DWORD  m_FailedInd; 
	    CCommResApi* m_pCommResApi;
};

#endif /*COMMRESRECURRENCERESPONSE_H_*/
