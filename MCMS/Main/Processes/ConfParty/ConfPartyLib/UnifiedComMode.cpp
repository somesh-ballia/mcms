//+========================================================================+
//                            UnifiedComMode.CPP                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       UnifiedComMode.CPP                                          |
// SUBSYSTEM:  ConfPartyLib                                                |
// PROGRAMMER: Romem                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 06/07/05    |                                                     |
//+========================================================================+
#include  "UnifiedComMode.h"
#include "CommModeInfo.h"
#include  "H320ComMode.h"
#include  "TraceStream.h"
#include  "CapInfo.h"
#include "ContentBridge.h"
#include "ConfPartyGlobals.h"
#include "SysConfigKeys.h"

ContentRateMap CUnifiedComMode::m_amcH264DynamicContentRateResTable;
Rate2ResolutionMap CUnifiedComMode::m_maxResolutionPerContentRateTable;
ContentRateMap CUnifiedComMode::m_amcH264DynamicContentRateResTable_HP;
Rate2ResolutionMap CUnifiedComMode::m_maxResolutionPerContentRateTable_HP;
BOOL CUnifiedComMode::m_initContentTableFlag = FALSE;

const TH239ToEPC CUnifiedComMode::g_239ToEPCTbl[MAX_H239_CAPS] =
{
	{AMC_0k, 	  AMSC_0k},
	{AMC_40k,     (BYTE)NA},
	{AMC_64k,     AMSC_64k},
	{AMC_96k,     (BYTE)NA},
	{AMC_128k,    AMSC_128k},
	{AMC_192k,    AMSC_192k},
	{AMC_256k,    AMSC_256k},
	{AMC_384k,    AMSC_384k},
	{AMC_512k,    AMSC_512k},
	{AMC_768k,    AMSC_768k},
	{AMC_HSDCap,   (BYTE)NA},
	{AMC_1024k,   AMSC_1024k},
	{AMC_1152k,   AMSC_1152k},
	{AMC_1280k,   AMSC_1280k},
	{AMC_1536k,   AMSC_1536k},
	{AMC_2048k,   AMSC_2048k},
	{AMC_2560k,   AMSC_2560k},
	{AMC_3072k,   AMSC_3072k},
	{AMC_4096k,   AMSC_4096k},
};
/////////////////////////////////////////////////////////////////////////////
CUnifiedComMode::CUnifiedComMode(BYTE xferCustomizedContentRate,BYTE lConfRate, BOOL bIsHighProfileContent)
	: m_pIPScm(new CIpComMode)
	, m_pIsdnScm(new CComMode)
	, m_pCopVideoTxModes(NULL)
    , m_isTIPContent(FALSE)
	, m_amcCustomizedContentRate(TranslateXferRateToAmcRate(xferCustomizedContentRate))
	, m_isHighProfileContent(bIsHighProfileContent)
{
	initStaticContentTables();	;
	m_pIPScm->SetCallRate(TranslateXferRateToIpRate(lConfRate));
}
/////////////////////////////////////////////////////////////////////////////
CUnifiedComMode::~CUnifiedComMode()
{
	POBJDELETE(m_pIPScm);
	POBJDELETE(m_pIsdnScm);
	POBJDELETE(m_pCopVideoTxModes);
}
/////////////////////////////////////////////////////////////////////////////
CUnifiedComMode::CUnifiedComMode(const CUnifiedComMode & rOtherUnifiedSCM)
 : CPObject(rOtherUnifiedSCM)
 , m_amcCustomizedContentRate(rOtherUnifiedSCM.m_amcCustomizedContentRate)
 , m_isHighProfileContent(rOtherUnifiedSCM.m_isHighProfileContent)
{
	m_pIPScm = new CIpComMode(*(rOtherUnifiedSCM.m_pIPScm));
	m_pIsdnScm = new CComMode(*(rOtherUnifiedSCM.m_pIsdnScm));

	m_pCopVideoTxModes = NULL;
	if (rOtherUnifiedSCM.m_pCopVideoTxModes)
		m_pCopVideoTxModes = new CCopVideoTxModes(*(rOtherUnifiedSCM.m_pCopVideoTxModes));
}
/////////////////////////////////////////////////////////////////////////////
CUnifiedComMode& CUnifiedComMode::operator=(const CUnifiedComMode & rOtherUnifiedSCM)
{
	if(this == &rOtherUnifiedSCM){
		return *this;
	}

	if(m_pIPScm)
		POBJDELETE(m_pIPScm);
	m_pIPScm = new CIpComMode(*(rOtherUnifiedSCM.m_pIPScm));

	if(m_pIsdnScm)
		POBJDELETE(m_pIsdnScm);
	m_pIsdnScm = new CComMode(*(rOtherUnifiedSCM.m_pIsdnScm));

	POBJDELETE(m_pCopVideoTxModes);
	if (rOtherUnifiedSCM.m_pCopVideoTxModes)
		m_pCopVideoTxModes = new CCopVideoTxModes(*(rOtherUnifiedSCM.m_pCopVideoTxModes));

	m_amcCustomizedContentRate = rOtherUnifiedSCM.m_amcCustomizedContentRate;
	m_isHighProfileContent = rOtherUnifiedSCM.m_isHighProfileContent;

	return *this;
}
/////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetConfType(WORD ConfType)
{
	EConfType ip_conf_type = kCp;
	WORD bIsFreeVideoRate = TRUE;
	switch(ConfType)
	{
		case VIDEO_SWITCH:
		{
			ip_conf_type = kVideoSwitch;
			bIsFreeVideoRate = FALSE;
			break;
		}
		case VIDEO_SWITCH_FIXED:
		{
			ip_conf_type = kVSW_Fixed;
			bIsFreeVideoRate = FALSE;
			break;
		}
		case CONTINUOUS_PRESENCE:
		{
			ip_conf_type = kCp;
			bIsFreeVideoRate = TRUE;
			break;
		}
		case SOFTWARE_CONTINUOUS_PRESENCE:
		{
			ip_conf_type = kSoftCp;
			bIsFreeVideoRate = TRUE;
			break;
		}
		case ADVANCED_LAYOUTS:
		{
			ip_conf_type = kCpQuad;
			bIsFreeVideoRate = TRUE;
			break;
		}
		case VIDEO_SESSION_COP:
		{
			ip_conf_type = kCop;
			bIsFreeVideoRate = FALSE;
			break;
		}
		default:
		{
			DBGPASSERT(ConfType);
			ip_conf_type = kCp;
			bIsFreeVideoRate = TRUE;
			break;
		}
	}

	m_pIPScm->SetConfType(ip_conf_type);

	m_pIsdnScm->SetIsFreeVideoRate(bIsFreeVideoRate);
}
/////////////////////////////////////////////////////////////////////////////
WORD CUnifiedComMode::GetConfType()
{
	return m_pIPScm->GetConfType();
}
/////////////////////////////////////////////////////////////////////////////
void  CUnifiedComMode::SetIsAutoVidRes( BYTE isAutoVidRes)
{
   	m_pIPScm->SetAutoVideoResolution(isAutoVidRes);
   	TRACESTR (eLevelInfoNormal) << "WARNING! CUnifiedComMode::SetIsAutoVidRes - No matching func in H320ComMode";

}
/////////////////////////////////////////////////////////////////////////////
WORD  CUnifiedComMode::IsAutoVidRes()
{
	return m_pIPScm->IsAutoVideoResolution();
}
/////////////////////////////////////////////////////////////////////////////
void  CUnifiedComMode::SetIsAutoVidProtocol(BYTE isAutoVidProtocol)
{
	m_pIPScm->SetAutoVideoProtocol(isAutoVidProtocol);
	TRACESTR (eLevelInfoNormal) << "WARNING! CUnifiedComMode::SetIsAutoVidProtocol - No matching func in H320ComMode";
}
/////////////////////////////////////////////////////////////////////////////
void  CUnifiedComMode::SetOperationPointPreset(EOperationPointPreset eOPPreset)
{
	m_pIPScm->SetOperationPointPreset(eOPPreset);
	TRACESTR (eLevelInfoNormal) << "WARNING! CUnifiedComMode::SetOperationPointPreset - No matching func in H320ComMode";
}
/////////////////////////////////////////////////////////////////////////////
WORD  CUnifiedComMode::IsAutoVidProtocol()
{
	return m_pIPScm->IsAutoVideoProtocol();
}
////////////////////////////////////////////////////////////////////////////
// call rate
////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetCallRate(BYTE reservationRate)
{
	CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
	DWORD h323Rate = lCapInfo.TranslateReservationRateToIpRate(reservationRate);
	 m_pIPScm->SetCallRate(h323Rate);

	 m_pIsdnScm->SetXferMode((WORD)reservationRate);

}
///////////////////////////////////////////////////////////////////////////
DWORD CUnifiedComMode::GetCallRate() const
{
	DWORD callRate = m_pIPScm->GetCallRate();
	callRate = callRate * 1000;
	return callRate;
}
///////////////////////////////////////////////////////////////////////////
// video params
// direction - 0 in and out (as today - default)
//           -  1 out
//            -  2 in
////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetH264VidMode(WORD h264Profile, BYTE h264Level, long maxBR, long maxMBPS,
		long maxFS, long maxDPB, long maxSAR, long maxStaticMB, WORD isAutoVidScm, WORD direction )
{
	APIU8 packatizationMode = H264_NON_INTERLEAVED_PACKETIZATION_MODE;

	if (!IsFeatureSupportedBySystem(eFeatureH264PacketizationMode))
		packatizationMode = H264_SINGLE_NAL_PACKETIZATION_MODE;

	m_pIPScm->SetH264Scm(h264Profile, h264Level, maxMBPS, maxFS, maxDPB, maxBR, maxSAR, maxStaticMB, (cmCapDirection)direction, packatizationMode);
	m_pIPScm->SetAutoVideoResolution(isAutoVidScm);

	H264VideoModeDetails h264VidModeDetails;
	h264VidModeDetails.profileValue = H264_Profile_BaseLine; // Currently ISDN support baseline only.
	h264VidModeDetails.levelValue =  h264Level;
	h264VidModeDetails.maxMBPS = maxMBPS;
	h264VidModeDetails.maxFS = maxFS;
	h264VidModeDetails.maxDPB = maxDPB;
	h264VidModeDetails.maxBR = maxBR;
	h264VidModeDetails.videoModeType = eCIF30; //put default value to avoid memory error
	h264VidModeDetails.maxStaticMbps = maxStaticMB;

	m_pIsdnScm->SetH264VideoParams(h264VidModeDetails,maxSAR,isAutoVidScm);

}

///////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetH264VidMode(H264VideoModeDetails h264VidModeDetails, long maxSAR, WORD isAutoVidScm, WORD direction )
{
	APIU8 packatizationMode = H264_NON_INTERLEAVED_PACKETIZATION_MODE;

	if (h264VidModeDetails.videoModeType == eHD720Asymmetric)
	{
		TRACESTR (eLevelInfoNormal) << "CUnifiedComMode::SetH264VidMode SETTING SCM TO SUPPORT HD720 asymmetric!!";
	}

	if (!IsFeatureSupportedBySystem(eFeatureH264PacketizationMode))
		packatizationMode = H264_SINGLE_NAL_PACKETIZATION_MODE;


	m_pIPScm->SetH264VideoParams(h264VidModeDetails, maxSAR, (cmCapDirection)direction, packatizationMode);
	m_pIPScm->SetAutoVideoResolution(isAutoVidScm);

	m_pIsdnScm->SetH264VideoParams(h264VidModeDetails,maxSAR,isAutoVidScm);

}
///////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetVP8VidMode(VP8VideoModeDetails vp8VideoDetails,WORD isAutoVidScm, WORD direction )
{//N.A. DEBUG VP8
	m_pIPScm->SetVP8VideoParams(vp8VideoDetails,(cmCapDirection)direction);
	m_pIPScm->SetAutoVideoResolution(isAutoVidScm);

}

