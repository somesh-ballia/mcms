// EncodingConvertor.cpp

#include "EncodingConvertor.h"

#include <stdio.h>
#include <errno.h>
#include "IConv.h"
#include "ObjString.h"
#include "NStream.h"
#include "Macros.h"
#include "StatusesGeneral.h"
#include "OsFileIF.h"
#include "ApiStatuses.h"
#include "SerializeObject.h"
#include "Trace.h"
#include "TraceStream.h"

STATUS CEncodingConvertor::Convert(const string & encodingTo,
                                   string & outEncodingTypeFrom,
                                   char *& outContent,
                                   ostream &outErrorString)
{
    if(outEncodingTypeFrom.empty())
    {
        // the charset was not found in the http header
        // so it will be extracted from the comment
        // <?xml version="1.0" encoding="utf-8"?>

        GetEncodingType(outContent, outEncodingTypeFrom, outErrorString);
    }

    bool isKnown = CEncodingConvertor::IsKnownEncoding(outEncodingTypeFrom);
    if(false == isKnown)
    {
        return STATUS_UNKNOWN_CHARSET_ENCODING;
    }

    if(outEncodingTypeFrom == encodingTo)
    {
        return STATUS_OK;
    }


    // during converting the size could increase, lets hope that *4 is enough.
    const DWORD maxBuffLen = strlen(outContent) * 4;
    char *pBufferTo = new char [maxBuffLen + 1];

    bool resConv = CIConv::ConvertStringEncoding(outContent,
                                                 pBufferTo,
                                                 maxBuffLen,
                                                 outEncodingTypeFrom,               // encode from
                                                 encodingTo,                        // encode to
                                                 outErrorString);
    if(!resConv)
    {
        outErrorString << "\nEncoding convertion failed. "
                       << outEncodingTypeFrom.c_str() << " -> " <<  encodingTo.c_str();

        PDELETEA(pBufferTo);
        return STATUS_ENCODING_CONVERTION_FAILED;
    }

    PDELETEA(outContent);
    outContent = pBufferTo;
    return STATUS_OK;
}

STATUS CEncodingConvertor::ConvertValidate(const std::string& encodingTo,
                                           std::string& outEncodingTypeFrom,
                                           char*& outContent,
                                           std::ostream& outErrorString)
{
    STATUS statusConvert = CEncodingConvertor::Convert(encodingTo,
                                                       outEncodingTypeFrom,
                                                       outContent,
                                                       outErrorString);
    if(STATUS_OK != statusConvert)
    {
        outErrorString << "\nEncoding convertion failed. "
                       << outEncodingTypeFrom.c_str() << " -> " <<  encodingTo.c_str();
        return statusConvert;
    }


    STATUS statusValidate = CEncodingConvertor::ValidateString(encodingTo,
                                                               outContent,
                                                               outErrorString);
    if(STATUS_OK != statusValidate)
    {
        outErrorString << "\nEncoding convertion failed. "
                       << outEncodingTypeFrom.c_str() << " -> " <<  encodingTo.c_str();
    }

    return statusValidate;
}

STATUS CEncodingConvertor::ValidateString(const std::string& encoding,
                                          const char* pContent,
                                          std::ostream& out)
{
  bool res = CIConv::ValidateStringEncoding(pContent, encoding.c_str(), out);

  if (!res)
  {
    out << "Encoding validation failed, " << encoding.c_str();
    return STATUS_ENCODING_VALIDATION_FAILED;
  }

  return STATUS_OK;
}

// Static
STATUS CEncodingConvertor::ValidateFile(const std::string& encoding,
                                        const std::string& fname,
                                        std::ostream& out)
{
  DWORD len = GetFileSize(fname);
  if (static_cast<DWORD>(-1) == len)
  {
    out << "Failed to stat " << fname << ": " << strerror(errno);
    return STATUS_FILE_NOT_EXISTS;
  }

  if (len > CSerializeObject::GetMaxXMLFileSize())
  {
    out << "File size of " << fname << " " << len
        << " bytes exceeded limit of " << CSerializeObject::GetMaxXMLFileSize()
        << " bytes";
    return STATUS_FILE_OPEN_ERROR;
  }

  if (0 == len)
  {
    out << "File " << fname << " is empty";
    return STATUS_FILE_OPEN_ERROR;
  }

  std::string buf;
  STATUS stat = ReadFileToString(fname.c_str(), len, buf);
  if (STATUS_OK != stat)
  {
    out << "Unable to read " << fname;
    return stat;
  }

  return CEncodingConvertor::ValidateString(encoding, buf.c_str(), out);
}

