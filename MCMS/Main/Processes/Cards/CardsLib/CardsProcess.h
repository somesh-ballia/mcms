// CardsProcess.h

#ifndef CARDS_PROCESS_H_
#define CARDS_PROCESS_H_

#include <set>
#include <map>

#include "ProcessBase.h"
#include "MediaIpParameters.h"
#include "MediaIpParamsVector.h"
#include "IvrAddMusicSourceVector.h"
#include "CardsInternalStructs.h"
#include "McuMngrInternalStructs.h"
#include "AlarmableTask.h"
#include "ObjString.h"
#include "OpcodesMcmsInternal.h"
#include "StructTm.h"
#include "CardsDefines.h"
#include "RtmIsdnMngrInternalDefines.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "RtmIsdnMngrCommonMethods.h"
#include "McmsAuthentication.h"
#include "IceCmInd.h"
#include "IceCmReq.h"
#include "ArtDefinitions.h"
#include "AllocateStructs.h"

class CCommCardDB;
class CEthernetSettingsStructWrappersList;
class CSlotsNumberingConversionTableWrapper;
struct SNMP_CARDS_INFO_S;

enum eCardsStartupCondType
{
  eCardsIpService = 0,
  eCardsIvr,

  NumOfCardsStartupCondTypes
};

enum eMfaTaskStateType
{
  eMfaTaskOk    = 0,
  eMfaTaskZombie,

  NumOfMfaTaskTypes
};

enum eCardsFipsSimulationMode
{
  eInactiveSimulation,
  eFailOpenSSLFipsTest
};

const DWORD DEFAULT_KEEP_ALIVE_SEND_PERIOD = 10;

#define SWITCH_IP_ADDRESS       "169.254.128.16"
#define MEDIA_CARD_1_IP_ADDRESS   "169.254.128.67"
#define MEDIA_CARD_2_IP_ADDRESS   "169.254.128.68"
#define MEDIA_CARD_3_IP_ADDRESS   "169.254.128.69"
#define MEDIA_CARD_4_IP_ADDRESS   "169.254.128.70"



class CCardIceResponse : public CPObject
{
  CLASS_TYPE_1(CCardsIceResponse, CPObject)

 public:
                      CCardIceResponse();
                      CCardIceResponse(DWORD board_id, DWORD subBoardId);
  virtual const char* NameOf() const { return "CCardsIceResponse";}
  void                SetIceResponse(DWORD req_id,
                                     DWORD Status,
                                     WORD STUN_Pass_status,
                                     WORD STUN_udp_status,
                                     WORD STUN_tcp_status,
                                     WORD Relay_udp_status,
                                     WORD Relay_tcp_status,
                                     WORD fw_type);
  void                SetResponseReceived(BOOL bResponseReceived);
  DWORD               GetBoardId();
  DWORD               GetSubBoardId();
  bool                GetResponseReceived();
  ICE_INIT_IND_S      GetIceResponse();
  void                ResetIceResponseDetails();

 private:
  DWORD          m_boardId;
  DWORD          m_subBoardId;
  ICE_INIT_IND_S m_IceResponse;
  bool           m_ResponseReceived;
};

class CCardsIceResponseList : public CPObject
{
  CLASS_TYPE_1(CCardsIceResponseList, CPObject)

 public:
                      CCardsIceResponseList();
  virtual            ~CCardsIceResponseList();
  virtual const char* NameOf() const { return "CCardsIceResponseList"; }
  void                SetReqId(DWORD id);
  void                SetIceServerTypes(ICE_SERVER_TYPES_S* pIceServerTypesStruct,
                                        int nService_id = 1);
  WORD                GetNumOfCards();
  CCardIceResponse*   GetCardResponse(WORD pos);
  DWORD               GetReqId();
  ICE_SERVER_TYPES_S* GetIceServerTypes(int nService_id = 1)
                      { return m_pIceServerTypesStruct[nService_id]; }
  void                AddCardIceResponse(DWORD board_id, DWORD sub_board_id);

