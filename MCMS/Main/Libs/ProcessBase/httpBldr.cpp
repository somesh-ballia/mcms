#include <malloc.h>
#include <string.h>
#include "DataTypes.h"
#include "httpBldr.h"
#include "HTTPDefi.h"
#include "Trace.h"
#include "HTTPPars.h"


CHTTPHeaderBuilder::CHTTPHeaderBuilder()
{
	m_pHTTPHeader=(char*)calloc(1000,1);;
	m_HTTPHeaderLen=0;
	m_HTTPMaxHeaderLen=1000;
	m_nStatus = HTTP_OK;
	m_nContentEncoding=CONTENT_ENCODING_NONE;
	m_nContentType=CONTENT_TYPE_TEXT;
	m_file_time = time(0);
	m_nContentLenght = 0;
}

CHTTPHeaderBuilder::~CHTTPHeaderBuilder()
{
	if(m_pHTTPHeader!=NULL)
		free(m_pHTTPHeader);

}

void CHTTPHeaderBuilder::AddStrToHttpBuffer(char* str)
{
	int prvLen=m_HTTPHeaderLen;
	ExpandHTTPBuffer(strlen(str));
	strcpy(&(m_pHTTPHeader[prvLen]),str);
}

void CHTTPHeaderBuilder::AddContentLengthHeader()
{
	char strContentLength[128];
	snprintf(strContentLength,sizeof(strContentLength), "Content-Length: %d\r\n",m_nContentLenght);
	AddStrToHttpBuffer(strContentLength);
}

void CHTTPHeaderBuilder::AddModifiedTimeHeader()
{
	char strTimeModified[128];
	char strTime[30];
	memset(strTimeModified,0,128);
	memset(strTime,0,30);
	
	strncpy(strTime, ctime(&m_file_time), 29);
	(strTime)[29]='\0';

	snprintf(strTimeModified, sizeof(strTimeModified), "Last-Modified: %s\r\n",strTime);
	AddStrToHttpBuffer(strTimeModified);
}

void CHTTPHeaderBuilder::AddContentTypeHeader()
{
	char strContent[128];
	switch(m_nContentType)
	{
		case CONTENT_TYPE_JPEG:
			snprintf(strContent, sizeof(strContent), "Content-Type: %s\r\n","image/jpeg");
			break;
		case CONTENT_TYPE_GIF:
			snprintf(strContent, sizeof(strContent), "Content-Type: %s\r\n","image/gif");
			break;
		case CONTENT_TYPE_JAVA:
			snprintf(strContent, sizeof(strContent), "Content-Type: %s\r\n","application/java-archive");
			break;
		case CONTENT_TYPE_XML:
			snprintf(strContent, sizeof(strContent), "Content-Type: %s; %s\r\n","text/xml", "charset=UTF-8");
//            sprintf(strContent,"Content-Type: %s\r\n","text/xml");
			break;
		case CONTENT_TYPE_WAV:
			snprintf(strContent, sizeof(strContent), "Content-Type: %s\r\n","audio/wav");
			break;
		case CONTENT_TYPE_WAV_DOWNLOAD:
			snprintf(strContent, sizeof(strContent), "Content-Disposition: attachment;\r\nContent-Type: %s\r\n","none");
			break;
		case CONTENT_TYPE_WAV_DOWNLOAD_BY_ID:  //add dummy filename
			snprintf(strContent, sizeof(strContent), "Content-Disposition: attachment; filename=recording.wav\r\nContent-Type: %s\r\n","none");
			break;
		case CONTENT_TYPE_XML_XSD_DOWNLOAD:  
			snprintf(strContent, sizeof(strContent), "Content-Disposition: attachment;\r\nContent-Type: %s\r\n","none");
			break;
		default:
			snprintf(strContent, sizeof(strContent), "Content-Type: %s\r\n","text/html");
			break;
	}
	AddStrToHttpBuffer(strContent);
}

void CHTTPHeaderBuilder::AddContentEncodingHeader()
{

	switch(m_nContentEncoding)
	{
		case CONTENT_ENCODING_ZIP:
			AddStrToHttpBuffer("Content-Encoding: zip\r\n");
			break;
	}
}

void CHTTPHeaderBuilder::AddLastCommonHeaders()
{
	AddStrToHttpBuffer(HTTP_COMMON_HEADERS);
}

void CHTTPHeaderBuilder::AddIfModifyHeader(time_t& file_time)
{
	char strTimeModified[128];
	char strTime[30];

	memset(strTimeModified,0,128);
	memset(strTime,0,30);

	strncpy(strTime, ctime(&file_time), 29);
	(strTime)[29]='\0';

	snprintf(strTimeModified, sizeof(strTimeModified), "If-Modified-Since: %s\r\n",strTime);
	AddStrToHttpBuffer(strTimeModified);
}

