#ifndef CSMNGRMPLMCMSPROTOCOLTRACER_H_
#define CSMNGRMPLMCMSPROTOCOLTRACER_H_

#include <ostream>
#include <map>
using namespace std;

#include "MplMcmsProtocolTracer.h"

class CCSMngrMplMcmsProtocolTracer;
class CMplMcmsProtocol;

typedef void (CCSMngrMplMcmsProtocolTracer::*DumpMethod)(std::ostream & ostr, const char *contentStr);
typedef map<OPCODE, DumpMethod> CDumpMethodMap;


class CCSMngrMplMcmsProtocolTracer : public CMplMcmsProtocolTracer
{
CLASS_TYPE_1(CCSMngrMplMcmsProtocolTracer, CMplMcmsProtocolTracer)
public:
	CCSMngrMplMcmsProtocolTracer();
	CCSMngrMplMcmsProtocolTracer(CMplMcmsProtocol &mplMcmsProt);
	virtual ~CCSMngrMplMcmsProtocolTracer();

	virtual void TraceContent(CObjString* pContentStr, eProcessType processType = eProcessTypeInvalid);
	
private:	
	void DumpCsNewInd			(std::ostream & ostr, const char *contentStr);
	void DumpConfigParamsInd	(std::ostream & ostr, const char *contentStr);
	void DumpEndConfigParamsInd	(std::ostream & ostr, const char *contentStr);
	void DumpEndCsStartupInd	(std::ostream & ostr, const char *contentStr);
	void TraceCSBuffer			(std::ostream & ostr, const char *contentStr);
	void DumpKeepAliveInd		(std::ostream & ostr, const char *contentStr);
	void DumpCsNewReq			(std::ostream & ostr, const char *contentStr);
	void DumpConfigParamsReq	(std::ostream & ostr, const char *contentStr);
	void DumpEndConfigParamsReq	(std::ostream & ostr, const char *contentStr);	
	void DumpCsLanCfgReq		(std::ostream & ostr, const char *contentStr);
    void DumpPingInd            (std::ostream & ostr, const char *contentStr);
    void TracePingReq           (std::ostream & ostr, const char *contentStr);

	void InitDumpMethodMap();
	
	CDumpMethodMap m_DumpMethodMap;		
};

#endif /*CSMNGRMPLMCMSPROTOCOLTRACER_H_*/
