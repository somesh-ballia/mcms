#ifndef __COLLECTOR_INFO_GET_H_
#define __COLLECTOR_INFO_GET_H_


#include "psosxml.h"
#include "SerializeObject.h"
#include "CollectorProcess.h"



class CCollectorInfoGet : public CSerializeObject
{
CLASS_TYPE_1(CCollectorInfoGet, CSerializeObject)
public:

	//Constructors
	CCollectorInfoGet();
	virtual const char* NameOf() const { return "CCollectorInfoGet";}
	CCollectorInfoGet(const CCollectorInfoGet &other);
	CCollectorInfoGet& operator = (const CCollectorInfoGet& other);
	virtual ~CCollectorInfoGet();


	virtual void  SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int		      DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CCollectorInfoGet;}

protected:
	CCollectorProcess* m_pProcess;
 };


#endif /*__COLLECTOR_INFO_GET_H_*/
