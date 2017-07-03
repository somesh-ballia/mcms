// HTTPPars.cpp: implementation of the CHTTPHeaderParser class.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "HTTPPars.h"
#include "HTTPDefi.h"
#include "Trace.h"
#include "UnicodeDefines.h"
#include "TraceStream.h"


char* CHTTPHeaderParser::WebDirectoryPhysicalNames[] = {"7.256/mcu/WebPages/demo/", "7.256/mcu/WebPages/recording/", "7.256/mcu/schemas/"} ;
char* CHTTPHeaderParser::WebDirectoryVirtualNames[] = {"demo", "recording", "schemas"};
char* CHTTPHeaderParser::WebDirectoryDefaultPageNames[] = {"default.asp", "login.htm", ""};
int CHTTPHeaderParser::m_NumOfDirectories=3;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTTPHeaderParser::CHTTPHeaderParser()
{

}

CHTTPHeaderParser::~CHTTPHeaderParser()
{

}


WORD CHTTPHeaderParser::GetContentLength(char* pHeader, DWORD* ContentLength)
{
	char*	beginContentLength	=NULL;
	char*	endContentLength	=NULL;
	char*	pContentLength		=NULL;
	int		len=0;

	beginContentLength = strstr(pHeader,"Content-Length: ");

	if(!beginContentLength)
	{
		beginContentLength = strstr(pHeader,"Content-length: "); //try with lower-case l
		if(!beginContentLength)
		{
			FTRACESTR(eLevelInfoNormal) << "CHTTPHeaderParser::GetContentLength - can't find Content-length string";
			return FALSE;
		}
	}

	beginContentLength += 16; //go to end of "Content-Length: "

	endContentLength = strstr(beginContentLength, "\r\n");
	if (!endContentLength)
	{
		FTRACESTR(eLevelInfoNormal) << "CHTTPHeaderParser::GetContentLength - can't find the end of the content-length";  
		return FALSE;
	}
	len = endContentLength-beginContentLength;

	pContentLength = new char[len+1];

	memcpy(pContentLength,beginContentLength,len);
	pContentLength[len]='\0';

	*ContentLength = atol(pContentLength);

	delete [] pContentLength;

	if (*ContentLength==0)
	{
		FTRACESTR(eLevelInfoNormal) << "CHTTPHeaderParser::GetContentLength - cant find the content length number";
		return FALSE;
	}
	else
		return TRUE;

	return TRUE;

}

WORD CHTTPHeaderParser::GetHttpRequestType(char* pHeader, int* HTTPType)
{
	if (strstr(pHeader,"POST "))  
	{
		*HTTPType = HTTP_TYPE_POST;
	}
	else if (strstr(pHeader,"GET "))
	{
		*HTTPType = HTTP_TYPE_GET;
	}
	else if (strstr(pHeader,"HEAD "))  
	{
		*HTTPType = HTTP_TYPE_HEAD;
	}
	else
	{
		*HTTPType = HTTP_TYPE_UNKNOWN;
		FTRACESTR(eLevelInfoNormal) << "CHTTPHeaderParser::GetHttpRequestType - HTTP_TYPE_UNKNOWN";
		return FALSE;
	}

	return TRUE;

}

WORD CHTTPHeaderParser::GetContentEncoding(char* pHeader)
{
	char* lpszTemp	=NULL;
	char* pLastModify =NULL;

	lpszTemp = strstr(pHeader,"Content-Encoding: zip");
	if (lpszTemp == NULL)
		return CONTENT_ENCODING_NONE;
	else 
		return CONTENT_ENCODING_ZIP;
	
}

void CHTTPHeaderParser::GetContentCharset(const char* pHeader, string &outEncoding)
{
// "Content-type": "text/xml; charset=UTF-8\r\n"
    
    const char *pszContentType = strstr(pHeader, "Content-Type");
    if(NULL == pszContentType)
    {
//        outEncoding = MCMS_INTERNAL_STRING_ENCODE_TYPE;
        return;
    }

    FPTRACE(eLevelInfoNormal, "cucu-lulu content type");
    FPTRACE(eLevelInfoNormal, pszContentType);
    
    
    const char *pszCharsetVal = strstr(pszContentType, "charset=");
    if(NULL == pszCharsetVal)
    {
//        outEncoding = MCMS_INTERNAL_STRING_ENCODE_TYPE;
        return;
    }
    pszCharsetVal += strlen("charset=");
    
    const char *pszCharsetValEnd = strstr(pszCharsetVal, "\r\n");
    if(NULL == pszCharsetValEnd)
    {
//        outEncoding = MCMS_INTERNAL_STRING_ENCODE_TYPE;
        return;
    }
    
    char buffer [128];
	int valLen = pszCharsetValEnd - pszCharsetVal;
	int minSize = min(valLen, 127);
	strncpy(buffer, pszCharsetVal, minSize);
    buffer[minSize] = '\0';

	outEncoding = buffer;
}

