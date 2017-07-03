
#ifndef _XML_API__
#define _XML_API__

class CXMLDOMElement;
class CXMLDOMDocument;

enum eXmlActionNodeType
{
	ACTION_GROUP = 0,
	ACTION_ELEMENT
};


extern CXMLDOMElement *BuildGeneralResponse(STATUS status,
										 const char *pStatusDescription,
									 	 const char *pStatusDescriptionEx,
										 DWORD MessageId,
										 WORD action,
										 DWORD userToken1,
										 DWORD userToken2);

//const char *GetStatusString(STATUS status);

extern BYTE ParseXMLFile(const char* szFileName, CXMLDOMDocument *pXMLDocument);


//Objects types
#define GATEWAY_CONFIGURATION_DELIMITERS_LIST          4
#define GATEWAY_CONFIGURATION_PHONE_RANGES_LIST        5
#define GATEWAY_CONFIGURATION_SERVICE_LIST             6
#define GATEWAY_CONFIGURATION_SPEED_DIAL_LIST          7
#define GATEWAY                                        8
#define GATEWAY_LIST                                   9
#define RESERVATIONS_LIST                              10
#define CONFERENCES_LIST                               11
#define CARDS_LIST                                     12
#define RESERVATION                                    13
#define CONFERENCE                                     14
#define PHONES_LIST                                    15
#define PHONE                                          16
#define MCU_STATE                                      17
#define CARD                                           19
#define TDM_LINE                                       20
#define TRACE_INFO                                     21
#define FAULTS_INFO                                    22
#define CALL_RECORD_INFO                               23
#define HLOGS_LIST                                     24
#define OPERATORS                                      25
#define OPERATORS_LIST                                 26
#define CONNECTIONS_LIST                               27
#define CONF_RES                                       28
#define SERV_PROV_LIST                                 29
#define SERVICE_PROVIDER                               30
#define SPANS_LIST                                     31
#define SPAN                                           32                      
//#define FAULTS_LIST	     33//must be removed     (replaced by HLOGS_LIST)
#define RSRC_REPORT_LIST                               34
#define PHONE_NUMBER                                   35
#define LAN_CONFIG                                     36
#define RESET_MCPU                                     37
#define MCU_TIME                                       38
#define RSRC_DETAIL_LIST                               39
#define CDR_LIST                                       40
#define FILE_UPDATED                                   41
#define CDR_DETAIL_REQUEST                             42
#define AV_MSG_SERV_LIST                               43
#define AV_MSG_SERVICE                                 44
#define FILE_LIST                                      45
#define ATTENDED_QUEUE_LIST                            46
#define ATM_SERVICE_LIST                               47
#define ATM_SERVICE                                    48
#define ATM_SPAN                                       49
#define ATM_PHONE_NUMBER                               50
#define H323_SERVICE_LIST                              51
#define H323_SERVICE                                   52
#define H323_SPAN                                      53
#define CONF_RES_FULL                                  54
#define MCU_MEMORY_STATE                               55
#define PERFORMANCE_MONITORING                         56
#define CASCADE_LIST                                   57
#define V35_SERVICE_LIST                               58
#define V35_SERVICE                                    59
#define V35_SPAN                                       60
#define PARTY_SPECIFIC                                 61
#define MEETING_ROOMS_LIST                             62
#define MEETING_ROOM                                   63
#define SNMP                                           64
#define GATEWAY_CONFIG                                 65
#define IVR_SERV_LIST                                  66                      
#define GATEWAY_CONFIGURATION_320_PROFILE_LIST         67
#define GATEWAY_CONFIGURATION_323_PROFILE_LIST         68
#define GATEWAY_GENERAL_INFO_LIST                      69
#define GATEWAY_CONFIGURATION_BASIC_PHONE_RANGES_LIST  70
#define DOUBLE_GATEWAY_REMOTE_GATEWAY_DEFINITION_LIST  71
#define DOUBLE_GATEWAY_LINKS_LIST                      72
#define IVR2_MSG_LIST                                  73
#define FILE_LIST_RECURSIVE                            74
#define PARTIES_USED_IN_PERIOD_LIST                    75
#define ARROW_CONFIGURATION_PARAMS                     76
#define ARROW_RSRC_REPORT_LIST                         77
#define CONFIGURATION_UPDATE                           78
#define TEMPLATES_LIST                                 79
#define TEMPLATE                                       80
#define PARTY_TEMPLATES_LIST                           81
#define T1CAS_SERVICE_LIST                             82
#define T1CAS_SERVICE                                  83
#define T1CAS_SPAN                                     84
#define RECORDING_LINKS_LIST                           85
#define RECORDING_LINK                                 86
#define CDR_DETAIL_XML_FORMATTED_REQUEST               87
#define CONFERENCE_PARTIAL							   88
#define RECORDING_LIST_ONGOING						   89
#define PLAYBACK_LIST_ONGOING						   90
#define RECORDING_CONFIGURATION_PARAMS				   91
#define USER_RECORDING_DIRECTORY					   92
#define USER_RECORDING_FILE							   93
#define CONF_RECORDER_USER							   94
#define CONF_RECORDER_USERS_LIST					   95
#define AUTHORIZATIONS_LIST							   96
#define BACKUP_APP_MSG								   97



#endif  // _XML_API__

