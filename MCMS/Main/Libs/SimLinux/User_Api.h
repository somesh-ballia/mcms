/* Portwell API releated value */

//Voltage read value define
typedef enum 
{
	VOL_VCORE=1,
	VOL_1P5V,
	VOL_3P3V,
	VOL_5V,
	VOL_12V
} VOLTAGE_TYPE;

//Fan value define
typedef enum  
{
    CPUFAN = 1,
    SYSFAN,
    AUXFAN
} FANTYPE;

typedef enum
{
    SM_WRITE = 0,
    SM_READ
} SMBUS_TYPE;

//SMBus Slave address
#define BUS_SLAVE   0x5E

int PET_API_Init ();
int PET_API_Uninit ();
int PET_API_GetVersion (char *api_version);
int PET_API_SetIOSleepTime (int msec);
int PET_Board_GetBIOSVersion (char *bios_version);
int PET_Board_GetPlatformName (char *name);

int PET_GPIO_TotalSet (int *gpio_set_number);
int PET_GPIO_TotalNumber (int gpio_set, int *available_pin_number);
int PET_GPIO_SetDirection (int gpio_set, unsigned char io_direction);
int PET_GPIO_SetPinDirection (int gpio_set, int pin_num, unsigned char io_direction);
int PET_GPIO_ReadDirection (int gpio_set, unsigned char *io_direction);
int PET_GPIO_ReadPinDirection (int gpio_set, int pin_num, unsigned char *io_direction);
int PET_GPIO_Read (int gpio_set, unsigned char *value);
int PET_GPIO_ReadPin (int gpio_set, int pin_num, unsigned char *value);
int PET_GPIO_Write (int gpio_set, unsigned char value);
int PET_GPIO_WritePin (int gpio_set, int pin_num, unsigned char value);

int PET_WDT_Available ();
int PET_WDT_GetRange (int type, int *minimum, int *maximum);
int PET_WDT_SetConfig (int type, int timeout);
int PET_WDT_GetConfig (int *type, int *timeout);
int PET_WDT_Trigger ();
int PET_WDT_Disable ();

int PET_HWM_CPUNumber (int *cpu_num);
int PET_HWM_FanNumber (int *fan_num);
int PET_HWM_CPUTemperature (int cpu_num, float *cputemp_value);
int PET_HWM_SysTemperature (float *systemp_value);
int PET_HWM_Voltage (int vol_type, float *vol_value);
int PET_HWM_GetFanSpeed (int fan_type, int *fan_value);
int PET_HWM_SetFanSpeed (int fan_type, unsigned char fan_value);
int PET_HWM_BeepAlarm (int enable);

int PET_SMBus_Available ();
int PET_SMBus_Quick (int r_w);
int PET_SMBus_Byte (int r_w, unsigned char dev_addr, unsigned char *value);
int PET_SMBus_ByteData (int r_w, unsigned char dev_addr, unsigned char offset, unsigned char *value);
int PET_SMBus_WordData (int r_w, unsigned char dev_addr, unsigned char offset, unsigned int *value);
int PET_SMBus_ProcessCall (unsigned char dev_addr, unsigned char offset, unsigned int *w_value, unsigned int *r_value);
int PET_SMBus_Block (int r_w, unsigned char dev_addr, unsigned char offset, int byte_count,unsigned int *value);
int PET_SMBus_BlockProcess (unsigned char dev_addr, unsigned char offset, int byte_count, unsigned int *w_value, unsigned int *r_value);
