//+========================================================================+
//                            CMINFO.CPP                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CMINFO.CPP                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |09/07/02     |                                                     |
//+========================================================================+
#include  "CommModeInfo.h"

#include "IpCommonUtilTrace.h"
#include "H221.h"
#include "H263.h"
#include "Trace.h"
#include "TraceStream.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialize static CommunicationMode array
// The table has start point for every type of media (audio, video, data) any added in that
// table must change this defines as well (defined in CMinfo.h)

TCommunicationMode CComModeInfo::g_comModeTbl[] =
{
	/*0*/
	{cmCapAudio,       Au_Neutral,                 (CapEnum)MODE_OFF,                    0,                                   (DWORD)0,         NonFrameBasedFPP},
	{cmCapAudio,       A_Law_OU,                   eG711Alaw64kCapCode,                  sizeof(g711Alaw64kCapStruct),        (DWORD)64*_K_,    NonFrameBasedFPP},
	{cmCapAudio,       U_Law_OU,                   eG711Ulaw64kCapCode,                  sizeof(g711Ulaw64kCapStruct),        (DWORD)64*_K_,    NonFrameBasedFPP},
	{cmCapAudio,       G722_m1,                    eG722_64kCapCode,                     sizeof(g722_64kCapStruct),           (DWORD)64*_K_,    NonFrameBasedFPP},
	{cmCapAudio,       Au_Off_U,                   (CapEnum)MODE_OFF,                    0,                                   (DWORD)0,         NonFrameBasedFPP},
	{cmCapAudio,       G723_1_Command,             eG7231CapCode,                        sizeof(g7231CapStruct),              (DWORD)7*_K_,     FrameBasedFPP},
	{cmCapAudio,       A_Law_OF,                   eG711Alaw64kCapCode,                  sizeof(g711Alaw64kCapStruct),        (DWORD)64*_K_,    NonFrameBasedFPP},
	{cmCapAudio,       U_Law_OF,                   eG711Ulaw64kCapCode,                  sizeof(g711Ulaw64kCapStruct),        (DWORD)64*_K_,    NonFrameBasedFPP},
	{cmCapAudio,       A_Law_48,                   eG711Alaw64kCapCode,                  sizeof(g711Alaw64kCapStruct),        (DWORD)64*_K_,    NonFrameBasedFPP},
	{cmCapAudio,       U_Law_48,                   eG711Ulaw64kCapCode,                  sizeof(g711Ulaw64kCapStruct),        (DWORD)64*_K_,    NonFrameBasedFPP},
	/*10*/
	{cmCapAudio,       G722_m2,                    eG722_64kCapCode,                     sizeof(g722_64kCapStruct),           (DWORD)64*_K_,    NonFrameBasedFPP},
	{cmCapAudio,       G722_m3,                    eG722_64kCapCode,                     sizeof(g722_64kCapStruct),           (DWORD)64*_K_,    NonFrameBasedFPP},
	{cmCapAudio,       Au_32k,                     eG7221_32kCapCode,                    sizeof(g7221_32kCapStruct),          (DWORD)32*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_24k,                     eG7221_24kCapCode,                    sizeof(g7221_24kCapStruct),          (DWORD)24*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_G7221_16k,               eG7221_16kCapCode,                    sizeof(g7221_16kCapStruct),          (DWORD)16*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_Siren7_16k,              eSiren7_16kCapCode,                   sizeof(siren7_16kCapStruct),         (DWORD)16*_K_,    FrameBasedFPP},
	{cmCapAudio,       G728,                       eG728CapCode,                         sizeof(g728CapStruct),               (DWORD)16*_K_,    NonFrameBasedFPP},
	{cmCapAudio,       Au_Off_F,                   (CapEnum)MODE_OFF,                    0,                                   (DWORD)0,         NonFrameBasedFPP},
	{cmCapAudio,       Au_Siren14_24k,             eSiren14_24kCapCode,                  sizeof(siren14_24kCapStruct),        (DWORD)24*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_Siren14_32k,             eSiren14_32kCapCode,                  sizeof(siren14_32kCapStruct),        (DWORD)32*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_Siren14_48k,             eSiren14_48kCapCode,                  sizeof(siren14_48kCapStruct),        (DWORD)48*_K_,    FrameBasedFPP},
	/*20*/
	{cmCapAudio,       G7221_AnnexC_24k,           eG7221C_24kCapCode,                   sizeof(g7221C_24kCapStruct),         (DWORD)24*_K_,    FrameBasedFPP},
	{cmCapAudio,       G7221_AnnexC_32k,           eG7221C_32kCapCode,                   sizeof(g7221C_32kCapStruct),         (DWORD)32*_K_,    FrameBasedFPP},
	{cmCapAudio,       G7221_AnnexC_48k,           eG7221C_48kCapCode,                   sizeof(g7221C_48kCapStruct),         (DWORD)48*_K_,    FrameBasedFPP},
	{cmCapAudio,       NA,                         eG7221C_CapCode,                      sizeof(g7221C_CapStruct),            (DWORD)0,         FrameBasedFPP},
	{cmCapAudio,       G729_8k,                    eG729AnnexACapCode,                   sizeof(g729AnnexACapStruct),         (DWORD)8*_K_,     2*FrameBasedFPP},
	{cmCapAudio,       Au_Siren14S_48k,            eSiren14Stereo_48kCapCode,            sizeof(siren14Stereo_48kCapStruct),  (DWORD)48*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_Siren14S_56k,            eSiren14Stereo_56kCapCode,            sizeof(siren14Stereo_56kCapStruct),  (DWORD)56*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_Siren14S_64k,            eSiren14Stereo_64kCapCode,            sizeof(siren14Stereo_64kCapStruct),  (DWORD)64*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_Siren14S_96k,            eSiren14Stereo_96kCapCode,            sizeof(siren14Stereo_96kCapStruct),  (DWORD)96*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_Siren22S_128k,           eSiren22Stereo_128kCapCode,           sizeof(siren22Stereo_128kCapStruct), (DWORD)128*_K_,   FrameBasedFPP},
	/*30*/
	{cmCapAudio,       Au_Siren22S_96k,            eSiren22Stereo_96kCapCode,            sizeof(siren22Stereo_96kCapStruct),  (DWORD)96*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_Siren22S_64k,            eSiren22Stereo_64kCapCode,            sizeof(siren22Stereo_64kCapStruct),  (DWORD)64*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_Siren22_64k,             eSiren22_64kCapCode,                  sizeof(siren22_64kCapStruct),        (DWORD)64*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_Siren22_48k,             eSiren22_48kCapCode,                  sizeof(siren22_48kCapStruct),        (DWORD)48*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_Siren22_32k,             eSiren22_32kCapCode,                  sizeof(siren22_32kCapStruct),        (DWORD)32*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_SirenLPR_32k,            eSirenLPR_32kCapCode,                 sizeof(sirenLPR_CapStruct),          (DWORD)32*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_SirenLPR_48k,            eSirenLPR_48kCapCode,                 sizeof(sirenLPR_CapStruct),          (DWORD)48*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_SirenLPR_64k,            eSirenLPR_64kCapCode,                 sizeof(sirenLPR_CapStruct),          (DWORD)64*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_SirenLPRS_64k,           eSirenLPRStereo_64kCapCode,           sizeof(sirenLPR_CapStruct),          (DWORD)64*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_SirenLPRS_96k,           eSirenLPRStereo_96kCapCode,           sizeof(sirenLPR_CapStruct),          (DWORD)96*_K_,    FrameBasedFPP},
	/*40*/
	{cmCapAudio,       Au_SirenLPRS_128k,          eSirenLPRStereo_128kCapCode,          sizeof(sirenLPR_CapStruct),          (DWORD)128*_K_,   FrameBasedFPP},
	{cmCapAudio,       Au_SirenLPR_Scalable_32k,   eSirenLPR_Scalable_32kCapCode,        sizeof(sirenLPR_Scalable_CapStruct), (DWORD)32*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_SirenLPR_Scalable_48k,   eSirenLPR_Scalable_48kCapCode,        sizeof(sirenLPR_Scalable_CapStruct), (DWORD)48*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_SirenLPR_Scalable_64k,   eSirenLPR_Scalable_64kCapCode,        sizeof(sirenLPR_Scalable_CapStruct), (DWORD)64*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_SirenLPRS_Scalable_64k,  eSirenLPRStereo_Scalable_64kCapCode,  sizeof(sirenLPR_Scalable_CapStruct), (DWORD)64*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_SirenLPRS_Scalable_96k,  eSirenLPRStereo_Scalable_96kCapCode,  sizeof(sirenLPR_Scalable_CapStruct), (DWORD)96*_K_,    FrameBasedFPP},
	{cmCapAudio,       Au_SirenLPRS_Scalable_128k, eSirenLPRStereo_Scalable_128kCapCode, sizeof(sirenLPR_Scalable_CapStruct), (DWORD)128*_K_,   FrameBasedFPP},
	{cmCapAudio,       G719_32k,                   eG719_32kCapCode,                     sizeof(g719_32kCapStruct),           (DWORD)32*_K_,    FrameBasedFPP},
	{cmCapAudio,       G719_48k,                   eG719_48kCapCode,                     sizeof(g719_48kCapStruct),           (DWORD)48*_K_,    FrameBasedFPP},
	{cmCapAudio,       G719_64k,                   eG719_64kCapCode,                     sizeof(g719_64kCapStruct),           (DWORD)64*_K_,    FrameBasedFPP},
	/*50*/
	{cmCapAudio,       G719S_128k,                 eG719Stereo_128kCapCode,              sizeof(g719Stereo_128kCapStruct),    (DWORD)128*_K_,   FrameBasedFPP},
	{cmCapAudio,       G719S_96k,                  eG719Stereo_96kCapCode,               sizeof(g719Stereo_96kCapStruct),     (DWORD)96*_K_,    FrameBasedFPP},
	{cmCapAudio,       G719S_64k,                  eG719Stereo_64kCapCode,               sizeof(g719Stereo_64kCapStruct),     (DWORD)64*_K_,    FrameBasedFPP},
	{cmCapVideo,       Video_Off,                  (CapEnum)MODE_OFF,                    0,                                   (DWORD)0,         NA},
	{cmCapVideo,       H261,                       eH261CapCode,                         sizeof(h261CapStruct),               (DWORD)0,         NA},
	{cmCapVideo,       H263,                       eH263CapCode,                         sizeof(h263CapStruct),               (DWORD)0,         NA},
	{cmCapVideo,       H264,                       eH264CapCode,                         sizeof(h264CapStruct),               (DWORD)0,         NA},
	{cmCapVideo,       SVC,                        eSvcCapCode,                          sizeof(svcCapStruct),                (DWORD)0,         NA},
	{cmCapVideo,       RTV,                        eRtvCapCode,                          sizeof(rtvCapStruct),                (DWORD)0,         NA},
	{cmCapVideo,       MS_SVC,                     eMsSvcCapCode,                        sizeof(msSvcCapStruct),              (DWORD)0,         NA},
	    {cmCapVideo, VP8,            eVP8CapCode,               sizeof(vp8CapStruct)         ,(DWORD)0             ,NA}, //N.A. DEBUG VP8
	{cmCapVideo,       NA,                         eIS11172VideoCapCode,                 sizeof(IS11172VideoCapStruct),       (DWORD)0,         NA},
	/*60*/
	{cmCapVideo,       NA,                         eGenericVideoCapCode,                 sizeof(genericVideoCapStruct),       (DWORD)0,         NA},
	{cmCapData,        Mlp_46_4,                   eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)46.4*_K_,  NA},
	{cmCapData,        Mlp_40,                     eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)40.0*_K_,  NA},
	{cmCapData,        Mlp_38_4,                   eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)38.4*_K_,  NA},
	{cmCapData,        Mlp_32,                     eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)32.0*_K_,  NA},
	{cmCapData,        Mlp_30_4,                   eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)30.4*_K_,  NA},
	{cmCapData,        Mlp_24,                     eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)24.0*_K_,  NA},
	{cmCapData,        Mlp_22_4,                   eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)22.4*_K_,  NA},
	{cmCapData,        Mlp_16,                     eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)16.0*_K_,  NA},
	{cmCapData,        Mlp_14_4,                   eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)14.4*_K_,  NA},
	/*70*/
	{cmCapData,        MLP_6_4k,                   eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)6.4*_K_,   NA},
	{cmCapData,        MLP_4k,                     eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)4.0*_K_,   NA},
	{cmCapData,        MLP_Off,                    (CapEnum)MODE_OFF,                    0,                                   (DWORD)0,         NA},
	{cmCapData,        H_Mlp_Com_384,              eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)384.0*_K_, NA},
	{cmCapData,        H_Mlp_Com_320,              eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)320.0*_K_, NA},
	{cmCapData,        H_Mlp_Com_256,              eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)256.0*_K_, NA},
	{cmCapData,        H_Mlp_Com_192,              eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)192.0*_K_, NA},
	{cmCapData,        H_Mlp_Com_128,              eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)128.0*_K_, NA},
	{cmCapData,        H_Mlp_Cap_64,               eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)64.0*_K_,  NA},
	{cmCapData,        H_Mlp_Cap_62_4,             eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)62.4*_K_,  NA},
	/*80*/
	{cmCapData,        H_Mlp_Cap_14_4,             eT120DataCapCode,                     sizeof(dataCapStructBase),           (DWORD)14.4*_K_,  NA},
	{cmCapData,        H_Mlp_Off_Com,              (CapEnum)MODE_OFF,                    0,                                   (DWORD)0,         NA},
	{cmCapData,        LSD_6400,                   eAnnexQCapCode,                       sizeof(dataCapStructBase),           (DWORD)6.4*_K_,   NA},
	{cmCapData,        LSD_4800,                   eAnnexQCapCode,                       sizeof(dataCapStructBase),           (DWORD)4.8*_K_,   NA},
	{cmCapData,        LSD_Off,                    (CapEnum)MODE_OFF,                    0,                                   (DWORD)0,         NA},
	{cmCapData,        LSD_6400,                   eRvFeccCapCode,                       sizeof(dataCapStructBase),           (DWORD)6.4*_K_,   NA},
	{cmCapData,        LSD_4800,                   eRvFeccCapCode,                       sizeof(dataCapStructBase),           (DWORD)4.8*_K_,   NA},
	{cmCapData,        LSD_Off,                    (CapEnum)MODE_OFF,                    0,                                   (DWORD)0,         NA},
	{cmCapAudio,       Au_AAC_LD,                  eAAC_LDCapCode,                       sizeof(AAC_LDCapStruct),             (DWORD)256*_K_,   FrameBasedFPP},
	{cmCapAudio,       Au_iLBC_13k,                eiLBC_13kCapCode,                     sizeof(iLBC_13kCapStruct),           (DWORD)16*_K_,    FrameBasedFPP},
	/*90*/
	{cmCapAudio,       Au_iLBC_15k,                eiLBC_15kCapCode,                     sizeof(iLBC_15kCapStruct),           (DWORD)16*_K_,    FrameBasedFPP},

		// Opus
		{cmCapAudio, Au_Opus_64k, eOpus_CapCode,       sizeof(opus_CapStruct),(DWORD)(64*_K_)     ,FrameBasedFPP},
		{cmCapAudio, Au_OpusStereo_128k, eOpusStereo_CapCode,       sizeof(opusStereo_CapStruct),(DWORD)(128*_K_)     ,FrameBasedFPP},
	{cmCapAudio,       Au_G722_Stereo_128,         eG722Stereo_128kCapCode,              sizeof(g722Stereo_128kCapStruct),    (DWORD)128*_K_,   FrameBasedFPP},
	{cmCapBfcp,        NA,                         eBFCPCapCode,                         0,                                   (DWORD)0,         NA},
	{cmCapEmpty,       NA,                         eUnknownAlgorithemCapCode,            0,                                   (DWORD)0,         NA}
};


