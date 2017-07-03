//+==============================================================================================================+
//                                                 IpAddress.h                                                   |
//										 Copyright 2007 Polycom Israel Ltd.                                      |
//                        			             All Rights Reserved.                      				         |
//---------------------------------------------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary information of Polycom Israel Ltd. and    |
// is protected by law. It may not be copied or distributed in any form or medium, disclosed to third parties,   |
// reverse engineered or used in any manner without prior written authorization from Polycom Israel Ltd.         |
//---------------------------------------------------------------------------------------------------------------|
// FILE:       IpAddress.h   						                             			                     |
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

#ifndef IPADDRESS_H_
#define IPADDRESS_H_

#include <netinet/in.h>
#include <typeinfo>
#include <memory>

#include "PObject.h"
#include "DataTypes.h"
#include "IpAddressDefinitions.h"
#include "ObjString.h"

using namespace std;

class CIpAddress;
typedef std::auto_ptr<CIpAddress>  CIpAddressPtr;

class CIpAddress : public CPObject
{
	CLASS_TYPE_1(CIpAddress , CPObject)      
public:
	
	virtual ~CIpAddress() {}

	// A Factory method to create the required ip address type
	static CIpAddressPtr CreateIpAddress(const mcTransportAddress& ipAddress);
	// A Factory method to create the required ip address type
	static CIpAddressPtr CreateIpAddress(const ipAddressStruct& ipAddress);

	const char*		NameOf() const {return "CIpAddress";}
	
	bool			operator==(const CIpAddress& other);
	bool			operator!=(const CIpAddress& other);

	virtual enIpVersion GetVersion() const = 0;
	// set m_ip from a given transport address
	virtual bool 	SetIP(const mcTransportAddress& ipAddress) = 0;
	// fill mcTransportAddress ip out of m_ip
	virtual void 	FillIPAddress(mcTransportAddress& ipAddress) const = 0;
	// fill ipAddressStruct ip out of m_ip
	virtual void 	FillIPAddress(ipAddressStruct& ipAddress) const = 0;
	
	// a method to convert the ip address to string presentation
	// string is entered into m_strPresentation and returned  
	virtual char*	ToString(BOOL addBrackets) = 0;
	
	virtual bool	isNull() const = 0;
	virtual bool	isSameTypeAs(const CIpAddress& other) = 0;
	
	virtual void 	Dump(std::ostream&) const {}	
	
	//virtual int		ToNumeric() = 0;

	static const APIU32 IPV4 = AF_INET;  /* 2  */
	static const APIU32 IPV6 = AF_INET6; /* 10 */
	static const APIU32 STR_PRESENTATION_LEN = 128;

protected:	

	CIpAddress();

	// defined as protected for internal use (by operator== ) 
	virtual bool	isEqual(const CIpAddress& other) const = 0;	
	
//	APIU32	m_ipVersion;	// family
	char	m_strPresentation[STR_PRESENTATION_LEN]; // a string to hold its string presentation 
	
};

//////////////////////////////////////////////////////////////////////////////////////

/*
// Structure describing an Internet (IP) socket address. 
#define __SOCK_SIZE__	16			// sizeof(struct sockaddr)	
struct sockaddr_in {
  sa_family_t			sin_family;	// Address family		
  unsigned short int	sin_port;	// Port number		
  struct in_addr		sin_addr;	// Internet address		

  // Pad to size of `struct sockaddr'. 
  unsigned char			__pad[__SOCK_SIZE__ - sizeof(short int) - sizeof(unsigned short int)
   						- sizeof(struct in_addr)];
};
*/

// 'm_ip' data member of CIpV4Address is actually represent 'sin_addr' of the above system structure
//////////////////////////////////////////////////////////////////////////////////////

class CIpV4Address : public CIpAddress
{
	friend class CIpAddress;
	CLASS_TYPE_1(CIpV4Address , CIpAddress)      
public:
	
	CIpV4Address();
	virtual ~CIpV4Address() {}

