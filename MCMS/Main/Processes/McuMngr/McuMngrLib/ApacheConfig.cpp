// ApacheConfig.cpp

#include "ApacheConfig.h"

#include <iostream>

#include "SystemFunctions.h"
#include "TraceStream.h"
#include "OsFileIF.h"
#include "SslFunc.h"
#include "Segment.h"
#include "ManagerApi.h"
#include "ConfigManagerOpcodes.h"
#include "DefinesGeneral.h"

CApacheConfig::CApacheConfig() :
  m_isPermanentNetworkOpen(TRUE),
  m_isSecured(FALSE),
  m_SSLProtocol(eTLSV1_SSLV3),
  m_isExternalContentSupported(FALSE),
  m_isRequestPeerCertificate(FALSE),
  m_isV35GWEnabled(FALSE),
  m_externalContentPort(80),
  m_KeepAliveTimeout(15),
  m_MaxKeepAliveRequests(0),
  m_ipType(eIpType_IpV4),
  m_isUseResponderOcspURI(FALSE),
  m_isIncompleteRevocation(TRUE),
  m_isSkipValidateOcspCert(TRUE),
  m_revocationMethodType(eNoneMethod),
  m_ocspGlobalResponderURI(""),
  m_strHostName(""),
  m_ocspResponderTimeout(3)
{
	m_manSecurityProtocol[eTLSV1_SSLV3] = "-all +TLSv1 +SSLv3\n";
	m_manSecurityProtocol[eTLSV1] = "-all +TLSv1 \n";
}
CApacheConfig::~CApacheConfig()
{}

// Virtual
const char* CApacheConfig::NameOf() const
{
  return GetCompileType();
}

// Static, Private
const char* CApacheConfig::YesNo(bool val)
{
  return val ? "yes" : "no";
}

const char* CApacheConfig::OnOff(bool val)
{
  return val ? "on" : "off";
}

const char* CApacheConfig::GetCARevocationCheck(BYTE revocationMethodType)
{
	if (revocationMethodType == eNoneMethod || revocationMethodType == eOcsp)
	{
		return "none";
	}
	if (revocationMethodType == eCrl )
	{
		return "chain";
	}
	return "none";
}


const char* CApacheConfig::CommentIf(bool condition)
{
	return condition ? "#" : "";
}

void CApacheConfig::Dump(std::ostream& out) const
{
  out << "CApacheConfig\n" << "Protocol type: ";

  switch (m_ipType)
  {
  case eIpType_None:
    out << "none\n";
    break;

  case eIpType_IpV4:
    out << "IPv4\n"
        << "IPv4 address: " << m_IpAddressToConfig_ipV4 << "\n";
    break;

  case eIpType_IpV6:
    out << "IPv6\n"
        << "IPv6 address: " << m_IpAddressToConfig_ipV6 << "\n";
    break;

  case eIpType_Both:
    out << "both\n"
        << "IPv4 address: " << m_IpAddressToConfig_ipV4 << "\n"
        << "IPv6 address: " << m_IpAddressToConfig_ipV6 << "\n";
    break;

  default:
    out << "unknown: " << (int) m_ipType << "\n";
  }

  out << "Secured management (TLS): " << YesNo(m_isSecured) << "\n"
      << "Request Peer Certificate: " << YesNo(m_isRequestPeerCertificate) << "\n"
      << "Revokation Method: " << (int)m_revocationMethodType  << "  ( eNoneMethod = 0, eNoneMethod = 1, eCrl = 2, eOcsp = 3 )" << "\n"
      << "Incomplete Revokation: " << YesNo(m_isIncompleteRevocation) << "\n"
      << "Permanent network: " << YesNo(m_isPermanentNetworkOpen) << "\n"
      << "External Content: " << YesNo(m_isExternalContentSupported) << "\n"
      << "Keep Alive Timeout: " << m_KeepAliveTimeout << "\n"
      << "Max Keep Alive Requests: " << m_MaxKeepAliveRequests << "\n"
      << "V35 Gateway: " << YesNo(m_isV35GWEnabled) << "\n"
      << "Internal GW address: " << m_internalGWAddress << "\n";

  if (m_isExternalContentSupported)
  {
    out << "External Content Dir: " << m_externalContentDir << "\n"
        << "External Content IP: " << m_externalContentIp << "\n"
        << "External Content Port: " << m_externalContentPort << "\n";
  }

  if (m_revocationMethodType == eOcsp)
  {
	  out << "Always Use GlobalOCSP Responder: " << YesNo(m_isUseResponderOcspURI)<< "\n"
		  << "OSCP Global ResponderURI: " << m_ocspGlobalResponderURI << "\n"
	  	  << "Skip Validate OCSP Certificate: " << YesNo(m_isSkipValidateOcspCert) << "\n";
	      out<<"OCSPResponderTimeout: " << m_ocspResponderTimeout << "\n";

  }
     out<<"ServerName: " << m_strHostName << "\n";

}

