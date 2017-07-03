/*
    Copyright (c) 2003-2005 Pigeon Point Systems.
    All rights reserved.

    Description:
	This module contains source code for
	Firmware Upgrade Protocol routines.
    
    $RCSfile: upgrade_io.c,v $
    $Revision: 1.3.10.4 $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <ipmidefs.h>
#include <upgdefs.h>

#include <upgrade_io.h>

/* Link-layer drivers */
extern link_driver link_serial_tm;
extern link_driver link_serial_bm;
//extern link_driver link_net;
//#ifdef _UPGRADE_IPMB
//extern link_driver link_ipmb;
//#endif

/* Link-layer driver table */
static link_driver *drivers[] = {
    &link_serial_tm,
    &link_serial_bm,
 //   &link_net,
//#ifdef _UPGRADE_IPMB
//    &link_ipmb,
//#endif
    NULL
};

/* Current link layer driver */
static link_driver *current_driver;
static int last_err = 0;

/* Message buffer */
static unsigned char msg[MAX_MSG];

/* -------------------------------------------------------------- */

/* Debug level */
int upgrade_debug = 0;

/* Dump packet */
void dump_packet(unsigned char *data, int n, char *comment)
{
    int i;
    
    if (!upgrade_debug) {
	return;
    }
    
    if (comment) {
        printf( "%s ", comment);
    }
    printf( "(%d bytes):", n);
    for (i = 0; i < n; i++) {
        printf( " %02X", data[i]);
    }
    printf("\n");
}

/* -------------------------------------------------------------- */

/*
    Send IPMI request
*/
static int link_send_packet(int netfn_lun, unsigned char *msg, int *length , int nFd)
{
    int x;

    if (current_driver) {
        if (current_driver->request) {
            x = current_driver->request(netfn_lun, msg, length , nFd);
            last_err = x;
            if (upgrade_debug) {
                printf("link driver status: %08X\n", x);
            }
            return x;
        }
    }
    return -1;
}
    
/*
    Send UPGRADE IPMI request
    msg (in): <command code> [<data>...] <place for checksum>
    msg (out): <command code> <status> [<data>...]
*/
static int upgrade_send_packet(unsigned char *msg, int *length , int nFd)
{
    int netfn_lun;
    int err, chksum, i;
    
    netfn_lun = IPMI_NETFN_FIRMWARE << 2;
    chksum = netfn_lun;
    for (i = 0; i < *length; i++) {
        chksum += msg[i];
    }    
    msg[i++] = (-chksum) & 0xFF;
    
    err = link_send_packet(netfn_lun, msg, &i , nFd);
    if (err != 0) {
        return err;
    }
    *length = i;
    return 0;
}

/* -------------------------------------------------------------- */

/*
    Firmware Upgrade Status
    st[0] <- upgrade status
    st[1] <- upgrade cause
*/
int upgrade_status(unsigned char *st , int nFd)
{
    int err, l, j;

    j = 0;
    msg[j++] = UPGRADE_STATUS;
    l = j;
    err = upgrade_send_packet(msg, &l , nFd);
    if (err != 0) {
        return err;
    }
    if (l < 2) {
        return -1;
    }
    if (l < 7) {
        return msg[1];
    }
    if (st) {
        st[0] = msg[5];
        st[1] = msg[6];
    }
    /* return status */
    return msg[1];
}

/*
    Firmware Upgrade Start
*/
int upgrade_start(int nFd)
{
    int err, l, j;

    j = 0;
    msg[j++] = UPGRADE_START;
    l = j;
    err = upgrade_send_packet(msg, &l , nFd);
    if (err != 0) {
        return err;
    }
    if (err) {
        return err;
    }
    if (l < 2) {
        return -1;
    }
    /* return status */
    return msg[1];
}

