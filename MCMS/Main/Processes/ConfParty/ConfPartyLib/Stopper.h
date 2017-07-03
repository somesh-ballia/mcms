#ifndef _STOPPER_H_
#define _STOPPER_H_

#include <sys/time.h>
#include <sstream>
#include "DataTypes.h"
#include <vector>
#include <map>
#include "SystemTick.h"
#include "ConfPartySharedDefines.h"

class CStopperTimeStamp
{
public:
	CStopperTimeStamp();
	virtual ~CStopperTimeStamp();

	CStopperTimeStamp operator-(const CStopperTimeStamp& other) const;
	void Dump(std::ostream& msg);
	void DumpMs(std::ostream& msg);

protected:
	CSystemTick m_ts;
};

typedef std::vector<CStopperTimeStamp> StopperTimesVector;

class CStopper
{
public:
	CStopper(const char* stopper_name = "stopper", const char* user_name = "", bool start = true);
	CStopper(const CStopper& rh);
	virtual ~CStopper();

	void Start();
	void AddTime();
	void Stop(bool print = true);
	void AddUserName(const char* user_name);
	void Dump(std::ostream& msg) const;
	void Dump() const;
	CStopper& operator=(const CStopper& rh);

protected:
	void AddTimeStamp();

	std::string m_name;
	std::string m_user_name;
	StopperTimesVector m_timesVector;
};

typedef std::map<PartyRsrcID, CStopper> PerformanceStatistics;

#endif
