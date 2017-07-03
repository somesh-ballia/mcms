// CommResAdd.h: interface for the CCommResAdd class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Add XML Reservation
//========   ==============   =================================================================


#if !defined(_CommResAdd_H__)
#define _CommResAdd_H__

#include "SerializeObject.h"
#include "CommResApi.h"
#include "ComResRepeatDetails.h"
#include "MessageOverlayInfo.h"

class CCommResAdd : public CSerializeObject
{
CLASS_TYPE_1(CCommResAdd, CSerializeObject)
public:

	//Constructors
	CCommResAdd();
	virtual const char* NameOf() const { return "CCommResAdd";}
	CCommResAdd(const CCommResAdd &other);
	CCommResAdd& operator = (const CCommResAdd& other);
	virtual ~CCommResAdd();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CCommResAdd;}

	int convertStrActionToNumber(const char * strAction);


	const CCommResApi* GetCommResApi();
	void  SetCommResApi(CCommResApi* pCommResApi);

	const CComResRepeatDetails* GetComResRepeatDetails();
	void  SetComResRepeatDetails(CComResRepeatDetails* pComResRepeatDetails);

protected:

	CCommResApi* m_pCommResApi;
	CComResRepeatDetails* m_pComResRepeatDetails;


};

#endif // !defined(_CommResAdd_H__)

