/*
    Copyright (c) 2003-2005 Pigeon Point Systems.
    All rights reserved.

    Description:
	This program upgrades the BMR firmware on a
	remote system using the Firmware Upgrade
	Protocol.
            
    $RCSfile: upgradefw.c,v $
    $Revision: 1.6.10.4 $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _WIN32
#include <unistd.h>
#endif

//#include <sipldefs.h>
//#include <ipmidefs.h>
#include <upgdefs.h>

#include <upgrade_img.h>
#include <upgrade_io.h>

/* upgrade interface type */
static int ug_type = -1;

/* upgrade device/parameters */
static char *device_name = NULL;
static char *device_param = NULL;

/* upgrade image */
static char *fw_file_name = NULL;

/* upgrade block size */
static int blk_size = 20;

/* quiet mode */
static int quiet = 0;

/* verbosity level */
static int verbosity = 0;

/* upgrade buffer */
static unsigned char *buf;

/* restore backup flags */
static int restore_backup = 0;
static int get_backup_version = 0;

/* upgrade image headers */
static ug_file_head fhead;
static ug_img_head head;

/* -------------------------------------------------------------- */

typedef struct _strtab_entry {
    int code;
    char *str;
} strtab_entry;

static strtab_entry ipmi_errors[] =
{
    { 0, "OK" },
    { -1, "Unknown error" },
    { ERR_TIMEOUT, "Target does not respond" },
    { ERR_INVALID_RESPONSE, "Communication is lost (invalid response)" },
    { ERR_SYNTAX, "Communication is lost (bad syntax)" },
    { IPMI_INVALID_COMMAND, "Upgrade is not supported" },
    { IPMI_DEVICE_IN_FIRMWARE_UPDATE_MODE, "Device is in update mode" },
    { IPMI_NODE_BUSY, "Device is busy" },
    { -1, NULL }
};

static char str_err_hex[16];

static char *strtab_lookup(strtab_entry *tab, int code)
{
    if (!tab) {
        return NULL;
    }

    while (tab->str) {
        if (code == tab->code) {
            return tab->str;
        }
        tab++;
    }
    return NULL;
}

static char *ipmi_error_str(int code)
{
    char *p;
    
    if (code & ERR_SYNTAX) {
        code = ERR_SYNTAX;
    }
    p = strtab_lookup(ipmi_errors, code);
    if (p) {
        return p;
    }
    sprintf(str_err_hex, "error %02X", code);
    return str_err_hex;
}

/* -------------------------------------------------------------- */

/*
    Open the Firmware Upgrade session
*/
#define SESSION_OPEN_RETRY	5
int open_session(int nFd)
{
    int err, t;

    if (verbosity) {
	printf("Opening upgrade session ... ");
	fflush(stdout);
    }

    /* Open session - Firmware Upgrade Start */
    for (t = 0; t < SESSION_OPEN_RETRY; t++) {
        err = upgrade_start(nFd);
        if (!err || err == IPMI_DEVICE_IN_FIRMWARE_UPDATE_MODE ||
		err == IPMI_INVALID_COMMAND) {
            break;
        }
        SLEEP(2000);
    }
        
    if (err) {
	if (verbosity) {
	    printf("failed\n");
	}
	printf("Error: ");
    }
    if (err == IPMI_INVALID_COMMAND) {
	printf("Device does not support firmware update\n");
	return err;
    }
    if (err == ERR_INVALID_RESPONSE) {
        printf("Invalid response\n");
        return err;
    }
    if (err == IPMI_DEVICE_IN_FIRMWARE_UPDATE_MODE) {
        printf("Upgrade session is already opened\n");
        return err;
    }
    if (err) {
        printf("%s\n", ipmi_error_str(err));
    } else {
	if (verbosity) {
	    printf("OK\n");
	}
    }

    return err;
}

/*
    Close the Firmware Upgrade session
*/
int close_session(int nFd)
{
    int err;

    /* Firmware Upgrade Complete */
    if (verbosity) {
	printf("Closing upgrade session ... ");
	fflush(stdout);
    }

    err = upgrade_complete(nFd);
    if (verbosity) {
	if (err) {
    	    printf("failed\n");
	} else {
    	    printf("OK\n");
	}
    }

    return err;
}

