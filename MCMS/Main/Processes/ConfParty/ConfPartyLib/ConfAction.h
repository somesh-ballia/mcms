// RsrvPartyAction.h: interface for the CRsrvPartyAction class.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class conf actions with one int param - such as auto layout = on/off
//========   ==============   =====================================================================



#if !defined(_ConfAction_H__)
#define _ConfAction_H__


#include "NStream.h"
#include "psosxml.h"
#include "SerializeObject.h"
#include "ConfContactInfo.h"
#include"ConfPartySharedDefines.h"
//#include"CDRDefines.h"

class CConfAction : public CSerializeObject
{
CLASS_TYPE_1(CRsrvPartyAction, CSerializeObject)
public:

	//Constructors
	CConfAction();
	virtual const char* NameOf() const { return "CConfAction";}
	CConfAction(const CConfAction &other);
	CConfAction& operator = (const CConfAction& other);
	virtual ~CConfAction();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
//	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CConfAction;}

	int convertStrActionToNumber(const char * strAction);
		
    
	
	DWORD GetConfID();
	int   GetNumAction();
	const char*  GetBillingData();
//	const char*  GetContactInfo1();
//	const char*  GetContactInfo2();
//	const char*  GetContactInfo3();
	const char*  GetContactInfo(int contactNum);
	void  SetNumAction(int nAction);
	const char*	GetEntryPassword();
	const char * GetChairPersonPassword()const{return m_chairPersonPassword;}
	DWORD GetRecordingCommand();
	int   GetNumAction1();
	int   GetNumAction2();
	  
protected:

	DWORD       m_ConfID;
	int         m_NumAction;
	int         m_NumAction1;
	int         m_NumAction2;
	char    	m_billing_data[H243_NAME_LEN];
	char   		m_contact_info_list[MAX_CONF_INFO_ITEMS][H243_NAME_LEN];
	char   		m_entry_password[CONFERENCE_ENTRY_PASSWORD_LEN];
	char        m_chairPersonPassword[H243_NAME_LEN];
	DWORD		m_recordingCommand;
};

#endif // !defined(_ConfAction_H__)

