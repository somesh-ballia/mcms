#ifndef I_FILE_DOWNLOADER_H__
#define I_FILE_DOWNLOADER_H__

/////////////////////////////////////////////////////////////////////////////
#include "Singleton.h"
#include "Tokenizer.h"
#include "DefinesGeneral.h"

/////////////////////////////////////////////////////////////////////////////
#include <string>
#include <map>

/////////////////////////////////////////////////////////////////////////////
class CFileDownloadersFactory;

/////////////////////////////////////////////////////////////////////////////
class IFileDownloader
{
	friend class CFileDownloadersFactory;

public:
    virtual DownloadFileResult download(const std::string& url, const std::string& path) = 0;

protected:

	virtual ~IFileDownloader()
	{}
};

/////////////////////////////////////////////////////////////////////////////
class CFileDownloadersFactory : public SingletonHolder<CFileDownloadersFactory>
{
	friend class SingletonHolder<CFileDownloadersFactory>;

public:

	IFileDownloader* lookup(const std::string& url) const;

	void destroy(IFileDownloader* pDownloader) const;

private:

	CFileDownloadersFactory();

private:

	typedef IFileDownloader* (*CreatorFunctionPtr)(bool);

	template <class T>
	static IFileDownloader* CreatorFunction(bool isSecure)
	{ return new T(isSecure); }

	template <class T>
	void AddToMap(const char* scheme, bool isSecure)
	{
		CreatorFunctionPtr function = &CreatorFunction<T>;
		map_.insert(std::make_pair(scheme, std::make_pair(function, isSecure)));
	}

private:

	typedef std::pair<CreatorFunctionPtr, bool> Creator;

	typedef std::map<CLexeme, Creator> Map;
	Map map_;
};

/////////////////////////////////////////////////////////////////////////////
#endif // I_FILE_DOWNLOADER_H__
