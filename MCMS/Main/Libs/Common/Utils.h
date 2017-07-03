/*
 * Utils.h
 *
 *  Created on: May 29, 2012
 *      Author: vasily
 */

#ifndef __UTILS_H__
#define __UTILS_H__


#include <string>

#include "IpAddressDefinitions.h"


class Utils
{
public:
	static std::string String2lower(std::string s);
	static std::string GenerateUid();

	static std::string IpAddressToString(mcTransportAddress& transAddr, bool bWithPrefix = false);

private:
	static void SeedRand();
	static int GetRandomFromRange(const int minVal, const int maxVal);
};



#endif /* __UTILS_H__ */




