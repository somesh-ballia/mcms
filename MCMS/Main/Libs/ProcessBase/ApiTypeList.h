#ifndef API_TYPE_LIST_H__
#define API_TYPE_LIST_H__

#include "ApiTypeHelper.h"
#include "ApiBaseObject.h"

#include <list>
#include <set>

///////////////////////////////////////////////////////////////////////////
/*
template <typename T>
struct TypeTraits<std::list<T>, void>
{
	enum { tag = ttv_stl_sequence };

	typedef T ValueType;
	typedef const T& ParamType;
};
*/
///////////////////////////////////////////////////////////////////////////
// --- a specialized version for an STL container ---
// (i.e. std::list, std::vector, std::set, std::multiset) of any T

template <class C>
struct ApiTypeHelper<C, void, true>
{
	typedef C                ValueType;
	typedef const ValueType& ParamType;

	typedef typename C::value_type T;

	static bool IsTheSame(ParamType obj_a, ParamType obj_b)
	{ return obj_a == obj_b; }

	static void Reinit(ValueType& obj)
	{ obj.clear(); }

	static size_t CurrentBinarySize(ParamType obj);

	static void InitHelper(void* var, const char* initializer, const xmlNode* node)
	{
		ValueType& obj = *reinterpret_cast<ValueType*>(var);
		ApiTypeHelper::ReadFromXml(obj, node, initializer);
	}

	static void ReadFromXml(ValueType& obj, const xmlNode* node, const char* initializer = NULL)
	{
		T newElem;

		if (initializer)
			ApiTypeHelper<T>::InitHelper(&newElem, initializer, node);
		else
			ApiTypeHelper<T>::ReadFromXml(newElem, node);

		const bool isAssigned = (node && initializer) ? ApiTypeHelper<T>::IsAssigned(newElem) : true;

		if (isAssigned)
			obj.push_back(newElem);
	}

	static bool IsAssigned(ParamType obj)
	{ return !obj.empty(); }

	static CSegment& WriteTo(CSegment& seg, ParamType obj);
	static CSegment& ReadFrom(CSegment& seg, ValueType& obj);
};

///////////////////////////////////////////////////////////////////////////
template <class C>
size_t ApiTypeHelper<C, void, true>::CurrentBinarySize(ParamType obj)
{
	size_t size = sizeof(size_t);

	for (typename C::const_iterator it = obj.begin(); it != obj.end(); ++it)
		size += ApiTypeHelper<T>::CurrentBinarySize(*it);

	return size;
}

template <class C>
CSegment& ApiTypeHelper<C, void, true>::WriteTo(CSegment& seg, ParamType obj)
{
	seg << obj.size();

	for (typename C::const_iterator it = obj.begin(); it != obj.end(); ++it)
		seg << *it;

	return seg;
}

template <class C>
CSegment& ApiTypeHelper<C, void, true>::ReadFrom(CSegment& seg, ValueType& obj)
{
	ApiTypeHelper::Reinit(obj);

	size_t size = 0;
	seg >> size;

	if (size)
	{
		T value;

		for (size_t i = 0; i < size; ++i)
		{
			seg >> value;
			obj.push_back(value);
		}
	}

	return seg;
}

///////////////////////////////////////////////////////////////////////////
template <typename T>
inline CSegment& operator <<(CSegment& seg, const std::list<T>& obj)
{ return ApiTypeHelper<std::list<T>, void, true>::WriteTo(seg, obj); }

///////////////////////////////////////////////////////////////////////////
template <typename T>
inline CSegment& operator >>(CSegment& seg, std::list<T>& obj)
{ return ApiTypeHelper<std::list<T>, void, true>::ReadFrom(seg, obj); }

///////////////////////////////////////////////////////////////////////////
template <typename T>
inline CSegment& operator <<(CSegment& seg, const std::vector<T>& obj)
{ return ApiTypeHelper<std::vector<T>, void, true>::WriteTo(seg, obj); }

///////////////////////////////////////////////////////////////////////////
template <typename T>
inline CSegment& operator >>(CSegment& seg, std::vector<T>& obj)
{ return ApiTypeHelper<std::vector<T>, void, true>::ReadFrom(seg, obj); }

///////////////////////////////////////////////////////////////////////////
template <typename T>
inline CSegment& operator <<(CSegment& seg, const std::set<T>& obj)
{ return ApiTypeHelper<std::set<T>, void, true>::WriteTo(seg, obj); }

///////////////////////////////////////////////////////////////////////////
template <typename T>
inline CSegment& operator >>(CSegment& seg, std::set<T>& obj)
{ return ApiTypeHelper<std::set<T>, void, true>::ReadFrom(seg, obj); }

///////////////////////////////////////////////////////////////////////////
template <
	class T,
	bool Q = is_stl_sequence<T>::value || is_base_of<ApiBaseObject, T>::value || is_same<ApiBaseObjectPtr, T>::value
>
struct EnumeratorHelper; // enumeration of either simple (Q = false) or compound (Q = true) container's data members

///////////////////////////////////////////////////////////////////////////
// --- a specialized version for an STL container ---
// (i.e. std::list, std::vector, std::set, std::multiset) of any T

template <class C>
struct ContainerEnumerator<C, void, true>
{
	typedef C                ValueType;
	typedef const ValueType& ParamType;

	typedef typename C::value_type T;

	// for containers of APiBaseObjectPtr-s
	static void write(ParamType obj, IApiObjectEncoder& encoder)
	{
		for (typename C::const_iterator it = obj.begin(); it != obj.end(); ++it)
			WriteToHelper<T, ds_member>::WriteTo(*it, encoder);
	}

	// for containers of other types;
	// for ApiBaseObject's derived classes/structs
	static void write(ParamType obj, IApiObjectEncoder& encoder, const utfChar* tag, const utfChar* prefix = NULL)
	{
		for (typename C::const_iterator it = obj.begin(); it != obj.end(); ++it)
			EnumeratorHelper<T>::encode(*it, encoder, tag, prefix);
	}
};

///////////////////////////////////////////////////////////////////////////
// for simple types
template <class T>
struct EnumeratorHelper<T, false>
{
	TYPEDEF_TRAITS(T)

	static void encode(ParamType obj, IApiObjectEncoder& encoder, const utfChar* tag, const utfChar* prefix)
	{
		std::string value;

		ValueFormatter<T>::get(obj, value);
		encoder.addElement(tag, (const utfChar*)value.c_str(), prefix);
	}
};

// for compound types
template <class T>
struct EnumeratorHelper<T, true>
{
	TYPEDEF_TRAITS(T)

	// for APiBaseObjectPtr-s
	static void encode(ParamType obj, IApiObjectEncoder& encoder)
	{ ContainerEnumerator<T>::write(obj, encoder); }

	// for all the rest
	static void encode(ParamType obj, IApiObjectEncoder& encoder, const utfChar* tag, const utfChar* prefix = NULL)
	{ ContainerEnumerator<T>::write(obj, encoder, tag, prefix); }
};

///////////////////////////////////////////////////////////////////////////
#endif // API_TYPE_LIST_H__
