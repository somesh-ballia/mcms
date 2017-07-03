// AuditorManager.h: interface for the CAuditorManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__AUDITOR_MANAGER_H__)
#define __AUDITOR_MANAGER_H__

#include "ManagerTask.h"
#include "Macros.h"



class CListenSocketApi;
class COsQueue;
class CAuditFileManager;
class CAuditorProcess;


void AuditorManagerEntryPoint(void* appParam);





class CAuditorManager : public CManagerTask
{
CLASS_TYPE_1(CAuditorManager,CManagerTask )
public:
	CAuditorManager();
	virtual ~CAuditorManager();

	virtual const char * NameOf() const {return "CAuditorManager";}
	virtual void ManagerPostInitActionsPoint();
    virtual void SelfKill();

    virtual void InformHttpGetFile(const std::string & file_name);
 
    
	TaskEntryPoint GetMonitorEntryPoint();

    void HandleAuditEventMcms(CSegment *pSeg);
    void HandleAuditEventOutsider(CSegment *pSeg);
    void HandleCloseConnection(CSegment *pSeg);
    void HandleOpenConnection(CSegment *pSeg);
    void HandleFlushBufferRequest(CSegment *pSeg);
    void HandleMidNightTimer(CSegment *pSeg);
    void OnTimerFileSystemWarningTest();
    
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

private:
    STATUS HandleTerminalDumpEvents(CTerminalCommand &command, std::ostream& answer);
    STATUS HandleTerminalDumpFiles(CTerminalCommand &command, std::ostream& answer);
    STATUS HandleTerminalFlush(CTerminalCommand &command, std::ostream& answer);
    STATUS HandleTerminalNext(CTerminalCommand &command, std::ostream& answer);


    void OnAuditIncomingEvent(CSegment *pSeg);
    void OpenSocketInterface();
    void StopFileSystem();
    DWORD GetTickNumUntilMidNight()const;


    CListenSocketApi   *m_pListenSocketApi;
    COsQueue           *m_pRcvMbx;
    CAuditorProcess    *pProcess;
    CAuditFileManager   *m_pAuditFileManager;
    
    BOOL                    m_fileSystemWarning;

};

#endif // !defined(__AUDITOR_MANAGER_H__)
