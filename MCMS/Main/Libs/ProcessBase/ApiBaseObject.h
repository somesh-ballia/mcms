#ifndef API_BASE_OBJECT_H__
#define API_BASE_OBJECT_H__

///////////////////////////////////////////////////////////////////////////
#include "ApiType.h"

///////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <vector>
#include <memory> // std::auto_ptr

///////////////////////////////////////////////////////////////////////////
#define API_OBJECT_FRAMEWORK_VERSION "3.0.22.0105"

///////////////////////////////////////////////////////////////////////////
class CSegment;

class ApiBaseObject;
class ApiBaseObjectPtr;

class IApiObjectsFactory;

///////////////////////////////////////////////////////////////////////////
#define API_TYPE_BASE(CLASS)\
public:\
	static const char* classType()\
	{ return #CLASS; }\
\
	virtual const char* objectCodeName() const\
	{ return #CLASS; }\

///////////////////////////////////////////////////////////////////////////
#define API_TYPE_TAG(FACTORY, ID, CLASS, TAG)\
	friend struct ApiTypeHelper<CLASS>;\
	friend struct ContainerEnumerator<CLASS>;\
\
	API_TYPE_BASE(CLASS)\
\
	static const utfChar* classTag()\
	{ return reinterpret_cast<const utfChar*>(TAG); }\
\
	virtual const utfChar* objectTag() const\
	{ return reinterpret_cast<const utfChar*>(TAG); }\
\
	static size_t classId()\
	{ return ID; }\
\
	virtual size_t objectId() const\
	{ return ID; }\
\
	static const IApiObjectsFactory* classFactory()\
	{ return &FACTORY::const_instance(); }\
\
	virtual const IApiObjectsFactory* objectFactory() const\
	{ return &FACTORY::const_instance(); }\

///////////////////////////////////////////////////////////////////////////
#define API_TYPE_DECLARE_NULL_NS\
\
	static const utfChar* classNsPrefix()\
	{ return NULL; }\
\
	static const utfChar* classNsUrn()\
	{ return NULL; }\

///////////////////////////////////////////////////////////////////////////
#define API_TYPE_DECLARE_NS(PREFIX, URN)\
\
	static const utfChar* classNsPrefix()\
	{ return PREFIX; }\
\
	virtual const utfChar* objectNsPrefix() const\
	{ return PREFIX; }\
\
	static const utfChar* classNsUrn()\
	{ return URN; }\
\
	virtual const utfChar* objectNsUrn() const\
	{ return URN; }\
\

///////////////////////////////////////////////////////////////////////////
#define API_TYPE_BODY(CLASS)\
	bool operator ==(const CLASS& obj) const\
	{ return IsTheSame(obj); }\
\
	bool operator !=(const CLASS& obj) const\
	{ return !(*this == obj); }\
\
	virtual void Clear()\
	{ *this = CLASS(); }\
\
	virtual CLASS* NewEmpty() const\
	{ return new CLASS; }\
\
	virtual CLASS* NewCopy() const\
	{ return new CLASS(*this); }\
\
	virtual bool IsTheSame(const ApiBaseObject& base) const;\
\
	virtual size_t CurrentBinarySize() const;\
\
	virtual void ReadFromXml(const xmlNode* pNode, bool initDefaults = true);\
\
protected:\
\
	virtual CSegment& WriteTo(CSegment& seg) const;\
	virtual CSegment& ReadFrom(CSegment& seg);\
\
	virtual void WriteTo(IApiObjectEncoder& encoder) const;\
\
public:\


///////////////////////////////////////////////////////////////////////////
#define API_TYPE(FACTORY, ID, CLASS, TAG)\
	API_TYPE_TAG(FACTORY, ID, CLASS, TAG)\
	API_TYPE_DECLARE_NULL_NS\
	API_TYPE_BODY(CLASS)

///////////////////////////////////////////////////////////////////////////
#define API_TYPE_NS(FACTORY, ID, CLASS, TAG, PREFIX, URN)\
	API_TYPE_TAG(FACTORY, ID, CLASS, TAG)\
	API_TYPE_DECLARE_NS(PREFIX, URN)\
	API_TYPE_BODY(CLASS)

///////////////////////////////////////////////////////////////////////////
class ApiEncodersFactory; // serialization codecs factory

///////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& os, const ApiBaseObject& obj);

///////////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const ApiBaseObject& obj);
CSegment& operator >>(CSegment& seg, ApiBaseObject& obj);

///////////////////////////////////////////////////////////////////////////
template <class T>
struct TypeTraits<T, ApiBaseObject>
{
	enum { tag = ttv_api_base_object_derived };

	typedef T                ValueType;
	typedef const ValueType& ParamType;
};

///////////////////////////////////////////////////////////////////////////
template <>
struct TypeTraits<ApiBaseObjectPtr>
{
	enum { tag = ttv_api_base_object_ptr };

	typedef ApiBaseObjectPtr ValueType;
	typedef const ValueType& ParamType;
};

///////////////////////////////////////////////////////////////////////////
class ApiBaseObject
{
	friend class ApiBaseObjectPtr;
	friend struct ContainerEnumerator<ApiBaseObjectPtr>;

	friend CSegment& operator <<(CSegment& seg, const ApiBaseObject& obj);
	friend CSegment& operator >>(CSegment& seg, ApiBaseObject& obj);

	friend std::ostream& operator <<(std::ostream& os, const ApiBaseObject& obj);

	API_TYPE_BASE(ApiBaseObject);

	virtual const IApiObjectsFactory* objectFactory() const = 0;

	virtual size_t objectId() const = 0;

	virtual const utfChar* objectTag() const = 0;

	virtual const utfChar* objectNsPrefix() const
	{ return NULL; }

	virtual const utfChar* objectNsUrn() const
	{ return NULL; }

	virtual ~ApiBaseObject()
	{}

public:

	bool ReadFromXmlStream(const char* xmlBuf, size_t len);
	bool ReadFromXmlFile(const std::string& sFullFilePath);
	bool WriteToXmlFile(const std::string& sFullFilePath, bool bOverride = false);

public:

	virtual size_t CurrentBinarySize() const = 0;

	virtual bool IsAssigned() const
	{ return true; }

	virtual void Clear() = 0;
	virtual bool IsTheSame(const ApiBaseObject& base) const = 0;

	virtual void ReadFromXml(const xmlNode* node, bool bIsInitDefaults = true) = 0;

	virtual ApiBaseObject* NewCopy() const = 0;
	virtual ApiBaseObject* NewEmpty() const = 0;

	virtual bool IsRootObject() const
	{ return false; }

protected:

	ApiBaseObject()
	{}

	ApiBaseObject(const ApiBaseObject&)
	{}

	ApiBaseObject& operator =(const ApiBaseObject&)
	{ return *this; }

protected:

	///////////////////////////////////////////////////////////////////////////
	typedef void* ApiBaseObject::* MemberPtr;

	///////////////////////////////////////////////////////////////////////////
	template <typename T, class C>
	union union_cast
	{
		MemberPtr var;
		T C::* hack;

		union_cast(T C::* h)
			: hack(h)
		{}
	};

	///////////////////////////////////////////////////////////////////////////
	template <typename FuncPtr>
	struct ApiBinding
	{
		MemberPtr var;
		FuncPtr funcPtr;
		DataSourceEnum source;

		template <typename T, class C>
		ApiBinding(T C::* v, FuncPtr fPtr, DataSourceEnum src)
			: var(union_cast<T, C>(v).var)
			, funcPtr(fPtr)
			, source(src)
		{}
	};

	///////////////////////////////////////////////////////////////////////////
	typedef void (*DecoderFuncPtr)(void* var, const char* initializer, const xmlNode* node);

	typedef void (*SimpleDecoderFuncPtr)(void* var, const char* initializer); // not yet in use
	typedef void (*ComplexDecoderFuncPtr)(void* var, const xmlNode* node);    // not yet in use

	///////////////////////////////////////////////////////////////////////////
	typedef std::pair<CLexeme, CLexeme> ElementName; // { name, namespace }
	typedef std::map< ElementName, ApiBinding<DecoderFuncPtr> > ReadersMeta;

	template <typename T, class C>
	static void AddReader(ReadersMeta& map, T C::* var, DataSourceEnum source, const utfChar* name, const utfChar* ns_urn = NULL)
	{
		DecoderFuncPtr funcPtr = &ApiTypeHelper<T>::InitHelper;
		map.insert(std::make_pair(std::make_pair(name, ns_urn), ApiBinding<DecoderFuncPtr>(var, funcPtr, source)));
	}
#if 0
	template <typename T, class C>
	static void AddSimpleReader(ReadersMeta& map, T C::* var, bool isAttribute, const utfChar* name, const utfChar* ns_urn = NULL)
	{
		DecoderFuncPtr funcPtr = reinterpret_cast<DecoderFuncPtr>(&ParserHelper<T>::read);
		map.insert(std::make_pair(std::make_pair(name, ns_urn), ApiBinding<DecoderFuncPtr>(var, funcPtr, isAttribute ? ds_attr : ds_text)));
	}

	template <typename T, class C>
	static void AddComplexReader(ReadersMeta& map, T C::* var, const utfChar* name, const utfChar* ns_urn = NULL)
	{
		DecoderFuncPtr funcPtr = reinterpret_cast<DecoderFuncPtr>(&EnumeratorHelper<T>::read);
		map.insert(std::make_pair(std::make_pair(name, ns_urn), ApiBinding<DecoderFuncPtr>(var, funcPtr, ds_member)));
	}
#endif

	///////////////////////////////////////////////////////////////////////////
	static void InvokeInitHelper(ReadersMeta::const_iterator it, const char* initializer, const xmlNode* node, ApiBaseObject* pThis, bool initDefaults);

	static void HandleReadXmlAttributes(const ReadersMeta& map, const xmlNode* node, ApiBaseObject* pThis, bool initDefaults = true);
	static void HandleReadXmlElements(const ReadersMeta& map, const xmlNode* node, ApiBaseObject* pThis, bool initDefaults = true);

private:

	virtual CSegment& WriteTo(CSegment& seg) const = 0;
	virtual CSegment& ReadFrom(CSegment& seg) = 0;

	virtual void WriteTo(IApiObjectEncoder& encoder) const = 0;
};

///////////////////////////////////////////////////////////////////////////
inline CSegment& operator <<(CSegment& seg, const ApiBaseObject& obj)
{ return obj.WriteTo(seg); }

inline CSegment& operator >>(CSegment& seg, ApiBaseObject& obj)
{ return obj.ReadFrom(seg); }

///////////////////////////////////////////////////////////////////////////
// --- a specialized version for ApiBaseObject's derived classes T ---
template <class T>
struct ApiTypeHelper<T, ApiBaseObject, false>
{
	TYPEDEF_TRAITS_EX(T, ApiBaseObject)

	static bool IsTheSame(ParamType obj_a, ParamType obj_b)
	{ return obj_a.IsTheSame(obj_b); }

	static void Reinit(ValueType& obj)
	{ obj.Clear(); }

	static size_t CurrentBinarySize(ParamType obj)
	{ return obj.CurrentBinarySize(); }

	static void InitHelper(void* var, const char* /*initializer*/, const xmlNode* node)
	{
		ValueType& obj = *reinterpret_cast<ValueType*>(var);
		ApiTypeHelper::ReadFromXml(obj, node);
	}

	static void ReadFromXml(ValueType& obj, const xmlNode* node)
	{ obj.ReadFromXml(node); }

	static bool IsAssigned(ParamType obj)
	{ return obj.IsAssigned(); }
};

///////////////////////////////////////////////////////////////////////////
// --- a specialized version for ApiBaseObject's derived classes T ---
template <class T>
struct ContainerEnumerator<T, ApiBaseObject, false>
{
	TYPEDEF_TRAITS_EX(T, ApiBaseObject)

	static void write(ParamType obj, IApiObjectEncoder& encoder, const utfChar* tag, const utfChar* prefix = NULL)
	{
		encoder.startElement(tag, prefix);
		obj.WriteTo(encoder);
		encoder.endElement();
	}
};

///////////////////////////////////////////////////////////////////////////
#endif // API_BASE_OBJECT_H__
