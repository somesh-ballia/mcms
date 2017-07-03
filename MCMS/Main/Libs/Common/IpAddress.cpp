//+==============================================================================================================+
//                                                 IpAddress.cpp                                                 |
//										 Copyright 2007 Polycom Israel Ltd.                                      |
//                        			             All Rights Reserved.                      				         |
//---------------------------------------------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary information of Polycom Israel Ltd. and    |
// is protected by law. It may not be copied or distributed in any form or medium, disclosed to third parties,   |
// reverse engineered or used in any manner without prior written authorization from Polycom Israel Ltd.         |
//---------------------------------------------------------------------------------------------------------------|
// FILE:       IpAddress.cpp   						                             			                     |
// SUBSYSTEM:  MCMS                                 									                         |
// PROGRAMMER: Oren M.                                                     										 |
//---------------------------------------------------------------------------------------------------------------|
//  Date       | 09/09/07			                         									                 |
//---------------------------------------------------------------------------------------------------------------|
// Description:			       												    	   						     |
//-------------																   								     |
// These classes function as a wrapper classes of the well known ip structures, representing ip addresses        |
// in the system. 							  	  																 |
// They wrap 'mcTransportAddress' and 'ipAddressStruct' union structure and encapsulate only the union section   | 
// of these structures. In other words, it encapsulate "ipAddressIf" union which contain						 | 
// ipAddressV4If/ipAddressV6If structure.																		 |
// Rest mcTransportAddress properties(port, distribution, distribution) may be added later (if will be required).|
// CIpAddress (the base class) is implemented as a Factory class and contains a factory method "CreateIpAddress" |
// which receive one of the mentioned above structure, decides which type of ip address (CIpV4Address or		 |
// CIpV6Address) to create, creates the appropriate address and return an 'auto_ptr' to it.						 |
// The user is free of releasing the created object memory!														 |
// If the given structure is field with an empty address, a null 'auto_ptr' is returned.						 |
// In order to verify if an ipAddress was allocated, you must check the returned 'auto_ptr' value (by get()).    | 
// 'CIpAddressPtr' typedef shall be used to keep the returned 'auto_ptr'.										 |
//																												 |
//	Typical use of the factory:																					 |
//	mcTransportAddress struct_ip1, struct_ip3; 																	 |
//  ipAddressStruct    struct_ip2, struct_ip1;																	 |
//	CIpAddressPtr ip1 = CIpAddress::CreateIpAddress(struct_ip1);												 |
//	CIpAddressPtr ip2 = CIpAddress::CreateIpAddress(struct_ip2);												 |
//																												 |
//  // fill struct_ip with ip details																			 |
//  if(ip1.get())	ip1->FillIPAddress(struct_ip3);																 |
//  if(ip2.get())	ip2->FillIPAddress(struct_ip4);																 |
//																												 |
//+==============================================================================================================+

#include <iomanip>
#include "IpAddress.h"
#include "SystemFunctions.h"

extern const char* IPv6ScopeIdToString(enScopeId theScopeId);

//////////////////////////////////////////////////////////////////////////////////////
//									CIpAddress										//
//////////////////////////////////////////////////////////////////////////////////////
CIpAddress::CIpAddress() 
{
	memset(m_strPresentation,0,STR_PRESENTATION_LEN);
}

//////////////////////////////////////////////////////////////////////////////////////
bool CIpAddress::operator==(const CIpAddress& other)
{
	return ((typeid(*this) == typeid(other)) &&	isEqual(other));
}

//////////////////////////////////////////////////////////////////////////////////////
bool CIpAddress::operator!=(const CIpAddress& other)
{
	return ((typeid(*this) != typeid(other)) ||	!isEqual(other));
}

//////////////////////////////////////////////////////////////////////////////////////
// A Factory method which returns an instance of specific IpAddress class
// Note: It returns an auto_ptr to a newed instance. Memory will be released automatically 
CIpAddressPtr CIpAddress::CreateIpAddress( const mcTransportAddress& ipAddress)
{
	CIpAddressPtr smart_ip_ptr;
	if (ipAddress.ipVersion == eIpVersion4)
		smart_ip_ptr = CIpAddressPtr(new CIpV4Address(ipAddress));
	else if (ipAddress.ipVersion == eIpVersion6)
		smart_ip_ptr = CIpAddressPtr(new CIpV6Address(ipAddress));

	// if empty CIpAddress was allocated (structure was empty)
	// release smart pointer. this pointer may be compared with null
	if( smart_ip_ptr.get() )
	{
	     if(smart_ip_ptr->isNull())
	     {
	     	smart_ip_ptr.reset(0);
	     	smart_ip_ptr.release(); 
	     }
	}
	return smart_ip_ptr;	
}

