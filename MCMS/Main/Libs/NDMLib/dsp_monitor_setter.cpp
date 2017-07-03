#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "dsp_monitor_setter.h"
#include "dsp_monitor.h"
#include "dsp_element.h"

#define MAX_STRING_SIZE 1024

using NetraDspMonitor::DspElement;
using NetraDspMonitor::DspEleId;



void NetraDspMonitorUpdateTemperature(int slotId, int dspId, unsigned int temperature)
{
    if (!NDMCheckValidBoardId(slotId) || !NDMCheckValidDspId(dspId))
        return;

    char temperature_str[MAX_STRING_SIZE];
    snprintf(temperature_str, sizeof(temperature_str), "%d", temperature);
    DspElement(NDM_TEMPERATURE, DspEleId(slotId, dspId)).SetValue(temperature_str);
}

void NetraDspMonitorUpdateHardwareVersion(int slotId, int dspId, const char *hardVersion)
{
    if (!NDMCheckValidBoardId(slotId) || !NDMCheckValidDspId(dspId))
        return;

    char hardVersion_str[MAX_STRING_SIZE];
    snprintf(hardVersion_str, sizeof(hardVersion_str), "%s", hardVersion);
    DspElement(NDM_HARDWARE_VERSION, DspEleId(slotId, dspId)).SetValue(hardVersion_str);
}

void NetraDspMonitorUpdateSoftwareVersion(int slotId, int dspId, const char *softVersion)
{
    if (!NDMCheckValidBoardId(slotId) || !NDMCheckValidDspId(dspId))
        return;

    char softVersion_str[MAX_STRING_SIZE];
    snprintf(softVersion_str, sizeof(softVersion_str), "%s", softVersion);
    DspElement(NDM_SOFTWARE_VERISON, DspEleId(slotId, dspId)).SetValue(softVersion_str);
}

void NetraDspMonitorUpdateCpuUsage(int slotId, int dspId, unsigned int cpuUsage)
{
    if (!NDMCheckValidBoardId(slotId) || !NDMCheckValidDspId(dspId))
        return;

    char cpuUsage_str[NDM_CPUUSAGE_LEN];
    snprintf(cpuUsage_str, sizeof(cpuUsage_str), "%u", cpuUsage);
    DspElement(NDM_CPU_USAGE, DspEleId(slotId, dspId)).SetValue(cpuUsage_str);
}

void NetraDspMonitorUpdateMemoryUsage(int slotId, int dspId, unsigned int memoryUsage)
{
    if (!NDMCheckValidBoardId(slotId) || !NDMCheckValidDspId(dspId))
        return;

    char memoryUsage_str[NDM_MEMORYUSAGE_LEN];
    snprintf(memoryUsage_str, sizeof(memoryUsage_str), "%u", memoryUsage);
    DspElement(NDM_MEMORY_USAGE, DspEleId(slotId, dspId)).SetValue(memoryUsage_str);
}



void NetraDspMonitorUpdateIsOccupied(int slotId, int dspId, bool isOccupied)
{
    if (!NDMCheckValidBoardId(slotId) || !NDMCheckValidDspId(dspId))
        return;

    char isOccupied_str[MAX_STRING_SIZE];
    snprintf(isOccupied_str, sizeof(isOccupied_str), "%s", isOccupied ? "true" : "false");
    DspElement(NDM_IS_OCCUPIED, DspEleId(slotId, dspId)).SetValue(isOccupied_str);
}

void NetraDspMonitorUpdateIsFaulty(int slotId, int dspId, int isFaulty)
{
    if (!NDMCheckValidBoardId(slotId)|| !NDMCheckValidDspId(dspId))
        return;

    char isFaulty_str[MAX_STRING_SIZE];
    snprintf(isFaulty_str, sizeof(isFaulty_str), "%d", isFaulty);
    DspElement(NDM_IS_FAULTY, DspEleId(slotId, dspId)).SetValue(isFaulty_str);
}

void NetraDspMonitorUpdatePercentOccupied(int slotId, int dspId, int percentOccupied)
{
    if (!NDMCheckValidBoardId(slotId) || !NDMCheckValidDspId(dspId))
        return;

    char percentOccupied_str[MAX_STRING_SIZE];
    snprintf(percentOccupied_str, sizeof(percentOccupied_str), "%d", percentOccupied);
    DspElement(NDM_PERCENT_OCCUPIED, DspEleId(slotId, dspId)).SetValue(percentOccupied_str);
}

void NetraDspMonitorUpdateConfId(int slotId, int dspId, int portId, int confId)
{
    if (!NDMCheckValidBoardId(slotId) 
        || !NDMCheckValidDspId(dspId)
        || !NDMCheckValidPortId(portId))
        return;

    char confId_str[NDM_CONFID_LEN];
    snprintf(confId_str, sizeof(confId_str), "%d", confId);
    DspElement(NDM_CONF_ID, DspEleId(slotId, dspId, portId)).SetValue(confId_str);
}

void NetraDspMonitorUpdatePartyId(int slotId, int dspId, int portId, int partyId)
{
    if (!NDMCheckValidBoardId(slotId) 
        || !NDMCheckValidDspId(dspId)
        || !NDMCheckValidPortId(portId))
        return;

    char partyId_str[NDM_PARTYID_LEN];
    snprintf(partyId_str, sizeof(partyId_str), "%d", partyId);
    DspElement(NDM_PARTY_ID, DspEleId(slotId, dspId, portId)).SetValue(partyId_str);
}

void NetraDspMonitorUpdateIsActive(int slotId, int dspId, int portId, bool isActive)
{
    if (!NDMCheckValidBoardId(slotId) 
        || !NDMCheckValidDspId(dspId)
        || !NDMCheckValidPortId(portId))
        return;

    char isActive_str[MAX_STRING_SIZE];
    snprintf(isActive_str, sizeof(isActive_str), "%s", isActive ? "true" : "false");
    DspElement(NDM_IS_ACTIVE, DspEleId(slotId, dspId, portId)).SetValue(isActive_str);
}

void NetraDspMonitorUpdatePercentOccupied(int slotId, int dspId, int portId, int percentOccupied)
{
    if (!NDMCheckValidBoardId(slotId) 
        || !NDMCheckValidDspId(dspId)
        || !NDMCheckValidPortId(portId))
        return;

    char percentOccupied_str[MAX_STRING_SIZE];
    snprintf(percentOccupied_str, sizeof(percentOccupied_str), "%d", percentOccupied);
    DspElement(NDM_PERCENT_OCCUPIED, DspEleId(slotId, dspId, portId)).SetValue(percentOccupied_str);
}


