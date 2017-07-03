// RtmIsdnMngrCommonMethods.cpp

#include "ObjString.h"
#include "SystemFunctions.h"
#include "RtmIsdnMngrCommonMethods.h"
#include "RtmIsdnMaintenanceStructs.h"
#include "TraceStream.h"

extern const char* SpanTypeToString(eSpanType theType);
extern const char* ServiceTypeToString(eServiceType theType);
extern const char* DfltNumToString(eDfltNumType theType);
extern const char* NumPlanToString(eNumPlanType theType);
extern const char* VoiceTypeToString(eVoiceType theType);
extern const char* FramingTypeToString(eFramingType theType);
extern const char* SideTypeToString(eSideType theType);
extern const char* LineCodingTypeToString(eLineCodingType theType);
extern const char* SwitchTypeToString(eSwitchType theType);

static const struct 
{
	APIU32		code;
	const char*	name;

}	countryNameCodeMap[MAX_NUM_OF_COUNTRY_CODES] = 
	{
		{AUSTRALIA,		"AUSTRALIA"},
		{AUSTRIA,		"AUSTRIA"},
		{BELGIUM,		"BELGIUM"},
		{CANADA,		"CANADA"},
		{CHILE,			"CHILE"},
		{DENMARK,		"DENMARK"},
		{EUROPE,		"EUROPE"},
		{FINLAND,		"FINLAND"},
		{FRANCE,		"FRANCE"},
		{GERMANY,		"GERMANY"},
		{HONG_KONG,		"HONG_KONG"},
		{INDIA,			"INDIA"},
		{IRLANDE,		"IRLANDE"},
		{ITALY,			"ITALY"},
		{JAPAN,			"JAPAN"},
		{KOREA,			"KOREA"},
		{LUXEMBOURG,	"LUXEMBOURG"},
		{NETHERLANDS,	"NETHERLANDS"},
		{NEW_ZEALAND,	"NEW_ZEALAND"},
		{NORWAY,		"NORWAY"},
		{PORTUGAL,		"PORTUGAL"},
		{SOUTH_AFRICA,	"SOUTH_AFRICA"},
		{SPAIN,			"SPAIN"},
		{SWEDEN,		"SWEDEN"},
		{SWITZERLAND,	"SWITZERLAND"},
		{TAIWAN,		"TAIWAN"},
		{UK,			"UK"},
		{USA,			"USA"},
		{USSR,			"USSR"},
		{COUNTRY_NIL,	"COUNTRY_NIL"}
	};


// ======== span types ==========
static const struct 
{
	eSpanType	asEnum;
	APIU32		asApiu32;

}	spanTypesMap[NUM_OF_SPAN_TYPES] = 
	{
		{eSpanTypeT1,	(APIU32)PCM_MODE_T1},
		{eSpanTypeE1,	(APIU32)PCM_MODE_E1},
	};

// ======= framing types ========
static const struct 
{
	eFramingType	asEnum;
	APIU32			asApiu32;

}	framingTypesMap[NUM_OF_FRAMING_TYPES] = 
	{
		{eFramingType_T1_Esf,			(APIU32)T1_FRAMING_ESF_CRC6},
		{eFramingType_T1_Sf,			(APIU32)T1_FRAMING_F12},
		{eFramingType_E1_Crc4_SiFebe,	(APIU32)E1_FRAMING_MFF_CRC4},
		{eFramingType_E1_Basic_NoCrc,	(APIU32)E1_FRAMING_DDF},
		{eFramingType_E1_Crc4_Si1,		(APIU32)T1_FRAMING_F12},  // Tomer (12/2006): not supported!! (return the default)
	};

// ======== side types ==========
static const struct 
{
	eSideType	asEnum;
	APIU32		asApiu32;

}	sideTypesMap[NUM_OF_SIDE_TYPES] = 
	{
		{eSideTypeUser,		(APIU32)FG_NT_NET}, // FG_NT_NET == user, FG_NT_TE == network
		{eSideTypeNetwork,	(APIU32)FG_NT_TE},	//  according to Tomer (31.12.06)
	};

