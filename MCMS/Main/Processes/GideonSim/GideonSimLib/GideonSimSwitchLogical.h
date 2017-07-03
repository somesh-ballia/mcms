// GideonSimLogicalModule.h

#if !defined(__GIDEONSIM_SWITCH_LOGICAL_)
#define __GIDEONSIM_SWITCH_LOGICAL_

class CStateMachine;
class CTaskApi;
class CMplMcmsProtocol;
class CSimMfaUnitsList;
class CIcComponent;
class CBarakIcComponent;
class CTbComponent;
class CSimSwitchUnitsList;

#include "IpCmInd.h"
#include "GideonSimParties.h"
#include "GideonSimLogicalUnit.h"

#define IVR_SIM_FOLDER_MAIN			"Cfg/IVR/"
#define IVR_SIM_FOLDER_ROLLCALL		"IVRX/RollCall/"
#define IVR_SIM_ROLLCALL_FILE		"/SimRollCall.wav"
#define MAX_IC_IVR_PLAY_MSGS		50

class CGideonSimSwitchLogical : public CGideonSimLogicalModule
{
CLASS_TYPE_1(CGideonSimSwitchLogical,CGideonSimLogicalModule)
public:
	CGideonSimSwitchLogical(CTaskApp* pTask,WORD boardId,WORD subBoardId);
	virtual ~CGideonSimSwitchLogical();
	const char * NameOf() const {return "CGideonSimSwitchLogical";}
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	void SetUnitStatustSwitchForKeepAliveInd(WORD UnitNum, STATUS status );
	void PrintSmFatalFailureToTrace(SWITCH_SM_KEEP_ALIVE_S fatalFailureStruct);
  void PrintSmSpecificCompFailure(SM_COMPONENT_STATUS_S compFailureStruct, char* resStr);
  void SetUnitStatusSwitchForMFARemoveInd(BYTE boardId, BYTE subBoardId);
  void SetUnitStatusSwitchForMFAInsertInd(BYTE boardId, BYTE subBoardId);
  void SendUserLdapLogin(const string& userName, const string& userPassword);
  
protected:
	void OnCmStartupIdle(CSegment* pMsg);
	void OnCmProcessMcmsReqStartup(CSegment* pMsg);
	void OnCmProcessMcmsReqConnect(CSegment* pMsg);
	void OnTimerCardsDelayTout(CSegment* pMsg);
	void OnTimerSwitchConfigTout(CSegment* pMsg);
	void OnTimerSoftwareUpgrade(CSegment* pMsg);
	void OnTimerIpmcSoftwareUpgrade(CSegment* pMsg);

	
	void EstablishConnectionInd();
	void ReestablishConnectionInd();
	void AuthentificationInd() const;
	void CntlIpConfigInd() const;
	void MngmntIpConfigInd() ;
	void SwLocationInd() const;
	void CardManagerLoadedInd();
	void SlotsNumberingConversion() const;
	void FillAmosSlotInConversionTable( SLOTS_NUMBERING_CONVERSION_TABLE_S *rStruct ) const;
	void FillSlotInConversionTable(SLOTS_NUMBERING_CONVERSION_TABLE_S *rStruct, DWORD ind, APIU32 boardId, APIU32 subBoardId, APIU32 displayBoardId) const;
  void Ack_keep_alive_Switch( CMplMcmsProtocol& rMplProt, STATUS status ) const;
  char * ConvertCardTypeToShelfMngrString(DWORD cardType) const;
  char * ConvertRtmCardTypeToShelfMngrString(DWORD RtmCardType) const;
	void SendStartupInds();
	STATUS ReadAutenticationStringsFromFile(char* pszLine1,char* pszLine2) const;
	STATUS WriteAutenticationStringsToFile(const char* pszLine1,const char* pszLine2) const;
	STATUS UpdateLicenseFile(CMplMcmsProtocol& rMplProtocol) const;
	STATUS CreateApiMessageFromServiceFile(IP_PARAMS_S& rStructIp);
	void   CreateApiMessageDefault(IP_PARAMS_S& rStructIp);
	CSimSwitchUnitsList* m_units;
	BOOL m_bCMLoadedInd;
	BOOL m_bSlotNumberingConversionInd;


	PDECLAR_MESSAGE_MAP;
};

#endif // !defined(__GIDEONSIM_SWITCH_LOGICAL_)
