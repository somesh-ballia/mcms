/*
 * CDnsCommonRmxMangment.h
 *
 *  Created on: Oct 21, 2013
 *      Author: stanny
 */

#ifndef CDNSCOMMONRMXMANGMENT_H_
#define CDNSCOMMONRMXMANGMENT_H_

#include "CDnsManagment.h"

namespace McmsNetworkPackage {

class CDnsCommonRmxMangment: public McmsNetworkPackage::CDnsManagment {
	CLASS_TYPE_1( CDnsCommonRmxMangment ,CDnsManagment)
protected:
		STATUS ConfigureDnsServers(const std::string& search,const std::string& dns1,const std::string& dns2,const std::string& dns3);
		STATUS NameServerUpdate(const std::string& dns, const std::string& host,const std::string& zone,const std::string& ip,const std::string& ipv6);
public:
	CDnsCommonRmxMangment();
	virtual ~CDnsCommonRmxMangment();
	virtual const char* NameOf() const { return "CDnsCommonRmxMangment";}
};

} /* namespace McmsNetworkPackage */
#endif /* CDNSCOMMONRMXMANGMENT_H_ */
