#if !defined(_SiteNameInfoDrv_H__)
#define _SiteNameInfoDrv_H__

#include "SiteNameInfo.h"
#include "SerializeObject.h"


class CSiteNameInfoDrv : public CSerializeObject
{
CLASS_TYPE_1(CSiteNameInfoDrv, CSerializeObject)
public:

	//Constructors
	CSiteNameInfoDrv();
	CSiteNameInfoDrv(const CSiteNameInfoDrv &other);
	CSiteNameInfoDrv& operator = (const CSiteNameInfoDrv& other);
	virtual ~CSiteNameInfoDrv();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CSiteNameInfoDrv();}

	int convertStrActionToNumber(const char * strAction);

	const char * NameOf() const {return "CSiteNameInfoDrv";}

	CSiteNameInfo*	GetSiteNameInfo();
	DWORD			GetConfID();


protected:

	DWORD				  m_ConfID;
	CSiteNameInfo*   		m_pSiteNameInfo;

};

#endif // !defined(_MessageOverlayInfoDrv_H__)

