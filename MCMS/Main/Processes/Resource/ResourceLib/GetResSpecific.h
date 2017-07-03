#ifndef _GET_RES_SPECIFIC_H_
#define _GET_RES_SPECIFIC_H_

#include "SerializeObject.h"

class CGetResSpecific : public CSerializeObject
{
CLASS_TYPE_1(CGetResSpecific, CSerializeObject)
public:

	//Constructors
	CGetResSpecific();
	CGetResSpecific(const CGetResSpecific &other);
	CGetResSpecific& operator = (const CGetResSpecific & other);
	virtual ~CGetResSpecific();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	CSerializeObject* Clone() {return new CGetResSpecific();}
		
	const char * NameOf() const {return "CGetResSpecific";}
    
	DWORD       GetResID()const;
protected:
	DWORD       m_ResID;
};

class CShiftTime : public CSerializeObject
{
CLASS_TYPE_1(CShiftTime, CSerializeObject)
public:
	CShiftTime();

	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);

	CSerializeObject* Clone() {return new CShiftTime();}

	WORD GetHour()const { return m_hour; }
	WORD GetMin()const { return m_min; }
	int  GetSign()const { return m_sign; }

private:

	WORD m_hour;
	WORD m_min;
	int  m_sign;
};
#endif 