////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetProtocolClassicVidMode(WORD protocol, int qcifMpi, int cifMpi, int cif4Mpi, int cif16Mpi,WORD IsAutoVidScm, WORD direction)
{
	CComModeInfo lComModeInfo(protocol,(WORD)StartVideoCap);

    CapEnum h323VideoCapCode = lComModeInfo.GetH323ModeType();
	m_pIPScm->SetScmMpi(h323VideoCapCode, qcifMpi,cifMpi,cif4Mpi,cif16Mpi,(cmCapDirection)direction);
	m_pIPScm->SetAutoVideoResolution(IsAutoVidScm);

	TRACESTR (eLevelInfoNormal) << "WARNING! CUnifiedComMode::SetProtocolClassicVidMode - No matching func in H320ComMode";

}
///////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetH263ScmPlus(BYTE bAnnexP, BYTE bAnnexT, BYTE bAnnexN, BYTE bAnnexI_NS,
									 char vga, char ntsc, char svga, char xga, char qntsc, WORD direction) // H.263 Annexes
{

    m_pIPScm->SetH263ScmPlus(bAnnexP, bAnnexT, bAnnexN, bAnnexI_NS, vga, ntsc,svga, xga, qntsc,(cmCapDirection) direction);
    TRACESTR (eLevelInfoNormal) << "WARNING! CUnifiedComMode::SetProtocolClassicVidMode - No matching func in H320ComMode";
}
///////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetVideoOff(WORD direction)
{
	m_pIPScm->SetMediaOff(cmCapVideo,/*cmCapRecieveAndTransmit*/(cmCapDirection)direction);
	m_pIsdnScm->SetVidModeOff();
}
///////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::GetH264VidMode(WORD& h264Profile, BYTE& h264Level, long& maxBR, long& maxMBPS,
		long& maxFS, long& maxDPB, long& maxSAR, long& maxStaticMB, WORD& isAutoVidScm, WORD direction) const
{
	m_pIPScm->GetH264Scm(h264Profile,h264Level,maxMBPS,maxFS,maxDPB,maxBR, maxSAR, maxStaticMB, (cmCapDirection)direction);

}

//////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::GetProtocolClassicVidMode(WORD protocol, int& qcifMpi, int& cifMpi,
												int& cif4Mpi, int& cif16Mpi,WORD& isAutoVidScm, WORD direction) const// H.263/H.261
{

	//CComModeInfo comModeInfo(protocol,(WORD)StartVideoCap);

    //CapEnum h323VideoCapCode = comModeInfo.GetH323ModeType();

    switch(protocol)
	{
	case H261: // eH261CapCode:
		{
			qcifMpi = m_pIPScm->GetFormatMpi((EFormat)kQCif, (cmCapDirection)direction);

			cifMpi = m_pIPScm->GetFormatMpi((EFormat)kCif, (cmCapDirection)direction);

			break;
		}
	case H263: //eH263CapCode:
		{
			qcifMpi = m_pIPScm->GetFormatMpi((EFormat)H263_QCIF_SQCIF, (cmCapDirection)direction);

			cifMpi = m_pIPScm->GetFormatMpi((EFormat)H263_CIF, (cmCapDirection)direction);

			cif4Mpi = m_pIPScm->GetFormatMpi((EFormat)H263_CIF_4, (cmCapDirection)direction);

			cif16Mpi = m_pIPScm->GetFormatMpi((EFormat)H263_CIF_16, (cmCapDirection)direction);
			break;
		}
	}

}
//////////////////////////////////////////////////////////////////////////
void  GetH263ScmPlus(BYTE& bAnnexP, BYTE& bAnnexT, BYTE& bAnnexN, BYTE& bAnnexI_NS,
		char& vga, char& ntsc, char& svga, char& xga, char& qntsc) // H.263 Annexes
{
	BYTE localbAnnexP = 0;
	BYTE localbAnnexT = 0;
	BYTE localbAnnexN = 0;
	BYTE localbAnnexI_NS = 0;
	BYTE localVga = 0;
	BYTE localNtsc = 0;
	BYTE localSvga = 0;
    BYTE localXga = 0;
	BYTE localQntsc = 0;

	bAnnexP = localbAnnexP;
	bAnnexT = localbAnnexT;
    bAnnexN = localbAnnexN;
    bAnnexI_NS = localbAnnexI_NS;
    vga = localVga;
    ntsc = localNtsc;
	svga = localSvga;
    xga = localXga;
    qntsc = localQntsc;
}

///////////////////////////////////////////////////////////////////////////
// audio params
///////////////////////////////////////////////////////////////////////////
void  CUnifiedComMode::SetAudModeAndAudBitRate(WORD audMode, WORD direction)
{
	CComModeInfo comModeInfo(audMode, StartAudioCap);

    CapEnum h323AudioCapCode = comModeInfo.GetH323ModeType();

	m_pIPScm->SetAudioAlg(h323AudioCapCode,(cmCapDirection)direction);

	CAudMode modeToSet;
	modeToSet.SetBitRate(audMode);
	m_pIsdnScm->SetAudMode(modeToSet);
}
///////////////////////////////////////////////////////////////////////////
WORD  CUnifiedComMode::GetAudMode(WORD direction) const
{
	CapEnum h323protocol = (CapEnum)(m_pIPScm->GetMediaType(cmCapAudio,(cmCapDirection)direction));
    CComModeInfo comModeInfo = h323protocol;
    WORD H320protocol = comModeInfo.GetH320ModeType();
	return H320protocol;
}
///////////////////////////////////////////////////////////////////////////
WORD  CUnifiedComMode::GetAudBitRate() const
{
	return 1;
}
///////////////////////////////////////////////////////////////////////////

void CUnifiedComMode::SetVideoBitRate(int newBitRate, cmCapDirection direction,ERoleLabel eRole)
{
	m_pIPScm->SetVideoBitRate(newBitRate,direction,eRole);
	TRACESTR (eLevelInfoNormal) << "WARNING! CUnifiedComMode::SetVideoBitRate - No matching func in H320ComMode";
}
///////////////////////////////////////////////////////////////////////////
// encryption
///////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetEncrypMode(WORD mode, BOOL bShouldDisconnectOnEncryptFailure /* = TRUE */  ) // if Encryp_On call CreateLocalComModeECS() of the internal object
{
	if (mode == Encryp_On)
	{
		m_pIPScm->SetIsEncrypted(mode, bShouldDisconnectOnEncryptFailure);
		m_pIPScm->CreateLocalComModeECS(kAES_CBC,kHalfKeyDH1024);
		m_pIsdnScm->SetOtherEncrypMode(mode);
		m_pIsdnScm->SetShouldDisconnectOnEncrypFailure(bShouldDisconnectOnEncryptFailure);
	}
}
///////////////////////////////////////////////////////////////////////////
WORD CUnifiedComMode::GetEncrypMode() const
{
	return m_pIPScm->GetIsEncrypted();
}
///////////////////////////////////////////////////////////////////////////
// restrict mode // TBD when IsDN comes in
//
///////////////////////////////////////////////////////////////////////////
//FECC
///////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetFECCMode(WORD mode, WORD direction)
{
	CComModeInfo comModeInfo(mode, (WORD)INDEX_START_OF_LSD);

    CapEnum ipDataCapCode = comModeInfo.GetH323ModeType();
    DWORD   ipFeccRate    = (mode == LSD_6400) ? 64 : 0; //NOTE: only 6.4 now!!

	m_pIPScm->SetFECCMode(ipDataCapCode, ipFeccRate, (cmCapDirection)direction);
}
///////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetFECCcap(WORD cap,WORD direction)
{
	m_pIPScm->SetDataBitRate(cap,(cmCapDirection)direction);
}

///////////////////////////////////////////////////////////////////////////
WORD CUnifiedComMode::GetFECCMode(WORD direction) const
{
	CapEnum lH323protocol = (CapEnum)(m_pIPScm->GetMediaType(cmCapData,(cmCapDirection)direction));
    CComModeInfo lComModeInfo = lH323protocol;
    WORD lH320protocol = lComModeInfo.GetH320ModeType();
	return lH320protocol;
}
///////////////////////////////////////////////////////////////////////////
WORD CUnifiedComMode::GetFECCcap(WORD direction) const
{
	WORD lDataRate = 0;
	lDataRate = m_pIPScm->GetMediaType(cmCapData, (cmCapDirection)direction);
	return lDataRate;
}
///////////////////////////////////////////////////////////////////////////
// Verify if  channels are opened
///////////////////////////////////////////////////////////////////////////
WORD CUnifiedComMode::IsMediaOn(WORD mediaType,WORD direction) const
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CUnifiedComMode::GetMediaBitRate(cmCapDataType type, cmCapDirection direction,ERoleLabel eRole) const
{
   if(CPObject::IsValidPObjectPtr(m_pIPScm) )
   		return m_pIPScm->GetMediaBitRate(type,direction,eRole);
   else
   {
   	 DBGPASSERT(1);
   	 return 0;
   }
}

