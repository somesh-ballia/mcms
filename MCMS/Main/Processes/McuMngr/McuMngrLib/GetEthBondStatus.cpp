#include "GetEthBondStatus.h"
#include "FileGuard.h"
#include "IpmiSensorEnums.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>

char const * const BONDING_PROC_PATH = "/proc/net/bonding";

inline char const * GetBond0ProcFileName(char *bufFilePath, size_t len, char const * name)
{
    snprintf(bufFilePath, len, "%s/%s", BONDING_PROC_PATH, name);
    return bufFilePath;
}

int IsBondEnabled(char const * name)
{
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "cat %s 2>/dev/null | grep 'Currently Active Slave:'", name);
    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        return false;
    }
    PCloseFile guard(fp);

    char bufContent[128];
    if (fread(bufContent, 1, sizeof(bufContent), fp)>0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline int ParseEthNoFromLine(char * line, int & ethNo)
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

    ethNo = atoi(pEthNo);
    return 0;
}

int GetActiveBondingSlave(char const * name, EthBondingInfo & info)
{
    char bufCmdLine[128]={0};
    snprintf(bufCmdLine, sizeof(bufCmdLine)-1, "cat %s 2>/dev/null | grep 'Currently Active Slave:'", name);
    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        return -1;
    }
    PCloseFile guard(fp);

    char bufContent[128]={0};
    size_t const bytes = fread(bufContent, 1, sizeof(bufContent)-1, fp);
    if (bytes<=0)
    {
        return -1;
    }

    if (-1==ParseEthNoFromLine(bufContent, info.activeEthNo))
    {
        return -1;
    }

    return 0;
}

int GetAllBondingSlaves(char const * name, EthBondingInfo & info)
{
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "cat %s 2>/dev/null | grep 'Slave Interface:'", name);
    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        return -1;
    }
    PCloseFile guard(fp);

    char line[128];
    info.slaveEthCount = 0;
    while (NULL!=fgets(line, sizeof(line)-1, fp))
    {
        if (0==ParseEthNoFromLine(line, info.slaveEthNos[info.slaveEthCount]))
        {
            ++info.slaveEthCount;
        }
    }

    return 0;
}

void GetBackupBondingSlaves(EthBondingInfo & info)
{
    int const activeSlaveNo = info.activeEthNo;
    for (int i=0; i<info.slaveEthCount; ++i)
    {
        if (activeSlaveNo==info.slaveEthNos[i])
        {
            for (int j=i; j<info.slaveEthCount-1; ++j)
            {
                info.slaveEthNos[j] = info.slaveEthNos[j+1];
            }
            --info.slaveEthCount;
            break;
        }
    }

    return;
}

int GetEthBondingInfo(EthBondingInfo & info)
{
    bzero(&info, sizeof(info));
    char const * bond0Name = "bond0";
    char bufName[128];
    GetBond0ProcFileName(bufName, sizeof(bufName), bond0Name);
    if (!IsBondEnabled(bufName))
    {
        return -1;
    }

    info.isBondingEnabled = true;


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

inline int IsInBondingSlaves(int lanNo, EthBondingInfo const & info)
{
    for (int i=0; i<info.slaveEthCount; ++i)
    {
        if (info.slaveEthNos[i]==lanNo)
        {
            return true;
        }
    }

    return false;
}

int GetLinkStatusWithBondingInfo(int lanNo, int isLinkUp, EthBondingInfo const & info)
{
    if (!isLinkUp)
    {
        return LAN_STATUS_INACTIVE;
    }

    if (info.isBondingEnabled)
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

