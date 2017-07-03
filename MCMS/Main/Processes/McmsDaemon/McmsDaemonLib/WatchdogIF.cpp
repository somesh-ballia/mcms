#include <stdlib.h>
#include "WatchdogIF.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>

#define TIMER_START             _IOWR(WATCHDOG_IOCTL_BASE, 10, int)
#define TIMER_STOP              _IOWR(WATCHDOG_IOCTL_BASE, 11, int)
#define TIMER_SETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 12, int)
#define TIMER_GETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 13, int)
#define TIMER_KEEPALIVE			_IOWR(WATCHDOG_IOCTL_BASE, 14, int)


int fd = 0;
//int timeout = WATCHDOG_TIMEOUT;

void open_wdt_device(void)
{
	fd = open("/dev/watchdog", O_WRONLY);

	if (fd==-1)
	{
		perror("open /dev/watchdog failed\n");
	}
}

void close_wdt_device(void)
{
	write(fd, "V", 1);
	close(fd);
	fd = 0;
}

void Set_Start_WDT(int timeout)
{
    ioctl(fd, WDIOC_SETTIMEOUT,&timeout );
}

void close_WDT_timer(int timeout)
{
    	ioctl(fd, TIMER_STOP, &timeout);
        close_wdt_device();
}

void disable_WDT()
{
    int param = 0;
    param |= WDIOS_DISABLECARD;
    ioctl(fd, WDIOC_SETOPTIONS , &param);
}

void enable_WDT()
{
    int param = 0;
    param |= WDIOS_ENABLECARD;
    ioctl(fd, WDIOC_SETOPTIONS , &param);
}
