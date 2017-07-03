// CardsDispatcherTask.h: interface for the CCardsDispatcherTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CARDS_DISPATCHERTASK_H__)
#define _CARDS_DISPATCHERTASK_H__


#include "DispatcherTask.h"
#include "CardsDefines.h"

class CCardsProcess;



class CCardsDispatcherTask : public CDispatcherTask
{
	CLASS_TYPE_1(CCardsDispatcherTask, CDispatcherTask)
public:
	
	CCardsDispatcherTask();
	virtual ~CCardsDispatcherTask();
	virtual const char* NameOf() const { return "CCardsDispatcherTask";}


	void InitTask(){;}
	BOOL TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode);
	void HandleMPLEvent(CSegment *pMsg);
	
	void SendRestartAuthenticationProcedureReqToCardsMngr(BYTE switchBoardId);
	void SendMfaNotCreatedToMplApi(BYTE boardId, BYTE subBoardId);
	void SendCardsNotReadyToMplApi(BYTE boardId, BYTE subBoardId);
	
	void SetIsAssertForThisMfaTask(BOOL isAssert, DWORD boardId, DWORD subBoardId);
	BOOL GetIsAssertForThisMfaTask(DWORD boardId, DWORD subBoardId);


protected:
	CCardsProcess*  m_pProcess;

	BOOL  m_isAssertForThisMfaTask[MAX_NUM_OF_BOARDS][MAX_NUM_OF_SUBBOARDS];

				// Action functions
};

#endif // _CARDS_DISPATCHERTASK_H__
