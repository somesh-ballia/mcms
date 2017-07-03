#ifndef RTMISDNMNGRINTERNALDEFINES_H_
#define RTMISDNMNGRINTERNALDEFINES_H_



#define MAX_ISDN_SERVICES_IN_LIST			2
#define MAX_NUM_OF_RTM_ISDN_BOARDS_RMX2000	2
#define MAX_NUM_OF_RTM_ISDN_BOARDS_RMX4000	4
#define MAX_NUM_OF_RTM_ISDN_BOARDS_RMX1500	1
#define MAX_NUM_OF_RTM_ISDN_BOARDS_RMX1800	1
#define MAX_ISDN_SPAN_MAPS_IN_BOARD			12
#define MAX_ISDN_SPAN_MAPS_IN_BOARD_RMX1500			4
#define MAX_ISDN_SPAN_MAPS_IN_BOARD_RMX1800			4
#define MAX_ISDN_SPAN_MAPS_IN_LIST			MAX_ISDN_SPAN_MAPS_IN_BOARD*MAX_NUM_OF_RTM_ISDN_BOARDS_RMX4000
#define MAX_ISDN_PHONE_NUMBER_IN_SERVICE	30
#define MAX_NUMBERS_IN_PHONE_RANGE			1000


#define NUM_OF_IP_ADDRESSES_PER_BOARD		2
#define RTM_ISDN_VLAN_ID					2099
#define RTM_ISDN_SUBNET_MASK				"255.255.240.0"

#define RTM_ISDN_MEDIA_IP_BOARD_1			"169.254.224.67"
#define RTM_ISDN_MEDIA_IP_BOARD_2			"169.254.224.68"
#define RTM_ISDN_MEDIA_IP_BOARD_3			"169.254.224.69"
#define RTM_ISDN_MEDIA_IP_BOARD_4			"169.254.224.70"

#define RTM_ISDN_RTM_IP_BOARD_1				"169.254.224.167"
#define RTM_ISDN_RTM_IP_BOARD_2				"169.254.224.168"
#define RTM_ISDN_RTM_IP_BOARD_3				"169.254.224.169"
#define RTM_ISDN_RTM_IP_BOARD_4				"169.254.224.170"

// ============================
// ======= enumerators ========
// ============================

// ----- spanEnable
enum eRtmIsdnSpanEnableType
{
	eRtmIsdnSpanEnabled			= 0,
	eRtmIsdnSpanDisabled		= 1,
	
	NUM_OF_SPAN_ENABLED_TYPES	= 2
};
static const char *spanEnableTypeStr[] = 
{
	"Enabled",
	"Disabled"
};


// ----- eSpanType
enum eSpanType
{
	eSpanTypeT1			= 0,
	eSpanTypeE1			= 1,
	
	NUM_OF_SPAN_TYPES	= 2
};//
static const char *spanTypeStr[] = 
{
	"T1",
	"E1"
};


// ----- eSystemSpanType
enum eSystemSpanType
{
	eSystemSpanType_T1			= 0,
	eSystemSpanType_E1			= 1,
	eSystemSpanType_Undefined	= 2,

	NUM_OF_SYSTEM_SPAN_TYPES	= 3
};
static const char *systemsSpanTypeStr[] = 
{
	"T1",
	"E1",
	"Undefined"
};


// ----- eServiceType
enum eServiceType
{
	eServiceTypePri			= 0,
	
	NUM_OF_SERVICE_TYPES	= 1	 // only PRI: leased line not supported
};
static const char *serviceTypeStr[] = 
{
	"PRI"
};


// ----- eDefNumType
enum eDfltNumType
{
	eDfltNumTypeUnknown			= 0,
	eDfltNumTypeInternational	= 1,
	eDfltNumTypeNational		= 2,
	eDfltNumTypeSubscriber		= 3,
	eDfltNumTypeAbbreviated		= 4,
	
	NUM_OF_DFLT_NUM_TYPES		= 5
};
static const char *dfltNumTypeStr[] = 
{
	"Unknown",
	"International",
	"National",
	"Subscriber",
	"Abbreviated"
};


// ----- eNumPlanType
enum eNumPlanType
{
	eNumPlanTypeUnknown		= 0,
	eNumPlanTypeIsdn		= 1,
	eNumPlanTypeNational	= 2,
	eNumPlanTypePrivate		= 3,
	
	NUM_OF_NUM_PLAN_TYPES	= 4
};
static const char *numPlanTypeStr[] = 
{
	"Unknown",
	"Isdn",
	"National",
	"Private"
};


// ----- eVoiceType
enum eVoiceType
{
	eVoiceTypeSpeech	= 0,
	
	NUM_OF_VOICE_TYPES	= 1
};
static const char *voiceTypeStr[] = 
{
	"Speech"
};


// ----- eNetSpecFacilityType
enum eNetSpecFacilityType
{
	eNetSpecFacilityTypeNull					= 0,
	eNetSpecFacilityTypeAttSdn					= 1,
	eNetSpecFacilityTypeAttMegacom800			= 2,
	eNetSpecFacilityTypeAttMegacom				= 3,
	eNetSpecFacilityTypeNtiFx					= 4,
	eNetSpecFacilityTypeNtiTieTrunk				= 5,
	eNetSpecFacilityTypeAttAccunet				= 6,
	NetSpecFacilityTypeNtiPrivate				= 7,
	eNetSpecFacilityTypeAtt1800					= 8,
	eNetSpecFacilityTypeNtiInwats				= 9,
	eNetSpecFacilityTypeVal						= 10,
	eNetSpecFacilityTypeNtiOutwats				= 11,
	eNetSpecFacilityTypeNtiTro					= 12,
	eNetSpecFacilityTypeNtiPrivate              = 13,
	eNetSpecFacilityTypeAttMultiquest			= 16,
	
