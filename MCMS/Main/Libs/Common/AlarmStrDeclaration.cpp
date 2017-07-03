#include "AlarmStrDeclaration.h"
#include "AlarmStrTable.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "HlogElement.h"

/*----------------------------------------------------------------------------------
  Insert your alarm codes here.
  1) first parameter is an error code. it should be defined in FaultsDefines.h.
  2) second parameter is a help description. it is not a name of the error code,
     the name(a string you see in EMA) will be created automatically.

  Ask Danny Bibi (461) if you have any problem
----------------------------------------------------------------------------------*/

BEGIN_ALARM_TABLE()

	ADD_ALARM_TO_ALARM_STR_TABLE(TEST_ERROR, "Used for testing the Active Alarms mechanism")

	ADD_ALARM_TO_ALARM_STR_TABLE(IDLE_DEADLINE_REACHED, "Process idle")
	ADD_ALARM_TO_ALARM_STR_TABLE(STARTUP_CONDITION_CONFIGURATION, "Internal System configuration during startup")
	ADD_ALARM_TO_ALARM_STR_TABLE(DEBUG_MODE, "DEBUG mode enabled")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_CONNECTION_WITH_CARD, "Card not responding")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_UNIT_NOT_RESPONDING, "Unit not responding")
	ADD_ALARM_TO_ALARM_STR_TABLE(CS_COMPONENT_FATAL, "Central signaling component failure")
	ADD_ALARM_TO_ALARM_STR_TABLE(NO_CONNECTION_WITH_CS, "No response from Central Signaling")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_MEDIA_IP_CONFIGURATION_FAILURE, "Failed to configure the Media card IP address")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CARD_FOLDER_MOUNTING_FAILED, "Failed to mount Card folder")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CARD_STARTUP_FAILURE, "Could not complete MPM Card startup procedure")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_CONNECTION_WITH_RTM_ISDN,"No response from RTM ISDN card")

	ADD_ALARM_TO_ALARM_STR_TABLE(AA_RTM_ISDN_STARTUP_FAILURE,"Could not complete RTM ISDN Card startup procedure")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_RTM_ISDN_STARTUP_PROBLEM,"RTM ISDN card startup procedure error")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_GATE_KEEPER_ERROR, "Gatekeeper failure")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_IP_SERVICE_PARAMS, "No IP Network Services defined")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_LICENSING, "License not found")
	ADD_ALARM_TO_ALARM_STR_TABLE(TASK_TERMINATED, "Task terminated")
	ADD_ALARM_TO_ALARM_STR_TABLE(CFG_CHANGED, "System Configuration modified")
	ADD_ALARM_TO_ALARM_STR_TABLE(CFG_INVALID, "Invalid System Configuration")
	ADD_ALARM_TO_ALARM_STR_TABLE(LOW_PROCESS_MEMORY_ALERT, "Low Processing Memory")
	ADD_ALARM_TO_ALARM_STR_TABLE(LOW_SYSTEM_MEMORY_ALERT, "Low system Memory")
	ADD_ALARM_TO_ALARM_STR_TABLE(HIGH_CPU_USAGE_PROCESS_ALERT, "High CPU utilization")
	ADD_ALARM_TO_ALARM_STR_TABLE(SYSTEM_CPU_USAGE_ALERT, "High system CPU usage")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_ISDN_SERVICE_PARAMS,"No ISDN/PSTN Network Services defined")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_DEFAULT_ISDN_SERVICE,"No default ISDN/PSTN Network Service defined in ISDN/PSTN Network Services list")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_READ_MR_DB_REQ_RECIEVED_FROM_RSRC,"Resource process failed to request the Meeting Room list during startup.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_SIP_SECURED_COMMUNICATION_FAILED, "Secured SIP communication failed")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_EXTERNAL_ALERT_EMB, "Alarm generated by an internal component")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_VERSION_INSTALL_PROGRESS, "Software upgrade in component")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_HIGH_SCHEDULER_LATENCY, "OS Latency Threshold Exceeded")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CORE_FILE_DETECTED, "New core file was detected")

	ADD_ALARM_TO_ALARM_STR_TABLE(AA_VERSION_IPMC_INSTALL_PROGRESS, "IPMC software upgrade in component")

	//safe upgrade
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_SAFE_UPGRADE_OFF, "Warning: Upgrade started and SAFE Upgrade protection is turned OFF")

	ADD_ALARM_TO_ALARM_STR_TABLE(IP_VERSION_CHANGED, "IP Version has been changed")

	// CSMngr errors
	ADD_ALARM_TO_ALARM_STR_TABLE(NO_SERVICE_FOUND_IN_DB, "IP Network Service not found")
	ADD_ALARM_TO_ALARM_STR_TABLE(CS_NOT_CONFIGURED, "Missing Central Signaling configuration")
	ADD_ALARM_TO_ALARM_STR_TABLE(CS_IP_NOT_CONFIGURED, "Missing Central Signaling IP configuration")
	ADD_ALARM_TO_ALARM_STR_TABLE(CS_SERVICE_STATE_FAIL, "Central Signaling indicating Faulty status")
	ADD_ALARM_TO_ALARM_STR_TABLE(CS_STARTUP_FAILED, "Central Signaling startup failure")
	ADD_ALARM_TO_ALARM_STR_TABLE(DUPLICATE_IP_CS_MNGMNT, "IP addresses of Signaling Host and Control Unit are the same")
	ADD_ALARM_TO_ALARM_STR_TABLE(IP_SERVICE_DELETED, "IP Network Service deleted")
	ADD_ALARM_TO_ALARM_STR_TABLE(IP_SERVICE_CHANGED, "IP Network Service configuration modified")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_EXTERNAL_ALERT_CS, "Alarm generated by a Central Signaling component")
	ADD_ALARM_TO_ALARM_STR_TABLE(IP_SERVICE_ADDED, "IP Network Service added")
	ADD_ALARM_TO_ALARM_STR_TABLE(NETWORK_INTERFACE_IS_NOT_CONFIGURED, "Network interface is not configured. New interface need to be chosen.")
	ADD_ALARM_TO_ALARM_STR_TABLE(CS_IP_IS_ZERO, "Signaling IP address is invalid.")
    ADD_ALARM_TO_ALARM_STR_TABLE(LAST_CONFIGURED_NETWORK_INTERFACE_IS_INVALID, "MFW Startup with invalid configured interface, FallBack.")
    
	// McuMngr errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_PRODUCT_ACTIVATION_FAILURE, "Product activation failure")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_MANAGEMENT_IP_INTERFACE, "Management Network not configured")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_TIME_CONFIGURATION, "Error reading MCU time")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_DNS_CONFIGURATION, "DNS not configured in IP Network Service")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_DNS_REGISTRAION_FAILED, "Failed to register with DNS server")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_MPL_STARTUP_FAILURE_AUTHENTICATION_NOT_RECEIVED, "MPL startup failure. Authentication not received.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_MPL_STARTUP_FAILURE_MNGMNT_IP_CNFG_NOT_RECEIVED, "MPL startup failure. Management Network configuration not received.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_PATCHED_VERSION, "The software contains patch(es)")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_PRIVATE_VERSION, "A private version is loaded")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_FAILED_TO_OPEN_APACHE_CONFIGURATION_FILE, "Failed to open Apache server configuration file")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_FAILED_TO_SAVE_APACHE_CONFIGURATION_FILE, "Failed to save Apache server configuration file")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NTP_SYNC_FAILURE, "NTP synchronization failure")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_ILLEGAL_TIME, "Invalid date and time")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_EXTERNAL_NTP_SERVERS_FAILURE, "External NTP servers failure")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_RUNNING_VERSION_MISMATCHES_CURRENT_VERSION, "Fallback version is being used")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_IDE_NOT_IN_DMA_MODE, "DMA not supported by IDE device")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_BAD_ETHERNET_SETTINGS, "Incorrect Ethernet Settings")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_SMART_REPORT_ERRORS_ON_HD, "Smart Report found errors on hard disk")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_ILLEGAL_MCU_VERSION, "Invalid MCU Version")
	ADD_ALARM_TO_ALARM_STR_TABLE(DEBUG_CFG_EXIST, "DEBUG mode flags in use")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_INSTALLING_NEW_VERSION, "Version upgrade is in progress")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_SSH_ENABLED, "SSH is enabled")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_HTTPS_DISABLED_IN_JITC, "The system has been configured for ULTRA SECURE mode, but communication is not secured until a TLS certificate is installed and the MCU is set to Secured communication.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_EXTERNAL_DB_CERTIFICATE_ERROR, "Error in external database certificate")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_EVENT_MODE_RESOURCE_DEFICIENCY_DUE_TO_WRONG_LICENSE, "Event Mode Conferencing resources deficiency due to inappropriate license. Please install a new license.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_FIPS_140_TEST_RESULT_FROM_ENCRYPTIONKEYSERVER, "FIPS 140 test result not received")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_FIPS_140_FAILURE_WITHIN_ENCRYPTIONKEYSERVER, "FIPS 140 failure")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_LICENSE_EXPIRED, "Please install a newer version")
	ADD_ALARM_TO_ALARM_STR_TABLE(TEMPERATURE_CRITICAL_SHUTDOWN, "A temperature sensor reached and passed the upper critical. Shut down the system.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_SYSTEM_IP_YET, "MCU invalid IP")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_LICENSE_ACQUISITION_FAIL,"Valid license not found.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_LICENSE_WAS_MODIFIED,"License has been modified. Modified license will take effect after system restart.")
	// MplApi errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_PRODUCT_TYPE_MISMATCH, "Product Type mismatch. System is restarting.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_UNKNOWN_CPU_SLOT_ID, "CPU slot ID not identified")

	//SipProxy errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_SIP_REGISTRAR, "Failed to connect to SIP registrar")
	ADD_ALARM_TO_ALARM_STR_TABLE(SIP_REGISTRATION_LIMIT_REACHED, "SIP registrations limit reached")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_FAILED_TO_SUBSCRIBE_WITH_THE_OCS, "Failed to subscribe with the OCS, therefore the A/V Edge Server URI was not received")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_RECEIVED_NOTIFICATION_FAILED, "Received Notification failed")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_SERVICE_REQ_FAILED, "Service Request failed");
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_INITIALIZE_ICE_STACK_FAILURE, "Initialization of ice stack failed")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_REGISTER_ICE_USER_FAILED, "OCS Registration failed")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_RESOLVE_ICE_SERVER_NAME_FAILED, "failed to resolve ICE server Host-Name")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_REGISTER_FAILED_FOR_SOME_CONF, "Registration failed for some Conference")
	
	//DNSAgent errors
	ADD_ALARM_TO_ALARM_STR_TABLE(FAILED_TO_ACCESS_DNS_SERVER, "Failed to access DNS server")
	ADD_ALARM_TO_ALARM_STR_TABLE(NO_IND_DNS_SUCCESS, "DNS configuration error")

	//McmsDaemon errors
	ADD_ALARM_TO_ALARM_STR_TABLE(PROCESS_TERMINATED,"Process terminated")
	ADD_ALARM_TO_ALARM_STR_TABLE(MODULE_DOES_NOT_EXIST,"Card not found")
	ADD_ALARM_TO_ALARM_STR_TABLE(RESET_MCU_DIAGNOSTICS, "MCU Reset to enable Diagnostics mode")
	ADD_ALARM_TO_ALARM_STR_TABLE(RESET_MCU, "MCU reset")
	ADD_ALARM_TO_ALARM_STR_TABLE(RESET_MCU_BY_TERMINAL, "Terminal initiated MCU reset")
	ADD_ALARM_TO_ALARM_STR_TABLE(RESET_MCU_BY_OPERATOR, "User initiated MCU reset")
	ADD_ALARM_TO_ALARM_STR_TABLE(RESET_MCU_INTERNAL, "Internal MCU reset")
	ADD_ALARM_TO_ALARM_STR_TABLE(PROCESS_STARTUP_FAILED, "Startup process failure")
	ADD_ALARM_TO_ALARM_STR_TABLE(SYSTEM_IN_SAFE_MODE, "Automatic reset is unavailable in Safe Mode")
	ADD_ALARM_TO_ALARM_STR_TABLE(GUI_SYSTEM_CONFIG_IS_ILLEGAL, "GUI System configuration file is invalid xml file")

	ADD_ALARM_TO_ALARM_STR_TABLE(CS_RECOVERY_STATUS, "Central Signaling indicating Recovery status")

	// Switch KeepAlive components problems
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_VOLTAGE_PROBLEM, "Voltage problem")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_RTM_LAN_OR_ISDN_MISSING, "No RTM-LAN or RTM-ISDN installed. One of these cards must be installed in your MCU")
    ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_LAN_CONNECTION_PORT_1, "No LAN connection port 1")
    ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_LAN_CONNECTION_PORT_2, "No LAN connection port 2")
    ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_LAN_CONNECTION_PORT_3, "No LAN connection port 3")
    ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_LAN_CONNECTION_PORT_4, "No LAN connection port 4")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_TEMPERATURE_MAJOR_PROBLEM, "Temperature Level Major")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_TEMPERATURE_CRITICAL_PROBLEM, "Temperature Level Critical")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_FAILURE_PROBLEM, "Card failure")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_POWER_OFF_PROBLEM, "Power off")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_OTHER_PROBLEM, "Unspecified problem")

	//Resource errors
	ADD_ALARM_TO_ALARM_STR_TABLE(INSUFFICIENT_RESOURCES, "Insufficient resources")
	ADD_ALARM_TO_ALARM_STR_TABLE(NO_MEETING_ROOM, "Resource process did not receive the Meeting Room list during startup.")
	ADD_ALARM_TO_ALARM_STR_TABLE(INSUFFICIENT_UDP_PORTS, "Insufficient UDP Ports")
	ADD_ALARM_TO_ALARM_STR_TABLE(PORT_CONFIGURATION_CHANGED, "Port configuration was modified")
	ADD_ALARM_TO_ALARM_STR_TABLE(NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER, "No usable unit for audio controller");
	ADD_ALARM_TO_ALARM_STR_TABLE(ALLOCATION_MODE_CHANGED, "Allocation mode was modified")

	ADD_ALARM_TO_ALARM_STR_TABLE(PORT_GAUGE_THRESHOLD_REACHED,"System resources usage has exceeded Port Gauge threshold")
	ADD_ALARM_TO_ALARM_STR_TABLE(AUDIO_PORT_GAUGE_THRESHOLD_REACHED,"System resources of Audio ports usage has exceeded Port Gauge threshold")
	ADD_ALARM_TO_ALARM_STR_TABLE(VIDEO_PORT_GAUGE_THRESHOLD_REACHED,"System resources of Video ports usage has exceeded Port Gauge threshold")

	// Logger errors
	ADD_ALARM_TO_ALARM_STR_TABLE(FAILED_TO_INIT_FILE_SYSTEM, "Failed to initialize the file system")
	ADD_ALARM_TO_ALARM_STR_TABLE(BAD_FILE_SYSTEM, "The Log file system is disabled")
	ADD_ALARM_TO_ALARM_STR_TABLE(BAD_HARD_DISK, "Hard disk error")
	ADD_ALARM_TO_ALARM_STR_TABLE(BACKUP_OF_LOG_FILES_IS_REQUIRED, "Backup of log files is required")
	ADD_ALARM_TO_ALARM_STR_TABLE(LOG_IS_DISABLED_BY_SYSTEM_LOAD, "The Log file system is disabled because of high system CPU usage")

	ADD_ALARM_TO_ALARM_STR_TABLE(BACKUP_OF_AUDIT_FILES_IS_REQUIRED, "Backup of audit files is required")
	ADD_ALARM_TO_ALARM_STR_TABLE(BACKUP_OF_CDR_FILES_IS_REQUIRED, "Backup of CDR files is required")
	ADD_ALARM_TO_ALARM_STR_TABLE(FAIL_TO_REGISTER_TO_CDR_SERVER, "Fail to register to CDR Server")


	// Apache errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_FAILED_TO_FILL_ACTION_REDIRECTION, "Action redirection failure")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_HTTPD_FAILED_USE_REVOCATION_METHOD, "Management TLS (httpd):: failed to verify with revocation method")

	ADD_ALARM_TO_ALARM_STR_TABLE(FAILED_CONFIG_USER_LIST_IN_LINUX, "Failed to configure the Users list in Linux")
	ADD_ALARM_TO_ALARM_STR_TABLE(SYSTEM_CONFIGURATION_CHANGED, "System configuration changed. Please reset the MCU.")

	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_MUSIC_SOURCE, "Music file error")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_SWITCH_NOT_LOADED, "SWITCH not responding")

	//Authentication errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_USERS_LIST_CORRUPTED,"Failed to open Users list file")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_DEFAULT_USER_EXISTS,"Polycom default User exists. For security reasons, it is recommended to delete this User and create your own User.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_SUPPORT_OPERATOR_ILLEGAL_IN_FEDERAL_MODE,"User Name  SUPPORT cannot be used in Enhanced Security Mode")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_MACHINE_ACCOUNT_EXPIRATION_ALERT,"Password expiration warning")

	//GKManager errors
	ADD_ALARM_TO_ALARM_STR_TABLE(GKMNGR_AVF_WRONG_MCU_CONFIG, "MCU is not configured for AVF gatekeeper mode")

	//RtmIsdnMngr errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_ISDN_SERVICES_CONFIGURATION_CHANGED,"ISDN/PSTN Network Services configuration changed")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_ISDN_PSTN_LICENSING, "No License for ISDN/PSTN. Please activate the RTM ISDN card through Polycom website.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_RTM_ISDN_CARD, "RTM ISDN card not found")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_NO_CLOCK_SOURCE, "No clock source")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_SINGLE_CLOCK_SOURCE, "Single clock source")

	ADD_ALARM_TO_ALARM_STR_TABLE(FILE_SYSTEM_FAILED_TO_SCAN, "File system scan failure")
	ADD_ALARM_TO_ALARM_STR_TABLE(FILE_SYSTEM_OVERFLOW, "File system space shortage")
	ADD_ALARM_TO_ALARM_STR_TABLE(HARD_DISK_FAILURE, "Hard disk error")
	ADD_ALARM_TO_ALARM_STR_TABLE(NEW_VERSION_INSTALLED, "A new version was installed. Reset the system.")
	ADD_ALARM_TO_ALARM_STR_TABLE(ACTIVATION_KEY_IS_NEEDED, "A matching activation key is required. To cancel the upgrade process, reset the MCU.")
	ADD_ALARM_TO_ALARM_STR_TABLE(UPGRADE_RECEIVE_VERSION, "MCU is uploading the version file. To cancel the upload and the upgrade, reset the MCU.")
	ADD_ALARM_TO_ALARM_STR_TABLE(NEW_VERSION_DOWNGRADE_FAILED, "new version downgrade failed.")

	ADD_ALARM_TO_ALARM_STR_TABLE(SHA1_VERSION, "MCU user/password list will be reset")

	ADD_ALARM_TO_ALARM_STR_TABLE(FPGA_CHECK_FAILED, "FPGA failure. Please upload and upgrade the software.")
	ADD_ALARM_TO_ALARM_STR_TABLE(FPGA_STARTUP_BURNING, "FPGA failure. Please wait for FPGA burning to complete.")
	
	ADD_ALARM_TO_ALARM_STR_TABLE(NEW_ACTIVATION_KEY_LOADED, "A new activation key was loaded. Reset the system.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_RED_ALARM, "Red Alarm")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_YELLOW_ALARM,"Yellow Alarm" )
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_D_CHANNEL_NOT_ESTABLISHED, "D channel cannot be established")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CARDS_CONFIG_EVENT, "Card configuration event")

	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CARDS_WRONG_FILE_MODE, "Cards wrong file's mode")

	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CARDS_FLASH_ACCESS_PROBLEM, "Card failed to switch to Enhanced Security Mode")
	ADD_ALARM_TO_ALARM_STR_TABLE(CPU_IPCM_SOFTWARE_IS_NOT_UPDATED, "CPU IPMC software was not updated")

	ADD_ALARM_TO_ALARM_STR_TABLE(AA_BAD_FILE, "File error")

	//ConfParty errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_IVR_SERVICE_LIST_MISSING_DEFAULT_SERVICE, "No default IVR Service in IVR Services list")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CAN_NOT_CREATE_DEFAULT_PROFILE, "Failed to create Default Profile")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CONF_ENCRYPTION_ERROR, "Conference Encryption Error")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_COP_MODE_NOT_SUPPORTED_MPM_PLUS_MODE, "System Cards MPM Plus mode are not supported in Event mode")


	// Encryption server errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_ENCRYPTION_SERVER_ERROR, "Encryption Server Error. Failed to generate the encryption key.")

	ADD_ALARM_TO_ALARM_STR_TABLE(AA_FAILED_TO_CONNECT_RECORDING_LINK, "Failed to connect to recording device")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_RECORDING_LINK_DISCONNECED_UNEXPECTEDLY, "Recording device has disconnected unexpectedly")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_SYSTEM_BASED_MODE_NOT_INTIALIZED, "Failed to initialize system base mode")

	// QAAPI errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER, "Failed to connect to application server")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_SYSTEM_CONFIGURATION, "Configuration of external database did not complete")

	// BackupRestore errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_RESTORE_SUCCEEDED, "Restore Succeeded")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_RESTORE_FAILED, "Restore Failed")

	// Snmp errors
	ADD_ALARM_TO_ALARM_STR_TABLE(SNMP_AGENT_INIT_FAILED, "Failure in initialization of SNMP agent")

	// Failover errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_HOT_BACKUP_PAIR_MCU_IS_UNREACHABLE, "Hot Backup: Paired MCU is unreachable")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_HOT_BACKUP_MASTER_SLAVE_CONFIGURATION_CONFLICT, "Hot Backup: Master-Slave configuration conflict")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_HOT_BACKUP_NETWORK_ISSUE, "Hot backup: Network issue")

	/*----------------------------------------------------------------------------------
		Faults(Not alarms) error codes
	----------------------------------------------------------------------------------*/
	ADD_FAULT_TO_ALARM_STR_TABLE(EXTERNAL_FAULT, "External component failure")

	ADD_FAULT_TO_ALARM_STR_TABLE(MCU_RESTART, "System is starting")

	// assert
	ADD_FAULT_TO_ALARM_STR_TABLE(SOFTWARE_ASSERT_FAILURE, "Software assert failure")
	ADD_FAULT_TO_ALARM_STR_TABLE(CARD_MANAGER_RECONNECT, "Reconnecting with card")

	// files
	ADD_FAULT_TO_ALARM_STR_TABLE(FILE_NOT_EXIST, "File not found")

	//	reservations
	ADD_FAULT_TO_ALARM_STR_TABLE(RESERVED_CONFER_HAS_NO_SUFFICIENT_RESOURCES, "Insufficient resources to start the conference")
	ADD_FAULT_TO_ALARM_STR_TABLE(START_TIME_OF_RESERVED_CONFER_IS_OVER, "Start Time of reserved conference past due")

	// general
	ADD_FAULT_TO_ALARM_STR_TABLE(GATEKEEPER_MESSAGE, "Gatekeeper status or error indication")
	ADD_FAULT_TO_ALARM_STR_TABLE(BAD_SPONTANEOUS_INDICATION, "MFA error")
	ADD_FAULT_TO_ALARM_STR_TABLE(AA_FAN_MAJOR_PROBLEM, "Fan Problem Level Major")
	ADD_FAULT_TO_ALARM_STR_TABLE(AA_FAN_CRITICAL_PROBLEM, "Fan Problem Level Critical")
	ADD_FAULT_TO_ALARM_STR_TABLE(AA_PWR_MAJOR_PROBLEM, "Power Problem Level Major")
	ADD_FAULT_TO_ALARM_STR_TABLE(AA_PWR_CRITICAL_PROBLEM, "Power Problem Level Critical")
	ADD_FAULT_TO_ALARM_STR_TABLE(APPLICATION_SERVER_INTERNAL_ERROR, "Internal error in application server")
	ADD_FAULT_TO_ALARM_STR_TABLE(SOCKET_DISCONNECT, "Socket is disconnected")
	ADD_FAULT_TO_ALARM_STR_TABLE(SOCKET_RECONNECT, "Socket reconnected")

	ADD_FAULT_TO_ALARM_STR_TABLE(XML_PARSE_ERROR, "XML parsing error")

	ADD_FAULT_TO_ALARM_STR_TABLE(CANT_STORE_DATA, "Cannot save data")

	// apache errors
	ADD_FAULT_TO_ALARM_STR_TABLE(FAILED_TO_RECEIVE_SYNC_MESSAGE, "Failed to receive synchronization message between processes")
	ADD_FAULT_TO_ALARM_STR_TABLE(FAILED_TO_SEND_SYNC_MESSAGE, "Failed to send synchronization message between processes")
	ADD_FAULT_TO_ALARM_STR_TABLE(APACHE_MODULE_EXCEPTION, "Exception in Apache module")
	ADD_FAULT_TO_ALARM_STR_TABLE(EXTERNAL_FAULT_EMB, "Alarm generated by an internal component");

	// ConfParty errors
	ADD_FAULT_TO_ALARM_STR_TABLE(RESTORING_DEFAULT_FACTORY, "Restoring Factory Defaults. Default system settings will be restored once Reset is completed.")
	ADD_FAULT_TO_ALARM_STR_TABLE(ACK_FAILED, "ACK failure")
	ADD_FAULT_TO_ALARM_STR_TABLE(ACK_NOT_RECEIVED, "ACK not received")
	ADD_FAULT_TO_ALARM_STR_TABLE(ILLEGAL_CONFERENCE_PARAMETERS, "Invalid conference settings")
	ADD_FAULT_TO_ALARM_STR_TABLE(INVALID_OPCODE_RESPONSE, "Invalid opcode Response")
	ADD_FAULT_TO_ALARM_STR_TABLE(INSUFFICIENT_EVENT_MODE_CONFERENCE_RESOURCES, "Insufficient Resources to start an Event Mode conference")
	ADD_FAULT_TO_ALARM_STR_TABLE(CONFERENCE_END_DUE_TO_RESOURCE_DEFICIENCY, "Conference ended due to resource deficiency")
	ADD_FAULT_TO_ALARM_STR_TABLE(CALL_END_DUE_TO_ICE_CONNECTIVITY_CHECK_FAILURE, "Call ended due to ICE connectivity check failure")
	ADD_FAULT_TO_ALARM_STR_TABLE(CALL_END_DUE_TO_ICE_INSUFFICIENT_BANDWIDTH, "Call ended due to ICE insufficient bandwidth")

	ADD_FAULT_TO_ALARM_STR_TABLE(PROCESS_FAILED, "Process failed")
	ADD_FAULT_TO_ALARM_STR_TABLE(INTERNAL_ICPA_EVENT, "Power down signal was detected with icpa event")
	ADD_FAULT_TO_ALARM_STR_TABLE(TASK_FAILED, "Task failed")

	// SNMP errors
	ADD_FAULT_TO_ALARM_STR_TABLE(SNMPD_FAILURE, "SNMP failure")
	ADD_FAULT_TO_ALARM_STR_TABLE(EXTERNAL_NTP_SERVER_FAILURE, "External NTP server failure")

	// McmsDaemon
	ADD_FAULT_TO_ALARM_STR_TABLE(WILD_RESET_IND, "System did not shut down properly")

	//RtmIsdnMngr errors
	ADD_FAULT_TO_ALARM_STR_TABLE(ISDN_SPAN_ATTACHED_TO_NONEXISTING_SERVICE,"ISDN Span assigned to inaccessible ISDN/PSTN Network Service")
	ADD_FAULT_TO_ALARM_STR_TABLE(INCONSISTENT_SPAN_TYPE,"Inconsistent ISDN span type - all span types in the system must be the same")
	ADD_FAULT_TO_ALARM_STR_TABLE(NUM_OF_CONFIGURED_SPANS_EXCEEDED,"Maximum number of configured ISDN spans has been reached")

	// CDR errors
	ADD_FAULT_TO_ALARM_STR_TABLE(CDR_FILE_REACH_MAX_CAPACITY, "A CDR file has reached its maximum capacity")
	ADD_FAULT_TO_ALARM_STR_TABLE(AA_CDR_SERVER_CONNECTION_DOWN, "CDR server connection down")
	ADD_FAULT_TO_ALARM_STR_TABLE(AA_CDR_CHECK_CONNECTION_TO_CDR_SERVER, "Check connection to CDR server. CDR events buffer will soon be full.")
	ADD_FAULT_TO_ALARM_STR_TABLE(AA_CDR_INFORMATION_TO_CDR_SERVER_LOST, "CDR information to remote CDR Service is being lost")

	//CSMNGR
	ADD_FAULT_TO_ALARM_STR_TABLE(EXTERNAL_FAULT_CS, "Alarm generated by a Central Signaling component")

	// ExchangeModule errors
	ADD_FAULT_TO_ALARM_STR_TABLE(AA_EXCHANGE_SERVER_CONNECTION_ERROR, "Unable to connect to Exchange Server")

	// Failover errors
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_LDAP_FAILED_TO_CONNECT_TO_ACTIVE_DIRECTORY_SERVER, "MCU fails to connect to Active Directory server")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_LDAP_FAILED_TO_CONNECT_TO_OCSP_SERVER, "LDAP TLS: Failed to connect to OSCP responder")

	// CertMngr
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_FAILED_TO_VALIDATE_CERTIFICATE, "Security mode failed. Error in certificate file.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CERTIFICATE_EXPIRED, "Security mode failed. Certificate has expired.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CERTIFICATE_COMMON_NAME_ERROR, "Security mode failed. Certificate host name does not match the MCU host name.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CERTIFICATE_NOT_VALID_YET, "Security mode failed. Certificate not yet valid.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CERTIFICATE_GOING_TO_BE_EXPIRED, "Security mode failed. Certificate is about to expire.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CERTIFICATE_REPOSITORY_NOT_UPDATED, "Requested changes to the certification repository were not completed. Repository must be updated to implement these changes.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CERTIFICATE_FOR_CS_NEED_RESET_TO_TAKE_EFFECT, "New certificate for CS need MCU reset to take effect.")
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_CERTIFICATE_SELF_CERT_NOT_ALLOWED_FOR_MS_SERVICE, "TLS: Self Sign Certificate not allowed with microsoft sip service.")

	// Utility active alarms
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_UTILITY_NETWORK_TRAFFIC_CAPTURE_ON, "Network traffic capture is on")

	// MCCFMngr
	ADD_ALARM_TO_ALARM_STR_TABLE(AA_MCCF_CHANNEL_IS_NOT_CONNECTED, "The MCCF channel is not connected")

      //video recovery
      /*Begin:added by Richer for BRIDGE-12231,2014.3.12 */
      ADD_ALARM_TO_ALARM_STR_TABLE(AA_VIDEO_RECOVERY, "Media is recoverying")
      /*End:added by Richer for BRIDGE-12231,2014.3.12 */

END_ALARM_TABLE()
