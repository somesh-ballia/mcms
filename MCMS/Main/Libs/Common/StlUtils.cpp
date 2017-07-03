/*
 * StlUtils.cpp
 *
 *  Created on: May 10, 2012
 *      Author: bguelfand
 */

#include "StlUtils.h"

CStlUtils::CStlUtils()
{
}

void CStlUtils::SerializeList(const std::list<unsigned int>& lst, CSegment* pSeg)
{
	std::list<unsigned int>::const_iterator it = lst.begin();
	for ( ; it != lst.end(); ++it )
		pSeg->Put(*it);
}

void CStlUtils::DeSerializeList(CSegment* pSeg, std::list<unsigned int>& lst)
{
	lst.clear();
	DWORD dwSize = pSeg->GetWrtOffset();
	int n = dwSize / sizeof(unsigned int);
	for (int i=0; i<n; ++i)
	{
		unsigned int item = 0;
		pSeg->Get(item);
		lst.push_back(item);
	}
}

void CStlUtils::SerializeListWithFlag(const std::list<unsigned int>& lst, unsigned int flag, CSegment* pSeg)
{
	pSeg->Put(flag);
	SerializeList(lst, pSeg);
}

void CStlUtils::DeSerializeListWithFlag(CSegment* pSeg, std::list<unsigned int>& lst, unsigned int& flag)
{
	lst.clear();
	pSeg->Get(flag);
	DWORD dwSize = pSeg->GetWrtOffset() - pSeg->GetRdOffset();
	int n = dwSize / sizeof(unsigned int);
	for (int i=0; i<n; ++i)
	{
		unsigned int item = 0;
		pSeg->Get(item);
		lst.push_back(item);
	}
}
