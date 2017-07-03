#include "Stopper.h"
#include "Trace.h"
#include "TraceStream.h"
#include "SystemFunctions.h"

#define NSEC_IN_SEC 1000000000
#define eStopperLogLevel eLevelError

////////////////////////////////////////////////////////////////////////////
//               CStopperTimeStamp
////////////////////////////////////////////////////////////////////////////
CStopperTimeStamp::CStopperTimeStamp()
{
	m_ts = SystemGetTickCount();
}

////////////////////////////////////////////////////////////////////////////
CStopperTimeStamp::~CStopperTimeStamp()
{
}

////////////////////////////////////////////////////////////////////////////
CStopperTimeStamp CStopperTimeStamp::operator-(const CStopperTimeStamp& other) const
{
	CStopperTimeStamp retVal;
	retVal.m_ts = m_ts - other.m_ts;
	return retVal;
}

////////////////////////////////////////////////////////////////////////////
void CStopperTimeStamp::Dump(std::ostream& msg)
{
	msg << m_ts.m_self.tv_sec << ":" << m_ts.m_self.tv_usec;
}

////////////////////////////////////////////////////////////////////////////
void CStopperTimeStamp::DumpMs(std::ostream& msg)
{
	msg << m_ts.GetMiliseconds();
}

////////////////////////////////////////////////////////////////////////////
//               CStopper
////////////////////////////////////////////////////////////////////////////
CStopper::CStopper(const char* stopper_name, const char* user_name, bool start)
{
	m_name = stopper_name;
	m_user_name = user_name;
	if (start)
		Start();
}

////////////////////////////////////////////////////////////////////////////
CStopper::CStopper(const CStopper& rh)
{
	m_name        = rh.m_name;
	m_user_name   = rh.m_user_name;
	m_timesVector = rh.m_timesVector;
}

////////////////////////////////////////////////////////////////////////////
CStopper::~CStopper()
{
}

////////////////////////////////////////////////////////////////////////////
void CStopper::Start()
{
	m_timesVector.clear();
	AddTimeStamp();
}

////////////////////////////////////////////////////////////////////////////
void CStopper::AddTime()
{
	AddTimeStamp();
}

////////////////////////////////////////////////////////////////////////////
void CStopper::Stop(bool print)
{
	AddTimeStamp();
	if (print)
		Dump();
}

////////////////////////////////////////////////////////////////////////////
void CStopper::AddTimeStamp()
{
	CStopperTimeStamp start_time;
	m_timesVector.push_back(start_time);
}

////////////////////////////////////////////////////////////////////////////
void CStopper::AddUserName(const char* user_name)
{
	m_user_name = user_name;
}

////////////////////////////////////////////////////////////////////////////
void CStopper::Dump(std::ostream& msg) const
{
	msg << m_name << "," << m_user_name << ": ";
	if (0 == m_timesVector.size())
	{
		msg << " No times";
		return;
	}
	CStopperTimeStamp current_interval;
	msg << "Total:";
	current_interval = m_timesVector[m_timesVector.size() - 1] - m_timesVector[0];
	current_interval.Dump(msg);
	msg << "  ";

	for (unsigned int times_index = 1; times_index < m_timesVector.size(); times_index++)
	{
		current_interval = m_timesVector[times_index] - m_timesVector[times_index - 1];
		msg << " " << times_index << ":";
		current_interval.Dump(msg);
	}
}

////////////////////////////////////////////////////////////////////////////
void CStopper::Dump() const
{
	std::ostringstream msg;
	Dump(msg);
	FTRACESTRFUNC(eStopperLogLevel) << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
CStopper& CStopper::operator=(const CStopper& rh)
{
	if (&rh == this)
		return *this;

	m_name        = rh.m_name;
	m_user_name   = rh.m_user_name;
	m_timesVector = rh.m_timesVector;
	return *this;
}
