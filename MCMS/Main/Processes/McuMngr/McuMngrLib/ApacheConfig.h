// ApacheConfig.h

#ifndef APACHE_CONFIG_H_
#define APACHE_CONFIG_H_

#include "PObject.h"
#include "DataTypes.h"
#include "CommonStructs.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ProcessBase.h"
#include <list>
#include <map>

typedef struct
{
 	std::string m_aliasName;
 	std::string m_internalIP;
 	std::string m_sslport;
 	std::string m_userName;
 	std::string m_password;
}RvgwItem;
class CApacheConfig: public CPObject
{
CLASS_TYPE_1(CApacheConfig, CPObject)
public:
	CApacheConfig(void);
	virtual ~CApacheConfig(void);
	virtual const char* NameOf(void) const;
	virtual void Dump(std::ostream& msg) const;

	void GenerateConfigFile(void) const;

	BOOL IsV35UseSSLSockets(void) const;
	BOOL m_isPermanentNetworkOpen;
	BOOL m_isSecured;
	eMangmentSecurityProtocol m_SSLProtocol; //Configure usable SSL/TLS protocol versions
	BOOL m_isExternalContentSupported;
	BOOL m_isRequestPeerCertificate;
	BOOL m_isV35GWEnabled;
	DWORD m_externalContentPort;
	DWORD m_KeepAliveTimeout;
	DWORD m_MaxKeepAliveRequests;
	eIpType m_ipType;
	std::string m_externalContentDir;
	std::string m_externalContentIp;
	std::string m_IpAddressToConfig_ipV4;
	std::string m_IpAddressToConfig_ipV6;
	std::string m_internalGWAddress;
    list<RvgwItem>  m_v35GwList;

  	BOOL    		m_isUseResponderOcspURI;
  	BOOL    		m_isIncompleteRevocation;
  	BOOL    		m_isSkipValidateOcspCert;
  	BYTE    		m_revocationMethodType;
  	std::string	 m_ocspGlobalResponderURI;
  	std::string	 m_strHostName;
  	DWORD  m_ocspResponderTimeout;

  	map< eMangmentSecurityProtocol ,std::string> m_manSecurityProtocol;
private:
  void GenerateClientCfgFile(void) const;
  void GenerateListenConfiguration(void) const;
	void GenerateSecureConfiguration(std::ostringstream &out,std::string port) const;
  void GenerateRESTListenConfiguration(void) const;
	static BOOL IsUltraSecureMode(void);
	static const char* YesNo(bool val);
	static const char* OnOff(bool val);
	static const char* GetCARevocationCheck(BYTE revocationMethodType);
	static const char* CommentIf(bool condition);	
	std::string GetApacheLogLevel(void) const;
};

#endif  // APACHE_CONFIG_H_
