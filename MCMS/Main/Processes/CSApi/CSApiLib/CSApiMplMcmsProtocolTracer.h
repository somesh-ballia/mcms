#ifndef CSAPIMPLMCMSPROTOCOLTRACER_H_
#define CSAPIMPLMCMSPROTOCOLTRACER_H_

#include "MplMcmsProtocolTracer.h"


class CMplMcmsProtocol;


class CCSApiMplMcmsProtocolTracer : public CMplMcmsProtocolTracer
{
CLASS_TYPE_1(CCSApiMplMcmsProtocolTracer, CMplMcmsProtocolTracer)	
public:
	CCSApiMplMcmsProtocolTracer();
	CCSApiMplMcmsProtocolTracer(CMplMcmsProtocol &mplMcmsProt);
	virtual ~CCSApiMplMcmsProtocolTracer();
	
//	virtual void TraceContent(CObjString* pContentStr, eProcessType processType = eProcessTypeInvalid);
	
};

#endif /*CSAPIMPLMCMSPROTOCOLTRACER_H_*/
