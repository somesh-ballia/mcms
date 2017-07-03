// SharedMcmsCardsStructs.h
//
//////////////////////////////////////////////////////////////////////
#if !defined(_SharedMcmsCardsStructs_H__)
#define _SharedMcmsCardsStructs_H__


#include "CardsStructs.h"
#include "McuMngrStructs.h"
//#include "SharedMcmsMcuMngrStructs.h"
#include "TBStructs.h"



//////////////////////////////
//   CONSTANTS DEFINITIONS
//////////////////////////////

//////////////////////////////
//       ENUMERATORS
//////////////////////////////


//////////////////////////////
//    API STRUCTURES
//////////////////////////////

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  csManager structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct
{
	IP_INTERFACE_S       interfacesList[MAX_NUM_OF_BOARDS * MAX_NUM_OF_PQS];
	NETWORK_PARAMS_S     networkParams;
	APIU32               isSecondaryNetwork;
	NETWORK_PARAMS_S     secondaryNetworkParams;
	DNS_CONFIGURATION_S  dnsConfig;
	APIU32				 v35GwIpv4Address;

	APIU32               future_use1;
	APIU32               future_use2;
} CS_IP_PARAMS_S;


typedef struct
{
	CS_IP_PARAMS_S  ipParams;
	APIU32          serviceId;
	APIU8           serviceName[NET_SERVICE_PROVIDER_NAME_LEN];
	APIU32			csIp;			//future_use1

	APIU32          future_use2;
} CS_MEDIA_IP_PARAMS_S;

typedef struct
{
	APIU32   iPv4Address;

	APIU32   ipType;	 // eIpType - IpV4/IpV6/both/none

	APIU32   boardId;    // id of card
	APIU32   pqId;       // id of PowerQuick, just index


} IP_INTERFACE_SHORT_S;


typedef struct
{
	APIU32				boardId;
	APIU32				subBoardId;
	MEDIA_IP_CONFIG_S	mediaIpConfig;
} CS_MEDIA_IP_CONFIG_S;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  rsrcAllocator structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct
{
	PHYSICAL_INFO_HEADER_S  physicalHeader;
	APIU32                  unitType;
	APIU32                  pqNumber;   // for ART units - their corresponding PowerQuick CPU
	APIU32                  status;
} RSRCALLOC_UNIT_CONFIG_PARAMS_S;


typedef struct
{
	APIU32                          cardType;
//	APIU32                          boardIdForPqAlloc; // adjustment between 'real' boardId and the boardId received from CsMngr for PQ allocation
	RSRCALLOC_UNIT_CONFIG_PARAMS_S  unitsConfigParamsList[MAX_NUM_OF_UNITS];
} RSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S;


typedef struct
{
	PHYSICAL_INFO_HEADER_S  physicalHeader;
	APIU32                  status;
} RSRCALLOC_ART_FIPS_140_IND_S;


typedef struct
{
	PHYSICAL_INFO_HEADER_S  physicalHeader;
	KEEP_ALIVE_S            keepAliveStruct;
} RSRCALLOC_KEEP_ALIVE_S;
/*
//hot swap
typedef struct
{
	APIU32                  BoardID;
	APIU32                  SubBoardID;   
	APIU32                  UnitId;
} SET_AUDIO_CNTRLR_MASTER_REQ_S;
*/
/*typedef struct
{
	APIU32                  BoardID;
	APIU32                  SubBoardID_1;   // USE eHotSwapRemoveType
	APIU32                  SubBoardID_2;   // USE eHotSwapRemoveType
} CARD_REMOVED_IND_S; //SubBoardID_1 = MFA  SubBoardID_2 = RTM*/
typedef struct
{
	APIU32                  BoardID;
	APIU32                  SubBoardID;
	APIU32                  cardType;
} CARD_REMOVED_IND_S; //SubBoardID_1 = MFA  SubBoardID_2 = RTM
#endif // !defined(_SharedMcmsCardsStructs_H__)

