#if !defined(_CONFPARTYAPIDEFINES_H__)
#define _CONFPARTYAPIDEFINES_H__

#include <ostream>
#include "DataTypes.h"
#include "ApiStatuses.h"
#include "SharedDefines.h"

extern DWORD GetMaxRecordingLinks();

// constants for reservations
#ifndef AUTO
  #define AUTO                                       255
#endif // ifndef AUTO

// Amos Capacity
#define MAX_PARTIES_IN_CONF                          800 // In order to define the array size, don't forget to update when updateing the value per system mode(the max)
#define MAX_PARTIES_IN_CONF_MPM_BASED                800    // temp - no MPM_BASED
#define MAX_PARTIES_IN_CONF_MPM_PLUS_BASED           800
#define MAX_PARTIES_IN_CONF_MPMX_BASED               720
#define MAX_PARTIES_IN_CONF_SOFT_MCU_MFW_BASED       2000 //Same as Rsrc capacity in MFW

#define MAX_PARTIES_IN_CONF_2C_VSW28                 28
#define MAX_PARTIES_IN_CONF_2C_VSW56                 56

#define MAX_VIDEO_PARTIES_IN_CONF_MPM_BASED          160
#define MAX_VIDEO_PARTIES_IN_CONF_MPM_PLUS_BASED     160
#define MAX_VIDEO_PARTIES_IN_CONF_MPMX_BASED         180
#define MAX_VIDEO_PARTIES_IN_CONF_MPMRX_BASED        200
#define MAX_VIDEO_PARTIES_IN_COP_CONF_MPM_PLUS_BASED 120  // 60*4
#define MAX_VIDEO_PARTIES_IN_COP_CONF_MPMX_BASED     360  // 360 (4 cards*90)
#define MAX_VIDEO_PARTIES_IN_CONF_SOFT_MCU_MFW_BASED 1000 //Same as Rsrc capacity in MFW
#define MAX_VIDEO_PARTIES_IN_CONF_NINJA              300

// Note: If this flag is to be changed, do not forget to change also the same flag at subsys/psosdrv/ppp/ppp_conf.c!!
#define MAX_OPERATORS_IN_MCU                         100
#define MAX_CHANNEL_NUMBER                           30
#define MAX_CELL_NUMBER                              16
#define MAX_VIDEO_LAYOUT_NUMBER                      48
#define MAX_MRC_MCU_ID                               1024 	// MCU ID = 10 bit

// An extension that is transferred from a dialOut party via TCS-4 to a gateway as a remote's alias.
#define PARTY_EXTENSION_LENGTH                       256    // The length - 256 characters - is according to the standard
#define PHONE_NUMBER_DIGITS_LEN                      31     // 40 characters + terminator
#define AV_SERVICE_NAME                              81     // 20 characters + terminator
#define MAX_FULL_PATH_LEN                            256
#define CONF_REMARKS_LEN                             301    // 300 characters + terminator
#define NUM_REMARKS_HISTORY                          10
#define USER_IDENTIFIER_STRING_LEN                   61

// Conference setup types
#define DIAL_OUT                                     0
#define MEET_ME_PER_MCU                              1
#define MEET_ME_PER_CONFERENCE                       2
#define MEET_ME_PER_USER                             3
#define MEET_ME_PER_CHANNEL                          4
#define DIAL_IN                                      5
#define DIRECT                                       6

#define OPERATOR_TERMINATE                           100
#define OPERATOR_ADD_PARTY                           101
#define OPERATORS_DELETE_PARTY                       102
#define OPERATOR_DISCONNECTE_PARTY                   103
#define OPERATOR_RECONNECT_PARTY                     104
#define OPERATOR_UPDATE_PARTY                        105
#define OPERATOR_SET_END_TIME                        106

// conference serialize/deserialize header types, CONFERENCE object type req with apiNum > 87
#define CONF_NOT_CHANGED                             1
#define CONF_COMPLETE_INFO                           2
#define CONF_FAST_INFO                               3
#define CONF_FAST_PLUS_SLOW_INFO                     4
#define CONF_FAST_PLUS_SLOW1_INFO                    5
#define CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO          6
#define CHANGE_SITE_NAME                             7

