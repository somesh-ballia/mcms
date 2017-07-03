// CommTCPDumpService.h

#ifndef COMMTCPDUMPSERVICE_H_
#define COMMTCPDUMPSERVICE_H_

#include "CommIPSListService.h"
#include "AllocateStructs.h"
#include "Macros.h"

class CIPService;

class CCommTCPDumpService : public CCommIPSListService
{
CLASS_TYPE_1(CCommTCPDumpService, CCommIPSListService)
public:
    CCommTCPDumpService(void);
    virtual const char* NameOf(void) const;
    virtual STATUS SendIpServiceParamInd(CIPService* srv);
    virtual STATUS SendIpServiceParamEndInd(void);
    virtual STATUS SendDelIpService(CIPService* srv);
	virtual STATUS SendServiceCfgList(CSegment *pSeg);

    //STATUS SendMngmIpService(DWORD ip, eIpType	 ipType) const;
    STATUS SendMngmIpService(DWORD ip, eIpType	 ipType, const char *str_Ipv6) const;

private:
    static const eProcessType kProcessType;

    void FillParams(IP_SERVICE_TCPDUMP_S& prm, CIPService* srv);

    DISALLOW_COPY_AND_ASSIGN(CCommTCPDumpService);
};

#endif  // COMMTCPDUMPSERVICE_H_
