/*
 * LookupId.h
 *
 *  Created on: Dec 17, 2013
 *      Author: dkrasnopolsky
 */

#ifndef LOOKUPID_H_
#define LOOKUPID_H_

#include <bitset>
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//                        CLookupId
////////////////////////////////////////////////////////////////////////////
template<size_t _size>
class CLookupId : public std::bitset<_size>
{
public:
	       CLookupId() : m_pos(0), m_size(_size) {}

	DWORD  Alloc();
	void   Clear(DWORD id);
	void   Limit(size_t limit) { FPASSERT_AND_RETURN(limit > m_size); m_size = limit; }

private:
	size_t m_pos;
	size_t m_size;
};

template<size_t _size>
DWORD CLookupId<_size>::Alloc()
{
	for (size_t i = m_pos; i < m_size; ++i)
	{
		if (!std::bitset<_size>::test(i))
		{
			std::bitset<_size>::set(i);
			m_pos = i+1;
			return m_pos;
		}
	}
	for (size_t i = 0; i < m_pos; ++i)
	{
		if (!std::bitset<_size>::test(i))
		{
			std::bitset<_size>::set(i);
			m_pos = i+1;
			return m_pos;
		}
	}
	FPASSERT(1);
	return (0);
}

template<size_t _size>
void CLookupId<_size>::Clear(DWORD id)
{
	size_t pos = id-1;

	FPASSERT_AND_RETURN(pos > m_size);

	FPASSERT_AND_RETURN(!std::bitset<_size>::test(pos));

	std::bitset<_size>::set(pos, false);
}

#endif /* LOOKUPID_H_ */
