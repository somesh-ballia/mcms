#ifndef Video_Defines_H__
#define Video_Defines_H__

#include <vector>

#include "ConfPartySharedDefines.h"

///////////////////////////////////////////////////////////////////////////
#define NIL(Type) (Type*)0
#define DEFAULT_DECODER_DETECTED_MODE_WIDTH                 0xFFFFFFFF
#define DEFAULT_DECODER_DETECTED_MODE_HEIGHT                0xFFFFFFFF
#define DEFAULT_SAMPLE_ASPECT_RATIO                         0
#define DEFAULT_STATIC_MB                                   0
#define DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH  0
#define DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT 0

///////////////////////////////////////////////////////////////////////////
enum eVideoResolution
{
	eVideoResolutionDummy,
	eVideoResolutionQCIF,
	eVideoResolutionCIF,
	eVideoResolutionVGA,
	eVideoResolution4SIF,
	eVideoResolution4CIF,
	eVideoResolution525SD,
	eVideoResolution625SD,
	eVideoResolutionSVGA,
	eVideoResolutionXGA,
	eVideoResolutionHD720,
	eVideoResolution16CIF,
	eVideoResolutionSIF,
	eVideoResolutionQVGA,
	eVideoResolutionHD1080,
	eVideoResolutionLast
};

struct resWidthAndHeightStruct
{
	eVideoResolution eResolution;
	DWORD            resWidth;
	DWORD            resHeight;
} ;

static resWidthAndHeightStruct ResoltionWidthAndHeight[eVideoResolutionLast] =
{
	// eResolution      /resWidth //resHeight
	{ eVideoResolutionDummy ,    0,    0},
	{ eVideoResolutionQCIF  ,  176,  144},
	{ eVideoResolutionCIF   ,  352,  288},
	{ eVideoResolutionVGA   ,  640,  480},
	{ eVideoResolution4SIF  ,  704,  480},
	{ eVideoResolution4CIF  ,  704,  576},
	{ eVideoResolution525SD ,  720,  480},
	{ eVideoResolution625SD ,  720,  576},
	{ eVideoResolutionSVGA  ,  800,  600},
	{ eVideoResolutionXGA   , 1024,  768},
	{ eVideoResolutionHD720 , 1280,  720},
	{ eVideoResolution16CIF , 1408, 1152},
	{ eVideoResolutionSIF   ,  352,  240},
	{ eVideoResolutionQVGA  ,  320,  240},
	{ eVideoResolutionHD1080, 1920, 1088},
};

static const char* VideoResolutionAsString[eVideoResolutionLast] =
{
	"VideoResolutionDummy",
	"VideoResolutionQCIF",
	"VideoResolutionCIF",
	"VideoResolutionVGA",
	"VideoResolution4SIF",
	"VideoResolution4CIF",
	"VideoResolution525SD",
	"VideoResolution625SD",
	"VideoResolutionSVGA",
	"VideoResolutionXGA",
	"VideoResolutionHD720",
	"VideoResolution16CIF",
	"VideoResolutionSIF",
	"VideoResolutionQVGA",
	"VideoResolutionHD1080"
};


enum eVideoFrameRate
{
	eVideoFrameRateDUMMY,
	eVideoFrameRate60FPS,
	eVideoFrameRate50FPS,
	eVideoFrameRate30FPS,
	eVideoFrameRate25FPS,
	eVideoFrameRate20FPS,
	eVideoFrameRate15FPS,
	eVideoFrameRate12_5FPS,
	eVideoFrameRate10FPS,
	eVideoFrameRate7_5FPS,
	eVideoFrameRate5FPS,
	eVideoFrameRate6FPS,
	eVideoFrameRate3FPS,
	eVideoFrameRateLast
};

typedef eVideoFrameRate eVideoFrameRateType;

enum eVideoProfile
{
	eVideoProfileDummy,
	eVideoProfileBaseline,
	eVideoProfileHigh,
	eVideoProfileRtv,
	eVideoProfileMain,
	eVideoProfileLast
};

typedef enum eVideoProfile eVideoProfileType;

enum eVideoPacketPayloadFormat
{
	eVideoPacketPayloadFormatDummy,
	eVideoPacketPayloadFormatSingleUnit,
	eVideoPacketPayloadFormatFragmentationUnit,
	eVideoPacketPayloadFormatLast
};

typedef enum eVideoPacketPayloadFormat eVideoPacketPayloadFormatType;

enum eLectureModeType
{
	eLectureModeNone = 0,     // Lecture Mode Off
	eLectureModeRegular,      // Regular Lecture Mode
	eLectureModePresentation  // Presentation mode
};

