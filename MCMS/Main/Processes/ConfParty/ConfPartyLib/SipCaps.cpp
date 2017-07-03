//+========================================================================+
//                            SipCaps.cpp                                  |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SipCaps.cpp                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/11/05   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#include <string>
#include <stdlib.h>
#include <sstream>
#include "ConfPartyProcess.h"

#include "Macros.h"
#include "Trace.h"
#include "TraceStream.h"
#include "NStream.h"
#include "DataTypes.h"
//#include "IpCommonTypes.h"
#include "IPUtils.h"

#include "IpAddressDefinitions.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "CapClass.h"
#include "CapInfo.h"
#include "Capabilities.h"
#include "IpScm.h"
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "SipCaps.h"
#include "SipUtils.h"
#include "SipScm.h"

#include "SIPCommon.h"
#include "IpCommonUtilTrace.h"
#include "SIPInternals.h"
#include "ConfPartyGlobals.h"
#include "H263VideoMode.h"
#include "H264.h"
#include "SIPInternals.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "ServiceConfigList.h"
#include "H264Util.h"
#include "MsSvcMode.h"
#include "vp8VideoMode.h"
#include "UnifiedComMode.h"
#include "EnumsToStrings.h"

extern WORD IsValidIpV4Address(const char* pIP);
extern DWORD CalculateAudioRate(DWORD call_rate);
extern CIpServiceListManager* GetIpServiceListMngr();


#define isCapInCapRange(_lowRange, _highRange, _cap) ((_cap <= _highRange) && (_cap >= _lowRange))
#define isSirenLPRCap(_cap) isCapInCapRange(eSirenLPR_32kCapCode, eSirenLPRStereo_128kCapCode, _cap)

#define isG719Cap(_cap) (isCapInCapRange(eG719_32kCapCode, eG719_128kCapCode, _cap) ||\
						 isCapInCapRange(eG719Stereo_64kCapCode, eG719Stereo_128kCapCode, _cap))
#define isiLBCCap(_cap) isCapInCapRange(eiLBC_13kCapCode, eiLBC_15kCapCode, _cap)
#define isOpusCap(_cap) (isCapInCapRange(eOpus_CapCode, eOpusStereo_CapCode, _cap))

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CSipCaps::CSipCaps()
{
	m_numOfVideoCapSets = 0;
	m_numOfAudioCapSets = 0;
	m_numOfFeccCapSets  = 0;
	m_numOfContentCapSets = 0;
	m_numOfBfcpCapSets = 0;
	for(int i = 0; i < MAX_MEDIA_CAPSETS; i++)
	{
		m_audioCapList[i]	= NULL;
		m_videoCapList[i]	= NULL;
		m_feccCapList[i]	= NULL;
		m_contentCapList[i]	= NULL;
		m_bfcpCapList[i]	= NULL;
	}
	m_h263_4CifMpi = -1;
	m_videoQuality = eVideoQualitySharpness;
	m_bIsLpr = FALSE;
	//LYNC2013_FEC_RED:
	m_bIsFec = FALSE;
	m_bIsRed = FALSE;
	//SDES
	m_numOfAudioSdesCapSets = 0;
	m_numOfVideoSdesCapSets = 0;
	m_numOfFeccSdesCapSets 	= 0;
	m_numOfContentSdesCapSets 	= 0;

	for(int i = 0; i < MAX_SDES_CAPSETS; i++)
	{
		m_audioSdesCapList[i]	= NULL;
		m_videoSdesCapList[i]	= NULL;
		m_feccSdesCapList[i]	= NULL;
		m_contentSdesCapList[i]	= NULL;
	}

	// AVMCU SDES
	m_numOfMsftAVMCUSdesCaps = 0;
	for(int i = 0; i < MaxMsftSvcSdpVideoMlines; i++)
	{
		m_msftAVMCUSdesCaps[i] = NULL;
	}

	m_numOfIceAudioCaps = 0;
	m_numOfIceVideoCaps = 0;
	m_numOfIceDataCaps = 0;
	m_numOfIceGeneralCaps = 0;

	for(int j = 0; j < MAX_MEDIA_CAPSETS; j++)
	{
		m_IceAudioCapList[j]= NULL;
		m_IceVideoCapList[j]= NULL;
		m_IceDataCapList[j]	= NULL;
		m_IceGeneralCapList[j] = NULL;
	}

/*	for(int k = 0; k < NumOfCandidates; k++)
	{
		m_RtpCandidateArr[k]= NULL;
		m_RtcpCandidateArr[k]= NULL;
	}
*/
	memset(&m_AudioHostPartyAddr,0,sizeof(mcTransportAddress));
	memset(&m_VideoHostPartyAddr,0,sizeof(mcTransportAddress));
	memset(&m_DataHostPartyAddr,0,sizeof(mcTransportAddress));


	memset(&m_AudioHostPartyAddr,0,sizeof(mcTransportAddress));
	memset(&m_VideoHostPartyAddr,0,sizeof(mcTransportAddress));
	memset(&m_DataHostPartyAddr,0,sizeof(mcTransportAddress));

	//VNGFE-5845
	memset(&m_AudioSrflxPartyAddr,0,sizeof(mcTransportAddress));
	memset(&m_VideoSrflxPartyAddr,0,sizeof(mcTransportAddress));
	memset(&m_DataSrflxPartyAddr,0,sizeof(mcTransportAddress));

	m_TipAuxFPS = eTipAuxNone;

	//DTLS
	m_numOfAudioDtlsCapSets = 0;
	m_numOfVideoDtlsCapSets = 0;
	m_numOfFeccDtlsCapSets 	= 0;
	m_numOfContentDtlsCapSets 	= 0;

	for(int i = 0; i < MAX_SDES_CAPSETS; i++)
	{
		m_audioDtlsCapList[i]	= NULL;
		m_videoDtlsCapList[i]	= NULL;
		m_feccDtlsCapList[i]	= NULL;
		m_contentDtlsCapList[i]	= NULL;
	}

	m_sessionLevelRate = 0xFFFFFFFF;
	
	m_msftVideoRxBw = 0;

	m_msftSsrcAudio = 0;
	memset(m_msftSsrcVideo,0,sizeof(m_msftSsrcVideo));
	m_msftMsiAudio = 0;
	m_encryptionKeyToUse=eUseBothEncryptionKeys;
	m_bUseNonMkiOrderFirst = FALSE;
	memset(m_msftMsiVideo,0,sizeof(m_msftMsiVideo));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CSipCaps::CSipCaps(const CSipCaps &other):CPObject()
{
	m_numOfVideoCapSets = 0;
	m_numOfAudioCapSets = 0;
	m_numOfFeccCapSets  = 0;
	m_numOfContentCapSets = 0;
	m_numOfBfcpCapSets = 0;
	for(int i = 0; i < MAX_MEDIA_CAPSETS; i++)
	{
		m_audioCapList[i]	= NULL;
		m_videoCapList[i]	= NULL;
		m_feccCapList[i]	= NULL;
		m_contentCapList[i]	= NULL;
		m_bfcpCapList[i]	= NULL;
	}
	m_h263_4CifMpi = -1;
	m_videoQuality = eVideoQualitySharpness;
	m_bIsLpr = FALSE;
	//LYNC2013_FEC_RED:
	m_bIsFec = FALSE;
	m_bIsRed = FALSE;
	//SDES
	m_numOfAudioSdesCapSets = 0;
	m_numOfVideoSdesCapSets = 0;
	m_numOfFeccSdesCapSets 	= 0;
	m_numOfContentSdesCapSets 	= 0;
	for(int i = 0; i < MAX_SDES_CAPSETS; i++)
	{
		m_audioSdesCapList[i]	= NULL;
		m_videoSdesCapList[i]	= NULL;
		m_feccSdesCapList[i]	= NULL;
		m_contentSdesCapList[i]	= NULL;
	}
	// AVMCU SDES
	m_numOfMsftAVMCUSdesCaps = 0;
	for(int i = 0; i < MaxMsftSvcSdpVideoMlines; i++)
	{
			m_msftAVMCUSdesCaps[i] = NULL;
	}

	m_numOfIceAudioCaps = 0;
	m_numOfIceVideoCaps = 0;
	m_numOfIceDataCaps = 0;
	m_numOfIceGeneralCaps = 0;

	for(int j = 0; j < MAX_MEDIA_CAPSETS; j++)
	{
		m_IceAudioCapList[j]= NULL;
		m_IceVideoCapList[j]= NULL;
		m_IceDataCapList[j]	= NULL;
		m_IceGeneralCapList[j] = NULL;
	}
/*
	for(int k = 0; k < NumOfCandidates; k++)
	{
		m_RtpCandidateArr[k]= NULL;
		m_RtcpCandidateArr[k]= NULL;
	}
*/

	memset(&m_AudioHostPartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_AudioHostPartyAddr),&(other.m_AudioHostPartyAddr), sizeof(mcTransportAddress));

	memset(&m_VideoHostPartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_VideoHostPartyAddr),&(other.m_VideoHostPartyAddr), sizeof(mcTransportAddress));

	memset(&m_DataHostPartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_DataHostPartyAddr),&(other.m_DataHostPartyAddr), sizeof(mcTransportAddress));

	//VNGFE-5845
	memset(&m_AudioSrflxPartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_AudioSrflxPartyAddr),&(other.m_AudioSrflxPartyAddr), sizeof(mcTransportAddress));

	memset(&m_VideoHostPartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_VideoSrflxPartyAddr),&(other.m_VideoSrflxPartyAddr), sizeof(mcTransportAddress));

	memset(&m_DataSrflxPartyAddr,0,sizeof(mcTransportAddress));
	memcpy(&(m_DataSrflxPartyAddr),&(other.m_DataSrflxPartyAddr), sizeof(mcTransportAddress));

	m_TipAuxFPS = eTipAuxNone;

	//DTLS
	m_numOfAudioDtlsCapSets = 0;
	m_numOfVideoDtlsCapSets = 0;
	m_numOfFeccDtlsCapSets 	= 0;
	m_numOfContentDtlsCapSets 	= 0;

	for(int i = 0; i < MAX_SDES_CAPSETS; i++)
	{
		m_audioDtlsCapList[i]	= NULL;
		m_videoDtlsCapList[i]	= NULL;
		m_feccDtlsCapList[i]	= NULL;
		m_contentDtlsCapList[i]	= NULL;
	}

	m_sessionLevelRate = 0xFFFFFFFF;

	m_msftSsrcAudio = 0;
	memset(m_msftSsrcVideo,0,sizeof(m_msftSsrcVideo));
	m_msftVideoRxBw = 0;

	m_msftMsiAudio = 0;
	memset(m_msftMsiVideo,0,sizeof(m_msftMsiVideo));
	//m_encryptionKeyToUse   = eUseBothEncryptionKeys;
	m_encryptionKeyToUse   = other.m_encryptionKeyToUse;
	m_bUseNonMkiOrderFirst = other.m_bUseNonMkiOrderFirst;
	*this = other;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
CSipCaps::~CSipCaps()
{
	for(int i = 0; i < MAX_MEDIA_CAPSETS; i++)
	{
		PDELETEA(m_audioCapList[i]);
		PDELETEA(m_videoCapList[i]);
		PDELETEA(m_feccCapList[i]);
		PDELETEA(m_contentCapList[i]);
		PDELETEA(m_bfcpCapList[i]);
	}
    CleanIceCapSets();
	for(int i = 0; i < MAX_SDES_CAPSETS; i++)
	{
		PDELETEA(m_audioSdesCapList[i]);
		PDELETEA(m_videoSdesCapList[i]);
		PDELETEA(m_feccSdesCapList[i]);
		PDELETEA(m_contentSdesCapList[i]);
	}
	// avmcu sdes
	for(int i = 0; i < MaxMsftSvcSdpVideoMlines; i++)
	{
		if(m_msftAVMCUSdesCaps[i]) PDELETEA(m_msftAVMCUSdesCaps[i]);
	}

	for(int i = 0; i < MAX_SDES_CAPSETS; i++)
	{
		PDELETEA(m_audioDtlsCapList[i]);
		PDELETEA(m_videoDtlsCapList[i]);
		PDELETEA(m_feccDtlsCapList[i]);
		PDELETEA(m_contentDtlsCapList[i]);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::CleanIceCapSets()
{
    m_numOfIceAudioCaps = 0;
    m_numOfIceVideoCaps = 0;
    m_numOfIceDataCaps = 0;
    m_numOfIceGeneralCaps = 0;

    for(int i = 0; i < MAX_MEDIA_CAPSETS; i++)
    {
        PDELETEA(m_IceAudioCapList[i]);
        PDELETEA(m_IceVideoCapList[i]);
        PDELETEA(m_IceDataCapList[i]);
        PDELETEA(m_IceGeneralCapList[i]);
    }
/*
    for(int k = 0; k < NumOfCandidates; k++)
    {
    	PDELETEA(m_RtpCandidateArr[k]);
    	PDELETEA(m_RtcpCandidateArr[k]);

    }
*/

}



///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::Create(const CIpComMode* pScm, DWORD videoLineRate, const char* pPartyName, eVideoQuality vidQuality, DWORD serviceId,DWORD encryptionKeyToUse, ECopVideoFrameRate highestframerate, BYTE maxResolution, BOOL bUseNonMkiOrderFirst,BYTE Lync2013)
{
	PTRACE(eLevelInfoNormal, "CSipCaps::Create");
	CleanAll();
	m_videoQuality = vidQuality;
	m_bIsLpr =FALSE;

	//LYNC2013_FEC_RED:
	m_bIsFec = FALSE;
	m_bIsRed = FALSE;

	m_encryptionKeyToUse=encryptionKeyToUse;
	m_bUseNonMkiOrderFirst = bUseNonMkiOrderFirst; //BRIDGE-11708
   	m_sessionLevelRate = ((CIpComMode *)pScm)->GetCallRate() * 10;

	SetAudio(pScm, videoLineRate, NO, pPartyName);
	SetVideo(pScm, videoLineRate, serviceId, highestframerate, maxResolution);
	SetFecc(pScm, pPartyName, serviceId);
	SetContent(pScm, pPartyName);
	SetBfcp(pScm, pPartyName);
	SetTipAuxFPS(pScm->GetTipAuxFPS());
	PTRACE2INT(eLevelInfoNormal, "CSipCaps::Create : Tip Aux - ", GetTipAuxFPS());
	if(pScm->GetMediaType(cmCapVideo, cmCapTransmit) != eH261CapCode && pScm->GetMediaType(cmCapVideo, cmCapTransmit) != eMsSvcCapCode )
		m_h263_4CifMpi = CH263VideoMode::GetH263Cif4VideoCardMPI(videoLineRate, vidQuality);

	if(Lync2013)
	{
		TRACEINTO << "Lync 2013 add DSH to audio";
		AddDSHForAvMcu();

	}

}


////////////////////////////////////////////////////////
void CSipCaps::DumpToString(CSuperLargeString& str) const
{
	str << "\nCall Rate is : " << m_sessionLevelRate;
	str << "\nMsCall RX video Rate is : " << m_msftVideoRxBw;
	str << "\nCapabilities are:\n";
	DumpMediaToString(cmCapAudio,str);
	DumpMediaToString(cmCapVideo,str);
	DumpMediaToString(cmCapData,str);
	DumpMediaToString(cmCapVideo,str,kRolePresentation);
	DumpMediaToString(cmCapBfcp,str);
	str << "\nMSFT SSRC, audio: " << m_msftSsrcAudio;
	for (int i=0; i<MaxMsftSvcSdpVideoMlines; i++)
	{
		str << ", video" << i+1 << ": first=" << m_msftSsrcVideo[i][0] << "/last=" << m_msftSsrcVideo[i][1] ;
	}

	str << "\nMSFT Msi, audio: " << m_msftMsiAudio;
	for (int i=0; i<MaxMsftSvcSdpVideoMlines; i++)
	{
		str << ", video" << i+1 << ": " << m_msftMsiVideo[i];
	}

	str << "\nm_encryptionKeyToUse use mki: " <<(int)m_encryptionKeyToUse;
	str << "\nm_bUseNonMkiOrderFirst: " <<(int)m_bUseNonMkiOrderFirst;
	
	DumpAVMCUVideoSdesCapsToString(str);

	str << "\n";

}
void CSipCaps::DumpAVMCUVideoSdesCapsToString(CSuperLargeString& str) const
{
	str << "\nAVMCU video SDES caps: ";
	for (DWORD i=0; i < m_numOfMsftAVMCUSdesCaps; i++)
	{
		if(m_msftAVMCUSdesCaps[i])
		{
			str << "\n" << i << " : ";

			CSdesCap* pCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,m_msftAVMCUSdesCaps[i]->dataCap);
			if (pCap)
			{
				COstrStream msg;
				pCap->Dump(msg);
				str << msg.str().c_str();
				str << "-\t-\t-\t-\t-\t-\t-\t-\t-";
				PDELETE(pCap);
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::DumpIceCapsToString(CLargeString& str) const
{
	str << "\nCall Rate is : " << m_sessionLevelRate << "\n";
	str << "\nMsCall RX video Rate is : " << m_msftVideoRxBw << "\n";
	str << "Capabilities are:\n";
	DumpICEMediaToString( eGeneralSession,str);
	DumpICEMediaToString(eAudioSession,str);
	DumpICEMediaToString(eVideoSession,str);
	DumpICEMediaToString(eDataSession,str);

}

///////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::DumpICEMediaToString(ICESessionsTypes SessionTyp,CLargeString& str) const
{
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaIceCaps(SessionTyp,&numOfMediaCapSet,&pMediaCapList);
	for(int i=0; i<numOfMediaCapSet; i++)
	{
		CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode,pMediaCapList[i]->dataCap);
		if (pCap)
		{
			COstrStream msg;
			pCap->Dump(msg);
			str << msg.str().c_str();
			str << "Payload type            = " << pMediaCapList[i]->sipPayloadType<< "\n";
			str << "-\t-\t-\t-\t-\t-\t-\t-\t-";
		}
		POBJDELETE(pCap);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::DumpMediaToString(cmCapDataType eMediaType,CSuperLargeString& str, ERoleLabel eRole) const
{
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;

	GetMediaCaps(eMediaType,&numOfMediaCapSet,&pMediaCapList,eRole);

	for (int i=0; i<numOfMediaCapSet; i++)
	{
		CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode,pMediaCapList[i]->dataCap);
		
		if (pCap)
		{
			COstrStream msg;
			pCap->Dump(msg);
			str << msg.str().c_str();
			str << "Payload type            = " << pMediaCapList[i]->sipPayloadType<< "\n";
			str << "-\t-\t-\t-\t-\t-\t-\t-\t-";
		}

		POBJDELETE(pCap);
	}

	GetSdesMediaCaps(eMediaType,&numOfMediaCapSet,&pMediaCapList,eRole);

	for (int i=0; i<numOfMediaCapSet; i++)
	{
		CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode,pMediaCapList[i]->dataCap);
		
		if (pCap)
		{
			COstrStream msg;
			pCap->Dump(msg);
			str << msg.str().c_str();
			str << "Payload type            = " << pMediaCapList[i]->sipPayloadType<< "\n";
			str << "-\t-\t-\t-\t-\t-\t-\t-\t-";
		}
		
		POBJDELETE(pCap);
	}

}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetAudio(const CIpComMode* pScm, DWORD videoLineRate, BYTE isAudioOnly, const char* pPartyName)
{
	TRACEINTOFUNC << "pScm->GetMediaType: " << pScm->GetMediaType(cmCapAudio, cmCapReceive);
	PASSERTMSG_AND_RETURN((!CProcessBase::GetProcess() || !CProcessBase::GetProcess()->GetSysConfig()), "CSipCaps::SetAudio - NULLs found in CProcessBase::GetProcess()->GetSysConfig()");

	/* MSSlave Flora comment : */	
	if (pScm->IsMediaOff(cmCapAudio, cmCapReceiveAndTransmit))
	{
		PTRACE(eLevelError,"CSipCaps::SetAudio: Media is off");
		return;
	}
	
	// ===== SDES
	BYTE is_party_encrypted = ((CComModeH323*)pScm)->GetIsEncrypted();
	if(is_party_encrypted==Encryp_On)
	{
		AddSdesCaps(cmCapAudio);
	}

	// ===== DTLS
	BYTE is_party_dtls_encrypted = ((CComModeH323*)pScm)->GetIsDtlsEncrypted();
	if(is_party_dtls_encrypted==Encryp_On)
	{
		AddSingleDtlsCap(cmCapAudio, TRUE);
	}

	// ===== LYNC2013_FEC_RED:
	if ( pScm->GetIsRed() )
	{
		TRACEINTO << "LYNC2013_FEC_RED: Adding RED cap";
		AddRedCap();
	}

	// ===== AddSingleVideoCap
    if ( CSacAudioCap::IsSacAudio((CapEnum)((CComModeH323*)pScm)->GetMediaType(cmCapAudio, cmCapReceive)) )
	{
        AddSingleAudioCap((CapEnum)((CComModeH323*)pScm)->GetMediaType(cmCapAudio, cmCapReceive));
		AddSingleAudioCap(eRfc2833DtmfCapCode);
		return;
	}

    
    if(pScm->GetMediaType(cmCapAudio, cmCapReceive) == eG722Stereo_128kCapCode)
    {
    	TRACEINTO << "for lync adding this cap to local caps!!!  ";
    	AddSingleAudioCap((CapEnum)((CComModeH323*)pScm)->GetMediaType(cmCapAudio, cmCapReceive));
    }

	BOOL isReduceCapsForRedcom = FALSE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("REDUCE_CAPS_FOR_REDCOM_SIP", isReduceCapsForRedcom);

	if(isReduceCapsForRedcom)
	{
		AddSingleAudioCap((CapEnum)pScm->GetMediaType(cmCapAudio, cmCapReceive));
		AddSingleAudioCap(eG711Ulaw64kCapCode);
		AddSingleAudioCap(eG711Alaw64kCapCode);
		return;

	}

	// ===== Add another codec(s)

	BOOL bIsForceG711A = TRUE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_FORCE_G711A, bIsForceG711A);

	//BRIDGE-12398
	//if( (FALSE == bIsForceG711A) && (strstr(pPartyName, "##FORCE_MEDIA_A")==NULL) )
	BOOL bIsForceSirenStero = isAudioOnly && strstr(pPartyName, "##FORCE_MEDIA_ASIREN")!=NULL && strstr(pPartyName, "STEREO")!=NULL; //no support for siren stereo on audio only calls
	BOOL bIsForceMedia 		= strstr(pPartyName, "##FORCE_MEDIA_A")!=NULL;

	if((!bIsForceG711A && !bIsForceMedia) || bIsForceSirenStero)
	{
		//============================================================
		// Siren7 is dependent on configuration only and added first
		//============================================================
		if (IsFeatureSupportedBySystem(eFeatureSiren7))
		{
			BOOL siren7Allowed = FALSE;
			CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ALLOW_SIREN7_CODEC, siren7Allowed);
			if (siren7Allowed)
			{
				AddSingleAudioCap(eSiren7_16kCapCode);
			}
		}


		// set the preferred audio algorithm according to the SCM
		DWORD confBitRate = pScm->GetCallRate() * 1000;
		DWORD calculatedConfRate = videoLineRate * 100;
		DWORD confAudioBitRate = ::CalculateAudioRate(confBitRate);
		uint32_t isSirenLPRSet = (is_party_encrypted == Encryp_On)?1:0; /* in case party encrypted we dont want sirenLPR Caps */
		bool isG719Set = false;
		BOOL isEncSirenLPRSet = NO;
		//BOOL isRemoveSirenLPR = NO;

		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("ENABLE_SIRENLPR_SIP_ENCRYPTION", isEncSirenLPRSet);
		/**
		 * We have 3 options when validating this flags:
		 * 1. Call is without Encryption, in this case we want to set sirenLPR, and "isSetSirenLPR" should be 0
		 * 2. Call is with Encryption, ENABLE_SIRENLPR_SIP_ENCRYPTION = NO, in this case we want to set "isSetSirenLPR" to > 0 so we wont set sirenLPR in capabilities.
		 * 3. Call is with Encryption, ENABLE_SIRENLPR_SIP_ENCRYPTION = YES,  in this case we want to set "isSetSirenLPR" to 0, in this case we'll have sirenLPR in Capabilities.
		 * Once openssl with SirenLPR is solved by algorithems team, this code should be removed with ENABLE_SIRENLPR_SIP_ENCRYPTION flag.
		 */
		if ((is_party_encrypted) && (isEncSirenLPRSet) ) {
			isSirenLPRSet = 0;
		}

		//Patch for Siren LPR - In Audio only calls we will remove SirenLPR from our caps so the HDX will open incoming audio channels.
		//if(!pScm->IsMediaOn(cmCapVideo,cmCapTransmit,kRolePeople))
		//{
		//	isSirenLPRSet = 1;
		//}

		/*
		 if X = confRate - audio rate; if X = video rate, take confRate.
		else
		{
		if X > video rate  or if X < video rate, rounded video rate up to the next multiple of 64K and take audio rate according to this rate
		}
		*/
		if((confBitRate - confAudioBitRate) != calculatedConfRate)
		{//because of the new rates of 96K and above the round up result is diferent then the conffference video rate
			// to make the calculated rate a multiple of 64k
			if((calculatedConfRate + rate8K) != confBitRate) // in case of rate96K we ignore this line (dial in problem).
			{
				calculatedConfRate = (calculatedConfRate % rate64K) ? (calculatedConfRate/rate64K + 1) * rate64K : (calculatedConfRate/rate64K) * rate64K;
				if(calculatedConfRate)
					confBitRate = calculatedConfRate;
			}
		}

		DecideOnConfBitRateForAudioSelection(confBitRate);

		BOOL isG711AlawFirst = NO;
		std::string key = "PREFERRED_G711_MODE";
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, isG711AlawFirst);

		bool isiLBCEnable = CProcessBase::GetProcess()->GetProductType() == eProductTypeSoftMCUMfw;   //BRIDGE-7453
		bool isiLBCFirst = (isiLBCEnable && confBitRate <= rate384K);
		bool isiLBCSet = false;
		if (!isiLBCEnable)
			isiLBCSet = true;
		bool isOpusFirst = confAudioBitRate < rate24K;
		bool isOpusSet = false;

		// Set the selected mode first
		switch(confBitRate)
		{
		case rate64K:
		case rate96K:
			if (isOpusFirst && !isOpusSet)
			{
				AddSingleAudioCap(CalculateAudioOpusCapEnum(confAudioBitRate), confAudioBitRate);
				isOpusSet = true;
			}
			if (isiLBCFirst)
			{
				AddSingleAudioCap(eiLBC_15kCapCode);
				isiLBCSet = true;
			}
			AddSingleAudioCap(eG729AnnexACapCode);
			AddSingleAudioCap(eG7231CapCode);
			AddSingleAudioCap(eG7221_16kCapCode);
			if (!isiLBCSet)
			{
				AddSingleAudioCap(eiLBC_15kCapCode);
				isiLBCSet = true;
			}
			AddSingleAudioCap(eG7221C_24kCapCode);
			AddSingleAudioCap(eG722_64kCapCode);
			if (!isOpusSet)
			{
				AddSingleAudioCap(CalculateAudioOpusCapEnum(confAudioBitRate), confAudioBitRate);
				isOpusSet = true;
			}
			if( !isG711AlawFirst )
			{
			   AddSingleAudioCap(eG711Ulaw64kCapCode);
			   AddSingleAudioCap(eG711Alaw64kCapCode);
			}
			else
			{
			   AddSingleAudioCap(eG711Alaw64kCapCode);
			   AddSingleAudioCap(eG711Ulaw64kCapCode);
			}
		    AddSingleAudioCap(eSiren14_24kCapCode);
			break;

		case rate128K:
			if (isOpusFirst && !isOpusSet)
			{
				AddSingleAudioCap(CalculateAudioOpusCapEnum(confAudioBitRate), confAudioBitRate);
				isOpusSet = true;
			}
			if (isiLBCFirst)
			{
				AddSingleAudioCap(eiLBC_15kCapCode);
				isiLBCSet = true;
			}
		    if (!isSirenLPRSet) {
		    	AddSingleAudioCap(eSirenLPR_32kCapCode);
		    	isSirenLPRSet = eSirenLPR_32kCapCode;
		    }
			AddSingleAudioCap(eG719_32kCapCode);
			AddSingleAudioCap(eSiren22_32kCapCode);
			AddSingleAudioCap(eG7221C_32kCapCode);
			AddSingleAudioCap(eG7221C_24kCapCode);
			AddSingleAudioCap(eG7221_32kCapCode);
			AddSingleAudioCap(eG7221_24kCapCode);
			AddSingleAudioCap(eG7221_16kCapCode);
			if (!isiLBCSet)
			{
				AddSingleAudioCap(eiLBC_15kCapCode);
				isiLBCSet = true;
			}
			AddSingleAudioCap(eG728CapCode);
			AddSingleAudioCap(eG729AnnexACapCode);
			AddSingleAudioCap(eG722_64kCapCode);
			if (!isOpusSet)
			{
				AddSingleAudioCap(CalculateAudioOpusCapEnum(confAudioBitRate), confAudioBitRate);
				isOpusSet = true;
			}
			if( !isG711AlawFirst )
			{
			   AddSingleAudioCap(eG711Ulaw64kCapCode);
			   AddSingleAudioCap(eG711Alaw64kCapCode);
			}
			else
			{
			   AddSingleAudioCap(eG711Alaw64kCapCode);
			   AddSingleAudioCap(eG711Ulaw64kCapCode);
			}
		    AddSingleAudioCap(eSiren14_32kCapCode);
//          AddSingleAudioCap(eG7231CapCode);
			break;

		case rate1024K:

			// TIP
			if (((CComModeH323*)pScm)->GetIsTipMode())
				AddSingleAudioCap(eAAC_LDCapCode);

			if (isOpusFirst && !isOpusSet)
			{
				AddSingleAudioCap(CalculateAudioOpusCapEnum(confAudioBitRate), confAudioBitRate);
				isOpusSet = true;
			}
			if (!isSirenLPRSet) {
				AddSingleAudioCap(eSirenLPRStereo_128kCapCode);
				isSirenLPRSet = eSirenLPRStereo_128kCapCode;
			}
			if (!isG719Set)
			{
				AddSingleAudioCap(eG719Stereo_128kCapCode);
				isG719Set = true;
			}
		    AddSingleAudioCap(eSiren22Stereo_128kCapCode);
		case rate512K:
			if (isOpusFirst && !isOpusSet)
			{
				AddSingleAudioCap(CalculateAudioOpusCapEnum(confAudioBitRate), confAudioBitRate);
				isOpusSet = true;
			}
			if (!isSirenLPRSet) {
				AddSingleAudioCap(eSirenLPRStereo_96kCapCode);
				isSirenLPRSet = eSirenLPRStereo_96kCapCode;
			}
			if (!isG719Set)
			{
				AddSingleAudioCap(eG719Stereo_96kCapCode);
				isG719Set = true;
			}
		    AddSingleAudioCap(eSiren22Stereo_96kCapCode);
		    AddSingleAudioCap(eSiren14Stereo_96kCapCode);
		case rate384K:
			if (isOpusFirst && !isOpusSet)
			{
				AddSingleAudioCap(CalculateAudioOpusCapEnum(confAudioBitRate), confAudioBitRate);
				isOpusSet = true;
			}
			if (isiLBCFirst)
			{
				AddSingleAudioCap(eiLBC_15kCapCode);
				isiLBCSet = true;
			}
			if (!isSirenLPRSet) {
				AddSingleAudioCap(eSirenLPRStereo_64kCapCode);
				isSirenLPRSet = eSirenLPRStereo_64kCapCode;
			}
			if (!isG719Set)
			{
				AddSingleAudioCap(eG719Stereo_64kCapCode);
				isG719Set = true;
			}
			AddSingleAudioCap(eSiren22Stereo_64kCapCode);
		    AddSingleAudioCap(eSiren14Stereo_64kCapCode);
			AddSingleAudioCap(eSiren22_64kCapCode);
		case rate256K:
			if (isOpusFirst && !isOpusSet)
			{
				AddSingleAudioCap(CalculateAudioOpusCapEnum(confAudioBitRate), confAudioBitRate);
				isOpusSet = true;
			}
			if (isiLBCFirst && !isiLBCSet)
			{
				AddSingleAudioCap(eiLBC_15kCapCode);
				isiLBCSet = true;
			}
			if (!isSirenLPRSet) {
				AddSingleAudioCap(eSirenLPR_48kCapCode);
				isSirenLPRSet = eSirenLPR_48kCapCode;
			}
			AddSingleAudioCap(eSiren14Stereo_48kCapCode);
			if (!isG719Set)
			{
				AddSingleAudioCap(eG719_48kCapCode);
				isG719Set = true;
			}
			AddSingleAudioCap(eSiren22_48kCapCode);
			AddSingleAudioCap(eG7221C_48kCapCode);
			AddSingleAudioCap(eSiren14_48kCapCode);
		case rate192K:
			if (isOpusFirst && !isOpusSet)
			{
				AddSingleAudioCap(CalculateAudioOpusCapEnum(confAudioBitRate), confAudioBitRate);
				isOpusSet = true;
			}
			if (isiLBCFirst && !isiLBCSet)
			{
				AddSingleAudioCap(eiLBC_15kCapCode);
				isiLBCSet = true;
			}
			if (!isSirenLPRSet) {
				AddSingleAudioCap(eSirenLPR_32kCapCode);
				isSirenLPRSet = eSirenLPR_32kCapCode;
			}
			if (!isG719Set)
			{
				AddSingleAudioCap(eG719_32kCapCode);
				isG719Set = true;
			}
			AddSingleAudioCap(eSiren22_32kCapCode);
			AddSingleAudioCap(eG7221C_32kCapCode);
			AddSingleAudioCap(eG7221C_24kCapCode);
			AddSingleAudioCap(eG7221_32kCapCode);
			AddSingleAudioCap(eG7221_24kCapCode);
			AddSingleAudioCap(eG7221_16kCapCode);
			if (!isiLBCSet) {
				AddSingleAudioCap(eiLBC_15kCapCode);
				isiLBCSet = true;
			}
			AddSingleAudioCap(eG722_64kCapCode);
			AddSingleAudioCap(eSiren14_32kCapCode);
		default:
			if (!isOpusSet)
			{
				DWORD opusBitRate = (confBitRate < rate1024K) ? confAudioBitRate : rate128K;
				AddSingleAudioCap(CalculateAudioOpusCapEnum(opusBitRate), opusBitRate);
				isOpusSet = true;
			}
			if( !isG711AlawFirst )
			{
			   AddSingleAudioCap(eG711Ulaw64kCapCode);
			   AddSingleAudioCap(eG711Alaw64kCapCode);
			}
			else
			{
			   AddSingleAudioCap(eG711Alaw64kCapCode);
			   AddSingleAudioCap(eG711Ulaw64kCapCode);
			}
			AddSingleAudioCap(eG729AnnexACapCode);
			AddSingleAudioCap(eG7231CapCode);
			break;
		}

		AddSingleAudioCap(eRfc2833DtmfCapCode);
	}
	else
	{
		SetAudioAccordingToPartyName(pPartyName);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetAudioAccordingToPartyName(const char* pPartyName)
{
	BOOL bIsForceG711A = TRUE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_FORCE_G711A, bIsForceG711A);

	if( (TRUE == bIsForceG711A) || (strstr(pPartyName, "##FORCE_MEDIA_AG711_A")!=NULL) )
		AddSingleAudioCap(eG711Alaw64kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG711_U")!=NULL)
		AddSingleAudioCap(eG711Ulaw64kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1_16K")!=NULL)
		AddSingleAudioCap(eG7221_16kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1_24K")!=NULL)
		AddSingleAudioCap(eG7221_24kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1_32K")!=NULL)
		AddSingleAudioCap(eG7221_32kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7231")!=NULL)
		AddSingleAudioCap(eG7231CapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG729")!=NULL)
		AddSingleAudioCap(eG729AnnexACapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN7_16K")!=NULL)
		AddSingleAudioCap(eSiren7_16kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14_48K")!=NULL)
		AddSingleAudioCap(eSiren14_48kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14_32K")!=NULL)
		AddSingleAudioCap(eSiren14_32kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14_24K")!=NULL)
		AddSingleAudioCap(eSiren14_24kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C_24K")!=NULL)
		AddSingleAudioCap(eG7221C_24kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C_32K")!=NULL)
		AddSingleAudioCap(eG7221C_32kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C_48K")!=NULL)
		AddSingleAudioCap(eG7221C_48kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722_1")!=NULL)
	{
		AddSingleAudioCap(eG7221_16kCapCode);
		AddSingleAudioCap(eG7221_24kCapCode);
		AddSingleAudioCap(eG7221_32kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG7221C")!=NULL)
	{
		AddSingleAudioCap(eG7221C_24kCapCode);
		AddSingleAudioCap(eG7221C_32kCapCode);
		AddSingleAudioCap(eG7221C_48kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722STEREO")!=NULL)
	{
		BOOL isSLyncEnableG722Stereo = FALSE;
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		pSysConfig->GetBOOLDataByKey("LYNC2013_ENABLE_G722Stereo128k", isSLyncEnableG722Stereo);
		TRACEINTO << "LYNC_G722Stereo128k - isSLyncEnableG722Stereo:" << (DWORD)isSLyncEnableG722Stereo;
		if (isSLyncEnableG722Stereo == TRUE)
			AddSingleAudioCap(eG722Stereo_128kCapCode);

		AddSingleAudioCap(eG711Alaw64kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG722")!=NULL)// must be last because it can be confuse with the ##FORCE_MEDIA_AG7221C
		AddSingleAudioCap(eG722_64kCapCode);

	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_48K")!=NULL)
		AddSingleAudioCap(eSiren14Stereo_48kCapCode);
//	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_56K")!=NULL)
//		AddSingleAudioCap(eSiren14Stereo_56kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_64K")!=NULL)
		AddSingleAudioCap(eSiren14Stereo_64kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO_96K")!=NULL)
		AddSingleAudioCap(eSiren14Stereo_96kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14STEREO")!=NULL)
	{
		AddSingleAudioCap(eSiren14Stereo_48kCapCode);
//		AddSingleAudioCap(eSiren14Stereo_56kCapCode);
		AddSingleAudioCap(eSiren14Stereo_64kCapCode);
		AddSingleAudioCap(eSiren14Stereo_96kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN7")!=NULL)
	{
		AddSingleAudioCap(eSiren7_16kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN14")!=NULL)
	{
		AddSingleAudioCap(eSiren14_24kCapCode);
		AddSingleAudioCap(eSiren14_32kCapCode);
		AddSingleAudioCap(eSiren14_48kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22STEREO_64K")!=NULL)
		AddSingleAudioCap(eSiren22Stereo_64kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22STEREO_96K")!=NULL)
		AddSingleAudioCap(eSiren22Stereo_96kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22STEREO_128K")!=NULL)
		AddSingleAudioCap(eSiren22Stereo_128kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22STEREO")!=NULL)
	{
		AddSingleAudioCap(eSiren22Stereo_128kCapCode);
		AddSingleAudioCap(eSiren22Stereo_96kCapCode);
		AddSingleAudioCap(eSiren22Stereo_64kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22_64K")!=NULL)
		AddSingleAudioCap(eSiren22_64kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22_48K")!=NULL)
		AddSingleAudioCap(eSiren22_48kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22_32K")!=NULL)
		AddSingleAudioCap(eSiren22_32kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIREN22")!=NULL)
	{
		AddSingleAudioCap(eSiren22_64kCapCode);
		AddSingleAudioCap(eSiren22_48kCapCode);
		AddSingleAudioCap(eSiren22_32kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_64K")!=NULL)
		AddSingleAudioCap(eSirenLPR_64kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_48K")!=NULL)
		AddSingleAudioCap(eSirenLPR_48kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_32K")!=NULL)
		AddSingleAudioCap(eSirenLPR_32kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR")!=NULL)
	{
		AddSingleAudioCap(eSirenLPR_64kCapCode);
		AddSingleAudioCap(eSirenLPR_48kCapCode);
		AddSingleAudioCap(eSirenLPR_32kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_128K")!=NULL)
		AddSingleAudioCap(eSirenLPRStereo_128kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_96K")!=NULL)
		AddSingleAudioCap(eSirenLPRStereo_96kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_64K")!=NULL)
		AddSingleAudioCap(eSirenLPRStereo_64kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO")!=NULL)
	{
		AddSingleAudioCap(eSirenLPRStereo_128kCapCode);
		AddSingleAudioCap(eSirenLPRStereo_96kCapCode);
		AddSingleAudioCap(eSirenLPRStereo_64kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_64K")!=NULL)
		AddSingleAudioCap(eG719_64kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_48K")!=NULL)
		AddSingleAudioCap(eG719_48kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719_32K")!=NULL)
		AddSingleAudioCap(eG719_32kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719")!=NULL)
	{
		AddSingleAudioCap(eG719_64kCapCode);
		AddSingleAudioCap(eG719_48kCapCode);
		AddSingleAudioCap(eG719_32kCapCode);
	}
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719STEREO_128K")!=NULL)
		AddSingleAudioCap(eG719Stereo_128kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719STEREO_96K")!=NULL)
		AddSingleAudioCap(eG719Stereo_96kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719STEREO_64K")!=NULL)
		AddSingleAudioCap(eG719Stereo_64kCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG719STEREO")!=NULL)
	{
		AddSingleAudioCap(eG719Stereo_128kCapCode);
		AddSingleAudioCap(eG719Stereo_96kCapCode);
		AddSingleAudioCap(eG719Stereo_64kCapCode);
	}
        else if(strstr(pPartyName, "##FORCE_MEDIA_AAC_LD")!=NULL)// TIP
		AddSingleAudioCap(eAAC_LDCapCode);
	else if(strstr(pPartyName, "##FORCE_MEDIA_AG728")!=NULL)
	{
			PTRACE(eLevelInfoNormal,"CSipCaps::SetAudioAccordingToPartyName -force G728");
			AddSingleAudioCap(eG728CapCode);
	}

	//Amihay: MRM CODE
    else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_SCALABLE_64K")!=NULL)
    {
        AddSingleAudioCap(eSirenLPR_Scalable_64kCapCode);
    }
    else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_SCALABLE_48K")!=NULL)
    {
        AddSingleAudioCap(eSirenLPR_Scalable_48kCapCode);
    }
    else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_SCALABLE_32K")!=NULL)
    {
        AddSingleAudioCap(eSirenLPR_Scalable_32kCapCode);
    }
    else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPR_SCALABLE")!=NULL)
    {
        AddSingleAudioCap(eSirenLPR_Scalable_64kCapCode);
        AddSingleAudioCap(eSirenLPR_Scalable_48kCapCode);
        AddSingleAudioCap(eSirenLPR_Scalable_32kCapCode);
    }
    else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_SCALABLE_128K")!=NULL)
        AddSingleAudioCap(eSirenLPRStereo_Scalable_128kCapCode);
    else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_SCALABLE_96K")!=NULL)
        AddSingleAudioCap(eSirenLPRStereo_Scalable_96kCapCode);
    else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_SCALABLE_64K")!=NULL)
        AddSingleAudioCap(eSirenLPRStereo_Scalable_64kCapCode);
    else if(strstr(pPartyName, "##FORCE_MEDIA_ASIRENLPRSTEREO_SCALABLE")!=NULL)
    {
        AddSingleAudioCap(eSirenLPRStereo_Scalable_128kCapCode);
        AddSingleAudioCap(eSirenLPRStereo_Scalable_96kCapCode);
        AddSingleAudioCap(eSirenLPRStereo_Scalable_64kCapCode);
    }
    else if(strstr(pPartyName, "##FORCE_MEDIA_AILBC")!=NULL)
		AddSingleAudioCap(eiLBC_15kCapCode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddSingleAudioCap(CCapSetInfo capInfo, APIS32 bitrate)
{
	TRACEINTOFUNC << (CapEnum)capInfo;

	if (capInfo.IsType(cmCapAudio))
	{
		BYTE bSupported = capInfo.IsSupporedCap();
		if (bSupported)
		{
			CBaseAudioCap* pAudioCap = (CBaseAudioCap*)CBaseCap::AllocNewCap((CapEnum)capInfo,NULL);
			if (pAudioCap)
			{
				EResult eResOfSet	= kSuccess;
				int		maxFpp		= capInfo.GetMaxFramePerPacket();
				int		minFpp		= 0;

				eResOfSet &= pAudioCap->SetStruct(cmCapReceiveAndTransmit, maxFpp, minFpp);
				if(eResOfSet)
				{
					pAudioCap->SetBitRate(bitrate);
					capBuffer* pCapBuffer = pAudioCap->GetAsCapBuffer();
					if (pCapBuffer)
					{
					    pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
					    AddCapSet(cmCapAudio,pCapBuffer);
					    PDELETEA(pCapBuffer);
					}
					else
					    PTRACE(eLevelError,"CSipCaps::AddSingleAudioCap: Create cap buffer has failed");
				}
				else
					PTRACE(eLevelError,"CSipCaps::AddSingleAudioCap: Set struct has failed");
				pAudioCap->FreeStruct();
			}
			POBJDELETE(pAudioCap);
		}
	}
	else
	{
		PTRACE2INT(eLevelError, "CSipCaps::AddSingleAudioCap: Not audio data type [%d]", (int)capInfo.GetSipCapType());
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetVideo(const CIpComMode* pScm, DWORD videoLineRate,DWORD serviceId, ECopVideoFrameRate highestframerate, BYTE maxResolution)
{
	// if the SCM is H264 we add to the capabilities, then we add the H263 and H261 according to system.cfg flags
	// if the SCM is H263 we add only the SCM algorithms since H263 conference doesn't declared on H264 alg.

	if (Video_Off == pScm->IsMediaOn(cmCapVideo))
	{
		TRACEINTO << "Video media is Off";
		CleanMedia(cmCapVideo);
	}
	else
	{
		// ===== SDES
		if (Encryp_On == ((CComModeH323*)pScm)->GetIsEncrypted())
		{
			AddSdesCaps(cmCapVideo);
		}

		// ===== DTLS
		if (Encryp_On == ((CComModeH323*)pScm)->GetIsDtlsEncrypted())
		{
			AddSingleDtlsCap(cmCapVideo , TRUE);
		}

		// ===== AddSingleVideoCap
		CapEnum	eAlg = (CapEnum) pScm->GetMediaType(cmCapVideo, cmCapReceive);
		EConfType confType = pScm->GetConfType();

		if ( (pScm->GetConfMediaType()==eMixAvcSvcVsw) && (eSvcCapCode != eAlg) ) // to add: && !mrc/svc eyaln9794
		{
			const VideoOperationPoint* pOperationPoint = pScm->GetLowestOperationPoint(0);
			AddSingleVideoCapForVideoRelayAvc(eAlg, pScm, videoLineRate, highestframerate, pOperationPoint);
		}
		else
		{
			AddSingleVideoCap(eAlg, pScm, videoLineRate, highestframerate, maxResolution);
		}

		// ===== Add another codec(s)

		CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(serviceId);

		// if the algorithm is H261 we don't add another video codec.
		if (eAlg == eH261CapCode)
		{
			// ===== LPR
			if ((pScm->GetIsLpr()) &&
				 (!((CComModeH323*)pScm)->GetIsTipMode() || ( (pService != NULL) && (pService->GetSipServerType() != eSipServer_CiscoCucm)) ) )
			{
				TRACEINTO << "Adding lpr cap";
				AddLprCap(kRolePeople, pScm->GetConfMediaType(), false);
			}

			TRACEINTO << "algorithm is H261, don't add another video codec";
			return;
		}

		if ((kVSW_Fixed != confType) && (pScm->GetConfMediaType()!=eMixAvcSvcVsw) && (eAlg != eSvcCapCode))
		{
			// in MS env we need to declare also RTV
		    BOOL bMsEnviroment = FALSE;
			
			if (pService != NULL && pService->GetConfigurationOfSipServers())
			{
				if (pService->GetSipServerType() == eSipServer_ms)
					bMsEnviroment = TRUE;
			}

			int numOfAlg = 0;
			CapEnum arrVideoAlg[4];
			bool bRtvAdded = false;
			
			if (eAlg == eMsSvcCapCode)
			{
				arrVideoAlg[0] = eRtvCapCode;
				arrVideoAlg[1] = eUnknownAlgorithemCapCode;
				arrVideoAlg[2] = eUnknownAlgorithemCapCode;
				numOfAlg = 1;
				bRtvAdded = true;

				//LYNC2013_FEC_RED:
				if ( pScm->GetIsFec() )
				{
					TRACEINTO << "LYNC2013_FEC_RED: Adding FEC cap";
					AddFecCap();
				}

			}
			else if (bMsEnviroment && (kVSW_Fixed != confType) 
				&& (IsFeatureSupportedBySystem(eFeatureRtv)))
			{
				arrVideoAlg[0] = eRtvCapCode;
				arrVideoAlg[1] = eH263CapCode;
				arrVideoAlg[2] = eH261CapCode;

				bRtvAdded = true;
				numOfAlg = 3;
			}
			else if(eVP8CapCode == eAlg) //N.A. DEBUG VP8
			{
				arrVideoAlg[0] = eVP8CapCode;
				arrVideoAlg[1] = eUnknownAlgorithemCapCode;
				arrVideoAlg[2] = eUnknownAlgorithemCapCode;
				numOfAlg = 1;
				bRtvAdded = false;
			}
			else
			{
				arrVideoAlg[0] = eVP8CapCode; //N.A. DEBUG VP8
				arrVideoAlg[1] = eH263CapCode;
				arrVideoAlg[2] = eH261CapCode;
				arrVideoAlg[3] = eUnknownAlgorithemCapCode;

				numOfAlg = 3; //N.A. DEBUG VP8
			}

			BOOL isReduceCapsForRedcom = FALSE;
			CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			pSysConfig->GetBOOLDataByKey("REDUCE_CAPS_FOR_REDCOM_SIP", isReduceCapsForRedcom);

		    if(isReduceCapsForRedcom)
		    	numOfAlg =0;

			TRACEINTO << "confType: "				<< confType
					  << ", eAlg: "					<< eAlg
					  << ", numOfAlg: "				<< numOfAlg
					  << ", arrVideoAlg[0]: "		<< arrVideoAlg[0]
			          << ", bMsEnviroment: "		<< (bMsEnviroment ? "yes" : "no")
			          << ", bRtvAdded: "			<< (bRtvAdded ? "yes" : "no")
			          << ", isReduceCapsForRedcom: "<< (isReduceCapsForRedcom ? "yes" : "no");

			for (int i = 0; i < numOfAlg; i++)
			{
				if ((arrVideoAlg[i] != eAlg) && (arrVideoAlg[i]!= eUnknownAlgorithemCapCode))
				{
					TRACEINTO << "arrVideoAlg[i]: " << arrVideoAlg[i];
					AddSingleVideoCap(arrVideoAlg[i], pScm, videoLineRate, eCopVideoFrameRate_None, maxResolution);
				}
			}
		}
		else // (kVSW_Fixed == confType) || (kVswRelayAvc == confType) || (eAlg == eSvcCapCode)
		{
			TRACEINTO << "No additional codecs required (confType: " << confType << ", eAlg: " << eAlg << ")";
		}

		// ===== LPR
		if ((pScm->GetIsLpr()) &&
			 (eAlg != eSvcCapCode) &&
			 (!((CComModeH323*)pScm)->GetIsTipMode() || ( (pService != NULL) && (pService->GetSipServerType() != eSipServer_CiscoCucm))))
		{
			TRACEINTO << "Adding lpr cap";
			bool isMrc = ( (eSvcCapCode == eAlg) ? true : false);
			AddLprCap(kRolePeople, pScm->GetConfMediaType(), isMrc);
		}
	} // end if (!Video_Off)
}

/////////////////////////////////////////////////////////////////////////////////////////
/*
 * ==============
 * old version!!!
 * ==============
void CSipCaps::SetVideo(const CIpComMode* pScm, DWORD videoLineRate,DWORD serviceId, ECopVideoFrameRate highestframerate, BYTE maxResolution)
{
	BYTE is_party_encrypted 	 = ((CComModeH323*)pScm)->GetIsEncrypted();
	BYTE is_party_dtls_encrypted = ((CComModeH323*)pScm)->GetIsDtlsEncrypted();
	// if the SCM is H264 we add to the capabilities, then we add the H263 and H261 according to system.cfg flags
	// if the SCM is H263 we add only the SCM algorithms since H263 conference doesn't declared on H264 alg.

	//In MS env we need to declare also RTV
//	BOOL bMsEnviroment = GetSystemCfgFlagInt<BOOL>("MS_ENVIRONMENT");

    BOOL bMsEnviroment = FALSE;
	CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(serviceId);
	if( pService != NULL && pService->GetConfigurationOfSipServers() )
	{
		if(pService->GetSipServerType() == eSipServer_ms)
			bMsEnviroment = TRUE;
	}


	PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetVideo, bMsEnviroment: ",bMsEnviroment);
	CapEnum arrVideoAlg[3];
	int numOfAlg;

	if(pScm->GetConfType() == kVswRelayAvc && pScm->GetMediaType(cmCapVideo, cmCapReceive)!=eSvcCapCode) // to add: && !mrc/svc eyaln9794
	{
//		if (pScm->GetMediaType(cmCapAudio, cmCapReceive) >= eSirenLPR_Scalable_32kCapCode &&
//			pScm->GetMediaType(cmCapAudio, cmCapReceive) <= eSirenLPRStereo_Scalable_128kCapCode) {
//			AddSingleAudioCap((CapEnum)pScm->GetMediaType(cmCapAudio, cmCapReceive));
//			return;
//		}

		CVideoOperationPointsSet* videoOperationPointsSet=((CIpComMode*)pScm)->GetOperationPoints();
	    const std::list <VideoOperationPoint>* operationPointList=videoOperationPointsSet->GetOperationPointsList();
	    int numOfOperationPoints = operationPointList->size();


//	    std::list <VideoOperationPoint>::const_iterator itr = --operationPointList->end(); - bad
//	    for (int i=operationPointList->size();i>0 &&  itr != operationPointList->end() ; --itr,--i) -bad

//	    std::list <VideoOperationPoint>::const_iterator itr = ++operationPointList->begin(); - good
//	    for (;itr != operationPointList->end() ; ++itr) - good

//	    std::list <VideoOperationPoint>::const_iterator itr = --operationPointList->end(); - bad
//	    for (int i=operationPointList->size()-1;i>0; --itr,--i) - bad

//	    	    std::list <VideoOperationPoint>::const_iterator itr = ++operationPointList->begin();
//	    	    itr++;
//	    	    itr++;
#if 1 // olga - please take this if 0 - it's part of the code!!!! // eyaln9794
	    std::list <VideoOperationPoint>::const_iterator itr = --operationPointList->end();
	    for (int i=operationPointList->size();i>0 &&  itr != operationPointList->end() ; --itr,--i)
	    {
	        const VideoOperationPoint*  operationPoint=&(*itr);
//	        TRACEINTOFUNC<<"@@@! operationPoint.m_frameWidth:"<<operationPoint->m_frameWidth<<" operationPoint.m_frameHeight:"<<operationPoint->m_frameHeight<<" operationPoint.m_frameRate:"<<operationPoint->m_frameRate;

	        CapEnum	eAlg = (CapEnum) pScm->GetMediaType(cmCapVideo, cmCapReceive);
	        AddSingleVideoCapForVideoRelayAvc(eAlg, pScm, videoLineRate,highestframerate,operationPoint);





//	        AddSingleVideoCap(eAlg, pScm, videoLineRate,highestframerate,operationPoint);



//			CapEnum	eAlg = (CapEnum) pScm->GetMediaType(cmCapVideo, cmCapReceive);
//			AddSingleVideoCapEx(eAlg, pScm,operationPoint);
//			AddSingleVideoCap(eAlg, pScm, videoLineRate,highestframerate);

	    }

		//Support LPR for AVC in kVswRelayAvc
		if (pScm->GetIsLpr())
		{
			PTRACE(eLevelInfoNormal,"Adding lpr cap");
			AddLprCap(kRolePeople, pScm->GetConfMediaType(), false);
		}

	    return;
#else
	    std::list <VideoOperationPoint>::const_iterator itr = --operationPointList->end();
	    itr--;
//	    itr--;
		const VideoOperationPoint*  operationPoint=&(*itr);
//	        TRACEINTOFUNC<<"@@@! operationPoint.m_frameWidth:"<<operationPoint->m_frameWidth<<" operationPoint.m_frameHeight:"<<operationPoint->m_frameHeight<<" operationPoint.m_frameRate:"<<operationPoint->m_frameRate;

		CapEnum	eAlg = (CapEnum) pScm->GetMediaType(cmCapVideo, cmCapReceive);
		AddSingleVideoCapForVideoRelayAvc(eAlg, pScm, videoLineRate,highestframerate,operationPoint);

#endif
	}

	if(bMsEnviroment && pScm->GetConfType() != kVSW_Fixed && IsFeatureSupportedBySystem(eFeatureRtv))
	{
		PTRACE(eLevelInfoNormal,"CSipCaps::SetVideo, Add Rtv" );
		arrVideoAlg[0] = eRtvCapCode;
		arrVideoAlg[1] = eH263CapCode;
		arrVideoAlg[2] = eH261CapCode;

		numOfAlg = 3;
	}
	else
	{
		arrVideoAlg[0] = eH263CapCode;
		arrVideoAlg[1] = eH261CapCode;
		arrVideoAlg[2] = eUnknownAlgorithemCapCode;

		numOfAlg = 2;
	}


	PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetVideo, arrVideoAlg[0]:",arrVideoAlg[0] );

	if ( pScm->IsMediaOn(cmCapVideo) != Video_Off )
	{
		if(is_party_encrypted==Encryp_On) {
			AddSdesCaps(cmCapVideo , pScm->GetIsTipMode());
		}

		if(is_party_dtls_encrypted==Encryp_On) {
			AddSingleDtlsCap(cmCapVideo , pScm->GetIsTipMode());
		}

		CapEnum	eAlg = (CapEnum) pScm->GetMediaType(cmCapVideo, cmCapReceive);
		AddSingleVideoCap(eAlg, pScm, videoLineRate, highestframerate, maxResolution);
		BOOL isReduceCapsForRedcom = FALSE;
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		pSysConfig->GetBOOLDataByKey("REDUCE_CAPS_FOR_REDCOM_SIP", isReduceCapsForRedcom);

		if(eAlg == eH261CapCode)
			return;// if the algorithm is H261 we don't add another video codec.
		if(eAlg == eSvcCapCode)
			return;// if the algorithm is Svc we don't add another video codec.
	    if (pScm->GetConfType()==kVswRelayAvc)
	        return; // if VSW relay we don't add another video codec.
	    if(isReduceCapsForRedcom)
	    	numOfAlg =0;

		if (pScm->GetConfType() != kVSW_Fixed)
		{
			for (int i = 0; i < numOfAlg; i++)
			{
				if ( (arrVideoAlg[i] != eAlg) &&  (arrVideoAlg[i]!= eUnknownAlgorithemCapCode))
				{
					PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetVideo, arrVideoAlg[i]:",arrVideoAlg[i] );
					AddSingleVideoCap(arrVideoAlg[i], pScm, videoLineRate, eCopVideoFrameRate_None, maxResolution);
				}
			}
		}
		if(pScm->GetIsLpr() && (!((CComModeH323*)pScm)->GetIsTipMode() || ( (pService != NULL) && (pService->GetSipServerType() != eSipServer_CiscoCucm)) ) )
		{
			PTRACE(eLevelInfoNormal,"Adding lpr cap");
			bool isMrc = (pScm->GetMediaType(cmCapVideo, cmCapReceive) == eSvcCapCode);
			AddLprCap(kRolePeople, pScm->GetConfMediaType(), isMrc);
		}
	}
	else
	{
		CleanMedia(cmCapVideo);
	}
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddSingleVideoCapForVideoRelayAvc(CCapSetInfo capInfo, const CIpComMode* pScm, int videoLineRate,ECopVideoFrameRate highestframerate,const VideoOperationPoint*  pOperationPoint)
{

	int isFreeStruct = NO;

	if (!(capInfo.IsType(cmCapVideo) && capInfo.IsSupporedCap()))
	{

		DBGPASSERT(1);
		return;
	}
	if(pOperationPoint==NULL)
	{

		DBGPASSERT(1);
		return;
	}


	CH264VideoCap* pVideoCapBaseline = (CH264VideoCap *)CBaseCap::AllocNewCap((CapEnum)capInfo/*(CapEnum)pCapBuffer->capTypeCode*/, NULL);
	if(pVideoCapBaseline == NULL)
	{
		DBGPASSERT(1);
		return;
	}



	CBaseCap* pTempCap = pScm->GetMediaAsCapClass(cmCapVideo,  cmCapReceive);
	pVideoCapBaseline->CopyQualities(*pTempCap);
//	pVideoCapBaseline->SetBitRate(pTempCap->GetBitRate());
	POBJDELETE(pTempCap);





	APIU16 profile;
	APIU8 level;
	long mbps, fs, dpb, brAndCpb, sar, staticMB,bitRate;
	ProfileToLevelTranslator plt;
	H264VideoModeDetails h264VidModeDetails;
	((CBaseVideoCap*)pVideoCapBaseline)->SetDefaults(cmCapReceiveAndTransmit/*cmCapReceive*/);
	pVideoCapBaseline->SetDefaults(cmCapReceiveAndTransmit/*cmCapReceive*//*pVideoCap->GetDirection()*/,kRolePeople/*pVideoCap->GetRole()*/);
//	pVideoCapBaseline->CopyQualities(*pVideoCap);

//  pVideoCapBaseline->SetDefaults(cmCapTransmit/*cmCapReceiveAndTransmit*/);

	pScm->GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive);

	pVideoCapBaseline->SetBitRate(pOperationPoint->m_maxBitRate*10/*pVideoCap->GetBitRate()*/);
	TRACEINTO<<"avc_vsw_relay pOperationPoint->m_maxBitRate"<<pOperationPoint->m_maxBitRate<<" pVideoCapBaseline->GetBitRate():"<<pVideoCapBaseline->GetBitRate();


//????				GetH264VideoParams(h264VidModeDetails, callRate*1000 , m_videoQuality, eHD1080Symmetric, FALSE);


//	mbps = (::CalcOperationPointMBPS(*operationPoint)/500);
//	fs = (operationPoint->m_frameWidth * operationPoint->m_frameHeight)>>16;
//    if(fs==0)
//    {
//    	fs=-1;
//    	mbps=-1;
//    }

    if ( SetPredefinedH264ParamsForVswRelayIfNeeded(pScm->GetOperationPointPreset(), level, fs, mbps, pScm->GetIsUseOperationPointesPresets(), *pOperationPoint) == false )
    {
    	mbps = CalcMBPSforVswRelay(*pOperationPoint);
    	fs = CalcFSforVswRelay(*pOperationPoint);
    	ProfileToLevelTranslator plt;
    	level = plt.ConvertResolutionAndRateToLevelEx(fs, mbps);
    }

	profile = ProfileToLevelTranslator::SvcProfileToH264(pOperationPoint->m_videoProfile);

//	pVideoCapBaseline->SetDefaults(cmCapReceive/*pVideoCap->GetDirection()*/,pVideoCap->GetRole());
//						pVideoCapBaseline->SetLevelAndAdditionals(H264_Profile_BaseLine, h264VidModeDetails.levelValue, h264VidModeDetails.maxMBPS,h264VidModeDetails.maxFS, h264VidModeDetails.maxDPB,h264VidModeDetails.maxCPB,((CH264VideoCap *)pVideoCap)->GetSampleAspectRatio(),h264VidModeDetails.maxStaticMbps);
	TRACEINTOFUNC<<"avc_vsw_relay: initialization adding video cap profile:"<<profile<<" level:"<<((int)level)<<" mbps:"<<mbps<<" fs:"<<fs<<" dpb:"<<dpb<<" brAndCpb:"<<brAndCpb<<" sar:"<<sar<<" staticMB:"<<staticMB<<" partyId:";//<<pScm->GetPartyId();
	pVideoCapBaseline->SetLevelAndAdditionals(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
	pVideoCapBaseline->RemoveDefaultAdditionals();
//	pVideoCapBaseline->SetPacketizationMode(0);

	capBuffer* pCapBuffer = pVideoCapBaseline->GetAsCapBuffer();
	if(pCapBuffer==NULL)
	{
		DBGPASSERT(1);
		pVideoCapBaseline->FreeStruct();
		POBJDELETE(pVideoCapBaseline);
		return;
	}
	pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
	h264CapStruct *p264 = ((h264CapStruct *)pCapBuffer->dataCap);
	p264->packetizationMode = H264_SINGLE_NAL_PACKETIZATION_MODE;
	AddCapSet(cmCapVideo, pCapBuffer);

	PDELETEA(pCapBuffer);

	pVideoCapBaseline->FreeStruct();
	POBJDELETE(pVideoCapBaseline);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::UpdateCapsForHdVswInMixedMode(CCapSetInfo capInfo, const CIpComMode* pScm, const VideoOperationPoint*  pOperationPoint)
{
    CSuperLargeString strCaps1;
    this ->DumpToString(strCaps1);
    PTRACE2(eLevelInfoNormal,"CSipCaps::UpdateCapsForHdVswInMixedMode begin:",strCaps1.GetString());

    if (!(capInfo.IsType(cmCapVideo) && capInfo.IsSupporedCap()))
    {
        DBGPASSERT(1);
        return;
    }

    if (pOperationPoint==NULL)
    {
        DBGPASSERT(1);
        return;
    }


    // get Cap from op. point
	CH264VideoCap* pOpPointCap = (CH264VideoCap*)CBaseCap::AllocNewCap(eH264CapCode, NULL);
	if (pOpPointCap==NULL)
	{
		DBGPASSERT(1);
		return;
	}


	pOpPointCap->InitAccordingToOperationPoint(*pOperationPoint);


    // go over all caps and lower the HD720 caps if needed
    int numOfMediaCapSet = 0;
    capBuffer** pMediaCapList = NULL;
    GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

    DWORD valuesToCompare = kCapCode|kH264Profile|kH264Additional|kBitRate;
    DWORD valuesNoProfileToCompare = kCapCode|kH264Additional|kBitRate;
    DWORD details = 0;
    bool flag = false;
    BYTE bIsAtleastHD720 = FALSE;
    DWORD localVideoRate = 0;

    if (pMediaCapList)
    {
        for (int i = 0; i < numOfMediaCapSet; i++)
        {
            if (IsH264Video((CapEnum)pMediaCapList[i]->capTypeCode))
            {
                CH264VideoCap* pCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
        		if (pCap )
                {
					if (pCap->GetCapCode() == eH264CapCode && pCap->GetRole()==kRolePeople)
					{
						//FSN-613: Dynamic Content for SVC/Mix Conf: check if need to limit videorate
						if (!bIsAtleastHD720 && pCap->IsCapableOfHD720At30())
						{
							bIsAtleastHD720 = TRUE;
							localVideoRate = pCap->GetBitRate();
							TRACEINTO << "localVideoRate = " << localVideoRate;
						}

						if (pCap->IsContaining(*pOpPointCap, valuesToCompare, &details))
						{// lower the cap
							TRACEINTO << "Update high profile cap #" << i;
							pCap->SetAccordingToOperationPoint(*pOperationPoint, true);
							flag = true;
						}
						else if (pCap->IsContaining(*pOpPointCap, valuesNoProfileToCompare, &details))
						{// lower the cap
							TRACEINTO << "Update base profile cap #" << i;
							pCap->SetAccordingToOperationPoint(*pOperationPoint, false);
							flag = true;
						}
					}
        			pCap->SetDirection(cmCapReceiveAndTransmit);
                }

        		POBJDELETE(pCap);
            }
        }
    }
	POBJDELETE(pOpPointCap);

	// Update the video bit rate for all caps
	if (flag || (bIsAtleastHD720 && localVideoRate > (pOperationPoint->m_maxBitRate*10)))
	{
		DWORD videoRate = (pOperationPoint->m_maxBitRate + this->GetAudioDesiredRate())*10;
		TRACEINTO << "GetAudioDesiredRate()=" << GetAudioDesiredRate() << " Setting videoBitRate to " << videoRate;
	    	
		 //FSN-613: Dynamic Content for SVC/Mix Conf, since bitRate = 1232 in pOperationPoint for HD1080, while it is different from AVC requirement for HD1080. (1536)
		/*if (pOperationPoint->m_rsrcLevel == eResourceLevel_HD1080)
		{
		       videoRate = pScm->GetVideoBitRate(cmCapReceive, kRolePeople);
			//videoRate = videoRate +  this->GetAudioDesiredRate()*10;
			//videoRate = CResRsrcCalculator::GetRateThrshldBasedOnVideoModeType(GetSystemCardsBasedMode(), m_videoQuality, isHighProfile, eHD1080Symmetric); 
			TRACEINTO << "videoRate = " << videoRate;
		 }*/
		
		SetVideoRateInallCaps(videoRate);
	}
	else
	{
        	TRACEINTO << "Local caps are less than highest operation point. No need to lower them.";
	}

   	CSuperLargeString strCaps2;
	this ->DumpToString(strCaps2);
	PTRACE2(eLevelInfoNormal,"CSipCaps::UpdateCapsForHdVswInMixedMode end:",strCaps2.GetString());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddSingleVideoCap(CCapSetInfo capInfo, const CIpComMode* pScm, int videoLineRate,ECopVideoFrameRate highestframerate, BYTE maxResolution, const VideoOperationPoint* pOperationPoint)
{
	int isFreeStruct = NO;
	EConfType confType = pScm->GetConfType();
	PTRACE2INT(eLevelError,"CSipCaps::AddSingleVideoCap: CapEnum: ",(CapEnum)capInfo);
	Eh264VideoModeType H264VideoModeType = eInvalidModeType;
	BOOL isReduceCapsForRedcom = FALSE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("REDUCE_CAPS_FOR_REDCOM_SIP", isReduceCapsForRedcom);

	if (capInfo.IsType(cmCapVideo) && capInfo.IsSupporedCap())
	{
		CBaseVideoCap* pVideoCap = (CBaseVideoCap*)CBaseCap::AllocNewCap((CapEnum)capInfo, NULL);
		isFreeStruct = YES;
		
		if(pVideoCap == NULL)
		{
			DBGPASSERT(1);
			return;
		}
		
		pVideoCap->SetDefaults(cmCapReceiveAndTransmit);

		if ((CapEnum)capInfo == (CapEnum) pScm->GetMediaType(cmCapVideo, cmCapReceive))
		{
			CBaseCap* pTempCap = pScm->GetMediaAsCapClass(cmCapVideo, cmCapReceive);
			
			if (pTempCap)
			{
			    pVideoCap->CopyQualities(*pTempCap);

//N.A. DEBUG VP8
			    PTRACE2INT(eLevelError,"N.A.DEBUG CSipCaps::AddSingleVideoCap -pTempCap->GetBitRate() = ", pTempCap->GetBitRate());
			    PTRACE2INT(eLevelError,"N.A.DEBUG CSipCaps::AddSingleVideoCap -videoLineRate = ", videoLineRate);
			   if(pTempCap->GetBitRate())
			   {
				   PTRACE2INT(eLevelError,"N.A.DEBUG CSipCaps::AddSingleVideoCap -pTempCap->GetBitRate() = ",pTempCap->GetBitRate());
				   pVideoCap->SetBitRate(pTempCap->GetBitRate());
			   }
			   else
			   {//N.A. DEBUG VP8
				   PTRACE2INT(eLevelError,"N.A.DEBUG CSipCaps::AddSingleVideoCap -pTempCap->GetBitRate() else  videoLineRate = ",videoLineRate);
				   pVideoCap->SetBitRate(videoLineRate);
			   }
			    POBJDELETE(pTempCap);
			}
			else
			    PTRACE(eLevelError,"CSipCaps::AddSingleVideoCap -pTempCap is NULL - SetDefaults to pVideoCap!");
		}
		else
		{
			if ((CapEnum)capInfo == eH261CapCode)
				((CH261VideoCap*)pVideoCap)->SetBitRateWithoutLimitation(videoLineRate);
			else
				pVideoCap->SetBitRate(videoLineRate);

			//struct number 1:
			if (confType == kCp || confType == kCop)
			{
				PTRACE2INT(eLevelError,"CSipCaps::AddSingleVideoCap: CapEnum: ",(CapEnum)capInfo);
				if ((CapEnum)capInfo == eRtvCapCode)
				{
					if (eH264CapCode == (CapEnum)pScm->GetMediaType(cmCapVideo, cmCapReceive))
					{	/// To check....
						eVideoPartyType videoPartyType = pScm->GetVideoPartyType(cmCapReceive);
						PTRACE2INT(eLevelError,"CSipCaps::AddSingleVideoCap: add RTV caps - videoPartyType ",videoPartyType);
						H264VideoModeType =  TranslateCPVideoPartyTypeToMaxH264VideoModeType(videoPartyType);
						pVideoCap->SetRtvCapForCpFromH264VideoType(H264VideoModeType,videoLineRate, maxResolution);
					}
					else if ( eMsSvcCapCode == (CapEnum)pScm->GetMediaType(cmCapVideo, cmCapReceive) )
					{
						eVideoPartyType videoPartyType = pScm->GetVideoPartyType(cmCapReceive);
						PTRACE2INT(eLevelError,"CSipCaps::AddSingleVideoCap: add MS SVC caps - videoPartyType ",videoPartyType);
						H264VideoModeType =  TranslateCPVideoPartyTypeToMaxH264VideoModeType(videoPartyType);
						pVideoCap->SetRtvCapForCpFromH264VideoType(H264VideoModeType,videoLineRate, maxResolution);
					}
				}
				else if ((CapEnum)capInfo == eMsSvcCapCode)
				{
					eVideoPartyType videoPartyType = pScm->GetVideoPartyType(cmCapReceive);
					PTRACE2INT(eLevelError,"CSipCaps::AddSingleVideoCap: add MSSVC Cap - videoPartyType ",videoPartyType);
					Eh264VideoModeType videoModeType = ::TranslateCPVideoPartyTypeToMaxH264VideoModeType(videoPartyType);
					CMsSvcVideoMode MsSvcVidMode;
					MsSvcVideoModeDetails MsSvcDetails;
					MsSvcVidMode.GetMsSvcVideoParamsByRate(MsSvcDetails, videoLineRate*100, videoModeType);
					((CMsSvcVideoCap*)pVideoCap)->SetMsSvcCapForCpFromMsSvcVideoType(MsSvcDetails);
				}
				else if ((CapEnum)capInfo == eVP8CapCode)
				{
					eVideoPartyType videoPartyType = pScm->GetVideoPartyType(cmCapReceive);
					PTRACE2INT(eLevelError,"CSipCaps::AddSingleVideoCap: add VP8 Cap - videoPartyType ",videoPartyType);
					Eh264VideoModeType videoModeType = ::TranslateCPVideoPartyTypeToMaxH264VideoModeType(videoPartyType);
					CVP8VideoMode VP8VideoMode;
					VP8VideoModeDetails VP8Details;
					VP8VideoMode.GetVp8VideoParamsByRate(VP8Details, videoLineRate*100, videoModeType);
					((CVP8VideoCap*)pVideoCap)->SetVP8CapForCpFromVP8VideoType(VP8Details);
				}
				else
				{
					//FROM 7.2 VIDEO RATE IS ACTUALLY CALL RATE WE NEED TO ADJUST THE RATE TO VIDEO RATE
					DWORD audioRate 		= CalculateAudioRate((videoLineRate*100));
					DWORD actualVideoRate 	= videoLineRate - (audioRate/100);
					pVideoCap->SetHighestCapForCpFromScmAndCardValues(actualVideoRate, m_videoQuality);//pass info in 100Bits
				}
			}
			else //VSW
				pVideoCap->SetHighestCapForVswFromScmAndCardValues();
		}

		// if it is H263 we first add a dynamic and then the regular since the new SIP SDP standard declare on H263 with values in SIP with dynamic payload type.
		capBuffer* pCapBuffer = pVideoCap->GetAsCapBuffer();
		
		if (pCapBuffer)
		    pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
		else
		    PTRACE(eLevelError,"CSipCaps::AddSingleVideoCap - pCapBuffer is NULL");

		if (pCapBuffer && pCapBuffer->capTypeCode == eH263CapCode)
		{
			if (GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_VIDEO_DRAFT) >= 5)
			{
				// add H263 dynamic to caps
				int capLen = sizeof(capBufferBase) + pCapBuffer->capLength;
				capBuffer* pDynamicCap = (capBuffer*)(new BYTE[capLen]);
				memcpy(pDynamicCap, pCapBuffer, capLen);
				pDynamicCap->sipPayloadType = capInfo.GetDynamicPayloadType(0); // asking the ip card to allocate dynamic payload
				AddCapSet(cmCapVideo, pDynamicCap);
				PDELETEA(pDynamicCap);
			}
		}

		if (pCapBuffer && pCapBuffer->capTypeCode == eRtvCapCode)
		{
			//Add rtv dynamic
			pCapBuffer->sipPayloadType = capInfo.GetDynamicPayloadType(0);
		}

		if (pCapBuffer && pCapBuffer->capTypeCode == eMsSvcCapCode)
		{
			//Add ms svc  dynamic
			pCapBuffer->sipPayloadType = capInfo.GetDynamicPayloadType(0);
		}

		// Add baseline profile if needed:
		if (pCapBuffer && (IsH264Video((CapEnum)pCapBuffer->capTypeCode)))
		{
			int profileValue = ((h264CapStruct *)pCapBuffer->dataCap)->profileValue;
			
			if (profileValue == H264_Profile_High || profileValue == H264_Profile_Main)
			{
				pCapBuffer->sipPayloadType = capInfo.GetDynamicPayloadType(0, profileValue);
				((h264CapStruct *)pCapBuffer->dataCap)->packetizationMode = H264_NON_INTERLEAVED_PACKETIZATION_MODE;

				PTRACE2INT(eLevelInfoNormal,"CSipCaps::AddSingleVideoCap, TIP - sipPayloadType: ", pCapBuffer->sipPayloadType);
				AddCapSet(cmCapVideo, pCapBuffer);

				CH264VideoCap* pVideoCapBaseline = (CH264VideoCap *)CBaseCap::AllocNewCap((CapEnum)pCapBuffer->capTypeCode, NULL);

				DWORD callRate = pScm->GetCallRate();
				PTRACE2INT(eLevelInfoNormal,"CSipCaps::AddSingleVideoCap pScm->GetCallRate = ", callRate);
				
				if (pVideoCapBaseline != NULL )
				{
					if (confType == kCop)
					{
						pVideoCapBaseline->SetDefaults(pVideoCap->GetDirection(),pVideoCap->GetRole());
						pVideoCapBaseline->CopyQualities(*pVideoCap);
						pVideoCapBaseline->SetBitRate(pVideoCap->GetBitRate());
						long currentFS = pVideoCapBaseline->GetFs();
						
						if (currentFS == -1 )
						{		
							PTRACE(eLevelInfoNormal,"CSipCaps::AddSingleVideoCap -current fs =-1");
							CH264Details thisH264Details = pVideoCapBaseline->GetLevel();
							currentFS = thisH264Details.GetDefaultFsAsProduct();
						}
						else
						{
							currentFS = currentFS * CUSTOM_MAX_FS_FACTOR;
						}
						
						if (isNeedToChangeResOfBaselineAccordingToRate(callRate,currentFS))
						{
							PTRACE(eLevelInfoNormal,"CSipCaps::AddSingleVideoCap -need to change baseline res");
							long levelValue ,maxMBPS,maxFS,maxDPB,maxBR,maxCPB,maxSAR,maxStaticMbps = 0;
							WORD encoderLevel = GetEncoderParamsForNewResOnH264BaseLineCap(callRate,highestframerate,levelValue,maxMBPS,maxFS,maxDPB,maxBR,maxCPB,maxSAR,maxStaticMbps);
							maxSAR = ((CH264VideoCap *)pVideoCap)->GetSampleAspectRatio();
							
							if (encoderLevel != (WORD)-1)
								pVideoCapBaseline->SetLevelAndAdditionals(H264_Profile_BaseLine,levelValue,maxMBPS,maxFS,maxDPB,maxBR,maxSAR,maxStaticMbps);
							else
								PTRACE(eLevelError,"CSipCaps::AddSingleVideoCap -need to change baseline res -can't get incoder index -stay with res to high");
						}
						
						PDELETEA(pCapBuffer);
						pCapBuffer = pVideoCapBaseline->GetAsCapBuffer();
					}
					else
					{ /* CP  */
						pVideoCapBaseline->SetDefaults(pVideoCap->GetDirection(),pVideoCap->GetRole());
						pVideoCapBaseline->CopyQualities(*pVideoCap);
						pVideoCapBaseline->SetBitRate(pVideoCap->GetBitRate());

						CLargeString str;

						str << "pScm->GetCallRate: " << callRate << ", m_videoQuality: " << m_videoQuality;
						PTRACE2(eLevelInfoNormal,"CSipCaps::AddSingleVideoCap - ",str.GetString());
						
						if (maxResolution == eAuto_Res)
						{
							Eh264VideoModeType resourceMaxVideoMode = IsFeatureSupportedBySystem(eFeatureHD1080p60Asymmetric) ? eHD1080At60Asymmetric : eHD1080At60Symmetric;
							H264VideoModeDetails h264VidModeDetails;
							GetH264VideoParams(h264VidModeDetails, callRate*1000 , m_videoQuality, resourceMaxVideoMode, FALSE);
							pVideoCapBaseline->SetLevelAndAdditionals(H264_Profile_BaseLine, h264VidModeDetails.levelValue, h264VidModeDetails.maxMBPS,h264VidModeDetails.maxFS, h264VidModeDetails.maxDPB,h264VidModeDetails.maxCPB,((CH264VideoCap *)pVideoCap)->GetSampleAspectRatio(),h264VidModeDetails.maxStaticMbps);
						}
						else
							pVideoCapBaseline->SetProfile(H264_Profile_BaseLine);

						DWORD details = 0;
						DWORD valuesToCompare = kBitRate|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS| kH264Additional_FS|kRoleLabel;
						
						if (pVideoCap->IsContaining(*pVideoCapBaseline, valuesToCompare, &details))
						{
							PTRACE(eLevelInfoNormal,"CSipCaps::AddSingleVideoCap : Add baseline cap according to decision matrix for baseline profile");
							PDELETEA(pCapBuffer);
							pCapBuffer = pVideoCapBaseline->GetAsCapBuffer();
						}
					}
					
					pVideoCapBaseline->FreeStruct();
					POBJDELETE(pVideoCapBaseline);
				}
				else
					DBGPASSERT(1);

				if (pCapBuffer)
				{
					((h264CapStruct *)pCapBuffer->dataCap)->profileValue = H264_Profile_BaseLine; // we need to set it anyway, in case there was a probelm in creating new cap from the decision matrix.
					pCapBuffer->sipPayloadType = capInfo.GetDynamicPayloadType(H264_Profile_BaseLine);
				}
				
				if (pScm->GetConfType() == kVSW_Fixed || isReduceCapsForRedcom)
				{
					if (pCapBuffer)
						PDELETEA(pCapBuffer);
				}
			}

			// add h264 non interleaved packetization mode if supported
			if (pCapBuffer && pCapBuffer->sipPayloadType == eH264DynamicPayload 
				&& IsFeatureSupportedBySystem(eFeatureH264PacketizationMode))
			{
				PTRACE2INT(eLevelError,"CSipCaps::AddSingleVideoCap: Adding Cap Buffer for dynamic payload type = ", pCapBuffer->sipPayloadType);

				h264CapStruct *p264 = ((h264CapStruct *)pCapBuffer->dataCap);
				
				if (CProcessBase::GetProcess()->GetProductType() != eProductTypeSoftMCUMfw 
					&& !isReduceCapsForRedcom)
				{
					PTRACE2INT(eLevelError,"CSipCaps::AddSingleVideoCap: Adding Cap Buffer for dynamic payload type noa = ", pCapBuffer->sipPayloadType);
					p264->packetizationMode = H264_NON_INTERLEAVED_PACKETIZATION_MODE;
					AddCapSet(cmCapVideo, pCapBuffer);
				}
				
				/* I want to add the same payload type with packetization mode of 0 */
				p264->packetizationMode = H264_SINGLE_NAL_PACKETIZATION_MODE;
				pCapBuffer->sipPayloadType = eH264NoPmDynamicPayload;
			}			
		}

		if (pCapBuffer)
		{
			PTRACE2INT(eLevelError,"CSipCaps::AddSingleVideoCap: Add  cap buffer for dynamic payload type =", pCapBuffer->sipPayloadType);
			AddCapSet(cmCapVideo, pCapBuffer);
			PDELETEA(pCapBuffer);			
		}
		else
		{
			PTRACE(eLevelError,"CSipCaps::AddSingleVideoCap: Create cap buffer has failed");
		}
		
		if (isFreeStruct == YES)
			pVideoCap->FreeStruct();
		
		POBJDELETE(pVideoCap);
	}
}

////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddH264HPCap(const CIpComMode* pScm)
{
		CBaseVideoCap* pVideoCap = (CBaseVideoCap*)CBaseCap::AllocNewCap(eH264CapCode, NULL);
		//isFreeStruct = YES;
		if(pVideoCap == NULL)
		{
			DBGPASSERT(1);
			return;
		}
		pVideoCap->SetDefaults(cmCapReceiveAndTransmit);
		CBaseCap* pTempCap = pScm->GetMediaAsCapClass(cmCapVideo, cmCapReceive);
		if (pTempCap)
		{
		    pVideoCap->CopyQualities(*pTempCap);
		    pVideoCap->SetBitRate(pTempCap->GetBitRate());
		    POBJDELETE(pTempCap);
		}
		else
		    PTRACE(eLevelError,"CSipCaps::AddH264HPCap - pTempCap is NULL - SetDefaults to pVideoCap");
		capBuffer* pCapBuffer = pVideoCap->GetAsCapBuffer();
		if (pCapBuffer)
		{
		    pCapBuffer->sipPayloadType = eH264HpDynamicPayload;
		    //		int profileValue = ((h264CapStruct *)pCapBuffer->dataCap)->profileValue;
		    //if(profileValue == H264_Profile_High)
		    //	pCapBuffer->sipPayloadType = capInfo.GetDynamicPayloadType(0, profileValue);
		    //else
		    //	DBGPASSERT(2);
		    if (pCapBuffer->capTypeCode == eH264CapCode)
		    {
		        AddCapSet(cmCapVideo, pCapBuffer);
		    }
		    PDELETEA(pCapBuffer);
		    pVideoCap->FreeStruct();
		}
		else
		    PTRACE(eLevelError,"CSipCaps::AddH264HPCap - pCapBuffer is NULL - can not AddCapSet");
		POBJDELETE(pVideoCap);
}


/////////////////////////////////////////////////////////////////////////////
void  CSipCaps::SetFecc(const CIpComMode* pScm, const char* pPartyName, DWORD serviceId)
{
//	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();
	BOOL bSipEnableFecc = TRUE;
// 	pSysConfig->GetBOOLDataByKey("SIP_ENABLE_FECC", bSipEnableFecc);
	if( pServiceSysCfg )
	    pServiceSysCfg->GetBOOLDataByKey(serviceId, "SIP_ENABLE_FECC", bSipEnableFecc);
 	if(!bSipEnableFecc)
 	{
 		PTRACE(eLevelError,"CSipCaps::SetFecc: SIP_ENABLE_FECC is false");
 		return;
 	}

	if(strstr(pPartyName, "##FORCE_MEDIA_WITHOUT_FECC")!=NULL)
	{
		PTRACE2(eLevelError,"CSipCaps::SetFecc: FORCE_MEDIA_WITHOUT_FECC - ", pPartyName);
		return;
	}

	if (pScm->IsMediaOff(cmCapData, cmCapTransmit))
	{
		PTRACE(eLevelError,"CSipCaps::SetFecc: Media is off");
		return;
	}

	BOOL bAnnexQFecc = 0;
	//pSysConfig->GetBOOLDataByKey("FECC_ANNEXQ", bAnnexQFecc);
	if( pServiceSysCfg )
	    pServiceSysCfg->GetBOOLDataByKey(serviceId, "FECC_ANNEXQ", bAnnexQFecc);

	if (bAnnexQFecc)
	{
		AddSingleFeccCap(pScm, eAnnexQCapCode);
	}
}

//////////////////////////////////////////////////////////////////////////////
void  CSipCaps::AddSingleFeccCap(const CIpComMode* pScm, CapEnum dataType)
{
	CCapSetInfo capInfo    = dataType;
	BYTE        bSupported = capInfo.IsSupporedCap();

	if (capInfo.IsType(cmCapData) && bSupported)
	{
		BYTE is_party_encrypted = ((CComModeH323*)pScm)->GetIsEncrypted();
		if(is_party_encrypted==Encryp_On) {
			AddSdesCaps(cmCapData);
		}

		BYTE is_party_dtls_encrypted = ((CComModeH323*)pScm)->GetIsDtlsEncrypted();
		if(is_party_dtls_encrypted==Encryp_On) {
			AddSingleDtlsCap(cmCapData, TRUE);
		}

		CH224DataCap* pH224DataCap = (CH224DataCap *)CBaseCap::AllocNewCap(dataType,NULL);

		if (pH224DataCap)
		{
			EResult eResOfSet = pH224DataCap->SetFromIpScm(pScm);
			if (eResOfSet)
			{
				capBuffer* pCapBuffer = pH224DataCap->GetAsCapBuffer();
				if (pCapBuffer)
				{
				    pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
				    AddCapSet(cmCapData,pCapBuffer);
				}
				else
				{
					PTRACE(eLevelError,"CSipCaps::AddSingleFeccCap: Create cap buffer has failed");
				}
				PDELETEA(pCapBuffer);
			}
			else
				PTRACE(eLevelInfoNormal,"CSipCaps::SetFeccCap: there is no feec in the conference.");

			pH224DataCap->FreeStruct();
			POBJDELETE(pH224DataCap);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetContent(const CIpComMode* pScm, const char* pPartyName)
{
	if (pScm->IsMediaOff(cmCapVideo, cmCapReceive, kRolePresentation))
	{
		PTRACE(eLevelError,"CSipCaps::SetContent: Media is off");
		return;
	}
	CapEnum	eAlg = (CapEnum) pScm->GetMediaType(cmCapVideo, cmCapReceive, kRolePresentation);
	ePresentationProtocol contentProtocolMode = pScm->GetContentProtocolMode();

	BYTE isSetH264Content = FALSE;
	BYTE isSetH263Content = FALSE;
	if(eAlg == eH264CapCode)
	{
		if(contentProtocolMode == ePresentationAuto)
		{
			isSetH263Content = TRUE;
		}
		if(contentProtocolMode == ePresentationAuto || contentProtocolMode == eH264Fix ||
			contentProtocolMode == eH264Dynamic)
		{
			isSetH264Content = TRUE;
		}
	}
	if(eAlg == eH263CapCode)
	{
		if(contentProtocolMode == ePresentationAuto ||  contentProtocolMode == eH263Fix)
		{
			isSetH263Content = TRUE;
		}
	}

	if((isSetH264Content == FALSE) && (isSetH263Content == FALSE))
		PTRACE(eLevelInfoNormal,"CSipCaps::SetContent: No content protocls to be set!");

	if (pScm->IsTIPContentEnableInH264Scm() == TRUE || pScm->GetTipContentMode() == eTipCompatiblePreferTIP)
	{
		isSetH263Content = FALSE;
	}
	
	BYTE bIsRcvContentHighProfile = pScm->IsH264HighProfileContent(cmCapReceive);  //HP content
	if (isSetH264Content)
	{
		AddSingleContentCap(pScm, eH264CapCode, bIsRcvContentHighProfile);
	}
	
	//HP content: Add baseline profile cap if needed:
	if (bIsRcvContentHighProfile && contentProtocolMode != eH264Fix)
	{
		AddSingleContentCap(pScm, eH264CapCode);
	}
	
	if (isSetH263Content)
	{
		AddSingleContentCap(pScm, eH263CapCode);
	}

	// AN, 12.9.12, VNGR-26955
	// No need to check &&( (pScm->IsTIPContentEnableInH264Scm() == FALSE) || !((CComModeH323*)pScm)->GetIsTipMode())
	// When checking it, when TIP vidoe + content is enable RMX open RTP with LPR but doesn't send LPR for content in SDP
	// HDX receive LPR for video but not for content and sends video without LPR. RTP can't open becuase it expect to get LPR
	if (pScm->GetIsLpr())
	{
		PTRACE(eLevelInfoNormal,"CSipCaps::SetContent : Adding lpr cap");
        bool isMrc = (pScm->GetMediaType(cmCapVideo, cmCapReceive) == eSvcCapCode);
		AddLprCap(kRolePresentation, pScm->GetConfMediaType(), isMrc);
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddSingleContentCap(const CIpComMode* pScm, CapEnum dataType, BYTE isH264HiProfile)  //HP content
{
	CCapSetInfo capInfo    = dataType;
	CBaseVideoCap* pVideoCap = (CBaseVideoCap*)CBaseCap::AllocNewCap(dataType, NULL);
	if(pVideoCap == NULL)
	{
		DBGPASSERT(1);
		return;
	}

	DWORD contentRate   = pScm->GetContentBitRate(cmCapReceive); //in 100 bit per second

	BYTE is_party_encrypted = ((CComModeH323*)pScm)->GetIsEncrypted();
	if(is_party_encrypted==Encryp_On)
	{
		CSdesCap* CSipCapsTemp = GetSdesCap(cmCapVideo, kRolePresentation); //N.A. BRIDGE-7943
		if(!CSipCapsTemp)
			AddSdesCaps(cmCapVideo, kRolePresentation);
		PDELETE(CSipCapsTemp);
	}

	BYTE is_party_dtls_encrypted = ((CComModeH323*)pScm)->GetIsDtlsEncrypted();
	if(is_party_dtls_encrypted==Encryp_On)
	{
		AddSingleDtlsCap(cmCapVideo, TRUE, kRolePresentation);
	}

	if (pScm->IsTIPContentEnableInH264Scm() == TRUE) //just for TipCompatibility:video&content!
	{
		((CH264VideoCap *)pVideoCap)->SetTIPContent(kRolePresentation, cmCapReceiveAndTransmit);
	}
	else if (pScm->GetTipContentMode() == eTipCompatiblePreferTIP)
	{
		((CH264VideoCap *)pVideoCap)->SetTIPContent(kRolePresentation, cmCapReceiveAndTransmit, FALSE);
	}
	else
	{
		BOOL bContentHD1080Enabled = FALSE;
		BYTE HDResMpi = 0;
		BYTE HDResMpiRcv = 0;
		BYTE HDResMpiTx = 0;
		BOOL bIsRcvContentHighProfile = pScm->IsH264HighProfileContent(cmCapReceive);

		//HP content:
		if (bIsRcvContentHighProfile == isH264HiProfile)
		{
			HDResMpiRcv = pScm->isHDContent1080Supported(cmCapReceive);
			HDResMpiTx  = pScm->isHDContent1080Supported(cmCapTransmit);

			if(HDResMpiRcv && HDResMpiTx)
			{
				bContentHD1080Enabled = TRUE;
				HDResMpi = HDResMpiRcv;

			}
			else
			{
				HDResMpiRcv = pScm->isHDContent720Supported(cmCapReceive);
				HDResMpiTx  = pScm->isHDContent720Supported(cmCapTransmit);
				if(HDResMpiRcv && HDResMpiTx)
				{
					HDResMpi = HDResMpiRcv;
				}
			}
		}
		else if (!isH264HiProfile)
		{
			eCascadeOptimizeResolutionEnum eMaxResolution = e_res_dummy;
			eMaxResolution = (eCascadeOptimizeResolutionEnum)CUnifiedComMode::getMaxContentResolutionbyRateAndProfile(contentRate*100, FALSE);
			PTRACE2INT(eLevelInfoNormal,"CSipCaps::AddSingleContentCap :  Content eMaxResolution: ", eMaxResolution);

			if (eMaxResolution > e_res_720_30fps)
				ON(bContentHD1080Enabled);

			switch(eMaxResolution)
			 {
			 case e_res_720_5fps:
				 HDResMpi = 10;
				 break;
			 case e_res_720_30fps:
				 HDResMpi = 2;
				 break;
			 case e_res_1080_15fps:
				 HDResMpi = 4;
				 break;
			 case e_res_1080_30fps:
				 HDResMpi = 2;
				 break;
			 case e_res_1080_60fps:
				 HDResMpi = 1;
				 break;
			 default:
				 PASSERT(eMaxResolution + 1);
				 HDResMpi = 10;
			 }

			if (bContentHD1080Enabled && HDResMpi < 4) // 1080p30/60
			{
				if(!IsFeatureSupportedBySystem(eFeatureHD1080p30Content))
					HDResMpi = 4;
				else if(HDResMpi == 1 && !IsFeatureSupportedBySystem(eFeatureHD1080p60Content)) 
					HDResMpi = 2;
			}

			PTRACE2INT(eLevelInfoNormal,"CSipCaps::AddSingleContentCap :  Content basline profile MPI: ", HDResMpi);
		}
		else
			PTRACE(eLevelError,"CSipCaps::AddSingleContentCap: should not happen!!!");

		pVideoCap->SetContent(kRolePresentation,cmCapReceiveAndTransmit,bContentHD1080Enabled,HDResMpi,isH264HiProfile);
	}

	pVideoCap->SetBitRate(contentRate);
	capBuffer* pCapBuffer = pVideoCap->GetAsCapBuffer();

	if (pCapBuffer)
	{
	    if (dataType == eH263CapCode)
	    {
	        BYTE isContentH263L = 2;
	        pCapBuffer->sipPayloadType = capInfo.GetDynamicPayloadType(isContentH263L);
	    }
	    else
	    {
			int profileValue = ((h264CapStruct *)pCapBuffer->dataCap)->profileValue;
			pCapBuffer->sipPayloadType = capInfo.GetDynamicPayloadType(0, profileValue);
	    }

	    AddCapSet(cmCapVideo,pCapBuffer);

	    PDELETEA(pCapBuffer);
	}
	else
	    PTRACE(eLevelError,"CSipCaps::AddSingleContentCap: Create cap buffer has failed");

	pVideoCap->FreeStruct();
	POBJDELETE(pVideoCap);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetBfcp(const CIpComMode* pScm, const char* pPartyName)
{
	if(IsMedia(cmCapBfcp))
	{
		PTRACE(eLevelError,"CSipCaps::SetBfcp: Bfcp already exists");
		return;
	}

	if (pScm->IsMediaOn(cmCapBfcp))
	{
		CBfcpCap* pBfcpCap = (CBfcpCap*)CBaseCap::AllocNewCap(eBFCPCapCode, NULL);
		if(pBfcpCap == NULL)
		{
			DBGPASSERT(1);
			return;
		}
		pBfcpCap->SetDefaults(cmCapReceiveAndTransmit);
		capBuffer* pCapBuffer = pBfcpCap->GetAsCapBuffer();
		CCapSetInfo capInfo    = eBFCPCapCode;

		enTransportType transType = pScm->GetBfcpTransportType();

		PTRACE2INT(eLevelError,"CSipCaps::SetBfcp: scm transport type:", transType);

		pBfcpCap->SetTransportType(transType);

		if (pCapBuffer)
		{
		    pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
		    AddCapSet(cmCapBfcp,pCapBuffer);
		}
		else
			PTRACE(eLevelError,"CSipCaps::SetBfcp: Create cap buffer has failed");
		PDELETEA(pCapBuffer);

		pBfcpCap->FreeStruct();
		POBJDELETE(pBfcpCap);
	}
	else
		PTRACE(eLevelError,"CSipCaps::SetBfcp: GetIsBfcp is false");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::CleanMedia(cmCapDataType eMediaType, ERoleLabel eRole)
{
//	CSmallString str;
//	str << "for type - " << ::GetTypeStr(eMediaType);
//	PTRACE2(eLevelInfoNormal,"CSipCaps::CleanMedia, ", str.GetString());
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (numOfMediaCapSet != 0)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
			PDELETEA(pMediaCapList[i]);
		SetNumOfCapSets(0, eMediaType, eRole);
	}
}

void CSipCaps::CleanSdesMedia(cmCapDataType eMediaType, ERoleLabel eRole)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	GetSdesMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (numOfMediaCapSet != 0)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
			PDELETEA(pMediaCapList[i]);
		SetNumOfSdesCapSets(0, eMediaType, eRole);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::CleanIceMedia(ICESessionsTypes SessionType)
{
	int numOfMediaIceCapSet		= 0;
	capBuffer** pMediaIceCapList	= NULL;
	GetMediaIceCaps(SessionType,&numOfMediaIceCapSet,&pMediaIceCapList);
	if (numOfMediaIceCapSet != 0)
	{
		for(int i=0; i<numOfMediaIceCapSet; i++)
			PDELETEA(pMediaIceCapList[i]);

		SetNumOfIceCapSets(0, SessionType);

	}
}

void CSipCaps::CleanAVMCUSdesCaps()
{
	for (int i = 0; i < MaxMsftSvcSdpVideoMlines; i++)
	{
		if(m_msftAVMCUSdesCaps[i])
			PDELETEA(m_msftAVMCUSdesCaps[i]);
	}
	m_numOfMsftAVMCUSdesCaps = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::CleanAll()
{
	CleanMedia(cmCapAudio);
	CleanMedia(cmCapVideo, kRolePeople);
	CleanMedia(cmCapVideo, kRolePresentation);
	CleanMedia(cmCapData);
	CleanMedia(cmCapBfcp);

	CleanIceMedia(eAudioSession);
	CleanIceMedia(eVideoSession);
	CleanIceMedia(eDataSession);
	CleanIceMedia(eGeneralSession);

	/*Need to clear the sdes capsets here */
	CleanSdesMedia(cmCapAudio);
	CleanSdesMedia(cmCapVideo, kRolePeople);
	CleanSdesMedia(cmCapVideo, kRolePresentation);
	CleanSdesMedia(cmCapData);
	m_sessionLevelRate = 0xFFFFFFFF;
	m_msftVideoRxBw = 0;
	
	m_msftSsrcAudio = 0;
	memset(m_msftSsrcVideo,0,sizeof(m_msftSsrcVideo));
	m_msftMsiAudio = 0;
	m_encryptionKeyToUse=eUseBothEncryptionKeys;
	m_bUseNonMkiOrderFirst=FALSE; //BRIDGE-11708
	memset(m_msftMsiVideo,0,sizeof(m_msftMsiVideo));

	//sdes avmcu
	CleanAVMCUSdesCaps();
}
////////////////////////////////////////////////////////////////////
void CSipCaps::CreateIgnoringRemoveCodec(const sipSdpAndHeadersSt & sdp, eConfMediaType aConfMediaType, bool aIsMrcCall)
{
	BYTE bIsRemoveCodecs = FALSE;
	Create(sdp, aConfMediaType, aIsMrcCall, bIsRemoveCodecs);
}

////////////////////////////////////////////////////////////////////
void CSipCaps::Create(const sipSdpAndHeadersSt & sdp, eConfMediaType aConfMediaType, bool aIsMrcCall, BYTE bIsRemoveCodec, RemoteIdent aRemoteId)
{
	// start from clean cap set
	CleanAll();


	if (sdp.sipMediaLinesLength)
	{
		if(0xFFFFFFFF == sdp.callRate){
			//if not set call rate, m_sessionLevelRate will be meaningless .So set m_sessionLevelRate as NO_SESSION_LEVEL_RATE at beginning.
			m_sessionLevelRate = NO_SESSION_LEVEL_RATE; 
			PTRACE(eLevelInfoNormal,"CSipCaps::Create -no call rate");
		}

		// if the remote doesn't declare on any rate we set a rate to its SDP according to our capabilities
		::SDPRateAlignment((sipSdpAndHeadersSt*) &sdp);

		if(m_sessionLevelRate!= NO_SESSION_LEVEL_RATE){
   
		    m_sessionLevelRate = sdp.callRate;
			PTRACE(eLevelInfoNormal,"CSipCaps::Create -call rate exist");
        }

		m_msftVideoRxBw    = sdp.msVideoRateRx * 10;

		BOOL isANATContained = IsANATPresentInSDP((sipSdpAndHeadersSt*) &sdp); //added for ANAT
		BOOL isAVMCUCall = IsAVMCUCall((sipSdpAndHeadersSt*) &sdp);
		const sipMediaLinesEntrySt* pMediaLinesEntry = (const sipMediaLinesEntrySt*) &sdp.capsAndHeaders[sdp.sipMediaLinesOffset];
		int mediaLinePos = 0;
		BYTE bIsFirstVideoLine = TRUE;
		int  indexVideoMediaLine = -1; // counts video media lines in SDP
		BYTE bIsFirstAudioLineForANAT = TRUE;
		BYTE bIsFirstVideoLineForANAT = TRUE;
		BYTE bIsFirstContentLineForANAT = TRUE;
		BYTE bIsFirstFeccLineForANAT = TRUE;
		BYTE bIsFirstBfcpLineForANAT = TRUE;

		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++)
		{
			const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;			
			//added for ANAT
			if (isANATContained)
			{
				BYTE bIsTaken = TRUE;
				if (pMediaLine->mediaIp.transAddr.port == 0)
					bIsTaken = FALSE;
				
				if ((kMediaLineInternalTypeAudio == pMediaLine->internalType && (!bIsFirstAudioLineForANAT || !bIsTaken)) ||
				     (kMediaLineInternalTypeVideo == pMediaLine->internalType && (!bIsFirstVideoLineForANAT || !bIsTaken))	||
				     (kMediaLineInternalTypeContent == pMediaLine->internalType && (!bIsFirstContentLineForANAT  || !bIsTaken))	||
				     (kMediaLineInternalTypeFecc== pMediaLine->internalType && (!bIsFirstFeccLineForANAT || !bIsTaken))	||
				     (kMediaLineInternalTypeBfcp == pMediaLine->internalType && (!bIsFirstBfcpLineForANAT || !bIsTaken)))
					continue;
			}

				
			const capBuffer* pCapBuffer = (capBuffer*) &pMediaLine->caps[0];
			const BYTE*	pTemp = (const BYTE*)pCapBuffer;

			//mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

			if (pMediaLine->type == eMediaLineTypeNotSupported)
				continue;

			// added for Lync2013 avmcu
			if(indexVideoMediaLine != -1 && pMediaLine->type == eMediaLineTypeVideo &&
					 (pMediaLine->internalType == kMediaLineInternalTypeNotSupported  ||  pMediaLine->internalType == kMediaLineInternalTypeVideo) )
			{
				PTRACE(eLevelInfoNormal,"CSipCaps::Create - do not add caps of simulcast video m line and of panoramic m line");
			} else
			{
				for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
				{


					CCapSetInfo capInfo = (CapEnum)pCapBuffer->capTypeCode;
					cmCapDataType eType = capInfo.GetSipCapType();

					PTRACE2INT(eLevelInfoNormal,"CSipCaps::Create - cap info:",capInfo);
					PTRACE2INT(eLevelInfoNormal,"CSipCaps::Create - length:",pCapBuffer->capLength);

					// if the IP or BW = 0 it means we can't transmit to the remote only it can transmit to us.
					// therefore we set the direction to transmit only in the capabilities.
					BYTE bIsZeroBandwidth = NO;
					ERoleLabel eRole = (ERoleLabel)((BaseCapStruct*)pCapBuffer->dataCap)->header.roleLabel;
					if ((pCapBuffer->capTypeCode) == eLPRCapCode &&
							(eRole == kRolePeople ||
							((aConfMediaType==eMixAvcSvc) && aIsMrcCall && eRole == kRolePresentation)
							|| ((aRemoteId == MicrosoftEP_Lync_CCS) &&  eRole == kRolePresentation)))
					{
						PTRACE(eLevelInfoNormal,"CSipCaps::Create -FOUND LPR");
						m_bIsLpr = TRUE;
					}

					//LYNC2013_FEC_RED:
					if ( pCapBuffer->capTypeCode == eFECCapCode)
					{
						PTRACE(eLevelInfoNormal,"CSipCaps::Create - LYNC2013_FEC_RED: FOUND FEC!!");
						m_bIsFec = TRUE;
					}

					if ( pCapBuffer->capTypeCode == eREDCapCode)
					{
						PTRACE(eLevelInfoNormal,"CSipCaps::Create - LYNC2013_FEC_RED: FOUND RED!!");
						m_bIsRed = TRUE;
					}

					if(eType == cmCapVideo)
					{// if the bandwidth of the media is zero we act the same as if it was port=0.

						switch(pCapBuffer->capTypeCode)
						{
							case eH261CapCode:
							{
								if(((h261CapStruct *)pCapBuffer->dataCap)->maxBitRate == 0)
									bIsZeroBandwidth = YES;
								break;
							}
							case eH263CapCode:
							{
								if(((h263CapStruct *)pCapBuffer->dataCap)->maxBitRate == 0)
									bIsZeroBandwidth = YES;
								break;
							}
							case eH264CapCode:
							case eSvcCapCode:
							{
								if(((h264CapStruct *)pCapBuffer->dataCap)->maxBitRate == 0)
									bIsZeroBandwidth = YES;

								break;
							}
							case eVP8CapCode:
							{//N.A. DEBUG VP8
								PTRACE(eLevelInfoNormal,"N.A. DEBUG CSipCaps::Create - eVP8CapCode case ");
								if(((vp8CapStruct *)pCapBuffer->dataCap)->maxBitRate == 0)
								{
									bIsZeroBandwidth = YES;
								}
								if(((vp8CapStruct *)pCapBuffer->dataCap)->maxBitRate < 0)
								{
									((vp8CapStruct *)pCapBuffer->dataCap)->maxBitRate = 1920*10;
								}
								if(((vp8CapStruct *)pCapBuffer->dataCap)->maxFR <= 0)
								{
									((vp8CapStruct *)pCapBuffer->dataCap)->maxFR = 30;
								}
								if(((vp8CapStruct *)pCapBuffer->dataCap)->maxFS <= 0)
								{
									((vp8CapStruct *)pCapBuffer->dataCap)->maxFS = 1280;
								}
								if(((vp8CapStruct *)pCapBuffer->dataCap)->rtcpFeedbackMask <= 0)
								{
									((vp8CapStruct *)pCapBuffer->dataCap)->rtcpFeedbackMask = (RTCP_MASK_FIR | RTCP_MASK_TMMBR);
								}

								
								break;
							}
							case eRtvCapCode:
							{
								if((((rtvCapStruct *)pCapBuffer->dataCap)->rtvCapItem[0]).maxBitrateInBps == 0)
									bIsZeroBandwidth = YES;
								break;
							}
						}
					}
					else
						bIsZeroBandwidth = (0 == sdp.callRate) ? YES : NO;

					if ((CapEnum)pCapBuffer->capTypeCode == eSdesCapCode) {
						//PTRACE(eLevelInfoNormal,"CSipCaps::Create -FOUND SDES");
						switch(pMediaLine->internalType)
						{
							case kMediaLineInternalTypeAudio:
								PTRACE(eLevelInfoNormal,"CSipCaps::Create -FOUND SDES for audio");
								AddSdesCapSet(cmCapAudio,pCapBuffer);
								break;
							case kMediaLineInternalTypeVideo:

								if(bIsFirstVideoLine)
								{
									PTRACE(eLevelInfoNormal,"CSipCaps::Create -FOUND SDES for video");
									AddSdesCapSet(cmCapVideo,pCapBuffer);
								} else
								{
									PTRACE(eLevelInfoNormal,"CSipCaps::Create -SDES for video not added for more than one video");
								}
								break;
							case kMediaLineInternalTypeFecc:
								PTRACE(eLevelInfoNormal,"CSipCaps::Create -FOUND SDES for data");
								AddSdesCapSet(cmCapData,pCapBuffer);
								break;
							case kMediaLineInternalTypeContent:
								PTRACE(eLevelInfoNormal,"CSipCaps::Create -FOUND SDES for content");
								AddSdesCapSet(cmCapVideo,pCapBuffer,kRolePresentation);
								break;
							default:
								PTRACE(eLevelInfoNormal,"CSipCaps::Create -FOUND SDES unknown");
								break;
						}
					}

					if(!bIsRemoveCodec)// if we don't want to remove codecs, we always add them to the caps.
					{
						AddCapSet(eType,pCapBuffer);
					}
					else if ( (eType == cmCapAudio || eType == cmCapVideo || eType == cmCapData || (eType == cmCapBfcp/*cmCapGeneric && pCapBuffer->capTypeCode==eBFCPCapCode*/) )
						//&& (ExtractMLineMediaIp(GetCardSdpArrayIndex(eType), &sdp, m_dummyMediaIp).transAddr.port)
						&& (pMediaLine->mediaIp.transAddr.port)
						&& (!bIsZeroBandwidth))
					{
						//if ( !(pCapBuffer->capTypeCode == eREDCapCode) && !(pCapBuffer->capTypeCode == eFECCapCode) )
								AddCapSet(eType,pCapBuffer);
					}

					pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
					pCapBuffer = (capBuffer*)pTemp;
				}
			}

			if(pMediaLine->type == eMediaLineTypeVideo  && pMediaLine->internalType == kMediaLineInternalTypeVideo) // added for lync2013 avmcu
				indexVideoMediaLine ++;

			if(bIsFirstVideoLine && pMediaLine->internalType == kMediaLineInternalTypeVideo)
			{
				bIsFirstVideoLine = FALSE;
			}
			//added for ANAT begin
			if (isANATContained)
			{
				switch(pMediaLine->internalType)
				{
					case kMediaLineInternalTypeAudio:
						bIsFirstAudioLineForANAT = FALSE;
						break;
					case kMediaLineInternalTypeVideo:
						bIsFirstVideoLineForANAT = FALSE;
						break;
					case kMediaLineInternalTypeFecc:
						bIsFirstFeccLineForANAT = FALSE;
						break;
					case kMediaLineInternalTypeContent:
						bIsFirstContentLineForANAT = FALSE;
						break;
					case kMediaLineInternalTypeBfcp:
						bIsFirstBfcpLineForANAT = FALSE;
						break;
					default:
						PTRACE(eLevelInfoNormal,"CSipCaps::Create -FOUND internalType unknown");
						break;
				}
			}
			//added for ANAT end

			// Lync2013 - set msi and ssrc range for audio and video + save sdes caps of the simulcast mlines
			if(pMediaLine->type == eMediaLineTypeAudio)
			{
				m_msftMsiAudio = pMediaLine->msi;

				m_msftSsrcAudio = pMediaLine->ssrcrange[0];
			}
			else if (pMediaLine->type == eMediaLineTypeVideo  && pMediaLine->internalType == kMediaLineInternalTypeVideo && indexVideoMediaLine > -1 && indexVideoMediaLine < MaxMsftSvcSdpVideoMlines)
			{
				m_msftMsiVideo[indexVideoMediaLine] = pMediaLine->msi;

				m_msftSsrcVideo[indexVideoMediaLine][0] = pMediaLine->ssrcrange[0];
				m_msftSsrcVideo[indexVideoMediaLine][1] = pMediaLine->ssrcrange[1];

				if(isAVMCUCall)
					GetVideoMLineSdesCapSetForAVMCU(indexVideoMediaLine, pMediaLine);
			}
		}
	}
	else
	{
		DBGPASSERT(YES);
		PTRACE2INT(eLevelInfoNormal,"CSipCaps::Create: No cap set. Length of dynamic section: ", sdp.lenOfDynamicSection);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::Serialize(WORD format, CSegment& seg) const
{
	if (format == NATIVE)
	{
		int curCapBufferSize = 0;

		seg << (DWORD)m_numOfAudioCapSets;
		for(int i=0; i<m_numOfAudioCapSets; i++)
		{
			if (m_audioCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_audioCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_audioCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}

		seg << (DWORD) m_numOfVideoCapSets;
		for(int i = 0; i < m_numOfVideoCapSets; i++)
		{
			if (m_videoCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_videoCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_videoCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}

		seg << (DWORD) m_numOfFeccCapSets;
		for(int i = 0; i < m_numOfFeccCapSets; i++)
		{
			if (m_feccCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_feccCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_feccCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}

		seg << (DWORD) m_numOfContentCapSets;
		for(int i = 0; i < m_numOfContentCapSets; i++)
		{
			if (m_contentCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_contentCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_contentCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}

		seg << (DWORD) m_numOfBfcpCapSets;
		for(int i = 0; i < m_numOfBfcpCapSets; i++)
		{
			if (m_bfcpCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_bfcpCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_bfcpCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}
		seg << (BYTE)m_h263_4CifMpi;
		seg << (BYTE)m_videoQuality;
        seg << (WORD)m_bIsLpr;

        //LYNC2013_FEC_RED
        seg << (WORD)m_bIsFec;
        seg << (WORD)m_bIsRed;

		//SDES
		seg << (DWORD) m_numOfAudioSdesCapSets;
		for(int i = 0; i < m_numOfAudioSdesCapSets; i++)
		{
			if (m_audioSdesCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_audioSdesCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_audioSdesCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}

		seg << (DWORD) m_numOfVideoSdesCapSets;
		for(int i = 0; i < m_numOfVideoSdesCapSets; i++)
		{
			if (m_videoSdesCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_videoSdesCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_videoSdesCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}



		seg << (DWORD) m_numOfFeccSdesCapSets;
		for(int i = 0; i < m_numOfFeccSdesCapSets; i++)
		{
			if (m_feccSdesCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_feccSdesCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_feccSdesCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}

		seg << (DWORD) m_numOfContentSdesCapSets;
		for(int i = 0; i < m_numOfContentSdesCapSets; i++)
		{
			if (m_contentSdesCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_contentSdesCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_contentSdesCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}

		seg << (DWORD)m_numOfIceAudioCaps;

		if(m_numOfIceAudioCaps)
		{
			seg << (DWORD)m_AudioHostPartyAddr.ipVersion ;
			seg << (DWORD)m_AudioHostPartyAddr.port;
			seg << (DWORD)m_AudioHostPartyAddr.distribution;
			seg << (DWORD)m_AudioHostPartyAddr.transportType;
			if ((enIpVersion)m_AudioHostPartyAddr.ipVersion == eIpVersion4)
			seg << (DWORD)m_AudioHostPartyAddr.addr.v4.ip;
		}
		for(int i=0; i<m_numOfIceAudioCaps; i++)
		{
			if (m_IceAudioCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_IceAudioCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_IceAudioCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}


		seg << (DWORD)m_numOfIceVideoCaps;


		if(m_numOfIceVideoCaps)
		{
			seg << (DWORD)m_VideoHostPartyAddr.ipVersion ;
			seg << (DWORD)m_VideoHostPartyAddr.port;
			seg << (DWORD)m_VideoHostPartyAddr.distribution;
			seg << (DWORD)m_VideoHostPartyAddr.transportType;
			if ((enIpVersion)m_VideoHostPartyAddr.ipVersion == eIpVersion4)
				seg << (DWORD)m_VideoHostPartyAddr.addr.v4.ip;
		}


		for(int i=0; i<m_numOfIceVideoCaps; i++)
		{
			if (m_IceVideoCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_IceVideoCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_IceVideoCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}
		seg << (DWORD)m_numOfIceDataCaps;

		if(m_numOfIceDataCaps)
		{
			seg << (DWORD)m_DataHostPartyAddr.ipVersion ;
			seg << (DWORD)m_DataHostPartyAddr.port;
			seg << (DWORD)m_DataHostPartyAddr.distribution;
			seg << (DWORD)m_DataHostPartyAddr.transportType;
			if ((enIpVersion)m_DataHostPartyAddr.ipVersion == eIpVersion4)
				seg << (DWORD)m_DataHostPartyAddr.addr.v4.ip;
		}

		for(int i=0; i<m_numOfIceDataCaps; i++)
		{
			if (m_IceDataCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_IceDataCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_IceDataCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}

		seg << (DWORD)m_numOfIceGeneralCaps;
		for(int i=0; i<m_numOfIceGeneralCaps; i++)
		{
			if (m_IceGeneralCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_IceGeneralCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_IceGeneralCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}
		seg << (WORD)m_TipAuxFPS;

		//DTLS
		seg << (DWORD) m_numOfAudioDtlsCapSets;
		for(int i = 0; i < m_numOfAudioDtlsCapSets; i++)
		{
			if (m_audioDtlsCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_audioDtlsCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_audioDtlsCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}

		seg << (DWORD) m_numOfVideoDtlsCapSets;
		for(int i = 0; i < m_numOfVideoDtlsCapSets; i++)
		{
			if (m_videoDtlsCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_videoDtlsCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_videoDtlsCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}

		seg << (DWORD) m_numOfFeccDtlsCapSets;
		for(int i = 0; i < m_numOfFeccDtlsCapSets; i++)
		{
			if (m_feccDtlsCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_feccDtlsCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_feccDtlsCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}

		seg << (DWORD) m_numOfContentDtlsCapSets;
		for(int i = 0; i < m_numOfContentDtlsCapSets; i++)
		{
			if (m_contentDtlsCapList[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_contentDtlsCapList[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_contentDtlsCapList[i],curCapBufferSize);
			}
			else
				PASSERT(YES);
		}
		seg << m_sessionLevelRate;
		seg << m_msftVideoRxBw;
		seg << m_msftSsrcAudio;
		for (int i=0; i<MaxMsftSvcSdpVideoMlines; i++)
		{
			seg<<m_msftSsrcVideo[i][0];
			seg<<m_msftSsrcVideo[i][1];
		}
		seg << m_msftMsiAudio;
		for(int i=0; i<MaxMsftSvcSdpVideoMlines; i++)
		{
			seg << m_msftMsiVideo[i];
		}
		seg << m_encryptionKeyToUse;

		seg << m_bUseNonMkiOrderFirst; //BRIDGE-11708

		//avmcu sdes
		seg << m_numOfMsftAVMCUSdesCaps;
		for(DWORD i=0; i<m_numOfMsftAVMCUSdesCaps; i++)
		{
			if (m_msftAVMCUSdesCaps[i])
			{
				curCapBufferSize = sizeof(capBufferBase) + m_msftAVMCUSdesCaps[i]->capLength;
				seg << (DWORD)curCapBufferSize;
				seg.Put((BYTE *)m_msftAVMCUSdesCaps[i],curCapBufferSize);
			} //else
				//PASSERT(YES);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::DeSerialize(WORD format, CSegment& seg)
{
	if (format == NATIVE)
	{
		for(int i = 0; i < MAX_MEDIA_CAPSETS; i++)
		{
			PDELETEA(m_audioCapList[i]);
			PDELETEA(m_videoCapList[i]);
			PDELETEA(m_feccCapList[i]);
			PDELETEA(m_contentCapList[i])
			PDELETEA(m_bfcpCapList[i])
		}

		for(int i = 0; i < MAX_SDES_CAPSETS; i++)
		{
			PDELETEA(m_audioSdesCapList[i]);
			PDELETEA(m_videoSdesCapList[i]);
			PDELETEA(m_feccSdesCapList[i]);
			PDELETEA(m_contentSdesCapList[i]);
		}


		seg >> (DWORD&)m_numOfAudioCapSets;
		int curCapBufferSize;

		for(int i = 0; i < m_numOfAudioCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_audioCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_audioCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_audioCapList[i],curCapBufferSize);
		}

		seg >> (DWORD&)m_numOfVideoCapSets;
		for(int i= 0; i < m_numOfVideoCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_videoCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_videoCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_videoCapList[i], curCapBufferSize);
		}

		seg >> (DWORD&)m_numOfFeccCapSets;
		for(int i= 0; i < m_numOfFeccCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_feccCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_feccCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_feccCapList[i], curCapBufferSize);
		}

		seg >> (DWORD&)m_numOfContentCapSets;
		for(int i= 0; i < m_numOfContentCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_contentCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_contentCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_contentCapList[i], curCapBufferSize);
		}

		seg >> (DWORD&)m_numOfBfcpCapSets;
		for(int i= 0; i < m_numOfBfcpCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_bfcpCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_bfcpCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_bfcpCapList[i], curCapBufferSize);
		}
		BYTE h263_4CifMpi;
		seg >> h263_4CifMpi;
		m_h263_4CifMpi = (APIS8)h263_4CifMpi;

		BYTE videoQuality;
		seg >> videoQuality;
		m_videoQuality = (eVideoQuality)videoQuality;
		WORD islpr;
		seg >> islpr;
		m_bIsLpr = (BYTE)islpr;

		//LYNC2013_FEC_RED:
		WORD isfec, isred;
		seg >> isfec;
		m_bIsFec = (BYTE)isfec;
		seg >> isred;
		m_bIsRed = (BYTE)isred;

		//SDES
		seg >> (DWORD&)m_numOfAudioSdesCapSets;
		for(int i= 0; i < m_numOfAudioSdesCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_audioSdesCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_audioSdesCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_audioSdesCapList[i], curCapBufferSize);
		}

		seg >> (DWORD&)m_numOfVideoSdesCapSets;
		for(int i= 0; i < m_numOfVideoSdesCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_videoSdesCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_videoSdesCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_videoSdesCapList[i], curCapBufferSize);
		}

		seg >> (DWORD&)m_numOfFeccSdesCapSets;
		for(int i= 0; i < m_numOfFeccSdesCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_feccSdesCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_feccSdesCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_feccSdesCapList[i], curCapBufferSize);
		}

		seg >> (DWORD&)m_numOfContentSdesCapSets;
		for(int i= 0; i < m_numOfContentSdesCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_contentSdesCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_contentSdesCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_contentSdesCapList[i], curCapBufferSize);
		}

		seg >> (DWORD&)m_numOfIceAudioCaps;

		if(m_numOfIceAudioCaps)
		{
			seg >> m_AudioHostPartyAddr.ipVersion;
			seg >> m_AudioHostPartyAddr.port;
			seg >> m_AudioHostPartyAddr.distribution;
			seg >> m_AudioHostPartyAddr.transportType;
			if ((enIpVersion)m_AudioHostPartyAddr.ipVersion == eIpVersion4)
				seg >> m_AudioHostPartyAddr.addr.v4.ip;
		}

		for(int i= 0; i < m_numOfIceAudioCaps; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_IceAudioCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_IceAudioCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_IceAudioCapList[i], curCapBufferSize);
		}
		seg >> (DWORD&)m_numOfIceVideoCaps;

		if(m_numOfIceVideoCaps)
		{
			seg >> m_VideoHostPartyAddr.ipVersion;
			seg >> m_VideoHostPartyAddr.port;
			seg >> m_VideoHostPartyAddr.distribution;
			seg >> m_VideoHostPartyAddr.transportType;
			if ((enIpVersion)m_VideoHostPartyAddr.ipVersion == eIpVersion4)
				seg >> m_VideoHostPartyAddr.addr.v4.ip;
		}

		for(int i= 0; i < m_numOfIceVideoCaps; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_IceVideoCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_IceVideoCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_IceVideoCapList[i], curCapBufferSize);
		}
		seg >> (DWORD&)m_numOfIceDataCaps;

		if(m_numOfIceDataCaps)
		{
			seg >> m_DataHostPartyAddr.ipVersion;
			seg >> m_DataHostPartyAddr.port;
			seg >> m_DataHostPartyAddr.distribution;
			seg >> m_DataHostPartyAddr.transportType;
			if ((enIpVersion)m_DataHostPartyAddr.ipVersion == eIpVersion4)
				seg >> m_DataHostPartyAddr.addr.v4.ip;
		}

		for(int i= 0; i < m_numOfIceDataCaps; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_IceDataCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_IceDataCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_IceDataCapList[i], curCapBufferSize);
		}
		seg >> (DWORD&)m_numOfIceGeneralCaps;
		for(int i= 0; i < m_numOfIceGeneralCaps; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_IceGeneralCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_IceGeneralCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_IceGeneralCapList[i], curCapBufferSize);
		}
		WORD tipAux = 0;
		seg >> tipAux;
		m_TipAuxFPS = (ETipAuxFPS)tipAux;

		//DTLS
		seg >> (DWORD&)m_numOfAudioDtlsCapSets;
		for(int i= 0; i < m_numOfAudioDtlsCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_audioDtlsCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_audioDtlsCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_audioDtlsCapList[i], curCapBufferSize);
		}

		seg >> (DWORD&)m_numOfVideoDtlsCapSets;
		for(int i= 0; i < m_numOfVideoDtlsCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_videoDtlsCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_videoDtlsCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_videoDtlsCapList[i], curCapBufferSize);
		}

		seg >> (DWORD&)m_numOfFeccDtlsCapSets;
		for(int i= 0; i < m_numOfFeccDtlsCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_feccDtlsCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_feccDtlsCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_feccDtlsCapList[i], curCapBufferSize);
		}

		seg >> (DWORD&)m_numOfContentDtlsCapSets;
		for(int i= 0; i < m_numOfContentDtlsCapSets; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_contentDtlsCapList[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_contentDtlsCapList[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_contentDtlsCapList[i], curCapBufferSize);
		}
		seg >> m_sessionLevelRate;
		seg >> m_msftVideoRxBw;
		
		seg >> m_msftSsrcAudio;
		for (int i=0; i<MaxMsftSvcSdpVideoMlines; i++)
		{
			seg>>m_msftSsrcVideo[i][0];
			seg>>m_msftSsrcVideo[i][1];
		}
		seg >> m_msftMsiAudio;
		for(int i=0; i<MaxMsftSvcSdpVideoMlines; i++)
		{
			seg >> m_msftMsiVideo[i];
		}
		seg >> m_encryptionKeyToUse;
		seg >> m_bUseNonMkiOrderFirst;
		
		// avmcu sdes
		for(int i = 0; i < MaxMsftSvcSdpVideoMlines; i++)
		{
			if(m_msftAVMCUSdesCaps[i])
			   PDELETEA(m_msftAVMCUSdesCaps[i]);
		}

		seg >> (DWORD&)m_numOfMsftAVMCUSdesCaps;

		for(DWORD i = 0; i < m_numOfMsftAVMCUSdesCaps; i++)
		{
			seg >> (DWORD&)curCapBufferSize;
			m_msftAVMCUSdesCaps[i] = (capBuffer*)(new BYTE[curCapBufferSize]);
			memset(m_msftAVMCUSdesCaps[i], 0, curCapBufferSize);
			seg.Get((BYTE*)m_msftAVMCUSdesCaps[i], curCapBufferSize);
		}

	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Must be exactly like the function in h323 caps, because the deserialize function is the same!
//---------------------------------------------------------------------------
void CSipCaps::SerializeCapArrayOnly(CSegment& seg, BYTE bOperUse)
{
	WORD	totalSize = CalcCapBuffersSizeWithSdes(cmCapReceiveAndTransmit, bOperUse, FALSE);

	BYTE*	capsList  = new BYTE[totalSize];
	memset(capsList, 0, totalSize);

	int numOfCapBuffers = 0;

	WORD offsetWrite = CopyCapBuffersWithSdesToBuffer(capsList,	totalSize, &numOfCapBuffers,
												cmCapReceiveAndTransmit, NO, bOperUse, FALSE);

	//TRACEINTO << "offsetWrite="<<offsetWrite<<" totalSize="<<totalSize;
	DBGPASSERT(offsetWrite != totalSize);
		seg << offsetWrite;
	seg.Put(capsList, offsetWrite);
	PDELETEA(capsList);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CSipCaps& CSipCaps::operator=(const CSipCaps& other)
{
	if(this != &other)
	{
		// start from clean cap set
		CleanAll();

		//audio
		m_numOfAudioCapSets	= other.m_numOfAudioCapSets;
		int curLen;
		for(int i = 0; i < m_numOfAudioCapSets; i++)
		{
			curLen = other.m_audioCapList[i]->capLength;
			m_audioCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_audioCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_audioCapList[i]->capLength		= curLen;
			m_audioCapList[i]->capTypeCode		= other.m_audioCapList[i]->capTypeCode;
			m_audioCapList[i]->sipPayloadType	= other.m_audioCapList[i]->sipPayloadType;
			memcpy(m_audioCapList[i]->dataCap, other.m_audioCapList[i]->dataCap, curLen);
		}

		//video
		m_numOfVideoCapSets	= other.m_numOfVideoCapSets;
		for(int i = 0; i < m_numOfVideoCapSets; i++)
		{
			curLen = other.m_videoCapList[i]->capLength;
			m_videoCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_videoCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_videoCapList[i]->capLength		= curLen;
			m_videoCapList[i]->capTypeCode		= other.m_videoCapList[i]->capTypeCode;
			m_videoCapList[i]->sipPayloadType	= other.m_videoCapList[i]->sipPayloadType;

			memcpy(m_videoCapList[i]->dataCap,other.m_videoCapList[i]->dataCap,curLen);
		}

		//fecc
		m_numOfFeccCapSets	= other.m_numOfFeccCapSets;
		for(int i = 0; i < m_numOfFeccCapSets; i++)
		{
			curLen = other.m_feccCapList[i]->capLength;
			m_feccCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_feccCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_feccCapList[i]->capLength		= curLen;
			m_feccCapList[i]->capTypeCode		= other.m_feccCapList[i]->capTypeCode;
			m_feccCapList[i]->sipPayloadType	= other.m_feccCapList[i]->sipPayloadType;
			memcpy(m_feccCapList[i]->dataCap,other.m_feccCapList[i]->dataCap,curLen);
		}

		//content
		m_numOfContentCapSets	= other.m_numOfContentCapSets;
		for(int i = 0; i < m_numOfContentCapSets; i++)
		{
			curLen = other.m_contentCapList[i]->capLength;
			m_contentCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_contentCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_contentCapList[i]->capLength		= curLen;
			m_contentCapList[i]->capTypeCode	= other.m_contentCapList[i]->capTypeCode;
			m_contentCapList[i]->sipPayloadType	= other.m_contentCapList[i]->sipPayloadType;
			memcpy(m_contentCapList[i]->dataCap,other.m_contentCapList[i]->dataCap,curLen);
		}

		//bfcp
		m_numOfBfcpCapSets	= other.m_numOfBfcpCapSets;
		for(int i = 0; i < m_numOfBfcpCapSets; i++)
		{
			curLen = other.m_bfcpCapList[i]->capLength;
			m_bfcpCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_bfcpCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_bfcpCapList[i]->capLength		= curLen;
			m_bfcpCapList[i]->capTypeCode		= other.m_bfcpCapList[i]->capTypeCode;
			m_bfcpCapList[i]->sipPayloadType	= other.m_bfcpCapList[i]->sipPayloadType;
			memcpy(m_bfcpCapList[i]->dataCap,other.m_bfcpCapList[i]->dataCap,curLen);
		}
		m_h263_4CifMpi = other.m_h263_4CifMpi;
		m_videoQuality = other.m_videoQuality;
		m_bIsLpr       = other.m_bIsLpr;

		//LYNC2013_FEC_RED:
		m_bIsFec       = other.m_bIsFec;
		m_bIsRed       = other.m_bIsRed;

		//SDES audio
		m_numOfAudioSdesCapSets	= other.m_numOfAudioSdesCapSets;
		for(int i = 0; i < m_numOfAudioSdesCapSets; i++)
		{
			curLen = other.m_audioSdesCapList[i]->capLength;
			m_audioSdesCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_audioSdesCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_audioSdesCapList[i]->capLength		= curLen;
			m_audioSdesCapList[i]->capTypeCode		= other.m_audioSdesCapList[i]->capTypeCode;
			m_audioSdesCapList[i]->sipPayloadType	= other.m_audioSdesCapList[i]->sipPayloadType;
			memcpy(m_audioSdesCapList[i]->dataCap,other.m_audioSdesCapList[i]->dataCap,curLen);
		}

		//SDES video
		m_numOfVideoSdesCapSets	= other.m_numOfVideoSdesCapSets;
		for(int i = 0; i < m_numOfVideoSdesCapSets; i++)
		{
			curLen = other.m_videoSdesCapList[i]->capLength;
			m_videoSdesCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_videoSdesCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_videoSdesCapList[i]->capLength		= curLen;
			m_videoSdesCapList[i]->capTypeCode		= other.m_videoSdesCapList[i]->capTypeCode;
			m_videoSdesCapList[i]->sipPayloadType	= other.m_videoSdesCapList[i]->sipPayloadType;
			memcpy(m_videoSdesCapList[i]->dataCap,other.m_videoSdesCapList[i]->dataCap,curLen);
		}

		//SDES data
		m_numOfFeccSdesCapSets	= other.m_numOfFeccSdesCapSets;
		for(int i = 0; i < m_numOfFeccSdesCapSets; i++)
		{
			curLen = other.m_feccSdesCapList[i]->capLength;
			m_feccSdesCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_feccSdesCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_feccSdesCapList[i]->capLength		= curLen;
			m_feccSdesCapList[i]->capTypeCode		= other.m_feccSdesCapList[i]->capTypeCode;
			m_feccSdesCapList[i]->sipPayloadType	= other.m_feccSdesCapList[i]->sipPayloadType;
			memcpy(m_feccSdesCapList[i]->dataCap,other.m_feccSdesCapList[i]->dataCap,curLen);
		}

		//SDES content
		m_numOfContentSdesCapSets	= other.m_numOfContentSdesCapSets;
		for(int i = 0; i < m_numOfContentSdesCapSets; i++)
		{
			curLen = other.m_contentSdesCapList[i]->capLength;
			m_contentSdesCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_contentSdesCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_contentSdesCapList[i]->capLength		= curLen;
			m_contentSdesCapList[i]->capTypeCode		= other.m_contentSdesCapList[i]->capTypeCode;
			m_contentSdesCapList[i]->sipPayloadType	= other.m_contentSdesCapList[i]->sipPayloadType;
			memcpy(m_contentSdesCapList[i]->dataCap,other.m_contentSdesCapList[i]->dataCap,curLen);
		}

		m_numOfIceAudioCaps = other.m_numOfIceAudioCaps;

		if (::isApiTaNull(const_cast<mcTransportAddress*>(&other.m_AudioHostPartyAddr)) == FALSE)
		{
			memset(&m_AudioHostPartyAddr,0,sizeof(mcTransportAddress));
			memcpy(&(m_AudioHostPartyAddr),&(other.m_AudioHostPartyAddr), sizeof(mcTransportAddress));
		}

		for(int i = 0; i < m_numOfIceAudioCaps; i++)
		{
			curLen = other.m_IceAudioCapList[i]->capLength;
			m_IceAudioCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_IceAudioCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_IceAudioCapList[i]->capLength		= curLen;
			m_IceAudioCapList[i]->capTypeCode		= other.m_IceAudioCapList[i]->capTypeCode;
			m_IceAudioCapList[i]->sipPayloadType	= other.m_IceAudioCapList[i]->sipPayloadType;
			memcpy(m_IceAudioCapList[i]->dataCap,other.m_IceAudioCapList[i]->dataCap,curLen);
		}

		m_numOfIceVideoCaps = other.m_numOfIceVideoCaps;

		if (::isApiTaNull(const_cast<mcTransportAddress*>(&other.m_VideoHostPartyAddr)) == FALSE)
		{
			memset(&m_VideoHostPartyAddr,0,sizeof(mcTransportAddress));
			memcpy(&(m_VideoHostPartyAddr),&(other.m_VideoHostPartyAddr), sizeof(mcTransportAddress));
		}

		for(int i = 0; i < m_numOfIceVideoCaps; i++)
		{
			curLen = other.m_IceVideoCapList[i]->capLength;
			m_IceVideoCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_IceVideoCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_IceVideoCapList[i]->capLength		= curLen;
			m_IceVideoCapList[i]->capTypeCode		= other.m_IceVideoCapList[i]->capTypeCode;
			m_IceVideoCapList[i]->sipPayloadType	= other.m_IceVideoCapList[i]->sipPayloadType;
			memcpy(m_IceVideoCapList[i]->dataCap,other.m_IceVideoCapList[i]->dataCap,curLen);
		}

		m_numOfIceDataCaps = other.m_numOfIceDataCaps;

		if (::isApiTaNull(const_cast<mcTransportAddress*>(&other.m_DataHostPartyAddr)) == FALSE)
		{
			memset(&m_DataHostPartyAddr,0,sizeof(mcTransportAddress));
			memcpy(&(m_DataHostPartyAddr),&(other.m_DataHostPartyAddr), sizeof(mcTransportAddress));
		}


		for(int i = 0; i < m_numOfIceDataCaps; i++)
		{
			curLen = other.m_IceDataCapList[i]->capLength;
			m_IceDataCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_IceDataCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_IceDataCapList[i]->capLength		= curLen;
			m_IceDataCapList[i]->capTypeCode		= other.m_IceDataCapList[i]->capTypeCode;
			m_IceDataCapList[i]->sipPayloadType	= other.m_IceDataCapList[i]->sipPayloadType;
			memcpy(m_IceDataCapList[i]->dataCap,other.m_IceDataCapList[i]->dataCap,curLen);
		}
		m_numOfIceGeneralCaps = other.m_numOfIceGeneralCaps;
		for(int i = 0; i < m_numOfIceGeneralCaps; i++)
		{
			curLen = other.m_IceGeneralCapList[i]->capLength;
			m_IceGeneralCapList[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
			memset(m_IceGeneralCapList[i], 0, sizeof(capBufferBase) + curLen);
			m_IceGeneralCapList[i]->capLength		= curLen;
			m_IceGeneralCapList[i]->capTypeCode		= other.m_IceGeneralCapList[i]->capTypeCode;
			m_IceGeneralCapList[i]->sipPayloadType	= other.m_IceGeneralCapList[i]->sipPayloadType;
			memcpy(m_IceGeneralCapList[i]->dataCap,other.m_IceGeneralCapList[i]->dataCap,curLen);
		}

		m_TipAuxFPS = other.m_TipAuxFPS;
		m_sessionLevelRate = other.m_sessionLevelRate;
		m_msftVideoRxBw    = other.m_msftVideoRxBw;
		
		m_msftSsrcAudio = other.m_msftSsrcAudio;
		for (int i=0; i<MaxMsftSvcSdpVideoMlines; i++)
		{
			m_msftSsrcVideo[i][0]= other.m_msftSsrcVideo[i][0];
			m_msftSsrcVideo[i][1]= other.m_msftSsrcVideo[i][1];
		}
		m_msftMsiAudio = other.m_msftMsiAudio;
		for(int i=0; i<MaxMsftSvcSdpVideoMlines; i++)
		{
			m_msftMsiVideo[i] = other.m_msftMsiVideo[i];
		}
		m_encryptionKeyToUse=other.m_encryptionKeyToUse;
		m_bUseNonMkiOrderFirst=other.m_bUseNonMkiOrderFirst; //BRIDGE-11708
		//avmcu sdes
		m_numOfMsftAVMCUSdesCaps = other.m_numOfMsftAVMCUSdesCaps;
		for(DWORD i=0; i < m_numOfMsftAVMCUSdesCaps; i++)
		{
			if(other.m_msftAVMCUSdesCaps[i])
			{
				curLen = other.m_msftAVMCUSdesCaps[i]->capLength;
				m_msftAVMCUSdesCaps[i] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + curLen]);
				memset(m_msftAVMCUSdesCaps[i], 0, sizeof(capBufferBase) + curLen);
				m_msftAVMCUSdesCaps[i]->capLength		= curLen;
				m_msftAVMCUSdesCaps[i]->capTypeCode		= other.m_msftAVMCUSdesCaps[i]->capTypeCode;
				m_msftAVMCUSdesCaps[i]->sipPayloadType	= other.m_msftAVMCUSdesCaps[i]->sipPayloadType;
				memcpy(m_msftAVMCUSdesCaps[i]->dataCap,other.m_msftAVMCUSdesCaps[i]->dataCap,curLen);
			}
		}

	}
	return *this;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddCapSet(cmCapDataType eMediaType, const capBuffer* pCapSet)
{
	if (eMediaType == cmCapAudio)
	{
		m_audioCapList[m_numOfAudioCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_audioCapList[m_numOfAudioCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_audioCapList[m_numOfAudioCapSets]->capLength		= pCapSet->capLength;
		m_audioCapList[m_numOfAudioCapSets]->capTypeCode    = pCapSet->capTypeCode;
		m_audioCapList[m_numOfAudioCapSets]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_audioCapList[m_numOfAudioCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
		m_numOfAudioCapSets++;
	}
	else if (eMediaType == cmCapVideo)
	{
		ERoleLabel eRole = (ERoleLabel)((BaseCapStruct*)pCapSet->dataCap)->header.roleLabel;
		if (eRole == kRolePeople)
		{
			m_videoCapList[m_numOfVideoCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
			memset(m_videoCapList[m_numOfVideoCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
			m_videoCapList[m_numOfVideoCapSets]->capLength		= pCapSet->capLength;
			m_videoCapList[m_numOfVideoCapSets]->capTypeCode	= pCapSet->capTypeCode;
			m_videoCapList[m_numOfVideoCapSets]->sipPayloadType = pCapSet->sipPayloadType;

			memcpy(m_videoCapList[m_numOfVideoCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);

			m_numOfVideoCapSets++;
		}
		else
		{
			m_contentCapList[m_numOfContentCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
			memset(m_contentCapList[m_numOfContentCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
			m_contentCapList[m_numOfContentCapSets]->capLength		= pCapSet->capLength;
			m_contentCapList[m_numOfContentCapSets]->capTypeCode	= pCapSet->capTypeCode;
			m_contentCapList[m_numOfContentCapSets]->sipPayloadType = pCapSet->sipPayloadType;
			memcpy(m_contentCapList[m_numOfContentCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
			m_numOfContentCapSets++;
		}
	}
	else if (eMediaType == cmCapData)
	{
		m_feccCapList[m_numOfFeccCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_feccCapList[m_numOfFeccCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_feccCapList[m_numOfFeccCapSets]->capLength		= pCapSet->capLength;
		m_feccCapList[m_numOfFeccCapSets]->capTypeCode	= pCapSet->capTypeCode;
		m_feccCapList[m_numOfFeccCapSets]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_feccCapList[m_numOfFeccCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
		m_numOfFeccCapSets++;
	}
	else if (eMediaType == cmCapBfcp)
	{
		m_bfcpCapList[m_numOfBfcpCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_bfcpCapList[m_numOfBfcpCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_bfcpCapList[m_numOfBfcpCapSets]->capLength		= pCapSet->capLength;
		m_bfcpCapList[m_numOfBfcpCapSets]->capTypeCode		= pCapSet->capTypeCode;
		m_bfcpCapList[m_numOfBfcpCapSets]->sipPayloadType 	= pCapSet->sipPayloadType;
		memcpy(m_bfcpCapList[m_numOfBfcpCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
		m_numOfBfcpCapSets++;
	}
	else
	{
		FTRACESTR(eLevelInfoNormal) << "CSipCaps::AddCapSet: unsupported media type - " << eMediaType;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddCapSet(cmCapDataType eMediaType, const CBaseCap* pCap)
{
	capBuffer* pBuffer = pCap->GetAsCapBuffer();

	if (pBuffer)
	{
		AddCapSet(eMediaType, pBuffer);
		PDELETEA(pBuffer);
	}
	else
	{
		PTRACE(eLevelError,"CSipCaps::AddCapSet: Error Couldn't get GetAsCapBuffer  ");
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddSdesCapSet(cmCapDataType eMediaType, const capBuffer* pCapSet, ERoleLabel eRole)
{
	if (eMediaType == cmCapAudio)
	{
		m_audioSdesCapList[m_numOfAudioSdesCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_audioSdesCapList[m_numOfAudioSdesCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_audioSdesCapList[m_numOfAudioSdesCapSets]->capLength		= pCapSet->capLength;
		m_audioSdesCapList[m_numOfAudioSdesCapSets]->capTypeCode    = pCapSet->capTypeCode;
		m_audioSdesCapList[m_numOfAudioSdesCapSets]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_audioSdesCapList[m_numOfAudioSdesCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
		m_numOfAudioSdesCapSets++;
	}
	else if ((eMediaType == cmCapVideo) && (eRole == kRolePeople))
	{
		m_videoSdesCapList[m_numOfVideoSdesCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_videoSdesCapList[m_numOfVideoSdesCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_videoSdesCapList[m_numOfVideoSdesCapSets]->capLength		= pCapSet->capLength;
		m_videoSdesCapList[m_numOfVideoSdesCapSets]->capTypeCode	= pCapSet->capTypeCode;
		m_videoSdesCapList[m_numOfVideoSdesCapSets]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_videoSdesCapList[m_numOfVideoSdesCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
		m_numOfVideoSdesCapSets++;
	}
	else if (eMediaType == cmCapData)
	{
		m_feccSdesCapList[m_numOfFeccSdesCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_feccSdesCapList[m_numOfFeccSdesCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_feccSdesCapList[m_numOfFeccSdesCapSets]->capLength		= pCapSet->capLength;
		m_feccSdesCapList[m_numOfFeccSdesCapSets]->capTypeCode	= pCapSet->capTypeCode;
		m_feccSdesCapList[m_numOfFeccSdesCapSets]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_feccSdesCapList[m_numOfFeccSdesCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
		m_numOfFeccSdesCapSets++;
	}
	else if ((eMediaType == cmCapVideo) && (eRole == kRolePresentation))
	{
		m_contentSdesCapList[m_numOfContentSdesCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_contentSdesCapList[m_numOfContentSdesCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_contentSdesCapList[m_numOfContentSdesCapSets]->capLength		= pCapSet->capLength;
		m_contentSdesCapList[m_numOfContentSdesCapSets]->capTypeCode	= pCapSet->capTypeCode;
		m_contentSdesCapList[m_numOfContentSdesCapSets]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_contentSdesCapList[m_numOfContentSdesCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
		m_numOfContentSdesCapSets++;
	}
	else
	{
		FTRACESTR(eLevelInfoNormal) << "CSipCaps::AddSdesCapSet: unsupported media type - " << eMediaType;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddDtlsCapSet(cmCapDataType eMediaType, const capBuffer* pCapSet, ERoleLabel eRole)
{
	if (eMediaType == cmCapAudio)
	{
		m_audioDtlsCapList[m_numOfAudioDtlsCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_audioDtlsCapList[m_numOfAudioDtlsCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_audioDtlsCapList[m_numOfAudioDtlsCapSets]->capLength		= pCapSet->capLength;
		m_audioDtlsCapList[m_numOfAudioDtlsCapSets]->capTypeCode    = pCapSet->capTypeCode;
		m_audioDtlsCapList[m_numOfAudioDtlsCapSets]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_audioDtlsCapList[m_numOfAudioDtlsCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
		m_numOfAudioDtlsCapSets++;
	}
	else if ((eMediaType == cmCapVideo) && (eRole == kRolePeople))
	{
		m_videoDtlsCapList[m_numOfVideoDtlsCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_videoDtlsCapList[m_numOfVideoDtlsCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_videoDtlsCapList[m_numOfVideoDtlsCapSets]->capLength		= pCapSet->capLength;
		m_videoDtlsCapList[m_numOfVideoDtlsCapSets]->capTypeCode	= pCapSet->capTypeCode;
		m_videoDtlsCapList[m_numOfVideoDtlsCapSets]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_videoDtlsCapList[m_numOfVideoDtlsCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
		m_numOfVideoDtlsCapSets++;
	}
	else if (eMediaType == cmCapData)
	{
		m_feccDtlsCapList[m_numOfFeccDtlsCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_feccDtlsCapList[m_numOfFeccDtlsCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_feccDtlsCapList[m_numOfFeccDtlsCapSets]->capLength		= pCapSet->capLength;
		m_feccDtlsCapList[m_numOfFeccDtlsCapSets]->capTypeCode	= pCapSet->capTypeCode;
		m_feccDtlsCapList[m_numOfFeccDtlsCapSets]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_feccDtlsCapList[m_numOfFeccDtlsCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
		m_numOfFeccDtlsCapSets++;
	}
	else if ((eMediaType == cmCapVideo) && (eRole == kRolePresentation))
	{
		m_contentDtlsCapList[m_numOfContentDtlsCapSets] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_contentDtlsCapList[m_numOfContentDtlsCapSets], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_contentDtlsCapList[m_numOfContentDtlsCapSets]->capLength		= pCapSet->capLength;
		m_contentDtlsCapList[m_numOfContentDtlsCapSets]->capTypeCode	= pCapSet->capTypeCode;
		m_contentDtlsCapList[m_numOfContentDtlsCapSets]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_contentDtlsCapList[m_numOfContentDtlsCapSets]->dataCap, pCapSet->dataCap,pCapSet->capLength);
		m_numOfContentDtlsCapSets++;
	}
	else
	{
		FTRACESTR(eLevelInfoNormal) << "CSipCaps::AddDtlsCapSet: unsupported media type - " << eMediaType;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddSdesCapSet(cmCapDataType eMediaType, const CBaseCap* pCap, ERoleLabel eRole)
{
	capBuffer* pBuffer = pCap->GetAsCapBuffer();

	if (pBuffer)
	{
		AddSdesCapSet(eMediaType, pBuffer, eRole);
		PDELETEA(pBuffer);
	}
	else
	{
		PTRACE(eLevelError,"CSipCaps::AddSdesCapSet: Error Couldn't get GetAsCapBuffer  ");
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddDtlsCapSet(cmCapDataType eMediaType, const CBaseCap* pCap, ERoleLabel eRole)
{
	capBuffer* pBuffer = pCap->GetAsCapBuffer();

	if (pBuffer)
	{
		AddDtlsCapSet(eMediaType, pBuffer, eRole);
		PDELETEA(pBuffer);
	}
	else
	{
		PTRACE(eLevelError,"CSipCaps::AddDtlsCapSet: Error Couldn't get GetAsCapBuffer  ");
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CSdesCap* CSipCaps::GetSdesCap(cmCapDataType eMediaType, ERoleLabel eRole , int arrIndex) const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	CSdesCap* pSdesCap = NULL;

	GetSdesMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (pMediaCapList && arrIndex < numOfMediaCapSet)
	{
		//for (int i = 0; i < numOfMediaCapSet; i++)
		//{
		//	pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[i]->dataCap);
		//}
		pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[arrIndex]->dataCap);
	}

	return pSdesCap;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CSdesCap* CSipCaps::GetVideoSdesCapAVMCU(DWORD mLine) const
{
	CSdesCap* pSdesCap = NULL;

	if(mLine < m_numOfMsftAVMCUSdesCaps && m_msftAVMCUSdesCaps[mLine])
	{
		pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,m_msftAVMCUSdesCaps[mLine]->dataCap);
	}

	return pSdesCap;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
CDtlsCap* CSipCaps::GetDtlsCap(cmCapDataType eMediaType, ERoleLabel eRole, int arrIndex) const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	CDtlsCap* pDtlsCap = NULL;

	GetDtlsMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (pMediaCapList && arrIndex < numOfMediaCapSet)
	{
		pDtlsCap = (CDtlsCap*)CBaseCap::AllocNewCap((CapEnum)eDtlsCapCode,pMediaCapList[arrIndex]->dataCap);
	}

	return pDtlsCap;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//void CSipCaps::UpdateSingleSdesCapSet(cmCapDataType eMediaType, const CBaseCap* pCap)
//{
//	capBuffer* pCapSet = pCap->GetAsCapBuffer();
//	if (eMediaType == cmCapAudio)
//	{
//		if(m_numOfAudioSdesCapSets < 1)
//			m_numOfAudioSdesCapSets = 1;
//		if(m_audioSdesCapList[0])
//			POBJDELETE(m_audioSdesCapList[0]);
//
//		m_audioSdesCapList[0] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
//		memset(m_audioSdesCapList[0], 0, sizeof(capBufferBase) + pCapSet->capLength);
//		m_audioSdesCapList[0]->capLength		= pCapSet->capLength;
//		m_audioSdesCapList[0]->capTypeCode    = pCapSet->capTypeCode;
//		m_audioSdesCapList[0]->sipPayloadType = pCapSet->sipPayloadType;
//		memcpy(m_audioSdesCapList[0]->dataCap, pCapSet->dataCap,pCapSet->capLength);
//	}
//	else if (eMediaType == cmCapVideo)
//	{
//		if(m_numOfVideoSdesCapSets < 1)
//			m_numOfVideoSdesCapSets = 1;
//		if(m_videoSdesCapList[0])
//			POBJDELETE(m_videoSdesCapList[0]);
//
//		m_videoSdesCapList[0] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
//		memset(m_videoSdesCapList[0], 0, sizeof(capBufferBase) + pCapSet->capLength);
//		m_videoSdesCapList[0]->capLength		= pCapSet->capLength;
//		m_videoSdesCapList[0]->capTypeCode	= pCapSet->capTypeCode;
//		m_videoSdesCapList[0]->sipPayloadType = pCapSet->sipPayloadType;
//		memcpy(m_videoSdesCapList[0]->dataCap, pCapSet->dataCap,pCapSet->capLength);
//	}
//	else if (eMediaType == cmCapData)
//	{
//		if(m_numOfFeccSdesCapSets < 1)
//			m_numOfFeccSdesCapSets = 1;
//		if(m_feccSdesCapList[0])
//			POBJDELETE(m_feccSdesCapList[0]);
//
//		m_feccSdesCapList[0] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
//		memset(m_feccSdesCapList[0], 0, sizeof(capBufferBase) + pCapSet->capLength);
//		m_feccSdesCapList[0]->capLength		= pCapSet->capLength;
//		m_feccSdesCapList[0]->capTypeCode	= pCapSet->capTypeCode;
//		m_feccSdesCapList[0]->sipPayloadType = pCapSet->sipPayloadType;
//		memcpy(m_feccSdesCapList[0]->dataCap, pCapSet->dataCap,pCapSet->capLength);
//	}
//	else
//	{
//		FTRACESTR(eLevelInfoNormal) << "CSipCaps::UpdateSdesCapSet: unsupported media type - " << eMediaType;
//	}
//}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::RemoveCapSet(const CCapSetInfo& capInfo, ERoleLabel eRole)
{
	capBuffer* pRemoved = NULL;
	cmCapDataType eMediaType = capInfo.GetSipCapType();
	int originalNumOfCaps	= GetNumOfCapSets(); // to prevent a loop
	int index = GetIndexInArr(capInfo,0,eRole);

	for (int i=0; i<originalNumOfCaps && index != NA ;i++)
	{
		pRemoved = RemoveCapSet(eMediaType, index,eRole);
		PDELETEA(pRemoved);
		// next cap
		index	= GetIndexInArr(capInfo,0,eRole);
	}
}
/////////////////////////////////////////////////////////////////////////////////
void CSipCaps::RemoveH264SpecifProfileCapSet(const CCapSetInfo& capInfo, ERoleLabel eRole,APIU16 profile)
{
	capBuffer* pRemoved = NULL;
	cmCapDataType eMediaType = capInfo.GetSipCapType();
	int originalNumOfCaps	= GetNumOfCapSets(); // to prevent a loop
	int index = GetIndexInArrAccordingToCapSetAndProfile(capInfo,0,eRole,profile);

	for (int i=0; i<originalNumOfCaps && index != NA ;i++)
	{
		pRemoved = RemoveCapSet(eMediaType, index,eRole);
		PDELETEA(pRemoved);
		// next cap
		index	= GetIndexInArrAccordingToCapSetAndProfile(capInfo,0,eRole,profile);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
capBuffer* CSipCaps::RemoveCapSet(cmCapDataType eMediaType, int arrIndex, ERoleLabel eRole)
{
	capBuffer* pRemoved = NULL;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;

	TRACEINTO << "CSipCaps::RemoveCapSet requested for Media Type : " << (int)eMediaType;
	
	GetMediaCaps(eMediaType,&numOfMediaCapSet,&pMediaCapList,eRole);

	if ( arrIndex <= numOfMediaCapSet-1 )
	{
		pRemoved = pMediaCapList[arrIndex];

		// if it is not the last we have to copy each pointer one place back
		if (arrIndex < numOfMediaCapSet-1)
		{
			for(int i = arrIndex; i < numOfMediaCapSet - 1; i++)
			{
				pMediaCapList[i] = pMediaCapList[i+1];
			}
		}

		pMediaCapList[numOfMediaCapSet-1] = NULL; // the last is empty now
		SetNumOfCapSets(numOfMediaCapSet-1, eMediaType,eRole);
	}

	return pRemoved;
}
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
capBuffer* CSipCaps::RemoveCapSetForSdes(cmCapDataType eMediaType, int arrIndex, ERoleLabel eRole)
{
	capBuffer* pRemoved = NULL;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	TRACEINTO << "CSipCaps::RemoveCapSetForSdes requested for Media Type : " << (int)eMediaType;
	
	GetSdesMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
	if ( arrIndex <= numOfMediaCapSet-1 )
	{
		pRemoved = pMediaCapList[arrIndex];
		// if it is not the last we have to copy each pointer one place back
		if (arrIndex < numOfMediaCapSet-1)
		{
			for(int i = arrIndex; i < numOfMediaCapSet - 1; i++)
			{
				pMediaCapList[i] = pMediaCapList[i+1];
			}
		}

		pMediaCapList[numOfMediaCapSet-1] = NULL; // the last is empty now
		SetNumOfSdesCapSets(numOfMediaCapSet-1, eMediaType,eRole);
	}

	return pRemoved;
}
/////////////////////////////////////////////////////////////////////////////
//TRUE - valid payload type
//FALSE - invalid payload type
WORD CSipCaps::CheckValidPayloadType(APIU8 nPayloadType)
{
	switch(nPayloadType)
	{
		case _PCMU:
		case _G7231:
		case _PCMA:
		case _G722:
		case _G728:
		case _G729:
		case _H261:
		case _H263:
 			return TRUE;
		default:
			if((__FIRST_DPT <= nPayloadType) && (nPayloadType <= __LAST_DPT))
				return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::CheckAndRemoveCapSetsWithInvalidPayloadType(BYTE &bRemovedAudio, BYTE &bRemovedVideo)
{
	int numOfMediaCapSet = 0;
	int originalNumOfCaps = GetNumOfCapSets(); // to prevent a loop
	capBuffer** pMediaCapList	= NULL;
	capBuffer* pRemoved = NULL;
	cmCapDataType mediaType;
	ERoleLabel eRole;

	for(int i = 0; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		GetMediaCaps(mediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

		if (mediaType == cmCapBfcp)
			continue;

		for (int j = 0; j < numOfMediaCapSet;)
		{
			if ( !CheckValidPayloadType(pMediaCapList[j]->sipPayloadType) && pMediaCapList[j]->capTypeCode != eLPRCapCode)
			{
				pRemoved = RemoveCapSet(mediaType,j,eRole);
				int newNumOfCapSets = GetNumOfMediaCapSets(mediaType,cmCapReceiveAndTransmit,eRole);

				if ( pRemoved && (newNumOfCapSets < numOfMediaCapSet) )
				{
					CCapSetInfo capInfo = (CapEnum)pRemoved->capTypeCode;
					ERoleLabel eRemovedRole = (ERoleLabel)((BaseCapStruct*)pRemoved->dataCap)->header.roleLabel;
					DBGPASSERT(pRemoved->capTypeCode? pRemoved->capTypeCode: NA);
					PTRACE2(eLevelError, "Removed capability with invalid payload type ", (const char*) capInfo);
					PDELETEA(pRemoved);

					if(capInfo.IsType(cmCapAudio))
						bRemovedAudio++;
					if(capInfo.IsType(cmCapVideo) && eRemovedRole == kRolePeople)
						bRemovedVideo++;

					numOfMediaCapSet = newNumOfCapSets;
				}
				else
				{
					DBGPASSERT(YES);
					j++;
				}
			}
			else
				j++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::GetMediaCaps(cmCapDataType eMediaType, int* pNumOfCaps, capBuffer*** ppMediaCapList, ERoleLabel eRole) const
{
	if(eMediaType == cmCapAudio)
	{
		*pNumOfCaps = m_numOfAudioCapSets;
		*ppMediaCapList	 = (capBuffer**)m_audioCapList;
	}
	else if (eMediaType == cmCapVideo)
	{
		if (eRole == kRolePeople)
		{
			*pNumOfCaps = m_numOfVideoCapSets;
			*ppMediaCapList	 = (capBuffer**)m_videoCapList;
		}
		else
		{
			*pNumOfCaps = m_numOfContentCapSets;
			*ppMediaCapList	 = (capBuffer**)m_contentCapList;
		}
	}
	else if (eMediaType == cmCapData)
	{
		*pNumOfCaps = m_numOfFeccCapSets;
		*ppMediaCapList	 = (capBuffer**)m_feccCapList;
	}
	else if (eMediaType == cmCapBfcp)
	{
		*pNumOfCaps = m_numOfBfcpCapSets;
		*ppMediaCapList	 = (capBuffer**)m_bfcpCapList;
	}
	else
	{
		*pNumOfCaps = 0;
	}
}


/*
 * Find the sirenlpr Cap in the media caps.
 */
CapEnum CSipCaps::FindSirenLPRInCaps(cmCapDataType eMediaType, ERoleLabel eRole) const
{
	CapEnum reVal				= eUnknownAlgorithemCapCode;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if (((CapEnum)pMediaCapList[i]->capTypeCode <= eSirenLPRStereo_128kCapCode) && ((CapEnum)pMediaCapList[i]->capTypeCode >= eSirenLPR_32kCapCode))
			{
				reVal = (CapEnum)pMediaCapList[i]->capTypeCode;
				break;
			}
		}
	}
	return reVal;
}

/*
 * Update "local" Cap code with "remote" capcode according to payload type
 * */
void CSipCaps::ReplaceCapWithOtherCapByPayload(CapEnum otherCap, WORD payload)
{
		if (m_audioCapList)
		{
			for (int i = 0; i < m_numOfAudioCapSets; i++)
			{
				if (m_audioCapList[i]->sipPayloadType == payload)
				{
					m_audioCapList[i]->capTypeCode = otherCap;
					break;
				}
			}
		}
}

////////////////////////////////////////////////////////////////////////
void CSipCaps::AddDSHForAvMcu()
{

	if (m_audioCapList)
	{
		for (int i = 0; i < m_numOfAudioCapSets; i++)
		{
			CBaseAudioCap* pAudioCap = (CBaseAudioCap*) GetCapSet(cmCapAudio, i);
			if(pAudioCap)
				pAudioCap->SetRtcpMask(RTCP_MASK_MS_DSH);
			POBJDELETE(pAudioCap);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::GetSdesMediaCaps(cmCapDataType eMediaType, int* pNumOfCaps, capBuffer*** ppMediaCapList, ERoleLabel eRole) const
{
	if(eMediaType == cmCapAudio)
	{
		*pNumOfCaps = m_numOfAudioSdesCapSets;
		*ppMediaCapList	 = (capBuffer**)m_audioSdesCapList;
	}
	else if ((eMediaType == cmCapVideo) && (eRole == kRolePeople))
	{
		*pNumOfCaps = m_numOfVideoSdesCapSets;
		*ppMediaCapList	 = (capBuffer**)m_videoSdesCapList;
	}
	else if (eMediaType == cmCapData)
	{
		*pNumOfCaps = m_numOfFeccSdesCapSets;
		*ppMediaCapList	 = (capBuffer**)m_feccSdesCapList;
	}
	else if ((eMediaType == cmCapVideo) && (eRole == kRolePresentation))
	{
		*pNumOfCaps = m_numOfContentSdesCapSets;
		*ppMediaCapList	 = (capBuffer**)m_contentSdesCapList;
	}
	else
	{
		*pNumOfCaps = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::GetDtlsMediaCaps(cmCapDataType eMediaType, int* pNumOfCaps, capBuffer*** ppMediaCapList, ERoleLabel eRole) const
{
	if(eMediaType == cmCapAudio)
	{
		*pNumOfCaps = m_numOfAudioDtlsCapSets;
		*ppMediaCapList	 = (capBuffer**)m_audioDtlsCapList;
	}
	else if ((eMediaType == cmCapVideo) && (eRole == kRolePeople))
	{
		*pNumOfCaps = m_numOfVideoDtlsCapSets;
		*ppMediaCapList	 = (capBuffer**)m_videoDtlsCapList;
	}
	else if (eMediaType == cmCapData)
	{
		*pNumOfCaps = m_numOfFeccDtlsCapSets;
		*ppMediaCapList	 = (capBuffer**)m_feccDtlsCapList;
	}
	else if ((eMediaType == cmCapVideo) && (eRole == kRolePresentation))
	{
		*pNumOfCaps = m_numOfContentDtlsCapSets;
		*ppMediaCapList	 = (capBuffer**)m_contentDtlsCapList;
	}
	else
	{
		*pNumOfCaps = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::GetMediaIceCaps(ICESessionsTypes SessionType, int* pNumOfIceCaps, capBuffer*** ppMediaIceCapList) const
{
	if(SessionType == eGeneralSession)
	{
		if(m_numOfIceGeneralCaps)
		{
			*pNumOfIceCaps = m_numOfIceGeneralCaps;
			*ppMediaIceCapList	 = (capBuffer**)m_IceGeneralCapList;
		}
	}
	else if(SessionType == eAudioSession)
	{
		if(m_numOfIceAudioCaps)
		{
			*pNumOfIceCaps = m_numOfIceAudioCaps;
			*ppMediaIceCapList	 = (capBuffer**)m_IceAudioCapList;
		}
	}
	else if (SessionType == eVideoSession)
	{
		if(m_numOfIceVideoCaps)
		{
			*pNumOfIceCaps = m_numOfIceVideoCaps;
			*ppMediaIceCapList	 = (capBuffer**)m_IceVideoCapList;
		}
	}
	else if (SessionType == eDataSession)
	{
		if(m_numOfIceDataCaps)
		{
			*pNumOfIceCaps = m_numOfIceDataCaps;
			*ppMediaIceCapList	 = (capBuffer**)m_IceDataCapList;
		}
	}
	else
	{
		*pNumOfIceCaps = 0;
		FTRACESTR(eLevelInfoNormal) << "CSipCaps::GetMediaIceCaps: unsupported Session type - " << SessionType;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
int CSipCaps::GetNumOfMediaIceCapSets(ICESessionsTypes SessionType) const
{
	int res		  = 0;
	int numOfCaps = 0;

	if (SessionType == eAudioSession)
		numOfCaps = m_numOfIceAudioCaps;
	else if (SessionType==eVideoSession)
		numOfCaps = m_numOfIceVideoCaps;
	else if (SessionType==eDataSession)
		numOfCaps = m_numOfIceDataCaps;
	else if (SessionType==eGeneralSession)
		numOfCaps = m_numOfIceGeneralCaps;

	return numOfCaps;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int CSipCaps::GetNumOfMediaCapSets(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole) const
{
	int res		  = 0;
	int numOfCaps = 0;

	if (eMediaType==cmCapAudio)
		numOfCaps = m_numOfAudioCapSets;
	else if (eMediaType==cmCapVideo)
	{
		if (eRole == kRolePeople)
			numOfCaps = m_numOfVideoCapSets;
		else
			numOfCaps = m_numOfContentCapSets;
	}
	else if (eMediaType==cmCapData)
		numOfCaps = m_numOfFeccCapSets;
	else if (eMediaType==cmCapBfcp)
			numOfCaps = m_numOfBfcpCapSets;
	if (eDirection != cmCapReceiveAndTransmit)
	{
		CBaseCap* pCap = NULL;
		for (int i=0; i<numOfCaps; i++)
		{
			pCap = GetCapSet(eMediaType,i,eRole);
			if (pCap && pCap->IsDirection(eDirection))
				res++;
			POBJDELETE(pCap);
		}
	}
	else
		res = numOfCaps;

	return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int CSipCaps::GetNumOfSdesMediaCapSets(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole) const
{
	int res		  = 0;
	int numOfCaps = 0;

	if (eMediaType==cmCapAudio)
		numOfCaps = m_numOfAudioSdesCapSets;
	else if (eMediaType==cmCapVideo)
	{
		if (eRole == kRolePeople)
			numOfCaps = m_numOfVideoSdesCapSets;
		else
			numOfCaps = m_numOfContentSdesCapSets;
	}
	else if (eMediaType==cmCapData)
		numOfCaps = m_numOfFeccSdesCapSets;

	if (eDirection != cmCapReceiveAndTransmit)
	{
		CBaseCap* pCap = NULL;
		for (int i=0; i<numOfCaps; i++)
		{
			pCap = GetSdesCapSet(eMediaType,i,eRole);
			if (pCap && pCap->IsDirection(eDirection))
				res++;
			POBJDELETE(pCap);
		}
	}
	else
		res = numOfCaps;

	return res;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Gets the rate of the first cap found
DWORD CSipCaps::GetAudioDesiredRate()
{
	DWORD desiredRate 		  = 0;
	int numOfMediaCapSet	  = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapAudio, &numOfMediaCapSet, &pMediaCapList);
	if (numOfMediaCapSet)
	{
		CapEnum capCode = (CapEnum)pMediaCapList[0]->capTypeCode;
		CCapSetInfo capInfo(capCode);
		desiredRate = capInfo.GetBitRate(&pMediaCapList[0]->dataCap[0])/1000;
	}
	return desiredRate;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
CBaseCap* CSipCaps::GetIceCapSet(ICESessionsTypes SessionType,int arrIndex) const //alloc memory
{
	CBaseCap* pCap				= NULL;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaIceCaps(SessionType, &numOfMediaCapSet, &pMediaCapList);
	if (arrIndex < numOfMediaCapSet)
		pCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[arrIndex]->capTypeCode,pMediaCapList[arrIndex]->dataCap);
	return pCap;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
CBaseCap* CSipCaps::GetCapSet(cmCapDataType eMediaType,int arrIndex, ERoleLabel eRole) const //alloc memory
{
	CBaseCap* pCap				= NULL;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
	if (arrIndex < numOfMediaCapSet)
		pCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[arrIndex]->capTypeCode,pMediaCapList[arrIndex]->dataCap);
	return pCap;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
CBaseCap* CSipCaps::GetSdesCapSet(cmCapDataType eMediaType,int arrIndex, ERoleLabel eRole) const //alloc memory
{
	CBaseCap* pCap				= NULL;
	int numOfSdesMediaCapSet		= 0;
	capBuffer** pSdesMediaCapList	= NULL;
	GetSdesMediaCaps(eMediaType, &numOfSdesMediaCapSet, &pSdesMediaCapList, eRole);
	if (arrIndex < numOfSdesMediaCapSet)
		pCap = CBaseCap::AllocNewCap((CapEnum)pSdesMediaCapList[arrIndex]->capTypeCode,pSdesMediaCapList[arrIndex]->dataCap);
	return pCap;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// If index is zero gets the first cap set according to the capInfo.
// If its 1 get the next. etc.profile = 0 meannig do not compare profile..
//---------------------------------------------------------------------------
CBaseCap* CSipCaps::GetCapSet(const CCapSetInfo& capInfo, int index, ERoleLabel eRole,APIU16 eProfile) const //alloc memory
{
	CBaseCap* pCap = NULL;
	CBaseCap* pHandlCap = NULL;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	cmCapDataType eMediaType = capInfo.GetSipCapType();
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if ( index < numOfMediaCapSet )
	{
		for (int i = 0, k = index; i < numOfMediaCapSet; i++)
		{
			pHandlCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode, pMediaCapList[i]->dataCap);

			if (pHandlCap && (pHandlCap->GetCapCode() == (CapEnum)capInfo) && (pHandlCap->GetRole() == eRole))	   // && ( (pHandlCap->GetCapCode() != (CapEnum)eH264CapCode) ||(pHandlCap->GetCapCode() == (CapEnum)eH264CapCode && ((CH264VideoCap *)pHandlCap)->GetProfile() == eProfile) ) /*and*/)  /*end if*/)
			{
				BYTE matchProfile  = TRUE;
				
				if(eProfile!=0 && pHandlCap->GetCapCode() == (CapEnum)eH264CapCode && ((CH264VideoCap *)pHandlCap)->GetProfile() != eProfile)
				{
					matchProfile = FALSE;
				}
				
				POBJDELETE(pHandlCap);
				
				if (k == 0 && matchProfile)
				{
					pCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode, pMediaCapList[i]->dataCap);
					break;
				}
				else if (matchProfile)
					k--;
			}
			else
				POBJDELETE(pHandlCap);
		}
	}

	return pCap;
}



/////////////////////////////////////////////////////////////////////////
CBaseCap* CSipCaps::GetCapSetAccordingToPayload(const CCapSetInfo& capInfo,WORD payload, ERoleLabel eRole) const //alloc memory
{
	CBaseCap* pHandlCap = NULL;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	cmCapDataType eMediaType = capInfo.GetSipCapType();
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	for (int i = 0; i < numOfMediaCapSet; i++)
	{
		if ( pMediaCapList[i]->sipPayloadType == payload  )// && ( (pHandlCap->GetCapCode() != (CapEnum)eH264CapCode) ||(pHandlCap->GetCapCode() == (CapEnum)eH264CapCode && ((CH264VideoCap *)pHandlCap)->GetProfile() == eProfile) ) /*and*/)  /*end if*/)
		{
			pHandlCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode, pMediaCapList[i]->dataCap);
			if (pHandlCap && (pHandlCap->GetCapCode() != (CapEnum)capInfo))
			{
				POBJDELETE(pHandlCap);
				DBGPASSERT (4);
			}
			break;
		}
	}
	return pHandlCap;
}
/////////////////////////////////////////////////////////////////////////
void CSipCaps::SwapPayloadTypes(const CCapSetInfo& capInfo,ERoleLabel eRole, WORD payload1, WORD profile1, WORD payload2)
{
	CBaseCap* pHandlCap = NULL;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	cmCapDataType eMediaType = capInfo.GetSipCapType();
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (payload1 == payload2)
		return;

	bool isNeedSwap = false;
	int i = 0;
	for (i = 0; i < numOfMediaCapSet; i++)
	{
		if ( pMediaCapList[i]->sipPayloadType == payload1  )
		{
			isNeedSwap = true;
			break;
		}
	} // end of loop

	//nothing to do -> return
	if (!isNeedSwap)
	{
		return;
	}

	//find the other cap to swap with
	for (int j = 0; j < numOfMediaCapSet; j++)
	{
		//find the swapped cap by payload type
		if ( pMediaCapList[j]->sipPayloadType == payload2  )
		{
			pMediaCapList[j]->sipPayloadType = payload1;
		}
	}

	CSmallString str;
	str << "swapping payload " << payload1 << " with payload " << payload2 << " profile1 " << profile1;
	PTRACE2(eLevelInfoNormal,"CSipCaps::SwapPayloadTypes, ", str.GetString());

	pMediaCapList[i]->sipPayloadType = payload2;
}
//////////////////////////////////////////////////////////////////////////////////////////////
DWORD CSipCaps::GetMaxFsAccordingToProfile(APIU16 profile, ERoleLabel eRole)
{
	CBaseCap* pHandlCap = NULL;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList, eRole);
	DWORD maxFS = 0;
	for (int i = 0; i < numOfMediaCapSet; i++)
	{
		// && ( (pHandlCap->GetCapCode() != (CapEnum)eH264CapCode) ||(pHandlCap->GetCapCode() == (CapEnum)eH264CapCode && ((CH264VideoCap *)pHandlCap)->GetProfile() == eProfile) ) /*and*/)  /*end if*/)
		pHandlCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode, pMediaCapList[i]->dataCap);
		PASSERTMSG_AND_RETURN_VALUE(pHandlCap==NULL, "AllocNewCap returned NULL", 0);
		if ( IsH264Video((CapEnum)pMediaCapList[i]->capTypeCode) && ((CH264VideoCap *)pHandlCap)->GetProfile() == profile)
		{
			maxFS =((CH264VideoCap *)pHandlCap)->GetFs();
			POBJDELETE (pHandlCap);
			PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetMaxFsAccordingToProfile - fs found is ,",maxFS);
			break;
		}
		POBJDELETE (pHandlCap);


	}

	PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetMaxFsAccordingToProfile - fs is ,",maxFS);
	return maxFS;



}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
payload_en CSipCaps::GetPayloadTypeByDynamicPreference(const CCapSetInfo& capInfo, WORD profile, ERoleLabel eRole, APIS32 H264mode, APIU8 packetizationMode) const
{
	payload_en ePayload 	   = _UnKnown;
	BYTE bIsDynamicFound       = FALSE;
	int numOfMediaCapSet 	   = 0;
	capBuffer** pMediaCapList  = NULL;
	cmCapDataType eMediaType   = capInfo.GetSipCapType();

	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
//	TRACEINTO<<"$#@ inside GetPayloadTypeByDynamicPreference";

	for (int i = 0; (i < numOfMediaCapSet) && !bIsDynamicFound; i++)
	{
		if ((CapEnum)pMediaCapList[i]->capTypeCode == (CapEnum)capInfo)
		{
			if (((CapEnum)capInfo == eH264CapCode) && (eRole & kRoleContentOrPresentation)) {
//				PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetPayloadTypeByDynamicPreference profileValue: ",((h264CapStruct *)pMediaCapList[i]->dataCap)->profileValue );
//				PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetPayloadTypeByDynamicPreference profile: ",profile );
//				PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetPayloadTypeByDynamicPreference ((h264CapStruct *)pMediaCapList[i]->dataCap)->H264mode: ",((h264CapStruct *)pMediaCapList[i]->dataCap)->H264mode);
//				PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetPayloadTypeByDynamicPreference H264mode: ",H264mode);
			}

			BYTE bPacketizationTypesEqual = TRUE;
			if( ((CapEnum)capInfo == eH264CapCode) &&
				(CProcessBase::GetProcess()->GetProductType() !=eProductTypeSoftMCUMfw) ) // we don't compare packetization types for SoftMCUMfw.
				bPacketizationTypesEqual = (((h264CapStruct *)pMediaCapList[i]->dataCap)->packetizationMode >= packetizationMode); //BRIDGE-13459
			

			if ( ((CapEnum)capInfo != eH264CapCode) ||
					((((h264CapStruct *)pMediaCapList[i]->dataCap)->profileValue == profile) && (((h264CapStruct *)pMediaCapList[i]->dataCap)->H264mode == H264mode) && (H264mode == 1) && bPacketizationTypesEqual) ||
		//				((((h264CapStruct *)pMediaCapList[i]->dataCap)->profileValue == profile) && (((h264CapStruct *)pMediaCapList[i]->dataCap)->H264mode == H264mode) && (H264mode == 1) && ((h264CapStruct *)pMediaCapList[i]->dataCap)->packetizationMode == packetizationMode) ||
				((((h264CapStruct *)pMediaCapList[i]->dataCap)->H264mode == H264mode) && (H264mode == H264_tipContent)) )
			{

				//PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetPayloadTypeByDynamicPreference ePayload: ",(payload_en)pMediaCapList[i]->sipPayloadType );
//				TRACEINTO<<"$#@ 1 pMediaCapList[i]->sipPayloadType: "<<(int)pMediaCapList[i]->sipPayloadType;
				ePayload = (payload_en)pMediaCapList[i]->sipPayloadType;
				if (IsDynamicPayloadType(ePayload))
					bIsDynamicFound = TRUE;
			}

		} else {

			//PTRACE(eLevelInfoNormal,"CSipCaps::GetPayloadTypeByDynamicPreference ELSE");

			if ((isSirenLPRCap((CapEnum)capInfo) && (isSirenLPRCap((CapEnum)pMediaCapList[i]->capTypeCode))) ||
				(isG719Cap((CapEnum)capInfo) && (isG719Cap((CapEnum)pMediaCapList[i]->capTypeCode))) ||
				(isiLBCCap((CapEnum)capInfo) && (isiLBCCap((CapEnum)pMediaCapList[i]->capTypeCode))) ||
				(isOpusCap((CapEnum)capInfo) && (isOpusCap((CapEnum)pMediaCapList[i]->capTypeCode))))
			{
				if ((CapEnum)pMediaCapList[i]->capTypeCode >= (CapEnum)capInfo)
				{
//					TRACEINTO<<"$#@ 2 pMediaCapList[i]->sipPayloadType: "<<(int)pMediaCapList[i]->sipPayloadType;
							ePayload = (payload_en)pMediaCapList[i]->sipPayloadType;
							if (IsDynamicPayloadType(ePayload))
								bIsDynamicFound = TRUE;
				}
			}
		}
	}
	return ePayload;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
const capBuffer* CSipCaps::GetCapSetAsCapBuffer(cmCapDataType eMediaType,int arrIndex, ERoleLabel eRole) const
{
	capBuffer* pCapBuffer		= NULL;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
	if (arrIndex < numOfMediaCapSet)
		pCapBuffer = pMediaCapList[arrIndex];
	else
		DBGPASSERT(arrIndex);
	return pCapBuffer;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
int CSipCaps::GetIndexInArr(const CCapSetInfo& capInfo,int index, ERoleLabel eRole) const
{
	int indexInArr = NA;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	cmCapDataType eMediaType = capInfo.GetSipCapType();

	if(capInfo.IsCapCode(eLPRCapCode) || capInfo.IsCapCode(eFECCapCode))
		eMediaType = cmCapVideo;
	if(capInfo.IsCapCode(eREDCapCode))
		eMediaType = cmCapAudio;

	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
	if (index < numOfMediaCapSet)
	{
		for (int i = 0 ,k = index; ( i < numOfMediaCapSet && indexInArr == NA); i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == (CapEnum)capInfo)
			{
				if (k == 0)
				{
					indexInArr = i;
				}
				else
				{
					k--;
				}
			}
		}
	}
	return indexInArr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int CSipCaps::GetIndexInArrAccordingToCapSetAndProfile(const CCapSetInfo& capInfo,int index, ERoleLabel eRole,APIU16 profile) const
{
	int indexInArr = NA;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	cmCapDataType eMediaType = capInfo.GetSipCapType();
	if(capInfo.IsCapCode(eLPRCapCode))
		eMediaType = cmCapVideo;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
	if (index < numOfMediaCapSet)
	{
		for (int i = 0 ,k = index; ( i < numOfMediaCapSet && indexInArr == NA); i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == (CapEnum)capInfo)
			{
				CBaseCap* pCurCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode,pMediaCapList[i]->dataCap);
				if ( (pCurCap && (CapEnum)capInfo == eH264CapCode && ((CH264VideoCap*)pCurCap)->GetProfile() == profile) || (pCurCap && (CapEnum)capInfo != eH264CapCode ) )
				{
					if (k == 0)
					{
						indexInArr = i;
					}
					else
					{
						k--;
					}
				}
				POBJDELETE(pCurCap);
			}
		}
	}
	return indexInArr;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////
const capBuffer* CSipCaps::GetCapSetAsCapBuffer(const CCapSetInfo& capInfo, int index, ERoleLabel eRole) const
{
	int numOfMediaCapSet = 0;
	capBuffer* pCapBuffer = NULL;
	capBuffer** pMediaCapList	= NULL;
	cmCapDataType eMediaType = capInfo.GetCapType();
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
	int indexInArr = GetIndexInArr(capInfo,index, eRole);

	if (indexInArr != NA)
	{
		pCapBuffer = pMediaCapList[indexInArr];
	}

	return pCapBuffer;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsContainingCapSet(cmCapDirection eDirection, const CBaseCap& rCap, DWORD valuesToCompare, DWORD* pDetails, int* pArrInd) const
{
	BYTE res = NO;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	cmCapDataType eMediaType = rCap.GetType();
	ERoleLabel eRole = rCap.GetRole();
	*pArrInd = 0xFFFFFFFF;

	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet && res == NO; i++)
		{
			CBaseCap* pCurCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode,pMediaCapList[i]->dataCap);
			
			if (pCurCap)
			{
				// don't need to check direction. We control mute in a different way.
				res = pCurCap->IsContaining(rCap, valuesToCompare, pDetails) ? YES : NO;

				if (res)
				{
					*pArrInd = i;
				}
			}
			
			POBJDELETE(pCurCap);
		}
	}

	return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsContainedInCapSet(const CBaseCap& rCap, DWORD valuesToCompare, DWORD* pDetails, int* pArrInd) const
{
	BYTE res = NO;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	cmCapDataType eMediaType = rCap.GetType();
	ERoleLabel eRole = rCap.GetRole();
	*pArrInd = 0xFFFFFFFF;

	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if ( pMediaCapList )
	{
		for (int i = 0; i < numOfMediaCapSet && res == NO; i++)
		{
			CBaseCap* pCurCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode,pMediaCapList[i]->dataCap);
			if ( pCurCap )
			{
				// don't need to check direction. We control mute in a different way.
				res = rCap.IsContaining(*pCurCap, valuesToCompare, pDetails) ? YES : NO;
				if (res)
				{
					*pArrInd = i;
				}
			}
			POBJDELETE(pCurCap);
		}
	}

	return res;
}

BYTE CSipCaps::IsSdesEquals(CSdesCap *pCurSdesCap,cmCapDataType eMediaType, cmCapDirection eDirection, ERoleLabel eRole) const
{
    BYTE res = FALSE;
    BYTE bCurIterationEqual = TRUE;
    int numOfMediaCapSet = 0;
    capBuffer** pMediaCapList = NULL;
    GetSdesMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

    if (pCurSdesCap == NULL)
    {
        return TRUE;
    }

    if (pMediaCapList)
    {
        for (int i = 0; (i < numOfMediaCapSet )&& (res == FALSE); i++)
        {
            if ((CapEnum)pMediaCapList[i]->capTypeCode == eSdesCapCode)
            {
                CSdesCap* pUpdatedSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[i]->dataCap);

                if(((!pCurSdesCap) && pUpdatedSdesCap) || ((!pUpdatedSdesCap) && pCurSdesCap))
                    return TRUE;

                if (pUpdatedSdesCap == NULL)
                {
                    PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals pUpdatedSdesCap is NULL!");
                    return FALSE;
                }
                else
                {
                    //BRIDGE-13046
                    //if (pCurSdesCap->GetSdesTag() != pUpdatedSdesCap->GetSdesTag()) {

                    //    bCurIterationEqual&= FALSE;
                    //    PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.Tag is different");
                    //}
                    if (pCurSdesCap->GetSdesCryptoSuite() != pUpdatedSdesCap->GetSdesCryptoSuite()) {

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.CryptoSuite is different");
                    }
                    if (pCurSdesCap->GetIsSdesFecOrderInUse()!= pUpdatedSdesCap->GetIsSdesFecOrderInUse()) {

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.FecOrderInUse is different");
                    }
                    if (pCurSdesCap->GetSdesFecOrder() != pUpdatedSdesCap->GetSdesFecOrder()) {

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.FecOrder is different");
                    }
                    if (pCurSdesCap->GetIsSdesKdrInUse()!= pUpdatedSdesCap->GetIsSdesKdrInUse()) {

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.IsSdesKdrInUse is different");
                    }
                    if (pCurSdesCap->GetSdesKdr() != pUpdatedSdesCap->GetSdesKdr()) {

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.GetSdesKdr is different");
                    }
                    if (pCurSdesCap->GetIsSdesWshInUse() != pUpdatedSdesCap->GetIsSdesWshInUse()) {

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.IsSdesWshInUse is different");
                    }
                    if (pCurSdesCap->GetSdesWsh() != pUpdatedSdesCap->GetSdesWsh()) {

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.SdesWsh is different");
                    }
                    if (pCurSdesCap->GetIsSdesFecKeyInUse()!= pUpdatedSdesCap->GetIsSdesFecKeyInUse()) {

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.IsSdesFecKeyInUse is different");
                    }
                    if (pCurSdesCap->GetIsSdesUnauthenticatedSrtp()!= pUpdatedSdesCap->GetIsSdesUnauthenticatedSrtp()){

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.IsSdesUnauthenticatedSrtp is different");
                    }
                    if (pCurSdesCap->GetIsSdesUnencryptedSrtcp()!= pUpdatedSdesCap->GetIsSdesUnencryptedSrtcp()) {

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.IsSdesUnencryptedSrtcp is different");
                    }
                    if (pCurSdesCap->GetIsSdesUnencryptedSrtp()!= pUpdatedSdesCap->GetIsSdesUnencryptedSrtp()) {

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.IsSdesUnencryptedSrtp is different");
                    }


                    APIU32 numOfThisKeys = pCurSdesCap->GetSdesNumOfKeysParam();
                    APIU32 numOfOtherKeys = pUpdatedSdesCap->GetSdesNumOfKeysParam();
                    if(numOfThisKeys != numOfOtherKeys) {

                        bCurIterationEqual&= FALSE;
                        PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.num of keys is different");
                    } else {
						for (APIU32 keyNumber = 0; keyNumber < numOfThisKeys; keyNumber++) {
							if (pCurSdesCap->GetSdesKeyMethod(keyNumber) != pUpdatedSdesCap->GetSdesKeyMethod(keyNumber)) {

								bCurIterationEqual&= FALSE;
								PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.SdesKeyMethod is different");
							}
							if(pCurSdesCap->GetIsSdesLifeTimeInUse(keyNumber)!= pUpdatedSdesCap->GetIsSdesLifeTimeInUse(keyNumber)) {

								bCurIterationEqual&= FALSE;
								PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.IsSdesLifeTimeInUse is different");
							}
							if (pCurSdesCap->GetSdesLifeTime(keyNumber) != pUpdatedSdesCap->GetSdesLifeTime(keyNumber)) {

								bCurIterationEqual&= FALSE;
								PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.SdesLifeTime is different");
							}
							if (pCurSdesCap->GetIsSdesMkiInUse(keyNumber) != pUpdatedSdesCap->GetIsSdesMkiInUse(keyNumber)) {

								bCurIterationEqual&= FALSE;
								PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.IsSdesMkiInUse is different");
							}
							if (pCurSdesCap->GetSdesMkiValue(keyNumber) != pUpdatedSdesCap->GetSdesMkiValue(keyNumber)) {

								bCurIterationEqual&= FALSE;
								PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.SdesMkiValue is different");
							}
							if (pCurSdesCap->GetIsSdesMkiValueLenInUse(keyNumber) != pUpdatedSdesCap->GetIsSdesMkiValueLenInUse(keyNumber)) {

								bCurIterationEqual&= FALSE;
								PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.SdesMkiValueLenInUse is different");
							}
							if (pCurSdesCap->GetSdesMkiValueLen(keyNumber) != pUpdatedSdesCap->GetSdesMkiValueLen(keyNumber)) {

								bCurIterationEqual&= FALSE;
								PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.SdesMkiValueLen is different");
							}
							if ( (pCurSdesCap->GetSdesBase64KeySalt(keyNumber) == NULL) || (pUpdatedSdesCap->GetSdesBase64KeySalt(keyNumber) == NULL) )
							{

								bCurIterationEqual&= FALSE;
								PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.SdesBase64KeySalt - GetSdesBase64KeySalt(keyNumber) is NULL");
							}
							else
							{
								char * pCurSdesBase64KeySalt     = pCurSdesCap->GetSdesBase64KeySalt(keyNumber);
								char * pUpdatedSdesBase64KeySalt = pUpdatedSdesCap->GetSdesBase64KeySalt(keyNumber);

								if ( (pCurSdesBase64KeySalt != NULL) && (pUpdatedSdesBase64KeySalt != NULL)  &&
									strcmp(pCurSdesBase64KeySalt, pUpdatedSdesBase64KeySalt))
								{

									bCurIterationEqual&= FALSE;
									PTRACE(eLevelInfoNormal, "CSipCaps::IsSdesEquals.SdesBase64KeySalt is different");
								}
							}
						}
                    }

                    POBJDELETE(pUpdatedSdesCap);
					if(TRUE== bCurIterationEqual)
						res = TRUE;
					bCurIterationEqual = TRUE;
                }
            }
        }
    }

    return res;

}
//BYTE CSipCaps::IsContainingEqualsCapSet(cmCapDirection eDirection,const CBaseCap & rCap,BYTE valuesToCompare,WORD * pDetails,int * pArrInd) const

//{

//	BYTE res = NO;

//	int numOfMediaCapSet		= 0;

//	capBuffer** pMediaCapList	= NULL;

//	cmCapDataType eMediaType	= rCap.GetType();

//	*pArrInd = NA;

//

//	GetMediaCaps(eMediaType,&numOfMediaCapSet,&pMediaCapList);

//

//	if (pMediaCapList)

//	{

//		for (int i=0; i<numOfMediaCapSet && res==NO; i++)

//		{

//			CBaseCap* pCurCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode,pMediaCapList[i]->dataCap);

//			if (pCurCap)

//			{

//				res = pCurCap->IsDirection(eDirection);

//				if (res)

//				{

//					res = pCurCap->IsEquals(rCap,valuesToCompare)? YES: NO;

//					if (res)

//						*pArrInd = i;

//				}

//			}

//			POBJDELETE(pCurCap);

//		}

//	}

//

//	return res;

//}



//BYTE CSipCaps::IsContainingScm(const CSipComMode* pScm) const

//{

//	BYTE bRes = YES;

//	cmCapDataType	mediaArr[MAX_SIP_MEDIA_TYPES]			= {cmCapAudio,cmCapVideo};

//	BYTE			valuesToCompare[MAX_SIP_MEDIA_TYPES]	= {(kCapCode|kFrameRate),(kCapCode|kFrameRate|kBitRate)};

//

//	WORD details = 0;

//	int  arrInd  = 0;

//

//	for (int i=0;i<MAX_SIP_MEDIA_TYPES && bRes;i++)

//		bRes = IsContainingMode(pScm,mediaArr[i],cmCapReceiveAndTransmit,valuesToCompare[i]);

//

//	return bRes;

//}



//BYTE CSipCaps::IsContainingMode(const CSipComMode* pScm,cmCapDataType eType,cmCapDirection eDirection,BYTE valuesToCompare) const

//{

//	cmCapDirection directionArr[2] = {cmCapReceive,cmCapTransmit};

//

//	BYTE bRes		= YES;

//	WORD details	= 0;

//	int  arrInd		= 0;

//

//	for (int j=0; j<2 && (directionArr[j] & eDirection) && bRes; j++)

//	{

//		CBaseCap* pMode = pScm->GetMediaAsCapClass(eType,directionArr[j]);

//		if (pMode)

//		{

//			cmCapDirection eOppositeDirection = (directionArr[j] == cmCapTransmit)? cmCapReceive: cmCapTransmit;

//			bRes = IsContainingCapSet(eOppositeDirection,*pMode,valuesToCompare,&details,&arrInd);

//			if (bRes == NO)

//			{

//				CSmallString str;

//				DumpDetailsToStream(eType,details,str);

//				PTRACE2(eLevelInfoNormal,"CSipCaps::IsContainingMode: Doesn't contain - Details: ",str.GetString());

//			}

//		}

//		POBJDELETE(pMode);

//	}

//

//	return bRes;

//}



///////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsContainingCapBuffer(cmCapDirection eDirection, const capBuffer& rCapBuffer, DWORD valuesToCompare, int* pArrInd,BYTE checkCapDirection) const
{
	BYTE res = NO;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	*pArrInd = 0xFFFFFFFF;
	DWORD details = 0;

	CBaseCap* pGivenCap = CBaseCap::AllocNewCap((CapEnum)rCapBuffer.capTypeCode, (void *)rCapBuffer.dataCap);

	if ( pGivenCap )
	{

		cmCapDataType eMediaType = pGivenCap->GetType();
		ERoleLabel eRole = pGivenCap->GetRole();
		GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

		if ( pMediaCapList )
		{
			for (int i = 0; i < numOfMediaCapSet && res==NO; i++)
			{
				res = (rCapBuffer.sipPayloadType == pMediaCapList[i]->sipPayloadType);

				if ( res )
				{
					CBaseCap* pCurCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode,pMediaCapList[i]->dataCap);

					if ( pCurCap )
					{

						res = pCurCap->IsDirection(eDirection);
						if ( res || checkCapDirection == FALSE )
						{
							//8.1.6 merge - Remove duplicate payloadtype in SDP
							if	( ( isG719Cap(pCurCap->GetCapCode())         && isG719Cap(pGivenCap->GetCapCode())         )    ||
								  ( isSirenLPRCap(pCurCap->GetCapCode())     && isSirenLPRCap(pGivenCap->GetCapCode())     )	||
								  ( pCurCap->GetCapCode() == eAAC_LDCapCode  && pGivenCap->GetCapCode() == eAAC_LDCapCode  )
								)
							{
								*pArrInd = i;
							}
							else
							{
								res = pCurCap->IsContaining(*pGivenCap, valuesToCompare, &details) ? YES : NO;
							}
							if (res)
							{
								*pArrInd = i;
							}
						}
					}

					POBJDELETE(pCurCap);
				}
			}
		}
	}

	POBJDELETE(pGivenCap);
	return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsContainingCapBufferForSdes(cmCapDirection eDirection, const capBuffer& rCapBuffer, DWORD valuesToCompare, int* pArrInd) const
{
	BYTE res = NO;
	int numOfSdesMediaCapSet = 0;
	capBuffer** pSdesMediaCapList	= NULL;
	*pArrInd = 0xFFFFFFFF;
	DWORD details = 0;

	CBaseCap* pGivenCap = CBaseCap::AllocNewCap((CapEnum)rCapBuffer.capTypeCode, (void *)rCapBuffer.dataCap);

	if ( pGivenCap )
	{
		cmCapDataType eMediaType = pGivenCap->GetType();
		ERoleLabel eRole = pGivenCap->GetRole();


			GetSdesMediaCaps(eMediaType,&numOfSdesMediaCapSet,&pSdesMediaCapList,eRole);
			if ( pSdesMediaCapList )
			{
				for (int i = 0; i < numOfSdesMediaCapSet && res==NO; i++)
				{



						CBaseCap* pCurCap = CBaseCap::AllocNewCap((CapEnum)pSdesMediaCapList[i]->capTypeCode,pSdesMediaCapList[i]->dataCap);

						if ( pCurCap )
						{
							if ( pCurCap->IsDirection(eDirection) )
							{

								res = pCurCap->IsContaining(*pGivenCap, valuesToCompare, &details) ? YES : NO;

								if (res)
								{
									*pArrInd = i;
								}
							}
						}

						POBJDELETE(pCurCap);

				}
			}




	}

	POBJDELETE(pGivenCap);
	return res;
}
//////////////////////////////////////////////////////////////////////////////////////
CBaseCap* CSipCaps::GetHighestRemoteAndLocalCaps(cmCapDirection eDirection, ERoleLabel eRole, cmCapDataType mediaType, const CBaseCap& rOtherCap,const CSipCaps& rAlternativeCaps) const
{

	CBaseCap* pBestCap = NULL;
	CBaseCap* pCurCap;
	CCapSetInfo capInfo;
	CapEnum eCurAlg;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	BYTE bFound = NO;
	rAlternativeCaps.GetMediaCaps(mediaType,&numOfMediaCapSet,&pMediaCapList,eRole);
	for (int k = 0; (k < numOfMediaCapSet) && (bFound == NO); k++)
	{
		pCurCap = rAlternativeCaps.GetCapSet(mediaType, k, eRole);
		eCurAlg = pCurCap ? pCurCap->GetCapCode() : eUnknownAlgorithemCapCode;
		capInfo = eCurAlg;
		if (pCurCap && capInfo.IsSupporedCap())
		{


			if(mediaType==cmCapVideo && eRole==kRolePeople)
			{
				COstrStream msg;
				msg<<"avc_vsw_relay: attempting video localCap:";
				pCurCap->Dump(msg);
				PTRACE(eLevelInfoNormal,msg.str().c_str());
			}
			else if(mediaType==cmCapVideo && eRole==kRolePresentation)
			{
				COstrStream msg;
				msg<<"avc_vsw_relay: attempting content localCap:";
				pCurCap->Dump(msg);
				PTRACE(eLevelInfoNormal,msg.str().c_str());
			}

			pBestCap = GetHighestCommon(eDirection, *pCurCap);
			if(pBestCap!=NULL)
			{
				bFound=YES;
			}
		}
		PDELETE(pCurCap);
	}
	if(pBestCap && mediaType==cmCapVideo && eRole==kRolePeople)
	{

		COstrStream msg;
		msg<<"avc_vsw_relay: found best video localCap:";
		pBestCap->Dump(msg);
		PTRACE(eLevelInfoNormal,msg.str().c_str());

	}
	else if(pBestCap && mediaType==cmCapVideo && eRole==kRolePresentation)
	{

		COstrStream msg;
		msg<<"avc_vsw_relay: found best content localCap:";
		pBestCap->Dump(msg);
		PTRACE(eLevelInfoNormal,msg.str().c_str());

	}

	return pBestCap;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// version 7.8
CBaseCap* CSipCaps::GetHighestCommon(cmCapDirection eDirection, const CBaseCap& rOtherCap) const
{
	DWORD details = 0;
	int	arrInd = NA;
	DWORD values = kCapCode|kRoleLabel|kH264Profile |kPacketizationMode;
	CBaseCap* pBestCap = NULL;

	if(CProcessBase::GetProcess()->GetProductType()==eProductTypeSoftMCUMfw)
	{
		values &= ~kPacketizationMode;
	}

	// we won't check if contains frame rate or video format cause we do a highest common
	// we also won't check bit rate because in CP we don't have to be contained and in vsw we will be set to secondary.

    BYTE res = IsContainingCapSet(eDirection, rOtherCap, values, &details, &arrInd);

	cmCapDataType eType = rOtherCap.GetType();
	CSmallString str;
	CCapSetInfo otherCapInfo = rOtherCap.GetCapCode();
	str << "Compared cap is " << (const char*)otherCapInfo << "\n";
	DumpDetailsToStream(eType, details, str);

	if (res && arrInd != NA)
	{
		cmCapDataType eType = rOtherCap.GetType();
		ERoleLabel eRole = rOtherCap.GetRole();

		CBaseCap* pCap = GetCapSet(eType,arrInd,eRole);

		if (pCap && isG719Cap(rOtherCap.GetCapCode()))
		{
			CapEnum capCodeOfPcap = pCap->GetCapCode();

			//vngr-22756, special case where RMX doesn't support G719_128,96 mono
			if ((capCodeOfPcap == eG719_128kCapCode) || (capCodeOfPcap == eG719_96kCapCode))
			{
				TRACEINTO << "changed to eG719_64kCapCode as we don't support G719_128,96 mono";
				pCap->SetCapCode(eG719_64kCapCode);
			}
		}

		if (pCap)
		{
			pBestCap = pCap->GetHighestCommon(rOtherCap);

			if(pBestCap == NULL) 
			{
				PTRACE2(eLevelInfoNormal, "CSipCaps::GetHighestCommon: No common Mode. ", str.GetString());
			}
			else 
			{
				if ((pCap->GetRole() == kRolePeople) &&
					(pCap->GetCapCode() == eH264CapCode)) 
				{
					APIS32 Mbpsr,Fsr,Mbpsl,Fsl,Mbpsb,Fsb;
					APIS16 frameRater,frameRatel,frameRateb;
					CH264VideoCap* pH264VideoCap;

					pH264VideoCap=(CH264VideoCap*)pCap; /* remote  */
					Mbpsr=pH264VideoCap->GetMbps();
					Fsr=pH264VideoCap->GetFs();
					frameRater=pH264VideoCap->GetFrameRate();

					pH264VideoCap=(CH264VideoCap*)&rOtherCap; /* local  */
					Mbpsl=pH264VideoCap->GetMbps();
					Fsl=pH264VideoCap->GetFs();
					frameRatel=pH264VideoCap->GetFrameRate();

					pH264VideoCap=(CH264VideoCap*)pBestCap; /* best  */
					Mbpsb=pH264VideoCap->GetMbps();
					Fsb=pH264VideoCap->GetFs();
					frameRateb=pH264VideoCap->GetFrameRate();

					TRACEINTOFUNC << "avc_vsw_relay: compare best mode for video:\nRemote: "<<"direction: "<<eDirection<<" Mbps: "<<Mbpsr<<" Fs: "<<Fsr<<" frameRate:"<<frameRater
								  << "\nLocal: "<<"direction: "<<eDirection<<" Mbps: "<<Mbpsl<<" Fs: "<<Fsl<<" frameRate:"<<frameRatel
								  << "\nBest: "<<"direction: "<<eDirection<<" Mbps: "<<Mbpsb<<" Fs: "<<Fsb<<" frameRate:"<<frameRateb;

					if (CProcessBase::GetProcess()->GetProductType() == eProductTypeSoftMCUMfw)
					{
						((CH264VideoCap*)pBestCap)->SetPacketizationMode(0);
						PTRACE2(eLevelInfoNormal, "CSipCaps::GetHighestCommon: Setting packetizationMode best mode to 0 ", str.GetString());
					}
				}
			}
		}
		
		POBJDELETE(pCap);
	}
	else if (details)
	{
		PTRACE2(eLevelInfoNormal, "CSipCaps::GetHighestCommon: Not containing. ", str.GetString());
	}

	return pBestCap;
}

///////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsCapSet(const CCapSetInfo& capInfo, ERoleLabel eRole) const
{
	BYTE res = NO;

	if (capInfo.IsType(cmCapAudio))
	{
		for (int i = 0; i < m_numOfAudioCapSets && res == NO; i++)
		{
			if (m_audioCapList[i]->capTypeCode == (CapEnum)capInfo)
			{
				res = YES;
			}
		}
	}
	else if (capInfo.IsType(cmCapVideo))
	{
		if (eRole == kRolePeople)
		{
			for (int i  =0; i < m_numOfVideoCapSets && res == NO; i++)
			{
				if (m_videoCapList[i]->capTypeCode == (CapEnum)capInfo)
				{
					res = YES;
				}
			}
		}
		else
		{
			for (int i  =0; i < m_numOfContentCapSets && res == NO; i++)
			{
				if (m_contentCapList[i]->capTypeCode == (CapEnum)capInfo)
				{
					res = YES;
				}
			}
		}
	}
	else if (capInfo.IsType(cmCapData))
	{
		for (int i  =0; i < m_numOfFeccCapSets && res == NO; i++)
		{
			if (m_feccCapList[i]->capTypeCode == (CapEnum)capInfo)
			{
				res = YES;
			}
		}
	}
	else if (capInfo.IsType(cmCapGeneric))
	{
		if (eRole == kRolePeople)
		{
			for (int i  =0; i < m_numOfVideoCapSets && res == NO; i++)
			{
				if (m_videoCapList[i]->capTypeCode == (CapEnum)capInfo)
				{
					res = YES;
				}
			}
		}
		else
		{
			for (int i  =0; i < m_numOfContentCapSets && res == NO; i++)
			{
				if (m_contentCapList[i]->capTypeCode == (CapEnum)capInfo)
				{
					res = YES;
				}
			}
		}
	}
	else if (capInfo.IsType(cmCapBfcp))
	{
		for (int i  =0; i < m_numOfBfcpCapSets && res == NO; i++)
		{
			if (m_bfcpCapList[i]->capTypeCode == (CapEnum)capInfo)
			{
				res = YES;
			}
		}
	}
	return res;
}

//// we try to find a match between the preffered mode and the current caps,
//// if not, we try to find a match between the alternative caps and the current caps.
//// if no match is found for either one of the media the function will return NULL pointer.
///// THIS FUNCTION CAN ONLY USE BY REMOTE CAPS, SINCE THE MATCH FOR RECEIVE AND TRANSMIT IS DIFFERENT!!!
////---------------------------------------------------------------------------
CSipComMode* CSipCaps::FindBestMode(cmCapDirection eDirection,const CSipComMode& rPreferredMode,const CSipCaps& rAlternativeCaps, BYTE &bWithinProtocolLimitation, BYTE bIsOffere, BYTE bIsMrcSlave) const
{
	CSipComMode* pBestMode = new CSipComMode;
	pBestMode->CopyStaticAttributes(rPreferredMode); // copy necessary data members like ConfType from preffered mode to best mode.
	CSipComMode* pTmpBestMode = NULL;

//	if(&rPreferredMode)		Commenting this, to trigger KW on calls to this function where the pointer was not checked before dereferencing it
//							Moreover, it triggered wrong KW-s on all the next uses of this reference.
//							At any rate - adding pointer checks to address-of statements of references is inconsistent with the entire code, for a good reason
	rPreferredMode.Dump("mix_mode: CSipCaps::FindBestMode: rPreferredMode is:\n",eLevelInfoNormal);

    if (rPreferredMode.GetConfMediaType() ==eMixAvcSvc && (rPreferredMode.GetMediaType(cmCapVideo, cmCapReceive) != eSvcCapCode))
    {// copy streams information
    	pBestMode->SetStreamsListForMediaMode(rPreferredMode.GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople), cmCapVideo, cmCapReceive, kRolePeople);
    	pBestMode->SetStreamsListForMediaMode(rPreferredMode.GetStreamsListForMediaMode(cmCapAudio, cmCapReceive, kRolePeople), cmCapAudio, cmCapReceive, kRolePeople);
    }

	cmCapDirection directionArr[2];
	int numOfDirection = (eDirection == cmCapReceiveAndTransmit) ? 2 : 1;
	directionArr[0] = (eDirection == cmCapTransmit) ? cmCapTransmit : cmCapReceive;
	directionArr[1]	= (eDirection == cmCapReceiveAndTransmit) ? cmCapTransmit : (cmCapDirection) 0;

	TRACEINTO << "rPreferredMode.GetConfType()= " << ConfTypeToString(rPreferredMode.GetConfType());

	WORD details = 0;
	BYTE bResArr[MAX_SIP_MEDIA_TYPES][2]	= {{NO,NO},{NO,NO},{NO,NO},{NO,NO}, {NO,NO}};
	CapEnum algFound[MAX_SIP_MEDIA_TYPES]	= {eUnknownAlgorithemCapCode, eUnknownAlgorithemCapCode, eUnknownAlgorithemCapCode, eUnknownAlgorithemCapCode, eUnknownAlgorithemCapCode};
	BYTE bAnyMediaFound = NO;
	CBaseCap* pBestCap = NULL;
	capBuffer* pBestCapBuffer = NULL;
	BYTE bIsVswFixed = (rPreferredMode.GetConfType() == kVSW_Fixed);
	BYTE bIsCp = (rPreferredMode.GetConfType() == kCp);
	BYTE bIsCop = (rPreferredMode.GetConfType() == kCop);
	cmCapDataType mediaType;
	ERoleLabel eRole;
	BYTE bIs264TipContentMatch = TRUE;

	for (int i = 0; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		BYTE bIsAudio 	= (mediaType == cmCapAudio);
		BYTE bIsVideo 	= ((mediaType == cmCapVideo) && (eRole==kRolePeople));
		BYTE bIsContent = ((mediaType == cmCapVideo) && (eRole==kRolePresentation));
		BYTE bIsData 	= (mediaType == cmCapData);
		BYTE bIsBfcp 	= (mediaType == cmCapBfcp);

		// in SVC_Cascade_Slave call, if its OperationPoints is not similar to Master's OperationPoints
        //    then the call should be downgraded to AudioOnly.
		//    note: 'FindBestMode' is called from 'remote' context; so in case we are Slave
        //       then 'this' is Master (the remote) and 'rPreferredMode' is Slave (the local)
        bool bIsSimilarOperationPoints = true;
        if (bIsMrcSlave && bIsVideo)
        {
        	bIsSimilarOperationPoints = IsSimilarOperationPoints(rPreferredMode);
        }

		for (int j = 0; j < numOfDirection; j++)
		{
			// if 'preferred mode' wishes to transmit h261, 'this' should be able to receive it
			cmCapDirection eOppositeDirection = (directionArr[j] == cmCapTransmit) ? cmCapReceive : cmCapTransmit;
			CBaseCap* pPreferredMedia = rPreferredMode.GetMediaAsCapClass(mediaType,eOppositeDirection,eRole);

			// SVC
			if (pPreferredMedia && pPreferredMedia->IsCapContainsStreamsGroup())
			{
				// in SVC_Cascade_Slave call, if its OperationPoints is not similar to Master's OperationPoints then the call should be downgraded to AudioOnly.
				if ( false == bIsSimilarOperationPoints )
				{
					TRACEINTO << "Slave's OperationPoints mismaches Master's OperationPoints; setting BEST mode to audio only";

					pBestMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
					pBestMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
					pBestMode->SetMediaOff(cmCapData, cmCapReceiveAndTransmit);
				}
				else
				{
					FindBestModeForSvc(directionArr, bResArr, algFound, i, j, eOppositeDirection, eRole, mediaType, rPreferredMode, pPreferredMedia, pBestMode, bIsMrcSlave);
				}
				POBJDELETE(pPreferredMedia); // BRIDGE-18219 (removed as in BRIDGE-14097, returned as in BRIDGE-14165)
				continue;
			}
			else if ((bIsAudio || bIsVideo) && pPreferredMedia && rPreferredMode.GetConfMediaType()==eMixAvcSvcVsw && rPreferredMode.GetMediaType(cmCapVideo, cmCapReceive)!=eSvcCapCode/*bIsActiveVswRelay*//*&& pPreferredMedia->IsCapContainsStreamsGroup()*/)
			{
				FindBestModeForAvcInVSWRelay(directionArr, bResArr, algFound, i, j, eOppositeDirection, eRole, mediaType, rPreferredMode, pPreferredMedia,rAlternativeCaps,pBestMode);
				POBJDELETE(pPreferredMedia); // BRIDGE-18219 (removed as in BRIDGE-14097, returned as in BRIDGE-14165)
				continue;
			}

            if(bIsVideo)
                PTRACE(eLevelInfoNormal, "CSipCaps::FindBestMode Is video cap ");

            bResArr[i][j] = NO; // zeroing

			if (pPreferredMedia) // there is a possibility that we won't prefer a specific media (auto)
			{
				CapEnum eAlg = pPreferredMedia->GetCapCode();
				CCapSetInfo capInfo = eAlg;

				if (capInfo.IsSupporedCap())
				{
					BYTE bTakeFromScm = FALSE;
					DWORD commonRtcpFbMask = 0;
					if (bIsVswFixed && bIsVideo)
					{
						BYTE bIsProtocolMatch = IsCapSet(capInfo,eRole);
						if (bIsProtocolMatch)
						{
						    //PTRACE2INT(eLevelInfoNormal, "CSipCaps::FindBestMode1 bTakeFromScm:",bTakeFromScm);
							DWORD tmpDetails	 = 0;
							int  tmpArrIndex = 0;
							//WE will not check bitrate in VSW
							BOOL bEnableFlowControlVSW = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_VSW_FLOW_CONTROL);
							DWORD videoValuesToCompare = kH264Profile|kCapCode|kRoleLabel|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS|kH264Additional_FS|kMaxFR|kH264Mode ;
							if(!bEnableFlowControlVSW)
								videoValuesToCompare = kH264Profile|kCapCode|kRoleLabel|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS|kH264Additional_FS|kMaxFR|kH264Mode | kBitRate ; // we do not check level
							BYTE bAreParametersInsideProtocolMatch = IsContainingCapSet(cmCapReceive, *pPreferredMedia, videoValuesToCompare, &tmpDetails, &tmpArrIndex);
							bTakeFromScm = bAreParametersInsideProtocolMatch;
							//PTRACE2INT(eLevelInfoNormal, "CSipCaps::FindBestMode2 bTakeFromScm:",bTakeFromScm);
							PTRACE2INT(eLevelInfoNormal, "CSipCaps::FindBestMode.bAreParametersInsideProtocolMatch: ",bAreParametersInsideProtocolMatch);
							//save RtcpFeedback
							if (bTakeFromScm && tmpArrIndex != NA)
							{
								CBaseVideoCap* pCap =  (CBaseVideoCap*)GetCapSet(mediaType,tmpArrIndex,eRole);
								if (pCap)
								{
									commonRtcpFbMask = pCap->GetRtcpFeedbackMask();
									POBJDELETE(pCap);
								}
							}
							if (bAreParametersInsideProtocolMatch == FALSE)
								bWithinProtocolLimitation = TRUE;


						}
					}
					else if (bIsContent)
					{
					    //PTRACE2INT(eLevelInfoNormal, "CSipCaps::FindBestMode3 bTakeFromScm:",bTakeFromScm);

					    BYTE bIsProtocolMatch = IsCapSet(capInfo,eRole);

						bIs264TipContentMatch = TRUE;
						if (rPreferredMode.IsTIPContentEnableInH264Scm() && !IsTipCompatibleContentSupported()) //just for TipCompatibility:video&content!
						{
							bIs264TipContentMatch = FALSE;
						}

						if (bIsProtocolMatch && bIs264TipContentMatch)
						{
							//WORD videoValuesToCompare = kCapCode|kRoleLabel|kBitRate|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS|kH264Additional_FS; // we do not check level
							//BYTE bAreParametersInsideProtocolMatch = IsContainingCapSet(cmCapReceive, *pPreferredMedia, videoValuesToCompare, &tmpDetails, &tmpArrIndex);
							bTakeFromScm = TRUE;//bAreParametersInsideProtocolMatch;
						}
						if (rPreferredMode.GetIsTipMode()) //is tip CALL..
						{
							PTRACE(eLevelInfoNormal, "CSipCaps::FindBestMode. Tip mode");
							bTakeFromScm = TRUE;
						}


						//PTRACE2INT(eLevelInfoNormal, "CSipCaps::FindBestMode4 bTakeFromScm:",bTakeFromScm);
						DWORD tmpDetails	 = 0;
						int  tmpArrIndex = 0;
						DWORD videoValuesToCompare = kCapCode|kRoleLabel; //Only compare CapCode abd RoleLabel
						if (IsContainingCapSet(cmCapReceive, *pPreferredMedia, videoValuesToCompare, &tmpDetails, &tmpArrIndex))
						{
							//save RtcpFeedback
							if (bTakeFromScm && tmpArrIndex != NA)
							{
								CBaseVideoCap* pCap =  (CBaseVideoCap*)GetCapSet(mediaType,tmpArrIndex,eRole);

								if (pCap)
								{
									commonRtcpFbMask = pCap->GetRtcpFeedbackMask();									
									PTRACE2INT(eLevelInfoNormal, "CSipCaps::FindBestMode for content, rtcp-fb: ", commonRtcpFbMask);
									POBJDELETE(pCap);
								}
							}
						}
					}
					else if (bIsCop && bIsVideo)
					{
					    //PTRACE2INT(eLevelInfoNormal, "CSipCaps::FindBestMode5 bTakeFromScm:",bTakeFromScm);

					    if (eOppositeDirection == cmCapTransmit)
						{
							// Cop Transmit - the rPreferredMode already contains the correct video cop level mode, now just intersect other params (dpb, br etc.):
							pBestCap = GetHighestCommon(directionArr[j], *pPreferredMedia);
						}
						else
						{
							// Cop Receive - the rPreferredMode contains the exact required video mode. We only should check if protocol is supported by remote caps:
							DWORD tmpDetails	 = 0;
							int  tmpArrIndex = 0;
							DWORD videoValuesToCompare = kCapCode|kRoleLabel|kH264Profile;
							if (IsContainingCapSet(cmCapReceive, *pPreferredMedia, videoValuesToCompare, &tmpDetails, &tmpArrIndex))
							{
								bTakeFromScm = TRUE;
								//save RtcpFeedback
								if (tmpArrIndex != NA)
								{
									CBaseVideoCap* pCap =  (CBaseVideoCap*)GetCapSet(mediaType,tmpArrIndex,eRole);
									if (pCap)
									{
										commonRtcpFbMask = pCap->GetRtcpFeedbackMask();
										POBJDELETE(pCap);
									}
								}
							}
						}
					}
					else if (bIsBfcp)
					{
						DWORD tmpDetails = 0;
						int  tmpArrIndex = 0;

						DWORD valuesToCompare = kCapCode|kRoleLabel|kTransportType;

						if (IsContainingCapSet(cmCapReceive, *pPreferredMedia, valuesToCompare, &tmpDetails, &tmpArrIndex))
						{
							PTRACE(eLevelInfoNormal, "CSipCaps::FindBestMode. bfcp find matching");
							bTakeFromScm = TRUE;
						}
						else
							PTRACE(eLevelInfoNormal, "CSipCaps::FindBestMode. bfcp didn't find matching");

					}
					else
					{
						if (bIsCp && bIsVideo && (eAlg == eH264CapCode) && (eOppositeDirection == cmCapTransmit) && (IsCapSet(eH264CapCode,eRole) ||IsCapSet(eRtvCapCode,eRole)))
						{
							/////////////////HD1080////////////////////////
							BYTE bRemoteSupportHD1080 = IsCapableOfHD1080();
							BYTE bLocalSupportHD1080  = rPreferredMode.IsHd1080Enabled();

							if (bRemoteSupportHD1080 && bLocalSupportHD1080)
							{
								CSipComMode* pScmWithHD1080 = new CSipComMode;
								*pScmWithHD1080 = rPreferredMode;
								pScmWithHD1080->SetScmToHdCp(eHD1080Res, cmCapTransmit);
								POBJDELETE(pPreferredMedia);
								pPreferredMedia = pScmWithHD1080->GetMediaAsCapClass(mediaType, cmCapTransmit);
								pBestCap = GetHighestCommon(directionArr[j], *pPreferredMedia);
								POBJDELETE(pScmWithHD1080);
							}

							/////////////////HD720@60////////////////////////
							BYTE bRemoteSupportHD720At60 = IsCapableOfHD720At50();//the threshold TO SUPPORT HD720At60asymmetric mode is HD720@50
							BYTE bLocalSupportHD720At60  = rPreferredMode.IsHd720At60Enabled();

							if (bRemoteSupportHD720At60 && bLocalSupportHD720At60)
							{
								CSipComMode* pScmWithHD720At60 = new CSipComMode;
								*pScmWithHD720At60 = rPreferredMode;
								pScmWithHD720At60->SetScmToCpHD720At60(cmCapTransmit);
								POBJDELETE(pPreferredMedia);
								pPreferredMedia = pScmWithHD720At60->GetMediaAsCapClass(mediaType, cmCapTransmit);
								pBestCap = GetHighestCommon(directionArr[j], *pPreferredMedia);
								POBJDELETE(pScmWithHD720At60);
							}

							BYTE bRemoteSupportHD1080At60 = IsCapableOfHD1080At60();
						    BYTE bLocalSupportHD1080At60  = rPreferredMode.IsHd1080At60Enabled();

							if (bRemoteSupportHD1080At60 && bLocalSupportHD1080At60)
							{
								   PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode - 1080 60 fps");
									CSipComMode* pScmWithHD1080At60 = new CSipComMode;
									*pScmWithHD1080At60 = rPreferredMode;
								    APIU8 prof = pScmWithHD1080At60->GetH264Profile(cmCapTransmit);
									pScmWithHD1080At60->SetScmToCpHD1080At60(cmCapTransmit);
									if(!IsSupportHighProfile())
										pScmWithHD1080At60->SetH264Profile(H264_Profile_BaseLine,cmCapTransmit);
									else
										pScmWithHD1080At60->SetH264Profile(prof,cmCapTransmit);

									POBJDELETE(pPreferredMedia);
									pPreferredMedia = pScmWithHD1080At60->GetMediaAsCapClass(mediaType, cmCapTransmit);
									pBestCap = GetHighestCommon(directionArr[j], *pPreferredMedia);
									if(pBestCap)
									{
										PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode - 1080 60 fps found best cap as 1080 60fps");
									}
									POBJDELETE(pScmWithHD1080At60);
						    }
						}

						if (pBestCap == NULL && !bIsBfcp){
							pBestCap = GetHighestCommon(directionArr[j], *pPreferredMedia);
						}

						////////////////
						// H263 4cif
						////////////////
						if (bIsVideo && pBestCap != NULL && !(rPreferredMode.IsHdVswInMixMode()))
						{ // PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode Is is not Null  ");
                            if(pBestCap->GetCapCode() == eH264CapCode  && eOppositeDirection == cmCapTransmit)
                            { //PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode H264  ");
                            	int local4CifMpi = rAlternativeCaps.Get4CifMpi();
								if( Get4CifMpi() != -1 &&  local4CifMpi != -1) // remote && local
                                {
                                    //Check if H263 4cif is better?
                                    if (((CBaseVideoCap *)pBestCap)->checkIsh263preffered(bIsCp, m_videoQuality))
                                    {
                                        APIS32 videoBitRate = pBestCap->GetBitRate();
                                        POBJDELETE(pBestCap);
                                        pBestCap = (CH263VideoCap *) CBaseCap::AllocNewCap (eH263CapCode, NULL);
                                        if (pBestCap)
                                        {
                                            pBestCap->SetStruct(cmCapVideo,cmCapTransmit, kRolePeople);
                                            ((CH263VideoCap *)pBestCap)->SetHighestCapForCpFromScmAndCardValues (videoBitRate, m_videoQuality);
                                            ((CH263VideoCap *)pBestCap)->SetBitRate(videoBitRate);
                                        }
                                        else
                                            PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode - pBestCap is NULL");
                                    }
                                }
                            }
							// if receiving best cap is h263 and 4cif is supported by local caps, set transmit direction with 4cif value
                            if (pBestCap && (pBestCap->GetCapCode() == eH263CapCode) && (eOppositeDirection == cmCapTransmit) )
							{//	PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode H263  ");
								int local4CifMpi = rAlternativeCaps.Get4CifMpi();
								if( Get4CifMpi() != -1 &&  local4CifMpi != -1) // remote && local
								{
									CBaseCap* pTmpCap = NULL;
									APIS8 formatMpi = pBestCap->GetFormatMpi(k4Cif);                // save 4cif value (we may need it for rolling back)
									((CH263VideoCap*)pBestCap)->SetFormatMpi(k4Cif, local4CifMpi);	// set 4cif transmit value
									pTmpCap = GetHighestCommon(directionArr[j], *pBestCap);

									if(pTmpCap) // found common cap with 4Cif
									{
										pBestCap->FreeStruct(); // release struct memory before deleting its object
										POBJDELETE(pBestCap);
										pBestCap = pTmpCap; pTmpCap = NULL;
									}
									else  // not found common cap with 4Cif
									{
										((CH263VideoCap*)pBestCap)->SetFormatMpi(k4Cif, formatMpi); // rollback format
									}
								}
							}

						}
					}

					if (pBestCap != NULL)
					{
						bResArr[i][j] = YES;
						pBestCapBuffer = pBestCap->GetAsCapBuffer();
						pBestCap->FreeStruct();
						POBJDELETE(pBestCap);
					}
					else if (bTakeFromScm)
					{
						//PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode bTakeFromScm");
						bResArr[i][j] = YES;
						//restore RtcpFeedback
						if (!bIsBfcp)
						{
						EResult eResOfSet = ((CBaseVideoCap*)pPreferredMedia)->SetRtcpFeedbackMask(commonRtcpFbMask);
			 			if (eResOfSet == kFailure)
							PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode::SetRtcpFeedbackMask failed!!");
						}
						pBestCapBuffer = pPreferredMedia->GetAsCapBuffer();
					}
				}
			}

			POBJDELETE(pPreferredMedia);

			if (bResArr[i][j] && pBestCapBuffer != NULL)
			{
				pBestMode->SetMediaMode(pBestCapBuffer,mediaType,eOppositeDirection,eRole);
				algFound[i] = (CapEnum)pBestCapBuffer->capTypeCode;
				PDELETEA(pBestCapBuffer);
			}
			else if (bIsCp  						// look in the local cap set
				|| (bIsVswFixed && !bIsVideo) 		// IN the case of VSW and video media we try only the SCM
				|| (bIsCop && !(bIsVideo && (eOppositeDirection == cmCapTransmit)))) // Cop video transmit we take intersection of the Scm only.
			{
				int numOfMediaCapSet		= 0;
				capBuffer** pMediaCapList	= NULL;
				BYTE bFound = NO;
				
				rAlternativeCaps.GetMediaCaps(mediaType,&numOfMediaCapSet,&pMediaCapList,eRole);
				
				for (int k = 0; (k < numOfMediaCapSet) && (bFound == NO); k++)
				{
					CBaseCap* pCurCap = rAlternativeCaps.GetCapSet(mediaType, k, eRole);
					CapEnum eCurAlg = pCurCap ? pCurCap->GetCapCode() : eUnknownAlgorithemCapCode;

					if (pCurCap && eCurAlg != eRfc2833DtmfCapCode && eCurAlg != eREDCapCode && pCurCap->IsDirection(eOppositeDirection))
					{
						capBuffer* pBestCapBufferFromCaps= NULL;
						
						if (bIsContent)
						{
							bIs264TipContentMatch = TRUE;

							if (rPreferredMode.IsTIPContentEnableInH264Scm()  && !IsTipCompatibleContentSupported()) //just for TipCompatibility:video&content!
							{
								bIs264TipContentMatch = FALSE;
							}

							CCapSetInfo contentCapInfo = eCurAlg;

							if (IsCapSet(contentCapInfo,eRole) && (bIs264TipContentMatch == TRUE))
							{
							    pBestCap = CBaseVideoCap::AllocNewCap((CapEnum)contentCapInfo,NULL);
							    if (pBestCap)
							    {
							        PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode : Take content from caps");
							        ((CBaseVideoCap*)pBestCap)->SetContent(kRolePresentation, cmCapReceiveAndTransmit);
							        ((CBaseVideoCap*)pBestCap)->SetBitRate(0/*pCurCap->GetBitRate()*/);
							    }
							    else
							        PTRACE(eLevelError,"CSipCaps::FindBestMode - pBestCap is NULL - can not take content from caps");
							}
						}
						else {
							pBestCap = GetHighestCommon(directionArr[j], *pCurCap);
						}

						////////////////
						// H263 4cif
						////////////////
						if (bIsVideo && pBestCap != NULL)
						{
							// if receiving best cap is h263 and 4cif is supported by local caps, set transmit direction with 4cif value
							if(pBestCap->GetCapCode() == eH263CapCode && eOppositeDirection == cmCapTransmit)
							{ PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode best cap found H263" );
								int local4CifMpi = rAlternativeCaps.Get4CifMpi();
								if( Get4CifMpi() != -1 &&  local4CifMpi != -1) // remote && local
								{
									CBaseCap* pTmpCap = NULL;
									APIS8 formatMpi = pBestCap->GetFormatMpi(k4Cif);                // save 4cif value (we may need it for rolling back)
									((CH263VideoCap*)pBestCap)->SetFormatMpi(k4Cif, local4CifMpi);	// set 4cif transmit values
									pTmpCap = GetHighestCommon(directionArr[j], *pBestCap);

									if(pTmpCap) // found common cap with 4Cif
									{
										pBestCap->FreeStruct(); // release struct memory before deleting its object
										POBJDELETE(pBestCap);
										pBestCap = pTmpCap; pTmpCap = NULL;
									}
									else  // not found common cap with 4Cif
									{
										((CH263VideoCap*)pBestCap)->SetFormatMpi(k4Cif, formatMpi); // // rollback format
									}
								}
							}
						}
						////////////////
						////////////////


						if (pBestCap != NULL)
						{
							PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode pBestCap != NULL #3" );
							bResArr[i][j] = YES;
							pBestCapBufferFromCaps = pBestCap->GetAsCapBuffer();
							pBestCap->FreeStruct();
							POBJDELETE(pBestCap);
						}

						if (bResArr[i][j] && pBestCapBufferFromCaps != NULL)
						{
							PTRACE(eLevelInfoNormal,"CSipCaps::FindBestMode pBestCap != NULL #4" );
							bFound = YES;
							pBestMode->SetMediaMode(pBestCapBufferFromCaps,mediaType,eOppositeDirection,eRole);
							algFound[i] = (CapEnum)pBestCapBufferFromCaps->capTypeCode;
							PDELETEA(pBestCapBufferFromCaps);
						}
						PDELETEA(pBestCapBufferFromCaps);
					}

					POBJDELETE(pCurCap);
				}
			}
			PDELETEA(pBestCapBuffer);
		}
	}


	// check if found any media
	for (int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		for (int j = 0; j < numOfDirection; j++)
		{
			bAnyMediaFound |= bResArr[i][j];
		}
	}

	if (bAnyMediaFound)
	{
		//pBestMode->Dump("CSipCaps::FindBestMode: Chosen mode is:\n");
		DWORD isEncrypted = ((CSipComMode&)rPreferredMode).GetIsEncrypted();
		if(isEncrypted == Encryp_On) {
			pTmpBestMode = FindSdesBestMode(rPreferredMode, rAlternativeCaps, (const CSipComMode&)*pBestMode, bIsOffere);
			pTmpBestMode->SetEncryption(isEncrypted, ((CSipComMode&)rPreferredMode).GetIsDisconnectOnEncryptionFailure());
			POBJDELETE(pBestMode);
			pBestMode = pTmpBestMode;
		}
	}
	else
	{
		POBJDELETE(pBestMode);
	}

	if (pBestMode && pBestMode->GetConfMediaType() == eMixAvcSvc && pBestMode->GetMediaType(cmCapVideo, cmCapReceive)!=eSvcCapCode)
	{
//	    FindBestModeForAvcInMixMode(pBestMode);
	    pBestMode->UpdateHdVswForAvcInMixMode();

	}

	//if(pBestMode)
		//pBestMode->Dump("CSipCaps::FindBestMode: pBestMode is:\n", eLevelInfoNormal);

	return pBestMode;
}

///////////////////////////////////////////////////////////////////////////////////
CSipComMode* CSipCaps::FindTargetMode(cmCapDirection eDirection,const CSipComMode& rPreferredMode) const
{
	CSipComMode* pBestMode = new CSipComMode;
	cmCapDirection directionArr[2];
	int numOfDirection	= (eDirection == cmCapReceiveAndTransmit) ? 2 : 1;
	BYTE bAnyMediaFound = NO;
	CMedString str = "CSipCaps::FindTargetMode: Chosen mode is:\n";
	directionArr[0]	= (eDirection == cmCapTransmit) ? cmCapTransmit : cmCapReceive;
	directionArr[1] = (eDirection == cmCapReceiveAndTransmit) ? cmCapTransmit : (cmCapDirection) 0;
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for(int i = 0; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		for (int j = 0; j < numOfDirection; j++)
		{
			CapEnum eAlg = (CapEnum) rPreferredMode.GetMediaType(mediaType, directionArr[j],eRole);
			CCapSetInfo capInfo = eAlg;

			if ( IsCapSet(eAlg) )
			{
				CBaseCap* pPreferredMedia = rPreferredMode.GetMediaAsCapClass(mediaType, directionArr[j],eRole);

				if ( pPreferredMedia )
				{
					CBaseCap* pBestMedia = GetHighestCommon(directionArr[j],*pPreferredMedia);
					if ( NULL != pBestMedia )
					{
						CCapSetInfo capInfo = pBestMedia->GetCapCode();
						str << "* " << GetTypeStr(mediaType) << " " << GetDirectionStr(directionArr[j]) << " " << (const char*) capInfo << " \n";
						pBestMode->SetMediaMode(pBestMedia,mediaType,directionArr[j],eRole);
						pBestMedia->FreeStruct();
						POBJDELETE(pBestMedia);
						bAnyMediaFound = YES;
					}
					else
					{
						PTRACE(eLevelInfoNormal, "CSipCaps::FindTargetMode. No highest common choice.");
					}
				}

				POBJDELETE(pPreferredMedia);
			}
			else // take first cap from capabilities
			{
				const capBuffer* pFirstCap = GetCapSetAsCapBuffer(mediaType,0,eRole);
				if ( pFirstCap )
				{
					CCapSetInfo capInfo = (CapEnum)pFirstCap->capTypeCode;
					str << "* " << GetTypeStr(mediaType) << " " << GetDirectionStr(directionArr[j]) << " " << (const char*)capInfo << " \n";
					pBestMode->SetMediaMode(pFirstCap,mediaType, directionArr[j],eRole);
					bAnyMediaFound = YES;
				}
			}
		}
	}

	if ( bAnyMediaFound )
	{
		PTRACE(eLevelInfoNormal, str.GetString());
	}
	else
	{
		POBJDELETE(pBestMode);
	}

	return pBestMode;
}


//////////////////////////////////////////////////////////////////////////
void CSipCaps::CompleteDataFromOtherCap(const CSipCaps& defaultParams)
{
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	BYTE bValuesToCheck;
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for (int j = 0; j < MAX_SIP_MEDIA_TYPES ; j++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[j], mediaType, eRole);
		GetMediaCaps(mediaType,&numOfMediaCapSet,&pMediaCapList,eRole);

		for(int i = 0; i < numOfMediaCapSet; i++)
		{
			CBaseCap* pCurCap = GetCapSet(mediaType,i,eRole);
			if (pCurCap)
			{
				//no check the rate because we are accepting the rate 0 as HOLD
				bValuesToCheck = kCapCode | kRoleLabel | kFrameRate;

				//but add the check if rate is -1, indication from card that
				//rate does not specified by remote side
				if (pCurCap->GetBitRate() == -1)
				{
					bValuesToCheck = bValuesToCheck | kBitRate;
				}

				BYTE bIsValid = pCurCap->CheckValidationAndSetValidValuesIfNeeded(bValuesToCheck);
				if (bIsValid == NO)
				{
					CCapSetInfo capInfo = pCurCap->GetCapCode();
					CBaseCap* pDefaultCap = defaultParams.GetCapSet(capInfo,0,eRole);

					if (pDefaultCap)
					{
						pCurCap->CheckValidationAndSetValidValuesIfNeeded(kCapCode|kRoleLabel|kFrameRate|kBitRate,YES,pDefaultCap);
					}
					POBJDELETE(pDefaultCap);
				}
			}
			POBJDELETE(pCurCap);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void CSipCaps::Complete2013DataFromOtherCapForMsSvc(const  CSipCaps& defaultParams,DWORD rmtMsftRxBitRate)
{
	PTRACE(eLevelInfoNormal, "CSipCaps::Complete2013DataFromOtherCapForMsSvc. ");
	//BYTE bRetVal = NO;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;
	//DWORD dwNewBitRate = GetSystemCfgFlagInt<DWORD>(CFG_KEY_IP_MOBILE_PHONE_RATE);
	CBaseCap* pCap = NULL;
	pCap = defaultParams.GetCapSet(eMsSvcCapCode,0,kRolePeople);
	if(pCap == NULL)
	{
		PTRACE(eLevelError, "CSipCaps::Complete2013DataFromOtherCapForMsSvc.no MS SVC cap on default caps ");
		return;
	}
	if(rmtMsftRxBitRate == 0 || rmtMsftRxBitRate == 0xFFFFFFFF)
	{
		PTRACE(eLevelError, "CSipCaps::Complete2013DataFromOtherCapForMsSvc.no remote bw rx rate ");
		POBJDELETE (pCap);
		return;
	}
	PTRACE2INT(eLevelError, "DBG CSipCaps::Complete2013DataFromOtherCapForMsSvc. IsMedia(cmCapVideo) = ", IsMedia(cmCapVideo));
	if ( IsMedia(cmCapVideo) )
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
		for ( int i = 0; i < numOfVideoMediaCapSet; i++)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*) GetCapSet(cmCapVideo, i);
			if (pVideoCap && pVideoCap->GetCapCode() == (CapEnum)eMsSvcCapCode)
			{
				 PTRACE(eLevelError, "CSipCaps::Complete2013DataFromOtherCapForMsSvc.remote has ms svc caps ");
				MsSvcVideoModeDetails MsSvcDetails;
				CMsSvcVideoMode* MsSvcVidMode = new CMsSvcVideoMode();
				MsSvcVidMode->GetMsSvcVideoParamsByRate(MsSvcDetails,(rmtMsftRxBitRate*100),eLasth264VideoMode,E_VIDEO_RES_ASPECT_RATIO_DUMMY);
				if(MsSvcDetails.videoModeType != eInvalidModeType)
				{
					 PTRACE(eLevelError, "CSipCaps::Complete2013DataFromOtherCapForMsSvc.ms svc found ");
					 ((CMsSvcVideoCap *)pVideoCap)->SetAspectRatio(E_VIDEO_RES_ASPECT_RATIO_DUMMY);
					 ((CMsSvcVideoCap *)pVideoCap)->SetBitRate(rmtMsftRxBitRate);
					 ((CMsSvcVideoCap *)pVideoCap)->SetHeight(MsSvcDetails.maxHeight);
					 ((CMsSvcVideoCap *)pVideoCap)->SetWidth(MsSvcDetails.maxWidth);
					 ((CMsSvcVideoCap *)pVideoCap)->SetMaxBitRate(rmtMsftRxBitRate);
					 ((CMsSvcVideoCap *)pVideoCap)->SetPacketizationMode(1);// ? noa to check
					 ((CMsSvcVideoCap *)pVideoCap)->SetMaxFrameRate(MsSvcDetails.maxFrameRate);
					 ((CMsSvcVideoCap *)pVideoCap)->SetMinBitRate(MsSvcDetails.minBitRate/100);
					 ((CMsSvcVideoCap *)pVideoCap)->SetMaxPixelNum(MsSvcDetails.maxNumOfPixels);

				}
				else
				{
					PTRACE(eLevelError, "CSipCaps::Complete2013DataFromOtherCapForMsSvc.no remote mode found taking local caps ");
					((CMsSvcVideoCap *)pVideoCap)->SetBitRate(rmtMsftRxBitRate);
					((CMsSvcVideoCap *)pVideoCap)->SetMaxBitRate(rmtMsftRxBitRate);

					 ((CMsSvcVideoCap *)pVideoCap)->SetAspectRatio(((CMsSvcVideoCap *)pCap)->GetAspectRatio());
					 ((CMsSvcVideoCap *)pVideoCap)->SetHeight(((CMsSvcVideoCap *)pCap)->GetHeight());
					 ((CMsSvcVideoCap *)pVideoCap)->SetWidth(((CMsSvcVideoCap *)pCap)->GetWidth());
					 ((CMsSvcVideoCap *)pVideoCap)->SetPacketizationMode(((CMsSvcVideoCap *)pCap)->GetPacketizationMode());
					 ((CMsSvcVideoCap *)pVideoCap)->SetMaxFrameRate(((CMsSvcVideoCap *)pCap)->GetMaxFrameRate());
					 ((CMsSvcVideoCap *)pVideoCap)->SetMinBitRate(((CMsSvcVideoCap *)pCap)->GetMinBitRate());
					 ((CMsSvcVideoCap *)pVideoCap)->SetMaxPixelNum( ((CMsSvcVideoCap *)pCap)->GetMaxPixel() );


				}
				POBJDELETE(MsSvcVidMode);

			}

			POBJDELETE(pVideoCap);
		}
	}
	else
	{
		PTRACE(eLevelError, "CSipCaps::Complete2013DataFromOtherCapForMsSvc.no remote mode found for video ");
	}

	POBJDELETE (pCap);


}


/////////////////////////////////////////////////////////////////////////
DWORD CSipCaps::GetMaxVideoBitRate(const CCapSetInfo& capInfo, ERoleLabel eRole) const
{
	DWORD maxRate = 0;
	if (capInfo.IsType(cmCapVideo))
	{
		CBaseVideoCap* pCap = (CBaseVideoCap *)GetCapSet(capInfo, 0, eRole);
		if (pCap)
			maxRate = pCap->GetBitRate();
		POBJDELETE(pCap);
	}
	return maxRate;
}

/////////////////////////////////////////////////////////////////////////
DWORD CSipCaps::GetMaxVideoBitRate(cmCapDirection eDirection, ERoleLabel eRole) const
{
	DWORD maxRate = 0;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;

	if (IsMedia(cmCapVideo,eDirection,eRole))
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo,eDirection,eRole);
		for (int i = 0; (i < numOfVideoMediaCapSet); i++)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*)GetCapSet(cmCapVideo, i,eRole);

			if (pVideoCap)
			{
				if(pVideoCap->GetBitRate() >= 0)
					maxRate = max(maxRate, (DWORD)pVideoCap->GetBitRate());
				else
					PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetMaxVideoBitRate - negative bit rate. Cap code : ", pVideoCap->GetCapCode());
			}
			POBJDELETE(pVideoCap);
		}
	}
	return maxRate;
}

///////////////////////////////////////////////////////////////////////////////////
int	CSipCaps::CalcCapBuffersSize(cmCapDirection eDirection, BYTE bOperUse, BOOL fAddMediaLine) const
{
	WORD totalSize = 0;
	WORD totalIceSize = 0;

	cmCapDataType mediaType;
	ERoleLabel eRole;

	for(int i = kMediaLineInternalTypeNone+1; i < kMediaLineInternalTypeLast; i++) {

		GetMediaDataTypeAndRole((eMediaLineInternalType)i, mediaType, eRole);

		totalSize += CalcCapBuffersSize(mediaType, eDirection, bOperUse, fAddMediaLine,eRole);

		if(mediaType==cmCapAudio)
			totalIceSize += CalcIceCapBufferSize(eAudioSession,eDirection);
		else if((mediaType==cmCapVideo) && (eRole==kRolePeople))
			totalIceSize += CalcIceCapBufferSize(eVideoSession,eDirection);
		else if(mediaType==cmCapData)
			totalIceSize += CalcIceCapBufferSize(eDataSession,eDirection);
	}

	return totalSize+totalIceSize;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int	CSipCaps::CalcSdesOnlyCapBuffersSize(cmCapDirection eDirection, BYTE bOperUse, BOOL fAddMediaLine) const
{
	WORD totalSize = 0;
	cmCapDataType mediaType;
	ERoleLabel eRole;

	for(int i = kMediaLineInternalTypeNone+1; i < kMediaLineInternalTypeLast; i++) {

		GetMediaDataTypeAndRole((eMediaLineInternalType)i, mediaType, eRole);

		totalSize += CalcSdesOnlyCapBuffersSize(mediaType, eDirection, bOperUse, fAddMediaLine,eRole);

	}

	return totalSize;
}

///////////////////////////////////////////////////////////////////////////////////
int	CSipCaps::CalcCapBuffersSize(cmCapDataType eType, cmCapDirection eDirection, BYTE bOperUse, BOOL fAddMediaLine, ERoleLabel eRole) const
{
	WORD totalSize = 0;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eType,&numOfMediaCapSet,&pMediaCapList,eRole);

	for (int j = 0; j < numOfMediaCapSet; j++)
	{
		CBaseCap* pCap = GetCapSet(eType,j,eRole);
		if ( pCap && (eDirection == cmCapReceiveAndTransmit || pCap->IsDirection(eDirection)) )
		{
			int capLen = pMediaCapList[j]->capLength;

			if ( bOperUse )
			{
				if ( (eType == cmCapAudio ||
					pMediaCapList[j]->capTypeCode == ePeopleContentCapCode ||
					pMediaCapList[j]->capTypeCode == eRoleLabelCapCode) && (pMediaCapList[j]->capTypeCode != eSdesCapCode) )

				{
					capLen -= sizeof(APIS32);
				}
			}
			if (fAddMediaLine && !totalSize)
				totalSize = sizeof(sipMediaLineBaseSt);
			totalSize += (sizeof(capBufferBase) + capLen);
		}
		POBJDELETE(pCap);
	}
	return totalSize;
}
///////////////////////////////////////////////////////////////////////////////////////
int	CSipCaps::CalcSdesOnlyCapBuffersSize(cmCapDataType eType, cmCapDirection eDirection, BYTE bOperUse, BOOL fAddMediaLine, ERoleLabel eRole) const
{
	WORD totalSize = 0;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	GetSdesMediaCaps(eType,&numOfMediaCapSet,&pMediaCapList,eRole);

	for (int j = 0; j < numOfMediaCapSet; j++)
	{
		CBaseCap* pCap = GetSdesCapSet(eType,j,eRole);
		if ( pCap && (eDirection == cmCapReceiveAndTransmit || pCap->IsDirection(eDirection)) )
		{
			int capLen = pMediaCapList[j]->capLength;

			if (fAddMediaLine && !totalSize)
				totalSize = sizeof(sipMediaLineBaseSt);
			totalSize += (sizeof(capBufferBase) + capLen);
		}
		POBJDELETE(pCap);
	}
	return totalSize;
}
///////////////////////////////////////////////////////////////////////////////////
int	CSipCaps::CalcCapBuffersSizeWithSdes(cmCapDirection eDirection, BYTE bOperUse, BOOL fAddMediaLine) const
{
	WORD totalSize = 0;
	WORD totalIceSize = 0;

	cmCapDataType mediaType;
	ERoleLabel eRole;
	for(int i = kMediaLineInternalTypeNone+1; i < kMediaLineInternalTypeLast; i++) {
		GetMediaDataTypeAndRole((eMediaLineInternalType)i, mediaType, eRole);
		totalSize += CalcCapBuffersSizeWithSdes(mediaType, eDirection, bOperUse, fAddMediaLine, eRole);

		if(mediaType==cmCapAudio)
			totalIceSize += CalcIceCapBufferSize(eAudioSession,eDirection);
		else if((mediaType==cmCapVideo) && (eRole==kRolePeople))
			totalIceSize += CalcIceCapBufferSize(eVideoSession,eDirection);
		else if(mediaType==cmCapData)
			totalIceSize += CalcIceCapBufferSize(eDataSession,eDirection);
	}

	return totalSize+totalIceSize;
}

///////////////////////////////////////////////////////////////////////////////////
int	CSipCaps::CalcCapBuffersSizeWithSdes(cmCapDataType eType, cmCapDirection eDirection, BYTE bOperUse, BOOL fAddMediaLine, ERoleLabel eRole) const
{
	WORD totalSize 				= 0;
	int numOfMediaCapSet 		= 0;
	int numOfSdesMediaCapSet 	= 0;
	int capLen					= 0;
	capBuffer** pMediaCapList	= NULL;
	capBuffer** pSdesMediaCapList	= NULL;
	GetMediaCaps(eType,&numOfMediaCapSet,&pMediaCapList,eRole);
	GetSdesMediaCaps(eType,&numOfSdesMediaCapSet,&pSdesMediaCapList,eRole);

	for (int j = 0; j < numOfMediaCapSet; j++)
	{
		CBaseCap* pCap = GetCapSet(eType,j,eRole);
		if ( pCap && (eDirection == cmCapReceiveAndTransmit || pCap->IsDirection(eDirection)) )
		{
			capLen = pMediaCapList[j]->capLength;
			if ( bOperUse )
			{
				if ( (eType == cmCapAudio ||
					pMediaCapList[j]->capTypeCode == ePeopleContentCapCode ||
					pMediaCapList[j]->capTypeCode == eRoleLabelCapCode) && (pMediaCapList[j]->capTypeCode != eSdesCapCode) )
				{
					capLen -= sizeof(APIS32);
				}
			}
			if (fAddMediaLine && !totalSize)
				totalSize = sizeof(sipMediaLineBaseSt);
			totalSize += (sizeof(capBufferBase) + capLen);
		}
		POBJDELETE(pCap);
	}

	for (int j = 0; j < numOfSdesMediaCapSet; j++)
	{
		CSdesCap* pCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)pSdesMediaCapList[j]->capTypeCode,pSdesMediaCapList[j]->dataCap);
		if (pCap && (eDirection == cmCapReceiveAndTransmit || pCap->IsDirection(eDirection)))
		{
			capLen = pCap->SizeOf();

			if (fAddMediaLine && !totalSize)
				totalSize = sizeof(sipMediaLineBaseSt);

			totalSize += (sizeof(capBufferBase) + capLen);
		}
		POBJDELETE(pCap);
	}

	return totalSize;
}
///////////////////////////////////////////////////////////////////////////////////
int CSipCaps::CalcIceCapBufferSize(ICESessionsTypes SessionType, cmCapDirection eDirection) const
{
	int numOfMediaICECapSet = 0;
	capBuffer** pMediaICECapList  = NULL;
	WORD totalSize = 0;

	GetMediaIceCaps(SessionType,&numOfMediaICECapSet,&pMediaICECapList);

	for (int j = 0; j < numOfMediaICECapSet; j++)
	{
		CBaseCap* pCap = GetIceCapSet(SessionType,j);
		if ( pCap && (eDirection == cmCapReceiveAndTransmit || pCap->IsDirection(eDirection)) )
		{
			if(pMediaICECapList[j])
			{
				int capLen = pMediaICECapList[j]->capLength;
				totalSize = (sizeof(capBufferBase) + capLen);
			}
		}
		POBJDELETE(pCap);
	}

	return totalSize;




}

///////////////////////////////////////////////////////////////////////////////////
void CSipCaps::CapSetBuffer(BYTE bSetOppositeDirection, CBaseCap *pCap, CCapSetInfo &capInfo, char *buffer, int capPos, int capEnum, int len) const
{
	((capBufferBase*)(buffer+capPos))->capTypeCode = capEnum;
	((capBufferBase*)(buffer+capPos))->capLength   = len;

	if (bSetOppositeDirection && pCap->IsDirection(cmCapReceiveAndTransmit) == NO)
	{
		cmCapDirection eOppositeDirection = pCap->IsDirection(cmCapReceive)? cmCapTransmit: cmCapReceive;
		CBaseCap* pTemp = CBaseCap::AllocNewCap((CapEnum)capInfo,((capBuffer*)(buffer+capPos))->dataCap);

		if (pTemp)
		{
			pTemp->SetDirection(eOppositeDirection);
		}

		POBJDELETE(pTemp);
	}
}
///////////////////////////////////////////////////////////////////////////////////
int	CSipCaps::CopyCapBuffersToBuffer(BYTE* buffer,int bufSize,int* pNumOfCaps,cmCapDirection eDirection,BYTE bSetOppositeDirection,BYTE bOperUse, int bufSdesSize, int* pNumOfSdesCaps,BOOL fAddMediaLine) const

{
	int totalSize	= CalcCapBuffersSize(eDirection, bOperUse, fAddMediaLine);
	int totalSizeSdes	= CalcSdesOnlyCapBuffersSize(eDirection, bOperUse, fAddMediaLine);
	PTRACE2INT(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer totalSizeSdes ",totalSizeSdes);
	int	offsetWrite = 0;
	*pNumOfCaps	= 0;

	int	capPosAudio = 0;
	int	capPosVideo = 0;
	int	capPosData	= 0;
	int	capPosContent = 0;
	int	capPosBfcp	= 0;
	int	capPosSaveAudio = 0;
	int	capPosSaveVideo = 0;
	int	capPosSaveData	= 0;
	int	capPosSaveContent = 0;
	int	capPosSaveBfcp	= 0;
	char bufAudio[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufVideo[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufData[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufContent[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufBfcp[SIP_MEDIA_LINE_BUFFER_SIZE];
	int index = 0;
	sipMediaLineSt *pMediaLineAudio = (sipMediaLineSt *) &bufAudio[0];
	sipMediaLineSt *pMediaLineVideo = (sipMediaLineSt *) &bufVideo[0];
	sipMediaLineSt *pMediaLineData = (sipMediaLineSt *) &bufData[0];
	sipMediaLineSt *pMediaLineContent = (sipMediaLineSt *) &bufContent[0];
	sipMediaLineSt *pMediaLineBfcp = (sipMediaLineSt *) &bufBfcp[0];

	memset(bufAudio, 0, sizeof(bufAudio));
	memset(bufVideo, 0, sizeof(bufVideo));
	memset(bufData, 0, sizeof(bufData));
	memset(bufContent, 0, sizeof(bufContent));
	memset(bufBfcp, 0, sizeof(bufBfcp));

	int	capSdesPosAudio = 0;
	int	capSdesPosVideo = 0;
	int	capSdesPosData	= 0;
	int	capSdesPosContent = 0;

	int	capSdesPosSaveAudio = 0;
	int	capSdesPosSaveVideo = 0;
	int	capSdesPosSaveData	= 0;
	int	capSdesPosSaveContent = 0;

	char bufSdesAudio[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufSdesVideo[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufSdesData[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufSdesContent[SIP_MEDIA_LINE_BUFFER_SIZE];

	memset(bufSdesAudio, 0, sizeof(bufSdesAudio));
	memset(bufSdesVideo, 0, sizeof(bufSdesVideo));
	memset(bufSdesData, 0, sizeof(bufSdesData));
	memset(bufSdesContent, 0, sizeof(bufSdesContent));

	if ((totalSize + totalSizeSdes) <= (bufSize + bufSdesSize))
	{
		int numOfMediaCapSet = 0;
		capBuffer** pMediaCapList	= NULL;
		WORD curCapBufferSize = 0;
		cmCapDataType mediaType;
		ERoleLabel eRole;
		for(int i = kMediaLineInternalTypeNone+1; i < kMediaLineInternalTypeLast; i++)
		{
			GetMediaDataTypeAndRole((eMediaLineInternalType)i, mediaType, eRole);
			GetMediaCaps(mediaType, &numOfMediaCapSet, &pMediaCapList,eRole);
			for (int j = 0; j < numOfMediaCapSet; j++)
			{
				CCapSetInfo capInfo = (CapEnum)pMediaCapList[j]->capTypeCode;
				CBaseCap*	pCap	= GetCapSet(mediaType,j,eRole);
				int	capEnum = pMediaCapList[j]->capTypeCode;
				int	len	= pMediaCapList[j]->capLength;

				if ( pCap &&
						(eDirection == cmCapReceiveAndTransmit || pCap->IsDirection(eDirection)) )
				{
					if (bOperUse)
					{
						int lenToRemove = 0;
						if (capInfo.GetSipCapType() == cmCapAudio ||
							 (CapEnum)capInfo == ePeopleContentCapCode || (CapEnum)capInfo == eRoleLabelCapCode)
						{
							lenToRemove = sizeof(APIS32);
						}

						capEnum = capInfo.GetIpCapCode();
						len	-= lenToRemove;
					}

					curCapBufferSize = sizeof(capBufferBase) + len;

					capPosSaveAudio = capPosAudio;
					capPosSaveVideo = capPosVideo;
					capPosSaveData = capPosData;
					capPosSaveContent = capPosContent;
					capPosSaveBfcp = capPosBfcp;

					if (fAddMediaLine) {
						if (mediaType == cmCapAudio) {
							if (AddCapInMediaLine(pMediaCapList[j], curCapBufferSize,
									pMediaLineAudio, SIP_MEDIA_LINE_BUFFER_SIZE, capPosAudio))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineAudio->caps, capPosSaveAudio, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Audio");
						}
						else if ((mediaType == cmCapVideo) && (eRole == kRolePeople)) {
							if (AddCapInMediaLine(pMediaCapList[j], curCapBufferSize,
									pMediaLineVideo, SIP_MEDIA_LINE_BUFFER_SIZE, capPosVideo))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineVideo->caps, capPosSaveVideo, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Video");
						}
						else if (mediaType == cmCapData) {
							if (AddCapInMediaLine(pMediaCapList[j], curCapBufferSize,
									pMediaLineData, SIP_MEDIA_LINE_BUFFER_SIZE, capPosData))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineData->caps, capPosSaveData, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Data");
						}
						else if ((mediaType == cmCapVideo) && (eRole == kRolePresentation)) {
							if (AddCapInMediaLine(pMediaCapList[j], curCapBufferSize,
									pMediaLineContent, SIP_MEDIA_LINE_BUFFER_SIZE, capPosContent))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineContent->caps, capPosSaveContent, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Content");
						}
						else if (mediaType == cmCapBfcp) {
							if (AddCapInMediaLine(pMediaCapList[j], curCapBufferSize,
									pMediaLineBfcp, SIP_MEDIA_LINE_BUFFER_SIZE, capPosBfcp))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineBfcp->caps, capPosSaveBfcp, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Bfcp");
						}
					}
					else {
						if (mediaType == cmCapAudio) {
							if (AddCapInBuffer(pMediaCapList[j], curCapBufferSize,
									bufAudio, SIP_MEDIA_LINE_BUFFER_SIZE, capPosAudio))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufAudio, capPosSaveAudio, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Audio");
						}
						else if ((mediaType == cmCapVideo) && (eRole == kRolePeople)) {
							if (AddCapInBuffer(pMediaCapList[j], curCapBufferSize,
									bufVideo, SIP_MEDIA_LINE_BUFFER_SIZE, capPosVideo))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufVideo, capPosSaveVideo, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Video");
						}
						else if (mediaType == cmCapData) {
							if (AddCapInBuffer(pMediaCapList[j], curCapBufferSize,
									bufData, SIP_MEDIA_LINE_BUFFER_SIZE, capPosData))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufData, capPosSaveData, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Data");
						}
						else if ((mediaType == cmCapVideo) && (eRole == kRolePresentation)) {
							if (AddCapInBuffer(pMediaCapList[j], curCapBufferSize,
									bufContent, SIP_MEDIA_LINE_BUFFER_SIZE, capPosContent))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufContent, capPosSaveContent, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Content");
						}
						else if (mediaType == cmCapBfcp) {
							if (AddCapInBuffer(pMediaCapList[j], curCapBufferSize,
									bufBfcp, SIP_MEDIA_LINE_BUFFER_SIZE, capPosBfcp))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufBfcp, capPosSaveBfcp, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Bfcp");
						}
					}
					(*pNumOfCaps)++;
				}
				POBJDELETE(pCap);
			}

			/////////////////////////////////////////////////////////////////////////////////
		}

		curCapBufferSize = 0;
		for(int i = kMediaLineInternalTypeNone+1; i < kMediaLineInternalTypeLast; i++)
		{
			capBuffer** pSdesMediaCapList	= NULL;
			int numOfSdesMediaCapSet 		= 0;

			GetMediaDataTypeAndRole((eMediaLineInternalType)i, mediaType, eRole);
			GetSdesMediaCaps(mediaType, &numOfSdesMediaCapSet, &pSdesMediaCapList,eRole);
			for (int j = 0; j < numOfSdesMediaCapSet; j++)
			{
				CCapSetInfo capInfo = (CapEnum)pSdesMediaCapList[j]->capTypeCode;
				CBaseCap*	pCap	= GetSdesCapSet(mediaType,j,eRole);
				int	capEnum = pSdesMediaCapList[j]->capTypeCode;
				int	len	= pSdesMediaCapList[j]->capLength;

				if ( pCap &&
						(eDirection == cmCapReceiveAndTransmit || pCap->IsDirection(eDirection)) )
				{
					if (bOperUse)
					{
						int lenToRemove = 0;
						if (capInfo.GetSipCapType() == cmCapAudio ||
								(CapEnum)capInfo == ePeopleContentCapCode || (CapEnum)capInfo == eRoleLabelCapCode)
						{
							lenToRemove = sizeof(APIS32);
						}

						capEnum = capInfo.GetIpCapCode();
						len	-= lenToRemove;
					}

					curCapBufferSize = sizeof(capBufferBase) + len;

					capSdesPosSaveAudio = capSdesPosAudio;
					capSdesPosSaveVideo = capSdesPosVideo;
					capSdesPosSaveData = capSdesPosData;
					capSdesPosSaveContent = capSdesPosContent;

					if (mediaType == cmCapAudio) {
						if (AddCapInBuffer(pSdesMediaCapList[j], curCapBufferSize,
								bufSdesAudio, SIP_MEDIA_LINE_BUFFER_SIZE, capSdesPosAudio))
							CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufSdesAudio, capSdesPosSaveAudio, capEnum, len);
						else
							PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Audio SDES");
					}
					else if ((mediaType == cmCapVideo) && (eRole == kRolePeople)) {
						if (AddCapInBuffer(pSdesMediaCapList[j], curCapBufferSize,
								bufSdesVideo, SIP_MEDIA_LINE_BUFFER_SIZE, capSdesPosVideo))
							CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufSdesVideo, capSdesPosSaveVideo, capEnum, len);
						else
							PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Video SDES");
					}
					else if (mediaType == cmCapData) {
						if (AddCapInBuffer(pSdesMediaCapList[j], curCapBufferSize,
								bufSdesData, SIP_MEDIA_LINE_BUFFER_SIZE, capSdesPosData))
							CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufSdesData, capSdesPosSaveData, capEnum, len);
						else
							PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Data SDES");
					}
					else if ((mediaType == cmCapVideo) && (eRole == kRolePresentation)) {
						if (AddCapInBuffer(pSdesMediaCapList[j], curCapBufferSize,
								bufSdesContent, SIP_MEDIA_LINE_BUFFER_SIZE, capSdesPosContent))
							CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufSdesContent, capSdesPosSaveContent, capEnum, len);
						else
							PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer AddCapInMediaLine fail - ", "Content SDES");
					}
					(*pNumOfSdesCaps)++;
				}
				POBJDELETE(pCap);
			}
		}

		if (fAddMediaLine) {
			capBuffer** pSdesMediaCapList	= NULL;
			int numOfSdesAudioMediaCapSet 		= 0;
			int numOfSdesVideoMediaCapSet 		= 0;
			int numOfSdesDataMediaCapSet 		= 0;
			int numOfSdesContentMediaCapSet		= 0;
			GetSdesMediaCaps(cmCapAudio,&numOfSdesAudioMediaCapSet,&pSdesMediaCapList);
			GetSdesMediaCaps(cmCapVideo,&numOfSdesVideoMediaCapSet,&pSdesMediaCapList);
			GetSdesMediaCaps(cmCapData,&numOfSdesDataMediaCapSet,&pSdesMediaCapList);
			GetSdesMediaCaps(cmCapVideo,&numOfSdesContentMediaCapSet,&pSdesMediaCapList,kRolePresentation);

			if (capPosAudio) {
				pMediaLineAudio->index = index;
				pMediaLineAudio->type = eMediaLineTypeAudio;

				if(numOfSdesAudioMediaCapSet) {
					pMediaLineAudio->subType = eMediaLineSubTypeRtpSavp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer- SAVP for Audio");
				} else {
					pMediaLineAudio->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer- AVP for Audio");
				}
				pMediaLineAudio->internalType = kMediaLineInternalTypeAudio;
				index++;
				memcpy(buffer + offsetWrite, bufAudio, sizeof(sipMediaLineBaseSt) + capPosAudio);
				offsetWrite += sizeof(sipMediaLineBaseSt) + capPosAudio;
			}
			if (capPosVideo) {
				pMediaLineVideo->index = index;
				pMediaLineVideo->type = eMediaLineTypeVideo;
				if(numOfSdesVideoMediaCapSet) {
					pMediaLineVideo->subType = eMediaLineSubTypeRtpSavp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer- SAVP for Video");
				} else {
					pMediaLineVideo->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer- AVP for Video");
				}
				pMediaLineVideo->internalType = kMediaLineInternalTypeVideo;
				index++;
				memcpy(buffer + offsetWrite, bufVideo, sizeof(sipMediaLineBaseSt) + capPosVideo);
				offsetWrite += sizeof(sipMediaLineBaseSt) + capPosVideo;
			}
			if (capPosData) {
				pMediaLineData->index = index;
				pMediaLineData->type = eMediaLineTypeApplication;
				if(numOfSdesDataMediaCapSet) {
					pMediaLineData->subType = eMediaLineSubTypeRtpSavp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer- SAVP for Data");
				} else {
					pMediaLineData->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer- AVP for Data");
				}
				pMediaLineData->internalType = kMediaLineInternalTypeFecc;
				index++;
				memcpy(buffer + offsetWrite, bufData, sizeof(sipMediaLineBaseSt) + capPosData);
				offsetWrite += sizeof(sipMediaLineBaseSt) + capPosData;
			}
			if (capPosContent) {
				pMediaLineContent->index = index;
				pMediaLineContent->type = eMediaLineTypeVideo;
				if(numOfSdesContentMediaCapSet) {
					pMediaLineContent->subType = eMediaLineSubTypeRtpSavp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer- SAVP for Content");
				} else {
					pMediaLineContent->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer- AVP for Content");
				}
				pMediaLineContent->internalType = kMediaLineInternalTypeContent;
				index++;
				memcpy(buffer + offsetWrite, bufContent, sizeof(sipMediaLineBaseSt) + capPosContent);
				offsetWrite += sizeof(sipMediaLineBaseSt) + capPosContent;
			}
			if (capPosBfcp) {
				pMediaLineBfcp->index = index;
				pMediaLineBfcp->type = eMediaLineTypeApplication;
				pMediaLineBfcp->subType = eMediaLineSubTypeUdpBfcp; //eMediaLineSubTypeTcpBfcp; udp is default (this will be changed later anyway)
				pMediaLineBfcp->internalType = kMediaLineInternalTypeBfcp;
				index++;
				memcpy(buffer + offsetWrite, bufBfcp, sizeof(sipMediaLineBaseSt) + capPosBfcp);
				offsetWrite += sizeof(sipMediaLineBaseSt) + capPosBfcp;
			}
		}
		else {
			if (capPosAudio) {
				memcpy(buffer + offsetWrite, bufAudio, capPosAudio);
				offsetWrite += capPosAudio;
				if (capSdesPosAudio) {
					memcpy(buffer + offsetWrite, bufSdesAudio, capSdesPosAudio);
					offsetWrite += capSdesPosAudio;

							}
				}


			if (capPosVideo) {
				memcpy(buffer + offsetWrite, bufVideo, capPosVideo);
				offsetWrite += capPosVideo;
				if (capSdesPosVideo) {
					memcpy(buffer + offsetWrite, bufSdesVideo, capSdesPosVideo);
					offsetWrite += capSdesPosVideo;

				}

			}
			if (capPosData) {
				PTRACE2INT(eLevelInfoNormal,"CSipCaps::CopyCapBuffersToBuffer capPosData",capPosData );
				memcpy(buffer + offsetWrite, bufData, capPosData);
				offsetWrite += capPosData;


			}
			if (capSdesPosData) {
				memcpy(buffer + offsetWrite, bufSdesData, capSdesPosData);
				offsetWrite += capSdesPosData;
			}
			if (capPosContent) {
				memcpy(buffer + offsetWrite, bufContent, capPosContent);
				offsetWrite += capPosContent;
				if (capSdesPosContent) {
					memcpy(buffer + offsetWrite, bufSdesContent, capSdesPosContent);
									offsetWrite += capSdesPosContent;

				}
			}
			if (capPosBfcp) {
				memcpy(buffer + offsetWrite, bufBfcp, capPosBfcp);
				offsetWrite += capPosBfcp;
			}

		}


	}

	if (offsetWrite != totalSize )
	{
		PTRACE(eLevelError, "CSipCaps::CopyCapBuffersToBuffer offsetWrite != totalSize");
	}

	return offsetWrite;
}

/////////////////////////////////////////////////////////////////////
int	CSipCaps::CopyCapBuffersWithSdesToBuffer(BYTE* buffer,int bufSize,int* pNumOfCaps,cmCapDirection eDirection,BYTE bSetOppositeDirection,BYTE bOperUse,BOOL fAddMediaLine) const
{
	int totalSize	= CalcCapBuffersSizeWithSdes(eDirection, bOperUse, fAddMediaLine);
	int	offsetWrite = 0;
	*pNumOfCaps	= 0;
	int	capPosAudio = 0;
	int	capPosVideo = 0;
	int	capPosData	= 0;
	int	capPosContent = 0;
	int	capPosBfcp	= 0;
	int	capPosSaveAudio = 0;
	int	capPosSaveVideo = 0;
	int	capPosSaveData	= 0;
	int	capPosSaveContent = 0;
	int	capPosSaveBfcp	= 0;
	char bufAudio[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufVideo[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufData[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufContent[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufBfcp[SIP_MEDIA_LINE_BUFFER_SIZE];
	int index = 0;
	BOOL bIsAudioSdesEnable = FALSE;
	BOOL bIsVideoSdesEnable = FALSE;
	BOOL bIsDataSdesEnable = FALSE;
	BOOL bIsContentSdesEnable = FALSE;

	sipMediaLineSt *pMediaLineAudio = (sipMediaLineSt *) &bufAudio[0];
	sipMediaLineSt *pMediaLineVideo = (sipMediaLineSt *) &bufVideo[0];
	sipMediaLineSt *pMediaLineData = (sipMediaLineSt *) &bufData[0];
	sipMediaLineSt *pMediaLineContent = (sipMediaLineSt *) &bufContent[0];
	sipMediaLineSt *pMediaLineBfcp = (sipMediaLineSt *) &bufBfcp[0];

	memset(bufAudio, 0, sizeof(bufAudio));
	memset(bufVideo, 0, sizeof(bufVideo));
	memset(bufData, 0, sizeof(bufData));
	memset(bufContent, 0, sizeof(bufContent));
	memset(bufBfcp, 0, sizeof(bufBfcp));

	if (totalSize <= bufSize)
	{
		int numOfMediaCapSet 			= 0;
		int numOfSdesMediaCapSet 		= 0;
		int numOfMediaICECapSet         = 0;
		capBuffer** pMediaCapList		= NULL;
		capBuffer** pSdesMediaCapList	= NULL;
		capBuffer** pMediaICECapList	= NULL;
		WORD curCapBufferSize = 0;
		cmCapDataType mediaType;
		ERoleLabel eRole;
		for(int i = kMediaLineInternalTypeNone+1; i < kMediaLineInternalTypeLast; i++)
		{
			GetMediaDataTypeAndRole((eMediaLineInternalType)i, mediaType, eRole);
			GetMediaCaps(mediaType, &numOfMediaCapSet, &pMediaCapList,eRole);
			GetSdesMediaCaps(mediaType,&numOfSdesMediaCapSet,&pSdesMediaCapList,eRole);
			GetMediaIceCaps(globalIceSessionTypes[i],&numOfMediaICECapSet,&pMediaICECapList);

			for (int j = 0; j < numOfMediaCapSet; j++)
			{
				CCapSetInfo capInfo = (CapEnum)pMediaCapList[j]->capTypeCode;
				CBaseCap*	pCap	= GetCapSet(mediaType,j,eRole);
				int	capEnum = pMediaCapList[j]->capTypeCode;
				int	len	= pMediaCapList[j]->capLength;

				if ( pCap &&
						(eDirection == cmCapReceiveAndTransmit || pCap->IsDirection(eDirection)) )
				{
					if (bOperUse)
					{
						int lenToRemove = 0;
						if (capInfo.GetCapType() == cmCapAudio ||
							 (CapEnum)capInfo == ePeopleContentCapCode || (CapEnum)capInfo == eRoleLabelCapCode)
						{
							lenToRemove = sizeof(APIS32);
						}

						capEnum = capInfo.GetIpCapCode();
						len	-= lenToRemove;
					}

					curCapBufferSize = sizeof(capBufferBase) + len;

					capPosSaveAudio = capPosAudio;
					capPosSaveVideo = capPosVideo;
					capPosSaveData = capPosData;
					capPosSaveContent = capPosContent;
					capPosSaveBfcp = capPosBfcp;

					if (fAddMediaLine)
					{
						//**********Audio*********//
						if (mediaType == cmCapAudio)
						{
							if (AddCapInMediaLine(pMediaCapList[j], curCapBufferSize,pMediaLineAudio, SIP_MEDIA_LINE_BUFFER_SIZE, capPosAudio))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineAudio->caps, capPosSaveAudio, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Audio");
						}
						//**********Video*********//
						else if ((mediaType == cmCapVideo) && (eRole == kRolePeople))
						{
							if (AddCapInMediaLine(pMediaCapList[j], curCapBufferSize,pMediaLineVideo, SIP_MEDIA_LINE_BUFFER_SIZE, capPosVideo))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineVideo->caps, capPosSaveVideo, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Video");
						}
						//**********Data*********//
						else if (mediaType == cmCapData)
						{
							if (AddCapInMediaLine(pMediaCapList[j], curCapBufferSize,pMediaLineData, SIP_MEDIA_LINE_BUFFER_SIZE, capPosData))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineData->caps, capPosSaveData, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Data");
						}
						//**********Content*********//
						else if ((mediaType == cmCapVideo) && (eRole == kRolePresentation))
						{
							if (AddCapInMediaLine(pMediaCapList[j], curCapBufferSize,pMediaLineContent, SIP_MEDIA_LINE_BUFFER_SIZE, capPosContent))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineContent->caps, capPosSaveContent, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Content");
						}
						//**********Bfcp*********//
						else if (mediaType == cmCapBfcp)
						{
							if (AddCapInMediaLine(pMediaCapList[j], curCapBufferSize,pMediaLineBfcp, SIP_MEDIA_LINE_BUFFER_SIZE, capPosBfcp))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineBfcp->caps, capPosSaveBfcp, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Bfcp");
						}
					}
					else {
						//**********Audio*********//
						if (mediaType == cmCapAudio)
						{
							if (AddCapInBuffer(pMediaCapList[j], curCapBufferSize,bufAudio, SIP_MEDIA_LINE_BUFFER_SIZE, capPosAudio))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufAudio, capPosSaveAudio, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Audio");
						}
						//**********Video*********//
						else if ((mediaType == cmCapVideo) && (eRole == kRolePeople))
						{
							if (AddCapInBuffer(pMediaCapList[j], curCapBufferSize,bufVideo, SIP_MEDIA_LINE_BUFFER_SIZE, capPosVideo))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufVideo, capPosSaveVideo, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Video");
						}
						//**********Data*********//
						else if (mediaType == cmCapData)
						{
							if (AddCapInBuffer(pMediaCapList[j], curCapBufferSize,bufData, SIP_MEDIA_LINE_BUFFER_SIZE, capPosData))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufData, capPosSaveData, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Data");
						}
						//**********Content*********//
						else if ((mediaType == cmCapVideo) && (eRole == kRolePresentation))
						{
							if (AddCapInBuffer(pMediaCapList[j], curCapBufferSize,bufContent, SIP_MEDIA_LINE_BUFFER_SIZE, capPosContent))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufContent, capPosSaveContent, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Content");
						}
						//**********Bfcp*********//
						else if (mediaType == cmCapBfcp)
						{
							if (AddCapInBuffer(pMediaCapList[j], curCapBufferSize,bufBfcp, SIP_MEDIA_LINE_BUFFER_SIZE, capPosBfcp))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufBfcp, capPosSaveBfcp, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Bfcp");
						}
					}

					(*pNumOfCaps)++;
				}
				POBJDELETE(pCap);
			}
			
			/////  Add  Sdes Caps //////////
			for (int j = 0; j < numOfSdesMediaCapSet; j++)
			{
				CCapSetInfo capInfo = (CapEnum)pSdesMediaCapList[j]->capTypeCode;
				CSdesCap* pCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)pSdesMediaCapList[j]->capTypeCode,pSdesMediaCapList[j]->dataCap);
				int	capEnum = pSdesMediaCapList[j]->capTypeCode;
				int	len	= 0;
				if (pCap)
				    len = pCap->SizeOf();
				else
				    PTRACE(eLevelError,"CSipCaps::CopyCapBuffersWithSdesToBuffer - pCap is NULL");

				if ( pCap &&
						(eDirection == cmCapReceiveAndTransmit || pCap->IsDirection(eDirection)) )
				{
					if (bOperUse)
					{
						capEnum = capInfo.GetIpCapCode();
					}

					curCapBufferSize = sizeof(capBufferBase) + len;

					capPosSaveAudio = capPosAudio;
					capPosSaveVideo = capPosVideo;
					capPosSaveData = capPosData;
					capPosSaveContent = capPosContent;
					if (fAddMediaLine)
					{

						//********** Sdes Audio*********//
						if (mediaType == cmCapAudio)
						{				
							if (AddCapInMediaLine(pSdesMediaCapList[j], curCapBufferSize,pMediaLineAudio, SIP_MEDIA_LINE_BUFFER_SIZE, capPosAudio))
							{
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineAudio->caps, capPosSaveAudio, capEnum, len);
								bIsAudioSdesEnable = TRUE;
							}
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Audio");
						}
						//********** Sdes Video*********//
						else if ((mediaType == cmCapVideo) && (eRole == kRolePeople))
						{						
							if (AddCapInMediaLine(pSdesMediaCapList[j], curCapBufferSize,pMediaLineVideo, SIP_MEDIA_LINE_BUFFER_SIZE, capPosVideo))
							{
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineVideo->caps, capPosSaveVideo, capEnum, len);
								bIsVideoSdesEnable = TRUE;
							}
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Video");
						}
						//********** Sdes Data*********//
						else if (mediaType == cmCapData)
						{							
							if (AddCapInMediaLine(pSdesMediaCapList[j], curCapBufferSize,pMediaLineData, SIP_MEDIA_LINE_BUFFER_SIZE, capPosData))
							{
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineData->caps, capPosSaveData, capEnum, len);
								bIsDataSdesEnable = TRUE;
							}
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Data");
						}
						//********** Sdes Content*********//
						else if ((mediaType == cmCapVideo) && (eRole == kRolePresentation))
						{
							if (AddCapInMediaLine(pSdesMediaCapList[j], curCapBufferSize,pMediaLineContent, SIP_MEDIA_LINE_BUFFER_SIZE, capPosContent))
							{
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineContent->caps, capPosSaveContent, capEnum, len);
								bIsContentSdesEnable = TRUE;
							}
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Content");
						}
					}
					else
					{

						//********** Sdes Audio*********//
						if (mediaType == cmCapAudio)
						{							
							if (AddCapInBuffer(pSdesMediaCapList[j], curCapBufferSize,bufAudio, SIP_MEDIA_LINE_BUFFER_SIZE, capPosAudio))
							{
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufAudio, capPosSaveAudio, capEnum, len);
								bIsAudioSdesEnable = TRUE;
							}
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Audio");
						}
						//********** Sdes Video*********//
						else if ((mediaType == cmCapVideo) && (eRole == kRolePeople))
						{							
							if (AddCapInBuffer(pSdesMediaCapList[j], curCapBufferSize,bufVideo, SIP_MEDIA_LINE_BUFFER_SIZE, capPosVideo))
							{
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufVideo, capPosSaveVideo, capEnum, len);
								bIsVideoSdesEnable = TRUE;
							}
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Video");
						}
						//********** Sdes Data*********//
						else if (mediaType == cmCapData)
						{							
							if (AddCapInBuffer(pSdesMediaCapList[j], curCapBufferSize,bufData, SIP_MEDIA_LINE_BUFFER_SIZE, capPosData))
							{
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufData, capPosSaveData, capEnum, len);
								bIsDataSdesEnable = TRUE;
							}
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Data");
						}
						//********** Sdes Content*********//
						else if ((mediaType == cmCapVideo) && (eRole == kRolePresentation))
						{							
							if (AddCapInBuffer(pSdesMediaCapList[j], curCapBufferSize,bufContent, SIP_MEDIA_LINE_BUFFER_SIZE, capPosContent))
							{
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufContent, capPosSaveContent, capEnum, len);
								bIsContentSdesEnable = TRUE;
							}
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "Content");
						}
					}
					(*pNumOfCaps)++;
				}
				POBJDELETE(pCap);
			}
		/*	///// Add ICE caps ///////
			for (int j = 0; j < numOfMediaICECapSet; j++)
			{
				CCapSetInfo capInfo = (CapEnum)pMediaICECapList[j]->capTypeCode;
				CBaseCap* pCap = GetIceCapSet(globalIceSessionTypes[j],j);
				int	capEnum = pMediaICECapList[j]->capTypeCode;
				int	len	= pMediaICECapList[j]->capLength;

				if ( pCap && (eDirection == cmCapReceiveAndTransmit || pCap->IsDirection(eDirection)) )
				{
					if (bOperUse)
					{
						capEnum = capInfo.GetIpCapCode();
					}

					curCapBufferSize = sizeof(capBufferBase) + len;

					capPosSaveAudio = capPosAudio;
					capPosSaveVideo = capPosVideo;
					capPosSaveData = capPosData;

					if (fAddMediaLine)
					{
						//////////  ICE Audio////////////
						if (globalMediaArr[i] == cmCapAudio)
						{
							if(AddCapInMediaLine(pMediaICECapList[j], curCapBufferSize,pMediaLineAudio,SIP_MEDIA_LINE_BUFFER_SIZE, capPosAudio))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineAudio->caps, capPosSaveAudio, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "ICE Audio");
						}
						///////// ICE Video ///////////////
						else if (globalMediaArr[i] == cmCapVideo)
						{
							if(AddCapInMediaLine(pMediaICECapList[j], curCapBufferSize,pMediaLineVideo,SIP_MEDIA_LINE_BUFFER_SIZE, capPosVideo))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineVideo->caps, capPosSaveVideo, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "ICE Video");
						}
						///////// ICE Data //////////////
						else if (globalMediaArr[i] == cmCapData)
						{
							if(AddCapInMediaLine(pMediaICECapList[j], curCapBufferSize,pMediaLineData,SIP_MEDIA_LINE_BUFFER_SIZE, capPosData))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, pMediaLineData->caps, capPosSaveData, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "ICE Data");
						}
					}
					else
					{
						///////// ICE Audio ////////////
						if (globalMediaArr[i] == cmCapAudio)
						{
							if(AddCapInBuffer(pMediaICECapList[j], curCapBufferSize,bufAudio,SIP_MEDIA_LINE_BUFFER_SIZE, capPosAudio))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufAudio, capPosSaveAudio, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "ICE Audio");
						}
						///////// ICE Video ////////////
						else if (globalMediaArr[i] == cmCapVideo)
						{
							if(AddCapInBuffer(pMediaICECapList[j], curCapBufferSize,bufVideo,SIP_MEDIA_LINE_BUFFER_SIZE, capPosVideo))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufVideo, capPosSaveVideo, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "ICE Video");
						}
						///////// ICE Data /////////
						else if (globalMediaArr[i] == cmCapData)
						{
							if(AddCapInBuffer(pMediaICECapList[j], curCapBufferSize,bufData,SIP_MEDIA_LINE_BUFFER_SIZE, capPosData))
								CapSetBuffer(bSetOppositeDirection, pCap, capInfo, bufData, capPosSaveData, capEnum, len);
							else
								PTRACE2(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer AddCapInMediaLine fail - ", "ICE Data");
						}
					}
					(*pNumOfCaps)++;
				}
				POBJDELETE(pCap);
			 }
	*/	}

		if (fAddMediaLine)
		{
			if (capPosAudio)
			{
				pMediaLineAudio->index = index;
				pMediaLineAudio->type = eMediaLineTypeAudio;
				if(bIsAudioSdesEnable == TRUE) {
					pMediaLineAudio->subType = eMediaLineSubTypeRtpSavp;					
					/* in case we have 2 sdes caps we need to advance the offset with sdes cap as well */
				} else {
					pMediaLineAudio->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer- AVP for Audio");
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
				if(bIsVideoSdesEnable == TRUE) {
					pMediaLineVideo->subType = eMediaLineSubTypeRtpSavp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer- SAVP for Video");
				} else {
					pMediaLineVideo->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer- AVP for Video");
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
				if(bIsDataSdesEnable == TRUE) {
					pMediaLineData->subType = eMediaLineSubTypeRtpSavp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer- SAVP for Data");
				} else {
					pMediaLineData->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer- AVP for Data");
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
				if(bIsContentSdesEnable == TRUE) {
					pMediaLineContent->subType = eMediaLineSubTypeRtpSavp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer- SAVP for Content");
				} else {
					pMediaLineContent->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::CopyCapBuffersWithSdesToBuffer- AVP for Content");
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
				pMediaLineBfcp->subType = eMediaLineSubTypeTcpBfcp; //Don't care - This code will never be executed! (you can check it for yourself)
				pMediaLineBfcp->internalType = kMediaLineInternalTypeBfcp;
				index++;
				memcpy(buffer + offsetWrite, bufBfcp, sizeof(sipMediaLineBaseSt) + capPosBfcp);
				offsetWrite += sizeof(sipMediaLineBaseSt) + capPosBfcp;
			}

		}
		else
		{
			if (capPosAudio)
			{
				memcpy(buffer + offsetWrite, bufAudio, capPosAudio);
				offsetWrite += capPosAudio;
			}
			if (capPosVideo)
			{
				memcpy(buffer + offsetWrite, bufVideo, capPosVideo);
				offsetWrite += capPosVideo;
			}
			if (capPosData)
			{
				memcpy(buffer + offsetWrite, bufData, capPosData);
				offsetWrite += capPosData;
			}
			if (capPosContent)
			{
				memcpy(buffer + offsetWrite, bufContent, capPosContent);
				offsetWrite += capPosContent;
			}
			if (capPosBfcp)
			{
				memcpy(buffer + offsetWrite, bufBfcp, capPosBfcp);
				offsetWrite += capPosBfcp;
			}

		}
	}

	if (offsetWrite != totalSize )
	{
		PTRACE(eLevelError, "CSipCaps::CopyCapBuffersWithSdesToBuffer offsetWrite != totalSize");
	}

	return offsetWrite;
}

//// Description:
//// Capabilities struct could be already initialized with cap sets.
//// We have to add cap sets to it without deleting the cap sets that already inside it.
//// We also have to check for each cap set we add that it is not already added to the struct.
//// -------------------------------------------------------------------------
BOOL CSipCaps::ValidateOffset(int offsetWrite, int structSize, char *text) const
{
	if (offsetWrite > structSize) {
		CSmallString str;
		str << text << " - Failed to add caps buffer structSize = " << structSize << " offsetWrite = " << offsetWrite;
		PTRACE2(eLevelError,"CSipCaps::ValidateOffset ",str.GetString());
		DBGPASSERT(YES);
		return FALSE;
	}

	return TRUE;
}

int CSipCaps::AddTmpMediaLine(cmCapDirection eDirection, BYTE bSetOppositeDirection,
		sipMediaLineSt *pMediaLine, sipMediaLineSt *pTmpMediaLine,
		int structSize, int fAudio, int fVideo, int fData, int fContent, int fBfcp) const
{
	BOOL res = TRUE;
	char *text = "AddTmpMediaLine";
	int offsetWrite	= 0;

	if (pMediaLine) {

		offsetWrite = sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

		res = ValidateOffset(offsetWrite, structSize, text);
		if (res) {
			memcpy(pTmpMediaLine, pMediaLine, sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection);
		}
	}

	if (res && (fAudio || fVideo || fData || fContent || fBfcp)) {

		offsetWrite = AddCapsToCapStruct(eDirection, bSetOppositeDirection, pTmpMediaLine,
											structSize, fAudio, fVideo, fData, fContent, fBfcp);
	//	offsetWrite = AddSdesCapsToCapStruct(eDirection, bSetOppositeDirection, pTmpMediaLine,
					//								structSize, fAudio, fVideo, fData, fContent, fBfcp);

	}

	return offsetWrite;
}
/*
 * parameters:
 * 	eDirection
 * 	bSetOppositeDirection
 * 	pMediaLinesEntry - media lines buffer to to which caps need to be added.
 * 	structSize		 - allocated size pMediaLinesEntry buffer
 * 	bfcpTransportType
 * 	addAudioCap - add audio caps from the current SipCaps object to the pMediaLinesEntry. if audio media line doesn't exist in pMediaLinesEntry, create one and add audio caps from the current SipCaps object
 * 	addVideoCap
 * 	addDataCap
 * 	addContentCap
 * 	addBfcpCap
 *
 *
 * description:
 * Phase 1: go over media lines in pMediaLinesEntry. for every media line (if exists) copy existing caps to corresponding temp media line,
 * and then add caps from the current SipCaps object. if media line of some type doesn't exist in pMediaLinesEntry, create one and add caps from the current SipCaps object to temp media line.
 * Phase 2: build temp MediaLinesEntry by copying temp media lines to it.
 * Phase 3: copy temp MediaLinesEntry to pMediaLinesEntry.
 *
 */


int	CSipCaps::AddCapsToCapStruct(cmCapDirection eDirection, BYTE bSetOppositeDirection, sipMediaLinesEntrySt *pMediaLinesEntry, int structSize, eMediaLineSubType bfcpTransportType,
									int addAudioCap, int addVideoCap, int addDataCap, int addContentCap, int addBfcpCap, BOOL bOverrideSavpWithAvp) const
{
	char *text = "AddCapsToCapStruct";
	int offsetWrite	= 0;
	int mediaLinePos = 0;
	sipMediaLinesEntrySt* pTmpMediaLinesEntry = NULL;
	sipMediaLineSt *pMediaLine = NULL;
	char *buffer = NULL;

	APIU8 subType = eMediaLineSubTypeNull;
	int fCreateMediaLineAudio = addAudioCap; // audio media lime doesn't exist in the given pMediaLinesEntry, so we need create new media line with caps from this object
	int fCreateMediaLineVideo = addVideoCap;
	int fCreateMediaLineData = addDataCap;
	int fCreateMediaLineContent = addContentCap;
	int fCreateMediaLineBfcp = addBfcpCap;
	BOOL fMediaLineAudio = FALSE; // there is update(new caps) in audio media line, so temp audio media line buffer(bufAudio) should be copied to the pMediaLinesEntry
	BOOL fMediaLineVideo = FALSE;
	BOOL fMediaLineData = FALSE;
	BOOL fMediaLineContent = FALSE;
	BOOL fMediaLineBfcp = FALSE;
	char bufAudio[SIP_MEDIA_LINE_BUFFER_SIZE]; // temp audio media line buffer
	char bufVideo[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufData[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufContent[SIP_MEDIA_LINE_BUFFER_SIZE];
	char bufBfcp[SIP_MEDIA_LINE_BUFFER_SIZE];
	int index = 0;
	sipMediaLineSt *pMediaLineAudio = (sipMediaLineSt *) &bufAudio[0];
	sipMediaLineSt *pMediaLineVideo = (sipMediaLineSt *) &bufVideo[0];
	sipMediaLineSt *pMediaLineData = (sipMediaLineSt *) &bufData[0];
	sipMediaLineSt *pMediaLineContent = (sipMediaLineSt *) &bufContent[0];
	sipMediaLineSt *pMediaLineBfcp = (sipMediaLineSt *) &bufBfcp[0];
	memset(bufAudio, 0, sizeof(bufAudio));
	memset(bufVideo, 0, sizeof(bufVideo));
	memset(bufData, 0, sizeof(bufData));
	memset(bufContent, 0, sizeof(bufContent));
	memset(bufBfcp, 0, sizeof(bufBfcp));

	//Allocate tmp sipMediaLinesEntrySt
	buffer = new char [structSize];
	if (!buffer) {
		CSmallString str;
		str << "Failed to allocate buffer length " << structSize;
		PTRACE2(eLevelError,"CSipCaps::AddCapsToCapStruct ",str.GetString());
		DBGPASSERT(YES);
		return 0;
	}
	memset(buffer, 0, structSize);
	pTmpMediaLinesEntry = (sipMediaLinesEntrySt *) &buffer[0];


	//Add to existing media lines


// ------------------------------   adding caps into the temp mline buffers for all media types 	--------------------------------
// ------------------------------   if the mline already exists, we first copy existing caps to 	--------------------------------
// ------------------------------   the temp media line buffers and then add other caps 			--------------------------------

	for (unsigned int i = 0; i < pMediaLinesEntry->numberOfMediaLines; i++) {

		pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
		mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

		int fAudio = NO;   // no - do not add new audio caps, yes - add audio caps
		int fVideo = NO;
		int fData = NO;
		int fContent = NO;
		int fBfcp = NO;

		if (pMediaLine->internalType == kMediaLineInternalTypeAudio) {
			if(pMediaLine->subType > eMediaLineSubTypeUnknown)
				subType = pMediaLine->subType;
			if (addAudioCap) {
				fCreateMediaLineAudio = NO;
				fAudio = YES;
			}

			//if (fAudio) copy from pMLine to pMediaLineAudio buffer and then add to pMediaLineAudio new caps
			offsetWrite = AddTmpMediaLine(eDirection, bSetOppositeDirection, pMediaLine, pMediaLineAudio,
					SIP_MEDIA_LINE_BUFFER_SIZE, fAudio, fVideo, fData, fContent, fBfcp);

			if (ValidateOffset(offsetWrite, SIP_MEDIA_LINE_BUFFER_SIZE, text))
				fMediaLineAudio = TRUE; // temp audio media line should be copied to the pMediaLinesEntry
		}

		else if (pMediaLine->internalType == kMediaLineInternalTypeVideo) {
			if (addVideoCap) {
				fCreateMediaLineVideo = NO;
				fVideo = YES;
			}
			offsetWrite = AddTmpMediaLine(eDirection, bSetOppositeDirection, pMediaLine, pMediaLineVideo,
					SIP_MEDIA_LINE_BUFFER_SIZE, fAudio, fVideo, fData, fContent, fBfcp);

			if (ValidateOffset(offsetWrite, SIP_MEDIA_LINE_BUFFER_SIZE, text))
				fMediaLineVideo = TRUE;
		}

		else if (pMediaLine->internalType == kMediaLineInternalTypeFecc) {
			if (addDataCap) {
				fCreateMediaLineData = NO;
				fData = YES;
			}
			offsetWrite = AddTmpMediaLine(eDirection, bSetOppositeDirection, pMediaLine, pMediaLineData,
					SIP_MEDIA_LINE_BUFFER_SIZE, fAudio, fVideo, fData, fContent, fBfcp);

			if (ValidateOffset(offsetWrite, SIP_MEDIA_LINE_BUFFER_SIZE, text))
				fMediaLineData = TRUE;
		}

		else if (pMediaLine->internalType == kMediaLineInternalTypeContent) {
			if (addContentCap) {
				fCreateMediaLineContent = NO;
				fContent = YES;
			}
			offsetWrite = AddTmpMediaLine(eDirection, bSetOppositeDirection, pMediaLine, pMediaLineContent,
					SIP_MEDIA_LINE_BUFFER_SIZE, fAudio, fVideo, fData, fContent, fBfcp);

			if (ValidateOffset(offsetWrite, SIP_MEDIA_LINE_BUFFER_SIZE, text))
				fMediaLineContent = TRUE;

		}

		else if (pMediaLine->internalType == kMediaLineInternalTypeBfcp) {
			if (addBfcpCap) {
				fCreateMediaLineBfcp = NO;
				fBfcp = YES;
			}
			offsetWrite = AddTmpMediaLine(eDirection, bSetOppositeDirection, pMediaLine, pMediaLineBfcp,
					SIP_MEDIA_LINE_BUFFER_SIZE, fAudio, fVideo, fData, fContent, fBfcp);

			if (ValidateOffset(offsetWrite, SIP_MEDIA_LINE_BUFFER_SIZE, text))
				fMediaLineBfcp = TRUE;
		}
	}


// we create new media line in tmpMLinesEntry in case: we need to add media cap, but we don't have media line of this type in the given pMLinesEntry.

	//Create new audio media line
	if (fCreateMediaLineAudio && GetNumOfMediaCapSets(cmCapAudio,eDirection)) {
		offsetWrite = AddTmpMediaLine(eDirection, bSetOppositeDirection, NULL, pMediaLineAudio,
				SIP_MEDIA_LINE_BUFFER_SIZE, YES, NO, NO, NO, NO);
		if (ValidateOffset(offsetWrite, SIP_MEDIA_LINE_BUFFER_SIZE, text))
			fMediaLineAudio = TRUE;
	}

	//Create new video media line
	if (fCreateMediaLineVideo && GetNumOfMediaCapSets(cmCapVideo,eDirection)) {
		offsetWrite = AddTmpMediaLine(eDirection, bSetOppositeDirection, NULL, pMediaLineVideo,
				SIP_MEDIA_LINE_BUFFER_SIZE, NO, YES, NO, NO, NO);
		if (ValidateOffset(offsetWrite, SIP_MEDIA_LINE_BUFFER_SIZE, text))
			fMediaLineVideo = TRUE;
	}

	//Create new data media line
	if (fCreateMediaLineData && GetNumOfMediaCapSets(cmCapData,eDirection)) {
		offsetWrite = AddTmpMediaLine(eDirection, bSetOppositeDirection, NULL, pMediaLineData,
				SIP_MEDIA_LINE_BUFFER_SIZE, NO, NO, YES, NO, NO);
		if (ValidateOffset(offsetWrite, SIP_MEDIA_LINE_BUFFER_SIZE, text))
			fMediaLineData = TRUE;
	}

	//Create new content media line
	if (fCreateMediaLineContent && GetNumOfMediaCapSets(cmCapVideo,eDirection,kRolePresentation)) {
		offsetWrite = AddTmpMediaLine(eDirection, bSetOppositeDirection, NULL, pMediaLineContent,
				SIP_MEDIA_LINE_BUFFER_SIZE, NO, NO, NO, YES, NO);
		if (ValidateOffset(offsetWrite, SIP_MEDIA_LINE_BUFFER_SIZE, text))
			fMediaLineContent = TRUE;
	}

	//Create new bfcp media line
	if (fCreateMediaLineBfcp && GetNumOfMediaCapSets(cmCapBfcp,eDirection)) {
		offsetWrite = AddTmpMediaLine(eDirection, bSetOppositeDirection, NULL, pMediaLineBfcp,
				SIP_MEDIA_LINE_BUFFER_SIZE, NO, NO, NO, NO, YES);
		if (ValidateOffset(offsetWrite, SIP_MEDIA_LINE_BUFFER_SIZE, text))
			fMediaLineBfcp = TRUE;
	}
// ------------------------------   end to build the temp mline buffers for all media types --------------------------------

	//Set the new media entry
	index = 0;
	offsetWrite = sizeof(sipMediaLinesEntryBaseSt);

	capBuffer** pSdesMediaCapList	= NULL;
	int numOfSdesAudioMediaCapSet 		= 0;
	int numOfSdesVideoMediaCapSet 		= 0;
	int numOfSdesDataMediaCapSet 		= 0;
	int numOfSdesContentMediaCapSet		= 0;
	GetSdesMediaCaps(cmCapAudio,&numOfSdesAudioMediaCapSet,&pSdesMediaCapList);
	GetSdesMediaCaps(cmCapVideo,&numOfSdesVideoMediaCapSet,&pSdesMediaCapList);
	GetSdesMediaCaps(cmCapData,&numOfSdesDataMediaCapSet,&pSdesMediaCapList);
	GetSdesMediaCaps(cmCapVideo,&numOfSdesContentMediaCapSet,&pSdesMediaCapList,kRolePresentation);
	// if there are new caps for audio media line and there is enogth space in buffer,
	// set media line generic attributes and copy it to temp buffer ( pTmpMediaLinesEntry )
	if (fMediaLineAudio && ValidateOffset(offsetWrite + sizeof(sipMediaLineBaseSt) + pMediaLineAudio->lenOfDynamicSection, structSize, text)) {
		pMediaLineAudio->index = index;
		pMediaLineAudio->type = eMediaLineTypeAudio;
		if (subType)
			pMediaLineAudio->subType = subType;
		else {
			if(numOfSdesAudioMediaCapSet)
			{
				if (bOverrideSavpWithAvp)
				{
					pMediaLineAudio->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- SDES AVP for Audio");
				}
				else
				{
				pMediaLineAudio->subType = eMediaLineSubTypeRtpSavp;
					PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- SDES SAVP for Audio");
				}
			} else {
				pMediaLineAudio->subType = eMediaLineSubTypeRtpAvp;
				PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- AVP for Audio");
			}
		}
		pMediaLineAudio->internalType = kMediaLineInternalTypeAudio;
		index++;
		memcpy(buffer + offsetWrite, bufAudio, sizeof(sipMediaLineBaseSt) + pMediaLineAudio->lenOfDynamicSection);
		offsetWrite += sizeof(sipMediaLineBaseSt) + pMediaLineAudio->lenOfDynamicSection;
		pTmpMediaLinesEntry->numberOfMediaLines++;
		pTmpMediaLinesEntry->lenOfDynamicSection += sizeof(sipMediaLineBaseSt) + pMediaLineAudio->lenOfDynamicSection;

	}
	// if there are new caps for video media line and there is enogth space in buffer,
	// set media line generic attributes and copy it to temp buffer ( pTmpMediaLinesEntry )
	if (fMediaLineVideo && ValidateOffset(offsetWrite + sizeof(sipMediaLineBaseSt) + pMediaLineVideo->lenOfDynamicSection, structSize, text)) {
		pMediaLineVideo->index = index;
		pMediaLineVideo->type = eMediaLineTypeVideo;
		if (subType)
			pMediaLineVideo->subType = subType;
		else {
			if(numOfSdesVideoMediaCapSet)
			{
				if (bOverrideSavpWithAvp)
				{
					pMediaLineVideo->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- SDES AVP for Video");
				}
				else
				{
				pMediaLineVideo->subType = eMediaLineSubTypeRtpSavp;
					PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- SDES SAVP for Video");
				}
			} else {
				pMediaLineVideo->subType = eMediaLineSubTypeRtpAvp;
				PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- AVP for Video");
			}
		}
		pMediaLineVideo->internalType = kMediaLineInternalTypeVideo;
		index++;
		memcpy(buffer + offsetWrite, bufVideo, sizeof(sipMediaLineBaseSt) + pMediaLineVideo->lenOfDynamicSection);
		offsetWrite += sizeof(sipMediaLineBaseSt) + pMediaLineVideo->lenOfDynamicSection;
		pTmpMediaLinesEntry->numberOfMediaLines++;
		pTmpMediaLinesEntry->lenOfDynamicSection += sizeof(sipMediaLineBaseSt) + pMediaLineVideo->lenOfDynamicSection;

	}
	// if there are new caps for data media line and there is enogth space in buffer,
	// set media line generic attributes and copy it to temp buffer ( pTmpMediaLinesEntry )
	if (fMediaLineData && ValidateOffset(offsetWrite + sizeof(sipMediaLineBaseSt) + pMediaLineData->lenOfDynamicSection, structSize, text)) {
		pMediaLineData->index = index;
		pMediaLineData->type = eMediaLineTypeApplication;
		if (subType)
			pMediaLineData->subType = subType;
		else {
			if(numOfSdesDataMediaCapSet)
			{
				if (bOverrideSavpWithAvp)
				{
					pMediaLineData->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- SDES AVP for Data");
				}
				else
				{
				pMediaLineData->subType = eMediaLineSubTypeRtpSavp;
					PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- SDES SAVP for Data");
				}
			} else {
				pMediaLineData->subType = eMediaLineSubTypeRtpAvp;
				PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- AVP for Data");
			}
		}
		pMediaLineData->internalType = kMediaLineInternalTypeFecc;
		index++;
		memcpy(buffer + offsetWrite, bufData, sizeof(sipMediaLineBaseSt) + pMediaLineData->lenOfDynamicSection);
		offsetWrite += sizeof(sipMediaLineBaseSt) + pMediaLineData->lenOfDynamicSection;
		pTmpMediaLinesEntry->numberOfMediaLines++;
		pTmpMediaLinesEntry->lenOfDynamicSection += sizeof(sipMediaLineBaseSt) + pMediaLineData->lenOfDynamicSection;

	}
	// if there are new caps for presentation media line,
	// set media line generic attributes and copy it to temp buffer ( pTmpMediaLinesEntry )
	if (fMediaLineContent && ValidateOffset(offsetWrite + sizeof(sipMediaLineBaseSt) + pMediaLineContent->lenOfDynamicSection, structSize, text)) {
		pMediaLineContent->index = index;
		pMediaLineContent->type = eMediaLineTypeVideo;
		if (subType)
			pMediaLineContent->subType = subType;
		else {
			if(numOfSdesContentMediaCapSet)
			{
				if (bOverrideSavpWithAvp)
				{
					pMediaLineContent->subType = eMediaLineSubTypeRtpAvp;
					PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- SDES AVP for Content");
				}
				else
				{
				pMediaLineContent->subType = eMediaLineSubTypeRtpSavp;
					PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- SDES SAVP for Content");
				}
			} else {
				pMediaLineContent->subType = eMediaLineSubTypeRtpAvp;
				PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct- AVP for Content");
			}
		}
		pMediaLineContent->internalType = kMediaLineInternalTypeContent;
		index++;
		memcpy(buffer + offsetWrite, bufContent, sizeof(sipMediaLineBaseSt) + pMediaLineContent->lenOfDynamicSection);
		offsetWrite += sizeof(sipMediaLineBaseSt) + pMediaLineContent->lenOfDynamicSection;
		pTmpMediaLinesEntry->numberOfMediaLines++;
		pTmpMediaLinesEntry->lenOfDynamicSection += sizeof(sipMediaLineBaseSt) + pMediaLineContent->lenOfDynamicSection;

	}
	// if there are new caps for bfcp media line and there is enogth space in buffer,
	// set media line generic attributes and copy it to temp buffer ( pTmpMediaLinesEntry )
	if (fMediaLineBfcp && ValidateOffset(offsetWrite + sizeof(sipMediaLineBaseSt) + pMediaLineBfcp->lenOfDynamicSection, structSize, text)) {
		pMediaLineBfcp->index = index;
		pMediaLineBfcp->type = eMediaLineTypeApplication;
		pMediaLineBfcp->subType = bfcpTransportType;
		pMediaLineBfcp->internalType = kMediaLineInternalTypeBfcp;
		index++;
		memcpy(buffer + offsetWrite, bufBfcp, sizeof(sipMediaLineBaseSt) + pMediaLineBfcp->lenOfDynamicSection);
		offsetWrite += sizeof(sipMediaLineBaseSt) + pMediaLineBfcp->lenOfDynamicSection;
		pTmpMediaLinesEntry->numberOfMediaLines++;
		pTmpMediaLinesEntry->lenOfDynamicSection += sizeof(sipMediaLineBaseSt) + pMediaLineBfcp->lenOfDynamicSection;
	}

	if (offsetWrite)
		memcpy(pMediaLinesEntry, pTmpMediaLinesEntry, offsetWrite);
	delete [] buffer;

	return offsetWrite;
}


int	CSipCaps::AddCapsToCapStruct(cmCapDirection eDirection, BYTE bSetOppositeDirection, sipMediaLineSt *pMediaLine, int structSize,
									int addAudioCap, int addVideoCap, int addDataCap, int addContentCap, int addBfcpCap) const
{
	PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct ");

	int offsetWrite	= 0;

	int numOfAudioCaps = 0;
	int numOfVideoCaps = 0;
	int numOfDataCaps = 0;
	int numOfContentCaps = 0;
	int numOfBfcpCaps = 0;

	int numOfAudioSdesCaps = 0;
	int numOfVideoSdesCaps = 0;
	int numOfDataSdesCaps = 0;
	int numOfContentSdesCaps = 0;


	CSipCaps* pTempCaps = new CSipCaps;
	*pTempCaps = *this;
	CBaseCap** initCapsPtrArr = NULL;
	unsigned numOfInitCaps	= pMediaLine->numberOfCaps;
	int sizeOfInitCaps	= 0;
	capBuffer* pCurCapBuffer = (capBuffer *) &pMediaLine->caps[0];

	if ( numOfInitCaps )
	{
		initCapsPtrArr = new CBaseCap*[numOfInitCaps];

		for(unsigned i = 0; i < numOfInitCaps; i++)
		{
			initCapsPtrArr[i] = CBaseCap::AllocNewCap((CapEnum)pCurCapBuffer->capTypeCode,pCurCapBuffer->dataCap);


			if ( initCapsPtrArr[i] )
			{
//				if ((CapEnum)pCurCapBuffer->capTypeCode == eBFCPCapCode)
//					initCapsPtrArr[i]->SetDefaults(cmCapReceiveAndTransmit);

				cmCapDataType eMediaType = initCapsPtrArr[i]->GetType();
				ERoleLabel eRole = initCapsPtrArr[i]->GetRole();

				if(pMediaLine->internalType == kMediaLineInternalTypeAudio)
				{

					if ( eMediaType == cmCapAudio || eMediaType == cmCapGeneric)
					{
						numOfAudioCaps++;					
					}
				}
				else if(pMediaLine->internalType == kMediaLineInternalTypeVideo)
				{
				//	PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct Video Mline");
					if	( ((eMediaType == cmCapVideo) && (eRole == kRolePeople)) ||  eMediaType == cmCapGeneric)
					{
						numOfVideoCaps++;
					}
				}
				else if(pMediaLine->internalType == kMediaLineInternalTypeFecc)
				{
				//	PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct data Mline");
					if ( eMediaType == cmCapData || eMediaType == cmCapGeneric)
					{
						PTRACE2INT(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct FECC Mline",numOfDataCaps);
						numOfDataCaps++;
					}
				}
				else if(pMediaLine->internalType == kMediaLineInternalTypeContent)
				{
					if	( ((eMediaType == cmCapVideo) && (eRole == kRolePresentation)) ||  eMediaType == cmCapGeneric)
					{
						numOfContentCaps++;
					}
				}
				else if(pMediaLine->internalType == kMediaLineInternalTypeBfcp)
				{
					if ( eMediaType == cmCapBfcp ||  eMediaType == cmCapGeneric)
					{
						numOfBfcpCaps++;
					}
				}
				//remove all capabilities that contain the current init cap set
				WORD details	= 0;
				int  arrIndex	= 0;
				DWORD values	= kCapCode | kRoleLabel | kFormat | kFrameRate | kCryptoSuit;
				/* Bridge-5173 - duplicate payload type appears due to mismatch of the bitrate */
				if (!addVideoCap)
					values |= kBitRate;

				BYTE checkCapDirection = TRUE;

				if(addVideoCap)
					checkCapDirection = FALSE;

				cmCapDirection eCapDirection = initCapsPtrArr[i]->GetDirection();

				if((int)eCapDirection == (int)kInactive)
					checkCapDirection = FALSE;
				if (bSetOppositeDirection)  //meaning that "this" is the remote caps
					eCapDirection = CalcOppositeDirection(eCapDirection);

				// Remove audio caps that are already in the buffer (not removing ice caps)
				if((initCapsPtrArr[i]->GetCapCode() != eSdesCapCode))
				{

					BYTE bIsContains = pTempCaps->IsContainingCapBuffer(eCapDirection, *pCurCapBuffer, values, &arrIndex,checkCapDirection);
				//	PTRACE2INT(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct bIsContains",bIsContains);
				//	PTRACE2INT(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct eCapDirection",eCapDirection);
					if(bIsContains && (eMediaType == cmCapVideo) && (eRole == kRolePeople) && (eH263CapCode == (CapEnum)pCurCapBuffer->capTypeCode))
					{// if its H263 we only remove one appearance from the temps caps, since we want to add the second H263
						// (if exist) with the static payload type to the request.
						// in general I had the filling that for all algorithm we should remove only one cap set, but I hadn't got the time to check it.
						TRACEINTO << "[REMOVED MEDIA LINE  (eMediaType == cmCapVideo) ] " << (int)eMediaType;
						capBuffer* pRemoved = pTempCaps->RemoveCapSet(eMediaType,arrIndex);
						PDELETEA(pRemoved);
					}
					else
					{
						while(bIsContains)
						{
							TRACEINTO << "In Remove, media type " << (int)eMediaType;
							TRACEINTO << "[REMOVED MEDIA LINE (eMediaType != cmCapVideo) ] " << (int)eMediaType;
							capBuffer* pRemoved = pTempCaps->RemoveCapSet(eMediaType,arrIndex,eRole);
							PDELETEA(pRemoved);
							//next cap that contains
							bIsContains = pTempCaps->IsContainingCapBuffer(eCapDirection, *pCurCapBuffer, values, &arrIndex,checkCapDirection);
						}
					}
				}
				else
				{
					PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct - try Remove SDES");

					BYTE bIsContains = FALSE;
					bIsContains = pTempCaps->IsContainingCapBufferForSdes(eCapDirection, *pCurCapBuffer, values, &arrIndex);
					//PTRACE2INT(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct - try Remove SDES arrIndex ",arrIndex);
						while(bIsContains)
						{
							TRACEINTO << "[REMOVED MEDIA LINE (initCapsPtrArr[i]->GetCapCode() == eSdesCapCode) ] " << (int)eMediaType;
							PTRACE(eLevelInfoNormal,"CSipCaps::AddCapsToCapStruct - In Remove SDES");
							capBuffer* pRemoved = pTempCaps->RemoveCapSetForSdes(eMediaType,arrIndex,eRole);
							PDELETEA(pRemoved);
							//next cap that contains
							//bIsContains = FALSE;
							bIsContains = pTempCaps->IsContainingCapBufferForSdes(eCapDirection, *pCurCapBuffer, values, &arrIndex);
						}

				}



				//next cap set
				sizeOfInitCaps += (sizeof(capBufferBase) + pCurCapBuffer->capLength);
				pCurCapBuffer  =  (capBuffer *)((BYTE*)pCurCapBuffer + sizeof(capBufferBase) + pCurCapBuffer->capLength);
			}
		}


	}
	if(addAudioCap == NO)
	{// remove audio caps
		pTempCaps->CleanMedia(cmCapAudio);
		pTempCaps->CleanSdesMedia(cmCapAudio);
	}

	if(addVideoCap == NO)
	{// remove video caps
		pTempCaps->CleanMedia(cmCapVideo);
		pTempCaps->CleanSdesMedia(cmCapVideo);
	}

	if(addDataCap == NO)
	{// remove data caps
		pTempCaps->CleanMedia(cmCapData);
		pTempCaps->CleanSdesMedia(cmCapData);
	}

	if(addContentCap == NO)
	{// remove content caps
		pTempCaps->CleanMedia(cmCapVideo,kRolePresentation);
		pTempCaps->CleanSdesMedia(cmCapVideo,kRolePresentation);
	}

	if(addBfcpCap == NO)
	{// remove bfcp caps
		pTempCaps->CleanMedia(cmCapBfcp);
	}

	//Remove ICE caps from TempCaps
	pTempCaps->CleanIceMedia(eAudioSession);
	pTempCaps->CleanIceMedia(eVideoSession);
	pTempCaps->CleanIceMedia(eDataSession);

	int sizeOfCapsWeWantToAdd = pTempCaps->CalcCapBuffersSize(eDirection,NO, FALSE);
	int sizeOfSdesCApsWeWAntToAdd = pTempCaps->CalcSdesOnlyCapBuffersSize(eDirection,NO, FALSE);
	int totalSize = sizeof(sipMediaLineBaseSt) + sizeOfInitCaps + sizeOfCapsWeWantToAdd + sizeOfSdesCApsWeWAntToAdd;

	offsetWrite = sizeof(sipMediaLineBaseSt) + sizeOfInitCaps;

	if ( structSize >= totalSize )
	{
		numOfAudioCaps += pTempCaps->GetNumOfMediaCapSets(cmCapAudio,eDirection);
		numOfVideoCaps += pTempCaps->GetNumOfMediaCapSets(cmCapVideo,eDirection);
		numOfDataCaps += pTempCaps->GetNumOfMediaCapSets(cmCapData,eDirection);
		numOfContentCaps += pTempCaps->GetNumOfMediaCapSets(cmCapVideo,eDirection,kRolePresentation);
		numOfBfcpCaps += pTempCaps->GetNumOfMediaCapSets(cmCapBfcp,eDirection);

		numOfAudioSdesCaps = pTempCaps->GetNumOfSdesMediaCapSets(cmCapAudio,eDirection);
		numOfVideoSdesCaps = pTempCaps->GetNumOfSdesMediaCapSets(cmCapVideo,eDirection);
		numOfDataSdesCaps = pTempCaps->GetNumOfSdesMediaCapSets(cmCapData,eDirection);
		numOfContentSdesCaps = pTempCaps->GetNumOfSdesMediaCapSets(cmCapVideo,eDirection, kRolePresentation);


		pMediaLine->numberOfCaps = (numOfAudioCaps + numOfVideoCaps + numOfDataCaps + numOfContentCaps + numOfBfcpCaps
				+ numOfAudioSdesCaps + numOfVideoSdesCaps + numOfDataSdesCaps + numOfContentSdesCaps);

		int numOfCapsWeWantToAdd = 0;
		int numOfSdesCapsWeWantToAdd = 0;
		offsetWrite += pTempCaps->CopyCapBuffersToBuffer((BYTE*)pCurCapBuffer, sizeOfCapsWeWantToAdd , &numOfCapsWeWantToAdd, eDirection, bSetOppositeDirection, NO, sizeOfSdesCApsWeWAntToAdd , &numOfSdesCapsWeWantToAdd, FALSE);

	}
	else
	{
		CSmallString str;
		str << "Struct size is " << structSize << " which is not enough for " << totalSize << " bytes to write on";
		PTRACE2(eLevelError,"CSipCaps::AddCapsToCapStruct ",str.GetString());
		DBGPASSERT(YES);
	}

	//free memory
	if ( numOfInitCaps )
	{
		for(unsigned int i = 0; i < numOfInitCaps; i++)
		{
			POBJDELETE(initCapsPtrArr[i]);
		}

		PDELETEA(initCapsPtrArr);
	}

	POBJDELETE(pTempCaps);

	pMediaLine->lenOfDynamicSection = offsetWrite - sizeof(sipMediaLineBaseSt);

	return offsetWrite;
}

/////////////////////////////////////////////////////////////////////////////////
void CSipCaps::RemoveSdesCaps(cmCapDataType mediaType, ERoleLabel eRole)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	CSdesCap* pSdesCap = NULL;
	GetSdesMediaCaps(mediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
	PTRACE2INT(eLevelInfoNormal,"CSipCaps::RemoveSdesCaps numOfMediaCapSet= ",numOfMediaCapSet);

	int numOfRemovedCaps = 0;
	if (pMediaCapList)
	{
		for (int j = 0; j < numOfMediaCapSet; j++)
		{
			PTRACE2INT(eLevelInfoNormal,"CSipCaps::RemoveSdesCaps j = ",j);
			if ((pMediaCapList[j-numOfRemovedCaps] != NULL) && (CapEnum)pMediaCapList[j-numOfRemovedCaps]->capTypeCode == eSdesCapCode)
			{
				PTRACE(eLevelInfoNormal,"CSipCaps::RemoveSdesCaps in SdesCap");
				pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[j-numOfRemovedCaps]->dataCap);
				if (pSdesCap)
				{
					PTRACE(eLevelInfoNormal,"CSipCaps::RemoveSdesCaps - remove");
					//	RemoveCapSetForSdes(mediaType,j,eRole);
					for(int  k= j-numOfRemovedCaps; k < numOfMediaCapSet - 1; k++)
					{
						PTRACE2INT(eLevelInfoNormal,"CSipCaps::RemoveSdesCaps k = ",k);
						pMediaCapList[k] = pMediaCapList[k+1];
					}
					numOfRemovedCaps++;
					numOfMediaCapSet--;
					PTRACE2INT(eLevelInfoNormal,"CSipCaps::RemoveSdesCaps numOfMediaCapSet = ",numOfMediaCapSet);
					pMediaCapList[numOfMediaCapSet] = NULL; // the last is empty now
					SetNumOfSdesCapSets(numOfMediaCapSet, mediaType,eRole);
				}
				else
					PTRACE(eLevelInfoNormal,"CSipCaps::RemoveSdesCaps - pSdesCap is NULL");

				POBJDELETE(pSdesCap);
			}
		}
	}
}

// copy sdes caps from pMediaLine to sdesCapSet
/////capBuffer*  m_msftAVMCUSdesCaps[MaxMsftSvcSdpVideoMlines]
/*
 * lineIndex  - index 0-5 of video mline
 * pVideoMediaLine - pointer to mline in sdp
 *
 */
BYTE CSipCaps::GetVideoMLineSdesCapSetForAVMCU(DWORD lineIndex, const sipMediaLineSt *pVideoMediaLine)
{
	if(! pVideoMediaLine) return false;
	FPASSERT_AND_RETURN_VALUE( ( lineIndex >= MaxMsftSvcSdpVideoMlines), FALSE);
	TRACEINTO << "DBG lineIndex:" << lineIndex;

	BYTE thereIsSdesCap = FALSE;
	const capBuffer* pCapBuffer = (capBuffer*) &pVideoMediaLine->caps[0];
	const BYTE*	pTemp           = (const BYTE*)pCapBuffer;
	DWORD numSdesCapsInLine = 0;

	for (unsigned int i = 0 ; i < pVideoMediaLine->numberOfCaps && 0 == numSdesCapsInLine; i++) // save the first sdes in media line
	{
		if ((CapEnum)pCapBuffer->capTypeCode == eSdesCapCode)
		{
			thereIsSdesCap = TRUE;

			m_msftAVMCUSdesCaps[lineIndex] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapBuffer->capLength]);
			memset(m_msftAVMCUSdesCaps[lineIndex], 0, sizeof(capBufferBase) + pCapBuffer->capLength);

			m_msftAVMCUSdesCaps[lineIndex]->capLength		= pCapBuffer->capLength;
			m_msftAVMCUSdesCaps[lineIndex]->capTypeCode		= pCapBuffer->capTypeCode;
			m_msftAVMCUSdesCaps[lineIndex]->sipPayloadType 	= pCapBuffer->sipPayloadType;

			memcpy(m_msftAVMCUSdesCaps[lineIndex]->dataCap, pCapBuffer->dataCap,pCapBuffer->capLength);

			m_numOfMsftAVMCUSdesCaps ++;
			numSdesCapsInLine ++ ;
		}
		pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTemp;
	}

	return thereIsSdesCap;
}

/////////////////////////////////////////////////////////////////////////////////
void CSipCaps::RemoveSdesCapsDifferentFromCryptoSuiteAndMKI(APIU16 cryptoSuite, BOOL bIsMkiInUse , cmCapDataType mediaType, ERoleLabel eRole)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	CSdesCap* pSdesCap = NULL;
	GetSdesMediaCaps(mediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
	PTRACE2INT(eLevelInfoNormal,"CSipCaps::RemoveSdesCapsDifferentFromCryptoSuiteAndMKI numOfMediaCapSet= ",numOfMediaCapSet);

	int numOfRemovedCaps = 0;
	if (pMediaCapList)
	{
		for (int j = 0; j < numOfMediaCapSet; j++)
		{
			PTRACE2INT(eLevelInfoNormal,"CSipCaps::RemoveSdesCapsDifferentFromCryptoSuiteAndMKI j = ",j);
			if ((pMediaCapList[j-numOfRemovedCaps] != NULL) && (CapEnum)pMediaCapList[j-numOfRemovedCaps]->capTypeCode == eSdesCapCode)
			{
				PTRACE(eLevelInfoNormal,"CSipCaps::RemoveSdesCapsDifferentFromCryptoSuiteAndMKI in SdesCap");
				pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[j-numOfRemovedCaps]->dataCap);
				if (pSdesCap)
				{
					if((cryptoSuite != eSha1_length_80_32 && pSdesCap->GetSdesCryptoSuite() != cryptoSuite)
						|| (pSdesCap->GetIsSdesMkiInUse(0) != bIsMkiInUse))
					{
						PTRACE(eLevelInfoNormal,"CSipCaps::RemoveSdesCapsDifferentFromCryptoSuiteAndMKI != cryptoSuite");
						//	RemoveCapSetForSdes(mediaType,j,eRole);
						for(int  k= j-numOfRemovedCaps; k < numOfMediaCapSet - 1; k++)
						{
							PTRACE2INT(eLevelInfoNormal,"CSipCaps::RemoveSdesCapsDifferentFromCryptoSuiteAndMKI k = ",k);
							pMediaCapList[k] = pMediaCapList[k+1];
						}
						numOfRemovedCaps++;
						numOfMediaCapSet--;
						PTRACE2INT(eLevelInfoNormal,"CSipCaps::RemoveSdesCapsDifferentFromCryptoSuiteAndMKI numOfMediaCapSet = ",numOfMediaCapSet);
						pMediaCapList[numOfMediaCapSet] = NULL; // the last is empty now
						SetNumOfSdesCapSets(numOfMediaCapSet, mediaType,eRole);


					}
				}
				else
					PTRACE(eLevelInfoNormal,"CSipCaps::RemoveSdesCapsDifferentFromCryptoSuiteAndMKI - pSdesCap is NULL");

				POBJDELETE(pSdesCap);
			}
		}

	}
}

///////////////////////////////////////////////////////////////////////////////////
void CSipCaps::UpdateSdesTagFromBestMode(CSdesCap *mSdesCap, cmCapDataType mediaType, ERoleLabel eRole)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	CSdesCap* pSdesCap = NULL;
	GetSdesMediaCaps(mediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
	PTRACE2INT(eLevelInfoNormal,"CSipCaps::UpdateSdesTagFromBestMode numOfMediaCapSet= ",numOfMediaCapSet);

	if (pMediaCapList)
	{
		for (int j = 0; j < numOfMediaCapSet; j++)
		{

			PTRACE(eLevelInfoNormal,"CSipCaps::UpdateSdesTagFromBestMode in SdesCap");
			pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[j]->dataCap);
			if (pSdesCap)
			{
				if(pSdesCap->GetSdesCryptoSuite() == mSdesCap->GetSdesCryptoSuite())
				{
					pSdesCap->SetSdesTag(mSdesCap->GetSdesTag());
				}
			}
			else
				PTRACE(eLevelInfoNormal,"CSipCaps::UpdateSdesTagFromBestMode - pSdesCap is NULL");

			POBJDELETE(pSdesCap);
		}
	}
}

/*
 * parameters:
 *
 * eMediaType
 * arrIndex - index of the cap in the current object, NA / -1 - not available
 * eDirection
 * bSetOppositeDirection
 * pCapabilities - MediaLinesEntrySt where the cap will be added
 * structSize
 * bfcpTransportType
 * eRole
 *
 * description:
 *		adds single cap to pCapabilities ( sipMediaLinesEntrySt ).
 *		the cap allowed to be of any media type except for bfcp.
 *
 *		create CSipCaps object -pCapsWithSingleCapSet-  which represents the needed cap,
 *		call AddCapsToCapStruct on it (with addBfcp set to NO).
 *
 */

///////////////////////////////////////////////////////////////////////////////////
int	CSipCaps::AddSingleCapToCapStruct(cmCapDataType eMediaType, int arrIndex, cmCapDirection eDirection, BYTE bSetOppositeDirection, sipMediaLinesEntrySt* pCapabilities, int structSize, eMediaLineSubType bfcpTransportType, ERoleLabel eRole) const // V4.1c <--> V6 merge const
{
	int offsetWrite	= 0;
	CSipCaps* pCapsWithSingleCapSet = new CSipCaps;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;

	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (arrIndex < numOfMediaCapSet)
	{
		pCapsWithSingleCapSet->AddCapSet(eMediaType, pMediaCapList[arrIndex]);
		offsetWrite = pCapsWithSingleCapSet->AddCapsToCapStruct(eDirection, bSetOppositeDirection, pCapabilities, structSize, bfcpTransportType,YES,YES,YES,YES,NO);
	}

	POBJDELETE(pCapsWithSingleCapSet);
	return offsetWrite;
}


///////////////////////////////////////////////////////////////////////////////////
int	CSipCaps::AddMediaToCapStruct(cmCapDataType eMediaType, cmCapDirection eDirection, BYTE bSetOppositeDirection, sipMediaLinesEntrySt* pCapabilities, int structSize, eMediaLineSubType bfcpTransportType) const
{
	int offsetWrite	= 0;
	CSipCaps* pCapsWithOneMedia = new CSipCaps;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;

	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList);

	for(int i = 0; i < numOfMediaCapSet; i++)
	{
		pCapsWithOneMedia->AddCapSet(eMediaType, pMediaCapList[i]);
	}

	offsetWrite = pCapsWithOneMedia->AddCapsToCapStruct(eDirection, bSetOppositeDirection, pCapabilities, structSize, bfcpTransportType);
	POBJDELETE(pCapsWithOneMedia);

	return offsetWrite;
}


//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsMediaMuted(cmCapDataType eMediaType, cmCapDirection eDirection, ERoleLabel eRole) const
{
	BYTE bIsMuted = NO;

	if (IsMedia(eMediaType, cmCapReceiveAndTransmit, eRole)) // if not exist - not muted
	{
		int numOfMediaCapSet		= 0;
		capBuffer** pMediaCapList	= NULL;
		GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
		bIsMuted = YES; // start from positive answer unless find an un-muted cap
		for (int i = 0; (i < numOfMediaCapSet) && bIsMuted; i++)
		{
			CBaseCap* pCurCap = GetCapSet(eMediaType,i,eRole);

			if (pCurCap)
			{
				//condition 1:
				if (pCurCap->IsDirection(eDirection))
					bIsMuted = NO; // found at least one cap that is not muted
				//condition 2:
//				else if (eMediaType ==  cmCapVideo)
//				{
//					if (pCurCap->GetBitRate() != 0)
//						bIsMuted = NO; // found at least one cap that is not muted
//				}
			}

			POBJDELETE(pCurCap);
		}
	}

	return bIsMuted;
}


//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::GetDirection(cmCapDataType eMediaType, ERoleLabel eRole, BOOL fullDirectionIfNotExist) const
{
	BYTE direction = (fullDirectionIfNotExist) ? cmCapReceiveAndTransmit: 0;

	if ( IsMedia(eMediaType,cmCapReceiveAndTransmit,eRole) ) // if not exist - not muted
	{
		int numOfMediaCapSet		= 0;
		capBuffer** pMediaCapList	= NULL;
		GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			CBaseCap* pCurCap = GetCapSet(eMediaType,i,eRole);

			if (pCurCap)
			{
				direction =  pCurCap->GetDirection();
				POBJDELETE(pCurCap);
			}
			break;
		}
	}
	PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetDirection Direction: ",direction);
	return direction;
}
///////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsContainingPayloadType(const CCapSetInfo& capInfo, APIU8 uiPayloadType, int* pArrInd, ERoleLabel eRole) const
{
	BYTE res = NO;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	cmCapDataType eMediaType = capInfo.GetSipCapType();
	*pArrInd = NA;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if ( pMediaCapList )
	{
		for (int i = 0; i < numOfMediaCapSet && res == NO; i++)
		{
			res = (pMediaCapList[i]->capTypeCode == (CapEnum)capInfo && pMediaCapList[i]->sipPayloadType == uiPayloadType);
			if (res)
			{
				*pArrInd = i;
			}
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsCapsContainingVideo() const
{
	BYTE        res              = FALSE;
	int         numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	 = NULL;

	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList, kRolePeople);

	if ( pMediaCapList && (numOfMediaCapSet > 0) )
		res = TRUE;

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
////FPS by UserAgent
//	//
//	//Change for MS Windows messenger MPI value
//	//soft client drop out packets in high FPS, so report lower limit FPS to bridge
//	/*
//		1. Must be defined value for FPS (MPI) in system.cfg or via terminal
//		2. If UserAgent is void - fix for all parties
//	*/
BYTE CSipCaps::FixFPSByUserAgent(const char* cUserAgent)
{
	BYTE bRetVal = NO;
	BYTE bFixFPS = NO;
	DWORD dwFPSFixTo;
	std::string sdata;
	ALLOCBUFFER(cSkipReason, 160);

	//vars for fixing MPS
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;

	//if defined to new value to set
	dwFPSFixTo = GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_FPS_FIX_TO);
	if ( dwFPSFixTo > 0)
	{
		//if defined to change FPS, check if it's asked for explicit remote
		sdata = GetSystemCfgFlagStr<std::string>(CFG_KEY_SIP_USER_AGENT_FPS_FIX);
		if ( sdata.length() > 0 )
		{
			//compare the remote to setting, not equal
			if ( NULL != strstr(cUserAgent, sdata.c_str()) )
			{
				bFixFPS = YES;
			}
			else
			{
				sprintf(cSkipReason, "Not an agent");
			}
		}
		else
		{
			bFixFPS = YES;
		}

		if ( YES == bFixFPS)
		{
			int iMPI = 30 / dwFPSFixTo;

			//relevant only for video transmited to remote
			if ( IsMedia(cmCapVideo) )
			{
				numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
				for ( int i = 0; i < numOfVideoMediaCapSet; i++)
				{
					CBaseVideoCap* pVideoCap = (CBaseVideoCap*) GetCapSet(cmCapVideo, i);

					if (!pVideoCap)
					    PTRACE(eLevelInfoNormal,"CSipCaps::FixFPSByUserAgent - pVideoCap is NULL");
					//change only if remote does not declared FPS for resolution
					else if ( -1 == pVideoCap->GetFormatMpi(kCif) )
					{
						if ( strlen(cSkipReason) < 128 )
						{
							sprintf(cSkipReason, "%s <%s to %d>", cSkipReason,
									CapEnumToString((CapEnum)pVideoCap->GetCapCode()), dwFPSFixTo);
						}

						pVideoCap->SetFormatMpi(kCif, iMPI);
						//update QCIF too because Intersection of VideoBaseCap
						//on create highest mode (assign higher resolution MPI if not
						//defined to current resolution
						if ( -1 == pVideoCap->GetFormatMpi(kQCif) )
						{
							pVideoCap->SetFormatMpi(kQCif, 1);
						}
						if ( -1 == pVideoCap->GetFormatMpi(kQCif) )
						{
							pVideoCap->SetFormatMpi(k4Cif, -1);
							sprintf(cSkipReason, "removing 4cif");

						}

						bRetVal = YES;
					}
					else
						sprintf(cSkipReason, "defined by remote");
					POBJDELETE(pVideoCap);
				}
			}
			else
			{
				sprintf(cSkipReason, "No video");
			}
		}
	}
	else
	{
		sprintf(cSkipReason, "%s", "CSipCaps::GetSipFPSFixTo <= 0");
	}

	ALLOCBUFFER(cLog, 256);
	memset(cLog, 0, 256);
	sprintf(cLog, "for Agent [%.15s] is [%s] note [%s]", cUserAgent ? cUserAgent:"Unknown", bRetVal ? "made":"skipped", cSkipReason);
	FPTRACE2(eLevelInfoNormal, "CSipCaps::FixFPSByUserAgent ", cLog);
	DEALLOCBUFFER(cLog);
	DEALLOCBUFFER(cSkipReason);
	return bRetVal;
}


///////////////////////////////////////////////////////////////////////////////////
//Correct remote caps according to known limitations and system settings.
//return YES if something is changed
BYTE CSipCaps::FixRemoteCapsBySystemSettings(sipMessageHeaders* pSipHeaders, BYTE bFixVideoRate)
{
	BYTE bRetVal = NO;
	CSipHeader* pHeader = NULL;
	char cHeaderValue[256] = {0};
	PTRACE(eLevelInfoNormal, "CSipCaps::FixRemoteCapsBySystemSettings");

	//get UserAgent header for FPS fix
	if (pSipHeaders)
	{
		CSipHeaderList cHeaders(*pSipHeaders);
		pHeader	= (CSipHeader*) cHeaders.GetNextHeader(kUserAgent);

		if ( pHeader )
		{
			strncpy(cHeaderValue, pHeader->GetHeaderStr(), sizeof(cHeaderValue)-1);
			cHeaderValue[sizeof(cHeaderValue)-1] = '\0';
		}
	}
	else
		return bRetVal;

	bRetVal = FixFPSByUserAgent(cHeaderValue);

	if(bFixVideoRate)
		bRetVal = FixVideoBitrateAndResolution();

	return bRetVal;
}


///////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::FixVideoBitrateAndResolution()
{
	BYTE bRetVal = NO;
	ALLOCBUFFER(cSkipReason, 50);
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;
	DWORD dwNewBitRate = GetSystemCfgFlagInt<DWORD>(CFG_KEY_IP_MOBILE_PHONE_RATE);

	if ( IsMedia(cmCapVideo) && dwNewBitRate)
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
		for ( int i = 0; i < numOfVideoMediaCapSet; i++)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*) GetCapSet(cmCapVideo, i);
			if (pVideoCap && (pVideoCap->GetBitRate() == -1) )
			{
				pVideoCap->SetBitRate(dwNewBitRate*10);
				bRetVal = YES;
			}
			FixCIFDeclarationForCP();
			POBJDELETE(pVideoCap);
		}
	}
	else
	{
		sprintf(cSkipReason, "No video");
	}

	ALLOCBUFFER(cLog, 256);
	memset(cLog, 0, 256);
	sprintf(cLog, " [%s] requested bit rate is [%d]Kb note [%s]", bRetVal ? "fixed":"skipped", dwNewBitRate, cSkipReason);
	FPTRACE2(eLevelInfoNormal, "CSipCaps::FixVideoBitrate ", cLog);
	DEALLOCBUFFER(cSkipReason);
	DEALLOCBUFFER(cLog);
	return bRetVal;
}


///////////////////////////////////////////////////////////////////////////////////
//Disable CIF in CP conferences up bit rate specified in system.cfg
//return YES if changed
BYTE CSipCaps::FixCIFDeclarationForCP()
{
	BYTE bRetVal = NO;

	//vars for fixing MPS
	int numOfVideoMediaCapSet = 0;

	//if defined new value to set
	DWORD dwRate = GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_CIF_IN_CP_FROM_RATE);
	if ( dwRate > 0 )
	{
		if ( IsMedia(cmCapVideo) )
		{
			numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
			for ( int i = 0; i < numOfVideoMediaCapSet; i++)
			{
				CBaseVideoCap* pVideoCap = (CBaseVideoCap*) GetCapSet(cmCapVideo, i);
				//change only if remote does not declared FPS for resolution
				if (pVideoCap && ((DWORD)pVideoCap->GetBitRate() <= dwRate*10) )
				{
					BYTE rval = NO;
					rval = pVideoCap->SetFormatMpi(kCif, -1);
					if(rval)
						PTRACE(eLevelInfoNormal,"CSipCaps::FixCIFDeclarationForCP - remove Cif from local caps");
					bRetVal = YES;
				}
				POBJDELETE(pVideoCap);
			}
		}
	}

	return bRetVal;
}


//////////////////////////////////////////////////////////////////////////////////
//Function is checking rate in received capabilities and if it's equal
//the passed param return true
//Different caps type rate is added, same type is override with biggest number
long CSipCaps::GetTotalRate(void) const
{
	long dAudioRate = -1; //as undefined
	long dVideoRate = -1; //as undefined

	for (int i=0; i < m_numOfAudioCapSets; i++)
	{
		CCapSetInfo capInfo = (CapEnum) m_audioCapList[i]->capTypeCode;

		if(capInfo.IsType(cmCapAudio))
		{
			dAudioRate = max((long) capInfo.GetBitRate(&m_audioCapList[i]->dataCap[0]), dAudioRate);
		}
	}

	bool is_rtv = false;
	bool is_h264 = false;
	bool is_MsSvc = false;

	for (int i = 0; i < m_numOfVideoCapSets; i++)
	{
		CCapSetInfo capInfo = (CapEnum)m_videoCapList[i]->capTypeCode;
		CBaseVideoCap* pVideoCap = (CBaseVideoCap *) CBaseCap::AllocNewCap(capInfo, m_videoCapList[i]->dataCap);

		if (pVideoCap)
		{
			if ((CapEnum)capInfo == eRtvCapCode)
			{
				if (!is_h264)
				{
					//in RTV get the cap rate only from the RTV caps
					//dVideoRate = (long)pVideoCap->GetBitRate();
					if(dVideoRate != (long)-1 && is_MsSvc)
						dVideoRate =  max((long)pVideoCap->GetBitRate(),dVideoRate);
					else
						dVideoRate = (long)pVideoCap->GetBitRate();

					PTRACE2INT(eLevelInfoNormal, "CSipCaps::GetTotalRate setting video rate as RTV rate =",dVideoRate);
					is_rtv = true;
				}
			}
			else
			{
				if((CapEnum)capInfo == eH264CapCode)
				{
					is_h264 = true;
					if (is_rtv) //in case we previously took RTV rate, overrun it with h264 rate
					{
						dVideoRate = (long)pVideoCap->GetBitRate();
						PTRACE2INT(eLevelInfoNormal, "CSipCaps::GetTotalRate setting video rate as h264 dVideoRate =",(CapEnum)dVideoRate);

					}
				}
				else if((CapEnum)capInfo == eMsSvcCapCode)
				{
					is_MsSvc = TRUE;
					if ( ((long)pVideoCap->GetBitRate()) > 0  ) //in case we previously took RTV rate, overrun it with ms svc rate
					{
						dVideoRate = max((long)pVideoCap->GetBitRate(),dVideoRate);
						PTRACE2INT(eLevelInfoNormal, "CSipCaps::GetTotalRate setting video rate as ms svc and rtv max dVideoRate =",(CapEnum)dVideoRate);

					}

				}


				if(((CapEnum)capInfo == eH263CapCode) && is_rtv)
				{
					//do nothing on h263 when RTV is present
				}
				else
				{//N.A. DEBUG VP8 - prints
					dVideoRate = max((long) pVideoCap->GetBitRate(), dVideoRate);
				}
			}
		}
		POBJDELETE(pVideoCap);
	}


	if (is_rtv && !is_h264)
	{
		dAudioRate = CalculateAudioRate(dVideoRate*100);
		PTRACE2INT(eLevelInfoNormal, "CSipCaps::GetTotalRate setting audio rate as RTV rate =",dAudioRate);
	}

	//alignment with video
	if ( 0 < dAudioRate ) {
		dAudioRate = dAudioRate / 100;
	}

	if( m_msftVideoRxBw !=0 &&   0xFFFFFFFF != m_msftVideoRxBw && dVideoRate != -1)
	{
			FPTRACE2INT(eLevelInfoNormal, "CSipCaps::GetTotalRate m_msftVideoRxBw ", m_msftVideoRxBw);
			dVideoRate =  min( ((long)m_msftVideoRxBw), dVideoRate );
	}

	DWORD totalrate = 0; // Fecc rate should not be calculate.
	if (dAudioRate != -1)
	{
		totalrate += dAudioRate;
	}
	if (dVideoRate != -1)
	{
		totalrate += dVideoRate;
	}

	ALLOCBUFFER(cLog, 256);
	memset(cLog, 0, 256);
	sprintf(cLog, "TotalRate is [%d] = dAudioRate is [%ld] in [%d] sets + dVideoRate is [%ld] in [%d] sets, the AS= rate is [%d]",
			totalrate, dAudioRate, m_numOfAudioCapSets, dVideoRate, m_numOfVideoCapSets, m_sessionLevelRate);
	FPTRACE2(eLevelInfoNormal, "CSipCaps::GetTotalRate ", cLog);
	DEALLOCBUFFER(cLog);
	if ( ( 0xFFFFFFFF != m_sessionLevelRate ) && ( 0xFFFFFFFF != m_sessionLevelRate ) && ( 0 < m_sessionLevelRate ))
	{
        //Jason.MA : some times the totalrate ( the sum of media level rate ) was greater than the session level rate, 
        //           and some times the dVideoRate was set to the m_AllocatedBandwidth ( for ICE ) which result in that the totalrate was less than the session level rate.
        //           we choose the the minimal value of the session level rate and the totalrate.

        
		return min( m_sessionLevelRate, totalrate );

	}

	return totalrate;
}


///////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetNumOfCapSets(int num, cmCapDataType eMediaType, ERoleLabel eRole)
{
	if (eMediaType == cmCapAudio)
		m_numOfAudioCapSets = num;
	else if (eMediaType == cmCapVideo)
	{
		if (eRole==kRolePeople)
			m_numOfVideoCapSets = num;
		else
			m_numOfContentCapSets = num;
	}
	else if (eMediaType == cmCapData)
		m_numOfFeccCapSets = num;
	else if (eMediaType == cmCapBfcp)
			m_numOfBfcpCapSets = num;
}

///////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetNumOfSdesCapSets(int num, cmCapDataType eMediaType, ERoleLabel eRole)
{
	if (eMediaType == cmCapAudio)
		m_numOfAudioSdesCapSets = num;
	else if (eMediaType == cmCapVideo)
	{
		if (eRole==kRolePeople)
			m_numOfVideoSdesCapSets = num;
		else
			m_numOfContentSdesCapSets = num;
	}
	else if (eMediaType == cmCapData)
		m_numOfFeccSdesCapSets = num;
}

///////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetNumOfIceCapSets(int num, ICESessionsTypes IceSessionsTypes)
{
	if (IceSessionsTypes == eAudioSession)
		m_numOfIceAudioCaps = num;
	else if (IceSessionsTypes == eVideoSession)
		m_numOfIceVideoCaps = num;
	else if (IceSessionsTypes == eDataSession)
		m_numOfIceDataCaps = num;
	else if (IceSessionsTypes == eGeneralSession)
		m_numOfIceGeneralCaps = num;
}

///////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsCapableOfHD720() const
{
	BYTE bIsHD720 = FALSE;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;

	if (IsMedia(cmCapVideo))
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
		for (int i = 0; (i < numOfVideoMediaCapSet) && !bIsHD720; i++)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*)GetCapSet(cmCapVideo, i);
			if (pVideoCap)
			{
			    bIsHD720 = pVideoCap->IsCapableOfHD720();
			    POBJDELETE(pVideoCap);
			}
			else
			    PTRACE(eLevelInfoNormal,"CSipCaps::IsCapableOfHD720 - pVideoCap is NULL");
		}
	}
	return bIsHD720;
}

///////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsCapableOfHD1080() const
{
	BYTE bIsHD1080 = FALSE;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;

	if (IsMedia(cmCapVideo))
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
		for (int i = 0; (i < numOfVideoMediaCapSet) && !bIsHD1080; i++)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*)GetCapSet(cmCapVideo, i);
			if (pVideoCap)
			{
			    bIsHD1080 = pVideoCap->IsCapableOfHD1080();
			    POBJDELETE(pVideoCap);
			}
			else
			    PTRACE(eLevelInfoNormal,"CSipCaps::IsCapableOfHD1080 - pVideoCap is NULL");
		}
	}
	return bIsHD1080;
}
//////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsCapableOfHDContent720() const
{
	BYTE 		hd720Mpi  = 0;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;
	// PTRACE(eLevelInfoNormal,"SipCaps::IsCapableOfHDContent720. Start");

	if (IsMedia(cmCapVideo,cmCapReceiveAndTransmit,kRolePresentation))
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo,cmCapReceiveAndTransmit,kRolePresentation);
		for (int i = 0; (i < numOfVideoMediaCapSet) && !hd720Mpi; i++)
		{
		  CBaseVideoCap* pVideoCap = (CBaseVideoCap*)GetCapSet(cmCapVideo,i,kRolePresentation);
		  if(pVideoCap && (pVideoCap->GetCapCode() == eH264CapCode) && pVideoCap->IsCapableOfHD720(kRolePresentation) )
		  {
		      hd720Mpi = ((CH264VideoCap *)pVideoCap)->GetH264Mpi();
		  }
		  POBJDELETE(pVideoCap);
		}
	}
	PTRACE2INT(eLevelInfoNormal,"SipCaps::IsCapableOfHDContent720. HD 720 Mpi is: ",hd720Mpi);
   return hd720Mpi;
}
//////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsCapableOfHDContent1080() const
{
	BYTE 		hd1080Mpi  = 0;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;
	// PTRACE(eLevelInfoNormal,"SipCaps::IsCapableOfHDContent1080. Start");
	if (IsMedia(cmCapVideo,cmCapReceiveAndTransmit,kRolePresentation))
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo,cmCapReceiveAndTransmit,kRolePresentation);
		for (int i = 0; (i < numOfVideoMediaCapSet) && !hd1080Mpi; i++)
		{
		  CBaseVideoCap* pVideoCap = (CBaseVideoCap*)GetCapSet(cmCapVideo, i,kRolePresentation);
		  if (pVideoCap && (pVideoCap->GetCapCode() == eH264CapCode) && pVideoCap->IsCapableOfHD1080() )
		  {
			  hd1080Mpi = ((CH264VideoCap *)pVideoCap)->GetH264Mpi();
		  }
		  POBJDELETE(pVideoCap);

		}
	}
	PTRACE2INT(eLevelInfoNormal,"SipCaps::IsCapableOfHDContent1080. HD 1080 Mpi is: ",hd1080Mpi);
   return hd1080Mpi;
}
///////////////////////////////////////////////////////////////////////////////////////////
/*
BYTE CSipCaps::IsSupportHighProfile() const
{
//	BYTE bIsHD1080 = FALSE;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;

	if (IsMedia(cmCapVideo))
	{
		PTRACE(eLevelInfoNormal,"CSipCaps::GetCPVideoPartyType MEDIA IS ");
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
		for (int i = 0; i < numOfVideoMediaCapSet; i++)
		{
			PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetCPVideoPartyType MEDIA IS num of video ",numOfVideoMediaCapSet);
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eH264CapCode )
			{
				PTRACE(eLevelInfoNormal,"CSipCaps::GetCPVideoPartyType MEDIA IS num of video -H264 ");
				CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
				if(pH264VideoCap && pH264VideoCap->GetRole() == kRolePeople && pH264VideoCap->GetProfile() == H264_Profile_High)
				{	PTRACE(eLevelInfoNormal,"CSipCaps::GetCPVideoPartyType MEDIA IS num of video -H264 HIGH ");
					POBJDELETE(pH264VideoCap);
					return TRUE;
				}

				POBJDELETE(pH264VideoCap);
			}
		}
	}
	return FALSE
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////

BYTE CSipCaps::IsSupportHighProfile() const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if (IsH264Video((CapEnum)pMediaCapList[i]->capTypeCode))
			{
				CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
				if (pH264VideoCap && (pH264VideoCap->GetRole() == kRolePeople) && (pH264VideoCap->GetProfile() == H264_Profile_High) )
				{
					POBJDELETE(pH264VideoCap);
					return TRUE;
				}
				POBJDELETE(pH264VideoCap);
			}
		}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsSupportMainProfile() const
{
    int numOfMediaCapSet = 0;
    capBuffer** pMediaCapList = NULL;
    GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

    if (pMediaCapList)
    {
        for (int i = 0; i < numOfMediaCapSet; i++)
        {
            if ((CapEnum)pMediaCapList[i]->capTypeCode == eH264CapCode)
            {
                CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
                if (pH264VideoCap && (pH264VideoCap->GetRole() == kRolePeople) && (pH264VideoCap->GetProfile() == H264_Profile_Main) )
                {
                    POBJDELETE(pH264VideoCap);
                    return TRUE;
                }
                POBJDELETE(pH264VideoCap);
            }
        }
    }
    return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsSupportBaseProfile() const
{
    int numOfMediaCapSet = 0;
    capBuffer** pMediaCapList = NULL;
    GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

    if (pMediaCapList)
    {
        for (int i = 0; i < numOfMediaCapSet; i++)
        {
            if ((CapEnum)pMediaCapList[i]->capTypeCode == eH264CapCode)
            {
                CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
                if (pH264VideoCap && (pH264VideoCap->GetRole() == kRolePeople) && (pH264VideoCap->GetProfile() == H264_Profile_BaseLine) )
                {
                    POBJDELETE(pH264VideoCap);
                    return TRUE;
                }
                POBJDELETE(pH264VideoCap);
            }
        }
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsCapableOfHD720At50()const
{
	BYTE bIsHD720At50 = FALSE;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;

	if (IsMedia(cmCapVideo))
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
		for (int i = 0; (i < numOfVideoMediaCapSet) && !bIsHD720At50; i++)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*)GetCapSet(cmCapVideo, i);
			if (pVideoCap)
			    bIsHD720At50 = pVideoCap->IsCapableOfHD720At50();
			else
			    PTRACE(eLevelInfoNormal,"CSipCaps::IsCapableOfHD720At50 - pVideoCap is NULL");
			POBJDELETE(pVideoCap);
		}
	}
	return bIsHD720At50;
}
//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsCapableOfHD1080At60()const
{
	BYTE bIsHD1080At60 = FALSE;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;

	if (IsMedia(cmCapVideo))
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
		for (int i = 0; (i < numOfVideoMediaCapSet) && !bIsHD1080At60; i++)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*)GetCapSet(cmCapVideo, i);
			if(pVideoCap)
			{
				bIsHD1080At60 = pVideoCap->IsCapableOfHD1080At60();
				POBJDELETE(pVideoCap);
			}
		}
	}
	return bIsHD1080At60;
}
//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsHighProfileContent() const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList, kRolePresentation);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if (IsH264Video((CapEnum)pMediaCapList[i]->capTypeCode))
			{
				CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
				if (pH264VideoCap && (pH264VideoCap->GetRole() & kRoleContentOrPresentation) && (pH264VideoCap->GetProfile() == H264_Profile_High) )
				{
					POBJDELETE(pH264VideoCap);
					return TRUE;
				}
				POBJDELETE(pH264VideoCap);
			}
		}
	}
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////////

eVideoPartyType CSipCaps::GetCPVideoPartyType()const
{
	eVideoPartyType resVideoPartyType = eVideo_party_type_none;
	eVideoPartyType tmpVideoPartyType = eVideo_party_type_none;
	int numOfVideoMediaCapSet = 0;
	if (IsMedia(cmCapVideo))
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
		for (int i = 0; (i < numOfVideoMediaCapSet); i++)
		{
			//COstrStream msg;
			//pVideoCap->Dump(msg);
			//PTRACE2(eLevelInfoNormal,"CSipCaps::GetCPVideoPartyType -cap is ",msg.str().c_str());
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*)GetCapSet(cmCapVideo, i);
			if (pVideoCap)
			{
			    tmpVideoPartyType = pVideoCap->GetCPVideoPartyType();
			    if(resVideoPartyType<tmpVideoPartyType)
			        resVideoPartyType = tmpVideoPartyType;
			    //PTRACE2INT(eLevelInfoNormal,"CCapH323::GetCPVideoPartyType type is  ",(WORD)tmpVideoPartyType);
			    POBJDELETE(pVideoCap);
			}
			else
			    PTRACE(eLevelInfoNormal,"CSipCaps::GetCPVideoPartyType - pVideoCap is NULL");
		}
	}
	return resVideoPartyType;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetLevelAndAdditionals(APIU16 profile, APIU8 level, APIS32 mbps, APIS32 fs, APIS32 dpb ,APIS32 brAndCpb, APIS32 sar, APIS32 staticMB, ERoleLabel eRole)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if (IsH264Video((CapEnum)pMediaCapList[i]->capTypeCode))
			{
				CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
				if(pH264VideoCap && (pH264VideoCap->GetRole() == eRole) )
					pH264VideoCap->SetLevelAndAdditionals(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB);
				POBJDELETE(pH264VideoCap);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetLevelAndAdditionals(H264VideoModeDetails& h264VidModeDetails, ERoleLabel eRole)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if (IsH264Video((CapEnum)pMediaCapList[i]->capTypeCode))
			{
				CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
				if(pH264VideoCap && (pH264VideoCap->GetRole() == eRole) )
					pH264VideoCap->SetLevelAndAdditionals(h264VidModeDetails.profileValue, h264VidModeDetails.levelValue, h264VidModeDetails.maxMBPS, h264VidModeDetails.maxFS, h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR,-1, h264VidModeDetails.maxStaticMbps);
				//PASSERT(1);// SAR and also staticMB are not pass when rebuilding H264 due to re-allocation. the maxStaticMB in the structure is not handle even once in the code.
				POBJDELETE(pH264VideoCap);
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetLevelAndAdditionalsForMainProfile(H264VideoModeDetails& h264VidModeDetails, ERoleLabel eRole)
{
    int numOfMediaCapSet = 0;
    capBuffer** pMediaCapList = NULL;
    GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

    if (pMediaCapList)
    {
        for (int i = 0; i < numOfMediaCapSet; i++)
        {
            if ((CapEnum)pMediaCapList[i]->capTypeCode == eH264CapCode)
            {
                CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
                PASSERTMSG_AND_RETURN(pH264VideoCap==NULL, "AllocNewCap returned NULL");
                if(pH264VideoCap->GetProfile() == H264_Profile_Main)
                {
                    if(pH264VideoCap->GetRole() == eRole)
                        pH264VideoCap->SetLevelAndAdditionals(h264VidModeDetails.profileValue, h264VidModeDetails.levelValue, h264VidModeDetails.maxMBPS, h264VidModeDetails.maxFS, h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR,-1, h264VidModeDetails.maxStaticMbps);
                }
                //PASSERT(1);// SAR and also staticMB are not pass when rebuilding H264 due to re-allocation. the maxStaticMB in the structure is not handle even once in the code.
                POBJDELETE(pH264VideoCap);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetNewProfileInsteadOfOldProfile(APIU8 newProfile,APIU8 oldProfile)
{
    int numOfMediaCapSet = 0;
    capBuffer** pMediaCapList = NULL;
    GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

    if (pMediaCapList)
    {
        for (int i = 0; i < numOfMediaCapSet; i++)
        {
            if ((CapEnum)pMediaCapList[i]->capTypeCode == eH264CapCode)
            {
                CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
                if (pH264VideoCap && (pH264VideoCap->GetRole() == kRolePeople) && (pH264VideoCap->GetProfile() == oldProfile) )
                {
                    pH264VideoCap->SetProfile(newProfile);
                    POBJDELETE(pH264VideoCap);
                    return;
                }
                POBJDELETE(pH264VideoCap);
            }
        }
    }
    return;
}
///////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetRtvParams(RTVVideoModeDetails& rtvVidModeDetails, ERoleLabel eRole,DWORD videoRate, BYTE isForceFps)//add payload tomo
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

	if (0 == videoRate)
	{
		videoRate = GetMaxVideoBitRate(cmCapTransmit, eRole);
	}

	PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetRtvParams: videoRate=", videoRate);

	bool found = false;
	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eRtvCapCode)
			{
				found = true;
				CRtvVideoCap* pRtvVideoCap = (CRtvVideoCap *)CBaseCap::AllocNewCap(eRtvCapCode, pMediaCapList[i]->dataCap);
				if(pRtvVideoCap)
				{
					if(pRtvVideoCap->GetRole() == eRole)
					{
						pRtvVideoCap->SetDefaults(pRtvVideoCap->GetDirection(), eRole);
						pRtvVideoCap->SetRtvCapForCpFromRtvVideoType(rtvVidModeDetails,videoRate);
						if(TRUE == isForceFps)
						{
							PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetRtvParams - force fps!! : i=", i);
							pRtvVideoCap->SetFpsToAllItems(pRtvVideoCap->GetDirection(), eRole,rtvVidModeDetails.FR);
						}
					}
					POBJDELETE(pRtvVideoCap);
				}
			}
		}
	}
	if (!found)
	{
		PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetRtvParams: Adding new RTV cap videoRate=", videoRate);

		CRtvVideoCap* pRtvVideoCap = (CRtvVideoCap *)CBaseCap::AllocNewCap(eRtvCapCode, NULL);
		if(pRtvVideoCap)
		{
			pRtvVideoCap->SetDefaults(cmCapReceiveAndTransmit, eRole);
			pRtvVideoCap->SetRtvCapForCpFromRtvVideoType(rtvVidModeDetails,videoRate);
			capBuffer* tempcapbuffrer = NULL;
			tempcapbuffrer = pRtvVideoCap->GetAsCapBuffer();

			if (tempcapbuffrer)
			{
				tempcapbuffrer->sipPayloadType = eRtvDynamicPayload;
				AddCapSet(cmCapVideo, tempcapbuffrer);
				PDELETEA(tempcapbuffrer);
			}
			else
			{
				PTRACE(eLevelError,"CSipCaps::SetRtvParams: tempcapbuffrer is NULL");
			}

			pRtvVideoCap->FreeStruct();
			POBJDELETE(pRtvVideoCap);

		}
		else
			PASSERTMSG(1, "AllocNewCap return NULL");
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::GetMaxH264Level(ERoleLabel eRole) const
{
	BYTE curlevel;
	BYTE maxLevel = 0;

	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if (IsH264Video((CapEnum)pMediaCapList[i]->capTypeCode))
			{
				CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
				if (pH264VideoCap && (pH264VideoCap->GetRole() == eRole))
				{
					curlevel = pH264VideoCap->GetLevel();
					if (curlevel > maxLevel)
						maxLevel = curlevel;
				}
				POBJDELETE(pH264VideoCap);
			}
		}
	}
	return maxLevel;
}

//////////////////////////////////////////////////////////////////////////////////////////
CH264VideoCap* CSipCaps::GetMaxH264LevelCap(ERoleLabel eRole) const
{
    BYTE curlevel;
    BYTE maxLevel = 0;
    CH264VideoCap* pMaxLevelCap = NULL;

    int numOfMediaCapSet = 0;
    capBuffer** pMediaCapList = NULL;
    GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

    if (pMediaCapList)
    {
        for (int i = 0; i < numOfMediaCapSet; i++)
        {
            if (IsH264Video((CapEnum)pMediaCapList[i]->capTypeCode))
            {
                CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
                if (pH264VideoCap && (pH264VideoCap->GetRole() == eRole))
                {
                    curlevel = pH264VideoCap->GetLevel();
                    if (curlevel > maxLevel)
                    {
                        maxLevel = curlevel;
                        POBJDELETE(pMaxLevelCap);
                        pMaxLevelCap = pH264VideoCap;
                    }
                    else
                        POBJDELETE(pH264VideoCap);
                }
            }
        }
    }
    return pMaxLevelCap;
}

///////////////////////////////////////////////////////////////////////////////////////////
void  CSipCaps::GetMaxH264CustomParameters(BYTE level, WORD& maxMBPS, WORD& maxFS, WORD& maxDPB, WORD& maxBRandCPB, WORD& maxSAR, WORD& maxStaticMB, ERoleLabel eRole,DWORD profile)
{
	APIS32 mbps, fs, dpb, brAndCpb, sar, staticMB;
	mbps = fs = dpb = brAndCpb = sar = staticMB = -1;
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList,eRole);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if (IsH264Video((CapEnum)pMediaCapList[i]->capTypeCode))
			{
				CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
				if (pH264VideoCap && (pH264VideoCap->GetLevel() >= level) && ((profile == 0) || (profile != 0 && pH264VideoCap->GetProfile() == profile)))
				{
					pH264VideoCap->GetAdditionalsAsExplicit(mbps, fs, dpb, brAndCpb, sar, staticMB);
					maxMBPS		= max((signed long)maxMBPS, mbps);
					maxFS		= max((signed long)maxFS,   fs);
					maxDPB		= max((signed long)maxDPB,  dpb);
					maxBRandCPB = max((signed long)maxBRandCPB, brAndCpb);
					maxSAR		= max((signed long)maxSAR, sar);
					maxStaticMB	= max((signed long)maxStaticMB, staticMB);
				}
				POBJDELETE(pH264VideoCap);
			}
		}
	}
}
///////////////////////////////////////////////////
void CSipCaps::SetVideoRateInallCaps(DWORD newRate, ERoleLabel eRole)
{
	PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetPeopleRateInallCaps -num of video caps is ", m_numOfVideoCapSets);

	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;

	if ( IsMedia(cmCapVideo, cmCapReceiveAndTransmit, eRole) )
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo,cmCapReceiveAndTransmit,eRole);
		for ( int i = 0; i < numOfVideoMediaCapSet; i++)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*) GetCapSet(cmCapVideo, i, eRole);
			if(pVideoCap && (pVideoCap->GetRole() == eRole) )
				pVideoCap->SetBitRate(newRate);
			POBJDELETE(pVideoCap);
		}
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"No video caps found ", m_numOfVideoCapSets);

	}

}
///////////////////////////////////////////////////////////////////////////////
DWORD CSipCaps::GetVideoRateInMatchingCap(CBaseVideoCap* otherCap, ERoleLabel eRole)
{
	PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetVideoRateInMatchingCap -num of video caps is ", m_numOfVideoCapSets);
	DWORD newRate=0;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;
	Bool isFound = false;

	if ( IsMedia(cmCapVideo,cmCapReceiveAndTransmit,eRole) )
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo,cmCapReceiveAndTransmit,eRole);
		for ( int i = 0; i < numOfVideoMediaCapSet && !isFound; i++)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*) GetCapSet(cmCapVideo, i, eRole);
			if(pVideoCap && ( pVideoCap->GetRole() == otherCap->GetRole() ) && ( pVideoCap->GetType() == otherCap->GetType() ) && (pVideoCap->GetCapCode() != eLPRCapCode) )
			{
				newRate=pVideoCap->GetBitRate();
				isFound = true;
			}
			POBJDELETE(pVideoCap);
		}
	}
	else
		PTRACE2INT(eLevelInfoNormal,"No video caps found ", m_numOfVideoCapSets);

	return newRate;

}

///////////////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsCapableOf4CIF() const
{
	return (Get4CifMpi() != -1);
}

///////////////////////////////////////////////////////////////////////////////////
CapEnum CSipCaps::FindAlgAccordingToPayload(cmCapDataType eMediaType, WORD payload, ERoleLabel eRole) const
{
	CapEnum reVal				= eUnknownAlgorithemCapCode;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if (pMediaCapList[i]->sipPayloadType == payload)
			{
				reVal = (CapEnum)pMediaCapList[i]->capTypeCode;
				break;
			}
		}
	}
	return reVal;
}
//////////////////////////////////////////////////////////////////////////////////
APIU16 CSipCaps::FindH264ProfileFromPayload(cmCapDataType eMediaType, WORD payload) const
{
	CapEnum reVal				= eUnknownAlgorithemCapCode;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList);
	APIU16 profile = 0;
	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if (pMediaCapList[i]->sipPayloadType == payload)
			{
				reVal = (CapEnum)pMediaCapList[i]->capTypeCode;
				if(IsH264Video(reVal))
				{
					CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
					if (pH264VideoCap)
					{
					    profile = pH264VideoCap->GetProfile();
					    POBJDELETE(pH264VideoCap);
					    return profile;
					}
					else
					    PTRACE(eLevelInfoNormal,"CSipCaps::FindH264ProfileFromPayload - pVideoCap is NULL");
				}
			}
		}
	}
	return profile;
}

//////////////////////////////////////////////////////////////////////////////////
APIU8 CSipCaps::FindH264PacketizationModeFromPayload(cmCapDataType eMediaType, WORD payload) const
{
	CapEnum reVal				= eUnknownAlgorithemCapCode;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList);
	APIU8 packetizationMode = 0;
	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if (pMediaCapList[i]->sipPayloadType == payload)
			{
				reVal = (CapEnum)pMediaCapList[i]->capTypeCode;
				if(reVal == eH264CapCode)
				{
					CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
					if (pH264VideoCap)
					{
						packetizationMode = pH264VideoCap->GetPacketizationMode();
					    POBJDELETE(pH264VideoCap);
					    return packetizationMode;
					}
					else
					    PTRACE(eLevelInfoNormal,"CSipCaps::FindH264PacketizationModeFromPayload - pVideoCap is NULL");
				}
			}
		}
	}
	return packetizationMode;
}

//////////////////////////////////////////////////////////////////////////////////
APIS32 CSipCaps::FindMaxFsFromPayload(cmCapDataType eMediaType, WORD payload) const
{
	CapEnum reVal				= eUnknownAlgorithemCapCode;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList);
	APIS32 fs = 0;
	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if (pMediaCapList[i]->sipPayloadType == payload)
			{
				reVal = (CapEnum)pMediaCapList[i]->capTypeCode;
				if(reVal == eH264CapCode)
				{
					CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
					if (pH264VideoCap)
					{
						fs = pH264VideoCap->GetFs();
					    POBJDELETE(pH264VideoCap);
					    return fs;
					}
					else
					    TRACEINTO << "pVideoCap is NULL";
				}
			}
		}
	}
	return fs;
}


///////////////////////////////////////////////////////////////////////////////////
APIS8 CSipCaps::Get4CifMpi() const
{
	// since 4cif is not being declared by local caps, 4cif (for local cap) is being set in seperate data member (m_h263_4CifMpi) on createure.
	// if 4cif data member is set with non -1 value, caps are not being checked (if they have 4cif ability).
	// if that data member is initilize with -1, there is still a posibility that these caps have 4cif ability (for instance, remote EP).
	// Therefore, caps are being checked as well.

	if(m_h263_4CifMpi != -1) return m_h263_4CifMpi;

	CH263VideoCap* pH263VideoCap = NULL;
	APIS8 bRes = -1;

	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

	for (int i = 0; i < numOfMediaCapSet && bRes == -1; i++)
	{
		if ((CapEnum)pMediaCapList[i]->capTypeCode == eH263CapCode)
		{
			pH263VideoCap = (CH263VideoCap *)CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode, pMediaCapList[i]->dataCap);
			// if role is peopel and 4Cif supported by this cap
			if (pH263VideoCap && (pH263VideoCap->GetRole() == kRolePeople) )
				bRes = pH263VideoCap->GetFormatMpi(k4Cif);
			POBJDELETE(pH263VideoCap);
		}
	}
	return bRes;
}

////////////////////////////////////////////////////////////////////////////
// video rate is in 100 bits per sec (e.g 128k = 1280)
void CSipCaps::Set4CifMpi(DWORD videoRateIn100bits, eVideoQuality vidQuality)
{
	m_h263_4CifMpi = CH263VideoMode::GetH263Cif4VideoCardMPI(videoRateIn100bits, vidQuality);
}

/////////////////////////////////////////////////////////////////////////////
void CSipCaps::DecideOnConfBitRateForAudioSelection(DWORD& confBitRate)
{
	// This is a temporary matrix - TBD by SRE
	if (confBitRate < rate96K)
		confBitRate = rate64K;
	else if (confBitRate < rate192K)
		confBitRate = rate128K;
	else if (confBitRate < rate256K)
		confBitRate = rate192K;
	else if (confBitRate < rate512K)
		confBitRate = rate384K;
	else if (confBitRate < rate1024K)
		confBitRate = rate512K;
	else
		confBitRate = rate1024K;
}


/////////////////////////////////////////////////////////////////////////
//void CSipCaps::ReduceRedundantAudioCaps()
//{
//	// Siren14 Stereo:
//	if (IsCapSet(eSiren14Stereo_48kCapCode) && (IsCapSet(eSiren14Stereo_96kCapCode) || IsCapSet(eSiren14Stereo_64kCapCode) || IsCapSet(eSiren14Stereo_56kCapCode)))
//		RemoveCapSet(eSiren14Stereo_48kCapCode);
//	if (IsCapSet(eSiren14Stereo_56kCapCode) && (IsCapSet(eSiren14Stereo_96kCapCode) || IsCapSet(eSiren14Stereo_64kCapCode)))
//		RemoveCapSet(eSiren14Stereo_56kCapCode);
//	if (IsCapSet(eSiren14Stereo_64kCapCode) && (IsCapSet(eSiren14Stereo_96kCapCode)))
//		RemoveCapSet(eSiren14Stereo_64kCapCode);
//
//	// Siren 22 Stereo:
//	if (IsCapSet(eSiren22Stereo_64kCapCode) && (IsCapSet(eSiren22Stereo_128kCapCode) || IsCapSet(eSiren22Stereo_96kCapCode)))
//		RemoveCapSet(eSiren22Stereo_64kCapCode);
//	if (IsCapSet(eSiren22Stereo_96kCapCode) && (IsCapSet(eSiren22Stereo_128kCapCode)))
//		RemoveCapSet(eSiren22Stereo_96kCapCode);
//
//	// Siren 22 Mono:
//	if (IsCapSet(eSiren22_32kCapCode) && (IsCapSet(eSiren22_64kCapCode) || IsCapSet(eSiren22_48kCapCode)))
//		RemoveCapSet(eSiren22_32kCapCode);
//	if (IsCapSet(eSiren22_48kCapCode) && (IsCapSet(eSiren22_64kCapCode)))
//		RemoveCapSet(eSiren22_48kCapCode);
//
//	// G719 Mono:
//	if (IsCapSet(eG719_32kCapCode) && (IsCapSet(eG719_64kCapCode) || IsCapSet(eG719_48kCapCode)))
//		RemoveCapSet(eG719_32kCapCode);
//	if (IsCapSet(eG719_48kCapCode) && (IsCapSet(eG719_64kCapCode)))
//		RemoveCapSet(eG719_48kCapCode);
//
//	// G719 Stereo:
//	if (IsCapSet(eG719Stereo_64kCapCode) && (IsCapSet(eG719Stereo_128kCapCode) || IsCapSet(eG719Stereo_96kCapCode)))
//		RemoveCapSet(eG719Stereo_64kCapCode);
//	if (IsCapSet(eG719Stereo_96kCapCode) && (IsCapSet(eG719Stereo_128kCapCode)))
//		RemoveCapSet(eG719Stereo_96kCapCode);
//}
/////////////////////////////////////////////////////////////////////////
void CSipCaps::RemoveAudioCapsAccordingToList()
{
	int i = 0;
	while (g_SipDialOutRemovedAudioCodecs[i] != eUnknownAlgorithemCapCode)
	{
		RemoveCapSet(g_SipDialOutRemovedAudioCodecs[i]);
		i++;
	}
}
/////////////////////////////////////////////////////////////////////////
void CSipCaps::RemoveCapsForTIPCall()
{
	int i = 0;
	while (g_TipRemoveCodecs[i] != eUnknownAlgorithemCapCode)
	{
		RemoveCapSet(g_TipRemoveCodecs[i]);
		i++;
	}
	//RemoveCapSet(eH261CapCode);
	//RemoveCapSet(eH263CapCode);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////BRIDGE-11708
void CSipCaps::AddSdesCapsByMki(cmCapDataType eMediaType, ERoleLabel eRole, int Sh1Length, EEncryptionKeyToUse mkiEncryptionKeyToUse, BOOL bUseNonMkiOrderFirst, int& nTagNum)
{
	TRACEINTO << "MediaType: " << eMediaType << " eRole: " << eRole << ", iSh1Length " << (int)Sh1Length << " mkiEncryptionKeyToUse " << (int)mkiEncryptionKeyToUse << " bUseNonMkiOrderFirst " << (int)bUseNonMkiOrderFirst;

	//ONE crypto line
	if(mkiEncryptionKeyToUse != eUseBothEncryptionKeys)
	{
		BOOL bDoNotUseMki = (mkiEncryptionKeyToUse == eUseNonMkiKeyOnly);
		TRACEINTO<< " nTagNum: " << (int)nTagNum << " use mki: "<<(int)!bDoNotUseMki<<" Sh1Length: "<<(int)Sh1Length;
		AddSingleSdesCap(eMediaType, bDoNotUseMki, TranslateShaLength((eTypeOfSha1Length)Sh1Length), nTagNum /*2*/,eRole);
		nTagNum++;
	}
	//TWO crypto line
	else
	{
		TRACEINTO<< " nTagNum: " << nTagNum <<" use mki: "<<(int)!bUseNonMkiOrderFirst<<" Sh1Length: "<<(int)Sh1Length;
		AddSingleSdesCap(eMediaType, bUseNonMkiOrderFirst,  TranslateShaLength((eTypeOfSha1Length)Sh1Length),nTagNum,eRole);
		nTagNum++;
		TRACEINTO<< " nTagNum: " << nTagNum <<" use mki: "<<(int)bUseNonMkiOrderFirst<<" Sh1Length: "<<(int)Sh1Length;
		AddSingleSdesCap(eMediaType, !bUseNonMkiOrderFirst,  TranslateShaLength((eTypeOfSha1Length)Sh1Length),nTagNum,eRole);
		nTagNum++;
	}

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////BRIDGE-11708
sdesCryptoSuiteEnum CSipCaps::TranslateShaLength(eTypeOfSha1Length Sh1Length)
{
	switch(Sh1Length)
	{
		case eSha1_length_80:
			return eAes_Cm_128_Hmac_Sha1_80;
		case eSha1_length_32:
			return eAes_Cm_128_Hmac_Sha1_32;
		default:
			return eUnknownSuiteParam;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////BRIDGE-11708
void CSipCaps::AddSdesCaps(cmCapDataType eMediaType, ERoleLabel eRole)
{
	TRACEINTO << "MediaType: " << eMediaType << " eRole: " << eRole;

	int Sh1Length    = GetSdesCapEnumFromSystemFlag();
	int nTagNum 	 = 1;

	switch(Sh1Length)
	{
		case eSha1_length_80:
		case eSha1_length_32:
		{
			AddSdesCapsByMki(eMediaType, eRole, Sh1Length, (EEncryptionKeyToUse)m_encryptionKeyToUse, m_bUseNonMkiOrderFirst, nTagNum);
			break;
		}
		case eSha1_length_80_32:
		{
			AddSdesCapsByMki(eMediaType, eRole, eSha1_length_80, (EEncryptionKeyToUse)m_encryptionKeyToUse, m_bUseNonMkiOrderFirst, nTagNum);
			AddSdesCapsByMki(eMediaType, eRole, eSha1_length_32, (EEncryptionKeyToUse)m_encryptionKeyToUse, m_bUseNonMkiOrderFirst, nTagNum);
			break;
		}
		default:
		{
			TRACEINTO << "MediaType: " << eMediaType << " eRole: " << eRole << ", incorrect eTypeOfSha1Length";
			break;
		}
	}
}

/*//BRIDGE-11708
void CSipCaps::AddSdesCaps(cmCapDataType eMediaType, ERoleLabel eRole)
{

	int Sh1Length = GetSdesCapEnumFromSystemFlag();
	TRACEINTO << "MediaType: " << eMediaType << " eRole: " << eRole << ", Sh1Length: " << Sh1Length;
	TRACEINTO<<"lyncCryptoLinesDebugBridge8497: m_encryptionKeyToUse: "<<(int)m_encryptionKeyToUse<<" Sh1Length:"<<(int)Sh1Length;
	switch(Sh1Length)
	{
	case eSha1_length_80:
		TRACEINTO << "MediaType: " << eMediaType << " eRole: " << eRole << ", iSh1Length eSha1_length_80";
		
		if(m_encryptionKeyToUse==eUseMkiKeyOnly || m_encryptionKeyToUse==eUseBothEncryptionKeys)
		{
			TRACEINTO<<"lyncCryptoLinesDebugBridge8497: m_encryptionKeyToUse: "<<(int)m_encryptionKeyToUse<<" Sh1Length:"<<(int)Sh1Length;
			AddSingleSdesCap(eMediaType, FALSE, eAes_Cm_128_Hmac_Sha1_80,1 *2*,eRole); // bridge 7868 - order of keys is important for lync
		}
		if(m_encryptionKeyToUse==eUseNonMkiKeyOnly || m_encryptionKeyToUse==eUseBothEncryptionKeys)
		{
			AddSingleSdesCap(eMediaType, TRUE,  eAes_Cm_128_Hmac_Sha1_80,2,eRole);
		}
		break;
	case eSha1_length_32:
		TRACEINTO << "MediaType: " << eMediaType << " eRole: " << eRole<< ", iSh1Length eSha1_length_32";
		if(m_encryptionKeyToUse==eUseMkiKeyOnly || m_encryptionKeyToUse==eUseBothEncryptionKeys)
		{
			AddSingleSdesCap(eMediaType, FALSE, eAes_Cm_128_Hmac_Sha1_32, 1,eRole);
		}
		if(m_encryptionKeyToUse==eUseNonMkiKeyOnly || m_encryptionKeyToUse==eUseBothEncryptionKeys)
		{
			AddSingleSdesCap(eMediaType, TRUE,  eAes_Cm_128_Hmac_Sha1_32, 2,eRole);
		}
		break;
	case eSha1_length_80_32:
		TRACEINTO << "MediaType: " << eMediaType << " eRole: " << eRole<< ", iSh1Length eSha1_length_80_32";
		if(m_encryptionKeyToUse==eUseMkiKeyOnly || m_encryptionKeyToUse==eUseBothEncryptionKeys)
		{
			AddSingleSdesCap(eMediaType, FALSE, eAes_Cm_128_Hmac_Sha1_32, 1,eRole);
			AddSingleSdesCap(eMediaType, FALSE, eAes_Cm_128_Hmac_Sha1_80, 2,eRole);
		}
		if(m_encryptionKeyToUse==eUseNonMkiKeyOnly || m_encryptionKeyToUse==eUseBothEncryptionKeys)
		{
			AddSingleSdesCap(eMediaType, TRUE,  eAes_Cm_128_Hmac_Sha1_32, 3,eRole);
			AddSingleSdesCap(eMediaType, TRUE,  eAes_Cm_128_Hmac_Sha1_80, 4,eRole);
		}
		break;
	default:
		TRACEINTO << "MediaType: " << eMediaType << " eRole: " << eRole << ", incorrect eTypeOfSha1Length";
		break;
	}
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*int CSipCaps::GetSdesCapEnumFromSystemFlag()
{

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	string ShaKeyType;
	sysConfig->GetDataByKey(CFG_KEY_SRTP_SRTCP_HMAC_SHA_LENGTH, ShaKeyType);

	int Sh1Length;
	CStringsMaps::GetValue(TRANSFER_RATE_ENUM, Sh1Length, (char*)ShaKeyType.c_str());
	return Sh1Length;
	if (ShaKeyType == "80")
	{
		Sh1Length = eSha1_length_80;
	}
	else if (ShaKeyType == "32")
	{
		Sh1Length = eSha1_length_32;
	}
	else if (ShaKeyType == "80_32")
	{
		Sh1Length = eSha1_length_80_32;
	}
	return Sh1Length;
}*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetTipDefaultsForEncrypt(BOOL& bIsNeedToSendMKI, BOOL& bLifeTimeInUse , BOOL& bFecKeyInUse)
{
	bIsNeedToSendMKI = FALSE;
	bLifeTimeInUse = FALSE;
	bFecKeyInUse = FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddSingleSdesCap(cmCapDataType eMediaType, BOOL bForceNoMki, APIU16 cryptoSuit, APIU32 sdesTag ,ERoleLabel eRole)
{
	BOOL bLifeTimeInUse   = TRUE;
	BOOL bIsNeedToSendMKI = TRUE;
	BOOL bFecKeyInUse	  = FALSE;
	//CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SEND_SRTP_MKI, bIsNeedToSendMKI);

	TRACEINTO << "MediaType: " << eMediaType << ", bForceNoMki: " << (bForceNoMki? "yes" : "no");
	if(bForceNoMki)
		SetTipDefaultsForEncrypt(bIsNeedToSendMKI,bLifeTimeInUse ,bFecKeyInUse);

	CSdesCap* pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,NULL);

	if(pSdesCap) {
		EResult eResOfSet	= kSuccess;

		eResOfSet &= pSdesCap->SetStruct(eMediaType, cmCapReceiveAndTransmit, eRole);
		pSdesCap->SetSdesTag(sdesTag);
		pSdesCap->SetSdesCryptoSuite(cryptoSuit);
		pSdesCap->SetNumOfKeysParam(1);
		pSdesCap->SetIsSdesUnencryptedSrtp(FALSE);
		pSdesCap->SetIsSdesUnencryptedSrtcp(FALSE);
		pSdesCap->SetIsSdesUnauthenticatedSrtp(FALSE);
		pSdesCap->SetIsSdesKdrInUse(FALSE);
		pSdesCap->SetSdesKdr(0);
		pSdesCap->SetIsSdesWshInUse(FALSE);
		pSdesCap->SetSdesWsh(0);
		pSdesCap->SetIsSdesFecOrderInUse(FALSE);
		pSdesCap->SetSdesFecOrder(eSrtpUnknownFec);
		pSdesCap->SetIsSdesFecKeyInUse(bFecKeyInUse);
		pSdesCap->SetSdesKeyMethod(0, eSdesInlineKeyMethod);
		pSdesCap->SetSdesBase64KeySalt(0, "ffffffffffffffffffffffffffffffffffffffff");
		pSdesCap->SetIsSdesLifeTimeInUse(0, bLifeTimeInUse);
		pSdesCap->SetSdesLifeTime(0, DEFAULT_LIFE_TIME);
		if(bIsNeedToSendMKI)
		{
			pSdesCap->SetIsSdesMkiInUse(0, TRUE);
			pSdesCap->SetSdesMkiValue(0, 1);
			pSdesCap->SetIsSdesMkiValueLenInUse(0, TRUE);
			pSdesCap->SetSdesMkiValueLen(0,DEFAULT_MKI_VAL_LEN);
		}
		else
		{
			pSdesCap->SetIsSdesMkiInUse(0, FALSE);
			pSdesCap->SetSdesMkiValue(0, 0);
			pSdesCap->SetIsSdesMkiValueLenInUse(0, FALSE);
			pSdesCap->SetSdesMkiValueLen(0,0);
		}

		if(eResOfSet) {
			capBuffer* pCapBuffer = pSdesCap->GetAsCapBuffer();
			if (pCapBuffer)
			{
				AddSdesCapSet(eMediaType,pCapBuffer,eRole);
			}
			else
			{
				TRACEINTO << "MediaType: " << eMediaType << ", Create cap buffer has failed";
			}
			PDELETEA(pCapBuffer);
		} else {
			TRACEINTO << "MediaType: " << eMediaType << ", Set struct has failed";
		}
		pSdesCap->FreeStruct();

	}

	POBJDELETE(pSdesCap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddSingleDtlsCap(cmCapDataType eMediaType, BOOL bForceNoMki, ERoleLabel eRole)
{
	BOOL bLifeTimeInUse   = TRUE;
	BOOL bIsNeedToSendMKI = TRUE;
	BOOL bFecKeyInUse	  = FALSE;
	//CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SEND_SRTP_MKI, bIsNeedToSendMKI);

	TRACEINTO << "MediaType: " << eMediaType << ", bForceNoMki: " << (bForceNoMki ? "yes" : "no");
	if(bForceNoMki)
		SetTipDefaultsForEncrypt(bIsNeedToSendMKI,bLifeTimeInUse ,bFecKeyInUse);

	CDtlsCap* pDtlsCap = (CDtlsCap*)CBaseCap::AllocNewCap((CapEnum)eDtlsCapCode,NULL);

	if(pDtlsCap) {
		EResult eResOfSet	= kSuccess;

		eResOfSet &= pDtlsCap->SetStruct(eMediaType, cmCapReceiveAndTransmit, eRole);
		pDtlsCap->SetSdesTag(1);
		pDtlsCap->SetSdesCryptoSuite(eAes_Cm_128_Hmac_Sha1_80);
		pDtlsCap->SetNumOfKeysParam(1);
		pDtlsCap->SetIsSdesUnencryptedSrtp(FALSE);
		pDtlsCap->SetIsSdesUnencryptedSrtcp(FALSE);
		pDtlsCap->SetIsSdesUnauthenticatedSrtp(FALSE);
		pDtlsCap->SetIsSdesKdrInUse(FALSE);
		pDtlsCap->SetSdesKdr(0);
		pDtlsCap->SetIsSdesWshInUse(FALSE);
		pDtlsCap->SetSdesWsh(0);
		pDtlsCap->SetIsSdesFecOrderInUse(FALSE);
		pDtlsCap->SetSdesFecOrder(eSrtpUnknownFec);
		pDtlsCap->SetIsSdesFecKeyInUse(bFecKeyInUse);
		pDtlsCap->SetSdesKeyMethod(0, eSdesInlineKeyMethod);
		pDtlsCap->SetSdesBase64KeySalt(0, "ffffffffffffffffffffffffffffffffffffffff");
		pDtlsCap->SetIsSdesLifeTimeInUse(0, bLifeTimeInUse);
		pDtlsCap->SetSdesLifeTime(0, DEFAULT_LIFE_TIME);
		if(bIsNeedToSendMKI)
		{
			pDtlsCap->SetIsSdesMkiInUse(0, TRUE);
			pDtlsCap->SetSdesMkiValue(0, 1);
			pDtlsCap->SetIsSdesMkiValueLenInUse(0, TRUE);
			pDtlsCap->SetSdesMkiValueLen(0,DEFAULT_MKI_VAL_LEN);
		}
		else
		{
			pDtlsCap->SetIsSdesMkiInUse(0, FALSE);
			pDtlsCap->SetSdesMkiValue(0, 0);
			pDtlsCap->SetIsSdesMkiValueLenInUse(0, FALSE);
			pDtlsCap->SetSdesMkiValueLen(0,0);
		}

		if(eResOfSet) {
			capBuffer* pCapBuffer = pDtlsCap->GetAsCapBuffer();
			if (pCapBuffer)
			{
				AddDtlsCapSet(eMediaType,pCapBuffer,eRole);
			}
			else
			{
				TRACEINTO << "MediaType: " << eMediaType << ", Create cap buffer has failed";
			}
			PDELETEA(pCapBuffer);
		} else {
			TRACEINTO << "MediaType: " << eMediaType << ", Set struct has failed";
		}
		pDtlsCap->FreeStruct();

	}

	POBJDELETE(pDtlsCap);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::UpdateSdesTag(cmCapDataType eMediaType,APIU32 tag, ERoleLabel eRole)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetSdesMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eSdesCapCode)
			{
				CSdesCap* pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[i]->dataCap);
				if (pSdesCap)
				{
				    pSdesCap->SetSdesTag(tag);
				    POBJDELETE(pSdesCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::UpdateSdesTag - pSdesCap is NULL");
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::UpdateSdesMasterSaltBase64Key(cmCapDataType eMediaType, char* key, BOOL bDisableDefaults, ERoleLabel eRole)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetSdesMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eSdesCapCode)
			{
				CSdesCap* pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[i]->dataCap);
				if (pSdesCap)
				{
				    pSdesCap->SetSdesBase64KeySalt(0,key);
				    if (bDisableDefaults) {
				    	pSdesCap->SetIsSdesLifeTimeInUse(0, FALSE);
				    	pSdesCap->SetIsSdesMkiInUse(0, FALSE);
				    	pSdesCap->SetIsSdesMkiValueLenInUse(0, FALSE);
				    }
				    POBJDELETE(pSdesCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::UpdateSdesMasterSaltBase64Key - pSdesCap is NULL");
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::GetSdesMasterSaltBase64Key(cmCapDataType eMediaType, char *pKey, int len, ERoleLabel eRole)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetSdesMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eSdesCapCode)
			{
				CSdesCap* pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[i]->dataCap);
				if (pSdesCap)
				{
					 char *pSrcKey = pSdesCap->GetSdesBase64KeySalt(0);
					 if ( NULL != pSrcKey )
					 {
					    strncpy(pKey, pSrcKey, len -1);
					 }
					 //to please klocwork
		        	POBJDELETE(pSdesCap);
		        	break;
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::GetSdesMasterSaltBase64Key - pSdesCap is NULL");
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSipCaps::GetSdesIsMkiInUseByTag(int tag, cmCapDataType eMediaType , ERoleLabel eRole)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	//BOOL bIsNeedToSendMKI = TRUE;
	//CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SEND_SRTP_MKI, bIsNeedToSendMKI);

	GetSdesMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
	BOOL bIsMkiInUse = YES;

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eSdesCapCode)
			{
				CSdesCap* pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[i]->dataCap);
				if (pSdesCap)
				{
			    	bIsMkiInUse = pSdesCap->GetIsSdesMkiInUse(0);

					if(tag == (int)(pSdesCap->GetSdesTag()))
					{
						POBJDELETE(pSdesCap);
						break;
					}
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::GetSdesIsMkiInUseByTag - pSdesCap is NULL");
				POBJDELETE(pSdesCap);
			}
		}
	}

	TRACEINTO << " bIsMkiInUse " <<(int)bIsMkiInUse;
	return bIsMkiInUse;
}

//////////////////////////////////////////////////////////////////////////////////////////////
int CSipCaps::GetSdesTag(cmCapDataType eMediaType, BOOL bIsTipMode, ERoleLabel eRole, APIU16 pcryptoSuite)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	//BOOL bIsNeedToSendMKI = TRUE;
	//CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SEND_SRTP_MKI, bIsNeedToSendMKI);

	GetSdesMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);
	int tag = -1;
	APIU16 cryptoSuite;

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eSdesCapCode)
			{
				CSdesCap* pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[i]->dataCap);
				if (pSdesCap)
				{
					cryptoSuite = pSdesCap->GetSdesCryptoSuite();
				    if(cryptoSuite == pcryptoSuite)
				    {
				    	tag = pSdesCap->GetSdesTag();

				    	//if( bIsTipMode || (bIsNeedToSendMKI == true && pSdesCap->GetIsSdesMkiInUse(0) == true) )
						if(!(pSdesCap->GetIsSdesUnencryptedSrtcp()))
				    	{
							POBJDELETE(pSdesCap);
							break;
				        }
				    }
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::GetSdesTag - pSdesCap is NULL");
				POBJDELETE(pSdesCap);
			}
		}
	}

	TRACEINTO << " tag " <<(int)tag;
	return tag;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::UpdateSdesMkiInUseRx(CSipComMode* pBestMode ,APIU32 audioTag , APIU32 videoTag, APIU32 dataTag , APIU32 contentTag)
{
	CSipCaps*	pCaps = const_cast<CSipCaps*>(this);

	PASSERTMSG_AND_RETURN(!pCaps  || !pBestMode , "!pCaps  || !pBestMode");

	BOOL bAudioMkiInUse   = TRUE;
	BOOL bVideoMkiInUse   = TRUE;
	BOOL bDataMkiInUse    = TRUE;
	BOOL bContentMkiInUse = TRUE;

	bAudioMkiInUse = pCaps ->GetSdesIsMkiInUseByTag((int)audioTag , cmCapAudio, kRolePeople);
	pBestMode->UpdateRxSdesAudioMkiInUse(bAudioMkiInUse);

	bVideoMkiInUse = pCaps ->GetSdesIsMkiInUseByTag((int) videoTag , cmCapVideo, kRolePeople);
	pBestMode->UpdateRxSdesVideoMkiInUse(bVideoMkiInUse);

	bDataMkiInUse = pCaps ->GetSdesIsMkiInUseByTag((int)dataTag , cmCapData, kRolePeople);
	pBestMode->UpdateRxSdesDataMkiInUse(bDataMkiInUse);

	bContentMkiInUse = pCaps ->GetSdesIsMkiInUseByTag((int) contentTag , cmCapVideo, kRolePresentation);
	pBestMode->UpdateRxSdesContentMkiInUse(bContentMkiInUse);

	TRACEINTO << " audioTag "   << audioTag   << " bAudioMkiInUse "  << (int)bAudioMkiInUse
			  << " videoTag "   << videoTag   << " bVideoMkiInUse "  << (int)bVideoMkiInUse
			  << " dataTag "    << dataTag    << " bDataMkiInUse "   << (int)bDataMkiInUse
			  << " contentTag " << contentTag << " bContentMkiInUse "<< (int)bContentMkiInUse;
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::UpdateSdesMkiInUseTx(CSipComMode* pBestMode ,APIU32 audioTag , APIU32 videoTag, APIU32 dataTag , APIU32 contentTag)
{
	CSipCaps*	pCaps  = const_cast<CSipCaps*>(this);

	PASSERTMSG_AND_RETURN(!pCaps  || !pBestMode , "!pCurRemoteCaps || !pBestMode");

	BOOL bAudioMkiInUse   = TRUE;
	BOOL bVideoMkiInUse   = TRUE;
	BOOL bDataMkiInUse    = TRUE;
	BOOL bContentMkiInUse = TRUE;

	bAudioMkiInUse = pCaps->GetSdesIsMkiInUseByTag((int)audioTag , cmCapAudio, kRolePeople);
	pBestMode->UpdateTxSdesAudioMkiInUse(bAudioMkiInUse);

	bVideoMkiInUse = pCaps->GetSdesIsMkiInUseByTag((int) videoTag , cmCapVideo, kRolePeople);
	pBestMode->UpdateTxSdesVideoMkiInUse(bVideoMkiInUse);

	bDataMkiInUse = pCaps->GetSdesIsMkiInUseByTag((int)dataTag , cmCapData, kRolePeople);
	pBestMode->UpdateTxSdesDataMkiInUse(bDataMkiInUse);

	bContentMkiInUse = pCaps->GetSdesIsMkiInUseByTag((int) contentTag , cmCapVideo, kRolePresentation);
	pBestMode->UpdateTxSdesContentMkiInUse(bContentMkiInUse);

	TRACEINTO << " audioTag "   << audioTag   << " bAudioMkiInUse "  << (int)bAudioMkiInUse
			  << " videoTag "   << videoTag   << " bVideoMkiInUse "  << (int)bVideoMkiInUse
			  << " dataTag "    << dataTag    << " bDataMkiInUse "   << (int)bDataMkiInUse
			  << " contentTag " << contentTag << " bContentMkiInUse "<< (int)bContentMkiInUse;
}

//////////////////////////////////////////////////////////////////////////////////////////////
CSipComMode* CSipCaps::FindSdesBestMode(const CSipComMode& pTmpPrefferedMode, const CSipCaps& pAlternativeCaps, const CSipComMode& pTempBestMode, BYTE bIsOffere) const
{
	PTRACE(eLevelError,"CSipCaps::FindSdesBestMode:  ");
	CSipComMode* pBestMode = new CSipComMode;
	CSipComMode* pPrefferedMode= new CSipComMode;
	int startPoint = 1;
	int endPoint   = 2;
	DWORD isEncrypted;
	APIU16 localCryptoSuite = eUnknownSuiteParam;
	APIU32 localTag = 0;
	APIU16 remoteCryptoSuite = eUnknownSuiteParam;
	APIU32 remoteTag = 0;
	int findedAudioSdesLocation = -1;
	int findedVideoSdesLocation = -1;
	int findedDataSdesLocation = -1;
	int findedContentSdesLocation = -1;
	int numOfRemoteSdesCapSets = 0;
	int length = 0;
	CSdesCap *pPrefferedSdesCap = NULL;
	capBuffer** pMediaCapList = NULL;
	capBuffer** pMediaCapListAlternate = NULL;
	int numOfSdesCapSetsAlternate = 0;
	*pPrefferedMode = pTmpPrefferedMode;
	*pBestMode = pTempBestMode;
	isEncrypted = pPrefferedMode->GetIsEncrypted();

	CSdesCap *pOldAudioSdesCap = pPrefferedMode->GetSipSdes(cmCapAudio,cmCapReceive,kRolePeople);
	CSipCaps*	pCurRemoteCaps = const_cast<CSipCaps*>(this);
	CSipCaps*	pAltLocalCaps = const_cast<CSipCaps*>(&pAlternativeCaps);
	APIU32 audioTag = 0;
	APIU32 videoTag = 0;
	APIU32 dataTag = 0;
	APIU32 contentTag = 0;
	BYTE bFound = NO;;
	cmCapDataType AlternativeMediaType;//local cap

	if(pOldAudioSdesCap && pCurRemoteCaps)
	{
		audioTag = pCurRemoteCaps->GetSdesTag(cmCapAudio, pPrefferedMode->GetIsTipMode(), kRolePeople, pOldAudioSdesCap->GetSdesCryptoSuite());
		if((int)audioTag != (int)pOldAudioSdesCap->GetSdesTag())
		{
			pBestMode->UpdateRxSdesAudioTag(audioTag);
		}
	}
	else if (pCurRemoteCaps && pOldAudioSdesCap == NULL)
	{
		audioTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapAudio, pPrefferedMode->GetIsTipMode());
		pBestMode->UpdateRxSdesAudioTag(audioTag);
	}

	CSdesCap *pOldVideoSdesCap = pPrefferedMode->GetSipSdes(cmCapVideo,cmCapReceive,kRolePeople);
	if(pOldVideoSdesCap && pCurRemoteCaps)
	{
		videoTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapVideo, pPrefferedMode->GetIsTipMode(), kRolePeople, pOldVideoSdesCap->GetSdesCryptoSuite());
		if((int)videoTag != (int)pOldVideoSdesCap->GetSdesTag())
		{
			pBestMode->UpdateRxSdesVideoTag(videoTag);
		}
	}
	else if (pCurRemoteCaps && pOldVideoSdesCap == NULL)
	{
		videoTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapVideo, pPrefferedMode->GetIsTipMode());
		pBestMode->UpdateRxSdesVideoTag(videoTag);
	}

	CSdesCap *pOldDataSdesCap = pPrefferedMode->GetSipSdes(cmCapData,cmCapReceive,kRolePeople);
	if(pOldDataSdesCap && pCurRemoteCaps)
	{
		dataTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapData, pPrefferedMode->GetIsTipMode(),kRolePeople, pOldDataSdesCap->GetSdesCryptoSuite());
		if((int)dataTag != (int)pOldDataSdesCap->GetSdesTag())
		{
			pBestMode->UpdateRxSdesDataTag(dataTag);
		}
	}
	else if (pCurRemoteCaps && pOldDataSdesCap == NULL)
	{
		dataTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapData, pPrefferedMode->GetIsTipMode());
		pBestMode->UpdateRxSdesDataTag(dataTag);
	}

	CSdesCap *pOldContentSdesCap = pPrefferedMode->GetSipSdes(cmCapVideo,cmCapReceive,kRolePresentation);
	if(pOldContentSdesCap && pCurRemoteCaps)
	{
		contentTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapVideo, pPrefferedMode->GetIsTipMode(),kRolePresentation, pOldContentSdesCap->GetSdesCryptoSuite());
		if((int)contentTag != (int)pOldContentSdesCap->GetSdesTag())
		{
			pBestMode->UpdateRxSdesContentTag(contentTag);
		}
	}
	else if (pCurRemoteCaps && pOldContentSdesCap == NULL)
	{
		contentTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapVideo, pPrefferedMode->GetIsTipMode(), kRolePresentation);
		pBestMode->UpdateRxSdesContentTag(contentTag);
	}

	pCurRemoteCaps->UpdateSdesMkiInUseRx(pBestMode ,audioTag ,videoTag, dataTag, contentTag);
	if(bIsOffere && pAltLocalCaps)
		pAltLocalCaps->UpdateSdesMkiInUseTx(pBestMode ,audioTag ,videoTag, dataTag, contentTag);
	else
		pCurRemoteCaps->UpdateSdesMkiInUseTx(pBestMode ,audioTag ,videoTag, dataTag, contentTag);


	cmCapDataType mediaType;
	ERoleLabel eRole;
	for(int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		PTRACE(eLevelError,"CSipCaps::FindSdesBestMode:  ");
		if (mediaType == cmCapBfcp)
			continue;

		for (int j = startPoint; j < endPoint; j++)
		{
			if ( pBestMode->IsMediaOn(mediaType, globalDirectionArr[j], eRole) )
			{
				cmCapDirection eOppositeDirection = (globalDirectionArr[j] == cmCapTransmit) ? cmCapReceive : cmCapTransmit;
				pPrefferedSdesCap =  pBestMode->GetSipSdes(mediaType,globalDirectionArr[j],eRole);
				//Find if remote caps specific media (audio, video or data) has SDES cap
				// if so : 	1. 	check if equal crypto suite in both caps
				// 		   	2. 	check received tag
				//			3.	if steps 1 and 2 are ok, check validity of received SDES params
				//			4.  if correct params- add it to best mode
				if(pPrefferedSdesCap) {
					pBestMode->SetSipSdes(mediaType,globalDirectionArr[j],eRole,pPrefferedSdesCap);
					pBestMode->RemoveSipSdes(mediaType,eOppositeDirection,eRole);	// remove cap from unneded opposite channel
					localCryptoSuite = pPrefferedSdesCap->GetSdesCryptoSuite();
					localTag = pPrefferedSdesCap->GetSdesTag();

					pMediaCapList	= NULL;
					numOfRemoteSdesCapSets = 0 ;
					GetSdesMediaCaps(mediaType,&numOfRemoteSdesCapSets,&pMediaCapList, eRole);

					for(int k = 0; k < numOfRemoteSdesCapSets; k++)
					{
						CSdesCap* pCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)pMediaCapList[k]->capTypeCode,pMediaCapList[k]->dataCap);
						if (pCap)
						{
							remoteCryptoSuite = pCap->GetSdesCryptoSuite();
							remoteTag = pCap->GetSdesTag();

							if (remoteCryptoSuite == localCryptoSuite && remoteTag == localTag)
							{
								// match found
								switch(mediaType)
								{
									case cmCapAudio:
										findedAudioSdesLocation = k;
										break;
									case cmCapVideo:
										if (eRole == kRolePeople)
											findedVideoSdesLocation = k;
										else
											findedContentSdesLocation = k;
										break;
									case cmCapData:
										findedDataSdesLocation = k;
										break;
									default:
										PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: incorrect cmCapDataType ");
										break;
								}
								POBJDELETE(pCap);
								break;

							}
							else
							{
								PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: remoteCryptoSuite  or tag different from local");
							}
						}
						else
						{
							PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: pCap is NULL ");
						}
						POBJDELETE(pCap);
					}
					/* In case the remote doesn't have SDES caps remove SDES from BestMode */
					//Yossi H - To update..
					/*if (findedAudioSdesLocation == -1)
					{
						pBestMode->RemoveSipSdes(cmCapAudio, cmCapReceiveAndTransmit, kRolePeople);
					}
					if (findedVideoSdesLocation == -1)
					{
						pBestMode->RemoveSipSdes(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
					}
					if (findedContentSdesLocation == -1)
					{
						pBestMode->RemoveSipSdes(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
					}
					if (findedDataSdesLocation == -1)
					{
						pBestMode->RemoveSipSdes(cmCapData, cmCapReceiveAndTransmit, kRolePeople);
					}*/
				}
			}

			localCryptoSuite = eUnknownSuiteParam;
			localTag = 0;
			remoteCryptoSuite = eUnknownSuiteParam;
			remoteTag = 0;
		}
	}

	// Check validity and insert if no problem found caps to best mode
	if(pBestMode->IsMediaOn(cmCapAudio) && (findedAudioSdesLocation != -1))
	{
		GetSdesMediaCaps(cmCapAudio,&numOfRemoteSdesCapSets,&pMediaCapList);
		CSdesCap* pCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)pMediaCapList[findedAudioSdesLocation]->capTypeCode,pMediaCapList[findedAudioSdesLocation]->dataCap);

		// Check validity
		if(pCap && IsSdesCapValid(pCap) ) {
			pBestMode->SetSipSdes(cmCapAudio,cmCapReceive,kRolePeople,pCap);
			UpdateSdesBestModeTransmitUnencryptedSrtcp(pBestMode, pCap, cmCapAudio, kRolePeople);
		}

		POBJDELETE(pCap);
	}
	else if(pBestMode->IsMediaOn(cmCapAudio))
	{
		pBestMode->RemoveSipSdes(cmCapAudio,cmCapReceive,kRolePeople);
		pBestMode->RemoveSipSdes(cmCapAudio,cmCapTransmit,kRolePeople);
	}

	if(pBestMode->IsMediaOn(cmCapVideo) && (findedVideoSdesLocation != -1))
	{
		GetSdesMediaCaps(cmCapVideo,&numOfRemoteSdesCapSets,&pMediaCapList);
		CSdesCap* pCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)pMediaCapList[findedVideoSdesLocation]->capTypeCode,pMediaCapList[findedVideoSdesLocation]->dataCap);

		if (!pCap)
		{
		    PTRACE(eLevelError,"CSipCaps::FindSdesBestMode: error AllocNewCap returned NULL");
		    DBGPASSERT(1201);
		}
		else
		{
			if(IsSdesCapValid(pCap)) {
		        pBestMode->SetSipSdes(cmCapVideo,cmCapReceive,kRolePeople,pCap);
		        UpdateSdesBestModeTransmitUnencryptedSrtcp(pBestMode, pCap, cmCapVideo, kRolePeople);
			}

		    POBJDELETE(pCap);
		}
	}
	else if(pBestMode->IsMediaOn(cmCapVideo))
	{
			pBestMode->RemoveSipSdes(cmCapVideo,cmCapReceive,kRolePeople);
			pBestMode->RemoveSipSdes(cmCapVideo,cmCapTransmit,kRolePeople);
	}
	if(pBestMode->IsMediaOn(cmCapData) && (findedDataSdesLocation != -1))
	{
		GetSdesMediaCaps(cmCapData,&numOfRemoteSdesCapSets,&pMediaCapList);
		CSdesCap* pCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)pMediaCapList[findedDataSdesLocation]->capTypeCode,pMediaCapList[findedDataSdesLocation]->dataCap);

		if (!pCap)
		{
			PTRACE(eLevelError,"CSipCaps::FindSdesBestMode: error AllocNewCap returned NULL");
			DBGPASSERT(1201);
		}
		else
		{
			if(IsSdesCapValid(pCap)) {
		        pBestMode->SetSipSdes(cmCapData,cmCapReceive,kRolePeople,pCap);
		        UpdateSdesBestModeTransmitUnencryptedSrtcp(pBestMode, pCap, cmCapData, kRolePeople);
			}

		    POBJDELETE(pCap);
		}
	}
	else if(pBestMode->IsMediaOn(cmCapData))
	{
		pBestMode->RemoveSipSdes(cmCapData,cmCapReceive,kRolePeople);
		pBestMode->RemoveSipSdes(cmCapData,cmCapTransmit,kRolePeople);
	}
	if(pBestMode->IsMediaOn(cmCapVideo,cmCapReceive, kRolePresentation) && (findedContentSdesLocation != -1))
	{
		GetSdesMediaCaps(cmCapVideo,&numOfRemoteSdesCapSets,&pMediaCapList,kRolePresentation);
		CSdesCap* pCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)pMediaCapList[findedContentSdesLocation]->capTypeCode,pMediaCapList[findedContentSdesLocation]->dataCap);

		if (!pCap)
		{
			PTRACE(eLevelError,"CSipCaps::FindSdesBestMode: error AllocNewCap returned NULL");
			DBGPASSERT(1201);
		}
		else
		{
			if(IsSdesCapValid(pCap)) {
		        pBestMode->SetSipSdes(cmCapVideo,cmCapReceive,kRolePresentation,pCap);
		        UpdateSdesBestModeTransmitUnencryptedSrtcp(pBestMode, pCap, cmCapVideo, kRolePresentation);
			}

		    POBJDELETE(pCap);
		}
	}
	else if(pBestMode->IsMediaOn(cmCapVideo,cmCapReceive, kRolePresentation))
	{
		PTRACE(eLevelError,"CSipCaps::FindSdesBestMode: no SDES remote caps remove SDES for content for best mode");
		pBestMode->RemoveSipSdes(cmCapVideo,cmCapReceive,kRolePresentation);

	    if( pBestMode->IsTipNegotiated() == TRUE &&  pBestMode->IsMediaOn(cmCapVideo,cmCapReceive) && pBestMode->GetSipSdes(cmCapVideo,cmCapReceive) != NULL)
	    {
	    	PTRACE(eLevelError,"CSipCaps::FindSdesBestMode: tip take sdes to contetn from people");
	    	CSdesCap *pSdesCap = pBestMode->GetSipSdes(cmCapVideo,cmCapReceive, kRolePeople);
	    	pBestMode->SetSipSdes(cmCapVideo,cmCapReceive,kRoleContentOrPresentation,pSdesCap);
	    }
	    else
	    {
	    	pBestMode->RemoveSipSdes(cmCapVideo,cmCapTransmit,kRolePresentation);
	    }
	}

	POBJDELETE(pPrefferedMode);

	//if 80 or 32 - finish here and return the best mode, else - check case 80_32
	int Sh1Length = GetSdesCapEnumFromSystemFlag();
	PTRACE2INT(eLevelInfoNormal,"FindSdesBestMode Sh1Length: ",Sh1Length);
	if(Sh1Length!= eSha1_length_80_32)
	{
		return pBestMode;
	}
	//eSha1_length_80_32 and ep is 80
	if(pBestMode->IsMediaOn(cmCapAudio) && (findedAudioSdesLocation == -1))
	{
		CSdesCap* alternativePcap = SetBestModeFromSdesCaplternate(pAlternativeCaps,eAes_Cm_128_Hmac_Sha1_80, cmCapAudio, (CSipComMode&)*pBestMode, kRolePeople);

		if(alternativePcap && pCurRemoteCaps)
		{
			audioTag = pCurRemoteCaps->GetSdesTag(cmCapAudio, pBestMode->GetIsTipMode(),kRolePeople, alternativePcap->GetSdesCryptoSuite());
			if((int)audioTag != (int)alternativePcap->GetSdesTag())
			{
				pBestMode->UpdateRxSdesAudioTag(audioTag);

			}
		}
		else if (pCurRemoteCaps && alternativePcap == NULL)
		{
			audioTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapAudio, pBestMode->GetIsTipMode());
			pBestMode->UpdateRxSdesAudioTag(audioTag);
		}
		POBJDELETE(alternativePcap);
	}
	if(pBestMode->IsMediaOn(cmCapVideo) && (findedVideoSdesLocation == -1))
	{
		CSdesCap* alternativePcap = SetBestModeFromSdesCaplternate(pAlternativeCaps,eAes_Cm_128_Hmac_Sha1_80, cmCapVideo, (CSipComMode&)*pBestMode, kRolePeople);

		if(alternativePcap && pCurRemoteCaps)
		{
			videoTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapVideo, pBestMode->GetIsTipMode(), kRolePeople, alternativePcap->GetSdesCryptoSuite());
			if((int)videoTag != (int)alternativePcap->GetSdesTag())
			{
				pBestMode->UpdateRxSdesVideoTag(videoTag);
			}
		}
		else if (pCurRemoteCaps && alternativePcap == NULL)
		{
			videoTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapVideo, pBestMode->GetIsTipMode());
			pBestMode->UpdateRxSdesVideoTag(videoTag);
		}
		POBJDELETE(alternativePcap);
	}
	if(pBestMode->IsMediaOn(cmCapData) && (findedDataSdesLocation == -1))
	{
		CSdesCap* alternativePcap = SetBestModeFromSdesCaplternate(pAlternativeCaps,eAes_Cm_128_Hmac_Sha1_80, cmCapData, (CSipComMode&)*pBestMode, kRolePeople);
		if(alternativePcap && pCurRemoteCaps)
		{
			dataTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapData, pBestMode->GetIsTipMode(),kRolePeople, alternativePcap->GetSdesCryptoSuite());
			if((int)dataTag != (int)alternativePcap->GetSdesTag())
			{
				pBestMode->UpdateRxSdesDataTag(dataTag);
			}
		}
		else if (pCurRemoteCaps && alternativePcap == NULL)
		{
			dataTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapData, pBestMode->GetIsTipMode());
			pBestMode->UpdateRxSdesDataTag(dataTag);
		}
		POBJDELETE(alternativePcap);
	}
	if(pBestMode->IsMediaOn(cmCapVideo,cmCapReceive, kRolePresentation) && (findedContentSdesLocation == -1))
	{
		CSdesCap* alternativePcap = SetBestModeFromSdesCaplternate(pAlternativeCaps,eAes_Cm_128_Hmac_Sha1_80, cmCapVideo, (CSipComMode&)*pBestMode, kRolePresentation);
		if(alternativePcap && pCurRemoteCaps)
		{
			contentTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapVideo, pBestMode->GetIsTipMode(),kRolePresentation, alternativePcap->GetSdesCryptoSuite());
			if((int)contentTag != (int)alternativePcap->GetSdesTag())
			{
				pBestMode->UpdateRxSdesContentTag(contentTag);
			}
		}
		else if (pCurRemoteCaps && alternativePcap == NULL)
		{
			contentTag = (APIU32)pCurRemoteCaps->GetSdesTag(cmCapVideo, pBestMode->GetIsTipMode(), kRolePresentation);
			pBestMode->UpdateRxSdesContentTag(contentTag);
		}
		POBJDELETE(alternativePcap);
	}

	pCurRemoteCaps->UpdateSdesMkiInUseRx(pBestMode ,audioTag ,videoTag, dataTag, contentTag);
	if(bIsOffere && pAltLocalCaps)
		pAltLocalCaps->UpdateSdesMkiInUseTx(pBestMode ,audioTag ,videoTag, dataTag, contentTag);
	else
		pCurRemoteCaps->UpdateSdesMkiInUseTx(pBestMode ,audioTag ,videoTag, dataTag, contentTag);

	return pBestMode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::UpdateSdesBestModeTransmitUnencryptedSrtcp(CSipComMode* pBestMode, CSdesCap *pSdesCap, cmCapDataType type, ERoleLabel eRole) const
{
	CSdesCap *pOpositeSdesCap = pBestMode->GetSipSdes(type, cmCapTransmit, eRole);
	if (!pOpositeSdesCap)
		return;

	pOpositeSdesCap->SetIsSdesUnencryptedSrtcp(pSdesCap->GetIsSdesUnencryptedSrtcp());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
CSdesCap* CSipCaps::SetBestModeFromSdesCaplternate(const CSipCaps& pAlternativeCaps, APIU16 rCryptoSuite, cmCapDataType eMediaType, CSipComMode& pTempBestMode, ERoleLabel eRole) const
{

	capBuffer** pMediaCapList = NULL;
	int numOfRemoteSdesCapSets = 0;
	APIU16 remoteCryptoSuite = eUnknownSuiteParam;
	APIU32 remoteTag = 0;
	//BOOL bIsNeedToSendMKI = TRUE;
	//CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SEND_SRTP_MKI, bIsNeedToSendMKI);
	CSdesCap* alternativePcap = GetSdesCapFromAlternate(eMediaType, pAlternativeCaps,rCryptoSuite ,eRole);//alternative = local
	if(alternativePcap)
	{
		APIU16 alternativeCryptoSuite = alternativePcap->GetSdesCryptoSuite();
		APIU16 alternativeLocalTag = alternativePcap->GetSdesTag();
		GetSdesMediaCaps(eMediaType,&numOfRemoteSdesCapSets,&pMediaCapList, eRole);

		for(int k = 0; k < numOfRemoteSdesCapSets; k++)
		{
			CSdesCap* pCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)pMediaCapList[k]->capTypeCode,pMediaCapList[k]->dataCap);
			if (pCap)
			{
				remoteCryptoSuite = pCap->GetSdesCryptoSuite();
				remoteTag = pCap->GetSdesTag();
				if (remoteCryptoSuite == alternativeCryptoSuite /*&& remoteTag == alternativeLocalTag*/)
				{
					if(pCap && IsSdesCapValid(pCap) )
					{
						pTempBestMode.SetSipSdes(eMediaType,cmCapReceive,eRole,pCap);
						PTRACE(eLevelError,"SetBestModeFromSdesCaplternate: after set");
						//if in pcap there is mki break(select the cap with mki)
						PTRACE2INT(eLevelError,"SetBestModeFromSdesCaplternate: k= ",k);
						PTRACE2INT(eLevelError,"SetBestModeFromSdesCaplternate: GetIsSdesMkiInUse= ",pCap->GetIsSdesMkiInUse(0));

						//if( pTempBestMode.GetIsTipMode() || (bIsNeedToSendMKI == true && pCap->GetIsSdesMkiInUse(0) == true) )
						if(!(pCap->GetIsSdesUnencryptedSrtcp()))
						{
							PTRACE(eLevelError,"SetBestModeFromSdesCaplternate: encrypted srtcp");
							POBJDELETE(pCap);
							break;
						}
					}



				}

				POBJDELETE(pCap);
			}
		}
		//set tx with cap of local 80
		pTempBestMode.SetSipSdes(eMediaType,cmCapTransmit,eRole,alternativePcap);
	}
	return alternativePcap;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

CSdesCap* CSipCaps::GetSdesCapFromAlternate(cmCapDataType eMediaType, const CSipCaps& pAlternativeCaps, APIU16 cryptoSuite ,ERoleLabel eRole) const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	CSdesCap* pSdesCap = NULL;

	pAlternativeCaps.GetSdesMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			pSdesCap = (CSdesCap*)CBaseCap::AllocNewCap((CapEnum)eSdesCapCode,pMediaCapList[i]->dataCap);
			if(pSdesCap && pSdesCap->GetSdesCryptoSuite() == cryptoSuite)
			{
				return pSdesCap;
			}

		}
	}

	return pSdesCap;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSipCaps::IsSdesCapValid(CSdesCap *pSdesCap) const
{
	BYTE bIsValidParam = 0;
	char sdesKeySalt[MAX_BASE64_KEY_SALT_LEN];
	APIU32 i = 0;
	APIU8 mkiValueLen = 0;;
	BOOL bIsMkiInUse = FALSE;
	BOOL bIsMkiValueLenInUse = FALSE;

	APIU32 numOfKeys = pSdesCap->GetSdesNumOfKeysParam();
	if (numOfKeys >= MAX_NUM_OF_SDES_KEYS) {
		bIsValidParam |= 1;
		PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: numOfKeys is too high");
	}
	if (pSdesCap->GetIsSdesUnencryptedSrtp()) {
		bIsValidParam |= 1;
		PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: GetIsSdesUnencryptedSrtp is true ");
	}
	if (pSdesCap->GetIsSdesUnauthenticatedSrtp()) {
		bIsValidParam |= 1;
		PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: GetIsSdesUnauthenticatedSrtp is true ");
	}
	if (pSdesCap->GetIsSdesKdrInUse()) {
		APIU8 sdesKdr = pSdesCap->GetSdesKdr();
		if(sdesKdr < 1 || sdesKdr > 24) {
			bIsValidParam |= 1;
			PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: wrong KDR value ");
		}
	}
	if(pSdesCap->GetIsSdesWshInUse()) {
		APIU16 sdesWsh = pSdesCap->GetSdesWsh();
		if(sdesWsh < 64) {
			bIsValidParam |= 1;
			PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: wrong sdesWsh value ");
		}
	}

	if(pSdesCap->GetIsSdesFecOrderInUse()) {
		APIU16 sdesFecOrder = pSdesCap->GetSdesFecOrder();
		if(sdesFecOrder != eFecSrtp && sdesFecOrder != eSrtpFec) {
			bIsValidParam |= 1;
			PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: wrong sdesFecOrder value ");
		}
	}

	//check for all keys
	for(i = 0; i < numOfKeys; i++) {
		if((pSdesCap->GetSdesKeyMethod(i)) != eSdesInlineKeyMethod) {
			bIsValidParam |= 1;
			PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: wrong key method value ");
		}

		char *sdesBase64KeySalt = pSdesCap->GetSdesBase64KeySalt(i);
		if ( sdesBase64KeySalt )
		    strncpy(&sdesKeySalt[0], sdesBase64KeySalt, MAX_BASE64_KEY_SALT_LEN);

		//decrypt and check validity
		if(strlen(&sdesKeySalt[0]) != (MAX_BASE64_KEY_SALT_LEN - 4)) {
			bIsValidParam |= 1;
			PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: wrong sdesKeySalt length ");
		} else {
			int decodedLen = 0;
			int encodedLen = 0;
			if (!IsMasterSaltKeyValid(&sdesKeySalt[0], &decodedLen, &encodedLen)) {
				//bIsValidParam |= 1;
				PTRACE2INT(eLevelError,"CSipCaps::IsSdesCapValid: wrong sdesKeySalt value. decoded len =", decodedLen);
				PTRACE2INT(eLevelError,"CSipCaps::IsSdesCapValid: wrong sdesKeySalt value. encodedLen len =", encodedLen);
			}
		}

		if(pSdesCap->GetIsSdesLifeTimeInUse(i)) {
			APIU32 lifeTime = pSdesCap->GetSdesLifeTime(i);
			if(lifeTime > DEFAULT_LIFE_TIME) {// 2^31
				bIsValidParam |= 1;
				PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: wrong lifeTime value");
			}
		}

		bIsMkiInUse = pSdesCap->GetIsSdesMkiInUse(i);
		bIsMkiValueLenInUse = pSdesCap->GetIsSdesMkiValueLenInUse(i);
		mkiValueLen = 0;

		if(bIsMkiValueLenInUse) {
			mkiValueLen = pSdesCap->GetSdesMkiValueLen(i);
			if(mkiValueLen < 1 || mkiValueLen > 128) {
				bIsValidParam |= 1;
				PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: wrong mkiValueLen value");
			}
		}
		if(numOfKeys > 1 && (!bIsMkiInUse || !bIsMkiValueLenInUse)) {
			bIsValidParam |= 1;
			PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: more than 1 key and not mki value in use");
		}
		if((bIsMkiInUse && !bIsMkiValueLenInUse) || (!bIsMkiInUse && bIsMkiValueLenInUse)) {
			bIsValidParam |= 1;
			PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: more than 1 key and not mki value in use 2");
		}

	}

	if (numOfKeys > 1) {
		bIsMkiValueLenInUse = pSdesCap->GetIsSdesMkiValueLenInUse(0);
		mkiValueLen = pSdesCap->GetSdesMkiValueLen(0);

		for (i = 1; i < numOfKeys; i++)
		{
			if((pSdesCap->GetIsSdesMkiValueLenInUse(i) != bIsMkiValueLenInUse) ||
					(pSdesCap->GetSdesMkiValueLen(i) != mkiValueLen)) {
				bIsValidParam |= 1;
				PTRACE(eLevelError,"CSipCaps::IsSdesCapValid: more than 1 key and not mki value in use 3");
			}
		}
	}

	if(bIsValidParam)
		return FALSE;
	else
		return TRUE;
}
////////////////////////////////////////////////////////////////////
// Send to ICE only the related M-lines
//if you have any issue with this routine please consult yossi g. - before making changes!
BOOL CSipCaps::SetICEsdp(const sipSdpAndHeadersSt* sdp, std::ostream &ostr,CSipChanDifArr* pIceChannelsDifArr,DWORD opcode,BYTE IsSecondary, DWORD serviceId,BOOL IsEncryptedCall)
{
	BOOL IsFoundIceAttributes = FALSE;
	ICESessionsTypes IceSessionsTypes = eGeneralSession;
	//BOOL ApplicationSession = FALSE;
	BOOL IsPortValid = FALSE;
	WORD NumOfOpenMlines = 0;
	BOOL bVideoLineAlreadyTaken = FALSE; 
	BOOL bAppLineAlreadyTaken = FALSE;

	BOOL bSipEnableVideo = FALSE;
	BOOL bSipEnableFecc = FALSE;
	BOOL bIsFecc = FALSE;

	if (sdp->sipMediaLinesLength)
	{
		const sipMediaLinesEntrySt* pMediaLinesEntry = (const sipMediaLinesEntrySt*) sdp->capsAndHeaders;
		int mediaLinePos = 0;
		char ipStr[64];
		memset(ipStr,'\0',64);
		mcTransportAddress GeneralIp;
        memset(&GeneralIp, 0, sizeof(mcTransportAddress));
		cmCapDataType mediaType;
		ERoleLabel eRole;
		int numOfAudioLines = 0;
		int numOfVideoLinesWithNonZeroPort = 0;
		int numOfApplicationLines = 0;
		APIU8 videoSubType = eMediaLineSubTypeNull;
		BOOL bIsPanoramic = FALSE;

		GeneralIp.ipVersion =  enIpVersionMAX;
		

		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++)
		{
			const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			DWORD port = pMediaLine->mediaIp.transAddr.port;

			if(pMediaLine->type == eMediaLineTypeAudio)
			{
				numOfAudioLines+=1;
			}
			else if(pMediaLine->type == eMediaLineTypeVideo && port != 0)
			{

				numOfVideoLinesWithNonZeroPort+=1;

				if(videoSubType == pMediaLine->subType)
					bIsPanoramic = TRUE;

				videoSubType = pMediaLine->subType;
			}
			else if (pMediaLine->type == eMediaLineTypeApplication)
			{
				numOfApplicationLines++;
			}

			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
		}
		mediaLinePos = 0; 

		PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp - Number of Audio Mlines  : ", numOfAudioLines);

		PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp - Number of Video Mlines with non zero port  : ", numOfVideoLinesWithNonZeroPort);

		 PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp - Number of Application Mlines  : ", numOfApplicationLines);

		//IBM ST sends two audio m-lines. then we need to choose according to encryption, mark this situation
		// to avoid deleting the audio on other EPs
		//We might receive SDP with 2 mlines - one regular and one encrypted (like IBM ST EPs does)
		// We have decided to rely on the IsEncrypted flag from the target mode and not with the conf setup
		PTRACE2(eLevelInfoNormal,"CSipCaps::SetICEsdp - encryption ", IsEncryptedCall?"enabled!!":"disabled!!");

		for (int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
		{
			GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

			if (mediaType == cmCapBfcp)
				continue;

			CSipChanDif* pIceChanDif = NULL;
			pIceChanDif = pIceChannelsDifArr->GetChanDif(mediaType,cmCapTransmit,eRole);

			if (pIceChanDif && (pIceChanDif->IsAddChannel()) )
			{
				

				///    Video
				if((mediaType == cmCapVideo) && (eRole == kRolePeople))
				{
					PTRACE(eLevelInfoNormal,"CSipCaps::SetICEsdp - video enable");
					bSipEnableVideo = TRUE;
				}

				if(mediaType == cmCapData)
				{
                    // VNGR-20883
                    CServiceConfigList *pServiceSysCfg=CProcessBase::GetProcess()->GetServiceConfigList();

                    if( pServiceSysCfg )
                                        pServiceSysCfg->GetBOOLDataByKey(serviceId, "SIP_ENABLE_FECC", bSipEnableFecc);

                    if (bSipEnableFecc)
                                    PTRACE(eLevelInfoNormal,"CSipCaps::SetICEsdp - FECC enable");
				}
			}

		}
		//Incase this is dial out flow and we should downgrade video (secondary)
		//we will first send process req with A+V and after Reinvite will send Modify with Audio only.
		if(opcode == ICE_PROCESS_ANSWER_REQ && !bSipEnableVideo && IsSecondary)
		{
			bSipEnableVideo = TRUE;
		}

		if(pMediaLinesEntry->bMainMediaIpPresent)
		{
			::ipToString(pMediaLinesEntry->mainMediaIp.transAddr,ipStr,FALSE);
			GeneralIp = pMediaLinesEntry->mainMediaIp.transAddr;
			PTRACE2(eLevelInfoNormal,"CSipCaps::SetICEsdp -General IP :",ipStr);
		}
		else
		{	//Find audio IP
			for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++)
			{
				const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];

				if(pMediaLine->type == eMediaLineTypeAudio)
				{
					GeneralIp = pMediaLine->mediaIp.transAddr;
					::ipToString(pMediaLine->mediaIp.transAddr,ipStr,FALSE);
					PTRACE2(eLevelInfoNormal,"CSipCaps::SetICEsdp -audio IP :",ipStr);
				}
				mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			}
		}

		mediaLinePos = 0;

		TRACEINTO << "General IP version: " << (GeneralIp.ipVersion == eIpVersion6) ? "IPv6" : "IPv4";

		// ms ipv6----------------------------------
		string strIpVersion("IP4");
		if( GeneralIp.ipVersion == eIpVersion6 )//remote media ip type is ipv6)
		{
			strIpVersion = "IP6";
		}
		//------------------------------------------



		//Build General Msg
		ostr << "v=0"<<"\n";
		ostr << "o=- 0 0 IN "<< strIpVersion << " "<< ipStr <<"\n";
		ostr <<"s=session"<<"\n";
		ostr <<"c=IN " << strIpVersion << " " << ipStr <<"\n";
		ostr <<"b=CT:99980" <<"\n";
		ostr <<"t=0 0"<<"\n";

		//build MLines Msg

		for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {

			PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp -j :",j);

			const sipMediaLineSt *pMediaLine = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
			const capBuffer* pCapBuffer = (capBuffer*) &pMediaLine->caps[0];
			const BYTE*	pTemp = (const BYTE*)pCapBuffer;

			PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp -pMediaLine->type :",pMediaLine->type);
			PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp -pMediaLine->subType :",pMediaLine->subType);

			DWORD port = pMediaLine->mediaIp.transAddr.port;

			PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp  - m line port: ",port);

			if(pMediaLine->type == eMediaLineTypeAudio)
			{
				if (numOfAudioLines == 1)
				{
					IceSessionsTypes = eAudioSession;
					ostr <<"m=audio ";
				}
				else if((pMediaLine->subType == eMediaLineSubTypeRtpAvp) && !IsEncryptedCall)
		//		if(bSipEnableNonEncryptAudio/* && (pMediaLine->subType == eMediaLineSubTypeRtpAvp) */ )
				{
					IceSessionsTypes = eAudioSession;
					ostr <<"m=audio ";
				}
				else if((pMediaLine->subType == eMediaLineSubTypeRtpSavp) && IsEncryptedCall)
				{
					IceSessionsTypes = eAudioSession;
					ostr <<"m=audio ";
				}
				else
				{
					IceSessionsTypes = eNotSupportedSession;
					PTRACE(eLevelInfoNormal,"CSipCaps::SetICEsdp -ICE for Audio not added for more than one audio mline");
				}

			}
			else if (pMediaLine->type == eMediaLineTypeVideo)
			{
				if (bSipEnableVideo && !bVideoLineAlreadyTaken && port)
				{
					if(numOfVideoLinesWithNonZeroPort == 1 || bIsPanoramic)
					{
						IceSessionsTypes = eVideoSession;
						ostr <<"m=video ";
					}
					else if((pMediaLine->subType == eMediaLineSubTypeRtpAvp) && !IsEncryptedCall)
					{
						IceSessionsTypes = eVideoSession;
						ostr <<"m=video ";
					}
					else if((pMediaLine->subType == eMediaLineSubTypeRtpSavp) && IsEncryptedCall)
					{
						IceSessionsTypes = eVideoSession;
						ostr <<"m=video ";
					}
					else
					{
						IceSessionsTypes = eNotSupportedSession;
						PTRACE(eLevelInfoNormal,"CSipCaps::SetICEsdp -ICE for video not added for at least one video mline");
					}

				}
				else
				{
					IceSessionsTypes = eNotSupportedSession;
				}
			}
			else if(pMediaLine->type == eMediaLineTypeApplication && !bAppLineAlreadyTaken)
			{
				// BFCP will be ignored!! since it has different subtype.
				if(numOfApplicationLines == 1 && (pMediaLine->subType == eMediaLineSubTypeRtpAvp || pMediaLine->subType == eMediaLineSubTypeRtpSavp))
				{
					if(bSipEnableFecc)
					{
						IceSessionsTypes = eDataSession;

						ostr <<"m=application ";
						bIsFecc = TRUE;
					}
				} 
				else if((pMediaLine->subType == eMediaLineSubTypeRtpAvp) && !IsEncryptedCall)
				{
					if(bSipEnableFecc)
					{
						IceSessionsTypes = eDataSession;
						ostr <<"m=application ";
						bIsFecc = TRUE;
					}
				}
				else if((pMediaLine->subType == eMediaLineSubTypeRtpSavp) && IsEncryptedCall)
				{
					if(bSipEnableFecc)
					{
						IceSessionsTypes = eDataSession;
						ostr <<"m=application ";
						bIsFecc = TRUE;
					}
				}
				// BFCP will be ignored!! since it has different subtype.
				else
				{
					IceSessionsTypes = eNotSupportedSession;
					PTRACE(eLevelInfoNormal,"CSipCaps::SetICEsdp -ICE for application not added for more than one video mline");
					bIsFecc = FALSE;
				}
			}
			else if((pMediaLine->type == eMediaLineTypeNotSupported ) && (pMediaLine->notSupportedSubType == eMediaLineNotSupportedSubTypeControl))
			{
				IceSessionsTypes = eNotSupportedSession;
				PTRACE(eLevelInfoNormal,"CSipCaps::SetICEsdp -Not Supported Media Line");
			}


			PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp -IceSessionsTypes :",IceSessionsTypes);


			if((IceSessionsTypes == eGeneralSession)                                                  ||
					(IceSessionsTypes == eAudioSession)                                               ||
					(IceSessionsTypes == eVideoSession && bSipEnableVideo && !bVideoLineAlreadyTaken) ||
					(IceSessionsTypes == eDataSession && bSipEnableFecc && bIsFecc && !bAppLineAlreadyTaken))
			{
				

				if(port)
				{
					IsPortValid = TRUE;
					ALLOCBUFFER(portStr, 12);
					portStr[0] = '\0';
					sprintf(portStr,"%d",port);
					ostr << portStr;
					DEALLOCBUFFER(portStr);

					//if we don't have port don't take encryption and IP
					if(pMediaLine->subType == eMediaLineSubTypeRtpAvp)
						ostr << " RTP/AVP";
					else if(pMediaLine->subType == eMediaLineSubTypeRtpSavp)
						ostr << " RTP/SAVP";
					else if(pMediaLine->subType == eMediaLineSubTypeTcpRtpAvp )
						ostr << " TCP/RTP/AVP";
					else if(pMediaLine->subType == eMediaLineSubTypeTcpRtpSavp)
						ostr << " TCP/RTP/SAVP";

					ostr << "\n";

					mcTransportAddress MediaIP = pMediaLine->mediaIp.transAddr;
					//Add C line for IP in m line incase the Media IP addrss is different from the General IP address.
					PASSERTSTREAM(GeneralIp.ipVersion == enIpVersionMAX, "GeneralIp is not set??");
					if(!isIpAddressEqual(&MediaIP,&GeneralIp))
					{
						char mediaIpStr[64];
						memset(mediaIpStr, '\0', 64);
						::ipToString(pMediaLine->mediaIp.transAddr,mediaIpStr,FALSE);
						PTRACE2(eLevelInfoNormal,"CSipCaps::SetICEsdp - m line IP :",mediaIpStr);
						// ms ipv6----------------------------------
						string strMediaIpVersion("IP4");
						if( MediaIP.ipVersion == eIpVersion6 )//remote media ip type is ipv6)
						{
							strMediaIpVersion = "IP6";
						}
						//------------------------------------------


						ostr <<"c=IN " << strMediaIpVersion << " "<< mediaIpStr <<"\n";

					} 
//					else {
//						// remarked - no need to add c line if same as general	
//						PTRACE2(eLevelInfoNormal,"CSipCaps::SetICEsdp - m line IP :", ipStr);
//						ostr <<"c=IN IP4 "<< ipStr <<"\n";
//					}

                //BRDIGE-7376/7228 Since we count video with port!=0 then set taken only if port!=0
                if (eMediaLineTypeVideo == pMediaLine->type)
					bVideoLineAlreadyTaken = TRUE; 
				} // if(port)


			//	mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;

				BOOL IsFindRTCPAttribute = FALSE;
				int  MatchCandidateNum = 0;
				int RtpCandidateNum = 0;

				for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
				{
					CCapSetInfo capInfo = (CapEnum)pCapBuffer->capTypeCode;
					cmCapDataType eType = capInfo.GetCapType();

					if(BuildMLineCapsMsg(pCapBuffer,ostr,IceSessionsTypes,IsFindRTCPAttribute,port,RtpCandidateNum))
						IsFoundIceAttributes = TRUE;

					if(RtpCandidateNum != 0)
						MatchCandidateNum = RtpCandidateNum;

					pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
					pCapBuffer = (capBuffer*)pTemp;
				}

				PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp - MatchCandidateNum: ",MatchCandidateNum);

				// Incase there is no attribute of
				if(!IsFindRTCPAttribute && MatchCandidateNum != 0)
				{
					PTRACE(eLevelInfoNormal,"CSipCaps::SetICEsdp - No RTCP cap, will add RTCP manually");
					const capBuffer* pSecCapBuffer = (capBuffer*) &pMediaLine->caps[0];
					const BYTE*	pSecTemp = (const BYTE*)pSecCapBuffer;
					DWORD RtcpPort = 0;

					for (unsigned int j = 0 ; j < pMediaLine->numberOfCaps; j++)
					{
						FindRtcpPortInIceCaps(pSecCapBuffer,MatchCandidateNum,RtcpPort);

						if(RtcpPort)
							break;

						pSecTemp += sizeof(capBufferBase) + pSecCapBuffer->capLength;
						pSecCapBuffer = (capBuffer*)pSecTemp;
					}

					PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp - RTCP port found is : ",RtcpPort );

					if(RtcpPort != 0)
						AddRtcpAttribute(ostr,RtcpPort);

				}

			if (eMediaLineTypeApplication == pMediaLine->type)
					bAppLineAlreadyTaken = TRUE;			
			}
			else
			{//for example: Incase video is closed but we want to add fecc
				PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp in else - pMediaLine->numberOfCaps : ",pMediaLine->numberOfCaps );
				for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
				{
					pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
					pCapBuffer = (capBuffer*)pTemp;
				}
				//mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
			}

			mediaLinePos += sizeof(sipMediaLineBaseSt) + pMediaLine->lenOfDynamicSection;
		}
	}
	else
	{
		DBGPASSERT(YES);
		PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetICEsdp: No cap set. Length of dynamic section: ", sdp->lenOfDynamicSection);
	}

	//If all ports (of M lines) are 0, we should not send the SDP to the stuck.
	if(!IsPortValid)
		IsFoundIceAttributes = FALSE;

	return IsFoundIceAttributes;
}

////////////////////////////////////////////////////////////////////
BOOL CSipCaps::BuildMLineCapsMsg(const capBuffer *pCapBuffer,std::ostream &ostr,ICESessionsTypes IceSessionsTypes,BOOL &IsFindRTCP,DWORD port,int& RtpCandidateNum)
{
	BOOL bFindIce = FALSE;
	DWORD RtcpPort = 0;

	switch(pCapBuffer->capTypeCode)
	{
		case eIceUfragCapCode:
		{
			iceUfragCapStruct* iceUfrag = (iceUfragCapStruct*)pCapBuffer->dataCap;
			ostr<<"a=ice-ufrag:"<<iceUfrag->iceUfrag<<"\n";

			bFindIce = TRUE;
			break;

		}
		case eIcePwdCapCode:
		{
			icePwdCapStruct* icePwd = (icePwdCapStruct*)pCapBuffer->dataCap;
			ostr<<"a=ice-pwd:"<<icePwd->icePwd<<"\n";
			bFindIce = TRUE;

			break;
		}
		case eIceCandidateCapCode:
		{
			iceCandidateCapStruct* iceCandidate = (iceCandidateCapStruct*)pCapBuffer->dataCap;
			
			if(iceCandidate->candidateType == eCandidateTypeV6)
				ostr<<"a=x-candidate-ipv6:"<<iceCandidate->candidate<<"\n";
			else
				ostr<<"a=candidate:"<<iceCandidate->candidate<<"\n";

		    char* lineStr = NULL;
		    lineStr = new char[512];
		    memset (lineStr, '\0', 512);
		    strncpy(lineStr,iceCandidate->candidate,511);

		    if(strstr(iceCandidate->candidate, "typ host"))
		    {
		    	FindIpAddrInCandidateHostLine(lineStr,IceSessionsTypes, iceCandidate->candidateType);
		    }

		    //Find only the RTP port line
		    FindRtpAndRtcpPortsInCandidateHostLine(lineStr,TRUE,FALSE,port,RtpCandidateNum,RtcpPort);

		    //VNGFE-4207 the old-style of ICE candidates also include a=candidate
		    //and we would like to ignore old-style candidate and not send it to ICE-stack.
		    //bFindIce = TRUE;
		    break;
		}
		case eIceRemoteCandidateCapCode:
		{
			iceRemoteCandidateCapStruct* iceRemoteCandidate = (iceRemoteCandidateCapStruct*)pCapBuffer->dataCap;
			ostr<<"a=remote-candidates:"<<iceRemoteCandidate->candidate<<"\n";

	/*		char* lineStr = NULL;
			lineStr = new char[512];
			memset (lineStr, '\0', 512);
			strncpy(lineStr,iceRemoteCandidate->candidate,511);

			//Find only the RTP port line
			FindRtpAndRtcpPortsInCandidateHostLine(lineStr,TRUE,FALSE,port,RtpCandidateNum,RtcpPort);
*/
			break;
		}
		case eRtcpCapCode:
		{
			rtcpCapStruct* pRtcp =  (rtcpCapStruct*)pCapBuffer->dataCap;
			if(pRtcp->port != 0)
			{
				ALLOCBUFFER(rtcpPort, 6);
				rtcpPort[0] = '\0';
				sprintf(rtcpPort,"%d",pRtcp->port);
				ostr<<"a=rtcp:"<<rtcpPort<<"\n";
				IsFindRTCP = TRUE;

				DEALLOCBUFFER(rtcpPort);
			}
			break;
		}

	}
	return bFindIce;
}
////////////////////////////////////////////////////////////////////
void CSipCaps::AddRtcpAttribute(std::ostream &ostr,DWORD RtcpPort)
{
	ALLOCBUFFER(rtcpPort, 12);
	rtcpPort[0] = '\0';
	sprintf(rtcpPort,"%d",RtcpPort);
	ostr<<"a=rtcp:"<<rtcpPort<<"\n";
	DEALLOCBUFFER(rtcpPort);
}
////////////////////////////////////////////////////////////////////
void CSipCaps::FindRtcpPortInIceCaps(const capBuffer *pCapBuffer,int RtpCandidateNum,DWORD& RtcpPort)
{
	switch(pCapBuffer->capTypeCode)
	{
			case eIceCandidateCapCode:
			case eIceRemoteCandidateCapCode:
			{

				PTRACE(eLevelInfoNormal,"CSipCaps::FindRtcpPortInIceCaps: candidate cap!!");
				iceCandidateCapStruct* iceCandidate = (iceCandidateCapStruct*)pCapBuffer->dataCap;

				char* lineStr = NULL;
				lineStr = new char[512];
				memset (lineStr, '\0', 512);
				strncpy(lineStr,iceCandidate->candidate,511);

				//Find Rtcp port by line
				FindRtpAndRtcpPortsInCandidateHostLine(lineStr,FALSE,TRUE,0,RtpCandidateNum,RtcpPort);

				break;

			}
			default:
			{
				PTRACE(eLevelInfoNormal,"CSipCaps::FindRtcpPortInIceCaps: Not candidate cap!!");
			}
	}
}

////////////////////////////////////////////////////////////////////
// From txt to Buffer
void CSipCaps::BuildLocalIceSdp(char* pSdpString, DWORD sdpSize, CIceParams* pIceParams)
{
	PTRACE(eLevelInfoNormal,"CSipCaps::BuildLocalIceSdp ");
	int length = 0;
	ICESessionsTypes IceSessionsTypes = eGeneralSession;
	DWORD strSize = 0;
	char* pStrTail = NULL;
	char *pStrPointer = NULL;
	char *TmpPtr = NULL;

	char* pUfragLine = NULL;
	char* pPwdLine = NULL;
	BOOL IsGeneralSessionUfrag = FALSE;
	BOOL IsGeneralSessionPwd = FALSE;

	BOOL IsAudioSessionUfrag = FALSE;
	BOOL IsVideoSessionUfrag = FALSE;
	BOOL IsDataSessionUfrag = FALSE;
	BOOL IsAudioSessionPwd = FALSE;
	BOOL IsVideoSessionPwd = FALSE;
	BOOL IsDataSessionPwd = FALSE;


	if(sdpSize)
	{
		pStrPointer = pSdpString;
		while(strSize<sdpSize)
		{
			length = 0;

			pStrTail = pStrPointer ? strstr(pStrPointer,"\r\n") : NULL;
			if(pStrTail)
			{
				length = pStrTail - pStrPointer;
				if(length)
				{
					char* pOneLineString = new char[length+1];
					strncpy(pOneLineString,pStrPointer,length);
					pOneLineString[length] = '\0';

					CMedString str;
					str <<  "pOneLineString - " << pOneLineString <<" length - " << length;
					PTRACE2(eLevelInfoNormal,"CSipCaps::BuildLocalIceSdp - ",str.GetString());

					if(strstr(pOneLineString, "v=0"))
							PTRACE2(eLevelInfoNormal,"CSipCaps::BuildLocalIceSdp - Start : ",str.GetString());

					else
					{
						IsGeneralSessionUfrag = FALSE;
						IsGeneralSessionPwd = FALSE;

						// Audio Mline
						if(strstr(pOneLineString, "m=audio"))
                        {
							IceSessionsTypes = eAudioSession;
                            pIceParams->SetAudioRtpPort(GetRtpPortNumberFromIceSdp(pOneLineString));
							pIceParams->SetSubType(cmCapAudio , GetMLineSubType(pOneLineString));
                        }

						//Video Mline
						else if(strstr(pOneLineString, "m=video"))
                        {
							if(IceSessionsTypes == eAudioSession)
							{
								if(!IsAudioSessionPwd)
								{
									HandleOneSdpLine(pPwdLine, IceSessionsTypes, pIceParams);
									IsAudioSessionPwd = TRUE;
								}
								if(!IsAudioSessionUfrag)
								{
									HandleOneSdpLine(pUfragLine, IceSessionsTypes, pIceParams);
									IsAudioSessionUfrag = TRUE;
								}
							}
						//	CopyTmpArrToCaps(IceSessionsTypes);
							IceSessionsTypes = eVideoSession;
							pIceParams->SetVideoRtpPort(GetRtpPortNumberFromIceSdp(pOneLineString));
							pIceParams->SetSubType(cmCapVideo , GetMLineSubType(pOneLineString));

					    }

						//Data Mline
						else if(strstr(pOneLineString, "m=application"))
                        {
							if(IceSessionsTypes == eVideoSession)
							{
								if(!IsVideoSessionUfrag)
								{
									HandleOneSdpLine(pUfragLine, IceSessionsTypes, pIceParams);
									IsVideoSessionUfrag = TRUE;
								}
								if(!IsVideoSessionPwd)
								{
									HandleOneSdpLine(pPwdLine, IceSessionsTypes, pIceParams);
									IsVideoSessionPwd = TRUE;
								}
							}

						//	CopyTmpArrToCaps(IceSessionsTypes);
							IceSessionsTypes = eDataSession;
                            pIceParams->SetDataRtpPort(GetRtpPortNumberFromIceSdp(pOneLineString));
							pIceParams->SetSubType(cmCapData , GetMLineSubType(pOneLineString));
                        }
						else
						{
							if(strstr(pOneLineString, "a=rtcp"))
							{
								if(IceSessionsTypes == eAudioSession )
									pIceParams->SetAudioRtcpPort(FindRtcpPortNumberFromIceSdp(pOneLineString));
								if(IceSessionsTypes == eVideoSession )
									pIceParams->SetVideoRtcpPort(FindRtcpPortNumberFromIceSdp(pOneLineString));
								if(IceSessionsTypes == eDataSession )
									pIceParams->SetDataRtcpPort(FindRtcpPortNumberFromIceSdp(pOneLineString));
							}
							//Ufrag and pwd in general session
							if(IceSessionsTypes == eGeneralSession )
							{
								if(strstr(pOneLineString, "a=ice-ufrag"))
								{
									if(pUfragLine)
									{
										PDELETEA(pUfragLine);
										PTRACE(eLevelInfoNormal, "ERROR pUfragLine is already exists! ");
									}
									pUfragLine = new char[length+1];
									strncpy(pUfragLine,pOneLineString,length);
									pUfragLine[length] = '\0';
									IsGeneralSessionUfrag = TRUE;
								}
								if(strstr(pOneLineString, "a=ice-pwd"))
								{
									if(pPwdLine)
									{
										PDELETEA(pPwdLine);
										PTRACE(eLevelInfoNormal, "ERROR pPwdLine is already exists! ");
									}
									pPwdLine = new char[length+1];
									strncpy(pPwdLine,pOneLineString,length);
									pPwdLine[length] = '\0';
									IsGeneralSessionPwd = TRUE;
								}
							}
							//Audio
							if(IceSessionsTypes == eAudioSession)
							{
								if(strstr(pOneLineString, "a=ice-ufrag"))
									IsAudioSessionUfrag = TRUE;
								if(strstr(pOneLineString, "a=ice-pwd"))
									IsAudioSessionPwd = TRUE;
							}
							//Video
							if(IceSessionsTypes == eVideoSession)
							{
								if(strstr(pOneLineString, "a=ice-ufrag"))
									IsVideoSessionUfrag = TRUE;
								if(strstr(pOneLineString, "a=ice-pwd"))
									IsVideoSessionPwd = TRUE;
							}

							//Data
							if(IceSessionsTypes == eDataSession)
							{
								if(strstr(pOneLineString, "a=ice-ufrag"))
									IsDataSessionUfrag = TRUE;
								if(strstr(pOneLineString, "a=ice-pwd"))
									IsDataSessionPwd = TRUE;
							}

							if(!IsGeneralSessionUfrag && !IsGeneralSessionPwd)
							{	//Add to Caps
								HandleOneSdpLine(pOneLineString, IceSessionsTypes, pIceParams);
								CMedString str2;
								str2 <<  "pOneLineString - " << pOneLineString;
								PTRACE2(eLevelInfoNormal,"CSipCaps::BuildLocalIceSdp - ",str2.GetString());
							}
						}

					}
					strSize	= strSize + length+2; //+2 for /r/n
					TmpPtr = strstr(pStrPointer,"\r\n");
					pStrPointer = TmpPtr+2;

					PDELETEA(pOneLineString);
				}
			}
			else
				break;
		}

		if(IceSessionsTypes == eAudioSession)
		{
			if(!IsAudioSessionUfrag)
			{
				HandleOneSdpLine(pUfragLine, IceSessionsTypes, pIceParams);
				IsAudioSessionUfrag = TRUE;
			}
			if(!IsAudioSessionPwd)
			{
				HandleOneSdpLine(pPwdLine, IceSessionsTypes, pIceParams);
				IsAudioSessionPwd = TRUE;
			}
		}

		if(IceSessionsTypes == eVideoSession)
		{
			if(!IsVideoSessionUfrag)
			{
				HandleOneSdpLine(pUfragLine, IceSessionsTypes, pIceParams);
				IsVideoSessionUfrag = TRUE;
			}
			if(!IsVideoSessionPwd)
			{
				HandleOneSdpLine(pPwdLine, IceSessionsTypes, pIceParams);
				IsVideoSessionPwd = TRUE;
			}
		}

		if(IceSessionsTypes == eDataSession)
		{
			if(!IsDataSessionUfrag)
			{
				HandleOneSdpLine(pUfragLine, IceSessionsTypes, pIceParams);
				IsDataSessionUfrag = TRUE;
			}
			if(!IsDataSessionPwd)
			{
				HandleOneSdpLine(pPwdLine, IceSessionsTypes, pIceParams);
				IsDataSessionPwd = TRUE;
			}
		}


		PDELETEA(pUfragLine);
		PDELETEA(pPwdLine);
	}
}

////////////////////////////////////////////////////////////////////
// From txt to Buffer
// This function reorders the candidates, and for each m-line adds ufrag+pwd
// The output of this function is m-line: audio,video,data, after each m-line ufrag+pwd, and
// then the candidates by their order.
void CSipCaps::BuildLocalMSIceSdp(char* pSdpString, DWORD sdpSize, CIceParams* pIceParams)
{
	PTRACE(eLevelInfoNormal,"CSipCaps::BuildLocalMSIceSdp Start");
	int length = 0;
	ICESessionsTypes IceSessionsTypes = eGeneralSession;
	DWORD strSize = 0;
	char* pStrTail = NULL;
	char *pStrPointer = NULL;
	char *TmpPtr = NULL;

	char* pUfragLine = NULL;
	char* pPwdLine = NULL;

	enum
	{
		AUDIO_SESSION=1,
		VIDEO_SESSION,
		DATA_SESSION,
		GENERAL_SESSION,
		LAST_SESSION
	};

	if(sdpSize)
	{
		for (int i=1;i<LAST_SESSION;i++)
		{

			int candidate_i = 1;
			string candidate_i_str = string("a=candidate:1");
			bool found = true;
			while(found)
			{
				found = false;
				pStrPointer = pSdpString;
				strSize = 0;
				PTRACE2INT(eLevelInfoNormal,"CSipCaps::BuildLocalMSIceSdp ***I=",i);
				IceSessionsTypes = eGeneralSession;
				while(strSize<sdpSize)
				{
					length = 0;

					pStrTail = pStrPointer ? strstr(pStrPointer,"\r\n") : NULL;
					if(pStrTail)
					{
						length = pStrTail - pStrPointer;
						if(length)
						{
							char* pOneLineString = new char[length+1];
							strncpy(pOneLineString,pStrPointer,length);
							pOneLineString[length] = '\0';

							bool is_audio = strstr(pOneLineString, "m=audio");
							bool is_video = strstr(pOneLineString, "m=video");
							bool is_data = strstr(pOneLineString, "m=application");
							bool is_rtcp = strstr(pOneLineString, "a=rtcp");
							bool is_in = strstr(pOneLineString, "c=IN");
							bool is_ufrag = strstr(pOneLineString, "a=ice-ufrag");
							bool is_pwd = strstr(pOneLineString, "a=ice-pwd");
							bool is_remote = strstr(pOneLineString, "a=remote-candidates");

							if (is_ufrag && (NULL == pUfragLine))
							{
								pUfragLine = new char[length+1];
								strncpy(pUfragLine,pOneLineString,length);
								pUfragLine[length] = '\0';
							}
							else if (is_pwd && (NULL == pPwdLine))
							{
								pPwdLine = new char[length+1];
								strncpy(pPwdLine,pOneLineString,length);
								pPwdLine[length] = '\0';
							}
							else
							{
								// Audio Mline
								if(is_audio)
								{
									IceSessionsTypes = eAudioSession;
									pIceParams->SetAudioRtpPort(GetRtpPortNumberFromIceSdp(pOneLineString));
									pIceParams->SetSubType(cmCapAudio , GetMLineSubType(pOneLineString));
								}

								//Video Mline
								if(is_video)
								{
									IceSessionsTypes = eVideoSession;
									pIceParams->SetVideoRtpPort(GetRtpPortNumberFromIceSdp(pOneLineString));
									pIceParams->SetSubType(cmCapVideo , GetMLineSubType(pOneLineString));
								}

								//Data Mline
								if(is_data)
								{
									IceSessionsTypes = eDataSession;
		                            pIceParams->SetDataRtpPort(GetRtpPortNumberFromIceSdp(pOneLineString));
		                            pIceParams->SetSubType(cmCapData , GetMLineSubType(pOneLineString));
								}

								if(is_rtcp)
								{
									if(IceSessionsTypes == eAudioSession )
										pIceParams->SetAudioRtcpPort(FindRtcpPortNumberFromIceSdp(pOneLineString));
									if(IceSessionsTypes == eVideoSession )
										pIceParams->SetVideoRtcpPort(FindRtcpPortNumberFromIceSdp(pOneLineString));
								}

								bool is_candidate_i = strstr(pOneLineString, candidate_i_str.c_str());

								if ((((AUDIO_SESSION == i) && (IceSessionsTypes == eAudioSession)) && ((is_candidate_i) || ((is_remote || is_audio) && (1==candidate_i)))) ||
									(((VIDEO_SESSION  == i) && (IceSessionsTypes == eVideoSession)) && ((is_candidate_i) || ((is_remote ||is_video) && (1==candidate_i)))) ||
									(((DATA_SESSION  == i) && (IceSessionsTypes == eDataSession)) && ((is_candidate_i) || ((is_remote || is_data) && (1==candidate_i)))) ||
									(((GENERAL_SESSION == i) && (is_rtcp))) ||
									(((1 == i) && (1==candidate_i) && (is_in))))
								{
									//Add to Caps
									HandleOneSdpLine(pOneLineString, IceSessionsTypes, pIceParams);
									CMedString str2;
									str2 <<  "i="<< i << ",candidate_i="<<candidate_i<<",is_rtcp="<< is_rtcp << ",pOneLineString - " << pOneLineString;
									PTRACE2(eLevelInfoNormal,"CSipCaps::BuildLocalMSIceSdp - **** ",str2.GetString());

									if (is_audio || is_video || is_data)
									{
										PTRACE2INT(eLevelInfoNormal,"CSipCaps::BuildLocalMSIceSdp adding ufr+pwd i=",i);

										HandleOneSdpLine(pUfragLine, IceSessionsTypes, pIceParams);
										HandleOneSdpLine(pPwdLine, IceSessionsTypes, pIceParams);
									}
									else if (is_candidate_i)
									{
										found=true;
									}
								}
							}
							strSize	= strSize + length+2; //+2 for /r/n
							TmpPtr = strstr(pStrPointer,"\r\n");
							pStrPointer = TmpPtr+2;

							PDELETEA(pOneLineString);
						}
					}
					else
						break;
				} //end of while

				candidate_i++;
				std::stringstream ss;
				ss << candidate_i;
				candidate_i_str = string("a=candidate:")+ss.str();
				PTRACE2(eLevelInfoNormal,"CSipCaps::BuildLocalMSIceSdp acandidate_i_str=",candidate_i_str.c_str());
			} //end of while


		} //end of for
		PDELETEA(pUfragLine);
		PDELETEA(pPwdLine);
	}
}

eMediaLineSubType CSipCaps::GetMLineSubType(char* pMLineString) const
{
	if(!pMLineString)
		return eMediaLineSubTypeNull;
	else if(strstr(pMLineString, "TCP/RTP/AVP"))
		return eMediaLineSubTypeTcpRtpAvp;
	else if(strstr(pMLineString, "TCP/RTP/SAVP"))
		return eMediaLineSubTypeTcpRtpSavp;
	else if(strstr(pMLineString, "RTP/AVP"))
		return eMediaLineSubTypeRtpAvp;
	else if(strstr(pMLineString, "RTP/SAVP"))
		return eMediaLineSubTypeRtpSavp;
	else
		return eMediaLineSubTypeUnknown;
}
//////////////////////////////////////////////////////////
WORD CSipCaps::GetRtpPortNumberFromIceSdp(char* OneLineString)
{
	char* HeaderEndPtr = NULL;
	char* PortBeginPtr = NULL;
	WORD RtpPort = 0;

	int i = 0;

	//Find the begining of the Data
	HeaderEndPtr = strchr(OneLineString,'=');
	if(HeaderEndPtr)
	{
		PortBeginPtr = HeaderEndPtr+1;

		while (!isdigit(PortBeginPtr[0]))
		{
			(PortBeginPtr)++;
		}// we are now pointing to the beginning of the port number

		while(isdigit(PortBeginPtr[i]))
		{
			i++;
		}// we are now pointing to ethe end of the port number by i

		if(i>0)
		{
			char  PortStr[i+1];
			memset(PortStr,'\0',i+1);

			strncpy(PortStr,PortBeginPtr,i+1);

			RtpPort = atoi(PortStr);
		}


	}

	PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetRtpPortNumber Rtp Port - ", RtpPort);
	return RtpPort;
}

//////////////////////////////////////////////////////////
WORD CSipCaps::FindRtcpPortNumberFromIceSdp(char* OneLineString)
{
	char* HeaderEndPtr = NULL;
	char* DataBeginPtr = NULL;
	char* DataEndPtr = NULL;
	DWORD DataLength = 0;
	WORD RtcpPort = 0;


	HeaderEndPtr = strchr(OneLineString,':');
	if(HeaderEndPtr)
	{
		DataBeginPtr = HeaderEndPtr+1;

		//Get Data
		DataEndPtr = strchr(DataBeginPtr,'\0');
		DataLength = DataEndPtr - DataBeginPtr;

		PTRACE2INT(eLevelInfoNormal,"CSipCaps::FindRtcpPortNumber  DataLen: ",DataLength);

		if(DataLength)
		{
			char  DataStr[DataLength+1];
			memset(DataStr,'\0',DataLength+1);

			strncpy(DataStr,DataBeginPtr,DataLength+1);

			DataStr[DataLength] = '\0';

			CMedString str;
			str <<  "  DataStr: " <<DataStr;
			PTRACE2(eLevelInfoNormal,"CSipCaps::FindRtcpPortNumber  : ",str.GetString());

			RtcpPort = atoi(DataStr);
		}

	}

	PTRACE2INT(eLevelInfoNormal,"CSipCaps::FindRtcpPortNumber Rtcp Port - ", RtcpPort);
	return RtcpPort;

}
////////////////////////////////////////////////////////////////////
// candidateType = eCandidateTypeV4/eCandidateTypeV6
void CSipCaps::FindIpAddrInCandidateHostLine(char* OneLineString,ICESessionsTypes IceSessionsTypes, APIS8 candidateType)
{
	char* HeaderEndPtr = NULL;
	char* DataBeginPtr = NULL;
	char* DataEndPtr = NULL;
	DWORD DataLength = 0;
	int CharacterCounter = 0;
	int RtpChar;
	BOOL IsHostRtp = FALSE;
	BOOL isValidAddr = FALSE;
	mcTransportAddress HostIp;

	DWORD Port = 0;

	CMedString str;
	str <<  "  OneLineString: " << OneLineString;
	PTRACE2(eLevelInfoNormal,"CSipCaps::FindIpAddrInCandidateHostLine str : ",str.GetString());
	memset(&HostIp,0,sizeof(mcTransportAddress));

	DataBeginPtr = OneLineString;
	DataEndPtr = strchr(OneLineString,' ');

	while(DataEndPtr)
	{

		DataLength = DataEndPtr - DataBeginPtr;
		if(DataLength)
		{
			char  DataStr[DataLength+1];
			strncpy(DataStr,DataBeginPtr,DataLength);
			DataStr[DataLength] = '\0';

			CharacterCounter++;
			if(CharacterCounter == 2)
			{
				RtpChar = atoi(DataStr);
				if(RtpChar == 1)
					IsHostRtp = TRUE;
			}

			if (candidateType == eCandidateTypeV4)
			{
				if(::IsValidIpV4Address(DataStr) && IsHostRtp)
				{
					isValidAddr = TRUE;
					::stringToIpV4 (&HostIp, DataStr);
				}
				else
				{
					DataBeginPtr = DataEndPtr+1;
					DataEndPtr = strchr(DataBeginPtr,' ');
				}
			}
			else if (candidateType == eCandidateTypeV6)
			{
				int ret = ::stringToIpV6 (&HostIp, DataStr);
				if (!ret) // in case ipv6 address is not valid search for the next address ret = 0 fail, ret=1 success
				{
					DataBeginPtr = DataEndPtr+1;
					DataEndPtr = strchr(DataBeginPtr,' ');
				}
				else
					isValidAddr = TRUE;

			}
			else
				PTRACE2INT(eLevelInfoNormal,"CSipCaps::FindIpAddrInCandidateHostLine ip candidate type is not recognized:", candidateType);

			if (isValidAddr)
			{
				// Get Port
				char* BeginPortPtr = NULL;
				char* EndPortPtr = NULL;

				BeginPortPtr = DataEndPtr+1;
				EndPortPtr = strchr(BeginPortPtr,' ');

				DWORD PortLen = 0;
				PortLen = EndPortPtr - BeginPortPtr;

				if(PortLen)
				{
					char  PortStr[PortLen+1];
					strncpy(PortStr,BeginPortPtr,PortLen);
					PortStr[PortLen] = '\0';

					DWORD Port = atoi(PortStr);
					HostIp.port = Port;

					SetHostIpAddress(IceSessionsTypes,&HostIp);
				}
				DataEndPtr = NULL;
			}
		}

	}

}
////////////////////////////////////////////////////////////////////
void CSipCaps::FindRtpAndRtcpPortsInCandidateHostLine(char* OneLineString,BYTE FindRtpPort,BYTE FindRtcpPort,DWORD RtpPort,int& RtpCandidateNum, DWORD& RtcpPort)
{
	PTRACE(eLevelInfoNormal,"CSipCaps::FindRtpAndRtcpPortsInCandidateHostLine ");

	char* HeaderEndPtr = NULL;
	char* DataBeginPtr = NULL;
	char* DataEndPtr = NULL;
	DWORD DataLength = 0;
	int CharacterCounter = 0;
	int RtpChar;
	DWORD Port = 0;
	int CandidateNum = 0;
	BOOL IsRtpLine = FALSE;
	BOOL IsRtcpLine = FALSE;

	//1 1 UDP 2130706431 192.168.110.153 5830 typ host

	CMedString str;
	str <<  "  OneLineString: " << OneLineString;
	PTRACE2(eLevelInfoNormal,"CSipCaps::FindRtpAndRtcpPortsInCandidateHostLine str : ",str.GetString());

	DataBeginPtr = OneLineString;
	DataEndPtr = strchr(OneLineString,' ');

	while(DataEndPtr)
	{
		DataLength = DataEndPtr - DataBeginPtr;

		if(DataLength)
		{
			char  DataStr[DataLength+1];
			strncpy(DataStr,DataBeginPtr,DataLength);
			DataStr[DataLength] = '\0';

			CharacterCounter++;

			if(CharacterCounter == 1)
			{
				CandidateNum = atoi(DataStr);
			}

			if(CharacterCounter == 2)
			{
				RtpChar = atoi(DataStr);
				if(RtpChar == 1 )
					IsRtpLine = TRUE;
				if(RtpChar == 2  )
					IsRtcpLine = TRUE;
			}

			if(CharacterCounter == 3)
			{
				if(strncmp(DataStr,"UDP",3) && strncmp(DataStr,"TCP-ACT",7) && strncmp(DataStr,"TCP-PASS",8))
					break;
			}

			mcTransportAddress HostIp;
			memset(&HostIp,0,sizeof(mcTransportAddress));

			//stringToIpV6 returns 1 if address is valid ipv6 address
			if(::IsValidIpV4Address(DataStr) || ::stringToIpV6 (&HostIp, DataStr) == 1 )
			{

				//::stringToIpV4 (&HostIp, DataStr);

				// Get Port
				char* BeginPortPtr = NULL;
				char* EndPortPtr = NULL;

				BeginPortPtr = DataEndPtr+1;
				EndPortPtr = strchr(BeginPortPtr,' ');

				DWORD PortLen = 0;
				PortLen = EndPortPtr - BeginPortPtr;

				if(PortLen)
				{
					char  PortStr[PortLen+1];
					strncpy(PortStr,BeginPortPtr,PortLen);
					PortStr[PortLen] = '\0';

					Port = atoi(PortStr);
					PTRACE2INT(eLevelInfoNormal,"CSipCaps::FindRtpAndRtcpPortsInCandidateHostLine Port: ",Port);

					//If need to match Rtp port
					if(FindRtpPort && IsRtpLine)
					{
						if(Port == RtpPort)
						{
							PTRACE2INT(eLevelInfoNormal,"CSipCaps::FindRtpAndRtcpPortsInCandidateHostLine Find RTP Match -  Match CandidateNum: ",CandidateNum);
							RtpCandidateNum = CandidateNum;
						}
					}

					if(FindRtcpPort && IsRtcpLine && CandidateNum == RtpCandidateNum)
					{
						PTRACE(eLevelInfoNormal,"CSipCaps::FindRtpAndRtcpPortsInCandidateHostLine Find RTCP Match");
						RtcpPort = Port;
					}
				}
				DataEndPtr = NULL;
			}
			else
			{
				DataBeginPtr = DataEndPtr+1;
				DataEndPtr = strchr(DataBeginPtr,' ');
			}


		}
	}

}

////////////////////////////////////////////////////////////////////
void CSipCaps::SetHostIpAddress(ICESessionsTypes IceSessionsTypes,mcTransportAddress* HostIp)
{
	PTRACE(eLevelInfoNormal,"CSipCaps::SetHostIpAddress ");

	if(IceSessionsTypes == eAudioSession)
		memcpy(&(m_AudioHostPartyAddr),HostIp, sizeof(mcTransportAddress));
	if(IceSessionsTypes == eVideoSession)
		memcpy(&(m_VideoHostPartyAddr),HostIp, sizeof(mcTransportAddress));
	if(IceSessionsTypes == eDataSession)
		memcpy(&(m_DataHostPartyAddr),HostIp, sizeof(mcTransportAddress));


}

////////////////////////////////////////////////////////////////////
void CSipCaps::HandleOneSdpLine(char* OneLineString,ICESessionsTypes IceSessionsTypes, CIceParams* pIceParams)
{
	PTRACE(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine ");
	char* HeaderEndPtr = NULL;
	char* DataBeginPtr = NULL;
	char* DataEndPtr = NULL;
	DWORD DataLength = 0;
	CCapSetInfo capInfo = eUnknownAlgorithemCapCode;

	capBuffer* pCapBuffer = NULL;
	BaseCapStruct *pBaseCap = NULL;

	if(OneLineString)
	{
		//Find the begining of the Data
		HeaderEndPtr = strchr(OneLineString,':');
		if(strstr(OneLineString, "c=IN"))
		{
			char * addrStart = 1 + rindex(OneLineString, ' ');

			if(IceSessionsTypes == eGeneralSession)
				PTRACE2(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine found general c-line ", addrStart);
			else
				PTRACE2(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine found mline c-line ", addrStart);

			mcTransportAddress ip;
			memset (&ip, 0, sizeof (mcTransportAddress));
			::stringToIp(&ip, addrStart);
			pIceParams->SetIceMediaIp (IceSessionsTypes, &ip);
		}
		// if there is ":" in OneLineString e.g. a=ice-pwd:Ixz3gQv1aEdFs/ACv8ZMh8w6 / a=candidate:1 1 UDP 2130706431 10.226.232.64 49272 typ host
		// but there is NO "c=IN"  e.g. c=IN IP6 abcd:10:226:236::54
		 else if(HeaderEndPtr)
		{
			DataBeginPtr = HeaderEndPtr+1;

			//Get Data
			DataEndPtr = strchr(DataBeginPtr,'\0');
			DataLength = DataEndPtr - DataBeginPtr;

			PTRACE2INT(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine  DataLen: ",DataLength);

			if(DataLength)
			{
				char  DataStr[DataLength+1];
				strncpy(DataStr,DataBeginPtr,DataLength);
				DataStr[DataLength] = '\0';
				APIS8 candidateType = eCandidateTypeV4;

				CMedString str;
				str <<  "  DataStr: " <<DataStr;
				PTRACE2(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine  : ",str.GetString());

				if(strstr(OneLineString, "a=ice-ufrag"))
				{
					PTRACE(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine found  a=ice-ufrag ");
					capInfo = eIceUfragCapCode;

				}
				else if(strstr(OneLineString, "a=ice-pwd"))
				{
					PTRACE(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine found a=ice-pwd ");
					capInfo = eIcePwdCapCode;

				}
				else if(strstr(OneLineString, "a=candidate"))
				{
					PTRACE(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine found a=candidate ");
					capInfo = eIceCandidateCapCode;

						candidateType = eCandidateTypeV4;
				}
				else if(strstr(OneLineString, "a=x-candidate-ipv6"))
				{
					PTRACE(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine found x-candidate-ipv6 ");
					capInfo = eIceCandidateCapCode;
					candidateType = eCandidateTypeV6;
				}
				else if(strstr(OneLineString, "a=remote-candidates"))
				{
					PTRACE(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine found a=Remotecandidate ");
					capInfo = eIceRemoteCandidateCapCode;
				}
				else if(strstr(OneLineString, "a=rtcp"))
				{
					PTRACE(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine found a=rtcp ");
					capInfo = eRtcpCapCode;
				}
				if(capInfo!=eUnknownAlgorithemCapCode)
					AddSingleICECap(IceSessionsTypes,capInfo,DataStr, candidateType);
			}
		}

		else
			PTRACE(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine - Didn't find ICE header!");
	}
	else
		PTRACE(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine - String is NULL");

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddSingleICECap(ICESessionsTypes IceSessionsTypes,CCapSetInfo capInfo,char* DataStr, APIS8 candidateType)
{
	BYTE bSupported = capInfo.IsSupporedCap();
	CBaseCap* pIceCap;
	EResult eResOfSet;


	if (bSupported)
	{
		switch(capInfo)
		{
			case eIceUfragCapCode:
			{
			    pIceCap = (CICEUfragCap*)CBaseCap::AllocNewCap((CapEnum)eIceUfragCapCode,NULL);
			    if (pIceCap)
			    {
			        eResOfSet	= kSuccess;
			        eResOfSet &= ((CICEUfragCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,DataStr);
			    }
			    break;
			}
			case eIcePwdCapCode:
			{
			    pIceCap = (CICEPwdCap*)CBaseCap::AllocNewCap((CapEnum)eIcePwdCapCode,NULL);
			    if (pIceCap)
			    {
			        eResOfSet	= kSuccess;
			        eResOfSet &= ((CICEPwdCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,DataStr);
			    }
			    break;
			}
			case eIceCandidateCapCode:
			{
			    pIceCap = (CICECandidateCap*)CBaseCap::AllocNewCap((CapEnum)eIceCandidateCapCode,NULL);
			    if (pIceCap)
			    {
					//VNGFE-5845
					char newDataStr[512];
					memset (newDataStr, '\0', sizeof(newDataStr));

					//BRIDGE-10631: urgly workaround, should be removed as soon as EYE-402 is fixed by Eyeball					
					eProductType prodType = CProcessBase::GetProcess()->GetProductType();					
					if((eProductTypeNinja == prodType) && FixSrflxCandidateType( DataStr,newDataStr))
					{
						DataStr = newDataStr;
					}
					
					if(HandleSrflxCandidate(DataStr,newDataStr,IceSessionsTypes))
						DataStr = newDataStr;

			        eResOfSet	= kSuccess;
			        eResOfSet &= ((CICECandidateCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,DataStr, candidateType);
			    }
			    break;
			}
			case eIceRemoteCandidateCapCode:
			{
			    pIceCap = (CICERemoteCandidateCap*)CBaseCap::AllocNewCap((CapEnum)eIceRemoteCandidateCapCode,NULL);
			    if (pIceCap)
			    {
			        eResOfSet	= kSuccess;
			        eResOfSet &= ((CICERemoteCandidateCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,DataStr);
			    }
			    break;
			}
			case eRtcpCapCode:
			{
			    pIceCap = (CRtcpCap*)CBaseCap::AllocNewCap((CapEnum)eRtcpCapCode,NULL);
			    if (pIceCap)
			    {
			        eResOfSet	= kSuccess;
			        eResOfSet &= ((CRtcpCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,DataStr);
			    }
			    break;
				}
			default:
				PTRACE(eLevelInfoNormal,"CSipCaps::AddSingleICECap: Unknown ICE capcode!!");
				PASSERTSTREAM(TRUE, "Unknown ICE capcode " << (capInfo));
				pIceCap = NULL;
				eResOfSet = kFailure;
		}
		if(pIceCap)
		{
			if(eResOfSet)
			{
				capBuffer* pCapBuffer = pIceCap->GetAsCapBuffer();

				if(pCapBuffer)
				{
				    pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
				    //if((CapEnum)capInfo == eIceCandidateCapCode)
				    //	AddICECapToTmpArray(pCapBuffer,DataStr);
				    //else
				    AddICECapSet(IceSessionsTypes,pCapBuffer);
				}
				else
				{
					PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Create cap buffer has failed");
				}
				PDELETEA(pCapBuffer);
			}
			else
				PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Set struct has failed");

			pIceCap->FreeStruct();
		}
		POBJDELETE(pIceCap);
	}
}

//VNGFE-5845
bool CSipCaps::HandleSrflxCandidate(const char* inDataStr,char* outDataStr,ICESessionsTypes IceSessionsTypes )
{
	bool isRtp = false;

	mcTransportAddress addrIPSrvflx;
	memset(&addrIPSrvflx,0,sizeof(mcTransportAddress));
	mcTransportAddress* rtpAddrIPSrvflx;

	if(IsICESrvflxCandidate(inDataStr , isRtp , addrIPSrvflx))
	{
		switch (IceSessionsTypes)
		{
			case eAudioSession:
			{
				rtpAddrIPSrvflx = &m_AudioSrflxPartyAddr;
				break;
			}
			case eVideoSession:
			{
				rtpAddrIPSrvflx = &m_VideoSrflxPartyAddr;
				break;
			}
			case eDataSession:
			{
				rtpAddrIPSrvflx = &m_DataSrflxPartyAddr;
				break;
			}
			default:
			{
				return false;
			}
		}

		if(isRtp)
		{
			memset(rtpAddrIPSrvflx,0,sizeof(mcTransportAddress));
			memcpy(rtpAddrIPSrvflx,&(addrIPSrvflx), sizeof(mcTransportAddress));
		}
		else if (::isApiTaNull(const_cast<mcTransportAddress*>(rtpAddrIPSrvflx)) == FALSE)
		{
			if((SwitchICESrvflxIPAddr(inDataStr, *rtpAddrIPSrvflx, outDataStr)))
				return true;
		}
	}

	return false;
}

//VNGFE-5845
bool CSipCaps::SwitchICESrvflxIPAddr(const char* pInBuffer, const mcTransportAddress addrIPSrvflxToReplace, char* pOutBuffer)
{
		char strIPSrvflxToReplace[16] = "\0";

		char* HeaderEndPtr = NULL;
		char* DataBeginPtr = NULL;
		char* DataEndPtr = NULL;
		DWORD DataLength = 0;
		bool bSwaped = false;
		char* OneLineString = NULL;
		CMedString strNewCandLine;

		if(!pInBuffer || !pOutBuffer)
			return false;

		OneLineString = new char[512];
		memset (OneLineString, '\0', 512);
		strncpy(OneLineString,pInBuffer,strlen(pInBuffer));
		OneLineString[sizeof(OneLineString) - 1] = '\0';

		if(strstr(OneLineString, "typ srflx"))
		{
			CMedString str;
			str <<  "  OneLineString: " << OneLineString;

			//memset(addrIPSrvflx,0,sizeof(mcTransportAddress));
			DataBeginPtr = OneLineString;
			DataEndPtr = strchr(OneLineString,' ');

			while(DataEndPtr && !bSwaped)
			{
				DataLength = DataEndPtr - DataBeginPtr;
				if(DataLength)
				{
					char  DataStr[DataLength+1];
					strncpy(DataStr,DataBeginPtr,DataLength);
					DataStr[DataLength] = '\0';


					if(::IsValidIpV4Address(DataStr))
					{
						unsigned char* ip = (unsigned char*)&addrIPSrvflxToReplace;
						snprintf(strIPSrvflxToReplace, sizeof(strIPSrvflxToReplace) ,"%d.%d.%d.%d", (int)ip[3], (int)ip[2], (int)ip[1], (int)ip[0]);

						//Before IP
						DWORD DataLength2 = DataBeginPtr - OneLineString;
						char  DataStr2[DataLength2+1];
						strncpy(DataStr2,OneLineString,DataLength2);
						DataStr2[DataLength2] = '\0';

						//After IP
						DWORD DataLength3 = strlen(OneLineString) - (DataEndPtr - OneLineString);
						char  DataStr3[DataLength3+1];
						strncpy(DataStr3,DataEndPtr,DataLength3);
						DataStr3[DataLength3] = '\0';

						//Final
						strNewCandLine << DataStr2 << strIPSrvflxToReplace << DataStr3;
						strncpy(pOutBuffer, strNewCandLine.GetString(), strNewCandLine.GetStringLength());
						bSwaped = true;
						PTRACE2(eLevelInfoNormal,"CSipCaps::SwitchICESrvflxIPAddr new candidate string: ",strNewCandLine.GetString());
					}
					else
					{
						DataBeginPtr = DataEndPtr+1;
						DataEndPtr = strchr(DataBeginPtr,' ');
					}

				}

			}

		}
		
		delete [] OneLineString;
		return bSwaped;
}

//VNGFE-5845
bool CSipCaps::IsICESrvflxCandidate(const char* pBuffer , bool& isRtp , mcTransportAddress& addrIPSrvflx)
{
	char* HeaderEndPtr = NULL;
	char* DataBeginPtr = NULL;
	char* DataEndPtr = NULL;
	DWORD DataLength = 0;
	int CharacterCounter = 0;
	int RtpChar;
	bool bIsSrflx = false;
	BOOL IsSrvflxRtp = FALSE;
	char* OneLineString = NULL;


	if(!pBuffer)
		return false;

	OneLineString = new char[512];
	memset (OneLineString, '\0', 512);
	strncpy(OneLineString,pBuffer,strlen(pBuffer));
	OneLineString[sizeof(OneLineString) - 1] = '\0';

	if(strstr(OneLineString, "typ srflx"))
	{
		CMedString str;
		str <<  "  OneLineString: " << OneLineString;

		DataBeginPtr = OneLineString;
		DataEndPtr = strchr(OneLineString,' ');

		while(DataEndPtr && !bIsSrflx)
		{

			DataLength = DataEndPtr - DataBeginPtr;
			if(DataLength)
			{
				char  DataStr[DataLength+1];
				strncpy(DataStr,DataBeginPtr,DataLength);
				DataStr[DataLength] = '\0';

				CharacterCounter++;
				if(CharacterCounter == 2)
				{
					RtpChar = atoi(DataStr);
					if(RtpChar == 1)
						IsSrvflxRtp = TRUE;
				}

				if(::IsValidIpV4Address(DataStr))
				{
					::stringToIpV4 (&addrIPSrvflx, DataStr);
					isRtp  = IsSrvflxRtp;
					bIsSrflx = true;
					PTRACE2(eLevelInfoNormal,"CSipCaps::IsICESrvflxCandidate Srvflx IP address found : ",DataStr);
				}
				else
				{
					DataBeginPtr = DataEndPtr+1;
					DataEndPtr = strchr(DataBeginPtr,' ');
				}

			}

		}
	}

	delete [] OneLineString;
	return bIsSrflx;
}

//BRIDGE-10631: urgly workaround, should be removed as soon as EYE-402 is fixed by Eyeball
bool  CSipCaps::FixSrflxCandidateType(const char* inDataStr,char* outDataStr)
{
	char	szOneLineString[512];
	char*	pszTyp = NULL;

	if(!inDataStr || !outDataStr)
	{
		PTRACE(eLevelError,"CSipCaps::FixSrflxCandidateType: invalid(NULL) pointer!!");
		return false;
	}
	PTRACE(eLevelInfoNormal,"CSipCaps::FixSrflxCandidateType ");
	
	strncpy(szOneLineString, inDataStr, sizeof(szOneLineString) - 1);
	szOneLineString[sizeof(szOneLineString) - 1] = '\0';

	pszTyp = strstr(szOneLineString, "typ srflx");
	if(pszTyp)
	{
		strncpy(outDataStr, szOneLineString, pszTyp-szOneLineString);
		outDataStr[pszTyp-szOneLineString] = '\0';
		
		if((NULL == strstr(pszTyp, "raddr")) || (NULL == strstr(pszTyp, "rport")))
		{
			strcat(outDataStr, "typ host");
			
			PTRACE2(eLevelInfoNormal,"CSipCaps::FixSrflxCandidate :  ", outDataStr);
			return true;
		}
	}
	
	return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddICECapSet(ICESessionsTypes IceSessionsTypes, const capBuffer* pCapSet)
{
	if (IceSessionsTypes == eAudioSession || IceSessionsTypes == eGeneralSession)
	{
		m_IceAudioCapList[m_numOfIceAudioCaps] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_IceAudioCapList[m_numOfIceAudioCaps], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_IceAudioCapList[m_numOfIceAudioCaps]->capLength		= pCapSet->capLength;
	//	m_IceAudioCapList[m_numOfIceAudioCaps]->capTypeCode    = cmCapAudio;
		m_IceAudioCapList[m_numOfIceAudioCaps]->capTypeCode    = pCapSet->capTypeCode;
		m_IceAudioCapList[m_numOfIceAudioCaps]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_IceAudioCapList[m_numOfIceAudioCaps]->dataCap, pCapSet->dataCap,pCapSet->capLength);


		m_numOfIceAudioCaps++;
	}
	if (IceSessionsTypes == eVideoSession || IceSessionsTypes == eGeneralSession)
	{
		m_IceVideoCapList[m_numOfIceVideoCaps] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_IceVideoCapList[m_numOfIceVideoCaps], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_IceVideoCapList[m_numOfIceVideoCaps]->capLength		= pCapSet->capLength;
	//	m_IceVideoCapList[m_numOfIceVideoCaps]->capTypeCode	= cmCapVideo;
		m_IceVideoCapList[m_numOfIceVideoCaps]->capTypeCode	= pCapSet->capTypeCode;
		m_IceVideoCapList[m_numOfIceVideoCaps]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_IceVideoCapList[m_numOfIceVideoCaps]->dataCap, pCapSet->dataCap,pCapSet->capLength);

		m_numOfIceVideoCaps++;
	}
	if (IceSessionsTypes == eDataSession || IceSessionsTypes == eGeneralSession)
	{
		m_IceDataCapList[m_numOfIceDataCaps] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
		memset(m_IceDataCapList[m_numOfIceDataCaps], 0, sizeof(capBufferBase) + pCapSet->capLength);
		m_IceDataCapList[m_numOfIceDataCaps]->capLength		= pCapSet->capLength;
	//	m_IceDataCapList[m_numOfIceDataCaps]->capTypeCode  = cmCapData;
		m_IceDataCapList[m_numOfIceDataCaps]->capTypeCode	= pCapSet->capTypeCode;
		m_IceDataCapList[m_numOfIceDataCaps]->sipPayloadType = pCapSet->sipPayloadType;
		memcpy(m_IceDataCapList[m_numOfIceDataCaps]->dataCap, pCapSet->dataCap,pCapSet->capLength);

		m_numOfIceDataCaps++;
	}
	// else if(IceSessionsTypes == eGeneralSession)
// 	{
// 		PTRACE(eLevelInfoNormal,"CSipCaps::HandleOneSdpLine - Add to General buffer");
// 		m_IceGeneralCapList[m_numOfIceGeneralCaps] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
// 		memset(m_IceGeneralCapList[m_numOfIceGeneralCaps], 0, sizeof(capBufferBase) + pCapSet->capLength);
// 		m_IceGeneralCapList[m_numOfIceGeneralCaps]->capLength		= pCapSet->capLength;
// 		m_IceGeneralCapList[m_numOfIceGeneralCaps]->capTypeCode	= pCapSet->capTypeCode;
// 		m_IceGeneralCapList[m_numOfIceGeneralCaps]->sipPayloadType = pCapSet->sipPayloadType;
// 		memcpy(m_IceGeneralCapList[m_numOfIceGeneralCaps]->dataCap, pCapSet->dataCap,pCapSet->capLength);

// 		m_numOfIceGeneralCaps++;
// 	}
// 	else
// 	{
// 		FTRACESTR(eLevelInfoNormal) << "CSipCaps::AddICECapSet: unsupported media type - " << IceSessionsTypes;
// 	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*void CSipCaps::CopyTmpArrToCaps(ICESessionsTypes IceSessionsTypes)
{
	for(int i=0; i< NumOfCandidates;i++)
	{
		if(m_RtpCandidateArr[i])
		{
			if (IceSessionsTypes == eAudioSession || IceSessionsTypes == eGeneralSession)
			{
				m_IceAudioCapList[m_numOfIceAudioCaps] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + m_RtpCandidateArr[i]->capLength]);
				memset(m_IceAudioCapList[m_numOfIceAudioCaps], 0, sizeof(capBufferBase) + m_RtpCandidateArr[i]->capLength);
				m_IceAudioCapList[m_numOfIceAudioCaps]->capLength		= m_RtpCandidateArr[i]->capLength;
				m_IceAudioCapList[m_numOfIceAudioCaps]->capTypeCode    = m_RtpCandidateArr[i]->capTypeCode;
				m_IceAudioCapList[m_numOfIceAudioCaps]->sipPayloadType = m_RtpCandidateArr[i]->sipPayloadType;
				memcpy(m_IceAudioCapList[m_numOfIceAudioCaps]->dataCap, m_RtpCandidateArr[i]->dataCap,m_RtpCandidateArr[i]->capLength);

				m_numOfIceAudioCaps++;
			}
			if (IceSessionsTypes == eAudioSession || IceSessionsTypes == eGeneralSession)
			{
				m_IceVideoCapList[m_numOfIceVideoCaps] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + m_RtpCandidateArr[i]->capLength]);
				memset(m_IceVideoCapList[m_numOfIceVideoCaps], 0, sizeof(capBufferBase) + m_RtpCandidateArr[i]->capLength);
				m_IceVideoCapList[m_numOfIceVideoCaps]->capLength		= m_RtpCandidateArr[i]->capLength;
				m_IceVideoCapList[m_numOfIceVideoCaps]->capTypeCode    = m_RtpCandidateArr[i]->capTypeCode;
				m_IceVideoCapList[m_numOfIceVideoCaps]->sipPayloadType = m_RtpCandidateArr[i]->sipPayloadType;
				memcpy(m_IceVideoCapList[m_numOfIceVideoCaps]->dataCap, m_RtpCandidateArr[i]->dataCap,m_RtpCandidateArr[i]->capLength);

				m_numOfIceVideoCaps++;
			}
			if (IceSessionsTypes == eDataSession || IceSessionsTypes == eGeneralSession)
			{
				m_IceDataCapList[m_numOfIceDataCaps] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + m_RtpCandidateArr[i]->capLength]);
				memset(m_IceDataCapList[m_numOfIceDataCaps], 0, sizeof(capBufferBase) + m_RtpCandidateArr[i]->capLength);
				m_IceDataCapList[m_numOfIceDataCaps]->capLength		= m_RtpCandidateArr[i]->capLength;
				m_IceDataCapList[m_numOfIceDataCaps]->capTypeCode	= m_RtpCandidateArr[i]->capTypeCode;
				m_IceDataCapList[m_numOfIceDataCaps]->sipPayloadType = m_RtpCandidateArr[i]->sipPayloadType;
				memcpy(m_IceDataCapList[m_numOfIceDataCaps]->dataCap, m_RtpCandidateArr[i]->dataCap,m_RtpCandidateArr[i]->capLength);

				m_numOfIceDataCaps++;
			}

		}
		if(m_RtcpCandidateArr[i])
		{
			if (IceSessionsTypes == eAudioSession || IceSessionsTypes == eGeneralSession)
			{
				m_IceAudioCapList[m_numOfIceAudioCaps] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + m_RtcpCandidateArr[i]->capLength]);
				memset(m_IceAudioCapList[m_numOfIceAudioCaps], 0, sizeof(capBufferBase) + m_RtcpCandidateArr[i]->capLength);
				m_IceAudioCapList[m_numOfIceAudioCaps]->capLength		= m_RtcpCandidateArr[i]->capLength;
				m_IceAudioCapList[m_numOfIceAudioCaps]->capTypeCode    = m_RtcpCandidateArr[i]->capTypeCode;
				m_IceAudioCapList[m_numOfIceAudioCaps]->sipPayloadType = m_RtcpCandidateArr[i]->sipPayloadType;
				memcpy(m_IceAudioCapList[m_numOfIceAudioCaps]->dataCap, m_RtcpCandidateArr[i]->dataCap,m_RtcpCandidateArr[i]->capLength);

				m_numOfIceAudioCaps++;
			}
			if (IceSessionsTypes == eAudioSession || IceSessionsTypes == eGeneralSession)
			{
					m_IceVideoCapList[m_numOfIceVideoCaps] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + m_RtcpCandidateArr[i]->capLength]);
					memset(m_IceVideoCapList[m_numOfIceVideoCaps], 0, sizeof(capBufferBase) + m_RtcpCandidateArr[i]->capLength);
					m_IceVideoCapList[m_numOfIceVideoCaps]->capLength		= m_RtcpCandidateArr[i]->capLength;
					m_IceVideoCapList[m_numOfIceVideoCaps]->capTypeCode    = m_RtcpCandidateArr[i]->capTypeCode;
					m_IceVideoCapList[m_numOfIceVideoCaps]->sipPayloadType = m_RtcpCandidateArr[i]->sipPayloadType;
					memcpy(m_IceVideoCapList[m_numOfIceVideoCaps]->dataCap, m_RtcpCandidateArr[i]->dataCap,m_RtcpCandidateArr[i]->capLength);

					m_numOfIceVideoCaps++;
			}
			if (IceSessionsTypes == eDataSession || IceSessionsTypes == eGeneralSession)
			{
				m_IceDataCapList[m_numOfIceDataCaps] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + m_RtcpCandidateArr[i]->capLength]);
				memset(m_IceDataCapList[m_numOfIceDataCaps], 0, sizeof(capBufferBase) + m_RtcpCandidateArr[i]->capLength);
				m_IceDataCapList[m_numOfIceDataCaps]->capLength		= m_RtcpCandidateArr[i]->capLength;
				m_IceDataCapList[m_numOfIceDataCaps]->capTypeCode	= m_RtcpCandidateArr[i]->capTypeCode;
				m_IceDataCapList[m_numOfIceDataCaps]->sipPayloadType = m_RtcpCandidateArr[i]->sipPayloadType;
				memcpy(m_IceDataCapList[m_numOfIceDataCaps]->dataCap, m_RtcpCandidateArr[i]->dataCap,m_RtcpCandidateArr[i]->capLength);

				m_numOfIceDataCaps++;
			}

		}


	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddICECapToTmpArray(const capBuffer* pCapSet,char* DataStr)
{
	int CandidateNum = 0;
	BOOL IsRtpLine = FALSE;
	BOOL IsRtcpLine = FALSE;

	FindFundationNumInLocalCandidateLine(DataStr,IsRtpLine,IsRtcpLine,CandidateNum);

	if(CandidateNum)
	{
		if(IsRtpLine)
			AddCapToRtpArr(CandidateNum,pCapSet);
		if(IsRtcpLine)
			AddCapToRtcpArr(CandidateNum,pCapSet);
	}


}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::FindFundationNumInLocalCandidateLine(char* OneLineString,BYTE& IsRtpLine,BYTE& IsRtcpLine,int& CandidateNum)
{
	PTRACE(eLevelInfoNormal,"CSipCaps::FindFundationNumInLocalCandidateLine ");

	char* HeaderEndPtr = NULL;
	char* DataBeginPtr = NULL;
	char* TypeBeginPtr = NULL;
	char* DataEndPtr = NULL;
	DWORD DataLength = 0;
	DWORD TypeLength = 0;
	int CharacterCounter = 0;
	int RtpChar;

	//1 1 UDP 2130706431 192.168.110.153 5830 typ host

	CMedString str;
	str <<  "  OneLineString: " << OneLineString;
	PTRACE2(eLevelInfoNormal,"CSipCaps::FindFundationNumInLocalCandidateLine str : ",str.GetString());


	// find fundation
	DataBeginPtr = OneLineString;
	DataEndPtr = strchr(OneLineString,' ');

	DataLength = DataEndPtr - DataBeginPtr;

	if(DataLength)
	{
		char  DataStr[DataLength+1];
		strncpy(DataStr,DataBeginPtr,DataLength);
		DataStr[DataLength] = '\0';

		CandidateNum = atoi(DataStr);
		PTRACE2INT(eLevelInfoNormal,"CSipCaps::FindFundationNumInLocalCandidateLine CandidateNum: ",CandidateNum);

		//// find Type - RTP or RTCP
		TypeBeginPtr = DataEndPtr+1;
		DataEndPtr = strchr(TypeBeginPtr,' ');

		TypeLength = DataEndPtr - TypeBeginPtr;

		char  TypeStr[TypeLength+1];
		strncpy(TypeStr,TypeBeginPtr,TypeLength);
		TypeStr[TypeLength] = '\0';

		RtpChar = atoi(TypeStr);
		if(RtpChar == 1 )
		{
			PTRACE(eLevelInfoNormal,"CSipCaps::FindFundationNumInLocalCandidateLine RTPline: ");
			IsRtpLine = TRUE;
		}
		if(RtpChar == 2  )
		{
			PTRACE(eLevelInfoNormal,"CSipCaps::FindFundationNumInLocalCandidateLine RTCPline: ");
			IsRtcpLine = TRUE;
		}
	}


}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddCapToRtpArr(int FundationNum,const capBuffer* pCapSet)
{
	m_RtpCandidateArr[FundationNum] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
	memset(m_RtpCandidateArr[FundationNum], 0, sizeof(capBufferBase) + pCapSet->capLength);
	m_RtpCandidateArr[FundationNum]->capLength		= pCapSet->capLength;
	m_RtpCandidateArr[FundationNum]->capTypeCode    = pCapSet->capTypeCode;
	m_RtpCandidateArr[FundationNum]->sipPayloadType = pCapSet->sipPayloadType;
	memcpy(m_RtpCandidateArr[FundationNum]->dataCap, pCapSet->dataCap,pCapSet->capLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::AddCapToRtcpArr(int FundationNum,const capBuffer* pCapSet)
{
	m_RtcpCandidateArr[FundationNum] = (capBuffer*)(new BYTE[sizeof(capBufferBase) + pCapSet->capLength]);
	memset(m_RtcpCandidateArr[FundationNum], 0, sizeof(capBufferBase) + pCapSet->capLength);
	m_RtcpCandidateArr[FundationNum]->capLength		= pCapSet->capLength;
	m_RtcpCandidateArr[FundationNum]->capTypeCode    = pCapSet->capTypeCode;
	m_RtcpCandidateArr[FundationNum]->sipPayloadType = pCapSet->sipPayloadType;
	memcpy(m_RtcpCandidateArr[FundationNum]->dataCap, pCapSet->dataCap,pCapSet->capLength);
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int CSipCaps::GetNumOfIceAudioCaps()
{
	return m_numOfIceAudioCaps;
}
int CSipCaps::GetNumOfIceVideoCaps()
{
	return m_numOfIceVideoCaps;
}
int CSipCaps::GetNumOfIceDataCaps()
{
	return m_numOfIceDataCaps;
}
int CSipCaps::GetNumOfIceGeneralCaps()
{
	return m_numOfIceGeneralCaps;
}
capBuffer* CSipCaps::GetIceAudioCapList(WORD index)
{
	return m_IceAudioCapList[index];
}
capBuffer* CSipCaps::GetIceVideoCapList(WORD index)
{
	return m_IceVideoCapList[index];
}
capBuffer* CSipCaps::GetIceDataCapList(WORD index)
{
	return m_IceDataCapList[index];
}
capBuffer* CSipCaps::GetIceGeneralCapList(WORD index)
{
	return m_IceGeneralCapList[index];
}

///////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CSipCaps::AddFecCap()
{
	CFecCap* pFecCap = (CFecCap*)CBaseCap::AllocNewCap(eFECCapCode, NULL);

	if(pFecCap)
	{
		EResult eResOfSet = kSuccess;
		eResOfSet &= pFecCap->SetStruct(cmCapVideo, cmCapReceive, kRolePeople); //(cmCapVideo, cmCapReceiveAndTransmit); //SetStruct(eType, eDirection, eRole)

		if (eResOfSet)
		{
			capBuffer* pCapBuffer = pFecCap->GetAsCapBuffer();
			CCapSetInfo capsetinfofec(eFECCapCode);

			if (pCapBuffer)
			{
				PTRACE(eLevelInfoNormal,"CSipCaps::AddFecCap - LYNC2013_FEC_RED: Add FEC");
				pCapBuffer->sipPayloadType = ::GetPayloadType(capsetinfofec);
				((ctCapStruct*)pCapBuffer->dataCap)->roleLabel = kRolePeople;
				AddCapSet(cmCapVideo , pCapBuffer);
				m_bIsFec = TRUE;
			}
			else
				PTRACE(eLevelError,"CSipCaps::AddFecCap: LYNC2013_FEC_RED: Create cap buffer has failed");

			PDELETEA(pCapBuffer);
		}
		else
			PTRACE(eLevelInfoNormal,"CSipCaps::AddFecCap: LYNC2013_FEC_RED: Set struct has failed!!!");
		pFecCap->FreeStruct();
		POBJDELETE(pFecCap);
	}
	else
		DBGPASSERT(1);

}

///////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CSipCaps::AddRedCap()
{
	CRedCap* pRedCap = (CRedCap*)CBaseCap::AllocNewCap(eREDCapCode, NULL);

	if(pRedCap)
	{
		EResult eResOfSet = kSuccess;
		eResOfSet &= pRedCap->SetStruct(cmCapReceive,0,0);

		if (eResOfSet)
		{
			capBuffer* pCapBuffer = pRedCap->GetAsCapBuffer();
			CCapSetInfo capsetinfored(eREDCapCode);

			if (pCapBuffer)
			{
				PTRACE(eLevelInfoNormal,"CSipCaps::AddRedCap - LYNC2013_FEC_RED: Add RED");
				pCapBuffer->sipPayloadType = ::GetPayloadType(capsetinfored);
				((ctCapStruct*)pCapBuffer->dataCap)->roleLabel = kRolePeople;
				AddCapSet(cmCapAudio , pCapBuffer);
				m_bIsRed = TRUE;
			}
			else
				PTRACE(eLevelError,"CSipCaps::AddRedCap: LYNC2013_FEC_RED: Create cap buffer has failed");

			PDELETEA(pCapBuffer);

		}
		else
			PTRACE(eLevelInfoNormal,"CSipCaps::AddRedCap: LYNC2013_FEC_RED: Set struct has failed!!!");
		pRedCap->FreeStruct();
		POBJDELETE(pRedCap);
	}
	else
		DBGPASSERT(1);

}
///////////////////////////////////////////////////////////////////////////
void CSipCaps::AddLprCap(ERoleLabel eRole, eConfMediaType aConfMediaType, bool aIsMrcCall)
{
	BOOL isSipLprEnabled = TRUE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("ENABLE_SIP_LPR", isSipLprEnabled);
	if(isSipLprEnabled == FALSE)
	{
		PTRACE(eLevelInfoNormal,"CSipCaps::AddLprCap - LPR is disabled");
		return;
	}

	//CLprCap
	CBaseVideoCap* pLprCap = (CBaseVideoCap*)CBaseCap::AllocNewCap(eLPRCapCode, NULL);

	if(pLprCap)
	{
		EResult eResOfSet = kSuccess;
		eResOfSet = ((CLprCap*)pLprCap)->SetDefaultsLpr(cmCapReceive, kRolePeople, cmCapVideo);
		((CLprCap*)pLprCap)->SetLprVersionID(2);
		if (eResOfSet)
		{
			//int structSize = pLprCap->SizeOf();
			capBuffer* pCapBuffer = pLprCap->GetAsCapBuffer();
			CCapSetInfo capsetinfolpr(eLPRCapCode);

			if (pCapBuffer)
			{
				PTRACE2INT(eLevelInfoNormal,"CSipCaps::AddLprCap - Add LPR. Role=", eRole);
				pCapBuffer->sipPayloadType = ::GetPayloadType(capsetinfolpr);
				((ctCapStruct*)pCapBuffer->dataCap)->roleLabel = eRole;
				AddCapSet(cmCapVideo , pCapBuffer);
				if (eRole == kRolePeople ||
				    ((aConfMediaType==eMixAvcSvc) && aIsMrcCall && eRole == kRolePresentation))
					m_bIsLpr = TRUE;
			}
			else
				PTRACE(eLevelError,"CSipCaps::AddLprCap: Create cap buffer has failed");

			PDELETEA(pCapBuffer);

		}
		else
			PTRACE(eLevelInfoNormal,"CSipCaps::AddLprCap: Set struct has failed!!!");
		pLprCap->FreeStruct();
		POBJDELETE(pLprCap);
	}
	else
		DBGPASSERT(1);

}
///////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
/*CFecCap* CSipCaps::GetFecCap()
{
	for(int i = 0; i < m_numOfVideoCapSets; i++)
	{
		if(m_videoCapList[i]->capTypeCode == eFECCapCode)
		{
			PTRACE(eLevelInfoNormal,"CSipCaps::GetFecCap -found cap");
			CFecCap* pCap = NULL;
			pCap = (CFecCap*)CBaseCap::AllocNewCap((CapEnum)m_videoCapList[i]->capTypeCode,m_videoCapList[i]->dataCap);
			return pCap;
		}
	}
	return NULL;

}*/
///////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
/*CRedCap* CSipCaps::GetRedCap()
{
	for(int i = 0; i < m_numOfAudioCapSets; i++)
	{
		if(m_audioCapList[i]->capTypeCode == eREDCapCode)
		{
			PTRACE(eLevelInfoNormal,"CSipCaps::GetRedCap -found cap");
			CRedCap* pCap = NULL;
			pCap = (CRedCap*)CBaseCap::AllocNewCap((CapEnum)m_audioCapList[i]->capTypeCode,m_audioCapList[i]->dataCap);
			return pCap;
		}
	}
	return NULL;

}*/
///////////////////////////////////////////////////////////////////////////
CLprCap* CSipCaps::GetLprCap()
{
	for(int i = 0; i < m_numOfVideoCapSets; i++)
	{
		if(m_videoCapList[i]->capTypeCode == eLPRCapCode)
		{
			PTRACE(eLevelInfoNormal,"CSipCaps::GetLprCap -found cap");
			CLprCap* pCap = NULL;
			pCap = (CLprCap*)CBaseCap::AllocNewCap((CapEnum)m_videoCapList[i]->capTypeCode,m_videoCapList[i]->dataCap);
			return pCap;
		}
	}
	return NULL;

}

///////////////////////////////////////////////////////////////////////////
CLprCap* CSipCaps::GetContentLprCap()
{
	for(int i = 0; i < m_numOfContentCapSets; i++)
	{
		if(m_contentCapList[i]->capTypeCode == eLPRCapCode)
		{
			PTRACE(eLevelInfoNormal,"CSipCaps::GetContentLprCap -found cap");
			CLprCap* pCap = NULL;
			pCap = (CLprCap*)CBaseCap::AllocNewCap((CapEnum)m_contentCapList[i]->capTypeCode,m_contentCapList[i]->dataCap);
			return pCap;
		}
	}
	return NULL;

}

///////////////////////////////////////////////////////////////////////////
void CSipCaps::GetRtvCap(RTVVideoModeDetails& rtvVidModeDetails,DWORD& BitRate) const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

	for(int i = 0; i < numOfMediaCapSet; i++)
	{
		if ((CapEnum)pMediaCapList[i]->capTypeCode == eRtvCapCode)
		{
			CRtvVideoCap* pRtvVideoCap = (CRtvVideoCap *)CBaseCap::AllocNewCap(eRtvCapCode, pMediaCapList[i]->dataCap);
			if (pRtvVideoCap)
			{
			    pRtvVideoCap->GetRtvCap(rtvVidModeDetails,BitRate);
			    PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetRtvCap -found cap, videoModeType=", rtvVidModeDetails.videoModeType);
				POBJDELETE(pRtvVideoCap);
			}
			else
			    PTRACE(eLevelInfoNormal,"CSipCaps::GetRtvCap -pRtvVideoCap is NULL");
		}
	}
}
EResult CSipCaps::GetRtvCapBitRateAccordingToResolution(DWORD Width,DWORD Height, DWORD& retBitRate) const
{
	EResult eRes = kFailure;

	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

	for(int i = 0; i < numOfMediaCapSet; i++)
	{
		if ((CapEnum)pMediaCapList[i]->capTypeCode == eRtvCapCode)
		{
			CRtvVideoCap* pRtvVideoCap = (CRtvVideoCap *)CBaseCap::AllocNewCap(eRtvCapCode, pMediaCapList[i]->dataCap);
			if (pRtvVideoCap)
			{
				PTRACE(eLevelInfoNormal,"CSipCaps::GetRtvCapAccordingToResolution -found cap");
				eRes = pRtvVideoCap->GetBitRateAccordingToResolution(Width, Height, retBitRate);

				POBJDELETE(pRtvVideoCap);
			}
			else
				PTRACE(eLevelInfoNormal,"CSipCaps::GetRtvCapAccordingToResolution -pRtvVideoCap is NULL");
		}
	}
	return eRes;
}
///////////////////////////////////////////////////////////////////////////
void CSipCaps::GetRtvCapFRAccordingToFS(DWORD& FrameRate,DWORD FS)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);

	for(int i = 0; i < numOfMediaCapSet; i++)
	{
		if ((CapEnum)pMediaCapList[i]->capTypeCode == eRtvCapCode)
		{
			CRtvVideoCap* pRtvVideoCap = (CRtvVideoCap *)CBaseCap::AllocNewCap(eRtvCapCode, pMediaCapList[i]->dataCap);
			if (pRtvVideoCap)
			{
			    PTRACE(eLevelInfoNormal,"CSipCaps::GetRtvCap -found cap");
			    pRtvVideoCap->GetRtvCapFRAccordingToFS(FrameRate,FS);
				POBJDELETE(pRtvVideoCap);
			}
			else
			    PTRACE(eLevelInfoNormal,"CSipCaps::GetRtvCap -pRtvVideoCap is NULL");
		}
	}
}
///////////////////////////////////////////////////////////////////////////
void CSipCaps::FindBestVidTxModeForCop(CCopVideoTxModes* pCopVideoTxModes, CSipComMode* pScm, BYTE definedProtocol, DWORD definedRate) const
{
    if (!pCopVideoTxModes)
	{
		PTRACE(eLevelInfoNormal,"CSipCaps::FindBestVidTxModeForCop pCopVideoTxModes is NULL");
		return;
	}

	pCopVideoTxModes->Dump("CSipCaps::FindBestVidTxModeForCop",eLevelInfoNormal);
	DWORD valuesToCompare = kCapCode|kBitRate|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS| kH264Additional_FS|kRoleLabel | kH264Profile | kMaxFR | kH264Mode | kPacketizationMode;
	CVidModeH323* pVidMode;
	int modeIndex = 0;
	for (modeIndex=0;modeIndex<NUMBER_OF_COP_LEVELS;modeIndex++)
	{
		pVidMode = pCopVideoTxModes->GetVideoMode(modeIndex);
		if (pVidMode && pVidMode->IsMediaOn() && pCopVideoTxModes->IsValidForDefinedParams(modeIndex, definedProtocol, definedRate))
		{
            CBaseCap* pCapClass = pVidMode->GetAsCapClass();
            DWORD details = 0;
            int  arrIndex = 0;
            if (pCapClass && IsContainingCapSet(cmCapReceive, *pCapClass, valuesToCompare, &details, &arrIndex))
			{
				COstrStream strBase;
				strBase << "Caps contain cop level index: " << modeIndex << "\n";
				pVidMode->Dump(strBase);
				PTRACE2(eLevelInfoNormal,"CSipCaps::FindBestVidTxModeForCop: \n", strBase.str().c_str());
				CSdesCap* pSdesCap = pScm->GetMediaMode(cmCapVideo, cmCapTransmit, kRolePeople).GetSdesCap();
				pScm->SetMediaMode(*pVidMode, cmCapVideo, cmCapTransmit, kRolePeople);

				CBaseVideoCap* pScmCap = (CBaseVideoCap*)pScm->GetMediaAsCapClass(cmCapVideo, cmCapTransmit, kRolePeople);
                CBaseVideoCap* pIntersect = NULL;
                if (pScmCap)
                {
                    pIntersect = pScmCap->CreateIntersectBetweenTwoVidCaps((CBaseVideoCap*)pCapClass, cmCapTransmit, TRUE);
                    POBJDELETE(pScmCap);
                }
                else
                    PTRACE(eLevelError,"CSipCaps::FindBestVidTxModeForCop - pScmCap is NULL");
				if (pIntersect)
				{

					pScm->SetMediaMode(pIntersect->GetCapCode(), pIntersect->SizeOf(), (BYTE*)pIntersect->GetStruct(), cmCapVideo, cmCapTransmit, kRolePeople);
					pScm->SetCopTxLevel(modeIndex);
					if(pScm->GetIsEncrypted() == Encryp_On)
					{
						PTRACE(eLevelInfoNormal,"CSipCaps::FindBestVidTxModeForCop -ADD SDES ON ENCRYPTED PARTY");
						(pScm->GetMediaMode(cmCapVideo, cmCapTransmit, kRolePeople)).SetSdesCap(pSdesCap);

						//pScm->CreateLocalSipComModeSdesForSpecficMedia(cmCapVideo, cmCapTransmit, kRolePeople);
					}
					pIntersect->FreeStruct();
					POBJDELETE(pIntersect);
					POBJDELETE(pCapClass);
					return;
				}
			}
            POBJDELETE(pCapClass);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::SetSingleVideoProtocolIfNeeded (BYTE protocol)
{
    CCapSetInfo capInfo261 (eH261CapCode);
    CCapSetInfo capInfo263 (eH263CapCode);
    CCapSetInfo capInfo264 (eH264CapCode);

    switch (protocol)
    {
		case(VIDEO_PROTOCOL_H261):
        {
			PTRACE (eLevelInfoNormal, "CSipCaps::SetSingleVideoProtocolIfNeeded - H261");
            RemoveCapSet (capInfo263);
            RemoveCapSet (capInfo264);
            Set4CifMpi(-1);
            break;
        }
        case(VIDEO_PROTOCOL_H263):
        {
        	PTRACE (eLevelInfoNormal, "CSipCaps::SetSingleVideoProtocolIfNeeded - H263");
            RemoveCapSet (capInfo264);
            RemoveCapSet (capInfo261);

            break;
        }
        case(VIDEO_PROTOCOL_H264):
        {
        	PTRACE (eLevelInfoNormal, "CSipCaps::SetSingleVideoProtocolIfNeeded - H264");
            RemoveCapSet (capInfo263);
            RemoveCapSet (capInfo261);
            Set4CifMpi(-1);
            break;
        }
        case AUTO:
			//do nothing!
			break;
    }
}

/////////////////////////////////////////////////////////////////////////
void CSipCaps::SetFormatsMpi(CapEnum protocol, ERoleLabel eRole, int qcifMpi, int cifMpi, int cif4Mpi, int cif16Mpi)
{
	int numOfVideoMediaCapSet = 0;

	if (IsMedia(cmCapVideo))
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
		for (int i = 0; (i < numOfVideoMediaCapSet); i++)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*)GetCapSet(cmCapVideo, i);
			if (pVideoCap && pVideoCap->GetCapCode()==protocol)
			{
				pVideoCap->SetFormatMpi(kQCif, qcifMpi);
				pVideoCap->SetFormatMpi(kCif, cifMpi);
				pVideoCap->SetFormatMpi(k4Cif, cif4Mpi);
				pVideoCap->SetFormatMpi(k16Cif, cif16Mpi);
			}
			POBJDELETE(pVideoCap);
		}
	}
}
/////////////////////////////////////////////////////////////////////////
void CSipCaps::SetVideoCapsExactlyAccordingToScm(const CIpComMode* pScm)
{
	CapEnum capCode = (CapEnum)(pScm->GetMediaType(cmCapVideo, cmCapReceive));
	BYTE protocol = ::ConvertCapEnumToReservationProtocol(capCode);
    DWORD rate = pScm->GetVideoBitRate(cmCapReceive, kRolePeople);
    DWORD contentRate = pScm->GetContentBitRate(cmCapReceive);
    if(contentRate > 0 )
    {
    	rate = rate + contentRate;
    	PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetVideoCapsExactlyAccordingToScm -active content this is new rate ",rate);
    }
    SetVideoRateInallCaps (rate, kRolePeople);

	/* In Sip we don't call SetSingleVideoProtocolIfNeeded as in H323.
	 * In Sip the local caps contain all the caps, but when sending the ReInvite or 200ok for reinvite, we will not add the local video caps, just the scm video.
	 */

	if ((capCode == eH261CapCode) || (capCode == eH263CapCode))
	{
		int qcifMpi = -1, cifMpi = -1, cif4Mpi = -1, cif16Mpi = -1;
		qcifMpi = pScm->GetFormatMpi(kQCif, cmCapReceive, kRolePeople);
		cifMpi = pScm->GetFormatMpi(kCif, cmCapReceive, kRolePeople);
		cif4Mpi = pScm->GetFormatMpi(k4Cif, cmCapReceive, kRolePeople);
		cif16Mpi = pScm->GetFormatMpi(k16Cif, cmCapReceive, kRolePeople);
		SetFormatsMpi(capCode, kRolePeople, qcifMpi, cifMpi, cif4Mpi, cif16Mpi);
	}
	else if (IsH264Video(capCode))
	{
		APIU16 profile = 0;
		APIU8 level=0;
		long mbps=0, fs=0, dpb=0, brAndCpb=0, sar=0, staticMB=0;
		pScm->GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive);
		profile = H264_Profile_None; // save profile as is
		SetLevelAndAdditionals(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, kRolePeople);
	}

}
//////////////////////////////////////////////////////////////////////
void CSipCaps::Reomve4cifFromCaps()
{
	for (int i = 0; i < m_numOfVideoCapSets; i++)
	{
		CCapSetInfo capInfo = (CapEnum)m_videoCapList[i]->capTypeCode;
		CBaseVideoCap* pVideoCap = (CBaseVideoCap *) CBaseCap::AllocNewCap(capInfo, m_videoCapList[i]->dataCap);
		if (pVideoCap && pVideoCap->GetCapCode() == eH263CapCode)
		{
			pVideoCap->SetFormatMpi(k4Cif,-1);
		}
		POBJDELETE(pVideoCap);
	}
}
/////////////////////////////////////////////////////
void CSipCaps::SetMpiFormatInCaps(EFormat format,BYTE val)
{
	for (int i = 0; i < m_numOfVideoCapSets; i++)
	{
		CCapSetInfo capInfo = (CapEnum)m_videoCapList[i]->capTypeCode;
		CBaseVideoCap* pVideoCap = (CBaseVideoCap *) CBaseCap::AllocNewCap(capInfo, m_videoCapList[i]->dataCap);
		if (pVideoCap && pVideoCap->GetCapCode() == eH263CapCode && pVideoCap->GetRole() == kRolePeople)
		{
			pVideoCap->SetFormatMpi(format,val);
		}
		POBJDELETE(pVideoCap);
	}

}

//////////////////////////////////////////////////////////////////////
void CSipCaps::CreatePartialAudioCaps()
{
	capBuffer* pRemoved = NULL;
	if (m_numOfAudioCapSets <= g_SipMaxPartialAudioCodecs)
	{
		//nothing to do here -> use the given caps set
		return;
	}

	int maxNumOfStaticCaps = 0;
	int maxNumOfDynamicCaps = 0;
	int i = 0;
	//identify how many codecs are in the "static" list
	for (i = 0; i < m_numOfAudioCapSets; i++)
	{
		if(IsAudioCodecInParitalList((CapEnum)m_audioCapList[i]->capTypeCode, FALSE))
		{
			maxNumOfStaticCaps++;
		}
	}
	maxNumOfDynamicCaps = g_SipMaxPartialAudioCodecs - maxNumOfStaticCaps;

	//remove all redundant dynamic caps
	int numOfDynamicCaps = 0;
	for (i = 0;
		 i < m_numOfAudioCapSets && m_audioCapList[i] != NULL;
		 i++)
	{
		//if it's static cap - do nothing
		if(IsAudioCodecInParitalList((CapEnum)m_audioCapList[i]->capTypeCode, FALSE))
		{
			continue;
		}

		//if we pass the dynamic limit -> remove the cap
		if (numOfDynamicCaps == maxNumOfDynamicCaps)
		{
			pRemoved = RemoveCapSet(cmCapAudio, i);
			PDELETEA(pRemoved);
			i--; //return the index as the remove shifted the list
		}
		else
		{
			numOfDynamicCaps++;
		}
	}

	PTRACE2INT(eLevelInfoNormal,"CSipCaps::CreatePartialAudioCaps m_numOfAudioCapSets=", m_numOfAudioCapSets);
	PTRACE2INT(eLevelInfoNormal,"CSipCaps::CreatePartialAudioCaps numOfDynamicCaps=", numOfDynamicCaps);
	PTRACE2INT(eLevelInfoNormal,"CSipCaps::CreatePartialAudioCaps maxNumOfStaticCaps=", maxNumOfStaticCaps);

}
/////////////////////////////////////////////////////////////////////////
void CSipCaps::CreatePartialAudioCapsForAudioOnly() //BRIDGE-11697
{
	capBuffer* pRemoved = NULL;
	int NumAudioOnlyCaps = 0;

	for (int i = 0; i < m_numOfAudioCapSets && m_audioCapList[i] != NULL; ++i)
	{

		if(IsAudioCodecInParitalList((CapEnum)m_audioCapList[i]->capTypeCode, TRUE))
		{
			++NumAudioOnlyCaps;
			continue;
		}
		else
		{
			pRemoved = RemoveCapSet(cmCapAudio, i);
			PDELETEA(pRemoved);
			i--; //return the index as the remove shifted the list
		}
	}
	PTRACE2INT(eLevelInfoNormal,"CSipCaps::CreatePartialAudioCapsForAudioOnly NumAudioOnlyCaps=", NumAudioOnlyCaps);

}

//////////////////////////////////////////////////////////////////////
BOOL CSipCaps::IsAudioCodecInParitalList(CapEnum dataType, bool audioOnly)
{
	PASSERTMSG_AND_RETURN_VALUE((!CProcessBase::GetProcess() || !CProcessBase::GetProcess()->GetSysConfig()), "CSipCaps::IsAudioCodecInParitalList - NULLs found in CProcessBase::GetProcess()->GetSysConfig()", FALSE);
	int i;
	BOOL foundCodec = FALSE;

	// Siren7 is dependent on configuration and not static
	//======================================================
	if (eSiren7_16kCapCode == dataType && IsFeatureSupportedBySystem(eFeatureSiren7))
	{
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ALLOW_SIREN7_CODEC, foundCodec);
	}

	// Choosing list to search
	//==========================
	CapEnum* partialList;
	if (audioOnly)
	{
		partialList = g_SipAudioOnlyPartialAudioCodecs;
	}
	else
	{
		partialList = g_SipStaticPartialAudioCodecs;
	}

	// Checking codec
	//=================
	for(i = 0; !foundCodec && partialList[i] != eUnknownAlgorithemCapCode; ++i)
	{
		if (dataType == partialList[i])
		{
			foundCodec = TRUE;
		}
	}

	return foundCodec;
}

/////////////////////////////////////////////////////////////////////////
void CSipCaps::SetConfUserIdForBfcp(WORD confid, WORD userid)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
				    pBfcpCap->SetConfId(confid);
				    pBfcpCap->SetUserId(userid);
				    POBJDELETE(pBfcpCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::SetConfUserIdForBfcp - pBfcpCap is NULL");
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////
void CSipCaps::SetFloorIdParamsForBfcp(char* pFloorid, char* pStreamid)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
				    pBfcpCap->SetFloorIdParams(0, pFloorid, pStreamid, NULL, NULL, NULL);
				    POBJDELETE(pBfcpCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::SetFloorIdParamsForBfcp - pBfcpCap is NULL");
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsBfcpSupported() const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
				return TRUE;
		}
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////
void CSipCaps::SetBfcpParameters(eBfcpSetup setup, eBfcpConnection connection, eBfcpFloorCtrl floorCtrl, eBfcpMStreamType mstreamType)
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
				    pBfcpCap->SetConnection(connection);
				    pBfcpCap->SetSetup(setup);
				    pBfcpCap->SetFloorCntl(floorCtrl);
				    pBfcpCap->SetMStreamType(mstreamType);
				    POBJDELETE(pBfcpCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::SetBfcpParameters - pBfcpCap is NULL");
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////
CapEnum CSipCaps::GetBestContentProtocol() const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	CapEnum contentProtocol = eUnknownAlgorithemCapCode;

	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList, kRolePresentation);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eH264CapCode)
				contentProtocol = eH264CapCode;
			else if (((CapEnum)pMediaCapList[i]->capTypeCode == eH263CapCode) && (contentProtocol != eH264CapCode))
				contentProtocol = eH263CapCode;
		}
	}
	return contentProtocol;
}

/////////////////////////////////////////////////////////////////////////
eBfcpSetup CSipCaps::GetBfcpSetupAttribute() const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	eBfcpSetup setup = bfcp_setup_null;
	BOOL isFound = FALSE;

	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet && !isFound; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
				    setup = (eBfcpSetup)pBfcpCap->GetSetup();
				    isFound = TRUE;
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::GetBfcpSetupAttribute - pBfcpCap is NULL");
				/*free pBfcpCap*/
				POBJDELETE(pBfcpCap);
			}
		}
	}
	return setup;
}
/////////////////////////////////////////////////////////////////////////
eBfcpConnection CSipCaps::GetBfcpConnectionAttribute() const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	eBfcpConnection connection = bfcp_connection_null;
	BOOL isFound = FALSE;

	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet && !isFound; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
				    connection = (eBfcpConnection)pBfcpCap->GetConnection();
				    isFound = TRUE;
					/*Need to free the pBfcpCap here*/
					POBJDELETE(pBfcpCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::GetBfcpConnectionAttribute - pBfcpCap is NULL");
			}
		}
	}

	if (!isFound)
	{
		connection = bfcp_connection_new;
	}
	return connection;
}

/////////////////////////////////////////////////////////////////////////
eBfcpMStreamType CSipCaps::GetBfcpMStreamType() const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	eBfcpMStreamType mstremType = bfcp_m_stream_None;
	BOOL isFound = FALSE;

	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet && !isFound; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
				    mstremType = pBfcpCap->GetMStreamType();
				    isFound = TRUE;

					POBJDELETE(pBfcpCap);
				}
			}
		}
	}
	return mstremType;
}
/////////////////////////////////////////////////////////////////////////
eBfcpFloorCtrl CSipCaps::GetBfcpFloorCtrlAttribute() const
{
	int numOfMediaCapSet = 0;
	capBuffer** pMediaCapList = NULL;
	eBfcpFloorCtrl floorCtrl = bfcp_flctrl_null;
	BOOL isFound = FALSE;

	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet && !isFound; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
				    floorCtrl = pBfcpCap->GetFloorCntl();
				    isFound = TRUE;
					POBJDELETE(pBfcpCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::GetBfcpFloorCtrlAttribute - pBfcpCap is NULL");
			}
		}
	}
	return floorCtrl;
}

/////////////////////////////////////////////////////////////////////////
void CSipCaps::SetBfcpTransportType(enTransportType transType)
{
	int numOfMediaCapSet = 0;
//	int i;
	capBuffer** pMediaCapList = NULL;
	BOOL isFound = FALSE;

	PTRACE2INT(eLevelInfoNormal,"CSipCaps::SetBfcpTransportType - transport Type is:", transType);

	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet && !isFound; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
					pBfcpCap->SetTransportType(transType);

				    isFound = TRUE;
					POBJDELETE(pBfcpCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::SetBfcpTransportType - pBfcpCap is NULL");
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////
enTransportType CSipCaps::GetBfcpTransportType() const
{
	int numOfMediaCapSet = 0;

	capBuffer** pMediaCapList = NULL;
	enTransportType transType = eUnknownTransportType;
	BOOL isFound = FALSE;

	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet && !isFound; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
					transType = pBfcpCap->GetTransportType();
					PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetBfcpTransportType - transport Type is:", transType);
				    isFound = TRUE;
					POBJDELETE(pBfcpCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::GetBfcpTransportType - pBfcpCap is NULL");
			}
		}
	}
	return transType;
}
/////////////////////////////////////////////////////////////////////////
WORD CSipCaps::GetBfcpUserId() const
{
	int numOfMediaCapSet = 0;

	capBuffer** pMediaCapList = NULL;

	WORD userId = 0;
	BOOL isFound = FALSE;

	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet && !isFound; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
					userId = AtoiBounded(pBfcpCap->GetUserId());
					PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetBfcpUserId - userId is:", userId);
				    isFound = TRUE;
					POBJDELETE(pBfcpCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::GetBfcpUserId - pBfcpCap is NULL");
			}
		}
	}
	return userId;
}
/////////////////////////////////////////////////////////////////////////
WORD CSipCaps::GetBfcpConfId() const
{
	int numOfMediaCapSet = 0;

	capBuffer** pMediaCapList = NULL;

	WORD confId = 0;
	BOOL isFound = FALSE;

	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet && !isFound; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
					confId = AtoiBounded(pBfcpCap->GetConfId());
					PTRACE2INT(eLevelInfoNormal,"CSipCaps::GetBfcpConfId - confId is:", confId);
				    isFound = TRUE;
					POBJDELETE(pBfcpCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::GetBfcpConfId - pBfcpCap is NULL");
			}
		}
	}
	return confId;
}
/////////////////////////////////////////////////////////////////////////
void CSipCaps::GetBfcpFloorIdParams(int floorIndex, char* pFloorId, char* pStreamId0, char* pStreamId1, char* pStreamId2, char* pStreamId3) const
{
	int numOfMediaCapSet = 0;

	capBuffer** pMediaCapList = NULL;

	BOOL isFound = FALSE;

	GetMediaCaps(cmCapBfcp, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet && !isFound; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == eBFCPCapCode)
			{
				CBfcpCap* pBfcpCap = (CBfcpCap *)CBaseCap::AllocNewCap(eBFCPCapCode, pMediaCapList[i]->dataCap);
				if (pBfcpCap)
				{
					pBfcpCap->GetFloorIdParams(floorIndex, pFloorId, pStreamId0, pStreamId1, pStreamId2, pStreamId3);
					isFound = TRUE;
					POBJDELETE(pBfcpCap);
				}
				else
					PTRACE(eLevelInfoNormal,"CSipCaps::GetBfcpConfId - pBfcpCap is NULL");
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::GetIsContainingCapCode(cmCapDataType eMediaType, CapEnum capCode) const
{
	CapEnum reVal				= eUnknownAlgorithemCapCode;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList);

	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			if ((CapEnum)pMediaCapList[i]->capTypeCode == capCode)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////
APIU16 CSipCaps::GetH264ProfileFromCapCode(cmCapDataType eMediaType) const
{
	CapEnum reVal				= eUnknownAlgorithemCapCode;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList);
	APIU16 profile = 0;
	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			reVal = (CapEnum)pMediaCapList[i]->capTypeCode;
			if(reVal == eH264CapCode)
			{
				CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);
				if (pH264VideoCap)
				{
				    profile = pH264VideoCap->GetProfile();
				    POBJDELETE(pH264VideoCap);
				    return profile;
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::GetH264ProfileFromCapCode - pH264VideoCap is NULL");
			}
		}
	}
	return profile;
}
/////////////////////////////////////////////////////////////////////////
long CSipCaps::GetVideoRate(void) const
{
	long dVideoRate = -1; //as undefined

	for (int i = 0; i < m_numOfVideoCapSets; i++)
	{
		CCapSetInfo capInfo = (CapEnum)m_videoCapList[i]->capTypeCode;
		CBaseVideoCap* pVideoCap = (CBaseVideoCap *) CBaseCap::AllocNewCap(capInfo, m_videoCapList[i]->dataCap);
		if (pVideoCap)
		{
			dVideoRate = max((long) pVideoCap->GetBitRate(), dVideoRate);
		}
		POBJDELETE(pVideoCap);
	}

	ALLOCBUFFER(cLog, 256);
	memset(cLog, 0, 256);
	sprintf(cLog, "Video Rate is [%ld] in [%d] sets", dVideoRate, m_numOfVideoCapSets);
	FPTRACE2(eLevelInfoNormal, "CSipCaps::GetVideoRate ", cLog);
	DEALLOCBUFFER(cLog);
	return (dVideoRate); // Fecc rate should not be calculate.
}
/////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::GetIsTipResolution() const
{
	if (IsCapableOfHD720() || IsCapableOfHD1080())
		return TRUE;

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsCapableTipAux5Fps() const
{
	BYTE bRes = FALSE;
	ETipAuxFPS auxFps = GetTipAuxFPS();
	if ((auxFps == eTipAux30FPS) || (auxFps == eTipAux5FPS))
		bRes = TRUE;
//	PTRACE2INT(eLevelInfoNormal,"CSipCaps::IsCapableTipAux5Fps : ",bRes);
	return bRes;
}

/////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::IsTipCompatibleContentSupported() const
{
	BYTE bRes = FALSE;

	APIS32 is264tipContent = 1;

	for (int i = 0; ((i < m_numOfContentCapSets) && (is264tipContent == 1)); i++)
	{
		CCapSetInfo capInfo = (CapEnum)m_contentCapList[i]->capTypeCode;
		CBaseVideoCap* pContentCap = (CBaseVideoCap *) CBaseCap::AllocNewCap(capInfo, m_contentCapList[i]->dataCap);
		if (pContentCap)
		{
			if (pContentCap->GetCapCode() == eH264CapCode)
			{
				is264tipContent = ((CH264VideoCap *)pContentCap)->GetH264mode();
			}
		}
		POBJDELETE(pContentCap);
	}

	if (is264tipContent == H264_tipContent) {
		bRes = TRUE;
	}
	PTRACE2INT(eLevelInfoNormal,"CSipCaps::IsTipCompatibleContentSupported ", bRes);
	return bRes;
}
//////////////////////////////////////////////////////////////////////
void CSipCaps::FixUnkownProfileInCapsIfNeeded()
{
	CapEnum reVal				= eUnknownAlgorithemCapCode;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList);
	if (pMediaCapList)
	{
		for (int i = 0; i < numOfMediaCapSet; i++)
		{
			reVal = (CapEnum)pMediaCapList[i]->capTypeCode;
			if(reVal == eH264CapCode)
			{
				CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, pMediaCapList[i]->dataCap);

				if (pH264VideoCap)
				{
				    APIU8 profile = pH264VideoCap->GetProfile();
				    //255 = unknown profile
				    if (255 == profile)
				    {
				        pH264VideoCap->SetProfile(64);
				        PTRACE(eLevelInfoNormal, "CSipCaps::FixUnkownProfileInCaps Changing unknown profile from 255 to 64");
				    }
				    APIU8 level = pH264VideoCap->GetLevel();
				    //255 = unknown level
				    if (255 == level)
				    {
				        pH264VideoCap->SetLevel(22);
				        PTRACE(eLevelInfoNormal, "CSipCaps::FixUnkownProfileInCaps Changing unknown level from 255 to 22");
				    }

				    POBJDELETE(pH264VideoCap);
				}
				else
				    PTRACE(eLevelInfoNormal,"CSipCaps::FixUnkownProfileInCaps - pH264VideoCap is NULL");
			}
		}
	}
}
/////////////////////////////////////////////////////
void CSipCaps::TipFixRemoteRateByTipNegotiation(WORD nTipStreams)
{
	DWORD dwCurrBitRate = GetVideoRate();

	PTRACE2INT(eLevelInfoNormal, "CSipCall::TipFixRemoteRateByTipNegotiation, curr remote video bit rate: ", dwCurrBitRate);

	DWORD dwNewBitRate = dwCurrBitRate;

	if (IsCapableTipAux5Fps())
	{
		dwNewBitRate = dwCurrBitRate - 5000;
		PTRACE2INT(eLevelInfoNormal, "CSipCall::TipFixRemoteRateByTipNegotiation, new remote video bit rate after reducing content: ", dwNewBitRate);
	}

	if (nTipStreams == 1 || nTipStreams == 3)
		dwNewBitRate = dwNewBitRate / nTipStreams;
	else
	{
		dwNewBitRate = dwCurrBitRate;
		PASSERTMSG(nTipStreams, "CSipCaps::TipFixRemoteRateByTipNegotiation - unexpected number of TIP streams");
	}

	PTRACE2INT(eLevelInfoNormal, "CSipCall::TipFixRemoteRateByTipNegotiation, final remote video bit rate: ", dwNewBitRate);

	SetVideoRateInallCaps(dwNewBitRate);
}


/////////////////////////////////////////////////////
void CSipCaps::RemoveAudioCapsAccordingToRtvFlag(bool is_single_core)
{
	std::string msClientAudioCodecStr;
    CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    if (is_single_core)
	{
        BOOL res = pSysConfig->GetDataByKey(CFG_FORCE_AUDIO_CODEC_FOR_MS_SINGLE_CORE, msClientAudioCodecStr);
    	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetDataByKey: " << CFG_FORCE_AUDIO_CODEC_FOR_MS_SINGLE_CORE);
	}
    else
    {
        BOOL res = pSysConfig->GetDataByKey(CFG_MS_CLIENT_AUDIO_CODEC, msClientAudioCodecStr);
    	PASSERTSTREAM_AND_RETURN(!res, "CSysConfig::GetDataByKey: " << CFG_MS_CLIENT_AUDIO_CODEC);
    }

	CapEnum flagCapInfo = eUnknownAlgorithemCapCode;
	flagCapInfo =  ("G711A" == msClientAudioCodecStr) ? eG711Alaw64kCapCode : flagCapInfo;
	flagCapInfo =  ("G711U" == msClientAudioCodecStr) ? eG711Ulaw64kCapCode : flagCapInfo;
	flagCapInfo =  ("G722" == msClientAudioCodecStr) ? eG722_64kCapCode : flagCapInfo;
	if (!is_single_core)
	{
		flagCapInfo =  ("G7231" == msClientAudioCodecStr) ? eG7231CapCode : flagCapInfo;
		flagCapInfo =  ("G7221_24" == msClientAudioCodecStr) ? eG7221_24kCapCode : flagCapInfo;
	}

	if (eUnknownAlgorithemCapCode == flagCapInfo)
	{
		PTRACE(eLevelInfoNormal, "CSipCaps::RemoveAudioCapsAccordingToRtvFlag, doing nothing");
		return;
	}

	bool found = false;

	//search that remote has the desired cap
	for (int i = 0;
		 i < m_numOfAudioCapSets && m_audioCapList[i] != NULL && !found;
		 i++)
	{
		if (flagCapInfo == (CapEnum)m_audioCapList[i]->capTypeCode)
		{
			found = true;
		}
	}

	if (!found)
	{
		PTRACE(eLevelInfoNormal, "CSipCaps::RemoveAudioCapsAccordingToRtvFlag, configured audio cap is not in remote caps!");
		return;
	}

	PTRACE2(eLevelInfoNormal, "CSipCaps::RemoveAudioCapsAccordingToRtvFlag, chosen audio cap is ", msClientAudioCodecStr.c_str());

	//now remove all other the audio caps
	for (int i = 0;
		 i < m_numOfAudioCapSets && m_audioCapList[i] != NULL;
		 i++)
	{
		if (flagCapInfo != (CapEnum)m_audioCapList[i]->capTypeCode
			&& eRfc2833DtmfCapCode != (CapEnum)m_audioCapList[i]->capTypeCode)
		{
			RemoveCapSet((CapEnum) m_audioCapList[i]->capTypeCode);
			i--; //return the index as the remove shifted the list
		}
	}

}

/////////////////////////////////////////////////////
BYTE CSipCaps::IsRemoteSdpContainsICE(const sipSdpAndHeadersSt* sdp)
{
	const sipMediaLinesEntrySt* pMediaLinesEntry 	= (const sipMediaLinesEntrySt*) sdp->capsAndHeaders;
	const sipMediaLineSt *pMediaLine 				= (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[0];

	const capBuffer *pCapBuffer						= (capBuffer*) &pMediaLine->caps[0];
	const BYTE		*pTemp 							= (const BYTE*) pCapBuffer;

	BYTE bFindIce = FALSE;

	for (unsigned int i = 0 ; i < pMediaLine->numberOfCaps; i++)
	{
		switch(pCapBuffer->capTypeCode)
		{
			case eIceUfragCapCode:
			{
				bFindIce = TRUE;
				break;

			}
			case eIcePwdCapCode:
			{
				bFindIce = TRUE;
				break;
			}
		}

		if (bFindIce == TRUE)
			break;

		pTemp += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)pTemp;
	}

	return bFindIce;
}

/////////////////////////////////////////////////////
// Amihay: MRM CODE

bool CSipCaps::IsH264Video(CapEnum capCode)
{
    if (capCode == (CapEnum)eH264CapCode || capCode == (CapEnum)eSvcCapCode)
    {
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::FindBestModeForSvc(cmCapDirection (&directionArr)[2], BYTE (&bResArr)[MAX_SIP_MEDIA_TYPES][2], CapEnum (&algFound)[MAX_SIP_MEDIA_TYPES],
		int &i, int &j, cmCapDirection eOppositeDirection, ERoleLabel eRole, cmCapDataType mediaType,
		const CSipComMode& rPreferredMode, const CBaseCap* pPreferredMedia, CSipComMode* pBestMode, BYTE bIsMrcSlave) const
{
	PTRACE(eLevelInfoNormal, "CSipCaps::FindBestModeForSvc");

	CBaseCap* pBestCap = NULL;
	capBuffer* pBestCapBuffer = NULL;
	bool intersectStreams = false;

	if (pPreferredMedia) // there is a possibility that we won't prefer a specific media (auto)
	{
		CapEnum eAlg = pPreferredMedia->GetCapCode();
		CCapSetInfo capInfo = eAlg;

		if (capInfo.IsSupporedCap())
		{
			pBestCap = GetHighestCommon(directionArr[j], *pPreferredMedia);

			if (pBestCap != NULL)
			{
				bResArr[i][j] = YES;
				pBestCapBuffer = pBestCap->GetAsCapBuffer();
				if(pBestCapBuffer)
				{
					// update the correct payload type in cap buffer
					// Transmit scm: payload type from remote caps , Receive scm: payload type from local scm
					if(eOppositeDirection == cmCapTransmit)
					{
//						TRACEINTO<<"$#@ before: eOppositeDirection == cmCapTransmit pBestCapBuffer->sipPayloadType:"<<(int)pBestCapBuffer->sipPayloadType;
						pBestCapBuffer->sipPayloadType = GetPayloadTypeByDynamicPreference(capInfo,pBestCap->GetProfile(),eRole);
//						TRACEINTO<<"$#@ after: eOppositeDirection == cmCapTransmit pBestCapBuffer->sipPayloadType:"<<(int)pBestCapBuffer->sipPayloadType;
					}
					else
					{
//						TRACEINTO<<"$#@ before: eOppositeDirection == cmCapReceive pBestCapBuffer->sipPayloadType:"<<(int)pBestCapBuffer->sipPayloadType;
						pBestCapBuffer->sipPayloadType = rPreferredMode.GetPayloadType(mediaType,eOppositeDirection,eRole);
//						TRACEINTO<<"$#@ after: eOppositeDirection == cmCapReceive pBestCapBuffer->sipPayloadType:"<<(int)pBestCapBuffer->sipPayloadType;
					}
				}
				else
					PASSERTMSG(1, "GetAsCapBuffer return NULL");

				pBestCap->FreeStruct();
				POBJDELETE(pBestCap);

				if (pPreferredMedia->IsCapContainsStreamsGroup())
				{
					intersectStreams = true;
				}
			}
		}
	}

	if (bResArr[i][j] && pBestCapBuffer)
	{
        pBestMode->SetMediaModeSvc(pBestCapBuffer, mediaType, eOppositeDirection, eRole);

        if (intersectStreams)
        {
        	// copy streams for this media from target mode to BestMode
        	std::list <StreamDesc> targetStreamsDescList = rPreferredMode.GetStreamsListForMediaMode(mediaType,eOppositeDirection,eRole);
            pBestMode->SetStreamsListForMediaMode(targetStreamsDescList,mediaType,eOppositeDirection,eRole);
         //??   pBestMode->SetStreamsGroup(*(rPreferredMode.GetStreamsGroup(mediaType,eOppositeDirection, eRole)), mediaType, eOppositeDirection, eRole);
           	// Intersect streams for this media and set the result in BestMode
           	pBestMode->IntersectStreams(this,mediaType,eOppositeDirection,eRole, bIsMrcSlave);
        }

		algFound[i] = (CapEnum)pBestCapBuffer->capTypeCode;

	}
/*
	if (bIsMrcSlave)
	{
		// in Slave, if its OperationPoints is not similar to Master's OperationPoints then the call is downgraded to AudioOnly
		// note: 'FindBestMode' is called from 'remote' context; so in case we are Slave, then
		//       'this' is Master (the remote) and 'rPreferredMode' is Slave (the local)
		bool bIsSimilarOpMasterSlave = IsSimilarOperationPoints(rPreferredMode);
		if (!bIsSimilarOpMasterSlave)
		{
			TRACEINTO << "Slave's OperationPoints mismaches Master's OperationPoints; setting BEST mode to audio only";

			pBestMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
			pBestMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			pBestMode->SetMediaOff(cmCapData, cmCapReceiveAndTransmit);
		}
	} // end if Slave
*/

	PDELETEA(pBestCapBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSipCaps::IsSimilarOperationPoints(const CSipComMode& rPreferredMode) const
{
	VIDEO_OPERATION_POINT_SET_S *pOtherOpStruct = rPreferredMode.GetOperationPoints(cmCapVideo, cmCapReceive, kRolePeople);
	if (NULL == pOtherOpStruct)
	{
		TRACEINTO << "pOtherOpStruct == NULL";
		return false;
	}

	bool bRet = false;

	for(int i=0; i<m_numOfVideoCapSets; i++)
	{
		CBaseVideoCap* pVideoCap = (CBaseVideoCap*) CBaseCap::AllocNewCap(m_videoCapList[i]->dataCap);
		if (pVideoCap)
		{
			if ( eSvcCapCode == pVideoCap->GetCapCode() )
			{
				VIDEO_OPERATION_POINT_SET_S* pOpStruct = ((CSvcVideoCap*)pVideoCap)->GetOperationPoints();
				DumpOperationPointsStructs(pOpStruct, pOtherOpStruct);

				bRet =  IsSimilarOperationPoints(pOpStruct, pOtherOpStruct);
				if (true == bRet)
				{
					POBJDELETE(pVideoCap);
					break;
				}
			} // end if SvcCapCode

			POBJDELETE(pVideoCap);
		} // end if (pVideoCap)
	}

	TRACEINTO << "OperationPoints are " << ( bRet ? "similar :-)" : "dissimilar :-(" );
	return bRet;
}


////////////////////////////////////////////////////////////////////////////////////////////////
bool CSipCaps::IsSimilarOperationPoints(const VIDEO_OPERATION_POINT_SET_S* pOpStruct, const VIDEO_OPERATION_POINT_SET_S *pOtherOpStruct) const
{
	if (!pOpStruct || !pOtherOpStruct)
	{
		TRACEINTO << "pOpStruct is " << (pOpStruct ? "valid" : "NULL") << ", pOtherOpStruct is " << (pOtherOpStruct ? "valid" : "NULL");
		return false;
	}

	else if ( pOpStruct->numberOfOperationPoints != pOtherOpStruct->numberOfOperationPoints)
	{
		TRACEINTO << "pOpStruct->numberOfOperationPoints: " << (int)(pOpStruct->numberOfOperationPoints)
				  << ", pOtherOpStruct->numberOfOperationPoints: " << (int)(pOtherOpStruct->numberOfOperationPoints);
		return false;
	}


	bool bRet = true;

	// prepare pOpStruct's operation_points string
	for (int i=0; i<pOpStruct->numberOfOperationPoints; i++)
	{
		if ( (pOpStruct->tVideoOperationPoints[i].layerId		!= pOtherOpStruct->tVideoOperationPoints[i].layerId)	||
		     (pOpStruct->tVideoOperationPoints[i].Tid			!= pOtherOpStruct->tVideoOperationPoints[i].Tid)		||
		     (pOpStruct->tVideoOperationPoints[i].Did			!= pOtherOpStruct->tVideoOperationPoints[i].Did)		||
		     (pOpStruct->tVideoOperationPoints[i].Qid			!= pOtherOpStruct->tVideoOperationPoints[i].Qid)		||
		     (pOpStruct->tVideoOperationPoints[i].Pid			!= pOtherOpStruct->tVideoOperationPoints[i].Pid)		||
		     (pOpStruct->tVideoOperationPoints[i].profile		!= pOtherOpStruct->tVideoOperationPoints[i].profile)	||
		     (pOpStruct->tVideoOperationPoints[i].level			!= pOtherOpStruct->tVideoOperationPoints[i].level)		||
		     (pOpStruct->tVideoOperationPoints[i].frameWidth	!= pOtherOpStruct->tVideoOperationPoints[i].frameWidth)	||
		     (pOpStruct->tVideoOperationPoints[i].frameHeight	!= pOtherOpStruct->tVideoOperationPoints[i].frameHeight)||
		     (pOpStruct->tVideoOperationPoints[i].frameRate		!= pOtherOpStruct->tVideoOperationPoints[i].frameRate)	||
		     (pOpStruct->tVideoOperationPoints[i].maxBitRate	!= pOtherOpStruct->tVideoOperationPoints[i].maxBitRate) )
		   {
			   bRet = false;
			   break;
		   }
	} // end loop over operationPoints

	TRACEINTO << "OperationPoints are " << ( bRet ? "similar :-)" : "dissimilar :-(" );
	return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::DumpOperationPointsStructs(const VIDEO_OPERATION_POINT_SET_S* pOpStruct, const VIDEO_OPERATION_POINT_SET_S *pOtherOpStruct) const
{
	CLargeString sOpStruct_op, sOtherOpStruct_op;

	if (!pOpStruct || !pOtherOpStruct)
	{
		TRACEINTO << "pOpStruct is " << (pOpStruct ? "valid" : "NULL") << ", pOtherOpStruct is " << (pOtherOpStruct ? "valid" : "NULL");
		return;
	}

	// prepare pOpStruct's operation_points string
	for (int i=0; i<pOpStruct->numberOfOperationPoints; i++)
	{
		sOpStruct_op << "\n\t" << i
				     << ": layerId: "		<< (int)( pOpStruct->tVideoOperationPoints[i].layerId )
				     << ", Tid: "			<< (int)( pOpStruct->tVideoOperationPoints[i].Tid)
				     << ", Did: "			<< (int)( pOpStruct->tVideoOperationPoints[i].Did )
				     << ", Qid: "			<< (int)( pOpStruct->tVideoOperationPoints[i].Qid )
				     << ", Pid: "			<< (int)( pOpStruct->tVideoOperationPoints[i].Pid )
				     << ", profile: "		<< (int)( pOpStruct->tVideoOperationPoints[i].profile )
				     << ", level: "			<< (int)( pOpStruct->tVideoOperationPoints[i].level )
				     << ", frameWidth: "	<< (int)( pOpStruct->tVideoOperationPoints[i].frameWidth )
				     << ", frameHeight: "	<< (int)( pOpStruct->tVideoOperationPoints[i].frameHeight )
				     << ", frameRate: "		<< (int)( pOpStruct->tVideoOperationPoints[i].frameRate )
				     << ", maxBitRate: "	<< (int)( pOpStruct->tVideoOperationPoints[i].maxBitRate );
	}

	// prepare pOtherOpStruct's operation_points string
	for (int i=0; i<pOtherOpStruct->numberOfOperationPoints; i++)
	{
		sOtherOpStruct_op << "\n\t" << i
						  << ": layerId: "		<< (int)( pOtherOpStruct->tVideoOperationPoints[i].layerId )
						  << ", Tid: "			<< (int)( pOtherOpStruct->tVideoOperationPoints[i].Tid)
						  << ", Did: "			<< (int)( pOtherOpStruct->tVideoOperationPoints[i].Did )
						  << ", Qid: "			<< (int)( pOtherOpStruct->tVideoOperationPoints[i].Qid )
						  << ", Pid: "			<< (int)( pOtherOpStruct->tVideoOperationPoints[i].Pid )
						  << ", profile: "		<< (int)( pOtherOpStruct->tVideoOperationPoints[i].profile )
						  << ", level: "			<< (int)( pOtherOpStruct->tVideoOperationPoints[i].level )
						  << ", frameWidth: "	<< (int)( pOtherOpStruct->tVideoOperationPoints[i].frameWidth )
						  << ", frameHeight: "	<< (int)( pOtherOpStruct->tVideoOperationPoints[i].frameHeight )
						  << ", frameRate: "		<< (int)( pOtherOpStruct->tVideoOperationPoints[i].frameRate )
						  << ", maxBitRate: "	<< (int)( pOtherOpStruct->tVideoOperationPoints[i].maxBitRate );
	}

	// print all params
	TRACEINTO << "\npOpStruct (remote, Master):"
			  << "\n\toperationPointSetId: " << pOpStruct->operationPointSetId << ", numberOfOperationPoints: " << (int)(pOpStruct->numberOfOperationPoints)
			  << sOpStruct_op.GetString() << "\n"
			  << "\npOtherOpStruct (local, Slave):"
			  << "\n\toperationPointSetId: " << pOtherOpStruct->operationPointSetId << ", numberOfOperationPoints: " << (int)(pOtherOpStruct->numberOfOperationPoints)
			  << sOtherOpStruct_op.GetString();
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::FindBestModeForAvcInVSWRelay(cmCapDirection (&directionArr)[2], BYTE (&bResArr)[MAX_SIP_MEDIA_TYPES][2], CapEnum (&algFound)[MAX_SIP_MEDIA_TYPES],
		int &i, int &j, cmCapDirection eOppositeDirection, ERoleLabel eRole, cmCapDataType mediaType,
		const CSipComMode& rPreferredMode, const CBaseCap* pPreferredMedia,const CSipCaps& rAlternativeCaps, CSipComMode* pBestMode) const
{
	CBaseCap* pBestCap = NULL;
	capBuffer* pBestCapBuffer = NULL;
	bool intersectStreams = false;

	if (pPreferredMedia) // there is a possibility that we won't prefer a specific media (auto)
	{

		CapEnum eAlg = pPreferredMedia->GetCapCode();
		CCapSetInfo capInfo = eAlg;

		if (capInfo.IsSupporedCap())
		{
			pBestCap = GetHighestCommon(directionArr[j], *pPreferredMedia);

			TRACEINTO << "mediaType: " << mediaType << ", role: " << eRole
					  << ", direction: " << directionArr[j] << ", eOppositeDirection: " << eOppositeDirection
					  << ", pBestCap " << (pBestCap ? "not NULL" : "NULL - please look at the next printing");

			if(pBestCap==NULL)
			{
				pBestCap=GetHighestRemoteAndLocalCaps(directionArr[j],eRole,mediaType, *pPreferredMedia,rAlternativeCaps);

				TRACEINTO << "mediaType: " << mediaType << ", role: " << eRole
						  << ", direction: " << directionArr[j] << ", eOppositeDirection: " << eOppositeDirection
						  << ", pBestCap " << (pBestCap ? "not NULL" : "NULL");
			}

			if (pBestCap != NULL)
			{
				bResArr[i][j] = YES;
				pBestCapBuffer = pBestCap->GetAsCapBuffer();

				// update the correct payload type in cap buffer
				// Transmit scm: payload type from remote caps , Receive scm: payload type from local scm
				if(pBestCapBuffer)
				{
					if(eOppositeDirection == cmCapTransmit)
						pBestCapBuffer->sipPayloadType = GetPayloadTypeByDynamicPreference(capInfo,pBestCap->GetProfile(),eRole);
					else
						pBestCapBuffer->sipPayloadType = rPreferredMode.GetPayloadType(mediaType,eOppositeDirection,eRole);
				}
				else
				{
					PASSERT(1);
				}

				/*pBestCap->FreeStruct();
				POBJDELETE(pBestCap);*/

				intersectStreams = true;
			}
			else
			{
				PTRACE(eLevelInfoNormal, "avc_vsw_relay: CSipCaps::FindBestModeForAvcInVSWRelay pBestCap == NULL");
			}
		}
	}

	if (pBestCap && bResArr[i][j] && pBestCapBuffer)
	{
        pBestMode->SetMediaModeSvc(pBestCapBuffer, mediaType, eOppositeDirection, eRole);

        if (intersectStreams)
        {
        	// copy streams for this media from target mode to BestMode
        	std::list <StreamDesc> targetStreamsDescList = rPreferredMode.GetStreamsListForMediaMode(mediaType,eOppositeDirection,eRole);
            pBestMode->SetStreamsListForMediaMode(targetStreamsDescList,mediaType,eOppositeDirection,eRole); /* this is needed for the SSRC  */
            FindBestOperationPointForAvcVSWRelay(eOppositeDirection, pBestMode);
//          // Intersect streams for this media and set the result in BestMode
//          pBestMode->IntersectStreams(this,mediaType,eOppositeDirection,eRole);
        }

		algFound[i] = (CapEnum)pBestCapBuffer->capTypeCode;

	}
	if(pBestCap)
	{
		pBestCap->FreeStruct();
		POBJDELETE(pBestCap);
		PDELETEA(pBestCapBuffer);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipCaps::FindBestOperationPointForAvcVSWRelay(cmCapDirection direction, CSipComMode* pBestMode) const
{
	int layerId = -1;
	CVideoOperationPointsSet* pOperationPoints = pBestMode->GetOperationPoints();
	EOperationPointPreset eOPPreset = pBestMode->GetOperationPointPreset();
	eIsUseOperationPointsPreset isUseOperationPointesPresets = pBestMode->GetIsUseOperationPointesPresets();

    if (direction==cmCapReceive)
    {
    	const VideoOperationPoint*  pOperationPoint = pOperationPoints->GetLowestOperationPoint(0);

    	PASSERTMSG_AND_RETURN(pOperationPoint==NULL, "pOperationPoint is NULL");
    	if ( IsContainingOperationPointForAvcVSWRelay(*pOperationPoint, eOPPreset, isUseOperationPointesPresets) )
    		layerId = pOperationPoint->m_layerId;
    }
    else // transmit
    {

    	    const std::list <VideoOperationPoint>* list = pOperationPoints->GetOperationPointsList();
    	    std::list <VideoOperationPoint>::const_iterator itr;

    	    for(itr = list->begin();itr != list->end(); itr++)
    	    {
    	       	const VideoOperationPoint operationPoint = (*itr);
    	       	if(IsContainingOperationPointForAvcVSWRelay(operationPoint, eOPPreset, isUseOperationPointesPresets))
    	       		layerId = operationPoint.m_layerId;
    	       	else
    	       		break;
    	    }

    }

    TRACEINTO << "direction: " << ::GetDirectionStr(direction) << ", layerId: " << layerId << ", eOPPreset: " << eOPPreset << ", isUseOperationPointesPresets: " << isUseOperationPointesPresets;
    if (layerId == -1)
    {
    	TRACEINTO << "layerId=-1. Set mode off";
		pBestMode->SetMediaOff(cmCapVideo, direction, kRolePeople);
    }
    else
    	pBestMode->SetVswRelayStreamDescAccordingToOperationPoint(layerId,direction,kRolePeople,0,true);


}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSipCaps::IsContainingOperationPointForAvcVSWRelay(const VideoOperationPoint &operationPoint, EOperationPointPreset eOPPreset, eIsUseOperationPointsPreset isUseOperationPointesPresets) const
{
	BOOL bRet = FALSE;

	APIU8 level;
    long mbps, fs;
    long opBitRate = operationPoint.m_maxBitRate*10;

    if ( SetPredefinedH264ParamsForVswRelayIfNeeded(eOPPreset, level, fs, mbps, isUseOperationPointesPresets, operationPoint) == false )
	{
		mbps = CalcMBPSforVswRelay(operationPoint);
		fs = CalcFSforVswRelay(operationPoint);
		ProfileToLevelTranslator plt;
		level = plt.ConvertResolutionAndRateToLevelEx(fs, mbps);
	}

	CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, NULL);
	if (!pH264VideoCap)
	{
		DBGPASSERT(1);
		return FALSE;
	}

	pH264VideoCap->SetDefaults(cmCapReceive);
	pH264VideoCap->SetBitRate(opBitRate);
	pH264VideoCap->SetLevel(level);
	pH264VideoCap->SetMbps(mbps);
	pH264VideoCap->SetFs(fs);
	pH264VideoCap->SetProfile(ProfileToLevelTranslator::SvcProfileToH264(operationPoint.m_videoProfile));

	DWORD details = 0;
	int arrInd = 0;
	DWORD videoValuesToCompare = kCapCode|kH264Profile|kH264Additional|kBitRate;

	bRet = IsContainingCapSet(cmCapReceive, *pH264VideoCap, videoValuesToCompare, &details, &arrInd);

    COstrStream msg;
    pH264VideoCap->Dump(msg);
    TRACEINTO << "IsContainingCapSet=" << (int)bRet << "\n" << msg.str().c_str();

	pH264VideoCap->FreeStruct();
	POBJDELETE(pH264VideoCap);

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
CH264VideoCap* CSipCaps::GetVideoCapFromOperationPoint(const VideoOperationPoint &operationPoint) const
{
    BOOL bRet = FALSE;

    APIU8 level;
    long mbps, fs;
    long opBitRate = operationPoint.m_maxBitRate;

    mbps = CalcMBPSforVswRelay(operationPoint);
    fs = CalcFSforVswRelay(operationPoint);
    ProfileToLevelTranslator plt;
    level = plt.ConvertResolutionAndRateToLevelEx(fs, mbps);

    CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, NULL);
    if (!pH264VideoCap)
    {
        DBGPASSERT(1);
        return FALSE;
    }

    pH264VideoCap->SetDefaults(cmCapReceive);
    pH264VideoCap->SetBitRate(opBitRate);
    pH264VideoCap->SetLevel(level);
    pH264VideoCap->SetMbps(mbps);
    pH264VideoCap->SetFs(fs);
    pH264VideoCap->SetProfile(ProfileToLevelTranslator::SvcProfileToH264(operationPoint.m_videoProfile));

//  COstrStream msg;
//  pH264VideoCap->Dump(msg);
//  TRACEINTO << msg.str().c_str();
    return pH264VideoCap;
}

/////////////////////////////////////////////////////////////////////////////
// Updates the SSRC Ids of the receive streams
void CSipCaps::SetSsrcIds(cmCapDataType eMediaType, cmCapDirection direction, ERoleLabel eRole, APIU32 *aSsrcIds, int aNumOfSsrcIds,bool isUpdate)
{
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(eMediaType, &numOfMediaCapSet, &pMediaCapList, eRole);

	for(int i=0; i < numOfMediaCapSet; i++)
	{
	    CBaseCap* pCap = CBaseCap::AllocNewCap((CapEnum)pMediaCapList[i]->capTypeCode, pMediaCapList[i]->dataCap);
		if (pCap)
		{
			if(pCap->IsCapContainsStreamsGroup())
			{
				if (direction & cmCapReceive)
					pCap->AddRecvStream(aSsrcIds,aNumOfSsrcIds,isUpdate);
				if (direction & cmCapTransmit)
					pCap->AddSendStream(aSsrcIds,aNumOfSsrcIds,isUpdate);
			}
			POBJDELETE(pCap);
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
// Check if Video's H263 cap support dynamic Payload Type only.
// Usually this returns FALSE. Return TRUE if there is only H263 cap(s) with dynamic PT.
BYTE CSipCaps::IsH263DynamicPayloadTypeOnly(ERoleLabel eRole) const
{
	BYTE isRmtRcvH263DynLoadOnly = FALSE;
	int numOfMediaCapSet		= 0;
	capBuffer** pMediaCapList	= NULL;
	GetMediaCaps(cmCapVideo, &numOfMediaCapSet, &pMediaCapList, eRole);

	if( pMediaCapList )
	{
		for(int i=0; i < numOfMediaCapSet; i++)
		{
			if(eH263CapCode == (CapEnum)pMediaCapList[i]->capTypeCode)
			{
				BYTE bIsDynamic = ::IsDynamicPayloadType(pMediaCapList[i]->sipPayloadType);
				if(bIsDynamic)
					isRmtRcvH263DynLoadOnly = TRUE;
				else
				{
					isRmtRcvH263DynLoadOnly = FALSE;
					break;
				}
			}
		}
	}

	TRACEINTO << "return: " << ((FALSE==isRmtRcvH263DynLoadOnly)?"FALSE":"TRUE");

	return isRmtRcvH263DynLoadOnly;

}
/////////////////////////////////////////////////////////////////////////////
void CSipCaps::setMsftSsrcAudio(DWORD ssrcAudio)
{
//	PTRACE2INT(eLevelInfoNormal, "CSipCaps::setMsftSsrcAudio ssrcAudio ", ssrcAudio);

	m_msftSsrcAudio = ssrcAudio;
}

void CSipCaps::setMsftSsrcVideo(DWORD ssrcVideoFirst, DWORD ssrcVideoLast, int lineNum)
{
	if (lineNum > MaxMsftSvcSdpVideoMlines || lineNum < 1)
	{
		PTRACE2INT(eLevelInfoNormal, "CSipCaps::setMsftSsrcVideo invalid lineNum ", lineNum);
		DBGPASSERT(1);
		return;
	}
//	PTRACE2INT(eLevelInfoNormal, "CSipCaps::setMsftSsrcVideo lineNum ", lineNum);
//	PTRACE2INT(eLevelInfoNormal, "CSipCaps::setMsftSsrcVideo ssrcVideo ", ssrcVideo);

	m_msftSsrcVideo[lineNum-1][0] = ssrcVideoFirst;
	m_msftSsrcVideo[lineNum-1][1] = ssrcVideoLast;

}

void CSipCaps::SetMsftMsiAudio(DWORD msiAudio)
{
	m_msftMsiAudio = msiAudio;
}

void CSipCaps::SetMsftMsiVideo(DWORD msiVideo, int lineNum)
{
	if (lineNum > MaxMsftSvcSdpVideoMlines || lineNum < 1)
	{
		PTRACE2INT(eLevelInfoNormal, "CSipCaps::SetMsftMsiVideo invalid lineNum ", lineNum);
		DBGPASSERT(1);
		return;
	}

	m_msftMsiVideo[lineNum-1] = msiVideo;
}


APIS32 CSipCaps::GetRtcpFbMask(ERoleLabel role)const
{
	APIS32 rtcpFbMask = 0;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;

	if (IsMedia(cmCapVideo, cmCapReceiveAndTransmit, role))
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo, cmCapReceiveAndTransmit, role);
		/* Get RTCP-FB mask from the first content caps, check only 1 content cap  */
		for (int i = 0; (i < numOfVideoMediaCapSet) ; i++)
		{
			CBaseVideoCap* pCap = (CBaseVideoCap*)GetCapSet(cmCapVideo, i, role);
			if(pCap)
			{
				//BRIDGE-12440: rtcp-fb mask in LPR caps is always 0!Do not use it.
				CapEnum capType = pCap->GetCapCode();
				if(eH261CapCode == capType || eH263CapCode == capType || eH264CapCode == capType|| eRtvCapCode==capType || eVP8CapCode==capType)
				{
					rtcpFbMask = pCap->GetRtcpFeedbackMask();
					POBJDELETE(pCap);
					return rtcpFbMask;															
				}
				POBJDELETE(pCap);
			}
		}
	}
	return rtcpFbMask;
}


//////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::GetMsSvcVidMode(MsSvcVideoModeDetails& MsSvcDetails) const
{
	PTRACE(eLevelInfoNormal, "CSipCaps::GetMsSvcVidMode ");
	
	BYTE bRetVal = NO;
	capBuffer** pMediaCapList = NULL;
	int numOfVideoMediaCapSet = 0;

	CBaseCap* pCap = NULL;
	pCap = this->GetCapSet(cmCapVideo);
	
	if(pCap == NULL)
	{
		PTRACE(eLevelInfoNormal, "CSipCaps::GetMsSvcVidMode no MS SVC cap");
		return bRetVal;
	}
	
	if ( IsMedia(cmCapVideo) )
	{
		numOfVideoMediaCapSet = GetNumOfMediaCapSets(cmCapVideo);
		for ( int i = 0; i < numOfVideoMediaCapSet && !bRetVal; i++)
		{
			CBaseVideoCap* pVideoCap = (CBaseVideoCap*) GetCapSet(cmCapVideo, i);
			if (pVideoCap && pVideoCap->GetCapCode() == (CapEnum)eMsSvcCapCode)
			{
				PTRACE(eLevelInfoNormal, "CSipCaps::GetMsSvcVidMode found ms svc caps ");
				MsSvcDetails.aspectRatio = ((CMsSvcVideoCap *)pVideoCap)->GetAspectRatio();
				MsSvcDetails.maxBitRate = ((CMsSvcVideoCap *)pVideoCap)->GetBitRate()*100;
				MsSvcDetails.minBitRate = ((CMsSvcVideoCap *)pVideoCap)->GetMinBitRate()*100;
				MsSvcDetails.maxHeight = ((CMsSvcVideoCap *)pVideoCap)->GetHeight();
				MsSvcDetails.maxWidth = ((CMsSvcVideoCap *)pVideoCap)->GetWidth();
				MsSvcDetails.maxFrameRate = ((CMsSvcVideoCap *)pVideoCap)->GetMaxFrameRate();
				MsSvcDetails.maxNumOfPixels = ((CMsSvcVideoCap *)pVideoCap)->GetMaxPixel();
//				MsSvcDetails.videoModeType = ((CMsSvcVideoCap *)pVideoCap)->GetCPVideoPartyType(); //TBD
				bRetVal = YES;
			}
			if(pVideoCap)
				PDELETE(pVideoCap);
		}
	}
	PDELETE(pCap);
	return bRetVal;
}

//////////////////////////////////////////////////////////////////////////
BYTE CSipCaps::SetMsSvcVidMode(MsSvcVideoModeDetails& MsSvcDetails)
{
	PTRACE(eLevelInfoNormal, "CSipCaps::SetMsSvcVidMode");
	
	BYTE bRetVal = NO;
	int numOfVideoMediaCapSet = 0;

	
	bool found = false;
	for ( int i = 0; i < m_numOfVideoCapSets && !found; i++)
	{
		CBaseVideoCap* pVideoCap = (CBaseVideoCap*) GetCapSet(cmCapVideo, i);
		if (pVideoCap && pVideoCap->GetCapCode() == (CapEnum)eMsSvcCapCode)
		{
			found = true;
		}
		if (pVideoCap)
			PDELETE(pVideoCap);
	}
	
	if (!found)
	{
		PTRACE(eLevelInfoNormal, "CSipCaps::SetMsSvcVidMode Add MsSvc cap");
		CMsSvcVideoCap* pMsSvcideoCap = (CMsSvcVideoCap *)CBaseCap::AllocNewCap(eMsSvcCapCode, NULL);
		if (pMsSvcideoCap)
		{
			PTRACE(eLevelInfoNormal, "CSipCaps::SetMsSvcVidMode Add MsSvc cap - adding cap as not found ms svc");
			pMsSvcideoCap->SetDefaults(cmCapReceiveAndTransmit);
			capBuffer* tempcapbuffrer = NULL;
			tempcapbuffrer = pMsSvcideoCap->GetAsCapBuffer();
			CCapSetInfo capInfo = eMsSvcCapCode;
			if(tempcapbuffrer)
			{
				tempcapbuffrer->sipPayloadType = eMsSvcDynamicPayload;
				AddCapSet(cmCapVideo, tempcapbuffrer);
				PDELETEA(tempcapbuffrer);
			}
			pMsSvcideoCap->FreeStruct();
			POBJDELETE(pMsSvcideoCap);

		}
	}
	
	for ( int i = 0; i < m_numOfVideoCapSets && !bRetVal; i++)
	{
		CBaseVideoCap* pVideoCap = (CBaseVideoCap*) GetCapSet(cmCapVideo, i); AUTO_DELETE(pVideoCap);
		if (pVideoCap && pVideoCap->GetCapCode() == (CapEnum)eMsSvcCapCode)
		{
			PTRACE(eLevelInfoNormal, "CSipCaps::SetMsSvcVidMode found ms svc caps ");
			((CMsSvcVideoCap *)pVideoCap)->SetAspectRatio(MsSvcDetails.aspectRatio);
			((CMsSvcVideoCap *)pVideoCap)->SetBitRate(MsSvcDetails.maxBitRate/100);
			((CMsSvcVideoCap *)pVideoCap)->SetMinBitRate(MsSvcDetails.minBitRate/100);
			((CMsSvcVideoCap *)pVideoCap)->SetHeight(MsSvcDetails.maxHeight);
			((CMsSvcVideoCap *)pVideoCap)->SetWidth(MsSvcDetails.maxWidth);
			((CMsSvcVideoCap *)pVideoCap)->SetMaxFrameRate(MsSvcDetails.maxFrameRate);
			((CMsSvcVideoCap *)pVideoCap)->SetMaxPixelNum(MsSvcDetails.maxNumOfPixels);
			bRetVal = YES;
		}
	}
	
	return bRetVal;
}
/////////////////////////////////////////////////////////////////////////////
void CSipCaps::ReplaceSendRecvStreams()
{
	// ===== audio
/*	for(int i = 0; i < m_numOfAudioCapSets; i++)
	{
		CBaseAudioCap* pAudioCap = (CBaseAudioCap*) CBaseCap::AllocNewCap(m_audioCapList[i]->dataCap);
		if (pAudioCap)
		{
			if ( CSacAudioCap::IsSacAudio(pAudioCap->GetCapCode()) )
			{
				((CSacAudioCap*)pAudioCap)->ReplaceSendRecvStreams();
			}

			POBJDELETE(pAudioCap);
		} // end if (pAudioCap)
	}
*/
/*	// ===== video
	for(int i = 0; i < m_numOfVideoCapSets; i++)
	{
		CBaseVideoCap* pVideoCap = (CBaseVideoCap*) CBaseCap::AllocNewCap(m_videoCapList[i]->dataCap);
		if (pVideoCap)
		{
			if ( eSvcCapCode == pVideoCap->GetCapCode() )
			{
				((CSvcVideoCap*)pVideoCap)->ReplaceSendRecvStreams();
			}

			POBJDELETE(pVideoCap);
		} // end if (pVideoCap)
	}*/
}
void CSipCaps::CreateIceFakeCandidates(UdpAddresses udpParams)
{
	CBaseCap* pIceCap;
	EResult eResOfSet;
	CCapSetInfo capInfo = eUnknownAlgorithemCapCode;
	//first part pwd
	pIceCap = (CICEPwdCap*)CBaseCap::AllocNewCap((CapEnum)eIcePwdCapCode,NULL);
	if (pIceCap)
	{
		eResOfSet	= kSuccess;
		eResOfSet &= ((CICEPwdCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,"aKe0e0F7a7dbZJECdN0sMs6/");
		if(eResOfSet)
			{
				capInfo = eIcePwdCapCode;
				capBuffer* pCapBuffer = pIceCap->GetAsCapBuffer();

				if(pCapBuffer)
				{
					pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
					//if((CapEnum)capInfo == eIceCandidateCapCode)
					//	AddICECapToTmpArray(pCapBuffer,DataStr);
					//else
					AddICECapSet(eAudioSession,pCapBuffer);
					if(m_numOfVideoCapSets > 0)
						AddICECapSet(eVideoSession,pCapBuffer);
				}
				else
				{
					PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Create cap buffer has failed");
				}
				PDELETEA(pCapBuffer);
			}
			else
				PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Set struct has failed");

			pIceCap->FreeStruct();

			POBJDELETE(pIceCap)
	}

	//second part
	 pIceCap = (CICEUfragCap*)CBaseCap::AllocNewCap((CapEnum)eIceUfragCapCode,NULL);
     if (pIceCap)
     {
    	 capInfo = eIceUfragCapCode;
		 eResOfSet	= kSuccess;
	     eResOfSet &= ((CICEUfragCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,"OqLs");
	     if(eResOfSet)
		{
			capBuffer* pCapBuffer = pIceCap->GetAsCapBuffer();

			if(pCapBuffer)
			{
				pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
				//if((CapEnum)capInfo == eIceCandidateCapCode)
				//	AddICECapToTmpArray(pCapBuffer,DataStr);
				//else
				AddICECapSet(eAudioSession,pCapBuffer);
				if(m_numOfVideoCapSets > 0)
					AddICECapSet(eVideoSession,pCapBuffer);
			}
			else
			{
				PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Create cap buffer has failed");
			}
			PDELETEA(pCapBuffer);
		}
		else
			PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Set struct has failed");

		pIceCap->FreeStruct();

		POBJDELETE(pIceCap)
	 }

     //3 part for audio and video set host candidates
     char ipStr[64];
     memset(ipStr,'\0',64);
     :: SystemDWORDToIpString(udpParams.IpV4Addr.ip,ipStr);
     capInfo = eIceCandidateCapCode;

 //start with audio RTP
	 pIceCap = (CICECandidateCap*)CBaseCap::AllocNewCap((CapEnum)eIceCandidateCapCode,NULL);
		if (pIceCap)
		{

			//VNGFE-5845
			char newDataStrRtpAudio[512];
			memset (newDataStrRtpAudio, '\0', sizeof(newDataStrRtpAudio));
		//	if(HandleSrflxCandidate(DataStr,newDataStr,IceSessionsTypes))
		//		DataStr = newDataStr;
			//1 1 UDP 2130706431 10.227.2.121 49360 typ host
			ALLOCBUFFER(rtpPortAudio, 12);
			rtpPortAudio[0] = '\0';
			sprintf(rtpPortAudio,"%d",udpParams.AudioChannelPort);

			snprintf(newDataStrRtpAudio, sizeof(newDataStrRtpAudio), "1 1 UDP 2130706431 %s %s typ host", ipStr, rtpPortAudio);

			PTRACE2(eLevelError,"CSipCaps::AddSingleICECap:  newDataStr",newDataStrRtpAudio);
			DEALLOCBUFFER(rtpPortAudio);
			eResOfSet	= kSuccess;
			eResOfSet &= ((CICECandidateCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,newDataStrRtpAudio, eIpVersion4);

			if(eResOfSet)
				{
					capBuffer* pCapBuffer = pIceCap->GetAsCapBuffer();

					if(pCapBuffer)
					{
						pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
						//if((CapEnum)capInfo == eIceCandidateCapCode)
						//	AddICECapToTmpArray(pCapBuffer,DataStr);
						//else
						AddICECapSet(eAudioSession,pCapBuffer);

					}
					else
					{
						PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Create cap buffer has failed");
					}
					PDELETEA(pCapBuffer);
				}
				else
					PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Set struct has failed");

				pIceCap->FreeStruct();

				POBJDELETE(pIceCap)
		}

		//now audio rtcp
		 pIceCap = (CICECandidateCap*)CBaseCap::AllocNewCap((CapEnum)eIceCandidateCapCode,NULL);
		if (pIceCap)
		{
			capInfo = eIceCandidateCapCode;
			//VNGFE-5845
			char newDataStrRtpAudio[512];
			memset (newDataStrRtpAudio, '\0', sizeof(newDataStrRtpAudio));

			ALLOCBUFFER(rtcpPortAudio, 12);
			rtcpPortAudio[0] = '\0';
			sprintf(rtcpPortAudio,"%d",(udpParams.AudioChannelPort +1));

			snprintf(newDataStrRtpAudio, sizeof(newDataStrRtpAudio), "1 2 UDP 2130706431 %s %s typ host", ipStr, rtcpPortAudio);

			PTRACE2(eLevelError,"CSipCaps::AddSingleICECap: newDataStr",newDataStrRtpAudio);
			DEALLOCBUFFER(rtcpPortAudio);
			eResOfSet	= kSuccess;
			eResOfSet &= ((CICECandidateCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,newDataStrRtpAudio, eIpVersion4);

			if(eResOfSet)
				{
					capBuffer* pCapBuffer = pIceCap->GetAsCapBuffer();

					if(pCapBuffer)
					{
						pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
						AddICECapSet(eAudioSession,pCapBuffer);

					}
					else
					{
						PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Create cap buffer has failed");
					}
					PDELETEA(pCapBuffer);
				}
				else
					PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Set struct has failed");

				pIceCap->FreeStruct();

				POBJDELETE(pIceCap)
		}
		if(m_numOfVideoCapSets > 0)
		{
			//now video rtp
			 pIceCap = (CICECandidateCap*)CBaseCap::AllocNewCap((CapEnum)eIceCandidateCapCode,NULL);
			if (pIceCap)
			{

				//VNGFE-5845
				char newDataStrRtpVideo[512];
				memset (newDataStrRtpVideo, '\0', sizeof(newDataStrRtpVideo));
			//	if(HandleSrflxCandidate(DataStr,newDataStr,IceSessionsTypes))
			//		DataStr = newDataStr;
				//1 1 UDP 2130706431 10.227.2.121 49360 typ host
				ALLOCBUFFER(rtpPortVideo, 12);
				rtpPortVideo[0] = '\0';
				sprintf(rtpPortVideo,"%d",udpParams.VideoChannelPort);

				snprintf(newDataStrRtpVideo, sizeof(newDataStrRtpVideo), "1 1 UDP 2130706431 %s %s typ host", ipStr, rtpPortVideo);

				PTRACE2(eLevelError,"CSipCaps::AddSingleICECap:  newDataStr",newDataStrRtpVideo);
				DEALLOCBUFFER(rtpPortVideo);
				eResOfSet	= kSuccess;
				eResOfSet &= ((CICECandidateCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,newDataStrRtpVideo, eIpVersion4);

				if(eResOfSet)
					{
						capBuffer* pCapBuffer = pIceCap->GetAsCapBuffer();

						if(pCapBuffer)
						{
							pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
							//if((CapEnum)capInfo == eIceCandidateCapCode)
							//	AddICECapToTmpArray(pCapBuffer,DataStr);
							//else
							AddICECapSet(eVideoSession,pCapBuffer);

						}
						else
						{
							PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Create cap buffer has failed");
						}
						PDELETEA(pCapBuffer);
					}
					else
						PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Set struct has failed");

					pIceCap->FreeStruct();

					POBJDELETE(pIceCap)
			}

			//now video rtcp

			 pIceCap = (CICECandidateCap*)CBaseCap::AllocNewCap((CapEnum)eIceCandidateCapCode,NULL);
			if (pIceCap)
			{
				capInfo = eIceCandidateCapCode;
				//VNGFE-5845
				char newDataStrRtpVideo[512];
				memset (newDataStrRtpVideo, '\0', sizeof(newDataStrRtpVideo));

				ALLOCBUFFER(rtcpPortVideo, 12);
				rtcpPortVideo[0] = '\0';
				sprintf(rtcpPortVideo,"%d",(udpParams.VideoChannelPort +1));

				snprintf(newDataStrRtpVideo, sizeof(newDataStrRtpVideo), "1 2 UDP 2130706431 %s %s typ host", ipStr, rtcpPortVideo);

				PTRACE2(eLevelError,"CSipCaps::AddSingleICECap: newDataStr",newDataStrRtpVideo);
				DEALLOCBUFFER(rtcpPortVideo);
				eResOfSet	= kSuccess;
				eResOfSet &= ((CICECandidateCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,newDataStrRtpVideo, eIpVersion4);

				if(eResOfSet)
					{
						capBuffer* pCapBuffer = pIceCap->GetAsCapBuffer();

						if(pCapBuffer)
						{
							pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
							AddICECapSet(eVideoSession,pCapBuffer);

						}
						else
						{
							PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Create cap buffer has failed");
						}
						PDELETEA(pCapBuffer);
					}
					else
						PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Set struct has failed");

					pIceCap->FreeStruct();

					POBJDELETE(pIceCap)
			}

		}
		/////////////////////////////////
		//now add rtcp for audio and video


		//now rtcp Audio
		pIceCap = (CRtcpCap*)CBaseCap::AllocNewCap((CapEnum)eRtcpCapCode,NULL);
		if (pIceCap)
		{
			capInfo = eRtcpCapCode;
			eResOfSet	= kSuccess;
			ALLOCBUFFER(rtcpPortAudio, 12); AUTO_DELETE_ARRAY(rtcpPortAudio);
			rtcpPortAudio[0] = '\0';
			sprintf(rtcpPortAudio,"%d",(udpParams.AudioChannelPort +1));
			eResOfSet &= ((CRtcpCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,rtcpPortAudio);

			 if(eResOfSet)
			 {
					capBuffer* pCapBuffer = pIceCap->GetAsCapBuffer();

					if(pCapBuffer)
					{
						pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
						AddICECapSet(eAudioSession,pCapBuffer);

					}
					else
					{
						PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Create cap buffer has failed");
					}
					PDELETEA(pCapBuffer);
			 }
			 else
				PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Set struct has failed");

				pIceCap->FreeStruct();

				POBJDELETE(pIceCap)
		}

		//now rtcp video
	if(m_numOfVideoCapSets > 0)
	{
			pIceCap = (CRtcpCap*)CBaseCap::AllocNewCap((CapEnum)eRtcpCapCode,NULL);
			if (pIceCap)
			{
				capInfo = eRtcpCapCode;
				eResOfSet	= kSuccess;
				ALLOCBUFFER(rtcpPortVideo, 12); AUTO_DELETE_ARRAY(rtcpPortVideo);
				rtcpPortVideo[0] = '\0';
				sprintf(rtcpPortVideo,"%d",(udpParams.VideoChannelPort +1));
				eResOfSet &= ((CRtcpCap*)pIceCap)->SetStruct(cmCapGeneric, cmCapReceiveAndTransmit,rtcpPortVideo);

				 if(eResOfSet)
				 {
						capBuffer* pCapBuffer = pIceCap->GetAsCapBuffer();

						if(pCapBuffer)
						{
							pCapBuffer->sipPayloadType = ::GetPayloadType(capInfo);
							AddICECapSet(eVideoSession,pCapBuffer);

						}
						else
						{
							PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Create cap buffer has failed");
						}
						PDELETEA(pCapBuffer);
				 }
				 else
					PTRACE(eLevelError,"CSipCaps::AddSingleICECap: Set struct has failed");

					pIceCap->FreeStruct();

					POBJDELETE(pIceCap)
			}
	}





}

///////////////////////////////////////////////////////////////////////////////BRIDGE-10123
EEncryptionKeyToUse CSipCaps::GetUseMkiEncrytionFlag()
{
	EEncryptionKeyToUse encryptionKeyFlag = eUseBothEncryptionKeys;

	CProcessBase* pProccess = CProcessBase::GetProcess();
	FPASSERTMSG_AND_RETURN_VALUE(!pProccess , "!pProccess",  encryptionKeyFlag);

	CSysConfig *sysConfig = pProccess->GetSysConfig();
	FPASSERTMSG_AND_RETURN_VALUE(!sysConfig , "!sysConfig",  encryptionKeyFlag);

	std::string strKey;
    sysConfig->GetDataByKey("SEND_SRTP_MKI", strKey);

    if(strKey.compare("AUTO")==0)
    	encryptionKeyFlag = eUseBothEncryptionKeys;

    else if(strKey.compare("YES")==0)
    	encryptionKeyFlag = eUseMkiKeyOnly;

    else if(strKey.compare("NO")==0)
    	encryptionKeyFlag = eUseNonMkiKeyOnly;

	FTRACEINTO << "SEND_SRTP_MKI is " << strKey << " encryptionKeyFlag " << (int)encryptionKeyFlag;

	return encryptionKeyFlag;
}
