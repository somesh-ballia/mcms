//////////////////////////////////////////////////////////////////////

#include "TraceStream.h"
#include "PrettyTable.h"

///////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
void HandleManager<R, INDEX_BITS>::reserve(size_t count)
{
	if (count <= capacity())
		return;

	if (count > MAX_COUNT)
		return;

	const size_t delta = count - capacity();

	allocationTable_.resize(count);
	free_.count += delta;

	const size_t begin = count - delta;
	const size_t end = count - 1;

	if (free_.head == EOLIST)
		free_.head = begin;

	if (free_.tail != EOLIST)
		allocationTable_[free_.tail].index = begin;

	free_.tail = end; // points to the last cell

	for (size_t i = begin; i < end; ++i)
		allocationTable_[i].index = i + 1; // building the linked list, each cell holds an index of the next free cell

	allocationTable_[end].index = EOLIST;
}

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
std::ostream& operator <<(std::ostream& ostr, const HandleManager<R, INDEX_BITS>& obj)
{
	CPrettyTable<size_t, size_t, size_t, R> allocated("ID", "Magic", "Ref.", "Resource");

	const size_t capacity = obj.capacity();
	const size_t size = obj.size();

	size_t count = 0;

	for (size_t i = 0; i < capacity && count < size; ++i)
	{
		if (obj.allocationTable_[i].resource)
		{
			++count;
			allocated.Add(i, obj.allocationTable_[i].magic, obj.allocationTable_[i].index, obj.allocationTable_[i].resource);
		}
	}

	if (!allocated.IsEmpty())
	{
		ostr << "Allocated handles:" << count << ", capacity:" << capacity << "\n";
		allocated.Dump(ostr);
		ostr << '\n';
	}

	return ostr;
}

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
void HandleManager<R, INDEX_BITS>::grow()
{
	size_t new_capacity = (capacity() + 1) * 2;

	if (new_capacity > MAX_COUNT)
		new_capacity = MAX_COUNT;

	reserve(new_capacity);
}

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
bool HandleManager<R, INDEX_BITS>::push_back(List& x, size_t i)
{
	if (i == EOLIST)
		return false;

	++x.count;

	if (x.tail != EOLIST)
		allocationTable_[x.tail].index = i;

	x.tail = i; // tail points to this cell
	allocationTable_[i].index = EOLIST; // mark the end of the list

	// let head point to the same as tail for one-element-list
	if (x.head == EOLIST)
		x.head = x.tail;

	return true;
}

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
size_t HandleManager<R, INDEX_BITS>::pop_front(List& x)
{
	const size_t i = x.head;

	if (i != EOLIST)
	{
		x.head = allocationTable_[i].index;

		if (x.head == EOLIST)
			x.tail = EOLIST;

		--x.count;
	}

	return i;
}

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
typename HandleManager<R, INDEX_BITS>::HANDLE HandleManager<R, INDEX_BITS>::create(R resource)
{
	if (!resource)
		return HANDLE();

	if (free_.head == EOLIST)
		grow();

	const size_t i = pop_front(free_);

	if (i == EOLIST)
		return HANDLE();

	allocationTable_[i].resource = resource;
	allocationTable_[i].index = 1; // initialize the reference counter

	return handleOf(i);
}

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
bool HandleManager<R, INDEX_BITS>::addRef(HANDLE handle)
{
	const size_t i = indexOf(handle);

	if (i == EOLIST)
		return false;

	FPASSERT_AND_RETURN_VALUE(allocationTable_[i].index == EOLIST, false);

	++allocationTable_[i].index; // increment refernce counter
	return true;
}

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
R HandleManager<R, INDEX_BITS>::close(HANDLE& handle)
{
	const size_t i = indexOf(handle);

	if (i == EOLIST)
		return 0;

	--allocationTable_[i].index; // decrement refernce counter
	handle.reset();

	R r(0);

	if (!allocationTable_[i].index)
	{
		r = allocationTable_[i].resource;
		allocationTable_[i].resource = 0;

		++allocationTable_[i].magic;

		push_back(free_, i);
	}

	return r;
}

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
size_t HandleManager<R, INDEX_BITS>::indexOf(HANDLE handle) const
{
	const size_t i = handle.index_ - 1;

	return (i < capacity() && handle.magic_ == allocationTable_[i].magic) ? i : size_t(EOLIST);
}

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
R HandleManager<R, INDEX_BITS>::resourceOf(HANDLE handle) const
{
	const size_t i = handle.index_ - 1;

	return i < capacity() ? R(allocationTable_[i]) : 0;
}

//////////////////////////////////////////////////////////////////////////////
template <class S, typename R, size_t INDEX_BITS>
inline S& operator <<(S& s, const Handle<R, INDEX_BITS>& obj)
{
	s << reinterpret_cast<void*>(obj.h_);
	return s;
}

//////////////////////////////////////////////////////////////////////////////
template <class S, typename R, size_t INDEX_BITS>
S& operator >>(S& s, Handle<R, INDEX_BITS>& obj)
{
	FPASSERT(obj.h_);
	s >> obj.h_;
	return s;
}

//////////////////////////////////////////////////////////////////////////////
