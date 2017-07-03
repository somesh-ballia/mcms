#ifndef __dsp_res_mgr_update__
#define __dsp_res_mgr_update__


void NetraDspMonitorUpdateTemperature(int slotId, int dspId, unsigned int temperature);
void NetraDspMonitorUpdateHardwareVersion(int slotId, int dspId, const char *hardVersion);
void NetraDspMonitorUpdateSoftwareVersion(int slotId, int dspId, const char *softVersion);
void NetraDspMonitorUpdateCpuUsage(int slotId, int dspId, unsigned int cpuUsage);
void NetraDspMonitorUpdateMemoryUsage(int slotId, int dspId, unsigned int memoryUsage);

void NetraDspMonitorUpdateIsOccupied(int slotId, int dspId, bool isOccupied);
void NetraDspMonitorUpdateIsFaulty(int slotId, int dspId, int isFaulty);
void NetraDspMonitorUpdatePercentOccupied(int slotId, int dspId, int percentOccupied);

void NetraDspMonitorUpdateConfId(int slotId, int dspId, int portId, int confId);
void NetraDspMonitorUpdatePartyId(int slotId, int dspId, int portId, int partyId);
void NetraDspMonitorUpdateIsActive(int slotId, int dspId, int portId, bool isActive);
void NetraDspMonitorUpdatePercentOccupied(int slotId, int dspId, int portId, int percentOccupied);

#endif // __dsp_res_mgr_update__

