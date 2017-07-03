// HttpSAP.cpp

#include "HttpSAP.h"
#include "TraceStream.h"
#include "Trace.h"


// TBD set it in configuration
static const int CURL_CONNECTTIMEOUT = 30;
static const int CURL_TIMEOUT = 120;

HttpSAP::HttpSAP()
{
  curl = curl_easy_init();
  if (!curl)
  {
      PTRACE(eLevelInfoNormal, "Failed to initialize LIB CURL.");
      exit(1);
  }
}

HttpSAP::~HttpSAP()
{
  if(curl)
      curl_easy_cleanup(curl);
}

void HttpSAP::perform_request(const string& url, string* response) const
{
  CURLcode res;

  if( !response )
      return;
  else
      response->clear();

  if ( curl != NULL )
  {
      if (CURLE_OK != (res=curl_easy_setopt(curl, CURLOPT_URL, url.c_str())) ||
          CURLE_OK != (res=curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &HttpSAP::http_response)) ||
          CURLE_OK != (res=curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response)) ||
          CURLE_OK != (res=curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT , CURL_CONNECTTIMEOUT)) ||
          CURLE_OK != (res=curl_easy_setopt(curl, CURLOPT_TIMEOUT , CURL_TIMEOUT)) ||
          CURLE_OK != (res=curl_easy_perform(curl)) )
      {
          TRACESTR (eLevelInfoNormal) << "Failed to send a HTTP request. Error [" << curl_easy_strerror(res)  <<"], code=" << res;
          return;
      }
  }
  return;
}

size_t HttpSAP::http_response(char *ptr, size_t size, size_t nmemb, void *userdata)
{ 
   string* str = reinterpret_cast<string *>(userdata);
   int num_proc_bytes = size * nmemb;

   if(str != NULL )
      str->append( ptr, num_proc_bytes);
   else
      num_proc_bytes=0; 

   return num_proc_bytes;
}
