#include "GetCntlBoardGeneralInfo.h"
#include "copy_string.h"
#include "FileGuard.h"
#include "IpmiConsts.h"
#include "ProcessBase.h"
#include "Trace.h"
#include "TraceStream.h"
namespace
{
    char const * ReadFileContent(char const * fn, char * buf, size_t len)
    {
        char bufCmdLine[128];
        snprintf(bufCmdLine, sizeof(bufCmdLine), "sudo cat %s 2>/dev/null", fn);
        FILE * fp = popen(bufCmdLine, "r");
        PCloseFile guard(fp);
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

	char const * GeteepromSerial(char * buf, size_t len)
	{
		std::string answer;
		std::string cmd = "sudo rm -rf "+MCU_TMP_DIR+"/ninjaSerial "+MCU_TMP_DIR+"/getSerialErrCode";
		SystemPipedCommand("cmd.c_str()",answer);
		cmd = "sudo "+MCU_MRMX_DIR+"/bin/RPCS1800getSerial "+MCU_TMP_DIR+"/ninjaSerial "+MCU_TMP_DIR+"/getSerialErrCode";
	    SystemPipedCommand(cmd.c_str(),answer);
		
	    std::string fname = MCU_TMP_DIR+"/ninjaSerial";
		return ReadFileContent(fname.c_str(), buf, len);
	}


    char const * GetBoardSerial(char * buf, size_t len)
    {
    	eProductType prodType = CProcessBase::GetProcess()->GetProductType();
		if (eProductTypeNinja == prodType)
		{
			return GeteepromSerial(buf,len);
		}
        return ReadFileContent("/sys/class/dmi/id/product_serial", buf, len);
    }

    char const * GetBoardPartNumber(char * buf, size_t len)
    {
        return ReadFileContent("/sys/class/dmi/id/product_uuid", buf, len);
    }

    inline void FixPartNumber(CntlBoardGeneralInfo & info)
    {
        char bufTemp[sizeof(info.partnumber)]={0};
        memcpy(bufTemp, info.partnumber, sizeof(bufTemp)-1);
        int offset = strlen(bufTemp);
        while (offset>0 && bufTemp[offset-1]!='-')
        {
            --offset;
        }
        offset += ('-'==bufTemp[offset]) ? 1 : 0;
        CopyString(info.partnumber, &(bufTemp[offset]));
    }
//#define USE_STATIC_INIT
#ifdef USE_STATIC_INIT
    CntlBoardGeneralInfo s_info;
    class BoardInfoGetter
    {
    public:
        BoardInfoGetter()
        {
            memset(&s_info, 0, sizeof(s_info));
            GetCntlBoardName(s_info.name, sizeof(s_info.name));
            GetHwVersion(s_info.hwver, sizeof(s_info.hwver));
            GetSwVersion(s_info.swver, sizeof(s_info.swver));
            GetBoardSerial(s_info.serial, sizeof(s_info.serial));
            GetBoardPartNumber(s_info.partnumber, sizeof(s_info.partnumber));
            FixPartNumber(s_info);
        }
    } s_getter;
#endif
}

int GetBoardSerialNumber(char * buf, size_t len)
{
#ifdef USE_STATIC_INIT
    CopyString(buf, len, s_info.serial);
#else
    GetBoardSerial(buf, len);
#endif
    return (0!=buf[0]);
}

std::string m_cpldVersion = "-";

char const * GetCPLDVersion(char *buf, size_t len)
{
    std::string answer = "";
    if(m_cpldVersion == "-")
    {
        std::string cmd = "sudo "+MCU_MRMX_DIR+"/bin/cpld_version";
        STATUS stat = SystemPipedCommand(cmd.c_str() ,answer);      
        if (stat == STATUS_OK)
        {
            m_cpldVersion = answer;   
        }
    }

    CopyString(buf, len, m_cpldVersion.c_str());
    return buf;
}
    
void GetCntlBoardGeneralInfo(CntlBoardGeneralInfo & info)
{
#ifdef USE_STATIC_INIT
    (void)s_getter;
    memcpy(&info, &s_info, sizeof(info));
#else
    memset(&info, 0, sizeof(info));
    GetCntlBoardName(info.name, sizeof(info.name));
    GetHwVersion(info.hwver, sizeof(info.hwver));
    GetSwVersion(info.swver, sizeof(info.swver));
    GetBoardSerial(info.serial, sizeof(info.serial));
    GetBoardPartNumber(info.partnumber, sizeof(info.partnumber));
    FixPartNumber(info);
    GetCPLDVersion(info.riserCardCpldVersion, sizeof(info.riserCardCpldVersion));
#endif
}

char const * GetCntlBoardName(char * bufName, size_t len)
{
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    if(eProductTypeGesher == curProductType)
    {
       
        return ReadFileContent("/sys/class/dmi/id/product_name", bufName, len);
    }
    else if(eProductTypeNinja == curProductType)
      {
          memset(bufName,0,len);
          strcpy(bufName,"NINJA_CNTL");
          return bufName;
      }
      return NULL;
}

