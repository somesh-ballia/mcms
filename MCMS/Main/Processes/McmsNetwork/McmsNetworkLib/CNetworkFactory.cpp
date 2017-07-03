/*
 * CNetworkFactory.cpp
 *
 *  Created on: Jul 30, 2013
 *      Author: stanny
 */

#include "CNetworkFactory.h"
#include "NetCommonDefines.h"
#include "Trace.h"
#include "TraceStream.h"
#include "CMngntRmx2000.h"
#include "CPureSoftProducts.h"
#include "CCommonRmx1500And4000.h"
#include "CCommonGesherNinja.h"
#include "SystemFunctions.h"
#include "CSgnlMdMfw.h"
#include "CSgnlMdSoftMcu.h"
#include "CSgnlMdEdgeAxis.h"
#include "CSgnlMdGesher.h"
#include "CSgnlMdNinja.h"
#include "CSgnlMdRmx1500.h"
#include "CSgnlMdRmx4000.h"
#include "CSgnlMdRmx2000.h"

#include "McmsDNSNetwork/CDnsCommonGesheNinjaMangment.h"
#include "McmsDNSNetwork/CDnsEdgeManagment.h"
#include "McmsDNSNetwork/CDnsCommonRmxMangment.h"
#include "McmsDNSNetwork/CDnsSoftMcuManagment.h"



namespace McmsNetworkPackage {

	CNetworkFactory::~CNetworkFactory() {

		if(m_pMngntNetwork)
			delete m_pMngntNetwork;

		if(m_pDnsMngmt)
			delete m_pDnsMngmt;
		if(m_pMngmtService)
			delete m_pMngmtService;
		if( NULL != m_pSgnlMdNetwork )
		{
			delete m_pSgnlMdNetwork;
			m_pSgnlMdNetwork = NULL;
		}
	}
	CNetworkFactory::CNetworkFactory() {

		m_pMngntNetwork = NULL;
		m_pDnsMngmt 	= NULL;
		m_pMngmtService = NULL;
		m_pSgnlMdNetwork = NULL;
	}

	CManagmentNetwork*    CNetworkFactory::CreateMngmntNetwork(eProductType curProductType)
	{
		if(m_pMngntNetwork)
			return m_pMngntNetwork;

		switch(curProductType)
		{
			case	eProductTypeRMX2000:  m_pMngntNetwork = new CMngntRmx2000();
											break;
			case  eProductTypeSoftMCU:    m_pMngntNetwork = new CMngntSoftMcu();
											break;
			case  eProductTypeSoftMCUMfw: m_pMngntNetwork = new CMngntMfw();
											break;
			case eProductTypeEdgeAxis:     m_pMngntNetwork = new CMngntEdgeAxis();
											break;
			case eProductTypeRMX4000:     m_pMngntNetwork = new CMngntRmx4000();
											break;
			case eProductTypeRMX1500:     m_pMngntNetwork = new CMngntRmx1500();
											break;
			case eProductTypeGesher:      m_pMngntNetwork = new CMngntGesher();
											break;
			case eProductTypeNinja:		  m_pMngntNetwork = new CMngntNinja();
											break;
			case eProductTypeCallGeneratorSoftMCU:
										  m_pMngntNetwork = new CMngntSoftCallGenerator();
											break;
			default:	return NULL;
		}

		return m_pMngntNetwork;
	}

