#ifndef ACTIVEALARMSLISTGET_H_
#define ACTIVEALARMSLISTGET_H_


#include "psosxml.h"
#include "SerializeObject.h"
#include "McuMngrProcess.h"



class CActiveAlarmsListGet : public CSerializeObject
{
CLASS_TYPE_1(CActiveAlarmsListGet, CSerializeObject)
public:

	//Constructors
	CActiveAlarmsListGet();
	virtual const char* NameOf() const { return "CActiveAlarmsListGet";}
	CActiveAlarmsListGet(const CActiveAlarmsListGet &other);
	CActiveAlarmsListGet& operator = (const CActiveAlarmsListGet& other);
	virtual ~CActiveAlarmsListGet();


	virtual void  SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int		      DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CActiveAlarmsListGet;}

protected:
 	CMcuMngrProcess* m_pProcess;
 	DWORD m_id;
 };


#endif /*ACTIVEALARMSLISTGET_H_*/
