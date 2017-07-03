#ifndef API_BASE_OBJECT_PTR_H__
#define API_BASE_OBJECT_PTR_H__

///////////////////////////////////////////////////////////////////////////
#include "ApiBaseObject.h"
#include "ApiFactory.h"

#include <libxml/tree.h>

///////////////////////////////////////////////////////////////////////////
CSegment& operator <<(CSegment& seg, const ApiBaseObjectPtr& obj);
CSegment& operator >>(CSegment& seg, ApiBaseObjectPtr& obj);

///////////////////////////////////////////////////////////////////////////
class ApiBaseObjectPtr
{
public:

	ApiBaseObjectPtr()
		: ptr_(NULL)
		, delete_(false)
	{}

	~ApiBaseObjectPtr()
	{ destroy(); }

	ApiBaseObjectPtr(const ApiBaseObjectPtr& obj)
		: ptr_(obj ? obj->NewCopy() : NULL)
		, delete_(true)
	{}

	ApiBaseObjectPtr(ApiBaseObject* ptr)
		: ptr_(ptr)
		, delete_(false)
	{}

public:

	ApiBaseObject* operator ->()
	{ return ptr_; }

	ApiBaseObject& operator *()
	{ return *ptr_; }

	const ApiBaseObject* operator ->() const
	{ return ptr_; }

	const ApiBaseObject& operator *() const
	{ return *ptr_; }

	template <class T>
	operator const T*() const
	{ return reinterpret_cast<T*>(ptr_); }

	template <class T>
	operator T*()
	{ return reinterpret_cast<T*>(ptr_); }

public:

	ApiBaseObjectPtr& operator =(const ApiBaseObjectPtr& obj)
	{
		destroy();

		if (obj)
			ptr_ = obj->NewCopy();

		delete_ = true;

		return *this;
	}

	bool operator ==(const ApiBaseObjectPtr& obj) const
	{ return (ptr_ && obj.ptr_) ? ptr_->IsTheSame(*obj.ptr_) : ptr_ == obj.ptr_; }

	bool operator !=(const ApiBaseObjectPtr& obj) const
	{ return !(*this == obj); }

	bool operator !() const
	{ return !ptr_; }

	operator bool() const
	{ return !!ptr_; }

	bool IsAssigned() const
	{ return !!ptr_; }

	bool Contains(const char* type) const
	{ return ptr_ ? 0 == strcmp(type, ptr_->objectCodeName()) : false; }

public:

	bool IsTheSame(const ApiBaseObjectPtr& obj) const
	{ return *this == obj; }

	void Clear()
	{ *this = ApiBaseObjectPtr(); }

	size_t CurrentBinarySize() const
	{
		return
			ApiTypeBase(IsAssigned()).CurrentBinarySize() +
			(IsAssigned() ? ApiTypeHelper<const char*>::CurrentBinarySize(ptr_->objectFactory()->id()) + sizeof(size_t) + ptr_->CurrentBinarySize() : 0);
	}

	CSegment& WriteTo(CSegment& seg) const;
	CSegment& ReadFrom(CSegment& seg);

	void WriteTo(IApiObjectEncoder& encoder) const
	{
		if (IsAssigned())
			ptr_->WriteTo(encoder);
	}

	void create(const utfChar* objectTag, const utfChar* objectNsUrn)
	{
		destroy();
		ptr_ = ApiObjectsRegistrar::const_instance().create(objectTag, objectNsUrn);
		delete_ = true;
	}

private:

	template <typename X>
	void create(const std::string& factoryId, X id)
	{
		const IApiObjectsFactory* factory = ApiObjectsFactoriesRegistrar::const_instance().get(factoryId.c_str());

		if (factory)
		{
			ptr_ = factory->create(id);
			delete_ = true;
		}
	}

	void destroy()
	{
		if (delete_)
			delete ptr_;

		ptr_ = NULL;
	}

private:

	ApiBaseObject* ptr_;
	bool           delete_;
};

///////////////////////////////////////////////////////////////////////////
inline CSegment& operator <<(CSegment& seg, const ApiBaseObjectPtr& obj)
{ return obj.WriteTo(seg); }

inline CSegment& operator >>(CSegment& seg, ApiBaseObjectPtr& obj)
{ return obj.ReadFrom(seg); }

///////////////////////////////////////////////////////////////////////////
// a specialized version for ApiBaseObjectPtr
template <typename Q>
struct ApiTypeHelper<ApiBaseObjectPtr, Q>
{
	TYPEDEF_TRAITS_EX(ApiBaseObjectPtr, Q)

	static bool IsTheSame(ParamType obj_a, ParamType obj_b)
	{ return obj_a.IsTheSame(obj_b); }

	static void Reinit(ValueType& obj)
	{ obj.Clear(); }

	static size_t CurrentBinarySize(ParamType obj)
	{ return obj.CurrentBinarySize(); }

	static void InitHelper(void* var, const char* initializer, const xmlNode* node)
	{
		ValueType& obj = *reinterpret_cast<ValueType*>(var);
		obj.create(reinterpret_cast<const utfChar*>(initializer), node->ns ? node->ns->href : NULL);
		ApiTypeHelper::ReadFromXml(obj, node);
	}

	static void ReadFromXml(ValueType& obj, const xmlNode* node)
	{ if (obj) obj->ReadFromXml(node); }

	static bool IsAssigned(ParamType obj)
	{ return obj.IsAssigned(); }
};

///////////////////////////////////////////////////////////////////////////
template <>
struct ContainerEnumerator<ApiBaseObjectPtr>
{
	typedef ApiBaseObjectPtr ValueType;
	typedef const ValueType& ParamType;

	static void write(ParamType obj, IApiObjectEncoder& encoder)
	{
		encoder.startElement(obj->objectTag(), obj->objectNsPrefix());
		obj->WriteTo(encoder);
		encoder.endElement();
	}
};

template <>
struct WriteToHelper<ApiBaseObjectPtr, ds_member>
{
	typedef ApiBaseObjectPtr ValueType;
	typedef const ValueType& ParamType;

	static void WriteTo(const ApiBaseObjectPtr& obj, IApiObjectEncoder& encoder)
	{
		if (obj.IsAssigned())
			ContainerEnumerator<ApiBaseObjectPtr>::write(obj, encoder);
	}
};

///////////////////////////////////////////////////////////////////////////
#endif // API_BASE_OBJECT_PTR_H__
