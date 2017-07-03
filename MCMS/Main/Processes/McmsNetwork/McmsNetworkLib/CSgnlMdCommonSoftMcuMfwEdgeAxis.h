/*
 * CSgnlMdCommonSoftMcuMfwEdgeAxis.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSGNLMDCOMMONSOFTMCUMFWEDGEAXIS_H_
#define CSGNLMDCOMMONSOFTMCUMFWEDGEAXIS_H_

#include "CSignalMediaNetwork.h"

namespace McmsNetworkPackage
{

class CSgnlMdCommonSoftMcuMfwEdgeAxis: public McmsNetworkPackage::CSignalMediaNetwork
{
	CLASS_TYPE_1(CSgnlMdCommonSoftMcuMfwEdgeAxis,CSignalMediaNetwork)

public:
	CSgnlMdCommonSoftMcuMfwEdgeAxis();
	virtual ~CSgnlMdCommonSoftMcuMfwEdgeAxis();
	virtual const char* NameOf() const { return "CSignalMediaNetwork";}

protected:
	virtual STATUS OnInitInterfaces();

	virtual STATUS ReadConfigurationFile();
};

} /* namespace McmsNetworkPackage */

#endif /* CSGNLMDCOMMONSOFTMCUMFWEDGEAXIS_H_ */
