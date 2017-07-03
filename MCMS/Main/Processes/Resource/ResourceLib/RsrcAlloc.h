#if !defined(AFX_RSRCALLOC_H__C3513383_F59F_4BE2_BA07_69B849DA4111__INCLUDED_)
	#define AFX_RSRCALLOC_H__C3513383_F59F_4BE2_BA07_69B849DA4111__INCLUDED_

#include "ConfResources.h"
#include "ConnToCardManager.h"
#include "AllocateStructs.h"
#include "ProcessBase.h"
#include "PObject.h"
#include "DataTypes.h"
#include "Reservator.h"

#include "IPMC.h"
#include "IPMCInterfaceApi.h"

typedef std::vector<ConnectionID> ConnectionIDs;

typedef struct
{
	BoardID boardId;
	UnitID  unitId;
	PortID  portId;
} BoardPortUnitStruct;

struct CompareBoardUnitPort
{
	bool operator ()(const BoardPortUnitStruct& lhs, const BoardPortUnitStruct& rhs) const
	{
		return (!(lhs.boardId == rhs.boardId && lhs.unitId == rhs.unitId && lhs.portId == rhs.portId));
	}
};

#ifdef MS_LYNC_AVMCU_LINK
typedef struct SigAvMcuConnIdsStruct             // MS Lync - AV MCU link
{
	SigAvMcuConnIdsStruct() : partySigOrganizerConnId(0), partySigFocusConnId(0), partySigEventPackConnId(0) { };
	DWORD partySigOrganizerConnId;
	DWORD partySigFocusConnId;
	DWORD partySigEventPackConnId;
}SigAvMcuConnIdsStruct;
#endif

////////////////////////////////////////////////////////////////////////////
//                        CRsrcAlloc
////////////////////////////////////////////////////////////////////////////
class CRsrcAlloc : public CPObject
{
	CLASS_TYPE_1(CRsrcAlloc, CPObject)

public:
	            CRsrcAlloc();
	           ~CRsrcAlloc();
	const char* NameOf() const { return "CRsrcAlloc"; }

	void        Allocate(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);
	void        DeAllocate(DEALLOC_PARTY_REQ_PARAMS_S* pParam, DEALLOC_PARTY_IND_PARAMS_S* pResult);
	void        ReAllocate(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);

	void        AllocateParty2C(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);
	void        AllocatePartyEQ(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);
	void        AllocateTIPContent(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);

	STATUS      CreateAllocPartyVSW(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, DWORD monitor_conf_id, DWORD monitor_party_id,
	                                DWORD& partyRsrcId, DWORD& partyCSconnId, PartyDataStruct& partyData,
	                                CRsrcDesc*& pEncDesc, CRsrcDesc*& pDecDesc, WORD* roomPartyId = NULL);

	void        ReAllocateART(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);
	void        ReAllocateRTMBoardFull(BOARD_FULL_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);

	STATUS      StartConf(CONF_RSRC_REQ_PARAMS_S* pParams, CONF_RSRC_IND_PARAMS_S* pResult);
	STATUS      TerminateConf(CONF_RSRC_REQ_PARAMS_S* pParams, CONF_RSRC_IND_PARAMS_S* pResult);
	STATUS      EndPartyMove(PARTY_MOVE_RSRC_REQ_PARAMS_S* pParams, PartyRsrcID& partyId, ConfRsrcID& confId);
	STATUS      EndPartyMove2C(PARTY_MOVE_RSRC_REQ_PARAMS_S* pParams, PARTY_MOVE_RSRC_IND_PARAMS_S* pResult);
	STATUS      StartPartyMove(PARTY_MOVE_RSRC_REQ_PARAMS_S* pParams, PartyRsrcID& partyId, ConfRsrcID& confId);

	// allocate
	STATUS      CreateAllocConfRsrc(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, ConfMonitorID monitor_conf_id, ConfRsrcID& confId, eSessionType sessionType, eConfMediaType confMediaType);

#ifndef MS_LYNC_AVMCU_LINK
	STATUS      CreateAllocPartyRsrc(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, ConfMonitorID monitor_conf_id, PartyMonitorID monitor_party_id, PartyRsrcID& partyId,
	                                 DWORD& partyCSconnId, PartyDataStruct& partyData, ETipPartyTypeAndPosition tipType = eTipNone, WORD* roomPartyId = NULL);
#else
	STATUS      CreateAllocPartyRsrc(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, ConfMonitorID monitor_conf_id, PartyMonitorID monitor_party_id, PartyRsrcID& partyId,
	                                 DWORD& partyCSconnId, PartyDataStruct& partyData, ETipPartyTypeAndPosition tipType = eTipNone, WORD* roomPartyId = NULL, SigAvMcuConnIdsStruct* pSigAvMcuConnIds = NULL);
#endif

