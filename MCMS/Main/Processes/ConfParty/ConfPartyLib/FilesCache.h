#ifndef FILES_CACHE_H__
#define FILES_CACHE_H__

/////////////////////////////////////////////////////////////////////////////
#include "Singleton.h"

#include "DataTypes.h"

#include "MediaTypes.h"

#include <iostream>
#include <map>
#include <set>

#include <sys/time.h>

/////////////////////////////////////////////////////////////////////////////
class CLocalFileDescriptor;
class CFilesCache;
class CFilesDownloader;

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const CLocalFileDescriptor& obj);
std::ostream& operator <<(std::ostream& ostr, const CFilesCache& obj);

/////////////////////////////////////////////////////////////////////////////
class ReferenceCounted
{
public:

	ReferenceCounted() : refCounter_(0) {}

	void AddRef()  { ++refCounter_; }
	void Release() { --refCounter_; }

	size_t counter() const { return refCounter_; }

private:

	size_t refCounter_;

private: // prohibit copying

	ReferenceCounted(const ReferenceCounted&);
	void operator =(const ReferenceCounted&);
};

/////////////////////////////////////////////////////////////////////////////
class CLocalFileDescriptor
{
	friend class CFilesCache;

	friend std::ostream& operator <<(std::ostream& ostr, const CLocalFileDescriptor& obj);

public:

	const std::string path() const
	{ return path_; }

	MediaFileTypeEnum type() const
	{ return type_; }

	bool available() const
	{ return available_; }

private:

	CLocalFileDescriptor(const std::string& path, MediaFileTypeEnum type)
		: path_(path)
		, type_(type)
		, lastModified_(0)
		, available_(false)
	{
	}

	~CLocalFileDescriptor() {}

	void SetAvailable(time_t lastModified)
	{
		available_ = true;
		lastModified_ = lastModified;
	}

private:

	const std::string       path_;
	const MediaFileTypeEnum type_;

	mutable time_t          lastModified_;
	mutable bool            available_;

	ReferenceCounted        servers_; // application servers' count
	ReferenceCounted        users_;   // local users' count

private: // prohibit copying

	CLocalFileDescriptor(const CLocalFileDescriptor&);
	void operator =(const CLocalFileDescriptor&);
};

/////////////////////////////////////////////////////////////////////////////
class CFilesCache : public SingletonHolder<CFilesCache>
{
	friend class SingletonHolder<CFilesCache>; // provide access to non-public constructor

	friend std::ostream& operator <<(std::ostream& ostr, const CFilesCache& obj);

public:

	const CLocalFileDescriptor* fileDescriptor(const std::string& url) const;
	const CLocalFileDescriptor* AddFile(HANDLE remoteID, const std::string& url, MediaFileTypeEnum type);

	void SetFileAvailable(const std::string& url, time_t lastModified);
	void RemoveFile(const std::string& url);

#if 0
public:

	typedef void (*ActionFuncPtr)(const char*);

	bool OnDisconnect(RemoteID remoteID, ActionFuncPtr deleteFunction);
#endif

	void Clean();

protected:

	CFilesCache();

private:

	void urlToPath(HANDLE remoteID, const std::string& url, std::string& path);

	typedef std::set<std::string> FilesList;
	FilesList* filesList(HANDLE remoteID);

	CLocalFileDescriptor* get(const std::string& url);

private:

	typedef std::map<HANDLE, FilesList*> RequestersMap;
	RequestersMap mapRequesters_;

	typedef std::map<std::string, CLocalFileDescriptor*> UrlsMap;
	UrlsMap mapURLs_;

	size_t counter_;
};

/////////////////////////////////////////////////////////////////////////////
#endif // FILES_CACHE_H__
