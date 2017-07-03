//============================================================================
// Name        : ExchangeDataTypes.cpp
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2009
// Description : Data types for Exchange negotiation
//============================================================================

#include <iomanip>
#include <list>
#include <time.h>

#include "StatusesGeneral.h"
#include "ParseHelper.h"
#include "EncodeHelper.h"
#include "ExchangeDataTypes.h"
#include <iostream>


const std::string CCalendarItem::CALENDAR_ITEM_TYPE         = "t:CalendarItem";
const std::string CCalendarItem::INBOX_ITEM_TYPE            = "t:ItemId";
const std::string CCalendarItem::INBOX_MEETING_REQUEST_TYPE = "t:MeetingRequest";

const std::string CCalendarItem::POLYCOM_BODY_HEADER        = "--=BEGIN POLYCOM VMR ENCODED TOKEN=--";
const std::string CCalendarItem::POLYCOM_BODY_FOOTER        = "--=END POLYCOM VMR ENCODED TOKEN=--";

const std::string CCalendarItem::POLYCOM_FIELD_DIALINPREFIX_SEQ          = "POLYCOM-DIALINPREFIX";
const std::string CCalendarItem::POLYCOM_FIELD_NUMERIC_ID_SEQ            = "POLYCOM-VMRNUMBER";
const std::string CCalendarItem::POLYCOM_FIELD_MEETINGPASSWORD_SEQ       = "POLYCOM-MEETINGPASSWORD";
const std::string CCalendarItem::POLYCOM_FIELD_CHAIRPASSWORD_SEQ         = "POLYCOM-CHAIRPASSWORD";
const std::string CCalendarItem::POLYCOM_FIELD_CHAIRPASSWORDREQUIRED_SEQ = "POLYCOM-CHAIRPASSWORDREQUIRED";
const std::string CCalendarItem::POLYCOM_FIELD_RECORDMEETING_SEQ         = "POLYCOM-RECORDMEETING";
const std::string CCalendarItem::POLYCOM_FIELD_STREAMMEETING_SEQ         = "POLYCOM-STREAMMEETING";
const std::string CCalendarItem::POLYCOM_FIELD_LANGUAGE_SEQ              = "POLYCOM-INVITATIONLANGUAGE";
const std::string CCalendarItem::POLYCOM_FIELD_AUDIONUMBER1_SEQ          = "POLYCOM-AUDIONUMBER1";
const std::string CCalendarItem::POLYCOM_FIELD_AUDIONUMBER2_SEQ          = "POLYCOM-AUDIONUMBER2";
const std::string CCalendarItem::POLYCOM_FIELD_SIGNALINGPREFIX_SEQ       = "POLYCOM-SIGNALINGPREFIX";
const std::string CCalendarItem::POLYCOM_FIELD_SIGNALINGPOSTFIX_SEQ      = "POLYCOM-SIGNALINGPOSTFIX";

//-POLYCOM-VERSION=1
//+POLYCOM-DIALINPREFIX=75
//+POLYCOM-VMRNUMBER=22222
//-POLYCOM-RSSVMRNAME=397993170157
//+POLYCOM-MEETINGPASSWORD=4242
//+POLYCOM-CHAIRPASSWORD=i61HdFJAoY5F4aZvJxeqIQ==
//+POLYCOM-CHAIRPASSWORDREQUIRED=false
//+POLYCOM-RECORDMEETING=false
//+POLYCOM-STREAMMEETING=false
//+POLYCOM-INVITATIONLANGUAGE=en_US
//-POLYCOM-RECORDINGFILENAME=VMR_1234.asf
//-POLYCOM-RECORDINGURI=http://rss.polycom.com/
//+POLYCOM-AUDIONUMBER1=+1 (800) 555-6789
//+POLYCOM-AUDIONUMBER2=+1 (303) 555-7890
//+POLYCOM-SIGNALINGPREFIX=sip:
//+POLYCOM-SIGNALINGPOSTFIX=example.com


