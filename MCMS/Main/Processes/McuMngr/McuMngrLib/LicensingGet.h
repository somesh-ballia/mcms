// LicensingGet.h: interface for the CLicensingGet class.
//
//////////////////////////////////////////////////////////////////////

#ifndef LICENSING_GET_H_
#define LICENSING_GET_H_






#include "SerializeObject.h"
#include "McuMngrProcess.h"




class CLicensingGet : public CSerializeObject
{
CLASS_TYPE_1(CLicensingGet, CSerializeObject)
public:

	//Constructors
	CLicensingGet();
	virtual const char* NameOf() const { return "CLicensingGet";}
	CLicensingGet(const CLicensingGet &other);
	CLicensingGet& operator = (const CLicensingGet& other);
	virtual ~CLicensingGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CLicensingGet();}


	//int convertStrActionToNumber(const char * strAction);
	

  
protected:
	CMcuMngrProcess* m_pProcess;
	

};












#endif /*MCUSTATEGET_H_*/
