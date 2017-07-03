#include <string>
#include <asm/param.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>

using namespace std;


#include "OsProcessUtils.h"
#include "SystemFunctions.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

int GetBootTime();
int GetHertz();



int GetBootTime()
{
    static int bTime = -1;
    if(-1 == bTime)
    {    
        char cbuf[1024];
        FILE* fp = NULL;
        
        if( (fp = fopen( "/proc/stat", "r" )) != NULL ){ 
            while(!feof(fp)) {
                if(fscanf(fp,"btime %d", &bTime) == 1)
                    break;
                if (fgets(cbuf, 1024, fp) == NULL)
                    break;           
            }
            fclose(fp);
        }
    }
    
    return bTime;
}
int GetHertz()
{
    static int hertz = -1;
    if(-1 == hertz)
    {
        
#ifdef HZ
        hertz = (unsigned long)HZ;    /* <asm/param.h> */
#else
        unsigned long user_j, nice_j, sys_j, other_j;  /* jiffies (clock ticks) */
        double up_1, up_2, seconds;
        unsigned long jiffies, h;
        int hertz = 0;
        
        do{
            FILE_TO_BUF(UPTIME_FILE,uptime_fd);  sscanf(buf, "%lf", &up_1);
            /* uptime(&up_1, NULL); */
            FILE_TO_BUF(STAT_FILE,stat_fd);
            sscanf(buf, "cpu %lu %lu %lu %lu", &user_j, &nice_j, &sys_j, &other_j);
            FILE_TO_BUF(UPTIME_FILE,uptime_fd);  sscanf(buf, "%lf", &up_2);
            /* uptime(&up_2, NULL); */
        } while((long)( (up_2-up_1)*1000.0/up_1 )); /* want under 0.1% error */
        
        jiffies = user_j + nice_j + sys_j + other_j;
        seconds = (up_1 + up_2) / 2;
        h = (unsigned long)( (double)jiffies/seconds );
        switch(h)
        {
            case   48 ...   52 :  hertz =   50; break;
            case   58 ...   62 :  hertz =   60; break;
            case   95 ...  105 :  hertz =  100; break; /* normal Linux */
            case  124 ...  132 :  hertz =  128; break;
            case  195 ...  204 :  hertz =  200; break; /* normal << 1 */
            case  253 ...  260 :  hertz =  256; break;
            case  393 ...  408 :  hertz =  400; break; /* normal << 2 */
            case  790 ...  808 :  hertz =  800; break; /* normal << 3 */
            case  990 ... 1010 :  hertz = 1000; break;
            case 1015 ... 1035 :  hertz = 1024; break; /* Alpha */
            default:
                hertz = (sizeof(long)==sizeof(int)) ? 100UL : 1024UL;
//		ppt_warn("Unknown HZ value! (%ld) Assume %ld.\n", h, hertz);
        }
#endif /* HZ */
        
    }
    
    if(0 == hertz)
    {
        hertz = 1;  // hertz is a divisor.
    }
    
    return hertz;
}




/*
	Read the first line of a file
	Return -1 if any error.
*/
static int process_readfile (const char *path, char *buf, int sizebuf)
{
	int ret = -1;
	// No error message signaled as "path" is a file under /proc/pid
	// and the process may vanish while we are reading it.
	FILE *fin = fopen (path,"r");
	if (fin != NULL){
		if (fgets (buf,sizebuf-1,fin)!=NULL){
			ret = 0;
		}
		fclose (fin);
	}
	return ret;
}


