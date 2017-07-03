/*
 * CDnsSoftMcuManagment.h
 *
 *  Created on: Oct 21, 2013
 *      Author: stanny
 */

#ifndef CDNSSOFTMCUMANAGMENT_H_
#define CDNSSOFTMCUMANAGMENT_H_

#include "CDnsManagment.h"

namespace McmsNetworkPackage {

class CDnsSoftMcuManagment: public McmsNetworkPackage::CDnsSoftMcuMfwEdgeAxis {
	CLASS_TYPE_1( CDnsSoftMcuManagment ,CDnsSoftMcuMfwEdgeAxis)
protected:
	virtual STATUS ConfigureDnsServers(const std::string& search,const std::string& dns1,const std::string& dns2,const std::string& dns3);
	virtual STATUS NameServerUpdate(const std::string& dns, const std::string& host,const std::string& zone,const std::string& ip,const std::string& ipv6);
public:
	CDnsSoftMcuManagment();
	virtual ~CDnsSoftMcuManagment();
	virtual const char* NameOf() const { return "CDnsSoftMcuManagment";}
};

} /* namespace McmsNetworkPackage */
#endif /* CDNSSOFTMCUMANAGMENT_H_ */