// ===== lineCoding types =======
static const struct 
{
	eLineCodingType	asEnum;
	APIU32			asApiu32;

}	lineCodingTypesMap[NUM_OF_LINE_CODING_TYPES] = 
	{
		{eLineCodingType_T1_B8ZS,	(APIU32)LINE_CODE_B8ZS},
		{eLineCodingType_T1_B7ZS,	(APIU32)LINE_CODE_B8ZS},  // Tomer (12/2006): same as B8ZS
		{eLineCodingType_AMI,		(APIU32)LINE_CODE_AMI},
		{eLineCodingType_E1_Hdb3,	(APIU32)LINE_CODE_HDB3},
	};

// ======= switch types =========
static const struct 
{
	eSwitchType		asEnum;
	APIU32			asApiu32;

}	switchTypesMap[NUM_OF_SWITCH_TYPES] = 
	{
		// All according to Tomer (12/2006)
		{eSwitchType_T1_Att4ess,	(APIU32)ATT_4ESS},
		{eSwitchType_T1_Att5ess10,	(APIU32)ATT_5E10},
		{eSwitchType_T1_Dms100,		(APIU32)NT_DMS100},
		{eSwitchType_T1_Dms250,		(APIU32)NT_DMS250},
		{eSwitchType_T1_Ntt,		(APIU32)NTT},
		{eSwitchType_T1_NI1,		(APIU32)N_ISDN1},
		{eSwitchType_T1_NI2,		(APIU32)N_ISDN2},
		{eSwitchType_E1_EuroIsdn,	(APIU32)ETSI},
	};



// ======= spanAlarm types ======
static const struct 
{
	eSpanAlarmType	asEnum;
	APIU32			asApiu32;

}	spanAlarmTypesMap[NUM_OF_SPAN_ALARM_TYPES] = 
	{
		// All according to Tomer (12/2006)
		{eSpanAlarmTypeNone,	(APIU32)NO_ALARM},
		{eSpanAlarmTypeYellow,	(APIU32)YELLOW_ALARM},
		{eSpanAlarmTypeRed,		(APIU32)RED_ALARM},
	};


// ======= dChannel types =======
static const struct 
{
	eDChannelStateType	asEnum;
	APIU32				asApiu32;

}	dChannelTypesMap[NUM_OF_D_CHANNEL_STATE_TYPES] = 
	{
		// All according to Tomer (12/2006)
		{eDChannelStateTypeEstablished,		(APIU32)D_CHANNEL_ESTABLISHED},
		{eDChannelStateTypeNotEstablished,	(APIU32)D_CHANNEL_NOT_ESTABLISHED},
	};


// ======= clocking types =======
static const struct 
{
	eClockingType	asEnum;
	APIU32			asApiu32;

}	clockingTypesMap[NUM_OF_CLOCKING_TYPES] = 
	{
		// All according to Tomer (12/2006)
		{eClockingTypeNone,		(APIU32)CLOCK_NONE},
		{eClockingTypePrimary,	(APIU32)TO_MASTER},
		{eClockingTypeBackup,	(APIU32)TO_BACKUP},
	};



