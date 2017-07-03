// IpEncryptionDefinitions.h
// Ran stone.

#ifndef __ENCRYPTIONDEFINITIONS_H__
#define	__ENCRYPTIONDEFINITIONS_H__

#define sizeOfIV			12

typedef enum
{
	kUnKnownMediaType = 0, 
	kAES_CBC,
	kAES_CTR,
	kAES_LAST
}EenMediaType;


#endif // __ENCRYPTIONDEFINITIONS_H__
