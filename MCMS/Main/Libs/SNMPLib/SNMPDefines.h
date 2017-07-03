// SNMPDefines.h

#ifndef SNMPDEFINES_H_
#define SNMPDEFINES_H_

#include <map>
#include <string>
#include <vector>

#include "Macros.h"
#include "PObject.h"
#include "Segment.h"
#include "DataTypes.h"

#define DEFAULT_TRAP_COM_NAME "Trap_community"
#define DEFAULT_LOCATION      "Default Location"
#define DEFAULT_CONTACT       "Default Contact"
#define DEFAULT_SYS_NAME      "Default System Name"

#define SNMP_NO_PERMISSION   0
#define SNMP_READ_ONLY       1
#define SNMP_READ_WRITE      2
#define SNMP_WRITE_CREATE    3

#define MAX_NUM_TRAP_DESTINATIONS 10
#define MAX_NUM_REQUEST_SOURCES   10
#define MAX_NUM_COMMUNITIES       10

#define SNMP_REQ_TOUT             2
#define SNMP_PORT 161

#define SNMPD_WD_TIME_INTERVAL 1000

#define AGENTX_PIPE_NAME ((std::string)(MCU_TMP_DIR+"/queue/agentX"))
// TODO: combine eSnmpVersionTrap and eSnmpVersion enumerators
enum eSnmpVersionTrap
{
  eSnmpVer1Trap             = 1,
  eSnmpVer2Trap             = 2,
  eSnmpVer3Trap             = 3,
  NUM_OF_SNMP_VERSION_TRAP  = 3   // DONT FORGET TO UPDATE THIS
};

enum eSnmpVersion
{
  eSnmpVer1             = eSnmpVer1Trap,
  eSnmpVer2             = eSnmpVer2Trap,
  eSnmpVer3             = eSnmpVer3Trap,
  NUM_OF_SNMP_VERSIONS  = NUM_OF_SNMP_VERSION_TRAP  // DONT FORGET TO UPDATE THIS
};

enum eSnmpAuthProtocol
{
  eSapNone = 0,
  eSapMD5  = 1,
  eSapSHA  = 2
};

enum eSnmpPrivProtocol
{
  eSppNone = 0,
  eSppDES  = 1,
  eSppAES  = 2
};

enum eSnmpSecurityLevel
{
  eSslNone   = 0,
  eSslNoAuth = 1,
  eSslAuth   = 2,
  eSslPriv   = 3
};

typedef struct
{
  DWORD IP_Num;
} SNMP_IP_PARAMS_S;

// If change update TelemetaryTypeToStr.
enum eTelemetryType
{
  eTT_Unknown = 0,

  eTT_IdentitySoftwareInfo,
  eTT_IdentityBuildDate,
  eTT_IdentityDeviceType,
  eTT_IdentityConsoleAccess,
  eTT_MCUDebug,
  eTT_NetworkSeparation,
  eTT_UltraSecureMode,
  eTT_MCUDisplayName,
  eTT_IncomingCallsReqrGK,
  eTT_OutgoingCallsReqrGK,
  eTT_PalNtsc,
  eTT_RRQFirst,

  eTT_HDBitrateThrshld,
  eTT_MaxCPRstln,
  eTT_MaxCPRstlnCfg,
  eTT_NumPorts,
  eTT_NumVideoPorts,
  eTT_NumVoicePorts,
  eTT_NumVideoPortsUsed,
  eTT_NumVoicePortsUsed,
  eTT_NumVideoPortsUsedPercentage,
  eTT_NumVoicePortsUsedPercentage,
  eTT_NumPortsUsed,
  eTT_RsrcAllocMode,
  
  eTT_H323Status,
  eTT_SIPStatus,
  eTT_ISDNStatus,
  eTT_VideoParticipants,
  eTT_VoiceParticipants,

  eTT_SuccessfulNewCalls,
  eTT_FailedNewCalls,
  eTT_SuccessfulEndCalls,
  eTT_FailedEndCalls,

