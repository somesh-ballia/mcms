#ifndef __SENSOR_READINGS_TO_CARD_ENTRY_H__
#define __SENSOR_READINGS_TO_CARD_ENTRY_H__

#include "CardContent.h"
#include "sensor_read.h"

enum IpmiCardStatus
{
      IPMI_CARD_STATUS_NORMAL = 0
    , IPMI_CARD_STATUS_MAJOR = 1
    , IPMI_CARD_STATUS_EMPTY = 2
};

enum IpmiMeteorStatus
{
      IPMI_METEOR_STATUS_NA = -1
    , IPMI_METEOR_STATUS_NORMAL = 0
    , IPMI_METEOR_STATUS_MAJOR = 1
    , IPMI_METEOR_STATUS_CRITICAL = 2
};

void SensorReadingsToCardEntry(CardContent & card, vector<sensor_t> const & readings);

IpmiCardStatus GetCardStatus(sensor_t const & reading);
IpmiMeteorStatus GetMeteorStatus(sensor_t const & reading);

#endif

