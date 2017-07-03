
#include "SystemAlarmProcessor.h"
#include "LedLightServer.h"
#include "LightCntlApi.h"


//compile comand: g++ -g cppTest.cpp -o gTest
//g++  cppTest.cpp -o cTest
SystemAlarmProcessor::SystemAlarmProcessor()
{
	//Do nothing
	with_alarms = false;
	with_eps = false;
}

SystemAlarmProcessor::~SystemAlarmProcessor()
{
	//Do nothing
}

void SystemAlarmProcessor::InitCliTable()
{
	cout << "SystemAlarmProcessor Init CLI Table !!! " << endl;
	ADD_CLI("add_alarm",SystemAlarmProcessor::SystemAddAlarm,"system Alarm");
	ADD_CLI("del_alarm",SystemAlarmProcessor::SystemDelAlarm,"no system Alarm, no endpoint join in.");
	//ADD_CLI("del_alarm_no_endpoint",SystemAlarmProcessor::SystemWithoutAlarmNoEp,"no system Alarm, no endpoint join in.");
	//ADD_CLI("del_alarm_endpoint_join",SystemAlarmProcessor::SystemWithoutAlarmEps, "no system Alarm, endpoint join in.");
	ADD_CLI("add_ep",SystemAlarmProcessor::SystemAddEp,"system Alarm");
	ADD_CLI("del_ep",SystemAlarmProcessor::SystemDelEp,"no system Alarm, no endpoint join in.");
}

void SystemAlarmProcessor::SystemAddAlarm(const std::string & parameters)
{
	with_alarms = true;
	//LCD-STATUS Red On
	LightCntlApi api;
	api.LightOn(eLightStatus,eLightRed);
}

void SystemAlarmProcessor::SystemDelAlarm(const std::string & parameters)
{
	with_alarms = false;
	if (true == with_eps)
	{
		SystemWithoutAlarmEps();
	}
	else
	{
		SystemWithoutAlarmNoEp();
	}
}

void SystemAlarmProcessor::SystemAddEp(const std::string & parameters)
{
	with_eps = true;
	if (false == with_alarms)
	{
		SystemWithoutAlarmEps();
	}
	
}

void SystemAlarmProcessor::SystemDelEp(const std::string & parameters)
{
	with_eps = false;
	if (false == with_alarms)
	{
		SystemWithoutAlarmNoEp();
	}
}


void SystemAlarmProcessor::SystemWithoutAlarmEps()
{
	//LCD-STATUS Green Blink
	LightCntlApi api;
	api.LightOn(eLightStatus,eLightGreen);
}

void SystemAlarmProcessor::SystemWithoutAlarmNoEp()
{
	//LCD-STATUS Green Blink
	LightCntlApi api;
	api.LightBlink(eLightStatus,eLightGreen);
}