const int MAX_COM_MODES = ARRAYSIZE(CComModeInfo::g_comModeTbl);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Constructors:

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Looks for the specific h320 mode in the table and sets its index.
//---------------------------------------------------------------------------------------------------

CComModeInfo::CComModeInfo(WORD h320ModeType, WORD startCapType)
{
	m_index = MAX_COM_MODES - 1;

	if (h320ModeType != (WORD)NA)
	{
		for (int i = startCapType; i < MAX_COM_MODES; i++)
		{
			if (h320ModeType == g_comModeTbl[i].h320ModeType)
			{
				m_index = i;
				break;
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Looks for the specific h323 mode with a specific rate in the table
//             and sets its index.
//---------------------------------------------------------------------------------------------------
CComModeInfo::CComModeInfo(CapEnum h323ModeType, DWORD bitRate)
{
	m_index = MAX_COM_MODES - 1;

	if (h323ModeType < eUnknownAlgorithemCapCode)
	{
		for (int i = 0; i < MAX_COM_MODES; i++)
		{
			if (h323ModeType == g_comModeTbl[i].h323ModeType) {
				if (bitRate == 0)
				{
					m_index = i;
					break;
				}
				else if (bitRate == g_comModeTbl[i].bitRate)
				{
					m_index = i;
					break;
				}
			}
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Operations:

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if mode is off.
//---------------------------------------------------------------------------------------------------

DWORD CComModeInfo::GetBitRate(BYTE* pData) const
{
	DWORD rate = g_comModeTbl[m_index].bitRate;

	if (pData) {
		switch (GetH323ModeType()) {
			case eOpus_CapCode:
			case eOpusStereo_CapCode:
			{
				opus_CapStruct *pOpus = (opus_CapStruct *)pData;
				rate = pOpus->maxAverageBitrate;
				PTRACE2INT(eLevelInfoNormal, "CComModeInfo::GetBitRate with data for opus, rate = ", rate);
			}
			break;

			default:
				rate = g_comModeTbl[m_index].bitRate;
		}
	}

	return rate;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if mode is off.
//---------------------------------------------------------------------------------------------------

BYTE CComModeInfo::IsModeOff() const
{
    BYTE bOff = FALSE;

    if ((g_comModeTbl[m_index].h320ModeType == Au_Neutral)||
		(g_comModeTbl[m_index].h320ModeType == Au_Off_U)||
        (g_comModeTbl[m_index].h320ModeType == Au_Off_F)||
		(g_comModeTbl[m_index].h320ModeType == Video_Off)||
        (g_comModeTbl[m_index].h320ModeType == MLP_Off)||
		(g_comModeTbl[m_index].h320ModeType == H_Mlp_Off_Com) ||
		(g_comModeTbl[m_index].h320ModeType == LSD_Off))
        bOff = TRUE;
    else
        bOff = FALSE;

    return bOff;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Finds the next h320 mode with the same h323 mode and set its index
//             When not found sets the last index in the table.
//---------------------------------------------------------------------------------------------------

void CComModeInfo::SetNextH320ModeWithSameH323ModeType()
{
	m_index = MAX_COM_MODES - 1;

	for (int i = m_index+1; i < MAX_COM_MODES; i++)
	{
		if (g_comModeTbl[i].h323ModeType == g_comModeTbl[m_index].h323ModeType)
		{
			m_index = i;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Finds the previous h320 mode with the same h323 mode and set its index
//             When not found sets the last index in the table.
//---------------------------------------------------------------------------------------------------
void CComModeInfo::SetPrevH320ModeWithSameH323ModeType()
{
	m_index = MAX_COM_MODES - 1;

	for (int i = m_index-1; i >= 0; i--)
	{
		if (g_comModeTbl[i].h323ModeType == g_comModeTbl[m_index].h323ModeType)
		{
			m_index = i;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Returns the name of the cap code.
//---------------------------------------------------------------------------------------------------

const char* CComModeInfo::GetH323CapName() const
{
	return CapEnumToString((CapEnum)g_comModeTbl[m_index].h323ModeType);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if it's the mode type
//---------------------------------------------------------------------------------------------------

BYTE CComModeInfo::IsModeType(CapEnum h323ModeType) const
{
	return (g_comModeTbl[m_index].h323ModeType == h323ModeType);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if it's the mode type
//---------------------------------------------------------------------------------------------------

BYTE CComModeInfo::IsModeType(WORD h320ModeType) const
{
	return (g_comModeTbl[m_index].h320ModeType == h320ModeType);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if it's the cap set type
//---------------------------------------------------------------------------------------------------
BYTE CComModeInfo::IsType(cmCapDataType type) const
{
	return (g_comModeTbl[m_index].type == type);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the frame per packet of alg
//---------------------------------------------------------------------------------------------------
int  CComModeInfo::GetFramePerPacket()
{
//	UpdateFromSystemCfg();
	return g_comModeTbl[m_index].framePerPacket;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//In data fecc in 320 it is LSD in 323 it could be two types annexQ and RadVision so I need to have the ability to
//change the type.
void CComModeInfo::ChangeType(CapEnum h323ModeType,int startIndex)
{
	for (int i = startIndex; i < MAX_COM_MODES; i++)
	{
		if (h323ModeType == g_comModeTbl[i].h323ModeType)
		{
			m_index = i;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CComModeInfo::GetCodecNumberOfChannels()
{
	return ::GetCodecNumberOfChannels(GetH323ModeType());
}
