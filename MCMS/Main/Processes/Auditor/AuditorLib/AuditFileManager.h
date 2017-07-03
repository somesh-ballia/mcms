#ifndef __AUDIT_FILE_MANAGER_H__
#define __AUDIT_FILE_MANAGER_H__

#include <string>
using namespace std;

#include "PObject.h"
#include "StructTm.h"


class CCyclicFileManager;
class CAuditFileList;
class CAuditHdrWrapper;
class CObjString;
class CCyclicFile;



class CAuditFileManager : public CPObject
{
CLASS_TYPE_1(CAuditFileManager, CPObject)    
public:
    CAuditFileManager();
    virtual ~CAuditFileManager();
    
    virtual const char*  NameOf() const{return "CAuditFileManager";}

    bool Init();
    void Shutdown();
    
    STATUS HandleEvent(CAuditHdrWrapper & auditWrapper);
    STATUS Flush();
    STATUS MoveToNextFile();
    
    void DumpFileList(std::ostream & answer);
    void InformAuditFileRetrieved(const std::string name);
    BOOL TestForFileSystemWarning();
    
private:
    // disabled
    CAuditFileManager(const CAuditFileManager&);
    CAuditFileManager operator=(const CAuditFileManager&);

    bool IsFileComplete(const CStructTm & firstEventTime,
                        const CStructTm & lastEventTime,
                        std::ostream & outDesc);
    void FormatAuditEvent(string & outBuffer,
                          const CAuditHdrWrapper & auditWrapper);

    void InsertXmlAuditListTags();
    STATUS InsertXmlRootBeginElement();
    STATUS InsertXmlRootEndElement();
    
    STATUS AppendToCurrentFile(const BYTE *data,     // data to file
                               DWORD dataLen);       // quantity of data
    void InitEventSequenceNumber();
    DWORD ExtractBigestSequenceNumberFromFile(const CCyclicFile & currentFile);
    
    
    CCyclicFileManager *m_pCyclicFileManager;
    
    CStructTm m_FirstMessageTime;
    CStructTm m_LastMessageTime;

    bool m_IsInit;
    bool m_IsFirstEvent;
    bool m_startUpFlag;
    DWORD m_SequenceNumber;
};




#endif // __AUDIT_FILE_MANAGER_H__
