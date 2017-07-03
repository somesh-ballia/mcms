#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

#ifndef _ICE_LOG_ADAPTER_HXX_
#define _ICE_LOG_ADAPTER_HXX_


//swith flag ( GideonSim/MediaMngr or CS IceSim )
#define _MCMS_SIM_ENV_

#if defined(_MCMS_SIM_ENV_)
//GideonSim/MediaMngr environment
#include <stdarg.h>

inline void IcePrintLog(const char *filename, int lineno, const char *fmt, ...)
{
	/* Guess we need no more than 100 bytes. */
	int n, size = 1000;
	va_list ap;

	auto_array<char> p(new char[size]);
	memset(p.c_array(), 0, size);

	/* Try to print in the allocated space. */
	va_start(ap, fmt);
	n = vsnprintf (p.c_array(), size-1, fmt, ap);
	va_end(ap);

	//PTRACE1(eLevelInfoNormal, p.c_array());
	OutTraceMessage(filename, lineno, eLevelInfoNormal, NULL, p.c_array(), NULL);
}

#define ICE_LOG_TRACE(fmt...) IcePrintLog(__FILE__, __LINE__, fmt)
#else
//CS environment
#include "AcPrintLog.h"
#define ICE_LOG_TRACE(fmt, args...) AcPrintLog(fmt, PERR, args)
#endif


#endif

#endif	//__DISABLE_ICE__
