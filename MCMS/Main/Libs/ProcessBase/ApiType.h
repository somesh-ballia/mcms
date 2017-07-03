#ifndef API_TYPE_H__
#define API_TYPE_H__

///////////////////////////////////////////////////////////////////////////
#include "ApiTypeBase.h"

#include "DataTypes.h"
#include "Segment.h"

#include <string>

///////////////////////////////////////////////////////////////////////////
template <typename T>
class ApiType;

// ApiType<...> template class provides a transparent interface for value objects
// that can either be NULL (non-assigned) or have any of a valid values of a type T

///////////////////////////////////////////////////////////////////////////
template <typename X>
CSegment& operator >>(CSegment& seg, ApiType<X>& obj);

template <typename X>
CSegment& operator <<(CSegment& seg, const ApiType<X>& obj);

template <class S, typename X>
S& operator >>(S& s, ApiType<X>& obj);

///////////////////////////////////////////////////////////////////////////
template <typename T>
struct TypeTraits< ApiType<T> >
{
	enum { tag = ttv_api_type };

	typedef ApiType<T>       ValueType;
	typedef const ValueType& ParamType;
};

///////////////////////////////////////////////////////////////////////////
// --- a specialized version for ApiType<T> for any T ---
template <typename T>
struct ApiTypeHelper<ApiType<T>, void, false>
{
	TYPEDEF_TRAITS(ApiType<T>)

	static bool IsTheSame(ParamType obj_a, ParamType obj_b)
	{ return obj_a.IsTheSame(obj_b); }

	static void Reinit(ValueType& obj)
	{ obj.Clear(); }

	static size_t CurrentBinarySize(ParamType obj)
	{ return obj.CurrentBinarySize(); }

	static void InitHelper(void* var, const char* initializer, const xmlNode* node)
	{
		ValueType& obj = *reinterpret_cast<ValueType*>(var);
		obj = initializer;
		ApiTypeHelper::ReadFromXml(obj, node);
	}

	static void ReadFromXml(ValueType& obj, const xmlNode* node)
	{ ApiTypeHelper<T>::ReadFromXml((T&)obj, node); }

	static bool IsAssigned(ParamType obj)
	{ return obj.IsAssigned(); }
};

///////////////////////////////////////////////////////////////////////////
// --- a specialized version for std::string ---
template <>
inline void ApiTypeHelper<std::string>::Reinit(ValueType& obj)
{ obj.clear(); }

template <>
inline size_t ApiTypeHelper<std::string>::CurrentBinarySize(ParamType obj)
{ return sizeof(size_t) + obj.size(); }

template <>
inline void ApiTypeHelper<std::string>::InitHelper(void* var, const char* initializer, const xmlNode* /*node*/)
{ *reinterpret_cast<ValueType*>(var) = (initializer ? initializer : ""); }

///////////////////////////////////////////////////////////////////////////
// --- a specialized version for c-style const char* strings ---
template <>
inline size_t ApiTypeHelper<const char*>::CurrentBinarySize(const char* obj)
{ return sizeof(size_t) + strlen(obj); }

///////////////////////////////////////////////////////////////////////////
template <typename T>
class ApiType : public ApiTypeBase, public ApiTypeProxy<T>
{
	template <typename X>
	friend CSegment& operator >>(CSegment& seg, ApiType<X>& obj);

	template <typename X>
	friend CSegment& operator <<(CSegment& seg, const ApiType<X>& obj);

	template <class S, typename X>
	friend S& operator >>(S& s, ApiType<X>& obj);

public:

	TYPEDEF_TRAITS(T)

	ApiType()
		: ApiTypeBase(false)
	{}

	ApiType(const ApiTypeProxy<T>& value)
		: ApiTypeBase(true)
		, ApiTypeProxy<T>(value)
	{}

	size_t CurrentBinarySize() const
	{
		return
			ApiTypeBase::CurrentBinarySize() +
			(ApiTypeBase::IsAssigned() ? ApiTypeHelper<T>::CurrentBinarySize(this->value()) : 0);
	}

	bool IsTheSame(const ApiType& other) const
	{ return ApiTypeBase::operator ==(other) && operator ==(other); }

	void Clear()
	{
		ApiTypeHelper<T>::Reinit(ref());
		ApiTypeBase::Clear();
	}

	// compares values only, without considering the 'assigned' property
	bool operator ==(const ApiType& other) const
	{ return this->value_ == other.value_; }

	bool operator !=(const ApiType& other) const
	{ return !operator ==(other); }

	operator ParamType() const
	{ return this->value_; }

	template <typename X>
	operator typename enable_if< !is_same<X, ParamType>::value, X >::type() const
	{ return this->value_; }

	template <typename X>
	operator X() const
	{ return this->value_; }

	ApiType& operator =(const ApiTypeProxy<T>& value)
	{
		ApiTypeProxy<T>::operator =(value);
		ApiTypeBase::SetAssigned();
		return *this;
	}

	ApiType& operator =(const ApiType& value)
	{
		ApiTypeProxy<T>::operator =(value);
		ApiTypeBase::operator =(value);
		return *this;
	}

	const ValueType* operator ->() const
	{ return &this->value_; }

	ValueType* operator ->()
	{ return &this->value_; }

	const ParamType value() const
	{ return this->value_; }

protected:

	ValueType& ref()
	{ return this->value_; }
};

///////////////////////////////////////////////////////////////////////////
template <typename T>
CSegment& operator >>(CSegment& seg, ApiType<T>& obj)
{
	seg >> (ApiTypeBase&)(obj);

	if (obj.IsAssigned())
		seg >> obj.value_;

	return seg;
}

///////////////////////////////////////////////////////////////////////////
template <typename T>
CSegment& operator <<(CSegment& seg, const ApiType<T>& obj)
{
	seg << (const ApiTypeBase&)(obj);

	if (obj.IsAssigned())
		seg << obj.value_;

	return seg;
}

///////////////////////////////////////////////////////////////////////////
template <class S, typename T>
S& operator <<(S& s, const ApiType<T>& obj)
{
	s << obj.value();
	return s;
}

///////////////////////////////////////////////////////////////////////////
template <class S, typename T>
S& operator >>(S& s, ApiType<T>& obj)
{
	s >> obj.ref();
	return s;
}

///////////////////////////////////////////////////////////////////////////
template <class T>
struct ValueFormatter< ApiType<T> >
{
	TYPEDEF_TRAITS(ApiType<T>)

	static void get(ParamType obj, std::string& value)
	{ ValueFormatter<T>::get(obj.value(), value); }
};

///////////////////////////////////////////////////////////////////////////
#include "ApiTypeImpl.h"

///////////////////////////////////////////////////////////////////////////
#endif // API_TYPE_H__
