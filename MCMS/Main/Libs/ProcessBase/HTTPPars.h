// HTTPPars.h: interface for the CHTTPHeaderParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPPARS_H__541F9EB8_33AC_4773_BF78_5D9CAE730BCC__INCLUDED_)
#define AFX_HTTPPARS_H__541F9EB8_33AC_4773_BF78_5D9CAE730BCC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <DataTypes.h>
#include <time.h>
#include <string>

using namespace std;



class CHTTPHeaderParser  
{
public:
	CHTTPHeaderParser();
	virtual ~CHTTPHeaderParser();

	static WORD GetContentLength(char* pHeader, DWORD* ContentLength);
	static WORD GetHttpRequestType(char* pHeader, int* HTTPType);
	static WORD GetLastModify(char* pHeader, time_t* time);
	static WORD GetFilePath(char* pHeader, char** pPath, char** pQueryString, int& DirectoryIndex, BYTE& bDirectory);
	static WORD GetHTTPResponseStatus(char* pHeader, int* status);
	static WORD GetContentEncoding(char* pHeader);
    static void GetContentCharset(const char* pHeader, string &outEncoding);
	static char* GetValueFromQueryString(char* pQueryString, char* pValueName);

	static char* WebDirectoryPhysicalNames[];
	static char* WebDirectoryVirtualNames[];
	static char* WebDirectoryDefaultPageNames[];
	static int m_NumOfDirectories;


protected:
	static int Mid_Num(char* str, int nFirst, int nCount);
	static int MonthFromStr(char* str, int nFirst);
	static void CutStr(char* strToCut, char* strFromWhereToCut);
	static time_t FromStringTimeToTime_t(char* pTime);
	static char *str_tolower(char *str);
};

#endif // !defined(AFX_HTTPPARS_H__541F9EB8_33AC_4773_BF78_5D9CAE730BCC__INCLUDED_)
