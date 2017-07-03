#include <ostream>
#include <istream>
#include "MessageOverlayInfo.h"
#include "Segment.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "ConfPartyApiDefines.h"
#include "VideoStructs.h"

////////////////////////////////////////////////////////////////////////////
//                        CMessageOverlayInfo
////////////////////////////////////////////////////////////////////////////
CMessageOverlayInfo::CMessageOverlayInfo()
{
	m_MessageOn                      = FALSE;
	m_MessageText                    = "";
	m_MessageNumOfRepetitions        = 3;
	m_MessageFontSize                = 12;
	m_MessageColorType               = eWhite_font_on_light_blue_background;
	m_MessageDisplayPositionType     = 90;
	m_MessageDisplaySpeedType        = eSlow;
	m_MessageTransparency            = 50;
	m_MessageFontSize_Old            = eSmall;
	m_MessageDisplayPositionType_Old = eBottom;
}

//--------------------------------------------------------------------------
CMessageOverlayInfo::CMessageOverlayInfo(const CMessageOverlayInfo& other)
	: CSerializeObject(other)
{
	m_MessageOn                      = other.m_MessageOn;
	m_MessageText                    = other.m_MessageText;
	m_MessageNumOfRepetitions        = other.m_MessageNumOfRepetitions;
	m_MessageFontSize                = other.m_MessageFontSize;
	m_MessageColorType               = other.m_MessageColorType;
	m_MessageDisplayPositionType     = other.m_MessageDisplayPositionType;
	m_MessageDisplaySpeedType        = other.m_MessageDisplaySpeedType;
	m_MessageTransparency            = other.m_MessageTransparency;
	m_MessageFontSize_Old            = other.m_MessageFontSize_Old;
	m_MessageDisplayPositionType_Old = other.m_MessageDisplayPositionType_Old;
}

//--------------------------------------------------------------------------
CMessageOverlayInfo& CMessageOverlayInfo::operator =(const CMessageOverlayInfo& other)
{
	if (&other == this) return *this;

	m_MessageOn                      = other.m_MessageOn;
	m_MessageText                    = other.m_MessageText;
	m_MessageNumOfRepetitions        = other.m_MessageNumOfRepetitions;
	m_MessageFontSize                = other.m_MessageFontSize;
	m_MessageColorType               = other.m_MessageColorType;
	m_MessageDisplayPositionType     = other.m_MessageDisplayPositionType;
	m_MessageDisplaySpeedType        = other.m_MessageDisplaySpeedType;
	m_MessageTransparency            = other.m_MessageTransparency;
	m_MessageFontSize_Old            = other.m_MessageFontSize_Old;
	m_MessageDisplayPositionType_Old = other.m_MessageDisplayPositionType_Old;

	return *this;
}

//--------------------------------------------------------------------------
CMessageOverlayInfo::~CMessageOverlayInfo()
{
}

//--------------------------------------------------------------------------
void CMessageOverlayInfo::Serialize(WORD format, CSegment& seg)
{
	if (format == NATIVE)
	{
		seg << m_MessageOn
		    << m_MessageDisplayPositionType
		    << m_MessageTransparency
		    << m_MessageDisplaySpeedType
		    << m_MessageColorType
		    << m_MessageFontSize
		    << m_MessageNumOfRepetitions
		    << m_MessageText
		    << m_MessageFontSize_Old
		    << m_MessageDisplayPositionType_Old;
	}
}

//--------------------------------------------------------------------------
void CMessageOverlayInfo::DeSerialize(WORD format, CSegment& seg)
{
	if (format == NATIVE)
	{
		m_MessageText.clear();

		seg >>  m_MessageOn
		>>  m_MessageDisplayPositionType
		>>  m_MessageTransparency
		>>  m_MessageDisplaySpeedType
		>>  m_MessageColorType
		>>  m_MessageFontSize
		>>  m_MessageNumOfRepetitions
		>>  m_MessageText
		>>  m_MessageFontSize_Old
		>>  m_MessageDisplayPositionType_Old;
	}
}

//--------------------------------------------------------------------------
void CMessageOverlayInfo::Serialize(WORD format, ostream& ostr) const
{
	ostr <<  (WORD)m_MessageOn << "\n";
	ostr << m_MessageDisplayPositionType << "\n";
	ostr << m_MessageTransparency << "\n";
	ostr << m_MessageDisplaySpeedType << "\n";
	ostr << m_MessageColorType << "\n";
	ostr << m_MessageFontSize << "\n";
	ostr << m_MessageNumOfRepetitions << "\n";
	ostr << m_MessageFontSize_Old << "\n";
	ostr << m_MessageDisplayPositionType_Old << "\n";

	char tmp_name[MAX_MESSAGE_OVERLAY_STRING_LENGTH+1];
	strcpy_safe(tmp_name, m_MessageText.c_str());
	ostr << tmp_name << "\n";
}