	// SD
	STATUS      UpdatePartyType(CConfRsrcDB* pConfDB, ConfMonitorID monitor_conf_id, PartyMonitorID monitor_party_id, eVideoPartyType videoPartyType);

	STATUS      AllocateISDNResources(DWORD monitor_conf_id, DWORD rsrcPartyId, ISDN_PARTY_IND_PARAMS_S& isdn_Params_Response, ISDN_SPAN_PARAMS_S& isdn_params, WORD reqArtBoardId, ipAddressV4If& artIpAddrV4, CRsrcDesc**& pRtmDescArray, ALLOC_PARTY_IND_PARAMS_S* pResult);
	STATUS      AllocateART(ConfMonitorID monitor_conf_id,
	                        PartyRsrcID rsrcPartyId,
	                        DWORD partyCapacity,
	                        WORD serviceId,
	                        WORD subServiceId,
	                        eSessionType sessionType,
	                        PartyDataStruct& partyData,
	                        CRsrcDesc*& pAudEncDesc,
	                        CRsrcDesc*& pAudDecDesc,
	                        CRsrcDesc*& pRtpDesc,
	                        CUdpRsrcDesc*& pUdpRsrcDesc,
	                        BOOL isIceParty = FALSE,
	                        BOOL isBFCPUDP = FALSE,
	                        WORD reqBoardId = 0);         //ICE 4 ports

	STATUS      AllocateVideo(DWORD monitor_conf_id, PartyRsrcID partyId, eSessionType sessionType, AllocData& videoAlloc, PartyDataStruct& partyData, CRsrcDesc**& pVideoDescArray);
	STATUS      AllocateRTMOneChannel(CConfRsrc* pConf, CSystemResources* pSystemResources, DWORD rsrcConfId, DWORD rsrcPartyId, BOOL bIsDialOut, WORD reqUnitId,
	                                  WORD reqBoardId, CRsrcDesc*& pRtmDesc);
	STATUS      AllocateRTMAllChannels(int actualNumberOfRTMPortsToAllocate, DWORD monitor_conf_id, DWORD rsrcPartyId, ISDN_PARTY_IND_PARAMS_S& isdn_Params_Response,
	                                   ISDN_SPAN_PARAMS_S& isdn_params, CRsrcDesc**& pRtmDescArray, DWORD rsrcConfId, BOOL bIsDialOut, CConfRsrc* pConf);

	// deallocate
	STATUS      DestroyDeAllocConfRsrc(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, DWORD monitor_conf_id, DWORD deallocateStatus = STATUS_OK);
	STATUS      DestroyDeAllocPartyRsrc(CSystemResources* pSystemResources, CConfRsrcDB* pConfDB, DWORD monitor_conf_id, DWORD monitor_party_id);

	STATUS      DeAllocateISDNResources(DWORD monitor_conf_id, DWORD rsrcPartyId, char* serviceName, DWORD* connIdRTMArray, BYTE is_udp = FALSE);
	STATUS      DestroyDeAllocConfRsrcCOP(CSystemResources* pSystemResources, CConfRsrc* pConfDB, DWORD monitor_conf_id, DWORD deallocateStatus);
	STATUS      DestroyDeAllocConfRsrcVSW(CSystemResources* pSystemResources, CConfRsrc* pConfDB, DWORD monitor_conf_id);


	STATUS      DeAllocateART(ConfMonitorID monitorConfId, PartyRsrcID partyId, DWORD& connIdAudEnc, DWORD& connIdAudDec, DWORD& connIdRTPOrMux, DWORD& connIdContentRTP,
	                          BYTE is_enc = FALSE, BYTE is_dec = FALSE, BYTE is_rtp = FALSE, BYTE is_mux = FALSE, BYTE is_udp = FALSE, BYTE portNotResponding = 0);
	STATUS      DeallocateVideo(ConfMonitorID monitorConfId, PartyRsrcID partyId, ConnectionIDs& connectionIdsVideo, BYTE kill_enc = FALSE, BYTE kill_dec = FALSE);

	STATUS      DeAllocateRTMOneChannel(CConfRsrc* pConf, const CRsrcDesc* pRTMDesc, BYTE is_udp = FALSE);
	STATUS      UpdateRsrcDesc(CConfRsrc* pSrcConfRsrc, CConfRsrc* pTrgConfRsrc, DWORD rsrcPartyId,
	                           eLogicalResourceTypes r_type, DWORD& connId, ECntrlType cntrl = E_NORMAL, WORD portId = 0, WORD unitId = 0, WORD acceleratorId = 0, WORD boardId = 0);

	void        TestAllocateDeAllocateFixed();

