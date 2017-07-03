#include <ostream>
#include <istream>
#include "SiteNameInfo.h"
#include "Segment.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "ConfPartyApiDefines.h"
#include "VideoStructs.h"

////////////////////////////////////////////////////////////////////////////
//                        CSiteNameInfo
////////////////////////////////////////////////////////////////////////////
CSiteNameInfo::CSiteNameInfo()
{
	m_SiteNameDisplayMode         = eSiteNameOff;
	m_SiteNameFontSize            = 12;
	m_SiteNameColorType           = eWhite_font_on_red_background;
	m_SiteNameDisplayPositionType = eLeft_top_position;
	m_SiteNameTransparance        = 50;
	m_SiteNameHorizontalPostion   = 0;
	m_SiteNameVerticalPosition    = 0;
	m_SiteNameTextColor           = eWhite_font;
	m_IsBorderEnabled             = FALSE;
}

//--------------------------------------------------------------------------
CSiteNameInfo::CSiteNameInfo(const CSiteNameInfo& other)
	: CSerializeObject(other)
{
	m_SiteNameDisplayMode         = other.m_SiteNameDisplayMode;
	m_SiteNameFontSize            = other.m_SiteNameFontSize;
	m_SiteNameColorType           = other.m_SiteNameColorType;
	m_SiteNameDisplayPositionType = other.m_SiteNameDisplayPositionType;
	m_SiteNameTransparance        = other.m_SiteNameTransparance;
	m_SiteNameHorizontalPostion   = other.m_SiteNameHorizontalPostion;
	m_SiteNameVerticalPosition    = other.m_SiteNameVerticalPosition;
	m_SiteNameTextColor           = other.m_SiteNameTextColor;
	m_IsBorderEnabled             = other.m_IsBorderEnabled;
}

//--------------------------------------------------------------------------
CSiteNameInfo& CSiteNameInfo::operator =(const CSiteNameInfo& other)
{
	if (&other == this) return *this;

	m_SiteNameDisplayMode         = other.m_SiteNameDisplayMode;
	m_SiteNameFontSize            = other.m_SiteNameFontSize;
	m_SiteNameColorType           = other.m_SiteNameColorType;
	m_SiteNameDisplayPositionType = other.m_SiteNameDisplayPositionType;
	m_SiteNameTransparance        = other.m_SiteNameTransparance;
	m_SiteNameHorizontalPostion   = other.m_SiteNameHorizontalPostion;
	m_SiteNameVerticalPosition    = other.m_SiteNameVerticalPosition;
	m_SiteNameTextColor           = other.m_SiteNameTextColor;
	m_IsBorderEnabled             = other.m_IsBorderEnabled;
	return *this;
}

//--------------------------------------------------------------------------
CSiteNameInfo::~CSiteNameInfo()
{
}

//--------------------------------------------------------------------------
void CSiteNameInfo::Serialize(WORD format, CSegment& seg)
{
	if (format == NATIVE)
	{
		seg << m_SiteNameDisplayMode
		    << m_SiteNameFontSize
		    << m_SiteNameColorType
		    << m_SiteNameDisplayPositionType
		    << m_SiteNameTransparance
		    << m_SiteNameHorizontalPostion
		    << m_SiteNameVerticalPosition
		    << m_SiteNameTextColor
		    << m_IsBorderEnabled;
	}
}

//--------------------------------------------------------------------------
void CSiteNameInfo::DeSerialize(WORD format, CSegment& seg)
{
	if (format == NATIVE)
	{
		seg >> m_SiteNameDisplayMode
		    >> m_SiteNameFontSize
		    >> m_SiteNameColorType
		    >> m_SiteNameDisplayPositionType
		    >> m_SiteNameTransparance
		    >> m_SiteNameHorizontalPostion
		    >> m_SiteNameVerticalPosition
		    >> m_SiteNameTextColor
		    >> m_IsBorderEnabled;
	}
}

