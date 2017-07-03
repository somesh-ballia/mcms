//////////////////////////////////////////////////////////////////////

#include "TraceStream.h"
#include "PrettyTable.h"

///////////////////////////////////////////////////////////////////////////
template <typename KeyID, size_t N>
void AllocationMap<KeyID, N>::reserve(size_t count)
{
	if (count <= capacity())
		return;

	const size_t delta = count - capacity();

	allocationTable_.resize(count, 0);
	free_[0].count += delta;

	const size_t begin = count - delta;
	const size_t end = count - 1;

	if (free_[0].head == EOLIST)
		free_[0].head = begin;

	if (free_[0].tail != EOLIST)
		allocationTable_[free_[0].tail] = begin;

	free_[0].tail = end; // points to the last cell

	// building the linked list, each cell holds an index of the next free cell
	for (size_t i = begin; i < end; ++i)
		allocationTable_[i] = i + 1;

	allocationTable_[end] = EOLIST;
}

//////////////////////////////////////////////////////////////////////////////
template <typename KeyID, size_t N>
bool AllocationMap<KeyID, N>::isAllocatedByKey(KeyID keyID) const
{
	typename DirectoryMap::const_iterator it = assignedCellsMap_.find(keyID);

	return (it != assignedCellsMap_.end());
}

//////////////////////////////////////////////////////////////////////////////
template <typename KeyID, size_t N>
std::ostream& operator <<(std::ostream& ostr, const AllocationMap<KeyID, N>& obj)
{
	CPrettyTable<KeyID, size_t> allocated("Key ID", "Cell");

	for (typename DirectoryMap::const_iterator it = assignedCellsMap_.begin(); it != assignedCellsMap_.end(); ++it)
	{
		size_t i = it->second;

		while (i != EOLIST)
		{
			allocated.Add(it->first, i);

			i = allocationTable_[i];
		}
	}

	if (!allocated.IsEmpty())
	{
		ostr << "Allocated cells:\n";
		allocated.Sort(1);
		allocated.Dump(ostr);
		ostr << '\n';
	}

	return ostr;
}

//////////////////////////////////////////////////////////////////////////////
template <typename KeyID, size_t N>
void AllocationMap<KeyID, N>::LinkCellToKey(KeyID keyID, size_t i)
{
	typename DirectoryMap::iterator it = assignedCellsMap_.find(keyID);

	if (it != assignedCellsMap_.end())
	{
		allocationTable_[i] = it->second; // push to front
		it->second = i;                    // update the "Head"
	}
	else
	{
		allocationTable_[i] = EOLIST;                       // mark as the last
		assignedCellsMap_.insert(std::make_pair(keyID, i)); // create the "Head"
	}
}

//////////////////////////////////////////////////////////////////////////////
template <typename KeyID, size_t N>
bool AllocationMap<KeyID, N>::allocate(KeyID keyID, size_t& cell)
{
	for (size_t h = 0; h < N; ++h)
	{
		if (free_[h].head == EOLIST) // the current Head does NOT point anywhere, try the next one if available
			continue;

		PASSERT(cell); // it should be ZERO before allocation

		++size_;

		size_t i = free_[h].head;
		free_[h].head = allocationTable_[i]; // update the Head to point to the next unoccupied cell

		if (free_[h].head == EOLIST)
			free_[h].tail = EOLIST;

		LinkCellToKey(keyID, i);

		--free_[h].count;

		cell = i;

		return true;
	}

	std::ostringstream msg;
	msg << "Internal failure!!!\n";
	Dump(msg);

	TRACESTRFUNC(eLevelWarn) << msg.str();
	return false;
}

//////////////////////////////////////////////////////////////////////////////
template <typename KeyID, size_t N>
bool AllocationMap<KeyID, N>::UnlinkCellFromKey(KeyID keyID, size_t& i, bool any/* = false*/)
{
	typename DirectoryMap::iterator it = assignedCellsMap_.find(keyID);

	if (it != assignedCellsMap_.end()) // are any open cells associated with the party?
	{
		size_t next = it->second; // index of the 1st open cell associated with the party
		size_t prev = EOLIST;     // index of the cell before the current

		while (next != EOLIST) // not at the end of the "linked list"
		{
			PASSERT(static_cast<size_t>(next) >= allocationTable_.size());

			size_t current = next; // save the cell index
			next = allocationTable_[current]; // retrieve the new next

			if (any || current == i)
			{
				if (prev)
					allocationTable_[prev] = next;
				else // removing the very 1st cell in the linked list
				{
					if (next) // the key still has opened cells associated with it?
						it->second = next;
					else
						assignedCellsMap_.erase(it);
				}

				i = current;
				return true;
			}

			prev = current;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////
template <typename KeyID, size_t N>
size_t AllocationMap<KeyID, N>::free(KeyID keyID, size_t* cell/* = NULL*/, size_t into/* = 0*/)
{
	PASSERT_AND_RETURN_VALUE(!keyID, false);
	PASSERT_AND_RETURN_VALUE(into >= N, false);

	size_t count = 0;
	size_t i = cell ? *cell : EOLIST;

	while (UnlinkCellFromKey(keyID, i, !cell))
	{
		++count;
		++free_[into].count; // update the unoccupied cells count

		--size_;

		// *** insert a new cell at the end of the list of unoccupied cells
		if (free_[into].tail != EOLIST)
			allocationTable_[free_[into].tail] = i;

		allocationTable_[i] = EOLIST; // mark the end of the list
		free_[into].tail = i;         // tail points to this currently freed cell

		if (free_[into].head == EOLIST) // let head point to the same as tail for one-element-list
			free_[into].head = free_[into].tail;

		if (cell)
			*cell = 0;
	}

	PASSERT(!count);
	return count;
}

//////////////////////////////////////////////////////////////////////////////
