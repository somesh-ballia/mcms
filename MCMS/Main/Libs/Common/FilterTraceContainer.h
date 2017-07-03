// FilterTraceContainer.h

#ifndef FILTER_TRACE_CONTAINER_H_
#define FILTER_TRACE_CONTAINER_H_

#include <vector>
#include "TraceHeader.h"

class CFilterTrace;

typedef std::vector<CFilterTrace*> CFilterVector;

class CFilterTraceContainer : private CFilterVector 
{
public:
	virtual ~CFilterTraceContainer();
	
	void AddFilter(const CFilterTrace& flt);
	bool CheckFilter(const TRACE_HEADER_S& tHeader);
	const CFilterTrace* GetFilterByName(const char* name) const;
};

#endif  // FILTER_TRACE_CONTAINER_H_
