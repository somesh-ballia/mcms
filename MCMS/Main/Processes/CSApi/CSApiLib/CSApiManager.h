// CSApiManager.h: interface for the CCSApiManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CSApiMANAGER_H__)
#define _CSApiMANAGER_H__

#include "ManagerTask.h"
#include "Macros.h"

class CListenSocketApi;
class CCSApiProcess;

class CCSApiManager : public CManagerTask
{
CLASS_TYPE_1(CCSApiManager,CManagerTask )
public:
	CCSApiManager();
	virtual ~CCSApiManager();

	void ManagerPostInitActionsPoint();
	const char * NameOf(void) const {return "CCSApiManager";}
	TaskEntryPoint GetMonitorEntryPoint();
	void SelfKill();
	void CreateDispatcher();
	void OnCSApiCloseCardConnection(CSegment* pMsg);
	void OnCSApiOpenCardConnection(CSegment* pMsg);

	void*  GetMessageMap();

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

private:
	STATUS HandleTerminalConn(CTerminalCommand &command, std::ostream& answer);


	CListenSocketApi *m_pListenSocketApi;
	CCSApiProcess *m_pCsApiProcess;
};

#endif // !defined(_CSApiMANAGER_H__)

