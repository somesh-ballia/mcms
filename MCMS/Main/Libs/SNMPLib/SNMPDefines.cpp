// SNMPDefines.cpp

#include "SNMPDefines.h"

#include <iomanip>
#include <iterator>

#include "Trace.h"
#include "TraceStream.h"

#define CASE(t) case t: return #t

const char* TelemetaryTypeToStr(eTelemetryType type)
{
  switch (type)
  {
    CASE(eTT_Unknown);

    CASE(eTT_IdentitySoftwareInfo);
    CASE(eTT_IdentityBuildDate);
    CASE(eTT_IdentityDeviceType);
    CASE(eTT_IdentityConsoleAccess);
    CASE(eTT_MCUDebug);
    CASE(eTT_NetworkSeparation);
    CASE(eTT_UltraSecureMode);
    CASE(eTT_MCUDisplayName);
    CASE(eTT_IncomingCallsReqrGK);
    CASE(eTT_OutgoingCallsReqrGK);
    CASE(eTT_PalNtsc);
    CASE(eTT_RRQFirst);

    CASE(eTT_HDBitrateThrshld);
    CASE(eTT_MaxCPRstln);
    CASE(eTT_MaxCPRstlnCfg);
    CASE(eTT_NumPorts);
    CASE(eTT_NumVideoPorts);
    CASE(eTT_NumVoicePorts);
    CASE(eTT_NumVideoPortsUsed);
    CASE(eTT_NumVoicePortsUsed);
    CASE(eTT_NumVideoPortsUsedPercentage);
    CASE(eTT_NumVoicePortsUsedPercentage);
    CASE(eTT_NumPortsUsed);
    CASE(eTT_RsrcAllocMode);

    CASE(eTT_H323Status);
    CASE(eTT_SIPStatus);
    CASE(eTT_ISDNStatus);
    CASE(eTT_VideoParticipants);
    CASE(eTT_VoiceParticipants);

    CASE(eTT_SuccessfulNewCalls);
    CASE(eTT_FailedNewCalls);
    CASE(eTT_SuccessfulEndCalls);
    CASE(eTT_FailedEndCalls);

    CASE(eTT_NewCalls);
    CASE(eTT_EndCalls);
    CASE(eTT_RatioSuccessfulNewCalls);
    CASE(eTT_RatioSuccessfulEndCalls);
    CASE(eTT_RatioPortsUsed);

    CASE(eTT_ActiveConf);
    CASE(eTT_ActiveParticipant);
    CASE(eTT_SystemStatus);

    CASE(eTT_HardwareOverallStatus);
    CASE(eTT_HardwareFanStatus);
    CASE(eTT_HardwarePowerSupplyStatus);
    CASE(eTT_HardwareChassisTempStatus);
    CASE(eTT_HardwareIntegratedBoardStatus);

    CASE(eTT_Max);
    default: FPASSERTSTREAM(true, "Unhandled enumerator value " << type);
  };

  return "";
}

const char* TelemetryActionToStr(eTelemetryAction action)
{
  switch (action)
  {
    CASE(eTA_Unknown);
    CASE(eTA_Increment);
    CASE(eTA_Decrement);
    CASE(eTA_Assign);
    default: FPASSERTSTREAM(true, "Unhandled enumerator value " << action);
  };

  return "";
}

const char* TelemetryTypeBasicToStr(eTelemetryTypeBasic type)
{
  switch (type)
  {
    CASE(eTTB_Unknown);
    CASE(eTTB_Integer);
    CASE(eTTB_Bool);
    CASE(eTTB_String);
    CASE(eTTB_DateAndTime);
    default: FPASSERTSTREAM(true, "Unhandled enumerator value " << type);
  };

  return "";
}

// Anonymous namespace.
namespace
{
  class Integer;
  class Boolean;
  class String;
  class DateAndTime;
}

