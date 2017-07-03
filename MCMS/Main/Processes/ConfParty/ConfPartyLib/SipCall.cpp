//+========================================================================+
//                            SipCall.cpp                                  |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SipCall.cpp                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/11/05   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#include "Macros.h"
#include "NStream.h"
#include "DataTypes.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "Segment.h"
//#include "IpCommonTypes.h"
#include "IpAddressDefinitions.h"
#include "IpChannelParams.h"
#include "IPUtils.h"
#include "IpScm.h"
#include "CapClass.h"
#include "Capabilities.h"
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "SipCaps.h"
#include "SipScm.h"
#include "SipCall.h"
#include "H264.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "EnumsToStrings.h"

/////////////////////////////////////
CSipCall::CSipCall()
{
	m_mcmsConId = 0;
	m_cardIndex = 0;
	m_pCallLegHeaders = NULL;
	m_pRmtHeaders	  = NULL;
	m_pCdrPrivateHeaders = NULL;
	m_eConnectionState = kDisconnected;
	m_bIsCallInitiator = NO;
	m_bIsCloseInitiator = NO;
	m_bIsViolentClose = NO;
	m_bIsReInviteInitiator = NO;
	m_eRejectReason = (enSipCodes) 0;
	m_eWarning = (enSipWarningCodes) 0;
	m_warningStr = NULL;
	m_eCancelType = kNoCancel;
	m_forwardAddrStr = NULL;

	SetCallIndex(0);
	m_numOfChannels = 0;
	m_numOfChannelsEx=0;
	for (int i = 0;i < MAX_SIP_CHANNELS; i++)
	{
		m_channelsPtrArr[i] = NULL;
	}
	for (int i = 0;i < MAX_INTERNAL_CHANNELS; i++)
	{
		m_channelsPtrArrEx[i] = NULL;
	}
	// Media IP mismatch
	memset(&m_DnsProxyIpAddressArray, 0, (sizeof(mcTransportAddress)*TOTAL_NUM_OF_IP_ADDRESSES));
	memset(&m_lprCapStruct,   0, (sizeof(lprCapCallStruct)*NUM_OF_LPR_CAPS));

	m_bIsTipCall = FALSE;
	m_confMediaType = eConfMediaType_dummy;
	m_bIsBfcpSupported = 0;
	
}

/////////////////////////////////////
void  CSipCall::SetConfMediaType(eConfMediaType aConfMediaType) 
{
	TRACEINTO << "type= " << ConfMediaTypeToString(aConfMediaType); 
	m_confMediaType = aConfMediaType;
}

/////////////////////////////////////
CSipCall::~CSipCall()
{
	POBJDELETE(m_pCallLegHeaders);
	PDELETEA(m_pRmtHeaders);
	POBJDELETE(m_pCdrPrivateHeaders);
	PDELETEA(m_forwardAddrStr);
	PDELETEA(m_warningStr);

	for (int i = 0;i < MAX_SIP_CHANNELS; i++)
	{
		POBJDELETE(m_channelsPtrArr[i]);
	}

    for (int i = 0; (i < MAX_INTERNAL_CHANNELS); i++)
    {
        if (m_channelsPtrArrEx[i])
        {
            POBJDELETE(m_channelsPtrArrEx[i]);
        }
    }

}

/////////////////////////////////////
void CSipCall::SetRejectReason(enSipCodes eReason)
{
	m_eRejectReason = eReason;
}
/////////////////////////////////////
EConnectionState CSipCall::GetChannelConnectionState(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole) const
{
	EConnectionState rVal = kUnknown;
	CSipChannel* pChannel = NULL;
	pChannel = GetChannel(true, eMediaType, eDirection, eRole);
	if(pChannel)
		rVal = pChannel->GetConnectionState();
	return rVal;
}

CSipChannel* CSipCall::GetChannel(bool aIsExternal, cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole, DWORD aRtpConnectionId,bool addChn) const
{
	int max;
	CSipChannel* pChannel      = NULL;
	BYTE         bChannelFound = NO;
	CSipChannel* const* channelsPtrArr;

	if (aIsExternal)
	{
	    channelsPtrArr = m_channelsPtrArr;
	    max = MAX_SIP_CHANNELS;
	}
	else
	{
        channelsPtrArr = m_channelsPtrArrEx;
        max = MAX_INTERNAL_CHANNELS;
	}

	for (int i = 0; i < max && bChannelFound == NO; i++)
	{
		if (channelsPtrArr[i] &&
		    channelsPtrArr[i]->m_eMediaType == eMediaType &&
			((channelsPtrArr[i]->m_eRole == eRole) || (channelsPtrArr[i]->m_eRole & eRole)) &&
			(channelsPtrArr[i]->m_eDirection == eDirection) &&
			(aRtpConnectionId == channelsPtrArr[i]->GetRtpConnectionId()))
		{
			pChannel = channelsPtrArr[i];
			bChannelFound = YES;
		}
	}
	
	if (bChannelFound==YES && addChn==TRUE)
	{
		TRACEINTO << "!@# failed trying to add an existing channel aIsExternal=" << aIsExternal << " eMediaType=" << ::GetTypeStr(eMediaType)
		          << " eDirection=" << ::GetDirectionStr(eDirection) << " eRole=" << ::GetRoleStr(eRole) << " aRtpConnectionId=" << aRtpConnectionId;
	}
	
	return pChannel;
}


bool CSipCall::IsInternalChannel(CSipChannel* pChannel)
{
   bool found=false;
   int max;
   max=MAX_INTERNAL_CHANNELS;
   //CSipChannel* pTmpChannel;
   CSipChannel* const* channelsPtrArr = m_channelsPtrArrEx;

   for (int i = 0; i < max && found == false; i++)
   {
      if (channelsPtrArr[i] && pChannel && pChannel== channelsPtrArr[i] )
      {
            found = true;
            TRACEINTO << "Channel found";
      }
  }
  
  return found; 
}

// Description: Get channel by mcms index or card index (or both).
//              If you have only the card index, put NA in mcms index
//---------------------------------------------------------------------------
CSipChannel* CSipCall::GetChannel(DWORD mcmsChannelIndex, DWORD cardChannelIndex) const
{
	CSipChannel* pChannel      = NULL;
	BYTE         bChannelFound = NO;

	// if the given mcms index is available
	if (mcmsChannelIndex != 0xFFFFFFFF)
	{
		// there is a possibility that channel index will always be arrayIndex-1. then there is no need for loop!
		for (int i = 0; i < MAX_SIP_CHANNELS && bChannelFound == NO; i++)
		{
			if (m_channelsPtrArr[i] && m_channelsPtrArr[i]->m_mcmsIndex == mcmsChannelIndex)
			{
				// if the given card index available but it's not matching
				if ((cardChannelIndex != 0xFFFFFFFF) && (m_channelsPtrArr[i]->m_cardIndex != cardChannelIndex))
				{
					CSmallString msg;
					msg << "Mcms index: " << mcmsChannelIndex <<" not match card index " << cardChannelIndex << "\n";
					PTRACE2(eLevelError,"CSipCall::GetChannel: ",msg.GetString());
					PASSERT(cardChannelIndex);
				}
				// else the given card index is not available or it's matching the given mcms index
				else
				{
					pChannel = m_channelsPtrArr[i];
					bChannelFound = YES;
				}
			}
		}
	}

	// else if the given card index is available
	else if (cardChannelIndex != 0xFFFFFFFF)
	{
		for (int i = 0; i < MAX_SIP_CHANNELS && bChannelFound == NO; i++)
		{
			if (m_channelsPtrArr[i] && m_channelsPtrArr[i]->m_cardIndex == cardChannelIndex)
			{
				pChannel = m_channelsPtrArr[i];
				bChannelFound = YES;
			}
		}
	}
	return pChannel;
}


//////////////////////////////////////////////////////////
CSipChannel* CSipCall::GetChannel(int arrIndex, bool aIsExternal) const
{
    CSipChannel* const* channelsPtrArr;
    int maxIndex = 0;
    if (aIsExternal)
    {
        channelsPtrArr = m_channelsPtrArr;
        maxIndex = MAX_SIP_CHANNELS;
    }
    else
    {
        channelsPtrArr = m_channelsPtrArrEx;
        maxIndex = MAX_INTERNAL_CHANNELS;
    }

	CSipChannel* pChannel = ((arrIndex < maxIndex)? channelsPtrArr[arrIndex]: NULL);
	return pChannel;
}

//////////////////////////////////////////////////////////
CSipChannel* CSipCall::GetChannel(EIpChannelType eChanType) const
{
	CSipChannel* pChannel = NULL;

	for (int i=0; i<MAX_SIP_CHANNELS && pChannel==NULL; i++)
	{
		if (m_channelsPtrArr[i] && m_channelsPtrArr[i]->IsChannelType(eChanType))
			pChannel = m_channelsPtrArr[i];
	}

	return pChannel;
}


//////////////////////////////////////////////////////////
DWORD CSipCall::GetCallRate(cmCapDirection eDirection) const
{
	DWORD callRate	= 0;

	CSipChannel* pAudChannel = GetChannel(true, cmCapAudio,eDirection);
	if (pAudChannel)
	{
		CCapSetInfo capInfo = pAudChannel->GetAlgorithm();
		callRate += (capInfo.GetBitRate(pAudChannel->GetData())/100);	// in 100 bit per sec
	}

	CSipChannel* pVidChannel = GetChannel(true, cmCapVideo,eDirection);
	if (pVidChannel)
	{
		CBaseCap* pVidCap = pVidChannel->GetDataAsCapClass();
		if (pVidCap)
			callRate += pVidCap->GetBitRate();
		POBJDELETE(pVidCap);
	}
	return callRate;
}
////////////////////////////////////////////////////////
DWORD CSipCall::GetAudioRate(cmCapDirection eDirection) const
{
	DWORD audioRate	= 0;

	CSipChannel* pAudChannel = GetChannel(true, cmCapAudio,eDirection);
	if (pAudChannel)
	{
		CCapSetInfo capInfo = pAudChannel->GetAlgorithm();
		audioRate += (capInfo.GetBitRate(pAudChannel->GetData())/100);	// in 100 bit per sec
	}
	return audioRate;


}

/////////////////////////////////////////////////////
DWORD CSipCall::GetVideoCallRate(cmCapDirection eDirection) const
{
	DWORD videorate	= 0;
	CSipChannel* pVidChannel = GetChannel(true, cmCapVideo,eDirection);
	if (pVidChannel)
	{
		CBaseCap* pVidCap = pVidChannel->GetDataAsCapClass();
		if (pVidCap)
			videorate = pVidCap->GetBitRate();
		POBJDELETE(pVidCap);
	}
	return videorate;
}


//////////////////////////////////////////////////////////
int	CSipCall::GetNumOfMediaChannels(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole) const
{
	int num = 0;
	if (eDirection & cmCapReceive)
	{
		CSipChannel* pChannel = GetChannel(true, eMediaType,cmCapReceive,eRole);

		if (pChannel && pChannel->m_eConnectionState != kDisconnecting)
			num++;
	}
	if (eDirection & cmCapTransmit)
	{
		CSipChannel* pChannel = GetChannel(true, eMediaType,cmCapTransmit,eRole);

		if (pChannel && pChannel->m_eConnectionState != kDisconnecting)
			num++;
	}
	return num;
}

/////////////////////////////////////////////////////////////////////////////////
BYTE CSipCall::AreAllOpenChannelsConnectionState(EConnectionState eState,BYTE bIsMrcCall,BYTE confType) const
{
	BYTE res = YES;
	EConnectionState channelState = kUnknown;

	for (int i = 0; (i < MAX_SIP_CHANNELS) && (res == YES); i++)
	{
		if (m_channelsPtrArr[i])
		{
			channelState = m_channelsPtrArr[i]->m_eConnectionState;
			if(channelState != eState)
			{
				TRACEINTO << "channel is not in required state; channel " << i 
					<< " state: " << ConnectionStateToString(channelState) 
					<< " (required state: " << ConnectionStateToString(eState) << ")";
			}

			res &= (m_channelsPtrArr[i]->m_eConnectionState == eState)? YES: NO;
		}
	}

	TRACEINTO << "mix_mode: m_confMediaType= " << ConfMediaTypeToString(m_confMediaType);

    if ((eMixAvcSvc == m_confMediaType || m_confMediaType == eMixAvcSvcVsw) && (confType != kCop))
    {
    	for (int i = 0; (i < MAX_INTERNAL_CHANNELS) && (res == YES); i++)
    	{
    		if (m_channelsPtrArrEx[i])
    		{
    			channelState = m_channelsPtrArrEx[i]->m_eConnectionState;

    			if (channelState != eState)
    			{
    				TRACEINTO << "channelEx is not in required state; channel " << i 
						<< " state: " << ConnectionStateToString(channelState)
						<< " (required state: " << ConnectionStateToString(eState) << ")";
    			}

    			res &= (m_channelsPtrArrEx[i]->m_eConnectionState == eState)? YES: NO;
    		}
    	}
    }

	return res;
}

///////////////////////////////////////////////////////////////////////////////////
BYTE CSipCall::AreAllChannelsSameDirection(int arrSize, EIpChannelType chanArr[], cmCapDirection eDirection) const
{
	BYTE res = YES;

	int numOfChannelsSet = 0;
	CSipChannel* pCurChannel = NULL;

	for (int i=0; i<arrSize; i++)
	{
		if (arrSize == MAX_SIP_CHANNELS)
			pCurChannel = GetChannel(i, true);
		else if (chanArr)
			pCurChannel = GetChannel(chanArr[i]);
		else
			break;

		if(pCurChannel)
			res = pCurChannel->IsDirection(eDirection);

		if(res == NO)
			return res;
	}

	return res;
}


//////////////////////////////////////////////////////////
BYTE CSipCall::IsAtLeastOneChannelConnectionState(EConnectionState eState) const
{
	BYTE res = NO;
	for (int i = 0; (i < MAX_SIP_CHANNELS) && (res == NO); i++)
	{
		if ( m_channelsPtrArr[i])
		{
			res = (m_channelsPtrArr[i]->m_eConnectionState == eState)? YES: NO;
		}
	}
    if(eMixAvcSvc == m_confMediaType || eMixAvcSvcVsw == m_confMediaType/* || bIsActiveVswRelay==TRUE*/) /* eyaln9794 */
    {
		for (int i = 0; (i < MAX_INTERNAL_CHANNELS) && (res == NO); i++)
		{
			if ( m_channelsPtrArrEx[i])
			{
				res = (m_channelsPtrArrEx[i]->m_eConnectionState == eState)? YES: NO;
			}
		}
    }
	return res;
}


//////////////////////////////////////////////////////////
int	CSipCall::GetCallLegHeadersLen() const
{
	int len = 0;
	if (m_pCallLegHeaders)
		len = m_pCallLegHeaders->GetTotalLen();
	return len;
}


//////////////////////////////////////////////////////////
int	CSipCall::GetCdrPrivateHeadersLen() const
{
	int len = 0;
	if (m_pCdrPrivateHeaders)
		len = m_pCdrPrivateHeaders->GetTotalLen();
	return len;
}


