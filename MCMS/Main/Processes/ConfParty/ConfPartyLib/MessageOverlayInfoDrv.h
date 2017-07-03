// MessageOverlayInfoDrv.h: interface for the CMessageOverlayInfoDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//9/05			Talya			  Class for Update Lecture Mode Params
//========   ==============   =====================================================================


#if !defined(_MessageOverlayInfoDrv_H__)
#define _MessageOverlayInfoDrv_H__

#include "MessageOverlayInfo.h"
#include "SerializeObject.h"


class CMessageOverlayInfoDrv : public CSerializeObject
{
CLASS_TYPE_1(CMessageOverlayInfoDrv, CSerializeObject)
public:

	//Constructors
	CMessageOverlayInfoDrv();
	CMessageOverlayInfoDrv(const CMessageOverlayInfoDrv &other);
	CMessageOverlayInfoDrv& operator = (const CMessageOverlayInfoDrv& other);
	virtual ~CMessageOverlayInfoDrv();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CMessageOverlayInfoDrv();}

	int convertStrActionToNumber(const char * strAction);

	const char * NameOf() const {return "CMessageOverlayInfoDrv";}

	CMessageOverlayInfo*	GetMessageOverlayInfo();
	DWORD					GetConfID();


protected:

	DWORD				  m_ConfID;
	CMessageOverlayInfo*   m_pMessageOverlayInfo;


};

#endif // !defined(_MessageOverlayInfoDrv_H__)

