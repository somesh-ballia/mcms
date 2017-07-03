/*
 * CSgnlMdSoftMcu.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSGNLMDSOFTMCU_H_
#define CSGNLMDSOFTMCU_H_

#include "CSgnlMdCommonSoftMcuMfwEdgeAxis.h"

namespace McmsNetworkPackage
{

class CSgnlMdSoftMcu: public McmsNetworkPackage::CSgnlMdCommonSoftMcuMfwEdgeAxis
{
	CLASS_TYPE_1(CSgnlMdSoftMcu,CSgnlMdCommonSoftMcuMfwEdgeAxis)

public:
	CSgnlMdSoftMcu();
	virtual ~CSgnlMdSoftMcu();
	virtual const char* NameOf() const { return "CSgnlMdSoftMcu";}

protected:
	virtual STATUS OnInitInterfaces() { return STATUS_OK; }
};

class CSgnlMdSoftCallGenerator: public McmsNetworkPackage::CSgnlMdCommonSoftMcuMfwEdgeAxis
{
	CLASS_TYPE_1(CSgnlMdSoftCallGenerator,CSgnlMdCommonSoftMcuMfwEdgeAxis)

public:
	CSgnlMdSoftCallGenerator();
	virtual ~CSgnlMdSoftCallGenerator();
	virtual const char* NameOf() const { return "CSgnlMdSoftCallGenerator";}

protected:
	virtual STATUS OnInitInterfaces() { return STATUS_OK; }
};

} /* namespace McmsNetworkPackage */

#endif /* CSGNLMDSOFTMCU_H_ */
