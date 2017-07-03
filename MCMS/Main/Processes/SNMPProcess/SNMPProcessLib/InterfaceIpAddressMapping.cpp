

#include "InterfaceIpAddressMapping.h"
#include "TraceStream.h"

CInterfaceIndexIpAddressMap& CInterfaceIndexIpAddressMapping::GetInterfaceIpAddressMap()
{    
    return m_interfaceIndexIpAddressMap;
}

const InterfaceIpAddress* CInterfaceIndexIpAddressMapping::GetIpAddressEntryByInterface(eInterfaceIndex eIfFace) const 
{       
    CInterfaceIndexIpAddressMap::const_iterator itMap = m_interfaceIndexIpAddressMap.find(eIfFace);
    if (itMap == m_interfaceIndexIpAddressMap.end() )
    {
    	return NULL;
    }
    
    return &itMap->second;
}

InterfaceIpAddress* CInterfaceIndexIpAddressMapping::GetOrCreateIpAddressEntryByInterface(eInterfaceIndex eIfFace)  
{	
    CInterfaceIndexIpAddressMap::const_iterator itMap = m_interfaceIndexIpAddressMap.find(eIfFace);
    if (itMap == m_interfaceIndexIpAddressMap.end() )
    {
    	FTRACEINTO << "Creating new Entry for index " << (int)eIfFace;		
    	m_interfaceIndexIpAddressMap[eIfFace] = InterfaceIpAddress();		    		   
    }
    return &m_interfaceIndexIpAddressMap[eIfFace];
    
}

void CInterfaceIndexIpAddressMapping::SetIpv4NetworkAddress(eInterfaceIndex eIfFace, APIU32 networkIp4Address)
{
	InterfaceIpAddress*  outInterfaceIpAddress = GetOrCreateIpAddressEntryByInterface(eIfFace);
	FPASSERTMSG_AND_RETURN(!outInterfaceIpAddress, "SetIpv4NetworkAddress failed create entry");
		
	outInterfaceIpAddress->SetNetworkIp4Address(networkIp4Address);
}

void CInterfaceIndexIpAddressMapping::SetIpV6Address(eInterfaceIndex eIfFace, const byte*  ipv6Address)
{
	InterfaceIpAddress*  outInterfaceIpAddress = GetOrCreateIpAddressEntryByInterface(eIfFace);
			
	FPASSERTMSG_AND_RETURN(!outInterfaceIpAddress, "SetIpV6Address failed create entry");
	outInterfaceIpAddress->SetIpv6Address(ipv6Address);
}


