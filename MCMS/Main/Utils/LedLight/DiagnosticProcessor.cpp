
#include "DiagnosticProcessor.h"
#include "LedLightServer.h"
#include "LightCntlApi.h"


//compile comand: g++ -g cppTest.cpp -o gTest
//g++  cppTest.cpp -o cTest
DiagnosticProcessor::DiagnosticProcessor()
{
	//Do nothing
}

DiagnosticProcessor::~DiagnosticProcessor()
{
	//Do nothing
}

void DiagnosticProcessor::InitCliTable()
{
	cout << "DiagnosticProcessor Init CLI Table !!! " << endl;
	ADD_CLI("diagnostic_completed",DiagnosticProcessor::DiagnosticCompleted,"system Alarm");
	ADD_CLI("diagnostic_in_progress",DiagnosticProcessor::DiagnosticInProgress,"system Alarm");
}

void DiagnosticProcessor::DiagnosticCompleted(const std::string & parameters)
{
	//Status Blue On
	LightCntlApi api;
	api.LightOn(eLightStatus,eLightBlue);
}

void DiagnosticProcessor::DiagnosticInProgress(const std::string & parameters)
{
	//Status Blue Blink
	LightCntlApi api;
	api.LightBlink(eLightStatus,eLightBlue);
}

