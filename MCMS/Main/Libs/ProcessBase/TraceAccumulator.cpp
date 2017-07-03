/*
 * TraceAccumulator.cpp
 *
 *  Created on: Mar 19, 2014
 *      Author: bguelfand
 */

#include <sys/time.h>
#include "TraceStream.h"
#include "TraceAccumulator.h"

CTraceAccumulator::CTraceAccumulator()
{
	m_predicat = NULL;
	m_eTraceLevel = eLevelInfoNormal;
	m_pSenderThis = 0;
	m_tvStartAccumulate.tv_sec = 0;
	m_tvEndAccumulate.tv_sec = 0;
	m_tvExplicit.tv_sec = 0;
}

CTraceAccumulator::CTraceAccumulator(bool (*predicat)(timeval&, timeval&),
		const std::string& sFunction, const CPObject* pSenderThis, eLogLevel traceLevel /* = eLevelInfoNormal*/)
{
	m_predicat = predicat;
	m_sFunction = sFunction;
	m_pSenderThis = pSenderThis;
	m_eTraceLevel = traceLevel;
//	gettimeofday(&m_tvStartAccumulate, 0);
	m_tvStartAccumulate.tv_sec = 0;
	m_tvEndAccumulate.tv_sec = 0;
	m_tvExplicit.tv_sec = 0;
}

CTraceAccumulator::~CTraceAccumulator()
{
//	if (m_predicat == 0)
//		FTRACESTRFUNC(m_eTraceLevel) << MSG.str().c_str();
//	else
//	{
//		if (m_predicat(m_tvStartAccumulate, m_tvEndAccumulate))
//			CTraceStream(__FILE__, __LINE__, (WORD)m_eTraceLevel, m_pSenderThis).seekp(0, std::ios_base::cur)
//				<< "_GLA_ " << TimeOfDayToString(m_tvStartAccumulate) << " - " << TimeOfDayToString(m_tvEndAccumulate)
//				<< " - " << m_sFunction.c_str() << "\n" << MSG.str().c_str();
//	}
}

void CTraceAccumulator::Reset()
{
	timerclear(&m_tvStartAccumulate);
	timerclear(&m_tvEndAccumulate);
	MSG.str("");
}

void CTraceAccumulator::Flush(const char* pszTitle)
{
	if (m_predicat == 0)
		FTRACESTRFUNC(m_eTraceLevel) << MSG.str().c_str();
	else
	{
		if (m_predicat(m_tvStartAccumulate, m_tvEndAccumulate))
			CTraceStream(__FILE__, __LINE__, (WORD)m_eTraceLevel, m_pSenderThis).seekp(0, std::ios_base::cur)
				<< pszTitle << TimeOfDayToString(m_tvStartAccumulate) << " - " << TimeOfDayToString(m_tvEndAccumulate)
				<< " - " << m_sFunction.c_str() << "\n" << MSG.str().c_str();
	}

	Reset();
}

timeval CTraceAccumulator::Flush(const char* pszTitle, const char* pszFileName, const WORD lineNumber)
{
	timeval tvEnd;
	timerclear(&tvEnd);
	if (m_predicat == 0)
		FTRACESTRFUNC(m_eTraceLevel) << MSG.str().c_str();
	else
	{
		bool bLongTime = m_predicat(m_tvStartAccumulate, m_tvEndAccumulate);
		if (bLongTime || (timerisset(&m_tvStartAccumulate) && timercmp(&m_tvExplicit, &m_tvStartAccumulate, >)))
		{
			CTraceStream(pszFileName, lineNumber, (WORD)(bLongTime ? eLevelError : m_eTraceLevel), m_pSenderThis).seekp(0, std::ios_base::cur)
				<< pszTitle << TimeOfDayToString(m_tvStartAccumulate) << " - " << TimeOfDayToString(m_tvEndAccumulate)
				<< " - " << m_sFunction.c_str() << "\n" << MSG.str().c_str();
			tvEnd = m_tvEndAccumulate;
		}
	}

	Reset();
	return tvEnd;
}

std::ostringstream& CTraceAccumulator::AppendWithTime()
{
	timeval tv;
	gettimeofday(&tv, 0);
	if (m_tvStartAccumulate.tv_sec == 0)
		m_tvStartAccumulate = tv;
	m_tvEndAccumulate = tv;
	MSG << "\n" << TimeOfDayToString(tv).c_str();
	return MSG;
}