WORD CHTTPHeaderParser::GetLastModify(char* pHeader, time_t* time)
{
	char* lpszTemp	=NULL;
	char* pLastModify =NULL;

	lpszTemp = strstr(pHeader,"If-Modified-Since");
	if (lpszTemp == NULL)
	{
		return FALSE;
	}
	else 
	{
		lpszTemp = strstr(lpszTemp, " ");
		lpszTemp++;

		pLastModify = new char[strlen(lpszTemp)+1];
		strcpy(pLastModify, lpszTemp);
		
		CutStr(pLastModify,"\r\n");

		*time = FromStringTimeToTime_t(pLastModify);

		delete [] pLastModify;

		return TRUE;
	}
}

WORD CHTTPHeaderParser::GetFilePath(char* pHeader, char** pPath, char** pQueryString, int& DirectoryIndex, BYTE& bDirectory)
{
	char* lpszTemp	=NULL;
	char* lpszCopiedTemp =NULL;
	char* lpsztempQueryString = NULL;
	
	*pPath=NULL;
	*pQueryString=NULL;

	lpszTemp = strstr(pHeader,"GET ");
	
	if (lpszTemp==NULL)
		lpszTemp = strstr(pHeader,"HEAD ");
	
	if(lpszTemp==NULL)
	{
		return HTTP_NOT_SUPPORTED;
	}

	lpszCopiedTemp = (char*)calloc (strlen(lpszTemp)+1,1);
	FPASSERT_AND_RETURN_VALUE(!lpszCopiedTemp,HTTP_UNKNOWN);
	strcpy (lpszCopiedTemp, lpszTemp);

	str_tolower(lpszCopiedTemp); //change to lower case

	for (DirectoryIndex=0; DirectoryIndex<m_NumOfDirectories ; DirectoryIndex++)
	{
		lpszTemp = strstr(lpszCopiedTemp,WebDirectoryVirtualNames[DirectoryIndex]); 
		
		if (lpszTemp != NULL)
		{
			lpszTemp--;
			if ((lpszTemp[0]=='/') && lpszTemp[strlen(WebDirectoryVirtualNames[DirectoryIndex])+1]=='/')	//check if this is the virtual directory
			{
				lpszTemp+=(strlen(WebDirectoryVirtualNames[DirectoryIndex])+1);	//go to the end of the virtual directory
				break;
			}
		}
	}	

	if(lpszTemp==NULL)
	{
		return HTTP_NOT_FOUND;   //the virtual directory was not found in the requested adress
	}

	CutStr(lpszTemp, " "); //cut what comes after space

	lpsztempQueryString = strstr(lpszTemp,"?");
	if (lpsztempQueryString) //there's something in the querystring
	{
		lpsztempQueryString++; //move after the "?"
		*pQueryString = (char*)calloc (strlen(lpsztempQueryString)+1,1);
		FPASSERT_AND_RETURN_VALUE(!pQueryString,HTTP_UNKNOWN);
		strcpy (*pQueryString, lpsztempQueryString);
		CutStr(lpszTemp, "?"); //cut what comes after ? in the file path
	}
	
	if(strlen(lpszTemp)==0)  //if path is only virtual directory name, then send redirect to default page
	{
		free(lpszCopiedTemp);
		return HTTP_MOVED;   
	}

	if(*lpszTemp!='/')  
	//there's no backslash after the web site name or there's no dot (indicating that not asking for a file)
	{
		free(lpszCopiedTemp);
		return HTTP_NOT_FOUND;   
	}

	lpszTemp++;   //move after the backslash

	if(strlen(lpszTemp)==0) //if path is only virtual directory name and backslach, ask for default page
	{
		*pPath = (char*)calloc (strlen(WebDirectoryDefaultPageNames[DirectoryIndex])+1,1);  
		if(*pPath)
			strcpy (*pPath, WebDirectoryDefaultPageNames[DirectoryIndex]);
		else
		{
			 FTRACESTR(eLevelError) << "CHTTPHeaderParser::GetFilePath - calloc failed";
			 return HTTP_UNKNOWN;
		}
	}
	else
	{
		*pPath = (char*)calloc (strlen(lpszTemp)+1,1); 
		if(*pPath)
			strcpy (*pPath, lpszTemp);
		else
		{
			FTRACESTR(eLevelError) << "CHTTPHeaderParser::GetFilePath - calloc failed";
			return HTTP_UNKNOWN;
		}
	}

	free(lpszCopiedTemp);

	if(!strstr(*pPath,"."))  
	//there's no dot (indicating that not asking for a file)
	{
		if (strcmp(*pPath,"")==0)
		{
			bDirectory = TRUE;
			return HTTP_OK;		//get directory file list
		}
		free(*pPath);
		return HTTP_NOT_FOUND;   
	}

	return HTTP_OK;
}

