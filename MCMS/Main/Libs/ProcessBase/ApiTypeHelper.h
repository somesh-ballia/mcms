#ifndef API_TYPE_HELPER_H__
#define API_TYPE_HELPER_H__

///////////////////////////////////////////////////////////////////////////
#include "ApiEncoder.h"

#include "DataTypes.h"
#include "Segment.h"

#include <string>
#include <sstream>

///////////////////////////////////////////////////////////////////////////
class ApiTypeBase;
class ApiBaseObject;
class ApiBaseObjectPtr;

///////////////////////////////////////////////////////////////////////////
enum DataSourceEnum
{
	ds_text,   // simple data types, written as non-attributes
	ds_attr,   // simple data types, written as attributes
	ds_member, // compound data types: mt_Class
	ds_choice, // compound data types: mt_Any, mt_Choice - used for *deserializing* only
};

///////////////////////////////////////////////////////////////////////////
template <typename T, typename Q = void>
struct TypeTraits; // generic type traits: value type, parameter type

template <typename T>
struct ApiTypeProxy; // construction using different data types

///////////////////////////////////////////////////////////////////////////
template <
	class T,
	DataSourceEnum D
>
struct WriteToHelper;

///////////////////////////////////////////////////////////////////////////
template <class T>
struct ValueFormatter; // specialize for every simple type in use

///////////////////////////////////////////////////////////////////////////
enum TypeTraitsValueTagEnum
{
	ttv_generic_type = 't',

	ttv_generic_pointer = 'p',
	ttv_generic_const_ref = 'r',

	ttv_const_char_pointer = 'c',
	ttv_std_string = 's',

	ttv_api_type = 'a',

	ttv_handle = 'h',

	ttv_api_base_object_ptr = 'P',
	ttv_api_base_object_derived = 'A',

	ttv_stl_sequence = 'S',

#if 0
	ttv_container_list = 'L',
	ttv_container_vector = 'V',
	ttv_container_map = 'M',
	ttv_container_multimap = 'W',
#endif
};

///////////////////////////////////////////////////////////////////////////
inline std::ostream& operator <<(std::ostream& ostr, TypeTraitsValueTagEnum v)
{ return ostr << (char)(v); }

///////////////////////////////////////////////////////////////////////////
template <typename T>
struct TypeTraits<T, void>
{
	enum { tag = ttv_generic_type };

	typedef T ValueType;
	typedef T ParamType;
};

///////////////////////////////////////////////////////////////////////////
#define TYPEDEF_TRAITS(T) \
	enum { tag = TypeTraits< T >::tag };\
	typedef typename TypeTraits< T >::ValueType  ValueType;\
	typedef typename TypeTraits< T >::ParamType  ParamType;

///////////////////////////////////////////////////////////////////////////
#define TYPEDEF_TRAITS_EX(T, Q) \
	enum { tag = TypeTraits< T, Q >::tag };\
	typedef typename TypeTraits< T, Q >::ValueType  ValueType;\
	typedef typename TypeTraits< T, Q >::ParamType  ParamType;

///////////////////////////////////////////////////////////////////////////
template <typename T>
struct TypeTraits<const T*>
{
	enum { tag = ttv_generic_pointer };
};

template <typename T>
struct TypeTraits<T*>
{
	enum { tag = ttv_generic_pointer };
};

///////////////////////////////////////////////////////////////////////////
template <>
struct TypeTraits<std::string>
{
	enum { tag = ttv_std_string };

	typedef std::string        ValueType;
	typedef const std::string& ParamType;
};

///////////////////////////////////////////////////////////////////////////
template <>
struct TypeTraits<const char*>
{
	enum { tag = ttv_const_char_pointer };

	typedef const char* ValueType;
	typedef const char* ParamType;
};

///////////////////////////////////////////////////////////////////////////
template <typename T>
struct TypeTraits<const T&>
{
	enum { tag = ttv_generic_const_ref };

	typedef T        ValueType;
	typedef const T& ParamType;
};

