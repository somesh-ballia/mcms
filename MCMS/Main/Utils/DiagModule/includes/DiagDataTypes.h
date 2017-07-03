/**********************************************************************/
/*
* Copyright (C) 2005 POLYCOM Networks Ltd.
* This file contains confidential information proprietary to POLYCOM
* Networks Ltd.
* The use or disclosure of any information contained in this file
* without the written consent of an officer of POLYCOM Networks Ltd 
* is expressly forbidden.
*
************************************************************************/

/**********************************************************************/
/*** APiTypes.h - 	header file for common types in all files 		***/
/***				this file constains all types defenition 		***/
/***				                								***/
/***																***/
/*** written by : Yigal Mizrahi										***/
/*** Date 		: 26/04/05											***/
/***																***/
/***																***/
/**********************************************************************/
/**********************************************************************/
#ifndef _DIAG_DATA_TYPES_H_
#define _DIAG_DATA_TYPES_H_

#include "DataTypes.h"
/**** Basic Types ****/
/*
//   INT8:   a signed byte
//   INT16:  a signed 16-bit word
//   INT32:  a signed 32-bit word
//   INT40:  a signed 40-bit word  (only for TI C3x and C6x
//   INT64:  a signed 64-bit word
//   UINT8:  an unsigned byte
//   UINT16: an unsigned 16-bit word
//   UINT32: an unsigned 32-bit word
//   UINT40: an unsigned 40-bit word  (only for TI C3x and C6x
//   UINT64: an unsigned 64-bit word
//   BOOL:   boolean
*/

#ifndef INT8
typedef char INT8 , APIS8 , *pINT8, Int8;
#endif /* INT8 */

#ifndef INT16
	typedef short INT16 , APIS16 , *pINT16, Int16;
#endif /* INT16 */

#ifndef INT32
	typedef int INT32 , APIS32 , *pINT32, Int32;
#endif /* INT32 */

#ifndef INT40
	typedef int INT40, *pINT40;
#endif /* INT32 */

#ifndef INT64
	typedef long INT64, *pINT64;
#endif /* INT64 */

#ifndef UINT8
	typedef unsigned char UINT8 , APIU8 , *pUINT8, Uint8, BYTE;
#endif /* UINT8 */

#ifdef UINT8
#undef UINT8
typedef char UINT8;
#endif

#ifndef UINT16
	typedef unsigned short UINT16 , APIU16 , *pUINT16, WORD;
#endif /* UINT16 */

#ifndef UINT32
	typedef unsigned int UINT32 , APIU32, APIUBOOL, *pUINT32, Uint32, DWORD;
#endif /* UINT32 */

#ifndef UINT40
	typedef unsigned long UINT40, *pUINT40;
#endif /* UINT40 */

/*
#if !defined(TYPE_UINT64)
typedef unsigned int64 UINT64, *pUINT64;
#define TYPE_UINT64
#endif / * UINT64 * /
*/

#ifndef BOOL
	typedef unsigned int BOOL, *pBOOL, Bool;
#endif /* BOOL */

#ifndef NULL
	#define NULL 0
#endif

#ifndef EMB_TRUE
#define EMB_TRUE  1
#endif 

#ifndef EMB_FALSE
#define EMB_FALSE 0 
#endif 

#ifndef EMB_YES
#define EMB_YES	  1
#endif 

#ifndef EMB_NO
#define EMB_NO	  0
#endif

//#define NOT_VALID 0xf3f3f3f3
#ifndef NOT_VALID
#define NOT_VALID 0xffffffff
#endif

#ifndef VOID
#define VOID    void
#endif

//#define X86_SIM //l.a. yigal's project
#ifdef x86_ARCH 
#define SWAPL(X) X
#define SWAPW(X) X 
#else
#define SWAPL(X) (((X)&0xff)<<24)+(((X)&0xff00)<<8)+(((X)&0xff0000)>>8)+(((X)&0xff000000)>>24)
#define SWAPW(X) (((X)&0xff00)>>8)+(((X)&0xff)<<8)  																																 
#endif


//original DiagDataTypes.h defs:


#endif 