	const char*		NameOf() const {return "CIpV4Address";}
		
	virtual enIpVersion GetVersion() const {return eIpVersion4;}
	// set m_ip from a given transport address
	virtual bool 	SetIP(const mcTransportAddress& ipAddress);
	// fill mcTransportAddress ip out of m_ip
	virtual void 	FillIPAddress(mcTransportAddress& ipAddress) const;
	// fill ipAddressStruct ip out of m_ip
	virtual void 	FillIPAddress(ipAddressStruct& ipAddress) const;
	virtual char*	ToString(BOOL addBrackets = 0);	
	
	virtual bool	isNull() const;
	virtual bool	isSameTypeAs(const CIpAddress& other);
	
	APIU32			GetIP() const {return m_ip;}
	virtual void 	Dump(std::ostream&) const;	
	
protected:

	virtual bool isEqual(const CIpAddress& other) const;
		
    APIU32		m_ip; /* IPv4 address */
    
private:
    // In order to avoid wrong ip setting, these constructore are defined as private.
	// They are used internally by CIpAddress factory methods.
	// Creation this class from outside should be done via default constructor
	// and may be set via SetIP() public method
	CIpV4Address(const mcTransportAddress& ipAddress);
	CIpV4Address(const ipAddressStruct& ipAddress);

};

//////////////////////////////////////////////////////////////////////////////////////
/*
struct in6_addr
{
	union 
	{
		__u8		u6_addr8[16];
		__u16		u6_addr16[8];
		__u32		u6_addr32[4];
	} in6_u;
#define s6_addr			in6_u.u6_addr8
#define s6_addr16		in6_u.u6_addr16
#define s6_addr32		in6_u.u6_addr32
};

struct sockaddr_in6 {
	unsigned short int	sin6_family;		// AF_INET6 
	__u16				sin6_port;      	// Transport layer port # 
	__u32				sin6_flowinfo;  	// IPv6 flow information 
	struct in6_addr		sin6_addr;			// IPv6 address 
	__u32				sin6_scope_id;  	// scope id (new in RFC2553) 
};
*/

// 'm_ip' data member of CIpV6Address is actually represent 'sin6_addr' of the above system structure
//////////////////////////////////////////////////////////////////////////////////////

class CIpV6Address : public CIpAddress
{
	friend class CIpAddress;
	CLASS_TYPE_1(CIpV6Address , CIpAddress)
	
public:

	CIpV6Address();
	virtual ~CIpV6Address() {}
	
	const char*		NameOf() const {return "CIpV6Address";}

	virtual enIpVersion GetVersion() const {return eIpVersion6;}
	// set m_ip from a given transport address
	virtual bool 	SetIP(const mcTransportAddress& ipAddress);
	// fill mcTransportAddress ip out of m_ip
	virtual void 	FillIPAddress(mcTransportAddress& ipAddress) const;
	// fill ipAddressStruct ip out of m_ip
	virtual void 	FillIPAddress(ipAddressStruct& ipAddress) const;
	virtual char*	ToString(BOOL addBrackets = 1);

	virtual bool	isNull() const;
	virtual bool	isSameTypeAs(const CIpAddress& other);

	const APIU8*	GetIP() const {return m_ip;}
	APIU32	GetScopId() const {return m_scopeId;}

	virtual void 	Dump(std::ostream&) const;	

protected:

	virtual bool	isEqual(const CIpAddress& other) const;	
	
	APIU8		m_ip[IPV6_ADDRESS_BYTES_LEN]; /* IPv6 address */
    APIU32		m_scopeId;
    
private:
    // In order to avoid wrong ip setting, these constructore are defined as private.
	// They are used internally by CIpAddress factory methods.
	// Creation this class from outside should be done via default constructor
	// and may be set via SetIP() public method
	CIpV6Address(const mcTransportAddress& ipAddress);		
	CIpV6Address(const ipAddressStruct& ipAddress);		
};

//////////////////////////////////////////////////////////////////////////////////////


#endif /*IPADDRESS_H_*/
