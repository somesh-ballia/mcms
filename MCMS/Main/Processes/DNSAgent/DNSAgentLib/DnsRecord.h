/*
 * DnsRecord.h
 *
 *  Created on: Jun 29, 2014
 *      Author: vasily
 */

#ifndef __DNSRECORD_H__
#define __DNSRECORD_H__


#include "IpAddressDefinitions.h"


class DnsRecord : public sDNS_RECORD
{
public:
	DnsRecord();
	DnsRecord(const sDNS_RECORD& r);
	DnsRecord(const DnsRecord& r);
	~DnsRecord() {}

	DnsRecord& operator=(const DnsRecord& r);
	void Clean();
	bool IsEmpty() const;
};

#endif /* __DNSRECORD_H__ */
