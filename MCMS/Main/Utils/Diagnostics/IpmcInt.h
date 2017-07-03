#ifndef _IPMCINT_H_
#define _IPMCINT_H_


enum eLedColor
{
    eRed   = 2,
    eGreen = 3,
    eAmber = 4
};

enum eLedState
{
    eTurnOff  = 0,
    eTurnOn = 1,
    eFlickering= 2,
};

typedef enum
{
	eIpmcSerialOpenFail                  = -1,
	eIpmcSerialUpdateIRQFail             = -2,
	eIpmcSerialSetParamFail              = -3,
	eIpmcOpenInterfaceFail               = -4,
	eIpmcWriteIpmcFail                   = -5,
	eIpmcRcvHexResponseFail              = -6,
	eIpmcGetHWSlotIdFail                 = -7,
	eIpmcReadEepromFail                  = -8,
	eIpmcGetFruLedPropertiesFail         = -9,
	eIpmcGetLedColorCapabilitiesFuncFail = -10,
    eIpmcReadIPMCVersionFromChipFail     = -11,
	eIpmcSuccess                         = 0

} 	EIpmcStatus;


#define IPMC_SERIAL_RATE  "9600"


#ifdef MFA_BOARD_PRINT
#define SERIAL_IRQ		  90
#else
#define SERIAL_IRQ        4
#endif

#define LINES_LIMIT       3
#define MAX_MSG           0x100
#define ERR_INVALID_RESPONSE  0x4000
#define ERR_SYNTAX            0x0100
#define SLEEP(ms)  usleep(ms *(1000))
#define ERR_TIMEOUT           0x8000
#define GetHWSlotIdReq          "[B8 00 05 00 40 0A]\r"

#define GetFruLedProperties     "[B0 00 05 00 00]\r"
#define GetLedColorCapabilities "[B0 00 06 00 00"      //need to complite the led id
#define SetFruLedState          "[B0 00 07 00 00"      //need to complite the led id and the blink duration


#define SetWatchDog            "[18 00 24 c0 01 01 00"// 20 00]\r"
#define ResetWatchDog          "[18 00 22]\r"
#define TurnOffWatchDog        "[18 00 24 80 00 00 00 00 00]\r"
#define SetCPUTemperature      "[48 00 0F" // tt]\r"
#define SetHDTemperature       "[48 00 10" // tt]\r"


#define SetKeepAliveResult      "[48 00 01"
#define SetPostResult           "[48 00 02"

#define SetPowerOffCpuData      "[48 00 0C]\r"

#define GetVersionNumber        "[48 00 F2]\r"

//#define LED_SUPPORT_BLUE        0x02
#define LED_SUPPORT_RED         0x04
#define LED_SUPPORT_GREEN       0x08
#define LED_SUPPORT_AMBER       0x10
#define LED_SUPPORT_ORANGE      0x20
#define LED_SUPPORT_WHITE       0x30



#define TURN_ON   1
#define TURN_OFF  0
#define BLINK_LED 2


#define RED       2
#define GREEN     3
#define AMBER     4


#define BOARD_EEPROM   0
#define BP_EEPROM      1

#define READ_EEPROM    0
#define WRITE_EEPROM   1

#define U_KEY_OFFSET    0x200
#define K_KEY_OFFSET    0x250

#define MAX_EEPROM_BYTES_PER_READ       0x0b
#define MAX_EEPROM_BYTES_PER_WRITE      0x16


#define ReadBoardEepromStartHeaderReq  "[28 00 11 00"
#define WriteBpEepromStartHeaderReq    "[28 00 12 01"
#define ReadBpEepromStartHeaderReq     "[28 00 11 01"

extern INT32 open_ipmc_interface() ;
extern INT32 write_ipmc_interface(char *str , int nFd) ;
extern void close_ipmc_interface(int nFd) ;
extern INT32 IpmcGetHWSlotId();
extern INT32 LedInterface(UINT32 unLedColor , UINT32 unStatus);
extern INT32 IpmcIsDiagMode(void);

#endif //_IPMCINT_H_
