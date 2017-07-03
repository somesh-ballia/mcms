#include "PrettyTable.h"
#include "BaseModeResources.h"
#include "Trace.h"
#include "TraceStream.h"
#include "ObjString.h"
#include "SystemResources.h"
#include "HelperFuncs.h"
#include "ResourceManager.h"
#include "ResRsrcCalculator.h"
#include "SysConfigKeys.h"

#undef min
#include "PrettyTable.h"

////////////////////////////////////////////////////////////////////////////
//                        CBaseModeResources
////////////////////////////////////////////////////////////////////////////
CBaseModeResources::CBaseModeResources(eSystemCardsMode cardsMode)
{
	memset(m_PromillesPerCard, 0, sizeof(m_PromillesPerCard));

	m_BusyInReconfigureUnits = FALSE;
	m_totalPromillesAccordingToCards = 0;
	m_logicalHD720WeightCopParty = 0;

	eProductType prodType = CProcessBase::GetProcess()->GetProductType();

	FTRACEINTO << "CardsMode:" << GetSystemCardsModeStr(cardsMode) << ", ProductType:" << ProductTypeToString(prodType);

	switch (cardsMode)
	{
		case eSystemCardsMode_breeze:
		{
			switch (prodType)
			{
				case eProductTypeSoftMCU:
				case eProductTypeGesher:
				case eProductTypeEdgeAxis:
					InitSoftMPMXWeightsAndValues();
					break;
				case eProductTypeSoftMCUMfw:
					InitSoftMFW_WeightsAndValues();
					break;
				default:
					InitBreezeWeightsAndValues();
					break;
			}
			break;
		}
		case eSystemCardsMode_mpmrx:
		case eSystemCardsMode_mixed_mode:
		{
			switch (prodType)
			{
				case eProductTypeNinja:
					InitSoftNinjaWeightsAndValues();
					break;
				case eProductTypeCallGeneratorSoftMCU:
					InitSoftCGWeightsAndValues();
					break;
				default:
					InitMpmRxWeightsAndValues();
					break;
			}
			break;
		}

		default:
			FPASSERTMSG(cardsMode, "Illegal cards mode in CBaseModeResources::CBaseModeResources");
			InitBreezeWeightsAndValues();
			break;
	} // switch

	PrintToTracePartyLogicalWeight("CBaseModeResources constructor");

	memset(m_numDonglePartiesSVC, 0, NUM_OF_CONF_MODE_TYPES * NUM_OF_PARTY_RESOURCE_TYPES);
}

//--------------------------------------------------------------------------
CBaseModeResources::~CBaseModeResources()
{
}

//--------------------------------------------------------------------------
CBaseModeResources::CBaseModeResources(const CBaseModeResources& other)
{
	*this = other;
}

//--------------------------------------------------------------------------
const CBaseModeResources& CBaseModeResources::operator=(const CBaseModeResources& other)
{
	memcpy(m_PromillesPerCard, other.m_PromillesPerCard, sizeof(m_PromillesPerCard));
	memcpy(m_LogicalHD720WeightAvcParty, other.m_LogicalHD720WeightAvcParty, sizeof(m_LogicalHD720WeightAvcParty));
	memcpy(m_logicalHD720WeightSvcParty, other.m_logicalHD720WeightSvcParty, sizeof(m_logicalHD720WeightSvcParty));
	memcpy(m_numDonglePartiesSVC, other.m_numDonglePartiesSVC, NUM_OF_CONF_MODE_TYPES * NUM_OF_PARTY_RESOURCE_TYPES);

	m_BusyInReconfigureUnits         = other.m_BusyInReconfigureUnits;
	m_totalPromillesAccordingToCards = other.m_totalPromillesAccordingToCards;
	m_ART_PROMILLES                  = other.m_ART_PROMILLES;
	m_VID_TOTAL_HD720_PROMILLES      = other.m_VID_TOTAL_HD720_PROMILLES;
	m_logicalHD720WeightCopParty     = other.m_logicalHD720WeightCopParty;

	return *this;
}

//--------------------------------------------------------------------------
void CBaseModeResources::InitBreezeWeightsAndValues()
{
	// If the product is RMX 1500Q with license of 25cif / 7hd720 ports
	WORD is1500QRatios = CResRsrcCalculator::IsRMX1500QRatios();

	if (is1500QRatios)
	{
		FTRACEINTO << "RMX1500Q";

		// "logical restrictions": on one card: 90 audio, 25 cif, 14 SD30, 7 HD720, 3 HD1080
		InitPortWeightsTo1500Q();
	}
	else
	{
		FTRACEINTO << "MPMX";

		BOOL isTrafficShapingEnabled     = FALSE;
		BOOL bLimitCifSdPortsPerMpmxCard = FALSE;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_RTP_TRAFFIC_SHAPING, isTrafficShapingEnabled);
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_LIMIT_CIF_SD_PORTS_PER_MPMX_CARD, bLimitCifSdPortsPerMpmxCard);

		float fCifNonMixWeight = 0;
		float fSDNonMixWeight  = 0;

		if (bLimitCifSdPortsPerMpmxCard)
		{
			fCifNonMixWeight = PORT_WEIGHT_CIF_SD30_LIMITED_MPM_X;
			fSDNonMixWeight  = PORT_WEIGHT_CIF_SD30_LIMITED_MPM_X;
		}
		else if (isTrafficShapingEnabled)
		{
			fCifNonMixWeight = PORT_WEIGHT_CIF_MPM_X_TRAFFIC_SHAPING;
			fSDNonMixWeight  = PORT_WEIGHT_SD30_MPM_X_TRAFFIC_SHAPING;
		}
		else
		{
			fCifNonMixWeight = PORT_WEIGHT_CIF_MPM_X;
			fSDNonMixWeight  = PORT_WEIGHT_SD30_MPM_X;
		}

		// "logical restrictions": one one card: 360 audio, 90 cif, 60 SD30, 30 HD720, 15 HD1080
		m_LogicalHD720WeightAvcParty[eNonMix][e_Audio]      = PORT_WEIGHT_AUDIO_MPM_X;
		m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]        = fCifNonMixWeight;
		m_LogicalHD720WeightAvcParty[eNonMix][e_SD30]       = fSDNonMixWeight;
		m_LogicalHD720WeightAvcParty[eNonMix][e_HD720]      = PORT_WEIGHT_HD720_MPM_X;
		m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p30]  = PORT_WEIGHT_HD1080_MPM_X;
		m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p60]  = PORT_WEIGHT_HD1080_60_MPM_X;

		m_LogicalHD720WeightAvcParty[eMix][e_Audio]         = PORT_WEIGHT_AUDIO_MPM_X;
		m_LogicalHD720WeightAvcParty[eMix][e_Cif]           = PORT_WEIGHT_CIF_MIXED_MPM_X_HW_LIMIT;
		m_LogicalHD720WeightAvcParty[eMix][e_SD30]          = PORT_WEIGHT_SD30_MIXED_MPM_X_HW_LIMIT;
		m_LogicalHD720WeightAvcParty[eMix][e_HD720]         = PORT_WEIGHT_HD720_MIXED_MPM_X_HW_LIMIT;
		m_LogicalHD720WeightAvcParty[eMix][e_HD1080p30]     = PORT_WEIGHT_HD1080_MIXED_MPM_X_HW_LIMIT;
		m_LogicalHD720WeightAvcParty[eMix][e_HD1080p60]     = PORT_WEIGHT_HD1080_60_MIXED_MPM_X_HW_LIMIT;

		m_logicalHD720WeightSvcParty[eNonMix][e_Audio]     = PORT_WEIGHT_SVC_MPM_X;
		m_logicalHD720WeightSvcParty[eNonMix][e_Cif]       = PORT_WEIGHT_SVC_MPM_X;
		m_logicalHD720WeightSvcParty[eNonMix][e_SD30]      = PORT_WEIGHT_SVC_MPM_X;
		m_logicalHD720WeightSvcParty[eNonMix][e_HD720]     = PORT_WEIGHT_SVC_MPM_X;
		m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p30] = PORT_WEIGHT_SVC_MPM_X;
		m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p60] = PORT_WEIGHT_SVC_MPM_X;

		m_logicalHD720WeightSvcParty[eMix][e_Audio]        = PORT_WEIGHT_SVC_MIXED_MPM_X_HW_LIMIT;
		m_logicalHD720WeightSvcParty[eMix][e_Cif]          = PORT_WEIGHT_SVC_MIXED_MPM_X_HW_LIMIT;
		m_logicalHD720WeightSvcParty[eMix][e_SD30]         = PORT_WEIGHT_SVC_MIXED_MPM_X_HW_LIMIT;
		m_logicalHD720WeightSvcParty[eMix][e_HD720]        = PORT_WEIGHT_SVC_MIXED_MPM_X_HW_LIMIT;
		m_logicalHD720WeightSvcParty[eMix][e_HD1080p30]    = PORT_WEIGHT_SVC_MIXED_MPM_X_HW_LIMIT;
		m_logicalHD720WeightSvcParty[eMix][e_HD1080p60]    = PORT_WEIGHT_SVC_MIXED_MPM_X_HW_LIMIT;

	}

	// for COP
	m_logicalHD720WeightCopParty = PORT_WEIGHT_COP_MPM_X;
	m_ART_PROMILLES                = CHelperFuncs::GetArtPromilsBreeze();
	m_VID_TOTAL_HD720_PROMILLES    = VID_TOTAL_HD720_30FS_PROMILLES_BREEZE;
}

