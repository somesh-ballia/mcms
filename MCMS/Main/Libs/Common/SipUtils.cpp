  //+========================================================================+
//                            SIPUTILS.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SIPUTILS.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara Avidan                                                |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 7/4/03    | Utilities fot SIP parties		                       |
//+========================================================================+

#include "SipUtils.h"
#include "Macros.h"
#include "DataTypes.h"
#include "ObjString.h"
#include "SharedDefines.h"
#include "SystemFunctions.h"
#include "ConfPartyApiDefines.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ConfigHelper.h"

#include "TraceStream.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>


const char * g_headerFieldStrings[enLastField+1] =
{
	"First Field",

	"To Display",
	"To",
	"ToTag",
	"From Display",
	"From",
	"FromTag",
	"Contact Display",
	"Contact",
	"Contact Rendering",
	"MS Diagnostics",
	"Plcm Diagnostics",
	"Via",
	"Req Line",
	"User Agent",
	"Allow Event",
	"Roster Manager",
	"End Points",
	"Propriety Header",
	"Private Header",
	"Referred By",
	"Warning",
	"CallId",
	"Event",
	"Accept",
	"ReferTo",
	"Supported",
	"SubscrpState",
	"Authorization",
	"Allow",
	"Require",
	"ContentType",
	"MinSe",
	"SessionExpires",
	"Retry After",
	"Ms Conversation Id",
	"Ms KeepAlive Timeout",
	"Mrd",

	"kMrasUri",
	"kCredUser",
	"kCredPass",
	"kCredDuration",
	"kMRelayLocation",
	"kMRelayHostName",
	"kMRelayUdpPort",
	"kMRelayTcpPort",
	"EnableBWPolicyCheck",
	"UcMaxVideoRateAllowed",

	"AclListDelta",
	"ContactListDelta",

	"RichPresence",

	"WebRtcSdp",

	"SdpSession_v",
	"SdpSession_o",
	"SdpSession_o_display",
	"SdpSession_o_version",
	"SdpSession_s",
	"SdpSession_i",
	"SdpSession_t",
	"SdpSession_u",
	"SdpSession_e",
	"SdpSession_b_AS",
	"SdpSession_b_CT",
	"SdpSession_b_X",
	"SdpSession_b_Default",

	"Last Field"
};

const TRejectReason g_rejectReasons[] =
{
	{SipCodesTrying				, 	"Trying"						},
	{SipCodesRinging			, 	"Ringing"						},
	{SipCodesForwarded			, 	"Forwarded" 					},
	{SipCodesQueued				,	"Queued"						},
	{SipCodesSessionProgress	,	"Session Progress"				},
	{SipCodesOk					,	"Ok"							},
//	{SipCodesAccepted			,	"Accepted"						},
	{SipCodesMultiChoice		,	"Multi Choice"					},
	{SipCodesMovedPerm			,	"Moved Permanently"				},
	{SipCodesMovedTemp			,	"Moved Temporarily"				},
	{SipCodesSeeOther			,	"See Other"						},
	{SipCodesUseProxy			,	"Use Proxy"						},
	{SipCodesAltService 		,	"Alt Service"					},
	{SipCodesBadRequest 		,	"Bad Request"					},
	{SipCodesUnauthorized		,	"Unauthorized"					},
	{SipCodesPaymentReq 		,	"Payment Req"					},
	{SipCodesForbidden			,	"Forbidden"						},
	{SipCodesNotFound			,	"Not Found"						},
	{SipCodesMethNotAllowed		,	"Method Not Allowed"			},
	{SipCodesNotAcceptable		,	"Not Acceptable"				},
	{SipCodesProxyAuthReq		,	"Proxy Authentication Required"	},
	{SipCodesRequestTimeout		,	"Request Timeout"				},
	{SipCodesConflict			,	"Conflict"						},
	{SipCodesGone				,	"Gone"							},
	{SipCodesLengthReq			,	"Length Req"					},
	{SipCodesReqEntTooLarge		,	"Req Entity Too Large"			},
	{SipCodesReqUriTooLarge		,	"Req Uri Too Large"				},
	{SipCodesUnsuppMediaType	,	"Unsupported Media Type" 		},
	{SipCodesBadExtension		,	"Bad Extension"					},
	{SipCodesIntervalTooSmall	,	"Session Interval Too Small"	},
	{SipCodesTempNotAvail		,	"Temporarily Not Available"		},
	{SipCodesCallNotExist		,	"Call Not Exist"				},
	{SipCodesLoopDetected		,	"Loop Detected"					},
	{SipCodesTooManyHops		,	"Too Many Hops" 				},
	{SipCodesAddressIncomp		,	"Address Incomplete"			},
	{SipCodesAmbiguous			,	"Ambiguous"						},
	{SipCodesBusyHere			,	"Busy Here"						},
	{SipCodesRequestTerminated	,	"Request Terminated"			},
	{SipCodesNotAcceptedInHere	,	"Not Accepted In Here"			},
	{SipCodesRequestPending		,	"Request Pending"				},
	{SipCodesInternalSrvErr		,	"Internal Server Err"			},
	{SipCodesNotImplemented		,	"Not Implemented"				},
	{SipCodesBadGateway 		,	"Bad Gateway"					},
	{SipCodesServiceUnavail		,	"Service Unavailable"			},
	{SipCodesGwTimeout			,	"Gate way Timeout"				},
	{SipCodesSipVerNotSupp		,	"Sip Version Not Supported"		},
	{SipCodesBusyEveryWhere		,	"Busy Every Where"				},
	{SipCodesDecline			,	"Decline" 						},
	{SipCodesDoesNotExist		,	"Does Not Exist"				},
	{SipCodesGlobNotAcceptable	,	"Not Acceptable"				},
	{SipCodesSipUnknownStatus	,	"Sip reject reason of type %d"	}
};
char g_unknownReject[256];

const TWarningCode g_warningCodes[] =
{
	{SipWarningIncompNetProtocol	,"Incompatible network protocol"				},
	{SipWarningIncompNetAddrFormats	,"Incompatible network address formats"			},
	{SipWarningIncompTransProtocol	,"Incompatible transport protocol"				},
	{SipWarningIncompBandwidthUnits	,"Incompatible bandwidth units"					},
	{SipWarningMediaTypeNotAvail	,"Media type not available"						},
	{SipWarningIncompMediaFormat	,"Incompatible media format"					},
	{SipWarningAttrNotUnderstood	,"Attribute not understood"						},
	{SipWarningSdpParamNotUnderstood,"Session description parameter not understood"	},
	{SipWarningMulticastNotAvail	,"Multicast not available"						},
	{SipWarningUnicastNotAvail		,"Uni cast not available"						},
	{SipWarningInsufficientBandwidth,"call failed due to insufficient bandwidth"						},
	{SipWarningMiscellaneous		,"Miscellaneous warning"						},	/* Must be one before last one (see GetWarningCodeStr() below)*/
	{SipWarningUnknown				,"Unknown"										}
};

const char* g_bodyTypeStrings[] =
{
	"NoBody",
	"Sdp",
	"Xml",
	"Other" //always last
};
//////////////////////////////////////////////////////////////////////////////////////
const char * GetRejectReasonStr(enSipCodes eReason)
{
	const char * str = "";
	int i;
	for (i=0; g_rejectReasons[i].eReason != SipCodesSipUnknownStatus; i++)
	{
		if (g_rejectReasons[i].eReason == eReason)
		{
			str = g_rejectReasons[i].strReason;
			break;
		}
	}
	// unknown reason received, create an apropriate reason string
	if(g_rejectReasons[i].eReason == SipCodesSipUnknownStatus) {
		snprintf(g_unknownReject,255, g_rejectReasons[i].strReason,eReason); // B.S: klocwork 2614
		g_unknownReject[255] = '\0';										 // B.S: klocwork 2614
		str = g_unknownReject;
	}
	return str;
}
//////////////////////////////////////////////////////////////////////////////////////
const char * GetWarningCodeStr(enSipWarningCodes eWarning)
{
	const char * str = "";
	int i;
	for (i=0; g_warningCodes[i].eWarning != SipWarningUnknown; i++)
	{
		if (g_warningCodes[i].eWarning == eWarning)
		{
			str = g_warningCodes[i].strWarning;
			break;
		}
	}

	if(g_warningCodes[i].eWarning == SipWarningUnknown)
		str = g_warningCodes[i-1].strWarning;

	return str;
}

//////////////////////////////////////////////////////////////////////////////////////
enSipCodes TranslateLobbyRejectReasonToSip(int reason)
{
	enSipCodes  eRejectReason;
	switch(reason)
	{
	case -1:
		eRejectReason = SipCodesOk;
		break;
	case cmReasonTypeCallForwarded:
		eRejectReason = SipCodesMovedTemp;
		break;
	case cmReasonTypeNoPermision:
		eRejectReason = SipCodesForbidden;
		break;
	case cmReasonTypeNoBandwidth:
		eRejectReason = SipCodesBusyHere;
		break;
	default:
		DBGFPASSERT(reason);
		eRejectReason = SipCodesInternalSrvErr;
		break;
	}
	return eRejectReason;
}


//////////////////////////////////////////////////////////////////////////////////////
BOOL AddCapInMediaLine(capBuffer* pCapBuffer, int capBufferSize, sipMediaLineSt *pMediaLine, int mediaLineBufSize, int &capPos)
{
	if (!pMediaLine)
		return FALSE;

	if (capPos + capBufferSize + (int)sizeof(sipMediaLineBaseSt) > mediaLineBufSize)
		return FALSE;

	memcpy(&pMediaLine->caps[capPos], pCapBuffer, capBufferSize);
	capPos += capBufferSize;

	pMediaLine->numberOfCaps++;
	pMediaLine->lenOfDynamicSection += capBufferSize;

	return TRUE;
}
///////////////////////////////////////////////////////
BOOL AddCapInBuffer(capBuffer* pCapBuffer, int capBufferSize, char *pCapBuf, int capBufSize, int &capPos)
{

	if (capPos + capBufferSize > capBufSize)
		return FALSE;

	memcpy(pCapBuf + capPos, pCapBuffer, capBufferSize);
	capPos += capBufferSize;

	return TRUE;
}
///////////////////////////////////////////////////////
mcXmlTransportAddress &ExtractMLineMediaIp(eMediaLineInternalType intType, const sipMediaLinesEntrySt *pMediaLinesEntry, mcXmlTransportAddress &dummyMediaIp, BYTE confIsEncrypt, int index)
{
	sipMediaLineSt *pMediaLine = NULL;
	unsigned int mediaLinePos = 0;

	if (pMediaLinesEntry && pMediaLinesEntry->lenOfDynamicSection) {
		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

			if (mediaLinePos >= pMediaLinesEntry->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			//checking if there is sdes cap in the media line:
			BYTE thereIsSdesCap = FALSE;
			const capBuffer* pCapBuffer = (capBuffer*) &pMediaLine->caps[0];
			const BYTE*	pTemp           = (const BYTE*)pCapBuffer;
			for (unsigned int j = 0 ; j < pMediaLine->numberOfCaps; j++)
			{
				if ((CapEnum)pCapBuffer->capTypeCode == eSdesCapCode)
				{
//					FPTRACE(eLevelInfoNormal,"ExtractMLineRtcpIp -FOUND SDES");
					thereIsSdesCap = TRUE;
				}
				pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
				pCapBuffer = (capBuffer*)pTemp;
			}

			//(if the conf is encrypted but there in no srtp) or (if the conf is encrypted, there is srtp but there is no SDES)
			//or (if the conf is not encrypted but there is srtp) don't take this media line.
			BYTE TakeThisMediaLine = TRUE;
			if ( ( (confIsEncrypt == FALSE) &&  (pMediaLine->subType == eMediaLineSubTypeRtpSavp) ) ||
				 ( (confIsEncrypt == TRUE)  &&  (pMediaLine->subType == eMediaLineSubTypeRtpSavp) && (thereIsSdesCap == FALSE) ) ||
				 ( (confIsEncrypt == TRUE)  &&  !(pMediaLine->subType == eMediaLineSubTypeRtpSavp) ) ) {
				TakeThisMediaLine = FALSE;
				FPTRACE(eLevelInfoNormal,"ExtractMLineMediaIp -don't take this media line");
			}

			if (index != -1)
			{
				if (i == (unsigned int)index)
					return pMediaLine->mediaIp;
			}
			else if ((intType == pMediaLine->internalType) && (TakeThisMediaLine == TRUE))
				return pMediaLine->mediaIp;
		}

		//If we didn't find a port that is matched to the conf (encrypted/srtp/SDES) we will take the first media line:
		mediaLinePos = 0;
		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {
			if (mediaLinePos >= pMediaLinesEntry->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			if (index != -1)
			{
				if (i == (unsigned int)index)
					return pMediaLine->mediaIp;
			}
			else if (intType == pMediaLine->internalType)
				return pMediaLine->mediaIp;
		}
	}

	memset(&dummyMediaIp, 0, sizeof(dummyMediaIp));

	return dummyMediaIp;
}
mcXmlTransportAddress &ExtractMLineMediaIp(eMediaLineInternalType intType, const sipSdpAndHeadersSt *pSdp, mcXmlTransportAddress &dummyMediaIp, BYTE confIsEncrypt, int index)
{
	if (pSdp && pSdp->sipMediaLinesLength) {
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdp->capsAndHeaders[pSdp->sipMediaLinesOffset];
		return ExtractMLineMediaIp(intType, pMediaLinesEntry, dummyMediaIp, confIsEncrypt, index);
	}

	memset(&dummyMediaIp, 0, sizeof(dummyMediaIp));

	return dummyMediaIp;
}
///////////////////////////////////////////////////////
mcXmlTransportAddress &ExtractMLineMediaIpAccordingToMediaType(eMediaLineType type, const sipMediaLinesEntrySt *pMediaLinesEntry, mcXmlTransportAddress &dummyMediaIp, BYTE confIsEncrypt, int index)
{
	sipMediaLineSt *pMediaLine = NULL;
	unsigned int mediaLinePos = 0;
	BYTE firstMediaLineIpSet = FALSE;
	mcXmlTransportAddress *firstMediaLineIp = NULL;

	if (pMediaLinesEntry && pMediaLinesEntry->lenOfDynamicSection) 
	{
		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) 
		{
			if (mediaLinePos >= pMediaLinesEntry->lenOfDynamicSection) 
			{
				DBGFPASSERT(mediaLinePos);
				break;
			}
			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			//checking if there is sdes cap in the media line:
			BYTE thereIsSdesCap = FALSE;
			const capBuffer* pCapBuffer = (capBuffer*) &pMediaLine->caps[0];
			const BYTE*	pTemp           = (const BYTE*)pCapBuffer;
			for (unsigned int j = 0 ; j < pMediaLine->numberOfCaps; j++)
			{
				if ((CapEnum)pCapBuffer->capTypeCode == eSdesCapCode)
				{
					thereIsSdesCap = TRUE;
				}
				pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
				pCapBuffer = (capBuffer*)pTemp;
			}
			//(if the conf is encrypted but there in no srtp) or (if the conf is encrypted, there is srtp but there is no SDES)
			//or (if the conf is not encrypted but there is srtp) don't take this media line.
			BYTE TakeThisMediaLine = TRUE;
			if ( ( (confIsEncrypt == FALSE) &&  (pMediaLine->subType == eMediaLineSubTypeRtpSavp) ) ||
				 ( (confIsEncrypt == TRUE)  &&  (pMediaLine->subType == eMediaLineSubTypeRtpSavp) && (thereIsSdesCap == FALSE) ) ||
				 ( (confIsEncrypt == TRUE)  &&  !(pMediaLine->subType == eMediaLineSubTypeRtpSavp) ) ) 
			{
				TakeThisMediaLine = FALSE;
				FPTRACE(eLevelInfoNormal,"ExtractMLineMediaIp -don't take this media line");
			}
			if (index != -1)
			{
				if (i == (unsigned int)index)
				{
					return pMediaLine->mediaIp;
				}
			}
			else if (type == pMediaLine->type)
			{
				if (TakeThisMediaLine == TRUE)
				{
					return pMediaLine->mediaIp;
				}
				else if (firstMediaLineIpSet == FALSE)
				{
					firstMediaLineIp = &pMediaLine->mediaIp;
					firstMediaLineIpSet = TRUE;
				}
			}
		}
		//If we didn't find a port that is matched to the conf (encrypted/srtp/SDES) we will take the first media line:
		if (firstMediaLineIpSet == TRUE)
		{
			return *firstMediaLineIp;
		}
	}
	memset(&dummyMediaIp, 0, sizeof(dummyMediaIp));
	return dummyMediaIp;
}

