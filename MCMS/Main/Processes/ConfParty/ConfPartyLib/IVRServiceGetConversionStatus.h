// IVRServiceGetConversionStatus.h: interface for the CIVRServiceGetConversionStatus class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Huizhao Sun 	  Class for GET_CONVERSION_STATUS XML IVR Service
//========   ==============   =====================================================================


#if !defined(_IVRSERVICEGETCONVERSIONSTATUS_H__)
#define _IVRSERVICEGETCONVERSIONSTATUS_H__

#include "SerializeObject.h"


class CIVRServiceGetConversionStatus : public CSerializeObject
{
CLASS_TYPE_1(IVRServiceGetConversionStatus, CSerializeObject)
public:

	//Constructors
	CIVRServiceGetConversionStatus();
	virtual const char* NameOf() const { return "CIVRServiceGetConversionStatus";}
    CIVRServiceGetConversionStatus(const CIVRServiceGetConversionStatus &other);
	CIVRServiceGetConversionStatus& operator = (const CIVRServiceGetConversionStatus& other);
	virtual ~CIVRServiceGetConversionStatus();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CIVRServiceGetConversionStatus;}

    void SetConnectId(DWORD conId) {m_connectId = conId;}
    DWORD GetConnectId(){return m_connectId;}
    
protected:
    DWORD   m_connectId;
	
};

#endif // !defined(_IVRSERVICEGETCONVERSIONSTATUS_H__)

