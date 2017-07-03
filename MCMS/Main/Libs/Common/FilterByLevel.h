// FilterByLevel.h

#ifndef FILTER_BY_LEVEL_H_
#define FILTER_BY_LEVEL_H_

#include <string>

#include "FilterTrace.h"

class CTerminalCommand;

class CFilterByLevel : public CFilterTrace
{
CLASS_TYPE_1(CFilterTrace, CPObject)
public:
	explicit CFilterByLevel(const CTerminalCommand& command);
	explicit CFilterByLevel(const std::string& level);
	virtual const char* NameOf(void) const;
	
	virtual bool CheckFilter(const TRACE_HEADER_S& tHeader) const;
	virtual bool IsValid(void) const;
	virtual CFilterTrace* Clone(void) const;
	virtual void Set(const CFilterTrace* rhs);
	virtual bool operator==(const CFilterTrace& rhs) const;
	unsigned int GetMaxLevel(void) const;
	
private:
	bool SetStrLevel(const std::string& level);

	bool m_IsValid;	
	DWORD m_MaxLevel;
};

#endif  //FILTER_BY_LEVEL_H_