///////////////////////////////////////////////////////
mcXmlTransportAddress &ExtractMLineMediaIpAccordingToMediaType(eMediaLineType type, const sipSdpAndHeadersSt *pSdp, mcXmlTransportAddress &dummyMediaIp, BYTE confIsEncrypt, int index)
{
	if (pSdp && pSdp->sipMediaLinesLength) {
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdp->capsAndHeaders[pSdp->sipMediaLinesOffset];
		return ExtractMLineMediaIpAccordingToMediaType(type, pMediaLinesEntry, dummyMediaIp, confIsEncrypt, index);
	}
	memset(&dummyMediaIp, 0, sizeof(dummyMediaIp));
	return dummyMediaIp;
}
///////////////////////////////////////////////////////
unsigned int &ExtractMLineRtcpPort(eMediaLineInternalType intType, const sipSdpAndHeadersSt *pSdp, unsigned int &dummyRtcpPort, BYTE confIsEncrypt, int index)
{
	sipMediaLineSt *pMediaLine = NULL;
	unsigned int mediaLinePos = 0;

	if (pSdp && pSdp->sipMediaLinesLength) {
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdp->capsAndHeaders[pSdp->sipMediaLinesOffset];
		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

			if (mediaLinePos >= pSdp->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			//checking if there is sdes cap in the media line:
			BYTE thereIsSdesCap = FALSE;
			const capBuffer* pCapBuffer = (capBuffer*) &pMediaLine->caps[0];
			const BYTE*	pTemp           = (const BYTE*)pCapBuffer;
			for (unsigned int j = 0 ; j < pMediaLine->numberOfCaps; j++)
			{
				if ((CapEnum)pCapBuffer->capTypeCode == eSdesCapCode)
				{
//					FPTRACE(eLevelInfoNormal,"ExtractMLineRtcpPort -FOUND SDES");
					thereIsSdesCap = TRUE;
				}
				pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
				pCapBuffer = (capBuffer*)pTemp;
			}

			//(if the conf is encrypted but there in no srtp) or (if the conf is encrypted, there is srtp but there is no SDES)
			//or (if the conf is not encrypted but there is srtp) don't take this media line.
			BYTE TakeThisMediaLine = TRUE;
			if ( ( (confIsEncrypt == FALSE) &&  (pMediaLine->subType == eMediaLineSubTypeRtpSavp) ) ||
				 ( (confIsEncrypt == TRUE)  &&  (pMediaLine->subType == eMediaLineSubTypeRtpSavp) && (thereIsSdesCap == FALSE) ) ||
				 ( (confIsEncrypt == TRUE)  &&  !(pMediaLine->subType == eMediaLineSubTypeRtpSavp) ) ||
				 ( ((confIsEncrypt == TRUE)  &&  !( (pMediaLine->subType == eMediaLineSubTypeTcpBfcp)    ||
				   									(pMediaLine->subType == eMediaLineSubTypeTcpTlsBfcp) || 
													(pMediaLine->subType == eMediaLineSubTypeUdpBfcp) 
				 	                              ) 
				   )
				 )
			   )
			{
					TakeThisMediaLine = FALSE;
					FPTRACE(eLevelInfoNormal,"ExtractMLineRtcpPort -don't take this media line");
			}

			if (index != -1)
			{
				if (i == (unsigned int)index)
					return pMediaLine->rtcpPort;
			}
			else if ((intType == pMediaLine->internalType) && (TakeThisMediaLine == TRUE))
				return pMediaLine->rtcpPort;
		}

		//If we didn't find a port that is matched to the conf (encrypted/srtp/SDES) we will take the first media line:
		mediaLinePos = 0;
		pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdp->capsAndHeaders[pSdp->sipMediaLinesOffset];
		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

			if (mediaLinePos >= pSdp->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			if (index != -1)
			{
				if (i == (unsigned int)index)
					return pMediaLine->rtcpPort;
			}
			else if (intType == pMediaLine->internalType)
				return pMediaLine->rtcpPort;
		}
	}

	dummyRtcpPort = 0;

	return dummyRtcpPort;
}

///////////////////////////////////////////////////////
eMediaLineInternalType GetMediaLineInternalType(const sipSdpAndHeadersSt *pSdp, int index) 
{
	sipMediaLineSt *pMediaLine = NULL;
	unsigned int mediaLinePos = 0;

	if (pSdp && pSdp->sipMediaLinesLength) {
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdp->capsAndHeaders[pSdp->sipMediaLinesOffset];
		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

			if (mediaLinePos >= pSdp->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			if (i == (unsigned int)index) {
				return (eMediaLineInternalType)pMediaLine->internalType;
			}

		}
	}
	return kMediaLineInternalTypeNone;
}

///////////////////////////////////////////////////////
ESipMediaChannelType GetSipMediaChannelType(cmCapDataType mediaType, ERoleLabel eRole)
{
	switch(mediaType)
	{
		case(cmCapAudio):
			if(eRole & kRoleContentOrPresentation)
				return kSipMediaChannelAudioContent;
			else
				return kSipMediaChannelAudio;

		case(cmCapVideo):
				if(eRole & kRoleContentOrPresentation)
					return kSipMediaChannelVideoContent;
				else
					return kSipMediaChannelVideo;

		case(cmCapData):
			return kSipMediaChannelFecc;

		default:
			return kNumOfSipMediaChannelTypes;
	}
}

///////////////////////////////////////////////////////
sipMediaLineSt* GetMediaLine(sipSdpAndHeadersSt &sdp, eMediaLineInternalType intType, int index)
{
	sipMediaLineSt *pMediaLine = NULL;
	unsigned int mediaLinePos = 0;
	int curIndex = 0;
	if (sdp.sipMediaLinesLength) {
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&sdp.capsAndHeaders[sdp.sipMediaLinesOffset];
		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

			if (mediaLinePos >= sdp.lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			//FPTRACE2INT(eLevelInfoNormal,"GetMediaLine: to want to find type = ", intType);   //debug for ANAT
			//FPTRACE2INT(eLevelInfoNormal,"GetMediaLine: now type = ", (eMediaLineInternalType)pMediaLine->internalType);  
			if (intType == (eMediaLineInternalType)pMediaLine->internalType)  //modified for ANAT
			{
				if ((curIndex==index) || (index == -1))
					return pMediaLine;
				curIndex++;
			}
		}
	}
	return NULL;
}

//added for ANAT begin
sipMediaLineSt* GetMediaLineAtIndex(sipSdpAndHeadersSt &sdp, int index)
{
	sipMediaLineSt *pMediaLine = NULL;
	unsigned int mediaLinePos = 0;
	
	if (sdp.sipMediaLinesLength) {
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&sdp.capsAndHeaders[sdp.sipMediaLinesOffset];
		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

			if (mediaLinePos >= sdp.lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			if (i == (unsigned int)index)
				return pMediaLine;
		}
	}
	
	return NULL;
}

/*sipMediaLineSt* GetOtherMediaLineInMidGroup(sipSdpAndHeadersSt &sdp, int mid)
{
	sipMediaLineSt *pMediaLine = NULL;
	unsigned int mediaLinePos = 0;
	
	if (sdp.sipMediaLinesLength) {
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&sdp.capsAndHeaders[sdp.sipMediaLinesOffset];
		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

			if (mediaLinePos >= sdp.lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			for (unsigned int i=0; i<MAX_GROUP_ANAT_MEMBER_NUM; i++) {
				if (mid == pMediaLine->midAnatGroup[i] && pMediaLine->mid != mid && pMediaLine->mid != 0)
					return pMediaLine;	
			}
		}
	}
	return NULL;
}*/

int GetIndexAccordingToInternalTypeAndIpVersion(eMediaLineInternalType intType, sipSdpAndHeadersSt *pSdp, enIpVersion selectedIpVersion)
{
	sipMediaLineSt *pMediaLine = NULL;
	unsigned int mediaLinePos = 0;
	int i = -1;

	if (pSdp && pSdp->sipMediaLinesLength) {
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdp->capsAndHeaders[pSdp->sipMediaLinesOffset];
		
		for (i = 0; i < (int)pMediaLinesEntry->numberOfMediaLines; i++) {
			if (mediaLinePos >= pSdp->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			
			if ((intType == (eMediaLineInternalType)pMediaLine->internalType) && (selectedIpVersion == (enIpVersion)pMediaLine->mediaIp.transAddr.ipVersion)) 
				return i;
		}
	}

	return -1;
}

sipMediaLineSt* GetMLineAccordingToInternalTypeAndIpVersion(eMediaLineInternalType intType, sipSdpAndHeadersSt *pSdp, enIpVersion selectedIpVersion)
{
	sipMediaLineSt *pMediaLine = NULL;
	unsigned int mediaLinePos = 0;

	if (pSdp && pSdp->sipMediaLinesLength) {
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdp->capsAndHeaders[pSdp->sipMediaLinesOffset];
		
		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {
			if (mediaLinePos >= pSdp->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			
			if ((intType == (eMediaLineInternalType)pMediaLine->internalType) && (selectedIpVersion == (enIpVersion)pMediaLine->mediaIp.transAddr.ipVersion)) 
				return pMediaLine;
		}
	}

	return NULL;
}

BOOL IsAVMCUCall(sipSdpAndHeadersSt* pSdpAndHeaders)
{
	if (pSdpAndHeaders && pSdpAndHeaders->sipHeadersLength)
	{
		sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];

		if (!pHeaders)
			return false;

		char cUserAgent[MaxUserAgentSize] = {0};
		::SipGetHeaderValue(pHeaders, kUserAgent, cUserAgent, MaxUserAgentSize);

		if (strlen(cUserAgent) > 0)
		{
			if( strstr(cUserAgent, "RTCC/5") && strstr(cUserAgent, "AV-MCU") )
			{
				return true;
			}
		}
	}
		return false;
}

BOOL IsANATPresentInSDP(const sipSdpAndHeadersSt* pSdp)
{
	BOOL	IsANATContained = FALSE;
	unsigned int mediaLinePos = 0;
	sipMediaLineSt *pMediaLine = NULL;

	if (pSdp && pSdp->sipMediaLinesLength)
	{
		const sipMediaLinesEntrySt* pMediaLinesEntry = (sipMediaLinesEntrySt *) &pSdp->capsAndHeaders[pSdp->sipMediaLinesOffset];

		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {
			if (mediaLinePos >= pSdp->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}
			
			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			if (pMediaLine->mediaIp.transAddr.port == 0)
				continue;

			{
				BOOL	IsMidZeroFound = FALSE;	
				
				for (int i=0; i<MAX_GROUP_ANAT_MEMBER_NUM; i++) 
				{
					if (pMediaLine->midAnatGroup[i] == 0)
						if (!IsMidZeroFound)
							IsMidZeroFound = TRUE;
						else
						{
							IsANATContained = FALSE;
							FPTRACE2INT(eLevelInfoNormal,"IsANATPresentInSDP: ", IsANATContained);  
							return IsANATContained;
						}
					else
					{
						IsANATContained = TRUE;
						FPTRACE2INT(eLevelInfoNormal,"IsANATPresentInSDP: ", IsANATContained);  
						return IsANATContained;
					}




				}
			}




		}
	}	
	FPTRACE2INT(eLevelInfoNormal,"IsANATPresentInSDP: ", IsANATContained);  
	return IsANATContained;
}

enIpVersion GetMediaLineIpVersionlType(const sipSdpAndHeadersSt *pSdp, int index) 
{
	sipMediaLineSt *pMediaLine = NULL;
	unsigned int mediaLinePos = 0;

	if (pSdp && pSdp->sipMediaLinesLength) {
		sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdp->capsAndHeaders[pSdp->sipMediaLinesOffset];
		for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

			if (mediaLinePos >= pSdp->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			if (i == (unsigned int)index) {
				return (enIpVersion)pMediaLine->mediaIp.transAddr.ipVersion;
			}

		}
	}
	return enIpVersionMAX;
}

//added for ANAT end

//Amihay: the same field is used in rtcpFeedbackMask
//BYTE SetFirFlag(sipSdpAndHeadersSt &sdp, eMediaLineInternalType intType)
//{
//	sipMediaLineSt *pMediaLine = NULL;
//	pMediaLine = GetMediaLine(sdp, intType);
//	if (pMediaLine)
//	{
//		pMediaLine->intraFlag = TRUE;
//		return TRUE;
//	}
//	return FALSE;
//}
///////////////////////////////////////////////////////
BYTE IsMediaContainedInSdp(sipSdpAndHeadersSt *pSdpAndHeaders, eMediaLineInternalType intType)
{
	if ( pSdpAndHeaders && pSdpAndHeaders->sipMediaLinesLength )
	{
		const sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipMediaLinesOffset];
		unsigned int mediaLinePos = 0;

		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {


			if (mediaLinePos >= pSdpAndHeaders->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			const capBuffer *pCapBuffer = (capBuffer *) &pMediaLine->caps[0];
			const BYTE *pTemp = (const BYTE*)pCapBuffer;

			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			if (intType == pMediaLine->internalType)
			{
				return YES;
			}

//			for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
//			{
//				BaseCapStruct *pBaseCap  = (BaseCapStruct*)pCapBuffer->dataCap;
//				cmCapDataType eSdpType = (cmCapDataType)pBaseCap->header.type;
//
//				if(eSdpType == eType)
//					return YES;
//
//				pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
//				pCapBuffer = (capBuffer*)pTemp;
//			}
		}
	}

	return NO;
}
///////////////////////////////////////////////////////
BYTE IsMediaContainedInDirtySdp(sipMediaLinesEntrySt* pMediaLinesEntry, eMediaLineInternalType intType)
{

	if(pMediaLinesEntry)
	{
		unsigned int mediaLinePos = 0;
		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++)
		{
			const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];

			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			if (intType == pMediaLine->internalType)
				return YES;

		}
	}

	return NO;
}
///////////////////////////////////////////////////////
BOOL IsSdpPresent(sipSdpAndHeadersSt *pSdpAndHeaders)
{
	sipMediaLinesEntrySt    *pMediaLinesEntry         = NULL;
	sipMediaLineSt     *pMediaLine                     = NULL;
	int i  = 0;
	int mediaLinePos = 0;
	BOOL sdpMissing = FALSE;
	BOOL partyHasSDES = FALSE;

	// It mean no dynamic part or capLength <= 0
	//------------------------------------------
	FPASSERT_AND_RETURN_VALUE(NULL == pSdpAndHeaders, FALSE);
	if (pSdpAndHeaders->sipMediaLinesLength == 0)
	{
		FPTRACE(eLevelInfoNormal,"IsSdesDeclaredInSdp - sipMediaLinesLength is 0");
		sdpMissing = TRUE;
	}
	else
	{
		// it means caps header exist but caps is zero.
		//---------------------------------------------
		pMediaLinesEntry = (sipMediaLinesEntrySt *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipMediaLinesOffset];

		if (!pMediaLinesEntry)
		{
			FPTRACE(eLevelInfoNormal,"IsSdesDeclaredInSdp - pMediaLinesEntry is NULL");
			sdpMissing = TRUE;
		}

		if (!sdpMissing && pMediaLinesEntry->numberOfMediaLines == 0)
		{
			FPTRACE(eLevelInfoNormal,"IsSdesDeclaredInSdp - numberOfMediaLines is 0");
			sdpMissing = TRUE;
		}

		for (i = 0; !sdpMissing && i < (int) pMediaLinesEntry->numberOfMediaLines; i++) {

			pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];

			if (!pMediaLine)
			{
				FPTRACE(eLevelInfoNormal,"IsSdesDeclaredInSdp - pMediaLine is NULL");
				sdpMissing = TRUE;
			}
			else
			{
				if (pMediaLine->numberOfCaps)
				{
					sdpMissing = FALSE;
					break;
				}

				mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			}
		}
	}
	return !sdpMissing;
}
///////////////////////////////////////////////////////
BOOL IsSdesDeclaredInSdp(sipSdpAndHeadersSt *pSdpAndHeaders)
{
	sipMediaLinesEntrySt    *pMediaLinesEntry         = NULL;
	sipMediaLineSt     *pMediaLine                     = NULL;
	int i  = 0;
	int mediaLinePos = 0;

	if (IsSdpPresent(pSdpAndHeaders))
	{

		sipMediaLinesEntrySt* pMediaLinesEntry = (sipMediaLinesEntrySt*)pSdpAndHeaders->capsAndHeaders;
		int mediaLinePos = 0;

		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {

		const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
		const capBuffer* pCapBuffer = (capBuffer*) &pMediaLine->caps[0];
		const BYTE* pTemp = (const BYTE*)pCapBuffer;

		mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

		for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
		{
			if ((CapEnum)pCapBuffer->capTypeCode == eSdesCapCode)
			{
				FPTRACE(eLevelInfoNormal,"IsSdesDeclaredInSdp -FOUND SDES");
					return TRUE;
			}

			pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)pTemp;
			}
		}
	}

	return FALSE;
}
DWORD GetNumberOfMediaLinesOfIntenalType(sipSdpAndHeadersSt *pSdpAndHeaders, eMediaLineInternalType intType, int &sizeOfMediaLines)
{
	DWORD retNumber = 0;
	sizeOfMediaLines = 0;


	if ( pSdpAndHeaders && pSdpAndHeaders->sipMediaLinesLength )
	{
		const sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipMediaLinesOffset];
		unsigned int mediaLinePos = 0;

		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {


			if (mediaLinePos >= pSdpAndHeaders->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			const capBuffer *pCapBuffer = (capBuffer *) &pMediaLine->caps[0];
			const BYTE *pTemp = (const BYTE*)pCapBuffer;

			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			if (intType == pMediaLine->internalType) {
				sizeOfMediaLines += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
				retNumber++;
			}
		}
	}

	FPTRACE2INT(eLevelInfoNormal,"GetNumberOfMediaLinesOfIntenalType - for ANAT type: ", intType);   //debug for ANAT
	FPTRACE2INT(eLevelInfoNormal,"GetNumberOfMediaLinesOfIntenalType - for ANAT number = ", retNumber); 
	return retNumber;
}

