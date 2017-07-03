// 
// ==========================================
//    Copyright (C) 2014           "POLYCOM"
// ==========================================
// FileName:               PlcmDNS_Packet.h  
// Include line recommended:
// #include "PlcmDNS_Packet.h"  //
// 
// ==========================================

#ifndef _PLCMDNS_PACKET_H_
#define _PLCMDNS_PACKET_H_

#include "McmsProcesses.h"

//--------|-----------|---------------|-------------------------------------------------
//Type		  Value    Defining 		Description	Function
//          (decimal)    RFC
//--------|-----------|---------------|-------------------------------------------------
//A				1		RFC 1035[1]		Address record	Returns a 32-bit IPv4 address, most commonly used to map hostnames to an IP address of the host, but also used for DNSBLs, storing subnet masks in RFC 1101, etc.
//AAAA			28		RFC 3596[2]		IPv6 address record	Returns a 128-bit IPv6 address, most commonly used to map hostnames to an IP address of the host.
//AFSDB			18		RFC 1183		AFS database record	Location of database servers of an AFS cell. This record is commonly used by AFS clients to contact AFS cells outside their local domain. A subtype of this record is used by the obsolete DCE/DFS file system.
//APL			42		RFC 3123		Address Prefix List	Specify lists of address ranges, e.g. in CIDR format, for various address families. Experimental.
//CAA			257		RFC 6844		Certification Authority Authorization	CA pinning, constraining acceptable CAs for a host/domain
//CERT			37		RFC 4398		Certificate record	Stores PKIX, SPKI, PGP, etc.
//CNAME			5		RFC 1035[1]		Canonical name record	Alias of one name to another: the DNS lookup will continue by retrying the lookup with the new name.
//DHCID			49		RFC 4701		DHCP identifier	Used in conjunction with the FQDN option to DHCP
//DLV			32769	RFC 4431		DNSSEC Lookaside Validation record	For publishing DNSSEC trust anchors outside of the DNS delegation chain. Uses the same format as the DS record. RFC 5074 describes a way of using these records.
//DNAME			39		RFC 2672		Delegation Name	DNAME creates an alias for a name and all its subnames, unlike CNAME, which aliases only the exact name in its label. Like the CNAME record, the DNS lookup will continue by retrying the lookup with the new name.
//DNSKEY		48		RFC 4034		DNS Key record	The key record used in DNSSEC. Uses the same format as the KEY record.
//DS			43		RFC 4034		Delegation signer	The record used to identify the DNSSEC signing key of a delegated zone
//HIP			55		RFC 5205		Host Identity Protocol	Method of separating the end-point identifier and locator roles of IP addresses.
//IPSECKEY		45		RFC 4025		IPsec Key	Key record that can be used with IPsec
//KEY			25		RFC 2535[3] 	and RFC 2930[4]	Key record	Used only for SIG(0) (RFC 2931) and TKEY (RFC 2930).[5] RFC 3445 eliminated their use for application keys and limited their use to DNSSEC.[6] RFC 3755 designates DNSKEY as the replacement within DNSSEC.[7] RFC 4025 designates IPSECKEY as the replacement for use with IPsec.[8]
//KX			36		RFC 2230		Key eXchanger record	Used with some cryptographic systems (not including DNSSEC) to identify a key management agent for the associated domain-name. Note that this has nothing to do with DNS Security. It is Informational status, rather than being on the IETF standards-track. It has always had limited deployment, but is still in use.
//LOC			29		RFC 1876		Location record	Specifies a geographical location associated with a domain name
//MX			15		RFC 1035[1]		Mail exchange record	Maps a domain name to a list of message transfer agents for that domain
//NAPTR			35		RFC 3403		Naming Authority Pointer	Allows regular expression based rewriting of domain names which can then be used as URIs, further domain names to lookups, etc.
//NS			2		RFC 1035[1]		Name server record	Delegates a DNS zone to use the given authoritative name servers
//NSEC			47		RFC 4034		Next-Secure record	Part of DNSSEC—used to prove a name does not exist. Uses the same format as the (obsolete) NXT record.
//NSEC3			50		RFC 5155		NSEC record version 3	An extension to DNSSEC that allows proof of nonexistence for a name without permitting zonewalking
//NSEC3PARAM	51		RFC 5155		NSEC3 parameters	Parameter record for use with NSEC3
//PTR			12		RFC 1035[1]		Pointer record	Pointer to a canonical name. Unlike a CNAME, DNS processing does NOT proceed, just the name is returned. The most common use is for implementing reverse DNS lookups, but other uses include such things as DNS-SD.
//RRSIG			46		RFC 4034		DNSSEC signature	Signature for a DNSSEC-secured record set. Uses the same format as the SIG record.
//RP			17		RFC 1183		Responsible person	Information about the responsible person(s) for the domain. Usually an email address with the @ replaced by a .
//SIG			24		RFC 2535		Signature	Signature record used in SIG(0) (RFC 2931) and TKEY (RFC 2930).[7] RFC 3755 designated RRSIG as the replacement for SIG for use within DNSSEC.[7]
//SOA			6		RFC 1035[1] 	Start of [a zone of] authority record	Specifies authoritative information about a DNS zone, including the primary name server, the email of the domain administrator, the domain serial number, and several timers relating to refreshing the zone.
//						+RFC 2308[9]	
//SPF			99		RFC 4408		Sender Policy Framework	Specified as part of the SPF protocol as an alternative to storing SPF data in TXT records, using the same format. It was later found[10] that the majority of SPF deployments lack proper support for this record type, and support for it was discontinued.[11]
//SRV			33		RFC 2782		Service locator	Generalized service location record, used for newer protocols instead of creating protocol-specific records such as MX.
//SSHFP			44		RFC 4255		SSH Public Key Fingerprint	Resource record for publishing SSH public host key fingerprints in the DNS System, in order to aid in verifying the authenticity of the host. RFC 6594 defines ECC SSH keys and SHA-256 hashes. See the IANA SSHFP RR parameters registry for details.
//TA			32768	N/A				DNSSEC Trust Authorities	Part of a deployment proposal for DNSSEC without a signed DNS root. See the IANA database and Weiler Spec for details. Uses the same format as the DS record.
//TKEY			249		RFC 2930		Secret key record	A method of providing keying material to be used with TSIG that is encrypted under the public key in an accompanying KEY RR.[12]
//TLSA			52		RFC 6698		TLSA certificate association	A record for DNS-based Authentication of Named Entities (DANE). RFC 6698 defines "The TLSA DNS resource record is used to associate a TLS server certificate or public key with the domain name where the record is found, thus forming a 'TLSA certificate association'".
//TSIG			250		RFC 2845		Transaction Signature	Can be used to authenticate dynamic updates as coming from an approved client, or to authenticate responses as coming from an approved recursive name server[13] similar to DNSSEC.
//TXT			16		RFC 1035[1]		Text record	Originally for arbitrary human-readable text in a DNS record. Since the early 1990s, however, this record more often carries machine-readable data, such as specified by RFC 1464, opportunistic encryption, Sender Policy Framework, DKIM, DMARC, DNS-SD, etc.



