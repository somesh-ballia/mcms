#ifndef GKMPLMCMSPROTOCOLTRACER_H_
#define GKMPLMCMSPROTOCOLTRACER_H_

#include "MplMcmsProtocolTracer.h"

class CMplMcmsProtocol;

class CGkMplMcmsProtocolTracer : public CMplMcmsProtocolTracer
{
CLASS_TYPE_1(CGkMplMcmsProtocolTracer, CMplMcmsProtocolTracer)	
public:
	CGkMplMcmsProtocolTracer(CMplMcmsProtocol &mplMcmsProt);
	virtual ~CGkMplMcmsProtocolTracer();
	
	virtual const char* NameOf() const { return "CGkMplMcmsProtocolTracer";}
	virtual void TraceContent(CObjString* pContentStr, eProcessType processType = eProcessTypeInvalid);
	
	// GK prints
	void PrintGkFs(h460FsSt fs, CObjString *pContentStr);	
	void PrintAltGkInfo(altGksListSt* pListStFromGk, CObjString *pContentStr);
	void PrintGkRejectInfo(rejectInfoSt* pRejectInfo, CObjString *pContentStr);	
	void PrintAltGkList(altGksListSt* pAltGkList, CObjString *pContentStr);
	void TraceGkIdent(CObjString *pContentStr, const char* gatekeeperIdent, unsigned int gkIdentLength);
	void TraceEpIdent(CObjString *pContentStr, const char* epIdent, unsigned int epIdentLength);	
	void TraceRasGRQReq(CObjString *pContentStr);
	void TraceRasGRQInd(CObjString *pContentStr);
	void TraceRasRRQReq(CObjString *pContentStr);
	void TraceRasRRQInd(CObjString *pContentStr);
	void TraceRasURQReq(CObjString *pContentStr);
    void TraceRasRAIReq(CObjString *pContentStr);
    
	void TraceRasURQInd(CObjString *pContentStr);
	void TraceRasTimeoutInd(CObjString *pContentStr);
	void TraceRasFailInd(CObjString *pContentStr);
	void TraceRasGkURQInd(CObjString *pContentStr);
	void TraceRasGkDRQInd(CObjString* pContentStr);
	void TraceRasGkLRQInd(CObjString* pContentStr);
	void TraceRasARQReq(CObjString *pContentStr);
	void TraceRasARQInd(CObjString *pContentStr);
	void TraceRasDRQReq(CObjString *pContentStr);
	void TraceRasDRQInd(CObjString *pContentStr);
	void TraceRasURQResponseReq(CObjString *pContentStr);
	void TraceRasDRQResponseReq(CObjString *pContentStr);	
	void TraceRasLRQResponseReq(CObjString *pContentStr);	
	void TraceRasBRQReq(CObjString *pContentStr);	
	void TraceRasBRQInd(CObjString *pContentStr);	
	void TraceRasRACInd(CObjString *pContentStr);
	void TraceRasIrrResponseReq(CObjString *pContentStr);
};


#endif /*GKMPLMCMSPROTOCOLTRACER_H_*/
