// FilterTraceCommand.h

#ifndef FILTER_TRACE_COMMAND_H_
#define FILTER_TRACE_COMMAND_H_

#include "PObject.h"
#include "TraceHeader.h"
#include "McmsProcesses.h"

class CFilterTrace : public CPObject
{
CLASS_TYPE_1(CFilterTrace, CPObject)
public:
	virtual ~CFilterTrace(void);
	
	virtual const char* NameOf(void) const;
	virtual bool CheckFilter(const TRACE_HEADER_S& tHeader) const = 0;
	virtual bool IsValid(void) const = 0;
	virtual CFilterTrace* Clone(void) const = 0;
	virtual void Set(const CFilterTrace* rhs) = 0;
	virtual bool operator==(const CFilterTrace& rhs) const = 0;
};

#endif  // FILTER_TRACE_COMMAND_H_
