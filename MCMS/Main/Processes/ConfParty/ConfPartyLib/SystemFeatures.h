// SystemFeatures.h
#ifndef SYSTEMFEATURES_H_
#define SYSTEMFEATURES_H_

#include "CardsStructs.h"
#include "PObject.h"
#include "ProductType.h"

enum eFeatureName
{
	eFeatureCOP = 0,
	eFeatureRtv,
	eFeatureTip,
	eFeatureH264HighProfile,
	eFeatureEncoderRecurrentIntra,
	eFeatureDecoderRecurrentIntra,
	eFeatureH264PacketizationMode,
	eFeatureIndicationOnLayout_PacketLost,
	eFeatureLegacyContentHD1080,
	eFeatureEchoSuppression,
	eFeatureKeyboardNoiseSuppression,
	eFeatureFontTypes,
	eFeatureHD1080p60Asymmetric,
	eFeatureHD1080p60Symmetric,
	eFeatureContentTranscoding,
	eFeatureWeightedArtMpmRx,
	eFeatureHD1080p30Content,
	eFeatureHD1080p60Content,
	eFeatureLineRate_6M,
	eFeatureMs2013SVC,
	eFeatureRssDialin, 	// EE-471
	eFeatureSiren7, 	// Dell

	// Always LAST
	eFeatureLast
};

class CSystemFeatures : public CPObject
{
	CLASS_TYPE_1(CSystemFeatures, CPObject)

public:
	CSystemFeatures();
	~CSystemFeatures() {}

	virtual const char* NameOf() const { return "CSystemFeatures";}

	BOOL IsFeatureSupportedByHardware(const eFeatureName featureName, const eSystemCardsMode systemCardsBasedMode, const eProductType productType) const;

private:
	void InitSupportedFeaturesOnSystemArray();

	WORD m_arrSupportedFeaturesOnSystem[eFeatureLast];
};

#endif /* SYSTEMFEATURES_H_ */
