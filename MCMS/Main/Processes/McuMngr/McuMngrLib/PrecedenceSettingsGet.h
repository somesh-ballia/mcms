
#ifndef _PRECEDENCE_SETTINGS_GET
#define _PRECEDENCE_SETTINGS_GET






#include "SerializeObject.h"
#include "McuMngrProcess.h"



class CPrecedenceSettingsGet : public CSerializeObject
{
CLASS_TYPE_1(CPrecedenceSettingsGet, CSerializeObject)
public:

	//Constructors
	CPrecedenceSettingsGet();
	virtual const char* NameOf() const { return "CPrecedenceSettingsGet";}
	CPrecedenceSettingsGet(CPrecedenceSettingsGet &other);
	CPrecedenceSettingsGet& operator = (const CPrecedenceSettingsGet& other);
	virtual ~CPrecedenceSettingsGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CPrecedenceSettingsGet();}


protected:
	CMcuMngrProcess* m_pProcess;


};












#endif /*MCUSTATEGET_H_*/

