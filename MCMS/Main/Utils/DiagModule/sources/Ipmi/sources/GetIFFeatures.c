#include "DiagDataTypes.h"
#include "SharedDefines.h"
#include "GetIFFeatures.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include "tools.h"
#include <linux/if.h>
#include <linux/sockios.h>

typedef unsigned long long u64;
typedef __uint32_t u32;
typedef __uint16_t u16;
typedef __uint8_t u8;
/*
typedef unsigned long long __u64;
typedef __uint32_t __u32;
typedef __uint16_t __u16;
typedef __uint8_t __u8;
*/
#include <linux/ethtool.h>

typedef struct
{
    int speed;
    int duplex;
    unsigned linkMode;
} IFCap;

 IFCap const s_ifcaps[] =
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
    unsigned i;
    for (i=0; i<sizeof(s_ifcaps)/sizeof(0[s_ifcaps]); ++i)
    {
        if ((s_ifcaps[i].speed==speed) && (s_ifcaps[i].duplex==duplex))
        {
            return s_ifcaps[i].linkMode;
        }
    }
    return BASE_10_HALF;
}

/*
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
    strcpy(ifr.ifr_name, devName);

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
    strcpy(ifr.ifr_name, devName);

    struct ethtool_value edata;
    edata.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = (caddr_t)&edata;
    if (ioctl(s, SIOCETHTOOL, &ifr)<0)
        return false;

    linkUp = (0!=edata.data) ? 1 : 0;

    return true;
}
*/

static void ExtractStringByDelim(char const * head, char const * trail, char const * inStr, char * buf, size_t bufLen)
{
    char * pBeg = strstr(inStr, head);
    if (!pBeg)
    {
        buf[0] = '\0';
        return;
    }
    pBeg += strlen(head);

    char * pEnd = strstr(pBeg, trail);
    if (!pEnd)
    {
        pEnd = pBeg + strlen(pBeg) - 1;
    }

    char * trimStr = StringTrim(pBeg, pEnd);

    if(trimStr)
    {
    	  strncpy(buf, trimStr, bufLen);
        buf[bufLen -1] = '\0';
	  free(trimStr);
    }
    else
    {
    	  buf[0] = '\0';
    }
	
    return;
}

static void ExtractSupportedLinkModes(char const * instr, char * buf, size_t bufLen)
{
    return ExtractStringByDelim("Supported link modes:", "Supports auto-negotiation:", instr, buf, bufLen);
}

static void ExtractSupportedAutoNegotiation(char const * instr, char * buf, size_t bufLen)
{
    return ExtractStringByDelim("Supports auto-negotiation:", "\n", instr, buf, bufLen);
}

static void ExtractAdvertisedLinkModes(char const * instr, char * buf, size_t bufLen)
{
    return ExtractStringByDelim("Advertised link modes:", "Advertised auto-negotiation:", instr, buf, bufLen);
}

static void ExtractAdvertisedAutoNegotiation(char const * inStr, char * buf, size_t bufLen)
{
    return ExtractStringByDelim("Advertised auto-negotiation:", "\n", inStr, buf, bufLen);
}

static void ExtractLinkSpeed(char const * instr, char * buf, size_t bufLen)
{
    return ExtractStringByDelim("Speed:", "\n", instr, buf, bufLen);
}

static void ExtractLinkDuplex(char const * instr, char * buf, size_t bufLen)
{
    return ExtractStringByDelim("Duplex:", "\n", instr, buf, bufLen);
}

static void ExtractAutoNegotiation(char const * instr, char * buf, size_t bufLen)
{
    return ExtractStringByDelim("Auto-negotiation:", "\n", instr, buf, bufLen);
}

typedef struct sLinkModStrVal
{
    char const * str;
    unsigned int val;
} LinkModStrVal;

static const LinkModStrVal s_linkModes[] =
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
    unsigned int result = 0, i;
    for (i=0; i<sizeof(s_linkModes)/sizeof(0[s_linkModes]); ++i)
    {
        if (strstr(modesStr, s_linkModes[i].str))
        {
            result |= s_linkModes[i].val;
        }
    }

    return result;
}

typedef struct 
{
    char const * speed;
    char const * duplex;
    unsigned int val;
}LinkSpeedDuplex;

LinkSpeedDuplex const s_linkModesMap[] =
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
    unsigned i=0;
    for (i=0; i<sizeof(s_linkModesMap)/sizeof(0[s_linkModesMap]); ++i)
    {
        if (strstr(speed, s_linkModesMap[i].speed) && strstr(duplex, s_linkModesMap[i].duplex))
        {
            return s_linkModesMap[i].val;
        }
    }

    return BASE_10_HALF;
}

BOOL GetIFFeaturesByCmdLine(int ethNo, IFFeatures *iff)
{
    BOOL ret = FALSE;
    bzero(iff, sizeof(*iff));
    
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "ethtool eth%d", ethNo);

    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        goto done;
    }

    char line[1024];
    size_t const bytes = fread(line, 1, sizeof(line), fp);
    if (bytes<sizeof(line))
        line[bytes] = 0;
    else
        line[sizeof(line)-1] = 0;

    char strTemp[1024], strTemp2[1024];

    {
        ExtractAdvertisedLinkModes(line, strTemp, 1024);
        iff->advertisedLinkMode = GetLinkModesNumeric(strTemp);
    }

    {
        ExtractAdvertisedAutoNegotiation(line, strTemp, 1024);
        iff->advertisedAutoNeg = strstr(strTemp, "Yes") ? LINK_AUTONEG_ENABLE : LINK_AUTONEG_DISABLE;
    }

    {
        ExtractAutoNegotiation(line, strTemp, 1024);
        iff->activeAutoNeg = strstr(strTemp, "on") ? LINK_AUTONEG_ENABLE : LINK_AUTONEG_DISABLE;
    }

    {
        ExtractLinkSpeed(line, strTemp, 1024);
        ExtractLinkDuplex(line, strTemp2, 1024);
        iff->activeLinkMode = GetActiveLinkMode(strTemp, strTemp2);
    }
    ret = TRUE;
done:
    if(fp) pclose(fp);
    return ret;
}

BOOL IsLinkUpByCmdLine(int ethNo, BOOL * linkUp)
{
    char bufCmdLine[128];
    BOOL ret = FALSE;
    *linkUp = FALSE;
    snprintf(bufCmdLine, sizeof(bufCmdLine), "ethtool eth%d", ethNo);

    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        *linkUp = FALSE;
        goto done;
    }

    char line[512];
    while (NULL!=fgets(line, sizeof(line)-1, fp))
    {
        if (strstr(line, "Link detected"))
        {
            if (strstr(line, "yes"))
            {
                *linkUp = TRUE;
            }
            else
            {
               *linkUp = FALSE;
            }

            ret = TRUE;
	     goto done;
        }
    }

done:
    if(fp) pclose(fp);
    return ret;
}

GetIFFeaturesFunc GetIFFeatures = GetIFFeaturesByCmdLine;
IsLinkUpFunc IsLinkUp = IsLinkUpByCmdLine;

