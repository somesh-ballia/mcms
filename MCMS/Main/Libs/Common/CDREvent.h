#ifndef _CDREVENTS
#define _CDREVENTS

#include <string>
#include "CDRDefines.h"
#include "ConfPartySharedDefines.h"
#include "DefinesIpService.h"
#include "StructTm.h"
#include "SharedDefines.h"
#include "IVRAvMsgStruct.h"
#include "LectureModeParams.h"
#include "CDRShort.h"

class ACCMCUTime;
class ACCResourceForce;
class CH221Str;
class CServicePhoneStr;
class CAvMsgStruct;

#define IP_LIMIT_ADDRESS_CHAR_LEN 256
#define MAX_DTMF_STRING_LENGTH    32
#define MAX_USER_ACCOUNT_LEN      32
#define MAX_SIP_PRIVATE_EXT_LEN   256
#define SIZE_OF_CALL_ID           16

struct ACCCdrPhone
{
  std::string phone_number;

public:
  bool operator ==(const ACCCdrPhone& other)
  {
    if (this == &other)
      return true;
    return phone_number == other.phone_number;
  }

  bool operator !=(const ACCCdrPhone& other)
  {
    return !operator ==(other);
  }
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCalledParty
////////////////////////////////////////////////////////////////////////////
class ACCCalledParty
{
public:
                                     ACCCalledParty();
                                     ACCCalledParty(const ACCCalledParty& other);
  virtual                           ~ACCCalledParty();

  ACCCalledParty&                    operator= (const ACCCalledParty& other);
  bool                               operator==(const ACCCalledParty& other);
  bool                               operator!=(const ACCCalledParty& other);

