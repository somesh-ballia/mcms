// InitCommonStrings.h:

#ifndef _INITCOMMONSTRINGS_H
#define _INITCOMMONSTRINGS_H

void InitCommonStrings();


// VALUES MUST BE BETWEEN 0-9999

#define IP_STRING_LENGTH					1
#define	DESCRIPTION_LENGTH					2
#define ONE_LINE_BUFFER_LENGTH				3
#define FIFTY_LINE_BUFFER_LENGTH			4
#define _BOOL								5
#define _0_TO_WORD							6
#define _0_TO_BYTE							7
#define _0_TO_DWORD							8
#define DATE_TIME							9
#define ACTION_TYPE_ENUM					10
#define IP_TYPE_ENUM						11
#define IP_ADDRESS							12	// IpV4
#define _1_TO_IP_ADDRESS_LENGTH				13
#define IP_V6_ADDRESS						14	// IpV6
#define _0_TO_IPV6_ADDRESS_LENGTH		    15
#define IP_V6_CONFIG_TYPE_ENUM				16
#define IP_V6_ADDRESS_SCOPE_ENUM			17
#define AUTHORIZATION_GROUP_ENUM			18
#define PRODUCT_TYPE_ENUM					19
#define OPERATING_SYSTEM_ENUM				20
#define ENTRY_QUEUE_ROUTING_ENUM			21
#define _0_TO_PRIVATE_VERSION_DESC_LENGTH	22
#define _1_TO_OPERATOR_NAME_LENGTH			23
#define _0_TO_OPERATOR_NAME_LENGTH			24
#define SIP_ADDRESS_TYPE_ENUM				25
#define _1_TO_DWORD	   				        26
#define _YES_NO						        27
#define SYSTEM_CARDS_MODE_ENUM				28
#define SYSTEM_RAM_SIZE_ENUM				29




#define _5_TO_999_DECIMAL                    30
#define _0_TO_99_DECIMAL                     31
#define _0_TO_59_DECIMAL                     32
#define _1_TO_60_DECIMAL                     33
#define _0_TO_60_DECIMAL                     34
#define _0_TO_30_DECIMAL                     35
#define _1_TO_5_DECIMAL                      36
#define _100_TO_1000_DECIMAL                 37
#define _0_TO_10_DECIMAL                     38
#define _0_TO_3600_DECIMAL                   39
#define _0_TO_100000_DECIMAL                 40
#define _0_TO_3000_DECIMAL                   41
#define _1_TO_31_DECIMAL                     42
#define _0_TO_23_DECIMAL		             43
#define CONF_ID				                 44
#define CONF_CP_RESOLUTION_ENUM		         45
#define _0_TO_90_DECIMAL                     46
#define _1_TO_90_DECIMAL		             47
#define _600_TO_DWORD		         		 48
#define _10_720_DECIMAL			             49

#define TRANSPORT_TYPE_ENUM				     50
#define CONFIGURE_SIP_SERVERS_ENUM			 51
#define REGISTRATION_MODE_ENUM				 52
#define COMPRESSION_CODE_ENUM				 53
#define ENTITY_ENUM					         54
#define RESOLUTION_SLIDER_ENUM		         55

#define _0_TO_360000_DECIMAL                 56

#define	RESTORE_TYPE_ENUM				57
#define BACKUP_TYPE_ENUM				58
#define _0_TO_100_DECIMAL               59
#define TEN_LINE_BUFFER_LENGTH				60

// Audible values.
#define AUDIBLE_ALARM_TYPE_ENUM				61
#define _1_TO_NUMBER_OF_AUDIBLE_REPETITIONS 62
#define _5_TO_NUMBER_OF_AUDIBLE_INTERVALS   63
#define _10000_TO_1000000_DECIMAL		    64
#define _0_TO_120_DECIMAL		            65

#define _1_TO_2_DECIMAL		            66
#define _1_TO_4_DECIMAL		            67
#define _0_TO_2_DECIMAL		            68
#define _0_TO_4_DECIMAL		            69
#define _5_TO_120_DECIMAL				70



// opcodes from V4.1C
#define _2_TO_120_DECIMAL		    	     71
#define	_3_TO_50_DECIMAL		    	     72
#define _6_TO_240_DECIMAL		    	     73
#define MCU_MODE_ENUM						 74
#define _1_TO_15_DECIMAL					 75
#define _1_TO_1000_DECIMAL					 76
#define _MINUS_1_TO_300_DECIMAL					 77

#define  _0_TO_300_DECIMAL						78
#define  _2_TO_3_DECIMAL		                79
#define API_FORMAT_TYPE_ENUM					80

#define _MINUS_1_TO_100_DECIMAL					81



