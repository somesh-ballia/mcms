// EMATrace.h

#ifndef EMA_TRACE_H_
#define EMA_TRACE_H_

#include "SerializeObject.h"
#include "TraceHeader.h"
#include "LoggerDefines.h"

class CEMATrace : public CSerializeObject
{
public:
	CEMATrace();
	virtual ~CEMATrace();
	virtual CSerializeObject* Clone(){return new CEMATrace;}
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual const char* NameOf() const { return "CEMATrace";}
	virtual int	 DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action);

	TRACE_HEADER_S* GetTraceHeader(void) { return &m_TraceHeader; }
	const char* GetContent(void) const { return m_Content; }
	
private:
	TRACE_HEADER_S m_TraceHeader;
	char m_Content[MAX_CONTENT_SIZE];

	DISALLOW_COPY_AND_ASSIGN(CEMATrace);
};

#endif  // EMA_TRACE_H_