/*
    Print backup version
*/
int show_version(int nFd)
{
    unsigned char st[2];
    int err;

    if (verbosity) {
	printf("Reading backup image version ... ");
	fflush(stdout);
    }

    /* Firmware Upgrade Complete */
    err = upgrade_get_backup_version(st,nFd);
    if (err) {
	if (verbosity) {
	    printf("failed\n");
	}
	printf("Error: ");
    }
    if (err == IPMI_REQUESTED_DATA_NOT_PRESENT) {
        printf("No valid backup copy\n");
        return err;
    } else if (err == IPMI_INVALID_COMMAND) {
        printf("Enhanced mode is not supported\n");
        return err;
    } else if (err) {
        printf("%s\n", ipmi_error_str(err));
        return err;
    } else {
	if (verbosity) {
    	    printf("OK\n");
	}
    }

    if (st[1] & 0x0F) {
	printf("Backup version: %d.%d.%d\n",
		st[0], st[1] >> 4, st[1] & 0x0F);
    } else {
	printf("Backup version: %d.%d\n",
		st[0], st[1] >> 4);
    }

    return 0;
}
/* -------------------------------------------------------------- */

/*
    Wait for backup/restore operaation
*/
#define BACKUP_TIMEOUT 60
int wait_backup(int nFd)
{
    int t;
    int err;
    unsigned char st[2];

    t = BACKUP_TIMEOUT;
    while (--t > 0) {
        SLEEP(1000);

        err = upgrade_status(st,nFd);
        if (err != 0) {
            return err;
        } else {
            if (st[0] == UPGRADE_STATUS_BUSY) {
                /* still busy */
                printf(".");
		fflush(stdout);
                continue;
            } else {
                break;
            }
        }
    }
	
    if (t <= 0) {
        return ERR_TIMEOUT;
    }
    return err;
}

/*
    Restore firmware from backup copy
*/
int do_restore(int nFd)
{
    int err;

    err = 0;
    
    printf("Restoring firmware from backup ...");
    fflush(stdout);

    err = upgrade_restore(nFd);
    if (err) {
	printf(" failed\n");
	printf("Error: ");
	if (err == ERR_TIMEOUT) {
    	    printf("Timeout\n");
	} else if (err == IPMI_INVALID_COMMAND) {
    	    printf("Enhanced mode is not supported\n");
	} else {
    	    printf("%s\n", ipmi_error_str(err));
	}
        return err;
    }
	
    err = wait_backup(nFd);
    if (err) {
	printf(" failed\n");
	printf("Error: ");
	if (err == ERR_TIMEOUT) {
    	    printf("Timeout\n");
	} else {
    	    printf("%s\n", ipmi_error_str(err));
	}
	return err;
    }

    printf(" OK\n");

    return err;
}

/*
    write firmware
*/
int write_image(unsigned char *data, int target, unsigned long addr,
	int size, int flag , int nFd)
{
    int i, percent, percent0;
    int err;
    unsigned long addr_start, addr_end;

    addr_start = addr;
    addr_end = addr_start + size;

    err = 0;
    
    /* Firmware Upgrade Prepare */
    if (flag & 1) {
        printf("Preparing target %i for programming ...", target);
	fflush(stdout);

    err = upgrade_prepare(target,nFd);
	if (err) {
	    printf(" failed\n");
	    printf("Error: ");
	    if (err == ERR_TIMEOUT) {
    		printf("Timeout\n");
    	    } else {
        	printf("%s\n", ipmi_error_str(err));
	    }
            return err;
        }

	err = wait_backup(nFd);
	if (err) {
	    printf(" failed\n");
	    printf("Error: ");
	    if (err == ERR_TIMEOUT) {
    		printf("Timeout\n");
	    } else {
    		printf("%s\n", ipmi_error_str(err));
	    }
	    return err;
	}

	printf(" OK\n");
    }

    i = 0; percent0 = -1;
    while (1) {
        if (size == 0) {
            percent = 0;
        } else if (i < size) {
            percent = (i * 100) / size;
        } else {
	    percent = 100;
	}
        
        if (percent != percent0) {
            printf("\rProgramming %d bytes to target %i at %06lX ... %3d%% ",
                    size, target, addr_start, percent);
	    fflush(stdout);
            percent0 = percent;
        }
	if (addr >= addr_end) {
	    break;
	}
        err = upgrade_write(target, addr, blk_size, data + i , nFd);
        if (err) {
            break;
        }
        addr += blk_size;
        i += blk_size;
    }

    printf("\n");

    if (err) {
	printf("Error: ");
	if (err == ERR_TIMEOUT) {
    	    printf("Connection lost\n");
	} else if (err == IPMI_DEVICE_IN_FIRMWARE_UPDATE_MODE) {
            printf("Session aborted\n");
	} else {
    	    printf("%s\n", ipmi_error_str(err));
	}
    }

    return err;
}

/* -------------------------------------------------------------- */

static char *usage = "Usage: upgradefw <options> upgrade_image\n"
        "Options:\n"
        "  -s device[:options]             upgrade over a serial line using\n"
        "                                  the SIPL-TM (Terminal Mode) protocol\n"
        "  -b device[:options]             upgrade over a serial line using\n"
        "                                  the SIPL-BM (Basic Mode) protocol\n"
        "  -n hostname[:port],targetaddr   upgrade over LAN/IPMB\n"