// Private
void CApacheConfig::GenerateClientCfgFile() const
{
  std::ostringstream out;

  out << "<CLIENT_CFG>\n"
      << "\t<REQUEST_PEER_CERTIFICATE>"
      << (m_isRequestPeerCertificate ? "true" : "false")
      << "</REQUEST_PEER_CERTIFICATE>\n"
      << "\t<JITC_MODE>" << (IsUltraSecureMode() ? "true" : "false") << "</JITC_MODE>\n"
      << "</CLIENT_CFG>";

  // Writes configuration file with root permissions
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERTMSG_AND_RETURN(NULL == proc, "Unable to continue");
  CSegment* seg = new CSegment;

  std::string fname = MCU_TMP_DIR+"/ClientCfg.xml";
  *seg << fname.c_str() << out.str();

  OPCODE opcode;
  CSegment ret_seg;
  CManagerApi api(eProcessConfigurator);
  STATUS stat = api.SendMessageSync(seg,
                                    CONFIGURATOR_WRITE_FILE,
                                    1 * SECOND,
                                    opcode,
                                    ret_seg);

  PASSERTSTREAM_AND_RETURN(stat != STATUS_OK,
      "Unable to send " << proc->GetOpcodeAsString(CONFIGURATOR_WRITE_FILE)
      << " (" << CONFIGURATOR_WRITE_FILE
      << ") to " << ProcessNames[eProcessConfigurator]
      << ": " << proc->GetStatusAsString(stat));

  PASSERTSTREAM(opcode != STATUS_OK,
      "Unable to write file " << fname.c_str()
      << ": " << proc->GetStatusAsString(opcode));
}