//////////////////////////////////////////////////////////////////////////////////////
// A Factory method which creates the required ip address type
CIpAddressPtr CIpAddress::CreateIpAddress(const ipAddressStruct& ipAddress)
{
	CIpAddressPtr smart_ip_ptr;
	if (ipAddress.ipVersion == eIpVersion4)
		smart_ip_ptr = CIpAddressPtr(new CIpV4Address(ipAddress));
	else if (ipAddress.ipVersion == eIpVersion6)
		smart_ip_ptr = CIpAddressPtr(new CIpV6Address(ipAddress));

	// if empty CIpAddress was allocated (structure was empty)
	// release smart pointer. this pointer may be compared with null
	if(smart_ip_ptr.get()) 
	{
		if(smart_ip_ptr->isNull())
		{
		        //printf("smart_ip_ptr->isNull(): %p\n", smart_ip_ptr.get());
			smart_ip_ptr.reset(0);
			smart_ip_ptr.release();
		
			//smart_ip_ptr.release();
			//smart_ip_ptr.reset(0);
		}
	}
	return smart_ip_ptr;	
}

//////////////////////////////////////////////////////////////////////////////////////
//									CIpV4Address									//
//////////////////////////////////////////////////////////////////////////////////////
CIpV4Address::CIpV4Address() 
{
	m_ip = 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// structure validity check should be done befor calling this constructor
CIpV4Address::CIpV4Address(const mcTransportAddress& ipAddress)
{
	m_ip = ipAddress.addr.v4.ip;
	ToString();
}

//////////////////////////////////////////////////////////////////////////////////////
// structure validity check should be done befor calling this constructor
CIpV4Address::CIpV4Address(const ipAddressStruct& ipAddress)
{
	m_ip = ipAddress.addr.v4.ip;
	ToString();
}

//////////////////////////////////////////////////////////////////////////////////////
// convert binary ip address to string, set strPresentation and return it  
char* CIpV4Address::ToString(BOOL addBrackets)
{
	//(char* sIpAddress, BOOL addBrackets)
	memset(m_strPresentation, 0, STR_PRESENTATION_LEN);
	::SystemDWORDToIpString(m_ip, m_strPresentation);
	
	return m_strPresentation;
}

//////////////////////////////////////////////////////////////////////////////////////
/*
char* CIpV4Address::ToNumeric() 
{
	//(mcTransportAddress *ipAddress, char *sIpAddress)
	return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////
bool CIpV4Address::SetIP(const mcTransportAddress& ipAddress)
{
	if (ipAddress.ipVersion == eIpVersion4)
	{
		m_ip = ipAddress.addr.v4.ip;
		ToString();
		return true;
	}
	DBGPASSERT(1);
	PTRACE(eLevelError,"CIpV4Address::SetIP. Trying to set an invalid ip address!!");
	return false;	
}

//////////////////////////////////////////////////////////////////////////////////////
// fill mcTransportAddress ip (and version) with "this" ip & version
void CIpV4Address::FillIPAddress(mcTransportAddress& ipAddress) const
{	
	ipAddress.ipVersion		= eIpVersion4;
	ipAddress.addr.v4.ip	= m_ip;	
}

//////////////////////////////////////////////////////////////////////////////////////
// fill ipAddressStruct ip (and version) with "this" ip & version
void CIpV4Address::FillIPAddress(ipAddressStruct& ipAddress) const
{
	ipAddress.ipVersion		= eIpVersion4;
	ipAddress.addr.v4.ip	= m_ip;		
}

//////////////////////////////////////////////////////////////////////////////////////
bool CIpV4Address::isEqual(const CIpAddress& other) const
{
	const CIpV4Address& ipv4 = static_cast<const CIpV4Address&>(other);
	return m_ip == ipv4.m_ip;
}

//////////////////////////////////////////////////////////////////////////////////////
bool CIpV4Address::isNull() const
{
	return m_ip == 0;
}

//////////////////////////////////////////////////////////////////////////////////////
bool CIpV4Address::isSameTypeAs(const CIpAddress& other)
{
	return ( (typeid(*this) == typeid(other)) );	
}

//////////////////////////////////////////////////////////////////////////////////////
void CIpV4Address::Dump(std::ostream& os) const 
{
	os << "Type:         IPv4"	<< std::endl;	
	os << "Ip:           "		<< m_strPresentation << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////
//									CIpV6Address									//
//////////////////////////////////////////////////////////////////////////////////////
CIpV6Address::CIpV6Address()
{
	memset(m_ip,0,IPV6_ADDRESS_BYTES_LEN);
}

//////////////////////////////////////////////////////////////////////////////////////
// structure validity check should be done befor calling this constructor
CIpV6Address::CIpV6Address(const mcTransportAddress& ipAddress)
{
	memcpy(m_ip, ipAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
	m_scopeId = ipAddress.addr.v6.scopeId;
	ToString(1);
}

//////////////////////////////////////////////////////////////////////////////////////
// structure validity check should be done befor calling this constructor
CIpV6Address::CIpV6Address(const ipAddressStruct& ipAddress)
{
	memcpy(m_ip, ipAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
	m_scopeId = ipAddress.addr.v6.scopeId;
	ToString();
}

//////////////////////////////////////////////////////////////////////////////////////
// convert binary ip address to string, set strPresentation and return it  
char* CIpV6Address::ToString(BOOL addBrackets)
{
	memset(m_strPresentation, 0, STR_PRESENTATION_LEN);
	::ipV6ToString(m_ip, m_strPresentation, addBrackets);
	
	return m_strPresentation;
}

//////////////////////////////////////////////////////////////////////////////////////
/*
int CIpV6Address::ToNumeric() 
{
//	return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////
bool CIpV6Address::SetIP(const mcTransportAddress& ipAddress)
{
	if (ipAddress.ipVersion == eIpVersion6)
	{
		memcpy(m_ip, ipAddress.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN);
		m_scopeId = ipAddress.addr.v6.scopeId;
		ToString(1);
		return true;
	}
	DBGPASSERT(1);
	PTRACE(eLevelError,"CIpV6Address::SetIP. Trying to set an invalid ip address!!");
	return false;	
}

//////////////////////////////////////////////////////////////////////////////////////
// fill mcTransportAddress ip (and version) with "this" ip & version
void CIpV6Address::FillIPAddress(mcTransportAddress& ipAddress) const
{	
	ipAddress.ipVersion = eIpVersion6;
	memcpy(ipAddress.addr.v6.ip, m_ip, IPV6_ADDRESS_BYTES_LEN);
	ipAddress.addr.v6.scopeId = m_scopeId;
}

//////////////////////////////////////////////////////////////////////////////////////
// fill ipAddressStruct ip (and version) with "this" ip & version
void CIpV6Address::FillIPAddress(ipAddressStruct& ipAddress) const
{
	ipAddress.ipVersion = eIpVersion6;
	memcpy(ipAddress.addr.v6.ip, m_ip, IPV6_ADDRESS_BYTES_LEN);
	ipAddress.addr.v6.scopeId = m_scopeId;
}

//////////////////////////////////////////////////////////////////////////////////////
bool CIpV6Address::isEqual(const CIpAddress& other) const
{
	const CIpV6Address& ipv6 = static_cast<const CIpV6Address&>(other);

	if (!memcmp(m_ip, ipv6.m_ip, IPV6_ADDRESS_BYTES_LEN))
		return true;	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////
bool CIpV6Address::isNull() const
{
	APIU8	nullIP[16] 	= {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
					  	   0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
						  	
	if (!memcmp(m_ip, nullIP, IPV6_ADDRESS_BYTES_LEN)) return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////
bool CIpV6Address::isSameTypeAs(const CIpAddress& other)
{
	if(typeid(*this) == typeid(other))
	{
		const CIpV6Address& ipv6 = static_cast<const CIpV6Address&>(other);
		if( m_scopeId == ipv6.m_scopeId)
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////
void CIpV6Address::Dump(std::ostream& os) const 
{
	os << "Type:         IPv6"	<< std::endl;	
	os << "Ip:           "		<< m_strPresentation << std::endl;
	os << "Scope id:     "		<< IPv6ScopeIdToString((enScopeId)m_scopeId) << std::endl;	
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