 private:
  DWORD               m_ReqId;
  CCardIceResponse*   m_ResponseList[MAX_NUM_OF_BOARDS];
  WORD                m_NumOfCards;
  ICE_SERVER_TYPES_S* m_pIceServerTypesStruct[MAX_NUM_OF_BOARDS];
};

class CCardsProcess : public CProcessBase
{
  CLASS_TYPE_1(CCardsProcess, CProcessBase)
  friend class CTestCardsProcess;

 public:
                                         CCardsProcess();
  virtual                               ~CCardsProcess();
  virtual const char*                    NameOf() const   { return "CCardsProcess"; }
  virtual eProcessType                   GetProcessType() { return eProcessCards; }
  virtual BOOL                           UsingSockets()   { return NO; }
  virtual TaskEntryPoint                 GetManagerEntryPoint();
  virtual void                           AddExtraStringsToMap();
  void                                   AddToMfaTasksList(COsQueue* mbx,
                                                           DWORD boardId,
                                                           DWORD subBoardId);
  void                                   RemoveFromMfaTasksList(DWORD boardId,
                                                                DWORD subBoardId);
  void                                   TurnMfaTaskToZombie(COsQueue* mbx);
  void                                   RemoveFromMfaTasksList(COsQueue* mbx);
  COsQueue*                              GetMfaMbx(DWORD boardId,
                                                   DWORD subBoardId);
  void                                   SetMfaTaskState(DWORD boardId,
                                                         DWORD subBoardId,
                                                         eMfaTaskStateType mfaTaskState);
  eMfaTaskStateType                      GetMfaTaskState(DWORD boardId,
                                                         DWORD subBoardId);
  virtual bool                           RequiresProcessInstanceForUnitTests()
                                         { return true; }
  CMediaIpParamsVector*                  GetMediaIpParamsVector() const;
  CIvrAddMusicSourceVector*              GetIvrAddMusicSourceVector() const;
  CCommCardDB*                           GetCardsMonitoringDB() const;
  void                                   PrintIvrAddMusicSourceDataToTrace(SIVRAddMusicSource* ivrAddMusicSource,
                                                                           eLogLevel traceLevel = eLevelInfoNormal);
  void                                   PrintFolderMountDataToTrace(CM_FOLDER_MOUNT_S pFolderMount);
  void                                   PrintLastKeepAliveTimes(DWORD boardId,
                                                                 CStructTm lastKeepAliveReqTime,
                                                                 CStructTm lastKeepAliveIndTime);
  void                                   SetAuthenticationStruct(const MCMS_AUTHENTICATION_S authentStruct);
  MCMS_AUTHENTICATION_S*                 GetAuthenticationStruct();
  void                                   SetLicensingStruct(CARDS_LICENSING_S* LicensingStruct);
  CARDS_LICENSING_S*                     GetLicensingStruct();
  void                                   SetIsAuthenticationSucceeded(const BOOL isSucceeded);
  BOOL                                   GetIsAuthenticationSucceeded() const;
  void                                   SetNumOfBoards();
  DWORD                                  GetNumOfBoards() const;
  void                                   SetNumOfBoardsExpected(const int numOfBoards);
  int                                    GetNumOfBoardsExpected() const;
  void                                   IncreaseNumOfBoardsExpected();
  BOOL                                   GetIsDebugMode() const;
  void                                   SetIsToTreatRtmIsdnKeepAlive(const BOOL isToTreat);
  BOOL                                   GetIsToTreatRtmIsdnKeepAlive() const;
  bool                                   SetSystemCardsModeCur(const std::string& theModeStr);
  void                                   SetSystemCardsModeCur(const eSystemCardsMode theMode);
  eSystemCardsMode                       GetSystemCardsModeCur() const;
  void                                   SetSystemCardsModeFromFile(const std::string& theModeStr);
  void                                   SetSystemCardsModeFromFile(const eSystemCardsMode theMode);
  eSystemCardsMode                       GetSystemCardsModeFromFile() const;
  
  void                                   SetNGBSystemCardsMode(const eNGBSystemCardsMode theMode);
  eNGBSystemCardsMode                    GetNGBSystemCardsMode() const;
  
