#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "DiagDataTypes.h"
#include "SharedDefines.h"
#include "GetEthBondStatus.h"
#include "IpmiSensorEnums.h"

char const * const BONDING_PROC_PATH = "/proc/net/bonding";

inline char const * GetBond0ProcFileName(char *bufFilePath, size_t len, char const * name)
{
    snprintf(bufFilePath, len, "%s/%s", BONDING_PROC_PATH, name);
    return bufFilePath;
}

BOOL IsBondEnabled(char const * name)
{
    char bufCmdLine[128];
    BOOL ret = FALSE;
    snprintf(bufCmdLine, sizeof(bufCmdLine), "cat %s 2>/dev/null | grep 'Currently Active Slave:'", name);
    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        goto done;
    }

    char bufContent[128];
    if (fread(bufContent, 1, sizeof(bufContent), fp)>0)
    {
        ret = TRUE;
        goto done;
    }
    else
    {
        ret = FALSE;
        goto done;
    }
	
done:
	if(fp) pclose(fp);
	return ret;
}

inline int ParseEthNoFromLine(char * line, int * ethNo)
{
    char * ethStr = strstr(line, "eth");
    if (!ethStr)
    {
        return -1;
    }

    char * pEthNo = ethStr + strlen("eth");
    if (0==pEthNo[0])
    {
        return -1;
    }

    *ethNo = atoi(pEthNo);
    return 0;
}

int GetActiveBondingSlave(char const * name, EthBondingInfo * info)
{
    char bufCmdLine[128];
    int ret = -1;
    snprintf(bufCmdLine, sizeof(bufCmdLine), "cat %s 2>/dev/null | grep 'Currently Active Slave:'", name);
    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        goto done;
    }

    char bufContent[128] = {0};
    size_t const bytes = fread(bufContent, 1, sizeof(bufContent) - 1, fp);
    if (bytes<=0)
    {
        goto done;
    }

    if (-1==ParseEthNoFromLine(bufContent, &(info->activeEthNo)))
    {
        goto done;
    }
    ret = 0;
done:
    if(fp) pclose(fp);
    return ret;
}

int GetAllBondingSlaves(char const * name, EthBondingInfo * info)
{
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "cat %s 2>/dev/null | grep 'Slave Interface:'", name);
    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        return -1;
    }

    char line[128];
    info->slaveEthCount = 0;
    while (NULL!=fgets(line, sizeof(line)-1, fp))
    {
        if (0==ParseEthNoFromLine(line, &(info->slaveEthNos[info->slaveEthCount])))
        {
            ++(info->slaveEthCount);
        }
    }

    if(fp) pclose(fp);
    return 0;
}

void GetBackupBondingSlaves(EthBondingInfo * info)
{
    int const activeSlaveNo = info->activeEthNo;
    int i,j;
    for (i=0; i<info->slaveEthCount; ++i)
    {
        if (activeSlaveNo==info->slaveEthNos[i])
        {
            for (j=i; j<info->slaveEthCount-1; ++j)
            {
                info->slaveEthNos[j] = info->slaveEthNos[j+1];
            }
            --(info->slaveEthCount);
            break;
        }
    }

    return;
}

int GetEthBondingInfo(EthBondingInfo * info)
{
    bzero(info, sizeof(EthBondingInfo));
    char const * bond0Name = "bond0";
    char bufName[128];
    GetBond0ProcFileName(bufName, sizeof(bufName), bond0Name);
    if (!IsBondEnabled(bufName))
    {
        return -1;
    }

    info->isBondingEnabled = TRUE;


    if (-1==GetActiveBondingSlave(bufName, info))
    {
        return -1;
    }

    if (-1==GetAllBondingSlaves(bufName, info))
    {
        return -1;
    }

    GetBackupBondingSlaves(info);

    return 0;
}

BOOL IsInBondingSlaves(int lanNo, EthBondingInfo * info)
{
    int i;
    for (i=0; i<info->slaveEthCount; ++i)
    {
        if (info->slaveEthNos[i]==lanNo)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int GetLinkStatusWithBondingInfo(int lanNo, BOOL isLinkUp, EthBondingInfo * info)
{
    if (!isLinkUp)
    {
        return LAN_STATUS_INACTIVE;
    }

    if (info->isBondingEnabled)
    {
        if (IsInBondingSlaves(lanNo, info))
        {
            return LAN_STATUS_STANDBY;
        }
        else
        {
            return LAN_STATUS_ACTIVE;
        }
    }
    else
    {
        return LAN_STATUS_ACTIVE;
    }
}