class CTelemetryValueImpl
{
 public:
  static CTelemetryValueImpl* Make(unsigned int val);
  static CTelemetryValueImpl* Make(unsigned char val);
  static CTelemetryValueImpl* Make(const char* val);
  static CTelemetryValueImpl* Make(const std::vector<unsigned char>& val);
  static void                 Destroy(CTelemetryValueImpl* val) { delete val; }

  virtual CTelemetryValueImpl* Clone() const = 0;
  virtual std::string          ToStr() const = 0;
  virtual void                 ToSeg(CSegment& out) const = 0;
  virtual unsigned char*       GetPtr(size_t& len) const = 0;

  virtual CTelemetryValueImpl& operator+=(const CTelemetryValueImpl& rhs)
  {
    Assert(rhs);
    return *this;
  }
  virtual CTelemetryValueImpl& operator-=(const CTelemetryValueImpl& rhs)
  {
    Assert(rhs);
    return *this;
  }
  virtual CTelemetryValueImpl& operator=(const CTelemetryValueImpl&) = 0;

  // Dispatchers for operators.
  virtual void AddTo(Integer& lhs) const         { Assert(lhs); }
  virtual void DeductTo(Integer& lhs) const      { Assert(lhs); }
  virtual void AssignTo(Integer& lhs) const      { Assert(lhs); }
  virtual void AssignTo(Boolean& lhs) const      { Assert(lhs); }
  virtual void AssignTo(String& lhs) const       { Assert(lhs); }
  virtual void AssignTo(DateAndTime& lhs) const  { Assert(lhs); }

 protected:
  template <typename T>
  void Assert(const T& rhs) const
  {
    FPASSERTSTREAM(true,
      "Illegal action between " << TelemetryTypeBasicToStr(GetType())
      << " and " << TelemetryTypeBasicToStr(rhs.GetType()));
  }

  // It is used only for assert message.
  virtual eTelemetryTypeBasic GetType() const = 0;

  virtual ~CTelemetryValueImpl() {}
};

// Anonymous namespace.
namespace
{

typedef CTelemetryValueImpl Value;

class Integer : public Value
{
  friend class CTelemetryValueImpl;

 protected:
  virtual Value& operator+=(const Value& rhs)
  {
    rhs.AddTo(*this);
    return *this;
  }
  virtual Value& operator-=(const Value& rhs)
  {
    rhs.DeductTo(*this);
    return *this;
  }
  virtual Value& operator=(const Value& rhs)
  {
    rhs.AssignTo(*this);
    return *this;
  }
  virtual void                AddTo(Integer& lhs) const    { lhs.m_val += m_val; }
  virtual void                DeductTo(Integer& lhs) const { lhs.m_val -= m_val; }
  virtual void                AssignTo(Integer& lhs) const { lhs.m_val = m_val; }
  virtual Value*              Clone() const                { return new Integer(m_val); }
  virtual eTelemetryTypeBasic GetType() const              { return eTTB_Integer; }

  virtual std::string ToStr() const
  {
    std::ostringstream os; os << m_val;
    return os.str();
  }
  virtual void ToSeg(CSegment& out) const
  {
    out << static_cast<DWORD>(m_val);
  }
  virtual unsigned char* GetPtr(size_t& len) const
  {
    len = sizeof m_val;
    return (unsigned char*)&m_val;
  }

 private:
  Integer(unsigned int val) : m_val(val) {}
  unsigned long m_val;
};

class Boolean : public Value
{
  friend class CTelemetryValueImpl;

 protected:
  virtual Value& operator=(const Value& rhs)
  {
    rhs.AssignTo(*this);
    return *this;
  }
  virtual void AssignTo(Boolean& lhs) const
  {
    lhs.m_val = m_val;
  }
  virtual eTelemetryTypeBasic GetType() const
  {
    return eTTB_Bool;
  }
  virtual Value* Clone() const
  {
    return new Boolean(2u == m_val ? '\0' : '\1');
  }
  virtual std::string ToStr() const
  {
    std::ostringstream os;
    os<<(2u==m_val ? "false" : "true");
    return os.str();
  }
  virtual void ToSeg(CSegment& out) const
  {
    out << static_cast<BYTE>(2u == m_val ? '\0' : '\1');
  }
  virtual unsigned char* GetPtr(size_t& len) const
  {
    len = sizeof m_val;
    return (unsigned char*)&m_val;
  }

