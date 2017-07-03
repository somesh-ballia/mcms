// HttpSAP.h

//Service access point to HTTP interface of a FNEServer
//
#ifndef HTTP_SAP_H_
#define HTTP_SAP_H_

#include <curl/curl.h>
#include <string>

#include "PObject.h"

using namespace std;

class HttpSAP : public CPObject
{
   CLASS_TYPE_1(HttpSAP , CPObject)

 public:
   HttpSAP();
   ~HttpSAP();
   const char* NameOf() const { return "HttpSAP";}
   void perform_request(const string& server_uri, string* response) const;
   static size_t http_response(char *ptr, size_t size, size_t nmemb, void *userdata);

 private:
   CURL *curl;
};

#endif

