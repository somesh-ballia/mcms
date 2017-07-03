#ifndef _CLectureModeParams_H_
#define _CLectureModeParams_H_

#include "PObject.h"
#include "ConfPartyApiDefines.h"
#include "ConfPartySharedDefines.h"
#include "SystemFunctions.h"

class CXMLDOMElement;
class CVideoBridgeLectureModeParams;
class CSegment;

////////////////////////////////////////////////////////////////////////////
//                        CLectureModeParams
////////////////////////////////////////////////////////////////////////////
class CLectureModeParams : public CPObject
{
	CLASS_TYPE_1(CLectureModeParams, CPObject)

public:
	                    CLectureModeParams();
	                    CLectureModeParams(const CLectureModeParams& other);
	virtual            ~CLectureModeParams();
	const char*         NameOf() const { return "CLectureModeParams"; }
	CLectureModeParams& operator=(const CLectureModeParams& other);
	void                Dump( std::ostream& msg ) const;

	// Implementation
	void                SetLectureModeType(BYTE lectureModeType)  { m_LectureModeOnOff = lectureModeType; }
	BYTE                GetLectureModeType() const                { return m_LectureModeOnOff; }

	void                SetLecturerName(const char* name)         { strcpy_safe(m_LecturerName, name); }
	const char*         GetLecturerName () const                  { return m_LecturerName; }

	void                SetLectureTimeInterval(WORD TimeInterval) { m_TimeInterval = TimeInterval; }
	WORD                GetLectureTimeInterval() const            { return m_TimeInterval; }

	void                SetTimerOnOff(BYTE timerOnOff)            { m_timerOnOff = timerOnOff; }
	BYTE                GetTimerOnOff() const                     { return m_timerOnOff; }

	void                SetAudioActivated(BYTE audioActivated)    { m_audioActivated = audioActivated; }
	BYTE                GetAudioActivated() const                 { return m_audioActivated; }

	void                SetLecturerId(PartyMonitorID lecturerId)  { m_lecturerId = lecturerId; }
	PartyMonitorID      GetLecturerId() const                     { return m_lecturerId; }

	void                Serialize(WORD format, CSegment& seg);
	void                DeSerialize(WORD format, CSegment& seg);
	void                Serialize(WORD format, std::ostream& ostr);
	void                DeSerialize(WORD format, std::istream& istr);
	int                 DeSerializeXml(CXMLDOMElement* pLectureModeNode, char* pszError);
	void                SerializeXml(CXMLDOMElement* pFatherNode);


	void                PrintAll() const;
	void                SetLectureModeRegularFromLecturerName();

	BYTE                m_LectureModeOnOff;
	char                m_LecturerName[H243_NAME_LEN];
	WORD                m_TimeInterval;
	BYTE                m_timerOnOff;
	BYTE                m_audioActivated;
	PartyMonitorID      m_lecturerId;
};

#endif //_CLectureModeParams_H_
