/*
 * McuStateGetEx.h
 *
 *  Created on: Nov 24, 2009
 *      Author: yael
 */

#ifndef MCUSTATEGETEX_H_
#define MCUSTATEGETEX_H_


#include "SerializeObject.h"


class CMcuStateGetEx : public CSerializeObject
{
CLASS_TYPE_1(CMcuStateGetEx, CSerializeObject)
public:

	//Constructors
	CMcuStateGetEx();
	CMcuStateGetEx(const CMcuStateGetEx &other);
	CMcuStateGetEx& operator = (const CMcuStateGetEx& other);
	virtual ~CMcuStateGetEx();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CMcuStateGetEx();}

	const char* NameOf() const {return "CMcuStateGetEx";}

	char* GetClientIp();

protected:
	char  m_client_ip[IP_ADDRESS_STR_LEN];
};

class CMcuStateGetExReply: public CSerializeObject
{
CLASS_TYPE_1(CMcuStateGetExReply, CSerializeObject)
public:

	//Constructors
	CMcuStateGetExReply();
	CMcuStateGetExReply(const CMcuStateGetExReply &other);
	CMcuStateGetExReply& operator = (const CMcuStateGetExReply& other);
	virtual ~CMcuStateGetExReply();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CMcuStateGetExReply();}
	const char* NameOf() const {return "CMcuStateGetExReply";}
	void SetFailoverTrigger(BOOL bFailoverTrigger) {m_bFailoverTrigger = bFailoverTrigger;}

protected:

	BOOL m_bFailoverTrigger;
};


#endif /* MCUSTATEGETEX_H_ */