  void                                   SetIsCardsModeEventAlertAlreadyProduced(const bool isProduced);
  bool                                   GetIsCardsModeEventAlertAlreadyProduced() const;
  void                                   SetIsIvrMusicAddSrcReqReceived(const bool isReceived);
  bool                                   GetIsIvrMusicAddSrcReqReceived() const;
  void                                   SetStartupCond(eCardsStartupCondType condType,
                                                        BYTE value);
  STATUS                                 IsCondOk(eCardsStartupCondType type);
  virtual int                            GetProcessAddressSpace() { return 50*1024*1024; }
  eCardState                             ConvertTaskStatusToCardState(eTaskStatus taskStatus,
                                                                      eTaskState taskState);
  eCardState                             ConvertTaskStatusToCardState(eTaskStatus taskStatus,
                                                                      eTaskState taskState,
                                                                      bool isFault);
  void                                   ActiveAlarmsStructAsString(ACTIVE_ALARMS_SPECIFIC_CARD_S* pAaStruct,
                                                                    CLargeString& AaStructStr,
                                                                    DWORD mfaBoardId);
  void                                   SendResetReqToDaemon(CSmallString errStr);
  STATUS                                 AddRtmIsdnParamsToList(const RTM_ISDN_PARAMS_MCMS_S& theStruct);
  STATUS                                 CancelRtmIsdnParams(const RTM_ISDN_SERVICE_NAME_S& theStruct);
  STATUS                                 DetachAllSpansInList(const char* serviceName);
  STATUS                                 UpdateSpanMapsList(const RTM_ISDN_SPAN_MAPS_LIST_S& theStruct);
  STATUS                                 UpdateSpecSpanMapInList(const RTM_ISDN_SPAN_MAP_S& theStruct);
  STATUS                                 DetachSpecSpanMapInList(const SPAN_DISABLE_S& theStruct);
  int                                    FindRtmIsdnService(const char* name);
  const RTM_ISDN_PARAMS_MCMS_S*          GetRtmIsdnParamsStruct(int idx) const;
  const RTM_ISDN_SPAN_MAPS_LIST_S*       GetRtmIsdnSpanMapsList() const;
  const RTM_ISDN_SPAN_MAP_S              GetRtmIsdnSpanMap(int idx) const;
  void                                   SendSnmpMfaInterface();

  void 									SendSnmpLinkStatus(DWORD boardId, WORD portId, BOOL isUp) const;

