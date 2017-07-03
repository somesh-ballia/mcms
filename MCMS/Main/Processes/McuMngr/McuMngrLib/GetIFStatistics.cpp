#include "GetIFStatistics.h"
#include "LineTokenizer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "copy_string.h"

bool GetIFStatistics(int ethNo, IFStatistics *pifStat)
{
    bzero(pifStat, sizeof(IFStatistics));
    
    if (ethNo<0 || ethNo>9)
        return false;

    char devName[64];
    bzero(devName, sizeof(devName));
    sprintf(devName, "eth%1d", ethNo);

    char const * const s_procDevFile = "/proc/net/dev";

    std::ifstream netDevFile(s_procDevFile);
    if (!netDevFile) return false;

    std::string line;
    while (std::getline(netDevFile, line))
    {
        //std::cout << line << "********" << std::endl << std::endl;
        if (std::string::npos!=line.find(devName))
        {
            LineTokenizer lt(line, ":| \t\n", LineTokenizer::STRIP_SPACE_NO);
            int const fieldNum = lt.GetFieldNum();
            assert(17<=fieldNum);
            
            CopyString(pifStat->bytesSent, lt.GetField(9).c_str());
            CopyString(pifStat->pktsSent, lt.GetField(10).c_str());
            CopyString(pifStat->pktsSentError, lt.GetField(11).c_str());
            CopyString(pifStat->pktsSentDrop, lt.GetField(12).c_str());
            CopyString(pifStat->pktsSentFifoError, lt.GetField(13).c_str());
            
            CopyString(pifStat->bytesRecved, lt.GetField(1).c_str());
            CopyString(pifStat->pktsRecved, lt.GetField(2).c_str());
            CopyString(pifStat->pktsRecvedError, lt.GetField(3).c_str());
            CopyString(pifStat->pktsRecvedDrop, lt.GetField(4).c_str());
            CopyString(pifStat->pktsRecvedFifoError, lt.GetField(5).c_str());
            CopyString(pifStat->pktsRecvedFrameError, lt.GetField(6).c_str());
            
            return true;
        }
    }

    return false;
}
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

