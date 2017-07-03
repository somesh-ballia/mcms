// ClientSendDtmf.h
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//24/7/08		Amir K.			  Class for handle set DTMF from client
//========   ==============   =====================================================================


#if !defined(_ClientSendDtmf_H__)
#define _ClientSendDtmf_H__

#include "SerializeObject.h"

#define MAX_DTMF_FROM_CLIENT_LEN 64
#define MAX_ID_SIZE_FOR_DTMF 100

class CClientSendDtmf : public CSerializeObject
{
CLASS_TYPE_1(CClientSendDtmf, CSerializeObject)
public:

	//Constructors
	CClientSendDtmf();
	CClientSendDtmf(const CClientSendDtmf &other);
	CClientSendDtmf& operator = (const CClientSendDtmf& other);
	virtual ~CClientSendDtmf();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CClientSendDtmf;}

	int convertStrActionToNumber(const char * strAction);
		
	const char * NameOf() const {return "CClientSendDtmf";}
    
	void SetDtmfString(const char *dtmfStr);
	const char *GetDtmfString();
	
	void SetDtmfDirection(int inOut);
	int  GetDtmfDirection();

	DWORD GetConfID();
	void SetConfID(DWORD confId);
	
	DWORD GetPartyID();
	void SetPartyID(DWORD partyId);
	
	const char* GetConfIdString();
	const char* GetPartyMonitorIdString();
	

protected:

	int     m_dtmfDirection;
	char	m_dtmfStr[MAX_DTMF_FROM_CLIENT_LEN];
	DWORD   m_ConfID;
	DWORD   m_PartyID;
	char	m_partyMonitorIdStr[MAX_ID_SIZE_FOR_DTMF];
	char	m_confIdStr[MAX_ID_SIZE_FOR_DTMF];
	

};

#endif // !defined(_ClientSendDtmf_H__)

