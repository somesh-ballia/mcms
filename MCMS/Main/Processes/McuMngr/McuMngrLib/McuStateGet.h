// CMcuStateGet.h: interface for the CMcuStateGet class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MCUSTATEGET_H_
#define MCUSTATEGET_H_






#include "SerializeObject.h"
#include "McuMngrProcess.h"



class CMcuStateGet : public CSerializeObject
{
CLASS_TYPE_1(CMcuStateGet, CSerializeObject)
public:

	//Constructors
	CMcuStateGet();
	virtual const char* NameOf() const { return "CMcuStateGet";}
	CMcuStateGet(const CMcuStateGet &other);
	CMcuStateGet& operator = (const CMcuStateGet& other);
	virtual ~CMcuStateGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CMcuStateGet();}

	//int convertStrActionToNumber(const char * strAction);

	
  
protected:
	CMcuMngrProcess* m_pProcess;
	

};












#endif /*MCUSTATEGET_H_*/
