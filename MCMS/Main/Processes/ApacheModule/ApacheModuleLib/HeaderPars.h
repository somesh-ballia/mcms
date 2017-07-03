// HTTPPars.h: interface for the CHTTPHeaderParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPPARS_H__541F9EB8_33AC_4773_BF78_5D9CAE730BCC__INCLUDED_)
#define AFX_HTTPPARS_H__541F9EB8_33AC_4773_BF78_5D9CAE730BCC__INCLUDED_


#include "httpd.h"
#include <string>

using namespace std;


class CHeaderParser  
{
public:

	CHeaderParser(){;}

public:

	static int GetMessageId(request_rec* pRequestRec);
	static void HttpStatusToCommonStatus(int nHttpStatus, int* pCommonStatus);
	static bool IsZippedContent(request_rec* pRequestRec);
//	static void GetURIInHeader(char* pszHeader, char* pszOutURI);
	static void GetURIInHeader(std::string strHeader, std::string& strOutURI);
    static void GetContentEncoding(request_rec* pRequestRec, string & outEncoding);
    static bool GetLastModify(request_rec *pRequestRec, time_t & outTime);    
	static void GetValueFromPragma(char* strPragma, char** strResult, char* strToLookFor);
	static int GetAction(request_rec* pRequestRec, char** pstrActionName);
	static int CheckIfPragmaValid(request_rec* pRequestRec, char** pszPragma);
	static int GetContentLength(request_rec* pRequestRec);
	static void GetValueFromPragmaStr(char* pszPragma, std::string* strResult, char* strToLookFor);
	static int GetLoginDetail(request_rec* pRequestRec, std::string* pstrUserName, std::string* pstrPassword, std::string* pstrStationName);
	static int GetIfNoneHeader(request_rec* pRequestRec);
};

#endif // !defined(AFX_HTTPPARS_H__541F9EB8_33AC_4773_BF78_5D9CAE730BCC__INCLUDED_)