//////////////////////////////////////////////////////////
int	CSipCall::CalcCapBuffersSize(cmCapDirection eDirection,BYTE bCountMutedChannels,EConnectionState eState) const
{
	int res = 0;
	int capBuffersSize	= 0;
	int	sizeOfICECapBuffer = 0;
	CSipChannel* pChannel = NULL;

	BOOL fAudio 	= FALSE;	// audio media line base size is already counted
	BOOL fVideo 	= FALSE;	// do not count twice the channel of the same type, at the same direction
	BOOL fData 		= FALSE;
	BOOL fContent 	= FALSE;
	BOOL fBfcp		= FALSE;

	for (int i=0; i<MAX_SIP_CHANNELS; i++)
	{
		pChannel = m_channelsPtrArr[i];

		if (pChannel && (pChannel->m_eDirection & eDirection) &&
			((eState == kUnknown && pChannel->m_eConnectionState != kDisconnecting) || pChannel->m_eConnectionState == eState) &&
			!(GetIsTipCall() && (pChannel->GetMediaType()==cmCapVideo) && (pChannel->GetRoleLabel() & kRoleContentOrPresentation)))
		{
			BYTE bMediaMuted = (pChannel->m_bIsMuted && pChannel->m_pSameSessionChannel && pChannel->m_pSameSessionChannel->m_bIsMuted);

			//TRACEINTO << "bMediaMuted:" << (DWORD)bMediaMuted << ", bCountMutedChannels:" << (DWORD)bCountMutedChannels << ", pChannel->GetMediaType():" << (DWORD)pChannel->GetMediaType();

			if (bMediaMuted == NO || bCountMutedChannels == YES) {

				//Add size for sipMediaLineBaseSt
				if (!fAudio && pChannel->GetMediaType() == cmCapAudio) {

					capBuffersSize += sizeof(sipMediaLineBaseSt);
					fAudio = TRUE;
				}
				else if (!fVideo && pChannel->GetMediaType() == cmCapVideo && pChannel->GetRoleLabel() == kRolePeople) {

					capBuffersSize += sizeof(sipMediaLineBaseSt);
					fVideo = TRUE;
				}
				else if (!fData && pChannel->GetMediaType() == cmCapData) {

					capBuffersSize += sizeof(sipMediaLineBaseSt);
					fData = TRUE;
				}
				else if (!fContent && pChannel->GetMediaType() == cmCapVideo && pChannel->GetRoleLabel() & kRoleContentOrPresentation) {

					capBuffersSize += sizeof(sipMediaLineBaseSt);
					fContent = TRUE;
				}
				else if (!fBfcp && pChannel->GetMediaType() == cmCapBfcp) {

					capBuffersSize += sizeof(sipMediaLineBaseSt);
					fBfcp = TRUE;
				}

				capBuffersSize += (sizeof(capBufferBase) + pChannel->m_dataLength);

				//Adding ICE caps size per channel
				for(int j=0;j<pChannel->GetNumOfIceCaps();j++)
				{
					if (j < MAX_MEDIA_CAPSETS)
					{
						sizeOfICECapBuffer	+= (sizeof(capBufferBase) + pChannel->m_IceCapList[j]->capLength);
					}
					else
					{
						PASSERT(j);
					}
				}
				capBuffersSize += sizeOfICECapBuffer;

				if(pChannel->GetSdesCapLen())
					capBuffersSize += (sizeof(capBufferBase) + pChannel->GetSdesCapLen());
			}
		}
	}

	int capabilitiesSize = capBuffersSize;
	res = capBuffersSize? capabilitiesSize: 0;

	//TRACEINTO << "capabilitiesSize:" << (DWORD)capabilitiesSize << ", capBuffersSize:" << (DWORD)capBuffersSize << ", sizeOfICECapBuffer:" << (DWORD)sizeOfICECapBuffer;

	return res;
}

// remote caps -	used to set the opposite direction in a response
//					if not NULL meaning that this is a response.
//---------------------------------------------------------------------------
int	CSipCall::CopyCapBuffersToBuffer(BYTE* buffer,int bufSize,int* pNumOfCaps,const CSipCaps* pRemoteCaps,const CSipCaps* pLastRemoteCaps,cmCapDirection eDirection, BYTE bTakeSdesFromTx, BYTE m_bIsReInviteTransaction, BOOL bAddSdesIfAvailable, EConnectionState eState, APIU16 m_plcmRequireMask, const CSipCaps* pChosenLocalCap) const //_dtls_
{
	BYTE	bCountMutedChannels = pRemoteCaps? YES: NO; // only if we have remote caps we count muted channels
	int		totalSize			= CalcCapBuffersSize(eDirection,bCountMutedChannels,eState);
	int		offsetWrite			= 0;
	int 	iceOffsetWrite      = 0;
	CSipChannel* pChannel	= NULL;
	CSipChannel* pOppositeChannel = NULL;
	*pNumOfCaps	= 0;
	BYTE	bIsSdesEnabled = FALSE;
    BYTE    bIsSdesEnabledAudio = FALSE;
    BYTE    bIsSdesEnabledVideo = FALSE;
    BYTE    bIsSdesEnabledData = FALSE;
    BYTE    bIsSdesEnabledContent = FALSE;
	CSdesCap* pSdesCap = NULL;
	BOOL isSdesCapDynamic = FALSE;

	int	capPosAudio		= 0;
	int	capPosVideo		= 0;
	int	capPosData		= 0;
	int	capPosContent 	= 0;
	int	capPosBfcp 		= 0;

	int	tmpCapPosAudio	= 0;
	int	tmpCapPosVideo	= 0;
	int	tmpCapPosData	= 0;
	int	tmpCapPosContent= 0;
	int	tmpCapPosBfcp 	= 0;

	char bufAudio[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufVideo[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufData[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufContent[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufBfcp[SIP_MEDIA_LINE_BUFFER_SIZE];

	int index = 0;

	sipMediaLineSt *pMediaLineAudio 	= (sipMediaLineSt *) &bufAudio[0];
	sipMediaLineSt *pMediaLineVideo 	= (sipMediaLineSt *) &bufVideo[0];
	sipMediaLineSt *pMediaLineData 		= (sipMediaLineSt *) &bufData[0];
	sipMediaLineSt *pMediaLineContent 	= (sipMediaLineSt *) &bufContent[0];
	sipMediaLineSt *pMediaLineBfcp 		= (sipMediaLineSt *) &bufBfcp[0];

	memset(bufAudio, 0, sizeof(bufAudio));
	memset(bufVideo, 0, sizeof(bufVideo));
	memset(bufData, 0, sizeof(bufData));
	memset(bufContent, 0, sizeof(bufContent));
	memset(bufBfcp, 0, sizeof(bufBfcp));

	cmCapDirection eOppositeDirection = (eDirection == cmCapTransmit) ? cmCapReceive : cmCapTransmit;

	if (totalSize <= bufSize)
	{
		for (int i=0; i<MAX_SIP_CHANNELS; i++)
		{
			pChannel = m_channelsPtrArr[i];

			if (GetIsTipCall() && pChannel && (pChannel->GetMediaType()==cmCapVideo) && (pChannel->GetRoleLabel() & kRoleContentOrPresentation))
				continue;

			if(bTakeSdesFromTx && bAddSdesIfAvailable) //_dtls_
			{
				// if encryption- set sdes from opposite direction channels
				for (int j=0; j<MAX_SIP_CHANNELS; j++)
				{
					pOppositeChannel = m_channelsPtrArr[j];

					if (pChannel && pOppositeChannel && (pChannel->m_eDirection & eDirection) &&
									(pOppositeChannel->m_eDirection & eOppositeDirection) && (pOppositeChannel->GetMediaType() == pChannel->GetMediaType()) &&
									(pOppositeChannel->GetRoleLabel() == pChannel->GetRoleLabel()) &&
									((eState == kUnknown && pOppositeChannel->m_eConnectionState != kDisconnecting) || pOppositeChannel->m_eConnectionState == eState)
									&& pChannel->IsMediaChannel())
					{
						bIsSdesEnabled = pOppositeChannel->IsChannelSdesEnabled();
						TRACEINTO << "bIsSdesEnabled = " << (bIsSdesEnabled? "TRUE" : "FALSE");
						if(bIsSdesEnabled == TRUE) {
							if (isSdesCapDynamic)
							{
								POBJDELETE(pSdesCap);
								isSdesCapDynamic = FALSE;
							}
							pSdesCap = pOppositeChannel->GetChannelSdes();
							if(pSdesCap) {

								pSdesCap->SetStruct(pOppositeChannel->GetMediaType(),cmCapReceiveAndTransmit,pOppositeChannel->GetRoleLabel());
								capBuffer*  pCurCapBuffer		= pSdesCap->GetAsCapBuffer();

								if (pCurCapBuffer)
								{
								    pCurCapBuffer->sipPayloadType	= _Sdes;
								    int			sizeOfCurCapBuffer	= sizeof(capBufferBase) + pOppositeChannel->GetSdesCapLen();

								    if (pOppositeChannel->GetMediaType() == cmCapAudio) {
								        bIsSdesEnabledAudio = TRUE;
								        if (!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,
								                pMediaLineAudio, SIP_MEDIA_LINE_BUFFER_SIZE, tmpCapPosAudio))
								            PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Audio");
								    }
								    else if ((pOppositeChannel->GetMediaType() == cmCapVideo) && (pOppositeChannel->GetRoleLabel() == kRolePeople)) {
                                        bIsSdesEnabledVideo = TRUE;
								        if (!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,
								                pMediaLineVideo, SIP_MEDIA_LINE_BUFFER_SIZE, tmpCapPosVideo))
								            PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Video");
								    }
								    else if (pOppositeChannel->GetMediaType() == cmCapData) {
                                        bIsSdesEnabledData = TRUE;
								        if (!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,
								                pMediaLineData, SIP_MEDIA_LINE_BUFFER_SIZE, tmpCapPosData))
								            PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Data");
								    }
								    else if ((pOppositeChannel->GetMediaType() == cmCapVideo) && (pOppositeChannel->GetRoleLabel() & kRoleContentOrPresentation)) {
                                        bIsSdesEnabledContent = TRUE;
								        if (!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,
								                pMediaLineContent, SIP_MEDIA_LINE_BUFFER_SIZE, tmpCapPosContent))
								            PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Content");
								    }
								    (*pNumOfCaps)++;
								    PDELETEA(pCurCapBuffer);
								}
							}
						}
					}
				}
			}

			if (pChannel && (pChannel->m_eDirection & eDirection) &&
				((eState == kUnknown && pChannel->m_eConnectionState != kDisconnecting) || pChannel->m_eConnectionState == eState))
			{
				BYTE bTakeSdesContentFromReceive = (pChannel->IsChannelType(VIDEO_CONT_IN) && !GetChannel(VIDEO_CONT_OUT)) ? TRUE : FALSE;
				// bTakeSdesContentFromReceive will be true in first reinvite dial-out with content, because we add content only in the reinvite.
				if (bTakeSdesContentFromReceive)
					PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer bTakeSdesContentFromReceive");

				BYTE bTakeSdesFromRxChannel = FALSE;
				if( bAddSdesIfAvailable && pChannel->IsMediaChannel()  && (pChannel->m_eDirection & cmCapReceive) && GetChannel(true, pChannel->GetMediaType(),cmCapTransmit,pChannel->GetRoleLabel()) == NULL)//amirk-rebase
				{
					PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer set  bTakeSdesFromRxChannel to true as there is no Tx channel but there is Rx channel");
					bTakeSdesFromRxChannel = TRUE;
				}


				if (bTakeSdesFromRxChannel || ( (!bTakeSdesFromTx || bTakeSdesContentFromReceive)
									&& bAddSdesIfAvailable && pChannel->IsMediaChannel() ) ) //_dtls_
				{
					bIsSdesEnabled = pChannel->IsChannelSdesEnabled();

					if (bIsSdesEnabled == TRUE)
					{

						int nNUmOfSdesCaps 	= 0;
						int nSdesCapIndex 	= 0;

						if (isSdesCapDynamic)
						{
							POBJDELETE(pSdesCap);
							isSdesCapDynamic = FALSE;
						}
						pSdesCap = pChannel->GetChannelSdes();

						if(pSdesCap)
							nNUmOfSdesCaps = 1;

						if(pChosenLocalCap && pSdesCap && bTakeSdesFromRxChannel)
						{
							pSdesCap = NULL;
							PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer take sdes from local caps");

							//pSdesCap = pChosenLocalCap->GetSdesCap(pChannel->GetMediaType(), pChannel->GetRoleLabel());
							nNUmOfSdesCaps = pChosenLocalCap->GetNumOfSdesMediaCapSets(pChannel->GetMediaType(),cmCapReceiveAndTransmit, pChannel->GetRoleLabel());
							pSdesCap = pChosenLocalCap->GetSdesCap(pChannel->GetMediaType(),pChannel->GetRoleLabel(),nSdesCapIndex );
							if (pSdesCap)
							{
								isSdesCapDynamic = TRUE;
							}
							PTRACE2INT(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer nNUmOfSdesCaps=", nNUmOfSdesCaps);
						}

						//if(pSdesCap)
						while(pSdesCap && nSdesCapIndex < nNUmOfSdesCaps)
						{
							pSdesCap->SetStruct(pChannel->GetMediaType(),cmCapReceiveAndTransmit,pChannel->GetRoleLabel());
							capBuffer*  pCurCapBuffer		= pSdesCap->GetAsCapBuffer();

							if (pCurCapBuffer)
							{
								pCurCapBuffer->sipPayloadType	= _Sdes;
								int			sizeOfCurCapBuffer	= sizeof(capBufferBase) + pChannel->GetSdesCapLen();

								if (pChannel->GetMediaType() == cmCapAudio) {
									bIsSdesEnabledAudio = TRUE;
									if (!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,
											pMediaLineAudio, SIP_MEDIA_LINE_BUFFER_SIZE, tmpCapPosAudio))
										PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Audio");
								}
								else if ((pChannel->GetMediaType() == cmCapVideo) && (pChannel->GetRoleLabel() == kRolePeople)) {
									bIsSdesEnabledVideo = TRUE;
									if (!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,
											pMediaLineVideo, SIP_MEDIA_LINE_BUFFER_SIZE, tmpCapPosVideo))
										PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Video");
								}
								else if (pChannel->GetMediaType() == cmCapData) {
									bIsSdesEnabledData = TRUE;
									if (!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,
											pMediaLineData, SIP_MEDIA_LINE_BUFFER_SIZE, tmpCapPosData))
										PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Data");
								}
								else if ((pChannel->GetMediaType() == cmCapVideo) && (pChannel->GetRoleLabel() & kRoleContentOrPresentation)) {
									bIsSdesEnabledContent = TRUE;
									if (!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,
											pMediaLineContent, SIP_MEDIA_LINE_BUFFER_SIZE, tmpCapPosContent))
										PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Content");
								}
								(*pNumOfCaps)++;
								PDELETEA(pCurCapBuffer);
							}
							else
								PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - pCurCapBuffer is NULL");

							//next sdes
							nSdesCapIndex++;
							if(nSdesCapIndex < nNUmOfSdesCaps)
							{
								if (isSdesCapDynamic)
								{
									POBJDELETE(pSdesCap);
									isSdesCapDynamic = FALSE;
								}
								pSdesCap = pChosenLocalCap->GetSdesCap(pChannel->GetMediaType(),pChannel->GetRoleLabel(), nSdesCapIndex);
								if(pSdesCap)
								{
									totalSize += (sizeof(capBufferBase) + pSdesCap->SizeOf());
									isSdesCapDynamic = TRUE;
								}
							}
						}
					}
				}

				CBaseCap* pCurCap = CBaseCap::AllocNewCap(pChannel->m_eAlgorithm,pChannel->m_pData);

				if (pCurCap)
				{
					BYTE bMediaMuted = (pChannel->m_bIsMuted && pChannel->m_pSameSessionChannel && pChannel->m_pSameSessionChannel->m_bIsMuted);

					// VNGSWIBM-903 - choose if to use pLastRemoteCaps or pRemoteCaps
					const CSipCaps* pRelevantRemoteCap = NULL;
					if ( (pLastRemoteCaps) &&
						 ( pLastRemoteCaps->IsMedia( pChannel->GetMediaType(), cmCapReceiveAndTransmit, pChannel->GetRoleLabel()) ) )
					{
						pRelevantRemoteCap = pLastRemoteCaps;
					}
					else
					{
						pRelevantRemoteCap = pRemoteCaps;
					}

					if (bMediaMuted == NO || (pRelevantRemoteCap && pRelevantRemoteCap->GetNumOfCapSets()))
					{
						cmCapDirection eCapDirection = cmCapReceiveAndTransmit;
						CCapSetInfo capInfo 		 = pChannel->GetAlgorithm();
						APIU8 payloadTypeToDeclare   = pChannel->GetPayloadType();

						// we set the opposite direction of the remote caps if there are
						if (pRelevantRemoteCap && !(pChannel->m_pSameSessionChannel && pChannel->m_pSameSessionChannel->m_bIsMuted) )
						{
							CBaseCap* pRemoteCap = pRelevantRemoteCap->GetCapSet(capInfo,0,pChannel->GetRoleLabel());

							if (pRemoteCap)
							{
								cmCapDirection eRemoteDirection	= pRemoteCap->GetDirection();
								eCapDirection = CalcOppositeDirection(eRemoteDirection);
							}
							POBJDELETE(pRemoteCap);
						}
						else // we use the channel that is mute (if there is) to set direction
						{
							if (pChannel->m_bIsMuted)
							{
								cmCapDirection eOppositeDirection = CalcOppositeDirection(eDirection);
								eCapDirection = eOppositeDirection;
							}
							else if (pChannel->m_pSameSessionChannel && pChannel->m_pSameSessionChannel->m_bIsMuted)
								eCapDirection = pChannel->m_eDirection;
						}

						PTRACE2INT( eLevelInfoNormal,"IS_PREFER_TIP_MODE: CSipCall::CopyCapBuffersToBuffer GetIsTipCall:",GetIsTipCall() );
						if ( pCurCap->GetType() == cmCapVideo &&  pCurCap->GetCapCode() == eH264CapCode  &&
						        pCurCap->GetRole() == kRolePeople &&  pCurCap->GetProfile() == H264_Profile_Main && GetIsTipCall() )
						{
						    PTRACE(eLevelInfoNormal,"IS_PREFER_TIP_MODE: CSipCall::CopyCapBuffersToBuffer change level to H264_Level_4");
							APIS32 mbps, fs, dpb, brAndCpb, sar, staticMB;
							((CH264VideoCap*)pCurCap)->GetAdditionalsAsExplicit(mbps, fs, dpb, brAndCpb, sar, staticMB);
							((CH264VideoCap*)pCurCap)->SetLevelAndAdditionals(H264_Profile_None, H264_Level_4, 
								mbps, fs, dpb, brAndCpb, sar, staticMB);
							((CH264VideoCap*)pCurCap)->RemoveDefaultAdditionals();
						}

						cmCapDirection eCurCapDirection = pCurCap->GetDirection();
						pCurCap->SetDirection(eCapDirection);
						capBuffer*  pCurCapBuffer		= pCurCap->GetAsCapBuffer();
						int			sizeOfCurCapBuffer	= sizeof(capBufferBase) + pChannel->m_dataLength;
						if( pCurCapBuffer )
							pCurCapBuffer->sipPayloadType	= payloadTypeToDeclare;
						else {
							POBJDELETE(pCurCap);
							DBGPASSERT(!pCurCapBuffer);
							//POBJDELETE(pSdesCap); // N.A. Core Dump BRDIGE-14232
							return 0;
						}

						int j=0;
//						else
//					{// the channel direction is receive
//							if(pChannel->GetAlgorithm() == eH263CapCode)
//								if((__FIRST_DPT <= pChannel->m_payloadType) && (pChannel->m_payloadType <= __LAST_DPT))
//									pCurCapBuffer->sipPayloadType = _H263Dynamic;
//						}
						//**********Audio*********//
						if (pChannel->GetMediaType() == cmCapAudio) {
							capPosAudio+= tmpCapPosAudio;
							if(pChannel->GetNumOfIceCaps())
							{//ICE
								PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer In Audio ICE");
								for(j=0;j<pChannel->GetNumOfIceCaps();j++)
								{
									if (j < MAX_MEDIA_CAPSETS)
									{
										int	sizeOfICECapBuffer	= sizeof(capBufferBase) + pChannel->m_IceCapList[j]->capLength;
										PTRACE2INT(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer sizeOfICECapBuffer - ",sizeOfICECapBuffer);

										if(!AddCapInMediaLine(pChannel->m_IceCapList[j], sizeOfICECapBuffer,pMediaLineAudio,SIP_MEDIA_LINE_BUFFER_SIZE, capPosAudio))
											PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "ICE Audio");									}
									else
									{
										PASSERT(j);
									}
								}
							}

							if(!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,pMediaLineAudio,SIP_MEDIA_LINE_BUFFER_SIZE, capPosAudio))
								PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Audio");
						}
						//**********Video*********//
						else if ((pChannel->GetMediaType() == cmCapVideo) && (pChannel->GetRoleLabel() == kRolePeople)){
							capPosVideo+= tmpCapPosVideo;
                            if(pChannel->GetNumOfIceCaps())
							{	//ICE
								PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer In Video ICE");
								for(j=0;j<pChannel->GetNumOfIceCaps() && j < MAX_MEDIA_CAPSETS ;j++)
								{
									int	sizeOfICECapBuffer	= sizeof(capBufferBase) + pChannel->m_IceCapList[j]->capLength;
									PTRACE2INT(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer sizeOfICECapBuffer - ",sizeOfICECapBuffer);

									if(!AddCapInMediaLine(pChannel->m_IceCapList[j], sizeOfICECapBuffer,pMediaLineVideo,SIP_MEDIA_LINE_BUFFER_SIZE, capPosVideo))
										PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "ICE Video");
								}
							}

							if(!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,pMediaLineVideo,SIP_MEDIA_LINE_BUFFER_SIZE, capPosVideo))
								PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Video");
						}
						//**********Data*********//
						else if (pChannel->GetMediaType() == cmCapData) {
							capPosData+= tmpCapPosData;
                            	//ICE
							if(pChannel->GetNumOfIceCaps())
							{
								PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer In Data ICE");
								for(j=0;j<pChannel->GetNumOfIceCaps()  && j < MAX_MEDIA_CAPSETS;j++)
								{
									int	sizeOfICECapBuffer	= sizeof(capBufferBase) + pChannel->m_IceCapList[j]->capLength;
									if(!AddCapInMediaLine(pChannel->m_IceCapList[j], sizeOfICECapBuffer,pMediaLineData,SIP_MEDIA_LINE_BUFFER_SIZE, capPosData))
										PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "ICE Data");
								}
							}

							if(!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,pMediaLineData,SIP_MEDIA_LINE_BUFFER_SIZE, capPosData))
								PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Data");
						}
						//**********Content*********//
						else if ((pChannel->GetMediaType() == cmCapVideo) && (pChannel->GetRoleLabel() & kRoleContentOrPresentation)){
							capPosContent+= tmpCapPosContent;
//							if(pChannel->GetNumOfIceCaps())
//							{	//ICE
//								PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer In Video ICE");
//								for(j=0;j<pChannel->GetNumOfIceCaps();j++)
//								{
//									int	sizeOfICECapBuffer	= sizeof(capBufferBase) + pChannel->m_IceCapList[j]->capLength;
//									PTRACE2INT(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer sizeOfICECapBuffer - ",sizeOfICECapBuffer);
//
//									if(!AddCapInMediaLine(pChannel->m_IceCapList[j], sizeOfICECapBuffer,pMediaLineContent,SIP_MEDIA_LINE_BUFFER_SIZE, capPosContent))
//										PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "ICE Content");
//								}
//							}

							if(!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,pMediaLineContent,SIP_MEDIA_LINE_BUFFER_SIZE, capPosContent))
								PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Content");
						}
						else if ((pChannel->GetMediaType() == cmCapBfcp)) {
							capPosBfcp+= tmpCapPosBfcp;

							if(!AddCapInMediaLine(pCurCapBuffer, sizeOfCurCapBuffer,pMediaLineBfcp,SIP_MEDIA_LINE_BUFFER_SIZE, capPosBfcp))
								PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Bfcp");
						}
						(*pNumOfCaps)++; //For the protocol cap
						(*pNumOfCaps) += pChannel->GetNumOfIceCaps();
						PTRACE2INT(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer - number of caps:",(*pNumOfCaps));
						PDELETEA(pCurCapBuffer);
						pCurCap->SetDirection(eCurCapDirection); // return to the original direction
					}
					else // if both muted and we don't have remote caps, we won't add this media to buffer
						PTRACE2(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer: Media is muted - ",GetTypeStr(pChannel->m_eMediaType));
				}
				else
					DBGPASSERT(YES);
				POBJDELETE(pCurCap);
			}
		}

		BOOL bOverrideSavpWithAvp = FALSE;
		if (m_plcmRequireMask & m_plcmRequireAvp)
		{
			bOverrideSavpWithAvp = TRUE;
			PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer: bOverrideSavpWithAvp == TRUE ");
		}

		if (capPosAudio)
		{
			pMediaLineAudio->index = index;
			pMediaLineAudio->type = eMediaLineTypeAudio;
            TRACEINTO << "bIsSdesEnabledAudio = " << (bIsSdesEnabledAudio? "TRUE" : "FALSE");
            if(bIsSdesEnabledAudio == TRUE) {
      			if ((bOverrideSavpWithAvp))// && (!bTakeSdesFromTx) && (m_bIsReInviteTransaction == FALSE))
      			{
      				PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer: bOverrideSavpWithAvp for audio");
            		pMediaLineAudio->subType = eMediaLineSubTypeRtpAvp;
            	}
            	else
            	{
            		pMediaLineAudio->subType = eMediaLineSubTypeRtpSavp;
            	}
            }
            else
            {
            	pMediaLineAudio->subType = eMediaLineSubTypeRtpAvp;
            }
			pMediaLineAudio->internalType = kMediaLineInternalTypeAudio;
			index++;
			memcpy(buffer + offsetWrite, bufAudio, sizeof(sipMediaLineBaseSt) + capPosAudio);
			offsetWrite += sizeof(sipMediaLineBaseSt) + capPosAudio;
		}
		if (capPosVideo)
		{
			pMediaLineVideo->index = index;
			pMediaLineVideo->type = eMediaLineTypeVideo;
            TRACEINTO << "bIsSdesEnabledVideo = " << (bIsSdesEnabledVideo? "TRUE" : "FALSE");
			if(bIsSdesEnabledVideo == TRUE)
			{
				if ((bOverrideSavpWithAvp))// && (!bTakeSdesFromTx) && (m_bIsReInviteTransaction == FALSE))
				{
					PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer: bOverrideSavpWithAvp for video");
					pMediaLineVideo->subType = eMediaLineSubTypeRtpAvp;
				}
				else
				{
					pMediaLineVideo->subType = eMediaLineSubTypeRtpSavp;
				}
			} else
			{
				pMediaLineVideo->subType = eMediaLineSubTypeRtpAvp;
			}
			pMediaLineVideo->internalType = kMediaLineInternalTypeVideo;
			index++;
			memcpy(buffer + offsetWrite, bufVideo, sizeof(sipMediaLineBaseSt) + capPosVideo);
			offsetWrite += sizeof(sipMediaLineBaseSt) + capPosVideo;
		}
		if (capPosData)
		{
			pMediaLineData->index = index;
			pMediaLineData->type = eMediaLineTypeApplication;
            TRACEINTO << "bIsSdesEnabledData = " << (bIsSdesEnabledData? "TRUE" : "FALSE");
			if(bIsSdesEnabledData == TRUE)
			{
				if ((bOverrideSavpWithAvp))// && (!bTakeSdesFromTx) && (m_bIsReInviteTransaction == FALSE))
				{
					PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer: bOverrideSavpWithAvp for data");
					pMediaLineData->subType = eMediaLineSubTypeRtpAvp;
				}
				else
				{
					pMediaLineData->subType = eMediaLineSubTypeRtpSavp;
				}
			} else
			{
				pMediaLineData->subType = eMediaLineSubTypeRtpAvp;
			}
			pMediaLineData->internalType = kMediaLineInternalTypeFecc;
			index++;
			memcpy(buffer + offsetWrite, bufData, sizeof(sipMediaLineBaseSt) + capPosData);
			offsetWrite += sizeof(sipMediaLineBaseSt) + capPosData;
		}
		if (capPosContent)
		{
			pMediaLineContent->index = index;
			pMediaLineContent->type = eMediaLineTypeVideo;
            TRACEINTO << "bIsSdesEnabledContent = " << (bIsSdesEnabledContent? "TRUE" : "FALSE");
			if(bIsSdesEnabledContent == TRUE)
			{
				if ((bOverrideSavpWithAvp))// && (!bTakeSdesFromTx) && (m_bIsReInviteTransaction == FALSE))
				{
					PTRACE(eLevelInfoNormal,"CSipCall::CopyCapBuffersToBuffer: bOverrideSavpWithAvp for content");
					pMediaLineContent->subType = eMediaLineSubTypeRtpAvp;
				}
				else
				{
					pMediaLineContent->subType = eMediaLineSubTypeRtpSavp;
				}
			} else
			{
				pMediaLineContent->subType = eMediaLineSubTypeRtpAvp;
			}
			pMediaLineContent->internalType = kMediaLineInternalTypeContent;
			index++;
			memcpy(buffer + offsetWrite, bufContent, sizeof(sipMediaLineBaseSt) + capPosContent);
			offsetWrite += sizeof(sipMediaLineBaseSt) + capPosContent;
		}
		if (capPosBfcp)
		{
			pMediaLineBfcp->index = index;
			pMediaLineBfcp->type = eMediaLineTypeApplication;
			pMediaLineBfcp->internalType = kMediaLineInternalTypeBfcp;
			index++;
			memcpy(buffer + offsetWrite, bufBfcp, sizeof(sipMediaLineBaseSt) + capPosBfcp);
			offsetWrite += sizeof(sipMediaLineBaseSt) + capPosBfcp;
		}
	}
	DBGPASSERT(offsetWrite > totalSize);

	//POBJDELETE(pSdesCap); // N.A. Core Dump BRDIGE-14232
	return offsetWrite;
}


