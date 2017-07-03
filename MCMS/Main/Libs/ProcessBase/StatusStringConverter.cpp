// StatusStringConverter.cpp

#include "StatusStringConverter.h"
#include <stdio.h>

#include "StatusesGeneral.h"
#include "DefinesGeneral.h"
#include "ApiStatuses.h"
#include "InternalProcessStatuses.h"
#include "psosxml.h"
#include "SysConfigKeys.h"

CStatusStringConverter::CStatusStringConverter()
{
  InitAllStatusString();
}

void CStatusStringConverter::InitAllStatusString()
{
	AddStatusString(STATUS_OK, "Status OK");
	AddStatusString(STATUS_FAIL, "Failure Status");
	AddStatusString(STATUS_VERSION_ALREADY_INSTALLED, "Version already installed");
	AddStatusString(STATUS_KEYCODE_INVALID, "Invalid key code");
	AddStatusString(STATUS_KEYCODE_ILLEGAL, "Illegal valid code");
	AddStatusString(STATUS_KEYCODES_VERSIONS_MISMATCH, "Key code versions mismatch");
	AddStatusString(STATUS_MCU_VERSION_INSTALL_FAILED, "Installation of MCU version failed");
	AddStatusString(STATUS_NO_FALLBACK_VERSION,"Installation of previous version failed no fallback version");
	AddStatusString(STATUS_NEW_MCU_VERSION_MISMATCHES_KEYCODE_VERSION, "New MCU Version does not match key code version");
	AddStatusString(STATUS_NEW_KEYCODE_VERSION_MISMATCHES_MCU_VERSION, "New key code version does not match MCU version");
	AddStatusString(STATUS_IN_PROGRESS, 			"In progress");
	AddStatusString(STATUS_ILLEGAL, 				"Illegal status");
	AddStatusString(STATUS_INCONSISTENT_PARAMETERS, "Inconsistent parameters");
	AddStatusString(STATUS_OUT_OF_RANGE, 			"Value is out of range");
	AddStatusString(STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP,"Invalid action during system start up");
	AddStatusString(STATUS_IP_ADDRESS_NOT_VALID, 	"Invalid IP address");
	AddStatusString(STATUS_PORT_DOES_NOT_SUPPORT_SPEED_1G ,"Speed of 1Gbps is not supported by the port");

	AddStatusString(STATUS_CERTIFICATE_CN_SHOULD_BE_EQUAL_TO_USERNAME ,"The user name in TLS should be same as Certificate Common Name");
	AddStatusString(STATUS_CERTIFICATE_SHOULD_INCLUDE_SANS_PRINCIPLE ,"TLS Certificate must include SANS Principle extention");
	AddStatusString(STATUS_CERTIFICATE_SHOULD_INCLUDE_SANS,"TLS Certificate must include SANS extention");
	AddStatusString(STATUS_CERTIFICATE_FILE_NOT_EXIST,"User should create aCertificate ");
	AddStatusString(STATUS_CERTIFICATE_COMMON_NAME_NOT_EXIST,"Certificate must include Common name");

	AddStatusString(STATUS_FILE_NOT_EXISTS, 		"File does not exist");
	AddStatusString(STATUS_NOT_APPROPRIATE_MSG_FILE,"Inappropriate message file");
	AddStatusString(STATUS_SYSCALL_FAILURE,			"System error");
	AddStatusString(STATUS_NET_SERVICE_NOT_FOUND,"ISDN Network Service does not exist");
	AddStatusString(STATUS_FALIED_TO_RESET_UNITS,"Failed to reset units");
	AddStatusString(STATUS_FALIED_TO_ENABLE_UNITS,"Failed to enable units");
	AddStatusString(STATUS_FALIED_TO_DISABLE_UNITS,"Failed to disable units");
	AddStatusString(STATUS_DUPLICATE_ADDRESS,		"IP address is already defined for another network component");
    AddStatusString(OPERATION_BLOCKED, "The operation is blocked");
    AddStatusString(STATUS_BAD_IP_ADDRESS_VERSION, "Incompatible IP address version");
	AddStatusString(STATUS_ACTION_ILLEGAL_AT_FAILOVER_SLAVE_MODE,"Invalid action on Slave MCU while the system is in Hot Backup mode");
    AddStatusString(STATUS_ACTION_ILLEGAL_AT_FAILOVER_MASTER_MODE,"Invalid action on Master MCU while the system is in Hot Backup mode");
    AddStatusString(STATUS_ILLEGAL_SYS_MODE, 	"Mismatch between the conferencing entity type and the System Mode.");//2 modes cop/cp

    //Multiple links for ITP in cascaded conference feature:
    AddStatusString(STATUS_MISMATCH_IN_NUMBER_OF_ITP_LINKS,    "disconnect cause:Number of cascading links is not identical for all conferences");

	// file error
	AddStatusString(STATUS_FILE_OPEN_ERROR, 	"Error opening file");
	AddStatusString(STATUS_FILE_WRITE_ERROR, 	"Error writing file");
	AddStatusString(STATUS_FILE_READ_ERROR, 	"Error reading file");
	AddStatusString(STATUS_FILE_SEEK_ERROR, 	"Error searching file");
	AddStatusString(STATUS_FILE_DELETE_ERROR, 	"Error deleting file");
	AddStatusString(STATUS_FILE_RENAME_ERROR, 	"Error renaming file");
	AddStatusString(STATUS_FILE_FORMAT_ERROR, 	"Error in file format");
//	AddStatusString(FAULT_ID_AND_FILE_NUM_MISMATCH,"FAULT_ID_AND_FILE_NUM_MISMATCH");
	// folder error
	AddStatusString(STATUS_FOLDER_CREATE_ERROR, "Error creating folder");
	AddStatusString(STATUS_FOLDER_REMOVE_ERROR, "Error deleting folder");
	AddStatusString(STATUS_FOLDER_RENAME_ERROR, "Error renaming folder");

	AddStatusString(STATUS_LOGIN_INVALID, "Invalid Login");
	AddStatusString(STATUS_EXT_APP_STATUS_OK, "External application returned status OK");
	AddStatusString(STATUS_EXT_APP_ILLEGAL_MCU_USER_NAME_OR_PASSWORD, "External application - illegal MCU user name or pwd");
	AddStatusString(STATUS_EXT_APP_REQUEST_TIMEOUT, "External application request timeout");
	AddStatusString(STATUS_EXT_APP_ILLEGAL_NUMERIC_ID, "External application - illegal numerid ID");
	AddStatusString(STATUS_EXT_APP_ILLEGAL_CLI, "External application - illegal CLI");
	AddStatusString(STATUS_EXT_APP_ILLEGAL_CLI_OR_NUMERIC_ID, "External application - illegal CLI or numeric ID");
	AddStatusString(STATUS_EXT_APP_INTERNAL_ERROR, "Error in external application");
	AddStatusString(STATUS_EXT_APP_ILLEGAL_USER_ID	, "External application - illegal user ID");
	AddStatusString(STATUS_EXT_APP_ILLEGAL_USER_NAME_OR_PASSWORD, "External application - illegal user name or pwd");

	AddStatusString(STATUS_NO_PERMISSION, "You are not authorized to perform this operation");
	AddStatusString(STATUS_LOGIN_NOT_EXISTS, "Login name does not exist");
	AddStatusString(STATUS_PARTY_DOES_NOT_EXIST, "Participant not found");
	AddStatusString(STATUS_CONF_NOT_EXISTS, "Conference name or ID does not exist");
	AddStatusString(STATUS_TERMINATING_CONFERENCE,		"Terminating the conference");
	AddStatusString(STATUS_PARTIES_NUMBER_LIMIT_EXCEEDED,"Maximum number of participants is reached");

	AddStatusString(STATUS_FAIL_TO_LOCATE_USER, "User not found");
	AddStatusString(STATUS_OBJECT_NOT_RECOGNIZED, "Unrecognized object");
	AddStatusString(STATUS_SPAN_NOT_EXISTS, "No Span defined in Network Service");
	AddStatusString(STATUS_IP_SPAN_IS_NOT_ON_THE_SAME_SUBNET_AS_THE_OTHER_SPANS, "Span IP Address is not on the same subnet as the other spans");
	AddStatusString(STATUS_IP_ROUTER_IS_NOT_ON_THE_SAME_SUBNET_AS_THE_SPANS, "Router IP Address is not on the same subnet as the other spans");
	AddStatusString(STATUS_PARTY_IVR, "Participant is in IVR queue");
	AddStatusString(STATUS_PARTY_INCONF, "Participant is in a conference");
	AddStatusString(STATUS_PARTY_NONE, "Participant is before IVR/inconf");
	AddStatusString(STATUS_INSUFFICIENT_RSRC, "Insufficient resources");
	AddStatusString(STATUS_ILLEGAL_IN_MPM_MODE, "Invalid operation in MPM Mode");
	AddStatusString(STATUS_ILLEGAL_IN_MPM_PLUS_MODE, "Invalid operation in MPM+ Mode");
	AddStatusString(STATUS_ILLEGAL_IN_FIXED_MPM_PLUS_MODE, "Invalid operation in Fixed Resource Capacity Mode");
	AddStatusString(STATUS_MEDIA_RECORDING_FOLDER_EXCEEDED_MAX_SIZE, "The Media Recording folder exceeded allowed max size on the MCU, Please clean MediaRecording folder");
	AddStatusString(STATUS_ILLEGAL_IN_NOT_FIXED_MPM_PLUS_MODE, "Invalid operation in Flexible Resource Capacity Mode in MPM+ Mode");
	AddStatusString(STATUS_ILLEGAL_PORTS_CONFIGURATION, "Invalid port configuration");
	AddStatusString(STATUS_PORTS_CONFIGURATION_DOES_NOT_USE_FULL_SYSTEM_CAPACITY, "Port configuration not optimized. There are unallocated resources in the system.");
	AddStatusString(STATUS_SYSTEM_BUSY_SETTING_LAST_CONFIGURATION, "System is applying last configuration. Please try again");
	AddStatusString(STATUS_CURRENT_SLIDER_SETTINGS_REQUIRES_MORE_RESOURCES_THAN_AVAILBLE, "The current slider settings require more system resources than are currently available");
	AddStatusString(STATUS_SYSTEM_SHOULD_BE_RESET, "Please reset the system");
	AddStatusString(STATUS_ILLEGAL_PHONE_LENGTH,"Invalid phone length");
	AddStatusString(STATUS_LACK_OF_AVAILABLE_NUMERIC_ID,"Insufficient conference ID numbers");
	AddStatusString(STATUS_INSUFFICIENT_MNTR_CONF_ID,"Insufficient conference ID numbers for monitoring");
	AddStatusString(STATUS_SYSTEM_CFG_NO_VALID,"Invalid system configuration");
	AddStatusString(STATUS_ILLEGAL_CONFERENCE_ID_LENGTH, "Invalid conference ID length. ID length must be in the range defined by the system flags: NUMERIC_CONF_ID_MIN_LEN and NUMERIC_CONF_ID_MAX_LEN.");
	AddStatusString(STATUS_NUMERIC_CONFERENCE_ID_OCCUPIED, "Conference ID is in use");
	AddStatusString(STATUS_NUMERIC_CONF_ID_OCCUPIED_BY_SIP_FACTORY, "Cannot create a Meeting Room with NID 7001 because it is used by the SIP Factory");
	AddStatusString(STATUS_OPERATOR_PARTY_CAN_NOT_BE_DELETED, "An Operator participant cannot be deleted");
	AddStatusString(STATUS_INSUFFICIENT_RTM_RSRC, "Insufficient RTM resources");
	AddStatusString(STATUS_INSUFFICIENT_ART_RSRC, "Insufficient ART resources");
	AddStatusString(STATUS_INSUFFICIENT_ART_CHANNELS_RSRC, "Insufficient ART channels resources");
	AddStatusString(STATUS_INSUFFICIENT_VIDEO_RSRC, "Insufficient video resources");
	AddStatusString(STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED, "Number of voice parties exceeded");
	AddStatusString(STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED, "Number of video parties exceeded");
	AddStatusString(STATUS_NUMBER_OF_MAX_PARTIES_EXCEEDED,"Failed to add conference. Number of reserved resources for participants is higher than Maximum Number of Participants defined in the Profile");
	AddStatusString(STATUS_MIN_VIDEO_PARTIES_EXCEEDS_MAX_VIDEO_PARTIES_PER_CONF,"Reserved resources for video participants exceeds the maximum of video participants per conference. Additional connections of video participants are denied");
	AddStatusString(STATUS_MIN_VIDEO_PLUS_AUDIO_PARTIES_EXCEEDS_MAX_PARTIES_PER_CONF,"Reserved resources for video and audio participants exceeds the maximum number of participants per conference. Additional connections of video participants are denied");
	AddStatusString(STATUS_MIN_AUDIO_PARTIES_EXCEEDS_MAX_PARTIES_PER_CONF,"Reserved resources for audio participants exceeds the maximum number of audio participants per conference. Additional connections of participant are denied.");
	AddStatusString(STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN, "Wait for resource and ask again");
	AddStatusString(STATUS_NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER, "No utilizable unit for audio controller");

	AddStatusString(STATUS_INSUFFICIENT_UDP_PORTS, "Insufficient UDP ports");
	AddStatusString(STATUS_PHONE_NUMBER_OCCUPIED, "ISDN dial-in number is already assigned to another conferencing entity");
	AddStatusString(STATUS_PROCESS_FAILURE, "Process failure");
	AddStatusString(STATUS_DECOMPRESSION_FAILED, "Decompression failed");
	AddStatusString(STATUS_REQUEST_TOO_LONG,"Request is too long");
	AddStatusString(STATUS_RESPONSE_TOO_LONG,"Response is too long");
	AddStatusString(STATUS_ALIAS_E164_TYPE_NAMES_MUST_BE_NUMERIC,"E164 Alias must be numeric");
	AddStatusString(STATUS_IP_ADDRESS_IN_SPAN_NOT_VALID,"Invalid span IP address");
	AddStatusString(STATUS_IP_SPAN_IS_NOT_ON_THE_SAME_SUBNET_AS_THE_ROUTERS,"Router IP Address is not on the same subnet as the other routers");
	AddStatusString(STATUS_NUMBER_OF_H323_SERVICES_EXCEEDED,"Number of H.323 Network Services exceeded");
	AddStatusString(STATUS_H323_SERVICE_NAME_EXISTS,"H.323 Network Service name already exists");
	AddStatusString(STATUS_H323_SERVICE_NAME_NOT_EXISTS,"H.323 Network Service name does not exist");
	AddStatusString(STATUS_NUMBER_OF_ROUTERS_IN_SERVICE_EXCEEDED,"Number of routers in the IP Network Service exceeded");
	AddStatusString(STATUS_ROUTER_EXISTS,"Router already exists");
	AddStatusString(STATUS_ROUTER_NOT_EXISTS,"Router does not exist");
	AddStatusString(STATUS_EMPTY_ALIAS_NAMES_LIST_IN_SPAN,"Span Aliases list is empty");
	AddStatusString(STATUS_IP_ROUTER_IS_NOT_ON_THE_SAME_SUBNET_AS_THE_OTHER_ROUTERS,"Router IP Address is not on the same subnet as the other routers");
	AddStatusString(STATUS_ILLEGAL_PORT_NUMBER_ABOVE_62999,"Invalid Port number. Number higher than 62999");
	AddStatusString(STATUS_ILLEGAL_PORT_NUMBER_BELOW_1025,"Invalid Port number. Number lower than 1025");
	AddStatusString(STATUS_ILLEGAL_RESERVED_PORT_IN_RANGE,"Invalid Port number. Number is out of defined range");
	AddStatusString(STATUS_THE_NUMBER_OF_PORTS_ALLOCATED_IS_BELOW_THE_MINIMUM,"Number of allocated ports is below the required minimum");
	AddStatusString(STATUS_INVALID_GATEKEEPER_ADDRESS,"Invalid gatekeeper IP address");
	AddStatusString(STATUS_PREFIX_MUST_BE_NUMERIC,"MCU prefix must be numeric");
	AddStatusString(STATUS_INVALID_GATEKEEPER_POLLING_INTERVAL,"Invalid gatekeeper polling interval");
	AddStatusString(STATUS_ALIAS_NAMES_CANNOT_BEGIN_WITH_A_COMMA,"Alias names cannot begin with a comma");
	AddStatusString(STATUS_ALIAS_NAMES_CANNOT_INCLUDE_SEMICOLON,"Alias names cannot include semicolons");
	AddStatusString(STATUS_ALIAS_NAMES_CANNOT_INCLUDE_QUOTATION_MARK,"Alias names cannot include quotation marks");
	AddStatusString(STATUS_ILLEGAL_VIDEO_PORT_ALLOCATION,"Invalid video port allocation");
	AddStatusString(STATUS_ILLEGAL_AUDIO_PORT_ALLOCATION,"Invalid audio port allocation");
	AddStatusString(STATUS_ILLEGAL_FECC_PORT_ALLOCATION,"Invalid FECC port allocation");
	AddStatusString(STATUS_ILLEGAL_CONTENT_PORT_ALLOCATION,"Invalid Content port allocation");
	AddStatusString(STATUS_UNKNOWN_CONFIGURATION_FOR_SIP_SERVERS,"Invalid SIP server configuration");
	AddStatusString(STATUS_SIP_TLS_NOT_SUPPORTED_IN_JITC_MODE,"SIP TLS is not supported in Enhance Security Mode");
	AddStatusString(STATUS_INVALID_NAT_IP_ADDRESS,"Invalid NAT IP address");
	AddStatusString(STATUS_UNKNOWN_NAT_STATUS,"NAT status unknown");
	AddStatusString(STATUS_UNKNOWN_DNS_STATUS,"DNS status unknown");
	AddStatusString(STATUS_INVALID_DNS_SERVER_ADDRESS,"Invalid DNS server IP address");
	AddStatusString(STATUS_INVALID_DNS_STATUS,"Invalid DNS status");
	AddStatusString(STATUS_UNKNOWN_SIP_ONGOING_CONFRENCES_STATUS,"Registration status of ongoing conferences with SIP server is unknown");
	AddStatusString(STATUS_UNKNOWN_SIP_MEETING_ROOM_STATUS,"Registration status of Meeting Rooms with SIP server is unknown");
	AddStatusString(STATUS_UNKNOWN_SIP_ENTRY_QUEUES,"Registration status of Entry Queues with SIP server is unknown");
	AddStatusString(STATUS_UNKNOWN_SIP_ACCEPT_MEET_ME_STATUS,"Accept status of Meet Me conferences registration with SIP server is unknown");
	AddStatusString(STATUS_UNKNOWN_SIP_ACCEPT_AD_HOK_STATUS,"Accept status of Ad Hoc Entry Queues registration with SIP server is unknown");
	AddStatusString(STATUS_UNKNOWN_SIP_ACCEPT_FACTORY_STATUS,"Accept status of Factories registration with SIP server is unknown");
	AddStatusString(STATUS_UNKNOWN_SIP_REFRESH_STATUS,"registration refresh status with SIP server is unknown");
	AddStatusString(STATUS_SPAN_EXISTS, "Span is already defined in Network Service");
	AddStatusString(STATUS_SPAN_OCCUPIED, "Span already assigned");
	AddStatusString(STATUS_INSUFFICIENT_RSRC_CARD_OVERLOAD, "insufficient resources - media card overload");

	// Start of IVR
	AddStatusString(STATUS_MAX_NUMBER_OF_IVR_SERVICES_EXCEEDED, 		"Maximum number of IVR Services exceeded");
	AddStatusString(STATUS_IVR_SERVICE_NAME_EXISTS,				"IVR Service name already exists");
	AddStatusString(STATUS_IVR_SERVICE_NAME_DOES_NOT_EXISTS, 		"IVR Service name does not exist");
	AddStatusString(STATUS_THIS_IS_NOT_A_NAME_OF_AN_IVR_SERVICE, 		"Invalid IVR Service name");
	AddStatusString(STATUS_IVR_ILLEAGAL_DTMF_TBL,				"Invalid DTMF table entry in Conference IVR Service");
	AddStatusString(STATUS_IVR_ILLEAGAL_ASSIST_REQEST, 			"Invalid Assistance Request");
	AddStatusString(STATUS_IVR_MSG_NAME_MISSING,				"Message file name is missing");
	AddStatusString(STATUS_IVR_MSG_ILLEGAL_NAME,				"Invalid IVR Message name or type");
	AddStatusString(STATUS_INCOMPATIBLE_IVR_DEFAULT_SERVICE, 		"Default IVR Service is incompatible with conferencing entity");
	AddStatusString(STATUS_MSG_SERVICE_NAME_ALREADY_EXISTS,			"IVR Service name already exists");
	AddStatusString(STATUS_MSG_SERVICE_NAME_DOES_NOT_EXISTS,		"IVR Service name does not exist");
	AddStatusString(STATUS_IVR_SERVICE_DISABLED,				"IVR Service disabled");
	AddStatusString(STATUS_INCOMPATIBLE_IVR_SERVICE,			"IVR Service is incompatible with Conferencing entity");
	AddStatusString(STATUS_INVALID_IVR_EXTERNAL_DB_CONFIGURATION, 		"Invalid external database configuration in IVR Service");
	AddStatusString(STATUS_IVR_EXTERNAL_DB_DISABLED,			"External database flag is disabled in System Configuration");
	AddStatusString(STATUS_INVALID_IVR_EXTERNAL_DB_PARAMETER,		"Invalid external database parameter in IVR Service");
	AddStatusString(STATUS_DEFAULT_MSG_SERVICE_CANNOT_BE_REMOVED, 		"Default IVR Service cannot be deleted");
	AddStatusString(STATUS_THE_MAXIMUM_NUMBER_OF_LANGUAGES_IN_IVR_SERVICE_IS_EXCEEDED, "Maximum number of IVR Languages exceeded");
	AddStatusString(STATUS_THIS_LANGUAGE_ALREADY_EXISTS_IN_THE_IVR_SERVICE, "Language name already exists");
	AddStatusString(STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE, "Language name does not exist");
	AddStatusString(STATUS_ILLEGAL_LANGUAGE_NAME,				"Invalid Language name");
	AddStatusString(STATUS_VIDEOCOMMANDER_IS_NOT_SUPPORTED,			"Click&View is not supported");
	AddStatusString(STATUS_ILLEGAL_DTMF_CODE_PERMISSON,			"No permission to use this DTMF code");
	AddStatusString(STATUS_ILLEGAL_DTMF_CODE_LEN,				"Invalid DTMF code length");
	AddStatusString(STATUS_ILLEGAL_DTMF_CODE_CHARACTER,			"Invalid DTMF code character");
	AddStatusString(STATUS_ILLEGAL_SERVICE_NAME,				"Invalid Service name");
	AddStatusString(STATUS_IVR_ILLEGAL_PARAMETERS,				"Invalid IVR Service parameters");
	AddStatusString(STATUS_IVR_INTERNAL_ERROR,				"IVR Service internal error");
	AddStatusString(STATUS_IVR_VIDEO_FILE_DOES_NOT_EXIST,			"Video file not found");
	AddStatusString(STATUS_IVR_AUDIO_FILE_DOES_NOT_EXIST,			"Audio file not found");
	AddStatusString(STATUS_IVR_INVALID_AUDIO_FILE,				"Invalid audio file");
	AddStatusString(STATUS_IVR_INVALID_VIDEO_FILE,				"Invalid video file");
	AddStatusString(STATUS_IVR_NUMERIC_ID_DISABLED,				"Numeric ID option in Entry Queue IVR Service is disabled");
	AddStatusString(STATUS_IVR_BILLING_CODE_MSG_ERROR,			"Error in Billing Code IVR message");
	AddStatusString(STATUS_RECORDING_DISABLED_IN_CONF,			"Recording is not enabled for this conference");
	AddStatusString(STATUS_IVR_DTMF_CODE_NOT_SUPPORTED_IN_SOFT_MCU,			"IVR DTMF code is not supported in soft mcu");
	AddStatusString(STATUS_IVR_GENERAL_FEATURE_NOT_SUPPORTED_IN_SOFT_MCU,   "IVR General feature not supported in soft mcu");
	AddStatusString(STATUS_IVR_ROLL_COLL_NOT_SUPPORTED_IN_SOFT_MCU,   "IVR Roll call not supported in soft mcu");

	AddStatusString(STATUS_ILLEGAL_WHILE_MEETING_ROOM_EXISTS, 		"IVR Service cannot be deleted while it is assigned to a Meeting Room");
	AddStatusString(STATUS_ILLEGAL_WHILE_ENTRY_QUEUE_EXISTS, 		"IVR Service cannot be deleted while it is assigned to an Entry Queue");
	AddStatusString(STATUS_ILLEGAL_WHILE_CONFERENCES_EXISTS, 		"Conference IVR Service cannot be deleted while it is assigned to an active conference");
	AddStatusString(STATUS_ILLEGAL_WHILE_EQ_IS_ACTIVE,              "An Entry Queue cannot be deleted while it is active");
	AddStatusString(STATUS_ILLEGAL_WHILE_MR_IS_ACTIVE,              "Meeting Room cannot be deleted while it is active");
	AddStatusString(STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_EXISTS, "This action is illegal while there are ongoing conferences");

	AddStatusString(STATUS_IVR_ILLEGAL_WHILE_PROFILE_EXISTS, 		"IVR Service cannot be deleted while it is assigned to a Profile");
	AddStatusString(STATUS_NO_IVR_SERVICE_IN_LIST, 				"No Conference IVR Service defined in the list");
	AddStatusString(STATUS_NO_EQ_SERVICE_IN_LIST, 				"No Entry Queue IVR Service defined in the list");
	AddStatusString(STATUS_ILLEGAL_PASSWORD_LENGTH, 			"Invalid password length");
	AddStatusString(STATUS_MAX_PASSWORD_REPEATED_DIGITS_EXCEEDED,	"Maximum number of permitted repeated characters in password has been exceeded");
	AddStatusString(STATUS_PASSWORD_NOT_VALID, 				"Invalid password");
	AddStatusString(STATUS_CASCADE_LINK_CANNOT_BE_SET_AS_CHAIRPERSON, 	"Cascade link cannot be set as the conference chairperson");
	AddStatusString(STATUS_PARTY_IN_IVR_CANNOT_BE_SET_AS_CHAIRPERSON, 	"Participant in IVR queue cannot be set as the conference chairperson");
	// End of IVR

	AddStatusString(STATUS_NO_REQUEST_OBJECT, 				"No request object");
	AddStatusString(STATUS_PROFILE_NOT_FOUND, 				"Profile not found");
	AddStatusString(STATUS_AD_HOC_EQ_MUST_HAVE_PROFILE,			"A profile must be assigned to the Entry Queue");
	AddStatusString(STATUS_PARTY_INTERFACE_TYPE_DOES_NOT_MATCH_SERVICE_TYPE,"Participant Interface Type setting does not match the selected Network Service");
	AddStatusString(STATUS_GATEWAY_MUST_HAVE_DIAL_OUT_PROTOCOL,"At least one dial out protocol must be selected for gateway calls");

	// CSMngr <-> EMA
	AddStatusString(STATUS_TOO_MANY_SERVICES, "Maximum number of Services already defined");
	AddStatusString(STATUS_SERVICE_PROVIDER_NAME_EXISTS, "Service provider name already exists");
	AddStatusString(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS,	 	"Service provider name not found");
	AddStatusString(STATUS_ENCRYPTION_IS_CURRENTLY_NOT_AVAILABLE, 		"Encryption is not available");
	AddStatusString(PARTICIPANT_ENCRYPTION_SETTINGS_DO_NOT_MATCH_THE_CONFERENCE_SETTINGS,
			"Participant encryption settings do not match conference settings");
	AddStatusString(STATUS_CONF_IS_SECURED, 		"This action is forbidden in Secured conferences");
	AddStatusString(STATUS_ADD_PARTICIPANT_TO_SECURE_CONF, "Adding participants to a Secured Conference is not permitted");
	AddStatusString(STATUS_CAN_NOT_ADD_SIP_H320_TO_VSW_LPR_CONFERENCE, "SIP and H.320 participants cannot be added to a Video Switching LPR-enabled conference");
	AddStatusString(STATUS_ISDN_VIDEO_PARTICIPANT_IS_NOT_SUPPORTED_IN_VS_CONF, "ISDN participants cannot be added to a HD Video Switching conference");
	AddStatusString(STATUS_AV_MCU_NOT_SUPPORTED_IN_MIXED_MODE_CONF, "Lync AV MCU cannot connect PLCM mixed AVC/SVC conference");
	AddStatusString(STATUS_NO_MORE_THEN_1_AV_MCU_LINK_IN_CONF_IS_ALLOWED, "Lync AV MCU cannot connect more than 1 AV MCU link");
	AddStatusString(STATUS_ISDN_CONNECTIONS_NOT_SUPPORTED,"ISDN connections are not supported");
	AddStatusString(STATUS_ENCRYPTION_NOT_AVAILABLE_IN_SIP_PARTY, 		"Encryption is not supported by SIP");
	AddStatusString(STATUS_ENCRYPTION_IS_NOT_AVAILABLE_IN_H320_PARTY,  	"ISDN Encryption is not supported");
	AddStatusString(STATUS_NO_NUM_OF_PORTS, "Central signaling failed to allocate ports");
	AddStatusString(STATUS_INVALID_SERVICE, "Invalid IP Network Service");
	AddStatusString(PARTICIPANT_SIP_ENCRYPTION_SETTINGS_DO_NOT_MATCH_SERVICE_TYPE,
				"Failed to connect to encrypted conference. Transport Type is not set to TLS in the IP Network Service");
	// Add Here
	AddStatusString(STATUS_LECTURE_MODE_AND_SAME_LAYOUT_ARE_NOT_AVAILABLE_IN_THE_SAME_CONFERENCE, "You cannot select Lecture Mode and Same Layout simultaneously");
	AddStatusString(STATUS_PRESENTATION_MODE_AND_SAME_LAYOUT_ARE_NOT_AVAILABLE_IN_THE_SAME_CONFERENCE, "You cannot select Presentation Mode and Same Layout simultaneously");
	AddStatusString(STATUS_IN_SAME_LAYOUT_MODE_VIDEO_FORCE_IS_FORBIDDEN, "Video forcing of participants is not allowed when Same Layout is active");
	AddStatusString(STATUS_AN_AUDIO_ONLY_PARTICIPANT_CANNOT_BE_SELECTED_AS_LECTURER, "Audio only participant cannot be selected as the conference lecturer");
	AddStatusString(STATUS_VIDEO_SOURCE_NOT_EXISTS, "No Video Source");
	AddStatusString(STATUS_VIDEO_SOURCE_EXISTS, "Video Source already exists");
	AddStatusString(STATUS_CELL_ID_NOT_EXISTS, "Cell ID does not exist");
	AddStatusString(STATUS_CELL_ID_EXISTS, "Cell ID already exists");
	AddStatusString(STATUS_PARTY_IS_PRESENT_IN_SEVERAL_CELLS, "Participant is already displayed in several video windows");
	AddStatusString(STATUS_NUMBER_OF_PARTICIPANTS_EXCEEDS_THE_DEFINED_MAXIMUM_FROM_PROFILE,"Number of participants exceeds maximum defined in Conference");
	AddStatusString(STATUS_AUTO_SELECT_NOT_SUPPORTED_IN_LECTURE_MODE,"Auto select is not supported in Lecture Mode");
    AddStatusString(STATUS_H239_SESSION_CANNOT_BE_ABORTED_FROM_A_SLAVE_CONFERENCE,"H239 session cannot be aborted from a slave conference");
	AddStatusString(STATUS_RESTRICT_CONTENT_CANNOT_BE_SET_ON_A_SLAVE_CONFERENCE,"Restrict content cannot be set on a slave conference");
    AddStatusString(STATUS_ILLEGAL_LAYOUT_COMPOSITION,"In Video Switching conferences the layout window cannot be defined as blank");
	AddStatusString(STATUS_IN_EVENT_MODE_CONFERENCE_PERSONAL_LAYOUT_IS_FORBIDDEN , "Personal layout and video forcing of participants is not permitted in Event Mode conferences");
        AddStatusString(STATUS_IN_VSW_CONFERENCE_PRIVATE_LAYOUT_IS_FORBIDDEN, "Personal layout is not permitted in Video Switching conferences");
	AddStatusString(STATUS_CANNOT_CHANGE_CONTENT_OWNER_IN_LECTURE_MODE, " Can't change content owner in Lecture Mode");

    AddStatusString(STATUS_ILLEGAL_PARTY_NAME, "Participant name is missing");
	AddStatusString(STATUS_PARTY_NAME_EXISTS, "Participant name already exists");
	AddStatusString(STATUS_PARTY_VISUAL_NAME_EXISTS, "Participant Display Name already in use");
	AddStatusString(STATUS_CANNOT_UPDATE_ACTIVE_MR_OR_EQ,"Cannot update an active Meeting Room or Entry Queue");
	AddStatusString(STATUS_PARTY_IP_ALIAS_ALREADY_EXISTS,"Participant IP alias already exists");
	AddStatusString(STATUS_ILLEGAL_CASCADE_TOPOLOGY,"The Conference's Cascade Topology does not support this type of call.");
	AddStatusString(STATUS_CANNOT_ADD_DEFINED_PARTY_TO_EQ,"Defined participants cannot be added to an Entry Queue");
	AddStatusString(STATUS_CANNOT_ADD_DEFINED_DIAL_IN_CASCADED_LINK,"Dial-in Cascade Link cannot be defined in a conference");
	AddStatusString(STATUS_RESERVATION_NAME_EXISTS,"Display name already exists");
	AddStatusString(STATUS_RESERVATION_ROUTING_NAME_EXISTS,"Routing name already exists");
	AddStatusString(STATUS_DONGLE_ENCRYPTION_VAL_IS_FALSE,"Wrong encryption value in dongle");
	AddStatusString(STATUS_INVALID_CHARACTER_IN_PARTY_NAME,"Invalid character in participant name");
	AddStatusString(STATUS_INVALID_CHARACTER_IN_CONFERENCE_NAME,"Invalid character in conference Routing Name");
	AddStatusString(STATUS_MAX_RESERVATIONS_EXCEEDED,"Maximum number of conferences exceeded");
	AddStatusString(STATUS_PROFILE_IN_USE_CANNOT_BE_DELETED,"Profile in use and cannot be deleted");
	AddStatusString(DEFAULT_PROFILE_CANNOT_BE_DELETED,"Default Profile cannot be deleted");
	AddStatusString(STATUS_MAX_CONF_TEMPLATES_EXCEEDED,"Maximum number of Conference Templates has been reached");
	AddStatusString(STATUS_CONFERENCE_CAN_NOT_BE_SAVED_AS_TEMPLATE,"SIP Factories and Entry Queues cannot be saved as Conference Templates");
	AddStatusString(STATUS_RESERVATION_NOT_EXISTS,"Conference not found");
	AddStatusString(STATUS_MAX_PROFILES_EXCEEDED,"Maximum number of Profiles reached");
	AddStatusString(STATUS_CONNECT_DISCONNECT_BLOCKED,"Operation cannot be performed while current operation is in progress");
	AddStatusString(STATUS_PROFILE_NAME_ALREADY_EXISTS,"Profile name already exists");
	AddStatusString(STATUS_CONF_TEMPLATE_NAME_ALREADY_EXISTS,"Conference Template Display Name or Routing Name is already assigned to another conferencing entity.");
	AddStatusString(STATUS_RESERVATION_NAME_EXISTS_IN_MEETING_ROOM_DB,"Conference name already exists in Meeting Rooms list");
	AddStatusString(STATUS_RESERVATION_ROUTING_NAME_EXISTS_IN_MEETING_ROOM_DB,"Conference Routing name already exists in Meeting Rooms list");
	AddStatusString(STATUS_RESERVATION_NAME_EXISTS_IN_EQ_LIST,"Conference name already exists in EntryQ list");
	AddStatusString(STATUS_ILLEGAL_BILLING_DATA_LENGTH,"Billing data length exceeded");
	AddStatusString(STATUS_CONFERENCE_RATE_BELLOW_HD_THRESHOLD,"Conference Line Rate is less than VSW resolution threshold");
	AddStatusString(STATUS_CONF_RATE_BELLOW_MIN_TIP_COMPATIBILITY_LINE_RATE,"Conference line rate is lower than the minimum required for TIP compatibility.");
	AddStatusString(STATUS_PARTY_RATE_BELLOW_MIN_TIP_COMPATIBILITY_LINE_RATE,"Participant line rate is lower than the minimum required for TIP compatibility.");
	AddStatusString(STATUS_CONF_NAT_KEEP_ALIVE_PERIOD_HIGHER_THAN_EXPECTED,"Conference NAT keep alive period is higher than expected 0..86400");
	AddStatusString(STATUS_PARTY_VIDEO_RESOLUTION_IS_NOT_TIP_COMPATIBLE, "Participants video resolution must be HD720p or higher.");
	AddStatusString(STATUS_PARTY_VIDEO_PROTOCOL_IS_NOT_TIP_COMPATIBLE, "Party protocol is not H264.");
	AddStatusString(STATUS_PARTY_IP_ALREADY_EXISTS,"Participant IP address already exists");
	AddStatusString(STATUS_PARTY_ALIAS_ALREADY_EXISTS,"Participant alias already exists");
	AddStatusString(STATUS_INVALID_PARTY_H320_BITRATE,"Conference bit rate must be set to a minimum of 128Kbps to enable ISDN participant connection");
	AddStatusString(STATUS_H320_DEFINED_DIALIN_IS_NOT_SUPPORTED,"H.320 defined dial-in participants are not supported");
	AddStatusString(STATUS_NUM_OF_AUDIO_PARTIES_EXCEEDS_MIN_AUDIO_PARTIES,"The added participant requires audio resources that were not reserved and may not be connected when the conference starts.");
	AddStatusString(STATUS_NUM_OF_VIDEO_PARTIES_EXCEEDS_MIN_VIDEO_PARTIES,"The added participant requires video resources that were not reserved and may not be connected when the conference starts.");
	AddStatusString(STATUS_NUM_OF_PARTIES_EXCEEDS_MIN_VIDEO_PLUS_AUDIO_PARTIES,"The added participant requires resources that were not reserved and may not be connected when the conference starts.");
	AddStatusString(STATUS_NODE_MISSING, "Field value is missing");
	AddStatusString(STATUS_NODE_VALUE_INVALID, "Invalid field value");
	AddStatusString(STATUS_NODE_LENGTH_TOO_LONG, "Field value is too long");
	AddStatusString(STATUS_FAIL_TO_VALIDATE_RESPONSE, "Response validation failed");
	AddStatusString(STATUS_TRANSACTION_IS_NOT_WELL_FORMED, "Transaction is not well formed");
	AddStatusString(STATUS_ACTION_NODE_IS_MISSING_OR_INVALID, "Field action is missing or invalid");
	AddStatusString(STATUS_VALUE_OUT_OF_RANGE, "value is out of range");
	AddStatusString(STATUS_INVALID_KEY, "Invalid key");
	AddStatusString(STATUS_NODE_LENGTH_TOO_SHORT, "Field value is too short");
	AddStatusString(STATUS_MIN_MAX_MISSING_IN_KEY, "Key minimum and maximum values are missing");
	AddStatusString(STATUS_API_INTERNAL_ERROR, "API internal error");
	AddStatusString(STATUS_ENUM_VALUE_INVALID, "Invalid ENUM value");
	AddStatusString(STATUS_IP_ADDRESS_INVALID, " Invalid IP address");
	AddStatusString(STATUS_ATM_ADDRESS_INVALID, "Invalid ATM address");
	AddStatusString(STATUS_DATE_TIME_INVALID, "Invalid date and time");
	AddStatusString(STATUS_ILLEGAL_END_TIME, "Invalid end time");
	AddStatusString(STATUS_TELEPRESENCE_CHANGED_TO_OFF_ITP_CERTIFICATION_IS_OFF_IN_SYSTEM, "Telepresence Mode cannot be enabled because the ITP_CERTIFICATION system flag is set to OFF");

	// Move Statuses
	AddStatusString(STATUS_MOVE_VSW_VSW_IS_FORBIDDEN, "Participants cannot be moved between Video Switching conferences");
	AddStatusString(STATUS_MOVE_CP_VSW_IS_FORBIDDEN, "Participants cannot be moved between Video Switching conferences and Continuous Presence conferences");
	AddStatusString(STATUS_MOVE_ENC_NON_ENC_IS_FORBIDDEN, "A non-encrypted participant cannot be moved to an encrypted conference");
    AddStatusString(STATUS_MOVE_TELEPRESENCE_NON_TELEPRESENCE_IS_FORBIDDEN,"Participants cannot be moved between Telepresence conferences and standard conferences");
	AddStatusString(STATUS_MOVE_CASCADED_LINK_IS_FORBIDDEN, "A cascaded link cannot be moved between conferences");
	AddStatusString(STATUS_MOVE_IP_LPR_NON_LPR_IS_FORBIDDEN, "IP participants cannot be moved between LPR-enabled conferences and standard conferences");
	AddStatusString(STATUS_PARTY_ILLEGAL_STATE_FOR_MOVE,"Participants cannot be moved during the connection or disconnection phase");
	AddStatusString(STATUS_MAX_PARTIES_OF_DESTINATION_CONF_EXCEEDED,"The participant cannot be moved to the destination conference because it already includes the maximum number of participants");
	AddStatusString(STATUS_MAX_VIDEO_PARTIES_OF_DESTINATION_CONF_EXCEEDED,"The participant cannot be moved to destination conference because it already includes the maximum number of video participants");
	// High Profile
	AddStatusString(STATUS_VIDEO_PREVIEW_NOT_SUPPORTED_WITH_HIGH_PROFILE,"Video Preview is not supported with High Profile");
	AddStatusString(STATUS_H261_H263_NOT_SUPPORTED_WITH_HIGH_PROFILE,"H.261 CIF and H.263 CIF is not supported with High Profile");

	AddStatusString(STATUS_VIDEO_PREVIEW_DISABLED,"Video Preview is disabled");

	AddStatusString(STATUS_ACCESS_DENIED, "Access denied");
	AddStatusString(STATUS_BROKEN_PIPE, "Broken pipe");
	AddStatusString(STATUS_TRANSPORT_NOTCONNECTED, "Transport not connected");

	AddStatusString(STATUS_NOT_FOUND, "Not found");
	AddStatusString(STATUS_SERVER_ERROR, "Server error");
	AddStatusString(STATUS_CANT_FIND_VIRTUAL_DIRECTORY, "Virtual Directory not found");
	AddStatusString(STATUS_DIRECTORY_ALREADY_EXISTS, "Directory already exists");
	AddStatusString(STATUS_QUEUE_TIMEOUT, "Queue timeout");

	AddStatusString(STATUS_CONF_NAME_EXISTS,"Display name already exists");
	AddStatusString(STATUS_ILLEGAL_OPERATION_VIDEO_CAPABILITIES_INACTIVE,"Invalid operation. No Video Capabilities");
	AddStatusString(STATUS_OPEN_FILE_FAILED, "Failed to open file");
	AddStatusString(STATUS_UNKNOWN_FILE_FAILURE, "File error");
	AddStatusString(STATUS_PARSING_XML_FILE_FAILED, "Failed to parse XML file");
	AddStatusString(STATUS_UNZIP_XML_FILE_FAILED, "Failed to UnZip XML file");
	AddStatusString(STATUS_CONFERENCE_ID_MUST_BE_NUMERIC,"Conference ID must be numeric");
	AddStatusString(STATUS_PHONE_MUST_BE_NUMERIC,"Phone must be numeric");
	AddStatusString(STATUS_MEETING_ROOM_NOT_EXISTS,"Meeting Room does not exist");

	AddStatusString(STATUS_NUMBER_OF_OPERATORS_IN_MCU_EXCEEDED, "Maximum number of user connections exceeded");
	AddStatusString(STATUS_EXE_NOT_FOUND, "EXE not found");
	AddStatusString(STATUS_LOGIN_EXISTS, "A user with this login name already exists");
	AddStatusString(STATUS_EMPTY_PARTY_ALIAS_NAME, "Participant alias name is empty");
	AddStatusString(STATUS_ALIAS_NAME_TOO_LONG,"Alias name is too long");
	AddStatusString(STATUS_ALIAS_NAMES_CANNOT_INCLUDE_A_COMMA,"Invalid alias name. The characters ',', ';', ''', and '\"' are not permitted in H323 ID alias names");
	AddStatusString(STATUS_ILLEGAL_EMAIL_ID_ALIAS,"Invalid alias name. Email ID alias names must have the format abc@example.com");
	AddStatusString(STATUS_ALIAS_E164_TYPE_NAMES_MUST_BE_ALPHA_NUMERIC,"Invalid alias name. E164 alias names can only contain *, #, and 0-9");
	AddStatusString(STATUS_INVALID_ALIAS,"Invalid alias");
	AddStatusString(STATUS_INVALID_ALIAS_TYPE,"Invalid alias type");

	AddStatusString(STATUS_LAST_SUPERVISOR_OPERATOR_IN_MCU, "At least one Administrator must be defined in the MCU");
	AddStatusString(STATUS_LAST_ENABLED_SUPERVISOR_OPERATOR_IN_MCU, "At least one Administrator must be defined in the MCU");
	AddStatusString(STATUS_ILLEGAL_CHARACTERS_IN_LOGIN_NAME,	"Login name contains invalid characters");
	AddStatusString(STATUS_ILLEGAL_CHARACTERS_IN_PASSWORD,		"Password contains invalid characters");
	AddStatusString(STATUS_EMPTY_PASSWORD,						"Password is empty");
	AddStatusString(STATUS_FORCE_CHANGE_PASSWORD,				"User must change password");
	AddStatusString(STATUS_PASSWORD_CANNOT_BE_CHANGED_YET,		"Password change is not permitted during time period specifed for Password retention");
	AddStatusString(STATUS_INVALID_STRONG_PASSWORD,				"Password does not meet strong password criteria");
	AddStatusString(STATUS_INVALID_JITC_STRONG_PASSWORD,		"Password characteristics do not comply with Enhanced Security Mode requirements");
	AddStatusString(STATUS_AT_LEAST_4_CHARACTERS_IN_PASSWORD_MUST_BE_CHANGED,		"The New password must differ in at least 4 characters from the old password.");
	AddStatusString(STATUS_NEW_PASSWORD_WAS_USED_RECENTLY,		"New password was used recently.");
	AddStatusString(STATUS_PASSWORD_LENGTH_IS_TOO_SHORT,		"Password is too short.");
	AddStatusString(STATUS_ACCOUNT_DISABLED,					"Account is disabled");
	AddStatusString(STATUS_ACCOUNT_LOCKED,						"The User Account is locked.");
    AddStatusString(STATUS_SYSTEM_SESSION_LIMIT_EXCEEDED,       "Maximum number of permitted user connections has been exceeded. New connection is denied.");

    AddStatusString(STATUS_USER_SESSION_LIMIT_EXCEEDED,         "A user with this name is already logged into the system. Additional connection is denied.");
	AddStatusString(STATUS_ONLY_ONE_ACCOUT_PER_ROLE_ALLOWED,	"Account of this type already exists");
	AddStatusString(STATUS_ACCOUNT_AUTHORIZATION_LEVEL_NOT_ALLOWED, "Account authorization level is not allowed in Enhanced Security Mode");
	AddStatusString(STATUS_SUPPORT_USER_NAME_IS_NOT_ALLOWED_JITC, "Username SUPPORT is not allowed  under the selected security settings.");

	AddStatusString(STATUS_FILE_NOT_EXIST, "File does not exist");
	AddStatusString(STATUS_UNKNOWN_SIP_REGISTRAR_STATUS, "SIP registrar status is unknown");
	AddStatusString(STATUS_WRONG_DATA_SIZE, "Invalid data size");

	AddStatusString(STATUS_OLD_PASSWORD_IS_INCORRECT, "Old password is incorrect");
	AddStatusString(STATUS_ADMIN_PASSWORD_IS_INCORRECT, "The Password of the Administrator user is incorrect");

	AddStatusString(STATUS_FAIL_TO_CONFIG_MNGMNT_IP,		"Failed to configure the Management Network Service");
	AddStatusString(STATUS_FAIL_TO_CONFIG_MNGMNT_ETHERNET,	"Failed to configure the Ethernet of the Management Network Service");

    AddStatusString(STATUS_ENCODING_CONVERTION_FAILED, "Failed to convert encoding");
    AddStatusString(STATUS_ENCODING_VALIDATION_FAILED, "Failed to validate encoding");
    AddStatusString(STATUS_CONVERTION_UTF8_TO_ORIGIN_FAILED, "Failed to convert UTF-8 to origin character set");

    AddStatusString(STATUS_NODE_VALUE_NOT_ASCII, "Field Value must be ASCII");
    AddStatusString(STATUS_UNKNOWN_CHARSET_ENCODING, "Cannot encode unknown character set");
    AddStatusString(STATUS_CHARSET_ENCODING_UNDETERMINABLE, "Cannot determine encoding of character set");

	AddStatusString(STATUS_FAIL_TO_CONFIG_DNS,			"DNS configuration failed");
	AddStatusString(STATUS_FAIL_TO_CONFIG_DHCP,			"Failed to configure the DHCP in the Management Network Service");

	AddStatusString(STATUS_SERVICE_NO_NAME, "Service name is missing");
	AddStatusString(STATUS_SERVICE_NO_SPAN, "Span definition in the Network Service is missing");
	AddStatusString(STATUS_GK_ALIAS_EMPTY_NAME, "Gatekeeper alias is missing");
	AddStatusString(STATUS_SERVICE_DUPLICATE_IP_CS_MNGMNT, "Central Signaling IP address already exists in the Network Service");
	AddStatusString(STATUS_GK_PREFIX_NOT_NUMERIC, "MCU Prefix in gatekeeper must be numeric");
	AddStatusString(STATUS_GK_PREFIX_TOO_MANY_DIGITS, "MCU Prefix in gatekeeper contains too many digits");
	AddStatusString(STATUS_GK_PREFIX_TOO_BIG, "MCU Prefix in gatekeeper is too long");
	AddStatusString(STATUS_SERVICE_NOT_EXIST, "Network Service does not exist");
	AddStatusString(STATUS_SERVICE_EXIST, "Network Service name already exists");
	AddStatusString(STATUS_IP_ADDRESS_MISMATCHES_NETMASK, "IP Address does not match the subnet mask");
	AddStatusString(STATUS_ILLEGAL_NUM_TCP_PORTS, "Invalid number of TCP ports");
	AddStatusString(STATUS_ODD_NUM_TCP_PORTS, "Odd number of TCP ports");
	AddStatusString(STATUS_SERVICE_LOCAL_IP_ADDRESS, "Network Service local IP address");
	AddStatusString(STATUS_INVALID_REGISTRATION_REFRESH_TIMEOUT, "Invalid registration refresh timeout value");
	AddStatusString(STATUS_ILLEGAL_DNS_SERVERS_ADDRESSES, "Invalid DNS servers addresses");
	AddStatusString(STATUS_PORT_IS_ALREADY_OCCUPIED_BY_ANOTHER_SERVICE, "Failed to assign span to Network Service as the span is assigned to another Network Service.");
	AddStatusString(STATUS_MAX_NUM_OF_PORTS_EXCEEDED, "Maximum number of ports exceeded");
	AddStatusString(STATUS_V35_GATEWAY_IS_ALREADY_CONFIGURED_IN_ANOTHER_SERVICE, "The Network Service dedicated to V35 serial gateway is already configured.");
	AddStatusString(STATUS_V35_SERVICE_IS_NEEDED, "The Network Service dedicated to V35 serial gateway must be configured");
	AddStatusString(STATUS_V35_MUST_BE_DEFINED_IN_THE_FIRST_PORT, "The V35 serial gateway must be mapped to the first port on the RTM LAN card in its dedicated Network Service");
	AddStatusString(STATUS_IPV6_ALREADY_DEFINED_DIFFERENTLY_IN_ANOTHER_SERVICE, "With IPv6 addressing, router definition must be identical in all Network Services");
	AddStatusString(STATUS_IPV4_ALREADY_DEFINED_THE_SAME_IN_ANOTHER_SERVICE, "With IPv4 addressing, router definition can not be identical in all Network Services");
	AddStatusString(STATUS_ROUTER_CANT_BE_CHANGED_IN_SMCU, "STATUS_ROUTER_CANT_BE_CHANGED_IN_SMCU");
	AddStatusString(STATUS_DNS_CANT_BE_CHANGED_IN_SMCU, "STATUS_DNS_CANT_BE_CHANGED_IN_SMCU");
	AddStatusString(STATUS_INTERFACE_IS_NOT_CONFIGURED_IN_THE_SYSTEM, "STATUS_INTERFACE_IS_NOT_CONFIGURED_IN_THE_SYSTEM");
	AddStatusString(STATUS_IP_ADDRESS_IS_DIFFERENT_FROM_THE_ONE_CONFIGURED_IN_THE_SYSTEM, "STATUS_IP_ADDRESS_IS_DIFFERENT_FROM_THE_ONE_CONFIGURED_IN_THE_SYSTEM");
	AddStatusString(STATUS_SUBNET_MASK_CANT_BE_CHANGED_IN_SMCU, "STATUS_SUBNET_MASK_CANT_BE_CHANGED_IN_SMCU");
	AddStatusString(STATUS_ONLY_IPV4_IS_SUPPORTED_IN_SMCU, "STATUS_ONLY_IPV4_IS_SUPPORTED_IN_SMCU");
	AddStatusString(STATUS_PORT_SPEED_CANT_BE_CHANGED_IN_SMCU, "STATUS_PORT_SPEED_CANT_BE_CHANGED_IN_SMCU");
	AddStatusString(STATUS_SHM_IP_ADDRESS_CANT_BE_CHANGED_IN_SMCU, "STATUS_SHM_IP_ADDRESS_CANT_BE_CHANGED_IN_SMCU");
	AddStatusString(STATUS_V35_CANT_BE_CHANGED_IN_SMCU, "STATUS_V35_CANT_BE_CHANGED_IN_SMCU");
	AddStatusString(STATUS_HOST_NAME_CANT_BE_CHANGED_IN_SMCU, "STATUS_HOST_NAME_CANT_BE_CHANGED_IN_SMCU");
	AddStatusString(STATUS_ETHERNET_SETTING_CANT_BE_UPDATED_IN_SMCU, "STATUS_ETHERNET_SETTING_CANT_BE_UPDATED_IN_SMCU");
	AddStatusString(STATUS_UNSUPPORTED_H323_PROTOCOL,"H.323 protocol cannot be selected for the product.");
	AddStatusString(STATUS_ILLEGAL_FIRST_UDP_PORT_VALUE,"First UDP port value is illegal for the product.");
	AddStatusString(STATUS_MUST_ASSIGN_INTERFACE_IN_MFW,"Must assign an network interface.");
	AddStatusString(STATUS_IP_ADDRESS_MISMATCHES_SYSTEM,"The specified IPv4 address doesn't match the specified network interface in system.");
	AddStatusString(STATUS_IPV6_GLOBAL_ADDRESS_MISMATCHES_SYSTEM,"The specified IPv6 Global address is wrong.");
	AddStatusString(STATUS_IPV6_SITE_ADDRESS_MISMATCHES_SYSTEM,"The specified IPv6 Site address is wrong.");
	AddStatusString(STATUS_IPV6_LINK_ADDRESS_MISMATCHES_SYSTEM,"The specified IPv6 Link address is wrong");
	AddStatusString(STATUS_NONE_IPV4_IPV6_ADDRESS,"There is none of IPv4 and IPv6 address in IP Span");
    AddStatusString(STATUS_MANAGEMENT_INTERFACE_CANT_BE_CHANGED_IN_EDGE, "Management interface can't be changed in Edge");
    AddStatusString(STATUS_SIGNALING_AND_MEDIA_MUST_BE_SAME_IN_EDGE, "Signaling and Media interface must be same in Edge");

	AddStatusString(STATUS_NTP_PEER_REJECT,		"NTP PEER rejected");
	AddStatusString(STATUS_NTP_PEER_FALSETICK,	"NTP PEER FALSETICK");
	AddStatusString(STATUS_NTP_PEER_EXCESS,		"NTP PEER exceeded");
	AddStatusString(STATUS_NTP_PEER_OUTLYER,	"NTP PEER OUTLYER");
	AddStatusString(STATUS_NTP_PEER_CANDIDAT,	"NTP PEER candidate");
	AddStatusString(STATUS_NTP_PEER_SELECTED,	"NTP PEER selected");
	AddStatusString(STATUS_NTP_PEER_SYS_PEER,	"NTP PEER SYS PEER");
	AddStatusString(STATUS_NTP_PEER_PPS_PEER,	"NTP PEER SYS PPS PEER");

	AddStatusString(STATUS_SMART_REPORT_ERRORS,	"Smart Report errors");
	AddStatusString(STATUS_SOCKET_WOULD_BLOCKED, "Socket is blocked");
	AddStatusString(STATUS_INTERNAL_COMMUNICATION_PROBLEM, "Internal communication error");
	AddStatusString(STATUS_ILLEGAL_REQUEST, "Invalid request");
	AddStatusString(STATUS_NO_SERVICES, "No Services");

	AddStatusString(STATUS_ILLEGAL_OPERATOR_CONFERENCE_NOT_LOGIN_NAME, "The name of the Operator conference must be the user login name");
	AddStatusString(STATUS_ILLEGAL_OPERATOR_CONFERENCE_ALREADY_EXIST, "An Operator conference with this login name already exists");
	AddStatusString(STATUS_ILLEGAL_OPERATOR_CONFERENCE_WRONG_NUM_OF_PARTIES, "An Operator Conference cannot be created without adding one (and only one) participant");
	AddStatusString(STATUS_ILLEGAL_HOME_CONF_NOT_FOUND, "Failed to move participant to source conference. Conference cannot be found.");
	AddStatusString(STATUS_ILLEGAL_HOME_CONF_EQ, "Move failed. Participants cannot be moved back to Entry Queues.");
	AddStatusString(STATUS_ILLEGAL_MOVE_DEST_CONF_NOT_FOUND, "Move failed. Destination conference cannot be found");
	AddStatusString(STATUS_ILLEGAL_MOVE_PARTY_NOT_FOUND, "Move failed. Participant cannot be found in the source conference");
	AddStatusString(STATUS_ILLEGAL_OPERATOR_CONF_NOT_FOUND, "Cannot assist participant. Operator conference cannot be found.");
	AddStatusString(STATUS_ILLEGAL_DEST_CONF_EQ,"Move failed. Participants cannot be moved to Entry Queues.");
	AddStatusString(STATUS_ILLEGAL_MOVE_TIP_PARTY, "TIP participants cannot be moved between conferences.");

	AddStatusString(STATUS_ILLEGAL_DIAL_OUT_PROTOCOL_H323,"H.323 protocol cannot be selected for dial-out in the Gateway Profile because H.323 Network Service is not configured.");
	AddStatusString(STATUS_ILLEGAL_DIAL_OUT_PROTOCOL_SIP,"SIP protocol cannot be selected for dial-out in the Gateway Profile because SIP Network Service is not configured.");
	AddStatusString(STATUS_ILLEGAL_DIAL_OUT_PROTOCOL_ISDN,"ISDN protocol cannot be selected for dial-out in the Gateway Profile because ISDN Network Service is not configured.");
	AddStatusString(STATUS_ILLEGAL_DIAL_OUT_PROTOCOL_H323_SIP,"H.323 and SIP protocols cannot be selected for dial-out in the Gateway Profile because H.323 and SIP Network Services are not configured.");
	AddStatusString(STATUS_ILLEGAL_DIAL_OUT_PROTOCOL_H323_ISDN,"H.323 and ISDN protocols cannot be selected for dial-out in the Gateway Profile because H.323 and ISDN Network Services are not configured.");
	AddStatusString(STATUS_ILLEGAL_DIAL_OUT_PROTOCOL_SIP_ISDN,"SIP and ISDN protocols cannot be selected for dial-out in the Gateway Profile because SIP and ISDN Network Services are not configured.");
	AddStatusString(STATUS_ILLEGAL_DIAL_OUT_PROTOCOL_H323_SIP_ISDN,"H.323, SIP and ISDN protocols cannot be selected for dial-out in the Gateway Profile because H.323, SIP and ISDN Network Services are not configured.");

//Installer
	AddStatusString(STATUS_INSTALLATION_IN_PROGRESS, "Installation in progress");
	AddStatusString(STATUS_NEW_VERSION_CANNOT_BE_INSTALLED, "New software version cannot be installed on this MCU.  Additional information is contained in the Faults list.");
	AddStatusString(STATUS_NEW_VERSION_CANNOT_BE_INSTALLED_IN_RMX1500, "The version you are attempting to install is not supported by RMX 1500.");
	AddStatusString(STATUS_INSTALLTION_WHILE_DEBUG_MODE, "Notice: New version download while system is in debug mode!");

	AddStatusString(STATUS_BACKUP_IN_PROGRESS, "Backup in progress");
	AddStatusString(STATUS_RESTORE_IN_PROGRESS, "Restore in progress");
	AddStatusString(STATUS_BACK_TO_FACTORY_DEFAULTS_IN_PROGRESS, "Restore Factory Defaults is in progress");

	AddStatusString(STATUS_CFS_MSG_0,	"Activation Key was loaded successfully. Reset the system to activate");
	AddStatusString(STATUS_CFS_MSG_1,	"Invalid Activation Key");
	AddStatusString(STATUS_CFS_MSG_2,	"Activation Key was loaded successfully");
	AddStatusString(STATUS_CFS_MSG_3,	"Current licensed version number is higher than the version number in the Activation Key");
	AddStatusString(STATUS_CFS_MSG_4,	"Version number upgrade completed successfully. The licensed version number is lower than the loaded version number");
	AddStatusString(STATUS_CFS_MSG_5,	"Version number upgrade completed successfully");
	AddStatusString(STATUS_CFS_MSG_6,	"Version is loaded successfully. Reset the system to activate");
	AddStatusString(STATUS_CFS_MSG_7,	"Activation Key and version are loaded successfully. Reset the system to activate");
	AddStatusString(STATUS_CFS_MSG_8,	"Version load failed");
	AddStatusString(STATUS_CFS_MSG_9,	"Activation key loaded successfully but software version load failed");
	AddStatusString(STATUS_CFS_MSG_10,	"Software version loaded. A matching Activation Key is required");
	AddStatusString(STATUS_CFS_MSG_11,	"Activation key loaded successfully but it matches a lower software version number");
	AddStatusString(STATUS_CFS_MSG_12,	"Version download failed");
	AddStatusString(STATUS_CFS_MSG_13,	"The Activation Key entered is invalid.\nThe system is loaded with Software Version: {0}\nPlease use one of the following corrective options:\n1. Upgrade the System Software to the latest version and use the current Activation Key.\n2. Visit the License and Product Activation page at support.polycom.com and generate a new Activation Key to match the Software Version that is loaded on the system.\nThank you for using a Polycom product.");
	AddStatusString(STATUS_PRODUCT_FAMILY_TYPE_MISMATCH,	"Product type mismatch");	// for Call Generator - ValidateProductFamily

	AddStatusString(STATUS_CFS_MSG_0_WARNING_MASKED,	"Activation Key was loaded successfully. Reset the MCU to activate");

	AddStatusString(STATUS_CFS_MSG_2_WARNING_MASKED,	"Activation Key was loaded successfully");
	AddStatusString(STATUS_CFS_MSG_3_WARNING_MASKED,	"Current licensed version number is higher than the version number in the Activation Key");
	AddStatusString(STATUS_CFS_MSG_4_WARNING_MASKED,	"Version number upgrade completed successfully. The licensed version number is lower than the loaded version number");
	AddStatusString(STATUS_CFS_MSG_5_WARNING_MASKED,	"Version number upgrade completed successfully");
	AddStatusString(STATUS_CFS_MSG_6_WARNING_MASKED,	"Version is loaded successfully. Reset the MCU to activate");
	AddStatusString(STATUS_CFS_MSG_7_WARNING_MASKED,	"Activation Key and version are loaded successfully. Reset the MCU to activate");

	AddStatusString(STATUS_CFS_MSG_9_WARNING_MASKED,	"Activation key loaded successfully but software version load failed");
	AddStatusString(STATUS_CFS_MSG_10_WARNING_MASKED,	"Software version loaded. A matching Activation Key is required");
	AddStatusString(STATUS_CFS_MSG_11_WARNING_MASKED,	"Activation key loaded successfully but it matches a lower software version number");

//Logger
	AddStatusString(STATUS_FAILED_TO_FLUSH, "Failed to save log data to file");
	AddStatusString(STATUS_FILE_SYSTEM_DISABLED, "File system disabled");
	AddStatusString(STATUS_INSUFFICIENT_DISK_SPACE, "Insufficient disk space");

//McuMngr
	AddStatusString(STATUS_CFG_PARAM_UNSUPPORTED_LAN_REDUNDANCY_IPV6, "The LAN_REDUNDANCY system flag cannot be set to YES when IPv6 addressing is enabled");

	AddStatusString(STATUS_CFG_PARAM_TRY_ADD_CS_LOGS_TRACE, "Flag cannot be added. Flag cannot be set in System Configuration.");
	AddStatusString(STATUS_BAD_RESTORE_TYPE, "Invalid Restore Factory Defaults type");
	AddStatusString(STATUS_CFG_PARAM_NOT_EXIST, "The new flag cannot be added. Flag is not recognized by the system");
	AddStatusString(STATUS_CFG_PARAM_TYPE_DONT_MATCH, "System flag was added to the incorrect configuration file");
	AddStatusString(STATUS_CFG_PARAM_MISSMATCH_LAN_REDUNDANCY_AND_MULTIPLE_SERVICES, "Conflicting configuration: 'MULTIPLE_SERVICES' and 'LAN_REDUNDANCY' flags cannot be set simultaneously");
	AddStatusString(STATUS_CFG_PARAM_MISSMATCH_LAN_REDUNDANCY_AND_NETWORK_SEPARATION, "Conflicting configuration: 'SEPARATE_MANAGEMENT_NETWORK' and 'LAN_REDUNDANCY' flags cannot be set simultaneously");

	AddStatusString(STATUS_CFG_PARAM_DATA_TYPE_DONT_MATCH, "Invalid flag value");
	AddStatusString(STATUS_CFG_PARAM_SECTION_DONT_MATCH, "Flag cannot be added. Flag added to the wrong System Configuration Section");
	AddStatusString(STATUS_DUPLICATE_ADDRESS_CNTRL, "IP address of the control unit is already defined for another network component");
	AddStatusString(STATUS_DUPLICATE_IPV4_ADDRESS_CNTRL, "IPv4 address of the control unit is already defined for another network component");
	AddStatusString(STATUS_DUPLICATE_IPV6_ADDRESS_CNTRL, "IPv6 address of the control unit is already defined for another network component");
	AddStatusString(STATUS_DUPLICATE_ADDRESS_SHELF, "IP address of the Shelf Management is already defined for another network component");
	AddStatusString(STATUS_FAILED_CFG_IPV6_DEF_GW, "IPv6DefGW config failure");
	AddStatusString(STATUS_CFG_PARAM_UNSUPPORTED_JITC_MODE, "The new flag cannot be added. Flag is unsupported in Enhance Security Mode");
	AddStatusString(STATUS_CFG_PARAM_UNSUPPORTED_MULTIPLE_SERVICE_IPV6, "The MULTIPLE_SERVICE system flag cannot be set to YES when IPv6 addressing is enabled");
	AddStatusString(STATUS_CFG_PARAM_MISSMATCH_NETWORK_SEPARATION_AND_MULTIPLE_SERVICES, "Conflicting configuration: 'SEPARATE_MANAGEMENT_NETWORK' and 'MULTIPLE_SERVICES' flags cannot be set simultaneously");
	AddStatusString(STATUS_CFG_PARAM_MISSMATCH_MULTIPLE_SERVICES_AND_V35_ULTRA_SECURED, "Conflicting configuration: 'MULTIPLE_SERVICES' and 'V35_ULTRA_SECURED_SUPPORT' flags cannot be set simultaneously");
	AddStatusString(STATUS_CFG_PARAM_UNSUPPORTED_MULTIPLE_SERVICE_LICENSE, "Since license, MULTIPLE_SERVICES cannot be enable");
	AddStatusString(STATUS_CFG_PARAM_CANT_REMOVE_ULTRA_SECURED_MODE, "The system is in Ultra Secure Mode, which cannot be disabled unless it is restored to factory defaults");
	AddStatusString(STATUS_CFG_PARAM_UNSUPPORTED_SEPARATE_MANAGEMENT_V35,
	    "The " CFG_KEY_SEPARATE_NETWORK " system flag cannot be set to NO "
	    "when " CFG_KEY_V35_ULTRA_SECURED_SUPPORT " is set to YES");
    AddStatusString(STATUS_IPV6_FORCE_DNS,
        "IPv6 address of the control unit requires configured DNS server");
    AddStatusString(STATUS_IP_SERVICE_DNS_TYPE_DNS,
        "IP type doesn't match the DNS server IP type");

	AddStatusString(STATUS_IPV6_MTU_SIZE_MUST_NO_LESS_THAN_1280,
	    "MTU size must no less than 1280 when IPV6 address is enable");

	AddStatusString(STATUS_CFG_PARAM_NUM_OF_PCM_APPLICABLE_MPMX_ONLY,
	            "Num of PCM is applicable for MPMX only");
    
// RtmIsdnMngr
	AddStatusString(STATUS_NUMBER_OF_ISDN_SERVICES_EXCEEDED, "Maximum number of ISDN/PSTN Network Services reached");
	AddStatusString(STATUS_NUMBER_OF_PHONES_IN_SERVICE_EXCEEDED, "Maximum number of phone numbers defined for the Network Service is reached");
	AddStatusString(STATUS_NUMBER_OF_DIALIN_EXCEEDED, "Maximum number of dial-in numbers reached");
	AddStatusString(STATUS_MCU_PHONE_NUMBER_ALREADY_EXISTS, "MCU phone number already exists");
	AddStatusString(STATUS_NUMBER_OF_ISDN_SPAN_MAPS_EXCEEDED, "Maximum number of ISDN spans connected to this RTM ISDN card has been reached");
	AddStatusString(STATUS_SPAN_MAP_ALREADY_EXISTS, "Span already defined");
	AddStatusString(STATUS_SPAN_MAP_NOT_EXISTS, "Span not defined");
	AddStatusString(STATUS_MISMATCH_SERVICE_NAME_AND_SPAN_ATTACHED, "Network Service Name does not match the assigned span");
	AddStatusString(STATUS_PHONE_RANGE_NOT_EXISTS, "Phone Number Range not defined");
	AddStatusString(STATUS_PHONE_RANGE_ALREADY_EXISTS, "Phone Number Range already defined");
	AddStatusString(STATUS_PHONE_NUMBER_EXISTS_IN_OTHER_SERVICE, "Phone number already defined in another Network Service");
	AddStatusString(STATUS_NUMBER_OF_PHONE_NUMBERS_IN_RANGE_EXCEEDED, "A range cannot exceed 1000 phone numbers");
	AddStatusString(STATUS_PHONE_NUMBER_CONFIGURED_IN_EQ, "Phone number cannot be deleted or modified while it is assigned to any conferencing entity");


	AddStatusString(STATUS_ILLEGAL_PHONE_RANGE, "Invalid Phone Number Range");
	AddStatusString(STATUS_PHONE_NUMBER_ALREADY_EXISTS, "Phone number already exists");
	AddStatusString(STATUS_PHONE_NUMBER_NOT_EXISTS, "Phone number not found");
	AddStatusString(STATUS_MAX_PHONE_NUMBER_EXCEEDED, "Maximum number of phone numbers has been reached");

	AddStatusString(STATUS_CONFERENCE_CREATION_IS_BLOCKED, "Conference Creation is blocked");
	AddStatusString(STATUS_FAIL_TO_UPDATE_GK, "Failed to update gatekeeper changes");
	AddStatusString(STATUS_CONFERENCE_CREATION_IS_BLOCKED_FOR_INVALID_CERTIFICATE,
		"Invalid Certificate, Conference Creation is blocked");

// Recording Link
	AddStatusString(STATUS_RECORDING_LINK_DOES_NOT_EXIST, "Recording Link not found");
	AddStatusString(STATUS_MAX_RECORDING_LINKS_REACHED, "Maximum number of Recording Links has been reached");
	AddStatusString(STATUS_INVALID_RECORDING_LINK_NETWORK_SETTINGS, "Invalid Recording Link network setting");
	AddStatusString(STATUS_CANNOT_DELETE_RECORDING_LINK_WHILE_ASSIGNED, "Cannot delete a Recording Link while it is assigned to a Profile or being used for recording");
	AddStatusString(STATUS_EQ_CANNOT_CONFIG_WITH_RECORDING_LINK, "An Entry Queue cannot be assigned a Profile in which Recording Link is enabled");

// SSL statuses
	AddStatusString(STATUS_CAN_NOT_SECURE_CONNECTION_WITHOUT_CERTIFICATE,
        "The connection cannot be secured because an SSL/TLS certificate cannot be found");
	AddStatusString(STATUS_SSL_CERTIFICATE_DOES_NOT_MATCH_THE_PRIVATE_KEY,
        "SSL/TLS certificate does not match the certificate request");
	AddStatusString(STATUS_CERTIFICATE_FILE_HAS_AN_ERROR,
        "Error in certificate file");
	AddStatusString(STATUS_CERTIFICATE_COMMON_NAME_DIFFER_THAN_RMX_HOST_NAME,
        "Certificate host name does not match the MCU host name");
	AddStatusString(STATUS_CERTIFICATE_ALREADY_EXPIRED,
        "The SSL/TLS certificate has expired");
	AddStatusString(STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST,
        "Cannot create the SSL/TLS certificate request");
	AddStatusString(STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE,
        "Cannot install the SSL/TLS certificate");
	AddStatusString(STATUS_CERTIFICATE_NOT_VALID_YET,
        "The SSL/TLS certificate is not yet valid");
	AddStatusString(STATUS_CERTIFICATE_IS_GOING_TO_BE_EXPIRED,
        "SSL/TLS certificate is about to expire");
	AddStatusString(STATUS_SSL_ILLEGAL_COUNTRY_NAME,
        "Invalid country name");
	AddStatusString(STATUS_SSL_ILLEGAL_STATE_OR_PROVINCE,
        "Invalid state or province name");
	AddStatusString(STATUS_SSL_ILLEGAL_LOCALITY_NAME,
        "Invalid locality name");
	AddStatusString(STATUS_SSL_ILLEGAL_ORGANIZATION_NAME,
        "Invalid organization name");
	AddStatusString(STATUS_SSL_ILLEGAL_ORGANIZATION_UNIT_NAME,
        "Invalid organizational unit name");
	AddStatusString(STATUS_SSL_ILLEGAL_COMMON_NAME,
        "Invalid common name");
	AddStatusString(STATUS_SSL_ILLEGAL_SUBJECT_ALT_NAME,
	        "Invalid subject alternative name");
	AddStatusString(STATUS_CANT_CHANGE_DNS_NAME_IN_SECURED_MODE,
        "DNS name cannot be changed while in Secure Mode");
	AddStatusString(STATUS_CERTIFICATE_IS_EMPTY,
        "Certificate is empty");
	AddStatusString(STATUS_SERIAL_NUMBER_IS_EMPTY,
        "Certificate serial number is empty");
	AddStatusString(STATUS_ISSUER_NAME_IS_EMPTY,
        "Certificate issuer name is empty");
	AddStatusString(STATUS_CERTIFICATE_CTL_FOR_CRL_NOT_EXIST,
        "CRL and the CA certificate must be issued by the same Certificate Authority server");
	AddStatusString(STATUS_CERTIFICATE_ALREADY_EXIST,
        "Certificate already exists on the MCU");
	AddStatusString(STATUS_UNABLE_TO_DELETE_LAST_CERTIFICATE,
        "Last remaining SSL/TLS certificate cannot be deleted while the system is in Ultra Secure Mode");
	AddStatusString(STATUS_CERTIFICATE_IS_NOT_CTL,
        "The certificate being installed must be a CA certificate");
	AddStatusString(STATUS_CERTIFICATE_CTL_NOT_EXIST,
        "The CA certificate is not installed on the MCU");
	AddStatusString(STATUS_CERTIFICATE_OWN_NOT_EXIST,
        "The Machine certificate is not installed on the MCU");
	AddStatusString(STATUS_CERTIFICATE_CRL_NOT_EXIST,
        "The CRL certificate is not installed on the MCU");
	AddStatusString(STATUS_CERTIFICATE_CRL_EXCEEDED_MAX_SIZE,
	    "The CRL certificates exceeded file max size on the MCU");

	AddStatusString(STATUS_NO_START_LINE_CERTIFICATE,
		    "The Certificate does not contain the start line, expect CERTIFICATE ");

	AddStatusString(STATUS_ILLEGAL_CHARS_IN_FLAG, "Flag contains invalid characters");
	AddStatusString(STATUS_RMX_PORT_GAUGE_THRESHOLD_EXCEEDED, "MCU port gauge threshold exceeded");

	AddStatusString(STATUS_COP_LEVELS_IDENTICAL, "Same threshold parameters found on more than one level");
	AddStatusString(STATUS_COP_LEVEL_HIGHER_THAN_PREVIOUS, "Level thresholds must be lower than those of higher levels");
	AddStatusString(STATUS_COP_LEVEL_LOW_BIT_RATE, "Bit rate is not sufficient for the selected video format");
        AddStatusString(STATUS_COP_LEVEL_INVALID_VIDEO_PROTOCOL, "Selected video protocol not sufficient for the first level");
	AddStatusString(STATUS_COP_LEVEL_PARAMS_HIGHER_THAN_MAX_PARAMS, "One of the Event Mode level parameters exceeds the maximun for its level");

	// Failover
	AddStatusString(STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_AND_RESERVATIONS_EXISTS, "Hot Backup can only be enabled after all ongoing conferences and reservations have been terminated or deleted.");
	AddStatusString(STATUS_ONGOING_CONFERENCE_AND_RESERVATIONS_EXISTS,"All ongoing conferences and reservations will be deleted.");

	// ExchangeModule
	AddStatusString(STATUS_EXCHANGE_BAD_PARAMETERS,"Invalid parameters");
	AddStatusString(STATUS_EXCHANGE_USERNAME_EMPTY,"User Name field is empty");
	AddStatusString(STATUS_EXCHANGE_PASSWORD_EMPTY,"Password field is empty");
	AddStatusString(STATUS_EXCHANGE_WEB_URL_EMPTY,"Exchange server URL field is empty");
	AddStatusString(STATUS_EXCHANGE_WEB_URL_ILLEGAL,"Invalid Exchange server URL");
	AddStatusString(STATUS_EXCHANGE_FAILED_CONNECTING_TO_EXCHANGE,"Failed to connect to the Exchange Server");
	AddStatusString(STATUS_EXCHANGE_DOMAIN_EMPTY,"Domain name field is empty");

	AddStatusString(STATUS_EMPTY_USER_NAME_IN_ICE_ENVIRONMENT,"The MCU Server Name for the ICE Environment was not entered");

	// LdapModule
	AddStatusString(STATUS_DIRECTORY_TYPE_INCORRECT,"Invalid directory type");
	AddStatusString(STATUS_DIRECTORY_NAME_ADDRESS_EMPTY,"Directory name/address is empty");
	AddStatusString(STATUS_DIRECTORY_PORT_INCORRECT,"Invalid directory port");
	AddStatusString(STATUS_BASE_DN_EMPTY,"Base DN is empty");
	AddStatusString(STATUS_BASE_DN_INCORRECT,"Invalid Base DN");
	AddStatusString(STATUS_AUTHENTICATION_TYPE_INCORRECT,"Invalid authentication type");
	AddStatusString(STATUS_ALL_DIR_AUTHORIZATION_GROUPS_ARE_EMPTY,"All relevant authorization groups are empty");
	AddStatusString(STATUS_LDAP_CONFIGURATION_UPDATE_FAILED,"Directory services configuration update has failed");
	AddStatusString(STATUS_ACTIVE_DIRECTORY_SERVER_IS_NOT_AVAILABLE,"Active directory server is not available");
	AddStatusString(STATUS_ACCOUNT_AUTHORIZATION_LEVEL_NOT_ALLOWED_LDAP,"There is a user (in MCU Users list) with an authorization level that is not allowed in Ultra Secure Mode when connecting to Active Directory (only Administrator is allowed)");
	AddStatusString(STATUS_ONLY_ONE_ACCOUNT_IS_ALLOWED_IN_JITC_LDAP,"Only one user (in MCU Users list) is allowed in Ultra Secure Mode when connecting to Active Directory (authorization level: Administrator)");
	AddStatusString(STATUS_INVALID_SECURE_MODE_IN_JITC_LDAP,"Invalid secure mode in Ultra Secure Mode when connecting to Active Directory");

	AddStatusString(STATUS_1500Q_NO_CP_HD_LICENSE, "No HD License installed. An HD license is required to enable HD resolution.");

	AddStatusString(STATUS_COLLECTING_ALREADY_IN_PROGRESS, "Collector is already in progress");
	AddStatusString(STATUS_COLLECTOR_IS_NOT_ACTIVE_AT_THE_MOMENT, "Collector is not active at the moment");
	AddStatusString(STATUS_ABORT_COLLECTOR_ALREADY_STARTED, "Abort Collector has already started");

	// NotificationManager
	AddStatusString(STATUS_NOTIF_INTERNAL_ERROR,"Notification Manager internal error");
	AddStatusString(STATUS_NOTIF_ID_GENERATION_FAILED,"ID generation failed");
	AddStatusString(STATUS_NOTIF_SUBSCRIPTION_NOT_FOUND,"Subscription not found");
	AddStatusString(STATUS_NOTIF_RESOURCE_NOT_SUPPORTED,"Resource type is not supported");
	AddStatusString(STATUS_NOTIF_MAXIMAL_SUBSCRIPTIONS_NUMBER_REACHED,"Maximal subscriptions number reached");

	// MediaRelay Conf Type
	AddStatusString(STATUS_ILLEGAL_DEST_CONF_MEDIA_RELAY, "Participant cannot be moved between conferences");
	AddStatusString(STATUS_ILLEGAL_H320_PARTY_IN_MEDIA_RELAY_CONF,"Cannot connect H320 party to Media-Relay conference");
	AddStatusString(STATUS_CANNOT_ADD_DIAL_OUT_PARTY_TO_MEDIA_RELAY_CONF,"Cannot connect Dial-Out party to Media-Relay conference");
	AddStatusString(STATUS_CANNOT_ADD_H323_PARTY_TO_MEDIA_RELAY_CONF,"Cannot connect H323 party to Media-Relay conference");
	AddStatusString(STATUS_SVC_CONFERENCE_NOT_SUPPORTED_BY_LICENSING,"The conference cannot be created due to invalid or expired license");
	

	// TIP Compatibility mode
	AddStatusString(STATUS_TIP_COMPATIBILITY_NOT_SUPPORTED_IN_VSW, "TIP Compatibility mode is not supported in VSW conferences");
	AddStatusString(STATUS_GATHERING_NOT_SUPPORTED_IN_TIP_COMPATIBILITY_MODE,"Gathering is not supported in TIP Compatibility mode");
	AddStatusString(STATUS_MESSAGE_OVERLAY_NOT_SUPPORTED_IN_TIP_COMPATIBILITY_MODE, "Message Overlay is not supported in TIP Compatibility mode");
	AddStatusString(STATUS_SITE_NAMES_NOT_SUPPORTED_IN_TIP_COMPATIBILITY_MODE, "Site Names are not supported in TIP Compatibility mode");
	AddStatusString(STATUS_LEGACY_CONTENT_NOT_SUPPORTED_IN_TIP_COMPATIBILITY_MODE, "Send Content to Legacy Endpoints is not supported in TIP Compatibility mode");
	AddStatusString(STATUS_TIP_COMPATIBILITY_NOT_SUPPORTED_BY_LICENSING, "The conference cannot be created due to incompatible license");
	AddStatusString(STATUS_INVALIDE_OR_EXPIRED_LICENSING, "The conference cannot be created due to invalid or expired license");
	AddStatusString(STATUS_INVALID_MFW_SNMP_CONFIGURATION, "Invalid SNMP Configuration");

	

	//Content transcoding
	AddStatusString(STATUS_CONTENT_XCODE_NOT_SUPPORTED, "Multiple Resolution (transcoding) feature is not supported.");
	AddStatusString(STATUS_VSW_CONTENT_XCODE_CONFLICT, "VSW and Multiple Resolution (transcoding) cound NOT be enabled together.");
	AddStatusString(STATUS_CONTENT_XCODE_ONLY_SUPPORTED_FOR_CP, "Multiple Resolution (transcoding) is only supported in CP mode for 1st phase.");
	
	//CDR
	AddStatusString(STATUS_FAILED_REGISTER_TO_CDR_SERVER, "Failed to register to CDR Server.");
	AddStatusString(STATUS_AT_LEAST_ONE_CDR_SERVICE, "At least one CDR service must be chosen.");

	//Manage layout internally
	AddStatusString(STATUS_PERSONAL_LAYOUT_IS_MANAGED_INTERNALLY, "Personal layout is managed internally in telepresence room switch mode.");
	AddStatusString(STATUS_CONFERENCE_LAYOUT_IS_MANAGED_INTERNALLY, "Conference layout is managed internally in telepresence room switch mode.");

	AddStatusString(STATUS_UNEXPECTED_VIDEO_PROTOCOL, "Unexpected video protocol.");
}

