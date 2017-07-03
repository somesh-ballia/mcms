/*
 * StlUtils.h
 *
 *  Created on: May 10, 2012
 *      Author: bguelfand
 */

#ifndef STLUTILS_H_
#define STLUTILS_H_

#include <string>
#include <list>
#include <iostream>
#include <sstream>
#include "Segment.h"

class CStlUtils
{
private:
	CStlUtils();

public:
	template<class T> static std::string ContainerToString(const T& rContainer)
	{
		std::stringstream ss(std::stringstream::in | std::stringstream::out);
	    typename T::const_iterator it(rContainer.begin());
	    typename T::const_iterator end(rContainer.end());
		for (bool b=false; it != end; ++it, b=true)
		{
			if (b)
				ss << "; ";
			ss << *it;
		}
		return ss.str();
	}

	template<class T> static std::string ValueToString(const T& rValue)
	{
		std::stringstream ss(std::stringstream::in | std::stringstream::out);
		ss << rValue;
		return ss.str();
	}

	static void SerializeList(const std::list<unsigned int>& lst, CSegment* pSeg);
	static void DeSerializeList(CSegment* pSeg, std::list<unsigned int>& lst);

	static void SerializeListWithFlag(const std::list<unsigned int>& lst, unsigned int flag, CSegment* pSeg);
	static void DeSerializeListWithFlag(CSegment* pSeg, std::list<unsigned int>& lst, unsigned int& flag);

};

#endif /* STLUTILS_H_ */
