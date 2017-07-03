#ifndef SYS_LIGHT_TIMER_MNGR_H_
#define SYS_LIGHT_TIMER_MNGR_H_

#include "BaseProcessor.h"
//#include "LedLightServer.h"
using namespace std;
class BaseProcessor;
class LedLightServer;
//typedef void Sigfuc(int);
//typedef void (BaseProcessor::*Sigfuc)(int);
typedef map<eLightType, lightInfo> CLightTimerMap; 

//typedef void (BaseProcessor::*HANDLE_TIMER)(int );
class sysLightTimerMngr
{
public:
                                sysLightTimerMngr();
    virtual                     ~sysLightTimerMngr();
    void                        AddTimerItem(lightInfo* timer);
    void                        DelTimerItem(lightInfo* timer);
    void                        DelAllItems();
    void                        Run();
    void                        SetTimerInterval(int tv_sec,int tv_usec, int itv_sec, int itv_usec);
    //void                        lightTimerExpired();
    CLightTimerMap              *GetLightTimerMap(){return &m_lightstimer_map;};
protected:
    void Lock() { pthread_mutex_lock(&m_mutex); }
    void Unlock() { pthread_mutex_unlock(&m_mutex); }

//members
private:
   struct itimerval tick;
   //int sigType;
   //list<lightInfo*>                m_lights_timer;
   CLightTimerMap               m_lightstimer_map;
   pthread_mutex_t	            m_mutex;
};

#endif