 private:
  Boolean(unsigned char val) : m_val('\0' == val ? 2u : 1u) {}
  unsigned long m_val;
};

class String : public Value
{
  friend class CTelemetryValueImpl;

 protected:
  virtual Value& operator=(const Value& rhs)  { rhs.AssignTo(*this); return *this; }
  virtual void AssignTo(String& lhs) const    { lhs.m_val = m_val; }

  virtual Value*              Clone() const   { return new String(m_val.c_str()); }
  virtual eTelemetryTypeBasic GetType() const { return eTTB_String; }
  virtual std::string         ToStr() const   { return m_val; }

  virtual void ToSeg(CSegment& out) const
  {
    out << m_val;
  }
  virtual unsigned char* GetPtr(size_t& len) const
  {
    len = m_val.length();
    return (unsigned char*)m_val.c_str();
  }

 private:
  String(const char* val) : m_val(val) {}
  std::string m_val;
};

class DateAndTime : public Value
{
  friend class CTelemetryValueImpl;

 protected:
  virtual Value& operator=(const Value& rhs)
  {
    rhs.AssignTo(*this);
    return *this;
  }
  virtual void AssignTo(DateAndTime& lhs) const
  {
    lhs.m_val = m_val;
  }
  virtual Value* Clone() const
  {
    return new DateAndTime(m_val);
  }
  virtual eTelemetryTypeBasic GetType() const
  {
     return eTTB_String;
  }
  virtual std::string ToStr() const
  {
    // Prints values as integers.
    std::ostringstream buf;
    std::copy(m_val.begin(), m_val.end(),
              std::ostream_iterator<int>(buf, " "));
    return buf.str();
  }
  virtual void ToSeg(CSegment& out) const
  {
    out << m_val.size();
    for (std::vector<unsigned char>::const_iterator i = m_val.begin(); i != m_val.end(); ++i)
      out << *i;
  }
  virtual unsigned char* GetPtr(size_t& len) const
  {
    len = m_val.size();
    return const_cast<unsigned char*>(&m_val[0]);
  }

 private:
  DateAndTime(const std::vector<unsigned char>& val) : m_val(val) {}
  std::vector<unsigned char> m_val;
};

}  // Anonymous namespace

// Static
Value* CTelemetryValueImpl::Make(unsigned int val)                      { return new Integer(val); }
Value* CTelemetryValueImpl::Make(unsigned char val)                     { return new Boolean(val); }
Value* CTelemetryValueImpl::Make(const char* val)                       { return new String(val); }
Value* CTelemetryValueImpl::Make(const std::vector<unsigned char>& val) { return new DateAndTime(val); }

CTelemetryValue::CTelemetryValue() : m_impl(NULL), m_type(eTT_Unknown)
{}

// SNMP syntax demands 1 for true and 2 for false
CTelemetryValue::CTelemetryValue(eTelemetryType type, unsigned char val) :
    m_impl(NULL), m_type(eTT_Unknown)
{
  PASSERTSTREAM_AND_RETURN(eTTB_Bool != CTelemetryTraits::Get(type).btype,
    "Inappropriate type "
    << TelemetryTypeBasicToStr(CTelemetryTraits::Get(type).btype)
    << " of " << TelemetaryTypeToStr(type));

  m_type = type;
  m_impl = Value::Make(val);
}

CTelemetryValue::CTelemetryValue(eTelemetryType type, unsigned int val) :
  m_impl(NULL), m_type(eTT_Unknown)
{
  PASSERTSTREAM_AND_RETURN(eTTB_Integer != CTelemetryTraits::Get(type).btype,
    "Inappropriate type "
    << TelemetryTypeBasicToStr(CTelemetryTraits::Get(type).btype)
    << " of " << TelemetaryTypeToStr(type));

  m_type = type;
  m_impl = Value::Make(val);
}

