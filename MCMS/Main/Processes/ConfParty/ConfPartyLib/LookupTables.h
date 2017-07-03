/*
 * LookupId.h
 *
 *  Created on: Dec 17, 2013
 *      Author: dkrasnopolsky
 */

#ifndef LOOKUPTABLES_H_
#define LOOKUPTABLES_H_

#include <map>
#include <utility>
#include "LookupId.h"
#include "RsrcDesc.h"

class CConf;
class CParty;
class CImage;


////////////////////////////////////////////////////////////////////////////
//                        CLookupTable
////////////////////////////////////////////////////////////////////////////
template <typename TKey, typename TVal>
class CLookupTable
{
public:
	void Add(TKey key, TVal val);
	void Del(TKey key);
	TVal Get(TKey key);

private:
	void Dump(const char* functionName);

private:
	std::map<TKey, TVal> m_Table;

	typedef typename std::map<TKey, TVal>::iterator       CLookupTableItr;
	typedef typename std::map<TKey, TVal>::const_iterator CLookupTableConstItr;
};

template <typename TKey, typename TVal>
void CLookupTable<TKey, TVal>::Add(TKey key, TVal val)
{
#ifdef LOOKUP_TABLE_DEBUG_TRACE
	FTRACESTR(eLevelInfoHigh) << "CLookupTable::Add - Key:" << key << ", Val:" << std::hex << val;
#endif

	std::pair<CLookupTableItr, bool> rc = m_Table.insert(std::make_pair(key, val));

	FPASSERTSTREAM_AND_RETURN(!rc.second, "Failed, Element already exists, key:" << key);

	Dump(__PRETTY_FUNCTION__);
}

template <typename TKey, typename TVal>
void CLookupTable<TKey, TVal>::Del(TKey key)
{
	CLookupTableItr _ii = m_Table.find(key);
	FPASSERTSTREAM_AND_RETURN(_ii == m_Table.end(), "Failed, Doesn't have an element, key:" << key);

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	FTRACESTR(eLevelInfoHigh) << "CLookupTable::Del - Key:" << key << ", Val:" << std::hex << _ii->second;
#endif

	m_Table.erase(_ii);

	Dump(__PRETTY_FUNCTION__);
}

template <typename TKey, typename TVal>
TVal CLookupTable<TKey, TVal>::Get(TKey key)
{
	CLookupTableConstItr _ii = m_Table.find(key);
	FPASSERTSTREAM_AND_RETURN_VALUE(_ii == m_Table.end(), "Failed, Doesn't have an element, key:" << key, NULL);

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	FTRACESTR(eLevelInfoHigh) << "CLookupTable::Get - Key:" << key << ", Val:" << std::hex << _ii->second;
#endif

	return _ii->second;
}

template <typename TKey, typename TVal>
void CLookupTable<TKey, TVal>::Dump(const char* functionName)
{
#ifdef LOOKUP_TABLE_DEBUG_TRACE
	std::ostringstream msg;
	msg << "called from " << functionName;
	for (CLookupTableConstItr _ii = m_Table.begin(); _ii != m_Table.end(); ++_ii)
	{
		msg << "\n  " << std::dec << _ii->first << ", " << std::hex << _ii->second;
	}
	FTRACESTR(eLevelInfoHigh) << "CLookupTable::Dump - " << msg.str().c_str();
#endif
}

#ifndef MAX_RSRC_PARTY_ID
	#define MAX_RSRC_PARTY_ID 4096
#endif

typedef CLookupId<MAX_RSRC_PARTY_ID> CLookupIdParty;

typedef CLookupTable<ConfRsrcID , CConf* > CLookupTableConf;
typedef CLookupTable<PartyRsrcID, CParty*> CLookupTableParty;
typedef CLookupTable<PartyRsrcID, CImage*> CLookupTableImage;

#endif /* LOOKUPTABLES_H_ */
