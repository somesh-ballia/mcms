#ifndef _IFCONFIG_H_
#define _IFCONFIG_H_


#define  IFCONFIG_OK 				0
#define  IFCONFIG_NI_NOTEXIST		1
#define  IFCONFIG_SOCKET_FAIL 		2
#define  IFCONFIG_IOCTL_FAIL		3
#define  IFCONFIG_NAMEINDEX_FAIL	4


#include <list>


#include "DataTypes.h"
#include "ConfigManagerApi.h"



// Global routines
//----------------
//---------------------------------------------------------------------------
//
//  Function name:  AcAddNi
//
//  Description: Add network interface
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------
WORD AcAddNi(
	char 	*pIfName,	// IN
	DWORD	ipAddress,	// IN
	DWORD	subnetMask, // IN
	DWORD	broadcast,  // IN
	DWORD   default_gw, // IN
	std::list<CRouter> &routerList, // IN
	char 	*route_dev,
	DWORD	&ret);// IN

//---------------------------------------------------------------------------
//
//  Function name:  AcAddNi6
//
//  Description: Add network interface IPv6
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------
DWORD AcAddNi6(
	char 	*pIfName,	// IN
	char	*ipAddress,	// IN
	char	*ipPrefix, // IN
	char 	*default_gw,
	char 	*gwPrefix);// IN
	
//---------------------------------------------------------------------------
//
//  Function name:  AcDelNi
//
//  Description: Delete network interface
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------

int AcDelNi(	
	char 	*pIfName,	// IN
	DWORD	ipAddress,	// IN
	DWORD	subnetMask, // IN
	int		mtu);		// IN




//---------------------------------------------------------------------------
//
//  Function name:  IsNiExist
//
//  Description: check the existance of a network interface
//
//  Return code:
//          IFCONFIG_OK		 		- success
//          IFCONFIG_SOCKET_FAIL	- error
//			IFCONFIG_IOCTL_FAIL		
//
//
//	DOES NOT WORK. It does not recognizes interfaces that configurator added.
//
//---------------------------------------------------------------------------
int IsNiExist(const char *pIfName,	// IN 
			  bool * result);  		// OUT 

//---------------------------------------------------------------------------
//
//  Function name:  GetNiIpV4Addr
//
//  Description: get ip address of network interface
//
//  Return code:
//          IFCONFIG_OK		 		- success
//          IFCONFIG_SOCKET_FAIL	- error
//			IFCONFIG_IOCTL_FAIL		
//
//---------------------------------------------------------------------------
int GetNiIpV4Addr(const char *pIfName,   // IN 
				DWORD *ppIpAddr);      // OUT
				

int GetNiIpV6Addr(const char *pIfName,   // IN 
				  char 	*pIPv6Addr,      // OUT
				  char 	*sRetGetIdx,     // OUT
				  char 	*sRetGetAdd);    // OUT


//---------------------------------------------------------------------------
//
//  Function name:  AddDefaultGw
//
//  Description: Add gw to route
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------				
int AddDefaultGw(struct ifreq *pIfr, DWORD gw);

//---------------------------------------------------------------------------
//
//  Function name:  AddIPv6DefaultGw
//
//  Description: Add gw to route IPv6
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------	
int AddIPv6DefaultGw(int fd, char *default_gw, int index, DWORD prefix_len);
//---------------------------------------------------------------------------
//
//  Function name:  AddIpRoute
//
//  Description: Add route entrystatic int SetIpv6Address(struct ifreq *pIfr, APIU8[] ipAddress)
//
//  Return code:
//          0                   - success
//          non zero value      - error
//
//---------------------------------------------------------------------------
int	AddIpRoute(DWORD target_ip, 
				DWORD submask,
				DWORD gateway, 
				DWORD type, 
				char* dev);

#endif //_ACIFCONFIG_H_
