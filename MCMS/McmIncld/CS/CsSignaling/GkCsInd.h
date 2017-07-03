//+========================================================================+
//                       GkCsInd.h		                                   |
//-------------------------------------------------------------------------|


#ifndef __GKCSIND_H__
#define __GKCSIND_H__

#include "H460_1.h"
#include "IpCommonDefinitions.h"
#include "IpCsEncryptionDefinitions.h"
#include "GateKeeperCommonParams.h"
#include "RvCommonDefs.h"



typedef struct {
	APIU32						gkIdentLength     ;
    char						gatekeeperIdent[MaxIdentifierSize];	
    APIS32   				    nRequredAuthMethod;// H.235 Gk Auth.Method Required from GK 
	mcXmlTransportAddress		rasAddress        ;// GK transport address
	h460FsSt					fs                ;
	mcRejectOrConfirmChoice   	rejectOrConfirmCh ;
} gkIndRasGRQ;

typedef struct {
	APIU32						timeToLive;
	APIU32						gkIdentLength;
	char						gatekeeperIdent[MaxIdentifierSize];
	APIU32						epIdentLength;
	char						endpointIdent[MaxIdentifierSize];	//changed receive upon RCF
	h460FsSt					fs;
	h460AvayaFeDscpInd			fsAvayaFeDscpInd;
	mcRejectOrConfirmChoice   	rejectOrConfirmCh; //Alternate GK list

} gkIndRasRRQ; 

typedef struct {
	rejectInfoSt			rejectInfo;							// in case of URJ 

} gkIndRasURQ;

//typedef struct {
//	mcXmlTransportAddress	callSignalAddress;					// addr to send Q.931 (== gk addr if routed mode) 
//	mcXmlTransportAddress	rasAddress;							// for located endpoint.
//	APIU32					bAuthenticated;
//	rejectInfoSt			rejectInfo;							// in case of LRJ
//
//} gkIndRasLRQ;


typedef struct {
	mcXmlTransportAddress	destCallSignalAddress;//according to the standart, the dest in the ACF should always (dialIn / dialOut) be the remote				
	int						crv;	
	cmRASCallType			callType;							
	cmRASCallModelType		callModel;	
	int						bandwidth;
	int						irrFrequency;
	APIU32					destExtraCallInfoTypes[MaxNumberOfAliases];
	h460AvayaFeVndrIndSt    avfFeVndIdInd;
	h460AvayaFeMaxNonAudioBitRateInd avfFeMaxNonAudioBitRateInd;
	char					destExtraCallInfo[MaxAddressListSize];		//canMapAlias
	char					destInfo[MaxAddressListSize];				//canMapAlias
	char					remoteExtensionAddress[MaxAliasLength];		//canMapAlias
	char					conferenceId[MaxConferenceIdSize];
	char					callId[Size16];
	int					numOfGkRoutedAddres;
	mcXmlTransportAddress			gkRouteAddress;
	rejectInfoSt			rejectInfo;
} gkIndRasARQ;

typedef struct {
	rejectInfoSt			rejectInfo;							// in case of DRJ

} gkIndRasDRQ;

typedef struct {
	int						bandwidth;
	rejectInfoSt			rejectInfo;							// in case of DRJ

} gkIndRasBRQ;

typedef struct {
	char					message[MaxErrorMessageSize];
	APIU32					FailIndicationOpcode;

} gkIndRasFail;

typedef struct {
	cmRASTransaction		transaction;
	mcXmlTransportAddress   gkAddress;	

} gkIndRasTimeout;

typedef struct {
	int			            hsRas;
    APIU32					unRegisterReason; 
	altGksListSt			altGkList;  // no need candidate to delete - Ask guy 

} gkIndURQFromGk;

typedef struct {
	cmRASDisengageReason	disengageReason; //GK if not transfer this param to mcms ask guy
	int			            hsRas;

} gkIndDRQFromGk;


typedef struct {
	int						hsRas;
	int						bandwidth;
	h460AvayaFeMaxNonAudioBitRateInd avfFeMaxNonAudioBitRateInd;
}gkIndBRQFromGk;


typedef struct {				
	mcXmlTransportAddress	gatekeeperAddress;						
	char					destinationInfo[MaxAddressListSize];	// aliases for dest.
	char					sourceInfo[MaxAddressListSize];			// aliases for dest.
	char					callId[Size16];							//Needed ??
	int			            hsRas;

}gkIndLRQFromGk;

typedef struct {
	int						hsRas;
} gkIndGKIRQ;

//typedef struct { (Empty)
//	
//} gkIndRasRAC;


#endif //GkCsInd_H
