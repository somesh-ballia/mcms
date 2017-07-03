// CommCsModuleService.h: interface for the CCommCsModuleService class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __COMMCSMODULESERVICE_H__
#define __COMMCSMODULESERVICE_H__


#include "CommService.h"


class CCSMngrMplMcmsProtocolTracer;


class CCommCsModuleService : public CCommService  
{
public:
	CCommCsModuleService();
	virtual ~CCommCsModuleService();
	
	void SetMplMcmsProtocolTracer(CCSMngrMplMcmsProtocolTracer *tracer){m_CSMngrMplMcmsProtocolTracer = tracer;}
	
protected:
	STATUS SendToCsApi(OPCODE opcode, int destUnitId, const int dataLen, const char * data, const int csId);
	virtual STATUS SendToCsApi(OPCODE opcode, const int dataLen, const char * data) = 0;
	
private:
	CCSMngrMplMcmsProtocolTracer *m_CSMngrMplMcmsProtocolTracer;	
};

#endif // __COMMCSMODULESERVICE_H__
