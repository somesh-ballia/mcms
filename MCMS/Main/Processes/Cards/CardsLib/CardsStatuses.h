// ConfPartyStatuses.h:
//
//
//Date         Updated By         Description
//
//21/12/05	  Judith			Statuses for ConfParty
//========   ==============   =====================================================================

#ifndef      __CARDS_STATUSES
#define      __CARDS_STATUSES

//Range 100000 and above

#define  STATUS_BUSY_SLOT_NUMBER             			100000
#define  STATUS_SLOT_EMPTY                   			100001
#define  STATUS_CARD_STATUS_EXISTS           			100002
#define  STATUS_CARD_STATUS_NOT_EXISTS       			100003
#define  STATUS_CONF_PARTY_NAME_EXISTS       			100004
#define  STATUS_CONF_PARTY_NAME_NOT_EXISTS   			100005

#define  STATUS_CARDS_FILE_CORRUPTED                    100006
#define  STATUS_CARDS_FILE_NOT_EXISTS                   100007
#define  STATUS_HDLC_CARD_NOT_EXISTS_IN_CONFIGURATION   100008
#define  STATUS_NETWORK_SERVICE_ERROR                   100009
#define  STATUS_LINE_NUMBER_NOT_EXIST_IN_SERVICES       100010
#define  STATUS_CARD_IN_CONFERENCE                      100011

#define  STATUS_MAX_CARD_STATUS_EXCEEDED     			100013
#define  STATUS_MAX_UNITS_IN_CARD_EXCEEDED   			100014


#endif  // __CARDS_STATUSES
