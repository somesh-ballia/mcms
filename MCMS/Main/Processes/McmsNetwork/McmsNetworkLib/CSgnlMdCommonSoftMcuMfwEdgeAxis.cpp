/*
 * CSgnlMdCommonSoftMcuMfwEdgeAxis.cpp
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */

#include "OsFileIF.h"
#include "Trace.h"
#include "TraceStream.h"
#include "NetCommonDefines.h"

#include "CSgnlMdCommonSoftMcuMfwEdgeAxis.h"

namespace McmsNetworkPackage
{

CSgnlMdCommonSoftMcuMfwEdgeAxis::CSgnlMdCommonSoftMcuMfwEdgeAxis()
{
	// TODO Auto-generated constructor stub

}

CSgnlMdCommonSoftMcuMfwEdgeAxis::~CSgnlMdCommonSoftMcuMfwEdgeAxis()
{
	// TODO Auto-generated destructor stub
}


STATUS CSgnlMdCommonSoftMcuMfwEdgeAxis::ReadConfigurationFile()
{
	STATUS stat = STATUS_OK;
	string ipServiceFileNameTmp;
	ipServiceFileNameTmp = IP_SERVICE_LIST_TMP_PATH;

	if (!IsFileExists(IP_SERVICE_LIST_TMP_PATH))
	{
		TRACESTR(eLevelInfoNormal) << "CSgnlMdCommonSoftMcuMfwEdgeAxis::ReadConfigurationFile IPService Configured file:" << IP_SERVICE_LIST_TMP_PATH << " is not exsit.";
		return PLATFORM_STATUS_SGNLMD_ERR_CONFIG;
	}

	m_pSignalMediaServiceList = new CIPServiceList;
	stat = m_pSignalMediaServiceList->ReadXmlFile(ipServiceFileNameTmp.c_str(), eNoActiveAlarm, eRenameFile);
	if (STATUS_OK != stat)
	{
		TRACESTR(eLevelInfoNormal) << "CSgnlMdCommonSoftMcuMfwEdgeAxis::ReadConfigurationFile ReadXmlFile error.";
		return PLATFORM_STATUS_SGNLMD_ERR_CONFIG;
	}

	return stat;
}

STATUS CSgnlMdCommonSoftMcuMfwEdgeAxis::OnInitInterfaces()
{
	return ReadConfigurationFile();
}

} /* namespace McmsNetworkPackage */


