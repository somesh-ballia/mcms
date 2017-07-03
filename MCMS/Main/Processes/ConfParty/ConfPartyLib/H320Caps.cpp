#include <sstream>
#include <iomanip>
#include "Macros.h"
#include "Segment.h"
#include "CDRUtils.h"
#include "H221.h"
#include "H263.h"
#include "H320Caps.h"
#include "NStream.h"
#include "Trace.h"
#include "VideoDefines.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "ConfPartyGlobals.h"
#include "IpCommon.h"
#include "SysConfig.h"
#include "ProcessBase.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////
//                        CCapH320
////////////////////////////////////////////////////////////////////////////
CCapH320::CCapH320()
{
	m_tic            = 1;
	m_chair          = 1;
	m_mih            = 0;
	m_mvc            = 0;
	m_pCapPP         = NULL;
	m_illegalCap     = 0;
	m_h239ControlCap = 0;
}

//--------------------------------------------------------------------------
CCapH320::CCapH320(const CCapH320& other) : CPObject(other)
{
	m_chair          = other.m_chair;
	m_tic            = other.m_tic;
	m_mih            = other.m_mih;
	m_mvc            = other.m_mvc;
	m_h239ControlCap = other.m_h239ControlCap;
	m_audioCap       = other.m_audioCap;
	m_rateCap        = other.m_rateCap;
	m_dataCap        = other.m_dataCap;
	m_encrypCap      = other.m_encrypCap;
	m_contentCap     = other.m_contentCap;
	m_videoCap       = other.m_videoCap;
	m_nsCap          = other.m_nsCap;
	m_illegalCap     = 0;
	m_pCapPP         = NULL;

	if (other.m_pCapPP)
		m_pCapPP = new CCapPP(*(other.m_pCapPP));
}

//--------------------------------------------------------------------------
CCapH320::~CCapH320()
{
	PDELETE(m_pCapPP);
}

//--------------------------------------------------------------------------
void CCapH320::Dump() const
{
	std::ostringstream dumpstring;
	Dump(dumpstring);
	TRACEINTO << dumpstring.str().c_str();
}

//--------------------------------------------------------------------------
void CCapH320::Dump(std::ostream& msg) const
{
	msg.setf(ios::left, ios::adjustfield);
	msg.setf(ios::showbase);

	msg << "==================    nCCapH320::Dump    ==================\n"
	    << setw(20) << "this"             << (hex) << (DWORD) this     << "\n"
	    << setw(20) << "m_h239ControlCap" << (hex) << m_h239ControlCap << "\n"
	    << setw(20) << "m_nsCap"          << (hex) << (DWORD)&m_nsCap  << "\n";

	m_audioCap.Dump(msg);
	m_rateCap.Dump(msg);

	m_videoCap.Dump(msg);
	m_dataCap.Dump(msg);

	m_nsCap.Dump(msg);
	m_encrypCap.Dump(msg);

	if (m_pCapPP)
		m_pCapPP->Dump(msg);

	m_contentCap.Dump(msg);

	msg << "\n===============  CCapH320::Dump Finished!!!  ===============\n" << endl;
}

//--------------------------------------------------------------------------
void CCapH320::CreateDefault()
{
	m_audioCap.AddAudioAlg(e_G722_48);
	m_rateCap.SetXferCap5H0();
	m_rateCap.AddXferCap(e_Xfer_Cap_NoRestrict); // restrict is not supported in RMX
	m_encrypCap.CreateDefault();
	m_audioCap.SetAudioDefaultAlg();
	m_videoCap.CreateDefault();
}

