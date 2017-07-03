/*
 * DnsRecordsMngr.h
 *
 *  Created on: Jun 29, 2014
 *      Author: vasily
 */

#ifndef __DNSRECORDSMNGR_H__
#define __DNSRECORDSMNGR_H__

//#include <string.h>
#include "PObject.h"
#include "DnsSharedMemoryArray.h"
#include "DnsRecord.h"

typedef bool (*DnsAuditFun) (DnsRecord* pRecord, int par_nDataType, void * par_pData);
//typedef bool (*DnsAuditFun) (sDNS_RECORD * par_pRecord, int par_nDataType, void * par_pData);

class DnsRecordsMngr : public CPObject
{
	CLASS_TYPE_1(DnsRecordsMngr, CPObject)

	friend class TestDnsRecordsMngr;

	struct predicates {
		static bool NotEmptyRecord(const DnsRecord& record) { return !record.IsEmpty(); }
		struct NameFunctor
		{
			NameFunctor(const char* name) : m_name(name) {}
			bool operator() (const DnsRecord& record) const;
		private:
			const char* m_name;
		};
		struct NameServiceFunctor
		{
			NameServiceFunctor(const char* name, const WORD serviceId) : m_name(name), m_serviceId(serviceId) {}
			bool operator() (const DnsRecord& record) const;
		private:
			const char* m_name;
			const WORD  m_serviceId;
		};
		struct NameIptypeFunctor
		{
			NameIptypeFunctor(const char* name, const int rRequiredIpType) : m_name(name), m_ipType(rRequiredIpType) {}
			bool operator() (const DnsRecord& record) const;
		private:
			const char* m_name;
			const DWORD m_ipType;
		};
		struct NameServiceIptypeFunctor
		{
			NameServiceIptypeFunctor(const char* name, const WORD serviceId, const int rRequiredIpType) : m_name(name), m_serviceId(serviceId), m_ipType(rRequiredIpType) {}
			bool operator() (const DnsRecord& record) const;
		private:
			const char* m_name;
			const WORD  m_serviceId;
			const DWORD m_ipType;
		};
		struct ServiceHostFunctor
		{
			ServiceHostFunctor(const ipAddressStruct& sIpAddr, const WORD serviceId) : m_pIpAddress(&sIpAddr), m_serviceId(serviceId) {}
			bool operator() (const DnsRecord& record) const;
		private:
			const ipAddressStruct* m_pIpAddress;
			const WORD  m_serviceId;
		};
		struct UpdateTimeFunctor
		{
			UpdateTimeFunctor(const DWORD timeStamp) : m_time(timeStamp) {}
			void operator() (DnsRecord& record);
		private:
			const DWORD  m_time;
		};
		struct AuditFunctor
		{
			AuditFunctor(DnsAuditFun func, const int nDataType, void* pData) : m_function(func), m_nDataType(nDataType), m_pData(pData) {}
			bool operator() (DnsRecord& record) const;
		private:
			DnsAuditFun   m_function;
			int           m_nDataType;
			void*         m_pData;
		};
	};

public:
	DnsRecordsMngr();
	virtual ~DnsRecordsMngr();
	const char* NameOf() const { return "DnsRecordsMngr"; } // override
	void Dump(std::ostream&) const;                         // override
	void DumpRecords(std::ostream&);

	static const char* ESharedMemStatus2string(ESharedMemStatus status);
	static const char* ERecordStatus2string(eDnsReslvSTATUS status);
	// API
	ESharedMemStatus DnsInsertHostByName               ( const sDNS_RECORD& rDnsRecord );
	ESharedMemStatus DnsDeleteHostByNameByIpType       ( const char *pszHostName, const WORD nServiceId, const DWORD rRequiredIpType );
	ESharedMemStatus DnsDeleteHostByName               ( const char *pszHostName, const WORD nServiceId );
	ESharedMemStatus DnsDeleteHostByNameAllDnsByIpType ( const char *pszHostName, const int rRequiredIpType );
	ESharedMemStatus DnsDeleteHostByNameAllDns         ( const char *pszHostName );
	ESharedMemStatus DnsGetHostByName                  ( const char *pszHostName, const WORD nServiceId, const DWORD eRequiredIpType, DWORD& rOutBuffSize, sDNS_RECORD* pOutBuff, const bool updateTime = false );
	ESharedMemStatus DnsGetNameByHost                  ( const ipAddressStruct& rIpHost, const WORD nServiceId, DWORD& rOutBuffSize, char* pszOutName );
	ESharedMemStatus DnsAudit                          ( DnsAuditFun pAuditFunction, const int nDataType, void* pData );
	ESharedMemStatus DnsAuditBreak                     ( DnsAuditFun pAuditFunction, const int nDataType, void* pData );

protected:
	DnsSharedMemoryArray<DnsRecord> m_dnsRecords;
};

inline std::ostream& operator <<(std::ostream& os, ESharedMemStatus status)
{ return os << DnsRecordsMngr::ESharedMemStatus2string(status) << "(" << (int)status << ") "; }



#endif /* __DNSRECORDSMNGR_H__ */