////////////////////////////////////////////////////////////////////////////////////////////////////
CCalendarItem::CCalendarItem() : m_bRecording(FALSE)
{
//	puts("DEBUG>>> CCalendarItem::CCalendarItem");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CCalendarItem::CCalendarItem(const CCalendarItem& other) : CPObject(other)
{
//	puts("DEBUG>>> CCalendarItem::CCalendarItem(copy)");
	*this = other;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CCalendarItem::~CCalendarItem()
{
//	puts("DEBUG>>> CCalendarItem::~CCalendarItem");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CCalendarItem::Dump(std::ostream& stream) const
{
	stream 	<< "ItemId              = " << m_strItemId << "\n"
			<< "ChangeKey           = " << m_strItemChangeKey << "\n"
			<< "ItemUId             = " << m_strItemUId << "\n"
			<< "ItemClass           = " << m_strItemClass << "\n"
			<< "NumericId           = " << m_strNumericId << "\n"
			<< "MyResponseType      = " << m_strMyResponseType << "\n"
			<< "AppointmentSubject  = " << m_strAppointmentSubject << "\n"
			<< "Duration            = " << m_tmDuration.m_day << "d " << m_tmDuration.m_hour << "h " << m_tmDuration.m_min  << "min\n"
			<< "Language            = " << m_strLanguage << "\n"
			<< "Organizer           = " << m_strOrganizer << "\n"
			<< "Recording           = " << ((m_bRecording == TRUE)? "true" : "false") << "\n"
			<< "Streaming           = " << ((m_bStreaming == TRUE)? "true" : "false") << "\n"
		    << "AppoitmentStartTime = " << "Month: " << m_dtExchangeConfStartTime.m_mon << " Day: " << m_dtExchangeConfStartTime.m_day << " at " << m_dtExchangeConfStartTime.m_hour << ":" << m_dtExchangeConfStartTime.m_min  << "\n";

	// print additional info only for DEBUG
//	stream  << "m_strDialInPrefix  = " << m_strDialInPrefix << "\n"
//			<< "m_strMeetingPassword  = " << m_strMeetingPassword << "\n"
//			<< "m_strChairPassword  = " << m_strChairPassword << "\n"
//			<< "m_bChairPasswordRequired  = " << ((m_bChairPasswordRequired == TRUE)? "true" : "false") << "\n"
//			<< "m_bStreaming  = " << ((m_bStreaming == TRUE)? "true" : "false") << "\n"
//			;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CCalendarItem& CCalendarItem::operator= (const CCalendarItem &other)
{
	if(this != &other)
	{
		m_strItemId        = other.m_strItemId;
		m_strItemUId       = other.m_strItemUId;
		m_strItemClass        = other.m_strItemClass;
		m_strItemChangeKey = other.m_strItemChangeKey;
		m_strMyResponseType = other.m_strMyResponseType;
		m_strOrganizer     = other.m_strOrganizer;
		m_strAppointmentSubject = other.m_strAppointmentSubject;
		m_tmDuration       = other.m_tmDuration;
		m_strFullData      = other.m_strFullData;

		m_strDialInPrefix  = other.m_strDialInPrefix;
		m_strNumericId     = other.m_strNumericId;
		m_strMeetingPassword = other.m_strMeetingPassword;
		m_strChairPassword = other.m_strChairPassword;
		m_bChairPasswordRequired = other.m_bChairPasswordRequired;
		m_bRecording       = other.m_bRecording;
		m_bStreaming       = other.m_bStreaming;
		m_strLanguage      = other.m_strLanguage;
		m_strAudioNumber1  = other.m_strAudioNumber1;
		m_strAudioNumber2  = other.m_strAudioNumber2;
		m_strSignalingPrefix  = other.m_strSignalingPrefix;
		m_strSignalingPostfix = other.m_strSignalingPostfix;
		m_dtExchangeConfStartTime = other.m_dtExchangeConfStartTime;
	}
	return *this;
}

// parse and update all fields
////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItem::SetFull(const std::string& strFullData,std::string itemType/*t:CalendarItem or INBOX_ITEM_TYPE*/)
{
	return Set(strFullData,TRUE,itemType);
}

// parse and update only ItemId and ItemChangeKey fields
////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItem::SetShort(const std::string& strFullData,std::string itemType/*t:CalendarItem or INBOX_ITEM_TYPE*/)
{
	return Set(strFullData,FALSE,itemType);
}

// parse and update fields
////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItem::Set(const std::string& strFullData, const BOOL isFull,std::string itemType/*t:CalendarItem or INBOX_ITEM_TYPE*/)
{
	std::string itemId;
	std::string changeKey;
	std::string strMyResponseType = "";

	std::string strTmp;
	std::string strCalendarItem;

	ParseHelper::EParserStatus status;

	// parse string, find Id and ChangeKey fields
		// get CalendarItem
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strCalendarItem,strFullData,itemType) ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - <t:CalendarItem>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

		// get ItemId
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strTmp,strCalendarItem,"t:ItemId") ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - <t:ItemId>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
	// get attributes of ItemId - Id and ChangeKey
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeAttributeValue(strTmp,"Id",itemId) ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - attribute <t:ItemId>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeAttributeValue(strTmp,"ChangeKey",changeKey) ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - attribute <ChangeKey>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

		// get MyResponseType only from ItemType t:CalendarItem
	if ( CALENDAR_ITEM_TYPE == itemType )
	{
		if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strTmp,strCalendarItem,"t:MyResponseType") ) )
		{
			PTRACE2(eLevelError,"CCalendarItem::Set - <t:MyResponseType>, status=",ParseHelper::StatusAsString(status));
			return STATUS_FAIL;
		}
		if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeValue(strTmp,strMyResponseType) ) )
		{
			PTRACE2(eLevelError,"CCalendarItem::Set - value of <MyResponseType> is BAD, status=",ParseHelper::StatusAsString(status));
			return STATUS_FAIL;
		}
	}
		// if short serialization -> save and get out
	if( FALSE == isFull )
	{
		m_strItemId = itemId;
		m_strItemChangeKey = changeKey;
		m_strMyResponseType = strMyResponseType;
		return STATUS_OK;
	}

	std::string subject;
		// get Subject
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strTmp,strCalendarItem,"t:Subject") ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - <t:Subject>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeValue(strTmp,subject) ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - value of <Subject> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

