/*
 * DnsRecord.cpp
 *
 *  Created on: Jun 29, 2014
 *      Author: vasily
 */




#include "DnsRecord.h"

#include <string.h>
#include <stdio.h>
#include "SharedHeader.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	unsigned int		dwRawId									;
	char				szHostName	[PLCM_DNS_HOST_NAME_SIZE]	;
	char				szDomainName[PLCM_DNS_DOMAIN_NAME_SIZE]	;
	ipAddressStruct		sResolveIp								;
	unsigned int		dwStatus								;
	unsigned int		dwTimeStamp_Responce					;
	unsigned int		dwTTL									;
	unsigned int		dwTimeStamp_NextReq						;
	ipAddressStruct		sDnsAddress								;
	char				Reserve [28]							;
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
DnsRecord::DnsRecord()
{
	memset(this,0,sizeof(DnsRecord));
	dwRawId = EMPTY_ENTRY;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
DnsRecord::DnsRecord(const DnsRecord& r)
{
	*this = r;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
DnsRecord::DnsRecord(const sDNS_RECORD& r)
{
	memcpy(static_cast<sDNS_RECORD*>(this),&r,sizeof(sDNS_RECORD));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
DnsRecord& DnsRecord::operator=(const DnsRecord& r)
{
	if (this != &r)
		memcpy(this,&r,sizeof(DnsRecord));
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void DnsRecord::Clean()
{
	dwRawId = EMPTY_ENTRY;
	szHostName[0] = '\0';
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool DnsRecord::IsEmpty() const
{
	return (szHostName[0] == '\0' || dwRawId == EMPTY_ENTRY);
}








