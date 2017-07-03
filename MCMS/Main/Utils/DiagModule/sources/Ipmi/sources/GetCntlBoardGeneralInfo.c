#include "GetCntlBoardGeneralInfo.h"
#include "IpmiConsts.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "GetIFList.h"
#include "tools.h"
#include "GetMacAddr.h"

char const * ReadFileContent(char const * fn, char * buf, size_t len)
{
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "cat %s 2>/dev/null", fn);
    FILE * fp = popen(bufCmdLine, "r");
    if (fp)
    {
        size_t bytes = fread(buf, 1, len, fp);
        int offset = (bytes>=len) ? len-1 : bytes;
        buf[bytes] = 0;

        {
            int offset = strlen(buf)-1;
            while (offset>=0 && isspace(buf[offset]))
            {
                buf[offset] = 0;
                --offset;
            }
        }
    }
    else
    {
        buf[0] = 0;
    }

    if(fp) pclose(fp);
	
    return buf;
}

char const * GetHwVersion(char *buf, size_t len)
{
    return ReadFileContent("/sys/class/dmi/id/board_version", buf, len);
}

char const * GetSwVersion(char *buf, size_t len)
{
    return ReadFileContent("/sys/class/dmi/id/bios_version", buf, len);
}

char const * GetBoardSerial(char * buf, size_t len)
{
    //return ReadFileContent("/sys/class/dmi/id/product_serial", buf, len);

    system("/usr/rmx1000/bin/RPCS1800getSerial /tmp/ninjaSerial_diag");                   
    return ReadFileContent("/tmp/ninjaSerial_diag", buf, len);
}

char const * GetBoardPartNumber(char * buf, size_t len)
{
    return ReadFileContent("/sys/class/dmi/id/product_uuid", buf, len);
}

inline void FixPartNumber(TIpmiFru * info)
{
    char bufTemp[sizeof(info->unBoardPartNumber)];
    memcpy(bufTemp, info->unBoardPartNumber, sizeof(bufTemp));
    int offset = strlen(bufTemp);
    while (offset>=0 && bufTemp[offset]!='-')
    {
        --offset;
    }
    if(offset < 0) offset = 0;
    offset += ('-'==bufTemp[offset]) ? 1 : 0;
    memset(info->unBoardPartNumber, 0, sizeof(info->unBoardPartNumber));
    strncpy(info->unBoardPartNumber, &(bufTemp[offset]), sizeof(bufTemp) - offset);
}

char m_cpldVersion[64] = "-";

char const * GetCPLDVersion(char *buf, size_t len)
{
    if(0 == strncmp(m_cpldVersion, "-", sizeof(m_cpldVersion)))
    {
        char answer[64] = {0};
        BOOL stat = SystemCommand("/usr/rmx1000/bin/cpld_version" ,answer, sizeof(answer));
         if (stat == TRUE)
        {
            strncpy(m_cpldVersion, answer, sizeof(m_cpldVersion));
            m_cpldVersion[sizeof(m_cpldVersion) -1] = '\0';
        }        
    }

    strncpy(buf, m_cpldVersion, len);
    buf[len -1] = '\0';
    
    return buf; 
}

//#define USE_STATIC_INIT
#ifdef USE_STATIC_INIT
TIpmiFru s_info;
#endif

int GetBoardSerialNumber(char * buf, size_t len)
{
#ifdef USE_STATIC_INIT
    strcpy(buf, len, s_info.serial);
#else
    GetBoardSerial(buf, len);
#endif
    return (0!=buf[0]);
}

