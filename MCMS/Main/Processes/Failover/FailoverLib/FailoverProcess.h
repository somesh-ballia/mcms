// FailoverProcess.h: interface for the CFailoverProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DEMOPROCESS_H__)
#define _DEMOPROCESS_H__

#include "ProcessBase.h"
#include "McuMngrInternalStructs.h"

class CFailoverConfiguration;
class CFailoverCommunication;



class CFailoverProcess : public CProcessBase
{
CLASS_TYPE_1(CFailoverProcess,CProcessBase )
public:
	friend class CTestFailoverProcess;

	CFailoverProcess();
	virtual ~CFailoverProcess();
	const char * NameOf() const {return "CFailoverProcess";}
	virtual eProcessType GetProcessType() {return eProcessFailover;}
	virtual int GetProcessAddressSpace() {return 80 * 1024 * 1024;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();
	virtual void AddExtraStringsToMap();
	CFailoverConfiguration* GetFailoverConfiguration() const;
	void SetFailoverConfiguration(CFailoverConfiguration * pFailoverData ){m_pFailoverConfiguration=pFailoverData;}
	DWORD	GetMcuToken() const;
	void	SetMcuToken(DWORD mcuToken);

	DWORD	GetAndIncreaseMsgId();

	CFailoverCommunication*	GetFailoverCommunication() const;
	void					SetFailoverCommunication(CFailoverCommunication* pComm);

	MASTER_SLAVE_DATA_S*	GetMngmntInfo() const;
	void					SetMngmntInfo(MASTER_SLAVE_DATA_S* pData);
	void 					UpdateMngmntInfo(MASTER_SLAVE_DATA_S* pData);

	COsQueue*	GetFailDetectionTaskMbx() const;
	void		SetFailDetectionTaskMbx(COsQueue* pMbx);

	COsQueue*	GetFailoverSyncTaskMbx() const;
	void		SetFailoverSyncTaskMbx(COsQueue* pMbx);

	BYTE	    GetRequestPeerCertificate() const
	{
		return m_isRequestPeerCertificate;
	}
	void		SetRequestPeerCertificate(BYTE bisRequestPeerCertificate)
	{
		m_isRequestPeerCertificate = bisRequestPeerCertificate;
	}


  virtual DWORD GetMaxTimeForIdle(void) const
  {
    return 12000;
  }

private:

	CFailoverConfiguration	*m_pFailoverConfiguration;
	CFailoverCommunication	*m_pFailoverCommunication;
	MASTER_SLAVE_DATA_S 	*m_pMngmntInfo;

	COsQueue				*m_pFailDetectionTaskMbx;
	COsQueue				*m_pFailoverSyncTaskMbx;

	DWORD					m_mcuToken;
	DWORD					m_msgId;
	BYTE 					m_isRequestPeerCertificate;
};

#endif // !defined(_DEMOPROCESS_H__)
