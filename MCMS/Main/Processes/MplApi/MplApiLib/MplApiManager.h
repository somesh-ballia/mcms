// MplApiManager.h

#ifndef MPL_API_MANAGER_H_
#define MPL_API_MANAGER_H_

#include "ManagerTask.h"
#include "PairOfSockets.h"
#include "Macros.h"
#include "IpParameters.h"

class CListenSocketApi;

class CMplApiManager : public CManagerTask  
{
CLASS_TYPE_1(CMplApiManager, CManagerTask)
public:
	CMplApiManager();
	virtual ~CMplApiManager();

	virtual const char* NameOf() const { return "CMplApiManager";}

	TaskEntryPoint GetMonitorEntryPoint();

	void SelfKill();

	void ManagerPostInitActionsPoint();

	void OnMplApiMsg(CSegment* pSeg);
	void OnMplApiOpenCardConnection(CSegment* pMsg);
	void OnMplApiCloseCardConnection(CSegment* pMsg);
	void OnCtrlIpConfigInd(CSegment* pSeg);
	void OnStartUpgradeInd(CSegment* pSeg);
	void OnSetLogLevelInd(CSegment* msg);
	void OnNackLogLevelOutOfRange(CSegment* msg);
// 1080_60
	void OnSystemCardsModeInd(CSegment* pMsg);

	void CreateDispatcher();

	CListenSocketApi *m_pListenSocketApi;

    WORD   m_Mpl_Api_Card2SocketTable[MAX_NUM_OF_BOARDS];
	void*  GetMessageMap();

	STATUS InitNetworkInterface();
	STATUS UpdateNetworkInterface();
	STATUS ConfigNetworkInterfaceInOS();

  virtual int GetTaskMbxBufferSize() const {return 4096 * 1024 - 1;}
	
protected:
	CIpParameters      m_controlIpParams;
	CIPService*        m_pCntrlIpParams_asIpService;
	
	BOOL m_isSystemTarget;	// Target or Pizza
	
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS
		
private:
	virtual void DeclareStartupConditions();
	virtual void ManagerStartupActionsPoint();
	virtual void AddFilterOpcodePoint();
	
	STATUS HandleTerminalConn(CTerminalCommand &command, std::ostream& answer);
	STATUS HandleTerminalLockCard(CTerminalCommand &command, std::ostream& answer);
	STATUS HandleTerminalSetSignal(CTerminalCommand &command, std::ostream& answer);
	STATUS HandleTerminalOpcodeOff(CTerminalCommand &command, std::ostream& answer);	
	STATUS HandleTerminalOpcodeOn(CTerminalCommand &command, std::ostream& answer);
	void PrintIgnoredOpcodesList(std::ostream& outputStream);
	STATUS HandleTerminalDisconn(CTerminalCommand &command, std::ostream& answer);
};

#endif  // MPL_API_MANAGER_H_
