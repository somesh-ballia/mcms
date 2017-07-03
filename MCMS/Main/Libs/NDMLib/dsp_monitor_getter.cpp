#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "dsp_element.h"
#include "dsp_monitor.h"
#include "dsp_monitor_getter.h"

using NetraDspMonitor::DspEleId;
using NetraDspMonitor::DspElement;


int GetDspMonitorStatus(DSPMonitorBoardList & board_list)
{
    for (int i = 0; i < DSPMONITOR_MAX_BOARD_LIST_LEN; i++)
    {
        int dspIdOfmaxTemperature = 0, maxTemperature = -273;
        for (int j = 0; j < DSPMONITOR_MAX_DSP_LIST_LEN; j++)
        {
            int temperature;
            if ((temperature = atoi(DspElement(NDM_TEMPERATURE, DspEleId(i, j)).GetValue())) > maxTemperature)
            {
                maxTemperature = temperature;
                dspIdOfmaxTemperature = j;
            }
        }
        NDMStrSafeCopy(board_list.status[i].temperature, DspElement(NDM_TEMPERATURE, DspEleId(i, dspIdOfmaxTemperature)).GetValue(), NDM_TEMPE_LEN);
        NDMStrSafeCopy(board_list.status[i].voltage, "null", 10);
        board_list.status[i].boardId = i;
    }
    board_list.len = DSPMONITOR_MAX_BOARD_LIST_LEN;
    return 0;
}

int GetDspMonitorStatus(DSPMonitorDspList & dsp_list, int boardId)
{
    if (!NDMCheckValidBoardId(boardId))
    {
        return -1;
    }

    for (int i = 0; i < DSPMONITOR_MAX_DSP_LIST_LEN; i++)
    {
        dsp_list.status[i].isOccupied = false;
        if (!strcmp(DspElement(NDM_IS_OCCUPIED, DspEleId(boardId, i)).GetValue(), "true"))
        {
            dsp_list.status[i].isOccupied = true;
        }
        else
        {
            dsp_list.status[i].isOccupied = false;
        }

        dsp_list.status[i].isFaulty = atoi(DspElement(NDM_IS_FAULTY, DspEleId(boardId, i)).GetValue());

        dsp_list.status[i].percentOccupied = atoi(DspElement(NDM_PERCENT_OCCUPIED, DspEleId(boardId, i)).GetValue());

        dsp_list.status[i].dspId = i;
    }
    dsp_list.len = DSPMONITOR_MAX_DSP_LIST_LEN;
    return 0;
}

int GetDspMonitorStatus(DSPMonitorPortList & port_list, int boardId, int dspId)
{
    if (!NDMCheckValidBoardId(boardId))
    {
        return -1;
    }

    if (!NDMCheckValidDspId(dspId))
    {
        return -2;
    }

    for (int i = 0; i < DSPMONITOR_MAX_PORT_LIST_LEN; i++)
    {
        port_list.status[i].isActive = false;
        if (!strcmp(DspElement(NDM_IS_ACTIVE, DspEleId(boardId, dspId, i)).GetValue(), "true"))
        {
            port_list.status[i].isActive = true;
        }
        port_list.status[i].percentOccupied = atoi(DspElement(NDM_PERCENT_OCCUPIED, DspEleId(boardId, dspId, i)).GetValue());
        port_list.status[i].confRsrcID = (unsigned int)atoi(DspElement(NDM_CONF_ID, DspEleId(boardId, dspId, i)).GetValue());
        port_list.status[i].partyRsrcID = (unsigned int)atoi(DspElement(NDM_PARTY_ID, DspEleId(boardId, dspId, i)).GetValue());
        port_list.status[i].portId = i;
    }
    port_list.len = DSPMONITOR_MAX_PORT_LIST_LEN;
    return 0;
}

#define DSP_ID_FOR_BOARD_VERSION 0
#define DSP_ID_FOR_CPLD_VERSION 1

int GetDspMonitorStatus(DSPMonitorGeneralList & general_list)
{
    for (int i = 0; i < DSPMONITOR_MAX_BOARD_LIST_LEN; i++)
    {
        NDMStrSafeCopy(general_list.status[i].boardName, "null", 128);
        NDMStrSafeCopy(general_list.status[i].hardwareVersion, DspElement(NDM_HARDWARE_VERSION, DspEleId(i, DSP_ID_FOR_BOARD_VERSION)).GetValue(), NDM_HARDV_LEN);
        NDMStrSafeCopy(general_list.status[i].softwareVersion, DspElement(NDM_SOFTWARE_VERISON, DspEleId(i, DSP_ID_FOR_BOARD_VERSION)).GetValue(), NDM_SOFTV_LEN);
        NDMStrSafeCopy(general_list.status[i].serialNumber, "null", 64);
        
        general_list.status[i].partNumber = -1; 
        general_list.status[i].boardId = i;

        if(i == RTM_ISDN_BOARD_SLOT_ID)
        {
            NDMStrSafeCopy(general_list.status[i].riserCardCpldVersion, DspElement(NDM_SOFTWARE_VERISON, DspEleId(i, DSP_ID_FOR_CPLD_VERSION)).GetValue(), NDM_SOFTV_LEN);
        }
        else
        {
            NDMStrSafeCopy(general_list.status[i].riserCardCpldVersion, "-", 64);
        }
    }
    general_list.len = DSPMONITOR_MAX_BOARD_LIST_LEN;
    return 0;
}

char * NDMStrSafeCopy(char *dst, const char *src, int max_dst_len)
{
    memset(dst, 0, max_dst_len);
    int len2copy = (int)strlen(src) < max_dst_len - 1 ? strlen(src) : max_dst_len - 1;
    memcpy(dst, src, len2copy);
    return dst;
}

bool NDMCheckValidBoardId(int boardId)
{
    if (boardId >= DSPMONITOR_MAX_BOARD_LIST_LEN || boardId < 0)
    {
        return false;
    }
    return true;
}

bool NDMCheckValidDspId(int dspId)
{
    if (dspId >= DSPMONITOR_MAX_DSP_LIST_LEN || dspId < 0)
    {
        return false;
    }
    return true;
}

bool NDMCheckValidPortId(int portId)
{
    if (portId >= DSPMONITOR_MAX_PORT_LIST_LEN || portId < 0)
    {
        return false;
    }
    return true;
}

// Below Functions Only been used in VMP
int GetDspMonitorTemperature(int dspIndex, int &temperature)
{
    int boardIndex = dspIndex / DSPMONITOR_MAX_DSP_LIST_LEN,
        dspIndexEveryBoard = dspIndex % DSPMONITOR_MAX_DSP_LIST_LEN;

    temperature = atoi(DspElement(NDM_TEMPERATURE, DspEleId(boardIndex, dspIndexEveryBoard)).GetValue());
    return 0;
}
// End
