
#ifndef CURLCOMMON_H_
#define CURLCOMMON_H_

#define MY_USR_AGENT "CURL-AGENT 1.0"

typedef struct _DATA
{
	std::string* pstr;
	bool bGrab;
	bool bClearString;
} DATA;

//#define http_get 0
//#define http_post 1
//#define http_put 2

typedef enum {
	eHttpGet = 0,
	eHttpPost,
	eHttpPut,
} HTTP_REQUEST_TYPE;

const std::string SOAP_OR_REST_RESPONSE_OK = "HTTP/1.1 200 OK";
const std::string REST_RESPONSE_CREATED = "HTTP/1.1 201 Created";
const std::string REST_RESPONSE_BAD_REQUEST = "HTTP/1.1 400 Bad Request";


#endif /* CURLCOMMON_H_ */
