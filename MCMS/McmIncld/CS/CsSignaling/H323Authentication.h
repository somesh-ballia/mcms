// H323Authentication.h
// Assaf Weissblat

#ifndef __CALLTASKAUTHENTICATION_H__
#define	__CALLTASKAUTHENTICATION_H__

#include "DataTypes.h"
#include "IpCsEncryptionDefinitions.h"
#include "IpCsSizeDefinitions.h"

#define authentication_sp1_string			"itu-t(0) recommendation(0) h(8) 235 0 3 60"
#define authentication_sp1_AvayEx_string	"itu-t(0) recommendation(0) h(8) 235 0 3 60"
#define AuthOidSp1InBytesSize				9
#define AuthnOidSp1ExInIntsSize				8
#define AuthOidSp1InBytesSizePlusOne		AuthOidSp1InBytesSize + 1
#define	MaxDebugOidSize						20

#define Pf1InitVectorId				1
#define Pf1NonseId					2
#define Pf1SessionIdId				5
#define Pf1IntegrityCheckId			6
#define Pf1InitVectorSize			sizeOfIV
#define Pf1NonceSize				4
//#define Pf1SessionIdSize			11
#define Pf1IntegrityCheckSize		12
#define EncodedMessageSize			4096
#define NumOfBitsInByte				8
#define DhHeaderAndKeyBufferSize	280 // 16 byte header (encryptionTokenBase) +
										// 2048 bits + 8 fillers
#define MaxAuthenticationTokens		1
#define AuthTokensBufferSize		DhHeaderAndKeyBufferSize * MaxAuthenticationTokens


// External variables:
// -------------------
extern BOOL bAuthenPrints;
extern BOOL bDisableAuthentication;


typedef struct {
	APIU8	buffer[EncodedMessageSize];
	int		length;
}encodedMessageSt;

typedef encodedMessageSt savedMessageSt;

typedef struct {
//	int		callIndex;
//	int		conId;
//	int		opcode;
	APIU8	integrityCFromMessageField[Pf1IntegrityCheckSize];
	APIU32	length;
	APIU8	payload;// must be the last

}waitingMessageForAuthenSt;

typedef struct  {
	BOOL						bAuthenticatedSession;
	APIU8						authKey[sizeOf128Key]; // Key for auth check of incoming message.
	waitingMessageForAuthenSt	*pWaitingMessage;
	BOOL						bAuthenticated;

} authenticationSt;

// External routines:
// ------------------
//extern int getH323IntegrityCheck(HPVT,int,INT16 *,int *,UINT8 *);
//extern int setIntegrityFieldInValTreeToZero(HPVT,int);
//extern int encodeIncomingMessage(HPVT,int,encodedMessageSt*);
//extern BOOL compareIntegrityCheck(UINT8 *, UINT8*);
// Extern variables
extern APIU8 authOidSp1InBytes[];
extern APIS32 authOidSp1ExInInts[];


#endif //__CALLTASKAUTHENTICATION_H__
 
