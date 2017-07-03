
#ifndef __STRINGS_LEN__
#define __STRINGS_LEN__

#ifdef MAX_STRING_LEN
#undef MAX_STRING_LEN
#define MAX_STRING_LEN                 201 // 200 characters + terminator
#endif
#define FILE_NAME_LEN                  13  // 12 characters + terminator
#define IP_STRING_LEN                  512
#define DESCRIPTION_LEN					    50
#define ERROR_MESSAGE_LEN	                1024       
#define IP_ADDRESS_LEN			            16       
#define MAX_RANDOM_NUMERIC_STRING           17

#define PRIVATE_VERSION_DESC_LEN		41 	// 40 chars + terminator
#define NEW_FILE_NAME_LEN              	255 // 30 increased To 255 accommodate full paths
#define IVR_FILE_NAME_LEN              	30  // Fill file list does not work with 255 chars so only 30 are used.
#define FULL_PATH_NAME_LEN				256

#define EXCEPT_HANDL_MES_LEN			400


// Licensing
#define VALIDATION_STRING_LEN					8			// length of the above 'VALIDATION_STRING'
#define DONGLE_SERIAL_NUM_LEN					31
#define CONFIGURATION_NAME_LEN					21
#define DONGLE_HEX_SERIAL_NUM_LEN				8
#define VER_NUM_LEN								10
#define DONGLE_PREPARE_CHECKSUM_STRING_LEN		150 // length of checksum preparation string.
													// should be big enough to hold config_name and all CArrowConfiguration's variables
#define DONGLE_FINAL_CHECKSUM_STRING_LEN		5	// DWORD contains 4 bytes == 8 hex digits (+ null terminator)


#define ISDN_PHONE_NUMBER_DIGITS_LEN			31  // 30 characters + terminator
#define PHONE_NUMBER_DIGITS_LEN					31  // 30 characters + terminator

// SNMP
#define SNMP_STRING_LEN 						80

//Faiolver
#define FAILOVER_STRING_LEN 					80


#define GATEKEEPER_NAME_LEN                     80
#define MAX_INFO_STRING_LEN                     512

#endif // __STRINGS_LEN__

