/*
 * TraceAccumulator.h
 *
 *  Created on: Mar 19, 2014
 *      Author: bguelfand
 */

#ifndef TRACEACCUMULATOR_H_
#define TRACEACCUMULATOR_H_

#include "Trace.h"
#include <sstream>

class CTraceAccumulator
{
public:
	CTraceAccumulator();
	CTraceAccumulator(bool (*predicat)(timeval&, timeval&),
			const std::string& sFunction, const CPObject* pSenderThis, eLogLevel traceLevel = eLevelInfoNormal);
	~CTraceAccumulator();

	void Reset();
	void Flush(const char* pszTitle);
	timeval Flush(const char* pszTitle, const char* pszFileName, const WORD lineNumber);
	std::ostringstream& AppendWithTime();
	std::ostringstream MSG;
	long GetTimesDiff() {return (m_tvEndAccumulate.tv_sec - m_tvStartAccumulate.tv_sec) * 1000000L + m_tvEndAccumulate.tv_usec - m_tvStartAccumulate.tv_usec;}
	void SetExplicitTime(timeval& tvExplicit) {m_tvExplicit = tvExplicit;}
private:
	bool (*m_predicat)(timeval&, timeval&);
	eLogLevel	m_eTraceLevel;
	timeval		m_tvStartAccumulate;
	timeval		m_tvEndAccumulate;
	timeval		m_tvExplicit;
	std::string	m_sFunction;
	const CPObject*	m_pSenderThis;
};

#endif /* TRACEACCUMULATOR_H_ */
