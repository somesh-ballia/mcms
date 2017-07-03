#include <errno.h>
#include <ostream>
#include <string>
#include <vector>
#include <algorithm>
#include "SnmpFileFormater.h"
#include "Macros.h"
#include "ObjString.h"
#include "NStream.h"
#include "OsFileIF.h"
#include "FaultDesc.h"
#include "FaultsDefines.h"
#include "AlarmStrTable.h"
#include "AlarmParamValidator.h"


using namespace std;

typedef vector<string> CNotificationNameVector;

//////////////////////////////////////////////////////////////////////
CNotificationNameVector& GetNotificationNameVector()
{
    static CNotificationNameVector vect;
    return vect;
}

//////////////////////////////////////////////////////////////////////
void CleanNotificationNames()
{
    CNotificationNameVector &vect = GetNotificationNameVector();
    vect.clear();
}

//////////////////////////////////////////////////////////////////////
bool IsNotificationNameExist(const char *name)
{
    CNotificationNameVector &vect = GetNotificationNameVector();
    CNotificationNameVector::iterator iFound = find(vect.begin(), vect.end(), name);
    return (iFound != vect.end());
}
//////////////////////////////////////////////////////////////////////
void InsertNotificationName(const char *name)
{
    if(IsNotificationNameExist(name))
    {
        string message = "Notification name [ ";
        message += name;
        message += " ] already exists in Notification map";
        FPASSERTMSG(TRUE, message.c_str());
        return;
    }
    
    CNotificationNameVector &vect = GetNotificationNameVector();
    vect.push_back(name);
}


////////////////////////////////////////////////////////////////////// 
bool IsNotAsciiChar(char ch)
{
    bool res = ( ('a' <= ch && ch <= 'z') ||
                 ('A' <= ch && ch <= 'Z') );
    return !res;
}

////////////////////////////////////////////////////////////////////// 
// converts: ;#&*** cUlU - LULU -> CucuLulu
void CSnmpFileFormater::CreateMIBAlertName(char * strErrorCodeBuffer)
{
    CObjString::ReplaceChar(strErrorCodeBuffer, IsNotAsciiChar, ' ');
    CObjString::ToLower(strErrorCodeBuffer);

    // every word must begin with Big letter.
    if('a' <= strErrorCodeBuffer[0] && strErrorCodeBuffer[0] <= 'z')
    {
        strErrorCodeBuffer[0] -= ('a' - 'A');// b -> B
    }
    
    char *ptrSpace = strstr(strErrorCodeBuffer, " ");
    while(NULL != ptrSpace)
    {
        ptrSpace++;
        if(NULL != ptrSpace)
        {
            if('a' <= *ptrSpace && *ptrSpace <= 'z')
            {
                *ptrSpace -= ('a' - 'A');// b -> B
            }
            ptrSpace = strstr(ptrSpace, " ");
        }   
    }
    
    CObjString::RemoveChars(strErrorCodeBuffer, " ");
}

////////////////////////////////////////////////////////////////////// 
bool CSnmpFileFormater::ReadFromMibFile(const string & fileNameSource, char *& buffer, std::ostream& answer)
{
    const int fileSize = GetFileSize(fileNameSource);
    if(-1 == fileSize)
    {
        int errnoCode = errno;
        answer << "FAILED to get file size of " << fileNameSource.c_str() << "; errno = " << errnoCode;
        return false;
    }
    
    FILE *pFileHandle = fopen(fileNameSource.c_str(), "r");
    if(NULL == pFileHandle)
    {
        int errnoCode = errno;
        answer << "FAILED to open file " << fileNameSource.c_str() << "; errno = " << errnoCode;
        return false;
    }
    
    buffer = new char[fileSize + 1];
    int read = fread(buffer, 1, fileSize, pFileHandle);
    if(0 == read)
    {
        fclose(pFileHandle);
        PDELETEA(buffer);
        
        int errnoCode = errno;
        answer << "FAILED to read from file " << fileNameSource.c_str() << "; errno = " << errnoCode;
        return false;
    }

    if(read < fileSize)
    {
        fclose(pFileHandle);
        PDELETEA(buffer);
        
        int errnoCode = errno;
        answer << "FAILED to read all file " << fileNameSource.c_str() << "; errno = " << errnoCode;
        answer << "\nread < fileSize <=> " << read << " < " << fileSize;
        return false;
    }
    
    fclose(pFileHandle);
    pFileHandle = NULL;
    
    buffer[fileSize] = '\0';
    return true;
}


