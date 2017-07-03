//+========================================================================+
//                            StringsMaps.h                                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       StringsMaps.h                                               |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Anatoly                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#ifndef __STRINGSMAPS__
#define __STRINGSMAPS__


#include "DataTypes.h"
#include "Transactions.h"

#define MAX_NUMBER_OF_ITEMS						10000


//=====================================
// ConfParty StringsMaps
//=====================================
#define CHANGE_STATUS_ENUM						500
#define _1_TO_H243_NAME_LENGTH				    501
#define _0_TO_H243_NAME_LENGTH					502
#define VIDEO_SESSION_ENUM						503
#define PHONE_NUMBER_DIGITS_LENGTH				504
#define MEDIA_ENUM						        505
#define REMARK_LENGTH						    506
#define VIDEO_PROTOCOL_ENUM						507
#define AUDIO_RATE_ENUM							508
#define VIDEO_FORMAT_ENUM		                509
#define FRAME_RATE_ENUM				            510
#define INTERFACE_ENUM					        511
#define CONNECTION_ENUM						    512
#define END_TIME_ALERT_TONE_ENUM				513
#define SERVICE_PHONE_PREFIX_LENGTH				514
#define DUAL_VIDEO_MODE_ENUM	                515
#define ADVANCED_MEDIA_ENUM			            516
#define HSD_RATE_ENUM					        517
#define CONF_CONTROL_ENUM					    518
#define EXTERNAL_MASTER_ENUM					519
#define VIDEO_QUALITY_ENUM						520
#define CALL_CONTENT_ENUM			            521
#define PARTY_CHANGE_STATE_ENUM			        522
#define ONGOING_PARTY_STATUS_ENUM				523
#define DISCONNECTION_CAUSE_ENUM			    524
#define NETWORK_ENUM							525
#define MAX_PARTIES_ENUM			            526
#define NET_CHANNEL_NUMBER_ENUM		            527
#define TRANSFER_RATE_ENUM				        528
#define FORCE_STATE_ENUM					    529
#define LAYOUT_ENUM								530
#define VIDEO_BIT_RATE_ENUM		                531
#define LAYOUT_TYPE_ENUM			            532
#define RECORDING_PORT_ENUM					    533
#define CASCADE_ROLE_ENUM						534
#define _0_TO_CONFERENCE_ENTRY_PASSWORD_LENGTH  535
#define _0_TO_NUMERIC_CONFERENCE_ID_LENGTH      536
#define LIMITED_SEQ_ENUM                        537
#define MR_STATE_ENUM                           538
#define MEET_ME_METHOD_ENUM						539
#define BOOL_AUTO_ENUM                          540
#define _1_TO_SIZE_OF_CALL_ID_LENGTH			541
#define ENTERPRISE_MODE_ENUM					542
#define CONFERENCE_TYPE_ENUM					543
#define ENTRY_QUEUE_TYPE_ENUM					544
#define TELEPRESENCE_PARTY_TYPE_ENUM            545
#define HD_RESOLUTION_ENUM						546
#define MONTHLY_PATTERN_ENUM					547
#define REPEATED_TYPE_ENUM						548
#define INSTANCE_ENUM							549
#define SIGN_TYPE								550
#define RESERVATION_STATUS_ENUM					551
#define ALLOCATION_MODE_ENUM					552
#define PRESENTATION_PROTOCOL_ENUM				553
#define IP_SUB_SERVICE_ENUM						554
#define LAST_QUIT_TYPE_ENUM						555

#define FIPS140_SIMULATE_CARD_PROCESS_ENUM				556
#define FIPS140_SIMULATE_ENCRYPTION_PROCESS_ENUM 			557
#define FIPS140_SIMULATE_CONFPARTY_PROCESS_ENUM				558
#define TELEPRESENCE_LAYOUT_MODE_ENUM           559


#define ATTENDED_MODE_ENUM						560

#define	_0_TO_AV_MSG_SERVICE_NAME_LENGTH		561
#define	_1_TO_AV_MSG_SERVICE_NAME_LENGTH		562

#define AV_MSG_LENGTH							563
/// Moved to InitCommonStrings.h
///#define _0_TO_NEW_FILE_NAME_LENGTH			564
///#define _1_TO_NEW_FILE_NAME_LENGTH			565

#define TIP_COMPATIBILITY_ENUM					564
#define TIP_PARTY_TYPE_ENUM						565

