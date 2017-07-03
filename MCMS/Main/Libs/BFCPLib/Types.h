//------------------------------------------------------------------------------
//
// Filename:    Types.h
//
// Description: This file contains common type definitions and structures
//              for basic data types.  Additionally it contains definitions
//              to make portability earier.  Some of these might be better
//              placed in another header file.
//
// Copyright:   Polycom, Inc. 2000
//
// History:     5/15/00 gdw     First put together.
//              6/8/00  gdw     Added TriMedia target.
//              1/4/01  gdw     Changed the definition of UInt64/SInt64 for Equ.
//              1/4/01  gdw     BOOL back in for Equator builds.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------

#ifndef __TYPES__
#define __TYPES__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
#include "OverRidePrintf.h" // over ride printf to os_print
#endif
/*------------------------------------------------------------------------------
    TARGET_CPU_*
    These conditionals specify which microprocessor instruction set is being
    generated.  At most one of these is true, the rest are false.

        TARGET_CPU_TM1          - Compiler is generating TriMedia 32 bit instructions
        TARGET_CPU_TM2          - Compiler is generating TriMedia 64 bit instructions
        TARGET_CPU_PPC          - Compiler is generating PowerPC instructions
        TARGET_CPU_X86          - Compiler is generating x86 instructions
        TARGET_CPU_MIPS         - Compiler is generating MIPS instructions
        TARGET_CPU_MAPCA        - Compiler is generating Equator MAP-CA instructions
        TARGET_CPU_MAP1000      - Compiler is generating Equator MAP-1000A instructions

    TARGET_OS_*
    These conditionals specify in which Operating System the generated code will
    run. At most one of the these is true, the rest are false.

        TARGET_OS_WIN32         - Generate code will run under 32-bit Windows
        TARGET_OS_PSOS          - Generate code will run under pSOS
        TARGET_OS_CE            - Generate code will run under Windows CE

    TARGET_RT_*
    These conditionals specify in which runtime the generated code will
    run. This is needed when the OS and CPU support more than one runtime

        TARGET_RT_LITTLE_ENDIAN - Generated code uses little endian format for data
        TARGET_RT_BIG_ENDIAN    - Generated code uses big endian format for data

    PRAGMA_*
    These conditionals specify whether the compiler supports particular #pragma's

        PRAGMA_IMPORT           - Compiler supports: #pragma import on/off/reset
        PRAGMA_ONCE             - Compiler supports: #pragma once
        PRAGMA_STRUCT_ALIGN     - Compiler supports: #pragma options
        PRAGMA_STRUCT_PACK      - Compiler supports: #pragma pack(n)
        PRAGMA_STRUCT_PACKPUSH  - Compiler supports: #pragma pack(push, n)/pack(pop)
        PRAGMA_ENUM_PACK        - Compiler supports: #pragma options(!pack_enums)
        PRAGMA_ENUM_ALWAYSINT   - Compiler supports: #pragma enumsalwaysint on/off/reset
        PRAGMA_ENUM_OPTIONS     - Compiler supports: #pragma options enum=int/small/reset

    FOUR_CHAR_CODE
    This conditional does the proper byte swapping to assue that a four character code (e.g. 'YUV2')
    is compiled down to the correct value on all compilers.

        FOUR_CHAR_CODE('abcd')  - Convert a four-char-code to the correct 32-bit value

    TYPE_*
    These conditionals specify whether the compiler supports particular types.

        TYPE_LONGLONG           - Compiler supports "long long" 64-bit integers
        TYPE_BOOL               - Compiler supports "bool"

    FUNCTION_*
    These conditionals specify whether the compiler supports particular language extensions
    to function prototypes and definitions.

        FUNCTION_DECLSPEC       - Compiler supports "__declspec(xxx) void Foo()"
        FUNCTION_WIN32CC        - Compiler supports "void __cdecl Foo()" and "void __stdcall Foo()"

------------------------------------------------------------------------------*/

#ifdef __TM1__
#include <tmlib/tmtypes.h>
#endif

#ifdef __EQUATOR_MODEL__
#include "mm.h"
#endif

