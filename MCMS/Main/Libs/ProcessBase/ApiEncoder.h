#ifndef API_ENCODER_H__
#define API_ENCODER_H__

///////////////////////////////////////////////////////////////////////////
#include "Singleton.h"
#include "Tokenizer.h"

///////////////////////////////////////////////////////////////////////////
#include <map>
#include <memory>

///////////////////////////////////////////////////////////////////////////
typedef unsigned char utfChar;

///////////////////////////////////////////////////////////////////////////
class IApiObjectEncoder
{
public:

	virtual ~IApiObjectEncoder() = 0; // MUST be public: it is accessed by auto_ptr's destructor

	virtual bool addNamespace(const utfChar* href, const utfChar* prefix = NULL) = 0;

	virtual bool startElement(const utfChar* tag, const utfChar* prefix = NULL) = 0;
	virtual bool endElement() = 0;

	virtual bool addElement(const utfChar* tag, const utfChar* value, const utfChar* prefix = NULL) = 0;

	virtual bool addAttribute(const utfChar* tag, const utfChar* value, const utfChar* prefix = NULL) = 0;

	virtual bool addComment(const utfChar* text) = 0;

	virtual const utfChar* buffer() const = 0;
};

///////////////////////////////////////////////////////////////////////////
inline IApiObjectEncoder::~IApiObjectEncoder()
{}

///////////////////////////////////////////////////////////////////////////
class ApiEncodersFactory : public SingletonHolder<ApiEncodersFactory>
{
	friend class SingletonHolder<ApiEncodersFactory>;

public:

	std::auto_ptr<IApiObjectEncoder> encoder(const char* codec) const;

private:

	ApiEncodersFactory();

private:

	typedef IApiObjectEncoder* (*CreatorFunctionPtr)();

	template <class T>
	static IApiObjectEncoder* CreatorFunction()
	{ return new T; }

	template <class T>
	void registerEncoder(const char* codec)
	{
		CreatorFunctionPtr function = &CreatorFunction<T>;
		map_.insert(std::make_pair(codec, function));
	}

private:

	typedef std::map<CLexeme, CreatorFunctionPtr> Map;
	Map map_;
};

///////////////////////////////////////////////////////////////////////////
#endif // API_ENCODER_H__
