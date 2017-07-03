#include "sysLightTimerMngr.h"
#include "LedLightServer.h"
#include "LightCntlApi.h"

//compile comand: g++ -g cppTest.cpp -o gTest
//g++  cppTest.cpp -o cTest

void getSystemTime()
{
    char sCurDate[100];
    struct tm *current_date;
    time_t seconds;

    time(&seconds);
    current_date = localtime(&seconds);

	if (NULL != current_date)
	{
    	memset(sCurDate, 0, 100);
    	sprintf(sCurDate, "%04d-%02d-%02d %02d:%02d:%02d\n", current_date->tm_year + 1900, current_date->tm_mon + 1, current_date->tm_mday, current_date->tm_hour, current_date->tm_min, current_date->tm_sec);
    	cout << "the current time is" << sCurDate << std::endl;
	}
	return;
}

void LightTimer_handler(int signal_number)
{
	sysLightTimerMngr* pTimerMngr = LedLightServer::GetCurrentServer()->GetLightTimerMngr();
	static int num_alr = 0;
	switch(signal_number)   
	{   
		case SIGALRM:
			num_alr++;
			
			{
				CLightTimerMap  *tmpLightTimerMap = pTimerMngr->GetLightTimerMap();
			    for(CLightTimerMap::iterator iter = tmpLightTimerMap->begin(); iter != tmpLightTimerMap->end(); iter++  )
			    {
			    	//cout << "Light Blink: [ "  << iter->second.t_timer << " ! " << iter->first << " : "<< iter->second.light_color  << " ]....."<< endl;
			    	if (!(iter->second.t_timer % 2))
					{
						LightCntlApi api;
						api.Onlight(iter->first, iter->second.light_color);
					}
					else
					{
						LightCntlApi api;
						api.Offlight(iter->first);
					}
					iter->second.t_timer++;
			    }
			}
			break;   
		default:   
			cout << "Error: get None Signal Alarm, do nothing!!! " << endl;   
			break;   
	}
}

sysLightTimerMngr::sysLightTimerMngr()
{

}

sysLightTimerMngr::~sysLightTimerMngr()
{
  //do nothing
}
void sysLightTimerMngr::AddTimerItem(lightInfo* timer)
{
    Lock();
	//if (m_lightstimer_map.find(timer->light_type) != m_lightstimer_map.end())
	{
		lightInfo tmp_info(timer->light_type, timer->light_color, timer->light_act, 0);
    	m_lightstimer_map[timer->light_type] = tmp_info;
	}
    Unlock();
}

void sysLightTimerMngr::DelTimerItem(lightInfo* timer)
{
    Lock();
	CLightTimerMap::iterator iter = m_lightstimer_map.find(timer->light_type); 
	if(iter != m_lightstimer_map.end()) 
	{ 
		m_lightstimer_map.erase(iter);
	} 
    Unlock();
}

void sysLightTimerMngr::DelAllItems()
{
    Lock();
    m_lightstimer_map.clear();

    Unlock();
}


void sysLightTimerMngr::SetTimerInterval(int tv_sec,int tv_usec, int itv_sec, int itv_usec)
{
    tick.it_value.tv_sec = tv_sec;
    tick.it_value.tv_usec = tv_usec;
    tick.it_interval.tv_sec  = itv_sec;
    tick.it_interval.tv_usec = itv_usec;
}

void sysLightTimerMngr::Run()
{
    Lock();
    signal(SIGALRM, LightTimer_handler);

    int ret = setitimer(ITIMER_REAL, &tick, NULL);
    if ( ret != 0)
    {
        printf("Set timer error. %s \n", strerror(errno) );
		Unlock();
        return;
    }

    Unlock();
}

