#ifndef __SITE_NAME_INFO_H_
#define __SITE_NAME_INFO_H_

#include <string>
#include <iomanip>
#include "Segment.h"
#include "NStream.h"
#include "SerializeObject.h"
#include "ConfPartyApiDefines.h"
using namespace std;

class CCommResApi;
class CXMLDOMElement;

////////////////////////////////////////////////////////////////////////////
//                        CSiteNameInfo
////////////////////////////////////////////////////////////////////////////
class CSiteNameInfo : public CSerializeObject
{
	CLASS_TYPE_1(CSiteNameInfo, CSerializeObject)

public:
	CSiteNameInfo();
	~CSiteNameInfo();

	CSiteNameInfo(const CSiteNameInfo& other);
	CSiteNameInfo& operator  =(const CSiteNameInfo& other);

	const virtual char*      NameOf() const { return "CSiteNameInfo";}

	void                     Serialize(WORD format, CSegment& seg);
	void                     DeSerialize(WORD format, CSegment& seg);
	// Serialize/DeSerialize - binary
	void                     Serialize(WORD format, ostream& ostr) const;
	void                     DeSerialize(WORD format, istream& istr);

	void                     SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int                      DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int                      DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError);
	CSerializeObject*        Clone()                                                { return new CSiteNameInfo(); }
	void                     Dump(const char* functionName) const;

	void                     SetSiteNameFontSize(DWORD value)                       { m_SiteNameFontSize = value; }
	DWORD                    GetFontSize() const                                    { return m_SiteNameFontSize; }

	void                     SetDisplayPositionType(eSiteNameDisplayPosition value) { m_SiteNameDisplayPositionType = value; }
	eSiteNameDisplayPosition GetDisplayPositionType() const                         { return (eSiteNameDisplayPosition)m_SiteNameDisplayPositionType; }

	void                     SetSiteNameColorType(eMessageOverlayColorType value)   { m_SiteNameColorType = value; }
	eMessageOverlayColorType GetColorType() const                                   { return (eMessageOverlayColorType)m_SiteNameColorType; }

	void                     SetDisplayMode(eSiteNameDisplayMode value)             { m_SiteNameDisplayMode = value; }
	eSiteNameDisplayMode     GetDisplayMode() const                                 { return (eSiteNameDisplayMode)m_SiteNameDisplayMode; }

	void                     SetHorizontalPostion(DWORD value)                      { m_SiteNameHorizontalPostion = value; }
	DWORD                    GetHorizontalPostion() const                           { return m_SiteNameHorizontalPostion; }

	void                     SetVerticalPosition(DWORD value)                       { m_SiteNameVerticalPosition = value; }
	DWORD                    GetVerticalPosition() const                            { return m_SiteNameVerticalPosition; }

	void                     SetTranceParence(DWORD value)                          { m_SiteNameTransparance = value; }
	DWORD                    GetTransParence() const                                { return m_SiteNameTransparance; }

	void                     SetTextColor(eTextColorType value)                     { m_SiteNameTextColor = value; }
	eTextColorType           GetTextColorType() const                               { return (eTextColorType)m_SiteNameTextColor; }

	void                     EnableBorder(BOOL value)                               { m_IsBorderEnabled = value; }
	BOOL                     IsEnalbeBorder()                                       { return m_IsBorderEnabled; }

	void                     GetCoordinateXY(long& x, long& y);
	void                     GetCoordinateXY(char& x, char& y);

protected:
	WORD                     m_SiteNameDisplayMode;
	DWORD                    m_SiteNameFontSize;
	DWORD                    m_SiteNameTransparance;
	DWORD                    m_SiteNameHorizontalPostion;
	DWORD                    m_SiteNameVerticalPosition;
	WORD                     m_SiteNameColorType;
	WORD                     m_SiteNameDisplayPositionType;
	WORD                     m_SiteNameTextColor;
	BOOL                     m_IsBorderEnabled;
};

#endif // __MESSAGE_OVERLAY_INFO_H_