enum ePartyLectureModeRole
{
	eREGULAR,
	eLECTURER,                // the party sees conf or private layout
	eLISTENER,                // the party sees the lecture only, in layout 1x1
	eCOLD_LISTENER            // the party sees someone (not the speaker), in layout 1x1
};

enum AudioActivatedType
{
	eAudioActivatedFalse = 0,
	eAudioActivatedRegular,
	eAudioActivated_LecturerDisonnect,
};

const WORD MIN_TIME_INTERVAL = 5;

#define MAX_SUB_IMAGES_IN_LAYOUT 16

enum LayoutType
{
	CP_LAYOUT_1X1                  = 0,
	CP_LAYOUT_1X2                  = 1,
	CP_LAYOUT_2X1                  = 2,

	CP_LAYOUT_2X2                  = 3,
	CP_LAYOUT_3X3                  = 4,
	CP_LAYOUT_1P5                  = 5,

	CP_LAYOUT_1P7                  = 6,
	CP_LAYOUT_1x2VER               = 7,
	CP_LAYOUT_1x2HOR               = 8,

	CP_LAYOUT_1P2VER               = 9,
	CP_LAYOUT_1P2HOR               = 10,
	CP_LAYOUT_1P3HOR               = 11,

	CP_LAYOUT_1P3VER               = 12,
	CP_LAYOUT_1P4HOR               = 13,
	CP_LAYOUT_1P4VER               = 14,

	CP_LAYOUT_1P8CENT              = 15,
	CP_LAYOUT_1P8UP                = 16,
	CP_LAYOUT_1P2HOR_UP            = 17,

	CP_LAYOUT_1P3HOR_UP            = 18,
	CP_LAYOUT_1P4HOR_UP            = 19,
	CP_LAYOUT_1P8HOR_UP            = 20,

	CP_LAYOUT_4X4                  = 21,
	CP_LAYOUT_2P8                  = 22,
	CP_LAYOUT_1P12                 = 23,

	CP_LAYOUT_1X2_FLEX             = 24,
	CP_LAYOUT_1P2HOR_RIGHT_FLEX    = 25,
	CP_LAYOUT_1P2HOR_LEFT_FLEX     = 26,

	CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX = 27,
	CP_LAYOUT_1P2HOR_UP_LEFT_FLEX  = 28,

	CP_LAYOUT_2X2_UP_RIGHT_FLEX    = 29,
	CP_LAYOUT_2X2_UP_LEFT_FLEX     = 30,

	CP_LAYOUT_2X2_DOWN_RIGHT_FLEX  = 31,
	CP_LAYOUT_2X2_DOWN_LEFT_FLEX   = 32,

	CP_LAYOUT_2X2_RIGHT_FLEX       = 33,
	CP_LAYOUT_2X2_LEFT_FLEX        = 34,

	CP_LAYOUT_OVERLAY_1P1          = 35,
	CP_LAYOUT_OVERLAY_1P2          = 36,
	CP_LAYOUT_OVERLAY_1P3          = 37,
	CP_LAYOUT_OVERLAY_ITP_1P2      = 38,
	CP_LAYOUT_OVERLAY_ITP_1P3      = 39,
	CP_LAYOUT_OVERLAY_ITP_1P4      = 40,
	CP_LAYOUT_1TOP_LEFT_P8         = 41,
	CP_LAYOUT_2TOP_P8              = 42,

	// ALWAYS the last one
	CP_NO_LAYOUT
};