	// ***udp 3 ports simulation
	STATUS      SimulateAllocateUDP(DWORD rsrcConfId, DWORD rsrcPartyId, eVideoPartyType videoPartyType, UdpAddresses& udp);
	STATUS      SimulateDeAllocateUDP(DWORD rsrcConfId, DWORD rsrcPartyId);

	STATUS      AllocateUDPtoParty(WORD servId, WORD subServiceId, WORD boxId, WORD boardId, WORD subBoardId, WORD unitId, eVideoPartyType videoPartyType, CUdpRsrcDesc* pUdpRsrcDesc, bool isIceParty = false, bool isBFCPUDP = false);         //ICE 4 ports
	STATUS      AllocateAdditionalUDPtoParty(WORD servId, WORD subServiceId, WORD boxId, WORD boardId, WORD subBoardId, WORD unitId, CUdpRsrcDesc* pUdpRsrcDesc, bool isIceParty = false, bool isBFCP = false);                                  //ICE 4 ports
	STATUS      DeAllocatePartyUDP(WORD boxId, WORD boardId, WORD subBoardId, WORD unitId, CUdpRsrcDesc* pUdpDesc, bool disable = false);

	STATUS      AllocateBondingTemporaryNumber(const char* serviceName, DWORD rsrcConfId, DWORD rsrcPartyId, Phone* tempBondingPhoneAllocated, const char* pSimilarToThisString = NULL);
	STATUS      DeAllocateBondingTemporaryNumber(const char* serviceName, DWORD monitorConfId, DWORD monitorPartyId, const char* tempBondingPhoneToDeAllocate);

	STATUS      RemoveAdditionalISDNResources(const char* serviceName, DWORD monitor_conf_id, DWORD rsrcPartyId);

	int         GetActualNumberOfRTMPortsToAllocate(ISDN_SPAN_PARAMS_S& isdn_params);

	void        AllocatePcm(ALLOC_PCM_RSRC_REQ_PARAMS_S* pParam, ALLOC_PCM_RSRC_IND_PARAMS_S* pResult);
	void        DeAllocatePcm(ALLOC_PCM_RSRC_REQ_PARAMS_S* pParam, DEALLOC_PARTY_IND_PARAMS_S* pResult);
	void        DeAllocatePcm(CConfRsrc* pConfRsrc, DWORD rsrcPartyId);

	void        UnitRecovery(UNIT_RECOVERY_S* pParam, RECOVERY_REPLACEMENT_UNIT_S* pResult);
	void        UnitRecoveryEnd(UNIT_RECOVERY_S* pParam);
	void        PrintUnitsList(BYTE board_id, eUnitType requestedUnitType);

	bool        Is1080p60SplitEncoderAllocateOnUnit(BYTE board_id, BYTE unit_id) const;

	void        ResponseWithExistingResources(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);

	STATUS      UpgradePartiesToAvcSvcMix(ConfRsrcID confId, BOOL upgradeAvc = TRUE, BOOL upgradeSvc = TRUE);

private:
	void        ReAllocateRTM(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);
	void        ReAllocateVideo(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);
	void        FreeNonAudioResources(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);
	void        ReallocateSoft(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);

	void        UpdateChannelIdForAudioEntriesInSharedMemory(DWORD rsrcPartyId, CConfRsrc* pConf, WORD rtmChannelId, CConnToCardManager* pConnToCardMngr);
	STATUS      AllocateConf2C(DWORD monitor_conf_id, CONF_RSRC_IND_PARAMS_S* pResult);
	STATUS      AllocateVideo2C(DWORD monitor_conf_id, AllocData& videoAlloc, CRsrcDesc**& pVideoDescArray);
	void        KillVideoPortVSW(DWORD monitor_conf_id, DWORD rsrcPartyId);
	STATUS      SendKillPortsCop(CConfRsrc* pConfRsrc);

	int         WriteChannelsToSharedMemoryRTM(CRsrcDesc** pRtmDescArray, int actualNumberOfRTMPortsToAllocate, CConnToCardManager* pConnToCardMngr, WORD& chanell_id_rtm, ALLOC_PARTY_IND_PARAMS_S* pResult, ConnToCardTableEntry& Entry, std::ostringstream& msg);