  BYTE                         GetCalledNumType() const;
  BYTE                         GetCalledNumPlan() const;
  const char*                        GetCalledPhoneNum() const;

protected:
  BYTE                               m_called_numType;
  BYTE                               m_called_numPlan;
  char                               m_called_digits[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCallingParty
////////////////////////////////////////////////////////////////////////////
class ACCCallingParty
{
public:
                                     ACCCallingParty();
                                     ACCCallingParty(const ACCCallingParty& other);
  virtual                           ~ACCCallingParty();

  ACCCallingParty&                   operator= (const ACCCallingParty& other);
  bool                               operator==(const ACCCallingParty& other);
  bool                               operator!=(const ACCCallingParty& other);

  BYTE                         GetCallingNumType() const;
  BYTE                         GetCallingNumPlan() const;
  BYTE                         GetCallingPresentInd() const;
  BYTE                         GetCallingScreenInd() const;
  const char*                        GetCallingPhoneNum() const;

protected:
  BYTE                               m_numType;
  BYTE                               m_numPlan;
  BYTE                               m_presentationInd;
  BYTE                               m_screeningInd;
  char                               m_digits[PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER];
};


////////////////////////////////////////////////////////////////////////////
//                        CCardRsrsStructBase
////////////////////////////////////////////////////////////////////////////
class CCardRsrsStructBase
{
public:
                                     CCardRsrsStructBase();
                                     CCardRsrsStructBase(const CCardRsrsStructBase& other);
  virtual                           ~CCardRsrsStructBase();

  bool                               operator==(const CCardRsrsStructBase& other);
  bool                               operator!=(const CCardRsrsStructBase& other);

  WORD                         GetAudioBoardId() const;
  WORD                         GetAudioUnitId() const;
  WORD                         GetVideoBoardId() const;
  WORD                         GetVideoUnitId() const;
  WORD                         GetDataBoardId() const;
  WORD                         GetDataUnitId() const;

protected:
  WORD                               m_audio_board_id;
  WORD                               m_audio_unit_id;
  WORD                               m_video_board_id;
  WORD                               m_video_unit_id;
  WORD                               m_data_board_id;
  WORD                               m_data_unit_id;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCLectureMode
////////////////////////////////////////////////////////////////////////////
class ACCLectureMode
{
public:
                                     ACCLectureMode();
                                     ACCLectureMode(const ACCLectureMode& other);
  virtual                           ~ACCLectureMode();

  ACCLectureMode&                    operator= (const ACCLectureMode& other);
  ACCLectureMode&                    operator= (const CLectureModeParams& other);
  bool                               operator==(const ACCLectureMode& other);
  bool                               operator!=(const ACCLectureMode& other);

  BYTE                               GetLectureModeOnOff() const;
  const char*                        GetSetLecturerName() const;
  WORD                               GetLectureTimeInterval() const;
  BYTE                               GetTimerOnOff() const;
  BYTE                               GetAudioActivated() const;
  DWORD                              GetLecturerId() const;

  // From version API_NUM_LECTURE_SHOW (150) m_LectureModeOnOff may be 0/1/2
  // from API_NUM_AUTO_LAYOUT m_LectureModeOnOff may be 0/1/2/3
  BYTE                               m_LectureModeOnOff;
  char                               m_LecturerName[H243_NAME_LEN];
  WORD                               m_TimeInterval;
  BYTE                               m_timerOnOff;
  BYTE                               m_audioActivated;
  DWORD                              m_lecturerId;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCAvMsgStruct
////////////////////////////////////////////////////////////////////////////
class ACCAvMsgStruct
{
public:
                                     ACCAvMsgStruct();
                                     ACCAvMsgStruct(const ACCAvMsgStruct& other);
  virtual                           ~ACCAvMsgStruct();

  ACCAvMsgStruct&                    operator=(const ACCAvMsgStruct& other);
  ACCAvMsgStruct&                    operator=(const CAvMsgStruct& other);

  BYTE                         GetAttendedWelcome() const;
  const char*                        GetAvMsgServiceName() const;
  void                               SetAttendedWelcome(const BYTE attended_welcome);
  void                               SetAvMsgServiceName(const char* av_msg_service_name);

protected:
  BYTE                               m_attended_welcome;
  char                               m_av_msg_service_name[AV_SERVICE_NAME];
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventRemoteCommMode
////////////////////////////////////////////////////////////////////////////
class ACCCDREventRemoteCommMode
{
public:
                                     ACCCDREventRemoteCommMode();
                                     ACCCDREventRemoteCommMode(const ACCCDREventRemoteCommMode& other);
  virtual                           ~ACCCDREventRemoteCommMode();

  ACCCDREventRemoteCommMode&         operator= (const ACCCDREventRemoteCommMode& other);
  bool                               operator==(const ACCCDREventRemoteCommMode& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  const CH221Str*                    GetRemoteCommMode() const;

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  CH221Str*                          m_pRemoteCommMode;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventPartyConnected
////////////////////////////////////////////////////////////////////////////
class ACCCDREventPartyConnected : public ACCCDREventRemoteCommMode
{
public:
                                     ACCCDREventPartyConnected();
                                     ACCCDREventPartyConnected(const ACCCDREventPartyConnected& other);
  virtual                           ~ACCCDREventPartyConnected();

  ACCCDREventPartyConnected&         operator= (const ACCCDREventPartyConnected& other);
  bool                               operator==(const ACCCDREventPartyConnected& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  DWORD                        GetPartyState() const;
  BYTE                         GetSecondaryCause() const;

  const CH221Str*                    GetCapabilities() const;
  const CH221Str*                    GetRemoteCommMode() const;

protected:
  DWORD                              m_party_state;
  BYTE                               m_second_cause;
  CH221Str*                          m_pCapabilities;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventPartyDisconnected
////////////////////////////////////////////////////////////////////////////
class ACCCDREventPartyDisconnected
{
public:
                                     ACCCDREventPartyDisconnected();
                                     ACCCDREventPartyDisconnected(const ACCCDREventPartyDisconnected& other);
  virtual                           ~ACCCDREventPartyDisconnected();

  ACCCDREventPartyDisconnected&      operator=(const ACCCDREventPartyDisconnected& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  DWORD                        GetDisconctCause() const;
  DWORD                        GetQ931DisonctCause() const;

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  DWORD                              m_disconct_cause;
  DWORD                              m_q931_disconct_cause;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDRPartyErrors
////////////////////////////////////////////////////////////////////////////
class ACCCDRPartyErrors
{
public:
                                     ACCCDRPartyErrors();
                                     ACCCDRPartyErrors(const ACCCDRPartyErrors& other);
  virtual                           ~ACCCDRPartyErrors();

  ACCCDRPartyErrors&                 operator=(const ACCCDRPartyErrors& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  DWORD                        GetResrcFailure() const;
  DWORD                        GetNumH221SyncLoss() const;
  DWORD                        GetDurationH221SyncLoss() const;
  DWORD                        GetNumH221SyncRLoss() const;
  DWORD                        GetDurationH221SyncRLoss() const;
  DWORD                        GetNumberRemoteVcu() const;

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  BYTE                               m_resrc_failure;
  DWORD                              m_numH221_sync_loss;
  DWORD                              m_duration_h221_sync_loss;
  DWORD                              m_numH221_syncR_loss;
  DWORD                              m_duration_h221_syncR_loss;
  DWORD                              m_num_remote_vcu;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventPartyDisconnectedCont1
////////////////////////////////////////////////////////////////////////////
class ACCCDREventPartyDisconnectedCont1
{
public:
                                     ACCCDREventPartyDisconnectedCont1();
                                     ACCCDREventPartyDisconnectedCont1(const ACCCDREventPartyDisconnectedCont1& other);
  virtual                           ~ACCCDREventPartyDisconnectedCont1();

  ACCCDREventPartyDisconnectedCont1& operator=(const ACCCDREventPartyDisconnectedCont1& other);

  WORD                               GetL_syncLostCounter() const;
  WORD                               GetR_syncLostCounter() const;
  WORD                               GetL_videoSyncLostCounter() const;
  WORD                               GetR_videoSyncLostCounter() const;
  WORD                               GetMuxBoardId() const;
  WORD                               GetMuxUnitId() const;
  WORD                               GetAudioCodecBoardId() const;
  WORD                               GetAudioCodecUnitId() const;
  WORD                               GetAudioBrgBoardId() const;
  WORD                               GetAudioBrgUnitId() const;
  WORD                               GetVideoBoardId() const;
  WORD                               GetVideoUnitId() const;
  WORD                               GetT120BoardId() const;
  WORD                               GetT120UnitId() const;
  WORD                               GetMCSBoardId() const;
  WORD                               GetMCSUnitId() const;
  WORD                               GetH323BoardId() const;
  WORD                               GetH323UnitId() const;

protected:
  WORD                               m_L_syncLostCounter;
  WORD                               m_R_syncLostCounter;
  WORD                               m_L_videoSyncLostCounter;
  WORD                               m_R_videoSyncLostCounter;
  WORD                               m_MuxBoardId;
  WORD                               m_MuxUnitId;
  WORD                               m_AudioCodecBoardId;
  WORD                               m_AudioCodecUnitId;
  WORD                               m_AudioBrgBoardId;
  WORD                               m_AudioBrgUnitId;
  WORD                               m_VideoBoardId;
  WORD                               m_VideoUnitId;
  WORD                               m_T120BoardId;
  WORD                               m_T120UnitId;
  WORD                               m_MCSBoardId;
  WORD                               m_MCSUnitId;
  WORD                               m_H323BoardId;
  WORD                               m_H323UnitId;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventConfStart
////////////////////////////////////////////////////////////////////////////
class ACCCDREventConfStart
{
public:
                                     ACCCDREventConfStart();
                                     ACCCDREventConfStart(const ACCCDREventConfStart& other);
  virtual                           ~ACCCDREventConfStart();

  ACCCDREventConfStart&              operator=(const ACCCDREventConfStart& other);

  BYTE                         GetStandBy() const;
  BYTE                         GetAutoTerminate() const;
  BYTE                         GetConfTransfRate() const;
  BYTE                         GetRestrictMode() const;
  BYTE                         GetAudioRate() const;
  BYTE                         GetVideoSession() const;
  BYTE                         GetVideoPicFormat() const;
  BYTE                         GetCifFrameRate() const;
  BYTE                         GetQcifFrameRate() const;
  BYTE                         GetLsdRate() const;
  BYTE                         GetHsdRate() const;
  BYTE                         GetT120BitRate() const;

protected:
  BYTE                               m_stand_by;
  BYTE                               m_auto_terminate;
  BYTE                               m_conf_transfer_rate;
  BYTE                               m_restrict_mode;
  BYTE                               m_audio_rate;
  BYTE                               m_video_session;
  BYTE                               m_video_pic_format;
  BYTE                               m_CIF_frame_rate;
  BYTE                               m_QCIF_frame_rate;
  BYTE                               m_LSD_rate;
  BYTE                               m_HSD_rate;
  BYTE                               m_T120_bit_rate;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventConfStartCont1
////////////////////////////////////////////////////////////////////////////
class ACCCDREventConfStartCont1 : public ACCCDREventConfStart
{
public:
                                     ACCCDREventConfStartCont1();
                                     ACCCDREventConfStartCont1(const ACCCDREventConfStartCont1& other);
  virtual                           ~ACCCDREventConfStartCont1();

  ACCCDREventConfStartCont1&         operator=(const ACCCDREventConfStartCont1& other);

  DWORD                        GetAudioTone() const;
  BYTE                         GetAlertToneTiming() const;
  WORD                         GetTalkHoldTime() const;
  BYTE                         GetAudioMixDepth() const;
  BYTE                         GetOperatorConf() const;
  BYTE                         GetVideoProtocol() const;  // H261, H263, Auto               //VERSION 1.3
  BYTE                         GetMeetMePerConf() const;  // YES, NO                        //VERSION 1.3
  WORD                               GetNumServicePhone() const;
  CServicePhoneStr*                  GetFirstServicePhone();
  CServicePhoneStr*                  GetNextServicePhone();
  int                                FindServicePhone(const CServicePhoneStr& other);

  const char*                        GetConf_password() const;
  BYTE                         GetChairMode() const;
  BYTE                         GetCascadeMode() const;
  const char*                        GetMasterName() const;
  WORD                         GetNumUndefParties() const;
  BYTE                         GetUnlimited_conf_flag() const;
  BYTE                         GetTime_beforeFirstJoin() const;
  BYTE                         GetTime_afterLastQuit() const;
  BYTE                         GetConfLockFlag() const;
  WORD                         GetMax_parties() const;
  CCardRsrsStructBase*               GetpCardRsrsStruct();
  CAvMsgStruct*                      GetpAvMsgStruct();
  ACCLectureMode*                    GetpLectureMode();

protected:

  DWORD                              m_audioTone;
  BYTE                               m_alertToneTiming;
  WORD                               m_talkHoldTime;
  BYTE                               m_audioMixDepth;
  BYTE                               m_operatorConf;
  BYTE                               m_videoProtocol;
  BYTE                               m_meetMePerConf;
  WORD                               m_numServicePhoneStr;
  WORD                               m_numServicePhone;
  CServicePhoneStr*                  m_pServicePhoneStr[MAX_NET_SERV_PROVIDERS_IN_LIST];
  char                               m_conf_password[H243_NAME_LEN]; // password
  BYTE                               m_chairMode;
  BYTE                               m_cascadeMode;
  char                               m_masterName[H243_NAME_LEN];
  WORD                               m_numUndefParties;
  BYTE                               m_unlimited_conf_flag;
  BYTE                               m_time_beforeFirstJoin;
  BYTE                               m_time_afterLastQuit;
  BYTE                               m_confLockFlag;
  WORD                               m_max_parties; // API - 137; changed from BYTE to WORD
  CCardRsrsStructBase*               m_pCardRsrsStruct;
  CAvMsgStruct*                      m_pAvMsgStruct;
  ACCLectureMode*                    m_pLectureMode;
  WORD                               m_ind_service_phone;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventConfStartCont2
////////////////////////////////////////////////////////////////////////////
class ACCCDREventConfStartCont2 : public ACCCDREventConfStart
{
public:
                                     ACCCDREventConfStartCont2();
                                     ACCCDREventConfStartCont2(const ACCCDREventConfStartCont2& other);
  virtual                           ~ACCCDREventConfStartCont2();

  ACCCDREventConfStartCont2&         operator=(const ACCCDREventConfStartCont2& other);

  BYTE                         GetwebReserved() const;
  DWORD                        GetwebReservUId() const;
  DWORD                        GetwebDBId() const;

protected:
  BYTE                               m_webReserved;
  DWORD                              m_webReservUId;
  DWORD                              m_webDBId;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventConfStartCont3
////////////////////////////////////////////////////////////////////////////
class ACCCDREventConfStartCont3 : public ACCCDREventConfStart
{
public:
                                     ACCCDREventConfStartCont3();
                                     ACCCDREventConfStartCont3(const ACCCDREventConfStartCont3& other);
  virtual                           ~ACCCDREventConfStartCont3();

  ACCCDREventConfStartCont3&         operator=(const ACCCDREventConfStartCont3& other);

  const char*                        GetConfRemarks() const;

protected:
  char                               m_confRemarks[CONF_REMARKS_LEN];
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventOperSetEndTime
////////////////////////////////////////////////////////////////////////////
class ACCCDREventOperSetEndTime
{
public:
                                     ACCCDREventOperSetEndTime()                          { memset(this, 0, sizeof(*this)); }
                                     ACCCDREventOperSetEndTime(const ACCCDREventOperSetEndTime& other) { *this = other; }

  ACCCDREventOperSetEndTime&         operator=(const ACCCDREventOperSetEndTime& other)    { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetOperatorName() const                              { return m_operator_name; }
  void                               SetOperatorName(const char* name)                    { SAFE_COPY(m_operator_name, name); }

  const CStructTm*                   GetNewEndTime() const                                { return &m_new_end_time; }
  void                               SetNewEndTime(const CStructTm& other)                { m_new_end_time = other; }

protected:
  char                               m_operator_name[OPERATOR_NAME_LEN];
  CStructTm                          m_new_end_time;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventOperDelParty
////////////////////////////////////////////////////////////////////////////
class ACCCDREventOperDelParty
{
public:
                                     ACCCDREventOperDelParty()                            { memset(this, 0, sizeof(*this)); m_partyId = 0xffffffff; }
                                     ACCCDREventOperDelParty(const ACCCDREventOperDelParty& other) { *this = other; }

  ACCCDREventOperDelParty&           operator=(const ACCCDREventOperDelParty& other)      { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetOperatorName() const                              { return m_operator_name; }
  void                               SetOperatorName(const char* name)                    { SAFE_COPY(m_operator_name, name); }

  const char*                        GetPartyName() const                                 { return m_party_name; }
  void                               SetPartyName(const char* name)                       { SAFE_COPY(m_party_name, name); }

  DWORD                        GetPartyId() const                                   { return m_partyId; }
  void                               SetPartyId(const DWORD partyId)                      { m_partyId = partyId; }

protected:
  char                               m_operator_name[OPERATOR_NAME_LEN];
  char                               m_party_name[H243_NAME_LEN];
  DWORD                              m_partyId;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventOperAddParty
////////////////////////////////////////////////////////////////////////////
class ACCCDREventOperAddParty : public ACCCDREventOperDelParty
{
public:
                                     ACCCDREventOperAddParty();
                                     ACCCDREventOperAddParty(const ACCCDREventOperAddParty& other);
                                    ~ACCCDREventOperAddParty();

  ACCCDREventOperAddParty&           operator =(const ACCCDREventOperAddParty& other);
  bool                               operator==(const ACCCDREventOperAddParty& other);

  int                                FindPartyPhoneNumber(const char* partyphoneNumber);
  ACCCdrPhone*                       GetFirstPartyPhoneNumber();
  ACCCdrPhone*                       GetFirstPartyPhoneNumber(int& nPos);
  ACCCdrPhone*                       GetNextPartyPhoneNumber();
  ACCCdrPhone*                       GetNextPartyPhoneNumber(int& nPos);
  int                                FindMcuPhoneNumber(const char* mcuphoneNumber);
  ACCCdrPhone*                       GetFirstMcuPhoneNumber();
  ACCCdrPhone*                       GetFirstMcuPhoneNumber(int& nPos);
  ACCCdrPhone*                       GetNextMcuPhoneNumber();
  ACCCdrPhone*                       GetNextMcuPhoneNumber(int& nPos);

  BYTE                               GetConnectionType() const                            { return m_connection_type; }
  void                               SetConnectionType(const BYTE connection_type)        { m_connection_type = connection_type; }

  BYTE                               GetBondingMode() const                               { return m_bonding_mode; }
  void                               SetBondingMode(const BYTE bonding_mode)              { m_bonding_mode = bonding_mode; }

  BYTE                               GetNetNumberChannel() const                          { return m_net_num_of_channel; }
  void                               SetNetNumberChannel(const BYTE net_numof_channel)    { m_net_num_of_channel = net_numof_channel; }

  BYTE                               GetNetChannelWidth() const                           { return m_net_channel_width; }
  void                               SetNetChannelWidth(const BYTE net_channel_width)     { m_net_channel_width = net_channel_width; }

  char*                              GetNetServiceName()                                  { return m_net_service_name; }
  void                               SetNetServiceName(char* name)                        { SAFE_COPY(m_net_service_name, name); }

  BYTE                               GetRestrict() const                                  { return m_restrict; }
  void                               SetRestrict(const BYTE restrict)                     { m_restrict = restrict; }

  BYTE                               GetVoice() const                                     { return m_voice; }
  void                               SetVoice(const BYTE voice)                           { m_voice = voice; }

  BYTE                               GetNumType() const                                   { return m_num_type; }
  void                               SetNumType(const BYTE num_type)                      { m_num_type = num_type; }

  char*                              GetNetSubServiceName()                               { return m_net_subservice_name; }
  void                               SetNetSubServiceName(char* name)                     { SAFE_COPY(m_net_subservice_name, name); }

  WORD                               GetNumPartyPhoneNumbers()                            { return m_NumPartyPhoneNumber; }
  void                               SetNumPartyPhoneNumbers(const WORD partyPhoneNumber) { m_NumPartyPhoneNumber = partyPhoneNumber; }

  WORD                               GetNumMcuPhoneNumbers()                              { return m_NumMcuPhoneNumber; }
  void                               SetNumMcuPhoneNumbers(const BYTE mcuPhoneNumber)     { m_NumMcuPhoneNumber = mcuPhoneNumber; }

  BYTE                               GetIdentMethod() const                               { return m_ident_method; }
  void                               SetIdentMethod(const BYTE ident_method)              { m_ident_method = ident_method; }

  BYTE                               GetMeetMeMethod() const                              { return m_meet_me_method; }
  void                               SetMeetMeMethod(const BYTE meet_me_method)           { m_meet_me_method = meet_me_method; }

protected:
  BYTE                               m_connection_type;
  BYTE                               m_bonding_mode;
  BYTE                               m_net_num_of_channel;
  BYTE                               m_net_channel_width;
  char                               m_net_service_name[NET_SERVICE_PROVIDER_NAME_LEN];
  BYTE                               m_restrict;
  BYTE                               m_voice;
  BYTE                               m_num_type;
  char                               m_net_subservice_name[NET_SERVICE_PROVIDER_NAME_LEN];
  BYTE                               m_ident_method;
  BYTE                               m_meet_me_method;
  ACCCdrPhone*                       m_pPartyPhoneNumberList[MAX_CHANNEL_NUMBER];
  ACCCdrPhone*                       m_pMcuPhoneNumberList[MAX_CHANNEL_NUMBER];
  BYTE                               m_NumPartyPhoneNumber;
  BYTE                               m_NumMcuPhoneNumber;
  BYTE                               m_indpartyphone;
  BYTE                               m_indmcuphone;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventOperAddPartyCont1
////////////////////////////////////////////////////////////////////////////
class ACCCDREventOperAddPartyCont1
{
public:
                                     ACCCDREventOperAddPartyCont1();
                                     ACCCDREventOperAddPartyCont1(const ACCCDREventOperAddPartyCont1& other);
                                    ~ACCCDREventOperAddPartyCont1();

  ACCCDREventOperAddPartyCont1&      operator=(const ACCCDREventOperAddPartyCont1& other);

  BYTE                               GetNodeType() const                                  { return m_nodeType; }
  void                               SetNodeType(const BYTE nodeType)                     { m_nodeType = nodeType; }

  BYTE                               GetChair() const                                     { return m_chair; }
  void                               SetChair(const BYTE chair)                           { m_chair = chair; }

  const char*                        GetPassword() const                                  { return m_H243_password; }
  void                               SetPassword(const char* name)                        { SAFE_COPY(m_H243_password, name); }

  DWORD                              GetIpAddress() const                                 { return m_ipAddress; }
  void                               SetIpAddress(const DWORD ipAddress)                  { m_ipAddress = ipAddress; }

  WORD                               GetCallSignallingPort() const                        { return m_callSignallingPort; }
  void                               SetCallSignallingPort(const WORD callSignallingPort) { m_callSignallingPort = callSignallingPort; }

  BYTE                               GetVideoProtocol() const                             { return m_videoProtocol; }
  void                               SetVideoProtocol(const BYTE videoProtocol)           { m_videoProtocol = videoProtocol; }

  DWORD                              GetVideoRate() const                                 { return m_videoRate; }
  void                               SetVideoRate(const DWORD videoRate)                  { m_videoRate = videoRate; }

  const ACCCdrPhone*                 GetBondingPhoneNumber() const                        { return m_bondingPhoneNumber; }
  void                               SetBondingPhoneNumber(const ACCCdrPhone* other)      { m_bondingPhoneNumber->phone_number = other->phone_number; }

  WORD                               GetIpPartyAliasType() const                          { return m_IpPartyAliasType; }
  void                               SetIpPartyAliasType(const WORD IpPartyAliasType)     { m_IpPartyAliasType = IpPartyAliasType; }

  const char*                        GetIpPartyAlias() const                              { return m_IpPartyAlias; }
  void                               SetIpPartyAlias(const char* name)                    { SAFE_COPY(m_IpPartyAlias, name); }

  WORD                               GetSipPartyAddressType() const                       { return m_sipPartyAddressType; }
  void                               SetSipPartyAddressType(const WORD partyAddressType)  { m_sipPartyAddressType = partyAddressType; }

  const char*                        GetSipPartyAddress() const                           { return m_sipPartyAddress; }
  void                               SetSipPartyAddress(const char* name)                 { SAFE_COPY(m_sipPartyAddress, name); }

  BYTE                               GetAudioVolume() const                               { return m_audioVolume; }
  void                               SetAudioVolume(const BYTE audioVolume)               { m_audioVolume = audioVolume; }

  BYTE                               GetUndefinedType() const                             { return m_undefinedType; }
  void                               SetUndefinedType(const BYTE undefinedType)           { m_undefinedType = undefinedType; }

  BYTE                               GetNetInterfaceType() const                          { return m_netInterfaceType; }
  void                               SetNetInterfaceType(const BYTE interfaceType)        { m_netInterfaceType = interfaceType; }

  WORD                               IsUndefinedParty()                                   { return (m_undefinedType == UNDEFINED_PARTY || m_undefinedType == UNRESERVED_PARTY); }

protected:
  WORD                               m_numCallingPhones;
  WORD                               m_numCalledPhones;
  BYTE                               m_netInterfaceType;
  char                               m_H243_password[H243_NAME_LEN];
  BYTE                               m_chair;
  BYTE                               m_videoProtocol;
  DWORD                              m_videoRate;
  BYTE                               m_audioVolume;
  BYTE                               m_undefinedType;
  BYTE                               m_nodeType;
  DWORD                              m_ipAddress;           // relevant only for 323party.
  WORD                               m_callSignallingPort;  // relevant only for 323party.
  ACCCdrPhone*                       m_bondingPhoneNumber;
  WORD                               m_IpPartyAliasType;
  char                               m_IpPartyAlias[IP_STRING_LEN];
  WORD                               m_sipPartyAddressType;
  char                               m_sipPartyAddress[IP_STRING_LEN];
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventOperAddPartyCont2
////////////////////////////////////////////////////////////////////////////
class ACCCDREventOperAddPartyCont2
{
public:
                                     ACCCDREventOperAddPartyCont2()                       { memset(this, 0, sizeof(*this)); m_partyId = 0xffffffff; }
                                     ACCCDREventOperAddPartyCont2(const ACCCDREventOperAddPartyCont2& other) { *this = other; }

  ACCCDREventOperAddPartyCont2&      operator=(const ACCCDREventOperAddPartyCont2& other) { memcpy(this, &other, sizeof(*this)); return *this; }

  BYTE                         GetIsEncryptedParty() const                          { return m_encrypted; }
  void                               SetIsEncryptedParty(const BYTE is_encrypted)         { m_encrypted = is_encrypted; }

  const char*                        GetPartyName() const                                 { return m_partyName; }
  void                               SetPartyName(const char* name)                       { SAFE_COPY(m_partyName, name); }

  DWORD                        GetPartyId() const                                   { return m_partyId; }
  void                               SetPartyId(const DWORD party_id)                     { m_partyId = party_id; }

protected:
  BYTE                               m_encrypted;
  char                               m_partyName [H243_NAME_LEN];
  DWORD                              m_partyId;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventOperMoveParty
////////////////////////////////////////////////////////////////////////////
class ACCCDREventOperMoveParty
{
public:
                                     ACCCDREventOperMoveParty()                           { memset(this, 0, sizeof(*this)); m_partyId = 0xffffffff; m_dest_conf_id = 0xffffffff; }
                                     ACCCDREventOperMoveParty(const ACCCDREventOperMoveParty& other) { *this = other; }

  ACCCDREventOperMoveParty&          operator=(const ACCCDREventOperMoveParty& other)     { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetOperatorName() const                              { return m_operator_name; }
  void                               SetOperatorName(const char* name)                    { SAFE_COPY(m_operator_name, name); }

  const char*                        GetPartyName() const                                 { return m_party_name; }
  void                               SetPartyName(const char* name)                       { SAFE_COPY(m_party_name, name); }

  DWORD                        GetPartyId() const                                   { return m_partyId; }
  void                               SetPartyId(const DWORD party_id)                     { m_partyId = party_id; }

  const char*                        GetDestConfName() const                              { return m_dest_conf_name; }
  void                               SetDestConfName(const char* name)                    { SAFE_COPY(m_dest_conf_name, name); }

  DWORD                        GetDestConfId() const                                { return m_dest_conf_id; }
  void                               SetDestConfId(const DWORD dest_conf_id)              { m_dest_conf_id = dest_conf_id; }

protected:
  char                               m_operator_name[OPERATOR_NAME_LEN];
  char                               m_party_name[H243_NAME_LEN];
  DWORD                              m_partyId;
  char                               m_dest_conf_name[H243_NAME_LEN];
  DWORD                              m_dest_conf_id;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventAddPartyDetailed
////////////////////////////////////////////////////////////////////////////
class ACCCDREventAddPartyDetailed : public ACCCDREventOperDelParty
{
public:
                                     ACCCDREventAddPartyDetailed();
                                     ACCCDREventAddPartyDetailed(const ACCCDREventAddPartyDetailed& other);
                                    ~ACCCDREventAddPartyDetailed();

  ACCCDREventAddPartyDetailed&       operator= (const ACCCDREventAddPartyDetailed& other);
  bool                               operator==(const ACCCDREventAddPartyDetailed& other);

  int                                FindPartyPhoneNumber(const char* partyphoneNumber);
  ACCCdrPhone*                       GetFirstPartyPhoneNumber();
  ACCCdrPhone*                       GetFirstPartyPhoneNumber(int& nPos);
  ACCCdrPhone*                       GetNextPartyPhoneNumber();
  ACCCdrPhone*                       GetNextPartyPhoneNumber(int& nPos);

  int                                FindMcuPhoneNumber(const char* mcuphoneNumber);
  ACCCdrPhone*                       GetFirstMcuPhoneNumber();
  ACCCdrPhone*                       GetFirstMcuPhoneNumber(int& nPos);
  ACCCdrPhone*                       GetNextMcuPhoneNumber();
  ACCCdrPhone*                       GetNextMcuPhoneNumber(int& nPos);

  BYTE                               GetConnectionType() const                            { return m_connection_type; }
  void                               SetConnectionType(const BYTE connection_type)        { m_connection_type = connection_type; }

  BYTE                               GetBondingMode() const                               { return m_bonding_mode; }
  void                               SetBondingMode(const BYTE bonding_mode)              { m_bonding_mode = bonding_mode; }

  BYTE                               GetNetNumberChannel() const                          { return m_net_num_of_channel; }
  void                               SetNetNumberChannel(const BYTE net_numof_channel)    { m_net_num_of_channel = net_numof_channel; }

  BYTE                               GetNetChannelWidth() const                           { return m_net_channel_width; }
  void                               SetNetChannelWidth(const BYTE net_channel_width)     { m_net_channel_width = net_channel_width; }

  char*                              GetNetServiceName()                                  { return m_net_service_name; }
  void                               SetNetServiceName(char* name)                        { SAFE_COPY(m_net_service_name, name); }

  BYTE                               GetRestrict() const                                  { return m_restrict; }
  void                               SetRestrict(const BYTE restrict)                     { m_restrict = restrict; }

  BYTE                               GetVoice() const                                     { return m_voice; }
  void                               SetVoice(const BYTE voice)                           { m_voice = voice; }

  BYTE                               GetNumType() const                                   { return m_num_type; }
  void                               SetNumType(const BYTE num_type)                      { m_num_type = num_type; }

  char*                              GetNetSubServiceName()                               { return m_net_subservice_name; }
  void                               SetNetSubServiceName(char* name)                     { SAFE_COPY(m_net_subservice_name, name); }

  BYTE                               GetIdentMethod() const                               { return m_ident_method; }
  void                               SetIdentMethod(const BYTE ident_method)              { m_ident_method = ident_method; }

  BYTE                               GetMeetMeMethod() const                              { return m_meet_me_method; }
  void                               SetMeetMeMethod(const BYTE meet_me_method)           { m_meet_me_method = meet_me_method; }

  BYTE                               GetNodeType() const                                  { return m_nodeType; }
  void                               SetNodeType(const BYTE nodeType)                     { m_nodeType = nodeType; }

  BYTE                               GetChair() const                                     { return m_chair; }
  void                               SetChair(const BYTE chair)                           { m_chair = chair; }

  const char*                        GetPassword() const                                  { return m_H243_password; }
  void                               SetPassword(const char* password)                    { SAFE_COPY(m_H243_password, password); }

  DWORD                              GetIpAddress() const                                 { return m_ipAddress; }
  void                               SetIpAddress(const DWORD ipAddress)                  { m_ipAddress = ipAddress; }

  WORD                               GetCallSignallingPort() const                        { return m_callSignallingPort; }
  void                               SetCallSignallingPort(const WORD callSignallingPort) { m_callSignallingPort = callSignallingPort; }

  BYTE                               GetVideoProtocol() const                             { return m_videoProtocol; }
  void                               SetVideoProtocol(const BYTE videoProtocol)           { m_videoProtocol = videoProtocol; }

  DWORD                              GetVideoRate() const                                 { return m_videoRate; }
  void                               SetVideoRate(const DWORD videoRate)                  { m_videoRate = videoRate; }

  const ACCCdrPhone*                 GetBondingPhoneNumber() const                        { return m_bondingPhoneNumber; }
  void                               SetBondingPhoneNumber(const ACCCdrPhone* other)      { m_bondingPhoneNumber->phone_number = other->phone_number; }

  WORD                               GetIpPartyAliasType() const                          { return m_IpPartyAliasType; }
  void                               SetIpPartyAliasType(const WORD IpPartyAliasType)     { m_IpPartyAliasType = IpPartyAliasType; }

  const char*                        GetIpPartyAlias() const                              { return m_IpPartyAlias; }
  void                               SetIpPartyAlias(const char* name)                    { SAFE_COPY(m_IpPartyAlias, name); }

  WORD                               GetSipPartyAddressType() const                       { return m_sipPartyAddressType; }
  void                               SetSipPartyAddressType(const WORD partyAddressType)  { m_sipPartyAddressType = partyAddressType; }

  const char*                        GetSipPartyAddress() const                           { return m_sipPartyAddress; }
  void                               SetSipPartyAddress(const char* name)                 { SAFE_COPY(m_sipPartyAddress, name); }

  BYTE                               GetAudioVolume() const                               { return m_audioVolume; }
  void                               SetAudioVolume(const BYTE audioVolume)               { m_audioVolume = audioVolume; }

  BYTE                               GetUndefinedType() const                             { return m_undefinedType; }
  void                               SetUndefinedType(const BYTE undefinedType)           { m_undefinedType = undefinedType; }
  BYTE                               GetNetInterfaceType() const                          { return m_netInterfaceType; }
  void                               SetNetInterfaceType(const BYTE interfaceType)        { m_netInterfaceType = interfaceType; }

  WORD                               GetNumPartyPhoneNumbers()                            { return m_NumPartyPhoneNumber; }
  void                               SetNumPartyPhoneNumbers(const WORD partyPhoneNumber) { m_NumPartyPhoneNumber = partyPhoneNumber; }

  WORD                               GetNumMcuPhoneNumbers()                              { return m_NumMcuPhoneNumber; }
  void                               SetNumMcuPhoneNumbers(const BYTE mcuPhoneNumber)     { m_NumMcuPhoneNumber = mcuPhoneNumber; }

  WORD                               IsUndefinedParty()                                   { return (m_undefinedType == UNDEFINED_PARTY || m_undefinedType == UNRESERVED_PARTY); }

protected:
  BYTE                               m_connection_type;
  BYTE                               m_bonding_mode;
  BYTE                               m_net_num_of_channel;
  BYTE                               m_net_channel_width;
  char                               m_net_service_name[NET_SERVICE_PROVIDER_NAME_LEN];
  BYTE                               m_restrict;
  BYTE                               m_voice;
  BYTE                               m_num_type;
  char                               m_net_subservice_name[NET_SERVICE_PROVIDER_NAME_LEN];
  BYTE                               m_ident_method;
  BYTE                               m_meet_me_method;
  ACCCdrPhone*                       m_pPartyPhoneNumberList[MAX_CHANNEL_NUMBER];
  ACCCdrPhone*                       m_pMcuPhoneNumberList[MAX_CHANNEL_NUMBER];
  BYTE                               m_NumPartyPhoneNumber;
  BYTE                               m_NumMcuPhoneNumber;
  BYTE                               m_indpartyphone;
  BYTE                               m_indmcuphone;
  WORD                               m_numCallingPhones;
  WORD                               m_numCalledPhones;
  BYTE                               m_netInterfaceType;
  char                               m_H243_password[H243_NAME_LEN];
  BYTE                               m_chair;
  BYTE                               m_videoProtocol;
  DWORD                              m_videoRate;
  BYTE                               m_audioVolume;
  BYTE                               m_undefinedType;
  BYTE                               m_nodeType;
  DWORD                              m_ipAddress;
  WORD                               m_callSignallingPort;
  ACCCdrPhone*                       m_bondingPhoneNumber;
  WORD                               m_IpPartyAliasType;
  char                               m_IpPartyAlias[IP_STRING_LEN];
  WORD                               m_sipPartyAddressType;
  char                               m_sipPartyAddress[IP_STRING_LEN];
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventMoveToConf
////////////////////////////////////////////////////////////////////////////
class ACCCDREventMoveToConf
{
public:
                                     ACCCDREventMoveToConf();
                                     ACCCDREventMoveToConf(const ACCCDREventMoveToConf& other);
                                    ~ACCCDREventMoveToConf();

  ACCCDREventMoveToConf&             operator=(const ACCCDREventMoveToConf& other);

  const char*                        GetOperatorName() const                              { return m_operator_name; }
  void                               SetOperatorName(const char* name)                    { SAFE_COPY(m_operator_name, name); }

  const char*                        GetSourceConfName() const                            { return m_source_conf_name; }
  void                               SetSourceConfName(const char* name)                  { SAFE_COPY(m_source_conf_name, name); }

  DWORD                        GetSourceConfId() const                              { return m_source_conf_id; }
  void                               SetSourceConfId(const DWORD source_conf_id)          { m_source_conf_id = source_conf_id; }

  ACCCDREventAddPartyDetailed*       GetpAddPartyDetailed() const                         { return m_pAddPartyDetailed; }

protected:
  char                               m_operator_name[OPERATOR_NAME_LEN];
  char                               m_party_name[H243_NAME_LEN];
  DWORD                              m_partyId;
  char                               m_source_conf_name[H243_NAME_LEN];
  DWORD                              m_source_conf_id;
  ACCCDREventAddPartyDetailed*       m_pAddPartyDetailed;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventNetChannelConnect
////////////////////////////////////////////////////////////////////////////
class ACCCDREventNetChannelConnect
{
public:
                                     ACCCDREventNetChannelConnect();
                                     ACCCDREventNetChannelConnect(const ACCCDREventNetChannelConnect& other);
  virtual                           ~ACCCDREventNetChannelConnect();

  ACCCDREventNetChannelConnect&      operator= (const ACCCDREventNetChannelConnect& other);
  bool                               operator==(const ACCCDREventNetChannelConnect& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  BYTE                         GetChanlId() const;
  BYTE                         GetChanlNum() const;
  BYTE                         GetConctIniator() const;
  DWORD                        GetCallType() const;
  BYTE                         GetNetSpec() const;
  BYTE                         GetPrfMode() const;
  const ACCCallingParty*             GetCallingParty() const;
  const ACCCalledParty*              GetCalledParty() const;

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  BYTE                               m_channel_Id;
  BYTE                               m_channel_num;
  BYTE                               m_connect_initiator;
  DWORD                              m_call_type;
  BYTE                               m_net_specific;
  BYTE                               m_prf_mode;
  ACCCallingParty                    m_calling_party;
  ACCCalledParty                     m_called_party;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventMPIChannelConnect
////////////////////////////////////////////////////////////////////////////
class ACCCDREventMPIChannelConnect
{
public:
                                     ACCCDREventMPIChannelConnect();
                                     ACCCDREventMPIChannelConnect(const ACCCDREventMPIChannelConnect& other);
  virtual                           ~ACCCDREventMPIChannelConnect();

  ACCCDREventMPIChannelConnect&      operator=(const ACCCDREventMPIChannelConnect& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  BYTE                         GetChanlId() const;
  BYTE                         GetConctIniator() const;
  DWORD                        GetResrict() const;
  BYTE                         GetChanlNum() const;
  const ACCCalledParty*              GetCalledParty() const;

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  BYTE                               m_channel_num;
  BYTE                               m_channel_Id;
  BYTE                               m_connect_initiator;
  DWORD                              m_resrict;
  ACCCalledParty                     m_called_party;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventATMChannelConnect
////////////////////////////////////////////////////////////////////////////
class ACCCDREventATMChannelConnect
{
public:
                                     ACCCDREventATMChannelConnect();
                                     ACCCDREventATMChannelConnect(const ACCCDREventATMChannelConnect& other);
  virtual                           ~ACCCDREventATMChannelConnect();

  ACCCDREventATMChannelConnect&      operator=(const ACCCDREventATMChannelConnect& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  BYTE                         GetChanlId() const;
  BYTE                         GetChanlNum() const;
  BYTE                         GetConctIniator() const;
  DWORD                        GetResrict() const;
  const char*                        GetATMaddress() const;
  const ACCCalledParty*              GetCalledParty() const;

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  BYTE                               m_channel_Id;
  BYTE                               m_channel_num;
  BYTE                               m_connect_initiator;
  DWORD                              m_resrict;
  char                               m_atm_address[20];
  ACCCalledParty                     m_called_party;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREvent323ChannelConnect
////////////////////////////////////////////////////////////////////////////
class ACCCDREvent323ChannelConnect
{
public:
                                     ACCCDREvent323ChannelConnect();
                                     ACCCDREvent323ChannelConnect(const ACCCDREvent323ChannelConnect& other);
  virtual                           ~ACCCDREvent323ChannelConnect();

  ACCCDREvent323ChannelConnect&      operator=(const ACCCDREvent323ChannelConnect& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  BYTE                         GetConctIniator() const;
  DWORD                              GetH323MaxRate()          { return m_IpmaxRate;     }
  DWORD                              GetH323MinRate()          { return m_IpminRate;     }
  BYTE                               GetEndPointType()         { return m_IpendpointType;  }
  char*                              GetH323srcPartyAddress()  { return m_IpsrcPartyAddress; }
  char*                              GetH323destPartyAddress() { return m_IpdestPartyAddress;}

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  BYTE                               m_connect_initiator;
  char                               m_IpsrcPartyAddress[IP_LIMIT_ADDRESS_CHAR_LEN];
  char                               m_IpdestPartyAddress[IP_LIMIT_ADDRESS_CHAR_LEN];
  DWORD                              m_IpmaxRate;
  DWORD                              m_IpminRate;
  BYTE                               m_IpendpointType;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventDisconnectCause
////////////////////////////////////////////////////////////////////////////
class ACCCDREventDisconnectCause
{
public:
                                     ACCCDREventDisconnectCause();
                                     ACCCDREventDisconnectCause(const ACCCDREventDisconnectCause& other);
  virtual                           ~ACCCDREventDisconnectCause();

  BYTE                         GetDisconctCauseCodeStandrd() const;
  BYTE                         GetDisconctCauseLocation() const;
  BYTE                         GetDisconctCauseValue() const;
  const char*                        NameOf() const;
  void                               SetDisconctCauseCodeStandrd(const BYTE code_stndrd);
  void                               SetDisconctCauseLocation(const BYTE location);
  void                               SetDisconctCauseValue(const BYTE cause_value);

  char*                              Serialize(WORD format);
  void                               Serialize(WORD format, std::ostream& m_ostr);
  void                               Serialize(WORD format, std::ostream& m_ostr, BYTE fullformat);
  void                               DeSerialize(WORD format, std::istream& m_istr);
  void                               SerializeXml(CXMLDOMElement* pFatherNode);
  int                                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

protected:
  BYTE                               m_coding_stndrd;
  BYTE                               m_location;
  BYTE                               m_cause_value;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventNetChannelDisconnect
////////////////////////////////////////////////////////////////////////////
class ACCCDREventNetChannelDisconnect
{
public:
                                     ACCCDREventNetChannelDisconnect();
                                     ACCCDREventNetChannelDisconnect(const ACCCDREventNetChannelDisconnect& other);
  virtual                           ~ACCCDREventNetChannelDisconnect();

  ACCCDREventNetChannelDisconnect&   operator=(const ACCCDREventNetChannelDisconnect& other);

  const char*                        GetNetDiscoPartyName() const;
  DWORD                        GetNetDiscoPartyId() const;
  BYTE                         GetNetDiscoChannelId() const;
  BYTE                         GetNetDiscoInitiator() const;
  const ACCCDREventDisconnectCause*  GetCDisconctCause() const;

protected:
  char                               m_partyName[H243_NAME_LEN];
  DWORD                              m_partId;
  BYTE                               m_channelId;
  BYTE                               m_disco_Init;
  ACCCDREventDisconnectCause         m_q931_cause;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventPartyAddBillingCode
////////////////////////////////////////////////////////////////////////////
class ACCCDREventPartyAddBillingCode
{
public:
                                     ACCCDREventPartyAddBillingCode();
                                     ACCCDREventPartyAddBillingCode(const ACCCDREventPartyAddBillingCode& other);
  virtual                           ~ACCCDREventPartyAddBillingCode();

  ACCCDREventPartyAddBillingCode&    operator=(const ACCCDREventPartyAddBillingCode& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  const char*                        GetBillingData() const;

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  char                               m_billing_data[32]; // temp
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventSetVisualName
////////////////////////////////////////////////////////////////////////////
class ACCCDREventSetVisualName
{
public:
                                     ACCCDREventSetVisualName()                         { memset(this, 0, sizeof(*this)); m_party_Id = 0xffffffff; }
                                     ACCCDREventSetVisualName(const ACCCDREventSetVisualName& other) { *this = other; }

  ACCCDREventSetVisualName&          operator=(const ACCCDREventSetVisualName& other)   { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetPartyName() const                               { return m_h243party_name; }
  void                               SetPartyName(const char* name)                     { SAFE_COPY(m_h243party_name, name); }

  DWORD                        GetPartyId() const                                 { return m_party_Id; }
  void                               SetPartyId(const DWORD partyid)                    { m_party_Id = partyid; }

  const char*                        GetVisualName() const                              { return m_visual_name; }
  void                               SetVisualName(const char* name)                    { SAFE_COPY(m_visual_name, name); }

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  char                               m_visual_name[H243_NAME_LEN]; // temp
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventMoveToConfCont1
////////////////////////////////////////////////////////////////////////////
class ACCCDREventMoveToConfCont1
{
public:
                                     ACCCDREventMoveToConfCont1()                       { memset(this, 0, sizeof(*this)); m_party_Id = 0xffffffff; }
                                     ACCCDREventMoveToConfCont1(const ACCCDREventMoveToConfCont1& other) { *this = other; }

  ACCCDREventMoveToConfCont1&        operator=(const ACCCDREventMoveToConfCont1& other) { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetPartyName() const                               { return m_h243party_name; }
  void                               SetPartyName(const char* name)                     { SAFE_COPY(m_h243party_name, name); }

  DWORD                        GetPartyId() const                                 { return m_party_Id; }
  void                               SetPartyId(const DWORD partyid)                    { m_party_Id = partyid; }

  const char*                        GetPartyCallingNum() const                         { return m_partyCallingNum; }
  void                               SetPartyCallingNum(const char* name)               { SAFE_COPY(m_partyCallingNum, name); }

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  char                               m_partyCallingNum[H243_NAME_LEN]; // temp
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventConfStartCont4
////////////////////////////////////////////////////////////////////////////
class ACCCDREventConfStartCont4
{
public:
                                     ACCCDREventConfStartCont4();
                                     ACCCDREventConfStartCont4(const ACCCDREventConfStartCont4& other);
  virtual                           ~ACCCDREventConfStartCont4();

  ACCCDREventConfStartCont4&         operator=(const ACCCDREventConfStartCont4& other);

  const char*                        GetContactInfo(int ContactNumber)  const;
  const char*                        GetConfBillingInfo()  const;
  const char*                        GetNumericConfId() const;
  const char*                        GetUser_password() const;
  const char*                        GetChair_password() const;

protected:
  char                               m_NumericConfId[NUMERIC_CONFERENCE_ID_LEN];
  char                               m_user_password[CONFERENCE_ENTRY_PASSWORD_LEN];
  char                               m_chair_password[CONFERENCE_ENTRY_PASSWORD_LEN];
  char                               m_ConfBillingInfo[CONF_BILLING_INFO_LEN];
  char                               m_ContactInfo[MAX_CONF_INFO_ITEMS][CONF_INFO_ITEM_LEN];
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventConfStartCont5
////////////////////////////////////////////////////////////////////////////
class ACCCDREventConfStartCont5
{
public:
                                     ACCCDREventConfStartCont5();
                                     ACCCDREventConfStartCont5(const ACCCDREventConfStartCont5& other);
  virtual                           ~ACCCDREventConfStartCont5();

  ACCCDREventConfStartCont5&         operator=(const ACCCDREventConfStartCont5& other);

  BYTE                         GetIsEncryptedConf()  const;

protected:
  BYTE                               m_encryption;  // YES, NO
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventConfStartCont10
////////////////////////////////////////////////////////////////////////////
class ACCCDREventConfStartCont10
{
public:
                                     ACCCDREventConfStartCont10();
                                     ACCCDREventConfStartCont10(const ACCCDREventConfStartCont10& other);
  virtual                           ~ACCCDREventConfStartCont10();

  ACCCDREventConfStartCont10&        operator=(const ACCCDREventConfStartCont10& other);

  const char*                       GetConfDisplayName()  const;
  BYTE								GetAvcSvc() const;

protected:
	  char							m_confDisplayName[H243_NAME_LEN]; // conferences UTF-8 name
	  BYTE							m_Avc_Svc;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventSetUserDefinedInfo
////////////////////////////////////////////////////////////////////////////
class ACCCDREventSetUserDefinedInfo
{
public:
                                     ACCCDREventSetUserDefinedInfo()                           { memset(this, 0, sizeof(*this)); }
                                     ACCCDREventSetUserDefinedInfo(const ACCCDREventSetUserDefinedInfo& other) { *this = other; }

  ACCCDREventSetUserDefinedInfo&     operator=(const ACCCDREventSetUserDefinedInfo& other)     { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetUserDefinedInfo(int number) const                      { return (number >= 0 && number < MAX_USER_INFO_ITEMS) ? m_UserDefinedInfo[number] : NULL; }
  void                               SetUserDefinedInfo(const char* info, int number)          { if (number >= 0 && number < MAX_USER_INFO_ITEMS) SAFE_COPY(m_UserDefinedInfo[number], info); }

  BYTE                               GetIsVip()                                                { return m_isVip; }
  void                               SetIsVip(BYTE isVip)                                      { m_isVip = isVip; }

protected:
  char                               m_UserDefinedInfo[MAX_USER_INFO_ITEMS][USER_INFO_ITEM_LEN];
  BYTE                               m_isVip; // YES/NO
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDRPartyDTMFfailureIndication
////////////////////////////////////////////////////////////////////////////
class ACCCDRPartyDTMFfailureIndication
{
public:
                                     ACCCDRPartyDTMFfailureIndication()                        { memset(this, 0, sizeof(*this)); m_party_Id = 0xffffffff; }
                                     ACCCDRPartyDTMFfailureIndication(const ACCCDRPartyDTMFfailureIndication& other) { *this = other; }

  ACCCDRPartyDTMFfailureIndication&  operator=(const ACCCDRPartyDTMFfailureIndication& other)  { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetPartyName() const                                      { return m_h243party_name; }
  void                               SetPartyName(const char* name)                            { SAFE_COPY(m_h243party_name, name); }

  DWORD                        GetPartyId() const                                        { return m_party_Id; }
  void                               SetPartyId(const DWORD partyid)                           { m_party_Id = partyid; }

  const char*                        GetDTMFcode() const                                       { return m_DTMFcode; }
  void                               SetDTMFcode(const char* name)                             { SAFE_COPY(m_DTMFcode, name); }

  const char*                        GetRightData() const                                      { return m_RightData; }
  void                               SetRightData(const char* name)                            { SAFE_COPY(m_RightData, name); }

  DWORD                        GetFailureStatus() const                                  { return m_FailureState; }
  void                               SetFailureStatus(const DWORD failureState)                { m_FailureState = failureState; }

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  char                               m_DTMFcode[MAX_DTMF_STRING_LENGTH]; // temp
  char                               m_RightData[MAX_DTMF_STRING_LENGTH]; // temp
  DWORD                              m_FailureState;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventMoveToConfCont2
////////////////////////////////////////////////////////////////////////////
class ACCCDREventMoveToConfCont2
{
public:
                                     ACCCDREventMoveToConfCont2()                              { memset(this, 0, sizeof(*this)); m_party_Id = 0xffffffff; }
                                     ACCCDREventMoveToConfCont2(const ACCCDREventMoveToConfCont2& other) { *this = other; }

  ACCCDREventMoveToConfCont2&        operator=(const ACCCDREventMoveToConfCont2& other)        { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetPartyName() const                                      { return m_h243party_name; }
  void                               SetPartyName(const char* name)                            { SAFE_COPY(m_h243party_name, name); }

  DWORD                        GetPartyId() const                                        { return m_party_Id; }
  void                               SetPartyId(const DWORD partyid)                           { m_party_Id = partyid; }

  const char*                        GetPartyCalledNum() const                                 { return m_partyCalledNum; }
  void                               SetPartyCalledNum(const char* name)                       { SAFE_COPY(m_partyCalledNum, name); }

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  char                               m_partyCalledNum[H243_NAME_LEN]; // temp
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDRPartyRecording
////////////////////////////////////////////////////////////////////////////
class ACCCDRPartyRecording
{
public:
                                     ACCCDRPartyRecording()                                    { memset(this, 0, sizeof(*this)); m_party_Id = 0xffffffff; }
                                     ACCCDRPartyRecording(const ACCCDRPartyRecording& other)   { *this = other; }

  ACCCDRPartyRecording&              operator=(const ACCCDRPartyRecording& other)              { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetPartyName() const                                      { return m_h243party_name; }
  void                               SetPartyName(const char* name)                            { SAFE_COPY(m_h243party_name, name); }

  DWORD                        GetPartyId() const                                        { return m_party_Id; }
  void                               SetPartyId(const DWORD partyid)                           { m_party_Id = partyid; }

  WORD                         GetOpType() const                                         { return m_opType; }
  void                               SetOpType(const WORD opType)                              { m_opType = opType; }

  WORD                         GetOpInitiator() const                                    { return m_opInitiator; }
  void                               SetOpInitiator(const WORD opInitiator)                    { m_opInitiator = opInitiator; }

  const char*                        GetRecordingLinkName() const                              { return m_recordingLinkName; }
  void                               SetRecordingLinkName(const char* name)                    { SAFE_COPY(m_recordingLinkName, name); }

  DWORD                        GetRecordingLinkId() const                                { return m_recordingLinkId; }
  void                               SetRecordingLinkId(const DWORD linkId)                    { m_recordingLinkId = linkId; }

  WORD                         GetStartRecPolicy() const                                 { return m_recordingPolicy; }
  void                               SetStartRecPolicy(const WORD startRecPolicy)              { m_recordingPolicy = startRecPolicy; }

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  WORD                               m_opType;                           // start recording\ Stop recording\ pause recording \resume recording
  WORD                               m_opInitiator;                      // Party \operator \auto  |not implemented in the XML !!!!
  char                               m_recordingLinkName[H243_NAME_LEN];
  DWORD                              m_recordingLinkId;
  WORD                               m_recordingPolicy;                  // Start immediately\ Upon request
};

///////////////////////////////////////////////////////////////////
class CCDRPartyCallInfo
{
	public:
	//Constructors
	CCDRPartyCallInfo();
	CCDRPartyCallInfo(const CCDRPartyCallInfo& other);

	virtual ~CCDRPartyCallInfo();
	CCDRPartyCallInfo& operator = (const CCDRPartyCallInfo& other);
	bool operator == (const CCDRPartyCallInfo& other);

	// Implementation
	const char  *GetPartyName()	const;
	DWORD  GetPartyId() const;
	DWORD  GetMaxBitRate() const;
	const char  *GetMaxResolution() const;
	const char  *GetMaxFrameRate() const;
	//const WORD   GetMaxFrameRate() const;
	const char  *GetAddress()	const;

	protected:
	char   m_partyName[H243_NAME_LEN];
	DWORD  m_partyId;
	DWORD  m_maxBitRate;
	char   m_maxResolution[RESOLUTION_LEN];
	char   m_maxFrameRate[FRAME_RATE_LEN];
	//WORD m_maxFrameRate;
	char   m_IPaddress[IPADDRESS_LEN];
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDRPartySystemRecording
////////////////////////////////////////////////////////////////////////////
class ACCCDRPartySystemRecording
{
public:
                                     ACCCDRPartySystemRecording()                              { memset(this, 0, sizeof(*this)); m_party_Id = 0xffffffff; }
                                     ACCCDRPartySystemRecording(const ACCCDRPartySystemRecording& other) { *this = other; }

  ACCCDRPartySystemRecording&        operator=(const ACCCDRPartySystemRecording& other)        { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetPartyName() const                                      { return m_h243party_name; }
  void                               SetPartyName(const char* name)                            { SAFE_COPY(m_h243party_name, name); }

  DWORD                        GetPartyId() const                                        { return m_party_Id; }
  void                               SetPartyId(const DWORD partyid)                           { m_party_Id = partyid; }

  WORD                         GetOpType() const                                         { return m_opType; }
  void                               SetOpType(const WORD opType)                              { m_opType = opType; }

  WORD                         GetPartyType() const                                      { return m_partyType; }
  void                               SetPartyType(const WORD partyType)                        { m_partyType = partyType; }

  const char*                        GetUserAccount() const                                    { return m_userAccount; }
  void                               SetUserAccount(const char* name)                          { SAFE_COPY(m_userAccount, name); }

  WORD                         GetFailed() const                                         { return m_failed; }
  void                               SetFailed(const WORD failed)                              { m_failed = failed; }

  WORD                         GetFailedCode() const                                     { return m_failedCode; }
  void                               SetFailedCode(const WORD failedCode)                      { m_failedCode = failedCode; }

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  WORD                               m_opType;       // start recording\ Stop recording\ pause recording \resume recording
  WORD                               m_partyType;    // Recording or Playback
  char                               m_userAccount[MAX_USER_ACCOUNT_LEN];
  WORD                               m_failed;       // 0/1
  WORD                               m_failedCode;   // error code in case of failure
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDRSipPrivateExtensions
////////////////////////////////////////////////////////////////////////////
class ACCCDRSipPrivateExtensions
{
public:
                                     ACCCDRSipPrivateExtensions()                              { memset(this, 0, sizeof(*this)); m_partyId = 0xffffffff; }
                                     ACCCDRSipPrivateExtensions(const ACCCDRSipPrivateExtensions& other) { *this = other; }

  ACCCDRSipPrivateExtensions&        operator=(const ACCCDRSipPrivateExtensions& other)        { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetPartyName() const                                      { return m_party_name; }
  void                               SetPartyName(const char* name)                            { SAFE_COPY(m_party_name, name); }

  DWORD                        GetPartyId() const                                        { return m_partyId; }
  void                               SetPartyId(const DWORD partyId)                           { m_partyId = partyId; }

  const char*                        GetPCalledPartyId() const                                 { return m_calledPartyId; }
  void                               SetPCalledPartyId(const char* name)                       { SAFE_COPY(m_calledPartyId, name); }

  const char*                        GetPAssertedIdentity() const                              { return m_assertedIdentity; }
  void                               SetPAssertedIdentity(const char* name)                    { SAFE_COPY(m_assertedIdentity, name); }

  const char*                        GetPChargingVector() const                                { return m_chargingVector; }
  void                               SetPChargingVector(const char* name)                      { SAFE_COPY(m_chargingVector, name); }

  const char*                        GetPPreferredIdentity() const                             { return m_preferredIdentity; }
  void                               SetPPreferredIdentity(const char* name)                   { SAFE_COPY(m_preferredIdentity, name); }

protected:
  char                               m_party_name[H243_NAME_LEN];
  DWORD                              m_partyId;
  char                               m_calledPartyId[MAX_SIP_PRIVATE_EXT_LEN];
  char                               m_assertedIdentity[MAX_SIP_PRIVATE_EXT_LEN];
  char                               m_chargingVector[MAX_SIP_PRIVATE_EXT_LEN];
  char                               m_preferredIdentity[MAX_SIP_PRIVATE_EXT_LEN];
};


////////////////////////////////////////////////////////////////////////////
//                        CCDRPartyGkInfo
////////////////////////////////////////////////////////////////////////////
class CCDRPartyGkInfo
{
public:
                                     CCDRPartyGkInfo();
                                     CCDRPartyGkInfo(const CCDRPartyGkInfo& other);
  virtual                           ~CCDRPartyGkInfo();

  CCDRPartyGkInfo&                   operator= (const CCDRPartyGkInfo& other);
  bool                               operator==(const CCDRPartyGkInfo& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  const BYTE*                        GetGkCallId() const;

protected:
  char                               m_partyName[H243_NAME_LEN];
  DWORD                              m_partyId;
  BYTE                               m_gkCallId[SIZE_OF_CALL_ID];
};


////////////////////////////////////////////////////////////////////////////
//                        CCDRPartyNewRateInfo
////////////////////////////////////////////////////////////////////////////
class CCDRPartyNewRateInfo
{
public:
                                     CCDRPartyNewRateInfo();
                                     CCDRPartyNewRateInfo(const CCDRPartyNewRateInfo& other);
  virtual                           ~CCDRPartyNewRateInfo();

  CCDRPartyNewRateInfo&              operator= (const CCDRPartyNewRateInfo& other);
  bool                               operator==(const CCDRPartyNewRateInfo& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  DWORD                        GetCurrentRate() const;

protected:
  char                               m_partyName[H243_NAME_LEN];
  DWORD                              m_partyId;
  DWORD                              m_partyCurrentRate;
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDRCOperIpV6PartyCont1
////////////////////////////////////////////////////////////////////////////
class ACCCDRCOperIpV6PartyCont1
{
public:
                                     ACCCDRCOperIpV6PartyCont1()                               { memset(this, 0, sizeof(*this)); }
                                     ACCCDRCOperIpV6PartyCont1(const ACCCDRCOperIpV6PartyCont1& other) { memcpy(this, &other, sizeof(*this)); }

  ACCCDRCOperIpV6PartyCont1&         operator=(const ACCCDRCOperIpV6PartyCont1& other)         { memcpy(this, &other, sizeof(*this)); return *this; }

  const char*                        GetIpV6Party() const                                      { return m_ipV6Address; }
  void                               SetIpV6Address(char* name)                                { SAFE_COPY(m_ipV6Address, name); }

protected:
  char                               m_ipV6Address[64]; // relevant only for 323party.
};


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventPartyChairPerson
////////////////////////////////////////////////////////////////////////////
class ACCCDREventPartyChairPerson

{
public:
                                     ACCCDREventPartyChairPerson();
                                     ACCCDREventPartyChairPerson(const ACCCDREventPartyChairPerson& other);
  virtual                           ~ACCCDREventPartyChairPerson();

  ACCCDREventPartyChairPerson&       operator =(const ACCCDREventPartyChairPerson& other);

  const char*                        GetPartyName() const;
  DWORD                        GetPartyId() const;
  bool                         IsChairPerson() const;

protected:
  char                               m_h243party_name[H243_NAME_LEN];
  DWORD                              m_party_Id;
  bool                               m_bChair;
};

////////////////////////////////////////////////////////////////////////////
//                        CCDRPartyCorrelationDataInfo
////////////////////////////////////////////////////////////////////////////

class CCDRPartyCorrelationDataInfo
{
public:
	CCDRPartyCorrelationDataInfo();
	CCDRPartyCorrelationDataInfo(const CCDRPartyCorrelationDataInfo& other);
	virtual                           ~CCDRPartyCorrelationDataInfo();

	CCDRPartyCorrelationDataInfo&       operator =(const CCDRPartyCorrelationDataInfo& other);
	bool operator == (const CCDRPartyCorrelationDataInfo& other);
	const char*                         GetPartyName() const;
	DWORD                               GetPartyId() const;
	const char*                         GetSigUid() const;

protected:
	DWORD                              m_party_Id;
	char                               m_sig_uuid[CORRELATION_SIG_UUID_LEN]; //correlation info
	char                               m_party_name[H243_NAME_LEN];
};
///////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//                        CCDRConfCorrelationDataInfo
////////////////////////////////////////////////////////////////////////////

class CCDRConfCorrelationDataInfo
{
public:
	CCDRConfCorrelationDataInfo();
	CCDRConfCorrelationDataInfo(const CCDRConfCorrelationDataInfo& other);
	virtual                           ~CCDRConfCorrelationDataInfo();

	CCDRConfCorrelationDataInfo&       operator =(const CCDRConfCorrelationDataInfo& other);
	bool operator == (const CCDRConfCorrelationDataInfo& other);
	std::string                         GetSigUid() const;
	const char*				            GetDisplayName() const;
	const char*         				GetFileName() const;

protected:
	std::string                        m_conf_uuid; //correlation info

};
///////////////////////////////////////////////////////////////////////////
struct SvcStreamDesc
{
	unsigned int m_bitRate;
	unsigned int m_frameRate;
	unsigned int m_height;
	unsigned int m_width;
};

#endif // ifndef _CDREVENTS
