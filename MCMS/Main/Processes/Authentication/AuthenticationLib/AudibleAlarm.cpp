/*
 * FailoverConfiguration.cpp
 *
 *  Created on: Aug 26, 2009
 *      Author: yael
 */

#include <ostream>
#include <istream>
#include "StatusesGeneral.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "ConfPartyApiDefines.h"
#include "Segment.h"
#include "AudibleAlarm.h"
#include "Trace.h"
#include "TraceStream.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAudibleAlarm::CAudibleAlarm()
{
	//m_AudibleAlarmType = eAwaitingOperatorAssistance;
	m_AudibleAlarmType = 0;
	m_bIsEnabled = FALSE;
	m_bIsRepeat = FALSE;
	m_numOfRepetitions = 3;
	m_Repetitions_interval = 40;
	m_user_name = "";

}

/////////////////////////////////////////////////////////////////////////////
CAudibleAlarm& CAudibleAlarm::operator = (const CAudibleAlarm &other)
{
	m_AudibleAlarmType = other.m_AudibleAlarmType;
	m_bIsEnabled = other.m_bIsEnabled;
	m_bIsRepeat = other.m_bIsRepeat;
	m_numOfRepetitions = other.m_numOfRepetitions;
	m_Repetitions_interval = other.m_Repetitions_interval;
	m_user_name = other.m_user_name;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CAudibleAlarm::~CAudibleAlarm()
{

}

///////////////////////////////////////////////////////////////////////////
void CAudibleAlarm::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pTempNode;

	pTempNode = pFatherNode->AddChildNode("AUDIBLE_ALARM_DATA");
	pTempNode->AddChildNode("AUDIBLE_ALARM_TYPE",m_AudibleAlarmType,AUDIBLE_ALARM_TYPE_ENUM);
	pTempNode->AddChildNode("ENABLE",m_bIsEnabled,_BOOL);
	pTempNode->AddChildNode("REPEAT",m_bIsRepeat,_BOOL);
	pTempNode->AddChildNode("NUM_OF_AUDIBLE_REPETITIONS",m_numOfRepetitions,_1_TO_NUMBER_OF_AUDIBLE_REPETITIONS);
	pTempNode->AddChildNode("REPETITIONS_INTERVAL",m_Repetitions_interval,_5_TO_NUMBER_OF_AUDIBLE_INTERVALS);


}

//////////////////////////////////////////////////////////
int CAudibleAlarm::DeSerializeXml(CXMLDOMElement *pNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pAudibleAlarmDataNode;
	GET_VALIDATE_CHILD(pNode,"USER_NAME",m_user_name,_0_TO_OPERATOR_NAME_LENGTH);
	GET_CHILD_NODE(pNode, "AUDIBLE_ALARM_DATA", pAudibleAlarmDataNode);
	if(pAudibleAlarmDataNode != NULL)
	{
		TRACESTR(eLevelInfoNormal) << "\nCAudibleAlarm::DeSerializeXml: child nodes";
		GET_VALIDATE_CHILD(pAudibleAlarmDataNode,"AUDIBLE_ALARM_TYPE",&m_AudibleAlarmType,AUDIBLE_ALARM_TYPE_ENUM);
		GET_VALIDATE_CHILD(pAudibleAlarmDataNode,"ENABLE",&m_bIsEnabled,_BOOL);
		GET_VALIDATE_CHILD(pAudibleAlarmDataNode,"REPEAT",&m_bIsRepeat,_BOOL);
		if (m_bIsRepeat == TRUE)
		{
			GET_VALIDATE_CHILD(pAudibleAlarmDataNode,"NUM_OF_AUDIBLE_REPETITIONS",&m_numOfRepetitions,_1_TO_NUMBER_OF_AUDIBLE_REPETITIONS);
			if (nStatus != STATUS_OK)
				return nStatus;
			//FTRACESTR(eLevelInfoNormal) << "CAudibleAlarm::DeSerializeXml- status:" << nStatus;
			GET_VALIDATE_CHILD(pAudibleAlarmDataNode,"REPETITIONS_INTERVAL",&m_Repetitions_interval,_5_TO_NUMBER_OF_AUDIBLE_INTERVALS);
			//FTRACESTR(eLevelInfoNormal) << "CAudibleAlarm::DeSerializeXml- status:" << nStatus;
		}
		else
		{
			GET_VALIDATE_CHILD(pAudibleAlarmDataNode,"NUM_OF_AUDIBLE_REPETITIONS",&m_numOfRepetitions,_0_TO_WORD);
			GET_VALIDATE_CHILD(pAudibleAlarmDataNode,"REPETITIONS_INTERVAL",&m_Repetitions_interval,_0_TO_WORD);
		}
	}
	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CAudibleAlarm::DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* strAction)
{
	return DeSerializeXml(pNode, pszError);
}
