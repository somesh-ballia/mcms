/*
 * CDnsCommonGesheNinjaMangment.h
 *
 *  Created on: Oct 21, 2013
 *      Author: stanny
 */

#ifndef CDNSCOMMONGESHENINJAMANGMENT_H_
#define CDNSCOMMONGESHENINJAMANGMENT_H_

#include "CDnsManagment.h"

namespace McmsNetworkPackage {

class CDnsCommonGesheNinjaMangment: public McmsNetworkPackage::CDnsManagment {
	CLASS_TYPE_1( CDnsCommonGesheNinjaMangment ,CDnsManagment)
protected:
		STATUS ConfigureDnsServers(const std::string& search,const std::string& dns1,const std::string& dns2,const std::string& dns3);
		std::string GetCmdLinePrefix() {return "sudo ";};

public:
	CDnsCommonGesheNinjaMangment();
	virtual ~CDnsCommonGesheNinjaMangment();
	virtual const char* NameOf() const { return "CDnsCommonGesheNinjaMangment";}
};

} /* namespace McmsNetworkPackage */
#endif /* CDNSCOMMONGESHENINJAMANGMENT_H_ */