#if defined(_M_IX86)
    #define TARGET_CPU_TM1          0
    #define TARGET_CPU_TM2          0
    #define TARGET_CPU_PPC          0
    #define TARGET_CPU_X86          1
    #define TARGET_CPU_MIPS         0
    #define TARGET_CPU_MAPCA        0
    #define TARGET_CPU_MAP1000      0

    #define TARGET_OS_WIN32         1
    #define TARGET_OS_PSOS          0
    #define TARGET_OS_CE            0

    #define TARGET_RT_LITTLE_ENDIAN 1
    #define TARGET_RT_BIG_ENDIAN    0

    #define PRAGMA_ONCE             1
    #define PRAGMA_IMPORT           0
    #define PRAGMA_STRUCT_ALIGN     0
    #define PRAGMA_STRUCT_PACK      1
    #define PRAGMA_STRUCT_PACKPUSH  1
    #define PRAGMA_ENUM_PACK        0
    #define PRAGMA_ENUM_ALWAYSINT   1
    #define PRAGMA_ENUM_OPTIONS     0
    #define TYPEDEF_BOOL            0
    #define FUNCTION_DECLSPEC       0
    #ifndef FUNCTION_WIN32CC            /* allow calling convention to be overriddden */
        #define FUNCTION_WIN32CC    1
    #endif

#elif defined(__EQUATOR__)
    /*
        gcc         Equator
    */
    #define TARGET_CPU_TM1          0
    #define TARGET_CPU_TM2          0
    #define TARGET_CPU_PPC          0
    #define TARGET_CPU_X86          0
    #define TARGET_CPU_MIPS         0
    #define TARGET_CPU_MAPCA        1
    #define TARGET_CPU_MAP1000      0

    #define TARGET_OS_WIN32         0
    #define TARGET_OS_PSOS          1
    #define TARGET_OS_CE            0

    #define TARGET_RT_LITTLE_ENDIAN 1
    #define TARGET_RT_BIG_ENDIAN    0

    #define PRAGMA_IMPORT           0
    #define PRAGMA_STRUCT_ALIGN     0
    #define PRAGMA_ONCE             0
    #define PRAGMA_STRUCT_PACK      1
    #define PRAGMA_STRUCT_PACKPUSH  0
    #define PRAGMA_ENUM_PACK        0
    #define PRAGMA_ENUM_ALWAYSINT   0
    #define PRAGMA_ENUM_OPTIONS     0
    #define TYPE_EXTENDED           0
    #define TYPE_LONGLONG           1
    #define TYPEDEF_BOOL            0
    #define FUNCTION_DECLSPEC       0
    #define FUNCTION_WIN32CC        0

#elif defined(__MIPS__)
    /*
        MIPS
    */
    #define TARGET_CPU_TM1          0
    #define TARGET_CPU_TM2          0
    #define TARGET_CPU_PPC          0
    #define TARGET_CPU_X86          0
    #define TARGET_CPU_MIPS         1
    #define TARGET_CPU_MAPCA        0
    #define TARGET_CPU_MAP1000      0

    #define TARGET_OS_WIN32         0
    #define TARGET_OS_PSOS          0
    #define TARGET_OS_CE            1

    #define TARGET_RT_LITTLE_ENDIAN 0
    #define TARGET_RT_BIG_ENDIAN    1

    #define PRAGMA_IMPORT           0
    #define PRAGMA_STRUCT_ALIGN     0
    #define PRAGMA_ONCE             0
    #define PRAGMA_STRUCT_PACK      1
    #define PRAGMA_STRUCT_PACKPUSH  0
    #define PRAGMA_ENUM_PACK        0
    #define PRAGMA_ENUM_ALWAYSINT   0
    #define PRAGMA_ENUM_OPTIONS     0
    #define TYPE_EXTENDED           0
    #define TYPE_LONGLONG           0
    #define TYPEDEF_BOOL            0
    #define FUNCTION_DECLSPEC       0
    #define FUNCTION_WIN32CC        0

#elif defined(__TM1__)
    /*
        TriMedia 32 bit arch.
    */
    #define TARGET_CPU_TM1          1
    #define TARGET_CPU_TM2          0
    #define TARGET_CPU_PPC          0
    #define TARGET_CPU_X86          0
    #define TARGET_CPU_MIPS         0
    #define TARGET_CPU_MAPCA        0
    #define TARGET_CPU_MAP1000      0

    #define TARGET_OS_WIN32         0
    #define TARGET_OS_PSOS          1
    #define TARGET_OS_CE            0

    #define TARGET_RT_LITTLE_ENDIAN 1
    #define TARGET_RT_BIG_ENDIAN    0

    #define PRAGMA_IMPORT           0
    #define PRAGMA_STRUCT_ALIGN     0
    #define PRAGMA_ONCE             0
    #define PRAGMA_STRUCT_PACK      1
    #define PRAGMA_STRUCT_PACKPUSH  0
    #define PRAGMA_ENUM_PACK        0
    #define PRAGMA_ENUM_ALWAYSINT   0
    #define PRAGMA_ENUM_OPTIONS     0
    #define TYPE_EXTENDED           0
    #define TYPE_LONGLONG           0
    #define TYPEDEF_BOOL            0
    #define FUNCTION_DECLSPEC       0
    #define FUNCTION_WIN32CC        0

