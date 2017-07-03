//+========================================================================+
//                       GkCsReq.h		                                   |
//+=========================================================================

#ifndef __GKCSREQ_H__
#define __GKCSREQ_H__

#include "H460_1.h"
#include "IpCommonDefinitions.h"
#include "IpCsEncryptionDefinitions.h"
#include "GateKeeperCommonParams.h"
#include "RvCommonDefs.h"



// H323_CS_RAS_GRQ_REQ
typedef struct {
	mcXmlTransportAddress		rasAddress;
	mcXmlTransportAddress		gatekeeperAddress;
	APIU32						gkIdentLength;
	char						gatekeeperIdent[MaxIdentifierSize];
    GkH235AuthParam	            H235AuthParams;                     //H.235 GK Authentication
	APIU32						endpointType;
	APIU32						bIsMulticast;
	APIU32						bSupportAltGk; 
	h460FsSt					fs;
	APIU32						aliasesTypes[MaxNumberOfAliases];	// emun cmAliasType.
	APIU32						aliasesLength;
	char						aliasesList[1];
} gkReqRasGRQ;

// H323_CS_RAS_RRQ_REQ
typedef struct {
	mcXmlTransportAddress		gatekeeperAddress;								// used when there is no gk_id.
	mcXmlTransportAddress		callSignalingAddress[MaxNumberOfEndpointIp];	// endpoint address.
	mcXmlTransportAddress		rasAddress;										// endpoint address for GK reply.	
	mcuVendor					endpointVendor;
    GkH235AuthParam	            H235AuthParams;                                 //H.235 GK Authentication
	char						gwPrefix[PHONE_NUMBER_DIGITS_LEN + 1];			//32 //for CISCO GK
    APIU32						gkIdentLength;
	char						gatekeeperIdent[MaxIdentifierSize];
    APIU32						epIdentLength;
	char						endpointIdent[MaxIdentifierSize];
    ctNonStandardParameterSt 	mcuDetails;
	h460FsSt					fs;
	APIU32						dicoveryComplete;								// TRUE ==> discovery + registration
	APIU32						bIsMulticast;
	APIU32						bIsKeepAlive;									// equal 0 if it is the first RRQ.
	APIU32						bSupportAltGk;
	APIU32						timeToLive;
	APIU32						aliasesTypes[MaxNumberOfAliases];				// emun cmAliasType.
	char						prefix[PHONE_NUMBER_DIGITS_LEN + 1];
    APIU32						aliasesLength;
	char						aliasesList[1];	
} gkReqRasRRQ;

// H323_CS_RAS_URQ_REQ
typedef struct {
	mcXmlTransportAddress	gatekeeperAddress;					        // used when there is no gk_id.
	mcXmlTransportAddress	callSignalingAddress[MaxNumberOfEndpointIp];// endpoint address.
    GkH235AuthParam	        H235AuthParams;                             //H.235 GK Authentication
	APIU32					aliasesTypes[MaxNumberOfAliases];	        // emun cmAliasType.
	APIU32					gkIdentLength;
	char					gatekeeperIdent[MaxIdentifierSize];
    APIU32					epIdentLength;
	char					endpointIdent[MaxIdentifierSize];
	APIU32					aliasesLength;
	char					sAliasesList[1];
} gkReqRasURQ;

// H323_CS_RAS_URQ_RESPONSE_REQ

typedef struct {
	int						hsRas;		
} gkReqURQResponse;

// H323_CS_RAS_DRQ_RESPONSE_REQ

typedef struct {
	int						hsRas;	
} gkReqDRQResponse;

// H323_CS_RAS_ARQ_REQ
typedef struct {
	mcXmlTransportAddress	gatekeeperAddress;					// used when there is no gk_id.
	mcXmlTransportAddress	destCallSignalAddress;				// used at dest for call signaling.
	mcXmlTransportAddress	srcCallSignalAddress;				// used at src for call signaling.
    GkH235AuthParam	        H235AuthParams;                             //H.235 GK Authentication
	char 					cid[MaxConferenceIdSize];			//Only for Arq requests
	char 					callId[Size16];						//Only for Arq requests
	APIU32					gkIdentLength;
	char					gatekeeperIdent[MaxIdentifierSize];
    APIU32					epIdentLength;
	char					endpointIdent[MaxIdentifierSize];
    cmRASCallType			callType;							// p2p,one2N,N2N act.
	cmRASCallModelType		callModel;							// direct or routed.
	int						bandwidth;
	APIU32					bIsDialIn;		//bIsAnswerCall
	APIU32					bCanMapAlias;
	h460AvayaFeVndrReqSt    avfFeVndIdReq;
	APIU32					destInfoTypes[MaxNumberOfAliases];	// emun cmAliasType.
	APIU32					srcInfoTypes[MaxNumberOfAliases];	// emun cmAliasType.
	APIU32					srcInfoLength;
	APIU32					destInfoLength;	
	APIU32					totalDynLen;
	char					srcAndDestInfo[1];
} gkReqRasARQ;

