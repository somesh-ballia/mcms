#ifndef __GET_IF_FEATURES_H__
#define __GET_IF_FEATURES_H__

typedef enum AutoNegStatus
{
    LINK_AUTONEG_DISABLE = 0,
    LINK_AUTONEG_ENABLE = 1
} AutoNegStatus;

enum
{
      BASE_10_HALF = 1<<0
    , BASE_10_FULL = 1<<1
    , BASE_100_HALF = 1<<2
    , BASE_100_FULL = 1<<3
    , BASE_1000_HALF = 1<<4
    , BASE_1000_FULL = 1<<5
};

typedef struct
{
    unsigned int advertisedLinkMode;
    unsigned int advertisedAutoNeg;

    AutoNegStatus activeAutoNeg;
    unsigned int activeLinkMode;
} IFFeatures;

typedef bool (*GetIFFeaturesFunc)(int ethNo, IFFeatures *iff);
extern GetIFFeaturesFunc GetIFFeatures;

typedef bool (*IsLinkUpFunc)(int ethNo,int & linkUp);
extern IsLinkUpFunc IsLinkUp;


#endif