//	<t:Organizer>
//		<t:Mailbox>
//			<t:Name>vasily bondarenko</t:Name>
//			<t:EmailAddress>vasily@exchlab.local</t:EmailAddress>
//			<t:RoutingType>SMTP</t:RoutingType>
//		</t:Mailbox>
//	</t:Organizer>
	std::string organizer;
		// get Organizer node
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strTmp,strCalendarItem,"t:Organizer") ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - <t:Organizer>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
		// get Organizer Mailbox node
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strTmp,strTmp,"t:Mailbox") ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - <t:Mailbox>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
		// get Organizer name node
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strTmp,strTmp,"t:Name") ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - <t:Name>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeValue(strTmp,organizer) ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - value of <Name> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

	std::string strTime;
		// get start time
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strTmp,strCalendarItem,"t:Start") ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - <t:Start>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeValue(strTmp,strTime) ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - value of <Start> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
	// get ItemUId
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strTmp,strCalendarItem,"t:UID") ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - <t:ItemUId>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeValue(strTmp,m_strItemUId) ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - value of <UID> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
	// get ItemClass to identify Occurence or Regular Meeting
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strTmp,strCalendarItem,"t:ItemClass") ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - <t:ItemClass>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeValue(strTmp,m_strItemClass) ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - value of <ItemClass> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}


	char szTime[32];
	szTime[31] = 0;

	strncpy(szTime,strTime.c_str(),31);
	CStructTm tmStartTime;
	if( ParseHelper::eStatusOk != ParseHelper::ParseDateTime(szTime,tmStartTime) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - value of <Start> is in BAD FORMAT, value=",strTime.c_str());
		return STATUS_FAIL;
	}

	m_dtExchangeConfStartTime = tmStartTime;

		// get end time
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strTmp,strCalendarItem,"t:End") ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - <t:End>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeValue(strTmp,strTime) ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - value of <End> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
	strncpy(szTime,strTime.c_str(),31);
	CStructTm tmEndTime;
	if( ParseHelper::eStatusOk != ParseHelper::ParseDateTime(szTime,tmEndTime) )
	{
		PTRACE2(eLevelError,"CCalendarItem::Set - value of <End> is in BAD FORMAT, value=",strTime.c_str());
		return STATUS_FAIL;
	}

	time_t timeDiffInSeconds = tmEndTime.GetAbsTime() - tmStartTime.GetAbsTime();
	if( timeDiffInSeconds <= 0 )
	{
		PTRACE2INT(eLevelError,"CCalendarItem::Set - Appointment duration is illegal, dur=",timeDiffInSeconds);
		return STATUS_FAIL;
	}
	int hours = timeDiffInSeconds / 3600;
	timeDiffInSeconds -= hours * 3600;
	int mins = timeDiffInSeconds / 60;
	timeDiffInSeconds -= mins * 60;
	int secs = timeDiffInSeconds;

	CStructTm tmDuration(0,0,0, hours, mins, secs);
//	printf("\n\nDEBUG>>> Duration %d:%d:%d\n",hours,mins,secs);

