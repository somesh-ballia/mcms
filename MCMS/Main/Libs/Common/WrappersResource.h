#ifndef WRAPPERSRESOURCE_H_
#define WRAPPERSRESOURCE_H_

#include "WrappersCSBase.h"
#include "AllocateStructs.h"
#include "IpAddressDefinitions.h"
#include "WrappersCommon.h"


class CUDPResourcePerPQWrapper : public CBaseWrapper 
{	
public:	
	CUDPResourcePerPQWrapper(const IP_SERVICE_UDP_RESOURCE_PER_PQ_S &data);
	virtual ~CUDPResourcePerPQWrapper();
	
	virtual void Dump(std::ostream&) const;
	
private:
	const IP_SERVICE_UDP_RESOURCE_PER_PQ_S &m_Data;
};




class CUDPResourceWrapper : public CBaseWrapper
{	
public:
	CUDPResourceWrapper(const IP_SERVICE_UDP_RESOURCES_S &data);
	virtual ~CUDPResourceWrapper();
	
	virtual void Dump(std::ostream&) const;
	
private:
	const IP_SERVICE_UDP_RESOURCES_S &m_Data;
};




class CIpServiceUdpResourcesPerPqForIpv6UpdateWrapper : public CBaseWrapper
{
public:
	CIpServiceUdpResourcesPerPqForIpv6UpdateWrapper(const IP_SERVICE_UDP_RESOURCE_PER_PQ_FOR_IPV6_UPDATE_S &data);
	virtual ~CIpServiceUdpResourcesPerPqForIpv6UpdateWrapper();
	
	virtual void Dump(std::ostream&) const;
	
private:
	const IP_SERVICE_UDP_RESOURCE_PER_PQ_FOR_IPV6_UPDATE_S &m_Data;
};




class CIpV6AddressUpdateResourcesWrapper : public CBaseWrapper
{
public: 
	CIpV6AddressUpdateResourcesWrapper(const IPV6_ADDRESS_UPDATE_RESOURCES_S &data);
	virtual ~CIpV6AddressUpdateResourcesWrapper();
	
	virtual void Dump(std::ostream&) const;
	
private:
	const IPV6_ADDRESS_UPDATE_RESOURCES_S &m_Data;
};




class CLastConfIdWrapper : public CBaseWrapper
{	
public:
	CLastConfIdWrapper(const LAST_CONF_ID_S &m_Data);
	virtual ~CLastConfIdWrapper();
	
	virtual void Dump(std::ostream&) const;
	
private:
	const LAST_CONF_ID_S &m_Data;	
};






class CCFSWrapper : public CBaseWrapper
{	
public:
	CCFSWrapper(const RSRC_CFS_S &m_Data);
	virtual ~CCFSWrapper();
	
	virtual void Dump(std::ostream&) const;
	
private:
	const RSRC_CFS_S &m_Data;	
};





class CUDPPortRangeWrapper : public CBaseWrapper
{	
public:
	CUDPPortRangeWrapper(const UDP_PORT_RANGE_S &m_Data);
	virtual ~CUDPPortRangeWrapper();
	
	virtual void Dump(std::ostream&) const;
	
private:
	const UDP_PORT_RANGE_S &m_Data;	
};

/*class CIPV4Wrapper : public CBaseWrapper
{	
public:
	CIPV4Wrapper(ipAddressV4If &data);
	virtual ~CIPV4Wrapper();
	
	virtual const char*  NameOf() const {return "CIPV4Wrapper";}
	virtual void Dump(std::ostream&) const;
	
	void NullData();
	void CopyData(ipAddressV4If &Data);
	
private:
	ipAddressV4If &m_Data;	
};

class CIPV6AraryWrapper : public CBaseWrapper
{	
public:
	CIPV6AraryWrapper(ipv6AddressArray &data);
	virtual ~CIPV6AraryWrapper();
	
	virtual const char*  NameOf() const {return "CIPV6AraryWrapper";}
	virtual void Dump(std::ostream&) const;
	
	void CopyData(const ipv6AddressArray& data);
	
private:
	ipv6AddressArray &m_Data;
};*/




#endif /*RESOURCEWRAPERS_H_*/
