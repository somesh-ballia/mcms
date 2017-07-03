#ifndef LIGHT_CNTL_API_H_
#define LIGHT_CNTL_API_H_

#include "BaseProcessor.h"
//#include "ledCntlApi.h"
//#include "./ledApi/tusb_led_api.h"
#include "ledApi/tusb_led_api.h"

static const eLED LEDCNTL_TYPE[NumOfLightType] ={
    eSTATUS,
    eMS
};

static const char LEDCNTL_COLOR[NumOfLightColor] ={
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

class LedLightServer;

class LightCntlApi
{
public:
	LightCntlApi();
    virtual ~LightCntlApi();
    void LightInit();
    void LightClose();

    void Onlight(eLightType light_type, eLightColor light_color);

    void Offlight(eLightType light_type);
    void LightOn(eLightType light_type, eLightColor light_color);

    void LightOff(eLightType light_type);

    void LightBlink(eLightType light_type, eLightColor light_color);

private:

};

#endif // #if !defined(_CTerminalCommand_H__)

