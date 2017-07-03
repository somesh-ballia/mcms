#ifndef CIPSERVICEFULLLISTGET_H_
#define CIPSERVICEFULLLISTGET_H_

#include "SerializeObject.h"

class CIPServiceFullListGet : public CSerializeObject
{
public:
	CIPServiceFullListGet();
	CIPServiceFullListGet(const CIPServiceFullListGet &);
	virtual ~CIPServiceFullListGet();

	const char * NameOf(void) const {return "CIPServiceFullListGet";}
	
	virtual void SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int		DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CIPServiceFullListGet;}
	
	
private:
	// disabled
	CIPServiceFullListGet& operator=(const CIPServiceFullListGet&);
};

#endif /*CIPSERVICEFULLLISTGET_H_*/
