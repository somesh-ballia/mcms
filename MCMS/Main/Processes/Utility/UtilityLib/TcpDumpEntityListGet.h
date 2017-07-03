
#ifndef TCPDUMPENTITYLISTGET_H_
#define TCPDUMPENTITYLISTGET_H_


#include "psosxml.h"
#include "SerializeObject.h"
#include "UtilityProcess.h"



class CTcpDumpEntityListGet : public CSerializeObject
{
CLASS_TYPE_1(CTcpDumpEntityListGet, CSerializeObject)
public:

	//Constructors
CTcpDumpEntityListGet();
	virtual const char* NameOf() const { return "CTcpDumpEntityListGet";}
	CTcpDumpEntityListGet(const CTcpDumpEntityListGet &other);
	CTcpDumpEntityListGet& operator = (const CTcpDumpEntityListGet& other);
	virtual ~CTcpDumpEntityListGet();


	virtual void  SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int		      DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CTcpDumpEntityListGet;}

protected:
 	CUtilityProcess* m_pProcess;
 };


#endif /*TCPDUMPENTITYLISTGET_H_*/