  void                                   FillSnmpMfaParam(SNMP_CARDS_INFO_S& outParam);
  EArtFips140SimulationErrCode           TranslateSysConfigDataToEnumForArt(std::string& data);
  void                                   RemoveCard(DWORD boardId,
                                                    DWORD subBoardId,
                                                    eCardType cardType);
  void                                   SendCardRemoveIndToRsrcalloc(DWORD boardId,
                                                                      DWORD subBoardId);
  void                                   SendHotSwapCardRemovedToRtmTask(DWORD boardId);
  void                                   SendHotSwapCardRemovedToCardMngr(DWORD boardId,
                                                                          DWORD subBoardId,
                                                                          eCardType cardType);
  bool                                   IsMpmCard(const eCardType cardType);
  bool                                   IsMpmPlusCard(const eCardType cardType);
  bool                                   IsBreezeCard(const eCardType cardType);
  bool                                   IsMpmRXCard(const eCardType cardType);
  bool                                   IsSoftCard(const eCardType cardType);
  bool                                   IsMediaCard(const eCardType cardType);
  bool                                   IsRtmIsdnCard(const eCardType cardType);
  bool                                   IsRtmIsdnCardExistInDB();
    bool IsRtmIsdnCard(int boardId,int SubBoardId);
  bool                                   IsMpmCardExistInDB();
  bool                                   IsMpmPlusCardExistInDB();
  bool                                   IsNotMpmPlus80ExistInDB();
  bool                                   IsBreezeCardExistInDB();
  bool                                   IsMPMRXCardExistInDB();
  bool                                   IsAnyMediaCardExistsInDB();
  int                                    GetNumOfMpmBoards();
  int                                    GetNumOfMpmPlusBoards();
  int                                    GetNumOfBreezeBoards();
  int                                    GetNumOfMPMRXBoards();
  ePlatformType                          ConvertProductTypeToPlatformType(eProductType productType);
  bool                                   IsSingleMediaBoardOnSecondSlot();
  bool                                   IsCardEnabled(eCardType curCardType);
  bool                                   IsCardEnabled_ProductType(eCardType curCardType);
  bool                                   IsCardEnabled_SystemCardsMode(eCardType curCardType);
  DWORD                                  CreateIdFromBoardIdMsgId(const int boardId,
                                                                  const int msgId,
                                                                  const int autoRemovalMode);
  void                                   SetCpuBoardIdSubBoardId(const int boardId,
                                                                 const int subBoardId);
  int                                    GetCpuBoardId();
  int                                    GetCpuSubBoardId();
  bool                                   IsCNRLPort(int portId, eProductType curProductType);
  CEthernetSettingsStructWrappersList*   GetEthernetSettingsStructsList();
  void                                   SendClearMaxCountersIndToSwitch(ETH_SETTINGS_STATE_S* pEthState);
  void                                   SetSlotsNumberingConversionTable(const SLOTS_NUMBERING_CONVERSION_TABLE_S* pTable);
  CSlotsNumberingConversionTableWrapper* GetSlotsNumberingConversionTable();
  void                                   InitMediaBoardsIds(eProductType curProductType);
  int                                    Get1stMediaBoardId();
  int                                    Get2ndMediaBoardId();
  int                                    Get3rdMediaBoardId();
  int                                    Get4thMediaBoardId();
  bool                                   IsCardEnabled_specRMX2000C(eCardType curCardType);
  void                                   InitIceResponseList();
  CCardsIceResponseList*                 GetCardsIceResponseList();
  void                                   PrintIceResponseListDB();
  void                                   UpdateIceResponseList(DWORD req_id,
                                                               DWORD Status,
                                                               DWORD board_Id,
                                                               DWORD subBoard_id,
                                                               WORD STUN_Pass_status, 
                                                               WORD STUN_udp_status,
                                                               WORD STUN_tcp_status,
                                                               WORD Relay_udp_status,
                                                               WORD Relay_tcp_status,
                                                               WORD fw_type);
  BOOL                                   IsAllICEResponseReceived(DWORD req_id);
  void                                   SendIceInitToSpecificMpmCM(const DWORD boardId,
                                                                    const DWORD subBoardId,
                                                                    ICE_SERVER_TYPES_S* pIceServerTypesStruct);
  DWORD                                  GetCardServiceId(DWORD board_id,
                                                          DWORD subBoardId);
  void                                   ResetBoardInstalling();
  void                                   AddBoardInstalling();
  void                                   DecearseBoardInstalling();
  void                                   ResetBoardInstallingIpmc();
  void                                   AddBoardInstallingIpmc(int board_id);
  void                                   DecearseBoardInstallingIpmc(int board_id);
  BOOL                                   TestIfCardNeedIpmcUpgrade(int board_id);
  void                                   IncBoardIpmcIndCounter(int board_id);
  void                                   DecBoardIpmcIndCounter(int board_id);
  int                                    GetBoardIpmcIndCounter();
  void                                   UpdateSoftwareProgress(int board,
                                                                int value);
  void                                   UpdateIpmcProgress(int board,
                                                            int value);
  BOOL                                   GetCardHotSwapStatus(int boardId) const;
  void                                   SetCardHotSwapStatus(int boardId,
                                                              const BOOL hotSwapStatus);
  void                                   SaveLastSsmKeepAliveStruct(SWITCH_SM_KEEP_ALIVE_S& lastSsmKeepAliveStruct);
  int                                    GetSubBoardStatus(DWORD boardId,
                                                           DWORD subBoardId,
                                                           DWORD& status);
  eCSExtIntMsgState                      GetCSMsgState(int serviceId) const;
  void                                   SetCSMsgState(int serviceId,
                                                       const eCSExtIntMsgState state);
  BOOL                                   IsExtMsgArrived(int serviceId) const;
  BOOL                                   IsIntMsgArrived(int serviceId) const;
  void                                   SetExtMsgArrived(int serviceId,
                                                          BOOL isArrive);
  void                                   SetIntMsgArrived(int serviceId,
                                                          BOOL isArrive);
  void                                   SetCSIpConfigBoardId(int serviceId,
                                                              DWORD boardId);
  DWORD                                  GetCSIpConfigBoardId(int serviceId) const;
  void                                   SetIsSNMPEnabled(bool e) { m_bSNMPEnabled = e; }
  bool                                   GetIsSNMPEnabled() const { return m_bSNMPEnabled; }

