#include "Macros.h"
#include "NStream.h"
#include "DataTypes.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "CapClass.h"
#include "Capabilities.h"
#include "SipScm.h"
#include "CommModeInfo.h"
#include "H264.h"

////////////////////////////////////////
CSipComMode::CSipComMode()
{
}

///////////////////////////////////////////////
CSipComMode::~CSipComMode()
{
}

/////////////////////////////////////////////////////////////////////////////
CSipComMode::CSipComMode(const CSipComMode& other):CIpComMode(other)
{
}

/////////////////////////////////////////////////////////////////////////////////
CSipComMode& CSipComMode::operator= (const CSipComMode &other)
{
	if(this != &other)
	{
		(CIpComMode&)*this = (CIpComMode&)other;
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sets only the selected caps as com mode.
// for instance if audioIndex == 0 sets the first audio cap that found.
// if audioIndex is greater than max audio caps, sets the last audio cap found.
//---------------------------------------------------------------------------
/*
void CSipComMode::Create(const sipSdpAndHeadersSt & sdp,cmCapDirection eDirection,int audioIndex,int videoIndex, int dataIndex)
{
	sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *) &sdp.capsAndHeaders[sdp.sipMediaLinesOffset];
	int mediaLinePos = 0;

	for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {

		const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
		mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

		const capBuffer * pCapBuffer = (capBuffer *) &pMediaLine->caps[0];
		const BYTE		* pTemp = (const BYTE *)pCapBuffer;
		int audioCapFound = 0;
		int videoCapFound = 0;
		int dataCapFound = 0;

		for (unsigned int i=0 ; i<pMediaLine->numberOfCaps; i++)
		{
			CCapSetInfo capInfo = (CapEnum)pCapBuffer->capTypeCode;
			cmCapDataType eType = capInfo.GetCapType();

			if ((eType == cmCapAudio && audioIndex >= audioCapFound)||
				(eType == cmCapVideo && videoIndex >= videoCapFound)||
				(eType == cmCapData && dataIndex >= dataCapFound))
			CIpComMode::SetMediaMode(pCapBuffer,eType,eDirection);

			if (eType == cmCapAudio) audioCapFound++;
			else if (eType == cmCapVideo) videoCapFound++;
			else if (eType == cmCapData) dataCapFound++;

			pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer *)pTemp;
		}
		PASSERT(321);

	}
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sets only the selected caps as com mode.
// for instance if audioIndex == 0 sets the first audio cap that found.
// if audioIndex is greater than max audio caps, sets the last audio cap found.
//---------------------------------------------------------------------------
void CSipComMode::Create(const sipSdpAndHeadersSt & sdp,cmCapDirection eDirection,int audioIndex,int videoIndex, int dataIndex, int contentIndex)
{
	sipMediaLinesEntrySt *pMediaLinesEntry = (sipMediaLinesEntrySt *) &sdp.capsAndHeaders[sdp.sipMediaLinesOffset];
	int mediaLinePos = 0;

	for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {

		const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
		mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

		const capBuffer * pCapBuffer = (capBuffer *) &pMediaLine->caps[0];
		const BYTE		* pTemp = (const BYTE *)pCapBuffer;
		int audioCapFound = 0;
		int videoCapFound = 0;
		int dataCapFound = 0;
		int contentCapFound = 0;

		for (unsigned int i=0 ; i<pMediaLine->numberOfCaps; i++)
		{
			CCapSetInfo capInfo = (CapEnum)pCapBuffer->capTypeCode;
			cmCapDataType eType = capInfo.GetCapType();
			ERoleLabel eRole = (ERoleLabel)((BaseCapStruct*)pCapBuffer->dataCap)->header.roleLabel;

			if ((eType == cmCapAudio && audioIndex >= audioCapFound)||
				(eType == cmCapVideo && eRole == kRolePeople && videoIndex >= videoCapFound)||
				(eType == cmCapVideo && eRole == kRolePresentation && contentIndex >= contentCapFound)||
				(eType == cmCapData && dataIndex >= dataCapFound))
			CIpComMode::SetMediaMode(pCapBuffer,eType,eDirection,eRole);

			if (eType == cmCapAudio) audioCapFound++;
			else if (eType == cmCapVideo && eRole == kRolePeople) videoCapFound++;
			else if (eType == cmCapVideo && eRole == kRolePresentation) contentCapFound++;
			else if (eType == cmCapData) dataCapFound++;

			pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer *)pTemp;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sets only the selected caps as com mode.
// for instance if audioIndex == 0 sets the first audio cap that found.
// if audioIndex is greater than max audio caps, sets the last audio cap found.
//---------------------------------------------------------------------------

void CSipComMode::Create(const CSipCaps & caps,cmCapDirection eDirection,int audioIndex,int videoIndex, int dataIndex, int contentIndex)
{
	for (int i=0 ; i<caps.GetNumOfMediaCapSets(cmCapAudio); i++)
	{
		if(audioIndex >= i)
		{
			const capBuffer * pCapBuffer = caps.GetCapSetAsCapBuffer(cmCapAudio,i);
			if (pCapBuffer)
				CIpComMode::SetMediaMode(pCapBuffer,cmCapAudio,eDirection);
		}
	}
	for (int i = 0 ; i < caps.GetNumOfMediaCapSets(cmCapVideo); i++)
	{
		if ( videoIndex >= i )
		{
			const capBuffer * pCapBuffer = caps.GetCapSetAsCapBuffer(cmCapVideo,i);

			if ( pCapBuffer )
			{
				CIpComMode::SetMediaMode(pCapBuffer,cmCapVideo,eDirection);
			}
		}
	}
	for (int i = 0 ; i < caps.GetNumOfMediaCapSets(cmCapData); i++)
	{
		if ( dataIndex >= i )
		{
			const capBuffer * pCapBuffer = caps.GetCapSetAsCapBuffer(cmCapData,i);

			if ( pCapBuffer )
			{
				CIpComMode::SetMediaMode(pCapBuffer,cmCapData,eDirection);
			}
		}
	}
	for (int i = 0 ; i < caps.GetNumOfMediaCapSets(cmCapVideo,cmCapReceiveAndTransmit,kRolePresentation); i++)
	{
		if ( contentIndex >= i )
		{
			const capBuffer * pCapBuffer = caps.GetCapSetAsCapBuffer(cmCapVideo,i,kRolePresentation);

			if ( pCapBuffer )
			{
				CIpComMode::SetMediaMode(pCapBuffer,cmCapVideo,eDirection,kRolePresentation);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////
void CSipComMode::Create(const CSipCall & call)
{
	CSipChannel* pChannel;
	BYTE* pData;
	int length;
	CapEnum capAlg;
	cmCapDataType mediaType;
	ERoleLabel eRole;

	for(int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		for (int j = 0; j < 2; j++)
		{
			pChannel = call.GetChannel(true, mediaType,globalDirectionArr[j],eRole);

			if (pChannel)
			{
				capAlg = pChannel->GetAlgorithm();
				length = pChannel->GetDataLength();
				pData = pChannel->GetData();
				CIpComMode::SetMediaMode(capAlg, length, pData, mediaType, globalDirectionArr[j],eRole);
				CIpComMode::SetPayloadTypeForMediaMode(mediaType, globalDirectionArr[j], eRole, (payload_en)pChannel->GetPayloadType());
				std::list <StreamDesc> streamsDescList = pChannel->GetStreams();
				CIpComMode::SetStreamsListForMediaMode(streamsDescList, mediaType, globalDirectionArr[j], eRole);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipComMode::CreateSipOptions(CCommConf* pCommConf,BYTE videoProtocol,BYTE partyEncryptionMode/* = AUTO*/)
{

	if(pCommConf->IsAudioConf())
	{
		SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit);
	}
	else
    {
		switch(videoProtocol)
	    {
	       case H264:
	       case AUTO:
		   {
			/*(Talya 29.11.05 Temp Patch)*///m_pUnifiedComMode->SetH264VidMode(H264_Level_2,-1,-1,-1,-1,m_pCommConf->GetVideoProtocol());
			//2000000/25000 = 80
			  SetH264Scm(H264_Profile_BaseLine,(WORD)H264_Level_1_2,-1,-1,-1,80,-1,-1,cmCapReceiveAndTransmit);
			  break;
		   }
	       case H263:
		   {
		   	  CComModeInfo lComModeInfo((WORD)videoProtocol,(WORD)StartVideoCap);
		   	  CapEnum h323VideoCapCode = lComModeInfo.GetH323ModeType();
	          SetScmMpi(h323VideoCapCode,  1, 1, 0, 0,cmCapReceiveAndTransmit);
			  break;
		   }
	       case H261:
		   {
		   	 CComModeInfo lComModeInfo((WORD)videoProtocol,(WORD)StartVideoCap);
		   	 CapEnum h323VideoCapCode = lComModeInfo.GetH323ModeType();
			 SetScmMpi(h323VideoCapCode,pCommConf->GetQCIFframeRate(),pCommConf->GetCIFframeRate(),0,0,cmCapReceiveAndTransmit);
			 break;
		   }
	    }
    }
    WORD confType = pCommConf->GetVideoSession();
	SetConfType((EConfType)confType);

	// set xfer mode
	WORD confBitRate = pCommConf->GetConfTransferRate();
	CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
	DWORD h323Rate = lCapInfo.TranslateReservationRateToIpRate(confBitRate);
	SetCallRate(h323Rate);
	//set audio mode
	WORD audRate = pCommConf->GetAudioRate();
	CComModeInfo comModeInfo(audRate, StartAudioCap);
    CapEnum h323AudioCapCode = comModeInfo.GetH323ModeType();
	SetAudioAlg(h323AudioCapCode,cmCapReceiveAndTransmit);

	if((GetIsEncrypted() == Encryp_On) && (partyEncryptionMode != NO))
	{
		CreateLocalSipComModeSdes(TRUE);
	}

}

