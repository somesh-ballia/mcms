#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "FileWrapper.h"
#include "GetCpuMemUsage.h"
#include "LineTokenizer.h"
#include "FileGuard.h"
#include <sys/sysinfo.h>
#include <assert.h>

CSysState::CSysState()
    : m_prevCpuUsage(0)
{
    for (int i=0;i<cpuStates;i++)
    {
        m_time[i]=0;
        m_old[i]=0;
        m_diff[i]=0;
    }
}

bool CSysState::Percentages(cpu_stat_t out)
{
    /* initialization */
    cpu_usage_int_type total_change=0;
    cpu_stat_t backupOld;
    memcpy(backupOld, m_old, sizeof(backupOld));
    cpu_usage_int_type *dp=m_diff;

    /* calculate changes for each state and the overall change */
    cpu_usage_int_type change;
    cpu_usage_int_type *pOld = m_old;
    cpu_usage_int_type *pTime = m_time;
    for (int i=0;i<cpuStates;i++)
    {
        change = *pTime - *pOld;
        total_change+=(*dp++=change);
        *pOld++=*pTime++;
    }

    if (total_change<10)
    {
        memcpy(m_old, backupOld, sizeof(m_old));
        return false;
    }
    
    /* calculate Percentages based on overall change,rounding up */
    cpu_usage_int_type half_total=total_change/2;
    cpu_usage_int_type *pDiff = m_diff;
    for (int i=0;i<cpuStates;i++)
        *out++=((*pDiff++*1000+half_total)/total_change);

    return true;
}

char *CSysState::skip_token(char *p)
{
    while (isspace(*p)) p++;
    while (*p && !isspace(*p)) p++;
    return p;
}

static int const MAX_BUF_LEN = 1024;
int CSysState::GetCPUUsage()
{
    CFile file("/proc/stat","r");
    if (!file.IsValid())
    {
        perror("CFile");
        return 100;
    }
    char buffer[MAX_BUF_LEN];
    int len=file.Read(buffer,sizeof(buffer)-1);
    buffer[len]='\0';
    file.Close();

    char *p=skip_token(buffer);		/* "cpu" */
    m_time[0]=strtoull(p,&p,0);
    m_time[1]=strtoull(p,&p,0);
    m_time[2]=strtoull(p,&p,0);
    m_time[3]=strtoull(p,&p,0);

    /* convert m_time counts to Percentages */
    cpu_stat_t cpu_states;
    if (!Percentages(cpu_states))
    {
        return m_prevCpuUsage;
    }
    else
    {
        int const cpuUsage = 1000-cpu_states[cpuIdle];
        m_prevCpuUsage = cpuUsage;
        return cpuUsage;
    }
}

namespace
{
static char const * const memInfoCmd = "cat /proc/meminfo 2>/dev/null";
enum
{
    MEM_STAT_ITEM_COUNT = 4
};
}
int CSysState::GetMemUsage()
{
#if 1
    FILE * const fp = popen(memInfoCmd, "r");
    PCloseFile guard(fp);
    if (fp)
    {
        char line[512];
        int stat[MEM_STAT_ITEM_COUNT] = {2, 1, 0, 0};
        int i=0;
        while (NULL!=fgets(line, sizeof(line)-1, fp))
        {
            LineTokenizer lt(line, " \t\n", LineTokenizer::STRIP_SPACE_NO);
            stat[i] = atoi(lt.GetField(1).c_str());
            ++i;
            if (i>=MEM_STAT_ITEM_COUNT)
                break;
        }

        if (stat[0]==0)
            stat[0] = 2;
        return ((int64_t)(stat[0]-(stat[1]+stat[2]+stat[3])))*1000/stat[0];
    }
    else
    {
        return 200;
    }
#else
    struct sysinfo info;
    if (sysinfo(&info)<0)
    {
        return 200;
    }
    else
    {
        return ((int64_t)(info.totalram-info.freeram)*1000)/info.totalram;
    }
#endif
}