// mod_ssl configuration
void CApacheConfig::GenerateSecureConfiguration(std::ostringstream &out,std::string port) const
{
  std::map<eMangmentSecurityProtocol, std::string>::const_iterator prot =
    m_manSecurityProtocol.find(m_SSLProtocol);
  PASSERT_AND_RETURN(prot == m_manSecurityProtocol.end());

  size_t loopCount = 0u;
  std::list<RvgwItem>::const_iterator it=m_v35GwList.begin();
  do
  {
    out << "<VirtualHost _default_:" << port << ">" << "\n"
        << "SSLEngine on\n"
        << "SSLCipherSuite ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:ECDHE-RSA-DES-CBC3-SHA:ECDHE-ECDSA-DES-CBC3-SHA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!DES:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA\n"
        << "SSLHonorCipherOrder on\n"
        << "SSLCompression off\n"
        << "SSLCertificateFile " << CERTF << "\n"
        << "SSLCertificateKeyFile " << COMMON_KEY_F << "\n"
        << "SSLProtocol " << prot->second << "\n";

    if (m_isRequestPeerCertificate)
    {
      out << "SSLVerifyClient require\n"
          << "SSLVerifyDepth 10\n"
          << "SSLCACertificateFile " << CA_CERTIFICATES_FILE << "\n";

      if (eCrl == m_revocationMethodType)
        out << "SSLCARevocationFile " << CRL_CA_FILE << "\n";

      out << "SSLOCSPEnable " << OnOff(eOcsp == m_revocationMethodType) << "\n"
          << "SSLCARevocationCheck " << GetCARevocationCheck(m_revocationMethodType) << "\n";
     }
     else
     {
       out << "#SSLVerifyClient require\n"
           << "#SSLVerifyDepth 10\n"
           << "#SSLCACertificateFile " << CA_CERTIFICATES_FILE << "\n"
           << "#SSLCARevocationFile " << CRL_CA_FILE << "\n"
           << "#SSLOCSPEnable " << "off" << "\n"
           << "#SSLCARevocationCheck " << "none" << "\n"
           << "#SSLCARevocationFile " << CRL_CA_FILE << "\n"
           << "#SSLCARevocationCheck " << "none" << "\n";
      }

      // ocsp configuration
      out	<< CommentIf(!m_isRequestPeerCertificate || (eOcsp != m_revocationMethodType) || m_ocspGlobalResponderURI.length() == 0) << "SSLOCSPDefaultResponder " << m_ocspGlobalResponderURI << "\n"
		    	<< CommentIf(!m_isRequestPeerCertificate || (eOcsp != m_revocationMethodType)|| m_ocspGlobalResponderURI.length() == 0) << "SSLOCSPOverrideResponder " << OnOff(!m_isUseResponderOcspURI) << "\n"
		    	<< CommentIf(!m_isRequestPeerCertificate || (eNoneMethod == m_revocationMethodType)) << "SSLPolycomOCSPLooseRevokation " << OnOff(m_isIncompleteRevocation) << "\n"
		    	<< CommentIf(!m_isRequestPeerCertificate || (eOcsp != m_revocationMethodType)) << "SSLPolycomOCSPSkipValidateCert " << OnOff(m_isSkipValidateOcspCert) << "\n"
		        << CommentIf(!m_isRequestPeerCertificate || (eOcsp != m_revocationMethodType)) << "SSLOCSPResponderTimeout " << m_ocspResponderTimeout << "\n";

		    CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		    BOOL isOCSPNoNonce = YES;
		    sysConfig->GetBOOLDataByKey(CFG_OCSP_NO_NONCE, isOCSPNoNonce);

		    out	<< CommentIf(!m_isRequestPeerCertificate || (eOcsp != m_revocationMethodType) || !isOCSPNoNonce) << "SSLPolycomOCSPNoNonce " << OnOff(isOCSPNoNonce) << "\n";


		    // Next block is for Port Consolidation with REST API

		    out << "RewriteEngine On\n"
			<< "RewriteCond %{REQUEST_URI} !.*/plcm/.*$\n"
			<< "RewriteCond %{REQUEST_URI} !.*/api/rest/.*$\n"
			<< "RewriteRule .? - [S=4]\n"
			<< "RewriteCond %{REQUEST_FILENAME} !-f\n"
			<< "RewriteCond %{REQUEST_FILENAME} !-d\n"
			<< "RewriteCond %{REQUEST_URI} !.*/WebHelp/.*$\n"
			<< "RewriteRule ^(.+) " << MCU_MCMS_DIR << "/EMA/htdocs/PHPUtils/dispatch.php$1 [L,QSA]\n"
			<< "RewriteRule ^(.+) " << MCU_MCMS_DIR << "/EMA$1 [L,QSA]\n"
			<< "RewriteCond %{REQUEST_METHOD} !^(GET|POST|HEAD)$\n"
			<< "RewriteRule .* - [F]\n";

		    out << "</VirtualHost>\n";
				loopCount++;
				if(it != m_v35GwList.end())
				{
					RvgwItem item = (*it);
					port = item.m_sslport;
					it++;
				}
			}
			while(loopCount < (1 + m_v35GwList.size()));
}

