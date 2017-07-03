// ConfPartySpecific.h: interface for the CConfPartySpecific class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Del XML Reservation
//========   ==============   =====================================================================


#if !defined(_ConfPartySpecific_H__)
#define _ConfPartySpecific_H__

#include "RsrvParty.h"
#include "SerializeObject.h"
#include "CommConf.h"


class CConfPartySpecific : public CSerializeObject
{
CLASS_TYPE_1(CConfPartySpecific, CSerializeObject)
public:

	//Constructors
	CConfPartySpecific();
	virtual const char* NameOf() const { return "CConfPartySpecific";}
	CConfPartySpecific(const CConfPartySpecific &other);
	CConfPartySpecific& operator = (const CConfPartySpecific& other);
	virtual ~CConfPartySpecific();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);
	CSerializeObject* Clone() {return new CConfPartySpecific();}

	int convertStrActionToNumber(const char* strAction);
		
    
	ConfMonitorID       GetConfID() { return m_ConfID; }
	PartyMonitorID      GetPartyID() { return m_PartyID; }

  
protected:

	ConfMonitorID        m_ConfID;
	PartyMonitorID       m_PartyID;
	

};


#endif // !defined(_ConfPartySpecific_H__)