// set capabilites in a given struct
//---------------------------------------------------------------------------
int CSipCall::CopyCapsToCapStruct( sipMediaLinesEntrySt* pCapabilities,int structSize,const CSipCaps* pRemoteCaps,const CSipCaps* pLastRemoteCaps,
								   cmCapDirection eDirection, BYTE bTakeSdesFromTx, BYTE m_bIsReInviteTransaction, BOOL bAddSdesIfAvailable,
								   EConnectionState eState, APIU16 m_plcmRequireMask, const CSipCaps* pChosenLocalCap) const //_dtls_
{
	int		res = 0;
	int		numOfAudioCaps		= 0;
	int		numOfVideoCaps		= 0;
	int		numOfDataCaps		= 0;
	int		numOfContentCaps	= 0;
	int		numOfBfcpCaps		= 0;

	BYTE	bCountMutedChannels = pRemoteCaps? YES: NO; // only if we have remote caps we count muted channels

	int capBuffersSize		= CalcCapBuffersSize(eDirection,bCountMutedChannels,eState);
	int capabilitiesSize	= sizeof(sipMediaLinesEntryBaseSt) + capBuffersSize;

	//TRACEINTO << "capBuffersSize:" << (DWORD)capBuffersSize << ", bCountMutedChannels:" << (DWORD)bCountMutedChannels
	//		  << ", capabilitiesSize:" << (DWORD)capabilitiesSize << ", structSize:" << (DWORD)structSize
	//		  << ", sizeof(sipMediaLinesEntryBaseSt):" << (DWORD)sizeof(sipMediaLinesEntryBaseSt);

	// check if there is enough space to set struct
	if (capabilitiesSize <= structSize)
	{

		BYTE bAudioMuted 	= IsChannelMuted(cmCapAudio,cmCapReceiveAndTransmit);
		BYTE bVideoMuted 	= IsChannelMuted(cmCapVideo,cmCapReceiveAndTransmit);
		BYTE bDataMuted 	= IsChannelMuted(cmCapData,cmCapReceiveAndTransmit);
		BYTE bContentMuted 	= IsChannelMuted(cmCapVideo,cmCapReceiveAndTransmit,kRolePresentation);
		BYTE bBfcpMuted		= IsChannelMuted(cmCapBfcp,cmCapReceiveAndTransmit);

		//TRACEINTO << "bAudioMuted:" << (DWORD)bAudioMuted << ", bVideoMuted:" << (DWORD)bVideoMuted
		//		  << ", bDataMuted:" << (DWORD)bDataMuted << ", bContentMuted:" << (DWORD)bContentMuted << ", bBfcpMuted:" << (DWORD)bBfcpMuted;

		// if we have remote caps meaning that this is a response so we don't mind if channels are mute or not
		// we set the response accoring to the request (with opposite directions)
		BYTE bIsRemoteAudio=NO, bIsRemoteVideo=NO, bIsRemoteData=NO, bIsRemoteContent=NO, bIsRemoteBfcp=NO;
		if (pLastRemoteCaps)
		{
			bIsRemoteAudio 		= pLastRemoteCaps->IsMedia(cmCapAudio);
			bIsRemoteVideo		= pLastRemoteCaps->IsMedia(cmCapVideo);
			bIsRemoteData		= pLastRemoteCaps->IsMedia(cmCapData);
			bIsRemoteContent	= pLastRemoteCaps->IsMedia(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
			bIsRemoteBfcp		= pLastRemoteCaps->IsMedia(cmCapBfcp);

			//TRACEINTO << "after pLastRemoteCaps - bIsRemoteAudio:" << (DWORD)bIsRemoteAudio << ", bIsRemoteVideo:" << (DWORD)bIsRemoteVideo
			//		  << ", bIsRemoteData:" << (DWORD)bIsRemoteData << ", bIsRemoteContent:" << (DWORD)bIsRemoteContent << ", bIsRemoteBfcp:" << (DWORD)bIsRemoteBfcp;

		}

		if (pRemoteCaps)
		{
			if (NO == bIsRemoteAudio)
				bIsRemoteAudio = pRemoteCaps->IsMedia(cmCapAudio);

			if (NO == bIsRemoteVideo)
				bIsRemoteVideo = pRemoteCaps->IsMedia(cmCapVideo);

			if (NO == bIsRemoteData)
				bIsRemoteData = pRemoteCaps->IsMedia(cmCapData);

			if (NO == bIsRemoteContent)
				bIsRemoteContent = pRemoteCaps->IsMedia(cmCapVideo,cmCapReceiveAndTransmit,kRolePresentation);

			if (NO == bIsRemoteBfcp)
				bIsRemoteBfcp  = pRemoteCaps->IsMedia(cmCapBfcp);

			//TRACEINTO << "after pRemoteCaps - bIsRemoteAudio:" << (DWORD)bIsRemoteAudio << ", bIsRemoteVideo:" << (DWORD)bIsRemoteVideo
			//		  << ", bIsRemoteData:" << (DWORD)bIsRemoteData << ", bIsRemoteContent:" << (DWORD)bIsRemoteContent << ", bIsRemoteBfcp:" << (DWORD)bIsRemoteBfcp;

		}


		numOfAudioCaps 		= (bAudioMuted && bIsRemoteAudio==NO) 		? 0: GetNumOfMediaChannels(cmCapAudio,eDirection);
		numOfVideoCaps 		= (bVideoMuted && bIsRemoteVideo==NO) 		? 0: GetNumOfMediaChannels(cmCapVideo,eDirection);
		numOfDataCaps 		= (bDataMuted && bIsRemoteData==NO) 		? 0: GetNumOfMediaChannels(cmCapData,eDirection);
		numOfContentCaps 	= (bContentMuted && bIsRemoteContent==NO) 	? 0: GetNumOfMediaChannels(cmCapVideo,eDirection,kRolePresentation);
		numOfBfcpCaps 		= (bBfcpMuted && bIsRemoteBfcp==NO) 		? 0: GetNumOfMediaChannels(cmCapBfcp,eDirection);

		//TRACEINTO << "numOfAudioCaps:" << (DWORD)numOfAudioCaps << ", numOfVideoCaps:" << (DWORD)numOfVideoCaps << ", numOfDataCaps:" << (DWORD)numOfDataCaps
		//		  << ", numOfDataCaps:" << (DWORD)numOfDataCaps << ", numOfContentCaps:" << (DWORD)numOfContentCaps << ", numOfBfcpCaps:" << (DWORD)numOfBfcpCaps;

		PTRACE2INT(eLevelError,"CSipCall::CopyCapsToCapStruct -numOfAudioCaps ",numOfAudioCaps);
		PTRACE2INT(eLevelError,"CSipCall::CopyCapsToCapStruct -bIsRemoteVideo ",bIsRemoteVideo);

		if (GetIsTipCall())
			numOfContentCaps = 0;

		int numOfAudioAlts 		= numOfAudioCaps? 1: 0;
		int numOfVideoAlts 		= numOfVideoCaps? 1: 0;
		int numOfDataAlts 		= numOfDataCaps? 1: 0;
		int numOfContentAlts 	= numOfContentCaps? 1: 0;
		int numOfBfcpAlts 		= numOfBfcpCaps? 1: 0;

		int numOfCapBuffers = 0;
		int offsetWrite = CopyCapBuffersToBuffer((BYTE*)&pCapabilities->mediaLines,capBuffersSize,&numOfCapBuffers,pRemoteCaps,pLastRemoteCaps,eDirection, bTakeSdesFromTx, m_bIsReInviteTransaction, bAddSdesIfAvailable, eState, m_plcmRequireMask ,pChosenLocalCap); //_dtls_
		pCapabilities->numberOfMediaLines = numOfAudioAlts + numOfVideoAlts + numOfDataAlts + numOfContentAlts + numOfBfcpAlts;
		pCapabilities->lenOfDynamicSection = offsetWrite;

		res = sizeof(sipMediaLinesEntryBaseSt) + offsetWrite;
	}
	return res;
}

//////////////////////////////////////////////////////////
mcTransportAddress CSipCall::GetChannelIpAddress(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole) const
{
	mcTransportAddress res;
	memset((void*) &res, 0, sizeof(mcTransportAddress));
	CSipChannel* pChannel = GetChannel(true, eMediaType, eDirection, eRole);

	if (pChannel)
		res = pChannel->GetAddress();

	return res;
}


//////////////////////////////////////////////////////////
DWORD CSipCall::GetChannelRate(cmCapDataType eMediaType, cmCapDirection eDirection, ERoleLabel eRole) const
{
	DWORD rate = 0;

	CSipChannel* pChannel = GetChannel(true, eMediaType, eDirection, eRole);
	if (pChannel)
	{
		CBaseCap* pCap = (CBaseCap*)pChannel->GetDataAsCapClass();
		if (pCap)
		    rate = pCap->GetBitRate();
		else
		    PTRACE(eLevelError,"CSipCall::GetChannelRate - pCap is NULL - can not GetBitRate");
                POBJDELETE(pCap);
	}

	return rate;
}

//////////////////////////////////////////////////////////
void CSipCall::SetChannelRate(DWORD newRate, cmCapDataType eMediaType, cmCapDirection eDirection, ERoleLabel eRole)
{

	CSipChannel* pChannel = GetChannel(true, eMediaType, eDirection, eRole);
	if (pChannel)
	{
		CBaseVideoCap* pVidCap = (CBaseVideoCap*)pChannel->GetDataAsCapClass();
		if (pVidCap)
		{
		    PTRACE2INT(eLevelInfoNormal,"CSipCall::SetChannelRate ", newRate);
		    pVidCap->SetBitRate(newRate);
			POBJDELETE(pVidCap);
		}
		else
		    PTRACE(eLevelError,"CSipCall::SetChannelRate - pVidCap is NULL - can not SetBitRate");
                POBJDELETE(pVidCap);
	}
}

//////////////////////////////////////////////////////////
BYTE CSipCall::IsMedia(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole) const
{
	CSipChannel* pChannelIn  = GetChannel(true, eMediaType,cmCapReceive,eRole);
	CSipChannel* pChannelOut = GetChannel(true, eMediaType,cmCapTransmit,eRole);
	BYTE res = YES;
	if (eDirection & cmCapReceive)
		res &= (pChannelIn && ((pChannelIn->m_eConnectionState == kConnected) || (pChannelIn->m_eConnectionState == kConnecting) || (pChannelIn->m_eConnectionState == kUpdating)))? YES: NO;
	if (eDirection & cmCapTransmit)
		res &= (pChannelOut && ((pChannelOut->m_eConnectionState == kConnected) || (pChannelOut->m_eConnectionState == kConnecting) || (pChannelOut->m_eConnectionState == kUpdating)))? YES: NO;
	return res;
}


//////////////////////////////////////////////////////////
BYTE CSipCall::IsChannelMuted(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole) const
{
	CSipChannel* pChannelIn  = GetChannel(true, eMediaType,cmCapReceive,eRole);
	CSipChannel* pChannelOut = GetChannel(true, eMediaType,cmCapTransmit,eRole);
	BYTE res = YES;
	if (eDirection & cmCapReceive)
		res &= (pChannelIn && pChannelIn->IsMuted())? YES: NO;
	if (eDirection & cmCapTransmit)
		res &= (pChannelOut && pChannelOut->IsMuted())? YES: NO;
	return res;
}


//////////////////////////////////////////////////////////
int	CSipCall::FindNextAvailableIndexInChannelArr() const
{
	int index = NA;
	for (int i = 0; i < MAX_SIP_CHANNELS && index == NA; i++)
	{
		if (m_channelsPtrArr[i] == NULL)
			index = i;
	}
	return index;
}

//////////////////////////////////////////////////////////
int	CSipCall::FindNextAvailableIndexInChannelArrEx() const
{
	int index = NA;
	
	for (int i = 0; i < MAX_INTERNAL_CHANNELS && index == NA; i++)
	{
		if (m_channelsPtrArrEx[i] == NULL)
			index = i;
	}
	
	return index;
}

//////////////////////////////////////////////////////////
void CSipCall::FindAndSetSameSassionChannel(CSipChannel* pChannel)
{
	cmCapDirection eSamesassionDirection = (pChannel->m_eDirection == cmCapReceive)? cmCapTransmit: cmCapReceive;
	CSipChannel* pSameSessionChannel = GetChannel(true, pChannel->m_eMediaType,eSamesassionDirection,pChannel->GetRoleLabel()); // NULL if not found
	pChannel->SetSameSessionChannel(pSameSessionChannel);
}
/////////////////////////////////////////////////////////////
void CSipCall::SetLprCapStruct(lprCapCallStruct* lprStruct, BYTE direction)
{
	if (direction > 1)
		PASSERT(direction);
	else
	{
		m_lprCapStruct[direction].versionID = lprStruct->versionID;
		m_lprCapStruct[direction].minProtectionPeriod = lprStruct->minProtectionPeriod;
		m_lprCapStruct[direction].maxProtectionPeriod = lprStruct->maxProtectionPeriod;
		m_lprCapStruct[direction].maxRecoverySet = lprStruct->maxRecoverySet;
		m_lprCapStruct[direction].maxRecoveryPackets = lprStruct->maxRecoveryPackets;
		m_lprCapStruct[direction].maxPacketSize = lprStruct->maxPacketSize;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
lprCapCallStruct* CSipCall::GetLprCapStruct(BYTE direction)
{
	if (direction > 1)
	{
		PASSERT(direction);
		return NULL;
	}

	return &(m_lprCapStruct[direction]);

}

///////////////////////////////////////////////////////////////////////////////

// Description: Create a new channel and set it's mcms index.
//				Return a pointer to the new channel in order to set its card index
//				and its connection state.
//---------------------------------------------------------------------------
CSipChannel* CSipCall::InsertNewChannel(cmCapDataType eMediaType,cmCapDirection eDirection,ERoleLabel eRole,CapEnum eAlgorithm,CSdesCap *pSdesCap, CDtlsCap *pDtlsCap , const BYTE data[],int length, const std::list <StreamDesc>* pStreams)
{
	CSipChannel* pChannel = GetChannel(true, eMediaType, eDirection, eRole,INVALID,true);
	// if channel of this type and direction is not found than it can be added

	if (pChannel == NULL)
	{
//		SystemCoreDump(FALSE);
		int i = FindNextAvailableIndexInChannelArr();

		if (i != NA)
		{
			pChannel = (eMediaType == cmCapAudio)? new CSipAudioChannel :new CSipChannel;
			pChannel->SetCallPtr(this);
			pChannel->SetMcmsIndex(i+1);
			pChannel->SetMediaType(eMediaType);
			pChannel->SetRoleLabel(eRole);
			pChannel->SetDirection(eDirection);

			pChannel->SetChannelQualities(eAlgorithm, data, length);
 			FindAndSetSameSassionChannel(pChannel);

 			pChannel->SetArrIndexInCall(i);

			pChannel->SetChannelSdes(pSdesCap);
			pChannel->SetChannelDtls(pDtlsCap);


			if(pStreams)
			{
				TRACEINTO<<"eMediaType:"<<(int)eMediaType<<"direction:"<<(int)eDirection<<"role:"<<(int)eRole;
				pChannel->SetStreamsList(*pStreams);
			}
			else
			{
				TRACEINTO<<"no streams: eMediaType:"<<::GetTypeStr(eMediaType)<<"direction:"<<::GetDirectionStr(eDirection)<<"role:"<<::GetRoleStr(eRole);
			}

			m_channelsPtrArr[i] = pChannel;
			m_numOfChannels++;

		    TRACEINTO << "Added external channel to index=" << i << " eMediaType=" << ::GetTypeStr(eMediaType)
		              << " eDirection=" << ::GetDirectionStr(eDirection) << " eRole=" << ::GetRoleStr(eRole) << "connectionId=" << pChannel->GetRtpConnectionId();
		}
		else
		{
			PASSERT(m_mcmsConId);
			PTRACE(eLevelError,"CSipCall::InsertNewChannel: No room for this channel");
		}
	}
	else
	{
		CSmallString msg;
		msg << "Channel of type " << ::CapDataTypeToString(eMediaType) << " direction " << eDirection << " is already exist in call " << m_mcmsConId << "\n";
		PTRACE2(eLevelError,"CSipCall::InsertNewChannel: ",msg.GetString());
		//PASSERT(pChannel->m_mcmsIndex);
		pChannel = NULL; //for the return value
	}
	return pChannel;
}

CSipChannel* CSipCall::InsertNewChannelEx(cmCapDataType eMediaType,cmCapDirection eDirection,ERoleLabel eRole,CapEnum eAlgorithm,CSdesCap *pSdesCap, const BYTE data[],int length,
                                          const std::list <StreamDesc>& rStreams, APIU32 aSsrcId)
{
	TRACEINTO << "mix_mode: Adding internal channel eMediaType=" << ::GetTypeStr(eMediaType)
			  << " eDirection=" << ::GetDirectionStr(eDirection) << " eRole=" << ::GetRoleStr(eRole);

	bool channelExists = false;

	CSipChannel* pChannel = GetChannel(false, eMediaType, eDirection, eRole,INVALID,true);

	// if channel of this type and direction and ssrc is not found than it can be added
	if (pChannel)
	{
		channelExists = true;
		const std::list <StreamDesc> streamsDescList = pChannel->GetStreams();
		std::list <StreamDesc>::const_iterator itr_streams;
		bool channelFound = false;

		for(itr_streams=streamsDescList.begin(); itr_streams!=streamsDescList.end(); itr_streams++)
		{
			if (itr_streams->m_pipeIdSsrc == aSsrcId || aSsrcId == INVALID)
			{
				channelFound = true;
				TRACEINTO << "mix_mode: Channel cannot be added. Channel with the same ssrc " << aSsrcId << " already exists";
				break;
			}
		}
		if (channelFound == false)
		{
			channelExists = false;
		}
	}

	if (channelExists == false)
	{
		int i = FindNextAvailableIndexInChannelArrEx();
		
		if (i != NA)
		{
			pChannel = (eMediaType == cmCapAudio)? new CSipAudioChannel :new CSipChannel;
			pChannel->SetCallPtr(this);
			pChannel->SetMcmsIndex(i+1);
			pChannel->SetMediaType(eMediaType);
			pChannel->SetRoleLabel(eRole);
			pChannel->SetDirection(eDirection);
			pChannel->SetChannelQualities(eAlgorithm, data, length);
			pChannel->SetArrIndexInCall(i);
			pChannel->SetChannelSdes(pSdesCap);
			pChannel->SetStreamsList(rStreams, aSsrcId);
			m_channelsPtrArrEx[i] = pChannel;
			m_numOfChannelsEx++;
			
			TRACEINTO << "i = " << i;
		}
		else
		{
			PASSERT(m_mcmsConId);
			PTRACE(eLevelError,"CSipCall::InsertNewChannelEx: No room for this channel");
		}
	}
	else
	{
		CSmallString msg;
		msg << "Channel of type " << ::CapDataTypeToString(eMediaType) << " direction " << eDirection << "	already exists in call " << m_mcmsConId << "\n";
		PTRACE2(eLevelError,"CSipCall::InsertNewChannelEx: ",msg.GetString());
		PASSERT(pChannel->m_mcmsIndex);
		pChannel = NULL; //for the return value
	}
	
	return pChannel;
}


//////////////////////////////////////////////////////////
void CSipCall::RemoveChannel(CSipChannel* pChannel)
{
	if (pChannel && pChannel->m_arrIndexInCall < MAX_SIP_CHANNELS)
	{
		if (pChannel->m_pSameSessionChannel)
			pChannel->m_pSameSessionChannel->m_pSameSessionChannel = NULL;
		if(m_channelsPtrArr[pChannel->m_arrIndexInCall]==pChannel)
		{
			m_channelsPtrArr[pChannel->m_arrIndexInCall] = NULL;
			POBJDELETE(pChannel);
			m_numOfChannels--;
			TRACEINTO << "Remove regular channel. m_numOfChannels=" <<m_numOfChannels;
		}
		else
		{
			TRACEINTO << "m_confMediaType=" << m_confMediaType;
			if(eMixAvcSvc == m_confMediaType || eMixAvcSvcVsw == m_confMediaType /*|| bIsActiveVswRelay==TRUE*/ ) /*eyaln9794  */
			{
				if(pChannel->m_arrIndexInCall < MAX_INTERNAL_CHANNELS)
				{
					if(m_channelsPtrArrEx[pChannel->m_arrIndexInCall]!=NULL && m_channelsPtrArrEx[pChannel->m_arrIndexInCall]==pChannel)
					{
						m_channelsPtrArrEx[pChannel->m_arrIndexInCall] = NULL;
						POBJDELETE(pChannel);
						m_numOfChannelsEx--;
						TRACEINTO << "mix_mode: Channel removed, m_numOfChannelsEx=" << m_numOfChannelsEx;
					}
				}

			}
		}
	}
}


//////////////////////////////////////////////////////////
void CSipCall::RemoveChannel(DWORD mcmsChannelIndex,DWORD cardChannelIndex)
{
	CSipChannel* pChannel = GetChannel(mcmsChannelIndex,cardChannelIndex);
	RemoveChannel(pChannel);
}


/////////////////////////////////////////////////////////////////////////////////
//void CSipCall::RemoveChannelInAState(EConnectionState eState)
//{
//	for (int i=0; i<MAX_SIP_CHANNELS; i++)
//	{
//		if ( (NULL != m_channelsPtrArr[i]) && (m_channelsPtrArr[i]->GetConnectionState() == eState) )
//			RemoveChannel(m_channelsPtrArr[i]);
//	}
//}


/////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetCallLegAndCdrHeaders(const sipMessageHeaders& headers)
{
	POBJDELETE(m_pCallLegHeaders);
	CSipHeaderList* pTemp = new CSipHeaderList(headers);

	// call leg headers
	const CSipHeader* pToDisplay	= pTemp->GetNextHeader(kToDisplay);
	const CSipHeader* pTo			= pTemp->GetNextHeader(kTo);
	const CSipHeader* pFromDisplay	= pTemp->GetNextHeader(kFromDisplay);
	const CSipHeader* pFrom			= pTemp->GetNextHeader(kFrom);

	// cdr headers (according to Lucent)
	const CSipHeader* pCalledPartyId	= pTemp->GetNextPrivateOrProprietyHeader(kPrivateHeader,strlen("P-Called-Party-ID"),"P-Called-Party-ID");
	const CSipHeader* pAssertedIdentity = pTemp->GetNextPrivateOrProprietyHeader(kPrivateHeader,strlen("P-Asserted-Identity"),"P-Asserted-Identity");
	const CSipHeader* pChargingVector	= pTemp->GetNextPrivateOrProprietyHeader(kPrivateHeader,strlen("P-Charging-Vector"),"P-Charging-Vector");
	const CSipHeader* pPreferredIdentity= pTemp->GetNextPrivateOrProprietyHeader(kPrivateHeader,strlen("P-Preferred-Identity"),"P-Preferred-Identity");

	m_pCallLegHeaders = new CSipHeaderList(MIN_ALLOC_HEADERS,0);

	if (pToDisplay)
		m_pCallLegHeaders->AddHeader(*pToDisplay);
	if (pTo)
		m_pCallLegHeaders->AddHeader(*pTo);
	if (pFromDisplay)
		m_pCallLegHeaders->AddHeader(*pFromDisplay);
	if (pFrom)
		m_pCallLegHeaders->AddHeader(*pFrom);

	m_pCdrPrivateHeaders = new CSipHeaderList(MIN_ALLOC_HEADERS,0);
	if (pCalledPartyId)
		m_pCdrPrivateHeaders->AddHeader(*pCalledPartyId);
	if (pAssertedIdentity)
		m_pCdrPrivateHeaders->AddHeader(*pAssertedIdentity);
	if (pChargingVector)
		m_pCdrPrivateHeaders->AddHeader(*pChargingVector);
	if (pPreferredIdentity)
		m_pCdrPrivateHeaders->AddHeader(*pPreferredIdentity);

	POBJDELETE(pTemp);
}


/////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetRemoteHeaders(const sipSdpAndHeadersSt* pSdpAndHeaders)
{
	if (pSdpAndHeaders->sipHeadersLength)
	{
		BYTE* pStart = (BYTE*) &(pSdpAndHeaders->capsAndHeaders[pSdpAndHeaders->sipHeadersOffset]);
		sipMessageHeaders* pHeaders = (sipMessageHeaders*) pStart;
		PDELETEA(m_pRmtHeaders);
		int length = pSdpAndHeaders->sipHeadersLength;
		m_pRmtHeaders = (sipMessageHeaders *)new BYTE[length];
		memset(m_pRmtHeaders, 0, length);
		memcpy(m_pRmtHeaders, pStart, length);
	}
	else
	{
		PDELETEA(m_pRmtHeaders);
	}
}


/////////////////////////////////////////////////////////////////////////////////
const CSipHeader* CSipCall::GetCdrSpecificHeader(int strLen,const char* strHeaderName)
{
    if (m_pCdrPrivateHeaders)
		return m_pCdrPrivateHeaders->GetNextPrivateOrProprietyHeader(kPrivateHeader,strLen,strHeaderName);
    else
        return NULL;
}


/////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetWarningString(const char* warningStr)
{
	if (warningStr)
	{
		PTRACE(eLevelInfoNormal, warningStr);
		PDELETEA(m_warningStr);
		int len = strlen(warningStr);
		m_warningStr = new char[len+1];
		memcpy(m_warningStr,warningStr,len);
		m_warningStr[len] = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetForwardAddr(const char* forwardAddrStr)
{
	if (forwardAddrStr)
	{
		PDELETEA(m_forwardAddrStr);
		int len = strlen(forwardAddrStr);
		m_forwardAddrStr = new char[len+1];
		memcpy(m_forwardAddrStr,forwardAddrStr,len);
		m_forwardAddrStr[len] = 0;
	}
}


/////////////////////////////////////////////////////////////////////////////////
int CSipCall::SetChannelsConnectingState(EConnectionState eNewState,int arrSize,EIpChannelType chanArr[], EConnectionState eState /*= kUnknown*/)
{
	int numOfChannelsSet = 0;
	for (int i=0; i<arrSize; i++)
	{
		CSipChannel* pChannel = NULL;

		if (arrSize == MAX_SIP_CHANNELS)
			pChannel = GetChannel(i, true);
		else if (chanArr)
		{
			if(i < CHANNEL_TYPES_COUNT)
				pChannel = GetChannel(chanArr[i]);
			else
				PASSERTMSG(1, "i exceed chanArr size"); 
		}
		else
			break;

		if (pChannel)
		{
			//SIP LATER VIDEO
			//if state defined and different from channel state pass over the channel
			if ( eState != kUnknown && pChannel->m_eConnectionState != eState )
				continue;

			pChannel->m_eConnectionState = eNewState;
			pChannel->m_eRtpConnectionState = eNewState;
			pChannel->m_eCmConnectionState = eNewState;

			if (eNewState == kDisconnecting)
			{
				memset((void*) &pChannel->m_address, 0, sizeof(mcTransportAddress));
//				pChannel->m_address.addr.v4.ip = 0;
//				pChannel->m_address.port = 0;
//				pChannel->m_eStreamState = kDisconnected;
			}
			numOfChannelsSet++;
		}
	}
	return numOfChannelsSet;
}


/////////////////////////////////////////////////////////////////////////////////
// @#@ - to be reviewed
void CSipCall::SetChannelsEx(CSipComMode* pComMode, cmCapDirection eDirection,BYTE bIsMrcCall)
{

	int startPoint = 0;
	int endPoint   = 2;
	DWORD isEncrypted;

	if (eMixAvcSvc == m_confMediaType)
	{
		endPoint = 1;
	}

	if (eDirection == cmCapReceive) //and not transmit
	{
		// startPoint is 0
		endPoint = 1;
	}
	else if (eDirection == cmCapTransmit) //and not receive
	{
	// endPoint is 2
		startPoint = 1;
	}

	int length = 0;
	capBuffer* pCapBuffer = NULL;
	isEncrypted = pComMode->GetIsEncrypted();
	CSdesCap *pSdesCap = NULL;
	cmCapDataType mediaType;
	ERoleLabel eRole;

//	for(int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
//	{
    	GetMediaDataTypeAndRole(kSipMediaChannelVideo, mediaType, eRole);

//		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		for (int j = startPoint; j < endPoint; j++)
		{
			pSdesCap = NULL;

			if ( pComMode->IsMediaOn(mediaType, globalDirectionArr[j], eRole) )
			{
				length  = pComMode->GetMediaLength(mediaType,globalDirectionArr[j],eRole);
				pCapBuffer = (capBuffer *)new BYTE[sizeof(capBufferBase) + length];
				memset(pCapBuffer, 0, sizeof(capBufferBase) + length);
				pComMode->CopyMediaToCapBuffer(pCapBuffer,mediaType,globalDirectionArr[j],eRole);

				if(isEncrypted == Encryp_On && mediaType != cmCapBfcp) {
					pSdesCap =  pComMode->GetSipSdes(mediaType,globalDirectionArr[j],eRole);
				}

				const std::list <StreamDesc> streamsDescList = pComMode->GetStreamsListForMediaMode(mediaType, globalDirectionArr[j], eRole);
				InsertNewChannelEx(mediaType,globalDirectionArr[j],eRole,(CapEnum)pCapBuffer->capTypeCode, pSdesCap, pCapBuffer->dataCap,pCapBuffer->capLength, streamsDescList);
				PDELETEA(pCapBuffer);
			}
		}
//	}

}

/////////////////////////////////////////////////////////////////////////////////
// @#@ - to be reviewed
CSipChannel*  CSipCall::AddChannelInternal(CSipComMode* pComMode, cmCapDataType aMediaType, cmCapDirection aDirection, CapEnum aCapCode, APIU32 aSsrcId)
{
    const std::list <StreamDesc> streamsDescList = pComMode->GetStreamsListForMediaMode(aMediaType, cmCapReceive, kRolePeople);
    return InsertNewChannelEx(aMediaType, aDirection, kRolePeople, aCapCode, NULL, NULL, 0, streamsDescList, aSsrcId);
}

void CSipCall::SetChannels(CSipComMode* pComMode, cmCapDirection eDirection)
{
	int startPoint = 0;
	int endPoint   = 2;
	DWORD isEncrypted;
	DWORD isDtlsEncrypted; //_dtls_

	pComMode->Dump("multi_line - CSipCall::SetChannels ", eLevelInfoNormal);

	if (eDirection == cmCapReceive) //and not transmit
	{
		// startPoint is 0
		endPoint = 1;
	}
	else if (eDirection == cmCapTransmit) //and not receive
	{
	// endPoint is 2
		startPoint = 1;
	}

	int length = 0;
	capBuffer* pCapBuffer = NULL;
	isEncrypted = pComMode->GetIsEncrypted();
	CSdesCap *pSdesCap = NULL;
	isDtlsEncrypted = pComMode->GetIsDtlsEncrypted();
	CDtlsCap *pDtlsCap = NULL;
	cmCapDataType mediaType;
	ERoleLabel eRole;

	for(int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		pSdesCap = NULL;

		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		for (int j = startPoint; j < endPoint; j++)
		{
			if ( pComMode->IsMediaOn(mediaType, globalDirectionArr[j], eRole) )
			{
				length  = pComMode->GetMediaLength(mediaType,globalDirectionArr[j],eRole);
				pCapBuffer = (capBuffer *)new BYTE[sizeof(capBufferBase) + length];
				memset(pCapBuffer, 0, sizeof(capBufferBase) + length);

				pComMode->CopyMediaToCapBuffer(pCapBuffer,mediaType,globalDirectionArr[j],eRole);

				if(isEncrypted == Encryp_On && mediaType != cmCapBfcp) {
					pSdesCap =  pComMode->GetSipSdes(mediaType,globalDirectionArr[j],eRole);
				}

				if(isDtlsEncrypted == Encryp_On && pComMode->GetIsDtlsAvailable() && mediaType != cmCapBfcp) {
					pDtlsCap =  pComMode->GetSipDtls(mediaType,globalDirectionArr[j],eRole);
				}

				const std::list <StreamDesc> streamsDescList = pComMode->GetStreamsListForMediaMode(mediaType, globalDirectionArr[j], eRole);
				InsertNewChannel(mediaType,globalDirectionArr[j],eRole,(CapEnum)pCapBuffer->capTypeCode, pSdesCap, pDtlsCap, pCapBuffer->dataCap,pCapBuffer->capLength, &streamsDescList);
				PDELETEA(pCapBuffer);
			}
		}
	}

	PTRACE(eLevelError,"CSipCall::SetChannels - end");
}


// updates only the existing channels
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////
int CSipCall::SetUpdatingChannels(CSipComMode* pComMode,int arrSize,EIpChannelType chanArr[],EUpdateType eUpdate)
{
	int numOfChannelsSet = 0;
	CSipChannel* pCurChannel = NULL;

	for (int i=0; i<arrSize; i++)
	{
		if (arrSize == MAX_SIP_CHANNELS)
			pCurChannel = GetChannel(i, true);
		else if (chanArr)
			pCurChannel = GetChannel(chanArr[i]);
		else
			break;

		if( pCurChannel )
		{
			if(eUpdate == kChannelParams || eUpdate == kChangeLpr)
			{
				if (pComMode && pComMode->IsMediaOn(pCurChannel->m_eMediaType,pCurChannel->m_eDirection,pCurChannel->GetRoleLabel()))
				{
					CBaseCap* pMode = pComMode->GetMediaAsCapClass(pCurChannel->m_eMediaType,pCurChannel->m_eDirection,pCurChannel->GetRoleLabel());
					if (pMode)
					{
						CapEnum	eAlgorithm	= pMode->GetCapCode();
						int		length		= pMode->SizeOf();
						BYTE*	data		= (BYTE *)pMode->GetStruct();
						pCurChannel->SetChannelQualities(eAlgorithm,data,length);
						pCurChannel->SetRtpConnectionState(kUpdating);
						pCurChannel->SetConnectionState(kUpdating);
						numOfChannelsSet++;
					}
					POBJDELETE(pMode);
				}
			}
			else if(eUpdate == kIpAddress)
			{
				pCurChannel->SetCmConnectionState(kUpdating);
				pCurChannel->SetConnectionState(kUpdating);
				numOfChannelsSet++;
			}
			else if(eUpdate == kChangeSdes)
			{
				CSdesCap *pSdesCap = NULL;
				if(pComMode && pComMode->IsMediaOn(pCurChannel->m_eMediaType,pCurChannel->m_eDirection,pCurChannel->GetRoleLabel()))
				{
					pSdesCap = pComMode->GetSipSdes(pCurChannel->m_eMediaType,pCurChannel->m_eDirection,pCurChannel->GetRoleLabel());
					pCurChannel->RemoveChannelSdes();
					pCurChannel->SetChannelSdes(pSdesCap);
					numOfChannelsSet++;
				}
				CSdesCap *pDtlsCap = NULL;
				if(pComMode && pComMode->IsMediaOn(pCurChannel->m_eMediaType,pCurChannel->m_eDirection,pCurChannel->GetRoleLabel()) && pCurChannel->IsChannelDtlsEnabled() )
				{
					pDtlsCap = pComMode->GetSipDtls(pCurChannel->m_eMediaType,pCurChannel->m_eDirection,pCurChannel->GetRoleLabel());
					if(pDtlsCap == NULL)
					{
						pCurChannel->RemoveChannelDtls();
						PTRACE(eLevelError,"CSipCall::SetUpdatingChannels -remove channel DTLS");
					}
				}
			}
		}
	}
	return numOfChannelsSet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
mcTransportAddress* CSipCall::GetDnsProxyIpAddress(WORD place)
{
	if (place < TOTAL_NUM_OF_IP_ADDRESSES)
		return &(m_DnsProxyIpAddressArray[place]);
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetDnsProxyIpAddressArray(ipAddressStruct* dnsProxyAddrArr)
{

	ALLOCBUFFER(mediaAddr,IPV6_ADDRESS_LEN);
	for (int i = 0; i < TOTAL_NUM_OF_IP_ADDRESSES; i++)
	{

		memset(mediaAddr,'\0',IPV6_ADDRESS_LEN);
		m_DnsProxyIpAddressArray[i].ipVersion = dnsProxyAddrArr[i].ipVersion;
		memcpy(&(m_DnsProxyIpAddressArray[i].addr), &(dnsProxyAddrArr[i].addr), sizeof(ipAddressIf));
		if (m_DnsProxyIpAddressArray[i].ipVersion == eIpVersion6)
		{
			::ipToString(m_DnsProxyIpAddressArray[i],mediaAddr,1);
			enScopeId ePerferedIpV6ScopeAddr;
			ePerferedIpV6ScopeAddr = ::getScopeId(mediaAddr);
			m_DnsProxyIpAddressArray[i].addr.v6.scopeId = ePerferedIpV6ScopeAddr;
		}

	}
	DEALLOCBUFFER(mediaAddr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*void CSipCall::SetMediaPayloadType(cmCapDirection eDirToSet, const CSipCaps& caps, int arrSize, EIpChannelType chanArr[])
{
	CSipChannel* pChannel = NULL;

	for (int i=0; i<arrSize; i++)
	{
		if (arrSize == MAX_SIP_CHANNELS)
			pChannel = GetChannel(i);
		else if (chanArr)
			pChannel = GetChannel(chanArr[i]);
		else
			break;

		// there is a possibility that not all media is set
		if (pChannel)
		{
			//update only if meet the eDirToSet channels
			if ( !(eDirToSet & pChannel->GetDirection()) )
				continue;

			CCapSetInfo capInfo	= pChannel->m_eAlgorithm;
			int indexToSearchInCaps = 1; //always prefer first priority in caps
			pChannel->m_payloadType	= caps.GetPayloadType(capInfo, indexToSearchInCaps);

			if (pChannel->m_payloadType == _H263)
			{// seach if we have H263 dynamic as second prefernce. If yes, use it instead the static one (will happen only with remote caps)
				indexToSearchInCaps = 2;
				payload_en secondPreference = caps.GetPayloadType(capInfo, indexToSearchInCaps);
				if (IsDynamicPayloadType(secondPreference))
					pChannel->m_payloadType	= secondPreference;
			}

			if (pChannel->m_eMediaType == cmCapAudio)
			{
				CSipAudioChannel* pAudioChannel  = (CSipAudioChannel*)pChannel;
				pAudioChannel->m_dtmfPayloadType = _Rfc2833DtmfDynamic;// this code is only for incoming channels!!!
			}
		}
	}
}*/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetMediaPayloadType(cmCapDirection eDirToSet, const CSipCaps* pLocalCaps, int arrSize, EIpChannelType chanArr[], const CSipCaps* pRemoteCaps)
{

	CSipChannel* pChannel = NULL;

	for (int i=0; i<arrSize; i++)
	{
		if (arrSize == MAX_SIP_CHANNELS)
			pChannel = GetChannel(i, true);
		else if (chanArr)
			pChannel = GetChannel(chanArr[i]);
		else
			break;

		// there is a possibility that not all media is set
		if (pChannel)
		{
			//update only if meet the eDirToSet channels
			if ( !(eDirToSet & pChannel->GetDirection()) )
				continue;

			CCapSetInfo capInfo	= pChannel->m_eAlgorithm;
			WORD   profile  = H264_Profile_None;
			APIS32 H264mode = H264_standard;
			APIU8  packetizationMode = 0;

			if ((CapEnum)capInfo == eH264CapCode)
			{
				CH264VideoCap* pH264VidCap = (CH264VideoCap*)pChannel->GetDataAsCapClass();
				if (pH264VidCap)
				{
					profile  = pH264VidCap->GetProfile();
					H264mode = pH264VidCap->GetH264mode();
					packetizationMode = pH264VidCap->GetPacketizationMode();
				}
				POBJDELETE(pH264VidCap);
			}
			payload_en payloadToSet = _UnKnown;

			if ((eDirToSet == cmCapTransmit) && pRemoteCaps) //transmit payload type is determined according to remote caps
			{
				payloadToSet = pRemoteCaps->GetPayloadTypeByDynamicPreference(capInfo, profile, pChannel->GetRoleLabel(), H264mode, packetizationMode);
//				TRACEINTO<<"$#@ setting: PayloadType (transmit case):"<<(int)payloadToSet;
				if (pLocalCaps)
				{
					if (::IsDynamicPayloadType(payloadToSet)) //if remote supports dynamic, check local support
					{
						payload_en localPayload = pLocalCaps->GetPayloadTypeByDynamicPreference(capInfo, profile, pChannel->GetRoleLabel(), H264mode, packetizationMode);

						if (::IsValidPayloadType(localPayload) && !::IsDynamicPayloadType(localPayload))	//if local doesn't support dynamic, we need to use the static
						{
//							TRACEINTO<<"$#@ setting: localPayload are we suppoosed to get here?"<<(int)localPayload;
							payloadToSet = localPayload;

						}
					}
				}
			}
			else	//receive payload type is determined according to local caps
			{
				payloadToSet = pLocalCaps->GetPayloadTypeByDynamicPreference(capInfo, profile, pChannel->GetRoleLabel(), H264mode, packetizationMode);
//				TRACEINTO<<"$#@ setting: PayloadType (receive case):"<<(int)payloadToSet;
				if(payloadToSet == _UnKnown)
					payloadToSet = (payload_en)capInfo.GetDynamicPayloadType(0);
				if (pRemoteCaps)
				{
					if (::IsDynamicPayloadType(payloadToSet)) //if local supports dynamic, check remote support
					{
						payload_en remotePayload = pRemoteCaps->GetPayloadTypeByDynamicPreference(capInfo, profile, pChannel->GetRoleLabel(), H264mode, packetizationMode);
						if (::IsValidPayloadType(remotePayload) && !::IsDynamicPayloadType(remotePayload))	//if remote doesn't support dynamic, we need to use the static
						{
//							TRACEINTO<<"$#@ setting: Payload are we supposed to get here??"<<(int)remotePayload;
							payloadToSet = remotePayload;
						}
					}
				}
			}

			if (GetIsTipCall() && (pChannel->GetMediaType() == cmCapVideo))
			{
				if ((pChannel->GetRoleLabel() & kRoleContentOrPresentation))
				{
					CSipChannel* pVidChannel = GetChannel(true, cmCapVideo, eDirToSet);

					if (pVidChannel)
					{
						payloadToSet = (payload_en)pVidChannel->GetPayloadType();
						PTRACE2INT(eLevelInfoNormal,"CSipCall::SetMediaPayloadType : Set payload type for content channel according to video ", payloadToSet);
					}
				}
				else if (eDirToSet == cmCapTransmit)
				{
				// vngr-25135 - if TIP EP switch to base profile, payload type = 112, RMX still send video in main profile, payload type = 112.
				if ((payloadToSet == _UnKnown) 			&&
					(profile      == H264_Profile_Main))
					{
						PTRACE(eLevelInfoNormal,"CSipCall::SetMediaPayloadType : Set payload type for TIP video transmit to 112");
						payloadToSet = (payload_en) eH264MpDynamicPayload;
					}
				}
			}

			pChannel->SetPayloadType(payloadToSet);

			if (pChannel->m_eMediaType == cmCapAudio)
			{
				CSipAudioChannel* pAudioChannel		= (CSipAudioChannel*)pChannel;

				if(pAudioChannel->m_dtmfPayloadType == _UnKnown)//VNGR-23900 change dtmf payload only if it has no value
				{
				    CCapSetInfo dtmfCapInfo				= eRfc2833DtmfCapCode;
				    pAudioChannel->m_dtmfPayloadType 	= dtmfCapInfo.GetDynamicPayloadType(0);// this code is only for incoming channels!!!
				}
			}
		}
	}
}

void CSipCall::SetMediaPayloadTypeEx(cmCapDirection eDirToSet, const CSipCaps* pLocalCaps, int arrSize, EIpChannelType chanArr[], const CSipCaps* pRemoteCaps)
{

	CSipChannel* pChannel = NULL;

	for (int i=0; i<arrSize; i++)
	{
		if (arrSize == MAX_INTERNAL_CHANNELS)
			pChannel = GetChannel(i, false);
		else if (chanArr)
			pChannel = GetChannel(chanArr[i], false);
		else
			break;

		// there is a possibility that not all media is set
		if (pChannel)
		{
			//update only if meet the eDirToSet channels
			if ( !(eDirToSet & pChannel->GetDirection()) )
				continue;

			CCapSetInfo capInfo	= pChannel->m_eAlgorithm;
			WORD   profile  = H264_Profile_None;
			APIS32 H264mode = H264_standard;
			APIU8  packetizationMode = 0;
			if ((CapEnum)capInfo == eH264CapCode)
			{
				CH264VideoCap* pH264VidCap = (CH264VideoCap*)pChannel->GetDataAsCapClass();
				if (pH264VidCap)
				{
					profile  = pH264VidCap->GetProfile();
					H264mode = pH264VidCap->GetH264mode();
					packetizationMode = pH264VidCap->GetPacketizationMode();

				}
				POBJDELETE(pH264VidCap);
			}
			payload_en payloadToSet = _UnKnown;

			if (GetIsTipCall() && (pChannel->GetMediaType() == cmCapVideo) && (pChannel->GetRoleLabel() & kRoleContentOrPresentation))
			{
				CSipChannel* pVidChannel = GetChannel(false, cmCapVideo, eDirToSet);
				if (pVidChannel)
				{
					payloadToSet = (payload_en)pVidChannel->GetPayloadType();
					PTRACE2INT(eLevelInfoNormal,"CSipCall::SetMediaPayloadType : Set payload type for content channel according to video ", payloadToSet);
				}
			}
			else if ((eDirToSet == cmCapTransmit) && pRemoteCaps) //transmit payload type is determined according to remote caps
			{
				payloadToSet = pRemoteCaps->GetPayloadTypeByDynamicPreference(capInfo, profile, pChannel->GetRoleLabel(), H264mode,packetizationMode);
				if (pLocalCaps)
				{
					if (::IsDynamicPayloadType(payloadToSet)) //if remote supports dynamic, check local support
					{
						payload_en localPayload = pLocalCaps->GetPayloadTypeByDynamicPreference(capInfo, profile, pChannel->GetRoleLabel(), H264mode,packetizationMode);
						if (::IsValidPayloadType(localPayload) && !::IsDynamicPayloadType(localPayload))	//if local doesn't support dynamic, we need to use the static
							payloadToSet = localPayload;
					}
				}
			}

			else	//receive payload type is determined according to local caps
			{
				payloadToSet = pLocalCaps->GetPayloadTypeByDynamicPreference(capInfo, profile, pChannel->GetRoleLabel(), H264mode,packetizationMode);
				if(payloadToSet == _UnKnown)
					payloadToSet = (payload_en)capInfo.GetDynamicPayloadType(0);
				if (pRemoteCaps)
				{
					if (::IsDynamicPayloadType(payloadToSet)) //if local supports dynamic, check remote support
					{
						payload_en remotePayload = pRemoteCaps->GetPayloadTypeByDynamicPreference(capInfo, profile, pChannel->GetRoleLabel(), H264mode,packetizationMode);
						if (::IsValidPayloadType(remotePayload) && !::IsDynamicPayloadType(remotePayload))	//if remote doesn't support dynamic, we need to use the static
							payloadToSet = remotePayload;
					}
				}
			}

			pChannel->SetPayloadType(payloadToSet);

			if (pChannel->m_eMediaType == cmCapAudio)
			{
				CSipAudioChannel* pAudioChannel		= (CSipAudioChannel*)pChannel;
				CCapSetInfo dtmfCapInfo				= eRfc2833DtmfCapCode;
				pAudioChannel->m_dtmfPayloadType 	= dtmfCapInfo.GetDynamicPayloadType(0);// this code is only for incoming channels!!!
			}
		}
	}
}
////////////////////////////////////////////////////////////////
void CSipCall::SetMediaPayloadTypeAccordingToNewPayload(cmCapDirection eDirToSet,EIpChannelType chanArr[],int arrSize,WORD payload)
{
	CSipChannel* pChannel = NULL;

	for (int i=0; i<arrSize; i++)
	{
		if (arrSize == MAX_SIP_CHANNELS)
			pChannel = GetChannel(i, true);
		else if (chanArr)
			pChannel = GetChannel(chanArr[i]);
		else
			break;

			// there is a possibility that not all media is set
		if (pChannel)
		{
				//update only if meet the eDirToSet channels
			if ( !(eDirToSet & pChannel->GetDirection()) )
				continue;

			pChannel->SetPayloadType(payload);
		}

	}


}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetMediaRtpAddress(const sipSdpAndHeadersSt& sdpAndHeaders, cmCapDirection eDirection, BYTE confIsEncrypt)
{
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for(int i = 0; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		eMediaLineInternalType mlineInternalType = GetMediaLineInternalType(mediaType, eRole);
		SetMediaRtpAddress(ExtractMLineMediaIp(mlineInternalType, &sdpAndHeaders, m_dummyMediaIp, confIsEncrypt).transAddr, mediaType, eDirection, eRole);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetMediaRtpAddress(const mcTransportAddress& ipAddress, cmCapDataType eType, cmCapDirection eDirection, ERoleLabel eRole)
{
	CSipChannel* pChannel = GetChannel(true, eType, eDirection, eRole);

	if (pChannel)
	{
		memcpy((void*) &pChannel->m_address, &ipAddress, sizeof(mcTransportAddress));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetMediaRtcpPort(const sipSdpAndHeadersSt& sdpAndHeaders, cmCapDirection eDirection, BYTE confIsEncrypt)
{
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for(int i = 0; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		eMediaLineInternalType mlineInternalType = GetMediaLineInternalType(mediaType, eRole);
		unsigned int dummyRtcpPort;
		SetMediaRtcpPort(ExtractMLineRtcpPort(mlineInternalType, &sdpAndHeaders, dummyRtcpPort, confIsEncrypt), mediaType, eDirection,eRole);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetMediaRtcpPort(const DWORD rtcpPort, cmCapDataType eType, cmCapDirection eDirection, ERoleLabel eRole)
{
	CSipChannel* pChannel = GetChannel(true, eType, eDirection, eRole);

	if (pChannel)
	{
		pChannel->SetRtcpPort(rtcpPort);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetMediaTos(int value,int rtcpValue,cmCapDataType eType,cmCapDirection eDirection, ERoleLabel eRole)
{
	CSipChannel*	pChannel	= NULL;

	BOOL isRtcpQosIsEqualToRtp = NO;
	std::string key = "RTCP_QOS_IS_EQUAL_TO_RTP";
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, isRtcpQosIsEqualToRtp);
	if( isRtcpQosIsEqualToRtp )
	   rtcpValue = value;
	if (eDirection & cmCapReceive)
	{
		pChannel = GetChannel(true, eType, cmCapReceive, eRole);
		if (pChannel)
		{
			pChannel->SetTos(MEDIA_TOS_VALUE_PLACE, value);
			pChannel->SetTos(RTCP_TOS_VALUE_PLACE, rtcpValue);
		}
	}
	if (eDirection & cmCapTransmit)
	{
		pChannel = GetChannel(true, eType, cmCapTransmit, eRole);
		if (pChannel)
		{
			pChannel->SetTos(MEDIA_TOS_VALUE_PLACE, value);
			pChannel->SetTos(RTCP_TOS_VALUE_PLACE, rtcpValue);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//void CSipCall::CloseAllChannels()
//{
//	CSipChannel* pChannel = NULL;
//	for (int i=0; i<MAX_SIP_CHANNELS; i++)
//	{
//		pChannel = m_channelsPtrArr[i];
//		RemoveChannel(pChannel);
//	}
//}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::MuteChannels(BYTE bIsMute,int arrSize,EIpChannelType chanArr[])
{
	CSipChannel* pChannel = NULL;

	for (int i=0; i<arrSize; i++)
	{
		if (arrSize == MAX_SIP_CHANNELS)
			pChannel = GetChannel(i, true);
		else if (chanArr)
			pChannel = GetChannel(chanArr[i]);
		else
			break;

		if (pChannel)
			pChannel->Mute(bIsMute);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::UpdateSdesChannels(CSipCaps* localCaps, BYTE bTakeSdesFromTx)
{
	PTRACE2INT(eLevelInfoNormal,"CSipCall::UpdateSdesChannels : SRTP SVC bTakeSdesFromTx=",bTakeSdesFromTx);

	cmCapDirection 	eDirection 		= cmCapReceive;
	CSipChannel* 	pChannel 		= NULL;
	BYTE			bIsSdesEnabled 	= FALSE;
	CSdesCap* 		pSdesCap 		= NULL;

	// Take and save SDES data to TX channel if it re-invite indication or 200OK message
	// else: take and save to RX channel
	if(bTakeSdesFromTx)
		eDirection = cmCapTransmit;

	for(int i = 0 ; i < MAX_SIP_CHANNELS; i++)
	{
		PTRACE2INT(eLevelInfoNormal,"CSipCall::UpdateSdesChannels : SRTP SVC i=",i);

		pChannel = m_channelsPtrArr[i];

		if(pChannel && (pChannel->m_eDirection & eDirection))
		{
			PTRACE2INT(eLevelInfoNormal,"CSipCall::UpdateSdesChannels : SRTP SVC channel i=",i);
			bIsSdesEnabled = pChannel->IsChannelSdesEnabled();
			PTRACE2INT(eLevelInfoNormal,"CSipCall::UpdateSdesChannels : SRTP SVC channel bIsSdesEnabled=",bIsSdesEnabled);

			if(bIsSdesEnabled == TRUE) {
				cmCapDataType dataType = pChannel->GetMediaType();
				ERoleLabel eRole = pChannel->GetRoleLabel();
				pSdesCap = localCaps->GetSdesCap(dataType, eRole);

				if(pSdesCap) {
					PTRACE(eLevelInfoNormal,"CSipCall::UpdateSdesChannels : SRTP SVC pSdesCap");

					pChannel->RemoveChannelSdes();
					pChannel->SetChannelSdes(pSdesCap);
					POBJDELETE(pSdesCap);
				}
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetIceData(CSipCaps* LocalCaps)
{
	cmCapDirection eDirection = cmCapReceive;
	CSipChannel* pChannel = NULL;
	for(int i = 0 ; i < MAX_SIP_CHANNELS; i++)
	{
		pChannel = m_channelsPtrArr[i];

		if(pChannel && (pChannel->m_eDirection & eDirection))
		{
			pChannel->SetIceData(LocalCaps);


		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::ClearIceData()
{
	cmCapDirection eDirection = cmCapReceive;
	CSipChannel* pChannel = NULL;
	for(int i = 0 ; i < MAX_SIP_CHANNELS; i++)
	{
		pChannel = m_channelsPtrArr[i];
		if(pChannel && (pChannel->m_eDirection & eDirection))
		{
			pChannel->CleanIceCaps();
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipCall::SetIsTipCall(BYTE bTipCall)
{
	PTRACE2INT(eLevelInfoNormal,"CSipCall::SetIsTipCall : ", bTipCall);
	m_bIsTipCall = bTipCall;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CSipCall::GetIsTipCall() const
{
	return m_bIsTipCall;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCall::AreAllDtlsChannelsConnectionState(EConnectionState eState) const
{
	BYTE 			res 	= YES;

	CSipChannel* 	pChannel = NULL;

	int numOfChannels = GetNumOfChannels();

	for (int i = 0; i < numOfChannels; i++)
	{
		pChannel = GetChannel(i, true);	//amirk-rebase

		if (pChannel)
		{
			PTRACE2INT(eLevelInfoNormal,"CSipCall::AreAllDtlsChannelsConnectionState: channel no. - ", i);
			PTRACE2INT(eLevelInfoNormal,"CSipCall::AreAllDtlsChannelsConnectionState: channel state - ", pChannel->m_eDtlsConnectionState);

			res = (pChannel->m_eDtlsConnectionState == eState)? YES: NO;

			if (res == NO)
			{
				PTRACE2INT(eLevelInfoNormal,"CSipCall::AreAllDtlsChannelsConnectionState: channel is not in required state, channel no. - ", i);
				break;
			}
		}
	}

	return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCall::AreAllDtlsChannelsNotInConnectionState(EConnectionState eState) const
{
	BYTE 			res 	= YES;

	CSipChannel* 	pChannel = NULL;

	int numOfChannels = GetNumOfChannels();

	for (int i = 0; i < numOfChannels; i++)
	{
		pChannel = GetChannel(i, true);

		if (pChannel)
		{
			PTRACE2INT(eLevelInfoNormal,"CSipCall::AreAllDtlsChannelsNotInConnectionState: channel no. - ", i);
			PTRACE2INT(eLevelInfoNormal,"CSipCall::AreAllDtlsChannelsNotInConnectionState: channel state - ", pChannel->m_eDtlsConnectionState);

			res = (pChannel->m_eDtlsConnectionState == eState)? NO: YES;

			if (res == NO)
			{
				PTRACE2INT(eLevelInfoNormal,"CSipCall::AreAllDtlsChannelsNotInConnectionState: channel is not in required state, channel no. - ", i);
				break;
			}
		}
	}

	return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCall::AreAllDtlsChannelsConnected() const
{
	BYTE 			res 	= YES;

	CSipChannel* 	pChannel = NULL;

	int numOfChannels = GetNumOfChannels();

	for (int i = 0; i < numOfChannels; i++)
	{
		pChannel = GetChannel(i, true); //amirk-rebase

		if (pChannel)
		{
			PTRACE2INT(eLevelInfoNormal,"CSipCall::AreAllDtlsChannelsConnected: channel no. - ", i);
			PTRACE2INT(eLevelInfoNormal,"CSipCall::AreAllDtlsChannelsConnected: channel state - ", pChannel->m_eDtlsConnectionState);

			if ((pChannel->m_eDtlsConnectionState == kConnecting) || (pChannel->m_eDtlsConnectionState == kDisconnecting))
			{
				PTRACE2INT(eLevelInfoNormal,"CSipCall::AreAllDtlsChannelsConnected: channel is not in required state, channel no. - ", i);
				return FALSE;
			}
		}
	}

	return res;
}
//////////////////////////////////////////////////////////
BYTE CSipCall::IsOnlyOneDtlsChannelDisconnectionState(EConnectionState eState) const
{
	int  nDisconnectingChannels = 0;

	for (int i = 0; i < MAX_SIP_CHANNELS; i++)
	{
		if ( m_channelsPtrArr[i] )
		{
			if (m_channelsPtrArr[i]->m_eDtlsConnectionState == eState)
			{
				nDisconnectingChannels++;
			}
		}
	}

	if (nDisconnectingChannels == 1)
		return TRUE;
	else
		return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////
//CSipChannel
CSipChannel::CSipChannel()
{
	m_pCall = NULL;
	m_arrIndexInCall = 0xFFFFFF;
	m_mcmsIndex = 0xFFFFFF;
	m_cardIndex = 0xFFFFFF;
	m_cardEntry = 0xFFFFFF;
	m_rtpConnectionId = INVALID;
	m_eMediaType = cmCapEmpty;
	m_eDirection = cmCapReceiveAndTransmit;
	m_eRole = kRolePeople;
	m_eConnectionState = kDisconnected;
	m_eCmConnectionState = kDisconnected;
	m_eDtlsConnectionState = kDisconnected;
	m_eRtpConnectionState = kDisconnected;
	//m_eStreamState = kDisconnected;
	m_bIsMuted = NO;
	m_eAlgorithm = eUnknownAlgorithemCapCode;
	m_payloadType = _UnKnown;
	m_currentRate = 0;
	m_pData = NULL;
	m_dataLength = 0;
	m_pSameSessionChannel = NULL;
	m_SeqNumRtp = 0;
	m_SeqNumCm = 0;
    m_SeqNumMrmp = 0;
	m_isSupportLpr = 0;
	m_eDiffPayloadState = kDiffPayload_NotNeeded;
	m_numOfIceCaps = 0;

	for(int i=0;i<MAX_MEDIA_CAPSETS;i++)
	{
		m_IceCapList[i] = NULL;
	}
	m_pSdesData = NULL;
	m_pDtlsData = NULL;

	memset((void*) &m_address, 0, sizeof(mcTransportAddress));
    memset((void*) &m_RmtAddress, 0, sizeof(mcTransportAddress));
	memset(m_tosValue, 0, sizeof(int) * NumberOfTosValues);

	// TIP
	m_tipBitRate = 0;

	//LYNC2013_FEC_RED:
	m_payloadTypeFec = _UnKnown;
	m_payloadTypeRed = _UnKnown;

	// soft mcu
	//m_channelHandle=0;
	m_channelHandle = INVALID_CHANNEL_HANDLE; // Eyal: init channelHandle with '-1' for scenario where retrieving NACK for FECC

	m_streams.clear();
	m_rtcpPort = 0;
	m_rtcpRmtPort = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
CSipChannel::~CSipChannel()
{
	PDELETEA(m_pData);
	if(m_pSdesData) {
		m_pSdesData->FreeStruct();
		POBJDELETE(m_pSdesData);
	}
	if(m_pDtlsData) {
		m_pDtlsData->FreeStruct();
		POBJDELETE(m_pDtlsData);
	}

	CleanIceCaps();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::SetCmConnectionState(EConnectionState eNewState)
{
	m_eCmConnectionState = eNewState;
	if(!IsMediaChannel())
		m_eRtpConnectionState = eNewState;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::CleanIceCaps()
{
	m_numOfIceCaps = 0;
	for(int i = 0; i < MAX_MEDIA_CAPSETS; i++)
	{
	   PDELETEA(m_IceCapList[i]);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::SetSameSessionChannel(CSipChannel* pSameSessionChannel)
{
	m_pSameSessionChannel = pSameSessionChannel;
	if (pSameSessionChannel)
		pSameSessionChannel->m_pSameSessionChannel = this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::SetData(const BYTE data[], int length)
{
	PDELETEA(m_pData);
	if (length)
	{
		m_pData = new BYTE[length];
		memcpy(m_pData,data,length);
	}
	else
	{
		m_pData = NULL;
	}
	m_dataLength = length;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
CBaseCap* CSipChannel::GetDataAsCapClass() const
{
	CBaseCap* pCap = CBaseCap::AllocNewCap(m_eAlgorithm,m_pData);
	return pCap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipChannel::IsChannelType(EIpChannelType eChanType) const
{
	BYTE bIsTransmit = (m_eDirection == cmCapTransmit);
	EIpChannelType eTempChanType = CalcChannelType(m_eMediaType,bIsTransmit,m_eRole);
	BYTE bRes = (eTempChanType == eChanType);
	
	return bRes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::SetChannelQualities(CapEnum eAlgorithm,const BYTE data[],int length)
{
	CCapSetInfo capInfo  = eAlgorithm;
	payload_en	payload  = capInfo.GetPayloadType();
	if (GetMediaType() == cmCapVideo && (GetRoleLabel() & kRoleContentOrPresentation) && eAlgorithm == eH263CapCode)
	{
		BYTE isContentH263L = 2;
		payload = (payload_en)capInfo.GetDynamicPayloadType(isContentH263L);
	}

	DWORD currentRate = 0;

	if (capInfo.IsType(cmCapAudio))
		currentRate = capInfo.GetBitRate((BYTE *)data)/_K_;
	else if (capInfo.IsType(cmCapVideo))
	{
		CBaseVideoCap* pVideo = (CBaseVideoCap *)CBaseCap::AllocNewCap(eAlgorithm,(BYTE *)data);
		currentRate = pVideo? pVideo->GetBitRate(): 0;

		if((NULL == data) && pVideo){
			pVideo->FreeStruct();
		}

		POBJDELETE(pVideo);
	}
	else if (capInfo.IsType(cmCapData))
	{
		CBaseDataCap* pData = (CBaseDataCap *)CBaseCap::AllocNewCap(eAlgorithm,(BYTE *)data);
		currentRate = pData? pData->GetBitRate(): 0;
		POBJDELETE(pData);
	}

	SetAlgorithm(eAlgorithm);
	SetPayloadType(payload);
	SetCurrentRate(currentRate);
	SetData(data,length);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipChannel::IsChannelDynamicPayloadType()
{
	BYTE bIsDynamic = ::IsDynamicPayloadType(m_payloadType);
	return bIsDynamic;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::SetChannelSdes(CSdesCap *pSdesCap)
{
	if(pSdesCap) {
		if(m_pSdesData) {
			m_pSdesData->FreeStruct();
			POBJDELETE(m_pSdesData);
		}

		sdesCapStruct* pStruct = (sdesCapStruct*)(new BYTE[pSdesCap->SizeOf()]);
		memcpy (pStruct, pSdesCap->GetStruct(), pSdesCap->SizeOf());
		m_pSdesData = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pStruct);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CSipChannel::GetSdesCapLen()
{
	APIU32 len = 0;
	if(m_pSdesData) {
		len = m_pSdesData->SizeOf();
	}

	return len;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CSdesCap* CSipChannel::GetChannelSdes()
{
	return m_pSdesData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipChannel::IsChannelSdesEnabled()
{
	if(m_pSdesData)
		return TRUE;
	else
		return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::RemoveChannelSdes()
{
	if(m_pSdesData) {
		m_pSdesData->FreeStruct();
		POBJDELETE(m_pSdesData);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::SetChannelDtls(CDtlsCap *pDtlsCap)
{
	if(pDtlsCap) {
		if(m_pDtlsData) {
			m_pDtlsData->FreeStruct();
			POBJDELETE(m_pDtlsData);
		}

		sdesCapStruct* pStruct = (sdesCapStruct*)(new BYTE[pDtlsCap->SizeOf()]);
		memcpy (pStruct, pDtlsCap->GetStruct(), pDtlsCap->SizeOf());
		m_pDtlsData = (CDtlsCap*)CBaseCap::AllocNewCap((CapEnum)eDtlsCapCode,pStruct);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
APIU32 CSipChannel::GetDtlsCapLen()
{
	APIU32 len = 0;
	if(m_pDtlsData) {
		len = m_pDtlsData->SizeOf();
	}

	return len;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CDtlsCap* CSipChannel::GetChannelDtls()
{
	return m_pDtlsData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipChannel::IsChannelDtlsEnabled()
{
	if(m_pDtlsData)
		return TRUE;
	else
		return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::RemoveChannelDtls()
{
	if(m_pDtlsData) {
		m_pDtlsData->FreeStruct();
		POBJDELETE(m_pDtlsData);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::SetStreamsList(const std::list <StreamDesc>&  rStreams, APIU32 aSsrcId)
{
    TRACEINTO << "mix_mode: aSsrcId = " << aSsrcId;

    m_streams.clear();

    if (aSsrcId == INVALID)
    {
        m_streams.assign(rStreams.begin(), rStreams.end());
        Dump("CSipChannel::SetStreamsList");
		
        return;
    }

    std::list<StreamDesc>::const_iterator itr_streams;
	
    for (itr_streams = rStreams.begin(); itr_streams != rStreams.end(); itr_streams++)
    {
        if (itr_streams->m_pipeIdSsrc == aSsrcId)
        {
            TRACEINTO << "Adding stream " << aSsrcId << " to channel";
            m_streams.push_back(*itr_streams);
        }
    }
	
    Dump("CSipChannel::SetStreamsList");
}

//CSipAudioChannel
////////////////////////////////////////////////////////////////////////////////////////////////////
CSipAudioChannel::CSipAudioChannel():m_dtmfPayloadType(_UnKnown)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::SetIceData(CSipCaps* Caps)
{
	m_numOfIceCaps = 0;
	capBuffer* LocalCapBuffer = NULL;

	if(m_eMediaType == cmCapAudio)
		m_numOfIceCaps = Caps->GetNumOfIceAudioCaps();
	else if((m_eMediaType == cmCapVideo) && (m_eRole == kRolePeople))
		m_numOfIceCaps = Caps->GetNumOfIceVideoCaps();
	else if(m_eMediaType == cmCapData)
		m_numOfIceCaps= Caps->GetNumOfIceDataCaps();

	for(int i=0;i<m_numOfIceCaps;i++)
	{
		if(m_eMediaType == cmCapAudio)
			LocalCapBuffer= Caps->GetIceAudioCapList(i);

		if((m_eMediaType == cmCapVideo) && (m_eRole == kRolePeople))
			LocalCapBuffer= Caps->GetIceVideoCapList(i);

		if(m_eMediaType == cmCapData)
			LocalCapBuffer= Caps->GetIceDataCapList(i);

		PASSERTMSG_AND_RETURN(!LocalCapBuffer, "CSipChannel::SetIceData - Failed, 'LocalCapBuffer' is NULL");

		int length = LocalCapBuffer->capLength;
		//(CapEnum)pCapBuffer->capTypeCode,pCapBuffer->dataCap,pCapBuffer->capLength)
		m_IceCapList[i] = (capBuffer *)new BYTE[sizeof(capBufferBase) + length];
		memset(m_IceCapList[i], 0, sizeof(capBufferBase) + length);

		m_IceCapList[i]->capTypeCode = LocalCapBuffer->capTypeCode;
		m_IceCapList[i]->capLength =  LocalCapBuffer->capLength;
		m_IceCapList[i]->sipPayloadType = LocalCapBuffer->sipPayloadType;
		if (LocalCapBuffer->capLength != 0)
			memcpy(m_IceCapList[i]->dataCap,LocalCapBuffer->dataCap,LocalCapBuffer->capLength);

	}

}
///////////////////////////////////////////////////////////////////////////////////////////////////
WORD CSipChannel::GetNumOfIceCaps()
{
	return m_numOfIceCaps;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
capBuffer* CSipChannel::GetIceCapBuffer(WORD index)
{
	return m_IceCapList[index];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChannel::Dump(char * header)
{
	CSuperLargeString str;
    str << "CSipChannel::Dump " << header << "\n";

    str << "m_arrIndexInCall      = " << m_arrIndexInCall << "\n"
        << "m_mcmsIndex           = " << m_mcmsIndex << "\n"
        << "m_eMediaType          = " << ::GetTypeStr(m_eMediaType) << "\n"
        << "m_eDirection          = " << ::GetDirectionStr(m_eDirection) << "\n"
        << "m_eRole               = " << ::GetRoleStr(m_eRole) << "\n"
        << "m_eConnectionState    = " << ConnectionStateToString(m_eConnectionState) << "\n"
        << "m_eRtpConnectionState = " << ConnectionStateToString(m_eRtpConnectionState) << "\n"
        << "m_eCmConnectionState  = " << ConnectionStateToString(m_eCmConnectionState) << "\n"
        << "m_eAlgorithm  = "         << CapEnumToString(m_eAlgorithm) << "\n";

    if (m_rtpConnectionId != INVALID)
        str << "m_rtpConnectionId  = " << m_rtpConnectionId << "\n";

     if (m_pSdesData)
     {
         COstrStream msg;
			m_pSdesData->Dump(msg);
			str << msg.str().c_str();
     }
     if(m_streams.size()!=0)
     {
         COstrStream msg1;
    	 DumpStreamsList(msg1);
    	 str << msg1.str().c_str();
     }
     else {
         str << "There are no streams \n";
     }

     if (m_pDtlsData)
	{
	  COstrStream msg;
	  m_pDtlsData->Dump(msg);
		str << msg.str().c_str();
	}

    PTRACE (eLevelInfoNormal, str.GetString());


}

void CSipChannel::ShortDump(char *header)
{
    CLargeString str;
    str << "CSipChannel::Dump " << header << " ";

    str << "m_eMediaType          = " << m_eMediaType << "\n"
        << "m_eDirection          = " << m_eDirection << "\n"
        << "m_eRole               = " << m_eRole << "\n"
        << "m_eConnectionState    = " << m_eConnectionState << "\n"
        << "m_eRtpConnectionState = " << m_eRtpConnectionState << "\n"
        << "m_eCmConnectionState  = " << m_eCmConnectionState << "\n";
    PTRACE (eLevelInfoNormal, str.GetString());
}

void CSipChannel::SetConnectionState(EConnectionState eNewState)
{
    m_eConnectionState = eNewState;
    TRACEINTO << "Channel state updated to " << ConnectionStateToString(eNewState);
    this->Dump("CSipChannel::SetConnectionState");
}


void CSipChannel::DumpStreamsList(std::ostream& msg)
{
	if (m_streams.empty())
		return;

	msg << "\n- - - - Streams list : - - - - - - - -" << "\n";
	std::list<StreamDesc>::const_iterator itr_streams;
	for (itr_streams = m_streams.begin();itr_streams != m_streams.end();itr_streams++)
	{
		msg << "ssrc:" << itr_streams->m_pipeIdSsrc;
		if (GetMediaType()==cmCapVideo)
		{
			msg << " width:" << itr_streams->m_width;
			msg << " height:" << itr_streams->m_height;
			msg << " frame rate:" << itr_streams->m_frameRate;
		}
		msg << " m_bitRate:" << itr_streams->m_bitRate;
		if (itr_streams->m_specificSourceSsrc)
			msg << " specified ssrc:" << itr_streams->m_sourceIdSsrc;

		msg << " payload type:" << itr_streams->m_payloadType;
		msg << " priority: " << itr_streams->m_priority;
        msg << " isLegal: " << itr_streams->m_isLegal;
        msg << " isAvcToSvcVsw: " << itr_streams->m_isAvcToSvcVsw;

		msg << "\n";
	}

}


//CSipChanDif
////////////////////////////////////////////////////////////////////////////////////////////////////
CSipChanDif::CSipChanDif():
m_eType((EIpChannelType)0),m_nNumOfChanges(0),
m_bAdd(NO),m_bRemove(NO),
m_bChangeIp(NO),m_bChangePort(NO),m_bChangeRtcpPort(NO),
m_bMute(NO),m_bUnMute(NO),
m_bChangeAlg(NO),m_bChangePayload(NO),
m_bChangeAudFpp(NO),
m_bChangeVidRate(NO),m_bChangeVidResolution(NO),m_bChangeVidFps(NO),m_bAddVidAnnex(NO),m_bRemoveVidAnnex(NO),m_bChangeLpr(NO),m_bChangeMSSsrc(NO),m_bChangeSdes(NO),
m_bChangeBfcpTransportProtocol(NO), m_bChangeQoS(NO)//,m_bChangeDtls(NO)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::Serialize(WORD format,CSegment& seg) const
{
    switch (format)
    {
    case NATIVE:
		seg << ((WORD)m_eType);
		seg << m_nNumOfChanges;
		seg << m_bAdd;
		seg << m_bRemove;
		seg << m_bChangeIp;
		seg << m_bChangePort;
		seg << m_bChangeRtcpPort;
		seg << m_bMute;
		seg << m_bUnMute;
		seg << m_bChangeAlg;
		seg << m_bChangePayload;
		seg << m_bChangeAudFpp;
		seg << m_bChangeVidRate;
		seg << m_bChangeVidResolution;
		seg << m_bChangeVidFps;
		seg	<< m_bAddVidAnnex;
		seg << m_bRemoveVidAnnex;
		seg << m_bChangeLpr;
		seg << m_bChangeMSSsrc;
		seg << m_bChangeSdes;
		seg << m_bChangeQoS;
		seg << m_bChangeBfcpTransportProtocol;
//		seg << m_bChangeDtls;

        break;

    default :
        break;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::DeSerialize(WORD format,CSegment& seg)
{
	WORD type;
    switch (format)
    {
    case NATIVE:
		seg >> type;
		m_eType = (EIpChannelType)type;
		seg >> m_nNumOfChanges;
		seg >> m_bAdd;
		seg >> m_bRemove;
		seg >> m_bChangeIp;
		seg >> m_bChangePort;
		seg >> m_bChangeRtcpPort;
		seg >> m_bMute;
		seg >> m_bUnMute;
		seg >> m_bChangeAlg;
		seg >> m_bChangePayload;
		seg >> m_bChangeAudFpp;
		seg >> m_bChangeVidRate;
		seg >> m_bChangeVidResolution;
		seg >> m_bChangeVidFps;
		seg	>> m_bAddVidAnnex;
		seg >> m_bRemoveVidAnnex;
		seg >> m_bChangeLpr;
		seg >> m_bChangeMSSsrc;
		seg >> m_bChangeSdes;
		seg >> m_bChangeQoS;
		seg >> m_bChangeBfcpTransportProtocol;
//		seg >> m_bChangeDtls;

        break;

    default :
        break;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::DumpToString(CObjString& string) const
{
	string << ::GetIpChanTypeStr(m_eType) << "- " << m_nNumOfChanges << " change(s):";
	DWORD oldStringLength = string.GetStringLength();
	string <<
	(m_bAdd?		" media added":			"") <<
	(m_bRemove?		" media removed":		"") <<
	(m_bChangeIp?	" ip changed":			"") <<
	(m_bChangePort? " port changed":			"") <<
	(m_bChangeRtcpPort? " rtcp port changed":	"") <<
	(m_bMute?		" media muted":			"") <<
	(m_bUnMute?		" media un-muted":		"") <<
	(m_bChangePayload?	" payload changed":	"") <<
	(m_bChangeAudFpp?	" fpp changed":		"") <<
	(m_bChangeLpr?	" lpr changed":		"") <<
	(m_bChangeMSSsrc?	" MS ssrc changed":		"") <<
	(m_bChangeQoS?	" QoS changed":		"") <<
	((m_bChangeAlg||m_bChangeVidRate||m_bChangeVidResolution||m_bChangeVidFps||m_bAddVidAnnex||m_bRemoveVidAnnex||m_bChangeSdes||m_bChangeBfcpTransportProtocol) ?// || m_bChangeDtls)?
	" change mode":"");
	if(oldStringLength == string.GetStringLength())
		string << " No Change";
}


////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipChanDif::IsChange(BYTE (CSipChanDif::*isFunction) (void) const) const
{
	BYTE res = (this->*isFunction) ();
	return res;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
EIpChannelType  CSipChanDif::GetChannelType()	const
{
	return m_eType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
WORD			CSipChanDif::GetNumOfChanges()	const
{
	return m_nNumOfChanges;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsAddChannel()		const
{
	return m_bAdd;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsRemoveChannel()   const
{
	return m_bRemove;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsChangeIp()		const
{
	return m_bChangeIp;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsChangePort()		const
{
	return m_bChangePort;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsChangeRtcpPort()		const
{
	return m_bChangeRtcpPort;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsMute()			const
{
	return m_bMute;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsUnMute()			const
{
	return m_bUnMute;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsChangeAlg()		const
{
	return m_bChangeAlg;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsChangeBfcpTransportProtocol()		const
{
	return m_bChangeBfcpTransportProtocol;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsChangePayload()	const
{
	return m_bChangePayload;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsChangeAudFpp()	const
{
	return m_bChangeAudFpp;
}
////////////////////////////////////////////////////////////
BYTE         CSipChanDif::IsChangeLpr()          const
{
	return m_bChangeLpr;
}
////////////////////////////////////////////////////////////
BYTE         CSipChanDif::IsChangeMSSsrc()       const
{
	return m_bChangeMSSsrc;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsChangeSdes()		const
{
	return m_bChangeSdes;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE			CSipChanDif::IsChangeQoS()		const
{
	return m_bChangeQoS;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//BYTE			CSipChanDif::IsChangeDtls()		const
//{
//	return m_bChangeDtls;
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
// add here more 'Get' functions
void CSipChanDif::SetDetails(WORD details)
{
	SetChangeAlg(details & DIFFERENT_CAPCODE);
	// to add functions
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetChannelType(EIpChannelType t)
{
	m_eType = t;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetAddChannel(BYTE yesno)
{
	if (m_bAdd != yesno)
	{
		m_bAdd = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetRemoveChannel(BYTE yesno)
{
	if (m_bRemove != yesno)
	{
		m_bRemove = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetChangeIp(BYTE yesno)
{
	if (m_bChangeIp != yesno)
	{
		m_bChangeIp = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetChangePort(BYTE yesno)
{
	if (m_bChangePort != yesno)
	{
		m_bChangePort = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetChangeRtcpPort(BYTE yesno)
{
	if (m_bChangeRtcpPort != yesno)
	{
		m_bChangeRtcpPort = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetMute(BYTE yesno)
{
	if (m_bMute != yesno)
	{
		m_bMute = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetUnMute(BYTE yesno)
{
	if (m_bUnMute != yesno)
	{
		m_bUnMute = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetChangeBfcpTransportProtocol(BYTE yesno)
{
	if (m_bChangeBfcpTransportProtocol != yesno)
	{
		m_bChangeBfcpTransportProtocol = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetChangeAlg(BYTE yesno)
{
	if (m_bChangeAlg != yesno)
	{
		m_bChangeAlg = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}
///////////////////////////////////////////////
void CSipChanDif::SetChangeLpr(BYTE yesno)
{
	if (m_bChangeLpr != yesno)
	{
		m_bChangeLpr = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}

}
///////////////////////////////////////////////
void CSipChanDif::SetChangeMSSsrc(BYTE yesno)
{
	if (m_bChangeMSSsrc != yesno)
	{
		m_bChangeMSSsrc = yesno;
		yesno ? m_nNumOfChanges++: m_nNumOfChanges--;
	}

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetChangePayload(BYTE yesno)
{
	if (m_bChangePayload != yesno)
	{
		m_bChangePayload = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetChangeAudFpp(BYTE yesno)
{
	if (m_bChangeAudFpp != yesno)
	{
		m_bChangeAudFpp = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetChangeSdes(BYTE yesno)
{
	if (m_bChangeSdes != yesno)
	{
		m_bChangeSdes = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//void CSipChanDif::SetChangeDtls(BYTE yesno)
//{
//	if (m_bChangeDtls != yesno)
//	{
//		m_bChangeDtls = yesno;
//		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
//	}
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDif::SetChangeQoS(BYTE yesno)
{
	if (m_bChangeQoS != yesno)
	{
		m_bChangeQoS = yesno;
		yesno? m_nNumOfChanges++: m_nNumOfChanges--;
	}
}


//******************************************************
// 			Add here more 'Set' functions
//******************************************************


//CSipChanDif
////////////////////////////////////////////////////////////////////////////////////////////////////
CSipChanDifArr::CSipChanDifArr()
{
	BYTE			bIsTransmit		= NO;
	int				arrInd			= 0;

	cmCapDataType mediaType;
	ERoleLabel eRole;
	for (int i=0 ; i<CHANNEL_TYPES_COUNT; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		for (int j=0; j<2; j++)
		{
			bIsTransmit = (globalDirectionArr[j] == cmCapTransmit);
			EIpChannelType type = ::CalcChannelType(mediaType,bIsTransmit,eRole);
			m_arr[arrInd++].SetChannelType(type);
		}
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDifArr::Serialize(WORD format,CSegment& seg) const
{
	for (int i=0; i<MAX_SIP_CHANNELS; i++)
		m_arr[i].Serialize(format,seg);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDifArr::DeSerialize(WORD format,CSegment& seg)
{
	for (int i=0; i<MAX_SIP_CHANNELS; i++)
		m_arr[i].DeSerialize(format,seg);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChanDifArr::DumpToString(CObjString& string) const
{
	for (int i=0; i<MAX_SIP_CHANNELS; i++)
	{
		m_arr[i].DumpToString(string);
		string<<"\n";
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////
CSipChanDif* CSipChanDifArr::operator[] (int i)
{
	CSipChanDif* res = NULL;
	if (i < MAX_SIP_CHANNELS)
		res = &(m_arr[i]);
	return res;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
CSipChanDif* CSipChanDifArr::GetChanDif(EIpChannelType type)
{
	CSipChanDif* res = NULL;
	for (int i=0 ; i<MAX_SIP_CHANNELS && res==NULL; i++)
	{
		if (m_arr[i].GetChannelType() == type)
			res = &(m_arr[i]);
	}
	return res;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
CSipChanDif* CSipChanDifArr::GetChanDif(cmCapDataType eMediaType,cmCapDirection eDirection,ERoleLabel eRole)
{
	EIpChannelType	type = ::CalcChannelType(eMediaType,(eDirection==cmCapTransmit),eRole);
	CSipChanDif*	res  = GetChanDif(type);
	return res;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipChanDifArr::IsChange(BYTE (CSipChanDif::*isFunction) (void) const,int arrSize,EIpChannelType chanArr[])
{
	BYTE			res = NO;
	CSipChanDif*	pChanDif = NULL;

	for (int i=0; i<arrSize && res==NO; i++)
	{
		if (arrSize == MAX_SIP_CHANNELS)
			pChanDif = &(m_arr[i]);
		else if (chanArr)
			pChanDif = GetChanDif(chanArr[i]);
		else
			break;

		res = pChanDif->IsChange(isFunction);
	}

	return res;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
int CSipChanDifArr::SetArrOfChanges(BYTE (CSipChanDif::*isFunction) (void) const,int& arrSize,EIpChannelType chanArr[])
{
	arrSize = 0;
	for(int i=0; i<MAX_SIP_CHANNELS; i++)
	{
		if(m_arr[i].IsChange(isFunction))
			chanArr[arrSize++] = m_arr[i].GetChannelType();
	}
	return arrSize;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCall::SetCallIndex(DWORD callIndex)
{
	m_callIndex = callIndex;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CSipCall::GetCallIndex ()
{
	return m_callIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// TIP -----------------------------------------------------------------------
//////////////////////////////////////////////////////////
DWORD CSipChannel::GetTipBitRate() const
{
	return m_tipBitRate;
}

//////////////////////////////////////////////////////////
void CSipChannel::SetTipBitRate(DWORD newRate)
{
	m_tipBitRate = newRate;
}
void 	CSipChannel::SetPayloadType(DWORD payloadType) //
{
	m_payloadType = payloadType;
/*	if(payloadType==114)
	{
		SystemCoreDump(FALSE);
	}
	if(payloadType==105)
	{
		SystemCoreDump(FALSE);
	}*/

}

//////////////////////////////////////////////////////////////////////
// Return Value: TRUE: disconnect call
//				    FALSE: not disconnect call
BYTE	CSipCall::HandleMediaDetectionInd(kChanneltype  ChannelType, BYTE  isRTP)
{
	PTRACE(eLevelInfoNormal,"CSipCall::HandleMediaDetectionInd");

	time_t now 			= time((time_t*)NULL);
	int connectionIndex	=0;
	BYTE disconnectCall	= FALSE;
	bool errFound 		= FALSE;

	switch (ChannelType)
	{
		case kIpAudioChnlType:
			{
				if(isRTP)
					m_stMediaDetectInfo.lastIndTime[MEDIA_DETECTION_AUDIO_RTP] = now;
				else
					m_stMediaDetectInfo.lastIndTime[MEDIA_DETECTION_AUDIO_RTCP] = now;
			}
			break;
		case kIpVideoChnlType:
			{
				if(FALSE == GetMediaDetectionHasVideo())
				{
					PTRACE(eLevelInfoNormal,"CSipCall::HandleMediaDetectionInd no Video Channel being monitored! Ignore this indication!");
					errFound = TRUE;
				}
				else
				{
					if(isRTP)
						m_stMediaDetectInfo.lastIndTime[MEDIA_DETECTION_VIDEO_RTP] = now;
					else
						m_stMediaDetectInfo.lastIndTime[MEDIA_DETECTION_VIDEO_RTCP] = now;
				}
			}
			break;
		default:
			PTRACE(eLevelInfoNormal,"CSipCall::HandleMediaDetectionInd -- NOT the expected indication!");
			break;
	}

	int	MaxMonitoredConnection	= MEDIA_DETECTION_MAX_CONNECTION;
	if(!errFound && !GetMediaDetectionHasVideo())
	{
		MaxMonitoredConnection = MEDIA_DETECTION_VIDEO_RTCP;
		//Only Audio connections, set the max connection number as the first Video connection
	}
	else
	{
		if(TRUE == GetMediaDetectionIsSvcOpened())
		{
			MaxMonitoredConnection = MEDIA_DETECTION_VIDEO_RTP;
			//SVC calls only check the RTCP for video
		}
		else
		{
			MaxMonitoredConnection	= MEDIA_DETECTION_MAX_CONNECTION;
			//the default value, check all the 4 channels for AVC video calls
		}
	}

	for(connectionIndex=0; !errFound && connectionIndex<MaxMonitoredConnection && 
	                       m_stMediaDetectInfo.MediaDisconnectedRecently(connectionIndex, now);
						   connectionIndex++)
	{
		if(now < m_stMediaDetectInfo.lastIndTime[connectionIndex])
		{
			PTRACE2INT(eLevelError, "CSipCall::HandleMediaDetectionInd -- ERROR!-- ", m_stMediaDetectInfo.lastIndTime[connectionIndex]);
			errFound = TRUE;
		}
	}

	if(!errFound && MaxMonitoredConnection == connectionIndex)
	{
		PTRACE(eLevelInfoNormal, "CSipCall::HandleMediaDetectionInd -- Disconnect call!!!!");
		disconnectCall = TRUE;
	}
	
	return disconnectCall;
}

//--------------------------------------------------------------------------
const char* ConnectionStateToString(EConnectionState connectionState)
{
	switch (connectionState)
	{
		case kUnknown                      : return "kUnknown";
		case kDisconnected                 : return "kDisconnected";
		case kConnected                    : return "kConnected";
		case kDisconnecting                : return "kDisconnecting";
		case kConnecting                   : return "kConnecting";
		case kUpdating                     : return "kUpdating";
		default                            : return "Invalid_EConnectionState";
	}
}