#endif

/*------------------------------------------------------------------------------
    Special values in C

        NULL        The C standard for an impossible pointer value
        nil         A carry over from the past, NULL is prefered for C
------------------------------------------------------------------------------*/
#ifndef NULL
    /* Some C compilers (but not C++) want NULL and nil to be (void*)0  */
    #if !defined(__cplusplus) && (defined(__SC__))
        #define NULL ((void *) 0)
    #else
        #define NULL 0L
    #endif
#endif

#ifndef nil
    #define nil NULL
#endif

/*------------------------------------------------------------------------------
    Base integer types for all target OS's and CPU's

        UInt8    8-bit unsigned integer
        SInt8    8-bit signed integer
        UInt16  16-bit unsigned integer
        SInt16  16-bit signed integer
        UInt32  32-bit unsigned integer
        SInt32  32-bit signed integer
        UInt64  64-bit unsigned integer
        SInt64  64-bit signed integer
------------------------------------------------------------------------------
        SInt16  16-bit signed integer
        UInt32  32-bit unsigned integer
        SInt32  32-bit signed integer
        UInt64  64-bit unsigned integer
        SInt64  64-bit signed integer
------------------------------------------------------------------------------*/

// Certain trimedia files defined variable types.  If such a file is included
// before including this file, then we need to take care of the potential
// duplicate definitions.  We use the _TMtypes_h sentinel to protect against
// the duplication definitions.

#ifndef _TMtypes_h
#if !defined(TYPE_UInt8)
    typedef unsigned char      UInt8;
#define TYPE_UInt8
#endif

#if !defined(TYPE_SInt8)
typedef signed char            SInt8;
#define TYPE_SInt8
#endif
#endif

#if !defined(TYPE_SINT8)
typedef SInt8 SINT8;
typedef SInt8 *PSINT8;
#define TYPE_SINT8
#endif

#if !defined(TYPE_UINT8)
#ifndef UINT8
typedef UInt8 UINT8;
#endif
typedef UInt8 *PUINT8;
#define TYPE_UINT8
#define UINT8_TYPEDEF
#endif

#ifndef _TMtypes_h
#if !defined(TYPE_Int16)
typedef short int          Int16;
#define TYPE_Int16
#endif
#if !defined(TYPE_UInt16)
typedef unsigned short int UInt16;
#define TYPE_UInt16
#endif
#endif
#if !defined(TYPE_SInt16)
typedef signed short int       SInt16;
#define TYPE_SInt16
#endif

#if !defined(TYPE_UINT16)
#ifndef UINT16
typedef UInt16 UINT16;
#endif
typedef UInt16 *PUINT16;
#define TYPE_UINT16
#define UINT16_TYPEDEF
#endif

#if !defined(TYPE_SINT16)
typedef SInt16 SINT16;
typedef SInt16 *PSINT16;
#define TYPE_SINT16
#endif

#ifndef _TMtypes_h
#if !defined(TYPE_Int)
    typedef int                Int;
#define TYPE_Int
#endif
#if !defined(TYPE_UInt32)
    typedef unsigned int       UInt32;
#define TYPE_UInt32
#endif
#endif

#if !defined(TYPE_SInt32)
typedef signed int             SInt32;
#define TYPE_SInt32
#endif

#if !defined(TYPE_UINT32)
#ifndef UINT32
typedef UInt32 UINT32;
#endif
typedef UInt32 *PUINT32;
#define TYPE_UINT32
#endif

#if !defined(TYPE_SINT32)
typedef SInt32 SINT32;
typedef SInt32 *PSINT32;
#define TYPE_SINT32
#endif

#ifndef _TMtypes_h
    typedef unsigned int       Bool;
#endif