#define DTMF_OPCODE_ENUM						566
#define DTMF_PERMISSION_ENUM					567
#define IVR_FEATURE_OPCODE_ENUM					568
#define IVR_EVENT_TYPE_ENUM						569

#define _0_TO_IVR_MSG_NAME_LENGTH				570

#define _1_TO_LANGUAGE_NAME_LENGTH				571
#define _0_TO_DTMF_STRING_LENGTH				572

#define IVR_EXTERNAL_SERVER_ENUM				573
#define IVR_REQUEST_PWD_ENUM					574

#define DELIMITERS_ENUM							575

#define LECTURE_MODE_TYPE_ENUM					576

/// Maybe "Private Name" should be changed since it is not a relevant name (???)
#define _0_TO_MAX_PRIVATE_NAME_LENGTH			577

// IP monitoring and secondary
#define MAP_PROBLEM_ENUM						578
#define CAP_CODE_ENUM							579
#define IP_CHANNEL_TYPE_ENUM					580
#define ANNEXES_ENUM							581
#define VIDEO_RESOLUTION_ENUM					582
#define SECONDARY_PROBLEM_ENUM					583
#define SIP_CONFERENCING_LIMITATION_ENUM		584
#define	IDENT_METHOD_ENUM		                585
#define GK_STATE_ENUM							586
#define INTRA_REQUEST_ENUM                    	587
#define ICE_CONNECTION_TYPE_ENUM               	588
#define ATTENDING_STATE_ENUM					590

#define _0_TO_MAX_FULL_PATH_LENGTH				591
#define _0_TO_USER_IDENTIFIER_STRING_LENGTH     592
#define _0_TO_APPOINTMENT_ID_LENGTH				593

#define LANGUAGES_ENUM							594

#define ASPECT_RATIO_ENUM						595
#define COP_VIDEO_FORMAT_ENUM					596
#define COP_VIDEO_FRAME_RATE_ENUM				597
#define AUTO_SCAN_INTERVAL_ENUM					598
#define TELEPRESENCE_MODE_CONFIGURATION_ENUM    599

// More ConfParty Opcodes afterwards
//=====================================
// CsMngr StringsMaps
//=====================================
#define DUMMY_ENUM								600
//#define NET_SERVICE_PROVIDER_NAME_LENGTH		601
#define GATEKEEPER_ENUM						    602
#define GATE_KEEPER_MODE_ENUM					603
#define PROTOCOL_TYPE_ENUM						604
#define SPEED_MODE_ENUM							605
#define QOS_ACTION_ENUM							606
#define QOS_DIFF_SERV_ENUM						607
#define QOS_TOS_ENUM							608
#define REMOTE_FLAG_ENUM						609
#define SERVER_STATUS_ENUM						610
#define _0_TO_5_DECIMAL							611
#define _QOS_IP_VIDEO_RANGE_DECIMAL	   			612
#define _QOS_IP_AUDIO_RANGE_DECIMAL	   			613
#define IP_SERVICE_TYPE_ENUM					614
#define IP_SERVICE_STATE_ENUM					615
#define SERVICE_ROLE_ENUM   					616
#define  SIP_SERVICE_STATUS_ENUM  				617
#define  PING_STATUS_ENUM  			         	618
#define  PING_IP_TYPE_ENUM  	    			619
#define ICE_ENVIRONMENT_ENUM					620
#define SIP_SERVER_TYPE_ENUM					621
#define SIP_REGISTRATION_STATUS_ENUM			622
#define SIP_REGISTRATIONS_TOTAL_STATUS_ENUM		623//sipProxySts
#define AUTHENTICATION_PROTOCOL_ENUM			624

//=====================================
// Cards StringsMaps
//=====================================
#define BOARD_ENUM								650
#define SUB_BOARD_ENUM				            651
#define UNIT_ENUM						        652
#define CARD_TYPE_ENUM						    653
#define CARD_STATE_ENUM						    654
#define UNIT_STATUS_ENUM						655
#define MEDIA_IP_CONFIG_STATUS_ENUM				656
#define UNIT_TYPE_ENUM                          657



#define AV_MCU_CASCADE_MODE_ENUM                658






