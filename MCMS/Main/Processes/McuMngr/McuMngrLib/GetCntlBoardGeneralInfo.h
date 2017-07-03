#ifndef __GET_CNTL_BOARD_GENERAL_INFO_H__
#define __GET_CNTL_BOARD_GENERAL_INFO_H__

#include <stddef.h>

struct CntlBoardGeneralInfo
{
    char name[32];
    char hwver[64];
    char swver[64];
    char serial[64];
    char partnumber[64];
    char riserCardCpldVersion[64];
};

void GetCntlBoardGeneralInfo(CntlBoardGeneralInfo & info);
char const * GetCntlBoardName(char * bufName, size_t len);

#endif

