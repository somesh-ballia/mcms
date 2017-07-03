#ifndef __CARD_TYPE_H__
#define __CARD_TYPE_H__

#include "IpmiHandleThread.h"

typedef int (*CollectCardContentsType)(TIpmiEntityListData * entityList);

extern CollectCardContentsType CollectCardContents;

extern  int UpdateFanLevel(TIpmiFanLevel * FanLevelList);

extern void GetSensorList(int slotID, TIpmiSensorList* info);
extern void GetSensorReadingList(int slotID, TIpmiSensorReadingList* info);
#endif