void GetCntlBoardGeneralInfo(TIpmiFru* info)
{
	
#ifdef USE_STATIC_INIT
    memcpy(info, &s_info, sizeof(TIpmiFru));
#else
    memset(info, 0, sizeof(TIpmiFru));
    info->unBoardMfgDateTime = 111;
    info->unChassisType = 0;
    info->unSubBoardID = 1;
    //GetCntlBoardName(info->unBoardProductName, sizeof(info->unBoardProductName));
    snprintf(info->unBoardProductName, sizeof(info->unBoardProductName), "%s", "NINJA_CNTL");
    GetHwVersion(info->unBoardHardwareVers, sizeof(info->unBoardHardwareVers));
    GetSwVersion(info->unChassisHwVersion, sizeof(info->unChassisHwVersion));
    GetBoardSerial(info->unBoardSerialNumber, sizeof(info->unBoardSerialNumber));
    snprintf(info->unBoardSerialNumber, sizeof(info->unBoardSerialNumber), "%s", "-");
    GetBoardPartNumber(info->unBoardPartNumber, sizeof(info->unBoardPartNumber));
    FixPartNumber(info);
    //GetCPLDVersion(info->unChassisHwVersion, sizeof(info->unChassisHwVersion));
#endif

        {
            IFInfo ifs[4];
            unsigned int count = 0;
            GetIFList(ifs, 4, &count);
		    unsigned int i;
            for (i=1; i<=count; ++i)
            {
                char bufMac[64];
                GetMacAddrWithSemiColon(ifs[i-1].name, bufMac, sizeof(bufMac));
                switch(ifs[i-1].index)
                {
                	case 0:
				strcpy(info->unBoardMacAddr1, bufMac);
				break;
                	case 1:
				strcpy(info->unBoardMacAddr2, bufMac);
				break;
                	case 2:
				strcpy(info->unBoardMacAddr3, bufMac);
				break;
                	case 3:
				strcpy(info->unBoardMacAddr4, bufMac);
				break;
                }
            }
        }

	info->ulNumOfElem = 0;
	info->ulNumOfElemFields = 4;
}

char const * GetCntlBoardName(char * bufName, size_t len)
{
    return ReadFileContent("/sys/class/dmi/id/product_name", bufName, len);
}

void GetDSPCardGeneralInfo(TIpmiFru* info)
{
	memset(info, 0, sizeof(TIpmiFru));
	info->unBoardMfgDateTime = 111;
	info->unChassisType = 0;
	info->unSubBoardID = 1;
	snprintf(info->unBoardProductName, sizeof(info->unBoardProductName), "%s", NETRA_DSP_BOARD_NAME);
	snprintf(info->unBoardHardwareVers, sizeof(info->unBoardHardwareVers), "%s",  g_hwVersion);
	snprintf(info->unChassisHwVersion, sizeof(info->unChassisHwVersion), "%s", "-");
	snprintf(info->unBoardSerialNumber, sizeof(info->unBoardSerialNumber), "%s", "-");
	snprintf(info->unBoardPartNumber, sizeof(info->unBoardPartNumber), "%s", "-");

	{
	strncpy(info->unBoardMacAddr1, "-", sizeof(info->unBoardMacAddr1) - 1);
	strncpy(info->unBoardMacAddr2, "-", sizeof(info->unBoardMacAddr2) - 1);
	strncpy(info->unBoardMacAddr3, "-", sizeof(info->unBoardMacAddr3) - 1);
	strncpy(info->unBoardMacAddr4, "-", sizeof(info->unBoardMacAddr4) - 1);
	}

	info->ulNumOfElem = 0;
	info->ulNumOfElemFields = 4;
}

void GetRTMBoardGeneralInfo(TIpmiFru* info)
{
	memset(info, 0, sizeof(TIpmiFru));
	info->unBoardMfgDateTime = 111;
	info->unChassisType = 0;
	info->unSubBoardID = 2;
	snprintf(info->unBoardProductName, sizeof(info->unBoardProductName), "%s", NETRA_RTM_ISDN_NAME);
	snprintf(info->unBoardHardwareVers, sizeof(info->unBoardHardwareVers), "%s",  "-");
	snprintf(info->unChassisHwVersion, sizeof(info->unChassisHwVersion), "%s", "-");
	snprintf(info->unBoardSerialNumber, sizeof(info->unBoardSerialNumber), "%s", "-");
	snprintf(info->unBoardPartNumber, sizeof(info->unBoardPartNumber), "%s", "-");

	{
	strncpy(info->unBoardMacAddr1, "-", sizeof(info->unBoardMacAddr1) - 1);
	strncpy(info->unBoardMacAddr2, "-", sizeof(info->unBoardMacAddr2) - 1);
	strncpy(info->unBoardMacAddr3, "-", sizeof(info->unBoardMacAddr3) - 1);
	strncpy(info->unBoardMacAddr4, "-", sizeof(info->unBoardMacAddr4) - 1);
	}

	info->ulNumOfElem = 0;
	info->ulNumOfElemFields = 4;
}
