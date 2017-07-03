#include "ConfPartyManager.h"

#include "NStream.h"
#include "IVRManager.h"
#include "ConfStructs.h"
#include "ConfIpParameters.h"
#include "Lobby.h"
#include "ConfPartyManagerLocalApi.h"
#include "SystemFunctions.h"
#include "VideoLayoutDrv.h"
#include "CommResDB.h"
#include "CommResDBAction.h"
#include "VideoLayoutPartyDrv.h"
#include "VisualEffectsParamsDrv.h"
#include "LectureModeParamsDrv.h"
#include "MessageOverlayInfoDrv.h"
#include "MessageOverlayInfoPartyDrv.h"
#include "ConfAction.h"
#include "IVRService.h"
#include "IVRServiceList.h"
#include "psosxml.h"
#include "ConfPartyDefines.h"
#include "WrappersConfParty.h"
#include "DefinesGeneral.h"
#include "IVRSlidesList.h"
#include "ConfApi.h"
#include "CommResShort.h"
#include "FaultsDefines.h"
#include "McuMngrInternalStructs.h"
#include "RsrvManagerApi.h"
#include "SysConfigKeys.h"
#include "SetEndTime.h"
#include "SipProxyManagerApi.h"
#include "ApiStatuses.h"
#include "DummyEntry.h"
#include "ConfPartyStatuses.h"
#include "TerminalCommand.h"
#include "OsFileIF.h"
#include "HlogApi.h"
#include "IPResourceReportGenerator.h"
#include "OpcodesMcmsInternal.h"
#include "RecordingLinkDB.h"
#include "DecoderResolutionTable.h"
#include "ResRsrcCalculator.h"
#include "IncludePaths.h"
#include "PartyPreviewDrv.h"
#include "ClientSendDtmf.h"
#include "CDRLogApi.h"
#include "ConfStart.h"
#include "CDRDetal.h"
#include "ProcessSettings.h"
#include "AllocateStructs.h"
#include "MoveAction.h"
#include "MoveInfo.h"
#include "OperatorConfInfo.h"
#include "IpCommon.h"
#include "FipsMode.h"
#include "GkTaskApi.h"
#include "ServiceConfigList.h"
#include "ConfPartyTimeOut.h"
#include "AutoScanOrderDrv.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "BridgePartyVideoUniDirection.h"
#include "ConfPartyMplMcmsProtocolTracer.h"
#include "OngoingConfStore.h"
#include "ConfPartyAssistMng.h"
#include "SiteNameInfoDrv.h"
#include "AcIndicationStructs.h"
#include "OsTask.h"
#include "EnumsToStrings.h" // TELEPRESENCE_LAYOUTS
#include "CPLayoutWrapper.h"
#include "AddressBook.h"
#include <fstream>

///////////////////////////////////////////////////////////////////////////
// MCCF IVR Package (for AT&T project)
#include "MscIvr.h"

#include "DialogStart.h"
#include "DialogPrepare.h"
#include "DialogTerminate.h"

#include "Event.h"

#include "MscIvrResponse.h"
#include "MccfIvrPackageResponse.h"
#include "IvrPackageStatusCodes.h"

#include "MccfIvrDialogManager.h"
#include "MccfIvrDispatcher.h"

#include "FilesCache.h"
#include "MediaTypeManager.h"

///////////////////////////////////////////////////////////////////////////
#include "CdrCommonApiFactory.h"
#include "CdrPersistApiFactory.h"
#include "EventPackageInterfaceApiFactory.h"
#include "IpCsOpcodes.h"
#include "MSSubscriberMngr.h"

///////////////////////////////////////////////////////////////////////////
#include "TraceStream.h"
#include "PrettyTable.h"

///////////////////////////////////////////////////////////////////////////
#include <fcntl.h>

#include <sys/signal.h>

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <sstream>

///////////////////////////////////////////////////////////////////////////
using namespace std;

/////////////////////////////////////////////////////////////////////////////
extern "C" void                 InitEncryptionTables();
extern "C" void                 ConfEntryPoint(void* appParam);

extern CCommConfDB*             GetpConfDB();
extern CPrecedenceSettings*		GetpPrecedenceSettingsDB();
extern CCommResDB *             GetpTemplatesDB();
extern CCommResDB *             GetpProfilesDB();
extern CCommResDB *             GetpMeetingRoomDB();
extern CIpServiceListManager*   GetIpServiceListMngr();
extern CRecordingLinkDB*        GetRecordingLinkDB();
extern CDecoderResolutionTable* GetpDecoderResolutionTable();

extern CCommResDB *             GetpConfTemplateDB();
extern BOOL                     GetVendorDetection();	// for Call Generator - Vendor detection
extern char* CardTypeToString(APIU32 cardType);
extern std::map<DWORD, PartyMonitorID> * GetMapPartiesTasksIds();
extern char* CascadeModeToString(BYTE cascadeMode);
//CDR_MCCF:
extern std::vector< COsQueue > GetClientRspMbxForMCCFtwinTxList();

extern std::map<DWORD, eIvrSlideConversionStatus> & GetConnectConversionStatus();

static CConfPartyRoutingTable*  pConfPartyRoutingTable = NULL;
static CCommConfDB*             pConfDB = NULL;
static CCommResDB*              g_pProfilesDB = NULL;
static CCommResDB*              g_pMeetingRoomsDB = NULL;
static CCommResDB*              g_pConfTemplateDB = NULL;
static CRecordingLinkDB*        pRecordingLinkDB = NULL;
static CDecoderResolutionTable* pDecoderResolutionTable = NULL;
static CPrecedenceSettings*		pPrecedenceSettingsDB = NULL;
static std::map<DWORD, DWORD> * g_TaskIdToPartyId = new std::map<DWORD, DWORD>;

//CDR_MCCF:
typedef std::vector< COsQueue > CLIENT_RSP_MBX_LIST;
static CLIENT_RSP_MBX_LIST clientRspMbxList;

// Conversion Status
typedef std::map<DWORD, eIvrSlideConversionStatus> IVR_SLIDE_CONVERSION_STATUS;
IVR_SLIDE_CONVERSION_STATUS g_mapConnectConversionStatus;

typedef struct _PartyPos
{
	int _pos;
	DWORD _partyId;

	static bool myCompare(_PartyPos a, _PartyPos b){ return (a._pos<b._pos);}
}PartyPos;
//std::map<std::string,CONF_PARTY_ID_S*>*  listOfConfIdPartyIdPair = new std::map<std::string,CONF_PARTY_ID_S*>;

static BOOL isVendorDetection = 1;	// for Call Generator - Vendor detection
BYTE tmp_pcm_debug_flag = 0;
extern BYTE GetTmpPcmDebugFlag() { return tmp_pcm_debug_flag;}
DWORD pcm_yuv_color = COLOR_YUV_LIGHT_GREEN;
extern BYTE GetPcmYUVColor() { return pcm_yuv_color;}

//rsrc table
CConfPartyRoutingTable* GetpConfPartyRoutingTable();
void SetpConfPartyRoutingTable(CConfPartyRoutingTable* p);

extern void ConfPartyMonitorEntryPoint(void* appParam);
extern void LobbyEntryPoint(void* appParam);

extern const CLobbyApi*   GetpLobbyApi();
extern CAVmsgServiceList* GetpAVmsgServList();

const WORD DEL_PARTY_COMPLETE = 100;
const WORD ADD_PARTY_COMPLETE = 101;

const WORD DEL_MOVED_PARTY_COMPLETE = 102;
const WORD ADD_MOVED_PARTY_COMPLETE = 103;

const WORD CHECK_IF_ALL_ACKS_RECEIVED = 104;
const WORD CHECK_IF_ALL_ACKS_RECEIVED_TOUT = 5*SECOND;

extern const char* GetSystemCardsModeStr(eSystemCardsMode theMode);
extern const char* GetSystemRamSizeStr(eSystemRamSize theSize);
extern WORD GetMaxConfTemplates();

#define DEFAULT_EQ_NAME "DefaultEQ"

#define AVERAGE_TIME_TO_DISCONNECT_PARTY  (60*SECOND) //TEMPORARY to be adjust
#define AVERAGE_TIME_TO_ALLOC_BUFFER  (3*SECOND)

bool gbStopInfinitLoop = false;
////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CConfPartyManager)
	//EMA
	ONEVENT(XML_REQUEST,                                        IDLE,    CConfPartyManager::HandlePostRequest)
	ONEVENT(ADD_OUT_PARTY,                                      ANYCASE, CConfPartyManager::OnAddDialOutPartyToConf)

	// *** MCCF ***
	// AppServer MCCF request / IVR package
	ONEVENT(MCCF_IVR_PACKAGE_REQUEST,                           ANYCASE, CConfPartyManager::OnMccfIvrMsg)
	ONEVENT(MCCF_IVR_FILE_DOWNLOADED,                           ANYCASE, CConfPartyManager::OnMccfIvrFileDownloadComplete)
	ONEVENT(MCCF_IVR_SLIDE_CONVERTED,                           ANYCASE, CConfPartyManager::OnMccfIvrSlideConvertComplete)

	// generic MCCF package initailization / termination
	// currently in use by MCCF CDR package only
	ONEVENT(MCCF_CHANNEL_SYNC,                                  ANYCASE, CConfPartyManager::OnMccfSyncMsg)
	ONEVENT(MCCF_CHANNEL_DROP,                                  ANYCASE, CConfPartyManager::OnMccfDropMsg)

	//RSRC
	ONEVENT(RSRC_START_CONF_IND,                                IDLE,    CConfPartyManager::OnRsrcStartConfInd)
	ONEVENT(RSRC_DEL_RSRV_CONF_IND,                             IDLE,    CConfPartyManager::OnRsrcDelRsrvConfInd)
	ONEVENT(RSRC_DEL_MR_IND,                                    IDLE,    CConfPartyManager::OnRsrcDelMRInd)
	ONEVENT(RSRC_ADD_MR_IND,                                    IDLE,    CConfPartyManager::OnRsrcAddMeetingRoomInd)
	ONEVENT(RSRC_ACTIVATE_MR_IND,                               IDLE,    CConfPartyManager::OnRsrcActivateMeetingRoomInd)
	ONEVENT(RSRC_DEACTIVATE_MR_IND,                             IDLE,    CConfPartyManager::OnRsrcDeActivateMeetingRoomInd)
	ONEVENT(RSRC_MR_UPDATE_IND,                                 ANYCASE, CConfPartyManager::OnRsrcUpdateMeetingRoomInd)
	ONEVENT(STARTUP_READ_MR_AND_PROFILE_DB_REQ,                 ANYCASE, CConfPartyManager::OnStartupReadMRDB)
	ONEVENT(STARTUP_INIT_MR_DB_IND,                             ANYCASE, CConfPartyManager::OnRAMeetingRoomDBInd)
	ONEVENT(HW_REMOVED_PARTY_LIST_IND,                          ANYCASE, CConfPartyManager::OnRsrcRemoveCardInd)
	ONEVENT(HW_NEW_IND,                                         ANYCASE, CConfPartyManager::OnRsrcAddCardInd)
	ONEVENT(UPDATE_IVR_CNTR_IND,                                ANYCASE, CConfPartyManager::OnRsrcUpdateIvrCntlInd)
	ONEVENT(SET_CONFERENCE_ENDTIME_IND,                         ANYCASE, CConfPartyManager::OnRsrcSetConferenceEndTimeInd)
	ONEVENT(RESOURCE_READY_IND,                                 ANYCASE, CConfPartyManager::OnRsrcReadyInd)
	ONEVENT(RESOURCE_NUMERIC_ID_IND,                            ANYCASE, CConfPartyManager::OnRsrcUpdateNumericIdInd)
	ONEVENT(RSRC_START_PREVIEW_IND,                             ANYCASE, CConfPartyManager::OnRsrcStartPreviewInd)
	ONEVENT(RSRC_STOP_PREVIEW_IND,                              ANYCASE, CConfPartyManager::OnRsrcStopPreviewInd)
	ONEVENT(EXTRACT_PARTY_RSRC_INFO,                            ANYCASE, CConfPartyManager::OnRsrcExtractPartyInfo)
	ONEVENT(RSRC_CARD_TYPE_IND,                                 ANYCASE, CConfPartyManager::OnRsrcCardTypeInd)

	// GK Manager resource Query
	ONEVENT(GKMNGR_RESOURCE_INFO_REQ,                           IDLE,    CConfPartyManager::OnGKManagerResourceQuery)
	ONEVENT(IP_RESOURCE_INFO_IND,                               IDLE,    CConfPartyManager::OnIpResourceReportInd)

	//LOBBY
	ONEVENT(START_MEETING_ROOM,                                 ANYCASE, CConfPartyManager::OnLobbyStartMeetingRoom)//ACTIVATEMR+IVRACTIVATEMR
	ONEVENT(ACCEPT_UNRESERVED_PARTY,                            IDLE,    CConfPartyManager::AddUnreservedParty)
	ONEVENT(RELEASE_UNRESERVED_PARTY,                           IDLE,    CConfPartyManager::RemoveUnreservedParty)
	ONEVENT(LOBBY_START_AD_HOC_CONF,                            IDLE,    CConfPartyManager::OnLobbyStartAdHocConf)
	ONEVENT(START_GATEWAY_CONF,                                 IDLE,    CConfPartyManager::OnLobbyStartGateWayConf)

	ONEVENT(START_MNG_ASSIST_TASK,                              ANYCASE, CConfPartyManager::OnStartAssistTask)
	//CS
	ONEVENT(CS_CONF_IP_SERVICE_PARAM_IND,                       ANYCASE, CConfPartyManager::OnConfIpServiceParamsInd)
	ONEVENT(CS_CONF_IP_SERVICE_PARAM_END_IND,                   ANYCASE, CConfPartyManager::OnConfIpServiceParamsEndInd)
	ONEVENT(CS_CONF_DELETE_IP_SERVICE_IND,                      ANYCASE, CConfPartyManager::OnConfDeleteIpeServiceParamsInd)
	ONEVENT(IPSERVICEFROMCSMNGRTOUT,                            ANYCASE, CConfPartyManager::OnTimerIPServiceFromCSMngrTout)
	ONEVENT(CS_SERVICE_CFG_UPDATE_IND,                          ANYCASE, CConfPartyManager::OnCSMngrServiceCfgUpdate)
	ONEVENT(CS_CONF_DEFAULT_SERVICE_IND,                        ANYCASE, CConfPartyManager::OnCSMngrServiceDefaultUpdate)

	//RTM
	ONEVENT(RTM_ISDN_PARAMS_IND,                                ANYCASE, CConfPartyManager::OnRtmIsdnParamsInd)
	ONEVENT(RTM_ISDN_PARAMS_END_IND,                            ANYCASE, CConfPartyManager::OnRtmIsdnParamsEndInd)
	ONEVENT(RTM_ISDN_DEFAULT_SERVICE_NAME_IND,                  ANYCASE, CConfPartyManager::OnRtmIsdnDefaultServiceNameInd)
	ONEVENT(RTM_ISDN_DELETE_SERVICE_IND,                        ANYCASE, CConfPartyManager::OnRtmDeleteIsdnServiceParamsInd)

	//IVR
	ONEVENT(IVR_MUSIC_GET_SOURCE_REQ,                           ANYCASE, CConfPartyManager::OnIvrAddMusicSourceReq)
	ONEVENT(SIP_PROXY_TO_CONF_PARTY_DB_REQ,                     ANYCASE, CConfPartyManager::OnSipProxyDBReq)
	ONEVENT(MOVE_PARTY_TO_CONF_OR_MR,                           IDLE,    CConfPartyManager::MovePartyToMROOM_OR_CONF)

	//McuMngr
	ONEVENT(MCUMNGR_TO_CONFPARTY_LICENSING_IND,                 ANYCASE, CConfPartyManager::OnMcuMngrLicensingInd)
	ONEVENT(MCUMNGR_PRECEDENCE_SETTINGS,                        ANYCASE, CConfPartyManager::OnMcuMngrPrecedenceSettings)
	ONEVENT(SYSTEM_RAM_SIZE_IND,                                ANYCASE, CConfPartyManager::OnSystemRamSizeInd)//McuMngr
	ONEVENT(MCUMNGR_CONF_GMT_UPDATE,                            ANYCASE, CConfPartyManager::OnMcuSetGMTOffsetInd)//McuMngr
	//Update part
	ONEVENT(DEL_PARTY_COMPLETE,                                 IDLE,    CConfPartyManager::OnTimerDelParty)
	ONEVENT(ADD_PARTY_COMPLETE,                                 IDLE,    CConfPartyManager::OnTimerAddParty)
	ONEVENT(ADD_RECORDING_LINK_PARTY,                           ANYCASE, CConfPartyManager::OnAddRecordingLinkParty) // natasha check during integration that event is correct
	ONEVENT(DISCONNECT_RECORDING_LINK_PARTY,                    ANYCASE, CConfPartyManager::OnDisconnectRecordingLinkParty) // natasha check during integration that event is correct

	// Move Party
	ONEVENT(DEL_MOVED_PARTY_COMPLETE,                           IDLE,    CConfPartyManager::OnTimerDelMovedParty)
	ONEVENT(ADD_MOVED_PARTY_COMPLETE,                           IDLE,    CConfPartyManager::OnTimerAddMovedParty)

	//Block conefrence creation
	ONEVENT(CONF_BLOCK_IND,                                     ANYCASE, CConfPartyManager::OnConfBlockInd) //Block conefrence creation

	//Get number of conferences
	ONEVENT(GET_CONF_NUM_REQ,                                   ANYCASE, CConfPartyManager::OnGetNumOfConferences)  //Get number of conferences

	//Cards
	ONEVENT(MCMS_SYSTEM_CARDS_MODE_IND,                         ANYCASE, CConfPartyManager::OnCardsSystemBasedModeInd)//Cards

	//Failover
	ONEVENT(FAILOVER_START_SLAVE,                               ANYCASE, CConfPartyManager::OnFailoverStartSlaveInd) // Failover got configure to Slave from GUI
	ONEVENT(FAILOVER_START_MASTER,                              ANYCASE, CConfPartyManager::OnFailoverStartMasterInd) // Failover got configure to Master from GUI
	ONEVENT(FAILOVER_SLAVE_BECOME_MASTER,                       ANYCASE, CConfPartyManager::OnFailoverSlaveBecomeMasterInd) // Failover descover that master fail
	ONEVENT(FAILOVER_START_MASTER_BECOME_SLAVE,                 ANYCASE, CConfPartyManager::OnFailoverStartMasterBecomeSlaveInd) // Failover descover that master has Network problems will send to the slave FAILOVER_SLAVE_BECOME_MASTER and in addition FAILOVER_MASTER_BECOME_SLAVE to the master
	ONEVENT(FAILOVER_RESTART_SLAVE,                             ANYCASE, CConfPartyManager::OnFailoverRestartSlaveInd) // Change only IP of the other sid3e (Master) slave stay slave
	ONEVENT(FAILOVER_CONFPARTY_ADD_OR_UPDATE_CONF_IND,          ANYCASE, CConfPartyManager::OnFailoverAddOrUpdateConfInd)
	ONEVENT(FAILOVER_CONFPARTY_TERMINATE_CONF_IND,              ANYCASE, CConfPartyManager::OnFailoverTerminateConfInd)
	ONEVENT(FAILOVER_CONFPARTY_ADD_OR_UPDATE_PROFILE_IND,       ANYCASE, CConfPartyManager::OnFailoverAddOrUpdateProfileInd)
	ONEVENT(FAILOVER_CONFPARTY_TERMINATE_PROFILE_IND,           ANYCASE, CConfPartyManager::OnFailoverTerminateProfileInd)
	ONEVENT(FAILOVER_CONFPARTY_ADD_OR_UPDATE_MEETING_ROOM_IND,  ANYCASE, CConfPartyManager::OnFailoverAddOrUpdateMeetingRoomInd)
	ONEVENT(FAILOVER_CONFPARTY_TERMINATE_MEETING_ROOM_IND,      ANYCASE, CConfPartyManager::OnFailoverTerminateMeetingRoomInd)
	ONEVENT(FAILOVER_CONFPARTY_IVR_SERVICE_LIST_IND,            ANYCASE, CConfPartyManager::OnFailoverIVRServiceListInd)
	ONEVENT(FAILOVER_CONFPARTY_RECORDING_LINKS_LIST_IND,        ANYCASE, CConfPartyManager::OnFailoverRecordingLinksListInd)

	//MPL
	ONEVENT(KILL_PORT_REQ,                                      ANYCASE, CConfPartyManager::OnKillPortAck)
	ONEVENT(ALLOC_STATUS_PER_UNIT_REQ,                          ANYCASE, CConfPartyManager::OnAllocStatusPerUnitAck)

	//Timer(Failover)
	ONEVENT(DELETE_ALL_CONFS_TOUT,                              ANYCASE, CConfPartyManager::OnDeleteAllConfsTimeOut)
	ONEVENT(MAX_TIME_DELETE_ALL_CONFS_TOUT,                     ANYCASE, CConfPartyManager::OnMaxTimeDeleteAllConfsTimeOut)

	ONEVENT(EXCHANGEMNGR_CONFIG_TO_CONFPARTY_IND,               ANYCASE, CConfPartyManager::OnExchangeConfigInd)
	ONEVENT(CONF_ACTIVEALARM_IND,                               ANYCASE, CConfPartyManager::OnConfPartyReportOnActiveAlarmInd)
	ONEVENT(SPECIFIC_MEETING_ROOM_AT_STARTUP_REQ,               ANYCASE, CConfPartyManager::OnSpecificMeetingRoomInformationReq)
	ONEVENT(SIP_PROXY_TO_CONF_END_INIT_ICE,                     ANYCASE, CConfPartyManager::OnSipProxyEndIceInit)
	ONEVENT(WEBRTC_ICE_SERVICE_MANG_TO_CONF_END_INIT_ICE,       ANYCASE, CConfPartyManager::OnWebRTCIceSeriveEndIceInit)
	ONEVENT(SIP_PROXY_TO_CONF_END_STAT_ICE,                     ANYCASE, CConfPartyManager::OnSipProxyEndIceStatus)
	ONEVENT(TIMER_START_ICE_INITIALIZATION,                     ANYCASE, CConfPartyManager::IceInitTimeout)


	//SIPProxy
	ONEVENT(CS_PROXY_CONF_STATUS_UPDATE_IND,                    ANYCASE, CConfPartyManager::OnCsProxyConfRegisterStatus)
	ONEVENT(TIMER_START_ICE_INITIALIZATION,                     ANYCASE, CConfPartyManager::IceInitTimeout)
	ONEVENT(SIP_PROXY_TO_CONF_PARTY_SERVER_TYPE_REQ,            ANYCASE, CConfPartyManager::OnProxyConfUpdateServerType)

	//Conf
	ONEVENT(CONF_DELETED_CONFERENCE,                            ANYCASE, CConfPartyManager::OnRemoveConfByTaskId)
	ONEVENT(CONF_FAILED_TASK,                                   ANYCASE, CConfPartyManager::OnConfOrPartyTaskFailed)
	ONEVENT(CONF_UPDATED_FAILED_TASK,                           ANYCASE, CConfPartyManager::OnConfUpdateTaskFailed)
	ONEVENT(CONFPARTY_IS_MCU_STARTUP_FINISHED_TIMER,            ANYCASE, CConfPartyManager::OnTimerIsMcuStartupFinished)

	//SNMP
	ONEVENT(SNMP_CONFIG_TO_OTHER_PROGRESS,                      ANYCASE, CConfPartyManager::OnSNMPConfigInd)

	ONEVENT(SET_CONF_AVC_SVC_MEDIA_STATE,                       ANYCASE, CConfPartyManager::OnResourcesSetConfAvcSvcMediaStateInd)

	ONEVENT(LOGGER_CURRENT_FILE_NUMBER_REPORT,                  ANYCASE, CConfPartyManager::OnLoggerUpdateNumber)

	ONEVENT(SIP_CS_SIG_BENOTIFY_IND,                            ANYCASE, CConfPartyManager::OnSipLyncBeNotify)

	ONEVENT(CONFPARTY_CHECK_MCCF_SLIDE_PROCEEDING_TIMER,  ANYCASE, CConfPartyManager::OnTimerCheckSlideProceeding)
    ONEVENT(CONFPARTY_DELETE_USELESS_IVR_FILES_TIMER,  			ANYCASE, CConfPartyManager::OnTimerDeleteUselessIVRFiles)

    // Convert slide from EMA done.
    ONEVENT(EMA_IVR_SLIDE_CONVERTED,                           ANYCASE, CConfPartyManager::OnEMAIvrSlideConvertComplete)
PEND_MESSAGE_MAP(CConfPartyManager, CManagerTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CConfPartyManager)
  ON_TRANS("TRANS_RES_1",                 "START",                          CCommResAdd,                CConfPartyManager::OnServerAddRsrv)
  ON_TRANS("TRANS_RES_CONTINUE_1",        "START",                          CCommResAdd,                CConfPartyManager::OnServerAddRsrvContinue1)
  ON_TRANS("TRANS_RES_CONTINUE_2",        "START",                          CCommResAdd,                CConfPartyManager::OnServerAddRsrvContinue2)
  ON_TRANS("TRANS_RES",                   "START",                          CCommResAdd,                CConfPartyManager::OnServerAddRsrvContinue)

  ON_TRANS("TRANS_RES_1",                 "START_REPEATED_EX",              CCommResAdd,                CConfPartyManager::OnServerAddRepeatedRsrv)
  ON_TRANS("TRANS_CONF_2",                "TERMINATE_CONF",                 CConfAction,                CConfPartyManager::OnServerTerminateConf)
  ON_TRANS("TRANS_RES_2",                 "TERMINATE_MEETING_ROOM",         CCommResDBAction,           CConfPartyManager::OnServerDelMR)
  ON_TRANS("TRANS_RES_2",                 "TERMINATE_PROFILE",              CCommResDBAction,           CConfPartyManager::OnServerDelProfile)
  ON_TRANS("TRANS_RES_2",                 "TERMINATE_CONFERENCE_TEMPLATE",  CCommResDBAction,           CConfPartyManager::OnServerDelConfTemplate)
  ON_TRANS("TRANS_RES_1",                 "SET_DEFAULT_EQ",                 CCommResDBAction,           CConfPartyManager::OnServerSetTransitEQ)
  ON_TRANS("TRANS_RES_1",                 "CANCEL_DEFAULT_EQ",              CCommResDBAction,           CConfPartyManager::OnServerCancelTransitEQ)
  ON_TRANS("TRANS_CONF_1",                "ADD_PARTY",                      CRsrvPartyAdd,              CConfPartyManager::OnServerAddPartyToConf)
  ON_TRANS("TRANS_CONF_1",                "UPDATE_PARTY",                   CRsrvPartyAdd,              CConfPartyManager::OnServerUpdateParty)
  ON_TRANS("TRANS_CONF_2",                "DELETE_PARTY",                   CRsrvPartyAction,           CConfPartyManager::OnServerDelParty)
  ON_TRANS("TRANS_RES_1",                 "UPDATE",                         CCommResAdd,                CConfPartyManager::OnServerUpdateAction)

  ON_TRANS("TRANS_CONF_2",                "SET_CONNECT",                    CRsrvPartyAction,           CConfPartyManager::OnServerSetConnectOrDisconnect)
  ON_TRANS("TRANS_CONF_1",                "SET_VIDEO_LAYOUT",               CVideoLayoutDrv,            CConfPartyManager::OnServerSetConfVideoLayout)
  ON_TRANS("TRANS_CONF_1",                "SET_PARTY_VIDEO_LAYOUT_EX",      CVideoLayoutPartyDrv,       CConfPartyManager::OnServerSetPartyVideoLayout)
  ON_TRANS("TRANS_CONF_2",                "SET_VISUAL_EFFECT",              CVisualEffectsParamsDrv,    CConfPartyManager::OnServerUpdateVisualEffects)
  ON_TRANS("TRANS_CONF_1",                "SET_PARTY_LAYOUT_TYPE",          CRsrvPartyAction,           CConfPartyManager::OnServerSetPartyPrivateLayoutButtonOnly)
  ON_TRANS("TRANS_CONF_1",                "SET_LECTURE_MODE",               CLectureModeParamsDrv,      CConfPartyManager::OnServerUpdateConfLectureModeParams)
  ON_TRANS("TRANS_CONF_2",                "SET_AUTO_LAYOUT",                CConfAction,                CConfPartyManager::OnServerUpdateConfAutoLayout)
  ON_TRANS("TRANS_CONF_1",                "START_PREVIEW",                  CPartyPreviewDrv,           CConfPartyManager::OnServerStartPreview)
  ON_TRANS("TRANS_CONF_1",                "STOP_PREVIEW",                   CRsrvPartyAction,           CConfPartyManager::OnServerStopPreview)
  ON_TRANS("TRANS_CONF_1",                "REQUEST_INTRA",                  CRsrvPartyAction,           CConfPartyManager::OnServerRequestIntra)

  ON_TRANS("TRANS_CONF_2",                "SET_AUDIO_VOLUME",               CRsrvPartyAction,           CConfPartyManager::OnServerSetAudioVolume)
  ON_TRANS("TRANS_CONF_2",                "SET_LISTEN_AUDIO_VOLUME",        CRsrvPartyAction,           CConfPartyManager::OnServerSetListeningAudioVolume)
  ON_TRANS("TRANS_CONF_2",                "SET_AUDIO_VIDEO_MUTE",           CRsrvPartyAction,           CConfPartyManager::OnServerSetAudioVideoMute)
  ON_TRANS("TRANS_CONF_2",                "SET_AUDIO_BLOCK",                CRsrvPartyAction,           CConfPartyManager::OnServerSetAudioBlock)
  ON_TRANS("TRANS_CONF_2",                "SET_PARTY_VISUAL_NAME",          CRsrvPartyAction,           CConfPartyManager::OnServerSetVisualName)
  ON_TRANS("TRANS_CONF_2",                "SET_BILLING_DATA",               CConfAction,                CConfPartyManager::OnServerSetBillingData)
  ON_TRANS("TRANS_CONF_2",                "SET_CONF_CONTACT_INFO",          CConfAction,                CConfPartyManager::OnServerConfContactInfo)
  ON_TRANS("TRANS_CONF_2",                "SET_PARTY_CONTACT_INFO",         CRsrvPartyAction,           CConfPartyManager::OnServerPartyContactInfo)
  ON_TRANS("TRANS_CONF_2",                "SET_AGC",                        CRsrvPartyAction,           CConfPartyManager::OnServerSetAGC)
  ON_TRANS("TRANS_CONF_2",                "SET_LEADER",                     CRsrvPartyAction,           CConfPartyManager::OnServerSetLeader)
  ON_TRANS("TRANS_CONF_2",                "SET_END_TIME",                   CSetEndTime,                CConfPartyManager::OnServerUpdateEndTime)

  ON_TRANS("TRANS_AV_MSG_SERVICE",        "ADD_IVR",                        CIVRServiceAdd,             CConfPartyManager::OnServerAddIVRService)
  ON_TRANS("TRANS_AV_MSG_SERVICE",        "UPDATE_IVR",                     CIVRServiceAdd,             CConfPartyManager::OnServerUpdateIVRService)
  ON_TRANS("TRANS_AV_MSG_SERVICE",        "DELETE",                         CIVRServiceDel,             CConfPartyManager::OnServerDeleteIVRService)
  ON_TRANS("TRANS_AV_MSG_SERVICE",        "ADD_IVR_LANGUAGE",               CIVRLanguageAdd,            CConfPartyManager::OnServerAddIVRLanguage)
  ON_TRANS("TRANS_AV_MSG_SERVICE",        "SET_DEFAULT",                    CIVRServiceSetDefault,      CConfPartyManager::OnServerSetDefaultIVRService)
  ON_TRANS("TRANS_AV_MSG_SERVICE",        "SET_DEFAULT_EQ",                 CIVRServiceSetDefault,      CConfPartyManager::OnServerSetDefaultEQService)
  ON_TRANS("TRANS_AV_MSG_SERVICE",        "CONVERT_SLIDE",                  CIVRServiceConvertSlide,    CConfPartyManager::OnServerConvertSlide)

  ON_TRANS("TRANS_CONF_2",                "SET_ENTRY_PASSWORD",             CConfAction,                CConfPartyManager::OnServerConfEntryPW)
  ON_TRANS("TRANS_CONF_2",                "SET_PASSWORD",                   CConfAction,                CConfPartyManager::OnServerSetConfChairPass)

  ON_TRANS("TRANS_CONF_2",                "WITHDRAW_CONTENT_TOKEN",         CConfAction,                CConfPartyManager::OnServerWithdrawContentToken)
  ON_TRANS("TRANS_CONF_2",                "START_CONTENT",                  CRsrvPartyAction,           CConfPartyManager::OnServerSendContentRequest)
  // for Call Generator
  ON_TRANS("TRANS_CONF_2",                "STOP_CONTENT",                   CRsrvPartyAction,           CConfPartyManager::OnServerStopContentRequest)
  // for Call Generator

  ON_TRANS("TRANS_CONF_2",                "START_RECORDING",                CConfAction,                CConfPartyManager::OnServerRecordingCommand)
  ON_TRANS("TRANS_CONF_2",                "STOP_RECORDING",                 CConfAction,                CConfPartyManager::OnServerRecordingCommand)
  ON_TRANS("TRANS_CONF_2",                "PAUSE_RECORDING",                CConfAction,                CConfPartyManager::OnServerRecordingCommand)
  ON_TRANS("TRANS_CONF_2",                "RESUME_RECORDING",               CConfAction,                CConfPartyManager::OnServerRecordingCommand)

  ON_TRANS("TRANS_CONF_2",                "SET_DTMF",                       CClientSendDtmf,            CConfPartyManager::OnServerSetDtmf)
  // for Call Generator

  ON_TRANS("TRANS_RECORDING_LINKS_LIST",  "ADD",                            CRsrvRecordLinkPartyAdd,    CConfPartyManager::OnServerAddRecordingLink)
  ON_TRANS("TRANS_RECORDING_LINKS_LIST",  "UPDATE",                         CRsrvRecordLinkPartyAdd,    CConfPartyManager::OnServerUpdateRecordingLink)
  ON_TRANS("TRANS_RECORDING_LINKS_LIST",  "DELETE",                         CRsrvPartyAction,           CConfPartyManager::OnServerDeleteRecordingLink)
  ON_TRANS("TRANS_RECORDING_LINKS_LIST",  "SET_DEFAULT_RECORDING_LINK",     CRsrvPartyAction,           CConfPartyManager::OnServerSetDefaultRecordingLink)

  ON_TRANS("TRANS_CONF_2",                "MOVE_PARTY",                     CMoveAction,                CConfPartyManager::OnServerMoveParty)
  ON_TRANS("TRANS_CONF_2",                "ATTEND_PARTY",                   CMoveBaseAction,            CConfPartyManager::OnServerAttendParty)
  ON_TRANS("TRANS_CONF_2",                "BACK_TO_CONF_PARTY",             CMoveBaseAction,            CConfPartyManager::OnServerMovePartyBackToHomeConf)

  ON_TRANS("TRANS_CONF_2",                "SET_VIDEO_CLARITY",              CConfAction,                CConfPartyManager::OnServerUpdateConfVideoClarity)
  ON_TRANS("TRANS_CONF_2",                "SET_AUTO_REDIAL",                CConfAction,                CConfPartyManager::OnServerUpdateConfAutoRedial)
  ON_TRANS("TRANS_CONF_2",                "SET_MESSAGE_OVERLAY",            CMessageOverlayInfoDrv,     CConfPartyManager::OnServerSetMessageOverLay)
  ON_TRANS("TRANS_CONF_2", "SET_PARTY_MESSAGE_OVERLAY",                  CMessageOverlayInfoPartyDrv, CConfPartyManager::OnServerSetPartyMessageOverLay)
  ON_TRANS("TRANS_CONF_2",                "SET_EXCLUSIVE_CONTENT",          CRsrvPartyAction,           CConfPartyManager::OnSetExclusiveContent)
  ON_TRANS("TRANS_CONF_2",                "SET_EXCLUSIVE_CONTENT_MODE",     CConfAction,                CConfPartyManager::OnServerSetExclusiveContentMode)
  ON_TRANS("TRANS_CONF_2",                "SET_MUTE_PARTIES_IN_LECTURE",    CConfAction,                CConfPartyManager::OnServerSetMuteIncomingLectureMode)
  ON_TRANS("TRANS_CONF_2", "SET_AUDIO_VIDEO_MUTE_PARTIES_EXCEPT_LEADER", CConfAction,                 CConfPartyManager::OnServerSetMuteAllAudioVideoPartiesExceptLeader)

  //Restricted content
  ON_TRANS("TRANS_CONF_2",                "REMOVE_EXCLUSIVE_CONTENT",       CConfAction,                CConfPartyManager::OnRemoveExclusiveContent)
  //Restricted content
  ON_TRANS("TRANS_CONF_2",                "SET_AUTO_SCAN_INTERVAL",         CConfAction,                CConfPartyManager::OnServerUpdateAutoScanInterval)
  ON_TRANS("TRANS_CONF_2",                "SET_AUTO_SCAN_ORDER",            CAutoScanOrderDrv,          CConfPartyManager::OnServerUpdateAutoScanOrder)
  ON_TRANS("TRANS_CONF_1",                "REQUEST_INTRA",                  CRsrvPartyAction,           CConfPartyManager::OnServerIntraRequest)

  ON_TRANS("TRANS_MCU",                   "SET_RESOLUTIONS_SET",            CSetResolutionSliderDetails,CConfPartyManager::OnSetResolutionThreshold)

  ON_TRANS("TRANS_CONF_2",                "SET_SITE_NAME",                  CSiteNameInfoDrv,           CConfPartyManager::OnServerSetSiteName)

  // TELEPRESENCE_LAYOUTS
  ON_TRANS("TRANS_CONF_2",                "SET_TELEPRESENCE_LAYOUT_MODE",   CConfAction,                CConfPartyManager::OnServerUpdateTelepresenceLayoutMode)

  ON_TRANS("TRANS_CUSTOMIZE_SETUP_ONGOING_CONF", "UPDATE",                  CCustomizeDisplaySettingForOngoingConfConfiguration,CConfPartyManager::OnServerSetCustomizeDisplayForConf)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CConfPartyManager)
	ONCOMMAND("StartMoveExport",                  CConfPartyManager::HandleTerminalMove,"start Move process")
	ONCOMMAND("fecc_token",                       CConfPartyManager::HandleTerminalFeccToken,"fecc_token [take|release] CONF PARTY")
	ONCOMMAND("SendAquire_ReleaseReq",            CConfPartyManager::HandleTerminalSendAquireReleaseReq,"SendAquire_ReleaseReq [Conf name] [Party ID] [Aquire/Release]")
	ONCOMMAND("removeAdd_mfa_card",               CConfPartyManager::HandleRemoveAddMFACard,"removeAdd_mfa_card CONF PARTY")
	ONCOMMAND("SendLPRInd",                       CConfPartyManager::HandleTerminalLPRInd,"SendLPRInd [Conf name] [Party ID] [LPR]")
	ONCOMMAND("SendDtmf",                         CConfPartyManager::HandleTerminalSendDtmf,"SendDtmf [Conf name] [Party ID] [DTMF string]")
	ONCOMMAND("block_conf",                       CConfPartyManager::HandleBlockConfIndication,"block_conf [block|release] CONF PARTY")
	ONCOMMAND("sim_update_rsrv_prefix",           CConfPartyManager::HandleUpdateRsrcPrefix,"sim_update_rsrv_prefix  - simulate update all reservation prefixes")
	ONCOMMAND("print_decoder_db",                 CConfPartyManager::HandlePrintDecoderDB,"print_decoder_db  - print to trace decoder resolution table")
	ONCOMMAND("limit_decoder_resolution",         CConfPartyManager::HandleLimitDecoderResolution,"limit_decoder_resolution")
	ONCOMMAND("intra_request_from_ep",            CConfPartyManager::HandleIntraRequestFromEP,"intra_request_from_ep")
	ONCOMMAND("recurrent_intra_request_from_ep",  CConfPartyManager::HandleRecurrentIntraRequestFromEP,"recurrent_intra_request_from_ep [Conf name] [Party ID] [TimeInterval] [NumIntraReq]")
	ONCOMMAND("send_rsrc_map",                    CConfPartyManager::HandleSendResourcesMap,"send_rsrc_map [board id] [unit id]")
	ONCOMMAND("OperatorAssist",                   CConfPartyManager::HandleOperatorAssistance,"Handle Operator Assistance")
	ONCOMMAND("pcm_debug",                        CConfPartyManager::HandleTerminalPCMReq,"pcm_action [Conf name] [Party name] [require action] [params (optional)]")
	ONCOMMAND("SetExclusiveContent",              CConfPartyManager::HandleSetExclusiveContent,"SetExclusiveContent [Conf ID] [Party ID]")//Restricted content

	ONCOMMAND("SetConfAvcSvcMode",                CConfPartyManager::HandleSetConfAvcSvcMode,"SetConfAvcSvcMode [Conf Type] [Conf ID]")
	ONCOMMAND("SetPartyAvcSvcMode",               CConfPartyManager::HandleSetPartyAvcSvcMode,"SetPartyAvcSvcMode [Conf Type] [Conf ID] [Monitor Party ID]")
	ONCOMMAND("RemExclusiveContent",              CConfPartyManager::HandleRemoveExclusiveContent,"RemExclusiveContent [Conf ID] ")//Restricted content
	ONCOMMAND("StartSlave",                       CConfPartyManager::HandleFailoverStartSlave,"StartSlave")
	ONCOMMAND("StartMaster",                      CConfPartyManager::HandleFailoverStartMaster,"StartMaster")
	ONCOMMAND("StartRestore",                     CConfPartyManager::HandleFailoverStopSlaveAndStartRestore,"StartRestore")
	ONCOMMAND("vendor_detection_for_CG",          CConfPartyManager::HandleVendorDetectionForCG,"vendor_detection for CallGenerator - toggle (default=ON)")
	ONCOMMAND("ChangeSysMode",                    CConfPartyManager::HandleChangeSysMode,"ChangeSysMode")//2 modes cop/cp
	ONCOMMAND("ConfPartyProcessInfo",             CConfPartyManager::HandleConfPartyProcessInfo,"display info about ConfParty process")
	ONCOMMAND("ProcessMemInfoPrint",              CConfPartyManager::HandleProcessMemInfoPrint,"Turn on(off) a timer for logging a process info each X seconds (default is 60sec)")
	ONCOMMAND("KillConf",                         CConfPartyManager::HandleTerminalKillConf,"Kill conference")
	ONCOMMAND("PCHANGE",                          CConfPartyManager::HandleTerminalPartySlowFastChange,"SLOW-FAST CHANGE for party")
	ONCOMMAND("KillParty",                        CConfPartyManager::HandleTerminalKillParty,"Kill party")

	ONCOMMAND("SetNetworkQuality",                CConfPartyManager::HandleNQIndicationIcon, "Set on(off) Network Quality indication icon")

	ONCOMMAND("external_ivr_cache",               CConfPartyManager::HandleIvrFilesCache, "Manage external IVR files cache")

	ONCOMMAND("Set1080p60FR",                     CConfPartyManager::HandleSet1080p60FR, "Set 1080 60 Frame Rate")

	ONCOMMAND("setdebugval",                      CConfPartyManager::HandleSetDebugVal, "Set Debug Param Value")

	ONCOMMAND("get_connectedparties",             CConfPartyManager::HandleGetConnectedPartiesNumber, "Get Connected Parties Number")
	ONCOMMAND("speaker_ind",                      CConfPartyManager::HandleSpeakerInd, "Speaker indication")
	ONCOMMAND("video_recovery",                   CConfPartyManager::HandleVideoRecovery, "Video recovery")//added by Richer for Video Recovery project om 2013.12.26

	ONCOMMAND("SetTelepresenceLayoutMode",         CConfPartyManager::HandleSetTelepresenceLayoutMode, "Set Telepresence Layout Mode")
	ONCOMMAND("LoadReserveGridMapFromXML",         CConfPartyManager::HandleLoadReserveGridMapFromXML, "Load Reserved Grid Map From XML")

END_TERMINAL_COMMANDS


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void ConfPartyManagerEntryPoint(void* appParam)
{
	CConfPartyManager * pConfPartyManager = new CConfPartyManager;
	pConfPartyManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CConfPartyManager::GetMonitorEntryPoint()
{
	return ConfPartyMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CConfPartyManager::GetLobbyEntryPoint()
{
	return LobbyEntryPoint;
}

////////////////////////////////////////////////////////////////////////////
extern "C" void ConfPartyDispatcherEntryPoint(void* appParam)
{
	CConfPartyDispatcherTask * ConfPartyDispatcherTask = new CConfPartyDispatcherTask(FALSE);
	ConfPartyDispatcherTask->Create(*(CSegment*)appParam);
}
////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfPartyManager::CConfPartyManager() //: m_dwSeqNumber(0)
{
	TRACEINTO;

	m_pIVRManager = NULL;
	m_pProcess = (CConfPartyProcess*)CProcessBase::GetProcess();
	m_isConfIpParamsEndReceived = NO;
	m_isConfIsdnParamsEndReceived = NO;
	m_isStartupReadMRDBReqRecieved = NO;
	m_bIsIBMLicense     = FALSE;
	m_isSystemCardsModeReceived = NO;
	m_isNeedToAddGWDefaults	= NO;
	m_isRsrcProcessReady	= NO;
	m_SipProxyDBReqReceived	= NO;
	m_isDefaultGWSessionAdded = NO;
	m_isWebRtcGWStarted = NO; // N.A. DEBUG - Need to change back to NO
	m_isWebRtcIceServerFailure = NO; // N.A. DEBUG
	m_operName = NULL;
	m_lockConfReqCounter = 0;
	m_bLockConfForInvalidCertificate = FALSE;
	m_dwInternalConfStatus = STATUS_OK;
	m_bExchangeConfigured = FALSE;
	m_pMrAndProfileListDuringStartup = NULL;
	m_pAssistMngApi = NULL;
	m_isProfilesFolderEmpty = FALSE;
	m_GMT_offset = 0;
	m_ConfTaskCrashesCounter = 0;
	m_PartyTaskCrashesCounter = 0;

	for (int i = 0; i < eNumOfSlaveSync; i++)
		m_slaveSyncElements[i] = FALSE;

	STATUS retStatus = STATUS_OK;
	retStatus = m_pProcess->GetCustomizeDisplaySettingForOngoingConfConfiguration()->ReadXmlFile(CUSTOMIZE_DISPLAY_SETTING_FOR_ONGGOING_CONF_CFG_FILE,eNoActiveAlarm, eRenameFile);

	if (STATUS_FILE_NOT_EXIST == retStatus || STATUS_OPEN_FILE_FAILED == retStatus) // no file exists (yet) - create a default, hard-coded MngmntNetworkInterface
	{
		if (STATUS_FILE_NOT_EXIST == retStatus)
			TRACEINTO << "Read CUSTOMIZE_DISPLAY_SETTING_FOR_ONGGOING_CONF_CFG_FILE: STATUS_FILE_NOT_EXIST";

		else if (STATUS_OPEN_FILE_FAILED == retStatus)
			TRACEINTO << "Read CUSTOMIZE_DISPLAY_SETTING_FOR_ONGGOING_CONF_CFG_FILE: STATUS_OPEN_FILE_FAILED";

		m_pProcess->GetCustomizeDisplaySettingForOngoingConfConfiguration()->WriteXmlFile(CUSTOMIZE_DISPLAY_SETTING_FOR_ONGGOING_CONF_CFG_FILE, "CUSTOMIZE_DISP_SETUP_ONGOING_CONF");
	}

	ApiObjectsFactoriesRegistrar& r(ApiObjectsFactoriesRegistrar::instance());

	// MCCF
	r.registerFactory(MscIvrMccfPackageApiFactory::const_instance());
	r.registerFactory(CdrCommonApiFactory::const_instance());

	// CDR Persistense
	r.registerFactory(CdrPersistApiFactory::const_instance());

	// Lync 2013
	r.registerFactory(EventPackageInterfaceApiFactory::const_instance());

	m_pProcess->m_NetSettings.LoadFromFile();
}

/////////////////////////////////////////////////////////////////////////////
void* CConfPartyManager::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
CConfPartyManager::~CConfPartyManager()
{
	POBJDELETE (m_pIVRManager);

	if (m_operName)
		delete [] m_operName;

	std::for_each(m_UpdateInfo.begin(), m_UpdateInfo.end(), CUpdateInfo::DeleteObject);
	m_UpdateInfo.clear();

//	CUpdateInfo* pTemp = NULL;
//	std::vector< CUpdateInfo* >::iterator itr =  m_UpdateInfo.begin();
//	while(itr != m_UpdateInfo.end())
//	{
//		pTemp = (*itr);
//		m_UpdateInfo.erase(itr);
//		POBJDELETE(pTemp);
//		itr = m_UpdateInfo.begin();
//	}
	// Erase Cold Move List
	CRsrvParty* pRsrvPartyTemp = NULL;
	std::vector< CRsrvParty* >::iterator itrMove =  m_MoveInfo.begin();
	while(itrMove != m_MoveInfo.end())
	{
		pRsrvPartyTemp = (*itrMove);
		m_MoveInfo.erase(itrMove);
		POBJDELETE(pRsrvPartyTemp);
		itrMove = m_MoveInfo.begin();
	}

	COngoingConfStore* pOngoingConfStore = ONGOING_CONF_STORE;
	if (pOngoingConfStore!=NULL)
		delete pOngoingConfStore;

}
/////////////////////////////////////////////////////////////////////////////
//Creation of all Singleton tasks that are unique for the Conf/Party should be initialized in this function
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::ManagerPostInitActionsPoint()
{
	STATUS status;
	PTRACE(eLevelInfoNormal, "CConfPartyManager::ManagerPostInitActionsPoint");

	CreateLobbyTask();
	CreateConfDB();
	CreateRecordingLinkDB();
	CreateAssistTask();


	// Init IVR Manager (Fill IVR service list and music table)
	POBJDELETE (m_pIVRManager);
	m_pIVRManager = new CIVRManager;  //// DELETE is missing
	m_pIVRManager->InitIVRConfig();	// Read IVR services configuration from disk

	BOOL isOpenSSLFunc = NO;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_OPENSSL_ENC_FUNC, isOpenSSLFunc);
	if (!isOpenSSLFunc)
		InitEncryptionTables(); //Init the algorythim library

  char* profilesFolder = PROFILES_DB_DIR;
  m_isProfilesFolderEmpty = GetDirFilesNum(profilesFolder) ? FALSE : TRUE;
  TRACEINTO << "CConfPartyManager::ManagerPostInitActionsPoint, Is Profiles Folder empty="<<(WORD)m_isProfilesFolderEmpty;

	status = CreateProfilesDB();
	status = CreateMeetingRoomDB();
	status = CreateConfTemplateDB();

	TestAndEnterFipsMode();


	//Get the system based mode from Cards
	SendSystemBasedModeReqToCardMngr();

    if (IsValidPObjectPtr(m_pProcess))
    {
        int dummy = m_pProcess->GetProcessAddressSpace();
        PTRACE2INT (eLevelInfoNormal, "Address Space is %d", dummy);
		m_pProcess->InitializeEncryptionKeysSharedMemory();
    }
    else
        PASSERTMSG(!m_pProcess, "CConfPartyManager::ManagerPostInitActionsPoint Not Valid Process!");


    CreateRoutingTable();

    CPLayoutWrapper wrapper;
    wrapper.LoadGridReservedMapAndCreateXMLIfNeeded(CTelepresenceSpeakerModeLayoutLogic::m_reservedScreenLayoutMap,CTelepresenceCpLayoutLogic::m_gridScreenLayoutMap);

    std::ostringstream answer;
	CProcessBase::GetProcess()->DumpTasks(answer);
	PTRACE2(eLevelInfoNormal, "CConfParty process task list:\n", answer.str().c_str());

	//Send ConfParty's ready to SNMPProcess
	CManagerApi api(eProcessSNMPProcess);
	CSegment *pSeg = new CSegment;
	DWORD type = eProcessConfParty;
	*pSeg << type;
	api.SendMsg(pSeg, SNMP_OTHER_PROCESS_READY);

	//Send PrecedenceSettings request to McuMngr
	CManagerApi apiM(eProcessMcuMngr);
	CSegment *pSegM = new CSegment;
	DWORD typeM = eProcessConfParty;
	*pSegM << typeM;
	apiM.SendMsg(pSegM,MCUMNGR_PRECEDENCE_SETTINGS_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::DeclareStartupConditions()
{
	CActiveAlarm aa(FAULT_GENERAL_SUBJECT,
					AA_NO_IP_SERVICE_PARAMS,
					MAJOR_ERROR_LEVEL,
					"No IP service was received from CSMngr",
					false,
					false);
 	AddStartupCondition(aa);
 	/*CActiveAlarm aa1(FAULT_GENERAL_SUBJECT,
					AA_NO_ISDN_SERVICE_PARAMS,
					SYSTEM_MESSAGE,
					"No ISDN service was received from RTMMngr",
					true,
					true);
 	AddStartupCondition(aa1);
 	if (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily())
 	{
 	    CActiveAlarm aa2(FAULT_GENERAL_SUBJECT,
					AA_NO_DEFAULT_ISDN_SERVICE,
					MAJOR_ERROR_LEVEL,
					"No Default ISDN service was received from RTMMngr",
					true,
					true);
 	AddStartupCondition(aa2);*/
 	CActiveAlarm aa3(FAULT_GENERAL_SUBJECT,
 			         AA_NO_LICENSING,
 					 MAJOR_ERROR_LEVEL,
 					 "Licensing was not received from McuMngr",
 					 false,
 					 false);
    AddStartupCondition(aa3);
 	CActiveAlarm aa4(FAULT_GENERAL_SUBJECT,
 			         AA_NO_READ_MR_DB_REQ_RECIEVED_FROM_RSRC,
 	 	 			 MAJOR_ERROR_LEVEL,
 	 	 			 "No MR DB request was recieved from Resource process",
 	 	 			 false,
 	 	 			 false);
   AddStartupCondition(aa4);
   CActiveAlarm aa5(FAULT_GENERAL_SUBJECT,
 			         AA_SYSTEM_BASED_MODE_NOT_INTIALIZED,
 	 	 			 MAJOR_ERROR_LEVEL,
 	 	 			 "No system based mode indication was received from Cards process",
 	 	 			 false,
 	 	 			 false);
   AddStartupCondition(aa5);



 	//AddStartupCondDependency(AA_NO_LICENSING, AA_SWITCH_NOT_LOADED);
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::ManagerStartupActionsPoint()
{
        BYTE rTransitEqWasFound = NO;

	// ask CentralSignaling to send ConfParty Params
	SendConfIPParamsReqToCS();

	CProcessSettings *pProcessSettings = CProcessBase::GetProcess()->GetProcessSettings();
	string data;
	bool res = pProcessSettings->GetSetting("TRANSIT_EQ_NAME", data);
	PTRACE(eLevelInfoNormal, "ConfPartyManager::ManagerStartupActionsPoint ");

	if(res)
	{

	    ALLOCBUFFER(transitEQName,H243_NAME_LEN);
	    strncpy(transitEQName,data.c_str(),H243_NAME_LEN);
		transitEQName[H243_NAME_LEN - 1] = '\0';

	//	CCommResDB::ReservArray::iterator itr=::GetpMeetingRoomDB()->FindName(transitEQName);
	    CCommResShort* pResShort =::GetpMeetingRoomDB()->GetCurrentRsrvShort(transitEQName);
		if(pResShort)
		{
			::GetpMeetingRoomDB()->SetTransitEQName(transitEQName);//should be in commResDB
			PTRACE(eLevelInfoNormal, "ConfPartyManager::ManagerStartupActionsPoint : Find transitEQ");
			rTransitEqWasFound = YES;
		}
		else
		{
			TRACESTR (eLevelInfoNormal) << "CConfPartyManager::ManagerStartupActionsPoint the EQ: " << transitEQName << "  not exist";
		//	PTRACE(eLevelInfoNormal, "ConfPartyManager::SetTransitEQ : EQ not exist");
			PASSERT(1);

		}
		POBJDELETE(pResShort);
		DEALLOCBUFFER(transitEQName);
	}
	 if((::GetpMeetingRoomDB()->IsNameExist(DEFAULT_EQ_NAME) == YES)  && rTransitEqWasFound == NO)
	 {
	    PTRACE(eLevelInfoNormal, "ConfPartyManager::ManagerStartupActionsPoint : No transitEQ then we put the Default one");
	    CCommResShort* pRsrv =::GetpMeetingRoomDB()->GetCurrentRsrvShort(DEFAULT_EQ_NAME);
	    ALLOCBUFFER(transitEQName,H243_NAME_LEN);
            strncpy(transitEQName,pRsrv->GetName(),H243_NAME_LEN);
			transitEQName[H243_NAME_LEN - 1] = '\0';
            ::GetpMeetingRoomDB()->SetTransitEQName(transitEQName);
            DEALLOCBUFFER(transitEQName);
            POBJDELETE(pRsrv);
	  }
	else
	  PTRACE(eLevelInfoNormal, "ConfPartyManager::ManagerStartupActionsPoint : No DefaultEQ was found");


}

/////////////////////////////////////////////////////////////////////////////
//Overwrite of ManagerTask function ::CreateDispatcher
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::CreateDispatcher()
{
	m_pDispatcherApi = new CTaskApi;
	CreateTask(m_pDispatcherApi, ConfPartyDispatcherEntryPoint, m_pRcvMbx);
//	m_pDispatcherApi->Create(ConfPartyDispatcherEntryPoint,*m_pRcvMbx);
}
//////////////////////////////////////////////////////////////////////
void CConfPartyManager::SelfKill()
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::SelfKill");

	((CTaskApi*)::GetpLobbyApi())->SyncDestroy();

	if(m_pProcess)
	{
		m_pProcess->KillAllConfAndPartyTasks();
		m_pProcess->FreeEncryptionKeysSharedMemory();
	}
	m_pAssistMngApi->SyncDestroy();
	POBJDELETE(m_pAssistMngApi);

	CManagerTask::SelfKill();
}
//////////////////////////////////////////////////////////////////////////
void CConfPartyManager::CreateLobbyTask()
{
	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();

	CLobbyApi* pLobbyApi = new CLobbyApi;
	COsQueue dummyLobbyMbx;

	CreateTask(pLobbyApi, GetLobbyEntryPoint(), &dummyLobbyMbx);
	pConfPartyProcess->SetpLobbyApi(pLobbyApi);

	PTRACE(eLevelInfoNormal, "CConfPartyManager::CreateLobbyTask  Lobby Task is Running and m_pLobbyApi is valid ");
}
//////////////////////////////////////////////////////////////////////////
void CConfPartyManager::CreateAssistTask()
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::CreateAssistTask ");

	if( IsValidPObjectPtr(m_pAssistMngApi) )
		m_pAssistMngApi->Destroy();//POBJDELETE(m_pAssistMngApi);
	else
		m_pAssistMngApi = new CTaskApi;

	CreateTask(m_pAssistMngApi, ConfPartyAssistMngEntryPoint, m_pRcvMbx);

	if( IsValidPObjectPtr(m_pAssistMngApi) )
	{
		CSegment *pDummy = new CSegment;
		*pDummy << "YES";
		*pDummy << PRINT_PROCESS_MEMORY_INFO_TOUT;
		m_pAssistMngApi->SendMsg(pDummy, PROCESS_MEMINFO_PRINT);
	}

}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::CreateRoutingTable()
{
	pConfPartyRoutingTable = new CConfPartyRoutingTable;
}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::CreateConfDB()
{
	pConfDB = new CCommConfDB;
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::CreateProfilesDB()
{
	TRACEINTO;

	if (!g_pProfilesDB)
	{
		g_pProfilesDB = new CCommResDB(PROFILES_DATABASE);
		const string folderName(PROFILES_DB_DIR);
		PASSERTMSG_AND_RETURN_VALUE(!IsFileExists(folderName) && !CreateDirectory(folderName.c_str()), "Cannot create Profiles directory", STATUS_FAIL);

		return g_pProfilesDB->SetFolderPath(folderName);
	}

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::CreateMeetingRoomDB()
{
	TRACEINTO;

	if (!g_pMeetingRoomsDB)
	{
		g_pMeetingRoomsDB = new CCommResDB(MEETING_ROOM_DATABASE);
		const string folderName(FILE_MEETING_ROOM_DB);
		PASSERTMSG_AND_RETURN_VALUE(!IsFileExists(folderName) && !CreateDirectory(folderName.c_str()), "Cannot create Meeting Room directory", STATUS_FAIL);

		return g_pMeetingRoomsDB->SetFolderPath(folderName);
	}

	return STATUS_OK;
}
void CConfPartyManager::CreateOngoingConfStore()
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::CreateOngoingConfStore - begin");
	ONGOING_CONF_STORE->InitOngoingConfStore();
	PTRACE(eLevelInfoNormal,"CConfPartyManager::CreateOngoingConfStore - end");
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::CreateConfTemplateDB()
{
	TRACEINTO;

	if (!g_pConfTemplateDB)
	{
		g_pConfTemplateDB = new CCommResDB(CONF_TEMPLATES_DATABASE);
		const string folderName(FILE_CONF_TEMPLATE_DB);
		PASSERTMSG_AND_RETURN_VALUE(!IsFileExists(folderName) && !CreateDirectory(folderName.c_str()), "Cannot create Conference Templates directory", STATUS_FAIL);

		return g_pConfTemplateDB->SetFolderPath(folderName);
	}

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::CreateRecordingLinkDB()
{
	TRACEINTO;

	if (!pRecordingLinkDB)
	{
		pRecordingLinkDB = new CRecordingLinkDB();
		const string folderName(FILE_RECORDLINK_SRV_DB);
		PASSERTMSG_AND_RETURN_VALUE(!IsFileExists(folderName) && !CreateDirectory(folderName.c_str()), "Cannot create Recording Links directory", STATUS_FAIL);

		return pRecordingLinkDB->SetFolderPath(folderName);
	}

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnLobbyStartAdHocConf(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnLobbyStartAdHocConf");
    char    adHocConfName[H243_NAME_LEN];
    *adHocConfName = '\0';
	char    adHocNumericID[NUMERIC_CONFERENCE_ID_LEN];
	*adHocNumericID = '\0';
    char    adHocConfDisplayName[H243_NAME_LEN];
    *adHocConfDisplayName = '\0';

	WORD	isFromFactory = FALSE, strSize=0;
   	CCommRes* pAdHocRsrv = new CCommRes;

   	CIstrStream istream(*pParam);
	pAdHocRsrv->DeSerialize(NATIVE, istream);
	istream >> isFromFactory;
	istream >> adHocConfName;
	istream >> adHocNumericID;
	istream >> adHocConfDisplayName;
	/*istream >> strSize;
	istream.ignore(1);
  	istream.getline(adHocConfName,strSize+1,'\n');
  	adHocConfName[strSize]='\0';
	istream >> strSize;
   	istream.ignore(1);
   	istream.getline(adHocNumericID,strSize+1,'\n');
   	adHocNumericID[strSize]='\0';*/

	//CLobbyApi* pLobbyApi= ((CLobbyApi*)::GetpLobbyApi()) ;

	pAdHocRsrv->SetAdHocConfParams();

	if(isFromFactory)
	{
		pAdHocRsrv->SetMeetMePerConf(NO);
		pAdHocRsrv->SetName(adHocConfName);
		pAdHocRsrv->SetDisplayName(adHocConfName);
		pAdHocRsrv->FillEmptyDiplayNameOrName();
		pAdHocRsrv->SetNumericConfId(adHocNumericID);
		pAdHocRsrv->SetMonitorConfId(0xFFFFFFFF);
		pAdHocRsrv->SetEntryQ(NO);
		pAdHocRsrv->SetMeetingRoom(NO);
		pAdHocRsrv->SetMeetMePerEntryQ(NO);
		pAdHocRsrv->SetIsAdHocConf(YES);
		pAdHocRsrv->SetSIPFactory(TRUE);
	}

	PTRACE2(eLevelInfoNormal, "CConfPartyManager::OnLobbyStartAdHocConf, Going to Start Conf  ", pAdHocRsrv->GetName());

	//status = CheckRsrvValidity(prsrvfromprofile);

	STATUS status = STATUS_OK;
	if(STATUS_OK == status)
	{
		PTRACE2(eLevelInfoNormal, "CConfPartyManager::OnLobbyStartAdHocConf, Apply to Exchange module to update conf params  ", pAdHocRsrv->GetName());
		STATUS updateRsrvByExchangeProcessStatus = STATUS_OK;
	    updateRsrvByExchangeProcessStatus = SyncExchangeProcessNotification(*pAdHocRsrv);
	}
    status = pAdHocRsrv->TestAddValidity();
    if(status == STATUS_OK)
    {
       if(m_lockConfReqCounter)
       {
          PTRACE2(eLevelInfoNormal, "CConfPartyManager::OnLobbyStartAdHocConf,  Conference establishment is blocked, Conf Name: ",pAdHocRsrv->GetName());

          if (m_bLockConfForInvalidCertificate)
          {
            status = STATUS_CONFERENCE_CREATION_IS_BLOCKED_FOR_INVALID_CERTIFICATE;
          }
          else
          {
            status = STATUS_CONFERENCE_CREATION_IS_BLOCKED;
          }
       }
    }

	if(STATUS_OK == status)
	{
		OPCODE opcode = START_AD_HOC_CONF_REQ;
		CSegment *pToRsrcSeg = new CSegment;

		COstrStream ostrToRsrc;
		pAdHocRsrv->Serialize(NATIVE, ostrToRsrc);

		// VNGR-22659: set the sender position identifier in order to identify ongoing
		// conference restore after reboot
		ostrToRsrc << (WORD)2 << "\n";

		ostrToRsrc.Serialize(*pToRsrcSeg);

		SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, opcode);

	}
    else //validity fail
	{
	  PTRACE2(eLevelInfoNormal, "CConfPartyManager::OnLobbyStartAdHocConf Reject to Lobby, Validation test failed. Conf Name: ",pAdHocRsrv->GetName());
	  RejectMeetingRoomOrAdHocActivation((DWORD)adHocNumericID, status,FALSE,TRUE,(char*)pAdHocRsrv->GetName());
    }


 POBJDELETE(pAdHocRsrv);

}

//////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerAddRsrvContinue(CRequest* pRequest)
{
	STATUS status = STATUS_FAIL;
	status = pRequest->GetStatus();
	TRACEINTO_GLA << "TransactionName = " << pRequest->GetTransName().c_str() << "\nActionName = " << pRequest->GetActionName().c_str() << "status = " << status;

	CCommResAdd* pCommResAdd = new CCommResAdd;
	CCommResApi* pCommResApi = new CCommResApi;

	*pCommResAdd = *(CCommResAdd*)pRequest->GetRequestObject();
	*pCommResApi = *(pCommResAdd->GetCommResApi());

	std::string responseTrancsName("TRANS_RES"); // instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pCommResApi);

	POBJDELETE(pCommResAdd);

	return status;
}

//////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerAddRsrvContinue1(CRequest* pRequest)
{
	STATUS status = STATUS_FAIL;
	status = pRequest->GetStatus();
	TRACEINTO_GLA << "TransactionName = " << pRequest->GetTransName().c_str() << "\nActionName = " << pRequest->GetActionName().c_str() << "status = " << status;

	CCommResAdd* pCommResAdd = new CCommResAdd;
	CCommResApi* pCommResApi = new CCommResApi;

	*pCommResAdd = *(CCommResAdd*)pRequest->GetRequestObject();
	*pCommResApi = *(pCommResAdd->GetCommResApi());

	if (pCommResApi->IsMeetingRoom())
	{
		OnRsrcAddMeetingRoom(pCommResApi);

		std::string responseTrancsName("TRANS_RES"); // instead of TRANS_RES_1
		pRequest->SetTransName(responseTrancsName);
		pRequest->SetConfirmObject(pCommResApi);

		POBJDELETE(pCommResAdd);
		status = STATUS_OK;
		TRACEINTO_GLA << "MR created, return status = STATUS_OK";
	}
	else
	{
		std::string responseTrancsName("TRANS_RES_CONTINUE_1"); // instead of TRANS_RES_1
		pRequest->SetTransName(responseTrancsName);
		pCommResAdd->SetCommResApi(pCommResApi);
		pRequest->SetRequestObject(pCommResAdd);

		POBJDELETE(pCommResApi);
		status = STATUS_FW_REQUEST_TO_RESOURCE;
		TRACEINTO_GLA << "return status = STATUS_FW_REQUEST_TO_RESOURCE";
	}

	return status;
}

//////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerAddRsrvContinue2(CRequest* pRequest)
{
	STATUS status = STATUS_OK;
	status = pRequest->GetStatus();
	TRACEINTO_GLA << "TransactionName = " << pRequest->GetTransName().c_str() << "\nActionName = " << pRequest->GetActionName().c_str() << "status = " << status;

	CCommResAdd* pCommResAdd = new CCommResAdd;
	CCommResApi* pCommResApi = new CCommResApi;

	*pCommResAdd = *(CCommResAdd*)pRequest->GetRequestObject();
	*pCommResApi = *(pCommResAdd->GetCommResApi());
	OnRsrcStartConf(pCommResApi);

	std::string responseTrancsName("TRANS_RES"); // instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pCommResApi);

	POBJDELETE(pCommResAdd);

	return status;
}

STATUS CConfPartyManager::OnServerAddRsrv(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACEINTO << "Failed, No permission (ADMINISTRATOR_READONLY)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;

	CCommRes*    pRsrv       = new CCommRes;
	CCommRes*    pRsrvBackup = new CCommRes;
	CCommResAdd* pCommResAdd = new CCommResAdd;
	CCommConfDB* pCommConfDB = ::GetpConfDB();

	*pCommResAdd = *(CCommResAdd*)pRequest->GetRequestObject();
	*pRsrv = *(pCommResAdd->GetCommResApi());

	pRsrv->FillEmptyDiplayNameOrName();

	TRACEINTO
		<< "ConfName:" << pRsrv->GetName()
		<< ", IsTemplate:" << (WORD)pRsrv->IsTemplate()
		<< ", IsConfTemplate:" << (WORD)pRsrv->IsConfTemplate()
		<< ", IsExclusiveContentMode:" << (WORD)pRsrv->IsExclusiveContentMode()
		<< ", AdHocProfileId:" << pRsrv->GetAdHocProfileId();

	*pRsrvBackup = *pRsrv;

	if (pRsrv->IsConfTemplate() && pRsrv->IsMeetingRoom())
		pRsrv->SetMeetingRoom(NO);

	if (m_lockConfReqCounter && !pRsrv->IsMeetingRoom() && !pRsrv->IsTemplate())
	{
		TRACEINTO << "ConfName:" << pRsrv->GetName() << " - Failed, any conference establishment is blocked";

		if (m_bLockConfForInvalidCertificate)
			status = STATUS_CONFERENCE_CREATION_IS_BLOCKED_FOR_INVALID_CERTIFICATE;
		else
			status = STATUS_CONFERENCE_CREATION_IS_BLOCKED;
	}

	if (STATUS_OK == status)
	{
		if (!pRsrv->IsMeetingRoom() && !pRsrv->IsTemplate() && !pRsrv->IsConfTemplate())
		{
			if (pCommConfDB->GetConfNumber() >= MAX_CONF_IN_LIST)
			{
				TRACEINTO << "ConfName:" << pRsrv->GetName() << " - Failed, any conference establishment is blocked because number of maximum conferences was exceeded";
				status = STATUS_MAX_RESERVATIONS_EXCEEDED;
			}

			if (strlen(pRsrv->GetAppointmentId()))
				SyncExchangeProcessNotification(*pRsrv);
		}
		else if (pRsrv->IsConfTemplate())
		{
			WORD maxConfTemplates = GetMaxConfTemplates();
			if (g_pConfTemplateDB->GetResNumber() >= maxConfTemplates)
			{
				TRACEINTO << "ConfName:" << pRsrv->GetName() << " - Failed, any conference establishment is blocked because number of maximum Conference Templates was exceeded";
				status = STATUS_MAX_CONF_TEMPLATES_EXCEEDED;
			}

			CCommResDB* pProfilesDB = ::GetpProfilesDB();

			const CCommResDB::ReservArray& templatesArray = GetpConfTemplateDB()->GetReservArray();
			CCommResDB::ReservArray::const_iterator itr_end = templatesArray.end();
			for (CCommResDB::ReservArray::const_iterator itr = templatesArray.begin(); itr != itr_end; ++itr)
			{
				if ((*itr) == NULL) continue;

				const char* db_template_name = (*itr)->GetName();
				DWORD profileId = (*itr)->GetAdHocProfileId();

				if (!strncmp(db_template_name, pRsrv->GetName(), H243_NAME_LEN))
				{
					TRACEINTO << "ConfName:" << pRsrv->GetName() << " - Failed, Conference template is rejected because a template by the same name already exists";
					status = STATUS_RESERVATION_NAME_EXISTS;
					break;
				}
			}

			if (STATUS_OK == status) // template name is unique - check for existing profile name
			{
				CCommResShort* db_profile = NULL;
				if (strlen(pRsrv->GetBaseProfileName()) > 0)
				{
					const CCommResDB::ReservArray& profilesArray = GetpProfilesDB()->GetReservArray();
					CCommResDB::ReservArray::iterator rsrv_it = GetpProfilesDB()->FindName(pRsrv->GetBaseProfileName(), TRUE);
					if (rsrv_it != profilesArray.end())
						db_profile = *rsrv_it;

					// db_profile =
					if (db_profile != NULL)
					{
						DWORD db_profile_id = db_profile->GetConferenceId();
						pRsrv->SetAdHocProfileId(db_profile_id);
						TRACEINTO << "ProfileId:" << db_profile_id << " - Conference template is updated with profile id";
					}
					else if (pRsrv->GetAdHocProfileId() == 0xFFFFFFFF)
					{
						TRACEINTO << "ProfileName:" << pRsrv->GetBaseProfileName() << " - Failed, No profile found for Conference template";
						status = STATUS_PROFILE_NOT_FOUND;
					}
				}
			}
		}
	}

	if (STATUS_OK == status)
	{
		if (FALSE == m_bIsIBMLicense)
		{
			if (!strncmp(pRsrv->GetName(), "RAS200I_", 8)) // if not IBM environment - any conference with prefix "RAS200I_" will be blocked
			{
				TRACEINTO << "ConfName:" << pRsrv->GetName() << " - Failed, Any conference establishment with prefix 'RAS200I_' is blocked because not IBM environment";
				status = STATUS_FAIL;
			}
		}
	}

	if (STATUS_OK == status)
	{
		// Romem - 21.7.08 According to Assaf W. decision - MR/EQ  and Regular Conferences are treated the same
		if (pRsrv->IsTemplate())
		{
			status = pRsrv->TestAddValidity();
		}
		else // for a conference/reservation  test validity on the copy of the original object
		{
			if (STATUS_OK == status)
			{
				// First check is for Auto Correction of Reservation parameters which are not profile based.
				status = pRsrv->TestReservValidityOfCommonParams();
				status = pRsrvBackup->TestAddValidity();

				pRsrv->SetMonitorIdForAllParties();   // Romem - 21.07.08 - Allocate Party Monitor Id on "START" OnGoing Conference.
				pRsrv->SetOperatorConfFromProfile();  // create operator conference
				if (pRsrv->GetOperatorConf())
				{
					if (STATUS_OK == status)
						status = pRsrv->TestOperatorConfValidity(GetLoginName().c_str());

					if (STATUS_OK == status)
						pRsrv->SetOperatorConfInfo();
				}
			}
		}
	}

	if ((STATUS_OK == status) || (status & WARNING_MASK))
	{
		AutoGeneratePasswords(pRsrv);

		if (pRsrv->IsTemplate() || pRsrv->IsConfTemplate())
		{
			if ((SUPER == pRequest->GetAuthorization()) || (ORDINARY == pRequest->GetAuthorization()))
			{
				if (pRsrv->IsTemplate())
				{
					STATUS addProfStatus = AddProfile(pRsrv);
					if (addProfStatus == STATUS_OK)
					{
						CSegment* pRASeg = new CSegment;
						CResRsrcCalculator rsrcCalc;
						PROFILE_IND_S profile_ind_s;
						profile_ind_s.profile_Id = pRsrv->GetMonitorConfId();
						profile_ind_s.maxVideoPartyType = rsrcCalc.GetRsrcVideoType(GetSystemCardsBasedMode(), pRsrv);
						TRACEINTO << "ConfName:" << pRsrv->GetName() << ", ProfileVideoType:" << eVideoPartyTypeNames[profile_ind_s.maxVideoPartyType] << " - Sending Profile weight to Resource process";
						pRASeg->Put((BYTE*)&profile_ind_s, sizeof(PROFILE_IND_S));
						SendAsyncMsgToRsrcProcess(pRASeg, PROFILE_ADD_RSRC_IND);
					}
					else
						status = addProfStatus;
				}
				else if (pRsrv->IsConfTemplate())
					status = AddConfTemplate(pRsrv);
			}
			else
			{
				TRACEINTO << "ConfName:" << pRsrv->GetName() << " - Failed, No permission to add profiles/Conf Templates";
				pRequest->SetConfirmObject(new CDummyEntry());
				pRequest->SetStatus(STATUS_NO_PERMISSION);
				POBJDELETE(pCommResAdd);
				POBJDELETE(pRsrv);
				POBJDELETE(pRsrvBackup);
				return STATUS_OK;
			}
		}
		else
		{
			pRsrv->SetHDVSW(pRsrvBackup->GetIsHDVSW()); // Serg
			pRsrv->SetIsCOPReservation(pRsrvBackup->IsCOPReservation()); // 2 modes cop/cp

			for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
			{
				const char* Service_nm = pRsrvBackup->GetServiceRegistrationContentServiceName(i);
				if (Service_nm[0] != '\0')
				{
					pRsrv->SetServiceRegistrationContentServiceName(i, pRsrvBackup->GetServiceRegistrationContentServiceName(i)); // In multiple services add services
					pRsrv->SetServiceRegistrationContentRegister(i, pRsrvBackup->GetServiceRegistrationContentRegister(i));      // In multiple services add services
					pRsrv->SetServiceRegistrationContentAcceptCall(i, pRsrvBackup->GetServiceRegistrationContentAcceptCall(i));  // In multiple services add services
					pRsrv->SetServiceRegistrationContentStatus(i, pRsrvBackup->GetServiceRegistrationContentStatus(i));          // In multiple services add services - sipProxySts
				}
			}

			pRsrv->SetSipRegistrationTotalSts(pRsrvBackup->GetSipRegistrationTotalSts()); // sipProxySts

			pCommResAdd->SetCommResApi(pRsrv);
			pRequest->SetRequestObject(pCommResAdd); // Set the new Object to be Send to Resources

			STATUS confStatus = pRsrvBackup->GetInternalConfStatus();
			if (STATUS_OK != confStatus)
				m_dwInternalConfStatus = confStatus;   // olga (VNGR-8621)

			POBJDELETE(pRsrv);                    // pCommResAdd deleted by skeleton
			POBJDELETE(pRsrvBackup);
			TRACEINTO_GLA << "STATUS_FW_REQUEST_TO_RESOURCE - return";
			return STATUS_FW_REQUEST_TO_RESOURCE;
		}
	}

	// We will be here in the following cases
	// 1.TestValidity failed
	// 2.ADD Profile
	pCommResAdd->SetCommResApi(pRsrv);
	pRequest->SetConfirmObject(pCommResAdd);
	pRequest->SetStatus(status);
	TRACEINTO << "ConfName:" << pRsrv->GetName() << ", Status:" << (int)status;

	POBJDELETE(pRsrv);
	POBJDELETE(pRsrvBackup);
	TRACEINTO_GLA << "STATUS_OK - return";
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerAddRepeatedRsrv(CRequest* pRequest)
{
	//tbd zoe - by confparty: maybe merge two codes of AddRsrv and AddRepeatedRsrv ???
	//and also remove code that has to do with profiles or meeting rooms, they shouldn't arrive here...

	//Send Reservation Module AddRsrv
 	//When Reservation time is up , Reservation Module will send a message to StartConf
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
       FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerAddRepeatedRsrv: No permission to OnServerAddRepeatedRsrv for administrator readony");
       pRequest->SetConfirmObject(new CDummyEntry());
       pRequest->SetStatus(STATUS_NO_PERMISSION);
       return STATUS_NO_PERMISSION;
    }

    STATUS status = STATUS_OK;

	CCommResApi* pRsrvApi = new CCommResApi;
	CCommResApi* pRsrvBackupApi = new CCommResApi;
	CCommResAdd* pCommResAdd = new CCommResAdd;
	CCommConfDB* pCommConfDB = ::GetpConfDB();

	*pCommResAdd = *(CCommResAdd*)pRequest->GetRequestObject() ;
	*pRsrvApi= *(pCommResAdd->GetCommResApi());

	if (pRsrvApi->IsPermanent()) //Permanent Conf
	{
		PTRACE(eLevelInfoNormal, "CConfPartyManager::OnServerAddRepeatedRsrv,  Permanent conf can't be repeated");
		status= STATUS_FAIL;
		pRequest->SetConfirmObject(pCommResAdd);
		pRequest->SetStatus(status);

		POBJDELETE(pRsrvApi);  //pCommResAdd deleted by skeleton
		POBJDELETE(pRsrvBackupApi);
		return status;
	}

//ANY VALIDITY TEST  FOR OnServerAddRsrv SHOUND BE ADDED IN TestAddValidity() (and not in this function).
// Romem - 4.2.07 - do not test validity for MR/EQ
	(static_cast<CCommRes*>(pRsrvApi))->FillEmptyDiplayNameOrName();
	// create a full copy of repeated
	 *pRsrvBackupApi=*pRsrvApi;

	if(m_lockConfReqCounter && (!pRsrvApi->IsMeetingRoom ()) && (!pRsrvApi->IsTemplate ()))
	{
		PTRACE2(eLevelInfoNormal, "CConfPartyManager::OnServerAddRepeatedRsrv,  Any conference establishment is blocked, Conf Name: ",pRsrvApi->GetName());

		if (m_bLockConfForInvalidCertificate)
		{
			status = STATUS_CONFERENCE_CREATION_IS_BLOCKED_FOR_INVALID_CERTIFICATE;
		}
		else
		{
			status = STATUS_CONFERENCE_CREATION_IS_BLOCKED;
		}
	}


	if(STATUS_OK == status)
	{
		if(FALSE == m_bIsIBMLicense)
		{
			if( !strncmp(pRsrvApi->GetName(),"RAS200I_",8) ) //if not IBM environment - any conf with prefix "RAS200I_" will be blocked
			{
				PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerAddRepeatedRsrv: Send Faile to Add Rsrv To The Resources. ConfName=",pRsrvApi->GetName());
				status = STATUS_FAIL;
			}
		}
	}

	if(STATUS_OK == status)
	{
		// First check is for Auto Correction of Reservation parameters which are not profile based.
		  (static_cast<CCommRes*>(pRsrvApi))->TestReservValidityOfCommonParams();
		// Test validity of full repeated with profile parameters
		   status = (static_cast<CCommRes*>(pRsrvBackupApi))->TestAddValidity();
		   // Romem - 21.07.08 - Allocate Party Monitor Id on Repeated scheduling Conference
		   (static_cast<CCommRes*>(pRsrvApi))->SetMonitorIdForAllParties();
	}

	if(STATUS_OK == status)
	{
		    PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerAddRepeatedRsrv: Send Add Repeated Rsrv To The Resources. ConfName=",pRsrvApi->GetName());
		    pRsrvApi->SetIsCOPReservation(pRsrvBackupApi->IsCOPReservation());//2 modes cop/cp
			pCommResAdd->SetCommResApi(pRsrvApi);
			pRequest->SetRequestObject(pCommResAdd); // Set the new Object to be Send to Resources

			POBJDELETE(pRsrvApi);  //pCommResAdd deleted by skeleton
			POBJDELETE(pRsrvBackupApi);
			return STATUS_FW_REQUEST_TO_RESOURCE;
	}


	//We will be here in the following cases
    //  1.TestValidity failed
	//  2.ADD Profile
	pCommResAdd->SetCommResApi(pRsrvApi);
	pRequest->SetConfirmObject(pCommResAdd);
	pRequest->SetStatus(status);

	POBJDELETE(pRsrvApi);  //pCommResAdd deleted by skeleton
	POBJDELETE(pRsrvBackupApi);
	return status;
}

//--------------------------------------------------------------------------
void CConfPartyManager::OnRsrcStartConf(CCommResApi* pCommResApi)
{
	STATUS status = STATUS_OK;

	char dialString[MaxAddressListSize];
	memset(dialString, '\0', MaxAddressListSize);

	char ISDNServiceNameStr[NET_SERVICE_PROVIDER_NAME_LEN];
	memset(ISDNServiceNameStr, '\0', NET_SERVICE_PROVIDER_NAME_LEN);

	CCommRes* pRsrv = new CCommRes(*pCommResApi);

	ConfMonitorID confMonitorID = pRsrv->GetMonitorConfId();
	BOOL IsAdHocConf = pRsrv->GetIsAdHocConf();

	TRACEINTO << "MonitorConfId:" << confMonitorID << ", DialString:" << dialString << ", ISDNServiceName:" << ISDNServiceNameStr << ", Status:" << status;

	// Zoe: in some cases Rsrc sends an error status to ConfParty, and this is OK!!!
	// For example if there's a request for starting a new Ad-Hoc conference, but the maximum number of ongoing conference has been reached
	// or in any other case of resources collision
	// TBD: maybe this should be send to IVR task, so they should play an appropriate message

	if (STATUS_OK == status && 0xFFFFFFFF != confMonitorID)
	{
		if (pRsrv->GetOperatorConf())
			pRsrv->SetOperatorConfInfo();

		pRsrv->InitPartiesMoveInfo();

		status = pRsrv->TestAddValidity();

		if (STATUS_OK == status)
		{
			// Set the Dial-in prefix for the new conference
			pRsrv->SetConfDialinPrefix();

			// Fill empty routing name after numeric id had been allocated
			pRsrv->FillEmptyDiplayNameOrName();

			const char* appointementId = pRsrv->GetAppointmentId();
			if (strlen(appointementId))
			{
				if (strlen(pRsrv->GetMeetingOrganizer()) == 0)
				{
					CCommResApi* pCommResApi = new CCommResApi(*pRsrv);
					STATUS updateRsrvByExchangeProcessStatus = SyncExchangeProcessNotification(*pCommResApi);
					if (updateRsrvByExchangeProcessStatus == STATUS_OK)
						*pRsrv = *pCommResApi;
					POBJDELETE(pCommResApi);
				}
			}

			TRACEINTO_GLA << "Starting OnGoingConf";
			StartOnGoingConf(*pRsrv, dialString, ISDNServiceNameStr);
		}
	}

	if (STATUS_OK != status)
	{
		TRACESTRFUNC(eLevelError) << "MonitorConfId:" << confMonitorID << ", IsAdHocConf:" << (int)IsAdHocConf << ", Status:" << status << " - Failed, Conference will not start";

		if (IsAdHocConf)
			RejectMeetingRoomOrAdHocActivation((DWORD)confMonitorID, STATUS_ILLEGAL, FALSE, TRUE, (char*)pRsrv->GetName());

		std::ostringstream msg;
		msg << "Reservation can't be OnGoing because of wrong parameters, ConfName:" << pRsrv->GetName() << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, ILLEGAL_CONFERENCE_PARAMETERS, MAJOR_ERROR_LEVEL, msg.str().c_str(), FALSE);

		CSegment* pSeg = new CSegment;
		*pSeg << confMonitorID;
		SendMsgToRsrvMngrRsrcProcess(pSeg, DEL_RSRV_CONF_REQ);
	}

	POBJDELETE(pRsrv);
}

//--------------------------------------------------------------------------
void CConfPartyManager::OnRsrcStartConfInd(CSegment* pMsg)
{
	STATUS status = STATUS_OK;

	char dialString[MaxAddressListSize];
	memset(dialString, '\0', MaxAddressListSize);

	char ISDNServiceNameStr[NET_SERVICE_PROVIDER_NAME_LEN];
	memset(ISDNServiceNameStr, '\0', NET_SERVICE_PROVIDER_NAME_LEN);

	CCommRes* pRsrv = new CCommRes;
	CIstrStream istr(*pMsg);

	pRsrv->DeSerialize(NATIVE, istr);
	istr >> status; // Read the Status from the Resource RsrvManager

	istr.ignore(1);
	WORD needToSendService = 0;
	istr >> needToSendService;
	if (needToSendService)
	{
		istr.ignore(1);
		istr.getline(ISDNServiceNameStr, NET_SERVICE_PROVIDER_NAME_LEN);
	}
	else
	{
		istr.ignore(1);
	}
	istr.getline(dialString, MaxAddressListSize);

	ConfMonitorID confMonitorID = pRsrv->GetMonitorConfId();
	BOOL IsAdHocConf = pRsrv->GetIsAdHocConf();

	TRACEINTO << "MonitorConfId:" << confMonitorID << ", DialString:" << dialString << ", ISDNServiceName:" << ISDNServiceNameStr << ", Status:" << status;

	// Zoe: in some cases Rsrc sends an error status to ConfParty, and this is OK!!!
	// For example if there's a request for starting a new Ad-Hoc conference, but the maximum number of ongoing conference has been reached
	// or in any other case of resources collision
	// TBD: maybe this should be send to IVR task, so they should play an appropriate message

	if (STATUS_OK == status && 0xFFFFFFFF != confMonitorID)
	{
		if (pRsrv->GetOperatorConf())
			pRsrv->SetOperatorConfInfo();

		pRsrv->InitPartiesMoveInfo();

		status = pRsrv->TestAddValidity();

		if (STATUS_OK == status)
		{
			// Set the Dial-in prefix for the new conference
			pRsrv->SetConfDialinPrefix();

			// Fill empty routing name after numeric id had been allocated
			pRsrv->FillEmptyDiplayNameOrName();

			const char* appointementId = pRsrv->GetAppointmentId();
			if (strlen(appointementId))
			{
				if (strlen(pRsrv->GetMeetingOrganizer()) == 0)
				{
					CCommResApi* pCommResApi = new CCommResApi(*pRsrv);
					STATUS updateRsrvByExchangeProcessStatus = SyncExchangeProcessNotification(*pCommResApi);
					if (updateRsrvByExchangeProcessStatus == STATUS_OK)
						*pRsrv = *pCommResApi;
					POBJDELETE(pCommResApi);
				}
			}

			StartOnGoingConf(*pRsrv, dialString, ISDNServiceNameStr);
		}
	}

	if (STATUS_OK != status)
	{
		TRACESTRFUNC(eLevelError) << "MonitorConfId:" << confMonitorID << ", IsAdHocConf:" << (int)IsAdHocConf << ", Status:" << status << " - Failed, Conference will not start";

		if (IsAdHocConf)
			RejectMeetingRoomOrAdHocActivation((DWORD)confMonitorID, STATUS_ILLEGAL, FALSE, TRUE, (char*)pRsrv->GetName());

		std::ostringstream msg;
		msg << "Reservation can't be OnGoing because of wrong parameters, ConfName:" << pRsrv->GetName() << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, ILLEGAL_CONFERENCE_PARAMETERS, MAJOR_ERROR_LEVEL, msg.str().c_str(), FALSE);

		CSegment* pSeg = new CSegment;
		*pSeg << confMonitorID;
		SendMsgToRsrvMngrRsrcProcess(pSeg, DEL_RSRV_CONF_REQ);
	}

	POBJDELETE(pRsrv);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::StartOnGoingConf(CCommRes& rsrv, char* dialString, char* isdnServiceName)
{
	STATUS status = STATUS_OK;

	CStructTm curTime;
	STATUS timeStatus = SystemGetTime(curTime);
	rsrv.SetStartTime(curTime);
	rsrv.SetEndTime();

	if ( rsrv.IsTemplate() == FALSE && rsrv.IsConfTemplate() == FALSE && rsrv.GetDisplayName()[0] !='\0' && curTime.m_year!=0)
	{
		rsrv.SetCorrelationId();
		TRACEINTO << "Correlation Id: " << rsrv.GetCorrelationId();
	}

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	CConfApi* pConfApi = new CConfApi(rsrv.GetMonitorConfId());
#else
	CConfApi* pConfApi = new CConfApi;
#endif
	pConfApi->Create(ConfEntryPoint, GetRcvMbx(), rsrv.GetMonitorConfId(), (char*)(rsrv.GetName()));

	// If there were conf task crashes, print the number of crashes to log
	TRACECOND(m_ConfTaskCrashesCounter > 0, "ConfTaskCrashesCounter:" << m_ConfTaskCrashesCounter);

	// Check if we exceeded the number of ongoing conferences and we have a leak in m_TaskIdToConfId map.
	if (m_TaskIdToConfId.size() > ::GetpConfDB()->GetConfNumber())
	{
		PASSERTMSGONCE(m_TaskIdToConfId.size(), "CConfPartyManager::StartOnGoingConf - Number of conferences on map out of sync!");
	}

	// Add the conference to the TaskId-ConfId map
	ConfMonitorID conf_id = rsrv.GetMonitorConfId();
	DWORD task_id = pConfApi->GetTaskAppPtr()->GetTaskId();
	m_TaskIdToConfId[task_id] = conf_id;

	// Adding the Conf to DB here (changes in Design due to conf spreading)
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(conf_id);

	BOOL bNeedToAddToDB = TRUE;
	if (pCommConf)
		bNeedToAddToDB = FALSE;
	else
		pCommConf = new CCommConf(rsrv);

	PASSERTMSG(!pCommConf, "Failed to get CCommConf!");

	if ((CProcessBase::GetProcess()->GetProductFamily() == eProductFamilySoftMcu && pCommConf->GetDisplayName() && strstr(pCommConf->GetDisplayName(), "##FORCE_CG")) ||
		(CProcessBase::GetProcess()->GetProductType() == eProductTypeCallGeneratorSoftMCU))
	{
		pCommConf->SetIsCallGeneratorConference(TRUE);
	}

	// Set the conf MailBox
	const COsQueue& confApiMailBox  = pConfApi->GetRcvMbx();
	COsQueue* pConfRcvMailBox = new COsQueue(confApiMailBox);
	pCommConf->SetRcvMbx(pConfRcvMailBox);

	if (bNeedToAddToDB)
	{
		int add_result = ::GetpConfDB()->Add(*pCommConf);
		if (STATUS_OK != add_result)
		{
			PASSERTMSG(add_result, "Failed on Add Conference to DB");
			POBJDELETE(pCommConf);
			pConfApi->ForceKill(); // Killing the Task
			POBJDELETE(pConfApi);
			return STATUS_FAIL;
		}
	}

	pCommConf->SetEPCContentSourceId(0xffffffff);
	pCommConf->UpdateParamsIfSlaveExistInConf();
	pCommConf->StartConference(m_GMT_offset, m_GMT_offsetSign); // CDR update

	TRACEINTO << "MonitorConfId:" << conf_id << ", IsGateway:" << (WORD)pCommConf->GetIsGateway();

	pConfApi->StartConf(rsrv);

	TRACEINTO_GLA << "before ONGOING_CONF_STORE->Add";
	ONGOING_CONF_STORE->Add(&rsrv);
	TRACEINTO_GLA << "after ONGOING_CONF_STORE->Add";

	if (pCommConf->GetIsGateway())
	{
		if (isdnServiceName && strlen(isdnServiceName))
			pConfApi->SetDialOutServiceNameForGW(isdnServiceName);

		if (dialString && strlen(dialString))
			pConfApi->SetDialStringForGW(dialString);
	}

	pConfApi->DestroyOnlyApi();
	POBJDELETE(pConfApi);

	TRACEINTO_GLA << "return status = " << status;
	return status;
}

//=====================================================================================================================================//
STATUS CConfPartyManager::OnServerTerminateConf(CRequest *pRequest)
{
  if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
  {
       FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerTerminateConf: No permission to OnServerTerminateConf for administrator readony");
       pRequest->SetConfirmObject(new CDummyEntry());
       pRequest->SetStatus(STATUS_NO_PERMISSION);
       return STATUS_NO_PERMISSION;
  }

  CConfAction* pConfAction = new CConfAction;
  *pConfAction = *(CConfAction*)pRequest->GetRequestObject();

  int status = STATUS_OK;

  const DWORD confID = pConfAction->GetConfID();

  if (::GetpConfDB()->FindId(confID) == NOT_FIND)
    status = STATUS_CONF_NOT_EXISTS;

  if (status == STATUS_OK)
  {
    CCommConf* pCurCommConf = (CCommConf*)::GetpConfDB()->GetCurrentConf(confID);

    if (!CPObject::IsValidPObjectPtr(pCurCommConf))
    {
      DBGPASSERT(1);
      status = STATUS_CONF_NOT_EXISTS;
    }
    else
    {
      PTRACE2(eLevelInfoNormal, "ConfPartyManager::OnServerTerminateConf - ConfName:",  pCurCommConf->GetName());

      if (pCurCommConf->Is_TerminatingState())
        status = STATUS_TERMINATING_CONFERENCE;

      // Delete operator party if he moved to other conference
      if (pCurCommConf->GetOperatorConf())
      {
        COperatorConfInfo* pOperatorConfInfo = pCurCommConf->GetOperatorConfInfo();
        if (pOperatorConfInfo != NULL)
        {
          string operatorPartyName = pOperatorConfInfo->GetOperatorPartyName();

          // Search for operator party in his operator conference
          CRsrvParty* pOperatorParty = pCurCommConf->GetCurrentParty(operatorPartyName.c_str());
          if (CPObject::IsValidPObjectPtr(pOperatorParty))
          {
            // Operator party found in his operator conference and will be deleted with the conference (do nothing)
            CSmallString sstr;
            sstr << "operator party: " << operatorPartyName.c_str() << " found in his operator conference: " << pCurCommConf->GetName();
            PTRACE2(eLevelInfoNormal, "ConfPartyManager::OnServerTerminateConf - ", sstr.GetString());
          }
          else
          {
            CCommConf* pFoundCommConf = NULL;
            CConfParty* pFoundConfParty = NULL;
            BYTE isOperatorPartyFound = ::GetpConfDB()->SearchOperatorParty(*pOperatorConfInfo, pFoundConfParty, pFoundCommConf);
            if (isOperatorPartyFound && pFoundConfParty != NULL && pFoundCommConf != NULL)
            {
              // Operator party found in another conference (delete it from this conference)
              CSmallString sstr;
              sstr << "operator party: " << operatorPartyName.c_str() << " found in conference: " << pFoundCommConf->GetName() << " - deleting party";
              PTRACE2(eLevelInfoNormal, "ConfPartyManager::OnServerTerminateConf - ", sstr.GetString());

              // drop party
              CConfApi confApi;
              confApi.CreateOnlyApi(*(pFoundCommConf->GetRcvMbx()));
              confApi.DropParty(pFoundConfParty->GetName());
              confApi.DestroyOnlyApi();
              // cdr
              ((CCommConf*)pFoundCommConf)->OperatorPartyAction(pFoundConfParty->GetName(), pFoundConfParty->GetPartyId(), m_operName, OPERATORS_DELETE_PARTY);
            }
            else
            {
              CSmallString sstr;
              sstr << "operator party: " << operatorPartyName.c_str() << " not found";
              PTRACE2(eLevelInfoNormal, "ConfPartyManager::OnServerTerminateConf - ", sstr.GetString());
            }
          }
        }
      }

      if (status == STATUS_OK)
      {
        //cdr
        ((CCommConf*)pCurCommConf)->OperatorTerminate(m_operName);
        ((CCommConf*)pCurCommConf)->SetStatus(CONFERENCE_TERMINATING);

        // Destroy on-going conference
        CConfApi confApi;
        confApi.CreateOnlyApi(*(pCurCommConf->GetRcvMbx()));
        confApi.Destroy();
        status = STATUS_IN_PROGRESS;

      }
    }
  }

  std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_CONF_2
  pRequest->SetTransName(responseTrancsName);
  pRequest->SetConfirmObject(pConfAction);
  pRequest->SetStatus(status);

  return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnRsrcDelRsrvConfInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnRsrcDelRsrvConfInd: CONF was deleted in Resource alloc ");
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnRsrcDelMRInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnRsrcDelMRInd: MR was deleted in Resource alloc ");
}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnRsrcDeActivateMeetingRoomInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnRsrcDeActivateMeetingRoomInd: MR was deactivate in Resource alloc ");
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnRsrcUpdateMeetingRoomInd(CSegment* pMsg)
{
  TRACESTR(eLevelInfoNormal) << "ConfPartyManager::OnRsrcUpdateMeetingRoomInd Update a specified MR" ;

  STATUS status=STATUS_OK;
  CCommRes* pRsrv = new CCommRes;

  pRsrv->DeSerialize(NATIVE, *pMsg);

  //Set the monitor Id of all parties
  pRsrv->SetMonitorIdForAllParties();

  //Set the Dial-in prefix for the new conference
  pRsrv->SetConfDialinPrefix();

  status=UpdateMR(pRsrv);

  POBJDELETE(pRsrv);

  if (STATUS_OK != status){
    TRACESTR(eLevelError) << "ConfPartyManager::OnRsrcUpdateMeetingRoomInd Update the MR was Failed !!!!" ;
    PASSERT(1);
    return;
  }

}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnRsrcSetConferenceEndTimeInd(CSegment* pMsg)
{
	STATUS status = STATUS_OK;
	DWORD confId;
	bool isSetByOperator;

	SET_CONFERENCE_ENDTIME_IND_PARAMS_S* pEndTimeData = new SET_CONFERENCE_ENDTIME_IND_PARAMS_S;
	pMsg->Get((BYTE*)(pEndTimeData),sizeof(SET_CONFERENCE_ENDTIME_IND_PARAMS_S));
	status = (STATUS)pEndTimeData->status;
	confId = pEndTimeData->monitorConfId;
	isSetByOperator = pEndTimeData->isSetByOperator;

	CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);

	if (!CPObject::IsValidPObjectPtr(pCurConf))
	{
		delete pEndTimeData;
	}

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pCurConf));

	if(status!=STATUS_OK)
	{
		TRACEINTO << "Conference time was not extended by Resource process, Name:" << pCurConf->GetName();
		delete pEndTimeData;

		return;
	}

	CStructTm* pTime = new CStructTm(pEndTimeData->newEndTime);

	TRACEINTO << "New End Conference Time recieved from Rsrc module, Conf Name:" << pCurConf->GetName();

	if (isSetByOperator)
	{
		pCurConf->OperatorSetEndTime(*pTime, m_operName);
	}
	else
	{
		pCurConf->OperatorSetEndTime(*pTime,"AUTO_EXTENSION");
	}

	// Send event to Conference
	CConfApi confApi;
	confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
	confApi.SetEndTime(*pTime);
	confApi.DestroyOnlyApi();
	POBJDELETE(pEndTimeData);
	POBJDELETE(pTime);
}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnRsrcReadyInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnRsrcReadyInd");
	m_isRsrcProcessReady = true;
	if (m_isNeedToAddGWDefaults)
		CreateDefaultsGWSessionAndSendToRsrc();


}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnRsrcUpdateNumericIdInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnRsrcUpdateNumericIdInd");
	ON(m_isDefaultGWSessionAdded);

	PREFERRED_NUMERIC_ID_S preferedNidStruct;
	memset(&preferedNidStruct,0,sizeof(PREFERRED_NUMERIC_ID_S));
	pMsg->Get((BYTE*)(&preferedNidStruct),sizeof(PREFERRED_NUMERIC_ID_S));

	char* mrName = preferedNidStruct.conf_name;
	STATUS status = preferedNidStruct.status;
	BYTE generateFault = FALSE;
	CMedString description;
	if (status != STATUS_OK)
	{
		int del_stat = ::GetpMeetingRoomDB()->Cancel(mrName);
		PASSERT(del_stat);

		description << " Could not add default gw session  status: " << (CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		generateFault = TRUE;
	}
	else
	{
		CCommRes* pDefaultGw = ::GetpMeetingRoomDB()->GetCurrentRsrv(mrName);
		if (pDefaultGw)
		{
			if (strncmp(pDefaultGw->GetNumericConfId(),preferedNidStruct.numeric_Id,NUMERIC_CONF_ID_MAX_LEN))
			{
				description << " Could not add default gw session with numeric id  " << pDefaultGw->GetNumericConfId() << " New numeric id: " << preferedNidStruct.numeric_Id;
				generateFault = TRUE;
			}

			pDefaultGw->SetMonitorConfId(preferedNidStruct.monitor_Id);
			pDefaultGw->SetNumericConfId(preferedNidStruct.numeric_Id);

			::GetpMeetingRoomDB()->Update(*pDefaultGw);


			POBJDELETE(pDefaultGw);
		}
		else
		{
			PASSERT(1);
			description << "CConfPartyManager::OnRsrcUpdateNumericIdInd can't find default GW in DB (name: " << mrName <<")";
			generateFault = TRUE;
		}
	}

	if (generateFault)
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,ILLEGAL_CONFERENCE_PARAMETERS,MAJOR_ERROR_LEVEL,description.GetString(), FALSE);

	// the DB is updated -> send ind to sip proxy (if needed)
	if (m_SipProxyDBReqReceived)
		OnSipProxyDBReq(NULL);


}
/////////////////////////////////////////////////////////////////////////////
/////////////      Meeting Room    //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnRsrcAddMeetingRoomInd(CSegment* pMsg)
{
  PTRACE(eLevelInfoNormal, "ConfPartyManager::OnRsrcAddMeetingRoomInd: ADD new MR To MeetingRoomDB recieved from Rsrc module  ");

  STATUS status = STATUS_OK;

  CCommRes* pMRoom= new CCommRes;
  pMRoom->DeSerialize(NATIVE, *pMsg);

  DWORD MRMonitorID = pMRoom->GetMonitorConfId();
  CSegment *pToRsrcSeg = new CSegment;
  *pToRsrcSeg << MRMonitorID ;

  //Set the monitor Id of all parties
  //(this is setting only for Adding MR reservations)
  pMRoom->SetMonitorIdForAllParties();

  //Set the Dial-in prefix for the new conference
  pMRoom->SetConfDialinPrefix();
  // fill empty routing name after numeric id had been allocated
  pMRoom->FillEmptyDiplayNameOrName();

  //Add a new meeting room to the MR DB
  status= ::GetpMeetingRoomDB()->Add(*pMRoom);
  if ( status != STATUS_OK)
    {
      PTRACE2(eLevelError, "CConfPartyManager::OnRsrcAddMeetingRoomInd :  Failed Adding a new meeting room to templates DB, Conf Name: ",pMRoom->GetName());

      SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, DEL_MR_REQ);
      PASSERT(status);//Answer Server Dont Know How!!!
    }

  else //(status == STATUS_OK)
    {
      PTRACE2(eLevelError, "CConfPartyManager::OnRsrcAddMeetingRoomInd :  Adding successfully a new meeting room to templates DB, Conf Name: ",pMRoom->GetName());
      POBJDELETE(pToRsrcSeg);
      CSipProxyManagerApi SipProxyApi;
		CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
		CConfIpParameters* pServiceParams = NULL;
		for (int i = 1; i <= NUM_OF_IP_SERVICES; i++) {
			pServiceParams = pIpServiceListManager->FindServiceByName(pMRoom->GetServiceRegistrationContentServiceName(i - 1));
			if (pServiceParams != NULL)
			{
				if (pMRoom->GetServiceRegistrationContentRegister(i - 1) == TRUE) {
					if (pMRoom->GetEntryQ())
						SipProxyApi.AddEQ(pServiceParams->GetServiceId(), pMRoom->GetName(),	pMRoom->GetMonitorConfId());
					else if (pMRoom->IsSIPFactory())
						SipProxyApi.AddFactory(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
					else if (pMRoom->GetIsGateway())
						SipProxyApi.AddGW(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
					else
						SipProxyApi.AddMR(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
				}
			}
		}
    }

  POBJDELETE(pMRoom);
}

void CConfPartyManager::OnRsrcAddMeetingRoom(CCommResApi* pCommResApi)
{
	STATUS status = STATUS_OK;
	CCommRes* pMRoom = new CCommRes(*pCommResApi);

	//Set the monitor Id of all parties
	//(this is setting only for Adding MR reservations)
	pMRoom->SetMonitorIdForAllParties();

	//Set the Dial-in prefix for the new conference
	pMRoom->SetConfDialinPrefix();
	// fill empty routing name after numeric id had been allocated
	pMRoom->FillEmptyDiplayNameOrName();

	//Add a new meeting room to the MR DB
	status = ::GetpMeetingRoomDB()->Add(*pMRoom);
	if (status != STATUS_OK)
	{
		TRACEINTO << "Failed Adding a new meeting room to templates DB, Conf Name: " << pMRoom->GetName();

		DWORD dwMRMonitorID = pMRoom->GetMonitorConfId();
		CSegment *pToRsrcSeg = new CSegment;
		*pToRsrcSeg << dwMRMonitorID;
		SendMsgToRsrvMngrRsrcProcess(pToRsrcSeg, DEL_MR_REQ);
		PASSERT(status);
		//Answer Server Dont Know How!!!
	}
	else //(status == STATUS_OK)
	{
		TRACEINTO << "Adding successfully a new meeting room to templates DB, Conf Name: " << pMRoom->GetName();
		CSipProxyManagerApi SipProxyApi;
		CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
		CConfIpParameters* pServiceParams = NULL;
		for (int i = 1; i <= NUM_OF_IP_SERVICES; i++)
		{
			pServiceParams = pIpServiceListManager->FindServiceByName(pMRoom->GetServiceRegistrationContentServiceName(i - 1));
			if (pServiceParams != NULL)
			{
				if (pMRoom->GetServiceRegistrationContentRegister(i - 1) == TRUE)
				{
					if (pMRoom->GetEntryQ())
						SipProxyApi.AddEQ(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
					else if (pMRoom->IsSIPFactory())
						SipProxyApi.AddFactory(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
					else if (pMRoom->GetIsGateway())
						SipProxyApi.AddGW(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
					else
						SipProxyApi.AddMR(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
				}
			}
		}
	}

	POBJDELETE(pMRoom);
}

 /////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerDelMR(CRequest* pRequest)
{
  if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
  {
       FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerDelMR: No permission to OnServerDelMR for administrator readony");
       pRequest->SetConfirmObject(new CDummyEntry());
       pRequest->SetStatus(STATUS_NO_PERMISSION);
       return STATUS_NO_PERMISSION;
  }

  STATUS status = STATUS_OK;
  PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerDelMR :  Got Delete Meeting Room Request");

  char* pName = new char[H243_NAME_LEN];

  if (pName == NULL)
  {
    PASSERT(1);
    return STATUS_FAIL;
  }

  memset(pName, 0, H243_NAME_LEN);

  CCommResDBAction * pCommResDBAction=new CCommResDBAction;
  *pCommResDBAction = *(CCommResDBAction*)pRequest->GetRequestObject() ;

  DWORD mrId = pCommResDBAction->GetConfID();

  CCommResShort*  pMrShort = ::GetpMeetingRoomDB()->GetCurrentRsrvShort(mrId);
    // In case MR is not found in DB, Reply the request with the relevant status and exit
  if (NULL == pMrShort)
  {
    status = STATUS_MEETING_ROOM_NOT_EXISTS;
  }

  if(status == STATUS_OK)
  {
     strncpy(pName, pMrShort->GetName(), H243_NAME_LEN);
     pName[H243_NAME_LEN - 1] = '\0';

     //If the MR state is ACTIVE we can not delete it
     if ( MEETING_ROOM_ACTIVE_STATE == pMrShort->GetMeetingRoomState() )
     {
       if(pMrShort->IsEntryQ())
       {
    	  TRACESTR (eLevelError) << "Deleting the active Entry Queue:" <<  pMrShort->GetName() << ", is not allowed!";
    	  status = STATUS_ILLEGAL_WHILE_EQ_IS_ACTIVE;
       }
       else
       {
    	  TRACESTR (eLevelError) << "Deleting the active meeting room:" <<  pMrShort->GetName() << ", is not allowed!";
    	  status = STATUS_ILLEGAL_WHILE_MR_IS_ACTIVE;
       }
     }

     POBJDELETE(pMrShort);

     if (STATUS_OK == status)
     {
       CSegment *pToRsrcSeg = new CSegment;
       *pToRsrcSeg << mrId ;

       SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, DEL_MR_REQ);

       status=(::GetpMeetingRoomDB())->Cancel(mrId);

		char* transitEQ = new char[H243_NAME_LEN];

		if (transitEQ == NULL)
		{
			PASSERT(1);
			DEALLOCBUFFER(pName);
			return STATUS_FAIL;
		}

		memset(transitEQ, 0, H243_NAME_LEN);
		strncpy(transitEQ,::GetpMeetingRoomDB()->GetTransitEQName(),H243_NAME_LEN);
		transitEQ[H243_NAME_LEN - 1] = '\0';

		if(!strcmp(pName,transitEQ))
		{
			PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerDelMR, The deleted room is the transit EQ");

/*	   		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentOnGoingEQ(transitEQ);
	    	if(NULL != pCurConf)
	    	{
	    		 // Send Auto terminate to conf
			    CConfApi* pConfApi = new CConfApi;
				pConfApi->CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			//	CSegment* seg = new CSegment;
			//	*seg << transitEQ;

	    		pConfApi->SetAutoTerminateTimer();
	    	//	POBJDELETE(seg);
	    		POBJDELETE(pConfApi);
	    	}
			else
			{
				PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerCancelTransitEQ, There is no ongoing conference from the default EQ");
			}
*/
			::GetpMeetingRoomDB()->CancelTransitEQ();
		}

       if ( status != STATUS_OK)
	   {
	     PTRACE(eLevelError, "CConfPartyManager::OnServerDelMR :  Failed Delete a meeting room on MR DB");
	     PASSERT(1);
	   }
       else
	   {
	     CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	     CSipProxyManagerApi SipProxyApi;
	     for( int i=1; i<=4;i++ )
	     {
	         CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(i);
	         if( pServiceParams == NULL )
	            continue;
	         SipProxyApi.DelConference(i, pName, mrId);   //////Michael+Judith - real serviceId should be added
	     }
	   }
	   DEALLOCBUFFER(transitEQ);
     }

  }

     DEALLOCBUFFER(pName);

    pRequest->SetTransName("TRANS_RES");
    pRequest->SetConfirmObject(pCommResDBAction);
    pRequest->SetStatus(status);

    return STATUS_OK;

}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::UpdateMR(CCommRes * commRes)
{
	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateMR :  Got Update Meeting Room Request");

	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	CConfIpParameters* pServiceParams = NULL;
	CSipProxyManagerApi SipProxyApi;
	const CStructTm* pStartTime = commRes->GetStartTime();
	const CStructTm* pEndTime = commRes->GetEndTime();
	DWORD  durationTime = *pEndTime - *pStartTime;


    for (int i=1; i<=NUM_OF_IP_SERVICES; i++)
    {
    	if ( (commRes->GetServiceRegistrationContentServiceName(i-1))[0] !='\0')
    	{
    		pServiceParams = pIpServiceListManager->FindServiceByName(commRes->GetServiceRegistrationContentServiceName(i-1));
    		if (pServiceParams != NULL)
    		{


    			if( commRes->GetServiceRegistrationContentRegister(i-1) == TRUE )
    			{
    				PTRACE(eLevelInfoNormal, "CConfPartyManager::OnServerUpdateMR :  service should be registered");
					if ( commRes->GetServiceRegistrationContentStatus(i-1) == eSipRegistrationStatusTypeRegistered)
					{
						PTRACE(eLevelInfoNormal, "CConfPartyManager::OnServerUpdateMR :  conference already registered. No need to re-register");
					}
					else
					{
						PTRACE(eLevelInfoNormal, "CConfPartyManager::OnServerUpdateMR :  conference not registered yet. Need to register");
    					if(commRes->GetEntryQ())
    						SipProxyApi.AddEQ(pServiceParams->GetServiceId(), commRes->GetName(), commRes->GetMonitorConfId(), durationTime);//////Michael+Judith - real serviceId should be added
    					else if(commRes->IsSIPFactory())
    						SipProxyApi.AddFactory(pServiceParams->GetServiceId(), commRes->GetName(), commRes->GetMonitorConfId(), durationTime); //////Michael+Judith - real serviceId should be added
    					else if(commRes->GetIsGateway())
    						SipProxyApi.AddGW(pServiceParams->GetServiceId(), commRes->GetName(), commRes->GetMonitorConfId(), durationTime); //////Michael+Judith - real serviceId should be added
    					else
    						SipProxyApi.AddMR(pServiceParams->GetServiceId(), commRes->GetName(), commRes->GetMonitorConfId(), durationTime); //////Michael+Judith - real serviceId should be added
					}

    			}
    			else
    			{
    				PTRACE(eLevelInfoNormal, "CConfPartyManager::OnServerUpdateMR : service should not be registered");
    				if ( commRes->GetServiceRegistrationContentStatus(i-1) == eSipRegistrationStatusTypeRegistered)
    					SipProxyApi.DelConference(pServiceParams->GetServiceId(), commRes->GetName(), commRes->GetMonitorConfId());

					commRes->SetServiceRegistrationContentStatus(i-1,eSipRegistrationStatusTypeNotConfigured);
					commRes->UpdateServiceRegistrationTotalStatus(i-1,eSipRegistrationTotalStatusTypeNotConfigured);
					//::GetpMeetingRoomDB()->Update(*commRes);
    			}
    		}
    		/* In case we didn't find the service it is NULL and we can't access it.
    		 * we dont want to delete the conf from all the services.
    		else
    		{
    			SipProxyApi.DelConference(pServiceParams->GetServiceId(), commRes->GetName(), commRes->GetMonitorConfId());
    		}
    		*/
    	}
    }

	//Update a specified meeting room on the MR DB
	STATUS status= (::GetpMeetingRoomDB())->Update(*commRes);
	if ( status != STATUS_OK)
	{
		PTRACE(eLevelError, "CConfPartyManager::OnServerUpdateMR :  Failed Update a meeting room on MR DB");
		PASSERT(1);
		return status;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnLobbyStartMeetingRoom(CSegment* pSeg)
{

	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnLobbyStartMeetingRoom: AWAKE Meeting Room recieved from Lobby  ");

	DWORD mrId=0xFFFFFFFF;
	STATUS status = STATUS_OK;
	*pSeg >> mrId;

 	//Find the reservation in the MR DB
	CCommRes*  pMR = ::GetpMeetingRoomDB()->GetCurrentRsrv(mrId);

	CLobbyApi* pLobbyApi= ((CLobbyApi*)::GetpLobbyApi()) ;

	//Send reject to Lobby - Start meeting room failed
	//if (!CPObject::IsValidPObjectPtr(pMR))
	if (pMR == NULL)
	{
		PTRACE(eLevelInfoNormal, "CConfPartyManager::OnLobbyStartMeetingRoom Reject to Lobby, can not find conf in MR DB");
		RejectMeetingRoomOrAdHocActivation(mrId, STATUS_ILLEGAL);
		return;
	}
	if(m_lockConfReqCounter)
	{
		PTRACE(eLevelInfoNormal, "CConfPartyManager::OnLobbyStartMeetingRoom Reject to Lobby, Any Conference establishemnt is blocked");

		if (m_bLockConfForInvalidCertificate)
		{
			RejectMeetingRoomOrAdHocActivation(mrId,
				STATUS_CONFERENCE_CREATION_IS_BLOCKED_FOR_INVALID_CERTIFICATE);
		}
		else
		{
			RejectMeetingRoomOrAdHocActivation(mrId, STATUS_CONFERENCE_CREATION_IS_BLOCKED);
		}

		return;
	}


	DWORD profileId = 0xFFFFFFFF;

	if(pMR->IsConfFromProfile(profileId) == YES )
	{
		CCommRes* pProfile = ::GetpProfilesDB()->GetCurrentRsrv(profileId);
		if (pProfile)
			pMR->SetMyProfileBasedParams(pProfile, FALSE);
		else
		  status = STATUS_PROFILE_NOT_FOUND;

		POBJDELETE(pProfile);
	}

	if(status!=STATUS_OK)
	{
	   RejectMeetingRoomOrAdHocActivation(mrId, status);
	   POBJDELETE(pMR);
	   return;
	}

	//Check if the Dongle allow encryption
	if ( ::GetDongleEncryptionValue() == FALSE )
	  pMR-> SetEncryptionParameters(NO, eEncryptNone);


	status = pMR->TestValidity();

	if(status != STATUS_OK)
	{
		PTRACE(eLevelError, "CConfPartyManager::OnLobbyStartMeetingRoom - Do not awake MR/EQ, because TestValidity fails - Send Reject to Lobby");
		CLargeString description;

	    description << "EQ/MR will not be established due to Wrong parameters";

        description << " MR/EQ Name: "
                     << pMR->GetName()
                     << ".";
        description << " Status is: " << (CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,ILLEGAL_CONFERENCE_PARAMETERS,MAJOR_ERROR_LEVEL,description.GetString(), FALSE);
		RejectMeetingRoomOrAdHocActivation(mrId, status);
		POBJDELETE(pMR);
		return;
	}


	//Set Auto Termination to the EntryQ
	if(pMR->GetEntryQ())
		pMR->SetAutomaticTermination(NO);

	int isMRActive=pMR->GetMeetingRoomState();

	if( isMRActive && (!(pMR->GetEntryQ())) )
		//Meeting room was already created but still not created as conf task
		PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnLobbyStartMeetingRoom: meeting room was found ACTIVE .Name=",pMR->GetName());
	//REJECT???
	else
	{
		OPCODE opcode = ACTIVATE_MR_REQ;
		CSegment *pToRsrcSeg = new CSegment;

		pMR->Serialize(NATIVE, *pToRsrcSeg);
		SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, opcode);
	}

	POBJDELETE(pMR);

}
/////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnLobbyStartGateWayConf(CSegment* pSeg)
{

	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnLobbyStartGateWayConf: START_GATEWAY_CONF recieved from Lobby  ");

	DWORD mrId=0xFFFFFFFF;
	char confName[H243_NAME_LEN];
	memset(confName,'\0',H243_NAME_LEN);
	WORD dialStringLen = 0;
	char dialString[MaxAddressListSize];
	memset(dialString,'\0',MaxAddressListSize);

	STATUS status = STATUS_OK;
	*pSeg >> mrId
		  >> confName;

	*pSeg >> dialStringLen;

	if (dialStringLen)
	{
		*pSeg >> dialString;
		PTRACE2(eLevelInfoNormal,"found dial string: ", dialString);
	}
	else
		PTRACE(eLevelInfoNormal,"dial string did not found!!!");

	//Find the reservation in the MR DB
	CCommRes*  pMRTemp = ::GetpMeetingRoomDB()->GetCurrentRsrv(mrId);

	CLobbyApi* pLobbyApi= ((CLobbyApi*)::GetpLobbyApi()) ;

	//Send reject to Lobby - Start meeting room failed
	if (!CPObject::IsValidPObjectPtr(pMRTemp))
	{
		PTRACE(eLevelInfoNormal, "CConfPartyManager::OnLobbyStartGateWayConf Reject to Lobby, can not find conf in MR DB");
		RejectMeetingRoomOrAdHocActivation(mrId, STATUS_ILLEGAL,/*sendDeactivationToRA*/FALSE, /*isAdHoc*/TRUE,/*adHocConfName*/confName);
		return;
	}
	if(m_lockConfReqCounter)
	{
		PTRACE(eLevelInfoNormal, "CConfPartyManager::OnLobbyStartGateWayConf Reject to Lobby, Any Conference establishemnt is blocked");

		if (m_bLockConfForInvalidCertificate)
		{
			RejectMeetingRoomOrAdHocActivation(mrId,
				STATUS_CONFERENCE_CREATION_IS_BLOCKED_FOR_INVALID_CERTIFICATE,
				/*sendDeactivationToRA*/FALSE,
				/*isAdHoc*/TRUE,
				/*adHocConfName*/confName);
		}
		else
		{
			RejectMeetingRoomOrAdHocActivation(mrId,
				STATUS_CONFERENCE_CREATION_IS_BLOCKED,
				/*sendDeactivationToRA*/FALSE,
				/*isAdHoc*/TRUE,
				/*adHocConfName*/confName);
		}

		return;
	}


	DWORD profileId = 0xFFFFFFFF;
	DWORD dwAdHocProfileId = pMRTemp->GetAdHocProfileId();
	// Get Profile according to Ad-Hoc Profile ID
	CCommRes*  pMR = ::GetpProfilesDB()->GetCurrentRsrv(dwAdHocProfileId);

	if (pMR && dwAdHocProfileId)
	{

		pMR->SetAdHocConfBasicParams(confName, "", profileId );
		pMR->SetIsGateway(YES);
		pMR->SetGWDialOutProtocols(pMRTemp->GetGWDialOutProtocols());
		// All GW sessions are cp
		pMR->SetHDVSW(NO);
		pMR->SetVideoSession(CONTINUOUS_PRESENCE);
		// direct dial in is disabled in GW sessions
		pMR->SetMeetMePerConf(NO);
		// all GW sessions are terminated immediatly with last party remains
		pMR->SetAutomaticTermination(YES);
		pMR->SetLastQuitType(eTerminateWithLastRemains);
		pMR->SetTimeAfterLastQuit(0);

		pMR->SetServiceNameForMinParties( pMRTemp->GetServiceNameForMinParties() );	//VNGR-18702
	}
	else
	  status = STATUS_PROFILE_NOT_FOUND;

	if (status == STATUS_OK && !pMR->IsDefinedGWDialOutProtocols())
		// gw conf can not be established without dial out protocol
		status = STATUS_ILLEGAL;



	if(status!=STATUS_OK)
	{
	   RejectMeetingRoomOrAdHocActivation(mrId, status,/*sendDeactivationToRA*/FALSE, /*isAdHoc*/TRUE,/*adHocConfName*/confName);
	   POBJDELETE(pMR);
	   return;
	}

	//Check if the Dongle allow encryption
	if ( ::GetDongleEncryptionValue() == FALSE )
	  pMR-> SetEncryptionParameters(NO, eEncryptNone);

	pMR->SetEntryQ(NO);
	if (!dialStringLen)
	{
		pMR->SetIsVideoInvite(YES);
	}

	status = pMR->TestValidity();

	if(status != STATUS_OK)
	{
		PTRACE(eLevelError, "CConfPartyManager::OnLobbyStartGateWayConf - Do not awake MR/EQ, because TestValidity fails - Send Reject to Lobby");
		CLargeString description;

	    description << "GW will not be established due to Wrong parameters";

        description << " GW Name: "
                     << pMR->GetName()
                     << ".";
        description << " Status is: " << (CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,ILLEGAL_CONFERENCE_PARAMETERS,MAJOR_ERROR_LEVEL,description.GetString(), FALSE);
		RejectMeetingRoomOrAdHocActivation(mrId, status,/*sendDeactivationToRA*/FALSE, /*isAdHoc*/TRUE,/*adHocConfName*/confName);
		POBJDELETE(pMR);
		return;
	}


	int isMRActive=pMR->GetMeetingRoomState();

	if( isMRActive && (!(pMR->GetEntryQ())) )
		//Meeting room was already created but still not created as conf task
		PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnLobbyStartGateWayConf: meeting room was found ACTIVE .Name=",pMR->GetName());
		//REJECT???
	else
	{

		OPCODE opcode = START_AD_HOC_CONF_REQ;
		CSegment *pToRsrcSeg = new CSegment;

		COstrStream ostrToRsrc;
		pMR->Serialize(NATIVE, ostrToRsrc);

		// VNGR-22659: set the sender position identifier in order to identify ongoing
		// conference restore after reboot
		ostrToRsrc << (WORD)1 << "\n";

		// set isdn num in order to identify later (in conf) from which service to dial out
		if (pMRTemp->IsEnableIsdnPstnAccess())
		{
			ostrToRsrc << (WORD)1 << "\n";
			CServicePhoneStr* pService = pMRTemp->GetFirstServicePhone();
			ostrToRsrc << pService->GetNetServiceName() << "\n";
		}
		else
		{
			ostrToRsrc << (WORD)0 << "\n";
		}

		if (dialStringLen)
			ostrToRsrc << dialString;

		//PTRACE2(eLevelInfoNormal,"*********ostrToRsrc: ",ostrToRsrc.str().c_str());
		ostrToRsrc.Serialize(*pToRsrcSeg);

		SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, opcode);

	}

	POBJDELETE(pMR);
	POBJDELETE(pMRTemp);
}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnRsrcActivateMeetingRoomInd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnRsrcActivateMeetingRoom Awake Meeting Room back from Resources");

   	STATUS status = STATUS_OK;
   	CCommRes* pConf= new CCommRes;
	CIstrStream istr(*pSeg);

	pConf->DeSerialize(NATIVE,istr);
	istr >> status ;//Read the Status from the Resource RsrvManager



	CCommRes*  pMRoom = ::GetpMeetingRoomDB()->GetCurrentRsrv(pConf->GetName());

	if (!pMRoom)
	  {
	    TRACESTR (eLevelError) << "CConfPartyManager::OnRsrcActivateMeetingRoomInd Can not find the MR: " << pConf->GetName() << " in the DB ";
	    PASSERT(1);
	    POBJDELETE(pConf);
	    return;
	  }

	 DWORD mrId = pMRoom->GetMonitorConfId();

	if ( STATUS_OK != status )
	  {
	    TRACESTR (eLevelError) << "CConfPartyManager::OnRsrcActivateMeetingRoomInd Resource NACK with status: " << status  << " in the DB ";
//	    PASSERT(1); // Sergey: The assert here is wrong(for instance reject from permanent MR is a legal situation
	    RejectMeetingRoomOrAdHocActivation(mrId, status);
	    POBJDELETE(pMRoom);
	    POBJDELETE(pConf);
	    return;

	  }



	//if its an EQ set its unique name
	if (pConf->GetEntryQ())
	  {
	  	// set routing name
	    ALLOCBUFFER(newConfName,H243_NAME_LEN);
	    memset(newConfName,'\0',H243_NAME_LEN);
	    sprintf(newConfName,"%s(%d)",pConf->GetName(),pConf->GetMonitorConfId());
	    TRACESTR (eLevelInfoNormal) << "CConfPartyManager::OnRsrcActivateMeetingRoomInd Changing EQ ongoing name from: " << pConf->GetName()
				     << " , to: " << newConfName;
	    pConf->SetName(newConfName);
	    DEALLOCBUFFER(newConfName);
	    // set display name
	    ALLOCBUFFER(newConfDisplayName,H243_NAME_LEN);
	    memset(newConfDisplayName,'\0',H243_NAME_LEN);
	    const char* resDisplayName = pConf->GetDisplayName();
	    sprintf(newConfDisplayName,"%s(%d)",resDisplayName,pConf->GetMonitorConfId());
	    TRACESTR (eLevelInfoNormal) << "CConfPartyManager::OnRsrcActivateMeetingRoomInd Changing EQ ongoing Display name from: " << resDisplayName
				     << " , to: " << newConfDisplayName;
	    pConf->SetDisplayName(newConfDisplayName);
	    DEALLOCBUFFER(newConfDisplayName);
	  }
	CLobbyApi* pLobbyApi= ((CLobbyApi*)::GetpLobbyApi()) ;

	if(pConf->GetOperatorConf() ){
	  PTRACE2(eLevelInfoNormal, "CConfPartyManager::OnRsrcStartConfInd , SetOperatorConfInfo : ", (char*)pConf->GetName());
	  pConf->SetOperatorConfInfo();
	}
	if(status == STATUS_OK)
	      status = pConf->TestValidity();

	if(status != STATUS_OK)
	{
		PTRACE(eLevelError, "CConfPartyManager::OnRsrcActivateMeetingRoomInd - Do not awake MR/EQ, because TestValidity fails - Send Reject to Lobby");
		RejectMeetingRoomOrAdHocActivation(mrId, status);
		POBJDELETE(pMRoom);
	    POBJDELETE(pConf);
		return;
	}
	if(STATUS_OK == status)
	{
		status = StartOnGoingConf(*pConf);
		if (status != STATUS_OK)
		{
			PTRACE(eLevelError, "CConfPartyManager::OnRsrcActivateMeetingRoomInd :  Failed");
			RejectMeetingRoomOrAdHocActivation(mrId, status,TRUE);
	  		POBJDELETE(pMRoom);
	  		POBJDELETE(pConf);
			return;
		}

		else
		{
			// Mark room state as Active
			// The new pCommRes has the conferenc(not the MR) information
			pMRoom->SetMeetingRoomState(MEETING_ROOM_ACTIVE_STATE);
			::GetpMeetingRoomDB()->Update(*pMRoom);
		}
		/*
			DWORD dwPartyId = 0xFFFFFFFF;

			//Check if the Dial in party is Defined/UnDefined
			isDefined = pRsrv->IsPartyDefined(phoneLenOrIP,alias, dwPartyId);

			// Undefined Party should not awake the MR
			//	if (pCommRes->GetUnlimitedReservFlag()==NO && !isDefined)
			//		{
			//			PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnStartMeetingRoom: Undefined Party Not Allowed to Awake MR. Name=",pCommRes->GetName());
			//			pLobbyApi->RejectStartMeetingRoom(confId, undefId, STATUS_ILLEGAL);
			//    		POBJDELETE(pCommRes);
			//			DEALLOCBUFFER(alias);
			//		}

			CCommConf * pCurCommConf =::GetpConfDB()->GetCurrentConf(pRsrv->GetName());

			if (!CPObject::IsValidPObjectPtr(pCurCommConf))
			{
				PASSERT(1);
				PTRACE(eLevelError,"CConfPartyManager::OnStartMeetingRoom can not find the conf in the OnGOing DB");
				//return STATUS_FAIL;
			}

			//Send Add Party if the Party UnDefined or send to the conf the msg on defined party connection
			if (isDefined == TRUE) //Defined Party
			{
				PTRACE(eLevelInfoNormal,"CConfPartyManager::OnStartMeetingRoom Defined Party Awake Meeting Room");
				//Send msg to the new conf
				CConfApi confApi;
				confApi.CreateOnlyApi(*(pCurCommConf->GetRcvMbx()));
				confApi.ReleaseReservedPartyFromLobby(undefId,dwPartyId);
				confApi.DestroyOnlyApi();
			}
			else  //UnDefined Party
			{
				PTRACE(eLevelInfoNormal,"CConfPartyManager::OnStartMeetingRoom UnDefined Party Awake Meeting Room");
				CSegment*  seg = new CSegment;
				*seg << phoneLenOrIP<< pCurCommConf->GetMonitorConfId() << undefId << bIsVoice << bIsSip;
				AddUnreservedParty(seg);
			}
		}
	}
	else
	{
		//SendRejectToLobby
	}*/

	//POBJDELETE(pRsrv);
//	DEALLOCBUFFER(alias);
	//return STATUS_OK;
	}
	POBJDELETE(pMRoom);
	POBJDELETE(pConf);
}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnDelParty(CRequest* pRequest, DWORD undefId)
{
  OnServerDelParty(pRequest, undefId);  //this event should be send from reservation module
  return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerDelParty(CRequest* pRequest, DWORD undefId)
{
  if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
  {
       FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerDelParty: No permission to OnServerDelParty for administrator readony");
       pRequest->SetConfirmObject(new CDummyEntry());
       pRequest->SetStatus(STATUS_NO_PERMISSION);
       return STATUS_NO_PERMISSION;
  }

  CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;

  *pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject();
  pRequest->SetObjectFlag(STRING_FLAG);

  DWORD confID  = pRsrvPartyAction->GetConfID();
  DWORD partyID = pRsrvPartyAction->GetPartyID();

  FTRACEINTO << "CConfPartyManager::OnServerDelParty - Deleting participant from conference, confId:" << confID << ", PartyId:" << partyID;

  int status = ::GetpConfDB()->SearchPartyName(confID, partyID);
  if (status == STATUS_OK)
  {
    CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confID);
    CConfParty* pCurConfParty = NULL;
    if (!pCurConf)
    {
      PASSERTSTREAM(STATUS_RESERVATION_NOT_EXISTS, "CConfPartyManager::OnServerDelParty - Failed, 'pCurConf' is NULL");
      status = STATUS_RESERVATION_NOT_EXISTS;
    }
    else
    {
    	CConfParty* pCurConfParty = pCurConf->GetCurrentParty(partyID);
    	if (!pCurConfParty)
    	{
    		PASSERTSTREAM(STATUS_PARTY_DOES_NOT_EXIST, "CConfPartyManager::OnServerDelParty - Failed, 'pCurConfParty' is NULL");
    		status = STATUS_PARTY_DOES_NOT_EXIST;
    	}
    	else
    	{
    		if (status == STATUS_OK)  // Disable action for 'secured' conference only if 'DelParty' request origin is API.
    			if (::GetpConfDB()->IsConfSecured(confID))
    				status = STATUS_CONF_IS_SECURED;

    		if (status == STATUS_OK)  // D.K.VNGR-21434 operator party can't be deleted
    			if (pCurConfParty->IsOperatorParty())
    				status = STATUS_OPERATOR_PARTY_CAN_NOT_BE_DELETED;

    //SEND DropParty to conference
    if (status == STATUS_OK)
    {
      // VNGR-22639: won't try restore lecturer video layout if any party is deleted
      // otherwise the logic might be complex?
      CLectureModeParams* pSavedLectureMode;
      CVideoLayout*  pSavedLecturerVideoLayout;

      ((CCommConf*)pCurConf)->GetLecturerVideoLayout(pSavedLectureMode, pSavedLecturerVideoLayout);

      if (pSavedLectureMode)
      {
        POBJDELETE(pSavedLectureMode);
        POBJDELETE(pSavedLecturerVideoLayout);
        ((CCommConf*)pCurConf)->SaveLecturerVideoLayout(pSavedLectureMode, pSavedLecturerVideoLayout);
      }

      char* curPartyName = (char*)pCurConfParty->GetName();

    			CConfApi confApi;
    			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
    			confApi.DropParty(curPartyName);
    			confApi.DestroyOnlyApi();
    			((CCommConf*)pCurConf)->OperatorPartyAction(curPartyName, partyID, m_operName, OPERATORS_DELETE_PARTY);
    		}
    	}
    }
  }

  std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_CONF_2
  pRequest->SetTransName(responseTrancsName);
  pRequest->SetStatus(status);
  pRequest->SetConfirmObject(pRsrvPartyAction);
  return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerAddPartyToConf(CRequest* pRequest)
{
	return OnServerAddPartyToConfByType(pRequest, 0);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerAddPartyToConfByType(CRequest* pRequest, DWORD undefId, eSipFactoryType factoryType, eTypeOfLinkParty partyType) // shiraITP - 45
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACEINTO << "Failed, No permission (ADMINISTRATOR_READONLY)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;
	STATUS respStatus = STATUS_OK;

	CRsrvPartyAdd* pRsrvPartyAdd = new CRsrvPartyAdd;

	*pRsrvPartyAdd = *(CRsrvPartyAdd*)pRequest->GetRequestObject();
	CRsrvParty* pRsrvParty = pRsrvPartyAdd->GetRsrvParty();

	BYTE isRecordingParty = pRsrvParty->GetRecordingLinkParty();
	ConfMonitorID confId = pRsrvPartyAdd->GetConfID();

	std::ostringstream msg;
	msg << "MonitorConfId:" << confId << ", PartyName:" << pRsrvParty->GetName() << ", MsftAvmcuState:" << enMsftAvmcuStateNames[pRsrvParty->GetMsftAvmcuState()];
	TRACEINTO << msg.str().c_str();

	CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
	if (!pCurConf)
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Cannot find conference";
		status = STATUS_CONF_NOT_EXISTS;
	}

	if (STATUS_OK == status)
	{
		if (pCurConf->GetVideoRecoveryStatus())
		{	/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
			TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Media is recovering";
			return STATUS_NO_PERMISSION;
		}	/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/
	}

	if (STATUS_OK == status)
	{
		if (pCurConf->GetStatus() & CONFERENCE_RESOURCES_DEFICIENCY)
		{
			TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Insufficient resources";
			status = STATUS_INSUFFICIENT_RSRC;
		}
	}

	if (STATUS_OK == status)
	{
		if (!undefId && YES == pCurConf->GetEntryQ())
		{
			TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Cannot add participant to the EQ";
			status = STATUS_CANNOT_ADD_DEFINED_PARTY_TO_EQ;
		}
	}

	// Test for AVC-SVC-Mix conference type
	if (STATUS_OK == status)
	{
#if 1
		// if media-relay-only conference the dial-out should to be blocked
		if (eSvcOnly == pCurConf->GetConfMediaType())
		{
			if (pRsrvParty->GetConnectionType() == DIAL_OUT)
			{
				TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Cannot connect Dial-Out to Media-Relay-Only conference";
				status = STATUS_CANNOT_ADD_DIAL_OUT_PARTY_TO_MEDIA_RELAY_CONF;  // interfering with VSW feature
			}

			if (pRsrvParty->GetNetInterfaceType() == H323_INTERFACE_TYPE)
			{
				TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Cannot connect H323 to Media-Relay-Only conference";
				status = STATUS_CANNOT_ADD_H323_PARTY_TO_MEDIA_RELAY_CONF;
			}
		}
#endif // if 1
		if (STATUS_OK == status)
		{
			if ((eMixAvcSvc == pCurConf->GetConfMediaType()) && (pRsrvParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE))
			{
				if ((CProcessBase::GetProcess()->GetProductFamily() == eProductFamilySoftMcu && (CProcessBase::GetProcess()->GetProductType() != eProductTypeNinja)) ||
				    ((CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyRMX) && (pRsrvParty->GetVoice() == NO)))
				{
					TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Cannot connect ISDN to conference";
					status = STATUS_ILLEGAL_H320_PARTY_IN_MEDIA_RELAY_CONF;
				}
			}
		}
	}

	if (STATUS_OK == status)
	{
		WORD confNumParts = pCurConf->GetNumParties();
		WORD confMaxParts = pCurConf->GetMaxParties();

		if (pCurConf->IncludeRecordingParty())
			confNumParts = confNumParts - 1; // we don't include the recording party in the max participants limitation.

		if ((confMaxParts <= confNumParts) && (confMaxParts != 0xFFFF) && !isRecordingParty)
		{
			if (!pCurConf->IsDefinedIVRService() || !pRsrvParty->IsUndefinedParty())
			{
				TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Conference has already " << pCurConf->GetNumParties() << " (maximum participants allowed)";
				status = STATUS_NUMBER_OF_PARTICIPANTS_EXCEEDS_THE_DEFINED_MAXIMUM_FROM_PROFILE;
			}
		}
	}

	if (STATUS_OK == status)
	{
		WORD confNumParts = pCurConf->GetNumParties();
		WORD maxPartiesInConfPerSystemMode = m_pProcess->GetMaxNumberOfPartiesInConf();
		if (confNumParts >= maxPartiesInConfPerSystemMode)
		{
			TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Maximum participants exceeded max allowed is: " << maxPartiesInConfPerSystemMode;
			status = STATUS_PARTIES_NUMBER_LIMIT_EXCEEDED;
		}
	}

	if (STATUS_OK == status)
	{
		BYTE isAudioOnlyParty = pRsrvParty->GetVoice();
		if (!isAudioOnlyParty)
		{
			WORD confNumVideoParts = pCurConf->GetNumVideoParties();
			WORD maxVideoPartiesInConfPerSystemMode = m_pProcess->GetMaxNumberOfVideoPartiesInConf();
			if (confNumVideoParts >= maxVideoPartiesInConfPerSystemMode)
			{
				TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Maximum video participants exceeded max allowed is: " << maxVideoPartiesInConfPerSystemMode;
				status = STATUS_PARTIES_NUMBER_LIMIT_EXCEEDED;
			}
		}
	}

	if (STATUS_OK == status)
	{
		// Only in case of DialOut it should be blocked.
		if (::GetpConfDB()->IsConfSecured(confId) && pRsrvParty->GetConnectionType() == DIAL_OUT)
		{
			TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Cannot add parties to the secured conference";
			status = STATUS_ADD_PARTICIPANT_TO_SECURE_CONF;
		}
	}

	if (STATUS_OK == status)
		status = pCurConf->TestPartyRsrvValidity(pRsrvParty);

	if (STATUS_OK == status)
	{
		status = ::GetpConfDB()->SearchPartyName(confId, pRsrvParty->GetName());
		if (status == STATUS_OK)
		{
			TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Participant with same name already exist";
			status = STATUS_PARTY_NAME_EXISTS;
		}
		else if (status == STATUS_PARTY_DOES_NOT_EXIST)
			status = STATUS_OK;
	}

	if (STATUS_OK == status) // if the party name is identical to visual name that already exist
	{
		status = ::GetpConfDB()->SearchPartyVisualName(confId, pRsrvParty->GetName());
		if (status == STATUS_OK)
		{
			TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Participant with same visual name already exist";
			status = STATUS_PARTY_NAME_EXISTS;
		}
		else if (status == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS)
			status = STATUS_OK;
	}

	// Alert the user if we have another party with the same ip
	if (STATUS_OK == status)
	{
		status = pCurConf->SearchPartyByIPOrAlias(pRsrvParty->GetIpAddress(), pRsrvParty->GetH323PartyAlias(), pRsrvParty->GetH323PartyAliasType());
		if ((STATUS_PARTY_DOES_NOT_EXIST == status) || (pCurConf->GetIsCallGeneratorConference() == TRUE))
			status = STATUS_OK;
		else // IpV6
		{
			char ipAddr[64];
			::ipToString(pRsrvParty->GetIpAddress(), ipAddr, 1);
			TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Found party with the same IP (" << ipAddr << ") or same alias name (" << pRsrvParty->GetH323PartyAlias() << ")";
			respStatus = status;
			status = STATUS_OK;
		}
	}

	// AUDIO BRIDGE TEMPORARY
	if (status == STATUS_OK)
	{
		if (pCurConf->IsAudioConf())
			pRsrvParty->SetVoice(YES);
	}

	// In HD VSW conference MCU force the defined endpoints bit-rate to be auto
	if ((status == STATUS_OK) || (status == (STATUS_PARTY_NAME_EXISTS | WARNING_MASK)))
	{
		if (pCurConf->GetIsHDVSW())
			pRsrvParty->SetVideoRate(0xFFFFFFFF); // automatic

	}

	if ((status == STATUS_OK) || (pCurConf && (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator) && (status == (STATUS_PARTY_IP_ALIAS_ALREADY_EXISTS | WARNING_MASK))))
	{
		DWORD nextPartyId = pCurConf->NextPartyId();
		// SET new partyId to party
		if (pRsrvParty->GetPartyId() <= HALF_MAX_DWORD || pRsrvParty->GetPartyId() == 0xFFFFFFFF)
			pRsrvParty->SetPartyId(nextPartyId);
	}

	if (STATUS_OK == status)
	{
		CMoveConfDetails confMoveDetails(pCurConf);
		CMoveInfo*partyMoveInfo = pRsrvParty->GetMoveInfo();
		partyMoveInfo->Create(confMoveDetails, pRsrvParty->IsOperatorParty());
	}

	// SEND Add party to conference
	if ((status == STATUS_OK) || (status == (STATUS_PARTY_NAME_EXISTS | WARNING_MASK))) // those are ok statuses as well
	{
#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
		CConfApi confApi(pCurConf->GetMonitorConfId());
#else
		CConfApi confApi;
#endif
		confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));

		TRACEINTO << "pRsrvParty->GetMsftAvmcuState()" << enMsftAvmcuStateNames[pRsrvParty->GetMsftAvmcuState()];

		confApi.AddParty(*pRsrvParty, undefId, (DWORD)partyType); // shiraITP - 1   //shiraITP - 46
		if(pCurConf->IsCOPReservation()) //VNGFE-7764
		{
			if (pRsrvParty->GetCascadeMode() == CASCADE_MODE_SLAVE && !pRsrvParty->GetVoice())
			{
				pCurConf->UpdateLectureModeAndLayoutBecauseSlaveInConf(pRsrvParty->GetName());
				confApi.UpdateAutoLayout(NO);
				confApi.SetVideoConfLayoutSeeMeAll(*pCurConf->GetVideoLayout());
				confApi.UpdateLectureMode(pCurConf->GetLectureMode());
			}
		}

		confApi.DestroyOnlyApi();
	}

	if (!undefId && eNotSipFactory == factoryType && !isRecordingParty) // In case of e302SipFactory listId can be 0.
	{
		// Write to CDR  if event was sent from Operator
		if (pCurConf && status == STATUS_OK)
		{
			pCurConf->OperatorAddParty(pRsrvParty, m_operName, OPERATOR_ADD_PARTY);
			pCurConf->OperatorAddPartyCont1(pRsrvParty, m_operName, OPERATOR_ADD_PARTY_CONTINUE_1);
			if (pRsrvParty->GetIpAddress().ipVersion == eIpVersion6)
				pCurConf->OperatorIpV6PartyCont1(pRsrvParty, m_operName, USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS);

			pCurConf->OperatorAddPartyCont2(pRsrvParty, OPERATOR_ADD_PARTY_CONTINUE_2);
			pCurConf->UpdateUserDefinedInformation(pRsrvParty);
			pCurConf->OperatorAddPartyEventToCdr(pRsrvParty, (pRsrvParty->GetServiceProviderName()), eOperatorAddPartyAction_OperatorAddPartyToOngoingConference);
		}
	}
	else
	{
		if (!isRecordingParty)
		{
			TRACEINTO << msg.str().c_str() << " - Write CDR info to Lobby call";

			// Event was sent from Lobby task Send reject to Lobby task
			POBJDELETE(pRequest);
			if ((status != STATUS_OK) && (status != (STATUS_PARTY_NAME_EXISTS | WARNING_MASK))) // those are ok statuses as well
			{
				TRACESTRFUNC(eLevelError) << msg.str().c_str() << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << " - Failed";
				CLobbyApi* pLobbyApi = (CLobbyApi*)::GetpLobbyApi();
				pLobbyApi->RejectUnreservedParty(confId, undefId, status);

				if (strlen(pCurConf->GetFocusUriScheduling()) && status != STATUS_CONF_NOT_EXISTS &&  status != STATUS_TERMINATING_CONFERENCE)
				{
					TRACEINTO << "scheduled av-mcu conf- when call is rejected maybe there is a need to auto terminate AV-MCU link";
					#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
							CConfApi confApi(pCurConf->GetMonitorConfId());
					#else
							CConfApi confApi;
					#endif
							confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));


					confApi.ActivateAvMcuAutoTerminationCheck();

					confApi.DestroyOnlyApi();

				}
			}
		}

		//Fix leak: pRsrvPartyAdd wasn't deleted if we are in recording link flow
			POBJDELETE(pRsrvPartyAdd);
		}

	if (!undefId && eNotSipFactory == factoryType && !isRecordingParty)
	{
		// event was sent from Ema Send reply to Ema
		std::string responseTrancsName("TRANS_CONF"); // Instead of TRANS_RES_1
		pRequest->SetTransName(responseTrancsName);

		pRequest->SetConfirmObject(pRsrvPartyAdd);
		if (STATUS_OK != respStatus)
			status = respStatus;

		pRequest->SetStatus(status);
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerUpdateParty(CRequest* pRequest)
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
       FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateParty: No permission to OnServerUpdateParty for administrator readony");
       pRequest->SetConfirmObject(new CDummyEntry());
       pRequest->SetStatus(STATUS_NO_PERMISSION);
       return STATUS_NO_PERMISSION;
    }
	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateParty");
	STATUS status = STATUS_OK;
	STATUS respStatus = STATUS_OK;
	CRsrvPartyAdd* pRsrvPartyAdd = new CRsrvPartyAdd;

	*pRsrvPartyAdd = *(CRsrvPartyAdd*)pRequest->GetRequestObject() ;
	CRsrvParty* pRsrvParty = pRsrvPartyAdd->GetRsrvParty();
	const DWORD confId   = pRsrvPartyAdd->GetConfID();
	DWORD partyId   = pRsrvParty->GetPartyId();

	/*** VALIDITY of conferenceId and CHECK does party already exist ***/
	//check in ongoing conference
	status = ::GetpConfDB()->SearchPartyName(confId, partyId);
	if (status == STATUS_OK)
	{
		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
		if( pCurConf)
		{

			//In HD VSW conference MCU force the defined endopoints bitrate to be auto
			if (pCurConf->GetIsHDVSW())
				pRsrvParty->SetVideoRate(0xFFFFFFFF); //automatic

			status = pCurConf->TestPartyRsrvValidity(pRsrvParty);
			if(status == STATUS_OK)
			{
				CConfParty* pConfParty = pCurConf->GetCurrentParty(partyId);
				if(pConfParty)
				{

				  // vngr-11091 update operator party
				  if(pConfParty->IsOperatorParty()){
				    if(pConfParty->GetOperatorConfInfo() != NULL){
				      pRsrvParty->SetOperatorParty( *(pConfParty->GetOperatorConfInfo()) );
				    }else{
				      PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateParty party set as operator withot operator conf info");
				      PASSERT(partyId);
				    }
				  }

					/////////////////////////////////////////////////////////////////////
					// If party name is changed check if the new name already exists
					/////////////////////////////////////////////////////////////////////
			        const char* party_name = pRsrvParty->GetName();
                    if(strncmp(party_name,pConfParty->GetName(),strlen(party_name)))
                    {
	                     status = ::GetpConfDB()->SearchPartyName(confId,party_name);
	                     if (status == STATUS_OK) status = STATUS_PARTY_NAME_EXISTS;
	                     else if (status == STATUS_PARTY_DOES_NOT_EXIST) status = STATUS_OK;
	                     if(STATUS_OK == status)
	                     {
	                    	if(strncmp(party_name,pConfParty->GetVisualPartyName(),strlen(party_name)))
	                    	{
	                           status = ::GetpConfDB()->SearchPartyVisualName(confId,party_name);
	                           if (status == STATUS_OK)
	  	                         status = STATUS_PARTY_NAME_EXISTS;
	                           else if (status == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS)
	  	                         status = STATUS_OK;
	                    	}
                         }

                    }

                    char tempName[IPV6_ADDRESS_LEN];
                    memset (&tempName,'\0',IPV6_ADDRESS_LEN);

                    if(::GetpConfDB()->IsConfSecured(confId))  //check both cases if in that way and if in second way
				      {
						TRACESTR (eLevelError) <<"ConfPartyManager::OnServerUpdateParty: In the Secured conference can not be updated any parties: ";
						status=STATUS_CONF_IS_SECURED;
	      			}

                    //If IP/Alias is changed in the update, alert the user if we have another party with the same ip/Alias
                    if(STATUS_OK == status)
                    {
                    	WORD isAliasTheSame = TRUE;
                    	mcTransportAddress rsrvIpAddr = pRsrvParty->GetIpAddress();
                    	mcTransportAddress confPartyIpAddr = pConfParty->GetIpAddress();
                    	isAliasTheSame = !strncmp(pRsrvParty->GetH323PartyAlias(),pConfParty->GetH323PartyAlias(),strlen(pRsrvParty->GetH323PartyAlias()));
                    	// ip has changed, alias hasn't
                    	if(isIpAddressEqual(&rsrvIpAddr,&confPartyIpAddr) == FALSE && isAliasTheSame)
                    	{
                    		// check if ip is ocuppied
                            mcTransportAddress spip = pRsrvParty->GetIpAddress();
                    		status = pCurConf->SearchPartyByIP(&spip);
                    		if ( STATUS_PARTY_DOES_NOT_EXIST == status )
                    			status = STATUS_OK;
							else	// party with same ip already exists
							{
								char str[128];
								memset(str, '\0',128);
								if (pRsrvParty->GetIpAddress().ipVersion == eIpVersion4)
									::ipV4ToString(pRsrvParty->GetIpAddress().addr.v4.ip, str);
								else
									::ipV6ToString(pRsrvParty->GetIpAddress().addr.v6.ip, str,1);


								TRACESTR(eLevelInfoNormal) << "CConfPartyManager::OnServerUpdateParty Found Party with the same IP: " << str ;
							}
                    	}
                    	// alias has changed, ip hasn't
                    	else if (!isAliasTheSame && isIpAddressEqual(&rsrvIpAddr,&confPartyIpAddr) == TRUE)
                    	{
                    		// check if alias is ocuppied
                    		status = pCurConf->SearchPartyByAlias(pRsrvParty->GetH323PartyAlias(), pRsrvParty->GetH323PartyAliasType());
                    		if ( STATUS_PARTY_DOES_NOT_EXIST == status )
                    			status = STATUS_OK;
							else	// party with same alias already exists
								TRACESTR(eLevelInfoNormal) << "CConfPartyManager::OnServerUpdateParty Found Party with the same alias: " << pRsrvParty->GetH323PartyAlias();
                    	}
						else if(!isAliasTheSame && isIpAddressEqual(&rsrvIpAddr,&confPartyIpAddr) == FALSE)
						{
							status = pCurConf->SearchPartyByIPOrAlias(pRsrvParty->GetIpAddress(), pRsrvParty->GetH323PartyAlias(), pRsrvParty->GetH323PartyAliasType());
							if ( STATUS_PARTY_DOES_NOT_EXIST == status )
								status = STATUS_OK;
							else
							{
								memset (&tempName,'\0',IPV6_ADDRESS_LEN);
								ipToString(rsrvIpAddr,tempName,1);
								TRACESTR(eLevelInfoNormal) << "CConfPartyManager::OnServerUpdateParty Found Party with the same IP: "
								<< tempName << " or same alias name: "<< pRsrvParty->GetH323PartyAlias ();
							}
						}
                    	if(STATUS_OK!=status)
                    	{
                    		respStatus = status;
                    		status = STATUS_OK;
                    	}



//                    	if(!isAliasTheSame || isIpAddressEqual(&rsrvIpAddr,&confPartyIpAddr) == FALSE)
//	 	                       status = pCurConf->SearchPartyByIPOrAlias(pRsrvParty->GetIpAddress(), pRsrvParty->GetH323PartyAlias ()
//	 						                                             , pRsrvParty->GetH323PartyAliasType () );
//
//		                      if ( STATUS_PARTY_DOES_NOT_EXIST == status )
//		                           status = STATUS_OK;
//		                      else
//		                      {
//		                      	char ipAddr[64];
//		                      	ipToString(rsrvIpAddr,ipAddr,1);
//		                         TRACESTR(eLevelInfoNormal) << "CConfPartyManager::OnServerUpdateParty Found Party with the same IP: "
//					                           << ipAddr << " or same aliase name: "<< pRsrvParty->GetH323PartyAlias ();
//
//		                       }
                    }

			        /////////////////////////////////////////////////////////////////////
				// 	if (STATUS_OK == status)
// 					{
// 						//Block Defined Dial-in party with Cascade on
// 						if((!pConfParty->GetLobbyId()) && (pRsrvParty->GetConnectionType() == DIAL_IN) && (NO != pRsrvParty->GetCascadeMode()))
// 						{
// 							TRACESTR (eLevelError) << "CCommRes::TestPartyRsrvValidity defined dial in party: "
// 															<< pRsrvParty->GetName() << ",with Cascade mode is not allowed";
// 							status = STATUS_CANNOT_ADD_DEFINED_DIAL_IN_CASCADED_LINK;
// 						}
// 					}

					if (STATUS_OK == status)
					{
					DWORD state = pConfParty->GetPartyState();
					if(PARTY_DISCONNECTED == state || PARTY_WAITING_FOR_DIAL_IN == state || PARTY_STAND_BY == state)
					  {
					    //Build update info :  conferId, pRsrvParty, operParty.
					    CUpdateInfo*  pUpdate_info = new CUpdateInfo;
					    pUpdate_info->m_pRsrvParty = new CRsrvParty(*pRsrvParty);
					    pUpdate_info->m_conferId   = confId;
					    pUpdate_info->m_partyId = partyId;
					    pUpdate_info->m_connectId  = pRequest->GetConnectId();

					    if (IsUpdateCompleted())
					    {
							//if there's no parallel Update we:

							//a). insert update info to our list
							InsertUpdateInfo(pUpdate_info);
							//b). send drop party to conference
							CUpdateInfo* pTemp =  (*m_UpdateInfo.begin());
							WORD drop_result = pTemp->SendDropParty();
							PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateParty: Sending Drop Party");

							//c). if send was unsuccessful - delete update info from list
							if (drop_result != STATUS_OK)
							{
							  DBGPASSERT(drop_result);
							  DeleteUpdateInfo(0);
							  PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateParty: Drop status is not OK");
							}

							//d). if send was successful - start timer to check the result of drop operation
							if (drop_result == STATUS_OK)
							  {
							    m_del_timer_num=0;
							    StartTimer(DEL_PARTY_COMPLETE, 1 * SECOND);
							    ((CCommConf*)pCurConf)->UpdateUserDefinedInformation(pRsrvParty);
							  }
						    }
						    else //if there's parallel Update we only insert update info to our list
						    {
						      status = InsertUpdateInfo(pUpdate_info);
						      PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateParty, Party will be updated later, Party Name: ",pConfParty->GetName());
						    }
					  }
					else
					  {
					    PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateParty Can not update update party Wrong state");
					  }
					}
				}

			}
		}
	}
	// event was sent from Ema Send reply to Ema
	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pRsrvPartyAdd);
	if ((STATUS_OK == status) && (STATUS_OK != respStatus))
		pRequest->SetStatus(respStatus);
	else
		pRequest->SetStatus(status);


	return status;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CConfPartyManager::IsUpdateCompleted()
{
	if(0 != m_UpdateInfo.size())
		return FALSE;
  	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CConfPartyManager::IsUpdateCompleted(DWORD confId)
{
	BYTE result = FALSE;
	CUpdateInfo* pTemp = NULL;
	std::vector< CUpdateInfo* >::iterator itr;
	for(itr = m_UpdateInfo.begin(); itr != m_UpdateInfo.end(); ++itr)
	{
		pTemp = (*itr);
		if(pTemp->m_conferId==confId)
			break;
		pTemp = NULL;
	}

	if(pTemp)
		result = TRUE;
	return result;
}

/////////////////////////////////////////////////////////////////////////////
WORD CConfPartyManager::InsertUpdateInfo(CUpdateInfo* pUpdate_info )
{
	if (m_UpdateInfo.size() >= 80)
		return STATUS_FAIL;

	m_UpdateInfo.push_back(pUpdate_info);
  	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::DeleteUpdateInfo(WORD ind)
{
	if (ind >= m_UpdateInfo.size())
		return;

	std::vector< CUpdateInfo * >::iterator itr;
	for(itr = m_UpdateInfo.begin(); itr != m_UpdateInfo.end() && ind--; ++itr)
	{}
	CUpdateInfo* pTemp = (*itr);
	m_UpdateInfo.erase(itr);
	POBJDELETE(pTemp);
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnTimerDelParty(CSegment* pMsg)
{
  	int status = STATUS_OK;

  	/*** VALIDITY of Update list ***/
	PASSERT(m_UpdateInfo.size()==0);

  	/*** GET confId, pRsrvParty  from the Update list ***/
  	CUpdateInfo* pUpdateInfo = (*(m_UpdateInfo.begin()));
  	CRsrvParty* pRsrvParty = pUpdateInfo->m_pRsrvParty;
  	DWORD confId	= pUpdateInfo->m_conferId;
  	DWORD partyId   = pUpdateInfo->m_partyId;

  	/*** VALIDITY of Update info ***/
  	PASSERT_AND_RETURN(pRsrvParty==NULL);

  	/*** CHECK - is party droped from the conferense ***/
  	status = ::GetpConfDB()->SearchPartyName(confId,partyId);

  	/*** REPEAT for current element in list or GO TO NEXT ***/
  	if (status == STATUS_OK )
  	{
		//party is still in the conferense
		if (m_del_timer_num<8)
		{
			//repeat TIMER again
			m_del_timer_num++;
			StartTimer(DEL_PARTY_COMPLETE, 1 * SECOND);
			return;
		}
		else
		{
			//Failed to drop party
			PTRACE2(eLevelError,"CConfPartyManager::OnTimerDelParty. Update party failed. Party name: ", pRsrvParty->GetName());
			//Go to next party in the Update list; see further in the text
		}
	}
	/*** DROP party was SUCCESSFUL ***/
	if (status == STATUS_PARTY_DOES_NOT_EXIST)
	{
		// ADD party
		//*SETTINGS & RESOUCES for the party
		CCommConf* pConf = ::GetpConfDB()->GetCurrentConf(confId);
		// Bug fix, in the update scenario where we remove and add the party we will stay with the original party ID
	/*	DWORD nextPartyId = pConf->NextPartyId();
		WORD  setId_flag = TRUE;
		if (partyId > HALF_MAX_DWORD)
		{
			if (::GetpConfDB()->SearchPartyName(confId,partyId)!=STATUS_OK)
				setId_flag = FALSE;
		}

		//SET new partyId to party
		if (setId_flag)
			((CRsrvParty*)pRsrvParty)->SetPartyId(nextPartyId);
	 */
	 // klokwork Romem
	 if(pConf)
	 {
		 CConfApi confApi;
		 confApi.CreateOnlyApi(*(pConf->GetRcvMbx()));
		 confApi.AddParty(*pRsrvParty);
		 if (pRsrvParty->GetCascadeMode() == CASCADE_MODE_SLAVE && !pRsrvParty->GetVoice())
		 {
			 pConf->UpdateLectureModeAndLayoutBecauseSlaveInConf(pRsrvParty->GetName());
			 confApi.UpdateAutoLayout(NO);
			 confApi.SetVideoConfLayoutSeeMeAll(*pConf->GetVideoLayout());
			 confApi.UpdateLectureMode(pConf->GetLectureMode());
		 }

		 confApi.DestroyOnlyApi();

		 //START TIMER to check the result of add operation
		 m_add_timer_num = 0;
		 StartTimer(ADD_PARTY_COMPLETE, 1 * SECOND);
		 status = STATUS_IN_PROGRESS;
		 return;
	 }
	}
	else if ( STATUS_CONF_NOT_EXISTS == status)
	  {
	    TRACESTR (eLevelError) << "CConfPartyManager::OnTimerDelParty Update party: "
				      << pRsrvParty->GetName()<< " failed,Conference id: "
				      <<confId<< " Not Exists !";
	  }

	/*** DELETE current update info ***/
  	DeleteUpdateInfo(0);
  	m_del_timer_num = 0;

  	/*** NEXT party to update ***/
  	//Loop till  send 'Dropparty' == successful to conference or
  	//end of the list
  	while ( !IsUpdateCompleted())
  	{
		status = pUpdateInfo->SendDropParty();

		//if unsuccessful send - delete update info from the list
		if (status != STATUS_OK){
			PTRACE2(eLevelError,"CConfPartyManager::OnTimerDelParty. Update party failed. ",
					CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
			DeleteUpdateInfo(0);
		}
		//if successful send - exit from the loop
		if (status == STATUS_OK)
			break;
  	}

  	//start TIMER - if successful send
  	if (!IsUpdateCompleted())
		StartTimer(DEL_PARTY_COMPLETE, 1 * SECOND);
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnTimerAddParty(CSegment* pMsg)
{
	int status = STATUS_OK;

  	/*** VALIDITY of Update info ***/
  	PASSERT(m_UpdateInfo.size()==0);

	CUpdateInfo* pUpdateInfo = (*(m_UpdateInfo.begin()));
  	DWORD partyId   = pUpdateInfo->m_pRsrvParty->GetPartyId();
  	DWORD confId	= pUpdateInfo->m_conferId;

	/*** CHECK - is party in the target conferense  ***/
  	status = ::GetpConfDB()->SearchPartyName(confId,partyId);

  	/*** REPEAT for current element in list or GO TO NEXT ***/
  	if (status != STATUS_OK)
  	{
		if (m_add_timer_num<8){
			m_add_timer_num++;
			StartTimer(ADD_PARTY_COMPLETE, 1 * SECOND);
			return;
		}
		else{
			//Failed to Add party
			PTRACE2(eLevelError,"CConfPartyManager::OnTimerAddParty. Update party failed. Party name: ",
					pUpdateInfo->m_pRsrvParty->GetName());
			//Go to next party in the Update list; see further in the text
		}
  	}

	/*** DELETE current update info ***/
 	DeleteUpdateInfo(0);
  	m_add_timer_num = 0;

  	/*** NEXT party to update ***/
  	//Loop till  send 'Dropparty' == successful to conference or
  	//end of the list
  	while ( !IsUpdateCompleted())
  	{
  		pUpdateInfo = (*(m_UpdateInfo.begin()));
  		PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnTimerAddParty. List is not empty, next party to be deleted is:  ",
					pUpdateInfo->m_pRsrvParty->GetName());

		status = pUpdateInfo->SendDropParty();
		if (status != STATUS_OK){
			PTRACE2(eLevelError,"CConfPartyManager::OnTimerAddParty. Update party failed.",
					CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
			DeleteUpdateInfo(0);
		}
		if (status == STATUS_OK)
			break;
  	}
  	//start TIMER - if successful send
  	if (!IsUpdateCompleted())
		StartTimer(DEL_PARTY_COMPLETE, 1 * SECOND);
}

//////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnTimerDelMovedParty(CSegment* pMsg)
{
  	int status = STATUS_OK;

	PASSERT_AND_RETURN(m_MoveInfo.size()==0);

  	CRsrvParty* pRsrvPartyInfo = (*(m_MoveInfo.begin()));
  	PASSERT_AND_RETURN(pRsrvPartyInfo ==NULL);
  	CMoveInfo* partyMoveInfo = pRsrvPartyInfo->GetMoveInfo();
  	PASSERT_AND_RETURN(partyMoveInfo==NULL);
  	DWORD sourceConfId	= partyMoveInfo->GetPreviousConf();
  	DWORD destConfId    =  partyMoveInfo->GetCurrentConf();
  	DWORD partyId   = pRsrvPartyInfo->GetPartyId();

  	/*** CHECK - is party droped from the conferense ***/
  	status = ::GetpConfDB()->SearchPartyName(sourceConfId,partyId);

  	/*** REPEAT for current element in list or GO TO NEXT ***/
  	if (status == STATUS_OK )
  	{
		//Failed to drop party
		PTRACE2(eLevelError,"CConfPartyManager::OnTimerDelMovedParty. Cold Move party failed. Party name: ", pRsrvPartyInfo->GetName());
		//Go to next party in the Update list; see further in the text
	}
	/*** DROP party was SUCCESSFUL ***/
	if (status == STATUS_PARTY_DOES_NOT_EXIST)
	{
		// ADD party
		CCommConf* pDestConf = ::GetpConfDB()->GetCurrentConf(destConfId);
		PASSERT_AND_RETURN(!pDestConf);
		DWORD nextPartyId = pDestConf->NextPartyId();
		//SET new partyId to party
		if (pRsrvPartyInfo->GetPartyId() <= HALF_MAX_DWORD || pRsrvPartyInfo->GetPartyId() == 0xFFFFFFFF)
        	    pRsrvPartyInfo->SetPartyId(nextPartyId);

		CConfApi confApi;
		confApi.CreateOnlyApi(*(pDestConf->GetRcvMbx()));
		confApi.AddParty(*pRsrvPartyInfo);
		confApi.DestroyOnlyApi();

		//START TIMER to check the result of add operation
		//m_add_timer_num = 0;
		StartTimer(ADD_MOVED_PARTY_COMPLETE, 1 * SECOND);
		status = STATUS_IN_PROGRESS;
		return;
	}
	else if ( STATUS_CONF_NOT_EXISTS == status)
	{
	    TRACESTR (eLevelError) << "CConfPartyManager::OnTimerDelMovedParty Move party: "
				      << pRsrvPartyInfo->GetName()<< " failed,Conference id: "
				      <<destConfId<< " Not Exists !";
	}

	/*** DELETE current move info ***/

	DeleteMovePartyInfo(0);
  	//m_del_timer_num = 0;


	while ( !IsMoveCompleted())
	{
		PASSERT_AND_RETURN(m_MoveInfo.size()==0);
		pRsrvPartyInfo = (*(m_MoveInfo.begin()));
		PASSERT_AND_RETURN(pRsrvPartyInfo ==NULL);
		partyMoveInfo = pRsrvPartyInfo->GetMoveInfo();
		PASSERT_AND_RETURN(partyMoveInfo==NULL);

		destConfId = partyMoveInfo->GetCurrentConf();
		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(sourceConfId);

		if(!CPObject::IsValidPObjectPtr(pCurConf))
		{
		   PTRACE2(eLevelError,"CConfPartyManager::OnTimerDelMovedParty. Cold move of  party failed. Source Conf not found. Party Name: ",pRsrvPartyInfo->GetName());
		   DeleteMovePartyInfo(0);
		}
		else
		{
		   CConfParty* pMovedConfParty = pCurConf->GetCurrentParty( pRsrvPartyInfo->GetName());
		   if(!CPObject::IsValidPObjectPtr(pMovedConfParty))
		   {
			   PTRACE2(eLevelError,"CConfPartyManager::OnTimerDelMovedParty. Cold move of  party failed. Party does not exist in Source Conf. Party Name: ",pRsrvPartyInfo->GetName());
			   DeleteMovePartyInfo(0);
		   }
		   else{
			   CConfApi confApi;
			   confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			   confApi.DropParty(pRsrvPartyInfo->GetName());
			   confApi.DestroyOnlyApi();
			   break;

		   }
		}
	}

	// Light timer if cols move is on the way
  	if ( !IsMoveCompleted())
  	{
		StartTimer(DEL_MOVED_PARTY_COMPLETE, 1 * SECOND);
  	}
}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnTimerAddMovedParty(CSegment* pMsg)
{
	int status = STATUS_OK;

  	/*** VALIDITY of Update info ***/
  	PASSERT_AND_RETURN(m_MoveInfo.size()==0);

  	/*** GET pRsrvParty  from Cold Move list ***/
  	CRsrvParty* pRsrvPartyInfo = (*(m_MoveInfo.begin()));
  	/*** VALIDITY of RsrvParty info ***/
  	PASSERT_AND_RETURN(pRsrvPartyInfo ==NULL);
  	CMoveInfo* partyMoveInfo = pRsrvPartyInfo->GetMoveInfo();
  	DWORD sourceConfId	= partyMoveInfo->GetPreviousConf();
  	DWORD destConfId    =  partyMoveInfo->GetCurrentConf();
  	DWORD partyId   = pRsrvPartyInfo->GetPartyId();
	/*** CHECK - is party in the target conferense  ***/
  	status = ::GetpConfDB()->SearchPartyName(destConfId,pRsrvPartyInfo->GetName());

  	/*** REPEAT for current element in list or GO TO NEXT ***/
  	if (status != STATUS_OK)
  	{
	   PTRACE2(eLevelError,"CConfPartyManager::OnTimerAddMovedParty. Cold Move of Party failed. Party name: ",
	   pRsrvPartyInfo->GetName());
  	}

	/*** DELETE current update info ***/
  	DeleteMovePartyInfo(0);
  	//m_add_timer_num = 0;

  	/*** NEXT party to move ***/
  	//Loop till  send 'Dropparty' == successful to conference or
  	//end of the list
  	if ( !IsMoveCompleted())
  	{
  		CRsrvParty* pRsrvPartyInfo = (*(m_MoveInfo.begin()));
  		/*** VALIDITY of RsrvParty info ***/
  		PASSERT_AND_RETURN(pRsrvPartyInfo ==NULL);
  		PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnTimerAddMovedParty. List is not empty, next party to be moved is:  ",
  		pRsrvPartyInfo->GetName());
  		CMoveInfo* partyMoveInfo = pRsrvPartyInfo->GetMoveInfo();
  		DWORD sourceConfId	= partyMoveInfo->GetPreviousConf();
  		DWORD destConfId    =  partyMoveInfo->GetCurrentConf();
  		DWORD partyId   = pRsrvPartyInfo->GetPartyId();

  		const char* partyName = ::GetpConfDB()->GetPartyName(sourceConfId, partyId);
  		if(!partyName)
  		{
  		   PTRACE(eLevelInfoNormal, "CConfPartyManager::OnTimerAddMovedParty: Party is not found  in Source Conf - Dump CMoveInfo object and ignore cold move operation");
  		   CMoveInfo*  pReservedMoveInfo = pRsrvPartyInfo->GetMoveInfo();
  		   if (IsValidPObjectPtr(pReservedMoveInfo))
  		   {
  			  pReservedMoveInfo->Dump();
  			  DeleteMovePartyInfo(0);
  		   }
  		   DBGPASSERT(1);
  		   return;
  		}
  		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(sourceConfId);

  		if(pCurConf)
  		{
  			CConfApi confApi;
  			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
  			confApi.DropParty(partyName);
  			confApi.DestroyOnlyApi();
  			StartTimer(DEL_MOVED_PARTY_COMPLETE, 1 * SECOND);
  		}
     }

}
//////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnAddDialOutPartyToConf(CSegment* pSeg)
{
    PTRACE(eLevelInfoNormal, "ConfPartyManager::OnAddDialOutPartyToConf : New Party Name : ");
	DWORD wMonitorConfID;
	WORD listId;
	int iStatus = STATUS_OK;
	BYTE interfaceType = H323_INTERFACE_TYPE, isTel = FALSE;
	ALLOCBUFFER(pPartyData, MaxAddressListSize);
	ALLOCBUFFER(pReferUri, MaxAddressListSize);
	ALLOCBUFFER(pRefferedByStr, MaxAddressListSize);
	ALLOCBUFFER(pMSAssociatedStr, MaxAddressListSize);
	char* temp = NULL;

	*pSeg 	>> wMonitorConfID
			>> listId
			>> interfaceType
			>> pPartyData
			>> isTel
			>> pReferUri
			>> pRefferedByStr
			>> pMSAssociatedStr;

	// check parameters first
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(wMonitorConfID);

	if (!pCommConf)
		iStatus = STATUS_CONF_NOT_EXISTS;

	if(iStatus == STATUS_OK)
	{


		if(STATUS_OK == iStatus)
		{
			ALLOCBUFFER(userName,MaxAddressListSize);

			// Build reservation for the party
			CRsrvParty* undefParty = new CRsrvParty;
			undefParty->SetConnectionType(DIAL_OUT);
			undefParty->SetNetInterfaceType(interfaceType);
			//*******undefParty->SetMeet_me_method(MEET_ME_PER_MCU);
			//*******undefParty->SetBondingMode1(AUTO);
			undefParty->SetExtension(pMSAssociatedStr);
			// IpV6
			if(H323_INTERFACE_TYPE == interfaceType || SIP_INTERFACE_TYPE == interfaceType)
			{
				temp = strstr(pPartyData, "@");
				if(temp)
				{
					strncpy(userName, pPartyData, temp-pPartyData);
					if(SIP_INTERFACE_TYPE == interfaceType)
						undefParty->SetSipPartyAddress(pPartyData);
					else
					{
						if(H323_INTERFACE_TYPE == interfaceType)
							undefParty->SetH323PartyAlias(pPartyData);
					}
					mcTransportAddress ipAddr;
					memset(&ipAddr,0,sizeof(mcTransportAddress));
					undefParty->SetIpAddress(ipAddr);
					undefParty->SetCallSignallingPort(5060);
				}
				else
				{
					WORD port = 1720;
					if(SIP_INTERFACE_TYPE == interfaceType)
						port = 5060;
					//if contains port data
					temp = strstr(pPartyData, ":");

					mcTransportAddress IpAddr;
					memset(&IpAddr,0,sizeof(mcTransportAddress));

					stringToIp(&IpAddr,pPartyData);
					undefParty->SetIpAddress(IpAddr);
					undefParty->SetCallSignallingPort(port);
				}
			}

			if(isTel)
			{
				undefParty->SetVoice(YES);
				undefParty->SetNetChannelNumber(AUTO);
				undefParty->SetPhoneNumber(pPartyData);
				undefParty->SetIdentificationMethod(CALLING_PHONE_NUMBER_IDENTIFICATION_METHOD);
			}
			else
			{
				undefParty->SetVoice(pCommConf->IsAudioConf());
				//********undefParty->SetNetChannelNumber(pCommConf->GetNetChannelNumber());
			}

			undefParty->SetUndefinedType(UNRESERVED_PARTY);


			ALLOCBUFFER(h243name,H243_NAME_LEN);
			ALLOCBUFFER(confName_20,21);

			strncpy(confName_20, pCommConf->GetName() ,20 );
			confName_20[20]='\0';

			//used for adding a party via REFER
			if(strcmp("", pReferUri))
			{
				undefParty->SetRefferedToUri(pReferUri);
				undefParty->SetRefferedBy(pRefferedByStr);

				char* temp = strstr(pReferUri, "@");
				if(temp)
					*temp = '\0';
				strncpy(h243name, pReferUri, H243_NAME_LEN);
			}
			else
			{
				if(!strcmp(userName, ""))
					strncpy(h243name, pPartyData, H243_NAME_LEN);
				else
					strncpy(h243name, userName, H243_NAME_LEN);
				h243name[H243_NAME_LEN - 12] = '\0';
				strcat(h243name, "_(referred)");
			}

			undefParty->SetName(h243name);

			if (iStatus == STATUS_OK)
			{
				DWORD nextPartyId = pCommConf->NextPartyId();
				//SET new partyId to party
			    if (undefParty->GetPartyId() <= HALF_MAX_DWORD || undefParty->GetPartyId() == 0xFFFFFFFF)
						 ((CRsrvParty*)undefParty)->SetPartyId(nextPartyId);
			 }

			//Add party to the conference
			if (iStatus == STATUS_OK)
			{
				CConfApi confApi;
				confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
				confApi.AddParty(*undefParty, listId);
				confApi.DestroyOnlyApi();
			}

			DEALLOCBUFFER(userName);
			DEALLOCBUFFER(h243name);
			DEALLOCBUFFER(confName_20);
			delete undefParty;
		}
		else
		{
//****			PTRACE2(eLevelError,"ConfPartyManager::OnAddDialOutPartyToConf, status = ", iStatus);
		}
	}
	DEALLOCBUFFER(pPartyData);
	DEALLOCBUFFER(pReferUri);
	DEALLOCBUFFER(pRefferedByStr);
	DEALLOCBUFFER(pMSAssociatedStr);
}
/////////////////////////////////////////////////////////////////////////////
WORD CConfPartyManager::InsertMovePartyInfo(CRsrvParty* pMoveParty_info )
{
	if (m_MoveInfo.size() >= 80)
		return STATUS_FAIL;

	m_MoveInfo.push_back(pMoveParty_info);
  	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::DeleteMovePartyInfo(WORD ind)
{
	if (ind >= m_MoveInfo.size())
		return;

	std::vector< CRsrvParty * >::iterator itr;
	for(itr = m_MoveInfo.begin(); itr != m_MoveInfo.end() && ind--; ++itr)
	  {
	  }

	CRsrvParty* pTemp = (*itr);
	m_MoveInfo.erase(itr);
	POBJDELETE(pTemp);
}
/////////////////////////////////////////////////////////////////////////////
BYTE CConfPartyManager::IsMoveCompleted()
{
	if(0 != m_MoveInfo.size())
		return FALSE;
  	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetConfVideoLayout(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACEINTO << "Failed, No permission (ADMINISTRATOR_READONLY)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;

	CVideoLayoutDrv* pVideoLayoutDrv = new CVideoLayoutDrv;

	*pVideoLayoutDrv = *(CVideoLayoutDrv*)pRequest->GetRequestObject();
	CVideoLayout* pVideoLayout = pVideoLayoutDrv->GetVideoLayout();

	ConfMonitorID confId = pVideoLayoutDrv->GetConfID();

	std::ostringstream msg;
	std::ostringstream dsc;
	msg << "MonitorConfId:" << confId;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	if (NULL == pCommConf)
		status = STATUS_CONF_NOT_EXISTS;

	if (STATUS_OK == status && !pVideoLayout)
	{
		dsc << " - Invalid parameters";
		status = STATUS_INCONSISTENT_PARAMETERS;
	}

	if (STATUS_OK == status && pCommConf->IsConfSecured())
	{
		dsc << " - Layout cannot be changed in Secured conference";
		status = STATUS_CONF_IS_SECURED;
	}

	if (status == STATUS_OK && pCommConf->GetEntryQ())
	{
		dsc << " - Layout cannot be changed in EQ";
		status = STATUS_ILLEGAL;
	}

    if(status==STATUS_OK)
  	{
		if (pCommConf->GetManageTelepresenceLayoutInternaly() && pCommConf->GetTelePresenceModeConfiguration() == YES)
		{
			dsc << " - Layout cannot be changed because layout is managed internally in telepresence room switch mode";
			status = STATUS_CONFERENCE_LAYOUT_IS_MANAGED_INTERNALLY ;
			pRequest->SetExDescription(CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		}
	}

	if (status == STATUS_OK)
	{
		BYTE screenLayout = pVideoLayout->GetScreenLayout();
		BYTE isHDVSW = pCommConf->GetIsHDVSW();

		msg << ", Layout:" << CVideoLayout::LayoutTypeToString(screenLayout) << ", IsVSW:" << (int)isHDVSW;

		CVideoLayout* pVideoLayoutConf = pCommConf->GetVideoLayout(screenLayout);

		status = CheckVideoLayout(pVideoLayout, pVideoLayoutConf, isHDVSW);

		if (status == STATUS_OK && (isHDVSW && screenLayout != ONE_ONE))
		{
			dsc << " - In VSW conference the layout type must be 1x1";
			status = STATUS_ILLEGAL;
		}

		if (status == STATUS_OK)
		{
			CConfApi confApi;
			ModifyITPLayout(pVideoLayout, confId, FALSE);
			confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
			confApi.SetVideoConfLayoutSeeMeAll(*pVideoLayout);
			confApi.DestroyOnlyApi();
		}
	}

	TRACEINTO << msg.str().c_str() << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << dsc.str().c_str();

	std::string responseTrancsName("TRANS_CONF"); // Instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pVideoLayoutDrv);
	pRequest->SetStatus(status);

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::ModifyITPLayout(CVideoLayout* pVideoLayout,DWORD confId, BOOL bIsPrivate)
{
	PTRACE(eLevelInfoNormal, "ConfPartyManager::ModifyITPLayout");
	int numCell = 0;
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);

	PASSERT_AND_RETURN(NULL == pCommConf);

	vector<PartyPos> vecPartyPos;

	if (pVideoLayout->GetScreenLayout() == ONE_PLUS_TWO_OVERLAY_ITP)
	{
		if (pVideoLayout->m_numb_of_cell > 1 && pVideoLayout->m_pCellLayout[1] != NULL)
		{
			const DWORD force_partyId = pVideoLayout->m_pCellLayout[1]->GetForcedPartyId();
			if (force_partyId != 0xFFFFFFFF)
			{
				CConfParty* pForceParty = (CConfParty*)::GetpConfDB()->GetCurrentParty(confId,force_partyId);
				string _strForcePartyName = pForceParty->GetName();
				vector<string> key_list;
				key_list.push_back("_1");
				key_list.push_back("_2");
				key_list.push_back("_3");
				key_list.push_back("_4");
				size_t found = string::npos;
				for(unsigned int t = 0 ; t < key_list.size(); t++)
				{
				 	found = _strForcePartyName.find(key_list[t]);
					if (found != string::npos)
						break;
				}
				if (found != string::npos)
				{
					string _strTempForcePartyName = _strForcePartyName.erase(found,_strForcePartyName.size());
					CConfParty* tempParty = pCommConf->GetFirstParty();
					while (tempParty)
					{
						string _name = tempParty->GetName();
						// (strcmp(pForceParty->GetName(),_name.c_str()) != 0)
						{
							CConfParty* _curParty = pCommConf->GetCurrentParty(_name.c_str());
							if(NULL == _curParty)
							{
								PASSERT(1);
								tempParty = pCommConf->GetNextParty();
								continue;
							}
							found = _name.find(_strTempForcePartyName);
							if (found != string::npos)
							{
								found = _name.find("_");
								if (found != string::npos)
								{
									string num = _name.substr(found+1);
									if (num.size() > 0)
									{
										PartyPos party_pos;
										party_pos._pos = atoi(num.c_str());
										party_pos._partyId = _curParty->GetPartyId();
										vecPartyPos.push_back(party_pos);
										PTRACE2(eLevelInfoNormal, "ConfPartyManager::ModifyITPLayout - push:\n", _strTempForcePartyName.c_str());
									}
								}

							}
						}

						tempParty = pCommConf->GetNextParty();
					}

					if (vecPartyPos.size() > 0)
					{
						sort(vecPartyPos.begin(),vecPartyPos.end(),PartyPos::myCompare);

						if (vecPartyPos.size() == 2)
						{
							CVideoCellLayout  cellLayout ;
 							cellLayout.SetCellId(2);
							if (bIsPrivate)
								cellLayout.SetForcedPartyId(vecPartyPos[1]._partyId, BY_OPERATOR_THIS_PARTY);
							else
								cellLayout.SetForcedPartyId(vecPartyPos[1]._partyId,BY_OPERATOR_ALL_CONF);
							pVideoLayout->UpdateCell(cellLayout);

							cellLayout.SetCellId(3);
							if (bIsPrivate)
								cellLayout.SetForcedPartyId(vecPartyPos[0]._partyId, BY_OPERATOR_THIS_PARTY);
							else
								cellLayout.SetForcedPartyId(vecPartyPos[0]._partyId,BY_OPERATOR_ALL_CONF);
							pVideoLayout->UpdateCell(cellLayout);
						}
						else if (vecPartyPos.size() == 3)
						{
							CVideoCellLayout  cellLayout ;
							cellLayout.SetCellId(2);
							if (bIsPrivate)
								cellLayout.SetForcedPartyId(vecPartyPos[1]._partyId, BY_OPERATOR_THIS_PARTY);
							else
								cellLayout.SetForcedPartyId(vecPartyPos[1]._partyId,BY_OPERATOR_ALL_CONF);
							pVideoLayout->UpdateCell(cellLayout);

							cellLayout.SetCellId(3);
							if (bIsPrivate)
								cellLayout.SetForcedPartyId(vecPartyPos[0]._partyId, BY_OPERATOR_THIS_PARTY);
							else
								cellLayout.SetForcedPartyId(vecPartyPos[0]._partyId,BY_OPERATOR_ALL_CONF);
							pVideoLayout->UpdateCell(cellLayout);

							cellLayout.SetCellId(4);
							if (bIsPrivate)
								cellLayout.SetForcedPartyId(vecPartyPos[2]._partyId, BY_OPERATOR_THIS_PARTY);
							else
								cellLayout.SetForcedPartyId(vecPartyPos[2]._partyId,BY_OPERATOR_ALL_CONF);
							pVideoLayout->AddCell(cellLayout);

							pVideoLayout->SetScreenLayout(ONE_PLUS_THREE_OVERLAY_ITP);
						}
						else if (vecPartyPos.size() == 4)
						{

							CVideoCellLayout  cellLayout ;
							cellLayout.SetCellId(2);
							if (bIsPrivate)
								cellLayout.SetForcedPartyId(vecPartyPos[3]._partyId, BY_OPERATOR_THIS_PARTY);
							else
								cellLayout.SetForcedPartyId(vecPartyPos[3]._partyId,BY_OPERATOR_ALL_CONF);
							pVideoLayout->UpdateCell(cellLayout);

							cellLayout.SetCellId(3);
							if (bIsPrivate)
								cellLayout.SetForcedPartyId(vecPartyPos[1]._partyId, BY_OPERATOR_THIS_PARTY);
							else
								cellLayout.SetForcedPartyId(vecPartyPos[1]._partyId,BY_OPERATOR_ALL_CONF);
							pVideoLayout->UpdateCell(cellLayout);

							cellLayout.SetCellId(4);
							if (bIsPrivate)
								cellLayout.SetForcedPartyId(vecPartyPos[0]._partyId, BY_OPERATOR_THIS_PARTY);
							else
								cellLayout.SetForcedPartyId(vecPartyPos[0]._partyId,BY_OPERATOR_ALL_CONF);
							pVideoLayout->AddCell(cellLayout);

							cellLayout.SetCellId(5);
							if (bIsPrivate)
								cellLayout.SetForcedPartyId(vecPartyPos[2]._partyId, BY_OPERATOR_THIS_PARTY);
							else
								cellLayout.SetForcedPartyId(vecPartyPos[2]._partyId,BY_OPERATOR_ALL_CONF);
							pVideoLayout->AddCell(cellLayout);
							pVideoLayout->SetScreenLayout(ONE_PLUS_FOUR_OVERLAY_ITP);
						}
					}
				}
			}
		}
	}

	string str;
	pVideoLayout->ToString(str);

	PTRACE2(eLevelInfoNormal, "ConfPartyManager::ModifyITPLayout - Layout:\n", str.c_str());
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerSetPartyVideoLayout(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACEINTO << "Failed, No permission (ADMINISTRATOR_READONLY)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;

	CVideoLayoutPartyDrv* pVideoLayoutPartyDrv = new CVideoLayoutPartyDrv;

	*pVideoLayoutPartyDrv = *(CVideoLayoutPartyDrv*)pRequest->GetRequestObject();
	CVideoLayout* pVideoLayout = pVideoLayoutPartyDrv->GetVideoLayout();

	ConfMonitorID  confId  = pVideoLayoutPartyDrv->GetConfID();
	PartyMonitorID partyId = pVideoLayoutPartyDrv->GetPartyID();

	WORD isPrivate = pVideoLayoutPartyDrv->GetIsPrivate();

	std::ostringstream msg;
	std::ostringstream dsc;
	msg << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId << ", IsPrivateLayout:" << isPrivate;

	// VALIDITY of conferenceId and partyId
	CCommConf*  pCommConf = (CCommConf *)::GetpConfDB()->GetCurrentConf(confId);
	CConfParty* pParty    = (CConfParty*)::GetpConfDB()->GetCurrentParty(confId, partyId);

	if (NULL == pCommConf)
		status = STATUS_CONF_NOT_EXISTS;

	if (NULL == pParty)
		status = STATUS_PARTY_DOES_NOT_EXIST;

	if (STATUS_OK == status && !pVideoLayout)
	{
		dsc << " - Invalid parameters";
		status = STATUS_INCONSISTENT_PARAMETERS;
	}

	if (STATUS_OK == status && pCommConf->IsConfSecured())
	{
		dsc << " - Layout cannot be changed in Secured conference";
		status = STATUS_CONF_IS_SECURED;
	}

	if (status == STATUS_OK && pCommConf->GetIsCOP())
	{
		dsc << " - Layout cannot be changed in COP conference";
		status = STATUS_IN_EVENT_MODE_CONFERENCE_PERSONAL_LAYOUT_IS_FORBIDDEN;
	}

	if (status == STATUS_OK && pCommConf->GetIsHDVSW() && isPrivate)
	{
		dsc << " - Personal layout does not allowed in VSW conference";
		status = STATUS_IN_VSW_CONFERENCE_PRIVATE_LAYOUT_IS_FORBIDDEN;
	}

	if (STATUS_OK == status) // Check  if the same party is present in several cells.
	{
		if (pParty->GetVoice() || pParty->GetPartyState() == PARTY_SECONDARY || pParty->GetPartyState() == PARTY_DISCONNECTED || pParty->GetPartyState() == PARTY_REDIALING)
		{
			msg << ", IsVoice:" << (int)pParty->GetVoice() << ", PartyState:" << pParty->GetPartyState();
			status = STATUS_ILLEGAL_OPERATION_VIDEO_CAPABILITIES_INACTIVE;
		}
		else if (pCommConf->GetIsSameLayout())
		{
			msg << ", IsSameLayout:" << (int)pCommConf->GetIsSameLayout();
			status = STATUS_IN_SAME_LAYOUT_MODE_VIDEO_FORCE_IS_FORBIDDEN;
		}
		else
		{
			status = CheckVideoLayout(pVideoLayout, pParty->GetVideoLayout(), pCommConf->GetIsHDVSW(), isPrivate);
		}
	}

	if(STATUS_OK == status)
	{
		if (pCommConf->GetManageTelepresenceLayoutInternaly() && isPrivate)
		{
			dsc << " - Layout cannot be changed because layout is managed internally in telepresence room switch mode";
			status = STATUS_PERSONAL_LAYOUT_IS_MANAGED_INTERNALLY ;
			pRequest->SetExDescription(CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		}
	}

	if (status == STATUS_OK)
	{
		CConfApi confApi;
		confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
		ModifyITPLayout(pVideoLayout, confId, isPrivate);
		if (isPrivate)
				confApi.SetVideoPrivateLayout(pParty->GetName(), *pVideoLayout);
		else
			confApi.SetVideoConfLayoutSeeMeParty(pParty->GetName(), *pVideoLayout);
		confApi.DestroyOnlyApi();
	}

	TRACEINTO << msg.str().c_str() << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << dsc.str().c_str();

	std::string responseTrancsName("TRANS_CONF"); // Instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pVideoLayoutPartyDrv);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetPartyPrivateLayoutButtonOnly(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerSetPartyPrivateLayoutButtonOnly");
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
	{
        FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetPartyPrivateLayoutButtonOnly: No permission to OnServerSetPartyPrivateLayoutButtonOnly for administrator readony");
        pRequest->SetConfirmObject(new CDummyEntry());
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_NO_PERMISSION;
	}
   	STATUS status = STATUS_OK;

   	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;

   	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject() ;
   	pRequest->SetObjectFlag(STRING_FLAG);

   	const DWORD confId   = pRsrvPartyAction->GetConfID();
   	const DWORD partyId  = pRsrvPartyAction->GetPartyID();
   	const WORD isPrivate = (pRsrvPartyAction->GetParam1())?YES:NO;

	/*** VALIDITY of conferenceId and partyId ***/
	const CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	CConfParty* pParty = (CConfParty*)::GetpConfDB()->GetCurrentParty(confId,partyId);

	if (NULL == pCommConf)
		status = STATUS_CONF_NOT_EXISTS;

	if(NULL == pParty)
		status = STATUS_PARTY_DOES_NOT_EXIST;

	if(STATUS_OK == status)
	{
	  if(::GetpConfDB()->IsConfSecured(confId))  //check both cases if in that way and if in second way
	      {
			TRACESTR (eLevelError) <<"ConfPartyManager::OnServerSetPartyPrivateLayoutButtonOnly: In the Secured conference can not be changed the layout of any parties: ";
			status=STATUS_CONF_IS_SECURED;
	    }
	}

  	if (status == STATUS_OK)
  	{
		if((pParty->GetPartyState() == PARTY_SECONDARY)||(pParty->GetVoice()))
		{
			status = STATUS_INCONSISTENT_PARAMETERS;
		}
		else if(pCommConf->GetIsSameLayout())
		{
			status = STATUS_IN_SAME_LAYOUT_MODE_VIDEO_FORCE_IS_FORBIDDEN;
		}
  	}

//		if (status == STATUS_OK){
//			if (!m_pMoveMngr->IsMoveCompleted(confId,pPartyName))
//			  status = STATUS_MOVE_PARTY_NOT_COMPLETED;
// 		}

	if (status == STATUS_OK)
	{
		BYTE isHDVSW = pCommConf->GetIsHDVSW();
		BYTE isCOP = pCommConf->GetIsCOP();
		if(!isHDVSW && !isCOP)
		{
			const char* pPartyName = NULL;
			pPartyName = pCommConf->GetCurrentParty(partyId)->GetName();
			CConfApi confApi;
			confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
			confApi.SetVideoPrivateLayoutButtonOnly(pPartyName, isPrivate);
			//status = STATUS_IN_PROGRESS;
			confApi.DestroyOnlyApi();
		}
		else
		{
			PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerSetPartyPrivateLayoutButtonOnly ,No personal layouts in HD VSW/COP conf");
		}
	}

			  //Trace
  	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerSetPartyPrivateLayoutButtonOnly, "
		  , CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

			  //Confirm
	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);

  	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetAudioVolume(CRequest *pRequest)
{
  PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerSetAudioVolume");

  if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
  {
       FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetAudioVolume: No permission to OnServerSetAudioVolume for administrator readony");
       pRequest->SetConfirmObject(new CDummyEntry());
       pRequest->SetStatus(STATUS_NO_PERMISSION);
       return STATUS_NO_PERMISSION;
  }
   STATUS status = STATUS_OK;

   CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;

   *pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject() ;
   pRequest->SetObjectFlag(STRING_FLAG);

   const DWORD confId   = pRsrvPartyAction->GetConfID();
   const DWORD partyId  = pRsrvPartyAction->GetPartyID();
   const BYTE volume 	= pRsrvPartyAction->GetParam1();

	/*** VALIDITY of conferenceId and partyId ***/
	status = ::GetpConfDB()->SearchPartyName(confId,partyId);

	if((status==STATUS_OK))
	{

	  if(::GetpConfDB()->IsConfSecured(confId))  //check both cases if in that way and if in second way
	      {
			TRACESTR (eLevelError) <<"ConfPartyManager::OnServerSetAudioVolume: The Secured conference can not be changed ";
			status=STATUS_CONF_IS_SECURED;
	    }
	}

	if (status == STATUS_OK)
	{
		  const char* pPartyName = NULL;
		  const CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
  	      if (pCommConf !=NULL)
			  pPartyName = pCommConf->GetCurrentParty(partyId)->GetName();
		  else
			  status = STATUS_CONF_NOT_EXISTS;

		  if (status == STATUS_OK)
		  {
			  CConfApi confApi;
			  confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
			  confApi.SetAudioVolume(pPartyName, volume, eMediaIn);
			  //status = STATUS_IN_PROGRESS;
			  confApi.DestroyOnlyApi();
		  }
	  }

			  //Trace
  PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerSetAudioVolume, "
		  , CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

    ///new confirm
	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);
	//end new confirm

  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerWithdrawContentToken(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerWithdrawContentToken");
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerWithdrawContentToken: No permission to OnServerWithdrawContentToken for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
    }

	STATUS status = STATUS_OK;

	CConfAction* pConfAction = new CConfAction;

   *pConfAction = *(CConfAction*)pRequest->GetRequestObject() ;
    pRequest->SetObjectFlag(STRING_FLAG);  ///????

    const DWORD confId   = pConfAction->GetConfID();

  	/*** VALIDITY of conference Id ***/
  	if (::GetpConfDB()->FindId(confId) == NOT_FIND)
		  status = STATUS_CONF_NOT_EXISTS;

 	CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
  	if(pCurConf && pCurConf->IsActiveSlaveConf())
  	{
  		PTRACE(eLevelError,  "CConfPartyManager::OnServerWithdrawContentToken : Can Not Abort Srom Slave !!!!" );
  		status = STATUS_H239_SESSION_CANNOT_BE_ABORTED_FROM_A_SLAVE_CONFERENCE;
  	}

 	if (status==STATUS_OK)
  	{
 		if (pCurConf)
		{
 			CConfApi confApi;
			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			confApi.ContentTokenWithdraw();
			confApi.DestroyOnlyApi();
	 	}
		else
	 	{
	  		PTRACE(eLevelError,  "CConfPartyManager::OnServerWithdrawContentToken :  Conf Id invalid" );
	  		DBGPASSERT(1);
	  		status = STATUS_ILLEGAL;
	 	}
  	}

  	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerWithdrawContentToken, ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

  	//New Confirm
 	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_CONF_2
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);

	return STATUS_OK;

}


STATUS CConfPartyManager::OnServerSendContentRequest(CRequest *pRequest)
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSendContentRequest: No permission to OnServerSendContentRequest for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
    }

	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSendContentRequest - ERROR - system is not CG!!");
		return STATUS_ILLEGAL;
	}

	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerSendContentRequest");
	STATUS status = STATUS_OK;

	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;

	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject() ;
	 pRequest->SetObjectFlag(STRING_FLAG);

	 const DWORD confId   = pRsrvPartyAction->GetConfID();
	 const DWORD partyId  = pRsrvPartyAction->GetPartyID();

	 /*** VALIDITY of conferenceId and partyId ***/
	 status = ::GetpConfDB()->SearchPartyName(confId,partyId);
	 if (status == STATUS_OK)
	 {
	 	const char* pPartyName = NULL;
	 	const CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	    if (pCommConf !=NULL)
	 		 pPartyName = pCommConf->GetCurrentParty(partyId)->GetName();
	    else
	 		status = STATUS_CONF_NOT_EXISTS;

	 	 if (status == STATUS_OK)
	 	 {
	    	if (pCommConf->GetManageTelepresenceLayoutInternaly() && pCommConf->GetTelePresenceModeConfiguration() == YES)
	    	{
	    		TRACEINTO << "Can't start feature because layout is managed internally in telepresence room switch mode";
	    		status = STATUS_CONFERENCE_LAYOUT_IS_MANAGED_INTERNALLY ;
	    		pRequest->SetExDescription(CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
	    	}
	     }
	 	 if (status == STATUS_OK)
	 	 {
	 		PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerSendContentRequest-state o.k");
	 		 CConfParty* pConfParty = NULL;
	 		 pConfParty = pCommConf->GetCurrentParty(pPartyName);
	 		 if (pConfParty)
	 		 {
	 			PTRACE2INT(eLevelInfoNormal, "ConfPartyManager::OnServerSendContentRequest-state o.k-id is ", pConfParty->GetPartyId());
	 			CConfApi confApi;
	 		 	confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
	 			confApi.SendCGStartContent(pPartyName);
	 			confApi.DestroyOnlyApi();
	 		  }
	 	  }
	 }



	     ///new confirm
	 	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	 	pRequest->SetTransName(responseTrancsName);
	 	pRequest->SetConfirmObject(pRsrvPartyAction);
	 	pRequest->SetStatus(status);
	 	//end new confirm


	return STATUS_OK;

}


///////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerStopContentRequest(CRequest *pRequest)
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
          FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerStopContentRequest: No permission to OnServerStopContentRequest for administrator readony");
          pRequest->SetConfirmObject(new CDummyEntry());
          pRequest->SetStatus(STATUS_NO_PERMISSION);
          return STATUS_NO_PERMISSION;
    }

	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerStopContentRequest - ERROR - system is not CG!!");
		return STATUS_ILLEGAL;
	}

	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerStopContentRequest");
	STATUS status = STATUS_OK;

	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;

	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject() ;
	 pRequest->SetObjectFlag(STRING_FLAG);

	 const DWORD confId   = pRsrvPartyAction->GetConfID();
	 const DWORD partyId  = pRsrvPartyAction->GetPartyID();

	 /*** VALIDITY of conferenceId and partyId ***/
	 status = ::GetpConfDB()->SearchPartyName(confId,partyId);
	 if (status == STATUS_OK)
	 {
	 	const char* pPartyName = NULL;
	 	const CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	    if (pCommConf !=NULL)
	 		 pPartyName = pCommConf->GetCurrentParty(partyId)->GetName();
	    else
	 		status = STATUS_CONF_NOT_EXISTS;

	 	 if (status == STATUS_OK)
	 	 {
	 		PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerStopContentRequest-state o.k");
	 		 CConfParty* pConfParty = NULL;
	 		 pConfParty = pCommConf->GetCurrentParty(pPartyName);
	 		if (pConfParty)
	 		{
	 			CConfApi confApi;
	 			confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
	 			confApi.SendCGStopContent(pPartyName);
	 			confApi.DestroyOnlyApi();
	 		}

	 	  }
	 }



	     ///new confirm
	 	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	 	pRequest->SetTransName(responseTrancsName);
	 	pRequest->SetConfirmObject(pRsrvPartyAction);
	 	pRequest->SetStatus(status);
	 	//end new confirm


	return STATUS_OK;

}


//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetVisualName(CRequest *pRequest)
{
  PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerSetVisualName");
  if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
  {
        FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetVisualName: No permission to OnServerSetVisualName for administrator readony");
        pRequest->SetConfirmObject(new CDummyEntry());
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_NO_PERMISSION;
  }

   STATUS status = STATUS_OK;

   CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;

   *pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject() ;
   pRequest->SetObjectFlag(STRING_FLAG);

   const DWORD confId   = pRsrvPartyAction->GetConfID();
   const DWORD partyId  = pRsrvPartyAction->GetPartyID();
   const BYTE volume 	= pRsrvPartyAction->GetParam1();
   ALLOCBUFFER(new_name,H243_NAME_LEN);
   strncpy(new_name,pRsrvPartyAction->GetName(),H243_NAME_LEN);
   new_name[H243_NAME_LEN - 1] = '\0';

   //check if the party's current visual name isn't equals to the new name if it does we will just confirm with status OK
   BYTE isEqualsToCurrentVisualName = NO;
   const CCommConf* pCommConf = NULL;
   CConfParty* pParty = NULL;
   pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
   if (pCommConf !=NULL)
   	   pParty = pCommConf->GetCurrentParty(partyId);
   if(pParty)
   {
   		char* currentVisualName = pParty->GetVisualPartyName();
   		if (currentVisualName)
   		{
   			BYTE currentVisualNameLen = strlen(currentVisualName);
   			BYTE newNameLen = strlen(new_name);
   			if((currentVisualNameLen == newNameLen) && (strncmp(currentVisualName,new_name,newNameLen)==0))
   			{
   					isEqualsToCurrentVisualName = YES;
   					PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerSetVisualName equals to current visual name");

   			}
   		}
   }
   if(!isEqualsToCurrentVisualName)
   {
   		//check if the new visual name is already exist
	   status = ::GetpConfDB()->SearchPartyName(confId,new_name); //if status does not exist, the returned status is STATUS_PARTY_DOES_NOT_EXIST.
	   BYTE isEQConf = NO;
	   CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	   if(pCommConf!=NULL && IsValidPObjectPtr(pCommConf))
   			isEQConf = pCommConf->GetEntryQ();
	   status= IsVisualNameConflict(pCommConf->GetName(),new_name,isEQConf,partyId);

	   if(isEQConf && ((status == STATUS_PARTY_NAME_EXISTS)||(status == STATUS_PARTY_VISUAL_NAME_EXISTS)))
	   {
   			ALLOCBUFFER(updateVisualName,H243_NAME_LEN);
   			memset (updateVisualName,'\0',H243_NAME_LEN);
	   		::GetUpdatedVisualNameForPartyInEQ(new_name, updateVisualName);
   			memset (new_name,'\0',H243_NAME_LEN);
   			strncpy(new_name,updateVisualName,H243_NAME_LEN);
	   		DEALLOCBUFFER(updateVisualName);
   			status = STATUS_OK;
   	}

		/*** VALIDITY of conferenceId and partyId ***/
		if (status == STATUS_OK)
			status = ::GetpConfDB()->SearchPartyName(confId,partyId);

		if((status==STATUS_OK))
		{

		  if(::GetpConfDB()->IsConfSecured(confId))  //check both cases if in that way and if in second way
	      {
			TRACESTR (eLevelError) <<"ConfPartyManager::OnServerSetVisualName: In the Secured conference the participants can not be changed ";
			status=STATUS_CONF_IS_SECURED;
		    }
		}

		if (status == STATUS_OK)
		{
			  const char* pPartyName = NULL;
			  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
  		      if (pCommConf !=NULL)
				  pPartyName = pCommConf->GetCurrentParty(partyId)->GetName();
			  else
				  status = STATUS_CONF_NOT_EXISTS;

			  if (status == STATUS_OK)
			  {
			  	CConfParty* pConfParty = NULL;
				  	pConfParty = pCommConf->GetCurrentParty(pPartyName);
			  	if (pConfParty)
			  	{
					pConfParty->SetVisualPartyName(new_name);
					TRACEINTO << "pConfParty->GetCorrelationId() : " << pConfParty->GetCorrelationId();
					pCommConf->OperatorSetVisualName(new_name, partyId, pPartyName, pConfParty->GetCorrelationId());

					CConfApi confApi;
			 		confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
					confApi.SiteAndVisualName(pConfParty->GetPartyId(), new_name);
					confApi.RefreshLayout();
					confApi.DestroyOnlyApi();
		  		}
			  }
		  }
   }

	//Trace
	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerSetVisualName, "
	  , CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

	 ///new confirm
	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);
	//end new confirm
	DEALLOCBUFFER(new_name);
	return STATUS_OK;
}



//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetAGC(CRequest *pRequest)
{
   PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerSetAGC");
   if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
   {
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetAGC: No permission to OnServerSetAGC for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
   }

   STATUS status = STATUS_OK;

   CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;

   *pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject() ;
   pRequest->SetObjectFlag(STRING_FLAG);

   const DWORD confId   = pRsrvPartyAction->GetConfID();
   const DWORD partyId  = pRsrvPartyAction->GetPartyID();
   const BOOL  isAGC 	= pRsrvPartyAction->GetParam1();

	/*** VALIDITY of conferenceId and partyId ***/
	status = ::GetpConfDB()->SearchPartyName(confId,partyId);

	 if((status==STATUS_OK))
	{

	  if(::GetpConfDB()->IsConfSecured(confId))  //check both cases if in that way and if in second way
	     {
			TRACESTR (eLevelError) <<"ConfPartyManager::OnServerSetAGC: The Secured conference can not be changed ";
			status=STATUS_CONF_IS_SECURED;
		  }
	}
	if (status == STATUS_OK)
	{
		  const char* pPartyName = NULL;
		  const CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
  	      if (pCommConf !=NULL)
			  pPartyName = pCommConf->GetCurrentParty(partyId)->GetName();
		  else
			  status = STATUS_CONF_NOT_EXISTS;

		  if (status == STATUS_OK)
		  {
		  	CConfParty* pConfParty = NULL;
		  	pConfParty = pCommConf->GetCurrentParty(pPartyName);
		  	if (pConfParty)
				pConfParty->SetAGC(isAGC);

			EOnOff eOnOff = eOff;
			eOnOff = isAGC ? eOn : eOff;
			CConfApi confApi;
		 	confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
			confApi.UpdateAGCExecFlag(pPartyName,eOnOff);
			confApi.DestroyOnlyApi();
		  }
	  }

  PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerSetAGC, ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

    ///new confirm
	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);
	//end new confirm

  return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerSetLeader(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACESTRFUNC(eLevelError) << "No permission (for administrator read-only)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;

	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;
	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject();

	const ConfMonitorID  confId  = pRsrvPartyAction->GetConfID();
	const PartyMonitorID partyId = pRsrvPartyAction->GetPartyID();
	const BOOL           bLeader = pRsrvPartyAction->GetParam1();

	std::ostringstream msg;
	msg << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId << ", IsLeader:" << (int)bLeader;

	TRACEINTO << msg.str().c_str();

	BYTE isLastChairPersonDroppedPermission = FALSE;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	if (!pCommConf)
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Can not find conference";
		status = STATUS_CONF_NOT_EXISTS;
	}
	if (status == STATUS_OK && pCommConf->IsConfSecured())
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Conference is secured";
		status = STATUS_CONF_IS_SECURED;
	}

	CConfParty* pConfParty = NULL;
	if (status == STATUS_OK)
	{
		pConfParty = pCommConf->GetCurrentParty(partyId);
		if (!pConfParty)
		{
			TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Can not find party";
			status = STATUS_PARTY_DOES_NOT_EXIST;
		}
		if (status == STATUS_OK && CASCADE_MODE_NONE != pConfParty->GetCascadeMode())
		{
			TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Cascade link cannot be set as ChairPerson";
			status = STATUS_CASCADE_LINK_CANNOT_BE_SET_AS_CHAIRPERSON;
		}
		if (status == STATUS_OK && STATUS_PARTY_IVR == pConfParty->GetOrdinaryParty())
		{
			TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Party in IVR, so cannot be set as ChairPerson";
			status = STATUS_PARTY_IN_IVR_CANNOT_BE_SET_AS_CHAIRPERSON;
		}
		// Nizar: Here we drop party if it was chairperson and there are no other chairs in conference in order to terminate conference
		if (status == STATUS_OK)
		{
			if (pConfParty->GetIsLeader() == eOn && bLeader == eOff && pCommConf->GetTerminateConfAfterChairDroppedOnOff() == TRUE && pCommConf->IsConfHasDifferentChairPerson(partyId) == FALSE)
			{
				status = DisconnectParty(confId, partyId, m_operName);
				if (status == STATUS_OK)
					isLastChairPersonDroppedPermission = TRUE;
			}
		}
	}

	if (status == STATUS_OK && isLastChairPersonDroppedPermission == FALSE)
	{
		EOnOff eOnOff = bLeader ? eOn : eOff;
#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
		CConfApi confApi(confId);
#else
		CConfApi confApi;
#endif

		confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
		confApi.SetPartyAsLeader(pConfParty->GetName(), eOnOff);
		confApi.DestroyOnlyApi();
	}

	pRequest->SetTransName("TRANS_CONF"); // Instead of TRANS_RES_1
	pRequest->SetObjectFlag(STRING_FLAG);
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerSetBillingData(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACESTRFUNC(eLevelError) << "No permission (for administrator read-only)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;

	CConfAction* pConfAction = new CConfAction;
	*pConfAction = *(CConfAction*)pRequest->GetRequestObject();
	pRequest->SetObjectFlag(STRING_FLAG);

	const ConfMonitorID  confId = pConfAction->GetConfID();

	/*** VALIDITY of conference Id ***/
	CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
	if (!pCurConf)
		status = STATUS_CONF_NOT_EXISTS;

	if (status == STATUS_OK && pCurConf->IsConfSecured())
		status = STATUS_CONF_IS_SECURED;

	if (status == STATUS_OK)
	{
		pCurConf->SetBillingData(pConfAction->GetBillingData());

		CStructTm curTime;
		SystemGetTime(curTime);

		CConfStartCont4* pConfStartCont4 = new CConfStartCont4;
		pConfStartCont4->SetConfBillingInfo(pConfAction->GetBillingData());

		CCdrEvent cdrEvent;
		cdrEvent.SetCdrEventType(CONFERENCE_START_CONTINUE_4);
		cdrEvent.SetTimeStamp(curTime);
		cdrEvent.SetConfStartCont4(pConfStartCont4);
		POBJDELETE(pConfStartCont4);

		CCdrLogApi cdrApi;
		cdrApi.ConferenceEvent(confId, cdrEvent);
	}

	TRACEINTO << "MonitorConfId:" << confId << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();

	//std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_RES_1
	//pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerConfContactInfo(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACESTRFUNC(eLevelError) << "No permission (for administrator read-only)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;

	CConfAction* pConfAction = new CConfAction;
	*pConfAction = *(CConfAction*)pRequest->GetRequestObject();
	pRequest->SetObjectFlag(STRING_FLAG);

	const ConfMonitorID confId = pConfAction->GetConfID();

	/*** VALIDITY of conference Id ***/
	CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
	if (!pCurConf)
		status = STATUS_CONF_NOT_EXISTS;

	if (status == STATUS_OK && pCurConf->IsConfSecured())
		status = STATUS_CONF_IS_SECURED;

	if (status == STATUS_OK)
	{
		pCurConf->SetConfContactInfo(pConfAction->GetContactInfo(0), 0);
		pCurConf->SetConfContactInfo(pConfAction->GetContactInfo(1), 1);
		pCurConf->SetConfContactInfo(pConfAction->GetContactInfo(2), 2);

		CStructTm curTime;
		SystemGetTime(curTime);

		CConfStartCont4* pConfStartCont4 = new CConfStartCont4;

		for (int j = 0; j < MAX_CONF_INFO_ITEMS; j++)
		{
			const char* pStrContactInfo = pConfAction->GetContactInfo(j);
			if (pStrContactInfo)
				pConfStartCont4->SetContactInfo(pStrContactInfo, j);
		}

		CCdrEvent cdrEvent;
		cdrEvent.SetCdrEventType(CONFERENCE_START_CONTINUE_4);
		cdrEvent.SetTimeStamp(curTime);
		cdrEvent.SetConfStartCont4(pConfStartCont4);
		POBJDELETE(pConfStartCont4);

		CCdrLogApi cdrApi;
		cdrApi.ConferenceEvent(confId, cdrEvent);
	}

	TRACEINTO << "MonitorConfId:" << confId << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();

	//std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_RES_1
	//pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerPartyContactInfo(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACESTRFUNC(eLevelError) << "No permission (for administrator read-only)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;

	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;
	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject();
	pRequest->SetObjectFlag(STRING_FLAG);

	const ConfMonitorID  confId  = pRsrvPartyAction->GetConfID();
	const PartyMonitorID partyId = pRsrvPartyAction->GetPartyID();

	/*** VALIDITY of conferenceId and partyId ***/
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	if (!pCommConf)
		status = STATUS_CONF_NOT_EXISTS;

	if (status == STATUS_OK && pCommConf->IsConfSecured())
		status = STATUS_CONF_IS_SECURED;

	if (status == STATUS_OK)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyId);
		if (!pConfParty)
		{
			status = STATUS_PARTY_NOT_EXISTS;
		}
		else
		{
			pConfParty->SetUserDefinedInfo(pRsrvPartyAction->GetContactInfo(0), 0);
			pConfParty->SetUserDefinedInfo(pRsrvPartyAction->GetContactInfo(1), 1);
			pConfParty->SetUserDefinedInfo(pRsrvPartyAction->GetContactInfo(2), 2);
			pConfParty->SetUserDefinedInfo(pRsrvPartyAction->GetContactInfo(3), 3);
			pConfParty->SetAdditionalInfo (pRsrvPartyAction->GetAddionalInfo());
		}
	}

	TRACEINTO << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();

	pRequest->SetTransName("TRANS_CONF"); // Instead of TRANS_RES_1
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerSetListeningAudioVolume(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACESTRFUNC(eLevelError) << "No permission (for administrator read-only)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;

	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;
	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject();
	pRequest->SetObjectFlag(STRING_FLAG);

	const ConfMonitorID  confId  = pRsrvPartyAction->GetConfID();
	const PartyMonitorID partyId = pRsrvPartyAction->GetPartyID();
	const BYTE volume            = pRsrvPartyAction->GetParam1();

	/*** VALIDITY of conferenceId and partyId ***/
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	if (!pCommConf)
		status = STATUS_CONF_NOT_EXISTS;

	if (status == STATUS_OK && pCommConf->IsConfSecured())
		status = STATUS_CONF_IS_SECURED;

	if (status == STATUS_OK)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyId);
		if (!pConfParty)
		{
			status = STATUS_PARTY_NOT_EXISTS;
		}
		else
		{
			CConfApi confApi;
			confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
			confApi.SetAudioVolume(pConfParty->GetName(), volume, eMediaOut);
			confApi.DestroyOnlyApi();
		}
	}

	TRACEINTO << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId << ", Volume:" << (int)volume << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();

	pRequest->SetTransName("TRANS_CONF");
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerSetAudioVideoMute(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACESTRFUNC(eLevelError) << "No permission (for administrator read-only)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;

	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;
	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject();
	pRequest->SetObjectFlag(STRING_FLAG);

	const ConfMonitorID  confId  = pRsrvPartyAction->GetConfID();
	const PartyMonitorID partyId = pRsrvPartyAction->GetPartyID();
	const BOOL isAudioMuted      = pRsrvPartyAction->GetParam1();
	const BOOL isVideoMuted      = pRsrvPartyAction->GetParam2();

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	if (!pCommConf)
		status = STATUS_CONF_NOT_EXISTS;

	if (status == STATUS_OK && pCommConf->IsConfSecured())
		status = STATUS_CONF_IS_SECURED;

	if (status == STATUS_OK)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyId);
		if (!pConfParty)
		{
			status = STATUS_PARTY_NOT_EXISTS;
		}
		else
		{
			switch (pConfParty->GetPartyState())
			{
				case PARTY_CONNECTED:
				case PARTY_SECONDARY:
				case PARTY_CONNECTED_WITH_PROBLEM:
				case PARTY_CONNECTED_PARTIALY:
					break;

				default:
					status = STATUS_ILLEGAL;
					break;
			}

			if (status == STATUS_OK)
			{
				// Send command to conference
				CConfApi confApi;
				confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
				SendPartyAudioVideoMute(pConfParty, &confApi, isAudioMuted, isVideoMuted);

				if (pConfParty->IsTIPMasterParty())
				{
					TRACEINTO << "MonitorPartyId:" << partyId << " - TIP master party";
					CConfParty* pTmpConfParty = pCommConf->GetFirstParty();
					while (pTmpConfParty)
					{
						// master and slave parties have same room Id
						if (pTmpConfParty != pConfParty && (pTmpConfParty->GetRoomId() == pConfParty->GetRoomId()))
						{
							TRACEINTO << "MonitorPartyId:" << pTmpConfParty->GetPartyId() << " - TIP slave party";
							BOOL muteSlaveFromConf = TRUE;
							SendPartyAudioVideoMute(pTmpConfParty, &confApi, isAudioMuted, isVideoMuted, muteSlaveFromConf);
						}
						pTmpConfParty = pCommConf->GetNextParty();
					}
				}
				confApi.DestroyOnlyApi();
			}
		}
	}

	TRACEINTO << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId << ", isAudioMuted:" << (WORD)isAudioMuted << ", isVideoMuted:" << (WORD)isVideoMuted << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();

	pRequest->SetTransName("TRANS_CONF"); // instead of TRANS_RES_1
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CConfPartyManager::SendPartyAudioVideoMute(CConfParty* pConfParty, CConfApi* confApi, BOOL isAudioMuted, BOOL isVideoMuted, BOOL muteSlaveFromConf/* = FALSE */)
{
	if (!CPObject::IsValidPObjectPtr(pConfParty))
		return;

	const char* pPartyName = pConfParty->GetName();
	if (!muteSlaveFromConf && pConfParty->IsTIPSlaveParty())
	{
		TRACEINTO << "PartyName:" << pPartyName << " - Failed, Action does not allowed for TIP slave party";
		return;
	}

	TRACEINTO << "PartyName:" << pPartyName << ", isAudioMuted:" << (WORD)isAudioMuted << ", isVideoMuted:" << (WORD)isVideoMuted;

	DWORD  mediaMask = 0;
	EOnOff eOnOff    = eOff;
	if (pConfParty->IsAudioMutedByOperator() != isAudioMuted)
	{
		eOnOff    = isAudioMuted ? eOn : eOff;
		mediaMask = 0x00000001;
		confApi->MuteMedia(pPartyName, eOnOff, mediaMask);
	}

	if (pConfParty->IsVideoMutedByOperator() != isVideoMuted)
	{
		eOnOff    = isVideoMuted ? eOn : eOff;
		mediaMask = 0x00000002;
		confApi->MuteMedia(pPartyName, eOnOff, mediaMask);
	}
}

//--------------------------------------------------------------------------
void CConfPartyManager::SendPartyAudioBlock(CConfParty* pConfParty, CConfApi* confApi, BOOL isAudioBlocked, BOOL blockSlaveFromConf/* = FALSE */)
{
	if (!CPObject::IsValidPObjectPtr(pConfParty))
		return;

	const char* pPartyName = pConfParty->GetName();
	if (!blockSlaveFromConf && pConfParty->IsTIPSlaveParty())
	{
		TRACEINTO << "PartyName:" << pPartyName << " - Failed, Action does not allowed for TIP slave party";
		return;
	}

	TRACEINTO << "PartyName:" << pPartyName << ", isAudioBlocked:" << (WORD)isAudioBlocked;

	DWORD  mediaMask = 0;
	EOnOff eOnOff    = eOff;
	if (pConfParty->IsAudioBlocked() != isAudioBlocked)
	{
		eOnOff    = isAudioBlocked ? eOn : eOff;
		mediaMask = 0x00000001;
		confApi->BlockMedia(pPartyName, eOnOff, mediaMask);
	}
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerSetAudioBlock(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACESTRFUNC(eLevelError) << "No permission (for administrator read-only)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;

	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;
	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject();
	pRequest->SetObjectFlag(STRING_FLAG);

	const ConfMonitorID  confId  = pRsrvPartyAction->GetConfID();
	const PartyMonitorID partyId = pRsrvPartyAction->GetPartyID();
	const BOOL isAudioBlocked    = pRsrvPartyAction->GetParam1();

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	if (!pCommConf)
		status = STATUS_CONF_NOT_EXISTS;

	if (status == STATUS_OK && pCommConf->IsConfSecured())
		status = STATUS_CONF_IS_SECURED;

	if (status == STATUS_OK)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyId);
		if (!pConfParty)
		{
			status = STATUS_PARTY_NOT_EXISTS;
		}
		else
		{
			switch (pConfParty->GetPartyState())
			{
				case PARTY_CONNECTED:
				case PARTY_SECONDARY:
				case PARTY_CONNECTED_WITH_PROBLEM:
				case PARTY_CONNECTED_PARTIALY:
					break;

				default:
					status = STATUS_ILLEGAL;
					break;
			}

			if (status == STATUS_OK)
			{
				CConfApi confApi;
				confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
				SendPartyAudioBlock(pConfParty, &confApi, isAudioBlocked);

				if (pConfParty->IsTIPMasterParty())
				{
					TRACEINTO << "MonitorPartyId:" << partyId << " - TIP master party";
					CConfParty* pTmpConfParty = pCommConf->GetFirstParty();
					while (pTmpConfParty)
					{
						// master and slave parties have same room Id
						if (pTmpConfParty != pConfParty && (pTmpConfParty->GetRoomId() == pConfParty->GetRoomId()))
						{
							TRACEINTO << "MonitorPartyId:" << pTmpConfParty->GetPartyId() << " - TIP slave party";
							BOOL blockSlaveFromConf = TRUE;
							SendPartyAudioBlock(pTmpConfParty, &confApi, isAudioBlocked, blockSlaveFromConf);
						}
						pTmpConfParty = pCommConf->GetNextParty();
					}
				}
				confApi.DestroyOnlyApi();
			}
		}
	}

	TRACEINTO << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId << ", isAudioBlocked:" << (WORD)isAudioBlocked << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();

	pRequest->SetTransName("TRANS_CONF"); // instead of TRANS_RES_1
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerUpdateConfAutoLayout(CRequest *pRequest)
{
  PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateConfAutoLayout");
  if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
  {
       FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateConfAutoLayout: No permission to OnServerUpdateConfAutoLayout for administrator readony");
       pRequest->SetConfirmObject(new CDummyEntry());
       pRequest->SetStatus(STATUS_NO_PERMISSION);
       return STATUS_NO_PERMISSION;
  }

   STATUS status = STATUS_OK;

   CConfAction* pConfAction = new CConfAction;

   *pConfAction = *(CConfAction*)pRequest->GetRequestObject() ;
   pRequest->SetObjectFlag(STRING_FLAG);

   const DWORD confId   = pConfAction->GetConfID();
   BYTE isAutoLayout   = (pConfAction->GetNumAction())?YES:NO;

  /*** VALIDITY of conference Id ***/
    CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
    if (pCommConf == NULL)
		  status = STATUS_CONF_NOT_EXISTS;

  if((status==STATUS_OK))
   {

	  if(::GetpConfDB()->IsConfSecured(confId))  //check both cases if in that way and if in second way
	      {
			TRACESTR (eLevelError) <<"ConfPartyManager::OnServerUpdateConfAutoLayout: The Secured conference can not be changed ";
			status=STATUS_CONF_IS_SECURED;
	    }
  }


  if(status ==STATUS_OK)
  {
	  if (pCommConf->GetManageTelepresenceLayoutInternaly() && pCommConf->GetTelePresenceModeConfiguration() == YES)
	  {
		  TRACEINTO << "Can't start feature because layout is managed internally in telepresence room switch mode";
		  status = STATUS_CONFERENCE_LAYOUT_IS_MANAGED_INTERNALLY ;
		  pRequest->SetExDescription(CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
	  }
  }


  if (status==STATUS_OK)
  {
	  CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);

	  if (pCurConf&&status==STATUS_OK)
	  {
		  CConfApi confApi;
		  confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
		  confApi.UpdateAutoLayout( isAutoLayout );
		  confApi.DestroyOnlyApi();
	  }
	  else
	  {
		  PTRACE(eLevelError,  "CConfPartyManager::OnServerUpdateConfAutoLayout :  Conf Id invalid" );
		  DBGPASSERT(1);
		  status = STATUS_ILLEGAL;
	  }

  }
	//Trace
	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateConfAutoLayout ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());


  	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerStartPreview(CRequest* pRequest)
{
	STATUS status = STATUS_OK;

	CPartyPreviewDrv* pPartyPreviewDrv = new CPartyPreviewDrv;
	*pPartyPreviewDrv = *(CPartyPreviewDrv*)pRequest->GetRequestObject();

	DWORD       confId          = pPartyPreviewDrv->GetConfID();
	DWORD       partyId         = pPartyPreviewDrv->GetPartyID();
	DWORD       RemoteIPAddress = pPartyPreviewDrv->GetRemoteIP();
	DWORD       VideoPort       = pPartyPreviewDrv->GetVideoPort();
	DWORD       AudioPort       = pPartyPreviewDrv->GetAudioPort();
	WORD        Direction       = pPartyPreviewDrv->GetDirection();
	CCommConf*  pCommConf       = GetpConfDB()->GetCurrentConf(confId);
	CConfParty* pParty          = pCommConf ? pCommConf->GetCurrentParty(partyId) : NULL;
	DWORD       PartyState      = pParty ? pParty->GetPartyState() : PARTY_DISCONNECTED;
	WORD        IsVideoMuted    = pParty ? pParty->IsVideoMutedByOperator() : TRUE;

	std::ostringstream msg;
	msg << "CConfPartyManager::OnServerStartPreview:"
	    << "\n  MonitorConfId   :" << confId
	    << "\n  MonitorPartyId  :" << partyId
	    << "\n  Direction       :" << Direction
	    << "\n  RemoteIPAddress :" << RemoteIPAddress
	    << "\n  VideoPort       :" << VideoPort
	    << "\n  AudioPort       :" << AudioPort
	    << "\n  PartyState      :" << PartyState
	    << "\n  IsVideoMuted    :" << IsVideoMuted;

	pRequest->SetObjectFlag(STRING_FLAG);

	BOOL bEnablVideoPreview = NO;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if (pSysConfig)
		pSysConfig->GetBOOLDataByKey(ENABLE_VIDEO_PREVIEW, bEnablVideoPreview);

	if (bEnablVideoPreview == NO)
		status = STATUS_VIDEO_PREVIEW_DISABLED;
	else if (!pCommConf)
		status = STATUS_CONF_NOT_EXISTS;
	else if (!pParty)
		status = STATUS_PARTY_DOES_NOT_EXIST;
	else if (Direction == 0 /*view participant sent video*/ && IsVideoMuted)
		status = STATUS_VIDEO_PREVIEW_DISABLED;
	else if (GetpConfDB()->IsConfSecured(confId))
		status = STATUS_CONF_IS_SECURED;
	else if (PartyState != PARTY_CONNECTED && PartyState != PARTY_SECONDARY && PartyState != PARTY_CONNECTED_WITH_PROBLEM && PartyState != PARTY_CONNECTED_PARTIALY)
		status = STATUS_ILLEGAL;
	else if (RemoteIPAddress == 0 || RemoteIPAddress == 0xFFFFFFFF)
		status = STATUS_IP_ADDRESS_NOT_VALID;
	else if (VideoPort == 0 || VideoPort == 0xFFFF)
		status = STATUS_IP_ADDRESS_NOT_VALID;

	if (STATUS_OK != status)
	{
		msg << "\n  Action          :" << "Failed, status:" << status << ", " << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
		TRACEINTO << msg.str().c_str();

		pRequest->SetTransName("TRANS_CONF");
		pRequest->SetConfirmObject(pPartyPreviewDrv);
		pRequest->SetStatus(status);
		return status;
	}
	msg << "\n  Action          :" << "Succeeded, Forward request to Resource Allocator";
	TRACEINTO << msg.str().c_str();

	pRequest->SetRequestObject(pPartyPreviewDrv);
	return STATUS_FW_REQUEST_TO_RESOURCE_MANGER;
}

//--------------------------------------------------------------------------
void CConfPartyManager::OnRsrcStartPreviewInd(CSegment* pMsg)
{
	START_PREVIEW_IND_PARAMS_S startPreview;
	pMsg->Get((BYTE*)&startPreview, sizeof(startPreview));

	TRACECOND_AND_RETURN(startPreview.status != STATUS_OK, "CConfPartyManager::OnRsrcStartPreviewInd - Start Preview was rejected by Resource Allocator, Status:" << startPreview.status);

	CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(startPreview.monitor_conf_id);
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pCurConf));

	TRACEINTO << "ConfPartyManager::OnRsrcStartPreviewInd - Forward request to Conference";

	CPartyPreviewDrv* pPartyPreviewReq = new CPartyPreviewDrv;
	pPartyPreviewReq->SetConfID(startPreview.monitor_conf_id);
	pPartyPreviewReq->SetPartyID(startPreview.monitor_party_id);
	pPartyPreviewReq->SetDirection(startPreview.m_Direction);
	pPartyPreviewReq->SetRemoteIP(startPreview.m_RemoteIPAddress);
	pPartyPreviewReq->SetAudioPort(startPreview.m_AudioPort);
	pPartyPreviewReq->SetVideoPort(startPreview.m_VideoPort);

	// Send event to Conference
	CConfApi confApi;
	confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
	confApi.SendStartPreviewReqToParty(pPartyPreviewReq);
	confApi.DestroyOnlyApi();
	POBJDELETE(pPartyPreviewReq);
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerStopPreview(CRequest* pRequest)
{
	STATUS status = STATUS_OK;

	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;
	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject();

	DWORD       confId    = pRsrvPartyAction->GetConfID();
	DWORD       partyId   = pRsrvPartyAction->GetPartyID();
	WORD        Direction = pRsrvPartyAction->GetParam1();
	CCommConf*  pCommConf = GetpConfDB()->GetCurrentConf(confId);
	CConfParty* pParty    = pCommConf ? pCommConf->GetCurrentParty(partyId) : NULL;

	std::ostringstream msg;
	msg << "CConfPartyManager::OnServerStopPreview:"
	    << "\n  MonitorConfId   :" << confId
	    << "\n  MonitorPartyId  :" << partyId
	    << "\n  Direction       :" << Direction;

	pRequest->SetObjectFlag(STRING_FLAG);

	if (!pCommConf)
		status = STATUS_CONF_NOT_EXISTS;
	else if (!pParty)
		status = STATUS_PARTY_DOES_NOT_EXIST;
	else if (GetpConfDB()->IsConfSecured(confId))
		status = STATUS_CONF_IS_SECURED;

	if (STATUS_OK != status)
	{
		msg << "\n  Action          :" << "Failed, status:" << status << ", " << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
		TRACEINTO << msg.str().c_str();

		pRequest->SetTransName("TRANS_CONF");
		pRequest->SetConfirmObject(pRsrvPartyAction);
		pRequest->SetStatus(status);
		return status;
	}

	msg << "\n  Action          :" << "Succeeded, Forward request to Resource Allocator";
	TRACEINTO << msg.str().c_str();

	CPartyPreviewDrv* pStopPartyPreviewReq = new CPartyPreviewDrv;
	pStopPartyPreviewReq->SetConfID(confId);
	pStopPartyPreviewReq->SetPartyID(partyId);
	pStopPartyPreviewReq->SetDirection(Direction);

	pRequest->SetRequestObject(pStopPartyPreviewReq);
	POBJDELETE(pRsrvPartyAction);
	return STATUS_FW_REQUEST_TO_RESOURCE_MANGER;
}

//--------------------------------------------------------------------------
void CConfPartyManager::OnRsrcStopPreviewInd(CSegment* pMsg)
{
	STOP_PREVIEW_IND_PARAMS_S* pStopPreview = new STOP_PREVIEW_IND_PARAMS_S;
	pMsg->Get((BYTE*)(pStopPreview), sizeof(STOP_PREVIEW_IND_PARAMS_S));

	STATUS status    = (STATUS)pStopPreview->status;
	DWORD  confId    = pStopPreview->monitor_conf_id;
	DWORD  partyId   = pStopPreview->monitor_party_id;
	WORD   direction = pStopPreview->m_Direction;

	POBJDELETE(pStopPreview);

	TRACECOND_AND_RETURN(status != STATUS_OK, "CConfPartyManager::OnRsrcStopPreviewInd - Stop Preview was rejected by Resource Allocator, Status:" << status);

	CCommConf* pCurConf = GetpConfDB()->GetCurrentConf(confId);
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pCurConf));

	CConfParty* pParty = pCurConf->GetCurrentParty(partyId);
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	// Send event to Conference
	cmCapDirection channelDirection = (direction) ? cmCapTransmit : cmCapReceive;

	TRACEINTO << "ConfPartyManager::OnRsrcStopPreviewInd - Forward request to Conference";

	CConfApi confApi;
	confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
	confApi.SendStopPreviewReqToParty(pParty->GetName(), channelDirection);
	confApi.DestroyOnlyApi();
}


/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnRsrcExtractPartyInfo(CSegment* pMsg)
{
	DWORD confNameLen = 0, partyNameLen = 0;
	pMsg->Get( confNameLen );
	char   confName[H243_NAME_LEN];
	memset(&confName, 0, H243_NAME_LEN);
	pMsg->Get( (BYTE*)(&confName), confNameLen );

	pMsg->Get( partyNameLen );
	char   partyName[H243_NAME_LEN];
	memset(&partyName, 0, H243_NAME_LEN);
	pMsg->Get( (BYTE*)(&partyName), partyNameLen );

	TRACEINTO << " CConfPartyManager::OnRsrcExtractPartyInfo : conf name = " <<  confName << ", party name = " << partyName;

	BOOL conf_found = FALSE, party_found = FALSE;

	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pCommConf = pCommConfDB->GetFirstCommConf();
	while( IsValidPObjectPtr(pCommConf) )
	{
		if( !strcmp(confName, pCommConf->GetName()) )
		{
			conf_found = TRUE;
			break;
		}
		pCommConf = pCommConfDB->GetNextCommConf();
	}
	if( conf_found )
	{
		CConfParty* pConfParty = pCommConf->GetFirstParty();
		while( pConfParty )
		{
			if( !strcmp( partyName, pConfParty->GetName()) )
			{
				party_found = TRUE;
				break;
			}
			pConfParty = pCommConf->GetNextParty();
		}
		if( party_found )
		{
			DWORD confID = pCommConf->GetMonitorConfId();
			DWORD partyID = pConfParty->GetPartyId();
			TRACEINTO << " CConfPartyManager::OnRsrcExtractPartyInfo : found conf id = " <<  confID <<", party id = " << partyID;

			//Send Answer Msg to the RA process
			CSegment*  ret_seg = new CSegment;
			ret_seg->Put( (BYTE *)(&confID), sizeof(DWORD) );
			ret_seg->Put( (BYTE *)(&partyID), sizeof(DWORD) );

			ResponedClientRequest(STATUS_OK,ret_seg);
		}
	}
	STATUS ret_status = ( conf_found && party_found ) ? STATUS_OK : STATUS_FAIL;
	return ret_status;
}
//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerRequestIntra(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerRequestIntra");

	STATUS status = STATUS_OK;
	cmCapDirection channelDirection = cmCapReceive;

	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;

	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject() ;
	pRequest->SetObjectFlag(STRING_FLAG);

	const DWORD confId   		= pRsrvPartyAction->GetConfID();
	const DWORD partyId  		= pRsrvPartyAction->GetPartyID();
	const BYTE  Direction 	    = pRsrvPartyAction->GetParam1();

	if(Direction)
		channelDirection = cmCapTransmit;


	/*** VALIDITY of conferenceId and partyId ***/
	const CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	CConfParty* pParty = (CConfParty*)::GetpConfDB()->GetCurrentParty(confId,partyId);

	if (NULL == pCommConf)
		status = STATUS_CONF_NOT_EXISTS;

	if(NULL == pParty)
		status = STATUS_PARTY_DOES_NOT_EXIST;

	 if(STATUS_OK == status)
	 {
		 if(::GetpConfDB()->IsConfSecured(confId))  //check both cases if in that way and if in second way
		 {
			 TRACESTR (eLevelError) <<"ConfPartyManager::OnServerRequestIntra: Can not preview party video in Secured conference ";
			 status=STATUS_CONF_IS_SECURED;
		  }
	 }

	 if(STATUS_OK == status)
	 {
		 const char* pPartyName = NULL;
		 pPartyName = pCommConf->GetCurrentParty(partyId)->GetName();
		 CConfApi confApi;
		 confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
		 confApi.SendRequestVideoPreviewIntra(pPartyName,channelDirection);
		 confApi.DestroyOnlyApi();
	 }

//Trace
	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerRequestIntra, "
		  , CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

    ///new confirm
	std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);
	//end new confirm

  	return STATUS_OK;

}
//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerUpdateEndTime(CRequest *pRequest)
{
  PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateEndTime");
  if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
  {
        FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateEndTime: No permission to OnServerUpdateEndTime for administrator readony");
        pRequest->SetConfirmObject(new CDummyEntry());
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_NO_PERMISSION;
  }

   STATUS status = STATUS_OK;
   CSetEndTime* pSetEndTime = new CSetEndTime;

   *pSetEndTime = *(CSetEndTime*)pRequest->GetRequestObject() ;
   pRequest->SetObjectFlag(STRING_FLAG);
   const DWORD confId   = pSetEndTime->GetConfID();
   const CStructTm* pTime = pSetEndTime->GetTime();

  /*** VALIDITY of conference Id ***/
  if (::GetpConfDB()->FindId(confId) == NOT_FIND)
		  status = STATUS_CONF_NOT_EXISTS;
  else if(!pTime->IsValid()) status = STATUS_ILLEGAL_END_TIME;


   if((status==STATUS_OK))
	{

	  if(::GetpConfDB()->IsConfSecured(confId))  //check both cases if in that way and if in second way
	     {
			TRACESTR (eLevelError) <<"ConfPartyManager::OnServerUpdateEndTime: The Secured conference can not be changed ";
			status=STATUS_CONF_IS_SECURED;
		  }
	}
  if (status==STATUS_OK)
  {
	  CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
	  CStructTm curTime;
	  PASSERT(SystemGetTime(curTime));
	  if(curTime >= (*pTime)){
	    TRACESTR (eLevelError) << "CConfPartyManager::OnServerUpdateEndTime :  Conf End Time is in the past" ;
	    status = STATUS_ILLEGAL_END_TIME;
	  }

	  // ===== 2. get the response

	 // pCurConf->OperatorSetEndTime(*pTime, m_operName);
	  	// End Romem 13.7.08
	  //Make sure the duration is not over limit
	  const CStructTm*  pStartTime = pCurConf->GetStartTime();
	  DWORD  newDurationTime = *pTime - *pStartTime; //Duration in seconds
	  DWORD maxDurationTimeInSeconds = MAXIMUM_CONF_DURATION*3600;

	  if (status==STATUS_OK && maxDurationTimeInSeconds < newDurationTime){
			TRACESTR (eLevelError)
					<< "ConfPartyManager::OnServerUpdateEndTime duration is over limit: newDuration is:"
					<< newDurationTime
					<< ",while maxDurationTimeInSeconds is: "
					<< maxDurationTimeInSeconds;
	    status = STATUS_ILLEGAL_END_TIME;
	  }


	  /*  Romem 13.7.08
	  if (pCurConf&&status==STATUS_OK)
	  {
		  CConfApi confApi;
		  confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
		  confApi.SetEndTime(*pTime);
		  confApi.DestroyOnlyApi();
	  }
	  else
	  {*/
	  if(status==STATUS_OK)
	  {
	  	   if(!pCurConf)
	  	   {
		      PTRACE(eLevelError,  "CConfPartyManager::OnServerUpdateEndTime :  Conf Id invalid" );
		      DBGPASSERT(1);
		      status = STATUS_ILLEGAL;
	  	   }
	  }

	  if(status==STATUS_OK && pCurConf->IsPermanent())
	  {
	  	   PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateEndTime setting end time can't be done in Permanent conference", pCurConf->IsPermanent());
	  	   std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	  	   pRequest->SetTransName(responseTrancsName);
	  	   pRequest->SetConfirmObject(pSetEndTime);
	  	   DWORD sts = STATUS_ILLEGAL; // not defined which status to use for rejecting
	  	   pRequest->SetStatus(sts);
	  }else{
		  if ( status )
	  {
	  		//Trace
			  PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateEndTime ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

			  std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
			  pRequest->SetTransName(responseTrancsName);
			  pRequest->SetConfirmObject(pSetEndTime);
			  pRequest->SetStatus(status);
			  DBGPASSERT(1);
		  }else
		  {
			  pRequest->SetRequestObject(pSetEndTime);
			  return STATUS_FW_REQUEST_TO_RESOURCE;
		  }
	  }
  }
	//Trace
    /* Romem 13.7.08
	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateEndTime ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

  	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pTimeDrv);
	pRequest->SetStatus(status);
	*/
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerUpdateVisualEffects(CRequest *pRequest)
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
        FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateVisualEffects: No permission to OnServerUpdateVisualEffects for administrator readony");
        pRequest->SetConfirmObject(new CDummyEntry());
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_NO_PERMISSION;
    }
	//This function always returns STATUS_ILLEGAL in carmel - we removed the option to change visual effects in ongoing conference. Hagai E Decision (5/2006)

	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateVisualEffects");
	//STATUS status = STATUS_OK;

	CVisualEffectsParamsDrv* pVisualEffectsDrv = new CVisualEffectsParamsDrv;

	*pVisualEffectsDrv = *(CVisualEffectsParamsDrv*)pRequest->GetRequestObject() ;
	/*CVisualEffectsParams* pVisualEffects = pVisualEffectsDrv->GetVisualEffectsParams();

	const DWORD confId   = pVisualEffectsDrv->GetConfID();

	//VALIDITY of conferenceId
	if (::GetpConfDB()->FindId(confId) == NOT_FIND)
		status = STATUS_CONF_NOT_EXISTS;

	if (status==STATUS_OK && pVisualEffects)
	{
		const CCommConf* pCommConf = (const CCommConf*) ::GetpConfDB()->GetCurrentConf(confId);

		CConfApi confApi;
		confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
		confApi.UpdateVisualEffects(pVisualEffects);
		confApi.DestroyOnlyApi();
	}
	*/
	STATUS status = STATUS_ILLEGAL;

	//Trace
	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateVisualEffects : Set Visual Effects is completed, "
		, CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pVisualEffectsDrv);
	pRequest->SetStatus(status);

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerUpdateConfLectureModeParams(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateConfLectureModeParams");
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateConfLectureModeParams: No permission to OnServerUpdateConfLectureModeParams for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
    }

	STATUS status = STATUS_OK;

	CLectureModeParamsDrv* pLectureModeDrv = new CLectureModeParamsDrv;

	*pLectureModeDrv = *(CLectureModeParamsDrv*)pRequest->GetRequestObject() ;
	CLectureModeParams* pLectureMode = pLectureModeDrv->GetLectureModeParams();

	const DWORD confId   = pLectureModeDrv->GetConfID();

	const CCommConf* pCommConf = (const CCommConf*) ::GetpConfDB()->GetCurrentConf(confId);

	if(pCommConf)
	{


	  ((CCommConf*)pCommConf)->SetSetLectureModeSameLayoutRules(pLectureMode);
		status = pCommConf->TestLectureModeValidity(pLectureMode);
	}
	else
		status = STATUS_CONF_NOT_EXISTS;

	if((status==STATUS_OK))
	{

	  if(::GetpConfDB()->IsConfSecured(confId))  //check both cases if in that way and if in second way
	      {
			TRACESTR (eLevelError) <<"ConfPartyManager::OnServerUpdateConfLectureModeParams: The Secured conference can not be changed ";
			status=STATUS_CONF_IS_SECURED;
	    }
	}

	if( status == STATUS_OK )
	{
		if (pCommConf->GetManageTelepresenceLayoutInternaly())
		{
			TRACEINTO << "Can't start feature because layout is managed internally in telepresence room switch mode";
			status = STATUS_CONFERENCE_LAYOUT_IS_MANAGED_INTERNALLY ;
			pRequest->SetExDescription(CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		}
	}

	if( status == STATUS_OK )
	{
		// VNGR-22639: won't try restore lecturer video layout if any party is deleted
		// otherwise the logic might be complex?
		CLectureModeParams* pSavedLectureMode;
		CVideoLayout*  pSavedLecturerVideoLayout;

		((CCommConf*)pCommConf)->GetLecturerVideoLayout(pSavedLectureMode, pSavedLecturerVideoLayout);

		if (pSavedLectureMode)
		{
			POBJDELETE(pSavedLectureMode);
			POBJDELETE(pSavedLecturerVideoLayout);
			((CCommConf*)pCommConf)->SaveLecturerVideoLayout(pSavedLectureMode, pSavedLecturerVideoLayout);
		}

		CConfApi confApi;
		confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
		confApi.UpdateLectureMode(pLectureMode);
		confApi.UpdateContentLectureMode(pCommConf->IsExclusiveContentMode());
		confApi.DestroyOnlyApi();
	}

		  //Trace
	PTRACE2(eLevelInfoNormal,"CConfMngr::SetConfLectureMode : Set conf lecture mode request, "
			, CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

			  //Confirm
	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pLectureModeDrv);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerSetConnectOrDisconnect(CRequest* pRequest) // shiraITP - 120
{
	// SEND ConnectDisConnect to Reservation Module  wait for ACK on SUCCESS
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACESTRFUNC(eLevelError) << "No permission (for administrator read-only)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;

	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;
	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject();

	const ConfMonitorID  confId  = pRsrvPartyAction->GetConfID();
	const PartyMonitorID partyId = pRsrvPartyAction->GetPartyID();
	const int actionNumber       = pRsrvPartyAction->GetNumAction();

	std::ostringstream msg;
	msg << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId << ", ActionNumber:" << actionNumber;

	TRACEINTO << msg.str().c_str();

	switch (actionNumber)
	{
		case RECONNECT_PARTY:
		{
			status = ReconnectParty(confId, partyId);
			break;
		}
		case DISCONNECT_PARTY:
		{
			status = DisconnectParty(confId, partyId, m_operName);     // shiraITP - 121
			break;
		}
	}

	pRequest->SetObjectFlag(STRING_FLAG);
	pRequest->SetTransName("TRANS_CONF");                      // Instead of TRANS_CONF_2
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::DisconnectParty(ConfMonitorID confId, PartyMonitorID partyId, const char* operName) // shiraITP - 121
{
	std::ostringstream msg;
	msg << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId;

	TRACEINTO << msg.str().c_str();

	// Check the conference
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	if (!pCommConf)
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Can not find conference";
		return STATUS_CONF_NOT_EXISTS;
	}
	if (pCommConf->Is_TerminatingState())
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Conference is in terminating state";
		return STATUS_TERMINATING_CONFERENCE;
	}
	if (pCommConf->IsConfSecured())
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Conference is secured";
		return STATUS_CONF_IS_SECURED;
	}

	// Check the party
	CConfParty* pConfParty = pCommConf->GetCurrentParty(partyId);
	if (!pConfParty)
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Can not find party";
		return STATUS_PARTY_DOES_NOT_EXIST;
	}
	if (!pConfParty->IsAllowDisconnect())
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << ", State:" << pConfParty->GetPartyState() << " - Failed, Invalid party state";
		return STATUS_CONNECT_DISCONNECT_BLOCKED;
	}
	if (PARTY_DISCONNECTED == pConfParty->GetPartyState())
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << "Failed, Party already disconnected";
		return STATUS_OK; // Should be STATUS_OK in this case
	}

	// Send DISCONNECT party to conference
	pConfParty->SetOperatorName(operName);
	char* partyName = (char*)pConfParty->GetName();

	CConfApi confApi;
	confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));

	if (pConfParty->IsUndefinedParty())
		confApi.DropParty(partyName, 0 /*delete*/);     // shiraITP - 122
	else
		confApi.DropParty(partyName, 1 /*disconnect*/); // shiraITP - 122

	confApi.DestroyOnlyApi();

	// cdr
	((CCommConf*)pCommConf)->OperatorPartyAction(partyName, partyId, m_operName, OPERATOR_DISCONNECTE_PARTY);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::ReconnectParty(ConfMonitorID confId, PartyMonitorID partyId)
{
	std::ostringstream msg;
	msg << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId;

	TRACEINTO << msg.str().c_str();

	// Check the conference
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	if (!pCommConf)
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Can not find conference";
		return STATUS_CONF_NOT_EXISTS;
	}
	if (pCommConf->Is_TerminatingState())
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Conference is in terminating state";
		return STATUS_TERMINATING_CONFERENCE;
	}
	if (pCommConf->IsConfSecured())
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Conference is secured";
		return STATUS_CONF_IS_SECURED;
	}

	// Check the party
	CConfParty* pConfParty = pCommConf->GetCurrentParty(partyId);
	if (!pConfParty)
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Can not find party";
		return STATUS_PARTY_DOES_NOT_EXIST;
	}
	if (!pConfParty->IsAllowReconnect())
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << ", State:" << pConfParty->GetPartyState() << " - Failed, Invalid party state";
		return STATUS_CONNECT_DISCONNECT_BLOCKED;
	}
	if (DIAL_OUT != pConfParty->GetConnectionTypeOper()) // bug fix vngr-3600 Allow Reconnect only in dial out
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << " - Failed, Cannot reconnect party because it's connection type is not Dial Out";
		return STATUS_CONNECT_DISCONNECT_BLOCKED;
	}
	if (PARTY_CONNECTED ==  pConfParty->GetPartyState())
	{
		TRACESTRFUNC(eLevelError) << msg.str().c_str() << "Failed, Party already connected";
		return STATUS_OK; // Should be STATUS_OK in this case
	}

	char* partyName = (char*)pConfParty->GetName();

	// Send RECONNECT party to conference
	CConfApi confApi;
	confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
	confApi.ReconnectParty(partyName, 0);
	confApi.DestroyOnlyApi();

	// cdr
	((CCommConf*)pCommConf)->OperatorPartyAction(partyName, partyId, m_operName, OPERATOR_RECONNECT_PARTY);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnConfIpServiceParamsInd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnConfIpServiceParamsInd ");
	STATUS insertStatus = STATUS_OK;
	// ===== 1. get the new ConfIpServiceParams
	CONF_IP_PARAMS_S *pNewMConfIpServiceParams = (CONF_IP_PARAMS_S*)pSeg->GetPtr();

	TRACEINTO << CConfIpParamWrapper(*pNewMConfIpServiceParams);

	// ===== 2. fill CMfaTask's attribute with data from structure received
	CConfIpParameters* pTempConfIpParams = new CConfIpParameters;
	pTempConfIpParams->SetData(pNewMConfIpServiceParams);

	// check if the new service has a different prefix or name than what we know
	CMedString cstr;
	BYTE needToUpdateAllRsrvPrefixes = 1;
	CConfIpParameters* oldServiceParams = m_pProcess->FindIpService(pTempConfIpParams->GetServiceId());
//	PTRACE2INT(eLevelInfoNormal, "CConfPartyManager::OnConfIpServiceParamsInd - SipServerType: ",pTempConfIpParams->GetSipServerType());
	if (CPObject::IsValidPObjectPtr(oldServiceParams))
	{
		cstr << "Service already exists in list\n";
		if(!strcmp((char*)oldServiceParams->GetServiceName(),(char*)pTempConfIpParams->GetServiceName()))
		{
			cstr << "Service names are the same:";
			needToUpdateAllRsrvPrefixes = 0;
		}
		else
		{
			cstr << "Service names are different:";
			needToUpdateAllRsrvPrefixes = 1;
		}
		cstr <<	"\nold name: " << (char*)oldServiceParams->GetServiceName() << "  , new name: " << (char*)pTempConfIpParams->GetServiceName() << "\n";

		if(!strcmp((char*)oldServiceParams->GetDialIn().aliasContent,(char*)pTempConfIpParams->GetDialIn().aliasContent))
		{
			cstr << "Service Prefixes are the same:";
			needToUpdateAllRsrvPrefixes |= 0;
		}
		else
		{
			cstr << "Service Prefixes are different:";
			needToUpdateAllRsrvPrefixes |= 1;
		}
		cstr << "\nold prefix: " << (char*)oldServiceParams->GetDialIn().aliasContent << "  , new prefix: " << (char*)pTempConfIpParams->GetDialIn().aliasContent << "\n";
	}
	else
	{
		cstr << "service is new\n";
	}
	cstr << "needToUpdateAllRsrvPrefixes? " << needToUpdateAllRsrvPrefixes;
	PTRACE(eLevelInfoNormal,cstr.GetString());


	BYTE needToUpdateICEconfiguration = TRUE;
	if (CPObject::IsValidPObjectPtr(oldServiceParams))
	{
		PTRACE2INT(eLevelInfoNormal, "CConfPartyManager::OnConfIpServiceParamsInd - oldServiceParams->GetICEType(): ",oldServiceParams->GetICEType());
			PTRACE2INT(eLevelInfoNormal, "CConfPartyManager::OnConfIpServiceParamsInd - pTempConfIpParams->GetICEType(): ",pTempConfIpParams->GetICEType());


		if(oldServiceParams->GetICEType() == pTempConfIpParams->GetICEType())
		{
			PTRACE(eLevelInfoNormal, "CConfPartyManager::OnConfIpServiceParamsInd - No chages in ICE setting ");
			needToUpdateICEconfiguration = FALSE;
		}
	}


	if(needToUpdateICEconfiguration)
	{
		// obsolete!!
		//BOOL bIsEnableGenericICE = 0;
		//std::string key = "ENABLE_STANDART_ICE";
		//CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, bIsEnableGenericICE);

		if(pTempConfIpParams->GetICEType()==eIceEnvironment_ms || (pTempConfIpParams->GetICEType()==eIceEnvironment_Standard) /*|| (pTempConfIpParams->GetICEType()==eIceEnvironment_WebRtc)*/) // && bIsEnableGenericICE))
		{
			PTRACE(eLevelInfoNormal, "CConfPartyManager::OnConfIpServiceParamsInd - changes in ICE setting- ICE is ON - Wait for initialization to end");
			if(m_pProcess->m_IceInitializationStatus == eIceStatusOFF)
			{
				PTRACE(eLevelInfoNormal, "CConfPartyManager::OnConfIpServiceParamsInd - Start timer for ICE initialization");
				StartTimer(TIMER_START_ICE_INITIALIZATION, 100*SECOND);
				m_pProcess->m_IceInitializationStatus = eIceStatusRegister;
			}
			else // Now we have new state - change from MS ICE to standard ICE
				DBGPASSERT(m_pProcess->m_IceInitializationStatus);
		}
//		if( pTempConfIpParams->GetICEType()==eIceEnvironment_Standard ) // && bIsEnableGenericICE)
//		{
//			PTRACE(eLevelInfoNormal, "CConfPartyManager::OnConfIpServiceParamsInd - Standard ice is enable in system ");
//			m_pProcess->m_IceInitializationStatus = eIceStatusON;
//		}
		if(pTempConfIpParams->GetICEType()== eIceEnvironment_None || pTempConfIpParams->GetICEType()== eIceEnvironment_WebRtc)//||pTempConfIpParams->GetICEType()==eIceEnvironment_Standard)
		{
			PTRACE(eLevelInfoNormal, "CConfPartyManager::OnConfIpServiceParamsInd - changes in ICE setting- ICE is OFF or WebRTC ICE");
			if(m_pProcess->m_IceInitializationStatus == eIceStatusON ||
               m_pProcess->m_IceInitializationStatus == eIceStatusRegister)
				m_pProcess->m_IceInitializationStatus = eIceStatusOFF;
			else
				DBGPASSERT(m_pProcess->m_IceInitializationStatus);

		}
	}

	// ===== 3. add the new MediaIpParams to process's list
	insertStatus = m_pProcess->insertIpService(pTempConfIpParams);
	if (insertStatus != STATUS_OK)
	{
		insertStatus = m_pProcess->updateIpService(pTempConfIpParams);
		PTRACE(eLevelInfoNormal, "CConfPartyManager::OnConfIpServiceParamsInd - Service already exists in list, update the service");
		if (insertStatus != STATUS_OK)
		{
			DBGPASSERT(insertStatus);
			PTRACE(eLevelError, "CConfPartyManager::OnConfIpServiceParamsInd - update the service failed");
		}
	}
	if(m_isConfIpParamsEndReceived && 0 != m_pProcess->numberOfIpServices())
	{}

	RemoveActiveAlarmByErrorCode(AA_NO_IP_SERVICE_PARAMS);

	//Update all reservations on the new prefixes
	if(needToUpdateAllRsrvPrefixes)
		::GetpMeetingRoomDB()->UpdateAllRsrvServicePrefixes();

	verifyExitFromStartUpAndPerformAfterStartUpActions();

	return insertStatus;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleUpdateRsrcPrefix(CTerminalCommand & command, std::ostream& answer)
{
    TICKS realBefore = SystemGetTickCount();
	::GetpMeetingRoomDB()->UpdateAllRsrvServicePrefixes();
    TICKS realAfter = SystemGetTickCount();
    TICKS diff = realAfter - realBefore;
    answer << "HandleUpdateRsrcPrefix - " << diff << " ticks\n";
    return STATUS_OK;

}

STATUS CConfPartyManager::HandlePrintDecoderDB(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	BYTE onlyValidEntries = FALSE;
	if(numOfParams)
	{
		const string &action = command.GetToken(eCmdParam1);
		int actionInt = atoi(action.c_str());
		if (actionInt)
			onlyValidEntries = TRUE;
	}

	::GetpDecoderResolutionTable()->DumpTable(onlyValidEntries);
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleLimitDecoderResolution(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(numOfParams == 1)
	{
		const string &action = command.GetToken(eCmdParam1);

		if (action == "on" || action == "On" || action == "ON")
		{
			answer << "Activating the feature of control the Decoder Resolution";
			::GetpDecoderResolutionTable()->SetUseDecoderTableTrueFalse(TRUE);

		}
		else if (action == "off" || action == "Off" || action == "OFF")
		{
			answer << "Disabling the feature of control the Decoder Resolution";
			::GetpDecoderResolutionTable()->SetUseDecoderTableTrueFalse(FALSE);
		}

	}
	else
	{
		answer << " Illegal num of params " << numOfParams;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// for Call Generator - Vendor detection
STATUS CConfPartyManager::HandleVendorDetectionForCG(CTerminalCommand & command, std::ostream& answer)
{
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
    {
    	PTRACE(eLevelInfoNormal,"CConfPartyManager::HandleVendorDetectionForCG - ERROR - system is not CG!!");
    	return STATUS_FAIL;
    }

    PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleVendorDetectionForCG ");
	if (isVendorDetection)
		isVendorDetection = 0;
	else
		isVendorDetection = 1;

	answer << "Vendor detection " << isVendorDetection << "\n";
	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleIntraRequestFromEP(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleIntraRequestFromEP ");
	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
			answer << "error: Conf/Party names should be specified\n";
			answer << "usage: Bin/McuCmd intra_request_from_ep ConfParty [Conf Name] [Party Name]\n";
			return STATUS_FAIL;
	}

	const string &confName =  command.GetToken(eCmdParam1);
	const string &partyName = command.GetToken(eCmdParam2);
	// Find Conf in DB
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = NULL;

	 pRequestedConf = pCommConfDB->GetCurrentConf(confName.c_str());
	if (!CPObject::IsValidPObjectPtr(pRequestedConf))
	{
		answer << "error: Conf does not exist in DB " <<  confName << " " << "IntraRequestFromEP ";
		return STATUS_FAIL;
	}

	if(pCommConfDB->SearchPartyName(confName.c_str(),partyName.c_str()) != STATUS_OK)
	{
		answer << "error: Party does not exist in DB" << " " <<  confName << " " << partyName;
		return STATUS_FAIL;
	}

	// Send Event to conf
#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	CConfApi* pConfApi = new CConfApi(pRequestedConf->GetMonitorConfId());
#else
	CConfApi* pConfApi = new CConfApi;
#endif
	pConfApi->CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));

	WORD TokenReq = 8;

	CSegment* Command = new CSegment;
	*Command << partyName;

	pConfApi->HandleTerminalEvent(TokenReq,Command);
	POBJDELETE(pConfApi);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleNQIndicationIcon(CTerminalCommand& command, std::ostream& answer)
{
	const DWORD paramsNumber = command.GetNumOfParams();

	// <ConfName> <PartyName> [<cell> <self>]

	if (paramsNumber != 2 && paramsNumber != 4)
	{
		answer << "parameters expected: <ConfName> <PartyName> [<per-cell-NQ> <layout-NQ>]\n";
		return STATUS_FAIL;
	}

	const string& confName = command.GetToken(eCmdParam1);
	const string& partyName = command.GetToken(eCmdParam2);

	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = pCommConfDB->GetCurrentConf(confName.c_str());

	if (!IsValidPObjectPtr(pRequestedConf))
	{
		answer << "@HandleNQIndicationIcon: error: Conf '" << confName << "' does not exist in DB.";
		return STATUS_FAIL;
	}

	CConfParty* pParty = pRequestedConf->GetCurrentParty(partyName.c_str());

	if (!pParty)
	{
		answer << "@HandleNQIndicationIcon: error: Party '" << partyName << "' does not exist in DB for Conf '" << confName << "'.";
		return STATUS_FAIL;
	}

	CConfApi confApi;
	confApi.CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));

	CSegment params;

	bool activate = (paramsNumber > 2);
	WORD cellNQ = activate ? atoi(command.GetToken(eCmdParam3).c_str()) : 0;
	WORD selfNQ = activate ? atoi(command.GetToken(eCmdParam4).c_str()) : 0;

	const CTaskApp* pPartyTask = pParty->GetTask();

	TRACESTR(eLevelInfoHigh) << "CConfPartyManager::HandleNQIndicationIcon: Conf: " << confName << ", Party: "<< partyName;

	confApi.NotifyVbOnNetworkQualityChange(pPartyTask, static_cast<eRtcpPacketLossStatus>(cellNQ), static_cast<eRtcpPacketLossStatus>(selfNQ));
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleIvrFilesCache(CTerminalCommand& command, std::ostream& answer)
{
	const DWORD paramsNumber = command.GetNumOfParams();

	if (paramsNumber != 1)
	{
		answer << "Commands supported: clean, dump";
		return STATUS_FAIL;
	}

	const string& cmd = command.GetToken(eCmdParam1);

	if ("clean" == cmd)
		CFilesCache::instance().Clean();

	else if ("dump" == cmd)
		answer << CFilesCache::const_instance();

	else
	{
		answer << "Command '" << cmd << "' is not (yet) implemented.";
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleGetConnectedPartiesNumber(CTerminalCommand& command, std::ostream& answer)
{
	bool bRet = FALSE;
	CCommConfDB* pConfDB = ::GetpConfDB();
	CCommConf*   pCommConf = pConfDB->GetFirstCommConf();
	// Search on going conference database
	while (IsValidPObjectPtr(pCommConf))
	{
		if (0 < pCommConf->GetConnectedPartiesNumber())
		{
			answer << "PARTIESCONNECTED\n";
			return STATUS_OK;
		}
		pCommConf = pConfDB->GetNextCommConf();
	}
	answer << "NOCONNECTEDPARTIES\n";
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

STATUS CConfPartyManager::HandleRecurrentIntraRequestFromEP(CTerminalCommand& command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleIntraRequestFromEP ");
	DWORD numOfParams = command.GetNumOfParams();
	if(4 > numOfParams || 5 < numOfParams)
	{
			answer << "error: Conf/Party names should be specified, the time interval and the number of recurrence\n";
			answer << "usage: Bin/McuCmd intra_request_from_ep ConfParty [Conf Name] [Party Name] [TimeInterval] [NumIntraReq][ChannelType(optional)]\n";
			return STATUS_FAIL;
	}
	const string &confName =  command.GetToken(eCmdParam1);
	const string &partyName = command.GetToken(eCmdParam2);

	const string &timeIntervalS = command.GetToken(eCmdParam3);
	const string &numRequestsS = 	command.GetToken(eCmdParam4);

	DWORD channelType = 1; //video
	if (numOfParams == 5)
	{
		const string &channelTypeS = command.GetToken(eCmdParam5);
		if (channelTypeS == "2" || channelTypeS == "content")
			channelType = 2;
	}

	DWORD timeInterval = atoi(timeIntervalS.c_str());
	DWORD numRequests = atoi(numRequestsS.c_str());

	// Find Conf in DB
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = NULL;
	pRequestedConf = pCommConfDB->GetCurrentConf(confName.c_str());
	if (!CPObject::IsValidPObjectPtr(pRequestedConf))
	{
		answer << "error: Conf does not exist in DB " <<  confName << " " << "IntraRequestFromEP ";
		return STATUS_FAIL;
	}
	if(pCommConfDB->SearchPartyName(confName.c_str(),partyName.c_str()) != STATUS_OK)
	{
		answer << "error: Party does not exist in DB" << " " <<  confName << " " << partyName;
		return STATUS_FAIL;
	}
	if(numRequests>100)
		numRequests=100;
	if(numRequests==0)
	{
		PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleIntraRequestFromEP NUM IRTRA = 0");
		return STATUS_OK;
	}
	// Send Event to conf
	CConfApi* pConfApi = new CConfApi;
	pConfApi->CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));
	WORD TokenReq = 9;
	CSegment* Command = new CSegment;
	*Command << partyName << (DWORD)timeInterval<< (DWORD)numRequests << (DWORD)channelType;

	pConfApi->HandleTerminalEvent(TokenReq,Command);

	return STATUS_OK;
}

STATUS CConfPartyManager::HandleSet1080p60FR(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoHigh, "CConfPartyManager::HandleSet1080p60FR ");
	DWORD numOfParams = command.GetNumOfParams();
	if(numOfParams > 1)
	{
			answer << "usage: Bin/McuCmd Set1080p60FR ConfParty [30/60]\n";
			return STATUS_FAIL;
	}
	const string &fr =  command.GetToken(eCmdParam1);

	DWORD frameRate = atoi(fr.c_str());

	if(frameRate == 30){
	  Set1080p60mbps(244800);
	}else{
	  Set1080p60mbps(489600);
	}

	return STATUS_OK;
}

STATUS CConfPartyManager::HandleSetDebugVal(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
			answer << "usage: Bin/McuCmd setdebugval ConfParty [Feature-Name] [debug-val]\n";
			answer << " Features-List: \nTRANSLATOR";
			return STATUS_FAIL;
	}

	const string &featureNameStr = command.GetToken(eCmdParam1);
	const string &debugValStr	 = command.GetToken(eCmdParam2);
	DWORD dwDebugValue 			 = atoi(debugValStr.c_str());
	TRACEINTO << " Set Debug Params, feature: " << featureNameStr << " , DebugValue: " << dwDebugValue;

	SetDebugValue( featureNameStr, dwDebugValue);

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleSpeakerInd(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoHigh, "CConfPartyManager::HandleSpeakerInd ");

	DWORD  partyAudioMSI = DUMMY_DOMINANT_SPEAKER_MSI;

	DWORD numOfParams = command.GetNumOfParams();
	if (2 != numOfParams)
	{
			answer << "usage: Bin/McuCmd speaker_ind ConfParty [conference monitor ID] [party monitor ID]\n";
			return STATUS_FAIL;
	}

	const string &sMonitorConfId = command.GetToken(eCmdParam1);
	const string &sMonitorPartyId = command.GetToken(eCmdParam2);
	DWORD monitorConfId = atoi(sMonitorConfId.c_str());
	DWORD monitorPartyId = atoi(sMonitorPartyId.c_str());


	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = pCommConfDB->GetCurrentConf(monitorConfId);

	if (!IsValidPObjectPtr(pRequestedConf))
	{
		answer << "@HandleSpeakerInd: error: Conf id " << monitorConfId << " does not exist in DB.";
		return STATUS_FAIL;
	}

	CConfParty* pParty = pRequestedConf->GetCurrentParty(monitorPartyId);

	if (!pParty)
	{
		answer << "@HandleSpeakerInd: error: Party id " << monitorPartyId << " does not exist in DB for Conf id " << monitorConfId << ".";
		return STATUS_FAIL;
	}

	CConfApi confApi;
	confApi.CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));

	CSegment params;

	CTaskApp* pPartyTask = pParty->GetTask();

	TRACESTR(eLevelInfoHigh) << "CConfPartyManager::HandleSpeakerInd: Conf id: " << monitorConfId << ", Party id: "<< monitorPartyId;

	confApi.SpeakersChanged(pPartyTask, pPartyTask,partyAudioMSI);

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleSendResourcesMap(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleSendResourcesMapCTerminalCommand ");
	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
			answer << "error: not enough params\n";
			answer << "usage: Bin/McuCmd send_rsrc_map [board id] [unit id]\n";
			return STATUS_FAIL;
	}
	const string &boardIdStr =  command.GetToken(eCmdParam1);
	const string &unitIdStr = command.GetToken(eCmdParam2);

	DWORD boardId = atoi(boardIdStr.c_str());
	DWORD unitId = atoi(unitIdStr.c_str());

	CSegment* pSeg = new CSegment();
	*pSeg << boardId << unitId;

	OnKillPortAck(pSeg);

	POBJDELETE(pSeg);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
//STATUS CConfPartyManager::HandleOpenContentDecoder(CTerminalCommand & command, std::ostream& answer)
//{
//	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleOpenContentDecoder ");
//	DWORD numOfParams = command.GetNumOfParams();
//	if(3 != numOfParams)
//	{
//			answer << "error: Conf name should be specified\n";
//			answer << "usage: Bin/McuCmd OpenContentDecoder ConfParty [Conf Name] [Party Name] [H263/H264]\n";
//			return STATUS_FAIL;
//	}
//	const string &confName =  command.GetToken(eCmdParam1);
//	const string &partyName = command.GetToken(eCmdParam2);
//	const string &protocol = command.GetToken(eCmdParam3);
//
//	// Find Conf in DB
//	CCommConfDB* pCommConfDB = ::GetpConfDB();
//	CCommConf* pRequestedConf = NULL;
//
//	 pRequestedConf = pCommConfDB->GetCurrentConf(confName.c_str());
//	if (!CPObject::IsValidPObjectPtr(pRequestedConf))
//	{
//		answer << "error: Conf does not exist in DB " <<  confName << " " << "OpenContentDecoder ";
//		return STATUS_FAIL;
//	}
//
//	if(pCommConfDB->SearchPartyName(confName.c_str(),partyName.c_str()) != STATUS_OK)
//	{
//		answer << "error: Party does not exist in DB" << " " <<  confName << " " << partyName;
//		return STATUS_FAIL;
//	}
//
//	// Send Event to conf
//	CConfApi* pConfApi = new CConfApi;
//	pConfApi->CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));
//
//	WORD TokenReq = 9;   //also 10
//
//	if ((protocol != "H264") && (protocol != "H263"))
//	{
//		answer << "error: Activating the featur with Protocol H263 or H264" << " " <<  confName << " " << partyName;
//		return STATUS_FAIL;
//	}
//
//
//	if (protocol == "H263")
//	{
//		TokenReq = 10;   //also 9
//	}
//
//	CSegment* Command = new CSegment;
//	*Command << partyName;
//
//	pConfApi->HandleTerminalEvent(TokenReq,Command);
//	return STATUS_OK;
//}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnConfIpServiceParamsEndInd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnConfIpServiceParamsEndInd ");
	m_isConfIpParamsEndReceived = YES;

	if(0 == m_pProcess->numberOfIpServices())
	{
		UpdateStartupConditionByErrorCode(AA_NO_IP_SERVICE_PARAMS, eStartupConditionFail);
	}

	GetpProfilesDB()->DeleteInexistentIPServiceFromServiceList();
	GetpMeetingRoomDB()->DeleteInexistentIPServiceFromServiceList();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnConfDeleteIpeServiceParamsInd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnConfDeleteIpeServiceParamsInd ");

	Del_Ip_Service_S *param = (Del_Ip_Service_S*)pSeg->GetPtr();
	DWORD serviceID = param->service_id;

	/* VNGR-20807: remove this service from all profiles */
	GetpProfilesDB()->DeleteIPServiceFromProfileServiceList(param->service_name);

	STATUS lRemoveStatus = m_pProcess->removeIpService(serviceID);
	if(lRemoveStatus != STATUS_OK)
      PTRACE(eLevelError, "CConfPartyManager::OnConfDeleteIpeServiceParamsInd - Deleted IP service was not found in ConfParty List ");
    if(0 == m_pProcess->numberOfIpServices())
    	AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
						AA_NO_IP_SERVICE_PARAMS,
						MAJOR_ERROR_LEVEL,
						"IP Network Service parameters missing",
						true,
						false
					);

	return lRemoveStatus;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnTimerIPServiceFromCSMngrTout(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnTimerIPServiceFromCSMngrTout ");
	if(!m_isConfIpParamsEndReceived)
	{
		//DBGPASSERT(1);
        PTRACE(eLevelInfoNormal, "CConfPartyManager::OnTimerIPServiceFromCSMngrTout - Failure of obtaining IP services from CS Mngr");
	}
	return STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
void  CConfPartyManager::OnCSMngrServiceCfgUpdate(CSegment* pSeg)
{

	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnCSMngrServiceCfgUpdate");
	CServiceConfigList *pServiceConfigList = new CServiceConfigList();
	pServiceConfigList->DeSerialize(pSeg);
	CProcessBase::GetProcess()->SetServiceConfigList(pServiceConfigList);


	/////////////////test/////////////////////////////////////////
	CServiceConfigList *testservicecfg=CProcessBase::GetProcess()->GetServiceConfigList();
	DWORD tt=0;
	testservicecfg->GetDWORDDataByKey(1,"SIP_REGISTER_DELAY_MILLI_SEC",tt);

	FTRACESTR(eLevelInfoNormal) << "CConfPartyManager::OnCSMngrServiceCfgUpdate = " << tt;
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnCSMngrServiceDefaultUpdate(CSegment* pSeg)
{
	DEFAULT_IP_SERVICE_S* pDefaultService = new DEFAULT_IP_SERVICE_S;
	WORD structLen = sizeof(DEFAULT_IP_SERVICE_S);
	memcpy(pDefaultService, pSeg->GetPtr(), structLen);
	TRACEINTO << " CConfPartyManager::OnCSMngrServiceDefaultUpdate :"
			<< "  defaultH323ServiceName = "
			<< pDefaultService->defaultH323ServiceName
			<< ", defaultSIPServiceName = "
			<< pDefaultService->defaultSIPServiceName;

	GetIpServiceListMngr()->SetDefaultIpServiceType(
			pDefaultService->defaultH323ServiceName,
			pDefaultService->defaultSIPServiceName);
	POBJDELETE(pDefaultService);
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::SendConfIPParamsReqToCS()
{
	CSegment*  pRetParam = new CSegment;

	//const COsQueue* pCsMbx =
	//	CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);
	//if(!pCsMbx->IsValid())
	//{
	//	return;
	//}

    CManagerApi api(eProcessCSMngr);

	STATUS res = api.SendMsg(pRetParam, CS_CONF_IP_SERVICE_PARAM_REQ);

	if(STATUS_OK != res)
	{
		PASSERT(1);
	}
	else
	{
		StartTimer(IPSERVICEFROMCSMNGRTOUT,30*SECOND);
		PTRACE(eLevelInfoNormal, "I am the Conference Party Manager - my goal is to get IP services from CSMngr - strat IPSERVICEFROMCSMNGRTOUT timer");
	}
}
/////////////////////////////////////////////////////////////////////////////
//  Isdn Sevices
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnRtmIsdnParamsInd(CSegment* pSeg)
{
	STATUS retStat = STATUS_OK;

	// ===== 1. extract data
	RTM_ISDN_PARAMS_MCMS_S* pRtmIsdnParams = new RTM_ISDN_PARAMS_MCMS_S;
	pSeg->Get( (BYTE*)pRtmIsdnParams, sizeof(RTM_ISDN_PARAMS_MCMS_S) );

	// ===== 2. print
	PTRACE(eLevelInfoNormal, "\nCConfPartyManager::OnRtmIsdnParamsInd");
	CRtmIsdnMngrCommonMethods rtmIsdnCommonMethods;
	rtmIsdnCommonMethods.PrintRtmIsdnParamsMcmsStruct(*pRtmIsdnParams, "CConfPartyManager::OnRtmIsdnParamsInd");

	m_pProcess->AddIsdnService(pRtmIsdnParams);
	RemoveActiveAlarmByErrorCode(AA_NO_ISDN_SERVICE_PARAMS);
	if (m_pProcess->numberOfIsdnServices() == 1)	// "default" is defined as the first one in the list
		RemoveActiveAlarmByErrorCode(AA_NO_DEFAULT_ISDN_SERVICE);

	POBJDELETE(pRtmIsdnParams);

	return retStat;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnRtmIsdnParamsEndInd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnRtmIsdnParamsEndInd ");
	m_isConfIsdnParamsEndReceived = YES;

	if(0 == m_pProcess->numberOfIsdnServices())
	{
		PTRACE(eLevelError, "\nCConfPartyManager::OnRtmIsdnParamsEndInd - No Isdn Services");
		RemoveActiveAlarmByErrorCode(AA_NO_ISDN_SERVICE_PARAMS);
		RemoveActiveAlarmByErrorCode(AA_NO_DEFAULT_ISDN_SERVICE);
		//RemoveActiveAlarmByErrorCode(AA_NO_ISDN_SERVICE_PARAMS);

		//BRIDGE-6283
		/*bool isExist = IsActiveAlarmExistByErrorCode(AA_NO_ISDN_SERVICE_PARAMS);
		if(!isExist)
			AddActiveAlarm(FAULT_GENERAL_SUBJECT, AA_NO_ISDN_SERVICE_PARAMS, MAJOR_ERROR_LEVEL, "No ISDN/PSTN Network Services defined", true, true);
*/
		//UpdateStartupConditionByErrorCode(AA_NO_ISDN_SERVICE_PARAMS, eStartupConditionFail);
	}


	eTaskState taskState = GetTaskState();
    const char *taskStateName = GetTaskStateName(taskState);
    TRACEINTO << "\nCConfPartyManager::OnRtmIsdnParamsEndInd : " << taskStateName;

    verifyExitFromStartUpAndPerformAfterStartUpActions();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnRtmIsdnDefaultServiceNameInd(CSegment* pSeg)
{
  RTM_ISDN_SERVICE_NAME_S theStruct;
  memset(&theStruct,0,sizeof(RTM_ISDN_SERVICE_NAME_S));
  pSeg->Get( (BYTE*)&theStruct, sizeof(RTM_ISDN_SERVICE_NAME_S) );

  TRACESTR (eLevelInfoNormal) <<  "CConfPartyManager::OnRtmIsdnDefaultServiceNameInd service name=" << (char *) (theStruct.serviceName);

  eTaskState taskState = GetTaskState();
  const char *taskStateName = GetTaskStateName(taskState);
  TRACEINTO << "\nCConfPartyManager::OnRtmIsdnDefaultServiceNameInd : " << taskStateName;

  //Set the Default service Name
    m_pProcess->SetIsdnServiceAsDefault((char *) (theStruct.serviceName));

    verifyExitFromStartUpAndPerformAfterStartUpActions();

  return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnRtmDeleteIsdnServiceParamsInd(CSegment* pSeg)
{
	RTM_ISDN_SERVICE_NAME_S theStruct;
    memset(&theStruct,0,sizeof(RTM_ISDN_SERVICE_NAME_S));
    pSeg->Get( (BYTE*)&theStruct, sizeof(RTM_ISDN_SERVICE_NAME_S) );
    m_pProcess->DeleteIsdnService((char *)(theStruct.serviceName));
   if (m_pProcess->numberOfIsdnServices() == 0)
   {
    	TRACEINTO <<  "ISDN service deleted. No ISDN services defined";
    	//BRIDGE-6283
    	//bool isExist = IsActiveAlarmExistByErrorCode(AA_NO_ISDN_SERVICE_PARAMS);
    	//if(!isExist)
    		//AddActiveAlarm(FAULT_GENERAL_SUBJECT, AA_NO_ISDN_SERVICE_PARAMS, MAJOR_ERROR_LEVEL, "No ISDN/PSTN Network Services defined", true, true);
   }
	return STATUS_OK;
}
///////////////////////////////////////////////////////////////////////////////
//void CConfPartyManager::SetRsrvQueueRecivedFromRsrcProcess()
//{
//	CSegment*  pRetParam = new CSegment;
//
//	const COsQueue* pRsrvManagerQ;
//
//	const COsQueue* pRsrcMbx =
//		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessResourceMngr, eManager);
//	if(!pRsrcMbx->IsValid())
//	{
//		return;
//	}
//
//	STATUS res = pRsrcMbx->Send(pRetParam,GET_SINGEL_TONE_TASK_QUEUE);
//
//	if(STATUS_OK != res)
//	{
//		PASSERT(1);
//	}
//	else
//	{
//		CProcessBase::GetProcess()->SetpRsrvManagerTask(Seg);
//		PTRACE(eLevelInfoNormal, "I  got the RsrvManager Queue");
//	}
//
//
//		rc = m_pConfApi->AddInH323Party(m_pH323NetSetup,this, *m_pRcvMbx, m_name,
//										GET_SINGLE_TONE_QUEUE_TOUT, &pRspMsg);
//		if (rc == 0)
//		{
//			if(pRspMsg)
//			{
//				WORD is_pRspMsg_valid = CPObject::IsValidPObjectPtr(pRspMsg);
//				if(is_pRspMsg_valid)
//					*pRspMsg >> COsQueue;
//			}
//		}
//
//		if (rc)
//		{
//			PTRACE(eLevelError,"CConfPartyManager::SetRsrvQueueRecivedFromRsrcProcess \' FAILED (TimeOut) !!!\'");
//		}
//
//		else if (!COsQueue)
//		{
//			PTRACE(eLevelError,"CH323PartyIn::OnLobbyTransferSetup : \' (Pointer to Queue is NOT valid) !!!\'");
//		}
//
//
//			/////////////////////////////////////////////////////////////////////////////
//WORD  CConfApi::AddInH323Party(CH323NetSetup* pH323NetSetup, CTaskApp* pParty,
//					 COsQueue& partyRcvMbx,char* name,DWORD tout, CSegment** pRspMsg)
//{
//  CSegment*  seg = new CSegment;
//
//	*seg << (DWORD)pParty;
//	*seg << name;
//
//
//	pH323NetSetup->Serialize(NATIVE,*seg);
//
//	partyRcvMbx.Serialize(*seg);
//
//	return(SendMessageSync(seg, H323ADDINPARTY, tout, pRspMsg));
//}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::CheckVideoLayout(CVideoLayout* pVideoLayoutOper, CVideoLayout* pVideoLayoutDB,BYTE isHDVSW,const WORD isPrivate)
{
  CVideoCellLayout*  pVideoCellLayoutOper = NULL;
  CVideoCellLayout*  pVideoCellLayoutDB = NULL;
  DWORD forced_partyId;
  WORD  j = 0;
  BYTE cellStatus;

  if (!pVideoLayoutOper /*|| !pVideoLayoutDB*/)
    return STATUS_ILLEGAL;

  for (WORD i=0; i<pVideoLayoutOper->m_numb_of_cell; i++)
    {
      // Check the structure received from OperatorWS

      pVideoCellLayoutOper = pVideoLayoutOper->m_pCellLayout[i];

      if (pVideoCellLayoutOper){
	forced_partyId = pVideoCellLayoutOper->GetForcedPartyId();

	if (forced_partyId!=0xFFFFFFFF)
	  {
	    cellStatus = pVideoCellLayoutOper->GetCellStatus();

	    if (cellStatus!=EMPTY_BY_OPERATOR_THIS_PARTY && cellStatus!=EMPTY_BY_OPERATOR_ALL_CONF &&
		cellStatus!=AUTO && cellStatus!=AUDIO_ACTIVATED)
	      {

		if ((pVideoLayoutOper->GetNumCells(forced_partyId))>1)
		{
		  PTRACE2INT(eLevelError,"STATUS CConfPartyManager::CheckVideoLayout forced_partyId:",forced_partyId);
		  PASSERT(1);
		  return  STATUS_PARTY_IS_PRESENT_IN_SEVERAL_CELLS;
		}
		// Check database current layout and received info consistency

					if (pVideoLayoutDB && !isPrivate)
		  for (j=0; j<pVideoLayoutDB->m_numb_of_cell; j++){
		    pVideoCellLayoutDB = pVideoLayoutDB->m_pCellLayout[j];

		    if (pVideoCellLayoutDB)
		      if (pVideoLayoutOper->FindCell(pVideoCellLayoutDB->GetCellId()) == NOT_FIND)
			{
									if (pVideoCellLayoutDB->GetCurrentPartyId() == forced_partyId)
			    {
			      cellStatus = pVideoCellLayoutDB->GetCellStatus();

			      if (cellStatus!=EMPTY_BY_OPERATOR_THIS_PARTY && cellStatus!=EMPTY_BY_OPERATOR_ALL_CONF &&
				  cellStatus!=AUTO && cellStatus!=AUDIO_ACTIVATED)
			      	{
			      	PTRACE2INT(eLevelError,"STATUS CConfPartyManager::CheckVideoLayout forced_partyId",forced_partyId);
				PTRACE2INT(eLevelError,"STATUS CConfPartyManager::CheckVideoLayout cellStatus",cellStatus);
			      	PASSERT(2);
				return STATUS_PARTY_IS_PRESENT_IN_SEVERAL_CELLS;
			      	}
			    }
			}
		  }
	      }
	  }
      }
    }
  return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
////////   Events from Lobby     ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////servicepro
void CConfPartyManager::AddUnreservedParty(CSegment* pParam)  // shiraITP - 44
{
	int status = STATUS_OK;

	mcTransportAddress destIP;
	ConfMonitorID ConfId;
	DWORD undefId;
	WORD bIsVoiceCall;
	WORD bIsSipCall;
	WORD bIsMrcHeader;
	WORD bIsWebRtcCall;
	WORD srsSessionType = 0;
	BYTE temp;
	BYTE encryptionType;
	//BYTE remoteCascadeMode;	//--- patch for ignoring Encryption in Cascade
	BYTE bIsMsConfInvite = 0;
	BYTE bIsNonCCCPAvMcu = 0;

	ALLOCBUFFER(pSipAddress, IP_STRING_LEN);
	ALLOCBUFFER(pH323useruserField, MaxUserUserSize);

	pParam->Get((BYTE*)&destIP, sizeof(mcTransportAddress));
	*pParam >> ConfId;
	*pParam >> undefId;
	*pParam >> bIsVoiceCall;
	*pParam >> bIsSipCall;
	*pParam >> bIsMsConfInvite;
	*pParam >> temp;
	*pParam >> bIsNonCCCPAvMcu;
	*pParam >> pSipAddress;
	*pParam >> encryptionType;
	*pParam >> pH323useruserField;
	*pParam >> bIsMrcHeader;
	*pParam >> bIsWebRtcCall;
	*pParam >> srsSessionType;
	//*pParam >> remoteCascadeMode;	//--- patch for ignoring Encryption in Cascade

	eSipFactoryType factoryType = (eSipFactoryType)temp;
	if (eNotSipFactory == factoryType)
		PASSERT(undefId == 0);

	TRACEINTO << "ConfId:" << ConfId << ", UndefId:" << undefId << ", IsVoiceCall:" << bIsVoiceCall << ", IsSipCall:" << bIsSipCall << ", SipAddress:" << pSipAddress << ", bIsMsConfInvite:" << bIsMsConfInvite;

	if (m_pProcess->m_IceInitializationStatus == eIceStatusRegister && bIsSipCall)
	{
		TRACEINTO << "Failed, ICE initialization is not finished yet, reject SIP party";
		status = STATUS_ILLEGAL;
	}
	if(bIsWebRtcCall)
	{
		if (!m_isWebRtcGWStarted || m_isWebRtcIceServerFailure )
		{
			TRACEINTO << "Failed, WebRtc ICE initialization failed or keep alive failure, reject SIP party";
			status = STATUS_ILLEGAL;
		}
	}
	/*** VALIDITY of conferenceId ***/
	CCommConf* pCommConf = NULL;
	if (status == STATUS_OK)
	{
		pCommConf = ::GetpConfDB()->GetCurrentConf(ConfId);
		if (!pCommConf)
		{
			TRACEINTO << "ConfId:" << ConfId << " - Failed to find conference in ConfDB";
			status = STATUS_CONF_NOT_EXISTS;
		}
	}

	if (status == STATUS_OK)
	{
		if (eNotSipFactory == factoryType)
		{
			int isIpNonValid = ::isIpTaNonValid(&destIP);
			int isIpZero     = ::isApiTaNull(&destIP);

			if (!bIsSipCall && (isIpNonValid || isIpZero) && !pCommConf->IsEnableIsdnPstnAccess() && !pCommConf->GetIsGateway())
			{
				TRACEINTO
					<< "IsSipCall:" << bIsSipCall
					<< ", IsIpNonValid:" << isIpNonValid
					<< ", IsIpZero:" << isIpZero
					<< ", IsEnableIsdnPstnAccess:" << (WORD)pCommConf->IsEnableIsdnPstnAccess()
					<< ", IsGateway:" << (WORD)pCommConf->GetIsGateway();
				status = STATUS_ILLEGAL;
			}
		}

		//In case AVC EP (HDX) connects as a SIP to SVC only this if will prevent it from penetrating in.
		if (status == STATUS_OK)
		{
			if ((pCommConf->GetConfMediaType() == eSvcOnly) && !bIsMrcHeader)
			{
				TRACEINTO << "ConfId:" << ConfId << " - Failed, AVC call in SVC conference";
				status = STATUS_ILLEGAL;  // interfering with VSW feature
			}
		}

		if (status == STATUS_OK)
		{
			// Find net interface type and Service name
			BYTE interfaceType = 0;
			WORD line = 0;
			std::string serviceNameStr;

			if (isApiTaNull(&destIP) && !bIsSipCall) // IsPSTNCall
				interfaceType = ISDN_INTERFACE_TYPE;
			else
				interfaceType = (bIsSipCall) ? SIP_INTERFACE_TYPE : H323_INTERFACE_TYPE;

			if (ISDN_INTERFACE_TYPE == interfaceType)
			{
				// for dial in call take the default service
				const RTM_ISDN_PARAMS_MCMS_S* pIsdnServiceStruct = m_pProcess->GetIsdnService("");
				CServicePhoneStr* pServicePhoneStr = pCommConf->GetFirstServicePhone();
				if (pServicePhoneStr)
				{
					serviceNameStr = (char*)pServicePhoneStr->GetNetServiceName();
					TRACEINTO << "ServiceName:" << serviceNameStr << " - Using conference ISDN service";
				}
				else if (pIsdnServiceStruct)
				{
					serviceNameStr = ((char*)pIsdnServiceStruct->serviceName);
					TRACEINTO << "ServiceName:" << serviceNameStr << " - Using default ISDN service";
				}
				else
				{
					TRACESTRFUNC(eLevelError) << "Failed, Can not find ISDN service";
					PASSERT(1);
				}
			}
			else
			{
				CConfIpParameters* pIpParams = ::GetIpServiceListMngr()->FindServiceByIPAddress(destIP);
				if (pIpParams)
					serviceNameStr = ((char*)pIpParams->GetServiceName());
			}

			// Build reservation for the party
			CRsrvPartyAdd* undefPartyAdd = new CRsrvPartyAdd;
			CRsrvParty* undefParty = undefPartyAdd->GetRsrvParty();

			TRACEINTO << "CascadeMode:" << CascadeModeToString(pCommConf->GetCascadeMode()) << ", IceStatus:" << m_pProcess->m_IceInitializationStatus <<" bIsNonCCCPAvMcu "<<bIsNonCCCPAvMcu;

			undefParty->SetConnectionType(DIAL_IN);
			undefParty->SetNetInterfaceType(interfaceType);
			undefParty->SetServiceProviderName(serviceNameStr.c_str());
			undefParty->SetIdentificationMethod(CALLING_PHONE_NUMBER_IDENTIFICATION_METHOD);
			undefParty->SetMeet_me_method(MEET_ME_PER_MCU);
			undefParty->SetNetChannelNumber(AUTO);
			undefParty->SetCascadeMode(pCommConf->GetCascadeMode());
			undefParty->SetIsEncrypted(encryptionType);
			undefParty->SetEnableICE(m_pProcess->m_IceInitializationStatus == eIceStatusON);
			undefParty->SetUndefinedType(UNRESERVED_PARTY);
			undefParty->SetIsAvMcuNonCCCPLink(bIsNonCCCPAvMcu);


			//eFeatureRssDialin
			enSrsSessionType srsSesType = static_cast<enSrsSessionType>(srsSessionType);
			undefParty->SetRecordingLinkParty(NO);
			undefParty->SetPlaybackLinkParty(NO);

			if(eSrsSessionTypeRecording == srsSesType)
			{
				undefParty->SetRecordingLinkParty(YES);
				//mute video for recording link
				undefParty->SetVideoMute(YES);
			}
			else if(eSrsSessionTypePlayback== srsSesType)
			{
				undefParty->SetPlaybackLinkParty(YES);
			}

			// ??? to remove? ???
			//BYTE tmpCascadeMode = CASCADE_MODE_NONE;
			//if (CASCADE_MODE_SLAVE == remoteCascadeMode)
			//	tmpCascadeMode = CASCADE_MODE_MASTER;
			//else if(CASCADE_MODE_MASTER == remoteCascadeMode)
			//	tmpCascadeMode = CASCADE_MODE_SLAVE;

			//undefParty->SetCascadeMode(tmpCascadeMode);	//--- patch for ignoring Encryption in Cascade

			undefParty->SetIsWebRtcCall(bIsWebRtcCall);

			if (bIsSipCall)
			{
				TRACEINTO << "Add MSconfInvite";
				undefParty->SetSipPartyAddress(pSipAddress);
				undefParty->SetSipPartyAddressType(PARTY_SIP_SIPURI_ID_TYPE);
				enMsftAvmcuState avmcuState = bIsMsConfInvite ? eMsftAvmcuUnkown : eMsftAvmcuNone;
				undefParty->SetMsftAvmcuState(avmcuState);
			}

			if (bIsVoiceCall)   // 2.05.00
				undefParty->SetVoice(YES);

			char h243name[H243_NAME_LEN];
			char confName[H243_NAME_LEN];
			memset(h243name, '\0', H243_NAME_LEN);
			memset(confName, '\0', H243_NAME_LEN);

			// unicode change - we copy conf display name (utf-8) to undefined party name
			// instead of conf (ascii) name
			strncpy(confName, pCommConf->GetDisplayName(), H243_NAME_LEN - 14);
			confName[H243_NAME_LEN-14] = '\0';

			// fix VNGFE-2711
			for (int i = 0; i < H243_NAME_LEN-13; i++) // fix invalid characters in name
			{
				if ((confName[i] == ',') || (confName[i] == ';'))
					confName[i] = '_';
			}

			// Multiple links for ITP in cascaded conference feature: CConfPartyManager::AddUnreservedParty - rename according to GetITPparams
			eTypeOfLinkParty linkType = eRegularParty;
			BYTE cascadedLinksNumber= 0;
			BYTE index = 0;
			DWORD mainLinkDialInNumber = 0;

			BOOL checkUserUserFieldValidity = ::GetITPparams(pH323useruserField, cascadedLinksNumber, index, linkType, mainLinkDialInNumber, interfaceType);

			TRACEINTO << "UserFieldValidity:" << (int)checkUserUserFieldValidity << ", LinkType:" << linkType << ", MainLinkDialInNumber:" << mainLinkDialInNumber;

			if (checkUserUserFieldValidity == TRUE) // rename:
			{
				undefParty->SetCascadedLinksNumber(cascadedLinksNumber);
				undefParty->SetPartyType(linkType);

				if (pCommConf && linkType == eMainLinkParty)
				{
					char mainName[H243_NAME_LEN];
					memset(mainName, '\0', H243_NAME_LEN);

					DWORD unrsrvPartiesCounter = pCommConf->NextUnrsrvPartiesCounter();
					snprintf(mainName, sizeof(mainName)-1, "%s_(%03d)", confName, unrsrvPartiesCounter);

					::CreateMainLinkName(mainName, h243name);
				}
				else
				{
					if (pCommConf && linkType == eSubLinkParty)
					{
						undefParty->SetMainPartyNumber(mainLinkDialInNumber);

						char findMainPartyNameOfThisSubLink[H243_NAME_LEN];
						BOOL isMainLinkDefined = pCommConf->GetMainLinkNameAccordingToMainPartiesCounterAndReturnIsMainLinkDefined(mainLinkDialInNumber, findMainPartyNameOfThisSubLink);

						::GetSubLinkName(findMainPartyNameOfThisSubLink, index, h243name);
						TRACEINTO << "MainPartyNameOfThisSubLink:" << findMainPartyNameOfThisSubLink << ", SubName:" << h243name;
					}
				}
			}
			else
				snprintf(h243name, sizeof( h243name ) -1, "%s_(%03d)", confName, pCommConf->NextUnrsrvPartiesCounter());

			undefParty->SetName(h243name);

			// Add party to the conference
			CRequest* pRequest = new CRequest;

			undefPartyAdd->SetConfID(ConfId);
			pRequest->SetConnectId(0);
			pRequest->SetRequestObject(undefPartyAdd);

			// POBJDELETE(undefParty)//Request Delete

			TRACEINTO << "bIsMsConfInvite2: MsftAvmcuState="
					"" << enMsftAvmcuStateNames[(undefPartyAdd->GetRsrvParty())->GetMsftAvmcuState()];

			OnServerAddPartyToConfByType(pRequest, undefId, factoryType, linkType);    // shiraITP - 45
		}
	}

	if (status != STATUS_OK) // Send reject to Lobby task
	{
		TRACESTRFUNC(eLevelError) << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
		CLobbyApi* pLobbyApi = (CLobbyApi*)::GetpLobbyApi();
		pLobbyApi->RejectUnreservedParty(ConfId, undefId, status);
	}

	DEALLOCBUFFER(pSipAddress);
	DEALLOCBUFFER(pH323useruserField);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnDisconnectRecordingLinkParty(CSegment* pSeg)
{
	DWORD conferId  = 0;
	DWORD partyID = 0;
	*pSeg >> conferId ;
	partyID = ::GetpConfDB()->GetRecordingLinkPartyId(conferId);
	DisconnectParty(conferId, partyID, m_operName);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnAddRecordingLinkParty(CSegment* pSeg)
{
  STATUS status = STATUS_OK;
  BYTE isMuteVideo = NO;
  DWORD confId = 0;
  CRsrvParty* pRecLinkParty = NULL; // from the recording link DB

  *pSeg >> isMuteVideo >> confId;

  TRACEINTO << "CConfPartyManager::OnAddRecordingLinkParty, ConfId:" << confId << ", IsMuteVideo:" << (WORD)isMuteVideo;

  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
  if (!pCommConf)
  {
    TRACEINTO << "CConfPartyManager::OnAddRecordingLinkParty - Failed, the conference is not exist, ConfId:" << confId;
    status = STATUS_CONF_NOT_EXISTS;
  }

  //validate that the conference is enabled recording
  if (status == STATUS_OK && !pCommConf->GetEnableRecording())
  {
    TRACEINTO << "CConfPartyManager::OnAddRecordingLinkParty - Failed, the conference is not enabled for recording, ConfId:" << confId;
    status = STATUS_ILLEGAL;
  }

  //validate that it's not an EQ conference
  if (status == STATUS_OK && pCommConf->GetEntryQ())
  {
    TRACEINTO << "CConfPartyManager::OnAddRecordingLinkParty - Failed, the conference is EQ, so can't create a recording link party, ConfId:" << confId;
    status = STATUS_ILLEGAL;
  }

  //validate that the recording link exists in the recording link DB
  if (status == STATUS_OK)
  {
    CRecordingLinkDB* pRecLinkDB = ::GetRecordingLinkDB();
    if (pRecLinkDB)
    {
      const char* pRecLinkPartyName = pCommConf->GetRecLinkName();
      if (pRecLinkPartyName)
      {
        pRecLinkParty = pRecLinkDB->GetParty(pRecLinkPartyName);
        if (pRecLinkParty == NULL)
        {
          TRACEINTO << "CConfPartyManager::OnAddRecordingLinkParty - Failed, can't find the recording link party name:" << pRecLinkPartyName;
          status = STATUS_ILLEGAL;
        }
      }
      else
      {
        TRACESTR(eLevelError) << "CConfPartyManager::OnAddRecordingLinkParty - Failed, the 'pRecordingLinkPartyName' is NULL, ConfId:" << confId;
        status = STATUS_ILLEGAL;
      }
    }
    else
    {
      TRACESTR(eLevelError) << "CConfPartyManager::OnAddRecordingLinkParty - Failed, the 'pRecordingLinkDB' is NULL, ConfId:" << confId;
      status = STATUS_ILLEGAL;
    }

  }

  //validate that there is no recording party already in the conference.
  if (status == STATUS_OK && pCommConf->IncludeRecordingParty())
  {
    TRACEINTO << "CConfPartyManager::OnAddRecordingLinkParty - Failed, there is already recording party in the conference, ConfId:" << confId;
    status = STATUS_ILLEGAL;
  }

  if (status == STATUS_OK && pCommConf)
  {
    // Build reservation for the party
    CRsrvPartyAdd* recPartyAdd = new CRsrvPartyAdd;
    CRsrvParty* recParty = recPartyAdd->GetRsrvParty();
    *recParty = *pRecLinkParty; //we copy all the properties from the recording link party

    recParty->SetConnectionType(DIAL_OUT);
    recParty->SetUndefinedType(UNRESERVED_PARTY);
    recParty->SetName("Recording");
    recParty->SetVideoMute(isMuteVideo);
    BYTE isAudioOnlyRec = pCommConf->GetIsAudioOnlyRecording();
    recParty->SetVoice(isAudioOnlyRec);

    recParty->SetIsEncrypted(AUTO);
    recParty->SetRecordingLinkParty(YES);
    recParty->SetIdentificationMethod(CALLING_PHONE_NUMBER_IDENTIFICATION_METHOD);
    recParty->SetMeet_me_method(MEET_ME_PER_MCU);

    //Add party to the conference
    CRequest request;
    recPartyAdd->SetConfID(confId);
    request.SetConnectId(0);
    request.SetRequestObject(recPartyAdd);
    DWORD undefId = 0;
    status = OnServerAddPartyToConfByType(&request, undefId);
  }

  if ((status != STATUS_OK) && (status != (STATUS_PARTY_NAME_EXISTS|WARNING_MASK)))
  {
    // Send reject to conference
    TRACESTR(eLevelError) << "CConfPartyManager::OnAddRecordingLinkParty - Failed, status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();

    if (pCommConf)
    {
      CConfApi confApi;
      confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
      confApi.RejectAddRecordingLinkParty(status);
      confApi.DestroyOnlyApi();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::RemoveUnreservedParty(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal/*|AUDBRDG_TRACE*/,"CConfMngr::RemoveUnreservedParty");
	DWORD dwConfId = 0xFFFFFFFF, dwPartyId = 0xFFFFFFFF;
	WORD undefId = 0xFFFF;
	//ALLOCBUFFER(pPartyName,H243_NAME_LEN);
	//pPartyName[0]='\0';

	*pParam >> dwConfId
			>> dwPartyId
			>> undefId;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(dwConfId) ;
	if (pCommConf)
	{
		CConfParty* pConfParty = pCommConf->GetUnreservedParty(dwPartyId);
		if(!pConfParty)
			pConfParty = pCommConf->GetCurrentParty(dwPartyId);
		if(pConfParty)
		{

		   //Del party from conference
		   CRequest* pRequest = new CRequest ;
		   CRsrvPartyAction* unResParty = new CRsrvPartyAction;

		   unResParty->SetPartyID(dwPartyId);
		   unResParty->SetConfID(dwConfId);

		   pRequest->SetConnectId(0);
		   pRequest->SetRequestObject(unResParty);

		  //POBJDELETE(undefParty)//Request Delete
		 //DEALLOCBUFFER(h243name);
		 // DEALLOCBUFFER(confName);

		   OnDelParty(pRequest,undefId);
		   delete pRequest;
		}
	}


		/*


			CSetRequest setRequest;
			setRequest.m_mes_header = 0;
			setRequest.m_objectFlag = 1;
			setRequest.m_action = DEL_PARTY;
			setRequest.SetPrivateName(dwConfId,0);
			char str[20];
			sprintf(str,"%d", pConfParty->GetPartyId());
			setRequest.SetPartyName(str);

			DelParty(setRequest,NO);
		}
	}
	DEALLOCBUFFER(pPartyName);*/
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerUpdateAction(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACEINTO << "Failed, No permission to (for administrator read-only)";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS       status      = STATUS_OK;
	CCommRes*    pRsrv       = new CCommRes;
	CCommRes*    pRsrvBackup = new CCommRes;
	CCommResAdd* pCommResAdd = new CCommResAdd;

	*pCommResAdd = *(CCommResAdd*)pRequest->GetRequestObject();
	*pRsrv       = *(pCommResAdd->GetCommResApi());
	*pRsrvBackup = *pRsrv;

	// Fill display name
	pRsrv->FillEmptyDiplayNameOrName();
	DWORD profileId = 0xFFFFFFFF;
	if (pRsrvBackup->IsConfFromProfile(profileId) == YES)
	{
		CCommRes* pProfile = ::GetpProfilesDB()->GetCurrentRsrv(profileId);
		if (pProfile)
			pRsrvBackup->SetMyProfileBasedParams(pProfile);
		else
			status = STATUS_PROFILE_NOT_FOUND;

		POBJDELETE(pProfile);
	}

	// ANY VALIDITY TEST  FOR OnServerUpdateAction SHOUND BE ADDED IN TestUpdateValidity() (and not in this function).
	if (STATUS_OK == status)
	{
		// First check is for Auto Correction of Reservation parameters which are not profile based.
		pRsrv->TestReservValidityOfCommonParams();

		// Perform test and Auto Correction on original object, in case of a profile
		if (pRsrv->IsTemplate())
			status = pRsrv->TestUpdateValidity();
		else
			status = pRsrvBackup->TestUpdateValidity();

		// Romem 22.7 - reset monitor party ID for each party and allocate a new one
		pRsrv->ResetMonitorIdForAllParties();
		pRsrv->SetMonitorIdForAllParties();
	}

	if ((STATUS_OK == status) || (status & WARNING_MASK))
	{
		TRACEINTO
			<< "ProfileName:" << pRsrv->GetName()
			<< ", IsTemplate:"     << (int)pRsrv->IsTemplate()
			<< ", IsConfTemplate:" << (int)pRsrv->IsConfTemplate()
			<< ", IsMeetingRoom:"  << (int)pRsrv->IsMeetingRoom()
			<< ", IsNormalRes:"    << (int)pRsrv->IsNormalRes();

		if (pRsrv->IsTemplate() || pRsrv->IsConfTemplate())
		{
			if (SUPER == pRequest->GetAuthorization() || ORDINARY == pRequest->GetAuthorization())
			{
				if (pRsrv->IsTemplate())
				{
					// patch to set WARNING for template
					STATUS updateProfStatus = STATUS_OK;
					updateProfStatus = UpdateProfile(pRsrv);
					if (updateProfStatus == STATUS_OK)
					{
						CResRsrcCalculator rsrcCalc;
						eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
						eVideoPartyType vidtype = rsrcCalc.GetRsrcVideoType(systemCardsBasedMode, pRsrv);

						CSegment* pRASeg = new CSegment;
						PROFILE_IND_S profile_ind_s;
						profile_ind_s.profile_Id = pRsrv->GetMonitorConfId();
						profile_ind_s.maxVideoPartyType = vidtype;
						pRASeg->Put((BYTE*)&profile_ind_s, sizeof(PROFILE_IND_S));
						// Send ASynch Msg to the RA process
						SendAsyncMsgToRsrcProcess(pRASeg, PROFILE_UPDATE_RSRC_IND);
					}
					else
						status = updateProfStatus;
				}
				else if (pRsrv->IsConfTemplate())
					status = UpdateConfTemplate(pRsrv);
			}
			else
			{
				pRequest->SetConfirmObject(new CDummyEntry());
				TRACEINTO << "Failed, No permission to update profile";
				pRequest->SetStatus(STATUS_NO_PERMISSION);
				POBJDELETE(pCommResAdd);
				POBJDELETE(pRsrv);  // pCommResAdd deleted by skeleton
				POBJDELETE(pRsrvBackup);
				return STATUS_OK;
			}
		}
		else if (pRsrv->IsMeetingRoom())
		{
			status = ValidateUpdateMR(pRsrv);   // Check update Validity
			if (STATUS_OK == status)
			{
				pRsrv->SetIsCOPReservation(pRsrvBackup->IsCOPReservation());  // 2 modes cop/cp
				for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
				{
					pRsrv->SetServiceRegistrationContentServiceName(i, pRsrvBackup->GetServiceRegistrationContentServiceName(i)); // In multiple services add services
					pRsrv->SetServiceRegistrationContentRegister(i, pRsrvBackup->GetServiceRegistrationContentRegister(i));       // In multiple services add services
					pRsrv->SetServiceRegistrationContentAcceptCall(i, pRsrvBackup->GetServiceRegistrationContentAcceptCall(i));   // In multiple services add services
				}

				pCommResAdd->SetCommResApi(pRsrv);
				pRequest->SetRequestObject(pCommResAdd); // Set the new Object to be Send to Resources

				STATUS confStatus = pRsrvBackup->GetInternalConfStatus();
				if (STATUS_OK != confStatus)
					m_dwInternalConfStatus = confStatus;   // olga (VNGR-8621)

				POBJDELETE(pRsrv);                       // pCommResAdd deleted by skeleton
				POBJDELETE(pRsrvBackup);
				return STATUS_FW_REQUEST_TO_RESOURCE;
			}
		}
		else if (pRsrv->IsNormalRes()) // updating of reservation is done through Resource process. They will send response to EMA
		{
			// Anything else to be checked or done before sending to Resource process???
			pRsrv->SetIsCOPReservation(pRsrvBackup->IsCOPReservation());  // 2 modes cop/cp
			for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
			{
				const char* Service_nm = pRsrvBackup->GetServiceRegistrationContentServiceName(i);
				if (Service_nm[0] != '\0')
				{
					pRsrv->SetServiceRegistrationContentServiceName(i, pRsrvBackup->GetServiceRegistrationContentServiceName(i)); // In multiple services add services
					pRsrv->SetServiceRegistrationContentRegister(i, pRsrvBackup->GetServiceRegistrationContentRegister(i));       // In multiple services add services
					pRsrv->SetServiceRegistrationContentAcceptCall(i, pRsrvBackup->GetServiceRegistrationContentAcceptCall(i));   // In multiple services add services
					pRsrv->SetServiceRegistrationContentStatus(i, pRsrvBackup->GetServiceRegistrationContentStatus(i));           // In multiple services add services - sipProxySts
				}
			}

			pRsrv->SetSipRegistrationTotalSts(pRsrvBackup->GetSipRegistrationTotalSts());  // sipProxySts

			pCommResAdd->SetCommResApi(pRsrv);
			pRequest->SetRequestObject(pCommResAdd); // Set the new Object to be Send to Resources

			STATUS confStatus = pRsrvBackup->GetInternalConfStatus();
			if (STATUS_OK != confStatus)
				m_dwInternalConfStatus = confStatus;   // olga (VNGR-8621)

			POBJDELETE(pRsrv);                       // pCommResAdd deleted by skeleton
			POBJDELETE(pRsrvBackup);
			return STATUS_FW_REQUEST_TO_RESOURCE;
		}
	}

	if (status != STATUS_OK)
	{
		TRACEINTO << "ProfileName:" << pRsrv->GetName() << ", Status:" << status << " - Failed";
		POBJDELETE(pCommResAdd);
		pRequest->SetConfirmObject(new CDummyEntry());
	}
	else
	{
		pCommResAdd->SetCommResApi(pRsrv);
		pRequest->SetConfirmObject(pCommResAdd);
	}

	std::string responseTrancsName("TRANS_RES"); // Instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetStatus(status);

	POBJDELETE(pRsrv);
	POBJDELETE(pRsrvBackup);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::CreateProfile(CCommRes* pCommRes)
{
	TRACEINTO << "ProfileName:" << pCommRes->GetName();

	STATUS status = pCommRes->TestAddValidity();
	if (STATUS_OK != status && status != STATUS_ILLEGAL_SYS_MODE)
	{
		TRACEINTO << "ProfileName:" << pCommRes->GetName() << ", Status:" << status << " - Failed on validation";
		PASSERT(status);
		AddActiveAlarm(FAULT_GENERAL_SUBJECT, AA_CAN_NOT_CREATE_DEFAULT_PROFILE, MAJOR_ERROR_LEVEL, "Failed to validate the Profile", true, true);
		return status;
	}

	status = AddProfile(pCommRes);
	if (STATUS_OK != status)
	{
		TRACEINTO << "ProfileName:" << pCommRes->GetName() << ", Status:" << status << " - Failed on adding";
		PASSERT(status);
		AddActiveAlarm(FAULT_GENERAL_SUBJECT, AA_CAN_NOT_CREATE_DEFAULT_PROFILE, MAJOR_ERROR_LEVEL, "Failed to add the Profile", true, true);
		return status;
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::AddProfile(CCommRes* pCommRes)
{
	DWORD profileId = ::GetpProfilesDB()->GetDBCounter();
	pCommRes->SetMonitorConfId(++profileId);

	// save the highest index seen (as "Profile_X") to roll back if 'add' fails
	DWORD profileHighestIndexId = ::GetpProfilesDB()->GetDBHighestIndexCounter();
	::GetpProfilesDB()->SetDBCounter(profileId);

	// set routing name for profile if display name is not ascii
	char profileRoutingName[H243_NAME_LEN+1];
	// uses 'GetDBHighestIndexCounter' value to avoid VNGR 23180
	::GetpProfilesDB()->CreateNewValidDefaultRoutingName(profileRoutingName);

	pCommRes->FillEmptyDiplayNameOrName(profileRoutingName);

	TRACEINTO << "ProfileName:" << pCommRes->GetName() << ", ProfileId:" << profileId;

	// Add a new profile to the profile DB
	STATUS status = ::GetpProfilesDB()->Add(*pCommRes);
	if (status != STATUS_OK)
	{
		TRACEINTO << "ProfileName:" << pCommRes->GetName() << ", Status:" << status << " - Failed to add new profile to templates DB";

		::GetpProfilesDB()->SetDBCounter(profileId-1);                       // A new Conf Template was not added, decrease the counter
		// roll back DBHighestIndexCounter
		::GetpProfilesDB()->SetDBHighestIndexCounter(profileHighestIndexId); // A new profile was not added, decrease the counter
		return status;
	}
	else
	{
		if (strncmp(pCommRes->GetName(), profileRoutingName, H243_NAME_LEN) != 0)
		{
			// we didn't use the default name generated.
			// first- roll back the "highest seen" index
			::GetpProfilesDB()->SetDBHighestIndexCounter(profileHighestIndexId);
			// second, check for a VNGR-23180 scenario (like in an import scenario)
			// check if it was in "template_<ID>" format, and we should update the "highest seen" index
			::GetpProfilesDB()->ParseGivenRoutingNameForIndex(pCommRes->GetName());
		}
	}
	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::AddProfileToSlave(CCommRes* pCommRes)
{
	DWORD current_max_profile_id = ::GetpProfilesDB()->GetDBCounter();
	DWORD new_profile_id = pCommRes->GetMonitorConfId();

	TRACEINTO << "ProfileName:" << pCommRes->GetName() << ", ProfileId:" << current_max_profile_id;

	// Add a new profile to the profile DB
	STATUS status = ::GetpProfilesDB()->Add(*pCommRes);
	if (status != STATUS_OK)
	{
		TRACEINTO << "ProfileName:" << pCommRes->GetName() << ", Status:" << status << " - Failed to add new profile to templates DB";
		return status;
	}
	else
	{
		if (new_profile_id > current_max_profile_id)
		{
			// VNGR-23048
			::GetpProfilesDB()->SetDBCounter(new_profile_id);
		}

		::GetpProfilesDB()->ParseGivenRoutingNameForIndex(pCommRes->GetName());
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnServerDelProfile(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == SUPER || pRequest->GetAuthorization() == ORDINARY)
	{
		CCommResDBAction* pCommResDBAction = new CCommResDBAction;
		*pCommResDBAction = *(CCommResDBAction*)pRequest->GetRequestObject();
		STATUS status = STATUS_OK;

		DWORD confId = pCommResDBAction->GetConfID();

		std::ostringstream msg;
		msg << "ProfileId:" << confId;

		CCommRes* pCommRes = ::GetpProfilesDB()->GetCurrentRsrv(confId);
		if (!pCommRes)
		{
			TRACEINTO << msg.str().c_str() << " - Failed, Cannot find profile in DB";
			status = STATUS_PROFILE_NOT_FOUND;
		}

		if (status == STATUS_OK)
		{
			msg << ", ProfileName:" << pCommRes->GetName();
			TRACEINTO << msg.str().c_str() << " - Request to delete profile";
		}

		if (status == STATUS_OK && ::GetpProfilesDB()->IsFirstProfile(confId))
		{
			TRACEINTO << msg.str().c_str() << " - Failed, Profile is selected as Default Profile";
			status = DEFAULT_PROFILE_CANNOT_BE_DELETED;
		}

		if (status == STATUS_OK && CCOPConfigurationList::GetProfileInfo(pCommRes->GetName()))
		{
			TRACEINTO << msg.str().c_str() << " - Failed, Profile is selected as COP Default Profile";
			status = DEFAULT_PROFILE_CANNOT_BE_DELETED;
		}

		if (status == STATUS_OK && ::GetpMeetingRoomDB()->IsREservationUsingProfile(confId))
		{
			TRACEINTO << msg.str().c_str() << " - Failed, Profile is already used in other reservation";
			status = STATUS_PROFILE_IN_USE_CANNOT_BE_DELETED;
		}

		if (status == STATUS_OK && ::GetpConfTemplateDB()->IsREservationUsingProfile(confId))
		{
			TRACEINTO << msg.str().c_str() << " - Failed, Profile is already used in other conference template";
			status = STATUS_PROFILE_IN_USE_CANNOT_BE_DELETED;
		}

		if (STATUS_OK == status)
			status = ::GetpProfilesDB()->Cancel(confId);

		if (STATUS_OK == status)
		{
			// Bridge-10335
			TRACEINTO << "Reset ad hoc profile id to -1 for those conf using this profile.";
			::GetpConfDB()->ResetAdHocProfileId(confId);
		}

		switch (status)
		{
			case STATUS_OK:
			case STATUS_PROFILE_NOT_FOUND:
			case STATUS_PROFILE_IN_USE_CANNOT_BE_DELETED:
			case DEFAULT_PROFILE_CANNOT_BE_DELETED:
			{
				pRequest->SetConfirmObject(pCommResDBAction);
				break;
			}
			default:
			{
				TRACEINTO << msg.str().c_str() << ", Status:" << status << " - Failed";
				PASSERT(1);
				POBJDELETE(pCommResDBAction);
				pRequest->SetConfirmObject(new CDummyEntry());
				break;
			}
		}

		pRequest->SetStatus(status);
		pRequest->SetTransName("TRANS_RES");

		if (STATUS_OK == status)
		{
			CSegment* pRASeg = new CSegment;
			PROFILE_IND_S profile_ind_s;
			profile_ind_s.profile_Id = confId;
			profile_ind_s.maxVideoPartyType = eVideo_party_type_dummy;
			pRASeg->Put((BYTE*)&profile_ind_s, sizeof(PROFILE_IND_S));
			SendAsyncMsgToRsrcProcess(pRASeg, PROFILE_DELETE_RSRC_IND);
		}

		POBJDELETE(pCommRes);
	}
	else
	{
		pRequest->SetConfirmObject(new CDummyEntry());
		TRACEINTO << "Failed, no permission to delete profile";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::UpdateProfile(CCommRes* pCommRes)
{
	// Update a specified profile to the profiles DB
	STATUS status = ::GetpProfilesDB()->Update(*pCommRes);
	TRACEINTO << "ProfileName:" << pCommRes->GetName() << ", Status:" << status;

	PASSERT(status != STATUS_OK);

	return status;
}

//---------------------------  Conf Templates DB Methods -----------------------------
STATUS CConfPartyManager::AddConfTemplate(CCommRes * commRes)
{
	PTRACE(eLevelInfoNormal, "ConfPartyManager::AddConfTemplate :  Adding a new ConfTemplate to Conf templates DB ");

	DWORD confTemplateId = ::GetpConfTemplateDB()->GetDBCounter();
	commRes->SetMonitorConfId(++confTemplateId);
	::GetpConfTemplateDB()->SetDBCounter(confTemplateId);

	// set routing name for profile if display name is not ASCII
	CSmallString confTemplateRoutingName;
	confTemplateRoutingName << "template_" << confTemplateId;
	commRes->FillEmptyDiplayNameOrName(confTemplateRoutingName.GetString());

	//Add a new Conf Template  to Conf Templates DB
	STATUS status= (::GetpConfTemplateDB())->Add(*commRes);

	if (status != STATUS_OK)
	{
		PTRACE(eLevelError, "CConfPartyManager::AddConfTemplate :  Failed Adding a new Conf Template to Conf templates DB");
		::GetpConfTemplateDB()->SetDBCounter(confTemplateId-1); //A new Conf Template was not added, decrease the counter
		return status;
	}

	return STATUS_OK;
}

STATUS CConfPartyManager::UpdateConfTemplate(CCommRes * commRes)
{
	PTRACE(eLevelInfoNormal, "UpdateConfTemplate :  Updating a Conf Template on Conf templates DB");

	//Update a specified profile to the profiles DB
	STATUS status= (::GetpConfTemplateDB())->Update(*commRes);
	if ( status != STATUS_OK)
		{
			PTRACE(eLevelError, "CConfPartyManager::UpdateConfTemplate :  Failed Update a Conf Template to Conf Templates DB");
			PASSERT(1);
			return status;
		}
	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerDelConfTemplate(CRequest* pRequest)
{
  STATUS status = STATUS_OK;
  PTRACE(eLevelInfoNormal, "CConfPartyManager::OnServerDelConfTemplate :  Request for delete a Conf Template ");
  if ( pRequest->GetAuthorization() == SUPER  ||  ORDINARY == pRequest->GetAuthorization() )
  {
        CCommResDBAction * pCommResDBAction=new CCommResDBAction;
        *pCommResDBAction = *(CCommResDBAction*)pRequest->GetRequestObject() ;
        STATUS status = STATUS_OK;

        const DWORD confTemplateID = pCommResDBAction->GetConfID();
        CCommRes* pDeletedConfTemplate = NULL;
        char deletedConfTemplateName[H243_NAME_LEN];
        memset(deletedConfTemplateName,'\0',H243_NAME_LEN);
        pDeletedConfTemplate = ::GetpConfTemplateDB()->GetCurrentRsrv(confTemplateID);

        if(!CPObject::IsValidPObjectPtr(pDeletedConfTemplate))
        {
        	PASSERT(1);
        	PTRACE(eLevelError, "CConfPartyManager::OnServerDelConfTemplate :  Conf Template not found ");
        }
        else
        {
        	strncpy(deletedConfTemplateName,pDeletedConfTemplate->GetName(),sizeof(deletedConfTemplateName) - 1);
        	deletedConfTemplateName[sizeof(deletedConfTemplateName) - 1] = '\0';
			/*to free the memory of pDeletedConfTemplate*/
			POBJDELETE(pDeletedConfTemplate);
        }

        status = ::GetpConfTemplateDB()->Cancel(confTemplateID);

        if(status != STATUS_OK)
        {
        	PTRACE2(eLevelError, "CConfPartyManager::OnServerDelConfTemplate :  Failed Delete a conf Template from Conf Template DB. Conf Template Name:",deletedConfTemplateName);
        	PASSERT(1);
        	POBJDELETE(pCommResDBAction);
        	pRequest->SetConfirmObject(new CDummyEntry());
        }
        else
  	        pRequest->SetConfirmObject(pCommResDBAction);

        pRequest->SetStatus(status);
        pRequest->SetTransName("TRANS_RES");
  }
  else
  {
        pRequest->SetConfirmObject(new CDummyEntry());
        FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerDelConfTemplate : No permission to delete Conf Template");
        pRequest->SetStatus(STATUS_NO_PERMISSION);
  }

    return STATUS_OK;
  }
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetTransitEQ(CRequest* pRequest)
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
       FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetTransitEQ: No permission to OnServerSetTransitEQ for administrator readony");
       pRequest->SetConfirmObject(new CDummyEntry());
       pRequest->SetStatus(STATUS_NO_PERMISSION);
       return STATUS_NO_PERMISSION;
    }
	STATUS status = STATUS_OK;
	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerSetTransitEQ : Set default EQ service completed");
	CCommResDBAction * pCommResDBAction=new CCommResDBAction;
	*pCommResDBAction = *(CCommResDBAction*)pRequest->GetRequestObject() ;
    pRequest->SetObjectFlag(STRING_FLAG);

    ALLOCBUFFER(transitEQName,H243_NAME_LEN);
    strncpy(transitEQName,pCommResDBAction->GetEqName(),H243_NAME_LEN);
    transitEQName[H243_NAME_LEN - 1] = '\0';

	CCommResDB::ReservArray::iterator itr=::GetpMeetingRoomDB()->FindName(transitEQName);
	if(*itr != NULL)
	{
		::GetpMeetingRoomDB()->SetTransitEQName(transitEQName);//should be in commResDB
		PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerSetTransitEQ : Find EQ");
	}
	else
	{
		status = STATUS_NOT_FOUND;
	}

    ///new confirm
//	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
//	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pCommResDBAction);
	pRequest->SetStatus(status);
	//end new confirm
	DEALLOCBUFFER(transitEQName);
    return status;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerCancelTransitEQ(CRequest* pRequest)
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
       FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerCancelTransitEQ: No permission to OnServerCancelTransitEQ for administrator readony");
       pRequest->SetConfirmObject(new CDummyEntry());
       pRequest->SetStatus(STATUS_NO_PERMISSION);
       return STATUS_NO_PERMISSION;
    }

	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerCancelTransitEQ");
	STATUS status=STATUS_OK;

	CCommResDBAction * pCommResDBAction=new CCommResDBAction;
	*pCommResDBAction = *(CCommResDBAction*)pRequest->GetRequestObject() ;
    pRequest->SetObjectFlag(STRING_FLAG);

    ALLOCBUFFER(transitEQName,H243_NAME_LEN);
    strncpy(transitEQName,pCommResDBAction->GetEqName(),H243_NAME_LEN);

    if(!strncmp(transitEQName,::GetpMeetingRoomDB()->GetTransitEQName(),H243_NAME_LEN))
    {
/*    	CCommConf* pCurConf = ::GetpConfDB()->GetCurrentOnGoingEQ(transitEQName);
    	if(NULL != pCurConf)
    	{
    		 // Send Auto terminate to conf
		    CConfApi* pConfApi = new CConfApi;
			pConfApi->CreateOnlyApi(*(pCurConf->GetRcvMbx()));
		//	CSegment* seg = new CSegment;
		//	*seg << transitEQName;
    		pConfApi->SetAutoTerminateTimer();
    	//	POBJDELETE(seg);
    		POBJDELETE(pConfApi);
    	}
		else
		{
			PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerCancelTransitEQ, There is no ongoing conference from the default EQ");
		}
*/
		::GetpMeetingRoomDB()->CancelTransitEQ();
    }
	else
	{
		PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerCancelTransitEQ, There is no match to Transit EQ : ",
		::GetpMeetingRoomDB()->GetTransitEQName());
		status = STATUS_NOT_FOUND;
	}

//Trace
    PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerCancelTransitEQ, "
		  , CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

    ///new confirm
//	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
//	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pCommResDBAction);
	pRequest->SetStatus(status);
	//end new confirm
	DEALLOCBUFFER(transitEQName);
    return status;

}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerAddIVRService(CRequest* pRequest)
{
	PTRACE(eLevelError,"CConfPartyManager::OnServerAddIVRService " );

	if (pRequest->GetAuthorization() == SUPER)
	{
		int status = STATUS_OK;
		int status_check = STATUS_OK;

	  	const WORD conId = pRequest->GetConnectId();

	  	// Check user's permission
		///	WORD perm = ::Permission(conId);
		///	if(perm==ORDINARY)
		///		status=STATUS_NO_PERMISSION;

		// Create instance of CIVRServiceAdd
		CIVRServiceAdd* pIVRServiceAddTemp = (CIVRServiceAdd*)pRequest->GetRequestObject();
		CIVRServiceAdd* pIVRServiceAdd = new CIVRServiceAdd;
		*pIVRServiceAdd = *pIVRServiceAddTemp;

		// Create instance of CAVmsgService - ?????****????? - consider change the format as above
		CAVmsgService* pAVmsgService = new CAVmsgService;
		*pAVmsgService = *(pIVRServiceAdd->GetAVmsgService());

		CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();
		if (NULL == pAVmsgServiceList)
		{
			PTRACE(eLevelError,"CConfPartyManager::OnServerAddIVRService - Internal Error - Illegal pointer to IVR List" );
			status = STATUS_IVR_INTERNAL_ERROR;	// shouldn't happened
		}
		else
			if (pAVmsgServiceList->GetServNumber() >= MAX_IVR_SERV_IN_LIST)
			{
				PTRACE(eLevelError,"CConfPartyManager::OnServerAddIVRService - Error: Number of IVR services exceeded" );
				status = STATUS_MAX_NUMBER_OF_IVR_SERVICES_EXCEEDED;	// shouldn't happened
			}

		// checks for legal IVR Service
		if (status == STATUS_OK)
		{
			string err = " ";
			WORD chkLevel = 2;	// 0: don't check media, 1: check media existance, 2: check media content
			pAVmsgService->SelfCorrections();
			status = pAVmsgService->IsLegalService( chkLevel, err );
			if (status != STATUS_OK)
				PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerAddIVRService - Error - Illegal Service: ", err.c_str());
		}

		// checks if name already exists in IVR list
		if (status == STATUS_OK)
		{
			int index = pAVmsgServiceList->FindAVmsgServ(pAVmsgService->GetName());
			if (-1 != index) {
				PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerAddIVRService - Service name already exists ", pAVmsgService->GetName());
				status = STATUS_IVR_SERVICE_NAME_EXISTS;
			}
		}

		if (status == STATUS_OK)	// all is legal, we can add the IVR to the list
		{
			// Create a new temporary IVR service list identical to the global list
			CAVmsgServiceList* pTmpAVmsgServiceList = new CAVmsgServiceList;
			*pTmpAVmsgServiceList = *pAVmsgServiceList;

			// Add the new service to the temporary list
			status =  pTmpAVmsgServiceList->AddOnlyMem(*pAVmsgService);
			if (status != STATUS_OK)
			{
				PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerAddIVRService - AddOnlyMem error ", pAVmsgService->GetName());
			}

			if (status == STATUS_OK)	// the IVR service was added to the list
			{
				// set as default if needed (e.g. it was the first IVR in list)
				pTmpAVmsgServiceList->SetAsDefaultIfNeeded( pAVmsgService );

				// save the temporary updated IVR service list to disk
				status = pTmpAVmsgServiceList->SecureSaveIvrListToFile();
				if (status == STATUS_OK)
				{
					// Set the global IVR service list to point on the updated temporary list
					::SetpAVmsgServList(pTmpAVmsgServiceList);
					POBJDELETE(pAVmsgServiceList)	// Delete the original IVR service list

					// update Slides List
					CIVRSlidesList* pSlidesList = ::GetpSlidesList();
					if (pSlidesList)
						pSlidesList->AddSlide( pAVmsgService->GetSlideName() );	// the slide may already exists
				}
			}

			if (status != STATUS_OK)
			{
				POBJDELETE(pTmpAVmsgServiceList)	// Delete the new temporary IVR service list - in case of a failure use the original list
			}

			// Remove Active Alarm if needed
			if (status == STATUS_OK)
			{
				if (STATUS_OK == ::GetpAVmsgServList()->CheckIvrListValidity())
				{
					CProcessBase *pProcess = CProcessBase::GetProcess();
					pProcess->RemoveActiveAlarmFromProcess(AA_IVR_SERVICE_LIST_MISSING_DEFAULT_SERVICE);

					PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerAddIVRService - The IVR Service List is legal now!");
				}
			}

		}

		// Confirm

		PTRACE(eLevelError,"CConfPartyManager::OnServerAddIVRService Seting Confirm Object" );

		pRequest->SetConfirmObject(pIVRServiceAdd);
		pRequest->SetStatus(status);

		POBJDELETE(pAVmsgService);
	}
	else
	{
		pRequest->SetConfirmObject(new CDummyEntry());
		FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerAddIVRService: No permission to add ivr service");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}
 	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerUpdateIVRService(CRequest* pRequest)
{
	PTRACE(eLevelError,"CConfPartyManager::OnServerUpdateIVRService " );

	if (pRequest->GetAuthorization() == SUPER)
	{
		int status=STATUS_OK;
		int status_check = STATUS_OK;

	 	const WORD conId = pRequest->GetConnectId();

	  	// Check user's permission
		///	WORD perm = ::Permission(conId);
		///	if(perm==ORDINARY)OnServerUpdateIVRService
		///		status=STATUS_NO_PERMISSION;

		// Create instance of CIVRServiceAdd
		CIVRServiceAdd* pIVRServiceAddTemp = (CIVRServiceAdd*)pRequest->GetRequestObject();
		CIVRServiceAdd* pIVRServiceAdd = new CIVRServiceAdd;
		*pIVRServiceAdd = *pIVRServiceAddTemp;

		// Create instance of CAVmsgService - ?????****????? - consider change the format as above
		CAVmsgService* pAVmsgService = new CAVmsgService;
		*pAVmsgService = *(pIVRServiceAdd->GetAVmsgService());

		CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();
		if (NULL == pAVmsgServiceList)
		{
			PTRACE(eLevelError,"CConfPartyManager::OnServerUpdateIVRService - Internal Error - Illegal pointer to IVR List" );
			status = STATUS_IVR_INTERNAL_ERROR;	// shouldn't happened
		}

		// checks for legal IVR Service
		if (status == STATUS_OK)
		{
			string err = " ";
			WORD chkLevel = 2;	// 0: don't check media, 1: check media existance, 2: check media content
			pAVmsgService->SelfCorrections();
			status = pAVmsgService->IsLegalService( chkLevel, err );
			if (status != STATUS_OK)
				PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateIVRService - Error - Illegal Service: ", err.c_str());
		}

		// checks if name already exists in IVR list
		if (status == STATUS_OK)
		{
			int index = pAVmsgServiceList->FindAVmsgServ(pAVmsgService->GetName());
			if (-1 == index)
				status = STATUS_IVR_SERVICE_NAME_DOES_NOT_EXISTS;
		}


//		if (status == STATUS_OK) 	// all is legal, we can update the IVR list
//		{
//			// Update the new service in the global list
//			status =  pAVmsgServiceList->Update(*pAVmsgService);
//
//			// Update IVR service list in disk
//			if (status == STATUS_OK)
//			{
//				pAVmsgServiceList->SaveIvrListToFile();
//
//				// update Slides List
//				CIVRSlidesList* pSlidesList = ::GetpSlidesList();
//				if (pSlidesList)
//					pSlidesList->AddSlide( pAVmsgService->GetSlideName() );	// the slide may already exists
//			}
//		}


		if (status == STATUS_OK)	// all is legal, we can add the IVR to the list
		{
			// Create a new temporary IVR service list identical to the global list
			CAVmsgServiceList* pTmpAVmsgServiceList = new CAVmsgServiceList;
			*pTmpAVmsgServiceList = *pAVmsgServiceList;

			// Update the new service in the temporary list
			status = pTmpAVmsgServiceList->Update(*pAVmsgService);

			if (status == STATUS_OK)	// the IVR service list was updated
			{
				// save the temporary updated IVR service list to disk
				status = pTmpAVmsgServiceList->SecureSaveIvrListToFile();

				if (status == STATUS_OK)
				{
					// Set the global IVR service list to point on the updated temporary list
					::SetpAVmsgServList(pTmpAVmsgServiceList);
					POBJDELETE(pAVmsgServiceList)	// Delete the original IVR service list

					// update Slides List
					CIVRSlidesList* pSlidesList = ::GetpSlidesList();
					if (pSlidesList)
						pSlidesList->AddSlide( pAVmsgService->GetSlideName() );	// the slide may already exists
				}
			}

			if (status != STATUS_OK)
				{POBJDELETE(pTmpAVmsgServiceList)}	// Delete the new temporary IVR service list - in case of a failure use the original list
		}

		// Confirm
		pRequest->SetConfirmObject(pIVRServiceAdd);
		pRequest->SetStatus(status);
		POBJDELETE(pAVmsgService);
	}
	else
	{
		pRequest->SetConfirmObject(new CDummyEntry());
		FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateIVRService: No permission to update ivr service");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}
 	return STATUS_OK;
}

/*
////////////////////////////////////////////////////////////////////////////////////////
int CConfPartyManager::CheckIVRLegalNames( const CAVmsgService* pAVmsgServ)
{
	const CIVRService* pIVRService = pAVmsgServ->GetIVRService();
	int status = STATUS_OK;
	WORD eventOpCode = 0;

// IVR_FEATURE_LANG_MENU   				1
// IVR_FEATURE_CONF_PASSWORD			2
// IVR_FEATURE_PIN_CODE	  		    	3
// IVR_FEATURE_OPER_ASSISTANCE			4
// IVR_FEATURE_WELCOME                	5
// IVR_FEATURE_CONF_LEADER            	6
// IVR_FEATURE_GENERAL		            7
// IVR_FEATURE_BILLING_CODE            	8
// IVR_FEATURE_INVITE_PARTY            	9
// IVR_FEATURE_ROLL_CALL				10
// IVR_FEATURE_VIDEO					11
// IVR_FEATURE_NUMERIC_CONFERENCE_ID   	12
// IVR_FEATURE_MUTE_NOISY_LINE          13

	for (int feature_opcode = 1; feature_opcode < 14; feature_opcode++) {
		if (feature_opcode == IVR_FEATURE_LANG_MENU)
			continue;		// currently not in use
		if (feature_opcode == IVR_FEATURE_PIN_CODE)
			continue;		// currently not in use
		if (feature_opcode == IVR_FEATURE_GENERAL)	// don't check the 'general' feature
			continue;
		if (feature_opcode == IVR_FEATURE_ROLL_CALL)
			continue;
		if (feature_opcode == IVR_FEATURE_NUMERIC_CONFERENCE_ID)
			continue;
		if (feature_opcode == IVR_FEATURE_VIDEO)
			continue;
		if (feature_opcode == IVR_FEATURE_INVITE_PARTY)
			continue;
		if (feature_opcode == IVR_FEATURE_BILLING_CODE)
			continue;

	    CIVRFeature* pIVRFeature = pIVRService->GetIVRFeature(feature_opcode);
		if (pIVRFeature) {	// if this feature exists
			if( NO == pIVRFeature->GetEnableDisable())
				continue;	// this feature disabled

			if (feature_opcode == IVR_FEATURE_MUTE_NOISY_LINE)
			{
				int CheckMessage = 0;
				for (int event = 0; event < 7; event++)
				{
					CIVREvent*  pEvent = pIVRFeature->GetIVREventInPos(event);
					if (!pEvent)
						status = STATUS_ILLEGAL;

					eventOpCode = pEvent->GetEventOpcode();

					CheckMessage = 0;
					switch(eventOpCode)
					{
						case IVR_EVENT_NOISY_LINE_HELP_MENU :
							{
								if(((CIVRMuteNoisyLineFeature*)pIVRFeature)->GetMuteNoisyLineMenu() == YES)
									CheckMessage = 1;
								break;
							}
						case IVR_EVENT_NOISY_LINE_MUTE :
						case IVR_EVENT_NOISY_LINE_UNMUTE_MESSAGE :
							{
								if(((CIVRMuteNoisyLineFeature*)pIVRFeature)->GetReturnAndMute() == YES)
									CheckMessage = 1;
								break;
							}
						case IVR_EVENT_NOISY_LINE_ADJUST :
							{
								if(((CIVRMuteNoisyLineFeature*)pIVRFeature)->GetAdjustNoiseDetection() == YES)
									CheckMessage = 1;
								break;
							}
						case IVR_EVENT_NOISY_LINE_DISABLE :
							{
								if(((CIVRMuteNoisyLineFeature*)pIVRFeature)->GetDisableNoiseDetection() == YES)
									CheckMessage = 1;
								break;
							}
						case IVR_EVENT_NOISY_LINE_UNMUTE :
							{
								if(((CIVRMuteNoisyLineFeature*)pIVRFeature)->GetReturnAndUnmute() == YES)
									CheckMessage = 1;
								break;
							}
    					case IVR_EVENT_PLAY_NOISY_LINE_MESSAGE :
							{
								if(((CIVRMuteNoisyLineFeature*)pIVRFeature)->GetPlayNoisyDetectionMessage() == YES)
									CheckMessage = 1;
								break;
							}
    					default :
							{
								status = STATUS_ILLEGAL;
								break;
							}
					}

					if (status == STATUS_OK && CheckMessage)
					{
						ALLOCBUFFER(fullPathMsgName, MAX_FILE_NAME_LEN);
						pIVRService->GetFullPathMsgFileName( fullPathMsgName, feature_opcode, eventOpCode, status );
						if (status == STATUS_OK && strcmp(fullPathMsgName, "(None)"))
							status = CheckIvrFile( fullPathMsgName );

						DEALLOCBUFFER(fullPathMsgName);

						if (status != STATUS_OK)
							return status;	// error
					}
				}
				continue;
			}

			for (int event = 0; event < 3; event++) {	// max 3 events in feature (currently)
				if ((event > 1) && feature_opcode != IVR_FEATURE_CONF_LEADER && feature_opcode != IVR_FEATURE_CONF_PASSWORD )	// the only one who has 3 events
					continue;	// this feature has only 2 events
				if ((event == 1) && (feature_opcode == IVR_FEATURE_WELCOME))
					if (NO == ((CIVRWelcomeFeature*)pIVRFeature)->m_bEntranceMsg)
						continue;	// 'entrance' event not configured
				if (feature_opcode == IVR_FEATURE_CONF_PASSWORD) {
					if ((event == 0 || event == 1)
						&& REQUEST_PASSWORD != ((CIVRConfPasswordFeature*)pIVRFeature)->GetDialOutEntryPassword()
						&& REQUEST_PASSWORD != ((CIVRConfPasswordFeature*)pIVRFeature)->GetDialInEntryPassword())
					{
						continue;
					}
					if(event == 2
						&& REQUEST_DIGIT != ((CIVRConfPasswordFeature*)pIVRFeature)->GetDialOutEntryPassword()
						&& REQUEST_DIGIT != ((CIVRConfPasswordFeature*)pIVRFeature)->GetDialInEntryPassword())
					{
						continue;
					}
				}
				// getting the event op-code
				CIVREvent*  pEvent = pIVRFeature->GetIVREventInPos(event);
				if (!pEvent)
					status = STATUS_ILLEGAL;

				int CheckConfPassword = 0;

				if(pIVRService->GetEntryQueueService() && feature_opcode == IVR_FEATURE_CONF_PASSWORD)
					CheckConfPassword = 1;

				if (status == STATUS_OK && !CheckConfPassword) {
					eventOpCode = pEvent->GetEventOpcode();
					ALLOCBUFFER(fullPathMsgName, MAX_FILE_NAME_LEN);
					pIVRService->GetFullPathMsgFileName( fullPathMsgName, feature_opcode, eventOpCode, status );
					if (status == STATUS_OK && strcmp(fullPathMsgName, "(None)"))
						status = CheckIvrFile( fullPathMsgName );
					DEALLOCBUFFER(fullPathMsgName);
				}
				if (status != STATUS_OK)
					return status;	// error
			}
		}

	}

	return status;
}
*/
/*
/////////////////////////////////////////////////////////////////////////////////////
int CConfPartyManager::CheckIvrFile( char *fullPathMsgName )
{
	int status = STATUS_OK;
	INT_MESSAGE_HEADER_S header_buf;
	FILE* infile = fopen( fullPathMsgName, "rb" );
	DWORD dwcontent = 2;		// ACA file

	if (infile != NULL) {
		fread( &header_buf, sizeof(INT_MESSAGE_HEADER_S), 1, infile );
		if ( strncmp((char *)header_buf.szIdCode,"ACRD",4) || header_buf.dwContent != dwcontent ){
			status = STATUS_NOT_APPROPRIATE_MSG_FILE;
			PTRACE2(eLevelInfoNormal,"CCardMngr::CheckIvrFile. Not appropriate message file: File name: ", fullPathMsgName);
		}
		fclose(infile);
	}
	else{
		status = STATUS_FILE_NOT_EXISTS;
		PTRACE2(eLevelInfoNormal,"CCardMngr::CheckIvrFile. Message file does not exist: File name: ", fullPathMsgName);
	}

	return status;
}
*/

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerDeleteIVRService(CRequest* pRequest)
{
	PTRACE(eLevelError,"CConfPartyManager::OnServerDeleteIVRService " );

	if (pRequest->GetAuthorization() == SUPER)
	{
	  	int status=STATUS_OK;
	  	const WORD conId = pRequest->GetConnectId();

	  	// Check user's permission
	///  WORD perm = ::Permission(conId);
	///  if(perm==ORDINARY)
	///    status=STATUS_NO_PERMISSION;

		// Create instance of CIVRServiceDel
		CIVRServiceDel* pIVRServiceDelTemp = (CIVRServiceDel*)pRequest->GetRequestObject();
		CIVRServiceDel* pIVRServiceDel = new CIVRServiceDel;
		*pIVRServiceDel = *pIVRServiceDelTemp;

		// Get IVR service name for deletion
	  	const char* ivrServiceName = pIVRServiceDel->GetIVRServiceName();
		PTRACE2( eLevelInfoNormal, "CConfPartyManager::OnServerDeleteIVRService - Request to delete IVR Service: ", ivrServiceName );

		CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();

		// checks if this a default IVR Service
		if (pAVmsgServiceList->IsDefaultService( ivrServiceName ))
			status = STATUS_DEFAULT_MSG_SERVICE_CANNOT_BE_REMOVED;

		// Check if the IVR Service that is going to be deleted is currently
		// used in conferences/meetingrooms/profiles
		if (status == STATUS_OK)
			status = IsAVServiceUsedBySomeConf(ivrServiceName);

		// Delete IVR service from temporary list and update XML file in disk
	  	if (status == STATUS_OK)
	  	{
			// Create a new temporary IVR service list identical to the global list
			CAVmsgServiceList* pTmpAVmsgServiceList = new CAVmsgServiceList;
			*pTmpAVmsgServiceList = *pAVmsgServiceList;

			status =  pTmpAVmsgServiceList->Cancel(ivrServiceName);
	  		if (status == STATUS_OK)	// the IVR service list was updated
			{
				// Update IVR service list in disk after deletion
				status = pTmpAVmsgServiceList->SecureSaveIvrListToFile();
				if (status == STATUS_OK)
				{
					::SetpAVmsgServList(pTmpAVmsgServiceList);		// set the updated IVR list to be the global
					POBJDELETE(pAVmsgServiceList)					// Delete the original IVR service list
				}
			}

			if (status != STATUS_OK)
				{POBJDELETE(pTmpAVmsgServiceList)}	// Delete the new temporary IVR service list - in case of a failure use the original list

	  	}


		if(status != STATUS_OK)
			PTRACE2(eLevelError,"CConfPartyManager::OnServerDeleteIVRService : status = ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		else
			PTRACE2( eLevelInfoNormal,"CConfPartyManager::OnServerDeleteIVRService - deleted = ", ivrServiceName );

	  	// Set CRequest status
	  	pRequest->SetStatus(status);

	  	// Confirm
	  	pRequest->SetConfirmObject(pIVRServiceDel);
	}
	else
	{
		pRequest->SetConfirmObject(new CDummyEntry());
		FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerDeleteIVRService: No permission to delete ivr service");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}
  	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
// Check if the AV service is used by conference, reservation, MR or EQ
// output : STATUS_OK - the service isn't used
ESTATUS CConfPartyManager::IsAVServiceUsedBySomeConf(const char* ivrServiceName)
{
	ESTATUS esResult = STATUS_OK;
	int CommPos = 0;

	// checks on-going confs
	CCommConf* TempCommConf=::GetpConfDB()->GetFirstCommConf(CommPos);
	while(TempCommConf !=NULL)
	{
		  CAvMsgStruct*	pConfAVMsgStruct = TempCommConf->GetpAvMsgStruct();
		  if(pConfAVMsgStruct != NULL)
		  {
			  if(pConfAVMsgStruct->GetAvMsgServiceName()!=NULL)
			  {
				  int iResult = strncmp(pConfAVMsgStruct->GetAvMsgServiceName(),ivrServiceName,AV_SERVICE_NAME);
				  if(iResult == 0)
				  {
						esResult = STATUS_ILLEGAL_WHILE_CONFERENCES_EXISTS;
						break;
				  }
			  }
		  }
		TempCommConf=::GetpConfDB()->GetNextCommConf(CommPos);
	}

 	// checks Meeting rooms and EQs
	if(esResult == STATUS_OK)
	{
		CCommResDB* pMRDataBase = ::GetpMeetingRoomDB();
		CCommRes*	pCommRes = NULL;
		CAvMsgStruct* pAvMsgStruct = NULL;

		const CCommResDB::ReservArray& tempReservArray = pMRDataBase->GetReservArray();
		CCommResDB::ReservArray::const_iterator itr_end = tempReservArray.end();
		for (CCommResDB::ReservArray::const_iterator itr = tempReservArray.begin(); itr != itr_end; ++itr)
		{
			if ((*itr) != NULL)
			{
				WORD wConfId = (*itr)->GetConferenceId();
				pCommRes = pMRDataBase->GetCurrentRsrv(wConfId);
				if(pCommRes != NULL)
				{
					DWORD profileId = 0xFFFFFFFF;
					if (pCommRes->IsConfFromProfile(profileId) == NO )
					{
						CAvMsgStruct* pAvMsgStruct = pCommRes->GetpAvMsgStruct();
						if(pAvMsgStruct != NULL)
						{
							int iResult = strncmp(pAvMsgStruct->GetAvMsgServiceName(),ivrServiceName,AV_SERVICE_NAME);
							if(iResult == 0)
							{
								if(pCommRes->GetEntryQ())
									esResult = STATUS_ILLEGAL_WHILE_ENTRY_QUEUE_EXISTS;
								else
									esResult = STATUS_ILLEGAL_WHILE_MEETING_ROOM_EXISTS;
								POBJDELETE(pCommRes);
								break;
							}
						}
					}
					POBJDELETE(pCommRes);
				}
			}
		}
	}

	// checks Profiles
	if(esResult == STATUS_OK)
	{
		CCommResDB* pProfDataBase = ::GetpProfilesDB();
		CCommRes*	pCommRes = NULL;
		CAvMsgStruct* pAvMsgStruct = NULL;

		CCommResDB::ReservArray tempReservArray = pProfDataBase->GetReservArray();
		for (CCommResDB::ReservArray::iterator itr=tempReservArray.begin() ; itr != tempReservArray.end() ; ++itr)
		{
			if ((*itr) != NULL)
			{
				WORD wConfId = (*itr)->GetConferenceId();
				pCommRes = pProfDataBase->GetCurrentRsrv(wConfId);
				if(pCommRes != NULL)
				{
					CAvMsgStruct* pAvMsgStruct = pCommRes->GetpAvMsgStruct();
					if(pAvMsgStruct != NULL)
					{
						int iResult = strncmp(pAvMsgStruct->GetAvMsgServiceName(),ivrServiceName,AV_SERVICE_NAME);
						if(iResult == 0)
						{
							esResult = STATUS_IVR_ILLEGAL_WHILE_PROFILE_EXISTS;
							POBJDELETE(pCommRes);
							break;
						}
					}
					POBJDELETE(pCommRes);
				}
			}
		}
	}


	return esResult;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerAddIVRLanguage(CRequest* pRequest)
{
	PTRACE(eLevelError,"CConfPartyManager::OnServerAddIVRLanguage " );

	if (pRequest->GetAuthorization() == SUPER)
	{
		TRACEINTO << " GetAuthorization == SUPER ";

		int status=STATUS_OK;
		const WORD conId = pRequest->GetConnectId();

	  	// Check user's permission
	///	WORD perm = ::Permission(conId);
	///	if(perm==ORDINARY)
	///		status=STATUS_NO_PERMISSION;

		// Create instance of CIVRLanguageAdd
		CIVRLanguageAdd* pIVRLanguageAddTemp = (CIVRLanguageAdd*)pRequest->GetRequestObject();
		CIVRLanguageAdd* pIVRLanguageAdd = new CIVRLanguageAdd;
		*pIVRLanguageAdd = *pIVRLanguageAddTemp;

		// Get IVR language name for addition
	  	const char* ivrLanguageName = pIVRLanguageAdd->GetIVRLanguageName();

		if (status == STATUS_OK)
		{
			TRACEINTO << " status == OK ";

			// update name: change ' ' to '_' if exists
			char lang_name[LANGUAGE_NAME_LEN];
			memset( lang_name, '\0', LANGUAGE_NAME_LEN );
			strncpy(lang_name, ivrLanguageName, sizeof(lang_name) - 1);
			lang_name[sizeof(lang_name) - 1] = '\0';
			for (int i = 0; i < LANGUAGE_NAME_LEN; i++)
				if (lang_name[i] == ' ')
					lang_name[i] = '_';

			// set the updated language name
			pIVRLanguageAdd->SetIVRLanguageName( lang_name );

			// create folders
			CIVRService* pIVRService = new CIVRService;
			status = pIVRService->NewIVRLanguage(lang_name);
			if (status !=  STATUS_OK)
			{
				TRACEINTO << " NewIVRLanguage: status != OK, status=" << (DWORD)status;
			}
			POBJDELETE(pIVRService);
		}
		else
		{
			TRACEINTO << " status != OK";
		}


		//Trace
	///	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerAddIVRLanguage : Add New IVR Language completed, "
	///		, GetStatusAsString(status));

		// Confirm
	  	pRequest->SetConfirmObject(pIVRLanguageAdd);
	  	pRequest->SetStatus(status);
	}
	else
	{
		pRequest->SetConfirmObject(new CDummyEntry());
		FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerAddIVRLanguage: No permission to add ivr language");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}
  	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnIvrAddMusicSourceReq(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnIvrAddMusicSourceReq ");
	// this request came from the Cards process (upon its startup)

	if (m_pIVRManager)
		m_pIVRManager->AddAllMusicSources();
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetDefaultIVRService(CRequest* pRequest)
{
	PTRACE( eLevelInfoNormal,"CConfPartyManager::OnServerSetDefaultIVRService " );

	SetDefaultIVRService( pRequest, 0 );	// 0:IVR Def. Service  1:EQ Def. Service

  	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetDefaultEQService(CRequest* pRequest)
{
	PTRACE( eLevelInfoNormal,"CConfPartyManager::OnServerSetDefaultEQService " );

	SetDefaultIVRService( pRequest, 1 );	// 0:IVR Def. Service  1:EQ Def. Service

  	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::SetDefaultIVRService(CRequest* pRequest, WORD isEQSettings )
{
	if (pRequest->GetAuthorization() == SUPER)
	{
	  	int status=STATUS_OK;
		const WORD conId = pRequest->GetConnectId();

	  	// Check user's permission
	///	WORD perm = ::Permission(conId);
	///	if(perm==ORDINARY)
	///		status=STATUS_NO_PERMISSION;

	  	// Create instance of CIVRServiceSetDefault
		CIVRServiceSetDefault* pIVRServiceSetDefaultTemp = (CIVRServiceSetDefault*)pRequest->GetRequestObject();

		CIVRServiceSetDefault* pIVRServiceSetDefault = new CIVRServiceSetDefault;
	  	if (status == STATUS_OK)
	  	{
			*pIVRServiceSetDefault = *pIVRServiceSetDefaultTemp;
	  	}

  		const char* ivrServiceName = NULL;
	  	if (status == STATUS_OK)
	  	{
	  		// Get IVR service name
	  		ivrServiceName = pIVRServiceSetDefault->GetIVRServiceName();
	   		if (NULL == ivrServiceName)
	   			status = STATUS_IVR_SERVICE_NAME_DOES_NOT_EXISTS;
	  	}

   		// Get service list
   		CAVmsgServiceList* pAVmsgServiceList = NULL;
	  	if (status == STATUS_OK)
	  	{
	   		pAVmsgServiceList = ::GetpAVmsgServList();
	  		if (NULL == pAVmsgServiceList)	// should not happened
	  			status = STATUS_FAIL;
	  	}

//	  	if (status == STATUS_OK)
//	  		status = pAVmsgServiceList->SetDefaultIVRName( ivrServiceName, isEQSettings );	// 0:IVR Service   1:EQ Service

		if (status == STATUS_OK)	// all is legal, we can add the IVR to the list
		{
			// Create a new temporary IVR service list identical to the global list
			CAVmsgServiceList* pTmpAVmsgServiceList = new CAVmsgServiceList;
			*pTmpAVmsgServiceList = *pAVmsgServiceList;

			// Update the new service in the temporary list
	  		status = pTmpAVmsgServiceList->SetDefaultIVRName( ivrServiceName, isEQSettings );	// 0:IVR Service   1:EQ Service

			if (status == STATUS_OK)	// the IVR service list was updated
			{
				// Update IVR service list in disk after setting the default service
				status = pTmpAVmsgServiceList->SecureSaveIvrListToFile();

				if (status == STATUS_OK)
				{
					// Set the global IVR service list to point on the updated temporary list
					::SetpAVmsgServList(pTmpAVmsgServiceList);
					POBJDELETE(pAVmsgServiceList)	// Delete the original IVR service list
				}
			}

			if (status != STATUS_OK)
				{POBJDELETE(pTmpAVmsgServiceList)}	// Delete the new temporary IVR service list - in case of a failure use the original list

		}

	  	// Set CRequest status
	  	pRequest->SetStatus(status);

	  	//Confirm
	  	pRequest->SetConfirmObject(pIVRServiceSetDefault);
	}
	else
	{
		pRequest->SetConfirmObject(new CDummyEntry());
		FPTRACE(eLevelError,"CConfPartyManager::OnServerSetDefaultIVRService: No permission to set default ivr service");
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}
  	return STATUS_OK;
}





////////////////////////////////////////////////////////////////////////////////////////
//int CCardMngr::CheckSlidesValidity(const CAVmsgService* pReqAVmsgServ, int *needSlide)
//{
//	(*needSlide) = 0;
//
//	if (!pReqAVmsgServ)
//		return STATUS_ILLEGAL;
//
//	CIVRService* pIVRService = NULL;
//
//	WORD bIVRServiceFlag = pReqAVmsgServ->GetIVRServiceFlag();	// get IVR yes / no
//	if(bIVRServiceFlag) {	// IVR service
//		pIVRService = (CIVRService*)pReqAVmsgServ->GetIVRService();
//		if (!pIVRService)
//			return STATUS_ILLEGAL;
//
//		CIVRVideoFeature* pIVRVideoFeature = (CIVRVideoFeature*)pIVRService->GetVideoFeature();
//		if(!pIVRVideoFeature)	// video feature does not exist
//			return STATUS_OK;
//
//		if(!pIVRVideoFeature->GetEnableDisable()) // video feature disabled
//			return STATUS_OK;
//
//		if(!strcmp(pReqAVmsgServ->GetVideoFileName(), "")) // video slide name is empty
//			return STATUS_OK;
//	}
//
//
//	// checking if there are more than 4 slides in the services
//	int i=0;
//	MSG_FILE_PARAM_S *msg_table = new MSG_FILE_PARAM_S[4];
//	for(int k=0; k<4; k++) {
//		msg_table[k].file_name[0] = 0;
//		msg_table[k].dummy = 0;
//	}
//
//	CAVmsgServiceList*  pAVmsgServiceList = ::GetpAVmsgServList();
//	CAVmsgService* pAVmsgService = (CAVmsgService*)pReqAVmsgServ;
//	while (pAVmsgService)	// AV service or IVR service with new slide
//	{
//		if(pAVmsgService->GetIVRServiceFlag()) {	//if IVR Service, check for Video Feature
//			pIVRService = (CIVRService*)pAVmsgService->GetIVRService();
//			if (pIVRService) {
//				CIVRVideoFeature* pIVRVideoFeature = (CIVRVideoFeature*)pIVRService->GetVideoFeature();
//				//If video disabled, pass to next service
//				if(!pIVRVideoFeature || !pIVRVideoFeature->GetEnableDisable()) {
//					if(pAVmsgService == pReqAVmsgServ)
//						pAVmsgService = pAVmsgServiceList->GetFirstService();
//					else
//						pAVmsgService = pAVmsgServiceList->GetNextService();
//
//					//make sure you don't check the updated Service as well
//					if( pAVmsgService && !strncmp(pAVmsgService->GetName(), pReqAVmsgServ->GetName(), AV_MSG_SERVICE_NAME_LEN))
//						pAVmsgService = pAVmsgServiceList->GetNextService();
//					continue;
//				}
//			}
//		}
//
//		if(0 != strncmp("", pAVmsgService->GetVideoFileName(), 16))	// not empty string
//		{
//			//check if slide already appears in table.
//			for(int k=0; k<4; k++){
//				if(!strncmp(msg_table[k].file_name, pAVmsgService->GetVideoFileName(), 16))
//					break;
//			}
//
//			if(k==4){ // if doesn't already appear, add it to the table.
//				if(i<4){
//					strncpy(msg_table[i].file_name, pAVmsgService->GetVideoFileName(), 16);
//					msg_table[i].dummy = 0;
//					i++;
//				}
//				else{	// more than 4 slides
//					delete [] msg_table;
//					return STATUS_MAX_4_DIFFERENT_SLIDES_IN_AV_AND_IVR_SERVICES;
//				}
//			}
//		} // slide exists
//
//		// get the next service to check
//		if(pAVmsgService == pReqAVmsgServ) // will occur only in the first loop
//			pAVmsgService = pAVmsgServiceList->GetFirstService();
//		else
//			pAVmsgService = pAVmsgServiceList->GetNextService();
//
//		//make sure you don't check the updated Service as well
//		if( pAVmsgService && !strncmp(pAVmsgService->GetName(), pReqAVmsgServ->GetName(), AV_MSG_SERVICE_NAME_LEN))
//			pAVmsgService = pAVmsgServiceList->GetNextService();
//
//	}//end while
//
//	delete [] msg_table;
//
//	// if we are here then we have a legal slides number
//	// at this point, the slide is needed.
//	CMessageEntryList* pMsgEntryList = ::GetpMessageEntryList();
//	WORD ind = pMsgEntryList->GetCardMsgServiceIndex( pReqAVmsgServ->GetVideoFileName(), VIDEO_SLIDE );
//	if (W_NOT_FIND == ind)	// the slide doesn't exist in the DB
//		return STATUS_RESET_MCU_INORDER_SETTINGS_WILL_TAKE_EFFECT;
//
//	// The slide is needed, and it exists in the DB
//	(*needSlide) = 1;
//	return STATUS_OK;
//}
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////
//int CCardMngr::UpdateSlideInList(const CAVmsgService* pReqAVmsgServ)
//{
//	CMessageEntryList* pMsgEntryList = ::GetpMessageEntryList();
//	WORD ind = pMsgEntryList->GetCardMsgServiceIndex( pReqAVmsgServ->GetVideoFileName(), VIDEO_SLIDE );
//	if (W_NOT_FIND == ind)
//		return STATUS_RESET_MCU_INORDER_SETTINGS_WILL_TAKE_EFFECT;
//
//	const char* serviceName = pReqAVmsgServ->GetName();
//	WORD status = pMsgEntryList->CopyVideoEntry(ind, serviceName, VIDEO_SLIDE);
//	if (status != STATUS_OK) {
//		char str[16];
//		sprintf( str, "%d", status );
//		PTRACE2(eLevelError,"CCardMngr::UpdateSlideInList - status = ", str );
//	}
//
//	return status;
//}
//
//
//
//
//


//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerConfEntryPW(CRequest *pRequest)
{
  	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerConfEntryPW");
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
          FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerConfEntryPW: No permission to OnServerConfEntryPW for administrator readony");
          pRequest->SetConfirmObject(new CDummyEntry());
          pRequest->SetStatus(STATUS_NO_PERMISSION);
          return STATUS_NO_PERMISSION;
     }

   	STATUS status = STATUS_OK;

   	CConfAction* pConfAction = new CConfAction;

   	*pConfAction = *(CConfAction*)pRequest->GetRequestObject();
   	pRequest->SetObjectFlag(STRING_FLAG);

   	const DWORD confId = pConfAction->GetConfID();
  	/*** VALIDITY of conference Id ***/
 	if (::GetpConfDB()->FindId(confId) == NOT_FIND)
 	{
    	PTRACE(eLevelError,"ConfPartyManager::OnServerConfEntryPW - Can not find conference id");
    	PASSERT(1);
		status = STATUS_CONF_NOT_EXISTS;
 	}

//   	ALLOCBUFFER(entryPassword, CONFERENCE_ENTRY_PASSWORD_LEN);
//   	strncpy(entryPassword, pConfAction->GetEntryPassword(), CONFERENCE_ENTRY_PASSWORD_LEN);

 	if (status == STATUS_OK)
  	{
		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
	  	if(pCurConf)
  		{
	  		DWORD entryPasswordMinLength;
	  		DWORD entryPasswordMaxLength;
			CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("NUMERIC_CONF_PASS_MIN_LEN", entryPasswordMinLength);
			CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("NUMERIC_CONF_PASS_MAX_LEN", entryPasswordMaxLength);
			status = pCurConf->CheckPasswordValidity(pConfAction->GetEntryPassword(),entryPasswordMinLength, entryPasswordMaxLength);

			if (status == STATUS_OK) {
				DWORD dwMaxRepeatedDigits;
				CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(
						"MAX_CONF_PASSWORD_REPEATED_DIGITS",
						dwMaxRepeatedDigits);
				status = pCurConf->CheckPasswordRepeatedDigitsValidity(
						pConfAction->GetEntryPassword(), dwMaxRepeatedDigits);
				if (status != STATUS_OK)
					FPTRACE(eLevelInfoNormal,"CCommRes::OnServerConfEntryPW - Maximum number of permitted repeated characters in conf. password has been exceeded");
				else
	  	 		pCurConf->SetEntryPassword(pConfAction->GetEntryPassword());
  		}
  	}
}

    if (status == STATUS_OK)
    {
		  CStructTm curTime;
		  PASSERT(SystemGetTime(curTime));

		  CCdrLogApi cdrApi;
		  CCdrEvent cdrEvent;
		  CConfStartCont4* confStartCont4 = new CConfStartCont4;
		  confStartCont4->SetUser_password(pConfAction->GetEntryPassword());


		  cdrEvent.SetCdrEventType(CONFERENCE_START_CONTINUE_4);
		  cdrEvent.SetTimeStamp(curTime);
		  cdrEvent.SetConfStartCont4(confStartCont4);
		  POBJDELETE(confStartCont4);

		  cdrApi.ConferenceEvent(confId, cdrEvent);
     }

 	//Trace
  	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerConfEntryPW, "
		  , CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

    // confirm
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);

  	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleTerminalFeccToken(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(3 != numOfParams)
	{
		answer << "error: action must be specified: take | release\n";
		answer << "usage: Bin/McuCmd fecc_token ConfParty [Action] [Conf Name] [Party Name]\n";
		return STATUS_FAIL;
	}

	const string &action = command.GetToken(eCmdParam1);
	const string &confName = command.GetToken(eCmdParam2);
	const string &partyName = command.GetToken(eCmdParam3);

	WORD feccRequest = 0;
	if (action == "take")
	{
		answer << action << " " << confName << " " << partyName;
		feccRequest = 1;
	}
	else if (action == "release")
	{
		answer << action << " " << confName << " " << partyName;
		feccRequest = 2;
	}
	else if (action == "takeParty")
	{
		answer << action << " " << confName << " " << partyName;
		feccRequest = 3;
	}
	else if (action == "releaseParty")
	{
		answer << action << " " << confName << " " << partyName;
		feccRequest = 4;
	}
	else
	{
		answer << "error: wrong action, use only: take/tkeParty | release/releaseParty";
		return STATUS_FAIL;
	}

	// Find Conf in DB
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = NULL;
	 pRequestedConf = pCommConfDB->GetCurrentConf(confName.c_str());
	 if(!CPObject::IsValidPObjectPtr(pRequestedConf))
	 {
	 	answer << "error: Conf does not exist in DB" << " " <<  confName << " " << action;
		return STATUS_FAIL;
	 }
	 if(pCommConfDB->SearchPartyName(confName.c_str(),partyName.c_str()) != STATUS_OK)
	 {
	 	answer << "error: Party does not exist in DB" << " " <<  confName << " " << partyName << " "<< action;
		return STATUS_FAIL;
	 }
	 // Send Event to FECC Bridge
	    CConfApi* pConfApi = new CConfApi;
		pConfApi->CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));
		//pConfApi->SetLocalMbx(GetLocalQueue());
		CSegment* Command = new CSegment;
		*Command << partyName;
		 pConfApi->HandleTerminalEvent(feccRequest,Command);

		 POBJDELETE(Command);
		 POBJDELETE(pConfApi);

	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleTerminalSendAquireReleaseReq(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleTerminalSendAquireReleaseReq ");

	DWORD numOfParams = command.GetNumOfParams();
	if(3 != numOfParams)
	{
		answer << "error: action must be specified: SendAquireReleseReq\n";
		answer << "usage: Bin/McuCmd SendAquire_ReleaseReq ConfParty [Conf name] [Party name] [Aquire/Release]\n";

		return STATUS_FAIL;
	}

	const string &confName = command.GetToken(eCmdParam1);
	const string &partyName = command.GetToken(eCmdParam2);
	const string &Action = command.GetToken(eCmdParam3);

	// Find Conf in DB
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = NULL;

	pRequestedConf = pCommConfDB->GetCurrentConf(confName.c_str());
	if (!CPObject::IsValidPObjectPtr(pRequestedConf))
	{
		answer << "error: Conf does not exist in DB " <<  confName << " " << "SendDtmf ";
		return STATUS_FAIL;
	}
	if(pCommConfDB->SearchPartyName(confName.c_str(),partyName.c_str()) != STATUS_OK)
	{
		answer << "error: Party does not exist in DB" << " " <<  confName << " " << partyName << " "<< Action;
		return STATUS_FAIL;
	}

	// Send Event to conf
	CConfApi* pConfApi = new CConfApi;
	pConfApi->CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));

	WORD TokenReq = 0;
	if(Action == "Aquire")
		TokenReq = 5;
	else if(Action == "Release")
		TokenReq = 6;

	CSegment* Command = new CSegment;
	*Command << partyName;

	TRACEINTO<< "Yoella" << Action;
	pConfApi->HandleTerminalEvent(TokenReq,Command);

	POBJDELETE(Command);
	POBJDELETE(pConfApi);

	return STATUS_OK;

}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleRemoveAddMFACard(CTerminalCommand & command, std::ostream& answer)
{
	const string &action = command.GetToken(eCmdParam1);
	int cardAction;

	int actionNumber; // 1 - Add 2 - remove
	if (action == "remove")
	{
		cardAction = 1;
	}
	else if (action == "add")
	{
		cardAction = 2;
	}
	else
	{
		answer << action << " " << "wrong action";
		return STATUS_FAIL;
	}

    DWORD numOfParams = command.GetNumOfParams();
	if((cardAction == 1) && numOfParams < 3)
	{
		answer << "error: Conf Name must be specified\n";
		answer << "usage: Bin/McuCmd HandleRemoveMFACard ConfParty [Conf Name] [Party_id_i] ...... [Conf Name] [Party_id_n] \n";
		return STATUS_FAIL;
	}

	//const string &confName = command.GetToken(eCmdParam2);
	if(cardAction ==1)
	{
	  DWORD numberOfConfPartyPairs = (numOfParams - 1)/2;
	  DWORD confId[numberOfConfPartyPairs];
	  DWORD partyID[numberOfConfPartyPairs];
	  string partyIdStr;
	  string confIdStr;

	/*CCommConf*  pCommConf = ::GetpConfDB()->GetCurrentConf(confName.c_str());
	if(!CPObject::IsValidPObjectPtr(pCommConf))
	 {
	 	answer << "error: Conf does not exist in DB" << " " <<  confName;
		return STATUS_FAIL;
	 }*/

	 // DWORD confMonitorId = pCommConf->GetMonitorConfId();
	  CCommConf*  pCommConf = NULL;

	  for(WORD i=1;i<=numberOfConfPartyPairs;i++)
	  {
		 confIdStr = command.GetToken(eCommandParamsIndex(2*i+2));
     	 confId[i-1] = atoi(confIdStr.c_str());
		 partyIdStr = command.GetToken(eCommandParamsIndex(2*i+3));
     	 partyID[i-1] = atoi(partyIdStr.c_str());

     	 pCommConf = ::GetpConfDB()->GetCurrentConf(confId[i-1]);
     	 if(!CPObject::IsValidPObjectPtr(pCommConf))
	     {
	 	   answer << "error: Conf does not exist in DB" << " " <<  confId[i-1];
		  return STATUS_FAIL;
	     }
	     if(::GetpConfDB()->SearchPartyName(confId[i-1],partyID[i-1]) != STATUS_OK)
	     {
	 	    answer << "error: Party does not exist in DB" << " " <<  pCommConf->GetName() << " " << partyID;
		    return STATUS_FAIL;
	     }
	  }
	  CSegment* pDataSeg = new CSegment;
	  *pDataSeg << numberOfConfPartyPairs;
	  for(WORD j=0; j<numberOfConfPartyPairs;j++)
	  {
		*pDataSeg << confId[j] << partyID[j];

	  }
	  CTaskApi::StaticSendLocalMessage(pDataSeg, HW_REMOVED_PARTY_LIST_IND);

	}
	else if(cardAction == 2)
	{

	  CTaskApi::StaticSendLocalMessage(NULL, HW_NEW_IND);
	}

 return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleTerminalLPRInd(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleTerminalLPRInd ");

	DWORD numOfParams = command.GetNumOfParams();
	if(3 != numOfParams)
	{
		answer << "error: action must be specified: SendLPRInd\n";
		answer << "usage: Bin/McuCmd SendLPRInd ConfParty [Conf name] [Party name] [LPR]\n";

		return STATUS_FAIL;
	}

	const string &confName = command.GetToken(eCmdParam1);
	const string &partyName = command.GetToken(eCmdParam2);
	const string &Action = command.GetToken(eCmdParam3);

	// Find Conf in DB
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = NULL;

	pRequestedConf = pCommConfDB->GetCurrentConf(confName.c_str());
	if (!CPObject::IsValidPObjectPtr(pRequestedConf))
	{
		answer << "error: Conf does not exist in DB " <<  confName << " " << "SendDtmf ";
		return STATUS_FAIL;
	}
	if(pCommConfDB->SearchPartyName(confName.c_str(),partyName.c_str()) != STATUS_OK)
	{
		answer << "error: Party does not exist in DB" << " " <<  confName << " " << partyName << " "<< Action;
		return STATUS_FAIL;
	}

	// Send Event to conf
	CConfApi* pConfApi = new CConfApi;
	pConfApi->CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));

	WORD TokenReq = 0;
	if(Action == "LPR")
		TokenReq = 7;


	CSegment* Command = new CSegment;
	*Command << partyName;

	pConfApi->HandleTerminalEvent(TokenReq,Command);

	POBJDELETE(Command);
	POBJDELETE(pConfApi);

	return STATUS_OK;

}
//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleBlockConfIndication(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
		if(1 != numOfParams)
		{
			answer << "error: action must be specified: block | release\n";
			answer << "usage: Bin/McuCmd block_conf ConfParty [Action]\n";
			return STATUS_FAIL;
		}

		const string &action = command.GetToken(eCmdParam1);
		BYTE blockRequest = 0;
		if (action == "block")
		{
			answer << action;
			blockRequest = 1;
		}
		else if (action == "release")
		{
			answer << action;
			blockRequest = 0;
		}
		else
		{
			answer << "error: wrong action, use only: block/release";
			return STATUS_FAIL;
		}

		CSegment* pSeg = new CSegment;
		*pSeg << blockRequest;
		DispatchEvent(CONF_BLOCK_IND,pSeg);

	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleTerminalPCMReq(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(numOfParams < 1)
	{
		answer << "error: not enough parameters \n";
		answer << "usage: Bin/McuCmd pcm_debug [params]\n";
		return STATUS_FAIL;
	}

//	const string &confName = command.GetToken(eCmdParam1);
//	const string &partyName = command.GetToken(eCmdParam2);
	const string &action = command.GetToken(eCmdParam1);

	if (action == "on" || action == "ON" || action == "On" || action == "1")
		tmp_pcm_debug_flag = 1;
	else
		tmp_pcm_debug_flag = 0;

	//answer << "set pcm debug flag to: " << (tmp_pcm_debug_flag? "1" : "0");
	answer << "pcm_debug - Flag not in use!!!";

	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::MovePartyToMROOM_OR_CONF(CSegment* pParam)
{
  DWORD dwSourceConfId = 0xFFFFFFFF;
  DWORD dwTargetConfId = 0xFFFFFFFF;
  DWORD dwPartyId      = 0xFFFFFFFF;

  STATUS status = STATUS_OK;

  DWORD tempTargetConfType;
  *pParam >> dwSourceConfId >> dwPartyId >> tempTargetConfType;
  ETargetConfType targetConfType = (ETargetConfType)tempTargetConfType;
  if ((targetConfType == eOnGoingConf) || (targetConfType == eMeetingRoom))
    *pParam >> dwTargetConfId;

  TRACEINTO << "CConfPartyManager::MovePartyToMROOM_OR_CONF - SourceConfId:" << dwSourceConfId << ", TargetConfId:" << dwTargetConfId << ", PartyId:" << dwPartyId;

  CCommConf* pCommConfSourceConf = ::GetpConfDB()->GetCurrentConf(dwSourceConfId);
  PASSERTMSG_AND_RETURN_VALUE(!pCommConfSourceConf, "Party-Move-Error: Failed, cannot find source conference", STATUS_FAIL);

  CConfParty* pConfParty = pCommConfSourceConf->GetCurrentParty(dwPartyId);
  PASSERTMSG_AND_RETURN_VALUE(!pConfParty, "Party-Move-Error: Failed, cannot find party", STATUS_FAIL);

  BYTE isPartyEncrypted = pConfParty->GetIsPartyCurrentlyEncrypted();
  BYTE isPartyDefinedInTargetConf;
  DWORD partyIDInDestConf;
  switch (targetConfType)
  {
    case eAdHoc:
    {
      PTRACE(eLevelInfoNormal,"CConfPartyManager::MovePartyToMROOM_OR_CONF - Move to AdHoc Conference");
      CCommRes* pTargetConf = new CCommRes();
      pTargetConf->DeSerialize(NATIVE, *pParam);
      mcTransportAddress partyAddress = pConfParty->GetIpAddress();
      isPartyDefinedInTargetConf = pTargetConf->IsPartyDefined(&partyAddress, pConfParty->GetH323PartyAlias(), partyIDInDestConf, pConfParty->GetNetInterfaceType());
      status = ::TestEncryMoveValidity(isPartyEncrypted, isPartyDefinedInTargetConf, pTargetConf->GetEncryptionType());
      if (STATUS_OK == status)
      {
        CLobbyApi* pLobbyApi = (CLobbyApi*)::GetpLobbyApi();
        pLobbyApi->SuspendIvrParty(targetConfType, dwTargetConfId, dwSourceConfId, dwPartyId, pTargetConf);
      }
      delete pTargetConf;
      break;
    }
    case eMeetingRoom:
    {
      PTRACE(eLevelInfoNormal,"CConfPartyManager::MovePartyToMROOM_OR_CONF - Move to MR Conference");

      CCommRes* pDestMR = ::GetpMeetingRoomDB()->GetCurrentRsrv(dwTargetConfId);
      if (!pDestMR)
      {
        PASSERTMSG(1, "CConfPartyManager::MovePartyToMROOM_OR_CONF - Party-Move-Error Failed, cannot find destination MR Conference");
        status = STATUS_FAIL;
        break;
      }

      if (pDestMR->GetIsGateway())
      {
        delete pDestMR;
        PTRACE(eLevelError,"CConfPartyManager::MovePartyToMROOM_OR_CONF - Party-Move-Error Can't move party to a GW session profile");
        status = STATUS_FAIL;
        break;
      }
      DWORD profileId = 0xFFFFFFFF;

	  if(pDestMR->IsConfFromProfile(profileId) == YES )
	  {
		  CCommRes* pProfile = ::GetpProfilesDB()->GetCurrentRsrv(profileId);
		  if (pProfile)
			  pDestMR->SetEncryptionParameters(pProfile->GetIsEncryption(),pProfile->GetEncryptionType());
		  POBJDELETE(pProfile);
	  }
      mcTransportAddress partyAddress = pConfParty->GetIpAddress();
	  isPartyDefinedInTargetConf = pDestMR->IsPartyDefined(&partyAddress, pConfParty->GetH323PartyAlias(), partyIDInDestConf, pConfParty->GetNetInterfaceType());
	  status = ::TestEncryMoveValidity(isPartyEncrypted, isPartyDefinedInTargetConf, pDestMR->GetEncryptionType());

      //MeetingRoom - We don`t have to TestDestConfMoveValidity since it can be only the first party to awake the MR
      //Others parties that are defined in the MR will take this test on DialOut ADD
      POBJDELETE(pDestMR); //No Need in The MR Reservation in this point, Lobby need Resrv only n Ad-Hoc case

      if (STATUS_OK != status)
      {
        TRACEINTO << "Party-Move-Error Party-Move-Error TestEncryMoveValidity return error";
        break;
      }

      CLobbyApi* pLobbyApi = (CLobbyApi*)::GetpLobbyApi();
      pLobbyApi->SuspendIvrParty(targetConfType, dwTargetConfId, dwSourceConfId, dwPartyId, pDestMR);
      break;
    }

    case eOnGoingConf:
    {
      PTRACE(eLevelInfoNormal, "CConfPartyManager::MovePartyToMROOM_OR_CONF - Start move party to OnGoing conference");
      CCommConf* pCommConfTargetConf = ::GetpConfDB()->GetCurrentConf(dwTargetConfId);
      ISDEBUGMODE_SET_VAL("MOVE", 1, pCommConfTargetConf, 0)
      if (!pCommConfTargetConf)
      {
        TRACEINTO << "Party-Move-Error: Failed, cannot find target conference in ConfDB";
        status = STATUS_FAIL;
        break;
      }
	  isPartyDefinedInTargetConf = STATUS_PARTY_DOES_NOT_EXIST != pCommConfTargetConf->SearchPartyByIPOrAlias(pConfParty->GetIpAddress(), pConfParty->GetH323PartyAlias(), pConfParty->GetH323PartyAliasType());
      // Dmitry: No need to check move validity here. The Conference itself will validate party movement and will play IVR if movement is not valid
      status = ::TestEncryMoveValidity(isPartyEncrypted, isPartyDefinedInTargetConf, pCommConfTargetConf->GetEncryptionType());
      //{VNGR-19012,VNGR-20938}
      //Allow to move party despite that the destination conference is full - the participant must hear IVR about that fact before disconnect
      if (status == STATUS_OK)
        status = ::TestMoveValidity(dwTargetConfId, dwSourceConfId, pConfParty, false);
  	  ISDEBUGMODE_SET_STATUS("MOVE", 2 ,1001)	// simulate validation error
  	  ISDEBUGMODE_SET_STATUS("MOVE", 9 ,STATUS_MAX_VIDEO_PARTIES_OF_DESTINATION_CONF_EXCEEDED)	// simulate validation error

      DWORD dwTipSlaveId[3] = { 0xFFFFFFFF,  0xFFFFFFFF, 0xFFFFFFFF };
      BYTE tipPartiesExist = FALSE;
      if (pConfParty->IsTIPMasterParty())
      {
        tipPartiesExist = TRUE;
        const char* confPartyName = pConfParty->GetName();
        PTRACE2(eLevelInfoNormal, "ConfPartyManager::MovePartyToMROOM_OR_CONF - TIP party master party, PartyName:", confPartyName);
        CConfParty* pTmpConfParty = pCommConfSourceConf->GetFirstParty();
        int i=0;
        while (pTmpConfParty)
        {
          if (pTmpConfParty != pConfParty && ( pTmpConfParty->GetRoomId() == pConfParty->GetRoomId() ))
          {
            PTRACE2(eLevelInfoNormal, "ConfPartyManager::MovePartyToMROOM_OR_CONF - TIP slave party, PartyName:", pTmpConfParty->GetName());
            dwTipSlaveId[i] = pTmpConfParty->GetPartyId();
            i++;
          }
          pTmpConfParty = pCommConfSourceConf->GetNextParty();
        }
      }
      switch (status)
      {
        case STATUS_OK:
        case STATUS_MAX_PARTIES_OF_DESTINATION_CONF_EXCEEDED:
        case STATUS_MAX_VIDEO_PARTIES_OF_DESTINATION_CONF_EXCEEDED:
        case STATUS_NUMBER_OF_PARTICIPANTS_EXCEEDS_THE_DEFINED_MAXIMUM_FROM_PROFILE:
        {
          CConfApi confApi;
          confApi.CreateOnlyApi(*(pCommConfSourceConf->GetRcvMbx()));
          confApi.MoveParty(dwPartyId, dwTargetConfId, eMoveIntoIvr);
          if( tipPartiesExist )
          {
        	  for( int i=0; i<3; i++ )
        	  {
        		  if( dwTipSlaveId[i] != 0xFFFFFFFF )
        	          confApi.MoveParty(dwTipSlaveId[i], dwTargetConfId, eMoveIntoIvr);
        	  }
          }
          confApi.DestroyOnlyApi();
        }
        break;
      }
      // Update move info
      CMoveConfDetails sourceConfMoveDetails(pCommConfSourceConf);
      CMoveConfDetails targetConfMoveDetails(pCommConfTargetConf);

      CMoveInfo* partyMoveInfo = pConfParty->GetMoveInfo();
      partyMoveInfo->UpdateMove(sourceConfMoveDetails, targetConfMoveDetails);
      // Update CDR (Corrects also VNGFE-1740)
      SendMoveCDREvents(pCommConfSourceConf, pCommConfTargetConf, dwPartyId, eMoveIntoIvr);
      break;
    }

    default:
    {
      PTRACE(eLevelError, "CConfPartyManager::MovePartyToMROOM_OR_CONF - Party-Move-Error Failed, no conference type found to move party to");
      status = STATUS_FAIL;
      break;
    }
  }

  if (STATUS_OK != status && pCommConfSourceConf)
  {
    //Send Drop party to source conference
    CConfApi confApi;
    confApi.CreateOnlyApi(*(pCommConfSourceConf->GetRcvMbx()));
    confApi.DropParty(pConfParty->GetName());
    confApi.DestroyOnlyApi();
    PTRACE(eLevelError,"CConfPartyManager::MovePartyToMROOM_OR_CONF - Party-Move-Error Failed, move validation failed, cannot move party, disconnect party");
    //PASSERT(1); Yoella with Moti we have decided to remove the ASSERT since there is enough information in the trace. (The user expected InfoMsg - Not assert)
    return STATUS_FAIL;
  }
  return status;
}
////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnStartupReadMRDB(CSegment* pSeg)
{
  PTRACE(eLevelInfoNormal, "CConfPartyManager::OnStartupReadMRDB begin");

   ON(m_isStartupReadMRDBReqRecieved);
   eTaskState taskState = GetTaskState();
   const char *taskStateName = GetTaskStateName(taskState);
   TRACEINTO << "\nCConfPartyManager::OnStartupReadMRDB : " << taskStateName;
   RemoveActiveAlarmByErrorCode(AA_NO_READ_MR_DB_REQ_RECIEVED_FROM_RSRC);

   verifyExitFromStartUpAndPerformAfterStartUpActions();

  return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::SendMsgToRsrvMngrRsrcProcess(CSegment* segment, OPCODE opcode)
{
	TRACEINTO << "Opcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(opcode);

	CRsrvManagerApi rsrvManagerApi;
	rsrvManagerApi.SendMsg(segment, opcode);
}


////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::SendAsyncMsgToRsrcProcess(CSegment* segment, OPCODE opcode)
{
	TRACEINTO << "CConfPartyManager::SendAsynchMsgToRA - Sending asynchronous message to RA, opcode:" << opcode;
	CManagerApi resourceManagerApi(eProcessResource);
	resourceManagerApi.SendMsg(segment,opcode);
}

////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::SendNumericIdListToRsrcProcess()
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::SendNumericIdListToRsrcProcess sending Numeric Id List to RA");

    CCommResDB* pMRDB = GetpMeetingRoomDB();
    m_pMrAndProfileListDuringStartup = new MR_AND_PROFILE_IND_LISTS;

	//Fill the list struct with the relevant data
	pMRDB->FillMRNumericIdList(m_pMrAndProfileListDuringStartup->mr_list);

	CSegment * pRASeg = new CSegment;


	if (false)
	  {// we will never send the meeting room DB in this function, it will be send later...

	    //write only the array of the data
	    pRASeg->Put( (BYTE*) &(m_pMrAndProfileListDuringStartup->mr_list.list_size),sizeof(DWORD));


	    if (m_pMrAndProfileListDuringStartup->mr_list.list_size)
	{
		PTRACE(eLevelInfoNormal, "CConfPartyManager::SendNumericIdListToRsrcProcess MR list is not empty");
		pRASeg->Put((BYTE *) (m_pMrAndProfileListDuringStartup->mr_list.monitor_numeric_list),
			    sizeof(MR_MONITOR_NUMERIC_ID_S)* m_pMrAndProfileListDuringStartup->mr_list.list_size);
	      }

	  }
	else
	  {
	    pRASeg->Put( (BYTE*) &(m_pMrAndProfileListDuringStartup->mr_list.list_size),sizeof(DWORD));
	    //DWORD stub_size = 0;
	       //write only the array of the data
	    //	    pRASeg->Put( (BYTE*) &stub_size,sizeof(DWORD));
	}

	//Fill the profiles list
	WORD numProfiles = ::GetpProfilesDB()->GetResNumber();
	WORD numProfilesAdded = 0;
        m_pMrAndProfileListDuringStartup->profile_list.list_size = numProfiles;
	m_pMrAndProfileListDuringStartup->profile_list.profile_list = new PROFILE_IND_S[numProfiles];

	// vngr-15549 not initiating maxVideoPartyType caused core dump (in resource print)
	for(WORD prof_index=0;prof_index<numProfiles;prof_index++){
	  m_pMrAndProfileListDuringStartup->profile_list.profile_list[prof_index].profile_Id = (DWORD)(-1);
	  m_pMrAndProfileListDuringStartup->profile_list.profile_list[prof_index].maxVideoPartyType = eVideo_party_type_dummy;
	}

	const CCommResDB::ReservArray& tempProfilesArray = ::GetpProfilesDB()->GetReservArray();
	CResRsrcCalculator rsrcCalc;

	CLargeString profilesDataStr;
	CCommResDB::ReservArray::const_iterator itr_end = tempProfilesArray.end();
	for (CCommResDB::ReservArray::const_iterator itr = tempProfilesArray.begin() ; itr != itr_end ; ++itr)
	{
		if ((*itr) != NULL)
		{
			if(numProfilesAdded>=numProfiles)
			{
				PASSERT(1); //there's an inconsistency between GetResNumber() and actula number of profiles
				break;
			}
			// get reservation from disc
			CCommRes* pCommRes = ::GetpProfilesDB()->GetCurrentRsrv((*itr)->GetConferenceId());
			if(CPObject::IsValidPObjectPtr(pCommRes))
			{
			  m_pMrAndProfileListDuringStartup->profile_list.profile_list[numProfilesAdded].profile_Id = (*itr)->GetConferenceId();
		      eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
			  eVideoPartyType vidtype = rsrcCalc.GetRsrcVideoType(systemCardsBasedMode, pCommRes);
			  m_pMrAndProfileListDuringStartup->profile_list.profile_list[numProfilesAdded].maxVideoPartyType = vidtype;
			  profilesDataStr	<< "    Profile Name: " << pCommRes->GetDisplayName()
			     		 		<< ": profile_Id = " << m_pMrAndProfileListDuringStartup->profile_list.profile_list[numProfilesAdded].profile_Id
			     		 		<< " maxVideoPartyType	= " << eVideoPartyTypeNames[vidtype]
			     		 		<<'\n';

			  numProfilesAdded++;
			  POBJDELETE(pCommRes);
			}else{
			  profilesDataStr	<<  "   conf_Id = " << (*itr)->GetConferenceId() << " not found in profile DB (disk) \n";
			}

		}
	}

	PTRACE2(eLevelInfoNormal,"CConfPartyManager::SendNumericIdListToRsrcProcess: Sending Profile weight to Resource process \n",profilesDataStr.GetString());
	//write only the array of the data
	pRASeg->Put( (BYTE*) &(m_pMrAndProfileListDuringStartup->profile_list.list_size),sizeof(DWORD));

	if (m_pMrAndProfileListDuringStartup->profile_list.list_size)
	{
		PTRACE(eLevelInfoNormal, "CConfPartyManager::SendNumericIdListToRsrcProcess Profile list is not empty");
		pRASeg->Put((BYTE *) (m_pMrAndProfileListDuringStartup->profile_list.profile_list),sizeof(PROFILE_IND_S)* m_pMrAndProfileListDuringStartup->profile_list.list_size);
	}

	//Send ASynch Msg to the RA process
	SendAsyncMsgToRsrcProcess(pRASeg,STARTUP_READ_MR_AND_PROFILE_DB_IND);

 	//delete the array of data
	// 	if(m_pMrAndProfileListDuringStartup->mr_list.monitor_numeric_list)
	//		delete [] m_pMrAndProfileListDuringStartup->mr_list.monitor_numeric_list ;
	//	if(m_pMrAndProfileListDuringStartup->profile_list.profile_list)
	//		delete [] m_pMrAndProfileListDuringStartup->profile_list.profile_list;
	//	delete m_pMrAndProfileListDuringStartup;
}

//////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnRsrcRemoveCardInd(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal,"CConfPartyManager::OnRsrcRemoveCardInd");

 	HW_REMOVED_PARTY_LIST_S* pParamData = new HW_REMOVED_PARTY_LIST_S;

    pParam->Get((BYTE*)( &(pParamData->list_size) ),sizeof(DWORD));
    DWORD listSize = pParamData->list_size;
    pParamData->conf_party_list = new CONF_PARTY_ID_S[listSize];
    pParam->Get( (BYTE*)( pParamData->conf_party_list ),sizeof(CONF_PARTY_ID_S)*listSize );


  	for(WORD j=0; j< pParamData->list_size;j++)
  	{

  	  CCommConf*  pCurConf  = ::GetpConfDB()->GetCurrentConf(pParamData->conf_party_list[j].monitor_conf_id);
  	  if (!CPObject::IsValidPObjectPtr(pCurConf))
      {
   	    CSmallString confIdStr;
        confIdStr << pParamData->conf_party_list[j].monitor_conf_id;
		PTRACE2(eLevelError,"CConfPartyManager::OnRsrcRemoveCardInd Conf is not found in DB, confId: !!!",confIdStr.GetString());
		continue;
      }
      CConfApi confApi;
      confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));

      if(pCurConf->GetContentMultiResolutionEnabled())
      {
    	  TRACEINTO << " Party in Multiple Resolutions Conf, Destroy Conference, Conf name: " << pCurConf->GetName() << " Party Monitor ID: " <<  pParamData->conf_party_list[j].monitor_party_id;
    	  confApi.Destroy();
      }
  	  else
  	  {
  		  CConfParty* pConfParty = pCurConf->GetCurrentParty(pParamData->conf_party_list[j].monitor_party_id);
  		  if(pConfParty)
  		  {
  			  confApi.DropParty(pConfParty->GetName(),1/*disconnect*/);
  		  }
  	  }
	  confApi.DestroyOnlyApi();
  	}

  	delete [] pParamData->conf_party_list;
  	delete pParamData;


  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnRsrcAddCardInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnRsrcAddCardInd");

	BOOL isSlave = CProcessBase::GetProcess()->GetIsFailoverSlaveMode();
	if (isSlave)
	{
	  PTRACE(eLevelInfoNormal,"CConfPartyManager::OnRsrcAddCardInd - RMX is in slave mode. nothing to do.");
	  return STATUS_OK;
	}

	WORD new_card_board_id = (WORD)-1;
	*pParam >> new_card_board_id;

	TRACEINTO << "AMOS_AC_DEBUG: CConfPartyManager::OnRsrcAddCardInd new_card_board_id = " << new_card_board_id;

	DWORD confId, partyId;
	// checks on-going confs
	int CommPos = 0;

	CCommConf* TempCommConf=::GetpConfDB()->GetFirstCommConf(CommPos);
	while(TempCommConf !=NULL)
	{
	 CConfApi confApi;
	 confApi.CreateOnlyApi(*(TempCommConf->GetRcvMbx()));
	 confApi.ReSendOpenConf(new_card_board_id);
	 confApi.DestroyOnlyApi();
	 TempCommConf=::GetpConfDB()->GetNextCommConf(CommPos);
	}

	return STATUS_OK;
}

///////////////////////////////////////////////////////
STATUS CConfPartyManager::OnRsrcCardTypeInd(CSegment* pParam)
{
	eProductType productType = CProcessBase::GetProcess()->GetProductType();
	DWORD card_type = 0;
	*pParam >> card_type;
	TRACEINTO << " card type = " << ::CardTypeToString(card_type);
	// check if product type is RMX1500Q
	if( eMpmx_20 == card_type &&
		eProductTypeRMX1500 == productType )
	{
		CResRsrcCalculator::SetRMX1500Q(TRUE);

		//blocking the HD options in the RMX 1500q
		if (FALSE == ::GetDongle1500qHDvalue())
		{
			TRACEINTO << " blocking the HD options in 1500Q";
			CResRsrcCalculator::SetHDenabled(FALSE);
		}
	}
	if( eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily() &&
		eProductTypeNinja != productType && // Change by Han - HD1080P60 support in Ninja
		eProductTypeGesher		!= productType &&		// Change by Hzsun - HD1080P30 support in Gesher
		eProductTypeEdgeAxis	!= productType &&	// Change by Hzsun - HD1080P30 support in Edge
		eProductTypeCallGeneratorSoftMCU != productType)
	{
		TRACEINTO << " blocking HD1080 in SoftMCU";
		CResRsrcCalculator::SetHD1080enabled(FALSE);
	}

	return STATUS_OK;
}
///////////////////////////////////////////////////////
STATUS CConfPartyManager::OnRsrcUpdateIvrCntlInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnRsrcUpdateIvrCntlInd");

	// checks on-going confs
	int CommPos = 0;
  	CCommConf* TempCommConf=::GetpConfDB()->GetFirstCommConf(CommPos);
  	while(TempCommConf != NULL)
  	{
		 CConfApi confApi;
	     confApi.CreateOnlyApi(*(TempCommConf->GetRcvMbx()));
	  	 confApi.SendCamIvrCntlInd();
	  	 confApi.DestroyOnlyApi();

  	 	TempCommConf=::GetpConfDB()->GetNextCommConf(CommPos);
  	}

	return STATUS_OK;
}
STATUS CConfPartyManager::OnRAMeetingRoomDBInd(CSegment* pSeg)
{

	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnRAMeetingRoomDBInd begin");
	DWORD listSize = 0;

	if (m_pMrAndProfileListDuringStartup)
	  {
	    //delete the array of data
	    if(m_pMrAndProfileListDuringStartup->mr_list.monitor_numeric_list)
	      delete [] m_pMrAndProfileListDuringStartup->mr_list.monitor_numeric_list ;
	    if(m_pMrAndProfileListDuringStartup->profile_list.profile_list)
	      delete [] m_pMrAndProfileListDuringStartup->profile_list.profile_list;
	    delete m_pMrAndProfileListDuringStartup;
	  }

	if (!CPObject::IsValidPObjectPtr(pSeg))
	{
		PTRACE(eLevelError,"CConfPartyManager::OnRAMeetingRoomDBInd Segment from RA is not valid!!!");
		PASSERT(1);
		return STATUS_FAIL;
	}

	*pSeg >>  listSize;

	if (listSize == 0){
		PTRACE(eLevelInfoNormal, "CConfPartyManager::OnRAMeetingRoomDBInd MR List is empty no indication from RA");
		return STATUS_OK;
	}

	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnRAMeetingRoomDBInd RA checked MR UnEmpty list");
	MR_IND_S * mr_list = new MR_IND_S[listSize];
	pSeg->Get((BYTE *) mr_list,sizeof(MR_IND_S)* listSize);

	DWORD mrMonitorID = 0;
	for( unsigned int i=0; i < listSize ; ++i)
	{
		mrMonitorID= (mr_list[i]).meeting_room_monitor_Id;
		if (mr_list[i].status != STATUS_OK)
		{
		  CCommResShort*  pMrShort = ::GetpMeetingRoomDB()->GetCurrentRsrvShort(mrMonitorID);
		  CLargeString description;
		  if (pMrShort)
		  {
			description << "RA did not confirm MR/EQ. Wrong parameters in MR/EQ " <<  pMrShort->GetName();
			CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,ILLEGAL_CONFERENCE_PARAMETERS,MAJOR_ERROR_LEVEL,description.GetString(), FALSE);
			TRACESTR (eLevelError) << "CConfPartyManager::OnRAMeetingRoomDBInd: " << description.GetString() ;
			delete pMrShort;
		  }
		  else
		  {
			description << "RA did not confirm MR/EQ. MR is not found in MR DB. MR Id: " <<  mrMonitorID;
			CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,ILLEGAL_CONFERENCE_PARAMETERS,MAJOR_ERROR_LEVEL,description.GetString(), FALSE);
			TRACESTR (eLevelError) << "CConfPartyManager::OnRAMeetingRoomDBInd RA did not confirm MR Id: " <<  mrMonitorID;
		  }
		 PASSERT(1);
		}
	}

	delete[] mr_list;
	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////
//RecordingLink List Functions
////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerAddRecordingLink(CRequest* pRequest)
{
	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerAddRecordingLink");
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerAddRecordingLink: No permission to OnServerAddRecordingLink for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
    }


	STATUS status = STATUS_OK;

	CRsrvRecordLinkPartyAdd* pRsrvRecLinkPartyAdd = new CRsrvRecordLinkPartyAdd;
	*pRsrvRecLinkPartyAdd = *(CRsrvRecordLinkPartyAdd*)pRequest->GetRequestObject() ;
	 CRsrvParty* pRsrvParty = pRsrvRecLinkPartyAdd->GetRsrvParty();


	CRecordingLinkDB* pRecordingLinkDB = ::GetRecordingLinkDB();

	if(pRecordingLinkDB->GetNumParties() >= MAX_RECORDING_LINKS_IN_LIST)
		status = STATUS_MAX_RECORDING_LINKS_REACHED;


//	pRecordingLinkDB->SetFolderPath(FILE_RECORDLINK_SRV_DB,MAX_RECORDING_LINKS_IN_LIST);

	// Does link name exists
	if(STATUS_OK == status)
	{
		if(pRecordingLinkDB->GetParty(pRsrvParty->GetName())!=NULL)
		{
			status = STATUS_PARTY_NAME_EXISTS;
		}
	}

	if (status == STATUS_OK)
	{
		DWORD nextPartyId = pRecordingLinkDB->NextPartyId();
	    if (pRsrvParty->GetPartyId() <= HALF_MAX_DWORD || pRsrvParty->GetPartyId() == 0xFFFFFFFF)
    	{
			 ((CRsrvParty*)pRsrvParty)->SetPartyId(nextPartyId);
		}
	}

	if(STATUS_OK == status)
		status = pRecordingLinkDB->TestPartyRsrvValidity(pRsrvParty);

	if(STATUS_OK == status)
		status = pRecordingLinkDB->Add(*pRsrvParty);

	if(STATUS_OK == status)
	{
		if (1 == pRecordingLinkDB->GetNumParties())
			pRecordingLinkDB->SetDefaultRecordingLinkName( pRsrvParty->GetName() );
			pRecordingLinkDB->IncreaseUpdateCounter();
	}

	if(STATUS_OK != status)
	{
		CSmallString sstr;
		sstr << " Status = " << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << " , Name - " << pRsrvParty->GetName();
		PTRACE2(eLevelError, "CConfPartyManager::OnServerAddRecordingLink status: ",sstr.GetString());
	}


    //responce to EMA
	pRequest->SetConfirmObject(pRsrvRecLinkPartyAdd);
	pRequest->SetStatus(status);

  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerUpdateRecordingLink(CRequest* pRequest)
{
	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateRecordingLink");
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
	{
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateRecordingLink: No permission to OnServerUpdateRecordingLink for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
	}


	STATUS status = STATUS_OK;

	CRsrvRecordLinkPartyAdd* pRsrvRecLinkPartyAdd = new CRsrvRecordLinkPartyAdd;
	*pRsrvRecLinkPartyAdd = *(CRsrvRecordLinkPartyAdd*)pRequest->GetRequestObject() ;
	 CRsrvParty* pRsrvParty = pRsrvRecLinkPartyAdd->GetRsrvParty();


	CRecordingLinkDB* pRecordingLinkDB = ::GetRecordingLinkDB();

	if(STATUS_OK == status)
		status = pRecordingLinkDB->TestPartyRsrvValidity(pRsrvParty);

	if(STATUS_OK == status)
	{
		status = pRecordingLinkDB->Update(*pRsrvParty);
		pRecordingLinkDB->IncreaseUpdateCounter();
	}

	if(STATUS_OK != status)
	{
		CSmallString sstr;
		sstr << " Status = " << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << " , Name - " << pRsrvParty->GetName();
		PTRACE2(eLevelError, "CConfPartyManager::OnServerUpdateRecordingLink status: ",sstr.GetString());
	}

    //responce to EMA
	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction();
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);

  return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerDeleteRecordingLink(CRequest* pRequest)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerDeleteRecordingLink");

	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
	{
        FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerDeleteRecordingLink: No permission to OnServerDeleteRecordingLink for administrator readony");
        pRequest->SetConfirmObject(new CDummyEntry());
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_NO_PERMISSION;
	}

	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;
	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject();
	pRequest->SetObjectFlag(STRING_FLAG); //?

	const DWORD partyID  = pRsrvPartyAction->GetPartyID();

	int status = STATUS_OK;
	int status_check = STATUS_OK;

	CRecordingLinkDB* pRecordingLinkDB = ::GetRecordingLinkDB();
	CRsrvParty* pRsrvParty = pRecordingLinkDB->GetPartyById(partyID);
	if(!pRsrvParty)
		status = STATUS_PARTY_DOES_NOT_EXIST;

	int	bSetDefaultLink = 0;	// if need to change default RL

	const char* delRecLinkName = NULL;

	if(STATUS_OK == status)
	{
		delRecLinkName = pRsrvParty->GetName();
		if (delRecLinkName==NULL)
			status = STATUS_ILLEGAL_PARTY_NAME;
		else {
			if (!strcmp(delRecLinkName,""))
				status = STATUS_ILLEGAL_PARTY_NAME;
			else	// there is a valid name
				if (0 == strncmp(pRecordingLinkDB->GetDefaultRecordingLinkName(), delRecLinkName, H243_NAME_LEN ))
				{
					// this is the default RL: set another one
					bSetDefaultLink = 1;
				}
		}
	}
	if(STATUS_OK == status)
	{
	  //We can delete a recording limk only if it's not part of a profile, MR or ongoing conf
	  BYTE isRecordingLinknUse = IsRecordingLinkInUse(pRsrvParty->GetName());
	  if(isRecordingLinknUse)
		 status = STATUS_CANNOT_DELETE_RECORDING_LINK_WHILE_ASSIGNED;
	}
	if(STATUS_OK == status)
		status = pRecordingLinkDB->Cancel(delRecLinkName);

	// update default if needed
	if(STATUS_OK == status)
	{
		pRecordingLinkDB->IncreaseUpdateCounter();
		if (0 == pRecordingLinkDB->GetNumParties()) {
			pRecordingLinkDB->SetDefaultRecordingLinkName("");
		}
		else {
			if (bSetDefaultLink) {	// this del RL was the default one
				CRsrvParty* firstRecLink = pRecordingLinkDB->GetFirstParty();
				if (firstRecLink)
					pRecordingLinkDB->SetDefaultRecordingLinkName(firstRecLink->GetName());
				else
					pRecordingLinkDB->SetDefaultRecordingLinkName( "" );
			}
		}
	}

	//responce to EMA
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);

	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerDeleteRecordingLink status: ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str() );
	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetDefaultRecordingLink(CRequest* pRequest)
{
	PTRACE(eLevelInfoNormal,"ConfPartyManager::OnServerSetDefaultRecordingLink");
	CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;
	*pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject();
	pRequest->SetObjectFlag(STRING_FLAG); //?

	const DWORD partyID  = pRsrvPartyAction->GetPartyID();

	int status = STATUS_OK;

	CRecordingLinkDB* pRecordingLinkDB = ::GetRecordingLinkDB();
	CRsrvParty* pRsrvParty = pRecordingLinkDB->GetPartyById(partyID);
	if(!pRsrvParty)
		status = STATUS_PARTY_DOES_NOT_EXIST;

	// update default if needed
	if(STATUS_OK == status)
	{
		pRecordingLinkDB->IncreaseUpdateCounter();
	    pRecordingLinkDB->SetDefaultRecordingLinkName(pRsrvParty->GetName());
	}

	//responce to EMA
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////////////
BYTE   CConfPartyManager::IsRecordingLinkInUse(const char* recordingLinkName)
{

	BYTE isRecordingLinkInUse = NO;
	int CommPos = 0;

	// checks on-going confs
	CCommConf* pTempCommConf=::GetpConfDB()->GetFirstCommConf(CommPos);
	while(pTempCommConf !=NULL)
	{
		const char* linkNameInConf = pTempCommConf->GetRecLinkName();
		if(linkNameInConf && pTempCommConf->GetEnableRecording())
		{
			if(!strncmp(linkNameInConf,recordingLinkName,H243_NAME_LEN))
			{
				isRecordingLinkInUse = YES;
				PTRACE(eLevelInfoNormal, "CConfPartyManager::IsRecordingLinkInUse, In On-going conference");
				break;
			}
		}

		pTempCommConf=::GetpConfDB()->GetNextCommConf(CommPos);
	}

 	// checks Meeting rooms (RecordingLink isn't valid in EQs)
	if(!isRecordingLinkInUse)
	{
		CCommResDB* pMRDataBase = ::GetpMeetingRoomDB();

		const CCommResDB::ReservArray& tempReservArray = pMRDataBase->GetReservArray();
		CCommResDB::ReservArray::const_iterator itr_end = tempReservArray.end();
		for (CCommResDB::ReservArray::const_iterator itr = tempReservArray.begin() ; itr != itr_end ; ++itr)
		{
			if ((*itr) != NULL)
			{
				WORD wConfId = (*itr)->GetConferenceId();
				CCommRes* pCommRes = pMRDataBase->GetCurrentRsrv(wConfId);
				if(pCommRes != NULL)
				{
					DWORD profileId = 0xFFFFFFFF;
					if (pCommRes->IsConfFromProfile(profileId) == NO &&  !pCommRes->GetEntryQ()) //We check only in MR, recording isn't valid in EQ
					{
						const char * linkNameInConf = pCommRes->GetRecLinkName();
						if(linkNameInConf && pCommRes->GetEnableRecording())
						{
							if(!strncmp(linkNameInConf,recordingLinkName,H243_NAME_LEN))
							{
								isRecordingLinkInUse = YES;
								PTRACE(eLevelInfoNormal, "CConfPartyManager::IsRecordingLinkInUse, Meeting Room ");
								POBJDELETE(pCommRes);
								break;
							}
						}
					}
					POBJDELETE(pCommRes);
				}
			}
		}
	}

	// checks Profiles
	if(!isRecordingLinkInUse)
	{
		CCommResDB* pProfDataBase = ::GetpProfilesDB();
		CCommRes*	pCommRes = NULL;

		CCommResDB::ReservArray tempReservArray = pProfDataBase->GetReservArray();
		for (CCommResDB::ReservArray::iterator itr=tempReservArray.begin() ; itr != tempReservArray.end() ; ++itr)
		{
			if ((*itr) != NULL)
			{
				WORD wConfId = (*itr)->GetConferenceId();
				pCommRes = pProfDataBase->GetCurrentRsrv(wConfId);
				if(pCommRes != NULL)
				{
					const char * linkNameInConf = pCommRes->GetRecLinkName();
					if(linkNameInConf && pCommRes->GetEnableRecording())
					{
						if(!strncmp(linkNameInConf,recordingLinkName,H243_NAME_LEN))
						{
							isRecordingLinkInUse = YES;
							PTRACE(eLevelInfoNormal, "CConfPartyManager::IsRecordingLinkInUse, In Profile");
							POBJDELETE(pCommRes);
							break;
						}
					}
					POBJDELETE(pCommRes);
				}
			}
		}
	}


	return isRecordingLinkInUse;

}
//=====================================================================================================================================//
void CConfPartyManager::OnCardsSystemBasedModeInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnCardsSystemBasedModeInd");
  if (m_isSystemCardsModeReceived)
  {
    PTRACE(eLevelInfoNormal, "CConfPartyManager::OnCardsSystemBasedModeInd - ConfParty was already updated with the system cards mode ");
    return;
  }
  else
  {
    DWORD tempMode = (DWORD)eSystemCardsMode_illegal;
    *pMsg >> tempMode;

    eSystemCardsMode systemCardsMode = (eSystemCardsMode)tempMode;
    PTRACE2(eLevelInfoNormal, "CConfPartyManager::OnCardsSystemBasedModeInd - Mode received from Cards process: ", ::GetSystemCardsModeStr(systemCardsMode));

    m_pProcess->SetSystemCardsBasedMode(systemCardsMode); //In this function we update the system capacity limits according to the system mode
    RemoveActiveAlarmByErrorCode(AA_SYSTEM_BASED_MODE_NOT_INTIALIZED);
    m_isSystemCardsModeReceived = YES;

    UpdateSystemCapacityLimits();
    CreateDefaultProfiles();
    AddActiveAlarmForCOPWithMPMPlus();


    // init the resolution control mechanism just in pure MPM mode
    if (systemCardsMode == eSystemCardsMode_mpm)
      pDecoderResolutionTable = new CDecoderResolutionTable();
    verifyExitFromStartUpAndPerformAfterStartUpActions();
  }
}
//=====================================================================================================================================//
void CConfPartyManager::SendCardConfigReq()//2 modes cop/cp - sends to CARDS->MFA
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::SendCardConfigReq");
  CSegment* pSeg = new CSegment;

  CARDS_CONFIG_PARAMS_S CardConfigParams;
  PTRACE(eLevelInfoNormal, "CConfPartyManager::SendCardConfigReq");
  CardConfigParams.unSystemConfMode = ::GetIsCOPdongleSysMode() ? eSystemConfMode_Cop : eSystemConfMode_Cp;
  CardConfigParams.unFutureUse = 0;

  pSeg->Put((BYTE*)&CardConfigParams, sizeof(CARDS_CONFIG_PARAMS_S));

  const COsQueue* pCardsMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);

  pCardsMbx->Send(pSeg, CARD_CONFIG_REQ);
}
////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::SendSystemBasedModeReqToCardMngr()
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::SendSystemBasedModeReqToCardMngr");
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pCardsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);

	STATUS res = pCardsMbx->Send(pRetParam, CONFPARTY_SYSTEM_CARDS_MODE_REQ);

}
////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnSystemRamSizeInd(CSegment* pMsg)
{
	DWORD tempSize = (DWORD)eSystemRamSize_illegal;
	*pMsg >> tempSize;

	eSystemRamSize systemRamSize = (eSystemRamSize)tempSize;
	PTRACE2(eLevelInfoNormal, "CConfPartyManager::OnSystemRamSizeInd Size received from McuMngr process: ", ::GetSystemRamSizeStr(systemRamSize));

//  Do what is needed
}

////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnExchangeConfigInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnExchangeConfigInd");
	BOOL exchangeConfigured;
    *pMsg >> exchangeConfigured;
    if (exchangeConfigured == FALSE)
    {
    	m_bExchangeConfigured = FALSE;
    	TRACEINTO << "\nCConfPartyManager::OnExchangeConfigInd Value received from ExchangeMngr process : Disabled";
    }
    else
    {
    	m_bExchangeConfigured = TRUE;
    	TRACEINTO << "\nCConfPartyManager::OnExchangeConfigInd Value received from ExchangeMngr process : Enabled";
    }
}
////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnConfBlockInd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnConfBlockInd");
	BYTE blockConf = FALSE;
	BYTE blockConfReason;
	STATUS status = STATUS_OK;

	if(NULL == pSeg)
	{
		PTRACE(eLevelError, "OnConfBlockInd(): Input segment is NULL");
		return STATUS_OK;
	}

	*pSeg >> blockConf;
	*pSeg >> blockConfReason;

	if(blockConf)
	{
		if(::GetpConfDB()->GetConfNumber() != 0 )
		{
			status = STATUS_FAIL;
			PTRACE(eLevelInfoNormal,"CConfPartyManager::OnConfBlockInd, block conf request denied - active conferences exist");
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CConfPartyManager::OnConfBlockInd, block conf request approved - no active conferences");
			m_lockConfReqCounter++;

			if (blockConfReason == eConfBlockReason_McuMngr_InvalidCertificate)
			{
				m_bLockConfForInvalidCertificate = TRUE;
			}
		}
	}
	else
	{
		if (blockConfReason == eConfBlockReason_McuMngr_InvalidCertificate ||
				blockConfReason == eConfBlockReason_McuMngr_Position_1)
		{
			m_bLockConfForInvalidCertificate = FALSE;
		}

		if(m_lockConfReqCounter)
			m_lockConfReqCounter--;
		if(m_lockConfReqCounter == 0)
		{
			PTRACE(eLevelInfoNormal,"CConfPartyManager::OnConfBlockInd, release conf blocking");
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CConfPartyManager::OnConfBlockInd, release conf blocking is not done, not all processes sent release");
		}

		return STATUS_OK;
	}


	CSegment*  pRetParam = new CSegment;
	*pRetParam << (BYTE)status;
	// Only on blocking request, and a sync one
	 if(eSyncMessage ==  m_ClientRspMsgType ||  eDirectSyncMessage ==  m_ClientRspMsgType)
	 {
		 ResponedClientRequest(status, pRetParam);
	 }

	 return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::CConfPartyManager::OnGetNumOfConferences(CSegment* pSeg)
{
    WORD confNum = ::GetpConfDB()->GetConfNumber();

    CSegment *pRetParam = new CSegment;
    *pRetParam << (DWORD)confNum;

    STATUS status = ResponedClientRequest(GET_CONF_NUM_IND, pRetParam);

    return status;
}

//=====================================================================================================================================//
STATUS CConfPartyManager::OnMcuMngrPrecedenceSettings(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnMcuMngrPrecedenceSettings");

	pPrecedenceSettingsDB = new CPrecedenceSettings();
	pPrecedenceSettingsDB->DeSerialize(NATIVE, *pSeg);

//	pPrecedenceSettingsDB->PrintToConsole();
	//::GetpPrecedenceSettingsDB()->PrintToConsole();

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnMcuMngrLicensingInd(CSegment* pSeg)
{
	CPrettyTable<const char*, int> tbl;

	const CONFPARTY_LICENSING_S *pParam = (const CONFPARTY_LICENSING_S*)(pSeg->GetPtr());
	eProductType productType = CProcessBase::GetProcess()->GetProductType();
	eProductFamily familyType = CProcessBase::GetProcess()->GetProductFamily();

	CONFPARTY_LICENSING_S CfsParams_fromMcuMngr;
	memcpy(&CfsParams_fromMcuMngr, pParam, sizeof(CONFPARTY_LICENSING_S));

	int dummy = CfsParams_fromMcuMngr.confPartyLicensing_encryption;
	tbl.Add("Is Encryption allowed", dummy);
	::SetDongleEncryptionValue(CfsParams_fromMcuMngr.confPartyLicensing_encryption);

	int dummy1 = CfsParams_fromMcuMngr.confPartyLicensing_pstn;
	tbl.Add("Is PSTN allowed", dummy1);
	::SetDonglePstnValue(CfsParams_fromMcuMngr.confPartyLicensing_pstn);

	int dummy2 = CfsParams_fromMcuMngr.confPartyLicensing_telepresence;
	tbl.Add("Is Telepresence allowed", dummy2);
	::SetDongleTelepresenceValue(CfsParams_fromMcuMngr.confPartyLicensing_telepresence);

	int dummy3 = CfsParams_fromMcuMngr.confPartyLicensing_ms;
	tbl.Add("Is MS allowed", dummy3);
	::SetDongleMsValue(CfsParams_fromMcuMngr.confPartyLicensing_ms);

	int dummy4 = CfsParams_fromMcuMngr.confPartyLicensing_partner_IBM;
	tbl.Add("Is IBM allowed", dummy4);
	m_bIsIBMLicense = (CfsParams_fromMcuMngr.confPartyLicensing_partner_IBM) ? TRUE : FALSE;

	int dummy5 = CfsParams_fromMcuMngr.federal;
	tbl.Add("Is Federal allowed", dummy5);
	::SetDongleFederalValue(CfsParams_fromMcuMngr.federal);

	DWORD dummy6 = CfsParams_fromMcuMngr.num_cop_parties;
	tbl.Add("Number Of COP parties", dummy6);
	::SetIsCOPdongleSysMode(dummy6 > 0 ? TRUE : FALSE);

	DWORD dummy7 = CfsParams_fromMcuMngr.confPartyLicensing_HD;
	tbl.Add("Is HD enabled for 1500_Q", dummy7);
	::SetDongle1500qHDvalue(CfsParams_fromMcuMngr.confPartyLicensing_HD);

	int dummy8 = CfsParams_fromMcuMngr.confPartyLicensing_svc;
	tbl.Add("Is SVC enabled", dummy8);
	::SetDongleSvcValue(CfsParams_fromMcuMngr.confPartyLicensing_svc);

//SRS licensing flexera:This item should be displayed as true for non VM MCU (Gesher, HW MCU and Ninja).
	if( eProductFamilyRMX == familyType ||
		(eProductFamilySoftMcu == familyType &&
		eProductTypeGesher == productType &&
		eProductTypeNinja == productType))
	{
		CfsParams_fromMcuMngr.confPartyLicensing_CIF_Plus = TRUE;
		CfsParams_fromMcuMngr.confPartyLicensing_TipInterop = TRUE;
	}



	int dummy9 = CfsParams_fromMcuMngr.confPartyLicensing_CIF_Plus;
	tbl.Add("Is AVC enabled", dummy9);

	::SetDongleCifPlusValue(CfsParams_fromMcuMngr.confPartyLicensing_CIF_Plus);

	CResRsrcCalculator resRsrcCalculator;
	STATUS ret_stat = resRsrcCalculator.SetResolutionAccordingToLicensing(CfsParams_fromMcuMngr.confPartyLicensing_CIF_Plus,GetSystemCardsBasedMode());
	if( ret_stat != STATUS_OK )
	{
		DBGPASSERT(ret_stat);
		PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnMcuMngrLicensingInd : licensing didn't success to write Resolution Slider Configuration file, status = ", ret_stat);
	}


	int dummy10 = CfsParams_fromMcuMngr.confPartyLicensing_TipInterop;
	tbl.Add("Is TipInterop enabled", dummy10);
	::SetDongleTipInteropValue(CfsParams_fromMcuMngr.confPartyLicensing_TipInterop);

	int dummy11 = CfsParams_fromMcuMngr.isLicenseExpired;
	tbl.Add("Is lincense Expired", dummy10);

	if(eProductTypeEdgeAxis != productType)
	{
		::SetDongleLicenseExpiredValue(TRUE);
	}
	else
	{
		::SetDongleLicenseExpiredValue(!CfsParams_fromMcuMngr.isLicenseExpired);
	}

	RemoveActiveAlarmByErrorCode(AA_NO_LICENSING);

	//	int dummy5 = CfsParams_fromMcuMngr.confPartyLicensing_internalScheduler;
	eTaskState taskState = GetTaskState();
	const char *taskStateName = GetTaskStateName(taskState);
	TRACEINTO << taskStateName << tbl.Get();

	UpdateSystemCapacityLimits();
	CreateDefaultProfiles();
	AddActiveAlarmForCOPWithMPMPlus();

	verifyExitFromStartUpAndPerformAfterStartUpActions();

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnSipProxyDBReq(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnSipProxyDBReq");
	ON(m_SipProxyDBReqReceived);

	if (m_isNeedToAddGWDefaults && !m_isDefaultGWSessionAdded)
	{
		PTRACE(eLevelInfoNormal, "CConfPartyManager::OnSipProxyDBReq - DB is not updated yet (need to add GW defaults before sending)");
		return STATUS_OK;
	}
	CSipProxyManagerApi SipProxyApi;
        CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
        CConfIpParameters* pServiceParams = NULL;

	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pCommConf = pCommConfDB->GetFirstCommConf();
	while(IsValidPObjectPtr(pCommConf))
	{
		const CStructTm* pStartTime = pCommConf->GetStartTime();
		const CStructTm* pEndTime = pCommConf->GetEndTime();
		DWORD  durationTime = *pEndTime - *pStartTime;
		for(int i=1; i<=NUM_OF_IP_SERVICES; i++)
		{
			pServiceParams = pIpServiceListManager->FindServiceByName(pCommConf->GetServiceRegistrationContentServiceName(i-1));
			if (pServiceParams != NULL)
			{
				if( pCommConf->GetServiceRegistrationContentRegister(i-1) == TRUE )
				  SipProxyApi.AddConference(pServiceParams->GetServiceId(), pCommConf->GetName(), pCommConf->GetMonitorConfId(), pCommConf->GetEntryQ() ,durationTime);
			}
		}
		pCommConf = pCommConfDB->GetNextCommConf();
	}

	CCommResDB *pMRDataBase = GetpMeetingRoomDB();

	const CCommResDB::ReservArray& tempReservArray = pMRDataBase->GetReservArray();
	CCommResDB::ReservArray::const_iterator itr_end = tempReservArray.end();
	for (CCommResDB::ReservArray::const_iterator itr = tempReservArray.begin() ; itr != itr_end ; ++itr)
	{
		if ((*itr) != NULL)
		{
			WORD wConfId = (*itr)->GetConferenceId();
			CCommRes* pCommRes = pMRDataBase->GetCurrentRsrv(wConfId);
			if(pCommRes != NULL)
			{
				const CStructTm* pStartTime = pCommRes->GetStartTime();
				const CStructTm* pEndTime = pCommRes->GetEndTime();
				DWORD  durationTime = *pEndTime - *pStartTime;
				for(int i=1; i<=NUM_OF_IP_SERVICES; ++i)
				{
					pServiceParams = pIpServiceListManager->FindServiceByName(pCommRes->GetServiceRegistrationContentServiceName(i-1));
					if (pServiceParams != NULL)
					{
					   if( pCommRes->GetServiceRegistrationContentRegister(i-1) == TRUE )
					   {
						 if(pCommRes->GetEntryQ())
							 SipProxyApi.AddEQ(pServiceParams->GetServiceId(), pCommRes->GetName(), pCommRes->GetMonitorConfId(), durationTime);
						 else if(pCommRes->IsSIPFactory())
							 SipProxyApi.AddFactory(pServiceParams->GetServiceId(), pCommRes->GetName(), pCommRes->GetMonitorConfId(), durationTime);
						 else if(pCommRes->GetIsGateway())
							 SipProxyApi.AddGW(pServiceParams->GetServiceId(), pCommRes->GetName(), pCommRes->GetMonitorConfId(), durationTime);
						 else
							 SipProxyApi.AddMR(pServiceParams->GetServiceId(), pCommRes->GetName(), pCommRes->GetMonitorConfId(), durationTime);

					   }
					}
				}
				POBJDELETE(pCommRes);
			}
		}
	}

    return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnConfPartyReportOnActiveAlarmInd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnConfPartyReportOnActiveAlarmInd");
	DWORD activeAlramOpcode = 0;
	DWORD activeAlramMsgLen = 0;
	char lStr[120];

	*pSeg  >> activeAlramOpcode;
	*pSeg  >> lStr;

	AddActiveAlarm(FAULT_GENERAL_SUBJECT, activeAlramOpcode, MAJOR_ERROR_LEVEL, lStr, true, true);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnSpecificMeetingRoomInformationReq(CSegment* pSeg)
{
  DWORD meeting_index;

  if (m_pMrAndProfileListDuringStartup == NULL)
    {
      PASSERT(1);
      return STATUS_FAIL;
    }

  *pSeg  >> meeting_index;


  if (meeting_index >= m_pMrAndProfileListDuringStartup->mr_list.list_size)
    {
          PASSERT(1);
	  return STATUS_FAIL;
    }

  CSegment*  ret_seg = new CSegment;
  ret_seg->Put((BYTE *) (&(m_pMrAndProfileListDuringStartup->mr_list.monitor_numeric_list[meeting_index])),
	       sizeof(MR_MONITOR_NUMERIC_ID_S));


  ResponedClientRequest(STATUS_OK,ret_seg);
  return STATUS_OK;
}
/////


////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleTerminalMove(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleTerminalMove ");

	DWORD numOfParams = command.GetNumOfParams();
	if(3 != numOfParams)
	{
		answer << "error: action must be specified:StartMoveExport\n";
		answer << "usage: Bin/McuCmd DisableUnit Resource [Conf Name] [Party Name] [Dest Conf Name]\n";
		return STATUS_FAIL;
	}

	const string &sourceConfName 	= command.GetToken(eCmdParam1);
	const string &partyName 		= command.GetToken(eCmdParam2);
	const string &destConfName 		= command.GetToken(eCmdParam3);

	answer <<  "Source Conf " << sourceConfName << "Party " << partyName << "Dest Conf " << destConfName;

	// Find Source/Dest Conf in DB
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pSourceRequestedConf = NULL;
	CCommConf* pDestRequestedConf = NULL;

	 pSourceRequestedConf = pCommConfDB->GetCurrentConf(sourceConfName.c_str());
	 if(!CPObject::IsValidPObjectPtr(pSourceRequestedConf))
	 {
	 	answer << "error: Source Conf does not exist in DB" << " " <<  sourceConfName << "StartMoveExport ";
		return STATUS_FAIL;
	 }
	 if(pCommConfDB->SearchPartyName(sourceConfName.c_str(),partyName.c_str()) != STATUS_OK)
	 {
	 	answer << "error: Party does not exist in DB" << " " <<  sourceConfName << " " << partyName << " "<< "StartMoveExport ";
		return STATUS_FAIL;
	 }

	  pDestRequestedConf = pCommConfDB->GetCurrentConf(destConfName.c_str());
	  if(!CPObject::IsValidPObjectPtr(pDestRequestedConf))
	 {
	 	answer << "error: Dest Conf does not exist in DB" << " " <<  destConfName << " " << "StartMoveExport ";
		return STATUS_FAIL;
	 }


	   DWORD destConfId = pDestRequestedConf->GetMonitorConfId();
	   DWORD partId = pCommConfDB->GetPartyId(sourceConfName.c_str(), partyName.c_str());
	   MoveActions(pSourceRequestedConf,pDestRequestedConf,partId);

	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::MoveActions(CCommConf* pSourceConf,CCommConf* pDestConf,DWORD sourcePartyId,EMoveType eMoveType)
{

  // validate source conf
  if(!CPObject::IsValidPObjectPtr(pSourceConf)){
      PTRACE(eLevelInfoNormal,"CConfPartyManager::MoveActions - pSourceConf is not valid");
      return STATUS_FAIL;
  }
  // validate dest  conf
  if(!CPObject::IsValidPObjectPtr(pDestConf)){
      PTRACE(eLevelInfoNormal,"CConfPartyManager::MoveActions - pDestConf is not valid");
      return STATUS_FAIL;
  }

  if(pDestConf->GetMonitorConfId()==pSourceConf->GetMonitorConfId()){
      PTRACE(eLevelInfoNormal,"CConfPartyManager::MoveActions - destination and source conf are identical");
      return STATUS_FAIL;
  }


	STATUS status = STATUS_OK;
	CConfParty* pSourceParty = NULL;
	// 1. Verify dest Conf is ready for move (sync)
	CConfApi* pDestConfApi = new CConfApi;
	pDestConfApi->CreateOnlyApi(*(pDestConf->GetRcvMbx()),NULL,NULL,1);
	STATUS rspStatus = STATUS_OK;
    OPCODE rspOpcode = STATUS_OK;
    DWORD destConfId = pDestConf->GetMonitorConfId();
    CSegment rspMsg;
	rspStatus =  pDestConfApi->IsDestConfReadyForMove( rspMsg, 5*SECOND, rspOpcode);
    pDestConfApi->DestroyOnlyApi();
	POBJDELETE(pDestConfApi);

    if ((rspStatus != STATUS_OK) || (rspOpcode != STATUS_OK))
	{
	  PTRACE2(eLevelError, "CConfPartyManager::MoveActions: Dest Conf is not ready for move", pDestConf->GetName());
	  return STATUS_FAIL;
	}

    // 2. Verify party exists in source conf
    pSourceParty = pSourceConf->GetCurrentParty(sourcePartyId);
    if (!pSourceParty){
    		PTRACE(eLevelError,"FATAL,CConfPartyManager::MoveActions, source party is NULL");

    		status = STATUS_PARTY_DOES_NOT_EXIST;
    		return status;

    }


    CMoveInfo* partyMoveInfo = pSourceParty->GetMoveInfo();
   	DWORD prevSourceConfId	= partyMoveInfo->GetPreviousConf();
   	DWORD prevDestConfId   =  partyMoveInfo->GetCurrentConf();
   	if((pDestConf->GetMonitorConfId() == prevDestConfId) &&  (pSourceConf->GetMonitorConfId() == prevSourceConfId))
   	{
   	   // AFter consulting with Varda -  in case of a successive identical move request - ignore it and return STATUS_OK to API.
   	   return STATUS_OK;
   	}
    //  3. Test Move Validity
    status = ::TestMoveValidity(pDestConf->GetMonitorConfId(),pSourceConf->GetMonitorConfId(),pSourceParty);
    if(status != STATUS_OK)
    {
    	 PTRACE2(eLevelError, "CConfPartyManager::MoveActions: Move Validity fails, Dest Conf: ", pDestConf->GetName());
         return status;
    }

    DWORD opcode = IVR_PARTY_START_MOVE_FROM_EQ;
    if(!pDestConf->GetEntryQ())
    {
    	opcode = IVR_PARTY_START_MOVE_FROM_CONF;
    }

    //4. update move info
     CMoveConfDetails sourceConfMoveDetails(pSourceConf);
     CMoveConfDetails destConfMoveDetails(pDestConf);
     partyMoveInfo->UpdateMove(sourceConfMoveDetails,destConfMoveDetails);

     DWORD state = pSourceParty->GetPartyState();
     if(PARTY_DISCONNECTED == state || PARTY_WAITING_FOR_DIAL_IN == state || PARTY_STAND_BY == state )
     {
        status = HandleColdMove(pSourceConf,pDestConf,pSourceParty);
        return status;
     }

    CConfApi* pConfApi = new CConfApi;
	pConfApi->CreateOnlyApi(*(pSourceConf->GetRcvMbx()));
	//		pConfApi->SetLocalMbx(pSourceRequestedConf->GetLocalQueue());
	CSegment* pSeg = new CSegment;
	*pSeg << pSourceParty->GetName();
	// 5. Notify Cam in Source Conf
    pConfApi->SendCAMGeneralNotifyCommand( (DWORD)0, opcode, pSeg  );




    // 6. Update CDR
    SendMoveCDREvents(pSourceConf,pDestConf,sourcePartyId,eMoveType);
    // 7. Send Export request to source Conference
    pConfApi->MoveParty(sourcePartyId,destConfId,eMoveType);
	POBJDELETE(pConfApi);
	POBJDELETE(pSeg);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::SendMoveCDREvents(CCommConf* pSourceConf,CCommConf* pDestConf,DWORD sourcePartyId,EMoveType eMoveType)
{
	STATUS status = STATUS_OK;
	// validate source conf
	 if(!CPObject::IsValidPObjectPtr(pSourceConf)){
	      PTRACE(eLevelInfoNormal,"CConfPartyManager::SendMoveCDREvents - pSourceConf is not valid");
	      return STATUS_FAIL;
	 }
	 // validate dest  conf
	 if(!CPObject::IsValidPObjectPtr(pDestConf)){
	      PTRACE(eLevelInfoNormal,"CConfPartyManager::SendMoveCDREvents - pDestConf is not valid");
	      return STATUS_FAIL;
	 }
	 //Verify party exists in source conf
	 CConfParty* pSourceParty=NULL;
	 pSourceParty = pSourceConf->GetCurrentParty(sourcePartyId);
	 if (!pSourceParty){
	    	PTRACE(eLevelError,"FATAL,CConfPartyManager::SendMoveCDREvents, source party is NULL");
	    	status = STATUS_PARTY_DOES_NOT_EXIST;
	    	return status;
	 }
	 // Update CDR
	 const char* operatoreName = NULL;
	 if(!pSourceConf->GetEntryQ())
		 operatoreName = GetLoginName().c_str();

	 switch(eMoveType)
	 {
	    case eMoveAttend:
	    {
	     	  pDestConf->OperatorMovePartyToConf(pSourceParty,operatoreName,(char*)pSourceConf->GetName(),pSourceConf->GetMonitorConfId(),OPERRATOR_ATTEND_PARTY_TO_CONFERENCE);
	     	  pDestConf->OperatorMovePartyToConfEventToCdr(pSourceParty,operatoreName,(char*)pSourceConf->GetName(),pSourceConf->GetMonitorConfId(), NULL, NULL);

	     	  pSourceConf->OperatorMovePartyFromConf(operatoreName,pSourceParty->GetName(),sourcePartyId,OPERRATOR_ATTEND_PARTY,(char*)pDestConf->GetName(),pDestConf->GetMonitorConfId());

	     	  break;
	    }

	    case eMoveBack:
	    case eMoveBackIntoIvr:
	    {
	     	  pDestConf->OperatorMovePartyFromConf(operatoreName,(char*)pSourceParty->GetName(),sourcePartyId,OPERRATOR_BACK_TO_CONFERENCE_PARTY,(char*)pDestConf->GetName(),pDestConf->GetMonitorConfId());
	     	  pSourceConf->OperatorMovePartyFromConf(operatoreName,pSourceParty->GetName(),sourcePartyId,OPERRATOR_BACK_TO_CONFERENCE_PARTY,(char*)pDestConf->GetName(),pDestConf->GetMonitorConfId());
	       	 break;
	    }
	    case eMoveDefault:
	    case eMoveIntoIvr:
	    default:
	    {

	       			// send move from conf and move to conf CDR events
	     	  pDestConf->OperatorMovePartyToConf(pSourceParty,operatoreName,(char*)pSourceConf->GetName(),pSourceConf->GetMonitorConfId(),OPERRATOR_MOVE_PARTY_TO_CONFERENCE);
	       	  pSourceConf->OperatorMovePartyFromConf(operatoreName,pSourceParty->GetName(),sourcePartyId,OPERRATOR_MOVE_PARTY_FROM_CONFERENCE,(char*)pDestConf->GetName(),pDestConf->GetMonitorConfId());
	       	// send move party calling number CDR event
	       	  Phone * CallingNum =pSourceParty->GetActualPartyPhoneNumber(0);
	       	  if (CallingNum!= NULL)
	       	  {
	       		 const char* pCallingNum = CallingNum->phone_number;
	       		if(strcmp(pCallingNum,""))
	       			pSourceConf->CDRPartyCallingNumber_Move_to_Cont_1(pDestConf->GetMonitorConfId(),pSourceParty->GetName(), sourcePartyId, pCallingNum);
	       	  }
	       	  // send move party called number CDR event
	       	  Phone * CalledNum =pSourceParty->GetActualMCUPhoneNumber(0);
	       	  if (CalledNum!= NULL)
	       	  {
	       		 const char* pCalledNum = CalledNum->phone_number;
	       		 if(strcmp(pCalledNum,""))
	       			   pSourceConf->CDRPartyCalledNumber_Move_to_Cont_2(pDestConf->GetMonitorConfId(),pSourceParty->GetName(), sourcePartyId, pCalledNum);
	       	  }

	       	  pDestConf->OperatorMovePartyToConfEventToCdr(pSourceParty,operatoreName,(char*)pSourceConf->GetName(),pSourceConf->GetMonitorConfId(), CallingNum, CalledNum);
	       	  break;
	      }

	    }


	     //pDestConf->OperatorAddPartyCont2(pSourceParty,OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_3,YES);

      return status;
}

////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::ReceiveAdditionalParams(CSegment* pSeg)
{
	DWORD dwLen = 0;

	if(!pSeg)
		return;

//	TRACEINTO_GLA << "Seg Len = " << pSeg->GetLen() << "     Seg Offset = " << pSeg->GetRdOffset();
	if (pSeg->GetLen() <= pSeg->GetRdOffset())
		return;

	if(m_operName != NULL)
	{
		delete [] m_operName;
		m_operName = NULL;
	}

	*pSeg >> dwLen;

//	TRACEINTO_GLA << "Seg Len = " << pSeg->GetLen() << ", Seg Offset = " << pSeg->GetRdOffset()	<< "\n\tdwLen = " << dwLen;
	if (dwLen > 0 && pSeg->GetLen() > pSeg->GetRdOffset())
	{
		m_operName = new char[dwLen+1];
		*pSeg >> m_operName;
//		TRACEINTO_GLA << "m_operName = " << m_operName;
	}
}
////////////////////////////////////////////////////////////////////////////////////
// IP Resource Report
////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnGKManagerResourceQuery(CSegment * pSeg)
{
    PTRACE(eLevelInfoNormal, "CConfPartyManager::OnGKManagerResourceQuery");
    CSegment *pDummy = new CSegment(*pSeg);
    SendMsgToRsrvMngrRsrcProcess(pDummy, IP_RESOURCE_INFO_REQ);
}
////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnIpResourceReportInd (CSegment * pSeg)
{
    PTRACE(eLevelInfoNormal, "CConfPartyManager::OnIpResourceReportInd");
    DWORD serviceId;
    *pSeg >> serviceId;
    ALLOC_REPORT_PARAMS_S* pResourceReport = (ALLOC_REPORT_PARAMS_S*) pSeg->GetPtr(1);
    CIPResourceGenerator  pIPRsrcGenerator;
    CIPAdHocProfilesReport * pAdHocReport = new CIPAdHocProfilesReport;
    CCommResDB * pResDB = GetpMeetingRoomDB();
    CCommResDB * pProfilesDB = GetpProfilesDB();

    pIPRsrcGenerator.GenerateAdHocProfilesReport(pAdHocReport, pResourceReport, pResDB,pProfilesDB );

    //Send to GK Manager
    CProcessBase* pProcess = CProcessBase::GetProcess();
	if (!pProcess)
	{
		PASSERTMSG(GKMNGR_RESOURCE_INFO_IND,"CConfPartyManager::OnIpResourceReportInd - Process not valid");
		delete pAdHocReport;

		return;
	}

	//const COsQueue* pCSManager = pProcess->GetOtherProcessQueue(eProcessGatekeeper, eManager);
	//CTaskApi api;
	//api.CreateOnlyApi(*pCSManager);
	CGatekeeperTaskApi api(serviceId);
    CSegment * pSegment = new CSegment;
    pAdHocReport->Serialize (NATIVE, *pSegment);
	STATUS res = api.SendMsg(pSegment ,GKMNGR_RESOURCE_INFO_IND);
	if (res != STATUS_OK)
		FPASSERT(GKMNGR_RESOURCE_INFO_IND);

    POBJDELETE(pAdHocReport);
}


////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/// Global Functions
//rsrc table
CConfPartyRoutingTable* GetpConfPartyRoutingTable()
{
  return pConfPartyRoutingTable;
}
/////////////////////////////////////////////////////////////////////////////
//confDB
extern CCommConfDB* GetpConfDB()
{
	return pConfDB;
}

/////////////////////////////////////////////////////////////////////////////
//CDR_MCCF:
extern std::vector< COsQueue > GetClientRspMbxForMCCFtwinTxList()
{
	return clientRspMbxList;
}

/////////////////////////////////////////////////////////////////////////////
//Slide Conversion Status
extern std::map<DWORD, eIvrSlideConversionStatus> & GetConnectConversionStatus()
{
    return g_mapConnectConversionStatus;
}

/////////////////////////////////////////////////////////////////////////////
//precedenceSettingsDB
extern CPrecedenceSettings* GetpPrecedenceSettingsDB()
{
	return pPrecedenceSettingsDB;
}
/////////////////////////////////////////////////////////////////////////////
// Ip Service List Manager
extern CIpServiceListManager* GetIpServiceListMngr()
{
	CConfPartyProcess* pCPProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	CIpServiceListManager* pIpServiceListManager = pCPProcess->GetIpServiceListManager();
	return pIpServiceListManager;
}
/////////////////////////////////////////////////////////////////////////////
extern CCommResDB * GetpProfilesDB()
{
	return g_pProfilesDB;
}
/////////////////////////////////////////////////////////////////////////////
extern CCommResDB * GetpMeetingRoomDB()
{
	return g_pMeetingRoomsDB;
}
/////////////////////////////////////////////////////////////////////////////
//RecordingLinkDB
extern CRecordingLinkDB* GetRecordingLinkDB()
{
	return pRecordingLinkDB;
}

/////////////////////////////////////////////////////////////////////////////
//DecoderResolutionTable
extern CDecoderResolutionTable* GetpDecoderResolutionTable()
{
	return pDecoderResolutionTable;
}
//////////////////////////////////////////////////////////////////////////
extern CCommResDB * GetpConfTemplateDB()
{
	return g_pConfTemplateDB;
}
/////////////////////////////////////////////////////////////////////////////
// for Call Generator - Vendor detection
extern BOOL GetVendorDetection()
{
	return isVendorDetection;
}
//////////////////////////////////////////////////////////////////////////

STATUS CConfPartyManager::OnServerSetConfChairPass(CRequest *pRequest)
{
  PTRACE(eLevelInfoNormal, "CConfPartyManager::OnServerSetConfChairPass Starting function");
  if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
  {
        FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetConfChairPass: No permission to OnServerSetConfChairPass for administrator readony");
        pRequest->SetConfirmObject(new CDummyEntry());
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_NO_PERMISSION;
  }
  STATUS status = STATUS_OK;

  CConfAction* pConfAction = new CConfAction;

  *pConfAction = *(CConfAction*)pRequest->GetRequestObject();
  pRequest->SetObjectFlag(STRING_FLAG);

  const DWORD confId = pConfAction->GetConfID();

  // VALIDITY of conference Id
  if (::GetpConfDB()->FindId(confId) == NOT_FIND){
    PTRACE(eLevelError,"CConfPartyManager::OnServerSetConfChairPass Can not find conference id");
    PASSERT(1);
    status = STATUS_CONF_NOT_EXISTS;
  }

  if (status == STATUS_OK)
  {
      CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
	  if(pCurConf)
      {
          DWORD chairPasswordMinLength;
          DWORD chairPasswordMaxLength;
		  CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("NUMERIC_CHAIR_PASS_MIN_LEN", chairPasswordMinLength);
		  CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("NUMERIC_CHAIR_PASS_MAX_LEN", chairPasswordMaxLength);
		  status = pCurConf->CheckPasswordValidity(pConfAction->GetChairPersonPassword(),chairPasswordMinLength, chairPasswordMaxLength);
		  if (status == STATUS_OK)
		  {
              DWORD dwMaxRepeatedDigits;
              CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("MAX_CONF_PASSWORD_REPEATED_DIGITS",dwMaxRepeatedDigits);
              status = pCurConf->CheckPasswordRepeatedDigitsValidity(pConfAction->GetChairPersonPassword(), dwMaxRepeatedDigits);
              if (status != STATUS_OK)
              {
                  FPTRACE(eLevelInfoNormal,"CCommRes::OnServerConfEntryPW - Maximum number of permitted repeated characters in conf. chair password has been exceeded");
              }
              else
              {
                  pCurConf->SetH243Password(pConfAction->GetChairPersonPassword());
              }
          }
      }
  }

    if (status == STATUS_OK)
    {
		  CStructTm curTime;
		  PASSERT(SystemGetTime(curTime));

		  CCdrLogApi cdrApi;
		  CCdrEvent cdrEvent;
		  CConfStartCont4* confStartCont4 = new CConfStartCont4;
		  confStartCont4->SetChair_password(pConfAction->GetChairPersonPassword());


		  cdrEvent.SetCdrEventType(CONFERENCE_START_CONTINUE_4);
		  cdrEvent.SetTimeStamp(curTime);
		  cdrEvent.SetConfStartCont4(confStartCont4);
		  POBJDELETE(confStartCont4);

		  cdrApi.ConferenceEvent(confId, cdrEvent);
     }

  //Trace
  PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerSetConfChairPass, "
	  , CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

  // confirm
  pRequest->SetConfirmObject(pConfAction);
  pRequest->SetStatus(status);

  return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::ValidateUpdateMR(CCommResApi * pRsrv)
{
	STATUS status = STATUS_OK;
	const char *  confName = pRsrv->GetName();
	CCommResShort * pMrShort = ::GetpMeetingRoomDB()->GetCurrentRsrvShort(confName);

	if (!pMrShort)
	{
		TRACESTR(eLevelError) << "CConfPartyManager::ValidateUpdateMR Can not find the MR: "<< pRsrv->GetName()  << ",in the MR DB !!" ;
		return STATUS_RESERVATION_NOT_EXISTS;
	}


	std::auto_ptr<CCommResShort>  shortAutoPtr(pMrShort);
	std::string oldNumericId(shortAutoPtr->GetNumericConfId ());

	// VNGFE-8630
	if (pRsrv->GetMeetingRoomState() != shortAutoPtr->GetMeetingRoomState())
	{
		BYTE inputState = pRsrv->GetMeetingRoomState();
		BYTE realState = shortAutoPtr->GetMeetingRoomState();

		TRACEINTOLVLERR << "input meeting room state is " << (WORD)inputState
			<< ", but should be " << (WORD)realState;
		pRsrv->SetMeetingRoomState(realState);
	}

	//Make sure we can not update numeric id when MR is active
	if (pRsrv->GetMeetingRoomState() == MEETING_ROOM_ACTIVE_STATE
		&& oldNumericId != pRsrv->GetNumericConfId ())
	{
		return STATUS_CANNOT_UPDATE_ACTIVE_MR_OR_EQ;
	}

	return status;
}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::RejectMeetingRoomOrAdHocActivation(DWORD mrId, STATUS status,BOOL sendDeactivationToRA, BOOL isAdHoc, char* adHocConfName)
{
	CLobbyApi* pLobbyApi= ((CLobbyApi*)::GetpLobbyApi()) ;
	if(isAdHoc)
	{
		PTRACE(eLevelError, "CConfPartyManager::RejectMeetingRoomActivation - Add Hoc Conference was failed by Resource process - Send Reject to Lobby");
	}
	else
	{
	    PTRACE(eLevelError, "CConfPartyManager::RejectMeetingRoomActivation - Do not awake MR/EQ, because TestValidity fails - Send Reject to Lobby");
	}
	pLobbyApi->RejectStartMeetingRoom(mrId/*, undefId,*/ ,status,isAdHoc,adHocConfName);
	if(sendDeactivationToRA)
	{
		PTRACE(eLevelError, "CConfPartyManager::RejectMeetingRoomActivation - Send Rjeect to RA ");
		OPCODE opcode = DEACTIVATE_MR_REQ;

		CSegment *pToRsrcSeg = new CSegment;
    	*pToRsrcSeg << mrId ;

		SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, opcode);

	}
}
//=====================================================================================================================================//
void CConfPartyManager::CreateDefaultConfig()
{
  TRACESTR (eLevelInfoNormal) << "CConfPartyManager::CreateDefaultConfig";

  const char* profileName = ::GetIsCOPdongleSysMode() ? "Event_Mode_720P_832Kb" : "Factory_Video_Profile";
  CCommRes* pProfile = ::GetpProfilesDB()->GetCurrentRsrv(profileName);
  if (pProfile == NULL)
  {
    TRACESTR (eLevelError) << "CConfPartyManager::CreateDefaultConfig - Failed on find Default Profile";
    PASSERT(1);
    AddActiveAlarm(FAULT_GENERAL_SUBJECT, AA_CAN_NOT_CREATE_DEFAULT_PROFILE, MAJOR_ERROR_LEVEL, "Failed to find the Default Profile", true, true);
    POBJDELETE(pProfile);
    return;
  }

  typedef std::vector<CCommRes *> RsrvVector;
  RsrvVector vect;
  DWORD currentNid = 1000;
  DWORD defaulFactoryNid = 7001; //VNGR-7638 - agreed with Assaf that this would be the default factory numeric id
  DWORD rsrvId = 0;

  //Create a new Eq reservation
  CCommRes* pEq = ::GetpProfilesDB()->GetDefaultEQ(DEFAULT_EQ_NAME, pProfile->GetMonitorConfId(), rsrvId++, currentNid++);
  pEq->FillEmptyDiplayNameOrName();
  vect.push_back(pEq);

  //Add a new MR reservation
  typedef std::vector<std::string> MeetingRoomNames;
  MeetingRoomNames meetingRooms;

  //All default Meeting Rooms names
  meetingRooms.push_back("Maple_Room");
  meetingRooms.push_back("Oak_Room");
  meetingRooms.push_back("Juniper_Room");
  meetingRooms.push_back("Fig_Room");

  for (MeetingRoomNames::iterator it = meetingRooms.begin(); it != meetingRooms.end(); ++it)
  {
    CCommRes* pTmp = ::GetpProfilesDB()->GetDefaultMR(*it, pProfile->GetMonitorConfId(), rsrvId++, currentNid++);
    pTmp->FillEmptyDiplayNameOrName();

    //Add it to the reservation vector
    vect.push_back(pTmp);
  }

  //Create a new Factory reservation
  CCommRes* pTmp = ::GetpProfilesDB()->GetDefaultFact("DefaultFactory", pProfile->GetMonitorConfId(), rsrvId++, defaulFactoryNid);
  pTmp->FillEmptyDiplayNameOrName();
  vect.push_back(pTmp);

  STATUS status = STATUS_OK;
  //For each new reservation Test it's validation
  for (RsrvVector::iterator it = vect.begin(); STATUS_OK == status && it != vect.end(); ++it)
    if (STATUS_OK != (status = (*it)->TestAddValidity()))
    {
      TRACESTR (eLevelError) << "CConfPartyManager::CreateDefaultConfig - Failed on validation of: " << (*it)->GetName();
      PASSERT(status);
    }

  //Add all the reservations
  for (RsrvVector::iterator it = vect.begin(); it != vect.end(); ++it)
  {
    if (!(*it)->IsTemplate())
      status = ::GetpMeetingRoomDB()->Add(*(*it));

    if (STATUS_OK != status)
    {
      TRACESTR (eLevelError) << "CConfPartyManager::CreateDefaultConfig - Failed on adding of: " << (*it)->GetName();
      PASSERT(status);
    }
  }

  //Clear the vector
  for (RsrvVector::iterator it = vect.begin(); it != vect.end(); ++it)
  {
    POBJDELETE(*it);
    (*it) = 0;
  }
  vect.clear();

  POBJDELETE(pProfile);
}
//=====================================================================================================================================//
STATUS CConfPartyManager::CreateDefaultProfiles()
{
	if (eStartupConditionOk != GetStartupConditionStatusByErrorCode(AA_SYSTEM_BASED_MODE_NOT_INTIALIZED))
	{
		PTRACE(eLevelInfoNormal, "CConfPartyManager::CreateDefaultProfiles - Profiles creation postponed because of System Based Mode does not initialized yet");
		return STATUS_ILLEGAL;
	}

	if (eStartupConditionOk != GetStartupConditionStatusByErrorCode(AA_NO_LICENSING))
	{
		PTRACE(eLevelInfoNormal, "CConfPartyManager::CreateDefaultProfiles - Profiles creation postponed because of System Licensing does not initialized yet");
		return STATUS_ILLEGAL;
	}

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	TRACEINTO << "ProductType:" << ::ProductTypeToString(curProductType) << ", IsProfilesFolderEmpty:" << (WORD)m_isProfilesFolderEmpty;

	bool isCOPdongleSysMode = ::GetIsCOPdongleSysMode();

	if (m_isProfilesFolderEmpty)
	{
		if (isCOPdongleSysMode)
		{
			// In COP version we create COP default profiles:
			CCommResDB* pProfilesDB = ::GetpProfilesDB();

			for (int profileIndex = 0; profileIndex < CCOPConfigurationList::GetProfilesCount(); ++profileIndex)
			{
				CopProfileInfo* pProfileInfo = CCOPConfigurationList::GetProfileInfo(profileIndex);

				if (IsFeatureSupportedBySystem(eFeatureCOP) && !pProfilesDB->IsNameExist(pProfileInfo->profileName))
					CreateDefaultProfile_Cop(profileIndex);
			}
		}
		else
		{
			// create Mix (SVC+AVC CP) Profile
			switch (curProductType)
			{
				case eProductTypeRMX1500:
				case eProductTypeRMX2000:
				case eProductTypeRMX4000:
				case eProductTypeSoftMCU:
				case eProductTypeGesher:
				case eProductTypeEdgeAxis:
				case eProductTypeNinja:
					CreateDefaultProfile_FactoryMixCpVideo();
					CreateDefaultProfile_FactorySvcVideo();
					break;

				case eProductTypeSoftMCUMfw:
					CreateDefaultProfile_FactorySvcVideo();
					break;

				default:
					break;
			}

			// In CP version we create CP default profiles:
			CreateDefaultProfile_FactoryVideo();
		}
		CreateDefaultConfig();
	}

	if (!isCOPdongleSysMode)
	{
		switch (curProductType)
		{
			case eProductTypeNinja:
			case eProductTypeSoftMCU:
			case eProductTypeGesher:
			case eProductTypeEdgeAxis:
			case eProductTypeSoftMCUMfw:
				break;

			default:
			{
				m_isNeedToAddGWDefaults = (!m_isProfilesFolderEmpty) ? IsNeedToAddGWDefaults() : true;

				if (m_isNeedToAddGWDefaults)          // in case it is upgrade from version without the GW feature - add the default GW profile to the list
					CreateDefaultProfile_FactoryGW();
				else if (IsNeedToUpdateGWDefaults())  // update case - there is default GW profile but with wrong params)
					UpdateGWDefaultProfile();

				break;
			}
		}
	}

	//As MPMx card does not support "Echo Suppression" and "Keyboard Noise Suppression" we should disable these flags in profiles
	if (!IsFeatureSupportedBySystem(eFeatureEchoSuppression))
	{
		::GetpProfilesDB()->ForEachDisableEchoSuppression();
	}

	if (!IsFeatureSupportedBySystem(eFeatureKeyboardNoiseSuppression))
	{
		::GetpProfilesDB()->ForEachDisableKeyboardSuppression();
	}

	// update the Status Field
	::GetpProfilesDB()->ChkAllRsrvSysMode();
	::GetpMeetingRoomDB()->ChkAllRsrvSysMode();
	::GetpConfTemplateDB()->ChkAllRsrvSysMode();

	return STATUS_OK;
}

//=====================================================================================================================================//
STATUS CConfPartyManager::CreateDefaultProfile_FactoryVideo()
{
  PTRACE2(eLevelInfoNormal, "ConfPartyManager::CreateDefaultProfile_FactoryVideo - ProfileName: ", "Factory_Video_Profile");

  CCommRes* pProfile = ::GetpProfilesDB()->GetDefaultProfile("Factory_Video_Profile",FALSE,TRUE);
  // VSGNINJA-1002: EDGE>>V8.2.0.69>>Gathering mode is not enabled by default in the factory_video_profile.
  pProfile->SetGatheringEnabled(TRUE);
  pProfile->FillEmptyDiplayNameOrName();

  STATUS status = CreateProfile(pProfile);
  POBJDELETE(pProfile);
  return status;
}
//=====================================================================================================================================//
STATUS CConfPartyManager::CreateDefaultProfile_FactorySvcVideo()
{
  PTRACE2(eLevelInfoNormal, "ConfPartyManager::CreateDefaultProfile_FactorySvcVideo - ProfileName: ", "Factory_SVC_Video_Profile");

  CCommRes* pProfile = ::GetpProfilesDB()->GetDefaultProfile("Factory_SVC_Video_Profile");
  pProfile->FillEmptyDiplayNameOrName();
  pProfile->SetConfTransferRate( Xfer_1920 );            // 1920 rate: should be defined as SVC_DEFAULT_XFERRATE
  pProfile->SetConfMediaType( eSvcOnly );

  STATUS status = CreateProfile(pProfile);
  POBJDELETE(pProfile);
  return status;

}
//=====================================================================================================================================//
STATUS CConfPartyManager::CreateDefaultProfile_FactoryMixCpVideo()
{
  PTRACE2(eLevelInfoNormal, "ConfPartyManager::CreateDefaultProfile_FactoryMixCpVideo - ProfileName: ", "Factory_Mix_Video_Profile");

  CCommRes* pProfile = ::GetpProfilesDB()->GetDefaultProfile("Factory_Mix_Video_Profile",FALSE,TRUE);
  pProfile->FillEmptyDiplayNameOrName();
  pProfile->SetConfTransferRate(Xfer_1920);            // 1920 rate: should be defined as MIX_CP_DEFAULT_XFERRATE
  pProfile->SetConfMediaType( eMixAvcSvc );

  STATUS status = CreateProfile(pProfile);
  POBJDELETE(pProfile);
  return status;
}

//=====================================================================================================================================//
STATUS CConfPartyManager::CreateDefaultProfile_FactoryGW()
{
	PTRACE2(eLevelInfoNormal, "ConfPartyManager::CreateDefaultProfile_FactoryGW - ProfileName: ", "Factory_GW_Profile");

	CCommRes* pProfile = ::GetpProfilesDB()->GetDefaultProfile("Factory_GW_Profile", TRUE,TRUE);
  pProfile->FillEmptyDiplayNameOrName();

  STATUS status = CreateProfile(pProfile);
  POBJDELETE(pProfile);
  return status;
}
//=====================================================================================================================================//
STATUS CConfPartyManager::CreateDefaultProfile_Cop(int profileIndex)
{
  CopProfileInfo* pProfileInfo = CCOPConfigurationList::GetProfileInfo(profileIndex);
  std::string profileName = pProfileInfo->profileName;

  CCommRes* pProfile = ::GetpProfilesDB()->GetDefaultProfile(profileName);
  pProfile->FillEmptyDiplayNameOrName();
  pProfile->SetVideoSession(VIDEO_SESSION_COP);
  CLectureModeParams* pLectuteModeParam = pProfile->GetLectureMode();
  if (pLectuteModeParam)
    pLectuteModeParam->SetLectureModeType(0);
  pProfile->SetIsSameLayout(YES);
  pProfile->SetIsAutoLayout(YES);

  pProfile->SetUseYUVcolor(YES);          //UseYUV color
  pProfile->SetBackgroundImageID(0);      //Old skin
  pProfile->SetBckgColorYUV(81, 144, 119);//Old skin color
  pProfile->SetCopConfigurationList(new CCOPConfigurationList(profileIndex));

  STATUS status = CreateProfile(pProfile);
  POBJDELETE(pProfile);
  return status;
}
//=====================================================================================================================================//
void CConfPartyManager::UpdateGWDefaultProfile()
{
	CMedString cstr;
  cstr << "CConfPartyManager::UpdateGWDefaultProfile";
  if (!::GetpProfilesDB()->IsNameExist("Factory_GW_Profile"))
  {
    cstr << "Factory_GW_Profile not found in DB -> add it";
    PTRACE(eLevelInfoNormal,cstr.GetString());
    CreateDefaultProfile_FactoryGW();
    return;
  }

  CCommRes * pOldGwProfile = ::GetpProfilesDB()->GetCurrentRsrv("Factory_GW_Profile");
  CCommRes * pNewGwProfile = ::GetpProfilesDB()->GetDefaultProfile("Factory_GW_Profile", TRUE,TRUE);
  if(pOldGwProfile == NULL || pNewGwProfile == NULL)
  {
	  PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateGWDefaultProfile, Old or new GW profiles are NULL, exit function");
	  delete pNewGwProfile;
	  return;
  }
  pNewGwProfile->SetMonitorConfId(pOldGwProfile->GetMonitorConfId());
  pNewGwProfile->FillEmptyDiplayNameOrName();
  STATUS status = pNewGwProfile->TestReservValidityOfCommonParams();
  if (STATUS_OK == status)
    status = pNewGwProfile->TestUpdateValidity();

  if (STATUS_OK != status)
  {
    cstr << "CConfPartyManager::UpdateGWDefaultProfile Failed on validation of: " << pNewGwProfile->GetName() << " , status = " << (CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
    PASSERTMSG(status,cstr.GetString());
    AddActiveAlarm(FAULT_GENERAL_SUBJECT, AA_CAN_NOT_CREATE_DEFAULT_PROFILE, MAJOR_ERROR_LEVEL, "Failed to validate the Default GW Profile", true, true);
    POBJDELETE(pNewGwProfile);
    POBJDELETE(pOldGwProfile);
    return;
  }

  status = UpdateProfile(pNewGwProfile);
  if (STATUS_OK != status)
  {
    cstr << "CConfPartyManager::UpdateGWDefaultProfile Failed on updating of: " << pNewGwProfile->GetName() << " , status = " << (CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
    PASSERTMSG(status,cstr.GetString());
    AddActiveAlarm(FAULT_GENERAL_SUBJECT, AA_CAN_NOT_CREATE_DEFAULT_PROFILE, MAJOR_ERROR_LEVEL, "Failed to update the Default GW Profile", true, true);
    POBJDELETE(pNewGwProfile);
    POBJDELETE(pOldGwProfile);
    return;
  }

  cstr << " - Default GW profile has been updated (OK)";
  PTRACE(eLevelInfoNormal,cstr.GetString());
  POBJDELETE(pNewGwProfile);
  POBJDELETE(pOldGwProfile);
}
//=====================================================================================================================================//
void CConfPartyManager::CreateDefaultsGWSessionAndSendToRsrc()
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::CreateDefaultsGWSessionAndSendToRsrc");

	DWORD GwRsrvId = 0xFFFFFFFF;
	DWORD GwDefaultNid = 2000;
	CMedString cstr;
	STATUS status = STATUS_OK;
	BYTE generateFault = TRUE;
	BYTE onlyWarning = FALSE;
	CMedString faultDescription;


	CCommRes * pNewGwProfile = ::GetpProfilesDB()->GetCurrentRsrv("Factory_GW_Profile");
	if (pNewGwProfile)
	{
		//Create a new GW Profile reservation
		CCommRes* pTmp = ::GetpProfilesDB()->GetDefaultGW("Default_GW_Session",pNewGwProfile->GetMonitorConfId(),GwRsrvId,GwDefaultNid);
		if (pTmp)
		{
			pTmp->FillEmptyDiplayNameOrName();
			status = pTmp->TestAddValidity();

			if ( STATUS_OK == status)
			{
				if (pTmp->GetInternalConfStatus() != STATUS_OK)
				{
					faultDescription << "WARNING! " << CProcessBase::GetProcess()->GetStatusAsString(pTmp->GetInternalConfStatus()).c_str();
					onlyWarning = TRUE;
				}
				status = ::GetpMeetingRoomDB()->Add(*pTmp);
				if ( STATUS_OK == status)
				{
					generateFault = FALSE;
					PREFERRED_NUMERIC_ID_S preferedNumericIdStruct;
					memset(&preferedNumericIdStruct,0,sizeof(PREFERRED_NUMERIC_ID_S));
					preferedNumericIdStruct.monitor_Id = pTmp->GetMonitorConfId();
					strncpy(preferedNumericIdStruct.numeric_Id,pTmp->GetNumericConfId(),NUMERIC_CONF_ID_MAX_LEN);
					preferedNumericIdStruct.conf_type = eMR_type;
					strncpy(preferedNumericIdStruct.conf_name,pTmp->GetName(),H243_NAME_LEN);
					CSegment* pRASeg = new CSegment;
					pRASeg->Put((BYTE*)&preferedNumericIdStruct,sizeof(PREFERRED_NUMERIC_ID_S));
					//Send ASynch Msg to the RA process
					SendAsyncMsgToRsrcProcess(pRASeg,RESOURCE_NUMERIC_ID_REQ);
				}
				else
				{
					faultDescription << "Failed on Adding: " << pTmp->GetName() << "(status = " << (CProcessBase::GetProcess()->GetStatusAsString(status).c_str()) << ")";
					onlyWarning = FALSE;
				}
			}
			else
			{

				faultDescription << "Failed on validation of: " << pTmp->GetName() << "(status = " << (CProcessBase::GetProcess()->GetStatusAsString(status).c_str()) << ")";

			}
			POBJDELETE(pTmp);
		}
		else
		{
			faultDescription << "Internal Error - Failed getting Default_GW_Session from local DB";
			PASSERT(1);
		}
		POBJDELETE(pNewGwProfile);
	}
	else
	{
		faultDescription << "Internal Error -  can't find Factory_GW_Profile in DB ";
		PASSERT(2);
	}

	if (generateFault)
	{
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,ILLEGAL_CONFERENCE_PARAMETERS,MAJOR_ERROR_LEVEL,faultDescription.GetString(),FALSE);
		PTRACE2(eLevelError,"CConfPartyManager::CreateDefaultsGWSessionAndSendToRsrc Failed adding default gw profile -  ",faultDescription.GetString());
	}
	else if (onlyWarning)
	{
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,ILLEGAL_CONFERENCE_PARAMETERS,MAJOR_ERROR_LEVEL,faultDescription.GetString(),FALSE);
		PTRACE2(eLevelError,"CConfPartyManager::CreateDefaultsGWSessionAndSendToRsrc ",faultDescription.GetString());
	}

}
/////////////////////////////////////////////////////////////////////////////
BOOL CConfPartyManager::IsNeedToAddGWDefaults(BYTE checkIfNeedToUpdate)
{
	BOOL isSystemAfterUpgrade;// = IsFileExists(SYSTEM_AFTER_VERSION_INSTALLED_FILE);
	BOOL ans = false;
	string runningVer, currentVer, fallbackVer, factoryVer;
	BOOL getVersionsVal = GetVersions(runningVer, currentVer, fallbackVer, factoryVer);
	isSystemAfterUpgrade = IsFileExists(SYSTEM_AFTER_VERSION_INSTALLED_FILE);
	if (!getVersionsVal)
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::IsNeedToAddGWDefaults - simulation return false");
		return false;
	}


	/* debug code - simulate an upgrade

		currentVer = "test ignore";
		fallbackVer ="RMX_4.1.0.19";
		isSystemAfterUpgrade = true;
	*/

	CMedString infoStr = "CConfPartyManager::IsNeedToAddGWDefaults -";
	infoStr << "\nCurrent:  " << currentVer.c_str()
	        << "\nFallback: " << fallbackVer.c_str();

	int versionDetails[4];

	for(int i = 0; i < 4; i++)
		versionDetails[i] = 0;


	size_t index = fallbackVer.find_first_of("1234567890");
	int i = 0;
	while (index!=string::npos && i < 4)
	{
		versionDetails[i]=atoi(&fallbackVer[index]);;
		index = fallbackVer.find_first_of("1234567890", index+1);
		i++;
	}

	int major = versionDetails[0];
	int minor = versionDetails[1];
	int release = versionDetails[2];
	int internal = versionDetails[3];

	infoStr << "\nmajor: " << major
			<< "\nminor: " << minor
			<< "\nrelease: " << release
			<< "\ninternal: " << internal;

	if (major < 4)
		ans = true;
	else if (major == 4 && (minor == 0 || minor == 5))
		ans = true;
	else if (major == 4 && minor == 1 && release == 0)
	{
		if( internal < 9 /*??*/)
			ans = true;
		else if (checkIfNeedToUpdate && internal < 24)
		{
			infoStr << "\nFallback ver > 9 but < 24 - UPDATE the default profile!";
			ans = true;
		}
	}



	infoStr << "\nIs Fallback ver < 4.1 ? " << (ans? "TRUE" : "FALSE");

	infoStr << "\nIs system after upgrade? " << (isSystemAfterUpgrade? "TRUE" : "FALSE");

	ans &= isSystemAfterUpgrade;

	PTRACE(eLevelInfoNormal,infoStr.GetString());
	return ans;

}
/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::verifyExitFromStartUpAndPerformAfterStartUpActions()
{
	 eTaskState taskState = GetTaskState();
	 const char *taskStateName = GetTaskStateName(taskState);
	 if(eTaskStateStartup == taskState)
	 {
			PTRACE2(eLevelInfoNormal,"CConfPartyManager::verifyExitFromStartUpAndPerformAfterStartUpActions, ConfParty process did not exits StartUp, Current Task State:",taskStateName);
			return;
	 }

	 if(m_isStartupReadMRDBReqRecieved && m_isSystemCardsModeReceived)
	 {
		 PTRACE(eLevelInfoNormal,"CConfPartyManager::verifyExitFromStartUpAndPerformAfterStartUpActions, start validate DB elements");
		 // Romem - 1.02.07 - Test Validity of DB elements
		 CCommResDB * pProfileDB = GetpProfilesDB();
		 CCommResDB *  pMrDB = GetpMeetingRoomDB();
		 CCommResDB *  pConfTemplateDB = GetpConfTemplateDB();
		 pProfileDB->TestValidityOfDBElementsParms();
		 pMrDB->TestValidityOfDBElementsParms();
		 pConfTemplateDB->TestValidityOfDBElementsParms();

		 PTRACE(eLevelInfoNormal,"CConfPartyManager::verifyExitFromStartUpAndPerformAfterStartUpActions, end validate DB elements");

		 // Olga - read a resolution slider configuration
		 eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
		 CResRsrcCalculator resRsrcCalculator;
		 resRsrcCalculator.DumpDynamicThresholdTbl(systemCardsBasedMode);
		 resRsrcCalculator.DumpDefaultThresholdTbl(systemCardsBasedMode);

		 STATUS status = resRsrcCalculator.ReadResolutionConfigurationFromFile( systemCardsBasedMode );
		 if( status != STATUS_OK )
		 {
			 status = resRsrcCalculator.SaveResolutionConfigurationToFile( systemCardsBasedMode, TRUE );
			 if( status != STATUS_OK )
				 PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::verifyExitFromStartUpAndPerformAfterStartUpActions : ConfParty didn't success to read & write Resolution Slider Configuration file, status = ", status);
		 }

		 OFF(m_isStartupReadMRDBReqRecieved);
		 SendNumericIdListToRsrcProcess();
		 StartTimer(CONFPARTY_IS_MCU_STARTUP_FINISHED_TIMER, 5 * SECOND);
	}
}

void CConfPartyManager::OnTimerIsMcuStartupFinished(CSegment*)
{
    CProcessBase* proc = CProcessBase::GetProcess();
    PASSERT_AND_RETURN(NULL == proc);

    eMcuState state = proc->GetSystemState();
	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnTimerIsMcuStartupFinished - MCU state = ", GetMcuStateName(state));

    if (state != eMcuState_Invalid && state != eMcuState_Startup)
    {
    	DeleteTimer(CONFPARTY_IS_MCU_STARTUP_FINISHED_TIMER);
    	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnTimerIsMcuStartupFinished - Startup is finished");
    	CreateOngoingConfStore();

        StartTimer(CONFPARTY_DELETE_USELESS_IVR_FILES_TIMER, 60 * 60 * SECOND);
    }
    else
    {
    	StartTimer(CONFPARTY_IS_MCU_STARTUP_FINISHED_TIMER, 5 * SECOND);
    	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnTimerIsMcuStartupFinished - Restart timer");
    }
}

/*
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleTerminalRestoreDefault(CTerminalCommand & command, std::ostream& answer)
{
 //  DWORD numOfParams = command.GetNumOfParams();

//   //Make sure we do not have any ongoing conferences
//   if (0 != ::GetpConfDB()->GetConfNumber())
//     {
//       answer <<"Error: Can not restore deafaults, still having  " << ::GetpConfDB()->GetConfNumber() << " Ongoing conferences" ;
//       return STATUS_FAIL;
//     }

//   //Arase active alarm restoring factory defaults
//   AddActiveAlarm(FAULT_GENERAL_SUBJECT
// 		 ,RESTORING_DEFAULT_FACTORY
// 		 ,MAJOR_ERROR_LEVEL,
// 		 "Restoring Factory Defaults",
// 		 true,
// 		 false
// 		 );

//   answer << "Creating the file: " << DEFAULT_RESTORE_FILE;
//   CreateFile(DEFAULT_RESTORE_FILE);

  return STATUS_OK;
}
*/
/*
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnReservationUpdateAction(CRequest* pRequest)
{

	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnReservationUpdateAction :  Got Action Update on conf ");
	STATUS status = STATUS_OK;
	CCommRes * pRsrv= new CCommRes;
	CCommResAdd* pCommResAdd = new CCommResAdd;

	*pCommResAdd = *(CCommResAdd*)pRequest->GetRequestObject() ;
	*pRsrv= *(pCommResAdd->GetCommRes());

	if (pRsrv->IsTemplate ())
		status=OnReservationUpdateProfile(pRsrv);
	else if (pRsrv->IsMeetingRoom() )
		status=OnReservationUpdateMR(pRsrv);
	//else update Conf

	if (status != STATUS_OK)
	{
		PTRACE(eLevelError, "CConfPartyManager::OnReservationUpdateAction :  Update Action Failed");
		PASSERT(1);
		POBJDELETE(pCommResAdd);
		POBJDELETE(pRsrv);
		return status;
	}

	pCommResAdd->SetCommRes(pRsrv);

	std::string responseTrancsName("TRANS_RES"); //insteade of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);

	pRequest->SetConfirmObject(pCommResAdd);

	POBJDELETE(pRsrv);
 	return status;
}


*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//void CConfPartyManager::OnLobbyActivateMR(CSegment* pParam)
//{
//
//	char    adHocConfName[H243_NAME_LEN];
//	char    adHocNumericID[NUMERIC_CONFERENCE_ID_LEN];
//
//   	CCommRes* pMRRsrv = new CCommRes;
//
//
//	*pParam >> Name
//	        >> adHocNumericID;
//
//	CCommRes* pEntryQueueConf = ::GetpConfDB()->GetCurrentConf(dwSourceConfId);
//
//	if (pEntryQueueConf && pEntryQueueConf->GetAdHoc())
//		dwAdHocProfileId = pEntryQueueConf->GetAdHocProfileId();
//
//	else
//	 PASSERT(1); // set Good treatment
//
//
////	PTRACE2(eLevelInfoNormal, "CRsrvMngr::OnLobyStartAdHocConf,  profile  ", dwAdHocProfileId);
//	CLobbyApi* pLobbyApi= ((CLobbyApi*)::GetpLobbyApi()) ;
//	//to check DB pointer
//	CCommResApi* pProfile = (CCommResApi*)::GetpProfilesDB()->GetCurrentRsrv(dwAdHocProfileId);
//	if (pProfile)
//	{
//		pAdHocRsrv->SetMyProfileBasedParams(pProfile);
//		pAdHocRsrv->SetAdHocConfParams(adHocConfName, adHocNumericID, dwAdHocProfileId);
//	}
//	else
//	{
//	   //Send reject to Lobby - Start ADHOC  failed
//		if (!CPObject::IsValidPObjectPtr(pProfile))
//		{
//			PTRACE(eLevelInfoNormal, "CConfPartyManager::OnLobbyStartMeetingRoom Reject to Lobby, can not find conf in MR DB");
//    		//pLobbyApi->RejectStartAdHoc(mrId/*, undefId,*/ ,STATUS_ILLEGAL);
//			return;
//		}
//	}
//	//status = CheckRsrvValidity(prsrvfromprofile);
//
//	STATUS status = STATUS_OK;
//	if(STATUS_OK == status)
//	{
//		OPCODE opcode = START_AD_HOC_CONF_REQ;
//		CSegment *pToRsrcSeg = new CSegment;
//
//		pAdHocRsrv->Serialize(NATIVE, *pToRsrcSeg);
//		SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, opcode);
//
//	}
////  else //validity fail
////	{
////	  PTRACE(eLevelInfoNormal, "CConfPartyManager::OnLobbyStartMeetingRoom Reject to Lobby, can not find conf in MR DB");
////    pLobbyApi->RejectStartMeetingRoom(mrId/*, undefId,*/ ,STATUS_ILLEGAL);
////	  return;
////	}
////
//
// POBJDELETE(pAdHocRsrv);
//
//}

///////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleTerminalSendDtmf(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleTerminalSendDtmf ");

	DWORD numOfParams = command.GetNumOfParams();
	if(3 != numOfParams)
	{
		answer << "error: action must be specified: SendDtmf\n";
///		answer << "usage: Bin/McuCmd SendDtmf ConfParty [Conf name] [Party name] [DTMF string]\n";
		answer << "usage: Bin/McuCmd SendDtmf ConfParty [Conf name] [Party ID] [DTMF string]\n";
		return STATUS_FAIL;
	}

	const string &confName = command.GetToken(eCmdParam1);
///	const string &partyName = command.GetToken(eCmdParam2);
	const string &partyRsrcId = command.GetToken(eCmdParam2);
	const string &dtmfString = command.GetToken(eCmdParam3);

///	answer <<  "Conf: " << confName << "Party: " << partyRsrcId << "DTMF String: " << dtmfString;

	// Find Conf in DB
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = NULL;

	pRequestedConf = pCommConfDB->GetCurrentConf(confName.c_str());
	if (!CPObject::IsValidPObjectPtr(pRequestedConf))
	{
		answer << "error: Conf does not exist in DB " <<  confName << " " << "SendDtmf ";
		return STATUS_FAIL;
	}
//	if (pCommConfDB->SearchPartyName(confName.c_str(), partyName.c_str()) != STATUS_OK)
//	{
//		answer << "error: Party does not exist in DB " <<  confName << " " << partyName << " " << "SendDtmf ";
//		return STATUS_FAIL;
//	}

///	DWORD partyId = pCommConfDB->GetPartyId(confName.c_str(), partyName.c_str());

	// Send Event to CAM
	CConfApi* pConfApi = new CConfApi;
	pConfApi->CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));

//	CSegment* Command = new CSegment;
//	*Command << partyName;
	pConfApi->SendDtmfFromParty(atoi(partyRsrcId.c_str()), dtmfString.c_str());

	POBJDELETE(pConfApi);

	return STATUS_OK;

}




//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerRecordingCommand(CRequest *pRequest)
{
 	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerRecordingCommand");

    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
        FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerRecordingCommand: No permission to OnServerRecordingCommand for administrator readony");
        pRequest->SetConfirmObject(new CDummyEntry());
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_NO_PERMISSION;
    }
	STATUS status = STATUS_OK;

	CConfAction* pConfAction = new CConfAction;

   *pConfAction = *(CConfAction*)pRequest->GetRequestObject() ;
    pRequest->SetObjectFlag(STRING_FLAG);  ///????

    const DWORD confId = pConfAction->GetConfID();

  	/*** VALIDITY of conference Id ***/
  	if (::GetpConfDB()->FindId(confId) == NOT_FIND)
		  status = STATUS_CONF_NOT_EXISTS;

 	 if (status==STATUS_OK)
  	{
  		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);

		if (pCurConf && status==STATUS_OK)
		{
			if (pCurConf->GetEnableRecording())
			{
			DWORD partyRsrcID = (DWORD)(-1);	// not relevant
			DWORD confOrPartyAction = EVENT_CONF_REQUEST;
			const DWORD opcode = pConfAction->GetRecordingCommand();
			DWORD param = (DWORD)(-1);			// not in use here

			CConfApi confApi;
			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			confApi.SendCAMGeneralActionCommand(partyRsrcID, confOrPartyAction, opcode, param);
			confApi.DestroyOnlyApi();
			}
			else
			{
				PTRACE(eLevelError, "CConfPartyManager::OnServerRecordingCommand : Recording is not enabled for this conference");
				status = STATUS_RECORDING_DISABLED_IN_CONF;
			}
	 	}
		else
	 	{
	  		PTRACE(eLevelError,  "CConfPartyManager::OnServerRecordingCommand :  Conf Id invalid" );
	  		DBGPASSERT(1);
	  		status = STATUS_ILLEGAL;
	 	}
  	}

  	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerRecordingCommand, ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

  	//New Confirm
 	std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_CONF_2
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);

	return STATUS_OK;

}
//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerUpdateConfVideoClarity(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateConfVideoClarity");
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
	{
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateConfVideoClarity: No permission to OnServerUpdateConfVideoClarity for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
	}
	STATUS status = STATUS_OK;
	CConfAction* pConfAction = new CConfAction;
	*pConfAction = *(CConfAction*)pRequest->GetRequestObject() ;
	pRequest->SetObjectFlag(STRING_FLAG);

	const DWORD confId   = pConfAction->GetConfID();
	BYTE isVideoClarityEnabled   = (pConfAction->GetNumAction())?YES:NO;

	/*** VALIDITY of conference Id ***/
	// Romem - clokwork
	//if (::GetpConfDB()->FindId(confId) == NOT_FIND)
	//	  status = STATUS_CONF_NOT_EXISTS;
	CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
	if(pCurConf == NULL)
		status = STATUS_CONF_NOT_EXISTS;

	if((status==STATUS_OK))
	{
		if(::GetpConfDB()->IsConfSecured(confId))  //check both cases if in that way and if in second way
	    {
			TRACESTR (eLevelError) <<"ConfPartyManager::OnServerUpdateConfVideoClarity: The Secured conference can not be changed ";
			status=STATUS_CONF_IS_SECURED;
	    }
	}
	if (status==STATUS_OK)
	{
		//VNGR-11671
		/*** VALIDITY of VideoClarity ***/
		if (YES == isVideoClarityEnabled)
		{
			eVideoQuality rsrvVidQuality = pCurConf->GetVideoQuality();
			if (eVideoQualityMotion == rsrvVidQuality)
			{
				pCurConf->SetIsVideoClarityEnabled(NO);
				isVideoClarityEnabled = NO;
				PTRACE(eLevelInfoNormal,"ConfPartyManager::OnServerUpdateConfVideoClarity - Video clarity was disabled since Video quality is motion");
			}
		}
	    //VNGR-11671
		if(pCurConf->GetIsHDVSW())
		{
			PTRACE(eLevelError,  "CConfPartyManager::OnServerUpdateConfVideoClarity: Video Clarity is Enabled is CP calls only" );
			DBGPASSERT(1);
			status = STATUS_ILLEGAL;
		}
		eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
		if(status==STATUS_OK && systemCardsBasedMode==eSystemCardsMode_mpm)
		{
			PTRACE(eLevelError,  "CConfPartyManager::OnServerUpdateConfVideoClarity: Video Clarity is not Enabled is MPM system cards mode");
			DBGPASSERT(1);
			status = STATUS_ILLEGAL;
		}
		if( status==STATUS_OK && (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily()) )
		{
			PTRACE(eLevelError,  "CConfPartyManager::OnServerUpdateConfVideoClarity: Video Clarity is not Enabled is Soft MCU mode");
			DBGPASSERT(1);
			status = STATUS_ILLEGAL;
		}
		if (status==STATUS_OK)
		{
			CConfApi confApi;
			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			confApi.UpdateVideoClarity(isVideoClarityEnabled);
			confApi.DestroyOnlyApi();
		}
		else
		{
			PTRACE(eLevelError,  "CConfPartyManager::OnServerUpdateConfVideoClarity :  Conf Id invalid" );
			DBGPASSERT(1);
			status = STATUS_ILLEGAL;
		}
	}

	//Trace
	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateConfVideoClarity ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());


  	std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetMessageOverLay(CRequest *pRequest)
{

	PTRACE(eLevelError,  "CConfPartyManager::OnServerSetMessageOverLay " );
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetMessageOverLay: No permission to OnServerSetMessageOverLay for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
    }

	STATUS status = STATUS_OK;

	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	if (systemCardsBasedMode == eSystemCardsMode_mpm)
	{
		PTRACE(eLevelError,  "CConfPartyManager::OnServerSetMessageOverLay message overlay is not supported on MPM mode!!!" );
		return STATUS_ILLEGAL_IN_MPM_MODE;
	}


	CMessageOverlayInfoDrv* pMessageOverlayInfoDrv = new CMessageOverlayInfoDrv;

	*pMessageOverlayInfoDrv = *(CMessageOverlayInfoDrv*)pRequest->GetRequestObject() ;

	CMessageOverlayInfo* pMessageOverlayInfo = pMessageOverlayInfoDrv->GetMessageOverlayInfo();

	const DWORD confId   = pMessageOverlayInfoDrv->GetConfID();

	// klocwork - Romem
	//if (::GetpConfDB()->FindId(confId) == NOT_FIND)
	//	  status = STATUS_CONF_NOT_EXISTS;
	CCommConf* pConf = ::GetpConfDB()->GetCurrentConf(confId);
	if(!pConf)
		status = STATUS_CONF_NOT_EXISTS;

	if (status==STATUS_OK)
	{
		// Romem klocwork
		//CommConf* pConf = ::GetpConfDB()->GetCurrentConf(confId);
		pConf->SetMessageOverlay(pMessageOverlayInfo);
		CConfApi confApi;
		confApi.CreateOnlyApi(*(pConf->GetRcvMbx()));
		confApi.UpdateMessageOverlay(pMessageOverlayInfo);
		confApi.DestroyOnlyApi();
	}

	std::string responseTrancsName("TRANS_CONF_2");
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pMessageOverlayInfoDrv);
	pRequest->SetStatus(status);

	return STATUS_OK;

}
//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetPartyMessageOverLay(CRequest *pRequest)
{

	PTRACE(eLevelError, "CConfPartyManager::OnServerSetPartyMessageOverLay " );
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetPartyMessageOverLay: No permission to OnServerSetPartyMessageOverLay for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
    }

	STATUS status = STATUS_OK;

	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	if (systemCardsBasedMode == eSystemCardsMode_mpm) {
		PTRACE(eLevelError, "CConfPartyManager::OnServerSetPartyMessageOverLay message overlay is not supported on MPM mode!!!" );
		return STATUS_ILLEGAL_IN_MPM_MODE;
	}


	CMessageOverlayInfoPartyDrv* pMessageOverlayInfoPartyDrv = new CMessageOverlayInfoPartyDrv;

	*pMessageOverlayInfoPartyDrv
				= *(CMessageOverlayInfoPartyDrv*) pRequest->GetRequestObject();


	CMessageOverlayInfo* pMessageOverlayInfo =
			pMessageOverlayInfoPartyDrv->GetMessageOverlayInfo();

	const DWORD confId = pMessageOverlayInfoPartyDrv->GetConfID();
	const DWORD partyId = pMessageOverlayInfoPartyDrv->GetPartyID();
	const WORD isPrivate = pMessageOverlayInfoPartyDrv->GetIsPrivate();


	/*** VALIDITY of conferenceId and partyId ***/
	const CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
	CConfParty* pParty = (CConfParty*) ::GetpConfDB()->GetCurrentParty(confId,
			partyId);
	const char* pPartyName = pParty->GetName();

	if (NULL == pCommConf)
		status = STATUS_CONF_NOT_EXISTS;

	if (NULL == pParty)
		status = STATUS_PARTY_DOES_NOT_EXIST;

	BYTE isHDVSW = FALSE;
	if (STATUS_OK == status)
	{
		isHDVSW = pCommConf->GetIsHDVSW();
		if(isHDVSW) // No Message Overlay in VSW conference
		{
			//Maybe status need to be changed
			status = STATUS_ILLEGAL_OPERATION_VIDEO_CAPABILITIES_INACTIVE;
		}
		else if (pMessageOverlayInfo)
		{
			if ((pParty->GetPartyState() == PARTY_SECONDARY)
					|| (pParty->GetVoice()) || (pParty->GetPartyState()
					== PARTY_DISCONNECTED) || (pParty->GetPartyState()
					== PARTY_REDIALING)) {
				status = STATUS_ILLEGAL_OPERATION_VIDEO_CAPABILITIES_INACTIVE;
			}
		}
		else
			status = STATUS_INCONSISTENT_PARAMETERS;
	}

	if (status == STATUS_OK)
	{
		// Romem klocwork
		//CCommConf* pConf = ::GetpConfDB()->GetCurrentConf(confId);

		CConfApi confApi;
		confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
		confApi.UpdateMessageOverlayParty(pPartyName, pMessageOverlayInfo);
		confApi.DestroyOnlyApi();
	}

	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerSetPartyMessageOverLay, "
			, CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
	std::string responseTrancsName("TRANS_CONF_2");
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pMessageOverlayInfoPartyDrv);
	pRequest->SetStatus(status);

	return STATUS_OK;

}
STATUS CConfPartyManager::OnServerUpdateAutoScanInterval(CRequest *pRequest)
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
          FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateAutoScanInterval: No permission to OnServerUpdateAutoScanInterval for administrator readony");
          pRequest->SetConfirmObject(new CDummyEntry());
          pRequest->SetStatus(STATUS_NO_PERMISSION);
          return STATUS_NO_PERMISSION;
    }
	STATUS status = STATUS_OK;
	CConfAction* pConfAction = new CConfAction;
	*pConfAction = *(CConfAction*)pRequest->GetRequestObject() ;
	pRequest->SetObjectFlag(STRING_FLAG);

	const DWORD confId   = pConfAction->GetConfID();
	WORD intervalValue   = pConfAction->GetNumAction();
    PTRACE2INT (eLevelInfoNormal, "ConfPartyManager::OnServerUpdateAutoScanInterval ", intervalValue);
    //ToDo - add test validity for the intervalValue
	/*** VALIDITY of conference Id ***/
	if (::GetpConfDB()->FindId(confId) == NOT_FIND)
		  status = STATUS_CONF_NOT_EXISTS;

	const CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);

	if (NULL == pCommConf)
	{
		status = STATUS_CONF_NOT_EXISTS;
	}

	if (status == STATUS_OK)
	{
		if (pCommConf->GetManageTelepresenceLayoutInternaly())
		{
			TRACEINTO << "Can't start feature because layout is managed internally in telepresence room switch mode";
			status = STATUS_CONFERENCE_LAYOUT_IS_MANAGED_INTERNALLY ;
			pRequest->SetExDescription(CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		}
	}

	if (status==STATUS_OK)
	{
		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
		if (pCurConf)
		{
			pCurConf->SetAutoScanInterval(intervalValue);
            CConfApi confApi;
			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			confApi.SetAutoScanInterval(intervalValue);
			confApi.DestroyOnlyApi();

		}
	  else
	  {
		  PTRACE(eLevelError,  "CConfPartyManager::OnServerUpdateAutoScanInterval :  Conf Id invalid" );
		  DBGPASSERT(1);
		  status = STATUS_ILLEGAL;
	  }

	}
	else
	{
		//Trace
		PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateAutoScanInterval ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		DBGPASSERT(status);
	}

  	std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerUpdateAutoScanOrder(CRequest *pRequest)
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
          FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateAutoScanOrder: No permission to OnServerUpdateAutoScanOrder for administrator readony");
          pRequest->SetConfirmObject(new CDummyEntry());
          pRequest->SetStatus(STATUS_NO_PERMISSION);
          return STATUS_NO_PERMISSION;
    }
	STATUS status = STATUS_OK;
	CAutoScanOrderDrv* pAutoScanOrderDrv = new CAutoScanOrderDrv;
	*pAutoScanOrderDrv = *(CAutoScanOrderDrv*)pRequest->GetRequestObject() ;
	pRequest->SetObjectFlag(STRING_FLAG);

	const DWORD confId   = pAutoScanOrderDrv->GetConfID();
	CAutoScanOrder* pAutoScanOrder = pAutoScanOrderDrv->GetAutoScanOrder();

    PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerUpdateAutoScanOrder ");
	/*** VALIDITY of conference Id ***/
	if (::GetpConfDB()->FindId(confId) == NOT_FIND)
		  status = STATUS_CONF_NOT_EXISTS;

	if (status==STATUS_OK)
	{
		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
		if (pCurConf)
		{
            pCurConf->SetAutoScanOrder(pAutoScanOrder);
            CConfApi confApi;
			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			confApi.SetAutoScanOrder(pAutoScanOrder);
			confApi.DestroyOnlyApi();

		}
		else
		{
		  PTRACE(eLevelError,  "CConfPartyManager::OnServerUpdateAutoScanOrder :  Conf Id invalid" );
		  DBGPASSERT(1);
		  status = STATUS_ILLEGAL;
		}

	}
	else
	{
		//Trace
		PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateAutoScanOrder ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		DBGPASSERT(status);
	}

  	std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pAutoScanOrderDrv);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerUpdateConfAutoRedial(CRequest *pRequest)
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
          FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateConfAutoRedial: No permission to OnServerUpdateConfAutoRedial for administrator readony");
          pRequest->SetConfirmObject(new CDummyEntry());
          pRequest->SetStatus(STATUS_NO_PERMISSION);
          return STATUS_NO_PERMISSION;
    }

	STATUS status = STATUS_OK;
	CConfAction* pConfAction = new CConfAction;
	*pConfAction = *(CConfAction*)pRequest->GetRequestObject() ;
	pRequest->SetObjectFlag(STRING_FLAG);

	const DWORD confId   = pConfAction->GetConfID();
	BYTE isAutoRedial   = (pConfAction->GetNumAction())?YES:NO;
    PTRACE2INT (eLevelInfoNormal, "ConfPartyManager::OnServerUpdateConfAutoRedial ", isAutoRedial);
	/*** VALIDITY of conference Id ***/
	if (::GetpConfDB()->FindId(confId) == NOT_FIND)
		  status = STATUS_CONF_NOT_EXISTS;

	if (status==STATUS_OK)
	{
		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
		if (pCurConf&&status==STATUS_OK)
		{
            pCurConf->SetAutoRedial(isAutoRedial);
            CConfApi confApi;
			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			confApi.SetAutoRedial(isAutoRedial);
			confApi.DestroyOnlyApi();
		}
	  else
	  {
		  PTRACE(eLevelError,  "CConfPartyManager::OnServerUpdateConfAutoRedial :  Conf Id invalid" );
		  DBGPASSERT(1);
		  status = STATUS_ILLEGAL;
	  }

  }
	//Trace
	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerUpdateConfAutoRedial ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());


  	std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::SendAdditionalParams(CSegment* pSeg)
{
	if(!pSeg)
		return;
	*pSeg << m_dwInternalConfStatus;
	/* reset this flag because it is related to the current sent request only */
	m_dwInternalConfStatus = STATUS_OK;
}
//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerMoveParty(CRequest* pRequest)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerMoveParty ");
   if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerMoveParty: No permission to OnServerMoveParty for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
    }
	STATUS status = STATUS_OK;

	// desirialize xml parameters using CMoveAction class
	CMoveAction* pMoveAction = new CMoveAction;
	*pMoveAction = *(CMoveAction*)pRequest->GetRequestObject() ;
	pRequest->SetObjectFlag(STRING_FLAG);
	//	pMoveAction->Dump();
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnServerMoveParty");
	if (!::GetIsCOPdongleSysMode())
	{
	  // Find Source/Dest Conf in DB
	  CCommConf* pSourceRequestedConf = NULL;
	  CCommConf* pDestRequestedConf = NULL;
	  DWORD partyId = (DWORD)(-1);
	  status = GetMoveConfsFromRequest(pMoveAction,pSourceRequestedConf,pDestRequestedConf,partyId);

	  // start move action
	  if(status == STATUS_OK){
	    ResetOperatorAssistanceInDB(pSourceRequestedConf,pDestRequestedConf,partyId );
	    status = MoveActions(pSourceRequestedConf,pDestRequestedConf,partyId,eMoveDefault);
	  }
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerMoveParty MOVE IS BLOCKED IN RMX2000C!!!");
		status = STATUS_FAIL;
	}
  	// confirm to EMA
	std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pMoveAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerAttendParty(CRequest* pRequest)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerAttendParty ");
   if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerAttendParty: No permission to OnServerAttendParty for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
    }

	STATUS status = STATUS_OK;

	// desirialize xml parameters using CMoveAction class
	CMoveBaseAction* pMoveBaseAction = new CMoveBaseAction;
	*pMoveBaseAction = *(CMoveBaseAction*)pRequest->GetRequestObject() ;
	pRequest->SetObjectFlag(STRING_FLAG);
	//pMoveBaseAction->Dump();
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnServerAttendParty");
	if (!::GetIsCOPdongleSysMode())
	{
	// Find Source/Dest Conf in DB
	CCommConf* pSourceRequestedConf = NULL;
	CCommConf* pDestRequestedConf = NULL;
	DWORD partyId = (DWORD)(-1);
	status = GetMoveConfsFromRequest(pMoveBaseAction,pSourceRequestedConf,partyId);

	// start move action
	if(status == STATUS_OK){
	  status = GetUserOperatorConf(pDestRequestedConf);
	}



	// start move action
	if(status == STATUS_OK){
	  //reset operator assistance in db (ivr message will stop on move)
	  ResetOperatorAssistanceInDB(pSourceRequestedConf,pDestRequestedConf,partyId );
	  // move
	  status = MoveActions(pSourceRequestedConf,pDestRequestedConf,partyId,eMoveAttend);
	}
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerAttendParty MOVE IS BLOCKED IN RMX2000C!!!");
		status = STATUS_FAIL;
	}

  	// confirm to EMA
	std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pMoveBaseAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerMovePartyBackToHomeConf(CRequest* pRequest)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerMovePartyBackToHomeConf ");
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerMovePartyBackToHomeConf: No permission to OnServerMovePartyBackToHomeConf for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
    }

	STATUS status = STATUS_OK;

	// desirialize xml parameters using CMoveAction class
	CMoveBaseAction* pMoveBaseAction = new CMoveBaseAction;
	*pMoveBaseAction = *(CMoveBaseAction*)pRequest->GetRequestObject() ;
	pRequest->SetObjectFlag(STRING_FLAG);
	//pMoveBaseAction->Dump();
	if (!::GetIsCOPdongleSysMode())
	{
	// Find Source/Dest Conf in DB
	CCommConf* pSourceConf = NULL;
	CCommConf* pHomeConf = NULL;
	DWORD partyId = (DWORD)(-1);
	status = GetMoveConfsFromRequest(pMoveBaseAction,pSourceConf,partyId);

	// start move action
	if(status == STATUS_OK){
	  status = GetHomeConfForParty(pSourceConf,partyId,pHomeConf);
	}

	// start move action
	if(status == STATUS_OK){
	  status = MoveActions(pSourceConf,pHomeConf,partyId,eMoveBack);
	}
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerMovePartyBackToHomeConf MOVE IS BLOCKED IN RMX2000C!!!");
		status = STATUS_FAIL;
	}
  	// confirm to EMA
	std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pMoveBaseAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerIntraRequest(CRequest *pRequest)
{
   if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
   {
       FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerIntraRequest: No permission to OnServerIntraRequest for administrator readony");
        pRequest->SetConfirmObject(new CDummyEntry());
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_NO_PERMISSION;
   }
   STATUS status = STATUS_OK;

   CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;

   *pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject() ;
   pRequest->SetObjectFlag(STRING_FLAG);

   const DWORD confId   = pRsrvPartyAction->GetConfID();
   const DWORD partyId  = pRsrvPartyAction->GetPartyID();
   const WORD intraDitrection  = pRsrvPartyAction->GetNumAction();

	/*** VALIDITY of conferenceId and partyId ***/
	status = ::GetpConfDB()->SearchPartyName(confId,partyId);

	if (status == STATUS_OK)
	{
		  const char* pPartyName = NULL;
		  const CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
  	      if (pCommConf !=NULL)
  	      {
			  pPartyName = pCommConf->GetCurrentParty(partyId)->GetName();
			  if(pPartyName == NULL)
				  status = STATUS_PARTY_DOES_NOT_EXIST;
  	      }
		  else
			  status = STATUS_CONF_NOT_EXISTS;

		  if (status == STATUS_OK)
		  {
		  	CConfParty* pConfParty = NULL;
		  	pConfParty = pCommConf->GetCurrentParty(pPartyName);
		  	CConfApi confApi;
		 	confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
			confApi.VideoRefreshBeforeRecording(pPartyName,intraDitrection);
			confApi.DestroyOnlyApi();
		  }
	  }

  PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerIntraRequest, ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

    ///new confirm
	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pRsrvPartyAction);
	pRequest->SetStatus(status);
	//end new confirm

  return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::GetMoveConfsFromRequest(CMoveAction* pMoveAction,CCommConf*& pSourceRequestedConf,CCommConf*& pDestRequestedConf,DWORD& partyId)
{

  DWORD sourceConfID = (DWORD)(-1);
  DWORD TargetConfID = (DWORD)(-1);

  // get IDs from CMoveAction
  if(pMoveAction){
    sourceConfID = pMoveAction->GetSourceConfID();
    TargetConfID = pMoveAction->GetTargetConfID();
    partyId = pMoveAction->GetPartyID();
  }else{
    PASSERT(102);
    return STATUS_ILLEGAL_MOVE_PARTY_NOT_FOUND;;
  }

  if(sourceConfID == TargetConfID){
    PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::GetMoveConfsFromRequest: sourceConfID == TargetConfID = ",sourceConfID);
    return STATUS_ILLEGAL_MOVE_DEST_CONF_NOT_FOUND;
  }


  // get Confs from DB
  CCommConfDB* pCommConfDB = ::GetpConfDB();
  pSourceRequestedConf = pCommConfDB->GetCurrentConf(sourceConfID);
  pDestRequestedConf = pCommConfDB->GetCurrentConf(TargetConfID);

  COstrStream trace_str;
  // validate source conf
  if(!CPObject::IsValidPObjectPtr(pSourceRequestedConf)){
      trace_str << " Source Conf does not exist in DB, Source_Conf_Id = " << sourceConfID;
      PTRACE2(eLevelInfoNormal,"CConfPartyManager::GetMoveConfsFromRequest: ",trace_str.str().c_str());
      return STATUS_ILLEGAL_MOVE_PARTY_NOT_FOUND;
  }
  // validate party name in source conf
  const char* party_name = NULL;
  party_name = pCommConfDB->GetPartyName (sourceConfID,partyId);
  if(party_name==NULL){
    trace_str << " Party with id = " << partyId << " could not be found in source conf - " << pSourceRequestedConf->GetName();
      PTRACE2(eLevelInfoNormal,"CConfPartyManager::GetMoveConfsFromRequest: ",trace_str.str().c_str());
      return STATUS_ILLEGAL_MOVE_PARTY_NOT_FOUND;
  }
	CConfParty* pSourceParty = pSourceRequestedConf->GetCurrentParty(partyId);
	if( pSourceParty && pSourceParty->GetIsTipCall() && !pSourceRequestedConf->GetEntryQ() )
	{
		PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::GetMoveConfsFromRequest: party can't be moved from TIP conf, partyId=",partyId);
		return STATUS_ILLEGAL_MOVE_TIP_PARTY;
	}
  // validate destination conf
  if(!CPObject::IsValidPObjectPtr(pDestRequestedConf)){
      trace_str << " Destination Conf does not exist in DB, TargetConfID = " << sourceConfID;
      PTRACE2(eLevelInfoNormal,"CConfPartyManager::GetMoveConfsFromRequest: ",trace_str.str().c_str());
      return STATUS_ILLEGAL_MOVE_DEST_CONF_NOT_FOUND;
  }
	if( pSourceRequestedConf->GetIsTipCompatible() != pDestRequestedConf->GetIsTipCompatible() )
	{
		PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::GetMoveConfsFromRequest: party can't be moved to TIP conf, partyId=",partyId);
		return STATUS_MOVE_TELEPRESENCE_NON_TELEPRESENCE_IS_FORBIDDEN;
	}

  trace_str << " move party  " << party_name << " from conf " << pSourceRequestedConf->GetName() << " to conf " << pDestRequestedConf->GetName();
  PTRACE2(eLevelInfoNormal,"CConfPartyManager::GetMoveConfsFromRequest: ",trace_str.str().c_str());

  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::GetMoveConfsFromRequest(CMoveBaseAction* pMoveBaseAction,CCommConf*& pSourceRequestedConf,DWORD& partyId)
{
  DWORD sourceConfID = (DWORD)(-1);

  // get IDs from CMoveAction
  if(pMoveBaseAction){
    sourceConfID = pMoveBaseAction->GetSourceConfID();
    partyId = pMoveBaseAction->GetPartyID();
  }

  // get Confs from DB
  CCommConfDB* pCommConfDB = ::GetpConfDB();
  pSourceRequestedConf = pCommConfDB->GetCurrentConf(sourceConfID);

  COstrStream trace_str;
  // validate source conf
  if(!CPObject::IsValidPObjectPtr(pSourceRequestedConf)){
      trace_str << " Source Conf does not exist in DB, Source_Conf_Id = " << sourceConfID;
      PTRACE2(eLevelInfoNormal,"CConfPartyManager::GetMoveConfsFromRequest: ",trace_str.str().c_str());
      return STATUS_ILLEGAL_MOVE_PARTY_NOT_FOUND;
  }
  // validate party name in source conf
  const char* party_name = NULL;
  party_name = pCommConfDB->GetPartyName (sourceConfID,partyId);
  if(party_name==NULL){
    trace_str << " Party with id = " << partyId << " could not be found in source conf - " << pSourceRequestedConf->GetName();
      PTRACE2(eLevelInfoNormal,"CConfPartyManager::GetMoveConfsFromRequest: ",trace_str.str().c_str());
      return STATUS_ILLEGAL_MOVE_PARTY_NOT_FOUND;
  }

  trace_str << " move party  " << party_name << " from conf " << pSourceRequestedConf->GetName();
  PTRACE2(eLevelInfoNormal,"CConfPartyManager::GetMoveConfsFromRequest: ",trace_str.str().c_str());

  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::GetUserOperatorConf(CCommConf*& pOperatorConf)
{
  // get login name
  const char* loginName = GetLoginName().c_str();
  CCommConfDB* pCommConfDB = ::GetpConfDB();
  pOperatorConf = pCommConfDB->GetCurrentConf(loginName);

  COstrStream trace_str;
  // validate operator conf
  if(!CPObject::IsValidPObjectPtr(pOperatorConf)){
      trace_str << " Operator Conf does not found in DB, loginName = " << loginName;
      PTRACE2(eLevelInfoNormal,"CConfPartyManager::GetUserOperatorConf: ",trace_str.str().c_str());
      return STATUS_ILLEGAL_OPERATOR_CONF_NOT_FOUND;
  }

  trace_str << " Operator Conf does found , Name = " << loginName;
  PTRACE2(eLevelInfoNormal,"CConfPartyManager::GetUserOperatorConf: ",trace_str.str().c_str());

  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::GetHomeConfForParty(CCommConf* pSourceConf,DWORD partyId,CCommConf*& pHomeConf)
{
  PTRACE(eLevelInfoNormal,"CConfPartyManager::GetHomeConfForParty");

  COstrStream trace_str;

  // validate source conf
  if(!CPObject::IsValidPObjectPtr(pSourceConf)){
      PTRACE(eLevelInfoNormal,"CConfPartyManager::GetHomeConfForParty: pSourceConf is not valid");
      return STATUS_ILLEGAL_HOME_CONF_NOT_FOUND;
  }

  // Get party
  CConfParty* pCurrentParty = pSourceConf->GetCurrentParty(partyId);
  // validate party
  if(!CPObject::IsValidPObjectPtr(pCurrentParty)){
    trace_str << " Party with id = " << partyId << " could not be found in source conf - " << pSourceConf->GetName();
    PTRACE2(eLevelInfoNormal,"CConfPartyManager::GetHomeConfForParty: ",trace_str.str().c_str());
    return STATUS_ILLEGAL_HOME_CONF_NOT_FOUND;
  }

  CMoveInfo* partyMoveInfo = pCurrentParty->GetMoveInfo();
  if(!CPObject::IsValidPObjectPtr(partyMoveInfo)){
    PASSERT(101);
    return STATUS_ILLEGAL_HOME_CONF_NOT_FOUND ;
  }

  if(partyMoveInfo->IsValidHomeConf()==false){
    trace_str << " Home conf for party  " << pCurrentParty->GetName() << " is not valid \n";
    partyMoveInfo->Dump(trace_str);
    PTRACE2(eLevelInfoNormal,"CConfPartyManager::GetHomeConfForParty: ",trace_str.str().c_str());
    eMoveConfType homeConfType = eMoveConfType_None;
    eMoveConfType currentConfType = eMoveConfType_None;
    eMoveConfType previousConfType = eMoveConfType_None;
    partyMoveInfo->GetMoveConfType(homeConfType, currentConfType, previousConfType);
    if(previousConfType == eMoveConfType_Eq){
      return STATUS_ILLEGAL_HOME_CONF_EQ;
    }else{
      return STATUS_ILLEGAL_HOME_CONF_NOT_FOUND;
    }
  }

  DWORD homeConfId = partyMoveInfo->GetHomeConf();
  CCommConfDB* pCommConfDB = ::GetpConfDB();
  pHomeConf = pCommConfDB->GetCurrentConf(homeConfId);
   // validate home conf
  if(!CPObject::IsValidPObjectPtr(pHomeConf)){
      PTRACE(eLevelInfoNormal,"CConfPartyManager::GetHomeConfForParty: pHomeConf is not valid");
      return STATUS_ILLEGAL_HOME_CONF_NOT_FOUND;
  }

  if(pHomeConf==pSourceConf){
      PTRACE(eLevelInfoNormal,"CConfPartyManager::GetHomeConfForParty: pHomeConf == pSourceConf");
      return STATUS_ILLEGAL_HOME_CONF_NOT_FOUND;
  }

  return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////


// OperatorAssist conf_name party name set/reset private/public dtmp/password
STATUS CConfPartyManager::HandleOperatorAssistance(CTerminalCommand & command, std::ostream& answer)
{
  PTRACE(eLevelInfoNormal,"CConfPartyManager::HandleOperatorAssistance [operator_assistance_trace] ");

  DWORD numOfParams = command.GetNumOfParams();
  if(/*numOfParams != 3 && numOfParams != 4 && */numOfParams != 5){
      answer << "error: action must be specified:\n";
      answer << "usage: Bin/McuCmd OperatorAssist ConfParty [conf_name] [party_name] [set/reset] [private/public] [dtmf/password] \n";
      return STATUS_FAIL;
  }

  const string &confName 	 = command.GetToken(eCmdParam1);
  const string &partyName  = command.GetToken(eCmdParam2);
  const string &action     = command.GetToken(eCmdParam3);
  const string &mode       = command.GetToken(eCmdParam4);
  const string &init_cause = command.GetToken(eCmdParam5);

  answer <<  "conf_name: " << confName << " , party_name: " << partyName << " , action: " << action << " , mode: " << mode << " , init_cause: " << init_cause;;

  // Find Source Conf and party in DB
  CCommConfDB* pCommConfDB = ::GetpConfDB();
  CCommConf* pSourceRequestedConf = NULL;

  pSourceRequestedConf = pCommConfDB->GetCurrentConf(confName.c_str());
  if(!CPObject::IsValidPObjectPtr(pSourceRequestedConf)){
      answer << "error: Source Conf does not exist in DB: " <<  confName << " (OperatorAssist) ";
      return STATUS_FAIL;
  }
  if(pCommConfDB->SearchPartyName(confName.c_str(),partyName.c_str()) != STATUS_OK){
      answer << "error: Party does not exist in DB " <<  confName.c_str() << " " << partyName.c_str() << " (OperatorAssist)";
      return STATUS_FAIL;
  }


 BYTE action_val = 0;
 if( strncmp(action.c_str(),"set",strlen("set")) == 0 ){
   action_val = 1;
 }

 BYTE mode_val = WAIT_FOR_OPER_NONE;
 if( strncmp(mode.c_str(),"private",strlen("private")) == 0 ){
   mode_val = WAIT_FOR_OPER_ON_REQ_PRIVATE;
 }else if( strncmp(mode.c_str(),"public",strlen("public")) == 0 ){
   mode_val = WAIT_FOR_OPER_ON_REQ_PUBLIC;
 }

 BYTE init_by = 0;
 if( strncmp(init_cause.c_str(),"dtmf",strlen("dtmf"))==0 ){
   init_by = DTMF_OPERATOR_ASSISTANCE_REQ;
 }else if( strncmp(init_cause.c_str(),"password",strlen("password"))==0 ){
   init_by = PASSWORD_FAILURE_OPERATOR_ASSISTANCE_REQ;
 }


 // update party DB
  CConfParty* pConfParty = pSourceRequestedConf->GetCurrentParty(partyName.c_str());
  // klocwork Romem
  if(pConfParty)
  {
	  if(action_val==1){//set
		  pConfParty->SetWaitForOperAssistance(mode_val);
	  }else if(action_val==0){//reset
		  pConfParty->SetWaitForOperAssistance(WAIT_FOR_OPER_NONE);
	  }
  }

// update conf DB
    DWORD partyId = pCommConfDB->GetPartyId(confName.c_str(), partyName.c_str());
   CConfApi* pConfApi = new CConfApi;
   pConfApi->CreateOnlyApi(*(pSourceRequestedConf->GetRcvMbx()),NULL,NULL,1);
  pConfApi->OperatorAssistance(partyId, action_val, mode_val ,init_by);

 return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleColdMove(CCommConf* pSourceConf,CCommConf* pDestConf,CConfParty* pConfParty)
{
   STATUS status= STATUS_OK;
   DWORD state = pConfParty->GetPartyState();
   if(PARTY_DISCONNECTED == state || PARTY_WAITING_FOR_DIAL_IN == state || PARTY_STAND_BY == state)
   {
	   //Build Move info :
	   CRsrvParty* pMovedParty = new CRsrvParty(*pConfParty);
	   CMoveInfo* partyMoveInfo = pConfParty->GetMoveInfo();
	   CMoveInfo*  pMove_info =  pMovedParty->GetMoveInfo();
	   CMoveConfDetails sourceConfMoveDetails(pSourceConf);
	   CMoveConfDetails destConfMoveDetails(pDestConf);
	   // Romem klocwork
	   if(pMove_info)
	   {
		   pMove_info->Create(sourceConfMoveDetails,pConfParty->IsOperatorParty());
		   pMove_info->UpdateMove(sourceConfMoveDetails,destConfMoveDetails);
	   }
	   //partyMoveInfo->UpdateMove(sourceConfMoveDetails,destConfMoveDetails);

	   if (IsMoveCompleted())
	   {
		  //if there's no parallel Move we:

		  //a). insert move info to our list
		   InsertMovePartyInfo(pMovedParty);
		  //b). send drop party to source conference
		   CRsrvParty* pRsrvMovedPartyTemp =  (*m_MoveInfo.begin());
		   const char* partyName = ::GetpConfDB()->GetPartyName(pSourceConf->GetMonitorConfId(), pConfParty->GetPartyId());
		   if(!partyName)
		   {
			   PTRACE(eLevelInfoNormal, "ConfPartyManager::HandleColdMove: Party is not found  in Source Conf - Dump CMoveInfo object and ignore cold move operation");
			   CMoveInfo*  pReservedMoveInfo = pRsrvMovedPartyTemp->GetMoveInfo();
			   if (IsValidPObjectPtr(pReservedMoveInfo))
			   {
				   pReservedMoveInfo->Dump();
				   DeleteMovePartyInfo(0);
			   }
			   status = STATUS_PARTY_DOES_NOT_EXIST;
			   DBGPASSERT(status);
			   return status;
		   }
		   CConfApi confApi;
		   confApi.CreateOnlyApi(*(pSourceConf->GetRcvMbx()));
		   confApi.DropParty(partyName);
		   confApi.DestroyOnlyApi();
		   //status = pTemp->SendDropParty();
		   PTRACE(eLevelInfoNormal, "ConfPartyManager::HandleColdMove: Sending Drop Party");

		   //c). if send was unsuccessful - delete update info from list
		   if (status != STATUS_OK)
		   {
			  DBGPASSERT(status);
			  DeleteMovePartyInfo(0);
			  PTRACE(eLevelInfoNormal, "ConfPartyManager::HandleColdMove: Drop status is not OK");
			  return status;
		   }

		   //d). if send was successful - start timer to check the result of drop operation
			  StartTimer(DEL_MOVED_PARTY_COMPLETE, 1 * SECOND);
		}
		else //if there's parallel Move we only insert update info to our list
		{
		   status = InsertMovePartyInfo(pMovedParty);
		    PTRACE2(eLevelInfoNormal,"ConfPartyManager::HandleColdMove, Party will be moved later, Party Name: ",pConfParty->GetName());
		}
	}
	else
	{
	   PTRACE(eLevelInfoNormal,"ConfPartyManager::HandleColdMove Can not move party. Party in Wrong state");
	   status = STATUS_FAIL;
	}
    return status;
}

//=====================================================================================================================================//
// STATUS CConfPartyManager::UpdateOperatorAssistanceInDB(CCommConf* pCommConf, DWORD partyId, BYTE action,BYTE mode )
// {
//   PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateOperatorAssistanceInDB [operator_assistance_trace] ");

//   if(!CPObject::IsValidPObjectPtr(pCommConf)){
//     PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateOperatorAssistanceInDB [operator_assistance_trace] pCommConf is not valid");
//     return STATUS_FAIL;
//   }

//   CConfParty* pConfParty = pCommConf->GetCurrentParty(partyId);
//   if(!CPObject::IsValidPObjectPtr(pConfParty)){
//     PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateOperatorAssistanceInDB [operator_assistance_trace] pConfParty is not valid");
//     return STATUS_FAIL;
//   }

//   BYTE is_operator_party = pConfParty->IsOperatorParty();
//   BYTE is_operator_conf = pCommConf->GetOperatorConf();

//   BYTE mode_val = mode;
//   if(action==0){ // reset
//       mode_val = WAIT_FOR_OPER_NONE;
//   }
//   if(!is_operator_party){
//     pConfParty->SetWaitForOperAssistance(mode_val);
//   }else{
//     if(is_operator_conf){
//       PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateOperatorAssistanceInDB [operator_assistance_trace] operator party moved into operator conf - do nothing");
//     }else{
//       // reset all WAIT_FOR_OPER_ON_REQ_PUBLIC parties
//       PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateOperatorAssistanceInDB [operator_assistance_trace] operator party moved into conf - reset public operator assistance parties");
//       pCommConf->ResetPublicOperatorAssistanceParties();
//       mode_val = WAIT_FOR_OPER_NONE;
//     }
//   }

//   CConfApi* pConfApi = new CConfApi;
//   pConfApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()),NULL,NULL,1);
//   pConfApi->OperatorAssistance(partyId, action, mode_val ,0);

//   return STATUS_OK;
// }
//=====================================================================================================================================//
STATUS CConfPartyManager::ResetOperatorAssistanceInDB(CCommConf* pSourceConf, CCommConf* pDestConf, DWORD partyId)
{
  PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateOperatorAssistanceInDB [operator_assistance_trace] ");

  if (!CPObject::IsValidPObjectPtr(pSourceConf))
  {
    PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateOperatorAssistanceInDB [operator_assistance_trace] failed - pSourceConf is not valid");
    return STATUS_FAIL;
  }

  if (!CPObject::IsValidPObjectPtr(pDestConf))
  {
    PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateOperatorAssistanceInDB [operator_assistance_trace] failed - pDestConf is not valid");
    return STATUS_FAIL;
  }

  CConfParty* pConfParty = pSourceConf->GetCurrentParty(partyId);
  if (!CPObject::IsValidPObjectPtr(pConfParty))
  {
    PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateOperatorAssistanceInDB [operator_assistance_trace] failed - pConfParty is not valid");
    return STATUS_FAIL;
  }

  BYTE is_operator_party = pConfParty->IsOperatorParty();
  BYTE is_source_operator_conf = pSourceConf->GetOperatorConf();
  BYTE is_dest_operator_conf = pDestConf->GetOperatorConf();

  CConfApi* pConfApi = new CConfApi;
  if (!is_operator_party)
  {
    // Regular party that moves into any conference - the operator assistance ended
    pConfParty->SetWaitForOperAssistance(WAIT_FOR_OPER_NONE);
    pConfApi->CreateOnlyApi(*(pSourceConf->GetRcvMbx()), NULL, NULL, 1);
    pConfApi->OperatorAssistance(partyId, 0, WAIT_FOR_OPER_NONE, 0);
    pConfApi->DestroyOnlyApi();
  }
  else
  {
    if (is_dest_operator_conf)
    {
      PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateOperatorAssistanceInDB [operator_assistance_trace] - operator party moved into operator conference - do nothing");
    }
    else
    {
      // Reset all WAIT_FOR_OPER_ON_REQ_PUBLIC parties
      PTRACE(eLevelInfoNormal,"CConfPartyManager::UpdateOperatorAssistanceInDB [operator_assistance_trace] - operator party moved into regular conference - reset public operator assistance parties");
      pDestConf->ResetPublicOperatorAssistanceParties();
      pConfApi->CreateOnlyApi(*(pDestConf->GetRcvMbx()), NULL, NULL, 1);
      pConfApi->OperatorAssistance(partyId, 0, WAIT_FOR_OPER_NONE, 0);
      pConfApi->DestroyOnlyApi();
    }
  }
  POBJDELETE(pConfApi);
  return STATUS_OK;

}

//=====================================================================================================================================//
void CConfPartyManager::AddActiveAlarmForCOPWithMPMPlus()
{

	if (eStartupConditionOk != GetStartupConditionStatusByErrorCode(AA_SYSTEM_BASED_MODE_NOT_INTIALIZED))
  {
    PTRACE(eLevelInfoNormal, "CConfPartyManager::AddActiveAlarmForCOPWithBreezeMode - System Based Mode does not initialized yet");
    return;
  }
  if (eStartupConditionOk != GetStartupConditionStatusByErrorCode(AA_NO_LICENSING))
  {
    PTRACE(eLevelInfoNormal, "CConfPartyManager::AddActiveAlarmForCOPWithBreezeMode - System Licensing does not initialized yet");
    return;
  }

  PTRACE(eLevelInfoNormal, "CConfPartyManager::AddActiveAlarmForCOPWithBreezeMode");

  eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
  CProcessBase *pProcess = (CProcessBase*)CProcessBase::GetProcess();

    if (::GetIsCOPdongleSysMode() && (systemCardsBasedMode == eSystemCardsMode_mpm_plus))
    {
    	pProcess->AddActiveAlarmFromProcess( FAULT_GENERAL_SUBJECT,
											 AA_COP_MODE_NOT_SUPPORTED_MPM_PLUS_MODE,
    										 MAJOR_ERROR_LEVEL,
    										 "System Cards MPM Plus mode are not supported in Event mode",
    										 true,		//isForEma
    										 true);		//inForFaults
    	PTRACE(eLevelInfoNormal, "CConfPartyManager::AddActiveAlarmForCOPWithBreezeMode : System Cards MPM Plus mode are not supported in Event mode");

    }

  return;
}
//=====================================================================================================================================//
void CConfPartyManager::UpdateSystemCapacityLimits()
{

	if (eStartupConditionOk != GetStartupConditionStatusByErrorCode(AA_SYSTEM_BASED_MODE_NOT_INTIALIZED))
  {
    PTRACE(eLevelInfoNormal, "CConfPartyManager::AddActiveAlarmForCOPWithBreezeMode - System Based Mode does not initialized yet");
    return;
  }
  if (eStartupConditionOk != GetStartupConditionStatusByErrorCode(AA_NO_LICENSING))
  {
    PTRACE(eLevelInfoNormal, "CConfPartyManager::AddActiveAlarmForCOPWithBreezeMode - System Licensing does not initialized yet");
    return;
  }

  PTRACE(eLevelInfoNormal, "CConfPartyManager::AddActiveAlarmForCOPWithBreezeMode");

  eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();

  m_pProcess->UpdateSystemCapacityLimitsAccordingToSystemMode();

  TRACEINTOFUNC << "systemCardsBasedMode = "<<systemCardsBasedMode ;

}

//=====================================================================================================================================//
//======== 	FAILOVER: HOT BACKUP FUNCTIONS  ==========================================================================================//
//=====================================================================================================================================//

void CConfPartyManager::OnFailoverStartMasterInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"\nCConfPartyManager::OnFailoverStartMasterInd");
	STATUS status = STATUS_OK;

	if (m_pProcess->GetIsFailoverSlaveMode() == TRUE)
	{//a warning should be sent to ema only in case of change from slave to master, not in case of just enabling master
		WORD confNum = ::GetpConfDB()->GetConfNumber();
		if (confNum > 0)
		{
			PTRACE(eLevelError,  "CConfPartyManager::OnFailoverStartMasterInd:warning on  start master while there are ongoing conferences" );
			status = STATUS_ONGOING_CONFERENCE_AND_RESERVATIONS_EXISTS;
		}

		//2) Terminate all ongoing conferences when move from slave to master:
		TerminateOngoingConferences();
	}

	CSegment*  pRspMsg = new CSegment;
	*pRspMsg << status;
	ResponedClientRequest(FAILOVER_START_MASTER, pRspMsg);


	//1) Set mode:
	m_pProcess->SetIsFailoverSlaveMode(false);
}

/////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleFailoverStartMaster(CTerminalCommand & command, std::ostream& answer)
{
	//1) Set mode:
	m_pProcess->SetIsFailoverSlaveMode(false);

	//2) Terminate all ongoing conferences:
	TerminateOngoingConferences();

	answer <<  "OK";
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::TerminateOngoingConferences()
{
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pCommConf = pCommConfDB->GetFirstCommConf();
	while (IsValidPObjectPtr(pCommConf))
	{
		DWORD confID = pCommConf->GetMonitorConfId();
		TerminateOngoingConf(confID);
		pCommConf = pCommConfDB->GetFirstCommConf();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::TerminateOngoingConf(DWORD confID)
{
	//1. Deallocate monitor ID
	DWORD opcode = SYNC_DEL_RSRV_CONF_REQ;
	//if (pCurCommConf->IsMeetingRoom()) //??? yael
	//	opcode = SYNC_DEACTIVATE_MR_REQ; //??? yael

	CRsrvManagerApi rsrvManagerApi;

	STATUS allocateStatus = STATUS_OK;

	CSegment rspMsg;
	OPCODE   rspOpcode;

	CSegment *seg = new CSegment;
	*seg << confID;

	STATUS responseStatus = rsrvManagerApi.SendMessageSync(seg,opcode,CONF_RSRC_REQ_TOUT, rspOpcode, rspMsg);

	if (STATUS_OK == responseStatus)
	{
		if(rspMsg.GetLen() > 0)
		{
			rspMsg >> allocateStatus;
			if(allocateStatus != STATUS_OK)
			{
				PTRACE2INT(eLevelInfoNormal,"ConfPartyManager::TerminateOngoingConf : CONF RSRC ALLOC/DeALLOC FAILED !!! ",confID);
				DBGPASSERT(allocateStatus);
				//status = allocateStatus;
			}
			else
			{
				PTRACE2INT(eLevelInfoNormal,"ConfPartyManager::TerminateOngoingConf : CONF RSRC ALLOC/DeALLOC O.K !!! ",confID);
			}
		}
		else // no content in segemnt
		{
			PTRACE2INT(eLevelInfoNormal,"ConfPartyManager::TerminateOngoingConf : CONF RSRC ALLOC/DeALLOC FAILED No Msg Content!!! ",confID);
			DBGPASSERT(confID);
			//status = STATUS_ILLEGAL;
		}
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"ConfPartyManager::TerminateOngoingConf : CONF RSRC ALLOC/DeALLOC TIME OUT !!! ",confID);
		DBGPASSERT(responseStatus);
		//status = responseStatus;
	}

	/*//2. Announce SipProxy
	const char* pConfName = ::GetpConfDB()->GetConName(confID);
	CSipProxyManagerApi SipProxyApi;
	SipProxyApi.DelConference(pConfName, confID);*/

	DWORD taskId = 0;

	for (TaskIdToConfId::iterator itr = m_TaskIdToConfId.begin() ; itr != m_TaskIdToConfId.end() ; ++itr)
	{
		if (itr->second == confID)
		{
			taskId = itr->first;

			// Notify ConfPartyManager to remove the conference from TaskId-ConfId map
			CSegment* taskAndConfId = new CSegment;
			*taskAndConfId << taskId;
			CManagerApi confpartyManagerApi(eProcessConfParty);
			STATUS res = confpartyManagerApi.SendMsg(taskAndConfId, CONF_DELETED_CONFERENCE);

			break;
		}
	}

	//2. Delete from DB
	::GetpConfDB()->Cancel(confID);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverRestartSlaveInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"\nCConfPartyManager::OnFailoverRestartSlaveInd");

	//1)
	TerminateOngoingConferences();

	//2) Delete:
	EmptyDbForFailoverSlave();
}

/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverStartSlaveInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"\nCConfPartyManager::OnFailoverStartSlaveInd");
	STATUS status = STATUS_OK;

	WORD confNum = ::GetpConfDB()->GetConfNumber();
	if (confNum > 0)
	{
		PTRACE(eLevelError,  "CConfPartyManager::OnFailoverStartSlaveInd:cant start slave while there are ongoing conferences" );
		status = STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_AND_RESERVATIONS_EXISTS;
	}
	CSegment*  pRspMsg = new CSegment;
	*pRspMsg << status;
	ResponedClientRequest(0,pRspMsg);

	if (status != STATUS_OK)
		return;

	 //1) Set mode
	m_pProcess->SetFailoverParams(true, true);

	//2) Delete:
	EmptyDbForFailoverSlave();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverStartMasterBecomeSlaveInd(CSegment* pMsg)
{
    PTRACE(eLevelInfoNormal,"\nCConfPartyManager::OnFailoverStartMasterBecomeSlaveInd");

    //1) Set mode: in order to prevent SET messages from UI
    m_pProcess->SetFailoverParams(true, true);

    //2.1) Terminate Conferences if needed:
    WORD confNum = ::GetpConfDB()->GetConfNumber();
    if (confNum > 0)
    {
         TerminateConferences();
         return;
    }

    //2.2) Empty DB and notify Failover process
    else
    {
		EmptyDbForFailoverSlave();
        NotifyFailoverOnEndPrepareMasterBecomeSlave(STATUS_OK);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::NotifyFailoverOnEndPrepareMasterBecomeSlave(STATUS status)
{
   TRACESTR(eLevelInfoNormal) << "\nCConfPartyManager::NotifyFailoverOnEndPrepareMasterBecomeSlave- status =" << status;

   CManagerApi api(eProcessFailover);
   CSegment *pSeg = new CSegment;
   *pSeg << (DWORD)status;
   STATUS sendStatus = api.SendMsg(pSeg, FAILOVER_CONFPARTY_END_PREPARE_MASTER_BECOME_SLAVE);

   if (sendStatus != STATUS_OK)
     FPASSERT(FAILOVER_CONFPARTY_END_PREPARE_MASTER_BECOME_SLAVE);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::TerminateConferences()
{
   PTRACE(eLevelInfoNormal,"\nCConfPartyManager::TerminateConferences, Master became slave due to NETWORK problems ");

   CCommConfDB* pCommConfDB = ::GetpConfDB();

   WORD  wNumberOfConfs = pCommConfDB->GetConfNumber();
   DWORD numOfTotalParties = 0;

   CCommConf* pCommConf = pCommConfDB->GetFirstCommConf();
   while (IsValidPObjectPtr(pCommConf))
   {
	  numOfTotalParties = numOfTotalParties + pCommConf->GetConnectedPartiesNumber();

	  //Destory on-going conf
	  CConfApi confApi;
	  confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
	  confApi.Destroy();

	  pCommConf = pCommConfDB->GetNextCommConf();
   }

   if(1 == numOfTotalParties)
   	 numOfTotalParties = 2;

   StartTimer(MAX_TIME_DELETE_ALL_CONFS_TOUT,numOfTotalParties*AVERAGE_TIME_TO_DISCONNECT_PARTY);
   StartTimer(DELETE_ALL_CONFS_TOUT,AVERAGE_TIME_TO_DISCONNECT_PARTY);

}
/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnDeleteAllConfsTimeOut(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal,"\nCConfPartyManager::OnDeleteAllConfsTimeOut ");

	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pCommConf = pCommConfDB->GetFirstCommConf();
	if(!IsValidPObjectPtr(pCommConf)) //All conferences deleted
	{
		DeleteTimer(MAX_TIME_DELETE_ALL_CONFS_TOUT);
		EmptyDbForFailoverSlave();
        NotifyFailoverOnEndPrepareMasterBecomeSlave(STATUS_OK);
	}

	else
	  StartTimer(DELETE_ALL_CONFS_TOUT,AVERAGE_TIME_TO_DISCONNECT_PARTY);

}

/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnMaxTimeDeleteAllConfsTimeOut(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal,"\nCConfPartyManager::OnMaxTimeDeleteAllConfsTimeOut ");

	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pCommConf = pCommConfDB->GetFirstCommConf();
	if(IsValidPObjectPtr(pCommConf)) //All conferences deleted
	{
		NotifyFailoverOnEndPrepareMasterBecomeSlave(STATUS_FAIL);
		//DBGASSERT(1);
	}
	else
	  NotifyFailoverOnEndPrepareMasterBecomeSlave(STATUS_OK);
}

////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::EmptyDbForFailoverSlave()
{
	//Conferences: At this point no ongoing conf should be here. Therefore, the flow here is short, and doesn't contain all the delete conf flow.
	CCommConfDB* pDeletedCommConfDB = ::GetpConfDB();
	pDeletedCommConfDB->DeleteDB();

	for (int i = 0; i < eNumOfSlaveSync; i++)
		m_slaveSyncElements[i] = FALSE;

	/*//Recording links:
	CRecordingLinkDB* pDeletedRecordingDB = ::GetRecordingLinkDB();
	pDeletedRecordingDB->ResetDB();

	//IVR
	m_pIVRManager->DeleteIVRServices();
	m_pIVRManager->InitIVRConfig();

	//Profiles:
	ResetProfiles();

	//Meeting Rooms:
	ResetMeetingRooms();*/
}

/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::ResetProfiles()
{
	PROFILE_IND_S profile_ind_s;

	CCommResDB *pProfilesDB = ::GetpProfilesDB();
	const CCommResDB::ReservArray& profilesArray = pProfilesDB->GetReservArray();
	CCommResDB::ReservArray::const_iterator itr_end = profilesArray.end();
	for (CCommResDB::ReservArray::const_iterator itr = profilesArray.begin() ; itr != itr_end ; ++itr)
	{
		if ((*itr) != NULL)
		{
			CSegment* pRASeg = new CSegment;

			profile_ind_s.profile_Id = (*itr)->GetConferenceId();
			profile_ind_s.maxVideoPartyType = eVideo_party_type_dummy;
			pRASeg->Put((BYTE*)&profile_ind_s, sizeof(profile_ind_s));
			SendAsyncMsgToRsrcProcess(pRASeg, PROFILE_DELETE_RSRC_IND);
		}
	}

	pProfilesDB->ResetDB(PROFILES_DB_DIR);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::ResetMeetingRooms()
{
	CCommResDB* pMrsDB = ::GetpMeetingRoomDB();
	const CCommResDB::ReservArray& mrsArray = pMrsDB->GetReservArray();
	CCommResDB::ReservArray::const_iterator itr_end = mrsArray.end();
	for (CCommResDB::ReservArray::const_iterator itr = mrsArray.begin() ; itr != itr_end ; ++itr)
	{
		if ((*itr) != NULL)
		{
			DWORD mrId = (*itr)->GetConferenceId();

			CSegment *pToRsrcSeg = new CSegment;
			*pToRsrcSeg << mrId ;
			SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, DEL_MR_REQ);

			const char* pConfName = (*itr)->GetName();
			CSipProxyManagerApi SipProxyApi;
			CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
			for( int i=1; i<=4;i++ )
			{
			    CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(i);
			    if( pServiceParams == NULL )
			       continue;
			    SipProxyApi.DelConference(i, pConfName, mrId);
			}
	   }
	}
	pMrsDB->ResetDB(FILE_MEETING_ROOM_DB);
}

/////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleFailoverStartSlave(CTerminalCommand & command, std::ostream& answer)
{
	//1) Set mode
	m_pProcess->SetIsFailoverSlaveMode(true);

	//2) Delete:
	EmptyDbForFailoverSlave();

	answer <<  "OK";
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverSlaveBecomeMasterInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnFailoverSlaveBecomeMasterInd");
	StopSlaveAndStartRestoreInd();
}


/////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleFailoverStopSlaveAndStartRestore(CTerminalCommand & command, std::ostream& answer)
{
	StopSlaveAndStartRestoreInd();

	answer <<  "OK";
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::StopSlaveAndStartRestoreInd()
{
	m_pProcess->SetIsFailoverSlaveMode(false);

	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pCommConf = pCommConfDB->GetFirstCommConf();
	while (IsValidPObjectPtr(pCommConf))
	{
		pCommConf->RestoreHotBackupFields();

		// VNGR-22639: check if it's in lecture mode, we need to save lecturer
		// layout and restore it if it's.
		pCommConf->GetLectureMode()->PrintAll();
		if (pCommConf->GetLectureMode()->GetLectureModeType() == 1)
		{
			CLectureModeParams* pLectureMode = new CLectureModeParams(*pCommConf->GetLectureMode());
			CConfParty* pLecturerConfParty = pCommConf->GetCurrentParty(pLectureMode->GetLecturerName());

			if (pLecturerConfParty)
			{
				CVideoLayout*  pLecturerVideoLayout = new CVideoLayout(*pLecturerConfParty->GetVideoLayout());

				pCommConf->SaveLecturerVideoLayout(pLectureMode, pLecturerVideoLayout);

				std::string sLayout;
				pLecturerVideoLayout->ToString(sLayout);

				TRACEINTO << "VNGR-22639: Conf: " << pCommConf->GetName() << ", LecturerVideoLayout: \n" << sLayout;
			}
			else
			{
				TRACEINTO << "VNGR-22639: Conf: " << pCommConf->GetName() << ", can't find lecturer party.\n";
			}
		}

		StartOnGoingConf(*pCommConf, "", "");
		pCommConf = pCommConfDB->GetNextCommConf();
	}
}

////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverTerminateConfInd(CSegment* pSeg)
{
    DWORD confId;
    *pSeg >> confId;

    PTRACE2INT(eLevelInfoNormal, "ConfPartyManager::OnFailoverTerminateConfInd -  confId = ", confId);

    CCommConf* pCurCommConf = (CCommConf*) ::GetpConfDB()->GetCurrentConf(confId);

	if (!CPObject::IsValidPObjectPtr(pCurCommConf))
	{
		DBGPASSERT(confId);
		return;
	}

	else
		TerminateOngoingConf(confId);
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverAddOrUpdateConfInd(CSegment* pSeg)
{
    PTRACE(eLevelInfoNormal, "ConfPartyManager::OnFailoverAddOrUpdateConfInd");
    DWORD len;
    *pSeg >> len;

    char* pConferenceElementStr = new char[len+1];
    pConferenceElementStr[len] = '\0';
    *pSeg >> pConferenceElementStr;

    CXMLDOMDocument *pDom = new CXMLDOMDocument;
    if (pDom->Parse((const char **)&pConferenceElementStr) == SEC_OK)
	{
		CXMLDOMElement* pRoot = pDom->GetRootElement();
		if(pRoot)
		{
			CCommConf *pCommConf = new CCommConf;
			char szErrorMsg[ERROR_MESSAGE_LEN];
			pCommConf->DeSerializeFullXml(pRoot, szErrorMsg);
			pCommConf->UpdateHotBackupFields();

			//in case of add:
			DWORD confId = pCommConf->GetMonitorConfId();
			if (::GetpConfDB()->FindId(confId) == NOT_FIND)
			{
				int status = ::GetpConfDB()->Add(*pCommConf);

				if (STATUS_OK == status)
				{
					CSegment *pToRsrcSeg = new CSegment;
					CCommResApi* pCommResApi = pCommConf;
					pCommResApi->Serialize(NATIVE, *pToRsrcSeg);
					SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, SLAVE_ADD_CONF_REQ);
				}

				else// (status != STATUS_OK)
				{
					PTRACE(eLevelError,"CConfPartyManager::OnFailoverAddOrUpdateConfInd - Hot backup - Failed on Add Conf To DB");
					PASSERT(status);
					POBJDELETE(pCommConf);
				}
			}

			//in case of update:
			else
			{
				CStructTm* pNewTime = const_cast<CStructTm*>(pCommConf->GetCalculatedEndTime());
				CCommConf* pCurCommConf = (CCommConf*)::GetpConfDB()->GetCurrentConf(confId);
				const CStructTm* pCurTime = pCurCommConf->GetCalculatedEndTime();
				if (*pNewTime != *pCurTime)
				{
					PTRACE(eLevelInfoNormal, "ConfPartyManager::OnFailoverAddOrUpdateConfInd -  new time!!");

					//
					CSegment *pSeg = new CSegment;
					*pSeg << confId;
					pNewTime->Serialize(NATIVE, *pSeg);

					CTaskApi resourceManagerApi(eProcessResource, eManager);
					STATUS msgStatus = resourceManagerApi.SendMsg(pSeg,SLAVE_UPDATE_CONF_TIME_REQ);
					//
				}

				::GetpConfDB()->Update(*pCommConf);
			}
		}
	}

    PDELETEA(pConferenceElementStr);
    POBJDELETE(pDom);
}


/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverAddOrUpdateProfileInd(CSegment* pSeg)
{
    PTRACE(eLevelInfoNormal, "ConfPartyManager::OnFailoverAddOrUpdateProfileInd");

    if (m_slaveSyncElements[eSlaveSyncProfiles] == FALSE)
	{
		PTRACE(eLevelError,"CConfPartyManager::OnFailoverAddOrUpdateProfileInd  - First Sync");
		ResetProfiles();
		m_slaveSyncElements[eSlaveSyncProfiles] = TRUE;
	}

    DWORD len;
    *pSeg >> len;

    char* pProfileElementStr = new char[len+1];
    pProfileElementStr[len] = '\0';
    *pSeg >> pProfileElementStr;

    CXMLDOMDocument *pDom = new CXMLDOMDocument;
    if (pDom->Parse((const char **)&pProfileElementStr) == SEC_OK)
	{
		CXMLDOMElement* pRoot = pDom->GetRootElement();
		if(pRoot)
		{
			CCommResApi* pRsrvApi = new CCommResApi;
			char szErrorMsg[ERROR_MESSAGE_LEN];
			pRsrvApi->DeSerializeXml(pRoot, szErrorMsg,ADD_RESERVE);

			PROFILE_IND_S profile_ind_s;
			profile_ind_s.profile_Id = pRsrvApi->GetMonitorConfId();
			CResRsrcCalculator rsrcCalc;
			eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
			eVideoPartyType vidtype = rsrcCalc.GetRsrcVideoType(systemCardsBasedMode, pRsrvApi);
			profile_ind_s.maxVideoPartyType = vidtype;

			int profileId = pRsrvApi->GetMonitorConfId();

			if (::GetpProfilesDB()->IsConfIdExist(profileId) == NO)
			{
	 			int status=AddProfileToSlave((CCommRes*)pRsrvApi);
            	if(status == STATUS_OK)
			    	SendSlaveAddOrUpdateProfileIndToRsrc((CCommRes*)pRsrvApi,SLAVE_ADD_PROFILE_REQ);

				else
				{
					PTRACE(eLevelError,"CConfPartyManager::OnFailoverAddOrUpdateProfileInd - Hot backup - Failed on Add Profile To DB");
					PASSERT(status);
					//POBJDELETE(pRsrvApi);
				}
			}
			else
			{
			   ::GetpProfilesDB()->Update((CCommRes&)(*pRsrvApi));
			   SendSlaveAddOrUpdateProfileIndToRsrc((CCommRes*)pRsrvApi,SLAVE_UPDATE_PROFILE_REQ);

			}
			POBJDELETE(pRsrvApi);
 		}
	}

    PDELETEA(pProfileElementStr);
    POBJDELETE(pDom);
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::SendSlaveAddOrUpdateProfileIndToRsrc(CCommRes* pRsrvApi, OPCODE opcode)
{
    PTRACE(eLevelInfoNormal, "CConfPartyManager::SendSlaveAddOrUpdateProfileIndToRsrc");

	PROFILE_IND_S profile_ind_s;
	profile_ind_s.profile_Id = pRsrvApi->GetMonitorConfId();
	CResRsrcCalculator rsrcCalc;
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	eVideoPartyType vidtype = rsrcCalc.GetRsrcVideoType(systemCardsBasedMode, pRsrvApi);
	profile_ind_s.maxVideoPartyType = vidtype;

   	TRACESTR (eLevelInfoNormal) << "CConfPartyManager::SendSlaveAddOrUpdateProfileIndToRsrd: Sending Profile weight to Resource process, profile name: " << pRsrvApi->GetName() << " Calc Profile video type: " << eVideoPartyTypeNames[profile_ind_s.maxVideoPartyType];
	CSegment * pRASeg = new CSegment;
	pRASeg->Put((BYTE*)&profile_ind_s,sizeof(PROFILE_IND_S));
	SendAsyncMsgToRsrcProcess(pRASeg,opcode);
}


////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverTerminateProfileInd(CSegment* pSeg)
{
    DWORD profileId;
    *pSeg >> profileId;

    PTRACE2INT(eLevelInfoNormal, "ConfPartyManager::OnFailoverTerminateProfileInd - profileId = ", profileId);

    CCommResApi* pProfile = (CCommResApi*)::GetpProfilesDB()->GetCurrentRsrv(profileId);

	if (!CPObject::IsValidPObjectPtr(pProfile))
	{
		DBGPASSERT(profileId);
		return;
	}

	STATUS status=(::GetpProfilesDB())->Cancel(profileId);

    if(STATUS_OK == status)
    {
    	CSegment * pRASeg = new CSegment;
    	PROFILE_IND_S profile_ind_s;
    	profile_ind_s.profile_Id = profileId;
    	profile_ind_s.maxVideoPartyType = eVideo_party_type_dummy;
		pRASeg->Put((BYTE*)&profile_ind_s,sizeof(PROFILE_IND_S));
		//Send ASynch Msg to the RA process
		SendAsyncMsgToRsrcProcess(pRASeg,SLAVE_DELETE_PROFILE_REQ);
    }
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverAddOrUpdateMeetingRoomInd(CSegment* pSeg)
{
    PTRACE(eLevelInfoNormal, "ConfPartyManager::OnFailoverAddOrUpdateMeetingRoomInd");

    if (m_slaveSyncElements[eSlaveSyncMeetingRooms] == FALSE)
	{
		PTRACE(eLevelError,"CConfPartyManager::OnFailoverAddOrUpdateMeetingRoomInd  - First Sync");
		ResetMeetingRooms();
		m_slaveSyncElements[eSlaveSyncMeetingRooms] = TRUE;
	}

    DWORD len;
    *pSeg >> len;

    char* pMeetingRoomElementStr = new char[len+1];
    pMeetingRoomElementStr[len] = '\0';
    *pSeg >> pMeetingRoomElementStr;

    CXMLDOMDocument *pDom = new CXMLDOMDocument;
    if (pDom->Parse((const char **)&pMeetingRoomElementStr) == SEC_OK)
	{
		CXMLDOMElement* pRoot = pDom->GetRootElement();
		if(pRoot)
		{
		   CCommRes* pMRoom= new CCommRes;
		   char szErrorMsg[ERROR_MESSAGE_LEN];
		   pMRoom->DeSerializeXml(pRoot, szErrorMsg,ADD_RESERVE);

  		   DWORD MRMonitorID = pMRoom->GetMonitorConfId();

  		   CCommResShort*  pMrShort = ::GetpMeetingRoomDB()->GetCurrentRsrvShort(MRMonitorID);
      	   if (NULL == pMrShort) //new MR
  		   {
		      //Add a new meeting room to the MR DB
			  STATUS status = ::GetpMeetingRoomDB()->Add(*pMRoom);
			  if ( STATUS_OK == status)
			  {
			  	 CSegment *pToRsrcSeg = new CSegment;
			  	 CCommResApi* pCommResApi = pMRoom;
				 pCommResApi->Serialize(NATIVE, *pToRsrcSeg);
				 SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, SLAVE_ADD_MEETING_ROOM_REQ);

			     CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
			     CConfIpParameters* pServiceParams = NULL;
			     CSipProxyManagerApi SipProxyApi;

			     for (int i = 1; i <= NUM_OF_IP_SERVICES; i++)
			     {
			    	 pServiceParams = pIpServiceListManager->FindServiceByName( pMRoom->GetServiceRegistrationContentServiceName(i - 1));
			    	 if (pServiceParams != NULL) {
			    		 if (pMRoom->GetServiceRegistrationContentRegister(i - 1) == TRUE) {
			    			 if (pMRoom->GetEntryQ())
			    				 SipProxyApi.AddEQ(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
			    			 else if (pMRoom->IsSIPFactory())
			    				 SipProxyApi.AddFactory(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
			    			 else if (pMRoom->GetIsGateway())
			    				 SipProxyApi.AddGW(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
			    			 else
			    				 SipProxyApi.AddMR(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
			    		 }
			    	 }
			     }
	//Added durimg merge of 4.7.2
//		 for(int i=1; i<=4; i++)
//
//			     {
//					 pServiceParams = pIpServiceListManager->FindServiceByName(pMRoom->GetServiceRegistrationContentServiceName(i-1));
//					 if (pServiceParams != NULL)
//					 {
//						 if( pMRoom->GetServiceRegistrationContentRegister(i-1) == TRUE )
//						 {
//							if(pMRoom->GetEntryQ())
//								SipProxyApi.AddEQ(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
//							else if(pMRoom->IsSIPFactory())
//								SipProxyApi.AddFactory(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
//							else if(pMRoom->GetIsGateway())
//								SipProxyApi.AddGW(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
//							else
//								SipProxyApi.AddMR(pServiceParams->GetServiceId(), pMRoom->GetName(), pMRoom->GetMonitorConfId());
//						 }
//					 }
//			     }
			  }
  		   }

  		   else  //UpdateMR
  		   {
  		   		TRACESTR(eLevelInfoNormal) << "ConfPartyManager::OnFailoverAddOrUpdateMeetingRoomInd Update a specified MR" ;
  				STATUS status=STATUS_OK;

			    status=UpdateMR(pMRoom);

				if (STATUS_OK != status)
				{
				    TRACESTR(eLevelError) << "ConfPartyManager::OnFailoverAddOrUpdateMeetingRoomInd Update the MR was Failed !!!!" ;
    				PASSERT(1);
    				PDELETEA(pMeetingRoomElementStr);
    				POBJDELETE(pDom);
					POBJDELETE(pMrShort);
    				return;
  				}

  		   }

  		   int isMRActive=pMRoom->GetMeetingRoomState();

		   if( isMRActive && (!(pMRoom->GetEntryQ())) && (!(pMRoom->IsSIPFactory())) && (!(pMRoom->GetIsGateway())) )
		   {
			 OPCODE opcode = SLAVE_ACTIVATE_MR_REQ;
			 CSegment *pToRsrcSeg = new CSegment;
 			 pMRoom->Serialize(NATIVE, *pToRsrcSeg);
			 SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, opcode);
		   }

  		   POBJDELETE(pMRoom);
		   POBJDELETE(pMrShort);
		}
	}

    PDELETEA(pMeetingRoomElementStr);
    POBJDELETE(pDom);
}

////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverTerminateMeetingRoomInd(CSegment* pSeg)
{
    DWORD mrId;
    *pSeg >> mrId;

    PTRACE2INT(eLevelInfoNormal, "ConfPartyManager::OnFailoverTerminateMeetingRoomInd - mrId = ", mrId);

    CCommResShort*  pMrShort = ::GetpMeetingRoomDB()->GetCurrentRsrvShort(mrId);

    if (!CPObject::IsValidPObjectPtr(pMrShort))
	{
		DBGPASSERT(mrId);
		delete pMrShort;
		return;
	}

    CSegment *pToRsrcSeg = new CSegment;
    *pToRsrcSeg << mrId ;
    SendMsgToRsrvMngrRsrcProcess (pToRsrcSeg, DEL_MR_REQ);

    STATUS status=::GetpMeetingRoomDB()->Cancel(mrId);

	char* transitEQ = new char[H243_NAME_LEN];

	if (transitEQ == NULL)
	{
		PASSERT(1);
		delete pMrShort;
		return;
	}

	memset(transitEQ, 0, H243_NAME_LEN);

	char* pName = new char[H243_NAME_LEN];

	if (pName == NULL)
	{
		PASSERT(1);
		DEALLOCBUFFER(transitEQ);
		delete pMrShort;
		return;
	}

	memset(pName, 0, H243_NAME_LEN);

    strncpy(pName, pMrShort->GetName(), H243_NAME_LEN);
	strncpy(transitEQ,::GetpMeetingRoomDB()->GetTransitEQName(),H243_NAME_LEN);

	pName[H243_NAME_LEN - 1] = '\0';
	transitEQ[H243_NAME_LEN - 1] = '\0';

	if(!strcmp(pName,transitEQ))
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnFailoverTerminateMeetingRoomInd, The deleted room is the transit EQ");
		::GetpMeetingRoomDB()->CancelTransitEQ();
	}

    if ( status != STATUS_OK)
	{
	    PTRACE(eLevelError, "CConfPartyManager::OnFailoverTerminateMeetingRoomInd :  Failed Delete a meeting room on MR DB");
	    PASSERT(1);
	}
    else
	{
	    CSipProxyManagerApi SipProxyApi;
	    CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	    for( int i=1; i<=4;i++ )
	    {
		CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(i);
		if( pServiceParams == NULL )
		   continue;
	        SipProxyApi.DelConference(i, pName, mrId);
	    }
	}

	DEALLOCBUFFER(transitEQ);
	DEALLOCBUFFER(pName);
	delete pMrShort;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverIVRServiceListInd(CSegment* pSeg)
{
	PTRACE(eLevelError,"CConfPartyManager::OnFailoverIVRServiceListInd " );

	if (m_slaveSyncElements[eSlaveSyncIvrServices] == FALSE)
	{
		PTRACE(eLevelError,"CConfPartyManager::OnFailoverIVRServiceListInd  - First Sync");
		m_pIVRManager->DeleteIVRServices();
		m_pIVRManager->InitIVRConfig();
		m_slaveSyncElements[eSlaveSyncIvrServices] = TRUE;
	}

	DWORD len;
    *pSeg >> len;

    char* pIVRServiceListElementStr = new char[len+1];
    pIVRServiceListElementStr[len] = '\0';
    *pSeg >> pIVRServiceListElementStr;

    CXMLDOMDocument *pDom = new CXMLDOMDocument;
    if (pDom->Parse((const char **)&pIVRServiceListElementStr) == SEC_OK)
	{
		CXMLDOMElement* pRoot = pDom->GetRootElement();
		if(pRoot)
		{

			CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();
			if (NULL == pAVmsgServiceList)
			{
				PTRACE(eLevelError,"CConfPartyManager::OnFailoverIVRServiceListInd - Internal Error - Illegal pointer to IVR List" );
				POBJDELETE(pDom);
				PDELETEA(pIVRServiceListElementStr);
				return;
			}

			pAVmsgServiceList->DeleteServices();
			//save the update counter
			DWORD originalUpdateCounter = pAVmsgServiceList->GetUpdateCounter();

			char szErrorMsg[ERROR_MESSAGE_LEN];
			pAVmsgServiceList->DeSerializeXml(pRoot, szErrorMsg);
			pAVmsgServiceList->SetUpdateCounter(originalUpdateCounter);
		}
	}
    PDELETEA(pIVRServiceListElementStr);
    POBJDELETE(pDom);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnFailoverRecordingLinksListInd(CSegment* pSeg)
{
	PTRACE(eLevelError,"CConfPartyManager::OnFailoverRecordingLinksListInd " );

	if (m_slaveSyncElements[eSlaveSyncRecordingLinks] == FALSE)
	{
		PTRACE(eLevelError,"CConfPartyManager::OnFailoverRecordingLinksListInd  - First Sync");
		CRecordingLinkDB* pDeletedRecordingDB = ::GetRecordingLinkDB();
		pDeletedRecordingDB->ResetDB();
		m_slaveSyncElements[eSlaveSyncRecordingLinks] = TRUE;
	}

	DWORD len;
    *pSeg >> len;

    char* pRecordingLinksListElementStr = new char[len+1];
    pRecordingLinksListElementStr[len] = '\0';
    *pSeg >> pRecordingLinksListElementStr;

    CXMLDOMDocument *pDom = new CXMLDOMDocument;
    if (pDom->Parse((const char **)&pRecordingLinksListElementStr) == SEC_OK)
	{
		CXMLDOMElement* pRoot = pDom->GetRootElement();
		if(pRoot)
		{
			CRecordingLinkDB* pRecordingLinkDB = ::GetRecordingLinkDB();
			if (NULL == pRecordingLinkDB)
			{
				PTRACE(eLevelError,"CConfPartyManager::OnFailoverRecordingLinksListInd - Internal Error - Illegal pointer to RecrdingLink List" );
				POBJDELETE(pDom);
				PDELETEA(pRecordingLinksListElementStr);
				return;
			}

			pRecordingLinkDB->ResetDB();
			DWORD originalUpdateCounter = pRecordingLinkDB->GetUpdateCounter();//save the update counter

			char szErrorMsg[ERROR_MESSAGE_LEN];
			pRecordingLinkDB->DeSerializeXml(pRoot, szErrorMsg ,"");
			pRecordingLinkDB->SetUpdateCounter(originalUpdateCounter);
		}
	}
    POBJDELETE(pDom);
    PDELETEA(pRecordingLinksListElementStr);
}

//=====================================================================================================================================//
STATUS CConfPartyManager::OnSetExclusiveContent(CRequest *pRequest)//Restricted content
{
  PTRACE(eLevelInfoNormal,"CConfPartyManager::OnSetExclusiveContent");
  if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
  {
        FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnSetExclusiveContent: No permission to OnSetExclusiveContent for administrator readony");
        pRequest->SetConfirmObject(new CDummyEntry());
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_NO_PERMISSION;
  }
  STATUS status = STATUS_OK;

  CRsrvPartyAction* pRsrvPartyAction = new CRsrvPartyAction;

  *pRsrvPartyAction = *(CRsrvPartyAction*)pRequest->GetRequestObject();
  pRequest->SetObjectFlag(STRING_FLAG);

  const DWORD confId = pRsrvPartyAction->GetConfID();
  const DWORD partyId = pRsrvPartyAction->GetPartyID();

  // get part name from DB by party id and conference id

  // Romem klocwork
  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
  if(pCommConf == NULL)
  {
	  status = STATUS_CONF_NOT_EXISTS;
  }
  if(status == STATUS_OK)
  {
	  const char* partyName = ::GetpConfDB()->GetPartyName(confId, partyId);
	  if(partyName == NULL)
	  {
		  TRACEINTO << "party not found, confId = " << confId << " , partyId = " << partyId;
		  status = STATUS_PARTY_DOES_NOT_EXIST;
	  }
	  else
	  {
		  BOOL bRestrictContentBroadcastToLecturer = FALSE;
		  CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("RESTRICT_CONTENT_BROADCAST_TO_LECTURER", bRestrictContentBroadcastToLecturer);
		  CLectureModeParams* pLectureModeParams = pCommConf ? pCommConf->GetLectureMode() : NULL;
          if (bRestrictContentBroadcastToLecturer && CPObject::IsValidPObjectPtr(pLectureModeParams)
					&& pLectureModeParams->GetLectureModeType() && !pLectureModeParams->GetAudioActivated() && strlen(pLectureModeParams->GetLecturerName()))
		  {
			 status = STATUS_CANNOT_CHANGE_CONTENT_OWNER_IN_LECTURE_MODE;
		  }
		  else
		  {
			  // Send event to Conference
			  CConfApi confApi;
			  //CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
			  confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));

			  confApi.SetExclusiveContentOn(partyName);
			  confApi.DestroyOnlyApi();}
	     }
  }
  std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_CONF_2
  pRequest->SetTransName(responseTrancsName);
  pRequest->SetStatus(status);
  pRequest->SetConfirmObject(pRsrvPartyAction);

  return STATUS_OK;
}
//=====================================================================================================================================//
STATUS CConfPartyManager::OnRemoveExclusiveContent(CRequest *pRequest)//Restricted content
{
  PTRACE(eLevelInfoNormal,"CConfPartyManager::OnRemoveExclusiveContent");
  if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
  {
        FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnRemoveExclusiveContent: No permission to OnRemoveExclusiveContent for administrator readony");
        pRequest->SetConfirmObject(new CDummyEntry());
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_NO_PERMISSION;
  }

  STATUS status = STATUS_OK;
  CConfAction* pCConfAction = new CConfAction;

  *pCConfAction = *(CConfAction*)pRequest->GetRequestObject();
  pRequest->SetObjectFlag(STRING_FLAG);

  const DWORD confId = pCConfAction->GetConfID();

  // Send event to Conference

  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confId);
  // Romem klocwork
  if(pCommConf == NULL)
  {
	  status = STATUS_CONF_NOT_EXISTS;
  }
  else
  {
	  CConfApi confApi;
	  confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));

	  confApi.RemoveExclusiveContent();
	  confApi.DestroyOnlyApi();
  }
  std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_CONF_2
  pRequest->SetTransName(responseTrancsName);
  pRequest->SetStatus(status);
  pRequest->SetConfirmObject(pCConfAction);

  return STATUS_OK;
}
//=====================================================================================================================================//
STATUS CConfPartyManager::HandleSetExclusiveContent(CTerminalCommand & command, std::ostream& answer)//Restricted content
{
  PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleSetExclusiveContent ");

  DWORD numOfParams = command.GetNumOfParams();
  if(2 != numOfParams)
  {
    answer << "Error: action must be specified: SetExclusiveContent\n";
    answer << "usage: Bin/McuCmd SetExclusiveContent ConfParty [Conf ID] [Party ID] \n";
    return STATUS_FAIL;
  }

  const string &confId = command.GetToken(eCmdParam1);
  const string &partyId = command.GetToken(eCmdParam2);

  DWORD confID = atoi(confId.c_str());
  DWORD partyID = atoi(partyId.c_str());

  // get part name from DB by party id and conference id
  const char * partyName = ::GetpConfDB()->GetPartyName(confID,partyID);
  if (NULL == partyName)
  {
    answer << "Error: Party name is NULL, confID=" << confID << ", partyID=" << partyID << ", SetExclusiveContent ";
    return STATUS_FAIL;
  }

    // Send event to Conference
  CConfApi confApi;

  CCommConfDB* pCommConfDB = ::GetpConfDB();
  CCommConf* pRequestedConf = NULL;

  pRequestedConf = pCommConfDB->GetCurrentConf(confID);
  if (!CPObject::IsValidPObjectPtr(pRequestedConf))
  {
    answer << "Error: Conf does not exist in DB confID=" << confID << " " << "SetExclusiveContent ";
    return STATUS_FAIL;
  }

  DWORD status = ::GetpConfDB()->SearchPartyName(confID, partyID);
  if (status != STATUS_OK)
  {
    answer << "Error: Party does not exist in DB partyID=" << partyID << " " << "SetExclusiveContent ";
    return STATUS_FAIL;
  }
  confApi.CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));
  confApi.SetExclusiveContentOn(partyName);
  confApi.DestroyOnlyApi();

  return STATUS_OK;
}

//=====================================================================================================================================//
STATUS CConfPartyManager::OnServerSetExclusiveContentMode(CRequest *pRequest)//Restricted content
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
          FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetExclusiveContentMode: No permission to OnServerSetExclusiveContentMode for administrator readony");
          pRequest->SetConfirmObject(new CDummyEntry());
          pRequest->SetStatus(STATUS_NO_PERMISSION);
          return STATUS_NO_PERMISSION;
     }
	STATUS status = STATUS_OK;
	CConfAction* pConfAction = new CConfAction;
	*pConfAction = *(CConfAction*)pRequest->GetRequestObject() ;
	pRequest->SetObjectFlag(STRING_FLAG);

	const DWORD confId   = pConfAction->GetConfID();
	BYTE isExclusiveContentMode   = (pConfAction->GetNumAction())?YES:NO;
    PTRACE2INT (eLevelInfoNormal, "ConfPartyManager::OnServerSetExclusiveContentMode received ", isExclusiveContentMode);
    //ToDo - add test validity for the intervalValue
	/*** VALIDITY of conference Id ***/
	if (::GetpConfDB()->FindId(confId) == NOT_FIND)
		  status = STATUS_CONF_NOT_EXISTS;

	if (status==STATUS_OK)
	{
		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
		if (pCurConf)
		{
			pCurConf->SetExclusiveContentMode(isExclusiveContentMode);
            CConfApi confApi;
			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			confApi.SetExclusiveContentMode(isExclusiveContentMode);
			confApi.DestroyOnlyApi();

		}
	  else
	  {
		  PTRACE2INT(eLevelError,  "CConfPartyManager::OnServerSetExclusiveContentMode :  Conf Id invalid- ",confId );
		  DBGPASSERT(1);
		  status = STATUS_ILLEGAL;
	  }

	}
	else
	{
		//Trace
		PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerSetExclusiveContentMode ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		DBGPASSERT(status);
	}

  	std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}
//=====================================================================================================================================//
STATUS CConfPartyManager::HandleRemoveExclusiveContent(CTerminalCommand & command, std::ostream& answer)//Restricted content
{
  PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleRemoveExclusiveContent ");

  DWORD numOfParams = command.GetNumOfParams();
  if(1 != numOfParams)
  {
    answer << "Error: action must be specified: RemExclusiveContent \n";
    answer << "usage: Bin/McuCmd RemExclusiveContent ConfParty [Conf ID] \n";
    return STATUS_FAIL;
  }

  const string &confId = command.GetToken(eCmdParam1);

  DWORD confID = atoi(confId.c_str());

    // Send event to Conference
  CConfApi confApi;
  CCommConfDB* pCommConfDB = ::GetpConfDB();
  CCommConf* pRequestedConf = NULL;

  pRequestedConf = pCommConfDB->GetCurrentConf(confID);
  if (!CPObject::IsValidPObjectPtr(pRequestedConf))
  {
    answer << "Error: Conf does not exist in DB confID=" << confID << " " << "RemExclusiveContent ";
    return STATUS_FAIL;
  }

  confApi.CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));
  confApi.RemoveExclusiveContent();
  confApi.DestroyOnlyApi();

  return STATUS_OK;
}

//=====================================================================================================================================//
STATUS CConfPartyManager::OnServerSetMuteIncomingLectureMode(CRequest *pRequest)//Restricted content
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
          FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetMuteIncomingLectureMode: No permission to OnServerSetMuteIncomingLectureMode for administrator readony");
          pRequest->SetConfirmObject(new CDummyEntry());
          pRequest->SetStatus(STATUS_NO_PERMISSION);
          return STATUS_NO_PERMISSION;
    }

	STATUS status = STATUS_OK;
	CConfAction* pConfAction = new CConfAction;
	*pConfAction = *(CConfAction*)pRequest->GetRequestObject() ;
	pRequest->SetObjectFlag(STRING_FLAG);

	const DWORD confId   = pConfAction->GetConfID();
	BYTE isMuteIncoming   = (pConfAction->GetNumAction())?YES:NO;
    PTRACE2INT (eLevelInfoNormal, "ConfPartyManager::OnServerSetMuteIncomingLectureMode received ", isMuteIncoming);
    //ToDo - add test validity for the intervalValue
	/*** VALIDITY of conference Id ***/
	if (::GetpConfDB()->FindId(confId) == NOT_FIND)
		  status = STATUS_CONF_NOT_EXISTS;

	if (status==STATUS_OK)
	{
		CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
		if (pCurConf)
		{
			pCurConf->SetMuteIncomingPartiesLectureMode(isMuteIncoming);
            CConfApi confApi;
			confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
			confApi.SetMuteIncomingLectureMode(isMuteIncoming);
			confApi.DestroyOnlyApi();

		}
	  else
	  {
		  PTRACE2INT(eLevelError,  "CConfPartyManager::OnServerSetMuteIncomingLectureMode :  Conf Id invalid- ",confId );
		  DBGPASSERT(1);
		  status = STATUS_ILLEGAL;
	  }

	}
	else
	{
		//Trace
		PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerSetMuteIncomingLectureMode ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());
		DBGPASSERT(status);
	}

  	std::string responseTrancsName("TRANS_CONF_2"); //instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetMuteAllAudioVideoPartiesExceptLeader(CRequest *pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
	{
		TRACEINTO << "No permission to OnServerSetExclusiveContentMode for administrator read only";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;
	CConfAction* pConfAction = new CConfAction;
	*pConfAction = *(CConfAction*)pRequest->GetRequestObject();
	pRequest->SetObjectFlag(STRING_FLAG);
	ConfMonitorID confId   = pConfAction->GetConfID();
	if (::GetpConfDB()->FindId(confId) == NOT_FIND)
		  status = STATUS_CONF_NOT_EXISTS;
	if (::GetpConfDB()->IsConfSecured(confId))
		status = STATUS_CONF_IS_SECURED;
	if (CProcessBase::GetProcess()->GetProductType() != eProductTypeSoftMCUMfw)
	{
		status = STATUS_NO_PERMISSION;
	}
	if (status==STATUS_OK)
	{
		int isAudioMute = pConfAction->GetNumAction();
		int isVideoMute = pConfAction->GetNumAction1();
		int isMuteIncludeExistingUsers  = pConfAction->GetNumAction2();
	    if (isVideoMute != -1 || isAudioMute != -1) // isVideoMute = -1 || isAudioMute = -1 in case one of the fields are missing in SET_AUDIO_VIDEO_MUTE_PARTIES_EXCEPT_LEADER transaction
	    {
			TRACEINTO << "ConfId:" << confId << " isAudioMute:" << (WORD)isAudioMute << " isVideoMute:" << (WORD)isVideoMute << " isMuteIncludeExistingUsers:" << (WORD)isMuteIncludeExistingUsers;
			CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confId);
			if (pCurConf)
			{
				CConfApi confApi;
				confApi.CreateOnlyApi(*(pCurConf->GetRcvMbx()));
				if (isMuteIncludeExistingUsers != -1)
				{
					if (isAudioMute != -1)
					{
						if (isAudioMute == YES)
						{
							confApi.UpdateMuteAllAudioExceptLeader(YES, (BYTE)isMuteIncludeExistingUsers);
						}
						else
						{
                            confApi.UpdateUnMuteAllAudioExceptLeader(YES, (BYTE)isMuteIncludeExistingUsers);
						}
					}
					if (isVideoMute != -1)
					{
						if (isMuteIncludeExistingUsers == NO)
						{
							confApi.UpdateMuteAllIncomingVideoExceptLeader((BYTE)isVideoMute);
						}
						else
						{
							confApi.UpdateMuteAllVideoExceptLeader((BYTE)isVideoMute);
						}
					}
				}
				confApi.DestroyOnlyApi();
			}
			else
			{
				DBGPASSERT(confId);
				status = STATUS_ILLEGAL;
			}
	    }
	    else
	    {
	    	TRACEINTO << "ConfId:" << confId << " isAudioMute:" << (WORD)isAudioMute << " isVideoMute:" << (WORD)isVideoMute << ", Do nothing!";
	    }
	}
	else
	{
		TRACEINTO << "status:"<< CProcessBase::GetProcess()->GetStatusAsString(status).c_str();
		DBGPASSERT(status);
	}
  	std::string responseTrancsName("TRANS_CONF_2"); //instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pConfAction);
	pRequest->SetStatus(status);
	return status;
}
STATUS CConfPartyManager::OnServerSetDtmf(CRequest *pRequest)
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
          FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetDtmf: No permission to OnServerSetDtmf for administrator readony");
          pRequest->SetConfirmObject(new CDummyEntry());
          pRequest->SetStatus(STATUS_NO_PERMISSION);
          return STATUS_NO_PERMISSION;
    }
	if ((CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator) &&
			(CProcessBase::GetProcess()->GetProductType() != eProductTypeCallGeneratorSoftMCU))
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetDtmf - ERROR - system is not CG!!");
		return STATUS_ILLEGAL;
	}

	PTRACE(eLevelInfoNormal, "ConfPartyManager::OnServerSetDtmf");
	STATUS status = STATUS_OK;

	CClientSendDtmf* pSendDtmf = new CClientSendDtmf;

   *pSendDtmf = *(CClientSendDtmf*)pRequest->GetRequestObject() ;
    pRequest->SetObjectFlag(STRING_FLAG);

   const DWORD confId   		= pSendDtmf->GetConfID();
   const DWORD partyMonitorId 	= pSendDtmf->GetPartyID();
   const DWORD dtmfDirection	= pSendDtmf->GetDtmfDirection();
   const char *dtmfString		= pSendDtmf->GetDtmfString();
   const char *partyMonitirIdString		= pSendDtmf->GetPartyMonitorIdString();
   const char *confIdString		= pSendDtmf->GetConfIdString();

	if (NULL == dtmfString)
	{
		PTRACE(eLevelError,  "CConfPartyManager::OnServerSetDtmf :  DTMF invalid" );
		DBGPASSERT(1);
		status = STATUS_ILLEGAL;
	}

	if (status==STATUS_OK)
	   if (0 == strlen(dtmfString))
	   {
			PTRACE(eLevelError,  "CConfPartyManager::OnServerSetDtmf :  DTMF empty" );
			DBGPASSERT(1);
			status = STATUS_ILLEGAL_DTMF_CODE_LEN;
	   }

	const CCommConf*  pCurCommConf = NULL;
	if (status==STATUS_OK)
	{
	  	/*** get conference from DB  ***/
		pCurCommConf  = ::GetpConfDB()->GetCurrentConf(confId);
		if (NULL == pCurCommConf)
		{
				PTRACE(eLevelError,  "CConfPartyManager::OnServerSetDtmf :  conf not found" );
				DBGPASSERT(1);
				status = STATUS_CONF_NOT_EXISTS;
		}
	}

	const CConfParty* pCurConfParty = NULL;
	if (status==STATUS_OK)
    {
	  	/*** get party from DB  ***/
		pCurConfParty = pCurCommConf->GetCurrentParty(partyMonitorId);
		if (NULL == pCurConfParty)
		{
				PTRACE(eLevelError,  "CConfPartyManager::OnServerSetDtmf :  party not found in conf" );
				DBGPASSERT(1);
				status = STATUS_PARTY_DOES_NOT_EXIST;
		}
    }

    char* curPartyName = NULL;
    if (status==STATUS_OK)
	{
 		curPartyName = (char* )pCurConfParty->GetName();
		if (NULL == curPartyName)
		{
				PTRACE(eLevelError,  "CConfPartyManager::OnServerSetDtmf :  party name is NULL" );
				DBGPASSERT(1);
				status = STATUS_ILLEGAL_PARTY_NAME;
		}
 	}

 	 if (status==STATUS_OK)
  	{
		CSegment param;
		param << dtmfDirection;
		param << dtmfString;
		param << confIdString;
		param << partyMonitirIdString;

		CConfApi confApi;
		confApi.CreateOnlyApi(*(pCurCommConf->GetRcvMbx()));
		confApi.SendDtmfFromClient( &param, curPartyName );
		confApi.DestroyOnlyApi();
  	}

  	PTRACE2(eLevelInfoNormal,"CConfPartyManager::OnServerSetDtmf, ", CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

	  	//New Confirm
 	std::string responseTrancsName("TRANS_CONF"); // insteade of TRANS_CONF_2
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pSendDtmf);
	pRequest->SetStatus(status);

	return STATUS_OK;

}

//=====================================================================================================================================//
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::SyncExchangeProcessNotification(CCommResApi& newConf)
{
	if (m_bExchangeConfigured == FALSE)
		return STATUS_OK;

	CSegment *pSeg = new CSegment;
	newConf.Serialize(NATIVE, *pSeg);

	CTaskApi exchangeMonitorApi(eProcessExchangeModule, eMonitor);

	CSegment rspMsg;
	OPCODE resOpcode;

	STATUS responseStatus = exchangeMonitorApi.SendMessageSync(pSeg, EXCHNAGE_UPDATE_CONF_DETAILS_REQ, EXCHANGE_SERVER_UPDATE_CONF_PATAMS_TOUT, resOpcode, rspMsg);

	if (STATUS_OK == responseStatus)
	{
		if (rspMsg.GetLen() > 0)
		{
			CCommResApi* pUpdatedConf = new CCommResApi(newConf);
			CIstrStream istream(rspMsg);
			pUpdatedConf->DeSerialize(NATIVE, istream);

			char buff[128] = { 0 };
			pUpdatedConf->GetExchangeConfStartTime()->DumpToBuffer(buff);

			WORD tmp;
			istream >> tmp;
			STATUS statusFromExchangeProcess = tmp;

			TRACEINTO
				<< "\n  DisplayName       :" << pUpdatedConf->GetDisplayName()
				<< "\n  NumericConfId     :" << pUpdatedConf->GetNumericConfId()
				<< "\n  AppointmentId     :" << pUpdatedConf->GetAppointmentId()
				<< "\n  MeetingOrganizer  :" << pUpdatedConf->GetMeetingOrganizer()
				<< "\n  GatheringEnabled  :" << ((pUpdatedConf->IsGatheringEnabled() == YES) ? "YES" : "NO")
				<< "\n  AudioNumber1      :" << pUpdatedConf->GetNumberAccess_1()
				<< "\n  AudioNumber2      :" << pUpdatedConf->GetNumberAccess_1()
				<< "\n  ExchangeStartTime :" << buff
				<< "\n  EnableRecording   :" << ((pUpdatedConf->GetEnableRecording() == YES) ? "YES" : "NO")
				<< "\n  IsStreaming       :" << ((pUpdatedConf->GetIsStreaming() == YES) ? "YES" : "NO")
				<< "\n  Status            :" << statusFromExchangeProcess;

			if (statusFromExchangeProcess != STATUS_OK)
			{
				TRACEINTO << "ConfName:" << newConf.GetName() << " - Conference is not found in Exchange process";
			}
			else
			{
				TRACEINTO << "ConfName:" << newConf.GetName() << " - Conference was founs and updated in Exchange process";
				newConf = *pUpdatedConf;

				std::string newName = pUpdatedConf->GetDisplayName();
				newName += "_";
				newName += pUpdatedConf->GetNumericConfId();
				newConf.SetDisplayName(newName.c_str());
			}
			POBJDELETE(pUpdatedConf);
		}
		else
		{
			PASSERTSTREAM_AND_RETURN_VALUE(true, "ConfName:" << newConf.GetName() << " - Update Conf in Exchange server failed", STATUS_ILLEGAL);
		}
	}
	else  // timeout or dead process
	{
		PASSERTSTREAM_AND_RETURN_VALUE(true, "ConfName:" << newConf.GetName() << " - Update Conf in Exchange Server TIME OUT", responseStatus);
	}
	return STATUS_OK;
}

//=====================================================================================================================================//
STATUS  CConfPartyManager::HandleChangeSysMode(CTerminalCommand & command, std::ostream& answer)
{//2 modes cop/cp
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleChangeSysMode ");

  DWORD numOfParams = command.GetNumOfParams();
  if (1 != numOfParams)
  {
    answer << "error: action must be specified:ChangeSysMode\n";
    answer << "usage: Bin/McuCmd ChangeSysMode ConfParty SysMode\n";
    return STATUS_FAIL;
  }
  BOOL IsCOP;
  const string &action = command.GetToken(eCmdParam1);

  if (action == "cop" || action == "Cop" || action == "COP")
  {
    IsCOP = TRUE;
    answer << "Setting system mode to COP";
  }
  else
  {
    if (action == "cp" || action == "Cp" || action == "CP")
    {
      IsCOP = FALSE;
      answer << "Setting system mode to CP";
    }
    else
    {
      IsCOP = FALSE;
      PASSERTSTREAM(2233, "Wrong sys mode " << action);
    }
  }
  ::SetIsCOPdongleSysMode(IsCOP);

  //For reservations
  CSegment * pRSRSSeg = new CSegment;
  *pRSRSSeg << IsCOP;
  SendAsyncMsgToRsrcProcess(pRSRSSeg, RSRC_CHANGE_SYS_MODE_REQ);

  SendCardConfigReq();

  //for existing elements:
  ::GetpMeetingRoomDB()->ChkAllRsrvSysMode();
  ::GetpProfilesDB()->ChkAllRsrvSysMode();

  return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleConfPartyProcessInfo(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleConfPartyProcessInfo ");

	std::string result, cmd, info;
	STATUS stat = STATUS_OK;
    if(YES==IsTarget() && (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() ) )
    {
    	cmd = "ps -eo pid,ppid,pgid,comm,nice,vsz,etime | grep ConfParty";
    	stat = SystemPipedCommand(cmd.c_str(), result);
    	answer << "\n | pid | ppid | pgid | comm | nice | vsz | etime |\n";
    }
    else
    {
	cmd = "ps -eLo pid,tid,comm,stat,pcpu,size,rssize,vsize,stackp,start_time,etime | grep ConfParty";
    	stat = SystemPipedCommand(cmd.c_str(), result);
	answer << "\n | pid | tid | name | status | pcpu | size | resident mem | virtual mem | stackp | start_time | end_time |\n";
    }

	answer << result.c_str();
	return STATUS_OK;
}

STATUS CConfPartyManager::HandleProcessMemInfoPrint(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleProcessMemInfoPrint ");

	DWORD numOfParams = command.GetNumOfParams();
	if ( numOfParams < 1) {
		answer << "error: action must be specified: ProcessMemInfoPrint\n";
		answer << "usage: Bin/McuCmd ProcessMemInfoPrint ConfParty [YES/NO] [TIME_SEC]\n";
		return STATUS_FAIL;
	}
	const string &e_flag = command.GetToken(eCmdParam1);
	BYTE enbl = (e_flag == "YES" ? TRUE : FALSE);
	DWORD interval_sec = 0;
	if( enbl && numOfParams > 1)
	{
		const string &interval = command.GetToken(eCmdParam2);
		interval_sec = atoi(interval.c_str());
	}
	if( IsValidPObjectPtr(m_pAssistMngApi) )
	{
		CSegment *pDummy = new CSegment;
		*pDummy << enbl;
		*pDummy << interval_sec;
		m_pAssistMngApi->SendMsg(pDummy, PROCESS_MEMINFO_PRINT);
	}
	answer << (enbl ? "Print timer will be run" : "Print timer will be deleted");
	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfPartyManager::OnSetResolutionThreshold(CRequest* pSetRequest)
{
  STATUS ret_status = STATUS_OK;

	if (pSetRequest->GetAuthorization() !=  SUPER)
	{
	    FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnSetResolutionThreshol: No permission to set tResolutionThreshold");
	    ret_status = STATUS_NO_PERMISSION;
	}
	else if (::GetIsCOPdongleSysMode())
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnSetResolutionThreshold - Resolution Slider disabled in COP mode");
		ret_status = STATUS_ILLEGAL_SYS_MODE;
	}

	if(::GetpConfDB()->GetConfNumber() != 0 )
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnSetResolutionThreshold - Block setting due active conferences exist");
		ret_status = STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_EXISTS;
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnSetResolutionThreshold ");

		eSystemCardsMode systemCardMode = GetSystemCardsBasedMode();
		CSetResolutionSliderDetails* pResolutionSlider = (CSetResolutionSliderDetails*)pSetRequest->GetRequestObject();

		if (CResRsrcCalculator::IsRMX1500Q() && !CResRsrcCalculator::IsHDenabled() &&
			(EResolutionType)((CResolutionSliderDetails*)pResolutionSlider)->GetMaxCPResolution() > e_sd30)
		{
			PTRACE(eLevelInfoNormal,"CConfPartyManager::OnSetResolutionThreshold : block settings due to HD license restrictions");
			ret_status = STATUS_1500Q_NO_CP_HD_LICENSE;
		}
		else
		{
			CSegment * pRASeg = new CSegment;

			CResRsrcCalculator resRsrcCalculator;
			BOOL wasChanged = resRsrcCalculator.UpdateResolutionConfiguration( systemCardMode, pResolutionSlider );

			if( wasChanged ) //need to update the profiles list for RA due the possible changes of maxVideoPartyType per profile
			{
				WORD numProfiles = ::GetpProfilesDB()->GetResNumber();
				WORD numProfilesAdded = 0;
				PROFILE_IND_LIST_S* pProfileList = new PROFILE_IND_LIST_S;
				pProfileList->list_size = numProfiles;
				pProfileList->profile_list = new PROFILE_IND_S[numProfiles];

				const CCommResDB::ReservArray& tempProfilesArray = ::GetpProfilesDB()->GetReservArray();

				std::ostringstream msg;
				CCommResDB::ReservArray::const_iterator itr_end=tempProfilesArray.end();
				for( CCommResDB::ReservArray::const_iterator itr=tempProfilesArray.begin(); itr != itr_end; ++itr)
				{
					if( (*itr) != NULL )
					{
						if( numProfilesAdded >= numProfiles ) {
							PASSERT(1); //there's an inconsistency between GetResNumber() and actual number of profiles
							break;
						}
						CCommRes* pCommRes = ::GetpProfilesDB()->GetCurrentRsrv((*itr)->GetConferenceId());
						if(CPObject::IsValidPObjectPtr(pCommRes))
						{
							pProfileList->profile_list[numProfilesAdded].profile_Id = (*itr)->GetConferenceId();
							eVideoPartyType vidtype = resRsrcCalculator.GetRsrcVideoType(systemCardMode, pCommRes);
							pProfileList->profile_list[numProfilesAdded].maxVideoPartyType = vidtype;
							msg	<< "\nProfileName:" << pCommRes->GetDisplayName()
								<< ", ProfileId:" << pProfileList->profile_list[numProfilesAdded].profile_Id
								<< ", MaxVideoPartyType:" << eVideoPartyTypeNames[vidtype];
							numProfilesAdded++;
							POBJDELETE(pCommRes);
						}
					}
				}

				TRACEINTO << msg.str().c_str();

				pRASeg->Put( (BYTE*) &(pProfileList->list_size),sizeof(DWORD));
				if( pProfileList->list_size )
					pRASeg->Put((BYTE *) (pProfileList->profile_list),sizeof(PROFILE_IND_S)* pProfileList->list_size);

				if( pProfileList->profile_list )
					delete [] pProfileList->profile_list;
				delete pProfileList;

				STATUS status = resRsrcCalculator.SaveResolutionConfigurationToFile( systemCardMode );
				if( status != STATUS_OK )
				{
					DBGPASSERT(status);
					PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnSetResolutionThreshold : ConfParty didn't success to write Resolution Slider Configuration file, status = ", status);
				}
				else //Send ASynch Msg to the RA process
				{
					SendAsyncMsgToRsrcProcess(pRASeg, UPDATE_RESOLUTION_THRESHOLD);
					pRASeg = NULL;
				}
			}

			delete pRASeg;
		}
	}

	CDummyEntry* pDummyEntry = new CDummyEntry;
	pSetRequest->SetConfirmObject(pDummyEntry);
	pSetRequest->SetStatus(ret_status);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::IceInitTimeout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::IceInitTimeout  ");
	m_pProcess->m_IceInitializationStatus = eIceStatusOFF;
	PTRACE(eLevelInfoNormal,"CConfPartyManager::IceInitTimeout Status Fail. ICE is OFF - allow SIP call without ICE  ");
}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnSipProxyEndIceInit(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnSipProxyEndIceInit  ");
	BYTE Status = STATUS_OK;
	BYTE IsEnableBWPolicyCheck = FALSE;
	DWORD maxVideoRateAllowed = 0;
	DWORD ICEType = 0;

//	*pParam >> Status >> IsEnableBWPolicyCheck >> maxVideoRateAllowed ;//>> (DWORD)ICEType;
	*pParam >> Status >> IsEnableBWPolicyCheck >> maxVideoRateAllowed >> ICEType;
	DeleteTimer(TIMER_START_ICE_INITIALIZATION);

	PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnSipProxyEndIceInit  ICEType -",ICEType);

	if(Status == STATUS_OK)
	{	//ICE status can be Register or off(incase the first registration failed, we will wait for success... and meanwhile will allow regular sip calls)
		if(m_pProcess->m_IceInitializationStatus == eIceStatusOFF ||
           m_pProcess->m_IceInitializationStatus == eIceStatusRegister)

		{
			m_pProcess->m_IceInitializationStatus = eIceStatusON;
			m_pProcess->m_IsEnableBWPolicyCheck = IsEnableBWPolicyCheck;
			m_pProcess->m_UcMaxVideoRateAllowed = maxVideoRateAllowed;
	//		m_pProcess->m_IceType = ICEType;

			PTRACE(eLevelInfoNormal,"CConfPartyManager::OnSipProxyEndIceInit is ON - allow ICE calls  ");
		}
	}
	else
	{
		m_pProcess->m_IceInitializationStatus = eIceStatusOFF;
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnSipProxyEndIceInit Status Fail. ICE is OFF - allow SIP call without ICE  ");
	}

	return STATUS_OK;

}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnWebRTCIceSeriveEndIceInit(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnWebRTCIceSeriveEndIceInit  ");
	BYTE Status = STATUS_OK;
	DWORD ICEType = 0;

	*pParam >> Status /*>> IsEnableBWPolicyCheck >> maxVideoRateAllowed*/ >> ICEType;
	/*DeleteTimer(TIMER_START_ICE_INITIALIZATION);*/

	PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnWebRTCIceSeriveEndIceInit  ICEType -",ICEType);

	if(Status == STATUS_OK)
	{
		if(m_pProcess->m_WebRTCIceInitializationStatus == eIceStatusOFF )
		{
			m_pProcess->m_WebRTCIceInitializationStatus = eIceStatusON;
			ON(m_isWebRtcGWStarted);
			PTRACE(eLevelInfoNormal,"N.A. DEBUG CConfPartyManager::OnSipProxyEndIceInit ON(m_isWebRtcGWStarted); ");
			//m_pProcess->m_IsEnableBWPolicyCheck = IsEnableBWPolicyCheck;
			//m_pProcess->m_UcMaxVideoRateAllowed = maxVideoRateAllowed;
			//m_pProcess->m_IceType = ICEType;

			PTRACE(eLevelInfoNormal,"CConfPartyManager::OnWebRTCIceSeriveEndIceInit is ON - allow ICE calls  ");
		}
	}
	else
	{
		m_pProcess->m_WebRTCIceInitializationStatus = eIceStatusOFF;
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnWebRTCIceSeriveEndIceInit Status Fail. ICE is OFF  ");
		m_isWebRtcGWStarted = NO;  //N.A. DEBUG - Set to reject incoming WebRtc Calls
	}

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnSipProxyEndIceStatus(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CConfPartyManager::OnSipProxyEndIceStatus  ");
	BYTE Status = STATUS_OK;
	DWORD ICEType = 0;

	*pParam >> Status >> ICEType;

	PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnSipProxyEndIceStatus  ICEType -",ICEType);
	if(ICEType != eIceEnvironment_WebRtc)
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnSipProxyEndIceStatus  Not WebRTc Call");
		ON(m_isWebRtcIceServerFailure);
		return STATUS_FAIL;
	}

	m_isWebRtcIceServerFailure = (Status == STATUS_OK) ? NO : YES ;

	PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnSipProxyEndIceStatus m_isWebRtcIceServerFailure = ", m_isWebRtcIceServerFailure);

	return Status;

}



/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnKillPortAck(CSegment* pSeg)
{
    TRACEINTO << "CConfPartyManager::OnKillPortAck => send message to Assist Manager Task";
	if( IsValidPObjectPtr(m_pAssistMngApi) )
	{
		CSegment *pDummy = new CSegment(*pSeg);
		m_pAssistMngApi->SendMsg(pDummy, KILL_PORT_REQ);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnAllocStatusPerUnitAck(CSegment* pSeg)
{
 	TRACEINTO << "CConfPartyManager::OnAllocStatusPerUnitAck  => send message to Assist Manager Task";
	if( IsValidPObjectPtr(m_pAssistMngApi) )
	{
		CSegment *pDummy = new CSegment(*pSeg);
		m_pAssistMngApi->SendMsg(pDummy, ALLOC_STATUS_PER_UNIT_REQ);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::AutoGeneratePasswords(CCommResApi* pRsrvApi) {
	PTRACE(eLevelInfoNormal,"CConfPartyManager::AutoGeneratePasswords - begin");
	BYTE btHidePassword = NO;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(
			"HIDE_CONFERENCE_PASSWORD", btHidePassword);
	if (btHidePassword == YES)
		return;
	char* pszConfPassword = (char*) pRsrvApi->GetEntryPassword();
	char* pszChairPassword = (char*) pRsrvApi->GetH243Password();
	if (strlen(pszConfPassword) == 0) {
		DWORD dwConfPasswordDefaultLength;
		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(
				NUMERIC_CONF_PASS_DEFAULT_LEN, dwConfPasswordDefaultLength);
		if (dwConfPasswordDefaultLength > 0) {
			DWORD dwConfPasswordMinLength;
			DWORD dwConfPasswordMaxLength;
			CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(
					NUMERIC_CONF_PASS_MAX_LEN, dwConfPasswordMaxLength);
			if (dwConfPasswordDefaultLength > dwConfPasswordMaxLength)
				dwConfPasswordDefaultLength = dwConfPasswordMaxLength;
			else {
				CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(
						"NUMERIC_CONF_PASS_MIN_LEN", dwConfPasswordMinLength);
				if (dwConfPasswordDefaultLength < dwConfPasswordMinLength)
					dwConfPasswordDefaultLength = dwConfPasswordMinLength;
			}
			AllocateStringOfRandomNumbers(dwConfPasswordDefaultLength,
					pszConfPassword);
			PTRACE(eLevelInfoNormal,"CConfPartyManager::AutoGeneratePasswords - Conf. Password created");
		}
	}
	if (strlen(pszChairPassword) == 0) {
		DWORD dwChairPasswordDefaultLength;
		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(
				NUMERIC_CHAIR_PASS_DEFAULT_LEN, dwChairPasswordDefaultLength);
		if (dwChairPasswordDefaultLength > 0) {
			DWORD dwChairPasswordMinLength;
			DWORD dwChairPasswordMaxLength;
			CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(
					NUMERIC_CHAIR_PASS_MAX_LEN, dwChairPasswordMaxLength);
			if (dwChairPasswordDefaultLength > dwChairPasswordMaxLength)
				dwChairPasswordDefaultLength = dwChairPasswordMaxLength;
			else {
				CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(
						"NUMERIC_CHAIR_PASS_MIN_LEN", dwChairPasswordMinLength);
				if (dwChairPasswordDefaultLength < dwChairPasswordMinLength)
					dwChairPasswordDefaultLength = dwChairPasswordMinLength;
			}
			AllocateStringOfRandomNumbers(dwChairPasswordDefaultLength,
					pszChairPassword);
			PTRACE(eLevelInfoNormal,"CConfPartyManager::AutoGeneratePasswords - Chair Password created");
		}
	}
	PTRACE(eLevelInfoNormal,"CConfPartyManager::AutoGeneratePasswords - end");
}

//////////////////////////////////////////////////////////////////////////////////////////
int CConfPartyManager::AllocateStringOfRandomNumbers(int iLength, char* pszRandomString)
{
  if (iLength >= MAX_RANDOM_NUMERIC_STRING)
    return STATUS_ILLEGAL;

  DWORD dwConfPasswordRepeatedDigits;
  CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("MAX_CONF_PASSWORD_REPEATED_DIGITS", dwConfPasswordRepeatedDigits);

  char szRandNum[MAX_RANDOM_NUMERIC_STRING + 5];

  for (int i = 0; i < iLength; ++i)
  {
	BOOL isFound = FALSE;
	while (!isFound)
	{
		char* p = NULL;

		memset(szRandNum, 0, sizeof(szRandNum));
		sprintf(szRandNum, "%d", rand());
		for (p = szRandNum; *p; ++p)
		{
			// Check on MAX_CONF_PASSWORD_REPEATED_DIGITS
			if (!IsRepeatedDigitsNextDigitOK(pszRandomString, *p, dwConfPasswordRepeatedDigits))
				continue;

			pszRandomString[i] = *p;
			isFound = TRUE;
			if (i == iLength - 1)
			{
				pszRandomString[iLength] = '\0';
				return STATUS_OK;
			}
			break;
		}
	}
  }
  return STATUS_ILLEGAL;
}

//////////////////////////////////////////////////////////////////////
bool CConfPartyManager::IsRepeatedDigitsNextDigitOK(char* psz,
		char chNextDigit, int nRepeatedMax) {
	int n = strlen(psz);
	if (n < nRepeatedMax)
		return true;
	if (nRepeatedMax == 0)
		return true;

	for (int i = n - 1 - (nRepeatedMax -1); i < n; ++i) {
		if (psz[i] != chNextDigit)
			return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnCsProxyConfRegisterStatus(CSegment* pSeg)//sipProxySts
{
	int  serviceIndex;
    WORD  serviceID = (WORD)-1;
    DWORD EntityID = (DWORD)-1;
    WORD  EntityType = (WORD)-1;
    DWORD registrationStatus = eSipRegistrationTotalStatusTypeNotConfigured;

    *pSeg 	>> serviceID
			>> EntityID
    		>> EntityType
    		>> registrationStatus;

	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(serviceID);
	if (pServiceParams == NULL)
	{
		PASSERTMSG(2,"CConfPartyManager::OnCsProxyConfRegisterStatus - IP Service does not exist!!!");
		return STATUS_FAIL;
	}

	char * Service_nm = (char*)pServiceParams->GetServiceName();
    PTRACE2(eLevelInfoNormal, "CConfPartyManager::OnCsProxyConfRegisterStatus - service nm = ", Service_nm);


    PTRACE2INT(eLevelInfoNormal, "CConfPartyManager::OnCsProxyConfRegisterStatus - EntityId = ", EntityID);
    PTRACE2INT(eLevelInfoNormal, "CConfPartyManager::OnCsProxyConfRegisterStatus - serviceID = ", serviceID);
    PTRACE2INT(eLevelInfoNormal, "CConfPartyManager::OnCsProxyConfRegisterStatus - EntityType = ", EntityType);
    PTRACE2INT(eLevelInfoNormal, "CConfPartyManager::OnCsProxyConfRegisterStatus - registrationStatus = ", registrationStatus);

    if ( eSipRegistrationConfTypeOngoing == (eSipRegistrationConfType)EntityType )
    {
    	CCommConfDB* pCommConfDB = ::GetpConfDB();
    	CCommConf* pRegisteredEntity = pCommConfDB->GetCurrentConf(EntityID);
    	if (!CPObject::IsValidPObjectPtr(pRegisteredEntity))
    	{
    		PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnCsProxyConfRegisterStatus Error!!! Conf does not exist in DB ID=",EntityID);
			return STATUS_FAIL;
    	}
    	else
    	{
    		serviceIndex = pRegisteredEntity->GetServiceRegistrationContentServiceIndexByName(Service_nm);
    		if (serviceIndex < 0)
    		{
    			FTRACEINTOFUNC<<" Error!!! Service name doesn't found, Service_nm = "<<Service_nm;
    			return STATUS_FAIL;
    		}
    	    PTRACE2INT(eLevelInfoNormal, "CConfPartyManager::OnCsProxyConfRegisterStatus - Internal ServiceId = ", serviceIndex);
    	    pRegisteredEntity->SetServiceRegistrationContentStatus(serviceIndex,registrationStatus);
    		pRegisteredEntity->UpdateServiceRegistrationTotalStatus(serviceIndex,registrationStatus);
    		pRegisteredEntity->SetSipTotRegistrationsStatus(pRegisteredEntity->GetSipRegistrationTotalSts());//from comresapi to commconf
    	    return STATUS_OK;
    	}
    }

    if ( ( eSipRegistrationConfTypeMR == (eSipRegistrationConfType)EntityType )
    		||
    	 ( eSipRegistrationConfTypeEQ == (eSipRegistrationConfType)EntityType )
    		||
    	 ( eSipRegistrationConfTypeFactory == (eSipRegistrationConfType)EntityType )
    		||
    	 ( eSipRegistrationConfTypeGWProfile == (eSipRegistrationConfType)EntityType ) )
    {
    	CCommResDB * pCommResDB = ::GetpMeetingRoomDB();
    	CCommRes* pRegisteredEntity = pCommResDB->GetCurrentRsrv(EntityID);
    	if (!CPObject::IsValidPObjectPtr(pRegisteredEntity))
    	{
    		PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnCsProxyConfRegisterStatus Error!!! Entity does not exist in DB ID=",EntityID);
			return STATUS_FAIL;
    	}
    	else
    	{
    		serviceIndex = pRegisteredEntity->GetServiceRegistrationContentServiceIndexByName(Service_nm);
    		if (serviceIndex < 0)
			{
    			FTRACEINTOFUNC<<" Error!!! Service name doesn't found, Service_nm = "<<Service_nm;
    			POBJDELETE(pRegisteredEntity);
				return STATUS_FAIL;
			}
    	    PTRACE2INT(eLevelInfoNormal, "CConfPartyManager::OnCsProxyConfRegisterStatus - Internal ServiceId = ", serviceIndex);
    		pRegisteredEntity->SetServiceRegistrationContentStatus(serviceIndex,registrationStatus);
    		pRegisteredEntity->UpdateServiceRegistrationTotalStatus(serviceIndex,registrationStatus);
    		PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnCsProxyConfRegisterStatus - update total status in CommResShort sts=",registrationStatus);
    		pCommResDB->UpdateForNewEntity(*pRegisteredEntity);
    		POBJDELETE(pRegisteredEntity);
    		return STATUS_OK;
    	}
    }

    PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnCsProxyConfRegisterStatus Error!!! got illegal EntityType =",EntityType);
    return STATUS_FAIL;
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnStartAssistTask(CSegment* pSeg)
{
	CreateAssistTask();

	std::ostringstream answer;
	CConfPartyProcess::GetProcess()->DumpTasks(answer);
	PTRACE2(eLevelInfoNormal, "CConfPartyManager::OnStartAssistTask - task list:\n", answer.str().c_str());
}
////////////////////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnMcuSetGMTOffsetInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::OnMcuSetGMTOffsetInd");
	BYTE  GMTOffset;
	BYTE  GMTOffsetSign;
    *pMsg >> GMTOffset;
    *pMsg >> GMTOffsetSign;

    m_GMT_offset = GMTOffset;
    m_GMT_offsetSign = GMTOffsetSign;
	//PTRACE2(eLevelInfoNormal, "CConfPartyManager::OnMcuSetGMTOffsetInd Size received from McuMngr process: ", GMTOffset);

//  Do what is needed
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnProxyConfUpdateServerType(CSegment* pSeg)
{
	DWORD serviceId;
	DWORD serverType;

	*pSeg >> serviceId;
	*pSeg >> serverType;

	if (serverType == eSipServer_CiscoCucm)
	{
		PTRACE2INT(eLevelInfoNormal, "CConfPartyManager::OnProxyConfUpdateServerType, sip server type is cisco cucm, serviceId: ", serviceId);

		CIpServiceListManager *pIpServiceListManager = ::GetIpServiceListMngr();

		if (pIpServiceListManager)
		{
			CConfIpParameters *pServiceParams = pIpServiceListManager->FindIpService(serviceId);

			if (pServiceParams)
				pServiceParams->SetSipServerType(serverType);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetSiteName(CRequest *pRequest)
{

	PTRACE(eLevelInfoNormal,  "CConfPartyManager::OnServerSetSiteName " );
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
	{
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetSiteName: No permission to OnServerSetSiteName for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
	}
	STATUS status = STATUS_OK;

	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	if (systemCardsBasedMode == eSystemCardsMode_mpm)
	{
		PTRACE(eLevelError,  "CConfPartyManager::OnServerSetSiteName message overlay is not supported on MPM mode!!!" );
		return STATUS_ILLEGAL_IN_MPM_MODE;
	}

	CSiteNameInfoDrv* pSiteNameInforDrv = new CSiteNameInfoDrv;

	*pSiteNameInforDrv = *(CSiteNameInfoDrv*)pRequest->GetRequestObject() ;

	CSiteNameInfo* pSiteNameInfo = pSiteNameInforDrv->GetSiteNameInfo();

	const DWORD confId   = pSiteNameInforDrv->GetConfID();

	// Romem klocwork
	//if (::GetpConfDB()->FindId(confId) == NOT_FIND)
	//	  status = STATUS_CONF_NOT_EXISTS;
	CCommConf* pConf = ::GetpConfDB()->GetCurrentConf(confId);
	if(!pConf)
	{
		status = STATUS_CONF_NOT_EXISTS;
	}

	if (status==STATUS_OK)
	{
		//CCommConf* pConf = ::GetpConfDB()->GetCurrentConf(confId);

		PASSERTMSG_AND_RETURN_VALUE(pConf==NULL, "GetCurrentConf returned NULL", STATUS_FAIL);
		pConf->SetSiteName(pSiteNameInfo);

		CConfApi confApi;
		confApi.CreateOnlyApi(*(pConf->GetRcvMbx()));
		confApi.UpdateSiteName(pSiteNameInfo);
		confApi.DestroyOnlyApi();
	}

	std::string responseTrancsName("TRANS_CONF_2");
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pSiteNameInforDrv);
	pRequest->SetStatus(status);


	return STATUS_OK;

}

////////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::OnServerSetCustomizeDisplayForConf(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal,  "CConfPartyManager::OnServerSetCustomizeDisplayForConf " );
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
         FPTRACE(eLevelInfoNormal,"CConfPartyManager::OnServerSetCustomizeDisplayForConf: No permission to OnServerSetCustomizeDisplayForConf for administrator readony");
         pRequest->SetConfirmObject(new CDummyEntry());
         pRequest->SetStatus(STATUS_NO_PERMISSION);
         return STATUS_NO_PERMISSION;
    }

	//printf("CConfPartyManager::OnServerSetCustomizeDisplayForConf \n");

	CCustomizeDisplaySettingForOngoingConfConfiguration* pDisplaySetting = new CCustomizeDisplaySettingForOngoingConfConfiguration;

	*pDisplaySetting = *(CCustomizeDisplaySettingForOngoingConfConfiguration*)pRequest->GetRequestObject() ;
	m_pProcess->GetCustomizeDisplaySettingForOngoingConfConfiguration()->SetObtainDisplayNamefromAddressBook(pDisplaySetting->IsObtainDsipalyNamefromAddressBook());
	m_pProcess->GetCustomizeDisplaySettingForOngoingConfConfiguration()->WriteXmlFile(CUSTOMIZE_DISPLAY_SETTING_FOR_ONGGOING_CONF_CFG_FILE, "CUSTOMIZE_DISP_SETUP_ONGOING_CONF");

	std::string responseTrancsName("TRANS_CUSTOMIZE_SETUP_ONGOING_CONF");
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pDisplaySetting);
	pRequest->SetStatus(STATUS_OK);


	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnRemoveConfByTaskId(CSegment* pSeg)
{
	DWORD taskId;

	*pSeg >> taskId;
	m_TaskIdToConfId.erase(taskId);
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnConfUpdateTaskFailed(CSegment* pSeg)
{
	DWORD oldtaskId;
	DWORD newtaskId;

	*pSeg >> oldtaskId;
	*pSeg >> newtaskId;

	std::map<DWORD, PartyMonitorID> * taskIdToPartyId = GetMapPartiesTasksIds();
	if(m_TaskIdToConfId.find(oldtaskId) != m_TaskIdToConfId.end())
	{
		DWORD conf_id = m_TaskIdToConfId[oldtaskId];
		m_TaskIdToConfId.erase(oldtaskId);
		m_TaskIdToConfId[newtaskId] = conf_id;
		COsTask::SendSignal(oldtaskId, SIGHUP);
		//kill the thread of the old conference
	}
	else if(taskIdToPartyId->find(oldtaskId) != taskIdToPartyId->end())
	{
		PartyMonitorID party_id = (*taskIdToPartyId)[oldtaskId];
		taskIdToPartyId->erase(oldtaskId);
		(*taskIdToPartyId)[newtaskId] = party_id;
		COsTask::SendSignal(oldtaskId, SIGHUP);
	}

}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnConfOrPartyTaskFailed(CSegment* pSeg)
{
	BOOL bEnableConfCleanup = TRUE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_CONFERENCE_CLEANUP_ON_FAULT, bEnableConfCleanup);
	if (!bEnableConfCleanup)
	{
		PTRACE(eLevelInfoNormal,"CConfPartyManager::OnConfOrPartyTaskFailed : Conference or party cleanup DISABLED by system flag!");
		return;
	}

	DWORD taskId;
	*pSeg >> taskId;

	if(m_TaskIdToConfId.find(taskId) != m_TaskIdToConfId.end())
	{
		ConfTaskCleanup(taskId);
	}
	else
	{
		//handle party
		PartyTaskCleanup(taskId);

	}
}
//////////////////////////////////////////////////////////////////////
void CConfPartyManager::ConfTaskCleanup(DWORD taskId)
{
	CMedString str;
	str << " ConfTaskID= " << taskId;
	str << " ConfTaskCrashesCounter= " << m_ConfTaskCrashesCounter;

	// Increase the conference crashes counter
	++m_ConfTaskCrashesCounter;
	PTRACE2(eLevelInfoNormal,"CConfPartyManager::ConfTaskCleanup : ", str.GetString());

	// Convert task id to conf id
	DWORD conf_id = m_TaskIdToConfId[taskId];
	m_TaskIdToConfId.erase(taskId);

	// Send cleanup message to ConfParty Assist Manager
	CSegment *pSeg = new CSegment;
	*pSeg << conf_id;
	*pSeg << taskId;
	*pSeg << CLEAN_FAILED_CONFERENCE;
	m_pAssistMngApi->SendMsg(pSeg, CLEAN_FAILED_CONFERENCE);
}
//////////////////////////////////////////////////////////////////////
void CConfPartyManager::PartyTaskCleanup(DWORD taskId)
{
	CMedString str;
	str << " PartyTaskID=" << taskId;
	str << " m_PartyTaskCrashesCounter=" << m_PartyTaskCrashesCounter;

	// Increase the conference crashes counter
	++m_PartyTaskCrashesCounter;
	PTRACE2(eLevelInfoNormal,"CConfPartyManager::PartyTaskCleanup : ", str.GetString());

	// Convert task id to conf id
	std::map<DWORD, PartyMonitorID> * taskIdToPartyId = GetMapPartiesTasksIds();
	PartyMonitorID party_id = (* taskIdToPartyId)[taskId];
	taskIdToPartyId->erase(taskId);

	// Send cleanup message to ConfParty Assist Manager
	CSegment *pSeg = new CSegment;
	*pSeg << party_id;
	*pSeg << taskId;
	*pSeg << CLEAN_FAILED_PARTY;
	m_pAssistMngApi->SendMsg(pSeg, CLEAN_FAILED_PARTY);
}
//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleTerminalKillConf(CTerminalCommand & command,std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleTerminalKillConf");

	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: action must be specified: KillConf\n";
		answer << "usage: Bin/McuCmd KillConf ConfParty [Conf ID]\n";
		return STATUS_FAIL;
	}

	const string &strConfId = command.GetToken(eCmdParam1);

	answer <<  "Kill conference " << strConfId << "\n";

	DWORD confID = atoi(strConfId.c_str());

	DWORD taskId = 0;

	for (TaskIdToConfId::iterator itr = m_TaskIdToConfId.begin() ; itr != m_TaskIdToConfId.end() ; ++itr)
	{
		if (itr->second == confID)
		{
			taskId = itr->first;

			if (taskId != 0)
			{
				// Send Segmentation violation to the thread of the conference
				COsTask::SendSignal(taskId, SIGSEGV);
			}

			break;
		}
	}

	if (taskId == 0)
	{
		answer <<  "Conference " << strConfId << " not found!\n";
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CConfPartyManager::HandleTerminalPartySlowFastChange(CTerminalCommand & command,std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleTerminalPartyFastChange");

	DWORD numOfParams = command.GetNumOfParams();
	if(3 != numOfParams)
	{
		answer << "error: action must be specified: PCHANGE\n";
		answer << "usage: Bin/McuCmd PCHANGE ConfParty  [Conf Name] [Party-ID] [0 (slow) or 1 (fast)] \n";
		return STATUS_FAIL;
	}

	const string &confName 	 = command.GetToken(eCmdParam1);
	const string &sPartyID    = command.GetToken(eCmdParam2);
	const string &action     = command.GetToken(eCmdParam3);

	DWORD partyID = atoi(sPartyID.c_str());
	int iAction = atoi(action.c_str());

	// Find Conf in DB
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = pCommConfDB->GetCurrentConf(confName.c_str());
	if(!CPObject::IsValidPObjectPtr(pRequestedConf))
	{
		answer << "PCHANGE for party error: Conf does not exist in DB" << " " <<  confName << " " << action;
		return STATUS_FAIL;
	}

	const char * partyName = ::GetpConfDB()->GetPartyName( confName.c_str() ,partyID );
	if (NULL == partyName)
	{
		answer << "PCHANGE for party error: Party ID does not exist in DB, action = " << " " << action;
		return STATUS_FAIL;
	}

	STATUS status = pRequestedConf->SLOW_FAST_CHANGE_Terminal( partyName, iAction );
	if (STATUS_OK != status)
	{
		answer << "PCHANGE for party error: Party does not exist in DB" << " " <<  partyName << " " << action;
		return STATUS_FAIL;
	}

	return STATUS_OK;
}


STATUS CConfPartyManager::HandleTerminalKillParty(CTerminalCommand & command,std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleTerminalKillParty");

	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: action must be specified: KillParty\n";
		answer << "usage: Bin/McuCmd KillConf ConfParty [Conf ID]\n";
		return STATUS_FAIL;
	}

	const string &strPartyId = command.GetToken(eCmdParam1);

	answer <<  "Kill party " << strPartyId << "\n";

	PartyMonitorID partyId = atoi(strPartyId.c_str());

	DWORD taskId = 0;

	std::map<DWORD, PartyMonitorID> * taskIdToPartyId = GetMapPartiesTasksIds();

	for (TaskIdToConfId::iterator itr = taskIdToPartyId->begin() ; itr != taskIdToPartyId->end() ; ++itr)
	{
		if (itr->second == partyId)
		{
			taskId = itr->first;

			if (taskId != 0)
			{
				// Send Segmentation violation to the thread of the conference
				COsTask::SendSignal(taskId, SIGSEGV);
			}

			break;
		}
	}

	if (taskId == 0)
	{
		answer <<  "party " << strPartyId << " not found!\n";
	}

	return STATUS_OK;
}

STATUS CConfPartyManager::HandleSetConfAvcSvcMode(CTerminalCommand & command,std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleSetConfAvcSvcMode");
	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
		answer << "error: action must be specified: SetConfAvcSvcMode\n";
		answer << "usage: Bin/McuCmd SetConfAvcSvcMode [Conf Type] [Conf ID]\n";
		return STATUS_FAIL;
	}

	const string &confType = command.GetToken(eCmdParam1);
	const string &confId = command.GetToken(eCmdParam2);

	DWORD confID = atoi(confId.c_str());
	eConfMediaState ConfState;

	ConfState=GetConfType(confType);
	if (ConfState == -1)
	{
		answer << "Error: Conf Type is wrong need to be SVC or AVC or MIX, given is: " << confType;
		return STATUS_FAIL;
	}

	// Send event to Conference
	CConfApi confApi;

	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = NULL;

	pRequestedConf = pCommConfDB->GetCurrentConf(confID);
	if (!CPObject::IsValidPObjectPtr(pRequestedConf))
	{
		answer << "Error: Conf does not exist in DB confID=" << confID << " " << "HandleSetConfAvcSvcMode ";
		return STATUS_FAIL;
	}

	confApi.CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));
	confApi.SetConfAvcSvcMode(ConfState,confID);
	confApi.DestroyOnlyApi();

	answer << "conference type: " << confType.c_str() << " conference ID: " << confID;

	return STATUS_OK;
}

eConfMediaState CConfPartyManager::GetConfType(const string &confType)
{

	if (confType=="SVC")
	{
		return eMediaStateSvcOnly;
	}

	if (confType=="AVC")
	{
		return eMediaStateAvcOnly;
	}

	if (confType=="MIX")
	{
		return eMediaStateMixAvcSvc;
	}

	return eConfMediaState(-1);

}
STATUS CConfPartyManager::HandleSetPartyAvcSvcMode(CTerminalCommand & command,std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "CConfPartyManager::HandleSetPartyAvcSvcMode");
	DWORD numOfParams = command.GetNumOfParams();
	if(3 != numOfParams)
	{
		answer << "error: action must be specified: SetPartyAvcSvcMode\n";
		answer << "usage: Bin/McuCmd SetConfAvcSvcMode [Conf Type] [Conf ID] [Party ID]\n";
		return STATUS_FAIL;
	}


	const string &confType = command.GetToken(eCmdParam1);
	const string &confId = command.GetToken(eCmdParam2);
	const string &partyMonitorId = command.GetToken(eCmdParam3);
	DWORD confID = atoi(confId.c_str());
	DWORD partyMonitorID = atoi(partyMonitorId.c_str());

	eConfMediaState ConfState;

	ConfState=GetConfType(confType);
	if (ConfState == -1)
	{
		answer << "Error: Conf Type is wrong need to be SVC or AVC or MIX, given is: " << confType;
		return STATUS_FAIL;
	}

	// Send event to Conference
	CConfApi confApi;

	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = NULL;

	pRequestedConf = pCommConfDB->GetCurrentConf(confID);
	if (!CPObject::IsValidPObjectPtr(pRequestedConf))
	{
		answer << "Error: Conf does not exist in DB confID=" << confID << " " << "HandleSetPartyAvcSvcMode "<< pRequestedConf;
		return STATUS_FAIL;
	}

	DWORD status = ::GetpConfDB()->SearchPartyName(confID, partyMonitorID);
	if (status != STATUS_OK)
	{
		answer << "Error: Party does not exist in DB partyID=" << partyMonitorID << " " << "HandleSetPartyAvcSvcMode ";
		return STATUS_FAIL;
	}

	confApi.CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));
	confApi.SetPartyAvcSvcMode(ConfState,confID,partyMonitorID);
	confApi.DestroyOnlyApi();

	answer << "Conference type: " << confType << " Conference ID: " << confID << " Party Monitor ID: " << partyMonitorID;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnSNMPConfigInd(CSegment* pSeg)
{
	BOOL bSNMPEnabled = FALSE;
    *pSeg >> bSNMPEnabled;

	PTRACE2INT(eLevelInfoNormal,"CConfPartyManager::OnSNMPConfigInd = ",(int)bSNMPEnabled);

	m_pProcess->SetIsSNMPEnabled(bSNMPEnabled);
}

extern std::map<DWORD, PartyMonitorID> * GetMapPartiesTasksIds()
{
	return g_TaskIdToPartyId;
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnMccfSyncMsg(CSegment* pSeg)
{
	AppServerID appServerID = 0;
	HANDLE hMccfMsg = NULL;
	*pSeg >> appServerID >> hMccfMsg;

	clientRspMbxList.push_back(*m_pClientRspMbx);
	TRACEINTO << "AppServerID:" << appServerID << ", request:" << hMccfMsg << ", Channels open (after sync):" << clientRspMbxList.size();

	CTaskApi api; // api to tx
	api.CreateOnlyApi(*m_pClientRspMbx);

	CSegment* pMsg = new CSegment;
	*pMsg << hMccfMsg;

	api.SendMsg(pMsg, MCCF_REQUEST_ACK); // notify SYNC is done
}

///////////////////////////////////////////////////////////////////////////////
bool operator ==(const COsQueue& a, const COsQueue& b)
{
	return
		a.m_id == b.m_id &&
		a.m_idType == b.m_idType &&
		a.m_process == b.m_process &&
		a.m_scope == b.m_scope;
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnMccfDropMsg(CSegment* pSeg)
{
	AppServerID appServerID;
	*pSeg >> appServerID;

	// TODO: remove the corresponding mailbox
	clientRspMbxList.erase(std::find(clientRspMbxList.begin(), clientRspMbxList.end(), *m_pClientRspMbx));
	TRACEINTO << "AppServerID:" << appServerID << ", Channels open (after drop):" << clientRspMbxList.size();
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnMccfIvrFileDownloadComplete(CSegment* pSeg)
{
	std::string url;
	int nDownloadResult;
	std::string baseFolder;

	*pSeg >> url >> nDownloadResult >> baseFolder;

    DownloadFileResult downloadResult = static_cast<DownloadFileResult>(nDownloadResult);

	CFilesCache& cache = CFilesCache::instance();

	if (DOWNLOAD_FILE_FAILED != downloadResult)
	{

		if(m_ProceedingSlides.find(url) != m_ProceedingSlides.end())//If the downloaded file is a slide, start to convert it.
		{
			if (DOWNLOAD_FILE_OK == downloadResult)
			{
				ConvertExternalSlideIVR(url, baseFolder);
				return;
			}
			else
    		{
				// Not modified
				DialogState state = m_ProceedingSlides[url];
				MccfIvrErrorCodesEnum status = mccf_ivr_OK;
				CMccfIvrPackageResponse::ResponseReportMsg(state, status);
				m_ProceedingSlides.erase(url);
			}
		}
		else	 //download file is audio
		{
		    CStructTm curTime;
		    SystemGetTime(curTime);
		    time_t lastModified = curTime.GetAbsTime(true);
			cache.SetFileAvailable(url, lastModified);
		}
	}
	else
		cache.RemoveFile(url);

	if(m_ProceedingAudio.find(url) != m_ProceedingAudio.end())
	{
		DialogState state = m_ProceedingAudio[url];
		MccfIvrErrorCodesEnum status = (DOWNLOAD_FILE_FAILED != downloadResult) ? mccf_ivr_OK : mccf_ivr_Resource_Cannot_Be_Retrieved;
		CMccfIvrPackageResponse::ResponseReportMsg(state, status);
		m_ProceedingAudio.erase(url);
	}
	else
	{
		TRACEINTO << "The file:" << url <<" downloaded successfully and no need to send response";
	}

}


//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnMccfIvrSlideConvertComplete(CSegment* pSeg)
{
	//
	std::string url;
	int retStatus = 0;

	*pSeg >> url >> retStatus;

	CFilesCache& cache = CFilesCache::instance();

	if (0 == retStatus)
	{
		CStructTm curTime;
		SystemGetTime(curTime);
		time_t lastModified = curTime.GetAbsTime(true);
		cache.SetFileAvailable(url, lastModified);
	}
	else
		cache.RemoveFile(url);

	TRACEINTO << "The slide file:" << url <<" convert status is:" << retStatus;

	if(m_ProceedingSlides.find(url) != m_ProceedingSlides.end())
	{
		DialogState state = m_ProceedingSlides[url];
		MccfIvrErrorCodesEnum status =  (0 ==retStatus) ? mccf_ivr_OK : mccf_ivr_Media_Stream_Not_Available;
		CMccfIvrPackageResponse::ResponseReportMsg(state, status);
		m_ProceedingSlides.erase(url);
	}
	else
		TRACEINTO << "The slide file:" << url <<" converted successfully and no need to send response";


}



//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnMccfIvrMsg(CSegment* pSeg)
{
	DialogState state;
    string appServerIp;

	*pSeg >> state.appServerID >> state.hMccfMsg >> appServerIp;
	PASSERTSTREAM_AND_RETURN(!state.hMccfMsg || !state.appServerID, "state:" << state);

    TRACEINTO << "Receive Mccf ivr msg from " << appServerIp.c_str();

    state.appServerIp = appServerIp;
	state.clientRspMbx = *m_pClientRspMbx;

	state.baseObject = new MscIvr;
	*pSeg >> *state.baseObject;

	TRACEINTO << state << '\n' << *state.baseObject;

	DialogHandler command = NULL;

	MscIvr& mscIvr = *(MscIvr*)(state.baseObject);
	const char* dialogName = mscIvr.m_pResponseType->objectCodeName();

	if (strcmp(dialogName, DialogTerminate::classType()) == 0)
		command = &CConfPartyManager::IVRDialogTerminateMsg;
	else
	{
		if (strcmp(dialogName, DialogStart::classType()) == 0)
			command = &CConfPartyManager::IVRDialogStartMsg;

		else if (strcmp(dialogName, DialogPrepare::classType()) == 0)
			command = &CConfPartyManager::IVRDialogPrepareMsg;

		else
		{
			PASSERTSTREAM_AND_RETURN(true, "Unsupported dialog command:" << dialogName);
			delete state.baseObject;
			state.baseObject = NULL;
		}
	}

	(this->*command)(state);
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::IVRDialogStartMsg(DialogState& state)
{
	MscIvr& mscIvr = *(MscIvr*)(state.baseObject);
	DialogStart* dialog = (DialogStart*)(mscIvr.m_pResponseType);
	TRACEINTO << "mscIvr:" << mscIvr;

	MccfIvrErrorCodesEnum status = mccf_ivr_OK;
	IvrControlTypeEnum allocatedType = ict_None;

	//check if dialod id wes determined in dialogprepare
	if (!dialog->m_preparedDialogId.empty())
	{
		state.dialogID = dialog->m_preparedDialogId;
		//check if this dialog id was allocated
		allocatedType = CMccfIvrDialogManager::instance().CheckDialogID(state.appServerID, state.dialogID);

		if(allocatedType != ict_Prepare)
			status = mccf_ivr_DialogID_Not_Exist;
	}
	//check if the appServer send us dialog id
	else if (!dialog->m_dialogId.empty())
	{
		state.dialogID = dialog->m_dialogId;
		//check if this dialog id was already allocated
		allocatedType = CMccfIvrDialogManager::instance().CheckDialogID(state.appServerID, state.dialogID);
	}

	//if it was already allocated
	if (allocatedType == ict_Start)
		status = mccf_ivr_DialogID_Already_Exists;

	if (status == mccf_ivr_OK)
	{
		//allocate the new dialog id if it was not allocated in prepare message
		if (allocatedType == ict_None)
			CMccfIvrDialogManager::instance().AllocateDialogID(state.appServerID, ict_Start, state.dialogID);

		if (state.action == DialogState::dsa_start)
		{
			DialogHandler handler = &CConfPartyManager::IVRDialogStartMsg;
			CMccfIvrDispatcher::instance().AddDialog(state, handler, this);
		}

		if (!dialog->m_conferenceId.empty())
		{
			const unsigned int confID = atoi(dialog->m_conferenceId.c_str()); //TODO check how we get this param
			CCommConf* pCurConf = ::GetpConfDB()->GetCurrentConf(confID);

			if (!pCurConf)
				status = mccf_ivr_ConfID_Not_Exist;
			else
				pCurConf->MccfIvrStartDialog(state);
		}
		else if (!dialog->m_connectionId.empty())
		{
			const char* tag = dialog->m_connectionId.c_str();
			CCommConfDB* pConfDB = ::GetpConfDB();
			CTaskApp* partyTask = pConfDB->GetPartyTask(tag);

			if (!partyTask)
			{
				status = mccf_ivr_ConnectionID_Not_Exist;
				TRACEINTO << "status = mccf_ivr_ConnectionID_Not_Exist because partyTask is NULL";
			}

			else //partyTask != NULL
			{
				DWORD confID = pConfDB->GetConfIDByTag(tag);
				DWORD partyID = pConfDB->GetPartyIDByTag(tag);
				TRACEINTO << "confID: " << confID << "partyID: " << partyID;
				const CConfParty* pConfParty = pConfDB->GetCurrentParty(confID, partyID);
				if (pConfParty)
				{
					DWORD partyState = pConfParty->GetPartyState();
					TRACEINTO << "partyState: " << partyState;
					if ( (PARTY_CONNECTED == partyState ) || (PARTY_CONNECTED_PARTIALY == partyState ) || ( PARTY_SECONDARY == partyState ) || ( PARTY_CONNECTED_WITH_PROBLEM == partyState ) )
					{
						//check whether file exist or not.
						for (std::list<MediaElementType>::const_iterator it = dialog->m_dialog.m_prompt.m_media.begin(); it != dialog->m_dialog.m_prompt.m_media.end(); ++it)
						{
							const CLocalFileDescriptor* file = CFilesCache::const_instance().fileDescriptor(it->m_loc);
							if(NULL== file)
							{
								status = mccf_ivr_Media_Stream_Not_Available;
								TRACEINTO << "status = mccf_ivr_Media_Stream_Not_Available because file is not found in File Cache";

								//Start to download/convert the file
								std::string url = it->m_loc;
								if((m_ProceedingSlides.find(url) != m_ProceedingSlides.end()) ||
									(m_ProceedingAudio.find(url) != m_ProceedingAudio.end()))
								{
									TRACEINTO << "URL:"<< url <<"is being downloading or converting, ignore this request";
									continue;
								}

                                SaveExternalIVRFilesInfo(url, state.appServerIp);
								PrepareExternalMediaFileIVR(dialog->m_dialog, *it, state.appServerID, state.appServerIp);

							}
						}


						if(status == mccf_ivr_OK)
						{
							TRACEINTO << "send start dialog to party";
							CPartyApi api;
							api.CreateOnlyApi(partyTask->GetRcvMbx());
							api.SendDialogStart(state);
						}
					}
					else
					{
						status = mccf_ivr_ConnectionID_Not_Exist;
						TRACEINTO << "status = mccf_ivr_ConnectionID_Not_Exist because partyState is: " << partyState;
					}
				}
				else
				{
					status = mccf_ivr_ConnectionID_Not_Exist;
					TRACEINTO << "status = mccf_ivr_ConnectionID_Not_Exist because pConfParty is NULL";
				}

				// TODO: update the map for terminate request

			} //partyTask != NULL

		} //connectionID
		else
			status = mccf_ivr_Syntax_Error;
	}

	if (status != mccf_ivr_OK)
	{
		PASSERT(status);
		CMccfIvrPackageResponse::ResponseReportMsg(state, status);
	}
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::IVRDialogPrepareMsg(DialogState& state)
{
	MscIvr& mscIvr = *(MscIvr*)(state.baseObject);
	DialogPrepare* dialog = (DialogPrepare*)(mscIvr.m_pResponseType);
	MccfIvrErrorCodesEnum status = mccf_ivr_OK;
	TRACEINTO << "dialog:" << dialog;

	if (state.action == DialogState::dsa_start)
	{
		CMccfIvrDialogManager& ivrDialogMngr = CMccfIvrDialogManager::instance();

		state.dialogID = dialog->m_dialogId.value();

		IvrControlTypeEnum allocatedType = ivrDialogMngr.CheckDialogID(state.appServerID, state.dialogID);
		status = (allocatedType != ict_None) ? mccf_ivr_DialogID_Already_Exists : mccf_ivr_OK;

		if (allocatedType == ict_None)
		{
			CMccfIvrDialogManager::instance().AllocateDialogID(state.appServerID, ict_Prepare, state.dialogID);

			// add the dialog to the map - for dialog terminate
			DialogHandler handler = &CConfPartyManager::IVRDialogPrepareMsg;
			CMccfIvrDispatcher::instance().AddDialog(state, handler, this);
		}


		for (std::list<MediaElementType>::const_iterator it = dialog->m_dialog.m_prompt.m_media.begin(); it != dialog->m_dialog.m_prompt.m_media.end(); ++it)
		{

			// TODO: make sure that we finish to get the files and then continue

			MediaFileTypeEnum ivrFileType = CMediaTypeManager::DeriveMediaType(it->m_type, it->m_loc);
			std::string url = it->m_loc;
			if((m_ProceedingSlides.find(url) != m_ProceedingSlides.end()) ||
				(m_ProceedingAudio.find(url) != m_ProceedingAudio.end()))
			{
					TRACEINTO << "URL:"<< url <<"is being downloading or converting, ignore this request";
					continue;
			}

			SaveExternalIVRFilesInfo(url, state.appServerIp);

			PrepareExternalMediaFileIVR(dialog->m_dialog, *it, state.appServerID, state.appServerIp);

			if(mft_Image == ivrFileType)
			{
				m_ProceedingSlides[url] = state;
				TRACEINTO << "This is a slide media file to download and convert, url is:" << url;
				if(!IsValidTimer(CONFPARTY_CHECK_MCCF_SLIDE_PROCEEDING_TIMER))  //only start timer when it's not started yet.
					StartTimer(CONFPARTY_CHECK_MCCF_SLIDE_PROCEEDING_TIMER, 2);
			}
			else if(mft_Audio == ivrFileType)
			{
				m_ProceedingAudio[url] = state;
				TRACEINTO << "This is a audio media file to download, url is:" << url;
			}
			else
			{
				//how to handle the error case???
				TRACEINTO << "unsupported media element type:" << ivrFileType;
				return;
			}
		}

	}
	else //terminate the dialog
	{
		//stop prepareExternalMediaFileIVR ??
		//if we did it successfully
		status = mccf_ivr_OK;
	}

	if(status == mccf_ivr_OK)
		CMccfIvrDispatcher::instance().RemoveDialog(state);

}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::IVRDialogTerminateMsg(DialogState& state)
{
	TRACEINTO;

	MscIvr& mscIvr = *(MscIvr*)(state.baseObject);
	DialogTerminate* dialog = (DialogTerminate*)(mscIvr.m_pResponseType);

	MccfIvrErrorCodesEnum status = mccf_ivr_OK;

	state.dialogID = dialog->m_dialogId;

	//check if there is dialog id in the message
	if (state.dialogID.empty())
	{
		status = mccf_ivr_Syntax_Error;
	}

	//the appServer send us dialog id
	else
	{
		//check if this dialog id was already allocated
		IvrControlTypeEnum allocatedType = CMccfIvrDialogManager::instance().CheckDialogID(state.appServerID, state.dialogID);

		if (allocatedType == ict_None)
		{
			status = mccf_ivr_DialogID_Not_Exist;
		}
	}

	//terminate the dialog
	if (status == mccf_ivr_OK)
		CMccfIvrDispatcher::instance().Dispatch(state.appServerID,state.dialogID, state.hMccfMsg); //TODO return Response message when we finish the terminate! (when we do removeDialog)
	else
		CMccfIvrPackageResponse::ResponseReportMsg(state, status);
}


//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnTimerCheckSlideProceeding(CSegment* pSeg)
{
	TRACEINTO << "proceeding slides size is: " << m_ProceedingSlides.size();
	if(m_ProceedingSlides.size() > 0)
	{
		time_t      curTime;
		curTime = time(NULL);
		MccfIvrErrorCodesEnum status = mccf_ivr_OK;
		for (ProceedingMedia::iterator itr = m_ProceedingSlides.begin() ; itr != m_ProceedingSlides.end() ; ++itr)
		{
			DialogState& dialogState = itr->second;
			double timeDiff = difftime(curTime,dialogState.dialogProceedTime);
			double roundDiff = timeDiff - (int)(timeDiff / 12) * 12;
			if((timeDiff >= 11.0) && ((roundDiff > 11.0) || (roundDiff <= 1.0)))
			{
				CMccfIvrPackageResponse::ResponseReportMsg(dialogState, status, true);
				dialogState.seqNum++;
			}
		}

		StartTimer(CONFPARTY_CHECK_MCCF_SLIDE_PROCEEDING_TIMER,2 * SECOND);
	}


}


//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnResourcesSetConfAvcSvcMediaStateInd(CSegment* pSeg)
{
  DWORD monitor_conf_id = 0;
  DWORD mediaState = 0;
  *pSeg >> mediaState >> monitor_conf_id;

  TRACEINTO << " monitor_conf_id = " << monitor_conf_id << " , mediaState = " << mediaState;

	// Send event to Conference
  CConfApi confApi;

  CCommConfDB* pCommConfDB = ::GetpConfDB();
  CCommConf* pRequestedConf = NULL;

  pRequestedConf = pCommConfDB->GetCurrentConf(monitor_conf_id);
  if (!CPObject::IsValidPObjectPtr(pRequestedConf))
    {
      TRACEINTO << "Error: Conf does not exist in DB monitor_conf_id = " << monitor_conf_id;
      return;
    }

  confApi.CreateOnlyApi(*(pRequestedConf->GetRcvMbx()));
  confApi.SetConfAvcSvcMode((eConfMediaState)mediaState,monitor_conf_id);
  confApi.DestroyOnlyApi();
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnLoggerUpdateNumber(CSegment* pSeg)
{
	DWORD currentLogNumber = 0;
	*pSeg >> currentLogNumber;
    TRACEINTO << " current: " << currentLogNumber;
    SetCurrentLoggerNumber(currentLogNumber);
}

//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnSipLyncBeNotify(CSegment* pSeg)
{
	TRACEINTO;
	CMSSubscriberMngr::ProcessBeNotify(pSeg);
}
//////////////////////////////////////////////////////////////////////
//added by Richer for Video Recovery project om 2013.12.26
STATUS CConfPartyManager::HandleVideoRecovery(CTerminalCommand & command, std::ostream& answer)
{
    PTRACE(eLevelInfoHigh, "CConfPartyManager::HandleVideoRecovery ");
    const DWORD paramsNumber = command.GetNumOfParams();

    if (paramsNumber != 1)
    {
        answer << "usage: Bin/McuCmd video_recovery ConfParty [start or over]\n";
        return STATUS_FAIL;
    }

    const string& cmd = command.GetToken(eCmdParam1);

    //start vedio recovery
    if ("start" == cmd)
    {
        PTRACE(eLevelInfoHigh, "CConfPartyManager::HandleVideoRecovery: start");

         //to disconnect all party
        CCommConfDB* pCommConfDB = ::GetpConfDB();
        CCommConf* pCommConf = pCommConfDB->GetFirstCommConf();
        CConfParty* pConfParty = NULL;

        /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
        CConfApi confApi;
        /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

        while(CPObject::IsValidPObjectPtr(pCommConf))
        {
            pConfParty = pCommConf->GetFirstParty();

            /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
            pCommConf->SetVideoRecoveryStatus(true);

            confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
            /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

            while(CPObject::IsValidPObjectPtr(pConfParty))
            {
                DisconnectParty(pCommConf->GetMonitorConfId(), pConfParty->GetPartyId(), m_operName);

                /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
                // Send event to Conference to disconnect party
                confApi.SendByeToConf(pConfParty->GetName());
                /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

                pConfParty->SetConfPartyVideoRecoveryStatus();

                //confApi.UpdateDB(pConfParty->GetTask(), PARTYSTATE,PARTY_DISCONNECTED);

                /*Begin:added by Richer for BRIDGE-13006,2014.04.29*/
                confApi.UpdateDB(pConfParty->GetTask(), DISCAUSE, DISCONNECTED_BY_VIDEO_RECOVERY, 1);
                /*End:added by Richer for BRIDGE-13006,2014.04.29*/

                pConfParty = pCommConf->GetNextParty();
            }

            /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
            confApi.DestroyOnlyApi();
            /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

            pCommConf = pCommConfDB->GetNextCommConf();
        }

         //to set ledlight

        //to send alarm to IMA
        AddActiveAlarmSingleton(
                                    FAULT_GENERAL_SUBJECT,
                                    VIDEO_RECOVERY,
                                    MAJOR_ERROR_LEVEL,
                                    "Media is recoverying",/*modified by Richer for BRIDGE-12231,2014.3.12 */
                                    true,
                                    true);

        return STATUS_OK;
    }

    //finish vedio recovery
    if ("over" == cmd)
    {
        PTRACE(eLevelInfoHigh, "CConfPartyManager::HandleVideoRecovery: over");

        //to reset ledlight

        //to send cancel alarm to IMA
        // RemoveActiveAlarmByErrorCode(VIDEO_RECOVERY);

        /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
        CCommConfDB* pCommConfDB = ::GetpConfDB();
        CCommConf* pCommConf = pCommConfDB->GetFirstCommConf();
        CConfParty* pConfParty = NULL;

        CConfApi confApi;

        while(CPObject::IsValidPObjectPtr(pCommConf))
        {

            //pCommConf->ClearVideoRecoveryStatus();
            confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
            pConfParty = pCommConf->GetFirstParty();

            while(CPObject::IsValidPObjectPtr(pConfParty))
            {
                confApi.UpdateDB(pConfParty->GetTask(), PARTYSTATE,PARTY_DISCONNECTED);
                pConfParty = pCommConf->GetNextParty();
            }
            confApi.DestroyOnlyApi();

            pConfParty = pCommConf->GetFirstParty();

            while(CPObject::IsValidPObjectPtr(pConfParty))
            {
                pConfParty->ClearConfPartyVideoRecoveryStatus();
                pConfParty = pCommConf->GetNextParty();
            }

            pCommConf->SetVideoRecoveryStatus(false);
            pCommConf = pCommConfDB->GetNextCommConf();
        }
        /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

        //to send cancel alarm to IMA
         RemoveActiveAlarmByErrorCode(VIDEO_RECOVERY);

        return STATUS_OK;
    }

    answer << "Command '" << cmd << "' is not (yet) implemented.";

    return STATUS_FAIL;
}


//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Added by Huizhao Sun on 2014-05-16 for converting slide
STATUS CConfPartyManager::OnServerConvertSlide(CRequest* pRequest)
{
    PTRACE(eLevelError,"CConfPartyManager::OnServerCovertSlide" );

    const DWORD conId = pRequest->GetConnectId();

    g_mapConnectConversionStatus[conId] = eIvrSlideConversionInProgress;

    CIVRServiceConvertSlide* pIVRServiceConvertSlideTemp = static_cast<CIVRServiceConvertSlide*>(pRequest->GetRequestObject());
    CIVRServiceConvertSlide* pIVRServiceConvertSlide = new CIVRServiceConvertSlide;
    *pIVRServiceConvertSlide = *pIVRServiceConvertSlideTemp;

    std::stringstream ssUrl;
    int conversionMethod = static_cast<int>(eIvrSlideLowHighRes); // low_high_res
    int imageType = static_cast<int>(eIvrSlideImageJpg);
    std::string outputPath = IVR_FOLDER_MAIN;
    outputPath += IVR_FOLDER_SLIDES;
    outputPath += "/";
    outputPath += pIVRServiceConvertSlide->GetSlideName();
    std::string inputFile = outputPath;
    inputFile += "/";
    inputFile += pIVRServiceConvertSlide->GetSlideName();
    if (eIvrSlideLowRes == pIVRServiceConvertSlide->GetConversionMethod())
    {
        inputFile += "_low";
    }
    else
    {
        if (eIvrSlideLowHighRes == pIVRServiceConvertSlide->GetConversionMethod() && eIvrSlideImageBmp == pIVRServiceConvertSlide->GetImageType())
        {
            // Change *_low.bmp to *_low.jpg
            stringstream sstr;
            sstr << "mv -f " << inputFile << "_low.bmp " << inputFile << "_low.jpg &> /dev/null";
            system(sstr.str().c_str());
        }
        inputFile += "_high";
    }

    if (eIvrSlideImageBmp == pIVRServiceConvertSlide->GetImageType())
    {
        inputFile += ".bmp";
        imageType = static_cast<int>(eIvrSlideImageBmp);
    }
    else
    {
        inputFile += ".jpg";
    }

    ssUrl << conId;
    conversionMethod = static_cast<int>(pIVRServiceConvertSlide->GetConversionMethod());

    CSegment* pSeg = new CSegment;
	*pSeg << eProcessConfParty << EMA_IVR_SLIDE_CONVERTED << ssUrl.str() << outputPath << inputFile
        << conversionMethod << imageType;

	FTRACEINTO
        << "Convert slide from EMA"
        << "\n URL:" << ssUrl.str()
        << "\n Output path:" << outputPath
        << "\n Input file:" << inputFile
        << "\n conversionMethod:" << conversionMethod
        << "\n imageType:" << imageType;

    CManagerApi api(eProcessUtility);
    api.SendMsg(pSeg, UTILITY_CONVERT_SLIDE);

    pRequest->SetStatus(STATUS_OK);
    pRequest->SetConfirmObject(new CDummyEntry());
    return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
// Added by Huizhao Sun on 2014-05-16 for slide from EMA converting completed
void CConfPartyManager::OnEMAIvrSlideConvertComplete(CSegment* pSeg)
{
    std::string url;
    int retStatus = 0;

    *pSeg >> url >> retStatus;

    eIvrSlideConversionStatus conversionStatus = eIvrSlideConversionSuccess;
    if (0 != retStatus) // faild
    {
        if (11 == retStatus) // invalid resolution
        {
            conversionStatus = eIvrSlideConversionInvalidResolution;
        }
        else
        {
            conversionStatus = eIvrSlideConversionFailed;
        }
    }

    if (eIvrSlideConversionSuccess != conversionStatus)
    {
        // If convert failed, delete the output path
        std::string outputPath;
        *pSeg >> outputPath;

        std::stringstream sstr;
        sstr    << "rm -fr \""
                << outputPath
                << "\""
                << " &> /dev/null";
        system(sstr.str().c_str());
    }
    else
    {
        int imageType;
        *pSeg >> imageType;
        if (eIvrSlideImageBmp == static_cast<eIvrSlideImageType>(imageType))
        {
            // Change the file *.bmp to *.jpg
            std::string srcImageFile;
            std::string extensionBmp = ".bmp";

            *pSeg >> srcImageFile;
            string::size_type pos= srcImageFile.find(extensionBmp);
            if (string::npos != pos)
            {
                std::string jpgImageFile = srcImageFile;
                jpgImageFile.replace(pos, extensionBmp.size(), ".jpg");
                stringstream sstr;
                sstr << "mv -f " << srcImageFile << " " << jpgImageFile << " &> /dev/null";
                system(sstr.str().c_str());
            }
        }
    }

    const DWORD conId = atoi(url.c_str());

    if (g_mapConnectConversionStatus.end() != g_mapConnectConversionStatus.find(conId))
    {
        g_mapConnectConversionStatus[conId] = conversionStatus;
    }

    TRACEINTO << "The slide file converted result from EMA: url is " << url << ", retStatus is:" << retStatus << ", conversionStatus is:" << conversionStatus;
}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
void CConfPartyManager::OnTimerDeleteUselessIVRFiles(CSegment* pSeg)
{
    TRACEINTO << "Begin to check if there are useless ivr files to need to delete.";

    struct dirent * pDirentAppServer;
    DIR * pDirExternal = opendir(IVR_EXTERNAL_FOLDER_MAIN);

    if (NULL == pDirExternal)
    {
    	return;
    }

    while ((pDirentAppServer = readdir(pDirExternal)) != NULL)
    {
        if (0 == strcmp(pDirentAppServer->d_name, ".") || 0 == strcmp(pDirentAppServer->d_name, ".."))
        {
            continue;
        }

        if (m_appServerIvrInfo.end()!= m_appServerIvrInfo.find(pDirentAppServer->d_name))
        {
            EXTERNAL_IVR_PROMPT_SET & ivrPromptSet = m_appServerIvrInfo[pDirentAppServer->d_name];
            struct dirent * pDirentPromptSet;
            std::string appServerPath = IVR_EXTERNAL_FOLDER_MAIN;
            appServerPath += "/";
            appServerPath += pDirentAppServer->d_name;
            DIR * dirAppServer = opendir(appServerPath.c_str());

            if (NULL == dirAppServer)
            {
                continue;
            }

            while ((pDirentPromptSet = readdir(dirAppServer)) != NULL)
            {
                if (0 == strcmp(pDirentPromptSet->d_name, ".") || 0 == strcmp(pDirentPromptSet->d_name, ".."))
                {
                    continue;
                }

                if (ivrPromptSet.end() != ivrPromptSet.find(pDirentPromptSet->d_name))
                {
                    TRACEINTO << "Checking media files under directory:" << appServerPath;
                    EXTERNAL_IVR_MEDIA_FILE_INFO & ivrMediaFile = ivrPromptSet[pDirentPromptSet->d_name];
                    struct dirent * pDirentMediaFile;
                    std::string mediaFilePath = appServerPath + "/" + pDirentPromptSet->d_name + IVR_FOLDER_MUSIC;
                    DIR * dirPromptSet = opendir(mediaFilePath.c_str());
                    if (NULL != dirPromptSet)
                    {
                        while ((pDirentMediaFile = readdir(dirPromptSet)) != NULL)
                        {
                            if (0 == strcmp(pDirentMediaFile->d_name, ".") || 0 == strcmp(pDirentMediaFile->d_name, ".."))
                            {
                                continue;
                            }

                            if (ivrMediaFile.end() == ivrMediaFile.find(pDirentMediaFile->d_name))
                            {
                                // delete music files
                                TRACEINTO << "Delete media file:" << mediaFilePath << "/" << pDirentMediaFile->d_name;
                                stringstream sstrIvrPromptSet;
                                sstrIvrPromptSet << "rm -fr " << mediaFilePath << "/" << pDirentMediaFile->d_name << " &> /dev/null";
                                system(sstrIvrPromptSet.str().c_str());
                            }
                        }

                        closedir(dirPromptSet);
                    }


                    mediaFilePath = appServerPath + "/" + pDirentPromptSet->d_name + IVR_FOLDER_SLIDES;
                    dirPromptSet = opendir(mediaFilePath.c_str());
                    if (NULL != dirPromptSet)
                    {
                        while ((pDirentMediaFile = readdir(dirPromptSet)) != NULL)
                        {
                            if (0 == strcmp(pDirentMediaFile->d_name, ".") || 0 == strcmp(pDirentMediaFile->d_name, ".."))
                            {
                                continue;
                            }

                            struct stat info;
                            std::string currentFilePath = mediaFilePath + "/" + pDirentMediaFile->d_name;
                            stat(currentFilePath.c_str(), &info);
                            if(S_ISDIR(info.st_mode))
                            {
                                continue;
                            }

                            if (ivrMediaFile.end() == ivrMediaFile.find(pDirentMediaFile->d_name))
                            {
                                // delete the folder for slides
                                std::string imageFolder = GetImageNameWithoutExtension(pDirentMediaFile->d_name);
                                if ("" != imageFolder)
                                {
                                    TRACEINTO << "Delete slides folder:" << mediaFilePath << "/" << imageFolder;
                                    stringstream sstrIvrSlidesFolder;
                                    sstrIvrSlidesFolder << "rm -fr " << mediaFilePath << "/" << imageFolder << " &> /dev/null";
                                    system(sstrIvrSlidesFolder.str().c_str());
                                }

                                // delete the image file
                                TRACEINTO << "Delete media file:" << currentFilePath;
                                stringstream sstrIvrImage;
                                sstrIvrImage << "rm -fr " << currentFilePath << " &> /dev/null";
                                system(sstrIvrImage.str().c_str());
                            }
                        }

                        closedir(dirPromptSet);
                    }
                }
                else
                {
                    TRACEINTO << "Delete prompt set " << pDirentPromptSet->d_name << " from " << pDirentAppServer->d_name;
                    stringstream sstrIvrPromptSet;
                    sstrIvrPromptSet << "rm -fr " << IVR_EXTERNAL_FOLDER_MAIN << "/" << pDirentAppServer->d_name << "/" << pDirentPromptSet->d_name << " &> /dev/null";
                    system(sstrIvrPromptSet.str().c_str());
                }
            }

            closedir(dirAppServer);
        }
        else
        {
            TRACEINTO << "Delete all prompt set from " << pDirentAppServer->d_name;
            stringstream sstrAppServer;
            sstrAppServer << "rm -fr " << IVR_EXTERNAL_FOLDER_MAIN << "/" << pDirentAppServer->d_name << " &> /dev/null";
            system(sstrAppServer.str().c_str());
        }
    }

    closedir(pDirExternal);
}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
void CConfPartyManager::SaveExternalIVRFilesInfo(const std::string & url, const std::string & appServerIp)
{
    // save the prompt set
//   const CLocalFileDescriptor* file = CFilesCache::const_instance().fileDescriptor(url);
    std::string::size_type pos_dir = url.find_last_of('/');
    if (std::string::npos == pos_dir)
    {
        return;
    }

    std::string path = url.substr(pos_dir+1);

    TRACEINTO << "Save a media file to the table for received external ivr files : " << path;

	std::string promptSetName;
	GetMediaPromptSetNameByUrl(url, promptSetName);
	if (m_appServerIvrInfo.end() != m_appServerIvrInfo.find(appServerIp))
	{
	    EXTERNAL_IVR_PROMPT_SET & ivrPromptSet = m_appServerIvrInfo[appServerIp];
		if (ivrPromptSet.end() == ivrPromptSet.find(promptSetName))
		{
		    EXTERNAL_IVR_MEDIA_FILE_INFO externalIvrMediaFile;
		    externalIvrMediaFile[path] = 0;
			ivrPromptSet[promptSetName] = externalIvrMediaFile;
		}
        else
        {
            EXTERNAL_IVR_MEDIA_FILE_INFO & externalIvrMediaFile = ivrPromptSet[promptSetName];
            if (externalIvrMediaFile.end() == externalIvrMediaFile.find(path))
            {
                externalIvrMediaFile[path] = 0;
            }
        }
	}
	else
	{
	    EXTERNAL_IVR_PROMPT_SET ivrPromptSet;
		EXTERNAL_IVR_MEDIA_FILE_INFO externalIvrMediaFile;
        externalIvrMediaFile[path] = 0;
		ivrPromptSet[promptSetName] = externalIvrMediaFile;
		m_appServerIvrInfo[appServerIp] = ivrPromptSet;
	}
}
//===============================================================================================================//
// TELEPRESENCE_LAYOUTS
STATUS CConfPartyManager::OnServerUpdateTelepresenceLayoutMode(CRequest *pRequest)
{

	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
	{
		TRACEINTO << "No permission to OnServerUpdateTelepresenceLayoutMode for administrator readony";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	STATUS status = STATUS_OK;
	CConfAction* pConfAction = new CConfAction;
	*pConfAction = *(CConfAction*)pRequest->GetRequestObject() ;
	pRequest->SetObjectFlag(STRING_FLAG);

	const DWORD confId   = pConfAction->GetConfID();
	DWORD telepresenceLayoutMode   = pConfAction->GetNumAction();

	CCommConf* pCurCommConf = (CCommConf*)::GetpConfDB()->GetCurrentConf(confId);
	if(NULL == pCurCommConf){
		TRACEINTO << " conf not exists";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_CONF_NOT_EXISTS);
		return STATUS_CONF_NOT_EXISTS;
	}

	ETelePresenceLayoutMode newLayoutMode = (ETelePresenceLayoutMode)telepresenceLayoutMode;
	// TELEPRESENCE_LAYOUTS
	if(newLayoutMode <= eTelePresenceLayoutCpParticipantsPriority){
		TRACEINTO << "TELEPRESENCE_LAYOUTS_DEBUG newLayoutMode = " << TelePresenceLayoutModeToString(newLayoutMode);
	}else{
		PASSERT(newLayoutMode);
		delete(pConfAction);
		TRACEINTO << "wrong newLayoutMode = " << TelePresenceLayoutModeToString(newLayoutMode);
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_FAIL);
		return STATUS_FAIL;
	}

	UpdateConfTelepresenceLayoutMode(*pCurCommConf, newLayoutMode);

	return STATUS_OK;
}
//===============================================================================================================//
// TELEPRESENCE_LAYOUTS
STATUS CConfPartyManager::HandleSetTelepresenceLayoutMode(CTerminalCommand& command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
			answer << "error: Conf/Mode names should be specified\n";
			answer << "usage: Bin/McuCmd SetTelepresenceLayoutMode ConfParty [Conf Name] [Telepresence Layout Mode: room_switch/cp_mla/speaker_priority/participants_priority/manual  ]\n";
			return STATUS_FAIL;
	}

	const string &confName =  command.GetToken(eCmdParam1);
	const string &layoutMode = command.GetToken(eCmdParam2);
	// Find Conf in DB
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommConf* pRequestedConf = NULL;

	 pRequestedConf = pCommConfDB->GetCurrentConf(confName.c_str());
	if (!CPObject::IsValidPObjectPtr(pRequestedConf))
	{
		answer << "error: Conf does not exist in DB " <<  confName << " " << "SetTelepresenceLayoutMode ";
		return STATUS_FAIL;
	}

	ETelePresenceLayoutMode newLayoutMode = eTelePresenceLayoutManual;
	if(layoutMode == "room_switch"){
		newLayoutMode = eTelePresenceLayoutRoomSwitch;
	}else if(layoutMode == "cp_mla"){
		newLayoutMode = eTelePresenceLayoutContinuousPresence;
	}else if(layoutMode == "speaker_priority"){
		newLayoutMode = eTelePresenceLayoutCpSpeakerPriority;
	}else if(layoutMode == "participants_priority"){
		newLayoutMode = eTelePresenceLayoutCpParticipantsPriority;
	}else if(layoutMode == "manual"){
		newLayoutMode = eTelePresenceLayoutManual;
	}else{
		answer << "error: illegal telepresence layout mode " <<  newLayoutMode << " " << "SetTelepresenceLayoutMode ";
		return STATUS_FAIL;
	}

	TRACEINTO << "TELEPRESENCE_LAYOUTS_DEBUG newLayoutMode = " << TelePresenceLayoutModeToString(newLayoutMode);

	UpdateConfTelepresenceLayoutMode(*pRequestedConf, newLayoutMode);

	return STATUS_OK;
}
//===============================================================================================================//
// TELEPRESENCE_LAYOUTS
void CConfPartyManager::UpdateConfTelepresenceLayoutMode(CCommConf& rCommConf, ETelePresenceLayoutMode newLayoutMode)
{
	TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG conf name: "<< rCommConf.GetName() << " , Layout Mode: " << TelePresenceLayoutModeToString(newLayoutMode);

	ETelePresenceLayoutMode oldLayoutMode = (ETelePresenceLayoutMode)(rCommConf.GetTelePresenceLayoutMode());
	if(newLayoutMode == oldLayoutMode){
		TRACEINTO << "no change in layout mode - do nothing";
		return;
	}

	rCommConf.SetTelePresenceLayoutMode((BYTE)newLayoutMode);
    // drop party
    CConfApi confApi;
    confApi.CreateOnlyApi(*(rCommConf.GetRcvMbx()));
    confApi.UpdateConfTelepresenceLayoutMode(newLayoutMode);
    confApi.DestroyOnlyApi();
}
//===============================================================================================================//
STATUS CConfPartyManager::HandleLoadReserveGridMapFromXML(CTerminalCommand& command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Conf/Mode names should be specified\n";
		answer << "usage: Bin/McuCmd LoadReserveGridMapFromXML ConfParty FileName \n";
		return STATUS_FAIL;
	}

	const string &fileName =  command.GetToken(eCmdParam1);

	std::string fullFilePath = (std::string)getenv("PWD") + std::string("/Cfg/")+fileName;

	TRACEINTO << "fullFilePath: " << fullFilePath.c_str() << "home: " << getenv("PWD");

	std::ifstream ifs;
	ifs.open(fullFilePath.c_str(), std::ios::in);

	if (ifs.good())
	{
		ifs.close();
		CPLayoutWrapper wrapper;
		wrapper.LoadGridReservedMapAndCreateXMLIfNeeded(CTelepresenceSpeakerModeLayoutLogic::m_reservedScreenLayoutMap,CTelepresenceCpLayoutLogic::m_gridScreenLayoutMap, fileName);
		return STATUS_OK;
	}
	else
	{
		ifs.close();
		answer << "error: File does not exist, check if it exists in /Cfg/" << fileName <<" \n";
		return STATUS_FAIL;
	}
}
