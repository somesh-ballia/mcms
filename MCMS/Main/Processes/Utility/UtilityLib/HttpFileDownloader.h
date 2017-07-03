#ifndef HTTP_FILE_DOWNLOADER_H__
#define HTTP_FILE_DOWNLOADER_H__

/////////////////////////////////////////////////////////////////////////////
#include "IFileDownloader.h"

/////////////////////////////////////////////////////////////////////////////
#include <curl/curl.h>

/////////////////////////////////////////////////////////////////////////////
class CHttpFileDownloader : public IFileDownloader
{
public:

	CHttpFileDownloader(bool isSecure);

	virtual ~CHttpFileDownloader();

	virtual DownloadFileResult download(const std::string& url, const std::string& path);

private:

	static size_t write_function(char* ptr, size_t size, size_t count, void* userdata);

        
private:

	CURL* session_;
	char error_[CURL_ERROR_SIZE];

	FILE* file_;
    time_t lastModifiedTime_;
    std::string path_;
    bool fileModified_;

    static const std::string tmpFileExtensionName_;
    static const int notModifiedCode_;
private: // prohibit copying

	CHttpFileDownloader(const CHttpFileDownloader&);
	void operator =(const CHttpFileDownloader&);
};

/////////////////////////////////////////////////////////////////////////////
#endif // HTTP_FILE_DOWNLOADER_H__
