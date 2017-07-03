/*
 * CDnsCommonGesheNinjaMangment.cpp
 *
 *  Created on: Oct 21, 2013
 *      Author: stanny
 */

#include "CDnsCommonGesheNinjaMangment.h"

namespace McmsNetworkPackage {

CDnsCommonGesheNinjaMangment::CDnsCommonGesheNinjaMangment() {
	// TODO Auto-generated constructor stub

}

CDnsCommonGesheNinjaMangment::~CDnsCommonGesheNinjaMangment() {
	// TODO Auto-generated destructor stub
}




STATUS CDnsCommonGesheNinjaMangment::ConfigureDnsServers(const std::string& search,const std::string& dns1,const std::string& dns2,const std::string& dns3)
{
	STATUS retStatus = STATUS_OK;
	std::string answer;
	SystemPipedCommand("sudo /bin/chmod 666 /config/common/etc/resolv.conf", answer);
	retStatus =  CDnsManagment::ConfigureDnsServers(search,dns1,dns2,dns3);
	SystemPipedCommand("sudo /bin/chmod 644 /config/common/etc/resolv.conf", answer);
	return retStatus;
}

} /* namespace McmsNetworkPackage */
