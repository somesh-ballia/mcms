/*
 * CDnsCommonRmxMangment.cpp
 *
 *  Created on: Oct 21, 2013
 *      Author: stanny
 */

#include "CDnsCommonRmxMangment.h"

namespace McmsNetworkPackage {

CDnsCommonRmxMangment::CDnsCommonRmxMangment() {
	// TODO Auto-generated constructor stub

}

CDnsCommonRmxMangment::~CDnsCommonRmxMangment() {
	// TODO Auto-generated destructor stub
}

STATUS CDnsCommonRmxMangment::NameServerUpdate(const std::string& dns, const std::string& host,const std::string& zone,const std::string& ip,const std::string& ipv6)
{
	STATUS retStatus = STATUS_OK;
	if ( FALSE == IsTarget() ) // no configuration should be done on Pizzas
	{
		return STATUS_OK;
	}
	retStatus =  CDnsManagment::NameServerUpdate(dns,host,zone,ip,ipv6);

	return retStatus;

}

STATUS CDnsCommonRmxMangment::ConfigureDnsServers(const std::string& search,const std::string& dns1,const std::string& dns2,const std::string& dns3)
{
	STATUS retStatus = STATUS_OK;
	if ( FALSE == IsTarget() ) // no configuration should be done on Pizzas
	{
		return STATUS_OK;
	}
	retStatus =  CDnsManagment::ConfigureDnsServers(search,dns1,dns2,dns3);

	return retStatus;
}

} /* namespace McmsNetworkPackage */