  eTT_NewCalls,
  eTT_EndCalls,
  eTT_RatioSuccessfulNewCalls,
  eTT_RatioSuccessfulEndCalls,
  eTT_RatioPortsUsed,
  
  eTT_ActiveConf,
  eTT_ActiveParticipant,
  eTT_SystemStatus,

  eTT_HardwareOverallStatus,
  eTT_HardwareFanStatus,
  eTT_HardwarePowerSupplyStatus,
  eTT_HardwareChassisTempStatus,
  eTT_HardwareIntegratedBoardStatus,

  eTT_Max
};

// Allows iterations on the enum, as ++i
inline eTelemetryType& operator++(eTelemetryType& type)
{
  return enum_increment(type, eTT_Unknown, eTT_Max);
}

// If change update TelemetaryTypeToStr
enum eTelemetryAction
{
  eTA_Unknown = 0,

  eTA_Increment,
  eTA_Decrement,
  eTA_Assign
};

// If change update TelemetryTypeBasicToStr
enum eTelemetryTypeBasic
{
  eTTB_Unknown = 0,

  eTTB_Integer,
  eTTB_Bool,
  eTTB_String,
  eTTB_DateAndTime
};

const char* TelemetaryTypeToStr(eTelemetryType type);
const char* TelemetryActionToStr(eTelemetryAction action);
const char* TelemetryTypeBasicToStr(eTelemetryTypeBasic type);

class CTelemetryValueImpl;

class CTelemetryValue : public CPObject
{
  friend class CTelemetryTraits;
  CLASS_TYPE_1(CTelemetryValue, CPObject);
  
 public:
  CTelemetryValue();
  CTelemetryValue(eTelemetryType type, unsigned char val);
  CTelemetryValue(eTelemetryType type, unsigned int val);
  CTelemetryValue(eTelemetryType type, const char* val);
  CTelemetryValue(eTelemetryType type, const std::vector<unsigned char>& val);
  explicit CTelemetryValue(CSegment& seg);
  CTelemetryValue(const CTelemetryValue& rhs);
  ~CTelemetryValue();

  CTelemetryValue& operator=(const CTelemetryValue& rhs);
  CTelemetryValue& Update(const CTelemetryValue& rhs);

  bool               IsEmpty() const;
  unsigned char*     GetPtr(size_t& len) const;
  CSegment           GetSegment() const;
  eTelemetryType     GetType() const;

 private:
  virtual const char* NameOf() const;
  virtual void        Dump(std::ostream& out) const;

  CTelemetryValueImpl* m_impl;
  eTelemetryType       m_type;
};

class CTelemetryTraits
{
  friend class CIndexer;

 public:
  static const CTelemetryTraits& Get(eTelemetryType type);

  const eTelemetryAction    action;
  const eTelemetryTypeBasic btype;
  const bool                is_prolonged;
  const bool                is_calculated;
  const CSegment            dvalue;

 private:
  CTelemetryTraits() :
    action(eTA_Unknown),
    btype(eTTB_Unknown),
    is_prolonged(false),
    is_calculated(false)
  {}

  CTelemetryTraits(eTelemetryAction    a,
                   eTelemetryTypeBasic b,
                   bool                p,
                   bool                c,
                   CSegment&           d) :
    action(a), btype(b), is_prolonged(p), is_calculated(c), dvalue(d)
  {}

  static const class CIndexer : CNonCopyable
  {
   public:
    friend class CTelemetryTraits;
    CIndexer();
    const CTelemetryTraits&   Find(eTelemetryType type) const;
    template<typename T> void Add(eTelemetryType      type,
                                  eTelemetryAction    action,
                                  eTelemetryTypeBasic btype,
                                  bool                prolonged,
                                  bool                calculated,
                                  T                   value);
   private:
    std::map<eTelemetryType, CTelemetryTraits> m_db;
  } s_traits;
};

#endif  // SNMPDEFINES_H_
