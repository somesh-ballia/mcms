#ifndef DATATYPES_H__
#define DATATYPES_H__

#include <stdio.h>
#include <bits/pthreadtypes.h>

//////////////////////////////////////////////////////////////////////
typedef void* HANDLE;

typedef pthread_t THREAD;
typedef int       SM_HANDLE;

//////////////////////////////////////////////////////////////////////
typedef unsigned char  byte;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

//////////////////////////////////////////////////////////////////////
typedef unsigned char      BYTE;
typedef unsigned short     WORD;
typedef unsigned int       DWORD;
typedef unsigned long      ULONG;
typedef unsigned long long ULONGLONG;

//////////////////////////////////////////////////////////////////////
typedef DWORD OPCODE;
typedef DWORD STATUS;

//////////////////////////////////////////////////////////////////////
#ifndef BOOL
#define BOOL unsigned char
#endif
// typedef unsigned char BOOL; // conflicts with BFCPLib/Types.h

//////////////////////////////////////////////////////////////////////
#ifndef NULL
#define NULL 0
#endif

#ifndef NOT_FIND
#define NOT_FIND -1
#endif

//////////////////////////////////////////////////////////////////////
// there are 100 ticks per second
#define TICKS CSystemTick
#define NEVER  TICKS(0xffffffff,0xffffffff)

//////////////////////////////////////////////////////////////////////
#define SERIALEMBD   (unsigned short)1 
#define NATIVE       (unsigned short)2 

//////////////////////////////////////////////////////////////////////
typedef uint   APIUBOOL;

typedef uint   APIU32;
typedef long   APIS32;

typedef ushort APIU16;
typedef short  APIS16;

typedef byte   APIU8;
typedef char   APIS8;

//////////////////////////////////////////////////////////////////////

//typedef unsigned char  UINT8;
//typedef unsigned short UINT16;
//typedef unsigned int   UINT32;
//
//typedef char  INT8;
//typedef short INT16;
//typedef int   INT32;

#ifndef UINT32
#define UINT32 unsigned int
#endif

#ifndef INT32
#define INT32 int
#endif

#ifndef INT8
#define INT8 char
#endif

#ifndef UINT8
#define UINT8 unsigned char
#endif

#ifndef INT16
#define INT16 short
#endif

#ifndef UINT16
#define UINT16 unsigned short
#endif

//////////////////////////////////////////////////////////////////////
#define SECOND 100

//size of i(o)strstream
#define SIZE_STREAM 0x4E20 //20000  Decimal

//////////////////////////////////////////////////////////////////////
#endif // DATATYPES_H__