//--------------------------------------------------------------------------
void CSiteNameInfo::Serialize(WORD format, ostream& ostr) const
{
	ostr << (WORD)m_SiteNameDisplayMode << "\n";
	ostr << (DWORD)m_SiteNameFontSize << "\n";
	ostr << (WORD)m_SiteNameColorType << "\n";
	ostr << (WORD)m_SiteNameDisplayPositionType << "\n";
	ostr << (DWORD)m_SiteNameTransparance << "\n";
	ostr << (DWORD)m_SiteNameHorizontalPostion << "\n";
	ostr << (DWORD)m_SiteNameVerticalPosition << "\n";
	ostr << (WORD)m_SiteNameTextColor << "\n";
	ostr << (WORD)m_IsBorderEnabled << "\n";
}

//--------------------------------------------------------------------------
void CSiteNameInfo::DeSerialize(WORD format, istream& istr)
{
	if (format != NATIVE)
	{
		PASSERT_AND_RETURN(1);
	}

	WORD tmpWord = 0;

	istr >> m_SiteNameDisplayMode;
	istr >> m_SiteNameFontSize;
	istr >> m_SiteNameColorType;
	istr >> m_SiteNameDisplayPositionType;
	istr >> m_SiteNameTransparance;
	istr >> m_SiteNameHorizontalPostion;
	istr >> m_SiteNameVerticalPosition;
	istr >> m_SiteNameTextColor;
	istr >> tmpWord;
	// istr >> m_IsBorderEnabled;
	m_IsBorderEnabled = (BYTE)tmpWord;
}

//--------------------------------------------------------------------------
void CSiteNameInfo::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pChildNode;
	pChildNode = pFatherNode->AddChildNode("SITE_NAME");
	pChildNode->AddChildNode("SITE_NAME_DISPLAY_MODE", m_SiteNameDisplayMode, SITE_NAME_DISPLAY_MODE_ENUM);
	pChildNode->AddChildNode("SITE_NAME_FONT_SIZE", m_SiteNameFontSize);
	pChildNode->AddChildNode("SITE_NAME_COLOR", m_SiteNameColorType, MESSAGE_OVERLAY_COLOR_TYPE_ENUM);
	pChildNode->AddChildNode("SITE_NAME_DISPLAY_POSITION", m_SiteNameDisplayPositionType, SITE_NAME_DISPLAY_POSITION_ENUM);
	pChildNode->AddChildNode("SITE_NAME_TRANSPARENCE", m_SiteNameTransparance);
	pChildNode->AddChildNode("SITE_NAME_HORIZONTAL_POSITION", m_SiteNameHorizontalPostion);
	pChildNode->AddChildNode("SITE_NAME_VERTICAL_POSITION", m_SiteNameVerticalPosition);
	pChildNode->AddChildNode("SITE_NAME_TEXT_COLOR", m_SiteNameTextColor, TEXT_COLOR_TYPE_ENUM);
}

