#include "LightCntlApi.h"
#include "LedLightServer.h"

/*---------------------------------------------------------------------------------------
  Implementation of Publics
---------------------------------------------------------------------------------------*/
LightCntlApi::LightCntlApi()
{
}

LightCntlApi::~LightCntlApi()
{
  //do nothing
}

void LightCntlApi::LightInit()
{
	//Call hardware API to init light
	cout << " ++++++++++++ Led Light is Opened !!! +++++++++" << endl;
	open_led();
	return;
}


void LightCntlApi::LightClose()
{
	//Call hardware API to close light
	cout << " ++++++++++++ Led Light is Closed !!! +++++++++" << endl;
	close_led();	
	return;
}

void LightCntlApi::Onlight(eLightType light_type, eLightColor light_color)
{	
	//Call hardware API to off light
	//cout << "Light: [ " << LightTypeStr[light_type] << " : " << LightColorStr[light_color] << " ] On On On ... "  << endl;
	set_led(LEDCNTL_TYPE[light_type],(char)LEDCNTL_COLOR[light_color]);
	return;
}

void LightCntlApi::Offlight(eLightType light_type)
{	
	//Call hardware API to off light
	//cout << "Light: [ " << LightTypeStr[light_type] << " ] Off Off Off ... "  << endl;
	set_led(LEDCNTL_TYPE[light_type],COLOR_DOWN);
	return;
}

void LightCntlApi::LightOn(eLightType light_type, eLightColor light_color)
{
	cout << "Light: [ " << LightTypeStr[light_type] << " : " << LightColorStr[light_color] << " ] On On On ... " << endl;
	lightInfo tmpLightInfo;
	tmpLightInfo.light_type = light_type;
	tmpLightInfo.light_color = light_color;
	LedLightServer::GetCurrentServer()->GetLightTimerMngr()->DelTimerItem(&tmpLightInfo);
	Onlight(light_type,light_color);
	return;
}

void LightCntlApi::LightOff(eLightType light_type)
{
	cout << "Light: [ " << LightTypeStr[light_type] << " ] Off Off Off ... " << endl;
	lightInfo tmpLightInfo;
	tmpLightInfo.light_type = light_type;
	LedLightServer::GetCurrentServer()->GetLightTimerMngr()->DelTimerItem(&tmpLightInfo);
	Offlight(light_type);
	return;
}

void LightCntlApi::LightBlink(eLightType light_type, eLightColor light_color)
{
	cout << "Light: [ " << LightTypeStr[light_type] << " : " << LightColorStr[light_color] << " ] Blink Blink Blink ... " << endl;
	lightInfo tmpLightInfo;
	tmpLightInfo.light_type = light_type;
	tmpLightInfo.light_color = light_color;
	LedLightServer::GetCurrentServer()->GetLightTimerMngr()->AddTimerItem(&tmpLightInfo);
	return;
}


