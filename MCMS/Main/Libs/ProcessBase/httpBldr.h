#ifndef _HTTP_BLDR
#define _HTTP_BLDR

#include <sys/time.h>

class CHTTPHeaderBuilder 
{

public:

    // Constructors
	CHTTPHeaderBuilder();		
	virtual ~CHTTPHeaderBuilder();

protected:

	void AddStrToHttpBuffer(char* str);
	void AddContentLengthHeader();
	void AddModifiedTimeHeader();
	void AddContentTypeHeader();
	void AddLastCommonHeaders();
	void ExpandHTTPBuffer(int size);
	void AddStatusHeader(int DirectoryIndex);
	void AddPostHeader(char *path);
	void AddHostHeader(char *ip,WORD port);
	void AddContentEncodingHeader();
	void AddGetHeader(char *path);
	void AddIfModifyHeader(time_t &file_time);

protected:

	time_t	m_file_time;
	char*	m_pHTTPHeader;				//the http header string
	int		m_HTTPHeaderLen;			//The HTTP header length
	int		m_HTTPMaxHeaderLen;			//the http header maximum allocated size	
	int		m_nStatus;					//the return status
	DWORD	m_nContentLenght;			//the length of the "get" file requested
	int		m_nContentType;				//the content type
	int		m_nContentEncoding;			//the content type


};


#endif
