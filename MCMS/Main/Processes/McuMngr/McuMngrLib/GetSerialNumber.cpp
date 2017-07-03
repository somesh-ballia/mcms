#include "GetSerialNumber.h"
#include "GetMacAddr.h"
#include "ProductType.h"
#include "ProcessBase.h"
#include <string.h>


int GetSerialNumberUsingMacAddr(char buf[], size_t size)
{
    string const mac = GetMacAddrWithoutSemiColon("eth0");
    int const len = (mac.size()>size-1) ? size-1 : mac.size();
    memcpy(buf, mac.c_str(), len);
    buf[len] = 0;
    return (len>0);
}


extern int GetBoardSerialNumber(char * buf, size_t len);
int GetSerialNumberUsingBoardInfo(char * buf, size_t len, char *err_buf, size_t err_size)
{
	int ret = GetBoardSerialNumber(buf, len);
	if (!ret)
	{
		ret = GetSerialNumberUsingMacAddr(buf, len);
	}

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

	if (curProductType == eProductTypeNinja)
	{
		err_buf[0] = '\0';
		std::string file = MCU_TMP_DIR+"/getSerialErrCode";

		FILE* pFile = fopen(file.c_str(), "r");
		if(pFile)
		{
			fgets(err_buf,err_size,pFile);
			fclose(pFile);
		}
		//no display "video card not match" due to dsp broken.
		//PLATFORM_CHECK_ERR_DM6467 = -7,		/* video card not match */
		//PLATFORM_CHECK_ERR_DM8168 = -10,		/* video card not match */             
		if(0 == strncmp(err_buf, "video card not match", strlen("video card not match")))
		{
		    err_buf[0] = '\0';
		}
	}
	return ret;
}

//GetSerialNumberFuncType GetSerialNumber = GetSerialNumberUsingMacAddr;
GetSerialNumberFuncType GetSerialNumber = GetSerialNumberUsingBoardInfo;

