// CommConfSpecific.h: interface for the CCommConfSpecific class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Del XML Reservation
//========   ==============   =====================================================================


#if !defined(_CommConfSpecific_H__)
#define _CommConfSpecific_H__

#include "RsrvParty.h"
#include "SerializeObject.h"
#include "CommConf.h"


class CCommConfSpecific : public CSerializeObject
{
CLASS_TYPE_1(CCommConfSpecific, CSerializeObject)
public:

	//Constructors
	CCommConfSpecific();
	virtual const char* NameOf() const { return "CCommConfSpecific";}
	CCommConfSpecific(const CCommConfSpecific &other);
	CCommConfSpecific& operator = (const CCommConfSpecific& other);
	virtual ~CCommConfSpecific();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);
	CSerializeObject* Clone() {return new CCommConfSpecific();}

	int convertStrActionToNumber(const char* strAction);
		
    
	ConfMonitorID       GetConfID() { return m_ConfID; }

  
protected:

	ConfMonitorID       m_ConfID;

};

#endif // !defined(_CommConfSpecific_H__)