//--------------------------------------------------------------------------
void CMessageOverlayInfo::DeSerialize(WORD format, istream& istr)
{
	if (format != NATIVE) {
		PASSERT_AND_RETURN(1);
	}

	m_MessageText.clear();
	WORD tmp;
	istr >> tmp;
	m_MessageOn   = (BYTE)tmp;

	istr >> m_MessageDisplayPositionType;
	istr >> m_MessageTransparency;
	istr >> m_MessageDisplaySpeedType;
	istr >> m_MessageColorType;
	istr >> m_MessageFontSize;
	istr >> m_MessageNumOfRepetitions;
	istr >> m_MessageFontSize_Old;
	istr >> m_MessageDisplayPositionType_Old;

	istr.ignore(1);
	char tmp_name[MAX_MESSAGE_OVERLAY_STRING_LENGTH+1];
	memset(tmp_name, '\0', MAX_MESSAGE_OVERLAY_STRING_LENGTH+1);
	istr.getline(tmp_name, MAX_MESSAGE_OVERLAY_STRING_LENGTH + 1, '\n');
	m_MessageText = tmp_name;
}

//--------------------------------------------------------------------------
void CMessageOverlayInfo::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pChildNode;
	pChildNode = pFatherNode->AddChildNode("MESSAGE_OVERLAY");
	pChildNode->AddChildNode("ON", m_MessageOn, _BOOL);
	pChildNode->AddChildNode("MESSAGE_TEXT", m_MessageText);
	pChildNode->AddChildNode("MESSAGE_FONT_SIZE", m_MessageFontSize_Old, MESSAGE_OVERLAY_FONT_TYPE_ENUM);
	pChildNode->AddChildNode("MESSAGE_FONT_SIZE_INT", m_MessageFontSize);
	pChildNode->AddChildNode("MESSAGE_COLOR", m_MessageColorType, MESSAGE_OVERLAY_COLOR_TYPE_ENUM);
	pChildNode->AddChildNode("NUM_OF_REPETITIONS", m_MessageNumOfRepetitions);
	pChildNode->AddChildNode("MESSAGE_DISPLAY_SPEED", m_MessageDisplaySpeedType, MESSAGE_OVERLAY_SPEED_TYPE_ENUM);
	pChildNode->AddChildNode("MESSAGE_DISPLAY_POSITION", m_MessageDisplayPositionType_Old, MESSAGE_OVERLAY_DISPALY_POSITION_TYPE_ENUM);
	pChildNode->AddChildNode("MESSAGE_DISPLAY_POSITION_INT", m_MessageDisplayPositionType);
	pChildNode->AddChildNode("MESSAGE_TRANSPARENCE", m_MessageTransparency);
}

//--------------------------------------------------------------------------
int CMessageOverlayInfo::DeSerializeXml(CXMLDOMElement* pResNode, char* pszError, const char* strAction)
{
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CMessageOverlayInfo::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;
	GET_VALIDATE_CHILD(pActionNode, "ON", &m_MessageOn, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "MESSAGE_TEXT", m_MessageText, _0_TO_MESSAGE_OVERLAY_TEXT_LENGTH);
	GET_VALIDATE_CHILD(pActionNode, "MESSAGE_FONT_SIZE", &m_MessageFontSize_Old, MESSAGE_OVERLAY_FONT_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "MESSAGE_FONT_SIZE_INT", &m_MessageFontSize, _0_TO_MESSAGE_OVERLAY_FONT_SIZE);
	GET_VALIDATE_CHILD(pActionNode, "MESSAGE_COLOR", &m_MessageColorType, MESSAGE_OVERLAY_COLOR_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "NUM_OF_REPETITIONS", &m_MessageNumOfRepetitions, _0_TO_MESSAGE_OVERLAY_NUM_OF_REPETITIONS);
	GET_VALIDATE_CHILD(pActionNode, "MESSAGE_DISPLAY_SPEED", &m_MessageDisplaySpeedType, MESSAGE_OVERLAY_SPEED_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "MESSAGE_DISPLAY_POSITION", &m_MessageDisplayPositionType_Old, MESSAGE_OVERLAY_DISPALY_POSITION_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "MESSAGE_DISPLAY_POSITION_INT", &m_MessageDisplayPositionType, _0_TO_MESSAGE_OVERLAY_DISPLAY_POSITION);
	GET_VALIDATE_CHILD(pActionNode, "MESSAGE_TRANSPARENCE", &m_MessageTransparency, _0_TO_MESSAGE_OVERLAY_TRANSPARENCE);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CMessageOverlayInfo::Dump(const char* functionName) const
{
	std::ostringstream msg;

	msg << "CMessageOverlayInfo::Dump - called from " << functionName
	    << "\n  Text                    :" << m_MessageText.c_str()
	    << "\n  FontSize                :" << (int)m_MessageFontSize
	    << "\n  ColorType               :" << (int)m_MessageColorType
	    << "\n  NumOfRepetitions        :" << (int)m_MessageNumOfRepetitions
	    << "\n  DisplaySpeedType        :" << (int)m_MessageDisplaySpeedType
	    << "\n  DisplayPositionType     :" << (int)m_MessageDisplayPositionType
	    << "\n  Transparency            :" << (int)m_MessageTransparency
	    << "\n  FontSize_Old            :" << (int)m_MessageFontSize_Old
	    << "\n  DisplayPositionType_Old :" << (int)m_MessageDisplayPositionType_Old;

	PTRACE(eLevelInfoNormal, (char*)msg.str().c_str());
}
