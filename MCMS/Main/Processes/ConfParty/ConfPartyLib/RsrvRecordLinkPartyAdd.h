// RsrvPartyAdd.h: interface for the CRsrvRecordLinkPartyAdd class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Del XML Party
//========   ==============   =====================================================================


#if !defined(_RsrvRecordLinkPartyAdd_H__)
#define _RsrvRecordLinkPartyAdd_H__

#include "RsrvParty.h"
#include "SerializeObject.h"



class CRsrvRecordLinkPartyAdd : public CSerializeObject
{
CLASS_TYPE_1(CRsrvRecordLinkPartyAdd, CSerializeObject)
public:

	//Constructors
	CRsrvRecordLinkPartyAdd();
	CRsrvRecordLinkPartyAdd(const CRsrvRecordLinkPartyAdd &other);
	CRsrvRecordLinkPartyAdd(CRsrvParty* pRsrvParty);
	CRsrvRecordLinkPartyAdd& operator = (const CRsrvRecordLinkPartyAdd& other);
	virtual ~CRsrvRecordLinkPartyAdd();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CRsrvRecordLinkPartyAdd;}

	int convertStrActionToNumber(const char * strAction);
		
	const char * NameOf() const {return "CRsrvRecordLinkPartyAdd";}
    
	CRsrvParty* GetRsrvParty();
	void  SetRsrvParty(CRsrvParty* pRsrvParty);
	void  SetConfID(DWORD confId);
	DWORD GetConfID();
  
protected:

	DWORD       m_ConfID;
	CRsrvParty* m_pRsrvParty;
	

};

#endif // !defined(_RsrvPartyAdd_H__)

