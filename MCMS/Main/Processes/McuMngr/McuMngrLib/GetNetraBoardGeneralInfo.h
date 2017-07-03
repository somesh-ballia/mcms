#ifndef __GET_NETRA_BOARD_GENERAL_INFO_H__
#define __GET_NETRA_BOARD_GENERAL_INFO_H__
#include <stddef.h>

struct NetraBoardGeneralInfo
{
 char name[32];
 char hwver[64];
 char swver[64];
 char serial[64];
 char riserCardCpldVersion[64];
};

void GetNetraBoardGeneralInfo(NetraBoardGeneralInfo& info, int boadid);

#endif
