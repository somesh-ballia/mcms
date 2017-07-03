#ifndef SIPPROXYMPLMCMSPROTOCOLTRACER_H_
#define SIPPROXYMPLMCMSPROTOCOLTRACER_H_

#include "MplMcmsProtocolTracer.h"

class CMplMcmsProtocol;

class CSipProxyMplMcmsProtocolTracer : public CMplMcmsProtocolTracer
{
CLASS_TYPE_1(CSipProxyMplMcmsProtocolTracer, CMplMcmsProtocolTracer)	
public:
	CSipProxyMplMcmsProtocolTracer(CMplMcmsProtocol &mplMcmsProt);
	virtual ~CSipProxyMplMcmsProtocolTracer();
	
	virtual void TraceContent(CObjString* pContentStr, eProcessType processType = eProcessTypeInvalid);
	
	void TraceProxyRegisterReq(CObjString* pContentStr);
	void TraceProxyRegisterResponseInd(CObjString *pContentStr);
	void TraceSipTraceInfoInd(CObjString *pContentStr);
	void TraceProxyMsKeepAlive(CObjString* pContentStr);
	void TraceProxyMsKeepAliveErrInd(CObjString* pContentStr);

	
};
#endif /*SIPPROXYMPLMCMSPROTOCOLTRACER_H_*/
