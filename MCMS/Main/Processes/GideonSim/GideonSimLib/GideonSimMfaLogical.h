// GideonSimLogicalModule

#if !defined(__GIDEONSIM_MFA_LOGICAL_)
#define __GIDEONSIM_MFA_LOGICAL_

#include "GideonSimLogicalModule.h"
#include "GideonSimLogicalParams.h"

class CIcComponent;
class CTbComponent;

class CGideonSimMfaLogical : public CGideonSimLogicalModule
{
CLASS_TYPE_1(CGideonSimMfaLogical,CGideonSimLogicalModule)
public:
	CGideonSimMfaLogical(CTaskApp* pTask,WORD boardId,WORD subBoardId);
	virtual ~CGideonSimMfaLogical();
	virtual const char* NameOf() const {return "CGideonSimMfaLogical";}
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	void ProcessEndpointsSimMsg(CSegment* pMsg);
  void SetUnitStatustSimMfaForKeepAliveInd(WORD UnitNum, STATUS status );
  void EstablishConnectionInd();
  void ReestablishConnectionInd();
	void SetBndLclAlignmentTimer( DWORD bndRmtLclAlignment );
	BOOL IsValidParty(const CMplMcmsProtocol& rMplProt);// const;
	int IsConfPlayMessage( const CMplMcmsProtocol& rMpl );

protected:
	void CardManagerLoadedInd() const;
	void CmUnitLoadedInd() const;
	void CmMediaIpConfigInd();
	void CmMediaIpConfigCompletedInd();
	void SendActiveSpeakerInd(const CAudioParty& party) const;
	void SendAudioSpeakerInd(const CAudioParty& party) const;
	void Ack_keep_alive(CMplMcmsProtocol& rMplProt,STATUS status);
	void SendSlaveEncoderOpenInd( CMplMcmsProtocol& rMplProt ) const;
	void FillRtpMonitoringStruct(TCmPartyMonitoringInd* pStruct) const;
	void FillRtpStatisticInfoStruct(TCmPartyInfoStatisticsInd* pSt) const; //CDR_MCCF
	CAudioParty* FindParty(const CMplMcmsProtocol& rMpl);
	CAudioParty* FindParty(const CAudioParty& other);
	void ForwardAudioMsg2Endpoints(const DWORD opcode, const CAudioParty* party,
								   BYTE* pData = NULL, const DWORD nDataLen = 0) const;
	void ForwardMuxBndReq2Endpoints(CMplMcmsProtocol& rMplProt) const;
	void ForwardMsg2MediaMngr( CMplMcmsProtocol& rMplProt ) const;

	void OnCmStartupIdle(CSegment* pMsg);
  void OnCmStartupConnect(CSegment* pMsg);
	void OnCmProcessMcmsReqStartup(CSegment* pMsg);
	void OnCmProcessMcmsReqConnect(CSegment* pMsg);
	void OnTimerUnitConfigTout(CSegment* pMsg);
	void OnTimerMediaConfigTout(CSegment* pMsg);
	void OnTimerMediaConfigCompletedTout(CSegment* pMsg);
	void OnTimerCardsDelayTout(CSegment* pMsg);
	void OnTimerSpeakerChangeTout(CSegment* pMsg);
	void OnTimerIsdnCfg(CSegment* pMsg);
	void OnTimerSoftwareUpgrade(CSegment* pMsg);
	void OnTimerIpmcSoftwareUpgrade(CSegment* pMsg);
	void OnCmLoaded_test();
	void OnCmProcessEpSimMsgConnect(CSegment* pMsg);
  int  UpdateBondingParameters(DWORD unit, DWORD port, DWORD muxConnectionID, DWORD confId, DWORD partyId, DWORD numOfChnls);

protected:
	CAudioParty			m_raAudioPartiesArr[MAX_AUDIO_PARTIES];
	int					m_nSpeakerIndex;
	CSimMfaUnitsList*	m_units;
	CIcComponent*		m_IC;
	CTbComponent*		m_TB;
	DWORD				m_cardType;
	BOOL				m_IsAudioControllerMaster;

///TENP
	DWORD	m_bndAddChannelCounter;
///TEMP
	int m_software_upgrade_done;
	int m_ipmc_software_upgrade_done;

	PDECLAR_MESSAGE_MAP;
};

#endif // !defined(__GIDEONSIM_MFA_LOGICAL_)
