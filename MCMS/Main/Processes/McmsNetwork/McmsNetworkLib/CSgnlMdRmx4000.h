/*
 * CSgnlMdRmx4000.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSGNLMDRMX4000_H_
#define CSGNLMDRMX4000_H_

#include "CSgnlMdCommonRmx1500And4000.h"

namespace McmsNetworkPackage
{

class CSgnlMdRmx4000: public McmsNetworkPackage::CSgnlMdCommonRmx1500And4000
{
	CLASS_TYPE_1(CSgnlMdRmx4000,CSgnlMdCommonRmx1500And4000)
public:
	CSgnlMdRmx4000();
	virtual ~CSgnlMdRmx4000();
	virtual const char* NameOf() const { return "CSgnlMdRmx4000";}

protected:
	virtual STATUS OnInitInterfaces() { return STATUS_OK;}
};

} /* namespace McmsNetworkPackage */

#endif /* CSGNLMDRMX4000_H_ */
