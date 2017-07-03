#include <ostream>
#include <istream>
#include <sstream>
#include "LectureModeParams.h"
#include "InitCommonStrings.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "Segment.h"
#include "ObjString.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//                        CLectureModeParams
////////////////////////////////////////////////////////////////////////////
CLectureModeParams::CLectureModeParams()
{
	m_LectureModeOnOff = NO;
	m_LecturerName[0]  ='\0';
	m_TimeInterval     = 0xFFFF;
	m_timerOnOff       = NO;
	m_audioActivated   = NO;
	m_lecturerId       = 0xFFFFFFFF;
}

/////////////////////////////////////////////////////////////////////////////
CLectureModeParams::CLectureModeParams(const CLectureModeParams &other) : CPObject(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
CLectureModeParams::~CLectureModeParams()
{

}

/////////////////////////////////////////////////////////////////////////////
CLectureModeParams& CLectureModeParams::operator =(const CLectureModeParams& other)
{
	if (&other == this)
		return *this;

	m_LectureModeOnOff = other.m_LectureModeOnOff;
	m_TimeInterval     = other.m_TimeInterval;
	m_timerOnOff       = other.m_timerOnOff;
	m_audioActivated   = other.m_audioActivated;
	m_lecturerId       = other.m_lecturerId;
	strcpy_safe(m_LecturerName, other.m_LecturerName);

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CLectureModeParams::DeSerialize(WORD format, CSegment& seg)
{
	seg
		>> m_LectureModeOnOff
		>> m_TimeInterval
		>> m_LecturerName
		>> m_audioActivated
		>> m_timerOnOff;
	m_LecturerName[H243_NAME_LEN-1]='\0';
}
/////////////////////////////////////////////////////////////////////////////
void CLectureModeParams::Serialize(WORD format, CSegment& seg)
{
	seg
		<< m_LectureModeOnOff
		<< m_TimeInterval
		<< m_LecturerName
		<< m_audioActivated
		<< m_timerOnOff;
}
/////////////////////////////////////////////////////////////////////////////
void CLectureModeParams::Dump(std::ostream& msg) const
{
	msg.setf(std::ios_base::left);
	msg << "\n  Mode           : ";
	switch (m_LectureModeOnOff)
	{
		case 0 : { msg << "None.       "     ; break; }
		case 1 : { msg << "LectureMode."     ; break; }
		case 2 : { msg << "LectureShow."     ; break; }
		case 3 : { msg << "PresentationMode."; break; }
		default: { PASSERT(1); }
	}

	msg << "\n  AudioActivated : ";
	if (m_audioActivated)
		msg << "YES";
	else
		msg << "NO";

	msg << "\n  Lecturer Name  : " << m_LecturerName << "\n  Timer          : ";
	if (m_timerOnOff)
		msg << "ON";
	else
		msg << "OFF";

	msg << "\n  Interval       : " << m_TimeInterval << " sec.";
}

/////////////////////////////////////////////////////////////////////////////
void CLectureModeParams::PrintAll() const
{
	std::ostringstream msg;
	Dump(msg);
	TRACEINTO << msg.str().c_str();
}

/////////////////////////////////////////////////////////////////////////////
void CLectureModeParams::SetLectureModeRegularFromLecturerName()
{
	DWORD len = 0;
	len = strlen(m_LecturerName);

	//it can happen using MGC custom API, see VNGFE-3640 and BRIDGE-7706
	if ((len > 0) && (0 != strcmp(m_LecturerName, "[Auto Select]") && (0 != strcmp(m_LecturerName, "[None]"))))
	{
		if (m_LectureModeOnOff == 0)
		{ // None
			m_LectureModeOnOff = 1; // LectureMode
			TRACEINTO << "LecturerName:" << m_LecturerName << " - Set from 'None' to 'LectureMode'";
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CLectureModeParams::Serialize(WORD format, std::ostream& ostr)
{
	ostr << (WORD)m_LectureModeOnOff << "\n";
	ostr << m_LecturerName << "\n";
	ostr << m_TimeInterval << "\n";
	ostr << (WORD)m_timerOnOff << "\n";
	ostr << (WORD)m_audioActivated << "\n";
	ostr << m_lecturerId << "\n";
}

/////////////////////////////////////////////////////////////////////////////
void CLectureModeParams::DeSerialize(WORD format, std::istream& istr)
{
	WORD tmp;

	istr >> tmp;
	m_LectureModeOnOff = (BYTE)tmp;
	istr.ignore(1);

	istr.getline(m_LecturerName, H243_NAME_LEN + 1, '\n');
	istr >> m_TimeInterval;
	istr >> tmp;
	m_timerOnOff = (BYTE)tmp;
	istr >> tmp;
	m_audioActivated = (BYTE)tmp;
	istr >> m_lecturerId;
}

/////////////////////////////////////////////////////////////////////////////
int CLectureModeParams::DeSerializeXml(CXMLDOMElement *pLectureModeNode, char *pszError)
{
	int nStatus;

	GET_VALIDATE_CHILD(pLectureModeNode, "ON", &m_LectureModeOnOff, _BOOL);
	GET_VALIDATE_CHILD(pLectureModeNode, "TIMER", &m_timerOnOff, _BOOL);
	GET_VALIDATE_CHILD(pLectureModeNode, "INTERVAL", &m_TimeInterval, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pLectureModeNode, "AUDIO_ACTIVATED", &m_audioActivated, _BOOL);
	GET_VALIDATE_CHILD(pLectureModeNode, "LECTURE_NAME", m_LecturerName, _0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pLectureModeNode, "LECTURE_MODE_TYPE", &m_LectureModeOnOff, LECTURE_MODE_TYPE_ENUM);
	GET_VALIDATE_CHILD(pLectureModeNode, "LECTURE_ID", &m_lecturerId, _0_TO_DWORD);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CLectureModeParams::SerializeXml(CXMLDOMElement *pFatherNode)
{
	CXMLDOMElement *pTempNode;

	pTempNode = pFatherNode->AddChildNode("LECTURE_MODE");
	pTempNode->AddChildNode("ON", m_LectureModeOnOff, _BOOL);
	pTempNode->AddChildNode("TIMER", m_timerOnOff, _BOOL);
	pTempNode->AddChildNode("INTERVAL", m_TimeInterval);
	pTempNode->AddChildNode("AUDIO_ACTIVATED", m_audioActivated, _BOOL);
	pTempNode->AddChildNode("LECTURE_NAME", m_LecturerName);
	pTempNode->AddChildNode("LECTURE_MODE_TYPE", m_LectureModeOnOff, LECTURE_MODE_TYPE_ENUM);
	pTempNode->AddChildNode("LECTURE_ID", m_lecturerId);
}