CTelemetryValue::CTelemetryValue(eTelemetryType type, const char* val) :
  m_impl(NULL), m_type(eTT_Unknown)
{
  PASSERTSTREAM_AND_RETURN(NULL == val, "Invalid input");
  PASSERTSTREAM_AND_RETURN(eTTB_String != CTelemetryTraits::Get(type).btype,
    "Inappropriate type "
    << TelemetryTypeBasicToStr(CTelemetryTraits::Get(type).btype)
    << " of " << TelemetaryTypeToStr(type));

  // 256 bytes is netsnmp restriction.
  PASSERTSTREAM_AND_RETURN(strlen(val) > 256u,
    "Illegal input, len:" << strlen(val) << ", val:" << val << "for "
    << TelemetryTypeBasicToStr(CTelemetryTraits::Get(type).btype)
    << " of " << TelemetaryTypeToStr(type));

  m_type = type;
  m_impl = Value::Make(val);
}

CTelemetryValue::CTelemetryValue(eTelemetryType type,
                                 const std::vector<unsigned char>& val) :
  m_impl(NULL), m_type(eTT_Unknown)
{
  PASSERTSTREAM_AND_RETURN(eTTB_DateAndTime != CTelemetryTraits::Get(type).btype,
    "Inappropriate type "
    << TelemetryTypeBasicToStr(CTelemetryTraits::Get(type).btype)
    << " of " << TelemetaryTypeToStr(type));

  // 11 or 8 bytes for DateAndTime is netsnmp restriction.
  PASSERTSTREAM_AND_RETURN(11 != val.size() && 8 != val.size(),
    "Illegal input data, len:" << val.size() << " for "
    << TelemetryTypeBasicToStr(CTelemetryTraits::Get(type).btype)
    << " of " << TelemetaryTypeToStr(type));

  m_type = type;
  m_impl = Value::Make(val);
}

CTelemetryValue::CTelemetryValue(CSegment& seg) :
  m_impl(NULL), m_type(eTT_Unknown)
{
  DWORD t;
  seg >> t;

  eTelemetryType type = static_cast<eTelemetryType>(t);
  PASSERTSTREAM_AND_RETURN(type < eTT_Unknown || type >= eTT_Max,
      "Invalid input " << TelemetaryTypeToStr(type));

  // It is serialized default value
  if (eTT_Unknown == type)
    return;

  switch (CTelemetryTraits::Get(type).btype)
  {
    case eTTB_Integer:
    {
      DWORD val;
      seg >> val;

      m_type = type;
      m_impl = Value::Make(val);
      break;
    }

    case eTTB_Bool:
    {
      BYTE val;
      seg >> val;

      m_type = type;
      m_impl = Value::Make(val);
      break;
    }

    case eTTB_String:
    {
      std::string val;
      seg >> val;

      // 256 bytes is netsnmp restriction.
      PASSERTSTREAM_AND_RETURN(val.length() > 256u,
          "Illegal input data for " << TelemetaryTypeToStr(type)
          << ", len:" << val.length() << ", val:" << val);

      m_type = type;
      m_impl = Value::Make(val.c_str());
      break;
    }

    case eTTB_DateAndTime:
    {
      DWORD len;
      seg >> len;

      PASSERTSTREAM_AND_RETURN(11 != len && 8 != len,
        "Illegal input data, len:" << len << " for "
        << TelemetryTypeBasicToStr(CTelemetryTraits::Get(type).btype)
        << " of " << TelemetaryTypeToStr(type));

      std::vector<unsigned char> val;
      val.reserve(len);
      for (unsigned int i = 0; i < len; ++i)
      {
        unsigned char c;
        seg >> c;
        val.push_back(c);
      }

      m_type = type;
      m_impl = Value::Make(val);
      break;
    }

    default:
      PASSERTSTREAM(true,
          "Unprocessed "
          << TelemetryTypeBasicToStr(CTelemetryTraits::Get(type).btype)
          << " for " << TelemetaryTypeToStr(type));
  }
}