//=====================================
// McuMngr StringsMaps
//=====================================
#define CFG_TYPE_ENUM 							700
#define MCU_STATE_ENUM							701
#define LICENSING_VALIDATION_ENUM				702
#define NTP_SERVER_STATUS_ENUM					703
#define MCU_RESTORE_TYPE_ENUM					704
#define _0_TO_MPL_SERIAL_NUM_LENGTH				705
#define _0_TO_MAC_ADDRESS_CONFIG_LENGTH         706
#define ETHERNET_PORT_TYPE_ENUM         		707
#define INSTALL_PHASE_TYPE_ENUM                         708
#define INSTALL_PHASE_STATUS_ENUM                       709
#define AUTHENTICATION_PROTOCOL_802_1X_ENUM             710
#define LICENSING_CONNECTION_STATUS_ENUM				711
#define LICENSING_STATUS_ENUM				            712


//=====================================
// Logger StringsMaps
//=====================================
#define TRACE_LEVEL_ENUM 						750
#define EMA_PROCESSES_ENUM 						751

#define _0_TO_MAX_OPCODE_NAME_LENGTH		    752
#define _0_TO_MAX_CONTENT_LENGTH		        753
#define _0_TO_MAX_TASK_NAME_LENGTH		        754
#define _0_TO_MAX_OBJECT_NAME_LENGTH		    755
#define _1_TO_LOGGER_FILE_MAX_NAME_LEN			756

//=====================================
// Resource StringsMaps
//=====================================
#define COP_PORT_TYPE_ENUM   				    800

//=====================================
// TcpDump StringsMaps
//=====================================

#define ENTITY_TYPE_ENUM   				    850
#define MAX_CAPTURE_SIZE_ENUM               851
#define MAX_CAPTURE_DURATION_ENUM           852
#define TCP_DUMP_STATE_ENUM                 853

//=====================================
// EndpointsSim StringsMaps
//=====================================
#define DTMF_SOURCE_TYPE_ENUM					900
#define VIDEO_MODE_H264_ENUM					901
#define SIM_VIDEO_H264_ASPECT_RATIO_LIMITS		        902
#define SIM_RECAP_MODE_ENUM                                     903




//=====================================
// SNMP StringsMaps
//=====================================
#define COMMUNITY_PERMISSION_ENUM				950
#define SNMP_VER_ENUM						    951
#define SNMP_AUTH_PROTOCOL					    952
#define SNMP_PRIV_PROTOCOL					    953
#define SNMP_SECURITY_LEVEL					    954

//=====================================
// RtmIsdnMngr StringsMaps
//=====================================
#define SPAN_TYPE_ENUM							1000
#define ISDN_SERVICE_TYPE_ENUM					1001
#define E1_FRAMING_ENUM							1002
#define E1_LINE_CODING_ENUM						1003
#define T1_FRAMING_ENUM							1004
#define T1_LINE_CODING_ENUM						1005
#define LINE_LENGTH_ENUM						1006
#define SWITCH_TYPE_ENUM						1007
#define SIDE_ENUM								1008
#define CALL_TYPE_ENUM							1009
#define THRESHOLD_ENUM							1010
#define PREFER_MODE_ENUM						1011
#define ALLOC_METHOD_ENUM						1012
#define NUM_PLAN_ENUM							1013
#define DFLT_NUM_TYPE_ENUM						1014
#define SPAN_ALARM_ENUM							1015
#define SPAN_D_CHANNEL_STATE_ENUM				1016
#define SPAN_CLOCKING_ENUM						1017
#define NET_SPEC_FACILITY_ENUM					1018


//=====================================
// Auditor StringsMaps
//=====================================
#define AUDIT_STATUS_GROUP_ENUM                 1050
#define AUDIT_TYPE_GROUP_ENUM                   1051
#define AUDIT_DATA_TYPE_GROUP_ENUM              1052


//=====================================
// Installer StringsMaps
//=====================================
#define VERSION_TYPE_ENUM						1100


//=====================================
// Message Overlay StringsMaps
//=====================================
#define MESSAGE_OVERLAY_DISPALY_POSITION_TYPE_ENUM   1200
#define MESSAGE_OVERLAY_FONT_TYPE_ENUM   1201
#define MESSAGE_OVERLAY_SPEED_TYPE_ENUM 1202
#define MESSAGE_OVERLAY_COLOR_TYPE_ENUM 1203
#define _0_TO_MESSAGE_OVERLAY_TEXT_LENGTH 1204
#define _0_TO_MESSAGE_OVERLAY_NUM_OF_REPETITIONS 1205
#define _0_TO_MESSAGE_OVERLAY_FONT_SIZE 1206
#define _0_TO_MESSAGE_OVERLAY_DISPLAY_POSITION 1207
#define _0_TO_MESSAGE_OVERLAY_TRANSPARENCE 1208


