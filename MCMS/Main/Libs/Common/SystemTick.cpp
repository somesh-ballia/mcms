

#include "SystemTick.h"

/////////////////////////////////////////////////////////////////
CSystemTick::CSystemTick()
{
    m_self.tv_sec = 0;
    m_self.tv_usec = 0;
}

/////////////////////////////////////////////////////////////////
CSystemTick::CSystemTick(DWORD centSec)
{
    m_self.tv_sec = centSec / 100;
    m_self.tv_usec = (centSec % 100) * 10000;
}

/////////////////////////////////////////////////////////////////
CSystemTick::CSystemTick(DWORD sec,DWORD uSec)
{
    m_self.tv_sec = sec;
    m_self.tv_usec = uSec;
}

/////////////////////////////////////////////////////////////////
DWORD CSystemTick::GetIntegerPartForTrace() const
{
    return m_self.tv_usec / 10000 + m_self.tv_sec*100;
}

/////////////////////////////////////////////////////////////////
DWORD CSystemTick::GetSeconds() const
{
	return m_self.tv_sec;
}

/////////////////////////////////////////////////////////////////
DWORD CSystemTick::GetMiliseconds() const
{
	return m_self.tv_sec * 1000 +  m_self.tv_usec / 1000;
}


/////////////////////////////////////////////////////////////////
CSystemTick CSystemTick::operator-(const CSystemTick& other) const
{
    // assuming this > other
    

    if (m_self.tv_usec > other.m_self.tv_usec)
        return CSystemTick(m_self.tv_sec-other.m_self.tv_sec,
                           m_self.tv_usec-other.m_self.tv_usec);

    // take 1 sec from bigger base
    return CSystemTick((m_self.tv_sec-1)-other.m_self.tv_sec,
                       (m_self.tv_usec+1000000)-other.m_self.tv_usec);
    
}

/////////////////////////////////////////////////////////////////
CSystemTick CSystemTick::operator+(const CSystemTick& other) const
{
    DWORD newusec = m_self.tv_usec+other.m_self.tv_usec;
    if (newusec > 999999)
        return CSystemTick(m_self.tv_sec+other.m_self.tv_sec+1,
                            newusec - 1000000);

    return CSystemTick(m_self.tv_sec + other.m_self.tv_sec,newusec);
}

/////////////////////////////////////////////////////////////////
int operator==(const CSystemTick& l,const CSystemTick& r)
{
    return (l.m_self.tv_sec == r.m_self.tv_sec &&
            l.m_self.tv_usec == r.m_self.tv_usec);
}

/////////////////////////////////////////////////////////////////
int operator!=(const CSystemTick& l, const CSystemTick & r)
{
    return (l.m_self.tv_sec != r.m_self.tv_sec ||
            l.m_self.tv_usec != r.m_self.tv_usec);
}

/////////////////////////////////////////////////////////////////
int operator<(const CSystemTick& l,const CSystemTick& r)
{
    return (l.m_self.tv_sec < r.m_self.tv_sec ||
            (l.m_self.tv_sec == r.m_self.tv_sec && l.m_self.tv_usec < r.m_self.tv_usec));
}

/////////////////////////////////////////////////////////////////
int operator>(const CSystemTick& l,const CSystemTick& r)
{
    return (l.m_self.tv_sec > r.m_self.tv_sec ||
            (l.m_self.tv_sec == r.m_self.tv_sec && l.m_self.tv_usec > r.m_self.tv_usec));
    
}

/////////////////////////////////////////////////////////////////
std::ostream& operator<< (std::ostream& os, const CSystemTick& ticks)
{
    os << ticks.GetIntegerPartForTrace();
    // todo
    return os;
}
