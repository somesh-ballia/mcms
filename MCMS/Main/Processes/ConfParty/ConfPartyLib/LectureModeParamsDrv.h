// LectureModeParamsDrv.h: interface for the CLectureModeParamsDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//9/05			Talya			  Class for Update Lecture Mode Params
//========   ==============   =====================================================================


#if !defined(_LectureModeParamsDrv_H__)
#define _LectureModeParamsDrv_H__

#include "LectureModeParams.h"
#include "SerializeObject.h"


class CLectureModeParamsDrv : public CSerializeObject
{
CLASS_TYPE_1(CLectureModeParamsDrv, CSerializeObject)
public:

	//Constructors
	CLectureModeParamsDrv();
	virtual const char* NameOf() const { return "CLectureModeParamsDrv";}
	CLectureModeParamsDrv(const CLectureModeParamsDrv &other);
	CLectureModeParamsDrv& operator = (const CLectureModeParamsDrv& other);
	virtual ~CLectureModeParamsDrv();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CLectureModeParamsDrv();}

	int convertStrActionToNumber(const char * strAction);
		
    
	CLectureModeParams*	GetLectureModeParams();
	DWORD				GetConfID();

  
protected:

	DWORD				  m_ConfID;
	CLectureModeParams*   m_pLectureModeParams;
	

};

#endif // !defined(_LectureModeParamsDrv_H__)

