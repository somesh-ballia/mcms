// FaultsManager.h: interface for the CFaultsManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_FaultsMANAGER_H__)
#define _FaultsMANAGER_H__


#include "Macros.h"
#include "ManagerTask.h"
#include "HlogList.h"


void FaultsManagerEntryPoint(void* appParam);


class CFaultsManager : public CManagerTask
{
CLASS_TYPE_1(CFaultsManager,CManagerTask )
public:
	CFaultsManager();
	virtual ~CFaultsManager();


	BOOL         TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode);
	TaskEntryPoint GetMonitorEntryPoint();

protected:
	void HandleHlogMessage(CSegment *pMsg,const DWORD msgLen,const BOOL isFullHlogOnly);
	void HlogsLogger(CHlogElement* pHlogElement,const BOOL isFullHlogOnly);

	STATUS HandleTerminalDumpAllFaultsToFile(CTerminalCommand & command, std::ostream& answer);
		// Attributes

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

private:
	virtual void ManagerPostInitActionsPoint();
	void DumpAllFaultsToFile(CSegment* pSeg);
	STATUS WriteAllFaultsToFile(const std::string& outputFaultFileName) const;
};

#endif // !defined(_FaultsMANAGER_H__)