//--------------------------------------------------------------------------
void CCapH320::Create(CComMode& scm, CCapH263* pCapH263, CCapH264* pCapH264, BYTE H263withH264)
{
	WORD T120Rate = 0;

	// set xfer rate cap
	SetXferRateFromScm(scm);
	// set restrict type
	if (scm.GetOtherRestrictMode() == Restrict)
		SetXferCap(e_Xfer_Cap_Restrict);

	// set video cap
	switch (scm.m_vidMode.GetVidMode())
	{
		case H261:
		{
			// no auto vid support yet
			PASSERT(scm.m_vidMode.GetVidFormat() == 0);

			if (scm.m_vidMode.GetVidFormat() == V_Cif)
			{
				DBGPASSERT(scm.m_vidMode.GetCifMpi() == 0);           // no auto vid mpi support yet
				DBGPASSERT(scm.m_vidMode.GetQcifMpi() == 0);          // no auto vid mpi support yet
				// V_Qcif
				m_videoCap.SetVideoCapCif(scm.m_vidMode.GetCifMpi(), scm.m_vidMode.GetQcifMpi());
			}
			else                                                    // Qcif
			{
				PASSERT(scm.m_vidMode.GetQcifMpi() == 0);             // no auto vid mpi support yet
				// V_Qcif
				m_videoCap.SetVideoCapQcif(scm.m_vidMode.GetQcifMpi());
			}
			break;
		}

		case H264:
		{
			// Caps can have only one set for level
			if (!CPObject::IsValidPObjectPtr(pCapH264) ||
			    ((CPObject::IsValidPObjectPtr(pCapH264)) && (!pCapH264->GetNumberOfH264Sets())))
			{
				CSegment seg;
				DWORD    h264len = 0;
				seg << scm.m_vidMode.GetH264VidMode().GetProfileValue();
				seg << scm.m_vidMode.GetH264VidMode().GetLevelValue();

				h264len = h264len+2;

				WORD customMaxMBPS     = scm.m_vidMode.GetH264VidMode().GetMaxMBPSasCustomWord();
				WORD customMaxFS       = scm.m_vidMode.GetH264VidMode().GetMaxFSasCustomWord();
				WORD customMaxDPB      = scm.m_vidMode.GetH264VidMode().GetMaxDPBasCustomWord();
				WORD customMaxBRandCPB = scm.m_vidMode.GetH264VidMode().GetMaxBRasCustomWord();
				WORD customSAR         = scm.m_vidMode.GetH264VidMode().GetSAR();

				if (customMaxMBPS != 0xFFFF)
				{
					seg << (BYTE)CUSTOM_MAX_MBPS_CODE;
					seg << CCDRUtils::CalcH264CustomParamFirstByte(customMaxMBPS);
					if (CCDRUtils::IsH264TwoBytesNumber(customMaxMBPS))
					{
						seg << CCDRUtils::CalcH264CustomParamSecondByte(customMaxMBPS);
						h264len = h264len+3;
					}
					else h264len = h264len+2;
				}

				if (customMaxFS != 0xFFFF)
				{
					seg << (BYTE)CUSTOM_MAX_FS_CODE;
					seg << CCDRUtils::CalcH264CustomParamFirstByte(customMaxFS);
					if (CCDRUtils::IsH264TwoBytesNumber(customMaxFS))
					{
						seg << CCDRUtils::CalcH264CustomParamSecondByte(customMaxFS);
						h264len = h264len+3;
					}
					else h264len = h264len+2;
				}

				if (customMaxDPB != 0xFFFF)
				{
					seg << (BYTE)CUSTOM_MAX_DPB_CODE;
					seg << CCDRUtils::CalcH264CustomParamFirstByte(customMaxDPB);
					if (CCDRUtils::IsH264TwoBytesNumber(customMaxDPB))
					{
						seg << CCDRUtils::CalcH264CustomParamSecondByte(customMaxDPB);
						h264len = h264len+3;
					}
					else h264len = h264len+2;
				}

				if (customMaxBRandCPB != 0xFFFF)
				{
					seg << (BYTE)CUSTOM_MAX_BR_CODE;
					seg << CCDRUtils::CalcH264CustomParamFirstByte(customMaxBRandCPB);
					if (CCDRUtils::IsH264TwoBytesNumber(customMaxBRandCPB))
					{
						seg << CCDRUtils::CalcH264CustomParamSecondByte(customMaxBRandCPB);
						h264len = h264len+3;
					}
					else h264len = h264len+2;
				}

				seg << (BYTE)CUSTOM_SAR_CODE;
				seg << CCDRUtils::CalcH264CustomParamFirstByte(customSAR);
				if (CCDRUtils::IsH264TwoBytesNumber(customSAR))
				{
					seg << CCDRUtils::CalcH264CustomParamSecondByte(customSAR);
					h264len = h264len+3;
				}
				else
					h264len = h264len+2;

				// Romem - 25.5.11: High profile compatibility with HDX ISDN
				BOOL bCfgEnableHighfProfileInIsdn = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_HIGH_PROFILE_WITH_ISDN);
				BOOL bEnableHighfProfileInIsdn    = FALSE;
				if (IsFeatureSupportedBySystem(eFeatureH264HighProfile) && bCfgEnableHighfProfileInIsdn)
				{
					bEnableHighfProfileInIsdn = 1;
				}

				if (bEnableHighfProfileInIsdn)
				{
					RemoveH264Caps(); // Remove all old CCapSetH264.
				}

				SetH264Caps(seg, h264len);

				// ****** To insert other sets too + custom parameters if needed *******************/
			}

			if (!H263withH264)
				break;
		}

		case H263:
		{
			if (!CPObject::IsValidPObjectPtr(pCapH263) ||
			    ((CPObject::IsValidPObjectPtr(pCapH263)) && (!pCapH263->GetNumberOfH263Sets())))
			{
				CSegment seg;
				BYTE     h263BaseCaps = 1;
				BYTE     temp         = 0;
				// What we have to do in order to create h263 caps:
				// 1. generate baseline h263 capability byte from h263 format & mpi of the
				// communication mode (scm)
				// 2. create h263 caps from single baseline h263 capability byte
				// 3. add all lower H263 formats with the same MPI
				// 1 //
				h263BaseCaps  = h263BaseCaps << 7;
				temp          = (BYTE)scm.m_vidMode.GetH263Mpi();
				temp          = temp << 3;
				h263BaseCaps |= temp;
				temp          = 0;
				temp          = (BYTE)scm.m_vidMode.GetVidFormat();
				temp          = temp << 1;
				h263BaseCaps |= temp;
				seg <<  h263BaseCaps;
				// 2 //
				SetH263Caps(seg, 2);
				// 3 //
				for (int format = (scm.m_vidMode.GetVidFormat() - 1); format >= H263_QCIF_SQCIF; format--)
					m_videoCap.SetOneH263Cap(format, scm.m_vidMode.GetH263Mpi());
			}
			else
			{
				(*m_videoCap.GetCapH263()) = *pCapH263;

				// Declare on H263 (2000) capabilities.
				if (pCapH263->IsSecondAdditionalCap() || pCapH263->IsAdditionalCapOrCustomFormats())
					m_videoCap.SetVideoH263Cap(H263_2000);
			}

			// PeopleContent ns-cap
			if (scm.GetNsContentMode())
				AddPeopleContent();
			else
				RemovePeopleContents();

			// VisualConcert PC ns-cap
			if (scm.GetNsModeVisualConcertPC())
				AddVisualConcertPC();
			else
				RemoveVisualConcertPC();

			// VisualConcert FX ns-cap
			if (scm.GetNsModeVisualConcertFX())
				AddVisualConcertFX();
			else
				RemoveVisualConcertFX();

			break;
		}

		case H26L:
		{
			m_nsCap.AddNSH26LVideoCap(scm.m_vidMode.GetOctet0(), scm.m_vidMode.GetOctet1());
			break;
		}
	} // switch

