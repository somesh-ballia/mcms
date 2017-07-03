
// IpCsEncryptionDefinitions.h
// Uri Avni.

#ifndef __IP_CS__ENCRYPTIONDEFINITIONS_H_
#define __IP_CS__ENCRYPTIONDEFINITIONS_H_

#include "IpEncryptionDefinitions.h"

// use only for signaling interface
typedef enum
{
	kHalfKeyUnKnownType = 0,
	kHalfKeyEndToEnd,	// E
	kHalfKeyDH1024,		// 3DES and AES	- 1024 bit
	kHalfKeyDHdummy,	// DES			- 512 bit
}EenHalfKeyType;


#endif /*__IP_CS__ENCRYPTIONDEFINITIONS_H_*/
