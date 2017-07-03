/*
 * CSgnlMdCommonRmx1500And4000.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSGNLMDCOMMONRMX1500AND4000_H_
#define CSGNLMDCOMMONRMX1500AND4000_H_

#include "CSignalMediaNetwork.h"

namespace McmsNetworkPackage
{

class CSgnlMdCommonRmx1500And4000: public McmsNetworkPackage::CSignalMediaNetwork
{
	CLASS_TYPE_1(CSgnlMdCommonRmx1500And4000,CSignalMediaNetwork)
public:
	CSgnlMdCommonRmx1500And4000();
	virtual ~CSgnlMdCommonRmx1500And4000();
	virtual const char* NameOf() const { return "CSgnlMdCommonRmx1500And4000";}

protected:
		virtual STATUS OnInitInterfaces() = 0;
};

} /* namespace McmsNetworkPackage */

#endif /* CSGNLMDCOMMONRMX1500AND4000_H_ */
