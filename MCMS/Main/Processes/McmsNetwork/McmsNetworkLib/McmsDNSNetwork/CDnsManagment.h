/*
 * CDnsManagment.h
 *
 *  Created on: Oct 7, 2013
 *      Author: stanny
 */

#ifndef CDNSMANAGMENT_H_
#define CDNSMANAGMENT_H_

#include "SharedMcmsCardsStructs.h"
#include "CardsStructs.h"
#include "McuMngrStructs.h"
#include "ConfigManagerApi.h"
#include "IpParameters.h"
#include "DefinesGeneral.h"
#include "IpService.h"
#include "CNetworkSettings.h"
#include "NetCommonDefines.h"


namespace McmsNetworkPackage {

class CDnsManagment  : public CPObject {
	CLASS_TYPE_1(CDnsManagment, CPObject)
protected:
	CIpDns*  m_pMngmntDns;
	CIPService* m_pMngmntService;
	eDnsConfigurationStatus		m_statusDnsConfig;
	// =====  RegistrationMode: Auto
	virtual STATUS ConfigDnsInOS();
	virtual STATUS ConfigDnsAuto();
	virtual STATUS ConfigDnsSpecify();
	virtual STATUS ConfigDnsOff();
	virtual STATUS UpdateDnsService(CIPService* pService){return STATUS_OK;};
	virtual STATUS ConfigureDnsServers(const std::string& search,const std::string& dns1,const std::string& dns2,const std::string& dns3);
	virtual STATUS NameServerUpdate(const std::string& dns, const std::string& host,const std::string& zone,const std::string& ip,const std::string& ipv6);
	virtual std::string GetCmdLinePrefix() {return "";};
	virtual STATUS RegisterDnsClient();

public:
	CDnsManagment();
	virtual ~CDnsManagment();
	virtual STATUS StartMngmtDNSConfig(CIPService *pService);
	virtual STATUS WriteMngmtDnsNetwork(CNetworkSettings& netSettings);
	virtual const char* NameOf() const { return "CDnsManagment";}

};

class CDnsSoftMcuMfwEdgeAxis  : public CDnsManagment {
	CLASS_TYPE_1(CDnsSoftMcuMfwEdgeAxis, CDnsManagment)
protected:
	BOOL  m_isResolvedConfig;
	STATUS UpdateDnsService(CIPService* pService);
	void   UpdateLocalDomain(std::string& line,int Ind,int tokenLen,CIPService* pService);
	void    UpdateNameServerInstances(std::string& line,int Ind,CIPService* pService);
	bool TestMcmsUser();
public:
	CDnsSoftMcuMfwEdgeAxis();
	virtual ~CDnsSoftMcuMfwEdgeAxis();
	const char* NameOf() const { return "CDnsSoftMcuMfwEdgeAxis";}
};

} /* namespace McmsNetworkPackage */
#endif /* CDNSMANAGMENT_H_ */
