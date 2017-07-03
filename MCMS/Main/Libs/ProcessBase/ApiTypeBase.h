#ifndef API_BASE_TYPE_H__
#define API_BASE_TYPE_H__

/////////////////////////////////////////////////////////////////////////////
#include "ApiTypeHelper.h"

/////////////////////////////////////////////////////////////////////////////
CSegment& operator >>(CSegment& seg, ApiTypeBase& obj);
CSegment& operator <<(CSegment& seg, const ApiTypeBase& obj);

/////////////////////////////////////////////////////////////////////////////
class ApiTypeBase
{
	friend CSegment& operator >>(CSegment& seg, ApiTypeBase& obj);
	friend CSegment& operator <<(CSegment& seg, const ApiTypeBase& obj);

	friend class ApiBaseObjectPtr;

public:

	size_t CurrentBinarySize() const
	{ return sizeof(bool); }

	bool IsAssigned() const
	{ return assigned_; }

protected:

	explicit ApiTypeBase(bool assigned = false)
		: assigned_(assigned)
	{}

	void Clear()
	{ assigned_ = false; }

	ApiTypeBase& operator =(const ApiTypeBase& other)
	{ assigned_ = other.assigned_; return *this; }

	void SetAssigned()
	{ assigned_ = true; }

	bool operator ==(const ApiTypeBase& other) const
	{ return assigned_ == other.assigned_; }

protected:

	bool assigned_;
};

/////////////////////////////////////////////////////////////////////////////
inline CSegment& operator >>(CSegment& seg, ApiTypeBase& obj)
{ return seg >> obj.assigned_; }

inline CSegment& operator <<(CSegment& seg, const ApiTypeBase& obj)
{ return seg << obj.assigned_; }

/////////////////////////////////////////////////////////////////////////////
#endif // API_BASE_TYPE_H__