/*
    Set up the audio algorithm according to the table:
  +-----------------+-------------------------+------------------+
  | Call rate <= 96 | 128 >= Call rate <= 192 |  Call rate >=256 |
  +-----------------+-------------------------+------------------+
  | G.728           | G722.1 C 32K            | G722.1 C 48K     |
  | G722.1 16K      | G722.1 C 24K            | G722.1 C 48K     |
  | G722.1 C 24K    | Siren14 32K             | G722.1 C 32K     |
  | Siren14 24K     | Siren14 24K             | G722.1 C 24K     |
  | G722 48K        | G722.1 32K              | Siren14 48K      |
  | G722 56K        | G722.1 24K              | Siren14 32K      |
  | G722 64K        | G.728                   | Siren14 24K      |
  | G711 56K        | G722 48K                | G722.1 32K       |
  | G711 64K        | G722 56K                | G722.1 24K       |
  |                 | G722 64K                | G722.1 16K       |
  |                 | G711 56K                | G722 48K         |
  |                 | G711 64K                | G722 56K         |
  |                 |                         | G722 64K         |
  |                 |                         | G728             |
  |                 |                         | G711 56K         |
  |                 |                         | G711 64K         |
  +-----------------+-------------------------+------------------+
*/

	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();

	BOOL sysISDN_G711_ALAW    = FALSE;
	BOOL sysISDN_G711_ULAW    = FALSE;
	BOOL sysISDN_G722_64k     = FALSE;
	BOOL sysISDN_G722_56k     = FALSE;
	BOOL sysISDN_G722_48k     = FALSE;
	BOOL sysISDN_G722_1_32k   = FALSE;
	BOOL sysISDN_G722_1_24k   = FALSE;
	BOOL sysISDN_G722_1_C_48k = FALSE;
	BOOL sysISDN_G722_1_C_32k = FALSE;
	BOOL sysISDN_G722_1_C_24k = FALSE;
	BOOL sysISDN_SIREN_14_48k = FALSE;
	BOOL sysISDN_SIREN_14_32k = FALSE;
	BOOL sysISDN_SIREN_14_24k = FALSE;
	BOOL sysISDN_G728         = FALSE;


	pSysConfig->GetBOOLDataByKey("ISDN_G711_ALAW", sysISDN_G711_ALAW);
	pSysConfig->GetBOOLDataByKey("ISDN_G711_ULAW", sysISDN_G711_ULAW);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_64k", sysISDN_G722_64k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_56k", sysISDN_G722_56k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_48k", sysISDN_G722_48k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_1_32k", sysISDN_G722_1_32k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_1_24k", sysISDN_G722_1_24k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_1_C_48k", sysISDN_G722_1_C_48k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_1_C_32k", sysISDN_G722_1_C_32k);
	pSysConfig->GetBOOLDataByKey("ISDN_G722_1_C_24k", sysISDN_G722_1_C_24k);
	pSysConfig->GetBOOLDataByKey("ISDN_SIREN_14_48k", sysISDN_SIREN_14_48k);
	pSysConfig->GetBOOLDataByKey("ISDN_SIREN_14_32k", sysISDN_SIREN_14_32k);
	pSysConfig->GetBOOLDataByKey("ISDN_SIREN_14_24k", sysISDN_SIREN_14_24k);
	pSysConfig->GetBOOLDataByKey("G728_ISDN", sysISDN_G728);

	if (sysISDN_G711_ALAW)
		m_audioCap.AddAudioAlg(e_A_Law);

	if (sysISDN_G711_ULAW)
		m_audioCap.AddAudioAlg(e_U_Law);

	if (sysISDN_G722_64k)
		m_audioCap.AddAudioAlg(e_G722_64);

	if (sysISDN_G722_48k)
		m_audioCap.AddAudioAlg(e_G722_48); // e_G722_48 & e_G722_56

	if (sysISDN_SIREN_14_24k)
		m_nsCap.AddNScapSiren1424();

	if (sysISDN_G722_1_C_24k)
		m_audioCap.AddAudioAlg(e_G722_1_Annex_C_24);

	if (sysISDN_G728)
		m_audioCap.AddAudioAlg(e_Au_16k);

	// G722.1 16K ????
	WORD xfer_B0_chnls = scm.GetNumB0Chnl();
	if (xfer_B0_chnls >= 2) // >= Xfer_128
	{
		if (sysISDN_G722_1_C_24k)
			m_audioCap.AddAudioAlg(e_G722_1_24);

		if (sysISDN_G722_1_C_32k)
			m_audioCap.AddAudioAlg(e_G722_1_32);

		if (sysISDN_SIREN_14_32k)
			m_nsCap.AddNScapSiren1432();

		if (sysISDN_G722_1_C_32k)
			m_audioCap.AddAudioAlg(e_G722_1_Annex_C_32);

		// Remove G722.1 16K if exists!
	}

	if (xfer_B0_chnls >= 4) // >= Xfer_256
	{
		if (sysISDN_G722_1_C_48k)
			m_audioCap.AddAudioAlg(e_G722_1_Annex_C_48);

		if (sysISDN_SIREN_14_48k)
			m_nsCap.AddNScapSiren1448();
	}

	if (scm.GetOtherEncrypMode() == Encryp_On)
	{
		m_encrypCap.SetEncrypCap(Encryp_Cap);
		CreateLocalCapECS();
	}
	else                               // If in scm Encryption is set to OFF - Remove Encryption Caps - Bug#17918 - Talya
	{
		m_encrypCap.RemoveEncrypCap();
	}

	// set data cap
	SetLsdCap(scm.m_lsdMlpMode.GetLsdMode());

	// In addition to the above.( There is a duplicity in a few cases)
	// We insert LSD_4800 and/or  LSD_6400 to local caps
	switch (scm.m_lsdMlpMode.GetlsdCap())
	{
		case LSD_NONE:
		{
			break; // No LSD capabilities are declared.
		}
		case LSD_DYNAMIC:
		{
			SetDataCap(Dxfer_Cap_4800);
			SetDataCap(Dxfer_Cap_6400);
			break;
		}
		case LSD_DYNAMIC_6400_ONLY:
		case LSD_FIXED: // Only LSD 6400 is enable at fix mode.
		{
			SetDataCap(Dxfer_Cap_6400);
			break;
		}
		case LSD_FIXED_4800_GW: // Bug fix 22200 in GateWay conference we enable LSD 4800 at fix mode
		case LSD_DYNAMIC_4800_ONLY:
		{
			SetDataCap(Dxfer_Cap_4800);
			RemoveDataCap(Dxfer_Cap_6400);
			break;
		}
		default:
		{
			if (scm.m_lsdMlpMode.GetlsdCap())
				PASSERT(scm.m_lsdMlpMode.GetlsdCap());
			else
				PASSERT(101);

			break;
		}
	} // switch

	if (scm.m_lsdMlpMode.GetMlpMode() != MLP_Off)
		T120Rate = scm.m_lsdMlpMode.GetMlpMode();

	if (scm.m_hsdHmlpMode.GetHmlpMode() != H_Mlp_Off_Com)
		T120Rate = scm.m_hsdHmlpMode.GetHmlpMode();

	// create Capabilities Set
	if (T120Rate)
		SetT120Cap(T120Rate);

	// set mbe cap
	m_videoCap.SetVideoMBECap(Mbe_Cap);

	// ContentMode ON and Dou ON can be only in H323 Duo conferences.
	// In this case we'll not add EPC caps
	if (scm.IsContentOn())
	{
		CCapH239 H239Cap;
		H239Cap.CreateCapH239(scm.GetXferMode(), scm.IsFreeVideoRate(), scm.m_lsdMlpMode.GetlsdCap() != LSD_NONE, scm.m_contentMode.GetContentLevel());
		SetOnlyExtendedVideoCaps(&H239Cap);
		m_h239ControlCap = 1;
	}
}

