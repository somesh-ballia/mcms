/*
 * TcpDumpStatusGet.h
 *
 *  Created on: Mar 29, 2011
 *      Author: racohen
 */

#ifndef TCPDUMPSTATUSGET_H_
#define TCPDUMPSTATUSGET_H_


#include "psosxml.h"
#include "SerializeObject.h"
#include "UtilityProcess.h"



class CTcpDumpStatusGet : public CSerializeObject
{
CLASS_TYPE_1(CTcpDumpStatusGet, CSerializeObject)
public:

	//Constructors
CTcpDumpStatusGet();
	virtual const char* NameOf() const { return "CTcpDumpStatusGet";}
	CTcpDumpStatusGet(const CTcpDumpStatusGet &other);
	CTcpDumpStatusGet& operator = (const CTcpDumpStatusGet& other);
	virtual ~CTcpDumpStatusGet();


	virtual void  SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int		      DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CTcpDumpStatusGet;}

protected:
 	CUtilityProcess* m_pProcess;
 };



#endif /* TCPDUMPSTATUSGET_H_ */