void CApacheConfig::GenerateRESTListenConfiguration() const
{
	 std::ostringstream out;
	 const char* port;
	   eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	   if ((eProductTypeSoftMCUMfw == curProductType))
	   {
	 	  CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	 	  const CCfgData *cfgPort = sysConfig->GetCfgEntryByKey("REST_API_PORT");
	 	  const char* rest_port;
	 	  if(NULL != cfgPort)
	 	  {
	 		  const std::string &cfgPortVal = cfgPort->GetData();
	 		  rest_port=cfgPortVal.c_str();
	 		  TRACEINTOFUNC << "custom rest api port is " << rest_port;
	 		  //printf("\nCApacheConfig::GenerateListenConfiguration -http_port:%s\n",http_port);
	 		 out << "Listen " << rest_port <<"\n";

	 		 CProcessBase* proc = CProcessBase::GetProcess();
	 		  PASSERT_AND_RETURN(NULL == proc);

	 		  CSegment* seg = new CSegment;
	 		  std::string fname = MCU_TMP_DIR+"/httpd.rest.conf";
	 		  *seg << fname.c_str() << out.str();

	 		  OPCODE opcode;
	 		  CSegment ret_seg;
	 		  CManagerApi api(eProcessConfigurator);
	 		  STATUS stat = api.SendMessageSync(seg,
	 		                                    CONFIGURATOR_WRITE_FILE,
	 		                                    1 * SECOND,
	 		                                    opcode,
	 		                                    ret_seg);

	 		  PASSERTSTREAM_AND_RETURN(stat != STATUS_OK,
	 		      "Unable to send " << proc->GetOpcodeAsString(CONFIGURATOR_WRITE_FILE)
	 		      << " (" << CONFIGURATOR_WRITE_FILE
	 		      << ") to " << ProcessNames[eProcessConfigurator]
	 		      << ": " << proc->GetStatusAsString(stat));

	 		  PASSERTSTREAM(opcode != STATUS_OK,
	 		      "Unable to write file " << fname.c_str()
	 		      << ": " << proc->GetStatusAsString(opcode));
	 	   }
	   }
}