	CSignalMediaNetwork*    CNetworkFactory::CreateSignalMediaNetwork(eProductType curProductType)
	{
		if(m_pSgnlMdNetwork)
			return m_pSgnlMdNetwork;

		switch(curProductType)
		{
			case	eProductTypeRMX2000:  m_pSgnlMdNetwork = new CSgnlMdRmx2000();
											break;
			case  eProductTypeSoftMCU:    m_pSgnlMdNetwork = new CSgnlMdSoftMcu();
											break;
			case  eProductTypeSoftMCUMfw: m_pSgnlMdNetwork = new CSgnlMdMfw();
											break;
			case eProductTypeEdgeAxis:     m_pSgnlMdNetwork = new CSgnlMdEdgeAxis();
											break;
			case eProductTypeRMX4000:     m_pSgnlMdNetwork = new CSgnlMdRmx4000();
											break;
			case eProductTypeRMX1500:     m_pSgnlMdNetwork = new CSgnlMdRmx1500();
											break;
			case eProductTypeGesher:      m_pSgnlMdNetwork = new CSgnlMdGesher();
											break;
			case eProductTypeNinja:		  m_pSgnlMdNetwork = new CSgnlMdNinja();
											break;
			case eProductTypeCallGeneratorSoftMCU:
										  m_pSgnlMdNetwork = new CSgnlMdSoftCallGenerator();
											break;
			default:	return NULL;
		}

		return m_pSgnlMdNetwork;
	}

	CDnsManagment*		  CNetworkFactory::CreateMngmntDns(eProductType curProductType)
	{
		if(m_pDnsMngmt)
			return m_pDnsMngmt;

		switch(curProductType)
		{
			case eProductTypeRMX1500:
			case  eProductTypeRMX4000:
			case  eProductTypeRMX2000:     m_pDnsMngmt = new CDnsCommonRmxMangment();
											break;

			case  eProductTypeCallGeneratorSoftMCU:
			case  eProductTypeSoftMCU:
			case  eProductTypeSoftMCUMfw:  m_pDnsMngmt = new CDnsSoftMcuManagment();
											break;
			case eProductTypeEdgeAxis:     m_pDnsMngmt = new CDnsEdgeManagment();
											break;
			case eProductTypeGesher:
			case eProductTypeNinja:		   m_pDnsMngmt = new CDnsCommonGesheNinjaMangment();
											break;

			default:	return NULL;
		}

		return m_pDnsMngmt;
	}

	STATUS CNetworkFactory::WriteNetworkConfigurationXML()
	{
		if(!m_pMngntNetwork)
			return NET_FACTORY_STATUS_ERR_MANGMENT_WAS_NOT_CREATED;

		STATUS status = STATUS_OK;
		CNetworkSettings netSettings;
		status = m_pMngntNetwork->WriteManagmentNetwork(netSettings);
		status = netSettings.WriteToFile();
		//testing
		CNetworkSettings netSettingsReader;

		netSettingsReader.LoadFromFile();

		if (netSettingsReader == netSettings)
			FTRACEDEBUG << "Testing netSettingsReader ==correct this  netSettingsReader == netSettings  netSettings is After load";
			else
				FTRACEDEBUG << "netSettingsReader != error netSettingsReader == netSettings should be equal";

		DumpFile(FULL_FILE_NAME_NET_SETTINGS);
		// end testing
		return status;
	}

	CIPService*      CNetworkFactory::ReadMngmntService(STATUS &stat)
	{
		static STATUS mngntFileStatus = STATUS_OK;
		stat = mngntFileStatus;

		//fail to read file from some reason.
		if(STATUS_OK != mngntFileStatus)
			return NULL;
		// Succeed to read file
		if(m_pMngmtService)
			return m_pMngmtService;


		CIPService tmpService;
		mngntFileStatus = tmpService.ReadXmlFile(MANAGEMENT_NETWORK_CONFIG_PATH, eNoActiveAlarm, eRenameFile);
		FTRACEINTO << "\n" << __FUNCTION__ << " - open file " << MANAGEMENT_NETWORK_CONFIG_PATH << " status: " <<CProcessBase::GetProcess()->GetStatusAsString(mngntFileStatus).c_str();
		if(STATUS_OK == mngntFileStatus)
		{
			m_pMngmtService = new CIPService(tmpService);
		}
		stat = mngntFileStatus;
		return m_pMngmtService;
	}

} /* namespace McmsNetworkPackage */
