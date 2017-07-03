/*
 * CSgnlMdRmx1500.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSGNLMDRMX1500_H_
#define CSGNLMDRMX1500_H_

#include "CSgnlMdCommonRmx1500And4000.h"

namespace McmsNetworkPackage
{

class CSgnlMdRmx1500: public McmsNetworkPackage::CSgnlMdCommonRmx1500And4000
{
	CLASS_TYPE_1(CSgnlMdRmx1500,CSgnlMdCommonRmx1500And4000)
public:
	CSgnlMdRmx1500();
	virtual ~CSgnlMdRmx1500();
	virtual const char* NameOf() const { return "CSgnlMdRmx1500";}

protected:
	virtual STATUS OnInitInterfaces() {return STATUS_OK;}
};

} /* namespace McmsNetworkPackage */

#endif /* CSGNLMDRMX1500_H_ */
