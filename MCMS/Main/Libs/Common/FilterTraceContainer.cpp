// FilterTraceContainer.h

#include "FilterTraceContainer.h"

#include <algorithm>
#include "FilterTrace.h"

struct CFilterByNameHunter : public std::unary_function<bool, const CFilterTrace*>
{
public:	
	CFilterByNameHunter(const char* name) :
	    m_Name(name)
	{}
	
	bool operator()(const CFilterTrace* flt)
	{
		return (0 == strcmp(m_Name, flt->NameOf()));
	}
	
private:
	const char *m_Name;	
};

CFilterTraceContainer::~CFilterTraceContainer()
{
	CFilterVector::iterator iTer = begin();
	CFilterVector::iterator iEnd = end();
	while(iTer != iEnd)
	{
		const CFilterTrace *flt = *iTer;
		PDELETE(flt);
		
		iTer++;
	}
}

void CFilterTraceContainer::AddFilter(const CFilterTrace &flt)
{
	CFilterByNameHunter hunter(flt.NameOf());
	CFilterVector::iterator found = find_if(begin(), end(), hunter);
	if(found == end())
	{
		push_back(flt.Clone());		
	}
	else
	{
		CFilterTrace *fltFound = *found;
		FPASSERT_AND_RETURN(fltFound == NULL);
		fltFound->Set(&flt);
	}
}

bool CFilterTraceContainer::CheckFilter(const TRACE_HEADER_S& tHeader)
{
	bool result = true;
	
	CFilterVector::iterator iTer = begin();
	CFilterVector::iterator iEnd = end();
	while(iTer != iEnd)
	{
		const CFilterTrace *flt = *iTer;
		result = flt->CheckFilter(tHeader);
		if(false == result)
		{
			break;
		}
		iTer++;
	}
	
	return result;
}

const CFilterTrace* CFilterTraceContainer::GetFilterByName(const char* name) const
{
    CFilterByNameHunter hunter(name);
    CFilterVector::const_iterator found = std::find_if(begin(), end(), hunter);
    if (found == end())
        return NULL;

    return *found;
}
