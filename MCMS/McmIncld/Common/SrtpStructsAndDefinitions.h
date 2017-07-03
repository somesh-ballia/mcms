/*
 * SrtpStructsAndDefinitions.h
 *
 *  Created on: Feb 18, 2010
 *      Author: Boris Sirotin
 */

#ifndef SRTPSTRUCTSANDDEFINITIONS_H_
#define SRTPSTRUCTSANDDEFINITIONS_H_

#define  	MAX_BASE64_KEY_SALT_LEN					 	44
#define  	MAX_NUM_OF_SDES_KEYS					 	5

typedef enum{
	eAes_Cm_128_Hmac_Sha1_80,
	eAes_Cm_128_Hmac_Sha1_32,
	eF8_Cm_128_Hmac_Sha1_80,
	eUnknownSuiteParam
} sdesCryptoSuiteEnum;


typedef enum{
	eFecSrtp,
	eSrtpFec,
	eSrtpUnknownFec
} sdesFecOrderEnum;

typedef enum{
	eSdesInlineKeyMethod,
	eSdesUnknownKeyMethod
} sdesKeyMethodEnum;

typedef struct{
	char				keySalt[MAX_BASE64_KEY_SALT_LEN];	// mandatory param
	APIU32				bIsLifeTimeInUse;
	APIU32				lifetime;							// optional param
	APIU32				bIsMkiInUse;
	APIU32 				mkiValue;							// optional param
	APIU32				bIsMkiValueLenInUse;
	APIU32 				mkiValueLen;						// optional param

} sdesKeyInfoStruct;

typedef struct{
	APIU32					keyMethod;
	sdesKeyInfoStruct		keyInfo;

} sdesKeyParamsStruct;

typedef struct {
	APIU32								sdesUnencryptedSrtp;		// default is 0-encrypted
	APIU32								sdesUnencryptedSrtcp;		// default is 0-encrypted
	APIU32								sdesUnauthenticatedSrtp;	// default is 0-authenticated
	APIU32								bIsKdrInUse;
	APIU32								sdesKdr;
	APIU32								bIsWshInUse;
	APIU32								sdesWsh;	//SRTP window size-parameter to protect against replay attacks
	APIU32								bIsFecOrderInUse;
	APIU32								sdesFecOrder;
	APIU32								bIsFecKeyInUse;

} sdesSessionParamsStruct;

typedef struct{
	APIU32					bIsSrtpInUse;
	APIU32					cryptoSuite;
	sdesSessionParamsStruct	sessionParams;

	APIU32					numKeyParams;
	sdesKeyParamsStruct		keyParamsList[MAX_NUM_OF_SDES_KEYS];			// Dynamic parts
}sdesCapSt;

#endif /* SRTPSTRUCTSANDDEFINITIONS_H_ */
