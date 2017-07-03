#ifndef API_FACTORY_H__
#define API_FACTORY_H__

///////////////////////////////////////////////////////////////////////////
#include "Singleton.h"
#include "Tokenizer.h"

#include <map>
#include <vector>

///////////////////////////////////////////////////////////////////////////
class ApiBaseObject;

///////////////////////////////////////////////////////////////////////////
class IApiObjectsFactory
{
public:

	virtual const char* id() const = 0;

	virtual ApiBaseObject* create(size_t classId) const = 0;

protected:

	virtual ~IApiObjectsFactory()
	{}
};

///////////////////////////////////////////////////////////////////////////
class ApiObjectsFactoriesRegistrar : public SingletonHolder<ApiObjectsFactoriesRegistrar>
{
	friend class SingletonHolder<ApiObjectsFactoriesRegistrar>; // non-public constructor access

public:

	void registerFactory(const IApiObjectsFactory& factory)
	{ map_.insert(std::make_pair(factory.id(), &factory)); }

	const IApiObjectsFactory* get(const char* id) const
	{
		Factories::const_iterator it = map_.find(id);
		return (it != map_.end()) ? it->second : NULL;
	}

private:

	typedef std::map<CLexeme, const IApiObjectsFactory*> Factories;
	Factories map_;
};

///////////////////////////////////////////////////////////////////////////
struct ApiObjectCreator
{
	typedef ApiBaseObject* (*FuncPtr)();

	template <class T>
	static ApiBaseObject* create()
	{ return new T; }
};

///////////////////////////////////////////////////////////////////////////
class ApiObjectsRegistrar : public SingletonHolder<ApiObjectsRegistrar>
{
public:

	ApiBaseObject* create(const utfChar* tag, const utfChar* urn) const
	{
		Objects::const_iterator it = map_.find(std::make_pair(tag, urn));
		return (it == map_.end()) ? NULL : (*it->second)();
	}

	template <class T>
	void registerObject()
	{ map_.insert(std::make_pair(std::make_pair(T::classTag(), T::classNsUrn()), &ApiObjectCreator::create<T>)); }

private:

	typedef std::pair<CLexeme, CLexeme> ObjectKey;
	typedef std::map<ObjectKey, ApiObjectCreator::FuncPtr> Objects;

	Objects map_;
};

///////////////////////////////////////////////////////////////////////////
// This template class is per-schema factory for ApiBaseObject's derived classes.
// Its constructor body is generated by the Xml2Source utility.
template <class F>
class ApiObjectsFactory : public IApiObjectsFactory, public SingletonHolder< ApiObjectsFactory<F> >
{
	friend class SingletonHolder< ApiObjectsFactory >; // provide access to non-public constructor

public:

	virtual const char* id() const
	{ return id_; }

	virtual ApiBaseObject* create(size_t classId) const
	{ return (classId < index_.size()) ? (*index_[classId])() : NULL; }

private:

	ApiObjectsFactory();

private:

	typedef std::vector<ApiObjectCreator::FuncPtr> Creators;
	
	template <class T>
	void registerObject()
	{
		index_[T::classId()] = &ApiObjectCreator::create<T>;
		registrar_.registerObject<T>();
	}

private:

	ApiObjectsRegistrar& registrar_;
	const char*          id_;
	Creators             index_;
};

///////////////////////////////////////////////////////////////////////////
#define DECLARE_API_OBJECTS_FACTORY(F)\
	struct F ## _TAG;\
\
	template class ApiObjectsFactory<F ## _TAG>;\
	typedef ApiObjectsFactory<F ## _TAG> F;\
\

///////////////////////////////////////////////////////////////////////////
#endif // API_FACTORY_H__
