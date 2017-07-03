
#include "UsbUpgradeProcessor.h"
#include "LedLightServer.h"
#include "LightCntlApi.h"


//compile comand: g++ -g cppTest.cpp -o gTest
//g++  cppTest.cpp -o cTest
UsbUpgradeProcessor::UsbUpgradeProcessor()
{
	//Do nothing
}

UsbUpgradeProcessor::~UsbUpgradeProcessor()
{
	//Do nothing
}

void UsbUpgradeProcessor::InitCliTable()
{
	cout << "UsbUpgradeProcessor Init CLI Table !!! " << endl;
	ADD_CLI("usb_upgrade_completed",UsbUpgradeProcessor::UsbUpgradeCompleted,"system Alarm");
	ADD_CLI("usb_upgrade_in_progress",UsbUpgradeProcessor::UsbUpgradeInProgress,"no system Alarm, no endpoint join in.");
}

void UsbUpgradeProcessor::UsbUpgradeCompleted(const std::string & parameters)
{
	//M/S Blue On
	LightCntlApi api;
	api.LightOn(eLightMS,eLightBlue);
}

void UsbUpgradeProcessor::UsbUpgradeInProgress(const std::string & parameters)
{
	//M/S Blue Blink
	LightCntlApi api;
	api.LightBlink(eLightMS,eLightBlue);
}