CTelemetryValue::CTelemetryValue(const CTelemetryValue& rhs) :
  CPObject(rhs),
  m_impl((NULL != rhs.m_impl) ? rhs.m_impl->Clone() : NULL),
  m_type(rhs.m_type)
{}

CTelemetryValue& CTelemetryValue::operator=(const CTelemetryValue& rhs)
{
  if (&rhs == this)
    return *this;

  
  if (NULL != m_impl)
  {
    if (NULL != rhs.m_impl)
    {
      if (rhs.m_type == m_type)
      {
        *m_impl = *rhs.m_impl;
      }
      else
      {
         Value::Destroy(m_impl);
         m_impl = rhs.m_impl->Clone();
      }
    }
    else
    {
      Value::Destroy(m_impl);
      m_impl = NULL;
    }
  }
  else
  {
    if (NULL != rhs.m_impl)
      m_impl = rhs.m_impl->Clone();
  }

  m_type = rhs.m_type;

  return *this;
}

CTelemetryValue::~CTelemetryValue()
{
  Value::Destroy(m_impl);
}

// Virtual, private.
const char* CTelemetryValue::NameOf() const
{
  return GetCompileType();
}

CTelemetryValue& CTelemetryValue::Update(const CTelemetryValue& rhs)
{
  if (IsEmpty())
    return *this = rhs;

  if (rhs.IsEmpty())
    return *this;

  // Update should be done only with the same type operands.
  PASSERTSTREAM_AND_RETURN_VALUE(m_type != rhs.m_type,
      "Bad types " << TelemetaryTypeToStr(m_type)
      << " and " << TelemetaryTypeToStr(rhs.m_type),
      *this);

  switch (CTelemetryTraits::Get(m_type).action)
  {
    case eTA_Increment: *m_impl += *rhs.m_impl; break;
    case eTA_Decrement: *m_impl -= *rhs.m_impl; break;
    case eTA_Assign:    *m_impl = *rhs.m_impl;  break;
    default:
      PASSERTSTREAM(true,
          "Unprocessed "
          << TelemetryActionToStr(CTelemetryTraits::Get(m_type).action)
          << " for " << TelemetaryTypeToStr(m_type));
  }

  return *this;
}

bool CTelemetryValue::IsEmpty() const
{
  PASSERTSTREAM(eTT_Unknown == m_type && m_impl != NULL,
      "Wrong initialization of " << TelemetaryTypeToStr(m_type));
  return eTT_Unknown == m_type;
}

unsigned char* CTelemetryValue::GetPtr(size_t& len) const
{
  if (IsEmpty())
    return NULL;

  return m_impl->GetPtr(len);
}

CSegment CTelemetryValue::GetSegment() const
{
  CSegment seg;
  seg << static_cast<DWORD>(m_type);

  if (IsEmpty())
    return seg;

  m_impl->ToSeg(seg);
  return seg;
}

eTelemetryType CTelemetryValue::GetType() const
{
  return m_type;
}

// Virtual, private.
void CTelemetryValue::Dump(std::ostream& out) const
{
  if (IsEmpty())
    return;

  out << m_impl->ToStr();
}

// Static, private.
const CTelemetryTraits::CIndexer CTelemetryTraits::s_traits;

// Template specification for std::vector. Does not work with the reference.
template<>
void CTelemetryTraits::CIndexer::Add(eTelemetryType             type,
                                     eTelemetryAction           action,
                                     eTelemetryTypeBasic        btype,
                                     bool                       prolonged,
                                     bool                       calculated,
                                     std::vector<unsigned char> value)
{
  // We could not keeep default value as CTelemetryValue because the constructor
  // and operator= use CTelemetryTraits. The workaround is to keep default
  // value as CSegment.
  CSegment dvalue;
  dvalue << static_cast<DWORD>(type);

  dvalue << value.size();
  for (std::vector<unsigned char>::const_iterator i = value.begin(); i != value.end(); ++i)
    dvalue << *i;

  m_db.insert(std::make_pair(type, CTelemetryTraits(action,
                                                    btype,
                                                    prolonged,
                                                    calculated,
                                                    dvalue)));
}