//	PTRACE2(eLevelError,"CCalendarItem::Set - CalendarItem:\n",strCalendarItem.c_str());

	if( STATUS_OK != DeserializePolycomFields(strCalendarItem) )
	{
		// it may be regular (not Polycom's) appointment
		PTRACE2(eLevelError,"CCalendarItem::Set - Polycom field retrieving failed, Subj=",subject.c_str());
		return STATUS_FAIL;
	}

	m_strItemId = itemId;
	m_strItemChangeKey = changeKey;
	m_strMyResponseType = strMyResponseType;
	m_strAppointmentSubject = subject;
	m_strOrganizer = organizer;
	m_tmDuration = tmDuration;
	m_strFullData = strFullData;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItem::GetPolycomSpecificFields(const std::string& strCalendarItem, const std::string& strFieldName,
		std::string& strFieldValue ) const
{
	ParseHelper::EParserStatus status;

	std::string strExtendedPropertyNode;

	int count = 0;
	while( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strExtendedPropertyNode,strCalendarItem,"t:ExtendedProperty",count) ) )
	{
//<t:ExtendedProperty>
//  <t:ExtendedFieldURI DistinguishedPropertySetId="PublicStrings" PropertyName="POLYCOM-VMRNUMBER" PropertyType="String"/>
//  <t:Value>1234</t:Value>
//</t:ExtendedProperty>

        std::string strURI;
		std::string strAttrPropertyName;

			// get ExtendedFieldURI node
		if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strURI,strExtendedPropertyNode,"t:ExtendedFieldURI") ) )
		{
			PTRACE2(eLevelError,"CCalendarItem::GetPolycomSpecificFields - <t:ExtendedFieldURI> is BAD, status=",ParseHelper::StatusAsString(status));
			//return STATUS_FAIL;
		}
			// get attribute of ExtendedFieldURI - PropertyName
		if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeAttributeValue(strURI,"PropertyName",strAttrPropertyName) ) )
		{
			PTRACE2(eLevelError,"CCalendarItem::GetPolycomSpecificFields - attribute <PropertyName> is BAD, status=",ParseHelper::StatusAsString(status));
			//return STATUS_FAIL;
		}

			// found Polycom field
		if( strFieldName == strAttrPropertyName )
		{
			std::string strValue;
				// get Value node
			if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strValue,strExtendedPropertyNode,"t:Value") ) )
			{
				PTRACE2(eLevelError,"CCalendarItem::GetPolycomSpecificFields - <t:Value> is BAD, status=",ParseHelper::StatusAsString(status));
				return STATUS_FAIL;
			}

				// get Value node value
			if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeValue(strValue,strFieldValue) ) )
			{
				PTRACE2(eLevelError,"CCalendarItem::GetPolycomSpecificFields - value of <t:Value> is BAD, status=",ParseHelper::StatusAsString(status));
				return STATUS_FAIL;
			}

			return STATUS_OK;
		}

		count++;
	}

	return STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItem::DeserializePolycomFields(const std::string& strCalendarItem )
{
	ParseHelper::EParserStatus status;

	std::string strBodyNode;
	std::string strTmp;

		// get Body
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strBodyNode,strCalendarItem,"t:Body") ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::DeserializePolycomFields - <t:Body>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
		// get attribute of Body - BodyType
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeAttributeValue(strBodyNode,"BodyType",strTmp) ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::DeserializePolycomFields - attribute <BodyType>, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
		// BodyType should be "HTML"
	/*if( strTmp != "HTML" )
	{
		PTRACE2(eLevelError,"CCalendarItem::DeserializePolycomFields - attribute <BodyType> is wrong, BodyType=",strTmp.c_str());
	}*/
		// get Body value - html
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeValue(strBodyNode,strTmp) ) )
	{
		PTRACE2(eLevelError,"CCalendarItem::DeserializePolycomFields - <t:Body> HTML value, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

		// in HTML body all '<' and '>' are changed to '&lt;' and '&gt;' - change back
	std::string::size_type start_pos = 0;
	while( std::string::npos != (start_pos = strTmp.find("&lt;")) )
	{
		strTmp.replace(start_pos,strlen("&lt;"),"<");
	}
	while( std::string::npos != (start_pos = strTmp.find("&gt;")) )
	{
		strTmp.replace(start_pos,strlen("&gt;"),">");
	}
	// compatibility with Exchange 2010 issue - change HTML endline code to ASCII endline code
	while( std::string::npos != (start_pos = strTmp.find("&#xD;")) )
	{
		strTmp.replace(start_pos,strlen("&#xD;"),"\n");
	}

//	puts("\n-------strTmp---------\n");
//	puts(strTmp.c_str());
//	puts("\n----------------\n");

	std::string strEncoded; //strTmp.substr(start_pos, end_pos-start_pos);

	// get Polycom HEADER position
	start_pos = strTmp.find(POLYCOM_BODY_HEADER);
	if( std::string::npos == start_pos )
	{
		PTRACE(eLevelError,"CCalendarItem::DeserializePolycomFields - Polycom Header not found");
		return STATUS_FAIL;
	}
	start_pos += POLYCOM_BODY_HEADER.length();

	// get Polycom FOOTER position
	std::string::size_type end_pos = strTmp.find(POLYCOM_BODY_FOOTER,start_pos);
	if( std::string::npos == end_pos )
	{
		PTRACE(eLevelError,"CCalendarItem::DeserializePolycomFields - Polycom Footer not found");
		return STATUS_FAIL;
	}

	strTmp = strTmp.substr(start_pos,end_pos-start_pos);

//	puts("\n-------substring---------\n");
//	puts(strTmp.c_str());
//	puts("\n----------------\n");

	// remove all tags like <div>, <font>, <br>, <\font>, <\div>
	while( std::string::npos != (start_pos = strTmp.find('<')) )
	{
		if( std::string::npos != (end_pos = strTmp.find('>',start_pos)) )
			strTmp = strTmp.erase(start_pos,end_pos-start_pos+1);
	}
//	puts("\n-------no tags---------\n");
//	puts(strTmp.c_str());
//	puts("\n----------------\n");

	// remove all '\r' and '\n'
	strEncoded = strTmp;

	while( std::string::npos != (start_pos = strEncoded.find('\r')) )
	{
		strEncoded = strEncoded.erase(start_pos,1);
	}
	while( std::string::npos != (start_pos = strEncoded.find(10)) )
	{
		strEncoded = strEncoded.erase(start_pos,1);
	}
	while( std::string::npos != (start_pos = strEncoded.find(13)) )
	{
		strEncoded = strEncoded.erase(start_pos,1);
	}
	while( std::string::npos != (start_pos = strEncoded.find('\n')) )
	{
		strEncoded = strEncoded.erase(start_pos,1);
	}

	/*while( std::string::npos != (start_pos = strEncoded.find("&amp;")) )
	{
		strEncoded = strEncoded.erase(start_pos,5);
	}
	while( std::string::npos != (start_pos = strEncoded.find("nbsp;")) )
	{
		strEncoded = strEncoded.erase(start_pos,5);
	}*/

	PTRACE2(eLevelError,"CCalendarItem::DeserializePolycomFields - strEncoded data:" , strEncoded.c_str());
//	puts("\n--------strEncoded--------\n");
//	puts(strEncoded.c_str());
//	puts("\n----------------\n");

	if( strEncoded.length() < 1 )
	{
		PTRACE(eLevelError,"CCalendarItem::DeserializePolycomFields - encoded string is empty");
		return STATUS_FAIL;
	}

	std::string strDecoded = EncodeHelper::base64_decode(strEncoded);

	//PTRACE2(eLevelError,"CCalendarItem::DeserializePolycomFields - strDecoded data:" , strDecoded.c_str());
//	puts("\n--------strDecoded--------\n");
//	puts(strDecoded.c_str());
//	puts("\n----------------\n");

	// we get few lines of such type: POLYCOM-VMRNUMBER=22222
	start_pos = 0;
	end_pos = strDecoded.find('\n',start_pos);

	while( std::string::npos != end_pos )
	{
		std::string strParamPair = strDecoded.substr( start_pos, end_pos-start_pos );

		std::string::size_type eq_pos = strParamPair.find('=');
		if( std::string::npos != eq_pos )
		{
			std::string strKey   = strParamPair.substr(0,eq_pos);
			std::string strValue = strParamPair.substr(eq_pos+1);

//			std::string strTttt = strKey + "->" + strValue;
//			puts(strTttt.c_str());

			SetPolycomField(strKey,strValue);
		}

		start_pos = end_pos + 1;
		end_pos = strDecoded.find('\n',start_pos);
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItem::SetPolycomField(const std::string& strKey, const std::string& strValue )
{
	if( CCalendarItem::POLYCOM_FIELD_DIALINPREFIX_SEQ == strKey )
	{
		m_strDialInPrefix = strValue;
	}
	else if( CCalendarItem::POLYCOM_FIELD_NUMERIC_ID_SEQ == strKey )
	{
		m_strNumericId = strValue;
	}
	else if( CCalendarItem::POLYCOM_FIELD_MEETINGPASSWORD_SEQ == strKey )
	{
		m_strMeetingPassword = strValue;
	}
	else if( CCalendarItem::POLYCOM_FIELD_CHAIRPASSWORD_SEQ == strKey )
	{
		m_strChairPassword = EncodeHelper::DecodeProprietaryPolycomString(strValue);

//		puts("\n***********Chair password********");
//		puts(strValue.c_str());
//		puts(m_strChairPassword.c_str());
//		puts("********************************");
	}
	else if( CCalendarItem::POLYCOM_FIELD_CHAIRPASSWORDREQUIRED_SEQ == strKey )
	{
		m_bChairPasswordRequired = ("true" == strValue) ? TRUE : FALSE;
	}
	else if( CCalendarItem::POLYCOM_FIELD_RECORDMEETING_SEQ == strKey )
	{
		m_bRecording = ("true" == strValue) ? TRUE : FALSE;
	}
	else if( CCalendarItem::POLYCOM_FIELD_STREAMMEETING_SEQ == strKey )
	{
		m_bStreaming = ("true" == strValue) ? TRUE : FALSE;
	}
	else if( CCalendarItem::POLYCOM_FIELD_LANGUAGE_SEQ == strKey )
	{
		m_strLanguage = strValue;
	}
	else if( CCalendarItem::POLYCOM_FIELD_AUDIONUMBER1_SEQ == strKey )
	{
		m_strAudioNumber1 = strValue;
	}
	else if( CCalendarItem::POLYCOM_FIELD_AUDIONUMBER2_SEQ == strKey )
	{
		m_strAudioNumber2 = strValue;
	}
	else if( CCalendarItem::POLYCOM_FIELD_SIGNALINGPREFIX_SEQ == strKey )
	{
		m_strSignalingPrefix = strValue;
	}
	else if( CCalendarItem::POLYCOM_FIELD_SIGNALINGPOSTFIX_SEQ == strKey )
	{
		m_strSignalingPostfix = strValue;
	}
	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CCalendarItem::IsOccurenceItem() const
{
	std::string prefix("IPM.OLE.CLASS.");
	if (!m_strItemClass.compare(0, prefix.size(), prefix))
		return true;
	if( m_strItemClass == "IPM.Appointment.Occurrence")
		return true;
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CCalendarItemList::CCalendarItemList()
{
//	puts("DEBUG>>> CCalendarItemList::CCalendarItemList");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CCalendarItemList::~CCalendarItemList()
{
//	puts("DEBUG>>> CCalendarItemList::~CCalendarItemList");
	Cleanup();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CCalendarItemList::Dump(std::ostream& stream) const
{
	stream 	<< "\n\n" << "CCalendarItemList" << "\n"
			<< "------------------------------\n";

	int count = 0;
	std::list<CCalendarItem*>::const_iterator iter = m_calendarItemsList.begin();
	while( m_calendarItemsList.end() != iter )
	{
		stream << "---- Item #" << count+1 << " ----\n";
		const CCalendarItem* pItem = ((CCalendarItem*)*iter);
		pItem->Dump(stream);
		iter++;
		count++;
	}
	stream 	<< "------------------------------\n";

}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CCalendarItemList::Cleanup()
{
	std::list<CCalendarItem*>::iterator iter = m_calendarItemsList.begin();

	while( m_calendarItemsList.end() != iter )
	{
		CCalendarItem* pItem = ((CCalendarItem*)*iter);
		POBJDELETE(pItem);
		iter++;
	}
	m_calendarItemsList.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CCalendarItemList& CCalendarItemList::operator= (const CCalendarItemList &other)
{
	if(this != &other)
	{
		Cleanup();

		std::list<CCalendarItem*>::const_iterator iter = other.m_calendarItemsList.begin();

		while( other.m_calendarItemsList.end() != iter )
		{
			CCalendarItem* pItem = new CCalendarItem( *((CCalendarItem*)*iter) );
			AddItem(pItem);
			iter++;
		}
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItemList::AddItem(CCalendarItem* pCalendarItem)
{
	m_calendarItemsList.push_back(pCalendarItem);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const CCalendarItem* CCalendarItemList::FindItemByNumericId(const char* pszNumericId) const
{
	if( NULL == pszNumericId  ||  strlen(pszNumericId) < 1 )
		return NULL;

	std::list<CCalendarItem*>::const_iterator iter = m_calendarItemsList.begin();

	while( m_calendarItemsList.end() != iter )
	{
		const CCalendarItem* pItem = ((CCalendarItem*)*iter);
		if( pItem->GetNumericId() == pszNumericId )
		{
			return pItem;
		}
		iter++;
	}

	return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const CCalendarItem* CCalendarItemList::FindItemByAppointmentId(const char* pszAppointmentId) const
{
	if( NULL == pszAppointmentId  ||  strlen(pszAppointmentId) < 1 )
		return NULL;

	std::list<CCalendarItem*>::const_iterator iter = m_calendarItemsList.begin();

	while( m_calendarItemsList.end() != iter )
	{
		const CCalendarItem* pItem = ((CCalendarItem*)*iter);
		//K.G we have changed the comparison to be by UID per DMA's request
		std::string strItemUID = pItem->GetItemUId();
		std::string strItemApptID = pItem->GetItemId();
		if( strItemUID == pszAppointmentId || strItemApptID == pszAppointmentId )
		{
			return pItem;
		}
		else if (pItem->IsOccurenceItem()) //if it is a reccurence Item Clear 32-40 bits and then compare to Master
		{
			for (int i=32; (strItemUID.length()>40 && i<40) ; i++)
				strItemUID[i] = '0';
			if( strItemUID == pszAppointmentId )
				return pItem;
		}
		iter++;
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItemList::DeSerializeFindItemRequest(const std::string& strResponse,std::string itemType/*t:CalendarItem or INBOX_ITEM_TYPE*/)
{
	STATUS retVal = STATUS_OK;

	Cleanup();

	ParseHelper::EParserStatus status;

		// get FindItemResponseMessage node
	std::string strFindItemResponseMessage;
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strFindItemResponseMessage,strResponse,"m:FindItemResponseMessage")) )
	{
		PTRACE2(eLevelError,"CCalendarItemList::DeSerializeFindItemRequest - <m:FindItemResponseMessage> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
		// check return status in ResponseClass attribute
	std::string strAttrResponseClass;
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeAttributeValue(strFindItemResponseMessage,"ResponseClass",strAttrResponseClass) ) )
	{
		PTRACE2(eLevelError,"CCalendarItemList::DeSerializeFindItemRequest - attribute <ResponseClass> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

		// if status not 'Success', get response code
	if( 0 != strcmp(strAttrResponseClass.c_str(),"Success") )
	{
		PTRACE2(eLevelError,"CCalendarItemList::DeSerializeFindItemRequest - status of ResponseClass is BAD, strAttrResponseClass=",strAttrResponseClass.c_str());
		std::string strCode;
		if( ParseHelper::eStatusOk == (status = ParseHelper::FindXmlNode(strCode,strFindItemResponseMessage,"m:ResponseCode")) )
		{
			PTRACE2(eLevelError,"CCalendarItemList::DeSerializeFindItemRequest - status of ResponseClass is BAD, ResponseCode=",strCode.c_str());
		}
		strCode = "";
		if( ParseHelper::eStatusOk == (status = ParseHelper::FindXmlNode(strCode,strFindItemResponseMessage,"m:MessageText")) )
		{
			PTRACE2(eLevelError,"CCalendarItemList::DeSerializeFindItemRequest - status of ResponseClass is BAD, MessageText=",strCode.c_str());
		}
		return STATUS_FAIL;
	}

		// calendar items
	std::string strCalendarItem;
	int count = 0;
	while( ParseHelper::eStatusOk == (status = ParseHelper::FindXmlNode(strCalendarItem,strFindItemResponseMessage,itemType,count)) )
	{
		CCalendarItem* pCalendarItem = new CCalendarItem();
		if( STATUS_OK == pCalendarItem->SetShort(strCalendarItem,itemType) )
		{
			AddItem(pCalendarItem);
		}
		else
		{
			POBJDELETE(pCalendarItem);
		}
		count++;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItemList::DeSerializeGetItemRequest(const std::string& strResponse,std::string itemType/*t:CalendarItem or INBOX_ITEM_TYPE*/)
{
	STATUS retVal = STATUS_OK;

	ParseHelper::EParserStatus status;

		// get FindItemResponseMessage node
	std::string strGetItemResponseMessage;
	int count = 0;
	while( ParseHelper::eStatusOk == (status = ParseHelper::FindXmlNode(strGetItemResponseMessage,strResponse,"m:GetItemResponseMessage",count)) )
	{
			// check return status in ResponseClass attribute
		std::string strAttrResponseClass;
		if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeAttributeValue(strGetItemResponseMessage,"ResponseClass",strAttrResponseClass) ) )
		{
			PTRACE2(eLevelError,"CCalendarItemList::DeSerializeGetItemRequest - attribute <ResponseClass> is BAD, status=",ParseHelper::StatusAsString(status));
			return STATUS_FAIL;
		}

			// if status not 'Success', get response code
		if( 0 != strcmp(strAttrResponseClass.c_str(),"Success") )
		{
			PTRACE2(eLevelError,"CCalendarItemList::DeSerializeGetItemRequest - status of ResponseClass is BAD, strAttrResponseClass=",strAttrResponseClass.c_str());
			std::string strCode;
			if( ParseHelper::eStatusOk == (status = ParseHelper::FindXmlNode(strCode,strGetItemResponseMessage,"m:ResponseCode")) )
			{
				PTRACE2(eLevelError,"CCalendarItemList::DeSerializeGetItemRequest - status of ResponseClass is BAD, ResponseCode=",strCode.c_str());
			}
			return STATUS_FAIL;
		}

			// calendar items
		std::string strCalendarItem;
		if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strCalendarItem,strGetItemResponseMessage,itemType)) )
		{
			PTRACE2(eLevelError,"CCalendarItemList::DeSerializeGetItemRequest - <t:CalendarItem> is BAD, status=",ParseHelper::StatusAsString(status));
			return STATUS_FAIL;
		}
		CCalendarItem* pCalendarItem = new CCalendarItem();
		if( STATUS_OK == pCalendarItem->SetShort(strCalendarItem,itemType) )
		{
				// find appropriate item in list and update
			std::list<CCalendarItem*>::iterator iter = m_calendarItemsList.begin();
			while( m_calendarItemsList.end() != iter )
			{
				CCalendarItem* pItem = ((CCalendarItem*)*iter);

				if( pItem->GetItemId() == pCalendarItem->GetItemId() )
				{
					// deserialize full appointment data, if failed - remove item from list
					if( STATUS_OK != pItem->SetFull(strCalendarItem,itemType) )
					{
						m_calendarItemsList.remove(pItem);
						POBJDELETE(pItem);
					}
					break;
				}
				iter++;
			}
		}
		POBJDELETE(pCalendarItem);

		count++;
	}


	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItemList::PrepareDifferentItemsMap( const CCalendarItemList& listOther,
			std::map<std::string,std::string>& mapDifferentItems )
{
	STATUS retVal = STATUS_OK;

	mapDifferentItems.clear();

	std::list<CCalendarItem*>::iterator iter1 = m_calendarItemsList.begin();

	while( m_calendarItemsList.end() != iter1 )
	{
		bool bFound = false;
		CCalendarItem* pItem1 = ((CCalendarItem*)*iter1);

		std::list<CCalendarItem*>::const_iterator iter2 = listOther.m_calendarItemsList.begin();
		while( listOther.m_calendarItemsList.end() != iter2 )
		{
			const CCalendarItem* pItem2 = ((CCalendarItem*)*iter2);

			if( pItem1->GetItemId() == pItem2->GetItemId() )
			{
				bFound = true;

				// if in matching item change key is different
				if( pItem1->GetItemChangeKey() != pItem2->GetItemChangeKey() )
					mapDifferentItems[pItem1->GetItemId()] = pItem1->GetItemChangeKey();
				else // copy element from old list to new
					*pItem1 = *pItem2;

				break;
			}

			iter2++;
		}
		// if not found -> add to map
		if( false == bFound )
			mapDifferentItems[pItem1->GetItemId()] = pItem1->GetItemChangeKey();

		iter1++;
	}

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItemList::PrepareItemsMap(std::map<std::string,std::string>& mapDifferentItems )
{
	STATUS retVal = STATUS_OK;

	mapDifferentItems.clear();

	std::list<CCalendarItem*>::iterator iter1 = m_calendarItemsList.begin();

	while( m_calendarItemsList.end() != iter1 )
	{
		bool bFound = false;
		CCalendarItem* pItem1 = ((CCalendarItem*)*iter1);
		mapDifferentItems[pItem1->GetItemId()] = pItem1->GetItemChangeKey();
		iter1++;
	}

	return retVal;
}

DWORD CCalendarItemList::GetSize() const
{
	return m_calendarItemsList.size();
}

CCalendarItem* CCalendarItemList::GetFirstItem()
{
	m_calendarItemListIterator = m_calendarItemsList.begin();

	if(m_calendarItemListIterator != m_calendarItemsList.end())
		return (*m_calendarItemListIterator);

	else
		return NULL;
}

CCalendarItem* CCalendarItemList::GetNextItem()
{
	//make sure not to increase iterator if it's already at end
	if(m_calendarItemListIterator == m_calendarItemsList.end())
		return NULL;

	m_calendarItemListIterator++;

	if(m_calendarItemListIterator != m_calendarItemsList.end())
		return (*m_calendarItemListIterator);

	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCalendarItemList::CheckFindItemResponseStatus(const std::string& strResponse, std::string& strErrCode, std::string& strErrDescription)
{
	ParseHelper::EParserStatus status;

	strErrCode = "";
	strErrDescription = "";

		// get FindItemResponseMessage node
	std::string strFindItemResponseMessage;
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strFindItemResponseMessage,strResponse,"m:FindItemResponseMessage")) )
	{
		FPTRACE2(eLevelError,"CCalendarItemList::CheckFindItemResponseStatus - <m:FindItemResponseMessage> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}
		// check return status in ResponseClass attribute
	std::string strAttrResponseClass;
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeAttributeValue(strFindItemResponseMessage,"ResponseClass",strAttrResponseClass) ) )
	{
		FPTRACE2(eLevelError,"CCalendarItemList::CheckFindItemResponseStatus - attribute <ResponseClass> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

		// if status not 'Success', get response code
	if( 0 != strcmp(strAttrResponseClass.c_str(),"Success") )
	{
		FPTRACE2(eLevelError,"CCalendarItemList::CheckFindItemResponseStatus - status of ResponseClass is BAD, strAttrResponseClass=",strAttrResponseClass.c_str());
		std::string strCode;
		if( ParseHelper::eStatusOk == (status = ParseHelper::FindXmlNode(strCode,strFindItemResponseMessage,"m:ResponseCode")) )
		{
			FPTRACE2(eLevelError,"CCalendarItemList::CheckFindItemResponseStatus - status of ResponseClass is BAD, ResponseCode=",strCode.c_str());
			if( ParseHelper::eStatusOk == (status = ParseHelper::GetXmlNodeValue(strCode,strErrCode) ) )
			{
			}
		}
		strCode = "";
		if( ParseHelper::eStatusOk == (status = ParseHelper::FindXmlNode(strCode,strFindItemResponseMessage,"m:MessageText")) )
		{
			FPTRACE2(eLevelError,"CCalendarItemList::CheckFindItemResponseStatus - status of ResponseClass is BAD, MessageText=",strCode.c_str());
			if( ParseHelper::eStatusOk == (status = ParseHelper::GetXmlNodeValue(strCode,strErrDescription) ) )
			{
			}
		}
		return STATUS_FAIL;
	}
	return STATUS_OK;
}

