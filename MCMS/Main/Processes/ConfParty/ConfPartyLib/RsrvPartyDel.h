// RsrvPartyDel.h: interface for the CRsrvPartyDel class.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Del XML Party
//========   ==============   =====================================================================



#if !defined(_RsrvPartyDel_H__)
#define _RsrvPartyDel_H__

#include "RsrvParty.h"
#include "SerializeObject.h"



class CRsrvPartyDel : public CSerializeObject
{
CLASS_TYPE_1(CRsrvPartyDel, CSerializeObject)
public:

	//Constructors
	CRsrvPartyDel();
	virtual const char* NameOf() const { return "CRsrvPartyDel";}
	CRsrvPartyDel(const CRsrvPartyDel &other);
	CRsrvPartyDel& operator = (const CRsrvPartyDel& other);
	virtual ~CRsrvPartyDel();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CRsrvPartyDel;}

	int convertStrActionToNumber(const char * strAction);
		
    
	
	DWORD       GetConfID();
	DWORD       GetPartyID();


  
protected:

	DWORD       m_ConfID;
	DWORD       m_PartyID;
	
};

#endif // !defined(_RsrvPartyDel_H__)

