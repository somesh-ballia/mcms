// VideoLayoutPartyDrv.h: interface for the CVideoLayoutPartyDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//8/05		Talya			  Class for Change XML Party Layout
//========   ==============   =====================================================================


#if !defined(_VideoLayoutPartyDrv_H__)
#define _VideoLayoutPartyDrv_H__

#include "VideoLayout.h"
#include "SerializeObject.h"


class CVideoLayoutPartyDrv : public CSerializeObject
{
CLASS_TYPE_1(CVideoLayoutPartyDrv, CSerializeObject)
public:

	//Constructors
	CVideoLayoutPartyDrv();
	virtual const char* NameOf() const { return "CVideoLayoutPartyDrv";}
	CVideoLayoutPartyDrv(const CVideoLayoutPartyDrv &other);
	CVideoLayoutPartyDrv& operator = (const CVideoLayoutPartyDrv& other);
	virtual ~CVideoLayoutPartyDrv();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CVideoLayoutPartyDrv();}

	int convertStrActionToNumber(const char * strAction);
		
    
	CVideoLayout*	  GetVideoLayout();
	DWORD       	  GetConfID();
	DWORD			  GetPartyID();
	WORD			  GetIsPrivate();

  
protected:

	DWORD       m_ConfID;
	DWORD		m_PartyID;
	WORD		m_IsPrivate;
	CVideoLayout* m_pVideoLayout;
	

};

#endif // !defined(_VideoLayoutPartyDrv_H__)

