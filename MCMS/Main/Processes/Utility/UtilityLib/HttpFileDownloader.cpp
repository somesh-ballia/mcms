#include "HttpFileDownloader.h"

#include "Trace.h"
#include "TraceStream.h"

#include <curl/curl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>  
#include <linux/rtc.h>
#include <time.h>
#include <sstream>

const std::string CHttpFileDownloader::tmpFileExtensionName_ = ".tmp";
const int CHttpFileDownloader::notModifiedCode_ = 304;

/////////////////////////////////////////////////////////////////////////////
CHttpFileDownloader::CHttpFileDownloader(bool isSecure)
	: session_(curl_easy_init())
	, error_()
	, file_(NULL)
	, lastModifiedTime_(0)
	, fileModified_(true)
{
	FPASSERTMSG(!session_, error_);
	curl_easy_setopt(session_, CURLOPT_HEADER, 0l);

	curl_easy_setopt(session_, CURLOPT_ERRORBUFFER, &error_[0]);
	curl_easy_setopt(session_, CURLOPT_WRITEFUNCTION, &CHttpFileDownloader::write_function);
//	curl_easy_setopt(session_, CURLOPT_TIMEOUT , 5); // TODO: replace with System Flag's value

	if (isSecure)
	{
		curl_easy_setopt(session_, CURLOPT_SSL_VERIFYHOST, 0l);
		curl_easy_setopt(session_, CURLOPT_SSL_VERIFYPEER, 0l);
	}
}

/////////////////////////////////////////////////////////////////////////////
CHttpFileDownloader::~CHttpFileDownloader()
{
	curl_easy_cleanup(session_);
	session_ = NULL;

	if (file_)
	{
		fclose(file_);
		file_ = NULL;

        if (fileModified_)
        {
            std::stringstream ssCmd;
            ssCmd << "mv " << path_ << tmpFileExtensionName_ << " " << path_;
            FTRACEINTO << "File is modified, update the original file:" << ssCmd.str().c_str();
            system(ssCmd.str().c_str());

            ssCmd.str("");
            ssCmd << "chmod 777 " << path_;
            system(ssCmd.str().c_str());
            
            char arrLastModified[256];
            struct tm *pLastModified = localtime(&lastModifiedTime_);
            if(NULL == pLastModified){
                FTRACEINTO << "Will not set modified time. localtime return NULL!";
                return;
            }
            snprintf(arrLastModified, sizeof(arrLastModified), "%4.4d%2.2d%2.2d%2.2d%2.2d.%2.2d", 
                (1900+pLastModified->tm_year),(1+pLastModified->tm_mon), pLastModified->tm_mday, 
                pLastModified->tm_hour, pLastModified->tm_min, pLastModified->tm_sec);
            std::stringstream ssChangeModifiedTime;
            ssChangeModifiedTime << "touch -c -m -t " << arrLastModified << " " << path_;
            FTRACEINTO << "Set modified time:" << ssChangeModifiedTime.str().c_str();
            system(ssChangeModifiedTime.str().c_str());
        }
        else
        {
            std::stringstream ssCmd;
            ssCmd << "rm -fr " << path_ << tmpFileExtensionName_;
            FTRACEINTO << "File is not modified, remove the temp file:" << ssCmd.str().c_str();
            system(ssCmd.str().c_str());
        }
	}
}

/////////////////////////////////////////////////////////////////////////////
DownloadFileResult CHttpFileDownloader::download(const std::string& url, const std::string& path)
{
	FTRACEINTO << "url:" << url << ", filepath:" << path;

    struct curl_slist *requestHeaders = 0;
    time_t localLastModified = 1;
    if (-1 != (access(path.c_str(),F_OK)))
    {
        struct stat info;
        stat(path.c_str(), &info);
        localLastModified = info.st_mtime;

        struct tm *pFileModified = localtime(&localLastModified);
        if(pFileModified){
            char arrFileModified[256];
        
            strftime(arrFileModified, sizeof(arrFileModified), "%a, %d %b %Y %X GMT", pFileModified);

            std::string ifModifiedSince("If-Modified-Since: ");
            ifModifiedSince.append(arrFileModified);
            requestHeaders = curl_slist_append(requestHeaders, ifModifiedSince.c_str());

            curl_easy_setopt(session_, CURLOPT_HTTPHEADER, requestHeaders);
        
            FTRACEINTO << "Set ifModifiedSince -- url:" << url << ", info.st_mtime:" << info.st_mtime 
                << ", ifModifiedSince:" << ifModifiedSince;
        }else{
            FTRACEINTO << "Set ifModifiedSince failed due to localtime return null!";
        }
    }
    
	file_ = fopen((path + tmpFileExtensionName_).c_str(), "wb");
	FPASSERT_AND_RETURN_VALUE(!file_, DOWNLOAD_FILE_FAILED);

	CURLcode code;

	code = curl_easy_setopt(session_, CURLOPT_FILE, file_);
	FTRACECOND_AND_RETURN_VALUE(code, &error_, DOWNLOAD_FILE_FAILED);

	code = curl_easy_setopt(session_, CURLOPT_URL, url.c_str());
	FTRACECOND_AND_RETURN_VALUE(code, &error_, DOWNLOAD_FILE_FAILED);

    // One last thing; always try to get the last modified time
    code = curl_easy_setopt(session_, CURLOPT_FILETIME, (long)localLastModified);
    FTRACECOND_AND_RETURN_VALUE(code, &error_, DOWNLOAD_FILE_FAILED);
    
	code = curl_easy_perform(session_);
	FTRACECOND_AND_RETURN_VALUE(code, &error_, DOWNLOAD_FILE_FAILED);

	long serverLastModified;
	curl_easy_getinfo(session_, CURLINFO_FILETIME, &serverLastModified);

    struct tm *pLastModified = localtime(&serverLastModified);
    if(pLastModified){
        char arrLastModified[256];

        strftime(arrLastModified, sizeof(arrLastModified), "%a, %d %b %Y %X GMT", pLastModified);
    
	    FTRACEINTO << "Downloaded file done. url:" << url << ", path:" << path << ", serverLastModified:" << serverLastModified << ", formatted time:" << arrLastModified;
    }

    int responseCode = 200;
    if (CURLE_OK == code)
    {
        if (curl_easy_getinfo(session_, CURLINFO_RESPONSE_CODE, &responseCode) == CURLE_OK)
        {
            fileModified_ = notModifiedCode_ == responseCode ? false : true;
            FTRACEINTO << "url:" << url << ", responseCode:" << responseCode;
        }
    }
    else
    {
        FTRACEINTO << "curl_easy_perform abnormal, CURLE_OK != code, url:" << url; 
    }

    lastModifiedTime_ = serverLastModified;
    path_ = path;

    curl_slist_free_all(requestHeaders);

    if (CURLE_OK == code)
    {
        return fileModified_ ? DOWNLOAD_FILE_OK : DOWNLOAD_FILE_NOT_MODIFIED;
    }
    else
    {
        return DOWNLOAD_FILE_FAILED;
    }
}

/////////////////////////////////////////////////////////////////////////////
size_t CHttpFileDownloader::write_function(char* ptr, size_t size, size_t count, void* userdata)
{
	FILE* file = reinterpret_cast<FILE*>(userdata);
	size_t written = fwrite(ptr, size, count, file);
	return written;
}

/////////////////////////////////////////////////////////////////////////////

