#include "ConfPartyGlobals.h"
#include "IVRServiceList.h"
#include "IVRSlidesList.h"
#include "ConfParty.h"
#include "CommConf.h"
#include "IpScm.h"
#include "H264Util.h"
#include "H320ComMode.h"
#include "StatusesGeneral.h"
#include "SystemFunctions.h"
#include "AudHostApiDefinitions.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "ApiStatuses.h"
#include "CommConfDB.h"
#include "HostCommonDefinitions.h"
#include "H264VideoMode.h"
#include "IConv.h"
#include "UnicodeDefines.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ConfPartyProcess.h"
#include "DefinesGeneral.h"
#include "SipUtils.h"
#include "HlogApi.h"
#include "COP_video_mode.h"
#include "RtvVideoMode.h"
#include "IpCommon.h"
#include "VideoOperationPointsSet.h"
#include "SipCaps.h"
#include "IpSizeDefinitions.h"
#include "RsrcParams.h"
#include "EventPackage.h"
#include <fstream>
#include "UnifiedComMode.h"

#define tid_0	0

// User Agents
const char* UA_MOC_PRODUCT_NAME      = "Microsoft "; // This product name is compatible with "Microsoft Office Communicator" and "Microsoft Lync"
const char* UA_SONY_PRODUCT_NAME     = "SONY";
const char* UA_PVX_PRODUCT_NAME      = "Polycom VV";
const char* UA_TANDBERG_PRODUCT_NAME = "TANDBERG";
const char* UA_UCCAPI_PRODUCT_NAME   = "UCCAPI";
const char* UA_CUCM_PRODUCT_NAME     = "Cisco-CUCM";
const char* UA_OCPHONE_PRODUCT_NAME  = "OCPhone";

static CAVmsgServiceList* pAVmsgServList     = NULL;
static CIVRSlidesList*    pSlidesList        = NULL;
static DWORD              sVisualNameCounter = 0;
static DWORD			  stdwCurrentLoggerNumber = 0xFFFFFFFF;

// 1080p60debug - to be removed
static DWORD s1080p60mbps = 489600;

namespace GlobalFlags
{
	static BOOL dongleEncryptionValue   = FALSE;
	static BOOL IsCOPdongleSysMode      = FALSE;
	static BOOL dongleFederalValue      = FALSE;
	static BOOL donglePstnValue         = FALSE;
	static BOOL dongleTelepresenceValue = FALSE;
	static BOOL dongleMsValue           = FALSE;
	static BOOL dongle1500qHDValue      = TRUE;
	static BOOL dongleSvcValue          = FALSE;
	static BOOL dongleCifPlus          = FALSE;
	static BOOL dongleTipInteropValue   = FALSE;
	static BOOL dongleLicenseValid   	= FALSE;
}

//--------------------------------------------------------------------------
CAVmsgServiceList* GetpAVmsgServList()
{
	return pAVmsgServList;
}

//--------------------------------------------------------------------------
void SetpAVmsgServList(CAVmsgServiceList* p)
{
	pAVmsgServList = p;
}

//--------------------------------------------------------------------------
CIVRSlidesList* GetpSlidesList()
{
	return pSlidesList;
}

//--------------------------------------------------------------------------
void SetpSlidesList(CIVRSlidesList* p)
{
	pSlidesList = p;
}

//--------------------------------------------------------------------------
const char* GetIVRFeatureDirectory(const WORD ivr_feature_opcode, int& status)
{
	status = STATUS_OK;
	switch (ivr_feature_opcode)
	{
		case IVR_FEATURE_LANG_MENU            : return("langmenu");
		case IVR_FEATURE_CONF_PASSWORD        : return("cpw");
		case IVR_FEATURE_PIN_CODE             : return("usrpin");
		case IVR_FEATURE_OPER_ASSISTANCE      : return("opassist");
		case IVR_FEATURE_WELCOME              : return("welc");
		case IVR_FEATURE_CONF_LEADER          : return("leader");
		case IVR_FEATURE_GENERAL              : return("general");
		case IVR_FEATURE_ROLL_CALL            : return("rollcall");
		case IVR_FEATURE_NUMERIC_CONFERENCE_ID: return("nid");
		case IVR_FEATURE_MUTE_NOISY_LINE      : return("noise");
		case IVR_FEATURE_RECORDING            : return("record");
		case IVR_FEATURE_PLAYBACK             : return("playback");
	}

	status = STATUS_ILLEGAL;
	return("");
}

//--------------------------------------------------------------------------
const char* GetIVREventDirectory(const WORD ivr_event_opcode, int& status)
{
	switch (ivr_event_opcode)
	{
		case IVR_EVENT_GET_LANGUAGE:
		{
			return("menumsg");
		}
		case IVR_EVENT_LANGUAGE_RETRY:
		{
			return("retlang");
		}
		case IVR_EVENT_GET_CONFERENCE_PASSWORD:
		{
			return("getcpw");
		}
		case IVR_EVENT_CONFERENCE_PASSWORD_RETRY:
		{
			return("retcpw");
		}
		case IVR_EVENT_GET_DIGIT:
		{
			return("getdig");
		}
		case IVR_EVENT_GET_PIN_CODE:
		{
			return("getpin");
		}
		case IVR_EVENT_PIN_CODE_RETRY:
		{
			return("retpin");
		}
		case IVR_EVENT_WAIT_FOR_OPERATOR_MESSAGE:
		{
			return("waitmsg");
		}
		case IVR_EVENT_SYSTEM_DISCONNECT_MESSAGE:
		{
			return("discmsg");
		}
		case IVR_EVENT_WELCOME_MSG:
		{
			return("welcmsg");
		}
		case IVR_EVENT_ENTRANCE_MSG:
		{
			return("entranc");
		}
		case IVR_EVENT_GET_LEADER_IDENTIFIER:
		{
			return("getlid");
		}
		case IVR_EVENT_GET_LEADER_PASSWORD:
		{
			return("getlpw");
		}
		case IVR_EVENT_LEADER_PASSWORD_RETRY:
		{
			return("retlpw");
		}

		case IVR_EVENT_FIRST_PARTY:
		case IVR_EVENT_SECURE_ON:
		case IVR_EVENT_SECURE_OFF:
		case IVR_EVENT_LOCK_ON:
		case IVR_EVENT_LOCK_OFF:
		case IVR_EVENT_ADD_ME_QA:
		case IVR_EVENT_REMOVE_ME_QA:
		case IVR_EVENT_NEXT_QA:
		case IVR_EVENT_PARTY_ENTER:
		case IVR_EVENT_PARTY_EXIT:
		case IVR_EVENT_CONF_END:
		case IVR_EVENT_RECORD_START:
		case IVR_EVENT_RECORD_END:
		case IVR_EVENT_RECORD_PAUSE:
		case IVR_EVENT_RECORD_NOT_AVAILABLE:
		case IVR_EVENT_PLAYBK_START:
		case IVR_EVENT_PLAYBK_END:
		case IVR_EVENT_PLAYBK_PAUSE:
		case IVR_EVENT_PLAYBK_NOT_AVAILABLE:
		case IVR_EVENT_CONF_ON_HOLD:
		case IVR_EVENT_MENU_LEADER:
		case IVR_EVENT_MENU_SIMPLE:
		case IVR_EVENT_BILLING_NUM:
		case IVR_EVENT_NUM_PARTIES_IN_CONF_BEGIN:
		case IVR_EVENT_NUM_PARTIES_IN_CONF_END:
		case IVR_EVENT_END_TIME_ALERT:
		case IVR_EVENT_CURRENT_SPEAKER:
		case IVR_EVENT_NO_RESOURCES:
		case IVR_EVENT_CHANGE_PWDS_MENU:
		case IVR_EVENT_CHANGE_PWDS_CONF:
		case IVR_EVENT_CHANGE_PWDS_LEADER:
		case IVR_EVENT_CHANGE_PWD_CONFIRM:
		case IVR_EVENT_CHANGE_PWD_INVALID:
		case IVR_EVENT_CHANGE_PWD_OK:
		case IVR_EVENT_CONF_LOCK:
		case IVR_EVENT_REQUIRES_LEADER:
		case IVR_EVENT_FIRST_TO_JOIN:
		case IVR_EVENT_MENU_INVITE:
		case IVR_EVENT_MENU_VOTING:
		case IVR_EVENT_MENU_QA:
		case IVR_EVENT_CHAIR_DROPPED:
		case IVR_EVENT_SELF_MUTE:
		case IVR_EVENT_MENU_SIMPLE_2:
		case IVR_EVENT_MENU_LEADER_2:
		case IVR_EVENT_MAX_PARTICIPANTS:
		case IVR_EVENT_MUTE_ALL_ON:
		case IVR_EVENT_MUTE_ALL_OFF:
		case IVR_EVENT_RECORDING_IN_PROGRESS:
		case IVR_EVENT_RECORDING_FAILED:
		case IVR_EVENT_SELF_UNMUTE:
		case IVR_EVENT_INVITE_CALL:
		case IVR_EVENT_ENTER_DEST_NUM:
		case IVR_EVENT_ILLEGAL_DEST_NUM:
		case IVR_EVENT_PLAY_DIAL_TONE:
		case IVR_EVENT_PLAY_RINGING_TONE:
		case IVR_EVENT_NO_VIDEO_RESOURCES:
		case IVR_EVENT_BLIP_ON_CASCADE_LINK:
		case IVR_EVENT_INVITE_PARTY:
		case IVR_EVENT_REINVITE_PARTY:
		case IVR_EVENT_PLAY_BUSY_MSG:
		case IVR_EVENT_PLAY_NOANSWER_MSG:
		case IVR_EVENT_PLAY_WRONG_NUMBER_MSG:
		{
			return("gen");
		}
// ROLL CALL events
		case IVR_EVENT_ROLLCALL_REC:
		case IVR_EVENT_ROLLCALL_VERIFY_REC:
		case IVR_EVENT_ROLLCALL_CONFIRM_REC:
		case IVR_EVENT_ROLLCALL_ENTER:
		case IVR_EVENT_ROLLCALL_EXIT:
		case IVR_EVENT_ROLLCALL_BEGIN_OF_NAME_REVIEW:
		case IVR_EVENT_ROLLCALL_END_OF_NAME_REVIEW:
// End of ROLL CALL events
		{
			return("roll");
		}
		case IVR_EVENT_GET_NUMERIC_ID:
		case IVR_EVENT_NUMERIC_ID_RETRY:
		{
			return("numid");
		}
		case IVR_EVENT_NOISY_LINE_HELP_MENU:
		{
			return("menu");
		}
		case IVR_EVENT_NOISY_LINE_MUTE:
		{
			return("mute");
		}
		case IVR_EVENT_NOISY_LINE_UNMUTE:
		{
			return("unmute");
		}
		case IVR_EVENT_NOISY_LINE_UNMUTE_MESSAGE:
		{
			return("unmutems");
		}
		case IVR_EVENT_NOISY_LINE_ADJUST:
		{
			return("adjust");
		}
		case IVR_EVENT_NOISY_LINE_DISABLE:
		{
			return("disable");
		}
		case IVR_EVENT_PLAY_NOISY_LINE_MESSAGE:
		{
			return("noisy");
		}
		// recording
		case IVR_EVENT_REC_STARTED:
		case IVR_EVENT_REC_STOPPED:
		case IVR_EVENT_REC_PAUSED:
		case IVR_EVENT_REC_RESUMED:
		case IVR_EVENT_REC_ILLEGAL_ACCOUNT:
		case IVR_EVENT_REC_UNAUTHORIZE:
		case IVR_EVENT_REC_GENERAL:
		case IVR_EVENT_REC_USER_OVERFLOW:
		{
			return("rec");
		}
		// playback
		case IVR_EVENT_PLCK_SESSIONID:
		case IVR_EVENT_PLCK_SESSIONID_ERROR:
		case IVR_EVENT_PLCK_SESSION_END:
		case IVR_EVENT_PLCK_MENU:
		case IVR_EVENT_PLCK_MENU_NOTIFY:
		case IVR_EVENT_PLCK_PAUSED:
		case IVR_EVENT_PLCK_RESUME:
		{
			return("plybck");
		}

		default: {
			status = STATUS_ILLEGAL;
			return("");
		}
	} // switch
}

//--------------------------------------------------------------------------
BYTE IsValidASCII(const char* buffToCheck, WORD buffLen, const char* validate_set, BYTE check_null_end)
{
	BYTE isLegal = YES;

	if (buffToCheck == NULL || buffToCheck[0] == '\0' || buffLen == 0)
	{
		FPTRACE(eLevelError, "IsValidASCII - Failed, The buffer to check is empty");
		return NO;
	}

	BYTE faulty_char       = 0; // initiated with 0 - valid char
	int  faulty_char_index = -1;
	BYTE null_end          = NO;
	BOOL IsCheckSipChars   = TRUE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("CHECK_SIP_CHARS_IN_CONF_NAME", IsCheckSipChars);

	for (WORD char_index = 0; char_index < buffLen; char_index++)
	{
		if (buffToCheck[char_index] == 0)
		{
			null_end = YES;
			break;
		}

		if (strncmp(validate_set, "ASCII_PRINTABLE", strlen("ASCII_PRINTABLE")) == 0)
		{
			// 'legal' ascii printable values: 0, 32-126
			if (buffToCheck[char_index] < 32 || buffToCheck[char_index] > 126)
			{
				isLegal           = NO;
				faulty_char       = buffToCheck[char_index];
				faulty_char_index = char_index;
				break;
			}
		}
		else if (strncmp(validate_set, "CONF_NAME", strlen("CONF_NAME")) == 0)
		{
			if (!((buffToCheck[char_index] > 47 && buffToCheck[char_index] < 58)
			    || (buffToCheck[char_index] > 64 && buffToCheck[char_index] < 91)
			    || (buffToCheck[char_index] > 96 && buffToCheck[char_index] < 123)
			    || buffToCheck[char_index] == ' ' || buffToCheck[char_index] == '_'
			    || buffToCheck[char_index] == '-' || buffToCheck[char_index] == '@'
			    || buffToCheck[char_index] == '(' || buffToCheck[char_index] == ')')
			    || (IsCheckSipChars && strchr(BAD_CHARACTERS_FOR_SIP_URI, buffToCheck[char_index])))
			{
				FPTRACE2INT(eLevelInfoNormal, "IsValidASCII - Failed, Illegal character found in position ", char_index);

				isLegal           = NO;
				faulty_char       = buffToCheck[char_index];
				faulty_char_index = char_index;
				break;
			}
		}
		else if (strncmp(validate_set, "ASCII_NUMERIC", strlen("ASCII_NUMERIC")) == 0)
		{
			if (buffToCheck[char_index] < 48 || buffToCheck[char_index] > 57)
			{
				isLegal           = NO;
				faulty_char       = buffToCheck[char_index];
				faulty_char_index = char_index;
				break;
			}
		}
		else   // default: all ASCII table legal
		{
			if (buffToCheck[char_index] > 126)
			{
				isLegal           = NO;
				faulty_char       = buffToCheck[char_index];
				faulty_char_index = char_index;
				break;
			}
		}
	}

	if (check_null_end == YES && null_end == NO && isLegal == YES)
	{
		FPTRACE(eLevelError, "IsValidASCII - Failed, The buffer is not NULL terminated");
		isLegal           = NO;
		faulty_char       = buffToCheck[buffLen-1];
		faulty_char_index = buffLen-1;
	}

	if (isLegal == NO)
	{
		CSmallString sstr;
		sstr << validate_set << " faulty_char_index = " << faulty_char_index << " ,faulty_char= " << (WORD)faulty_char;
		FPTRACE2(eLevelError, "IsValidASCII - Failed, ", sstr.GetString());
	}

	return isLegal;
}

//--------------------------------------------------------------------------
BYTE IsValidStringUTF8(const char* string_to_check, const char* validate_set, BYTE print_error_status)
{
	BYTE isLegal = YES;

	if (string_to_check == NULL)
	{
		isLegal = NO;
		if (print_error_status)
		{
			FPTRACE(eLevelInfoNormal, "IsValidStringUTF8 - Failed, The buffer to check is empty");
		}
		return isLegal;
	}

	COstrStream statusString;
	BYTE isValidUtf8 = CIConv::ValidateStringEncoding(string_to_check, MCMS_INTERNAL_STRING_ENCODE_TYPE, statusString);
	if (!isValidUtf8)
	{
		isLegal = NO;
		if (print_error_status)
		{
			FPTRACE2(eLevelInfoNormal, "IsValidStringUTF8 - Failed, status: ", statusString.str().c_str());
			FPTRACE2(eLevelInfoNormal, "IsValidStringUTF8 - Failed, string: ", string_to_check);
		}
	}

	if (strncmp(validate_set, "NOT_EMPTY", strlen("NOT_EMPTY")) == 0)
	{
		if (string_to_check[0] == '\0')
		{
			isLegal = NO;
			if (print_error_status)
			{
				FPTRACE(eLevelInfoNormal, "IsValidStringUTF8 - Failed, empty string");
			}
		}
	}

	return isLegal;
}

//--------------------------------------------------------------------------
BYTE IsNumeric(const char* string)
{
	if (string)
	{
		int length = strlen(string);
		if (length)
		{
			for (int i = 0; i < length; i++)
				if (!isdigit(string[i]))
					return FALSE;

			// All characters are digits
			return TRUE;
		}
	}
	return FALSE;
}

