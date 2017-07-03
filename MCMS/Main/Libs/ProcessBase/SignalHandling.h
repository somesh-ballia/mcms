// SignalHandling.h: Directives in case a thread Crashes .
//
//////////////////////////////////////////////////////////////////////


#if !defined(_SIGNALHANDLING_H__)
#define _SIGNALHANDLING_H__

#include "DataTypes.h"

struct CSignaldHandling
{
	BOOL m_bDeleteQueue;
	BOOL m_bDeleteTask;
    BOOL m_bDeleteProcess;
};

#endif // _SHAREDHEADER_H__
                               