  eNGBSystemCardsMode                    ConvertSystemCardsModeToNGBCardsMode(eSystemCardsMode cardMode);

 protected:
  COsQueue*         m_pMfaTasksMbxs_List[MAX_NUM_OF_BOARDS][MAX_NUM_OF_SUBBOARDS];
  eMfaTaskStateType m_MfaTasksState_List[MAX_NUM_OF_BOARDS][MAX_NUM_OF_SUBBOARDS];

  CARDS_LICENSING_S*                   m_pLicensingStruct;
  BOOL                                 m_isAuthenticationSucceeded;
  DWORD                                m_numOfBoards;
  int                                  m_numOfBoardsExpected;
  MCMS_AUTHENTICATION_S*               m_pAuthenticationStruct;
  CMediaIpParamsVector*                m_pIpMediaParamsVector;
  CIvrAddMusicSourceVector*            m_pIvrAddMusicSourceVector;
  CCommCardDB*                         m_pCardsMonitoringDB;
  RTM_ISDN_SPAN_MAPS_LIST_S*           m_pRtmIsdnSpanMapsList;
  RTM_ISDN_PARAMS_MCMS_S*              m_pRtmIsdnParamsList[MAX_ISDN_SERVICES_IN_LIST];
  int                                  m_rtmIsdnParamsIdx;
  BOOL                                 m_isToTreatRtmIsdnKeepAlive;
  CRtmIsdnMngrCommonMethods            m_rtmIsdnCommonMethods;
  eSystemCardsMode                     m_systemCardsModeCur;
  eSystemCardsMode                     m_systemCardsModeFromFile;
  eNGBSystemCardsMode                  m_NGBSystemCardsMode;
  bool                                 m_isCardsModeEventAlertAlreadyProduced;
  bool                                 m_isIvrMusicAddSrcReqReceived;
  CEthernetSettingsStructWrappersList* m_pEthernetSettingsStructsList;

  void 								   SetCardIPAddressesInICEInitReq(const DWORD boardId, const DWORD subBoardId, ICE_SERVER_TYPES_S* pIceServerTypesStruct);

private:
  CSlotsNumberingConversionTableWrapper* m_pSlotsNumConversionTable;
  BYTE m_StartupEndCondArray[NumOfCardsStartupCondTypes];
  int  m_cpuBoardId;
  int  m_cpuSubBoardId;
  int  m_1stMediaBoardId;
  int  m_2ndMediaBoardId;
  int  m_3rdMediaBoardId;
  int  m_4thMediaBoardId;
  int  m_numberOfBoardInstalling;
  int  m_numberOfBoardNeedsIpmc;
  int  m_numberofIpmcIndToCount;
  bool m_bSNMPEnabled;

  std::set<int>      m_boards_requires_ipmc_upgrade;
  std::map<int, int> m_software_progress_map;
  int                m_last_total_software_progress;
  std::map<int, int> m_ipmc_progress_map;
  int                m_last_total_ipmc_progress;

  CCardsIceResponseList*       m_pIceResponseList;
  BOOL                         m_CardsHotSwapStatus[MAX_NUM_OF_BOARDS];
  CS_EXT_INT_MSG_CARD_PARAMS_S m_CSInternalExtMsgsStatus[MAX_NUM_OF_IP_SERVICES+1];
  BOOL                         m_firstSsmKeepAliveReceived;
  SWITCH_SM_KEEP_ALIVE_S       m_lastSsmKeepAliveStruct;
  eSystemCardsMode             m_SystemCardsMode;
};

#endif  // CARDS_PROCESS_H_
