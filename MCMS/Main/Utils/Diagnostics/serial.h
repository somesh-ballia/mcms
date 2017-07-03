/*
    Copyright (c) 2003-2005 Pigeon Point Systems.
    All rights reserved.

    Description:
        Header file for low-level serial port
	access routines

    $RCSfile: serial.h,v $
    $Revision: 1.2.10.2 $
*/

#ifndef __SERIAL_H__
#define __SERIAL_H__

int serial_open(char*);
int serial_set_param(char* , int);
int serial_close(int nFd);
int serial_write_line(char* , int);
int serial_write(char*, int , int);
int serial_read_line(char*, int , int);
int serial_read(char*, int , int);
int serial_flush(int);
int serial_set_timeout(int);
int serial_set_baud(int);
int serial_updateIRQ(unsigned int newIrq , int) ;

#endif /* __SERIAL_H__ */
