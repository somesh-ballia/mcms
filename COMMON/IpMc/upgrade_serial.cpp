/*
    Copyright (c) 2003-2005 Pigeon Point Systems.
    All rights reserved.

    Description:
        This module contains source code for the
	Terminal Mode (serial) link-layer driver.
    
    $RCSfile: upgrade_serial.c,v $
    $Revision: 1.2.10.3 $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "DataTypes.h"

//#include <ipmidefs.h>
#include <upgdefs.h>

#include <upgrade_io.h>
#include <serial.h>

#if !defined(alloca)
#define alloca  _alloca
#endif

/* Sequence number */
static int seq = 0;

/* Timeout */
static int timeout_ms = 1000;

/* Retry limit */
#define POLL_LIMIT	64
#define RETRY_LIMIT	3
#define LINES_LIMIT     8

/* Basic mode additional parameters */
#define SIPL_BM_RSADDR  0x61

extern int recv_hex_response(unsigned char *data, int *length , int nFd);
extern INT32 open_ipmc_interface() ;
extern INT32 open_ipmc_interface_wrap() ;
extern INT32 write_ipmc_interface(char *str , int nFd) ;
extern void close_ipmc_interface(int nFd) ;
/*
    Open serial device
*/
int upgrade_serial_open(int type, char *name, char *param)
{
    int rc;
    
    rc = open_ipmc_interface_wrap() ;
    
    return rc;
    /*
    if(rc == 0)
    	return 1 ;
    return 0 ;
    */
}

/*
    Close serial device
*/
int upgrade_serial_close(int nFd)
{
    close_ipmc_interface(nFd);
    return 1;
}

/*
    Send a command to device
    data: <command code> [<data> ...] <chksum>
    Returns: error code, 0 = success
*/
int upgrade_serial_request_tm(int netfn_rslun, unsigned char *data, int *length , int nFd)
{
    char hex_rq[MAX_MSG * 2 + 16];
    unsigned char msg[MAX_MSG];
    int x, n, i, j, t, tt, cmd, err;
    char *p;

    /* check message length */
    if (*length > MAX_MSG - 2 || *length == 0) {
	return IPMI_INVALID_COMMAND;
    }

    /* update sequence number */
    seq = (seq + 1) & 0x3F;
    
    /* compose a message */
    j = 0;
    msg[j++] = netfn_rslun;
    msg[j++] = seq << 2;
    cmd = data[0];
    for (i = 0; i < *length; i++) {
        msg[j++] = data[i];
    }

    /* convert it to the Terminal Mode format */
    n = j;
    strcpy(hex_rq, "\r\r[");
    for (i = 0; i < n; i++) {
        x = msg[i];
        p = hex_rq + strlen(hex_rq);
        sprintf(p, "%02X", x);
    }
    strcat(hex_rq, "]\r");


    /* Send the message and receive the answer */
    t = 0;
    while (t < RETRY_LIMIT) {
        /* send request */
        if (upgrade_debug) {
            printf("< %s\n", hex_rq);
        }
        serial_flush(nFd);
        write_ipmc_interface(hex_rq,nFd);

        /* Wait for response */
	for (tt = 0; tt < RETRY_LIMIT; tt++) {
            /* receive hex response */
            n = MAX_MSG;
    	    err = recv_hex_response(msg, &n,nFd);
            if (upgrade_debug) {
                fprintf(stderr, "recv_hex_response err: %04X length: %d\n", err, n);
            }

            if ((err & ERR_SYNTAX) || err == ERR_INVALID_RESPONSE) {
        	write_ipmc_interface("\r\r",nFd);
        	SLEEP(1000);
                serial_flush(nFd);
    	    }

            if (err) {
                break;
            }

            /* check if the response matches the request */
    	    if (msg[0] != (netfn_rslun | 4)) {
                continue;
            }
	    if (msg[1] != (seq << 2)) {
                continue;
            }
	    if (msg[2] != cmd) {
                continue;
            }
	    /* valid response */
            if (upgrade_debug) {
               printf("Status: %02X, n = %d\n", msg[3], n);
            }
            break;
	}
        if (!err) {
            /* We've got a valid response */
            break;
        }
        if (err == IPMI_INVALID_COMMAND) {
            /* Command is not supported - we needn't repeat it */
            break;
        }
        if (err == IPMI_NODE_BUSY) {
            SLEEP(1000);
        } else {
            t++;
        }
    }
    if (err) {
        return err;
    }
    
    /* Copy response data */
    j = 0;
    for (i = 2; i < n; i++) {
        data[j++] = msg[i];
    }
    *length = j;

    return 0;
}

/*
    Send message in basic format.
*/
static int write_sipl_bm(unsigned char *buf, int cb , int nFd)
{
    unsigned char *tmp;
    int tmplen;
    int i;

    if (upgrade_debug) {
	printf( ">>> [");
	for (i = 0; i < cb; i++) {
	    printf( "%02X ", buf[i]);
	}
	printf("]\r\n");
    }

    tmp = (unsigned char *) alloca(cb*2+2);
    if (NULL == tmp) {
	return -1;
    }
    tmplen = 0;

    tmp[tmplen++] = 0xA0;
    for (i = 0; i < cb; i++) {
	if ((0xA0 == buf[i]) || (0xA5 == buf[i]) || (0xAA == buf[i]) ||
		(0xA6 == buf[i]) || (0x1B == buf[i])) {
	    tmp[tmplen++] = 0xAA;
	    if (0x1B == buf[i]) {
		tmp[tmplen++] = 0x3B;
	    } else {
		/* A0 A5 AA A6 -> [AA] B0 B5 BA B6 */
		tmp[tmplen++] = buf[i] | 0x10;
	    }
	} else {
	    tmp[tmplen++] = buf[i];
	}
    }
    tmp[tmplen++] = 0xA5;
    cb = serial_write((char*)tmp , tmplen , nFd);

    return cb;
}

