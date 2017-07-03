#include "GetIFStatus.h"
#include "SocketGuard.h"
#include "FileGuard.h"
#include <iostream>
#include <fstream>
#include <string>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>

namespace
{
    bool GetIFFlags(char const *devName, short int &ifFlags)
    {
        int const s = socket(AF_INET, SOCK_DGRAM,0);
        if (s<0)
            return false;
        CloseSocket cs(s);

        struct ifreq ifr;
        bzero(&ifr, sizeof(ifr));
        snprintf(ifr.ifr_name,sizeof(ifr.ifr_name)-1, "%s", devName);
        int const err = ioctl(s, SIOCGIFFLAGS, &ifr);
        if (err<0)
        {
            return false;
        }

        ifFlags = ifr.ifr_flags;
        return true;
    }
}

bool IsIFUpByFunctionCall(char const * name)
{
    short int ifFlags = 0;
    if (GetIFFlags(name, ifFlags))
    {
        bool const isIFUp = 0!=(ifFlags&IFF_UP);
        return isIFUp;
    }
    else
    {
        return false;
    }
}

bool IsIFUpByCmdLine(char const * name)
{
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "ifconfig %s", name);

    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        return false;
    }

    PCloseFile guard(fp);

    char line[512];
    while (NULL!=fgets(line, sizeof(line)-1, fp))
    {
        if (strstr(line, "UP"))
        {
            return true;
        }
    }
    
    return false;
}

IsIFUpFunc IsIFUp = IsIFUpByCmdLine;


