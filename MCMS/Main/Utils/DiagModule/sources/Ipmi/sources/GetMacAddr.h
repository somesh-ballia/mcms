#ifndef __GET_MAC_ADDR_H__
#define __GET_MAC_ADDR_H__

//string GetMacAddrWithoutSemiColon(char const * devName);
char const * GetMacAddrWithSemiColon(char const * devName, char * buf, size_t bufLen);

#endif

