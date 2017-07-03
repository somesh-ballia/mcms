/*
 * RvgwSslPorts.cpp
 *
 *  Created on: Mar 14, 2012
 *      Author: stanny
 */
#include "RvgwSslPorts.h"
#include <string.h>


const char * mapAliasToPort(const char *pAlias)
{
	for(int i=0;i<MAX_RVGW_PORTS;i++)
	{
		if(strcmp(rvgwAliasNames[i],pAlias)==0)
		{
			return rvgwSslPorts[i];
		}
	}
	return NULL;
}

const char * mapPortToAlias(const char *pPort)
{

	for(int i=0;i<MAX_RVGW_PORTS;i++)
	{
		if(strcmp(rvgwSslPorts[i],pPort)==0)
		{
			return rvgwAliasNames[i];
		}
	}
	return NULL;
}
