/*
    Copyright (c) 2003-2005 Pigeon Point Systems.
    All rights reserved.
    
    Description:
        This header file defines the interfaces
    provided by the upgradefw utility modules.
    
    $RCSfile: upgrade_io.h,v $
    $Revision: 1.2.10.2 $
*/

#ifndef __UPGRADE_IO_H__
#define __UPGRADE_IO_H__

/* maximum message size */
#define MAX_MSG     0x100

/* error codes */
#define ERR_TIMEOUT         0x8000
#define ERR_INVALID_RESPONSE    0x4000
#define ERR_SYNTAX          0x0100

#ifdef _WIN32
#include <windows.h>
#define SLEEP(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP(ms) usleep((ms)*1000)
#endif

/* interface provided by upgrade_io.c */
int upgrade_open(int, char*, char*);
int upgrade_close(int nFd);
int upgrade_start(int nFd);
int upgrade_status(unsigned char* , int nFd);
int upgrade_prepare(int target , int nFd);
int upgrade_complete(int nFd);
int upgrade_restore(int nFd);
int upgrade_write(int target, unsigned long addr, int n, void *data , int nFd);
int upgrade_get_backup_version(unsigned char* , int nFd);

const char *upgrade_if_name(int type);

/* link layer driver object */
typedef struct _link_driver {
    int type;
    const char *name;
    int (*open)(int type, char *name, char *param);
    int (*close)(int nFd);
    int (*request)(int netfn_rslun, unsigned char *data, int *length , int nFd);
} link_driver;

/* debug interfaces */
extern int upgrade_debug;
void dump_packet(unsigned char *data, int n, char *comment);

#endif /* __UPGRADE_IO_H__ */