///////////////////////////////////////////////////////////////////////////
template <bool B, typename T = void>
struct enable_if
{
	typedef void type;
};

template <typename T>
struct enable_if<true, T>
{
	typedef T type;
};

///////////////////////////////////////////////////////////////////////////
template <typename, typename>
struct is_same
{
	enum { value = false }; 
};

template <typename T>
struct is_same<T, T>
{
	enum { value = true };
	typedef T type;
};

///////////////////////////////////////////////////////////////////////////
template <class B, class D>
struct is_base_of
{
private:

	typedef typename TypeTraits<B>::ValueType* BasePtr;
	typedef typename TypeTraits<D>::ValueType* DerivedPtr;

	struct N { char x[1]; };
	struct Y { char x[2]; };

	static Y test(B*);
	static N test(...);

	static DerivedPtr create_derived();

	static void check_constraints(DerivedPtr p) { BasePtr pBase = p; pBase = p; }

public:

	enum { value = sizeof(test(create_derived())) == sizeof(Y) };
};

///////////////////////////////////////////////////////////////////////////
template <class T>
struct is_stl_sequence
{
private:

	struct N { char x[1]; };
	struct Y { char x[2]; };

	template <typename C>
	static Y test(const typename C::value_type*);

	template <typename C>
	static N test(...);

public:

	enum { value = sizeof(test<T>(0)) == sizeof(Y) };
};

template <>
struct is_stl_sequence<std::string>
{ enum { value = false }; };

///////////////////////////////////////////////////////////////////////////
typedef struct _xmlNode xmlNode;

///////////////////////////////////////////////////////////////////////////
// --- a generic version for integral type T ---
template <
	typename T,
	typename Qualifier = typename enable_if<is_base_of<ApiBaseObject, T>::value, ApiBaseObject>::type,
	bool C = is_stl_sequence<T>::value
>
struct ApiTypeHelper
{
	TYPEDEF_TRAITS_EX(T, Qualifier)

	static bool IsTheSame(ParamType obj_a, ParamType obj_b)
	{ return obj_a == obj_b; }

	static void Reinit(ValueType& obj)
	{ obj = ValueType(); }

	static size_t CurrentBinarySize(ParamType /*obj*/)
	{ return sizeof(ValueType); }

	static void InitHelper(void* var, const char* initializer, const xmlNode* /*node*/)
	{
		ValueType& obj = *reinterpret_cast<ValueType*>(var);

		if (initializer)
			obj = static_cast<ValueType>(atol(initializer));
		else
			obj = ValueType();
	}

	static void ReadFromXml(ValueType& obj, const xmlNode* /*node*/)
	{}

	static bool IsAssigned(ParamType obj)
	{ return true; }
};

// --- a specialized version for bool ---
template <>
inline void ApiTypeHelper<bool>::InitHelper(void* var, const char* initializer, const xmlNode* /*node*/)
{
	ValueType& obj = *reinterpret_cast<ValueType*>(var);
	obj = initializer && 0 == strcmp(initializer, "true");
}

// --- a specialized version for double ---
template <>
inline void ApiTypeHelper<double>::InitHelper(void* var, const char* initializer, const xmlNode* /*node*/)
{
	ValueType& obj = *reinterpret_cast<ValueType*>(var);
	obj = initializer && atof(initializer);
}

///////////////////////////////////////////////////////////////////////////
// ApiTypeProxy provides a way to initialize ApiType<T> with a variety of ways

// --- a generic version for integral type T ---
template <typename T>
struct ApiTypeProxy
{
	template <typename X>
	friend class ApiType;

	TYPEDEF_TRAITS(T)

	ApiTypeProxy()
		: value_()
	{}

	ApiTypeProxy(ParamType value)
		: value_(value)
	{}

	ApiTypeProxy(const char* value)
		: value_(static_cast<T>(value ? atol(value) : 0))
	{}

protected:

	ValueType value_;
};

// --- a specialized version for bool ---
template <>
struct ApiTypeProxy<bool>
{
	template <typename X>
	friend class ApiType;