//////////////////////////////////////////////////////////////////////
void CSnmpFileFormater::DumpAlertMap(std::ostream & ostr)
{
    char strMibName[1024];
    CleanNotificationNames();
    
    const std::map<int, AlarmValue>& map = CAlarmStringConverter::Instance().GetMap();
    for (std::map<int, AlarmValue>::const_iterator iter = map.begin();
         iter != map.end() ;
         iter++)
    {   
        WORD        errorCode           = iter->first;
        const char* strAlarmErrorValue  = iter->second.first.c_str();
        const char* strAlarmDescription = iter->second.second.c_str();
        
        if(!CAlarmParamValidator::IsInAlarmRange(errorCode))
        {
            // this error is fault only
            continue;
        }

        strncpy(strMibName, strAlarmErrorValue, sizeof(strMibName)-1);
        strMibName[sizeof(strMibName)-1] = '\0';

        CreateMIBAlertName(strMibName);

        // remove Duplicate names
        while(IsNotificationNameExist(strMibName))
        {
            strcat(strMibName, "1");
        }
        InsertNotificationName(strMibName);
        
        ostr << endl << endl;
        DumpSigleAlert(strMibName, "AlarmFault", errorCode, strAlarmDescription, ostr);
        ostr << endl << endl;

        WORD clearErrorCode = errorCode + AA_RANGE_LAST;

        ostr << endl << endl;
        DumpSigleAlert(strMibName, "AlarmClear", clearErrorCode, strAlarmDescription, ostr);
        ostr << endl << endl;
    }   
}

//////////////////////////////////////////////////////////////////////
//     rmxNewAlarm  NOTIFICATION-TYPE
//              OBJECTS   { rmxAlarmDescription , rmxActiveAlarmDateAndTime , rmxActiveAlarmIndex  , rmxActiveAlarmListName , rmxActiveAlarmRmxStatus }
//              STATUS  current
//              DESCRIPTION
//                   "New active alarm"
//               ::= { rmxTraps 1 }
void CSnmpFileFormater::DumpSigleAlert(const char *mibName,
                                       const char *mibNameExtension,
                                       WORD errorCode,
                                       const char *strAlarmDescription,
                                       std::ostream & ostr)
{
    const char *newLine = "\n             ";
    
    ostr << "rmx" << mibName << mibNameExtension << "  ";
    ostr << "NOTIFICATION-TYPE" << newLine;
    ostr << "OBJECTS   { rmxAlarmDescription , rmxActiveAlarmDateAndTime, rmxActiveAlarmIndex , rmxActiveAlarmListName , rmxActiveAlarmRmxStatus }" << newLine;
    ostr << "STATUS  current" << newLine;
    ostr << "DESCRIPTION"  << newLine;
    ostr << "\"" << strAlarmDescription << "\"" << newLine;
    ostr << "::= { rmxTraps " << errorCode << " } ";
}


////////////////////////////////////////////////////////////////////// 
bool CSnmpFileFormater::WriteToMibFile(const string & fileNameDest, COstrStream & ostr, std::ostream & answer)
{
    FILE *pFileHandle = fopen(fileNameDest.c_str(), "w");
    if(NULL == pFileHandle)
    {
        int errnoCode = errno;
        answer << "FAILED to open file " << fileNameDest.c_str() << "; errno = " << errnoCode;
        return false;
    }

    bool ret = true;
    DWORD writen = fwrite(ostr.str().c_str(), 1, ostr.str().length(), pFileHandle);
    if(writen != ostr.str().length())
    {
        int errnoCode = errno;
        answer << "FAILED to write to file " << fileNameDest.c_str() << "; errno = " << errnoCode;
        ret = false;
    }
    
    fclose(pFileHandle);
    pFileHandle = NULL;

    return ret;
}

////////////////////////////////////////////////////////////////////// 
void CSnmpFileFormater::RemoveEndPattern(char *buffer, const char *strEndPattern)
{
    CObjString::Reverse(buffer);
    
    string strEndPatternReverse = strEndPattern;
    CObjString::Reverse((char*)(strEndPatternReverse.c_str()));
    
    char *ptrEnd = strstr(buffer, strEndPatternReverse.c_str());
    if(NULL != ptrEnd)
    {
        memset(ptrEnd, ' ', strEndPatternReverse.length());
    }

    CObjString::Reverse(buffer);
}

////////////////////////////////////////////////////////////////////// 
bool CSnmpFileFormater::UpdateNsmpMibFile(const string & fileNameSource,
                                          const string & fileNameDest,
                                          std::ostream & answer)
{
    char *buffer = NULL;
    bool resRead = ReadFromMibFile(fileNameSource, buffer, answer);
    if(false == resRead)
    {
        return false;
    }

    const char *strEndPattern = "END";
    RemoveEndPattern(buffer, strEndPattern);
    
    COstrStream ostr;
    ostr << buffer;
    PDELETEA(buffer);

    DumpAlertMap(ostr);

    ostr << strEndPattern << endl;

    bool resWrite = WriteToMibFile(fileNameDest, ostr, answer);

    return resWrite;
}
