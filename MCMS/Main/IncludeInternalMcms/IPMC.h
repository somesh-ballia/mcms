#if !defined(_IPMC_H__)
#define _IPMC_H__

enum eLedColor
{
    eRed   = 2,
    eGreen = 3,
    eAmber = 4,
    LAST_LED_COLOR
};

enum eLedState
{

    eTurnOff  = 0,
    eTurnOn = 1,
    eFlickering= 2,
    eNoState  // should be the last

};


    

#endif // !defined(_IPMC_H__)
