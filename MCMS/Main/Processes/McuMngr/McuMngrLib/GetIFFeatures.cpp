#include "GetIFFeatures.h"
#include "FileGuard.h"
#include <iostream>
#include <fstream>
#include <string>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>

#include <linux/if.h>
#include <linux/sockios.h>
typedef unsigned long long u64;
typedef __uint32_t u32;
typedef __uint16_t u16;
typedef __uint8_t u8;
typedef unsigned long long __u64;
typedef __uint32_t __u32;
typedef __uint16_t __u16;
typedef __uint8_t __u8;
#include <linux/ethtool.h>
#include "SocketGuard.h"

using std::string;

namespace
{
struct IFCap
{
    int speed;
    int duplex;
    unsigned linkMode;
} const s_ifcaps[] =
{
      { SPEED_10, DUPLEX_HALF, BASE_10_HALF }
    , { SPEED_10, DUPLEX_FULL, BASE_10_FULL }
    , { SPEED_100, DUPLEX_HALF, BASE_100_HALF }
    , { SPEED_100, DUPLEX_FULL, BASE_100_FULL }
    , { SPEED_1000, DUPLEX_HALF, BASE_1000_HALF }
    , { SPEED_1000, DUPLEX_FULL, BASE_1000_FULL }
};
unsigned GetLinkMode(int speed, int duplex)
{
    for (unsigned i=0; i<sizeof(s_ifcaps)/sizeof(0[s_ifcaps]); ++i)
    {
        IFCap const & ifcap = s_ifcaps[i];
        if ((ifcap.speed==speed) && (ifcap.duplex==duplex))
        {
            return ifcap.linkMode;
        }
    }
    return BASE_10_HALF;
}
}

bool GetIFFeaturesByFunctionCall(int ethNo, IFFeatures *iff)
{
    bzero(iff, sizeof(IFFeatures));
    
    if (ethNo<0 || ethNo>9)
    {
        return false;
    }

    int const s = socket(AF_INET, SOCK_DGRAM,0);
    if (s<0)
    {
        return false;
    }
    
    CloseSocket cs(s);
    
    char devName[32];
    bzero(devName, sizeof(devName));
    sprintf(devName, "eth%1d", ethNo);
    struct ifreq ifr;
    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, devName,sizeof(ifr.ifr_name)-1);

    struct ethtool_cmd ecmd;
    bzero(&ecmd, sizeof(ecmd));
    ecmd.cmd = ETHTOOL_GSET;
    ifr.ifr_data = (caddr_t)&ecmd;

    int const err = ioctl(s, SIOCETHTOOL, &ifr);
    if (err < 0)
    {
        return false;
    }
    else
    {
        unsigned int const mask = ecmd.supported;
        if (mask & SUPPORTED_10baseT_Half)
            iff->advertisedLinkMode |= BASE_10_HALF;
        if (mask & SUPPORTED_10baseT_Full)
            iff->advertisedLinkMode |= BASE_10_FULL;
        if (mask & SUPPORTED_100baseT_Half)
            iff->advertisedLinkMode |= BASE_100_HALF;
        if (mask & SUPPORTED_100baseT_Full)
            iff->advertisedLinkMode |= BASE_100_FULL;
        if (mask & SUPPORTED_1000baseT_Half)
            iff->advertisedLinkMode |= BASE_1000_HALF;
        if (mask & SUPPORTED_1000baseT_Full)
            iff->advertisedLinkMode |= BASE_1000_FULL;
        
        if (mask & SUPPORTED_Autoneg)
            iff->advertisedAutoNeg = 1;

        iff->activeLinkMode = GetLinkMode(ecmd.speed, ecmd.duplex);
        
        switch (ecmd.autoneg)
        {
        case AUTONEG_ENABLE:
            iff->activeAutoNeg = LINK_AUTONEG_ENABLE;
            break;
        default:
            iff->activeAutoNeg = LINK_AUTONEG_DISABLE;
            break;
        }
    }

    return true;
}

bool IsLinkUpByFunctionCall(int ethNo, int & linkUp)
{
    linkUp = 0;
    if (ethNo<0 || ethNo>9)
        return false;

    int const s = socket(AF_INET, SOCK_DGRAM,0);
    if (s<0)
        return false;
    CloseSocket cs(s);

    char devName[32];
    bzero(devName, sizeof(devName));
    sprintf(devName, "eth%1d", ethNo);
    struct ifreq ifr;
    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, devName,sizeof(ifr.ifr_name)-1);

    struct ethtool_value edata;
    edata.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = (caddr_t)&edata;
    if (ioctl(s, SIOCETHTOOL, &ifr)<0)
        return false;

    linkUp = (0!=edata.data) ? 1 : 0;

    return true;
}

namespace
{
    string ExtractStringByDelim(char const * head, char const * trail, char const * inStr)
    {
        char const * pBeg = strstr(inStr, head);
        if (!pBeg)
        {
            return "";
        }

        char const * pEnd = strstr(pBeg, trail);
        if (!pEnd)
        {
            return pBeg;
        }

        return string(pBeg+strlen(head), pEnd-pBeg);
    }

