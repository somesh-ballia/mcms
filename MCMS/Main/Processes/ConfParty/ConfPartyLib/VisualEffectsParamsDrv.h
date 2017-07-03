// VisualEffectsParamsDrv.h: interface for the CVideoLayoutDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//8/05		Talya			  Class for Update Visual Effects Params
//========   ==============   =====================================================================


#if !defined(_VisualEffectsParamsDrv_H__)
#define _VisualEffectsParamsDrv_H__

#include "VisualEffectsParams.h"
#include "SerializeObject.h"


class CVisualEffectsParamsDrv : public CSerializeObject
{
CLASS_TYPE_1(CVisualEffectsParamsDrv, CSerializeObject)
public:

	//Constructors
	CVisualEffectsParamsDrv();
	virtual const char* NameOf() const { return "CVisualEffectsParamsDrv";}
	CVisualEffectsParamsDrv(const CVisualEffectsParamsDrv &other);
	CVisualEffectsParamsDrv& operator = (const CVisualEffectsParamsDrv& other);
	virtual ~CVisualEffectsParamsDrv();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CVisualEffectsParamsDrv();}

	int convertStrActionToNumber(const char * strAction);
		
    
	CVisualEffectsParams*	GetVisualEffectsParams();
	DWORD					GetConfID();

  
protected:

	DWORD				  m_ConfID;
	CVisualEffectsParams* m_pVisualEffects;
	

};

#endif // !defined(_VideoLayoutDrv_H__)