	ApiTypeProxy()
		: value_()
	{}

	ApiTypeProxy(bool value)
		: value_(value)
	{}

	ApiTypeProxy(const char* value)
		: value_(value ? !strcmp(value, "true") : false)
	{}

protected:

	bool value_;
};

// --- a specialized version for double ---
template <>
struct ApiTypeProxy<double>
{
	template <typename X>
	friend class ApiType;

	ApiTypeProxy()
		: value_()
	{}

	ApiTypeProxy(double value)
		: value_(value)
	{}

	ApiTypeProxy(const char* value)
		: value_(value ? atof(value) : 0.)
	{}

protected:

	double value_;
};

// --- a specialized version for std::string ---
template <>
struct ApiTypeProxy<std::string>
{
	template <typename X>
	friend class ApiType;

	ApiTypeProxy()
	{}

	ApiTypeProxy(const std::string& value)
		: value_(value)
	{}

	ApiTypeProxy(const char* value)
		: value_(value ? value : "")
	{}

protected:

	std::string value_;
};

///////////////////////////////////////////////////////////////////////////
template <class T>
struct ValueFormatter
{
	TYPEDEF_TRAITS(T)

	static void get(ParamType obj, std::string& value);
};

///////////////////////////////////////////////////////////////////////////
template <class T>
void ValueFormatter<T>::get(ParamType obj, std::string& value)
{
	std::ostringstream os;
	os << obj;
	value = os.str();
}

template <>
inline void ValueFormatter<std::string>::get(ParamType obj, std::string& value)
{ value = obj; }

template <>
inline void ValueFormatter<const char*>::get(ParamType obj, std::string& value)
{ value = obj; }

template <>
inline void ValueFormatter<bool>::get(ParamType obj, std::string& value)
{ value = obj ? "true" : "false"; }

///////////////////////////////////////////////////////////////////////////
template <
	class T,
	class Q = typename enable_if<is_base_of<ApiBaseObject, T>::value, ApiBaseObject>::type,
	bool C = is_stl_sequence<T>::value
>
struct ContainerEnumerator; // specialize for every complex type in use

///////////////////////////////////////////////////////////////////////////
template <class T>
struct WriteToHelper<T, ds_member>
{
	TYPEDEF_TRAITS(T)

	// for containers of APiBaseObjectPtr-s
	static void WriteTo(const ValueType& obj, IApiObjectEncoder& encoder)
	{
		if (ApiTypeHelper<T>::IsAssigned(obj))
			ContainerEnumerator<T>::write(obj, encoder);
	}

	// for containers of other types;
	// for ApiBaseObject's derived classes/structs
	static void WriteTo(const ValueType& obj, IApiObjectEncoder& encoder, const utfChar* tag, const utfChar* prefix = NULL)
	{
		if (ApiTypeHelper<T>::IsAssigned(obj))
			ContainerEnumerator<T>::write(obj, encoder, tag, prefix);
	}
};

template <class T>
struct WriteToHelper<T, ds_attr>
{
	TYPEDEF_TRAITS(T)

	static void WriteTo(ParamType obj, IApiObjectEncoder& encoder, const utfChar* tag, const utfChar* prefix = NULL)
	{
		if (ApiTypeHelper<T>::IsAssigned(obj))
		{
			std::string value;
			ValueFormatter<T>::get(obj, value);
			encoder.addAttribute(tag, (const utfChar*)value.c_str(), prefix);
		}
	}
};

template <class T>
struct WriteToHelper<T, ds_text>
{
	TYPEDEF_TRAITS(T)

	static void WriteTo(ParamType obj, IApiObjectEncoder& encoder, const utfChar* tag, const utfChar* prefix = NULL)
	{
		if (ApiTypeHelper<T>::IsAssigned(obj))
		{
			std::string value;
			ValueFormatter<T>::get(obj, value);
			encoder.addElement(tag, (const utfChar*)value.c_str(), prefix);
		}
	}
};

///////////////////////////////////////////////////////////////////////////
#endif // API_TYPE_HELPER_H__
