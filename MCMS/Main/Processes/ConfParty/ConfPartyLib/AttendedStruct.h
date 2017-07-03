#ifndef __ATTENDED_STRUCT_H__
#define __ATTENDED_STRUCT_H__

class CXMLDOMElement;

////////////////////////////////////////////////////////////////////////////
//                        CAttendedStruct
////////////////////////////////////////////////////////////////////////////
class CAttendedStruct : public CPObject
{
	CLASS_TYPE_1(CAttendedStruct, CPObject)

public:
	                 CAttendedStruct();
	                 CAttendedStruct(const CAttendedStruct& other);
	virtual         ~CAttendedStruct();

	CAttendedStruct& operator=(const CAttendedStruct& other);

	const char*      NameOf() const;
	void             SerializeXml(CXMLDOMElement* pFatherNode);
	int              DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
	void             SetOrdinaryParty(const BYTE ordinary_party);
	BYTE             GetOrdinaryParty() const;
	// Attributes
	BYTE             m_ordinary_party;
};

#endif  // __ATTENDED_STRUCT_H__
