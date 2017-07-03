#ifndef HELPERFUNCS_H_
#define HELPERFUNCS_H_

#include "AllocateStructs.h"
#include "ProcessBase.h"
#include <set>
#include "InnerStructs.h"
#include "CardsStructs.h"
#include "CRsrvDB.h"
#include "CentralConferencesDB.h"

class CConnToCardManager;
class CConfRsrcDB;
class CSystemResources;
class CProcessSettings;
class CNetServicesDB;
class CReservator;
class CRsrvDB;
class CSleepingConference;
class CConfRsrvRsrc;
class CMoveManager;
class CResourceManager;
class CProfilesDB;

class CHelperFuncs
{
public:
  static BOOL IsAudioParty(eVideoPartyType videoPartyType);
  static BOOL IsVideoParty(eVideoPartyType videoPartyType);
  static BOOL IsAudioContentParty(eVideoPartyType videoPartyType);
  static BOOL IsISDNParty(eNetworkPartyType networkPartyType);
  static BOOL IsIPParty(eNetworkPartyType networkPartyType);
  static BOOL IsVideoISDNParty(eNetworkPartyType networkPartyType, eVideoPartyType videoPartyType);
  static BOOL IsPSTNParty(eNetworkPartyType networkPartyType, eVideoPartyType videoPartyType);
  static BOOL IsLegalVideoParty(eVideoPartyType videoPartyType);
  static BOOL IsTIPSlavePartyType(ETipPartyTypeAndPosition tipPartyType);
  static BOOL IsDialOutParty(ISDN_SPAN_PARAMS_S* pIsdnParams);
  static eVideoPartyType GetNextLowerVideoPartyType(eVideoPartyType videoPartyType);
  static BOOL IsVideoBasicParty(eVideoPartyType videoPartyType);
  static BOOL IsVideo2CParty(eVideoPartyType videoPartyType);
  static BOOL IsVideoSwitchParty(eVideoPartyType videoPartyType);
  static BOOL IsVideoRelayParty(eVideoPartyType videoPartyType, bool includingRelayVSW = true);
  static BOOL IsVswRelayParty(eVideoPartyType videoPartyType);
  static BOOL IsNeedAllocateVideo(eVideoPartyType videoPartyType, eConfModeTypes confModeType, eProductType prodType);
  static BOOL IsMasterType(ECntrlType cntrl_type);
  static BOOL IsSlaveType(ECntrlType cntrl_type);
  static float GetNumPhysicalHD720Ports(eVideoPartyType videoPartyType);
  static WORD GetNumArtChannels(PartyDataStruct& partyData);
  static BOOL IsBreeze(eCardType cardType);
  static BOOL IsMpmRx(eCardType cardType);
  static BOOL IsMpmRxOrNinja(eCardType cardType);
  static BOOL IsSoftCard(eCardType cardType); //OLGA - SoftMCU
  static BOOL IsSoftMCU(eProductType cardType); //OLGA - SoftMCU
  static BOOL IsValidBoardId(WORD bId); //1-based
  static eResourceAllocationTypes GetResourceAllocationType();
  static BOOL IsEnhancedCardsAllocationType();
  static BOOL IsLegalCardsAllocationType();
  static BOOL IsAutoModeAllocationType();
  static BOOL IsFixedModeAllocationType();
  static BOOL IsEventMode();
  static BOOL IsLogicalVideoEncoderType(eLogicalResourceTypes type);
  static BOOL IsLogicalVideoDecoderType(eLogicalResourceTypes type);
  static BOOL IsLogicalSoftMixVideoEncoderType(eLogicalResourceTypes type);
  static BOOL IsLogicalRTPtype(eLogicalResourceTypes type);
  static CConnToCardManager* GetConnToCardManager();
  static CConfRsrcDB* GetConfRsrcDB();
  static CSystemResources* GetSystemResources();
  static eProcessStatus GetProcessStatus();
  static CProcessSettings* GetProcessSettings();
  static CNetServicesDB* GetNetServicesDB();
  static CReservator* GetReservator();
  static CRsrvDB* GetRsrvDB();
  static SleepingConferences* GetSleepingConferences();
  static ReservedConferences* GetConfRsrvRsrcs();
  static CProfilesDB* GetProfilesDB();
  static CMoveManager* GetMoveManager();
  static CResourceManager* GetResourceManager();
  static BOOL IsMode2C();
  static BOOL IsSessionTypeCOP(eSessionType sessionType);
  static WORD GetMaxUnitsNeededForVideo();
  static WORD GetMaxRequiredPortsPerMediaUnit();
  static DWORD CalcIpServicePart(DWORD total, float service_factor, BOOL round_up);
  static float CalcIpServicePart(float total, float service_factor, BOOL round_up);
  static BYTE IsMultipleService();
  static eSystemCardsMode GetSystemCardsModeFromCardType(eCardType cardType);
  static STATUS CheckAllReservationsSysMode();
  static const char* GetIpServiceName(DWORD service_id);
  static DWORD GetIpServiceId(const char* service_name);
  static float GetArtPromilsBreeze();
  static void GetRequiredAvcSvcTranslatingVideoResorces(eVideoPartyType videoPartyType,BOOL& isNeeded, BOOL& isSvc, BOOL& isHD);
  static eConfModeTypes GetConferenceMode(ConfMonitorID confId);
  static bool IsDynamicMixedAvcSvcAllocationMode();
};

#endif /*HELPERFUNCS_H_*/
