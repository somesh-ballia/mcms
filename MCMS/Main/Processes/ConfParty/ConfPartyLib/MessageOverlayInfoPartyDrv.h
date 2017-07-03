// CMessageOverlayInfoPartyDrv.h: interface for the CMessageOverlayInfoDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date                 Created By        Description
//
//13 April 2011			Marina			 Class for Change Message Overlay Params per specific Party
//===============   ==============   =====================================================================


#if !defined(_MessageOverlayInfoPartyDrv_H__)
#define _MessageOverlayInfoPartyDrv_H__

#include "MessageOverlayInfo.h"
#include "SerializeObject.h"


class CMessageOverlayInfoPartyDrv : public CSerializeObject
{
CLASS_TYPE_1(CMessageOverlayInfoPartyDrv, CSerializeObject)
public:

	//Constructors
	CMessageOverlayInfoPartyDrv();
	CMessageOverlayInfoPartyDrv(const CMessageOverlayInfoPartyDrv &other);
	CMessageOverlayInfoPartyDrv& operator = (const CMessageOverlayInfoPartyDrv& other);
	virtual ~CMessageOverlayInfoPartyDrv();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);

	CSerializeObject* Clone() {return new CMessageOverlayInfoPartyDrv();}

	int convertStrActionToNumber(const char * strAction);

	const char * NameOf() const {return "CMessageOverlayInfoPartyDrv";}

	CMessageOverlayInfo*	GetMessageOverlayInfo();
	DWORD					GetConfID();
	DWORD			  		GetPartyID();
	WORD			  		GetIsPrivate();


protected:

	DWORD					m_ConfID;
	DWORD					m_PartyID;
	WORD					m_IsPrivate;
	CMessageOverlayInfo*   	m_pMessageOverlayInfo;



};

#endif // !defined(_MessageOverlayInfoPartyDrv_H__)