#if !defined(DEF_BOOL) && !defined(TYPE_BOOL)
#define DEF_BOOL 1
#define TYPE_BOOL 1
#ifndef BOOL
typedef Bool            BOOL;
#endif
#endif

#if BIG_ENDIAN

struct wide {
    SInt32                  hi;
    SInt32                  lo;
};
typedef struct wide         wide;

struct UnsignedWide {
    UInt32                  hi;
    UInt32                  lo;
};
typedef struct UnsignedWide UnsignedWide;

#else

struct wide {
    SInt32                  lo;
    SInt32                  hi;
};
typedef struct wide         wide;

struct UnsignedWide {
    UInt32                  lo;
    UInt32                  hi;
};
typedef struct UnsignedWide UnsignedWide;
#endif  /* BIG_ENDIAN */

#ifdef TARGET_RT_LITTLE_ENDIAN
#define FOUR_CHAR_CODE(x)   ((((UInt32) ((x) & 0x000000FF)) << 24) \
                             | (((UInt32) ((x) & 0x0000FF00)) << 8) \
                             | (((UInt32) ((x) & 0x00FF0000)) >> 8) \
                             | (((UInt32) ((x) & 0xFF000000)) >> 24))
#else

#define FOUR_CHAR_CODE(x)   (x)

#endif

/*------------------------------------------------------------------------------
    Note:   UInt64 and SInt64 can be either a struct or a long long,
            depending on the compiler.

            The MS Visual C/C++ compiler uses __int64 instead of long long.
------------------------------------------------------------------------------*/

#if TYPE_LONGLONG
    #if defined(_MSC_VER) && !defined(__MWERKS__) && defined(_M_IX86)
        typedef   signed __int64   SInt64;
        typedef unsigned __int64   UInt64;
    #else
        typedef   signed long long SInt64;
        #ifndef TYPE_UInt64
          typedef unsigned long long UInt64;
           #define TYPE_UInt64
        #endif
    #endif
#else

    #if defined(__EQUATOR__) || defined (__EQUATOR_MODEL__)

        #ifndef TYPE_SINT64
           typedef n64 SINT64;
           #define TYPE_SINT64
        #endif
        #ifndef TYPE_SInt64
       typedef n64                 SInt64;
           #define TYPE_SInt64
        #endif
        #ifndef TYPE_UINT64
           typedef n64 UINT64;
           #define TYPE_UINT64
        #endif
        #ifndef TYPE_UInt64
       typedef n64                 UInt64;
           #define TYPE_UInt64
        #endif
    #ifndef TYPE_INT64
            typedef n64 INT64;
       #define TYPE_INT64
    #endif
        #ifndef TYPE_Int64
           typedef n64 Int64;
           #define TYPE_Int64
        #endif
    #elif defined(_MSC_VER) && !defined(__MWERKS__) && defined(_M_IX86)
        typedef   signed __int64   SInt64;
        typedef unsigned __int64   UInt64;

    #else
        typedef wide               SInt64;
        typedef UnsignedWide       UInt64;
    #endif


#endif // TYPE_LONGLONG

/*------------------------------------------------------------------------------
    Base fixed point types

        Fixed           16-bit signed integer plus 16-bit fraction
        UnsignedFixed   16-bit unsigned integer plus 16-bit fraction
------------------------------------------------------------------------------*/

typedef long                       Fixed;
typedef Fixed *                    FixedPtr;
typedef unsigned long              UnsignedFixed;
typedef UnsignedFixed *            UnsignedFixedPtr;

/*------------------------------------------------------------------------------

    Memory Manager types

        Ptr             Pointer to a non-relocatable block
        Size            The number of bytes in a block

------------------------------------------------------------------------------*/
typedef char *                     Ptr;
typedef long                       Size;

/*------------------------------------------------------------------------------

    Higher level basic types

        OSErr                   16-bit result error code
        LogicalAddress          Address in the clients virtual address space
        ConstLogicalAddress     Address in the clients virtual address space
                                that will only be read.
        PhysicalAddress         Real address as used on the hardware bus
        BytePtr                 Pointer to an array of bytes
        ByteCount               The size of an array of bytes
        ByteOffset              An offset into an array of bytes
        ItemCount               32-bit iteration count
        AbsoluteTime            64-bit clock
        FourCharCode            A 32-bit value made by packing four 1 byte
                                characters together.

------------------------------------------------------------------------------*/
typedef SInt16                     OSErr;
typedef void *                     LogicalAddress;
typedef const void *               ConstLogicalAddress;
typedef void *                     PhysicalAddress;
typedef UInt8 *                    BytePtr;
typedef UInt32                     ByteCount;
typedef UInt32                     ByteOffset;
typedef UInt32                     ItemCount;
typedef UnsignedWide               AbsoluteTime;
typedef unsigned long              FourCharCode;