void CApacheConfig::GenerateListenConfiguration() const
{
  std::ostringstream out;
  int num_of_listen_address = 0;
  std::string fname;

  const char* port;
  eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
  port = m_isSecured ? "443" : "80";

  if (eProductTypeSoftMCUMfw == curProductType)
  {
	  CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	  const CCfgData *cfgPort = sysConfig->GetCfgEntryByKey("XML_API_PORT");
	  const char* http_port;
	  if(NULL != cfgPort)
	  {
		  const std::string &cfgPortVal = cfgPort->GetData();
		  http_port=cfgPortVal.c_str();
		  TRACEINTOFUNC << "custom xml api port is " << http_port;
	   }
	  const CCfgData*cfgHttpsPort = sysConfig->GetCfgEntryByKey("XML_API_HTTPS_PORT");
	  const char* https_port;
	  if(NULL != cfgHttpsPort)
	  {
		  const std::string &cfgHTTPSPortVal = cfgHttpsPort->GetData();
		  https_port= cfgHTTPSPortVal.c_str();
		  TRACEINTOFUNC << "custom https xml api port is " << https_port;
	   }
	  if (cfgHttpsPort != NULL && cfgPort != NULL)
		  port = m_isSecured ? https_port : http_port;
	  else {
		  TRACEINTOFUNC << "MFW port no /mcu_custom_config/custom.cfg entry found using default port:" << port;
	  }
  }
  
  // Permanent network
  if (m_isPermanentNetworkOpen)
  {
    out << "Listen 169.254.192.10:" << port << "\n";
    num_of_listen_address++;
  }

  if (m_strHostName=="")
  {
	  out << "ServerName PolycomMCU" << "\n";
  }
  else
  {
	  out << "ServerName " << m_strHostName << "\n";
  }

  // Permanent networkIPv4
  if (m_ipType == eIpType_IpV4 || m_ipType == eIpType_Both)
  {
    std::string ipv4 = m_IpAddressToConfig_ipV4;
    if (!ipv4.empty() && "0.0.0.0" != ipv4)
    {
      // Fixes the address for simulation
      if (!IsTarget() && "127.0.0.1" == ipv4)
        ipv4 = "0.0.0.0";

      out << "#IPv4\n"
          << "Listen " << ipv4 << ":" << port << "\n";
      if (eProductTypeEdgeAxis == curProductType && !m_isSecured)
    	  //out  << "Listen " << ipv4 << ":80\n";
      //for RVGW
     	      if(IsV35UseSSLSockets())
     	      {
     			  list<RvgwItem>::const_iterator it=m_v35GwList.begin();
     			  for(;it != m_v35GwList.end();it++)
     			  {
     				  RvgwItem item = (*it);
     				  out << "Listen " << ipv4 << ":" << item.m_sslport
     								  << std::endl;
     			  }
     	      }
      //debug only remove
    //  out << "#IPv4\n"
      //         << "Listen " << ipv4 << ":" << "8080" << "\n";
      TRACEINTOFUNC << "IPv4 address is " << ipv4 << ":" << port;

      num_of_listen_address++;
    }
    else
    {
      TRACEINTOFUNC << "IPv4 address is empty";
    }
  }

  // Permanent networkIPv6
  if (m_ipType == eIpType_IpV6 || m_ipType == eIpType_Both)
  {
    std::string ipv6 = m_IpAddressToConfig_ipV6;
    if (!ipv6.empty() && "::" != ipv6)
    {
      // Trims possible brackets
      if ('[' == *ipv6.begin() && ']' == *ipv6.rbegin())
      {
        ipv6.erase(ipv6.begin());
        ipv6.erase(ipv6.end() - 1);
      }

      out << "#IPv6\n"
          << "Listen " << "[" << ipv6 << "]:" << port << "\n";
      //for RVGW ssl ports
      	      if(IsV35UseSSLSockets())
      	      {
      	    	  list<RvgwItem>::const_iterator it=m_v35GwList.begin();
      	    	  for(;it != m_v35GwList.end();it++)
      	    	  {
      	    	    	  RvgwItem item = (*it);
      	    	       	  out << "#ipv6" << std::endl << "Listen " << "[" << ipv6 << "]:" << item.m_sslport
      	    	        	 	          << std::endl;
      	    	  }
      	      }
      TRACEINTOFUNC << "IPv6 address is " << "[" << ipv6 << "]:" << port;

      num_of_listen_address++;
    }
    else
    {
      TRACEINTOFUNC << "IPv6 address is empty";
    }
  }

  out << "#PID FILE\n"
      << "<IfModule !mpm_netware.c>\n";
  if (!IsTarget() || eProductTypeGesher == curProductType || ( eProductTypeNinja == curProductType))
  {
	 out << "PidFile " << MCU_TMP_DIR << "/httpd.pid\n";
  }
  else
  {
         out << "PidFile " << MCU_VAR_DIR << "/run/httpd.pid\n";
  }
  out<< "</IfModule>\n";

  // Fallback configuration
  if (num_of_listen_address == 0)
  {
    PASSERTMSG(true, "Fallback Apache listen configuration");
    if(IsRmxSimulation())
    {
		out << "#FALLBACK LISTEN CONFIGURATION\n"
			<< "Listen 8080\n";
    }
    else
    {
    	out << "#FALLBACK LISTEN CONFIGURATION\n"
    			<< "Listen 80\n";
    }
  }

  
#ifdef _X86_PLATFORM
	out   << "#Proxy mode for Shelf\n"
	      << "ProxyPass /SHM http://127.0.0.1:8090\n"
	      << "ProxyPassReverse /SHM http://127.0.0.1:8090\n";
#else
  out   << "#Proxy mode for Shelf\n"
        << "ProxyPass /SHM http://169.254.128.16\n"
        << "ProxyPassReverse /SHM http://169.254.128.16\n";  
#endif

  if (m_isSecured)
  {
    out << "<IfModule mod_ssl.c>\n"
        << "SSLPassPhraseDialog exec:" << SSL_PASS_PHRASE_DIALOG_LINK << "\n"
        << "SSLSessionCache none\n"
        << "Mutex sem\n"
        << "SSLFIPS on\n";

    GenerateSecureConfiguration(out, port);
    out   << "</IfModule>\n";
  }
  // Next else is for Non-Secured mode Port Consolidation with REST API
  else {
	   out << "<VirtualHost _default_>\n";
	   out << "RewriteEngine On\n"
	       << "RewriteCond %{REQUEST_URI} !.*/plcm/.*$\n"
	       << "RewriteCond %{REQUEST_URI} !.*/api/rest/.*$\n"
	       << "RewriteRule .? - [S=4]\n"
	       << "RewriteCond %{REQUEST_FILENAME} !-f\n"
	       << "RewriteCond %{REQUEST_FILENAME} !-d\n"
	       << "RewriteCond %{REQUEST_URI} !.*/WebHelp/.*$\n"
	       << "RewriteRule ^(.+) " << MCU_MCMS_DIR << "/EMA/htdocs/PHPUtils/dispatch.php$1 [L,QSA]\n"
	       << "RewriteRule ^(.+) " << MCU_MCMS_DIR << "/EMA$1 [L,QSA]\n"
		   << "RewriteCond %{REQUEST_METHOD} !^(GET|POST|HEAD)$\n"
		   << "RewriteRule .* - [F]\n";
	   out << "</VirtualHost>\n";
  }

  // Next is for all platforms - adding Username
  {
  	std::string result;
  	STATUS stat = SystemPipedCommand("whoami", result);
  	if (STATUS_OK == stat)
  	{
		out << "User " << result.c_str();
  	}
  	else
 		TRACEWARN << "whoami system pipe command failed.";
  }
  // Next Block is for Simulation - adding Listen 127.0.0.1:8080
  if (!IsTarget() && 
       eProductTypeEdgeAxis != curProductType && 
       eProductTypeSoftMCUMfw != curProductType && 
       eProductTypeGesher != curProductType && 
       eProductTypeNinja != curProductType) {
    std::string ipv4 = m_IpAddressToConfig_ipV4;
    if (!ipv4.empty() && "127.0.0.1" != ipv4) {
    	out << "Listen 127.0.0.1:80\n";
    }
  }

  if (m_isExternalContentSupported)
  {
    out << "SSLProxyEngine on\n";

    std::string protocolPrefix;
    if (0 == m_externalContentIp.find("http"))
      protocolPrefix = " ";
    else if (443 == m_externalContentPort || 4433 == m_externalContentPort)
      protocolPrefix = " https://";
    else
      protocolPrefix = " http://";

    out << "ProxyPass " << m_externalContentDir << protocolPrefix
        << m_externalContentIp << m_externalContentDir << "\n"
        << "ProxyPassReverse " << m_externalContentDir << protocolPrefix
        << m_externalContentIp << m_externalContentDir << "\n";

    TRACEINTO << "\nProxyPass: " << m_externalContentDir << protocolPrefix
              << m_externalContentIp << m_externalContentDir
              << "\nProxyPassReverse: " << m_externalContentDir << protocolPrefix
              << m_externalContentIp << m_externalContentDir;
  }

  if (IsUltraSecureMode())
    out << "LimitRequestBody 786432000\n";

  // Discards EMA UI for MFW.
  if (eProductTypeSoftMCUMfw == curProductType)
    out << "Alias /index.html "+MCU_MCMS_DIR+"/EMA/EMA.UI/Dummy.html\n";

  fname = MCU_TMP_DIR+"/apache-log";
  std::string fname1 = MCU_TMP_DIR+"/apache-log-request custom";
  out << "KeepAliveTimeout " << m_KeepAliveTimeout << "\n"
      << "MaxKeepAliveRequests " << m_MaxKeepAliveRequests << "\n"
      << "#Uncomment to get log details\n"
      << "LogLevel " << GetApacheLogLevel().c_str() << "\n"
      << "#ErrorLog " << fname.c_str() << "\n"
      << "#LogFormat \"%h %l %u %t \\\"%r\\\" %>s %b\" custom\n"
      << "#CustomLog " << fname1.c_str() << "\n";

  if (m_isV35GWEnabled)
  {
    out << "#Proxy mode for V35 Gateway\n";

    if (!m_isExternalContentSupported) // otherwise "SSLProxyEngine on" is already written
      out << "SSLProxyEngine on\n";

    std::string sInternalGwAddress =
        m_internalGWAddress.empty() ? "0.0.0.0" : m_internalGWAddress;

    list<RvgwItem>::const_iterator it=m_v35GwList.begin();
    for(;it != m_v35GwList.end();it++)
    {
    	RvgwItem item = (*it);
    	std::string alias = "/"+item.m_aliasName;
    	if(m_isSecured)
    	{
    		out << "ProxyPass "+alias+" https://" << item.m_internalIP << std::endl
    		<< "ProxyPassReverse "+alias +" https://"<<item.m_internalIP << std::endl;
    	}
    	else
    	{
    		out << "ProxyPass "+alias+" http://" << item.m_internalIP << std::endl
    		<< "ProxyPassReverse "+alias +" http://"<<item.m_internalIP << std::endl;
    	}

    } 
    // set proxy settings
    out << "SSLProxyProtocol -all +TLSv1" << std::endl;
    out << "SSLProxyVerify none" << std::endl;
    out << "SSLProxyCheckPeerCN off" << std::endl;
    out << "SSLProxyCheckPeerExpire off" << std::endl;
  }

  TRACEINTOFUNC << "V35GW Enabled: " << YesNo(m_isV35GWEnabled)
                << "\nInternal GW Address: " << m_internalGWAddress;

  // Writes configuration file with root permissions
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  CSegment* seg = new CSegment;
  fname = MCU_TMP_DIR+"/httpd.listen.conf";
  *seg << fname.c_str() << out.str();

  OPCODE opcode;
  CSegment ret_seg;
  CManagerApi api(eProcessConfigurator);
  STATUS stat = api.SendMessageSync(seg,
                                    CONFIGURATOR_WRITE_FILE,
                                    1 * SECOND,
                                    opcode,
                                    ret_seg);

  PASSERTSTREAM_AND_RETURN(stat != STATUS_OK,
      "Unable to send " << proc->GetOpcodeAsString(CONFIGURATOR_WRITE_FILE)
      << " (" << CONFIGURATOR_WRITE_FILE
      << ") to " << ProcessNames[eProcessConfigurator]
      << ": " << proc->GetStatusAsString(stat));

  PASSERTSTREAM(opcode != STATUS_OK,
      "Unable to write file " << fname
      << ": " << proc->GetStatusAsString(opcode));
}