inline LayoutType& operator ++(LayoutType& e)
{
	return enum_increment(e, CP_LAYOUT_1X1, CP_NO_LAYOUT);
}
static char* LayoutTypeAsString[CP_NO_LAYOUT + 1] =
{
	"LAYOUT_1X1", "LAYOUT_1X2", "LAYOUT_2X1",
	"LAYOUT_2X2", "LAYOUT_3X3", "LAYOUT_1P5",
	"LAYOUT_1P7", "LAYOUT_1x2VER", "LAYOUT_1x2HOR",
	"LAYOUT_1P2VER", "LAYOUT_1P2HOR", "LAYOUT_1P3HOR",
	"LAYOUT_1P3VER", "LAYOUT_1P4HOR", "LAYOUT_1P4VER",
	"LAYOUT_1P8CENT", "LAYOUT_1P8UP", "LAYOUT_1P2HOR_UP",
	"LAYOUT_1P3HOR_UP", "LAYOUT_1P4HOR_UP", "LAYOUT_1P8HOR_UP",
	"LAYOUT_4X4", "LAYOUT_2P8", "LAYOUT_1P12",
	"LAYOUT_1X2_FLEX", "LAYOUT_1P2HOR_RIGHT_FLEX", "LAYOUT_1P2HOR_LEFT_FLEX",
	"LAYOUT_1P2HOR_UP_RIGHT_FLEX", "LAYOUT_1P2HOR_UP_LEFT_FLEX",
	"LAYOUT_2X2_UP_RIGHT_FLEX", "LAYOUT_2X2_UP_LEFT_FLEX",
	"LAYOUT_2X2_DOWN_RIGHT_FLEX", "LAYOUT_2X2_DOWN_LEFT_FLEX",
	"LAYOUT_2X2_RIGHT_FLEX", "LAYOUT_2X2_LEFT_FLEX",
	"CP_LAYOUT_OVERLAY_1P1","CP_LAYOUT_OVERLAY_1P2","CP_LAYOUT_OVERLAY_1P3",
	"CP_LAYOUT_OVERLAY_ITP_1P2","CP_LAYOUT_OVERLAY_ITP_1P3","CP_LAYOUT_OVERLAY_ITP_1P4",
	"LAYOUT_1TOP_LEFT_P8","LAYOUT_2TOP_P8",
	"NO_LAYOUT"
};

enum RequestPriority
{
	AUTO_Prior=0,
	PARTY_Prior,
	CHAIRMAN_Prior,
	OPERATOR_Prior,
	MCMS_Prior,
	SYNC_LOST_Prior
};

enum Force_Level
{
	CONF_lev,
	PARTY_lev
};


enum VideoActivities
{
	DEFAULT_Activ,
	AUTO_PARTY_Activ,
	FORCE_PARTY_Activ,
	BLANK_PARTY_Activ,
	AUTO_CONF_Activ,
	FORCE_CONF_Activ,
	FORCE_MVC_CONF_Activ,       // new cap MVC
	FORCE_CONTENT_Activ,        // PictureTel PeopleContentV0 force
	BLANK_CONF_Activ,
	FORCE_PRIVATE_PARTY_Active, // Private layout force
	BLANK_PRIVATE_PARTY_Active, // Personal layout forcing.
	AUTO_SCAN_Active
};

enum EConditionForContentDecoder
{
	eCondition_confTerminate,
	eCondition_StartPresentation,
	eCondition_StopPresentation,
	eCondition_LegacyConnected,
	eCondition_LastLegacyLeftConf,
	eCondition_SnatchPresentation,
	eCondition_Last
};

enum eVideoConfType
{
	eVideoConfTypeDummy,
	eVideoConfTypeCP,
	eVideoConfTypeVSW,
	eVideoConfTypeCopHD108025fps,
	eVideoConfTypeCopHD72050fps,
	eVideoConfTypeLast
};

static char* VideoConfTypeAsString[eVideoConfTypeLast+1] =
{
	"eVideoConfTypeDummy",
	"eVideoConfTypeCP",
	"eVideoConfTypeVSW",
	"eVideoConfTypeCopHD108025fps",
	"eVideoConfTypeCopHD72050fps",
	"eVideoConfTypeLast"
};

///////////////////////////////////////////////////////////////////////////
const WORD CIF_X_SIZE  = 352;
const WORD CIF_Y_SIZE  = 288;
const WORD QCIF_X_SIZE = 176;
const WORD QCIF_Y_SIZE = 144;

///////////////////////////////////////////////////////////////////////////
#define LAY_1P2HOR_CROP_Y_OFFSET 70
#define LAY_1P3HOR_CROP_Y_OFFSET 48
#define LAY_1P4HOR_CROP_Y_OFFSET 35

// new defines
#define LAY_1x2HOR_Y_OFFSET      70
#define LAY_1P2HOR_UP_Y_OFFSET   68
#define LAY_1P3HOR_UP_Y_OFFSET   43
#define LAY_1P4HOR_UP_Y_OFFSET   34
// ------------------------------------
#define LAY_1P8HOR_UP_Y_OFFSET   68

///////////////////////////////////////////////////////////////////////////

typedef std::vector<DWORD> CPartyImageVector;

typedef std::vector<std::pair<PartyRsrcID, bool> > CPartyImageInfoVector;
typedef std::vector<std::pair<RoomID, CPartyImageInfoVector> > CRoomSpeakerVector;
typedef std::vector<std::pair<RoomID, CPartyImageInfoVector> >::iterator CRoomPartyVectorIter;

///////////////////////////////////////////////////////////////////////////
#endif // Video_Defines_H__