/*
    Receive the message in basic format.
*/
static int read_sipl_bm(unsigned char *buf, int cb , int nFd)
{
    unsigned char ch;
    int ret, len, i, err;

    /* Wait for start byte */
    for (ch = 0; 0xA0 != ch; ) {
	ret = serial_read((char *)&ch, 1,nFd);
	if (1 != ret) {
	    return (0 == ret) ? 0 : -1;
	}
    }

    /* recv the message */
    for (len = 0, err = 0; len < IPMI_MAX_MESSAGE_SIZE; ) {
	ret = serial_read((char *)&ch, 1,nFd);
	if (1 != ret) {
	    return (0 == ret) ? 0 : (- len - 2);
	}
	if (0xA0 == ch) {
	    /* double start */
	    if (0 != len) {
		err = 1;
	    }
	} else if (0xA5 == ch) {
	    /* end-of-message marker found */
	    break;
	} else if (0xA6 == ch) {
	    /* handshake char, skipping */
	    continue;
	} else if (0xAA == ch) {
	    ret = serial_read((char *)&ch, 1,nFd);
	    if (1 != ret) {
		return (0 == ret) ? 0 : (- len - 2);
	    }
	    if ((0xB0 == ch) || (0xB5 == ch) || (0xBA == ch) ||
		    (0xB6 == ch)) {
		buf[len] = 0xEF & ch;
		len++;
	    } else if (0x3B == ch) {
		buf[len] = 0x1B;
		len++;
	    } else {
		err = 2;
	    }
	} else {
	    buf[len] = ch;
	    len++;
	}
    }

    if (upgrade_debug) {
	printf( "<<< [");
	for (i = 0; i < len; i++) {
	    printf("%02X ", buf[i]);
	}
	printf( "]\r\n");
    }

    if ((0 != err) || (len == IPMI_MAX_MESSAGE_SIZE)) {
	return -err;
    }

    return len;
}

/*
    Send a command to device
    data: <command code> [<data> ...] <chksum>
    Returns: error code, 0 = success
*/
int upgrade_serial_request_bm(int netfn_rslun, unsigned char *data, 
	int *length , int nFd)
{
    unsigned char req[MAX_MSG];
    int req_len;
    unsigned char resp[MAX_MSG];
    int resp_len;
    unsigned char cks, i, req_tries, resp_tries;
    
    /* check message length */
    if (*length > MAX_MSG - 6 || *length == 0) {
	return IPMI_INVALID_COMMAND;
    }

    /* update sequence number */
    seq = (seq + 1) & 0x3F;

    /* Prepare outgoing packet send */
    cks = (req[0] = SIPL_BM_RSADDR);
    cks += (req[1] = netfn_rslun);
    req[2] = -cks;

    cks = (req[3] = 0x20);  /* rqAddr */
    cks += (req[4] = seq << 2); /* rqSeq/rqLUN */

    for (i = 0; i < *length; i++) {
	cks += (req[5+i] = data[i]);
    }
    req[5+i] = -cks;    
    req_len = 5+i+1;

    /* Send the message and receive the answer */
    for (req_tries = 0; req_tries < RETRY_LIMIT; req_tries++) {
        
	/* send request */
        serial_flush(nFd);
	if (0 >= write_sipl_bm(req, req_len,nFd)) {
	    if (upgrade_debug) {
                printf( "Retry...\r\n");
	    }
	    continue;
	}

        /* Wait for response */
	for (resp_tries = 0; resp_tries < RETRY_LIMIT; resp_tries++) {

            /* receive hex response */
	    resp_len = read_sipl_bm(resp, sizeof(resp),nFd);
	    if (0 > resp_len) {
		if (upgrade_debug) {
		    printf("Cannot read\r\n");
		}
		return -1;
	    }
	    if (0 == resp_len) {
		SLEEP(1000);
		continue;
	    }
            /* check if the response matches the request */
    	    if (resp[1] != (netfn_rslun | 4)) {
                continue;
            }
	    if (resp[4] != (seq << 2)) {
                continue;
            }
	    if (resp[5] != req[5]) {
                continue;
            }
	    if (resp[3] != SIPL_BM_RSADDR) {
		continue;
	    }
	    /* verify the checksum */
	    for (i = 0, cks = 0; i < resp_len; i++) {
		cks += resp[i];
		if ((2 == i) && (0 != cks)) {
		    break;
		}
	    }
	    if (0 != cks) {
		if (upgrade_debug) {
		    printf("Checksum failed at %i, checksum %02X\n", 
			    i, cks);
		}
		continue;
	    }
	    /* valid response */
            if (upgrade_debug) {
                printf("Status: %02X\n", resp[6]);
            }
            break;
	}
        if (resp_tries < RETRY_LIMIT) {
            /* We've got a valid response */
            break;
        }
    }
    
    if ((RETRY_LIMIT == req_tries) || (RETRY_LIMIT == resp_tries)) {
	return 0;
    }
    
    /* Copy response data */
    *length = resp_len - 6;
    memcpy(data, resp + 5, *length);

    return 0;
}

/* link-level driver object */
link_driver link_serial_tm = {
    0,
    "serial (terminal mode)",
    upgrade_serial_open,
    upgrade_serial_close,
    upgrade_serial_request_tm
};

/* link-level driver object */
link_driver link_serial_bm = {
    3,
    "serial (basic mode)",
    upgrade_serial_open,
    upgrade_serial_close,
    upgrade_serial_request_bm
};
