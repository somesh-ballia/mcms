// SingleToneApi.h: interface for the CSingleToneApi class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SINGLETONEAPI_H__)
#define _SINGLETONEAPI_H__


#include "TaskApi.h"


class CSingleToneApi : public CTaskApi  
{
CLASS_TYPE_1(CSingleToneApi,CTaskApi)
public:
	CSingleToneApi(const eProcessType processType,
                   const char * taskName,
                   COsQueue * pQueue = NULL);
	virtual const char* NameOf() const { return "CSingleToneApi";}
	virtual ~CSingleToneApi();
	void CleanUp();
	
private:
	void  CreateOnlyApi(const COsQueue& rcvMbx,
						CStateMachine* pStateMachine = NULL,
						LocalQueue* pLocalRcvMbx = NULL,
						WORD syncCall = 0){;}
	void  SetLocalMbx(LocalQueue* localRcvMbx) {;}
	void  DestroyOnlyApi() {;}
	void  Create(const COsQueue& creatorRcvMbx,WORD syncCall = 0) {;}
	CTaskApp* GetTaskAppPtr(){return NULL;}
	const LocalQueue& GetLocalRcvMbx() const{return dummy;}

    LocalQueue dummy;

};

#endif // !defined(_SINGLETONEAPI_H__)
