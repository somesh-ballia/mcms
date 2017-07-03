#ifndef SYSSTATE_H
#define SYSSTATE_H

class CSysState
{
public:
    enum { cpuUser,cpuNice,cpuSystem,cpuIdle,cpuStates };

    CSysState();
    virtual ~CSysState() {}

    int GetCPUUsage();
    int GetMemUsage();

protected:
    typedef unsigned long long cpu_usage_int_type;
    typedef cpu_usage_int_type cpu_stat_t[cpuStates];

    bool Percentages(cpu_stat_t out);

    static char *skip_token(char *p);

private:
    cpu_stat_t	m_time;
    cpu_stat_t	m_old;
    cpu_stat_t	m_diff;
    int m_prevCpuUsage;
};

#endif
