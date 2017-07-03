/*
 * FailDetectionTask.h
 *
 *  Created on: Aug 25, 2009
 *      Author: yael
 */

#ifndef FAILDETECTIONTASK_H_
#define FAILDETECTIONTASK_H_


#include "TaskApp.h"
#include "ManagerApi.h"

class CFailoverProcess;

extern "C" void FailDetectionEntryPoint(void* appParam);



class CFailDetectionTask : public CTaskApp
{
CLASS_TYPE_1(CFailDetectionTask, CTaskApp)


public:

// Constructors
  CFailDetectionTask();
 ~CFailDetectionTask();

 // Initializations
  void  InitTask();
  BOOL  IsSingleton() const {return NO;}
  //void Create(CSegment& appParam);
  //void Destroy();
  //virtual void  SelfKill();

  // Operations
  virtual const char*  NameOf() const;
  virtual const char * GetTaskName() const;


private:

	void OnTimerMasterKeepAliveSendTimeout(CSegment* pParam);
	void OnTimerMasterKeepAliveFailureTimeout(CSegment* pParam);
	void OnSocketRcvInd(CSegment* pMsg);
	void OnFailureResumeSlaveTask(CSegment* pMsg);

	void InitMcuStateTransStr();
	void SendMcuStateTrans();
	void SendMasterDownIndToManager();

	void StartFailureDetection();


protected:
	CFailoverProcess *m_pProcess;

	bool	m_isKeepAliveResponseReceived;
	bool	m_isMasterDownSentToManager; //for network problems in slave - don't send this message again and again

	char	*m_mcuStateTransStr;
	WORD    m_keepAliveFailureTimeout;


	PDECLAR_MESSAGE_MAP
};


#endif /* FAILDETECTIONTASK_H_ */
