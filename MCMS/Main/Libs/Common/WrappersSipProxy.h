#ifndef WRAPPERSSIPPROXY_H_
#define WRAPPERSSIPPROXY_H_

#include "WrappersCSBase.h"
#include "SIPProxyStructs.h"







class CSipProxyIpParamsWrapper : public CBaseWrapper
{	
public:	
	CSipProxyIpParamsWrapper(const SIP_PROXY_IP_PARAMS_S &data);
	virtual ~CSipProxyIpParamsWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSipProxyIpParamsWrapper";}
	
private:
	const SIP_PROXY_IP_PARAMS_S &m_Data;
};








class CSipDynamicProxyParamWrapper : public CBaseWrapper
{	
public:	
	CSipDynamicProxyParamWrapper(const SIP_PROXY_DYNAMIC_PARAMS_S &data);
	virtual ~CSipDynamicProxyParamWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSipDynamicProxyParamWrapper";}
	
private:
	const SIP_PROXY_DYNAMIC_PARAMS_S &m_Data;
};







class CSipProxyStatusIndWrapper : public CBaseWrapper
{	
public:	
	CSipProxyStatusIndWrapper(const SIP_PROXY_STATUS_PARAMS_S &data);
	virtual ~CSipProxyStatusIndWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CSipProxyStatusIndWrapper";}
	
private:
	const SIP_PROXY_STATUS_PARAMS_S &m_Data;
};








#endif /*WRAPPERSSIPPROXY_H_*/
