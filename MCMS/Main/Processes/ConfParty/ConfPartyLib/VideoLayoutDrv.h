// VideoLayoutDrv.h: interface for the CVideoLayoutDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//8/05		Talya			  Class for Change XML Conf Layout
//========   ==============   =====================================================================


#if !defined(_VideoLayoutDrv_H__)
#define _VideoLayoutDrv_H__

#include "VideoLayout.h"
#include "SerializeObject.h"


class CVideoLayoutDrv : public CSerializeObject
{
CLASS_TYPE_1(CVideoLayoutDrv, CSerializeObject)
public:

	//Constructors
	CVideoLayoutDrv();
	virtual const char* NameOf() const { return "CVideoLayoutDrv";}
	CVideoLayoutDrv(const CVideoLayoutDrv &other);
	CVideoLayoutDrv& operator = (const CVideoLayoutDrv& other);
	virtual ~CVideoLayoutDrv();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CVideoLayoutDrv();}

	int convertStrActionToNumber(const char * strAction);
		
    
	CVideoLayout*	  GetVideoLayout();
	DWORD       GetConfID();

  
protected:

	DWORD       m_ConfID;
	CVideoLayout* m_pVideoLayout;
	

};

#endif // !defined(_VideoLayoutDrv_H__)

