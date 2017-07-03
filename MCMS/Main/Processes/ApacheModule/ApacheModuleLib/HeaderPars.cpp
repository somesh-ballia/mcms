// HeaderPars.cpp: implementation of the CHeaderParser class.
//
//////////////////////////////////////////////////////////////////////

#include "HeaderPars.h"
#include <ctype.h>
#include "TraceStream.h"
#include "UnicodeDefines.h"
#include "StatusesGeneral.h"


static time_t ParseTimeString(const char* pTime);
static void CutStr(char* strToCut, char* strFromWhereToCut);
static int MonthFromStr(char* str);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int CHeaderParser::GetMessageId(request_rec* pRequestRec)
{
	if(!pRequestRec)
    {
        //FTRACEINTO << "CXmlMiniParser::GetMessageId: !pRequestRec";
        FTRACESTR(eLevelInfoNormal) << "CXmlMiniParser::GetMessageId: !pRequestRec";
		return -1;
    }

	char* pszMessageId = (char*)apr_table_get(pRequestRec->headers_in,"Pragma");
	if(!pszMessageId || strlen(pszMessageId)==0)
    {
        //FTRACEINTO << "CXmlMiniParser::GetMessageId: No pragma tag in HTTP header";
        //FTRACESTR(eLevelInfoNormal) << "CXmlMiniParser::GetMessageId: No pragma tag in HTTP header";
		return -1;
    }

	pszMessageId = strstr(pszMessageId,"MESSAGE_ID");
	if(!pszMessageId)
    {
		//FTRACEINTO << "CXmlMiniParser::GetMessageId: !pszMessageId";
		FTRACESTR(eLevelInfoNormal) << "CXmlMiniParser::GetMessageId: !pszMessageId";
		return -1;
    }

	pszMessageId += strlen("MESSAGE_ID=");
	if(!isdigit(pszMessageId[0]))
    {
        //FTRACEINTO << "CXmlMiniParser::GetMessageId: !isdigit(pszMessageId[0]";
        FTRACESTR(eLevelInfoNormal) << "CXmlMiniParser::GetMessageId: !isdigit(pszMessageId[0]";
		return -1;
    }

	return atoi(pszMessageId);
}
/////////////////////////////////////////////////////////////////////////////////
int CHeaderParser::GetContentLength(request_rec* pRequestRec)
{
	if(!pRequestRec)
    {

        FTRACESTR(eLevelInfoNormal) << "CHeaderParser::GetContentLength: pRequestRec = null";
		return -1;
    }

	const char* pszContentLength = apr_table_get(pRequestRec->headers_in,"Content-Length");
	if(pszContentLength !=NULL)
	{
		return  atoi(pszContentLength);
	}
	else
	{
		 FTRACESTR(eLevelInfoNormal)  << "CHeaderParser::GetContentLength: pszContentLength = null";
		return -1;
	}

}
int CHeaderParser::GetIfNoneHeader(request_rec* pRequestRec)
{

	if(!pRequestRec)
    {

        FTRACESTR(eLevelDebug) << "CHeaderParser::GetIfNoneHeader: pRequestRec = null";
		return -1;
    }

	const char* pszContentLength = apr_table_get(pRequestRec->headers_in,"If-None-Match");
	if(pszContentLength !=NULL)
	{
		return  atoi(pszContentLength);
	}
	else
	{
		 FTRACESTR(eLevelDebug)  << "CHeaderParser::GetContentLength: pszContentLength = null";

		return -1;
	}
}
///////////////////////////////////////////////////////////////////////////////
int CHeaderParser::GetAction(request_rec* pRequestRec, char** pstrActionName)
{
	int status = STATUS_OK;
	char* pszPragma=NULL;

	status = CheckIfPragmaValid(pRequestRec, &pszPragma);

	if (status == STATUS_OK)
		GetValueFromPragma(pszPragma, &(*pstrActionName), ";ACTION");	//Since ACTION is part of TRANSACTION word, we look for it in with the ;. It must not be the first in the string!!!

	return status;
}
///////////////////////////////////////////////////////////////////////////////
int CHeaderParser::CheckIfPragmaValid(request_rec* pRequestRec, char** pszPragma)
{
	if(!pRequestRec)
    {
        FPASSERTMSG(TRUE, "!pRequestRec");
		return STATUS_FAIL;
    }

	*pszPragma = (char*)apr_table_get(pRequestRec->headers_in,"Pragma");

	if(!(*pszPragma))
    {
        FTRACEINTO << "\nNo pragma tag in HTTP header";
		return STATUS_FAIL;
    }

	return STATUS_OK;
}
///////////////////////////////////////////////////////////////////////////////
void CHeaderParser::GetValueFromPragma(char* pszPragma, char** strResult, char* strToLookFor)
{
	char *pstrStringSearchBegin = NULL, *pstrStringSearchEnd = NULL;

	pstrStringSearchBegin = strstr(pszPragma,strToLookFor);
	if (pstrStringSearchBegin)
	{
		pstrStringSearchBegin += strlen(strToLookFor);
		pstrStringSearchBegin++;	//to skip the equal '=' too.
		pstrStringSearchEnd = strstr(pstrStringSearchBegin, ";");
		int len = 0;
		if (pstrStringSearchEnd)	//if it is not the last text
			len = pstrStringSearchEnd-pstrStringSearchBegin;
		else
			len = strlen(pstrStringSearchBegin);
		*strResult = new char[len+1];
		strncpy(*strResult, pstrStringSearchBegin, len);
		(*strResult)[len]='\0';
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int CHeaderParser::GetLoginDetail(request_rec* pRequestRec, std::string* pstrUserName, std::string* pstrPassword, std::string* pstrStationName)
{
	int status = STATUS_OK;
	char* pszPragma=NULL;

	status = CheckIfPragmaValid(pRequestRec, &pszPragma);

	GetValueFromPragmaStr(pszPragma, pstrStationName, "STATION_NAME");
	GetValueFromPragmaStr(pszPragma, pstrUserName, "USER_NAME");
	GetValueFromPragmaStr(pszPragma, pstrPassword, "PASSWORD");

	return status;
}
///////////////////////////////////////////////////////////////////////////////
void CHeaderParser::GetValueFromPragmaStr(char* pszPragma, std::string* strResult, char* strToLookFor)
{

	std::string pszPragmaStr = std::string(pszPragma);
	unsigned pos = pszPragmaStr.find(strToLookFor);
	std::string strBegin = pszPragmaStr.substr (pos);// STATION_NAME=EMA.F5-ONISSEL-DT;USER_NAME=SUPPORT;PASSWORD=SUPPORT
	unsigned posEnd;
	if (strcmp(strToLookFor,"PASSWORD")==0)
		posEnd = strBegin.length();
	else
		posEnd = strBegin.find(";");
	pos = 1 + strlen(strToLookFor);
	*strResult = strBegin.substr (pos , posEnd-pos);

}
///////////////////////////////////////////////////////////////////////////////
// If-Modified-Since : Tue,27 may 2008 12:52:52 GMT
bool CHeaderParser::GetLastModify(request_rec *pRequestRec, time_t & outTime)
{
    char* pLastModify =NULL;
    char *lpszTemp = (char*)apr_table_get(pRequestRec->headers_in,"If-Modified-Since");
    if (lpszTemp == NULL)
    {
        //FTRACEINTO << "\nNo If-Modified-Since tag in HTTP header";
        return false;
    }

    pLastModify = new char[strlen(lpszTemp)+1];
    strcpy(pLastModify, lpszTemp);

    CutStr(pLastModify,"\r\n");

    outTime = ParseTimeString(pLastModify);
    delete [] pLastModify;

    return true;
}
///////////////////////////////////////////////////////////////////////////////

void CutStr(char* strToCut, char* strFromWhereToCut)
{
	char* lpszTemp = strstr(strToCut, strFromWhereToCut);

	if(lpszTemp)
		*lpszTemp=0;
}

time_t ParseTimeString(const char* pTime)
{
    enum eTimeParseState
        {
            eTimeParseState_Day,
            eTimeParseState_Mounth,
            eTimeParseState_Year,
            eTimeParseState_Hour,
            eTimeParseState_Minute,
            eTimeParseState_Second,
            eTimeParseState_End
        };
#define OVER_STATE(state) (state = (eTimeParseState)(state + 1))
#define IS_DIGIT(d) ('0' <= d && d <= '9')
#define IS_CHAR(ch) ( ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') )

    char tmpRes [5];
    int j = 0;

    struct tm tmTime;
    tmTime.tm_wday = -1;
    tmTime.tm_yday = -1;
    tmTime.tm_isdst = -1;

    bool isFieldFinished = false;
    eTimeParseState currentState = eTimeParseState_Day;

    const int timeLen = strlen(pTime);
    for(int i = 0 ; i < timeLen && eTimeParseState_End > currentState ; i++)
    {
        char ch = pTime[i];

        switch (currentState)
        {
            case eTimeParseState_Day:
            case eTimeParseState_Year:
            case eTimeParseState_Hour:
            case eTimeParseState_Minute:
            case eTimeParseState_Second:
                if(IS_DIGIT(ch))
                {
                    tmpRes[j++] = ch;
                }
                else
                {
                    if(j > 0) // some field has just finished
                    {
                        tmpRes[j] = '\0';
                        j = 0;
                        isFieldFinished = true;
                    }
                }
                break;

            case eTimeParseState_Mounth:
                if(IS_CHAR(ch))
                {
                    tmpRes[j++] = ch;
                }
                else
                {
                    if(j > 0) // some field has just finished
                    {
                        tmpRes[j] = '\0';
                        j = 0;
                        isFieldFinished = true;
                    }
                }
                break;

            default:
                currentState = eTimeParseState_End; // bad flow
                break;
        };

        if(isFieldFinished)
        {
            isFieldFinished = false;
            switch (currentState)
            {
                case eTimeParseState_Day:
                    tmTime.tm_mday = atoi(tmpRes);
                    break;
                case eTimeParseState_Year:
                    tmTime.tm_year = atoi(tmpRes) - 1900;
                    break;
                case eTimeParseState_Hour:
                    tmTime.tm_hour = atoi(tmpRes);
                    break;
                case eTimeParseState_Minute:
                    tmTime.tm_min  = atoi(tmpRes);
                    break;
                case eTimeParseState_Second:
                    tmTime.tm_sec  = atoi(tmpRes);
                    break;
                case eTimeParseState_Mounth:
                    tmTime.tm_mon  = MonthFromStr(tmpRes);
                    break;
                case eTimeParseState_End:
                    break;

                default:
                    currentState = eTimeParseState_End; // bad flow
                    break;
            }

            OVER_STATE(currentState);
        }
    }

    return mktime(&tmTime);
}

int MonthFromStr(char* str)
{
    char* aMonths[] = {
		"xxx", "jan", "feb", "mar", "apr", "may", "jun",
		"jul", "aug", "sep", "oct", "nov", "dec" };
	int nMonth;
	char new_str[4];

	memset(new_str,0,4);

	strncpy(new_str, str, 3);
	new_str[3]='\0';

    // to lower : A -> a
    for(int i = 0 ; i < 3 ; i++)
    {
        if('A' <= new_str[i] && new_str[i] <= 'Z')
        {
            new_str[i] -= ('A' - 'a');
        }
    }

	for( nMonth=1; nMonth <= 12; ++nMonth )
	{
		if (strcmp(new_str, aMonths[nMonth])==0)
			break;
	}

	return nMonth-1;
}

bool CHeaderParser::IsZippedContent(request_rec* pRequestRec)
{
	if(!pRequestRec)
		return false;

	char* pszZip = (char*)apr_table_get(pRequestRec->headers_in,"Content-Encoding");

	if (!pszZip)
		return false;

	return true;
}

void CHeaderParser::GetContentEncoding(request_rec* pRequestRec, string & outEncoding)
{
    if(NULL == pRequestRec)
    {
//		outEncoding = MCMS_INTERNAL_STRING_ENCODE_TYPE;
        return;
    }

	const char *pszContentType = (char*)apr_table_get(pRequestRec->headers_in,"Content-Type");

//     FPTRACE(eLevelInfoNormal, "cucu_lulu content type");
//     FPTRACE(eLevelInfoNormal, pszContentType);

    if (NULL == pszContentType)
    {
//		outEncoding = MCMS_INTERNAL_STRING_ENCODE_TYPE;
        return;
    }

    // pszCharsetVal: Content-Type: text/html; charset=ISO-8859-4
    const char *pszCharsetVal = strstr(pszContentType, "charset=");
    if(NULL == pszCharsetVal)
    {
//        outEncoding = MCMS_INTERNAL_STRING_ENCODE_TYPE;
        return;
    }
    pszCharsetVal += strlen("charset=");

    // pszCharsetVal: ISO-8859-4
    // pszCharsetVal: ISO-8859-4\n
    // pszCharsetVal: ISO-8859-4 something
    // pszCharsetVal: ISO-8859-4 something\n
    const char *spaceEnd = strstr(pszCharsetVal, " ");
    const char *newLineEnd = strstr(pszCharsetVal, "\n");

    const char *pszCharsetValEnd = NULL;
	if(NULL == spaceEnd)
	{
		if(NULL == newLineEnd)
		{
			pszCharsetValEnd = pszCharsetVal + strlen(pszCharsetVal);
		}
		else
		{
			pszCharsetValEnd = newLineEnd;
		}
	}
	else
	{
		pszCharsetValEnd = spaceEnd;
	}

	FPASSERTMSG(NULL == pszCharsetValEnd, "Bad flow : NULL == pszCharsetValEnd");

	char buffer [128]={0};
	int valLen = min(pszCharsetValEnd - pszCharsetVal, (int) sizeof(buffer) - 1);
	strncpy(buffer, pszCharsetVal, valLen);
    buffer[valLen] = '\0';

	outEncoding = buffer;
}



/*void CHeaderParser::GetURIInHeader(char* pszHeader, char* pszOutURI)
{
	if(!pszOutURI)
		return;

	pszOutURI[0] = 0;

	const apr_array_header_t *arr;
    const apr_table_entry_t *elts;

	char* pszStartURI = strstr(pszHeader," ");

	if(!pszStartURI)
	{
		FTRACEINTO << "CXmlMiniParser::GetURIInHeader: failed finding the URI beginning in header";
		return;
	}

	while(isspace(*pszStartURI) && (pszStartURI < (pszHeader + strlen(pszHeader))))
		pszStartURI++;

	if(pszStartURI == (pszHeader + strlen(pszHeader)))
	{
		FTRACEINTO << "CXmlMiniParser::GetURIInHeader: failed finding the URI beginning in header";
		return;
	}

	char* pszEndURI = pszHeader + strlen(pszHeader) - 1;

	while(!isspace(*pszEndURI) && (pszEndURI > pszStartURI))
		pszEndURI--;

	while(isspace(*pszEndURI) && (pszEndURI > pszStartURI))
		pszEndURI--;

	if(pszEndURI <= pszStartURI)
	{
		FTRACEINTO << "CXmlMiniParser::GetURIInHeader: failed finding the URI end in header";
		return;
	}

	strncpy(pszOutURI,pszStartURI,pszEndURI-pszStartURI+1);
	pszOutURI[pszEndURI-pszStartURI+1] = 0;
}*/

void CHeaderParser::GetURIInHeader(std::string strHeader, std::string& strURI)
{
	strURI = "";

	int nStartURI = strHeader.find_first_of(' ');

	if(nStartURI < 0)
	{
		FTRACEINTO << "CXmlMiniParser::GetURIInHeader: failed finding the URI beginning in header";
		return;
	}

	while(isspace(strHeader[nStartURI]) && (nStartURI < (int)strHeader.length()))
		nStartURI++;

	if(nStartURI == (int)strHeader.length())
	{
		FTRACEINTO << "CXmlMiniParser::GetURIInHeader: failed finding the URI beginning in header";
		return;
	}

	int nEndURI = strHeader.find_last_of(' ');

	while(isspace(strHeader[nEndURI]) && (nEndURI > nStartURI))
		nEndURI--;

	if(nEndURI <= nStartURI)
	{
		FTRACEINTO << "CXmlMiniParser::GetURIInHeader: failed finding the URI end in header";
		return;
	}

	strURI = strHeader.substr(nStartURI,nEndURI-nStartURI+1);
}
