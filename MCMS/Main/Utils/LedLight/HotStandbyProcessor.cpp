
#include "HotStandbyProcessor.h"
#include "LedLightServer.h"
#include "LightCntlApi.h"


//compile comand: g++ -g cppTest.cpp -o gTest
//g++  cppTest.cpp -o cTest
HotStandbyProcessor::HotStandbyProcessor()
{
	//Do nothing
}

HotStandbyProcessor::~HotStandbyProcessor()
{
	//Do nothing
}

void HotStandbyProcessor::InitCliTable()
{
	cout << "HotStandbyProcessor Init CLI Table !!! " << endl;
	ADD_CLI("single_mode",HotStandbyProcessor::SingleMode,"system Alarm");
	ADD_CLI("is_master",HotStandbyProcessor::HotStandbyMaster,"system Alarm");
	ADD_CLI("is_slave",HotStandbyProcessor::HotStandbySlave,"no system Alarm, no endpoint join in.");
}

void HotStandbyProcessor::SingleMode(const std::string & parameters)
{
	//M/S Green On
	LightCntlApi api;
	api.LightOn(eLightMS,eLightGreen);
}

void HotStandbyProcessor::HotStandbyMaster(const std::string & parameters)
{
	//M/S Green On
	LightCntlApi api;
	api.LightOn(eLightMS,eLightGreen);
}

void HotStandbyProcessor::HotStandbySlave(const std::string & parameters)
{
	//M/S Green Blink
	LightCntlApi api;
	api.LightBlink(eLightMS,eLightGreen);
}