#define RESOLUTION_THRESHOLD_RATE_ENUM			1210
#define RESOLUTION_THRESHOLD_TYPE_ENUM			1211
#define RESOLUTION_CONFIG_TYPE_ENUM			1212

//=====================================
// Failover StringsMaps
//=====================================
#define FAILOVER_STATUS_TYPE					1150
#define FAILOVER_TYPE							1151
#define FAILOVER_MASTER_SLAVE_STATE				1152
#define FAILOVER_EVENT_TRIGGER					1153

//=====================================
// ApacheModule StringsMaps
//=====================================
#define DISCONNECT_REASON_ENUM						1250

//=====================================
// ConfParty  StringsMaps Cont.
//=====================================

#define ITP_CROPPING_ENUM                           1300

#define SIP_REG_STATUS_ENUM							1301
#define SIP_REG_TOTAL_STATUS_ENUM					1302
#define H264_VSW_HIGH_PROFILE_PREFERENCE_ENUM		1303

#define LOG4CXX_LEVEL_ENUM 						1304
#define RMX_ALL_PROCESSES_ENUM					1305
#define UDP_SEND_DURATION_ENUM					1306
#define INTERFACE_TYPE_ENUM						1307
#define ENCRYPTION_TYPE_ENUM                    1308
#define CASCADE_OPTIMIZE_RESOLUTION_ENUM		1309
#define _0_TO_MAXKAVALUE						1310 /* The max Keepalive value is 1 day (in seconds) which is 86400*/

#define FONT_TYPES_ENUM                         1311
#define SPEAKER_CHANGE_THRESHOLD_ENUM           1312
#define CONF_MEDIA_TYPE_ENUM                    1313
#define PARTY_MEDIA_TYPE_ENUM                   1314
#define LINK_TYPE_ENUM                          1315
#define RELAY_CODEC_TYPE_ENUM                   1316
#define OPERATION_POINTS_PRESET_ENUM            1317
#define SIP_ENCRYPTION_KEY_EXCHANGE_MODE_ENUM   1318
#define SRS_PLAYBACK_LAYOUT_MODE_ENUM      1319
#define IVR_SLIDE_CONVERSION_METHOD_ENUM        1320
#define IVR_SLIDE_IMAGE_TYPE_ENUM               1321
//=====================================
// Collector StringsMaps
//=====================================

#define COLLECTING_TYPE_ENUM						1400
#define SITE_NAME_DISPLAY_MODE_ENUM				1401
#define SITE_NAME_DISPLAY_POSITION_ENUM			1402
#define _0_TO_SITE_NAME_FONT_SIZE 				1404
#define _0_TO_SITE_NAME_TRANCEPARENCE			1405
#define _0_TO_SITE_NAME_CUSTOM_POSITION			1406
#define	TEXT_COLOR_TYPE_ENUM						1407

////////////////////////////
#define REVOCATION_METHOD_ENUM					1430

//=====================================
// ConfParty StringsMaps 
//=====================================
//Indication for audio participants
#define ICON_DISPLAY_POSITION_ENUM				1440
#define ICON_DISPLAY_MODE_ENUM					1441


/////////////////////////////////////////////////////////////////////////
class CItem
{
public:
	CItem(int value,const char *pDescription);
	CItem* GetNextItem();
	int GetValue();
	const char *GetDescription();
	int GetDescriptionLength();
	void SetNextItem(CItem *pItem);
	void DeleteList();

private:
	CItem *m_pNextItem;
	int m_Value;
	const char *m_pDescription;
	int m_DesctiptionLen;

	DWORD dummy1;
	DWORD dummy2;
	DWORD dummy3;


};


/////////////////////////////////////////////////////////////////////////
class CStringsMaps
{
public:
	static void AddItem(int type,int value,const char *strDescription);
	static void AddMinMaxItem(int type,int min,int max);
	static void Build();
	static BYTE GetDescription(int type,int value,const char **strDescription);
	static BYTE GetValue(int type,int &value,char *strDescription);
	static void CleanUp();
	static BOOL IsDefineType(int type);
    static BOOL CheckExistance(const int type, const int value);

private:

	static CItem ** m_pItemsArray;

	static WORD m_bInit;
};
////////////////////////////////////////////////////////////////////////////



#endif /* __STRINGSMAPS__ */
