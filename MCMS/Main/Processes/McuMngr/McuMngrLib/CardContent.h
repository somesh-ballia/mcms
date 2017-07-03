#ifndef __CARD_TYPE_H__
#define __CARD_TYPE_H__

#include <vector>
using std::vector;

typedef struct EntitySummaryList
{
} EntitySummaryList;

typedef struct CardContent
{
    int temperature;
    int slotID;
    int status;
    int ipmbAddress;
    int voltage;
    int numMezzanine;
    int subBoardId;
    char cardType[32];
    int bitFail;
    EntitySummaryList entitySummaryList;
} CardContent;

typedef int (*CollectCardContentsType)(vector<CardContent> & cards);

extern CollectCardContentsType CollectCardContents;

#endif

