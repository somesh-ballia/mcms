#ifndef TOOLS_H_
#define TOOLS_H_

#include "DiagDataTypes.h"
#include "SharedDefines.h"

#define NVSTR(i) ((i==NULL)?("NULL"):(i))

extern BOOL g_isServiceRun;
/*
//#define x86_ARCH
#ifdef x86_ARCH 
#define SWAPL(X) X
#define SWAPW(X) X 
#else
#define SWAPL(X) (((X)&0xff)<<24)+(((X)&0xff00)<<8)+(((X)&0xff0000)>>8)+(((X)&0xff000000)>>24)
#define SWAPW(X) (((X)&0xff00)>>8)+(((X)&0xff)<<8)  																																 
#endif
*/
//extern INT8 my_strccpy(INT8 *pDest, INT8 *pSource);
extern void GetIpAddr(UINT8* pucIpAddr , UINT32 unIpAddr);
extern UINT8* GetFilePlace(UINT8 *pSource);
extern UINT32 GetEndOfLineLen(UINT8 *pSource);

extern void ServiceStop();

// opcode (see DiagSwitchEmaOPcode2Str)
// SOCKET_OPERATION_FAILED
// TPCKT_OPERATION_FAILED
// ...

// ulSlotId (see DiagSwitchEmaSlot2Str)
// e_slotIdAll_4000
// e_slotIdMfa1_4000
typedef enum
{
    eRecv,
    eHandle,
    eSend
}eTraceDiagSwitchEmaComLocation;

extern void TraceDiagSwitchEmaCom(eTraceDiagSwitchEmaComLocation place,
                                  UINT32 ulMsgId,
                                  UINT32 opcode,
                                  UINT32 ulSlotId,
                                  const char *message);

extern char *StringTrim(char * bstr, char * estr);

extern int LineSplitTrim(char * line, char *** retbuf, const char * delim);

extern void LineSplitFree(char ** buf, int count);

extern int IsDirFileExist(const char *dir);

#define PATHNAME_NinjaDiagCmdPath	"/usr/rmx1000/bin/ninja_diag/ninja_diag"
#define PATHNAME_NinjaCodecStressCmdPath	"/usr/rmx/bin/eps_simulator"
#define PATHNAME_NinjaRtmDiagCmdPath	"/usr/rmx1000/bin/RtmDiagTestCommand diag"
#define NINJA_CMD_PREFIX			"ninja_diag:"

#define GET_HARDWARE				"GetHardVer"
#define GET_DSP_STATUS				"GetDspStatus"

extern char g_boardType[];
extern char g_hwVersion[];

#define CMD_START_VIDEO         "/usr/rmx1000/bin/video &>> /tmp/diag_video.log &"
#define CMD_STOP_VIDEO          "killall video"
#define CMD_START_RTMDIAGSERVER         "/usr/rmx1000/bin/RtmDiagTestCommand &>> /tmp/diag_rtm.log &"
#define CMD_STOP_RTMDIAGSERVER          "killall RtmDiagTestCommand"

extern BOOL SystemCommand(const char * system_command, char * output, int size);
extern UINT32 CovnSlotIDToHW(UINT32 slotId);
extern UINT32 CovnUnitIdToHW(UINT32 slotId, int unitid);
extern int ExecNinjaDiagCmd(char *** szOutArray, int * pCount, const char * cmdPath, const char * command, const char * param1);
extern int InitCardHWVersion();
extern int InitDspStatus();
extern BOOL isValidCard(UINT32 ulSlotId);
extern BOOL isValidDsp(UINT32 ulSlotId, UINT32 unitId);
extern void InitRtmIsdnDspStatus();
extern BOOL isValidRtmIsdnCard(UINT32 ulSlotId);
extern BOOL isValidRtmIsdnDSP(UINT32 ulSlotId, UINT32 unitId);

extern void LedDiagInProgress();
extern void LedDiagComplete();

extern BOOL RetrieveIpV6Address(char * ipv6GlobalAddr, unsigned int len);
extern void StartRtmDiagServer();
extern void StopRtmDiagServer();

#endif /*TOOLS_H_*/
