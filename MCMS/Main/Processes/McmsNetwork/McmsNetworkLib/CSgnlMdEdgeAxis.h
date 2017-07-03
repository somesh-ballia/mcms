/*
 * CSgnlMdEdgeAxis.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSGNLMDEDGEAXIS_H_
#define CSGNLMDEDGEAXIS_H_

#include "CSgnlMdCommonSoftMcuMfwEdgeAxis.h"

class CSystemInterfaceList;
class CIPSpan;

namespace McmsNetworkPackage
{

class CSgnlMdEdgeAxis: public McmsNetworkPackage::CSgnlMdCommonSoftMcuMfwEdgeAxis
{
	CLASS_TYPE_1(CSgnlMdEdgeAxis,CSgnlMdCommonSoftMcuMfwEdgeAxis)
public:
	CSgnlMdEdgeAxis();
	virtual ~CSgnlMdEdgeAxis();
	virtual const char* NameOf() const { return "CSgnlMdEdgeAxis";}
	STATUS TryToRecover(STATUS eSelfLastStatus, STATUS eOtherLastStatus, CBaseNetworkPlatform *pOtherNetwork);

protected:
    virtual STATUS OnInitInterfaces();
	virtual STATUS OnPreConfigNetworkSignalMedia() {return STATUS_OK;} //
	virtual STATUS OnConfigNetworkSignalMedia() { return STATUS_OK;}
	virtual STATUS OnPostConfigNetworkSignalMedia() {return STATUS_OK;}

private:
    CSystemInterfaceList m_systemInterfaceList;
};

} /* namespace McmsNetworkPackage */

#endif /* CSGNLMDEDGEAXIS_H_ */
