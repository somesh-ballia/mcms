#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DiagDataTypes.h"
#include "SharedDefines.h"
#include "GetSysTemperature.h"
#include "tools.h"

BOOL CmdGetHDDTemperature(int * temperature)
{
    char bufCmdLine[128];
    BOOL ret = FALSE;
    int count = 0;
    char ** strArray = NULL;
    snprintf(bufCmdLine, sizeof(bufCmdLine), "/usr/sbin/smartctl -a /dev/sda");

    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        *temperature = 33;
        goto done;
    }
	
    char line[512];
    while (NULL!=fgets(line, sizeof(line)-1, fp))
    {
        if (strstr(line, "Temperature"))
        {
	     count = LineSplitTrim(line, &strArray, "\t \n");
            if (count<10 || NULL == strArray){
			if(NULL != strArray)
			{
				LineSplitFree(strArray, count);
				strArray = NULL;
			}
			continue;
		}

            int const val = atoi(strArray[9]);
            if (val<=0 || val>200)
	     {
			if(NULL != strArray)
			{
				LineSplitFree(strArray, count);
				strArray = NULL;
			}
			continue;
		}

            *temperature = val;

		ret = TRUE;
            goto done;
        }
    }

    *temperature = 33;

done:
    if(fp) pclose(fp);
    if(NULL != strArray)
    {
    	LineSplitFree(strArray, count);
    	strArray = NULL;
    }
    return ret;
}

BOOL FunctionGetHDDTemperature(int * temperature)
{
    *temperature = 33;
    return TRUE;
}
GetSysTemperatureFunc GetHDDTemperature = CmdGetHDDTemperature;


