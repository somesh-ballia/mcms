// CMcuTimeGet.h: interface for the CMcuStateGet class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MCU_TIME_GET_H_
#define MCU_TIME_GET_H_






#include "SerializeObject.h"
#include "McuMngrProcess.h"



class CMcuTimeGet : public CSerializeObject
{
CLASS_TYPE_1(CMcuTimeGet, CSerializeObject)
public:

	//Constructors
	CMcuTimeGet();
	virtual const char* NameOf() const { return "CMcuTimeGet";}
	CMcuTimeGet(const CMcuTimeGet &other);
	CMcuTimeGet& operator = (const CMcuTimeGet& other);
	virtual ~CMcuTimeGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CMcuTimeGet();}

	//int convertStrActionToNumber(const char * strAction);

	
  
protected:
	CMcuMngrProcess* m_pProcess;
	

};












#endif /*MCUSTATEGET_H_*/
