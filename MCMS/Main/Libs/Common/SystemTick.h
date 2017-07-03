

#ifndef _SYSTEM_TICKS_H_
#define _SYSTEM_TICKS_H_

#include <sys/time.h>
#include <ostream>
#include "DataTypes.h"

struct CSystemTick
{
    CSystemTick();
    CSystemTick(DWORD psosTicks); // 1/100 sec
    CSystemTick(DWORD sec,DWORD uSec); // sec,usec

    DWORD GetIntegerPartForTrace() const;
    DWORD GetSeconds() const;
    DWORD GetMiliseconds() const;
    
    struct timeval m_self;
    CSystemTick operator-(const CSystemTick& other) const;
    CSystemTick operator+(const CSystemTick& other) const;

};

std::ostream& operator<< (std::ostream& os, const CSystemTick& tick);
int operator==(const CSystemTick&,const CSystemTick&);
int operator!=(const CSystemTick&,const CSystemTick&);
int operator<(const CSystemTick&,const CSystemTick&);
int operator>(const CSystemTick&,const CSystemTick&);

#endif 
