// DNSAgentProcess.h: interface for the CDNSAgentProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DNSAgentPROCESS_H__)
#define _DNSAgentPROCESS_H__

#include "ProcessBase.h"
#include "DnsRecordsMngr.h"

class CDNSAgentProcess : public CProcessBase  
{
CLASS_TYPE_1(CDNSAgentProcess,CProcessBase )
public:
	friend class CTestDNSAgentProcess;

	CDNSAgentProcess();
	virtual ~CDNSAgentProcess();
	virtual eProcessType GetProcessType() {return eProcessDNSAgent;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
    virtual BOOL HasMonitorTask() {return FALSE;}

    DnsRecordsMngr* GetDnsRecordsMngr() { return &m_dnsRecordsManager; }

protected:
    DnsRecordsMngr m_dnsRecordsManager;
};

#endif // !defined(_DNSAgentPROCESS_H__)