// party serialize/deserialize header types, CONFERENCE object type req with apiNum > 87
#define PARTY_NOT_CHANGED                            1
#define PARTY_COMPLETE_INFO                          2
#define PARTY_FAST_INFO                              3
#define PARTY_FAST_PLUS_SLOW_INFO                    4
#define PARTY_NEW_INFO                               5
#define PARTY_FAST_PLUS_SLOW1_INFO                   6
#define PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO         7

#define AUDIO_VOLUME_MIN                             1
#define AUDIO_VOLUME_MAX                             10

#define UNKNOWN_ACTION                               -1
#define ADD_RESERVE                                  0  // Add reservation to reservation list
#define UPDATE_RESERVE                               1  // Update reservation in reservation list
#define CANCEL_RESERVE                               2  // Cancel reservation
#define ADD_REPEATED                                 3  // Add repeated reservation (recurrence list)
#define CANCEL_REPEATED                              4  // Delete repeated reservation
#define AUTHENTICATED_START                          5
#define ADD_REPEATED_EX                              6  // Add repeated reservation (pattern)

// conference action types - section one
#define DEL_PARTY                                    10 // Delete party from ongoing conference
#define NEW_PARTY                                    11 // Add party to ongoing conference

#define TERMINATE_CONF                               12 // Terminate ongoing conference

#define SET_END_TIME                                 14 // Set end time of ongoing conference
// see rest of conference action types list at the end
#define BLOCK_PARTY                                  16

#define DISCONNECT_PARTY                             18
#define RECONNECT_PARTY                              19

#define SET_CONF_VIDEO                               21
#define SET_PARTY_VIDEO                              22

#define UPDATE_PARTY                                 23 // Update party in ongoing conference

#define UPDATE_VISUAL_EFFECTS                        56

#define START_PARTY_PTRVIEW                          60 // Start video/audio preview of party

// /Recording List List
#define MAX_RECORDING_LINKS_IN_LIST                  ::GetMaxRecordingLinks()
#define ADD                                          24 // Add party to the recording link list
#define UPDATE                                       25 // Update party in the recording link list
#define RESERV_RL                                    26 // Update party in the recording link list

/*---------------------------------------------------------------------*/
/* Network SubService Name                                             */
/*---------------------------------------------------------------------*/

#ifndef SUB_SERVICE_NAME_AUTO
  #define SUB_SERVICE_NAME_AUTO                      "Auto"
#endif
#ifndef SUB_SERVICE_NAME_PRIMARY
  #define SUB_SERVICE_NAME_PRIMARY                   "Primary"
#endif
#ifndef SUB_SERVICE_NAME_SECONDARY
  #define SUB_SERVICE_NAME_SECONDARY                 "Secondary"
#endif

// party status (within reservation or within conference)
#define PASSWORD_IDENTIFICATION_METHOD               0
#define CALLED_PHONE_NUMBER_IDENTIFICATION_METHOD    1
#define CALLING_PHONE_NUMBER_IDENTIFICATION_METHOD   2

// channel protocol type
#define ISDN_CONNECTION                              1
#define DIRECT_CONNECTION                            2
#define ATM_CONNECTION                               3
#define H323_CONNECTION                              4
#define V35_CONNECTION                               5
#define V35_CONNECTION_DIRECT                        6
#define T1CAS_CONNECTION                             7
#define SIP_CONNECTION                               8

// Conference Type
#define CONF_TYPE_STANDARD                           1
#define CONF_TYPE_MEETING_ROOM                       2
#define CONF_TYPE_OPERATOR                           3

// Media
#define AUDIO_MEDIA                                  1
#define VIDEO_MEDIA                                  2
#define DATA_MEDIA                                   3
#define AUDIO_VIDEO_MEDIA                            4
#define AUDIO_DATA_MEDIA                             5
#define VIDEO_DATA_MEDIA                             6
#define ALL_MEDIA                                    7

// Network
#define NETWORK_H320                                 1
#define NETWORK_H323                                 2
#define NETWORK_H320_H323                            3