template<typename T>
void CTelemetryTraits::CIndexer::Add(eTelemetryType      type,
                                     eTelemetryAction    action,
                                     eTelemetryTypeBasic btype,
                                     bool                prolonged,
                                     bool                calculated,
                                     T                   value)
{
  // We could not keeep default value as CTelemetryValue because the constructor
  // and operator= use CTelemetryTraits. The workaround is to keep default
  // value as CSegment.
  CSegment dvalue;
  dvalue << static_cast<DWORD>(type) << value;

  m_db.insert(std::make_pair(type, CTelemetryTraits(action,
                                                    btype,
                                                    prolonged,
                                                    calculated,
                                                    dvalue)));
}

CTelemetryTraits::CIndexer::CIndexer()
{
  // Telemetry Type, Action, Value Type, Is Prolonged, Is Calculated, Default Value
  Add(eTT_Unknown                      , eTA_Unknown,   eTTB_Integer,     false, false, -1u);

  unsigned char null[] = { 0, 0, 1, 1, 0, 0, 0, 0 };
  Add(eTT_IdentitySoftwareInfo         , eTA_Assign   , eTTB_String,      true, false,  "0.0.0.0");
  Add(eTT_IdentityBuildDate            , eTA_Assign   , eTTB_DateAndTime, true, false,  std::vector<unsigned char>(null, ARRAYEND(null)));
  Add(eTT_IdentityDeviceType           , eTA_Assign   , eTTB_String,      true, false,  "Undefined");
  Add(eTT_IdentityConsoleAccess        , eTA_Assign   , eTTB_Bool,        true, false,  (unsigned char)'\0');
  Add(eTT_MCUDebug                     , eTA_Assign   , eTTB_Bool,        true, false,  (unsigned char)'\0');
  Add(eTT_NetworkSeparation            , eTA_Assign   , eTTB_Bool,        true, false,  (unsigned char)'\0');
  Add(eTT_UltraSecureMode              , eTA_Assign   , eTTB_String,      true, false,  "Undefined");
  Add(eTT_MCUDisplayName               , eTA_Assign   , eTTB_String,      true, false,  "Undefined");
  Add(eTT_IncomingCallsReqrGK          , eTA_Assign   , eTTB_Bool,        true, false,  (unsigned char)'\0');
  Add(eTT_OutgoingCallsReqrGK          , eTA_Assign   , eTTB_Bool,        true, false,  (unsigned char)'\0');
  Add(eTT_PalNtsc                      , eTA_Assign   , eTTB_Integer,     true, false,  -1u);
  Add(eTT_RRQFirst                     , eTA_Assign   , eTTB_Bool,        true, false,  (unsigned char)'\0');

  Add(eTT_HDBitrateThrshld             , eTA_Assign   , eTTB_Integer,     true, false,  -1u);
  Add(eTT_MaxCPRstln                   , eTA_Assign   , eTTB_Integer,     true, false,  -1u);
  Add(eTT_MaxCPRstlnCfg                , eTA_Assign   , eTTB_Integer,     true, false,  -1u);
  Add(eTT_NumPorts                     , eTA_Assign   , eTTB_Integer,     true, false,  0u);
  Add(eTT_NumVideoPorts                , eTA_Assign   , eTTB_Integer,     true, false,  0u);
  Add(eTT_NumVoicePorts                , eTA_Assign   , eTTB_Integer,     true, false,  0u);
  Add(eTT_NumVideoPortsUsed            , eTA_Assign   , eTTB_Integer,     true, false,  0u);
  Add(eTT_NumVoicePortsUsed            , eTA_Assign   , eTTB_Integer,     true, false,  0u);
  Add(eTT_NumVideoPortsUsedPercentage  , eTA_Assign   , eTTB_Integer,     true, true,  0u);
  Add(eTT_NumVoicePortsUsedPercentage  , eTA_Assign   , eTTB_Integer,     true, true,  0u);
  Add(eTT_NumPortsUsed                 , eTA_Assign   , eTTB_Integer,     true, true,  0u);
  Add(eTT_RsrcAllocMode                , eTA_Assign   , eTTB_Integer,     true, false,  -1u);

  Add(eTT_H323Status                   , eTA_Assign   , eTTB_Integer,     true, false,  1u);
  Add(eTT_SIPStatus                    , eTA_Assign   , eTTB_Integer,     true, false,  1u);
  Add(eTT_ISDNStatus                   , eTA_Assign   , eTTB_Integer,     true, false,  1u);
  Add(eTT_VideoParticipants            , eTA_Assign   , eTTB_Integer,     true, false,  -1u);
  Add(eTT_VoiceParticipants            , eTA_Assign   , eTTB_Integer,     true, false,  -1u);
  
  Add(eTT_SuccessfulNewCalls           , eTA_Increment, eTTB_Integer,     false, false, 0u);
  Add(eTT_FailedNewCalls               , eTA_Increment, eTTB_Integer,     false, false, 0u);
  Add(eTT_SuccessfulEndCalls           , eTA_Increment, eTTB_Integer,     false, false, 0u);
  Add(eTT_FailedEndCalls               , eTA_Increment, eTTB_Integer,     false, false, 0u);

  Add(eTT_NewCalls                     , eTA_Assign   , eTTB_Integer,     false, true,  0u);
  Add(eTT_EndCalls                     , eTA_Assign   , eTTB_Integer,     false, true,  0u);
  Add(eTT_RatioSuccessfulNewCalls      , eTA_Assign   , eTTB_Integer,     false, true,  100u);
  Add(eTT_RatioSuccessfulEndCalls      , eTA_Assign   , eTTB_Integer,     false, true,  100u);
  Add(eTT_RatioPortsUsed               , eTA_Assign   , eTTB_Integer,     false, true,  100u);

  Add(eTT_ActiveConf                   , eTA_Assign   , eTTB_Integer,     true, false,  0u);
  Add(eTT_ActiveParticipant            , eTA_Assign   , eTTB_Integer,     true, false,  0u);
  Add(eTT_SystemStatus                 , eTA_Assign   , eTTB_Integer,     true, false,  1u);

  Add(eTT_HardwareOverallStatus        , eTA_Assign   , eTTB_Integer,     true, false,  3u);
  Add(eTT_HardwareFanStatus            , eTA_Assign   , eTTB_Integer,     true, false,  3u);
  Add(eTT_HardwarePowerSupplyStatus    , eTA_Assign   , eTTB_Integer,     true, false,  3u);
  Add(eTT_HardwareChassisTempStatus    , eTA_Assign   , eTTB_Integer,     true, false,  3u);
  Add(eTT_HardwareIntegratedBoardStatus, eTA_Assign   , eTTB_Integer,     true, false,  3u);

  Add(eTT_Max                          , eTA_Unknown  , eTTB_Unknown,     false, false, -1u);
}

const CTelemetryTraits& CTelemetryTraits::CIndexer::Find(eTelemetryType type) const
{
  static CTelemetryTraits undefined;
  std::map<eTelemetryType, CTelemetryTraits>::const_iterator ii = m_db.find(type);
  FPASSERTSTREAM_AND_RETURN_VALUE(m_db.end() == ii,
      "Unable to find traits for " << TelemetaryTypeToStr(type),
      undefined);

  return ii->second;
}

// Static.
const CTelemetryTraits& CTelemetryTraits::Get(eTelemetryType type)
{
  return s_traits.Find(type);
}