//--------------------------------------------------------------------------
WORD Get261VideoCardMPI(DWORD bitRate, APIS8* qcifMpi, APIS8* cifMpi, eVideoQuality videoQuality)
{
	if (bitRate >= 56 && bitRate <= 8192)
	{
		*qcifMpi = 1; // 30 FPS
		*cifMpi  = 1;
	}
	else
	{
		FPASSERT(bitRate);
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
WORD Get263VideoCardMPI(DWORD bitRate, BYTE* bVF)
{
	if (!bVF)
		return STATUS_ILLEGAL;

	memset(bVF, -1, sizeof(bVF));
	WORD lMPI = 0;
	CCapSetInfo lCapInfo(eH263CapCode);

	if (bitRate >= 56 && bitRate <= 320)
	{
		lMPI = lCapInfo.TranslateIsdnMpiToIp(MPI_2);
		bVF[H263_QCIF_SQCIF] = lMPI;
		bVF[H263_CIF]        = lMPI;
	}
	else if (bitRate >= 384 && bitRate <= 8192)
	{
		lMPI = lCapInfo.TranslateIsdnMpiToIp(MPI_1);
		bVF[H263_QCIF_SQCIF] = lMPI;
		bVF[H263_CIF]        = lMPI;
	}
	else
	{
		FPASSERT(1);
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
const char* Geth264VideoModeTypeAsString(Eh264VideoModeType VideoMode)
{
	switch (VideoMode)
	{
		case eCIF30               : return "CIF30";
		case eCIF60               : return "eCIF60";
		case e2CIF30              : return "2CIF30";
		case eWCIF60              : return "eWCIF60";
		case eSD15                : return "SD15";
		case eSD30                : return "SD30";
		case eW4CIF30             : return "eW4CIF30";
		case eHD720Asymmetric     : return "HD720Asymmetric";
		case eSD60                : return "eSD60";
		case eHD720Symmetric      : return "HD720Symmetric";
		case eHD720At60Asymmetric : return "eHD720At60Asymmetric";
		case eHD1080Asymmetric    : return "HD1080Asymmetric";
		case eHD720At60Symmetric  : return "HD720At60Symmetric";
		case eHD1080Symmetric     : return "HD1080Symmetric";
		case eHD1080At60Asymmetric: return "HD1080At60Asymmetric";
		case eHD1080At60Symmetric : return "HD1080At60Symmetric";
		case eLasth264VideoMode   : return "Last";
		case eInvalidModeType     : return "Invalid";
		default                   : return "Unknown";
	} // switch
}

//--------------------------------------------------------------------------
Eh264VideoModeType GetMaxVideoModeBySysCfg() // from SRS V7: the MAX_CP_RESOLUTION flag will be part of the Resolution Slider
{
	eSystemCardsMode   systemCardsBasedMode = GetSystemCardsBasedMode();
	Eh264VideoModeType maxVideoMode = CResRsrcCalculator::GetMaxCPResolution(systemCardsBasedMode);
	FTRACEINTO << "GetMaxVideoModeBySysCfg: call to CResRsrcCalculator::GetMaxCPResolution = " << Geth264VideoModeTypeAsString(maxVideoMode);
	return 	maxVideoMode;
}

//--------------------------------------------------------------------------
Eh264VideoModeType TranslateCPVideoPartyTypeToMaxH264VideoModeType(eVideoPartyType videoPartyType)
{
	Eh264VideoModeType videoModeType = eInvalidModeType;

	switch (videoPartyType)
	{
		case eCP_VP8_upto_CIF_video_party_type:
		case eCP_H264_upto_CIF_video_party_type:
		case eCP_H261_H263_upto_CIF_video_party_type:
		{
			videoModeType = eCIF30;
			break;
		}
		case eCP_VP8_upto_SD30_video_party_type:
		case eCP_H264_upto_SD30_video_party_type:
		case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
		{
			videoModeType = eW4CIF30; // W4CIF30 mode was entered in V4.1, it consults the same resources as SD30 but it is greater than SD30-->It is the max
			break;
		}
		case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
		{
			videoModeType = eHD720Symmetric;
			break;
		}
		case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
		{
			videoModeType = eHD1080Symmetric;
			break;
		}
		case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
		{
			videoModeType = eHD720At60Symmetric;
			break;
		}
		case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:
		{
			videoModeType = eHD1080At60Asymmetric;
			break;
		}
		case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
		{
			videoModeType = eHD1080At60Symmetric;
			break;
		}
		default:
		{
			FPTRACE2INT(eLevelError, "TranslateCPVideoPartyTypeToMaxH264VideoModeType - Failed, Invalid videoPartyType:", videoPartyType);
		}
	} // switch

	FTRACEINTO << "TranslateCPVideoPartyTypeToMaxH264VideoModeType - videoModeType:" << Geth264VideoModeTypeAsString(videoModeType);
	return videoModeType;
}

//--------------------------------------------------------------------------
void TraceH264DecisionMatrix(H264VideoModeDetails& h264VidModeDetails, DWORD callRate, eVideoQuality videoQuality, Eh264VideoModeType resourceMaxVideoMode, Eh264VideoModeType sysMaxVideoMode)
{
  std::ostringstream msg;
  msg << "TraceH264DecisionMatrix:"
      << "\n  callRate             :" << callRate
      << "\n  sysMaxVideoMode      :" << Geth264VideoModeTypeAsString(sysMaxVideoMode)
      << "\n  resourceMaxVideoMode :" << Geth264VideoModeTypeAsString(resourceMaxVideoMode)
      << "\n  videoQuality         :";

	switch (videoQuality)
	{
		case eVideoQualityAuto     : { msg << "Auto"; break; }
		case eVideoQualityMotion   : { msg << "Motion"; break; }
		case eVideoQualitySharpness: { msg << "Sharpness"; break; }
		default                    : { msg << "Unknown (" << videoQuality << ")"; break; }
	}

	msg << "\n  h264VidModeDetails   :"
	    << " video_mode_type:" << Geth264VideoModeTypeAsString(h264VidModeDetails.videoModeType)
	    << ", profile:"        << h264VidModeDetails.profileValue
	    << ", level:"          << h264VidModeDetails.levelValue
	    << ", maxMBPS:"        << h264VidModeDetails.maxMBPS
	    << ", maxFS:"          << h264VidModeDetails.maxFS
	    << ", maxDPB:"         << h264VidModeDetails.maxDPB
	    << ", maxBR:"          << h264VidModeDetails.maxBR
	    << ", maxCPB:"         << h264VidModeDetails.maxCPB;

	FTRACEINTO << msg.str().c_str();
}

//--------------------------------------------------------------------------
void GetRtvVideoParams(RTVVideoModeDetails& rtvVidModeDetails, DWORD callRate, eVideoQuality videoQuality, Eh264VideoModeType resourceMaxVideoMode)
{
	rtvVidModeDetails.videoModeType = eLastRtvVideoMode;
	rtvVidModeDetails.Width         = 0;
	rtvVidModeDetails.Height        = 0;
	rtvVidModeDetails.FR            = 0;

	H264VideoModeDetails h264VidModeDetails;

	GetH264VideoParams(h264VidModeDetails, callRate, eVideoQualitySharpness, resourceMaxVideoMode);

	CRtvVideoMode::GetRtvVideoParams(rtvVidModeDetails, h264VidModeDetails.videoModeType);
}

//--------------------------------------------------------------------------
void GetH264VideoParams(H264VideoModeDetails& h264VidModeDetails, DWORD callRate, eVideoQuality videoQuality, Eh264VideoModeType resourceMaxVideoMode, BOOL isHighProfile)
{
	h264VidModeDetails.profileValue  = H264_Profile_BaseLine;
	h264VidModeDetails.levelValue    = -1;
	h264VidModeDetails.maxMBPS       = -1;
	h264VidModeDetails.maxFS         = -1;
	h264VidModeDetails.maxDPB        = -1;
	h264VidModeDetails.maxBR         = -1;
	h264VidModeDetails.maxCPB        = -1;
	h264VidModeDetails.maxStaticMbps = -1;

	if (resourceMaxVideoMode == eHD1080At60Asymmetric ||
		resourceMaxVideoMode == eHD1080At60Symmetric)
	{
		BOOL Is1080p60 = TRUE;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("1080_60_FPS", Is1080p60);
		if (FALSE == Is1080p60)
		{
			resourceMaxVideoMode = eHD1080Symmetric;
			FTRACEINTO << "GetH264VideoParams - 1080p60 is disabled by system configuration, The resource max video mode will be " << Geth264VideoModeTypeAsString(resourceMaxVideoMode);
		}
	}

	Eh264VideoModeType sysMaxVideoMode = GetMaxVideoModeBySysCfg(); // max video mode according to system.cfg (from V7 - it's according to resolution Slider)
	Eh264VideoModeType maxVideoMode    = sysMaxVideoMode;
	if (sysMaxVideoMode > resourceMaxVideoMode)
		maxVideoMode = resourceMaxVideoMode;

	CH264VideoMode* pH264VidMode = new CH264VideoMode();
	pH264VidMode->GetH264VideoParams(h264VidModeDetails, callRate, videoQuality, maxVideoMode, isHighProfile);
	TraceH264DecisionMatrix(h264VidModeDetails, callRate, videoQuality, resourceMaxVideoMode, sysMaxVideoMode);
	POBJDELETE(pH264VidMode);
}

//--------------------------------------------------------------------------
void GetH264AssymetricVideoParams(H264VideoModeDetails& h264VidModeDetails, DWORD callRate, eVideoQuality videoQuality, Eh264VideoModeType resourceMaxVideoMode, BOOL isHighProfile /* = TRUE*/)
{
	h264VidModeDetails.profileValue  = H264_Profile_BaseLine;
	h264VidModeDetails.levelValue    = -1;
	h264VidModeDetails.maxMBPS       = -1;
	h264VidModeDetails.maxFS         = -1;
	h264VidModeDetails.maxDPB        = -1;
	h264VidModeDetails.maxBR         = -1;
	h264VidModeDetails.maxCPB        = -1;
	h264VidModeDetails.maxStaticMbps = -1;

	if (resourceMaxVideoMode == eHD1080At60Asymmetric ||
		resourceMaxVideoMode == eHD1080At60Symmetric)
	{
		BOOL Is1080p60 = TRUE;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("1080_60_FPS", Is1080p60);
		if (FALSE == Is1080p60)
		{
			resourceMaxVideoMode = eHD1080Symmetric;
			FTRACEINTO << "GetH264AssymetricVideoParams - 1080p60 is disabled by system configuration, The resource max video mode will be " << Geth264VideoModeTypeAsString(resourceMaxVideoMode);
		}
	}

	Eh264VideoModeType sysMaxVideoMode = GetMaxVideoModeBySysCfg(); // max video mode according to system.cfg (from V7 - it's according to resolution Slider)

	Eh264VideoModeType maxVideoMode = sysMaxVideoMode;
	if (sysMaxVideoMode > resourceMaxVideoMode)
		maxVideoMode = resourceMaxVideoMode;

	CH264VideoMode* pH264VidMode = new CH264VideoMode();
	pH264VidMode->GetH264AssymetricVideoParams(h264VidModeDetails, callRate, videoQuality, maxVideoMode, isHighProfile);
	TraceH264DecisionMatrix(h264VidModeDetails, callRate, videoQuality, resourceMaxVideoMode, sysMaxVideoMode);
	POBJDELETE(pH264VidMode);
}

//--------------------------------------------------------------------------
WORD GetH264VideoHdVswParam(int& supportedProfile, int& supportedLevel, long& customMbps, long& customFS, long& customDpb, long& customBrAndCpb, long& customSar, long& staticMB, BYTE HDResolution)
{
	WORD retStatus = STATUS_OK;
	switch (HDResolution)
	{
		case eHD1080p60Res:
		{
			supportedProfile = H264_Profile_BaseLine;
			supportedLevel = H264_Level_3_1;
			customMbps = GetMaxMbpsAsDevision(H264_HD1080_60_VSW_MBPS);
			customFS = GetMaxFsAsDevision(H264_HD1080_FS);
			customDpb = -1;
			customBrAndCpb = -1;
			customSar = -1;
			staticMB = -1;
			break;
		}
		case eHD720Res:
		{
			supportedProfile = H264_Profile_BaseLine;
			supportedLevel   = H264_Level_3_1;
			customMbps       = -1;
			customFS         = GetMaxFsAsDevision(H264_HD720_FS); // -1;
			customDpb        = -1;
			customBrAndCpb   = -1;
			customSar        = -1;
			staticMB         = -1;
			break;
		}
		case eHD1080Res:
		{
			supportedProfile = H264_Profile_BaseLine;
			supportedLevel   = H264_Level_3_1;
			customMbps       = GetMaxMbpsAsDevision(H264_HD1080_30_MBPS);
			customFS         = GetMaxFsAsDevision(H264_HD1080_FS);
			customDpb        = -1;
			customBrAndCpb   = -1;
			customSar        = -1;
			staticMB         = -1;
			break;
		}
		case eHD720p60Res:
		{
			supportedProfile = H264_Profile_BaseLine;
			supportedLevel   = H264_Level_3_1;
			customMbps       = GetMaxMbpsAsDevision(H264_HD720_60_MBPS);
			customFS         = GetMaxFsAsDevision(H264_HD720_FS);
			customDpb        = -1;
			customBrAndCpb   = -1;
			customSar        = -1;
			staticMB         = -1;
			break;
		}
		case eSDRes:
		{
			supportedProfile = H264_Profile_BaseLine;
			supportedLevel   = H264_Level_2_2;
			customMbps       = GetMaxMbpsAsDevision(H264_L3_DEFAULT_MBPS);
			customFS         = -1;
			customDpb        = -1;
			customBrAndCpb   = -1;
			customSar        = -1;
			staticMB         = -1;
			break;
		}
		case eH264Res:
		{
			supportedProfile = H264_Profile_BaseLine;
			supportedLevel   = H264_Level_2;
			customMbps       = -1;
			customFS         = -1;
			customDpb        = -1;
			customBrAndCpb   = -1;
			customSar        = -1;
			staticMB         = -1;
			break;
		}
		default:
		{
			FPTRACE2INT(eLevelError, "GetH264VideoHdVswParam - Failed, Invalid HDResolution:", HDResolution);
			retStatus = STATUS_OUT_OF_RANGE;
		}
	} // switch

	return retStatus;
}

//--------------------------------------------------------------------------
void GetH264VideoTransmitParamForAsymmetricModes(int& supportedLevel, long& customMbps, long& customFS, long& customDpb, long& customBrAndCpb, Eh264VideoModeType h264VidModeType)
{
	H264VideoModeDetails h264VidModeDetails;
	h264VidModeDetails.levelValue = -1;
	h264VidModeDetails.maxMBPS    = -1;
	h264VidModeDetails.maxFS      = -1;
	h264VidModeDetails.maxDPB     = -1;
	h264VidModeDetails.maxBR      = -1;
	h264VidModeDetails.maxCPB     = -1;
// h264VidModeDetails.maxStaticMbps = -1; ISDN only not needed

	CH264VideoMode* pH264VidMode = new CH264VideoMode();
	pH264VidMode->GetH264AsymmetricTransmitVideoModeDetailsAccordingToType(h264VidModeDetails, h264VidModeType);
	supportedLevel = h264VidModeDetails.levelValue;
	customMbps     = h264VidModeDetails.maxMBPS;
	customFS       = h264VidModeDetails.maxFS;
	customDpb      = h264VidModeDetails.maxDPB;
	customBrAndCpb = h264VidModeDetails.maxBR;
	POBJDELETE(pH264VidMode);
}

//--------------------------------------------------------------------------
// should return 14
WORD GetMinimumHd720Fs()
{
	CH264Details thisH264Details = H264_HD720_LEVEL;
	DWORD        multFs          = thisH264Details.GetDefaultFsAsProduct();
	WORD         fsRoundedDown   = multFs / CUSTOM_MAX_FS_FACTOR;
	return fsRoundedDown;
}

//--------------------------------------------------------------------------
WORD GetMinimumHd720At15Mbps()
{
	return (H264_HD720_15_MBPS / CUSTOM_MAX_MBPS_FACTOR);
}

//--------------------------------------------------------------------------
WORD GetMinimumHd720At5Mbps()
{
	return (H264_HD720_5_MBPS / CUSTOM_MAX_MBPS_FACTOR);
}

//--------------------------------------------------------------------------
WORD GetMinimumHd1080Fs()
{
	return (H264_HD1080_FS / CUSTOM_MAX_FS_FACTOR);
}

//--------------------------------------------------------------------------
extern WORD GetMinimumHd1080At15Mbps()
{
	return ((H264_HD1080_FS*15) / CUSTOM_MAX_MBPS_FACTOR);
}

//--------------------------------------------------------------------------
extern WORD GetMinimumHd720At50Mbps()
{
	return (H264_HD720_50_MBPS / CUSTOM_MAX_MBPS_FACTOR);
}

//--------------------------------------------------------------------------
extern WORD GetMinimumHd1080At60Mbps()
{
	return (H264_HD1080_60_MBPS / CUSTOM_MAX_MBPS_FACTOR);
}

//--------------------------------------------------------------------------
extern WORD GetMinimumHd720At30Mbps()
{
	return (H264_HD720_30_MBPS / CUSTOM_MAX_MBPS_FACTOR);
}

//--------------------------------------------------------------------------
// IP SECTION
// Adds bch (returns TDM rate)
// Rate in 100 bit per sec ((100bit)/sec)
// in default round up
DWORD CalculateTdmRate(DWORD videoRate, BYTE bRoundDown)
{
	DWORD tdmRate = videoRate * 100;
	if (bRoundDown == FALSE)
	{
		tdmRate = (tdmRate % 96 ?  tdmRate/96 + 1 :  tdmRate /96); // rounded up
		DWORD div = tdmRate / 8;
		if (div * 8 < tdmRate)                                     // bitRate must be divided by eight:
			tdmRate = (div+1)*8;
	}
	else
	{
		tdmRate = tdmRate / 96; // rounded down
		DWORD div = tdmRate / 8;
		tdmRate = div * 8; // bitRate must be divided by eight:
	}

	return tdmRate;
}

//--------------------------------------------------------------------------
DWORD CalculateAudioRate(DWORD call_rate)
{
	DWORD audioRate = rate48K;
	if (call_rate < rate128K)
		audioRate = rate8K;
	else if (call_rate < rate256K)
		audioRate = rate32K;
	else if (call_rate < rate384K)
		audioRate = rate48K;
	else
		audioRate = rate64K;
	return audioRate;
}

//--------------------------------------------------------------------------
CapEnum CalculateAudioOpusCapEnum(DWORD audioRate)
{
	if (audioRate >= rate128K)
		return eOpusStereo_CapCode;

	return eOpus_CapCode;
}

//--------------------------------------------------------------------------
DWORD CalculateAudioRateAccordingToVideoRateOfCopLevel(DWORD videorate)
{
	DWORD audioRate = 48;
	if (videorate < 96)
		audioRate = 8;
	else if (videorate < 208)
		audioRate = 32;
	else if (videorate < 320)
		audioRate = 48;
	else
		audioRate = 64;

	return audioRate;
}

//--------------------------------------------------------------------------
DWORD ChangeVideoRateAccordingToPartyTypeIfNeeded(eVideoPartyType videoPartyType, DWORD videoRate)
{
	DWORD nVidBitRateDefault  = 0;

	FTRACEINTO << "videoPartyType " << VideoPartyTypeToString(videoPartyType) << " videoRate " << videoRate;

	if( ( eCP_H264_upto_CIF_video_party_type <= videoPartyType ) &&
		( eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type >= videoPartyType ) )
	{
		nVidBitRateDefault =  min((int)videoRate, 10240);
	}
	else if( ( eCP_H264_upto_HD720_30FS_Symmetric_video_party_type == videoPartyType ) )
	{
		nVidBitRateDefault =  min((int)videoRate, 40960);
	}
	else if( ( eCP_H264_upto_HD720_60FS_Symmetric_video_party_type <= videoPartyType ) &&
		(eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type >= videoPartyType ) )
	{
		nVidBitRateDefault =  min((int)videoRate, 61440);
	}
	else
	{
		FTRACEINTO << "unsupported partyType - " << videoPartyType;
	}

	return nVidBitRateDefault;
}

// Description: after the parameters set calculate the rate
// Warning: Calculate now only for Ip only conferences!!!
void CalculateIpRate(CIpComMode* pIpPartyScm, DWORD& confRate, DWORD& videoRate, DWORD audioRate, DWORD reservationRate, BOOL bIsEncryption)
{
	DWORD decisionRate = 0;

	if (reservationRate != 0xFFFFFFFF)
	{
		reservationRate = reservationRate * 1000; // party reservation video bitrate is in 100 bits/sec.
		decisionRate    = (reservationRate % rate64K) ? (reservationRate/rate64K + 1) * rate64K : (reservationRate/rate64K) * rate64K;
	}
	else
		decisionRate = confRate;

	// Continuous Presence
	// if (pIpPartyScm->GetConfType() == kCp)
	// {
	// in CP conference, if auto audio mode was selected in conf. reservation
	// audio bitrate is zero at this point , because we don't know which
	// audio algorithm will be selected (depends on remote capabilities, we don't
	// know at this point).

	// We need a temp value - in case we have 0xFFFFFFFF we don't want to change it's value.
	if (audioRate == 0)
	{
		audioRate = CalculateAudioRate(decisionRate);
	}

	if (pIpPartyScm->GetConfType() != kCp && pIpPartyScm->GetConfType() != kCop)   // vsw auto, vsw fixed
	{
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		std::string key1      = CFG_KEY_MIX_LINK_ENVIRONMENT;
		std::string key2      = CFG_KEY_IP_LINK_ENVIRONMENT;
		BOOL        bMixLinkEnv, bIpLinkEenv;
		sysConfig->GetBOOLDataByKey(key1, bMixLinkEnv);
		sysConfig->GetBOOLDataByKey(key2, bIpLinkEenv);

		// if vsw and we are in mix environment (such as RMX and MGC) and conf rate is 1920
		// video rate must be reduced to 1787900
		if (confRate == E1Rate && bMixLinkEnv == YES)
		{
			if (bIsEncryption)
				videoRate = MaxE1MixWithEncryptionLinkEnvRate;
			else
				videoRate = MaxE1MixLinkEnvRate;

			FPTRACE2INT(eLevelInfoNormal, "CalculateIpRate - Adjusting video rate to ", videoRate);
		}
		else if (confRate == E1Rate && bIpLinkEenv == YES)
		{
			videoRate = MaxE1Rate;
			FPTRACE2INT(eLevelInfoNormal, "CalculateIpRate - Adjusting video rate to ", videoRate);
		}
		else
		{
			videoRate = confRate - audioRate;
			confRate  = videoRate + audioRate;
		}
	}

	else
	{
		if (reservationRate == 0xFFFFFFFF)
			// if auto video BitRate was selected for H323 Party in CP conference,
			// we set H323VideoRate according to conference rate reservation
			videoRate = confRate /*- audioRate*/;
		else
			videoRate = reservationRate;

		// Bridge_10000
		eVideoPartyType transmitPartyType = pIpPartyScm->GetVideoPartyType(cmCapTransmit);
		eVideoPartyType receivePartyType = pIpPartyScm->GetVideoPartyType(cmCapReceive);
		eVideoPartyType maxPartyType = max(transmitPartyType, receivePartyType);

		DWORD changedRate = ChangeVideoRateAccordingToPartyTypeIfNeeded(maxPartyType, videoRate/100);

		if (changedRate)
			videoRate = changedRate * 100;

		if ((videoRate > MaxCPRate) && (pIpPartyScm->GetConfType() == kCp))
			videoRate = MaxCPRate;

	}
}

//--------------------------------------------------------------------------
// Description: Calculate all the rates for H323 conference
// Warning: Calculate now only for Ip only conferences!!!
int CalculateRateForIpCalls(CConfParty* pConfParty, CIpComMode* pIpPartyScm, DWORD& confRate, DWORD& videoRate, BYTE bIsEncryption)
{
	if ( !pConfParty || !pIpPartyScm)
		return -1;

	// Encryption rate calculation is with no change. Any special rate calculation for H320 parties should be done by the H320 parties.

	// Audio Only
	WORD bIsAudioOnly = pConfParty->GetVoice();
	if (bIsAudioOnly)
	{
		videoRate = 0;
		confRate  = rate64K; // aud_bitrate today audio only is just 64k maybe it will change in the future;
		return 0;
	}

	confRate = pIpPartyScm->GetCallRate() * 1000;
	FPTRACE2INT(eLevelInfoNormal,"CalculateRateForIpCalls - confRate: ", confRate);

	DWORD audioRate = pIpPartyScm->GetMediaBitRate(cmCapAudio);
	audioRate *= 1000;
	DWORD reservationRate = pConfParty->GetVideoRate();

	//N.A. DEBUG VP8
	FPTRACE2INT(eLevelInfoNormal,"CalculateRateForIpCalls - audioRate: ", audioRate);
	FPTRACE2INT(eLevelInfoNormal,"CalculateRateForIpCalls - reservationRate: ", reservationRate);

	CalculateIpRate(pIpPartyScm, confRate, videoRate, audioRate, reservationRate, bIsEncryption);

	return 0;
}

//--------------------------------------------------------------------------
// Description: Calculate all the rates for H323 conference
// Warning: Calculate now only for Ip only conferences!!!
int CalculateRateForSipOptions(CConfParty* pConfParty, CIpComMode* pIpPartyScm, DWORD& confRate, DWORD& videoRate, CCommConf* pCommConf)
{
	if (!pCommConf || !pIpPartyScm)
		return -1;

	confRate = pIpPartyScm->GetCallRate() * 1000;
	DWORD audioRate = pIpPartyScm->GetMediaBitRate(cmCapAudio);
	audioRate *= 1000;
	DWORD reservationRate = 0xFFFFFFFF;

	if (pConfParty == NULL)
		CalculateIpRate(pIpPartyScm, confRate, videoRate, audioRate, reservationRate, pCommConf->GetIsEncryption());
	else
	{
		// Audio Only
		WORD bIsAudioOnly = pConfParty->GetVoice();
		if (bIsAudioOnly)
		{
			videoRate = 0;
			confRate  = rate64K; // aud_bitrate today audio only is just 64k maybe it will change in the future;
			return 0;
		}

		CalculateRateForIpCalls(pConfParty, pIpPartyScm, confRate, videoRate, pCommConf->GetIsEncryption());
	}

	return 0;
}

//--------------------------------------------------------------------------
int ReCalculateRateForIpCpDialInCalls(CConfParty* pConfParty, CIpComMode* pPartyScm, BYTE networkType, DWORD setupRate, DWORD& confRate, DWORD& videoRate, BYTE audZeroRate)
{
	if (!pConfParty || !pPartyScm)
		return -1;

	// in order not to allocate too high resources:
	confRate = setupRate;

	DWORD decisionRate         = 0;
	DWORD videoReservationRate = pConfParty->GetVideoRate();
	if (videoReservationRate != 0xFFFFFFFF)
	{
		videoReservationRate = videoReservationRate * 1000; // party reservation video bitrate is in 100 bits/sec.
		decisionRate         = (videoReservationRate % rate64K) ? (videoReservationRate/rate64K + 1) * rate64K : (videoReservationRate/rate64K) * rate64K;
	}
	else
		decisionRate = confRate;

	DWORD audioRate = 0;
/* 1- set audio rate */
	if (!audZeroRate)
		audioRate = pPartyScm->GetMediaBitRate(cmCapAudio) * 1000;

	if (audioRate == 0)
	{
		audioRate = CalculateAudioRate(decisionRate);
	}

/* 2- set video rate */
	if ((videoReservationRate == 0xFFFFFFFF) || (videoReservationRate + audioRate > confRate))
		// if auto video BitRate was not selected for H323 Party in CP conference,
		// we set H323VideoRate according to conference rate reservation
		videoRate = confRate /*Shmulik - audioRate*/;
	else
		videoRate = videoReservationRate * 1000; // in party reservation video bitrate is in 100 bits/sec.

	if (videoRate > MaxCPRate)
		videoRate = MaxCPRate;

	/*Shmulik confRate = videoRate + audioRate;*/

	return 0;
}

//--------------------------------------------------------------------------
int CalculateReservationRates(CCommConf* pCommConf, DWORD& confRate, DWORD& audRate, DWORD& videoRate)
{
	if (!pCommConf)
		return -1;

	BYTE        reservationRate = pCommConf->GetConfTransferRate();
	CCapSetInfo lCapInfo        = eUnknownAlgorithemCapCode;
	confRate  = lCapInfo.TranslateReservationRateToIpRate(reservationRate);
	confRate *= 1000;

	audRate = CalculateAudioRate(confRate);

	if (pCommConf->GetIsHDVSW())
	{
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		BOOL        bMixLinkEnv, bIpLinkEenv;
		sysConfig->GetBOOLDataByKey(CFG_KEY_MIX_LINK_ENVIRONMENT, bMixLinkEnv);
		sysConfig->GetBOOLDataByKey(CFG_KEY_IP_LINK_ENVIRONMENT, bIpLinkEenv);

		// if vsw and we are in mix enviroment (such as RMX and MGC) and conf rate is 1920
		// video rate must be reduced to 1787900
		if (confRate == E1Rate && bMixLinkEnv == YES)
		{
			if (pCommConf->GetIsEncryption())
				videoRate = MaxE1MixWithEncryptionLinkEnvRate;
			else
				videoRate = MaxE1MixLinkEnvRate;

			FPTRACE2INT(eLevelInfoNormal, "CalculateReservationRates - Adjusting video rate to ", videoRate);
		}
		else if (confRate == E1Rate && bIpLinkEenv == YES)
		{
			videoRate = MaxE1Rate;
			FPTRACE2INT(eLevelInfoNormal, "CalculateReservationRates - Adjusting video rate to ", videoRate);
		}
		else
			videoRate = confRate - rate64K;
	}
	else
		videoRate = confRate - audRate;

	return 0;
}

//--------------------------------------------------------------------------
// Get MAC Address for the allocation of unique number
// Since we have trouble to get the MAC address currently we take the signaling IP address
void getMACaddress(BYTE* theMAC, DWORD signalingIpAddress)
{
	// need to replace it with the write MAC address
	// until Oshi/Anatoly will work with Yoram F. on the Mac Address it will be combination of the
	// MCU IP address and defualt values
	BYTE* pTtempIP;

	pTtempIP  = (BYTE*)&signalingIpAddress;
	theMAC[0] = pTtempIP[0];
	theMAC[1] = pTtempIP[1];
	theMAC[2] = pTtempIP[2];
	theMAC[3] = pTtempIP[3];
	theMAC[4] = 0x34;
	theMAC[5] = 0xef;
}

//--------------------------------------------------------------------------
void DumpUniqueNumberToString(char* uniqueNumber, int size, const char* separatorBefore, const char* separatorAfter, CObjString& str)
{
	for (int i = 0; i < size; i++)
	{
		str << separatorBefore;
		str.SetFormat("%x");
		str << (BYTE)uniqueNumber[i] << separatorAfter;
	}

	str.SetFormat("%d");
}

//--------------------------------------------------------------------------
// This function calculates a Unique number according to the GUID
void CalculateUniqueNumber(char* UniqueNumberString, DWORD signalingIpAddress)
{
	FPTRACE(eLevelInfoNormal,"CalculateUniqueNumber - : ");
	// 0-3 time low (32 bit)
	// 4-5 time mid (16 bit)
	// 6-7 time_hi_and_version. Timestamp multiplexed with the version number and set to 1
	char         UniqueNumber[16];
	static char  oldUniqueNumber[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static DWORD clockSeq            = 0;
	DWORD        date                = 0;
	DWORD        time                = 0;
	DWORD        ticks               = 0;
	static DWORD oldDate             = 0;
	static DWORD oldTime             = 0;
	static DWORD oldTicks            = 0;
	BYTE         macAddress[6];

	memset(UniqueNumber, 0, 16);
	if (clockSeq == 0)
		clockSeq = rand();

	// set time_low, time_mid, time_high_and_version_fields in octect 0-7 (64 bit)
	CStructTm tempT;
	tm        tm_other;
	SystemGetTime(tempT);
	tempT.GetAsTm(tm_other);
	date |= (tm_other.tm_year << 16);
	date |= (tm_other.tm_mon << 8);
	date |= (tm_other.tm_mday);
	time |= (tm_other.tm_hour << 16);
	time |= (tm_other.tm_min << 8);
	time |= (tm_other.tm_sec);
	ticks = SystemGetTickCount().GetIntegerPartForTrace(); // SAGI ????

	memcpy(UniqueNumber, &ticks, 4);
	memcpy(UniqueNumber+4, &time, 4);

	// 4 most significant buts of 6-7 bytes set to version number '0001'
	*((char*)(UniqueNumber+7)) &= 0x1f;
	*((char*)(UniqueNumber+7)) |= 0x10;

	// Increment clock sequence if needed
	if ((date != oldDate) || (time != oldTime) || (ticks != oldTicks) ||
	    ((date == oldDate) && (time == oldTime) && (ticks == oldTicks)))
	{
		clockSeq++;
		clockSeq %= 16384; // 16384 = 0x4000
	}

	// update the old time
	oldDate  = date;
	oldTime  = time;
	oldTicks = ticks;

	// set clock_seq_hi_and_reserved and clock_seq_low fields 8-9
	UniqueNumber[9]  = (BYTE)clockSeq;
	UniqueNumber[8]  = (BYTE)(clockSeq>>8);
	UniqueNumber[8] &= 0xbf;

	// set 10 - 15 to Mac Address
	getMACaddress(macAddress, signalingIpAddress);
	memcpy(UniqueNumber+10, macAddress, 6);

	if (memcmp(UniqueNumber, oldUniqueNumber, 16) == 0)
		FPTRACE(eLevelError, "CalculateUniqueNumber - Failed, Identical unique number");

	memcpy(oldUniqueNumber, UniqueNumber, 16);
	memcpy(UniqueNumberString, UniqueNumber, 16);
}

//--------------------------------------------------------------------------
// This function calculates a Unique number according to the GUID
void AllocateRejectID(DWORD& RejectId)
{
	static DWORD currentRejectId = CONFPARTY_MIN_REJECT_ID;
	RejectId = currentRejectId;
	currentRejectId++;
	if (currentRejectId == DUMMY_PARTY_ID)
		currentRejectId = CONFPARTY_MIN_REJECT_ID;
}

//--------------------------------------------------------------------------
BOOL IsValidRsrcID(DWORD dwRsrcID)
{
	if ((0 == dwRsrcID) || (0xFFFFFFFF == dwRsrcID))
		return FALSE;
	else
		return TRUE;
}

//--------------------------------------------------------------------------
BOOL IsValidInterfaceType(WORD wInterfaceType)
{
	switch (wInterfaceType)
	{
		case ISDN_INTERFACE_TYPE:
		case H323_INTERFACE_TYPE:
		case V35_INTERFACE_TYPE:
		case SIP_INTERFACE_TYPE:
			return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------
BOOL IsValidAudioVolume(DWORD dwVolume)
{
	// audioVolume have to be in range 0..10
	if (/*dwVolume < AUDIO_VOLUME_MIN  ||*/ AUDIO_VOLUME_MAX < dwVolume)
		return FALSE;
	else
		return TRUE;
}

//--------------------------------------------------------------------------
BOOL IsValidNoiseDetectionThreshold(BYTE noiseDetectionThreshold)
{
	// noiseDetectionThreshold have to be in range 0..10
	if (noiseDetectionThreshold < E_NOISE_DETECTION_THRESHOLD_1 || noiseDetectionThreshold > E_NOISE_DETECTION_THRESHOLD_10)
		return FALSE;
	else
		return TRUE;
}

//--------------------------------------------------------------------------
BOOL IsValidAudioSampleRate(BYTE bAudioSampleRate)
{
	switch (bAudioSampleRate)
	{
		case AUDIO_SAMPLE_RATE_8KHZ:
		case AUDIO_SAMPLE_RATE_16KHZ:
		case AUDIO_SAMPLE_RATE_32KHZ:
		case AUDIO_SAMPLE_RATE_48KHZ:
			return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------------
BOOL IsValidOpusCodecBitRate(DWORD maxAverageBitrate)
{

//	if (maxAverageBitrate < MIN_OPUS_AVERAGE_BITRATE || maxAverageBitrate > MAX_OPUS_AVERAGE_BITRATE)
//	{
//		//TRACEINTO << " Invalid Opus BitRate: " << maxAverageBitrate;
//		return FALSE;
//	}
	return TRUE;
}

//--------------------------------------------------------------------------
BOOL IsValidMsftClientType(MSFT_CLIENT_ENUM eMsftClientType)
{
	switch (eMsftClientType)
	{
		case MSFT_CLIENT_DUMMY:
		case MSFT_CLIENT_MOC:
		case MSFT_CLIENT_LYNC:
		case MSFT_CLIENT_AVMCU:
		case MSFT_CLIENT_LYNC2013:
		case MSFT_CLIENT_AVMCU2013:
		case MSFT_CLIENT_NONE_MSFT:
		case MSFT_CLIENT_LAST:
			return TRUE;
	}

	return FALSE;
}


//--------------------------------------------------------------------------
BOOL IsSendVinEnabledInMCU()
{
	BOOL bIsFECC;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("FECC", bIsFECC);

	return bIsFECC;
}

//--------------------------------------------------------------------------
ESTATUS TestEncryMoveValidity(BYTE partyEncryVal, BOOL bIsDefinedParty, BYTE destConfEncryType)
{
	ESTATUS status =  STATUS_OK;
	switch (destConfEncryType)
	{
		case eEncryptAll:
		{
			if (partyEncryVal != YES)
				status = STATUS_MOVE_ENC_NON_ENC_IS_FORBIDDEN;
			break;
		}

		case eEncryptWhenAvailable:
		{
			BOOL disconnectUndefinedPartyIfNonEncrypted = TRUE;
			CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("FORCE_ENCRYPTION_FOR_UNDEFINED_PARTICIPANT_IN_WHEN_AVAILABLE_MODE", disconnectUndefinedPartyIfNonEncrypted);
			if ((partyEncryVal != YES) && ((!bIsDefinedParty) && disconnectUndefinedPartyIfNonEncrypted))
				status = STATUS_MOVE_ENC_NON_ENC_IS_FORBIDDEN;
			break;
		}

		case eEncryptNone:
		default:
			break;
	}

	std::ostringstream msg;
	msg << "TestEncryMoveValidity "
	    << "- ConfEncryptionType:" << (WORD)destConfEncryType
	    << ", PartyEncryptionVal:" << (WORD)partyEncryVal
	    << ", IsPartyDefined:"     << (WORD)(bIsDefinedParty > 0)
	    << ", Status:"             << (WORD)status;

	if (status != STATUS_OK /*FALSE == IsNonEncryInEncConf && NO == parteEncryVal && YES == destConfEncryVal*/)
	{
		FTRACEINTO << msg.str().c_str() << ", Illegal move of non-Encrypted party to Encrypted conference";
		return STATUS_MOVE_ENC_NON_ENC_IS_FORBIDDEN;
	}
	FTRACEINTO << msg.str().c_str();

	return STATUS_OK;
}

//--------------------------------------------------------------------------
ESTATUS TestCopMoveValidity(CCommConf* pSourceConf, CCommConf* pDestConf)
{
	if (!GetIsCOPdongleSysMode())
		return STATUS_OK;

	if (pSourceConf->GetVideoSession() == pDestConf->GetVideoSession())
	{
		if (pSourceConf->GetVideoSession() == VIDEO_SESSION_COP)
		{
			CCOPConfigurationList* sourceCopConfig = pSourceConf->GetCopConfigurationList();
			CCOPConfigurationList* destCopConfig   = pDestConf->GetCopConfigurationList();
			if (*(sourceCopConfig->GetVideoMode(0)) != *(destCopConfig->GetVideoMode(0)))
			{
				FPTRACE(eLevelInfoNormal, "TestCopMoveValidity - Illegal Move - cop configurations (the highest level) of source and destination are different");
				sourceCopConfig->Dump("TestCopMoveValidity source cop configuration:", eLevelInfoNormal);
				destCopConfig->Dump("TestCopMoveValidity dest cop configuration:", eLevelInfoNormal);
				return STATUS_MOVE_DIFFERENT_COP_CONFIG;
			}
		}
		else
		{
			FPTRACE(eLevelInfoNormal, "TestCopMoveValidity - Move between non-cop conferences (VSW?), system mode is COP");
		}
	}
	else
	{
		FPTRACE(eLevelInfoNormal, "TestCopMoveValidity - Source and destination session types are different, system mode is COP");
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
ESTATUS TestDestConfMoveValidity(DWORD dwTargetConfId, BYTE isAudioOnlyParty)
{
	STATUS             status                             = STATUS_OK;
	CCommConf*         pCommConfDestConf                  = ::GetpConfDB()->GetCurrentConf(dwTargetConfId);
	CConfPartyProcess* pConfPartyProcess                  = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	WORD               maxPartiesInConfPerSystemMode      = pConfPartyProcess->GetMaxNumberOfPartiesInConf();
	WORD               maxVideoPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf();

	if (pCommConfDestConf)
	{
		if (pCommConfDestConf->GetNumParties() >= maxPartiesInConfPerSystemMode)
		{
			FPTRACE(eLevelInfoNormal, "TestDestConfMoveValidity - Failed, status:STATUS_MAX_PARTIES_OF_DESTINATION_CONF_EXCEEDED");
			status = STATUS_MAX_PARTIES_OF_DESTINATION_CONF_EXCEEDED;
		}
		else if (!isAudioOnlyParty && pCommConfDestConf->GetNumVideoParties() >= maxVideoPartiesInConfPerSystemMode)
		{
			FPTRACE(eLevelInfoNormal, "TestDestConfMoveValidity - Failed, status:MAX_VIDEO_PARTIES_OF_DESTINATION_CONF_EXCEEDED");
			status = STATUS_MAX_VIDEO_PARTIES_OF_DESTINATION_CONF_EXCEEDED;
		}

		WORD destConfMaxParts = pCommConfDestConf->GetMaxParties();
		WORD destConfNumParts = pCommConfDestConf->GetNumParties();
		if (destConfNumParts >= destConfMaxParts)
		{
			FPTRACE(eLevelInfoNormal, "TestDestConfMoveValidity - Failed, status:NUMBER_OF_PARTICIPANTS_EXCEEDS_THE_DEFINED_MAXIMUM_FROM_PROFILE");
			status = STATUS_NUMBER_OF_PARTICIPANTS_EXCEEDS_THE_DEFINED_MAXIMUM_FROM_PROFILE;
		}
	}
	else
	{
		FPTRACE(eLevelInfoNormal, "TestDestConfMoveValidity - Failed, Destination conference not found in conf DB");
		status = STATUS_FAIL;
	}

	return status;
}

//--------------------------------------------------------------------------
ESTATUS TestMoveValidity(DWORD dwTargetConfId, DWORD dwSourceConfId, CConfParty* pConfParty, bool isMoveByOperator)
{
	STATUS status                 = STATUS_OK;
	STATUS HDMoveStatus           = STATUS_OK;
	STATUS LPRMoveStatus          = STATUS_OK;
	STATUS EncryptionMoveStatus   = STATUS_OK;
	STATUS CopMoveStatus          = STATUS_OK;
	STATUS GeneralMoveParamStatus = STATUS_OK;
	STATUS MediaTypeMoveStatus 	  = STATUS_OK;

	CCommConf* pCommConfDestConf = ::GetpConfDB()->GetCurrentConf(dwTargetConfId);
	if (!pCommConfDestConf)
	{
		FPTRACE(eLevelInfoNormal, "TestMoveValidity - Failed, Target conference was not found in conf DB");
		return STATUS_FAIL;
	}

	CCommConf* pCommConfSourceConf = ::GetpConfDB()->GetCurrentConf(dwSourceConfId);
	if (!pCommConfSourceConf)
	{
		FPTRACE(eLevelInfoNormal, "TestMoveValidity - Failed, Source conference was not found in conf DB");
		return STATUS_FAIL;
	}

	if (!pConfParty)
	{
		FPTRACE(eLevelInfoNormal, "TestMoveValidity - Failed, ConfParty object is NULL");
		return STATUS_FAIL;
	}

	if ((pConfParty->GetPartyState() == PARTY_DISCONNECTING) || (pConfParty->GetPartyState() == PARTY_CONNECTING) || (pConfParty->GetPartyState() == PARTY_REDIALING))
	{
		FPTRACE2INT(eLevelInfoNormal, "TestMoveValidity - Failed, Party is in Connecting/Redialing/Disconnecting state, PartyId:", pConfParty->GetPartyId());
		return STATUS_PARTY_ILLEGAL_STATE_FOR_MOVE;
	}

	if (pCommConfDestConf->GetEntryQ())
	{
		FPTRACE2INT(eLevelInfoNormal, "TestMoveValidity - Failed, Destination conference is EQ, PartyId:", pConfParty->GetPartyId());
		return STATUS_ILLEGAL_DEST_CONF_EQ;
	}

//	if (pCommConfDestConf->GetConfMediaType() == eSvcOnly)
//	{
//		FPTRACE2(eLevelInfoNormal, "TestMoveValidity - Destination Conference is Media_Relay-Only - can't move party to it. Party Name:  ",pConfParty->GetName());
//		//return STATUS_ILLEGAL_DEST_CONF_MEDIA_RELAY;
//	}

	MediaTypeMoveStatus = ::TestMoveValidityAccordingToConfAndPartyMediaType(pCommConfSourceConf, pCommConfDestConf, pConfParty);
	if (MediaTypeMoveStatus != STATUS_OK)
	{
		return STATUS_ILLEGAL_DEST_CONF_MEDIA_RELAY;
	}
// if (Party == SVC party) --> return error, currently no SVC move

	if (isMoveByOperator)
	{
		// Block VSW->VSW move from API
		if (pCommConfSourceConf->GetIsHDVSW() && pCommConfDestConf->GetIsHDVSW())
			return STATUS_MOVE_VSW_VSW_IS_FORBIDDEN;

		// Block move from API if TelePresenceMode is not identical
		if (pCommConfSourceConf->GetIsTelePresenceMode() != pCommConfDestConf->GetIsTelePresenceMode())
			return STATUS_MOVE_TELEPRESENCE_NON_TELEPRESENCE_IS_FORBIDDEN;
	}

	// Block move of cascaded link from non EQ conference
	if (!pCommConfSourceConf->GetEntryQ() && (CASCADE_MODE_NONE != pConfParty->GetCascadeMode()))
	{
		FPTRACE2INT(eLevelInfoNormal, "TestMoveValidity - Failed, Cascaded Link can't move from non-EQ conference to another conference, PartyId:", pConfParty->GetPartyId());
		return STATUS_MOVE_CASCADED_LINK_IS_FORBIDDEN;
	}

	if (pConfParty->GetNetInterfaceType() != ISDN_INTERFACE_TYPE)
		HDMoveStatus = ::TestHDVSWMoveValidity(pCommConfDestConf->GetIsHDVSW(), pCommConfSourceConf->GetIsHDVSW(), pCommConfDestConf->GetConfTransferRate(), pCommConfSourceConf->GetConfTransferRate(), pCommConfDestConf->GetHDResolution(), (EHDResolution)pCommConfSourceConf->GetHDResolution());
	else if (!pConfParty->GetVoice() && pCommConfDestConf->GetIsHDVSW() == YES)
	{
		FPTRACE2INT(eLevelInfoNormal, "TestMoveValidity - Failed, ISDN Video participant is not supported in VSW HD conference, PartyId:", pConfParty->GetPartyId());
		HDMoveStatus = STATUS_ISDN_VIDEO_PARTICIPANT_IS_NOT_SUPPORTED_IN_VS_CONF;   // STATUS_FAIL;
	}

	if (HDMoveStatus != STATUS_OK)
		return HDMoveStatus;
	if( !pCommConfSourceConf->GetEntryQ() && (pCommConfDestConf->GetIsTipCompatible() != eTipCompatibleNone) )
	{
		FPTRACE2INT(eLevelInfoNormal, "TestMoveValidity - Failed, TIP party can't move from non-EQ conference to another conference, PartyId:", pConfParty->GetPartyId());
		return STATUS_ILLEGAL_MOVE_TIP_PARTY;
	}

	LPRMoveStatus = ::TestLPRMoveValidity(pConfParty->GetNetInterfaceType(), pCommConfSourceConf->GetLpr(), pCommConfDestConf->GetLpr(), pCommConfDestConf->GetIsHDVSW());

	if (LPRMoveStatus != STATUS_OK)
		return LPRMoveStatus;

	BYTE isPartyEncrypted =  pConfParty->GetIsPartyCurrentlyEncrypted();
	BYTE isPartyDefinedInTargetConf = STATUS_PARTY_DOES_NOT_EXIST != pCommConfDestConf->SearchPartyByIPOrAlias(pConfParty->GetIpAddress(), pConfParty->GetH323PartyAlias(), pConfParty->GetH323PartyAliasType());
	EncryptionMoveStatus = ::TestEncryMoveValidity(isPartyEncrypted, isPartyDefinedInTargetConf, pCommConfDestConf->GetEncryptionType());
	if (EncryptionMoveStatus != STATUS_OK)
		return EncryptionMoveStatus;

	CopMoveStatus = ::TestCopMoveValidity(pCommConfSourceConf, pCommConfDestConf);
	if (CopMoveStatus != STATUS_OK)
		return CopMoveStatus;

	BYTE isAudioOnlyParty = pConfParty->GetVoice();
	GeneralMoveParamStatus = ::TestDestConfMoveValidity(dwTargetConfId, isAudioOnlyParty);

	if (GeneralMoveParamStatus != STATUS_OK)
		return GeneralMoveParamStatus;

	// VNGR-24145 - Check if the target conf doesn't contain a party with the same name
	CConfParty* pTmpConfParty = pCommConfDestConf->GetFirstParty();
	while (CPObject::IsValidPObjectPtr(pTmpConfParty))
	{
		if (strcmp(pTmpConfParty->GetName(), pConfParty->GetName()) == 0)
			return STATUS_PARTY_NAME_EXISTS;

		pTmpConfParty = pCommConfDestConf->GetNextParty();
	}

	return status;
}
//--------------------------------------------------------------------------
ESTATUS TestMoveValidityAccordingToConfAndPartyMediaType(CCommConf* pCommConfSourceConf, CCommConf* pCommConfDestConf, CConfParty* pConfParty)
{
	eConfMediaType sourceConfMediaType = (eConfMediaType)pCommConfSourceConf->GetConfMediaType();
	eConfMediaType destConfMediaType = (eConfMediaType)pCommConfDestConf->GetConfMediaType();

	if (sourceConfMediaType == destConfMediaType)
	{
		if (pConfParty->GetPartyMediaType() == eSvcPartyType)
		{
			if (!pCommConfSourceConf->GetEntryQ())
			{
				FTRACEINTO <<  "Party is SVC, Source conf is not EQ! move from non-Eq conferences is not supported for SVC parties, Party name: " << pConfParty->GetName();
				return STATUS_ILLEGAL_DEST_CONF_MEDIA_RELAY;
			}

			CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
			DWORD sourceConfReservationRate = lCapInfo.TranslateReservationRateToIpRate(pCommConfSourceConf->GetConfTransferRate());
			DWORD destConfReservationRate = lCapInfo.TranslateReservationRateToIpRate(pCommConfDestConf->GetConfTransferRate());

			if (sourceConfReservationRate != destConfReservationRate)
			{
				FTRACEINTO <<  "Source Conf  and Dest conf reservation rates are not the same and therefore operation points are not the same -> Move is illegal"
				  		   <<  "(Source conf: "<< pCommConfSourceConf->GetName() << ", rate: " << sourceConfReservationRate << " Dest conf:" << pCommConfDestConf->GetName() << ", rate: " << destConfReservationRate << "), Party name: "<<  pConfParty->GetName();
				return STATUS_ILLEGAL_DEST_CONF_MEDIA_RELAY;
			}
		}
		else //AVC party
		{
			if (!pCommConfSourceConf->GetEntryQ() && destConfMediaType != eAvcOnly)
			{
				FTRACEINTO <<  "Party is AVC, Source conf is not EQ! ,Dest conf is not eAvcOnly (CP) - move from non-Eq conferences is not supported for mixed conferences (eMixAvcSvc,	eMixAvcSvcVsw), Party name: " << pConfParty->GetName();
				return STATUS_ILLEGAL_DEST_CONF_MEDIA_RELAY;
			}
		}
	}
	else
	{
		FTRACEINTO <<  "Source conf media type is different then Dest conf media type , move is not supported, Party name: " << pConfParty->GetName();
		return STATUS_ILLEGAL_DEST_CONF_MEDIA_RELAY;
	}
	return STATUS_OK;

}

void IsForceAvpOnEncryptWhenPossibleFlag(BOOL& bIsForceAvpAllFlag , BOOL& bIsForceAvpCucmFlag)
{
	bIsForceAvpAllFlag		 = FALSE;
	bIsForceAvpCucmFlag	 	 = FALSE;

	std::string strKey;
	CSysConfig* sysConfig = NULL;
	CProcessBase *pProcess = CProcessBase::GetProcess();
	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();
	if(sysConfig)
	{
		sysConfig->GetDataByKey(CFG_KEY_FORCE_AVP_ON_ENCRYPT_WHEN_POSSIBLE, strKey);

		if(strKey.compare("YES")==0)
			bIsForceAvpAllFlag = TRUE;
		else if(strKey.compare("YES_CUCM_ONLY")==0)
			bIsForceAvpCucmFlag = TRUE;
	}
}


//--------------------------------------------------------------------------
void ResolveEncryptionParameters(BYTE eConfEncryptionType, const CConfParty* pConfParty, BYTE& resultShouldEncrypt, BYTE& resultShouldDisconnectOnEncryptFailure)
{
	BYTE partyEncryptionType = pConfParty->GetIsEncrypted();
	BYTE bIsRecordingLink    = pConfParty->GetRecordingLinkParty();
	BYTE bIsDefinedParty     = (pConfParty->IsUndefinedParty() != TRUE) || bIsRecordingLink;

	if (eConfEncryptionType == eEncryptNone)
	{
		if (bIsDefinedParty && (partyEncryptionType == YES))
		{
			resultShouldEncrypt = TRUE;
			resultShouldDisconnectOnEncryptFailure = TRUE;
		}
		else
		{
			resultShouldEncrypt = FALSE;
			resultShouldDisconnectOnEncryptFailure = FALSE;
		}
	}
	else
	{
		resultShouldEncrypt = TRUE;
		resultShouldDisconnectOnEncryptFailure = TRUE;
		switch (partyEncryptionType)
		{
			case YES:
				break;

			case NO:
			// undefined parties may be set to "NO" in the lobby if they are missing SDES. in this case we check the sys. config. flag
			if (bIsDefinedParty)
			{
				FPASSERTSTREAM(eConfEncryptionType == eEncryptAll, "Non-encrypted party in encrypted conference, PartyId:" << pConfParty->GetPartyId());
				resultShouldEncrypt = FALSE;
				break;                                                    // and the connection should be rejected outright.
			}

			case AUTO:
			{
				switch (eConfEncryptionType)
				{
					case eEncryptAll:
						if (bIsRecordingLink)   // check system flag- in EncryptAll, recording links (and only them) may still connect as non-encrypted
						{
							BOOL bAllowREcordingPartyToBeNonEncrypted = NO;
							CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("ALLOW_NON_ENCRYPT_RECORDING_LINK_IN_ENCRYPT_CONF", bAllowREcordingPartyToBeNonEncrypted);
							resultShouldDisconnectOnEncryptFailure = (bAllowREcordingPartyToBeNonEncrypted == YES) ? FALSE : TRUE;
						}
						break;

					case eEncryptWhenAvailable:
					{
						BOOL bForceUndefinedPartyToBeEncrypted = YES;
						BOOL bIsForceAvpFlag = FALSE;

						IsForceAvpOnEncryptWhenPossibleFlag(bIsForceAvpFlag , bIsForceAvpFlag);

						if (!bIsDefinedParty)
						{
							CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("FORCE_ENCRYPTION_FOR_UNDEFINED_PARTICIPANT_IN_WHEN_AVAILABLE_MODE", bForceUndefinedPartyToBeEncrypted);
							// undefined parties may be set to "NO" in the lobby if they are missing SDES or IP service is without TLS
							if ((partyEncryptionType == NO) && (bForceUndefinedPartyToBeEncrypted == NO))
								resultShouldEncrypt = FALSE;
						}
						resultShouldDisconnectOnEncryptFailure = (bIsDefinedParty == TRUE) ? FALSE : bForceUndefinedPartyToBeEncrypted;

						if (bIsDefinedParty)
							resultShouldDisconnectOnEncryptFailure = FALSE;
						else
						{
							resultShouldDisconnectOnEncryptFailure = bForceUndefinedPartyToBeEncrypted;

							if (pConfParty->GetPlcmRequireMask() && (pConfParty->GetPlcmRequireMask() & m_plcmRequireAvp) && !bIsForceAvpFlag)
							{
								FTRACEINTO << "Based on the plcmRequiremask call should be unencrypted, mask = " << pConfParty->GetPlcmRequireMask();
								resultShouldEncrypt = FALSE;
							}
						}

						break;
					}

					default:
						break;
				} // switch
			}
			break;
		} // switch
	}

	FTRACEINTO << "ResolveEncryptionParameters"
	           << "- ConfEncryptionType:"  << (int)eConfEncryptionType
	           << ", PartyEncryptionType:" << (int)partyEncryptionType
	           << ", IsDefinedParty:"      << (WORD)bIsDefinedParty
	           << ", EncryptionResult:"    << (WORD)resultShouldEncrypt
	           << ", ShouldDisconnect:"    << (WORD)resultShouldDisconnectOnEncryptFailure;
}

//--------------------------------------------------------------------------
BOOL GetDongleEncryptionValue()
{
	return GlobalFlags::dongleEncryptionValue;
}

//--------------------------------------------------------------------------
void SetDongleEncryptionValue(BOOL dongleEncVal)
{
	GlobalFlags::dongleEncryptionValue = dongleEncVal;
}

//--------------------------------------------------------------------------
BOOL GetDongleFederalValue()
{
	return GlobalFlags::dongleFederalValue;
}

//--------------------------------------------------------------------------
void SetDongleFederalValue(BOOL dongleEncVal)
{
	GlobalFlags::dongleFederalValue = dongleEncVal;
}

//--------------------------------------------------------------------------
BOOL GetDonglePstnValue()
{
	return GlobalFlags::donglePstnValue;
}

//--------------------------------------------------------------------------
void SetDonglePstnValue(BOOL donglePstnVal)
{
	GlobalFlags::donglePstnValue = donglePstnVal;
}

//--------------------------------------------------------------------------
BOOL GetDongleTelepresenceValue()
{
	return GlobalFlags::dongleTelepresenceValue;
}

//--------------------------------------------------------------------------
void SetDongleTelepresenceValue(BOOL dongleTelepresenceVal)
{
	GlobalFlags::dongleTelepresenceValue = dongleTelepresenceVal;
}

//--------------------------------------------------------------------------
BOOL GetDongleMsValue()
{
	return GlobalFlags::dongleMsValue;
}

//--------------------------------------------------------------------------
void SetDongleMsValue(BOOL dongleMsVal)
{
	GlobalFlags::dongleMsValue = dongleMsVal;
}

//--------------------------------------------------------------------------
ESTATUS TestHDVSWMoveValidity(BYTE destConfHDVal, BYTE SourceConfHDVal, BYTE destConfRate, BYTE SourceConfRate, EHDResolution destConfHdVSWRes, EHDResolution sourceConfHdVswRes)
{
	// If only one of the conf set to HD
	if (destConfHDVal != SourceConfHDVal)
	{
		FTRACEINTO << "TestHDVSWMoveValidity - Failed, No both source conference and destination conference are HD, destConfHDVal:" << (WORD)destConfHDVal << ", SourceConfHDVal:" << (WORD)SourceConfHDVal;
		return STATUS_MOVE_CP_VSW_IS_FORBIDDEN;
	}
	else // If both of the confs are HD but the rate is different
	{
		if (destConfHDVal == YES && SourceConfHDVal == YES)
		{
			if (destConfRate != SourceConfRate)
			{
				FTRACEINTO << "TestHDVSWMoveValidity - Failed, Source and destination HD conferences do not have the same rate, destConfRate:" << (WORD)destConfRate << ", SourceConfRate:" << (WORD)SourceConfRate;
				return STATUS_FAIL;
			}

			if (destConfHdVSWRes != sourceConfHdVswRes)
			{
				FTRACEINTO << "TestHDVSWMoveValidity - Failed, Source and destination HD VSW conferences do not have the same resolutions, destConfHdVSWRes:" << (WORD)destConfHdVSWRes << ", sourceConfHdVswRes:" << (WORD)sourceConfHdVswRes;
				return STATUS_FAIL;
			}
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
ESTATUS TestLPRMoveValidity(BYTE PartyinterfaceType, BYTE SourceConfLPRVal, BYTE destConfLPRVal, BYTE IsHDVSW)
{
	if (destConfLPRVal != SourceConfLPRVal)
	{
		if (IsHDVSW)
		{
			if (!destConfLPRVal && SourceConfLPRVal && PartyinterfaceType == H323_INTERFACE_TYPE) // If source is with LPR and the Target conf without LPR - move is block
			{
				FTRACEINTO << "TestLPRMoveValidity - Failed, Source conference with LPR and target conference without LPR";
				return STATUS_MOVE_IP_LPR_NON_LPR_IS_FORBIDDEN;
			}
			else if ((PartyinterfaceType == SIP_INTERFACE_TYPE || PartyinterfaceType == ISDN_INTERFACE_TYPE) && destConfLPRVal)
			{
				FTRACEINTO << "TestLPRMoveValidity - Failed, Trying to move SIP Party to VSW LPR conference";
				return STATUS_FAIL;
			}
		}
		else  //added by Jason for VNGR-26566
		{
			if (!destConfLPRVal && SourceConfLPRVal) // If source is with LPR and the Target conf without LPR - move is block
			{
				FTRACEINTO << "TestLPRMoveValidity - Failed, Source conference with LPR and target conference without LPR";
				return STATUS_MOVE_IP_LPR_NON_LPR_IS_FORBIDDEN;
			}
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
ESTATUS TestH264ContentValidity(BYTE srcProtocolType, BYTE dstProtocolType)
{
	if (srcProtocolType != dstProtocolType)
	{
		// If Source is with H264Fix and the Target is with AUTO or H263 FIx - move will be block!!
		FTRACEINTO << "TestH264ContentValidity - Failed, Source and destination conferences do not have the same content protocol, srcProtocolType:" << (WORD)srcProtocolType << ", dstProtocolType:" << (WORD)dstProtocolType;
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
DWORD GetVisualNameCounter()
{
	DWORD visualNameCounter = sVisualNameCounter;
	IncreaseVisualNameCounter();
	return visualNameCounter;
}

//--------------------------------------------------------------------------
void IncreaseVisualNameCounter()
{
	if (sVisualNameCounter >= 99999)
		sVisualNameCounter = 0;
	else
		sVisualNameCounter++;
}

//--------------------------------------------------------------------------
STATUS IsVisualNameConflict(const char* confName, char* newName, BYTE isEQConf, DWORD partyId)
{
	STATUS status = STATUS_OK;
	if (isEQConf)
	{
		status = ::GetpConfDB()->SearchPartyNameInAllConferences(newName);
		if (status == STATUS_OK)
			status = STATUS_PARTY_NAME_EXISTS;
		else if (status == STATUS_PARTY_DOES_NOT_EXIST)
		{
			status = ::GetpConfDB()->SearchPartyVisualNameInAllConferences(newName, partyId);
			if (status == STATUS_OK)
				status = STATUS_PARTY_VISUAL_NAME_EXISTS;
			else if (status == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS)
				status = STATUS_OK;
		}
	}
	else
	{
		status = ::GetpConfDB()->SearchPartyName(confName, newName);
		if (status == STATUS_OK)
		{
			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confName);
			if (pCommConf)
			{
				CRsrvParty* pParty = pCommConf->GetCurrentParty(newName);
				if (pParty)
				{
					if (pParty->GetPartyId() != partyId) // only if it's not the same party
						status = STATUS_PARTY_NAME_EXISTS;
				}
			}
		}
		else if (status == STATUS_PARTY_DOES_NOT_EXIST)
		{
			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confName);
			if (pCommConf)
			{
				status = ::GetpConfDB()->SearchPartyVisualNameByPartyId(confName, newName,partyId);
			}
			if (status == STATUS_OK)
				status = STATUS_PARTY_VISUAL_NAME_EXISTS;
			else if (status == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS)
				status = STATUS_OK;
		}
	}

	return status;
}

//--------------------------------------------------------------------------
void GetUpdatedVisualName(const char* confName, char* visualName, char* updatedVisualName)
{
	char tempVisualName[H243_NAME_LEN];
	char counterString[20];

	while (1)
	{
		memset(tempVisualName, 0, sizeof(tempVisualName));

		strncpy(tempVisualName, visualName, H243_NAME_LEN-9);
		tempVisualName[H243_NAME_LEN-9] = '\0';

		memset(counterString, 0, sizeof(counterString));
		sprintf(counterString, "_(%05d)", GetVisualNameCounter());
		strncat(tempVisualName, counterString, 8);

		STATUS isPartyNameExists       = ::GetpConfDB()->SearchPartyName(confName, tempVisualName); // if status does not exist, the returned status is STATUS_PARTY_DOES_NOT_EXIST.
		STATUS isPartyVisualNameExists = ::GetpConfDB()->SearchPartyVisualName(confName, tempVisualName);
		if ((isPartyVisualNameExists == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS) && (isPartyNameExists == STATUS_PARTY_DOES_NOT_EXIST)) // This name exists in a party name or visual name
		{
			strcpy_safe(updatedVisualName, H243_NAME_LEN, tempVisualName);
			return;
		}
	}
}

//--------------------------------------------------------------------------
void GetUpdatedVisualNameForPartyInEQ(char* visualName, char* updatedVisualName)
{
	char tempVisualName[H243_NAME_LEN];
	char counterString[20];

	while (1)
	{
		memset(tempVisualName, 0, sizeof(tempVisualName));

		strncpy(tempVisualName, visualName, H243_NAME_LEN-9);
		tempVisualName[H243_NAME_LEN-9] = '\0';

		memset(counterString, 0, sizeof(counterString));
		sprintf(counterString, "_(%05d)", GetVisualNameCounter());
		strncat(tempVisualName, counterString, 8);

		STATUS isPartyNameExists       = ::GetpConfDB()->SearchPartyNameInAllConferences(tempVisualName);
		STATUS isPartyVisualNameExists = ::GetpConfDB()->SearchPartyVisualNameInAllConferences(tempVisualName);
		if ((isPartyVisualNameExists == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS) && (isPartyNameExists == STATUS_PARTY_DOES_NOT_EXIST))
		{
			strcpy_safe(updatedVisualName, H243_NAME_LEN, tempVisualName);
			return;
		}
	}
}

//--------------------------------------------------------------------------
DWORD CalculateMcuInternalProblemErrorNumber(BYTE MipHardWareConn, BYTE MipMedia, BYTE MipDirection, BYTE MipTimerStatus, BYTE MipAction)
{
	DWORD MipErrorNumber = 0;
	MipErrorNumber = (MipHardWareConn*10000) + (MipMedia*1000) + (MipDirection*100) + (MipTimerStatus*10) + MipAction;
	FPTRACE2INT(eLevelInfoNormal, "MCU INTERNAL NUMBER = ", MipErrorNumber);
	return MipErrorNumber;
}

//--------------------------------------------------------------------------
void GetVSWH263VideoFormatCapBuf(WORD H263Format, BYTE* pH263CapSetBuf)
{
	switch (H263Format)
	{
		case H263_QCIF_SQCIF:
		{
			pH263CapSetBuf[MinPictureHeight]    = 17; // 176 x 144
			pH263CapSetBuf[MinPictureWidth]     = 21;
			pH263CapSetBuf[VideoFormat]         = H263Format;
			pH263CapSetBuf[MPI]                 = MPI_1;
			break;
		}
		case H263_CIF:
		{
			pH263CapSetBuf[MinPictureHeight]    = 35; // 352 x 288
			pH263CapSetBuf[MinPictureWidth]     = 43;
			pH263CapSetBuf[VideoFormat]         = H263Format;
			pH263CapSetBuf[MPI]                 = MPI_1;
			break;
		}
		case H263_CIF_4:
		{
			pH263CapSetBuf[MinPictureHeight]    = 71; // 704 x 576
			pH263CapSetBuf[MinPictureWidth]     = 87;
			pH263CapSetBuf[VideoFormat]         = H263Format;
			pH263CapSetBuf[MPI]                 = MPI_2;
			break;
		}
		case H263_CIF_16:
		{
			pH263CapSetBuf[MinPictureHeight]    = 143; // 1408 x 1152
			pH263CapSetBuf[MinPictureWidth]     = 175;
			pH263CapSetBuf[VideoFormat]         = H263Format;
			pH263CapSetBuf[MPI]                 = MPI_5;
			break;
		}
		case VGA:
		{
			pH263CapSetBuf[MinPictureHeight]    = 59; // 640 x 480
			pH263CapSetBuf[MinPictureWidth]     = 79;
			pH263CapSetBuf[VideoFormat]         = H263_CUSTOM_FORMAT;
			pH263CapSetBuf[ClockDivisor]        = 60;
			pH263CapSetBuf[ClockConversionCode] = 1;
			pH263CapSetBuf[CustomMPIIndicator]  = MPI_2;
			break;
		}
		case NTSC:
		{
			pH263CapSetBuf[MinPictureHeight]    = 59; // 704 x 480
			pH263CapSetBuf[MinPictureWidth]     = 87;
			pH263CapSetBuf[VideoFormat]         = H263_CUSTOM_FORMAT;
			pH263CapSetBuf[ClockDivisor]        = 60;
			pH263CapSetBuf[ClockConversionCode] = 1;
			pH263CapSetBuf[CustomMPIIndicator]  = MPI_2;
			break;
		}
		case SVGA:
		{
			pH263CapSetBuf[MinPictureHeight]    = 74; // 800 x 600
			pH263CapSetBuf[MinPictureWidth]     = 99;
			pH263CapSetBuf[VideoFormat]         = H263_CUSTOM_FORMAT;
			pH263CapSetBuf[ClockDivisor]        = 60;
			pH263CapSetBuf[ClockConversionCode] = 1;
			pH263CapSetBuf[CustomMPIIndicator]  = MPI_3;
			break;
		}
		case XGA:
		{
			pH263CapSetBuf[MinPictureHeight]    = 95; // 1024 x 768
			pH263CapSetBuf[MinPictureWidth]     = 127;
			pH263CapSetBuf[VideoFormat]         = H263_CUSTOM_FORMAT;
			pH263CapSetBuf[ClockDivisor]        = 60;
			pH263CapSetBuf[ClockConversionCode] = 1;
			pH263CapSetBuf[CustomMPIIndicator]  = MPI_4;
			break;
		}
		case NTSC_60_FIELDS:
		{
			pH263CapSetBuf[MinPictureHeight]    = 29; // 352 x 240
			pH263CapSetBuf[MinPictureWidth]     = 43;
			pH263CapSetBuf[VideoFormat]         = H263_CUSTOM_FORMAT;
			pH263CapSetBuf[ClockDivisor]        = CLOCK_DEVISOR_60_FILEDS;
			pH263CapSetBuf[ClockConversionCode] = CLOCK_CONVERSION_CODE_60_FILEDS_ISDN;
			pH263CapSetBuf[CustomMPIIndicator]  = CUSTOM_MPI_INDICATOR_ISDN;
			pH263CapSetBuf[InterlaceMode]       = 1;
			break;
		}
		case PAL_50_FIELDS: // exactly like CIF thus we relate to it as CIF.
		{
			pH263CapSetBuf[MinPictureHeight]    = 35; // 352 x 288
			pH263CapSetBuf[MinPictureWidth]     = 43;
			pH263CapSetBuf[VideoFormat]         = H263_CIF;
			pH263CapSetBuf[ClockDivisor]        = CLOCK_DEVISOR_50_FILEDS;
			pH263CapSetBuf[ClockConversionCode] = CLOCK_CONVERSION_CODE_50_FILEDS_ISDN;
			pH263CapSetBuf[CustomMPIIndicator]  = CUSTOM_MPI_INDICATOR_ISDN;
			pH263CapSetBuf[MPI]                 = MPI_1; // Relates to the INITIAL part - we declare on 30 FPS
			pH263CapSetBuf[InterlaceMode]       = 1;
			pH263CapSetBuf[Annex_N]             = 1;
			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
void GetXferBitrate(DWORD& xfer_bitrate, CComMode workComMode)
{
	WORD numChnls  = workComMode.GetNumChnl();
	WORD chnlWidth = workComMode.GetChnlWidth();
	xfer_bitrate = numChnls * chnlWidth * 64 * 1000;
	if (workComMode.m_otherMode.GetRestrictMode() == Restrict)
		xfer_bitrate = (xfer_bitrate * 7)/8;
}

//--------------------------------------------------------------------------
LPRParams* lookupLprParams(unsigned int protection, unsigned int mtbf, unsigned int packetLen, unsigned int rate)
{
	unsigned int protIndex = 0, rateIndex = 0;

	if ((protection == 0) || (rate == 0))
	{
		return(&gLPRParamTable[0].params[0]);
	}
	else
	{
		// compute the number of data and recovery packets we want to use
		// based upon the percent protection, MTBF
		for (protIndex = 1; protIndex < gLPRParamTableSize/sizeof(LPRRateParams); protIndex++)
		{
			// find the best protection table
			if ((protection <= gLPRParamTable[protIndex].protection) &&
			    (mtbf <= gLPRParamTable[protIndex].minMTBF))
			{
				// now match the rate
				for (rateIndex = 1; rateIndex < LPR_TABLE_LEN; rateIndex++)
				{
					// make sure we are within the receiver's max limits
					if (((gLPRParamTable[protIndex].params[rateIndex].numData > gLPRMaxParamTable.numData) ||
					     (gLPRParamTable[protIndex].params[rateIndex].numRecovery > gLPRMaxParamTable.numRecovery)))
					{
						rateIndex--;
						break;
					}

					// match the rate
					if ((rate <= gLPRParamTable[protIndex].params[rateIndex].bitRate) &&
					    (rate > gLPRParamTable[protIndex].params[rateIndex-1].bitRate))
					{
						break;
					}
				}

				if (rateIndex >= LPR_TABLE_LEN)
					rateIndex = LPR_TABLE_LEN-1;

				return(&gLPRParamTable[protIndex].params[rateIndex]);
			}
		}
	}

	return (NULL);
}

//--------------------------------------------------------------------------
eSystemCardsMode GetSystemCardsBasedMode()
{
	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	return pConfPartyProcess->GetSystemCardsBasedMode();
}

//--------------------------------------------------------------------------
eVideoPartyType GetCPH264ResourceVideoPartyType(DWORD MaxFS, DWORD MaxMBPS, BOOL isRtv/*=false*/,BYTE IsRsrcByFs)
{
	eVideoPartyType  videoPartyType       = eVideo_party_type_none;
	DWORD            minFrameRateForSD;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("MINIMUM_FRAME_RATE_TRESHOLD_FOR_SD", minFrameRateForSD);
	eProductType prodType = CProcessBase::GetProcess()->GetProductType();
	eSystemCardsMode   systemCardsMode = GetSystemCardsBasedMode();

	DWORD remoteFrameRate = 0;

	if (MaxFS != 0)
		remoteFrameRate = MaxMBPS / MaxFS;

	if (IsRsrcByFs)
	{
		if (MaxFS <= H264_W4CIF_FS)
			videoPartyType = eCP_H264_upto_CIF_video_party_type;
		else
		{
			if (MaxFS <= SD_15_FS)
			{
				videoPartyType = eCP_H264_upto_SD30_video_party_type;
			}
			else
			{
				if (MaxFS <= H264_HD720_FS)
				{
					if (MaxMBPS <= H264_HD720_30_MBPS)
						videoPartyType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
					else //------HD720 60
						videoPartyType = eCP_H264_upto_HD720_60FS_Symmetric_video_party_type; //only breeze mode
				}
				else
					videoPartyType = eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
			}
		}
	}

	else
	{
		if (MaxFS <= H264_HD720_FS)
		{
			// in RTV we want to enable new threshold for HD
			if (isRtv && MaxMBPS > H264_SD_30_MBPS)
			{
				std::string max_rtv_protocol_str;
				BOOL res = CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey(CFG_MAX_RTV_RESOLUTION, max_rtv_protocol_str);
				FPASSERTSTREAM_AND_RETURN_VALUE(!res, "CSysConfig::GetDataByKey: " << CFG_MAX_RTV_RESOLUTION, eCP_H264_upto_CIF_video_party_type);

				bool  is_HD_max_rtv_protocol   = ("HD720" == max_rtv_protocol_str);
				bool  is_AUTO_max_rtv_protocol = ("AUTO" == max_rtv_protocol_str);
				DWORD fps                      = MaxMBPS/MaxFS;
				DWORD max_allowed_rtv_hd_fps   = GetSystemCfgFlagInt<DWORD>(CFG_KEY_MAX_ALLOWED_RTV_HD_FRAME_RATE);

				if ((is_AUTO_max_rtv_protocol && max_allowed_rtv_hd_fps != 0) || is_HD_max_rtv_protocol)
				{
					if (fps >= max_allowed_rtv_hd_fps)
					{
						videoPartyType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
						FTRACEINTO << "fps:" << fps << ", max_allowed_rtv_hd_fps:" << max_allowed_rtv_hd_fps << ", videoPartyType:" << eVideoPartyTypeNames[videoPartyType];
						return videoPartyType;
					}
				}
			}

			// -------CIF30/SD8/-----------
			if ((remoteFrameRate < minFrameRateForSD && MaxFS <= H264_W4CIF_FS && !isRtv) || (MaxMBPS < H264_SD_8_MBPS))
			{
				videoPartyType = eCP_H264_upto_CIF_video_party_type; // only breeze mode
			}
			// -------CIF50/SIF60/2CIF25/2SIF30/SD15-----
			else if (MaxMBPS <= H264_SD_15_MBPS)
			{
				videoPartyType = eCP_H264_upto_SD30_video_party_type;
			}
			// -----------  SD30/2SIF50/2CIF60  ------------
			else if (MaxMBPS <= H264_W4CIF_30_MBPS)
			{
				// -------For MPM-Rx and Ninja - CIF50/SIF60 ----------
				if ((eSystemCardsMode_mpmrx == systemCardsMode) && (MaxFS < MSSVC_SD_FS))
				{
					videoPartyType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
				}
				else
				{
					videoPartyType = eCP_H264_upto_SD30_video_party_type;
				}
			}
			// --------For MPM-Rx and Ninja - SD60 ----------
			else if ((eSystemCardsMode_mpmrx == systemCardsMode) && (MaxFS <= H264_W4CIF_FS) && (MaxMBPS <= H264_HD720_30_MBPS))
			{
				videoPartyType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
			}
			// -------HD720 30/ SD60----------
			else if (MaxMBPS <= H264_HD720_30_MBPS)
			{
				videoPartyType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
			}
			else                                                                    // ------HD720 60
			{
				videoPartyType = eCP_H264_upto_HD720_60FS_Symmetric_video_party_type;
			}
		}
		else  // In case the Frame Size is greater than HD720
		{
			// Checking if the mbps is larger than supported by 1080p30 (rounding up to CUSTOM_MAX_MBPS_FACTOR)
			if (MaxMBPS <= (DWORD)CUSTOM_MAX_MBPS_FACTOR*GetMaxMbpsAsDevision(H264_HD1080_30_MBPS_TOLERACE))
			{
				videoPartyType = eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
			}
			else
			{
				if (IsFeatureSupportedBySystem(eFeatureHD1080p60Asymmetric))
					videoPartyType = eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type;
				else // MPM-Rx or Mixed Mode (MPMx and MPM-Rx)
					videoPartyType = eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type;
			}
		}
	}

	FTRACEINTO << "MaxFS:"             << MaxFS
	           << ", MaxMBPS:"         << MaxMBPS
	           << ", IsRsrcByFs:"      << (WORD)IsRsrcByFs
	           << ", VideoPartyType:"  << eVideoPartyTypeNames[videoPartyType];

	return videoPartyType;
}
//--------------------------------------------------------------------------
eVideoPartyType GetCPHVP8ResourceVideoPartyType(DWORD MaxFS, DWORD MaxMBPS, DWORD FR, BYTE IsRsrcByFs)
{
	eVideoPartyType  videoPartyType       = eVideo_party_type_none;
	DWORD            minFrameRateForSD;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("MINIMUM_FRAME_RATE_TRESHOLD_FOR_SD", minFrameRateForSD);
	eProductType prodType = CProcessBase::GetProcess()->GetProductType();
	eSystemCardsMode   systemCardsMode = GetSystemCardsBasedMode();

	DWORD remoteFrameRate = 0;

	if (IsRsrcByFs)
	{
		if (MaxFS <= H264_W4CIF_FS)
		{
			videoPartyType = eCP_VP8_upto_CIF_video_party_type;
		}
		else if(MaxFS <= SD_15_FS)
		{
			videoPartyType = eCP_VP8_upto_SD30_video_party_type;
		}
		else if (MaxFS <= H264_HD720_FS)
		{
			videoPartyType = eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type;
		}
	}
	else
	{
		if (MaxFS <= H264_HD720_FS)
		{
			// -------CIF30/SD8/-----------
			if ((remoteFrameRate < minFrameRateForSD && MaxFS <= H264_W4CIF_FS) || (MaxMBPS < H264_SD_8_MBPS))
			{
				videoPartyType = eCP_VP8_upto_CIF_video_party_type; // only breeze mode
			}
			// -------CIF50/SIF60/2CIF25/2SIF30/SD15-----
			else if (MaxMBPS <= H264_SD_15_MBPS)
			{
				videoPartyType = eCP_VP8_upto_SD30_video_party_type;
			}
			// -----------  SD30/2SIF50/2CIF60  ------------
			else if (MaxMBPS <= H264_W4CIF_30_MBPS)
			{
				videoPartyType = eCP_VP8_upto_SD30_video_party_type;
			}
			// -------HD720 30/ SD60----------
			else if (MaxMBPS <= H264_HD720_30_MBPS)
			{
				videoPartyType = eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type;
			}
		}
		else  // In case the Frame Size is greater than HD720
		{
			// Checking if the mbps is larger than supported by 1080p30 (rounding up to CUSTOM_MAX_MBPS_FACTOR)
			if (MaxMBPS <= (DWORD)CUSTOM_MAX_MBPS_FACTOR*GetMaxMbpsAsDevision(H264_HD1080_30_MBPS_TOLERACE))
			{
				videoPartyType = eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
			}
			else
			{
				if (IsFeatureSupportedBySystem(eFeatureHD1080p60Asymmetric))
					videoPartyType = eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type;
				else // MPM-Rx or Mixed Mode (MPMx and MPM-Rx)
					videoPartyType = eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type;
			}
		}
	}


	FTRACEINTO << "MaxFS:"             << MaxFS
	           << ", MaxMBPS:"         << MaxMBPS
	           << ", IsRsrcByFs:"      << (WORD)IsRsrcByFs
	           << ", VideoPartyType:"  << eVideoPartyTypeNames[videoPartyType];

	return videoPartyType;
}

//--------------------------------------------------------------------------
eVideoPartyType GetH261H263ResourcesPartyType(BYTE is4CIF)
{
	eVideoPartyType videoPartyType = eVideo_party_type_none;

	if(is4CIF)
		videoPartyType = eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type;
	else
		videoPartyType = eCP_H261_H263_upto_CIF_video_party_type;

	return videoPartyType;
}

//--------------------------------------------------------------------------
BYTE isIpVersionMatchBetweenPartyAndService(mcTransportAddress* partyAddr, CConfIpParameters* pServiceParams)
{
	BYTE isMatch = FALSE;

	eIpType ipTypeInService = pServiceParams->GetIPAddressTypesInService();

	if (eIpType_None == ipTypeInService)
	{
		isMatch = FALSE;
	}
	else if (((DWORD)eIpVersion4 == partyAddr->ipVersion) && ((eIpType_IpV4 == ipTypeInService) || (eIpType_Both == ipTypeInService)))
	{
		isMatch = TRUE;
	}
	else if (((DWORD)eIpVersion6 == partyAddr->ipVersion) && ((eIpType_IpV6 == ipTypeInService) || (eIpType_Both == ipTypeInService)))
	{
		isMatch = TRUE;
	}

	if (FALSE == isMatch)
	{
		FTRACEINTO << "isIpVersionMatchBetweenPartyAndService - Failed, "
		           << "- ipTypeInService:" << ::IpTypeToString((DWORD)ipTypeInService)
		           << ", ipTypeInParty:"   << (((DWORD)eIpVersion4 == partyAddr->ipVersion) ? "IPV4" : "IPV6");
	}

	return isMatch;
}

//--------------------------------------------------------------------------
BYTE FindIpVersionScopeIdMatchBetweenPartyAndService(const mcTransportAddress* partyAddr, CConfIpParameters* pServiceParams)
{
	BYTE isMatch = 0xFF;

	if (pServiceParams->GetIPAddressTypesInService() == eIpType_None)
		return isMatch;

	std::ostringstream dumpstring;
	pServiceParams->Dump(dumpstring);
	FPTRACE2(eLevelInfoNormal, "FindIpVersionScopeIdMatchBetweenPartyAndService - service params: \n", (char*)dumpstring.str().c_str());

	char strPartyAddr[IPV6_ADDRESS_LEN];
	memset(&strPartyAddr, '\0', IPV6_ADDRESS_LEN);
	::ipToString(*partyAddr, strPartyAddr, 1);
	enScopeId ePartyScopeId = ::getScopeId(strPartyAddr);
	if (ePartyScopeId == eScopeIdGlobal)
		FPTRACE2INT(eLevelInfoNormal, "FindIpVersionScopeIdMatchBetweenPartyAndService - gloadal add = ", (DWORD)ePartyScopeId);

	ipAddressIf* ipV6ServiceAddrArr = pServiceParams->GetIpV6AddressArr();
	char         strServiceAddr[IPV6_ADDRESS_LEN];

	for (int i = 0; i < NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(&strServiceAddr, '\0', IPV6_ADDRESS_LEN);
		static APIU8 ipNull[16] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

		if (memcmp(ipV6ServiceAddrArr[i].v6.ip, ipNull, sizeof(ipV6ServiceAddrArr[i].v6.ip)) == TRUE)        // not equal to NULL return true
		{
			mcTransportAddress pTempSerAddr;
			memset(&pTempSerAddr, 0, sizeof(mcTransportAddress));
			memcpy(&pTempSerAddr.addr.v6.ip, &ipV6ServiceAddrArr[i], IPV6_ADDRESS_BYTES_LEN);
			pTempSerAddr.addr.v6.scopeId = ipV6ServiceAddrArr[i].v6.scopeId;
			pTempSerAddr.ipVersion       = eIpVersion6;
			::ipToString(pTempSerAddr, strServiceAddr, 1);
			enScopeId eServiceScopeId = ::getScopeId(strServiceAddr);

			char tempName[IPV6_ADDRESS_LEN];
			memset(tempName, 0, sizeof(tempName));
			::ipToString(pTempSerAddr, tempName, 1);
			FTRACEINTO << "FindIpVersionScopeIdMatchBetweenPartyAndService - service ip " << tempName;
			if (eServiceScopeId == ePartyScopeId)
			{
				isMatch = i;
				break;
			}
		}
	}

	if (isMatch == 0xFF)
		return 0;

	return isMatch;
}

//--------------------------------------------------------------------------
BYTE FindPlaceAccordingtoScopeType(enScopeId scopeId, CConfIpParameters* pServiceParams)
{
	BYTE place = 0;
	ipAddressIf* ipV6ServiceAddrArr = pServiceParams->GetIpV6AddressArr();
	for (int i = 0; i < NUM_OF_IPV6_ADDRESSES; i++)
	{
		static APIU8 ipNull[16] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

		if (memcmp(ipV6ServiceAddrArr[i].v6.ip, ipNull, sizeof(ipV6ServiceAddrArr[i].v6.ip)) == TRUE)        // not equal to NULL return true
		{
			if ((enScopeId)(ipV6ServiceAddrArr[i].v6.scopeId) == scopeId)
				return i;
		}
	}

	return 0;
}

//--------------------------------------------------------------------------
BYTE FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(const mcTransportAddress* partyAddr, ipAddressV6If* pUdpAddrArr)
{
	BYTE isMatch = 0;

	char strPartyAddr[IPV6_ADDRESS_LEN];
	memset(strPartyAddr, 0, sizeof(strPartyAddr));
	::ipToString(*partyAddr, strPartyAddr, 1);
	enScopeId ePartyScopeId = ::getScopeId(strPartyAddr);

	char strServiceAddr[IPV6_ADDRESS_LEN];

	for (int i = 0; i < NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(&strServiceAddr, '\0', IPV6_ADDRESS_LEN);
		static APIU8 ipNull[16] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			                         0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

		if (memcmp(pUdpAddrArr[i].ip, ipNull, sizeof(pUdpAddrArr[i].ip)) == TRUE)        // not equal to NULL return true
		{
			mcTransportAddress pTempSerAddr;
			memset(&pTempSerAddr, 0, sizeof(mcTransportAddress));
			pTempSerAddr.ipVersion = eIpVersion6;
			memcpy(&pTempSerAddr.addr.v6.ip, &pUdpAddrArr[i], IPV6_ADDRESS_BYTES_LEN);
			::ipToString(pTempSerAddr, strServiceAddr, 1);
			enScopeId eServiceScopeId = ::getScopeId(strServiceAddr);
			if (eServiceScopeId == ePartyScopeId)
			{
				isMatch = i;
				break;
			}
		}
	}

	return isMatch;
}

//--------------------------------------------------------------------------
// / We should call this function in CP sharpness conferences, were we support H263 4CIf15 mode (according to the H263 decision matrix)
// / and the EP sent capabilities with both H264 and H263. This function defines the threshold in which we would prefer to send H263 4CIF 15 over H264 in this case
BOOL IsH2634Cif15PreferedOverH264InSharpnessConf(DWORD h264MaxFS, DWORD h264MaxMBPS)
{
	BOOL  isH2634Cif15Preferered = FALSE;
	DWORD minFrameRateForSD;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("MINIMUM_FRAME_RATE_TRESHOLD_FOR_SD", minFrameRateForSD);

	if (h264MaxMBPS < H264_VGA_15_MBPS)
	{
		isH2634Cif15Preferered = TRUE;
		FTRACEINTO << "IsH2634Cif15PreferedOverH264InSharpnessConf - isH2634Cif15Preferered:" << isH2634Cif15Preferered << ", h264MaxMBPS:" << h264MaxMBPS;
	}
	else if (h264MaxFS < H264_VGA_FS) // if the remote H264 capabilities are with large enough MBPS but the FS is small
	{
		isH2634Cif15Preferered = TRUE;
		FTRACEINTO << "IsH2634Cif15PreferedOverH264InSharpnessConf - isH2634Cif15Preferered:" << isH2634Cif15Preferered << ", h264MaxFS:" << h264MaxFS;
	}

	if (h264MaxFS >= SD_15_FS)
	{
		DWORD remoteFrameRate = h264MaxMBPS/h264MaxFS;
		if (remoteFrameRate < minFrameRateForSD)
			isH2634Cif15Preferered = TRUE;
		FTRACEINTO << "IsH2634Cif15PreferedOverH264InSharpnessConf - isH2634Cif15Preferered:" << isH2634Cif15Preferered << ", h264MaxFS:" << h264MaxFS;
	}

	return isH2634Cif15Preferered;
}

//--------------------------------------------------------------------------
LayoutType TranslateSysConfigStringToLayoutType(std::string layoutStr)
{
	if (layoutStr == "CP_LAYOUT_1X1" || layoutStr == "CP_LAYOUT_1x1")
		return CP_LAYOUT_1X1;

	if (layoutStr == "CP_LAYOUT_1X2" || layoutStr == "CP_LAYOUT_1x2")
		return CP_LAYOUT_1X2;

	if (layoutStr == "CP_LAYOUT_2X1" || layoutStr == "CP_LAYOUT_2x1")
		return CP_LAYOUT_2X1;

	if (layoutStr == "CP_LAYOUT_2X2" || layoutStr == "CP_LAYOUT_2x2")
		return CP_LAYOUT_2X2;

	if (layoutStr == "CP_LAYOUT_3X3" || layoutStr == "CP_LAYOUT_3x3")
		return CP_LAYOUT_3X3;

	if (layoutStr == "CP_LAYOUT_1P5")
		return CP_LAYOUT_1P5;

	if (layoutStr == "CP_LAYOUT_1P7")
		return CP_LAYOUT_1P7;

	if (layoutStr == "CP_LAYOUT_1X2VER" || layoutStr == "CP_LAYOUT_1x2VER")
		return CP_LAYOUT_1x2VER;

	if (layoutStr == "CP_LAYOUT_1X2HOR" || layoutStr == "CP_LAYOUT_1x2HOR")
		return CP_LAYOUT_1x2HOR;

	if (layoutStr == "CP_LAYOUT_1P2VER")
		return CP_LAYOUT_1P2VER;

	if (layoutStr == "CP_LAYOUT_1P2HOR")
		return CP_LAYOUT_1P2HOR;

	if (layoutStr == "CP_LAYOUT_1P3HOR")
		return CP_LAYOUT_1P3HOR;

	if (layoutStr == "CP_LAYOUT_1P3VER")
		return CP_LAYOUT_1P3VER;

	if (layoutStr == "CP_LAYOUT_1P4HOR")
		return CP_LAYOUT_1P4HOR;

	if (layoutStr == "CP_LAYOUT_1P4VER")
		return CP_LAYOUT_1P4VER;

	if (layoutStr == "CP_LAYOUT_1P8CENT")
		return CP_LAYOUT_1P8CENT;

	if (layoutStr == "CP_LAYOUT_1P8UP")
		return CP_LAYOUT_1P8UP;

	if (layoutStr == "CP_LAYOUT_1P2HOR_UP")
		return CP_LAYOUT_1P2HOR_UP;

	if (layoutStr == "CP_LAYOUT_1P3HOR_UP")
		return CP_LAYOUT_1P3HOR_UP;

	if (layoutStr == "CP_LAYOUT_1P4HOR_UP")
		return CP_LAYOUT_1P4HOR_UP;

	if (layoutStr == "CP_LAYOUT_1P8HOR_UP")
		return CP_LAYOUT_1P8HOR_UP;

	if (layoutStr == "CP_LAYOUT_4X4" || layoutStr == "CP_LAYOUT_4x4")
		return CP_LAYOUT_4X4;

	if (layoutStr == "CP_LAYOUT_2P8")
		return CP_LAYOUT_2P8;

	if (layoutStr == "CP_LAYOUT_1P12")
		return CP_LAYOUT_1P12;

	if (layoutStr == "CP_LAYOUT_1TOP_LEFT_P8")
		return CP_LAYOUT_1TOP_LEFT_P8;

	if (layoutStr == "CP_LAYOUT_2TOP_P8")
		return CP_LAYOUT_2TOP_P8;

	return CP_NO_LAYOUT;
}

//--------------------------------------------------------------------------
// receive user product ID and check if it part of a flag to force static MB at the encoder.
// return TRUE or FALSE
BYTE IsSetStaticMbForUser(const char* productStr)
{
	if (productStr == NULL || strlen(productStr) == 0)
		return FALSE;

	std::string sProducts;
	CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey("FORCE_STATIC_MB_ENCODING", sProducts);
	if (sProducts.size() > 0)
	{
		if (strcmp("NONE", sProducts.c_str()) == 0)
			return FALSE;

		if (strcmp("ANY", sProducts.c_str()) == 0)
			return TRUE;

		return ::IsStringMatchFlagOfMultiNames(productStr, sProducts);
	}

	return FALSE; // string is empty
}

//--------------------------------------------------------------------------
// receive user product ID and check if it part of a flag to force CIF rsrc at the encoder.
// return TRUE or FALSE
BYTE IsSetCIFRsrcForUser(const char* productStr, DWORD callrate)
{
	if (productStr == NULL || strlen(productStr) == 0)
		return 0;

	FTRACEINTO << "IsSetCIFRsrcForUser - productStr:" << productStr << ", callRate:" << callrate;

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	std::string sProducts;
	sysConfig->GetDataByKey("FORCE_CIF_PORT_ALLOCATION", sProducts);
	if (sProducts.size() > 0)
	{
		BYTE isneedtocheck = TRUE;
		if (strcmp("NONE", sProducts.c_str()) == 0)
			isneedtocheck = FALSE;

		if (strcmp("ANY", sProducts.c_str()) == 0)
			return TRUE;

		if (isneedtocheck && ::IsStringMatchFlagOfMultiNames(productStr, sProducts))
			return TRUE;
	}

	std::string sProductsForMpmxOnly;
	sysConfig->GetDataByKey("FORCE_CIF_PORT_ALLOCATION_MPMX", sProductsForMpmxOnly);
	if (sProductsForMpmxOnly.size() > 0)
	{
		if (strcmp("NONE", sProductsForMpmxOnly.c_str()) == 0)
			return FALSE;

		if (strcmp("ANY", sProductsForMpmxOnly.c_str()) == 0)
			return TRUE;

		return ::IsStringMatchFlagOfMultiNames(productStr, sProductsForMpmxOnly);
	}

	return FALSE; // string is empty
}

//--------------------------------------------------------------------------
BYTE IsSetSmartSwitchForUser(const char* productStr)
{
	if (productStr == NULL || strlen(productStr) == 0)
		return 0;

	FTRACEINTO << "IsSetSmartSwitchForUser - productStr:" << productStr;

	std::string sProducts;
	CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey("USE_SMART_SWITCH_PRODUCT_NAME", sProducts);
	if (sProducts.size() > 0)
	{
		if (strcmp("NONE", sProducts.c_str()) == 0)
			return FALSE;

		if (strcmp("ANY", sProducts.c_str()) == 0)
			return TRUE;

		if (::IsStringMatchFlagOfMultiNames(productStr, sProducts))
			return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------------
DWORD GetNewRateForUser(const char* productStr, DWORD callrate)
{
	if (productStr == NULL || strlen(productStr) == 0)
		return 0;

	FTRACEINTO << "GetNewRateForUser - productStr:" << productStr << ", callRate:" << callrate;

	std::string sProductsForMpmxOnly;
	CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey("REDUCE_RATE_MPMX", sProductsForMpmxOnly);

	if (sProductsForMpmxOnly.size() > 0 && callrate > 768)
	{
		DWORD newVideoRate = 7040;
		if (strcmp("NONE", sProductsForMpmxOnly.c_str()) == 0)
			return FALSE;

		if (strcmp("ANY", sProductsForMpmxOnly.c_str()) == 0)
			return newVideoRate;

		if (::IsStringMatchFlagOfMultiNames(productStr, sProductsForMpmxOnly))
			return newVideoRate;
	}

	return FALSE; // string is empty
}

//--------------------------------------------------------------------------
BYTE IsSetHD720RsrcForUser(const char* productStr)
{
	if (productStr == NULL || strlen(productStr) == 0)
		return 0;

	FTRACEINTO << "IsSetHD720RsrcForUser - productStr:" << productStr;

	std::string sProductsForMpmxOnly;
	CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey("DO_NOT_LIMIT_TO_HD720_MPMX", sProductsForMpmxOnly);

	if (sProductsForMpmxOnly.size() > 0)
	{
		if (strcmp("NONE", sProductsForMpmxOnly.c_str()) == 0)
			return TRUE;

		if (strcmp("ANY", sProductsForMpmxOnly.c_str()) == 0)
			return FALSE;

		if (::IsStringMatchFlagOfMultiNames(productStr, sProductsForMpmxOnly))
			return FALSE;
	}

	return TRUE;
}


BOOL Is1080pSupportedInOperationPoint(DWORD confRate)
{
	
	if(confRate < 2048)
		return NO;
	
	//1080p SVC is supported on RMX1500/2000/4000 (MpmRx) and Ninja, Edge, SoftMCU.
	eProductType productType = CProcessBase::GetProcess()->GetProductType();
	if(((eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily()) && 
			(eSystemCardsMode_breeze !=GetSystemCardsBasedMode())) || 
		(eProductTypeNinja == productType) ||
		(eProductTypeSoftMCU == productType) ||
		(eProductTypeEdgeAxis == productType))
			return YES;

	return NO;

}


//--------------------------------------------------------------------------
// receive string to match with and a flag of multiple names and find if the matching string contains one of the names in the flag
// return TRUE or FALSE
BYTE IsStringMatchFlagOfMultiNames(const char* searchedStr, const std::string& BufferStr)
{
	if (searchedStr == NULL || strlen(searchedStr) == 0)
		return 0;

	char* pSubStringInFlagData = NULL;

	int length = 0;
	int i = strlen(BufferStr.c_str());
	if (i > 1)
	{
		char* pFlagString = new char[i+1];
		strcpy(pFlagString, BufferStr.c_str());
		char* pSubFlagString = new char[i+1];
		char* pStrPointer    = NULL;
		pStrPointer = pFlagString;
		do
		{
			pSubStringInFlagData = (char*)strchr(pStrPointer, ';');

			if (pSubStringInFlagData)
				length = pSubStringInFlagData - pStrPointer;
			else
				length = strlen(pStrPointer);

			if (length == 0) // for string that ends with ';
			{
				PDELETEA(pFlagString);
				PDELETEA(pSubFlagString);
				return FALSE;
			}

			strncpy(pSubFlagString, pStrPointer, length);
			pSubFlagString[length] = '\0';

			if (strstr(searchedStr, pSubFlagString)) // found a match
			{
				PDELETEA(pFlagString);
				PDELETEA(pSubFlagString);
				return TRUE;
			}

			pStrPointer = pSubStringInFlagData;
			pStrPointer++; // assuming the string can be one note!!
		}
		while (pSubStringInFlagData != NULL);

		PDELETEA(pFlagString);
		PDELETEA(pSubFlagString);
	}

	return FALSE;
}

//--------------------------------------------------------------------------
BYTE checkRsrcLimitationsByPartyType(cmCapDirection direction, CIpComMode* pScm, H264VideoModeDetails& h264VidModeDetails)
{
	long   NewFs       = 0;
	long   ScmFS       = 0;
	long   ScmMBPS     = 0;
	long   ScmSAR      = 0;
	APIU8  ScmLevel    = 0;
	APIU16 ScmProfile  = 0;
	long   ScmStaticMB = 0;
	long   ScmDPB      = 0;

	if (pScm->IsMediaOn(cmCapVideo, direction, kRolePeople))
	{
		CapEnum algorithm = (CapEnum)(pScm->GetMediaType(cmCapVideo, direction));
		if (algorithm == eH264CapCode)
			pScm->GetFSandMBPS(direction, ScmProfile, ScmLevel, ScmFS, ScmMBPS, ScmSAR, ScmStaticMB, ScmDPB);
		else if (algorithm == eRtvCapCode)
			pScm->GetRtvFSandMBPS(direction, ScmFS, ScmMBPS);
	}

	CH264Details ScmH264Details = ScmLevel;
	if (ScmFS == -1)
		ScmFS = ScmH264Details.GetDefaultFsAsDevision();

	CH264Details NewH264Details = h264VidModeDetails.levelValue;
	NewFs = h264VidModeDetails.maxFS;
	if (NewFs == -1)
		NewFs = NewH264Details.GetDefaultFsAsDevision();

	FTRACEINTO << "***NewFs:" << NewFs << ", ScmFS:" << ScmFS << ", direction:" << direction;

	if (NewFs < ScmFS)
	{
		// TIP fix for ReINVITE in rate=1024
		if ((pScm->GetIsTipMode()) && (NewFs < (long)H264_HD720_FS_AS_DEVISION))
			return FALSE;
		else
			return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------
DWORD GetMinBitRateForCopLevel(BYTE copLevelFormat, BYTE copLevelFrameRate, BYTE copLevelProtocol, DWORD copLevelBitRate)
{
	std::ostringstream msg;
	msg << "GetMinBitRateForCopLevel "
	    << "- copLevelFormat:"    << (int)copLevelFormat
	    << ", copLevelFrameRate:" << (int)copLevelFrameRate
	    << ", copLevelProtocol:"  << (int)copLevelProtocol
	    << ", copLevelBitRate:"   << copLevelBitRate;

	DWORD  retVal                       = 0;
	APIS32 tresholdAccordingToLevelRate = 0;
	DWORD  percentVal                   = 0;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("LEVEL_RATE_REDUCTION_PERCENTAGE", percentVal);

	if (percentVal > 0 && copLevelBitRate > 0)
	{
		APIS32 rateratio = 100 - percentVal;
		tresholdAccordingToLevelRate = ((rateratio * copLevelBitRate) / 100);
		FPTRACE2INT(eLevelInfoNormal, "GetMinBitRateForCopLevel - tresholdAccordingToLevelRate:", (DWORD)tresholdAccordingToLevelRate);
	}

	if (copLevelProtocol != VIDEO_PROTOCOL_H264_HIGH_PROFILE)
	{
		switch (copLevelFormat)
		{
			case eCopLevelEncoderVideoFormat_HD1080p:
				retVal = H264_COP_THRESHOLD_1080;
				break;

			case eCopLevelEncoderVideoFormat_HD720p:
				if ((copLevelFrameRate == eCopVideoFrameRate_50) || (copLevelFrameRate == eCopVideoFrameRate_60))
					retVal = H264_COP_THRESHOLD_720_50;
				else
					retVal = H264_COP_THRESHOLD_720;
				break;

			case eCopLevelEncoderVideoFormat_4CIF_16_9:
			case eCopLevelEncoderVideoFormat_4CIF:
				retVal = H264_COP_THRESHOLD_4CIF;
				break;

			case eCopLevelEncoderVideoFormat_CIF:
			case eCopLevelEncoderVideoFormat_QCIF:
				retVal = H264_COP_THRESHOLD_CIF;
				break;

			default:
				FPASSERTMSG(1, "GetMinBitRateForCopLevel - copLevelFormat not supported");
				break;
		}    // switch

	}
	else   // High Profile thresholds
	{
		switch (copLevelFormat)
		{
			case eCopLevelEncoderVideoFormat_HD1080p:
				retVal = H264_HP_COP_THRESHOLD_1080;
				break;

			case eCopLevelEncoderVideoFormat_HD720p:
				if ((copLevelFrameRate == eCopVideoFrameRate_50) || (copLevelFrameRate == eCopVideoFrameRate_60))
					retVal = H264_HP_COP_THRESHOLD_720_50;
				else
					retVal = H264_HP_COP_THRESHOLD_720;
				break;

			case eCopLevelEncoderVideoFormat_4CIF_16_9:
			case eCopLevelEncoderVideoFormat_4CIF:
				retVal = H264_HP_COP_THRESHOLD_4CIF;
				break;

			case eCopLevelEncoderVideoFormat_CIF:
			case eCopLevelEncoderVideoFormat_QCIF:
				retVal = H264_HP_COP_THRESHOLD_CIF;
				break;

			default:
				FPASSERTMSG(2, "GetMinBitRateForCopLevel - copLevelFormat not supported");
				break;
		} // switch
	}

	if (tresholdAccordingToLevelRate != 0)
		retVal = max(retVal, (DWORD)tresholdAccordingToLevelRate);

	if (percentVal == 0 && copLevelBitRate != 0)
		retVal = copLevelBitRate;

	msg << ", retVal:" << retVal;
	FTRACEINTO << msg.str().c_str();
	return retVal;
}

//--------------------------------------------------------------------------
BYTE GetMaxCopLevelForBitRate(DWORD rate)
{
	FPASSERTSTREAM_AND_RETURN_VALUE(rate < H264_COP_THRESHOLD_CIF, "ChangeH264ScmAccordingToRateForCop - Reservation rate cannot be less than H264_COP_THRESHOLD_CIF", eCopLevelEncoderVideoFormat_CIF);

	if (rate < H264_COP_THRESHOLD_4CIF)
		return eCopLevelEncoderVideoFormat_CIF;
	else if (rate < H264_COP_THRESHOLD_720)
		return eCopLevelEncoderVideoFormat_4CIF_16_9;
	else if (rate < H264_COP_THRESHOLD_1080)
		return eCopLevelEncoderVideoFormat_HD720p;

	return eCopLevelEncoderVideoFormat_CIF;
}

//--------------------------------------------------------------------------
// receive user product ID and check if m_remoteVendor.productIdit part of a flag to force CIF rsrc at the encoder.
// return TRUE or FALSE
BYTE IsForceResolutionForParty(const char* productStr)
{
	if (productStr == NULL || strlen(productStr) == 0)
		return 0;

	FTRACEINTO << "IsForceResolutionForParty - productStr:" << productStr;

	std::string sProducts;
	CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey("FORCE_RESOLUTION", sProducts);

	if (sProducts.size() > 0)
	{
		if (strcmp("NONE", sProducts.c_str()) == 0)
			return FALSE;

		if (strcmp("ANY", sProducts.c_str()) == 0)
			return TRUE;

		return ::IsStringMatchFlagOfMultiNames(productStr, sProducts);
	}

	return FALSE; // string is empty
}

//--------------------------------------------------------------------------
void IdentifyVersionId(char* pSource, char** pVersionId, char* pProductId, int sourceLen)
{
	// Beta - 2.5.0.7-4014 or Polycom HDX 7000 (Beta - 2.5.0.7-4015) for SIP
	if (pSource != NULL && pSource[0] != '\0' && pProductId != NULL && pProductId[0] != '\0')
	{
		char* pStrPointer = NULL;

		if (strstr(pProductId, "HDX"))
		{
			pStrPointer = (char*)strchr(pSource, '-');
			if(pStrPointer ==  NULL)
			{
				pStrPointer = (char*)strchr(pSource, '/');
			}
		}
		else
			pStrPointer = pSource;

		if (pStrPointer)
		{
			int j = 0;
			for (j = 0; j < sourceLen; j++)
			{
				if (pStrPointer[0] != '\0' && !isdigit(pStrPointer[0]))
					(pStrPointer)++;
				else
					break;
			} // we are now pointing to the beginning of the version number

			int newLen = sourceLen - j - 1;

			int i = 0;
			for (i = 0; i < newLen; i++)
			{
				if (!isdigit(pStrPointer[i]) && pStrPointer[i] != '.')
					break;
			} // we are now pointing to the end of the version number by i

			if (i > 0)
			{
				strncpy(*pVersionId, pStrPointer, i+1);
				(*pVersionId)[i] = '\0';
			}

			FPTRACE2(eLevelInfoNormal, "IdentifyVersionId - The VersionId is ", *pVersionId);
		}
	}
}

//--------------------------------------------------------------------------
void SetProblematicVersionId(char* pSource, char** pProblematicVersionId)
{
	if (pSource != NULL && pSource[0] != '\0')
	{
		// set the problematic version ID of the remote
		if (strstr(pSource, "HDX 800"))
		{
			strcpy(*pProblematicVersionId, "2.5.0.5");
		}
		else if ((strstr(pSource, "HDX 700")) || (strstr(pSource, "HDX 900")) || (strstr(pSource, "HDX 400")))
		{
			strcpy(*pProblematicVersionId, "2.5.0.6");
		}
		else if (strstr(pSource, "VSX 8000"))
		{
			strcpy(*pProblematicVersionId, "9.5.0.1");
		}

		// etc, etc, according to the table.
	}
}

//--------------------------------------------------------------------------
/* the function return 0 if version to check is bigger or equal than the pivot version
    the function return 1 if the pivot version is bigger than the version to check
  return 1 - meaning need to send alternitive resolution table from the encoder.
  return 0 - meaning need to send regular resolution table from the encoder
*/
int CompareTwoVersionId(char* pPivotVersionID, char* pToCheckVersionId)
{
	long int la1 = 0, la2 = 0, la3 = 0, la4 = 0;
	long int lb1 = 0, lb2 = 0, lb3 = 0, lb4 = 0;
	char*    pEnd;

	la1 = strtol(pPivotVersionID, &pEnd, 10);
	la2 = strtol(pEnd+1, &pEnd, 10);
	la3 = strtol(pEnd+1, &pEnd, 10);
	la4 = strtol(pEnd+1, &pEnd, 10);

	lb1 = strtol(pToCheckVersionId, &pEnd, 10);
	lb2 = strtol(pEnd+1, &pEnd, 10);
	lb3 = strtol(pEnd+1, &pEnd, 10);
	lb4 = strtol(pEnd+1, &pEnd, 10);

	if (la1 > lb1)
		return 1;
	else if (la1 == lb1 && la2 > lb2)
		return 1;
	else if (la1 == lb1 && la2 == lb2 && la3 > lb3)
		return 1;
	else if (la1 == lb1 && la2 == lb2 && la3 == lb3 && la4 > lb4)
		return 1;

	return 0;
}

//--------------------------------------------------------------------------
EFpsMode TranslateSysConfigStringToFpsMode(const std::string& sFpsMode)
{
	if (sFpsMode == "PAL")
		return E_FPS_PAL_25;

	if (sFpsMode == "NTSC")
		return E_FPS_NTSC_30;

	if (sFpsMode == "AUTO")
		return E_FPS_AUTO;

	return E_FPS_MODE_DUMMY;
}

//--------------------------------------------------------------------------
DWORD GetMaxRecordingLinks()
{
	DWORD maxRecordingLinks;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(CFG_KEY_MAXIMUM_RECORDING_LINKS, maxRecordingLinks);
	return maxRecordingLinks;
}

//--------------------------------------------------------------------------
eConfPartyFipsSimulationMode TranslateSysConfigDataToEnumForConfParty(std::string& data)
{
	eConfPartyFipsSimulationMode eSimValue = eInactiveSimulation;

	if (data == "FAIL_CIPHER_TEST")
		eSimValue = eFailPartyCipherFipsTest;
	else if (data == "FAIL_BYPASS_TEST")
		eSimValue = eFailBypassFipsTest;

	return eSimValue;
}

//--------------------------------------------------------------------------
void SetIsCOPdongleSysMode(BOOL i_IsCOPdongleSysMode) // 2 modes cop/cp
{
	GlobalFlags::IsCOPdongleSysMode = i_IsCOPdongleSysMode;
}

//--------------------------------------------------------------------------
BOOL GetIsCOPdongleSysMode() // 2 modes cop/cp
{
	return GlobalFlags::IsCOPdongleSysMode;
}

//--------------------------------------------------------------------------

void SetDongle1500qHDvalue(BOOL confPartyLicensing_HD)      // 1500Q	Not 1500Q
{                                                           // License bit 0	  0			1
	GlobalFlags::dongle1500qHDValue = confPartyLicensing_HD;  // License bit 1	  1			1
}

//--------------------------------------------------------------------------
BOOL GetDongle1500qHDvalue()
{
	return GlobalFlags::dongle1500qHDValue;
}

//--------------------------------------------------------------------------
void SetDongleSvcValue(BOOL confPartyLicensing_svc)
{
	GlobalFlags::dongleSvcValue = confPartyLicensing_svc;
}

//--------------------------------------------------------------------------
BOOL GetDongleSvcValue()
{
	return GlobalFlags::dongleSvcValue;
}

//--------------------------------------------------------------------------
void SetDongleCifPlusValue(BOOL confPartyLicensing_avc)
{
	GlobalFlags::dongleCifPlus = confPartyLicensing_avc;
}

//--------------------------------------------------------------------------
BOOL GetDongleCifPlusValue()
{
	return GlobalFlags::dongleCifPlus;
}

//--------------------------------------------------------------------------
void SetDongleTipInteropValue(BOOL confPartyLicensing_TipInterop)
{
	GlobalFlags::dongleTipInteropValue = confPartyLicensing_TipInterop;
}

//--------------------------------------------------------------------------
BOOL GetDongleTipInteropValue()
{
	return GlobalFlags::dongleTipInteropValue;
}

BOOL GetDongleLicenseExpiredValue()
{
	return GlobalFlags::dongleLicenseValid;
}

//--------------------------------------------------------------------------
void SetDongleLicenseExpiredValue(BOOL isLicenseValid)
{
	GlobalFlags::dongleLicenseValid = isLicenseValid;
}

//--------------------------------------------------------------------------

Eh264VideoModeType GetMaxVideoModeByResolutionType(EVideoResolutionType resType, eVideoQuality rsrvVidQuality, DWORD decisionRate)
{
	const char* descr = NULL;
	BYTE res = CStringsMaps::GetDescription(RESOLUTION_SLIDER_ENUM, resType, &descr);
	if (res)
		FTRACEINTO << "GetMaxVideoModeByResolutionType - maxResolution:" << descr;

	Eh264VideoModeType resourceMaxVideoMode = IsFeatureSupportedBySystem(eFeatureHD1080p60Asymmetric) ? eHD1080At60Asymmetric : eHD1080At60Symmetric;
	switch (resType)
	{
		case eAuto_Res:
		case eHD1080p60_Res:
		case eHD1080_Res:
		{
			if (eVideoQualityMotion == rsrvVidQuality)
			{
				if (IsFeatureSupportedBySystem(eFeatureHD1080p60Symmetric))
					resourceMaxVideoMode = eHD1080At60Symmetric;
				else if (IsFeatureSupportedBySystem(eFeatureHD1080p60Asymmetric))
					resourceMaxVideoMode = eHD1080At60Asymmetric;
				else
					resourceMaxVideoMode = eHD720At60Symmetric;
			}
			else
			{
				if (IsFeatureSupportedBySystem(eFeatureHD1080p60Symmetric))
					resourceMaxVideoMode = eHD1080At60Symmetric;
				else
					resourceMaxVideoMode = eHD1080Symmetric;
			}
			if(decisionRate && decisionRate < 2048000 )
			{ // 1080p60 not supported in rate < 2048k
				resourceMaxVideoMode = eHD1080Symmetric ;
			}
			break;
		}

		case eCIF_Res:
		{
			if (eVideoQualityMotion == rsrvVidQuality)
				resourceMaxVideoMode = eWCIF60;
			else
				resourceMaxVideoMode = eCIF30;
			break;
		}

		case eSD_Res:
		{
			// vngr-15694 : i="SD" in profile means SD60 in MPMx,MPM+ motion
			if (eVideoQualityMotion == rsrvVidQuality)
				resourceMaxVideoMode = eSD60;
			else
				resourceMaxVideoMode = eW4CIF30;   // max sd value is eSD60 ?
			break;
		}

		case eHD720_Res:
		{
			if (eVideoQualityMotion == rsrvVidQuality)
				resourceMaxVideoMode = eHD720At60Symmetric;
			else
				resourceMaxVideoMode = eHD720Symmetric;
			break;
		}
	} // switch

	return resourceMaxVideoMode;
}

//BRIDGE-12596
//--------------------------------------------------------------------------
eVideoPartyType GetMaxVideoPartyTypeByVideoQuality(eVideoPartyType videoPartyType, eVideoQuality rsrvVidQuality)
{
	eVideoPartyType choosenVideoPartyType = videoPartyType;

	if(rsrvVidQuality == eVideoQualitySharpness)
	{
		switch (videoPartyType)
		{
			case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
			{
				choosenVideoPartyType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type ;
				break;
			}
			case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:
			{
				choosenVideoPartyType = eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
				break;
			}
			default:
			{
				choosenVideoPartyType = videoPartyType;
				break;
			}
		} // switch
	}

	FTRACEINTO << "rsrvVidQuality " << (int)rsrvVidQuality << " videoPartyType: " << eVideoPartyTypeNames[videoPartyType] << " choosenVideoPartyType: " << eVideoPartyTypeNames[choosenVideoPartyType];
	return choosenVideoPartyType;
}
//--------------------------------------------------------------------------
void DumpMcuInternalDetailed(CLargeString& cstr, DWORD faultOpcode)
{
	CLargeString cstr1;
	cstr1 << "################################################ MCU INTERNAL PROBLEM ######################################################\n";
	cstr1 << "##\n";
	cstr1 << "##\n";
	// Party:1 Conf:1 receives Failure Status for opcode: TB_MSG_OPEN_PORT_REQ from: video encoder req:53
	cstr1 << "##   " << cstr << "\n";
	cstr1 << "##\n";
	cstr1 << "##\n";
	cstr1 << "############################################################################################################################";


	FPTRACE(eLevelError, cstr1.GetString());

	CLargeString cstr2;
	cstr2 << "McuInternalProblem - " <<  cstr;
	CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, faultOpcode, MAJOR_ERROR_LEVEL, cstr2.GetString(), TRUE);
}

//--------------------------------------------------------------------------
eVideoPartyType GetLowestVideoAllocationAccordingToSystemMode(char* isH263)
{
	if (isH263)
		return eCP_H261_H263_upto_CIF_video_party_type;
	else
		return eCP_H264_upto_CIF_video_party_type;
}

//--------------------------------------------------------------------------
BOOL isVendorSupportFullAudioCaps(const char* cUserAgent, const char* pVersionId /*=NULL*/)
{
	BOOL bIsAlwaysPartialAudioCaps = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SIP_ALWAYS_USE_PARTIAL_AUDIO_CAPS);
	if (bIsAlwaysPartialAudioCaps)
	{
		FPTRACE(eLevelInfoNormal, "isVendorSupportFullAudioCaps - Always using Partial Audio Caps");
		return false;
	}

	BOOL bIsVendorSupportFullAudioCaps = TRUE;

	// check user agent
	if (strstr(cUserAgent, UA_SONY_PRODUCT_NAME) || strstr(cUserAgent, UA_MOC_PRODUCT_NAME) ||
	    strstr(cUserAgent, UA_PVX_PRODUCT_NAME) || strstr(cUserAgent, UA_TANDBERG_PRODUCT_NAME) ||
	    strstr(cUserAgent, UA_UCCAPI_PRODUCT_NAME) || strstr(cUserAgent, UA_CUCM_PRODUCT_NAME) ||
	    strstr(cUserAgent, UA_OCPHONE_PRODUCT_NAME))
	{
		bIsVendorSupportFullAudioCaps = FALSE;
	}
	return bIsVendorSupportFullAudioCaps;
}

//--------------------------------------------------------------------------
APIU16 BuildRtcpCnameMask(BYTE isRoomSwitch)
{
	APIU16 mask = 0x0001; // 1 in the LSD indicates we are MCU and not EP;
	if (isRoomSwitch)
		mask = mask | ITP_ZOOM_IN_MODE;
	else
		mask = mask | ITP_ZOOM_OUT_MODE;

	return mask;
}

//--------------------------------------------------------------------------
bool operator<(const stBoardUnitParams& lhs, const stBoardUnitParams& rhs)
{
	return ((lhs.m_boardId < rhs.m_boardId) &&
	        (lhs.m_subBoardId < rhs.m_subBoardId) &&
	        (lhs.m_unitId < rhs.m_unitId));
}

//--------------------------------------------------------------------------
APIU16 GetProfileAccordingToCopProtocol(BYTE protocol)
{
	if (protocol == VIDEO_PROTOCOL_H264_HIGH_PROFILE)
		return H264_Profile_High;
	else if (protocol == VIDEO_PROTOCOL_H264)
		return H264_Profile_BaseLine;
	else
	{
		FPASSERT(protocol+1000);
		return H264_Profile_BaseLine;
	}
}

//--------------------------------------------------------------------------
BYTE GetMaxCopLevelForBitRateForBaseline(DWORD rate)
{
	FPASSERTSTREAM_AND_RETURN_VALUE(rate < H264_COP_THRESHOLD_CIF, "GetMaxCopLevelForBitRateForBaseline - Reservation rate cannot be less than H264_COP_THRESHOLD_CIF", eCopLevelEncoderVideoFormat_HD1080p);

	if (rate < H264_COP_THRESHOLD_4CIF)
		return eCopLevelEncoderVideoFormat_CIF;
	else if (rate < H264_COP_THRESHOLD_720)
		return eCopLevelEncoderVideoFormat_4CIF_16_9;
	else if (rate < H264_COP_THRESHOLD_1080)
		return eCopLevelEncoderVideoFormat_HD720p;

	return eCopLevelEncoderVideoFormat_HD1080p;
}

//--------------------------------------------------------------------------
BYTE isNeedToChangeResOfBaselineAccordingToRate(DWORD rate, long currentFS)
{
	sCopEncoderFormatConfig copEncoderFormatConfig;
	copEncoderFormatConfig.encoderFormat = ((ECopLevelEncoderVideoFormat)GetMaxCopLevelForBitRateForBaseline(rate));
	copEncoderFormatConfig.encoderFrameRate = eCopVideoFrameRate_30; // fps is not relevant for res checking

	sCopH264VideoMode  copH264VideoMode;
	CCopVideoModeTable copTble;

	long maxFsBaseLineAllowed = 0;
	WORD encoderIndex = copTble.GetEncoderH264Mode(copEncoderFormatConfig, copH264VideoMode);
	if (encoderIndex != (WORD)-1)
		maxFsBaseLineAllowed = copH264VideoMode.maxFS;
	else
	{
		FPTRACE(eLevelError, "isNeedToChangeResOfBaselineAccordingToRate - No encoder index found, continue with current fs");
		maxFsBaseLineAllowed = currentFS;
	}

	if (maxFsBaseLineAllowed == -1)
	{
		CH264Details MaxResDetailes = copH264VideoMode.levelValue;
		maxFsBaseLineAllowed = MaxResDetailes.GetDefaultFsAsProduct();
	}

	maxFsBaseLineAllowed = GetMaxFsAsDevision(maxFsBaseLineAllowed);
	maxFsBaseLineAllowed = maxFsBaseLineAllowed * CUSTOM_MAX_FS_FACTOR; // to make sure rounding up
	if (currentFS > maxFsBaseLineAllowed)
	{
		FPTRACE2INT(eLevelInfoNormal, "isNeedToChangeResOfBaselineAccordingToRate - Current fs are higher than allowed according to baseline threshold, currentFS:", currentFS);
		return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------------
BYTE GetEncoderParamsForNewResOnH264BaseLineCap(DWORD rate, ECopVideoFrameRate highestframerate, long& levelValue, long& maxMBPS, long& maxFS, long& maxDPB, long& maxBR, long& maxCPB, long& maxSAR, long& maxStaticMbps)
{
	sCopEncoderFormatConfig copEncoderFormatConfig;
	copEncoderFormatConfig.encoderFormat = ((ECopLevelEncoderVideoFormat)GetMaxCopLevelForBitRateForBaseline(rate));
	if (highestframerate > eCopVideoFrameRate_30)
		highestframerate = eCopVideoFrameRate_30;

	copEncoderFormatConfig.encoderFrameRate = highestframerate;

	sCopH264VideoMode  copH264VideoMode;
	CCopVideoModeTable copTble;
	WORD encoderIndex = copTble.GetEncoderH264Mode(copEncoderFormatConfig, copH264VideoMode);
	if (encoderIndex != (WORD)-1)
	{
		FPTRACE2INT(eLevelInfoNormal, "GetEncoderParamsForNewResOnH264BaseLineCap - This is chosen encoder index, encoderIndex:", encoderIndex);
		copTble.GetValuesAsDevision(copH264VideoMode);
		levelValue    = copH264VideoMode.levelValue;
		maxMBPS       = copH264VideoMode.maxMBPS;
		maxFS         = copH264VideoMode.maxFS;
		maxDPB        = copH264VideoMode.maxDPB;
		maxBR         = copH264VideoMode.maxBR;
		maxCPB        = copH264VideoMode.maxCPB;
		maxSAR        = copH264VideoMode.maxSAR;
		maxStaticMbps = copH264VideoMode.maxStaticMbps;
	}

	return encoderIndex;
}

//--------------------------------------------------------------------------
//Multiple links for ITP in cascaded conference feature: GetITPparams
BOOL GetITPparams(const char* pH323useruserField, BYTE& cascadedLinksNumber, BYTE& index, eTypeOfLinkParty& linkType, DWORD& unrsrvMainLinkDialInNumber, BYTE interfaceType)
{
	//check if we have 3 comma
	//std::string s ( pH323useruserField, min(pH323useruserField,MaxUserUserSize-1) );
	//int countComma = std::count(s.begin(), s.end(), ',');

	BOOL answer = TRUE;

	if ((pH323useruserField != NULL) && (interfaceType == H323_INTERFACE_TYPE) && (strcmp(pH323useruserField, "") != 0))
	{
		//#define maxUserUserSize 128
		//WORD maxUserUserSize = 128;

		char partyTypeIs[MaxUserUserSize];
		memset(partyTypeIs, '\0', MaxUserUserSize);

		char *posCharOfFirstComma = (char*)strchr(pH323useruserField, ',');
		if (posCharOfFirstComma != NULL)
		{
			int pos = posCharOfFirstComma - pH323useruserField;
			if (pos > MaxUserUserSize - 1)
			{
				FPTRACE2INT(eLevelError, "ITP_CASCADE: GetITPparams NOTICE (pos > MaxUserUserSize-1) pos:", pos);
				return FALSE;
			}

			strcpy_safe(partyTypeIs, pH323useruserField);

			if (strncmp(partyTypeIs, "Main", 4) == 0)
				linkType = eMainLinkParty;
			else if (strncmp(partyTypeIs, "Sub", 3) == 0)
				linkType = eSubLinkParty;
			else
			{
				FPTRACE(eLevelError, "ITP_CASCADE: GetITPparams NOTICE this is no Sub or Main");
				return FALSE;
			}

			posCharOfFirstComma++;

			if (posCharOfFirstComma)
			{
				char *posCharOfNextComma = (char*)strchr(posCharOfFirstComma, ',');

				if (posCharOfNextComma != NULL)
				{
					pos = posCharOfNextComma - posCharOfFirstComma;

					int check_pos = posCharOfNextComma - pH323useruserField;
					if (check_pos > MaxUserUserSize - 1)
					{
						FPTRACE(eLevelError, "ITP_CASCADE: GetITPparams NOTICE (check_pos > MaxUserUserSize-1)");
						return FALSE;
					}

					char partyIndexIs[4];
					memset(partyIndexIs, '\0', 4);

					if (pos > 3)
					{
						FPTRACE(eLevelError, "ITP_CASCADE: GetITPparams NOTICE (pos > 3)");
						return FALSE;
					}

					strncpy(partyIndexIs, posCharOfFirstComma, pos);
					partyIndexIs[pos] = '\0';

					index = (BYTE)atoi(partyIndexIs);

					char *mainLinkOfDialInNumber;
					mainLinkOfDialInNumber = (char*)strrchr(pH323useruserField, ',');  //find the last ',' in the string
					check_pos = mainLinkOfDialInNumber - pH323useruserField;

					if (check_pos > MaxUserUserSize - 1)
					{
						FPTRACE(eLevelError, "ITP_CASCADE: GetITPparams NOTICE (check_pos > MaxUserUserSize-1)");
						return FALSE;
					}

					if (mainLinkOfDialInNumber != NULL)
					{
						char numOfLink[4];

						memset(numOfLink, '\0', 4);
						pos = mainLinkOfDialInNumber - posCharOfNextComma;
						if (pos > 3)
						{
							FPTRACE(eLevelError, "ITP_CASCADE: GetITPparams NOTICE (pos > 3)");
							return FALSE;
						}

						strncpy(numOfLink, posCharOfNextComma, pos);
						numOfLink[pos] = '\0';

						mainLinkOfDialInNumber++;

						if (numOfLink[1] != '\0' && mainLinkOfDialInNumber != NULL)
						{
							cascadedLinksNumber = (BYTE)atoi(&numOfLink[1]);
							unrsrvMainLinkDialInNumber = (BYTE)atoi(mainLinkOfDialInNumber);

							FTRACESTRFUNC(eLevelError) << "cascadedLinksNumber:" << cascadedLinksNumber << ", index:" << index << ", linkType:" << linkType << ", unrsrvMainLinkDialInNumber:" << unrsrvMainLinkDialInNumber << " - ITP_CASCADE: DialIn";

						}
						else
							answer = FALSE;
					}
					else
						answer = FALSE;
				}
				else
					answer = FALSE;
			}
			else
				answer = FALSE;
		}
		else
			answer = FALSE;
	}
	else
		answer = FALSE;

	return answer;
}

////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: GetMainLinkName
void GetMainLinkName(const char *subPartyName, char *mainPartyName)
{
    memset(mainPartyName, '\0', H243_NAME_LEN);

    char *posChar = (char*)strrchr(subPartyName,'_');

    if (posChar)
    {
        int pos = posChar - subPartyName;
        strncpy(mainPartyName, subPartyName, pos+1);
        mainPartyName[pos+1] = '\0';
    }
    else
    {
        strcpy(mainPartyName, subPartyName);
        strcat(mainPartyName,"_");
    }

    strcat(mainPartyName,"1");
    strcat(mainPartyName,"\0");
}

////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: GetSubLinkName
void GetSubLinkName(const char *mainPartyName, BYTE index, char *subPartyName)
{
    memset(subPartyName, '\0', H243_NAME_LEN);

    char *posChar = (char*)strrchr(mainPartyName ,'_');

    int pos = 0;

    if (posChar)
        pos = posChar - (mainPartyName);

    char  indexOfSub [5];
    snprintf(indexOfSub,sizeof(indexOfSub),"%d",index);

    if (pos > 0 )
    {
        strncpy(subPartyName, (mainPartyName), pos+1);
        subPartyName[pos+1] = '\0';
        strcat(subPartyName, indexOfSub);
    }
    else
    {
        strcpy(subPartyName,mainPartyName);
        strcat(subPartyName, "_");
        strcat(subPartyName, indexOfSub);
    }

    strcat(subPartyName,"\0");

}

////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CreateMainLinkName
void CreateMainLinkName(const char *originalPartyName, char *mainPartyName)
{
    memset(mainPartyName, '\0', H243_NAME_LEN);

    strncpy(mainPartyName, originalPartyName, H243_NAME_LEN - 3);

    strcat(mainPartyName,"_1");
    strcat(mainPartyName,"\0");
}

////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CreateSubLinkName
void CreateSubLinkName(const char *originalPartyName, BYTE index, char *subPartyName)
{
    memset(subPartyName, '\0', H243_NAME_LEN);
    //strcat(subPartyName, originalPartyName);
    strncpy(subPartyName, originalPartyName, H243_NAME_LEN - 3);

    char  indexOfLink [5];
    snprintf(indexOfLink,sizeof(indexOfLink),"%d",index);

    strcat(subPartyName, "_");
    strcat(subPartyName, indexOfLink);
    strcat(subPartyName,"\0");
}

/////////////////////////////////////////////////////////////////////////////
Eh264VideoModeType GetMaxH264VideoModeForMsSvcAccordingToSettings( CConfParty* pConfParty, const CCommConf* pCommConf)
{
	//const CCommConf* pCommConf = m_pConf->GetCommConf();

	if(!pCommConf)
	{
		FPTRACE(eLevelError,"GetMaxH264VideoModeForMsSvcAccordingToSettings : error no comconf");
		return eInvalidModeType;
	}

	EVideoResolutionType maxConfResolution  = (EVideoResolutionType)(pCommConf->GetConfMaxResolution());
	BYTE partyResolution =  eAuto_Res;
	if (pConfParty)
		partyResolution =  pConfParty->GetMaxResolution();

	BYTE maxPartyResolution = eAuto_Res;
	if(partyResolution != eAuto_Res)
		maxPartyResolution = partyResolution;
	else if (maxConfResolution != eAuto_Res)
		maxPartyResolution = maxConfResolution;

	Eh264VideoModeType partyMaxVideoMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)maxPartyResolution,eVideoQualitySharpness );
	partyMaxVideoMode = GetMaxMsSvcVideoModeByFlag(partyMaxVideoMode);

	//========================================================================================
	// Adding consideration for the maximum resolution specified above the resolution slider
	//========================================================================================
	Eh264VideoModeType systemMaxVideoMode = GetMaxVideoModeBySysCfg();
	if (eSD30 == systemMaxVideoMode) systemMaxVideoMode = eW4CIF30;
	Eh264VideoModeType chosenMaxVideoMode = min(partyMaxVideoMode, systemMaxVideoMode);

	CSmallString log;
	log << "partyMaxVideoMode[" << partyMaxVideoMode << "], systemMaxVideoMode[" << systemMaxVideoMode << "], chosenMaxVideoMode[" << chosenMaxVideoMode << "]";

	FPTRACE2(eLevelInfoNormal,"GetMaxH264VideoModeForMsSvcAccordingToSettings - ", log.GetString());

	return chosenMaxVideoMode;

}
////////////////////////////////////////////////////////////////////////////
Eh264VideoModeType GetMaxMsSvcVideoModeByFlag(Eh264VideoModeType partyVideoMode)
{
	std::string max_mssvc_protocol_str;
    CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL res = pSysConfig->GetDataByKey(CFG_MAX_MSSVC_RESOLUTION, max_mssvc_protocol_str);
	FPASSERTSTREAM(!res, "CSysConfig::GetDataByKey: " << CFG_MAX_MSSVC_RESOLUTION);
	FPTRACE2(eLevelInfoNormal,"GetMaxMsSvcVideoModeByFlag - MAX_MS_SVC_RESOLUTION flag is: ", (max_mssvc_protocol_str.c_str()));

	BOOL bIsAuto      = ("AUTO" == max_mssvc_protocol_str);
	BOOL bForceHd1080 = ("HD1080" == max_mssvc_protocol_str);
	BOOL bForceHd720  = ("HD720" == max_mssvc_protocol_str);
	BOOL bForceVga    = ("VGA" == max_mssvc_protocol_str);
//	BOOL bForceCif    = ("CIF" == max_mssvc_protocol_str);
//	BOOL bForceQCif   = ("QCIF" == max_mssvc_protocol_str);


	if (bIsAuto || bForceHd1080)
	{
		return partyVideoMode;
	}
	if (bForceHd720)
	{
		return (partyVideoMode >= eHD720At60Asymmetric) ? eHD720Symmetric : partyVideoMode;
	}
	if (bForceVga)
	{
		return (partyVideoMode >= eSD60) ? eW4CIF30 : partyVideoMode;
	}

	//bForceCif || bForceQCif
	return eCIF30;
}
////////////////////////////////////////////////////////////////////////////////
//TIP call from Polycom EPs feature: CheckIfRemoteSdpIsTipCompatible
BYTE CheckIfRemoteSdpIsTipCompatible(const CSipCaps* pCurRemoteCaps,BYTE checkAlsoVideoCap)
{
    BYTE bIsTipAudio                  = FALSE;
    BYTE bIsTipVideo                  = TRUE;
    BYTE bIsTipResolution             = FALSE;
    BYTE bIsRemoteSdpIsTipCompatible  = TRUE;

    if (pCurRemoteCaps->GetIsContainingCapCode(cmCapAudio, eAAC_LDCapCode))
        bIsTipAudio = TRUE;

    if (checkAlsoVideoCap==TRUE && pCurRemoteCaps->GetH264ProfileFromCapCode(cmCapVideo)!=H264_Profile_Main)
        bIsTipVideo = FALSE;

    if (pCurRemoteCaps->GetIsTipResolution())
        bIsTipResolution = TRUE;

    if (!bIsTipAudio || !bIsTipResolution || !bIsTipVideo)
        bIsRemoteSdpIsTipCompatible = FALSE;

    CLargeString str;
    str << " bIsTipAudio: " << bIsTipAudio <<  ", bIsTipResolution: " << bIsTipResolution << ", bIsRemoteSdpIsTipCompatible: " << bIsRemoteSdpIsTipCompatible;
    FPTRACE2(eLevelInfoNormal, "IS_PREFER_TIP_MODE: ::CheckIfRemoteSdpIsTipCompatible (1=TRUE): ", str.GetString());

    return bIsRemoteSdpIsTipCompatible;
}

////////////////////////////////////////////////////////////////////////////////
//TIP call from Polycom EPs feature: IsNeedToRejectTheCallForPreferTIPmode
BYTE IsNeedToRejectTheCallForPreferTIPmode(const CSipCaps* pCurRemoteCaps)
{
    BYTE bIsTipAudio                            = FALSE;
    BYTE bIsTipResolution                       = FALSE;
    BYTE bIsNeedToRejectTheCallForPreferTIPmode = FALSE;

    if (pCurRemoteCaps->GetIsContainingCapCode(cmCapAudio, eAAC_LDCapCode))
        bIsTipAudio = TRUE;

    if (pCurRemoteCaps->GetIsTipResolution())
        bIsTipResolution = TRUE;

    if (bIsTipAudio && !bIsTipResolution)
        bIsNeedToRejectTheCallForPreferTIPmode = TRUE;  //reject the call

    CLargeString str;
    str << " bIsTipAudio: " << bIsTipAudio <<  ", bIsTipResolution: " << bIsTipResolution << ", bIsNeedToRejectTheCallForPreferTIPmode: " << bIsNeedToRejectTheCallForPreferTIPmode;
    FPTRACE2(eLevelInfoNormal, "IS_PREFER_TIP_MODE: ::IsNeedToRejectTheCallForPreferTIPmode (1=TRUE): ", str.GetString());

    return bIsNeedToRejectTheCallForPreferTIPmode;
}

////////////////////////////////////////////////////////////////////////////////
BYTE GetCopFrameRateAccordingtoMbpsAndFs(BYTE level, long maxMBPS, long maxFS)
{
	int fps = 0;
	if (maxFS == -1)
	{
		CH264Details MaxResDetailes = level;
		maxFS = MaxResDetailes.GetDefaultFsAsProduct();
	}
	else
		maxFS = maxFS *CUSTOM_MAX_FS_FACTOR;

	if (maxMBPS == -1)
	{
		CH264Details MaxResDetailes = level;
		maxMBPS = MaxResDetailes.GetDefaultMbpsAsProduct();
	}
	else
		maxMBPS = maxMBPS* CUSTOM_MAX_MBPS_FACTOR;

	fps = (int)(maxMBPS/maxFS);
	FPASSERTSTREAM_AND_RETURN_VALUE(fps <= 12, "GetCopFrameRateAccordingtoMbpsAndFs - fps two low, fps:" << fps, eCopVideoFrameRate_12_5);

	FPTRACE2INT(eLevelInfoNormal, "GetCopFrameRateAccordingtoMbpsAndFs - this is fps ", fps);

	if (fps <= 15)
		return eCopVideoFrameRate_15;

	if (fps <= 25)
		return eCopVideoFrameRate_25;
	else
		return eCopVideoFrameRate_30;
}

//--------------------------------------------------------------------------
BYTE IsPalFrameRate(eVideoFrameRate videoFrameRate)
{
	BYTE retVal = NO;
	switch (videoFrameRate)
	{
		case eVideoFrameRate12_5FPS:
		case eVideoFrameRate25FPS:
		case eVideoFrameRate50FPS:
		{
			retVal = YES;
			break;
		}
		case eVideoFrameRate7_5FPS:
		case eVideoFrameRate15FPS:
		case eVideoFrameRate30FPS:
		case eVideoFrameRate60FPS:
		{
			retVal = NO;
			break;
		}
		case eVideoFrameRate3FPS:
		case eVideoFrameRate5FPS:
		case eVideoFrameRate6FPS:
		case eVideoFrameRate10FPS:
		default:
			FPASSERTSTREAM_AND_RETURN_VALUE(1, "IsPalFrameRate - Not PAL nor NTSC resolution, videoFrameRate:" << (WORD)videoFrameRate, NO);
			break;
	} // switch

	return retVal;
}

//--------------------------------------------------------------------------
eVideoResolution TranslateFSToH263H261CopEncoderResolutions(DWORD maxFS)
{
	eVideoResolution videoResolution = eVideoResolutionQCIF;
	maxFS = maxFS*CUSTOM_MAX_FS_FACTOR;
	if (maxFS > H264_L1_DEFAULT_FS)
	{
		if (maxFS > H264_L1_2_DEFAULT_FS)
			videoResolution = eVideoResolution4CIF;
		else
			videoResolution = eVideoResolutionCIF;
	}

	FTRACEINTO << "TranslateFSToH263H261CopEncoderResolutions - videoResolution:" << VideoResolutionAsString[videoResolution];
	return videoResolution;
}

//--------------------------------------------------------------------------
DWORD GetPartyArtWeight(DWORD inRateInKunits, DWORD outRateInKunits, BYTE isLpr, BYTE isEncryption, BYTE isAudioStereo, BYTE isSirenFamily)
{
	DWORD totalRateWeight = inRateInKunits + outRateInKunits;
	totalRateWeight = (totalRateWeight * (100 + 15*isLpr +25*isSirenFamily + 25*isAudioStereo + 25*isEncryption)) / 100;

	FPTRACE2INT(eLevelInfoNormal, "GetPartyArtWeight - totalRateWeight:", totalRateWeight);
	return totalRateWeight;
}

//--------------------------------------------------------------------------
BOOL isPartyMeetContentRateThreshold(DWORD confContentRate, DWORD partyContentRate, BYTE confEnterpriseMode, BYTE confPresentationProtocol, DWORD &thresholdRate)
{
	DWORD dwConetentThresholdRate = 0;

	BOOL  isPartyMeetContentRateThreshold = FALSE;

	if (confPresentationProtocol == eH264Fix)
	{
		if (partyContentRate >= confContentRate)
		{
			isPartyMeetContentRateThreshold = TRUE;
		}
		else
		{
			FTRACEINTO << "isPartyMeetContentRateThreshold - H264 Fixed Mode, party does not meet content Rate threshold" << ", partyContentRate:" << partyContentRate;
			thresholdRate = confContentRate;
		}

		return isPartyMeetContentRateThreshold;
	}

	if (confPresentationProtocol != eH264Fix)
	{
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();

		switch (confEnterpriseMode)
		{
			case eGraphics:
			{
				pSysConfig->GetDWORDDataByKey(CFG_KEY_H264_HD_GRAPHICS_MIN_CONTENT_RATE, dwConetentThresholdRate);
				break;
			}
			case eHiResGraphics:
			{
				pSysConfig->GetDWORDDataByKey(CFG_KEY_H264_HD_HIGHRES_MIN_CONTENT_RATE, dwConetentThresholdRate);
				break;
			}
			case eLiveVideo:
			{
				pSysConfig->GetDWORDDataByKey(CFG_KEY_H264_HD_LIVEVIDEO_MIN_CONTENT_RATE, dwConetentThresholdRate);
				break;
			}
		} // switch

		if (!dwConetentThresholdRate)  // No threshold
		{
			isPartyMeetContentRateThreshold = 1;
			return isPartyMeetContentRateThreshold;
		}
		else
		{
			dwConetentThresholdRate = min(confContentRate, dwConetentThresholdRate);
		}

		if (partyContentRate >= dwConetentThresholdRate)
		{
			isPartyMeetContentRateThreshold = 1;
			return isPartyMeetContentRateThreshold;
		}
		else
		{
			FTRACEINTO << "isPartyMeetContentRateThreshold - party does not meet content Rate threshold, ConetentThresholdRate:" << dwConetentThresholdRate << ", partyContentRate:" << partyContentRate << ", confPresentationProtocol:" << confPresentationProtocol;
			thresholdRate = dwConetentThresholdRate;
		}
	}

	return isPartyMeetContentRateThreshold;
}

//--------------------------------------------------------------------------
BYTE IsIntermediateSDRes(const char* productStr)
{
	if (productStr == NULL || strlen(productStr) == 0)
		return 0;

	return (strstr(productStr, "Tandberg MXP")) ? TRUE : FALSE;
}

//--------------------------------------------------------------------------
// In this function we check if all the parameters in the operation point with the firstlayerId
// Are equal or smaller than the parameters in the operation  point with the second layerId
bool IsOperationPointContainedInOtherOperationPoint(int firstLayerId, int secondLayerId, const CVideoOperationPointsSet* videoOperationPointsSet)
{
	std::ostringstream omsg;
	omsg << "firstLayerId:"<< firstLayerId << ", secondLayerId:"<< secondLayerId;

	VideoOperationPoint firstVideoOperationPoint;
	VideoOperationPoint secondVideOperationPoint;

	bool ret = true;
	bool foundFirst  = false;
	bool foundSecond = false;

	std::list <VideoOperationPoint>::const_iterator itr = videoOperationPointsSet->GetOperationPointsList()->begin();
	while ((itr != videoOperationPointsSet->GetOperationPointsList()->end()) && !(foundFirst && foundSecond))
	{
		if ((*itr).m_layerId == firstLayerId)
		{
			firstVideoOperationPoint = *itr;
			foundFirst = true;
		}

		if ((*itr).m_layerId == secondLayerId)
		{
			secondVideOperationPoint = *itr;
			foundSecond = true;
		}

		itr++;
	}

	if (!(foundFirst && foundSecond))
	{
		ret = false;
		omsg << " - Didn't find the operation points, foundFirst:" << (int)foundFirst << ", foundSecond:" << (int)foundSecond << " ===> ret = false";
		FTRACEINTO << omsg.str().c_str();
		return ret;
	}

	// we need to verify that the bit rate <=
	if (firstVideoOperationPoint.m_maxBitRate > secondVideOperationPoint.m_maxBitRate)
	{
		ret = false;
		omsg << " - m_maxBitRate is greater ===> ret = false";
		FTRACEINTO << omsg.str().c_str();
		return ret;
	}

	// we need to verify that the resolution frame size <=
	int firstFrameSize  = firstVideoOperationPoint.m_frameWidth*firstVideoOperationPoint.m_frameHeight/256;
	int secondFrameSize = secondVideOperationPoint.m_frameWidth*secondVideOperationPoint.m_frameHeight/256;
	if (firstFrameSize > secondFrameSize)
	{
		ret = false;
		omsg << " - frame size is greater ===> ret = false";
		FTRACEINTO << omsg.str().c_str();
		return ret;
	}

	// we need to verify that the max MBPS  <=
	unsigned long firstMBPS  = firstFrameSize*firstVideoOperationPoint.m_frameRate/256;
	unsigned long secondMBPS = secondFrameSize*secondVideOperationPoint.m_frameRate/256;
	if (firstMBPS > secondMBPS)
	{
		ret = false;
		omsg << " - firstMBPS:" << firstMBPS << ", secondMBPS:" << secondMBPS << ", mbps is greater ===> ret = false";
		FTRACEINTO << omsg.str().c_str();
		return ret;
	}

	omsg << " ===> ret = true";
	FTRACEINTO << omsg.str().c_str();
	return ret;
}

//--------------------------------------------------------------------------
unsigned long CalcOperationPointMBPS(const VideoOperationPoint& videoOperationPoint)
{
    int frameSize = (videoOperationPoint.m_frameWidth*videoOperationPoint.m_frameHeight)>>8;
    unsigned long aMBPS = (frameSize*videoOperationPoint.m_frameRate)>>8;
    return aMBPS;
}


unsigned long CalcOperationPointFS(const VideoOperationPoint& videoOperationPoint)
{
    int frameSize = (videoOperationPoint.m_frameWidth*videoOperationPoint.m_frameHeight)>>8;
    return frameSize;
}
//--------------------------------------------------------------------------
long CalcFSforVswRelay(const VideoOperationPoint& videoOperationPoint)
{
	long fs = NA;
	fs = (videoOperationPoint.m_frameWidth * videoOperationPoint.m_frameHeight) >> 8; // FS is in 256 units

	// convert to signaling units (round-up):
	if (fs != NA)
		fs = (fs % CUSTOM_MAX_FS_FACTOR) ? (fs/CUSTOM_MAX_FS_FACTOR + 1) : (fs/CUSTOM_MAX_FS_FACTOR);

	return fs;
}
//--------------------------------------------------------------------------
long CalcMBPSforVswRelay(const VideoOperationPoint& videoOperationPoint)
{
	//FSN-613: Dynamic Content for SVC/Mix Conf, only support 1080p30, no other FR
	if (videoOperationPoint.m_rsrcLevel == eResourceLevel_HD1080)
		return GetMaxMbpsAsDevision(H264_HD1080_30_MBPS);
	
	long mbps = NA;
	long fs = (videoOperationPoint.m_frameWidth * videoOperationPoint.m_frameHeight) >> 8; // FS is in 256 units  
	mbps = (fs * videoOperationPoint.m_frameRate) >> 8; // because frame rate should be divided by 256

	// convert to signaling units (round-down):
	if (mbps != NA)
		mbps =  mbps/CUSTOM_MAX_MBPS_FACTOR;

	return mbps;
}
//--------------------------------------------------------------------------
bool SetPredefinedH264ParamsForVswRelayIfNeeded(EOperationPointPreset eOPPreset, APIU8 &level, long &fs, long &mbps, eIsUseOperationPointsPreset isUseOperationPointesPresets, const VideoOperationPoint &operationPoint)
{
	level = 0;
	mbps = NA;
	fs = NA;

	FTRACEINTO << "eOPPreset: " << eOPPreset << ", isUseOperationPointesPresets: " << isUseOperationPointesPresets;

	if (eIsUseOPP_No == isUseOperationPointesPresets)
	{
		// For 240p we use the default valuses of CIF levels:
		if ((operationPoint.m_frameHeight==240) && (operationPoint.m_frameWidth==424))
		{
			FTRACEINTO << "frameHeight: 240p, frameWidth: 424, " << "frameRate: " << operationPoint.m_frameRate;

			mbps = NA;
			fs = NA;
			if (operationPoint.m_frameRate ==  1920)		// 7.5
				level = H264_Level_1_1;
			else if (operationPoint.m_frameRate ==  3840)	// 15
				level = H264_Level_1_2;
			else if(operationPoint.m_frameRate ==  7680)	// 30
				level = H264_Level_1_3;
			else
			{
				FTRACEINTO << "unknown frame rate";
			}
		}

	//	// For 240p we use the default valuses of CIF levels:
	//	if ((videoOperationPoint.m_frameHeight==240) && (videoOperationPoint.m_frameWidth==424))
	//	{
	//		FTRACEINTO << "240p";
	//		mbps = NA;
	//		fs = NA;
	//		if ((videoOperationPoint.m_frameRate ==  1920) || (videoOperationPoint.m_frameRate ==  3840))       // 7.5 or 15
	//			level = H264_Level_1_1;
	////		else if (videoOperationPoint.m_frameRate ==  3840)  //15
	////			level = H264_Level_1_2;
	//		else if(videoOperationPoint.m_frameRate ==  7680)   //30
	//			level = H264_Level_1_3;
	//		else
	//			FTRACEINTO << "unknown frame rate";
	//	}
	//	// For 720p we use the default values of HD levels:
	//	else if ((videoOperationPoint.m_frameHeight==720) && (videoOperationPoint.m_frameWidth==1280))
	//	{>
	//		FTRACEINTO << "720p";
	//		mbps = NA;
	//		fs = NA;
	//		if(videoOperationPoint.m_frameRate ==  7680)   //30
	//			level = H264_Level_3_1;
	//		else
	//			FTRACEINTO << "unknown frame rate";
	//	}
	//	// For 180p we use the default values of level 1:
	//	else if ((videoOperationPoint.m_frameHeight==180) && (videoOperationPoint.m_frameWidth==320))
	//	{
	//		FTRACEINTO << "180p";
	//		mbps = NA;
	//		fs = NA;
	//		if(videoOperationPoint.m_frameRate ==  3840) // 15
	//			level = H264_Level_1;
	//		else
	//			FTRACEINTO << "unknown frame rate";
	//	}
	} // end if (eIsUseOPP_No == isUseOperationPointesPresets)

	else // isUseOperationPointesPresets == eIsUseOPP_Yes...
	{
		switch (eOPPreset)
		{

			case eOPP_mobile:
				level = H264_Level_1;
				break;
			case eOPP_qvga:
				level = H264_Level_1_1;
				break;
			case eOPP_cif:
				level = H264_Level_1_2;
				break;
			case eOPP_vga:
				level = H264_Level_3;
				break;
			case eOPP_sd:
				level = H264_Level_3_1;
				break;
			case eOPP_hd:
				level = H264_Level_3_1;
				break;
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}
	}

	return level ? true : false;
}

//--------------------------------------------------------------------------
// same did of operation points indicate same resolution
bool IsSameResolutionLayerId(int requestedLayerId, int streamLayerId, const CVideoOperationPointsSet* pVideoOperationPointsSet)
{
	bool isSameResolution = false;
	if (NULL == pVideoOperationPointsSet)
	{
		FTRACEINTO << "IsSameResolutionLayerId - Failed to get VideoOperationPointsSet from video bridge";
		return false;
	}

	const std::list <VideoOperationPoint>* opList = pVideoOperationPointsSet->GetOperationPointsList();

	int requested_did = -1;
	int stream_did    = -1;
	for (std::list <VideoOperationPoint>::const_iterator  opItr = opList->begin(); opItr != opList->end(); ++opItr)
	{
		// LOG("requestedLayerId =  " << requestedLayerId << " streamLayerId = " << streamLayerId);
		if (requestedLayerId == opItr->m_layerId)
		{
			requested_did = opItr->m_did;
		}

		if (streamLayerId == opItr->m_layerId)
		{
			stream_did = opItr->m_did;
		}

		if (requested_did != -1 && stream_did != -1)
		{
			break;
		}
	}

	if (requested_did == stream_did && requested_did != -1)
	{
		isSameResolution = true;
	}

	return isSameResolution;
}

//--------------------------------------------------------------------------
int GetTidFromLayerId(int streamLayerId,const CVideoOperationPointsSet* pVideoOperationPointsSet)
{
    if(NULL == pVideoOperationPointsSet)
	{
	      FTRACEINTO << "failed to get VideoOperationPointsSet from video bridge";
	      return -1;
	}
    std::list <VideoOperationPoint>::const_iterator opItr;
    const std::list <VideoOperationPoint>* opList = pVideoOperationPointsSet->GetOperationPointsList();

     for(opItr = opList->begin();opItr!=opList->end();++opItr)
 	   if(streamLayerId == opItr->m_layerId)
		   return opItr->m_tid;

     return -1;
}
//--------------------------------------------------------------------------
bool GetBestSsrcAndLayerIdFromImage(int requestedLayerId,const CImage* pImage, unsigned int& resultSsrc, int &resultLayerId,const CVideoOperationPointsSet* pVideoOperationPointsSet, bool bAllowOnlyTemporal_T0)
{
	bool succeeded = true;

	int bestSsrcSharp = -1, bestSsrcMotion = -1;
	int bestLayerIdSharp = -1, bestLayerIdMotion = -1;
	int bestFrameRateMotion = -1, bestFrameRateSharpness = -1;
	unsigned long bestMBPSMotion = 0, bestMBPSSharpness = 0;
	bool foundRequestedLayerId = false;
	std::ostringstream omsg;

	if(pImage && pImage->IsVideoRelayImage())
	{
		std::list <CVideoRelayInMediaStream*> pVideoMediaStreamsList = pImage->GetVideoRelayMediaStreamsList();
		std::list <CVideoRelayInMediaStream*>::iterator itVideoMediaStream = pVideoMediaStreamsList.begin();

		// 1) look for same layer id
		for(; itVideoMediaStream!=pVideoMediaStreamsList.end(); ++itVideoMediaStream)
		{
			int imageLayerID = (*itVideoMediaStream)->GetLayerId();

			// check if only T0 is allowed, and if so only layers 0,3,6 are allowed
			if (bAllowOnlyTemporal_T0)
			{
				int tid = GetTidFromLayerId( imageLayerID, pVideoOperationPointsSet);
				if ((-1 == tid) || (tid != tid_0))
					continue;
			}

			// if the stream has the requested layer - finished
			if(imageLayerID == requestedLayerId && !pImage->IsVideoStreamMuted((*itVideoMediaStream)->GetSsrc()))
			{
				bestSsrcSharp = (*itVideoMediaStream)->GetSsrc();
				bestLayerIdSharp = imageLayerID;
				foundRequestedLayerId = true;
				break;
			}
		}

		// not found same layer id
		if(!foundRequestedLayerId)
		{
			if(NULL == pVideoOperationPointsSet)
			{
				omsg << ", failed to get VideoOperationPointsSet from video bridge";
				succeeded = false;
			}

			// checking if the requested is suitable for this request
			bool bIsRequestedLayerOkForRequest = true;
			if (bAllowOnlyTemporal_T0)
			{
				int tid = GetTidFromLayerId( requestedLayerId, pVideoOperationPointsSet);
				if (tid != tid_0)
					bIsRequestedLayerOkForRequest = false; // the requested Layer is not suitable but we may find another one that is OK
			}

			VideoOperationPoint rFoundVideoOperationPoint;
			for(itVideoMediaStream = pVideoMediaStreamsList.begin(); succeeded && itVideoMediaStream!=pVideoMediaStreamsList.end(); ++itVideoMediaStream)
			{

                if( pImage->IsVideoStreamMuted((*itVideoMediaStream)->GetSsrc()) )
                    continue;

                int imageLayerID = (*itVideoMediaStream)->GetLayerId();

				// check if only T0 is allowed, and if so only layers 0,3,6 are allowed
				bool bIsImageLayerOkForRequest = true;
				if (bAllowOnlyTemporal_T0)
				{
					int tid = GetTidFromLayerId( imageLayerID, pVideoOperationPointsSet);
					if (-1 == tid)
						continue; // ERROR...
					if (tid != tid_0)
						bIsImageLayerOkForRequest = false;	// must be TO but it is not, but we may find another one suitable with the same resolution
				}

				// 2) look for layer id from the same resolution in this case we take the minimum between them
				bool isSameResolution = IsSameResolutionLayerId(requestedLayerId, imageLayerID, pVideoOperationPointsSet);
				if( isSameResolution )
				{
					if(((imageLayerID < requestedLayerId) && bIsImageLayerOkForRequest) || ((requestedLayerId < imageLayerID) && bIsRequestedLayerOkForRequest))
					{
						bestLayerIdSharp = requestedLayerId;
						if(imageLayerID < requestedLayerId)
							bestLayerIdSharp = imageLayerID;

						bestSsrcSharp = (*itVideoMediaStream)->GetSsrc();

						pVideoOperationPointsSet->GetOperationPointFromList( bestLayerIdSharp, rFoundVideoOperationPoint );
						bestFrameRateSharpness = rFoundVideoOperationPoint.m_frameRate;
						bestMBPSSharpness = CalcOperationPointMBPS( rFoundVideoOperationPoint );
						break;
					}
				}

				// 3) look for layer id from the lower resolution (exception: found same resolution and allow-only-T0: need to keep search in this resolution for lower layers)
				if (!isSameResolution || !bAllowOnlyTemporal_T0)
				{
					if(imageLayerID > requestedLayerId)
					{
						// higher resolution - continue
						continue;
					}
				}

				bool isContain = false;
				// IsOperationPointContainedInOtherOperationPoint => is second layer id contains the first
				int containtedlayerId =  -1;
				if (bIsImageLayerOkForRequest)	// as we check if imageLayerID is contained, we need to do it only if imageLayerID is OK to be the selected.
					isContain = IsOperationPointContainedInOtherOperationPoint( imageLayerID, requestedLayerId,	pVideoOperationPointsSet);
				if(isContain)
				{
					containtedlayerId = imageLayerID;
				}
				else//maybe in a same stream with lower temporal layer is contained in the requested layer id
				{
					int layerId = imageLayerID;
					bool isResolutionSame = true;
					for(int i=layerId-1; i>=0; i--)
					{
						if (bAllowOnlyTemporal_T0 && (GetTidFromLayerId( i, pVideoOperationPointsSet) != tid_0))
								continue;

						isResolutionSame = IsSameResolutionLayerId(i,layerId, pVideoOperationPointsSet);
						if (!isResolutionSame)
							break;

						isContain = IsOperationPointContainedInOtherOperationPoint(i, requestedLayerId, pVideoOperationPointsSet);
						if(isContain)
						{
							containtedlayerId = i;
							break;
						}
					}

/*					for(int i=layerId-1; i>=0 && isResolutionSame; i--)
					{
						isResolutionSame = IsSameResolutionLayerId(i,layerId, pVideoOperationPointsSet);

						isContain = IsOperationPointContainedInOtherOperationPoint(i, requestedLayerId, pVideoOperationPointsSet);

						if(isResolutionSame && isContain)
						{
							containtedlayerId = i;
							break;
						}
					}
*/
				}


				if( containtedlayerId !=-1 )
				{
					pVideoOperationPointsSet->GetOperationPointFromList( containtedlayerId, rFoundVideoOperationPoint);

					// look for the maximum layer that contains in the requested layer
					if( bestLayerIdSharp == -1 || containtedlayerId > bestLayerIdSharp )
					{
						bestSsrcSharp = (*itVideoMediaStream)->GetSsrc();
						bestLayerIdSharp = containtedlayerId;
						bestFrameRateSharpness = rFoundVideoOperationPoint.m_frameRate;
						bestMBPSSharpness = CalcOperationPointMBPS( rFoundVideoOperationPoint );
					}
					// look for the motion before sharpness
					if( bestLayerIdMotion == -1 || (containtedlayerId > bestLayerIdMotion &&
													(int (rFoundVideoOperationPoint.m_frameRate)) >= bestFrameRateMotion) )
					{
						bestSsrcMotion = (*itVideoMediaStream)->GetSsrc();
						bestLayerIdMotion = containtedlayerId;
						bestFrameRateMotion = rFoundVideoOperationPoint.m_frameRate;
						bestMBPSMotion = CalcOperationPointMBPS( rFoundVideoOperationPoint );
					}
				}
			}
			omsg << "\n\tbest motion choice:  layer = " << bestLayerIdMotion << ", frameRate = " << bestFrameRateMotion << ", mbps = " << bestMBPSMotion
				 << "\n\tbest sharpness choice:  layer = " << bestLayerIdSharp << ",  frameRate = " << bestFrameRateSharpness << ", mbps = " << bestMBPSSharpness;

		}
		if(bestLayerIdSharp == -1 || bestSsrcSharp == -1)
		{
			omsg << "\nNo streams / ssrc in image";
			succeeded = false;
		}
	}
	else
	{
		omsg << "\nImage is null or nor relay ";
		succeeded = false;
	}

	if( !foundRequestedLayerId && succeeded ) // only if we need to supply layer id which is not equal to the requested
	{                                         // then we'll choose the layer id to the sharpness mode flag
		eVideoResPriority prior = eVideoResPriorSharp;//m_pVideoBridge->GetVideoResPriority();
		switch(prior)
		{
			case eVideoResPriorAuto:
				if( bestMBPSSharpness >= bestMBPSMotion )
				{
					resultSsrc = bestSsrcSharp;
					resultLayerId = bestLayerIdSharp;
				}
				else
				{
					resultSsrc = bestSsrcMotion;
					resultLayerId = bestLayerIdMotion;
				}
				omsg << "\n\tCurrent Conf Video Resolution Priority is \"Auto\" ==> best layer id=" << resultLayerId;
				break;
			case eVideoResPriorSharp:
				resultSsrc = bestSsrcSharp;
				resultLayerId = bestLayerIdSharp;
				omsg << "\n\tCurrent Conf Video Resolution Priority is \"Sharpness vs. Motion\" ==> best layer id=" << resultLayerId;
				break;
			case eVideoResPriorMotion:
				resultSsrc = bestSsrcMotion;
				resultLayerId = bestLayerIdMotion;
				omsg << "\n\tCurrent Conf Video Resolution Priority is \"Motion vs. Sharpness\" ==> best layer id=" << resultLayerId;
				break;
		}
	}
	else
	{
		resultSsrc = bestSsrcSharp;
		resultLayerId = bestLayerIdSharp;
		omsg << "\n Found, Result SSRC: " << (DWORD)resultSsrc << ", Result Layer Id: " << (DWORD)resultLayerId;
	}

	FTRACESTRFUNC(eLevelDebug) << omsg.str().c_str();
	//FTRACEINTO << omsg.str().c_str();
	return succeeded;
}


////////////////////////////////////////////////////////////////////////////
void FillVideoOperationSetParams(VIDEO_OPERATION_POINT_SET_S& videoOperationPointSetStruct, CVideoOperationPointsSet* videoOperationPointsSet)
{
    memset(&videoOperationPointSetStruct, 0, sizeof(VIDEO_OPERATION_POINT_SET_S));

    if (!videoOperationPointsSet)
        return;

    videoOperationPointSetStruct.operationPointSetId = videoOperationPointsSet->GetSetId();
    const std::list <VideoOperationPoint>* operationPointList = videoOperationPointsSet->GetOperationPointsList();
    int numOfOperationPoints = operationPointList->size();
    if(numOfOperationPoints>MAX_NUM_OPERATION_POINTS_PER_SET)
    {
        FPASSERT(numOfOperationPoints);
        numOfOperationPoints = MAX_NUM_OPERATION_POINTS_PER_SET;
    }
    videoOperationPointSetStruct.numberOfOperationPoints = numOfOperationPoints;
    std::list <VideoOperationPoint>::const_iterator itr = operationPointList->begin();
    for (int i = 0; itr != operationPointList->end() && i < MAX_NUM_OPERATION_POINTS_PER_SET; ++itr, i++ )
    {
        videoOperationPointSetStruct.tVideoOperationPoints[i].layerId = (*itr).m_layerId;
        videoOperationPointSetStruct.tVideoOperationPoints[i].Tid = (*itr).m_tid;
        videoOperationPointSetStruct.tVideoOperationPoints[i].Did = (*itr).m_did;
        videoOperationPointSetStruct.tVideoOperationPoints[i].Qid = (*itr).m_qid;
        videoOperationPointSetStruct.tVideoOperationPoints[i].Pid = 0;//Currently not in use keren to check
        videoOperationPointSetStruct.tVideoOperationPoints[i].profile = (*itr).m_videoProfile;
        videoOperationPointSetStruct.tVideoOperationPoints[i].level = 0;//currently not needed in MRMP
        videoOperationPointSetStruct.tVideoOperationPoints[i].frameWidth = (*itr).m_frameWidth;
        videoOperationPointSetStruct.tVideoOperationPoints[i].frameHeight = (*itr).m_frameHeight;
        videoOperationPointSetStruct.tVideoOperationPoints[i].frameRate = (*itr).m_frameRate;
        videoOperationPointSetStruct.tVideoOperationPoints[i].maxBitRate = (*itr).m_maxBitRate;

    }

}


//--------------------------------------------------------------------------
BOOL IsRtvBframeEnabled(BOOL isAvmcu)
{
	std::string encodeRtvBFrameStr;
	BOOL res = CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey(CFG_ENCODE_RTV_B_FRAME, encodeRtvBFrameStr);
	FPASSERTSTREAM_AND_RETURN_VALUE(!res, "CSysConfig::GetDataByKey: " << CFG_ENCODE_RTV_B_FRAME, FALSE);

	bool isAlways    =  ("ALWAYS" == encodeRtvBFrameStr);
	bool isAvmcuOnly =  ("AVMCU_ONLY" == encodeRtvBFrameStr);

	if (isAlways ||
	    (isAvmcu && isAvmcuOnly))
	{
		FPTRACE(eLevelInfoNormal, "IsRtvBframeEnabled - RTV BFrame is enabled");
		return TRUE;
	}

// FPTRACE(eLevelInfoNormal, "IsRtvBframeEnabled - RTV BFrame is NOT enabled.");

	return FALSE;
}

//--------------------------------------------------------------------------
eVideoPartyType GetVideoPartyTypeAllocationForRtvBframe(eVideoPartyType videoPartyType)
{
	if (videoPartyType < eCP_H261_H263_upto_CIF_video_party_type)
		videoPartyType = eCP_H264_upto_SD30_video_party_type;
	else if (videoPartyType < eCP_H264_upto_HD720_30FS_Symmetric_video_party_type)
		videoPartyType = eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
	else
		videoPartyType = eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;

	FTRACEINTO << "GetVideoPartyTypeAllocationForRtvBframe - videoPartyType:" << eVideoPartyTypeNames[videoPartyType];
	return videoPartyType;
}

//--------------------------------------------------------------------------
void RateToTmmbrParams(uint32_t rate, uint32_t* mantissa, uint32_t* exp)
{
	uint32_t exponential  = 0,
	         bitRemainder = 0;


	/** The mantissa is 17 bits in size, after shifting 17 bits, we divide the remainder by 2;
	 *  when we reach 0, we find exponential */
	bitRemainder = rate >> 17;

	if (bitRemainder)
	{
		while (bitRemainder)
		{
			bitRemainder = bitRemainder >> 1;
			exponential++;
		}

		// for(exponential; bitRemainder > 0; exponential++)
	}

	/* verification exponential is within limits (6 bits)*/
	if (exponential >= 64)
		FPTRACE2INT(eLevelInfoNormal, "RateToTmmbrParams: exponential over the limit max exp=64 calculated exp=", exponential);

	*mantissa = rate >> exponential;
	*exp      = exponential;
}

//--------------------------------------------------------------------------
void TmmbrParamsToRate(uint32_t mantissa, uint32_t exp, uint32_t* rate)
{
	/* in which units do we want to rate? 1k or bytes? */
	*rate = mantissa << exp;
}

//--------------------------------------------------------------------------
BOOL IsFeatureSupportedBySystem(const eFeatureName featureName)
{
	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	FPASSERT_AND_RETURN_VALUE(!pConfPartyProcess, FALSE);

	return pConfPartyProcess->IsFeatureSupportedByHardware(featureName);
}

//--------------------------------------------------------------------------
CPartyImageLookupTable* GetPartyImageLookupTable()
{
	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	FPASSERT_AND_RETURN_VALUE(!pConfPartyProcess, FALSE);

	return pConfPartyProcess->GetPartyImageLookupTable();
}

//--------------------------------------------------------------------------
CLookupIdParty* GetLookupIdParty()
{
	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();

	return pConfPartyProcess->GetLookupIdParty();
}

//--------------------------------------------------------------------------
CLookupTableParty* GetLookupTableParty()
{
	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	FPASSERT_AND_RETURN_VALUE(!pConfPartyProcess, FALSE);

	return pConfPartyProcess->GetLookupTableParty();
}

//--------------------------------------------------------------------------
// this function remove leaked party id's from CLookupIdParty
// it should be called only when LookupIdParty full
// LookupIdParty can't be full unless there is a leak
// - it initiated to much more party id's then required
WORD CleanLookupIdTablefromLookupPartyTable()
{
	CLookupIdParty*    pLookupIdParty    = GetLookupIdParty();
	CLookupTableParty* pLookupTableParty = GetLookupTableParty();
	FPASSERT_AND_RETURN_VALUE((!pLookupTableParty || !pLookupIdParty), 0);

	std::ostringstream msg;
	WORD clean_count = 0;
	size_t size = pLookupIdParty->size();
	for (PartyRsrcID id = 1; id < (PartyRsrcID)size; ++id)
	{
		if (!pLookupTableParty->Get(id))
		{
			// removing party id that exist in LookupId and not in LookupTableParty
			pLookupIdParty->Clear(id);
			msg << id << " ";
			++clean_count;
			if ((clean_count % 50) == 0)
			msg << "\n";
		}
	}
	FTRACESTR(eLevelInfoHigh) << "Removed " << clean_count << " party id's: \n" << msg.str().c_str();
	return clean_count;
}

//--------------------------------------------------------------------------
BOOL IsMsFECEnabled()
{
	std::string MsFecStr;
	CProcessBase::GetProcess()->GetSysConfig()->GetDataByKey(CFG_KEY_ENABLE_MS_FEC, MsFecStr);

	if (MsFecStr.size() > 0)
	{
		if ((strcmp("AUTO", MsFecStr.c_str()) == 0) ||
		    (strcmp("DV00", MsFecStr.c_str()) == 0) ||
		    (strcmp("DV01", MsFecStr.c_str()) == 0))
			return TRUE;
	}

	FPTRACE(eLevelInfoNormal, "IsMsFECEnabled - MS FEC is disabled.");
	return FALSE;
}

//--------------------------------------------------------------------------
BOOL IsLyncRTCPIntraEnabled()
{
	BOOL IsRTCPIntraEnabled = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_LYNC_RTCP_INTRA, IsRTCPIntraEnabled);
	return IsRTCPIntraEnabled;
}
//--------------------------------------------------------------------------
BOOL IsLyncRTCPIntraForAVMCUEnabled()
{
	BOOL IsRTCPIntraEnabled = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_LYNC_RTCP_INTRA_AVMCU, IsRTCPIntraEnabled);
	return IsRTCPIntraEnabled;
}
//--------------------------------------------------------------------------
BOOL IsSendPreferenceRequestToAVMCU2010()
{
	BOOL IsSendPreferenceReq = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SEND_PREFERENCE_REQUEST_TO_AVMCU2010, IsSendPreferenceReq);
	return IsSendPreferenceReq;
}
//--------------------------------------------------------------------------
WORD GetMaxConfTemplates()
{
	WORD numOfConfTemplates = MAX_CONF_TEMPLATES_RMX;


	FTRACEINTO << "GetMaxConfTemplates - numOfConfTemplates:" << numOfConfTemplates;
	return numOfConfTemplates;
}

//--------------------------------------------------------------------------
// 1080p60debug - to be removed
void Set1080p60mbps(DWORD mbps)
{
	FPTRACE2INT(eLevelInfoNormal, "Set1080p60mbps - s1080p60mbps:", s1080p60mbps);
	s1080p60mbps = mbps;
}

//--------------------------------------------------------------------------
// 1080p60debug - to be removed
DWORD Get1080p60mbps()
{
	FPTRACE2INT(eLevelInfoNormal, "get1080p60mbps - s1080p60mbps:", s1080p60mbps);
	return s1080p60mbps;
}

//--------------------------------------------------------------------------

BOOL IsRelaySupported(eConfMediaType confMediaType)
{
	if(eSvcOnly == confMediaType || eMixAvcSvc == confMediaType || eMixAvcSvcVsw == confMediaType)
		return TRUE;
	else
		return FALSE;
}
//------------------------------------------------------------------------
BOOL IsSoftMcu()
{
	return (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily()) ? TRUE : FALSE;
//	eProductType prodType = CProcessBase::GetProcess()->GetProductType();
//	if(prodType == eProductTypeSoftMCU || prodType == eProductTypeSoftMCUMfw
//		|| prodType == eProductTypeGesher || prodType == eProductTypeNinja)
//	{
//		return TRUE;
//	}
//	return FALSE;
}
//------------------------------------------------------------------------
BOOL IsIvrForSVCEnabled()
{
	BOOL IsIvrForSVCFlag = TRUE;
	// EE-462 VEQ support for AVC and SVC Calls
//	if(CProcessBase::GetProcess()->GetProductType() == eProductTypeSoftMCUMfw)
//	{
//		FTRACEINTO<< "SVC IVR not supported for MFW, return FALSE ";
//		IsIvrForSVCFlag = FALSE;
//	}
	return IsIvrForSVCFlag;
}

BOOL IsOmitDomainFromPartyName()
{
	BOOL bIsOmit = FALSE;

	std::string strKey;
	CSysConfig* sysConfig = NULL;
	CProcessBase *pProcess = CProcessBase::GetProcess();
	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();
	if(sysConfig)
		sysConfig->GetBOOLDataByKey(CFG_KEY_SIP_OMIT_DOMAIN_FROM_PARTY_NAME , bIsOmit);

	return bIsOmit;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetSdesCapEnumFromSystemFlag()
{

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	string ShaKeyType;
	sysConfig->GetDataByKey(CFG_KEY_SRTP_SRTCP_HMAC_SHA_LENGTH, ShaKeyType);
	int Sh1Length;
	CStringsMaps::GetValue(SRTP_SRTCP_HMAC_SHA_LENGTH_ENUM, Sh1Length, (char*)ShaKeyType.c_str());

	return Sh1Length;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
static DWORD dwDebugValue = 0;
static std::string stDebugFeatureStr="";
///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsDebugModeStr( const char *featureStr, DWORD dwDebugToTest )
{
	if (dwDebugToTest == dwDebugValue)
	{
		if (0 == stDebugFeatureStr.compare(featureStr))
		{
			FTRACEINTO << "IsDebugModeStr: Feature = " << featureStr << " , dwDebug = " << dwDebugToTest;
			FPTRACE( eLevelError, " Error - Debug Parameter Match !! " );
			return true;			// match the number
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetDebugValue( const std::string& featureNameStr, DWORD debugValue)
{
	stDebugFeatureStr = featureNameStr;
	dwDebugValue = debugValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void SerializeNonMandatoryRsrcParams(CSegment* pParam, CRsrcParams* &pRsrcParams, const char *aName)
{
	BYTE bRsrcExists = FALSE;
	if (pRsrcParams)
	{
		bRsrcExists = TRUE;
		*pParam << bRsrcExists;
		pRsrcParams->Serialize(NATIVE, *pParam);
	}
	else
	{
		*pParam << bRsrcExists;
		FTRACEINTO << "Resource " << aName << " doesn't exist.";
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void SerializeNonMandatoryRsrcParamsArray(CSegment* pParam, CRsrcParams** pRsrcParamsArray, int aArrayLen, const char *aName)
{
	BYTE bRsrcExists = FALSE;
	if (pRsrcParamsArray)
	{
		for(int i = 0; i < aArrayLen; ++i)
		{
			SerializeNonMandatoryRsrcParams(pParam, pRsrcParamsArray[i], aName);
		}
	}
	else
	{
		CRsrcParams* pRsrcParams = NULL;
		for(int i = 0; i < aArrayLen; ++i)
		{
			SerializeNonMandatoryRsrcParams(pParam, pRsrcParams, aName);
		}
	}
}

void DeSerializeNonMandatoryRsrcParams(CSegment* pParam, CRsrcParams* &apRsrcParams, const char *aName)
{
	apRsrcParams = NULL;

	BYTE bRsrcExists = FALSE;
	*pParam >> bRsrcExists;
	if (bRsrcExists)
	{
		apRsrcParams = new CRsrcParams;
		apRsrcParams->DeSerialize(NATIVE, *pParam);
	}
	else
	{
		FTRACEINTO << "Resource " << aName << " doesn't exist.";
	}
}

DWORD GetCurrentLoggerNumber()
{
	return stdwCurrentLoggerNumber;
}
void SetCurrentLoggerNumber(DWORD loggerNumber)
{
	stdwCurrentLoggerNumber = loggerNumber;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
size_t ReadXmlFile(char* xmlFile, char** xmlBuffer)
{
	const size_t lFileSize = GetFileSize(xmlFile);

	std::ifstream ifs;
	ifs.open(xmlFile, std::ios::in);

	if (ifs)
	{
		*xmlBuffer = new char[lFileSize+1];
		memset(*xmlBuffer, 0, lFileSize+1);

		ifs.read(*xmlBuffer, lFileSize);
		ifs.close();
		return lFileSize;
	}
	return 0;
}
BOOL isMsftSvc2013Supported()
{
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL IsMsft2013Blocked = FALSE;

	sysConfig->GetBOOLDataByKey(CFG_KEY_BLOCK_NEW_LYNC2013_FUNCTIONALITY, IsMsft2013Blocked);

	return (IsFeatureSupportedBySystem(eFeatureMs2013SVC) && !IsMsft2013Blocked);
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
const char* ConfTypeToString(EConfType confType)
{
	switch (confType)
	{
		case kVideoSwitch               : return "kVideoSwitch";
		case kVSW_Fixed                 : return "kVSW_Fixed";
		case kSoftCp                    : return "kSoftCp";
		case kCp                        : return "kCp";
		case kCpQuad                    : return "kCpQuad";
		case kCop                       : return "kCop";
		default:
			FTRACEINTO << "unknown EConfType " << confType;
	}

	return "Invalid_ConfType";
}

//--------------------------------------------------------------------------
const char* VideoPartyTypeToString(eVideoPartyType videoPartyType)
{
	if (videoPartyType <= NUM_OF_VIDEO_PARTY_TYPES)
	{
		return eVideoPartyTypeNames[videoPartyType];
	}
	else
	{
		return "Invalid_VideoPartyType";
	}
}

//--------------------------------------------------------------------------
const char* RemoteIdentToString(RemoteIdent remoteIdent)
{
	switch (remoteIdent)
	{
		case Regular:                     return "Regular";
		case PolycomMGC:                  return "PolycomMGC";
		case PolycomRMX:                  return "PolycomRMX";
		case PolycomNPG:                  return "PolycomNPG";
		case RvMCU:                       return "RvMCU";
		case RvGWOrProxy:                 return "RvGWOrProxy";
		case RvEp:                        return "RvEp";
		case RVTestapplication:           return "RVTestapplication";
		case VconEp:                      return "VconEp";
		case TandbergEp:                  return "TandbergEp";
		case VIU:                         return "VIU";
		case NetMeeting:                  return "NetMeeting";
		case EricssonVIG:                 return "EricssonVIG";
		case EricssonVigSip:              return "EricssonVigSip";
		case SonyEp:                      return "SonyEp";
		case DstH323Mcs:                  return "DstH323Mcs";
		case PolycomEp:                   return "PolycomEp";
		case MicrosoftEP_R1:              return "MicrosoftEP_R1";
		case MicrosoftEP_R2:              return "MicrosoftEP_R2";
		case MicrosoftEP_Lync_R1:         return "MicrosoftEP_Lync_R1";
		case MicrosoftEP_Lync_2013:       return "MicrosoftEP_Lync_2013";
		case MicrosoftEP_MAC:             return "MicrosoftEP_MAC";
		case MicrosoftEP_MAC_Lync:        return "MicrosoftEP_MAC_Lync";
		case AvistarGW:                   return "AvistarGW";
		case CiscoGW:                     return "CiscoGW";
		case LifeSizeEp:                  return "LifeSizeEp";
		case IbmSametimeEp:               return "IbmSametimeEp";
		case PolycomQDX:                  return "PolycomQDX";
		case PolycomVVX:                  return "PolycomVVX";
		case AvayaEP:                     return "AvayaEP";
		case CiscoCucm:                   return "CiscoCucm";
		case MicrosoftMediationServer:    return "MicrosoftMediationServer";
		case Microsoft_AV_MCU:            return "Microsoft_AV_MCU";
		case Microsoft_AV_MCU2013:        return "Microsoft_AV_MCU2013";
		case MicrosoftEP_Lync_CCS:        return "MicrosoftEP_Lync_CCS";
		case Polycom_Lync_CCS_Gw:        return "Polycom CSS Gateway";

		default:
			FTRACEINTO << "unknown RemoteIdent " << remoteIdent;
			break;
	}

	return "Unknown_RemoteIdent";
};

//--------------------------------------------------------------------------
const char* MediaDirectionToString(EMediaDirection mediaDirection)
{
	switch (mediaDirection)
	{
		case eNoDirection:              return "eNoDirection";
		case eMediaIn:                  return "eMediaIn";
		case eMediaOut:                 return "eMediaOut";
		case eMediaInAndOut:            return "eMediaInAndOut";

		default:
			FTRACEINTO << "unknown EMediaDirection " << mediaDirection;
			break;
	}

	return "Unknown_EMediaDirection";
};

//--------------------------------------------------------------------------
const char* AppPartyStateToString(TAppPartyState appPartyState)
{
	switch (appPartyState)
	{
		case eAPP_PARTY_STATE_MIN:                   return "eAPP_PARTY_STATE_MIN";
		case eAPP_PARTY_STATE_IDLE:                  return "eAPP_PARTY_STATE_IDLE";
		case eAPP_PARTY_STATE_IVR_ENTRY:             return "eAPP_PARTY_STATE_IVR_ENTRY";
		case eAPP_PARTY_STATE_IVR_FEATURE:           return "eAPP_PARTY_STATE_IVR_FEATURE";
		case eAPP_PARTY_STATE_CONNECTING_TO_MIX:     return "eAPP_PARTY_STATE_CONNECTING_TO_MIX";
		case eAPP_PARTY_STATE_MIX:                   return "eAPP_PARTY_STATE_MIX";
		case eAPP_PARTY_STATE_DISCONNECTED:          return "eAPP_PARTY_STATE_DISCONNECTED";
		case eAPP_PARTY_STATE_MOVING:                return "eAPP_PARTY_STATE_MOVING";
		case eAPP_PARTY_STATE_DELETED:               return "eAPP_PARTY_STATE_DELETED";
		case eAPP_PARTY_STATE_MAX:                   return "eAPP_PARTY_STATE_MAX";

		default:
			FTRACEINTO << "unknown TAppPartyState " << appPartyState;
			break;
	}

	return "Unknown_TAppPartyState";
};

//--------------------------------------------------------------------------
const char* IpChannelTypeToString(EIpChannelType ipChannelType)
{
	switch (ipChannelType)
	{
		case H225:                     return "H225_or_SIGNALING";
		case H245:                     return "H245_or_SDP";
		case AUDIO_IN:                 return "AUDIO_IN";
		case AUDIO_OUT:                return "AUDIO_OUT";
		case VIDEO_IN:                 return "VIDEO_IN";
		case VIDEO_OUT:                return "VIDEO_OUT";
		case AUDIO_CONT_IN:            return "AUDIO_CONT_IN";
		case AUDIO_CONT_OUT:           return "AUDIO_CONT_OUT";
		case VIDEO_CONT_IN:            return "VIDEO_CONT_IN";
		case VIDEO_CONT_OUT:           return "VIDEO_CONT_OUT";
		case FECC_IN:                  return "FECC_IN";
		case FECC_OUT:                 return "FECC_OUT";
		case BFCP_IN:                  return "BFCP_IN";
		case BFCP_OUT:                 return "BFCP_OUT";
		case BFCP:                     return "BFCP";
		case BFCP_UDP:                 return "BFCP_UDP";
		case IP_CHANNEL_TYPES_NUMBER:  return "IP_CHANNEL_TYPES_NUMBER";

		default:
			FTRACEINTO << "unknown EIpChannelType " << ipChannelType;
			break;
	}

	return "Unknown_EIpChannelType";
};

//--------------------------------------------------------------------------
const char* CsChannelStateToString(ECsChannelState csChannelState)
{
	switch (csChannelState)
	{
		case kDisconnectedState:         return "kDisconnectedState";
		case kFirstConnectingState:      return "kFirstConnectingState";
		case kConnectingState:           return "kConnectingState";
		case kBeforeResponseReq:         return "kBeforeResponseReq";
		case kBeforeConnectedInd:        return "kBeforeConnectedInd";
		case kLastConnectingState:       return "kLastConnectingState";
		case kConnectedState:            return "kConnectedState";
		case kFirstDisconnectingState:   return "kFirstDisconnectingState";
		case kDisconnectingState:        return "kDisconnectingState";
		case kWaitToSendChannelDrop:     return "kWaitToSendChannelDrop";
		case kCheckSendCallDrop:         return "kCheckSendCallDrop";
		case kNoNeedToDisconnect:        return "kNoNeedToDisconnect";
		case kLastDisconnectingState:    return "kLastDisconnectingState";

		default:
			FTRACEINTO << "unknown ECsChannelState " << csChannelState;
			break;
	}

	return "Unknown_ECsChannelState";
};

//--------------------------------------------------------------------------
const char* ArtRtpPortChannelStateToString(EArtRtpPortChannelState artRtpPortChannelState)
{
	switch (artRtpPortChannelState)
	{
		case kRtpPortNotSendOpenYet:       return "kRtpPortNotSendOpenYet";
		case kRtpPortOpenSent:             return "kRtpPortOpenSent";
		case kRtpPortUpdateSent:           return "kRtpPortUpdateSent";
		case kRtpPortReceivedOpenAck:      return "kRtpPortReceivedOpenAck";
		case kRtpPortReceivedUpdateAck:    return "kRtpPortReceivedUpdateAck";

		default:
			FTRACEINTO << "unknown EArtRtpPortChannelState " << artRtpPortChannelState;
			break;
	}

	return "Unknown_EArtRtpPortChannelState";
};

//--------------------------------------------------------------------------
const char* CmUdpChannelStateToString(ECmUdpChannelState cmUdpChannelState)
{
	switch (cmUdpChannelState)
	{
		case kNotSendOpenYet:       return "kNotSendOpenYet";
		case kSendOpen:             return "kSendOpen";
		case kRecieveOpenAck:       return "kRecieveOpenAck";
		case kNeedsToBeClosed:      return "kNeedsToBeClosed";
		case kSendClose:            return "kSendClose";
		case kRecieveCloseAck:      return "kRecieveCloseAck";

		default:
			FTRACEINTO << "unknown ECmUdpChannelState " << cmUdpChannelState;
			break;
	}

	return "Unknown_ECmUdpChannelState";
};

EVideoResolutionType convertVideoTypeToResType(eVideoPartyType videoPartyType)
{
	switch (videoPartyType)
	{
	case eCP_H264_upto_CIF_video_party_type:
		return eCIF_Res;
	case eCP_H261_H263_upto_CIF_video_party_type:
	case eCP_H264_upto_SD30_video_party_type:
	case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
		return eSD_Res;
	case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
    case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
    case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:
    case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
		return eHD720_Res;
	default:
		return eAuto_Res;
	}
};

//--------------------------------------------------------------------------
DWORD GetPrIdByResolutionType(EVideoResolutionType resolutionType)
{
	switch(resolutionType)
	{
		case eCIF_Res:
			return 0;

		case eSD_Res:
			return 10;

		case eHD720_Res:
			return 20;

		default:
			return 0;
	}
};

//--------------------------------------------------------------------------
EVideoResolutionType GetResolutionTypeByPrId(DWORD prID)
{
	EVideoResolutionType resType = eAuto_Res;

	if (prID < 10)
		resType = eCIF_Res;

	else if (prID < 20)
		resType = eSD_Res;

	else if (prID < 30)
		resType = eHD720_Res;

	return resType;
};

//--------------------------------------------------------------------------
BYTE GetResetSdesForVendorFlag()
{
	BYTE resetSdesForVendorFlag = 0;

	std::string strKey;
	CSysConfig* sysConfig = NULL;
	CProcessBase *pProcess = CProcessBase::GetProcess();
	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();
	if(sysConfig)
	{
		sysConfig->GetDataByKey("RESET_SDES_FOR_VENDOR", strKey);

		FTRACEINTO << "flag:" << strKey;

		std::size_t found = strKey.find("ALL");
		if (found != std::string::npos)
		{
			resetSdesForVendorFlag = MASK_RESET_SDES_FOR_ALL;
			return resetSdesForVendorFlag;
		}

		found = strKey.find("Cisco");
		if (found != std::string::npos)
			resetSdesForVendorFlag |= MASK_RESET_SDES_FOR_CISCO;
		found = strKey.find("Microsoft");
		if (found != std::string::npos)
			resetSdesForVendorFlag |= MASK_RESET_SDES_FOR_MICROSOFT;
		found = strKey.find("Polycom");
		if (found != std::string::npos)
			resetSdesForVendorFlag |= MASK_RESET_SDES_FOR_POLYCOM;
	}
	return resetSdesForVendorFlag;
}

//////////////////////////////////////////////////
//eFeatureRssDialin
BOOL GetFlagEnableRssControlViaInfo()
{
	BOOL          bRetVal 	 = YES;
	CSysConfig*   sysConfig  = NULL;
	CProcessBase* pProcess   = CProcessBase::GetProcess();

	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();

	if(sysConfig)
		sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_RECORDING_CONTROL_VIA_SIPINFO, bRetVal);

	return bRetVal;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType translateVideoPartyTypeToH264VideoMode(Eh264VideoModeType h264videomode)
{
	switch(h264videomode)
	{
		case(eInvalidModeType):
		{
			return eCP_H264_upto_CIF_video_party_type;
			FPTRACE(eLevelError, "translateVideoPartyTypeToH264VideoMode - inavlid mode");
		}
		case(eCIF30):
				return eCP_H264_upto_CIF_video_party_type;
		case(eCIF60):
				return eCP_H264_upto_SD30_video_party_type;
		case(e2CIF30):
				return eCP_H264_upto_SD30_video_party_type;
		case(eWCIF60):
				return eCP_H264_upto_SD30_video_party_type;

		case(eSD15):
				return eCP_H264_upto_SD30_video_party_type;
		case(eSD30):
				return eCP_H264_upto_SD30_video_party_type;
		case(eW4CIF30):
					return eCP_H264_upto_SD30_video_party_type;
		case(eSD60):
				return eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
		case(eHD720Asymmetric):
				return eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
		case(eHD720Symmetric):
				return eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
		case(eHD720At60Asymmetric):
				return eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
		case(eHD720At60Symmetric):
				return eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
		case(eHD1080Asymmetric):
				return eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
		case(eHD1080Symmetric):
				return eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
		default:
			return eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;






	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Eh264VideoModeType translateToH264VideoModeVideoPartyType(eVideoPartyType  videoPartyType)
{
	switch(videoPartyType)
	{
	case eVideo_party_type_dummy:
	case eVideo_party_type_none:
	{
		FPTRACE(eLevelError, "translateToH264VideoModeVideoPartyTyp - invalid Video Party Type");
		return eInvalidModeType;
	}
	case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
	case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
	{
		return eHD720Symmetric;
	}
	case eCP_H264_upto_SD30_video_party_type:
	case eCP_H261_H263_upto_CIF_video_party_type:
	{
	     return eW4CIF30;
	}
	case eCP_H264_upto_CIF_video_party_type:
	{
		 return eCIF30;
	}
	case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
	case eCP_H261_CIF_equals_H264_HD1080_video_party_type:
	case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
	case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:
	case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
	{
		return eHD1080Symmetric;
	}
	default:
	{
		FPASSERT(videoPartyType);
		return eInvalidModeType;
	}
	}
}

//////////////////////////////////////////////////
extern void UpdateTrafficShapingParams(TUpdateRtpSpecificChannelParams& rtp, const BYTE isMrcCall)
{
	CSysConfig*   pSysConfig  = NULL;
	CProcessBase* pProcess   = CProcessBase::GetProcess();

	if(pProcess) pSysConfig =	pProcess->GetSysConfig();

	if(pSysConfig)
	{
		BOOL isTrafficShapingEnabled = NO;
		std::string key = CFG_KEY_ENABLE_RTP_TRAFFIC_SHAPING;
		pSysConfig->GetBOOLDataByKey(key, isTrafficShapingEnabled);
		FTRACEINTO << "isTrafficShapingEnabled = " << static_cast<int>(isTrafficShapingEnabled);
		if( isTrafficShapingEnabled && !isMrcCall )
		{
			std::string key = CFG_KEY_TRAFFIC_SHAPING_WINDOW_SIZE;
			pSysConfig->GetDWORDDataByKey(key, rtp.unTrafficShapingWindowSize);
			rtp.b32TrafficShapingEnabled = static_cast<APIU32>(isTrafficShapingEnabled);
		}
		else
		// for SVC party the params b32TrafficShapingEnabled and unTrafficShapingWindowSize should be 0
		{
			rtp.unTrafficShapingWindowSize = 0;
			rtp.b32TrafficShapingEnabled = 0;
		}
	}
}

//13003
DWORD GetRTVMaxBitRateForForceCIFParticipant()
{
	DWORD nRTVMaxBitRate = 192;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("RTV_MAX_BIT_RATE_FOR_FORCE_CIF_PARTICIPANT", nRTVMaxBitRate);
	return nRTVMaxBitRate;
}

int HtmlToUtf8(char** ppInString, char* pOutString, int bytecount)
{
	if (ppInString == NULL || pOutString == NULL || bytecount <= 0)
		return 0;

	char* pInString = *ppInString;
	int bytesleft = bytecount;
	unsigned int wChar = 0;
	char c = 0;

	while (bytesleft > 0)
	{
		c = *pInString++;
		if (c == 0x5c)
		{
			c = *pInString++;
			if (c == 'u')
			{
				sscanf((char*)pInString, "%4x", (unsigned int *)&wChar);
				if (wChar < 0x80)
				{
					// 1 byte UTF-8
					*pOutString++ = (char)(wChar & 0x7f);
					bytesleft -= 6;
				}
				if (wChar < 0x800)
				{
					// 2 byte UTF-8
					*pOutString++ = (char)(0xc0 + (wChar >> 6));
					*pOutString++ = (char)(0x80 + (wChar & 0x3f));
					bytesleft -= 6;
				}
				else
				{
					// 3 byte UTF-8
					*pOutString++ = (char)(0xE0 + (wChar >> 12));
					*pOutString++ = (char)(0x80 + ((wChar >> 6) & 0x3f));
					*pOutString++ = (char)(0x80 + (wChar & 0x3f));
					bytesleft -= 6;
				}
				pInString += 4;
			}
			else
			{
				*pOutString++ = c;
				bytesleft -= 2;
			}
		}
		else
		{
			*pOutString++ = c;
			bytesleft -= 1;
		}
	}
	*pOutString = '\0';

	return 1;
}

BOOL GetBOOLDataByKey(const std::string& key)
{
	BOOL bRetVal = FALSE;

	CSysConfig* sysConfig = NULL;
	CProcessBase *pProcess = CProcessBase::GetProcess();
	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();
	else
		FPASSERTMSG(!pProcess,"!pProcess");

	if(sysConfig)
		sysConfig->GetBOOLDataByKey(key, bRetVal);
	else
		FPASSERTMSG(!sysConfig,"!sysConfig");

	return bRetVal;
}