///////////////////////////////////////////////////////////////////////////
// Content
///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
//Set the m_pIPScm with Content Capabilities
void CUnifiedComMode::SetContentMode(BYTE Xfer_XCallRate,eEnterpriseMode ContRatelevel ,cmCapDirection eDirection,
                                    ePresentationProtocol ContProtocol, eCascadeOptimizeResolutionEnum resolutionLevel,
                                    BYTE bContentAsVideo, eConfMediaType aConfMediaType, BOOL isHD1080, BYTE HDMpi)
{
	// Find Max content rate by call rate
	BYTE MaxContentRate = FindAMCContentRateByLevel(Xfer_XCallRate, aConfMediaType, ContRatelevel, ContProtocol, resolutionLevel);

	CapEnum H239Protocol = eH264CapCode;

	if(ContProtocol == eH263Fix)
		H239Protocol = eH263CapCode;
	else
	{
		if(ContProtocol == ePresentationAuto || ContProtocol ==  eH264Fix || ContProtocol == eH264Dynamic)
			H239Protocol = eH264CapCode;
		else
		{
			PASSERTSTREAM(TRUE, "ContProtocol " << (DWORD)ContProtocol << " is not as expected.");
			H239Protocol = eH264CapCode;
		}
	}

	if(MaxContentRate!=AMC_0k)
	{
		// Translate content rate(AMC_Xk) to 323 content rate.
	   	DWORD H323MaxContentRate = TranslateAMCRateIPRate(MaxContentRate);
	    PTRACE2INT(eLevelInfoNormal,"CUnifiedComMode::SetContentMode : Contet as video is  - ", bContentAsVideo);
	   	m_pIPScm->SetIsShowContentAsVideo(bContentAsVideo);
		m_pIPScm->SetContent(H323MaxContentRate,eDirection,H239Protocol,isHD1080,HDMpi,m_isHighProfileContent);


		m_pIsdnScm->m_contentMode.CreateMinimal(ContRatelevel);
		//m_pIsdnScm->SetContentModeContentRate(MaxContentRate);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////

void CUnifiedComMode::SetContentTIPMode( eEnterpriseMode ContRatelevel, cmCapDirection eDirection, BYTE set264ModeAsTipContent )
{
//	PTRACE("CUnifiedComMode::SetContentTIPMode");
	m_isTIPContent = TRUE;
	BYTE MaxContentRate = AMC_512k;
   	DWORD IpMaxContentRate = TranslateAMCRateIPRate(MaxContentRate);
	//m_pIPScm->SetIsShowContentAsVideo(bContentAsVideo);

   	if ( set264ModeAsTipContent == FALSE)
   		m_pIPScm->SetTIPContent(IpMaxContentRate, eDirection, FALSE);
   	else
   		m_pIPScm->SetTIPContent(IpMaxContentRate, eDirection);

	m_pIPScm->SetTipAuxFPS(eTipAux5FPS);
	m_pIsdnScm->m_contentMode.CreateMinimal(ContRatelevel);
}

/////////////////////////////////////////////////////////////////////////////
//HP content:
void CUnifiedComMode::SetIsHighProfileContent(const BYTE isHighProfileContent)
{
	m_isHighProfileContent = isHighProfileContent;
}

BYTE CUnifiedComMode::GetIsHighProfileContent() const
{
	return m_isHighProfileContent;
}

////////////////////////////////////////////////////////////////////////////////////////////
//Get the max content Rate allowed by CONF - RMX max content caps
BYTE CUnifiedComMode::GetContentModeAMC(BYTE Xfer_XlConfRate,eEnterpriseMode ContRatelevel,
										ePresentationProtocol contentProtocolMode,
										eCascadeOptimizeResolutionEnum resolutionLevel,
										eConfMediaType aConfMediaType)
{
	BYTE MaxContentRate;
	if( m_isTIPContent )
		MaxContentRate = AMC_512k;
	else
		MaxContentRate = FindAMCContentRateByLevel(Xfer_XlConfRate, aConfMediaType, ContRatelevel, contentProtocolMode, resolutionLevel);

	return MaxContentRate;
}
////////////////////////////////////////////////////////////////////////////////////////////
DWORD CUnifiedComMode::GetContentModeAMCInIPRate(BYTE Xfer_XlConfRate,eEnterpriseMode ContRatelevel,
													ePresentationProtocol contentProtocolMode,
													eCascadeOptimizeResolutionEnum resolutionLevel,
													eConfMediaType aConfMediaType)
{
	DWORD H323MaxContentRate = 0;
	// Find Max content rate by call rate
	BYTE MaxContentRate = GetContentModeAMC( Xfer_XlConfRate, ContRatelevel, contentProtocolMode, resolutionLevel, aConfMediaType);//MaxContentRate = FindAMCContentRateByLevel(Xfer_XlConfRate,ContRatelevel);
	if( MaxContentRate!=AMC_0k )
	{
		// Translate content rate(AMC_Xk) to 323 content rate.
		H323MaxContentRate = TranslateAMCRateIPRate(MaxContentRate);
	}
	return H323MaxContentRate;

}
////////////////////////////////////////////////////////////////////////////////////////////
//Set the current content Bit Rate
void CUnifiedComMode::SetNewContentBitRate(BYTE newAMCBitRate,cmCapDirection eDirection)
{
	if( m_isTIPContent )
		PTRACE(eLevelInfoNormal, "CUnifiedComMode::SetNewContentBitRate : WARNING - TIP content mode is active!");
	// Translate content rate(AMC_Xk) to 323 content rate.
	DWORD H323ContentRate = TranslateAMCRateIPRate(newAMCBitRate);
	m_pIPScm->SetContentBitRate(H323ContentRate,eDirection);

	// in Isdn we use content rate in AMSC values
	BYTE  IsdnContentRate = TranslateAmcRateToAmscRate(newAMCBitRate);
	m_pIsdnScm->SetContentModeContentRate(IsdnContentRate);
}

///////////////////////////////////////////////////////////////////////////
//Get the current content Bit Rate
BYTE CUnifiedComMode::GetCurrentContentBitRateAMC()
{
	DWORD H323ContentRate = (m_pIPScm->GetContentBitRate(cmCapReceive));
	BYTE  H239Rate = TranslateRateToAMCRate(H323ContentRate);
	BYTE  IsdnAmscRate = m_pIsdnScm->GetContentModeContentRate();
	BYTE  IsdnRate = TranslateAmscRateToAmcRate(IsdnAmscRate);
	/*
	 * Eitan - temp for debug
	CMedString cstr;
	cstr << "CUnifiedComMode::GetCurrentContentBitRateAMC\n"
		 << "CurrentContentBitRate(from IP scm) : " << H239Rate
		 << "\nCurrentContentBitRate(from ISDN scm) : " << IsdnRate;

	PTRACE(eLevelInfoNormal,cstr.GetString());
	*/
	if (IsdnRate != H239Rate)
	{
		PTRACE(eLevelError,"AMC RATES ARE DIFFERENT!!!");
		PASSERT(1);
	}

	//FSN-613: Dynamic Content for SVC/Mix Conf test
	PTRACE2INT(eLevelInfoNormal,"CUnifiedComMode::GetCurrentContentBitRateAMC : rate  - ", (int)H239Rate);
	return H239Rate;
}
////////////////////////////////////////////////////////////////////////////////////////////
//Set the current content Protocol
void CUnifiedComMode::SetNewContentProtocol(BYTE newProtocol,cmCapDirection eDirection,BOOL isHD1080,BYTE HDContentMpi)
{
	if( m_isTIPContent )
	{
		PTRACE(eLevelInfoNormal, "CUnifiedComMode::SetNewContentProtocol : WARNING - TIP content mode is active!");
		//TODO return?
	}
	//Set the m_pIPScm with Content Capabilities and the max content rate
	CapEnum lProtocol = (CapEnum)m_pIPScm->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	/*if(lProtocol == newProtocol)
		return;*/


	CapEnum H323Protocol = TranslateConfProtocolToH323Protocol(newProtocol);
	DWORD contentRate = m_pIPScm->GetContentBitRate(cmCapTransmit);
	m_pIPScm->RemoveContent(cmCapReceiveAndTransmit);
	m_pIPScm->SetContent(contentRate, cmCapReceiveAndTransmit, H323Protocol,isHD1080,HDContentMpi,m_isHighProfileContent);

}
///////////////////////////////////////////////////////////////////////////
CapEnum CUnifiedComMode::TranslateConfProtocolToH323Protocol(BYTE Protocol)
{
	switch (Protocol)
	{
		case H264:
		{
			return eH264CapCode;
		}
		case H263:
		{
			return eH263CapCode;
		}
		case H261:
		{
			return eH261CapCode;
		}
		default:
		{
			DBGPASSERT(Protocol);
			return eH263CapCode;

		}
	}
}

///////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::GetCurrentContentProtocolInIsdnValues()
{
	CapEnum Conf239Protocol =  ((CapEnum)m_pIPScm->GetMediaType(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation));

	CComModeInfo cmInfo = Conf239Protocol;
	BYTE ProtocolInReservationTerms = cmInfo.GetH320ModeType();

	return ProtocolInReservationTerms;

}

///////////////////////////////////////////////////////////////////////////
CapEnum CUnifiedComMode::GetCurrentContentProtocolInIpValues()
{
	CapEnum Conf239Protocol =  ((CapEnum)m_pIPScm->GetMediaType(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation));

	return Conf239Protocol;
}
///////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::TranslateRateToAMCRate(DWORD h323Rate)
{
	DWORD Rate = h323Rate/10;

	if(Rate<40*BCH_FACTOR/100) return (BYTE)AMC_0k;
	else if (Rate>=40*BCH_FACTOR/100 && Rate<64*BCH_FACTOR/100) return (BYTE)AMC_40k;
	else if (Rate>=64*BCH_FACTOR/100 && Rate<96*BCH_FACTOR/100) return (BYTE)AMC_64k;
	else if (Rate>=96*BCH_FACTOR/100 && Rate<128*BCH_FACTOR/100) return (BYTE)AMC_96k;
	else if (Rate>=128*BCH_FACTOR/100 && Rate<192*BCH_FACTOR/100) return (BYTE)AMC_128k;
	else if (Rate>=192*BCH_FACTOR/100 && Rate<256*BCH_FACTOR/100) return (BYTE)AMC_192k;
	else if (Rate>=256*BCH_FACTOR/100 && Rate<384*BCH_FACTOR/100) return (BYTE)AMC_256k;
	else if (Rate>=384*BCH_FACTOR/100 && Rate<512*BCH_FACTOR/100) return (BYTE)AMC_384k;
	else if (Rate>=512*BCH_FACTOR/100 && Rate<768*BCH_FACTOR/100) return (BYTE)AMC_512k;
	else if (Rate>=768*BCH_FACTOR/100 && Rate<1024*BCH_FACTOR/100) return (BYTE)AMC_768k;
	else if (Rate>=1024*BCH_FACTOR/100 && Rate<1152*BCH_FACTOR/100) return (BYTE)AMC_1024k;
	else if (Rate>=1152*BCH_FACTOR/100 && Rate<1280*BCH_FACTOR/100) return (BYTE)AMC_1152k;
	else if (Rate>=1280*BCH_FACTOR/100 && Rate<1536*BCH_FACTOR/100) return (BYTE)AMC_1280k;
	else if (Rate>=1536*BCH_FACTOR/100 && Rate<2048*BCH_FACTOR/100) return (BYTE)AMC_1536k;
	else if (Rate>=2048*BCH_FACTOR/100 && Rate<2560*BCH_FACTOR/100) return (BYTE)AMC_2048k;
	else if (Rate>=2560*BCH_FACTOR/100 && Rate<3072*BCH_FACTOR/100) return (BYTE)AMC_2560k;
	else if (Rate>=3072*BCH_FACTOR/100 && Rate<4096*BCH_FACTOR/100) return (BYTE)AMC_3072k;
	else if (Rate>=4096*BCH_FACTOR/100) return (BYTE)AMC_4096k;
    return 255;
}
///////////////////////////////////////////////////////////////////////////
DWORD CUnifiedComMode::TranslateAMCRateIPRate(BYTE AMCRate) // static
{
	BYTE units = 10;
	switch(AMCRate)
	{
		case AMC_0k:
		{
			return 0 ;
		}
		case AMC_40k:
		{
			return (40 * units) ;
		}
		case AMC_64k:
		{
			return (64 * units);
		}
		case AMC_96k:
		{
			return (96 * units);
		}
		case AMC_128k:
		{
			return (128 * units);
		}
		case AMC_192k:
		{
			return (192 * units);
		}
		case AMC_256k:
		{
			return (256 * units);
		}
		case AMC_384k:
		{
			return (384 * units);
		}
		case AMC_512k:
		{
			return (512 * units);
		}
		case AMC_768k:
		{
			return (768 * units);
		}
		case AMC_1024k:
		{
			return (1024 * units);
		}
		case AMC_1152k:
		{
			return (1152 * units);
		}
		case AMC_1280k:
		{
			return (1280 * units);
		}
		case AMC_1536k:
		{
			return (1536 * units);
		}
		case AMC_2048k:
		{
			return (2048 * units);
		}
		case AMC_2560k:
		{
			return (2560 * units);
		}
		case AMC_3072k:
		{
			return (3072 * units);
		}
		case AMC_4096k:
		{
			return (4096 * units);
		}
		default :
		{
			DBGFPASSERT(AMCRate);
			return AMC_0k;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////
DWORD CUnifiedComMode::TranslateXferRateToIpRate(BYTE XCallRate) // static
{
	return static_cast<DWORD>(CCapSetInfo::TranslateReservationRateToIpRate(XCallRate));
}
/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::TranslateXferRateToAmcRate(BYTE xferRate) // static
{
	switch (xferRate)
	{
	case Xfer_64: return AMC_64k;
	case Xfer_96: return AMC_96k;
	case Xfer_128: return AMC_128k;
	case Xfer_192: return AMC_192k;
	case Xfer_256: return AMC_256k;
	case Xfer_384: return AMC_384k;
	case Xfer_512: return AMC_512k;
	case Xfer_768: return AMC_768k;
	case Xfer_1024: return AMC_1024k;
	case Xfer_1152: return AMC_1152k;
	case Xfer_1280: return AMC_1280k;
	case Xfer_1536: return AMC_1536k;
	case Xfer_2048: return AMC_2048k;
	case Xfer_2560: return AMC_2560k;
	case Xfer_3072: return AMC_3072k;
	case Xfer_4096: return AMC_4096k;

	case AUTO: return AMC_0k; // dummy case, just to prevent assert

	default:
		DBGFPASSERT(xferRate);
		return AMC_0k;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::TranslateAmcRateToXferRate(BYTE AMCRate)
{
	switch (AMCRate)
	{
	case AMC_64k: return Xfer_64;
	case AMC_96k: return Xfer_96;
	case AMC_128k: return Xfer_128;
	case AMC_192k: return Xfer_192;
	case AMC_256k: return Xfer_256;
	case AMC_384k: return Xfer_384;
	case AMC_512k: return Xfer_512;
	case AMC_768k: return Xfer_768;
	case AMC_1024k: return Xfer_1024;
	case AMC_1152k: return Xfer_1152;
	case AMC_1280k: return Xfer_1280;
	case AMC_1536k: return Xfer_1536;
	case AMC_2048k: return Xfer_2048;
	case AMC_2560k: return Xfer_2560;
	case AMC_3072k: return Xfer_3072;
	case AMC_4096k: return Xfer_4096;

	default:
		DBGFPASSERT(AMCRate);
		return Xfer_64;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////
DWORD CUnifiedComMode::FindIpContentRateByLevel(DWORD CallRate, eEnterpriseMode ContentRateLevel, ePresentationProtocol contentProtocolMode,
                                                eCascadeOptimizeResolutionEnum resolutionLevel, eConfMediaType aConfMediaType)
{

	TRACEINTO << "N.A. DEBUG CUnifiedComMode::FindIpContentRateByLevel CallRate: " << CallRate << "ContentRateLevel: " << ContentRateLevel << "contentProtocolMode: "
			 << contentProtocolMode << "resolutionLevel: " << resolutionLevel <<"aConfMediaType: " << aConfMediaType ;
	BYTE AMCCallRate = TranslateIPRateToXferRate(CallRate);
	TRACEINTO << "AMCCallRate: "<< (int)AMCCallRate;
	BYTE SelectedAMCRate = FindAMCContentRateByLevel(AMCCallRate, aConfMediaType, ContentRateLevel, contentProtocolMode, resolutionLevel);
	TRACEINTO << "SelectedAMCRate: "<< (int)SelectedAMCRate;
	DWORD ans = TranslateAMCRateIPRate(SelectedAMCRate)*100;
	TRACEINTO << "ans: "<< ans;

	return ans;
}
/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::FindAMCContentRateByLevel(BYTE Xfer_XCallRate,
                                                eConfMediaType aConfMediaType,
                                                eEnterpriseMode ContentRateLevel,
												ePresentationProtocol contentProtocolMode,
												eCascadeOptimizeResolutionEnum resolutionLevel)
{
	if (ContentRateLevel == eCustomizedRate)
		return m_amcCustomizedContentRate;

	// support rate for specific customers request
	BOOL bEnableContentOf768 = NO;
	std::string key = "ENABLE_CONTENT_OF_768_FOR_1024_LV";
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, bEnableContentOf768);
	BYTE ContentRate = AMC_0k;

	if (ContentRateLevel == eLiveVideo && bEnableContentOf768 && Xfer_XCallRate == Xfer_1024)
	{
		BYTE XferRate = TranslateIPRateToXferRate(768 * 1000);
		ContentRate = TranslateXferRateToAmcRate(XferRate);

		TRACEINTO << "ENABLE_CONTENT_OF_768_FOR_1024_LV is yes, Content ACM Rate is: " << (int)ContentRate;
		return ContentRate;
	}
	TRACEINTO << "CUnifiedComMode::FindAMCContentRateByLevel contentProtocolMode == eH264Fix Xfer_XCallRate: " << (int)Xfer_XCallRate << " ContentRateLevel: " << ContentRateLevel
				<< " resolutionLevel: "<< resolutionLevel <<" resolutionLevel: "<< resolutionLevel << " contentProtocolMode: " <<contentProtocolMode << " aConfMediaType: " << aConfMediaType;

	if (contentProtocolMode == eH264Fix)
	{

		return FindAMCH264DynamicContentRateByLevel(Xfer_XCallRate, ContentRateLevel, resolutionLevel, aConfMediaType);
	}

	/****************************   ContentRateControl Table:  *******************************************
	______________________________________________________________________________________________________
	|Conf bps |64/96 | 128| 256  | 384 | 512| 768| 832|	 1024| 1152| 1472| 1728| 1920| 2048 | 4096 | 6144 |
	|_________|______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|_____________________________________________________________________________________________________|
	|Graphics |0     |64  |  64  | 128 | 128| 256| 256|	 256 | 384 | 384 | 512 | 512 | 512  | 1280 | 2048 |
	|_________|______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|Hi Res   |0  	 |64  | 128  | 192 | 256| 384| 384|  512 | 512 | 512 | 768 | 768 | 1024 | 2048 | 3072 |
	|Graphics |      |    |      |     |    |    |    |	     |     |     |     |     |      |      |      |
	|_________|______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|LiveVideo|0     |64  | 128  | 256 | 384| 512| 512|	 512 | 768 | 768 | 1152|1280 | 1280 | 2560 | 4096 |
	|_________|______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|

     Graphics - the old division of content rate

	******************************************************************************************************/
//what about rate 192-for now we will use it the same as 128 - noa R .
	
	//The highest common table have below differences values for SVC enabled conference.
	if (eSvcOnly == aConfMediaType || eMixAvcSvc == aConfMediaType || eMixAvcSvcVsw == aConfMediaType)
	{
		if(ContentRateLevel == eLiveVideo)
		{
			switch(Xfer_XCallRate)
			{
				case Xfer_512:
					if(m_isHighProfileContent)
						ContentRate = AMC_256k;
					break;
				case Xfer_1152:
				case Xfer_1280:	
					ContentRate = AMC_512k;
					break;
				case Xfer_1536:	
					ContentRate = AMC_768k;
					break;
			}

			if(ContentRate != AMC_0k)
				return ContentRate;
		}
	}		

	switch(Xfer_XCallRate)
	{
	case Xfer_64:
	case Xfer_96:
	{
		ContentRate = AMC_0k;
		break;
	}
	case Xfer_128:
	case Xfer_2x64:
	case Xfer_192:
	{
		ContentRate = AMC_64k;
		break;
	}
	case Xfer_256:
	case Xfer_4x64:
	case Xfer_320:
	{
		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_64k;
		else
			ContentRate = AMC_128k;
		break;
	}
	case Xfer_384:
	case Xfer_6x64:
	{
		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_128k;
		else
		{
			if(ContentRateLevel == eHiResGraphics)
				ContentRate = AMC_192k;
			else
				ContentRate = AMC_256k;
		}
		break;
	}
	case Xfer_512:
	{
		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_128k;
		else
		{
			if(ContentRateLevel == eHiResGraphics)
				ContentRate = AMC_256k;
			else
				ContentRate = AMC_384k;
		}
		break;
	}
	case Xfer_768:
	case Xfer_832:
	{
		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_256k;
		else
		{
			if(ContentRateLevel == eHiResGraphics)
				ContentRate = AMC_384k;
			else
				ContentRate = AMC_512k;
		}
		break;
	}
	case Xfer_1024:
	{
		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_256k;
		else
			ContentRate = AMC_512k;
		break;
	}
	case Xfer_1152:
	case Xfer_1280:
	case Xfer_1472:
	{
		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_384k;
		else
		{
			if(ContentRateLevel == eHiResGraphics)
				ContentRate = AMC_512k;
			else
				ContentRate = AMC_768k;
		}
		break;
	}
	case Xfer_1536:
	case Xfer_1728:
	{
		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_512k;
		else
		{
			if(ContentRateLevel == eHiResGraphics)
				ContentRate = AMC_768k;
			else
				ContentRate = AMC_1024k;
		}
		break;
	}
	case Xfer_1920:
	case Xfer_2048:
	{

		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_512k;
		else
		{
			if(ContentRateLevel == eHiResGraphics)
				ContentRate = AMC_1024k;
			else
				ContentRate = AMC_1280k;
		}
		break;
	}
	case Xfer_2560:
	{
		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_768k;
		else
		{
			if(ContentRateLevel == eHiResGraphics)
				ContentRate = AMC_1280k;
			else
				ContentRate = AMC_1536k;
		}
		break;
	}
	case Xfer_3072:
	case Xfer_3584:
	{
		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_768k;
		else
		{
			if(ContentRateLevel == eHiResGraphics)
				ContentRate = AMC_1536k;
			else
				ContentRate = AMC_2048k;
		}
		break;
	}
	case Xfer_4096:
	{
		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_1280k;
		else
		{
			if(ContentRateLevel == eHiResGraphics)
				ContentRate =  AMC_2048k;
			else
				ContentRate = AMC_2560k;
		}
		break;
	}
	case Xfer_6144:
	case Xfer_8192:
	{
		if(ContentRateLevel == eGraphics)
			ContentRate = AMC_2048k;
		else
		{
			if(ContentRateLevel == eHiResGraphics)
				ContentRate =  AMC_3072k;
			else
				ContentRate = AMC_4096k;
		}
		break;
	}
	default:
	{
		TRACEINTO << "Undefined Content Rate - " << Xfer_XCallRate;
		break;
	}
	}

	if(!IsFeatureSupportedBySystem(eFeatureHD1080p30Content))
	{
		if(ContentRate > MAX_AMC_FOR_1080P15_ONLY)
			ContentRate = MAX_AMC_FOR_1080P15_ONLY;
	}
	else if(!IsFeatureSupportedBySystem(eFeatureHD1080p60Content))
	{
		if(ContentRate > MAX_AMC_FOR_1080P30_ONLY)
			ContentRate = MAX_AMC_FOR_1080P30_ONLY;
	}

	PTRACE2INT(eLevelInfoNormal,"CUnifiedComMode::FindAMCContentRateByLevel : Chosen rate - ",(int)ContentRate);
	return ContentRate;

}
///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::TranslateIPRateToXferRate(DWORD H323CallRate) // static
{
	if (H323CallRate==0)
		return 0xFF;

	if (H323CallRate <= 64000) return Xfer_64;
	else if (H323CallRate <=96000)   return ( (H323CallRate-64000)   <= ( (96000-64000)/2 ))     ? Xfer_64 : Xfer_96;
	else if (H323CallRate <=128000)  return ( (H323CallRate-96000)   <= ( (128000-96000)/2 ))    ? Xfer_96 : Xfer_128;
	else if (H323CallRate <=256000)  return ( (H323CallRate-128000)  <= ( (256000-128000)/2 ))   ? Xfer_128: Xfer_256;
	else if (H323CallRate <=384000)  return ( (H323CallRate-256000)  <= ( (384000-256000)/2 ))   ? Xfer_256: Xfer_384;
	else if (H323CallRate <=512000)  return ( (H323CallRate-384000)  <= ( (512000-384000)/2 ))   ? Xfer_384: Xfer_512;
	else if (H323CallRate <=768000)  return ( (H323CallRate-512000)  <= ( (768000-512000)/2 ))   ? Xfer_512: Xfer_768;
	else if (H323CallRate <=832000)  return ( (H323CallRate-768000)  <= ( (832000-768000)/2 ))   ? Xfer_768: Xfer_832;
	else if (H323CallRate <=1024000) return ( (H323CallRate-832000)  <= ( (1024000-832000)/2 ))  ? Xfer_832: Xfer_1024;
	else if (H323CallRate <=1152000) return ( (H323CallRate-1024000) <= ( (1152000-1024000)/2 )) ? Xfer_1024: Xfer_1152;
	else if (H323CallRate <=1280000) return ( (H323CallRate-1152000) <= ( (1280000-1152000)/2 )) ? Xfer_1152: Xfer_1280;
	else if (H323CallRate <=1472000) return ( (H323CallRate-1280000) <= ( (1472000-1280000)/2 )) ? Xfer_1280: Xfer_1472;
	else if (H323CallRate <=1536000) return ( (H323CallRate-1472000) <= ( (1536000-1472000)/2 )) ? Xfer_1472: Xfer_1536;
	else if (H323CallRate <=1728000) return ( (H323CallRate-1536000) <= ( (1728000-1536000)/2 )) ? Xfer_1536: Xfer_1728;
	else if (H323CallRate <=1920000) return ( (H323CallRate-1728000) <= ( (1920000-1728000)/2 )) ? Xfer_1728: Xfer_1920;
	else if (H323CallRate <=2048000) return ( (H323CallRate-1920000) <= ( (2048000-1920000)/2 )) ? Xfer_1920: Xfer_2048;
	else if (H323CallRate <=2560000) return ( (H323CallRate-2048000) <= ( (2560000-2048000)/2 )) ? Xfer_2048: Xfer_2560;
	else if (H323CallRate <=3072000) return ( (H323CallRate-2560000) <= ( (3072000-2560000)/2 )) ? Xfer_2560: Xfer_3072;
	else if (H323CallRate <=4096000) return ( (H323CallRate-3072000) <= ( (4096000-3072000)/2 )) ? Xfer_3072: Xfer_4096;
	else if (H323CallRate <=6144000) return ( (H323CallRate-4096000) <= ( (6144000-4096000)/2 )) ? Xfer_4096: Xfer_6144;
	else if (H323CallRate <=8192000) return ( (H323CallRate-6144000) <= ( (8192000-6144000)/2 )) ? Xfer_6144: Xfer_8192;

	return 0xFF;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::TranslateIPRateToXferRateForCss(DWORD H323CallRate) // static
{
	if (H323CallRate==0)
		return 0xFF;
	
	if (H323CallRate <= 64000) return Xfer_64;
	else if (H323CallRate <=96000)   return Xfer_64 ;
	else if (H323CallRate <=128000)  return Xfer_96;
	else if (H323CallRate <=192000)   return Xfer_128;
	else if (H323CallRate <=256000)   return Xfer_192;
	else if (H323CallRate <=384000)  return Xfer_256;
	else if (H323CallRate <=512000)  return Xfer_384;
	else if (H323CallRate <=768000)  return   Xfer_512;
	else if (H323CallRate <=1024000) return Xfer_768;
	else if (H323CallRate <=1152000) return Xfer_1024;
	else if (H323CallRate <=1280000) return Xfer_1152;
	else if (H323CallRate <=1536000) return Xfer_1280;
	else if (H323CallRate <=2048000) return Xfer_1536;
	else if (H323CallRate <=2560000) return Xfer_2048;
	else if (H323CallRate <=3072000) return Xfer_2560;
	else if (H323CallRate <=4096000) return Xfer_3072;
	else if (H323CallRate <=6192000) return Xfer_4096;
	return 0xFF;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::TranslateAmcRateToAmscRate(BYTE AmcRate) // static
{
	BYTE res = AMSC_0k;

	if (AmcRate < MAX_H239_CAPS)
	{
		res = g_239ToEPCTbl[AmcRate].AmscRate;

		if (res == (BYTE)NA)
		{
			FPASSERT(1);
			PTRACE1(eLevelInfoNormal, "CUnifiedComMode::TranslateAmcRateToAmscRate AMSC rate is not valid setting to AMSC_0k");
			res = AMSC_0k;
		}
	//	if(bIsRestricted)
	//		res = res * 7 / 8; //In restriced call the content rate is 7/8 from the total rate.
	}
	return res;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::TranslateAmscRateToAmcRate(BYTE AMSCRate)
{
	BYTE res = AMC_0k;

	for (int i=0; i<MAX_H239_CAPS; i++)
	{
		if (g_239ToEPCTbl[i].AmscRate == AMSCRate)
		{
			res = g_239ToEPCTbl[i].AmcRate;
			break;
		}
	}

	return res;

}
////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CUnifiedComMode::TranslateAmscRateToIPRate(BYTE AMSCRate)
{
	DWORD IPRate =0;
	IPRate = TranslateAmscRateToAmcRate(AMSCRate);
	IPRate = TranslateAMCRateIPRate(IPRate)*100;
	return IPRate;
}
////////////////////////////////////////////////////////////////////////////////////////////////
//BRIDGE-9899: Some value of xfer rate was not defined in the DynamicContentTable.
//TBD: Add those values to DynamicContentTable will have interworking issues or not???
BYTE CUnifiedComMode::MapXferRateForDynamicContentRateTable(BYTE xferRate)
{
	switch(xferRate)
	{
	case Xfer_192:
		return Xfer_128;
	case Xfer_320:
		return Xfer_256;
	default:
		return xferRate;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetIsLpr(BYTE mode)
{
	m_pIPScm->SetIsLpr(mode);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::GetIsLpr() const
{
	return(m_pIPScm->GetIsLpr());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetHdVswResolution(EHDResolution hdVswRes)
{
	m_pIPScm->SetHdVswResolution(hdVswRes);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
CCopVideoTxModes* CUnifiedComMode::GetCopVideoTxModes() const
{
	return m_pCopVideoTxModes;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::SetCopVideoTxModes(CCopVideoTxModes* pCopVideoTxModes)
{
	POBJDELETE(m_pCopVideoTxModes);
	m_pCopVideoTxModes = new CCopVideoTxModes(*pCopVideoTxModes);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::isHDContent1080Supported(cmCapDirection direction) const
{
	BYTE HDContent1080Mpi = 0;
	 HDContent1080Mpi = m_pIPScm->isHDContent1080Supported(direction);
	return HDContent1080Mpi;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::isHDContent720Supported(cmCapDirection direction) const
{
	BYTE HDContent720Mpi = 0;
	 HDContent720Mpi = m_pIPScm->isHDContent720Supported(direction);
	return HDContent720Mpi;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::initStaticContentTables()
{
	if(!m_initContentTableFlag)
	{
		m_initContentTableFlag = TRUE;
		initAMCContentRateCascdeOptimizeResTable();
		initMaxResolutionPerContentRateTable();
		initAMCContentRateCascdeOptimizeResTable_HP();
		initMaxResolutionPerContentRateTable_HP();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::initAMCContentRateCascdeOptimizeResTable()
{
	/****************************   ContentRateControl Table:  *******************************************
	_______________________________________________________________________________________________________________________
	|Conf bps |Resolution	 | 64/96 | 128| 256  | 384 | 512| 768| 832|	 1024| 1152| 1472| 1728| 1920| 2048 | 4096 | 6144 |
	|_________|______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|_____________________________________________________________________________________________________________________|
	|		  |	720/5fps	 |NA     |64  |  64  | 128 | 128| 256| 256|	256  | 256 | 256 | 256 | 256 | 512  | 512  | 512  |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|Graphics |	720/30fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | 512 | 512 | 512 | 512  | 512  | 768  |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 15fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | NA  | NA  | 768 | 768  | 1152 | 1152 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 30fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | NA  | NA  | NA  | NA   | NA   | 2048 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 60fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | NA  | NA  | NA  | NA   | NA   | NA   |
	|_________|______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|_____________________________________________________________________________________________________________________|
	|		  |	720/5fps	 |NA     |64  |  128 | 192 | 256| 384| 384|	384  | 384 | 512 | 512 | 512 | 512  | 512  | 512  |
	| Hi-res  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|Graphics |	720/30fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | 512 | 512 | 512 | 512 | 768  | 768  | 768  |
	|	50%	  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 15fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | 768 | 768 | 768 | 768  | 1152 | 1152 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 30fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | NA  | NA  | NA  | NA   | 2048 | 3072 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 60fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | NA  | NA  | NA  | NA   | NA   | 3072 |
	|_________|______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|_____________________________________________________________________________________________________________________|
	|		  |	720/5fps	 |NA     |64  |  128 | 256 | 384| 512| 512|	768  | 768 | 768 | 768 | 768 | 768  | 768  | 768  |
	|  Live   |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|  Video  |	720/30fps	 |NA     |NA  |  NA  | NA  | NA | NA | 512|	512  | 768 | 768 | 768 | 768 | 768  | 768  | 768  |
	|	66%	  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 15fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | 768 | 768 | 768 | 768 | 1152 | 1152 | 1152 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 30fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA | NA   |	NA |  NA |  NA |  NA |  NA  | 2560 | 3072 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 60fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA | NA   |	NA |  NA |  NA |  NA |  NA  | NA   | 4096 |
	|_________|______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|

     Graphics - the old division of content rate

	******************************************************************************************************/
//what about rate 192-for now we will use it the same as 128 - noa R .
// and for now we use 1536 as the same as 1472 - Cao Yiping

	if (!m_amcH264DynamicContentRateResTable.empty())
		return;

	m_amcH264DynamicContentRateResTable[Xfer_128][eGraphics][e_res_720_5fps]  = AMC_64k ;
	m_amcH264DynamicContentRateResTable[Xfer_256][eGraphics][e_res_720_5fps]  = AMC_64k ;
	m_amcH264DynamicContentRateResTable[Xfer_384][eGraphics][e_res_720_5fps]  = AMC_128k ;
	m_amcH264DynamicContentRateResTable[Xfer_512][eGraphics][e_res_720_5fps]  = AMC_128k ;
	m_amcH264DynamicContentRateResTable[Xfer_768][eGraphics][e_res_720_5fps]  = AMC_256k ;
	m_amcH264DynamicContentRateResTable[Xfer_832][eGraphics][e_res_720_5fps]  = AMC_256k ;
	m_amcH264DynamicContentRateResTable[Xfer_1024][eGraphics][e_res_720_5fps] = AMC_256k ;
	m_amcH264DynamicContentRateResTable[Xfer_1152][eGraphics][e_res_720_5fps] = AMC_256k ;
	m_amcH264DynamicContentRateResTable[Xfer_1280][eGraphics][e_res_720_5fps] = AMC_256k ;
	m_amcH264DynamicContentRateResTable[Xfer_1472][eGraphics][e_res_720_5fps] = AMC_256k ;
	m_amcH264DynamicContentRateResTable[Xfer_1536][eGraphics][e_res_720_5fps] = AMC_256k ;
	m_amcH264DynamicContentRateResTable[Xfer_1728][eGraphics][e_res_720_5fps] = AMC_256k ;
	m_amcH264DynamicContentRateResTable[Xfer_1920][eGraphics][e_res_720_5fps] = AMC_256k ;
	m_amcH264DynamicContentRateResTable[Xfer_2048][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_2560][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_3072][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_3584][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_4096][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_6144][eGraphics][e_res_720_5fps] = AMC_512k ;

	/*m_amcH264DynamicContentRateResTable[Xfer_64][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_96][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_128][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_256][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_384][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_512][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_768][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_832][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1024][eGraphics][e_res_720_30fps] = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1152][eGraphics][e_res_720_30fps] = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1280][eGraphics][e_res_720_30fps] = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable[Xfer_1472][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1536][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1728][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1920][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_2048][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_2560][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_3072][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_3584][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_4096][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_6144][eGraphics][e_res_720_30fps] = AMC_768k ;

	/*m_amcH264DynamicContentRateResTable[Xfer_64][eGraphics][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_96][eGraphics][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_128][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_256][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_384][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_512][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_768][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_832][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1024][eGraphics][e_res_1080_15fps] = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1152][eGraphics][e_res_1080_15fps] = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1280][eGraphics][e_res_1080_15fps] = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1472][eGraphics][e_res_1080_15fps] = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1536][eGraphics][e_res_1080_15fps] = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1728][eGraphics][e_res_1080_15fps] = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable[Xfer_1920][eGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_2048][eGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_2560][eGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_3072][eGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_3584][eGraphics][e_res_1080_15fps] = AMC_1152k ;
	m_amcH264DynamicContentRateResTable[Xfer_4096][eGraphics][e_res_1080_15fps] = AMC_1152k ;
	m_amcH264DynamicContentRateResTable[Xfer_6144][eGraphics][e_res_1080_15fps] = AMC_1152k ;

	/*m_amcH264DynamicContentRateResTable[Xfer_64][eHiResGraphics][e_res_720_5fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_96][eHiResGraphics][e_res_720_5fps]  = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable[Xfer_128][eHiResGraphics][e_res_720_5fps] = AMC_64k ;	// for BRIDGE-6601 - Anat L. request
	m_amcH264DynamicContentRateResTable[Xfer_256][eHiResGraphics][e_res_720_5fps]  = AMC_128k ;	// for BRIDGE-6601 - Anat L. request
	m_amcH264DynamicContentRateResTable[Xfer_384][eHiResGraphics][e_res_720_5fps]  = AMC_192k ;
	m_amcH264DynamicContentRateResTable[Xfer_512][eHiResGraphics][e_res_720_5fps]  = AMC_256k ;
	m_amcH264DynamicContentRateResTable[Xfer_768][eHiResGraphics][e_res_720_5fps]  = AMC_384k ;
	m_amcH264DynamicContentRateResTable[Xfer_832][eHiResGraphics][e_res_720_5fps]  = AMC_384k ;
	m_amcH264DynamicContentRateResTable[Xfer_1024][eHiResGraphics][e_res_720_5fps] = AMC_384k ;
	m_amcH264DynamicContentRateResTable[Xfer_1152][eHiResGraphics][e_res_720_5fps] = AMC_384k ;
	m_amcH264DynamicContentRateResTable[Xfer_1280][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1472][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1536][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1728][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1920][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_2048][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_2560][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_3072][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_3584][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_4096][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_6144][eHiResGraphics][e_res_720_5fps] = AMC_512k ;

	/*m_amcH264DynamicContentRateResTable[Xfer_64][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_96][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_128][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_256][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_384][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_512][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_768][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_832][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1024][eHiResGraphics][e_res_720_30fps] = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable[Xfer_1152][eHiResGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1280][eHiResGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1472][eHiResGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1536][eHiResGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1728][eHiResGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1920][eHiResGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_2048][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_2560][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_3072][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_3584][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_4096][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_6144][eHiResGraphics][e_res_720_30fps] = AMC_768k ;

	/*m_amcH264DynamicContentRateResTable[Xfer_64][eHiResGraphics][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_96][eHiResGraphics][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_128][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_256][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_384][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_512][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_768][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_832][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1024][eHiResGraphics][e_res_1080_15fps] = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1152][eHiResGraphics][e_res_1080_15fps] = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable[Xfer_1472][eHiResGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1728][eHiResGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1920][eHiResGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_2048][eHiResGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_2560][eHiResGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_3072][eHiResGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_3584][eHiResGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_4096][eHiResGraphics][e_res_1080_15fps] = AMC_1152k ;
	m_amcH264DynamicContentRateResTable[Xfer_6144][eHiResGraphics][e_res_1080_15fps] = AMC_1152k ;


	/*m_amcH264DynamicContentRateResTable[Xfer_64][eLiveVideo][e_res_720_5fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_96][eLiveVideo][e_res_720_5fps]   = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable[Xfer_128][eLiveVideo][e_res_720_5fps]  = AMC_64k ; //BRIDGE-9867
	m_amcH264DynamicContentRateResTable[Xfer_256][eLiveVideo][e_res_720_5fps]  = AMC_128k ;//BRIDGE-9867
	m_amcH264DynamicContentRateResTable[Xfer_384][eLiveVideo][e_res_720_5fps]  = AMC_256k ;
	m_amcH264DynamicContentRateResTable[Xfer_512][eLiveVideo][e_res_720_5fps]  = AMC_384k ;
	m_amcH264DynamicContentRateResTable[Xfer_768][eLiveVideo][e_res_720_5fps]  = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_832][eLiveVideo][e_res_720_5fps]  = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1024][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1152][eLiveVideo][e_res_720_5fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1280][eLiveVideo][e_res_720_5fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1472][eLiveVideo][e_res_720_5fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1536][eLiveVideo][e_res_720_5fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1728][eLiveVideo][e_res_720_5fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1920][eLiveVideo][e_res_720_5fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_2048][eLiveVideo][e_res_720_5fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_2560][eLiveVideo][e_res_720_5fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_3072][eLiveVideo][e_res_720_5fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_3584][eLiveVideo][e_res_720_5fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_4096][eLiveVideo][e_res_720_5fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_6144][eLiveVideo][e_res_720_5fps] = AMC_768k ;

	/*m_amcH264DynamicContentRateResTable[Xfer_64][eLiveVideo][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_96][eLiveVideo][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_128][eLiveVideo][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_256][eLiveVideo][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_384][eLiveVideo][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_512][eLiveVideo][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_768][eLiveVideo][e_res_720_30fps]  = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable[Xfer_832][eLiveVideo][e_res_720_30fps]  = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1024][eLiveVideo][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable[Xfer_1152][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1280][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1472][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1536][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1728][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1920][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_2048][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_2560][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_3072][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_3584][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_4096][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_6144][eLiveVideo][e_res_720_30fps] = AMC_768k ;

	/*m_amcH264DynamicContentRateResTable[Xfer_64][eLiveVideo][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_96][eLiveVideo][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_128][eLiveVideo][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_256][eLiveVideo][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_384][eLiveVideo][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_512][eLiveVideo][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_768][eLiveVideo][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_832][eLiveVideo][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable[Xfer_1024][eLiveVideo][e_res_1080_15fps] = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable[Xfer_1152][eLiveVideo][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1280][eLiveVideo][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1472][eLiveVideo][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1536][eLiveVideo][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1728][eLiveVideo][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_1920][eLiveVideo][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable[Xfer_2048][eLiveVideo][e_res_1080_15fps] = AMC_1152k ;
	m_amcH264DynamicContentRateResTable[Xfer_2560][eLiveVideo][e_res_1080_15fps] = AMC_1152k ;
	m_amcH264DynamicContentRateResTable[Xfer_3072][eLiveVideo][e_res_1080_15fps] = AMC_1152k ;
	m_amcH264DynamicContentRateResTable[Xfer_3584][eLiveVideo][e_res_1080_15fps] = AMC_1152k ;
	m_amcH264DynamicContentRateResTable[Xfer_4096][eLiveVideo][e_res_1080_15fps] = AMC_1152k ;
	m_amcH264DynamicContentRateResTable[Xfer_6144][eLiveVideo][e_res_1080_15fps] = AMC_1152k ;

	if (IsFeatureSupportedBySystem(eFeatureHD1080p30Content))
	{
		m_amcH264DynamicContentRateResTable[Xfer_6144][eGraphics][e_res_1080_30fps] = AMC_2048k ;

		m_amcH264DynamicContentRateResTable[Xfer_4096][eHiResGraphics][e_res_1080_30fps] = AMC_2048k ;
		m_amcH264DynamicContentRateResTable[Xfer_6144][eHiResGraphics][e_res_1080_30fps] = AMC_2048k ;

		m_amcH264DynamicContentRateResTable[Xfer_3072][eLiveVideo][e_res_1080_30fps] = AMC_2048k ;
		m_amcH264DynamicContentRateResTable[Xfer_3584][eLiveVideo][e_res_1080_30fps] = AMC_2048k ;
		m_amcH264DynamicContentRateResTable[Xfer_4096][eLiveVideo][e_res_1080_30fps] = AMC_2048k ;
		m_amcH264DynamicContentRateResTable[Xfer_6144][eLiveVideo][e_res_1080_30fps] = AMC_2048k ;

		if (IsFeatureSupportedBySystem(eFeatureHD1080p60Content))
		{
			m_amcH264DynamicContentRateResTable[Xfer_6144][eGraphics][e_res_1080_60fps] = AMC_3072k ;

			m_amcH264DynamicContentRateResTable[Xfer_6144][eHiResGraphics][e_res_1080_30fps] = AMC_3072k ;

			m_amcH264DynamicContentRateResTable[Xfer_6144][eHiResGraphics][e_res_1080_60fps] = AMC_3072k ;

			m_amcH264DynamicContentRateResTable[Xfer_4096][eLiveVideo][e_res_1080_30fps] = AMC_2560k ;
			m_amcH264DynamicContentRateResTable[Xfer_6144][eLiveVideo][e_res_1080_30fps] = AMC_3072k ;

			m_amcH264DynamicContentRateResTable[Xfer_6144][eLiveVideo][e_res_1080_60fps] = AMC_4096k ;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::initMaxResolutionPerContentRateTable()
{
	if (!m_maxResolutionPerContentRateTable.empty())
		return;

	m_maxResolutionPerContentRateTable[Xfer_64] = e_res_720_5fps;
	m_maxResolutionPerContentRateTable[Xfer_128] = e_res_720_5fps;
	m_maxResolutionPerContentRateTable[Xfer_192] = e_res_720_5fps;
	m_maxResolutionPerContentRateTable[Xfer_256] = e_res_720_5fps;
	m_maxResolutionPerContentRateTable[Xfer_384] = e_res_720_5fps;
	m_maxResolutionPerContentRateTable[Xfer_512] = e_res_720_30fps;
	m_maxResolutionPerContentRateTable[Xfer_768] = e_res_1080_15fps;
	m_maxResolutionPerContentRateTable[Xfer_1024] = e_res_1080_15fps;
	m_maxResolutionPerContentRateTable[Xfer_1152] = e_res_1080_15fps;
	m_maxResolutionPerContentRateTable[Xfer_1280] = e_res_1080_15fps;
	m_maxResolutionPerContentRateTable[Xfer_1536] = e_res_1080_15fps;
	if (IsFeatureSupportedBySystem(eFeatureHD1080p30Content))
	{
		m_maxResolutionPerContentRateTable[Xfer_1536] = e_res_1080_30fps;
		m_maxResolutionPerContentRateTable[Xfer_2048] = e_res_1080_30fps;
		if (IsFeatureSupportedBySystem(eFeatureHD1080p60Content))
		{
			m_maxResolutionPerContentRateTable[Xfer_2560] = e_res_1080_30fps;
			m_maxResolutionPerContentRateTable[Xfer_3072] = e_res_1080_60fps;
			m_maxResolutionPerContentRateTable[Xfer_4096] = e_res_1080_60fps;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::initAMCContentRateCascdeOptimizeResTable_HP()
{
	/****************************   ContentRateControl Table:  *******************************************
	_______________________________________________________________________________________________________________________
	|Conf bps |Resolution	 | 64/96 | 128| 256  | 384 | 512| 768| 832|	 1024| 1152| 1472| 1728| 1920| 2048 | 4096 | 6144 |
	|_________|______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|_____________________________________________________________________________________________________________________|
	|		  |	720/5fps	 |NA     |64  |  64  | 128 | 128| 256| 256|	256  | 384 | 384 | 512 | 512 | 512  | 512  | 512  |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|Graphics |	720/30fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | 384 | 384 | 512 | 512 | 512  | 768  | 768  |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 15fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | NA  | 512 | 512 | 512  | 1280 | 1280 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 30fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | NA  | NA  | NA  | NA   | 1280 | 2048 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 60fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | NA  | NA  | NA  | NA   | NA   | 2048 |
	|_________|______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|_____________________________________________________________________________________________________________________|
	|		  |	720/5fps	 |NA     |64  |  128 | 192 | 256| 384| 384|	384  | 512 | 512 | 512 | 512 | 512  | 512  | 512  |
	| Hi-res  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|Graphics |	720/30fps	 |NA     |NA  |  NA  | NA  | NA | 384| 384|	512  | 512 | 512 | 768 | 768 | 768  | 768  | 768  |
	|	50%	  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 15fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	512  | 512 | 512 | 768 | 768 | 768  | 1280 | 1280 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 30fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | NA  | 768 | 768 | 1024 | 2048 | 2048 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 60fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA |	NA   | NA  | NA  | NA  | NA  | NA   | 2048 | 3072 |
	|_________|______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|_____________________________________________________________________________________________________________________|
	|		  |	720/5fps	 |NA     |64  |  128 | 256 | 256| 384| 512|	512  | 512 | 512 | 512 | 512 | 512  | 512  | 512  |
	|  Live   |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|  Video  |	720/30fps	 |NA     |NA  |  NA  | NA  | NA | 512| 512|	512  | 768 | 768 | 768 | 768 | 768  | 768  | 768  |
	|	66%	  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 15fps	 |NA     |NA  |  NA  | NA  | NA | 512| 512|	512  | 768 | 768 | 768 | 1280| 1280 | 1280 | 1280 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 30fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA | NA   | 768 | 768 | 1024| 1280| 1280 | 2048 | 2048 |
	|		  |______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|
	|		  |	1080/ 60fps	 |NA     |NA  |  NA  | NA  | NA | NA | NA | NA   |	NA |  NA |  NA |  NA |  NA  | 2560 | 4096 |
	|_________|______________|_______|____|______|_____|____|____|____|______|_____|_____|_____|_____|______|______|______|

     Graphics - the old division of content rate

	******************************************************************************************************/
//what about rate 192-for now we will use it the same as 128 - noa R .

	if (!m_amcH264DynamicContentRateResTable_HP.empty())
		return;

	m_amcH264DynamicContentRateResTable_HP[Xfer_128][eGraphics][e_res_720_5fps]  = AMC_64k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_256][eGraphics][e_res_720_5fps]  = AMC_64k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_384][eGraphics][e_res_720_5fps]  = AMC_128k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_512][eGraphics][e_res_720_5fps]  = AMC_128k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_768][eGraphics][e_res_720_5fps]  = AMC_256k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_832][eGraphics][e_res_720_5fps]  = AMC_256k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1024][eGraphics][e_res_720_5fps] = AMC_256k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1152][eGraphics][e_res_720_5fps] = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1280][eGraphics][e_res_720_5fps] = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1472][eGraphics][e_res_720_5fps] = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1536][eGraphics][e_res_720_5fps] = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1728][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1920][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2048][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eGraphics][e_res_720_5fps] = AMC_512k ;

	/*m_amcH264DynamicContentRateResTable_HP[Xfer_64][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_96][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_128][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_256][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_384][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_512][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_768][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_832][eGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1024][eGraphics][e_res_720_30fps] = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable_HP[Xfer_1152][eGraphics][e_res_720_30fps] = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1280][eGraphics][e_res_720_30fps] = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1472][eGraphics][e_res_720_30fps] = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1536][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1728][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1920][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2048][eGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eGraphics][e_res_720_30fps] = AMC_768k ;

	/*m_amcH264DynamicContentRateResTable_HP[Xfer_64][eGraphics][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_96][eGraphics][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_128][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_256][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_384][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_512][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_768][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_832][eGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1024][eGraphics][e_res_1080_15fps] = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1152][eGraphics][e_res_1080_15fps] = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1280][eGraphics][e_res_1080_15fps] = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1472][eGraphics][e_res_1080_15fps] = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable_HP[Xfer_1536][eGraphics][e_res_1080_15fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1728][eGraphics][e_res_1080_15fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1920][eGraphics][e_res_1080_15fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2048][eGraphics][e_res_1080_15fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eGraphics][e_res_1080_15fps] = AMC_1280k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eGraphics][e_res_1080_15fps] = AMC_1280k ;

	/*m_amcH264DynamicContentRateResTable_HP[Xfer_64][eHiResGraphics][e_res_720_5fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_96][eHiResGraphics][e_res_720_5fps]  = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable_HP[Xfer_128][eHiResGraphics][e_res_720_5fps] = AMC_64k ;	// for BRIDGE-6601 - Anat L. request
	m_amcH264DynamicContentRateResTable_HP[Xfer_256][eHiResGraphics][e_res_720_5fps]  = AMC_128k ;	// for BRIDGE-6601 - Anat L. request
	m_amcH264DynamicContentRateResTable_HP[Xfer_384][eHiResGraphics][e_res_720_5fps]  = AMC_192k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_512][eHiResGraphics][e_res_720_5fps]  = AMC_256k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_768][eHiResGraphics][e_res_720_5fps]  = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_832][eHiResGraphics][e_res_720_5fps]  = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1024][eHiResGraphics][e_res_720_5fps] = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1152][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1280][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1472][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1536][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1728][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1920][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2048][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eHiResGraphics][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eHiResGraphics][e_res_720_5fps] = AMC_512k ;

	/*m_amcH264DynamicContentRateResTable_HP[Xfer_64][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_96][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_128][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_256][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_384][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_512][eHiResGraphics][e_res_720_30fps]  = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable_HP[Xfer_768][eHiResGraphics][e_res_720_30fps]  = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_832][eHiResGraphics][e_res_720_30fps]  = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1024][eHiResGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1152][eHiResGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1280][eHiResGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1472][eHiResGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1536][eHiResGraphics][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1728][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1920][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2048][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eHiResGraphics][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eHiResGraphics][e_res_720_30fps] = AMC_768k ;

	/*m_amcH264DynamicContentRateResTable_HP[Xfer_64][eHiResGraphics][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_96][eHiResGraphics][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_128][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_256][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_384][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_512][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_768][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_832][eHiResGraphics][e_res_1080_15fps]  = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable_HP[Xfer_1024][eHiResGraphics][e_res_1080_15fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1152][eHiResGraphics][e_res_1080_15fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1280][eHiResGraphics][e_res_1080_15fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1472][eHiResGraphics][e_res_1080_15fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1536][eHiResGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1728][eHiResGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1920][eHiResGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2048][eHiResGraphics][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eHiResGraphics][e_res_1080_15fps] = AMC_1280k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eHiResGraphics][e_res_1080_15fps] = AMC_1280k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eHiResGraphics][e_res_1080_15fps] = AMC_1280k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eHiResGraphics][e_res_1080_15fps] = AMC_1280k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eHiResGraphics][e_res_1080_15fps] = AMC_1280k ;


	/*m_amcH264DynamicContentRateResTable_HP[Xfer_64][eLiveVideo][e_res_720_5fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_96][eLiveVideo][e_res_720_5fps]   = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable_HP[Xfer_128][eLiveVideo][e_res_720_5fps] = AMC_64k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_256][eLiveVideo][e_res_720_5fps]  = AMC_128k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_384][eLiveVideo][e_res_720_5fps]  = AMC_256k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_512][eLiveVideo][e_res_720_5fps]  = AMC_256k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_768][eLiveVideo][e_res_720_5fps]  = AMC_384k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_832][eLiveVideo][e_res_720_5fps]  = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1024][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1152][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1280][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1472][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1536][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1728][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1920][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2048][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eLiveVideo][e_res_720_5fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eLiveVideo][e_res_720_5fps] = AMC_512k ;

	/*m_amcH264DynamicContentRateResTable_HP[Xfer_64][eLiveVideo][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_96][eLiveVideo][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_128][eLiveVideo][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_256][eLiveVideo][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_384][eLiveVideo][e_res_720_30fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_512][eLiveVideo][e_res_720_30fps]  = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable_HP[Xfer_768][eLiveVideo][e_res_720_30fps]  = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_832][eLiveVideo][e_res_720_30fps]  = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1024][eLiveVideo][e_res_720_30fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1152][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1280][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1472][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1536][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1728][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1920][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2048][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eLiveVideo][e_res_720_30fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eLiveVideo][e_res_720_30fps] = AMC_768k ;

	/*m_amcH264DynamicContentRateResTable_HP[Xfer_64][eLiveVideo][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_96][eLiveVideo][e_res_1080_15fps]   = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_128][eLiveVideo][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_256][eLiveVideo][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_384][eLiveVideo][e_res_1080_15fps]  = AMC_0k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_512][eLiveVideo][e_res_1080_15fps]  = AMC_0k ;*/
	m_amcH264DynamicContentRateResTable_HP[Xfer_768][eLiveVideo][e_res_1080_15fps]  = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_832][eLiveVideo][e_res_1080_15fps]  = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1024][eLiveVideo][e_res_1080_15fps] = AMC_512k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1152][eLiveVideo][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1280][eLiveVideo][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1472][eLiveVideo][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1536][eLiveVideo][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1728][eLiveVideo][e_res_1080_15fps] = AMC_768k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_1920][eLiveVideo][e_res_1080_15fps] = AMC_1280k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2048][eLiveVideo][e_res_1080_15fps] = AMC_1280k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eLiveVideo][e_res_1080_15fps] = AMC_1280k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eLiveVideo][e_res_1080_15fps] = AMC_1280k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eLiveVideo][e_res_1080_15fps] = AMC_1280k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eLiveVideo][e_res_1080_15fps] = AMC_1280k ;
	m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eLiveVideo][e_res_1080_15fps] = AMC_1280k ;

	if (IsFeatureSupportedBySystem(eFeatureHD1080p30Content))
	{
		m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eGraphics][e_res_1080_30fps] = AMC_768k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eGraphics][e_res_1080_30fps] = AMC_768k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eGraphics][e_res_1080_30fps] = AMC_768k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eGraphics][e_res_1080_30fps] = AMC_1280k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eGraphics][e_res_1080_30fps] = AMC_2048k ;

		m_amcH264DynamicContentRateResTable_HP[Xfer_1536][eHiResGraphics][e_res_1080_30fps] = AMC_768k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_1728][eHiResGraphics][e_res_1080_30fps] = AMC_768k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_1920][eHiResGraphics][e_res_1080_30fps] = AMC_768k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_2048][eHiResGraphics][e_res_1080_30fps] = AMC_1024k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eHiResGraphics][e_res_1080_30fps] = AMC_1024k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eHiResGraphics][e_res_1080_30fps] = AMC_1024k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eHiResGraphics][e_res_1080_30fps] = AMC_1024k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eHiResGraphics][e_res_1080_30fps] = AMC_2048k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eHiResGraphics][e_res_1080_30fps] = AMC_2048k ;

		m_amcH264DynamicContentRateResTable_HP[Xfer_1152][eLiveVideo][e_res_1080_30fps] = AMC_768k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_1280][eLiveVideo][e_res_1080_30fps] = AMC_768k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_1472][eLiveVideo][e_res_1080_30fps] = AMC_768k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_1536][eLiveVideo][e_res_1080_30fps] = AMC_1024k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_1728][eLiveVideo][e_res_1080_30fps] = AMC_1024k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_1920][eLiveVideo][e_res_1080_30fps] = AMC_1280k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_2048][eLiveVideo][e_res_1080_30fps] = AMC_1280k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_2560][eLiveVideo][e_res_1080_30fps] = AMC_1280k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_3072][eLiveVideo][e_res_1080_30fps] = AMC_1280k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_3584][eLiveVideo][e_res_1080_30fps] = AMC_1280k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eLiveVideo][e_res_1080_30fps] = AMC_2048k ;
		m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eLiveVideo][e_res_1080_30fps] = AMC_2048k ;

		if (IsFeatureSupportedBySystem(eFeatureHD1080p60Content))
		{
			m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eGraphics][e_res_1080_60fps] = AMC_2048k ;

			m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eHiResGraphics][e_res_1080_60fps] = AMC_2048k ;
			m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eHiResGraphics][e_res_1080_60fps] = AMC_3072k ;

			m_amcH264DynamicContentRateResTable_HP[Xfer_4096][eLiveVideo][e_res_1080_60fps] = AMC_2560k ;
			m_amcH264DynamicContentRateResTable_HP[Xfer_6144][eLiveVideo][e_res_1080_60fps] = AMC_4096k ;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::initMaxResolutionPerContentRateTable_HP()
{
	if (!m_maxResolutionPerContentRateTable_HP.empty())
		return;

	m_maxResolutionPerContentRateTable_HP[Xfer_64] = e_res_720_5fps;
	m_maxResolutionPerContentRateTable_HP[Xfer_128] = e_res_720_5fps;
	m_maxResolutionPerContentRateTable_HP[Xfer_192] = e_res_720_5fps;
	m_maxResolutionPerContentRateTable_HP[Xfer_256] = e_res_720_5fps;
	m_maxResolutionPerContentRateTable_HP[Xfer_384] = e_res_720_30fps;
	m_maxResolutionPerContentRateTable_HP[Xfer_512] = e_res_1080_15fps;
	m_maxResolutionPerContentRateTable_HP[Xfer_768] = e_res_1080_15fps;
	m_maxResolutionPerContentRateTable_HP[Xfer_1024] = e_res_1080_15fps;
	m_maxResolutionPerContentRateTable_HP[Xfer_1152] = e_res_1080_15fps;
	m_maxResolutionPerContentRateTable_HP[Xfer_1280] = e_res_1080_15fps;
	m_maxResolutionPerContentRateTable_HP[Xfer_1536] = e_res_1080_15fps;
	if (IsFeatureSupportedBySystem(eFeatureHD1080p30Content))
	{
		m_maxResolutionPerContentRateTable_HP[Xfer_768] = e_res_1080_30fps;
		m_maxResolutionPerContentRateTable_HP[Xfer_1024] = e_res_1080_30fps;
		m_maxResolutionPerContentRateTable_HP[Xfer_1152] = e_res_1080_30fps;
		m_maxResolutionPerContentRateTable_HP[Xfer_1280] = e_res_1080_30fps;
		m_maxResolutionPerContentRateTable_HP[Xfer_1536] = e_res_1080_30fps;
		m_maxResolutionPerContentRateTable_HP[Xfer_2048] = e_res_1080_30fps;
		if (IsFeatureSupportedBySystem(eFeatureHD1080p60Content))
		{
			m_maxResolutionPerContentRateTable_HP[Xfer_2048] = e_res_1080_60fps;
			m_maxResolutionPerContentRateTable_HP[Xfer_2560] = e_res_1080_60fps;
			m_maxResolutionPerContentRateTable_HP[Xfer_3072] = e_res_1080_60fps;
			m_maxResolutionPerContentRateTable_HP[Xfer_4096] = e_res_1080_60fps;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::FindContentRateInCascadeOPtimizedTable(BYTE Xfer_XCallRate,
														  eEnterpriseMode ContentRateLevel,
														  eCascadeOptimizeResolutionEnum resolutionLevel)
{

	BYTE ContentRate = AMC_0k;
	ContentRateMap* pH264DynamicContentRateResTable = 
	m_isHighProfileContent? &m_amcH264DynamicContentRateResTable_HP : &m_amcH264DynamicContentRateResTable;

	map<BYTE, map<eEnterpriseMode, map<eCascadeOptimizeResolutionEnum, BYTE> > >::iterator findCallRateItr =
			(*pH264DynamicContentRateResTable).find(Xfer_XCallRate);
	if(findCallRateItr != (*pH264DynamicContentRateResTable).end())
	{
		map<eEnterpriseMode, map<eCascadeOptimizeResolutionEnum, BYTE> >::iterator findContentLevelItr =
				(*findCallRateItr).second.find(ContentRateLevel);

		if(findContentLevelItr != (*findCallRateItr).second.end())
		{
			map<eCascadeOptimizeResolutionEnum, BYTE>::iterator findResolutionItr =
					(*findContentLevelItr).second.find(resolutionLevel);

			if(findResolutionItr != (*findContentLevelItr).second.end())
			{
				ContentRate = (*findResolutionItr).second;
			}

		}

	}
	TRACEINTO << "N.A. DEBUG CUnifiedComMode::FindContentRateInCascadeOPtimizedTable Input Params : Xfer_XCallRate : " << (int)Xfer_XCallRate << " ContentRateLevel: "<< ContentRateLevel
			   << " resolutionLevel "<< resolutionLevel << " m_isHighProfileContent " << m_isHighProfileContent;
	PTRACE2(eLevelInfoNormal,"CUnifiedComMode::FindContentRateInCascadeOPtimizedTable : ContentRate : ",CContentBridge::GetContentRateAsString(ContentRate));
	return ContentRate;
	
}

///////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CUnifiedComMode::FindAMCH264DynamicContentRateByLevel(BYTE Xfer_XCallRate,
														  eEnterpriseMode ContentRateLevel,
														  eCascadeOptimizeResolutionEnum resolutionLevel,
														  eConfMediaType aConfMediaType)
{

	BYTE ContentRate = AMC_0k;
	
    if (eSvcOnly == aConfMediaType || eMixAvcSvc == aConfMediaType || eMixAvcSvcVsw == aConfMediaType)
    {// fix content rate
    	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
		if((eProductTypeSoftMCUMfw == curProductType) || (eProductTypeGesher == curProductType))
        {
            TRACEINTO << "SVC containing conference - content rate is AMC_128 for MFW and Gesher";
            return AMC_128k;
        }	

	}
    
    TRACEINTO << "calculate content rate aConfMediaType=" << aConfMediaType << " Xfer_XCallRate: " << (int)Xfer_XCallRate << " ContentRateLevel : " << " resolutionLevel "
    		   << " resolutionLevel: "<< resolutionLevel;

	BYTE XferRate = MapXferRateForDynamicContentRateTable(Xfer_XCallRate);

	TRACEINTO << " after MapXferRateForDynamicContentRateTable - Xfer_XCallRate: " << (int)Xfer_XCallRate ;

	//---start fix of BRIDGE-14420---
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL isAvcToSvcHDEnable = FALSE;
	std::string key = "ENABLE_HIGH_VIDEO_RES_AVC_TO_SVC_IN_MIXED_MODE";
	sysConfig->GetBOOLDataByKey(key, isAvcToSvcHDEnable);	

	if((eMixAvcSvc == aConfMediaType) && isAvcToSvcHDEnable)
	{
		//In case AVC to SVC HD video switch flag is enabled, AVC EP call rate are limit to 1232k in 1080p conf, 832k in 720p conf
		//In order to improve AVC EP's content experience, downgrade all EP's content rate to adapt AVC EP's content capibility.
		BYTE avcCallRate = Xfer_8192;
		if(m_pIPScm)	
		{			
			DWORD confRate = m_pIPScm->GetCallRate();
			PTRACE2INT(eLevelInfoNormal,"CUnifiedComMode::FindAMCH264DynamicContentRateByLevel : m_pIPScm->GetCallRate()",(int)(confRate));
			if(confRate >= 2048)
				avcCallRate = Xfer_1280;  
			else if(confRate >= 768)
				avcCallRate = Xfer_832; 
				
		}

		PTRACE2INT(eLevelInfoNormal,"CUnifiedComMode::FindAMCH264DynamicContentRateByLevel : call rate is: ",(int)(TranslateXferRateToIpRate(Xfer_XCallRate)));
		if(TranslateXferRateToIpRate(Xfer_XCallRate) >= TranslateXferRateToIpRate(avcCallRate))
		{
			//adapt to AVC call content rate
			ContentRate = FindContentRateInCascadeOPtimizedTable(avcCallRate,ContentRateLevel,resolutionLevel);
			if((AMC_0k != ContentRate) || (Xfer_XCallRate == avcCallRate))	
			{
				PTRACE2(eLevelInfoNormal,
					"CUnifiedComMode::FindAMCH264DynamicContentRateByLevel,ENABLE_HIGH_VIDEO_RES_AVC_TO_SVC_IN_MIXED_MODE is enabled,CONTENT RATE : ",
					CContentBridge::GetContentRateAsString(ContentRate));
				return ContentRate;
			}	
			//else : From cascade optimized table, AVC EP have no content capibility under current configuration, then 
					//no need to downgrade other EP's content rate, find content rate with real call rate.			
		}	
	
	}

	//---end fix of BRIDGE-14420---
	
	ContentRate = FindContentRateInCascadeOPtimizedTable(Xfer_XCallRate,ContentRateLevel,resolutionLevel);
	

	//	ContentRate = m_amcH264DynamicContentRateResTable[Xfer_XCallRate][ContentRateLevel][resolutionLevel] ;
	PTRACE2(eLevelInfoNormal,"CUnifiedComMode::FindAMCH264DynamicContentRateByLevel : SELECTED CONTENT RATE : ",CContentBridge::GetContentRateAsString(ContentRate));
	return ContentRate;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
eCascadeOptimizeResolutionEnum CUnifiedComMode::getMaxContentResolutionbyAMCRate(BYTE AMCRate)
{
	TRACEINTO << "AMCRate=" << (int)AMCRate << ", m_isHighProfileContent=" << (int)m_isHighProfileContent;

	if(m_isHighProfileContent)
		return m_maxResolutionPerContentRateTable_HP[TranslateAmcRateToXferRate(AMCRate)];
	else
		return m_maxResolutionPerContentRateTable[TranslateAmcRateToXferRate(AMCRate)];
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//HP content:
eCascadeOptimizeResolutionEnum CUnifiedComMode::getMaxContentResolutionbyRateAndProfile(DWORD contentIPRate, BOOL isHighProfile)
{
	initStaticContentTables();
	
	if(isHighProfile)
		return m_maxResolutionPerContentRateTable_HP[TranslateIPRateToXferRate(contentIPRate)];
	else
		return m_maxResolutionPerContentRateTable[TranslateIPRateToXferRate(contentIPRate)];
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::getAmcH264DynamicContentRateResTable(ContentRateMap& amcH264DynamicContentRateResTable, BOOL isHighProfile)
{
	initStaticContentTables();

	if(isHighProfile)
		amcH264DynamicContentRateResTable = m_amcH264DynamicContentRateResTable_HP;
	else
		amcH264DynamicContentRateResTable = m_amcH264DynamicContentRateResTable;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CUnifiedComMode::getMaxResolutionPerContentRateTable(Rate2ResolutionMap& maxResolutionPerContentRateValues, BOOL isHighProfile)
{
	initStaticContentTables();

	if(isHighProfile)
		maxResolutionPerContentRateValues = m_maxResolutionPerContentRateTable_HP;
	else
		maxResolutionPerContentRateValues = m_maxResolutionPerContentRateTable;
}

////////////////////////////////////////////////////////////////////////////
void  CUnifiedComMode::SetH264PacketizationMode(APIU8  H264PacketizationMode)
{
		long currentFs = 0;
		long currentMBPS = 0;
		APIU16 currentProfile = 0;
		long currentDPB = 0;
		long currentBRandCPB = 0;
		long currentSAR = 0;
		long currentStaticMB=0;
		APIU8 currentLevel =0;
		
		m_pIPScm->GetH264Scm(currentProfile, currentLevel, currentMBPS, currentFs, currentDPB, currentBRandCPB, currentSAR, currentStaticMB, cmCapReceive);
		m_pIPScm->SetH264Scm(currentProfile, currentLevel, currentMBPS, currentFs, currentDPB, currentBRandCPB, currentSAR, currentStaticMB, cmCapReceive, H264PacketizationMode);
		m_pIPScm->GetH264Scm(currentProfile, currentLevel, currentMBPS, currentFs, currentDPB, currentBRandCPB, currentSAR, currentStaticMB, cmCapTransmit);
		m_pIPScm->SetH264Scm(currentProfile, currentLevel, currentMBPS, currentFs, currentDPB, currentBRandCPB, currentSAR, currentStaticMB, cmCapTransmit, H264PacketizationMode);
}



