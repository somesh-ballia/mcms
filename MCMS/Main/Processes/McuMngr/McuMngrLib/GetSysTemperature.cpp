#include "GetSysTemperature.h"
#include "FileGuard.h"
#include "LineTokenizer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool CmdGetHDDTemperature(string & temperature)
{
    char bufCmdLine[128];
    snprintf(bufCmdLine, sizeof(bufCmdLine), "sudo /usr/sbin/smartctl -a /dev/sda");

    FILE * const fp = popen(bufCmdLine, "r");
    if (!fp)
    {
        temperature = "33";
        return false;
    }

    PCloseFile guard(fp);

    char line[512];
    while (NULL!=fgets(line, sizeof(line)-1, fp))
    {
        if (strstr(line, "Temperature"))
        {
            LineTokenizer lt(line, "\t \n", LineTokenizer::STRIP_SPACE_NO);
            if (lt.GetFieldNum()<10) continue;

            int const val = atoi(lt.GetField(9).c_str());
            if (val<=0 || val>200) continue;

            temperature = lt.GetField(9);
            
            return true;
        }
    }

    temperature = "33";
    return false;
}

bool FunctionGetHDDTemperature(string & temperature)
{
    temperature = "33";
    return true;
}
GetSysTemperatureFunc GetHDDTemperature = CmdGetHDDTemperature;