void CApacheConfig::GenerateConfigFile() const
{
  GenerateListenConfiguration();
  GenerateClientCfgFile();
}

std::string CApacheConfig::GetApacheLogLevel() const
{

	 const string values[]={"debug", "info", "notice", "warn", "error", "crit"};
	 CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	 std::string strValue;
	 sysConfig->GetDataByKey(CFG_KEY_APACHE_LOG_LEVEL, strValue);
	 for(int i=0; i < (int)(sizeof(values)/sizeof(string)); i++ )
	 {
		 if(strValue.compare(values[i])==0)
		 {
			 return strValue;
		 }
	 }

	 return "notice";
}


BOOL CApacheConfig::IsV35UseSSLSockets() const
{
	 CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	  BOOL isSSLSockets = NO;
	  sysConfig->GetBOOLDataByKey("V35_USE_SSL_PORTS", isSSLSockets);
	  return isSSLSockets;
}

// Static, Private
BOOL CApacheConfig::IsUltraSecureMode()
{
  CSysConfig* cfg = CProcessBase::GetProcess()->GetSysConfig();
  FPASSERT_AND_RETURN_VALUE(NULL == cfg, FALSE);

  BOOL val;
  BOOL res = cfg->GetBOOLDataByKey(CFG_KEY_JITC_MODE, val);
  FPASSERTSTREAM_AND_RETURN_VALUE(!res,
      "CSysConfig::GetBOOLDataByKey: " << CFG_KEY_JITC_MODE,
      FALSE);

  return val;
}

