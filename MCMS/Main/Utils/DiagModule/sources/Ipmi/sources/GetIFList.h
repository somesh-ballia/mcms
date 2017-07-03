#ifndef __GET_IF_LIST_H__
#define __GET_IF_LIST_H__
#include "LanHandleThread.h"

typedef struct
{
    int index;
    char name[8];
} IFInfo;

void GetIFList(IFInfo * ifs, unsigned int size, unsigned int * count);
void GetLanStatPortsList(TLanStatGetPortsList* ptLanStatGetPortsList);   
void GetLanStatInfo(TLanStatInfo* ptLanStatInfo, int lanNo);

#endif

