#ifndef __OSPROCESS_UTILS_H__
#define __OSPROCESS_UTILS_H__







// All of these function:
// returns 0 on success
//        -1 on failure

#define cmdlineLen 256
#define userLen    10
#define cmdLen     40
#define ttycLen     4

struct PS_PROC {
	
	PS_PROC& operator = (const PS_PROC  & r);
	
	char cmdline[cmdlineLen], user[userLen], cmd[cmdLen], state, ttyc[ttycLen];
	int uid, pid, ppid, pgrp, session, tty, tpgid, utime, stime,
	cutime, cstime, counter, priority, start_time, signal, blocked,
	sigignore, sigcatch;
	unsigned int flags, min_flt, cmin_flt, maj_flt, cmaj_flt, timeout,
	it_real_value, vsize, rss, rss_rlim, start_code, end_code,
	start_stack, kstk_esp, kstk_eip, wchan;
};

typedef enum
{
	eCPUTime     = 1,
	eRSS,
	eInvalidStat
} ePS_PROC;

static const char * GetPsProcEnumName(ePS_PROC en)
{
    static const char *sPS_PROC[] = 
        {
            "Invalid name",                 //Illegal
            "TotalCPUTime (in jiffies)",    //eCPUTime
            "eRSS (in KB)"                  //Resident Set Size	
        };
    
    const char *name = (sizeof(sPS_PROC) / sizeof(sPS_PROC[0]) > (unsigned int)en
                        ?
                        sPS_PROC[en] : "Invalid");
    return name;
}


int process_readstat(int pid, PS_PROC &_psp);
int GetProcessId(const char *procName);
int GetProcessAbsTime(int pid);
int GetProcessAbsTime(const char *procName);



#endif   // __OSPROCESS_UTILS_H__
