// ConfIpParameters.h: interface for the CConfIpParameters class.
//
//////////////////////////////////////////////////////////////////////


#ifndef _ConfIpParameters_H_
#define _ConfIpParameters_H_


#include "IpParameters.h"
#include "ConfStructs.h"
#include "IpAddressDefinitions.h"
#include <functional>
#include <algorithm>
#include "ObjString.h"

typedef struct
{
  BYTE                numOfWaiting;
  ipAddressStruct     ProxyIpFromDns;
} DnsParamsSt;

class CConfIpParameters : public CPObject
{
  CLASS_TYPE_1(CConfIpParameters , CPObject)

public:
                      CConfIpParameters();
                      CConfIpParameters(const CONF_IP_PARAMS_S &confIpParams);
  virtual            ~CConfIpParameters();
  virtual const char* NameOf() const { return "CConfIpParameters"; }
  virtual void        Dump(std::ostream& msg) const;

  CConfIpParameters&  operator =(const CConfIpParameters& rOther);
  friend WORD         operator==(const CConfIpParameters& lhs, const CConfIpParameters& rhs);
  friend bool         operator<(const CConfIpParameters& lhs, const CConfIpParameters& rhs);

  CONF_IP_PARAMS_S*   GetConfIpParamsStruct();

  void                SetServiceId(DWORD id);
  DWORD               GetServiceId();

  BYTE*               GetServiceName();
  void                SetServiceName(const BYTE* name);

  void                SetData(const CONF_IP_PARAMS_S* data);

  BOOL                isGKExternal();
  ipAddressStruct*    GetGKIp();
  ALIAS_S*            GetAliasArray();
  ipAddressIf         GetIpV4Address();
  ipAddressIf*        GetIpV6AddressArr();
  ipAddressIf         GetIpV6Address(int idx);
  eIpType             GetIPAddressTypesInService();
  void                SetIPAddressTypesInService(eIpType ipVType);
  ALIAS_S             GetDialIn();
  QOS_S               GetQOS();

  // for SIP party use
  const CSmallString  GetRegistrarDomainName() const;
  const CSmallString  GetSipProxyName() const;
  const CSmallString  GetLocalDomainName() const;
  DWORD               GetSipProxyStatus();
  DWORD               GetSipProxyIpV4Address();
  BYTE                GetSipTransportType();
  WORD                GetConfigurationOfSipServers();

  //DNS
  void                SetNumOfDnsWaiting(BYTE num);
  BYTE                GetNumOfDnsWaiting();
  void                IncreaseNumOfDnsWaiting();
  void                DecreaseNumOfDnsWaiting();
  void                SetProxyIpFromDns(ipAddressStruct* pProxyAddr);

  //ICE
  WORD                GetICEType();
  void                SetICEType(WORD Type);

  WORD                GetSipServerType();
  void                SetSipServerType(WORD Type);

  BYTE                GetIsDefaultH323SipService()const;
  void                SetIsDefaultH323SipService(BYTE val);


protected:
  CONF_IP_PARAMS_S    m_confIpParamsStruct;
  DnsParamsSt         m_dnsParams;
};

struct CConfIpServiceHunterByServiceID : std::unary_function<CConfIpParameters*, bool>
{
  DWORD serviceID;

  bool operator()(CConfIpParameters* service) const
  {
    bool isSameServiceID = 0;
    if (service->GetServiceId() == serviceID)
      isSameServiceID = 1;
    return isSameServiceID;
  }
};
struct CConfIpServiceHunterByName : std::unary_function<CConfIpParameters*, bool> 
{
	  char serviceName[NET_SERVICE_PROVIDER_NAME_LEN];

	  bool operator()(CConfIpParameters *service) const
	  {
	    if (!service || !service->GetServiceName())
	      return FALSE;
	    if (!strcmp((char*)(service->GetServiceName()), serviceName))
	      return TRUE;
	    return FALSE;
	  }
};
struct CConfIpServiceHunterByIPAddress : std::unary_function<CConfIpParameters*, bool> 
{
  mcTransportAddress IPAddress;

  bool operator()(CConfIpParameters* service) const
  {
    bool isIPAddressEqual = FALSE;
    if (eIpVersion4 == IPAddress.ipVersion)
    {
      if (service->GetIpV4Address().v4.ip == IPAddress.addr.v4.ip)
        isIPAddressEqual = TRUE;
    }
    else
    {
      if (eIpVersion6 == IPAddress.ipVersion)
      {
        int index = IPAddress.addr.v6.scopeId;
        if (memcmp(service->GetIpV6Address(index).v6.ip, IPAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN) == 0)
          isIPAddressEqual = TRUE;
      }
    }
    return isIPAddressEqual;
  }
};
struct CConfIpServiceHunterByDialinString : std::unary_function<CConfIpParameters*, bool> 
{
  char dialinString[ALIAS_NAME_LEN];

  bool operator()(CConfIpParameters* service) const
  {
    bool isSameDialin = FALSE;
    if (!service || !service->GetDialIn().aliasContent)
      return isSameDialin;
    WORD objectDialinLength = strlen((char*)(service->GetDialIn().aliasContent));
    if ((objectDialinLength > 0) && !strncmp(dialinString, (char*)(service->GetDialIn().aliasContent), objectDialinLength))
      isSameDialin = 1;
    return isSameDialin;
  }
};
#endif // _ConfIpParameters_H_