// CsMngr and ConfParty Common values.
#define ALIAS_TYPE_ENUM								100
#define ALIAS_NAME_LENGTH							101
#define NET_SERVICE_PROVIDER_NAME_LENGTH			102
#define _0_TO_RTM_ISDN_SERVICE_PROVIDER_NAME_LENGTH	103
#define _0_TO_NEW_FILE_NAME_LENGTH					104
#define _1_TO_NEW_FILE_NAME_LENGTH					105
#define ICON_LOCATION_ENUM							106

#define HASH_METHOD_ENUM			107
#define _1_TO_STATION_NAME_LENGTH					108



// CsMngr and Cards Common values
#define DHCP_STATE_ENUM                     150
#define _20_TO_120_DECIMAL					151

// Faults
#define FAULT_SUBJECT_ENUM					201
#define FAULT_LEVEL_ENUM					202
#define FAULT_FILE_ENUM						203
#define PROCESS_NAME_ENUM					204
#define FAULT_TYPE_ENUM						205


#define _1_TO_FILE_NAME_LENGTH					241
#define _1_TO_EXCEPT_HANDL_MES_LENGTH			242
#define _0_TO_GENERAL_MES_LENGTH				245

// CDR
#define UNLIMITED_CHAR_LENGTH					300
#define _0_TO_31_STRING_LENGTH					301
#define CONF_END_CAUSE_TYPE_ENUM				302
#define CDR_STATUS_ENUM							303
#define _0_TO_MAX_DOS_FILE_NAME_LENGTH			304
#define RESTRICT_MODE_ENUM						305
#define LSD_RATE_ENUM							306
#define T120_RATE_ENUM							307
#define CHAIR_MODE_ENUM							308
#define _0_TO_CONF_INFO_ITEM_LENGTH				309
#define NUM_TYPE_ENUM							310
#define NUM_PLAN_TYPE_ENUM						311
#define PRESENTATION_INDICATOR_TYPE_ENUM		312
#define SCREEN_INDICATOR_TYPE_ENUM				313
#define PRI_LIMIT_PHONE_DIGITS_LENGTH			314
#define INITIATOR_ENUM							315
#define NET_SPECIFIC_TYPE_ENUM					316
#define PREFERRED_TYPE_ENUM						317
#define CHANNEL_CALL_TYPE_ENUM					318

#define END_POINT_TYPE_ENUM						319
#define DB_AUTHENTICATION_MODE_ENUM				320
#define DISCONNECT_LOCATION_TYPE_ENUM			321
#define INITIATOR_TYPE_ENUM						322
#define WAIT_FOR_ASSISTANCE_ENUM				323
#define _0_TO_IP_LIMIT_ADDRESS_CHAR_LENGTH		324
#define BONDING_ENUM							325
#define NODE_TYPE_ENUM							326
#define _1_TO_10_DECIMAL						327
#define _0_TO_USER_INFO_ITEM_LENGTH				328

#define DTMF_FAILURE_ENUM						329
#define START_REC_POLICY_ENUM					330
#define RECORDING_STATUS_ENUM					331
#define _0_TO_MAX_SIP_PRIVATE_EXT_LENGTH		332
#define DISCONNECT_CODING_TYPE_ENUM				333
#define MULTI_RATE_ENUM							334
#define _2_TO_MAX_LOG_SIZE						335

#define HD_RATES_ENUM						    340
#define CONTENT_RATES_ENUM					    341
#define _64_TO_8192_DECIMAL						342
#define _0_TO_1536_DECIMAL            343
#define _0_TO_255_DECIMAL            			344
#define _0_TO_63_DECIMAL            			345

#define COUNTRY_CODE_EXTERNAL_ENUM				350

#define TLSV_ENUM								352

#define ISDN_RESOURCE_POLICY_ENUM               353
#define BONDING_DIALING_METHOD_ENUM             354

// Licensing
#define _0_TO_DONGLE_SERIAL_NUM_LENGTH			360
#define _0_TO_VALIDATION_STRING_LENGTH			361
#define _0_TO_CONFIGURATION_NAME_LENGTH			362
#define _0_TO_VER_NUM_LENGTH					363
#define _0_TO_KEYCODE_LENGTH					364
#define _0_TO_LICENSING_SERVER_LENGTH			365

// Gen cfg
#define _0_TO_5000_DECIMAL						366

// CsMngr and GK Common values
#define GK_CONNECTION_STATE_ENUM                400
#define GK_SERVICE_STATUS_ENUM                  401
#define GK_DYNAMIC_ROLE_ENUM                 	402

// CSMngr
#define _0_TO_MAX_GK_NAME_ENUM                  410

// 0-0xFF enum
#define HEXA_BYTE_ENUM							411

// ip service new field
#define _0_TO_MAX_VLAN_PRIORITY	                420

