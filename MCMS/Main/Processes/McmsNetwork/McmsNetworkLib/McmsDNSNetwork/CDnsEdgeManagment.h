/*
 * CDnsEdgeManagment.h
 *
 *  Created on: Oct 21, 2013
 *      Author: stanny
 */

#ifndef CDNSEDGEMANAGMENT_H_
#define CDNSEDGEMANAGMENT_H_

#include "CDnsManagment.h"

namespace McmsNetworkPackage {

class CDnsEdgeManagment: public McmsNetworkPackage::CDnsSoftMcuMfwEdgeAxis {
	CLASS_TYPE_1(CDnsEdgeManagment,CDnsSoftMcuMfwEdgeAxis)
protected:
		STATUS ConfigureDnsServers(const std::string& search,const std::string& dns1,const std::string& dns2,const std::string& dns3);
		STATUS ConfigDnsOff();
		STATUS NameServerUpdate(const std::string& dns, const std::string& host,const std::string& zone,const std::string& ip,const std::string& ipv6);
public:
	CDnsEdgeManagment();
	virtual ~CDnsEdgeManagment();
	virtual const char* NameOf() const { return "CDnsEdgeManagment";}
};

} /* namespace McmsNetworkPackage */
#endif /* CDNSEDGEMANAGMENT_H_ */
