// TraceStream.h: interface for the CTraceStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TRACESTREAM_H)
#define _TRACESTREAM_H


#include "NStream.h"
#include "DataTypes.h"

class CPObject;

class CTraceStream : public COstrStream
{
	CLASS_TYPE_1(CTraceStream, COstrStream)
public:
	CTraceStream(const char *fileName, const WORD lineNumber,
                 const WORD level, const CPObject * object = NULL);
	~CTraceStream();
	
	virtual const char* NameOf() const { return "CTraceStream";}
    
private:
    // disabled
    CTraceStream(const CTraceStream&);
    CTraceStream& operator=(const CTraceStream&);
    
    
	WORD m_level;
	const CPObject * m_pObject;
    const char *m_FileName;
    const WORD m_LineNumber;
    
};


#endif // !defined(_TRACESTREAM_H)
