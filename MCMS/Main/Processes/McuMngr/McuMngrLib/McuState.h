// McuState.h: interface for the CMcuState class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MCUSTATE_H_
#define MCUSTATE_H_


#include "PObject.h"
#include "psosxml.h"
#include "McmsProcesses.h"
#include "DefinesGeneral.h"
#include "McuMngrDefines.h"
#include "McuMngrStructs.h"
#include "ProductType.h"
#include "StructTm.h"

class CMcuMngrProcess;


using namespace std;



class CMcuState : public CPObject
{

CLASS_TYPE_1(CMcuState, CPObject)

public:
	CMcuState ();
	virtual const char* NameOf() const { return "CMcuState";}
	virtual ~CMcuState ();
	virtual void Dump(ostream& msg) const;
    void  SerializeXml(CXMLDOMElement* pFatherNode);
    int   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	CMcuState& operator = (const CMcuState &rOther);

	void						SetNumOfActiveAlarms(WORD activeAlarmsNum);
	WORD						GetNumOfActiveAlarms();

	void						ClearSerialNumber();
	void						SetMplSerialNumber(char* serialNum);
	char*						GetMplSerialNumber();


	void						SetMcuState(const string &callerName, eMcuState mcuState);
	eMcuState					GetMcuState()const;

	void						SetValidationState(eLicensingValidationState validationState);
	eLicensingValidationState	GetValidationState();
	
	void						IncreaseNumOfCoreDumps();
	void						SetNumOfCoreDumps(DWORD numOfCores);
	DWORD						GetNumOfCoreDumps();
	DWORD						GetActualNumberOfCoreDumps();
	
	void						SetMcuStateProductType(eProductType theType);
	eProductType				GetMcuStateProductType();

	void						SetMediaRecordingState(DWORD mediaRecording);
	DWORD						GetMediaRecordingState();

	void						SetCollectingInfoState(DWORD collectingInfo);
	DWORD						GetCollectingInfoState();
	
	void						SetBackupState(BYTE progress);
	eBackupProgressType			GetBackupState();
	
	void						SetRestoreState(BYTE progress);
	eRestoreProgressType		GetRestoreState();

    void                        SetIsSshOn(BOOL val);
    BOOL                        GetIsSshOn()const;
	
    void                        SetRemainingTimeForStartup(int val);
    int                         GetRemainingTimeForStartup()const;

	eSystemCardsMode	        GetMcuStateSystemCardsMode() const;
	void                        SetMcuStateSystemCardsMode(eSystemCardsMode theMode);
	
	void                        ClearPrimaryLicenseServer();
	void                        SetPrimaryLicenseServer(std::string primaryLicenseSrver);
	std::string                 GetPrimaryLicenseServer();

	void                        SetPrimaryLicenseServerPort(DWORD portNum);
	DWORD                       GetPrimaryLicenseServerPort();
	void                        SetExpirationDate(const CStructTm &other);
	CStructTm                   GetExpirationDate(void);
protected:
	CMcuMngrProcess*			m_pProcess;

	WORD						m_numOfActiveAlarms;
	DWORD						m_numOfCoreDumps;
	DWORD						m_mediaRecordingState;
	DWORD						m_collectingInfoState;
	eBackupProgressType			m_backupState;
	eRestoreProgressType		m_restoreState;
	char						m_mplSerialNumber[MPL_SERIAL_NUM_LEN];
	eLicensingValidationState	m_validationState;
	eProductType				m_mcuStateProductType;
	eSystemCardsMode            m_mcuStateSystemCardsMode;
    BOOL                        m_IsSshOn;
	int		                	m_remainingTimeForStartup;
	std::string                 m_primaryLicenseServer;
	DWORD                       m_primaryLicenseServerPort;
	CStructTm                   m_expirationDate;
};


#endif /*MCUSTATE_H_*/
