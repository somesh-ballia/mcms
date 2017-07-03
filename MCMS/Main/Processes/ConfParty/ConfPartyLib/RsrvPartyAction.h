// RsrvPartyAction.h: interface for the CRsrvPartyAction class.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Del XML Party
//========   ==============   =====================================================================



#if !defined(_RsrvPartyAction_H__)
#define _RsrvPartyAction_H__


#include "NStream.h"
#include "psosxml.h"
#include "SerializeObject.h"
#include "RsrvParty.h"



class CRsrvPartyAction : public CSerializeObject
{
CLASS_TYPE_1(CRsrvPartyAction, CSerializeObject)
public:

	//Constructors
	CRsrvPartyAction();
	virtual const char* NameOf() const { return "CRsrvPartyAction";}
	CRsrvPartyAction(const CRsrvPartyAction &other);
	CRsrvPartyAction& operator = (const CRsrvPartyAction& other);
	virtual ~CRsrvPartyAction();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
/*	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);*/
	CSerializeObject* Clone() {return new CRsrvPartyAction;}

	int convertStrActionToNumber(const char * strAction);
		
    
	
	DWORD GetConfID();
	DWORD GetPartyID();
	void SetConfID(DWORD confId);
	void SetPartyID(DWORD partyId);
	const char*  GetName();
	const char*  GetContactInfo(int contactNum);
	const char* GetAddionalInfo();
	int   GetNumAction();
	int   GetParam1();
	int   GetParam2();
	void  SetNumAction(int nAction);
    
  
protected:

	DWORD       m_ConfID;
	DWORD       m_PartyID;
	char    	m_partyName[H243_NAME_LEN]; 
	char   		m_contact_info_list[MAX_USER_INFO_ITEMS][H243_NAME_LEN];
    char        m_AdditionalInfo [H243_NAME_LEN];
	int         m_NumAction;
	int			m_param1;//optional not required in each request
	int			m_param2;//optional not required in each request
	
};

#endif // !defined(_RsrvPartyAction_H__)

