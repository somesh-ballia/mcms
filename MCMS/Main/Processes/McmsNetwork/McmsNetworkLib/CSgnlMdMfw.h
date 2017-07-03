/*
 * CSgnlMdMfw.h
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#ifndef CSGNLMDMFW_H_
#define CSGNLMDMFW_H_

#include "CSgnlMdCommonSoftMcuMfwEdgeAxis.h"

class CSystemInterfaceList;
class CIPSpan;

namespace McmsNetworkPackage
{

class CSgnlMdMfw: public McmsNetworkPackage::CSgnlMdCommonSoftMcuMfwEdgeAxis
{
	CLASS_TYPE_1(CSgnlMdMfw,CSgnlMdCommonSoftMcuMfwEdgeAxis)
public:
	CSgnlMdMfw();
	virtual ~CSgnlMdMfw();
	virtual const char* NameOf() const { return "CSgnlMdMfw";}
	STATUS TryToRecover(STATUS eSelfLastStatus, STATUS eOtherLastStatus, CBaseNetworkPlatform *pOtherNetwork);

protected:
	virtual STATUS OnPreConfigNetworkSignalMedia();
	virtual STATUS OnConfigNetworkSignalMedia() { return STATUS_OK;}
	virtual STATUS OnPostConfigNetworkSignalMedia() {return STATUS_OK;}

private:
	STATUS CheckPerIPSpan(string strServiceName, CIPSpan *pIpSpan, eIpType  ipType);
	STATUS CheckStatusWithSystmSetting(CIPSpan *pIpSpan, STATUS status);

private:
    CSystemInterfaceList m_systemInterfaceList;
};

} /* namespace McmsNetworkPackage */

#endif /* CSGNLMDMFW_H_ */
