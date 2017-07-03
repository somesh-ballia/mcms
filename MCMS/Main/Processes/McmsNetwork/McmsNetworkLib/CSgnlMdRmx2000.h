/*
 * CSgnlMdRmx2000.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSGNLMDRMX2000_H_
#define CSGNLMDRMX2000_H_

#include "CSignalMediaNetwork.h"

namespace McmsNetworkPackage
{

class CSgnlMdRmx2000: public McmsNetworkPackage::CSignalMediaNetwork
{
	CLASS_TYPE_1(CSgnlMdRmx2000,CSignalMediaNetwork)
public:
	CSgnlMdRmx2000();
	virtual ~CSgnlMdRmx2000();
	virtual const char* NameOf() const { return "CSgnlMdRmx2000";}

protected:
	virtual STATUS OnInitInterfaces() {return STATUS_OK;}
};

} /* namespace McmsNetworkPackage */

#endif /* CSGNLMDRMX2000_H_ */
