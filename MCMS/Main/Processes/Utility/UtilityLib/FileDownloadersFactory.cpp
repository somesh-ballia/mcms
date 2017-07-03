#include "HttpFileDownloader.h"

/////////////////////////////////////////////////////////////////////////////
CFileDownloadersFactory::CFileDownloadersFactory()
{
	AddToMap<CHttpFileDownloader>("",         false);
	AddToMap<CHttpFileDownloader>("http://",  false);
	AddToMap<CHttpFileDownloader>("https://", true);
}

/////////////////////////////////////////////////////////////////////////////
IFileDownloader* CFileDownloadersFactory::lookup(const std::string& url) const
{
	const char scheme_delimiter[] = "://";

	std::string::size_type pos = url.find(scheme_delimiter);

	switch (pos)
	{
	case std::string::npos:
		pos = 0;
		break;

	default:
		pos += sizeof(scheme_delimiter) - 1;
		break;
	}

	CLexeme scheme(url.c_str(), pos);

	Map::const_iterator it = map_.find(scheme);

	return it == map_.end() ? NULL : (*it->second.first)(it->second.second);
}

/////////////////////////////////////////////////////////////////////////////
void CFileDownloadersFactory::destroy(IFileDownloader* pDownloader) const
{
	delete pDownloader;
}

/////////////////////////////////////////////////////////////////////////////
