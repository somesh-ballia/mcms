// CommMcmsService.h: interface for the CCommMcmsService class.

#ifndef __COMMMCMSSERVICE_H__
#define __COMMMCMSSERVICE_H__

#include "Macros.h"
#include "CommService.h"
#include "McmsProcesses.h"

class CIPService;
class CIPServiceList;

class CCommMcmsService : public CCommService  
{
CLASS_TYPE_1(CCommMcmsService, CCommService)
public:
	CCommMcmsService(void);
	virtual const char* NameOf(void) const;
	
protected:
	STATUS SendToMcmsProcess(eProcessType dest, OPCODE opcode, CSegment* data) const;
	STATUS SendDelIpServiceToMcmsProcess(eProcessType dest, OPCODE opcode, CIPService *service);
	CIPService* GetDynamicIpServiceIncListCnt(WORD serviceId)const;
	
	virtual STATUS SendDelIpService(CIPService *service) = 0;

private:
	DISALLOW_COPY_AND_ASSIGN(CCommMcmsService);
};

#endif  // __COMMMCMSSERVICE_H__
