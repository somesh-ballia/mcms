#include <stdlib.h>


#define TIMER_START             _IOWR(WATCHDOG_IOCTL_BASE, 10, int)
#define TIMER_STOP              _IOWR(WATCHDOG_IOCTL_BASE, 11, int)
#define TIMER_SETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 12, int)
#define TIMER_GETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 13, int)
#define TIMER_KEEPALIVE			_IOWR(WATCHDOG_IOCTL_BASE, 14, int)
#define WATCHDOG_TIMEOUT        15 //seconds
#define WATCHDOG_RESET_TIMEOUT  60

void open_wdt_device(void);


void close_wdt_device(void);


void Set_Start_WDT(int timeout);


void close_WDT_timer(int timeout);

void disable_WDT();
void enable_WDT();

