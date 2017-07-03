/*
 * Utils.cpp
 *
 *  Created on: May 29, 2012
 *      Author: vasily
 */


#include "Utils.h"
#include <sstream>
#include <sys/time.h>
#include <cstdlib>

////////////////////////////////////////////////////////////////////////////////
// Convert string characters to lower case
// Params:
//   IN:  s - any string
// Return:
//   string in lower case
std::string Utils::String2lower(std::string s)
{
	int len = s.length();
	for( int i=0; i<len; i++ )
		s[i] = tolower( (unsigned char) s[i] );
	return s;
}

////////////////////////////////////////////////////////////////////////////////
// Generate UUID string according to specification of random UUID type (version 4)
//   http://en.wikipedia.org/wiki/Universally_Unique_Identifier
// Return:
//   UUID string
std::string Utils::GenerateUid()
{
	// well formed UUIDs examples:
	//   b8b1100e-4c00-4049-bf69-b3fd8aeb0591
	//   86cdf691-da93-44be-880c-7abd17b6b84b
	//   533a770d-fb97-4916-8c06-cf40ac58fd92
	//   ab4b5309-1851-4e51-85bb-7433953075c9
	//   bb230bc7-eccc-434f-b628-26c28b6941ce

	SeedRand();

	std::stringstream sUuidStream;
	for( int i=0; i<32; i++ )
	{
		if( 8 == i || 12 == i || 16 == i || 20 == i )
			sUuidStream << "-";
		if( 12 == i )
		{
			sUuidStream << "4";
		}
		else if( 16 == i )
		{
			sUuidStream << std::hex << GetRandomFromRange(8,0xB);
		}
		else
			sUuidStream << std::hex << GetRandomFromRange(0,0xF);
	}

	return sUuidStream.str();
}

////////////////////////////////////////////////////////////////////////////////
// Initializes randomize mechanism with random value
void Utils::SeedRand(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	srand(tv.tv_sec * tv.tv_usec);
}

////////////////////////////////////////////////////////////////////////////////
// Generate random number from range
// Params:
//   IN:  minVal - minimal value in range
//   IN:  maxVal - maximal value in range
// Return:
//   random number from range
int Utils::GetRandomFromRange(const int minVal, const int maxVal)
{
	if(minVal<0 || maxVal <= minVal)
		return -1;
	return minVal + (rand() % (maxVal-minVal+1));
}

std::string Utils::IpAddressToString(mcTransportAddress& transAddr, bool bWithPrefix /*= false*/)
{
	std::ostringstream ossIpAddress;
	if (transAddr.ipVersion == eIpVersion6)
	{
		char ipAddr[40];
		BYTE* pIp = transAddr.addr.v6.ip;
		snprintf(ipAddr,sizeof ipAddr,"%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x",
				pIp[0], pIp[1], pIp[2],  pIp[3],  pIp[4],  pIp[5],  pIp[6],  pIp[7],
				pIp[8], pIp[9], pIp[10], pIp[11], pIp[12], pIp[13], pIp[14], pIp[15]);
		if (bWithPrefix)
			ossIpAddress << "IPv6 address = ";
		ossIpAddress << ipAddr;
	}
	else if (transAddr.ipVersion == eIpVersion4)
	{
		char ipAddr[16];
		DWORD dwIP = transAddr.addr.v4.ip;
		snprintf(ipAddr,sizeof ipAddr,"%u.%u.%u.%u" ,(dwIP & 0x000000ff)
													,(dwIP & 0x0000ff00) >> 8
													,(dwIP & 0x00ff0000) >> 16
													,(dwIP & 0xff000000) >> 24);
		if (bWithPrefix)
			ossIpAddress << "IPv4 address = ";
		ossIpAddress << ipAddr;
	}
	return ossIpAddress.str();
}




