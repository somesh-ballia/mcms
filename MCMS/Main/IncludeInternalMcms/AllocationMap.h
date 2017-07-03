#ifndef ALLOCATION_MAP_H__
#define ALLOCATION_MAP_H__

#include <set>
#include <vector>
#include <map>

template <typename KeyID, size_t N>
class AllocationMap;

template <typename KeyID, size_t N>
std::ostream& operator <<(std::ostream& ostr, const AllocationMap<KeyID, N>& obj);

// This template class is to be used for resource allocation management, like TCP / UDP ports allocation;
// KeyID represents the "owner" entity, the resources are allocated to.
template <typename KeyID, size_t N = 1>
class AllocationMap
{
	template <typename KeyID, size_t N>
	friend std::ostream& operator <<(std::ostream& ostr, const AllocationMap<KeyID, N>& obj);

public:

	// the index is 0-based; EOL (-1) means `NULL`
	enum { EOLIST = static_cast<size_t>(-1) };

public:

	AllocationMap()
		: size_(0)
	{}

	void reserve(size_t count);

	size_t capacity() const
	{ return allocationTable_.size(); }

	size_t size() const
	{ return size_; }

	size_t countOf(size_t i = 0) const
	{ return i < N ? free_[i].count : 0; }

	bool   allocate(KeyID keyID, size_t& cell);
	size_t free(KeyID keyID, size_t* cell = NULL, size_t into = 0);

	bool isAllocatedByKey(KeyID keyID) const;

private:

	void LinkCellToKey(KeyID keyID, size_t i);
	bool UnlinkCellFromKey(KeyID keyID, size_t& i, bool any = false);

private:

	typedef std::map<KeyID, size_t> DirectoryMap; // maps Key ID into one of its allocated cells

	DirectoryMap assignedCellsMap_; // index to the 1st cell assigned to the Key

	// linked list in array: every cell "points" to another one through the 0-based index, or is EOL (-1) == LAST in the list
	typedef std::vector<size_t> CellsAllocationMap;
	CellsAllocationMap allocationTable_;

	struct List
	{
		size_t head;  // "points" to the start of the list
		size_t tail;  // "points" to the last cell of the list
		size_t count; // counts available cells in the list

		List() : head(EOLIST), tail(EOLIST), count(0) {}
	};

	List free_[N];
	size_t size_; // actually allocated
};

#include "AllocationMapImpl.h"

//////////////////////////////////////////////////////////////////////////////
#endif // !ALLOCATION_MAP_H__