//--------------------------------------------------------------------------
void CBaseModeResources::InitPortWeightsTo1500Q()
{
	for (int ci = 0; ci < NUM_OF_CONF_MODE_TYPES; ++ci)
	{
		m_LogicalHD720WeightAvcParty[ci][e_Audio]      = PORT_WEIGHT_AUDIO_1500Q;
		m_LogicalHD720WeightAvcParty[ci][e_Cif]        = PORT_WEIGHT_CIF_1500Q;
		m_LogicalHD720WeightAvcParty[ci][e_SD30]       = PORT_WEIGHT_SD30_1500Q;
		m_LogicalHD720WeightAvcParty[ci][e_HD720]      = PORT_WEIGHT_HD720_1500Q;
		m_LogicalHD720WeightAvcParty[ci][e_HD1080p30]  = PORT_WEIGHT_HD1080_1500Q;
		m_LogicalHD720WeightAvcParty[ci][e_HD1080p60]  = PORT_WEIGHT_HD1080_1500Q; //Tsahi - change this if 1080p60 supported on 1500Q

		m_logicalHD720WeightSvcParty[ci][e_Audio]     = PORT_WEIGHT_SVC_1500Q;
		m_logicalHD720WeightSvcParty[ci][e_Cif]       = PORT_WEIGHT_SVC_1500Q;
		m_logicalHD720WeightSvcParty[ci][e_SD30]      = PORT_WEIGHT_SVC_1500Q;
		m_logicalHD720WeightSvcParty[ci][e_HD720]     = PORT_WEIGHT_SVC_1500Q;
		m_logicalHD720WeightSvcParty[ci][e_HD1080p30] = PORT_WEIGHT_SVC_1500Q;
		m_logicalHD720WeightSvcParty[ci][e_HD1080p60] = PORT_WEIGHT_SVC_1500Q;
	}
}

//--------------------------------------------------------------------------
void CBaseModeResources::InitSoftMPMXWeightsAndValues()
{
	FTRACEINTO << "Soft_MPMX";

	for (int i = 0; i < NUM_OF_CONF_MODE_TYPES; ++i)
	{
		m_LogicalHD720WeightAvcParty[i][e_Audio]      = PORT_WEIGHT_AUDIO_SOFT_MPM_X_A_LA_CART;
		m_LogicalHD720WeightAvcParty[i][e_Cif]        = PORT_WEIGHT_CIF_SOFT_MPM_X;
		m_LogicalHD720WeightAvcParty[i][e_SD30]       = PORT_WEIGHT_SD30_SOFT_MPM_X;
		m_LogicalHD720WeightAvcParty[i][e_HD720]      = PORT_WEIGHT_HD720_SOFT_MPM_X;
		m_LogicalHD720WeightAvcParty[i][e_HD1080p30]  = PORT_WEIGHT_HD1080_SOFT_MPM_X;
		m_LogicalHD720WeightAvcParty[i][e_HD1080p60]  = PORT_WEIGHT_HD1080_P60_SOFT_MPM_X;

		m_logicalHD720WeightSvcParty[i][e_Audio]     = PORT_WEIGHT_SVC_SOFT_MPM_X_A_LA_CART;
		m_logicalHD720WeightSvcParty[i][e_Cif]       = PORT_WEIGHT_SVC_SOFT_MPM_X_A_LA_CART;
		m_logicalHD720WeightSvcParty[i][e_SD30]      = PORT_WEIGHT_SVC_SOFT_MPM_X_A_LA_CART;
		m_logicalHD720WeightSvcParty[i][e_HD720]     = PORT_WEIGHT_SVC_SOFT_MPM_X_A_LA_CART;
		m_logicalHD720WeightSvcParty[i][e_HD1080p30] = PORT_WEIGHT_SVC_SOFT_MPM_X_A_LA_CART;
		m_logicalHD720WeightSvcParty[i][e_HD1080p60] = PORT_WEIGHT_SVC_SOFT_MPM_X_A_LA_CART;
	}

	m_ART_PROMILLES             = ART_PROMILLES_SOFT_MCU;
	m_VID_TOTAL_HD720_PROMILLES = VID_TOTAL_HD720_PROMILLES_SOFT_MPMX;
}

//--------------------------------------------------------------------------
void CBaseModeResources::InitSoftMFW_WeightsAndValues()
{
	FTRACEINTO << "Soft_MFW";

	InitHighCapacityAndWeightsSoftMFW();

	m_logicalHD720WeightCopParty = 0;
	m_ART_PROMILLES                = ART_PROMILLES_SOFT_MFW;
	m_VID_TOTAL_HD720_PROMILLES    = VIDEO_PROMILLES_SOFT_MFW;
}

//--------------------------------------------------------------------------
void CBaseModeResources::InitDemoCapacityAndWeightsSoftMFW()
{
	FTRACEINTO << "MFW_DEBUG: Init DEMO mcu type values";

	for (int ci=0; ci<NUM_OF_CONF_MODE_TYPES; ++ci)
	{
		m_numDonglePartiesSVC[ci][e_Audio]     = 100;
		m_numDonglePartiesSVC[ci][e_Cif]       = MAX_NUMBER_SVC_PARTICIPANTS_SOFT_DEMO;
		m_numDonglePartiesSVC[ci][e_SD30]      = 25;
		m_numDonglePartiesSVC[ci][e_HD720]     = 10;
		m_numDonglePartiesSVC[ci][e_HD1080p30] = 5;
		m_numDonglePartiesSVC[ci][e_HD1080p60] = 5;
	}
//	m_numDonglePartiesSVC[eMix][e_Audio]     = 100;
//	m_numDonglePartiesSVC[eMix][e_Cif]       = 50;
//	m_numDonglePartiesSVC[eMix][e_SD30]      = 25;
//	m_numDonglePartiesSVC[eMix][e_HD720]     = 10;
//	m_numDonglePartiesSVC[eMix][e_HD1080]    = 5;

	InitWeightsSoftMFW (MAX_NUMBER_HD_PARTICIPANTS_SOFT_MFW_8);
}

//--------------------------------------------------------------------------
void CBaseModeResources::InitLowCapacityAndWeightsSoftMFW()
{
	FTRACEINTO << "MFW_DEBUG: Init LOW mcu type values";

	for (int ci=0; ci<NUM_OF_CONF_MODE_TYPES; ++ci)
	{
		m_numDonglePartiesSVC[ci][e_Audio]     = 400;
		m_numDonglePartiesSVC[ci][e_Cif]       = MAX_NUMBER_SVC_PARTICIPANTS_SOFT_MFW_LOW;
		m_numDonglePartiesSVC[ci][e_SD30]      = 100;
		m_numDonglePartiesSVC[ci][e_HD720]     = 40;
		m_numDonglePartiesSVC[ci][e_HD1080p30] = 20;
		m_numDonglePartiesSVC[ci][e_HD1080p60] = 20;
	}
//	m_numDonglePartiesSVC[eMix][e_Audio]     = 280;
//	m_numDonglePartiesSVC[eMix][e_Cif]       = 200;
//	m_numDonglePartiesSVC[eMix][e_SD30]      = 100;
//	m_numDonglePartiesSVC[eMix][e_HD720]     = 40;
//	m_numDonglePartiesSVC[eMix][e_HD1080]    = 20;

	InitWeightsSoftMFW (MAX_NUMBER_HD_PARTICIPANTS_SOFT_MFW_16);
}

