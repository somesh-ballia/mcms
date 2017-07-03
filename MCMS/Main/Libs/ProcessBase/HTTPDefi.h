#define HTTP_TYPE_UNKNOWN	0
#define HTTP_TYPE_POST		1
#define HTTP_TYPE_GET		2
#define HTTP_TYPE_HEAD		3
#define HTTP_TYPE_RESPONSE	4


#define DOWNLOADFILE "downloadfile"
#define LISTENTOFILE "listentofile"
#define DOWNLOADFILE_BYID "downloadfilebyid"
#define DOWNLOAD "download"

#define HTTP_COMMON_HEADERS "Server: PolycomHTTPServer\r\nConnection: Keep-Alive\r\nCache-control: private\r\n\r\n"

#define HTTP_XSL_STRING "<?xml-stylesheet type=\"text/xsl\" href=\"FilesList.xsl\"?>"
#define HTTP_XSL_SIZE 55

#define CONTENT_TYPE_TEXT	0
#define CONTENT_TYPE_JAVA	1
#define CONTENT_TYPE_JPEG	2
#define CONTENT_TYPE_GIF	3
#define CONTENT_TYPE_XML	4
#define CONTENT_TYPE_WAV	5
#define CONTENT_TYPE_WAV_DOWNLOAD	6
#define CONTENT_TYPE_WAV_DOWNLOAD_BY_ID	7
#define CONTENT_TYPE_XML_XSD_DOWNLOAD	8

#define CONTENT_ENCODING_NONE	0
#define CONTENT_ENCODING_ZIP	1

#define HTTP_UNKNOWN		0
#define HTTP_OK				200
#define HTTP_NOT_SUPPORTED  405
#define HTTP_NOT_FOUND		404
#define HTTP_MOVED			302 
#define HTTP_NOT_MODIFIED	304
#define HTTP_SERVER_ERROR	500


