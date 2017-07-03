#ifndef __DSP_MONITOR_DEF_H__
#define __DSP_MONITOR_DEF_H__

#include "CommonStructs.h"

#define DSPMONITOR_MAX_BOARD_LIST_LEN 4 //Netra DSP: 0-2    RTM ISDN: 3
#define DSPMONITOR_MAX_DSP_LIST_LEN 6
#define DSPMONITOR_MAX_PORT_LIST_LEN MAX_VIDEO_PORTS_PER_DSP_NINJA

#define NDM_SUFFIX "ndm"

#define NDM_TEMPERATURE "temperature"
#define NDM_TEMPE_LEN 16
#define NDM_TEMPE_DEFAULT "-273"

#define NDM_HARDWARE_VERSION "hardwareVersion"
#define NDM_HARDV_LEN 64
#define NDM_HARDV_DEFAULT "hv-6-6-6"

#define NDM_SOFTWARE_VERISON "softwareVersion"
#define NDM_SOFTV_LEN 64
#define NDM_SOFTV_DEFAULT "sv-6-6-6"

#define NDM_CPU_USAGE "cpuUsage"
#define NDM_CPUUSAGE_LEN 16
#define NDM_CPUUSAGE_DEFAULT "0"

#define NDM_MEMORY_USAGE "memoryUsage"
#define NDM_MEMORYUSAGE_LEN 16
#define NDM_MEMORYUSAGE_DEFAULT "0"

#define NDM_IS_OCCUPIED "isOccupied"
#define NDM_ISOCD_LEN 16
#define NDM_ISOCD_DEFAULT "false"

#define NDM_IS_FAULTY "isFaulty"
#define NDM_ISFTY_LEN 16
#define NDM_ISFTY_DEFAULT "1"

#define NDM_PERCENT_OCCUPIED "percentOccupied"
#define NDM_PEROC_LEN 16
#define NDM_PEROC_DEFAULT "0"

#define NDM_IS_ACTIVE "isActive"
#define NDM_ISACT_LEN 16
#define NDM_ISACT_DEFAULT "false"

#define NDM_CONF_ID "confRsrcID"
#define NDM_CONFID_LEN 16
#define NDM_CONFID_DEFAULT "0"

#define NDM_PARTY_ID "partyRsrcID"
#define NDM_PARTYID_LEN 16
#define NDM_PARTYID_DEFAULT "0"

const char *dsp_monitor_get_name_suffix_by_id(char *temp, int id);

char * NDMStrSafeCopy(char *dst, const char *src, int max_dst_len);

bool NDMCheckValidBoardId(int boardId);
bool NDMCheckValidDspId(int dspId);
bool NDMCheckValidPortId(int portId);

#endif // __DSP_MONITOR_DEF_H__