//--------------------------------------------------------------------------
void CBaseModeResources::InitHighCapacityAndWeightsSoftMFW()
{
	FTRACEINTO << "MFW_DEBUG: Init HIGH mcu type values";

	for (int ci=0; ci<NUM_OF_CONF_MODE_TYPES; ++ci)
	{
		m_numDonglePartiesSVC[ci][e_Audio]     = 2000;
		m_numDonglePartiesSVC[ci][e_Cif]       = MAX_NUMBER_SVC_PARTICIPANTS_SOFT_MFW_HIGH;
		m_numDonglePartiesSVC[ci][e_SD30]      = 500;
		m_numDonglePartiesSVC[ci][e_HD720]     = 200;
		m_numDonglePartiesSVC[ci][e_HD1080p30] = 100;
		m_numDonglePartiesSVC[ci][e_HD1080p60] = 100;
	}
//	m_numDonglePartiesSVC[eMix][e_Audio]        = 1000;
//	m_numDonglePartiesSVC[eMix][e_Cif]          = 400;
//	m_numDonglePartiesSVC[eMix][e_SD30]         = 240;
//	m_numDonglePartiesSVC[eMix][e_HD720]        = 140;
//	m_numDonglePartiesSVC[eMix][e_HD1080p30]    = 70;
//	m_numDonglePartiesSVC[eMix][e_HD1080p60]    = 70;

	InitWeightsSoftMFW (MAX_NUMBER_HD_PARTICIPANTS_SOFT_MFW);
}

//--------------------------------------------------------------------------
void CBaseModeResources::InitWeightsSoftMFW(float num_parties)
{
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		//AVC weight -  non mixed conf  - improve resource management policy, so that AVC parties will be counted as in the "SVC only" case even in mixed mode until a first SVC connects to this conference
		m_LogicalHD720WeightAvcParty[eNonMix][i] = num_parties / m_numDonglePartiesSVC[eNonMix][i];

		m_logicalHD720WeightSvcParty[eNonMix][i] = num_parties / m_numDonglePartiesSVC[eNonMix][i];  //SVC weight - SVC only conf
	}

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		m_LogicalHD720WeightAvcParty[eMix][i] = num_parties / m_numDonglePartiesSVC[eMix][i];  //AVC weight - mixed conf

		m_logicalHD720WeightSvcParty[eMix][i] = num_parties / m_numDonglePartiesSVC[eMix][i]; //SVC weight - mixed conf
	}
}

//--------------------------------------------------------------------------
void CBaseModeResources::InitMpmRxWeightsAndValues()
{
	FTRACEINTO << "MPM-Rx";

	BOOL isTrafficShapingEnabled = NO;

	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_RTP_TRAFFIC_SHAPING, isTrafficShapingEnabled);

	//"logical restrictions": one one card: 300 audio, 200 cif, 200 SD30, 100 HD720, 50 HD1080p30
	m_LogicalHD720WeightAvcParty[eNonMix][e_Audio] = PORT_WEIGHT_AUDIO_MPMRX;

	if (isTrafficShapingEnabled)
	{
		m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]  = PORT_WEIGHT_CIF_MPMRX_HW_LIMIT_TRAFFIC_SHAPING;
		m_LogicalHD720WeightAvcParty[eNonMix][e_SD30] = PORT_WEIGHT_SD30_MPMRX_HW_LIMIT_TRAFFIC_SHAPING;
	}
	else
	{
		m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]  = PORT_WEIGHT_CIF_MPMRX;
		m_LogicalHD720WeightAvcParty[eNonMix][e_SD30] = PORT_WEIGHT_SD30_MPMRX;
	}

	m_LogicalHD720WeightAvcParty[eNonMix][e_HD720]      = PORT_WEIGHT_HD720_MPMRX;
	m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p30]  = PORT_WEIGHT_HD1080_MPMRX;
	m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p60]  = PORT_WEIGHT_HD1080_60_MPMRX;

	m_LogicalHD720WeightAvcParty[eMix][e_Audio]         = PORT_WEIGHT_AUDIO_MIXED_MPMRX_HW_LIMIT;
	m_LogicalHD720WeightAvcParty[eMix][e_Cif]           = PORT_WEIGHT_CIF_MIXED_MPMRX_HW_LIMIT;
	m_LogicalHD720WeightAvcParty[eMix][e_SD30]          = PORT_WEIGHT_SD30_MIXED_MPMRX_HW_LIMIT;
	m_LogicalHD720WeightAvcParty[eMix][e_HD720]         = PORT_WEIGHT_HD720_MIXED_MPMRX_HW_LIMIT;
	m_LogicalHD720WeightAvcParty[eMix][e_HD1080p30]     = PORT_WEIGHT_HD1080_MIXED_MPMRX_HW_LIMIT;
	m_LogicalHD720WeightAvcParty[eMix][e_HD1080p60]     = PORT_WEIGHT_HD1080_60_MIXED_MPMRX_HW_LIMIT;

	m_logicalHD720WeightSvcParty[eNonMix][e_Audio]     = PORT_WEIGHT_SVC_MPMRX;
	m_logicalHD720WeightSvcParty[eNonMix][e_Cif]       = PORT_WEIGHT_SVC_MPMRX;
	m_logicalHD720WeightSvcParty[eNonMix][e_SD30]      = PORT_WEIGHT_SVC_MPMRX;
	m_logicalHD720WeightSvcParty[eNonMix][e_HD720]     = PORT_WEIGHT_SVC_MPMRX;
	m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p30] = PORT_WEIGHT_SVC_MPMRX;
	m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p60] = PORT_WEIGHT_SVC_MPMRX;

	m_logicalHD720WeightSvcParty[eMix][e_Audio]        = PORT_WEIGHT_SVC_MIXED_MPMRX_HW_LIMIT;
	m_logicalHD720WeightSvcParty[eMix][e_Cif]          = PORT_WEIGHT_SVC_MIXED_MPMRX_HW_LIMIT;
	m_logicalHD720WeightSvcParty[eMix][e_SD30]         = PORT_WEIGHT_SVC_MIXED_MPMRX_HW_LIMIT;
	m_logicalHD720WeightSvcParty[eMix][e_HD720]        = PORT_WEIGHT_SVC_MIXED_MPMRX_HW_LIMIT;
	m_logicalHD720WeightSvcParty[eMix][e_HD1080p30]    = PORT_WEIGHT_SVC_MIXED_MPMRX_HW_LIMIT;
	m_logicalHD720WeightSvcParty[eMix][e_HD1080p60]    = PORT_WEIGHT_SVC_MIXED_MPMRX_HW_LIMIT;

	m_logicalHD720WeightCopParty = 0;
	m_ART_PROMILLES                = ART_PROMILLES_UP_TO_1MB_MPMRX;
	m_VID_TOTAL_HD720_PROMILLES    = VID_TOTAL_HD720_30FS_PROMILLES_MPMRX;
}

//--------------------------------------------------------------------------
void CBaseModeResources::InitSoftCGWeightsAndValues()
{
	FTRACEINTO << "SoftMCU Call Generator";

	m_LogicalHD720WeightAvcParty[eNonMix][e_Audio]     = PORT_WEIGHT_AUDIO_SOFT_CG;
	m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]       = PORT_WEIGHT_CIF_SOFT_CG;
	m_LogicalHD720WeightAvcParty[eNonMix][e_SD30]      = PORT_WEIGHT_SD30_SOFT_CG;
	m_LogicalHD720WeightAvcParty[eNonMix][e_HD720]     = PORT_WEIGHT_HD720_SOFT_CG;
	m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p30] = PORT_WEIGHT_HD1080_SOFT_CG;
	m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p60] = PORT_WEIGHT_HD1080_60_SOFT_CG;

	m_LogicalHD720WeightAvcParty[eMix][e_Audio]        = PORT_WEIGHT_AUDIO_SOFT_CG;
	m_LogicalHD720WeightAvcParty[eMix][e_Cif]          = PORT_WEIGHT_CIF_SOFT_CG;
	m_LogicalHD720WeightAvcParty[eMix][e_SD30]         = PORT_WEIGHT_SD30_SOFT_CG;
	m_LogicalHD720WeightAvcParty[eMix][e_HD720]        = PORT_WEIGHT_HD720_SOFT_CG;
	m_LogicalHD720WeightAvcParty[eMix][e_HD1080p30]    = PORT_WEIGHT_HD1080_SOFT_CG;
	m_LogicalHD720WeightAvcParty[eMix][e_HD1080p60]    = PORT_WEIGHT_HD1080_60_SOFT_CG;

	memset(m_logicalHD720WeightSvcParty, 0, sizeof(m_logicalHD720WeightSvcParty));

	m_ART_PROMILLES             = ART_PROMILLES_SOFT_CG;
	m_VID_TOTAL_HD720_PROMILLES = VID_TOTAL_HD720_30FS_PROMILLES_SOFT_CG;
}