//--------------------------------------------------------------------------
void CCapH320::HandleBas(BYTE bas, CSegment& seg)
{
	char s1[16];
	switch (bas)
	{
		case AUDRATECAPATTR | Neutral:
			m_audioCap.AddAudioAlg(e_Neutral);
			break;
		case AUDRATECAPATTR | A_Law:
			m_audioCap.AddAudioAlg(e_A_Law);
			break;
		case AUDRATECAPATTR | U_Law:
			m_audioCap.AddAudioAlg(e_U_Law);
			break;
		case AUDRATECAPATTR | G722_64:
			m_audioCap.AddAudioAlg(e_G722_64);
			break;
		case AUDRATECAPATTR | G722_48:
			m_audioCap.AddAudioAlg(e_G722_48);
			break;
		case AUDRATECAPATTR | Au_16k:
			m_audioCap.AddAudioAlg(e_Au_16k);
			break;
		case AUDRATECAPATTR | Sm_comp:
			m_rateCap.AddXferCap(e_Xfer_Cap_Sm_comp);
			break;
		case AUDRATECAPATTR | Xfer_Cap_Restrict:
			m_rateCap.AddXferCap(e_Xfer_Cap_Restrict);
			break;
		case AUDRATECAPATTR | Xfer_Cap_6B_H0_Comp:
			m_rateCap.AddXferCap(e_Xfer_Cap_6B_H0_Comp);
			break;
		case AUDRATECAPATTR | Xfer_Cap_B:
			m_rateCap.SetXferCapB();
			break;
		case AUDRATECAPATTR | Xfer_Cap_2B:
			m_rateCap.SetXferCap2B();
			break;
		case AUDRATECAPATTR | Xfer_Cap_3B:
			m_rateCap.SetXferCap3B();
			break;
		case AUDRATECAPATTR | Xfer_Cap_4B:
			m_rateCap.SetXferCap4B();
			break;
		case AUDRATECAPATTR | Xfer_Cap_5B:
			m_rateCap.SetXferCap5B();
			break;
		case AUDRATECAPATTR | Xfer_Cap_6B:
			m_rateCap.SetXferCap6B();
			break;
		case AUDRATECAPATTR | Xfer_Cap_H0:
			m_rateCap.SetXferCapH0();
			break;
		case AUDRATECAPATTR | Xfer_Cap_2H0:
			m_rateCap.SetXferCap2H0();
			break;
		case AUDRATECAPATTR | Xfer_Cap_3H0:
			m_rateCap.SetXferCap3H0();
			break;
		case AUDRATECAPATTR | Xfer_Cap_4H0:
			m_rateCap.SetXferCap4H0();
			break;
		case AUDRATECAPATTR | Xfer_Cap_5H0:
			m_rateCap.SetXferCap5H0();
			break;
		case AUDRATECAPATTR | Xfer_Cap_128:
			m_rateCap.AddXferCap(e_Xfer_Cap_128);
			break;
		case AUDRATECAPATTR | Xfer_Cap_192:
			m_rateCap.AddXferCap(e_Xfer_Cap_192);
			break;
		case AUDRATECAPATTR | Xfer_Cap_256:
			m_rateCap.AddXferCap(e_Xfer_Cap_256);
			break;
		case AUDRATECAPATTR | Xfer_Cap_320:
			m_rateCap.AddXferCap(e_Xfer_Cap_320);
			break;
		case AUDRATECAPATTR | Xfer_Cap_512:
			m_rateCap.AddXferCap(e_Xfer_Cap_512);
			break;
		case AUDRATECAPATTR | Xfer_Cap_768:
			m_rateCap.AddXferCap(e_Xfer_Cap_768);
			break;
		case AUDRATECAPATTR | Xfer_Cap_1152:
			m_rateCap.AddXferCap(e_Xfer_Cap_1152);
			break;
		case AUDRATECAPATTR | Xfer_Cap_1472:
			m_rateCap.AddXferCap(e_Xfer_Cap_1472);
			break;
		case AUDRATECAPATTR | Xfer_Cap_H11:
			m_rateCap.AddXferCap(e_Xfer_Cap_H11);
			break;
		case AUDRATECAPATTR | Xfer_Cap_H12:
			m_rateCap.AddXferCap(e_Xfer_Cap_H12);
			break;
		case DATAVIDCAPATTR | V_Qcif:
			m_videoCap.SetVideoCapQcif(seg);
			break;
		case DATAVIDCAPATTR | V_Cif:
			m_videoCap.SetVideoCapCif(seg);
			break;
		case DATAVIDCAPATTR | Mlp_Set_1:
		{
			m_dataCap.SetDataCap(Mlp_Set_1);
			m_dataCap.SetDataCap(Dxfer_Cap_Mlp_6_4k);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_40);
			break;
		}
		case DATAVIDCAPATTR | Mlp_Set_2:
		{
			m_dataCap.SetDataCap(Mlp_Set_1);
			m_dataCap.SetDataCap(Mlp_Set_2);
			m_dataCap.SetDataCap(Dxfer_Cap_Mlp_6_4k);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_38_4);
			m_dataCap.SetMlpBas(Mlp_Cap_40);
			m_dataCap.SetMlpBas(Mlp_Cap_46_4);
			m_dataCap.SetMlpBas(Mlp_Cap_62_4);
			break;
		}
		case DATAVIDCAPATTR | Ter2_Var_Lsd:
		case DATAVIDCAPATTR | Dxfer_Cap_300:
		case DATAVIDCAPATTR | Dxfer_Cap_1200:
		case DATAVIDCAPATTR | Dxfer_Cap_4800:
		case DATAVIDCAPATTR | Dxfer_Cap_6400:
		case DATAVIDCAPATTR | Dxfer_Cap_8000:
		case DATAVIDCAPATTR | Dxfer_Cap_9600:
		case DATAVIDCAPATTR | Dxfer_Cap_14400:
		case DATAVIDCAPATTR | Dxfer_Cap_16k:

		case DATAVIDCAPATTR | Dxfer_Cap_24k:
		case DATAVIDCAPATTR | Dxfer_Cap_32k:
		case DATAVIDCAPATTR | Dxfer_Cap_40k:
		case DATAVIDCAPATTR | Dxfer_Cap_48k:
		case DATAVIDCAPATTR | Dxfer_Cap_56k:
		case DATAVIDCAPATTR | Dxfer_Cap_62_4k:
		case DATAVIDCAPATTR | Dxfer_Cap_64k:
		case DATAVIDCAPATTR | Dxfer_Cap_Mlp_4k:
		case DATAVIDCAPATTR | Dxfer_Cap_Mlp_6_4k:
		case DATAVIDCAPATTR | Var_Mlp:
		{
			m_dataCap.SetDataCap(bas ^ DATAVIDCAPATTR);
			break;
		}
		case DATAVIDCAPATTR | Mbe_Cap:
			m_videoCap.SetVideoMBECap(Mbe_Cap);
			break;
		case DATAVIDCAPATTR | Encryp_Cap:
			m_encrypCap.SetEncrypCap(Encryp_Cap);
			break;
		case DATAVIDCAPATTR | H263_2000:
			m_videoCap.SetVideoH263Cap(H263_2000);
			break;
		case OTHERCAPATTR | Restrict_L:
			m_rateCap.AddXferCap(e_Xfer_Cap_Restrict_L);
			break;
		case OTHERCAPATTR | Restrict_P:
			m_rateCap.AddXferCap(e_Xfer_Cap_Restrict_P);
			break;
		case OTHERCAPATTR | NoRestrict:
			m_rateCap.AddXferCap(e_Xfer_Cap_NoRestrict);
			break;
		case OTHERCAPATTR | G722_1_32:
			m_audioCap.AddAudioAlg(e_G722_1_32);
			break;
		case OTHERCAPATTR | G722_1_24:
			m_audioCap.AddAudioAlg(e_G722_1_24);
			break;
		case OTHERCAPATTR | G722_1_Annex_C_48:
			m_audioCap.AddAudioAlg(e_G722_1_Annex_C_48);
			break;
		case OTHERCAPATTR | G722_1_Annex_C_32:
			m_audioCap.AddAudioAlg(e_G722_1_Annex_C_32);
			break;
		case OTHERCAPATTR | G722_1_Annex_C_24:
			m_audioCap.AddAudioAlg(e_G722_1_Annex_C_24);
			break;
		case ESCAPECAPATTR | Hsd_Esc:
		{
			BYTE hsdBas, capattr;
			seg >> hsdBas;
			capattr = hsdBas & 0xE0;
			switch (capattr)
			{
				case HSDHMPLCAPATTR:
				{
					hsdBas ^= HSDHMPLCAPATTR;
					m_dataCap.SetHsdHmlpBas((WORD)hsdBas);
					break;
				}
				case MLPCAPATTR:
				{
					hsdBas ^= MLPCAPATTR;
					m_dataCap.SetMlpBas((WORD)hsdBas);
					break;
				}
				default:
				{
					ON(m_illegalCap);
					sprintf(s1, " %02x ", capattr);
					PTRACE2(eLevelInfoNormal, "CCapH320::HandleBas : \' UNKNOWN HSD CAP\':", s1);
					break;
				}
			} // switch

			break;
		}
		case ESCAPECAPATTR | H230_Esc:
		{
			BYTE h230Bas;
			seg >> h230Bas;
			switch (h230Bas)
			{
				case (Attr010 | CIC):
				{             // chair cntl cap
					m_chair = 1;
					break;
				}
				case (Attr001 | MIH):
				{             // MIH cap
					m_mih = 1;
					break;
				}
				case (Attr010 | MVC):
				{
					PTRACE(eLevelInfoNormal, "CCapH320::HandleBas : MVC from remote!!");
					m_mvc = YES;
					break;
				}
				case (Attr001 | TIC):
				{
					m_tic = 1;
					break;
				}
				case (Attr101 | h239ControlCapability):
				{
					PTRACE(eLevelInfoNormal, "CCapH320::HandleBas : h239ControlCapability from remote!!");
					m_h239ControlCap = 1;
					break;
				}
				default:
				{
					ON(m_illegalCap);
					sprintf(s1, " %02x ", h230Bas);
					PTRACE2(eLevelInfoNormal, "CCapH320::HandleBas : \' UNKNOWN H230 CAP\': ", s1);
					break;
				}
			} // switch

			break;
		}
		case ESCAPECAPATTR | Data_Apps_Esc:
		{
			PTRACE(eLevelInfoNormal, "CCapH320::HandleBas : T120 Capabilities");
			BYTE t120_local_cap;
			seg >> t120_local_cap;
			break;
		}
		case ESCAPECAPATTR | Start_Mbe:
		{
			PTRACE(eLevelInfoNormal, "CCapH320::HandleBas : found Mbe Capabilities");
			MbeCommandParser(seg);
			break;
		}
		case ESCAPECAPATTR | Aggrgat_Esc:
		{
			// In this case we need to take additional parameter from Table_A.6 of standards
			// Because we're not use this table yet, we're take this param-r and drop him out :
			BYTE param;
			seg >> param;
			break;
		}
		// Now we get non-standard capabilities in an special
		// indication REMOTE_NS_CAP
		case (ESCAPECAPATTR | Ns_Cap):
		{
			PTRACE(eLevelInfoNormal, "CCapH320::HandleBas : found NS (non-standard) Capabilities");
			HandleNSCap(seg);
			break;
		}
		default:
		{
			ON(m_illegalCap);
			sprintf(s1, " %02x ", bas);
			PTRACE2(eLevelInfoNormal, "CCapH320::HandleBas : \' UNKNOWN BAS\':", s1);
			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
void CCapH320::HandleNSCap(CSegment& seg)
{
	BYTE      msgLen, country1, country2, manufCode1, manufCode2;
	CSegment* pCopySeg = new CSegment;
	*pCopySeg = seg;

	*pCopySeg >> msgLen >> country1 >> country2 >> manufCode1 >> manufCode2;

	// Public Polycom Caps
	if ((country1 == (BYTE)COUNTRY_CODE_USA_BYTE_1) &&
	    (country2 == (BYTE)COUNTRY_CODE_USA_BYTE_2) &&
	    (manufCode1 == (BYTE)MANUFACT_CODE_PUBLIC_PP_BYTE_1) &&
	    (manufCode2 == (BYTE)MANUFACT_CODE_PUBLIC_PP_BYTE_2))
	{
		TRACEINTO << "CCapH320::HandleNSCap - HandlePPCap";
		HandlePPCap(seg);
	}
	else
	{
		TRACEINTO << "CCapH320::HandleNSCap - AddNSItem";
		m_nsCap.AddNSItem(seg);
		if (IsPeopleContent())
			m_mvc = YES;
	}

	POBJDELETE(pCopySeg);
}

//--------------------------------------------------------------------------
void CCapH320::HandlePPCap(CSegment& seg)
{
	if (m_pCapPP)
		POBJDELETE(m_pCapPP);

	m_pCapPP = new CCapPP;
	m_pCapPP->DeSerialize(SERIALEMBD, seg);
}

//--------------------------------------------------------------------------
void CCapH320::CreatePPCap(const CCapPP* pCapPP)
{
	if (m_pCapPP)
		POBJDELETE(m_pCapPP);

	m_pCapPP = new CCapPP(*pCapPP);
}

//--------------------------------------------------------------------------
void CCapH320::MbeCommandParser(CSegment& seg)
{
	// can handle now just ONE value from
	// table_2 H230 : <H.262/H.263/H.264>
	BYTE len = 0;
	BYTE table2_H230Value;
	seg >> len;
	seg >> table2_H230Value;

	switch (table2_H230Value)
	{
		case H262_H263:
		{
			SetH263Caps(seg, len);
			break;
		}
		case H264_Mbe: // H264_Mbe
		{
			// Romem - 25.5.11: High profile compatibility with HDX ISDN
			if (m_videoCap.IsH264())
			{
				SetH264Caps(seg, len-1); // table2_H230Value has exit
				PTRACE(eLevelInfoNormal, "CCapH320::MbeCommandParser : 2nd and more H264 cap set !!!");
			}
			else
			{
				SetH264Caps(seg, len-1);       // table2_H230Value has exit
				PTRACE(eLevelInfoNormal, "CCapH320::MbeCommandParser : 1st H264 cap set !!!");
			}

			break;
		}
		case h239ExtendedVideoCapability:
		{
			SetH239ExtendedVideoCaps(seg, len-1);
			PTRACE(eLevelInfoNormal, "CCapH320::MbeCommandParser : Received from remote h239ExtendedVideoCapability - ERROR! Should not be sent in cap set!!!");
			break;
		}
		case AMC_cap:
		{
			SetAMCCaps(seg, len);
			PTRACE(eLevelInfoNormal, "CCapH320::MbeCommandParser : Received from remote AMC_Cap");
			break;
		}
		case H239_messsage:
		{
			PTRACE(eLevelInfoNormal, "CCapH320::MbeCommandParser : Received from remote H239_messsage");
			break;
		}
		case AMC_CI:
		{
			PTRACE(eLevelInfoNormal, "CCapH320::MbeCommandParser : Received from remote AMC_CI");
			break;
		}

		default:
		{ // if it not H263, H264 or H239 we can't process it, just obtain bytes from segment
			BYTE dump;
			for (BYTE i = 0; i < len-1; i++)
				seg >> dump;

			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
void CCapH320::RemoveNSCap()
{
	CCapNS emptyNsCap;
	m_nsCap = emptyNsCap;

	if (m_pCapPP)
		POBJDELETE(m_pCapPP);
}

//--------------------------------------------------------------------------
WORD CCapH320::GetH26LMpi(WORD h320FormatType) const
{
	if (h320FormatType == H26L_CIF)
		return GetH26LCifMpiForOneVideoStream();
	else if (h320FormatType == H26L_CIF_4)
		return GetH26L4CifMpiForOneVideoStream();
	else
		return 0;
}

//--------------------------------------------------------------------------
WORD CCapH320::IsH26LMpi(WORD h320FormatType) const
{
	if (h320FormatType == H26L_CIF)
		return GetH26LCifMpiForOneVideoStream();
	else if (h320FormatType == H26L_CIF_4)
		return GetH26L4CifMpiForOneVideoStream();
	else
		return 0;
}

//--------------------------------------------------------------------------
void CCapH320::Serialize(WORD format, CSegment& H221StringSeg)
{
	BYTE seriableVal = 0;

	switch (format)
	{
		case SERIALEMBD:
		{
			// serialize audio
			m_audioCap.Serialize(format, H221StringSeg);

			// add here G.722.1/24 and G.722.1/32  // done while Serialize of CAudioCap

			// serialize transfer rate
			m_rateCap.Serialize(format, H221StringSeg);

			// serialize restrict (done while Serialize of CRateCap)

			// serialize video
			m_videoCap.Serialize(format, H221StringSeg);

			// serialize Encryption capability
			m_encrypCap.Serialize(format, H221StringSeg);

			// serialize mbe capability
			seriableVal =  Mbe_Cap | DATAVIDCAPATTR;
			H221StringSeg << seriableVal;

			// serialize data HSD / Hmlp, LSD, MLP
			m_dataCap.Serialize(format, H221StringSeg);


			if (m_videoCap.IsVideoCapSupported(H263_2000))
			{
				seriableVal = H263_2000 | DATAVIDCAPATTR;
				H221StringSeg << seriableVal;
			}

			// serialize Restrict_L and NoRestrict - done while Serialize of CRateCap

			// serialize G723_1 and G729 - done while Serialize of CAudioCap

			// serialize data MLP (done while Serialize of CDataCap)

			// serialize H263 and H264 - done while Serialize of CVideoCap

			if (m_chair)
				H221StringSeg << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(CIC | Attr010);

			if (m_mih)
				H221StringSeg << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(MIH | Attr001);

			if (m_h239ControlCap)
			{
				H221StringSeg << (BYTE)(H230_Esc | ESCAPECAPATTR) << (BYTE)(h239ControlCapability | Attr101);
			}

			m_contentCap.Serialize(format, H221StringSeg);

			// add here Non-Standard caps
			m_nsCap.Serialize(format, H221StringSeg);

			if (m_pCapPP)
				m_pCapPP->Serialize(format, H221StringSeg);

			break;
		}

		case  NATIVE: {
			H221StringSeg << m_chair << m_tic << m_mih << m_mvc << m_h239ControlCap;

			m_audioCap.Serialize(format, H221StringSeg);
			m_rateCap.Serialize(format, H221StringSeg);
			m_dataCap.Serialize(format, H221StringSeg);
			m_videoCap.Serialize(format, H221StringSeg);
			m_contentCap.Serialize(format, H221StringSeg);
			m_encrypCap.Serialize(format, H221StringSeg);
			m_nsCap.Serialize(format, H221StringSeg);

			if (m_pCapPP)
			{
				H221StringSeg << (BYTE)1; // is PP Caps
				m_pCapPP->Serialize(format, H221StringSeg);
			}
			else
				H221StringSeg << (BYTE)0; // is PP Caps

			break;
		}
		default:
			break;
	} // switch
}

//--------------------------------------------------------------------------
void CCapH320::DeSerialize(WORD format, CSegment& H221StringSeg)
{
	BYTE     bas;
	CSegment copy_of_seg(H221StringSeg);
	CSegment seg(H221StringSeg);

	switch (format)
	{
		case SERIALEMBD:
		{
			PTRACE(eLevelInfoNormal, "CCapH320::HandleBas : Remove H264 caps before parsing segment");
			// Romem - 25.5.11: High profile compatability with HDX ISDN
			RemoveH264Caps();
			while (!H221StringSeg.EndOfSegment())
			{
				H221StringSeg >> bas;
				HandleBas(bas, H221StringSeg);
			}

			PTRACE2INT(eLevelInfoNormal, "CCapH320::HandleBas : Num of H264 caps after parsing segment", m_videoCap.IsH264());

			if (m_illegalCap)
			{
				const WORD msgBufSize = 1024;
				char*      msgStr     = new char[msgBufSize];
				memset(msgStr, '\0', msgBufSize);
				int        count = 0, byte_count = 0;

				while (!seg.EndOfSegment())
				{
					char temp[16];
					BYTE dump = '\0';
					seg >> dump;
					memset(temp, '\0', 16);
					sprintf(temp, " %02x", dump);
					strcat(msgStr, temp);
					byte_count += 3;
					count++;
					if (!(count%8))
					{
						strcat(msgStr, "\n");
						byte_count += 1;
					}

					if (!(count%32))
					{
						strcat(msgStr, "\n");
						byte_count += 1;
					}

					if (byte_count >= msgBufSize)              // buffer length
					{
						PTRACE(eLevelInfoNormal, "***  WARNING - SEGMENT IS TOO LONG (size > 1024)  ***\n");
						break;
					}
				}

				msgStr[msgBufSize-1] = '\0';

				PTRACE2(eLevelInfoNormal, "***  UNKNOWN BAS CAPABILITIES  ***\n", msgStr);

				delete [] msgStr;
			}

			m_audioCap.SetAudioDefaultAlg();
			break;
		}

		case NATIVE:
		{
			H221StringSeg >> m_chair >> m_tic >> m_mih >> m_mvc >> m_h239ControlCap;

			m_audioCap.DeSerialize(format, H221StringSeg);
			m_rateCap.DeSerialize(format, H221StringSeg);
			m_dataCap.DeSerialize(format, H221StringSeg);
			m_videoCap.DeSerialize(format, H221StringSeg);
			m_contentCap.DeSerialize(format, H221StringSeg);
			m_encrypCap.DeSerialize(format, H221StringSeg);
			m_nsCap.DeSerialize(format, H221StringSeg);
			BYTE isPPCap;
			H221StringSeg >> isPPCap;
			if (isPPCap)
			{
				if (!m_pCapPP)
					m_pCapPP = new CCapPP;

				m_pCapPP->DeSerialize(format, H221StringSeg);
			}
			break;
		}

		default:
			break;
	} // switch
}

//--------------------------------------------------------------------------
///////////////////////////// audio operations /////////////////////////////
//--------------------------------------------------------------------------
void CCapH320::SetVoiceOnlyCap(WORD audio_cap)
{
	// Create voice-only (telephone) caps
	// 1.  Audio caps {A_Law or U_Law}
	// 2.  Xfer  caps 1B

	m_audioCap.ResetAudioCap();
	m_rateCap.ResetXferCap();
	m_dataCap.ResetDataCaps();
	m_videoCap.ResetVideoCaps();

	m_tic            = 0;
	m_chair          = 0;
	m_mih            = 0;
	m_mvc            = 0;
	m_h239ControlCap = 0;
	OFF(m_illegalCap);

	EAudioCapAlgorithm e_alg = (A_Law == audio_cap) ? e_A_Law : e_U_Law;
	SetAudioCap(e_alg);
	SetXferCap(e_Xfer_Cap_B);
}

//--------------------------------------------------------------------------
/////////////////////////// transfer operations ////////////////////////////
//--------------------------------------------------------------------------
void CCapH320::SetXferRateFromScm(const CComMode& scm)
{
	switch (scm.m_xferMode.GetXferMode())
	{
		case Xfer_64   : { SetXferCap(e_Xfer_Cap_B);  break; }
		case Xfer_2x64 : { SetXferCap(e_Xfer_Cap_2B);  break; }
		case Xfer_3x64 : { SetXferCap(e_Xfer_Cap_3B);  break; }
		case Xfer_4x64 : { SetXferCap(e_Xfer_Cap_4B);  break; }
		case Xfer_5x64 : { SetXferCap(e_Xfer_Cap_5B);  break; }
		case Xfer_6x64 : { SetXferCap(e_Xfer_Cap_6B);  break; }
		case Xfer_384  : { SetXferCap(e_Xfer_Cap_H0);  break; }
		case Xfer_2x384: { SetXferCap(e_Xfer_Cap_2H0);  break; }
		case Xfer_3x384: { SetXferCap(e_Xfer_Cap_3H0);  break; }
		case Xfer_4x384: { SetXferCap(e_Xfer_Cap_4H0);  break; }
		case Xfer_5x384: { SetXferCap(e_Xfer_Cap_5H0);  break; }
		case Xfer_128  : { SetXferCap(e_Xfer_Cap_128);  break; }
		case Xfer_192  : { SetXferCap(e_Xfer_Cap_192);  break; }
		case Xfer_256  : { SetXferCap(e_Xfer_Cap_256);  break; }
		case Xfer_320  : { SetXferCap(e_Xfer_Cap_320);  break; }
		case Xfer_512  : { SetXferCap(e_Xfer_Cap_512);  break; }
		case Xfer_768  : { SetXferCap(e_Xfer_Cap_768);  break; }
		case Xfer_1152 : { SetXferCap(e_Xfer_Cap_1152);  break; }
		case Xfer_1472 : { SetXferCap(e_Xfer_Cap_1472);  break; }
		case Xfer_1536 : { SetXferCap(e_Xfer_Cap_H11);  break; }
		case Xfer_1920 : { SetXferCap(e_Xfer_Cap_H12);  break; }
		default:
		{
			ALLOCBUFFER(Mess, ONE_LINE_BUFFER_LEN);
			sprintf(Mess, "CCapH320::SetXferRateFromScm - BAD ARGUMENT <%d>", scm.m_xferMode.GetXferMode());
			PTRACE(eLevelError, Mess);
			DEALLOCBUFFER(Mess);
			if (scm.m_xferMode.GetXferMode())
				PASSERT(scm.m_xferMode.GetXferMode());
			else
				PASSERT(101);

			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
//////////////////////////// video  operations /////////////////////////////
//--------------------------------------------------------------------------
// CIF mode set routine
//--------------------------------------------------------------------------
void CCapH320::SetH261Caps(WORD cap, WORD mpiCif, WORD mpiQcif)
{
	CSegment mpiSeg;
	BYTE     bas;

	bas  = (BYTE)cap;
	bas |= DATAVIDCAPATTR;

	mpiQcif |= DATAVIDCAPATTR;
	mpiSeg << (BYTE)mpiQcif;

	if (mpiCif)
	{
		mpiCif |= DATAVIDCAPATTR;
		mpiSeg << (BYTE)mpiCif;
	}

	HandleBas(bas, mpiSeg);
}

//--------------------------------------------------------------------------
//////////////////////////// data  operations //////////////////////////////
//--------------------------------------------------------------------------
void CCapH320::SetMlpCap(WORD cap)
{
	CSegment dummySeg;
	BYTE     bas;

	bas  = (BYTE)cap;
	bas |= DATAVIDCAPATTR;

	HandleBas(bas, dummySeg);
}

//--------------------------------------------------------------------------
void CCapH320::SetT120Cap(WORD T120Val)
{
	switch (T120Val)
	{
		// mlp Caps
		case MLP_4k:
		{
			SetDataCap(Dxfer_Cap_Mlp_4k);
			break;
		}
		case MLP_6_4k:
		{
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case Mlp_14_4:
		{
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case Mlp_16:
		{
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case Mlp_22_4:
		{
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case Mlp_24:
		{
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case Mlp_30_4:
		{
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case Mlp_32:
		{
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case Mlp_38_4:
		{
			m_dataCap.SetMlpBas(Mlp_Cap_38_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case Mlp_40:
		{
			m_dataCap.SetMlpBas(Mlp_Cap_40);
			m_dataCap.SetMlpBas(Mlp_Cap_38_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case Mlp_46_4:
		{
			m_dataCap.SetMlpBas(Mlp_Cap_46_4);
			m_dataCap.SetMlpBas(Mlp_Cap_40);
			m_dataCap.SetMlpBas(Mlp_Cap_38_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case Data_var_MLP:
		{
			SetDataCap(Var_Mlp);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		// hmlp Caps
		case Var_H_Mlp_Com_R:
		{
			m_dataCap.SetHsdHmlpBas(Var_H_Mlp_Cap_R);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case H_Mlp_Com_14_4:
		{
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_14_4);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case H_Mlp_Com_384:
		{
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_384);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_320);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_256);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_192);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_128);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_64);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_62_4);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_14_4);
			m_dataCap.SetMlpBas(Mlp_Cap_46_4);
			m_dataCap.SetMlpBas(Mlp_Cap_40);
			m_dataCap.SetMlpBas(Mlp_Cap_38_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case H_Mlp_Com_320:
		{
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_320);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_256);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_192);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_128);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_64);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_62_4);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_14_4);
			m_dataCap.SetMlpBas(Mlp_Cap_46_4);
			m_dataCap.SetMlpBas(Mlp_Cap_40);
			m_dataCap.SetMlpBas(Mlp_Cap_38_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case H_Mlp_Com_256:
		{
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_256);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_192);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_128);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_64);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_62_4);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_14_4);
			m_dataCap.SetMlpBas(Mlp_Cap_46_4);
			m_dataCap.SetMlpBas(Mlp_Cap_40);
			m_dataCap.SetMlpBas(Mlp_Cap_38_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case H_Mlp_Com_192:
		{
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_192);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_128);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_64);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_62_4);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_14_4);
			m_dataCap.SetMlpBas(Mlp_Cap_46_4);
			m_dataCap.SetMlpBas(Mlp_Cap_40);
			m_dataCap.SetMlpBas(Mlp_Cap_38_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case H_Mlp_Com_128:
		{
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_128);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_64);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_62_4);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_14_4);
			m_dataCap.SetMlpBas(Mlp_Cap_46_4);
			m_dataCap.SetMlpBas(Mlp_Cap_40);
			m_dataCap.SetMlpBas(Mlp_Cap_38_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case H_Mlp_Com_64:
		{
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_64);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_62_4);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_14_4);
			m_dataCap.SetMlpBas(Mlp_Cap_46_4);
			m_dataCap.SetMlpBas(Mlp_Cap_40);
			m_dataCap.SetMlpBas(Mlp_Cap_38_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			m_dataCap.SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		case H_Mlp_Com_62_4:
		{
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_64);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_62_4);
			m_dataCap.SetHsdHmlpBas(H_Mlp_Cap_14_4);
			m_dataCap.SetMlpBas(Mlp_Cap_46_4);
			m_dataCap.SetMlpBas(Mlp_Cap_40);
			m_dataCap.SetMlpBas(Mlp_Cap_38_4);
			m_dataCap.SetMlpBas(Mlp_Cap_32);
			m_dataCap.SetMlpBas(Mlp_Cap_30_4);
			m_dataCap.SetMlpBas(Mlp_Cap_24);
			m_dataCap.SetMlpBas(Mlp_Cap_22_4);
			m_dataCap.SetMlpBas(Mlp_Cap_16);
			m_dataCap.SetMlpBas(Mlp_Cap_14_4);
			SetDataCap(Dxfer_Cap_Mlp_6_4k);
			break;
		}
		default:
		{
			if (T120Val)
				PASSERT(T120Val);
			else
				PASSERT(101);

			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
void CCapH320::SetLsdCap(WORD lsdMode)
{
	// SetDataVidCap(Dxfer_Cap_6400); //removed -  do not support HSD/LSD
	switch (lsdMode)
	{
		case LSD_Off  : { break; }
		case LSD_300  : { SetDataCap(Dxfer_Cap_300); break; }
		case LSD_1200 : { SetDataCap(Dxfer_Cap_1200); break; }
		case LSD_4800 : { SetDataCap(Dxfer_Cap_4800); break; }
		case LSD_6400 : { SetDataCap(Dxfer_Cap_6400); break; }
		case LSD_8000 : { SetDataCap(Dxfer_Cap_8000); break; }
		case LSD_9600 : { SetDataCap(Dxfer_Cap_9600); break; }
		case LSD_14400: { SetDataCap(Dxfer_Cap_14400); break; }
		case LSD_16k  : { SetDataCap(Dxfer_Cap_16k); break; }
		case LSD_24k  : { SetDataCap(Dxfer_Cap_24k); break; }
		case LSD_32k  : { SetDataCap(Dxfer_Cap_32k); break; }
		case LSD_40k  : { SetDataCap(Dxfer_Cap_40k); break; }
		case LSD_48k  : { SetDataCap(Dxfer_Cap_48k); break; }
		case LSD_56k  : { SetDataCap(Dxfer_Cap_56k); break; }
		case LSD_62_4k: { SetDataCap(Dxfer_Cap_62_4k); break; }
		case LSD_64k  : { SetDataCap(Dxfer_Cap_64k); break; }
		case 255      : { SetDataCap(Dxfer_Cap_6400); break; }
		default:
		{
			if (lsdMode)
				PASSERT(lsdMode);
			else
				PASSERT(101);

			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
///////////////////////////// H239 operations //////////////////////////////
//--------------------------------------------------------------------------
void CCapH320::SetH239ExtendedVideoCaps(CSegment& seg, BYTE len)
{
	m_h239ControlCap = 1;
	m_contentCap.SetH239ExtendedVideoCaps(seg, len);
}

//--------------------------------------------------------------------------
void CCapH320::SetOnlyExtendedVideoCaps(const CCapH239* pCapH239)
{
	m_contentCap.SetOnlyExtendedVideoCaps(pCapH239);
}

//--------------------------------------------------------------------------
BYTE CCapH320::IsH239Cap() const
{
	if (!m_contentCap.IsCapH239() || !m_h239ControlCap)
		return FALSE;

	return TRUE;
}

//--------------------------------------------------------------------------
void CCapH320::RemoveH239Caps()
{
	m_h239ControlCap = 0;
	m_contentCap.RemoveH239Caps();
}

//--------------------------------------------------------------------------
//////////////////////////// H221 string operations ////////////////////////
//--------------------------------------------------------------------------
BYTE* CCapH320::GetH221str(int& len)
{
	CH221strCapDrv* H221string = new CH221strCapDrv;

	UpdateH221string(*H221string);

	len = H221string->GetLen();
	return H221string->GetPtr();
}

//--------------------------------------------------------------------------
void CCapH320::UpdateH221string(CH221strCapDrv& str)
{
	CSegment seg;
	// serialize caps in H221 format on seg
	Serialize(SERIALEMBD, seg);
	// deserialize the H221 string from the seg
	str.DeSerialize(SERIALEMBD, seg);
}

//--------------------------------------------------------------------------
CCapH320& CCapH320::operator=(CCapH320& other)
{
	if (this == &other)
		return *this;

	m_audioCap       = other.m_audioCap;
	m_rateCap        = other.m_rateCap;
	m_dataCap        = other.m_dataCap;
	m_videoCap       = other.m_videoCap;
	m_nsCap          = other.m_nsCap;
	m_contentCap     = other.m_contentCap;
	m_encrypCap      = other.m_encrypCap;
	m_chair          = other.m_chair;
	m_tic            = other.m_tic;
	m_mih            = other.m_mih;
	m_mvc            = other.m_mvc;
	m_h239ControlCap = other.m_h239ControlCap;

	POBJDELETE(m_pCapPP);

	if (other.m_pCapPP)
		m_pCapPP  = new CCapPP(*(other.m_pCapPP));

	return *this;
}

//--------------------------------------------------------------------------
eVideoPartyType CCapH320::GetCPVideoPartyTypeAccordingToCapabilities(BYTE isH2634Cif15Supported)
{
	return m_videoCap.GetCPVideoPartyTypeAccordingToCapabilities(isH2634Cif15Supported);
}

//--------------------------------------------------------------------------
eVideoPartyType CCapH320::GetCPVideoPartyTypeAccordingToLocalCapabilities(BYTE isH2634Cif15Supported)
{
	return m_videoCap.GetCPVideoPartyTypeAccordingToLocalCapabilities(isH2634Cif15Supported);
}

//--------------------------------------------------------------------------
BOOL CCapH320::IsReducedcapbilities()
{
	if (IsCapableOfVideo())  // reduced - no video caps
		return FALSE;

	if (OnAudioCap(e_G722_64) || OnAudioCap(e_G722_56) || OnAudioCap(e_G722_48) || OnAudioCap(e_G722_1_32) || OnAudioCap(e_Au_16k)) // g711 only
		return FALSE;

	return TRUE;
}
