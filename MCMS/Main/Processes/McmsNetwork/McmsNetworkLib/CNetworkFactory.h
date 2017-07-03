/*
 * CNetworkFactory.h
 *
 *  Created on: Jul 30, 2013
 *      Author: stanny
 */

#ifndef CNETWORKFACTORY_H_
#define CNETWORKFACTORY_H_


#include "CManagmentNetwork.h"
#include "McmsDNSNetwork/CDnsManagment.h"
#include "CSignalMediaNetwork.h"

namespace McmsNetworkPackage {


class CNetworkFactory {
public:
	virtual ~CNetworkFactory();
	static CNetworkFactory& GetInstance()
	{
		static CNetworkFactory singleton;
		return singleton;
	};
	CManagmentNetwork*    CreateMngmntNetwork(eProductType curProductType);
	CDnsManagment*		  CreateMngmntDns(eProductType curProductType);
	CIPService*           ReadMngmntService(STATUS &stat);

	STATUS WriteNetworkConfigurationXML();
	//unit testing

	CSignalMediaNetwork* CreateSignalMediaNetwork(eProductType curProductType);

private:
	CNetworkFactory();
	CManagmentNetwork*  m_pMngntNetwork;
	CDnsManagment*		m_pDnsMngmt;
	CIPService*			m_pMngmtService;
	CSignalMediaNetwork* m_pSgnlMdNetwork;

};

} /* namespace McmsNetworkPackage */
#endif /* CNETWORKFACTORY_H_ */
