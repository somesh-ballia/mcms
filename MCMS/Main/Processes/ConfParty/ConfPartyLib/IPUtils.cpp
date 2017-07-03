//+========================================================================+
//                            IPUTILS.CPP                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IPUTILS.CPP                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara Avidan                                                |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 17/2/03    | Utilities fot IP parties		                       |
//+========================================================================+

#include <stdlib.h>
#include "IPUtils.h"
#include "DefinesIpService.h"
#include "ObjString.h"
#include "BigDiv.h"
#include "IpPartyMonitorDefinitions.h"
#include "CapInfo.h"
#include "CapClass.h"
#include "Segment.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "Trace.h"
#include "CapInfo.h"
#include "StringsLen.h"
#include "TraceStream.h"

#include "IpCommon.h"
#include "IpService.h"
#include "IpCommonUtilTrace.h"
#include "OpcodesRanges.h"

#include "SipStructures.h"
#include "PrecedenceSettings.h"
extern CPrecedenceSettings* GetpPrecedenceSettingsDB();


/*
#ifndef _CAPINFO
#include  <CapInfo.h>
#endif

#ifndef _CAPCLASS
#include  <CapClass.h>
#endif

#ifndef _COBJSTRINGAPP
#include    <obstring.h>
#endif*/

#define  MultiplePartyMonitorValue 100000 // to convert the information to % with two digits after the '.' (XY.ZT), and also multiple it by 10 because of backward compatibility.
#define  DefenseValue 0xF0000000
#define  NumberOfUniqueString 200000

const char* g_directionStrings[]	= {"Inactive","Receive","Transmit","Receive And Transmit"};
const char* g_roleLabelStrings[]	= {"People", "Content", "Presentation", "Content Or Presentation", "Live", "", "Live Or Presentation"};
const char* g_typeStrings[] =
{
	"EMPTY",
	"AUDIO",
	"VIDEO",
	"DATA",
	"NON STANDARD",
	"USER INPUT",
	"CONFERENCE",
	"H235",
	"MAX PENDING REPLACEMENT FOR",
	"GENERIC",
	"MULTIPLEXED STREAM",
	"AUDIO TELEPHONY EVENT",
	"AUDIO TONE",
	"BFCP",
	"LPR"
};

const char* g_ipChanTypeStrings[] =
{
	"H225/SIGNALING",
	"H245/SDP",
	"AUDIO IN ",
	"AUDIO OUT",
	"VIDEO IN ",
	"VIDEO OUT",
//	"T120  IN ",
//	"T120  OUT",
	"AUDIO CONT IN ", //not supported yet (used for calculation)
	"AUDIO CONT OUT",//not supported yet
	"VIDEO CONT IN ",
	"VIDEO CONT OUT",
	"FECC  IN ",
	"FECC  OUT",
	"BFCP  IN",
	"BFCP  OUT"
};

//////////////////////////////////////////////////////////////////////////////////////////////////
cmCapDirection CalcOppositeDirection(cmCapDirection d)
{
	cmCapDirection res = d; // if receive & transmit
	if (d==cmCapReceive)
		res = cmCapTransmit;
	else if (d==cmCapTransmit)
		res = cmCapReceive;
	return res;
}

/////////////////////////////////////////////////////////////////////////////
WORD IsValidIPAddress(const char* pIP)
{
	char *pFront, *pRear;
	char sTmp[IP_ADDRESS_LEN];
	int i, val;
	WORD bIsValid = YES;

	if ( strlen(pIP) > IP_ADDRESS_LEN )
	{
		//This condition also insures that all fields are smaller then sTmp size.
		return NO;
	}

	pRear = (char*) pIP;

	for( i = 0; i < 4; i++ )
	{
		if ( i == 3 )
		{ //last field
		  strncpy(sTmp, pRear, sizeof(sTmp)-1);
		  sTmp[sizeof(sTmp)-1] = '\0';
			val = atoi(sTmp);
		}
		else if ( (pFront = strchr(pRear, '.')) )
		{
			strncpy(&sTmp[0], pRear, pFront - pRear);
			sTmp[pFront - pRear] = '\0';
			val = atoi(sTmp);
			pRear = (pFront + 1);
			if ( strlen(pRear) == 0 )
			{
				bIsValid = NO; //string that end by '.'
				break;
			}
		}
		else //not found next "."
		{
			bIsValid = NO;
			break;
		}

		//retrieved value does not valid
		if ( (val < 0) || (val > 255) )
		{
			bIsValid = NO;
			break;
		}
	}

	return bIsValid;
}

