/*
 * CDnsEdgeManagment.cpp
 *
 *  Created on: Oct 21, 2013
 *      Author: stanny
 */

#include "CDnsEdgeManagment.h"

namespace McmsNetworkPackage {

CDnsEdgeManagment::CDnsEdgeManagment() {
	// TODO Auto-generated constructor stub

}

CDnsEdgeManagment::~CDnsEdgeManagment() {
	// TODO Auto-generated destructor stub
}

STATUS CDnsEdgeManagment::ConfigureDnsServers(const std::string& search,const std::string& dns1,const std::string& dns2,const std::string& dns3)
{
	STATUS retStatus = STATUS_OK;
	if(TestMcmsUser())
	{
		std::string answer;
		SystemPipedCommand("/usr/bin/sudo /bin/chmod -R 777 /etc/resolv.conf", answer);
		retStatus =  CDnsManagment::ConfigureDnsServers(search,dns1,dns2,dns3);
		SystemPipedCommand("/usr/bin/sudo /bin/chmod -R 555 /etc/resolv.conf", answer);
	}
	return retStatus;
}
STATUS CDnsEdgeManagment::NameServerUpdate(const std::string& dns, const std::string& host,const std::string& zone,const std::string& ip,const std::string& ipv6)
{
	STATUS retStatus = STATUS_OK;
	return retStatus;
}

STATUS CDnsEdgeManagment::ConfigDnsOff()
{
	STATUS retStatus = STATUS_OK;
	return retStatus;
}

} /* namespace McmsNetworkPackage */
