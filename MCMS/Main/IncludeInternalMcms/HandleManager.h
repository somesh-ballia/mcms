#ifndef HANDLE_MANAGER_H__
#define HANDLE_MANAGER_H__

//////////////////////////////////////////////////////////////////////////////
#include "Singleton.h"

#include <set>
#include <vector>
#include <map>

#include <limits.h>

//////////////////////////////////////////////////////////////////////////////
// *** Template parameters ***
// R - Resource
// INDEX_BITS - how many bits are used to generate a unique index; the rest of the bits available in the underlied type (size_t) are used to validate the handle
template <typename R, size_t INDEX_BITS>
class Handle;

template <typename R, size_t INDEX_BITS>
class HandleManager;

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
std::ostream& operator <<(std::ostream& ostr, const HandleManager<R, INDEX_BITS>& obj);

template <class S, typename R, size_t INDEX_BITS>
S& operator <<(S& s, const Handle<R, INDEX_BITS>& obj);

template <class S, typename R, size_t INDEX_BITS>
S& operator >>(S& s, Handle<R, INDEX_BITS>& obj);

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS>
class Handle
{
	template <typename R_, size_t INDEX_BITS_>
	friend class HandleManager;

	template <class S, typename R_, size_t INDEX_BITS_>
	friend S& operator <<(S& s, const Handle<R_, INDEX_BITS_>& obj);

	template <class S, typename R_, size_t INDEX_BITS_>
	friend S& operator >>(S& s, Handle<R_, INDEX_BITS_>& obj);

public:

	typedef HandleManager<R, INDEX_BITS> Manager;
	typedef R Resource;

public:

	static Manager& manager()
	{ return Manager::instance(); }

public:

	Handle()
		: h_(0)
	{}

	explicit Handle(R r)
		: h_(Manager::instance().create(r).h_)
	{}

private:

	void operator =(const Handle&); // only assignment is prohibited, not the copy-construction

public:

	R operator *() const
	{ return Manager::instance()[*this]; }

	R operator ->() const
	{ return Manager::instance()[*this]; }

	R close()
	{ return Manager::instance().close(*this); }

	bool addRef()
	{ return Manager::instance().addRef(*this); }

	bool operator !() const
	{ return !h_; }

	operator void*() const
	{ return reinterpret_cast<void*>(h_); }

private:

	Handle(size_t i, size_t m)
		: index_(i)
		, magic_(m)
	{}

	void reset()
	{ h_ = 0; }

private:

	enum { MAGIC_BITS = sizeof(size_t) * CHAR_BIT - INDEX_BITS };

	union
	{
		size_t h_;

		struct
		{
			size_t index_ : INDEX_BITS;
			size_t magic_ : MAGIC_BITS;
		};
	};
};

//////////////////////////////////////////////////////////////////////////////
template <typename R, size_t INDEX_BITS = sizeof(size_t) * CHAR_BIT / 2>
class HandleManager : public SingletonHolder< HandleManager<R, INDEX_BITS> >
{
	template <typename R_, size_t INDEX_BITS_>
	friend std::ostream& operator <<(std::ostream& ostr, const HandleManager<R_, INDEX_BITS_>& obj);

public:

	typedef Handle<R, INDEX_BITS> HANDLE;
	enum { MAX_COUNT = (1UL << INDEX_BITS) - 1 };

	// the index is 0-based
	enum { EOLIST = MAX_COUNT };

public:

	void reserve(size_t count);

	size_t capacity() const
	{ return allocationTable_.size(); }

	size_t size() const
	{ return capacity() - free_.count; }

	HANDLE create(R resource);

	bool addRef(HANDLE handle);

	// handle is zeroed on success (when handle is valid)
	// upon reaching the last reference to the resource, that resource is returned for deletion
	R close(HANDLE& handle);

	R operator [](HANDLE handle) const
	{ return resourceOf(handle); }

private:

	void grow();

	HANDLE handleOf(size_t i) const
	{ return HANDLE(i + 1, allocationTable_[i].magic); }

	size_t indexOf(HANDLE handle) const;

	R resourceOf(HANDLE handle) const;

private:

	enum { MAGIC_BITS = HANDLE::MAGIC_BITS };

	struct Slot
	{
		size_t magic : MAGIC_BITS; // stores the "magic" value of the currently allocated handle for handle validation
		size_t index : INDEX_BITS; // a) index of the next free slot; b) reference counter for used slot

		R resource;

		Slot()
			: magic(0)
			, index(EOLIST)
			, resource(0)
		{}

		operator R() const
		{ return resource; }
	};

	// linked list in array: every cell "points" to another one through the 0-based index, or is EOL (-1) == LAST in the list
	typedef std::vector<Slot> CellsAllocationMap;
	CellsAllocationMap allocationTable_;

	struct List
	{
		size_t head;  // "points" to the start of the list
		size_t tail;  // "points" to the last cell of the list
		size_t count; // counts available cells in the list

		List() : head(EOLIST), tail(EOLIST), count(0) {}
	};

private:

	bool   push_back(List& x, size_t i);
	size_t pop_front(List& x);

private:

	List free_;
};

//////////////////////////////////////////////////////////////////////////////
#include "HandleManagerImpl.h"

//////////////////////////////////////////////////////////////////////////////
#endif // !HANDLE_MANAGER_H__