/////////////////////////////////////////////////////////////////////////////
WORD IsValidPort(const char* pPort)
{
	WORD res = 0;
	if (pPort[5] < '0' || pPort[5] > '9') // not a digit
	{
		DWORD port = atoi(pPort);
		if (port <= 65535)
		res = (WORD)port;
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
int GetPayloadType(CCapSetInfo &capInfo)
{
	DWORD payloadType = capInfo.GetPayloadType();
	CapEnum algCapEnum = capInfo.GetIpCapCode();

	switch (algCapEnum)
	{
		case eG7221_32kCapCode:
		case eG7221_24kCapCode:
		case eG7221_16kCapCode:
		case eSiren7_16kCapCode:
		case eSiren14_48kCapCode:
		case eSiren14_32kCapCode:
		case eSiren14_24kCapCode:
		case eG7221C_48kCapCode:
		case eG7221C_32kCapCode:
		case eG7221C_24kCapCode:
		case eRfc2833DtmfCapCode:
		case eH264CapCode:
		case eVP8CapCode:
		case eSvcCapCode:
		case eLPRCapCode:
		case eAnnexQCapCode:
		case eSiren14Stereo_48kCapCode:
		case eSiren14Stereo_56kCapCode:
		case eSiren14Stereo_64kCapCode:
		case eSiren14Stereo_96kCapCode:
		case eSiren22Stereo_128kCapCode:
		case eSiren22Stereo_96kCapCode:
		case eSiren22Stereo_64kCapCode:
		case eSiren22_64kCapCode:
		case eSiren22_48kCapCode:
		case eSiren22_32kCapCode:
		case eSirenLPR_32kCapCode:
		case eSirenLPR_48kCapCode:
		case eSirenLPR_64kCapCode:
		case eSirenLPRStereo_64kCapCode:
		case eSirenLPRStereo_96kCapCode:
		case eSirenLPRStereo_128kCapCode:
		case eSirenLPR_Scalable_32kCapCode:
		case eSirenLPR_Scalable_48kCapCode:
		case eSirenLPR_Scalable_64kCapCode:
		case eSirenLPRStereo_Scalable_64kCapCode:
		case eSirenLPRStereo_Scalable_96kCapCode:
		case eSirenLPRStereo_Scalable_128kCapCode:
		case eiLBC_13kCapCode:
		case eiLBC_15kCapCode:
	    	case eOpus_CapCode:
	    	case eOpusStereo_CapCode:
		case eG719_64kCapCode:
		case eG719_48kCapCode:
		case eG719_32kCapCode:
		case eG719Stereo_128kCapCode:
		case eG719Stereo_96kCapCode:
		case eG719Stereo_64kCapCode:
		case eAAC_LDCapCode:// TIP
		case eG722Stereo_128kCapCode:
		case eFECCapCode: //LYNC2013_FEC_RED
		case eREDCapCode: //LYNC2013_FEC_RED
		{
			payloadType = capInfo.GetDynamicPayloadType(0);// no content
		}
		default:
			break;
	}
	return payloadType;
}

///////////////////////////////////////////////////////////////////////////////////////
BYTE IsDynamicPayloadType(DWORD payload)
{
	return ( (payload >= __FIRST_DPT) && (payload <= __LAST_DPT));
}


///////////////////////////////////////////////////////////////////////////////////////
BYTE IsValidPayloadType(DWORD payload)
{
	return (payload != _UnKnown);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
EIpChannelType CalcChannelType(cmCapDataType eType,BOOL bTransmit,ERoleLabel eRole,CapEnum capCode)
{
	EIpChannelType channelType = H225;
	switch(eType)
	{
		case cmCapAudio:
			if(bTransmit)
				channelType = AUDIO_OUT;
			else
				channelType = AUDIO_IN;
			break;
		case cmCapVideo:
			if(bTransmit)
			{
				if(eRole == kRolePeople)
					channelType = VIDEO_OUT;
				else
					channelType = VIDEO_CONT_OUT;
			}
			else
			{
				if(eRole == kRolePeople)
					channelType = VIDEO_IN;
				else
					channelType = VIDEO_CONT_IN;
			}
			break;
		case cmCapData:
			if (bTransmit)
				channelType = FECC_OUT;
			else
				channelType = FECC_IN;
			break;
		case cmCapBfcp:
			if (bTransmit)
				channelType = BFCP_OUT;
			else
				channelType = BFCP_IN;
			break;
		default:
			FPTRACE(eLevelInfoNormal,"Unexpected channel type");
	}

	return channelType;
}


/////////////////////////////////////////////////////////////////////////////
kChanneltype DataTypeToChannelType(cmCapDataType dataType, ERoleLabel eRole)
{
	kChanneltype eChannelType = kEmptyChnlType;
	switch (dataType)
	{
		case cmCapAudio:
             eChannelType = kIpAudioChnlType;
			break;
		case cmCapVideo:

                if (eRole == kRolePeople)
                    eChannelType = kIpVideoChnlType;
                else
                    eChannelType = kIpContentChnlType;
			break;
		case cmCapData:
			eChannelType = kIpFeccChnlType;
			break;
		case cmCapBfcp:
			eChannelType = kBfcpChnlType;
			break;
		default:
			eChannelType = kUnknownChnlType;
			break;
	}
	return eChannelType;
}

/////////////////////////////////////////////////////////////////////////////
cmCapDataType ChannelTypeToDataType( kChanneltype eChannelType, ERoleLabel &eRole)
{
	cmCapDataType eDataType = cmCapEmpty;
	eRole = kRolePeople;

	switch (eChannelType)
	{
		case kIpAudioChnlType:
			eDataType = cmCapAudio;
			break;
		case kIpVideoChnlType:
			eDataType = cmCapVideo;
			break;
		case kIpFeccChnlType:
			eDataType = cmCapData;
			break;
		case kBfcpChnlType:
			eDataType = cmCapBfcp;
			break;
		case kIpContentChnlType:
			eDataType = cmCapVideo;
			eRole	  = kRoleContentOrPresentation;
			break;
		case kSvcAvcChnlType:
			eDataType = cmCapVideo;
			break;
		case kAvcVSWChnType:  /* kAvcVSWChnType need to change APICOM for a new channel type  */
			eDataType = cmCapVideo;
			break;
		case kAvcToSvcChnlType:
			eDataType = cmCapVideo;
			break;
		case kAvcToSacChnlType:
			eDataType = cmCapAudio;
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}

	return eDataType;
}


/////////////////////////////////////////////////////////////////////////////
int AllocateUniqueString(char* sDest, DWORD signalingIpAddress)
{
	static APIS32 counter  = 0;
	ALLOCBUFFER(str16, Size16);
	ALLOCBUFFER(TempStr, 20);

	//need that for creating a uniqe callId also between MCU's
//    DWORD MCU_IP = 0x12ac34bd;//::GetMCUIP();

	counter ++;

	if (counter > NumberOfUniqueString)
		counter = 1;

    sprintf(str16, "%u", signalingIpAddress);
    sprintf(TempStr, "%ld", counter);

	memset(sDest, '0', Size16);
	memcpy(sDest, str16, strlen(str16));

	size_t len = strlen(TempStr);
	int position = Size16 - len;
	memcpy(sDest + position, TempStr, len); //add it in the end of the string

	DEALLOCBUFFER(str16);
	DEALLOCBUFFER(TempStr);

    return 0;

} //AllocateUniqueString

//////////////////////////////////////////////////////////////////////////////////////////////////
void  RoundAudioRate(DWORD &actualRate, DWORD channelRate)
{
	int intervalRate = channelRate - actualRate;

	if (intervalRate >0)
	{
		if(intervalRate <= rate4K)
			actualRate = channelRate;
		else //intervalRate > rate4K
		{
			int	   reminder = intervalRate % rate8K;
			int    diff = intervalRate / rate8K;
			if(diff < 1)
				actualRate = channelRate - rate8K;
			else //diff >= 1
			{
				if(reminder)
					actualRate = channelRate - (rate8K * (diff + 1));
				else //reminder==0
					actualRate = channelRate - (rate8K * diff);
			}
		}
	}
	else //intervalRate <= 0 the actualRate is bigger than the rate of the channel - the max rate can be the rate of the channel
	{
		actualRate = channelRate;
	}
}

///////////////////////////////////30//////////////////////////////////////////
void VideoSyncParamsParser(DWORD streamVideoSyncParams, BYTE & rIntraSyncFlag,
						   BYTE & rVideoBCHSyncFlag, WORD & rBchOutOfSyncCount,
						   BYTE & rProtocolSyncFlag, WORD & rProtocolOutOfSyncCount)
{
	////////////////////////////////////////////////////////////////////////////////
	// streamVideoSyncParams (DWORD) structure:
	//  _____________________________________________________________
	// | 0 | 1 | 2 . . . . . . . 15 | 16 | 17 | 18 . . . . . . . 31 |
	// |___|___|____________________|____|____|_____________________|
	//
	// 0	  - Valid Cell Flag
	// 1  	  - Video BCH Sync Flag
	// 2..15  - BCH Out Of Sync Counter
	// 16	  - Intra Sync Flag
	// 17	  - Video Protocol Sync Flag
	// 18..31 - Video Protocol Out Of Sync Count
	//////////////////////////////////////////////////////////////////////////////////

	rIntraSyncFlag			= 0;
    rVideoBCHSyncFlag		= 0;
	rBchOutOfSyncCount		= 0;
    rProtocolSyncFlag		= 0;
	rProtocolOutOfSyncCount = 0;

	if ( !(ValidBitMask & streamVideoSyncParams) )
		return;

	DWORD temp = 0;

	// 1). Set BCH params
	if ( streamVideoSyncParams & BchSyncMask )
		rVideoBCHSyncFlag = 1;

	temp = streamVideoSyncParams & BchCountMask;
	rBchOutOfSyncCount = temp >> 16;

	// 2). Set video protocol params
	if ( streamVideoSyncParams & ProtocolSyncMask )
		rProtocolSyncFlag = 1;

	rProtocolOutOfSyncCount = streamVideoSyncParams & ProtocolCountMask;

	// 3). Set Intra Sync Flag
	if ( streamVideoSyncParams & IntraSyncBitMask )
		rIntraSyncFlag = 1;
}

/////////////////////////////////////////////////////////////////////////////
void SetPartyMonitorBaseParams(CPrtMontrBaseParams *pPrtMonitrParams,DWORD channelType,DWORD rate,
									mcTransportAddress* partyAdd,mcTransportAddress* mcuAdd,DWORD protocol,DWORD pmIndex,
									BYTE IsIce,mcTransportAddress* IcePartyAdd,mcTransportAddress* IceMcuAdd, EIceConnectionType IceConnectionType)
{
	if (pPrtMonitrParams == NULL)
	{
		FPTRACE(eLevelInfoNormal, "SetPartyMonitorBaseParams: pPrtMonitrParams = NULL!!!");
		return;
	}

	pPrtMonitrParams->SetChannelIndex(pmIndex);
	pPrtMonitrParams->SetProtocol(protocol);
	pPrtMonitrParams->SetBitRate(rate);
	pPrtMonitrParams->SetChannelType(channelType);
	pPrtMonitrParams->SetPartyAddr(partyAdd);
	pPrtMonitrParams->SetPartyPort(partyAdd->port);
	pPrtMonitrParams->SetMcuAddr(mcuAdd);
	pPrtMonitrParams->SetMcuPort(mcuAdd->port);
	pPrtMonitrParams->SetConnectionStatus(1);
	pPrtMonitrParams->SetIsIce(IsIce);

	if (IsIce && IcePartyAdd && IceMcuAdd)
	{
		if (!(isApiTaNull(IcePartyAdd) || isApiTaNull(IceMcuAdd)))
		{//Ice
			pPrtMonitrParams->SetIcePartyAddr(IcePartyAdd);
			pPrtMonitrParams->SetIcePartyPort(IcePartyAdd->port);
			pPrtMonitrParams->SetIceMcuAddr(IceMcuAdd);
			pPrtMonitrParams->SetIceMcuPort(IceMcuAdd->port);
			pPrtMonitrParams->SetIceConnectionType(IceConnectionType);

		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void SetPartyMonitorParams(CPrtMontrGlobalParams *pPrtMonitrParams,TAdvanceMonitoringResultsSt *pAdvanceMonitoring)
{
	TRtcpInfoSt tRtcpInfoSt			= pAdvanceMonitoring->tRtcpInfoSt;

	pPrtMonitrParams->SetNumOfPackets(tRtcpInfoSt.unAccumulatedPacket);
	pPrtMonitrParams->SetLatency(tRtcpInfoSt.unLatency);
	////////////////////
	CSmallString str;
	str<<"tRtcpInfoSt.unAccumulatedPacketLoss = "<< tRtcpInfoSt.unAccumulatedPacketLoss<< ".\n";
	str<<"ttRtcpInfoSt.unAccumulatedPacket = "<< tRtcpInfoSt.unAccumulatedPacket<< ".\n";
	str<<"ttRtcpInfoSt.unIntervalPeakJitter = "<< tRtcpInfoSt.unIntervalPeakJitter<< ".\n";
	FPTRACE2(eLevelInfoNormal,"SetPartyMonitorParams:  -  ", str.GetString());
	///////////////////////
	pPrtMonitrParams->SetPacketLoss(tRtcpInfoSt.unAccumulatedPacketLoss);
	pPrtMonitrParams->SetJitter(tRtcpInfoSt.unIntervalJitter);
	pPrtMonitrParams->SetJitterPeak(tRtcpInfoSt.unIntervalPeakJitter);
	pPrtMonitrParams->SetFranctionLoss(tRtcpInfoSt.unIntervalFractionLoss);
	pPrtMonitrParams->SetFranctionLossPeak(tRtcpInfoSt.unIntervalPeakFractionLoss);
}

/////////////////////////////////////////////////////////////////////////////////////
void SetAdvanceInfo(CAdvanceChInfo& pPrt, TAdvanceInfoSt& advanceInfo, TAdvanceMonitoringResultsSt *pAdvanceMonitoring,
					BOOL isCalculateAccumulated, BOOL isCalculateInterval, BOOL isOutOfOrderCalculation)
{
	DWORD rtpAccumulatedPackets	= pAdvanceMonitoring->tRtpStatisticSt.unAccumulatedPacket;
	DWORD rtpIntervalPackets	= pAdvanceMonitoring->tRtpStatisticSt.unIntervalPacket;
	DWORD percentResult 		= 0;

	//We must not get negative value - when divided with a negative value the PSOS stack!!!
	if(rtpAccumulatedPackets & DefenseValue)
	{
		CSmallString str;
		str<<"rtpAccumulatedPackets = "<< (DWORD)rtpAccumulatedPackets;
		FPTRACE2(eLevelInfoNormal,"SetAdvanceInfo 1 : receiving a negative value -  ", str.GetString());
		DBGFPASSERT(rtpAccumulatedPackets);
		return;
	}

	if(rtpIntervalPackets & DefenseValue)
	{
		CSmallString str;
		str<<"rtpIntervalPackets = "<< (DWORD)rtpIntervalPackets;
		FPTRACE2(eLevelInfoNormal,"SetAdvanceInfo 2 : receiving a negative value -  ", str.GetString());
		DBGFPASSERT(rtpIntervalPackets);
		return;
	}

	pPrt.SetAccumulate(advanceInfo.unAccumulated);
	pPrt.SetInterval(advanceInfo.unInterval);
	pPrt.SetIntervalPeak(advanceInfo.unPeak);

	if(rtpAccumulatedPackets && isCalculateAccumulated)
	{
		long long int Acc   = advanceInfo.unAccumulated;
		long long int Multi = MultiplePartyMonitorValue;
		long long int dive  = rtpAccumulatedPackets;
		if(isOutOfOrderCalculation)
			percentResult = (DWORD)((Acc*Multi)/(Acc+dive));
		else
			percentResult = (DWORD)((Acc*Multi)/dive);
		pPrt.SetAccumulatePercent(percentResult);
	}
	if(rtpIntervalPackets && isCalculateInterval)
	{
		long long int Acc   = advanceInfo.unInterval;
		long long int Multi = MultiplePartyMonitorValue;
		long long int dive  = rtpIntervalPackets;
		if(isOutOfOrderCalculation)
			percentResult = (DWORD)((Acc*Multi)/(Acc+dive));
		else
			percentResult = (DWORD)((Acc*Multi)/dive);
		pPrt.SetIntervalPercent(percentResult);
	}
}

/////////////////////////////////////////////////////////////////////////////
void SetRtpInfo(CRtpInfo *pPrtMonitrRtpParams,TAdvanceMonitoringResultsSt *pAdvanceMonitoring)
{
	TRtpStatisticSt tRtpStatisticSt		= pAdvanceMonitoring->tRtpStatisticSt;

	// this value should not be display, no matter what the card report.
//	tRtpStatisticSt.tJitterBufferSize.unInterval = 0xFFFF;

	SetAdvanceInfo(pPrtMonitrRtpParams->GetRtpPacketLoss(),tRtpStatisticSt.tPacketsActualLoss,pAdvanceMonitoring,TRUE,TRUE,TRUE);
	SetAdvanceInfo(pPrtMonitrRtpParams->GetRtpOutOfOrder(),tRtpStatisticSt.tPacketsOutOfOrder,pAdvanceMonitoring);
	SetAdvanceInfo(pPrtMonitrRtpParams->GetRtpFragmentPackets(),tRtpStatisticSt.tPacketsFragmented,pAdvanceMonitoring);
	SetAdvanceInfo(pPrtMonitrRtpParams->GetJitterBufferSize(),tRtpStatisticSt.tJitterBufferSize,pAdvanceMonitoring,FALSE, TRUE);
	// remove Jiettr buffer 'n - accumulated' field (SRS demand)
	pPrtMonitrRtpParams->GetJitterBufferSize().SetAccumulate(0xFFFFFFFF);

	SetAdvanceInfo(pPrtMonitrRtpParams->GetJitterLatePackets(),tRtpStatisticSt.tJitterLatePackets,pAdvanceMonitoring);
	SetAdvanceInfo(pPrtMonitrRtpParams->GetJitterOverflows(),tRtpStatisticSt.tJitterOverFlows,pAdvanceMonitoring);
	SetAdvanceInfo(pPrtMonitrRtpParams->GetJitterSamplePacketInterval(),tRtpStatisticSt.tJitterSamplePacketInterval,pAdvanceMonitoring,FALSE, TRUE);
	// remove sample packet 'n - accumulated' (SRS demand)
	pPrtMonitrRtpParams->GetJitterSamplePacketInterval().SetAccumulate(0xFFFFFFFF);
}


/////////////////////////////////////////////////////////////////////////////
void  UpdatePartyMonitoring(TRtpCommonChannelMonitoring *pChannelMonitoring,
							CPrtMontrBaseParams *pPrtMonitrParams,
							DWORD	*pOldMediaBytesArr,
							CapEnum capCode,
							EIpChannelType channelType,
							DWORD channelIndex,
							DWORD callIndex,
							DWORD *pOldFrames,
							DWORD rate,
							char *pChannelParams,
							mcTransportAddress* partyAddr,
							mcTransportAddress* mcuAddr,
							BYTE IsIce ,
							mcTransportAddress* IcePartyAddr,
							mcTransportAddress* IceMcuAddr,
							EIceConnectionType IceConnectionType)
{
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	DWORD ticksIntervalInMilSec;
	DWORD tempMediaBytes;
	DWORD mediaBytes;
	//DWORD tempFrames;
	DWORD frameRate = 0;
	DWORD frames;
	DWORD actualFrames      = 0;
	DWORD actualRate        = 0;

	CCapSetInfo capInfo = capCode;
	CMedString	strExcFieldsRules;

	ticksIntervalInMilSec = pChannelMonitoring->unTicksInterval;
	tempMediaBytes        = pChannelMonitoring->unMediaBytes;
	mediaBytes			  = tempMediaBytes - pOldMediaBytesArr[channelType-2];

	CMedString strBase1;
	strBase1<<", ticksIntervalInMilSec = "<<ticksIntervalInMilSec<<", tempMediaBytes = "<<tempMediaBytes;
	strBase1<<", pOldMediaBytesArr[channelType-2] = "<<pOldMediaBytesArr[channelType-2];
	strBase1<<", mediaBytes = "<<mediaBytes;

	pOldMediaBytesArr[channelType-2] = tempMediaBytes;

	if((pChannelMonitoring->unChannelType == kIpVideoChnlType) || (pChannelMonitoring->unChannelType == kIpContentChnlType))
	{// calculated frame rate (FPS)

	    frameRate = ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unFrameRate;
	    FPTRACE2INT(eLevelInfoNormal,"UpdatePartyMonitoring this is frame rate- noa dbg ",frameRate);

	    /*	tempFrames = ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unFrames;
		if(tempFrames > pOldFrames[channelType-2])
			frames	= tempFrames - pOldFrames[channelType-2];
		else
			frames	= 0;

		pOldFrames[channelType - 2] = tempFrames;
		if(ticksIntervalInMilSec != 0)
			actualFrames=((frames*1000)/ticksIntervalInMilSec);*/
	}

	if(ticksIntervalInMilSec!=0)
	{// calculate actual channel bit rate
		// in order to recognize overflow, by looking in the operator, the actual rate
		// is float instead of int (we do it when the actual monitoring flag is ON or
		// if its G7231 channel becuase the rate at that channel is less than 8k and
		// in the operator it is always zero).
		long long int Acc   = mediaBytes;
		long long int Multi = 8000;
		long long int dive  = ticksIntervalInMilSec;

		// test
//		Acc  = (long long int)554805;
//		dive = (long long int)0xfa0;//0xFFFFFF2D;//4294967085;
		// ens test values
//		actualRate = GetAverageRate(mediaBytes,ticksIntervalInMilSec,8000);
		actualRate = (DWORD)((Acc*Multi)/dive);

		// get system.cfg values
		std::string key = "IP_SET_ACTUAL_BITRATE_TO_FLOAT";
		BOOL data;
		sysConfig->GetBOOLDataByKey(key, data);

		if ( !data &&
			(capCode != eG7231CapCode)		&&
			(capInfo.GetCapType() == cmCapAudio))
				RoundAudioRate(actualRate,rate*_K_);

		strBase1<<" ,A mediaBytes = "<<mediaBytes<<" ,Actual Rate = "<<actualRate;
		strBase1<<" ,Channel index = "<<channelIndex;

		// get system.cfg values
		key =  "ENABLE_IP_PARTY_MONITORING";
		sysConfig->GetBOOLDataByKey(key, data);

		if( data != NO)
			FPTRACE2(eLevelInfoNormal,"UpdatePartyMonitoring: ", strBase1.GetString());
	}

	SetPartyMonitorBaseParams((CPrtMontrBaseParams *)pPrtMonitrParams,channelType,actualRate,partyAddr,mcuAddr,
		(DWORD)capInfo.GetIpCapCode(),channelIndex,IsIce,IcePartyAddr,IceMcuAddr,IceConnectionType);


	if((pChannelMonitoring->unChannelType == kIpAudioChnlType) ||
		(pChannelMonitoring->unChannelType == kIpVideoChnlType)||
		(pChannelMonitoring->unChannelType == kIpContentChnlType)||
		(pChannelMonitoring->unChannelType == kIpFeccChnlType))
	{
		CPrtMontrGlobalParams *pPtrGlobalParam = new CPrtMontrGlobalParams;
		CRtpInfo *pRtpInfo = NULL;

		if(pChannelMonitoring->unChannelType == kIpAudioChnlType)
		{
			DWORD framesPerPacket		= ((TRtpAudioChannelMonitoring *)pChannelMonitoring)->unFramesPerPacket;
			CBaseCap *pCap				= CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE*)pChannelParams);
			DWORD chanFramePerPacket = 0;
			if (pCap)
			    chanFramePerPacket = pCap->GetFramePerPacket();
			else
			    FPTRACE(eLevelError,"UpdatePartyMonitoring - pCap is NULL");
			TAdvanceMonitoringResultsSt *pAdvanceMonitoring = &((TRtpAudioChannelMonitoring *)pChannelMonitoring)->tAdvanceMonitoringResultsSt;
			SetPartyMonitorParams(pPtrGlobalParam,pAdvanceMonitoring);

			if(channelType == AUDIO_IN)
			{
				pRtpInfo = new CRtpInfo;
				SetRtpInfo(pRtpInfo,pAdvanceMonitoring);
				((CAdvanceAudioIn *)pPrtMonitrParams)->SetRtpInfo(*pRtpInfo);
				POBJDELETE(pRtpInfo);
			}
			((CAdvanceAudio *)pPrtMonitrParams)->SetGlobalParam(*pPtrGlobalParam);
			((CAdvanceAudio *)pPrtMonitrParams)->SetFramePerPacket(framesPerPacket);
			((CAdvanceAudio *)pPrtMonitrParams)->CheckExceedingFieldsRules(rate,capInfo.GetIpCapCode(),chanFramePerPacket,strExcFieldsRules);

			POBJDELETE(pCap);

		}//end audio
		else if((pChannelMonitoring->unChannelType == kIpVideoChnlType) || (pChannelMonitoring->unChannelType == kIpContentChnlType))
		{
			DWORD	annexes		= ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unAnnexes;
			int		resolution	= ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unVideoResolution;
			int		maxResolution	= ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unVideoMaxResolution;
			int		minResolution	= ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unVideoMinResolution;
			//int		frameRate	= ((H323_VIDEO_CHANNEL_MONITORING_S *)pChannelMonitoring)->frames;
			TAdvanceMonitoringResultsSt *pAdvanceMonitoring = &((TRtpVideoChannelMonitoring *)pChannelMonitoring)->tAdvanceMonitoringResultsSt;

			CBaseVideoCap *pCap = (CBaseVideoCap *)CBaseCap::AllocNewCap((CapEnum)capInfo,(BYTE*)pChannelParams);

			if (!pCap)
			{
			    FPTRACE(eLevelError,"UpdatePartyMonitoring - pCap is NULL - return..");
			    PDELETE(pPtrGlobalParam);
			    return;
			}

			DWORD  chanAnnexes = 0;
			int		chanResolution;
			DWORD	chanFramRate;

			BYTE	bIsChanContainFormat = pCap->IsFormat((EFormat)resolution);
			//In case the channel resolution contain the resolution that monitoring reported we can initialize the
			//chanResolution as the monitoring resolution.
			if(bIsChanContainFormat)
				chanResolution = resolution;
			else
				chanResolution = pCap->GetFormat();

			chanFramRate = pCap->GetFormatMpi((EFormat)chanResolution);

			if(capCode == eH263CapCode)
				chanAnnexes = ((CH263VideoCap *)pCap)->GetAnnexes();

			FPTRACE2INT(eLevelError,"N.A. DEBUG  UpdatePartyMonitoring - pChannelMonitoring->unChannelType", pChannelMonitoring->unChannelType);
			SetPartyMonitorParams(pPtrGlobalParam,pAdvanceMonitoring);

			if((channelType == VIDEO_IN) || (channelType == VIDEO_CONT_IN))
			{
				FPTRACE(eLevelError,"N.A. DEBUG  UpdatePartyMonitoring - if((channelType == VIDEO_IN) || (channelType == VIDEO_CONT_IN))");
				CAdvanceVideoIn *pPrtVideoIn = (CAdvanceVideoIn *)pPrtMonitrParams;
				pRtpInfo = new CRtpInfo;
				SetRtpInfo(pRtpInfo,pAdvanceMonitoring);
				pPrtVideoIn->SetRtpInfo(*pRtpInfo);
				SetAdvanceInfo(pPrtVideoIn->GetErrorResilience(),
								pAdvanceMonitoring->tRtpStatisticSt.tErrorResilienceRepairs,
								pAdvanceMonitoring);
				PDELETE(pRtpInfo);
			}

			((CAdvanceVideo *)pPrtMonitrParams)->SetGlobalParam(*pPtrGlobalParam);
			((CAdvanceVideo *)pPrtMonitrParams)->SetFrameRate(frameRate);
			((CAdvanceVideo *)pPrtMonitrParams)->SetMaxFrameRate(((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unMaxFrameRate);
			((CAdvanceVideo *)pPrtMonitrParams)->SetMinFrameRate(((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unMinFrameRate);
			((CAdvanceVideo *)pPrtMonitrParams)->SetAnnexes(annexes);
			((CAdvanceVideo *)pPrtMonitrParams)->SetResolution(resolution);
			((CAdvanceVideo *)pPrtMonitrParams)->SetMaxResolution(maxResolution);
			((CAdvanceVideo *)pPrtMonitrParams)->SetMinResolution(minResolution);

			APIU32 width  = ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unVideoWidth;
			APIU32 maxWidth  = ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unVideoMaxWidth;
			APIU32 minWidth  = ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unVideoMinWidth;
			APIU32 height = ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unVideoHeight;
			APIU32 maxHeight = ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unVideoMaxHeight;
			APIU32 minHeight = ((TRtpVideoChannelMonitoring *)pChannelMonitoring)->unVideoMinHeight;
			((CAdvanceVideo *)pPrtMonitrParams)->SetResolutionWidth(width);
			((CAdvanceVideo *)pPrtMonitrParams)->SetMaxResolutionWidth(maxWidth);
			((CAdvanceVideo *)pPrtMonitrParams)->SetMinResolutionWidth(minWidth);
			((CAdvanceVideo *)pPrtMonitrParams)->SetResolutionHeight(height);
			((CAdvanceVideo *)pPrtMonitrParams)->SetMaxResolutionHeight(maxHeight);
			((CAdvanceVideo *)pPrtMonitrParams)->SetMinResolutionHeight(minHeight);

			((CAdvanceVideo *)pPrtMonitrParams)->CheckExceedingFieldsRules(rate,capInfo.GetIpCapCode(),
				chanAnnexes,chanResolution,chanFramRate,strExcFieldsRules);
			POBJDELETE(pCap);
		} //end video
		else if(pChannelMonitoring->unChannelType == kIpFeccChnlType)
		{
			TAdvanceMonitoringResultsSt *pAdvanceMonitoring = &((TRtpFeccChannelMonitoring *)pChannelMonitoring)->tAdvanceMonitoringResultsSt;
			SetPartyMonitorParams(pPtrGlobalParam,pAdvanceMonitoring);
			if(channelType == FECC_IN)
			{
				CAdvanceFeccIn *pPrtFeccIn	= (CAdvanceFeccIn *)pPrtMonitrParams;
				TRtpStatisticSt tRtpStatisticSt	= pAdvanceMonitoring->tRtpStatisticSt;

				SetAdvanceInfo(pPrtFeccIn->GetRtpPacketLoss(),tRtpStatisticSt.tPacketsActualLoss,pAdvanceMonitoring);
				SetAdvanceInfo(pPrtFeccIn->GetRtpOutOfOrder(),tRtpStatisticSt.tPacketsOutOfOrder,pAdvanceMonitoring);
				SetAdvanceInfo(pPrtFeccIn->GetRtpFragmentPackets(),tRtpStatisticSt.tPacketsFragmented,pAdvanceMonitoring);
			}
			((CAdvanceFeccIn *)pPrtMonitrParams)->SetGlobalParam(*pPtrGlobalParam);
			((CAdvanceFeccIn *)pPrtMonitrParams)->CheckExceedingFieldsRules(rate,capInfo.GetIpCapCode(),strExcFieldsRules);
		}

		//In case of any problem print the problem
		if(pPrtMonitrParams->GetProblem())
		{
			CMedString strBase;
//			strBase<<"pmIndex = "<<channelIndex <<" ,mcms index = "<< pChannelMonitoring->mcChannelIndex <<" ,Call index " << callIndex;
			strBase<<"Channel Type = "<< pChannelMonitoring->unChannelType <<" , Channel Direction = "<< pChannelMonitoring->unChannelDirection ;
			FPTRACE2(eLevelInfoNormal,"UpdatePartyMonitoring: Problem details: ", strBase.GetString());
			FPTRACE(eLevelInfoNormal,strExcFieldsRules.GetString());
		}

		// get system.cfg values
		std::string key = "IP_MONITORING_SHOW_FAULTY_MARK";
		BOOL data = NO;
		sysConfig->GetBOOLDataByKey(key, data);

		if(!data)
			pPrtMonitrParams->ClearMapProblem();

		POBJDELETE(pPtrGlobalParam);
	}
}

/////////////////////////////////////////////////////////////////////////////
const char* GetMediaLineSubTypeStr(eMediaLineSubType subType)
{
	switch(subType)
	{
		case eMediaLineSubTypeNull		: return "null";
		case eMediaLineSubTypeUnknown	: return "UnKnown";
		case eMediaLineSubTypeRtpAvp	: return "RtpAvp";
		case eMediaLineSubTypeRtpSavp	: return "RtpSavp";
		case eMediaLineSubTypeTcpBfcp	: return "TcpBfcp";
		case eMediaLineSubTypeTcpTlsBfcp: return "TcpTls";
		case eMediaLineSubTypeUdpBfcp	: return "UdpBfcp";

		case eMediaLineSubTypeTcpRtpAvp : return "TcpRtpAvp";
		case eMediaLineSubTypeTcpRtpSavp: return "cpRtpSavp";
		case eMediaLineSubTypeTcpCfw    : return "TcpCfw";

		default: return "";
	}
}

/////////////////////////////////////////////////////////////////////////////
const char* GetResourceLevelStr(RelayResourceLevelUsage resourceLevel)
{
    switch(resourceLevel)
    {
        case eResourceLevel_CIF     : return "eResourceLevel_CIF";
        case eResourceLevel_SD      : return "eResourceLevel_SD";
        case eResourceLevel_HD720   : return "eResourceLevel_HD720";
        case eResourceLevel_HD1080  : return "eResourceLevel_HD1080";
        default: return "";
    }
}

/////////////////////////////////////////////////////////////////////////////
const char* GetBfcpSetupStr(eBfcpSetup setup)
{
	switch(setup)
	{
		case bfcp_setup_null	: return "null";
		case bfcp_setup_actpass	: return "actpass";
		case bfcp_setup_active	: return "active";
		case bfcp_setup_passive	: return "passive";
		case bfcp_setup_holdconn: return "holdconn";
		default					: return "";
	}
}

const char* GetBfcpConnectionStr(eBfcpConnection connection)
{
	switch(connection)
	{
		case bfcp_connection_null	 : return "null";
		case bfcp_connection_new	 : return "new";
		case bfcp_connection_existing: return "existing";
		default						 : return "";
	}
}

const char* GetBfcpFloorCtrlStr(eBfcpFloorCtrl floorcntl)
{
	switch(floorcntl)
	{
		case bfcp_flctrl_null: return "null";
		case bfcp_flctrl_c_s: return "c_s";
		case bfcp_flctrl_c_only: return "c_only";
		case bfcp_flctrl_s_only: return "s_only";
		default: return "";
	}
}

/////////////////////////////////////////////////////////////////////////////
//  class CQoS - QualityOfService data

/////////////////////////////////////////////////////////////////////////////
CQoS::CQoS() : m_bIpAudio(0x0), m_bIpVideo(0x0), m_bIpRtcp(0x0), m_bIpSignaling(0x0),
m_bAtmAudio(H323_ATM_QOS_CBR), m_bAtmVideo(H323_ATM_QOS_CBR), m_audioFromService(0x0), m_videoFromService(0x0)
{
	if (IsDynamicRPriorityEnabled())
	{
		//=======================================
		// Get configured tos values as default
		//=======================================
		const CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		DWORD videoTos;
		DWORD audioTos;

		pSysConfig -> GetHexDataByKey("QOS_IP_AUDIO", audioTos);
		pSysConfig -> GetHexDataByKey("QOS_IP_VIDEO", videoTos);
		m_bIpAudio = (BYTE)audioTos;
		m_bIpVideo = (BYTE)videoTos;
	}
}
/////////////////////////////////////////////////////////////////////////////
CQoS::CQoS(const QOS_S& rQos) :
	m_bIpAudio(0x0),
	m_bIpVideo(0x0),
	m_bIpRtcp(0x0),
	m_bIpSignaling(0x0),
	m_bAtmAudio(H323_ATM_QOS_CBR),
	m_bAtmVideo(H323_ATM_QOS_CBR),
	m_audioFromService(0x0),
	m_videoFromService(0x0)
{
	AssembleValFromPrecedence(rQos);
}
BOOL CQoS::IsDynamicRPriorityEnabled() const
{
    const CPrecedenceSettings* pPrecedenceSettingsDB = GetpPrecedenceSettingsDB();
    BOOL ret = FALSE;
    if (pPrecedenceSettingsDB && pPrecedenceSettingsDB -> IsPrecedenceEnabled())
    {
		PTRACE(eLevelInfoNormal, "CQoS::IsDynamicRPriorityEnabled - Dynamic RPriority is ENABLED.");
    	ret = TRUE;
    }
    else
    {
		PTRACE(eLevelInfoNormal, "CQoS::IsDynamicRPriorityEnabled - Dynamic RPriority is DISABLED.  Should use defaults");
    }
    return ret;
}
void  CQoS::AssembleValFromRPrio(const char* szPrecedDomain, const BYTE precedRPrio)
{
	CPrecedenceSettings* pPrecedenceSettingsDB = GetpPrecedenceSettingsDB();
	CMedString log;
	log << "CQoS::AssembleValFromRPrio - RPriority[" << precedRPrio << "], Domain[" << szPrecedDomain << "]";
	PTRACE(eLevelInfoNormal, log.GetString());
	if (IsDynamicRPriorityEnabled() && pPrecedenceSettingsDB)
	{
		int domainId = pPrecedenceSettingsDB -> GetDomainId(szPrecedDomain);
		int precedLevel = pPrecedenceSettingsDB -> GetPrecedenceLevelForRPrio(domainId, precedRPrio);
		m_bIpSignaling = pPrecedenceSettingsDB -> GetSignalingDSCP() << 2;
		PTRACE2INT(eLevelInfoNormal, "CQoS::AssembleValFromRPrio - Signaling DSCP is ", m_bIpSignaling);
		CSmallString log;
		log << "CQoS::AssembleValFromRPrio - domainId[" << domainId << "], precedLevel[" << precedLevel << "]";
		PTRACE(eLevelInfoNormal, log.GetString());
		if (domainId != NUM_SINGLE_DOMAINS && precedLevel != NUM_PRECEDENCE_LEVELS)
		{
			const SingleDomainSettings& rDomain = pPrecedenceSettingsDB -> GetSingleDomain(domainId);
			const PRECEDENCE_LEVEL& rPrecedLevel = rDomain.m_PrecedenceLevels[precedLevel];
			CSmallString log;
			log << 	"CQoS::AssembleValFromRPrio - Matched priority, DSCP values: video [" << rPrecedLevel.video_dscp <<
					"]   audio [" << rPrecedLevel.audio_dscp << "]";
			PTRACE(eLevelInfoNormal, log.GetString());
			m_bIpAudio = rPrecedLevel.audio_dscp << 2;
			m_bIpVideo = rPrecedLevel.video_dscp << 2;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CQoS::AssembleValFromPrecedence(const QOS_S& rQos)
{
	if(rQos.m_eIpStatus == YES )
	{
		if (rQos.m_bIpIsDiffServ)
		{
			if (CProcessBase::GetProcess()->GetProductType() != eProductTypeSoftMCUMfw)
			{
				CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
				// get media Qos from SYSTEM.CFG
				m_bIpAudio = (BYTE)(GetSystemCfgFlagHex<DWORD>("QOS_IP_AUDIO"));
				m_bIpVideo = (BYTE)(GetSystemCfgFlagHex<DWORD>("QOS_IP_VIDEO"));

			}
			else //In MFW, the DSCP values are taken from the IP Service
			{
                m_bIpAudio = (BYTE)(rQos.m_bIpPrecedenceAudio) << 2;
                m_bIpVideo = (BYTE)(rQos.m_bIpPrecedenceVideo) << 2;
			}
		}

		else // !(rQos.m_bIpIsDiffServ)
		{
			// get media precedence from service
			m_bIpAudio = (BYTE)( ((rQos.m_bIpPrecedenceAudio << 5) | (rQos.m_bIpValueTOS << 1)) );
			m_bIpVideo = (BYTE)( ((rQos.m_bIpPrecedenceVideo << 5) | (rQos.m_bIpValueTOS << 1)) );
		}
	} // end if (rQos.m_eIpStatus == YES )
}
/////////////////////////////////////////////////////////////////////////////
void CQoS::AdjustToService(const QOS_S& qos)
{
	//===================================
	// Deduce required media TOS values
	//===================================
	AssembleValFromPrecedence(qos);

	//======================================================================================
	// Record previous decisions so service configuration will have priority on TOS values
	//======================================================================================
	m_audioFromService = m_bIpAudio;
	m_videoFromService = m_bIpVideo;
	CSmallString log;
	log << "CQoS::AdjustToService - Audio[" << m_audioFromService << "], Video[" << m_videoFromService << "]";
	PTRACE(eLevelInfoNormal, log.GetString());
}
/////////////////////////////////////////////////////////////////////////////
/*
CQoS::CQoS(QOS_S rQos) :
	m_bIpAudio(rQos.m_bIpPrecedenceAudio),
	m_bIpVideo(rQos.m_bIpPrecedenceVideo),
	m_bIpRtcp(0x0),
	m_bIpSignaling(0x0),
	m_bAtmAudio(rQos.m_bAtmPrecedenceAudio),
	m_bAtmVideo(rQos.m_bAtmPrecedenceVideo)
{
}
*/
/////////////////////////////////////////////////////////////////////////////
CQoS::~CQoS()
{
	int i=0;
}

/////////////////////////////////////////////////////////////////////////////
CQoS& CQoS::operator=(const CQoS& other)
{
	if(this != &other)
	{
		m_bIpAudio  		= other.m_bIpAudio;
		m_bIpVideo  		= other.m_bIpVideo;
		m_bIpRtcp   		= other.m_bIpRtcp;
		m_bIpSignaling 		= other.m_bIpSignaling;
		m_bAtmAudio 		= other.m_bAtmAudio;
		m_bAtmVideo 		= other.m_bAtmVideo;
		m_audioFromService	= other.m_audioFromService;
		m_videoFromService	= other.m_videoFromService;
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void  CQoS::DeSerialize(WORD format,CSegment & seg)
{

	DWORD audioFromService	= m_audioFromService;
	DWORD videoFromService	= m_videoFromService;

	if (format == NATIVE)
	{
		WORD  tmp;
		seg >> tmp;
		m_bIpAudio  = (BYTE)tmp;
		seg >> tmp;
		m_bIpVideo  = (BYTE)tmp;
		seg >> tmp;
		m_bIpRtcp   = (BYTE)tmp;
		seg >> tmp;
		m_bAtmAudio = (BYTE)tmp;
		seg >> tmp;
		m_bAtmVideo = (BYTE)tmp;
		seg >> tmp;
		m_bIpSignaling = (BYTE)tmp;
		seg >> tmp;
		m_audioFromService = (BYTE)tmp;
		seg >> tmp;
		m_videoFromService = (BYTE)tmp;
	}

	if (m_bIpAudio == 0 && m_bIpVideo == 0 && (IsDynamicRPriorityEnabled() || audioFromService || videoFromService|| m_audioFromService || m_videoFromService) && CProcessBase::GetProcess())
	{

		//==================================================================================================
		// Under dynamic RPriority scheme, overrun zero tos with configured defaults
		// If it was configured to zero explicitly in precedence table, it will be reverted to zero later.
		//==================================================================================================
		const CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		CMedString log;
		if (IsDynamicRPriorityEnabled() && pSysConfig)
		{
			pSysConfig -> GetHexDataByKey("QOS_IP_AUDIO", audioFromService);
			pSysConfig -> GetHexDataByKey("QOS_IP_VIDEO", videoFromService);
			log << "CQoS::DeSerialize - RPriority enabled";
		}
		else if (m_audioFromService || m_videoFromService)
		{
			audioFromService = m_audioFromService;
			videoFromService = m_videoFromService;
			log << "CQoS::DeSerialize - Deserialized TOS-from-service";
		}
		else
		{
			m_audioFromService = audioFromService;
			m_videoFromService = videoFromService;
			log << "CQoS::DeSerialize - tos values bound to service";
		}

		//=======================================================
		// Preparing default values that can be overriden later
		//=======================================================
		log << ", will use the following as defaults: Audio[" << audioFromService << "], Video[" << videoFromService << "]";
		PTRACE(eLevelInfoNormal, log.GetString());
		m_bIpAudio = (BYTE)audioFromService;
		m_bIpVideo = (BYTE)videoFromService;
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CQoS::Serialize(WORD format,CSegment& seg) const
{
	if (format == NATIVE)
	{
		WORD tmp;
		tmp = m_bIpAudio;
		seg << tmp;
		tmp = m_bIpVideo;
		seg << tmp;
		tmp = m_bIpRtcp;
		seg << tmp;
		tmp = m_bAtmAudio;
		seg << tmp;
		tmp = m_bAtmVideo;
		seg << tmp;
		tmp = m_bIpSignaling;
		seg << tmp;
		tmp = m_audioFromService;
		seg << tmp;
		tmp = m_videoFromService;
		seg << tmp;
	}
}


///////////////////////////////////////////////////////////////////////////////////////
void BuildCapMatrix(ctCapabilitiesStruct* pCap,int numOfAudioCaps, int numOfVideoCaps, int numOfDataCaps)
{
	int numOfCaps = numOfAudioCaps + numOfVideoCaps + numOfDataCaps;
	if (numOfCaps != pCap->numberOfCaps)
	{
		CSmallString str;
		str << "numOfCaps is " << numOfCaps << " pCap->numberOfCaps " << pCap->numberOfCaps;
		DBGFPASSERT (YES);

		FPTRACE2(eLevelError,"::BuildCapMatrix: ",str.GetString());
	}

	// set the matrix to zero and then refill it with the new cap
	CAP_FD_ZERO(&(pCap->altMatrix));

	WORD location = 0;
	capBuffer * pCapBuffer = (capBuffer *) &pCap->caps;
	char      * tempPtr    = (char*)pCapBuffer;

	for(BYTE index=0; index<numOfCaps; index++)
	{
		CCapSetInfo capInfo = (CapEnum)pCapBuffer->capTypeCode;
		CBaseCap * pCurCap  = CBaseCap::AllocNewCap((CapEnum)capInfo,pCapBuffer->dataCap);

		int k = 0;

		if (pCurCap)
		{
			if (pCurCap->IsType(cmCapAudio))
			{
				location = k * numOfCaps + index;
				CAP_FD_SET(location, &(pCap->altMatrix));
			}

			else if (pCurCap->IsType(cmCapVideo))
			{
				if(numOfAudioCaps)
					k++;
				location = k * numOfCaps + index;
				CAP_FD_SET(location, &(pCap->altMatrix));
			}

			else if (pCurCap->IsType(cmCapData))
			{
				if(numOfAudioCaps)
					k++;
				if(numOfVideoCaps)
					k++;
				location = k * numOfCaps + index;
				CAP_FD_SET(location, &(pCap->altMatrix));
			}
		}
		POBJDELETE(pCurCap);

		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
BYTE ConvertCapEnumToReservationProtocol(CapEnum capCode,APIU16 profile)
{
	BYTE protocol = AUTO;
	if (capCode == eH261CapCode)
		protocol = VIDEO_PROTOCOL_H261;
	else if (capCode == eH263CapCode)
		protocol = VIDEO_PROTOCOL_H263;
	else if (capCode == eH264CapCode && profile == H264_Profile_BaseLine)
		protocol = VIDEO_PROTOCOL_H264;
	else if (capCode == eRtvCapCode)
		protocol = VIDEO_PROTOCOL_RTV;
	else if (capCode == eMsSvcCapCode)
		protocol = VIDEO_PROTOCOL_MS_SVC;
	else if (capCode == eVP8CapCode) //N.A. DEBUG VP8
			protocol = VIDEO_PROTOCOL_VP8;
	else if(capCode == eH264CapCode && profile == H264_Profile_High)
		protocol = VIDEO_PROTOCOL_H264_HIGH_PROFILE;

	return protocol;
}

/////////////////////////////////////////////////////////////////////////////
const char* GetPartyStateStr(WORD partyState)
{
	switch (partyState)
	{
		case PARTY_IDLE                  : return "IDLE";
		case PARTY_CONNECTED             : return "CONNECTED";
		case PARTY_DISCONNECTED          : return "DISCONNECTED";
		case PARTY_WAITING_FOR_DIAL_IN   : return "WAITING_FOR_DIAL_IN";
		case PARTY_CONNECTING            : return "CONNECTING";
		case PARTY_DISCONNECTING         : return "DISCONNECTING";
		case PARTY_CONNECTED_PARTIALY    : return "CONNECTED_PARTIALY";
		case PARTY_DELETED_BY_OPERATOR   : return "DELETED_BY_OPERATOR";
		case PARTY_SECONDARY             : return "SECONDARY";
		case PARTY_STAND_BY              : return "STAND_BY";
		case PARTY_CONNECTED_WITH_PROBLEM: return "CONNECTED_WITH_PROBLEM";
		case PARTY_REDIALING             : return "REDIALING";
		default: return "UNKNOWN";
	}
}

///////////////////////////////////////////////////////////////////////////////////
bool IsMrmpOpcode(DWORD opcode)
{
	return (opcode >= CONF_PARTY_MRC_FIRST_OPCODE && opcode <= CONF_PARTY_MRC_LAST_OPCODE);
}

///////////////////////////////////////////////////////////////////////////////////
BYTE GetCodecNumberOfChannels(CapEnum CapCode)
{
	switch (CapCode)
	{
		case eG722Stereo_128kCapCode:
		case eSiren14Stereo_48kCapCode:
		case eSiren14Stereo_56kCapCode:
		case eSiren14Stereo_64kCapCode:
		case eSiren14Stereo_96kCapCode:
		case eSiren22Stereo_64kCapCode:
		case eSiren22Stereo_96kCapCode:
		case eSiren22Stereo_128kCapCode:
		case eSirenLPRStereo_64kCapCode:
		case eSirenLPRStereo_96kCapCode:
		case eSirenLPRStereo_128kCapCode:
		case eG719Stereo_64kCapCode:
		case eG719Stereo_96kCapCode:
		case eG719Stereo_128kCapCode:
		case eSirenLPRStereo_Scalable_64kCapCode:
		case eSirenLPRStereo_Scalable_96kCapCode:
		case eSirenLPRStereo_Scalable_128kCapCode:
			return AUDIO_STEREO_NUM_OF_CHANNELS;

		default:
			return AUDIO_MONO_NUM_OF_CHANNELS;
	}

	return AUDIO_MONO_NUM_OF_CHANNELS;
}

///////////////////////////////////////////////////////////////////////////////////////
////// Moved from SIP to serve H.323 as well.
////// YP.Cao: Get the System Flag and convert into [0, 4*N,  4<N<75]
////// return value: [0, 4*N,  4<N<75]
DWORD  GetMediaDetectionTimer(const std::string& key)
{
	DWORD tTimeLen= 0;
	DWORD tValue=0;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	FPASSERT_AND_RETURN_VALUE(!pSysConfig, 0);

	pSysConfig->GetDWORDDataByKey(key, tValue);

	//SRS Reuirement 1: 0-14, disabled
	//SRS Reuirement 2: the timer use 4 seconds units, so the timer may be rounded up
	if( 14 < tValue && 300 >= tValue)
	{
		DWORD remaining = (tValue%4)?1:0;
		tTimeLen = (tValue/4 + remaining )*4;
	}
	if(tTimeLen > 300 || (0 < tTimeLen && 16 >  tTimeLen))
	{
		FPTRACE2INT( eLevelInfoNormal,"GetMediaDetectionTimer - Time length is out of range(0, 16-300): ", tTimeLen);
		tTimeLen = 0;
	}
	return tTimeLen;
}
const DWORD stMediaDetectionInfo::MinimumMuteDuration = 20;

stMediaDetectionInfo::__MediaDetectionInfo(): detectTimeLen(0), hasVideo(0), isSVCOpened(0)
{
	memset(timeoutStarted,	0,	sizeof(timeoutStarted));
	memset(prevIndTime, 	0, 	sizeof(prevIndTime));
	memset(lastIndTime, 	0, 	sizeof(lastIndTime));
	memset(inTimeout, 		0, 	sizeof(inTimeout));
}

//============================================================================================================
// Optional handling when receiving a media disconnection indication from embedded -
// checking if we consider this media in timeout (in which case, for instance, a video cell may be removed).
// PLEASE KEEP THIS IMPLEMENTATION SEPARATE FROM THE CALL DISCONNECTION IMPLEMENTATION OF THIS STRUCT.
//============================================================================================================
bool stMediaDetectionInfo::CheckForMediaTimeout(int connId)
{
	//===============================
	// General configuration checks
	//===============================
	CSysConfig* pSysConfig 		= CProcessBase::GetProcess() ? CProcessBase::GetProcess()->GetSysConfig() : NULL;
	const std::string muteKey 	= CFG_KEY_REMOVE_EP_FROM_LAYOUT_ON_NO_VIDEO_TIMER;
	const std::string unmuteKey	= CFG_KEY_RETURN_EP_TO_LAYOUT_ON_NO_VIDEO_TIMER;
	DWORD muteDuration;
	DWORD unmuteDuration;
	FPASSERT_AND_RETURN_VALUE(connId >= MEDIA_DETECTION_MAX_CONNECTION || !pSysConfig || !pSysConfig->GetDWORDDataByKey(muteKey, muteDuration) || !pSysConfig->GetDWORDDataByKey(unmuteKey, unmuteDuration), 0);

	time_t currentMediaDisconnectionTime = time((time_t*)NULL);
	CSmallString log;
	log << "stMediaDetectionInfo::CheckForMediaTimeout - connId[" << connId << "] currentMediaDisconnectionTime[" << currentMediaDisconnectionTime << "]";
	FPTRACE(eLevelInfoNormal,log.GetString());

	if (!inTimeout[connId] && unmuteDuration && muteDuration >= MinimumMuteDuration)
	{
		if (timeoutStarted[connId] && prevIndTime[connId])
		{
			//====================================================================================================================================
			// If previous disconnection happened X seconds ago, where X is more than the unmute duration, it means we had media recently,
			// so the media will not be considered in timeout (and for instance, the video cell will not be removed), if however we got frequent
			// media disconnections for the entire MUTE duration, it means that media is off for enough time to be considered in timeout.
			//====================================================================================================================================
			log.Clear();
			log << "stMediaDetectionInfo::CheckForMediaTimeout - timeoutStarted[" << timeoutStarted[connId] << "] prevIndTime[" << prevIndTime[connId] << "]";
			FPTRACE(eLevelInfoNormal,log.GetString());


			//===========================================================================================
			// Checking whether to unmute at least at the frequency emb will send the indications to us
			//===========================================================================================
			unmuteDuration = max(unmuteDuration, detectTimeLen + 2);
			if (currentMediaDisconnectionTime - prevIndTime[connId] >= (time_t) unmuteDuration)
			{
				//==========================================================================================
				// Too much time between two consecutive disconnections means that we had media in between
				//==========================================================================================
				log.Clear();
				log << "stMediaDetectionInfo::CheckForMediaTimeout - no media disconnections in the past " << unmuteDuration << " seconds, restarting media disconnection check";
				FPTRACE(eLevelInfoNormal,log.GetString());
				timeoutStarted[connId] = currentMediaDisconnectionTime;
			}
			else if (currentMediaDisconnectionTime - timeoutStarted[connId] >= (time_t)muteDuration)
			{
				//======================================================================================================================================
				// Enough time between initial media disconnection and current one, without media returning in between - media is considered in timeout
				//======================================================================================================================================
				log.Clear();
				log << "stMediaDetectionInfo::CheckForMediaTimeout - media disconnections consistent for more than " << muteDuration << " seconds ==> media in timeout.";
				FPTRACE(eLevelInfoNormal,log.GetString());
				inTimeout[connId] = true;
			}
			prevIndTime[connId]	= currentMediaDisconnectionTime;
		}
		else
		{
			//==============================
			// Initial media disconnection
			//==============================
			log.Clear();
			log << "stMediaDetectionInfo::CheckForMediaTimeout - timeoutStarted[" << timeoutStarted[connId] << "] prevIndTime[" << prevIndTime[connId] << "] - initial disconnection";
			FPTRACE(eLevelInfoNormal, log.GetString());
			timeoutStarted[connId]	= currentMediaDisconnectionTime;
			prevIndTime[connId]		= currentMediaDisconnectionTime;
		}
	}
	else
	{
		//=================================================================
		// Current state suggest that the mechanism is currently disabled
		//=================================================================
		log.Clear();
		log << "stMediaDetectionInfo::CheckForMediaTimeout - disabled by either muteDuration[" << muteDuration << "], unmuteDuration [" << unmuteDuration << "], or already inTimeout[" << inTimeout[connId] << "]";
		FPTRACE(eLevelInfoNormal, log.GetString());
	}

	return inTimeout[connId];
}

void stMediaDetectionInfo::MediaResumed(int connId)
{
	FPASSERT_AND_RETURN(connId >= MEDIA_DETECTION_MAX_CONNECTION);

	inTimeout[connId] = false;
	prevIndTime[connId] = 0;
	timeoutStarted[connId] = 0;
}