const std::string& CStatusStringConverter::GetStringByStatus(STATUS status)
{
  return GetStr(status);
}

void CStatusStringConverter::AddStatusString(STATUS status,
                                             const std::string& str)
{
  Add(status, RebrandString(str));
}

void CStatusStringConverter::SerializeApiStatuses(CXMLDOMElement* pLanguageNode)
{
  for (std::map<STATUS, std::string>::const_iterator ii = begin(); end() != ii; ++ii)
  {
    STATUS status = ii->first;
    if (status < 72000)
    {
      char strKey[7];
      snprintf(strKey, sizeof(strKey), "%d", status);
      AddStatusNode(pLanguageNode, strKey, ii->second);
    }
  }

  //Add EMA statuses
  AddStatusNode(pLanguageNode, "-4"    , "Terminated");
  AddStatusNode(pLanguageNode, "-3"    , "Failed to connect to the MCU");
  AddStatusNode(pLanguageNode, "-2"    , "Disconnected");
  AddStatusNode(pLanguageNode, "65536" , "Expired");
  AddStatusNode(pLanguageNode, "-5"    , "Communication disconnected");
  AddStatusNode(pLanguageNode, "-6"    , "Redirected to different site due to change in IP configuration");
  AddStatusNode(pLanguageNode, "-7"    , "MCU Web Client session has timed out due to no user activity. Please login again");
  AddStatusNode(pLanguageNode, "-8"    , "Connection authentication has failed");
  AddStatusNode(pLanguageNode, "-9"    , "EMA Shelf Failed to Logout");
  AddStatusNode(pLanguageNode, "-10"   , "EMA Shelf Get State Error");
  AddStatusNode(pLanguageNode, "-11"   , "EMA Shelf connection list failed");
  AddStatusNode(pLanguageNode, "-12"   , "EMA Shelf Clear connection list failed");
  AddStatusNode(pLanguageNode, "-13"   , "EMA Shelf Remove connection failed");
  AddStatusNode(pLanguageNode, "-14"   , "EMA Shelf Connection Id invalid");
}

void CStatusStringConverter::AddStatusNode(CXMLDOMElement *pLanguageNode,
                                           char strKey[7],
                                           const std::string& value)
{
	CXMLDOMElement* pStringNode = pLanguageNode->AddChildNode("String");

	CXMLDOMAttribute* pKey_Attribute;
	CXMLDOMAttribute* pValue_Attribute;

	pKey_Attribute = new CXMLDOMAttribute;

	pKey_Attribute->set_nodeName("Key");
	pKey_Attribute->SetValueForElement(strKey);
	pStringNode->AddAttribute(pKey_Attribute);

	pValue_Attribute = new CXMLDOMAttribute;
	pValue_Attribute->set_nodeName("Value");
	pValue_Attribute->SetValueForElement(value.c_str());
	pStringNode->AddAttribute(pValue_Attribute);
}
