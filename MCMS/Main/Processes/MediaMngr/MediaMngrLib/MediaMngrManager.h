// MediaMngrManager.h: interface for the CMediaMngrManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DEMOMANAGER_H__)
#define _DEMOMANAGER_H__

#include "ManagerTask.h"
#include "ConnectionsTaskApi.h"
#include "MediaMngrCfg.h"
#include "MediaRepository.h"



// task creation function
void MediaMngrManagerEntryPoint(void* appParam);



class CMediaMngrManager : public CManagerTask
{
CLASS_TYPE_1(CMediaMngrManager,CManagerTask )
public:
	CMediaMngrManager();
	virtual ~CMediaMngrManager();

	const char * NameOf() const {return "CMediaMngrManager";}
	void ManagerPostInitActionsPoint();
	void SelfKill();

	TaskEntryPoint GetMonitorEntryPoint();
	
	
	//configuration of media manager
	CMediaMngrCfg*  m_pMediaMngrConfig;
	
	//Media Repository
	CMediaRepository* m_pMediaRepository;
	
	//connections list api
	CConnectionsTaskApi* m_pConnectionsTaskApi;
	
protected:
	
				// Action functions
	void OnGideonSimMsgAnystate(CSegment* pParam);
	
	
	STATUS HandleSetEPMediaParam(CRequest* pRequest);
	STATUS HandleResetChannelOut(CRequest* pRequest);
	STATUS HandleUpdateMediaLibrary(CRequest* pRequest);
	
	
	// terminal commands
	STATUS HandleTerminalMMState(CTerminalCommand & command, std::ostream& answer);
	//STATUS HandleTerminalMMConnections(CTerminalCommand & command, std::ostream& answer);
	
	STATUS HandleTerminalMMRecording(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleTerminalMMCheckSeqNum(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleTerminalMMDetectIntra(CTerminalCommand & command, std::ostream& answer);
	
	STATUS HandleTerminalMMResetChannel(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleTerminalMMVideoUpdatePic(CTerminalCommand & command, std::ostream& answer);

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

};

#endif // !defined(_DEMOMANAGER_H__)