// H323_CS_RAS_BRQ_REQ
typedef struct {
	mcXmlTransportAddress	gatekeeperAddress;
    GkH235AuthParam	        H235AuthParams;                     // H.235 GK Authentication 
    APIU32					gkIdentLength;
	char					gatekeeperIdent[MaxIdentifierSize];
    APIU32					epIdentLength;
	char					endpointIdent[MaxIdentifierSize];	
 	APIU32					callType;							// p2p,one2N,N2N act.
	int						bandwidth;
	APIU32					bIsDialIn;	                        // bIsAnswerCall
} gkReqRasBRQ;

// H323_CS_RAS_IRR_REQ
// H323_CS_RAS_IRR_RESPONSE_REQ

typedef struct {
	mcXmlTransportAddress	gatekeeperAddress;					// used when there is no gk_id.
	mcXmlTransportAddress	rasAddress;			                //Local ras address need to take it from Mcms now - CardRasAddress
	mcXmlTransportAddress	srcCallSignalAddress;	
	mcXmlTransportAddress	destCallSignalAddress;	
    GkH235AuthParam	        H235AuthParams;                     //H.235 GK Authentication
	int 					bandwidth;
    //------------HOMOLOGATION. --------------------------------//
    int                     n931Crv;                            //
    //----------------------------------------------------------// 
	int						hsRas;
	APIU32					epIdentLength;
	char					endpointIdent[MaxIdentifierSize];
	char 					callId[Size16];	
	//-S- HOMOLOGATION 3.0 RAS_TE_STA_02 -----------------------//
	char 					conferenceId[MaxConferenceIdSize];
	//-S- HOMOLOGATION 3.0 RAS_TE_STA_02 -----------------------//
	cmRASCallType			callType;							// Use In GKIF As Constant cmCallTypeP2P p2p,one2N,N2N act.
	cmRASCallModelType		callModel;							// direct or routed.
	APIU32					bIsAnswer;							// is call originator.
	APIU32					bIsNeedResponse;
	APIU32					srcInfoTypes[MaxNumberOfAliases];	// emun cmAliasType.
	APIU32					srcInfoLength;
	char					srcInfo[1];
} gkReqRasIRR;//common for IRR and IRRResponse

// H323_CS_RAS_DRQ_REQ
typedef struct {
	mcXmlTransportAddress	gatekeeperAddress;					// used when there is no gk_id.
    GkH235AuthParam	        H235AuthParams;                     // H.235 GK Authentication 
	APIU32					gkIdentLength;
	char					gatekeeperIdent[MaxIdentifierSize];
    APIU32					epIdentLength;
	char					endpointIdent[MaxIdentifierSize];
	cmRASDisengageReason	disengageReason;
	APIU32					bIsDialIn;	//bIsAnswerCall
} gkReqRasDRQ;

// H323_CS_RAS_LRQ_RESPONSE_REQ
typedef struct {
	int						hsRas;
	mcXmlTransportAddress	callSignalingAddress[MaxNumberOfEndpointIp];	
	mcXmlTransportAddress	rasAddress;			                //Local ras address need to take it from Mcms now
} gkReqLRQResponse;

// H323_CS_RAS_BRQ_RESPONSE_REQ 
typedef struct {
	int						hsRas;
	int 					bandwidth;
} gkReqBRQResponse;

// H323_CS_RAS_RAI_REQ
typedef struct {
	
	mcXmlTransportAddress		gatekeeperAddress;	
  	APIU32    					bAlmostOutOfResources; 	// boolean
  	
  	APIU32    					maximumAudioCapacity;   // Max Rai CallCapacity
  	APIU32    					maximumVideoCapacity; 
  	
  	APIU32    					currentAudioCapacity;	// Current Rai CallCapacity
  	APIU32    					currentVideoCapacity; 
  	  	
 	APIU16	  					numOfSupportedProfiles;	// integer
	xmlDynamicProperties		xmlDynamicProps;
 	char						profilesArray [1];
} gkReqRasRAI;

#endif	
