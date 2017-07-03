/*
 * CSgnlMdGesher.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSGNLMDGESHER_H_
#define CSGNLMDGESHER_H_

#include "CSgnlMdCommonGesherNinja.h"

namespace McmsNetworkPackage
{

class CSgnlMdGesher: public McmsNetworkPackage::CSgnlMdCommonGesherNinja
{
	CLASS_TYPE_1(CSgnlMdGesher,CSgnlMdCommonGesherNinja)
public:
	CSgnlMdGesher();
	virtual ~CSgnlMdGesher();
	virtual const char* NameOf() const { return "CSgnlMdGesher";}

protected:
	virtual STATUS OnInitInterfaces() {return STATUS_OK;}
};

} /* namespace McmsNetworkPackage */

#endif /* CSGNLMDGESHER_H_ */