// returns current format. if for mat is not provided, by default it returns UTF-8.
// for all possible formats run "conv -l".
bool CEncodingConvertor::GetEncodingType(const char *buffer,
                                         string &outEncodingType,
                                         ostream &outErrorStr)
{
    outEncodingType =  "UTF-8";

    //  buffer: <?xml version="1.0" encoding="utf-8"?>
    const char *ptrEncodingType = strstr(buffer, "encoding");
    if(NULL == ptrEncodingType)
    {
        outErrorStr << "the transaction does not contain explicit encoding type. \n" ;
        return false;
    }

    //  ptrEncodingType: encoding="utf-8"?>
    const char *ptrEncodingTypeEqual = strstr(ptrEncodingType, "=");
    if(NULL == ptrEncodingTypeEqual)
    {
        outErrorStr << "no = found. \n" ;
        return false;
    }

    //  ptrEncodingTypeEqual: ="utf-8"?>
    const char *ptrEncodingValueStart = strstr(ptrEncodingTypeEqual, "\"");
    if(NULL == ptrEncodingValueStart)
    {
        outErrorStr << "no first \" found. \n" ;
        return false;
    }
    ptrEncodingValueStart++;

    //  ptrEncodingValueStart: utf-8"?>
    const char *ptrEncodingValueEnd = strstr(ptrEncodingValueStart, "\"");
    if(NULL == ptrEncodingValueEnd)
    {
        outErrorStr << "no last \" found. \n" ;
        return false;
    }

    const int encodingTypeValueLen = ptrEncodingValueEnd - ptrEncodingValueStart;

    char encodingTypeValue[encodingTypeValueLen + 1];
    strncpy(encodingTypeValue, ptrEncodingValueStart, sizeof(encodingTypeValue) - 1);
    encodingTypeValue[sizeof(encodingTypeValue) - 1] = '\0';

    outEncodingType = encodingTypeValue;

    CObjString::ToUpper((char*)outEncodingType.c_str());

    return true;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CEncodingConvertor::GetUtf8NextCharLength(const BYTE currentByte)
{
    static struct{
        const BYTE msBitMask;
        const BYTE firstBytePattern;
		const DWORD len;
    } FirstByteCodeTable [] =
        {
         {0xFC, 0xF8, 5},     // {b11111100, 0x11111000, 5},
         {0xF8, 0xF0, 4},     // {0x11111000, 0x11110000, 4},
         {0xF0, 0xE0, 3},     // {0x11110000, 0x11100000, 3},
         {0xE0, 0xC0, 2},     // {0x11100000, 0x11000000, 2},
         {0x80, 0x0,  1},     // {0x10000000, 0x00000000, 1},   // US-ASCII
        {0, 0, 0xFFFFFFFF}
        };

    for(int i = 0 ; 0xFFFFFFFF != FirstByteCodeTable[i].len ; i++)
    {
        const BYTE currentByteMSB = FirstByteCodeTable[i].msBitMask & currentByte;
        if(FirstByteCodeTable[i].firstBytePattern == currentByteMSB)
        {
            return FirstByteCodeTable[i].len;
        }
    }

    // error case
    perror("current byte was not found in the table");

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
bool CEncodingConvertor::IsFirstByte(const BYTE currentByte)
{
    const BYTE msBitMask = 0xC0;            // 0x11000000
    const BYTE notFirstBytePattern = 0x80;  // 0x10000000

    bool res = (notFirstBytePattern == (currentByte & msBitMask));

    return !res;
}

// the space for limit should be allocated: outBuffer[limitLength] = '\0'
void CEncodingConvertor::CutUtf8String(const char *buffer,   // should be NULL terminated string
                                       char *outBuffer,
                                       const DWORD limitLength)
{
    DWORD strLen = strlen(buffer);
    if(strLen < limitLength)
    {
        strcpy(outBuffer, buffer);
        return;
    }

    for(int i = limitLength ; i > 0 ; i--)
    {
        const BYTE currentByte = buffer[i];
        bool isFirst = IsFirstByte(currentByte);
        if(isFirst)
        {
            strncpy(outBuffer, buffer, i);
            outBuffer[i] = '\0';
            return;
        }
    }
}

bool CEncodingConvertor::IsKnownEncoding(const string &encoding)
{
    static const char * KnownEncodings [] =
        {
            "UTF-8",
            "ASCII",
            "ISO_8859",
            "ISO_8859-1",   // The Americas, Western Europe, Oceania, and much of Africa
            "ISO_8859-2",   // Bosnian, Croatian, Czech, Hungarian, Polish, Romanian, Serbian (in Latin transcription), Serbocroatian, Slovak, Slovenian, Upper Sorbian and Lower Sorbian
            "ISO_8859-3",   // Turkish, Maltese and Esperanto
            "ISO_8859-4",   // Estonian, Latvian, Lithuanian, Greenlandic, and Sami
            "ISO_8859-5",   // Cyrillic
            "ISO_8859-6",   // Arabic
            "ISO_8859-7",   // Greek
            "ISO_8859-8",   // Hebrew
            "ISO_8859-9",   // Turkish
            "ISO_8859-10",  // North Germanic
            "ISO-2022-JP",  // Japanese
            "ISO-2022-KR",  // Korean
            "ISO-2022-CN",  // Chinese
            "The End, Used for eliminate the missed ',' problem"
        };

    string upperLetterEncoding = encoding.c_str();
    CObjString::ToUpper((char *)upperLetterEncoding.c_str());

    const DWORD len = sizeof(KnownEncodings) / sizeof(KnownEncodings[0]) - 1;
    for(DWORD i = 0 ; i < len ; i++)
    {
        if(upperLetterEncoding == KnownEncodings[i])
        {
            return true;
        }
    }
    return false;
}

