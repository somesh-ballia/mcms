// ConfPartyStatuses.h:
//
//
//Date         Updated By         Description
//
//21/12/05	  Judith			Statuses for ConfParty
//========   ==============   =====================================================================

#ifndef      __CONF_PARTY_STATUSES
#define      __CONF_PARTY_STATUSES

//Range 100000 and above

//ongoing conference status
//#define STATUS_MEETING_ROOM_NOT_EXISTS    						100000

//#define STATUS_ILLEGAL_WHILE_MEETING_ROOM_EXISTS              100011
//#define STATUS_ILLEGAL_WHILE_CONFERENCES_EXISTS    			100012
//#define STATUS_ILLEGAL_WHILE_PROFILE_EXISTS    				100013

// IVR statuses
#define STATUS_MAX_IVR_EVENT_IN_FEATURE_EXCEEDED  				100014
#define STATUS_IVR_EVENT_IN_FEATURE_EXISTS        				100015
#define STATUS_IVR_EVENT_IN_FEATURE_NOT_EXISTS    				100016
#define STATUS_MAX_DTMF_CODE_NUM_EXCEEDED         				100017
#define STATUS_DTMF_CODE_EXISTS                   				100018
#define STATUS_DTMF_CODE_NOT_EXISTS               				100019
#define STATUS_FEATURE_NOT_EXISTS                 				100020
// Status values for GetIVRMsgParams function
// (CIVRService & CIVRServiceList)
#define STATUS_NO_NEED_TO_UPDATE								100021
#define STATUS_NEED_TO_UPDATE									100022
#define STATUS_IVR_ILLEGAL_PW_LENGTH							100023

#endif  // __CONF_PARTY_STATUSES
