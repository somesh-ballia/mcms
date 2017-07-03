#ifndef __GET_IF_STATISTICS_H__
#define __GET_IF_STATISTICS_H__

typedef struct
{
    char bytesSent[24];
    char pktsSent[24];
    char pktsSentDrop[24];
    char pktsSentError[24];
    char pktsSentFifoError[24];
    
    char bytesRecved[24];
    char pktsRecved[24];
    char pktsRecvedDrop[24];
    char pktsRecvedError[24];
    char pktsRecvedFifoError[24];
    char pktsRecvedFrameError[24];
} IFStatistics;
bool GetIFStatistics(int ethNo, IFStatistics *pifStat);
void DumpIFStatistics(IFStatistics const &);

#endif