// Video protocols
#define VIDEO_PROTOCOL_H261                          1
#define VIDEO_PROTOCOL_H263                          2
#define VIDEO_PROTOCOL_H26L                          3
#define VIDEO_PROTOCOL_H264                          4
#define VIDEO_PROTOCOL_H264_HIGH_PROFILE             5
#define VIDEO_PROTOCOL_RTV                           6
#define VIDEO_PROTOCOL_MS_SVC                        7
#define VIDEO_PROTOCOL_VP8                           8 //N.A. DEBUG VP8

/*---------------------------------------------------------------------*/
/* Flags for m_dwRsrvFlags can be (Conference Type):                   */
/*---------------------------------------------------------------------*/

#define ACTIVE_PERMANEMT                             0x00000001
#define AUDIO_ONLY                                   0x00000002
#define LOCKED                                       0x00000004
#define H323_ONLY                                    0x00000008
#define LECTURE_MODE                                 0x00000010
#define LECTURE_MODE_SHOW                            0x00000020
#define CASCADING                                    0x00000040
#define CASCADING_AUTO                               0x00000080
#define GREET_AND_GUIDE                              0x00000100
#define GREET_AND_GUIDE_CONF_WELCOME                 0x00000200
#define GREET_AND_GUIDE_CONF_IVR                     0x00000400
#define OPERATOR_ASSISTANCE                          0x00000800
#define ALL_BUT_ONE_MUTE                             0x00001000
#define ON_HOLD                                      0x00002000
#define ENTRY_QUEUE                                  0x00004000
#define MUTE_INCOMING_PARTIES                        0x00008000
#define UNDER_VOTING                                 0x00010000
#define INVITE_PARTICPANT                            0x00020000
#define SECURED                                      0x00040000
#define ROLL_CALL_FLAG                               0x00080000
#define AUTO_LAYOUT_FLAG                             0x00100000
#define LECTURE_MODE_PRESENTATION                    0x00200000
#define ENCRYPTION                                   0x00400000
#define ENTRY_QUEUE_ACCESS                           0x00800000
#define SIP_FACTORY                                  0x01000000
#define TELEPRESENCE_MODE                            0x02000000
#define LPR                                          0x04000000
#define VIDEO_CLARITY_FLAG                           0x08000000
#define ECHO_SUPPRESSION                             0x10000000
#define KEYBOARD_SUPPRESSION                         0x20000000
#define RMX_GATEWAY                                  0x40000000
#define VIDEO_AS_CONTENT_FLAG                        0x80000000

/*---------------------------------------------------------------------*/
/* Flags for m_dwRsrvFlags2 can be (Conference Type):                   */
/*---------------------------------------------------------------------*/

#define LECTURE_SHOW                                 2
#define PRESENTATION_LECTURE_MODE                    3

// Continuous presence screen number
#define ONE_ONE                                      0
#define ONE_TWO                                      1
#define TWO_ONE                                      4
#define TWO_TWO                                      5
#define THREE_THREE                                  6
#define ONE_AND_FIVE                                 7
#define ONE_SEVEN                                    11
#define ONE_TWO_VER                                  12
#define ONE_TWO_HOR                                  13
#define ONE_PLUS_TWO_HOR                             14
#define ONE_PLUS_TWO_VER                             15
#define ONE_PLUS_THREE_HOR                           16
#define ONE_PLUS_THREE_VER                           17
#define ONE_PLUS_FOUR_VER                            18
#define ONE_PLUS_FOUR_HOR                            19
#define ONE_PLUS_EIGHT_CENTRAL                       20
#define ONE_PLUS_EIGHT_UPPER                         21

#define ONE_PLUS_TWO_HOR_UPPER                       22
#define ONE_PLUS_THREE_HOR_UPPER                     23
#define ONE_PLUS_FOUR_HOR_UPPER                      24
#define ONE_PLUS_EIGTH                               25

#define FOUR_FOUR                                    26
#define TWO_PLUS_EIGHT                               27
#define ONE_PLUS_TWELVE                              28
#define ONE_ONE_QCIF                                 29

