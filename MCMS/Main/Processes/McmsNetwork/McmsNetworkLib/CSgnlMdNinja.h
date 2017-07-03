/*
 * CSgnlMdNinja.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSGNLMDNINJA_H_
#define CSGNLMDNINJA_H_

#include "CSgnlMdCommonGesherNinja.h"

namespace McmsNetworkPackage
{

class CSgnlMdNinja: public McmsNetworkPackage::CSgnlMdCommonGesherNinja
{
	CLASS_TYPE_1(CSgnlMdNinja,CSgnlMdCommonGesherNinja)
public:
	CSgnlMdNinja();
	virtual ~CSgnlMdNinja();
	virtual const char* NameOf() const { return "CSgnlMdNinja";}

protected:
	virtual STATUS OnInitInterfaces() {return STATUS_OK;}
};

} /* namespace McmsNetworkPackage */

#endif /* CSGNLMDNINJA_H_ */
