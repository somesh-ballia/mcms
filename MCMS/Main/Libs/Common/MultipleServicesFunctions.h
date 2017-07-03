
// MultipleServicesFunctions.h: interface for the SystemFunctions class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MULTIPLESERVICESFUNCTIONS_H___
#define _MULTIPLESERVICESFUNCTIONS_H___


#include "SystemFunctions.h"
#include "DefinesGeneral.h"

#define VLAN_2012_CS_ADDRESS	"169.254.12.10"
#define VLAN_2013_CS_ADDRESS	"169.254.13.10"
#define VLAN_2022_CS_ADDRESS	"169.254.22.10"
#define VLAN_2023_CS_ADDRESS	"169.254.23.10"
#define VLAN_2032_CS_ADDRESS	"169.254.32.10"
#define VLAN_2033_CS_ADDRESS	"169.254.33.10"
#define VLAN_2042_CS_ADDRESS	"169.254.42.10"
#define VLAN_2043_CS_ADDRESS	"169.254.43.10"

#define VLAN_2012_MEDIA_ADDRESS	"169.254.12.67"
#define VLAN_2013_MEDIA_ADDRESS	"169.254.13.67"
#define VLAN_2022_MEDIA_ADDRESS	"169.254.22.68"
#define VLAN_2023_MEDIA_ADDRESS	"169.254.23.68"
#define VLAN_2032_MEDIA_ADDRESS	"169.254.32.69"
#define VLAN_2033_MEDIA_ADDRESS	"169.254.33.69"
#define VLAN_2042_MEDIA_ADDRESS	"169.254.42.70"
#define VLAN_2043_MEDIA_ADDRESS	"169.254.43.70"

#define MS_VLAN_SUBNET_MASK		"255.255.255.0"

/////////////////////////////////////////////////////////////////////////////
static DWORD CalcMSvlanId(const DWORD boardId, const DWORD subBoardId)
{
	DWORD vlan = 0;
	switch(boardId)
	{
		case(FIXED_BOARD_ID_MEDIA_1):
		{
			if (subBoardId == 1)
				vlan = 2012;
			else
				vlan = 2013;
			break;
		}
		case(FIXED_BOARD_ID_MEDIA_2):
		{
			if (subBoardId == 1)
				vlan = 2022;
			else
				vlan = 2023;
			break;
		}
		case(FIXED_BOARD_ID_MEDIA_3):
		{
			if (subBoardId == 1)
				vlan = 2032;
			else
				vlan = 2033;
			break;
		}
		case(FIXED_BOARD_ID_MEDIA_4):
		{
			if (subBoardId == 1)
				vlan = 2042;
			else
				vlan = 2043;
			break;
		}
	}
	return vlan;
}

/////////////////////////////////////////////////////////////////////////////
static DWORD GetVlanCSInternalIpv4Address(const DWORD vlanId)
{
	std::string internalIpv4Address = "";

	switch(vlanId)
	{
		case(2012):
		{
			internalIpv4Address = VLAN_2012_CS_ADDRESS;
			break;
		}
		case(2013):
		{
			internalIpv4Address = VLAN_2013_CS_ADDRESS;
			break;
		}
		case(2022):
		{
			internalIpv4Address = VLAN_2022_CS_ADDRESS;
			break;
		}
		case(2023):
		{
			internalIpv4Address = VLAN_2023_CS_ADDRESS;
			break;
		}
		case(2032):
		{
			internalIpv4Address = VLAN_2032_CS_ADDRESS;
			break;
		}
		case(2033):
		{
			internalIpv4Address = VLAN_2033_CS_ADDRESS;
			break;
		}
		case(2042):
		{
			internalIpv4Address = VLAN_2042_CS_ADDRESS;
			break;
		}
		case(2043):
		{
			internalIpv4Address = VLAN_2043_CS_ADDRESS;
			break;
		}

	}

	DWORD ipv4 = SystemIpStringToDWORD(internalIpv4Address.c_str());
	return ipv4;
}

/////////////////////////////////////////////////////////////////////////////
static DWORD GetVlanCardInternalIpv4Address(const DWORD vlanId)
{
	std::string internalIpv4Address = "";

	switch(vlanId)
	{
		case(2012):
		{
			internalIpv4Address = VLAN_2012_MEDIA_ADDRESS;
			break;
		}
		case(2013):
		{
			internalIpv4Address = VLAN_2013_MEDIA_ADDRESS;
			break;
		}
		case(2022):
		{
			internalIpv4Address = VLAN_2022_MEDIA_ADDRESS;
			break;
		}
		case(2023):
		{
			internalIpv4Address = VLAN_2023_MEDIA_ADDRESS;
			break;
		}
		case(2032):
		{
			internalIpv4Address = VLAN_2032_MEDIA_ADDRESS;
			break;
		}
		case(2033):
		{
			internalIpv4Address = VLAN_2033_MEDIA_ADDRESS;
			break;
		}
		case(2042):
		{
			internalIpv4Address = VLAN_2042_MEDIA_ADDRESS;
			break;
		}
		case(2043):
		{
			internalIpv4Address = VLAN_2043_MEDIA_ADDRESS;
			break;
		}

	}

	DWORD ipv4 = SystemIpStringToDWORD(internalIpv4Address.c_str());
	return ipv4;
}

/////////////////////////////////////////////////////////////////////////////
static eConfigInterfaceType GetSignalingNetworkType(const DWORD boardId, const DWORD subBoardId)
{
	eConfigInterfaceType type = eSignalingNetwork;
	switch(boardId)
	{
		case(FIXED_BOARD_ID_MEDIA_1):
		{
			if (subBoardId == 1)
				type = eSeparatedSignalingNetwork_1_1;
			else
				type = eSeparatedSignalingNetwork_1_2;
			break;
		}
		case(FIXED_BOARD_ID_MEDIA_2):
		{
			if (subBoardId == 1)
				type = eSeparatedSignalingNetwork_2_1;
			else
				type = eSeparatedSignalingNetwork_2_2;
			break;
		}
		case(FIXED_BOARD_ID_MEDIA_3):
		{
			if (subBoardId == 1)
				type = eSeparatedSignalingNetwork_3_1;
			else
				type = eSeparatedSignalingNetwork_3_2;
			break;
		}
		case(FIXED_BOARD_ID_MEDIA_4):
		{
			if (subBoardId == 1)
				type = eSeparatedSignalingNetwork_4_1;
			else
				type = eSeparatedSignalingNetwork_4_2;
			break;
		}
		case(MULTIPLE_SERVICE_GESHER_FAKE_BOARD_ID)://for Gesher
		{
			if (subBoardId == 1)
				type = eMultipleSignalingNetwork_1;
			else if (subBoardId == 2)
				type = eMultipleSignalingNetwork_2;
			else
				type = eMultipleSignalingNetwork_3;
			break;
		}
		case(MULTIPLE_SERVICE_NINJA_FAKE_BOARD_ID)://for Ninja
		{
			if (subBoardId == 1)
				type = eMultipleSignalingNetwork_1;
			else
				type = eMultipleSignalingNetwork_2;
			break;
		}
	}
	return type;
}

#endif