#pragma pack(1)

typedef struct _DNS_PACKET_HEADER
{
	short  wId        ;
	short  wFlags     ; // 0x0100
	short  wQdCount   ;
	short  wAnCount   ;
	short  wNsCount   ;
	short  wArCount   ;
}
DNS_PACKET_HEADER;

typedef struct _DNS_REPLY_DATA
{
	unsigned short  wType;
	unsigned short  wClass;
	unsigned int    dwTTL;
	unsigned short  wDataLen;
}
DNS_REPLY_DATA;

typedef struct _DNS_REQUEST_TYPE_CLASS
{
	unsigned short  wType;
	unsigned short  wClass;
}
DNS_REQUEST_TYPE_CLASS;

typedef struct _DNS_REPLY_DATA_ADDITIONAL
{
	unsigned short  wNameOffset;
	unsigned short  wType;
	unsigned short  wClass;
	unsigned int    dwTTL;
	unsigned short  wDataLen;
}
DNS_REPLY_DATA_ADDITIONAL;

typedef enum eDNS_ADVANCED
{
	 AUTORIZ_INFO		= 1
	,ADDITIONAL_INFO
}
eDNS_ADVANCED;

typedef enum _DNS_REQ_RES_TYPE
{
    eDNS_TYPE_UNDEF     = 0x00
   ,eDNS_TYPE_A_IPv4    = 0x01
   ,eDNS_TYPE_AAAA_IPv6 = 0x1c
   ,eDNS_TYPE_SRV		= 0x21
}
DNS_REQ_RES_TYPE;


#pragma pack(0)

#endif //_PLCMDNS_PACKET_H_


