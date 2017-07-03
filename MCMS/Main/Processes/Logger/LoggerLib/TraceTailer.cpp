#include "TraceTailer.h"

CTraceTailer::CTraceTailer()
	: m_Size(0), m_MaxTailLen(DEFAULT_MAX_TAIL_LEN)
{
}

CTraceTailer::~CTraceTailer()
{
}

void CTraceTailer::SetTailLen(DWORD len)
{
	m_MaxTailLen = std::max(std::min(len,MAX_TAIL_LEN),MIN_TAIL_LEN);
	while ( m_Size > m_MaxTailLen )
	{
		PopFront();
	}
}

void CTraceTailer::SetTrace(const std::string &trace)
{
	PushBack(trace);
	if (m_Size > m_MaxTailLen)
	{
		PopFront();
	}
}

void CTraceTailer::GetTail(std::ostream& answer) const
{
	CTraceList::const_iterator iEnd = m_TailContent.end();
	for (CTraceList::const_iterator it = m_TailContent.begin(); it != iEnd; ++it)
		answer << *it;
}

void CTraceTailer::PushBack(const std::string &trace)
{
	m_Size++;
	m_TailContent.push_back(trace);
}

void CTraceTailer::PopFront()
{
	m_Size--;
	m_TailContent.pop_front();
}