//--------------------------------------------------------------------------
void CBaseModeResources::InitSoftNinjaWeightsAndValues()
{
	FPTRACE(eLevelInfoNormal, "CBaseModeResources::InitSoftNinjaWeightsAndValues ");

	m_LogicalHD720WeightAvcParty[eNonMix][e_Audio]      = PORT_WEIGHT_AUDIO_SOFT_NINJA;
	m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]        = PORT_WEIGHT_CIF_SD_SOFT_NINJA;
	m_LogicalHD720WeightAvcParty[eNonMix][e_SD30]       = PORT_WEIGHT_CIF_SD_SOFT_NINJA;
	m_LogicalHD720WeightAvcParty[eNonMix][e_HD720]      = PORT_WEIGHT_HD720P30_SOFT_NINJA;
	m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p30]  = PORT_WEIGHT_HD1080P30_SOFT_NINJA;
	m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p60]  = PORT_WEIGHT_HD1080P60_SOFT_NINJA;

	m_LogicalHD720WeightAvcParty[eMix][e_Audio]         = PORT_WEIGHT_AUDIO_MIXED_SOFT_NINJA_3_CARDS;
	m_LogicalHD720WeightAvcParty[eMix][e_Cif]           = PORT_WEIGHT_CIF_MIXED_SOFT_NINJA;
	m_LogicalHD720WeightAvcParty[eMix][e_SD30]          = PORT_WEIGHT_SD_MIXED_SOFT_NINJA;
	m_LogicalHD720WeightAvcParty[eMix][e_HD720]         = PORT_WEIGHT_HD720P30_MIXED_SOFT_NINJA;
	m_LogicalHD720WeightAvcParty[eMix][e_HD1080p30]     = PORT_WEIGHT_HD1080P30_MIXED_SOFT_NINJA;
	m_LogicalHD720WeightAvcParty[eMix][e_HD1080p60]     = PORT_WEIGHT_HD1080P60_MIXED_SOFT_NINJA;

	m_logicalHD720WeightSvcParty[eNonMix][e_Audio]     = PORT_WEIGHT_SVC_SOFT_NINJA;
	m_logicalHD720WeightSvcParty[eNonMix][e_Cif]       = PORT_WEIGHT_SVC_SOFT_NINJA;
	m_logicalHD720WeightSvcParty[eNonMix][e_SD30]      = PORT_WEIGHT_SVC_SOFT_NINJA;
	m_logicalHD720WeightSvcParty[eNonMix][e_HD720]     = PORT_WEIGHT_SVC_SOFT_NINJA;
	m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p30] = PORT_WEIGHT_SVC_SOFT_NINJA;
	m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p60] = PORT_WEIGHT_SVC_SOFT_NINJA;

	m_logicalHD720WeightSvcParty[eMix][e_Audio]        = PORT_WEIGHT_SVC_MIXED_SOFT_NINJA;
	m_logicalHD720WeightSvcParty[eMix][e_Cif]          = PORT_WEIGHT_SVC_MIXED_SOFT_NINJA;
	m_logicalHD720WeightSvcParty[eMix][e_SD30]         = PORT_WEIGHT_SVC_MIXED_SOFT_NINJA;
	m_logicalHD720WeightSvcParty[eMix][e_HD720]        = PORT_WEIGHT_SVC_MIXED_SOFT_NINJA;
	m_logicalHD720WeightSvcParty[eMix][e_HD1080p30]    = PORT_WEIGHT_SVC_MIXED_SOFT_NINJA;
	m_logicalHD720WeightSvcParty[eMix][e_HD1080p60]    = PORT_WEIGHT_SVC_MIXED_SOFT_NINJA;

	m_ART_PROMILLES             = ART_PROMILLES_SOFT_NINJA;
	m_VID_TOTAL_HD720_PROMILLES = VID_TOTAL_HD720_30FS_PROMILLES_MPMRX;
}

//--------------------------------------------------------------------------
void CBaseModeResources::PrintToTrace(float number[NUM_OF_PARTY_RESOURCE_TYPES], char* name)
{
	CPrettyTable<const char*, float> tbl("Type", "Total");

	for (int i = e_Audio; i < NUM_OF_PARTY_RESOURCE_TYPES; ++i)
	{
		ePartyResourceTypes partyResourceType = (ePartyResourceTypes)i;
		tbl.Add(to_string(partyResourceType), number[i]);
	}
	FTRACEINTO << name << tbl.Get() <<"\nLogicalHD720WeightCopParty:" <<  m_logicalHD720WeightCopParty;
}

//--------------------------------------------------------------------------
void CBaseModeResources::PrintToTracePartyLogicalWeight(char* name)
{
	CPrettyTable<const char*, float, float> tbl_AVC("Type", "NonMixed", "Mixed");
	CPrettyTable<const char*, float, float> tbl_SVC("Type", "NonMixed", "Mixed");

	for (int i = e_Audio; i < NUM_OF_PARTY_RESOURCE_TYPES; ++i)
	{
		ePartyResourceTypes partyResourceType = (ePartyResourceTypes)i;
		tbl_AVC.Add(to_string(partyResourceType), m_LogicalHD720WeightAvcParty[eNonMix][i], m_LogicalHD720WeightAvcParty[eMix][i]);
		tbl_SVC.Add(to_string(partyResourceType), m_logicalHD720WeightSvcParty[eNonMix][i], m_logicalHD720WeightSvcParty[eMix][i]);
	}
	FTRACEINTO
		<< name
		<< "\nLogicalHD720WeightAvcParty: (AVC)" << tbl_AVC.Get()
		<< "\nLogicalHD720WeightSvcParty: (SVC)" << tbl_SVC.Get();
}

//--------------------------------------------------------------------------
void CBaseModeResources::PrintToTrace(WORD number[NUM_OF_PARTY_RESOURCE_TYPES], char* name)
{
	CPrettyTable<const char*, WORD> tbl("Type", "Total");

	for (int i = e_Audio; i < NUM_OF_PARTY_RESOURCE_TYPES; ++i)
	{
		ePartyResourceTypes partyResourceType = (ePartyResourceTypes)i;
		tbl.Add(to_string(partyResourceType), number[i]);
	}
	FTRACEINTO << name << tbl.Get();
}

//--------------------------------------------------------------------------
ePartyResourceTypes CBaseModeResources::VideoPartyTypeToPartyResourceType(eVideoPartyType videoPartyType)
{
	eResourceAllocationTypes rsrcAllocType = CHelperFuncs::GetResourceAllocationType();

	switch (rsrcAllocType)
	{
		case eFixedBreezeMode:
		case eAutoBreezeMode:
		case eAutoMixedMode: // In Mixed Mode we return the higher resource party type between MPMx and MPM-Rx (same as MPMx)
		{
			switch (videoPartyType)
			{
				case eVideo_party_type_none:
				case eVoice_relay_party_type:
				case eVSW_Content_for_CCS_Plugin_party_type:
					return e_Audio;

				case eVSW_video_party_type:
				case eCP_H264_upto_CIF_video_party_type:
				case eCP_VP8_upto_CIF_video_party_type:
				case eVideo_relay_CIF_party_type:
				case eVSW_relay_CIF_party_type:
					return e_Cif;

				case eCP_Content_for_Legacy_Decoder_video_party_type:
				case eCP_H264_upto_SD30_video_party_type:
				case eCP_VP8_upto_SD30_video_party_type:
				case eCP_H261_H263_upto_CIF_video_party_type:
				case eVideo_relay_SD_party_type:
				case eVSW_relay_SD_party_type:
					return e_SD30;

				case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
				case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
				case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
				case eVideo_relay_HD720_party_type:
				case eVSW_relay_HD720_party_type:
					return e_HD720;

				case eCP_Content_for_Legacy_Decoder_HD1080_video_party_type:
				case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
				case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
				case eCP_H261_CIF_equals_H264_HD1080_video_party_type:
				case eVideo_relay_HD1080_party_type:
				case eVSW_relay_HD1080_party_type:
					return e_HD1080p30;

				case eCP_Content_for_Legacy_Decoder_HD1080_60FS_video_party_type:
				case eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type:
				case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
					return e_HD1080p60;

				default:
					FPASSERT(1 + videoPartyType);
					return e_Audio;
			} // switch
		}
		break;

		case eAutoMpmRxMode:
		case eFixedMpmRxMode:
		{
			switch (videoPartyType)
			{
				case eVideo_party_type_none:
				case eVoice_relay_party_type:
				case eVSW_Content_for_CCS_Plugin_party_type:
					return e_Audio;

				case eVideo_relay_CIF_party_type:
				case eVSW_relay_CIF_party_type:
				case eCP_H264_upto_CIF_video_party_type:
				case eCP_VP8_upto_CIF_video_party_type:
					return e_Cif;

				case eCP_Content_for_Legacy_Decoder_video_party_type:
				case eCP_H264_upto_SD30_video_party_type:
				case eCP_VP8_upto_SD30_video_party_type:
				case eVideo_relay_SD_party_type:
				case eVSW_relay_SD_party_type:
					return e_SD30;

				case eCP_H261_H263_upto_CIF_video_party_type:
				case eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type:
				case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
				case eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type:
				case eVideo_relay_HD720_party_type:
				case eVSW_relay_HD720_party_type:
				case eVSW_video_party_type:
					return e_HD720;

				case eCP_Content_for_Legacy_Decoder_HD1080_video_party_type:
				case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
				case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
				case eCP_H261_CIF_equals_H264_HD1080_video_party_type:
				case eVideo_relay_HD1080_party_type:
				case eVSW_relay_HD1080_party_type:
					return e_HD1080p30;

				case eCP_Content_for_Legacy_Decoder_HD1080_60FS_video_party_type:
				case eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type:
					return e_HD1080p60;

				default:
					FPASSERT(1 + videoPartyType);
					return e_Audio;
			} // switch
		}
		break;

		default:
			break;
	} // switch

	FPASSERT(1 + rsrcAllocType);
	return e_Audio;
}