//for predefined layouts
#define FULL_LAYOUT_ENUM						425
#define SITE_NAMES_LOCATION_ENUM				426
#define PCM_LANGUAGE_ENUM						427

//failover
#define _0_TO_FAILOVER_STRING_LENGTH	        429

// Snmp
#define _0_TO_SNMP_STRING_LENGTH	            430

//Jitc
#define _0_TO_14_DECIMAL                     	431
#define _7_TO_14_DECIMAL					 	432
#define _0_TO_16_DECIMAL                     	433
#define _10_TO_16_DECIMAL					 	434
#define _0_TO_7_DECIMAL                      	435
#define _1_TO_7_DECIMAL					 	 	436
#define _15_TO_30_DECIMAL					 	437
#define _0_TO_999_DECIMAL                    	438
#define _1_TO_999_DECIMAL                    	439
#define _7_TO_90_DECIMAL					 	440
#define _10_TO_80_DECIMAL					 	441
#define _4_TO_80_DECIMAL					 	442
#define _0_TO_45000_DECIMAL						443
#define _1_TO_45000_DECIMAL						444
#define _30_TO_90_DECIMAL						445
#define _0_TO_480_DECIMAL						446
#define _YES_ONLY							    447
#define _NO_ONLY							    448
#define _15_TO_20_DECIMAL					 	449
#define _0_TO_20_DECIMAL						450
#define _9_TO_16_DECIMAL                        451
#define _5_TO_60_DECIMAL                    	452
#define _2_TO_10_DECIMAL                    	453
#define _2_TO_160_DECIMAL                       454
#define _6_TO_20_DECIMAL                        455
#define _0_TO_75_DECIMAL                        456

//EMA
#define DIRECTION_TYPE_ENUM						457

// Valid values for flag MPMX_MAX_NUMBER_OF_VIDEO_PARTIES_IN_CONF
#define _180_TO_360_DECIMAL                     458

//MLPP
#define PRECEDENCE_LEVEL_TYPE_ENUM				459
#define PRECEDENCE_DOMAIN_ID_ENUM				460

#define FPS_MODE_ENUM							462

#define _CHANGE_AD_HOC_CONF_DURATION 			463	//Ad hoc conference duration

#define _0_TO_ENCRYPT_PASSWORD_LENGTH		    464
#define ICE_SERVER_ROLE_ENUM			    465
#define ICE_SERVER_STATUS_ENUM			    466
#define ICE_FIREWALL_DETECTION_ENUM		    467
#define _0_TO_ENCRYPT_PASSWORD_SHA256_LENGTH 468

// Amdocs Encoder/Decoder Gain
#define _0_TO_1000_DECIMAL                  469

#define	TWO_HUNDRED_LINE_BUFFER_LENGTH		470

//ldap definitions
#define  LDAP_DIR_TYPE_ENUM					471
#define  LDAP_DIR_PORT_ENUM					472
#define  LDAP_AUTHENTICATION_TYPE_ENUM		473

#define  _5_TO_1440_DECIMAL					475

//RTV
#define MAX_RTV_RESOLUTION_ENUM				476
#define MS_CLIENT_AUDIO_CODEC_ENUM			477
#define MS_CLIENT_AUDIO_CODEC__FOR_SINGLE_CORE_ENUM		478
#define ENCODE_RTV_B_FRAME_ENUM				479
#define _0_TO_384_DECIMAL                   480
#define MS_FEC_ENUM							481
#define SRTP_SRTCP_HMAC_SHA_LENGTH_ENUM	    482

//Alternative Network Address Types
#define ANAT_IP_PROTOCOL_ENUM					483


#define CRL_MODE_802_1X_ENUM                    484
#define CERTIFICATE_MODE_802_1X_ENUM            485
//Ocsp timeout
#define _1_TO_20_DECIMAL                        486
#define _70_TO_95_DECIMAL                       487

#define MAX_MSSVC_RESOLUTION_ENUM               488
#define _50_TO_1000_DECIMAL                 	489

#define _3_TO_300_DECIMAL                       490
#define FOLLOW_SPEAKER_ON_1X1_ENUM             491

#define LICENSE_FEATURE_ENUM                    492
#define LICENSE_FEATURE_STATUS_ENUM             493
#define LICENSE_STATUS_REASON_ENUM              494


// Active speaker preference
#define _0_TO_MAX_ACTIVE_SPEAKER_PREFERENCE     495

#define _0_OR_1_OR_4_DECIMAL                        	496

#define LICENSE_MODE_ENUM                          	497

//******************************************************
// 500 and up are defined in StringMaps.h!!!
//******************************************************


// VALUES MUST BE BETWEEN 0-9999


#endif // _INITCOMMONSTRINGS_H

