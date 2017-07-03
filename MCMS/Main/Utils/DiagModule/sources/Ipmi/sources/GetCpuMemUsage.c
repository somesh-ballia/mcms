#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GetCpuMemUsage.h"
#include "tools.h"
#include "SharedDefines.h"
#include <sys/sysinfo.h>
#include <assert.h>

enum { cpuUser,cpuNice,cpuSystem,cpuIdle,cpuStates };

typedef unsigned long long cpu_usage_int_type;
typedef cpu_usage_int_type cpu_stat_t[cpuStates];

static cpu_stat_t	m_time;
static cpu_stat_t	m_old;
static cpu_stat_t	m_diff;
static int m_prevCpuUsage;

TTimerJobReq tCheckCPUReq;

void CheckCPUUsage(void *p)
{
	GetCPUUsage();
}


BOOL Percentages(cpu_stat_t out)
{
    int i;
    /* initialization */
    cpu_usage_int_type total_change=0;
    cpu_stat_t backupOld;
    memcpy(backupOld, m_old, sizeof(backupOld));
    cpu_usage_int_type *dp=m_diff;

    /* calculate changes for each state and the overall change */
    cpu_usage_int_type change;
    cpu_usage_int_type *pOld = m_old;
    cpu_usage_int_type *pTime = m_time;
    for (i=0;i<cpuStates;i++)
    {
        change = *pTime - *pOld;
        total_change+=(*dp++=change);
        *pOld++=*pTime++;
    }

    if (total_change<10)
    {
        memcpy(m_old, backupOld, sizeof(m_old));
        return FALSE;
    }
    
    /* calculate Percentages based on overall change,rounding up */
    cpu_usage_int_type half_total=total_change/2;
    cpu_usage_int_type *pDiff = m_diff;
    for (i=0;i<cpuStates;i++)
        *out++=((*pDiff++*1000+half_total)/total_change);

    return TRUE;
}

static char * skip_token(char *p)
{
    while (isspace(*p)) p++;
    while (*p && !isspace(*p)) p++;
    return p;
}

static int const MAX_BUF_LEN = 1024;

int GetCPUUsage()
{
    char * filename = "/proc/stat";
    FILE * fp = fopen(filename, "r");
    if (NULL == fp)
    {
        perror("CFile");
        return 100;
    }
    char buffer[MAX_BUF_LEN];
    int len = fread(buffer, sizeof(char), MAX_BUF_LEN -1,  fp);
    buffer[len]='\0';
    fclose(fp);

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

static char const * const memInfoCmd = "cat /proc/meminfo 2>/dev/null";
enum
{
    MEM_STAT_ITEM_COUNT = 4
};

int GetMemUsage()
{
#if 1
    FILE * const fp = popen(memInfoCmd, "r");
    int count = 0;
    char ** strArray = NULL;
    if (fp)
    {
        char line[512];
        int stat[MEM_STAT_ITEM_COUNT] = {2, 1, 0, 0};
        int i=0;
        while (NULL!=fgets(line, sizeof(line)-1, fp))
        {
            count = LineSplitTrim(line, &strArray, " \t\n");
			if(NULL != strArray)
			{
				if(NULL != strArray[1]) stat[i] = atoi(strArray[1]);
				LineSplitFree(strArray, count);
				strArray = NULL;
			}
			++i;
            if (i>=MEM_STAT_ITEM_COUNT)
                break;
        }

        if (stat[0]==0)
            stat[0] = 2;
		
	 pclose(fp);
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
