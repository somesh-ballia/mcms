
// GatekeeperCommonParams.h
// Avishay Halavy
// Apr-2011. H.235 realization. Alexander Sheinberg.

#ifndef __GATEKEEPERCOMMONPARAMS_H__
#define __GATEKEEPERCOMMONPARAMS_H__

#include "ChannelParams.h"
#include "NonStandard.h"
#include "Capabilities.h"
#include "IpAddressDefinitions.h"

#define MaxNumberOfGatekeeperIp			3 
#define MaxNumberOfEndpointIp			TOTAL_NUM_OF_IP_ADDRESSES
#define	MaxElemInAltGkList				10
#define	MaxAltGkInfoSize				1024
#define GwTypeLen						8		// H320 or H323

#define PHONE_NUMBER_DIGITS_LEN			31		// Same as in mcmsoper/mcmsoper.h

typedef struct {
	APIU32		calls;
	char		group[10];

} CallsAvailable;

typedef struct {
	CallsAvailable		voiceGwCallsAvailable;
//	CallsAvailable		h310GwCallsAvailable;
	CallsAvailable		h320GwCallsAvailable;
//	CallsAvailable		h321GwCallsAvailable;
//	CallsAvailable		h322GwCallsAvailable;
	CallsAvailable		h323GwCallsAvailable;
//	CallsAvailable		h324GwCallsAvailable;
//	CallsAvailable		t120OnlyGwCallsAvailable;
//	CallsAvailable		t38FaxAnnexbOnlyGwCallsAvailable;
	CallsAvailable		terminalCallsAvailable;
	CallsAvailable		mcuCallsAvailable;		

} CallCapacity;


//anatNon--------------------------------
typedef struct
{
    ctNonStandardIdentifierSt	info;		//8
    int							productLen;	//4
    int							versionLen;	//4
    char						productID[64];//32
    char						versionID[32];//32
} mcuVendor;					// total = 80

//  Alternate GK
// ==============

typedef struct {
	xmlDynamicHeader		xmlHeader;	
	APIU32					bNeedToRegister;
	mcXmlTransportAddress	rasAddress;		
	int						priority;
	APIU32					gkIdentLength;
	char					gkIdent[MaxIdentifierSize];
} alternateGkSt;

typedef struct {
	APIU32					bIsReject;
	int			    		numOfAltGks; //like altGksListStBase
	xmlDynamicProperties    xmlDynamicProps;	
	char					altGkSt[1];  //list of alternateGkSt
} altGksListSt;


//--------------------------------------

typedef struct  {
	APIU32						bIsReject;
	APIU32						rejectReason;	
	APIU32				    	bAltGkPermanent;
	altGksListSt				altGkList;   	
} rejectInfoSt;

typedef union{
	rejectInfoSt				rejectInfo;		// in case of reject 
	altGksListSt				altGkList;		// in case of confirm 
} rejectOrConfirmChoice;

//--------------------------------------

typedef enum {
	eChReject			= 0,
	eChConfirm			= 1
}eChoiceRejectOrConfirm;


typedef struct {
 
    xmlUnionPropertiesSt  		unionProps;
    rejectOrConfirmChoice	  	choice;
} mcRejectOrConfirmChoice;


//----- H.235 GK Authentication -----//
#define H235MaxAuthUserName     128
#define H235MaxAuthPwd			128


#define    eH235Method_Undef     0xFFFFFFFF
#define    eH235Method_Non       0x00 
#define    eH235Method_MD5       0x01
#define    eH235Method_SHA1      0x02
#define    eH235Method_SHA256    0x04   

typedef  struct  _GkH235AuthParam 
{ 
    APIS32         eEncryptMethodConfiguredScale    ; // Default: eH235Method_Non   or  eH235Method_SHA1 | eH235Method_MD5 | eH235Method_SHA256
    APIS32         eEncryptMethodRequired           ; // Default: eH235Method_Undef 
    APIS32         nIsAuth                          ; // Default: FALSE/0
    char           AuthUserName[H235MaxAuthUserName]; // Default: ‘\0’
    char           AuthPassword[H235MaxAuthPwd]     ; // Default: ‘\0’ 
}GkH235AuthParam;
//-----------------------------------//

#endif // __GATEKEEPERCOMMONPARAMS_H__
