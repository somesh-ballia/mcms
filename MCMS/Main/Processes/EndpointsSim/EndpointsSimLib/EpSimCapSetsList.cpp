//+========================================================================+
//                  EpSimCapSetsList.cpp								   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
//+========================================================================+

#include "Macros.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "IpCsOpcodes.h"
#include "H264.h"

#include "Segment.h"
#include "ProcessBase.h"
#include "TaskApi.h"
#include "MplMcmsProtocol.h"
#include "SocketApi.h"

#include "SimApi.h"
#include "EndpointsSim.h"
#include "EndpointsGuiApi.h"
#include "EpSimCapSetsList.h"
#include "IpCsContentRoleToken.h"
#include "RvCommonDefs.h"
#include "H323CsInd.h"


#include "TraceStream.h"

#include "SipStructures.h"

#include "SipUtils.h"
#include "OpcodesMcmsInternal.h"

class CCapSetInfo;
BOOL CCapSetsList::m_isTIP = FALSE;

/* from IPUtils.h */
typedef enum{
//	__FIRST_DPT		= 96,	// first valid dynamic payload type

	_AACLD			= 96,// TIP
	_SirenLPRS_128	= 97,
	_RtvDynamic 	= 98,
	_AnnexQDynamic	= 99,
	_G7221_16		= 100,
	_G7221_24		= 101,
	_G7221_32		= 102,
	_Siren14_24		= 103,
	_Siren14_32		= 104,
	_Siren14_48		= 105,
	_PLCM_SVC       = 105,
	_G7221C_24		= 106,
	_G7221C_32		= 107,
	_G7221C_48		= 108,
	_H263Dynamic	= 109,
	_H264Dynamic	= 110,
	_H26LDynamic	= 111,
	_Siren7_16		= 111,   //Naturally, two codecs can share one DYNAMIC payload type, just not at the same time and port
	_Rfc2833DtmfDynamic	= 112,
	_Siren14S_48		= 113,
	_Siren14S_64		= 114,
	_Siren14S_96		= 115,
	_G719_32			= 116,
	_G719_48			= 117,
	_G719_64			= 118,
	_G719S_64			= 119,
	_G719S_96			= 120,
	_G719S_128			= 121,
	_Siren22_32			= 122,
	_Siren22_48			= 123,
	_Siren22_64			= 124,
	_Siren22S_64		= 125,
	_Siren22S_96		= 126,
	_Siren22S_128		= 127,
	_SdesDynamic		= 128,	///TBD??

//	__LAST_DPT		= 127,	// last valid dynamic payload type
} eMcmsPayloadType;
/**/

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//   CCapSetsList - List of EP Capability elements
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CCapSetsList::CCapSetsList()      // constructor
{
	m_updateCounter = 1;

	for (int i = 0; i < MAX_H323_CAPS; i++)
		m_paCapArray[i] = NULL;

	m_nDefaultCapIndex = 0;
	char szStatusStr[256];

	//*********-----******** Predefined cap sets - start
	CVideoCapH264  rH264capHd1080(eVideoModeHD1080);
	CVideoCapH264  rH264capHd1080p60(eVideoModeHD1080p60);
	CVideoCapH264  rH264capHd720(eVideoModeHD720);
	CVideoCapH264  rH264capSd60(eVideoModeSD60);
	CVideoCapH264  rH264capW4Cif30(eVideoModeW4CIF30);
	CVideoCapH264  rH264capSd30(eVideoModeSD30);
	CVideoCapH264  rH264capSd15(eVideoModeSD15);
	CVideoCapH264  rH264cap2Cif30(eVideoMode2Cif30);
	CVideoCapH264  rH264capSd7_5(eVideoModeSD7_5);
	CVideoCapH264  rH264capCif(eVideoModeCif);
	CVideoCapH264  rH264HighProfilecapHD1080(eVideoModeHD1080,-1,-1,H264_Profile_High);
	CVideoCapH264  rH264HighProfilecapHD1080p60(eVideoModeHD1080p60,-1,-1,H264_Profile_High);
	CVideoCapH264  rH264MainProfilecapHD1080(eVideoModeHD1080,-1,-1,H264_Profile_Main);

	CVideoCapSVC   rSVC(eVideoModeHD1080);

	CVideoCapRTV   rRtvHDcap(e_rtv_HD720Symmetric);
	CVideoCapRTV   rRtvVGAcap(e_rtv_VGA30);
	CVideoCapRTV   rRtvCifcap(e_rtv_CIF30);
	CVideoCapRTV   rRtvQCifcap(e_rtv_QCIF15);


	CVideoCapMSSvc rMsSvcCap;

	CVideoCapVP8   rVP8cap; //N.A. DEBUG VP8
	CVideoCapH263  rH263cap;
	CVideoCapH261  rH261cap;
	CVideoCapH263  rH263capPresentation;
	rH263capPresentation.CreatePresentationCap();

	CCapSet  rCap(100,"FULL CAPSET");
	FillFullNonVideoCapset(&rCap, eTransportTypeUdp);

	// TIP support for testing embedded MLA feature
	if (CCapSetsList::m_isTIP == TRUE)
	{
		rCap.AddVideoProtocol(rH264MainProfilecapHD1080);
	}

	rCap.AddVideoProtocol(rH264HighProfilecapHD1080);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap);
//	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
/*	rCap.SetH239Rate(190);
	rCap.SetDynamicPTReplacement(TRUE);*/
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(101);
	rCap.SetName("G711+H264sd+Fecc");
	rCap.Empty();
	rCap.SetRate(1920);
	rCap.SetFecc(TRUE);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capSd30);
	AddCap(rCap,szStatusStr);

	rCap.SetID(102);
	rCap.SetName("G711+H263+Fecc");
	rCap.Empty();
	rCap.SetRate(384);
	rCap.SetFecc(TRUE);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH263cap);
	AddCap(rCap,szStatusStr);

	rCap.SetID(103);
	rCap.SetName("G711+H264sd+H263+Fecc");
	rCap.Empty();
	rCap.SetRate(384);
	rCap.SetFecc(TRUE);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capSd30);
	rCap.AddVideoProtocol(rH263cap);
	AddCap(rCap,szStatusStr);

	rCap.SetID(104);
	rCap.SetName("G711+H264cif+H263+Fecc");
	rCap.Empty();
	rCap.SetRate(1920);
	rCap.SetFecc(TRUE);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capCif);
	rCap.AddVideoProtocol(rH263cap);
	AddCap(rCap,szStatusStr);

	rCap.SetID(105);
	rCap.SetName("H264(cif)+ALL");
	rCap.Empty();
	rCap.SetRate(1920);
	// Siren 14 Stereo
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capCif);
	rCap.AddVideoProtocol(rH263cap);
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd720);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);


	rCap.SetID(106);
	rCap.SetName("H264-HIGH-PROFILE(cif)+ALL");
	rCap.Empty();
	rCap.SetRate(1920);
	// Siren 14 Stereo
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264HighProfilecapHD1080);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd720);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);


	rCap.SetID(107);
	rCap.SetName("H264(2cif30)+ALL");
	rCap.Empty();
	rCap.SetRate(1920);
	// Siren 14 Stereo
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264cap2Cif30);
	rCap.AddVideoProtocol(rH263cap);
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd720);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);


	rCap.SetID(108);
	rCap.SetName("H264(sd15)+ALL");
	rCap.Empty();
	rCap.SetRate(1920);
	// Siren 14 Stereo
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capSd15);
	rCap.AddVideoProtocol(rH263cap);
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd720);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(109);
	rCap.SetName("H264(sd30)+ALL");
	rCap.Empty();
	rCap.SetRate(1920);
	// Siren 14 Stereo
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capSd30);
	rCap.AddVideoProtocol(rH263cap);
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd720);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(110);
	rCap.SetName("H264(hd720)+ALL");
	rCap.Empty();
	rCap.SetRate(1920);
	// Siren 14 Stereo
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capHd720);
	rCap.AddVideoProtocol(rH263cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(180);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(111);
	rCap.SetName("H264(hd1080)+ALL");
	rCap.Empty();
	rCap.SetRate(6144);
	// Siren 14 Stereo
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);


	rCap.SetID(112);
	rCap.SetName("H264(sd60)+ALL");
	rCap.Empty();
	rCap.SetRate(6144);
	// Siren 14 Stereo
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capSd60);
	rCap.AddVideoProtocol(rH263cap);
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(113);
	rCap.SetName("H264(w4cif30)+ALL");
	rCap.Empty();
	rCap.SetRate(6144);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capW4Cif30);
	rCap.AddVideoProtocol(rH263cap);
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(114);
	rCap.SetName("H261+ALL");
	rCap.Empty();
	rCap.SetRate(1920);
	// Siren 14 Stereo
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH261cap);
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd720);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(115);
	rCap.SetName("FULL_H264_CONTENT_ONLY");
	rCap.Empty();
	rCap.SetRate(4096);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
		//rCap.AddAudioAlg(eG7221_24kCaCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
//	rCap.AddAudioAlg(eG7221C_CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capHd720);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(170);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd720);
	AddCap(rCap,szStatusStr);

	rCap.SetID(116);
	rCap.SetName("FULL_H263_CONTENT_ONLY");
	rCap.Empty();
	rCap.SetRate(4096);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
		//rCap.AddAudioAlg(eG7221_24kCaCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
//	rCap.AddAudioAlg(eG7221C_CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	//rCap.AddVideoProtocol(rH264capHd720);//try to fix bug that MCMS don't send recap when H263 content endpoint join the conference
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(180);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(117);
	rCap.SetName("FULL_H264_CONTENT_WITH_SD7_5");
	rCap.Empty();
	rCap.SetRate(384);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capSd7_5);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(190);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);


	rCap.SetID(118);
	rCap.SetName("AudioOnly");
	rCap.Empty();
	rCap.SetRate(64);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	AddCap(rCap,szStatusStr);


	rCap.SetID(119);
	rCap.SetName("Audio Only");
	rCap.Empty();
	rCap.SetRate(64);
	// Siren 14 Stereo
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	AddCap(rCap, szStatusStr);

	rCap.SetID(120);
	rCap.SetName("FULL_With_Siren22Stereo128K");
	rCap.Empty();
	rCap.SetRate(6144);