#define ONE_TWO_FLEX                                 30
#define ONE_PLUS_TWO_HOR_R_FLEX                      31
#define ONE_PLUS_TWO_HOR_L_FLEX                      32
#define ONE_PLUS_TWO_HOR_UP_R_FLEX                   33
#define ONE_PLUS_TWO_HOR_UP_L_FLEX                   34
#define TWO_TWO_UP_R_FLEX                            35
#define TWO_TWO_UP_L_FLEX                            36
#define TWO_TWO_DOWN_R_FLEX                          37
#define TWO_TWO_DOWN_L_FLEX                          38
#define TWO_TWO_R_FLEX                               39
#define TWO_TWO_L_FLEX                               40
#define ONE_PLUS_ONE_OVERLAY                         41
#define ONE_PLUS_TWO_OVERLAY                         42
#define ONE_PLUS_THREE_OVERLAY                       43

#define ONE_PLUS_TWO_OVERLAY_ITP                     44
#define ONE_PLUS_THREE_OVERLAY_ITP                   45
#define ONE_PLUS_FOUR_OVERLAY_ITP                    46
#define ONE_TOP_LEFT_PLUS_EIGHT                      47
#define TWO_TOP_PLUS_EIGHT                           48

// Video cell status
#define AUDIO_ACTIVATED                              0
#define EMPTY_BY_OPERATOR_ALL_CONF                   1
#define EMPTY_BY_OPERATOR_THIS_PARTY                 2
#define BY_OPERATOR_ALL_CONF                         3
#define BY_OPERATOR_THIS_PARTY                       4
#define BY_H243_CHAIR_ALL_CONF                       5
#define BY_H243_CHAIR_THOS_PARTY                     6
#define BY_H243_REQUEST_ALL_CONF                     7
#define BY_H243_REQUEST_THIS_PARTY                   8
#define AUTO_SCAN                                    9
#define BY_ITP_LAYOUT_MGMT                           10

// Meeting room states
#define MEETING_ROOM_PASSIVE_STATE                   0
#define MEETING_ROOM_ACTIVE_STATE                    1
#define MEETING_ROOM_INITIALIZE_STATE                2

// Undefined party types
#define UNDEFINED_PARTY                              1
#define UNRESERVED_PARTY                             2

// Conference attended types
#define CONF_NONE_ATTENDED                           0
#define CONF_IVR                                     3

// Recording types - other types of recording will be added in the future
#define DEFAULT_RECORDING_TYPE                       1

// Cascade mode types
#define CASCADE_MODE_NONE                            0
#define CASCADE_MODE_MASTER                          1
#define CASCADE_MODE_SLAVE                           2
#define CASCADE_MODE_NEGOTIATED                      3
#define CASCADE_MODE_CONFLICT                        4
#define CASCADE_MODE_MCU                             5
#define CASCADE_MODE_AUTO                            255

#define LANGUAGE_NAME_LEN                            65 // 64 letters + terminator

// Num_type
#define UNKNOWN                                      0
#define INTERNATIONAL_TYPE                           1
#define NATIONAL_TYPE                                2
#define NETWORK_SPECIFIC_TYPE                        3
#define SUBSCRIBER_TYPE                              4
#define ABBREVIATED_TYPE                             6

/*---------------------------------------------------------------------*/
/* Flags for m_GWDialOutProtocols can be (Conference Type):            */
/*---------------------------------------------------------------------*/
#define GW_H323_OUT                                  1  // 0x0001
#define GW_SIP_OUT                                   2  // 0x0010
#define GW_H320_OUT                                  4  // 0x0100
#define GW_PSTN_OUT                                  8  // 0x1000

enum  eSubCPtype
{
  eSubCPtypeClassic,
  eSubCPtypeAdvanced,
  eSubCPtypeUnknown,
};

enum  eDualVideoMode
{
  eDualModeNone = 0,
  eDualModePeopleContent,
  eDualModeIsdnDuoVideo,
  eDualModeVisualConcertPC,
  eDualModeVisualConcertFX,
  eDualModeEnterprisePC,
  eDualModeIpDuoVideo,
  eDualModeUnknown                                      // MUST be last
};

enum eTelePresencePartyType
{
  eTelePresencePartyNone = 0,
  eTelePresencePartyRPX,
  eTelePresencePartyFlex,
  eTelePresencePartyCTS,
  eTelePresencePartyMaui,
  eTelePresencePartyInactive = 0xff
};

enum eIPSubServiceType
{
  eAutoIpSubService = 0,
  ePrimaryIpSubService,
  eSecondaryIpSubService,
};

