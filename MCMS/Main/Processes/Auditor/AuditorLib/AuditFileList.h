#ifndef __AUDIT_FILE_LIST_H__
#define __AUDIT_FILE_LIST_H__

#include "CyclicFileList.h"



// class CAuditorFileList : public CCyclicFile
// {
// public:
//     virtual const char* GetXmlTag()const{return "AUDIT_FILE"}
//     virtual const char* GetVisualFileName()const{return "Audit";}
// };


// CCyclicFile* CreateAuditorFile()
// {
//     return 
// }



class CAuditFileList : public CCyclicFileList
{
public:
    CAuditFileList();
    virtual ~CAuditFileList();

    virtual BOOL DeleteFilesIfNeeded();
    virtual BOOL TestForFileSystemWarning();
    
    
    virtual void  SerializeXml(CXMLDOMElement*& pFatherNode) const;

    void SetFileNameHeaders(const FileNameHeaders_S & fnh);
    void FixLastFile();
    
private:
    FileNameHeaders_S m_FileNameHeaders;
};




#endif // __AUDIT_FILE_LIST_H__
