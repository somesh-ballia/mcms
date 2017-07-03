#ifndef RTMISDNMNGRCOMMONMETHODS_H_
#define RTMISDNMNGRCOMMONMETHODS_H_

/////////////////////////////////
// RtmIsdnMngrCommonMethods.h
////////////////////////////////

#include "RtmIsdnMngrInternalStructs.h"
#include "RtmIsdnMngrInternalDefines.h"

using namespace std;

// ======= country codes ========
#define MAX_NUM_OF_COUNTRY_CODES	30

class CRtmIsdnMngrCommonMethods : public CPObject
{
CLASS_TYPE_1(CRtmIsdnMngrCommonMethods, CPObject)	

public:
	
	CRtmIsdnMngrCommonMethods ();
	CRtmIsdnMngrCommonMethods (const CRtmIsdnMngrCommonMethods &other);
	virtual ~CRtmIsdnMngrCommonMethods ();
	virtual const char* NameOf() const { return "CRtmIsdnMngrCommonMethods";}

	void PrintRtmIsdnParamsMcmsStruct(const RTM_ISDN_PARAMS_MCMS_S &theStruct, const char* theSource);
	void PrintRtmIsdnServiceNameStruct(const RTM_ISDN_SERVICE_NAME_S &theStruct, const char* theSource);
	void PrintRtmIsdnSpanMapStruct(const RTM_ISDN_SPAN_MAP_S &theStruct, const char* theSource);
	void PrintRtmIsdnSpansMapsListStruct(const RTM_ISDN_SPAN_MAPS_LIST_S &theStruct, const char* theSource);
	void PrintRtmIsdnSpansDisableStruct(const SPAN_DISABLE_S &theStruct, const char* theSource);
	void PrintRtmIsdnPhoneRangeUpdateStruct(const RTM_ISDN_PHONE_RANGE_UPDATE_S &theStruct, const char* theSource);
	void PrintRtmIsdnServiceCancelStruct(const RTM_ISDN_SERVICE_CANCEL_S &theStruct, const char* theSource);
	
	APIU32				CountryCodeToAPIU32(string sCode);
	string				CountryCodeToString(APIU32 dCode);
	string				CountryIndexToString(int index);

	
	APIU32				SpanTypeToApiu32(eSpanType eType);
	eSpanType			SpanTypeToEnum(APIU32 dType);
	
	APIU32				FramingTypeToApiu32(eFramingType eType);
	eFramingType		FramingTypeToEnum(APIU32 dType);
	
	APIU32				SideTypeToApiu32(eSideType eType);
	eSideType			SideTypeToEnum(APIU32 dType);
	
	APIU32				LineCodingTypeToApiu32(eLineCodingType eType);
	eLineCodingType		LineCodingTypeToEnum(APIU32 dType);
	
	APIU32				SwitchTypeToApiu32(eSwitchType eType);
	eSwitchType			SwitchTypeToEnum(APIU32 dType);
	
	APIU32				SpanAlarmTypeToApiu32(eSpanAlarmType eType);
	eSpanAlarmType		SpanAlarmTypeToEnum(APIU32 dType);
	
	APIU32				DChannelTypeToApiu32(eDChannelStateType eType);
	eDChannelStateType	DChannelTypeToEnum(APIU32 dType);
	
	APIU32				ClockingTypeToApiu32(eClockingType eType);
	eClockingType		ClockingTypeToEnum(APIU32 dType);
};




#endif /*RTMISDNMNGRCOMMONMETHODS_H_*/
