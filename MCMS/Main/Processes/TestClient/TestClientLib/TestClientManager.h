// TestClientManager.h: interface for the CTestClientManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TestClientMANAGER_H__)
#define _TestClientMANAGER_H__

#include "ManagerTask.h"
#include "Macros.h"
#include "StatusesGeneral.h"

class CListenSocketApi;
class CClientSocket;

class CTestClientManager : public CManagerTask
{
CLASS_TYPE_1(CTestClientManager,CManagerTask )
public:
	CTestClientManager();
	virtual ~CTestClientManager();

	virtual BOOL TaskHandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	virtual void ManagerPostInitActionsPoint();

	void  SelfKill();
	void* GetMessageMap();

	TaskEntryPoint GetMonitorEntryPoint();

	void  OnTimerPingPong(CSegment* pMsg);

	CListenSocketApi * m_pListenSocketApi;
	CTaskApi         * m_pSingleToneApi;

    std::list< CTaskApi* > m_tasks_list;




	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

// TERMINAL COMMNADS
    //////////////////////////////////////////////////////////////////////
    STATUS ReadUnallocatedMemory(CTerminalCommand &command,std::ostream& answer);
    STATUS WriteUnallocatedMemory(CTerminalCommand &command,std::ostream& answer);
    STATUS ReadBeforeMemory(CTerminalCommand &command,std::ostream& answer);
    STATUS WriteBeforeMemory(CTerminalCommand &command,std::ostream& answer);
    STATUS ReadAfterMemory(CTerminalCommand &command,std::ostream& answer);
    STATUS WriteAfterMemory(CTerminalCommand &command,std::ostream& answer);
    STATUS DeallocateTwice(CTerminalCommand &command,std::ostream& answer);
    STATUS DeallocateNonAllocated(CTerminalCommand &command,std::ostream& answer);
    STATUS DeallocateMiddle(CTerminalCommand &command,std::ostream& answer);
    STATUS DeallocateArrayAsObject(CTerminalCommand &command,std::ostream& answer);
    STATUS DeallocateObjectAsArray(CTerminalCommand &command,std::ostream& answer);
    STATUS AllocateZero(CTerminalCommand &command,std::ostream& answer);
    STATUS ReadUninitMemoryHeap(CTerminalCommand &command,std::ostream& answer);
    STATUS ReadUninitMemoryStack(CTerminalCommand &command,std::ostream& answer);
    STATUS BufferOverflow(CTerminalCommand &command,std::ostream& answer);
    STATUS BufferUnderflow(CTerminalCommand &command,std::ostream& answer);
    STATUS MemoryLeakNew(CTerminalCommand &command,std::ostream& answer);
    STATUS MemoryLeakMalloc(CTerminalCommand &command,std::ostream& answer);
    STATUS StackOverFlow(CTerminalCommand &command,std::ostream& answer);
    STATUS CyclicMemory(CTerminalCommand &command,std::ostream& answer);
    STATUS CreateTasks(CTerminalCommand &command,std::ostream& answer);
    STATUS DestroyTasks(CTerminalCommand &command,std::ostream& answer);

};

#endif // !defined(_TestClientMANAGER_H__)

