/*
    Copyright (c) 2003-2005 Pigeon Point Systems.
    All rights reserved.

    Description:
	Firmware Upgrade Protocol definitions
    
    $RCSfile: upgdefs.h,v $
    $Revision: 1.3.10.2 $
*/

#ifndef __UPGDEFS_H__
#define __UPGDEFS_H__

/* Firmware Upgrade Protocol command codes */
#define UPGRADE_STATUS		0x00   /* Firmware Upgrade Status */
#define UPGRADE_START		0x01   /* Firmware Upgrade Start */
#define UPGRADE_PREPARE		0x02   /* Firmware Upgrade Prepare */
#define UPGRADE_WRITE		0x03   /* Firmware Upgrade Write */
#define UPGRADE_COMPLETE	0x04   /* Firmware Upgrade Complete */
#define UPGRADE_RESTORE		0x05   /* Firmware Upgrade Restore Backup */
#define UPGRADE_BACKUP_VERSION	0x06   /* Firmware Upgrade Get Backup Version */

/* Supported targets (not used on BMR-H8S) */
#define UPGRADE_TARGET_MASTER		0x00	/* Master AVR Flash */
#define UPGRADE_TARGET_SLAVE		0x01	/* Slave AVR Flash */

/* Upgrade status codes */
#define UPGRADE_STATUS_NOUPGRADE	0x00	/* Non-upgrade mode */
#define UPGRADE_STATUS_NOSESSION	0x01	/* Upgrade mode, no session */
#define UPGRADE_STATUS_IDLE		0x02	/* Session is open, idle */
#define UPGRADE_STATUS_BUSY		0x03	/* Session is open, busy */

/* Upgrade cause codes */
#define UPGRADE_CAUSE_USERRQ		0x00	/* User request */
#define UPGRADE_CAUSE_COMMAND		0x01	/* Upgrade Start command */
#define UPGRADE_CAUSE_CHECKSUM		0x02	/* Invalid checksum */
#define UPGRADE_CAUSE_NEWFWFAIL		0x03	/* New firmware failure */
#define UPGRADE_CAUSE_SLAVEFAIL		0x04	/* Slave failure */

/* from ipmidefs.h */
/* Maximum IPMI message size */
#define IPMI_MAX_MESSAGE_SIZE			32
#define IPMI_MAX_RESPONSE_SIZE			(IPMI_MAX_MESSAGE_SIZE - 7)

/* Network function codes */
#define IPMI_NETFN_CHASSIS                              0x00
#define IPMI_NETFN_BRIDGE                               0x02
#define IPMI_NETFN_SENSOR_EVENT                         0x04
#define IPMI_NETFN_APP                                  0x06
#define IPMI_NETFN_FIRMWARE                             0x08
#define IPMI_NETFN_STORAGE                              0x0A
#define IPMI_NETFN_TRANSPORT                            0x0C
#define IPMI_NETFN_GROUP_EXT                            0x2C
#define IPMI_NETFN_OEM_GROUP                            0x2E
#define IPMI_NETFN_CS_OEM_GROUP                         0x30

#define MAKE_NETFN_LUN( netFn, resp, lun )  (((netFn) | (resp)) << 2 | (lun))

/*  Command completion codes */
#define IPMI_SUCCESS                                    0x00
#define IPMI_NODE_BUSY                                  0xC0
#define IPMI_INVALID_COMMAND                            0xC1
#define IPMI_COMMAND_INVALID_FOR_LUN                    0xC2
#define IPMI_TIMEOUT                                    0xC3
#define IPMI_OUT_OF_SPACE                               0xC4
#define IPMI_RESERVATION_INVALID_OR_CANCELED            0xC5
#define IPMI_REQUEST_DATA_TRUNCATED                     0xC6
#define IPMI_REQUEST_DATA_LENGTH_INVALID                0xC7
#define IPMI_REQUEST_DATA_LENGTH_LIMIT_EXCEEDED         0xC8
#define IPMI_PARAMETER_OUT_OF_RANGE                     0xC9
#define IPMI_CANNOT_RETURN_NUMBER_OF_REQUESTED_BYTES    0xCA
#define IPMI_REQUESTED_DATA_NOT_PRESENT                 0xCB
#define IPMI_INVALID_DATA_IN_REQUEST                    0xCC
#define IPMI_COMMAND_ILLEGAL_FOR_SPECIFIED_OBJECT       0xCD
#define IPMI_CANNOT_PROVIDE_COMMAND_RESPONSE            0xCE
#define IPMI_CANNOT_EXECUTE_DUPLICATED_REQUEST          0xCF
#define IPMI_SDR_REPOSITORY_IN_UPDATE_MODE              0xD0
#define IPMI_DEVICE_IN_FIRMWARE_UPDATE_MODE             0xD1
#define IPMI_INITIALIZATION_IN_PROGRESS                 0xD2
#define IPMI_DESTINATION_UNAVAILABLE                    0xD3
#define IPMI_INSUFFICIENT_PRIVILEGE_LEVEL               0xD4
#define IPMI_COMMAND_NOT_SUPPORTED_IN_PRESENT_STATE     0xD5
#define IPMI_UNSPECIFIED_ERROR                          0xFF

/* from sipldefs.h */

#ifndef IANA_MANUFACTURER_PPS
#define IANA_MANUFACTURER_PPS				0x00400AL
#endif

#endif /* __UPGDEFS_H__ */
