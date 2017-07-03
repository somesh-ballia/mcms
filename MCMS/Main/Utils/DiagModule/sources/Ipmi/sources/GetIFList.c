#include "DiagDataTypes.h"
#include "SharedDefines.h"
#include "GetIFList.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "GetEthBondStatus.h"
#include "GetIFFeatures.h"
#include "IpmiSensorEnums.h"
#include "IpmiEntitySlotIDs.h"
#include "GetIFStatistics.h"

void GetIFList(IFInfo * ifs, unsigned int size, unsigned int * count)
{
    
    char const * const s_procDevFile = "/proc/net/dev";
    char line[1024];
    char * ptr;
    unsigned int cur_index = 0;
    *count = 0;
    FILE * fp = fopen(s_procDevFile, "r");
    if (!fp) goto done;

   
    while (fgets(line, sizeof(line), fp) != NULL)
    {
	  if((ptr = strstr(line, "eth")) != NULL)
	  {
	       if(cur_index < size)
		{
			memcpy(ifs[cur_index].name, ptr, 4);
			ifs[cur_index].name[4] = '\0';
			ifs[cur_index].index = ifs[cur_index].name[3] - '0';
			cur_index++;
		}
	  }
    }
    *count = cur_index;
	
done:
    if(fp) fclose(fp);
    return;
}

void GetLanStatPortsList(TLanStatGetPortsList* ptLanStatGetPortsList)
{
    IFInfo ifs[MAX_NIC_COUNT];
    unsigned int i=0;
    unsigned int count = 0;
    GetIFList(ifs, MAX_NIC_COUNT, &count);

    EthBondingInfo bondingInfo;
    GetEthBondingInfo(&bondingInfo);

    ptLanStatGetPortsList->ulNumOfElem = 0;
    ptLanStatGetPortsList->ulNumOfElemFields = 3;
    for (i=0; i<count; ++i)
    {

        int const ifIndex = ifs[i].index;
        
        BOOL isLinkUp = FALSE;
        if ((*IsLinkUp)(ifIndex, &isLinkUp))
        {
            ptLanStatGetPortsList->tPorts[ptLanStatGetPortsList->ulNumOfElem].unStatus = GetLinkStatusWithBondingInfo(ifIndex, isLinkUp, &bondingInfo);
        }
        else
        {
            ptLanStatGetPortsList->tPorts[ptLanStatGetPortsList->ulNumOfElem].unStatus = LAN_STATUS_INACTIVE;
        }
        ptLanStatGetPortsList->tPorts[ptLanStatGetPortsList->ulNumOfElem].unPortID = 0;
        ptLanStatGetPortsList->tPorts[ptLanStatGetPortsList->ulNumOfElem].unSlotID = LAN_SLOT_ID_START+ifIndex;

        ptLanStatGetPortsList->ulNumOfElem++;
    }
}

void GetLanStatInfo(TLanStatInfo* ptLanStatInfo, int lanNo)
{
	bzero(ptLanStatInfo, sizeof(TLanStatInfo));

	EthBondingInfo bondingInfo;
	GetEthBondingInfo(&bondingInfo);

	{
	    IFFeatures ifFeatures;
	    (*GetIFFeatures)(lanNo, &ifFeatures);

	    ptLanStatInfo->unActLinkAutoNeg= ifFeatures.activeAutoNeg;
	    ptLanStatInfo->unActLinkMode= ifFeatures.activeLinkMode;
	    BOOL isLinkUp = FALSE;
	    if ((*IsLinkUp)(lanNo, &isLinkUp))
	    {
	        ptLanStatInfo->unActLinkStatus = GetLinkStatusWithBondingInfo(lanNo, isLinkUp, &bondingInfo);
	    }
	    else
	    {
	        ptLanStatInfo->unActLinkStatus = LAN_STATUS_INACTIVE;
	    }

	    ptLanStatInfo->unAdvLinkAutoNeg = ifFeatures.advertisedAutoNeg;
	    ptLanStatInfo->unAdvLinkMode = ifFeatures.advertisedLinkMode;
	}

	{
	    IFStatistics ifStatistics;
	    GetIFStatistics(lanNo, &ifStatistics);

	    strncpy(ptLanStatInfo->unTxOctets, ifStatistics.bytesSent, sizeof(ptLanStatInfo->unTxOctets) - 1);
	    strncpy(ptLanStatInfo->unMaxTxOctets, ifStatistics.bytesSent, sizeof(ptLanStatInfo->unMaxTxOctets) - 1);
	    strncpy(ptLanStatInfo->unTxPackets, ifStatistics.pktsSent, sizeof(ptLanStatInfo->unTxPackets) - 1);
	    strncpy(ptLanStatInfo->unMaxTxPackets, ifStatistics.pktsSent, sizeof(ptLanStatInfo->unMaxTxPackets) - 1);
	    strncpy(ptLanStatInfo->unTxBadPackets, ifStatistics.pktsSentError, sizeof(ptLanStatInfo->unTxBadPackets) - 1);
	    strncpy(ptLanStatInfo->unMaxTxBadPackets, ifStatistics.pktsSentError, sizeof(ptLanStatInfo->unMaxTxBadPackets) - 1);
	    strncpy(ptLanStatInfo->unTxFifoDrops, ifStatistics.pktsSentFifoError, sizeof(ptLanStatInfo->unTxFifoDrops) - 1);
	    strncpy(ptLanStatInfo->unMaxTxFifoDrops, ifStatistics.pktsSentFifoError, sizeof(ptLanStatInfo->unMaxTxFifoDrops) - 1);
	    
	    strncpy(ptLanStatInfo->unRxOctets, ifStatistics.bytesRecved, sizeof(ptLanStatInfo->unRxOctets) - 1);
	    strncpy(ptLanStatInfo->unMaxRxOctets, ifStatistics.bytesRecved, sizeof(ptLanStatInfo->unMaxRxOctets) - 1);
	    strncpy(ptLanStatInfo->unRxPackets, ifStatistics.pktsRecved, sizeof(ptLanStatInfo->unRxPackets) - 1);
	    strncpy(ptLanStatInfo->unMaxRxPackets, ifStatistics.pktsRecved, sizeof(ptLanStatInfo->unMaxRxPackets) - 1);
	    strncpy(ptLanStatInfo->unRxBadPackets, ifStatistics.pktsRecvedError, sizeof(ptLanStatInfo->unRxBadPackets) - 1);
	    strncpy(ptLanStatInfo->unMaxRxBadPackets, ifStatistics.pktsRecvedError, sizeof(ptLanStatInfo->unMaxRxBadPackets) - 1);
	    strncpy(ptLanStatInfo->unRxCRC, ifStatistics.pktsRecvedFrameError, sizeof(ptLanStatInfo->unRxCRC) - 1);
	    strncpy(ptLanStatInfo->unMaxRxCRC, ifStatistics.pktsRecvedFrameError, sizeof(ptLanStatInfo->unMaxRxCRC) - 1);
	}
}