#ifdef _UPGRADE_IPMB
        "  -I device:targetaddr            upgrade over IPMB\n"
#endif
        "  -r                              restore from the backup copy\n"
        "  -R                              get backup version\n"
        "  -q                              quiet mode";

static int optindex = 0;

int LoadIpmcSoftware(int argc, char **argv)
{
    FILE *f;
    unsigned long addr, size;
    int c;
    int err = 0 , nFd = 0;
    char *p;

    puts("BMR firmware upgrade utility. Pigeon Point Systems (c) 2004-2005.");

    while (1) {
        if (++optindex >= argc) {
            break;
        }
        p = argv[optindex];
        if (p[0] == '-') {
            c = p[1];
        } else {
	    if (fw_file_name != NULL) {
		ug_type = -1;
		break;
	    }
            c = 1;
        }
        
        switch (c) {
        case 'q':
            quiet = 1;
            break;
            
        case 'v':
            verbosity = 1;
            break;
            
        case 'd':
            upgrade_debug = 1;
            break;

        case 'r':
            restore_backup = 1;
            break;
            
        case 'R':
            get_backup_version = 1;
            break;
            
        case 'I':
            p = argv[++optindex];
            device_name = p;
    	    p = strchr(device_name, ':');
    	    if (p) {
    		*p = 0;
    		device_param = p + 1;
    	    }
            ug_type = 1;
            break;
            
        case 'n':
            p = argv[++optindex];
            device_param = p;
    	    device_name = NULL;
            ug_type = 2;
            break;
            

	case 'b':
        case 's':
            p = argv[++optindex];
            device_name = p;
    	    p = strchr(device_name, ':');
    	    if (p) {
    		*p = 0;
    		device_param = p + 1;
    	    }
            ug_type = ('b' == c) ? 3 : 0;
            break;

        case 1:
            fw_file_name = p;
            break;
        }
    }

    if (ug_type < 0) {
        puts(usage);
        return 0;
    }

    if (!restore_backup && !get_backup_version && fw_file_name == NULL) {
        puts(usage);
        return 0;
    }
        
    setbuf(stdout, NULL);

    if (!quiet) {
        p = (char*)upgrade_if_name(ug_type);
        printf("Upgrade interface: %s", p);
	if (device_name) {
	    printf(", device: %s", device_name);
	}
        if (device_param) {
            printf(", options: %s", device_param);
        }
	printf("\n");
    }
    
    nFd = upgrade_open(ug_type, device_name, device_param);
    if (nFd == 0) {
        printf("Cannot open upgrade interface: error %d\n", err);
        return nFd;
    }

    err = open_session(nFd);

    if (err) {
        upgrade_close(nFd);
        return err;
    }

    if (get_backup_version) {
        show_version(nFd);
    }

    if (restore_backup) {
        err = do_restore(nFd);
	close_session(nFd);
        upgrade_close(nFd);
        return 0;
    }

    if (fw_file_name == NULL) {
	close_session(nFd);
        upgrade_close(nFd);
	return 0;
    }
    
    if (!quiet) {
        printf("Firmware upgrade image: %s\n", fw_file_name);
    }

    f = fopen(fw_file_name, "rb");
    if (!f) {
        perror(fw_file_name);
        return 1;
    }

    c = fread(&fhead, sizeof(ug_file_head), 1, f);
    if (fhead.mfg[0] != ((IANA_MANUFACTURER_PPS >> 16) & 0xFF) ||
	    fhead.mfg[1] != ((IANA_MANUFACTURER_PPS >> 8) & 0xFF) ||
	    fhead.mfg[2] != ((IANA_MANUFACTURER_PPS) & 0xFF)) {
        printf("Invalid image: %s\n", fw_file_name);
	close_session(nFd);
        upgrade_close(nFd);
        return 1;
    }
    if (fhead.version != 0x10) {
        printf("Incorrect image format version (%02X)\n",
		fhead.version);
	close_session(nFd);
        upgrade_close(nFd);
        return 1;
    }

    err = 0;
    while (err == 0) {
        c = fread(&head, sizeof(ug_img_head), 1, f);
        if (c < 1) {
            break;
        }
        addr = get3(head.offset);
        size = get3(head.size);
        buf = (unsigned char*)malloc(size);
        if (!buf) {
            puts("Not enough memory");
            return 1;
        }
        fread(buf, 1, size, f);
        
        err = write_image(buf, head.target, addr, size, head.flags , nFd);

        if (buf) {
            free(buf);
            buf = NULL;
        }

    	if (err) {
    	    break;
        }
    }

    if (f) {
        fclose(f);
        f = NULL;
    }

    close_session(nFd);
    upgrade_close(nFd);

    return err;
}
