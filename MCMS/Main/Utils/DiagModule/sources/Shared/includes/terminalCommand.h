#ifndef TERMINALCOMMAND_H_
#define TERMINALCOMMAND_H_


#include <stdarg.h>
#include "LinuxSystemCallsApi.h"
#include "timers.h"
#include "EmaApi.h"
// #include "general.h"


#define MAX_NUM_OF_PARAM 6
#define MAX_PRINT_TO_MCMS_TERMINAL_SIZE  16384

#include "Print.h"
#include "LinuxSystemCallsApi.h"
#include "McmsApi.h"
#include "MplMcmsStructs.h"
#include "SocketApiTypes.h"
#include "DbgCfg.h"
#include "TraceHeader.h"
#include "OutsideEntities.h"

extern t_TcpConnParams TcpConnection[eMaxTcpConnections];
//extern UINT32 pQMainPalBaseAddr;
#define SHVL_BASE_OFFSET	0x10000 // 64K

// Turn ON/OFF SSH mode request from the switch
#define SSH_MODE_REQUEST_OPCODE			0x77	

typedef struct
{
	TEmaReqHeader		tEmaReqHeader;
	APIUBOOL			isOnRequest;
}SWITCH_MPL_API_REQUEST_S;

#endif