BOOL IsPanoramicVideoMediaLine(const sipMediaLineSt *pMediaLine)
{
	return (kMediaLineInternalTypeVideo == pMediaLine->internalType && strstr(pMediaLine->label, "panoramic-video")) ? TRUE : FALSE;
}

DWORD GetNumberOfMediaLinesOfPanoramicType(sipSdpAndHeadersSt *pSdpAndHeaders, int &sizeOfPanoramicMediaLines)
{
	DWORD retNumber = 0;
	sizeOfPanoramicMediaLines = 0;

	if ( pSdpAndHeaders && pSdpAndHeaders->sipMediaLinesLength )
	{
		const sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipMediaLinesOffset];
		unsigned int mediaLinePos = 0;
		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++)
		{
			if (mediaLinePos >= pSdpAndHeaders->lenOfDynamicSection)
			{
				DBGFPASSERT(mediaLinePos);
				break;
			}
			const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			const capBuffer *pCapBuffer = (capBuffer *) &pMediaLine->caps[0];
			const BYTE *pTemp = (const BYTE*)pCapBuffer;

			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			if (kMediaLineInternalTypeVideo == pMediaLine->internalType && IsPanoramicVideoMediaLine(pMediaLine))
			{
				FPTRACE(eLevelInfoNormal,"GetNumberOfMediaLinesOfPanoramicType - panoramic video line found ");
				sizeOfPanoramicMediaLines += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
				retNumber++;
			}
		}
	}

	return retNumber;
}

