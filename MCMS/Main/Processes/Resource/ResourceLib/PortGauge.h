#ifndef PORTGAUGE_H_
#define PORTGAUGE_H_

#include "PObject.h"
#include "SerializeObject.h"

class CPortGauge: public CSerializeObject
{
CLASS_TYPE_1(CPortGauge,CSerializeObject)

public:
	CPortGauge();
	virtual ~CPortGauge();
	const char * NameOf() const { return "CPortGauge"; }

	//CSerializeObject overrides
	virtual CSerializeObject* Clone() { return new CPortGauge; }
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int DeSerializeXml(CXMLDOMElement *pNode, char *pszError, const char* action);

	void  SetPortGauge(DWORD portGauge) { m_portGauge = portGauge;}
	DWORD GetPortGauge() { return m_portGauge; }

private:
	DWORD m_portGauge;
};

#endif /* PORTGAUGE_H_ */