// H239 Rate control - Relevant only when eDualVideoMode = eDualModeEnterprisePC
enum eEnterpriseMode
{
  eGraphics = 0,        // Default content rate  - LOW
  eHiResGraphics,       // MEDIUM
  eLiveVideo,           // HIGH
  // used for "Manual Control of Content Line Rate"
  eCustomizedRate,      // Customized Rate
};

// H239 protocol
enum ePresentationProtocol
{
	eH263Fix = 0,
	ePresentationAuto,    // Start as H264 and can be downgraded to H263.
	eH264Fix,             // For cascaded conferences
	eH264Dynamic,         // High quality H264
	eNoPresentation       // No conference content protocol mode was set.
};

inline std::ostream& operator <<(std::ostream& os, ePresentationProtocol val)
{
	switch (val)
	{
		case eH263Fix         : return os << "eH263Fix";
		case ePresentationAuto: return os << "ePresentationAuto";
		case eH264Fix         : return os << "eH264Fix";
		case eH264Dynamic     : return os << "eH264Dynamic";
		case eNoPresentation  : return os << "eNoPresentation";
	}
	return os << "Invalid #" << (int)val;
};

enum ePartyData
{
  FULL_DATA,
  SHORT_DATA
};

typedef struct
{
  char phone_number[PHONE_NUMBER_DIGITS_LEN];
} Phone;

// IVR Message Descriptor
typedef struct SIVRMSGDESCRIPTOR
{
  char ivrMsgFullPath[MAX_FULL_PATH_LEN];
  WORD ivrMsgDuration;
  WORD ivrMsgCheckSum;
} IVRMsgDescriptor;

typedef enum
{
  eVideoQualityAuto,
  eVideoQualityMotion,
  eVideoQualitySharpness
}eVideoQuality;

// Sorted!
typedef enum
{
	eInvalidModeType,
	eCIF30,
	eCIF60,
	e2CIF30,
	eWCIF60,
	eSD15,
	eSD30,
	eW4CIF30,
	eSD60,
	eHD720Asymmetric,
	eHD720Symmetric,
	eHD720At60Asymmetric,
	eHD720At60Symmetric,
	eHD1080Asymmetric,
	eHD1080Symmetric,
	eHD1080At60Asymmetric,
	eHD1080At60Symmetric,

	eLasth264VideoMode
} Eh264VideoModeType;

typedef enum
{
  eAuto_Res,
  eCIF_Res,
  eSD_Res,
  eHD720_Res,
  eHD1080_Res,
  eHD1080p60_Res
} EVideoResolutionType;

static const char* eVideoResolutionTypeNames[] =
{
	"eAuto_Res",
	"eCIF_Res",
	"eSD_Res",
	"eHD720_Res",
	"eHD1080_Res",
	"eHD1080p60_Res"
};


enum eMonthlyPattern
{
  eByDate,
  eByDay
};

enum eRepeatedType
{
  eDaily,
  eWeekly,
  eMonthly
};

enum eRepeatedInstance
{
  eFirst = 1,
  eSecond,
  eThird,
  eFourth,
  eLast
};

enum  eMessageOverlayFontSizeType
{
  eSmall = 0,
  eMedium,
  eLarge
};

enum  eMessageOverlayPositionType
{
  eTop = 0,
  eMiddle,
  eBottom
};

enum  eMessageOverlaySpeedType
{
  eStatic = 0,
  eSlow,
  eFast
};

enum  eMessageOverlayColorType
{
  eWhite_font_on_light_blue_background =0,
  eWhite_font_on_black_background,
  eWhite_font_on_gray_background,
  eWhite_font_on_red_background,
  eWhite_font_on_orange_background,
  eWhite_font_on_blue_background,
  eWhite_font_on_olive_background,
  eWhite_font_on_green_background,
  eWhite_font_on_purple_background,
  eRed_font_on_white_background,
  eWhite_font_on_deep_brown_background,
  eWhite_font_on_brown_background,
  eYellow_font_on_black_background,
  eYellow_font_on_deep_blue_background,
  eLightBlue_font_on_black_background,
  eBlue_font_on_white_background,
  eGreen_font_on_black_background,
  eGreyGreen_font_on_white_background,
  eBlack_font_on_gray_background,
  eBlack_font_on_white_background
};

