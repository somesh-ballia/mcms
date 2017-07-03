#ifndef __GET_CNTL_BOARD_GENERAL_INFO_H__
#define __GET_CNTL_BOARD_GENERAL_INFO_H__

#include <stddef.h>
#include "IpmiHandleThread.h"

void GetCntlBoardGeneralInfo(TIpmiFru* info);
char const * GetCntlBoardName(char * bufName, size_t len);
void GetDSPCardGeneralInfo(TIpmiFru* info);
void GetRTMBoardGeneralInfo(TIpmiFru* info);

#endif

