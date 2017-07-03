
#include <errno.h>

#include "AuditFileList.h"
#include "AuditorDefines.h"
#include "psosxml.h"
#include "OsFileIF.h"
#include "ObjString.h"
#include "HlogApi.h"


//////////////////////////////////////////////////////////////////////
CAuditFileList::CAuditFileList()
        :CCyclicFileList(AUDIT_DIRECTORY)
{
}

//////////////////////////////////////////////////////////////////////
CAuditFileList::~CAuditFileList()
{
}

//////////////////////////////////////////////////////////////////////
void CAuditFileList::FixLastFile()
{
    CCyclicFile & refCurrentFile = Back();
    const char *oldFileName =  refCurrentFile.GetFileName();
    const char *directoryName = GetCurrentDirrectory();

    string oldFullName = directoryName;
    oldFullName += oldFileName;

    int size = GetFileSize(oldFullName);
    if(-1 == size)
    {
        CLargeString message = "FAILED to get file size\n";
        message << oldFullName;
        
        PASSERTMSG(TRUE, message.GetString());
        return;
    }
    
    refCurrentFile.SetSize(size);

    CStructTm now;
    SystemGetTime(now);
    refCurrentFile.SetLastMessageTime(now);

    char newFileName[CYCLIC_FILE_MAX_NAME_LEN];
	CCyclicFileList::CreateFileName(newFileName,
                                    m_FileNameHeaders,
                                    refCurrentFile.GetSequenceNumber(),
                                    refCurrentFile.GetSize(),
                                    refCurrentFile.GetFirstMessageTime(),
                                    refCurrentFile.GetLastMessageTime(),
                                    refCurrentFile.IsContainStartup(),
                                    refCurrentFile.IsRetrieved(),
                                    refCurrentFile.GetCmprFormat());
    
    string newFullName = directoryName;
    newFullName += newFileName;

    BOOL renameRes = RenameFile(oldFullName, newFullName);
    int errnoCode = errno;
    if(renameRes)
    {
        refCurrentFile.SetFileName(newFileName);
    }
    else
    {
        CLargeString message = "FAILED to rename audit file\n";
        message << oldFullName
                << " -> "
                << newFullName
                << "\n"
                << "errno : "
                << errnoCode;
        
        PASSERTMSG(TRUE, message.GetString());

        // returns file params to the origins
        refCurrentFile.SetSize(0);
        refCurrentFile.SetLastMessageTime(refCurrentFile.GetFirstMessageTime());
    }
}

//////////////////////////////////////////////////////////////////////
BOOL CAuditFileList::DeleteFilesIfNeeded()
{
    if(0 == GetNumOfFiles())
    {
        return TRUE;
    }
    
    DELETE_FILE_REPORT_VECTOR deletedFile;
    BOOL res = DeleteOldFilesCyclic(MAX_NUM_AUDIT_FILES, deletedFile);
    
    return res;
}

///////////////////////////////////////////////////////
void CAuditFileList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pFileListNode = pFatherNode->AddChildNode(AUDIT_FILE_LIST_XML_TAG);
	CCyclicFileList::SerializeXml(pFileListNode);    
}

///////////////////////////////////////////////////////
void CAuditFileList::SetFileNameHeaders(const FileNameHeaders_S & fnh)
{
    memcpy(&m_FileNameHeaders, &fnh, sizeof(FileNameHeaders_S));
}


///////////////////////////////////////////////////////
BOOL CAuditFileList::TestForFileSystemWarning()
{
    int skip = 0;
    CYCLIC_FILE_LIST::iterator iTer = m_fileList.end();

    do
    {
        skip++;
        iTer--;
        if (skip > MAX_NUM_AUDIT_FILES / 2)
         // the newest audot files (half of max size of the list)
         // will never make such alert   
        {
            
            if (iTer->IsRetrieved() == FALSE)
            {
                return TRUE;
                // one of the audit files (in the older half)
                // need to be backuped
            }
            
        }
        
    } while (iTer != m_fileList.begin());

    
     return FALSE;
    
}