/*
	Read the file /proc/pid/stat into the struct PS_PROC
	Return -1 if any error.
*/
int process_readstat(int pid, PS_PROC &_psp)
{
    int ret = -1;
	char tmppath[PATH_MAX];
	sprintf (tmppath,"/proc/%d/stat",pid);
	char buf[PATH_MAX];
	if (process_readfile (tmppath,buf,sizeof(buf))!=-1){
		PS_PROC psp;
        psp.cmdline[0] = 0;
        psp.user[0] = 0;
        psp.cmd[0] = 0;
        psp.ttyc[0] = 0;
		psp.uid = -1;
        
		sscanf(buf, "%d %s %c %d %d %d %d %d %u %u "
			"%u %u %u %d %d %d %d %d %d %u "
			"%u %d %u %u %u %u %u %u %u %u %u "
			"%u %u %u %u\n",
			&psp.pid, psp.cmd, &psp.state, &psp.ppid,
			&psp.pgrp, &psp.session, &psp.tty, &psp.tpgid,
			&psp.flags, &psp.min_flt, &psp.cmin_flt,
			&psp.maj_flt, &psp.cmaj_flt,
			&psp.utime, &psp.stime, &psp.cutime, &psp.cstime,
			&psp.counter, &psp.priority, &psp.timeout,
			&psp.it_real_value, &psp.start_time,
			&psp.vsize, &psp.rss, &psp.rss_rlim,
			&psp.start_code, &psp.end_code, &psp.start_stack,
			&psp.kstk_esp, &psp.kstk_eip,
			&psp.signal, &psp.blocked, &psp.sigignore, &psp.sigcatch,
			&psp.wchan);
		_psp = psp;
		ret = 0;
	}
	return ret;
}


int GetProcessId(const char *procName)
{
    char command[128];
    snprintf(command, sizeof(command), "ps -e | grep %s | grep -v grep | grep -v defunct | grep -v \" Z \" | awk '{ print $1 }'", procName);
    std::string answer;
    SystemPipedCommand(command, answer);
    int procId = (!answer.empty() ? atoi(answer.c_str()) : -1);
    return procId;
}

int GetProcessAbsTime(const char *procName)
{
    int absTime = -1;
    int pid = GetProcessId(procName);
    if(-1 != pid)
    {
        absTime = GetProcessAbsTime(pid);
    }
    return absTime;
}

int GetProcessAbsTime(int pid)
{
    PS_PROC psp;
    memset(&psp, 0, sizeof(PS_PROC));
    int res = process_readstat(pid, psp);
    if(-1 == res)
    {
        return -1;
    }
    
    time_t procUptime = (psp.start_time / GetHertz()) + GetBootTime();
    time_t now = time((time_t*)NULL);
    int absProcUptime = now - procUptime;
    
    return absProcUptime;
}

PS_PROC& PS_PROC::operator = (const PS_PROC  & r)
{
	strncpy(cmdline,r.cmdline,cmdlineLen-1);
	cmdline[cmdlineLen-1] = '\0';
	
	strncpy(user,r.user,userLen-1);
	user[userLen-1] = '\0';
	
	strncpy(cmd,r.cmd,cmdLen-1);
	cmd[cmdLen-1] = '\0';
	
	strncpy(ttyc,r.ttyc,ttycLen-1);
	ttyc[ttycLen-1] = '\0';
	
	uid = r.uid;
	pid = r.pid;
	ppid = r.ppid;
	pgrp = r.pgrp;
	session = r.session;
	tty = r.tty;
	tpgid = r.tpgid;
	utime = r.utime;
	stime = r.stime;
	cutime = r.cutime;
	cstime = r.cstime;
	counter = r.counter;
	priority = r.priority;
	start_time = r.start_time;
	blocked = r.blocked;
	sigignore = r.sigignore;
	sigcatch = r.sigcatch;
	
	min_flt = r.min_flt;
	cmin_flt = r.cmin_flt;
	maj_flt = r.maj_flt;
	cmaj_flt = r.cmaj_flt;
	timeout = r.timeout;
	it_real_value = r.it_real_value;
	vsize = r.vsize;
	rss = r.rss;
	rss_rlim = r.rss_rlim;
	start_code = r.start_code;
	end_code = r.end_code;
	start_stack = r.start_stack;
	kstk_esp = r.kstk_esp;
	kstk_eip = r.kstk_eip;
	wchan = r.wchan;
	
	return *this;
}
	
	
	