/////////////////////////////////////////////////////////////////////////////
CRtmIsdnMngrCommonMethods::CRtmIsdnMngrCommonMethods ()
{
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnMngrCommonMethods::CRtmIsdnMngrCommonMethods (const CRtmIsdnMngrCommonMethods &other)
: CPObject()
{
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnMngrCommonMethods::~CRtmIsdnMngrCommonMethods ()
{
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrCommonMethods::PrintRtmIsdnParamsMcmsStruct( const RTM_ISDN_PARAMS_MCMS_S &theStruct,
                                                              const char* theSource)
{
	CLargeString retStr =  "\nPrintRtmIsdnParamsMcmsStruct";
	             retStr << " (from " << theSource << ")"
	                    << "\n============================";       

	// ===== 1. basic service params
	retStr << "\nService Name: "	<< (char*)(theStruct.serviceName)
           << "\nDflt Num:     "	<< ::DfltNumToString( (eDfltNumType)(theStruct.dfltNumType) )
           << "\nNum Plan:     "	<< ::NumPlanToString( (eNumPlanType)(theStruct.numPlan) )
           << "\nVoice:        "	<< ::VoiceTypeToString( (eVoiceType)(theStruct.voice) );

	// ===== 2. span definition
	retStr << "\nSpan Type:    "	<< ::SpanTypeToString( (eSpanType)(theStruct.spanDef.spanType))
           << "\nService Type: "	<< ::ServiceTypeToString( (eServiceType)(theStruct.spanDef.serviceType) )
           << "\nFraming:      "	<< ::FramingTypeToString( (eFramingType)(theStruct.spanDef.framing) )
           << "\nSide:         "	<< ::SideTypeToString( (eSideType)(theStruct.spanDef.side) )
           << "\nLine Coding:  "	<< ::LineCodingTypeToString( (eLineCodingType)(theStruct.spanDef.lineCoding) )
           << "\nSwitch Type:  "	<< ::SwitchTypeToString( (eSwitchType)(theStruct.spanDef.switchType) )
           << "\nnetSpecFacility: " << theStruct.netSpecFacility;


	// ===== 2. phone ranges
	BOOL isAny = FALSE;
	retStr << "\n\nPhone Ranges:"
	       <<   "\n~~~~~~~~~~~~~";

    for ( int i=0; i < MAX_ISDN_PHONE_NUMBER_IN_SERVICE; i++ )
    {
    	if ( 0 != theStruct.phoneRangesList[i].firstPhoneNumber[0] )
    	{
    		isAny = TRUE;

    		retStr << "\nRange " << i << ": "
    	           << (char*)(theStruct.phoneRangesList[i].firstPhoneNumber)
    	           << "-"
    	           << (char*)(theStruct.phoneRangesList[i].lastPhoneNumber);

    	    retStr << "\n   DialIn Group Id: " << theStruct.phoneRangesList[i].dialInGroupId
    	           << ", Category: "           << theStruct.phoneRangesList[i].category
    	           << ", FirstPort Id: "       << theStruct.phoneRangesList[i].firstPortId
    	           << "\n";
    	}
    }
    if (FALSE == isAny)
    {
        retStr << "\n(no phone ranges in list)";
    }

	// ===== 3. ip addresses
	isAny = FALSE;
	retStr << "\n\nIP Addresses:"
	       <<   "\n~~~~~~~~~~~~~";

	char ipAddressRtmStr[IP_ADDRESS_LEN],
	     ipAddressRtmMediaStr[IP_ADDRESS_LEN];

    for ( int i=0; i < MAX_NUM_OF_BOARDS; i++ )
    {
    	DWORD ipAddressRtm		= theStruct.ipAddressesList[i].ipAddress_Rtm,
    	      ipAddressRtmMedia	= theStruct.ipAddressesList[i].ipAddress_RtmMedia;

    	if ( ( (0 != ipAddressRtm      ) && (0xFFFF != ipAddressRtm      ) )  ||
    	     ( (0 != ipAddressRtmMedia ) && (0xFFFF != ipAddressRtmMedia ) )  )
    	{
    		isAny = TRUE;

			SystemDWORDToIpString(ipAddressRtm,      ipAddressRtmStr);
  			SystemDWORDToIpString(ipAddressRtmMedia, ipAddressRtmMediaStr);
    		
    		retStr << "\nBoard Id "				<< theStruct.ipAddressesList[i].boardId;
    	    retStr << "\n  Rtm Address:       "	<< ipAddressRtmStr
    	           << "\n  Rtm_Media Address: "	<< ipAddressRtmMediaStr
    	           << "\n";
    	}
    }
    if (FALSE == isAny)
    {
        retStr << "\n(no addresses in list)\n";
    }
    
    retStr << "\n";
    TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return;    	           
}


/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrCommonMethods::PrintRtmIsdnServiceNameStruct(const RTM_ISDN_SERVICE_NAME_S &theStruct, const char* theSource)
{
	CLargeString retStr =  "\nPrintRtmIsdnServiceNameStruct";
	             retStr << " (from " << theSource << ")"
	                    << "\n=============================";       

	retStr << "\nService Name: " << (char*)(theStruct.serviceName);

    retStr << "\n\n";
    TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return;    	           
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrCommonMethods::PrintRtmIsdnSpanMapStruct( const RTM_ISDN_SPAN_MAP_S &theStruct,
                                                           const char* theSource )
{
	CLargeString retStr =  "\nPrintRtmIsdnSpanMapStruct";
	             retStr << " (from " << theSource << ")"
	                    << "\n=========================";       

	retStr << "\nService Name: " << (char*)(theStruct.serviceName)
           << ", Board Id: "	 << theStruct.boardId
           << ", Span Id: "		 << theStruct.spanId;

    retStr << "\n\n";
    TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return;    	           
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrCommonMethods::PrintRtmIsdnSpansMapsListStruct( const RTM_ISDN_SPAN_MAPS_LIST_S &theStruct,
                                                                 const char* theSource )
{
	CLargeString retStr =  "\nPrintRtmIsdnSpansMapsListStruct";
	             retStr << " (from " << theSource << ")"
	                    << "\n===============================";       

	BOOL isAny = FALSE;

    for ( int i=0; i < MAX_ISDN_SPAN_MAPS_IN_LIST; i++ )
    {
    	if ( 0 != theStruct.spanMap[i].serviceName[0] )
    	{
    		isAny = TRUE;

    		retStr << "\nSpan Map "		<< i << ": "
    		       << "Service Name: "	<< (char*)(theStruct.spanMap[i].serviceName)
    	           << ", Board Id: "	<< theStruct.spanMap[i].boardId
    	           << ", Span Id: "		<< theStruct.spanMap[i].spanId;
    	}
    }
    
    if (FALSE == isAny)
    {
    	retStr << "\n(no span maps in list)";
    }

    retStr << "\n\n";
    TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return;    	           
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrCommonMethods::PrintRtmIsdnSpansDisableStruct( const SPAN_DISABLE_S &theStruct,
                                                                const char* theSource )
{
	CLargeString retStr =  "\nPrintRtmIsdnSpansDisableStruct";
	             retStr << " (from " << theSource << ")"
	                    << "\n==============================";       

	retStr << "\nBoardId: "	<< theStruct.boardId
	       << "\nSpanId:  "	<< theStruct.spanId
	       << "\nStatus:  " << theStruct.status;
 
    retStr << "\n\n";
    TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return;    	           
}




/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrCommonMethods::PrintRtmIsdnPhoneRangeUpdateStruct(const RTM_ISDN_PHONE_RANGE_UPDATE_S &theStruct, const char* theSource)
{
	CLargeString retStr =  "\nPrintRtmIsdnPhoneRangeUpdateStruct";
	             retStr << " (from " << theSource << ")"
	                    << "\n==================================";       

	retStr << "\nService Name: " << (char*)(theStruct.serviceName);

	retStr << "\nRange: "
           << (char*)(theStruct.phoneRange.firstPhoneNumber)
           << "-"
           << (char*)(theStruct.phoneRange.lastPhoneNumber);

    retStr << "\nDialIn Group Id: " << theStruct.phoneRange.dialInGroupId
           << ", Category:        " << theStruct.phoneRange.category
           << ", FirstPort Id:    " << theStruct.phoneRange.firstPortId;
   
	retStr << "\nStatus: " << theStruct.status
           << "\n\n";

    TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return;    	           
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrCommonMethods::PrintRtmIsdnServiceCancelStruct(const RTM_ISDN_SERVICE_CANCEL_S &theStruct, const char* theSource)
{
	CLargeString retStr =  "\nPrintRtmIsdnServiceCancelStruct";
	             retStr << " (from " << theSource << ")"
	                    << "\n===============================";       

	retStr << "\nService Name: " << (char*)(theStruct.serviceName)
	       << "\nStatus:       " << theStruct.status;

    retStr << "\n\n";
    TRACESTR(eLevelInfoNormal) << retStr.GetString();

	return;    	           
}

/////////////////////////////////////////////////////////////////////////////
APIU32 CRtmIsdnMngrCommonMethods::CountryCodeToAPIU32(string sCode)
{
	APIU32 dCode = COUNTRY_NIL;

	for (int i = 0; i < MAX_NUM_OF_COUNTRY_CODES; i++)
	{
		if (!strcmp(sCode.c_str(), countryNameCodeMap[i].name))
		{
			dCode = countryNameCodeMap[i].code;
			break;
		}
	}

	return dCode;
}

/////////////////////////////////////////////////////////////////////////////
string CRtmIsdnMngrCommonMethods::CountryCodeToString(APIU32 dCode)
{
	string sCode = "COUNTRY_NIL";

	for (int i=0; i<MAX_NUM_OF_COUNTRY_CODES; i++)
	{
		if (dCode == countryNameCodeMap[i].code)
		{
			sCode = countryNameCodeMap[i].name;
			break;
		}
	}

	return sCode;
}

std::string CRtmIsdnMngrCommonMethods::CountryIndexToString(int index)
{
	if (index < MAX_NUM_OF_COUNTRY_CODES)
		return countryNameCodeMap[index].name;

	return "COUNTRY_NIL";
}

APIU32 CRtmIsdnMngrCommonMethods::SpanTypeToApiu32(eSpanType eType)
{
	APIU32 dType = (APIU32)PCM_MODE_T1;

	for (int i=0; i<NUM_OF_SPAN_TYPES; i++)
	{
		if (eType == spanTypesMap[i].asEnum)
		{
			dType = spanTypesMap[i].asApiu32;
			break;
		}
	}

	return dType;
}

/////////////////////////////////////////////////////////////////////////////
eSpanType CRtmIsdnMngrCommonMethods::SpanTypeToEnum(APIU32 dType)
{
	eSpanType eType = eSpanTypeT1;

	for (int i=0; i<NUM_OF_SPAN_TYPES; i++)
	{
		if (dType == spanTypesMap[i].asApiu32)
		{
			eType = spanTypesMap[i].asEnum;
			break;
		}
	}

	return eType;
}

/////////////////////////////////////////////////////////////////////////////
APIU32 CRtmIsdnMngrCommonMethods::FramingTypeToApiu32(eFramingType eType)
{
	APIU32 dType = (APIU32)T1_FRAMING_F12;

	for (int i=0; i<NUM_OF_FRAMING_TYPES; i++)
	{
		if (eType == framingTypesMap[i].asEnum)
		{
			dType = framingTypesMap[i].asApiu32;
			break;
		}
	}

	return dType;
}

/////////////////////////////////////////////////////////////////////////////
eFramingType CRtmIsdnMngrCommonMethods::FramingTypeToEnum(APIU32 dType)
{
	eFramingType eType = eFramingType_T1_Sf;

	for (int i=0; i<NUM_OF_FRAMING_TYPES; i++)
	{
		if (dType == framingTypesMap[i].asApiu32)
		{
			eType = framingTypesMap[i].asEnum;
			break;
		}
	}

	return eType;
}

/////////////////////////////////////////////////////////////////////////////
APIU32 CRtmIsdnMngrCommonMethods::SideTypeToApiu32(eSideType eType)
{
	APIU32 dType = (APIU32)FG_NT_NET;	// FG_NT_NET == user (according to Tomer, 31.12.06)

	for (int i=0; i<NUM_OF_SIDE_TYPES; i++)
	{
		if (eType == sideTypesMap[i].asEnum)
		{
			dType = sideTypesMap[i].asApiu32;
			break;
		}
	}

	return dType;
}

/////////////////////////////////////////////////////////////////////////////
eSideType CRtmIsdnMngrCommonMethods::SideTypeToEnum(APIU32 dType)
{
	eSideType eType = eSideTypeUser;

	for (int i=0; i<NUM_OF_SIDE_TYPES; i++)
	{
		if (dType == sideTypesMap[i].asApiu32)
		{
			eType = sideTypesMap[i].asEnum;
			break;
		}
	}

	return eType;
}

/////////////////////////////////////////////////////////////////////////////
APIU32 CRtmIsdnMngrCommonMethods::LineCodingTypeToApiu32(eLineCodingType eType)
{
	APIU32 dType = (APIU32)LINE_CODE_B8ZS;

	for (int i=0; i<NUM_OF_LINE_CODING_TYPES; i++)
	{
		if (eType == lineCodingTypesMap[i].asEnum)
		{
			dType = lineCodingTypesMap[i].asApiu32;
			break;
		}
	}

	return dType;
}

/////////////////////////////////////////////////////////////////////////////
eLineCodingType CRtmIsdnMngrCommonMethods::LineCodingTypeToEnum(APIU32 dType)
{
	eLineCodingType eType = eLineCodingType_T1_B8ZS;

	for (int i=0; i<NUM_OF_LINE_CODING_TYPES; i++)
	{
		if (dType == lineCodingTypesMap[i].asApiu32)
		{
			eType = lineCodingTypesMap[i].asEnum;
			break;
		}
	}

	return eType;
}

/////////////////////////////////////////////////////////////////////////////
APIU32 CRtmIsdnMngrCommonMethods::SwitchTypeToApiu32(eSwitchType eType)
{
	APIU32 dType = (APIU32)ATT_4ESS;

	for (int i=0; i<NUM_OF_SWITCH_TYPES; i++)
	{
		if (eType == switchTypesMap[i].asEnum)
		{
			dType = switchTypesMap[i].asApiu32;
			break;
		}
	}

	return dType;
}

/////////////////////////////////////////////////////////////////////////////
eSwitchType CRtmIsdnMngrCommonMethods::SwitchTypeToEnum(APIU32 dType)
{
	eSwitchType eType = eSwitchType_T1_Att4ess;

	for (int i=0; i<NUM_OF_SWITCH_TYPES; i++)
	{
		if (dType == switchTypesMap[i].asApiu32)
		{
			eType = switchTypesMap[i].asEnum;
			break;
		}
	}

	return eType;
}

/////////////////////////////////////////////////////////////////////////////
APIU32 CRtmIsdnMngrCommonMethods::SpanAlarmTypeToApiu32(eSpanAlarmType eType)
{
	APIU32 dType = (APIU32)NO_ALARM;

	for (int i=0; i<NUM_OF_SPAN_ALARM_TYPES; i++)
	{
		if (eType == spanAlarmTypesMap[i].asEnum)
		{
			dType = spanAlarmTypesMap[i].asApiu32;
			break;
		}
	}

	return dType;
}

/////////////////////////////////////////////////////////////////////////////
eSpanAlarmType CRtmIsdnMngrCommonMethods::SpanAlarmTypeToEnum(APIU32 dType)
{
	eSpanAlarmType eType = eSpanAlarmTypeNone;

	for (int i=0; i<NUM_OF_SPAN_ALARM_TYPES; i++)
	{
		if (dType == spanAlarmTypesMap[i].asApiu32)
		{
			eType = spanAlarmTypesMap[i].asEnum;
			break;
		}
	}

	return eType;
}

/////////////////////////////////////////////////////////////////////////////
APIU32 CRtmIsdnMngrCommonMethods::DChannelTypeToApiu32(eDChannelStateType eType)
{
	APIU32 dType = (APIU32)D_CHANNEL_ESTABLISHED;

	for (int i=0; i<NUM_OF_D_CHANNEL_STATE_TYPES; i++)
	{
		if (eType == dChannelTypesMap[i].asEnum)
		{
			dType = dChannelTypesMap[i].asApiu32;
			break;
		}
	}

	return dType;
}

/////////////////////////////////////////////////////////////////////////////
eDChannelStateType CRtmIsdnMngrCommonMethods::DChannelTypeToEnum(APIU32 dType)
{
	eDChannelStateType eType = eDChannelStateTypeEstablished;

	for (int i=0; i<NUM_OF_D_CHANNEL_STATE_TYPES; i++)
	{
		if (dType == dChannelTypesMap[i].asApiu32)
		{
			eType = dChannelTypesMap[i].asEnum;
			break;
		}
	}

	return eType;
}

/////////////////////////////////////////////////////////////////////////////
APIU32 CRtmIsdnMngrCommonMethods::ClockingTypeToApiu32(eClockingType eType)
{
	APIU32 dType = (APIU32)CLOCK_NONE;

	for (int i=0; i<NUM_OF_CLOCKING_TYPES; i++)
	{
		if (eType == clockingTypesMap[i].asEnum)
		{
			dType = clockingTypesMap[i].asApiu32;
			break;
		}
	}

	return dType;
}

/////////////////////////////////////////////////////////////////////////////
eClockingType CRtmIsdnMngrCommonMethods::ClockingTypeToEnum(APIU32 dType)
{
	eClockingType eType = eClockingTypeNone;

	for (int i=0; i<NUM_OF_CLOCKING_TYPES; i++)
	{
		if (dType == clockingTypesMap[i].asApiu32)
		{
			eType = clockingTypesMap[i].asEnum;
			break;
		}
	}

	return eType;
}
