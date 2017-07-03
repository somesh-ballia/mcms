/*
 * DnsRecordsMngr.cpp
 *
 *  Created on: Jun 29, 2014
 *      Author: vasily
 */


#include "DnsRecordsMngr.h"
#include <iostream>
#include "Trace.h"
#include "TraceStream.h"
#include "PrettyTable.h"
#include "CommonStructs.h"
#include "SystemFunctions.h"
#include "PlcmDNS_Tools.h"



////////////////////////////////////////////////////////////////////////////////////////////////////
DnsRecordsMngr::DnsRecordsMngr()
	: m_dnsRecords("DnsRecordsSharedMemory",READ_WRITE,PLCM_DNS_RECORDS_AMOUNT)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
DnsRecordsMngr::~DnsRecordsMngr()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void DnsRecordsMngr::Dump(std::ostream& os) const
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void DnsRecordsMngr::DumpRecords(std::ostream& os)
{
	std::list<DnsRecord> recordsList;
	ESharedMemStatus  result = m_dnsRecords.Get(predicates::NotEmptyRecord,recordsList);

	PASSERTSTREAM_AND_RETURN(eSharedMem_StatusOk != result,"get failed, status:" << result);

	size_t size = recordsList.size();

	os << "DnsRecordsMngr::DumpRecords - records number: " << m_dnsRecords.EntriesNumber() << ", list size: " << size << ", nextId: " << m_dnsRecords.HeaderData();

	CPrettyTable<DWORD, const char*, WORD, const char*, const char*, char*, char*, DWORD, char*, const char*, const char*> tbl("Id", "Host Name", "Srvc", "Domain", "IP type", "IP", "Time Stamp", "TTL", "Update Time", "Status", "Last Using");

	std::list<DnsRecord>::const_iterator end = recordsList.end();
	for (std::list<DnsRecord>::const_iterator it=recordsList.begin(); it != end; ++it )
	{
		const   DnsRecord& r = *it;
		char    ip[IPV6_ADDRESS_LEN];
		char    szOrderedTimeStamp[32]="";
		char    szUpdateTimeStamp[32]="";
		char    szLastUsingTimeStamp[32]="";

		Pm_GetTimeStringT(r.dwTimeStamp_Responce, E_TIMER_RESOLUTION_SEC, E_TIMER_FORMAT_MONTH, sizeof(szOrderedTimeStamp)-1, szOrderedTimeStamp);
		Pm_GetTimeStringT(r.dwTimeStamp_NextReq, E_TIMER_RESOLUTION_SEC, E_TIMER_FORMAT_MONTH, sizeof(szUpdateTimeStamp)-1, szUpdateTimeStamp);
		Pm_GetTimeStringT(r.dwLastUseTimeStamp, E_TIMER_RESOLUTION_SEC, E_TIMER_FORMAT_MONTH, sizeof(szLastUsingTimeStamp)-1, szLastUsingTimeStamp);

		tbl.Add(r.dwRawId, r.szHostName, r.wServiceId, r.szDomainName,
				(eIpVersion4 == r.sResolveIp.ipVersion) ? "IPv4" : "IPv6",
				(eIpVersion4 == r.sResolveIp.ipVersion) ? (ipV4ToString(r.sResolveIp.addr.v4.ip,ip)) : (ipV6ToString(r.sResolveIp.addr.v6.ip,ip,false)),
				szOrderedTimeStamp, r.dwTTL, szUpdateTimeStamp, ERecordStatus2string(r.eStatus),szLastUsingTimeStamp);
	}
	os << tbl.Get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const char* DnsRecordsMngr::ERecordStatus2string(eDnsReslvSTATUS status)
{
	switch (status)
	{
	case eDnsStatusResolved		: return "RESOLVED";
	case eDnsStatusNotResolved	: return "N.RESOLVED";
	case eDnsStatusTimeOut		: return "TIMEOUT";

	default : return " ??? ";
	}
	return " ??? ";
}


////////////////////////////////////////////////////////////////////////////////////////////////////
const char* DnsRecordsMngr::ESharedMemStatus2string(ESharedMemStatus status)
{
	switch (status)
	{
		case eSharedMem_StatusOk         : return "eSharedMem_StatusOk";
		case eSharedMem_MemoryNotCreated : return "eSharedMem_MemoryNotCreated";
		case eSharedMem_MemoryNotFound   : return "eSharedMem_MemoryNotFound";
		case eSharedMem_EntryNotFound    : return "eSharedMem_EntryNotFound";
		case eSharedMem_BadParameter     : return "eSharedMem_BadParameter";
		case eSharedMem_NoPermissions    : return "eSharedMem_NoPermissions";
		case eSharedMem_MemoryIsFull     : return "eSharedMem_MemoryIsFull";
		case eSharedMem_MemoryIsEmpty    : return "eSharedMem_MemoryIsEmpty";
	}
	return "Unknown value";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ESharedMemStatus DnsRecordsMngr::DnsInsertHostByName( const sDNS_RECORD& rDnsRecord )
{
	DWORD nextId = m_dnsRecords.HeaderData();

	DnsRecord rec(rDnsRecord);
	rec.dwRawId = nextId;

	ESharedMemStatus  result = m_dnsRecords.Add(rec);

	if ( eSharedMem_StatusOk == result )
	{
		TRACEINTO << "record successfully added (HostName:" << rDnsRecord.szHostName << ", SrvcId:" << rDnsRecord.wServiceId << ", DomainName:" << rDnsRecord.szDomainName << ", IpType:" << rDnsRecord.sResolveIp.ipVersion << ")";
		++nextId;
		if (EMPTY_ENTRY == nextId)
			nextId = 1;
		m_dnsRecords.HeaderData(nextId);
	}

	PASSERTSTREAM(eSharedMem_StatusOk != result,"add failed, status:" << result);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ESharedMemStatus DnsRecordsMngr::DnsDeleteHostByNameByIpType( const char *pszHostName, const WORD nServiceId, const DWORD rRequiredIpType )
{
	size_t count = 0;
	ESharedMemStatus  result = m_dnsRecords.Remove(predicates::NameServiceIptypeFunctor(pszHostName,nServiceId,rRequiredIpType),count);

	TRACEINTO << count << " records removed successfully (HostName:" << pszHostName << ", SrvcId:" << nServiceId << ", IpType:" << rRequiredIpType << ")";

	PASSERTSTREAM(eSharedMem_StatusOk != result,"remove failed, status:" << result << ", name:" << pszHostName);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ESharedMemStatus DnsRecordsMngr::DnsDeleteHostByName( const char *pszHostName, const WORD nServiceId )
{
	size_t count = 0;
	ESharedMemStatus  result = m_dnsRecords.Remove(predicates::NameServiceFunctor(pszHostName,nServiceId),count);

	TRACEINTO << count << " records removed successfully (HostName:" << pszHostName << ", SrvcId:" << nServiceId << ")";

	PASSERTSTREAM(eSharedMem_StatusOk != result,"remove failed, status:" << result);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ESharedMemStatus DnsRecordsMngr::DnsDeleteHostByNameAllDnsByIpType( const char *pszHostName, const int rRequiredIpType )
{
	size_t count = 0;
	ESharedMemStatus  result = m_dnsRecords.Remove(predicates::NameIptypeFunctor(pszHostName,rRequiredIpType),count);

	TRACEINTO << count << " records removed successfully (HostName:" << pszHostName << ", IpType:" << rRequiredIpType << ")";

	PASSERTSTREAM(eSharedMem_StatusOk != result,"remove failed, status:" << result);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ESharedMemStatus DnsRecordsMngr::DnsDeleteHostByNameAllDns( const char *pszHostName )
{
	size_t count = 0;
	ESharedMemStatus  result = m_dnsRecords.Remove(predicates::NameFunctor(pszHostName),count);

	TRACEINTO << count << " records removed successfully (HostName:" << pszHostName << ")";

	PASSERTSTREAM(eSharedMem_StatusOk != result,"remove failed, status:" << result);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ESharedMemStatus DnsRecordsMngr::DnsGetHostByName( const char *pszHostName, const WORD nServiceId, const DWORD rRequiredIpType,
		DWORD& rOutBuffSize, sDNS_RECORD* pOutBuff, const bool updateTime /*=false*/ )
{
	std::list<DnsRecord> recordsList;

	ESharedMemStatus  result;
	if ( !updateTime )
	{
		if (enIpVersionMAX == rRequiredIpType)
			result = m_dnsRecords.Get(predicates::NameServiceFunctor(pszHostName,nServiceId),recordsList);
		else
			result = m_dnsRecords.Get(predicates::NameServiceIptypeFunctor(pszHostName,nServiceId,rRequiredIpType),recordsList);
	}
	else
	{
		DWORD  ti = Pm_getCurrentTimestamp(E_TIMER_RESOLUTION_SEC);

		if (enIpVersionMAX == rRequiredIpType)
			result = m_dnsRecords.GetAndUpdate(predicates::NameServiceFunctor(pszHostName,nServiceId),predicates::UpdateTimeFunctor(ti),recordsList);
		else
			result = m_dnsRecords.GetAndUpdate(predicates::NameServiceIptypeFunctor(pszHostName,nServiceId,rRequiredIpType),predicates::UpdateTimeFunctor(ti),recordsList);
	}

	PASSERTSTREAM_AND_RETURN_VALUE(eSharedMem_StatusOk != result,"Get() failed, status:" << result
			<< "; Host:" << pszHostName << "; SrvId:" << nServiceId << "; IpType:" << rRequiredIpType << "; UpdateTime:" << updateTime << ".",result);

	size_t size = recordsList.size();
	TRACEINTO << size << " records retrieved successfully (HostName:" << pszHostName << ", IpType:" << rRequiredIpType << ")";

	size_t count = 0;
	std::list<DnsRecord>::iterator end = recordsList.end();
	for (std::list<DnsRecord>::iterator it=recordsList.begin(); count<rOutBuffSize && count<size && it != end; ++it, ++count )
	{
		memcpy(&pOutBuff[count],&(*it),sizeof(sDNS_RECORD));
	}
	if ( count < rOutBuffSize )
		rOutBuffSize = count;

	return eSharedMem_StatusOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ESharedMemStatus DnsRecordsMngr::DnsGetNameByHost( const ipAddressStruct& rIpHost, const WORD nServiceId, DWORD& rOutBuffSize, char* pszOutName )
{
	DnsRecord record;
	ESharedMemStatus  result = m_dnsRecords.Get(predicates::ServiceHostFunctor(rIpHost,nServiceId),record);

	PASSERTSTREAM_AND_RETURN_VALUE(eSharedMem_StatusOk != result,"Get() failed, status:" << result,result);

	if(NULL != pszOutName)
	{
		memset(pszOutName, '\0', rOutBuffSize);
		strncpy(pszOutName, record.szHostName, rOutBuffSize-1);
        pszOutName[rOutBuffSize-1]='\0';   
		rOutBuffSize = strlen(pszOutName);

		TRACEINTO << "1 record retrieved successfully (HostName:" << pszOutName << ")";
	}
	else
		rOutBuffSize = 0;

	return eSharedMem_StatusOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ESharedMemStatus DnsRecordsMngr::DnsAudit( DnsAuditFun pAuditFunction, const int nDataType, void* pData )
{
	size_t count = 0;
	ESharedMemStatus  result = m_dnsRecords.Use(predicates::AuditFunctor(pAuditFunction,nDataType,pData),false,count);

	TRACEINTO << "Count:" << count << " records updated successfully";

	PASSERTSTREAM_AND_RETURN_VALUE(eSharedMem_StatusOk != result,"audit failed, status:" << result,result);

	return eSharedMem_StatusOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ESharedMemStatus DnsRecordsMngr::DnsAuditBreak( DnsAuditFun pAuditFunction, const int nDataType, void* pData )
{
	size_t count = 0;
	ESharedMemStatus  result = m_dnsRecords.Use(predicates::AuditFunctor(pAuditFunction,nDataType,pData),true,count);

	TRACEINTO << "record updated successfully";

	PASSERTSTREAM_AND_RETURN_VALUE(eSharedMem_StatusOk != result,"audit failed, status:" << result,result);

	return eSharedMem_StatusOk;
}




////////////////////////////////////////////////////////////////////////////////////////////////////
//  Functors
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
bool DnsRecordsMngr::predicates::NameFunctor::operator() (const DnsRecord& record) const
{
	return !strncasecmp(m_name,record.szHostName,sizeof(record.szHostName));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool DnsRecordsMngr::predicates::NameServiceFunctor::operator() (const DnsRecord& record) const
{
	return ( !strncasecmp(m_name,record.szHostName,sizeof(record.szHostName))
			&& ( m_serviceId == record.wServiceId));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool DnsRecordsMngr::predicates::NameIptypeFunctor::operator() (const DnsRecord& record) const
{
	return ( !strncasecmp(m_name,record.szHostName,sizeof(record.szHostName))
			&& ( m_ipType == record.sResolveIp.ipVersion ));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool DnsRecordsMngr::predicates::NameServiceIptypeFunctor::operator() (const DnsRecord& record) const
{
	return ( !strncasecmp(m_name,record.szHostName,sizeof(record.szHostName))
			&& ( m_ipType == record.sResolveIp.ipVersion )
			&& ( m_serviceId == record.wServiceId));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool DnsRecordsMngr::predicates::ServiceHostFunctor::operator() (const DnsRecord& record) const
{
	return ( !memcmp(m_pIpAddress,&record.sResolveIp,sizeof(record.sResolveIp))
			&& ( m_serviceId == record.wServiceId));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void DnsRecordsMngr::predicates::UpdateTimeFunctor::operator() (DnsRecord& record)
{
	record.dwLastUseTimeStamp = m_time;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool DnsRecordsMngr::predicates::AuditFunctor::operator() (DnsRecord& record) const
{
	return (*m_function)(&record,m_nDataType,m_pData);
}











