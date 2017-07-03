/*
    Copyright (c) 2003-2005 Pigeon Point Systems.
    All rights reserved.

    Description:
        This module contains source code for
	low-level serial port access on Linux

    $RCSfile: serial-linux.c,v $
    $Revision: 1.2.10.3 $
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <linux/serial.h>

#include <serial.h>

//static int dev = -1;
static int ms_timeout = 1000;

/*
    Read a line from serial port
    Returns > 0 if there is a line,
    0 on timeout, < 0 on error
*/
int serial_read_line(char *str, int n , int nFd)
{
    int x, i;
    struct pollfd pfd;

    if (!str) {
        return -1;
    }
    *str = 0;
    i = 0;
    while (i < n) {
        pfd.fd = nFd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        x = poll(&pfd, 1, ms_timeout);
        if (x == 0) {
            return 0;
        }
        if (x < 0) {
            return x;
        }
        x = read(nFd, str + i, 1);
        if (x < 0) {
            return x;
        }
        if (x == 0) {
            return x;
        }
        if (str[i] == '\n') {
            str[i] = 0;
            i++;
            return i;
        } else {
            i++;
        }
    }
    return 0;
}

/*
    Read the message in basic mode
*/
int serial_read(char *buf, int cb , int nFd)
{
    int x, i;
    struct pollfd pfd;

    if (NULL == buf) {
        return -1;
    }
    i = 0;
    for (i = 0; i < cb; i++) {
        pfd.fd = nFd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        x = poll(&pfd, 1, ms_timeout);
        if (x == 0) {
            return 0;
        }
        if (x < 0) {
            perror("serial_read failed: ");
            return x;
        }
        x = read(nFd, buf + i, 1);
        if (x <= 0) {
            return x;
        }
    }
    return cb;
}

/*
    Read the message in basic mode
*/
int serial_read_ex(char *buf, int cb , int nFd)
{
    int x, i;
    struct pollfd pfd;

    if (NULL == buf) {
        return -1;
    }
    i = 0;
    for (i = 0; i < cb; i++) {
        pfd.fd = nFd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        x = poll(&pfd, 1, 1000);
        if (x == 0) {
            return i;
        }
        if (x < 0) {
            perror("serial_read failed: ");
            return i;
        }
        x = read(nFd, buf + i, 1);
        if (x <= 0) {
            return i;
        }
    }
    return cb;
}


/*
    Send zero-terminated string to serial port
    Returns the string length or negative error code
*/
int serial_write_line(char *str , int nFd)
{
    return write(nFd, str, strlen(str));
}

/*
    Send basic mode message.
*/
int serial_write(char *buf, int cb , int nFd)
{
    return write(nFd, buf, cb);
}

/*
    Flush the Tx buffers
*/
int serial_flush(int nFd)
{
    return ioctl(nFd, TCFLSH, TCIOFLUSH);
}

/*
    Set I/O timeout
*/
int serial_set_timeout(int ms_t)
{
    ms_timeout = ms_t;
    return 0;
}

/*
    Table of supported baud rates
*/
static int rates[] = {
    B2400, 2400,
    B9600, 9600,
    B19200, 19200,
    B38400, 38400,
    B57600, 57600,
    B115200, 115200,
    -1, -1
};

/*
    Set serial port parameters
    Format: <baud rate>
    "9600"
*/
int serial_set_param(char *param , int nFd)
{
    struct termios ti;
    long rate;
    int x, i;

    rate = atol(param);
    x = -1;
    for (i = 0; rates[i] != -1; i += 2) {
        if (rates[i + 1] == rate) {
            x = rates[i];
            break;
        }
    }
    if (x == -1) {
	return x;
    }

    tcgetattr(nFd, &ti);

    /* set baud rate */
    cfsetspeed(&ti, x);

    /* 8N1 */
    ti.c_cflag &= ~PARENB;
    ti.c_cflag &= ~CSTOPB;
    ti.c_cflag &= ~CSIZE;
    ti.c_cflag |= CS8;

    /* enable the receiver and set local mode */
    ti.c_cflag |= (CLOCAL | CREAD);

    /* no flow control */
    ti.c_cflag &= ~CRTSCTS;
    ti.c_iflag &= ~(IGNBRK | IGNCR | INLCR | ICRNL | IUCLC | INPCK | ISTRIP
            | IXON | IXOFF | IXANY);

    ti.c_oflag &= ~(OPOST);
    ti.c_lflag &= ~(ICANON | ISIG | ECHO | XCASE | ECHONL | NOFLSH);

    /* set the new options for the port with flushing */
    tcsetattr(nFd, TCSAFLUSH, &ti);
    return 0;
}

/*
    Open the serial port
*/
int serial_open(char *name)
{
    int dev = -1;

    if (!name)
    {
        return -1;
    }

    dev = open(name, O_RDWR | O_NONBLOCK, 0);
    if (dev < 0)
    {
        perror(name);
    }

    return dev;
}

/*
    Close serial port
*/
int serial_close(int nFd)
{
    if (nFd < 0) {
        return nFd;
    }
    return close(nFd);
}

/*
 * Update serial IRQ
 */

int serial_updateIRQ(unsigned int newIrq , int nFd)
{
	struct serial_struct serial_irq,old_serinfo ;

	if(ioctl(nFd,TIOCGSERIAL,&old_serinfo) < 0)
	{
		perror("can not get old serial info \n") ;
		return 0 ;
	}
	//printf("old serial IRQ = %d \n",old_serinfo.irq) ;

	if((unsigned int)old_serinfo.irq == newIrq)
		return 1 ;

	//serial_irq = old_serinfo ;
	memcpy(&serial_irq,&old_serinfo,sizeof(struct serial_struct)) ;

	serial_irq.irq = newIrq ;

	if(ioctl(nFd,TIOCSSERIAL,&serial_irq) < 0)
	{
		perror("can not set serial irq \n") ;
		return 0 ;
	}

	return 1 ;


}