///////////////////////////////////////////////////////
void GetMediaDataTypeAndRole(ESipMediaChannelType sipMediaType, cmCapDataType &retMediaType, ERoleLabel &retRole)
{
	retRole = kRolePeople;
	retMediaType = cmCapEmpty;

	switch (sipMediaType)
	{
		case kSipMediaChannelAudio:
			retMediaType 	= cmCapAudio;
			break;
		case kSipMediaChannelVideo:
			retMediaType 	= cmCapVideo;
			break;
		case kSipMediaChannelFecc:
			retMediaType 	= cmCapData;
			break;
		case kSipMediaChannelContent:
			retMediaType 	= cmCapVideo;
			retRole	  		= kRolePresentation;
			break;
		case kSipMediaChannelBfcp:
			retMediaType 	= cmCapBfcp;
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
}
///////////////////////////////////////////////////////
void GetMediaDataTypeAndRole(eMediaLineInternalType sipMediaLineIntType, cmCapDataType &retMediaType, ERoleLabel &retRole)
{
	retRole = kRolePeople;
	retMediaType = cmCapEmpty;

	switch (sipMediaLineIntType)
	{
		case kMediaLineInternalTypeAudio:
			retMediaType = cmCapAudio;
			break;
		case kMediaLineInternalTypeVideo:
			retMediaType = cmCapVideo;
			break;
		case kMediaLineInternalTypeFecc:
			retMediaType = cmCapData;
			break;
		case kMediaLineInternalTypeContent:
			retMediaType = cmCapVideo;
			retRole	  = kRolePresentation;
			break;
		case kMediaLineInternalTypeBfcp:
			retMediaType = cmCapBfcp;
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
}

eMediaLineInternalType GetMediaLineInternalType(ESipMediaChannelType sipMediaType)
{
	eMediaLineInternalType retType = kMediaLineInternalTypeNone;
	switch (sipMediaType)
	{
		case kSipMediaChannelAudio:
			retType = kMediaLineInternalTypeAudio;
			break;
		case kSipMediaChannelVideo:
			retType = kMediaLineInternalTypeVideo;
			break;
		case kSipMediaChannelFecc:
			retType = kMediaLineInternalTypeFecc;
			break;
		case kSipMediaChannelContent:
			retType = kMediaLineInternalTypeContent;
			break;
		case kSipMediaChannelBfcp:
			retType = kMediaLineInternalTypeBfcp;
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	return retType;
}
///////////////////////////////////////////////////////
eMediaLineInternalType GetMediaLineInternalType(cmCapDataType eDataType, ERoleLabel eRole)
{
	eMediaLineInternalType retType = kMediaLineInternalTypeNone;
	switch (eDataType)
	{
		case cmCapAudio:
			retType = kMediaLineInternalTypeAudio;
			break;
		case cmCapVideo:
			if (eRole == kRolePeople)
				retType = kMediaLineInternalTypeVideo;
			else
				retType = kMediaLineInternalTypeContent;
			break;
		case cmCapData:
			retType = kMediaLineInternalTypeFecc;
			break;
		case cmCapBfcp:
			retType = kMediaLineInternalTypeBfcp;
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	return retType;
}
///////////////////////////////////////////////////////
eMediaLineSubType ConvertBfcpTransportToMediaLineSubType(enTransportType transType)
{
	eMediaLineSubType subType = eMediaLineSubTypeNull;

	switch(transType)
	{
		case eTransportTypeUdp:
			subType = eMediaLineSubTypeUdpBfcp;
			break;
		case eTransportTypeTls:
			subType = eMediaLineSubTypeTcpTlsBfcp;
			break;
		case eTransportTypeTcp:
			subType = eMediaLineSubTypeTcpBfcp;
			break;
		default:
			break;
	}
	return subType;
}
///////////////////////////////////////////////////////
enTransportType ConvertBfcpMediaLineSubTypeToTransportType(eMediaLineSubType subType)
{
	enTransportType transType = eUnknownTransportType;

	switch(subType)
	{
		case eMediaLineSubTypeUdpBfcp:
			transType = eTransportTypeUdp;
			break;
		case eMediaLineSubTypeTcpTlsBfcp:
			transType = eTransportTypeTls;
			break;
		case eMediaLineSubTypeTcpBfcp:
			transType = eTransportTypeTcp;
			break;
		default:
			break;
	}
	return transType;
}
///////////////////////////////////////////////////////
eBfcpSetup ConvertBFCPSetupTypeToApi(eBFCPSetup setup)
{
	switch(setup)
	{
		case kSetupActive	: return bfcp_setup_active;
		case kSetupPassive	: return bfcp_setup_passive;
		case kSetupActPass	: return bfcp_setup_actpass;
		case kSetupHoldConn	: return bfcp_setup_holdconn;
		default				: return bfcp_setup_null;
	}
}
///////////////////////////////////////////////////////
eBfcpConnection ConvertBFCPConnectionTypeToApi(eBFCPConnection connection)
{
	switch(connection)
	{
		case kConnectionNew			: return bfcp_connection_new;
		case kConnectionExisting	: return bfcp_connection_existing;
		default						: return bfcp_connection_null;
	}
}
///////////////////////////////////////////////////////
eBFCPSetup ConvertApiToBFCPSetupType(eBfcpSetup setup)
{
	switch(setup)
	{
		case bfcp_setup_active	: return kSetupActive;
		case bfcp_setup_passive	: return kSetupPassive;
		case bfcp_setup_actpass	: return kSetupActPass;
		case bfcp_setup_holdconn: return kSetupHoldConn;
		default					: return kSetupActPass;
	}
}
///////////////////////////////////////////////////////
eBFCPConnection ConvertApiToBFCPConnectionType(eBfcpConnection connection)
{
	switch(connection)
	{
		case bfcp_connection_new		: return kConnectionNew;
		case bfcp_connection_existing	: return kConnectionExisting;
		default							: return kConnectionNew;
	}
}
///////////////////////////////////////////////////////
eBfcpFloorCtrl GetAnswerFloorCtrl(eBfcpFloorCtrl offererFloorCtrl)
{
	switch(offererFloorCtrl)
	{
		case bfcp_flctrl_c_s: 	 return bfcp_flctrl_s_only;
		case bfcp_flctrl_c_only: return bfcp_flctrl_s_only;
		case bfcp_flctrl_s_only: return bfcp_flctrl_c_only;
		default: 				 return bfcp_flctrl_s_only;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//-------------CSipHeader----------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////
CSipHeader::CSipHeader():m_eField(kFirstField),m_flags(0),m_strLen(0),m_strHeader(NULL)
{
}
///////////////////////////////////////////////////////////////////////////////////////

CSipHeader::CSipHeader(enHeaderField eField,BYTE flags,int strLen,const char * headerStr)
: m_eField(eField),m_flags(flags),m_strLen(strLen)
{
	if (headerStr )
	{
		m_strHeader = new char[strLen+1];
		memcpy(m_strHeader,headerStr,strLen);
		m_strHeader[strLen] = 0;
	}
	else
	{
		m_strHeader = NULL;
		m_strLen	= 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
CSipHeader::CSipHeader(const CSipHeader & other) : CPObject(other), m_eField(kFirstField),m_flags(0),m_strLen(0),m_strHeader(NULL)
{

	*this = other;
}

//////////////////////////////////////////////////////////////////////////////////////
CSipHeader::~CSipHeader()
{
	PDELETEA(m_strHeader);
}
//////////////////////////////////////////////////////////////////////////////////////
void CSipHeader::Serialize(WORD format,CSegment& seg) const
{
    switch (format)
    {
    case NATIVE:
		seg << (WORD)m_eField;
		seg << (WORD)m_strLen;
		if (m_strLen)
			seg.Put((BYTE *)m_strHeader,m_strLen);
        break;

    default :
        break;
    }

}
//////////////////////////////////////////////////////////////////////////////////////
void CSipHeader::DeSerialize(WORD format,CSegment& seg)
{
    switch (format)
    {
    case NATIVE:
		WORD eField;
		WORD strLen;
		seg >> eField;
		seg >> strLen;
		m_eField = (enHeaderField)eField;
		m_strLen = (int)strLen;
		if (m_strLen)
		{
			PDELETEA(m_strHeader);
			m_strHeader = new char[m_strLen+1];
			seg.Get((BYTE *)m_strHeader,m_strLen);
			m_strHeader[m_strLen] = 0;
		}
		m_flags	= 0;
        break;

    default :
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////
CSipHeader& CSipHeader::operator=(const CSipHeader& other)
{
	if(this != &other)
	{
		m_eField	= other.m_eField;
		m_flags		= 0; // zero flag when copied
		m_strLen	= other.m_strLen;
		PDELETEA(m_strHeader);
		m_strHeader = new char[other.m_strLen+1];
		memcpy(m_strHeader,other.m_strHeader,other.m_strLen);
		m_strHeader[other.m_strLen] = 0;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////
// unlike regular header, private header string contains header type and name
// this method returns only header name.
// for example: calling this method on following header 'P-Preferred-Identity: "Alice" <sip:mobile1@homnet.com>'
// returns - ' "Alice" <sip:mobile1@homnet.com>'

const char* CSipHeader::GetPrivateHeaderStr(const char* strHeaderType) const
{
	const char* str = NULL;

	// this method should be run only on private headers
	if (m_eField != kPrivateHeader) return NULL;

	const char* strHeader = GetHeaderStr();
	int headerLen = strlen(strHeader);
	int headerTypeLen = strlen(strHeaderType);

	if (strncmp(strHeader,strHeaderType,headerTypeLen) == 0 && strHeader[headerTypeLen] == ':')
		// set str to beginning of header content
	  	str = strHeader+headerTypeLen+1;
	return str;
}

void CSipHeader::GetHeaderStrLower(char *str, int len) const
{
	int minlen = (m_strLen > len) ? len : m_strLen;

	for (int i = 0; i < minlen; i++)
		str[i] = tolower(m_strHeader[i]);
}

//////////////////////////////////////////////////////////////////////////////////////
//-------------CSipHeaderList----------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////
CSipHeaderList::~CSipHeaderList()
{
	CleanArr();
}
///////////////////////////////////////////////////////////////////////////////////////
CSipHeaderList::CSipHeaderList(const CSipHeaderList & other) : CPObject(other), m_maxAlloc(0),m_numOfElem(0),m_pHeadersArr(NULL)
{
	*this = other;
}

///////////////////////////////////////////////////////////////////////////////////////
CSipHeaderList::CSipHeaderList(const sipMessageHeaders & headers)
{
	m_numOfElem = headers.numOfHeaders;
	m_maxAlloc  = m_numOfElem;

	// MFW Core FSN-671, BRIDGE-18254
	if(headers.numOfHeaders > 1000)
	{
		PTRACE(eLevelError, "headers.numOfHeaders > 1000 !!!!!!!!!!!!!!! Don't create and print sip headers");
		PASSERT(headers.numOfHeaders > 1000);
		return;
	}

	m_pHeadersArr = new CSipHeader*[headers.numOfHeaders];

	// first header
	const char * pCurHeader = headers.headersList + headers.numOfHeaders * sizeof(sipHeaderElement);

	sipHeaderElement * pCurElement  = (sipHeaderElement *)headers.headersList;
	sipHeaderElement * pNextElement = (headers.numOfHeaders > 1)? pCurElement+1: NULL;

	int endPosition   = headers.headersListLength - headers.numOfHeaders * sizeof(sipHeaderElement);
	int nextPosition  = pNextElement? pNextElement->position: endPosition;

	int curStrLen	  = (nextPosition - pCurElement->position) -1; //without the string null terminator


	for(int i=0; i<headers.numOfHeaders; i++)
	{
		if(curStrLen < 0 || curStrLen > 1000)
		{// MFW Core FSN-671, BRIDGE-18254
			PTRACE(eLevelError, "if(curStrLen < 0 || curStrLen > 1000)");
			PASSERT(curStrLen);
			break;
		}

		m_pHeadersArr[i] =
			new CSipHeader((enHeaderField)pCurElement->eHeaderField,0,curStrLen,pCurHeader);

		// move to the next header
		pCurHeader  += curStrLen+1;
		pCurElement += 1;
		pNextElement = (i+2 < headers.numOfHeaders)? (pNextElement+1) :NULL;
		nextPosition = pNextElement? pNextElement->position: endPosition;
		curStrLen = (nextPosition - pCurElement->position) -1; //without the string null terminator
	}
}

//////////////////////////////////////////////////////////////////////////////////////
CSipHeaderList::CSipHeaderList(int maxAlloc,int numOfElem,.../*params format: numOfElem X (enHeaderField eField, int strLen,char * str)*/)
{

	m_numOfElem     = numOfElem;
	m_maxAlloc		= max(maxAlloc,numOfElem);

	if (m_maxAlloc > 0)
	{
		m_pHeadersArr = new CSipHeader*[m_maxAlloc];

		va_list			params;
		va_start(params, numOfElem);     // Initialize variable arguments.

		for (int i=0; i<numOfElem; i++)
		{
			int				eField  = va_arg(params,int);
			BYTE			flags	= 0;
			int				strLen  = va_arg(params, int);
			const char *	str		= va_arg(params, const char *);

			m_pHeadersArr[i] = new CSipHeader((enHeaderField)eField,flags,strLen,str);
		}

		va_end(params);               // Reset variable arguments.
	}
	else
		m_pHeadersArr = NULL;
}
//////////////////////////////////////////////////////////////////////////////////////
void CSipHeaderList::Serialize(WORD format,CSegment& seg) const
{
    switch (format)
    {
    case NATIVE:
		{
			seg << (WORD)m_maxAlloc;
			seg << (WORD)m_numOfElem;
			for (int i=0; i<m_numOfElem; i++)
				m_pHeadersArr[i]->Serialize(format,seg);
		}
        break;

    default :
        break;
    }

}
//////////////////////////////////////////////////////////////////////////////////////
void CSipHeaderList::DeSerialize(WORD format,CSegment& seg)
{
    switch (format)
    {
    case NATIVE:
		{
			WORD maxAlloc;
			WORD numOfElem;
			seg >> maxAlloc;
			seg >> numOfElem;
			if (numOfElem >= maxAlloc)
				maxAlloc = numOfElem*2;
			CleanArr();
			m_maxAlloc	= maxAlloc;
			m_numOfElem = numOfElem;
			m_pHeadersArr = new CSipHeader*[m_maxAlloc];
			for (int i=0; i<m_numOfElem; i++)
			{
				m_pHeadersArr[i] = new CSipHeader;
				m_pHeadersArr[i]->DeSerialize(format,seg);
			}
		}
        break;
    default :
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////
void CSipHeaderList::AddHeader(const CSipHeader & header)
{
	ReallocIfNeeded();
	m_pHeadersArr[m_numOfElem] = new CSipHeader(header);
	m_pHeadersArr[m_numOfElem]->m_flags = 0;
	m_numOfElem++;
}
//////////////////////////////////////////////////////////////////////////////////////
void CSipHeaderList::AddHeaders(const CSipHeaderList& headers)
{
	for(int i=0; i<headers.GetNumOfElem(); i++)
		AddHeader(*headers[i]);
}
//////////////////////////////////////////////////////////////////////////////////////
void CSipHeaderList::AddHeader(enHeaderField eField,int strLen,const char * str)
{
	ReallocIfNeeded();
	m_pHeadersArr[m_numOfElem] = new CSipHeader(eField,0,strLen,str);
	m_numOfElem++;
}

//////////////////////////////////////////////////////////////////////////////////////
void CSipHeaderList::ReallocIfNeeded()
{
	if (m_maxAlloc == m_numOfElem)
	{
		m_maxAlloc *= 2;
		CSipHeader ** pTempHeadersArr = new CSipHeader*[m_maxAlloc];
		memcpy(pTempHeadersArr,m_pHeadersArr,m_numOfElem*sizeof(CSipHeader*));
		PDELETEA(m_pHeadersArr);
		m_pHeadersArr = pTempHeadersArr;
	}
}
//////////////////////////////////////////////////////////////////////////////////////
void CSipHeaderList::CleanArr()
{
	for (int i=0; i<m_numOfElem; i++)
		POBJDELETE(m_pHeadersArr[i]);
	m_numOfElem = 0;
	PDELETEA(m_pHeadersArr);
	m_maxAlloc = 0;
}

//////////////////////////////////////////////////////////////////////////////////////
int CSipHeaderList::GetTotalLen() const
{
	int len = sizeof(sipMessageHeadersBase);
	len += (m_numOfElem * sizeof(sipHeaderElement));

	for(int i=0; i<m_numOfElem; i++)
		len += (m_pHeadersArr[i]->m_strLen + 1);

	return len;
}
//////////////////////////////////////////////////////////////////////////////////////
const CSipHeader* CSipHeaderList::operator[] (int i) const
{
	const CSipHeader* res = (i<m_numOfElem)? m_pHeadersArr[i]: NULL;
	return res;
}
//////////////////////////////////////////////////////////////////////////////////////
void CSipHeaderList::ZeroingFlags()
{
	for(int i=0; i<m_numOfElem; i++)
		m_pHeadersArr[i]->m_flags = NO;
}

//////////////////////////////////////////////////////////////////////////////////////
const CSipHeader* CSipHeaderList::GetNextHeader(enHeaderField eField)
{
	int index = GetNextHeaderIndex(eField);
	const CSipHeader* res = (index != -1)? m_pHeadersArr[index]: NULL;
	return res;
}

//_mccf_
//////////////////////////////////////////////////////////////////////////////////////
bool CSipHeaderList::RemoveNextHeader(enHeaderField eField)
{
	int index = GetNextHeaderIndex(eField);

	if(index == -1)
		return false;

	CSipHeader* pHeader = (index != -1)? m_pHeadersArr[index]: NULL;

	if(!pHeader)
		return false;

	CSipHeader* newHeader = new CSipHeader();
	*pHeader = *newHeader;
	POBJDELETE(newHeader);
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////
int CSipHeaderList::GetNextHeaderIndex(enHeaderField eField)
{
	int res = -1;
	for(int i=0; i<m_numOfElem && res == -1; i++)
	{
		if (m_pHeadersArr[i]->m_eField == eField && m_pHeadersArr[i]->m_flags == NO)
		{
			m_pHeadersArr[i]->m_flags = YES;
			res = i;
		}
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////////////
const CSipHeader* CSipHeaderList::GetNextPrivateOrProprietyHeader(enHeaderField eField,int strLen,const char* strHeaderName)
{
	const CSipHeader* res = NULL;
	for(int i=0; i<m_numOfElem && res == NULL; i++)
	{
		if (m_pHeadersArr[i]->m_eField == eField && m_pHeadersArr[i]->m_flags == NO)
		{
			const char* strHeader = m_pHeadersArr[i]->GetHeaderStr();
			if (strncmp(strHeader,strHeaderName,strLen) == 0 && strHeader[strLen] == ':')
			{
				m_pHeadersArr[i]->m_flags = YES;
				res = m_pHeadersArr[i];
			}
		}
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////
int CSipHeaderList::BuildMessage(sipMessageHeaders * pMsg) const
{
	int res = sizeof(sipMessageHeadersBase);
	pMsg->numOfHeaders = m_numOfElem;
	pMsg->headersListLength = m_numOfElem * sizeof(sipHeaderElement);

	char				* pCurHeader	= pMsg->headersList + m_numOfElem * sizeof(sipHeaderElement);
	sipHeaderElement	* pCurElement	= (sipHeaderElement *)pMsg->headersList;
	int					  curPos		= 0;

	for(int i=0; i<m_numOfElem; i++)
	{
		pCurElement->eHeaderField	= m_pHeadersArr[i]->m_eField;
		pCurElement->flags			= 0;
		pCurElement->position		= curPos;

		memcpy(pCurHeader,m_pHeadersArr[i]->m_strHeader,m_pHeadersArr[i]->m_strLen);
		pCurHeader[m_pHeadersArr[i]->m_strLen] = 0; //null terminator

		// set the next header
		pCurHeader  += (m_pHeadersArr[i]->m_strLen + 1);
		pCurElement += 1;
		curPos		+= (m_pHeadersArr[i]->m_strLen + 1);
		pMsg->headersListLength += (m_pHeadersArr[i]->m_strLen + 1);

		res += ((sizeof(sipHeaderElement) + (m_pHeadersArr[i]->m_strLen + 1)));
	}
	return res;
}
//////////////////////////////////////////////////////////////////////////////////////
CSipHeaderList& CSipHeaderList::operator=(const CSipHeaderList & other)
{
	if(this != &other)
	{
		CleanArr();
		m_maxAlloc = other.m_maxAlloc;
		m_pHeadersArr = new CSipHeader*[other.m_maxAlloc];
		m_numOfElem = other.m_numOfElem;
		for(int i=0; i<other.m_numOfElem; i++)
		{
			const CSipHeader * pCur = other.m_pHeadersArr[i];
			m_pHeadersArr[i] =
				new CSipHeader(pCur->m_eField,pCur->m_flags,pCur->m_strLen,pCur->m_strHeader);
		}
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////////
void CSipHeaderList::Dump(COstrStream& msg) const
{
	for(int i=0; i<m_numOfElem; i++)
	{
		msg << GetHeaderFieldStr(m_pHeadersArr[i]->m_eField) << ": ";
		msg << m_pHeadersArr[i]->m_strHeader << "\n";
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void CSipHeaderList::DumpToStream(CObjString* pMsgStr) const
{
	for(int i=0; i<m_numOfElem; i++)
	{
		*pMsgStr << GetHeaderFieldStr(m_pHeadersArr[i]->m_eField) << ": ";
		if (m_pHeadersArr[i]->m_strHeader && strcmp(m_pHeadersArr[i]->m_strHeader, ""))
			*pMsgStr << m_pHeadersArr[i]->m_strHeader;
		*pMsgStr << "\n";
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////
CSipBaseStruct::CSipBaseStruct()
{
	m_pHeaders	= new CSipHeaderList(MIN_ALLOC_HEADERS,0);
	m_buffer	= NULL;
	m_bufferLen = 0;
}

///////////////////////////////////////////////////////////////////////////////////////
CSipBaseStruct::~CSipBaseStruct()
{
	POBJDELETE(m_pHeaders);
	PDELETEA(m_buffer);
}



///////////////////////////////////////////////////////////////////////////////////////
//serialize m_pHeaders and m_pHeaders into m_buffer
void CSipBaseStruct::Serialize()
{
	PDELETEA(m_buffer);
	int len = m_pHeaders? m_pHeaders->GetTotalLen(): 0;
	if (len)
	{
		m_buffer	= new BYTE[len];
		m_bufferLen	= len;
		m_pHeaders->BuildMessage((sipMessageHeaders*)m_buffer);
	}
}


///////////////////////////////////////////////////////////////////////////////////////
//Deserialize m_pHeaders from m_buffer
void CSipBaseStruct::DeSerialize()
{
	POBJDELETE(m_pHeaders);
	if (m_buffer)
		m_pHeaders = new CSipHeaderList(*(sipMessageHeaders*)m_buffer);
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipBaseStruct::SetHeaderField(enHeaderField eField,const char* headerStr)
{
	if (m_pHeaders && headerStr)
		m_pHeaders->AddHeader(eField,strlen(headerStr),headerStr);
}

///////////////////////////////////////////////////////////////////////////////////////
const char* CSipBaseStruct::GetHeaderField(enHeaderField eField,int index) const
{
	const CSipHeader*	pHeader			= NULL;
	const char*			headerStr		= "";
	int numOfHeaders = GetNumOfHeaders();

	m_pHeaders->ZeroingFlags(); // start from the begining

	for (int i=0;i<=index && i<numOfHeaders;i++)
	{
		pHeader		= m_pHeaders->GetNextHeader(eField);
		headerStr	= pHeader? pHeader->GetHeaderStr(): "";
	}


	return headerStr;
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL CSipBaseStruct::FindHeaderField(enHeaderField eField, const char* headerStr, BOOL fStart)
{
	const CSipHeader* pHeader = NULL;
	int numOfHeaders = GetNumOfHeaders();

	if (fStart)
		m_pHeaders->ZeroingFlags();

	for (int i = 0; i < numOfHeaders; i++) {
		pHeader = m_pHeaders->GetNextHeader(eField);
		if (pHeader && headerStr && !strcmp(pHeader->GetHeaderStr(), headerStr))
			return TRUE;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
void  CSipBaseStruct::Dump(COstrStream& msg) const
{
	msg << "Headers:\n";
	msg << "----------------------\n";
	m_pHeaders->Dump(msg);
}

///////////////////////////////////////////////////////////////////////////////////////
//						CSipInviteStruct
///////////////////////////////////////////////////////////////////////////////////////
CSipInviteStruct::CSipInviteStruct()
{
	m_callIndex = 0;
	m_csServiceId = 0xFFFFFFFF;
	m_pSdpAndHeaders = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////
CSipInviteStruct::~CSipInviteStruct()
{
	PDELETEA(m_pSdpAndHeaders);
}


///////////////////////////////////////////////////////////////////////////////////////
void CSipInviteStruct::Dump(COstrStream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	msg << "CSipInviteStruct::Dump\n";
	msg << "----------------------\n";
	msg << "CallIndex:   " << m_callIndex << "\n";
	msg << "CsServiceId: " << m_csServiceId << "\n";
	CSipBaseStruct::Dump(msg);
	//PTRACE(eLevelInfoNormal,msg.str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipInviteStruct::ReadInviteInd(mcIndInvite * pMsg)
{
	BYTE * pSipCaps = (BYTE *)pMsg->sipSdpAndHeaders.capsAndHeaders;
	sipMessageHeaders * pSipHeaders = (sipMessageHeaders *)(pSipCaps + pMsg->sipSdpAndHeaders.sipHeadersOffset);

	//FSN-671 MFW-Core, BRIDGE-18254
	if((*pSipHeaders).numOfHeaders>100){
		FPTRACE(eLevelError, "CSipInviteStruct::ReadInviteInd headers.numOfHeaders > 100 !!!!!!!!!!!!!!! Don't create and print sip headers");
		FPASSERT((*pSipHeaders).numOfHeaders);
		return;
	}

	PDELETEA(m_pSdpAndHeaders);
	int length = sizeof(sipSdpAndHeadersBaseSt) + pMsg->sipSdpAndHeaders.lenOfDynamicSection;
	m_pSdpAndHeaders = (sipSdpAndHeadersSt *)new BYTE[length];
	memset(m_pSdpAndHeaders, 0, length);
	memcpy(m_pSdpAndHeaders, &pMsg->sipSdpAndHeaders, length);
	
	PDELETEA(m_buffer);
	m_bufferLen = sizeof(sipMessageHeadersBase) + pSipHeaders->headersListLength;
	m_buffer = new BYTE[m_bufferLen];
	memcpy(m_buffer,(BYTE*)pSipHeaders,m_bufferLen);

	//Deserialize m_pHeaders from m_buffer
	DeSerialize();
}

///////////////////////////////////////////////////////////////////////////////////////
//						CSipBeNotifyStruct
///////////////////////////////////////////////////////////////////////////////////////
void CSipBeNotifyStruct::Dump(COstrStream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	msg << "CSipBeNotifyStruct::Dump";
	msg << "\n----------------------\n";
	CSipBaseStruct::Dump(msg);
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipBeNotifyStruct::ReadHeaders(BYTE* buffer, int bufferLen)
{
	PDELETEA(m_buffer);
	m_bufferLen = bufferLen;
	m_buffer = new BYTE[bufferLen];
	memcpy(m_buffer, buffer, m_bufferLen);

	//Deserialize m_pHeaders from m_buffer
	DeSerialize();
}

///////////////////////////////////////////////////////////////////////////////////////
//			CSipRegisterStruct
///////////////////////////////////////////////////////////////////////////////////////
CSipRegisterStruct::CSipRegisterStruct():m_expire(3600),m_id(0xFFFFFFFF),m_subOpcode(GeneralRegisterNull)
{
}

///////////////////////////////////////////////////////////////////////////////////////
CSipRegisterStruct::~CSipRegisterStruct()
{
}

///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
void CSipRegisterStruct::Dump(COstrStream& msg) const
{
	if(msg){
		msg << "CSipRegisterStruct::Dump\n";
		msg << "----------------------\n";
		CSipBaseStruct::Dump(msg);
		if(m_proxyAddress.ipVersion == eIpVersion4)
		{
			char proxyIp[16];
			SystemDWORDToIpString(m_proxyAddress.addr.v4.ip, proxyIp);
			msg << "ProxyIpV4 = " << proxyIp << "\n";
		}
		else if(m_proxyAddress.ipVersion == eIpVersion6)
		{
			char strIp[128];
			ipV6ToString(m_proxyAddress.addr.v6.ip, strIp, TRUE);
			FTRACESTR(eLevelInfoNormal) << "CSipRegisterStruct::Dump ProxyIpV6 = " << strIp;
			msg << "ProxyIpV6 = " << strIp << "\n";
		}

		msg << "Proxy Port = " << m_proxyAddress.port << "\n";

		if(m_registrarAddress.ipVersion == eIpVersion4)
		{
			char registrarIp[16];
			SystemDWORDToIpString(m_registrarAddress.addr.v4.ip, registrarIp);
			msg << "RegistrarIpV4 = " << registrarIp << "\n";
		}
		else if(m_registrarAddress.ipVersion == eIpVersion6)
		{
			char registrarIpV6[128];
			ipV6ToString(m_registrarAddress.addr.v6.ip, registrarIpV6, TRUE);

			msg << "RegistrarIpV6 = " << registrarIpV6 << "\n";
		}
		msg << "Registrar Port = " << m_registrarAddress.port << "\n";
		msg << "Expires = " <<m_expire << "\n";
		msg << "Id = " << m_id << "\n";
		msg << "subOpcode = " << m_subOpcode << "\n";
//		PTRACE(eLevelInfoNormal,msg.str());
	}
}
///////////////////////////////////////////////////////////////////////////////////////
mcReqRegister* CSipRegisterStruct::BuildRegister()
{
	mcReqRegister * pMsg = NULL;

	//serialize m_pHeaderFields and m_pHeaders into m_buffer
	Serialize();

	if(m_buffer && m_bufferLen)
	{
		int totalLength	= sizeof(mcReqRegisterBase) - sizeof(sipMessageHeadersBase) + m_bufferLen;

		pMsg = (mcReqRegister *)new BYTE[totalLength];
		sipMessageHeaders* pSipHeaders = &(pMsg->sipHeaders);
		memcpy(pSipHeaders,m_buffer,m_bufferLen);
		pMsg->expires	= m_expire;
		pMsg->id		= m_id;
		pMsg->subOpcode = m_subOpcode;

		if(m_registrarAddress.ipVersion == eIpVersion4)
			pMsg->registrarTransportAddr.transAddr.addr.v4.ip = m_registrarAddress.addr.v4.ip;
		else if(m_registrarAddress.ipVersion == eIpVersion6)
		{
			memcpy(pMsg->registrarTransportAddr.transAddr.addr.v6.ip, m_registrarAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
			pMsg->registrarTransportAddr.transAddr.addr.v6.scopeId = m_registrarAddress.addr.v6.scopeId;
		}
		pMsg->registrarTransportAddr.transAddr.port = m_registrarAddress.port;
		pMsg->registrarTransportAddr.transAddr.transportType = m_transportType;
		pMsg->registrarTransportAddr.transAddr.ipVersion = m_registrarAddress.ipVersion;//eIpVersion4;
		pMsg->registrarTransportAddr.transAddr.distribution = eDistributionUnicast;
		pMsg->registrarTransportAddr.unionProps.unionType = m_registrarAddress.ipVersion;//eIpVersion4;
//		pMsg->registrarTransportAddr.unionProps.unionType = eIpVersion6;//eIpVersion4;
		pMsg->registrarTransportAddr.unionProps.unionSize = 0x14;
//		FTRACESTR(eLevelInfoNormal) << "CSipRegisterStruct::BuildRegister registrarTransportAddr unionSize = " << pMsg->registrarTransportAddr.unionProps.unionSize;

//		pMsg->registrarTransportAddr.unionProps.unionSize = 0x14;

		pMsg->proxyTransportAddr.transAddr.port = m_proxyAddress.port;
		if(m_proxyAddress.ipVersion == eIpVersion4)
			pMsg->proxyTransportAddr.transAddr.addr.v4.ip = m_proxyAddress.addr.v4.ip;
		else if(m_proxyAddress.ipVersion == eIpVersion6)
		{
			memcpy(pMsg->proxyTransportAddr.transAddr.addr.v6.ip, m_proxyAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
			pMsg->proxyTransportAddr.transAddr.addr.v6.scopeId = m_proxyAddress.addr.v6.scopeId;
		}
//        if (m_proxyPort)
//            pMsg->proxyTransportAddr.transAddr.port = m_proxyPort;
//        else
//            pMsg->proxyTransportAddr.transAddr.port = m_registrarPort;
//
//        if (m_proxyIp)
//            pMsg->proxyTransportAddr.transAddr.addr.v4.ip = m_proxyIp;
//        else
//            pMsg->proxyTransportAddr.transAddr.addr.v4.ip = m_registrarIp;

        // TODO #error("missing code for ipv6")

		pMsg->proxyTransportAddr.transAddr.transportType= m_transportType;
		pMsg->proxyTransportAddr.transAddr.ipVersion = m_proxyAddress.ipVersion;//eIpVersion4;
		pMsg->proxyTransportAddr.transAddr.distribution = eDistributionUnicast;
		pMsg->proxyTransportAddr.unionProps.unionType = m_proxyAddress.ipVersion;//eIpVersion4;
	//	pMsg->proxyTransportAddr.unionProps.unionType = eIpVersion6;//eIpVersion4;
//		pMsg->proxyTransportAddr.unionProps.unionSize = sizeof(ipAddressIf);
		pMsg->proxyTransportAddr.unionProps.unionSize = 0x14;
//		pMsg->proxyTransportAddr.unionProps.unionSize = 0x14;
	}
	return pMsg;
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipRegisterStruct::ReadRegister(mcIndRegisterResp *pMsg)
{
	m_expire = pMsg->expires;
	m_id = pMsg->id;

	//Deserialize m_pHeaders from m_buffer
	DeSerialize();
}

///////////////////////////////////////////////////////////////////////////////////////
int CSipRegisterStruct::AddContact(char* contact, char* contactDisplay)
{
	int res = 0;
	if(contact && contactDisplay)
	{
		SetHeaderField(kContactDisplay,contactDisplay);
		SetHeaderField(kContact,contact);
		res = 1;
	}
	return res;

}

///////////////////////////////////////////////////////////////////////////////////////////////
// CSipSubscribeStruct
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
CSipSubscribeStruct::CSipSubscribeStruct()
{
	m_pEvent = new char[H243_NAME_LEN];
	m_pAccept = new char[H243_NAME_LEN];
	m_pSupported = new char[H243_NAME_LEN];
	m_expires = 0;
	m_Id = 0;

	//m_proxyIp = 0;
	m_proxyIp.v4.ip = 0;
	m_proxyIp.v6.scopeId = 0;
	memset(m_proxyIp.v6.ip, 0, IPV6_ADDRESS_BYTES_LEN);
	m_proxyIpVersion = eIpVersion4;

	m_proxyPort = 0;
	m_transportType = 0;
	m_subOpcode = GeneralSubscribeNull;
}

///////////////////////////////////////////////////////////////////////
CSipSubscribeStruct::~CSipSubscribeStruct()
{
	DEALLOCBUFFER(m_pEvent);
	DEALLOCBUFFER(m_pAccept);
	DEALLOCBUFFER(m_pSupported);
}

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::Dump(COstrStream& msg) const
{
	msg << "CSipSubscribeStruct::Dump\n";
	msg << "-------------------------\n";
	CSipBaseStruct::Dump(msg);
	msg << "Id:\t " << m_Id << "\n";
	msg << "Expires:\t " << m_expires << "\n";
	msg << "Sub opcode:\t " << m_subOpcode << "\n";

	if (m_proxyIpVersion == eIpVersion4)
		msg << "MS_IPV6: Proxy IPV4 = " << m_proxyIp.v4.ip << "\n";
	else //eIpVersion6
		msg << "MS_IPV6: Proxy IPV6 = " << m_proxyIp.v6.ip << "\n";

	msg << "Proxy Port = " << m_proxyPort << "\n";
	//	PTRACE(eLevelInfoNormal,msg.str());
}

///////////////////////////////////////////////////////////////////////
mcReqSubscribe* CSipSubscribeStruct::BuildSubscribeReq()
{
	mcReqSubscribe * pMsg = NULL;

	//serialize m_pHeaderFields and m_pHeaders into m_buffer
	Serialize();

	if(m_buffer && m_bufferLen)
	{
		int totalLength	= sizeof(mcReqSubscribeBase) - sizeof(sipMessageHeadersBase) + m_bufferLen;

		pMsg = (mcReqSubscribe *)new BYTE[totalLength];
		memset(pMsg,0,totalLength);
		sipMessageHeaders* pSipHeaders = &(pMsg->sipHeaders);
		memcpy(pSipHeaders,m_buffer,m_bufferLen);
		pMsg->expires = m_expires;
		pMsg->id = m_Id;
		pMsg->subOpcode = m_subOpcode;

		if (m_proxyIpVersion == eIpVersion4)
		{
            TRACEINTO << "MS_IPV6: eIpVersion4";
			pMsg->transportAddr.transAddr.addr.v4.ip = m_proxyIp.v4.ip;
			pMsg->transportAddr.transAddr.ipVersion = eIpVersion4;
		}
		else
		{
			TRACEINTO << "MS_IPV6: eIpVersion6";
			memcpy(pMsg->transportAddr.transAddr.addr.v6.ip, m_proxyIp.v6.ip, IPV6_ADDRESS_BYTES_LEN);
			pMsg->transportAddr.transAddr.ipVersion = eIpVersion6;
		}

		pMsg->transportAddr.transAddr.port = m_proxyPort;

		pMsg->transportAddr.transAddr.transportType = m_transportType;
		pMsg->transportAddr.transAddr.distribution = eDistributionUnicast;
		pMsg->transportAddr.unionProps.unionType = eIpVersion4;
		pMsg->transportAddr.unionProps.unionSize = sizeof(ipAddressIf);
	}
	return pMsg;
}
///////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::SetProxyIpVersion(enIpVersion proxyIpVersion)
{
	m_proxyIpVersion = proxyIpVersion;
}
///////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::SetProxyIpV4(DWORD proxyIpV4)
{
	m_proxyIp.v4.ip = proxyIpV4;
}

///////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::SetProxyIpV6(char* pProxyIpV6)
{
	memcpy(m_proxyIp.v6.ip, pProxyIpV6, IPV6_ADDRESS_BYTES_LEN);
}

///////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::SetProxyPort(WORD port)
{
	m_proxyPort = port;
}

///////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::SetTransportType(WORD transportType)
{
	m_transportType = transportType;
}

///////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::SetEvent(char* pEvent)
{
	if(pEvent)
		strncpy(m_pEvent, pEvent, H243_NAME_LEN);
}

///////////////////////////////////////////////////////////////////////
char* CSipSubscribeStruct::GetEvent() const
{
	return m_pEvent;
}

///////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::SetAccept(char* pAccept)
{
	if(pAccept)
		strncpy(m_pAccept, pAccept, H243_NAME_LEN);
}

///////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::SetSupported(char* pSupported)
{
	if(pSupported)
		strncpy(m_pSupported, pSupported, H243_NAME_LEN);
}

///////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::SetExpires(DWORD expires)
{
	m_expires = expires;
}

///////////////////////////////////////////////////////////////////////
DWORD CSipSubscribeStruct::GetExpires()
{
	return m_expires;
}

///////////////////////////////////////////////////////////////////////////////////////
DWORD CSipSubscribeStruct::GetId() const
{
	return m_Id;
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::SetId(DWORD id)
{
	m_Id = id;
}
///////////////////////////////////////////////////////////////////////////////////////
WORD CSipSubscribeStruct::GetSubOpcode() const
{
	return m_subOpcode;
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipSubscribeStruct::SetSubOpcode(WORD subOpcode)
{
	m_subOpcode = subOpcode;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// CSipUnknownMsgStruct
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
CSipUnknownMsgStruct::CSipUnknownMsgStruct()
{
//	m_registrarIp = 0;
//	m_registrarPort = 0;

	m_registrarAddress.addr.v4.ip = 0;
//	m_registrarAddress.addr.v6.ip = NULL;
	memset(m_registrarAddress.addr.v6.ip, 0, IPV6_ADDRESS_BYTES_LEN);
	m_registrarAddress.addr.v6.scopeId = 0;
	m_method = 0xFF; // MESSAGE/ SERVICE
	m_pContentType = new char[H243_NAME_LEN];
	m_pContent = NULL;
}

///////////////////////////////////////////////////////////////////////
CSipUnknownMsgStruct::~CSipUnknownMsgStruct()
{
	DEALLOCBUFFER(m_pContentType);
	DEALLOCBUFFER(m_pContent);
}

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
void CSipUnknownMsgStruct::Dump(COstrStream& msg) const
{
	//CLanCfg*  pLanCfg = new CLanCfg;
	char* registrarIp= NULL; //pLanCfg->TranslDwordToString(m_registrarIp);
	msg << "CSipUnknownMsgStruct::Dump\n";
	msg << "--------------------------\n";
	if(m_method == SIP_MESSAGE_METHOD)
		msg << "MESSAGE\t ";
	else
		msg << "SERVICE\t ";
	msg << registrarIp <<"\n";
	CSipBaseStruct::Dump(msg);
	msg << "ContentType:\t " << m_pContentType << "\n";
	msg << m_pContent << "\n";
	//	PTRACE(eLevelInfoNormal,msg.str());

//	PDELETE(pLanCfg);
	PDELETEA(registrarIp);
}

///////////////////////////////////////////////////////////////////////
mcReqUnknownMethod* CSipUnknownMsgStruct::BuildUnknownMsgReq()
{
	int i=0;
	mcReqUnknownMethod* pMsg = new mcReqUnknownMethod;

	int totalLength	= sizeof(mcReqUnknownMethod);

	//pMsg = (mcReqUnknownMethod *)new char[totalLength];
	switch(m_method)
	{
		case(SIP_SERVICE_METHOD):
		{
			strncpy(pMsg->methodName, "SERVICE", 8);
			break;
		}
		case(SIP_MESSAGE_METHOD):
		{
			strncpy(pMsg->methodName, "MESSAGE", 8);
			break;
		}
		default:
			PASSERT(1);
	}

	strncpy(pMsg->localAddress, GetHeaderField(kFrom), MaxLengthOfSingleUrl - 1);
	pMsg -> localAddress[MaxLengthOfSingleUrl - 1] = 0;

	if (m_registrarAddress.ipVersion == eIpVersion4)
		pMsg->remoteTransportAddress.transAddr.addr.v4.ip = m_registrarAddress.addr.v4.ip;
	else if(m_registrarAddress.ipVersion == eIpVersion6)
	{
		memcpy(pMsg->remoteTransportAddress.transAddr.addr.v6.ip, m_registrarAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
		pMsg->remoteTransportAddress.transAddr.addr.v6.scopeId = m_registrarAddress.addr.v6.scopeId;
	}
	pMsg->remoteTransportAddress.transAddr.port = m_registrarAddress.port;
	pMsg->remoteTransportAddress.transAddr.transportType = m_transportType;
	pMsg->remoteTransportAddress.transAddr.ipVersion = m_registrarAddress.ipVersion;//eIpVersion4;
	pMsg->remoteTransportAddress.transAddr.distribution = eDistributionUnicast;
	pMsg->remoteTransportAddress.unionProps.unionType = m_registrarAddress.ipVersion;//eIpVersion4;
	pMsg->remoteTransportAddress.unionProps.unionSize = sizeof(ipAddressIf);
	strncpy(pMsg->contentType, m_pContentType, MaxLengthOfNonSdpContentType-1);
	strncpy(pMsg->content, m_pContent, MaxLengthOfNonSdpContent-1);

	return pMsg;
}

///////////////////////////////////////////////////////////////////////
void CSipUnknownMsgStruct::SetMethod(BYTE bMethod)
{
	if((bMethod == SIP_SERVICE_METHOD) || (bMethod == SIP_MESSAGE_METHOD))
		m_method = bMethod;
}

///////////////////////////////////////////////////////////////////////
void CSipUnknownMsgStruct::SetTransportType(WORD transportType)
{
	m_transportType = transportType;
}

///////////////////////////////////////////////////////////////////////
void CSipUnknownMsgStruct::SetContentType(char* pType)
{
	if(pType)
		strncpy(m_pContentType, pType, H243_NAME_LEN);
}

///////////////////////////////////////////////////////////////////////
void CSipUnknownMsgStruct::SetContent(char* pContent)
{
	if(pContent)
	{
		int len = strlen(pContent);
		if(m_pContent)
			PDELETEA(m_pContent);
		m_pContent = new char[len+1];
		strncpy(m_pContent, pContent, len);
		m_pContent[len] = '\0';
	}
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipUnknownMsgStruct::SetRegistrarIpV4(DWORD registrarIpV4)
{
	m_registrarAddress.addr.v4.ip = registrarIpV4;
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipUnknownMsgStruct::SetRegistrarIpV6(char* registrarIpV6)
{
	memcpy(m_registrarAddress.addr.v6.ip, registrarIpV6, IPV6_ADDRESS_BYTES_LEN );
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipUnknownMsgStruct::SetRegistrarV6scopeId(DWORD scopeId)
{
	m_registrarAddress.addr.v6.scopeId = scopeId;
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipUnknownMsgStruct::SetRegistrarPort(WORD port)
{
	m_registrarAddress.port = port;
}
///////////////////////////////////////////////////////////////////////////////////////
void CSipUnknownMsgStruct::SetRegistrarIpVersion(DWORD ipVersion)
{
	m_registrarAddress.ipVersion = ipVersion;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// CSipNotifyStruct
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
CSipNotifyStruct::CSipNotifyStruct()
{
	m_ip = 0;
	m_port = 0;
	m_transportType = 0;
	m_pContentType = new char[H243_NAME_LEN];
	memset(m_pContentType, 0, H243_NAME_LEN);
	m_pContent = NULL;
	m_CSeq = 0;
	m_CallIndex = 0;
}

///////////////////////////////////////////////////////////////////////
CSipNotifyStruct::~CSipNotifyStruct()
{
	DEALLOCBUFFER(m_pContentType);
	DEALLOCBUFFER(m_pContent);
}

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
void CSipNotifyStruct::Dump(COstrStream& msg) const
{
	char Ip[16];
	SystemDWORDToIpString(m_ip, Ip);

	msg << "CSipNotifyStruct::Dump\n";
	msg << "--------------------------\n";
	msg << "CSeq= " <<m_CSeq <<"\n";
	msg << "CallIndex=" << m_CallIndex << "\n";
	msg << "ContentType:\t " << m_pContentType << "\n";
	msg << m_pContent << "\n";
	msg << Ip <<"\n";
	CSipBaseStruct::Dump(msg);
}
/*
///////////////////////////////////////////////////////////////////////
mcReqNotify* CSipNotifyStruct::BuildNotifyReq()
{
	mcReqNotify * pMsg = NULL;

	//serialize m_pHeaderFields and m_pHeaders into m_buffer
	Serialize();

	if(m_buffer && m_bufferLen)
	{
		int totalLength	= sizeof(mcReqNotifyBase) - sizeof(sipMessageHeadersBase) + m_bufferLen;


		pMsg = (mcReqNotify *)new BYTE[totalLength];
		sipMessageHeaders * pSipHeaders = &(pMsg->sipHeaders);
		memcpy(pSipHeaders,m_buffer,m_bufferLen);
		pMsg->transportAddress.transAddr.addr.v4.ip = m_ip;
		pMsg->transportAddress.transAddr.port = m_port;
		pMsg->transportAddress.transAddr.transportType = m_transportType;
		pMsg->transportAddress.transAddr.ipVersion = eIpVersion4;
		pMsg->transportAddress.transAddr.distribution = eDistributionUnicast;
		pMsg->transportAddress.unionProps.unionType = eIpVersion4;
		pMsg->transportAddress.unionProps.unionSize = sizeof(ipAddress);
		*pMsg->contentType = '\0';
		* pMsg->content = '\0';
		strncpy(pMsg->contentType, m_pContentType, MaxLengthOfNonSdpContentType-1);
		strncpy(pMsg->content, m_pContent, MaxLengthOfNonSdpContent-1);
		pMsg->cseq = m_CSeq;
//		pMsg->header.pmHeader.callIndex = m_CallIndex;
	}


	return pMsg;
}
*/

///////////////////////////////////////////////////////////////////////
mcReqNotify* CSipNotifyStruct::BuildNotifyReq()
{
	mcReqNotify * pMsg = NULL;

	//serialize m_pHeaderFields and m_pHeaders into m_buffer
	int len = m_pHeaders ? m_pHeaders->GetTotalLen(): 0;
	if (!m_pHeaders)
	{
	    PTRACE(eLevelError, "CSipDistrNotifyStruct::BuildNotifyReq - m_pHeaders is NULL - pMsg will be NULL!");
	    DBGPASSERT(301);
	    return pMsg;
	}

	int contentSize = strlen(m_pContent);
	int contentAndHeadersSize	= len + contentSize;
	int totalSize			= sizeof(mcReqNotifyBase);
	totalSize				+= contentAndHeadersSize;

	pMsg = (mcReqNotify *)new BYTE[totalSize];
	memset(pMsg, 0, totalSize);

	sipContentAndHeadersSt* pContentAndHeaders	= (sipContentAndHeadersSt*) &pMsg->sipContentAndHeaders;
	memcpy((char*) pContentAndHeaders->contentAndHeaders,m_pContent,contentSize);
	//		int contentAndHeadersSize = headersSize + contentSize;
	pContentAndHeaders->lenOfDynamicSection = contentAndHeadersSize;
	pContentAndHeaders->sipHeadersOffset = contentSize;
	sipMessageHeaders* pHeaders = (sipMessageHeaders*) ((char*) pContentAndHeaders->contentAndHeaders + pContentAndHeaders->sipHeadersOffset);

	m_pHeaders->BuildMessage(pHeaders);

	//	}
//	if(m_buffer && m_bufferLen)
//	{

	//	int totalLength	= sizeof(mcReqNotifyBase) - sizeof(sipContentAndHeadersBaseSt) + m_bufferLen;

	//	pMsg = (mcReqNotify *)new BYTE[totalLength];
		//sipMessageHeaders * pSipHeaders = &(pMsg->sipContentAndHeaders);

//		sipContentAndHeadersSt* pContentAndHeaders	= (sipContentAndHeadersSt*) &pMsg->sipContentAndHeaders;
//		sipMessageHeaders * pSipHeaders = (sipMessageHeaders*) ((char*) pContentAndHeaders->contentAndHeaders + pContentAndHeaders->sipHeadersOffset);
//		memcpy(pSipHeaders,m_buffer,m_bufferLen);
		pMsg->transportAddress.transAddr.addr.v4.ip = m_ip;
		pMsg->transportAddress.transAddr.port = m_port;
		pMsg->transportAddress.transAddr.transportType = m_transportType;
		pMsg->transportAddress.transAddr.ipVersion = eIpVersion4;
		pMsg->transportAddress.transAddr.distribution = eDistributionUnicast;
		pMsg->transportAddress.unionProps.unionType = eIpVersion4;
		pMsg->transportAddress.unionProps.unionSize = sizeof(ipAddressIf);
/*
		*pMsg->contentType = '\0';
		* pMsg->content = '\0';
		strncpy(pMsg->contentType, m_pContentType, MaxLengthOfNonSdpContentType-1);
		strncpy(pMsg->content, m_pContent, MaxLengthOfNonSdpContent-1);
*/

		pMsg->cseq = m_CSeq;
//****		pMsg->header.pmHeader.callIndex = m_CallIndex;
//	}

	return pMsg;

}

mcReqNotify* CSipNotifyStruct::BuildNotifyReq(mcXmlTransportAddress& transportAddress)
{
	mcReqNotify* pMsg = NULL;

	//serialize m_pHeaderFields and m_pHeaders into m_buffer
	int len = m_pHeaders ? m_pHeaders->GetTotalLen(): 0;
	if (!m_pHeaders)
	{
	    PTRACE(eLevelError, "CSipDistrNotifyStruct::BuildNotifyReq - m_pHeaders is NULL - pMsg will be NULL!");
	    DBGPASSERT(301);
	    return pMsg;
	}

	int contentSize = strlen(m_pContent);
	int contentAndHeadersSize	= len + contentSize;
	int totalSize			= sizeof(mcReqNotifyBase);
	totalSize				+= contentAndHeadersSize;

	pMsg = (mcReqNotify*)new BYTE[totalSize];
	memset(pMsg, 0, totalSize);

	sipContentAndHeadersSt* pContentAndHeaders	= (sipContentAndHeadersSt*) &pMsg->sipContentAndHeaders;
	memcpy((char*) pContentAndHeaders->contentAndHeaders,m_pContent,contentSize);
	pContentAndHeaders->lenOfDynamicSection = contentAndHeadersSize;
	pContentAndHeaders->sipHeadersOffset = contentSize;
	sipMessageHeaders* pHeaders = (sipMessageHeaders*) ((char*) pContentAndHeaders->contentAndHeaders + pContentAndHeaders->sipHeadersOffset);

	m_pHeaders->BuildMessage(pHeaders);

	pMsg->transportAddress = transportAddress;
	if (pMsg->transportAddress.transAddr.ipVersion == eIpVersion4)
		pMsg->transportAddress.transAddr.addr.v4.ip = htonl(transportAddress.transAddr.addr.v4.ip);

	pMsg->cseq = m_CSeq;

	return pMsg;
}
///////////////////////////////////////////////////////////////////////
void CSipNotifyStruct::SetIp(DWORD ip)
{
	m_ip = ip;
}

///////////////////////////////////////////////////////////////////////
void CSipNotifyStruct::SetTransportType(WORD transportType)
{
	m_transportType = transportType;
}

///////////////////////////////////////////////////////////////////////
void CSipNotifyStruct::SetPort(WORD port)
{
	m_port = port;
}

///////////////////////////////////////////////////////////////////////
void CSipNotifyStruct::SetContentType(char* pType)
{
	if(pType)
		strncpy(m_pContentType, pType, H243_NAME_LEN);
}

///////////////////////////////////////////////////////////////////////
void CSipNotifyStruct::SetContent(const char* pContent)
{
	if(pContent)
	{
		int len = strlen(pContent);
		if(m_pContent)
			PDELETEA(m_pContent);
		m_pContent = new char[len+1];
		strncpy(m_pContent, pContent, len);
		m_pContent[len] = '\0';
	}
}

///////////////////////////////////////////////////////////////////////
void CSipNotifyStruct::SetCSeq(DWORD cseq)
{
	m_CSeq = cseq;
}

///////////////////////////////////////////////////////////////////////
void CSipNotifyStruct::SetCallIndex(DWORD callIndex)
{
	m_CallIndex = callIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// CSipDistrNotifyStruct
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
CSipDistrNotifyStruct::CSipDistrNotifyStruct() : m_unzippedOrgSize(0),m_zipCheckSum(0),m_WatchersCount(0)
{
	m_pContentType = new char[H243_NAME_LEN];
	m_pContent = NULL;
	for(int i=0; i<MAX_WATCHERS_IN_CONF; ++i)
		m_WatchersArray[i] = 0xffff;
}

///////////////////////////////////////////////////////////////////////
CSipDistrNotifyStruct::~CSipDistrNotifyStruct()
{
	DEALLOCBUFFER(m_pContentType);
	DEALLOCBUFFER(m_pContent);
}

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
void CSipDistrNotifyStruct::Dump(COstrStream& msg) const
{
	msg << "CSipDistrNotifyStruct::Dump\n";
	msg << "--------------------------\n";
	msg << "Watchers count: " << m_WatchersCount << "\n";
	for(int i=0; i<m_WatchersCount; i++)
		msg << i << ": " << m_WatchersArray[i] << "\n";
		if(m_unzippedOrgSize)
		{
			msg << "IsCompressed: " << TRUE << "\n";
			msg << "unzipped orginal size: " << m_unzippedOrgSize << "\n";
			msg << "zip checksum: " << m_zipCheckSum << "\n";
		}
		else
			msg << "IsCompressed: " << FALSE << "\n";

	CSipBaseStruct::Dump(msg);
	msg << "ContentType:\t " << m_pContentType << "\n";
	msg << "Content: " << m_pContent << "\n";
//		PTRACE(eLevelInfoNormal,msg.str());
}

///////////////////////////////////////////////////////////////////////
/*mcReqSipCxNotify* CSipDistrNotifyStruct::BuildDistrNotifyReq()
{
	mcReqSipCxNotify * pMsg = NULL;
	int bodyLen = strlen(m_pContent);
	//serialize m_pHeaderFields and m_pHeaders into m_buffer
	Serialize();

	if(m_buffer && m_bufferLen)
	{
		int bodyLen1 = bodyLen;
		//XML data must be aligned
		if(bodyLen%4)
			bodyLen = bodyLen + 4 - bodyLen%4;

		int totalLength	= sizeof(mcReqSipCxNotifyBase) + m_bufferLen + bodyLen;

		pMsg = (mcReqSipCxNotify *)new BYTE[totalLength];
		sipBodyAndHeadersSt * pSipHeaders = &(pMsg->sipBodyAndHeaders);
		pSipHeaders->lenOfDynamicSection = m_bufferLen + bodyLen;
		pSipHeaders->bodyType = kXml;
		pSipHeaders->bodyLength = bodyLen1;
		pSipHeaders->sipHeadersOffset = bodyLen;
		memcpy(pSipHeaders->bodyAndHeaders, m_pContent, bodyLen1);
		memcpy(pSipHeaders->bodyAndHeaders + bodyLen,m_buffer,m_bufferLen);
		pMsg->unzippedOrgSize = m_unzippedOrgSize;
		pMsg->ZipCheckSum = m_zipCheckSum;
		pMsg->NumOfWatcherIds = m_WatchersCount;
		for(int i=0; i<m_WatchersCount; i++)
			pMsg->WatcherIds[i] = m_WatchersArray[i];
		strncpy(pMsg->contentType, m_pContentType, MaxLengthOfNonSdpContentType-1);
	}

	return pMsg;
}
*/
///////////////////////////////////////////////////////////////////////
void CSipDistrNotifyStruct::SetContentUnzippedOrgSize(DWORD length)
{
	m_unzippedOrgSize = length;
}

void CSipDistrNotifyStruct::SetContentZipCheckSum(DWORD length)
{
	m_zipCheckSum = length;
}

///////////////////////////////////////////////////////////////////////
void CSipDistrNotifyStruct::AddWatcherId(WORD watcherId)
{
	if(m_WatchersCount < MAX_WATCHERS_IN_CONF - 1)
	{
		char s[H243_NAME_LEN];
		sprintf(s, "Id=%i , WatchersCount=%i", watcherId, m_WatchersCount);
		PTRACE2(eLevelInfoNormal, "CSipDistrNotifyStruct::AddWatcherId, ", s);
		m_WatchersArray[m_WatchersCount] = watcherId;
		++m_WatchersCount;
	}
}

///////////////////////////////////////////////////////////////////////
void CSipDistrNotifyStruct::SetContentType(char* pType)
{
	if(pType)
		strncpy(m_pContentType, pType, H243_NAME_LEN);
}

///////////////////////////////////////////////////////////////////////
void CSipDistrNotifyStruct::SetContent(char* pContent, int length)
{
	if(pContent)
	{
	//	int len = strlen(pContent);
		if(m_pContent)
			PDELETEA(m_pContent);
		m_pContent = new char[length+1];
		memcpy(m_pContent, pContent, length);
		m_pContent[length] = '\0';
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
// CSipServiceStruct
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
CSipServiceStruct::CSipServiceStruct()
{

	m_Id = 0;

	//m_proxyIp = 0;
	m_proxyIp.v4.ip = 0;
	m_proxyIp.v6.scopeId = 0;
	memset(m_proxyIp.v6.ip, 0, IPV6_ADDRESS_BYTES_LEN);
	m_proxyIpVersion = eIpVersion4;

	m_proxyPort = 0;
	m_transportType = 0;
	m_subOpcode = GeneralServiceNull;
}

///////////////////////////////////////////////////////////////////////
CSipServiceStruct::~CSipServiceStruct()
{

}
///////////////////////////////////////////////////////////////////////
mcReqService* CSipServiceStruct::BuildServiceReq()
{
	mcReqService * pMsg = NULL;

	//serialize m_pHeaderFields and m_pHeaders into m_buffer
	Serialize();

	if(m_buffer && m_bufferLen)
	{
		int totalLength	= sizeof(mcReqServiceBase) - sizeof(sipMessageHeadersBase) + m_bufferLen;

		pMsg = (mcReqService *)new BYTE[totalLength];
		sipMessageHeaders* pSipHeaders = &(pMsg->sipHeaders);
		memcpy(pSipHeaders,m_buffer,m_bufferLen);
		pMsg->id = m_Id;
		pMsg->subOpcode = m_subOpcode;

		if (m_proxyIpVersion == eIpVersion4)
		{
			TRACEINTO << "MS_IPV6: eIpVersion4";
			pMsg->transportAddress.transAddr.addr.v4.ip = m_proxyIp.v4.ip;
			pMsg->transportAddress.transAddr.ipVersion = eIpVersion4;
		}
		else
		{
			TRACEINTO << "MS_IPV6: eIpVersion6";
			memcpy(pMsg->transportAddress.transAddr.addr.v6.ip, m_proxyIp.v6.ip, IPV6_ADDRESS_BYTES_LEN);
			pMsg->transportAddress.transAddr.ipVersion = eIpVersion6;
		}

		pMsg->transportAddress.transAddr.port = m_proxyPort;
		pMsg->transportAddress.transAddr.transportType = m_transportType;
		pMsg->transportAddress.transAddr.distribution = eDistributionUnicast;
		pMsg->transportAddress.unionProps.unionType = eIpVersion4;
		pMsg->transportAddress.unionProps.unionSize = sizeof(ipAddressIf);
	}
	return pMsg;
}
///////////////////////////////////////////////////////////////////////
void CSipServiceStruct::Dump(COstrStream& msg) const
{
	msg << "CSipServiceStruct::Dump\n";
	msg << "-------------------------\n";
	CSipBaseStruct::Dump(msg);
	msg << "Id: " << m_Id << "\n";
	msg << "Sub opcode: " << m_subOpcode << "\n";

	if (m_proxyIpVersion == eIpVersion4)
		msg << "MS_IPV6: Proxy IPV4 = " << m_proxyIp.v4.ip << "\n";
	else //eIpVersion6
		msg << "MS_IPV6: Proxy IPV6 = " << m_proxyIp.v6.ip << "\n";

	msg << "Proxy Port = " << m_proxyPort << "\n";
}


///////////////////////////////////////////////////////////////////////////////////////
DWORD CSipServiceStruct::GetId() const
{
	return m_Id;
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipServiceStruct::SetId(DWORD id)
{
	m_Id = id;
}

///////////////////////////////////////////////////////////////////////
void CSipServiceStruct::SetProxyIpV4(DWORD proxyIpV4)
{
	m_proxyIp.v4.ip = proxyIpV4;
}

///////////////////////////////////////////////////////////////////////
void CSipServiceStruct::SetProxyIpV6(char* pProxyIpV6)
{
	memcpy(m_proxyIp.v6.ip, pProxyIpV6, IPV6_ADDRESS_BYTES_LEN);
}
///////////////////////////////////////////////////////////////////////
void CSipServiceStruct::SetProxyIpVersion(enIpVersion proxyIpVersion)
{
	m_proxyIpVersion = proxyIpVersion;
}
///////////////////////////////////////////////////////////////////////
void CSipServiceStruct::SetProxyPort(WORD port)
{
	m_proxyPort = port;
}
///////////////////////////////////////////////////////////////////////
void CSipServiceStruct::SetTransportType(WORD transportType)
{
	m_transportType = transportType;
}
///////////////////////////////////////////////////////////////////////////////////////
WORD CSipServiceStruct::GetSubOpcode() const
{
	return m_subOpcode;
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipServiceStruct::SetSubOpcode(WORD subOpcode)
{
	m_subOpcode = subOpcode;
}

///////////////////////////////////////////////////////////////////////////////////////
//Please, before using this function in array, check the value, so it is not -1
//int GetCardSdpArrayIndex(cmCapDataType eMediaType)
//{
//	int res = -1;
//	if (eMediaType > cmCapData)
//		FPTRACE2INT(eLevelInfoNormal, "CSipDistrNotifyStruct::GetCardSdpArrayIndex, unknown media", eMediaType);
//	else
//		res = eMediaType - 1;
//	return res;
//}
/////////////////////////////////////////////////////////////////////////////////////////
bool IsMrcHeader(const sipSdpAndHeadersSt *pSdpAndHeaders, const char *pSearchStrInSessionHeader)
{
	bool IsMrcFound = false; 
	if (pSdpAndHeaders && pSdpAndHeaders->sipHeadersLength)
	{
		sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];

		if (!pHeaders)
			return IsMrcFound;

		char cMrcHeader[MAX_MRC_HEADER_SIZE] = "";
		if (SipGetHeaderValue(pHeaders, kMrd, cMrcHeader, MAX_MRC_HEADER_SIZE) == YES && cMrcHeader[0]) // svc check
		{
			FPTRACE(eLevelInfoNormal,"SVC was found in SIP header");
			IsMrcFound = true;
		}

		// VNGSWIBM-31
		// svc check (looking at session_name in SDP header, in case MRD header is missing from SIP header)
		if (!IsMrcFound)
		{
			char cSessionName[MaxUserAgentSize] = {0};
			::SipGetHeaderValue(pHeaders, kSdpSession_s, cSessionName, MaxUserAgentSize);
			if ( (strlen(cSessionName) > 0) && (NULL != strstr(cSessionName, pSearchStrInSessionHeader) ) )
			{
				FPTRACE(eLevelInfoNormal,"SVC was found in SDP header");
				IsMrcFound = true;
			}
		}

		//BRIDGE-13047: Ignore MRC header in case ANAT and MRD are present at the same time, i.e. ANAT (AVC) has higher priority than SVC.
		if (IsMrcFound && IsANATPresentInSDP(pSdpAndHeaders))
		{
			FPTRACE(eLevelInfoNormal,"MRD header is ignored in case ANAT and MRD are present at the same time");
			IsMrcFound = false;
		}
	}

	return IsMrcFound;
}
bool IsAvMcuNonCCCPCascade(const sipSdpAndHeadersSt *pSdpAndHeaders,BYTE isMsConfInvite)
{
	if (pSdpAndHeaders && pSdpAndHeaders->sipHeadersLength)
	{
		sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];

		if (!pHeaders)
			return false;

		char cUserAgent[MaxUserAgentSize] = {0};
		::SipGetHeaderValue(pHeaders, kUserAgent, cUserAgent, MaxUserAgentSize);
		if (strlen(cUserAgent) > 0)

		if(strstr(cUserAgent, "AV-MCU") && isMsConfInvite == FALSE )
		{
			FPTRACE(eLevelInfoNormal,"non cccp av-mcu found");
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//--- patch for ignoring Encryption in Cascade
BYTE GetCascadeModeFromHeader(const sipSdpAndHeadersSt *pSdpAndHeaders)
{
	if (pSdpAndHeaders && pSdpAndHeaders->sipHeadersLength)
	{
		sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];

		if (!pHeaders)
			return CASCADE_MODE_NONE;

		char cInformation[MAX_SDP_INFORMATION_HEADER_SIZE] = "";
		BYTE bIsInfoHeader = ::SipGetHeaderValue(pHeaders, kSdpSession_i, cInformation, MAX_SDP_INFORMATION_HEADER_SIZE);

		if ( (NO == bIsInfoHeader) || (!cInformation[0]) )
		{
		    return CASCADE_MODE_NONE;
		}

		if ( NULL != strstr(cInformation, INFO_HEADER_STR_SLAVE) )
		{
			return CASCADE_MODE_SLAVE;
		}
		else if ( NULL != strstr(cInformation, INFO_HEADER_STR_MASTER) )
		{
			return CASCADE_MODE_MASTER;
		}
	}

	return CASCADE_MODE_NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////
bool IsLync2013Client(const sipSdpAndHeadersSt *pSdpAndHeaders)
{
	if (pSdpAndHeaders && pSdpAndHeaders->sipHeadersLength)
	{
		sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];
		char cUserAgent[MaxUserAgentSize] = {0};
		if (SipGetHeaderValue(pHeaders, kUserAgent, cUserAgent, MaxUserAgentSize) == YES && cUserAgent[0])
		{
			if ( strstr(cUserAgent, "UCCAPI/15.0.4481.1000 OC/15.0.4481.1000 (Microsoft Lync)") )
				return true;
		}
	}

	return false;
}


/* The function check the existance of x-cisco tags in supported header */
BOOL CheckXciscoInSupportedHeader(const sipSdpAndHeadersSt *pSdpAndHeaders)
{
	BOOL isXciscoHeaderTagExist = NO;
	const CSipHeader* pHeader = NULL;


	if (pSdpAndHeaders && pSdpAndHeaders->sipHeadersLength)
	{
		sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];
		const char *pProprietryHeader;

		CSipHeaderList cHeaders(*pHeaders);


		const CSipHeader* pHeader = cHeaders.GetNextHeader(kSupported);

		while (pHeader && !isXciscoHeaderTagExist)
		{

			if (strstr(pHeader->GetHeaderStr(), "X-cisco-srtp-fallback") ||
				strstr(pHeader->GetHeaderStr(), "X-cisco-original-called"))
			{
				/* X-cisco exist in supported header*/
				isXciscoHeaderTagExist = YES;
			}
			pHeader = cHeaders.GetNextHeader(kSupported);
		}


	}

	return isXciscoHeaderTagExist;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
/* The function check the if "Plcm-Call-ID" if yes then the call is from DMA*/
BOOL GetIsCallWithDma(const sipSdpAndHeadersSt *pSdpAndHeaders)
{
	BOOL isDmaCall = NO;
	const CSipHeader* pHeader = NULL;


	if (pSdpAndHeaders && pSdpAndHeaders->sipHeadersLength)
	{
		sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];
		const char *pProprietryHeader;

		CSipHeaderList cHeaders(*pHeaders);

		const char* pPlcmHeaderStr = NULL;
		const CSipHeader* pPlcmHeader = cHeaders.GetNextPrivateOrProprietyHeader(kProprietyHeader, strlen(PLCM_CALL_ID_HEADER), PLCM_CALL_ID_HEADER);
		if (pPlcmHeader)
		{
			pPlcmHeaderStr = pPlcmHeader->GetHeaderStr();
				if (pPlcmHeaderStr)
				{
					isDmaCall = YES;
					FPTRACE2(eLevelInfoNormal,"GetIsCallWithDma pPlcmHeaderStr ",pPlcmHeaderStr);
					return isDmaCall ;
				}
		}

	}

	return isDmaCall;

}


////////////////////////////////////////////////////////////////////////////////////////////////


/* The function retrieve the x-plcm-require sip propriety header and set the mask according to the attributes that appear in the header */
void GetPlcmRequireHeaderMask(const sipSdpAndHeadersSt *pSdpAndHeaders, APIU16 *mask, eMediaLineInternalType plcmReqMaskMlineOrder[])
{
	*mask = 0;

	if (pSdpAndHeaders && pSdpAndHeaders->sipHeadersLength)
	{
		sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];
		const char *pProprietryHeader;

		CSipHeaderList cHeaders(*pHeaders);

		const CSipHeader* pPlcmRequireHeader = cHeaders.GetNextPrivateOrProprietyHeader(kProprietyHeader, strlen(X_PLCM_REQUIRE_STR), X_PLCM_REQUIRE_STR);
		//const char* xPlcmRequire = NULL;

		if (pPlcmRequireHeader)
		{
			pProprietryHeader = pPlcmRequireHeader->GetHeaderStr();

			if (strstr (pProprietryHeader, X_PLCM_SDP_RTP_STR))
				*mask |= m_plcmRequireAvp;

			if (strstr (pProprietryHeader, X_PLCM_SDP_SRTP_STR))
				*mask |= m_plcmRequireSavp;

			if (strstr (pProprietryHeader, X_PLCM_SDP_AUDIO_STR))
				*mask |= m_plcmRequireAudio;

			if (strstr (pProprietryHeader, X_PLCM_SDP_VIDEO_MAIN_STR))
				*mask |= m_plcmRequireVideoMain;

			if (strstr (pProprietryHeader, X_PLCM_SDP_VIDEO_SLIDES_STR))
				*mask |= m_plcmRequireVideoSlides;

			if (strstr (pProprietryHeader, X_PLCM_SDP_BFCP_UDP_STR))
				*mask |= m_plcmRequireBfcpUdp;

			if (strstr (pProprietryHeader, X_PLCM_SDP_BFCP_TCP_STR))
				*mask |= m_plcmRequireBfcpTcp;

			if (strstr (pProprietryHeader, X_PLCM_SDP_FECC_STR))
				*mask |= m_plcmRequireFecc;

			FPTRACE2INT(eLevelInfoNormal,"GetPlcmRequireHeaderMask: X-plcm-Require mask = ", *mask);

			const char *pStr = pProprietryHeader;
			int index = 0;

			/* Get mlines order */
			while ((pStr = strstr (pStr, "sdp-")) != NULL)
			{
				if (!strncmp(pStr+4, "audio", 5))
				{
					plcmReqMaskMlineOrder[index] = kMediaLineInternalTypeAudio;
					index++;
				}
				if (!strncmp(pStr+4, "video-main", 10))
				{
					plcmReqMaskMlineOrder[index] = kMediaLineInternalTypeVideo;
					index++;
				}
				if (!strncmp(pStr+4, "bfcp", 4))
				{
					plcmReqMaskMlineOrder[index] = kMediaLineInternalTypeBfcp;
					index++;
				}
				if (!strncmp(pStr+4, "fecc", 4))
				{
					plcmReqMaskMlineOrder[index] = kMediaLineInternalTypeFecc;
					index++;
				}
				pStr = pStr + 4;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
void TurnPlcmReqMaskSavp(APIU16& plcmRequireMask , BOOL bYesNo)
{
	if(bYesNo == YES)
	{
		plcmRequireMask = plcmRequireMask & ~m_plcmRequireAvp; /* turn off avp bit */
		plcmRequireMask |= m_plcmRequireSavp; /* turn on savp bit*/
	}
	else if(bYesNo == NO)
	{
		plcmRequireMask = plcmRequireMask & ~m_plcmRequireSavp; /* turn off savp bit */
		plcmRequireMask |= m_plcmRequireAvp; /* turn on avp bit*/
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
BYTE IsWebRtcCallSupported()
{
	BYTE status = NO;

	status = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_WEBRTC_SUPPORT);


	return status;
}

/////////////////////////////////////////////////////////////////////////////////////////
BYTE IsWebRtcCall(const sipSdpAndHeaders* pSdpAndHeaders)
{
	if (pSdpAndHeaders && pSdpAndHeaders->sipHeadersLength) {
		sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];
		if (pHeaders) {
			CSipHeaderList cHeaders(*pHeaders);
			if (cHeaders.GetNextHeader(kWebRtcSdp))
				return YES;
		}
	}

	return NO;
}

/////////////////////////////////////////////////////////////////////////////////////////
//return Header value from sipMessageHeaders in cValue
//if header exist return YES
BYTE SipGetHeaderValue(sipMessageHeaders *pHeaders, enHeaderField eHeader, char *cValue, size_t valueSize)
{
	BYTE bRetVal = NO;
	CSipHeader* pHeader = NULL;
	if ( !pHeaders || !cValue || !(eHeader > 0 && eHeader < enLastField) )
	{
	  ALLOCBUFFER(cLog, 256);
	
	  sprintf(cLog, "pHeader [0x%x] cValue [0x%x] eHeader [%d]", (int)pHeaders, (int)cValue, eHeader);
	  FPTRACE2(eLevelInfoNormal, "SipGetHeaderValue Failed - ", cLog);
	  DEALLOCBUFFER(cLog);
	}
	else
	{
		CSipHeaderList cHeaders(*pHeaders);
		pHeader	= (CSipHeader*) cHeaders.GetNextHeader(eHeader);

		if ( pHeader )
		{
			strcpy_safe(cValue, valueSize, pHeader->GetHeaderStr());
			bRetVal = YES;
		}
	}
	return bRetVal;
}

/////////////////////////////////////////////////////////////////////////
BYTE IsGoodConfName(const char* cName)
{
	BYTE bResult = NO;
	bResult = (strcspn(cName, BAD_CHARACTERS_FOR_SIP_URI) == strlen(cName));
	return bResult;
}

/////////////////////////////////////////////////////////////////////////
BYTE CheckIsFactoryAtDomain(const char* pStr)
{
	if (!pStr || pStr[0] == '\0')
		return NO;
	//find at @
	int nPlace = strcspn(pStr, "@");
	int nSize = strlen(pStr);
	if (nPlace == 0 || nPlace >= nSize - 1)
		return NO;
	//take string before at @
	char* pBase = new char[nPlace + 1];
	memset(pBase, 0, nPlace + 1);
	strncpy(pBase, pStr, nPlace);
	pBase[nPlace] = '\0';
	//check is string valid
	BYTE bIsValid = IsGoodConfName(pBase);
	delete[] pBase;
	if (bIsValid == NO)
		return NO;
	//take string after at @
	pBase = new char[nSize - nPlace];
	memset(pBase, 0, nSize - nPlace);
	strncpy(pBase, pStr + nPlace + 1, nSize - nPlace - 1);
	pBase[nSize - nPlace - 1] = '\0';
	//check is string valid
	bIsValid = IsGoodConfName(pBase);
	delete[] pBase;
	if (bIsValid == NO)
		return NO;
	return YES;
}

/////////////////////////////////////////////////////////////////////////
//extracts @Domain from given string and checks that Domain is valid name, otherwise NULL
const char* GetCurrentDomainAt(const char* pStr)
{
	if (!pStr || pStr[0] == '\0')
		return NULL;
	//find at @
	int nPlace = strcspn(pStr, "@");
	int nSize  = strlen(pStr);
	if (nPlace < nSize - 1)
	{
		//check is string valid and return with at @
		if (IsGoodConfName(pStr + nPlace + 1) == YES)
			return (pStr + nPlace);
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////
//extracts Domain from given string and checks that Domain is valid name, otherwise NULL
const char* GetCurrentDomain(const char* pStr)
{
	if (!pStr || pStr[0] == '\0')
		return NULL;
	//find at @
	int nPlace = strcspn(pStr, "@");
	int nSize  = strlen(pStr);
	if (nPlace < nSize - 1)
	{
		//check is string valid and return without at @
		if (IsGoodConfName(pStr + nPlace + 1) == YES)
			return (pStr + nPlace + 1);
	}
	return NULL;
}
#if 0
int GetLayerId(const VIDEO_OPERATION_POINT_SET_S &rOperationPoints, unsigned int width, unsigned int hieght, unsigned int frameRate)
{
	int retVal = -1;
	for (int i = 0;i < rOperationPoints.m_numberOfOperationPoints;i++)
	{
			if ((rOperationPoints.m_aVideoOperationPoints[i].frameWidth == width)
				&& (rOperationPoints.m_aVideoOperationPoints[i].frameHeight == hieght)
				&& (rOperationPoints.m_aVideoOperationPoints[i].frameRate >= frameRate))
			{

				retVal = i;
				break;
			}
	}
	return retVal;
}
#endif
/*
////////////////////////////////////////////////////////////////////
void SipSetNortelMode(int cSwitch)
{
	ALLOCBUFFER(cLog, 256);

	switch(cSwitch)
		{

		case 0: //OFF

			::GetpSystemCfg()->SetSipCIFRateInCPLimit(0);
			::GetpSystemCfg()->SetSipUserAgentVideoFlowControl("");
			::GetpSystemCfg()->SetSipUserAgentVideoFlowControlRate("64,128,192,256,384,512,768");
			::GetpSystemCfg()->SetSipTreatHoldAs(0);

			sprintf(cLog, " Setting NT flags to OFF");
			break;

		case 1: //ON

			::GetpSystemCfg()->SetSipCIFRateInCPLimit(128);
			::GetpSystemCfg()->SetSipUserAgentVideoFlowControl("Polycom");
			::GetpSystemCfg()->SetSipUserAgentVideoFlowControlRate( "8,64,128,256,384,512,768");
			::GetpSystemCfg()->SetSipTreatHoldAs(1);

			sprintf(cLog, " Setting NT flags to ON");
			break;

		default:

			sprintf(cLog, "Param [%d] not supported. Use 0 for OFF or 1 for ON", cSwitch);
			break;

		}

	FPTRACE2(eLevelInfoNormal, "SipUtils.SipSetNortelMode ", cLog);
	DEALLOCBUFFER(cLog);
}
*/

#define ILLEGAL_LAYER_ID -1

/////////////////////////////////////////////////////////////////////////////////////////////////////
int GetLayerId(const VIDEO_OPERATION_POINT_SET_S &rOperationPoints, unsigned int width, 
	unsigned int hieght, unsigned int frameRate,unsigned int bitRate, 
	cmCapDirection direction, DWORD partyId, CLargeString* pMsgStr)
{
	std::ostringstream ostr;
	int retVal = ILLEGAL_LAYER_ID; //-1;
	
	if (direction == cmCapReceive)
	{
		for (int i = 0; i < rOperationPoints.numberOfOperationPoints; i++)
		{
			// when looking for receive stream: find the lowest operation point that it's resolution >= stream resolution
			if ((rOperationPoints.tVideoOperationPoints[i].frameWidth >= width)
					&& (rOperationPoints.tVideoOperationPoints[i].frameHeight >= hieght)
					&& (rOperationPoints.tVideoOperationPoints[i].frameRate >= frameRate)
					&& (rOperationPoints.tVideoOperationPoints[i].maxBitRate >= bitRate))
			{
				retVal=rOperationPoints.tVideoOperationPoints[i].layerId;
				break;
			}
		}
	}
	else //direction == cmCapTransmit
	{
		if (pMsgStr)
		{
			*pMsgStr << "(pseudo) avc_vsw_relay function GetLayerId for partyId: " << partyId
				<< "\n EP requested by SCP request the following: width: " << width 
				<< " hieght: " <<hieght << "frameRate: " << frameRate << "\n";
		}
		
		for (int i = rOperationPoints.numberOfOperationPoints - 1; i >= 0; i--)
		{		
			// when looking for send stream: find the highest operation point that it's resolution <= stream resolution
			if ((rOperationPoints.tVideoOperationPoints[i].frameWidth <= width)
					&& (rOperationPoints.tVideoOperationPoints[i].frameHeight <= hieght)
					&& ((rOperationPoints.tVideoOperationPoints[i].frameRate <= frameRate) || (frameRate == 0))
					&& ((rOperationPoints.tVideoOperationPoints[i].maxBitRate <= bitRate) || (bitRate == 0)) )
			{
				retVal = rOperationPoints.tVideoOperationPoints[i].layerId;

				if (pMsgStr)
				{
					*pMsgStr << "operation points LayerId: " << i 
						<< " frameWidth: " << rOperationPoints.tVideoOperationPoints[i].frameWidth
						<< " frameHeight: " << rOperationPoints.tVideoOperationPoints[i].frameHeight
						<< " frameRate: " << rOperationPoints.tVideoOperationPoints[i].frameRate 
						<< " is selected. \n";
				}
			
				break;
			}
		}

	}

	return retVal;
}

unsigned int GetFrameRateInSignalingValues(unsigned int scpFrameRate)
{
	switch (scpFrameRate)
	{
	case(0):
		return 1920; //(7.5 * 256) --> 7.5 fps
	case(1):
		return 3840; //(15 * 256) --> 15 fps
	case(2):
		return 7680; //(30 * 256) --> 30 fps
	case(3):
		return 15360; //(60 * 256) --> 60 fps
	default:
		{
			FPTRACE2INT(eLevelError," _scp_flow_ GetFrameRateInSignalingValues,unsupported scp frame rate:",scpFrameRate);
			return 0;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
const char* eScpNotificationReasonsAsString[] =
{
	"stream is now provided",
	"stream not available",
	"stream is muted",
	"stream not provided because of bandwidth",
	"stream not provided because of error resiliency",
	"stream not provided because ep disconnected",
    "stream not provided because of ivr slide state"
};

const char* GetScpNotificationReasonsEnumValueAsString(eScpNotificationReasons val)
{
	const char* sName =	( ((eStreamIsNowProvided/*0*/ <= val) && (val < eScpNotificationReasonsMaxNumOfValues))
						?
						eScpNotificationReasonsAsString[val-eStreamIsNowProvided/*0*/] : "Invalid value" );
	return sName;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
const char* eScpNotificationTypeAsString[] =
{
	"default scp notification type",
	"stream cannot be provided",
	"stream can now be provided"
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
WORD ExtractNumOfScreensFromContactHeader(const char* strContact)
{
	char parsStr[512];//have some comilation error... will restrun to IP_LEN later
	WORD numOfScreens = 0;
	//const char* strContact = m_pSIPNetSetup->GetRemoteSipContact();
	//TRACEINTO << "ExtractNumOfScreensFromContactHeader: Contact=" <<  strContact;
	memset(parsStr, '\0', 512);
	strncpy(parsStr, strContact, 511);
	char *start = strstr(parsStr, "x-cisco-multiple-screen=");
	if( start!=NULL )
	{
		start+=strlen("x-cisco-multiple-screen=");
		numOfScreens = atoi(start);
	}
	//TRACEINTO << "ExtractNumOfScreensFromContactHeader: numOfScreens=" <<  (int)numOfScreens;
	return numOfScreens;
}

const char* GetScpNotificationTypeEnumValueAsString(eScpNotificationType val)
{
	const char* sName =	( ((eScpNotificationTypeUndefined <= val) && (val < eScpNotificationTypeMaxNumOfValues))
						?
						eScpNotificationTypeAsString[val-eScpNotificationTypeUndefined] : "Invalid value" );
	return sName;
};

/*
///////////////////////////////////////////////////////////////////////////////////
WORD ExtractNumOfScreensFromContactHeader(const char* strContact)
{
	char parsStr[512];//have some comilation error... will restrun to IP_LEN later
	WORD numOfScreens = 0;
	//const char* strContact = m_pSIPNetSetup->GetRemoteSipContact();
	//TRACEINTO << "ExtractNumOfScreensFromContactHeader: Contact=" <<  strContact;
	memset(parsStr, '\0', IP_STRING_LEN);
	strncpy(parsStr, strContact, 511);
	char *start = strstr(parsStr, "x-cisco-multiple-screen=");
	if( start!=NULL )
	{
		start+=strlen("x-cisco-multiple-screen=");
		numOfScreens = atoi(start);
	}
	//TRACEINTO << "ExtractNumOfScreensFromContactHeader: numOfScreens=" <<  (int)numOfScreens;
	return numOfScreens;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
//eFeatureRssDialin
enSrsSessionType   getSrsSessionType(const sipSdpAndHeadersSt  *pSdpAndHeaders)
{
const uint 	SRS_CONNECTION_TYPE_RECORDING=		1; 
const uint 	SRS_CONNECTION_TYPE_PLAYBACK=		2; 
const char*  	SRS_INDENTIFIER_IN_SDP=					"SRS";
const char* 	SRS_INDENTIFIER_IN_SDP_2= 				"Srs";
const char* 	SRS_INDENTIFIER_IN_SDP_3=				"srs";



	BOOL isSRSOwner = false;
	BOOL isSRSSession = false;
	
	if (NULL == pSdpAndHeaders || 0 >= pSdpAndHeaders->sipHeadersLength)
	{
		FPTRACE(eLevelInfoNormal," invalid parameter");
		return eSrsSessionTypeRegular;
	}


	sipMessageHeaders *pHeaders = (sipMessageHeaders *) &pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset];	
	if (!pHeaders)
	{	
		FPTRACE(eLevelInfoNormal,"  invalid SDP headers");
		return eSrsSessionTypeRegular;
	}
	
	char cSessionName[MaxUserAgentSize] = {0};
	char cSessionOwner[MaxUserAgentSize] = {0};

	//Need to check both the 'o' and 's' for RSS 
	::SipGetHeaderValue(pHeaders, kSdpSession_o, cSessionOwner,MaxUserAgentSize);
	if(strlen(cSessionOwner) > 0)
	{
		if  (NULL != strstr(cSessionOwner, const_cast<char*>(SRS_INDENTIFIER_IN_SDP)) ) 
		{
			isSRSOwner  =  true;
		}
		else if   (NULL != strstr(cSessionOwner, const_cast<char*>(SRS_INDENTIFIER_IN_SDP_2)) ) 
		{
			isSRSOwner  =  true;
		}
		else if  (NULL != strstr(cSessionOwner, const_cast<char*>(SRS_INDENTIFIER_IN_SDP_3)) ) 
		{
			isSRSOwner  =  true;
		}
		else
		{
			FPTRACE(eLevelInfoNormal," no SRS identifier in SDP o-line ");			
		}
	}
	
	::SipGetHeaderValue(pHeaders, kSdpSession_s, cSessionName,MaxUserAgentSize);	
	if(strlen(cSessionName) > 0)
	{
		if (NULL != strstr(cSessionName, const_cast<char*>(SRS_INDENTIFIER_IN_SDP)) ) 
		{
			isSRSSession  =  true;
		}
		else if(NULL != strstr(cSessionName, const_cast<char*>(SRS_INDENTIFIER_IN_SDP_2)) ) 
		{
			isSRSSession  =  true;
		}		
		else if (NULL != strstr(cSessionName, const_cast<char*>(SRS_INDENTIFIER_IN_SDP_3)) ) 
		{
			isSRSSession  =  true;
		}
		else
		{
			FPTRACE(eLevelInfoNormal," no SRS identifier in SDP s-line ");
		}
	}

	if(false == isSRSOwner&&isSRSSession)
	{
		return eSrsSessionTypeRegular;
	}

	//ConnectionType
	if(SRS_CONNECTION_TYPE_RECORDING == pSdpAndHeaders->rssConnectionType)
	{
		return eSrsSessionTypeRecording;
	}
	else if(SRS_CONNECTION_TYPE_PLAYBACK == pSdpAndHeaders->rssConnectionType)
	{
		return  eSrsSessionTypePlayback;
	}
	else
	{
		return eSrsSessionTypeRegular;
	}
		
	return  eSrsSessionTypeRegular;
}

///////////////////////////////////////////////////////
BOOL  IsMediaAudioOnly(sipSdpAndHeadersSt *pSdpAndHeaders)
{
	BOOL		result = NO;
	if ( pSdpAndHeaders && pSdpAndHeaders->sipMediaLinesLength )
	{
		const sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *)&pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipMediaLinesOffset];
		unsigned int mediaLinePos = 0;

		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {


			if (mediaLinePos >= pSdpAndHeaders->lenOfDynamicSection) {
				DBGFPASSERT(mediaLinePos);
				break;
			}

			const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			const capBuffer *pCapBuffer = (capBuffer *) &pMediaLine->caps[0];
			const BYTE *pTemp = (const BYTE*)pCapBuffer;

			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			if (eMediaLineTypeAudio == pMediaLine->type)
			{
				 result = YES;
			}

			if ((eMediaLineTypeVideo == pMediaLine->type) && (eMediaLineContentSlides != pMediaLine->content))
			{
				result = NO;
				return result;
			}

		}
	}

	return result;
}