enum eTextColorType
{
  eRed_font =0,         // 0xFF0000
  eBlue_font,           // 0x0026FF
  eSkyBlue_font,        // 0X00BDF4
  eGreen_font,          // 0X007F0E
  eYellowGreen_font,    // 0X00FF21
  eLightYellow_font,    // 0XFAFFAA
  eYellow_font,         // 0XFFFA1B
  eBlack_font,          // 0X000000
  eWhite_font,          // 0XFFFFFF
};

enum eSiteNameDisplayMode
{
  eSiteNameAuto=0,
  eSiteNameOn,
  eSiteNameOff
};

enum eSiteNameDisplayPosition
{
  eLeft_top_position=0,
  eTop_position,
  eRight_top_position,
  eLeft_middle_position,
  eRight_middle_position,
  eLeft_bottom_position,
  eBottom_position,
  eRight_bottom_position,
  eCustom
};

enum eIconDisplayMode
{
  eIconDisplayOnChange = 0,
  eIconDIsplayPermanent
};


enum eSignType
{
  eMinus = -1,
  ePlus  = 1
};

enum eAllocationModeType
{
  eAllocationModeNone,
  eAllocationModeAuto,
  eAllocationModeFixed,
  LAST_ALLOCATION_MODE_TYPE
};

typedef enum
{
  eHD1080p60Res,
  eHD1080Res,
  eHD720Res,
  eHD720p60Res,
  eSDRes,
  eH264Res,
  eH263Res,
  eH261Res
} EHDResolution;

typedef enum
{
  eTerminateAfterLastLeaves,
  eTerminateWithLastRemains
} ELastQuitType;

typedef enum
{
  AspectRatio_4X3,
  AspectRatio_16X9
} EAspectRatio;

typedef enum
{
  eCopLevelEncoderVideoFormat_None = -1,
  eCopLevelEncoderVideoFormat_QCIF = 0,
  eCopLevelEncoderVideoFormat_CIF,
  eCopLevelEncoderVideoFormat_4CIF,
  eCopLevelEncoderVideoFormat_4CIF_16_9,
  eCopLevelEncoderVideoFormat_HD720p,
  eCopLevelEncoderVideoFormat_HD1080p,
  eCopLevelEncoderVideoFormat_Last
} ECopLevelEncoderVideoFormat;

typedef enum
{
  eCopVideoFrameRate_None = -1,
  eCopVideoFrameRate_12_5 = 0,
  eCopVideoFrameRate_15,
  eCopVideoFrameRate_25,
  eCopVideoFrameRate_30,
  eCopVideoFrameRate_50,
  eCopVideoFrameRate_60,
  eCopVideoFrameRate_Last
} ECopVideoFrameRate;

typedef enum
{
  eEnglish,
  eGerman,
  eSpanishSA,
  eFrench,
  eJapanese,
  eKorean,
  eChineseSimpl,
  LAST_LANGUAGE
} ELanguges;

typedef enum
{
  eTelePresenceLayoutManual,
  eTelePresenceLayoutContinuousPresence,
  eTelePresenceLayoutRoomSwitch,
  eTelePresenceLayoutCpSpeakerPriority,
  eTelePresenceLayoutCpParticipantsPriority
} ETelePresenceLayoutMode;

typedef enum
{
  eTipCompatibleNone,
  eTipCompatibleVideoOnly,
  eTipCompatibleVideoAndContent,
  eTipCompatiblePreferTIP, //TIP call from Polycom EPs feature
} eTipCompatibilityConfMode;

typedef enum
{
  eMsSvcResourceOptimize,
  eMsSvcVideoOptimize,
} eMsSvcVideoMode;

typedef enum
{
  eTipPartyNone,
  eTipPartyMaster,
  eTipPartySlave,
} eTipPartyModeType;

typedef enum
{
  e64000,
  e96000,
  e128000,
  e192000,
  e256000,
  e320000,
  e384000,
  e512000,
  e768000,
  e832000,
  e1024000,
  e1152000,
  e1280000,
  e1472000,
  e1536000,
  e1728000,
  e1920000,
  e2048000,
  e2560000,
  e3072000,
  e3584000,
  e4096000,
  e6144000,
  e8192000,
  eMAX_NUM_THRESHOLD_RATES
} EThresholdBitRate;