/*------------------------------------------------------------------------------
    Boolean types and values

        Boolean         A one byte value, holds "false" (0) or "true" (1)
        false           The Boolean value of zero (0)
        true            The Boolean value of one (1)

    The identifiers "true" and "false" are becoming keywords in C++
    and work with the new built-in type "bool" "Boolean" will remain
    an unsigned char for compatibility with source code written before
    "bool" existed.
------------------------------------------------------------------------------*/

#if !TYPEDEF_BOOL
    #if TARGET_OS_WIN32
        /* MS VC normally warns if true or false is defined */
        #pragma warning (disable: 4237)
    #endif

#ifndef __cplusplus
#ifndef false
enum {
    false                       = 0,
    true                        = 1
};
#endif
#endif /*#ifndef __cplusplus*/

    #if TARGET_OS_WIN32
        #pragma warning (default: 4237)
    #endif
#endif  /* !TYPEDEF_BOOL */

typedef unsigned char              Boolean;

/*------------------------------------------------------------------------------

    Under Win32, there are two calling conventions: __cdecl or __stdcall
    Headers and implementation files can use the following macros to make their
    source more portable by hiding the calling convention details:

    EXTERN_API*
    These macros are used to specify the calling convention on a function prototype.

        EXTERN_API_C            - C,      Win32: __cdecl
        EXTERN_API_C_STDCALL    - C,      Win32: __stdcall

    DEFINE_API*
    These macros are used to specify the calling convention on a function definition.

        DEFINE_API_C            - C,      Win32: __cdecl
        DEFINE_API_C_STDCALL    - C,      Win32: __stdcall

    CALLBACK_API*
    These macros are used to specify the calling convention of a function pointer.

        CALLBACK_API_C          - C,      Win32: __stdcall
        CALLBACK_API_C_STDCALL  - C,      Win32: __cdecl

------------------------------------------------------------------------------*/

#if FUNCTION_DECLSPEC && !FUNCTION_WIN32CC
    /* compiler supports __declspec() */
    #define EXTERN_API(_type)                    extern __declspec(dllimport) _type
    #define EXTERN_API_C(_type)                  extern __declspec(dllimport) _type
    #define EXTERN_API_STDCALL(_type)            extern __declspec(dllimport) _type
    #define EXTERN_API_C_STDCALL(_type)          extern __declspec(dllimport) _type

    #define DEFINE_API(_type)                    __declspec(dllexport) _type
    #define DEFINE_API_C(_type)                  __declspec(dllexport) _type
    #define DEFINE_API_STDCALL(_type)            __declspec(dllexport) _type
    #define DEFINE_API_C_STDCALL(_type)          __declspec(dllexport) _type

    #define CALLBACK_API(_type, _name)           _type ( * _name)
    #define CALLBACK_API_C(_type, _name)         _type ( * _name)
    #define CALLBACK_API_STDCALL(_type, _name)   _type ( * _name)
    #define CALLBACK_API_C_STDCALL(_type, _name) _type ( * _name)

#elif FUNCTION_DECLSPEC && FUNCTION_WIN32CC
    /* compiler supports __declspec() and __cdecl */
    #define EXTERN_API(_type)                    __declspec(dllimport) _type __cdecl
    #define EXTERN_API_C(_type)                  __declspec(dllimport) _type __cdecl
    #define EXTERN_API_STDCALL(_type)            __declspec(dllimport) _type __stdcall
    #define EXTERN_API_C_STDCALL(_type)          __declspec(dllimport) _type __stdcall

    #define DEFINE_API(_type)                    __declspec(dllexport) _type __cdecl
    #define DEFINE_API_C(_type)                  __declspec(dllexport) _type __cdecl
    #define DEFINE_API_STDCALL(_type)            __declspec(dllexport) _type __stdcall
    #define DEFINE_API_C_STDCALL(_type)          __declspec(dllexport) _type __stdcall

    #define CALLBACK_API(_type, _name)           _type (__cdecl * _name)
    #define CALLBACK_API_C(_type, _name)         _type (__cdecl * _name)
    #define CALLBACK_API_STDCALL(_type, _name)   _type (__stdcall * _name)
    #define CALLBACK_API_C_STDCALL(_type, _name) _type (__stdcall * _name)

