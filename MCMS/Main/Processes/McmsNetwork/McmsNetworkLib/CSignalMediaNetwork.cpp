/*
 * CSignalMediaNetwork.cpp
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */



#include "IpService.h"
#include "Trace.h"
#include "TraceStream.h"

#include "CSignalMediaNetwork.h"

namespace McmsNetworkPackage
{

CSignalMediaNetwork::CSignalMediaNetwork() : m_pSignalMediaServiceList(NULL)
{
	// TODO Auto-generated constructor stub


}

CSignalMediaNetwork::~CSignalMediaNetwork()
{
	// TODO Auto-generated destructor stub
	if( NULL != m_pSignalMediaServiceList )
	{
		delete m_pSignalMediaServiceList;
		m_pSignalMediaServiceList = NULL;
	}
}

STATUS CSignalMediaNetwork::ConfigNetworkSgnlMd()
{
	STATUS status = STATUS_OK;
	TRACEINTO << "CSignalMediaNetwork::ConfigNetworkSgnlMd";

	status = OnInitInterfaces();
	if( STATUS_OK != ValidateInitInterfacesStatus(status) )
		return status;

	status = OnPreConfigNetworkSignalMedia();
	if( STATUS_OK != status )
	{
		TRACEINTO << "CSignalMediaNetwork::ConfigNetworkSgnlMd : Return from OnPreConfigNetworkSignalMedia, reason status :"<< GetStatusAsString(status);
		return status;
	}

	status = OnConfigNetworkSignalMedia();
	if( STATUS_OK != status )
	{
		TRACEINTO << "CSignalMediaNetwork::ConfigNetworkSgnlMd : Return from OnConfigNetworkSignalMedia, reason status :"<< GetStatusAsString(status);
		return status;
	}

	status = OnPostConfigNetworkSignalMedia();
	if( STATUS_OK != status )
	{
		TRACEINTO << "CSignalMediaNetwork::ConfigNetworkSgnlMd : Return from OnPostConfigNetworkSignalMedia, reason status :"<< GetStatusAsString(status);
		return status;
	}

	return status;
}

STATUS CSignalMediaNetwork::OnPreConfigNetworkSignalMedia()
{
	STATUS status = STATUS_OK;

	return status;
}
STATUS CSignalMediaNetwork::OnConfigNetworkSignalMedia()
{
	STATUS status = STATUS_OK;

	return status;
}
STATUS CSignalMediaNetwork::OnPostConfigNetworkSignalMedia()
{
	STATUS status = STATUS_OK;

	return status;
}



} /* namespace McmsNetworkPackage */
