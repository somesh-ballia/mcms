#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "DiagDataTypes.h"
#include "SharedDefines.h"
#include "GetIFStatistics.h"
#include "tools.h"


BOOL GetIFStatistics(int ethNo, IFStatistics *pifStat)
{
    BOOL ret = FALSE;
    char ** strArray;
    FILE * fp = NULL;
    int count;
    bzero(pifStat, sizeof(IFStatistics));
    
    if (ethNo<0 || ethNo>9)
        goto done;

    char devName[64];
    bzero(devName, sizeof(devName));
    sprintf(devName, "eth%1d", ethNo);

    char const * const s_procDevFile = "/proc/net/dev";

    fp = fopen(s_procDevFile, "r");
    if (!fp) goto done;

    char line[1024];
    while (fgets(line,sizeof(line),fp)!=NULL)
    {
        if (strstr(line, devName) != NULL)
        {
            count = LineSplitTrim(line, &strArray, ":| \t\n");
            assert(17<=count);
            if(NULL == strArray) continue;
            
            strncpy(pifStat->bytesSent, strArray[9], sizeof(pifStat->bytesSent) - 1);
            strncpy(pifStat->pktsSent, strArray[10], sizeof(pifStat->pktsSent) - 1);
            strncpy(pifStat->pktsSentError, strArray[11], sizeof(pifStat->pktsSentError) - 1);
            strncpy(pifStat->pktsSentDrop, strArray[12], sizeof(pifStat->pktsSentDrop) - 1);
            strncpy(pifStat->pktsSentFifoError, strArray[13], sizeof(pifStat->pktsSentFifoError) - 1);
            
            strncpy(pifStat->bytesRecved, strArray[1], sizeof(pifStat->bytesRecved) - 1);
            strncpy(pifStat->pktsRecved, strArray[2], sizeof(pifStat->pktsRecved) - 1);
            strncpy(pifStat->pktsRecvedError, strArray[3], sizeof(pifStat->pktsRecvedError) - 1);
            strncpy(pifStat->pktsRecvedDrop, strArray[4], sizeof(pifStat->pktsRecvedDrop) - 1);
            strncpy(pifStat->pktsRecvedFifoError, strArray[5], sizeof(pifStat->pktsRecvedFifoError) - 1);
            strncpy(pifStat->pktsRecvedFrameError, strArray[6], sizeof(pifStat->pktsRecvedFrameError) - 1);
		if(NULL != strArray)
		{
			LineSplitFree(strArray, count);
			strArray = NULL;
		}            
            ret = TRUE;
	     goto done;
        }
    }
	
done:
    if(fp) fclose(fp);
    return ret;
}
/*
void DumpIFStatistics(IFStatistics const &ifstat)
{
    std::cout <<"bytesSent: " << ifstat.bytesSent << std::endl;
    std::cout << "pktsSent" << ifstat.pktsSent << std::endl;;
    std::cout << "pktsSentDrop" <<ifstat.pktsSentDrop << std::endl;
    std::cout << "pktsSentError" << ifstat.pktsSentError << std::endl;
    std::cout << "bytesRecved" << ifstat.bytesRecved << std::endl;
    std::cout << "pktsRecved" << ifstat.pktsRecved << std::endl;
    std::cout << "pktsRecvedDrop" << ifstat.pktsRecvedDrop << std::endl;
    std::cout << "pktsRecvedError" << ifstat.pktsRecvedError << std::endl;
    std::cout << std::endl;
}
*/

