// RsrvPartyAdd.h: interface for the CRsrvPartyAdd class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Del XML Party
//========   ==============   =====================================================================


#if !defined(_RsrvPartyAdd_H__)
#define _RsrvPartyAdd_H__

#include "RsrvParty.h"
#include "SerializeObject.h"



class CRsrvPartyAdd : public CSerializeObject
{
CLASS_TYPE_1(CRsrvPartyAdd, CSerializeObject)
public:

	//Constructors
	CRsrvPartyAdd();
	virtual const char* NameOf() const { return "CRsrvPartyAdd";}
	CRsrvPartyAdd(const CRsrvPartyAdd &other);
	CRsrvPartyAdd& operator = (const CRsrvPartyAdd& other);
	virtual ~CRsrvPartyAdd();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CRsrvPartyAdd;}

	int convertStrActionToNumber(const char * strAction);
		
    
	CRsrvParty* GetRsrvParty();
	void  SetConfID(DWORD confId);
	DWORD GetConfID();
  
protected:

	DWORD       m_ConfID;
	CRsrvParty* m_pRsrvParty;
	

};

#endif // !defined(_RsrvPartyAdd_H__)

