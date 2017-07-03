#include <errno.h>

#include "ResetHistory.h"
#include "OsFileIF.h"
#include "StructTm.h"
#include "SystemFunctions.h"
#include "NStream.h"
#include "ObjString.h"

const char *stateDir = "States";
const char *resetListFileName = "States/McmsDaemonResetHistory.txt";
const char *startupTitle = "Startup: ";


static bool ReadFile(const char *fileName, char *buffer, const int bufferLen);
static void AppendToFile(const char *fileName, const char *buffer);






/*--------------------------------------------------------------------------
  Implementation of the interface
--------------------------------------------------------------------------*/

/////////////////////////////////////////////////////////////////////////////
int ResetHistory_GetResetNumber()
{
    char buffer[1024];
    
    bool readRes = ReadFile(resetListFileName, buffer, 1024);
    if(false == readRes)
    {
        return 0;
    }

    int bufferLen = strlen(buffer);
    if(0 == bufferLen)
    {
        return 0;
    }

    const char *ptr = strstr(buffer, startupTitle);
    int startupNum = 0;
    while(NULL != ptr)
    {
        startupNum++;

        ptr++;
        if(NULL == ptr)
        {
            break;
        }
        
        ptr = strstr(ptr, startupTitle);
    }
    
    return startupNum;
}

/////////////////////////////////////////////////////////////////////////////
void ResetHistory_AddStartup()
{
    CStructTm startupTime;
    SystemGetTime(startupTime);

    COstrStream ostr;
    startupTime.Serialize(ostr);

    const string time  = ostr.str();

    string buffer = startupTitle;
    buffer += time;

    AppendToFile(resetListFileName, buffer.c_str());
}

/////////////////////////////////////////////////////////////////////////////
void ResetHistory_Remove()
{
    DeleteFile(resetListFileName);
}






/*--------------------------------------------------------------------------
  Implementation of internals
--------------------------------------------------------------------------*/

/////////////////////////////////////////////////////////////////////////////
bool ReadFile(const char *fileName, char *buffer, const int bufferLen)
{
    FILE *file = fopen(resetListFileName, "r");
    if(NULL == file)
    {
        int errnoCode = errno;
        if(ENOENT == errnoCode)
        {
            // file not exist, very good, it means that number of resets is 0
            buffer[0] = '\0';
            return true;
        }
        else
        {
            CSmallString str = "failed to open file : ";
            str << fileName;
            str << "; errno : ";
            str << errnoCode;
            FPASSERTMSG(TRUE, str.GetString());
            return false;
        }
    }

    int read = fread(buffer, sizeof(char), bufferLen - 1, file);
    int errorStatus = ferror(file);
    fclose(file);
    
    if(0 != errorStatus)
    {
        CSmallString str = "failed to read file : ";
        str << fileName;
        str << "; error : ";
        str << errorStatus;
        FPASSERTMSG(TRUE, str.GetString());
        return false;
    }
    
    buffer[read] = '\0';
    return true;
}

/////////////////////////////////////////////////////////////////////////////
void AppendToFile(const char *fileName, const char *buffer)
{
    CreateDirectory(stateDir);
    
    FILE *file = fopen(fileName, "a+");
    if(NULL == file)
    {
        CSmallString str = "failed to open file for appending: ";
        str << fileName;
        str << "; errno : ";
        str << errno;
        FPASSERTMSG(TRUE, str.GetString());
        return;
    }

    fwrite(buffer,  sizeof(char),  strlen(buffer), file);
    int errorStatus = ferror(file);
    fclose(file);
    
    if(0 != errorStatus)
    {
        CSmallString str = "failed to write to file : ";
        str << resetListFileName;
        str << "; error : ";
        str << errorStatus;
        FPASSERTMSG(TRUE, str.GetString());
        return;
    }
}


