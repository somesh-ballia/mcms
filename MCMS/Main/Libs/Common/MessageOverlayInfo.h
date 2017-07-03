#ifndef __MESSAGE_OVERLAY_INFO_H_
#define __MESSAGE_OVERLAY_INFO_H_

#include <string>
#include <iomanip>

#include "Segment.h"
#include "NStream.h"
#include "SerializeObject.h"
using namespace std;

class CCommResApi;
class CXMLDOMElement;


////////////////////////////////////////////////////////////////////////////
//                        CMessageOverlayInfo
////////////////////////////////////////////////////////////////////////////
class CMessageOverlayInfo : public CSerializeObject
{
	CLASS_TYPE_1(CMessageOverlayInfo, CSerializeObject)

public:
	CMessageOverlayInfo();
	~CMessageOverlayInfo();

	CMessageOverlayInfo(const CMessageOverlayInfo& other);
	CMessageOverlayInfo& operator =(const CMessageOverlayInfo& other);

	const virtual char*           NameOf() const { return "CMessageOverlayInfo";}

	void                          Serialize(WORD format, CSegment& seg);
	void                          DeSerialize(WORD format, CSegment& seg);
	// Serialize/DeSerialize - binary
	void                          Serialize(WORD format, ostream& ostr) const;
	void                          DeSerialize(WORD format, istream& istr);

	void                          SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int                           DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int                           DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError);
	CSerializeObject*             Clone()                                        { return new CMessageOverlayInfo(); }
	void                          Dump(const char* functionName) const;

	void                          SetMessageNumOfRepetitions(DWORD value)        { m_MessageNumOfRepetitions = value; }
	DWORD                         GetNumOfRepetitions() const                    { return m_MessageNumOfRepetitions; }

	void                          SetMessageText(string value)                   { m_MessageText = value; }
	string                        GetMessageText() const                         { return m_MessageText; }

	void                          SetMessageFontSize(DWORD value)                { m_MessageFontSize = value; }
	DWORD                         GetFontSize() const                            { return m_MessageFontSize; }

	void                          SetMessageDisplaySpeedType(DWORD value)        { m_MessageDisplaySpeedType = value; }
	DWORD                         GetDisplaySpeedType() const                    { return m_MessageDisplaySpeedType; }

	void                          SetMessageDisplayPositionType(DWORD value)     { m_MessageDisplayPositionType = value; }
	DWORD                         GetDisplayPositionType() const                 { return m_MessageDisplayPositionType; }

	void                          SetMessageColorType(DWORD value)               { m_MessageColorType = value; }
	DWORD                         GetColorType() const                           { return m_MessageColorType; }

	void                          SetMessageOnOff(BYTE value)                    { m_MessageOn = value; }
	BYTE                          GetMessageOn() const                           { return m_MessageOn; }

	void                          SetMessageTransparency(DWORD value)            { m_MessageTransparency = value; }
	DWORD                         GetTransparency() const                        { return m_MessageTransparency; }

	void                          SetMessageFontSize_Old(DWORD value)            { m_MessageFontSize_Old = value; }
	DWORD                         GetFontSize_Old() const                        { return m_MessageFontSize_Old; }

	void                          SetMessageDisplayPositionType_Old(DWORD value) { m_MessageDisplayPositionType_Old = value; }
	DWORD                         GetDisplayPositionType_Old() const             { return m_MessageDisplayPositionType_Old; }

protected:
	BYTE                          m_MessageOn;
	string                        m_MessageText;
	DWORD                         m_MessageFontSize;
	DWORD                         m_MessageColorType;
	DWORD                         m_MessageNumOfRepetitions;
	DWORD                         m_MessageDisplaySpeedType;
	DWORD                         m_MessageDisplayPositionType;
	DWORD                         m_MessageTransparency;

	DWORD                         m_MessageFontSize_Old;
	DWORD                         m_MessageDisplayPositionType_Old;
};

#endif // __MESSAGE_OVERLAY_INFO_H_
