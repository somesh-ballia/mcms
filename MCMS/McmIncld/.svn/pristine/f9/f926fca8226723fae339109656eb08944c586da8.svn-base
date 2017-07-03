//=================================================================================================
//
//Module Name:  IvrStructs.h
//
//=================================================================================================

#ifndef __IVR_STRUCTS_H__
#define __IVR_STRUCTS_H__

#include "DataTypes.h"

// slide header size is 256 BYTES
#define SLIDE_HEADER_SIZE	256
	
typedef struct
{
	APIU32 	fileSize;				// header+data
	APIS8	marker[8];				// "POLYCOM "
	APIS8	version[4];				// 001
	APIS8	protocol[4];			// H263 / H264
	APIS8	resolution[4];			// CIF / QCIF
	APIU32	numberOfPacketsInFile;	// number of media peckets in data
	APIU32	checksum;				// accumulator of all media BYTES (modulu WORD)
	APIU32	fileHeaderSize;			// =256 for version #1
} SSlideHeader;


#endif	// __IVR_STRUCTS_H__ 

