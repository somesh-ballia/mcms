// ConnectionList.h: interface for the CConnectionList class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(_VirtualDirectory_H__)
#define _VirtualDirectory_H__


#include <string>
#include "McmsProcesses.h"


class CVirtualDirectory
{
	
public:

	CVirtualDirectory();
	~CVirtualDirectory();
	
private:
	int m_Access;
	char* m_pVirtualDirectory;
	char* m_pPhysicalDirectory;
	int m_CreateDir;
    bool m_isAuditable;
    eProcessType m_InformProcess;
    
	
public:
	void SetPhysicalPath(char* pszVirtual, const char* pszPhysicalPath);
	char* GetVirtualPath();
	char* GetPhysicalPath();
    
	void SetDirectoryAccess(const int access);
	int GetDirectoryAccess();
    
	void SetCreateDirectory(const int create_dir);
	int GetCreateDirectory();
    
    void SetAuditability(bool val);
    bool GetAuditability() const;
    
    void SetInformProcess( eProcessType processType);
    void SetInformProcess( const std::string & process_name);
    eProcessType GetInformProcess() const;
    

};

#endif // !defined(_VirtualDirectory_H__)