typedef enum
{
  e_cif30,
  e_cif60,
  e_wcif,
  e_sd15,
  e_sd30,
  e_sd60,
  e_hd720p30,
  e_hd720p60,
  e_hd1080p30,
  e_hd1080p60,
  e_MAX_NUM_RESOLUTION_TYPES
} EResolutionType;


typedef enum
{
  e_balanced,
  e_resource_optimized,
  e_user_exp_optimized,
  e_hi_profile_optimized,
  e_manual
} EResolutionConfigType;

typedef enum                                // Status Field
{
  eStsOK,
  eStsExpired,
  eStsSUSPEND,
  ePasswdConflict,
  eWrongSysMode
} EResStatusList;

typedef enum                                // H264 VSW HighProfile Preference Type
{                                           // eWhenPossible, // in V7.6 decided not to implement this option
  eAlways,
  eBaseLineOnly
} EH264VSWHighProfilePreferenceType;


typedef enum                                // Encryption Mode Field
{
  eEncryptNone,
  eEncryptAll,
  eEncryptWhenAvailable
} EEncyptionType;


typedef struct
{
  char service_name[ONE_LINE_BUFFER_LEN];
  BOOL sip_register;
  BOOL accept_call;
  BYTE status;                              // sipProxySts
} ServiceRegistrationContent;

#define SIP_REG_NOT_CONFIGURED 0x00000001   // sipProxySts
#define SIP_REG_REGISTERED     0x00000002   // sipProxySts
#define SIP_REG_FAILED         0x00000004   // sipProxySts


#define MS_SVC_HD_720_PRID    20
#define MS_SVC_SD_PRID        10
#define MS_SVC_CIF_PRID        0

#define SIP_REG_FAILED         0x00000004   // sipProxySts



typedef enum
{
  e_rtv_InvalidModeType,
  e_rtv_QCIF15,
  e_rtv_QCIF30,
  e_rtv_CIF15,
  e_rtv_CIF30,
  e_rtv_2CIF30,
  e_rtv_WCIF30,
  e_rtv_VGA15,
  e_rtv_VGA30,
  e_rtv_SD15,
  e_rtv_SD30,
  e_rtv_HD720Asymmetric,
  e_rtv_W4CIF30,
  e_rtv_HD720Symmetric,
  eLastRtvVideoMode
} ERtvVideoModeType;

typedef enum
{
	e_res_dummy,
	e_res_720_5fps,
	e_res_720_30fps,
	e_res_1080_15fps,
	e_res_1080_30fps,
	e_res_1080_60fps,
	e_res_last
} eCascadeOptimizeResolutionEnum;

inline std::ostream& operator <<(std::ostream& os, eCascadeOptimizeResolutionEnum val)
{
	switch (val)
	{
		case e_res_dummy     : return os << "eRes_dummy";
		case e_res_720_5fps  : return os << "eRes_720_5fps";
		case e_res_720_30fps : return os << "eRes_720_30fps";
		case e_res_1080_15fps: return os << "eRes_1080_15fps";
		case e_res_1080_30fps: return os << "eRes_1080_30fps";
		case e_res_1080_60fps: return os << "eRes_1080_60fps";
		case e_res_last      : return os << "eRes_last";
	}
	return os << "Invalid #" << (int)val;
};

typedef enum  {
	eConfMediaType_dummy,
	eAvcOnly,
	eSvcOnly,
	eMixAvcSvc,
	eMixAvcSvcVsw,
	eConfMediaType_last
} eConfMediaType;

typedef enum 
{
	eHdVswNon,        //mean the HdVsw is not enabled in mixed avc and svc mode.
	e_HdVsw720,
	e_HdVsw1080,
	eHdVswLast
}eHdVswTypesInMixAvcSvc;



typedef enum
{
	eCodecSubTypeUndefined,
	eH264,
	eH264SVC,
	eSAC,
	eG711,
	eG722,
	eG722_1,
	eCodecSubTypeMaxNumOfValues
} ECodecSubType;