#elif !FUNCTION_DECLSPEC && FUNCTION_WIN32CC
    /* compiler supports __cdecl */
    #define EXTERN_API(_type)                    _type __cdecl
    #define EXTERN_API_C(_type)                  _type __cdecl
    #define EXTERN_API_STDCALL(_type)            _type __stdcall
    #define EXTERN_API_C_STDCALL(_type)          _type __stdcall

    #define DEFINE_API(_type)                    _type __cdecl
    #define DEFINE_API_C(_type)                  _type __cdecl
    #define DEFINE_API_STDCALL(_type)            _type __stdcall
    #define DEFINE_API_C_STDCALL(_type)          _type __stdcall

    #define CALLBACK_API(_type, _name)           _type (__cdecl * _name)
    #define CALLBACK_API_C(_type, _name)         _type (__cdecl * _name)
    #define CALLBACK_API_STDCALL(_type, _name)   _type (__stdcall * _name)
    #define CALLBACK_API_C_STDCALL(_type, _name) _type (__stdcall * _name)

#else
    /* compiler supports no extensions */
    #define EXTERN_API(_type)                    extern _type
    #define EXTERN_API_C(_type)                  extern _type
    #define EXTERN_API_STDCALL(_type)            extern _type
    #define EXTERN_API_C_STDCALL(_type)          extern _type

    #define DEFINE_API(_type)                    _type
    #define DEFINE_API_C(_type)                  _type
    #define DEFINE_API_STDCALL(_type)            _type
    #define DEFINE_API_C_STDCALL(_type)          _type

    #define CALLBACK_API(_type, _name)           _type ( * _name)
    #define CALLBACK_API_C(_type, _name)         _type ( * _name)
    #define CALLBACK_API_STDCALL(_type, _name)   _type ( * _name)
    #define CALLBACK_API_C_STDCALL(_type, _name) _type ( * _name)
#endif

/*------------------------------------------------------------------------------
    Function Pointer Types

        ProcPtr                 Generic pointer to a function
------------------------------------------------------------------------------*/

typedef CALLBACK_API_C( long , ProcPtr )();

/*------------------------------------------------------------------------------
    Common Constants

        noErr                   OSErr: function performed properly - no error
        kNilOptions             OptionBits: all flags false
------------------------------------------------------------------------------*/

enum {
    noErr                       = 0
};

enum {
    kNilOptions                 = 0
};

/*------------------------------------------------------------------------------
    Versioning structures

        VersRec                 Contents of a 'vers' resource
        VersRecPtr              Pointer to a VersRecPtr
        NumVersion              Packed BCD version representation
                                (e.g. "4.2.1a3" is 0x04214003)
------------------------------------------------------------------------------*/

#if BIG_ENDIAN

struct NumVersion {
                                                    /* Numeric version part of 'vers' resource */
    UInt8                       majorRev;           /*1st part of version number in BCD*/
    UInt8                       minorAndBugRev;     /*2nd & 3rd part of version number share a byte*/
    UInt8                       stage;              /*stage code: dev, alpha, beta, final*/
    UInt8                       nonRelRev;          /*revision level of non-released version*/
};
typedef struct NumVersion       NumVersion;
#else

struct NumVersion {
                                                    /* Numeric version part of 'vers' resource accessable in little endian format */
    UInt8                       nonRelRev;          /*revision level of non-released version*/
    UInt8                       stage;              /*stage code: dev, alpha, beta, final*/
    UInt8                       minorAndBugRev;     /*2nd & 3rd part of version number share a byte*/
    UInt8                       majorRev;           /*1st part of version number in BCD*/
};
typedef struct NumVersion       NumVersion;
#endif  /* BIG_ENDIAN */

enum {
                                                    /* Version Release Stage Codes */
    developStage                = 0x20,
    alphaStage                  = 0x40,
    betaStage                   = 0x60,
    finalStage                  = 0x80
};

struct Rect {
    SInt16                      top;
    SInt16                      left;
    SInt16                      bottom;
    SInt16                      right;
};
typedef struct Rect             Rect;
typedef Rect                    *RectPtr;

#ifdef __cplusplus
}
#endif

#endif /* __TYPES__ */