//--------------------------------------------------------------------------
eVideoPartyType CBaseModeResources::PartyResourceTypeToVideoPartyType(ePartyResourceTypes partyResourceType, eSystemCardsMode cardsMode)
{
	switch (partyResourceType)
	{
		case e_Audio:
			return eVideo_party_type_none;
		case e_Cif:
			return eCP_H264_upto_CIF_video_party_type;
		case e_SD30:
			return eCP_H264_upto_SD30_video_party_type;
		case e_HD720:
			return eCP_H264_upto_HD720_30FS_Symmetric_video_party_type;
		case e_HD1080p30:
			return eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
		case e_HD1080p60:
			if (eSystemCardsMode_mpmrx == cardsMode)
				return eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type;
			else
				return eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	} // switch
	FPASSERT(partyResourceType);
	return eVideo_party_type_none;
}

//--------------------------------------------------------------------------
void CBaseModeResources::OnReconfigureUnitsTimer()
{
	m_BusyInReconfigureUnits = FALSE;
}

//--------------------------------------------------------------------------
void CBaseModeResources::PureModeReconfigureUnitsAccordingToProportion()
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	FPASSERT_AND_RETURN(pSystemResources == NULL);

	DWORD numArtUnitsPerBoard[BOARDS_NUM];
	DWORD numVideoUnitsPerBoard[BOARDS_NUM];
	DWORD currentNumArtUnitsPerBoard[BOARDS_NUM];
	DWORD currentNumVideoUnitsPerBoard[BOARDS_NUM];
	int numOfUnitsToReconfigure[BOARDS_NUM];
	eUnitType newUnitType[BOARDS_NUM];

	// //////////////////////////////////////////////////////////////////////////////////////
	// Step 1: calculate how much ART and Video units there currently are, and divide art and video according to proportion
	// ///////////////////////////////////////////////////////////////////////////////////////
	DWORD totalNumOfArtUnits   = 0;
	DWORD totalNumOfVideoUnits = 0;

	for (int i = 0; i < BOARDS_NUM; i++)
	{
		numOfUnitsToReconfigure[i]      = 0;
		numArtUnitsPerBoard[i]          = 0;
		numVideoUnitsPerBoard[i]        = 0;
		currentNumArtUnitsPerBoard[i]   = 0;
		currentNumVideoUnitsPerBoard[i] = 0;
		newUnitType[i]                  = eUnitType_Generic;

		int oneBasedBoardId = i + 1;
		if (!pSystemResources->IsBoardIdExists(oneBasedBoardId))
			continue;

		CBoard* pBoard = pSystemResources->GetBoard(oneBasedBoardId);
		if (pBoard == NULL)
			continue;

		pBoard->CountUnits(currentNumArtUnitsPerBoard[i], currentNumVideoUnitsPerBoard[i], TRUE);
		pBoard->DivideArtAndVideoUnitsAccordingToProportion(totalNumOfArtUnits, totalNumOfVideoUnits, numArtUnitsPerBoard[i], numVideoUnitsPerBoard[i]);
		totalNumOfArtUnits   += numArtUnitsPerBoard[i];
		totalNumOfVideoUnits += numVideoUnitsPerBoard[i];
	}

	PrintDataAboutUnitReconfiguration(currentNumArtUnitsPerBoard,
	                                  currentNumVideoUnitsPerBoard,
	                                  numArtUnitsPerBoard,
	                                  numVideoUnitsPerBoard,
	                                  "After dividing according to proportion");

	// ///////////////////////////////////////////////////////////////////////////////////////
	// Step 2: fine-tuning for video reconfiguration
	// ///////////////////////////////////////////////////////////////////////////////////////
	BOOL bFound = FALSE;
	if (CheckVideoUnitsWithConfig(numVideoUnitsPerBoard) == FALSE)
	{
		// this means we have to find a board which has at least two ART units, and change it to a video unit
		for (int bid_to_remove_art = 0; bid_to_remove_art < BOARDS_NUM; bid_to_remove_art++)
		{
			if (numArtUnitsPerBoard[bid_to_remove_art] >= 2)
			{
				// now try to find a board where art can be added: this is a board that exists (it has art units, and that is different than the one to remove from)
				for (int bid_to_add_art = 0; bid_to_add_art < BOARDS_NUM; bid_to_add_art++)
				{
					if (numArtUnitsPerBoard[bid_to_add_art] > 0 && bid_to_add_art != bid_to_remove_art)
					{
						// try and see if now it's OK
						numVideoUnitsPerBoard[bid_to_remove_art]++;
						numVideoUnitsPerBoard[bid_to_add_art]--;
						if (CheckVideoUnitsWithConfig(numVideoUnitsPerBoard) == TRUE)
						{
							// we found a better way to configure the units
							FTRACEINTO << "Removing one ART unit from board " << bid_to_remove_art + 1 << " and adding one ART unit to board " << bid_to_add_art + 1;
							bFound = TRUE;
							numArtUnitsPerBoard[bid_to_remove_art]--;
							numArtUnitsPerBoard[bid_to_add_art]++;
							break;
						}
						else
						{
							// it's not the correct one, continue (but first change the numbers back to their original ones)
							numVideoUnitsPerBoard[bid_to_remove_art]--;
							numVideoUnitsPerBoard[bid_to_add_art]++;
						}
					}
				}
			}
		}

		if (bFound == FALSE)
		{
			FTRACEINTO << "Returned false, but didn't find other way to configure units";
		}

		PrintDataAboutUnitReconfiguration(currentNumArtUnitsPerBoard,
		                                  currentNumVideoUnitsPerBoard,
		                                  numArtUnitsPerBoard,
		                                  numVideoUnitsPerBoard,
		                                  "After fine-tuning");
	}

	// ///////////////////////////////////////////////////////////////////////////////////////
	// Step 3: fill in what has to be reconfigured
	// ///////////////////////////////////////////////////////////////////////////////////////
	BOOL bSomethingHasToBeReconfigured = FALSE;
	for (int i = 0; i < BOARDS_NUM; i++)
	{
		if (numArtUnitsPerBoard[i] > currentNumArtUnitsPerBoard[i])
		{
			numOfUnitsToReconfigure[i] = numArtUnitsPerBoard[i] - currentNumArtUnitsPerBoard[i];
			newUnitType[i] = eUnitType_Art;
			bSomethingHasToBeReconfigured = TRUE;
		}
		else if (numVideoUnitsPerBoard[i] > currentNumVideoUnitsPerBoard[i])
		{
			numOfUnitsToReconfigure[i] = numVideoUnitsPerBoard[i] - currentNumVideoUnitsPerBoard[i];
			newUnitType[i] = eUnitType_Video;
			bSomethingHasToBeReconfigured = TRUE;
		}
		else
		{
			FTRACEINTO << "BoardId:" << i << " - No need to reconfigure units on this board";
		}
	}

	if (bSomethingHasToBeReconfigured == FALSE)
	{
		FTRACEINTO << "No need to reconfigure anything";
		return;
	}

	// ///////////////////////////////////////////////////////////////////////////////////////
	// Step 4: actual reconfiguration
	// ///////////////////////////////////////////////////////////////////////////////////////
	ReconfigureUnits(numOfUnitsToReconfigure, newUnitType);
}

