// FilterByLevel.cpp

#include "FilterByLevel.h"
#include "TerminalCommand.h"
#include "TraceClass.h"

CFilterByLevel::CFilterByLevel(const std::string& level) :
    m_IsValid(SetStrLevel(level))
{}

CFilterByLevel::CFilterByLevel(const CTerminalCommand& command)
{
	m_MaxLevel 	= NUM_OF_TRACE_TYPES;
	m_IsValid 	= false;
	
	if (1 != command.GetNumOfParams())
		return;
	
	m_IsValid = SetStrLevel(command.GetToken(eCmdParam1));
}

// Virtual
const char* CFilterByLevel::NameOf(void) const
{
    return GetCompileType();
}

// Virtual
bool CFilterByLevel::IsValid(void) const
{
    return m_IsValid;
}

// Virtual
bool CFilterByLevel::CheckFilter(const TRACE_HEADER_S& hdr) const
{
    return hdr.m_level <= m_MaxLevel;
}

CFilterTrace* CFilterByLevel::Clone(void) const
{
	return new CFilterByLevel(*this);
}

void CFilterByLevel::Set(const CFilterTrace* rHnd)
{
	const CFilterByLevel* other = dynamic_cast<const CFilterByLevel*>(rHnd);
	if(NULL != other)
	{
		*this = *other;
	}
}

bool CFilterByLevel::SetStrLevel(const string& level)
{
	m_MaxLevel = CTrace::GetTraceLevelByName(level.c_str());
	
	return NUM_OF_TRACE_TYPES != m_MaxLevel;
}

// Virtual
bool CFilterByLevel::operator==(const CFilterTrace& rhs) const
{
    // Checks type before casting
    if (0 != strcmp(NameOf(), rhs.NameOf()))
            return false;

    const CFilterByLevel& obj = reinterpret_cast<const CFilterByLevel&>(rhs);
    return m_IsValid == obj.m_IsValid && m_MaxLevel == obj.m_MaxLevel;
}

unsigned int CFilterByLevel::GetMaxLevel(void) const
{
    return m_MaxLevel;
}
