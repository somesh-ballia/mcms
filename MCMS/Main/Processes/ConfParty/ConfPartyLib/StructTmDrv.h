#ifndef SETENDTIME_
#define SETENDTIME_


#include "StructTm.h"
#include "SerializeObject.h"


class CSetEndTime : public CSerializeObject
{
CLASS_TYPE_1(CSetEndTime, CSerializeObject)
public:

	//Constructors
	CSetEndTime();
	CSetEndTime(const CSetEndTime &other);
	CSetEndTime& operator = (const CSetEndTime& other);
	virtual ~CSetEndTime();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CSetEndTime();}

	int convertStrActionToNumber(const char * strAction);
		
	const char * NameOf() const {return "CSetEndTime";}
    
	CStructTm*		GetTime();
	DWORD		GetConfID();

  
protected:

	DWORD       m_ConfID;
	CStructTm*	m_pTime;
	

};


#endif /*SETENDTIME_*/
