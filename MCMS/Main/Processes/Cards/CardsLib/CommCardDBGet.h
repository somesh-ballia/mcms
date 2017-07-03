// CommCardDBGet.h: interface for the CCommCardDBGet class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Add XML Reservation
//========   ==============   =====================================================================


#if !defined(_CommCardDBGet_H__)
#define _CommCardDBGet_H__


#include "SerializeObject.h"
#include "CardsProcess.h"



class CCommCardDBGet : public CSerializeObject
{
CLASS_TYPE_1(CCommCardDBGet, CSerializeObject)
public:

	//Constructors
	CCommCardDBGet();
	virtual const char* NameOf() const { return "CCommCardDBGet";}
	CCommCardDBGet(const CCommCardDBGet &other);
	CCommCardDBGet& operator = (const CCommCardDBGet& other);
	virtual ~CCommCardDBGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError);
	CSerializeObject* Clone() {return new CCommCardDBGet();}

	//int convertStrActionToNumber(const char * strAction);

	
  
protected:
	CCardsProcess* m_pProcess;
	

};

#endif // !defined(_CommCardDBGet_H__)