    string ExtractSupportedLinkModes(char const * instr)
    {
        return ExtractStringByDelim("Supported link modes:", "Supports auto-negotiation:", instr);
    }

    string ExtractSupportedAutoNegotiation(char const * instr)
    {
        return ExtractStringByDelim("Supports auto-negotiation:", "\n", instr);
    }

    string ExtractAdvertisedLinkModes(char const * instr)
    {
        return ExtractStringByDelim("Advertised link modes:", "Advertised auto-negotiation:", instr);
    }

    string ExtractAdvertisedAutoNegotiation(char const * inStr)
    {
        return ExtractStringByDelim("Advertised auto-negotiation:", "\n", inStr);
    }

    string ExtractLinkSpeed(char const * instr)
    {
        return ExtractStringByDelim("Speed:", "\n", instr);
    }

    string ExtractLinkDuplex(char const * instr)
    {
        return ExtractStringByDelim("Duplex:", "\n", instr);
    }

    string ExtractAutoNegotiation(char const * instr)
    {
        return ExtractStringByDelim("Auto-negotiation:", "\n", instr);
    }

    struct LinkModStrVal 
    {
        char const * str;
        unsigned int val;
    }
    const s_linkModes[] =
    {
          { "10baseT/Half", BASE_10_HALF }
        , { "10baseT/Full", BASE_10_FULL}
        , { "100baseT/Half", BASE_100_HALF }
        , { "100baseT/Full", BASE_100_FULL}
        , { "1000baseT/Half", BASE_1000_HALF }
        , { "1000baseT/Full", BASE_1000_FULL }
    };

    unsigned int GetLinkModesNumeric(char const * modesStr)
    {
        unsigned int result = 0;
        for (unsigned i=0; i<sizeof(s_linkModes)/sizeof(0[s_linkModes]); ++i)
        {
            if (strstr(modesStr, s_linkModes[i].str))
            {
                result |= s_linkModes[i].val;
            }
        }

        return result;
    }

    struct LinkSpeedDuplex
    {
        char const * speed;
        char const * duplex;
        unsigned int val;
    } const s_linkModesMap[] =
    {
          { "10M", "Half", BASE_10_HALF }
        , { "10M", "Full", BASE_10_FULL }
        , { "100M", "Half", BASE_100_HALF }
        , { "100M", "Full", BASE_100_FULL }
        , { "1000M", "Half", BASE_1000_HALF }
        , { "1000M", "Full", BASE_1000_FULL }
    };

    unsigned int GetActiveLinkMode(char const * speed, char const * duplex)
    {
        for (unsigned i=0; i<sizeof(s_linkModesMap)/sizeof(0[s_linkModesMap]); ++i)
        {
            LinkSpeedDuplex const & elem = s_linkModesMap[i];
            if (strstr(speed, elem.speed) && strstr(duplex, elem.duplex))
            {
                return elem.val;
            }
        }

        return BASE_10_HALF;
    }
}

bool GetIFFeaturesByCmdLine(int ethNo, IFFeatures *iff)
{
    bzero(iff, sizeof(*iff));
    
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "sudo ethtool eth%d", ethNo);

    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        return false;
    }

    PCloseFile guard(fp);

    char line[1024];
    size_t const bytes = fread(line, 1, sizeof(line), fp);
    if (bytes<sizeof(line))
        line[bytes] = 0;
    else
        line[sizeof(line)-1] = 0;

    {
        string const advertisedLinkModesStr = ExtractAdvertisedLinkModes(line);
        iff->advertisedLinkMode = GetLinkModesNumeric(advertisedLinkModesStr.c_str());
    }

    {
        string const advertisedAutoNeg = ExtractAdvertisedAutoNegotiation(line);
        iff->advertisedAutoNeg = strstr(advertisedAutoNeg.c_str(), "Yes") ? LINK_AUTONEG_ENABLE : LINK_AUTONEG_DISABLE;
    }

    {
        string const activeAutoNeg = ExtractAutoNegotiation(line);
        iff->activeAutoNeg = strstr(activeAutoNeg.c_str(), "on") ? LINK_AUTONEG_ENABLE : LINK_AUTONEG_DISABLE;
    }

    {
        string const lanSpeed = ExtractLinkSpeed(line);
        string const lanDuplex = ExtractLinkDuplex(line);
        iff->activeLinkMode = GetActiveLinkMode(lanSpeed.c_str(), lanDuplex.c_str());
    }
    
    return false;
}

bool IsLinkUpByCmdLine(int ethNo, int & linkUp)
{
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "sudo ethtool eth%d", ethNo);

    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        linkUp = false;
        return false;
    }

    PCloseFile guard(fp);

    char line[512];
    while (NULL!=fgets(line, sizeof(line)-1, fp))
    {
        if (strstr(line, "Link detected"))
        {
            if (strstr(line, "yes"))
            {
                linkUp = true;
            }
            else
            {
                linkUp = false;
            }

            return true;
        }
    }

    linkUp = false;
    return false;
}

GetIFFeaturesFunc GetIFFeatures = GetIFFeaturesByCmdLine;
IsLinkUpFunc IsLinkUp = IsLinkUpByCmdLine;