void CHTTPHeaderBuilder::ExpandHTTPBuffer(int size)
{
	m_HTTPHeaderLen+=size;

	if((m_HTTPHeaderLen+1)>m_HTTPMaxHeaderLen)
	{
		char *pHttpRes;
		
		pHttpRes = (char*)calloc(m_HTTPHeaderLen+1,1);
		if(pHttpRes)
		{
			m_HTTPMaxHeaderLen=m_HTTPHeaderLen+1;

			if (m_pHTTPHeader)
			{
				strcpy(pHttpRes,m_pHTTPHeader);
				free(m_pHTTPHeader);
			}
			m_pHTTPHeader=pHttpRes;
		}
	}
}

void CHTTPHeaderBuilder::AddStatusHeader(int DirectoryIndex)
{

	char strStatus[128];
	if(m_nStatus==HTTP_OK)
		snprintf(strStatus, sizeof(strStatus), "HTTP/1.1 %d %s\r\n",m_nStatus,"OK");
	else if(m_nStatus==HTTP_NOT_SUPPORTED)
		snprintf(strStatus, sizeof(strStatus), "HTTP/1.1 %d %s\r\n",m_nStatus,"Server does not support requested method");
	else if(m_nStatus==HTTP_NOT_FOUND)
		snprintf(strStatus, sizeof(strStatus), "HTTP/1.1 %d %s\r\n",m_nStatus,"Not found");
	else if(m_nStatus==HTTP_NOT_MODIFIED)
		snprintf(strStatus, sizeof(strStatus), "HTTP/1.1 %d %s\r\n",m_nStatus,"Not Modified");
	else if (m_nStatus==HTTP_MOVED)
		snprintf(strStatus, sizeof(strStatus), "HTTP/1.1 %d %s\r\nLocation: %s\\\r\n",m_nStatus,"Moved", CHTTPHeaderParser::WebDirectoryVirtualNames[DirectoryIndex]);
	else 
		snprintf(strStatus, sizeof(strStatus), "HTTP/1.1 %d %s\r\n",HTTP_SERVER_ERROR,"Unknown server error");
	
	m_HTTPHeaderLen=strlen(strStatus); 

	if((m_HTTPHeaderLen+1)>m_HTTPMaxHeaderLen)
	{
		if (m_pHTTPHeader)
			free(m_pHTTPHeader);
		m_pHTTPHeader = (char*)calloc(m_HTTPHeaderLen+1,1);
		m_HTTPMaxHeaderLen=m_HTTPHeaderLen+1;
	}

	if(m_pHTTPHeader)
		strcpy(m_pHTTPHeader,strStatus);
	else
	{
		m_HTTPHeaderLen = 0;
		FPASSERTMSG(1, "CHTTPHeaderBuilder::AddStatusHeader- calloc failed!!!");
	}
}

void CHTTPHeaderBuilder::AddPostHeader(char *path)
{

	char strStatus[128];
	
	snprintf(strStatus, sizeof(strStatus), "POST %s HTTP/1.0\r\n",path);	//changed to 1.0 cause we don't support "Continue" that we get from IIS in 1.1
		
	m_HTTPHeaderLen=strlen(strStatus); 

	if((m_HTTPHeaderLen+1)>m_HTTPMaxHeaderLen)
	{
		if (m_pHTTPHeader)
			free(m_pHTTPHeader);
		m_pHTTPHeader = (char*)calloc(m_HTTPHeaderLen+1,1);
		m_HTTPMaxHeaderLen=m_HTTPHeaderLen+1;
	}

	if(m_pHTTPHeader)
		strcpy(m_pHTTPHeader,strStatus);
	else
	{
		m_HTTPHeaderLen = 0;
		FPASSERTMSG(1, "CHTTPHeaderBuilder::AddPostHeader- calloc failed!!!");
	}
}

void CHTTPHeaderBuilder::AddHostHeader(char *ip,WORD port)
{
	char strContentLength[128];
	snprintf(strContentLength, sizeof(strContentLength), "Host: %s:%d\r\n",ip,port);
	AddStrToHttpBuffer(strContentLength);
}

void CHTTPHeaderBuilder::AddGetHeader(char *path)
{
	char strStatus[128];

	snprintf(strStatus, sizeof(strStatus), "GET %s HTTP/1.1\r\n",path);

	m_HTTPHeaderLen=strlen(strStatus); 

	if((m_HTTPHeaderLen+1)>m_HTTPMaxHeaderLen)
	{
		if (m_pHTTPHeader)
			free(m_pHTTPHeader);
		m_pHTTPHeader = (char*)calloc(m_HTTPHeaderLen+1,1);
		m_HTTPMaxHeaderLen=m_HTTPHeaderLen+1;
	}

	if(m_pHTTPHeader)
		strcpy(m_pHTTPHeader,strStatus);
	else
	{
		m_HTTPHeaderLen = 0;
		FPASSERTMSG(1, "CHTTPHeaderBuilder::AddGetHeader- calloc failed!!!");
	}
}
