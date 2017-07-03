// IVRServiceAdd.h: interface for the CIVRServiceAdd class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Add XML IVR Service
//========   ==============   =====================================================================


#if !defined(_IVRSERVICEADD_H__)
#define _IVRSERVICEADD_H__

#include "SerializeObject.h"
#include "IVRService.h"



class CIVRServiceAdd : public CSerializeObject
{
CLASS_TYPE_1(CIVRServiceAdd, CSerializeObject)
public:

	//Constructors
	CIVRServiceAdd();
	virtual const char* NameOf() const { return "CIVRServiceAdd";}
	CIVRServiceAdd(const CIVRServiceAdd &other);
	CIVRServiceAdd& operator = (const CIVRServiceAdd& other);
	virtual ~CIVRServiceAdd();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CIVRServiceAdd;}

	int convertStrActionToNumber(const char * strAction);
		
    
	const CAVmsgService* GetAVmsgService();
	void  SetAVmsgService(CAVmsgService* pAVmsgService);
	
  
protected:

	CAVmsgService* m_pAVmsgService;
	

};

#endif // !defined(_IVRSERVICEADD_H__)

