#ifndef _ISDNPARTYCNTL
#define _ISDNPARTYCNTL

#include "PartyCntl.h"
#include "IsdnNetSetup.h"
#include "H320Caps.h"
#include "H320ComMode.h"


////////////////////////////////////////////////////////////////////////////
//                        CIsdnPartyCntl
////////////////////////////////////////////////////////////////////////////
class CIsdnPartyCntl : public CPartyCntl
{
	CLASS_TYPE_1(CIsdnPartyCntl, CPartyCntl)

public:
	                        CIsdnPartyCntl();
	virtual                ~CIsdnPartyCntl();

	CIsdnPartyCntl&         operator=(const CIsdnPartyCntl& other);

	virtual const char*     NameOf() const { return "CIsdnPartyCntl"; }
	virtual CNetSetup*      GetNetSetUp()  { return &m_netSetUp; }
	virtual void            Destroy();
	void                    SetDataForImportVoicePartyCntl(CIsdnPartyCntl& otherPArtyCntl);
	void                    OnCAMUpdatePartyInConf(CSegment* pParam);
	void                    OnAudBrdgDisconnect(CSegment* pParam);
	void                    OnPartyMuteAudioAnycase(CSegment* pParam);
	virtual BYTE            IsRemoteAndLocalCapSetHasContent(eToPrint toPrint = eToPrintNo) const;
	virtual BYTE            IsLegacyContentParty();
	virtual eVideoPartyType GetVideoPartyTypeAccordingToCapabilities(CCapH320* pCap, bool isLocalCaps = false);
	bool                    IsCpH263_4Cif_SupportedByRateAndVideoQuality();
	virtual void            HandleReallocateResponse(BYTE bAllocationFailed, BYTE bLocalCapsChanged);
	BYTE                    ReAllocatePartyResourcesIfNeeded();
	void                    OnRsrcReAllocatePartyRspReAllocate(CSegment* pParam);
	void                    OnAllocateMixResourcesResponse(CSegment* pParam);
	BYTE                    IsReallocateResponseValid(CPartyRsrcDesc* pTempPartyAllocatedRsrc);
	BYTE                    UpdateH264ModeInLocalCaps();
	void                    UpdateCurrComMode(CComMode* pScm);
	void                    RemoveH2634CifFromTargetScm();
	void                    DispatchChangeModeEvent();
	BYTE                    IsPartyH239() const;
	void                    OnRmtXmitMode(CSegment* pParam);
	void                    OnPartyDeallocateBondingTmpPhoneNumber(CSegment* pParam);
	BYTE                    DisconnectPartyFromVideoOut();
	BYTE                    DisconnectPartyFromVideoIn();
	BYTE                    DisconnectPartyFromVideoInAndOut();
	CComMode*               GetCurrentReceiveScm()               { return m_pCurrentReceiveScm; }
	CComMode*               GetCurrentTransmitScm()              { return m_pCurrentTransmitScm; }
	CComMode*               GetTargetReceiveScm()                { return m_pTargetReceiveScm; }
	CComMode*               GetTargetTransmitScm()               { return m_pTargetTransmitScm; }

	virtual void            UpdateBridgeFlowControlRateIfNeeded(){ /*do nothing*/}
	void                    OnSmartRecovery(CSegment* pParam);
	BYTE                    IsRemoteContentFailed() const;

	void                    DisableHD720AsymmetricFromTargetScm();
	void                    DisableHD1080AsymmetricFromTargetScm();
	void                    DisableHD720At60AsymmetricFromTargetScm();
	void                    DisableHD1080At60AsymmetricFromTargetScm();
	virtual BYTE            IsRemoteAndLocalHasHDContent1080() const {return NO;}
	virtual BYTE            IsRemoteAndLocalHasHDContent720() const  {return NO;}
	//HP content:
	virtual BYTE            IsRemoteAndLocalHasHighProfileContent() const { return NO; }
	virtual DWORD           GetPossibleContentRate() const { return 0; }

	virtual void            InitVideoParams(CComMode* pScm, CBridgePartyVideoParams* pMediaParams);

	virtual BYTE            GetPartyCascadeType() const;
	virtual void            UpdateAudioDelay(CBridgePartyVideoParams* pMediaParams);

	virtual void            AvcToSvcArtTranslatorConnected(STATUS status){};

	void                    OnSetPartyAvcSvcMediaState(CSegment* pParam);

protected:
	void                    RedialIfNeeded();
	bool                    IsNetworkProblemsDisconnection() const;
	BYTE                    UpdateH264BaseProfileTresholdInLocalCaps();
	bool                    StartAvcToSvcArtTranslator();
	void                    DisconnectAvcToSvcArtTranslator();
	bool                    IsAllPartyRsrcForTranslatorExist();
	void                    FillTranslatorRsrcParamsAndStartCreateTranslator();


	bool                    ReAllocateRtmRequest;
	CIsdnNetSetup           m_netSetUp;
	ENetworkType            m_networkType;

	CCapH320*               m_pLocalCap;
	CCapH320*               m_pRemoteCap;
	CComMode*               m_pCurrentReceiveScm;  // EP --> MCU
	CComMode*               m_pCurrentTransmitScm; // MCU --> EP
	CComMode*               m_pTargetReceiveScm;   // the target scm we would like to receive from EP after change mode ends
	CComMode*               m_pTargetTransmitScm;  // the target scm we would like to transmit to EP after change mode ends
	CComMode*               m_pConfScm;            // the initial party scm according to conf settings
	                                               // (The first scm we get from conf after SetPartyCapsAndVideoParams)
	BYTE                    m_presentationStreamOutIsUpdated;
	BYTE                    m_WaitForEndChangeModeToConnectContent;
	BYTE                    m_isSmartRecovery;
	BYTE                    m_EpDidNotOpenContent;

	DWORD                   m_ssrcAudio;
	bool                    m_bIsAlreadyUpgradeToMixAvcToSvc;
	bool                    m_bIsAckOnCreatePartyReceived;
	bool                    m_bIsAckOnAvcToSvcArtTransalatorReceived;
	PDECLAR_MESSAGE_MAP
};

#endif // ifndef _ISDNPARTYCNTL

