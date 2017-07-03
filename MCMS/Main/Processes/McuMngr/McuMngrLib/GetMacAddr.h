#ifndef __GET_MAC_ADDR_H__
#define __GET_MAC_ADDR_H__

#include <stddef.h>
#include <string>
using std::string;

string GetMacAddrWithoutSemiColon(char const * devName);
char const * GetMacAddrWithSemiColon(char const * devName, char * buf, size_t bufLen);

#endif

