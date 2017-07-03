/*
 * CDnsSoftMcuManagment.cpp
 *
 *  Created on: Oct 21, 2013
 *      Author: stanny
 */

#include "CDnsSoftMcuManagment.h"

namespace McmsNetworkPackage {

CDnsSoftMcuManagment::CDnsSoftMcuManagment() {
	// TODO Auto-generated constructor stub

}



CDnsSoftMcuManagment::~CDnsSoftMcuManagment() {
	// TODO Auto-generated destructor stub
}

STATUS CDnsSoftMcuManagment::ConfigureDnsServers(const std::string& search,const std::string& dns1,const std::string& dns2,const std::string& dns3)
{
	return STATUS_OK;
}
STATUS CDnsSoftMcuManagment::NameServerUpdate(const std::string& dns, const std::string& host,const std::string& zone,const std::string& ip,const std::string& ipv6)
{
	return STATUS_OK;
}

} /* namespace McmsNetworkPackage */
