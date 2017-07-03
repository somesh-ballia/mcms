// SystemFeatures.cpp

#include "SystemFeatures.h"
#include "Trace.h"
#include "ConfPartyGlobals.h"

#define CARD_MPM         0x0001
#define CARD_MPM_PLUS    0x0002
#define CARD_MPMX        0x0004
#define CARD_MPMRX       0x0008
#define CARD_SOFT        0x0010
#define CARD_SOFT_NINJA  0x0020
#define CARD_SOFT_CG     0x0040
#define CARD_ALL		 (Power<256,sizeof(m_arrSupportedFeaturesOnSystem[0])>::result - 1)

#define CARD_MPMRX_AND_SOFT_NINJA	(CARD_MPMRX | CARD_SOFT_NINJA)

#define SET_CARDS_SUPPORT_FOR_FEATURE(e,c) {m_arrSupportedFeaturesOnSystem[e] = c;}
#define SET_FEATURE_FOR_ALL_CARDS_EXCEPT(e,c) {m_arrSupportedFeaturesOnSystem[e] = ~(c) & CARD_ALL;}

CSystemFeatures::CSystemFeatures()
{
	memset(m_arrSupportedFeaturesOnSystem, 0, sizeof(m_arrSupportedFeaturesOnSystem));

	InitSupportedFeaturesOnSystemArray();
}

void CSystemFeatures::InitSupportedFeaturesOnSystemArray()
{
	// Example:
	// SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureName, CARD_MPMX | CARD_MPMRX);
	//---------------------------------------------------------------------

	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureCOP, CARD_MPMX);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureRtv, CARD_MPMX | CARD_SOFT | CARD_MPMRX_AND_SOFT_NINJA | CARD_SOFT_CG);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureTip, CARD_MPMX | CARD_MPMRX_AND_SOFT_NINJA | CARD_SOFT_CG);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureH264HighProfile, CARD_MPMX | CARD_MPMRX_AND_SOFT_NINJA | CARD_SOFT | CARD_SOFT_CG);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureEncoderRecurrentIntra, CARD_MPMX | CARD_MPM_PLUS | CARD_MPMRX_AND_SOFT_NINJA | CARD_SOFT);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureDecoderRecurrentIntra, CARD_MPMX | CARD_MPMRX_AND_SOFT_NINJA | CARD_SOFT);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureH264PacketizationMode, CARD_MPMX | CARD_MPMRX_AND_SOFT_NINJA | CARD_SOFT);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureIndicationOnLayout_PacketLost, CARD_MPMX | CARD_MPMRX_AND_SOFT_NINJA | CARD_SOFT);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureLegacyContentHD1080, CARD_MPMX | CARD_MPMRX_AND_SOFT_NINJA);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureEchoSuppression, CARD_MPM_PLUS);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureKeyboardNoiseSuppression, CARD_MPM_PLUS);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureFontTypes, CARD_MPMX | CARD_MPMRX_AND_SOFT_NINJA | CARD_SOFT | CARD_SOFT_CG);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureHD1080p60Asymmetric, CARD_MPMX | CARD_SOFT);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureHD1080p60Symmetric, CARD_MPMRX_AND_SOFT_NINJA | CARD_SOFT_CG);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureContentTranscoding, CARD_MPMX | CARD_MPMRX_AND_SOFT_NINJA | CARD_SOFT);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureMs2013SVC,CARD_SOFT | CARD_MPMRX_AND_SOFT_NINJA | CARD_MPMX /*noa-to remove mpmx*/);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureWeightedArtMpmRx, CARD_MPMX | CARD_MPMRX);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureLineRate_6M, CARD_MPMRX_AND_SOFT_NINJA); //6M only for Ninja and MPMRX currently, please add your product type if needed
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureHD1080p30Content, CARD_MPMRX | CARD_SOFT_NINJA);
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureHD1080p60Content, CARD_MPMRX | CARD_SOFT_NINJA); // AmirK to Feature 353
	//EE-471
	SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureRssDialin, CARD_MPM | CARD_MPM_PLUS | CARD_MPMX | CARD_MPMRX | CARD_SOFT | CARD_SOFT_NINJA);

    SET_CARDS_SUPPORT_FOR_FEATURE(eFeatureSiren7, CARD_ALL);
}

BOOL CSystemFeatures::IsFeatureSupportedByHardware(const eFeatureName featureName, const eSystemCardsMode systemCardsBasedMode, const eProductType productType) const
{
	BOOL bIsSupported = FALSE;

	switch(systemCardsBasedMode)
	{
		case eSystemCardsMode_mpm:
			bIsSupported = m_arrSupportedFeaturesOnSystem[featureName] & CARD_MPM;
			break;
		case eSystemCardsMode_mpm_plus:
			bIsSupported = m_arrSupportedFeaturesOnSystem[featureName] & CARD_MPM_PLUS;
			break;
		case eSystemCardsMode_breeze:
			if (productType == eProductTypeSoftMCU || productType == eProductTypeGesher || productType == eProductTypeSoftMCUMfw || productType == eProductTypeEdgeAxis)
			{
				bIsSupported = m_arrSupportedFeaturesOnSystem[featureName] & CARD_SOFT;
			}
			else
			{
				bIsSupported = m_arrSupportedFeaturesOnSystem[featureName] & CARD_MPMX;
			}
			break;
		case eSystemCardsMode_mpmrx:
			if (productType == eProductTypeCallGeneratorSoftMCU)
			{
				bIsSupported = m_arrSupportedFeaturesOnSystem[featureName] & CARD_SOFT_CG;
			}
			else if (productType == eProductTypeNinja)
			{
				bIsSupported = m_arrSupportedFeaturesOnSystem[featureName] & CARD_SOFT_NINJA;
			}
			else
			{
				bIsSupported = m_arrSupportedFeaturesOnSystem[featureName] & CARD_MPMRX;
			}
			break;
		case eSystemCardsMode_illegal:
		default:
			PASSERTMSG(1, "CSystemFeatures::IsFeatureSupportedByHardware - Illegal system cards mode!");
			break;
	}

	return bIsSupported;
}

