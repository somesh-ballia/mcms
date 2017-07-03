//------------------------------------------------------------------------------
//
// Filename:	OverRidePrintf.h
//
// Description:	This allow printf to be macro out
//              and redirect prinft to stdout and/or telnet
//
// Copyright:	Polycom, Inc. 2000

#ifndef _OS_PRINTF__
#define _OS_PRINTF__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(WIN32) || defined(_WINDOWS)
    #include <stdio.h> // to ensure no "error C2375 os_print: redefinition, different linkage"
    //#define printf os_print
    extern int os_print(const char *pVar, ...);
#if defined (_VCLAN_)
    extern void enable_os_print(int f);
#endif  // #if defined (_VCLAN_)
#endif  // #if defined(WIN32) || defined(_WINDOWS)

#ifdef __cplusplus
}
#endif

#endif // _OS_PRINTF__
