// ConfiguratorManager.h

#ifndef CONFIGURATOR_MANAGERSOFTMCU_H_
#define CONFIGURATOR_MANAGERSOFTMCU_H_

#include "ManagerTask.h"
#include "Macros.h"
#include "ConfigManagerApi.h"
#include "ProductTypeDecider.h"
#include "ConfiguratorManager.h"

void ConfiguratorManagerEntryPoint(void* appParam);

class CConfiguratorManagerSoftMcu : public CConfiguratorManager
{
CLASS_TYPE_1(CConfiguratorManagerSoftMcu, CConfiguratorManager)
public:
	CConfiguratorManagerSoftMcu(void);
	~CConfiguratorManagerSoftMcu(void);


    virtual void RestartSSH(CSegment* pSeg);
    virtual void KillSsh(CSegment* seg);

    virtual void EnableDHCPIPv6(CSegment* pSeg);
    virtual void DisableDHCPIPv6(CSegment* pSeg);

    void HandleNtpService(CSegment* seg);
    void ConfigNtpServers(CSegment* seg);


	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

private:
	STATUS	SSHTerminalRequest(CTerminalCommand & command, std::ostream& answer);
  
  DISALLOW_COPY_AND_ASSIGN(CConfiguratorManagerSoftMcu);
};

#endif // !defined(CONFIGURATOR_MANAGERSOFTMCU_H_)