//	rCap.AddAudioAlg(eG719Stereo_128kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_96kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_128kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_64kCapCode);
	rCap.AddAudioAlg(eG719_64kCapCode);
	rCap.AddAudioAlg(eG719_48kCapCode);
	rCap.AddAudioAlg(eG719_32kCapCode);
	rCap.AddAudioAlg(eSiren22_64kCapCode);
	rCap.AddAudioAlg(eSiren22_48kCapCode);
	rCap.AddAudioAlg(eSiren22_32kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
		//rCap.AddAudioAlg(eG7221_24kCaCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
//	rCap.AddAudioAlg(eG7221C_CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(121);
	rCap.SetName("FULL_With_Siren22Stereo96K");
	rCap.Empty();
	rCap.SetRate(6144);
//	rCap.AddAudioAlg(eG719Stereo_128kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_96kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_64kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_128kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_64kCapCode);
	rCap.AddAudioAlg(eG719_64kCapCode);
	rCap.AddAudioAlg(eG719_48kCapCode);
	rCap.AddAudioAlg(eG719_32kCapCode);
	rCap.AddAudioAlg(eSiren22_64kCapCode);
	rCap.AddAudioAlg(eSiren22_48kCapCode);
	rCap.AddAudioAlg(eSiren22_32kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
		//rCap.AddAudioAlg(eG7221_24kCaCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
//	rCap.AddAudioAlg(eG7221C_CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(122);
	rCap.SetName("FULL_With_Siren14Stereo");
	rCap.Empty();
	rCap.SetRate(6144);
//	rCap.AddAudioAlg(eG719Stereo_128kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_96kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_64kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_128kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_96kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_64kCapCode);
	rCap.AddAudioAlg(eG719_64kCapCode);
	rCap.AddAudioAlg(eG719_48kCapCode);
	rCap.AddAudioAlg(eG719_32kCapCode);
	rCap.AddAudioAlg(eSiren22_64kCapCode);
	rCap.AddAudioAlg(eSiren22_48kCapCode);
	rCap.AddAudioAlg(eSiren22_32kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
		//rCap.AddAudioAlg(eG7221_24kCaCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
//	rCap.AddAudioAlg(eG7221C_CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(123);
	rCap.SetName("FULL_With_Siren22Mono");
	rCap.Empty();
	rCap.SetRate(6144);
//	rCap.AddAudioAlg(eG719Stereo_128kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_96kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_64kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_128kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_96kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_64kCapCode);
//	rCap.AddAudioAlg(eG719_64kCapCode);
//	rCap.AddAudioAlg(eG719_48kCapCode);
//	rCap.AddAudioAlg(eG719_32kCapCode);
	rCap.AddAudioAlg(eSiren22_64kCapCode);
	rCap.AddAudioAlg(eSiren22_48kCapCode);
	rCap.AddAudioAlg(eSiren22_32kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
		//rCap.AddAudioAlg(eG7221_24kCaCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
//	rCap.AddAudioAlg(eG7221C_CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);


	rCap.SetID(124);
	rCap.SetName("FULL_With_Siren22Stereo128K");
	rCap.Empty();
	rCap.SetRate(6144);
//	rCap.AddAudioAlg(eG719Stereo_128kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_96kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_128kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_64kCapCode);
	rCap.AddAudioAlg(eG719_64kCapCode);
	rCap.AddAudioAlg(eG719_48kCapCode);
	rCap.AddAudioAlg(eG719_32kCapCode);
	rCap.AddAudioAlg(eSiren22_64kCapCode);
	rCap.AddAudioAlg(eSiren22_48kCapCode);
	rCap.AddAudioAlg(eSiren22_32kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
		//rCap.AddAudioAlg(eG7221_24kCaCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
//	rCap.AddAudioAlg(eG7221C_CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(125);
	rCap.SetName("FULL_With_Siren22Stereo96K");
	rCap.Empty();
	rCap.SetRate(6144);
//	rCap.AddAudioAlg(eG719Stereo_128kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_96kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_64kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_128kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_64kCapCode);
	rCap.AddAudioAlg(eG719_64kCapCode);
	rCap.AddAudioAlg(eG719_48kCapCode);
	rCap.AddAudioAlg(eG719_32kCapCode);
	rCap.AddAudioAlg(eSiren22_64kCapCode);
	rCap.AddAudioAlg(eSiren22_48kCapCode);
	rCap.AddAudioAlg(eSiren22_32kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
		//rCap.AddAudioAlg(eG7221_24kCaCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
//	rCap.AddAudioAlg(eG7221C_CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(126);
	rCap.SetName("FULL_With_Siren14Stereo");
	rCap.Empty();
	rCap.SetRate(6144);
//	rCap.AddAudioAlg(eG719Stereo_128kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_96kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_64kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_128kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_96kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_64kCapCode);
	rCap.AddAudioAlg(eG719_64kCapCode);
	rCap.AddAudioAlg(eG719_48kCapCode);
	rCap.AddAudioAlg(eG719_32kCapCode);
	rCap.AddAudioAlg(eSiren22_64kCapCode);
	rCap.AddAudioAlg(eSiren22_48kCapCode);
	rCap.AddAudioAlg(eSiren22_32kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
		//rCap.AddAudioAlg(eG7221_24kCaCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
//	rCap.AddAudioAlg(eG7221C_CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(127);
	rCap.SetName("FULL_With_Siren22Mono");
	rCap.Empty();
	rCap.SetRate(6144);
//	rCap.AddAudioAlg(eG719Stereo_128kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_96kCapCode);
//	rCap.AddAudioAlg(eG719Stereo_64kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_128kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_96kCapCode);
//	rCap.AddAudioAlg(eSiren22Stereo_64kCapCode);
//	rCap.AddAudioAlg(eG719_64kCapCode);
//	rCap.AddAudioAlg(eG719_48kCapCode);
//	rCap.AddAudioAlg(eG719_32kCapCode);
	rCap.AddAudioAlg(eSiren22_64kCapCode);
	rCap.AddAudioAlg(eSiren22_48kCapCode);
	rCap.AddAudioAlg(eSiren22_32kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
		//rCap.AddAudioAlg(eG7221_24kCaCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
//	rCap.AddAudioAlg(eG7221C_CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetFecc(TRUE);
	rCap.SetEncryption(TRUE);
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(201);
	rCap.SetName("FULL CAPSET NO BFCP");
	FillFullNonVideoCapset(&rCap, eUnknownTransportType);
	rCap.AddVideoProtocol(rH264HighProfilecapHD1080);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(202);
	rCap.SetName("FULL CAPSET BFCP TCP");
	FillFullNonVideoCapset(&rCap, eTransportTypeTcp);
	rCap.AddVideoProtocol(rH264HighProfilecapHD1080);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);
	
	rCap.SetID(301);
	rCap.SetName("RTV_HD_CAPSET");
	FillFullNonVideoCapset(&rCap, eTransportTypeTcp);
	rCap.AddVideoProtocol(rRtvHDcap);
	rCap.AddVideoProtocol(rH263cap);
	AddCap(rCap,szStatusStr);

	rCap.SetID(302);
	rCap.SetName("RTV_VGA_CAPSET");
	FillFullNonVideoCapset(&rCap, eTransportTypeTcp);
	rCap.AddVideoProtocol(rRtvVGAcap);
	rCap.AddVideoProtocol(rH263cap);
	AddCap(rCap,szStatusStr);

	rCap.SetID(303);
	rCap.SetName("RTV_CIF_CAPSET");
	FillFullNonVideoCapset(&rCap, eTransportTypeTcp);
	rCap.AddVideoProtocol(rRtvCifcap);
	rCap.AddVideoProtocol(rH263cap);
	AddCap(rCap,szStatusStr);

	rCap.SetID(304);
	rCap.SetName("RTV_QCIF_CAPSET");
	FillFullNonVideoCapset(&rCap, eTransportTypeTcp);
	rCap.AddVideoProtocol(rRtvQCifcap);
	rCap.AddVideoProtocol(rH263cap);
	AddCap(rCap,szStatusStr);

	/*
	rCap.SetID(305);
	rCap.SetName("SVC_HD720_CAPSET");
	FillFullNonVideoCapset(&rCap, eTransportTypeTcp);
	rCap.AddVideoProtocol(rMsSvcCap);
	//rCap.AddVideoProtocol(rRtvHDcap);
	AddCap(rCap,szStatusStr);

*/
	rCap.SetID(305);
	rCap.SetName("SVC_HD720_CAPSET");
	rCap.Empty();
	rCap.SetRate(1920);
		// Siren 14 Stereo
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddVideoProtocol(rMsSvcCap);
	rCap.AddVideoProtocol(rRtvHDcap);
	AddCap(rCap,szStatusStr);


	rCap.SetID(306);
	rCap.SetName("FULL CAPS 1080p60");
	rCap.Empty();
	rCap.SetRate(6144);
	rCap.AddAudioAlg(eG719Stereo_128kCapCode);
	rCap.AddAudioAlg(eG719Stereo_96kCapCode);
	rCap.AddAudioAlg(eG719Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_128kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren22Stereo_64kCapCode);
	rCap.AddAudioAlg(eG719_64kCapCode);
	rCap.AddAudioAlg(eG719_48kCapCode);
	rCap.AddAudioAlg(eG719_32kCapCode);
	rCap.AddAudioAlg(eSiren22_64kCapCode);
	rCap.AddAudioAlg(eSiren22_48kCapCode);
	rCap.AddAudioAlg(eSiren22_32kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	//rCap.AddAudioAlg(eOpusStereo_CapCode);
	rCap.AddAudioAlg(eG7221C_48kCapCode);
	rCap.AddAudioAlg(eG7221C_24kCapCode);
	rCap.AddAudioAlg(eG7221C_32kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	//rCap.AddAudioAlg(eOpus_CapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_16kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG7221C_CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG722Stereo_128kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddAudioAlg(eSirenLPRStereo_128kCapCode);
	rCap.AddVideoProtocol(rH264HighProfilecapHD1080p60);
	rCap.AddVideoProtocol(rH264capHd1080p60);
	rCap.AddVideoProtocol(rH263cap);
	rCap.AddVideoProtocol(rH261cap);
	//rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	rCap.SetH239Rate(1920);
	rCap.SetDynamicPTReplacement(TRUE);
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	AddCap(rCap,szStatusStr);

	rCap.SetID(307);
	rCap.SetName("WEBRTC_CAPS");
	rCap.Empty();
	FillFullNonVideoCapset(&rCap, eTransportTypeUdp);
	rCap.SetRate(1920);
	rCap.AddVideoProtocol(rVP8cap); //N.A. DEBUG VP8
	AddCap(rCap,szStatusStr);

	rCap.SetID(308);
	rCap.SetName("FULL CAPSET SVC");
	rCap.Empty();
	rCap.SetRate(6144);
	// Siren 14 Stereo
	rCap.AddAudioAlg(eSirenLPR_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap.AddAudioAlg(eSiren14Stereo_48kCapCode);
	rCap.AddAudioAlg(eSiren14_24kCapCode);
	rCap.AddAudioAlg(eSiren14_32kCapCode);
	rCap.AddAudioAlg(eSiren14_48kCapCode);
	rCap.AddAudioAlg(eG7221_32kCapCode);
	rCap.AddAudioAlg(eG7221_24kCapCode);
	rCap.AddAudioAlg(eG7231CapCode);
	rCap.AddAudioAlg(eG729AnnexACapCode);
	rCap.AddAudioAlg(eG728CapCode);
	rCap.AddAudioAlg(eG722_48kCapCode);
	rCap.AddAudioAlg(eG722_56kCapCode);
	rCap.AddAudioAlg(eG722_64kCapCode);
	rCap.AddAudioAlg(eG711Ulaw56kCapCode);
	rCap.AddAudioAlg(eG711Alaw56kCapCode);
	rCap.AddAudioAlg(eG711Ulaw64kCapCode);
	rCap.AddAudioAlg(eG711Alaw64kCapCode);
	rCap.AddAudioAlg(eSirenLPR_Scalable_32kCapCode);
	rCap.AddAudioAlg(eSirenLPR_Scalable_48kCapCode);
	rCap.AddAudioAlg(eSirenLPR_Scalable_64kCapCode);
	rCap.AddVideoProtocol(rH264HighProfilecapHD1080);
	rCap.AddVideoProtocol(rH264capHd1080);
	rCap.AddVideoProtocol(rSVC);
	rCap.AddPresentationVideoH264(rH264capHd1080);
	rCap.AddPresentationVideo(rH263capPresentation);
	rCap.SetFecc(TRUE);

	AddCap(rCap, szStatusStr);


//*********-----******** Predefined cap sets - end

	TRACEINTO << "CCapSetsList::CCapSetsList counter-" << m_updateCounter;
}


/////////////////////////////////////////////////////////////////////////////
CCapSetsList::~CCapSetsList()     // destructor
{
	for (int i = 0; i < MAX_H323_CAPS; i++)
		POBJDELETE(m_paCapArray[i]);
}


/////////////////////////////////////////////////////////////////////////////
/*const CCapSet* CCapSetsList::GetCurrentCap( const DWORD capID ) const
{
	PTRACE(eLevelInfoNormal,"CCapSetsList::GetCurrentCap");

	for (int i = 0; i < m_nNumElements; i++) {
		if( m_paCapArray[i] != NULL ) {
			if ( capID == m_paCapArray[i]->GetID() )
				return ( m_paCapArray[i] );
		} else
			DBGPASSERT(1000+i);
	}

	return NULL;
}*/


/////////////////////////////////////////////////////////////////////////////
const CCapSet* CCapSetsList::GetDefaultCap() const
{
//	PTRACE(eLevelInfoNormal,"CCapSetsList::GetDefaultCap");

	if (m_nDefaultCapIndex >= MAX_H323_CAPS)
		return NULL;		// invalid index

	return ( m_paCapArray[m_nDefaultCapIndex] );
}


/////////////////////////////////////////////////////////////////////////////
STATUS CCapSetsList::AddCap( const CCapSet& rNewCap, char* pszStatusStr )
{
	TRACEINTO << "CCapSetsList::AddCap name - " << rNewCap.GetName();

//	if ( m_nNumElements >= MAX_H323_CAPS )
//		return STATUS_FAIL;	// list is full

	// checking if legal parameters
//	for ( int i = 0; i < m_nNumElements; i++ )
//		if ( 0 == strcmp (m_paCapArray[i]->GetName(), rNewCap.GetName()) )
//			return STATUS_FAIL;	// name already exists

	CCapSet* pCapTmp = NULL;
		// check if exists
	if( NULL != (pCapTmp = FindCapSet(rNewCap.GetName())) )
	{
		strcpy(pszStatusStr,"Can't add capset: Name already exists.");
		return STATUS_FAIL;
	}
//	if( NULL != (pCapTmp = FindCapSet(rNewCap.GetID())) )
//		return STATUS_FAIL;

	// if not found
		// check for free place
	int index = GetNextEmptyPlace();
	if( -1 == index )
	{
		strcpy(pszStatusStr,"Can't add capset: Max num of capsets reached.");
		return STATUS_FAIL;
	}

	// create ID for this capability
	DWORD newCapID = GenerateID();

	// inserting the capability to the list
	m_paCapArray[index] = new CCapSet( rNewCap );
	m_paCapArray[index]->SetID( newCapID );

	m_updateCounter++;

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
/*STATUS CCapSetsList::DelCap( const char* capName )
{
	PTRACE(eLevelInfoNormal,"CCapSetsList::DelCap");

	if ( NULL == capName )
		return STATUS_FAIL;

	for ( int i = 0; i<MAX_H323_CAPS; i++ )
	{
		if( m_paCapArray[i] != NULL )
		{
			if ( 0 == strcmp(m_paCapArray[i]->GetName(), capName) )
			{
				PTRACE2(eLevelInfoNormal,"CCapSetsList::DelCap - cap set to be deleted, Name - ",capName);
				POBJDELETE (m_paCapArray[i]);
			}
		}
	}

	return STATUS_OK;
}*/


/////////////////////////////////////////////////////////////////////////////
void CCapSetsList::HandleBatchEvent(const DWORD opcode, CSegment* pParam)
{
	TRACEINTO << " CCapSetsList::HandleBatchEvent, opcode: " << opcode;
	switch( opcode )
	{
		case BATCH_DEL_CAPSET:
		{
			char capName[MAX_CAP_NAME];
			*pParam >> capName;
			STATUS status = DeleteCapSet(capName);
			break;
		}
		case BATCH_ADD_CAPSET:
		{
			CCapSet* pCap = new CCapSet();
			pCap->DeSerialize(*pParam);

			char szStatusString[256];
			STATUS result = AddCap(*pCap,szStatusString);
			POBJDELETE(pCap);

			if(result != STATUS_OK)
				TRACEINTO << " CCapSetsList::HandleBatchEvent, failed to add caps, reason: " << szStatusString;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
DWORD CCapSetsList::GenerateID() const
{
const DWORD INITIAL_ID = 3000;

	DWORD  id = INITIAL_ID;
	for( int i=0; i<MAX_H323_CAPS; i++ )
		if( m_paCapArray[i] != NULL && m_paCapArray[i]->GetID() > id )
			id = m_paCapArray[i]->GetID();

	return id + 1;
}


/////////////////////////////////////////////////////////////////////////////
void CCapSetsList::GetFullCapListToGui(CSegment* pParam)
{
	DWORD	nGuiUpdateCounter = 0;
	*pParam >> nGuiUpdateCounter;

	COsQueue txMbx;
	txMbx.DeSerialize(*pParam);

	// check if there is e.p. changed
	BOOL need2update = ( m_updateCounter > nGuiUpdateCounter ) ? TRUE : FALSE;

	int  i = 0;
	for( i=0; i<MAX_H323_CAPS && need2update == FALSE; i++ )
	{
		if ( NULL != m_paCapArray[i] ) {
			if( m_paCapArray[i]->IsChanged() != FALSE ) {
				m_paCapArray[i]->ClearChanged();
				need2update = TRUE;
				break;
			}
		}
	}
		// all endpoints in list are not changed
	if( FALSE == need2update )
		return;

	CSegment* pMsgSeg = new CSegment;
	*pMsgSeg	<< GUI_TO_ENDPOINTS
				<< GET_CAP_LIST_REQ
				<< (DWORD)STATUS_OK;

	*pMsgSeg	<< m_updateCounter
				<< GetCapListLength();


	for ( int i = 0; i < MAX_H323_CAPS; i++ )
	{
		if ( m_paCapArray[i] != NULL )
			m_paCapArray[i]->Serialize(*pMsgSeg);
	}

	CTaskApi api;
	api.CreateOnlyApi(txMbx);
	api.SendMsg(pMsgSeg,SOCKET_WRITE);
	api.DestroyOnlyApi();

}


/////////////////////////////////////////////////////////////////////////////
void CCapSetsList::AddCapFromGui(CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CCapSetsList::AddCapFromGui");
//	int nextemptyindex = GetNextEmptyPlace();
//	m_paCapArray[nextemptyindex] = new CH323Cap(0xFFFFFFFF,"TEMP"/*temp dummy values*/);
//	m_paCapArray[nextemptyindex]->OnStartElement( pParam );
	CCapSet* pCap = new CCapSet();
	pCap->DeSerialize(*pParam);

	char szStatusString[256];
	STATUS result = AddCap(*pCap,szStatusString);
	POBJDELETE(pCap);
	if( STATUS_OK != result )
	{
		COsQueue txMbx;
		txMbx.DeSerialize(*pParam);

		CSegment* pMsgSeg = new CSegment;
		*pMsgSeg	<< GUI_TO_ENDPOINTS
					<< ADD_CAP_REQ
					<< (DWORD)result;
		*pMsgSeg	<< szStatusString;

		CTaskApi api;
		api.CreateOnlyApi(txMbx);
		api.SendMsg(pMsgSeg,SOCKET_WRITE);
		api.DestroyOnlyApi();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CCapSetsList::DeleteCapFromGui(CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CCapSetsList::DeleteCapFromGui");

	DWORD capid = 0xFFFFFFFF;
	*pParam >> capid;

	COsQueue txMbx;
	txMbx.DeSerialize(*pParam);

	STATUS status = DeleteCapSet(capid);
}

/////////////////////////////////////////////////////////////////////////////
void CCapSetsList::UpdateCapFromGui(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CCapSetsList::UpdateCapFromGui");

//	DWORD capid;
//	*pParam >> capid;
	CCapSet* pCapSource = new CCapSet();
	pCapSource->DeSerialize(*pParam);

	COsQueue txMbx;
	txMbx.DeSerialize(*pParam);

	POBJDELETE(pCapSource);
}

/////////////////////////////////////////////////////////////////////////////
CCapSet* CCapSetsList::FindCapSet(const DWORD nId)
{
	for ( int i = 0; i < MAX_H323_CAPS; i++ )
		if ( NULL != m_paCapArray[i] )
			if ( m_paCapArray[i]->GetID() == nId )
				return m_paCapArray[i];
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
CCapSet* CCapSetsList::FindCapSet(const char* pszName)
{
	for ( int i = 0; i < MAX_H323_CAPS; i++ )
		if ( NULL != m_paCapArray[i] )
			if ( 0 == strcmp(m_paCapArray[i]->GetName(),pszName) )
				return m_paCapArray[i];
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS	CCapSetsList::DeleteCapSet(const DWORD nId)
{
	for ( int i = 0; i < MAX_H323_CAPS; i++ ) {
		if ( NULL != m_paCapArray[i] ) {
			if ( m_paCapArray[i]->GetID() == nId ) {
				POBJDELETE(m_paCapArray[i]);
				m_updateCounter++;
				return STATUS_OK;
			}
		}
	}
	return STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS	CCapSetsList::DeleteCapSet(const char* pszName)
{
	for ( int i = 0; i < MAX_H323_CAPS; i++ ) {
		if ( NULL != m_paCapArray[i] ) {
			if ( 0 == strcmp(m_paCapArray[i]->GetName(),pszName) ) {
				POBJDELETE(m_paCapArray[i]);
				m_updateCounter++;
				return STATUS_OK;
			}
		}
	}
	return STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
int CCapSetsList::GetNextEmptyPlace() const
{
	for ( int i = 0; i < MAX_H323_CAPS; i++ )
		if ( NULL == m_paCapArray[i] )
			return i;
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CCapSetsList::GetCapListLength() const
{
	DWORD size=0;
	for ( int i = 0; i < MAX_H323_CAPS; i++ )
		if ( NULL != m_paCapArray[i] )
			size++;

	//cout << "CCapSetsList::GetCapListLength() size = " << size << endl;

	return size;
}

void CCapSetsList::FillFullNonVideoCapset(CCapSet* rCap, enTransportType bfcpTransportType)
{
	rCap->Empty();
	rCap->SetRate(6144);

	//TIP support for testing embedded MLA feature
	if (CCapSetsList::m_isTIP == TRUE)
	{
		rCap->AddAudioAlg(eAAC_LDCapCode);// TIP //_dtls_
	}

	rCap->AddAudioAlg(eG719Stereo_128kCapCode);
	rCap->AddAudioAlg(eG719Stereo_96kCapCode);
	rCap->AddAudioAlg(eG719Stereo_64kCapCode);
	rCap->AddAudioAlg(eSiren22Stereo_128kCapCode);
	rCap->AddAudioAlg(eSiren22Stereo_96kCapCode);
	rCap->AddAudioAlg(eSiren22Stereo_64kCapCode);
	rCap->AddAudioAlg(eG719_64kCapCode);
	rCap->AddAudioAlg(eG719_48kCapCode);
	rCap->AddAudioAlg(eG719_32kCapCode);
	rCap->AddAudioAlg(eSiren22_64kCapCode);
	rCap->AddAudioAlg(eSiren22_48kCapCode);
	rCap->AddAudioAlg(eSiren22_32kCapCode);
	rCap->AddAudioAlg(eSiren14Stereo_96kCapCode);
	rCap->AddAudioAlg(eSiren14Stereo_64kCapCode);
	rCap->AddAudioAlg(eSiren14Stereo_48kCapCode);
	//rCap->AddAudioAlg(eOpusStereo_CapCode);
	rCap->AddAudioAlg(eG7221C_48kCapCode);
	rCap->AddAudioAlg(eG7221C_24kCapCode);
	rCap->AddAudioAlg(eG7221C_32kCapCode);
	rCap->AddAudioAlg(eSiren14_24kCapCode);
	rCap->AddAudioAlg(eSiren14_32kCapCode);
	rCap->AddAudioAlg(eSiren14_48kCapCode);
	//rCap->AddAudioAlg(eOpus_CapCode);
	rCap->AddAudioAlg(eG7221_32kCapCode);
	//rCap->AddAudioAlg(eG7221_24kCaCode);
	rCap->AddAudioAlg(eG7221_16kCapCode);
	rCap->AddAudioAlg(eG7231CapCode);
	rCap->AddAudioAlg(eG7221C_CapCode);
	rCap->AddAudioAlg(eG729AnnexACapCode);
	rCap->AddAudioAlg(eG728CapCode);
	rCap->AddAudioAlg(eG722_48kCapCode);
	rCap->AddAudioAlg(eG722_56kCapCode);
	rCap->AddAudioAlg(eG722_64kCapCode);
	rCap->AddAudioAlg(eG722Stereo_128kCapCode);
	rCap->AddAudioAlg(eG711Ulaw56kCapCode);
	rCap->AddAudioAlg(eG711Alaw56kCapCode);
	rCap->AddAudioAlg(eG711Ulaw64kCapCode);
	rCap->AddAudioAlg(eG711Alaw64kCapCode);
	rCap->AddAudioAlg(eSirenLPRStereo_128kCapCode);
	rCap->SetH239Rate(1920);
	rCap->SetDynamicPTReplacement(TRUE);
	rCap->SetBfcpTransportType(bfcpTransportType);
}
/////////////////////////////////////////////////////////////////////////////
/*CH323Cap** CCapSetsList::Getm_capArrayPtr()
{
	return m_paCapArray;
}*/





/////////////////////////////////////////////////////////////////////////////
//
//   CCapSet - EP Capability element
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
CCapSet::CCapSet() // constructor
{
	SetName("DUMMY");

	m_nID			= 0xFFFFFFFF;
	m_isChanged		= TRUE;

	m_nCallRate		= 1920;
	m_isFecc		= TRUE;
//	m_isDPTR		= FALSE;
	m_isEncryption	= FALSE;
	m_isLPR			= FALSE;
	m_nH239Rate		= 1920;
	m_bfcpTransType = eTransportTypeUdp;

	// cleanup
	m_nAudAlgNum = 0;
	for( int i=0; i<MAX_AUDIO_ALG_NUM; i++ )
		m_audioPayloadTypes[i] = (DWORD)eUnknownAlgorithemCapCode;
	m_nVidProtocolNum = 0;
	for( int i=0; i<MAX_VIDEO_PROTOCOL_NUM; i++ )
		m_paVideoCaps[i] = 0;
	m_nSdesCapNum = 0;
	for( int i=0; i<MAX_SDES_CAP_NUM; i++ )
		m_paSdesCaps[i] = 0;

	// TIP
	//m_paVideoCaps[0] = 	new CVideoCapH264(eVideoModeHD1080);

	m_pVideoCapH264Presentation = new CVideoCapH264(eVideoModeHD720);

	m_pVideoCapH263Presentation = new CVideoCapH263();
	m_pVideoCapH263Presentation->CreatePresentationCap();

	DefaultAudioSet();
	DefaultVideoSet();
}


/////////////////////////////////////////////////////////////////////////////
CCapSet::CCapSet( const DWORD nId, const char* pszName ) // constructor
{
	SetName(pszName);
	m_nID = nId;

	m_isChanged		= TRUE;

	m_nCallRate		= 384;
	m_isFecc		= TRUE;
	//m_isDPTR		= FALSE;
	m_isEncryption	= FALSE;
	m_isLPR			= FALSE;
	m_nH239Rate		= 192;
	m_bfcpTransType = eTransportTypeUdp;

	// cleanup
	m_nAudAlgNum = 0;
	for( int i=0; i<MAX_AUDIO_ALG_NUM; i++ )
		m_audioPayloadTypes[i] = (DWORD)eUnknownAlgorithemCapCode;
	m_nVidProtocolNum = 0;
	for( int i=0; i<MAX_VIDEO_PROTOCOL_NUM; i++ )
		m_paVideoCaps[i] = 0;
	m_nSdesCapNum = 0;
	for( int i=0; i<MAX_SDES_CAP_NUM; i++ )
		m_paSdesCaps[i] = 0;

	m_pVideoCapH264Presentation = new CVideoCapH264(eVideoModeHD720);

	m_pVideoCapH263Presentation = new CVideoCapH263();
	m_pVideoCapH263Presentation->CreatePresentationCap();

	DefaultAudioSet();
	DefaultVideoSet();
}


/////////////////////////////////////////////////////////////////////////////
CCapSet::CCapSet( const CCapSet& other ) : CPObject(other) // constructor
{
	// cleanup
	m_nAudAlgNum = 0;
	for( int i=0; i<MAX_AUDIO_ALG_NUM; i++ )
		m_audioPayloadTypes[i] = (DWORD)eUnknownAlgorithemCapCode;
	m_nVidProtocolNum = 0;
	for( int i=0; i<MAX_VIDEO_PROTOCOL_NUM; i++ )
		m_paVideoCaps[i] = 0;
	m_nSdesCapNum = 0;
	for( int i=0; i<MAX_SDES_CAP_NUM; i++ )
		m_paSdesCaps[i] = 0;

	m_pVideoCapH264Presentation = NULL;
	m_pVideoCapH263Presentation = NULL;

	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
CCapSet::CCapSet( const CCapSet& first, const CCapSet& second,BOOL isTIP) // highest common constructor
{
	/// this function saves audio / video priorities of second cap set

	SetName("HIGHEST_COMMON");
	m_nID 				= 0xFFFFFFFF;
	m_isChanged   		= TRUE;

	m_nCallRate 		= (first.m_nCallRate<second.m_nCallRate) ? first.m_nCallRate : second.m_nCallRate;

	// cleanup
	m_nAudAlgNum = 0;
	for( int i=0; i<MAX_AUDIO_ALG_NUM; i++ )
		m_audioPayloadTypes[i] = (DWORD)eUnknownAlgorithemCapCode;
	m_nVidProtocolNum = 0;
	for( int i=0; i<MAX_VIDEO_PROTOCOL_NUM; i++ )
		m_paVideoCaps[i] = 0;
	m_nSdesCapNum = 0;
	for( int i=0; i<MAX_SDES_CAP_NUM; i++ )
		m_paSdesCaps[i] = 0;

	int i=0, j=0;
	for( i=0; i<second.m_nAudAlgNum; i++ )
	{
		BOOL  found = FALSE;
		for( j=0; j<first.m_nAudAlgNum && !found; j++ )
		{
			if( first.m_audioPayloadTypes[j] == second.m_audioPayloadTypes[i] )
				found = TRUE;
			else if(first.m_audioPayloadTypes[j] == eG7221C_CapCode)
				found = TRUE;
		}
		if( found )
			AddAudioAlg(second.m_audioPayloadTypes[i]);
	}

	//TIP support for testing embedded MLA feature
	if (isTIP == TRUE)
	{
		AddVideoProtocol(*(new CVideoCapH264(eVideoModeHD1080,-1,-1,H264_Profile_Main)));
	}

	for( i=0; i<second.m_nVidProtocolNum; i++ )
	{
		BOOL  found = FALSE;
		for( j=0; j<first.m_nVidProtocolNum; j++ )
		{
			//// TODO: add checks here
			if( second.m_paVideoCaps[i]->GetPayloadType() == first.m_paVideoCaps[j]->GetPayloadType() )
			{
				CVideoCap* pVideoCap = NULL;
				if( second.m_paVideoCaps[i]->GetPayloadType() == eH261CapCode )
					pVideoCap = new CVideoCapH261(*(first.m_paVideoCaps[j]),*(second.m_paVideoCaps[i]));
				else if( second.m_paVideoCaps[i]->GetPayloadType() == eH263CapCode )
					pVideoCap = new CVideoCapH263(*(first.m_paVideoCaps[j]),*(second.m_paVideoCaps[i]));
				else if( second.m_paVideoCaps[i]->GetPayloadType() == eH264CapCode )
					pVideoCap = new CVideoCapH264(*(first.m_paVideoCaps[j]),*(second.m_paVideoCaps[i]));
				else if( second.m_paVideoCaps[i]->GetPayloadType() == eVP8CapCode ) //N.A. DEBUG VP8
					pVideoCap = new CVideoCapVP8(*(first.m_paVideoCaps[j]),*(second.m_paVideoCaps[i]));
				else if( second.m_paVideoCaps[i]->GetPayloadType() == eRtvCapCode )
					pVideoCap = new CVideoCapRTV(*(first.m_paVideoCaps[j]),*(second.m_paVideoCaps[i]));
				else if( second.m_paVideoCaps[i]->GetPayloadType() == eMsSvcCapCode )
					pVideoCap = new CVideoCapMSSvc(*(first.m_paVideoCaps[j]),*(second.m_paVideoCaps[i]));
				else if( second.m_paVideoCaps[i]->GetPayloadType() == eSvcCapCode ) {
					pVideoCap = new CVideoCapSVC(*(first.m_paVideoCaps[j]),*(second.m_paVideoCaps[i]));
				}
				if( pVideoCap != NULL )
					AddVideoProtocol(*pVideoCap);

				POBJDELETE(pVideoCap);
				break;
			}
		}
	}

	if(GetVideoProtocolType(0)!=eUnknownAlgorithemCapCode
		&& NULL != first.m_pVideoCapH264Presentation
		&&  NULL != second.m_pVideoCapH264Presentation )
	{
		m_pVideoCapH264Presentation = new CVideoCapH264(*(first.m_pVideoCapH264Presentation),*(second.m_pVideoCapH264Presentation));
	}
	else
		m_pVideoCapH264Presentation = NULL;

	if(GetVideoProtocolType(0)!=eUnknownAlgorithemCapCode
		&& NULL != first.m_pVideoCapH263Presentation
		&&  NULL != second.m_pVideoCapH263Presentation )
	{
		m_pVideoCapH263Presentation = new CVideoCapH263(*(first.m_pVideoCapH263Presentation),*(second.m_pVideoCapH263Presentation));
	}
	else
		m_pVideoCapH263Presentation = NULL;

	m_isFecc       =       first.m_isFecc && second.m_isFecc;
	SetEncryption( first.m_isEncryption && second.m_isEncryption );

	m_nH239Rate	   = ( first.m_nH239Rate > second.m_nH239Rate ) ? second.m_nH239Rate : first.m_nH239Rate;
	//m_isDPTR = first.m_isDPTR && second.m_isDPTR;
	//m_isLPR		   = first.m_isLPR || second.m_isLPR;
	m_isLPR =  second.m_isLPR;
	m_bfcpTransType = second.m_bfcpTransType;
	m_isDPTR = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CCapSet::~CCapSet()     // destructor
{
	for( int i=0; i<m_nVidProtocolNum; i++ )
		POBJDELETE(m_paVideoCaps[i]);
	POBJDELETE(m_pVideoCapH264Presentation);
	POBJDELETE(m_pVideoCapH263Presentation);
	for( int i=0; i<m_nSdesCapNum; i++ )
		POBJDELETE(m_paSdesCaps[i]);

}

/////////////////////////////////////////////////////////////////////////////
BOOL CCapSet::IsH239() const
{
	if (m_nH239Rate)
	{
		if(GetPresentVideoProtocolType()!=eUnknownAlgorithemCapCode
			&& GetVideoProtocolType(0)!=eUnknownAlgorithemCapCode)
			return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::CreateFrom323Struct(ctCapabilitiesStruct* pCapStruct)
{
	Empty();

	DWORD audioRate = 64; // mak audio rate is 64k 640 bit/Hsync
	DWORD videoRate = 0;
	DWORD h239Rate  = 0;

	capBuffer* pCapBuffer = (capBuffer *) &(pCapStruct->caps);
	char*      tempPtr = (char*)pCapBuffer;
	for(int i = 0; i < pCapStruct->numberOfCaps; i++)
	{
		switch( (CapEnum)pCapBuffer->capTypeCode )
		{
			case eH261CapCode:
			{
				h261CapStruct* pCap261 = (h261CapStruct*) pCapBuffer->dataCap;
				if( pCap261->header.roleLabel == kRolePeople )
				{
					CVideoCapH261* pVideoCap = new CVideoCapH261(*pCap261);
					AddVideoProtocol(*pVideoCap);
					POBJDELETE(pVideoCap);
					videoRate = pCap261->maxBitRate; // in units of 100 Bps
					// translate to kBps
					if( videoRate % 10 == 0 )
						videoRate = videoRate/10;
					else
						videoRate = videoRate/10 + 1;
					// compute call rate
					if( videoRate > 64 )
					videoRate = (videoRate / 64) * 64;
				}
				break;
			}
			case eH263CapCode:
			{
				h263CapStruct* pCap263 = (h263CapStruct*) pCapBuffer->dataCap;
				if( pCap263->header.roleLabel == kRolePeople )
				{
					CVideoCapH263* pVideoCap = new CVideoCapH263(*pCap263);
					AddVideoProtocol(*pVideoCap);
					POBJDELETE(pVideoCap);
					videoRate = pCap263->maxBitRate; // in units of 100 Bps
					// translate to kBps
					if( videoRate % 10 == 0 )
						videoRate = videoRate/10;
					else
						videoRate = videoRate/10 + 1;
					// compute call rate
					if( videoRate > 64 )
						videoRate = (videoRate / 64) * 64;
				}
				else if( pCap263->header.roleLabel == kRolePresentation )
				{
					if(m_pVideoCapH263Presentation)
						POBJDELETE(m_pVideoCapH263Presentation);
					m_pVideoCapH263Presentation = new CVideoCapH263(*pCap263);
					h239Rate = pCap263->maxBitRate; // in units of 100 Bps
				}
				break;
			}
			case eH264CapCode:
			{
				h264CapStruct* pCap264 = (h264CapStruct*) pCapBuffer->dataCap;
				if( pCap264->header.roleLabel == kRolePeople )
				{
				    CVideoCapH264* pVideoCap = new CVideoCapH264(*pCap264);
					AddVideoProtocol(*pVideoCap);
					POBJDELETE(pVideoCap);
					videoRate = pCap264->maxBitRate; // in units of 100 Bps
					// translate to kBps
					if( videoRate % 10 == 0 )
						videoRate = videoRate/10;
					else
						videoRate = videoRate/10 + 1;
					// compute call rate
					if( videoRate > 64 )
					videoRate = (videoRate / 64) * 64;
				}
				else if( pCap264->header.roleLabel == kRolePresentation )
				{
					if(m_pVideoCapH264Presentation)
						POBJDELETE(m_pVideoCapH264Presentation);
					m_pVideoCapH264Presentation = new CVideoCapH264(*pCap264);
					h239Rate = pCap264->maxBitRate; // in units of 100 Bps
				}
				break;
			}
			case eRvFeccCapCode:
			case eAnnexQCapCode:
				m_isFecc = TRUE;
				break;
			case eH239ControlCapCode:
				m_nH239Rate = h239Rate / 10;
				break;
			//case eDynamicPTRCapCode:
			//	m_isDPTR = TRUE;
			//	break;
			case eEncryptionCapCode:
				m_isEncryption = TRUE;
				break;
			case eLPRCapCode:
			{
				lprCapStruct* pCapLpr = (lprCapStruct*) pCapBuffer->dataCap;
				if( pCapLpr->header.roleLabel == kRolePeople )
				{
					m_isLPR = TRUE;
				}
				break;
			}

			default:
				AddAudioAlg((CapEnum)pCapBuffer->capTypeCode);
				break;
		}
		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}
	m_nCallRate = videoRate+audioRate; // + 64k for audio
}

void CCapSet::CreateFromSipStruct(sipMediaLinesEntrySt* pMediaLinesEntry)
{
	Empty();

	DWORD audioRate = 64; // mak audio rate is 64k 640 bit/Hsync
	DWORD videoRate = 0;
	DWORD h239Rate  = 0;

	int mediaLinePos = 0;

	PTRACE2INT(eLevelInfoNormal, "pMediaLinesEntry->numberOfMediaLines:", pMediaLinesEntry->numberOfMediaLines);

	for (unsigned int j = 0; j < pMediaLinesEntry->numberOfMediaLines; j++) {

		sipMediaLineSt* pCapStruct = (sipMediaLineSt *) &pMediaLinesEntry->mediaLines[mediaLinePos];
		mediaLinePos += sizeof(sipMediaLineBaseSt) + pCapStruct->lenOfDynamicSection;

		capBuffer* pCapBuffer = (capBuffer *) &(pCapStruct->caps[0]);
		char*      tempPtr = (char*)pCapBuffer;
		for(unsigned int i = 0; i < pCapStruct->numberOfCaps; i++)
		{
			switch( (CapEnum)pCapBuffer->capTypeCode )
			{
				case eH261CapCode:
				{
					h261CapStruct* pCap261 = (h261CapStruct*) pCapBuffer->dataCap;
					if( pCap261->header.roleLabel == kRolePeople )
					{
						CVideoCapH261* pVideoCap = new CVideoCapH261(*pCap261);
						AddVideoProtocol(*pVideoCap);
						POBJDELETE(pVideoCap);
						videoRate = pCap261->maxBitRate; // in units of 100 Bps
						// translate to kBps
						if( videoRate % 10 == 0 )
							videoRate = videoRate/10;
						else
							videoRate = videoRate/10 + 1;
						// compute call rate
						if( videoRate > 64 )
						videoRate = (videoRate / 64) * 64;
					}
					break;
				}
				case eH263CapCode:
				{
					h263CapStruct* pCap263 = (h263CapStruct*) pCapBuffer->dataCap;
					if( pCap263->header.roleLabel == kRolePeople )
					{
						CVideoCapH263* pVideoCap = new CVideoCapH263(*pCap263);
						AddVideoProtocol(*pVideoCap);
						POBJDELETE(pVideoCap);
						videoRate = pCap263->maxBitRate; // in units of 100 Bps
						// translate to kBps
						if( videoRate % 10 == 0 )
							videoRate = videoRate/10;
						else
							videoRate = videoRate/10 + 1;
						// compute call rate
						if( videoRate > 64 )
							videoRate = (videoRate / 64) * 64;
					}
					else if( pCap263->header.roleLabel == kRolePresentation )
					{
						m_pVideoCapH263Presentation = new CVideoCapH263(*pCap263);
						h239Rate = pCap263->maxBitRate; // in units of 100 Bps
					}
					break;
				}
				case eH264CapCode:
				{
					h264CapStruct* pCap264 = (h264CapStruct*) pCapBuffer->dataCap;
					if( pCap264->header.roleLabel == kRolePeople )
					{
						CVideoCapH264* pVideoCap = new CVideoCapH264(*pCap264);
						AddVideoProtocol(*pVideoCap);
						POBJDELETE(pVideoCap);
						videoRate = pCap264->maxBitRate; // in units of 100 Bps
						// translate to kBps
						if( videoRate % 10 == 0 )
							videoRate = videoRate/10;
						else
							videoRate = videoRate/10 + 1;
						// compute call rate
						if( videoRate > 64 )
						videoRate = (videoRate / 64) * 64;
					}
					else if( pCap264->header.roleLabel == kRolePresentation )
					{
						m_pVideoCapH264Presentation = new CVideoCapH264(*pCap264);
						h239Rate = pCap264->maxBitRate; // in units of 100 Bps
					}
					break;
				}
				case eSvcCapCode:
				{
					svcCapStruct* pCapSVC = (svcCapStruct*) pCapBuffer->dataCap;
				    if (pCapSVC->h264.header.roleLabel == kRolePeople) {
				    	CVideoCapSVC* pVideoCap = new CVideoCapSVC(*pCapSVC);
					    AddVideoProtocol(*pVideoCap);
					    POBJDELETE(pVideoCap);
					    videoRate = pCapSVC->h264.maxBitRate; // in units of 100 Bps
					    // translate to kBps
					    if (videoRate % 10 == 0)
						    videoRate = videoRate / 10;
					    else
						    videoRate = videoRate / 10 + 1;
					    // compute call rate
					    if (videoRate > 64)
						    videoRate = (videoRate / 64) * 64;
				    } else if (pCapSVC->h264.header.roleLabel == kRolePresentation) {
					    PASSERT(1);// We don't support SVC content now
				    }
				    break;
			    }
				case eRvFeccCapCode:
				case eAnnexQCapCode:
					m_isFecc = TRUE;
					break;
				case eH239ControlCapCode:
					m_nH239Rate = h239Rate / 10;
					break;
			//	case eDynamicPTRCapCode:
			//		m_isDPTR = TRUE;
			//		break;
				case eEncryptionCapCode:
					m_isEncryption = TRUE;
					break;
				case eLPRCapCode:
				{
					lprCapStruct* pCapLpr = (lprCapStruct*) pCapBuffer->dataCap;
					if( pCapLpr->header.roleLabel == kRolePeople )
					{
						m_isLPR = TRUE;
					}
					break;
				}
				case eSdesCapCode:
				{
					m_isEncryption = TRUE;
					sdesCapStruct* pSdesCapStruct = (sdesCapStruct*) pCapBuffer->dataCap;
					CSdesEpCap* pSdesCap = new CSdesEpCap(*pSdesCapStruct);
					AddSdesCap(*pSdesCap);
					POBJDELETE(pSdesCap);
					break;
				}

				case eBFCPCapCode:
				{
					m_nH239Rate = 1920;
					if(pCapStruct->subType == eMediaLineSubTypeUdpBfcp)
						m_bfcpTransType = eTransportTypeUdp;
					else if(pCapStruct->subType == eMediaLineSubTypeTcpBfcp)
						m_bfcpTransType = eTransportTypeTcp;
					else if(pCapStruct->subType == eMediaLineSubTypeTcpTlsBfcp)
						m_bfcpTransType = eTransportTypeTls;
					break;
				}

				default:
					AddAudioAlg((CapEnum)pCapBuffer->capTypeCode);
					break;
			}
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
	}

	m_nCallRate = videoRate+audioRate; // + 64k for audio
}

/////////////////////////////////////////////////////////////////////////////
/// Allocates and fills mcIndCapabilities indication
WORD CCapSet::CreateCapStructH323(BYTE** ppIndication, const cmCapDirection eDirection, const BOOL bIsAnnexQ) const
{
	return CreateCapStructIP(ppIndication, eDirection, eDirection, eDirection, eProtocolH323, FALSE /* sip first cap*/, bIsAnnexQ);
}

/////////////////////////////////////////////////////////////////////////////
/// Allocates and fills mcIndCapabilities indication
WORD CCapSet::CreateCapStructIP(BYTE** ppIndication, const cmCapDirection eAudioDirection, const cmCapDirection eVideoDirection, const cmCapDirection eGeneralDirection, const TEpProtocolType protocol,
				const BOOL bIsSipFirstCap, const BOOL bIsAnnexQ) const
{
	WORD indLen = 0;
	BYTE numOfAlts = 0;
	BYTE numOfCaps = 0;
	BYTE numOfAudioCap = 0;
	BYTE numOfVideoCap = 0;
	BYTE numOfContentVideoCap = 0;
	BYTE numOfH239Cap  = 0;
	//BYTE numOfDPTRCap = 0;
	BYTE numOfFeccCap  = 0;
	BYTE numOfEncrypCap = 0;
	BYTE numOfLPRCap = 0;

	WORD   nAllBuffersLen = 0;
	BYTE*  pBytesBuffer = new BYTE [4096];
	memset(pBytesBuffer,0,4096);

	capBuffer* pCapBuffer = (capBuffer*)pBytesBuffer;
	char* tempPtr = (char*)pCapBuffer;

	// Add to cap buffer all audio algorithms
	for( int i=0; i<m_nAudAlgNum; i++ )
	{
		switch( m_audioPayloadTypes[i] )
		{
			case eG711Alaw64kCapCode:
			case eG711Alaw56kCapCode:
			case eG711Ulaw64kCapCode:
			case eG711Ulaw56kCapCode:
			case eG722_64kCapCode:
			case eG722_56kCapCode:
			case eG722_48kCapCode:
			case eG728CapCode:
			{
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(audioCapStructBase);
				pCapBuffer->capTypeCode					= m_audioPayloadTypes[i];
				pCapBuffer->sipPayloadType				= GetPayloadType(m_audioPayloadTypes[i]);
				pCapBuffer->capLength 					= sizeof(audioCapStructBase);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(audioCapStructBase);

				audioCapStructBase* pAudioStruct = (audioCapStructBase*)pCapBuffer->dataCap;
				pAudioStruct->header.xmlHeader.dynamicType   = m_audioPayloadTypes[i];
				pAudioStruct->header.xmlHeader.dynamicLength = sizeof(audioCapStructBase);
				pAudioStruct->header.direction   = eAudioDirection;
				pAudioStruct->header.type        = cmCapAudio;
				pAudioStruct->header.roleLabel   = kRolePeople;
				pAudioStruct->header.capTypeCode = m_audioPayloadTypes[i];
				pAudioStruct->maxValue = NonFrameBasedFPP;
				pAudioStruct->minValue = 0;

				numOfAudioCap++;
				break;
			}
			case eG729CapCode:
			case eG729AnnexACapCode:
			case eG729wAnnexBCapCode:
			case eG729AnnexAwAnnexBCapCode:
			case eG7231CapCode:
			case eG7221_32kCapCode:
			case eG7221_24kCapCode:
			case eG7221_16kCapCode:
			case eSiren14_48kCapCode:
			case eSiren14_32kCapCode:
			case eSiren14_24kCapCode:
			//case eOpus_CapCode:
			case eG7221C_48kCapCode:
			case eG7221C_32kCapCode:
			case eG7221C_24kCapCode:
			case eRfc2833DtmfCapCode:
			case eSiren14Stereo_48kCapCode:
			case eSiren14Stereo_64kCapCode:
			case eSiren14Stereo_96kCapCode:
			//case eOpusStereo_CapCode:
			case eG719Stereo_128kCapCode:
			case eG719Stereo_96kCapCode:
			case eG719Stereo_64kCapCode:
			case eG719_64kCapCode:
			case eG719_48kCapCode:
			case eG719_32kCapCode:
			case eSiren22Stereo_128kCapCode:
			case eSiren22Stereo_96kCapCode:
			case eSiren22Stereo_64kCapCode:
			case eSiren22_64kCapCode:
			case eSiren22_48kCapCode:
			case eSiren22_32kCapCode:
			case eG722Stereo_128kCapCode:
			{
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(audioCapStructBase);
				pCapBuffer->capTypeCode					= m_audioPayloadTypes[i];
				pCapBuffer->sipPayloadType				= GetPayloadType(m_audioPayloadTypes[i]);
				pCapBuffer->capLength 					= sizeof(audioCapStructBase);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(audioCapStructBase);

				audioCapStructBase* pAudioStruct = (audioCapStructBase*)pCapBuffer->dataCap;
				pAudioStruct->header.xmlHeader.dynamicType   = m_audioPayloadTypes[i];
				pAudioStruct->header.xmlHeader.dynamicLength = sizeof(audioCapStructBase);
				pAudioStruct->header.direction   = eAudioDirection;
				pAudioStruct->header.type        = cmCapAudio;
				pAudioStruct->header.roleLabel   = kRolePeople;
				pAudioStruct->header.capTypeCode = m_audioPayloadTypes[i];
				pAudioStruct->maxValue = FrameBasedFPP;
				pAudioStruct->minValue = 0;

				if(eG729AnnexACapCode)
					pAudioStruct->maxValue = 2*FrameBasedFPP;

				numOfAudioCap++;
				break;
			}

			case eSirenLPR_32kCapCode:
			case eSirenLPR_48kCapCode:
			case eSirenLPR_64kCapCode:
			case eSirenLPRStereo_64kCapCode:
			case eSirenLPRStereo_96kCapCode:
			case eSirenLPRStereo_128kCapCode:
			{
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(sirenLPR_CapStruct);
				pCapBuffer->capTypeCode					= m_audioPayloadTypes[i];
				pCapBuffer->sipPayloadType				= GetPayloadType(m_audioPayloadTypes[i]);
				pCapBuffer->capLength 					= sizeof(sirenLPR_CapStruct);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(sirenLPR_CapStruct);

				sirenLPR_CapStruct* pAudioStruct = (sirenLPR_CapStruct*)pCapBuffer->dataCap;
				pAudioStruct->header.xmlHeader.dynamicType   = m_audioPayloadTypes[i];
				pAudioStruct->header.xmlHeader.dynamicLength = sizeof(sirenLPR_CapStruct);
				pAudioStruct->header.direction   = eAudioDirection;
				pAudioStruct->header.type        = cmCapAudio;
				pAudioStruct->header.roleLabel   = kRolePeople;
				pAudioStruct->header.capTypeCode = m_audioPayloadTypes[i];
				pAudioStruct->maxValue = FrameBasedFPP;
				pAudioStruct->minValue = 0;
				pAudioStruct->sirenLPRMask = (i >= eSirenLPRStereo_64kCapCode) ? sirenLPRMono : sirenLPRStereo;

				numOfAudioCap++;
				break;
			}
			case eG7221C_CapCode:
			{
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(g7221C_CapStruct);
				pCapBuffer->capTypeCode					= m_audioPayloadTypes[i];
				pCapBuffer->capLength 					= sizeof(g7221C_CapStruct);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(g7221C_CapStruct);

				g7221C_CapStruct* pAudioStruct = (g7221C_CapStruct*)pCapBuffer->dataCap;
				pAudioStruct->header.xmlHeader.dynamicType   = m_audioPayloadTypes[i];
				pAudioStruct->header.xmlHeader.dynamicLength = sizeof(g7221C_CapStruct);
				pAudioStruct->header.direction   = eAudioDirection;
				pAudioStruct->header.type        = cmCapAudio;
				pAudioStruct->header.roleLabel   = kRolePeople;
				pAudioStruct->header.capTypeCode = m_audioPayloadTypes[i];
				pAudioStruct->maxValue = FrameBasedFPP;
				pAudioStruct->minValue = 0;
				pAudioStruct->capBoolMask = g7221C_Mask_Rate48K | g7221C_Mask_Rate32K | g7221C_Mask_Rate24K;

				numOfAudioCap++;
				break;
			}
			case eIS11172AudioCapCode:
			{
//				pAudioStruct->maxValue = NonFrameBasedFPP;
//				numOfAudioCap++;
				break;
			}
			case eIS13818CapCode:
			{
//				pAudioStruct->maxValue = NonFrameBasedFPP;
//				numOfAudioCap++;
				break;
			}
			case eG7231AnnexCapCode:
			{
//				pAudioStruct->maxValue = FrameBasedFPP;
//				numOfAudioCap++;
				break;
			}

			case eAAC_LDCapCode:
			{
				//TIP support for testing embedded MLA feature
				if (CCapSetsList::m_isTIP == TRUE)
				{
					PTRACE(eLevelInfoNormal,"CCapSet::CreateCapStructH323 - eAAC_LDCapCode");
					pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
					pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(AAC_LDCapStruct);
					pCapBuffer->capTypeCode					= m_audioPayloadTypes[i];
					pCapBuffer->sipPayloadType				= GetPayloadType(m_audioPayloadTypes[i]);
					pCapBuffer->capLength 					= sizeof(AAC_LDCapStruct);
					pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
					pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(AAC_LDCapStruct);

					AAC_LDCapStruct *pAACLD = (AAC_LDCapStruct*)pCapBuffer->dataCap;

					pAACLD->header.xmlHeader.dynamicType   = m_audioPayloadTypes[i];
					pAACLD->header.xmlHeader.dynamicLength = sizeof(AAC_LDCapStruct);
					pAACLD->header.direction   = eAudioDirection;
					pAACLD->header.type        = cmCapAudio;
					pAACLD->header.roleLabel   = kRolePeople;

					pAACLD->profileLevelId 		= 16;
					pAACLD->streamType			= 5;
					strncpy(pAACLD->mode, "AAC-hbr", MAX_AACLD_MODE);
					strncpy(pAACLD->config, "B98C00", MAX_AACLD_CONFIG);
					pAACLD->sizeLength 			= 13;
					pAACLD->indexLength 		= 3;
					pAACLD->indexDeltaLength 	= 3;
					pAACLD->constantDuration 	= 480;

					numOfAudioCap++;

					break;
				}
			    case eSirenLPR_Scalable_32kCapCode:
			    case eSirenLPR_Scalable_48kCapCode:
			    case eSirenLPR_Scalable_64kCapCode:
			    case eSirenLPRStereo_Scalable_64kCapCode:
			    case eSirenLPRStereo_Scalable_96kCapCode:
			    case eSirenLPRStereo_Scalable_128kCapCode:
			    {
			    	PTRACE(eLevelInfoNormal,"CCapSet::CreateCapStructIP - eSirenLPR_Scalable_*kCapCode or eSirenLPRStereo_Scalable_*kCapCode");
				    pCapBuffer->xmlHeader.dynamicType = tblCapBuffer;
				    pCapBuffer->xmlHeader.dynamicLength = sizeof(capBufferBase) + sizeof(sirenLPR_Scalable_CapStruct);
				    pCapBuffer->capTypeCode = m_audioPayloadTypes[i];
				    pCapBuffer->sipPayloadType = GetPayloadType(m_audioPayloadTypes[i]);
				    pCapBuffer->capLength = sizeof(sirenLPR_Scalable_CapStruct);
				    pCapBuffer->xmlDynamicProps.numberOfDynamicParts = 1;
				    pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts =	sizeof(sirenLPR_Scalable_CapStruct);

				    sirenLPR_Scalable_CapStruct *pSAC = (sirenLPR_Scalable_CapStruct*) pCapBuffer->dataCap;
				    pSAC->header.xmlHeader.dynamicType   = m_audioPayloadTypes[i];
				    pSAC->header.xmlHeader.dynamicLength = sizeof(sirenLPR_Scalable_CapStruct);
				    pSAC->header.direction   = eAudioDirection;
				    pSAC->header.type        = cmCapAudio;
				    pSAC->header.roleLabel   = kRolePeople;
				    pSAC->header.capTypeCode = m_audioPayloadTypes[i];
				    pSAC->maxValue = FrameBasedFPP;
				    pSAC->minValue = 0;
				    pSAC->sirenLPRMask = (i >= eSirenLPRStereo_Scalable_64kCapCode) ? sirenLPRMono : sirenLPRStereo;

				    pSAC->rtcpFeedbackMask = 0;
				    pSAC->sampleRate = 48;
				    pSAC->mixDepth = 3;
				    pSAC->recvStreamsGroup.numberOfStreams = 0; //?
				    pSAC->sendStreamsGroup.numberOfStreams = 3;
				    pSAC->sendStreamsGroup.streamGroupId = 916; //?
				    pSAC->sendStreamsGroup.streams[0].streamSsrcId = 110401;
				    pSAC->sendStreamsGroup.streams[1].streamSsrcId = 110402;
				    pSAC->sendStreamsGroup.streams[2].streamSsrcId = 110403;


			    	numOfAudioCap++;
			    	break;
			    }
			}

		}
		nAllBuffersLen += sizeof(capBufferBase) + pCapBuffer->capLength;
		// move ptr to next capBuffer position
		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}

	PTRACE2INT(eLevelInfoNormal,"CCapSet::CreateCapStructIP - m_nVidProtocolNum:", m_nVidProtocolNum);
	// Add to cap buffer all video protocols
	for( int i=0; i<m_nVidProtocolNum; i++ )
	{
		switch( m_paVideoCaps[i]->GetPayloadType() )
		{
			case eH261CapCode:
			{
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(h261CapStruct);//sizeof(h263CapStructBase);
				pCapBuffer->capTypeCode					= (BYTE)eH261CapCode;
				pCapBuffer->sipPayloadType				= GetPayloadType(eH261CapCode);
				pCapBuffer->capLength 					= sizeof(h261CapStruct);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(h261CapStruct);

				h261CapStruct* pVideoStruct = (h261CapStruct*)pCapBuffer->dataCap;

				((CVideoCapH261*)m_paVideoCaps[i])->FillStructH323(pVideoStruct,
						(m_nCallRate-64)*10,eVideoDirection,kRolePeople);

				numOfVideoCap++;
				break;
			}
			case eH263CapCode:
			{
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(h263CapStruct);//sizeof(h263CapStructBase);
				pCapBuffer->capTypeCode					= (BYTE)eH263CapCode;
				pCapBuffer->sipPayloadType				= GetPayloadType(eH263CapCode);
				pCapBuffer->capLength 					= sizeof(h263CapStruct);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(h263CapStruct);

				h263CapStruct* pVideoStruct = (h263CapStruct*)pCapBuffer->dataCap;

				((CVideoCapH263*)m_paVideoCaps[i])->FillStructH323(pVideoStruct,
						(m_nCallRate-64)*10,eVideoDirection,kRolePeople);

				numOfVideoCap++;
				break;
			}
			case eH264CapCode:
			{
				PTRACE(eLevelInfoNormal,"CCapSet::CreateCapStructIP - eH264CapCode");
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(h264CapStruct);//sizeof(h263CapStructBase);
				pCapBuffer->capTypeCode					= (BYTE)eH264CapCode;
				pCapBuffer->sipPayloadType				= GetPayloadType(eH264CapCode);
				pCapBuffer->capLength 					= sizeof(h264CapStruct);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(h264CapStruct);

				h264CapStruct* pVideoStruct = (h264CapStruct*)pCapBuffer->dataCap;

				((CVideoCapH264*)m_paVideoCaps[i])->FillStructH323( pVideoStruct,
						(m_nCallRate-48)*10,eVideoDirection,kRolePeople);

				numOfVideoCap++;
				break;
			}
			case eSvcCapCode:
			{
				PTRACE(eLevelInfoNormal,"CCapSet::CreateCapStructIP - eSvcCapCode");
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(svcCapStruct);
				pCapBuffer->capTypeCode					= (BYTE)eSvcCapCode;
				pCapBuffer->sipPayloadType				= GetPayloadType(eSvcCapCode);
				pCapBuffer->capLength 					= sizeof(svcCapStruct);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(svcCapStruct);

				svcCapStruct* pVideoStruct = (svcCapStruct*)pCapBuffer->dataCap;

				((CVideoCapSVC*)m_paVideoCaps[i])->FillStructH323( pVideoStruct,
						(m_nCallRate-48)*10,eVideoDirection,kRolePeople);

				numOfVideoCap++;
				break;
			}
			case eVP8CapCode:
			{//N.A. DEBUG VP8

				PTRACE(eLevelInfoNormal,"CCapSet::CreateCapStructH323 - eVP8CapCode");
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(eVP8CapCode);
				pCapBuffer->capTypeCode					= (BYTE)eVP8CapCode;
				pCapBuffer->sipPayloadType				= GetPayloadType(eVP8CapCode);
				pCapBuffer->capLength 					= sizeof(vp8CapStruct);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(vp8CapStruct);

				vp8CapStruct* pVideoStruct = (vp8CapStruct*)pCapBuffer->dataCap;

				((CVideoCapVP8*)m_paVideoCaps[i])->FillStructH323( pVideoStruct,
						(m_nCallRate-48)*10,eVideoDirection,kRolePeople);

				numOfVideoCap++;
				break;
			}
			case eRtvCapCode:
			{
				PTRACE(eLevelInfoNormal,"CCapSet::CreateCapStructIP - eRtvCapCode");
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(rtvCapStruct);
				pCapBuffer->capTypeCode					= (BYTE)eRtvCapCode;
				pCapBuffer->sipPayloadType				= GetPayloadType(eRtvCapCode);
				pCapBuffer->capLength 					= sizeof(rtvCapStruct);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(rtvCapStruct);
				rtvCapStruct* pVideoStruct = (rtvCapStruct*)pCapBuffer->dataCap;
				((CVideoCapRTV*)m_paVideoCaps[i])->FillStructH323( pVideoStruct,
						(m_nCallRate-48)*10,eVideoDirection,kRolePeople);
				numOfVideoCap++;
				break;
			}
			case eMsSvcCapCode:
			{
				PTRACE(eLevelInfoNormal,"CCapSet::CreateCapStructIP - mssvc cap");
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(msSvcCapStruct);
				pCapBuffer->capTypeCode					= (BYTE)eMsSvcCapCode;
				pCapBuffer->sipPayloadType				= GetPayloadType(eMsSvcCapCode);
				pCapBuffer->capLength 					= sizeof(msSvcCapStruct);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(msSvcCapStruct);
				msSvcCapStruct* pVideoStruct = (msSvcCapStruct*)pCapBuffer->dataCap;
				((CVideoCapMSSvc*)m_paVideoCaps[i])->FillStructH323( pVideoStruct,
						(m_nCallRate-48)*10,eVideoDirection,kRolePeople);
				numOfVideoCap++;
				PTRACE2INT(eLevelInfoNormal,"CCapSet::CreateCapStructIP - mssvc cap numOfVideoCap",numOfVideoCap);
				break;


			}
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}
		nAllBuffersLen += sizeof(capBufferBase) + pCapBuffer->capLength;
		// move ptr to next capBuffer position
		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;
	}

	if( ((m_nH239Rate != 0 && eProtocolSip != protocol) || (eProtocolSip == protocol && IsBfcpSupported() )) && m_nVidProtocolNum > 0)
	{
		BOOL bDeclareVideoContentCap = TRUE; // if SIP first cap we don't declare Video Content caps

		if( eProtocolSip != protocol ) // H323
		{
			pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
			pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(h239ControlCapStruct);
			pCapBuffer->capTypeCode					= (BYTE)eH239ControlCapCode;
			pCapBuffer->capLength 					= sizeof(h239ControlCapStruct);
			pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
			pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(h239ControlCapStruct);

			h239ControlCapStruct* pH239Struct = (h239ControlCapStruct*)pCapBuffer->dataCap;
			pH239Struct->header.xmlHeader.dynamicType   = eH239ControlCapCode;
			pH239Struct->header.xmlHeader.dynamicLength = sizeof(h239ControlCapStruct);
			pH239Struct->header.direction   = eGeneralDirection;
			pH239Struct->header.type        = cmCapGeneric;
			pH239Struct->header.roleLabel   = kRolePeople;
			pH239Struct->header.capTypeCode = eH239ControlCapCode;
			pH239Struct->maxValue = 0; // FrameBasedFPP;
			pH239Struct->minValue = 0;

			numOfH239Cap++;

			nAllBuffersLen += sizeof(capBufferBase) + pCapBuffer->capLength;
			// move ptr to next capBuffer position
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
		else// Sip
		{
			/*
typedef struct{
	ctCapStruct			header;
	APIU8				floorctrl;
	APIS8				confid[32];
	APIS8				userid[32];
	bfcpFlooridStruct	floorid_0;
	bfcpFlooridStruct	floorid_1;
	bfcpFlooridStruct	floorid_2;
	bfcpFlooridStruct	floorid_3;
//	APIU8				setup;
//	APIU8				connection;
	APIU8				xbfcp_info_enabled;
	APIU16				xbfcp_info_time;
}bfcpCapStruct;

			 */
			pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
			pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(bfcpCapStruct);
			pCapBuffer->capTypeCode					= (BYTE)eBFCPCapCode;
			pCapBuffer->capLength 					= sizeof(bfcpCapStruct);
			pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
			pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(bfcpCapStruct);

			bfcpCapStruct* pBfcpStruct = (bfcpCapStruct*)pCapBuffer->dataCap;
			pBfcpStruct->header.xmlHeader.dynamicType   = eBFCPCapCode;
			pBfcpStruct->header.xmlHeader.dynamicLength = sizeof(bfcpCapStruct);
			pBfcpStruct->header.direction   = eGeneralDirection;
			pBfcpStruct->header.type        = cmCapBfcp;
			pBfcpStruct->header.roleLabel   = kRolePeople;
			pBfcpStruct->header.capTypeCode = eBFCPCapCode;
			pBfcpStruct->floorctrl = 1;
			pBfcpStruct->confid[0] = 0;
			pBfcpStruct->userid[0] = 0;
			pBfcpStruct->xbfcp_info_enabled = 0;
			pBfcpStruct->xbfcp_info_time = 0;
			pBfcpStruct->connection = bfcp_connection_new;
			pBfcpStruct->setup = bfcp_setup_actpass;
			pBfcpStruct->transType = eTransportTypeUdp;

			numOfH239Cap++;

			nAllBuffersLen += sizeof(capBufferBase) + pCapBuffer->capLength;
			// move ptr to next capBuffer position
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;

			if( TRUE == bIsSipFirstCap )
				bDeclareVideoContentCap = FALSE;
		}

		if( TRUE == bDeclareVideoContentCap  &&  NULL != m_pVideoCapH264Presentation )
		{
			pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
			pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(h264CapStruct);//sizeof(h263CapStructBase);
			pCapBuffer->capTypeCode					= (BYTE)eH264CapCode;
			pCapBuffer->capLength 					= sizeof(h264CapStruct);
			pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
			pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(h264CapStruct);

			h264CapStruct* pVideoStruct = (h264CapStruct*)pCapBuffer->dataCap;

			m_pVideoCapH264Presentation->FillStructH323(pVideoStruct,m_nH239Rate*10,cmCapReceive,kRolePresentation);

			nAllBuffersLen += sizeof(capBufferBase) + pCapBuffer->capLength;
			// move ptr to next capBuffer position
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
			numOfContentVideoCap++;
		}

		if( TRUE == bDeclareVideoContentCap  &&  NULL != m_pVideoCapH263Presentation )
		{
			#define CUSTOM_FORMATS_OFF_MASK ~(0x000003ff)//bits 22-31
			typedef struct{
				h263CapStructBase			base263; // without last field.
				h263OptionsStruct			annex1;
				customPic_StBase			customBase;
				customPicFormatSt			custom1;
				customPicFormatSt			custom2;
				customPicFormatSt			custom3;
			}h263WithContentSt;

			int customFormatNo = 3;
			int annexesNo = 1;
			int structureSizeOf = 0;

			structureSizeOf = sizeof(h263CapStructBase) + (annexesNo*sizeof(h263OptionsStruct)) + sizeof(customPic_StBase) + (customFormatNo*sizeof(customPicFormatSt));

			pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
			pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + structureSizeOf;//sizeof(h263CapStruct);//sizeof(h263CapStructBase);
			pCapBuffer->capTypeCode					= (BYTE)eH263CapCode;
			pCapBuffer->capLength 					= structureSizeOf;
			pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
			pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = structureSizeOf;

			h263CapStruct* pVideoStruct = (h263CapStruct*)pCapBuffer->dataCap;
			h263WithContentSt* pCustomVideoStruct = (h263WithContentSt*)pCapBuffer->dataCap;

			m_pVideoCapH263Presentation->FillStructH323(pVideoStruct,m_nH239Rate*10,cmCapReceive,kRolePresentation);
			pVideoStruct->header.xmlHeader.dynamicLength = structureSizeOf;
			pVideoStruct->capBoolMask = 0;

			// set mask
			pVideoStruct->annexesMask.fds_bits[0] &= CUSTOM_FORMATS_OFF_MASK;
			DWORD startIndex = H263_Annexes_Number;
			for (DWORD maskIndex = startIndex; maskIndex < startIndex+customFormatNo; maskIndex++)
				CAP_FD_SET(maskIndex, &(pVideoStruct->annexesMask));
			CAP_FD_SET(typeAnnexT, &(pVideoStruct->annexesMask));

			pVideoStruct->xmlDynamicProps.numberOfDynamicParts = 2;// 2 = 1 annexes dynamic + 1 custom format dynamic;
			pVideoStruct->xmlDynamicProps.sizeOfAllDynamicParts = structureSizeOf - sizeof(h263CapStructBase);

			//set the annex T;
			pCustomVideoStruct->annex1.annexT.xmlHeader.dynamicType = typeAnnexT;
			pCustomVideoStruct->annex1.annexT.xmlHeader.dynamicLength = sizeof(h263OptionsStruct);
			pCustomVideoStruct->annex1.annexT.annexBoolMask = 0;
			pCustomVideoStruct->annex1.annexT.annexBoolMask |= annexT_modifiedQuantizationMode;

			// set general customPic
			pCustomVideoStruct->customBase.xmlHeader.dynamicType = tblCustomFormat;//typeCustomPic;
			pCustomVideoStruct->customBase.xmlHeader.dynamicLength = sizeof(customPic_StBase) + (customFormatNo*sizeof(customPicFormatSt));
			pCustomVideoStruct->customBase.customPictureClockFrequency.clockConversionCode = -1;
			pCustomVideoStruct->customBase.customPictureClockFrequency.clockDivisor = -1;
			pCustomVideoStruct->customBase.customPictureClockFrequency.sqcifMPI = -1;
			pCustomVideoStruct->customBase.customPictureClockFrequency.qcifMPI = -1;
			pCustomVideoStruct->customBase.customPictureClockFrequency.cifMPI = -1;
			pCustomVideoStruct->customBase.customPictureClockFrequency.cif4MPI = -1;
			pCustomVideoStruct->customBase.customPictureClockFrequency.cif16MPI = -1;
			pCustomVideoStruct->customBase.numberOfCustomPic = customFormatNo;
			pCustomVideoStruct->customBase.xmlDynamicProps.numberOfDynamicParts = customFormatNo;
			pCustomVideoStruct->customBase.xmlDynamicProps.sizeOfAllDynamicParts = customFormatNo*sizeof(customPicFormatSt);

			// set the first custom format
			pCustomVideoStruct->custom1.xmlHeader.dynamicType = typeCustomPic;
			pCustomVideoStruct->custom1.xmlHeader.dynamicLength = sizeof(customPicFormatSt);
			pCustomVideoStruct->custom1.maxCustomPictureWidth = 160;
			pCustomVideoStruct->custom1.maxCustomPictureHeight = 120;
			pCustomVideoStruct->custom1.minCustomPictureWidth = 160;
			pCustomVideoStruct->custom1.minCustomPictureHeight = 120;
			pCustomVideoStruct->custom1.customMPI = 0;
			pCustomVideoStruct->custom1.clockConversionCode = 0;
			pCustomVideoStruct->custom1.clockDivisor = 0;
			pCustomVideoStruct->custom1.standardMPI = 2;
			pCustomVideoStruct->custom1.pixelAspectCode[0] = 1;

			// set the first custom format
			pCustomVideoStruct->custom2.xmlHeader.dynamicType = typeCustomPic;
			pCustomVideoStruct->custom2.xmlHeader.dynamicLength = sizeof(customPicFormatSt);
			pCustomVideoStruct->custom2.maxCustomPictureWidth = 200;
			pCustomVideoStruct->custom2.maxCustomPictureHeight = 150;
			pCustomVideoStruct->custom2.minCustomPictureWidth = 200;
			pCustomVideoStruct->custom2.minCustomPictureHeight = 150;
			pCustomVideoStruct->custom2.customMPI = 0;
			pCustomVideoStruct->custom2.clockConversionCode = 0;
			pCustomVideoStruct->custom2.clockDivisor = 0;
			pCustomVideoStruct->custom2.standardMPI = 3;
			pCustomVideoStruct->custom2.pixelAspectCode[0] = 1;

			// set the first custom format
			pCustomVideoStruct->custom3.xmlHeader.dynamicType = typeCustomPic;
			pCustomVideoStruct->custom3.xmlHeader.dynamicLength = sizeof(customPicFormatSt);
			pCustomVideoStruct->custom3.maxCustomPictureWidth = 256;
			pCustomVideoStruct->custom3.maxCustomPictureHeight = 192;
			pCustomVideoStruct->custom3.minCustomPictureWidth = 256;
			pCustomVideoStruct->custom3.minCustomPictureHeight = 192;
			pCustomVideoStruct->custom3.customMPI = 0;
			pCustomVideoStruct->custom3.clockConversionCode = 0;
			pCustomVideoStruct->custom3.clockDivisor = 0;
			pCustomVideoStruct->custom3.standardMPI = 4;
			pCustomVideoStruct->custom3.pixelAspectCode[0] = 1;


			nAllBuffersLen += sizeof(capBufferBase) + pCapBuffer->capLength;
			// move ptr to next capBuffer position
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
			numOfContentVideoCap++;
		}
	}


/*
	if( m_isDPTR )
	{

		if( protocol != eProtocolSip ) // H323
		{
			pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
			pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(DynamicPTRCapStruct);
			pCapBuffer->capTypeCode					= (BYTE)eDynamicPTRCapCode;
			pCapBuffer->capLength 					= sizeof(DynamicPTRCapStruct);
			pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
			pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(DynamicPTRCapStruct);

			DynamicPTRCapStruct* pDPTRStruct = (DynamicPTRCapStruct*)pCapBuffer->dataCap;
			pDPTRStruct->header.xmlHeader.dynamicType   = eDynamicPTRCapCode;
			pDPTRStruct->header.xmlHeader.dynamicLength = sizeof(DynamicPTRCapStruct);
			pDPTRStruct->header.direction   = eGeneralDirection;
			pDPTRStruct->header.type        = cmCapGeneric;
			pDPTRStruct->header.roleLabel   = kRolePeople;
			pDPTRStruct->header.capTypeCode = eDynamicPTRCapCode;
			pDPTRStruct->maxValue = 0; // FrameBasedFPP;
			pDPTRStruct->minValue = 0;

			numOfDPTRCap++;

			nAllBuffersLen += sizeof(capBufferBase) + pCapBuffer->capLength;
			// move ptr to next capBuffer position
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
	}

*/
	if( m_isFecc )
	{
		if(bIsAnnexQ)
		{
			pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
			pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(annexQDataCapStruct);//sizeof(h263CapStructBase);
			pCapBuffer->capTypeCode					= (BYTE)eAnnexQCapCode;
			pCapBuffer->sipPayloadType				= GetPayloadType(eAnnexQCapCode);
			pCapBuffer->capLength 					= sizeof(annexQDataCapStruct);
			pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
			pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(annexQDataCapStruct);

			annexQDataCapStruct* pFeccStruct = (annexQDataCapStruct*)pCapBuffer->dataCap;
			pFeccStruct->header.xmlHeader.dynamicType   = eAnnexQCapCode;
			pFeccStruct->header.xmlHeader.dynamicLength = sizeof(annexQDataCapStruct);
			pFeccStruct->header.direction   = eGeneralDirection;
			pFeccStruct->header.type        = cmCapData;
			pFeccStruct->header.roleLabel   = kRolePeople;
			pFeccStruct->header.capTypeCode = eAnnexQCapCode;

			pFeccStruct->maxBitRate = 64; // units of 100 bit/s

			numOfFeccCap++;

			nAllBuffersLen += sizeof(capBufferBase) + pCapBuffer->capLength;
			// move ptr to next capBuffer position
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
		else
		{
			pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
			pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(rvFeccDataCapStruct);//sizeof(h263CapStructBase);
			pCapBuffer->capTypeCode					= (BYTE)eRvFeccCapCode;
			pCapBuffer->sipPayloadType				= GetPayloadType(eRvFeccCapCode);
			pCapBuffer->capLength 					= sizeof(rvFeccDataCapStruct);
			pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
			pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(rvFeccDataCapStruct);

			rvFeccDataCapStruct* pFeccStruct = (rvFeccDataCapStruct*)pCapBuffer->dataCap;
			pFeccStruct->header.xmlHeader.dynamicType   = eRvFeccCapCode;
			pFeccStruct->header.xmlHeader.dynamicLength = sizeof(rvFeccDataCapStruct);
			pFeccStruct->header.direction   = eGeneralDirection;
			pFeccStruct->header.type        = cmCapData;
			pFeccStruct->header.roleLabel   = kRolePeople;
			pFeccStruct->header.capTypeCode = eRvFeccCapCode;

			pFeccStruct->maxBitRate = 64; // units of 100 bit/s

			numOfFeccCap++;

			nAllBuffersLen += sizeof(capBufferBase) + pCapBuffer->capLength;
			// move ptr to next capBuffer position
			tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
			pCapBuffer = (capBuffer*)tempPtr;
		}
	}

	//add LPR
	if (m_isLPR)
	{
		pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
		pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(lprCapStruct);
		pCapBuffer->capTypeCode					= (BYTE)eLPRCapCode;
		pCapBuffer->sipPayloadType				= GetPayloadType(eLPRCapCode);
		pCapBuffer->capLength 					= sizeof(lprCapStruct);
		pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
		pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(lprCapStruct);

		lprCapStruct* pLPRCapStruct = (lprCapStruct*)pCapBuffer->dataCap;
		pLPRCapStruct->header.xmlHeader.dynamicType = eLPRCapCode;
		pLPRCapStruct->header.xmlHeader.dynamicLength = sizeof(lprCapStruct);
		pLPRCapStruct->header.direction   = eGeneralDirection;
		pLPRCapStruct->header.type        = cmCapGeneric;
		pLPRCapStruct->header.roleLabel   = kRolePeople;
		pLPRCapStruct->header.capTypeCode = eLPRCapCode;

		pLPRCapStruct->versionID 			= 1;
		pLPRCapStruct->minProtectionPeriod  = 150;
		pLPRCapStruct->maxProtectionPeriod  = 150;
		pLPRCapStruct->maxRecoverySet		= 52;
		pLPRCapStruct->maxRecoveryPackets   = 10;
		pLPRCapStruct->maxPacketSize     	= 1260;

		numOfLPRCap++;


		nAllBuffersLen += sizeof(capBufferBase) + pCapBuffer->capLength;
		// move ptr to next capBuffer position
		tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
		pCapBuffer = (capBuffer*)tempPtr;

	}

	// add encryption
	if( m_isEncryption )
	{
		WORD        entry  = 1;
		capBuffer*	pCapBufferA	= (capBuffer *) pBytesBuffer;
    	char*		pTempPtrA	= (char*)pCapBufferA;

		numOfCaps = numOfAudioCap + numOfVideoCap + numOfContentVideoCap + numOfFeccCap;

		for( int i=0; i<numOfCaps; i++ )
		{
			if( (pCapBufferA->capTypeCode != ePeopleContentCapCode) && (pCapBufferA->capTypeCode != eRoleLabelCapCode) )
			{
				pCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
				pCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sizeof(encryptionCapStruct);//sizeof(h263CapStructBase);
				pCapBuffer->capTypeCode					= (BYTE)eEncryptionCapCode;
				pCapBuffer->sipPayloadType				= GetPayloadType(eEncryptionCapCode);
				pCapBuffer->capLength 					= sizeof(encryptionCapStruct);
				pCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
				pCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(encryptionCapStruct);

				encryptionCapStruct* pEncryStruct = (encryptionCapStruct*)pCapBuffer->dataCap;
				pEncryStruct->header.xmlHeader.dynamicType   = eEncryptionCapCode;
				pEncryStruct->header.xmlHeader.dynamicLength = sizeof(encryptionCapStruct);
				pEncryStruct->header.direction   = eGeneralDirection;
				pEncryStruct->header.type        = cmCapH235;
				pEncryStruct->header.roleLabel   = kRolePeople;
				pEncryStruct->header.capTypeCode = eEncryptionCapCode;

			//	pFeccStruct->maxBitRate = 64; // units of 100 bit/s
				pEncryStruct->type	= (APIU16)kAES_CBC;
				pEncryStruct->entry	= entry;

				numOfEncrypCap++;

				nAllBuffersLen += sizeof(capBufferBase) + pCapBuffer->capLength;
				// move global ptr to next capBuffer position
				tempPtr += sizeof(capBufferBase) + pCapBuffer->capLength;
				pCapBuffer = (capBuffer*)tempPtr;

				// move encry local ptr to next capBuffer position
				pTempPtrA   += sizeof(capBufferBase) + pCapBuffer->capLength;
				pCapBufferA = (capBuffer*)pTempPtrA;
				entry++;
			}
		}
	}

	if( numOfAudioCap )
		numOfAlts++;

	if( numOfVideoCap )
		numOfAlts++;

	if( numOfFeccCap )
		numOfAlts++;

	if( numOfH239Cap )
		numOfAlts++;

	//if( numOfDPTRCap )
	//	numOfAlts++;

	if( numOfLPRCap )
		numOfAlts++;

	numOfCaps = numOfAudioCap + numOfVideoCap + numOfContentVideoCap + numOfH239Cap + numOfFeccCap + numOfEncrypCap + numOfLPRCap;

	cap_fd_set  matrix;
	FillCapMatrix(matrix,numOfAudioCap,numOfVideoCap,numOfContentVideoCap, numOfH239Cap,numOfFeccCap,numOfEncrypCap,numOfLPRCap);

	/// Allocate structure with real length
	indLen = sizeof(mcIndCapabilities) + nAllBuffersLen;
	*ppIndication = new BYTE[indLen];
	memset(*ppIndication,0,indLen);
	mcIndCapabilities*  pInd = (mcIndCapabilities*)(*ppIndication);
	pInd->capabilitiesSize = sizeof(ctCapabilitiesBasicStruct) + nAllBuffersLen;

	ctCapabilitiesStruct*  pCapabilities = &(pInd->capabilities);
	pCapabilities->numberOfCaps  = numOfCaps;
	pCapabilities->numberOfAlts  = 6; //per FillCapMatrix
	pCapabilities->numberOfSim   = 1;
	pCapabilities->xmlDynamicProps.numberOfDynamicParts   = numOfCaps;
	pCapabilities->xmlDynamicProps.sizeOfAllDynamicParts  = /*sizeof(ctCapabilitiesBasicStruct) +*/ nAllBuffersLen;

	memcpy(&(pCapabilities->caps),pBytesBuffer,nAllBuffersLen);
	memcpy(&(pCapabilities->altMatrix),&matrix,sizeof(cap_fd_set));

	PDELETEA(pBytesBuffer);

	return indLen;
}

/////////////////////////////////////////////////////////////////////////////
/// Allocates and fills ctCapabilitiesStruct indication
WORD CCapSet::CreateCapStructSIP(BYTE** ppCapStruct, const BOOL isFirstCap, const cmCapDirection eAudioDirection, const cmCapDirection eVideoDirection, const cmCapDirection eGeneralDirection) const
{
	BYTE* pCap323Buffer = NULL;
	CreateCapStructIP(&pCap323Buffer, eAudioDirection, eVideoDirection, eGeneralDirection, eProtocolSip, isFirstCap, TRUE /*bIsAnnexQ*/); // returns 323 mcIndCapabilities struct

	mcIndCapabilities*  p323struct = (mcIndCapabilities*)pCap323Buffer;
	ctCapabilitiesStruct* pCapabilities = (ctCapabilitiesStruct*)(&p323struct->capabilities);

	sipMediaLinesEntrySt *pMediaLinesEntry = NULL;
	int	capPosAudio	  = 0;
	int	capPosVideo	  = 0;
	int	capPosData	  = 0;
	int capPosBfcp    = 0;
	int	capPosContent = 0;

	ALLOCBUFFER(bufAudio,SIP_MEDIA_LINE_BUFFER_SIZE);
	ALLOCBUFFER(bufVideo,SIP_MEDIA_LINE_BUFFER_SIZE);
	ALLOCBUFFER(bufData,SIP_MEDIA_LINE_BUFFER_SIZE);
	ALLOCBUFFER(bufBfcp,SIP_MEDIA_LINE_BUFFER_SIZE);
	ALLOCBUFFER(bufContent,SIP_MEDIA_LINE_BUFFER_SIZE);

	int index = 0;
	sipMediaLineSt *pMediaLineAudio = (sipMediaLineSt *) &bufAudio[0];
	sipMediaLineSt *pMediaLineVideo = (sipMediaLineSt *) &bufVideo[0];
	sipMediaLineSt *pMediaLineData  = (sipMediaLineSt *) &bufData[0];
	sipMediaLineSt *pMediaLineBfcp  = (sipMediaLineSt *) &bufBfcp[0];
	sipMediaLineSt *pMediaLineContent = (sipMediaLineSt *) &bufContent[0];

	capBuffer *pCapBuffer = NULL;
	char *pCapBufferPos = NULL;
	int capsPos = 0;
	int capLen = 0;
	int mediaLinePos = 0;
	cmCapDataType eMediaType;

	memset(bufAudio, 0, sizeof(bufAudio));
	memset(bufVideo, 0, sizeof(bufVideo));
	memset(bufData,  0, sizeof(bufData));
	memset(bufContent, 0, sizeof(bufContent));
	memset(bufBfcp, 0, sizeof(bufBfcp));

	pCapBufferPos = (char *) &pCapabilities->caps;

	PTRACE2INT(eLevelInfoNormal,"CCapSet::CreateCapStructSIP pCapabilities->numberOfCaps:", pCapabilities->numberOfCaps);
	// Add the different capabilities to the relevant media line.
	// The first capability of each media line, in case of SIP encrypted call, will be SDES cap.
	for (int i = 0; i < pCapabilities->numberOfCaps; i++)
	{
		pCapBuffer = (capBuffer *) &pCapBufferPos[capsPos];
		capLen = sizeof(capBufferBase) + pCapBuffer->capLength;
		capsPos += capLen;

		BaseCapStruct* pCapStr = (BaseCapStruct*) pCapBuffer->dataCap;
		int role = pCapStr->header.roleLabel;

		int capCode = pCapBuffer->capTypeCode;

		if (capCode >= eG711Alaw64kCapCode && capCode <= eRfc2833DtmfCapCode)
			eMediaType = cmCapAudio;
		else if ((capCode >= eH261CapCode && capCode <= eGenericVideoCapCode)
				|| (capCode == eVP8CapCode) //N.A. DEBUG VP8
				|| (capCode == eMsSvcCapCode)
				|| (capCode == eSvcCapCode) )
			eMediaType = cmCapVideo;
		else if (eEncryptionCapCode == capCode)
			continue;
		else if ( eBFCPCapCode == capCode )
			eMediaType = cmCapBfcp;
		else
			eMediaType = cmCapData;

		if (eMediaType == cmCapAudio)
		{
			if ((m_isEncryption) && (0 == pMediaLineAudio->numberOfCaps)){
				if (!AddSdesCapToMediaLine(eMediaType, pMediaLineAudio, capPosAudio))
					PTRACE2(eLevelInfoNormal,"CCapSet::CreateCapStructSIP AddSdesCapToMediaLine fail - ", "Sdes for Audio");
			}
			if (!AddCapInMediaLine(pCapBuffer, capLen,
					pMediaLineAudio, SIP_MEDIA_LINE_BUFFER_SIZE, capPosAudio))
				PTRACE2(eLevelInfoNormal,"CCapSet::CreateCapStructSIP AddCapInMediaLine fail - ", "Audio");
		}
		else if (eMediaType == cmCapVideo && kRolePeople == role)
		{
			if ((m_isEncryption) && (0 == pMediaLineVideo->numberOfCaps)){
				if (!AddSdesCapToMediaLine(eMediaType, pMediaLineVideo, capPosVideo))
					PTRACE2(eLevelInfoNormal,"CCapSet::CreateCapStructSIP AddSdesCapToMediaLine fail - ", "Sdes for Video");
			}
			if (!AddCapInMediaLine(pCapBuffer, capLen,
					pMediaLineVideo, SIP_MEDIA_LINE_BUFFER_SIZE, capPosVideo))
				PTRACE2(eLevelInfoNormal,"CCapSet::CreateCapStructSIP AddCapInMediaLine fail - ", "Video");
			PTRACE2(eLevelInfoNormal,"CCapSet::CreateCapStructSIP AddCapInMediaLine OK - ", "Video");
		}
		else if (eMediaType == cmCapData)
		{
			if ((m_isEncryption) && (0 == pMediaLineData->numberOfCaps)){
				if (!AddSdesCapToMediaLine(eMediaType, pMediaLineData, capPosData))
					PTRACE2(eLevelInfoNormal,"CCapSet::CreateCapStructSIP AddSdesCapToMediaLine fail - ", "Sdes for Data");
			}
			if (!AddCapInMediaLine(pCapBuffer, capLen,
					pMediaLineData, SIP_MEDIA_LINE_BUFFER_SIZE, capPosData))
				PTRACE2(eLevelInfoNormal,"CCapSet::CreateCapStructSIP AddCapInMediaLine fail - ", "Data");
		}
		else if (eMediaType == cmCapBfcp)
		{
			if ((m_isEncryption) && (0 == pMediaLineData->numberOfCaps)){
				if (!AddSdesCapToMediaLine(eMediaType, pMediaLineBfcp, capPosBfcp))
					PTRACE2(eLevelInfoNormal,"CCapSet::CreateCapStructSIP AddSdesCapToMediaLine fail - ", "Sdes for Bfcp");
			}
			if (!AddCapInMediaLine(pCapBuffer, capLen,
					pMediaLineBfcp, SIP_MEDIA_LINE_BUFFER_SIZE, capPosBfcp))
				PTRACE2(eLevelInfoNormal,"CCapSet::CreateCapStructSIP AddCapInMediaLine fail - ", "Bfcp");
		}
		else if (eMediaType == cmCapVideo && kRolePresentation == role)
		{
			if ((m_isEncryption) && (0 == pMediaLineData->numberOfCaps)){
				if (!AddSdesCapToMediaLine(eMediaType, pMediaLineContent, capPosContent))
					PTRACE2(eLevelInfoNormal,"CCapSet::CreateCapStructSIP AddSdesCapToMediaLine fail - ", "Sdes for Content");
			}
			if (!AddCapInMediaLine(pCapBuffer, capLen,
					pMediaLineContent, SIP_MEDIA_LINE_BUFFER_SIZE, capPosContent))
				PTRACE2(eLevelInfoNormal,"CCapSet::CreateCapStructSIP AddCapInMediaLine fail - ", "Content");
		}
	}

	int offsetWrite = 0;
	int len = sizeof(sipMediaLinesEntryBaseSt);
	if (capPosAudio)
		len += sizeof(sipMediaLineBaseSt) + capPosAudio;
	if (capPosVideo)
		len += sizeof(sipMediaLineBaseSt) + capPosVideo;
	if (capPosData)
		len += sizeof(sipMediaLineBaseSt) + capPosData;
	if (capPosBfcp)
		len += sizeof(sipMediaLineBaseSt) + capPosBfcp;
	if (capPosContent)
		len += sizeof(sipMediaLineBaseSt) + capPosContent;

	*ppCapStruct = new BYTE [len];
	memset(*ppCapStruct, 0, len);
	pMediaLinesEntry = (sipMediaLinesEntrySt *) (*ppCapStruct);
	char *buffer = (char *)(*ppCapStruct) + sizeof(sipMediaLinesEntryBaseSt);

	if (capPosAudio)
	{
		pMediaLineAudio->index = index;
		pMediaLineAudio->ssrcrange[0] = 1;
		pMediaLineAudio->ssrcrange[1] = 1;
		pMediaLineAudio->type = eMediaLineTypeAudio;
		pMediaLineAudio->subType = eMediaLineSubTypeRtpAvp;
		pMediaLineAudio->internalType = kMediaLineInternalTypeAudio;
		index++;
		memcpy(buffer + offsetWrite, bufAudio, sizeof(sipMediaLineBaseSt) + capPosAudio);
		offsetWrite += sizeof(sipMediaLineBaseSt) + capPosAudio;
		pMediaLinesEntry->numberOfMediaLines++;
	}
	if (capPosVideo)
	{
		pMediaLineVideo->index = index;
		pMediaLineVideo->ssrcrange[0] = 101;
		pMediaLineVideo->ssrcrange[1] = 200;
		pMediaLineVideo->type = eMediaLineTypeVideo;
		pMediaLineVideo->subType = eMediaLineSubTypeRtpAvp;
		pMediaLineVideo->internalType = kMediaLineInternalTypeVideo;
		pMediaLineVideo->content = eMediaLineContentMain;
		strncpy(pMediaLineVideo->label,"1",2);
		index++;
		memcpy(buffer + offsetWrite, bufVideo, sizeof(sipMediaLineBaseSt) + capPosVideo);
		offsetWrite += sizeof(sipMediaLineBaseSt) + capPosVideo;
		pMediaLinesEntry->numberOfMediaLines++;
	}
	if (capPosData)
	{
		pMediaLineData->index = index;
		pMediaLineData->type = eMediaLineTypeApplication;
		pMediaLineData->subType = eMediaLineSubTypeRtpAvp;
		pMediaLineData->internalType = kMediaLineInternalTypeFecc;
		index++;
		memcpy(buffer + offsetWrite, bufData, sizeof(sipMediaLineBaseSt) + capPosData);
		offsetWrite += sizeof(sipMediaLineBaseSt) + capPosData;
		pMediaLinesEntry->numberOfMediaLines++;
	}
	if (capPosBfcp)
	{
		pMediaLineBfcp->index = index;
		pMediaLineBfcp->type = eMediaLineTypeApplication;
		pMediaLineBfcp->internalType = kMediaLineInternalTypeBfcp;

		if(m_bfcpTransType == eTransportTypeUdp)
			pMediaLineBfcp->subType = eMediaLineSubTypeUdpBfcp;
		else if(m_bfcpTransType == eTransportTypeTcp)
			pMediaLineBfcp->subType = eMediaLineSubTypeTcpBfcp;
		else if(m_bfcpTransType == eTransportTypeTls)
			pMediaLineBfcp->subType = eTransportTypeTls;
		index++;
		memcpy(buffer + offsetWrite, bufBfcp, sizeof(sipMediaLineBaseSt) + capPosBfcp);
		offsetWrite += sizeof(sipMediaLineBaseSt) + capPosBfcp;
		pMediaLinesEntry->numberOfMediaLines++;
	}
	if (capPosContent)
	{
		pMediaLineContent->index = index;
		pMediaLineContent->type = eMediaLineTypeVideo;
		pMediaLineContent->subType = eMediaLineSubTypeRtpAvp;
		pMediaLineContent->internalType = kMediaLineInternalTypeContent;
		pMediaLineContent->content = eMediaLineContentSlides;
		strncpy(pMediaLineContent->label,"3",2);
		index++;
		memcpy(buffer + offsetWrite, bufContent, sizeof(sipMediaLineBaseSt) + capPosContent);
		offsetWrite += sizeof(sipMediaLineBaseSt) + capPosContent;
		pMediaLinesEntry->numberOfMediaLines++;
	}

	pMediaLinesEntry->lenOfDynamicSection = offsetWrite;

	DEALLOCBUFFER(bufAudio);
	DEALLOCBUFFER(bufVideo);
	DEALLOCBUFFER(bufData);
	DEALLOCBUFFER(bufBfcp);
	DEALLOCBUFFER(bufContent);

	PDELETEA(pCap323Buffer);

	return len;
}


/////////////////////////////////////////////////////////////////////////////
BOOL CCapSet::AddSdesCapToMediaLine(cmCapDataType eMediaType, sipMediaLineSt *pMediaLine, int &capPos) const
{
	BOOL retValue = FALSE;
	BYTE*  pBytesBuffer = new BYTE [4096];
	memset(pBytesBuffer,0,4096);
	capBuffer* pSdesCapBuffer = (capBuffer*)pBytesBuffer;
	int sdesCapLen = 0;

	int sdesCapStructDynamicLen = sizeof(sdesCapStruct) + sizeof(xmlSdesKeyParamsStruct);

	// Create temporary cap buffer for sdes
	for( int i=0; i<m_nSdesCapNum; i++ )
	{
		if (eMediaType == m_paSdesCaps[i]->GetSdesMediaType())
		{
			pSdesCapBuffer->xmlHeader.dynamicType		= tblCapBuffer;
			pSdesCapBuffer->xmlHeader.dynamicLength		= sizeof(capBufferBase) + sdesCapStructDynamicLen;
			pSdesCapBuffer->capTypeCode					= (BYTE)eSdesCapCode;
			pSdesCapBuffer->sipPayloadType				= GetPayloadType(eSdesCapCode);
			pSdesCapBuffer->capLength 					= sdesCapStructDynamicLen;
			pSdesCapBuffer->xmlDynamicProps.numberOfDynamicParts  = 1;
			pSdesCapBuffer->xmlDynamicProps.sizeOfAllDynamicParts = sdesCapStructDynamicLen;

			sdesCapStruct* pSdesCapStruct = (sdesCapStruct*)pSdesCapBuffer->dataCap;

			(m_paSdesCaps[i])->FillStructSdes(pSdesCapStruct, cmCapReceiveAndTransmit, kRolePeople);

			sdesCapLen = sizeof(capBufferBase) + pSdesCapBuffer->capLength;


			if (!AddCapInMediaLine(pSdesCapBuffer, sdesCapLen,
					pMediaLine, SIP_MEDIA_LINE_BUFFER_SIZE, capPos))
			{
				PTRACE2(eLevelInfoNormal,"CCapSet::AddSdesCapToMediaLine AddCapInMediaLine fail - ", "Sdes for Audio");
				retValue = FALSE;
				break;
			}
			retValue = TRUE;
			break;
		}
	}
	if (FALSE == retValue)
		TRACEINTO << "CCapSet::AddSdesCapToMediaLine Sdes cap for this media type was not found - " << eMediaType;

	PDELETEA(pBytesBuffer);
	return retValue;
}

/////////////////////////////////////////////////////////////////////////////
CCapSet&  CCapSet::operator=(const CCapSet& other)
{
	if( this == &other )
		return *this;

	int i=0;

	strncpy( m_szName, other.m_szName, MAX_CAP_NAME-1 );
	m_szName[MAX_CAP_NAME-1] = '\0';

	m_nID = other.m_nID;

	m_nCallRate	= other.m_nCallRate;

	m_nAudAlgNum	= other.m_nAudAlgNum;
	for( i=0; i<MAX_AUDIO_ALG_NUM; i++ )
		m_audioPayloadTypes[i] = other.m_audioPayloadTypes[i];

	m_nVidProtocolNum	= other.m_nVidProtocolNum;
	for( i=0; i<MAX_VIDEO_PROTOCOL_NUM; i++ )
	{
		POBJDELETE(m_paVideoCaps[i]);
		if( i<m_nVidProtocolNum && other.m_paVideoCaps[i] != NULL )
			m_paVideoCaps[i] = other.m_paVideoCaps[i]->Clone();
	}

	m_nSdesCapNum = other.m_nSdesCapNum;
	for( i=0; i<MAX_SDES_CAP_NUM; i++ )
	{
		POBJDELETE(m_paSdesCaps[i]);
		if( i<m_nSdesCapNum && other.m_paSdesCaps[i] != NULL )
			m_paSdesCaps[i] = other.m_paSdesCaps[i]->Clone();
	}
	// copy presentation video mode
	POBJDELETE(m_pVideoCapH264Presentation);
	if( NULL != other.m_pVideoCapH264Presentation )
		m_pVideoCapH264Presentation = new CVideoCapH264(*(other.m_pVideoCapH264Presentation));

	POBJDELETE(m_pVideoCapH263Presentation);
	if( NULL != other.m_pVideoCapH263Presentation )
		m_pVideoCapH263Presentation = new CVideoCapH263(*(other.m_pVideoCapH263Presentation));

	m_isFecc       = other.m_isFecc;
	m_isEncryption = other.m_isEncryption;
	m_nH239Rate    = other.m_nH239Rate;
	m_isLPR        = other.m_isLPR;
	m_bfcpTransType= other.m_bfcpTransType;
	//m_isDPTR 	   = other.m_isDPTR;

	m_isChanged    = TRUE;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::Empty()
{
	m_nCallRate 		= 0;

//	int i=0;

	EmptyAudio();
//	m_nAudAlgNum		= 0;
//	for( i=0; i<MAX_AUDIO_ALG_NUM; i++ )
//		m_audioPayloadTypes[i] = (DWORD)eUnknownAlgorithemCapCode;

	EmptyVideo();
//	m_nVidProtocolNum	= 0;
//	for( i = 0; i<MAX_VIDEO_PROTOCOL_NUM; i++ )
//		POBJDELETE(m_paVideoCaps[i]);
//	POBJDELETE(m_pVideoCapH263Presentation);

	EmptySdes();

	m_isFecc       = FALSE;
	m_isEncryption = FALSE;
	m_nH239Rate		= 0;
	m_isLPR = FALSE;
	m_bfcpTransType = eTransportTypeUdp;
	//m_isDPTR = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::EmptyAudio()
{
	m_nAudAlgNum = 0;
	for( int i=0; i<MAX_AUDIO_ALG_NUM; i++ )
		m_audioPayloadTypes[i] = (DWORD)eUnknownAlgorithemCapCode;
}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::EmptyVideo()
{
	m_nVidProtocolNum = 0;
	for( int i = 0; i<MAX_VIDEO_PROTOCOL_NUM; i++ )
		POBJDELETE(m_paVideoCaps[i]);

	EmptyH239();
}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::EmptyH239()
{
	SetH239Rate(0);
	SetBfcpTransportType(eUnknownTransportType);

	POBJDELETE(m_pVideoCapH264Presentation);
	POBJDELETE(m_pVideoCapH263Presentation);
}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::EmptySdes()
{
	m_nSdesCapNum = 0;
	for( int i = 0; i<MAX_SDES_CAP_NUM; i++ )
		POBJDELETE(m_paSdesCaps[i]);
}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::SetName( const char* pszCapName )
{
	if ( NULL != pszCapName ) {
		strncpy(m_szName, pszCapName, MAX_CAP_NAME-1);
		m_szName[MAX_CAP_NAME-1] = '\0';
	}
}


/////////////////////////////////////////////////////////////////////////////
void CCapSet::SetID( const DWORD capID )
{
	m_nID = capID;
}


/////////////////////////////////////////////////////////////////////////////
void CCapSet::DefaultAudioSet()
{
	m_nAudAlgNum = 0;
	for( int i=0; i<MAX_AUDIO_ALG_NUM; i++ )
		m_audioPayloadTypes[i] = (DWORD)eUnknownAlgorithemCapCode;

	AddAudioAlg(eG711Alaw64kCapCode);
//	AddAudioAlg(eG711Alaw56kCapCode);
	AddAudioAlg(eG711Ulaw64kCapCode);
//	AddAudioAlg(eG711Ulaw56kCapCode);
}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::DefaultVideoSet()
{
	for( int i=0; i<m_nVidProtocolNum; i++ )
		POBJDELETE(m_paVideoCaps[i]);
	m_nVidProtocolNum = 0;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CCapSet::SetRate(const WORD rate)
{
	switch( rate )
	{
		case 64:
		case 128:
		case 192:
		case 256:
		case 384:
		case 512:
		case 1024:
		case 1920:
		case 4096:
		case 6144:
		{
			m_nCallRate = rate;
			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::SetFecc(const BOOL fecc)
{
	m_isFecc = fecc;
}



/////////////////////////////////////////////////////////////////////////////
void CCapSet::SetEncryption(const BOOL encry)
{
	m_isEncryption = encry;

	for( int i=0; i<m_nSdesCapNum; i++ )	// clean existing sdes cap objects
		POBJDELETE(m_paSdesCaps[i]);
	m_nSdesCapNum = 0;

	if( TRUE == encry )
		AddSdesCaps();
}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::SetH239Rate(const WORD nRate)
{
	if(nRate > 0 && nRate < 6144)
		m_nH239Rate = nRate;
	else
		m_nH239Rate = 192;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CCapSet::AddAudioAlg(const DWORD alg)
{
	if (m_nAudAlgNum >= MAX_AUDIO_ALG_NUM)
		return FALSE;
	switch (alg)
	{
		case eG711Alaw64kCapCode:
		case eG711Alaw56kCapCode:
		case eG711Ulaw64kCapCode:
		case eG711Ulaw56kCapCode:
		case eG722_64kCapCode:
		case eG722_56kCapCode:
		case eG722_48kCapCode:
		case eG722Stereo_128kCapCode:
		case eG728CapCode:
		case eG729CapCode:
		case eG729AnnexACapCode:
		case eG729wAnnexBCapCode:
		case eG729AnnexAwAnnexBCapCode:
		case eG7231CapCode:
		case eIS11172AudioCapCode:
		case eIS13818CapCode:
		case eG7231AnnexCapCode:
		case eG7221_32kCapCode:
		case eG7221_24kCapCode:
		case eG7221_16kCapCode:
		case eSiren14_48kCapCode:
		case eSiren14_32kCapCode:
		case eSiren14_24kCapCode:
		//case eOpus_CapCode:
		case eG7221C_CapCode:
		case eG7221C_48kCapCode:
		case eG7221C_32kCapCode:
		case eG7221C_24kCapCode:
		case eRfc2833DtmfCapCode:
		case eSiren14Stereo_48kCapCode:
		case eSiren14Stereo_64kCapCode:
		case eSiren14Stereo_96kCapCode:
		//case eOpusStereo_CapCode:
		case eG719Stereo_128kCapCode:
		case eG719Stereo_96kCapCode:
		case eG719Stereo_64kCapCode:
		case eG719_64kCapCode:
		case eG719_48kCapCode:
		case eG719_32kCapCode:
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
		// For SAC
		case eSirenLPR_Scalable_32kCapCode:
		case eSirenLPR_Scalable_48kCapCode:
		case eSirenLPR_Scalable_64kCapCode:
		case eSirenLPRStereo_Scalable_64kCapCode:
		case eSirenLPRStereo_Scalable_96kCapCode:
		case eSirenLPRStereo_Scalable_128kCapCode:
		{
			// if already exists - return success
			for (int i = 0; i < m_nAudAlgNum; i++)
				if (m_audioPayloadTypes[i] == alg)
					return TRUE;
			m_audioPayloadTypes[m_nAudAlgNum] = alg;
			m_nAudAlgNum++;
			return TRUE;
		}
		case eAAC_LDCapCode:     // TIP
		{
			//TIP support for testing embedded MLA feature
			if (CCapSetsList::m_isTIP == TRUE)
			{
				// if already exists - return success
				for (int i = 0; i < m_nAudAlgNum; i++)
					if (m_audioPayloadTypes[i] == alg)
						return TRUE;
				m_audioPayloadTypes[m_nAudAlgNum] = alg;
				m_nAudAlgNum++;
				return TRUE;
			}
		}
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
DWORD CCapSet::GetAudioAlgType( const WORD ind ) const
{
	if ( (ind < m_nAudAlgNum) && (ind < MAX_AUDIO_ALG_NUM) )
		return m_audioPayloadTypes[ind];
	return eUnknownAlgorithemCapCode;
}


/////////////////////////////////////////////////////////////////////////////
BOOL CCapSet::AddVideoProtocol( const CVideoCap& rVideoCap )
{
	int i;
	for( i=0; i<m_nVidProtocolNum; i++ )
	{
		if( m_paVideoCaps[i]->GetPayloadType() == rVideoCap.GetPayloadType() )
		{
			if (rVideoCap.GetPayloadType()==eH264CapCode || rVideoCap.GetPayloadType() == eSvcCapCode)
			{
				DWORD profile1 =((CVideoCapH264*)(m_paVideoCaps[i]))->GetProfile();
				DWORD profile2 =((CVideoCapH264&)(rVideoCap)).GetProfile();

				if (profile1 != profile2)
				{
					TRACEINTO << "CCapSet::AddVideoProtocol:profile1 != profile2";
					break;
				}
			}
			/// overwrite video set
			POBJDELETE(m_paVideoCaps[i]);
			m_paVideoCaps[i] = rVideoCap.Clone();
			return TRUE;
		}
	}
	if( m_nVidProtocolNum < MAX_VIDEO_PROTOCOL_NUM )
	{
		POBJDELETE(m_paVideoCaps[m_nVidProtocolNum]);
		m_paVideoCaps[m_nVidProtocolNum] = rVideoCap.Clone();
		m_nVidProtocolNum++;
		return TRUE;
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
DWORD CCapSet::GetVideoProtocolType( const WORD ind ) const
{
	if ( (ind < m_nVidProtocolNum) && (ind < MAX_VIDEO_PROTOCOL_NUM) )
		return m_paVideoCaps[ind]->GetPayloadType();
	return eUnknownAlgorithemCapCode;
}


/////////////////////////////////////////////////////////////////////////////
DWORD CCapSet::GetPresentVideoProtocolType() const
{
	if(GetPresentationVideo())
		return GetPresentationVideo()->GetPayloadType();
	return eUnknownAlgorithemCapCode;
}


/////////////////////////////////////////////////////////////////////////////
const CVideoCap* CCapSet::GetVideoProtocol(const WORD ind) const
{
	if( ind<m_nVidProtocolNum )
		return m_paVideoCaps[ind];
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CCapSet::AddPresentationVideo( const CVideoCapH263& rVideoCap )
{
	POBJDELETE(m_pVideoCapH263Presentation);
	m_pVideoCapH263Presentation = new CVideoCapH263(rVideoCap);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CCapSet::AddPresentationVideoH264( const CVideoCapH264& rVideoCap )
{
	POBJDELETE(m_pVideoCapH264Presentation);
	m_pVideoCapH264Presentation = new CVideoCapH264(rVideoCap);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
const CVideoCap* CCapSet::GetPresentationVideo() const
{
	if(m_pVideoCapH264Presentation)
	{
		return m_pVideoCapH264Presentation;
	}
	return m_pVideoCapH263Presentation;
}


/////////////////////////////////////////////////////////////////////////////
void CCapSet::AddSdesCaps()
{
	CSdesEpCap  rSdesAudioCap(cmCapAudio);
	CSdesEpCap  rSdesVideoCap(cmCapVideo);
	CSdesEpCap  rSdesDataCap(cmCapData);

	AddSdesCap(rSdesAudioCap);
	AddSdesCap(rSdesVideoCap);
	AddSdesCap(rSdesDataCap);

}

/////////////////////////////////////////////////////////////////////////////
BOOL CCapSet::AddSdesCap( const CSdesEpCap& rSdesCap )
{
	int i;
	for( i=0; i<m_nSdesCapNum; i++ )
	{
		if( m_paSdesCaps[i]->GetSdesMediaType() == rSdesCap.GetSdesMediaType() )
		{
			/// overwrite sdes cap
			POBJDELETE(m_paSdesCaps[i]);
			m_paSdesCaps[i] = rSdesCap.Clone();
			return TRUE;
		}
	}
	if( m_nSdesCapNum < MAX_SDES_CAP_NUM )
	{
		POBJDELETE(m_paSdesCaps[m_nSdesCapNum]);
		m_paSdesCaps[m_nSdesCapNum] = rSdesCap.Clone();
		m_nSdesCapNum++;
		return TRUE;
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
void CCapSet::Serialize(CSegment& rSegCap) const
{
	rSegCap << m_nID;
	rSegCap << m_szName;
	rSegCap << m_nCallRate;

	rSegCap << (DWORD)m_isFecc
			<< (DWORD)m_isEncryption
			<< (DWORD)m_nH239Rate;

	int i;

	rSegCap << m_nAudAlgNum;
	//cout << "m_nAudAlgNum = " << m_nAudAlgNum << endl;
	for( i=0; i<m_nAudAlgNum; i++ )
		rSegCap << m_audioPayloadTypes[i];

	rSegCap << m_nVidProtocolNum;
	//cout << "m_nVidProtocolNum = " << m_nVidProtocolNum << endl;
	for( i=0; i<m_nVidProtocolNum; i++ )
		m_paVideoCaps[i]->Serialize(rSegCap);

	// the sdes caps are not displayed/modified in the GUI - no need to send them
//	rSegCap << m_nSdesCapNum;
//	for( i=0; i<m_nSdesCapNum; i++ )
//		m_paSdesCaps[i]->Serialize(rSegCap);
}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::DeSerialize(CSegment& rSegParam)
{
	DWORD  dummy = 0;

	rSegParam >> m_nID;
	rSegParam >> m_szName;
	rSegParam >> m_nCallRate;

	rSegParam >> dummy;
	m_isFecc = (dummy)? TRUE : FALSE;

	rSegParam >> dummy;
	SetEncryption( (dummy)? TRUE : FALSE );

	rSegParam >> dummy; // h239 rate
	m_nH239Rate = (WORD)dummy;

	if( m_nH239Rate > 0 )
	{
		POBJDELETE(m_pVideoCapH264Presentation);
		POBJDELETE(m_pVideoCapH263Presentation);
		if(m_nH239Rate == 170)
			m_pVideoCapH264Presentation = new CVideoCapH264(eVideoModeHD1080);
		if (m_nH239Rate == 180)
			{
				m_pVideoCapH263Presentation = new CVideoCapH263();
				m_pVideoCapH263Presentation->CreatePresentationCap();
			}
		if(m_nH239Rate != 170 &&  m_nH239Rate != 180)
		{
			m_pVideoCapH264Presentation = new CVideoCapH264(eVideoModeHD1080);
			m_pVideoCapH263Presentation = new CVideoCapH263();
			m_pVideoCapH263Presentation->CreatePresentationCap();
		}
	}
	WORD tempNum = 0;
	rSegParam >> tempNum;

	int i=0;
	m_nAudAlgNum = 0;// m_nAudAlgNum is set through the AddAudioAlg function
	for( i=0; i<tempNum; i++ )
	{

		if (i==21)
		{
			TRACEINTO << "CCapSet::DeSerialize-" << tempNum << "i : " << i;
		}
		rSegParam >> dummy;
		AddAudioAlg(dummy);
	}

	// clean existing video cap objects
	for( i=0; i<m_nVidProtocolNum; i++ )
		POBJDELETE(m_paVideoCaps[i]);

	rSegParam >> tempNum;
	m_nVidProtocolNum = 0;//(tempNum < MAX_VIDEO_PROTOCOL_NUM) ? tempNum : MAX_VIDEO_PROTOCOL_NUM;
	for( i=0; i<tempNum; i++ )
	{
		DWORD  payloadType = 0xFFFFFFFF;
		rSegParam >> payloadType;

		CVideoCap* pVideo = NULL;
		if( payloadType == eH261CapCode )
			pVideo = new CVideoCapH261();
		else if( payloadType == eH263CapCode )
			pVideo = new CVideoCapH263();
		else if( payloadType == eH264CapCode )
			pVideo = new CVideoCapH264();
		else if( payloadType ==  eRtvCapCode)
			pVideo = new CVideoCapRTV();
		else if(payloadType == eMsSvcCapCode)
			pVideo = new CVideoCapMSSvc();
		else if (payloadType == eSvcCapCode)
			pVideo = new  CVideoCapSVC();
		else
			break;

		if( pVideo != NULL )
		{
			pVideo->DeSerialize(rSegParam);
			AddVideoProtocol(*pVideo);
		}
		POBJDELETE(pVideo);
	}


	// the sdes caps are hard coded - no need to get them from GUI
	//   add sdes cap in SetEncryption(true)
	/*if (m_isEncryption)
	{
		for( i=0; i<m_nSdesCapNum; i++ )	// clean existing sdes cap objects
			POBJDELETE(m_paSdesCaps[i]);

		AddSdesCaps();
	}*/

//	for( i=0; i<m_nSdesCapNum; i++ )	// clean existing sdes cap objects
//		POBJDELETE(m_paSdesCaps[i]);
//
//	rSegParam >> tempNum;
//	m_nSdesCapNum = 0;
//	for( i=0; i<tempNum; i++ )
//	{
//		CSdesEpCap* pSdesCap = NULL;
//		pSdesCap = new CSdesEpCap();
//
//		if( pSdesCap != NULL )
//		{
//			pSdesCap->DeSerialize(rSegParam);
//			AddSdesCap(*pSdesCap);
//		}
//		POBJDELETE(pSdesCap);
//	}

}

/////////////////////////////////////////////////////////////////////////////
void CCapSet::FillCapMatrix( cap_fd_set& matrix, const BYTE numOfAudioCap,
		const BYTE numOfVideoCap, const BYTE numOfContentVideoCap, const BYTE numOfH239Cap,
		const BYTE numOfFeccCap, const BYTE numOfEncrypCap,
		const BYTE numOfLPRCap) const
{
	int i, k;
//	cap_fd_set  matrix;
	for (k = 0; k < FD_SET_SIZE; k++)
		matrix.fds_bits[k] = 0;

	int currentAltNumber	= 1;
	int currentBitToBeSet	= 0;
	int currentEncrToBeSet  = 0;

	int numOfCaps = numOfAudioCap + numOfVideoCap + numOfContentVideoCap + numOfH239Cap + numOfFeccCap + numOfEncrypCap + numOfLPRCap;

	if( numOfEncrypCap )
		currentEncrToBeSet = numOfCaps - numOfEncrypCap;

	if (currentEncrToBeSet < 0)
	{
		PASSERT(currentEncrToBeSet);
		currentEncrToBeSet = 0;
	}
	
		// set audio mask
	for( i=currentBitToBeSet; i<numOfAudioCap; i++ ){
		PASSERT_AND_RETURN((i+currentEncrToBeSet)/0x20 >= FD_SET_SIZE);
		CAP_FD_SET(i+currentEncrToBeSet,&(matrix));
	}

	currentBitToBeSet = currentAltNumber * numOfCaps + numOfAudioCap;
	currentAltNumber++;
		// set video mask
	for( i=currentBitToBeSet; i < (currentBitToBeSet + numOfVideoCap); i++ )
	{
		//	if ((pCapBuffer->capTypeCode != ePeopleContentCapCode) && (pCapBuffer->capTypeCode != eRoleLabelCapCode))
		//		CAP_FD_SET(i+currentEncrToBeSet,&(m_pCap->altMatrix));
		//	else
		PASSERT_AND_RETURN((i+currentEncrToBeSet)/0x20 >= FD_SET_SIZE);
		CAP_FD_SET(i+currentEncrToBeSet,&(matrix));
	}
	currentBitToBeSet = currentAltNumber * numOfCaps + numOfAudioCap + numOfVideoCap;
	currentAltNumber++;
    // set H239 Control caps
	for( i=currentBitToBeSet; i < (currentBitToBeSet + numOfH239Cap); i++ )
	{
		PASSERT_AND_RETURN((i)/0x20 >= FD_SET_SIZE);
		CAP_FD_SET(i,&(matrix));
		// control caps should not be encrypted
        if (currentEncrToBeSet)
			currentEncrToBeSet--;
	}
    currentBitToBeSet = currentAltNumber * numOfCaps + numOfAudioCap + numOfVideoCap + numOfH239Cap;
	currentAltNumber++;
	// set Content video (H239) mask
	for( i=currentBitToBeSet; i < (currentBitToBeSet + numOfContentVideoCap); i++ )
	{
		//	if ((pCapBuffer->capTypeCode != ePeopleContentCapCode) && (pCapBuffer->capTypeCode != eRoleLabelCapCode))
		//		CAP_FD_SET(i+currentEncrToBeSet,&(m_pCap->altMatrix));
		//	else
		PASSERT_AND_RETURN((i+currentEncrToBeSet)/0x20 >= FD_SET_SIZE);
		CAP_FD_SET(i+currentEncrToBeSet,&(matrix));
	}
	currentBitToBeSet = currentAltNumber * numOfCaps + numOfAudioCap + numOfVideoCap + numOfH239Cap + numOfContentVideoCap;
	currentAltNumber++;
		// set Fecc mask
	for( i = currentBitToBeSet; i < (currentBitToBeSet + numOfFeccCap); i++ ){
		PASSERT_AND_RETURN((i+currentEncrToBeSet)/0x20 >= FD_SET_SIZE);
		CAP_FD_SET(i+currentEncrToBeSet,&(matrix));
	}

	currentBitToBeSet = currentAltNumber * numOfCaps + numOfAudioCap + numOfVideoCap + numOfH239Cap + numOfContentVideoCap + numOfFeccCap;
	currentAltNumber++;

	// set LPR Control caps
	for( i = currentBitToBeSet; i < (currentBitToBeSet + numOfLPRCap); i++ )
	{
		// control caps should not be encrypted
		PASSERT_AND_RETURN((i)/0x20 >= FD_SET_SIZE);
		CAP_FD_SET(i,&(matrix));
		if (currentEncrToBeSet)
			currentEncrToBeSet--;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCapSet::GetPayloadType( const DWORD algo )
{
	switch( algo )
	{
		case eG711Ulaw64kCapCode: return _PCMU;
		case eG711Ulaw56kCapCode: return _PCMU;
		case eG711Alaw64kCapCode: return _PCMA;
		case eG711Alaw56kCapCode: return _PCMA;
		case eG722_64kCapCode: return _G722;
		case eG722_56kCapCode: return _G722;
		case eG722_48kCapCode: return _G722;
		case eG722Stereo_128kCapCode: return _G722;
		case eG728CapCode:     return _G728;
		case eG729CapCode:     return _G729;
		case eG729AnnexACapCode:        return _G729;
		case eG729wAnnexBCapCode:       return _G729;
		case eG729AnnexAwAnnexBCapCode: return _G729;
		case eG7231CapCode:      return _G7231;
		case eG7231AnnexCapCode: return _G7231;
		case eH261CapCode:  return _H261;
		case eH263CapCode:  return _H263;
		case eVP8CapCode:  return _VP8; //N.A. DEBUG VP8
		case eH26LCapCode:  return _H26L;
		case eT120DataCapCode:  return _T120;
		case eRvFeccCapCode:    return _RvFecc;
		case eNonStandardCapCode:   return _NONS;
		case ePeopleContentCapCode: return _H323_P_C;
		case eH239ControlCapCode:   return _H239;
		case eDynamicPTRCapCode:	return _DynamicPTRep;
		case eLPRCapCode:   return _H239;
		case eSdesCapCode:	return _Sdes;
		case eRtvCapCode: return _RtvDynamic;
		case eMsSvcCapCode: return _MS_SVC;
		case eSvcCapCode: return _PLCM_SVC;

//		case eH264CapCode:  return _H264;
//		case eGenericCapCode:	return _ANY;
//		case eG7221_16kCapCode:	return _G7221;
//		case eG7221_16kCapCode:	return _Siren14;
//		case eG7221_16kCapCode:	return _Rfc2833Dtmf;

		// dynamic
		case eG7221_16kCapCode:	return _G7221_16;
		case eG7221_24kCapCode:	return _G7221_24;
		case eG7221_32kCapCode:	return _G7221_32;
		case eSiren14_24kCapCode: return _Siren14_24;
		case eSiren14_32kCapCode: return _Siren14_32;
		case eSiren14_48kCapCode: return _Siren14_48;
		//case eOpus_CapCode: return 111;
		case eG7221C_48kCapCode: return _G7221C_48;
		case eG7221C_32kCapCode: return _G7221C_32;
		case eG7221C_24kCapCode: return _G7221C_24;
		case eH264CapCode:        return _H264Dynamic;
		case eRfc2833DtmfCapCode: return _Rfc2833DtmfDynamic;
		case eAnnexQCapCode:	  return _AnnexQDynamic;
		case eSiren14Stereo_48kCapCode: return _Siren14S_48;
		case eSiren14Stereo_64kCapCode: return _Siren14S_64;
		case eSiren14Stereo_96kCapCode: return _Siren14S_96;
		//case eOpusStereo_CapCode: return 111;
		case eG719Stereo_128kCapCode: return _G719S_128;
		case eG719Stereo_96kCapCode: return _G719S_96;
		case eG719Stereo_64kCapCode: return _G719S_64;
		case eG719_64kCapCode: return _G719_64;
		case eG719_48kCapCode: return _G719_48;
		case eG719_32kCapCode: return _G719_32;
		case eSiren22Stereo_128kCapCode:
		case eSirenLPRStereo_Scalable_128kCapCode:
			return _Siren22S_128;
		case eSiren22Stereo_96kCapCode:
		case eSirenLPRStereo_Scalable_96kCapCode:
			return _Siren22S_96;
		case eSiren22Stereo_64kCapCode:
		case eSirenLPRStereo_Scalable_64kCapCode:
			return _Siren22S_64;
		case eSiren22_64kCapCode:
		case eSirenLPR_Scalable_64kCapCode:
			return _Siren22_64;
		case eSiren22_48kCapCode:
		case eSirenLPR_Scalable_48kCapCode:
			return _Siren22_48;
		case eSiren22_32kCapCode:
		case eSirenLPR_Scalable_32kCapCode:
			return _Siren22_32;

		case eSirenLPR_32kCapCode:
		case eSirenLPR_48kCapCode:
		case eSirenLPRStereo_64kCapCode:
		case eSirenLPRStereo_96kCapCode:
		case eSirenLPR_64kCapCode:
		case eSirenLPRStereo_128kCapCode: return _SirenLPRS_128;

		//TIP support for testing embedded MLA feature
		case eAAC_LDCapCode:
		{
			if (CCapSetsList::m_isTIP == TRUE)
			{
				return _AACLD;// TIP
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CCapSet::IsBfcp() const
{
	if (IsBfcpSupported())
	{
		if(GetPresentVideoProtocolType()!=eUnknownAlgorithemCapCode
			&& GetVideoProtocolType(0)!=eUnknownAlgorithemCapCode)
			return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//
//   CVideoCap - Base Video Capability element
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
WORD CVideoCap::MpiToFps(const WORD mpi)
{
	switch( mpi )
	{
		case 1:
			return 30;
		case 2:
			return 15;
		case 3:
			return 10;
		case 4:
			return 7;
		case 5:
			return 6;
		case 6:
			return 5;
		case 10:
			return 3;
		case 15:
			return 2;
		case 30:
			return 1;
	}
	return (WORD)-1;
}

/////////////////////////////////////////////////////////////////////////////
WORD CVideoCap::FpsToMpi(const WORD fps)
{
	switch( fps )
	{
		case 30:
			return 1;
		case 15:
			return 2;
		case 10:
			return 3;
		case 7:
			return 4;
		case 6:
			return 5;
		case 5:
			return 6;
		case 3:
			return 10;
		case 2:
			return 15;
		case 1:
			return 30;
	}
	return (WORD)-1;
}


/////////////////////////////////////////////////////////////////////////////
//
//   CVideoCapH261 - H261 Video Capability element
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CVideoCapH261::CVideoCapH261()
{
	for( int i=0; i<eUnknownVideoFormat; i++ )
		m_resolutions[i] = (WORD)-1;

	m_resolutions[eQcif] = 30;
	m_resolutions[eCif]  = 30;
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH261::CVideoCapH261(const CVideoCapH261& other)
		: CVideoCap(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
// creates highest common video cap
CVideoCapH261::CVideoCapH261(const CVideoCap& first,const CVideoCap& second)
		: CVideoCap(first,second)
{
	for( int i=0; i<eUnknownVideoFormat; i++ )
		m_resolutions[i] = (WORD)-1;

	if( first.GetPayloadType() != GetPayloadType() )
		return;
	if( second.GetPayloadType() != GetPayloadType() )
		return;

	const CVideoCapH261*  pFirst  = (CVideoCapH261*) &first;
	const CVideoCapH261*  pSecond = (CVideoCapH261*) &second;
	for( int i=0; i<eUnknownVideoFormat; i++ )
	{
		if( (WORD)-1 == pFirst->m_resolutions[i]  ||  (WORD)-1 == pSecond->m_resolutions[i] )
			continue;
		m_resolutions[i] = (pFirst->m_resolutions[i] < pSecond->m_resolutions[i]) ?
				pFirst->m_resolutions[i] : pSecond->m_resolutions[i];
	}
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH261::CVideoCapH261(const h261CapStruct& tStruct)
{
	m_resolutions[eQcif]  = MpiToFps(tStruct.qcifMPI);
	m_resolutions[eCif]   = MpiToFps(tStruct.cifMPI);
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH261::~CVideoCapH261()
{
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH261& CVideoCapH261::operator= (const CVideoCapH261& other)
{
	if( this == &other )
		return *this;

	for( int i=0; i<eUnknownVideoFormat; i++ )
		m_resolutions[i] = other.m_resolutions[i];

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapH261::Serialize(CSegment& rSegCap) const
{
	rSegCap << (DWORD)GetPayloadType();
	//cout << "Payload type: " << GetPayloadType() << endl;
	for( int i=0; i<eUnknownVideoFormat; i++ )
		rSegCap << m_resolutions[i];
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapH261::DeSerialize(CSegment& rSegCap)
{
	for( int i=0; i<eUnknownVideoFormat; i++ )
		rSegCap >> m_resolutions[i];
}

/////////////////////////////////////////////////////////////////////////////
CVideoCap* CVideoCapH261::Clone() const
{
	return new CVideoCapH261(*this);
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapH261::FillStructH323(h261CapStruct* pStruct,const DWORD maxVideoRate,
		const BYTE direction,const BYTE role) const
{
	pStruct->header.xmlHeader.dynamicType   = eH261CapCode;
	pStruct->header.xmlHeader.dynamicLength = sizeof(h261CapStruct);
	pStruct->header.direction   = direction; //cmCapReceiveAndTransmit; // cmCapReceive;
	pStruct->header.type        = cmCapVideo;
	pStruct->header.roleLabel   = role;      // kRolePeople / kRolePresentation;
	pStruct->header.capTypeCode = eH261CapCode;

	pStruct->maxBitRate   = maxVideoRate;
	pStruct->qcifMPI  = FpsToMpi(m_resolutions[eQcif]);
	pStruct->cifMPI   = FpsToMpi(m_resolutions[eCif]);
}


/////////////////////////////////////////////////////////////////////////////
//
//   CVideoCapH263 - H263 Video Capability element
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CVideoCapH263::CVideoCapH263()
{
	for( int i=0; i<eUnknownVideoFormat; i++ )
		m_resolutions[i] = (WORD)-1;

	m_resolutions[eQcif] = 30;
	m_resolutions[eCif]  = 30;
	m_resolutions[e4Cif] = 15;

}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH263::CVideoCapH263(const CVideoCapH263& other)
		: CVideoCap(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
// creates highest common video cap
CVideoCapH263::CVideoCapH263(const CVideoCap& first,const CVideoCap& second)
		: CVideoCap(first,second)
{
	for( int i=0; i<eUnknownVideoFormat; i++ )
		m_resolutions[i] = (WORD)-1;

	if( first.GetPayloadType() != GetPayloadType() )
		return;
	if( second.GetPayloadType() != GetPayloadType() )
		return;

	const CVideoCapH263*  pFirst  = (CVideoCapH263*) &first;
	const CVideoCapH263*  pSecond = (CVideoCapH263*) &second;
	for( int i=0; i<eUnknownVideoFormat; i++ )
	{
		if( (WORD)-1 == pFirst->m_resolutions[i]  ||  (WORD)-1 == pSecond->m_resolutions[i] )
			continue;
		m_resolutions[i] = (pFirst->m_resolutions[i] < pSecond->m_resolutions[i]) ?
				pFirst->m_resolutions[i] : pSecond->m_resolutions[i];
	}
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH263::CVideoCapH263(const h263CapStruct& tStruct)
{
	m_resolutions[eSQcif] = MpiToFps(tStruct.sqcifMPI);
	m_resolutions[eQcif]  = MpiToFps(tStruct.qcifMPI);
	m_resolutions[eCif]   = MpiToFps(tStruct.cifMPI);
	m_resolutions[e4Cif]  = MpiToFps(tStruct.cif4MPI);
	m_resolutions[e16Cif] = MpiToFps(tStruct.cif16MPI);
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH263::~CVideoCapH263()
{
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH263& CVideoCapH263::operator= (const CVideoCapH263& other)
{
	if( this == &other )
		return *this;

	for( int i=0; i<eUnknownVideoFormat; i++ )
		m_resolutions[i] = other.m_resolutions[i];

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapH263::Serialize(CSegment& rSegCap) const
{
	rSegCap << (DWORD)GetPayloadType();
	//cout << "Payload type: " << GetPayloadType() << endl;
	for( int i=0; i<eUnknownVideoFormat; i++ )
		rSegCap << m_resolutions[i];
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapH263::DeSerialize(CSegment& rSegCap)
{
	for( int i=0; i<eUnknownVideoFormat; i++ )
		rSegCap >> m_resolutions[i];
}

/////////////////////////////////////////////////////////////////////////////
CVideoCap* CVideoCapH263::Clone() const
{
	return new CVideoCapH263(*this);
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapH263::FillStructH323(h263CapStruct* pStruct,const DWORD maxVideoRate,
		const BYTE direction,const BYTE role) const
{
	pStruct->header.xmlHeader.dynamicType   = eH263CapCode;
	pStruct->header.xmlHeader.dynamicLength = sizeof(h263CapStruct);
	pStruct->header.direction   = direction; // cmCapReceive;
	pStruct->header.type        = cmCapVideo;
	pStruct->header.roleLabel   = role;      // kRolePeople;
	pStruct->header.capTypeCode = eH263CapCode;

	pStruct->maxBitRate   = maxVideoRate;
	pStruct->hrd_B        = 0;
	pStruct->bppMaxKb     = 0;
	pStruct->slowSqcifMPI = -1;
	pStruct->slowQcifMPI  = -1;
	pStruct->slowCifMPI   = -1;
	pStruct->slowCif4MPI  = -1;
	pStruct->slowCif16MPI = -1;

	pStruct->sqcifMPI = FpsToMpi(m_resolutions[eSQcif]);
	pStruct->qcifMPI  = FpsToMpi(m_resolutions[eQcif]);
	pStruct->cifMPI   = FpsToMpi(m_resolutions[eCif]);
	pStruct->cif4MPI  = FpsToMpi(m_resolutions[e4Cif]);
	pStruct->cif16MPI = FpsToMpi(m_resolutions[e16Cif]);
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapH263::CreatePresentationCap()
{
	for( int i=0; i<eUnknownVideoFormat; i++ )
		m_resolutions[i] = (WORD)-1;

	m_resolutions[eQcif] = 30;
	m_resolutions[eCif]  = 30;
	m_resolutions[e4Cif] = 15;
}


/////////////////////////////////////////////////////////////////////////////
//
//   CVideoCapH264 - H264 Video Capability element
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CVideoCapH264::CVideoCapH264()
{
	m_levelValue = H264_Level_2; // 30 fps
	m_mbps       = (DWORD)-1;
	m_fs         = (DWORD)-1;
	m_dpb        = (DWORD)-1;
	m_brAndCpb   = (DWORD)-1;
	m_staticMB	 = (DWORD)-1;
	m_aspectRatio = (DWORD)-1;
	m_profileValue = H264_Profile_BaseLine;// H264_Profile_Main
	m_maxFR        = 0;
	m_H264mode     = H264_standard;
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH264::CVideoCapH264(const enVideoModeH264 mode,const DWORD ratio, const DWORD staticMB,const DWORD profile)
{
	m_aspectRatio = ratio;
	m_staticMB = staticMB;
	m_profileValue = profile;
	m_maxFR        = 0;
	m_H264mode     = H264_standard;
	TRACEINTO << "CVideoCapH264::CVideoCapH264:m_profileValue - " << m_profileValue;
	switch( mode )
	{

// 	        case eVideoModeHD1080p60:
// 		{
// 			m_levelValue = H264_Level_4;
// 			m_mbps       = (DWORD)1180;
// 			m_fs         = (DWORD)-1;
// 			m_dpb        = (DWORD)-1;
// 			m_brAndCpb   = (DWORD)-1;
// 			m_staticMB	 = (DWORD)-1;
// 			break;
// 		}

		case eVideoModeHD1080:
		{
			m_levelValue = H264_Level_4;
			m_mbps       = (DWORD)-1;
			m_fs         = (DWORD)-1;
			m_dpb        = (DWORD)-1;
			m_brAndCpb   = (DWORD)-1;
			m_staticMB	 = (DWORD)-1;
			break;
		}
		case eVideoModeHD1080p60:
		{
			TRACEINTO << "CVideoCapH264::CVideoCapH264:simulating 1080 60fps mode";
			m_levelValue =  H264_Level_3_1;
			m_mbps       =  980;
			m_fs         =  32;
			m_dpb        = (DWORD)-1;
			m_brAndCpb   = (DWORD)-1;
			m_staticMB	 = (DWORD)-1;
			break;
		}
		case eVideoModeHD720:
		{
			m_levelValue =  H264_Level_3_1;
			m_mbps       = (DWORD)-1;
			m_fs         =  15;
			m_dpb        = (DWORD)-1;
			m_brAndCpb   = (DWORD)-1;
			m_staticMB	 = (DWORD)-1;
			break;
		}
		case eVideoModeSD60:
		{
			m_levelValue = H264_Level_3;
			m_mbps       = 162;
			m_fs         = (DWORD)-1;
			m_dpb        = (DWORD)-1;
			m_brAndCpb   = (DWORD)-1;
			m_staticMB	 = (DWORD)-1;
			break;
		}
		case eVideoModeW4CIF30:
		{
			m_levelValue = H264_Level_3;
			m_mbps       = 139;
			m_fs         = 9;
			m_dpb        = (DWORD)-1;
			m_brAndCpb   = (DWORD)-1;
			m_staticMB	 = (DWORD)-1;
			break;
		}
		case eVideoModeSD30:
		{
			m_levelValue = H264_Level_3;
			m_mbps       = (DWORD)-1;
			m_fs         = (DWORD)-1;
			m_dpb        = (DWORD)-1;
			m_brAndCpb   = (DWORD)-1;
			m_staticMB	 = (DWORD)-1;
			break;
		}
		case eVideoModeSD15:
		{
			m_levelValue = H264_Level_2_2;
			m_mbps       = (DWORD)-1;
			m_fs         = (DWORD)-1;
			m_dpb        = (DWORD)-1;
			m_brAndCpb   = (DWORD)-1;
			m_staticMB	 = (DWORD)-1;
			break;
		}
		case eVideoMode2Cif30:
		{
			m_levelValue = H264_Level_2_1;
			m_mbps       = (DWORD)-1;
			m_fs         = (DWORD)-1;
			m_dpb        = (DWORD)-1;
			m_brAndCpb   = (DWORD)-1;
			m_staticMB	 = (DWORD)-1;
			break;
		}
		case eVideoModeSD7_5:
		{
			m_levelValue = H264_Level_1_2;
			m_mbps       = 20;
			m_fs         = 7;
			m_dpb        = (DWORD)-1;
			m_brAndCpb   = (DWORD)-1;
			m_staticMB	 = (DWORD)-1;
			break;
		}
		case eVideoModeCif:
		default:
		{
			m_levelValue = H264_Level_2; // 30 fps
			m_mbps       = (DWORD)-1;
			m_fs         = (DWORD)-1;
			m_dpb        = (DWORD)-1;
			m_brAndCpb   = (DWORD)-1;
			m_staticMB	 = (DWORD)-1;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH264::CVideoCapH264(const CVideoCapH264& other)
		: CVideoCap(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
// creates highest common video cap
CVideoCapH264::CVideoCapH264(const CVideoCap& first,const CVideoCap& second)
		: CVideoCap(first,second)
{
	if( first.GetPayloadType() != GetPayloadType() )
		return;
	if( second.GetPayloadType() != GetPayloadType() )
		return;

	m_maxFR        = 0;
	m_H264mode     = H264_standard;

	const CVideoCapH264*  pFirst  = (CVideoCapH264*) &first;
	const CVideoCapH264*  pSecond = (CVideoCapH264*) &second;

	if( pFirst->m_aspectRatio == (DWORD)-1  || pSecond->m_aspectRatio == (DWORD)-1 )
		m_aspectRatio = (DWORD)-1;
	else
		m_aspectRatio = (pFirst->m_aspectRatio < pSecond->m_aspectRatio ) ?
				pFirst->m_aspectRatio : pSecond->m_aspectRatio;

	if( pFirst->m_staticMB == (DWORD)-1  || pSecond->m_staticMB == (DWORD)-1 )
		m_staticMB = (DWORD)-1;
	else
		m_staticMB = (pFirst->m_staticMB < pSecond->m_staticMB ) ?
				pFirst->m_staticMB : pSecond->m_staticMB;

	if( pFirst->m_levelValue < pSecond->m_levelValue )
	{
		m_levelValue = pFirst->m_levelValue;
		m_mbps       = pFirst->m_mbps;
		m_fs         = pFirst->m_fs;
		m_dpb        = pFirst->m_dpb;
		m_brAndCpb   = pFirst->m_brAndCpb;
	}
	else if( pFirst->m_levelValue == pSecond->m_levelValue )
	{
		m_levelValue = pFirst->m_levelValue;
		/*if( pFirst->m_mbps < pSecond->m_mbps || pFirst->m_mbps == (DWORD)-1 )
		{
			m_mbps       = pFirst->m_mbps;
			m_fs         = pFirst->m_fs;
			m_dpb        = pFirst->m_dpb;
			m_brAndCpb   = pFirst->m_brAndCpb;
		}
		else
		{
			m_mbps       = pSecond->m_mbps;
			m_fs         = pSecond->m_fs;
			m_dpb        = pSecond->m_dpb;
			m_brAndCpb   = pSecond->m_brAndCpb;
		}*/
		m_mbps       = min ((signed long)pFirst->m_mbps, (signed long)pSecond->m_mbps);
		m_fs         = min ((signed long)pFirst->m_fs, (signed long)pSecond->m_mbps);
		m_dpb        = min ((signed long)pFirst->m_dpb, (signed long)pSecond->m_mbps);
		m_brAndCpb   = min ((signed long)pFirst->m_brAndCpb, (signed long)pSecond->m_mbps);
	}
	else
	{
		m_levelValue = pSecond->m_levelValue;
		m_mbps       = pSecond->m_mbps;
		m_fs         = pSecond->m_fs;
		m_dpb        = pSecond->m_dpb;
		m_brAndCpb   = pSecond->m_brAndCpb;
	}

	//==============================================================================
	// EPSim logic caused lower levels with higher frame sizes to determine higher
	// FS than the actual highest common - overrided fs to the actual common
	//==============================================================================
	//m_fs = min ((signed long)pFirst->m_fs, (signed long)pSecond->m_fs);
	CSmallString log;
	log << "CVideoCapH264::CVideoCapH264 - Simulated FS brought to common. first level " << pFirst->m_levelValue
		<< " second level " << pSecond->m_levelValue << " overridden to lowest FS " << m_fs;
	PTRACE(eLevelInfoNormal, log.GetString());
	////////////////////////////////////////////////


	if (pFirst -> m_profileValue == H264_Profile_High && pSecond -> m_profileValue == H264_Profile_High)
		m_profileValue = H264_Profile_High;
//	else if (pFirst -> m_profileValue == H264_Profile_Main && pSecond -> m_profileValue == H264_Profile_Main)// TIP
	//	m_profileValue = H264_Profile_Main;
	else
		m_profileValue = H264_Profile_BaseLine;

}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH264::CVideoCapH264(const h264CapStruct& tStruct)
{
	m_levelValue = tStruct.levelValue;
	m_mbps       = tStruct.customMaxMbpsValue;
	m_fs         = tStruct.customMaxFsValue;
	m_dpb        = tStruct.customMaxDpbValue;
	m_brAndCpb   = tStruct.customMaxBrAndCpbValue;
	m_staticMB	 = (DWORD)-1;//tStruct.maxStaticMbpsValue;
	m_aspectRatio = tStruct.sampleAspectRatiosValue;
	m_profileValue = tStruct.profileValue;
	m_maxFR        = tStruct.maxFR;
	m_H264mode     = tStruct.H264mode;
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH264::~CVideoCapH264()
{
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapH264& CVideoCapH264::operator= (const CVideoCapH264& other)
{
	if( this == &other )
		return *this;

	m_levelValue   = other.m_levelValue;
	m_mbps         = other.m_mbps;
	m_fs           = other.m_fs;
	m_dpb          = other.m_dpb;
	m_brAndCpb     = other.m_brAndCpb;
	m_aspectRatio  = other.m_aspectRatio;
	m_profileValue = other.m_profileValue;
	m_maxFR        = other.m_maxFR;
	m_H264mode     = other.m_H264mode;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapH264::Serialize(CSegment& rSegCap) const
{
	//cout << "Payload type: " << GetPayloadType() << endl;

	rSegCap << (DWORD)GetPayloadType()
			<< m_levelValue
			<< m_mbps
			<< m_fs
			<< m_dpb
			<< m_brAndCpb
			<< m_staticMB
			<< m_aspectRatio
			<<m_profileValue
			<< m_maxFR
			<< m_H264mode;
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapH264::DeSerialize(CSegment& rSegCap)
{
	rSegCap >> m_levelValue
			>> m_mbps
			>> m_fs
			>> m_dpb
			>> m_brAndCpb
			>> m_staticMB
			>> m_aspectRatio
			>>m_profileValue
			>> m_maxFR
			>> m_H264mode;
}

/////////////////////////////////////////////////////////////////////////////
CVideoCap* CVideoCapH264::Clone() const
{
	return new CVideoCapH264(*this);
}



/////////////////////////////////////////////////////////////////////////////
void CVideoCapH264::FillStructH323(h264CapStruct* pStruct,const DWORD maxVideoRate,
		const BYTE direction,const BYTE role) const
{
	/// maxBitRate = units of 100 (320k = 3200)
	pStruct->header.xmlHeader.dynamicType   = eH264CapCode;
	pStruct->header.xmlHeader.dynamicLength = sizeof(h264CapStruct);
	pStruct->header.direction   = direction;//cmCapReceive;
	pStruct->header.type        = cmCapVideo;
	pStruct->header.roleLabel   = role;//kRolePeople;
	pStruct->header.capTypeCode = eH264CapCode;

	pStruct->customMaxMbpsValue      = m_mbps;
	pStruct->customMaxFsValue        = m_fs;
	pStruct->customMaxDpbValue       = m_dpb;
	pStruct->customMaxBrAndCpbValue  = m_brAndCpb;
	pStruct->maxStaticMbpsValue      = -1;//m_staticMB;
	pStruct->sampleAspectRatiosValue = m_aspectRatio;
	pStruct->maxFR                   = m_maxFR;
	pStruct->H264mode                = m_H264mode;

	pStruct->maxBitRate             = maxVideoRate;

	TRACEINTO << "CVideoCapH264::FillStructH323:m_profileValue - " << m_profileValue;

	pStruct->profileValue           = m_profileValue;
	pStruct->levelValue             = m_levelValue;
	if( m_profileValue == 8)
		pStruct->packetizationMode = 1;
}
//N.A. DEBUG VP8
/////////////////////////////////////////////////////////////////////////////
//
//   CVideoCapVP8 - Vp8 Video Capability element
//
/////////////////////////////////////////////////////////////////////////////

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CVideoCapVP8::CVideoCapVP8(): m_maxFR(30),m_maxFS(3600),m_maxBitRate(1920)
{}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CVideoCapVP8::CVideoCapVP8(const CVideoCapVP8& other)
		: CVideoCap(other)
{
	*this = other;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CVideoCapVP8& CVideoCapVP8::operator= (const CVideoCapVP8& other)
{
	if( this == &other )
			return *this;

		m_maxFR      	= other.m_maxFR;
		m_maxFS    	 	= other.m_maxFS;
		m_maxBitRate	= other.m_maxBitRate;

		return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ // creates highest common video cap
CVideoCapVP8::CVideoCapVP8(const CVideoCap& first,const CVideoCap& second)
		: CVideoCap(first,second)
{
	if( first.GetPayloadType() != GetPayloadType() )
		return;
	if( second.GetPayloadType() != GetPayloadType() )
		return;

	//m_maxFR        = 0;

	const CVideoCapVP8*  pFirst  = (CVideoCapVP8*) &first;
	const CVideoCapVP8*  pSecond = (CVideoCapVP8*) &second;

	/*
	if( pFirst->m_aspectRatio == (DWORD)-1  || pSecond->m_aspectRatio == (DWORD)-1 )
		m_aspectRatio = (DWORD)-1;
	else
		m_aspectRatio = (pFirst->m_aspectRatio < pSecond->m_aspectRatio ) ? pFirst->m_aspectRatio : pSecond->m_aspectRatio;
*/
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoCapVP8::Serialize(CSegment& rSegCap) const
{
	rSegCap << (DWORD)GetPayloadType()
			<< m_maxFR
			<< m_maxFS
			<< m_maxBitRate;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoCapVP8::DeSerialize(CSegment& rSegCap)
{
	rSegCap >> m_maxFR
			>> m_maxFS
			>> m_maxBitRate;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CVideoCap* CVideoCapVP8::Clone() const
{
	return new CVideoCapVP8(*this);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoCapVP8::FillStructH323(vp8CapStruct* pStruct,const DWORD maxVideoRate,
		const BYTE direction,const BYTE role) const
{
	pStruct->header.xmlHeader.dynamicType   = eVP8CapCode;
	pStruct->header.xmlHeader.dynamicLength = sizeof(vp8CapStruct);
	pStruct->header.direction   = direction;//cmCapReceive;
	pStruct->header.type        = cmCapVideo;
	pStruct->header.roleLabel   = role;//kRolePeople;
	pStruct->header.capTypeCode = eVP8CapCode;

	pStruct->maxFR                   = m_maxFR;
	pStruct->maxFS                	 = m_maxFS;
	pStruct->maxBitRate				 = m_maxBitRate;

	//Removed from struct
	//pStruct->height					 = m_Height;
	//pStruct->width					 = m_Width;
	//pStruct->aspectRatio             = m_aspectRatio; // N.A. DEBUG - maybe will need to remove

	TRACEINTO << "CVideoCapVP8::FillStructH323: sizeof(vp8CapStruct) = "<< sizeof(vp8CapStruct) ;

}
//N.A. DEBUG VP8
/////////////////////////////////////////////////////////////////////////////
//   CVideoCapVP8 - Vp8 Video Capability element End					   //
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//
//   CVideoCapSVC - SVC Video Capability element
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CVideoCapSVC::CVideoCapSVC()
{
	memset(&m_operationPoints, 0, sizeof(m_operationPoints));
	memset(&m_recvStreamsGroup, 0, sizeof(m_recvStreamsGroup));
	memset(&m_sendStreamsGroup, 0, sizeof(m_recvStreamsGroup));
	m_scalableLayerId = 0;
	m_isLegacy = 0;

}

/////////////////////////////////////////////////////////////////////////////
CVideoCapSVC::CVideoCapSVC(const enVideoModeH264 mode,const DWORD ratio, const DWORD staticMB,const DWORD profile)
: CVideoCapH264(mode, ratio, staticMB, profile)
{
	memset(&m_operationPoints, 0, sizeof(m_operationPoints));
	memset(&m_recvStreamsGroup, 0, sizeof(m_recvStreamsGroup));
	memset(&m_sendStreamsGroup, 0, sizeof(m_recvStreamsGroup));
	m_scalableLayerId = 0;
	m_isLegacy = 0;
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapSVC::CVideoCapSVC(const CVideoCapSVC& other)
		: CVideoCapH264(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
// creates highest common video cap
// first is EP, second is conf
CVideoCapSVC::CVideoCapSVC(const CVideoCap& first,const CVideoCap& second)
		: CVideoCapH264(first,second)
{
	memset(&m_operationPoints, 0, sizeof(m_operationPoints));
	memset(&m_recvStreamsGroup, 0, sizeof(m_recvStreamsGroup));
	memset(&m_sendStreamsGroup, 0, sizeof(m_recvStreamsGroup));
	m_scalableLayerId = 0;
	m_isLegacy = 0;

	const CVideoCapSVC*  pFirst  = (CVideoCapSVC*) &first;
	const CVideoCapSVC*  pSecond = (CVideoCapSVC*) &second;

	if(first.GetPayloadType() != GetPayloadType()){
		return;
	}

	if(second.GetPayloadType() != GetPayloadType()){
		return;
	}

	*this = *pSecond; // for simulation, we just use the cap of conf

	m_sendStreamsGroup.numberOfStreams = 1;
	m_sendStreamsGroup.streams[0].streamSsrcId = 210501;
	int num = pSecond->GetOperationSet().numberOfOperationPoints;
	if(num > 0 ) {
		VIDEO_OPERATION_POINT_S op = pSecond->GetOperationSet().tVideoOperationPoints[num -1];
		m_sendStreamsGroup.streams[0].frameWidth = op.frameWidth;
		m_sendStreamsGroup.streams[0].frameHeight = op.frameHeight;
		m_sendStreamsGroup.streams[0].maxFrameRate = op.frameRate;

		TRACEINTO << "CVideoCapSVC::CVideoCapSVC - send stream - width : " << (DWORD)op.frameWidth
				<< " height : " << (DWORD)op.frameHeight
				<< " max frame rate : " << (DWORD)op.frameRate;

	}else{
        PASSERT(1);
		m_sendStreamsGroup.streams[0].frameWidth = 1280;
		m_sendStreamsGroup.streams[0].frameHeight = 720;
		m_sendStreamsGroup.streams[0].maxFrameRate = 7680;
	}

	return;
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapSVC::CVideoCapSVC(const svcCapStruct& tStruct)
: CVideoCapH264(tStruct.h264)
{
	m_operationPoints = tStruct.operationPoints;
	m_recvStreamsGroup = tStruct.recvStreamsGroup;
	m_sendStreamsGroup = tStruct.sendStreamsGroup;
	m_scalableLayerId = tStruct.scalableLayerId;
	m_isLegacy = tStruct.isLegacy;
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapSVC::~CVideoCapSVC()
{
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapSVC& CVideoCapSVC::operator= (const CVideoCapSVC& other)
{
	if( this == &other )
		return *this;

	CVideoCapH264::operator =(other);

	m_operationPoints = other.m_operationPoints;
	m_recvStreamsGroup = other.m_recvStreamsGroup;
	m_sendStreamsGroup = other.m_sendStreamsGroup;
	m_scalableLayerId = other.m_scalableLayerId;
	m_isLegacy = other.m_isLegacy;

	return *this;
}

void SerializeStreamGroup(CSegment& rSegCap, const STREAM_GROUP_S& streamGroup)
{
	rSegCap << streamGroup.streamGroupId
			<< (DWORD)streamGroup.numberOfStreams;

	for(int i = 0; i < streamGroup.numberOfStreams; ++i){
		rSegCap << streamGroup.streams[i].streamSsrcId;
		rSegCap << streamGroup.streams[i].frameWidth;
		rSegCap << streamGroup.streams[i].frameHeight;
		rSegCap << streamGroup.streams[i].maxFrameRate;
		rSegCap << streamGroup.streams[i].requstedStreamSsrcId;
	}

}

void DeSerializeStreamGroup(CSegment& rSegCap, STREAM_GROUP_S& streamGroup)
{
	rSegCap >> streamGroup.streamGroupId
			>> streamGroup.numberOfStreams;

	for(int i = 0; i < streamGroup.numberOfStreams; ++i){
		rSegCap >> streamGroup.streams[i].streamSsrcId;
		rSegCap >> streamGroup.streams[i].frameWidth;
		rSegCap >> streamGroup.streams[i].frameHeight;
		rSegCap >> streamGroup.streams[i].maxFrameRate;
		rSegCap >> streamGroup.streams[i].requstedStreamSsrcId;
	}

}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapSVC::Serialize(CSegment& rSegCap) const
{
	CVideoCapH264::Serialize(rSegCap);

	rSegCap << m_operationPoints.operationPointSetId
			<< (DWORD)m_operationPoints.numberOfOperationPoints;
	for(int i = 0; i < this->m_operationPoints.numberOfOperationPoints; ++i){
		rSegCap << (DWORD)this->m_operationPoints.tVideoOperationPoints[i].layerId;
		rSegCap << (DWORD)this->m_operationPoints.tVideoOperationPoints[i].Tid;
		rSegCap << (DWORD)this->m_operationPoints.tVideoOperationPoints[i].Did;
		rSegCap << (DWORD)this->m_operationPoints.tVideoOperationPoints[i].Qid;
		rSegCap << (DWORD)this->m_operationPoints.tVideoOperationPoints[i].Pid;
		rSegCap << (DWORD)this->m_operationPoints.tVideoOperationPoints[i].profile;
		rSegCap << (DWORD)this->m_operationPoints.tVideoOperationPoints[i].level;
		rSegCap << (DWORD)this->m_operationPoints.tVideoOperationPoints[i].frameWidth;
		rSegCap << (DWORD)this->m_operationPoints.tVideoOperationPoints[i].frameHeight;
		rSegCap << (DWORD)this->m_operationPoints.tVideoOperationPoints[i].frameRate;
		rSegCap << (DWORD)this->m_operationPoints.tVideoOperationPoints[i].maxBitRate;

	}

	SerializeStreamGroup(rSegCap, this->m_recvStreamsGroup);
	SerializeStreamGroup(rSegCap, this->m_sendStreamsGroup);

	rSegCap << (DWORD)m_scalableLayerId
	        << (DWORD)m_isLegacy;
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapSVC::DeSerialize(CSegment& rSegCap)
{
	CVideoCapH264::DeSerialize(rSegCap);
	DWORD dummy;
	rSegCap >> m_operationPoints.operationPointSetId
			>> dummy;
	m_operationPoints.numberOfOperationPoints = (APIU8)dummy;
	for(int i = 0; i < this->m_operationPoints.numberOfOperationPoints; ++i){

		rSegCap >> this->m_operationPoints.tVideoOperationPoints[i].layerId;
		rSegCap >> dummy;
		this->m_operationPoints.tVideoOperationPoints[i].Tid = (APIU8)dummy;
		rSegCap >> dummy;
		this->m_operationPoints.tVideoOperationPoints[i].Did= (APIU8)dummy;
		rSegCap >> dummy;
		this->m_operationPoints.tVideoOperationPoints[i].Qid= (APIU8)dummy;
		rSegCap >> dummy;
		this->m_operationPoints.tVideoOperationPoints[i].Pid= (APIU8)dummy;
		rSegCap >> dummy;
		this->m_operationPoints.tVideoOperationPoints[i].profile= (APIU16)dummy;
		rSegCap >> dummy;
		this->m_operationPoints.tVideoOperationPoints[i].level= (APIU8)dummy;
		rSegCap >> this->m_operationPoints.tVideoOperationPoints[i].frameWidth;
		rSegCap >> this->m_operationPoints.tVideoOperationPoints[i].frameHeight;
		rSegCap >> this->m_operationPoints.tVideoOperationPoints[i].frameRate;
		rSegCap >> this->m_operationPoints.tVideoOperationPoints[i].maxBitRate;

	}

	DeSerializeStreamGroup(rSegCap, this->m_recvStreamsGroup);
	DeSerializeStreamGroup(rSegCap, this->m_sendStreamsGroup);

	rSegCap >> dummy;
	m_scalableLayerId = (APIU8)dummy;
	rSegCap >> dummy;
	m_isLegacy = (APIU8)dummy;
}

/////////////////////////////////////////////////////////////////////////////
CVideoCap* CVideoCapSVC::Clone() const
{
	return new CVideoCapSVC(*this);
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapSVC::FillStructH323(svcCapStruct* pStruct,const DWORD maxVideoRate,
		const BYTE direction,const BYTE role) const
{
	CVideoCapH264::FillStructH323(&(pStruct->h264), maxVideoRate, direction, role);

	/// maxBitRate = units of 100 (320k = 3200)
	pStruct->h264.header.xmlHeader.dynamicType   = eSvcCapCode; //?
	pStruct->h264.header.xmlHeader.dynamicLength = sizeof(svcCapStruct); //?
	pStruct->h264.header.direction   = direction;//cmCapReceive;
	pStruct->h264.header.type        = cmCapVideo;
	pStruct->h264.header.roleLabel   = role;//kRolePeople;
	pStruct->h264.header.capTypeCode = eSvcCapCode; //?

	pStruct->h264.customMaxMbpsValue      = m_mbps;
	pStruct->h264.customMaxFsValue        = m_fs;
	pStruct->h264.customMaxDpbValue       = m_dpb;
	pStruct->h264.customMaxBrAndCpbValue  = m_brAndCpb;
	pStruct->h264.maxStaticMbpsValue      = -1;//m_staticMB;
	pStruct->h264.sampleAspectRatiosValue = m_aspectRatio;
	pStruct->h264.maxFR                   = m_maxFR;
	pStruct->h264.H264mode                = m_H264mode;

	pStruct->h264.maxBitRate             = maxVideoRate;

	TRACEINTO << "CVideoCapH264::FillStructH323:m_profileValue - " << m_profileValue;

	pStruct->h264.profileValue           = m_profileValue;
	pStruct->h264.levelValue             = m_levelValue;
	if( m_profileValue == 8)
		pStruct->h264.packetizationMode = 1;

    // svc fields
	pStruct->operationPoints = this->m_operationPoints;
	pStruct->recvStreamsGroup = this->m_recvStreamsGroup;
	pStruct->sendStreamsGroup = this->m_sendStreamsGroup;
	pStruct->scalableLayerId = this->m_scalableLayerId;
	pStruct->isLegacy = this->m_isLegacy;
}


/////////////////////////////////////////////////////////////////////////////
//
//   CSdesEpCap - SDES Capability element
//
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CSdesEpCap::CSdesEpCap(cmCapDataType capDataType)
{
	m_mediaType = capDataType;

	m_tag = 1;	 ///TBD MCMS value ?
	m_cryptoSuite = eAes_Cm_128_Hmac_Sha1_80;

	memset(&m_sessionParams, 0, sizeof(sdesSessionParamsStruct));
	m_numKeyParams = 1;

	m_sdesKeyParamsStruct.keyMethod = eSdesInlineKeyMethod;
	strncpy(m_sdesKeyParamsStruct.keyInfo.keySalt, "MXA8vSrWomMhhlum98R5bDHH8lTHZrsGXZN4DhP9", MAX_BASE64_KEY_SALT_LEN);
	m_sdesKeyParamsStruct.keyInfo.bIsLifeTimeInUse = FALSE;//TRUE;
	m_sdesKeyParamsStruct.keyInfo.lifetime = DEFAULT_LIFE_TIME;
	m_sdesKeyParamsStruct.keyInfo.bIsMkiInUse = FALSE;//TRUE;
	m_sdesKeyParamsStruct.keyInfo.mkiValue = 1;
	m_sdesKeyParamsStruct.keyInfo.bIsMkiValueLenInUse = FALSE;//TRUE;
	m_sdesKeyParamsStruct.keyInfo.mkiValueLen = 0;//1;

}


/////////////////////////////////////////////////////////////////////////////
CSdesEpCap::CSdesEpCap(sdesCapStruct& tStruct)
{
	m_mediaType = (cmCapDataType)tStruct.header.type;

	m_tag = tStruct.tag;
	m_cryptoSuite = tStruct.cryptoSuite;
	memcpy( &m_sessionParams, &tStruct.sessionParams, sizeof(sdesSessionParamsStruct) );
	m_numKeyParams = tStruct.numKeyParams;
	memcpy( &m_sdesKeyParamsStruct, tStruct.keyParamsList, sizeof(sdesKeyParamsStruct) );

}

/*
/////////////////////////////////////////////////////////////////////////////
void CSdesEpCap::Serialize(CSegment& rSegCap) const
{
	rSegCap << m_tag
			<< m_cryptoSuite;
	rSegCap.Put((BYTE*)(&m_sessionParams), sizeof(sdesSessionParamsStruct));
	rSegCap	<< m_numKeyParams;
	rSegCap.Put((BYTE*)(&m_sdesKeyParamsStruct), sizeof(sdesKeyParamsStruct));
	rSegCap	<< m_mediaType;
}


/////////////////////////////////////////////////////////////////////////////
void CSdesEpCap::DeSerialize(CSegment& rSegCap)
{
	rSegCap >> m_tag
			>> m_cryptoSuite;
	rSegCap.Get((BYTE*)(&m_sessionParams), sizeof(sdesSessionParamsStruct));
	rSegCap	>> m_numKeyParams;
	rSegCap.Get((BYTE*)(&m_sdesKeyParamsStruct), sizeof(sdesKeyParamsStruct));
	rSegCap	>> m_mediaType;
}
*/

/////////////////////////////////////////////////////////////////////////////
CSdesEpCap* CSdesEpCap::Clone() const
{
	return new CSdesEpCap(*this);
}


/////////////////////////////////////////////////////////////////////////////
void CSdesEpCap::FillStructSdes(sdesCapStruct* pStruct, const BYTE direction, const BYTE role) const
{
	pStruct->header.xmlHeader.dynamicType = eSdesCapCode;
	pStruct->header.xmlHeader.dynamicLength = sizeof(sdesCapStruct);
	pStruct->header.direction   = direction;
	pStruct->header.type        = GetSdesMediaType();	// cmCapAudio, cmCapVideo or cmCapData
	pStruct->header.roleLabel   = role;
	pStruct->header.capTypeCode = eSdesCapCode;

	pStruct->tag = m_tag; ///TBD MCMS value ?
	pStruct->cryptoSuite = m_cryptoSuite;
	memcpy( &pStruct->sessionParams, &m_sessionParams, sizeof(sdesSessionParamsStruct) );
	pStruct->numKeyParams = m_numKeyParams;
	pStruct->xmlDynamicProps.numberOfDynamicParts = 1;
	pStruct->xmlDynamicProps.sizeOfAllDynamicParts = sizeof(xmlSdesKeyParamsStruct);

	xmlSdesKeyParamsStruct *pXmlElem = (xmlSdesKeyParamsStruct *) &pStruct->keyParamsList;
	xmlDynamicHeader *pXmlHeader = &pXmlElem->xmlHeader;
	pXmlHeader->dynamicType 	= eSdesCapCode;
	pXmlHeader->dynamicLength 	= sizeof(sdesKeyParamsStruct);

	memcpy( &pXmlElem->elem, &m_sdesKeyParamsStruct, sizeof(sdesKeyParamsStruct) );

}

/////////////////////////////////////////////////////////////////////////////
//
//   CVideoCapRTV - RTV Video Capability element
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CVideoCapRTV::CVideoCapRTV(ERtvVideoModeType rtvVideoMode/*=e_rtv_HD720Symmetric*/)
{
	m_rtvVideoMode = rtvVideoMode;
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapRTV::CVideoCapRTV(const CVideoCapRTV& other)
		: CVideoCap(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
// creates highest common video cap
CVideoCapRTV::CVideoCapRTV(const CVideoCap& first,const CVideoCap& second)
		: CVideoCap(first,second)
{
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapRTV::CVideoCapRTV(const rtvCapStruct& tStruct)
{
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapRTV::~CVideoCapRTV()
{
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapRTV& CVideoCapRTV::operator= (const CVideoCapRTV& other)
{
	if( this == &other )
		return *this;

	m_rtvVideoMode = other.m_rtvVideoMode;
	
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapRTV::Serialize(CSegment& rSegCap) const
{
	rSegCap << (DWORD)GetPayloadType();
	rSegCap << m_rtvVideoMode; 
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapRTV::DeSerialize(CSegment& rSegCap)
{
	rSegCap >> m_rtvVideoMode;

}

/////////////////////////////////////////////////////////////////////////////
CVideoCap* CVideoCapRTV::Clone() const
{
	return new CVideoCapRTV(*this);
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapRTV::FillStructH323(rtvCapStruct* pStruct,const DWORD maxVideoRate,
		const BYTE direction,const BYTE role) const
{
	pStruct->header.xmlHeader.dynamicType   = eRtvCapCode;
	pStruct->header.xmlHeader.dynamicLength = sizeof(rtvCapStruct);
	pStruct->header.direction   = direction; // cmCapReceive;
	pStruct->header.type        = cmCapVideo;
	pStruct->header.roleLabel   = role;      // kRolePeople;
	pStruct->header.capTypeCode = eRtvCapCode;

	pStruct->rtcpFeedbackMask = 0;
	pStruct->numOfItems = 4;
	
	int itemId = 0;
	if (m_rtvVideoMode >= e_rtv_HD720Asymmetric)
	{		
		//HD
		pStruct->rtvCapItem[itemId].capabilityID = 263;
		pStruct->rtvCapItem[itemId].widthVF = 1280;
		pStruct->rtvCapItem[itemId].heightVF = 720;
		pStruct->rtvCapItem[itemId].fps = 30;
		pStruct->rtvCapItem[itemId].maxBitrateInBps = 15000;
		++itemId;
	}
	
	if (m_rtvVideoMode >= e_rtv_VGA30)
	{		
		//VGA
		pStruct->rtvCapItem[itemId].capabilityID = 4359;
		pStruct->rtvCapItem[itemId].widthVF = 640;
		pStruct->rtvCapItem[itemId].heightVF = 480;
		pStruct->rtvCapItem[itemId].fps = 30;
		pStruct->rtvCapItem[itemId].maxBitrateInBps = 6000;
		++itemId;
	}

	if (m_rtvVideoMode >= e_rtv_CIF15)
	{		
		//CIF
		pStruct->rtvCapItem[itemId].capabilityID = 8455;
		pStruct->rtvCapItem[itemId].widthVF = 352;
		pStruct->rtvCapItem[itemId].heightVF = 288;
		pStruct->rtvCapItem[itemId].fps = 15;
		pStruct->rtvCapItem[itemId].maxBitrateInBps = 2500;
		++itemId;
	}

	//QCIF
	pStruct->rtvCapItem[itemId].capabilityID = 12551;
	pStruct->rtvCapItem[itemId].widthVF = 176;
	pStruct->rtvCapItem[itemId].heightVF = 144;
	pStruct->rtvCapItem[itemId].fps = 14;
	pStruct->rtvCapItem[itemId].maxBitrateInBps = 1800;

	
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapRTV::CreatePresentationCap()
{
}
////////////////////////////////////////////////////////
CVideoCapMSSvc::CVideoCapMSSvc()
{
	m_width = 0;
	m_height=0;
	m_maxFrameRate=0;
	m_packatizationmode = 1;
	m_aspectRatio = 0;

}

////////////////////MS SVC caps
////////////////////////////////////////////////////////////////////
//CVideoCapMSSvc::CVideoCapMSSvc(Eh264VideoModeType mssvcVideoMode/*=e_rtv_HD720Symmetric*/)
//{
	//m_msSvcVideoMode = mssvcVideoMode;
//	m_width = 0;
//	m_height=0;
//	m_maxFrameRate=0;
//	m_packatizationmode = 1;
//	m_aspectRatio = 0;
//
//}
CVideoCapMSSvc::CVideoCapMSSvc(const CVideoCapMSSvc& other)
		: CVideoCap(other)
{
	*this = other;
}
// creates highest common video cap
CVideoCapMSSvc::CVideoCapMSSvc(const CVideoCap& first,const CVideoCap& second)
		: CVideoCap(first,second)
{
	if( first.GetPayloadType() != GetPayloadType() )
		return;
	if( second.GetPayloadType() != GetPayloadType() )
		return;



	const CVideoCapMSSvc*  pFirst  = (CVideoCapMSSvc*) &first;
	const CVideoCapMSSvc*  pSecond = (CVideoCapMSSvc*) &second;

	m_width        = 0;
	m_height     = 0;
	m_maxFrameRate = 0;
	m_packatizationmode =1;
	m_aspectRatio =0;






	//==============================================================================
	// EPSim logic caused lower levels with higher frame sizes to determine higher
	// FS than the actual highest common - overrided fs to the actual common
	//==============================================================================
	//m_fs = min ((signed long)pFirst->m_fs, (signed long)pSecond->m_fs);
	CSmallString log;
	log << "CVideoCapMSSvc::CVideoCapMSSvc - Simulated m_width brought to common. first level " << pFirst->m_width;

	PTRACE(eLevelInfoNormal, log.GetString());
	////////////////////////////////////////////////


}

/////////////////////////////////////////////////////////////////////////////
CVideoCapMSSvc::CVideoCapMSSvc(const msSvcCapStruct& tStruct)
{
	m_width = tStruct.width;
	m_height = tStruct.height;
	m_maxFrameRate = m_maxFrameRate;
	m_packatizationmode =1;
	m_aspectRatio =0;
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapMSSvc::~CVideoCapMSSvc()
{
}

/////////////////////////////////////////////////////////////////////////////
CVideoCapMSSvc& CVideoCapMSSvc::operator= (const CVideoCapMSSvc& other)
{
	if( this == &other )
		return *this;

	m_width         = other.m_width;
	m_height        = other.m_height;
	m_maxFrameRate   = other.m_maxFrameRate;
	m_packatizationmode          = other.m_packatizationmode;
	m_aspectRatio     = other.m_aspectRatio;


	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapMSSvc::Serialize(CSegment& rSegCap) const
{
	rSegCap     << (DWORD)GetPayloadType()
				<< m_width
				<< m_height
				<< m_maxFrameRate
				<< m_packatizationmode
				<< m_aspectRatio;

}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapMSSvc::DeSerialize(CSegment& rSegCap)
{
	rSegCap >> m_width
				>> m_height
				>> m_maxFrameRate
				>> m_packatizationmode
				>> m_aspectRatio;


}

/////////////////////////////////////////////////////////////////////////////
CVideoCap* CVideoCapMSSvc::Clone() const
{
	return new CVideoCapMSSvc(*this);

}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapMSSvc::FillStructH323(msSvcCapStruct* pStruct,const DWORD maxVideoRate,
		const BYTE direction,const BYTE role) const
{

	//m_fs = min ((signed long)pFirst->m_fs, (signed long)pSecond->m_fs);
	CSmallString log;
	log << "CVideoCapMSSvc::FillStructH323 - Simulated m_width brought to common. first level " ;

	PTRACE(eLevelInfoNormal, log.GetString());
	pStruct->header.xmlHeader.dynamicType   = eMsSvcCapCode;
	pStruct->header.xmlHeader.dynamicLength = sizeof(msSvcCapStruct);
	pStruct->header.direction   = direction; // cmCapReceive;
	pStruct->header.type        = cmCapVideo;
	pStruct->header.roleLabel   = role;      // kRolePeople;
	pStruct->header.capTypeCode = eMsSvcCapCode;

	pStruct->rtcpFeedbackMask = 0;
//	pStruct->numOfItems = 4;
	//ms svc cap in cas is 0 only using VSR we get real values.
	pStruct->width = 0;
	pStruct->height = 0;
	pStruct->maxFrameRate =0;
	pStruct->packetizationMode = 1;
	pStruct->maxBitRate= maxVideoRate;
	pStruct->aspectRatio=0;

}

/////////////////////////////////////////////////////////////////////////////
void CVideoCapMSSvc::CreatePresentationCap()
{
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//							The End
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