char* CHTTPHeaderParser::GetValueFromQueryString(char* pQueryString, char* pValueName)
{
	char* pValue = NULL;
	if (pQueryString)
	{
		char* pTemp = strstr(pQueryString, pValueName);

		if (pTemp != NULL)
		{
			pTemp += (strlen(pValueName)+1); //move after valuename and "="

			pValue = (char*)calloc (strlen(pTemp)+1,1);
			FPASSERT_AND_RETURN_VALUE(!pValue,NULL);			
			strcpy (pValue, pTemp);

			CutStr(pValue, "&");
		}
	}

	return pValue;
}

WORD CHTTPHeaderParser::GetHTTPResponseStatus(char* pHeader, int* status)
{  
	char strStatus[10];

	pHeader += 9 ; //go after "HTTP/1.1 "

	strncpy(strStatus, pHeader, sizeof(strStatus) - 1);
	strStatus[sizeof(strStatus) - 1] = '\0';

	CutStr(strStatus, " "); //cut after the status number;

	*status = atoi(strStatus);

	if ((*status == HTTP_OK) || (*status == HTTP_NOT_SUPPORTED) || (*status == HTTP_NOT_FOUND) 
		|| (*status == HTTP_NOT_MODIFIED) || (*status == HTTP_MOVED) || (*status == HTTP_SERVER_ERROR))
	{
		return TRUE;
	}
	else
		return FALSE; //not one of the known statuses
	
}





time_t CHTTPHeaderParser::FromStringTimeToTime_t(char* pTime)
{
	char* temp_str;
	struct tm tmTime;

	switch (pTime[3])
	{
		case ',':
			// read RFC-1123 (preferred)....
			tmTime.tm_mday = Mid_Num(pTime, 5, 2);
			tmTime.tm_mon  = MonthFromStr(pTime, 8);
			tmTime.tm_year = Mid_Num(pTime, 12, 4)-1900;
			tmTime.tm_hour = Mid_Num(pTime, 17, 2);
			tmTime.tm_min  = Mid_Num(pTime, 20, 2);
			tmTime.tm_sec  = Mid_Num(pTime, 23, 2);
			break;
		case ' ':
			// read ANSI-C m_time format....
			tmTime.tm_mday = Mid_Num(pTime, 8, 2);
			tmTime.tm_mon  = MonthFromStr(pTime, 4);
			tmTime.tm_year = Mid_Num(pTime, 20, 4)-1900;
			tmTime.tm_hour = Mid_Num(pTime, 11, 2);
			tmTime.tm_min  = Mid_Num(pTime, 14, 2);
			tmTime.tm_sec  = Mid_Num(pTime, 17, 2);
			break;
		default:
			temp_str = strstr(pTime, ", ");
			if (temp_str)
			{
				tmTime.tm_mday = Mid_Num(pTime, 2, 2);
				tmTime.tm_mon  = MonthFromStr(pTime, 5);
				tmTime.tm_year = Mid_Num(pTime, 9, 2);
				tmTime.tm_hour = Mid_Num(pTime, 12, 2);
				tmTime.tm_min  = Mid_Num(pTime, 15, 2);
				tmTime.tm_sec  = Mid_Num(pTime, 18, 2);
				// add the correct century....
				tmTime.tm_year += (tmTime.tm_year > 50)?0:100;
			}

			break;
	}

	return mktime(&tmTime);


}






////////////////////////////////////////////////////////////////////////////////////////////
//Function Name : Mid_Num
//Parameters	: char* str - the string
//				  int nFirst - the first index of the string
//				  int nCount - number of digits
//Return Value	: The number
//Description	: take a part of a string and convert it to an integer
////////////////////////////////////////////////////////////////////////////////////////////
int CHTTPHeaderParser::Mid_Num(char* str, int nFirst, int nCount)
{
	char new_str[5];
	int i;
	int num;

	memset(new_str,0,5);

	for (i=0; i<nCount; i++)
		new_str[i]=str[nFirst+i];
	new_str[i]='\0';

	num = atoi(new_str);
	return num;
}


////////////////////////////////////////////////////////////////////////////////////////////
//Function Name : MonthFromStr
//Parameters	: char* str - the date string
//				  int nFirst - the month first position
//Return Value	: The number of the month
//Description	: return the number of the month, according to the month name
////////////////////////////////////////////////////////////////////////////////////////////
int CHTTPHeaderParser::MonthFromStr(char* str, int nFirst)
{
	char* aMonths[] = {
		"xxx", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	int nMonth;
	char new_str[4];

	memset(new_str,0,4);

	strncpy(new_str, str+nFirst, 3);
	new_str[3]='\0';

	for( nMonth=1; nMonth <= 12; ++nMonth )
	{
		if (strcmp(new_str, aMonths[nMonth])==0)
			break;
	}

	return nMonth-1;
}


void CHTTPHeaderParser::CutStr(char* strToCut, char* strFromWhereToCut)
{
	char* lpszTemp = strstr(strToCut, strFromWhereToCut);

	if(lpszTemp)
		*lpszTemp=0;
}
char *CHTTPHeaderParser::str_tolower(char *str)
{
        int i=0;

        while (str[i]!='\0') {
                if (isupper(str[i]))
                        str[i] = tolower(str[i]);
                i++;
        };

        return str;

}

