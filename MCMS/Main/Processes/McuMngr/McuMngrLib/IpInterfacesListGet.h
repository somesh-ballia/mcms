
#ifndef IPINTERFACESLISTGET_H_
#define IPINTERFACESLISTGET_H_


#include "psosxml.h"
#include "SerializeObject.h"
#include "McuMngrProcess.h"



class CIpInterfacesListGet : public CSerializeObject
{
CLASS_TYPE_1(CIpInterfacesListGet, CSerializeObject)
public:

	//Constructors
	CIpInterfacesListGet();
	virtual const char* NameOf() const { return "CIpInterfacesListGet";}
	CIpInterfacesListGet(const CIpInterfacesListGet &other);
	CIpInterfacesListGet& operator = (const CIpInterfacesListGet& other);
	virtual ~CIpInterfacesListGet();


	virtual void  SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int		      DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CIpInterfacesListGet;}

protected:
 	CMcuMngrProcess* m_pProcess;
 };

#endif /*IPINTERFACESLISTGET_H_*/