typedef enum  {
	ePartyMediaType_dummy,
	eAvcPartyType,
	eSvcPartyType,
//	eAutoPartyType,
	ePartyMediaType_last
} ePMediaType;

typedef enum
{
  eAutoTicks,
  e150Ticks,
  e300Ticks,
  e500Ticks,
} eSpeakerChangeThresholdEnum;

enum eTypeOfLinkParty {
	eRegularParty,
	eSubLinkParty,
	eMainLinkParty
};

enum eTypeOfSha1Length {
	eSha1_length_80,
	eSha1_length_32,
	eSha1_length_80_32
};

typedef enum
{
    eMediaStateEmpty = 0, // no participants in conf
    eMediaStateAvcOnly,
    eMediaStateSvcOnly,
    eMediaStateMixAvcSvc
    // future states - only audio is mixed:
    // eMediaStateAvcVideoMixedAudio
    // eMediaStateSvcVideoMixedAudio
} eConfMediaState;

enum EOperationPointPreset
{
	eOPP_dummy,
	eOPP_mobile,
	eOPP_qvga,
	eOPP_cif,
	eOPP_vga,
	eOPP_sd,
	eOPP_hd,
	eOPP_last
};

enum eIsUseOperationPointsPreset
{
	eIsUseOPP_No = 0,
	eIsUseOPP_Yes_SameTime,
	eIsUseOPP_Yes_Unrecognized
};

enum eAvMcuLinkType // MS Lync
{
	eAvMcuLinkNone = 0,
	eAvMcuLinkMain,
	eAvMcuLinkSlaveOut,
	eAvMcuLinkSlaveIn,
};

static const char* eAvMcuLinkTypeNames[] = // MS Lync
{
	"eAvMcuLinkNone",
	"eAvMcuLinkMain",
	"eAvMcuLinkSlaveOut",
	"eAvMcuLinkSlaveIn"
};

typedef enum
{
	eMsft2013None  			= 0,
	eMsft2013LyncClient     = 1,
	eMsft2013AvMCU          = 2
}enMsft2013ActivationState;

typedef enum enMsftAvmcuState
{
	eMsftAvmcuNone 		    = 0,
	eMsftAvmcuUnkown 		= 1,
	eMsftAvmcu2010	 		= 2,
	eMsftAvmcu2013   		= 3,
} eMsftAvmcuState ;

static const char* enMsftAvmcuStateNames[] =
{
     "eMsftAvmcuNone",
     "eMsftAvmcuUnkown",
     "eMsftAvmcu2010",
     "eMsftAvmcu2013"
};

typedef enum {
	eMediaTypeUpdateNotNeeded = 0,
	eMediaTypeUpdateAudioToVideo = 1,
	eMediaTypeUpdateVideoToAudio = 2
} EMediaTypeUpdate;

typedef enum
{
	eMsftEscalationNone     = 0,
	eMsftEscalationActive 	= 1,
	eMsftEscalationInActive	= 2,

} EMsftMediaEscalationStatus;

//eFeatureRssDialin
enum  enSrsPlaybackLayoutMode
{
	eSrsAutoMode 		=0,
	eSrsLectureMode		=1
};
enum  enSrsVideoLayoutType
{
	eSrsVideoLayoutAuto 		=0,
	eSrsVideoLayout1X1		=1,
	eSrsVideoLayout1X2		=2,
	eSrsVideoLayoutLecture		=3  //for playback
};

enum eIvrSlideConversionMethod
{
    eIvrSlideLowRes         = 0,
    eIvrSlideHighRes        = 1,
    eIvrSlideLowHighRes     = 2
};

enum eIvrSlideConversionStatus
{
    eIvrSlideConversionInProgress     = 0,
    eIvrSlideConversionSuccess        = 1,
    eIvrSlideConversionFailed         = 2,
    eIvrSlideConversionInvalidResolution    = 3
};

static const char * eIvrSlideConversionStatusNames[] = 
{
    "in_progress",
    "success",
    "failed",
    "invalid_resolution"
};

enum eIvrSlideImageType
{
    eIvrSlideImageJpg           = 0,
    eIvrSlideImageBmp           = 1,
};

#endif // _CONFPARTYAPIDEFINES_H__
