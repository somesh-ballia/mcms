#ifndef __DSP_MONITOR_GETTER_H__
#define __DSP_MONITOR_GETTER_H__

#include "dsp_monitor.h"

enum
{
    RTM_ISDN_BOARD_SLOT_ID = 3
};

enum
{
      DSP_STAT_ALIVE = 0
    , DSP_STAT_FAULTY = 1
    , DSP_STAT_DEAD = 2
};

struct DSPMonitorBoardList
{
    struct
    {
        int boardId;
        char temperature[NDM_TEMPE_LEN]; // Highest temperature
        char voltage[10]; // not supported
    } status[DSPMONITOR_MAX_BOARD_LIST_LEN];

    int len;
};


struct DSPMonitorDspList
{
    struct
    {
        int dspId;
        bool isOccupied;
        int isFaulty;
        int percentOccupied;
    } status[DSPMONITOR_MAX_DSP_LIST_LEN];

    int len;
};

struct DSPMonitorPortList
{
    struct
    {
        int             portId;
        unsigned  int   confRsrcID;
        unsigned  int   partyRsrcID;
        bool            isActive;
        int             percentOccupied;
    } status[DSPMONITOR_MAX_PORT_LIST_LEN];

    int len;
};

struct DSPMonitorGeneralList
{
    struct DSPGeneral
    {
        int  boardId;
        char hardwareVersion[NDM_HARDV_LEN];
        char softwareVersion[NDM_SOFTV_LEN];
        
        char boardName[128]; // not supported
        char serialNumber[64]; // not supported
        int  partNumber; // not supported
        char riserCardCpldVersion[64];
    } status[DSPMONITOR_MAX_BOARD_LIST_LEN];

    int len;
};


int GetDspMonitorStatus(DSPMonitorBoardList & dsp_board_list);
int GetDspMonitorStatus(DSPMonitorDspList & dsp_dsp_list, int boardId);
int GetDspMonitorStatus(DSPMonitorPortList & dsp_port_list, int boardId, int dspId);
int GetDspMonitorStatus(DSPMonitorGeneralList & general_list);

// Below Functions Only been used in VMP
int GetDspMonitorTemperature(int dspIndex, int &temperature);
// End
#endif // __DSP_MONITOR_GETTER_H__