	NUM_OF_NET_SPEC_FACILITY_TYPES				= 17
};
static const char *facilityTypeStr[] = 
{
	"Null",
	"Att_Sdn",
	"Att_Megacom800",
	"Att_Megacom",
	"Nti_Fx",
	"Nti_Tie_Trunk",
	"Att_Accunet",
	"Nti_Private",
	"Att_1800",	
	"Nti_Inwats",
	"Val",
	"Nti_Outwats",
	"Nti_Tro",
	"Nti_Private",
	"Null"
	"Null"
	"Att_Multiquest"
};


// ----- eFramingType
enum eFramingType
{
	eFramingType_T1_Esf			= 0,
	eFramingType_T1_Sf			= 1,
	eFramingType_E1_Crc4_SiFebe	= 2,
	eFramingType_E1_Basic_NoCrc	= 3,
	eFramingType_E1_Crc4_Si1	= 4,		// Tomer (12/2006): not supported!!

	NUM_OF_FRAMING_TYPES		= 5
};
static const char *framingTypeStr[] = 
{
	"T1_Esf",
	"T1_Sf",
	"E1_Crc4_SiFebe",
	"E1_Crc4_Si1",
	"E1_Basic_NoCrc"
};


// ----- eSideType
enum eSideType
{
	eSideTypeUser		= 0,
	eSideTypeNetwork	= 1,
	
	NUM_OF_SIDE_TYPES	= 2
};
static const char *sideTypeStr[] = 
{
	"User",
	"Network"
};


// ----- eLineCodingType
enum eLineCodingType
{
	eLineCodingType_T1_B8ZS		= 0,
	eLineCodingType_T1_B7ZS		= 1,
	eLineCodingType_AMI			= 2,
	eLineCodingType_E1_Hdb3		= 3,
	
	NUM_OF_LINE_CODING_TYPES	= 4
};
static const char *lineCodingTypeStr[] = 
{
	"T1_B8ZS",
	"T1_B7ZS",
	"AMI",
	"E1_Hdb3"
};


// ----- eSwitchType
enum eSwitchType
{
	eSwitchType_T1_Att4ess		= 0,
	eSwitchType_T1_Att5ess10	= 1,
	eSwitchType_T1_Dms100		= 2,
	eSwitchType_T1_Dms250		= 3,
	eSwitchType_T1_Ntt			= 4,
	eSwitchType_T1_NI1			= 5,
	eSwitchType_T1_NI2			= 6,
	eSwitchType_E1_EuroIsdn		= 7,
	
	NUM_OF_SWITCH_TYPES			= 8
};
static const char *switchTypeStr[] = 
{
	"T1_Att4ess",
	"T1_Att5ess10",
	"T1_Dms100",
	"T1_Dms250",
	"T1_Ntt",
	"T1_NI1",
	"T1_NI2",
	"E1_EuroIsdn"
};



#define RTM_ISDN_SERVICE_LIST_TMP_PATH	"Cfg/RtmIsdnServiceListTmp.xml"
#define RTM_ISDN_SERVICE_LIST_PATH		"Cfg/RtmIsdnServiceList.xml"
#define RTM_ISDN_SPAN_MAP_LIST_PATH		"Cfg/RtmIsdnSpanMapList.xml"

//for rtm_isdn_pri9 and rtm_isdn_pri9_10g  pri 10-12 are disabled
#define RTM_ISDN_PRI_10        10
#define RTM_ISDN_PRI_11        11
#define RTM_ISDN_PRI_12        12


#define RTM_ISDN_9PRI          9
#define RTM_ISDN_12PRI          12




//////////////////////////////
//       ENUMERATORS
//////////////////////////////

// =================================
// ========= eSpanAlarmType ========
// =================================
enum eSpanAlarmType
{
	eSpanAlarmTypeNone			= 0,
	eSpanAlarmTypeYellow		= 1,
	eSpanAlarmTypeRed			= 2,
	
	NUM_OF_SPAN_ALARM_TYPES		= 3
};
static const char *spanAlarmTypeStr[] = 
{
	"No_Alarm",
	"Yellow_Alarm",
	"Red_Alarm"
};


// =================================
// ======= eDChannelStateType ======
// =================================
enum eDChannelStateType
{
	eDChannelStateTypeEstablished		= 0,
	eDChannelStateTypeNotEstablished	= 1,
	
	NUM_OF_D_CHANNEL_STATE_TYPES		= 2
};
static const char *dChannelStateTypeStr[] = 
{
	"Established",
	"Not_Established"
};

// =================================
// ========== eClockingType ========
// =================================
enum eClockingType
{
	eClockingTypeNone		= 0,
	eClockingTypePrimary	= 1,
	eClockingTypeBackup		= 2,
	
	NUM_OF_CLOCKING_TYPES	= 3
};
static const char *clockingTypeStr[] = 
{
	"None",
	"Primary",
	"Backup"
};




#endif /*RTMISDNMNGRINTERNALDEFINES_H_*/