	int         AllocateRelayPartyResources(ALLOC_PARTY_REQ_PARAMS_S* pParam,
	                                        CRsrcDesc* pAllocatedRtpDesc,
	                                        ConnToCardTableEntry& cmEntry,
	                                        std::ostringstream& msg,
	                                        ALLOC_PARTY_IND_PARAMS_S* pResult);
	int         AllocateAvcRelayResources(ALLOC_PARTY_REQ_PARAMS_S* pParam,
	                                      PartyDataStruct& partyData,
	                                      CRsrcDesc* pAllocatedRtpDesc,
	                                      ConnToCardTableEntry& cmEntry,
	                                      std::ostringstream& msg,
	                                      ALLOC_PARTY_IND_PARAMS_S* pResult);
	int         AllocateAvcARTResources(ALLOC_PARTY_REQ_PARAMS_S* pParam,
	                                    PartyDataStruct& partyData,
	                                    WORD reqBoardId,
	                                    ConnToCardTableEntry& cmEntry,
	                                    std::ostringstream& msg,
	                                    ALLOC_PARTY_IND_PARAMS_S* pResult);

	void        WriteRsrcDescToSharedMemory(CRsrcDesc* pAllocatedRsrcDesc,
	                                        ConnToCardTableEntry& cmEntry,
	                                        std::ostringstream& msg);
	void        DeAllocateAvcRelayResources(DWORD monitor_conf_id, DWORD rsrcPartyId, bool needToKillPort = false);

	BOOL        CheckIfNewPartyHasDifferentType(DWORD monitor_conf_id, eVideoPartyType videoPartyType);

	// mixed Avc-Svc dynamic allocation
	STATUS      UpgradeAvcPartyToAvcSvcMix(CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc);
	STATUS      UpgradeSvcPartyToAvcSvcMix(CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc);
	void        InitAllocPartyReqOnUpgradeToAvcSvcMix(ALLOC_PARTY_REQ_PARAMS_S& allocPartyReq, CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc);
	void        InitPartyDataStructOnUpgradeToAvcSvcMix(PartyDataStruct& partyData, CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc, ISDN_SPAN_PARAMS_S* pIsdn_Params_Request = NULL);
	int         AllocatePartyRelayVideoResources(CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc, PartyDataStruct& partyData, ConnToCardTableEntry& cmEntry, std::ostringstream& msg);
	BOOL        CheckIfConfNeedToMoveToMixMode( DWORD monitor_conf_id, eVideoPartyType videoPartyType, BOOL& upgradeAvcParties, BOOL& upgradeSvcParties);
	void        SendUpgradeToAvcSvcMixToConfPartyProcess( DWORD monitor_conf_id);
	void        UpdatePartyResourceUsage(CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc, eVideoPartyType newVideoPartyType,     BOOL bAddAudioAsVideo = FALSE);
	void        UpgradePartyResourceUsageToMixedMode(CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc);
	int         AllocateAvcRelayVideoResources(CConfRsrc& rConfRsrc, PartyDataStruct& partyData);
	void        DeallocateAvcToSvcMixedART(CConfRsrc* pConf, const CPartyRsrc* pParty, eLogicalResourceTypes lrt, bool needToKillPort);
	STATUS      AllocateAvcToSvcMixedART(ALLOC_PARTY_REQ_PARAMS_S* pParam, PartyDataStruct& partyData, WORD reqBoardId,
	                                     ConnToCardTableEntry& cmEntry, std::ostringstream& msg,
	                                     ALLOC_PARTY_IND_PARAMS_S* pResult);

	bool        CheckEnoughResourcesTransferConfToMixedMode(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc);
	bool        IfEnoughLicensesTransferConfToMixedMode(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc, bool isNewParty);
	bool        IfEnoughARTResourcesTransferConfToMixedMode(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc, bool isNewParty);
	bool        IfEnoughVideoResourcesTransferConfToMixedMode(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc, bool isNewParty);
	WORD        GetBoardsOrderForAdditionalVideoResources(CConfRsrc& rConfRsrc, CPartyRsrc& rPartyRsrc, WORD* arrBoardsOrder);
	STATUS      StartMixedAvcSvcMove(CConfRsrc& rSrcConfRsrc, CConfRsrc& rTargetConfRsrc, CPartyRsrc& rPartyRsrc );

	static int  ConnToCardEntryDump(std::ostringstream& msg, ConnToCardTableEntry& Entry);
	static void ConnToCardEntryDumpHeader(std::ostringstream& msg, char* headerText, DWORD confId, DWORD partyId, DWORD roomId);
	static void ConnToCardEntryDumpFooter(std::ostringstream& msg);
	bool        isAVCHasHDRes(eVideoPartyType videoPartyType);

	void        CheckIfAllocOfAVCHDisSupportedAndUpdate(ALLOC_PARTY_REQ_PARAMS_S* pParam, ALLOC_PARTY_IND_PARAMS_S* pResult);

	bool        CheckBeforeUpgradePartyResourceUsage(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc, eConfMediaState currentConfMediaState);

	STATUS      AddEntryToSharedMemory(ConnToCardTableEntry& rEntry);
};

#endif // !defined(AFX_RSRCALLOC_H__C3513383_F59F_4BE2_BA07_69B849DA4111__INCLUDED_)
