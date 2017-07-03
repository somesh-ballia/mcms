#ifndef PROCESSITEMDATA_H_
#define PROCESSITEMDATA_H_
#include "SerializeObject.h"


class CProcessItemData : public CSerializeObject
{
CLASS_TYPE_1(CProcessItemData,CSerializeObject)
public:

    //Constructors
    
	CProcessItemData();     
	CProcessItemData(const BYTE isEnabled, const DWORD processName);
	CProcessItemData(const CProcessItemData &other);
	CProcessItemData& operator = (const CProcessItemData& other);
	virtual ~CProcessItemData();
	virtual CSerializeObject* Clone() {return new CProcessItemData();}  
	virtual void   SerializeXml(CXMLDOMElement*& pParentNode) const;
	//void 	SerializeXml(CXMLDOMElement *pFatherNode, DWORD processName,BYTE bEnabled);
	virtual int    DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action);

	const char*  NameOf() const  { return "CProcessItemData"; }       

	void			SetEnabled(BYTE bEnabled);
	void			SetName(DWORD name);
	BYTE			IsEnabled() const;
	DWORD			GetName() const;
	
protected:
	
	BYTE			m_bEnabled;
	DWORD			m_iProcessame;
  	
};

/////////////////////////////////////////////////////////////////////////
#endif /*MODULECONTENT_H_*/