//--------------------------------------------------------------------------
int CSiteNameInfo::DeSerializeXml(CXMLDOMElement* pResNode, char* pszError, const char* strAction)
{
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CSiteNameInfo::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "SITE_NAME_DISPLAY_MODE", &m_SiteNameDisplayMode, SITE_NAME_DISPLAY_MODE_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "SITE_NAME_FONT_SIZE", &m_SiteNameFontSize, _0_TO_SITE_NAME_FONT_SIZE);
	GET_VALIDATE_CHILD(pActionNode, "SITE_NAME_COLOR", &m_SiteNameColorType, MESSAGE_OVERLAY_COLOR_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "SITE_NAME_DISPLAY_POSITION", &m_SiteNameDisplayPositionType, SITE_NAME_DISPLAY_POSITION_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "SITE_NAME_TRANSPARENCE", &m_SiteNameTransparance, _0_TO_SITE_NAME_TRANCEPARENCE);
	GET_VALIDATE_CHILD(pActionNode, "SITE_NAME_HORIZONTAL_POSITION", &m_SiteNameHorizontalPostion, _0_TO_SITE_NAME_CUSTOM_POSITION);
	GET_VALIDATE_CHILD(pActionNode, "SITE_NAME_VERTICAL_POSITION", &m_SiteNameVerticalPosition, _0_TO_SITE_NAME_CUSTOM_POSITION);
	GET_VALIDATE_CHILD(pActionNode, "SITE_NAME_TEXT_COLOR", &m_SiteNameTextColor, TEXT_COLOR_TYPE_ENUM);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CSiteNameInfo::Dump(const char* functionName) const
{
	std::ostringstream msg;

	msg << "CSiteNameInfo::Dump - called from " << functionName
	    << "\n  DisplayMode             :" << m_SiteNameDisplayMode
	    << "\n  FontSize                :" << m_SiteNameFontSize
	    << "\n  ColorType               :" << m_SiteNameColorType
	    << "\n  DisplayPositionType     :" << m_SiteNameDisplayPositionType
	    << "\n  Transparency            :" << m_SiteNameTransparance
	    << "\n  HorizontalPostion       :" << m_SiteNameHorizontalPostion
	    << "\n  VerticalPosition        :" << m_SiteNameVerticalPosition
	    << "\n  TextColor               :" << m_SiteNameTextColor
	    << "\n  BorderEnabled           :" << (int)m_IsBorderEnabled;

	PTRACE(eLevelInfoNormal, (char*)msg.str().c_str());
}

//--------------------------------------------------------------------------
void CSiteNameInfo::GetCoordinateXY(long& x, long& y)
{
	switch (m_SiteNameDisplayPositionType)
	{
		case eLeft_top_position:
		{
			x = 0;
			y = 0;
		}
		break;

		case eTop_position:
		{
			x = 50;
			y = 0;
		}
		break;

		case eRight_top_position:
		{
			x = 100;
			y = 0;
		}
		break;

		case eLeft_middle_position:
		{
			x = 0;
			y = 50;
		}
		break;

		case eRight_middle_position:
		{
			x = 100;
			y = 50;
		}
		break;

		case eLeft_bottom_position:
		{
			x = 0;
			y = 100;
		}
		break;

		case eBottom_position:
		{
			x = 50;
			y = 100;
		}
		break;

		case eRight_bottom_position:
		{
			x = 100;
			y = 100;
		}
		break;

		case eCustom:
		{
			x = m_SiteNameHorizontalPostion;
			y = m_SiteNameVerticalPosition;
		}
		break;

		default:
		{
			x = 10;
			y = 10;
		}
		break;
	} // switch
}

//--------------------------------------------------------------------------
void CSiteNameInfo::GetCoordinateXY(char& x, char& y)
{
	long tmpX, tmpY;
	GetCoordinateXY(tmpX, tmpY);

	if (tmpX < 0)
	{
		tmpX = 0;
		PTRACE(eLevelInfoNormal, "CSiteNameInfo::GetCoordinateXY - x is less than 0; set to 0");
		DBGPASSERT(1);
	}
	else if (tmpX > 100)
	{
		tmpX = 100;
		PTRACE(eLevelInfoNormal, "CSiteNameInfo::GetCoordinateXY - x is greater than 100; set to 100");
		DBGPASSERT(1);
	}

	if (tmpY < 0)
	{
		PTRACE(eLevelInfoNormal, "CSiteNameInfo::GetCoordinateXY - y is less than 0; set to 0");
		DBGPASSERT(1);
		tmpY = 0;
	}
	else if (tmpY > 100)
	{
		tmpY = 100;
		PTRACE(eLevelInfoNormal, "CSiteNameInfo::GetCoordinateXY - y is greater than 100; set to 100");
		DBGPASSERT(1);
	}

	x = (char)tmpX;
	y = (char)tmpY;
}

