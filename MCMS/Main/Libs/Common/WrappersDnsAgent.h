#ifndef WRAPPERSDNSAGENT_H_
#define WRAPPERSDNSAGENT_H_

#include "WrappersCSBase.h"
#include "DNSAgentManagerStructs.h"


class CDnsParamsIpWrapper : public CBaseWrapper
{	
public:
	CDnsParamsIpWrapper(const DNS_PARAMS_IP_S &data);
	virtual ~CDnsParamsIpWrapper();
	
	virtual void Dump(std::ostream&) const;
	virtual const char* NameOf() const { return "CDnsParamsIpWrapper";}
	
private:
	const DNS_PARAMS_IP_S &m_Data;	
};

#endif /*WRAPPERSDNSAGENT_H_*/
