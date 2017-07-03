/*
 * CSgnlMdCommonGesherNinja.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSGNLMDCOMMONGESHERNINJA_H_
#define CSGNLMDCOMMONGESHERNINJA_H_

#include "CSignalMediaNetwork.h"

namespace McmsNetworkPackage
{

class CSgnlMdCommonGesherNinja: public McmsNetworkPackage::CSignalMediaNetwork
{
	CLASS_TYPE_1(CSgnlMdCommonGesherNinja,CSignalMediaNetwork)
public:
	CSgnlMdCommonGesherNinja();
	virtual ~CSgnlMdCommonGesherNinja();
	virtual const char* NameOf() const { return "CSgnlMdCommonGesherNinja";}

protected:
	virtual STATUS OnInitInterfaces() = 0;
};

} /* namespace McmsNetworkPackage */

#endif /* CSGNLMDCOMMONGESHERNINJA_H_ */
