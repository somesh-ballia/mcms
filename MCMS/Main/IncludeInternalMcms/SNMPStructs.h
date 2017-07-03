// SNMPStructs.h

#ifndef SNMP_STRUCTS_H_
#define SNMP_STRUCTS_H_

struct SNMP_MMGMNT_INFO_S
{
  DWORD mngmntIp;
  DWORD shelfIp;
  
  APIU8   mngmtIpv6Address[IPV6_ADDRESS_BYTES_LEN];
  APIU8   shelfIpv6Address[IPV6_ADDRESS_BYTES_LEN];
  
  BOOL		isMngmtIpv6Address;
  BOOL		isShelfIpv6Address;

public:
  SNMP_MMGMNT_INFO_S()
  {
    mngmntIp = 0;
    shelfIp = 0;
    memset(mngmtIpv6Address, 0, (size_t)IPV6_ADDRESS_BYTES_LEN);
    memset(shelfIpv6Address, 0, (size_t)IPV6_ADDRESS_BYTES_LEN);
    isMngmtIpv6Address = FALSE;
    isShelfIpv6Address = FALSE;
  }
};

struct SNMP_CS_INFO_S
{
  SNMP_CS_INFO_S()
  {
    csIp = 0;
    gkIp = 0;
    type = 0;
  }

  DWORD csIp;
  DWORD gkIp;
  DWORD type;
};

#define MAX_MFA_NUM 10

struct LINK_STATUS_S
{
	WORD 	boardId;
     WORD   portId ;
     BOOL	isUp;

};
struct LINK_DETAILS_S
{
	DWORD 	ipAddress; // currently not support ipv6. 
	WORD	boardId;
	WORD   portId ; // currently not in use. only board considered 
};
struct SNMP_CARDS_INFO_S
{
	LINK_DETAILS_S mfaLinks[MAX_MFA_NUM];

public:
  SNMP_CARDS_INFO_S()
  {
    memset(mfaLinks, 0, sizeof(mfaLinks));
  }
};


#endif  // SNMP_STRUCTS_H_

