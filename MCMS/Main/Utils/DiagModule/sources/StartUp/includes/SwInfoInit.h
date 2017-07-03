#ifndef _PARSER_H_
#define _PARSER_H_

//#include "DownLoad.h"

#define MAX_FILES_PER_APPL		1
#define MAX_FILENAME_LEN		120
#define MAX_ITEM_NAME_LEN		50

#define BOARD_TYPE_STR 		"BOARD_TYPE="
#define MCU_VER_ITEM_NAME 	"MCU_VERSION="
#define FILE_0 0
#define FILE_1 1
#define FILE_2 2
#define FILE_3 3



//#define ROOT_CRAMFS_TEMP_FILE         "tmproot.cramfs"
#define SWMAP_TEMP_FILE               "tmpswmap"
#define RTC_DRIVER_TEMP_FILE          "tmpds-133x"
#define USB_HAL_MFA_DRIVER_TEMP_FILE  "tmphal_mfa"
#define USB_PHCI_HCD_DRIVER_TEMP_FILE "tmpphci-hcd"
#define USB_FILE_NAME                 "lan.cfg"
#define DRIVERLOAD_TEMP_FILE          "tmpdriverload"
#define SWITCH_IPMC_TEMP_FILE         "tmpupgrade"
#define EMA_PACKET_TEMP_FILE          "tmpemapacket"
#define IPMI_SHELF_TEMP_FILE          "tmpipmishelf"






extern int errno;
extern UINT32 parse_appl_name(char *pBuffer);
extern int GetSwFilesToFlash();
#endif //_PARSER_H_
