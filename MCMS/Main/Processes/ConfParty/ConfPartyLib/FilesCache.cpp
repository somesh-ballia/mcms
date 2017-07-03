#include "FilesCache.h"

#include "Trace.h"
#include "TraceStream.h"

#include "Tokenizer.h"

#include <iomanip>

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const CLocalFileDescriptor& obj)
{
	return ostr << obj.path_ << ": {s:" << obj.servers_.counter() << ", u:" << obj.users_.counter() << '}';
}

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator <<(std::ostream& ostr, const CFilesCache& obj)
{
	ostr << "Files in Cache: " << obj.mapURLs_.size() << '\n';

	for (CFilesCache::UrlsMap::const_iterator it = obj.mapURLs_.begin() ; it != obj.mapURLs_.end(); ++it)
		ostr << it->first << " --> " << *(it->second) << '\n';

	return ostr;
}

/////////////////////////////////////////////////////////////////////////////
CFilesCache::CFilesCache()
	: counter_(0)
{
}

#if 0
/////////////////////////////////////////////////////////////////////////////
bool CFilesCache::OnDisconnect(HANDLE remoteID, ActionFuncPtr deleteFunction)
{
	RequestersMap::iterator it = mapRequesters_.find(remoteID);

	if (it == mapRequesters_.end())
		return false;

	FilesList* filesList = it->second;

	while (!filesList->empty())
	{
		CLocalFileDescriptor* file = mapURLs_.find(filesList->front())->second;
		file->servers_.Release();

		if (!file->servers_.counter())
		{
			(*deleteFunction)(file->path().c_str());
			delete file;
		}

		filesList->pop_front();
	}

	delete filesList;
	mapRequesters_.erase(it);

	return true;
}
#endif
/////////////////////////////////////////////////////////////////////////////
const CLocalFileDescriptor* CFilesCache::AddFile(HANDLE remoteID, const std::string& url, MediaFileTypeEnum type)
{
	FilesList* pList = filesList(remoteID);
	std::pair<FilesList::iterator, bool> res = pList->insert(url);

	CLocalFileDescriptor* file = get(url);

	if (!file)
	{
		std::string path;
		urlToPath(remoteID, url, path);

		file = new CLocalFileDescriptor(path, type);
		mapURLs_.insert(std::make_pair(url, file));
	}
	else
		file->available_ = false;

	if (res.second)
		file->servers_.AddRef();

	return file;
}

/////////////////////////////////////////////////////////////////////////////
void CFilesCache::SetFileAvailable(const std::string& url, time_t lastModified)
{
	CLocalFileDescriptor* file = get(url);
	FTRACECOND_AND_RETURN(!file, "The file was removed from cache, url:" << url);

	file->SetAvailable(lastModified);
}

/////////////////////////////////////////////////////////////////////////////
void CFilesCache::RemoveFile(const std::string& url)
{
	// TODO: implement file removal from cache
	// TODO: implement callback for actual deletion of file on disk
	mapURLs_.erase(url);
}

/////////////////////////////////////////////////////////////////////////////
void CFilesCache::Clean()
{
	// TODO: handle deletion of files on disk

	// TODO: handle deletion of FileList objects
	mapRequesters_.clear();

	// TODO: handle deletion of Local File Descriptor objects
	UrlsMap::iterator it = mapURLs_.begin();
	while (it != mapURLs_.end())
	{
		free((*it).second);
		mapURLs_.erase(it);
		it = mapURLs_.begin();
	}

	mapURLs_.clear();
}

/////////////////////////////////////////////////////////////////////////////
const CLocalFileDescriptor* CFilesCache::fileDescriptor(const std::string& url) const
{
	UrlsMap::const_iterator it = mapURLs_.find(url);

	if (it != mapURLs_.end())
	{
		it->second->users_.AddRef();
		return it->second;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
void CFilesCache::urlToPath(HANDLE remoteID, const std::string& url, std::string& path)
{
	++counter_;

	std::string::size_type pos_dir = url.find_last_of('/');

	std::ostringstream ostr;
//	ostr
//		<< std::hex << std::setfill('0') << std::setw(8) << size_t(remoteID)
//		<< '_'
//		<< std::setw(4) << counter_
//		<< '_'
//		<< CLexeme(url.c_str() + pos_dir + 1, url.size() - pos_dir - 1);

    ostr << CLexeme(url.c_str() + pos_dir + 1, url.size() - pos_dir - 1);

	path = ostr.str();

	FTRACEINTO << "URL:" << url << ", Path:" << path;
}

/////////////////////////////////////////////////////////////////////////////
CLocalFileDescriptor* CFilesCache::get(const std::string& url)
{
	UrlsMap::iterator it = mapURLs_.find(url);

	return it != mapURLs_.end() ? it->second : NULL;
}

/////////////////////////////////////////////////////////////////////////////
CFilesCache::FilesList* CFilesCache::filesList(HANDLE remoteID)
{
	RequestersMap::const_iterator it = mapRequesters_.find(remoteID);

	if (it != mapRequesters_.end())
		return it->second;

	FilesList* pList = new FilesList;
	mapRequesters_.insert(std::make_pair(remoteID, pList));

	return pList;
}

/////////////////////////////////////////////////////////////////////////////
