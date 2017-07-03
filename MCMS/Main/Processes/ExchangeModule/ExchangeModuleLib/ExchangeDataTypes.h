//============================================================================
// Name        : ExchangeDataTypes.h
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2009
// Description : Data types for Exchange negotiation
//============================================================================


#ifndef __ExchangeDataTypes_h__
#define __ExchangeDataTypes_h__


#include "PObject.h"
#include "StructTm.h"


class CCalendarItem : public CPObject
{
CLASS_TYPE_1(CalendarItem, CPObject)
public:
		// Constructors
	CCalendarItem();
	CCalendarItem(const CCalendarItem& other);
	virtual ~CCalendarItem();
	CCalendarItem& operator= (const CCalendarItem &other);

		// Overrides
	virtual const char*  NameOf() const { return "CCalendarItem"; }
	virtual void Dump(std::ostream& stream) const;

	STATUS SetFull(const std::string& strFullData,std::string itemType/*CALENDAR_ITEM_TYPE or INBOX_ITEM_TYPE*/); // parse and update all fields
	STATUS SetShort(const std::string& strFullData,std::string itemType/*CALENDAR_ITEM_TYPE or INBOX_ITEM_TYPE*/); // parse and update only ItemId and ItemChangeKey fields

	const std::string GetItemId() const { return m_strItemId; }
	const std::string GetItemUId() const { return m_strItemUId; }
	const std::string GetItemClass() const { return m_strItemClass; }
	const std::string GetItemChangeKey() const { return m_strItemChangeKey; }
	const std::string GetNumericId() const { return m_strNumericId; }
	const std::string GetItemMyResponseType() const { return m_strMyResponseType; }
	const std::string GetAppointmentSubject() const { return m_strAppointmentSubject; }
	const std::string GetOrganizer() const { return m_strOrganizer; }
	const CStructTm GetDuration() const { return m_tmDuration; }
	BYTE GetRecording() const { return m_bRecording; }
	const std::string GetMeetingPassword() const { return m_strMeetingPassword; }
	const std::string GetChairPassword() const { return m_strChairPassword; }
	BOOL  GetIsStreaming() const { return m_bStreaming; }
	const std::string GetAudioNumber1() const { return m_strAudioNumber1; }
	const std::string GetAudioNumber2() const { return m_strAudioNumber2; }
	const std::string GetDialInPrefix() const { return m_strDialInPrefix; }
	const std::string GetLanguage() const { return m_strLanguage; }
	const CStructTm GetExchangeConfStartTime()const { return m_dtExchangeConfStartTime; }
	BOOL GetIsChairPasswordRequired() const {  return m_bChairPasswordRequired; }
	BOOL IsOccurenceItem() const;

protected:
	STATUS Set(const std::string& strFullData, const BOOL isFull,std::string itemType/*CALENDAR_ITEM_TYPE or INBOX_ITEM_TYPE*/); // parse and update fields

	STATUS GetPolycomSpecificFields(const std::string& strCalendarItem, const std::string& strFieldName,
			std::string& strFieldValue ) const;

	STATUS DeserializePolycomFields(const std::string& strCalendarItem );
	STATUS SetPolycomField(const std::string& strKey, const std::string& strValue );

public:
	const static std::string CALENDAR_ITEM_TYPE;
	const static std::string INBOX_ITEM_TYPE;
	const static std::string INBOX_MEETING_REQUEST_TYPE;

protected:

		// fields of Appointment
	std::string m_strItemId;
	std::string m_strItemUId;
	std::string m_strItemClass;
	std::string m_strItemChangeKey;
	std::string m_strMyResponseType;
	std::string m_strAppointmentSubject;
	std::string m_strOrganizer;
	CStructTm m_tmDuration;
	std::string m_strFullData;

		// Polycom extended fields
	std::string m_strDialInPrefix;
	std::string m_strNumericId;
	std::string m_strMeetingPassword;
	std::string m_strChairPassword;
	BOOL m_bChairPasswordRequired;
	BOOL m_bRecording;
	BOOL m_bStreaming;
	std::string m_strLanguage;
	std::string m_strAudioNumber1;
	std::string m_strAudioNumber2;
	std::string m_strSignalingPrefix;
	std::string m_strSignalingPostfix;

	CStructTm m_dtExchangeConfStartTime;

	const static std::string POLYCOM_BODY_HEADER;
	const static std::string POLYCOM_BODY_FOOTER;

	const static std::string POLYCOM_FIELD_DIALINPREFIX_SEQ;
	const static std::string POLYCOM_FIELD_NUMERIC_ID_SEQ;
	const static std::string POLYCOM_FIELD_MEETINGPASSWORD_SEQ;
	const static std::string POLYCOM_FIELD_CHAIRPASSWORD_SEQ;
	const static std::string POLYCOM_FIELD_CHAIRPASSWORDREQUIRED_SEQ;
	const static std::string POLYCOM_FIELD_RECORDMEETING_SEQ;
	const static std::string POLYCOM_FIELD_STREAMMEETING_SEQ;
	const static std::string POLYCOM_FIELD_LANGUAGE_SEQ;
	const static std::string POLYCOM_FIELD_AUDIONUMBER1_SEQ;
	const static std::string POLYCOM_FIELD_AUDIONUMBER2_SEQ;
	const static std::string POLYCOM_FIELD_SIGNALINGPREFIX_SEQ;
	const static std::string POLYCOM_FIELD_SIGNALINGPOSTFIX_SEQ;

};

class CCalendarItemList : public CPObject
{
	CLASS_TYPE_1(CalendarItemList, CPObject)
public:
		// Constructors
	CCalendarItemList();
	virtual ~CCalendarItemList();

		// Overrides
	virtual const char*  NameOf() const { return "CCalendarItemList"; }
	virtual void Dump(std::ostream& stream) const;

	CCalendarItemList& operator= (const CCalendarItemList& other);
	const CCalendarItem* FindItemByNumericId(const char* pszNumericId) const;
	const CCalendarItem* FindItemByAppointmentId(const char* pszAppointmentId) const;

	STATUS DeSerializeFindItemRequest(const std::string& strResponse,std::string itemType/*CALENDAR_ITEM_TYPE or INBOX_ITEM_TYPE*/);
	STATUS DeSerializeGetItemRequest(const std::string& strResponse,std::string itemType/*CALENDAR_ITEM_TYPE or INBOX_ITEM_TYPE*/);

	STATUS PrepareDifferentItemsMap(const CCalendarItemList& listOther, std::map<std::string,std::string>& mapDifferentItems);
	STATUS PrepareItemsMap( std::map<std::string,std::string>& mapDifferentItems);
	DWORD GetSize() const;
	CCalendarItem* GetFirstItem();
	CCalendarItem* GetNextItem();

	static STATUS CheckFindItemResponseStatus(const std::string& strResponse, std::string& strErrCode, std::string& strErrDescription);

protected:
	void Cleanup();
	STATUS AddItem(CCalendarItem* pCalendarItem);
//	STATUS RemoveItem(CCalendarItem* pCalendarItem);

protected:
	std::list<CCalendarItem*> m_calendarItemsList;
	std::list<CCalendarItem*>::const_iterator m_calendarItemListIterator;
};


#endif /* __ExchangeDataTypes_h__ */