/////////////////////////////////////////////////////////////////////////////
// If audio is opened with higher rate than expected, we should reduce the video outgoing rate.
void CSipComMode::UpdateVideoOutRateIfNeeded(const CSipComMode& rPreferredMode, DWORD vidRateTx, RemoteIdent remoteIdent, BYTE bIsMrcCall, BYTE isFecOrRedOn)
{
    if (GetConfMediaType()==eMixAvcSvcVsw && !bIsMrcCall)
    {
        DWORD actualAudioRate   = 10 * GetMediaBitRate(cmCapAudio, cmCapReceive);
        DWORD actualVideoRate   = GetMediaBitRate(cmCapVideo, cmCapReceive);
        DWORD callRate = 10 * GetCallRate();

        TRACEINTO << "actualAudioRate=" << actualAudioRate << " actualVideoRate=" << actualVideoRate << " callRate=" << callRate;
        if (callRate < actualAudioRate+actualVideoRate)
        {
            TRACEINTO << "Not enough bit rate for both audio and video! actualAudioRate=" << actualAudioRate << " actualVideoRate=" << actualVideoRate << " callRate=" << callRate;
            SetVideoBitRate(0, cmCapTransmit);
            SetVideoBitRate(0,  cmCapReceive);
            SetTotalVideoRate(0);
        }
        return;
    }

    if (GetConfType() != kCp)
        return;

    // chech for VSW stream in mix mode
    const std::list <StreamDesc> streamsDescList = GetStreamsListForMediaMode(cmCapVideo, cmCapTransmit, kRolePeople);
    std::list <StreamDesc>::const_iterator itr_streams;
    int numberOfStreams = streamsDescList.size();
    bool bUpdateVideoInRate = false;
    for(itr_streams = streamsDescList.begin(); itr_streams != streamsDescList.end() ;itr_streams++)
    {
        if (itr_streams->m_isAvcToSvcVsw)
        {
            TRACEINTO << "There is a VSW stream for the AVC. Do not change the video in rate.";
            bUpdateVideoInRate = false;
        }
    }

	DWORD actualAudioRate   = 10 * GetMediaBitRate(cmCapAudio, cmCapReceive);
	BOOL  bTipMode 			= GetIsTipMode(); //BRIDGE-5753

	//if (remoteIdent == CiscoCucm)
	if(bTipMode)
		actualAudioRate = 10*64;

	DWORD actualVideoRate   = GetMediaBitRate(cmCapVideo, cmCapReceive);
	DWORD actConfRate = 0;
	DWORD actConfRate2 = 0;

	//PTRACE2INT(eLevelInfoNormal,"CSipComMode::UpdateVideoOutRateIfNeeded - vidRateTx:",vidRateTx);
	//PTRACE2INT(eLevelInfoNormal,"CSipComMode::UpdateVideoOutRateIfNeeded - m_callRate:",m_callRate*10);
	//PTRACE2INT(eLevelInfoNormal,"CSipComMode::UpdateVideoOutRateIfNeeded - actualVideoRate:",actualVideoRate);
	//PTRACE2INT(eLevelInfoNormal,"CSipComMode::UpdateVideoOutRateIfNeeded - TipMode: ",(int)bTipMode);
	TRACEINTO << "vidRateTx:" << vidRateTx << ", m_callRate:" << (m_callRate*10) << ", actualVideoRate:" << actualVideoRate << ", TipMode:" << (int)bTipMode << ", LYNC2013_FEC_RED: isFecOrRedOn:" << (DWORD)isFecOrRedOn;

	DWORD totalRateTxOfRemote = vidRateTx + actualAudioRate;

	//take the minimum between the conf rate and the rate of the other side
	if(remoteIdent >=MicrosoftEP_R1 && remoteIdent<= MicrosoftEP_MAC_Lync)
	{
		PTRACE(eLevelInfoNormal, "CSipComMode::UpdateVideoOutRateIfNeeded-MS");
		if(vidRateTx != 0 && vidRateTx != (DWORD)(-1))
		{
			actConfRate2 = min(totalRateTxOfRemote, actualVideoRate+ actualAudioRate);
			actConfRate = min(actConfRate2, m_callRate*10 );
		}
		else
			actConfRate =  actualVideoRate ;
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CSipComMode::UpdateVideoOutRateIfNeeded-non MS");

		if (vidRateTx != 0 && vidRateTx != (DWORD)(-1))
			actConfRate = min(totalRateTxOfRemote, m_callRate*10);
		else
			actConfRate =  m_callRate*10;
	}

	int newVideoOutRate = actConfRate - actualAudioRate;
	int newVideoInRate  = newVideoOutRate;

	// // AN: for IBM vngfe-4195 and CTS
	if ((remoteIdent == IbmSametimeEp)||(remoteIdent == IbmSametimeEp_Legacy))
		newVideoInRate = actConfRate;

	// in TIP, RMX video-in rate is like the call rate
	//if (remoteIdent == CiscoCucm) //BRIDGE-5753
	if(bTipMode)
	{
		newVideoInRate  = m_callRate*10;
	}
	
	// if SVC and the new video in/out rate is lower than the lowest op. point bit rate,
	// then we don't have enough bit rate to send/receive video
	// set the new video rate to zero!
	const VideoOperationPoint *pLowestOpPoint = GetLowestOperationPoint(GetPartyId());
	CapEnum capInfo = (CapEnum)GetMediaType(cmCapVideo, cmCapReceive, kRolePeople);
	if (capInfo == eSvcCapCode && GetOperationPoints()->m_numberOfOperationPoints && pLowestOpPoint)
	{
		DWORD lowestOpPointBitRate = pLowestOpPoint->m_maxBitRate*10;
		TRACEINTO << "pLowestOpPoint->m_maxBitRate=" << lowestOpPointBitRate << " actualAudioRate=" << actualAudioRate
				<< " newVideoOutRate=" << newVideoOutRate;
		if (lowestOpPointBitRate > actualAudioRate + newVideoOutRate)
		{
			TRACEINTO << "Not enough bit rate to send video - changing video in/out rate to zero! pLowestOpPoint->m_maxBitRate=" << lowestOpPointBitRate << " actualAudioRate=" << actualAudioRate
					<< " newVideoOutRate=" << newVideoOutRate;
			newVideoOutRate = 0;
			newVideoInRate = 0;
		}
		if (lowestOpPointBitRate > actualAudioRate + newVideoInRate)
		{
			TRACEINTO << "Not enough bit rate to receive video - changing video in/out rate to zero! pLowestOpPoint->m_maxBitRate=" << lowestOpPointBitRate << " actualAudioRate=" << actualAudioRate
					<< " newVideoOutRate=" << newVideoInRate;
			newVideoOutRate = 0;
			newVideoInRate = 0;
		}
	}

	if (isFecOrRedOn == TRUE)
		newVideoOutRate = min((DWORD)newVideoOutRate,GetMediaBitRate(cmCapVideo, cmCapTransmit));

	CMedString cLog;
	cLog << "remote ident: " << RemoteIdentToString(remoteIdent) << ", changing video out rate to = " << newVideoOutRate<< " ,video in rate = " << newVideoInRate << ",actConfRate=" << actConfRate << ",actualAudioRate="<< actualAudioRate*10;
	PTRACE2(eLevelInfoNormal,"CSipComMode::UpdateVideoOutRateIfNeeded - ", cLog.GetString());

	SetVideoBitRate(newVideoOutRate, cmCapTransmit);
	if (bUpdateVideoInRate)
		SetVideoBitRate(newVideoInRate,  cmCapReceive);

	SetTotalVideoRate(newVideoOutRate);
}
