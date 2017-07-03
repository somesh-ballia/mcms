// TraceStream.cpp

#include "TraceStream.h"
#include "Trace.h"

CTraceStream::CTraceStream(const char* fname,
                           const WORD line,
                           const WORD level,
                           const CPObject* object) :
  COstrStream(),
  m_level(level),
  m_pObject(object),
  m_FileName(fname),
  m_LineNumber(line)
{}

CTraceStream::~CTraceStream()
{
  OutTraceMessage(m_FileName,
                  m_LineNumber,
                  m_level,
                  m_pObject,
                  str().c_str(),
                  NULL);
}