//--------------------------------------------------------------------------
void CBaseModeResources::PrintDataAboutUnitReconfiguration(DWORD currentNumArtUnitsPerBoard[BOARDS_NUM],
                                                           DWORD currentNumVideoUnitsPerBoard[BOARDS_NUM],
                                                           DWORD numArtUnitsPerBoard[BOARDS_NUM],
                                                           DWORD numVideoUnitsPerBoard[BOARDS_NUM],
                                                           char* name)
{
	CPrettyTable<int, WORD, WORD, WORD, WORD> tbl("BoardId", "CurrentNumArtUnits", "CurrentNumVidUnits", "TotalNumArtUnits", "TotalNumVidUnits");

	for (int i = 0; i < BOARDS_NUM; i++)
		tbl.Add(i, currentNumArtUnitsPerBoard[i], currentNumVideoUnitsPerBoard[i], numArtUnitsPerBoard[i], numVideoUnitsPerBoard[i]);

	FTRACEINTO << name << tbl.Get();
}

//--------------------------------------------------------------------------
void CBaseModeResources::ReconfigureUnits(int numOfUnitsToReconfigure[BOARDS_NUM], eUnitType newUnitType[BOARDS_NUM])
{
	FTRACEINTO;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	FPASSERT_AND_RETURN(!pSystemResources);

	m_BusyInReconfigureUnits = TRUE;

	for (int i = 0; i < BOARDS_NUM; i++)
	{
		if (numOfUnitsToReconfigure[i] != 0)
		{
			int oneBasedBoardId = i + 1;
			if (!pSystemResources->IsBoardIdExists(oneBasedBoardId))
			{
				FPASSERTSTREAM(1, "BoardId:" << oneBasedBoardId-1 << " - Failed, board does not exist");
				continue;
			}

			CBoard* pBoard = pSystemResources->GetBoard(oneBasedBoardId);
			if (pBoard == NULL)
			{
				FPASSERTSTREAM(1, "BoardId:" << oneBasedBoardId-1 << " - Failed, board does not found");
				continue;
			}

			int numOfUnitsChanged = pBoard->ChangeUnits(numOfUnitsToReconfigure[i], newUnitType[i]);
			if (numOfUnitsChanged != numOfUnitsToReconfigure[i])
			{
				FPASSERTSTREAM(1, "BoardId:" << oneBasedBoardId-1 << ", numOfUnitsToReconfigure:" << numOfUnitsToReconfigure[i] << ", numOfUnitsChanged:" << numOfUnitsChanged);
				continue;
			}
		}
	}

	// start timer
	CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();
	FPASSERT_AND_RETURN(!pResourceManager);

	pResourceManager->StartTimer(RECONFIGURE_UNITS_TIMER, SECONDS_FOR_RECONFIGURE_UNITS_TIMER*SECOND);
}

//--------------------------------------------------------------------------
STATUS CBaseModeResources::BaseCanSetConfigurationNow()
{
	// illegal when setting last configuration
	if (m_BusyInReconfigureUnits == TRUE)
	{
		FTRACEINTO << "Status:STATUS_SYSTEM_BUSY_SETTING_LAST_CONFIGURATION";
		return STATUS_SYSTEM_BUSY_SETTING_LAST_CONFIGURATION;
	}

	// illegal during startup
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessInvalid == curStatus) || (eProcessStartup == curStatus) || (eProcessIdle == curStatus))
	{
		FTRACEINTO << "Status:STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP";
		return STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP;
	}

	// illegal when there are conferences running
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (!pConfRsrcDB || pConfRsrcDB->GetNumConfRsrcs() != 0)
	{
		FTRACEINTO << "Status:STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_EXISTS";
		return STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_EXISTS;
	}

	FTRACEINTO << "Status:STATUS_OK";
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CBaseModeResources::BaseInitDongleRestriction(DWORD& num_parties)
{
	CSystemResources* pSyst = CHelperFuncs::GetSystemResources();
	FPASSERT_AND_RETURN(!pSyst);

	int ports_configuration_step = pSyst->GetPortsConfigurationStep();
	if (num_parties % ports_configuration_step != 0)
	{
		DWORD before = num_parties;
		num_parties = num_parties + ports_configuration_step - num_parties % ports_configuration_step;
		FTRACEINTO << "Updating Dongle from " << before << " to " << num_parties << " , ports_configuration_step:" << ports_configuration_step;
	}
}

//--------------------------------------------------------------------------
void CBaseModeResources::UpdateInternalDataFromIsRsrcEnough(CBoardsStatistics* pBoardsStatistics)
{
	m_BoardsStatistics = *pBoardsStatistics;

	// Tsahi - TODO: remove all the rest below this line, we dont need it
	m_totalPromillesAccordingToCards = 0;

	for (int boardId = 0; boardId < BOARDS_NUM; boardId++)
	{
		FPASSERT_AND_RETURN(pBoardsStatistics->m_NumOfEnabledUnits[boardId] > pBoardsStatistics->m_NumOfUnits[boardId]);
		m_PromillesPerCard[boardId] = pBoardsStatistics->m_NumOfUnits[boardId] * 1000;

		m_totalPromillesAccordingToCards += m_PromillesPerCard[boardId];
	}
}

//--------------------------------------------------------------------------
void CBaseModeResources::UpdatePortWeightsTo1500Q()
{

	if (m_LogicalHD720WeightAvcParty[eNonMix][e_Audio] == PORT_WEIGHT_AUDIO_1500Q &&
		m_LogicalHD720WeightAvcParty[eNonMix][e_Cif] == PORT_WEIGHT_CIF_1500Q &&
		m_LogicalHD720WeightAvcParty[eNonMix][e_SD30] == PORT_WEIGHT_SD30_1500Q &&
		m_LogicalHD720WeightAvcParty[eNonMix][e_HD720] == PORT_WEIGHT_HD720_1500Q &&
		m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p30] == PORT_WEIGHT_HD1080_1500Q &&
		m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p60] == PORT_WEIGHT_HD1080_1500Q)
	{
		FTRACEINTO << "Port weights are already set to RMX1500Q";
	}
	else
	{
		FTRACEINTO << "Setting RMX1500Q weights";

		InitPortWeightsTo1500Q();

		//for COP
		m_logicalHD720WeightCopParty = PORT_WEIGHT_COP_1500Q;
	}
}

//--------------------------------------------------------------------------
void CBaseModeResources::UpdateLogicalWeightsAndValues(eSystemCardsMode cardsMode)
{
	CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();
	FPASSERTMSG_AND_RETURN(!pResourceManager, "CBaseModeResources::UpdateLogicalWeightsAndValues - Failed, pResourceManager is NULL");

	eProductType prodType = CProcessBase::GetProcess()->GetProductType();
	eProductFamily prodFamily = CProcessBase::GetProcess()->GetProductFamily();
	FTRACEINTO << "CardsMode:" << GetSystemCardsModeStr(cardsMode) << ", ProductType:" << ProductTypeToString(prodType);

	switch (cardsMode)
	{
		case eSystemCardsMode_breeze:
			if (eProductFamilySoftMcu == prodFamily && prodType != eProductTypeSoftMCUMfw && prodType != eProductTypeGesher)
				UpdateSoftMPMXWeightsAndValues(pResourceManager->isRPPLicense());
			break;

		case eSystemCardsMode_mpmrx:
			if (eProductFamilyRMX == prodFamily)
				UpdateMpmRxWeightsAndValues();
			else if (eProductTypeNinja == prodType)
				UpdateSoftNinjaWeightsAndValues();
			break;

		case eSystemCardsMode_mixed_mode:
			break;

		default:
			FPASSERTMSG(cardsMode, "Illegal cards mode in CBaseModeResources::UpdateLogicalWeightsAndValues");
			break;
	} // switch

	PrintToTracePartyLogicalWeight("CBaseModeResources::UpdateLogicalWeightsAndValues");
}

