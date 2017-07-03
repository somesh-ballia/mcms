#ifndef __GET_IF_LIST_H__
#define __GET_IF_LIST_H__

#include <vector>
#include <string>
using std::vector;

typedef struct IFInfo
{
    int index;
    char name[8];
} IFInfo;

void GetIFList(vector<IFInfo> & ifs);

#endif

