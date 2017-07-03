// FilterTraceCommand.cpp

#include "Trace.h"
#include "TraceClass.h"
#include "FilterTrace.h"
#include "ProcessBase.h"

// Virtual
CFilterTrace::~CFilterTrace(void)
{}

// Virtual
const char* CFilterTrace::NameOf(void) const
{
    return GetCompileType();
}