/*
    Firmware Upgrade Prepare
*/
int upgrade_prepare(int target , int nFd)
{
    int err, l, j;

    j = 0;
    msg[j++] = UPGRADE_PREPARE;
    msg[j++] = target;
    l = j;
    err = upgrade_send_packet(msg, &l , nFd);
    if (err != 0) {
        return err;
    }
    if (l < 2) {
        return -1;
    }
    if (msg[0] != UPGRADE_PREPARE) {
	return -1;
    }
    
    /* return status */
    return msg[1];
}

/*
    Firmware Upgrade Write
*/
int upgrade_write(int target, unsigned long addr, int n, void *data , int nFd)
{
    int err, l, i, j;
    unsigned char *pb;

    pb = (unsigned char*)data;
    j = 0;
    msg[j++] = UPGRADE_WRITE;
    msg[j++] = target;
    msg[j++] = (unsigned char) (addr & 0xFF);
    msg[j++] = (unsigned char) ((addr >> 8) & 0xFF);
    msg[j++] = (unsigned char) ((addr >> 16) & 0xFF);
    for (i = 0; i < n; i++) {
        msg[j++] = pb[i];
    }

    l = j;
    err = upgrade_send_packet(msg, &l , nFd);
    if (err) {
        return err;
    }
    if (l < 2) {
        return -1;
    }
    if (upgrade_debug) {
        printf("upgrade_write: status byte = %02X\n", msg[1]);
    }
    return msg[1];
}

/*
    Firmware Upgrade Complete
*/
int upgrade_complete(int nFd)
{
    int err, l, j;

    j = 0;
    msg[j++] = UPGRADE_COMPLETE;
    l = j;
    err = upgrade_send_packet(msg, &l , nFd);
    if (err != 0) {
        return err;
    }
    if (l < 2) {
        return -1;
    } 
    return msg[1];
}

/*
    Firmware Upgrade Restore Backup
*/
int upgrade_restore(int nFd)
{
    int err, j;

    j = 0;
    msg[j++] = UPGRADE_RESTORE;
    err = upgrade_send_packet(msg, &j , nFd);
    if (err) {
        return err;
    }
    if (j < 2) {
        return -1;
    } 
    return msg[1];
}

/*
    Get Backup Version
    st[0] <- major
    st[1] <- minor
*/
int upgrade_get_backup_version(unsigned char *st , int nFd)
{
    int err, l, j;

    j = 0;
    msg[j++] = UPGRADE_BACKUP_VERSION;
    l = j;
    err = upgrade_send_packet(msg, &l , nFd);
    if (err != 0) {
        return err;
    }
    if (l < 2) {
        return -1;
    }
    if (l > 2) {
        st[0] = msg[2];
    }
    if (l > 3) {
        st[1] = msg[3];
    }
    /* return status */
    return msg[1];
}

/* -------------------------------------------------------------- */

/*
    Find driver object for interface type
*/
static link_driver *find_driver(int type)
{
    link_driver **pdrv;

    for (pdrv = drivers; *pdrv; pdrv++) {
        if ((*pdrv)->type < 0) {
            break;
        }
        if ((*pdrv)->type == type) {
            return *pdrv;
            break;
        }
    }

    return NULL;
}

/*
    Get interface name
*/
const char *upgrade_if_name(int type)
{
    link_driver *drv;
    int x;
    
    x = -1;

    drv = find_driver(type);
    if (drv) {
	return drv->name;
    }
    return NULL;
}

/*
    Open IPMI device
*/
int upgrade_open(int type, char *device_name, char *param)
{
    int x;
    
    x = -1;

    current_driver = find_driver(type);
    
    if (!current_driver) {
        printf("upgrade_open: interface type %d is not supported\n",
		type);
        return -1;
    }
    
    if (current_driver->open) {
        return current_driver->open(type, device_name, param);
    }
    return 0;
}

/*
    Close IPMI device
*/
int upgrade_close(int nFd)
{
    if (current_driver) {
        if (current_driver->close) {
            return current_driver->close(nFd);
        }
    }
    return -1;
}