//--------------------------------------------------------------------------
void CBaseModeResources::UpdateMpmRxWeightsAndValues()
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	FPASSERTMSG_AND_RETURN(!pSystemResources, "CBaseModeResources::UpdateMpmRxWeightsAndValues - Failed, pSystemResources is NULL");

	BOOL isTrafficShapingEnabled = NO;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_RTP_TRAFFIC_SHAPING, isTrafficShapingEnabled);
	WORD numHD720PortsAccordingToCards = pSystemResources->GetHD720PortsAccordingToCards();

	DWORD dongleNumOfParties = pSystemResources->GetDongleNumOfParties();

	// If the license is higher than the number of hardware ports, make it equal to the HW ports for our calculation.
	if (dongleNumOfParties > numHD720PortsAccordingToCards)
		dongleNumOfParties = numHD720PortsAccordingToCards;

	float fMaxCifWeightNonMix     = isTrafficShapingEnabled ? PORT_WEIGHT_CIF_MPMRX_HW_LIMIT_TRAFFIC_SHAPING : PORT_WEIGHT_CIF_MPMRX;
	float fMaxSdWeightNonMix      = isTrafficShapingEnabled ? PORT_WEIGHT_SD30_MPMRX_HW_LIMIT_TRAFFIC_SHAPING : PORT_WEIGHT_SD30_MPMRX;
	float fMaxAudioWeightNonMix   = PORT_WEIGHT_AUDIO_MPMRX_S_HW_LIMIT;
	float fMaxSvc1080WeightNonMix = PORT_WEIGHT_SVC_HD1080_MPMRX_S_HW_LIMIT;

	float fMaxCifWeightMix        = PORT_WEIGHT_CIF_MIXED_MPMRX_HW_LIMIT;
	float fMaxSdWeightMix         = PORT_WEIGHT_SD30_MIXED_MPMRX_HW_LIMIT;
	float fMaxHD720WeightMix      = PORT_WEIGHT_HD720_MIXED_MPMRX_HW_LIMIT;
	float fMaxHD1080p30WeightMix  = PORT_WEIGHT_HD1080_MIXED_MPMRX_HW_LIMIT;
	float fMaxHD1080p60WeightMix  = PORT_WEIGHT_HD1080_60_MIXED_MPMRX_HW_LIMIT;
	float fMaxSvcWeightMix        = PORT_WEIGHT_SVC_MIXED_MPMRX_HW_LIMIT;
	float fMaxAudioWeightMix      = PORT_WEIGHT_AUDIO_MIXED_MPMRX_S_HW_LIMIT;

	for (WORD i = 1; i <= BOARDS_NUM; i++)
	{
		CBoard *pBoard = pSystemResources->GetBoard(i);
		if (pBoard)
		{
			switch (pBoard->GetCardType())
			{
				case eMpmRx_Full:
					fMaxAudioWeightNonMix   = PORT_WEIGHT_AUDIO_MPMRX_HW_LIMIT;
					fMaxSvc1080WeightNonMix = PORT_WEIGHT_SVC_HD1080_MPMRX;
					fMaxAudioWeightMix      = PORT_WEIGHT_AUDIO_MIXED_MPMRX_HW_LIMIT;
					break;

				case eMpmRx_Half:
					fMaxCifWeightMix        = PORT_WEIGHT_CIF_MIXED_MPMRX_HW_HALF_LIMIT;
					fMaxSdWeightMix         = PORT_WEIGHT_SD30_MIXED_MPMRX_HW_HALF_LIMIT;
					break;

				default:
					break;
			}
		}
	}

	// Calculate the optimal weight according to license and hardware.
	// Max_HW_port_weight * license_ports / actual_HW_ports = actual port weight per hardware capacity.
	// Using the max function with the wanted license weight as first parameter and the above calculation as second parameter,
	// will give us the true weight if the hardware cannot reach the desired license weight.
	// In Non-Mix: Audio, CIF, SD can be changed - CIF/SD in case of Traffic Shaping enabled.
	// In Mix: All resolutions can be changed.
	m_LogicalHD720WeightAvcParty[eNonMix][e_Cif] = max((float)PORT_WEIGHT_CIF_MPMRX, fMaxCifWeightNonMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_LogicalHD720WeightAvcParty[eNonMix][e_SD30] = max((float)PORT_WEIGHT_SD30_MPMRX, fMaxSdWeightNonMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_LogicalHD720WeightAvcParty[eNonMix][e_Audio] = max((float)PORT_WEIGHT_AUDIO_MPMRX, fMaxAudioWeightNonMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p30] = max((float)PORT_WEIGHT_SVC_MPMRX, fMaxSvc1080WeightNonMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p60] = max((float)PORT_WEIGHT_SVC_MPMRX, fMaxSvc1080WeightNonMix * dongleNumOfParties / numHD720PortsAccordingToCards);

	// For Mix Mode - use the same ratios of MPMx Mix Mode if possible.
	m_LogicalHD720WeightAvcParty[eMix][e_Audio] = max((float)PORT_WEIGHT_AUDIO_MPMRX, fMaxAudioWeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_LogicalHD720WeightAvcParty[eMix][e_Cif] = max((float)PORT_WEIGHT_CIF_MIXED_MPM_X_HW_LIMIT, fMaxCifWeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_LogicalHD720WeightAvcParty[eMix][e_SD30] = max((float)PORT_WEIGHT_SD30_MIXED_MPM_X_HW_LIMIT, fMaxSdWeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	//m_LogicalHD720WeightOfPartyPerType[eMix][e_HD720] = max((float)PORT_WEIGHT_HD720_MIXED_MPM_X_HW_LIMIT, fMaxHD720WeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	//m_LogicalHD720WeightOfPartyPerType[eMix][e_HD1080p30] = max((float)PORT_WEIGHT_HD1080_MIXED_MPM_X_HW_LIMIT, fMaxHD1080p30WeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	//m_LogicalHD720WeightOfPartyPerType[eMix][e_HD1080p60] = max((float)PORT_WEIGHT_HD1080_60_MIXED_MPM_X_HW_LIMIT, fMaxHD1080p60WeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	float fCalculatedSvcWeightMix = max((float)PORT_WEIGHT_SVC_MIXED_MPM_X_HW_LIMIT, fMaxSvcWeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_logicalHD720WeightSvcParty[eMix][e_Audio] = fCalculatedSvcWeightMix;
	m_logicalHD720WeightSvcParty[eMix][e_Cif] = fCalculatedSvcWeightMix;
	m_logicalHD720WeightSvcParty[eMix][e_SD30] = fCalculatedSvcWeightMix;
	m_logicalHD720WeightSvcParty[eMix][e_HD720] = fCalculatedSvcWeightMix;
	m_logicalHD720WeightSvcParty[eMix][e_HD1080p30] = max((float)PORT_WEIGHT_SVC_MPMRX, fMaxSvc1080WeightNonMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_logicalHD720WeightSvcParty[eMix][e_HD1080p60] = max((float)PORT_WEIGHT_SVC_MPMRX, fMaxSvc1080WeightNonMix * dongleNumOfParties / numHD720PortsAccordingToCards);
}

//--------------------------------------------------------------------------
void CBaseModeResources::UpdateSoftNinjaWeightsAndValues()
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	FPASSERTMSG_AND_RETURN(!pSystemResources, "CBaseModeResources::UpdateSoftNinjaWeightsAndValues - Failed, pSystemResources is NULL");

	WORD numHD720PortsAccordingToCards;

	DWORD dongleNumOfParties = pSystemResources->GetDongleNumOfParties();
	WORD ninjaDspCount = pSystemResources->GetDSPCount(); // Ninja has 1-3 DSP cards, each one with 6 DSPs.

	// Initialize Non-Mix values according to Ninja 1800
	float fMaxAudioWeightNonMix;
	float fMaxSvc1080WeightNonMix;

	// Initialize Mix Mode values according to Ninja 1800
	float fMaxAudioWeightMix;
	float fMaxCifWeightMix;
	float fMaxSdWeightMix;
	float fMaxSvcWeightMix;
	float fMaxSvc1080WeightMix;

	// Ninja 1800 with 1 Meridian card
	if (ninjaDspCount > 0 && ninjaDspCount <= 6)
	{
		numHD720PortsAccordingToCards = 35;

		fMaxAudioWeightNonMix = PORT_WEIGHT_AUDIO_SOFT_NINJA_HW_LIMIT_1_CARD;
		fMaxSvc1080WeightNonMix = PORT_WEIGHT_SVC_HD1080_SOFT_NINJA_HW_LIMIT_1_CARD;

		fMaxAudioWeightMix = PORT_WEIGHT_AUDIO_MIXED_SOFT_NINJA_1_CARD;
		fMaxCifWeightMix = PORT_WEIGHT_CIF_MIXED_SOFT_NINJA_1_CARD;
		fMaxSdWeightMix = PORT_WEIGHT_SD_MIXED_SOFT_NINJA_1_CARD;
		fMaxSvcWeightMix = PORT_WEIGHT_SVC_MIXED_SOFT_NINJA_1_CARD;
		fMaxSvc1080WeightMix = PORT_WEIGHT_SVC_HD1080_MIXED_SOFT_NINJA_1_CARD;
	}
	else // with 3 Meridian Card
	{
		numHD720PortsAccordingToCards = 100;

		fMaxAudioWeightNonMix = PORT_WEIGHT_AUDIO_SOFT_NINJA_HW_LIMIT_3_CARDS;
		fMaxSvc1080WeightNonMix = PORT_WEIGHT_SVC_HD1080_SOFT_NINJA_HW_LIMIT_3_CARDS;

		fMaxAudioWeightMix = PORT_WEIGHT_AUDIO_MIXED_SOFT_NINJA_3_CARDS;
		fMaxCifWeightMix = PORT_WEIGHT_CIF_MIXED_SOFT_NINJA_3_CARDS;
		fMaxSdWeightMix = PORT_WEIGHT_SD_MIXED_SOFT_NINJA_3_CARDS;
		fMaxSvcWeightMix = PORT_WEIGHT_SVC_MIXED_SOFT_NINJA_3_CARDS;
		fMaxSvc1080WeightMix = PORT_WEIGHT_SVC_HD1080_MIXED_SOFT_NINJA_3_CARDS;
	}

	// If the license is higher than the number of hardware ports, make it equal to the HW ports for our calculation.
	if (dongleNumOfParties > numHD720PortsAccordingToCards)
		dongleNumOfParties = numHD720PortsAccordingToCards;

	// Calculate the optimal weight according to license and hardware.
	// Max_HW_port_weight * license_ports / actual_HW_ports = actual port weight per hardware capacity.
	// Using the max function with the wanted license weight as first parameter and the above calculation as second parameter,
	// will give us the true weight if the hardware cannot reach the desired license weight.

	// For NonMix Mode
	m_LogicalHD720WeightAvcParty[eNonMix][e_Audio] = max((float)PORT_WEIGHT_AUDIO_SOFT_NINJA, fMaxAudioWeightNonMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p30] = max((float)PORT_WEIGHT_SVC_SOFT_NINJA, fMaxSvc1080WeightNonMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p60] = max((float)PORT_WEIGHT_SVC_SOFT_NINJA, fMaxSvc1080WeightNonMix * dongleNumOfParties / numHD720PortsAccordingToCards);

	// For Mix Mode - use the same ratios of MPMx Mix Mode if possible.
	m_LogicalHD720WeightAvcParty[eMix][e_Audio] = max((float)PORT_WEIGHT_AUDIO_MIXED_SOFT_NINJA, fMaxAudioWeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_LogicalHD720WeightAvcParty[eMix][e_Cif] = max((float)PORT_WEIGHT_CIF_MIXED_SOFT_NINJA, fMaxCifWeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_LogicalHD720WeightAvcParty[eMix][e_SD30] = max((float)PORT_WEIGHT_SD_MIXED_SOFT_NINJA, fMaxSdWeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	float fCalculatedSvcWeightMix = max((float)PORT_WEIGHT_SVC_MIXED_SOFT_NINJA, fMaxSvcWeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_logicalHD720WeightSvcParty[eMix][e_Audio] = fCalculatedSvcWeightMix;
	m_logicalHD720WeightSvcParty[eMix][e_Cif] = fCalculatedSvcWeightMix;
	m_logicalHD720WeightSvcParty[eMix][e_SD30] = fCalculatedSvcWeightMix;
	m_logicalHD720WeightSvcParty[eMix][e_HD720] = fCalculatedSvcWeightMix;
	m_logicalHD720WeightSvcParty[eMix][e_HD1080p30] = max((float)PORT_WEIGHT_SVC_MIXED_SOFT_NINJA, fMaxSvc1080WeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_logicalHD720WeightSvcParty[eMix][e_HD1080p60] = max((float)PORT_WEIGHT_SVC_MIXED_SOFT_NINJA, fMaxSvc1080WeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
}

//--------------------------------------------------------------------------
void CBaseModeResources::UpdateSoftMPMXWeightsAndValues(bool isRPPMode)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	FPASSERTMSG_AND_RETURN(!pSystemResources, "CBaseModeResources::UpdateMpmRxWeightsAndValues - Failed, pSystemResources is NULL");

	WORD numHD720PortsAccordingToCards = pSystemResources->GetHD720PortsAccordingToCards();

	DWORD dongleNumOfParties = pSystemResources->GetDongleNumOfParties();

	// If the license is higher than the number of hardware ports, make it equal to the HW ports for our calculation.
	if (dongleNumOfParties > numHD720PortsAccordingToCards)
		dongleNumOfParties = numHD720PortsAccordingToCards;

	float fMaxCifWeightMix = PORT_WEIGHT_CIF_MIXED_SOFT_MPM_X;
	float fMaxSdWeightMix = PORT_WEIGHT_SD30_MIXED_SOFT_MPM_X;
	float fMaxHD720WeightMix = PORT_WEIGHT_HD720_MIXED_SOFT_MPM_X;
	float fMaxHD1080p30WeightMix = PORT_WEIGHT_HD1080_MIXED_SOFT_MPM_X;
	float fMaxHD1080p60WeightMix = PORT_WEIGHT_HD1080_P60_MIXED_SOFT_MPM_X;
	float fMaxSvcWeightMix = isRPPMode ? PORT_WEIGHT_SVC_MIXED_SOFT_MPM_X_RPP : PORT_WEIGHT_SVC_MIXED_SOFT_MPM_X_A_LA_CART;
	float fMaxSvcWeightNonMix = isRPPMode ? PORT_WEIGHT_SVC_SOFT_MPM_X_RPP : PORT_WEIGHT_SVC_SOFT_MPM_X_A_LA_CART;
	float fMaxSvc1080WeightNonMix = isRPPMode ? PORT_WEIGHT_SVC_1080_SOFT_MPM_X_RPP : PORT_WEIGHT_SVC_1080_SOFT_MPM_X_A_LA_CART;
	float fMaxAudioWeightNonMix = isRPPMode ? PORT_WEIGHT_AUDIO_SOFT_MPM_X_RPP : PORT_WEIGHT_AUDIO_SOFT_MPM_X_A_LA_CART;

	m_LogicalHD720WeightAvcParty[eNonMix][e_Audio] = fMaxAudioWeightNonMix;

	for (size_t i=0;i <NUM_OF_PARTY_RESOURCE_TYPES-2;++i)
			m_logicalHD720WeightSvcParty[eNonMix][i]  = fMaxSvcWeightNonMix;

	m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p30] = fMaxSvc1080WeightNonMix;
	m_logicalHD720WeightSvcParty[eNonMix][e_HD1080p60] = fMaxSvc1080WeightNonMix;

	// In Mix: All resolutions can be changed.
	m_LogicalHD720WeightAvcParty[eMix][e_Cif] = max((float)PORT_WEIGHT_CIF_SOFT_MPM_X, fMaxCifWeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_LogicalHD720WeightAvcParty[eMix][e_SD30] = max((float)PORT_WEIGHT_SD30_SOFT_MPM_X, fMaxSdWeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_LogicalHD720WeightAvcParty[eMix][e_HD720] = max((float)PORT_WEIGHT_HD720_SOFT_MPM_X, fMaxHD720WeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_LogicalHD720WeightAvcParty[eMix][e_HD1080p30] = max((float)PORT_WEIGHT_HD1080_SOFT_MPM_X, fMaxHD1080p30WeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
	m_LogicalHD720WeightAvcParty[eMix][e_HD1080p60] = max((float)PORT_WEIGHT_HD1080_P60_SOFT_MPM_X, fMaxHD1080p60WeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);

	float fCalculatedSvcWeightMix;

	if (isRPPMode)
	{
		fCalculatedSvcWeightMix = fMaxSvcWeightMix;
		m_LogicalHD720WeightAvcParty[eMix][e_Audio] = PORT_WEIGHT_AUDIO_MIXED_SOFT_MPM_X;
	}
	else
	{
		fCalculatedSvcWeightMix = max((float)PORT_WEIGHT_SVC_SOFT_MPM_X_A_LA_CART, fMaxSvcWeightMix * dongleNumOfParties / numHD720PortsAccordingToCards);
		m_LogicalHD720WeightAvcParty[eMix][e_Audio] = max((float)PORT_WEIGHT_AUDIO_SOFT_MPM_X_A_LA_CART, (float)PORT_WEIGHT_AUDIO_MIXED_SOFT_MPM_X * dongleNumOfParties / numHD720PortsAccordingToCards);
	}

	for(size_t i=0;i<NUM_OF_PARTY_RESOURCE_TYPES;++i)
		m_logicalHD720WeightSvcParty[eMix][i] = fCalculatedSvcWeightMix;
}
